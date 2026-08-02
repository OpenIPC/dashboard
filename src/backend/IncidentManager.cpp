#include "IncidentManager.h"

#include "CameraModel.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QJsonArray>
#include <QMetaType>
#include <QUuid>

#include <algorithm>

namespace {

QString isoUtc(qint64 timestampMs)
{
    return QDateTime::fromMSecsSinceEpoch(timestampMs, Qt::UTC).toString(Qt::ISODateWithMs);
}

QString sourceDedupKey(const QVariantMap &event)
{
    const QString sourceEventId = event.value(QStringLiteral("sourceEventId")).toString();
    if (sourceEventId.isEmpty()) return {};
    return event.value(QStringLiteral("source")).toString() + QLatin1Char('|') + sourceEventId;
}

QString firstNonEmpty(const QStringList &values)
{
    for (const QString &value : values) {
        const QString trimmed = value.trimmed();
        if (!trimmed.isEmpty()) return trimmed;
    }
    return {};
}

bool containsSensitiveKey(const QString &key)
{
    QString normalized = key.toLower();
    normalized.remove(QLatin1Char('_'));
    normalized.remove(QLatin1Char('-'));
    normalized.remove(QLatin1Char('.'));
    return normalized.contains(QStringLiteral("password"))
        || normalized.contains(QStringLiteral("passwd"))
        || normalized.contains(QStringLiteral("token"))
        || normalized.contains(QStringLiteral("secret"))
        || normalized.contains(QStringLiteral("privatekey"))
        || normalized.contains(QStringLiteral("apikey"))
        || normalized.contains(QStringLiteral("credential"))
        || normalized.contains(QStringLiteral("authorization"))
        || normalized.contains(QStringLiteral("cookie"))
        || normalized == QStringLiteral("psk");
}

} // namespace

IncidentManager::IncidentManager(CameraModel *cameraModel, QObject *parent)
    : QObject(parent)
    , m_cameraModel(cameraModel)
{
}

void IncidentManager::setLocationResolver(LocationResolver resolver)
{
    m_locationResolver = std::move(resolver);
}

QVariantMap IncidentManager::summary() const
{
    QVariantMap severityCounts{
        {QStringLiteral("info"), 0},
        {QStringLiteral("warning"), 0},
        {QStringLiteral("error"), 0},
        {QStringLiteral("critical"), 0}
    };
    QVariantMap sourceCounts;
    for (const QVariant &value : m_events) {
        const QVariantMap event = value.toMap();
        const QString severity = event.value(QStringLiteral("severity")).toString();
        severityCounts.insert(severity, severityCounts.value(severity).toInt() + 1);
        const QString source = event.value(QStringLiteral("source")).toString();
        sourceCounts.insert(source, sourceCounts.value(source).toInt() + 1);
    }
    return {
        {QStringLiteral("schemaVersion"), 1},
        {QStringLiteral("eventCount"), m_events.size()},
        {QStringLiteral("correlationCount"), m_correlations.size()},
        {QStringLiteral("severityCounts"), severityCounts},
        {QStringLiteral("sourceCounts"), sourceCounts},
        {QStringLiteral("latestOccurredAt"), m_events.isEmpty()
             ? QString() : m_events.first().toMap().value(QStringLiteral("occurredAt"))}
    };
}

QVariantMap IncidentManager::ingestEvent(const QVariantMap &sourceEvent)
{
    const QVariantMap normalized = normalizeEvent(sourceEvent);
    if (normalized.isEmpty()) {
        setLastError(QStringLiteral("Event could not be normalized"));
        return {};
    }

    const QString eventId = normalized.value(QStringLiteral("id")).toString();
    const QString dedupKey = sourceDedupKey(normalized);
    if (m_eventIds.contains(eventId) || (!dedupKey.isEmpty() && m_sourceEventIds.contains(dedupKey))) {
        for (const QVariant &value : m_events) {
            const QVariantMap existing = value.toMap();
            if (existing.value(QStringLiteral("id")).toString() == eventId
                || (!dedupKey.isEmpty() && sourceDedupKey(existing) == dedupKey)) {
                return existing;
            }
        }
        return {};
    }

    m_events.prepend(normalized);
    while (m_events.size() > maximumEvents) m_events.removeLast();
    rebuildIndexes();
    rebuildCorrelations();
    setLastError({});
    emit eventsChanged();
    emit correlationsChanged();
    emit summaryChanged();
    emit stateChanged();
    emit eventIngested(normalized);
    return normalized;
}

QVariantMap IncidentManager::ingestAnalyticsEvent(const QVariantMap &analyticsEvent)
{
    QVariantMap attributes = analyticsEvent;
    const QString eventType = firstNonEmpty({analyticsEvent.value(QStringLiteral("eventType")).toString(),
                                             QStringLiteral("detection")});
    const QString title = firstNonEmpty({analyticsEvent.value(QStringLiteral("ruleName")).toString(),
                                         analyticsEvent.value(QStringLiteral("label")).toString(),
                                         analyticsEvent.value(QStringLiteral("moduleId")).toString(),
                                         QStringLiteral("Analytics event")});
    QVariantList evidence;
    const QString snapshotPath = analyticsEvent.value(QStringLiteral("snapshotPath")).toString();
    const QString clipPath = analyticsEvent.value(QStringLiteral("clipPath")).toString();
    if (!snapshotPath.isEmpty()) {
        evidence.append(QVariantMap{{QStringLiteral("kind"), QStringLiteral("snapshot")},
                                    {QStringLiteral("path"), snapshotPath}});
    }
    if (!clipPath.isEmpty()) {
        evidence.append(QVariantMap{{QStringLiteral("kind"), QStringLiteral("recording")},
                                    {QStringLiteral("path"), clipPath}});
    }
    return ingestEvent({
        {QStringLiteral("source"), QStringLiteral("analytics")},
        {QStringLiteral("sourceEventId"), analyticsEvent.value(QStringLiteral("id"))},
        {QStringLiteral("category"), QStringLiteral("analytics")},
        {QStringLiteral("type"), eventType},
        {QStringLiteral("severity"), analyticsEvent.value(QStringLiteral("severity"),
                                                            analyticsEvent.value(QStringLiteral("actionNotify")).toBool()
                                                                ? QStringLiteral("warning")
                                                                : QStringLiteral("info"))},
        {QStringLiteral("occurredAtMs"), analyticsEvent.value(QStringLiteral("timestampMs"))},
        {QStringLiteral("cameraId"), analyticsEvent.value(QStringLiteral("cameraId"))},
        {QStringLiteral("cameraIp"), analyticsEvent.value(QStringLiteral("cameraIp"))},
        {QStringLiteral("title"), title},
        {QStringLiteral("message"), analyticsEvent.value(QStringLiteral("message"), title)},
        {QStringLiteral("evidence"), evidence},
        {QStringLiteral("attributes"), attributes}
    });
}

QVariantList IncidentManager::ingestHealthRun(const QVariantMap &healthRun)
{
    QVariantList ingested;
    const QString runId = healthRun.value(QStringLiteral("id")).toString();
    const QVariantList cameras = healthRun.value(QStringLiteral("cameras")).toList();
    for (const QVariant &value : cameras) {
        const QVariantMap camera = value.toMap();
        const QString status = camera.value(QStringLiteral("status")).toString().toLower();
        const QString cameraIp = camera.value(QStringLiteral("ip")).toString();
        const QString cameraName = firstNonEmpty({camera.value(QStringLiteral("name")).toString(),
                                                  cameraIp,
                                                  QStringLiteral("Camera health")});
        const QVariantMap event = ingestEvent({
            {QStringLiteral("source"), QStringLiteral("health")},
            {QStringLiteral("sourceEventId"), runId + QLatin1Char(':') + cameraIp},
            {QStringLiteral("category"), QStringLiteral("health")},
            {QStringLiteral("type"), QStringLiteral("camera-health")},
            {QStringLiteral("severity"), status == QStringLiteral("error")
                 ? QStringLiteral("error")
                 : (status == QStringLiteral("warning") ? QStringLiteral("warning")
                                                          : QStringLiteral("info"))},
            {QStringLiteral("occurredAt"), healthRun.value(QStringLiteral("completedAt"))},
            {QStringLiteral("cameraId"), camera.value(QStringLiteral("cameraId"))},
            {QStringLiteral("cameraIp"), cameraIp},
            {QStringLiteral("title"), cameraName},
            {QStringLiteral("message"), firstNonEmpty({camera.value(QStringLiteral("recommendation")).toString(),
                                                        healthRun.value(QStringLiteral("summary")).toString(),
                                                        QStringLiteral("Health run completed")})},
            {QStringLiteral("evidence"), QVariantList{QVariantMap{
                 {QStringLiteral("kind"), QStringLiteral("health-run")},
                 {QStringLiteral("id"), runId}}}},
            {QStringLiteral("attributes"), QVariantMap{
                 {QStringLiteral("runId"), runId},
                 {QStringLiteral("profile"), healthRun.value(QStringLiteral("profile"))},
                 {QStringLiteral("status"), status},
                 {QStringLiteral("probes"), camera.value(QStringLiteral("probes"))},
                 {QStringLiteral("temperatureC"), camera.value(QStringLiteral("temperatureC"))},
                 {QStringLiteral("firmwareVersion"), camera.value(QStringLiteral("firmwareVersion"))},
                 {QStringLiteral("majesticVersion"), camera.value(QStringLiteral("majesticVersion"))}}}
        });
        if (!event.isEmpty()) ingested.append(event);
    }

    if (cameras.isEmpty()) {
        const QVariantMap event = ingestEvent({
            {QStringLiteral("source"), QStringLiteral("health")},
            {QStringLiteral("sourceEventId"), runId},
            {QStringLiteral("category"), QStringLiteral("health")},
            {QStringLiteral("type"), QStringLiteral("health-run")},
            {QStringLiteral("severity"), healthRun.value(QStringLiteral("errorCount")).toInt() > 0
                 ? QStringLiteral("error")
                 : (healthRun.value(QStringLiteral("warningCount")).toInt() > 0
                        ? QStringLiteral("warning") : QStringLiteral("info"))},
            {QStringLiteral("occurredAt"), healthRun.value(QStringLiteral("completedAt"))},
            {QStringLiteral("title"), QStringLiteral("Health run")},
            {QStringLiteral("message"), healthRun.value(QStringLiteral("summary"))},
            {QStringLiteral("attributes"), healthRun}
        });
        if (!event.isEmpty()) ingested.append(event);
    }
    return ingested;
}

QVariantMap IncidentManager::ingestRecordingEvent(const QString &eventType,
                                                   const QString &cameraIp,
                                                   const QString &path,
                                                   const QString &message,
                                                   const QVariantMap &attributes)
{
    const QString type = normalizedToken(eventType.isEmpty() ? QStringLiteral("recording-event")
                                                              : eventType, 80);
    QVariantMap safeAttributes = attributes;
    safeAttributes.insert(QStringLiteral("path"), path);
    return ingestEvent({
        {QStringLiteral("source"), QStringLiteral("recording")},
        {QStringLiteral("category"), QStringLiteral("recording")},
        {QStringLiteral("type"), type},
        {QStringLiteral("severity"), type.contains(QStringLiteral("error"))
             || type.contains(QStringLiteral("failed")) ? QStringLiteral("error")
                                                         : QStringLiteral("info")},
        {QStringLiteral("cameraIp"), cameraIp},
        {QStringLiteral("title"), QStringLiteral("Recording %1").arg(type)},
        {QStringLiteral("message"), message},
        {QStringLiteral("evidence"), path.isEmpty() ? QVariantList{} : QVariantList{QVariantMap{
             {QStringLiteral("kind"), QStringLiteral("recording")},
             {QStringLiteral("path"), path}}}},
        {QStringLiteral("attributes"), safeAttributes}
    });
}

QVariantMap IncidentManager::ingestAuditEvent(const QVariantMap &auditEvent)
{
    const QString action = firstNonEmpty({auditEvent.value(QStringLiteral("action")).toString(),
                                          QStringLiteral("operation")});
    const QString outcome = auditEvent.value(QStringLiteral("outcome")).toString().toLower();
    return ingestEvent({
        {QStringLiteral("source"), QStringLiteral("audit")},
        {QStringLiteral("sourceEventId"), auditEvent.value(QStringLiteral("id"))},
        {QStringLiteral("category"), QStringLiteral("device-operation")},
        {QStringLiteral("type"), action},
        {QStringLiteral("severity"), outcome == QStringLiteral("failed")
             ? QStringLiteral("error") : QStringLiteral("info")},
        {QStringLiteral("occurredAt"), auditEvent.value(QStringLiteral("occurredAt"))},
        {QStringLiteral("cameraId"), auditEvent.value(QStringLiteral("cameraId"))},
        {QStringLiteral("cameraIp"), auditEvent.value(QStringLiteral("cameraIp"))},
        {QStringLiteral("siteId"), auditEvent.value(QStringLiteral("siteId"))},
        {QStringLiteral("areaId"), auditEvent.value(QStringLiteral("areaId"))},
        {QStringLiteral("actor"), auditEvent.value(QStringLiteral("actor"))},
        {QStringLiteral("title"), action},
        {QStringLiteral("message"), auditEvent.value(QStringLiteral("detail"))},
        {QStringLiteral("attributes"), auditEvent}
    });
}

QVariantList IncidentManager::filterEvents(const QVariantMap &filters) const
{
    const QString cameraId = filters.value(QStringLiteral("cameraId")).toString().trimmed();
    const QString siteId = filters.value(QStringLiteral("siteId")).toString().trimmed();
    const QString areaId = filters.value(QStringLiteral("areaId")).toString().trimmed();
    const QString source = filters.value(QStringLiteral("source")).toString().trimmed().toLower();
    const QString severity = filters.value(QStringLiteral("severity")).toString().trimmed().toLower();
    const QString search = filters.value(QStringLiteral("search")).toString().trimmed();
    const qint64 fromMs = filters.value(QStringLiteral("fromMs"), 0).toLongLong();
    const qint64 toMs = filters.value(QStringLiteral("toMs"), 0).toLongLong();
    const int limit = qBound(1, filters.value(QStringLiteral("limit"), 500).toInt(), maximumEvents);
    QVariantList result;
    for (const QVariant &value : m_events) {
        const QVariantMap event = value.toMap();
        const qint64 occurredAtMs = event.value(QStringLiteral("occurredAtMs")).toLongLong();
        if (!cameraId.isEmpty() && event.value(QStringLiteral("cameraId")).toString() != cameraId) continue;
        if (!siteId.isEmpty() && event.value(QStringLiteral("siteId")).toString() != siteId) continue;
        if (!areaId.isEmpty() && event.value(QStringLiteral("areaId")).toString() != areaId) continue;
        if (!source.isEmpty() && event.value(QStringLiteral("source")).toString() != source) continue;
        if (!severity.isEmpty() && event.value(QStringLiteral("severity")).toString() != severity) continue;
        if (fromMs > 0 && occurredAtMs < fromMs) continue;
        if (toMs > 0 && occurredAtMs > toMs) continue;
        if (!search.isEmpty()) {
            const QString haystack = QStringList{
                event.value(QStringLiteral("title")).toString(),
                event.value(QStringLiteral("message")).toString(),
                event.value(QStringLiteral("type")).toString(),
                event.value(QStringLiteral("cameraId")).toString(),
                event.value(QStringLiteral("cameraIp")).toString()
            }.join(QLatin1Char(' '));
            if (!haystack.contains(search, Qt::CaseInsensitive)) continue;
        }
        result.append(event);
        if (result.size() >= limit) break;
    }
    return result;
}

QVariantMap IncidentManager::eventById(const QString &eventId) const
{
    const QString id = eventId.trimmed();
    for (const QVariant &value : m_events) {
        const QVariantMap event = value.toMap();
        if (event.value(QStringLiteral("id")).toString() == id) return event;
    }
    return {};
}

void IncidentManager::clear()
{
    if (m_events.isEmpty()) return;
    m_events.clear();
    m_correlations.clear();
    m_eventIds.clear();
    m_sourceEventIds.clear();
    setLastError({});
    emit eventsChanged();
    emit correlationsChanged();
    emit summaryChanged();
    emit stateChanged();
}

QJsonObject IncidentManager::toJson() const
{
    return QJsonObject{
        {QStringLiteral("schemaVersion"), 1},
        {QStringLiteral("events"), QJsonArray::fromVariantList(m_events)}
    };
}

void IncidentManager::restoreJson(const QJsonObject &object)
{
    QVariantList restored;
    QSet<QString> eventIds;
    QSet<QString> sourceIds;
    const QVariantList values = object.value(QStringLiteral("events")).toArray().toVariantList();
    for (const QVariant &value : values) {
        const QVariantMap event = normalizeEvent(value.toMap());
        if (event.isEmpty()) continue;
        const QString eventId = event.value(QStringLiteral("id")).toString();
        const QString sourceId = sourceDedupKey(event);
        if (eventIds.contains(eventId) || (!sourceId.isEmpty() && sourceIds.contains(sourceId))) continue;
        eventIds.insert(eventId);
        if (!sourceId.isEmpty()) sourceIds.insert(sourceId);
        restored.append(event);
        if (restored.size() >= maximumEvents) break;
    }
    std::sort(restored.begin(), restored.end(), [](const QVariant &left, const QVariant &right) {
        return left.toMap().value(QStringLiteral("occurredAtMs")).toLongLong()
            > right.toMap().value(QStringLiteral("occurredAtMs")).toLongLong();
    });
    m_events = restored;
    rebuildIndexes();
    rebuildCorrelations();
    setLastError({});
    emit eventsChanged();
    emit correlationsChanged();
    emit summaryChanged();
}

QVariantMap IncidentManager::normalizeEvent(const QVariantMap &sourceEvent) const
{
    if (sourceEvent.isEmpty()) return {};
    const qint64 nowMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    const qint64 occurredAtMs = eventTimeMs(sourceEvent, nowMs);
    QString receivedAt = isoUtc(nowMs);
    if (sourceEvent.value(QStringLiteral("schemaVersion")).toInt() == 1) {
        const QDateTime restoredReceivedAt = QDateTime::fromString(
            sourceEvent.value(QStringLiteral("receivedAt")).toString(), Qt::ISODateWithMs);
        if (restoredReceivedAt.isValid()) receivedAt = isoUtc(restoredReceivedAt.toMSecsSinceEpoch());
    }
    const QString source = normalizedToken(sourceEvent.value(QStringLiteral("source"),
                                                              QStringLiteral("system")).toString(), 80);
    const QString type = normalizedToken(sourceEvent.value(QStringLiteral("type"),
                                                            QStringLiteral("event")).toString(), 120);
    const QString category = normalizedToken(sourceEvent.value(QStringLiteral("category"), source).toString(), 80);
    QString cameraId = sourceEvent.value(QStringLiteral("cameraId")).toString().trimmed().left(200);
    const QString cameraIp = sourceEvent.value(QStringLiteral("cameraIp")).toString().trimmed().left(120);
    if (cameraId.isEmpty() && !cameraIp.isEmpty()) cameraId = cameraIdForIp(cameraIp);
    const QVariantMap location = resolveLocation(cameraId, cameraIp);
    const QString siteId = firstNonEmpty({sourceEvent.value(QStringLiteral("siteId")).toString(),
                                          location.value(QStringLiteral("siteId")).toString()}).left(200);
    const QString areaId = firstNonEmpty({sourceEvent.value(QStringLiteral("areaId")).toString(),
                                          location.value(QStringLiteral("areaId")).toString()}).left(200);
    QString eventId = sourceEvent.value(QStringLiteral("normalizedEventId")).toString().trimmed();
    if (eventId.isEmpty() && sourceEvent.value(QStringLiteral("schemaVersion")).toInt() == 1) {
        eventId = sourceEvent.value(QStringLiteral("id")).toString().trimmed();
    }
    if (eventId.isEmpty()) eventId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    eventId = eventId.left(200);

    QVariantMap event{
        {QStringLiteral("schemaVersion"), 1},
        {QStringLiteral("id"), eventId},
        {QStringLiteral("source"), source.isEmpty() ? QStringLiteral("system") : source},
        {QStringLiteral("sourceEventId"), sourceEvent.value(QStringLiteral("sourceEventId")).toString().left(240)},
        {QStringLiteral("category"), category.isEmpty() ? QStringLiteral("system") : category},
        {QStringLiteral("type"), type.isEmpty() ? QStringLiteral("event") : type},
        {QStringLiteral("severity"), normalizedSeverity(sourceEvent.value(QStringLiteral("severity")).toString())},
        {QStringLiteral("occurredAtMs"), occurredAtMs},
        {QStringLiteral("occurredAt"), isoUtc(occurredAtMs)},
        {QStringLiteral("receivedAt"), receivedAt},
        {QStringLiteral("cameraId"), cameraId},
        {QStringLiteral("cameraIp"), cameraIp},
        {QStringLiteral("siteId"), siteId},
        {QStringLiteral("areaId"), areaId},
        {QStringLiteral("actor"), sourceEvent.value(QStringLiteral("actor")).toString().left(160)},
        {QStringLiteral("title"), sourceEvent.value(QStringLiteral("title"), type).toString().trimmed().left(240)},
        {QStringLiteral("message"), sourceEvent.value(QStringLiteral("message")).toString().trimmed().left(2000)},
        {QStringLiteral("evidence"), sanitizeVariant(sourceEvent.value(QStringLiteral("evidence"))).toList()},
        {QStringLiteral("attributes"), sanitizeVariant(sourceEvent.value(QStringLiteral("attributes"))).toMap()}
    };

    QString correlationKey = sourceEvent.value(QStringLiteral("correlationKey")).toString().trimmed().left(300);
    if (correlationKey.isEmpty()) {
        const QString scope = !cameraId.isEmpty() ? QStringLiteral("camera:") + cameraId
                              : (!siteId.isEmpty() ? QStringLiteral("site:") + siteId
                                                   : QStringLiteral("global"));
        correlationKey = QStringLiteral("%1|%2|%3|%4")
            .arg(scope,
                 event.value(QStringLiteral("category")).toString(),
                 event.value(QStringLiteral("type")).toString(),
                 QString::number(occurredAtMs / correlationWindowMs));
    }
    event.insert(QStringLiteral("correlationKey"), correlationKey);
    event.insert(QStringLiteral("fingerprint"), eventFingerprint(event));
    return event;
}

QString IncidentManager::cameraIdForIp(const QString &cameraIp) const
{
    if (!m_cameraModel) return {};
    const int index = m_cameraModel->findIndexByIp(cameraIp.trimmed());
    if (index < 0) return {};
    const Camera camera = m_cameraModel->getCamera(index);
    return camera.id.trimmed().isEmpty() ? camera.ip.trimmed() : camera.id.trimmed();
}

QVariantMap IncidentManager::resolveLocation(const QString &cameraId, const QString &cameraIp) const
{
    if (!m_locationResolver) return {};
    return sanitizeVariant(m_locationResolver(cameraId, cameraIp)).toMap();
}

void IncidentManager::rebuildIndexes()
{
    m_eventIds.clear();
    m_sourceEventIds.clear();
    for (const QVariant &value : m_events) {
        const QVariantMap event = value.toMap();
        m_eventIds.insert(event.value(QStringLiteral("id")).toString());
        const QString dedupKey = sourceDedupKey(event);
        if (!dedupKey.isEmpty()) m_sourceEventIds.insert(dedupKey);
    }
}

void IncidentManager::rebuildCorrelations()
{
    QHash<QString, QVariantMap> groups;
    QStringList order;
    for (const QVariant &value : m_events) {
        const QVariantMap event = value.toMap();
        const QString key = event.value(QStringLiteral("correlationKey")).toString();
        QVariantMap group = groups.value(key);
        if (group.isEmpty()) {
            group = {
                {QStringLiteral("key"), key},
                {QStringLiteral("cameraId"), event.value(QStringLiteral("cameraId"))},
                {QStringLiteral("cameraIp"), event.value(QStringLiteral("cameraIp"))},
                {QStringLiteral("siteId"), event.value(QStringLiteral("siteId"))},
                {QStringLiteral("areaId"), event.value(QStringLiteral("areaId"))},
                {QStringLiteral("category"), event.value(QStringLiteral("category"))},
                {QStringLiteral("type"), event.value(QStringLiteral("type"))},
                {QStringLiteral("severity"), event.value(QStringLiteral("severity"))},
                {QStringLiteral("firstOccurredAt"), event.value(QStringLiteral("occurredAt"))},
                {QStringLiteral("lastOccurredAt"), event.value(QStringLiteral("occurredAt"))},
                {QStringLiteral("count"), 0},
                {QStringLiteral("eventIds"), QVariantList{}},
                {QStringLiteral("sources"), QStringList{}}
            };
            order.append(key);
        }
        group.insert(QStringLiteral("count"), group.value(QStringLiteral("count")).toInt() + 1);
        QVariantList eventIds = group.value(QStringLiteral("eventIds")).toList();
        eventIds.append(event.value(QStringLiteral("id")));
        group.insert(QStringLiteral("eventIds"), eventIds);
        QStringList sources = group.value(QStringLiteral("sources")).toStringList();
        const QString source = event.value(QStringLiteral("source")).toString();
        if (!sources.contains(source)) sources.append(source);
        group.insert(QStringLiteral("sources"), sources);
        if (severityRank(event.value(QStringLiteral("severity")).toString())
            > severityRank(group.value(QStringLiteral("severity")).toString())) {
            group.insert(QStringLiteral("severity"), event.value(QStringLiteral("severity")));
        }
        group.insert(QStringLiteral("firstOccurredAt"), event.value(QStringLiteral("occurredAt")));
        groups.insert(key, group);
    }

    QVariantList correlations;
    for (const QString &key : order) correlations.append(groups.value(key));
    m_correlations = correlations;
}

void IncidentManager::setLastError(const QString &message)
{
    const QString normalized = message.trimmed().left(500);
    if (m_lastError == normalized) return;
    m_lastError = normalized;
    emit lastErrorChanged();
}

QVariant IncidentManager::sanitizeVariant(const QVariant &value, int depth)
{
    if (!value.isValid() || value.isNull()) return {};
    if (depth >= 6) return QStringLiteral("[truncated]");
    if (value.typeId() == QMetaType::QVariantMap) {
        QVariantMap result;
        int count = 0;
        const QVariantMap source = value.toMap();
        for (auto it = source.cbegin(); it != source.cend() && count < 64; ++it, ++count) {
            result.insert(it.key().left(120), containsSensitiveKey(it.key())
                ? QVariant(QStringLiteral("[redacted]")) : sanitizeVariant(it.value(), depth + 1));
        }
        return result;
    }
    if (value.typeId() == QMetaType::QVariantList || value.typeId() == QMetaType::QStringList) {
        QVariantList result;
        const QVariantList source = value.toList();
        const int count = qMin(source.size(), 64);
        for (int index = 0; index < count; ++index) result.append(sanitizeVariant(source.at(index), depth + 1));
        return result;
    }
    if (value.typeId() == QMetaType::QString || value.typeId() == QMetaType::QByteArray
        || value.typeId() == QMetaType::QUrl || value.typeId() == QMetaType::QDateTime) {
        return value.toString().left(2000);
    }
    return value;
}

QString IncidentManager::normalizedToken(const QString &value, int maximumLength)
{
    QString token = value.trimmed().toLower().left(maximumLength);
    for (qsizetype index = 0; index < token.size(); ++index) {
        QChar character = token.at(index);
        if (!character.isLetterOrNumber() && character != QLatin1Char('-')
            && character != QLatin1Char('_') && character != QLatin1Char('.')) {
            token[index] = QLatin1Char('-');
        }
    }
    while (token.contains(QStringLiteral("--"))) token.replace(QStringLiteral("--"), QStringLiteral("-"));
    return token;
}

QString IncidentManager::normalizedSeverity(const QString &value)
{
    const QString severity = value.trimmed().toLower();
    if (severity == QStringLiteral("critical") || severity == QStringLiteral("fatal"))
        return QStringLiteral("critical");
    if (severity == QStringLiteral("error") || severity == QStringLiteral("high"))
        return QStringLiteral("error");
    if (severity == QStringLiteral("warning") || severity == QStringLiteral("warn")
        || severity == QStringLiteral("medium")) return QStringLiteral("warning");
    return QStringLiteral("info");
}

int IncidentManager::severityRank(const QString &severity)
{
    if (severity == QStringLiteral("critical")) return 4;
    if (severity == QStringLiteral("error")) return 3;
    if (severity == QStringLiteral("warning")) return 2;
    return 1;
}

qint64 IncidentManager::eventTimeMs(const QVariantMap &event, qint64 fallback)
{
    bool ok = false;
    const qint64 direct = event.value(QStringLiteral("occurredAtMs")).toLongLong(&ok);
    if (ok && direct > 0) return direct;
    const QVariant timestamp = event.value(QStringLiteral("timestampMs"));
    const qint64 timestampMs = timestamp.toLongLong(&ok);
    if (ok && timestampMs > 0) return timestampMs;
    const QString occurredAt = event.value(QStringLiteral("occurredAt")).toString();
    const QDateTime dateTime = QDateTime::fromString(occurredAt, Qt::ISODateWithMs);
    if (dateTime.isValid()) return dateTime.toUTC().toMSecsSinceEpoch();
    return fallback;
}

QString IncidentManager::eventFingerprint(const QVariantMap &event)
{
    const QByteArray input = QStringList{
        event.value(QStringLiteral("source")).toString(),
        event.value(QStringLiteral("category")).toString(),
        event.value(QStringLiteral("type")).toString(),
        event.value(QStringLiteral("cameraId")).toString(),
        event.value(QStringLiteral("cameraIp")).toString(),
        event.value(QStringLiteral("title")).toString(),
        event.value(QStringLiteral("message")).toString()
    }.join(QLatin1Char('|')).toUtf8();
    return QString::fromLatin1(QCryptographicHash::hash(input, QCryptographicHash::Sha256).toHex());
}
