#include "MajesticClient.h"

#include <QJsonDocument>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTest>

class MajesticClientTests : public QObject
{
    Q_OBJECT

private slots:
    void buildsMinimalNestedPatch();
    void masksSensitiveDiffValues();
    void flattensSchemaGroupsAndMetadata();
    void infersFieldsForLegacyConfig();
    void usesCurrentMajesticHttpContract();
};

void MajesticClientTests::buildsMinimalNestedPatch()
{
    const QVariantMap original{
        {QStringLiteral("video0"), QVariantMap{
             {QStringLiteral("codec"), QStringLiteral("h264")},
             {QStringLiteral("fps"), 25},
             {QStringLiteral("size"), QStringLiteral("1920x1080")}}},
        {QStringLiteral("audio"), QVariantMap{{QStringLiteral("enabled"), true}}}
    };
    QVariantMap edited = original;
    QVariantMap video = edited.value(QStringLiteral("video0")).toMap();
    video.insert(QStringLiteral("fps"), 30);
    edited.insert(QStringLiteral("video0"), video);

    const QVariantMap patch = MajesticClient::buildPatchForTest(original, edited);
    QCOMPARE(patch.size(), 1);
    QCOMPARE(patch.value(QStringLiteral("video0")).toMap(),
             QVariantMap({{QStringLiteral("fps"), 30}}));
}

void MajesticClientTests::masksSensitiveDiffValues()
{
    const QVariantMap original{
        {QStringLiteral("netip"), QVariantMap{
             {QStringLiteral("password"), QVariantMap{{QStringLiteral("plain"), QStringLiteral("old")}}}}}
    };
    const QVariantMap edited{
        {QStringLiteral("netip"), QVariantMap{
             {QStringLiteral("password"), QVariantMap{{QStringLiteral("plain"), QStringLiteral("new")}}}}}
    };

    const QVariantList changes = MajesticClient::describeChangesForTest(original, edited);
    QCOMPARE(changes.size(), 1);
    const QVariantMap change = changes.constFirst().toMap();
    QCOMPARE(change.value(QStringLiteral("path")).toString(), QStringLiteral("netip.password.plain"));
    QVERIFY(change.value(QStringLiteral("sensitive")).toBool());
    QVERIFY(!change.value(QStringLiteral("before")).toString().contains(QStringLiteral("old")));
    QVERIFY(!change.value(QStringLiteral("after")).toString().contains(QStringLiteral("new")));
}

void MajesticClientTests::flattensSchemaGroupsAndMetadata()
{
    const QVariantMap fpsSchema{
        {QStringLiteral("type"), QStringLiteral("integer")},
        {QStringLiteral("title"), QStringLiteral("Frame rate")},
        {QStringLiteral("minimum"), 1},
        {QStringLiteral("maximum"), 60},
        {QStringLiteral("default"), 25}
    };
    const QVariantMap sectionSchema{
        {QStringLiteral("type"), QStringLiteral("object")},
        {QStringLiteral("properties"), QVariantMap{{QStringLiteral("fps"), fpsSchema}}}
    };
    const QVariantMap schema{
        {QStringLiteral("properties"), QVariantMap{{QStringLiteral("video0"), sectionSchema}}},
        {QStringLiteral("x-groups"), QVariantList{QVariantMap{
             {QStringLiteral("id"), QStringLiteral("video")},
             {QStringLiteral("label"), QStringLiteral("Video")},
             {QStringLiteral("sections"), QVariantList{QStringLiteral("video0")}}
         }}}
    };
    const QVariantMap config{{QStringLiteral("video0"), QVariantMap{{QStringLiteral("fps"), 30}}}};

    const QVariantList fields = MajesticClient::flattenFieldsForTest(schema, config);
    QCOMPARE(fields.size(), 1);
    const QVariantMap field = fields.constFirst().toMap();
    QCOMPARE(field.value(QStringLiteral("path")).toString(), QStringLiteral("video0.fps"));
    QCOMPARE(field.value(QStringLiteral("groupId")).toString(), QStringLiteral("video"));
    QCOMPARE(field.value(QStringLiteral("groupLabel")).toString(), QStringLiteral("Video"));
    QCOMPARE(field.value(QStringLiteral("value")).toInt(), 30);
    QCOMPARE(field.value(QStringLiteral("minimum")).toInt(), 1);
    QVERIFY(field.value(QStringLiteral("hasDefault")).toBool());
}

void MajesticClientTests::infersFieldsForLegacyConfig()
{
    const QVariantMap config{
        {QStringLiteral("video0"), QVariantMap{
             {QStringLiteral("enabled"), true},
             {QStringLiteral("fps"), 25},
             {QStringLiteral("size"), QStringLiteral("1920x1080")}}}
    };

    const QVariantList fields = MajesticClient::flattenFieldsForTest({}, config);
    QCOMPARE(fields.size(), 3);
    QStringList paths;
    for (const QVariant &field : fields) {
        paths.append(field.toMap().value(QStringLiteral("path")).toString());
    }
    QVERIFY(paths.contains(QStringLiteral("video0.enabled")));
    QVERIFY(paths.contains(QStringLiteral("video0.fps")));
    QVERIFY(paths.contains(QStringLiteral("video0.size")));
}

void MajesticClientTests::usesCurrentMajesticHttpContract()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));

    QList<QByteArray> requests;
    connect(&server, &QTcpServer::newConnection, &server, [&server, &requests]() {
        while (server.hasPendingConnections()) {
            QTcpSocket *socket = server.nextPendingConnection();
            QObject::connect(socket, &QTcpSocket::readyRead, socket, [socket, &requests]() {
                QByteArray request = socket->property("requestBuffer").toByteArray();
                request += socket->readAll();
                const qsizetype headerEnd = request.indexOf("\r\n\r\n");
                if (headerEnd < 0) {
                    socket->setProperty("requestBuffer", request);
                    return;
                }
                qsizetype contentLength = 0;
                const QList<QByteArray> headerLines = request.left(headerEnd).split('\n');
                for (const QByteArray &line : headerLines) {
                    if (line.toLower().startsWith("content-length:")) {
                        contentLength = line.mid(line.indexOf(':') + 1).trimmed().toLongLong();
                    }
                }
                if (request.size() < headerEnd + 4 + contentLength) {
                    socket->setProperty("requestBuffer", request);
                    return;
                }
                requests.append(request);
                const QByteArray firstLine = request.left(request.indexOf("\r\n"));
                QByteArray body;
                if (firstLine.startsWith("GET /api/v1/config.json ")) {
                    body = R"({"video0":{"fps":25,"codec":"h264"}})";
                } else if (firstLine.startsWith("GET /api/v1/config.schema.json ")) {
                    body = R"({"properties":{"video0":{"type":"object","properties":{"fps":{"type":"integer","minimum":1,"maximum":60},"codec":{"type":"string"}}}},"x-groups":[{"id":"video","label":"Video","sections":["video0"]}]})";
                } else if (firstLine.startsWith("POST /api/v1/config ")) {
                    body = R"({"ok":true})";
                } else {
                    body = R"({"error":"not found"})";
                }
                const QByteArray status = firstLine.contains("/api/v1/")
                    ? QByteArrayLiteral("HTTP/1.1 200 OK\r\n")
                    : QByteArrayLiteral("HTTP/1.1 404 Not Found\r\n");
                socket->write(status
                              + QByteArrayLiteral("Content-Type: application/json\r\nContent-Length: ")
                              + QByteArray::number(body.size())
                              + QByteArrayLiteral("\r\nConnection: close\r\n\r\n") + body);
                socket->disconnectFromHost();
            });
        }
    });

    MajesticClient client;
    QSignalSpy loaded(&client, &MajesticClient::configurationLoaded);
    const QString requestId = client.loadConfiguration(
        QStringLiteral("127.0.0.1"), server.serverPort(),
        QStringLiteral("root"), QStringLiteral("secret"));
    QTRY_COMPARE_WITH_TIMEOUT(loaded.count(), 1, 5000);

    const QList<QVariant> loadedArguments = loaded.takeFirst();
    QCOMPARE(loadedArguments.at(0).toString(), requestId);
    QCOMPARE(loadedArguments.at(1).toMap().value(QStringLiteral("video0")).toMap()
                 .value(QStringLiteral("fps")).toInt(), 25);
    QCOMPARE(loadedArguments.at(3).toList().size(), 2);
    QVERIFY(loadedArguments.at(4).toMap().value(QStringLiteral("configWrite")).toBool());
    QCOMPARE(requests.size(), 2);
    const QByteArray expectedAuth = QByteArrayLiteral("Authorization: Basic ")
        + QByteArray("root:secret").toBase64();
    QVERIFY(requests.at(0).contains(expectedAuth));
    QVERIFY(requests.at(1).contains(expectedAuth));

    QSignalSpy failed(&client, &MajesticClient::operationFailed);
    const QVariantMap invalidPatch{{QStringLiteral("video0"),
                                    QVariantMap{{QStringLiteral("fps"), 100}}}};
    const QString invalidApplyId = client.applyConfiguration(
        QStringLiteral("127.0.0.1"), server.serverPort(),
        QStringLiteral("root"), QStringLiteral("secret"), invalidPatch);
    QCOMPARE(failed.count(), 0);
    QTRY_COMPARE_WITH_TIMEOUT(failed.count(), 1, 1000);
    QCOMPARE(failed.takeFirst().at(0).toString(), invalidApplyId);
    QCOMPARE(requests.size(), 2);

    QSignalSpy applied(&client, &MajesticClient::configurationApplied);
    const QVariantMap patch{{QStringLiteral("video0"),
                             QVariantMap{{QStringLiteral("fps"), 30}}}};
    client.applyConfiguration(QStringLiteral("127.0.0.1"), server.serverPort(),
                              QStringLiteral("root"), QStringLiteral("secret"), patch);
    QTRY_COMPARE_WITH_TIMEOUT(applied.count(), 1, 5000);
    QCOMPARE(requests.size(), 3);
    QVERIFY(requests.constLast().startsWith("POST /api/v1/config HTTP/1.1\r\n"));
    QVERIFY(requests.constLast().contains("\r\nContent-Type: application/json\r\n"));
    QVERIFY(requests.constLast().endsWith("{\"video0\":{\"fps\":30}}"));
}

QTEST_MAIN(MajesticClientTests)
#include "MajesticClientTests.moc"
