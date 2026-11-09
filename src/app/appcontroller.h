// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "models.h"
#include "settings.h"
#include "steamcmd.h"
#include "wallpapermanager.h"
#include "workshop.h"

#include <QObject>
#include <QVariantList>
#include <QVariantMap>

namespace wpe {

// Everything the QML UI (and the D-Bus API) talks to.
class AppController : public QObject {
    Q_OBJECT
    Q_PROPERTY(wpe::WorkshopModel *workshop READ workshopModel CONSTANT)
    Q_PROPERTY(wpe::LibraryModel *library READ libraryModel CONSTANT)
    Q_PROPERTY(bool windowVisible READ windowVisible WRITE setWindowVisible NOTIFY windowVisibleChanged)
    Q_PROPERTY(bool onboarding READ onboarding WRITE setOnboarding NOTIFY onboardingChanged)
    Q_PROPERTY(QString environmentSummary READ environmentSummary NOTIFY environmentChanged)
    Q_PROPERTY(QStringList caveats READ caveats NOTIFY environmentChanged)
    Q_PROPERTY(QString desktopId READ desktopId NOTIFY environmentChanged)
    Q_PROPERTY(QVariantList displays READ displays NOTIFY displaysChanged)
    Q_PROPERTY(QString selectedOutput READ selectedOutput WRITE setSelectedOutput NOTIFY selectedOutputChanged)
    Q_PROPERTY(QVariantList active READ active NOTIFY activeChanged)
    Q_PROPERTY(QVariantMap tools READ toolStatus NOTIFY toolsChanged)
    Q_PROPERTY(QVariantMap settings READ settingsMap NOTIFY settingsChanged)
    Q_PROPERTY(bool loadingWorkshop READ loadingWorkshop NOTIFY loadingChanged)
    Q_PROPERTY(bool loadingMore READ loadingMore NOTIFY loadingChanged)
    Q_PROPERTY(QString workshopError READ workshopError NOTIFY loadingChanged)
    Q_PROPERTY(quint64 workshopTotal READ workshopTotal NOTIFY loadingChanged)
    Q_PROPERTY(QVariantMap downloads READ downloads NOTIFY downloadsChanged)
    Q_PROPERTY(bool applying READ applying NOTIFY applyingChanged)

public:
    AppController(Settings *settings, WallpaperManager *manager, QObject *parent = nullptr);

    WorkshopModel *workshopModel() { return &m_workshop; }
    LibraryModel *libraryModel() { return &m_library; }
    bool windowVisible() const { return m_windowVisible; }
    void setWindowVisible(bool v);
    bool onboarding() const { return m_onboarding; }
    void setOnboarding(bool v);
    QString environmentSummary() const;
    QStringList caveats() const;
    QString desktopId() const;
    QVariantList displays() const;
    QString selectedOutput() const { return m_selectedOutput; }
    void setSelectedOutput(const QString &o);
    QVariantList active() const;
    QVariantMap toolStatus() const { return m_tools; }
    QVariantMap settingsMap() const;
    bool loadingWorkshop() const { return m_loading && !m_loadingMore; }
    bool loadingMore() const { return m_loadingMore; }
    QString workshopError() const { return m_error; }
    quint64 workshopTotal() const { return m_total; }
    QVariantMap downloads() const { return m_downloads; }
    bool applying() const { return m_applying; }

    WallpaperManager *manager() { return m_manager; }
    Settings *settings() { return m_settings; }

    Q_INVOKABLE void search(const QString &query);
    Q_INVOKABLE void loadMore();
    Q_INVOKABLE void setSort(const QString &sort);
    Q_INVOKABLE void setTypeFilter(const QString &type);
    Q_INVOKABLE void filterLibrary(const QString &text, const QString &type);
    Q_INVOKABLE void refreshLibrary();
    Q_INVOKABLE void applyLibraryItem(int row);
    Q_INVOKABLE void applyWorkshopId(const QString &workshopId);
    Q_INVOKABLE void applyPath(const QString &pathOrUrl);
    Q_INVOKABLE void stopWallpaper();
    Q_INVOKABLE void download(const QString &itemId);
    Q_INVOKABLE void saveSettings(const QVariantMap &map);
    Q_INVOKABLE void steamLogin(const QString &user, const QString &password);
    Q_INVOKABLE void submitGuardCode(const QString &code);
    Q_INVOKABLE void installAssets();
    Q_INVOKABLE void installGnomeExtension();
    Q_INVOKABLE void installPlasmaPlugin();
    Q_INVOKABLE void refreshTools();
    Q_INVOKABLE void openExternal(const QString &url);
    Q_INVOKABLE void quitApp();
    Q_INVOKABLE QString formatBytes(qulonglong b) const;
    Q_INVOKABLE QString formatCount(qulonglong n) const;

    // used by the D-Bus API
    void applyRandom(const QString &output);

signals:
    void windowVisibleChanged();
    void onboardingChanged();
    void environmentChanged();
    void displaysChanged();
    void selectedOutputChanged();
    void activeChanged();
    void toolsChanged();
    void settingsChanged();
    void loadingChanged();
    void downloadsChanged();
    void applyingChanged();
    void toast(QString message, QString kind); // kind: info | success | error
    void guardCodeRequired(QString kind);
    void loginFinished(bool ok, QString message);
    void quitRequested();

private:
    void runSearch(int page);
    void syncActive();

    Settings *m_settings;
    WallpaperManager *m_manager;
    WorkshopClient m_client;
    SteamCmd m_steam;
    WorkshopModel m_workshop;
    LibraryModel m_library;

    bool m_windowVisible = true;
    bool m_onboarding = false;
    QString m_selectedOutput;
    QVariantMap m_tools;

    QString m_query, m_sort = "vote", m_type;
    int m_page = 1;
    bool m_loading = false, m_loadingMore = false;
    QString m_error;
    quint64 m_total = 0;
    QVariantMap m_downloads; // itemId -> last progress line
    bool m_applying = false;
};

} // namespace wpe
