#ifndef APPUPDATECHECKER_H
#define APPUPDATECHECKER_H

#include <QObject>
#include <QString>
#include <QDateTime>

class QFile;
class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

class AppUpdateChecker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
    Q_PROPERTY(bool checking READ checking NOTIFY checkingChanged)
    Q_PROPERTY(bool hasUpdate READ hasUpdate NOTIFY updateStateChanged)
    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY updateStateChanged)
    Q_PROPERTY(QString latestName READ latestName NOTIFY updateStateChanged)
    Q_PROPERTY(QString releaseUrl READ releaseUrl NOTIFY updateStateChanged)
    Q_PROPERTY(QString releaseNotes READ releaseNotes NOTIFY updateStateChanged)
    Q_PROPERTY(bool latestPrerelease READ latestPrerelease NOTIFY updateStateChanged)
    Q_PROPERTY(QString assetName READ assetName NOTIFY updateStateChanged)
    Q_PROPERTY(qint64 assetSize READ assetSize NOTIFY updateStateChanged)
    Q_PROPERTY(bool downloadAvailable READ downloadAvailable NOTIFY updateStateChanged)
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadStateChanged)
    Q_PROPERTY(bool installing READ installing NOTIFY installStateChanged)
    Q_PROPERTY(int downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)
    Q_PROPERTY(qint64 downloadReceivedBytes READ downloadReceivedBytes NOTIFY downloadProgressChanged)
    Q_PROPERTY(qint64 downloadTotalBytes READ downloadTotalBytes NOTIFY downloadProgressChanged)
    Q_PROPERTY(QString downloadedFilePath READ downloadedFilePath NOTIFY downloadStateChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY updateStateChanged)
    Q_PROPERTY(QDateTime lastChecked READ lastChecked NOTIFY updateStateChanged)

public:
    explicit AppUpdateChecker(QObject *parent = nullptr);

    QString currentVersion() const { return m_currentVersion; }
    bool checking() const { return m_checking; }
    bool hasUpdate() const { return m_hasUpdate; }
    QString latestVersion() const { return m_latestVersion; }
    QString latestName() const { return m_latestName; }
    QString releaseUrl() const { return m_releaseUrl; }
    QString releaseNotes() const { return m_releaseNotes; }
    bool latestPrerelease() const { return m_latestPrerelease; }
    QString assetName() const { return m_assetName; }
    qint64 assetSize() const { return m_assetSize; }
    bool downloadAvailable() const { return !m_assetDownloadUrl.isEmpty(); }
    bool downloading() const { return m_downloading; }
    bool installing() const { return m_installing; }
    int downloadProgress() const { return m_downloadProgress; }
    qint64 downloadReceivedBytes() const { return m_downloadReceivedBytes; }
    qint64 downloadTotalBytes() const { return m_downloadTotalBytes; }
    QString downloadedFilePath() const { return m_downloadedFilePath; }
    QString errorString() const { return m_errorString; }
    QDateTime lastChecked() const { return m_lastChecked; }

    Q_INVOKABLE void checkNow();
    Q_INVOKABLE void openReleasePage() const;
    Q_INVOKABLE void downloadUpdate();
    Q_INVOKABLE void cancelDownload();
    Q_INVOKABLE void installDownloadedUpdate();
    Q_INVOKABLE void dismissCurrentUpdate();
    Q_INVOKABLE void remindLater();
    Q_INVOKABLE bool isNewerThanCurrent(const QString &version) const;

    static int compareVersions(const QString &left, const QString &right);
    static bool isVersionNewer(const QString &candidate, const QString &current);

signals:
    void checkingChanged();
    void updateStateChanged();
    void downloadStateChanged();
    void downloadProgressChanged();
    void installStateChanged();
    void updateAvailable(const QString &version, const QString &url, bool prerelease);
    void checkFinished(bool hasUpdate);
    void downloadFinished(bool success, const QString &filePath);
    void installStarted(const QString &filePath);

private:
    struct ReleaseInfo {
        QString version;
        QString name;
        QString url;
        QString notes;
        bool prerelease = false;
        QString assetName;
        QString assetDownloadUrl;
        qint64 assetSize = 0;
    };

    void setChecking(bool checking);
    void setDownloading(bool downloading);
    void setInstalling(bool installing);
    void setError(const QString &error);
    void clearUpdate();
    void applyRelease(const ReleaseInfo &release);
    void handleReply(QNetworkReply *reply);
    void handleDownloadFinished(QNetworkReply *reply);
    bool prepareDownloadFile(QString *error);
    bool startInstallerHelper(QString *error);
    void resetDownloadState(bool removeFile);
    static QString compatibleAssetName(const QJsonArray &assets, QString *downloadUrl, qint64 *size);
    static bool isCompatibleAssetName(const QString &name);
    static QString platformAssetHint();
    static QString safeDownloadFileName(const QString &assetName);
    static QString normalizeVersion(QString version);

    QString m_currentVersion;
    bool m_checking = false;
    bool m_hasUpdate = false;
    QString m_latestVersion;
    QString m_latestName;
    QString m_releaseUrl;
    QString m_releaseNotes;
    bool m_latestPrerelease = false;
    QString m_assetName;
    QString m_assetDownloadUrl;
    qint64 m_assetSize = 0;
    bool m_downloading = false;
    bool m_installing = false;
    int m_downloadProgress = 0;
    qint64 m_downloadReceivedBytes = 0;
    qint64 m_downloadTotalBytes = 0;
    QString m_downloadedFilePath;
    QNetworkReply *m_downloadReply = nullptr;
    QFile *m_downloadFile = nullptr;
    QString m_errorString;
    QDateTime m_lastChecked;
    QNetworkAccessManager *m_network = nullptr;
    QTimer *m_periodicTimer = nullptr;
    QString m_ignoredVersion;
};

#endif // APPUPDATECHECKER_H
