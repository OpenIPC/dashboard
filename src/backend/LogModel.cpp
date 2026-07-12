#include "LogModel.h"
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>
#include <QUrl>
#include <utility>

namespace {
constexpr int kMaxInMemoryLogs = 2000;

QString localPathFromInput(const QString &pathOrUrl)
{
    const QUrl url(pathOrUrl);
    return url.isLocalFile() ? url.toLocalFile() : pathOrUrl;
}

QString levelForType(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg: return QStringLiteral("DBG");
    case QtInfoMsg: return QStringLiteral("INF");
    case QtWarningMsg: return QStringLiteral("WRN");
    case QtCriticalMsg: return QStringLiteral("CRT");
    case QtFatalMsg: return QStringLiteral("FTL");
    }
    return QStringLiteral("UNK");
}

QtMsgType typeForLevel(const QString &level)
{
    if (level == QStringLiteral("DBG")) return QtDebugMsg;
    if (level == QStringLiteral("WRN")) return QtWarningMsg;
    if (level == QStringLiteral("CRT")) return QtCriticalMsg;
    if (level == QStringLiteral("FTL")) return QtFatalMsg;
    return QtInfoMsg;
}

LogEntry parseLogLine(const QString &line)
{
    static const QRegularExpression pattern(
        QStringLiteral(R"(^(\S+)\s+\[([A-Z]{3})\]\s?(.*)$)"));

    const QRegularExpressionMatch match = pattern.match(line);
    if (!match.hasMatch()) {
        return {QDateTime::currentDateTime(), QtInfoMsg, line};
    }

    QDateTime timestamp = QDateTime::fromString(match.captured(1), Qt::ISODateWithMs);
    if (!timestamp.isValid()) {
        timestamp = QDateTime::fromString(match.captured(1), Qt::ISODate);
    }
    if (!timestamp.isValid()) {
        timestamp = QDateTime::currentDateTime();
    }

    return {timestamp, typeForLevel(match.captured(2)), match.captured(3)};
}
} // namespace

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

int LogModel::count() const
{
    return m_logs.count();
}

QString LogModel::sourcePath() const
{
    return m_sourcePath;
}

void LogModel::setSourcePath(const QString &path)
{
    const QString normalized = localPathFromInput(path);
    if (m_sourcePath == normalized)
        return;

    m_sourcePath = normalized;
    emit sourcePathChanged();
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
        return levelForType(entry.type);
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
    appendEntry(QDateTime::currentDateTime(), type, message);
}

void LogModel::appendEntry(const QDateTime &timestamp, QtMsgType type, const QString &message)
{
    beginInsertRows(QModelIndex(), m_logs.count(), m_logs.count());
    m_logs.append({timestamp, type, message});
    endInsertRows();

    trimToLimit();
    emit countChanged();
}

void LogModel::trimToLimit()
{
    const int overflow = m_logs.count() - kMaxInMemoryLogs;
    if (overflow > 0) {
        beginRemoveRows(QModelIndex(), 0, overflow - 1);
        m_logs.remove(0, overflow);
        endRemoveRows();
    }
}

void LogModel::clear()
{
    if (m_logs.isEmpty())
        return;

    beginResetModel();
    m_logs.clear();
    endResetModel();
    emit countChanged();
}

void LogModel::saveLog(const QString &fileUrl)
{
    const QString localPath = localPathFromInput(fileUrl);
    
    QFile file(localPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const auto &entry : m_logs) {
             out << entry.timestamp.toString(Qt::ISODateWithMs)
                 << " [" << levelForType(entry.type) << "] "
                 << entry.message << "\n";
        }
        file.close();
    }
}

bool LogModel::loadFromFile(const QString &fileUrl)
{
    const QString localPath = localPathFromInput(fileUrl);
    if (localPath.trimmed().isEmpty()) {
        return false;
    }

    QFile file(localPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QVector<LogEntry> loaded;
    loaded.reserve(kMaxInMemoryLogs);
    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine();
        loaded.append(parseLogLine(line));
        if (loaded.count() > kMaxInMemoryLogs) {
            loaded.remove(0, loaded.count() - kMaxInMemoryLogs);
        }
    }

    setSourcePath(localPath);

    beginResetModel();
    m_logs = std::move(loaded);
    endResetModel();
    emit countChanged();
    return true;
}

bool LogModel::reloadFromFile()
{
    return loadFromFile(m_sourcePath);
}
