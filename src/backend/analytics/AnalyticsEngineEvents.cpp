#include "AnalyticsEngine.h"
#include "../PathUtils.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QSaveFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

QVariantList AnalyticsEngine::analyticsEvents() const
{
    QMutexLocker locker(&m_analyticsEventsMutex);
    return m_analyticsEvents;
}

QVariantList AnalyticsEngine::queryAnalyticsEvents(int type, const QString &cameraId, const QString &text, int limit) const
{
    const QString trimmedCameraId = cameraId.trimmed();
    const QString trimmedText = text.trimmed();
    const int safeLimit = qBound(1, limit, 5000);

    {
        QMutexLocker locker(&m_eventStoreMutex);
        if (m_eventStoreReady && QSqlDatabase::contains(m_eventStoreConnectionName)) {
            QSqlDatabase db = QSqlDatabase::database(m_eventStoreConnectionName, false);
            if (db.isOpen()) {
                QSqlQuery query(db);
                query.prepare(
                    "SELECT payload_json FROM analytics_events "
                    "WHERE (? < 0 OR module_type = ?) "
                    "AND (? = '' OR camera_id = ?) "
                    "AND (? = '' OR label LIKE ? OR message LIKE ? OR rule_name LIKE ? OR event_type LIKE ?) "
                    "ORDER BY timestamp_ms DESC, rowid DESC LIMIT ?"
                );

                const QString likeText = "%" + trimmedText + "%";
                query.addBindValue(type);
                query.addBindValue(type);
                query.addBindValue(trimmedCameraId);
                query.addBindValue(trimmedCameraId);
                query.addBindValue(trimmedText);
                query.addBindValue(likeText);
                query.addBindValue(likeText);
                query.addBindValue(likeText);
                query.addBindValue(likeText);
                query.addBindValue(safeLimit);

                QVariantList result;
                if (!query.exec()) {
                    qWarning() << "Failed to query analytics events:" << query.lastError().text();
                    return result;
                }

                while (query.next()) {
                    const QJsonDocument doc = QJsonDocument::fromJson(query.value(0).toString().toUtf8());
                    if (doc.isObject()) {
                        result.append(doc.object().toVariantMap());
                    }
                }
                return result;
            }
        }
    }

    QVariantList eventSnapshot;
    {
        QMutexLocker locker(&m_analyticsEventsMutex);
        eventSnapshot = m_analyticsEvents;
    }

    QVariantList filtered;
    for (const QVariant &eventVar : eventSnapshot) {
        const QVariantMap event = eventVar.toMap();
        if (type >= 0 && event.value("moduleType").toInt() != type) {
            continue;
        }
        if (!trimmedCameraId.isEmpty() && event.value("cameraId").toString() != trimmedCameraId) {
            continue;
        }
        if (!trimmedText.isEmpty()) {
            const QString haystack = QStringList{
                event.value("label").toString(),
                event.value("message").toString(),
                event.value("ruleName").toString(),
                event.value("eventType").toString()
            }.join(" ");
            if (!haystack.contains(trimmedText, Qt::CaseInsensitive)) {
                continue;
            }
        }
        filtered.append(event);
        if (filtered.size() >= safeLimit) {
            break;
        }
    }
    return filtered;
}

void AnalyticsEngine::clearAnalyticsEvents(int type, const QString &cameraId)
{
    const QString trimmedCameraId = cameraId.trimmed();
    if (type < 0 && trimmedCameraId.isEmpty()) {
        bool wasEmpty = false;
        {
            QMutexLocker locker(&m_analyticsEventsMutex);
            wasEmpty = m_analyticsEvents.isEmpty();
            m_analyticsEvents.clear();
        }
        deleteStoredAnalyticsEvents(type, trimmedCameraId);
        if (!wasEmpty) {
            emit analyticsEventsChanged();
        }
        return;
    }

    QVariantList eventSnapshot;
    {
        QMutexLocker locker(&m_analyticsEventsMutex);
        eventSnapshot = m_analyticsEvents;
    }

    QVariantList filtered;
    bool changed = false;

    for (const QVariant &eventVar : eventSnapshot) {
        const QVariantMap event = eventVar.toMap();
        bool matches = true;

        if (type >= 0) {
            matches = matches && (event.value("moduleType").toInt() == type);
        }

        if (!trimmedCameraId.isEmpty()) {
            matches = matches && (event.value("cameraId").toString() == trimmedCameraId);
        }

        if (matches) {
            changed = true;
            continue;
        }

        filtered.append(event);
    }

    if (!changed) {
        return;
    }

    {
        QMutexLocker locker(&m_analyticsEventsMutex);
        m_analyticsEvents = filtered;
    }
    deleteStoredAnalyticsEvents(type, trimmedCameraId);
    emit analyticsEventsChanged();
}

void AnalyticsEngine::appendAnalyticsEvents(const QVariantList &events)
{
    if (events.isEmpty()) {
        return;
    }

    persistAnalyticsEvents(events);

    {
        QMutexLocker locker(&m_analyticsEventsMutex);
        for (int i = events.size() - 1; i >= 0; --i) {
            m_analyticsEvents.prepend(events.at(i));
        }

        while (m_analyticsEvents.size() > m_maxAnalyticsEvents) {
            m_analyticsEvents.removeLast();
        }
    }

    emit analyticsEventsChanged();
}

void AnalyticsEngine::initEventStore()
{
    QMutexLocker locker(&m_eventStoreMutex);
    if (m_eventStorePath.isEmpty() || m_eventStoreConnectionName.isEmpty()) {
        return;
    }

    QDir dir(QFileInfo(m_eventStorePath).absolutePath());
    if (!dir.exists() && !dir.mkpath(".")) {
        qWarning() << "Failed to create analytics event store directory:" << dir.absolutePath();
        return;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_eventStoreConnectionName);
    db.setDatabaseName(m_eventStorePath);
    if (!db.open()) {
        qWarning() << "Failed to open analytics event store:" << db.lastError().text();
        return;
    }

    QSqlQuery query(db);
    query.exec("PRAGMA journal_mode=WAL");
    query.exec("PRAGMA synchronous=NORMAL");

    const bool tableOk = query.exec(
        "CREATE TABLE IF NOT EXISTS analytics_events ("
        "id TEXT PRIMARY KEY,"
        "timestamp_ms INTEGER NOT NULL,"
        "camera_id TEXT,"
        "module_type INTEGER,"
        "module_id TEXT,"
        "event_type TEXT,"
        "label TEXT,"
        "confidence REAL,"
        "rule_name TEXT,"
        "message TEXT,"
        "snapshot_path TEXT,"
        "clip_path TEXT,"
        "payload_json TEXT NOT NULL"
        ")"
    );

    if (!tableOk) {
        qWarning() << "Failed to initialize analytics_events table:" << query.lastError().text();
        db.close();
        return;
    }

    query.exec("CREATE INDEX IF NOT EXISTS idx_analytics_events_time ON analytics_events(timestamp_ms DESC)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_analytics_events_camera ON analytics_events(camera_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_analytics_events_module ON analytics_events(module_type)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_analytics_events_type ON analytics_events(event_type)");

    m_eventStoreReady = true;
}

QVariantList AnalyticsEngine::loadRecentAnalyticsEvents(int limit) const
{
    QVariantList events;
    QMutexLocker locker(&m_eventStoreMutex);
    if (!m_eventStoreReady || !QSqlDatabase::contains(m_eventStoreConnectionName)) {
        return events;
    }

    QSqlDatabase db = QSqlDatabase::database(m_eventStoreConnectionName, false);
    if (!db.isOpen()) {
        return events;
    }

    QSqlQuery query(db);
    query.prepare("SELECT payload_json FROM analytics_events ORDER BY timestamp_ms DESC, rowid DESC LIMIT ?");
    query.addBindValue(qMax(1, limit));
    if (!query.exec()) {
        qWarning() << "Failed to load analytics events:" << query.lastError().text();
        return events;
    }

    while (query.next()) {
        const QByteArray payload = query.value(0).toString().toUtf8();
        const QJsonDocument doc = QJsonDocument::fromJson(payload);
        if (doc.isObject()) {
            events.append(doc.object().toVariantMap());
        }
    }

    return events;
}

void AnalyticsEngine::persistAnalyticsEvents(const QVariantList &events)
{
    QMutexLocker locker(&m_eventStoreMutex);
    if (!m_eventStoreReady || !QSqlDatabase::contains(m_eventStoreConnectionName)) {
        return;
    }

    QSqlDatabase db = QSqlDatabase::database(m_eventStoreConnectionName, false);
    if (!db.isOpen()) {
        return;
    }

    if (!db.transaction()) {
        qWarning() << "Failed to start analytics event transaction:" << db.lastError().text();
    }

    QSqlQuery query(db);
    query.prepare(
        "INSERT OR REPLACE INTO analytics_events ("
        "id, timestamp_ms, camera_id, module_type, module_id, event_type, label, confidence, "
        "rule_name, message, snapshot_path, clip_path, payload_json"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    );

    bool ok = true;
    for (const QVariant &eventVar : events) {
        const QVariantMap event = eventVar.toMap();
        QString id = event.value("id").toString();
        const qint64 timestampMs = event.value("timestampMs").toLongLong();
        if (id.isEmpty()) {
            id = QStringLiteral("event:%1:%2:%3")
                .arg(event.value("cameraId").toString(),
                     event.value("moduleType").toString(),
                     QString::number(timestampMs));
        }

        query.bindValue(0, id);
        query.bindValue(1, timestampMs > 0 ? timestampMs : QDateTime::currentMSecsSinceEpoch());
        query.bindValue(2, event.value("cameraId").toString());
        query.bindValue(3, event.value("moduleType").toInt());
        query.bindValue(4, event.value("moduleId").toString());
        query.bindValue(5, event.value("eventType").toString());
        query.bindValue(6, event.value("label").toString());
        query.bindValue(7, event.value("confidence").toDouble());
        query.bindValue(8, event.value("ruleName").toString());
        query.bindValue(9, event.value("message").toString());
        query.bindValue(10, event.value("snapshotPath").toString());
        query.bindValue(11, event.value("clipPath").toString());
        query.bindValue(12, QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(event)).toJson(QJsonDocument::Compact)));

        if (!query.exec()) {
            ok = false;
            qWarning() << "Failed to persist analytics event:" << query.lastError().text();
        }
    }

    if (ok) {
        db.commit();
    } else {
        db.rollback();
    }
}

void AnalyticsEngine::deleteStoredAnalyticsEvents(int type, const QString &cameraId)
{
    QMutexLocker locker(&m_eventStoreMutex);
    if (!m_eventStoreReady || !QSqlDatabase::contains(m_eventStoreConnectionName)) {
        return;
    }

    QSqlDatabase db = QSqlDatabase::database(m_eventStoreConnectionName, false);
    if (!db.isOpen()) {
        return;
    }

    QSqlQuery query(db);
    if (type < 0 && cameraId.trimmed().isEmpty()) {
        if (!query.exec("DELETE FROM analytics_events")) {
            qWarning() << "Failed to clear analytics events:" << query.lastError().text();
        }
        return;
    }

    query.prepare(
        "DELETE FROM analytics_events "
        "WHERE (? < 0 OR module_type = ?) "
        "AND (? = '' OR camera_id = ?)"
    );
    query.addBindValue(type);
    query.addBindValue(type);
    query.addBindValue(cameraId.trimmed());
    query.addBindValue(cameraId.trimmed());
    if (!query.exec()) {
        qWarning() << "Failed to delete analytics events:" << query.lastError().text();
    }
}

QVariantMap AnalyticsEngine::exportAnalyticsEvents(const QString &path,
                                                   int type,
                                                   const QString &cameraId,
                                                   const QString &text,
                                                   const QString &format,
                                                   int limit) const
{
    QVariantMap result;
    result["ok"] = false;

    QString normalizedFormat = format.trimmed().toLower();
    if (normalizedFormat.isEmpty()) {
        normalizedFormat = QStringLiteral("json");
    }
    if (normalizedFormat != QStringLiteral("json") && normalizedFormat != QStringLiteral("csv")) {
        result["message"] = QStringLiteral("Unsupported export format");
        return result;
    }

    QString targetPath = PathUtils::localPathFromUserInput(path.trimmed());
    if (targetPath.isEmpty()) {
        QString baseDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        if (baseDir.isEmpty()) {
            baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        }
        targetPath = QDir(baseDir).filePath(QStringLiteral("OpenIPC/Analytics/analytics-events-%1.%2")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss")),
                 normalizedFormat));
    }

    QDir outputDir(QFileInfo(targetPath).absolutePath());
    if (!outputDir.exists() && !outputDir.mkpath(QStringLiteral("."))) {
        result["path"] = targetPath;
        result["message"] = QStringLiteral("Cannot create export directory");
        return result;
    }

    const QVariantList events = queryAnalyticsEvents(type, cameraId, text, limit);
    QSaveFile file(targetPath);
    if (!file.open(QIODevice::WriteOnly)) {
        result["path"] = targetPath;
        result["message"] = file.errorString();
        return result;
    }

    if (normalizedFormat == QStringLiteral("json")) {
        const QJsonDocument doc(QJsonArray::fromVariantList(events));
        file.write(doc.toJson(QJsonDocument::Indented));
    } else {
        auto csvCell = [](const QString &value) {
            QString cell = value;
            cell.replace(QStringLiteral("\""), QStringLiteral("\"\""));
            if (cell.contains(QLatin1Char(',')) || cell.contains(QLatin1Char('"'))
                || cell.contains(QLatin1Char('\n')) || cell.contains(QLatin1Char('\r'))) {
                cell = QStringLiteral("\"%1\"").arg(cell);
            }
            return cell;
        };

        QString csv;
        csv += QStringLiteral("timestamp,cameraId,eventType,moduleType,label,confidence,ruleName,message,snapshotPath,clipPath\n");
        for (const QVariant &eventVar : events) {
            const QVariantMap event = eventVar.toMap();
            const QStringList row{
                event.value("timestampText").toString(),
                event.value("cameraId").toString(),
                event.value("eventType").toString(),
                event.value("moduleType").toString(),
                event.value("label").toString(),
                event.value("confidence").toString(),
                event.value("ruleName").toString(),
                event.value("message").toString(),
                event.value("snapshotPath").toString(),
                event.value("clipPath").toString()
            };

            QStringList escaped;
            for (const QString &cell : row) {
                escaped.append(csvCell(cell));
            }
            csv += escaped.join(QLatin1Char(',')) + QLatin1Char('\n');
        }
        file.write(csv.toUtf8());
    }

    if (!file.commit()) {
        result["path"] = targetPath;
        result["message"] = file.errorString();
        return result;
    }

    result["ok"] = true;
    result["path"] = targetPath;
    result["format"] = normalizedFormat;
    result["count"] = events.size();
    result["message"] = QStringLiteral("Exported %1 analytics event(s)").arg(events.size());
    return result;
}
