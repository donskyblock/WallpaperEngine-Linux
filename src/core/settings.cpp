// SPDX-License-Identifier: GPL-3.0-or-later
#include "settings.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

namespace wpe {

Settings::Settings(QObject *parent) : QObject(parent)
{
    m_data.downloadDir = defaultDownloadDir();
    load();
}

QString Settings::filePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/wpe-linux/settings.json");
}

QString Settings::defaultDownloadDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/wpe-linux/wallpapers");
}

void Settings::set(const AppSettings &s)
{
    m_data = s;
    if (m_data.downloadDir.isEmpty()) m_data.downloadDir = defaultDownloadDir();
    save();
    emit changed();
}

bool Settings::load()
{
    QFile f(filePath());
    if (!f.open(QIODevice::ReadOnly)) return false;
    const QJsonObject o = QJsonDocument::fromJson(f.readAll()).object();
    auto str = [&](const char *k, const QString &d) { return o.contains(QLatin1String(k)) ? o[QLatin1String(k)].toString() : d; };
    m_data.steamApiKey = str("steam_api_key", m_data.steamApiKey);
    m_data.downloadDir = str("download_dir", m_data.downloadDir);
    if (m_data.downloadDir.isEmpty()) m_data.downloadDir = defaultDownloadDir();
    m_data.defaultMonitor = str("default_monitor", m_data.defaultMonitor);
    m_data.steamUsername = str("steam_username", m_data.steamUsername);
    m_data.audioEnabled = o.value("audio_enabled").toBool(m_data.audioEnabled);
    m_data.volume = o.value("volume").toInt(m_data.volume);
    m_data.fpsLimit = o.value("fps_limit").toInt(m_data.fpsLimit);
    m_data.placementOverride = str("placement_override", m_data.placementOverride);
    m_data.restoreOnStart = o.value("restore_on_start").toBool(m_data.restoreOnStart);
    return true;
}

bool Settings::save() const
{
    QDir().mkpath(QFileInfo(filePath()).absolutePath());
    QSaveFile f(filePath());
    if (!f.open(QIODevice::WriteOnly)) return false;
    const QJsonObject o{{"steam_api_key", m_data.steamApiKey}, {"download_dir", m_data.downloadDir},
                        {"default_monitor", m_data.defaultMonitor}, {"steam_username", m_data.steamUsername},
                        {"audio_enabled", m_data.audioEnabled}, {"volume", m_data.volume}, {"fps_limit", m_data.fpsLimit},
                        {"placement_override", m_data.placementOverride}, {"restore_on_start", m_data.restoreOnStart}};
    f.write(QJsonDocument(o).toJson(QJsonDocument::Indented));
    f.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner); // holds an API key
    return f.commit();
}

bool Settings::needsOnboarding() const { return m_data.steamApiKey.isEmpty(); }

} // namespace wpe
