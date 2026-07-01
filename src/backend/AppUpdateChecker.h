#ifndef APPUPDATECHECKER_H
#define APPUPDATECHECKER_H

#include <QObject>
#include <QString>
#include <QDateTime>

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
    QString errorString() const { return m_errorString; }
    QDateTime lastChecked() const { return m_lastChecked; }

    Q_INVOKABLE void checkNow();
    Q_INVOKABLE void openReleasePage() const;
    Q_INVOKABLE void dismissCurrentUpdate();
    Q_INVOKABLE void remindLater();
    Q_INVOKABLE bool isNewerThanCurrent(const QString &version) const;

    static int compareVersions(const QString &left, const QString &right);
    static bool isVersionNewer(const QString &candidate, const QString &current);

signals:
    void checkingChanged();
    void updateStateChanged();
    void updateAvailable(const QString &version, const QString &url, bool prerelease);
    void checkFinished(bool hasUpdate);

private:
    struct ReleaseInfo {
        QString version;
        QString name;
        QString url;
        QString notes;
        bool prerelease = false;
    };

    void setChecking(bool checking);
    void setError(const QString &error);
    void clearUpdate();
    void applyRelease(const ReleaseInfo &release);
    void handleReply(QNetworkReply *reply);
    static QString normalizeVersion(QString version);

    QString m_currentVersion;
    bool m_checking = false;
    bool m_hasUpdate = false;
    QString m_latestVersion;
    QString m_latestName;
    QString m_releaseUrl;
    QString m_releaseNotes;
    bool m_latestPrerelease = false;
    QString m_errorString;
    QDateTime m_lastChecked;
    QNetworkAccessManager *m_network = nullptr;
    QTimer *m_periodicTimer = nullptr;
    QString m_ignoredVersion;
};

#endif // APPUPDATECHECKER_H
