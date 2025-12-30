#include "LogModel.h"
#include <QFile>
#include <QTextStream>
#include <QUrl>

LogModel::LogModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int LogModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_logs.count();
}

QVariant LogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_logs.count())
        return QVariant();

    const LogEntry &entry = m_logs[index.row()];

    switch (role) {
    case TimestampRole:
        return entry.timestamp;
    case TypeRole:
        return static_cast<int>(entry.type);
    case MessageRole:
        return entry.message;
    case FormattedTimeRole:
        return entry.timestamp.toString("HH:mm:ss.zzz");
    case LevelStringRole:
        switch (entry.type) {
        case QtDebugMsg: return "DBG";
        case QtInfoMsg: return "INF";
        case QtWarningMsg: return "WRN";
        case QtCriticalMsg: return "CRT";
        case QtFatalMsg: return "FTL";
        default: return "UNK";
        }
    }

    return QVariant();
}

QHash<int, QByteArray> LogModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TimestampRole] = "timestamp";
    roles[TypeRole] = "type";
    roles[MessageRole] = "message";
    roles[FormattedTimeRole] = "formattedTime";
    roles[LevelStringRole] = "levelString";
    return roles;
}

void LogModel::addLog(QtMsgType type, const QString &message)
{
    beginInsertRows(QModelIndex(), m_logs.count(), m_logs.count());
    m_logs.append({QDateTime::currentDateTime(), type, message});
    endInsertRows();

    // Limit log size to prevent memory leaks
    if (m_logs.count() > 2000) {
        beginRemoveRows(QModelIndex(), 0, 0);
        m_logs.removeFirst();
        endRemoveRows();
    }
}

void LogModel::clear()
{
    beginResetModel();
    m_logs.clear();
    endResetModel();
}

void LogModel::saveLog(const QString &fileUrl)
{
    QUrl url(fileUrl);
    QString localPath = url.isLocalFile() ? url.toLocalFile() : fileUrl;
    
    QFile file(localPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const auto &entry : m_logs) {
             QString level;
             switch (entry.type) {
                 case QtDebugMsg: level = "DBG"; break;
                 case QtInfoMsg: level = "INF"; break;
                 case QtWarningMsg: level = "WRN"; break;
                 case QtCriticalMsg: level = "CRT"; break;
                 case QtFatalMsg: level = "FTL"; break;
                 default: level = "UNK"; break;
             }
             out << entry.timestamp.toString(Qt::ISODate) << " [" << level << "] " << entry.message << "\n";
        }
        file.close();
    }
}
