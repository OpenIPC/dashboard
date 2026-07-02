#ifndef REMOTEFSMODEL_H
#define REMOTEFSMODEL_H

#include <QAbstractListModel>
#include <QProcess>
#include <QDateTime>
#include <QtQml/qqmlregistration.h>
#include <functional>

struct RemoteFileEntry {
    QString name;
    quint64 size;
    bool isDir; 
    QString permissions;
    QString dateStr;
};

class RemoteFsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString currentPath READ currentPath NOTIFY currentPathChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    
public:
    enum RemoteRoles {
        NameRole = Qt::UserRole + 1,
        SizeRole,
        IsDirRole, // true/false
        DateRole,
        PermissionsRole
    };

    explicit RemoteFsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString currentPath() const;
    bool isLoading() const;

    Q_INVOKABLE void connectAndList(const QString &ip, const QString &user, const QString &password);
    Q_INVOKABLE void navigateUp();
    Q_INVOKABLE void navigate(const QString &folderName);
    Q_INVOKABLE void refresh();
    
    Q_INVOKABLE void deleteItem(const QString &fileName);
    Q_INVOKABLE void renameItem(const QString &oldName, const QString &newName);
    Q_INVOKABLE void createDirectory(const QString &dirName);
    
    // Local File Operations
    Q_INVOKABLE void localDeleteItem(const QString &path);
    Q_INVOKABLE void localRenameItem(const QString &oldPath, const QString &newName);
    Q_INVOKABLE void localCreateDirectory(const QString &parentPath, const QString &dirName);

    Q_INVOKABLE void downloadFile(const QString &fileName, const QString &localDestPath);
    Q_INVOKABLE void uploadFile(const QString &localPath);

signals:
    void currentPathChanged();
    void isLoadingChanged();
    void errorOccurred(const QString &error);
    void fileDownloaded(const QString &fileName);
    void fileUploaded(const QString &fileName);

private:
    void runSshCommand(const QString &cmd, std::function<void(const QString &output, const QString &error)> callback);
    void listDirectory(const QString &path);
    void parseLsOutput(const QString &output);
    QProcessEnvironment buildSshEnvironment() const;
    QString askPassExecutablePath() const;
    QString knownHostsFilePath() const;
    QStringList commonSshOptions() const;

    QList<RemoteFileEntry> m_entries;
    QString m_currentPath;
    bool m_isLoading;
    QString m_ip;
    QString m_user;
    QString m_password;
};

#endif // REMOTEFSMODEL_H
