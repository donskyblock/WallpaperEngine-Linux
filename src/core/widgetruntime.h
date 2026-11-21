// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QTimer>
#include <QSet>
#include <QElapsedTimer>
#include <memory>
namespace wpe {
class WidgetDataSources : public QObject {
    Q_OBJECT
public:
    explicit WidgetDataSources(QObject *parent = nullptr);
    void configure(const QVariantMap &document);
    void refresh();
    QVariantMap media() const { return m_media; }
    QVariantMap values() const { return m_values; }
signals:
    void changed();
private:
    QVariantList m_sources;
    QVariantMap m_media, m_values;
    bool m_mediaEnabled = false, m_mediaPending = false;
    QSet<QString> m_pending;
    quint64 m_generation = 0;
};

class WidgetRuntime : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(QVariantList frames READ frames NOTIFY framesChanged)
    Q_PROPERTY(QStringList errors READ errors NOTIFY errorsChanged)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(qreal surfaceWidth MEMBER m_width)
    Q_PROPERTY(qreal surfaceHeight MEMBER m_height)
public:
    explicit WidgetRuntime(QObject *parent = nullptr);
    ~WidgetRuntime() override;
    QVariantMap document() const { return m_document; }
    void setDocument(const QVariantMap &document);
    QVariantList frames() const { return m_frames; }
    QStringList errors() const { return m_errors; }
    bool active() const { return m_active; }
    void setActive(bool active);
    Q_INVOKABLE void setPointer(qreal x, qreal y, bool inside, bool pressed);
    Q_INVOKABLE void tick();
signals:
    void documentChanged();
    void framesChanged();
    void errorsChanged();
    void activeChanged();
private:
    struct Private;
    std::unique_ptr<Private> d;
    WidgetDataSources m_data;
    QTimer m_timer, m_dataTimer;
    QElapsedTimer m_elapsed;
    QVariantMap m_document, m_pointer;
    QVariantList m_frames;
    QStringList m_errors;
    bool m_active = true;
    bool m_pointerEnabled = false;
    qreal m_width = 1920, m_height = 1080;
    qint64 m_lastFrame = 0;
};
void registerNativeWidgets();
}
