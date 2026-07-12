#ifndef LOGMODEL_H
#define LOGMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QVector>

struct LogEntry {
    QDateTime timestamp;
    QtMsgType type;
    QString message;
};

class LogModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString sourcePath READ sourcePath NOTIFY sourcePathChanged)

public:
    enum LogRoles {
        TimestampRole = Qt::UserRole + 1,
        TypeRole,
        MessageRole,
        FormattedTimeRole,
        LevelStringRole
    };

    explicit LogModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int count() const;
    QString sourcePath() const;
    void setSourcePath(const QString &path);

    void addLog(QtMsgType type, const QString &message);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void saveLog(const QString &fileUrl);
    Q_INVOKABLE bool loadFromFile(const QString &fileUrl);
    Q_INVOKABLE bool reloadFromFile();

signals:
    void countChanged();
    void sourcePathChanged();

private:
    void appendEntry(const QDateTime &timestamp, QtMsgType type, const QString &message);
    void trimToLimit();

    QVector<LogEntry> m_logs;
    QString m_sourcePath;
};

#endif // LOGMODEL_H
