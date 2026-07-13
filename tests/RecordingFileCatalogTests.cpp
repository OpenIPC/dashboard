#include <QtTest>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QUrl>

#include "RecordingFileCatalog.h"

class RecordingFileCatalogTests : public QObject
{
    Q_OBJECT

private slots:
    void sanitizesCameraTokens();
    void generatedPathRoundTripsThroughInspector();
    void acceptsLegacyIpTimestampNames();
    void rejectsUnsupportedOrForeignFiles();
};

void RecordingFileCatalogTests::sanitizesCameraTokens()
{
    QCOMPARE(RecordingFileCatalog::cameraToken(QStringLiteral("192.168.0.219")),
             QStringLiteral("192_168_0_219"));
    QCOMPARE(RecordingFileCatalog::cameraToken(QStringLiteral(" openipc-hi3516ev200.local ")),
             QStringLiteral("openipc-hi3516ev200_local"));
    QCOMPARE(RecordingFileCatalog::cameraToken(QStringLiteral("")),
             QStringLiteral("camera"));
}

void RecordingFileCatalogTests::generatedPathRoundTripsThroughInspector()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString cameraIp = QStringLiteral("192.168.0.219");
    const QDateTime timestamp(QDate(2026, 7, 11), QTime(23, 16, 18, 199));
    const QString path = RecordingFileCatalog::buildRecordingPath(
        dir.path(), cameraIp, timestamp, QStringLiteral("manual"));

    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write(QByteArray(4096, 'x')) > 0);
    file.close();

    const auto inspected = RecordingFileCatalog::inspectFile(QFileInfo(path), cameraIp);
    QVERIFY(inspected.has_value());
    QCOMPARE(inspected->startTime, timestamp);
    QCOMPARE(inspected->cameraToken, QStringLiteral("192_168_0_219"));
    QCOMPARE(inspected->source, QStringLiteral("manual"));
    QCOMPARE(inspected->filePath, QFileInfo(path).absoluteFilePath());
    QCOMPARE(inspected->fileUrl, QUrl::fromLocalFile(QFileInfo(path).absoluteFilePath()).toString());
    QCOMPARE(inspected->sizeBytes, qint64(4096));
}

void RecordingFileCatalogTests::acceptsLegacyIpTimestampNames()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = QDir(dir.path()).filePath(
        QStringLiteral("192_168_0_219_2026-07-11_23-16-18.mp4"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write(QByteArray(32, 'x')) > 0);
    file.close();

    const auto inspected = RecordingFileCatalog::inspectFile(QFileInfo(path), QStringLiteral("192.168.0.219"));
    QVERIFY(inspected.has_value());
    QCOMPARE(inspected->startTime, QDateTime(QDate(2026, 7, 11), QTime(23, 16, 18)));
    QCOMPARE(inspected->type, 0);
}

void RecordingFileCatalogTests::rejectsUnsupportedOrForeignFiles()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString supportedPath = QDir(dir.path()).filePath(
        QStringLiteral("192_168_0_219_2026-07-11_23-16-18.mp4"));
    QFile supported(supportedPath);
    QVERIFY(supported.open(QIODevice::WriteOnly));
    QVERIFY(supported.write(QByteArray(1, 'x')) > 0);
    supported.close();

    const QString unsupportedPath = QDir(dir.path()).filePath(
        QStringLiteral("192_168_0_219_2026-07-11_23-16-18.txt"));
    QFile unsupported(unsupportedPath);
    QVERIFY(unsupported.open(QIODevice::WriteOnly));
    QVERIFY(unsupported.write(QByteArray(1, 'x')) > 0);
    unsupported.close();

    QVERIFY(!RecordingFileCatalog::inspectFile(QFileInfo(supportedPath), QStringLiteral("10.0.0.5")).has_value());
    QVERIFY(!RecordingFileCatalog::inspectFile(QFileInfo(unsupportedPath), QStringLiteral("192.168.0.219")).has_value());
}

QTEST_APPLESS_MAIN(RecordingFileCatalogTests)

#include "RecordingFileCatalogTests.moc"
