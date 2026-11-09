// SPDX-License-Identifier: GPL-3.0-or-later
#include "dbusapi.h"

#include "appcontroller.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace wpe {

DBusApi::DBusApi(AppController *c) : QDBusAbstractAdaptor(c), m_c(c) {}

void DBusApi::SetWallpaper(const QString &path, const QString &type, const QString &output) { m_c->manager()->apply(path, type, output); }
void DBusApi::Random(const QString &output) { m_c->applyRandom(output); }
void DBusApi::Stop(const QString &output) { m_c->manager()->stop(output); }
void DBusApi::Restore() { m_c->manager()->restore(); }
void DBusApi::Show() { m_c->setWindowVisible(true); }
void DBusApi::Quit() { m_c->quitApp(); }

QString DBusApi::Outputs()
{
    QJsonArray a;
    for (const Display &d : m_c->manager()->displays()) a.append(d.toJson());
    return QString::fromUtf8(QJsonDocument(a).toJson(QJsonDocument::Compact));
}

QString DBusApi::Status() { return QString::fromUtf8(QJsonDocument(m_c->manager()->status()).toJson(QJsonDocument::Compact)); }

QString DBusApi::Environment()
{
    const wpe::Environment e = m_c->manager()->environment();
    const Plan p = m_c->manager()->plan();
    const QJsonObject o{{"desktop", desktopName(e.desktop)}, {"session", sessionName(e.session)}, {"raw", e.rawDesktop},
                        {"placement", placementName(p.window)}, {"summary", p.summary},
                        {"caveats", QJsonArray::fromStringList(p.caveats)}};
    return QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact));
}

} // namespace wpe
