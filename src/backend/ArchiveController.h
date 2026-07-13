#ifndef ARCHIVECONTROLLER_H
#define ARCHIVECONTROLLER_H

#include <QObject>
#include <QDateTime>
#include <QList>
#include <QVariantList>
#include <QVariantMap>
#include <QMutex>

#include "RecordingFileCatalog.h"

using RecordedFile = RecordingFileCatalog::RecordingFile;

class ArchiveController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isSearching READ isSearching NOTIFY isSearchingChanged)
    Q_PROPERTY(QVariantList searchResults READ searchResults NOTIFY searchResultsChanged)
    Q_PROPERTY(bool isExporting READ isExporting NOTIFY exportStateChanged)
    Q_PROPERTY(int exportProgress READ exportProgress NOTIFY exportStateChanged)
    Q_PROPERTY(QString exportStatus READ exportStatus NOTIFY exportStateChanged)
    Q_PROPERTY(QString exportErrorText READ exportErrorText NOTIFY exportStateChanged)
    Q_PROPERTY(QString exportOutputFile READ exportOutputFile NOTIFY exportStateChanged)

public:
    explicit ArchiveController(QObject *parent = nullptr);
    ~ArchiveController();

    Q_INVOKABLE void login(const QString &ip, int port, const QString &username, const QString &password);
    Q_INVOKABLE void logout();
    Q_INVOKABLE void search(const QDateTime &startTime, const QDateTime &endTime, const QString &cameraIp, const QString &recordingsPath);
    Q_INVOKABLE void download(int index, const QString &savePath);
    
    Q_INVOKABLE void exportVideo(const QString &inputFile, const QString &outputFile, qint64 startMs, qint64 endMs);
    Q_INVOKABLE void clearExportStatus();
    Q_INVOKABLE QVariantMap storageSummary(const QString &recordingsPath) const;
    Q_INVOKABLE QVariantMap cleanupRecordings(const QString &recordingsPath,
                                              int keepDays,
                                              qint64 maxBytes,
                                              bool dryRun = true);

    bool isSearching() const { return m_isSearching; }
    bool isExporting() const { return m_isExporting; }
    int exportProgress() const { return m_exportProgress; }
    QString exportStatus() const { return m_exportStatus; }
    QString exportErrorText() const { return m_exportErrorText; }
    QString exportOutputFile() const { return m_exportOutputFile; }
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
    void exportStarted(const QString &outputFile);
    void exportProgress(int progress);
    void exportFinished();
    void exportError(const QString &error);
    void exportStateChanged();
    void cleanupFinished(const QVariantMap &result);

private:
    bool m_isSearching = false;
    bool m_isExporting = false;
    int m_exportProgress = 0;
    QString m_exportStatus;
    QString m_exportErrorText;
    QString m_exportOutputFile;
    QList<RecordedFile> m_files;
    mutable QMutex m_mutex;
    quint64 m_searchRequestId = 0;
    quint64 m_exportRequestId = 0;
};

#endif // ARCHIVECONTROLLER_H
