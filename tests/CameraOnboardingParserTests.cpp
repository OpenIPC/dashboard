#include <QtTest>

#include "CameraOnboardingParser.h"

class CameraOnboardingParserTests : public QObject
{
    Q_OBJECT

private slots:
    void parsesOpenIpcPayloadWithoutEmbeddingCredentialsInRtspUrl();
    void rejectsUnsupportedPayload();
};

void CameraOnboardingParserTests::parsesOpenIpcPayloadWithoutEmbeddingCredentialsInRtspUrl()
{
    const QVariantMap camera = CameraOnboardingParser::parse(QStringLiteral(
        "openipc://root:s3cret@192.168.1.10:554/stream=0?name=Gate&onvifPort=80"));
    QVERIFY(camera.value(QStringLiteral("valid")).toBool());
    QCOMPARE(camera.value(QStringLiteral("name")).toString(), QStringLiteral("Gate"));
    QCOMPARE(camera.value(QStringLiteral("login")).toString(), QStringLiteral("root"));
    QCOMPARE(camera.value(QStringLiteral("password")).toString(), QStringLiteral("s3cret"));
    QCOMPARE(camera.value(QStringLiteral("hdStreamUrl")).toString(),
             QStringLiteral("rtsp://192.168.1.10:554/stream=0"));
    QCOMPARE(camera.value(QStringLiteral("sdStreamUrl")).toString(),
             QStringLiteral("rtsp://192.168.1.10:554/stream=1"));
}

void CameraOnboardingParserTests::rejectsUnsupportedPayload()
{
    QVERIFY(!CameraOnboardingParser::parse(QStringLiteral("https://example.com"))
                 .value(QStringLiteral("valid")).toBool());
}

QTEST_APPLESS_MAIN(CameraOnboardingParserTests)

#include "CameraOnboardingParserTests.moc"
