#pragma once

#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include <functional>

class CameraModel;

class IncidentManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList events READ events NOTIFY eventsChanged)
    Q_PROPERTY(QVariantList correlations READ correlations NOTIFY correlationsChanged)
    Q_PROPERTY(QVariantMap summary READ summary NOTIFY summaryChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    using LocationResolver = std::function<QVariantMap(const QString &cameraId,
                                                        const QString &cameraIp)>;

    explicit IncidentManager(CameraModel *cameraModel, QObject *parent = nullptr);

    QVariantList events() const { return m_events; }
    QVariantList correlations() const { return m_correlations; }
    QVariantMap summary() const;
    QString lastError() const { return m_lastError; }

    void setLocationResolver(LocationResolver resolver);

    Q_INVOKABLE QVariantMap ingestEvent(const QVariantMap &sourceEvent);
    Q_INVOKABLE QVariantMap ingestAnalyticsEvent(const QVariantMap &analyticsEvent);
    Q_INVOKABLE QVariantList ingestHealthRun(const QVariantMap &healthRun);
    Q_INVOKABLE QVariantMap ingestRecordingEvent(const QString &eventType,
                                                 const QString &cameraIp,
                                                 const QString &path,
                                                 const QString &message = QString(),
                                                 const QVariantMap &attributes = {});
    Q_INVOKABLE QVariantMap ingestAuditEvent(const QVariantMap &auditEvent);
    Q_INVOKABLE QVariantList filterEvents(const QVariantMap &filters = {}) const;
    Q_INVOKABLE QVariantMap eventById(const QString &eventId) const;
    Q_INVOKABLE void clear();

    QJsonObject toJson() const;
    void restoreJson(const QJsonObject &object);

signals:
    void eventsChanged();
    void correlationsChanged();
    void summaryChanged();
    void lastErrorChanged();
    void stateChanged();
    void eventIngested(const QVariantMap &event);

private:
    static constexpr int maximumEvents = 5000;
    static constexpr qint64 correlationWindowMs = 5 * 60 * 1000;

    QVariantMap normalizeEvent(const QVariantMap &sourceEvent) const;
    QString cameraIdForIp(const QString &cameraIp) const;
    QVariantMap resolveLocation(const QString &cameraId, const QString &cameraIp) const;
    void rebuildIndexes();
    void rebuildCorrelations();
    void setLastError(const QString &message);

    static QVariant sanitizeVariant(const QVariant &value, int depth = 0);
    static QString normalizedToken(const QString &value, int maximumLength = 120);
    static QString normalizedSeverity(const QString &value);
    static int severityRank(const QString &severity);
    static qint64 eventTimeMs(const QVariantMap &event, qint64 fallback);
    static QString eventFingerprint(const QVariantMap &event);

    CameraModel *m_cameraModel = nullptr;
    LocationResolver m_locationResolver;
    QVariantList m_events;
    QVariantList m_correlations;
    QSet<QString> m_eventIds;
    QSet<QString> m_sourceEventIds;
    QString m_lastError;
};
