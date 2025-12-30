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

    void addLog(QtMsgType type, const QString &message);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void saveLog(const QString &fileUrl);

private:
    QVector<LogEntry> m_logs;
};

#endif // LOGMODEL_H
