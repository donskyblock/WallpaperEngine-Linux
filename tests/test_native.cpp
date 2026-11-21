// SPDX-License-Identifier: GPL-3.0-or-later
#include "nativeproject.h"
#include "studio.h"
#include "library.h"
#include <QtTest>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QTimer>
#include <QCryptographicHash>
#include <cstring>
using namespace wpe;
class MemoryReply : public QNetworkReply {
    QByteArray m_data;
    qint64 m_offset = 0;
public:
    MemoryReply(const QNetworkRequest &req, QByteArray data, QObject *parent) : QNetworkReply(parent), m_data(data) {
        setRequest(req); setUrl(req.url()); setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
        open(QIODevice::ReadOnly | QIODevice::Unbuffered);
        QTimer::singleShot(0, this, [this] { emit readyRead(); setFinished(true); emit finished(); });
    }
    void abort() override { setError(OperationCanceledError, "aborted"); }
    qint64 bytesAvailable() const override { return m_data.size() - m_offset + QNetworkReply::bytesAvailable(); }
protected:
    qint64 readData(char *data, qint64 max) override {
        const auto n = qMin(max, m_data.size() - m_offset);
        if (n <= 0) return -1;
        std::memcpy(data, m_data.constData() + m_offset, n); m_offset += n; return n;
    }
};
class MemoryNetwork : public QNetworkAccessManager {
public:
    QHash<QString, QByteArray> responses;
protected:
    QNetworkReply *createRequest(Operation, const QNetworkRequest &req, QIODevice *) override {
        return new MemoryReply(req, responses.value(req.url().path()), this);
    }
};
class TestNative : public QObject {
    Q_OBJECT
    QTemporaryDir config;
    static void put(const QString &path, const QByteArray &data) {
        QFile f(path); QVERIFY(f.open(QIODevice::WriteOnly)); QCOMPARE(f.write(data), data.size());
    }
    static QJsonObject document() {
        return {{"format", "wpe-native"}, {"version", 1}, {"title", "Clock"}, {"background", "#102030"},
                {"layers", QJsonArray{QJsonObject{{"type", "clock"}, {"text", "hh:mm"}, {"x", 0.2}, {"height", 0.3}}}}};
    }
private slots:
    void initTestCase() { qputenv("XDG_CONFIG_HOME", config.path().toUtf8()); }
    void validatesDataOnlyFormat() {
        auto p = document(); QString error; QTemporaryDir d;
        QVERIFY(validateNativeProject(p, d.path(), &error));
        p["version"] = 3; QVERIFY(!validateNativeProject(p, d.path(), &error));
        p = document(); p["layers"] = QJsonArray{QJsonObject{{"type", "qml"}}};
        QVERIFY(!validateNativeProject(p, d.path(), &error));
        p["layers"] = QJsonArray{QJsonObject{{"type", "text"}, {"opacity", 2}}};
        QVERIFY(!validateNativeProject(p, d.path(), &error));
    }
    void rejectsTraversalMissingAssetsAndSymlinks() {
        QTemporaryDir d, outside; QString error;
        put(outside.filePath("secret.png"), "image");
        QVERIFY(QFile::link(outside.path(), d.filePath("linked")));
        for (const auto &path : {QString("../secret.png"), QString("/tmp/a.png"), QString("https://host/a.png"), QString("linked/secret.png"), QString("missing.png")}) {
            auto p = document(); p["layers"] = QJsonArray{QJsonObject{{"type", "image"}, {"source", path}}};
            QVERIFY2(!validateNativeProject(p, d.path(), &error), qPrintable(path));
        }
    }
    void importsMediaAndSavesPortableProjects() {
        QTemporaryDir d, sources;
        Settings settings; auto cfg = settings.data(); cfg.downloadDir = d.path(); settings.set(cfg);
        Studio studio(&settings); QSignalSpy installed(&studio, &Studio::installed);
        put(sources.filePath("photo.png"), "fake image bytes");
        auto imported = studio.importMedia(QUrl::fromLocalFile(sources.filePath("photo.png")).toString());
        QVERIFY(!imported.isEmpty());
        auto p = document(); p["layers"] = QJsonArray{QJsonObject{{"type", "image"}, {"source", QUrl::fromLocalFile(sources.filePath("photo.png")).toString()}}};
        const auto native = studio.saveProject(p.toVariantMap());
        QVERIFY2(!native.isEmpty(), qPrintable(studio.error()));
        auto loaded = studio.loadProject(native);
        QVERIFY(!loaded.isEmpty());
        QVERIFY(loaded.value("layers").toList()[0].toMap().value("source").toString().startsWith("file:"));
        auto resaved = studio.saveProject(loaded); QVERIFY(!resaved.isEmpty());
        QVERIFY(QFile::remove(sources.filePath("photo.png")));
        QCOMPARE(classify(native).kind, Kind::Native);
        QCOMPARE(classify(imported).kind, Kind::Image);
        QVERIFY(!readNativeProject(native + "/wallpaper.json").isEmpty());
        const auto items = scanLibrary(d.path());
        QCOMPARE(std::count_if(items.begin(), items.end(), [](const auto &i) { return i.workshopId.isEmpty(); }), 3);
        QCOMPARE(installed.size(), 3);
    }
    void failedSaveDoesNotPublishPartialProject() {
        QTemporaryDir d; Settings settings; auto cfg = settings.data(); cfg.downloadDir = d.path(); settings.set(cfg);
        Studio studio(&settings);
        auto p = document(); p["layers"] = QJsonArray{QJsonObject{{"type", "image"}, {"source", "https://host/a.png"}}};
        QVERIFY(studio.saveProject(p.toVariantMap()).isEmpty());
        QVERIFY(!studio.error().isEmpty());
        QVERIFY(QDir(d.filePath("local")).entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden).isEmpty());
    }
    void rejectsInsecureCatalog() {
        Settings settings; Studio studio(&settings);
        studio.refreshCatalog("http://example.com/catalog.json");
        QVERIFY(!studio.busy()); QVERIFY(!studio.error().isEmpty()); QVERIFY(studio.entries().isEmpty());
    }
    void catalogInstallation_data() {
        QTest::addColumn<bool>("corrupt");
        QTest::newRow("valid") << false;
        QTest::newRow("hash-mismatch") << true;
    }
    void catalogInstallation() {
        QFETCH(bool, corrupt);
        QTemporaryDir d; Settings settings; auto cfg = settings.data(); cfg.downloadDir = d.path(); settings.set(cfg);
        MemoryNetwork network;
        const auto payload = QJsonDocument(document()).toJson();
        QJsonObject file{{"path", "wallpaper.json"}, {"url", "https://cdn.test/wallpaper.json"}, {"bytes", payload.size()},
                         {"sha256", QString::fromLatin1(QCryptographicHash::hash(payload, QCryptographicHash::Sha256).toHex())}};
        QJsonObject entry{{"id", "clock"}, {"title", "Clock"}, {"type", "native"}, {"files", QJsonArray{file}}};
        network.responses["/catalog.json"] = QJsonDocument(QJsonObject{{"version", 1}, {"wallpapers", QJsonArray{entry}}}).toJson();
        auto bad = payload; bad[0] = '[';
        network.responses["/wallpaper.json"] = corrupt ? bad : payload;
        Studio studio(&settings, nullptr, &network); QSignalSpy installed(&studio, &Studio::installed);
        studio.refreshCatalog("https://catalog.test/catalog.json");
        QTRY_VERIFY(!studio.busy());
        QVERIFY2(studio.error().isEmpty(), qPrintable(studio.error())); QCOMPARE(studio.entries().size(), 1);
        studio.install(0); QTRY_VERIFY(!studio.busy());
        if (corrupt) {
            QVERIFY(studio.error().contains("SHA-256")); QCOMPARE(installed.size(), 0);
            QVERIFY(QDir(d.filePath("local")).entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden).isEmpty());
        } else {
            QVERIFY2(studio.error().isEmpty(), qPrintable(studio.error())); QCOMPARE(installed.size(), 1);
            const auto path = installed[0][0].toString(); QCOMPARE(classify(path).kind, Kind::Native);
            QVERIFY(!readNativeProject(path + "/wallpaper.json").isEmpty());
        }
    }
    void onboardingDoesNotRequireSteam() {
        Settings settings; auto cfg = settings.data(); cfg.steamApiKey.clear(); cfg.onboardingCompleted = true; settings.set(cfg);
        QVERIFY(!settings.needsOnboarding());
        Settings reloaded; QVERIFY(!reloaded.needsOnboarding());
    }
};
QTEST_GUILESS_MAIN(TestNative)
#include "test_native.moc"
