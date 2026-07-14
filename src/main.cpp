#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQml>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QTimer>
#include <QStandardPaths>
#include <QPointer>
#include <QMutex>
#include <QMutexLocker>
#include <QSslSocket>
#include "backend/SystemController.h"
#include "backend/gst/GstPlayer.h"
#include "backend/analytics/AnalyticsEngine.h"
#include "backend/SshClient.h"
#include "backend/RemoteFsModel.h"
#include "backend/CamexController.h"
#include <functional>
#include <atomic>
#include <cstdio>
#include "config.h"

// Hardcoded GStreamer paths for Windows environment
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {
struct LogState {
    QFile logFile;
    QMutex logMutex;
    std::function<void(QtMsgType, const QString&)> logCallback;
    std::atomic_bool logTeardown{false};
};

LogState &logState()
{
    static LogState state;
    return state;
}

void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    LogState &state = logState();

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

    std::function<void(QtMsgType, const QString&)> callbackCopy;

    {
        QMutexLocker locker(&state.logMutex);
        // Write to file (thread-safe)
        if (state.logFile.isOpen()) {
            // Log rotation: 10 MB limit
            if (state.logFile.size() > 10 * 1024 * 1024) {
                QString logPath = state.logFile.fileName();
                state.logFile.close();
                
                // Rotate up to 5 files
                for (int i = 4; i >= 1; --i) {
                    QString oldFile = QString("%1.%2").arg(logPath).arg(i);
                    QString newFile = QString("%1.%2").arg(logPath).arg(i + 1);
                    if (QFile::exists(oldFile)) {
                        QFile::remove(newFile);
                        QFile::rename(oldFile, newFile);
                    }
                }
                QFile::remove(logPath + ".1");
                QFile::rename(logPath, logPath + ".1");
                
                state.logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
            }
            
            QTextStream ts(&state.logFile);
            ts << formattedMsg << '\n';
            ts.flush();
        }

        if (!state.logTeardown.load(std::memory_order_relaxed)) {
            callbackCopy = state.logCallback;
        }
    }
    
    // Write to stderr so it shows up in the terminal
    fprintf(stderr, "%s\n", qPrintable(formattedMsg));
    fflush(stderr);

    if (callbackCopy) {
        callbackCopy(type, msg);
    }

    if (type == QtFatalMsg)
        abort();
    Q_UNUSED(context);
}
} // namespace

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    // Ensure Qt can find bundled plugins when running from installer
    const QString appDir = QDir::cleanPath(QFileInfo(QString::fromLocal8Bit(argv[0])).absolutePath());
    const QString qtPluginsDir = appDir + "/qt_plugins";
    const QString qtPlatformsDir = qtPluginsDir + "/platforms";
    if (QDir(qtPluginsDir).exists()) {
        qputenv("QT_PLUGIN_PATH", qtPluginsDir.toLocal8Bit());
    }
    if (QDir(qtPlatformsDir).exists()) {
        qputenv("QT_QPA_PLATFORM_PLUGIN_PATH", qtPlatformsDir.toLocal8Bit());
    }

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

    // Force Software rendering to avoid D3D11/OpenGL issues
    // This MUST be done before QGuiApplication is created
    // qputenv("QSG_RHI_BACKEND", "software");
    // QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);

    // Keep QML resources deterministic after upgrades and language/UI changes.
    qputenv("QML_DISABLE_DISK_CACHE", "1");

    QGuiApplication app(argc, argv); 
    app.setOrganizationName("OpenIPC");
    app.setApplicationName("Dashboard");
#ifdef APP_VERSION
    app.setApplicationVersion(QString::fromUtf8(APP_VERSION));
#endif

    const bool smokeQml = app.arguments().contains(QStringLiteral("--smoke-qml"));

    if (app.arguments().contains(QStringLiteral("--self-test-tls"))) {
        const bool tlsAvailable = QSslSocket::supportsSsl();
        qInfo().noquote() << "TLS self-test:"
                          << "supportsSsl=" << tlsAvailable
                          << "build=" << QSslSocket::sslLibraryBuildVersionString()
                          << "runtime=" << QSslSocket::sslLibraryVersionString();
        return tlsAvailable ? 0 : 2;
    }

    // Configure GStreamer paths for standalone deployment
    // This allows the app to find plugins in ./lib/gstreamer-1.0 relative to executable
    const QString appDirPath = QCoreApplication::applicationDirPath();
    QString localGstPlugins = appDirPath + "/lib/gstreamer-1.0";
    if (QDir(localGstPlugins).exists()) {
        qputenv("GST_PLUGIN_PATH", localGstPlugins.toLocal8Bit());
        qputenv("GST_PLUGIN_PATH_1_0", localGstPlugins.toLocal8Bit());
        qputenv("GST_PLUGIN_SYSTEM_PATH", ""); // Ignore system install if local exists
        qputenv("GST_PLUGIN_SYSTEM_PATH_1_0", "");
        qputenv("PATH", (appDirPath + ";" + qEnvironmentVariable("PATH")).toLocal8Bit());

        // Use local registry to avoid stale system cache paths
        QString localRegistry = appDirPath + "/gstreamer-registry.bin";
        qputenv("GST_REGISTRY", localRegistry.toLocal8Bit());
        qputenv("GST_REGISTRY_FORK", "0");

        QString localScanner = localGstPlugins + "/gst-plugin-scanner.exe";
        if (QFile::exists(localScanner)) {
            qputenv("GST_PLUGIN_SCANNER", localScanner.toLocal8Bit());
            qputenv("GST_PLUGIN_SCANNER_1_0", localScanner.toLocal8Bit());
        }
    }

    QString logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(logPath);
    LogState &state = logState();
    state.logFile.setFileName(logPath + "/app.log");
    
    const bool useCustomLogger = (qEnvironmentVariable("OPENIPC_DISABLE_CUSTOM_LOGGER") != "1");

    if (state.logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream ts(&state.logFile);
        ts << QDateTime::currentDateTime().toString(Qt::ISODate) << " [INF] app start" << '\n';
        ts.flush();
    } else {
        fprintf(stderr, "Failed to open log file, logging to stderr only.\n");
    }

    if (useCustomLogger) {
        qInstallMessageHandler(logMessageHandler);
    }

    qInfo().noquote() << "Platform" << QGuiApplication::platformName()
                      << "QSG_RHI_BACKEND=" << qEnvironmentVariable("QSG_RHI_BACKEND")
                      << "QT_QUICK_BACKEND=" << qEnvironmentVariable("QT_QUICK_BACKEND")
                      << "graphicsApi=" << QQuickWindow::graphicsApi();
    qInfo() << "Custom Qt message handler enabled:" << useCustomLogger;
    qInfo() << "Log file path:" << state.logFile.fileName();

    if (smokeQml) {
        qputenv("OPENIPC_SMOKE_QML", "1");
    }

    // Register the C++ backend controller FIRST to ensure it outlives the engine
    SystemController systemController;    
    QPointer<SystemController> systemControllerPtr(&systemController);
    if (LogModel *model = systemController.logModel()) {
        model->setSourcePath(state.logFile.fileName());
        model->reloadFromFile();
    }
    // Hook up logging to SystemController
    state.logCallback = [systemControllerPtr](QtMsgType type, const QString &msg) {
        if (systemControllerPtr) {
            systemControllerPtr->addLog(type, msg);
        }
    };

    QObject::connect(&app, &QCoreApplication::aboutToQuit, &app, []() {
        // Stop forwarding logs into QObject-based models during teardown.
        LogState &shutdownState = logState();
        shutdownState.logTeardown.store(true, std::memory_order_relaxed);
        QMutexLocker locker(&shutdownState.logMutex);
        shutdownState.logCallback = nullptr;
    });

    QQmlApplicationEngine engine;
#ifdef APP_VERSION
    engine.rootContext()->setContextProperty("AppVersion", QString::fromUtf8(APP_VERSION));
#else
    engine.rootContext()->setContextProperty("AppVersion", QString());
#endif

#ifdef APP_AUTHOR
    engine.rootContext()->setContextProperty("AppAuthor", QString::fromUtf8(APP_AUTHOR));
#else
    engine.rootContext()->setContextProperty("AppAuthor", QString("Rinat Ibragimov"));
#endif

#ifdef APP_BUILD_YEAR
    engine.rootContext()->setContextProperty("AppBuildYear", QString::fromUtf8(APP_BUILD_YEAR));
#else
    engine.rootContext()->setContextProperty("AppBuildYear", QString("2026"));
#endif

    // Runtime registrations stay explicit. Qt 6.4's qmltyperegistrar does not
    // reliably export classes that were added to the executable before
    // qt_add_qml_module(), and missing VideoPlayer/RemoteFsModel/SshClient types
    // make the application fail to start before the first window appears.
    qmlRegisterType<GstPlayer>("OpenIPC", 1, 0, "VideoPlayer");
    qmlRegisterType<AnalyticsEngine>("OpenIPC", 1, 0, "AnalyticsEngine");
    qmlRegisterType<SshClient>("OpenIPC", 1, 0, "SshClient");
    qmlRegisterType<RemoteFsModel>("OpenIPC", 1, 0, "RemoteFsModel");
    qmlRegisterUncreatableType<CamexController>("OpenIPC", 1, 0, "CamexController",
                                                "Use SystemController.camexController");
    qmlRegisterSingletonInstance("OpenIPC", 1, 0, "SystemController", &systemController);

    const QUrl url = smokeQml
        ? QUrl(u"qrc:/OpenIPC/src/ui/SmokeHarness.qml"_qs)
        : QUrl(u"qrc:/OpenIPC/src/ui/Main.qml"_qs);
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

    if (smokeQml) {
        QObject *rootObject = engine.rootObjects().constFirst();
        const bool connected = QObject::connect(rootObject, SIGNAL(smokeFinished(bool,QString)),
                                                &app, SLOT(quit()));

        if (!connected) {
            qCritical() << "Smoke harness did not expose smokeFinished(bool, QString)";
            return -2;
        }

        QTimer::singleShot(15000, &app, []() {
            qCritical() << "QML smoke timed out";
            QCoreApplication::exit(3);
        });

        const int smokeEventLoopResult = app.exec();
        if (smokeEventLoopResult != 0) {
            return smokeEventLoopResult;
        }

        const bool smokeOk = rootObject->property("smokeOk").toBool();
        const QString smokeMessage = rootObject->property("smokeMessage").toString();
        if (smokeOk) {
            qInfo().noquote() << smokeMessage;
            return 0;
        }

        qCritical().noquote() << "QML smoke failed:" << smokeMessage;
        return 4;
    }
    
    if (auto windowObj = qobject_cast<QWindow*>(engine.rootObjects().constFirst())) {
        qInfo() << "Initial window visible=" << windowObj->isVisible();
        windowObj->show(); // Force show immediate
    }

    return app.exec();
}
