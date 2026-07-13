#include <QtTest>

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUrl>

#include "ArchiveController.h"

class ArchiveControllerTests : public QObject
{
    Q_OBJECT

private slots:
    void searchReturnsRealPathsAndNewestFirst();
};

namespace {

QString createRecording(const QString &root, const QString &fileName, int bytes)
{
    const QString path = QDir(root).filePath(fileName);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return {};
    }
    if (file.write(QByteArray(bytes, 'x')) != bytes) {
        return {};
    }
    file.close();
    return QFileInfo(path).absoluteFilePath();
}

} // namespace

void ArchiveControllerTests::searchReturnsRealPathsAndNewestFirst()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString cameraIp = QStringLiteral("192.168.0.219");
    const QString older = createRecording(
        dir.path(), QStringLiteral("192_168_0_219_2026-07-11_10-00-00_manual.mp4"), 10);
    const QString newer = createRecording(
        dir.path(), QStringLiteral("192_168_0_219_2026-07-11_11-00-00_manual.mp4"), 20);
    createRecording(dir.path(), QStringLiteral("10_0_0_5_2026-07-11_12-00-00_manual.mp4"), 30);

    QVERIFY(!older.isEmpty());
    QVERIFY(!newer.isEmpty());

    ArchiveController controller;
    QSignalSpy searchFinished(&controller, &ArchiveController::searchFinished);

    controller.search(QDateTime(QDate(2026, 7, 11), QTime(0, 0, 0)),
                      QDateTime(QDate(2026, 7, 11), QTime(23, 59, 59)),
                      cameraIp,
                      dir.path());

    QVERIFY(searchFinished.wait(3000));
    QCOMPARE(searchFinished.takeFirst().at(0).toInt(), 2);

    const QVariantList results = controller.searchResults();
    QCOMPARE(results.size(), 2);

    const QVariantMap first = results.at(0).toMap();
    const QVariantMap second = results.at(1).toMap();

    QCOMPARE(first.value(QStringLiteral("filePath")).toString(), newer);
    QCOMPARE(first.value(QStringLiteral("fileUrl")).toString(), QUrl::fromLocalFile(newer).toString());
    QCOMPARE(first.value(QStringLiteral("sizeBytes")).toLongLong(), qint64(20));
    QCOMPARE(first.value(QStringLiteral("source")).toString(), QStringLiteral("manual"));
    QCOMPARE(second.value(QStringLiteral("filePath")).toString(), older);
}

QTEST_GUILESS_MAIN(ArchiveControllerTests)

#include "ArchiveControllerTests.moc"
