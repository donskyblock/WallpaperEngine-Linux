// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "environment.h"
#include "library.h"
#include "settings.h"
#include "types.h"

#include <QHash>
#include <QJsonArray>
#include <QObject>
#include <QProcess>

namespace wpe {

// Owns every running wallpaper: one Session per output. Picks the right mechanism for the current
// compositor (see planFor()), supervises the child process, and persists state for restore-on-login.
class WallpaperManager : public QObject {
    Q_OBJECT
public:
    explicit WallpaperManager(Settings *settings, QObject *parent = nullptr);
    ~WallpaperManager() override;

    const Environment &environment() const { return m_env; }
    Plan plan() const;                        // includes the user's placement override
    QList<Display> displays() const;
    QJsonArray status() const;                // [{output, path, type, mechanism, running}]
    bool isActive(const QString &output) const;

public slots:
    // output: "" = default/primary monitor, "*" = every monitor, otherwise a connector name (DP-1, HDMI-A-1, ...)
    void apply(const QString &path, const QString &typeHint, const QString &output);
    void stop(const QString &output);         // "" or "*" = everything
    void restore();

signals:
    void statusChanged();
    void applied(QString output);
    void failed(QString output, QString message);

private:
    struct Session {
        QString output, path, type, mechanism;
        Kind kind = Kind::Unknown;
        QProcess *proc = nullptr;
        QStringList argv;       // for restart
        QString program;
        QProcessEnvironment env;
        Placement placement = Placement::LayerShell;
        int restarts = 0;
        bool settled = false;   // survived the start-up grace period
        QByteArray stderrTail;
        bool native = false;    // applied through the DE, no process of ours
    };

    QStringList resolveOutputs(const QString &output) const;
    void applyOne(const QString &output, const Classified &c, const QString &typeHint);
    bool spawnHost(Session *s, const Classified &c, QString *err);
    bool spawnLwe(Session *s, const Classified &c, QString *err);
    void launch(Session *s);
    void stopOne(const QString &output);
    void saveState() const;
    QRect geometryOf(const QString &output) const;
    void fail(const QString &output, const QString &msg);
    static QString explainLweCrash(const QByteArray &stderrTail);

    Settings *m_settings;
    Environment m_env;
    Plan m_plan;
    QHash<QString, Session *> m_sessions;
    bool m_restoring = false;
};

} // namespace wpe
