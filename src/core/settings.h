// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <QString>

namespace wpe {

struct AppSettings {
    QString steamApiKey;
    QString downloadDir;
    QString defaultMonitor;
    QString steamUsername;
    bool audioEnabled = false;
    int volume = 50;
    int fpsLimit = 30;
    QString placementOverride; // "" = auto
    bool restoreOnStart = true;
};

// Same file + keys as the Tauri version (~/.config/wpe-linux/settings.json), so
// existing installs keep working. New keys are additive.
class Settings : public QObject {
    Q_OBJECT
public:
    explicit Settings(QObject *parent = nullptr);
    static QString filePath();
    static QString defaultDownloadDir();

    const AppSettings &data() const { return m_data; }
    void set(const AppSettings &s);
    bool load();
    bool save() const;
    bool needsOnboarding() const;

signals:
    void changed();

private:
    AppSettings m_data;
};

} // namespace wpe
