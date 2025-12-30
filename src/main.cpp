#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQml>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QTimer>
#include "backend/SystemController.h"
#include "backend/MdkPlayer.h"
#include "backend/AnalyticsModel.h"
#include "backend/analytics/AnalyticsEngine.h"
#include "backend/SshClient.h"
#include <functional>

namespace {
QFile gLogFile;
std::function<void(QtMsgType, const QString&)> gLogCallback;

void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Filter out annoying warnings
    if (msg.contains("No QSGTexture provided from updateSampledImage")) {
        return;
    }

    const char *level = "LOG";
    switch (type) {
    case QtDebugMsg: level = "DBG"; break;
    case QtInfoMsg: level = "INF"; break;
    case QtWarningMsg: level = "WRN"; break;
    case QtCriticalMsg: level = "CRT"; break;
    case QtFatalMsg: level = "FTL"; break;
    }
    
    QString formattedMsg = QString("%1 [%2] %3").arg(QDateTime::currentDateTime().toString(Qt::ISODate), level, msg);

    // Write to file
    if (gLogFile.isOpen()) {
        QTextStream ts(&gLogFile);
        ts << formattedMsg << '\n';
        ts.flush();
    }
    
    // Write to stderr so it shows up in the terminal
    fprintf(stderr, "%s\n", qPrintable(formattedMsg));
    fflush(stderr);

    if (gLogCallback) {
        gLogCallback(type, msg);
    }

    if (type == QtFatalMsg)
        abort();
    Q_UNUSED(context);
}
} // namespace

int main(int argc, char *argv[])
{
    // Check for ASKPASS mode
    if (qEnvironmentVariableIsSet("OPENIPC_ASKPASS_MODE")) {
        QTextStream out(stdout);
        out << qEnvironmentVariable("SSH_PASS") << Qt::endl;
        return 0;
    }

    QGuiApplication app(argc, argv); // Create app first to get standard paths
    app.setOrganizationName("OpenIPC");
    app.setApplicationName("Dashboard");

    QString logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(logPath);
    gLogFile.setFileName(logPath + "/app.log");
    
    if (gLogFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream ts(&gLogFile);
        ts << QDateTime::currentDateTime().toString(Qt::ISODate) << " [INF] app start" << '\n';
        ts.flush();
        qInstallMessageHandler(logMessageHandler);
    } else {
        // Fallback to temp if AppData fails
        gLogFile.setFileName(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/appOpenIPC-Dashboard.log");
        if (gLogFile.open(QIODevice::Append | QIODevice::Text)) {
             qInstallMessageHandler(logMessageHandler);
        }
    }

    // Prefer OpenGL for MDK. Force RHI to OpenGL regardless of external env, so QQuickFramebufferObject works.
    qunsetenv("QT_QUICK_BACKEND");
    qputenv("QSG_RHI_BACKEND", "opengl");
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLRhi);
    
    // Force Software rendering to avoid D3D11/OpenGL issues in deployed environment
    // QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);

    // QGuiApplication app(argc, argv); // Already created above

    qInfo().noquote() << "Platform" << QGuiApplication::platformName()
                      << "QSG_RHI_BACKEND=" << qEnvironmentVariable("QSG_RHI_BACKEND")
                      << "QT_QUICK_BACKEND=" << qEnvironmentVariable("QT_QUICK_BACKEND")
                      << "graphicsApi=" << QQuickWindow::graphicsApi();
    qInfo() << "Log file path:" << gLogFile.fileName();

    QQmlApplicationEngine engine;

    // Register MdkPlayer
    qmlRegisterType<MdkPlayer>("OpenIPC", 1, 0, "MdkPlayer");
    qmlRegisterType<AnalyticsModel>("OpenIPC", 1, 0, "AnalyticsModel");
    qmlRegisterType<AnalyticsEngine>("OpenIPC", 1, 0, "AnalyticsEngine");
    qmlRegisterType<SshClient>("OpenIPC", 1, 0, "SshClient");

    // Register the C++ backend controller
    SystemController systemController;    
    // Hook up logging to SystemController
    gLogCallback = [&](QtMsgType type, const QString &msg) {
        systemController.addLog(type, msg);
    };
    // engine.rootContext()->setContextProperty("systemController", &systemController);
    qmlRegisterSingletonInstance("OpenIPC", 1, 0, "SystemController", &systemController);

    const QUrl url(u"qrc:/OpenIPC/src/ui/Main.qml"_qs);
    // const QUrl url(u"qrc:/OpenIPC/src/ui/Test.qml"_qs);
    qInfo() << "engine.load start" << url;
    engine.load(url);
    qInfo() << "engine.load finished";

    qInfo() << "engine.load done, rootObjects=" << engine.rootObjects().size();

    qInfo() << "engine.load done, rootObjects=" << engine.rootObjects().size();
    if (!engine.rootObjects().isEmpty()) {
        if (auto windowObj = qobject_cast<QWindow*>(engine.rootObjects().constFirst())) {
            qInfo() << "post-load window visible=" << windowObj->isVisible()
                    << "geometry=" << windowObj->geometry()
                    << "flags=" << Qt::hex << static_cast<quint64>(windowObj->flags()) << Qt::dec
                    << "handle=" << windowObj->winId();
        } else {
            qWarning() << "post-load root object is not a QWindow";
        }
    }

    QTimer::singleShot(200, [&engine]() {
        qInfo() << "singleShot root objects=" << engine.rootObjects().size();
        if (!engine.rootObjects().isEmpty()) {
            if (auto windowObj = qobject_cast<QWindow*>(engine.rootObjects().constFirst())) {
                windowObj->showMaximized();
                windowObj->requestActivate();
                qInfo() << "singleShot window visible=" << windowObj->isVisible()
                        << "geometry=" << windowObj->geometry()
                        << "flags=" << Qt::hex << static_cast<quint64>(windowObj->flags()) << Qt::dec
                        << "handle=" << windowObj->winId();
            } else {
                qWarning() << "singleShot: root object is not a QWindow";
            }
        }
    });

    // Runtime check: if we still ended up on software or a non-OpenGL backend, warn loudly.
    auto api = QQuickWindow::graphicsApi();
    if (api != QSGRendererInterface::OpenGL && api != QSGRendererInterface::OpenGLRhi) {
        qWarning() << "Scene graph backend is not OpenGL (" << api << "). MDK video will not render."
                   << "Ensure OpenGL/ANGLE is available or place opengl32sw.dll next to the executable.";
    }

    return app.exec();
}
