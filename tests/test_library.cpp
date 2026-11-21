// SPDX-License-Identifier: GPL-3.0-or-later
#include "library.h"

#include <QtTest>

using namespace wpe;

class TestLibrary : public QObject {
    Q_OBJECT
    static void touch(const QString &p, const QByteArray &data = "x")
    {
        QDir().mkpath(QFileInfo(p).absolutePath());
        QFile f(p);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(data);
    }

private slots:
    void classifiesFilesByExtension()
    {
        QTemporaryDir d;
        touch(d.filePath("a.mp4"));
        touch(d.filePath("b.PNG"));
        QCOMPARE(classify(d.filePath("a.mp4")).kind, Kind::Video);
        QCOMPARE(classify(d.filePath("b.PNG")).kind, Kind::Image);
    }

    void sceneWithPkgIsScene()
    {
        QTemporaryDir d;
        touch(d.filePath("w/scene.pkg"));
        touch(d.filePath("w/project.json"), R"({"type":"Scene","title":"T"})");
        const Classified c = classify(d.filePath("w"));
        QCOMPARE(c.kind, Kind::Scene);
        QCOMPARE(c.mainPath, QDir(d.filePath("w")).absolutePath());
    }

    // The Rust version special-cased this: renderer crashes on "scene" folders that only hold a video.
    void sceneWithoutPkgButWithVideoIsVideo()
    {
        QTemporaryDir d;
        touch(d.filePath("w/loop.mp4"));
        touch(d.filePath("w/project.json"), R"({"type":"Scene"})");
        const Classified c = classify(d.filePath("w"));
        QCOMPARE(c.kind, Kind::Video);
        QVERIFY(c.mainPath.endsWith("loop.mp4"));
    }

    void videoProjectUsesDeclaredFile()
    {
        QTemporaryDir d;
        touch(d.filePath("w/a.mp4"));
        touch(d.filePath("w/main.webm"));
        touch(d.filePath("w/project.json"), R"({"type":"Video","file":"main.webm"})");
        QVERIFY(classify(d.filePath("w")).mainPath.endsWith("main.webm"));
    }

    void webProject()
    {
        QTemporaryDir d;
        touch(d.filePath("w/index.html"));
        touch(d.filePath("w/project.json"), R"({"type":"Web"})");
        QCOMPARE(classify(d.filePath("w")).kind, Kind::Web);
    }

    void scanFindsWorkshopItemsAndLooseFiles()
    {
        QTemporaryDir d;
        const QString root = d.filePath("steamapps/workshop/content/431960");
        touch(root + "/111/project.json", R"({"type":"Video","title":"Rain","file":"v.mp4","preview":"p.jpg"})");
        touch(root + "/111/v.mp4");
        touch(root + "/111/p.jpg");
        touch(d.filePath("loose.png"));
        const auto items = scanLibrary(d.path());
        QVERIFY(items.size() >= 2);
        bool rain = false, loose = false;
        for (const auto &it : items) {
            if (it.title == "Rain") { rain = true; QCOMPARE(it.workshopId, QString("111")); QCOMPARE(it.contentType, QString("video")); QVERIFY(it.previewPath.endsWith("p.jpg")); }
            if (it.title == "loose") { loose = true; QVERIFY(it.workshopId.isEmpty()); }
        }
        QVERIFY(rain && loose);
    }
};

QTEST_APPLESS_MAIN(TestLibrary)
#include "test_library.moc"
