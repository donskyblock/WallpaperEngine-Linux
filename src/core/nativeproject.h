// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QJsonObject>
#include <QString>
namespace wpe {
// Version 1 is data-only; version 2 adds bounded JavaScript widgets and declared
// read-only data sources. Neither format loads custom QML or external asset paths.
bool safeAssetPath(const QString &path);
bool validateNativeProject(const QJsonObject &project, const QString &directory, QString *error, bool requireFiles = true);
QJsonObject readNativeProject(const QString &file, QString *error = nullptr);
}
