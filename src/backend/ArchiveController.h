#ifndef ARCHIVECONTROLLER_H
#define ARCHIVECONTROLLER_H

#include <QObject>
#include <QDateTime>
#include <QVariantList>
#include <QMutex>

struct RecordedFile {
    QDateTime startTime;
    QDateTime endTime;
    int size;
    int channel;
    QString fileName;
    QString filePath;
    int type; // 0: All, 1: Alarm, 2: Motion, etc.
};

class ArchiveController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isSearching READ isSearching NOTIFY isSearchingChanged)
    Q_PROPERTY(QVariantList searchResults READ searchResults NOTIFY searchResultsChanged)

public:
    explicit ArchiveController(QObject *parent = nullptr);
    ~ArchiveController();

    Q_INVOKABLE void login(const QString &ip, int port, const QString &username, const QString &password);
    Q_INVOKABLE void logout();
    Q_INVOKABLE void search(const QDateTime &startTime, const QDateTime &endTime, const QString &cameraIp, const QString &recordingsPath);
    Q_INVOKABLE void download(int index, const QString &savePath);
    
    Q_INVOKABLE void exportVideo(const QString &inputFile, const QString &outputFile, qint64 startMs, qint64 endMs);

    bool isSearching() const { return m_isSearching; }
    QVariantList searchResults() const;

signals:
    void isSearchingChanged();
    void searchResultsChanged();
    void loginSuccess();
    void loginFailed(const QString &error);
    void searchFinished(int count);
    void downloadProgress(int index, int progress);
    void downloadFinished(int index);
    void downloadError(int index, const QString &error);
    void exportProgress(int progress);
    void exportFinished();
    void exportError(const QString &error);

private:
    bool m_isSearching = false;
    QList<RecordedFile> m_files;
    QMutex m_mutex;
};

#endif // ARCHIVECONTROLLER_H
