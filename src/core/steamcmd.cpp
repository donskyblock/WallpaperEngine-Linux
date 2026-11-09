// SPDX-License-Identifier: GPL-3.0-or-later
#include "steamcmd.h"

#include "library.h"
#include "tools.h"
#include "types.h"

#include <QDir>
#include <QRegularExpression>

namespace wpe {

SteamCmd::SteamCmd(QObject *parent) : QObject(parent) {}

bool SteamCmd::available() const { return !tools::find(QStringLiteral("steamcmd")).isEmpty(); }

QString SteamCmd::stripAnsi(QString s)
{
    static const QRegularExpression re(QStringLiteral("\x1b\\[[0-9;?]*[A-Za-z]"));
    return s.remove(re);
}

QString SteamCmd::explainError(const QString &line)
{
    if (line.contains("Invalid Password")) return tr("Steam rejected the password.");
    if (line.contains("Account Logon Denied") || line.contains("Steam Guard")) return tr("Steam Guard confirmation is required. Sign in from Settings → Steam first.");
    if (line.contains("Not logged on")) return tr("steamcmd is not signed in. Sign in from Settings → Steam first.");
    if (line.contains("No subscription")) return tr("This account doesn't own Wallpaper Engine (required for Workshop downloads).");
    return QStringLiteral("steamcmd: ") + line.trimmed();
}

// ---- login -----------------------------------------------------------------------
void SteamCmd::login(const QString &user, const QString &password)
{
    if (m_proc) { emit loginFinished(false, tr("steamcmd is busy.")); return; }
    const QString exe = tools::find(QStringLiteral("steamcmd"));
    if (exe.isEmpty()) { emit loginFinished(false, tr("steamcmd is not installed.")); return; }

    m_loginMode = true;
    m_password = password;
    m_sentPassword = false;
    m_buf.clear();
    m_lines.clear();
    m_proc = new QProcess(this);
    m_proc->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_proc, &QProcess::readyRead, this, &SteamCmd::onReadyRead);
    connect(m_proc, &QProcess::finished, this, [this](int, QProcess::ExitStatus) {
        onReadyRead();
        const QString all = m_lines.join('\n');
        const bool ok = all.contains("Logged in OK") || all.contains("Waiting for user info...OK");
        QString msg = ok ? tr("Signed in.") : tr("Sign-in failed.");
        if (!ok)
            for (const QString &l : std::as_const(m_lines))
                if (l.contains("FAILED") || l.contains("ERROR") || l.contains("Invalid")) { msg = explainError(l); break; }
        m_proc->deleteLater();
        m_proc = nullptr;
        m_loginMode = false;
        m_password.clear();
        emit loginFinished(ok, msg);
        pump();
    });
    m_proc->start(exe, {"+login", user, "+quit"});
}

void SteamCmd::submitGuardCode(const QString &code)
{
    if (m_proc && m_loginMode) m_proc->write(code.toUtf8() + "\n");
}

// ---- downloads -------------------------------------------------------------------
void SteamCmd::download(const QString &itemId, const QString &user, const QString &installDir)
{
    m_queue.enqueue({Job::Download, itemId, user, installDir});
    pump();
}

void SteamCmd::installAssets(const QString &user, const QString &installDir)
{
    m_queue.enqueue({Job::Assets, QStringLiteral("assets"), user, installDir});
    pump();
}

void SteamCmd::pump()
{
    if (m_proc || m_queue.isEmpty()) return;
    startJob(m_queue.dequeue());
}

void SteamCmd::startJob(const Job &j)
{
    m_job = j;
    const QString exe = tools::find(QStringLiteral("steamcmd"));
    auto fail = [&](const QString &msg) {
        if (j.type == Job::Download) emit downloadFinished(j.id, false, msg);
        else emit assetsFinished(false, msg);
        QMetaObject::invokeMethod(this, &SteamCmd::pump, Qt::QueuedConnection);
    };
    if (exe.isEmpty()) return fail(tr("steamcmd is not installed."));
    if (j.user.isEmpty()) return fail(tr("Steam username required: Wallpaper Engine items need an account that owns the app."));
    QDir().mkpath(j.dir);

    m_loginMode = false;
    m_buf.clear();
    m_lines.clear();
    m_proc = new QProcess(this);
    m_proc->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_proc, &QProcess::readyRead, this, &SteamCmd::onReadyRead);
    connect(m_proc, &QProcess::finished, this, [this](int, QProcess::ExitStatus) {
        onReadyRead();
        const QString all = m_lines.join('\n');
        const Job j = m_job;
        m_proc->deleteLater();
        m_proc = nullptr;

        QString err;
        for (const QString &l : std::as_const(m_lines))
            if (l.contains("ERROR")) { err = explainError(l); break; }
        if (err.isEmpty() && all.contains("Timeout downloading item")) err = tr("Download timed out. Check your connection.");
        if (err.isEmpty() && !all.contains("Success")) err = tr("steamcmd failed: %1").arg(m_lines.mid(qMax(0, m_lines.size() - 3)).join(" | "));

        if (j.type == Job::Assets) {
            emit assetsFinished(err.isEmpty(), err.isEmpty() ? j.dir : err);
        } else if (!err.isEmpty()) {
            emit downloadFinished(j.id, false, err);
        } else {
            const QString expected = workshopContentDir(j.dir) + "/" + j.id;
            emit downloadFinished(j.id, QDir(expected).exists(), QDir(expected).exists() ? expected : tr("Download reported success but files are missing at %1").arg(expected));
        }
        pump();
    });

    QStringList args{"+force_install_dir", j.dir, "+login", j.user};
    if (j.type == Job::Download) args << "+workshop_download_item" << QString::number(kWallpaperEngineAppId) << j.id;
    else args << "+app_update" << QString::number(kWallpaperEngineAppId) << "validate";
    args << "+quit";
    m_proc->start(exe, args);
}

void SteamCmd::onReadyRead()
{
    if (!m_proc) return;
    // steamcmd prompts ("Password:") have no trailing newline, so match on the raw buffer, not on lines.
    m_buf += stripAnsi(QString::fromUtf8(m_proc->readAll()));
    m_buf.replace(QLatin1Char('\r'), QLatin1Char('\n'));

    if (m_loginMode) {
        const QString low = m_buf.toLower();
        if (!m_sentPassword && low.contains("password:")) {
            m_proc->write(m_password.toUtf8() + "\n");
            m_sentPassword = true;
            m_buf.clear();
            return;
        }
        if (low.contains("steam guard code") || low.contains("two-factor code") || low.contains("authenticator")) {
            emit guardCodeRequired(low.contains("two-factor") || low.contains("authenticator") ? "two-factor" : "steam-guard");
            m_buf.clear();
            return;
        }
    }

    int nl;
    while ((nl = m_buf.indexOf(QLatin1Char('\n'))) >= 0) {
        const QString line = m_buf.left(nl).trimmed();
        m_buf.remove(0, nl + 1);
        if (line.isEmpty()) continue;
        m_lines << line;
        if (!m_loginMode) emit progress(m_job.id, line);
    }
}

} // namespace wpe
