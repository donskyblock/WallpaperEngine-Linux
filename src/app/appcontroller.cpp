// SPDX-License-Identifier: GPL-3.0-or-later
#include "appcontroller.h"

#include "tools.h"

#include <QDesktopServices>
#include <QGuiApplication>
#include <QScreen>
#include <QLocale>
#include <QRandomGenerator>
#include <QTimer>
#include <QUrl>

namespace wpe {

AppController::AppController(Settings *settings, WallpaperManager *manager, QObject *parent)
    : QObject(parent), m_settings(settings), m_manager(manager)
{
    m_onboarding = settings->needsOnboarding();
    refreshTools();
    refreshLibrary();

    auto ds = m_manager->displays();
    m_selectedOutput = settings->data().defaultMonitor;
    if (m_selectedOutput.isEmpty() && !ds.isEmpty()) {
        m_selectedOutput = ds.first().name;
        for (const Display &d : ds) if (d.primary) m_selectedOutput = d.name;
    }

    connect(&m_client, &WorkshopClient::searchFinished, this, [this](int page, QList<WorkshopItem> items, quint64 total) {
        m_loading = m_loadingMore = false;
        m_error.clear();
        m_total = total;
        m_page = page;
        if (page <= 1) m_workshop.reset(items); else m_workshop.append(items);
        m_workshop.setDownloaded(m_library.workshopIds());
        emit loadingChanged();
    });
    connect(&m_client, &WorkshopClient::failed, this, [this](QString msg) {
        m_loading = m_loadingMore = false;
        m_error = msg;
        emit loadingChanged();
        emit toast(msg, "error");
    });

    connect(&m_steam, &SteamCmd::progress, this, [this](QString id, QString line) {
        m_downloads[id] = line;
        emit downloadsChanged();
    });
    connect(&m_steam, &SteamCmd::downloadFinished, this, [this](QString id, bool ok, QString msg) {
        m_downloads.remove(id);
        emit downloadsChanged();
        if (ok) { emit toast(tr("Download finished"), "success"); refreshLibrary(); }
        else emit toast(msg, "error");
    });
    connect(&m_steam, &SteamCmd::guardCodeRequired, this, &AppController::guardCodeRequired);
    connect(&m_steam, &SteamCmd::loginFinished, this, [this](bool ok, QString msg) {
        emit loginFinished(ok, msg);
        emit toast(msg, ok ? "success" : "error");
    });
    connect(&m_steam, &SteamCmd::assetsFinished, this, [this](bool ok, QString msg) {
        m_downloads.remove(QStringLiteral("assets"));
        emit downloadsChanged();
        refreshTools();
        emit toast(ok ? tr("Wallpaper Engine assets installed") : msg, ok ? "success" : "error");
    });

    connect(m_manager, &WallpaperManager::statusChanged, this, &AppController::syncActive);
    connect(m_manager, &WallpaperManager::applied, this, [this](QString) { m_applying = false; emit applyingChanged(); emit toast(tr("Wallpaper applied"), "success"); });
    connect(m_manager, &WallpaperManager::failed, this, [this](QString, QString msg) { m_applying = false; emit applyingChanged(); emit toast(msg, "error"); });
    connect(m_settings, &Settings::changed, this, &AppController::settingsChanged);

    if (qGuiApp) {
        auto refresh = [this] { emit displaysChanged(); };
        connect(qGuiApp, &QGuiApplication::screenAdded, this, refresh);
        connect(qGuiApp, &QGuiApplication::screenRemoved, this, refresh);
    }
    syncActive();
}

void AppController::setWindowVisible(bool v) { if (m_windowVisible != v) { m_windowVisible = v; emit windowVisibleChanged(); } }
void AppController::setOnboarding(bool v) { if (m_onboarding != v) { m_onboarding = v; emit onboardingChanged(); } }

QString AppController::environmentSummary() const { return m_manager->plan().summary; }
QStringList AppController::caveats() const { return m_manager->plan().caveats; }
QString AppController::desktopId() const { return desktopName(m_manager->environment().desktop); }

QVariantList AppController::displays() const
{
    QVariantList l;
    for (const Display &d : m_manager->displays()) l << d.toJson().toVariantMap();
    return l;
}

void AppController::setSelectedOutput(const QString &o)
{
    if (m_selectedOutput == o) return;
    m_selectedOutput = o;
    emit selectedOutputChanged();
    syncActive();
}

QVariantList AppController::active() const
{
    QVariantList l;
    for (const QJsonValue &v : m_manager->status()) l << v.toObject().toVariantMap();
    return l;
}

void AppController::syncActive()
{
    QSet<QString> paths;
    for (const QJsonValue &v : m_manager->status()) {
        const QJsonObject o = v.toObject();
        if (m_selectedOutput.isEmpty() || o.value("output").toString() == m_selectedOutput) paths.insert(o.value("path").toString());
    }
    m_library.setActivePaths(paths);
    emit activeChanged();
}

QVariantMap AppController::settingsMap() const
{
    const AppSettings &s = m_settings->data();
    return {{"steamApiKey", s.steamApiKey}, {"downloadDir", s.downloadDir}, {"defaultMonitor", s.defaultMonitor},
            {"steamUsername", s.steamUsername}, {"audioEnabled", s.audioEnabled}, {"volume", s.volume},
            {"fpsLimit", s.fpsLimit}, {"placementOverride", s.placementOverride}, {"restoreOnStart", s.restoreOnStart}};
}

void AppController::saveSettings(const QVariantMap &m)
{
    AppSettings s = m_settings->data();
    auto str = [&](const char *k, QString &dst) { if (m.contains(k)) dst = m.value(k).toString(); };
    str("steamApiKey", s.steamApiKey); str("downloadDir", s.downloadDir); str("defaultMonitor", s.defaultMonitor);
    str("steamUsername", s.steamUsername); str("placementOverride", s.placementOverride);
    if (m.contains("audioEnabled")) s.audioEnabled = m.value("audioEnabled").toBool();
    if (m.contains("volume")) s.volume = m.value("volume").toInt();
    if (m.contains("fpsLimit")) s.fpsLimit = m.value("fpsLimit").toInt();
    if (m.contains("restoreOnStart")) s.restoreOnStart = m.value("restoreOnStart").toBool();
    s.steamApiKey = s.steamApiKey.trimmed();
    m_settings->set(s);
    emit environmentChanged();
    refreshLibrary();
    emit toast(tr("Settings saved"), "success");
}

// ---- workshop --------------------------------------------------------------------------
void AppController::runSearch(int page)
{
    const AppSettings &s = m_settings->data();
    m_loading = true;
    m_loadingMore = page > 1;
    m_error.clear();
    emit loadingChanged();
    m_client.search(s.steamApiKey, m_query, page, m_sort, m_type);
}

void AppController::search(const QString &query) { m_query = query; runSearch(1); }
void AppController::loadMore() { if (!m_loading && m_workshop.count() > 0 && quint64(m_workshop.count()) < m_total) runSearch(m_page + 1); }
void AppController::setSort(const QString &s) { m_sort = s; runSearch(1); }
void AppController::setTypeFilter(const QString &t) { m_type = t == "all" ? QString() : t; runSearch(1); }

void AppController::download(const QString &id)
{
    if (m_downloads.contains(id)) return;
    m_downloads[id] = tr("Queued…");
    emit downloadsChanged();
    m_steam.download(id, m_settings->data().steamUsername, m_settings->data().downloadDir);
}

// ---- library / apply -------------------------------------------------------------------
void AppController::refreshLibrary()
{
    m_library.setItems(scanLibrary(m_settings->data().downloadDir));
    m_workshop.setDownloaded(m_library.workshopIds());
    syncActive();
}

void AppController::filterLibrary(const QString &text, const QString &type) { m_library.setFilter(text, type); }

void AppController::applyLibraryItem(int row)
{
    const QVariantMap it = m_library.get(row);
    if (it.isEmpty()) return;
    m_applying = true;
    emit applyingChanged();
    m_manager->apply(it.value("path").toString(), it.value("contentType").toString(), m_selectedOutput);
}

void AppController::applyWorkshopId(const QString &id)
{
    for (const LibraryItem &it : scanLibrary(m_settings->data().downloadDir))
        if (it.workshopId == id) {
            m_applying = true;
            emit applyingChanged();
            m_manager->apply(it.path, it.contentType, m_selectedOutput);
            return;
        }
    emit toast(tr("Download it first."), "error");
}

void AppController::applyPath(const QString &pathOrUrl)
{
    const QUrl u(pathOrUrl);
    const QString path = u.isLocalFile() ? u.toLocalFile() : pathOrUrl;
    m_applying = true;
    emit applyingChanged();
    m_manager->apply(path, {}, m_selectedOutput);
}

void AppController::applyRandom(const QString &output)
{
    const QList<LibraryItem> items = scanLibrary(m_settings->data().downloadDir);
    if (items.isEmpty()) { emit toast(tr("Library is empty"), "error"); return; }
    const LibraryItem &it = items.at(QRandomGenerator::global()->bounded(items.size()));
    m_manager->apply(it.path, it.contentType, output);
}

void AppController::stopWallpaper() { m_manager->stop(m_selectedOutput); }

// ---- steam / tools ---------------------------------------------------------------------
void AppController::steamLogin(const QString &user, const QString &password)
{
    AppSettings s = m_settings->data();
    s.steamUsername = user.trimmed();
    m_settings->set(s);
    m_steam.login(user.trimmed(), password);
}

void AppController::submitGuardCode(const QString &code) { m_steam.submitGuardCode(code.trimmed()); }

void AppController::installAssets()
{
    m_downloads[QStringLiteral("assets")] = tr("Starting steamcmd…");
    emit downloadsChanged();
    m_steam.installAssets(m_settings->data().steamUsername, tools::ownAssetsInstallDir());
}

void AppController::installGnomeExtension()
{
    QString err;
    if (tools::installGnomeExtension(&err)) emit toast(tr("GNOME Shell extension installed. Log out and back in to activate it."), "success");
    else emit toast(err, "error");
    refreshTools();
}

void AppController::installPlasmaPlugin()
{
    QString err;
    if (tools::installPlasmaPlugin(&err)) emit toast(tr("Plasma wallpaper plugin installed."), "success");
    else emit toast(err, "error");
    refreshTools();
}

void AppController::refreshTools()
{
    m_tools = tools::status();
    emit toolsChanged();
}

void AppController::openExternal(const QString &url) { QDesktopServices::openUrl(QUrl::fromUserInput(url)); }
void AppController::quitApp() { emit quitRequested(); }

QString AppController::formatBytes(qulonglong b) const { return QLocale().formattedDataSize(qint64(b), 1); }

QString AppController::formatCount(qulonglong n) const
{
    if (n >= 1000000) return QString::number(n / 1000000.0, 'f', 1) + "M";
    if (n >= 1000) return QString::number(n / 1000.0, 'f', 1) + "k";
    return QString::number(n);
}

} // namespace wpe
