// SPDX-License-Identifier: GPL-3.0-or-later
#include "wallpapermanager.h"

#include "nativeapply.h"
#include "tools.h"

#include <QDir>
#include <csignal>
#include <sys/prctl.h>
#include <QFile>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QScreen>
#include <QTimer>
#include <QUrl>

namespace wpe {

WallpaperManager::WallpaperManager(Settings *settings, QObject *parent)
    : QObject(parent), m_settings(settings), m_env(detectEnvironment()), m_plan(planFor(m_env))
{
}

WallpaperManager::~WallpaperManager()
{
    for (Session *s : std::as_const(m_sessions)) {
        if (s->proc) {
            s->proc->disconnect(this);
            s->proc->terminate();
            if (!s->proc->waitForFinished(1500)) s->proc->kill();
        }
        delete s;
    }
}

Plan WallpaperManager::plan() const
{
    Plan p = m_plan;
    Placement forced;
    if (parsePlacement(m_settings->data().placementOverride, &forced)) {
        p.window = forced;
        p.plasmaVideoPlugin = false;
        p.summary = tr("%1 · %2 (forced in settings)").arg(desktopName(m_env.desktop), placementName(forced));
        p.caveats.clear();
        p.caveats << tr("Placement forced to \"%1\" in settings.").arg(placementName(forced));
    }
    return p;
}

QList<Display> WallpaperManager::displays() const
{
    QList<Display> out;
    if (!qGuiApp) return out;
    for (QScreen *sc : QGuiApplication::screens()) {
        Display d;
        d.name = sc->name();
        d.description = (sc->manufacturer() + QLatin1Char(' ') + sc->model()).trimmed();
        if (d.description.isEmpty()) d.description = d.name;
        d.width = sc->geometry().width();
        d.height = sc->geometry().height();
        d.x = sc->geometry().x();
        d.y = sc->geometry().y();
        d.refreshRate = sc->refreshRate();
        d.scale = sc->devicePixelRatio();
        d.primary = sc == QGuiApplication::primaryScreen();
        out << d;
    }
    return out;
}

QRect WallpaperManager::geometryOf(const QString &output) const
{
    for (QScreen *sc : QGuiApplication::screens())
        if (sc->name() == output) return sc->geometry();
    return {};
}

QStringList WallpaperManager::resolveOutputs(const QString &output) const
{
    const auto ds = displays();
    if (output == QLatin1String("*")) {
        QStringList all;
        for (const Display &d : ds) all << d.name;
        return all;
    }
    if (!output.isEmpty()) return {output};
    const QString def = m_settings->data().defaultMonitor;
    for (const Display &d : ds)
        if (d.name == def) return {def};
    for (const Display &d : ds)
        if (d.primary) return {d.name};
    return ds.isEmpty() ? QStringList{} : QStringList{ds.first().name};
}

bool WallpaperManager::isActive(const QString &output) const { return m_sessions.contains(output); }

QJsonArray WallpaperManager::status() const
{
    QJsonArray a;
    for (const Session *s : m_sessions)
        a.append(QJsonObject{{"output", s->output}, {"path", s->path}, {"type", s->type}, {"mechanism", s->mechanism},
                             {"running", s->native || (s->proc && s->proc->state() == QProcess::Running)}});
    return a;
}

void WallpaperManager::fail(const QString &output, const QString &msg)
{
    qWarning().noquote() << "[wpe]" << output << msg;
    emit failed(output, msg);
}

// ---- public actions --------------------------------------------------------------------
void WallpaperManager::apply(const QString &path, const QString &typeHint, const QString &output)
{
    const Classified c = classify(path, typeHint);
    if (c.kind == Kind::Unknown) { fail(output, tr("Don't know how to display \"%1\".").arg(path)); return; }
    const QStringList outs = resolveOutputs(output);
    if (outs.isEmpty()) { fail(output, tr("No monitors found.")); return; }
    for (const QString &o : outs) applyOne(o, c, typeHint);
    saveState();
}

void WallpaperManager::stop(const QString &output)
{
    if (output.isEmpty() || output == QLatin1String("*")) {
        const auto keys = m_sessions.keys();
        for (const QString &k : keys) stopOne(k);
    } else {
        stopOne(output);
    }
    saveState();
    emit statusChanged();
}

void WallpaperManager::stopOne(const QString &output)
{
    Session *s = m_sessions.take(output);
    if (!s) return;
    if (s->proc) {
        s->proc->disconnect(this);
        s->proc->terminate();
        if (!s->proc->waitForFinished(1500)) s->proc->kill();
        s->proc->deleteLater();
    }
    if (s->placement == Placement::KWinScript) nativeapply::kwinUnpin("wpe-" + output);
    delete s;
}

// ---- dispatch --------------------------------------------------------------------------
void WallpaperManager::applyOne(const QString &output, const Classified &c, const QString &typeHint)
{
    Q_UNUSED(typeHint)
    stopOne(output);
    const Plan plan = this->plan();

    auto *s = new Session;
    s->output = output;
    s->path = c.mainPath;
    s->kind = c.kind;
    s->type = kindName(c.kind);
    s->placement = plan.window;

    QString err;
    bool ok = false;

    if (c.kind == Kind::Image && plan.still != StaticMode::HostWindow) {
        s->native = true;
        s->mechanism = QStringLiteral("native");
        ok = nativeapply::applyStill(plan.still, output, c.mainPath, &err);
    } else if (c.kind == Kind::Video && plan.plasmaVideoPlugin && tools::plasmaPluginInstalled()) {
        s->native = true;
        s->mechanism = QStringLiteral("plasma-plugin");
        ok = nativeapply::plasmaSetWallpaper("org.wpelinux.video", "VideoPath", QUrl::fromLocalFile(c.mainPath).toString(), output, &err);
    } else if (c.kind == Kind::Image || c.kind == Kind::Video) {
        s->mechanism = placementName(plan.window);
        ok = spawnHost(s, c, &err);
    } else {
        s->mechanism = QStringLiteral("lwe/") + placementName(plan.window);
        ok = spawnLwe(s, c, &err);
    }

    if (!ok) {
        delete s;
        fail(output, err);
        return;
    }
    m_sessions.insert(output, s);
    if (s->native) {
        emit applied(output);
        emit statusChanged();
    } else {
        launch(s);
    }
}

bool WallpaperManager::spawnHost(Session *s, const Classified &c, QString *err)
{
    const QString host = tools::hostBinary();
    if (host.isEmpty()) { *err = tr("wpe-wallpaper-host was not found next to the application."); return false; }
    const AppSettings &cfg = m_settings->data();
    s->program = host;
    s->argv = {"--output", s->output, "--kind", kindName(c.kind), "--path", c.mainPath, "--placement", placementName(s->placement),
               "--volume", QString::number(cfg.audioEnabled ? cfg.volume : 0)};
    s->env = QProcessEnvironment::systemEnvironment();
    return true;
}

bool WallpaperManager::spawnLwe(Session *s, const Classified &c, QString *err)
{
    const QString lwe = tools::find(QStringLiteral("linux-wallpaperengine"));
    if (lwe.isEmpty()) {
        *err = tr("linux-wallpaperengine is not installed. It renders Scene and Web wallpapers (Arch: yay -S linux-wallpaperengine).");
        return false;
    }
    const QString assets = tools::wallpaperEngineAssets();
    if (assets.isEmpty()) {
        *err = tr("Wallpaper Engine assets not found. Install Wallpaper Engine through Steam, or use Settings → Download assets.");
        return false;
    }
    const AppSettings &cfg = m_settings->data();
    // Scenes want the *directory*, Web wallpapers want the project directory too.
    const QString target = (c.kind == Kind::Web) ? QFileInfo(c.mainPath).absolutePath() : c.mainPath;

    s->program = lwe;
    s->argv = {"--assets-dir", assets, "--bg", target, "--disable-mouse", "--fps", QString::number(cfg.fpsLimit)};
    if (cfg.audioEnabled) s->argv << "--volume" << QString::number(cfg.volume);
    else s->argv << "--silent";

    switch (s->placement) {
    case Placement::LayerShell:
    case Placement::X11Desktop:
        s->argv << "--screen-root" << s->output;
        break;
    case Placement::GnomeShellExtension:
    case Placement::KWinScript: {
        // No layer-shell: run as a normal window sized to the monitor; our extension / KWin script pins it.
        const QRect g = geometryOf(s->output);
        s->argv << "--window" << QStringLiteral("%1x%2x%3x%4").arg(g.x()).arg(g.y()).arg(g.width()).arg(g.height());
        break;
    }
    }
    s->env = QProcessEnvironment::systemEnvironment();
    s->env.insert("LC_ALL", "C"); // lwe parses JSON floats with the C locale rules
    return true;
}

void WallpaperManager::launch(Session *s)
{
    s->proc = new QProcess(this);
    s->proc->setProgram(s->program);
    s->proc->setArguments(s->argv);
    s->proc->setProcessEnvironment(s->env);
    s->proc->setStandardOutputFile(QProcess::nullDevice());
    // Children must never outlive us (logout, SIGTERM from systemd, a crash): ask the kernel to signal them.
    s->proc->setChildProcessModifier([] { ::prctl(PR_SET_PDEATHSIG, SIGTERM); });
    s->stderrTail.clear();
    s->settled = false;

    connect(s->proc, &QProcess::readyReadStandardError, this, [s] {
        s->stderrTail += s->proc->readAllStandardError();
        if (s->stderrTail.size() > 8192) s->stderrTail.remove(0, s->stderrTail.size() - 8192);
    });

    const QString out = s->output;
    connect(s->proc, &QProcess::finished, this, [this, out, s](int code, QProcess::ExitStatus st) {
        if (m_sessions.value(out) != s) return; // already replaced/stopped
        const QString why = st == QProcess::CrashExit ? tr("crashed") : tr("exited with code %1").arg(code);
        const QString detail = explainLweCrash(s->stderrTail);
        s->proc->deleteLater();
        s->proc = nullptr;

        if (!s->settled || s->restarts >= 3) {
            m_sessions.remove(out);
            if (s->placement == Placement::KWinScript) nativeapply::kwinUnpin("wpe-" + out);
            fail(out, tr("%1 %2: %3").arg(QFileInfo(s->program).fileName(), why, detail));
            delete s;
            saveState();
            emit statusChanged();
            return;
        }
        ++s->restarts;
        QTimer::singleShot(2000, this, [this, out, s] {
            if (m_sessions.value(out) == s) launch(s);
        });
    });

    s->proc->start();

    QTimer::singleShot(900, this, [this, out, s] {
        if (m_sessions.value(out) != s || !s->proc || s->proc->state() != QProcess::Running) return;
        s->settled = true;
        if (s->placement == Placement::KWinScript) {
            QString err;
            if (!nativeapply::kwinPinWindow("wpe-" + out, s->proc->processId(), geometryOf(out), &err)) fail(out, err);
        }
        emit applied(out);
        emit statusChanged();
    });
}

QString WallpaperManager::explainLweCrash(const QByteArray &tail)
{
    const QString t = QString::fromUtf8(tail).trimmed();
    if (t.isEmpty()) return tr("no output — likely a Vulkan/EGL init failure; update your GPU drivers and the renderer.");
    const QStringList lines = t.split('\n');
    for (auto it = lines.crbegin(); it != lines.crend(); ++it) {
        const QString l = it->trimmed();
        if (l.contains("what():") || l.contains("json.exception") || l.contains("Cannot find") || l.contains("filesystem error")
            || l.contains("Failed to") || l.contains("terminate called") || l.contains("not supported"))
            return l;
    }
    return lines.last().trimmed();
}

// ---- persistence -----------------------------------------------------------------------
void WallpaperManager::saveState() const
{
    if (m_restoring) return;
    QJsonObject o;
    for (const Session *s : m_sessions) o[s->output] = QJsonObject{{"path", s->path}, {"type", s->type}};
    const QString p = tools::stateFilePath();
    QDir().mkpath(QFileInfo(p).absolutePath());
    QSaveFile f(p);
    if (f.open(QIODevice::WriteOnly)) { f.write(QJsonDocument(o).toJson(QJsonDocument::Compact)); f.commit(); }
}

void WallpaperManager::restore()
{
    QFile f(tools::stateFilePath());
    if (!f.open(QIODevice::ReadOnly)) return;
    const QJsonObject o = QJsonDocument::fromJson(f.readAll()).object();
    const auto known = displays();
    m_restoring = true;
    for (auto it = o.begin(); it != o.end(); ++it) {
        const QString out = it.key();
        const bool exists = std::any_of(known.begin(), known.end(), [&](const Display &d) { return d.name == out; });
        const QString path = it.value().toObject().value("path").toString();
        if (exists && QFileInfo::exists(path)) apply(path, it.value().toObject().value("type").toString(), out);
    }
    m_restoring = false;
}

} // namespace wpe
