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
#include "backend/gst/GstPlayer.h"
#include "backend/AnalyticsModel.h"
#include "backend/analytics/AnalyticsEngine.h"
#include "backend/SshClient.h"
#include "backend/RemoteFsModel.h"
#include <functional>

// Hardcoded GStreamer paths for Windows environment
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {
QFile gLogFile;
std::function<void(QtMsgType, const QString&)> gLogCallback;

void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Filter out annoying warnings
    if (msg.contains("No QSGTexture provided from updateSampledImage")) {
        return;
    }

    // Disable excessive logging to prevent memory buffer growth
    // if (type == QtDebugMsg || type == QtInfoMsg) return;

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
#ifdef Q_OS_WIN
    // Force GStreamer paths to known installation if local bundle is missing
    // Detailed handling for development environment where plugins are in C:\Program Files\...
    qputenv("GST_DEBUG", "2"); // Enable warning logs
    
    // Explicitly set the plugin path to the standard MinGW 64-bit GStreamer install
    // This fixes the issue where plugins are not found despite being installed
    const char* winGstPlugins = "C:\\Program Files\\gstreamer\\1.0\\mingw_x86_64\\lib\\gstreamer-1.0";
    qputenv("GST_PLUGIN_PATH", winGstPlugins);
    qputenv("GST_PLUGIN_SYSTEM_PATH", winGstPlugins);
    
    // Also ensure bin is in PATH for DLL resolution
    const char* winGstBin = "C:\\Program Files\\gstreamer\\1.0\\mingw_x86_64\\bin";
    QString currentPath = qEnvironmentVariable("PATH");
    if (!currentPath.contains("mingw_x86_64\\bin")) {
         qputenv("PATH", (QString(winGstBin) + ";" + currentPath).toLocal8Bit());
    }
#endif

    // Check for ASKPASS mode
    if (qEnvironmentVariableIsSet("OPENIPC_ASKPASS_MODE")) {
        QTextStream out(stdout);
        out << qEnvironmentVariable("SSH_PASS") << Qt::endl;
        return 0;
    }

    // Force Software rendering to avoid D3D11/OpenGL issues
    // This MUST be done before QGuiApplication is created
    // qputenv("QSG_RHI_BACKEND", "software");
    // QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);

    QGuiApplication app(argc, argv); 
    app.setOrganizationName("OpenIPC");
    app.setApplicationName("Dashboard");

    // Configure GStreamer paths for standalone deployment
    // This allows the app to find plugins in ./lib/gstreamer-1.0 relative to executable
    QString appDir = QCoreApplication::applicationDirPath();
    QString localGstPlugins = appDir + "/lib/gstreamer-1.0";
    if (QDir(localGstPlugins).exists()) {
        qputenv("GST_PLUGIN_PATH", localGstPlugins.toLocal8Bit());
        qputenv("GST_PLUGIN_PATH_1_0", localGstPlugins.toLocal8Bit());
        qputenv("GST_PLUGIN_SYSTEM_PATH", ""); // Ignore system install if local exists
        qputenv("GST_PLUGIN_SYSTEM_PATH_1_0", "");
        qputenv("PATH", (appDir + ";" + qEnvironmentVariable("PATH")).toLocal8Bit());

        QString localScanner = localGstPlugins + "/gst-plugin-scanner.exe";
        if (QFile::exists(localScanner)) {
            qputenv("GST_PLUGIN_SCANNER", localScanner.toLocal8Bit());
        }
    }


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
        // gLogFile.setFileName(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/appOpenIPC-Dashboard.log");
        // if (gLogFile.open(QIODevice::Append | QIODevice::Text)) {
        //      qInstallMessageHandler(logMessageHandler);
        // }
        // Simplify fallback logging to stderr if file fails
        fprintf(stderr, "Failed to open log file, logging to stderr only.\n");
    }

    qInfo().noquote() << "Platform" << QGuiApplication::platformName()
                      << "QSG_RHI_BACKEND=" << qEnvironmentVariable("QSG_RHI_BACKEND")
                      << "QT_QUICK_BACKEND=" << qEnvironmentVariable("QT_QUICK_BACKEND")
                      << "graphicsApi=" << QQuickWindow::graphicsApi();
    qInfo() << "Log file path:" << gLogFile.fileName();

    // Register the C++ backend controller FIRST to ensure it outlives the engine
    SystemController systemController;    
    // Hook up logging to SystemController
    gLogCallback = [&](QtMsgType type, const QString &msg) {
        systemController.addLog(type, msg);
    };

    QQmlApplicationEngine engine;

    // Register GStreamer Player
    qmlRegisterType<GstPlayer>("OpenIPC", 1, 0, "VideoPlayer");
    qmlRegisterType<AnalyticsModel>("OpenIPC", 1, 0, "AnalyticsModel");
    qmlRegisterType<AnalyticsEngine>("OpenIPC", 1, 0, "AnalyticsEngine");
    qmlRegisterType<SshClient>("OpenIPC", 1, 0, "SshClient");
    qmlRegisterType<RemoteFsModel>("OpenIPC", 1, 0, "RemoteFsModel");

    qmlRegisterSingletonInstance("OpenIPC", 1, 0, "SystemController", &systemController);

    const QUrl url(u"qrc:/OpenIPC/src/ui/Main.qml"_qs);
    qInfo() << "engine.load start" << url;
    
    // Connect to objectCreated to catch errors early
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            qCritical() << "Failed to load QML app! Object is null.";
            QCoreApplication::exit(-1);
        } else {
            qInfo() << "QML Object created successfully:" << objUrl;
        }
    }, Qt::QueuedConnection);

    engine.load(url);
    qInfo() << "engine.load finished";

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "No root objects loaded - Code -1";
        return -1;
    }
    
    if (auto windowObj = qobject_cast<QWindow*>(engine.rootObjects().constFirst())) {
        qInfo() << "Initial window visible=" << windowObj->isVisible();
        windowObj->show(); // Force show immediate
    }

    return app.exec();
}
