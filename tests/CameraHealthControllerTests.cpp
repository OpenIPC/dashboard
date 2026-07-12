#include <QtTest>

#include "CameraHealthController.h"
#include "CameraModel.h"

#include <QJsonArray>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>

class CameraHealthControllerTests : public QObject
{
    Q_OBJECT

private slots:
    void quickProfileCompletesAndProducesReport();
    void openIpcProfileExtractsTemperature();
    void refreshesSidebarTelemetryWithoutCreatingHistory();
    void restoredHistoryIsValidatedAndCapped();
};

void CameraHealthControllerTests::quickProfileCompletesAndProducesReport()
{
    QTcpServer rtspServer;
    QVERIFY(rtspServer.listen(QHostAddress::LocalHost));
    connect(&rtspServer, &QTcpServer::newConnection, &rtspServer, [&rtspServer]() {
        QTcpSocket *socket = rtspServer.nextPendingConnection();
        QObject::connect(socket, &QTcpSocket::readyRead, socket, [socket]() {
            socket->readAll();
            socket->write("RTSP/1.0 200 OK\r\nCSeq: 1\r\n\r\n");
            socket->flush();
            socket->disconnectFromHost();
        });
    });

    QTcpServer httpServer;
    QVERIFY(httpServer.listen(QHostAddress::LocalHost));
    connect(&httpServer, &QTcpServer::newConnection, &httpServer, [&httpServer]() {
        QTcpSocket *socket = httpServer.nextPendingConnection();
        QObject::connect(socket, &QTcpSocket::readyRead, socket, [socket]() {
            socket->readAll();
            socket->write("HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\n"
                          "Content-Length: 4\r\nConnection: close\r\n\r\nJPEG");
            socket->flush();
            socket->disconnectFromHost();
        });
    });

    CameraModel cameras;
    CameraModel grid;
    Camera camera;
    camera.id = QStringLiteral("camera-1");
    camera.name = QStringLiteral("Test camera");
    camera.ip = QStringLiteral("127.0.0.1");
    camera.port = static_cast<int>(rtspServer.serverPort());
    camera.onvifPort = static_cast<int>(httpServer.serverPort());
    camera.streamUrl = QStringLiteral("rtsp://127.0.0.1:%1/stream=0")
                           .arg(rtspServer.serverPort());
    camera.hdStreamUrl = camera.streamUrl;
    cameras.addCamera(camera);
    grid.addCamera(camera);

    CameraHealthController controller(&cameras, &grid);
    QSignalSpy completed(&controller, &CameraHealthController::runCompleted);
    QVERIFY(controller.runAll(QStringLiteral("quick")));
    QVERIFY(completed.wait(7000));
    QVERIFY(!controller.running());
    QCOMPARE(controller.history().size(), 1);

    const QVariantMap run = controller.latestRun();
    QCOMPARE(run.value(QStringLiteral("healthyCount")).toInt(), 1);
    const QVariantMap result = controller.resultForCamera(camera.ip);
    QCOMPARE(result.value(QStringLiteral("status")).toString(), QStringLiteral("ok"));
    QCOMPARE(result.value(QStringLiteral("probes")).toList().size(), 2);

    const QString report = controller.reportText();
    QVERIFY(report.contains(QStringLiteral("Test camera")));
    QVERIFY(report.contains(QStringLiteral("RTSP endpoint responded")));
    QVERIFY(!report.contains(QStringLiteral("password")));
}

void CameraHealthControllerTests::openIpcProfileExtractsTemperature()
{
    QTcpServer httpServer;
    QVERIFY(httpServer.listen(QHostAddress::LocalHost));
    connect(&httpServer, &QTcpServer::newConnection, &httpServer, [&httpServer]() {
        while (httpServer.hasPendingConnections()) {
            QTcpSocket *socket = httpServer.nextPendingConnection();
            QObject::connect(socket, &QTcpSocket::readyRead, socket, [socket]() {
                QByteArray request = socket->property("request").toByteArray();
                request += socket->readAll();
                socket->setProperty("request", request);
                if (!request.contains("\r\n\r\n")) return;

                const QByteArray target = request.split(' ').value(1);
                QByteArray status = "HTTP/1.1 200 OK\r\n";
                QByteArray contentType = "Content-Type: text/plain\r\n";
                QByteArray body = "ok";
                if (target.startsWith("/api/v1/config.json")) {
                    contentType = "Content-Type: application/json\r\n";
                    body = R"({"version":"2026.07","video0":{"enabled":true}})";
                } else if (target.startsWith("/api/v1/config.schema.json")) {
                    contentType = "Content-Type: application/json\r\n";
                    body = R"({"type":"object"})";
                } else if (target.startsWith("/cgi-bin/status.cgi")) {
                    body = "Firmware: 2.5.0";
                } else if (target.startsWith("/metrics")) {
                    body = "node_hwmon_temp_celsius 58.5\nnode_time_seconds 100\n";
                } else if (target.startsWith("/ws/logs")) {
                    status = "HTTP/1.1 426 Upgrade Required\r\n";
                } else if (target.startsWith("/cgi-bin/j/run.cgi")) {
                    body = "majestic started";
                } else if (target.startsWith("/image.jpg")) {
                    contentType = "Content-Type: image/jpeg\r\n";
                    body = "JPEG";
                }

                socket->write(status + contentType
                              + "Content-Length: " + QByteArray::number(body.size())
                              + "\r\nConnection: close\r\n\r\n" + body);
                socket->flush();
                socket->disconnectFromHost();
            });
        }
    });

    CameraModel cameras;
    CameraModel grid;
    Camera camera;
    camera.id = QStringLiteral("camera-openipc");
    camera.name = QStringLiteral("OpenIPC camera");
    camera.ip = QStringLiteral("127.0.0.1");
    camera.onvifPort = static_cast<int>(httpServer.serverPort());
    cameras.addCamera(camera);

    CameraHealthController controller(&cameras, &grid);
    QSignalSpy completed(&controller, &CameraHealthController::runCompleted);
    QVERIFY(controller.runAll(QStringLiteral("openipc")));
    QVERIFY(completed.wait(10000));

    const QVariantMap result = controller.resultForCamera(camera.ip);
    QCOMPARE(result.value(QStringLiteral("temperatureC")).toDouble(), 58.5);
    QCOMPARE(result.value(QStringLiteral("majesticVersion")).toString(),
             QStringLiteral("2026.07"));
    QCOMPARE(result.value(QStringLiteral("firmwareVersion")).toString(),
             QStringLiteral("2.5.0"));
    QVERIFY(controller.reportText().contains(QStringLiteral("majestic started")));
    QVERIFY(controller.reportText().contains(QStringLiteral("SoC temperature: 58.5 C")));
}

void CameraHealthControllerTests::refreshesSidebarTelemetryWithoutCreatingHistory()
{
    QTcpServer httpServer;
    QVERIFY(httpServer.listen(QHostAddress::LocalHost));
    connect(&httpServer, &QTcpServer::newConnection, &httpServer, [&httpServer]() {
        QTcpSocket *socket = httpServer.nextPendingConnection();
        QObject::connect(socket, &QTcpSocket::readyRead, socket, [socket]() {
            socket->readAll();
            const QByteArray body = "node_hwmon_temp_celsius 61.5226\n";
            socket->write("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n"
                          "Content-Length: " + QByteArray::number(body.size())
                          + "\r\nConnection: close\r\n\r\n" + body);
            socket->flush();
            socket->disconnectFromHost();
        });
    });

    CameraModel cameras;
    CameraModel grid;
    Camera camera;
    camera.id = QStringLiteral("sidebar-camera");
    camera.name = QStringLiteral("Sidebar camera");
    camera.ip = QStringLiteral("127.0.0.1");
    camera.onvifPort = static_cast<int>(httpServer.serverPort());
    cameras.addCamera(camera);

    CameraHealthController controller(&cameras, &grid);
    QSignalSpy updated(&controller, &CameraHealthController::telemetryUpdated);
    QVERIFY(controller.refreshCameraTelemetry(camera.ip));
    QVERIFY(updated.wait(7000));
    QCOMPARE(controller.resultForCamera(camera.ip)
                 .value(QStringLiteral("temperatureC")).toDouble(),
             61.5226);
    QVERIFY(controller.history().isEmpty());
    QVERIFY(!controller.refreshCameraTelemetry(camera.ip));
}

void CameraHealthControllerTests::restoredHistoryIsValidatedAndCapped()
{
    CameraModel cameras;
    CameraModel grid;
    CameraHealthController controller(&cameras, &grid);

    QJsonArray history;
    history.append(QJsonObject{{QStringLiteral("summary"), QStringLiteral("invalid")}});
    for (int index = 0; index < 35; ++index) {
        history.append(QJsonObject{
            {QStringLiteral("id"), QStringLiteral("run-%1").arg(index)},
            {QStringLiteral("profile"), QStringLiteral("quick")},
            {QStringLiteral("cameras"), QJsonArray{}}
        });
    }

    controller.restoreHistory(history);
    QCOMPARE(controller.history().size(), 30);
    QCOMPARE(controller.historyJson().size(), 30);
    QCOMPARE(controller.runById(QStringLiteral("run-0"))
                 .value(QStringLiteral("profile")).toString(),
             QStringLiteral("quick"));

    controller.clearHistory();
    QVERIFY(controller.history().isEmpty());
    QVERIFY(!controller.runAll(QStringLiteral("quick")));
}

QTEST_GUILESS_MAIN(CameraHealthControllerTests)

#include "CameraHealthControllerTests.moc"
