#include "FleetManager.h"

#include "CameraHealthController.h"
#include "CameraModel.h"
#include "LogModel.h"
#include "MajesticClient.h"
#include "OpenIpcFirmwareClient.h"
#include "UserManager.h"

#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMap>
#include <QRegularExpression>
#include <QSaveFile>
#include <QTextStream>
#include <QTime>
#include <QTimer>
#include <QUrl>
#include <QUuid>

#include <algorithm>
#include <functional>

namespace {

constexpr int kMaximumSites = 256;
constexpr int kMaximumAreas = 2048;
constexpr int kMaximumSavedViews = 100;
constexpr int kMaximumBaselines = 100;
constexpr int kMaximumBatchHistory = 30;
constexpr qint64 kMaximumImportBytes = 8 * 1024 * 1024;

bool sensitiveKey(const QString &path)
{
    static const QRegularExpression expression(
        QStringLiteral("(^|[._/-])(password|passwd|pwd|secret|token|credential|private[_-]?key|psk|api[_-]?key)([._/-]|$)"),
        QRegularExpression::CaseInsensitiveOption);
    if (expression.match(path).hasMatch()) return true;
    QString segment = path;
    const int separator = std::max({segment.lastIndexOf(QLatin1Char('.')),
                                    segment.lastIndexOf(QLatin1Char('/')),
                                    segment.lastIndexOf(QLatin1Char('-'))});
    if (separator >= 0) segment = segment.mid(separator + 1);
    segment = segment.toLower();
    segment.remove(QRegularExpression(QStringLiteral("[^a-z0-9]")));
    if (segment == QStringLiteral("credentialsincluded")) return false;
    return segment.contains(QStringLiteral("password"))
        || segment.contains(QStringLiteral("passwd"))
        || segment == QStringLiteral("pwd")
        || segment.contains(QStringLiteral("secret"))
        || segment.contains(QStringLiteral("token"))
        || segment.contains(QStringLiteral("credential"))
        || segment.contains(QStringLiteral("privatekey"))
        || segment == QStringLiteral("psk")
        || segment.contains(QStringLiteral("apikey"));
}

QVariant sanitizeVariant(const QVariant &value, const QString &path)
{
    if (sensitiveKey(path)) return {};

    if (value.metaType().id() == QMetaType::QVariantMap) {
        QVariantMap result;
        const QVariantMap source = value.toMap();
        for (auto it = source.cbegin(); it != source.cend(); ++it) {
            const QString childPath = path.isEmpty() ? it.key() : path + QLatin1Char('.') + it.key();
            const QVariant sanitized = sanitizeVariant(it.value(), childPath);
            if (sanitized.isValid()) result.insert(it.key().left(120), sanitized);
        }
        return result;
    }
    if (value.metaType().id() == QMetaType::QVariantList
        || value.metaType().id() == QMetaType::QStringList) {
        QVariantList result;
        const QVariantList source = value.toList();
        const int limit = std::min(512, static_cast<int>(source.size()));
        for (int index = 0; index < limit; ++index) {
            const QVariant sanitized = sanitizeVariant(source.at(index), path);
            if (sanitized.isValid()) result.append(sanitized);
        }
        return result;
    }
    if (value.metaType().id() == QMetaType::QString) {
        return value.toString().left(4096);
    }
    return value;
}

bool containsSensitiveField(const QVariant &value, const QString &path = QString())
{
    if (sensitiveKey(path)) return true;
    if (value.metaType().id() == QMetaType::QVariantMap) {
        const QVariantMap map = value.toMap();
        for (auto it = map.cbegin(); it != map.cend(); ++it) {
            const QString childPath = path.isEmpty() ? it.key() : path + QLatin1Char('.') + it.key();
            if (containsSensitiveField(it.value(), childPath)) return true;
        }
    } else if (value.metaType().id() == QMetaType::QVariantList
               || value.metaType().id() == QMetaType::QStringList) {
        for (const QVariant &item : value.toList()) {
            if (containsSensitiveField(item, path)) return true;
        }
    }
    return false;
}

QVariantMap mergedConfiguration(const QVariantMap &current, const QVariantMap &patch)
{
    QVariantMap result = current;
    for (auto it = patch.cbegin(); it != patch.cend(); ++it) {
        if (it.value().metaType().id() == QMetaType::QVariantMap) {
            result.insert(it.key(), mergedConfiguration(result.value(it.key()).toMap(),
                                                        it.value().toMap()));
        } else {
            result.insert(it.key(), it.value());
        }
    }
    return result;
}

QString compactValue(const QVariant &value)
{
    if (!value.isValid() || value.isNull()) return QStringLiteral("—");
    if (value.metaType().id() == QMetaType::QVariantMap
        || value.metaType().id() == QMetaType::QVariantList
        || value.metaType().id() == QMetaType::QStringList) {
        return QString::fromUtf8(QJsonDocument::fromVariant(value)
                                     .toJson(QJsonDocument::Compact)).left(240);
    }
    return value.toString().left(240);
}

void flattenConfiguration(const QVariantMap &map, const QString &prefix,
                          QMap<QString, QVariant> *destination)
{
    if (!destination) return;
    for (auto it = map.cbegin(); it != map.cend(); ++it) {
        const QString path = prefix.isEmpty() ? it.key() : prefix + QLatin1Char('.') + it.key();
        if (it.value().metaType().id() == QMetaType::QVariantMap) {
            flattenConfiguration(it.value().toMap(), path, destination);
        } else {
            destination->insert(path, it.value());
        }
    }
}

bool variantEquivalent(const QVariant &left, const QVariant &right)
{
    return QJsonValue::fromVariant(left) == QJsonValue::fromVariant(right);
}

QString csvCell(QString value)
{
    value.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QLatin1Char('"') + value + QLatin1Char('"');
}

QVariantMap readJsonObject(const QString &path, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) *error = file.errorString();
        return {};
    }
    if (file.size() > kMaximumImportBytes) {
        if (error) *error = QStringLiteral("Site definition exceeds the 8 MiB limit");
        return {};
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error) *error = parseError.errorString();
        return {};
    }
    return document.object().toVariantMap();
}

bool writeJsonObject(const QString &path, const QVariantMap &object, QString *error)
{
    const QFileInfo info(path);
    if (!QDir().mkpath(info.absolutePath())) {
        if (error) *error = QStringLiteral("Unable to create destination directory");
        return false;
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error) *error = file.errorString();
        return false;
    }
    const QByteArray data = QJsonDocument::fromVariant(object).toJson(QJsonDocument::Indented);
    if (file.write(data) != data.size() || !file.commit()) {
        if (error) *error = file.errorString();
        return false;
    }
    return true;
}

QString statusLower(const QString &value)
{
    return value.trimmed().toLower();
}

bool onlineStatus(const QString &value)
{
    const QString status = statusLower(value);
    return status == QStringLiteral("online") || status == QStringLiteral("ok")
        || status == QStringLiteral("streaming") || status == QStringLiteral("connected");
}

QStringList stringList(const QVariant &value)
{
    QStringList result;
    for (const QVariant &item : value.toList()) result.append(item.toString());
    if (result.isEmpty() && value.canConvert<QStringList>()) result = value.toStringList();
    return result;
}

} // namespace

FleetManager::FleetManager(CameraModel *cameraModel,
                           CameraHealthController *healthController,
                           OpenIpcFirmwareClient *firmwareClient,
                           MajesticClient *majesticClient,
                           UserManager *userManager,
                           LogModel *logModel,
                           QObject *parent)
    : QObject(parent)
    , m_cameraModel(cameraModel)
    , m_healthController(healthController)
    , m_firmwareClient(firmwareClient)
    , m_majesticClient(majesticClient)
    , m_userManager(userManager)
    , m_logModel(logModel)
{
    m_batchState = {
        {QStringLiteral("running"), false},
        {QStringLiteral("progress"), 0.0},
        {QStringLiteral("total"), 0},
        {QStringLiteral("completed"), 0}
    };

    if (m_cameraModel) {
        const auto changed = [this]() {
            syncLastSeen();
            emit inventoryChanged();
        };
        connect(m_cameraModel, &QAbstractItemModel::rowsInserted, this, changed);
        connect(m_cameraModel, &QAbstractItemModel::rowsRemoved, this, changed);
        connect(m_cameraModel, &QAbstractItemModel::modelReset, this, changed);
        connect(m_cameraModel, &QAbstractItemModel::dataChanged, this, changed);
    }

    if (m_userManager) {
        connect(m_userManager, &UserManager::currentUserChanged, this, [this]() {
            emit topologyChanged();
            emit savedViewsChanged();
            emit baselinesChanged();
            emit inventoryChanged();
        });
    }

    if (m_healthController) {
        connect(m_healthController, &CameraHealthController::currentResultsChanged,
                this, &FleetManager::inventoryChanged);
        connect(m_healthController, &CameraHealthController::historyChanged,
                this, &FleetManager::inventoryChanged);
        connect(m_healthController, &CameraHealthController::telemetryUpdated,
                this, [this](const QString &) { emit inventoryChanged(); });
        connect(m_healthController, &CameraHealthController::runCompleted, this,
                [this](const QString &) {
            if (m_activeHealthCameraId.isEmpty()) return;
            const QString cameraId = m_activeHealthCameraId;
            m_activeHealthCameraId.clear();
            const int index = cameraIndexForKey(cameraId);
            const Camera camera = m_cameraModel ? m_cameraModel->getCamera(index) : Camera{};
            const QVariantMap health = m_healthController->resultForCamera(camera.ip);
            QVariantMap metadata = m_deviceMetadata.value(cameraId);
            metadata.insert(QStringLiteral("healthStatus"), health.value(QStringLiteral("status")));
            metadata.insert(QStringLiteral("healthDetail"), health.value(QStringLiteral("recommendation")));
            if (!health.value(QStringLiteral("firmwareVersion")).toString().isEmpty()) {
                metadata.insert(QStringLiteral("firmwareVersion"),
                                health.value(QStringLiteral("firmwareVersion")));
            }
            if (!health.value(QStringLiteral("majesticVersion")).toString().isEmpty()) {
                metadata.insert(QStringLiteral("majesticVersion"),
                                health.value(QStringLiteral("majesticVersion")));
            }
            metadata.insert(QStringLiteral("lastInventoryAt"),
                            QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
            m_deviceMetadata.insert(cameraId, metadata);
            touchState();
            completeBatchDevice(cameraId,
                                statusLower(health.value(QStringLiteral("status")).toString())
                                        == QStringLiteral("error")
                                    ? QStringLiteral("failed") : QStringLiteral("success"),
                                health.value(QStringLiteral("recommendation")).toString(), health);
        });
    }

    if (m_firmwareClient) {
        connect(m_firmwareClient, &OpenIpcFirmwareClient::statusLoaded, this,
                [this](const QString &requestId, const QVariantMap &status) {
            const auto it = m_pendingRequests.find(requestId);
            if (it == m_pendingRequests.end() || it->stage != QStringLiteral("inventory")) return;
            const PendingRequest pending = it.value();
            m_pendingRequests.erase(it);
            QVariantMap metadata = m_deviceMetadata.value(pending.cameraId);
            const QString version = firstVersionValue(status);
            if (!version.isEmpty()) metadata.insert(QStringLiteral("firmwareVersion"), version);
            metadata.insert(QStringLiteral("firmwareStatus"),
                            sanitizedConfiguration(status));
            metadata.insert(QStringLiteral("lastInventoryAt"),
                            QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
            m_deviceMetadata.insert(pending.cameraId, metadata);
            touchState();
            completeBatchDevice(pending.cameraId, QStringLiteral("success"),
                                QStringLiteral("Firmware inventory refreshed"),
                                {{QStringLiteral("firmwareVersion"), version}});
        });
        connect(m_firmwareClient, &OpenIpcFirmwareClient::operationFailed, this,
                [this](const QString &requestId, const QString &, const QString &message,
                       int httpStatus) {
            handleRequestFailure(requestId, message, httpStatus);
        });
    }

    if (m_majesticClient) {
        connect(m_majesticClient, &MajesticClient::configurationLoaded, this,
                [this](const QString &requestId, const QVariantMap &configuration,
                       const QVariantMap &schema, const QVariantList &,
                       const QVariantMap &capabilities) {
            handleConfigurationLoaded(requestId, configuration, schema, capabilities);
        });
        connect(m_majesticClient, &MajesticClient::backupSaved, this,
                [this](const QString &requestId, const QString &path) {
            const auto it = m_pendingRequests.find(requestId);
            if (it == m_pendingRequests.end() || it->stage != QStringLiteral("apply-backup")) return;
            const PendingRequest pending = it.value();
            m_pendingRequests.erase(it);
            if (m_batchState.value(QStringLiteral("cancelRequested")).toBool()) {
                completeBatchDevice(pending.cameraId, QStringLiteral("cancelled"),
                                    QStringLiteral("Cancelled after backup"));
                return;
            }
            const int index = cameraIndexForKey(pending.cameraId);
            const Camera camera = m_cameraModel ? m_cameraModel->getCamera(index) : Camera{};
            const QString applyId = m_majesticClient->applyConfiguration(
                camera.ip, camera.onvifPort > 0 ? camera.onvifPort : 80,
                camera.login, camera.password, pending.patch);
            PendingRequest next = pending;
            next.stage = QStringLiteral("apply-commit");
            next.backupPath = path;
            m_pendingRequests.insert(applyId, next);
        });
        connect(m_majesticClient, &MajesticClient::configurationApplied, this,
                [this](const QString &requestId) {
            const auto it = m_pendingRequests.find(requestId);
            if (it == m_pendingRequests.end() || it->stage != QStringLiteral("apply-commit")) return;
            const PendingRequest pending = it.value();
            m_pendingRequests.erase(it);
            m_deviceConfigurations.insert(pending.cameraId,
                                          sanitizedConfiguration(pending.configuration));
            touchState();
            completeBatchDevice(pending.cameraId, QStringLiteral("success"),
                                QStringLiteral("Baseline applied; backup retained"),
                                {{QStringLiteral("backupPath"), pending.backupPath},
                                 {QStringLiteral("changedSections"), pending.patch.size()}});
        });
        connect(m_majesticClient, &MajesticClient::operationFailed, this,
                [this](const QString &requestId, const QString &, const QString &message,
                       int httpStatus) {
            handleRequestFailure(requestId, message, httpStatus);
        });
    }
}

QString FleetManager::normalizedId(const QString &value)
{
    return value.trimmed().left(160);
}

QVariantList FleetManager::sites() const
{
    if (!m_userManager) return m_sites;
    if (!m_userManager->isLoggedIn()) return {};
    if (m_userManager->isAdmin()) return m_sites;
    const QStringList scopes = m_userManager->currentUser()
                                   .value(QStringLiteral("cameraScopes")).toStringList();
    if (scopes.isEmpty()) return m_sites;
    QSet<QString> allowed;
    for (const QString &scope : scopes) {
        if (scope.startsWith(QStringLiteral("site:"), Qt::CaseInsensitive))
            allowed.insert(scope.mid(5));
        if (scope.startsWith(QStringLiteral("area:"), Qt::CaseInsensitive)) {
            const QVariantMap area = areaById(scope.mid(5));
            if (!area.isEmpty()) allowed.insert(area.value(QStringLiteral("siteId")).toString());
        }
    }
    for (const QVariant &value : inventory())
        allowed.insert(value.toMap().value(QStringLiteral("siteId")).toString());
    QVariantList result;
    for (const QVariant &value : m_sites) {
        if (allowed.contains(value.toMap().value(QStringLiteral("id")).toString())) result.append(value);
    }
    return result;
}

QVariantList FleetManager::areas() const
{
    if (!m_userManager) return m_areas;
    if (!m_userManager->isLoggedIn()) return {};
    if (m_userManager->isAdmin()) return m_areas;
    const QStringList scopes = m_userManager->currentUser()
                                   .value(QStringLiteral("cameraScopes")).toStringList();
    if (scopes.isEmpty()) return m_areas;
    QSet<QString> allowedAreas;
    QSet<QString> allowedSites;
    for (const QString &scope : scopes) {
        if (scope.startsWith(QStringLiteral("site:"), Qt::CaseInsensitive))
            allowedSites.insert(scope.mid(5));
        else if (scope.startsWith(QStringLiteral("area:"), Qt::CaseInsensitive))
            allowedAreas.insert(scope.mid(5));
    }
    for (const QVariant &value : inventory()) {
        const QVariantMap item = value.toMap();
        allowedAreas.insert(item.value(QStringLiteral("areaId")).toString());
    }
    QVariantList result;
    for (const QVariant &value : m_areas) {
        const QVariantMap area = value.toMap();
        if (allowedAreas.contains(area.value(QStringLiteral("id")).toString())
            || allowedSites.contains(area.value(QStringLiteral("siteId")).toString())) {
            result.append(area);
        }
    }
    return result;
}

QVariantList FleetManager::savedViews() const
{
    if (!m_userManager) return m_savedViews;
    if (!m_userManager->isLoggedIn()) return {};
    if (m_userManager->isAdmin()) return m_savedViews;
    const QString actor = m_userManager->currentUser()
                              .value(QStringLiteral("username")).toString();
    QVariantList result;
    for (const QVariant &value : m_savedViews) {
        if (value.toMap().value(QStringLiteral("owner")).toString() == actor) result.append(value);
    }
    return result;
}

QVariantList FleetManager::baselines() const
{
    if (!m_userManager || !m_userManager->canSettings()) return {};
    if (m_userManager->isAdmin()) return m_baselines;
    const QVariantList visibleSites = sites();
    const QVariantList visibleAreas = areas();
    QSet<QString> siteIds;
    QSet<QString> areaIds;
    for (const QVariant &value : visibleSites)
        siteIds.insert(value.toMap().value(QStringLiteral("id")).toString());
    for (const QVariant &value : visibleAreas)
        areaIds.insert(value.toMap().value(QStringLiteral("id")).toString());
    QVariantList result;
    for (const QVariant &value : m_baselines) {
        const QVariantMap baseline = value.toMap();
        const QString siteId = baseline.value(QStringLiteral("siteId")).toString();
        const QString areaId = baseline.value(QStringLiteral("areaId")).toString();
        if ((!areaId.isEmpty() && areaIds.contains(areaId))
            || (areaId.isEmpty() && (siteId.isEmpty() || siteIds.contains(siteId)))) {
            result.append(value);
        }
    }
    return result;
}

QString FleetManager::newId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString FleetManager::localPath(const QString &pathOrUrl)
{
    const QUrl url(pathOrUrl);
    return url.isLocalFile() ? url.toLocalFile() : pathOrUrl.trimmed();
}

QStringList FleetManager::normalizedTags(const QStringList &tags)
{
    QStringList result;
    for (const QString &tagValue : tags) {
        const QString tag = tagValue.trimmed().left(80);
        if (!tag.isEmpty() && !result.contains(tag, Qt::CaseInsensitive)) result.append(tag);
        if (result.size() >= 64) break;
    }
    return result;
}

QVariantMap FleetManager::sanitizedConfiguration(const QVariantMap &configuration)
{
    return sanitizeVariant(configuration, QString()).toMap();
}

QVariantMap FleetManager::configurationPatch(const QVariantMap &current,
                                             const QVariantMap &expected)
{
    QVariantMap patch;
    for (auto it = expected.cbegin(); it != expected.cend(); ++it) {
        if (it.value().metaType().id() == QMetaType::QVariantMap) {
            const QVariantMap child = configurationPatch(current.value(it.key()).toMap(),
                                                         it.value().toMap());
            if (!child.isEmpty()) patch.insert(it.key(), child);
        } else if (!current.contains(it.key())
                   || !variantEquivalent(current.value(it.key()), it.value())) {
            patch.insert(it.key(), it.value());
        }
    }
    return patch;
}

QVariantList FleetManager::configurationDifferences(const QVariantMap &current,
                                                    const QVariantMap &expected)
{
    QMap<QString, QVariant> currentFlat;
    QMap<QString, QVariant> expectedFlat;
    flattenConfiguration(current, QString(), &currentFlat);
    flattenConfiguration(expected, QString(), &expectedFlat);
    QVariantList result;
    for (auto it = expectedFlat.cbegin(); it != expectedFlat.cend(); ++it) {
        const bool exists = currentFlat.contains(it.key());
        if (exists && variantEquivalent(currentFlat.value(it.key()), it.value())) continue;
        const bool redacted = sensitiveKey(it.key());
        result.append(QVariantMap{
            {QStringLiteral("path"), it.key()},
            {QStringLiteral("current"), redacted ? QStringLiteral("[redacted]")
                                                  : compactValue(currentFlat.value(it.key()))},
            {QStringLiteral("expected"), redacted ? QStringLiteral("[redacted]")
                                                   : compactValue(it.value())},
            {QStringLiteral("missing"), !exists}
        });
        if (result.size() >= 2048) break;
    }
    return result;
}

QString FleetManager::safeFilePart(const QString &value)
{
    QString result = value.trimmed();
    result.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9._-]+")), QStringLiteral("_"));
    return result.left(80).isEmpty() ? QStringLiteral("camera") : result.left(80);
}

QString FleetManager::firstVersionValue(const QVariantMap &map)
{
    const QStringList directKeys{QStringLiteral("version"), QStringLiteral("firmwareVersion"),
                                 QStringLiteral("firmware"), QStringLiteral("build")};
    for (const QString &key : directKeys) {
        const QString value = map.value(key).toString().trimmed();
        if (!value.isEmpty()) return value.left(120);
    }
    for (auto it = map.cbegin(); it != map.cend(); ++it) {
        if (it.value().metaType().id() != QMetaType::QVariantMap) continue;
        const QString nested = firstVersionValue(it.value().toMap());
        if (!nested.isEmpty()) return nested;
    }
    return {};
}

bool FleetManager::canManageTopology() const
{
    return m_userManager && m_userManager->isAdmin();
}

int FleetManager::cameraIndexForKey(const QString &cameraId) const
{
    if (!m_cameraModel) return -1;
    const QString key = normalizedId(cameraId);
    for (int index = 0; index < m_cameraModel->rowCount(); ++index) {
        const Camera camera = m_cameraModel->getCamera(index);
        if (key == cameraKeyAt(index) || key == camera.id || key == camera.ip
            || key == QStringLiteral("camera:") + camera.id
            || key == QStringLiteral("ip:") + camera.ip) {
            return index;
        }
    }
    return -1;
}

QString FleetManager::cameraKeyAt(int index) const
{
    if (!m_cameraModel || index < 0 || index >= m_cameraModel->rowCount()) return {};
    const Camera camera = m_cameraModel->getCamera(index);
    return camera.id.trimmed().isEmpty() ? QStringLiteral("ip:") + camera.ip.trimmed()
                                         : camera.id.trimmed();
}

bool FleetManager::canConfigureCamera(const QString &cameraId) const
{
    if (!m_userManager || !m_userManager->canSettings()) return false;
    const int index = cameraIndexForKey(cameraId);
    if (index < 0) return false;
    const Camera camera = m_cameraModel->getCamera(index);
    return m_userManager->canAccessCamera(camera.id, camera.ip, index);
}

QVariantMap FleetManager::siteById(const QString &siteId) const
{
    const QString id = normalizedId(siteId);
    for (const QVariant &value : m_sites) {
        const QVariantMap site = value.toMap();
        if (site.value(QStringLiteral("id")).toString() == id) return site;
    }
    return {};
}

QVariantMap FleetManager::areaById(const QString &areaId) const
{
    const QString id = normalizedId(areaId);
    for (const QVariant &value : m_areas) {
        const QVariantMap area = value.toMap();
        if (area.value(QStringLiteral("id")).toString() == id) return area;
    }
    return {};
}

QVariantMap FleetManager::baselineById(const QString &baselineId) const
{
    const QString id = normalizedId(baselineId);
    for (const QVariant &value : m_baselines) {
        const QVariantMap baseline = value.toMap();
        if (baseline.value(QStringLiteral("id")).toString() == id) return baseline;
    }
    return {};
}

void FleetManager::setLastError(const QString &message) const
{
    if (m_lastError == message) return;
    m_lastError = message.left(500);
    emit const_cast<FleetManager *>(this)->lastErrorChanged();
}

void FleetManager::touchState(bool topology)
{
    if (topology) emit topologyChanged();
    emit inventoryChanged();
    emit stateChanged();
}

void FleetManager::audit(const QString &action, const QString &target,
                         const QString &outcome, const QString &detail) const
{
    const QString username = m_userManager
        ? m_userManager->currentUser().value(QStringLiteral("username")).toString()
        : QStringLiteral("system");
    const QString message = QStringLiteral("AUDIT fleet.%1 actor=%2 target=%3 outcome=%4%5")
        .arg(action.left(80), username.left(80), target.left(160), outcome.left(40),
             detail.isEmpty() ? QString() : QStringLiteral(" detail=") + detail.left(300));
    if (m_logModel) m_logModel->addLog(outcome == QStringLiteral("failed")
                                           ? QtWarningMsg : QtInfoMsg, message);
}

QString FleetManager::createSite(const QString &name, const QString &description,
                                 const QStringList &tags)
{
    if (!canManageTopology()) {
        setLastError(QStringLiteral("Administrator permission is required"));
        return {};
    }
    const QString normalizedName = name.trimmed().left(120);
    if (normalizedName.isEmpty() || m_sites.size() >= kMaximumSites) {
        setLastError(QStringLiteral("Site name is empty or the site limit was reached"));
        return {};
    }
    for (const QVariant &value : m_sites) {
        if (value.toMap().value(QStringLiteral("name")).toString()
                .compare(normalizedName, Qt::CaseInsensitive) == 0) {
            setLastError(QStringLiteral("A site with this name already exists"));
            return {};
        }
    }
    const QString id = newId();
    m_sites.append(QVariantMap{
        {QStringLiteral("id"), id},
        {QStringLiteral("name"), normalizedName},
        {QStringLiteral("description"), description.trimmed().left(500)},
        {QStringLiteral("tags"), normalizedTags(tags)},
        {QStringLiteral("firmwareBaseline"), QString()},
        {QStringLiteral("majesticBaseline"), QString()},
        {QStringLiteral("maintenanceWindow"), QVariantMap{
             {QStringLiteral("enabled"), false},
             {QStringLiteral("days"), QVariantList{1, 2, 3, 4, 5, 6, 7}},
             {QStringLiteral("start"), QStringLiteral("00:00")},
             {QStringLiteral("end"), QStringLiteral("23:59")}
         }},
        {QStringLiteral("createdAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}
    });
    setLastError({});
    audit(QStringLiteral("site.create"), id, QStringLiteral("success"), normalizedName);
    touchState(true);
    return id;
}

bool FleetManager::updateSite(const QString &siteId, const QVariantMap &changes)
{
    if (!canManageTopology()) return false;
    const QString id = normalizedId(siteId);
    for (int index = 0; index < m_sites.size(); ++index) {
        QVariantMap site = m_sites.at(index).toMap();
        if (site.value(QStringLiteral("id")).toString() != id) continue;
        if (changes.contains(QStringLiteral("name"))) {
            const QString name = changes.value(QStringLiteral("name")).toString().trimmed().left(120);
            if (name.isEmpty()) return false;
            site.insert(QStringLiteral("name"), name);
        }
        if (changes.contains(QStringLiteral("description"))) {
            site.insert(QStringLiteral("description"),
                        changes.value(QStringLiteral("description")).toString().trimmed().left(500));
        }
        if (changes.contains(QStringLiteral("tags"))) {
            site.insert(QStringLiteral("tags"), normalizedTags(changes.value(QStringLiteral("tags")).toStringList()));
        }
        site.insert(QStringLiteral("updatedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
        m_sites[index] = site;
        audit(QStringLiteral("site.update"), id, QStringLiteral("success"));
        touchState(true);
        return true;
    }
    return false;
}

bool FleetManager::removeSite(const QString &siteId)
{
    if (!canManageTopology()) return false;
    const QString id = normalizedId(siteId);
    bool removed = false;
    for (int index = m_sites.size() - 1; index >= 0; --index) {
        if (m_sites.at(index).toMap().value(QStringLiteral("id")).toString() == id) {
            m_sites.removeAt(index);
            removed = true;
        }
    }
    if (!removed) return false;
    QSet<QString> removedAreas;
    for (int index = m_areas.size() - 1; index >= 0; --index) {
        const QVariantMap area = m_areas.at(index).toMap();
        if (area.value(QStringLiteral("siteId")).toString() == id) {
            removedAreas.insert(area.value(QStringLiteral("id")).toString());
            m_areas.removeAt(index);
        }
    }
    for (auto it = m_assignments.begin(); it != m_assignments.end(); ++it) {
        QVariantMap assignment = it.value();
        if (assignment.value(QStringLiteral("siteId")).toString() == id) {
            assignment.insert(QStringLiteral("siteId"), QString());
            assignment.insert(QStringLiteral("areaId"), QString());
            it.value() = assignment;
        } else if (removedAreas.contains(assignment.value(QStringLiteral("areaId")).toString())) {
            assignment.insert(QStringLiteral("areaId"), QString());
            it.value() = assignment;
        }
    }
    audit(QStringLiteral("site.delete"), id, QStringLiteral("success"));
    emit assignmentsChanged();
    touchState(true);
    return true;
}

QString FleetManager::createArea(const QString &siteId, const QString &name,
                                 const QStringList &tags)
{
    if (!canManageTopology() || siteById(siteId).isEmpty()
        || m_areas.size() >= kMaximumAreas) return {};
    const QString normalizedName = name.trimmed().left(120);
    if (normalizedName.isEmpty()) return {};
    for (const QVariant &value : m_areas) {
        const QVariantMap area = value.toMap();
        if (area.value(QStringLiteral("siteId")).toString() == siteId
            && area.value(QStringLiteral("name")).toString()
                   .compare(normalizedName, Qt::CaseInsensitive) == 0) return {};
    }
    const QString id = newId();
    m_areas.append(QVariantMap{
        {QStringLiteral("id"), id},
        {QStringLiteral("siteId"), normalizedId(siteId)},
        {QStringLiteral("name"), normalizedName},
        {QStringLiteral("tags"), normalizedTags(tags)},
        {QStringLiteral("createdAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}
    });
    audit(QStringLiteral("area.create"), id, QStringLiteral("success"), normalizedName);
    touchState(true);
    return id;
}

bool FleetManager::updateArea(const QString &areaId, const QVariantMap &changes)
{
    if (!canManageTopology()) return false;
    const QString id = normalizedId(areaId);
    for (int index = 0; index < m_areas.size(); ++index) {
        QVariantMap area = m_areas.at(index).toMap();
        if (area.value(QStringLiteral("id")).toString() != id) continue;
        if (changes.contains(QStringLiteral("name"))) {
            const QString name = changes.value(QStringLiteral("name")).toString().trimmed().left(120);
            if (name.isEmpty()) return false;
            area.insert(QStringLiteral("name"), name);
        }
        if (changes.contains(QStringLiteral("tags"))) {
            area.insert(QStringLiteral("tags"), normalizedTags(changes.value(QStringLiteral("tags")).toStringList()));
        }
        m_areas[index] = area;
        audit(QStringLiteral("area.update"), id, QStringLiteral("success"));
        touchState(true);
        return true;
    }
    return false;
}

bool FleetManager::removeArea(const QString &areaId)
{
    if (!canManageTopology()) return false;
    const QString id = normalizedId(areaId);
    bool removed = false;
    for (int index = m_areas.size() - 1; index >= 0; --index) {
        if (m_areas.at(index).toMap().value(QStringLiteral("id")).toString() == id) {
            m_areas.removeAt(index);
            removed = true;
        }
    }
    if (!removed) return false;
    for (auto it = m_assignments.begin(); it != m_assignments.end(); ++it) {
        QVariantMap assignment = it.value();
        if (assignment.value(QStringLiteral("areaId")).toString() == id) {
            assignment.insert(QStringLiteral("areaId"), QString());
            it.value() = assignment;
        }
    }
    audit(QStringLiteral("area.delete"), id, QStringLiteral("success"));
    emit assignmentsChanged();
    touchState(true);
    return true;
}

bool FleetManager::setSiteMaintenanceWindow(const QString &siteId, const QVariantMap &window)
{
    if (!canManageTopology()) return false;
    QVariantList days;
    const QVariantList requestedDays = window.value(QStringLiteral("days")).toList();
    for (const QVariant &dayValue : requestedDays) {
        const int day = dayValue.toInt();
        if (day >= 1 && day <= 7 && !days.contains(day)) days.append(day);
    }
    if (days.isEmpty()) days = {1, 2, 3, 4, 5, 6, 7};
    const QTime start = QTime::fromString(window.value(QStringLiteral("start")).toString(), QStringLiteral("HH:mm"));
    const QTime end = QTime::fromString(window.value(QStringLiteral("end")).toString(), QStringLiteral("HH:mm"));
    if (!start.isValid() || !end.isValid()) return false;
    for (int index = 0; index < m_sites.size(); ++index) {
        QVariantMap site = m_sites.at(index).toMap();
        if (site.value(QStringLiteral("id")).toString() != siteId) continue;
        site.insert(QStringLiteral("maintenanceWindow"), QVariantMap{
            {QStringLiteral("enabled"), window.value(QStringLiteral("enabled"), true).toBool()},
            {QStringLiteral("days"), days},
            {QStringLiteral("start"), start.toString(QStringLiteral("HH:mm"))},
            {QStringLiteral("end"), end.toString(QStringLiteral("HH:mm"))}
        });
        m_sites[index] = site;
        audit(QStringLiteral("maintenance.update"), siteId, QStringLiteral("success"));
        touchState(true);
        return true;
    }
    return false;
}

bool FleetManager::setSiteVersionBaseline(const QString &siteId,
                                          const QString &firmwareVersion,
                                          const QString &majesticVersion)
{
    if (!canManageTopology()) return false;
    for (int index = 0; index < m_sites.size(); ++index) {
        QVariantMap site = m_sites.at(index).toMap();
        if (site.value(QStringLiteral("id")).toString() != siteId) continue;
        site.insert(QStringLiteral("firmwareBaseline"), firmwareVersion.trimmed().left(120));
        site.insert(QStringLiteral("majesticBaseline"), majesticVersion.trimmed().left(120));
        m_sites[index] = site;
        touchState(true);
        return true;
    }
    return false;
}

bool FleetManager::assignCamera(const QString &cameraId, const QString &siteId,
                                const QString &areaId, const QStringList &tags,
                                const QString &maintenanceState)
{
    const int index = cameraIndexForKey(cameraId);
    if (index < 0 || !canConfigureCamera(cameraId)) return false;
    const QString normalizedSite = normalizedId(siteId);
    const QString normalizedArea = normalizedId(areaId);
    if (!normalizedSite.isEmpty() && siteById(normalizedSite).isEmpty()) return false;
    if (!normalizedArea.isEmpty()) {
        const QVariantMap area = areaById(normalizedArea);
        if (area.isEmpty() || area.value(QStringLiteral("siteId")).toString() != normalizedSite) return false;
    }
    const QString requestedState = maintenanceState.trimmed().toLower();
    const QString state = requestedState == QStringLiteral("maintenance")
        || requestedState == QStringLiteral("retired")
        ? requestedState : QStringLiteral("active");
    const QString key = cameraKeyAt(index);
    m_assignments.insert(key, QVariantMap{
        {QStringLiteral("cameraId"), key},
        {QStringLiteral("siteId"), normalizedSite},
        {QStringLiteral("areaId"), normalizedArea},
        {QStringLiteral("tags"), normalizedTags(tags)},
        {QStringLiteral("maintenanceState"), state},
        {QStringLiteral("updatedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}
    });
    audit(QStringLiteral("camera.assign"), key, QStringLiteral("success"),
          QStringLiteral("site=%1 area=%2").arg(normalizedSite, normalizedArea));
    emit assignmentsChanged();
    touchState();
    return true;
}

QVariantMap FleetManager::cameraAssignment(const QString &cameraId) const
{
    const int index = cameraIndexForKey(cameraId);
    if (index < 0) return {};
    if (m_userManager) {
        const Camera camera = m_cameraModel->getCamera(index);
        if (!m_userManager->canAccessCamera(camera.id, camera.ip, index)) return {};
    }
    const QString key = cameraKeyAt(index);
    QVariantMap result = m_assignments.value(key);
    if (!result.contains(QStringLiteral("cameraId"))) result.insert(QStringLiteral("cameraId"), key);
    return result;
}

QStringList FleetManager::scopeAliases(const QString &cameraId, const QString &cameraIp,
                                       int cameraIndex) const
{
    int index = cameraIndex;
    if (index < 0 || !m_cameraModel || index >= m_cameraModel->rowCount()) {
        index = cameraIndexForKey(!cameraId.trimmed().isEmpty() ? cameraId : cameraIp);
    }
    if (index < 0) return {};
    const QString key = cameraKeyAt(index);
    const QVariantMap assignment = m_assignments.value(key);
    QStringList aliases;
    const QString siteId = assignment.value(QStringLiteral("siteId")).toString();
    const QString areaId = assignment.value(QStringLiteral("areaId")).toString();
    if (!siteId.isEmpty()) aliases.append(QStringLiteral("site:") + siteId);
    if (!areaId.isEmpty()) aliases.append(QStringLiteral("area:") + areaId);
    for (const QString &tag : stringList(assignment.value(QStringLiteral("tags")))) {
        aliases.append(QStringLiteral("tag:") + tag);
    }
    return aliases;
}

void FleetManager::syncLastSeen()
{
    if (!m_cameraModel) return;
    bool changed = false;
    const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    for (int index = 0; index < m_cameraModel->rowCount(); ++index) {
        const Camera camera = m_cameraModel->getCamera(index);
        if (!onlineStatus(camera.status)) continue;
        const QString key = cameraKeyAt(index);
        QVariantMap metadata = m_deviceMetadata.value(key);
        if (metadata.value(QStringLiteral("lastSeen")).toString() == now) continue;
        metadata.insert(QStringLiteral("lastSeen"), now);
        m_deviceMetadata.insert(key, metadata);
        changed = true;
    }
    if (changed) emit stateChanged();
}

QVariantList FleetManager::inventory() const
{
    QVariantList result;
    if (!m_cameraModel) return result;
    result.reserve(m_cameraModel->rowCount());
    for (int index = 0; index < m_cameraModel->rowCount(); ++index) {
        const Camera camera = m_cameraModel->getCamera(index);
        if (m_userManager && !m_userManager->canAccessCamera(camera.id, camera.ip, index)) continue;
        const QString key = cameraKeyAt(index);
        const QVariantMap assignment = m_assignments.value(key);
        const QVariantMap site = siteById(assignment.value(QStringLiteral("siteId")).toString());
        const QVariantMap area = areaById(assignment.value(QStringLiteral("areaId")).toString());
        const QVariantMap metadata = m_deviceMetadata.value(key);
        const QVariantMap health = m_healthController
            ? m_healthController->resultForCamera(camera.ip) : QVariantMap{};
        const QString firmware = !metadata.value(QStringLiteral("firmwareVersion")).toString().isEmpty()
            ? metadata.value(QStringLiteral("firmwareVersion")).toString()
            : health.value(QStringLiteral("firmwareVersion")).toString();
        const QString majestic = !metadata.value(QStringLiteral("majesticVersion")).toString().isEmpty()
            ? metadata.value(QStringLiteral("majesticVersion")).toString()
            : health.value(QStringLiteral("majesticVersion")).toString();
        const QString expectedFirmware = site.value(QStringLiteral("firmwareBaseline")).toString();
        const QString expectedMajestic = site.value(QStringLiteral("majesticBaseline")).toString();
        const bool firmwareDrift = (!expectedFirmware.isEmpty() && !firmware.isEmpty()
                                    && expectedFirmware != firmware)
            || (!expectedMajestic.isEmpty() && !majestic.isEmpty() && expectedMajestic != majestic);
        int driftCount = 0;
        for (const QVariant &baselineValue : m_baselines) {
            const QVariantMap baseline = baselineValue.toMap();
            const bool applies = (!baseline.value(QStringLiteral("areaId")).toString().isEmpty()
                                  && baseline.value(QStringLiteral("areaId"))
                                         == assignment.value(QStringLiteral("areaId")))
                || (baseline.value(QStringLiteral("areaId")).toString().isEmpty()
                    && !baseline.value(QStringLiteral("siteId")).toString().isEmpty()
                    && baseline.value(QStringLiteral("siteId"))
                           == assignment.value(QStringLiteral("siteId")));
            if (!applies) continue;
            driftCount = configurationDifferences(
                m_deviceConfigurations.value(key),
                baseline.value(QStringLiteral("configuration")).toMap()).size();
            break;
        }
        QStringList capabilities;
        if (camera.isOpenIpc || camera.manufacturer.compare(QStringLiteral("OpenIPC"), Qt::CaseInsensitive) == 0)
            capabilities << QStringLiteral("OpenIPC") << QStringLiteral("Majestic");
        if (!camera.streamUrl.isEmpty()) capabilities << QStringLiteral("RTSP");
        if (camera.onvifPort > 0) capabilities << QStringLiteral("HTTP/ONVIF");
        if (m_firmwareClient && m_firmwareClient->webSocketsAvailable()) capabilities << QStringLiteral("WebSocket");

        result.append(QVariantMap{
            {QStringLiteral("cameraId"), key},
            {QStringLiteral("cameraIndex"), index},
            {QStringLiteral("name"), camera.name.trimmed().isEmpty() ? camera.ip : camera.name},
            {QStringLiteral("ip"), camera.ip},
            {QStringLiteral("status"), camera.status},
            {QStringLiteral("online"), onlineStatus(camera.status)},
            {QStringLiteral("siteId"), assignment.value(QStringLiteral("siteId"))},
            {QStringLiteral("siteName"), site.value(QStringLiteral("name"))},
            {QStringLiteral("areaId"), assignment.value(QStringLiteral("areaId"))},
            {QStringLiteral("areaName"), area.value(QStringLiteral("name"))},
            {QStringLiteral("group"), camera.group},
            {QStringLiteral("tags"), assignment.value(QStringLiteral("tags")).toStringList()},
            {QStringLiteral("manufacturer"), camera.manufacturer},
            {QStringLiteral("model"), metadata.value(QStringLiteral("model"), camera.manufacturer)},
            {QStringLiteral("serialNumber"), camera.serialNumber},
            {QStringLiteral("firmwareVersion"), firmware},
            {QStringLiteral("majesticVersion"), majestic},
            {QStringLiteral("capabilities"), capabilities},
            {QStringLiteral("healthStatus"), health.value(QStringLiteral("status"), metadata.value(QStringLiteral("healthStatus")))},
            {QStringLiteral("healthDetail"), health.value(QStringLiteral("recommendation"), metadata.value(QStringLiteral("healthDetail")))},
            {QStringLiteral("lastSeen"), metadata.value(QStringLiteral("lastSeen"))},
            {QStringLiteral("lastInventoryAt"), metadata.value(QStringLiteral("lastInventoryAt"))},
            {QStringLiteral("maintenanceState"), assignment.value(QStringLiteral("maintenanceState"), QStringLiteral("active"))},
            {QStringLiteral("firmwareDrift"), firmwareDrift},
            {QStringLiteral("driftCount"), driftCount}
        });
    }
    return result;
}

QVariantList FleetManager::filterInventory(const QVariantMap &filters) const
{
    QVariantList result;
    const QString search = filters.value(QStringLiteral("search")).toString().trimmed().toLower();
    const QString siteId = filters.value(QStringLiteral("siteId")).toString();
    const QString areaId = filters.value(QStringLiteral("areaId")).toString();
    const QString tag = filters.value(QStringLiteral("tag")).toString().trimmed();
    const QString model = filters.value(QStringLiteral("model")).toString().trimmed().toLower();
    const QString maintenance = filters.value(QStringLiteral("maintenanceState")).toString();
    const bool offlineOnly = filters.value(QStringLiteral("offlineOnly")).toBool();
    const bool firmwareDriftOnly = filters.value(QStringLiteral("firmwareDrift")).toBool();
    const bool driftOnly = filters.value(QStringLiteral("driftOnly")).toBool();
    for (const QVariant &value : inventory()) {
        const QVariantMap item = value.toMap();
        if (!siteId.isEmpty() && item.value(QStringLiteral("siteId")).toString() != siteId) continue;
        if (!areaId.isEmpty() && item.value(QStringLiteral("areaId")).toString() != areaId) continue;
        if (!tag.isEmpty() && !stringList(item.value(QStringLiteral("tags"))).contains(tag, Qt::CaseInsensitive)) continue;
        if (!model.isEmpty() && !item.value(QStringLiteral("model")).toString().toLower().contains(model)) continue;
        if (!maintenance.isEmpty()
            && item.value(QStringLiteral("maintenanceState")).toString() != maintenance) continue;
        if (offlineOnly && item.value(QStringLiteral("online")).toBool()) continue;
        if (firmwareDriftOnly && !item.value(QStringLiteral("firmwareDrift")).toBool()) continue;
        if (driftOnly && item.value(QStringLiteral("driftCount")).toInt() <= 0) continue;
        if (!search.isEmpty()) {
            const QString haystack = QStringList{
                item.value(QStringLiteral("name")).toString(),
                item.value(QStringLiteral("ip")).toString(),
                item.value(QStringLiteral("siteName")).toString(),
                item.value(QStringLiteral("areaName")).toString(),
                item.value(QStringLiteral("group")).toString(),
                item.value(QStringLiteral("manufacturer")).toString(),
                item.value(QStringLiteral("model")).toString(),
                item.value(QStringLiteral("firmwareVersion")).toString(),
                stringList(item.value(QStringLiteral("tags"))).join(QLatin1Char(' '))
            }.join(QLatin1Char(' ')).toLower();
            if (!haystack.contains(search)) continue;
        }
        result.append(item);
    }
    return result;
}

void FleetManager::refreshInventory()
{
    syncLastSeen();
    emit inventoryChanged();
}

QString FleetManager::createSavedView(const QString &name, const QVariantMap &filters)
{
    if (!m_userManager || !m_userManager->isLoggedIn()) return {};
    const QString normalizedName = name.trimmed().left(120);
    if (normalizedName.isEmpty() || m_savedViews.size() >= kMaximumSavedViews) return {};
    const QString id = newId();
    m_savedViews.prepend(QVariantMap{
        {QStringLiteral("id"), id},
        {QStringLiteral("name"), normalizedName},
        {QStringLiteral("filters"), sanitizedConfiguration(filters)},
        {QStringLiteral("owner"), m_userManager->currentUser().value(QStringLiteral("username"))},
        {QStringLiteral("createdBy"), m_userManager ? m_userManager->currentUser().value(QStringLiteral("username")) : QVariant()},
        {QStringLiteral("createdAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}
    });
    emit savedViewsChanged();
    emit stateChanged();
    return id;
}

bool FleetManager::updateSavedView(const QString &viewId, const QString &name,
                                   const QVariantMap &filters)
{
    for (int index = 0; index < m_savedViews.size(); ++index) {
        QVariantMap view = m_savedViews.at(index).toMap();
        if (view.value(QStringLiteral("id")).toString() != viewId) continue;
        const QString owner = view.value(QStringLiteral("owner")).toString();
        const QString actor = m_userManager
            ? m_userManager->currentUser().value(QStringLiteral("username")).toString() : QString();
        if (!m_userManager || (!m_userManager->isAdmin() && owner != actor)) return false;
        const QString normalizedName = name.trimmed().left(120);
        if (normalizedName.isEmpty()) return false;
        view.insert(QStringLiteral("name"), normalizedName);
        view.insert(QStringLiteral("filters"), sanitizedConfiguration(filters));
        m_savedViews[index] = view;
        emit savedViewsChanged();
        emit stateChanged();
        return true;
    }
    return false;
}

bool FleetManager::removeSavedView(const QString &viewId)
{
    for (int index = 0; index < m_savedViews.size(); ++index) {
        const QVariantMap view = m_savedViews.at(index).toMap();
        if (view.value(QStringLiteral("id")).toString() != viewId) continue;
        const QString owner = view.value(QStringLiteral("owner")).toString();
        const QString actor = m_userManager
            ? m_userManager->currentUser().value(QStringLiteral("username")).toString() : QString();
        if (!m_userManager || (!m_userManager->isAdmin() && owner != actor)) return false;
        m_savedViews.removeAt(index);
        emit savedViewsChanged();
        emit stateChanged();
        return true;
    }
    return false;
}

bool FleetManager::exportInventory(const QString &pathOrUrl, const QVariantMap &filters)
{
    const QString path = localPath(pathOrUrl);
    if (!m_userManager || !m_userManager->canExport() || path.isEmpty()) return false;
    const QVariantList rows = filterInventory(filters);
    if (QFileInfo(path).suffix().compare(QStringLiteral("csv"), Qt::CaseInsensitive) == 0) {
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            setLastError(file.errorString());
            return false;
        }
        QTextStream stream(&file);
        stream << "name,ip,site,area,group,tags,status,firmware,majestic,health,last_seen,maintenance\n";
        for (const QVariant &value : rows) {
            const QVariantMap item = value.toMap();
            stream << csvCell(item.value(QStringLiteral("name")).toString()) << ','
                   << csvCell(item.value(QStringLiteral("ip")).toString()) << ','
                   << csvCell(item.value(QStringLiteral("siteName")).toString()) << ','
                   << csvCell(item.value(QStringLiteral("areaName")).toString()) << ','
                   << csvCell(item.value(QStringLiteral("group")).toString()) << ','
                   << csvCell(stringList(item.value(QStringLiteral("tags"))).join(QLatin1Char(';'))) << ','
                   << csvCell(item.value(QStringLiteral("status")).toString()) << ','
                   << csvCell(item.value(QStringLiteral("firmwareVersion")).toString()) << ','
                   << csvCell(item.value(QStringLiteral("majesticVersion")).toString()) << ','
                   << csvCell(item.value(QStringLiteral("healthStatus")).toString()) << ','
                   << csvCell(item.value(QStringLiteral("lastSeen")).toString()) << ','
                   << csvCell(item.value(QStringLiteral("maintenanceState")).toString()) << '\n';
        }
        if (!file.commit()) {
            setLastError(file.errorString());
            return false;
        }
    } else {
        QString error;
        if (!writeJsonObject(path, {
                {QStringLiteral("format"), QStringLiteral("openipc-dashboard-fleet-inventory")},
                {QStringLiteral("version"), 1},
                {QStringLiteral("exportedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
                {QStringLiteral("devices"), rows}
            }, &error)) {
            setLastError(error);
            return false;
        }
    }
    audit(QStringLiteral("inventory.export"), path, QStringLiteral("success"),
          QStringLiteral("devices=%1").arg(rows.size()));
    setLastError({});
    return true;
}

bool FleetManager::exportDiagnostics(const QString &pathOrUrl, const QVariantList &cameraIds)
{
    if (!m_userManager || !m_userManager->canExport()) return false;
    const QString path = localPath(pathOrUrl);
    if (path.isEmpty()) return false;
    const QStringList ids = normalizedCameraIds(cameraIds);
    QVariantList devices;
    for (const QVariant &value : inventory()) {
        QVariantMap device = value.toMap();
        if (!ids.isEmpty() && !ids.contains(device.value(QStringLiteral("cameraId")).toString())) continue;
        if (m_healthController) {
            device.insert(QStringLiteral("diagnostics"), sanitizedConfiguration(
                m_healthController->resultForCamera(device.value(QStringLiteral("ip")).toString())));
        }
        devices.append(device);
    }
    QString error;
    const bool ok = writeJsonObject(path, {
        {QStringLiteral("format"), QStringLiteral("openipc-dashboard-fleet-diagnostics")},
        {QStringLiteral("version"), 1},
        {QStringLiteral("exportedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
        {QStringLiteral("devices"), devices}
    }, &error);
    setLastError(ok ? QString() : error);
    audit(QStringLiteral("diagnostics.export"), path,
          ok ? QStringLiteral("success") : QStringLiteral("failed"));
    return ok;
}

QString FleetManager::createBaseline(const QString &name, const QString &siteId,
                                     const QString &areaId,
                                     const QVariantMap &configuration)
{
    if (!m_userManager || !m_userManager->canSettings()
        || m_baselines.size() >= kMaximumBaselines) return {};
    const QString normalizedName = name.trimmed().left(120);
    const QVariantMap safeConfiguration = sanitizedConfiguration(configuration);
    if (normalizedName.isEmpty() || safeConfiguration.isEmpty()) return {};
    const QString normalizedSite = normalizedId(siteId);
    const QString normalizedArea = normalizedId(areaId);
    if (!m_userManager->isAdmin()) {
        bool siteVisible = normalizedSite.isEmpty();
        bool areaVisible = normalizedArea.isEmpty();
        for (const QVariant &value : sites())
            siteVisible = siteVisible || value.toMap().value(QStringLiteral("id")).toString() == normalizedSite;
        for (const QVariant &value : areas())
            areaVisible = areaVisible || value.toMap().value(QStringLiteral("id")).toString() == normalizedArea;
        if (!siteVisible || !areaVisible) return {};
    }
    if (!normalizedArea.isEmpty()) {
        const QVariantMap area = areaById(normalizedArea);
        if (area.isEmpty() || area.value(QStringLiteral("siteId")).toString() != normalizedSite) return {};
    } else if (!normalizedSite.isEmpty() && siteById(normalizedSite).isEmpty()) {
        return {};
    }
    const QString id = newId();
    m_baselines.prepend(QVariantMap{
        {QStringLiteral("id"), id},
        {QStringLiteral("name"), normalizedName},
        {QStringLiteral("siteId"), normalizedSite},
        {QStringLiteral("areaId"), normalizedArea},
        {QStringLiteral("configuration"), safeConfiguration},
        {QStringLiteral("createdAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
        {QStringLiteral("createdBy"), m_userManager->currentUser().value(QStringLiteral("username"))}
    });
    emit baselinesChanged();
    emit inventoryChanged();
    emit stateChanged();
    audit(QStringLiteral("baseline.create"), id, QStringLiteral("success"), normalizedName);
    return id;
}

QString FleetManager::captureBaselineFromCamera(const QString &cameraId, const QString &name)
{
    if (!canConfigureCamera(cameraId) || !m_majesticClient) return {};
    const int index = cameraIndexForKey(cameraId);
    const Camera camera = m_cameraModel->getCamera(index);
    const QString requestId = m_majesticClient->loadConfiguration(
        camera.ip, camera.onvifPort > 0 ? camera.onvifPort : 80,
        camera.login, camera.password);
    PendingRequest pending;
    pending.cameraId = cameraKeyAt(index);
    pending.stage = QStringLiteral("capture-baseline");
    pending.name = name.trimmed().left(120);
    m_pendingRequests.insert(requestId, pending);
    return requestId;
}

bool FleetManager::removeBaseline(const QString &baselineId)
{
    if (!m_userManager || !m_userManager->canSettings()) return false;
    if (!m_userManager->isAdmin()) {
        bool visible = false;
        for (const QVariant &value : baselines())
            visible = visible || value.toMap().value(QStringLiteral("id")).toString() == baselineId;
        if (!visible) return false;
    }
    for (int index = 0; index < m_baselines.size(); ++index) {
        if (m_baselines.at(index).toMap().value(QStringLiteral("id")).toString() != baselineId) continue;
        m_baselines.removeAt(index);
        emit baselinesChanged();
        emit inventoryChanged();
        emit stateChanged();
        audit(QStringLiteral("baseline.delete"), baselineId, QStringLiteral("success"));
        return true;
    }
    return false;
}

void FleetManager::setDeviceConfigurationSnapshot(const QString &cameraId,
                                                  const QVariantMap &configuration,
                                                  const QVariantMap &schema)
{
    if (!canConfigureCamera(cameraId)) return;
    const int index = cameraIndexForKey(cameraId);
    if (index < 0) return;
    const QString key = cameraKeyAt(index);
    m_deviceConfigurations.insert(key, sanitizedConfiguration(configuration));
    m_deviceSchemas.insert(key, sanitizedConfiguration(schema));
    touchState();
}

QVariantMap FleetManager::driftPreview(const QString &baselineId,
                                       const QVariantList &cameraIds) const
{
    if (!m_userManager || !m_userManager->canSettings()) return {};
    if (!m_userManager->isAdmin()) {
        bool visible = false;
        for (const QVariant &value : baselines())
            visible = visible || value.toMap().value(QStringLiteral("id")).toString() == baselineId;
        if (!visible) return {};
    }
    const QVariantMap baseline = baselineById(baselineId);
    if (baseline.isEmpty()) return {};
    QStringList ids = normalizedCameraIds(cameraIds);
    if (ids.isEmpty()) {
        const QString siteId = baseline.value(QStringLiteral("siteId")).toString();
        const QString areaId = baseline.value(QStringLiteral("areaId")).toString();
        for (const QVariant &value : inventory()) {
            const QVariantMap item = value.toMap();
            if (!areaId.isEmpty() && item.value(QStringLiteral("areaId")).toString() != areaId) continue;
            if (areaId.isEmpty() && !siteId.isEmpty()
                && item.value(QStringLiteral("siteId")).toString() != siteId) continue;
            ids.append(item.value(QStringLiteral("cameraId")).toString());
        }
    }
    QVariantList devices;
    int drifted = 0;
    int unknown = 0;
    const QVariantMap expected = baseline.value(QStringLiteral("configuration")).toMap();
    for (const QString &cameraId : ids) {
        const int index = cameraIndexForKey(cameraId);
        if (index < 0) continue;
        const Camera camera = m_cameraModel->getCamera(index);
        const QVariantMap current = m_deviceConfigurations.value(cameraKeyAt(index));
        const QVariantList differences = current.isEmpty()
            ? QVariantList{} : configurationDifferences(current, expected);
        if (current.isEmpty()) ++unknown;
        else if (!differences.isEmpty()) ++drifted;
        devices.append(QVariantMap{
            {QStringLiteral("cameraId"), cameraKeyAt(index)},
            {QStringLiteral("name"), camera.name.trimmed().isEmpty() ? camera.ip : camera.name},
            {QStringLiteral("ip"), camera.ip},
            {QStringLiteral("status"), current.isEmpty() ? QStringLiteral("unknown")
                                                           : (differences.isEmpty() ? QStringLiteral("aligned")
                                                                                  : QStringLiteral("drift"))},
            {QStringLiteral("differenceCount"), differences.size()},
            {QStringLiteral("differences"), differences},
            {QStringLiteral("redacted"), true}
        });
    }
    return {
        {QStringLiteral("baselineId"), baselineId},
        {QStringLiteral("baselineName"), baseline.value(QStringLiteral("name"))},
        {QStringLiteral("deviceCount"), devices.size()},
        {QStringLiteral("driftedCount"), drifted},
        {QStringLiteral("unknownCount"), unknown},
        {QStringLiteral("devices"), devices},
        {QStringLiteral("redacted"), true}
    };
}

bool FleetManager::isWithinMaintenanceWindow(const QString &siteId) const
{
    const QVariantMap window = siteById(siteId).value(QStringLiteral("maintenanceWindow")).toMap();
    if (!window.value(QStringLiteral("enabled")).toBool()) return false;
    const QDateTime local = QDateTime::currentDateTime();
    bool dayAllowed = false;
    for (const QVariant &value : window.value(QStringLiteral("days")).toList()) {
        if (value.toInt() == local.date().dayOfWeek()) {
            dayAllowed = true;
            break;
        }
    }
    if (!dayAllowed) return false;
    const QTime start = QTime::fromString(window.value(QStringLiteral("start")).toString(), QStringLiteral("HH:mm"));
    const QTime end = QTime::fromString(window.value(QStringLiteral("end")).toString(), QStringLiteral("HH:mm"));
    if (!start.isValid() || !end.isValid()) return false;
    const QTime now = local.time();
    return start <= end ? (now >= start && now <= end) : (now >= start || now <= end);
}

QStringList FleetManager::normalizedCameraIds(const QVariantList &cameraIds) const
{
    QStringList result;
    for (const QVariant &value : cameraIds) {
        const int index = cameraIndexForKey(value.toString());
        if (index < 0) continue;
        if (m_userManager) {
            const Camera camera = m_cameraModel->getCamera(index);
            if (!m_userManager->canAccessCamera(camera.id, camera.ip, index)) continue;
        }
        const QString key = cameraKeyAt(index);
        if (!result.contains(key)) result.append(key);
        if (result.size() >= 256) break;
    }
    return result;
}

QVariantMap FleetManager::devicePreflight(const QString &operation,
                                          const QString &cameraId,
                                          const QVariantMap &options) const
{
    const int index = cameraIndexForKey(cameraId);
    QVariantMap result{{QStringLiteral("cameraId"), cameraId},
                       {QStringLiteral("ready"), true},
                       {QStringLiteral("blockers"), QVariantList{}},
                       {QStringLiteral("warnings"), QVariantList{}}};
    QVariantList blockers;
    QVariantList warnings;
    if (index < 0) {
        blockers.append(QStringLiteral("Camera no longer exists"));
    } else {
        const Camera camera = m_cameraModel->getCamera(index);
        result.insert(QStringLiteral("name"), camera.name.trimmed().isEmpty() ? camera.ip : camera.name);
        result.insert(QStringLiteral("ip"), camera.ip);
        if (m_userManager && !m_userManager->canAccessCamera(camera.id, camera.ip, index)) {
            blockers.append(QStringLiteral("Camera scope denies access"));
        }
        const bool configurationOperation = operation == QStringLiteral("configuration-read")
            || operation == QStringLiteral("apply-baseline");
        const bool openIpc = camera.isOpenIpc
            || camera.manufacturer.compare(QStringLiteral("OpenIPC"), Qt::CaseInsensitive) == 0
            || camera.onboardingProfile == QStringLiteral("openipc");
        if (configurationOperation && !openIpc) {
            blockers.append(QStringLiteral("Majestic compatibility was not detected"));
        }
        if (operation == QStringLiteral("configuration-read") && !canConfigureCamera(cameraId)) {
            blockers.append(QStringLiteral("Settings permission is required"));
        }
        if (operation == QStringLiteral("apply-baseline")) {
            if (!canConfigureCamera(cameraId)) blockers.append(QStringLiteral("Settings permission is required"));
            const QVariantMap baseline = baselineById(options.value(QStringLiteral("baselineId")).toString());
            if (baseline.isEmpty()) blockers.append(QStringLiteral("A configuration baseline is required"));
            const QVariantMap assignment = m_assignments.value(cameraId);
            const QString siteId = assignment.value(QStringLiteral("siteId")).toString();
            const QString areaId = assignment.value(QStringLiteral("areaId")).toString();
            if (siteId.isEmpty()) blockers.append(QStringLiteral("Camera is not assigned to a site"));
            const QString baselineSite = baseline.value(QStringLiteral("siteId")).toString();
            const QString baselineArea = baseline.value(QStringLiteral("areaId")).toString();
            if ((!baselineArea.isEmpty() && baselineArea != areaId)
                || (baselineArea.isEmpty() && !baselineSite.isEmpty()
                    && baselineSite != siteId)) {
                blockers.append(QStringLiteral("Baseline does not apply to the camera location"));
            }
            const bool overrideWindow = options.value(QStringLiteral("maintenanceOverride")).toBool();
            if (!overrideWindow && !isWithinMaintenanceWindow(siteId)) {
                if (options.value(QStringLiteral("dryRun"), true).toBool())
                    warnings.append(QStringLiteral("Execution is outside the configured maintenance window"));
                else
                    blockers.append(QStringLiteral("Outside the configured maintenance window"));
            }
            if (!options.value(QStringLiteral("dryRun"), true).toBool()) {
                if (!options.value(QStringLiteral("backupBeforeChange")).toBool()) {
                    blockers.append(QStringLiteral("Backup-before-change is required"));
                }
                const QString backupDirectory = localPath(options.value(QStringLiteral("backupDirectory")).toString());
                const QFileInfo backupInfo(backupDirectory);
                if (backupDirectory.isEmpty() || !backupInfo.exists()
                    || !backupInfo.isDir() || !backupInfo.isWritable()) {
                    blockers.append(QStringLiteral("A valid backup directory is required"));
                }
            }
            if (m_deviceConfigurations.value(cameraId).isEmpty()) {
                warnings.append(QStringLiteral("Current configuration will be read before applying"));
            }
        }
        if (!onlineStatus(camera.status)) warnings.append(QStringLiteral("Camera is not currently online"));
    }
    result.insert(QStringLiteral("blockers"), blockers);
    result.insert(QStringLiteral("warnings"), warnings);
    result.insert(QStringLiteral("ready"), blockers.isEmpty());
    return result;
}

QVariantMap FleetManager::preflightBatch(const QString &operation,
                                         const QVariantList &cameraIds,
                                         const QVariantMap &options) const
{
    const QString normalizedOperation = operation.trimmed().toLower();
    const QStringList allowed{QStringLiteral("inventory"), QStringLiteral("health"),
                              QStringLiteral("configuration-read"),
                              QStringLiteral("apply-baseline")};
    QVariantList devices;
    int blockers = 0;
    int warnings = 0;
    const QStringList ids = normalizedCameraIds(cameraIds);
    for (const QString &cameraId : ids) {
        const QVariantMap item = devicePreflight(normalizedOperation, cameraId, options);
        blockers += item.value(QStringLiteral("blockers")).toList().size();
        warnings += item.value(QStringLiteral("warnings")).toList().size();
        devices.append(item);
    }
    return {
        {QStringLiteral("operation"), normalizedOperation},
        {QStringLiteral("mutating"), normalizedOperation == QStringLiteral("apply-baseline")},
        {QStringLiteral("allowed"), allowed.contains(normalizedOperation)
                                      && !ids.isEmpty() && blockers == 0},
        {QStringLiteral("deviceCount"), ids.size()},
        {QStringLiteral("blockerCount"), blockers},
        {QStringLiteral("warningCount"), warnings},
        {QStringLiteral("devices"), devices},
        {QStringLiteral("recoveryGuidance"),
         QStringLiteral("Keep every per-device backup until all cameras are verified. Failed devices must be recovered individually; automatic firmware rollback is not attempted.")}
    };
}

bool FleetManager::startBatch(const QString &operation, const QVariantList &cameraIds,
                              const QVariantMap &options)
{
    if (m_batchState.value(QStringLiteral("running")).toBool()) {
        setLastError(QStringLiteral("Another fleet operation is already running"));
        return false;
    }
    const QString normalizedOperation = operation.trimmed().toLower();
    const QStringList allowedOperations{QStringLiteral("inventory"), QStringLiteral("health"),
                                        QStringLiteral("configuration-read"),
                                        QStringLiteral("apply-baseline")};
    if (!allowedOperations.contains(normalizedOperation)) {
        setLastError(QStringLiteral("Unsupported fleet operation"));
        return false;
    }
    QVariantMap normalizedOptions = options;
    normalizedOptions.insert(QStringLiteral("dryRun"), options.value(QStringLiteral("dryRun"), true).toBool());
    normalizedOptions.insert(QStringLiteral("concurrency"),
                             std::clamp(options.value(QStringLiteral("concurrency"), 2).toInt(), 1, 8));
    const QVariantMap preflight = preflightBatch(operation, cameraIds, normalizedOptions);
    const bool dryRun = normalizedOptions.value(QStringLiteral("dryRun")).toBool();
    if (!dryRun && !preflight.value(QStringLiteral("allowed")).toBool()) {
        setLastError(QStringLiteral("Batch preflight contains blockers"));
        audit(QStringLiteral("batch.preflight"), operation, QStringLiteral("failed"),
              QStringLiteral("blockers=%1").arg(preflight.value(QStringLiteral("blockerCount")).toInt()));
        return false;
    }
    const QStringList ids = normalizedCameraIds(cameraIds);
    if (ids.isEmpty()) return false;

    m_batchQueue = ids;
    m_batchInFlight.clear();
    m_batchResults.clear();
    m_batchState = {
        {QStringLiteral("id"), newId()},
        {QStringLiteral("operation"), normalizedOperation},
        {QStringLiteral("running"), true},
        {QStringLiteral("dryRun"), dryRun},
        {QStringLiteral("options"), normalizedOptions},
        {QStringLiteral("total"), ids.size()},
        {QStringLiteral("completed"), 0},
        {QStringLiteral("succeeded"), 0},
        {QStringLiteral("failed"), 0},
        {QStringLiteral("skipped"), 0},
        {QStringLiteral("progress"), 0.0},
        {QStringLiteral("cancelRequested"), false},
        {QStringLiteral("startedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
        {QStringLiteral("preflight"), preflight}
    };
    setLastError({});
    emit batchResultsChanged();
    emit batchStateChanged();
    audit(QStringLiteral("batch.start"), m_batchState.value(QStringLiteral("id")).toString(),
          QStringLiteral("success"), QStringLiteral("operation=%1 devices=%2 dryRun=%3")
              .arg(operation).arg(ids.size()).arg(dryRun));

    if (dryRun) {
        m_batchQueue.clear();
        int succeeded = 0;
        int failed = 0;
        for (const QVariant &value : preflight.value(QStringLiteral("devices")).toList()) {
            const QVariantMap device = value.toMap();
            const bool ready = device.value(QStringLiteral("ready")).toBool();
            if (ready) ++succeeded;
            else ++failed;
            m_batchResults.append(QVariantMap{
                {QStringLiteral("cameraId"), device.value(QStringLiteral("cameraId"))},
                {QStringLiteral("name"), device.value(QStringLiteral("name"))},
                {QStringLiteral("ip"), device.value(QStringLiteral("ip"))},
                {QStringLiteral("status"), ready ? QStringLiteral("ready")
                                                  : QStringLiteral("blocked")},
                {QStringLiteral("message"), ready
                     ? QStringLiteral("Dry-run preflight passed")
                     : device.value(QStringLiteral("blockers")).toStringList().join(QStringLiteral("; "))},
                {QStringLiteral("details"), device},
                {QStringLiteral("completedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}
            });
        }
        m_batchState.insert(QStringLiteral("completed"), ids.size());
        m_batchState.insert(QStringLiteral("succeeded"), succeeded);
        m_batchState.insert(QStringLiteral("failed"), failed);
        m_batchState.insert(QStringLiteral("progress"), 1.0);
        emit batchResultsChanged();
        emit batchStateChanged();
        finishBatch(failed == 0 ? QStringLiteral("completed") : QStringLiteral("blocked"));
        return true;
    }
    dispatchBatch();
    return true;
}

void FleetManager::cancelBatch()
{
    if (!m_batchState.value(QStringLiteral("running")).toBool()) return;
    m_batchState.insert(QStringLiteral("cancelRequested"), true);
    const QStringList queued = m_batchQueue;
    m_batchQueue.clear();
    emit batchStateChanged();
    audit(QStringLiteral("batch.cancel"), m_batchState.value(QStringLiteral("id")).toString(),
          QStringLiteral("requested"));
    for (const QString &cameraId : queued) {
        completeBatchDevice(cameraId, QStringLiteral("cancelled"),
                            QStringLiteral("Cancelled before dispatch"));
    }
    if (m_batchInFlight.isEmpty()) finishBatch(QStringLiteral("cancelled"));
}

void FleetManager::dispatchBatch()
{
    if (!m_batchState.value(QStringLiteral("running")).toBool()) return;
    if (m_batchState.value(QStringLiteral("cancelRequested")).toBool()) {
        if (m_batchInFlight.isEmpty()) finishBatch(QStringLiteral("cancelled"));
        return;
    }
    const QString operation = m_batchState.value(QStringLiteral("operation")).toString();
    const QVariantMap options = m_batchState.value(QStringLiteral("options")).toMap();
    const int concurrency = operation == QStringLiteral("health") ? 1
        : std::clamp(options.value(QStringLiteral("concurrency"), 2).toInt(), 1, 8);
    while (!m_batchQueue.isEmpty() && m_batchInFlight.size() < concurrency) {
        const QString cameraId = m_batchQueue.takeFirst();
        const int index = cameraIndexForKey(cameraId);
        if (index < 0) {
            completeBatchDevice(cameraId, QStringLiteral("failed"), QStringLiteral("Camera no longer exists"));
            continue;
        }
        const Camera camera = m_cameraModel->getCamera(index);
        m_batchInFlight.insert(cameraId);
        if (operation == QStringLiteral("health")) {
            if (!m_healthController || !m_healthController->runCamera(camera.ip, QStringLiteral("deep"))) {
                completeBatchDevice(cameraId, QStringLiteral("failed"),
                                    QStringLiteral("Health diagnostics could not be started"));
            } else {
                m_activeHealthCameraId = cameraId;
            }
        } else if (operation == QStringLiteral("inventory")) {
            const bool openIpc = camera.isOpenIpc
                || camera.manufacturer.compare(QStringLiteral("OpenIPC"), Qt::CaseInsensitive) == 0
                || camera.onboardingProfile == QStringLiteral("openipc");
            if (!openIpc || !m_firmwareClient) {
                QVariantMap metadata = m_deviceMetadata.value(cameraId);
                metadata.insert(QStringLiteral("lastInventoryAt"),
                                QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
                m_deviceMetadata.insert(cameraId, metadata);
                completeBatchDevice(cameraId, QStringLiteral("success"),
                                    QStringLiteral("Generic camera inventory refreshed"));
            } else {
                const QString requestId = m_firmwareClient->loadStatus(
                    camera.ip, camera.onvifPort > 0 ? camera.onvifPort : 80,
                    camera.login, camera.password);
                PendingRequest pending;
                pending.cameraId = cameraId;
                pending.stage = QStringLiteral("inventory");
                m_pendingRequests.insert(requestId, pending);
            }
        } else if (operation == QStringLiteral("configuration-read")
                   || operation == QStringLiteral("apply-baseline")) {
            if (!m_majesticClient) {
                completeBatchDevice(cameraId, QStringLiteral("failed"),
                                    QStringLiteral("Majestic client is unavailable"));
            } else {
                const QString requestId = m_majesticClient->loadConfiguration(
                    camera.ip, camera.onvifPort > 0 ? camera.onvifPort : 80,
                    camera.login, camera.password);
                PendingRequest pending;
                pending.cameraId = cameraId;
                pending.stage = operation == QStringLiteral("configuration-read")
                    ? QStringLiteral("configuration-read") : QStringLiteral("apply-load");
                m_pendingRequests.insert(requestId, pending);
            }
        }
    }
    if (m_batchQueue.isEmpty() && m_batchInFlight.isEmpty()) {
        finishBatch(QStringLiteral("completed"));
    }
}

void FleetManager::completeBatchDevice(const QString &cameraId, const QString &status,
                                       const QString &message, const QVariantMap &details)
{
    m_batchInFlight.remove(cameraId);
    const int index = cameraIndexForKey(cameraId);
    const Camera camera = m_cameraModel ? m_cameraModel->getCamera(index) : Camera{};
    m_batchResults.append(QVariantMap{
        {QStringLiteral("cameraId"), cameraId},
        {QStringLiteral("name"), camera.name.trimmed().isEmpty() ? camera.ip : camera.name},
        {QStringLiteral("ip"), camera.ip},
        {QStringLiteral("status"), status},
        {QStringLiteral("message"), message.left(500)},
        {QStringLiteral("details"), sanitizedConfiguration(details)},
        {QStringLiteral("completedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}
    });
    const int completed = m_batchState.value(QStringLiteral("completed")).toInt() + 1;
    m_batchState.insert(QStringLiteral("completed"), completed);
    if (status == QStringLiteral("failed") || status == QStringLiteral("blocked")) {
        m_batchState.insert(QStringLiteral("failed"),
                            m_batchState.value(QStringLiteral("failed")).toInt() + 1);
    } else if (status == QStringLiteral("skipped") || status == QStringLiteral("cancelled")) {
        m_batchState.insert(QStringLiteral("skipped"),
                            m_batchState.value(QStringLiteral("skipped")).toInt() + 1);
    } else {
        m_batchState.insert(QStringLiteral("succeeded"),
                            m_batchState.value(QStringLiteral("succeeded")).toInt() + 1);
    }
    const int total = std::max(1, m_batchState.value(QStringLiteral("total")).toInt());
    m_batchState.insert(QStringLiteral("progress"), static_cast<double>(completed) / total);
    emit batchResultsChanged();
    emit batchStateChanged();
    audit(QStringLiteral("batch.device"), cameraId,
          status == QStringLiteral("success") || status == QStringLiteral("ready")
              ? QStringLiteral("success") : status,
          message);
    QTimer::singleShot(0, this, &FleetManager::dispatchBatch);
}

void FleetManager::finishBatch(const QString &outcome)
{
    if (!m_batchState.value(QStringLiteral("running")).toBool()) return;
    m_batchState.insert(QStringLiteral("running"), false);
    m_batchState.insert(QStringLiteral("outcome"), outcome);
    m_batchState.insert(QStringLiteral("endedAt"),
                        QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    if (m_batchState.value(QStringLiteral("completed")).toInt()
        >= m_batchState.value(QStringLiteral("total")).toInt()) {
        m_batchState.insert(QStringLiteral("progress"), 1.0);
    }
    QVariantMap historyEntry = m_batchState;
    QVariantMap historyOptions = historyEntry.value(QStringLiteral("options")).toMap();
    historyOptions.remove(QStringLiteral("backupDirectory"));
    historyEntry.insert(QStringLiteral("options"), historyOptions);
    historyEntry.insert(QStringLiteral("results"), m_batchResults);
    m_batchHistory.prepend(historyEntry);
    while (m_batchHistory.size() > kMaximumBatchHistory) m_batchHistory.removeLast();
    audit(QStringLiteral("batch.finish"), m_batchState.value(QStringLiteral("id")).toString(),
          outcome, QStringLiteral("success=%1 failed=%2 skipped=%3")
              .arg(m_batchState.value(QStringLiteral("succeeded")).toInt())
              .arg(m_batchState.value(QStringLiteral("failed")).toInt())
              .arg(m_batchState.value(QStringLiteral("skipped")).toInt()));
    emit batchStateChanged();
    emit batchHistoryChanged();
    emit stateChanged();
}

void FleetManager::handleConfigurationLoaded(const QString &requestId,
                                             const QVariantMap &configuration,
                                             const QVariantMap &schema,
                                             const QVariantMap &capabilities)
{
    const auto it = m_pendingRequests.find(requestId);
    if (it == m_pendingRequests.end()) return;
    PendingRequest pending = it.value();
    m_pendingRequests.erase(it);
    const QVariantMap safeConfiguration = sanitizedConfiguration(configuration);
    if (pending.stage == QStringLiteral("capture-baseline")) {
        const QVariantMap assignment = cameraAssignment(pending.cameraId);
        const QString id = createBaseline(
            pending.name.isEmpty() ? QStringLiteral("Camera baseline") : pending.name,
            assignment.value(QStringLiteral("siteId")).toString(),
            assignment.value(QStringLiteral("areaId")).toString(), safeConfiguration);
        if (!id.isEmpty()) {
            setDeviceConfigurationSnapshot(pending.cameraId, safeConfiguration, schema);
            emit operationMessage(QStringLiteral("Baseline captured"));
        } else {
            setLastError(QStringLiteral("Baseline could not be created"));
        }
        return;
    }
    if (m_batchState.value(QStringLiteral("cancelRequested")).toBool()) {
        completeBatchDevice(pending.cameraId, QStringLiteral("cancelled"),
                            QStringLiteral("Cancelled before configuration change"));
        return;
    }
    m_deviceConfigurations.insert(pending.cameraId, safeConfiguration);
    m_deviceSchemas.insert(pending.cameraId, sanitizedConfiguration(schema));
    QVariantMap metadata = m_deviceMetadata.value(pending.cameraId);
    metadata.insert(QStringLiteral("majesticCapabilities"), sanitizedConfiguration(capabilities));
    const QString majesticVersion = firstVersionValue(configuration);
    if (!majesticVersion.isEmpty()) metadata.insert(QStringLiteral("majesticVersion"), majesticVersion);
    metadata.insert(QStringLiteral("lastConfigurationReadAt"),
                    QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    m_deviceMetadata.insert(pending.cameraId, metadata);
    touchState();
    if (pending.stage == QStringLiteral("configuration-read")) {
        completeBatchDevice(pending.cameraId, QStringLiteral("success"),
                            QStringLiteral("Configuration snapshot refreshed"),
                            {{QStringLiteral("sections"), safeConfiguration.size()}});
        return;
    }
    if (pending.stage != QStringLiteral("apply-load")) return;
    const QVariantMap options = m_batchState.value(QStringLiteral("options")).toMap();
    const QVariantMap baseline = baselineById(options.value(QStringLiteral("baselineId")).toString());
    const QVariantMap expected = baseline.value(QStringLiteral("configuration")).toMap();
    const QVariantMap patch = configurationPatch(safeConfiguration, expected);
    if (patch.isEmpty()) {
        completeBatchDevice(pending.cameraId, QStringLiteral("skipped"),
                            QStringLiteral("Camera already matches the baseline"));
        return;
    }
    const int index = cameraIndexForKey(pending.cameraId);
    const Camera camera = m_cameraModel->getCamera(index);
    const QString directory = localPath(options.value(QStringLiteral("backupDirectory")).toString());
    const QString filename = QStringLiteral("%1-%2-majestic-backup.json")
        .arg(safeFilePart(camera.name.isEmpty() ? camera.ip : camera.name),
             QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz")));
    const QString backupPath = QDir(directory).filePath(filename);
    const QString backupId = m_majesticClient->saveConfigurationBackup(
        safeConfiguration, sanitizedConfiguration(schema), backupPath);
    pending.stage = QStringLiteral("apply-backup");
    pending.configuration = mergedConfiguration(safeConfiguration, patch);
    pending.schema = sanitizedConfiguration(schema);
    pending.patch = patch;
    pending.backupPath = backupPath;
    m_pendingRequests.insert(backupId, pending);
}

void FleetManager::handleRequestFailure(const QString &requestId,
                                        const QString &message, int httpStatus)
{
    const auto it = m_pendingRequests.find(requestId);
    if (it == m_pendingRequests.end()) return;
    const PendingRequest pending = it.value();
    m_pendingRequests.erase(it);
    if (pending.stage == QStringLiteral("capture-baseline")) {
        setLastError(message);
        emit operationMessage(QStringLiteral("Baseline capture failed"));
        audit(QStringLiteral("baseline.capture"), pending.cameraId,
              QStringLiteral("failed"), message);
        return;
    }
    completeBatchDevice(pending.cameraId, QStringLiteral("failed"), message,
                        {{QStringLiteral("httpStatus"), httpStatus},
                         {QStringLiteral("stage"), pending.stage},
                         {QStringLiteral("recoveryGuidance"),
                          pending.backupPath.isEmpty()
                              ? QStringLiteral("No configuration change was committed")
                              : QStringLiteral("Use the retained per-device backup for manual recovery")}});
}

QVariantMap FleetManager::previewSiteImport(const QString &pathOrUrl) const
{
    if (!canManageTopology()) {
        setLastError(QStringLiteral("Administrator permission is required"));
        return {};
    }
    QString error;
    const QVariantMap document = readJsonObject(localPath(pathOrUrl), &error);
    if (document.value(QStringLiteral("format")).toString()
        != QStringLiteral("openipc-dashboard-sites")) {
        setLastError(error.isEmpty() ? QStringLiteral("Unsupported site definition format") : error);
        return {};
    }
    int conflicts = 0;
    QStringList conflictNames;
    for (const QVariant &value : document.value(QStringLiteral("sites")).toList()) {
        const QVariantMap incoming = value.toMap();
        for (const QVariant &existingValue : m_sites) {
            const QVariantMap existing = existingValue.toMap();
            if (existing.value(QStringLiteral("id")) == incoming.value(QStringLiteral("id"))
                || existing.value(QStringLiteral("name")).toString().compare(
                       incoming.value(QStringLiteral("name")).toString(), Qt::CaseInsensitive) == 0) {
                ++conflicts;
                conflictNames.append(incoming.value(QStringLiteral("name")).toString());
                break;
            }
        }
    }
    setLastError({});
    return {
        {QStringLiteral("valid"), true},
        {QStringLiteral("siteCount"), document.value(QStringLiteral("sites")).toList().size()},
        {QStringLiteral("areaCount"), document.value(QStringLiteral("areas")).toList().size()},
        {QStringLiteral("assignmentCount"), document.value(QStringLiteral("assignments")).toList().size()},
        {QStringLiteral("baselineCount"), document.value(QStringLiteral("baselines")).toList().size()},
        {QStringLiteral("conflictCount"), conflicts},
        {QStringLiteral("conflicts"), conflictNames},
        {QStringLiteral("containsCredentials"), containsSensitiveField(document)}
    };
}

bool FleetManager::importSiteDefinitions(const QString &pathOrUrl, bool merge)
{
    if (!canManageTopology()) return false;
    const QVariantMap preview = previewSiteImport(pathOrUrl);
    if (preview.isEmpty()) return false;
    if (preview.value(QStringLiteral("containsCredentials")).toBool()) {
        setLastError(QStringLiteral("Site definition contains credential fields"));
        return false;
    }
    if (!merge && preview.value(QStringLiteral("conflictCount")).toInt() > 0) {
        setLastError(QStringLiteral("Import conflicts must be resolved or merge must be enabled"));
        return false;
    }
    QString error;
    const QVariantMap document = readJsonObject(localPath(pathOrUrl), &error);
    auto mergeList = [merge](QVariantList *destination, const QVariantList &incoming,
                             int maximum) {
        int processed = 0;
        for (const QVariant &value : incoming) {
            if (++processed > maximum * 4) break;
            QVariantMap item = FleetManager::sanitizedConfiguration(value.toMap());
            const QString id = item.value(QStringLiteral("id")).toString().trimmed().left(160);
            const QString name = item.value(QStringLiteral("name")).toString().trimmed().left(120);
            if (id.isEmpty() || name.isEmpty()) continue;
            item.insert(QStringLiteral("id"), id);
            item.insert(QStringLiteral("name"), name);
            int existingIndex = -1;
            for (int index = 0; index < destination->size(); ++index) {
                const QVariantMap existing = destination->at(index).toMap();
                if (existing.value(QStringLiteral("id")).toString() == id
                    || existing.value(QStringLiteral("name")).toString()
                           .compare(name, Qt::CaseInsensitive) == 0) {
                    existingIndex = index;
                    break;
                }
            }
            if (existingIndex >= 0) {
                if (merge) (*destination)[existingIndex] = item;
            } else if (destination->size() < maximum) {
                destination->append(item);
            }
        }
    };
    mergeList(&m_sites, document.value(QStringLiteral("sites")).toList(), kMaximumSites);
    mergeList(&m_areas, document.value(QStringLiteral("areas")).toList(), kMaximumAreas);
    mergeList(&m_savedViews, document.value(QStringLiteral("savedViews")).toList(), kMaximumSavedViews);

    QSet<QString> validSiteIds;
    for (const QVariant &value : m_sites)
        validSiteIds.insert(value.toMap().value(QStringLiteral("id")).toString());
    for (int index = m_areas.size() - 1; index >= 0; --index) {
        if (!validSiteIds.contains(m_areas.at(index).toMap()
                                       .value(QStringLiteral("siteId")).toString())) {
            m_areas.removeAt(index);
        }
    }
    QSet<QString> validAreaIds;
    for (const QVariant &value : m_areas)
        validAreaIds.insert(value.toMap().value(QStringLiteral("id")).toString());

    QVariantList safeBaselines;
    for (const QVariant &value : document.value(QStringLiteral("baselines")).toList()) {
        if (safeBaselines.size() >= kMaximumBaselines) break;
        QVariantMap baseline = value.toMap();
        const QString baselineSite = baseline.value(QStringLiteral("siteId")).toString();
        const QString baselineArea = baseline.value(QStringLiteral("areaId")).toString();
        if ((!baselineSite.isEmpty() && !validSiteIds.contains(baselineSite))
            || (!baselineArea.isEmpty() && !validAreaIds.contains(baselineArea))) continue;
        baseline.insert(QStringLiteral("configuration"),
                        sanitizedConfiguration(baseline.value(QStringLiteral("configuration")).toMap()));
        safeBaselines.append(baseline);
    }
    mergeList(&m_baselines, safeBaselines, kMaximumBaselines);

    int assignmentCount = 0;
    for (const QVariant &value : document.value(QStringLiteral("assignments")).toList()) {
        if (++assignmentCount > 1024) break;
        const QVariantMap incoming = value.toMap();
        const QVariantMap reference = incoming.value(QStringLiteral("cameraRef")).toMap();
        int cameraIndex = cameraIndexForKey(reference.value(QStringLiteral("id")).toString());
        if (cameraIndex < 0) cameraIndex = cameraIndexForKey(reference.value(QStringLiteral("ip")).toString());
        if (cameraIndex < 0) continue;
        const QString key = cameraKeyAt(cameraIndex);
        const QVariantMap assignment = sanitizedConfiguration(incoming);
        assignCamera(key,
                     assignment.value(QStringLiteral("siteId")).toString(),
                     assignment.value(QStringLiteral("areaId")).toString(),
                     normalizedTags(stringList(assignment.value(QStringLiteral("tags")))),
                     assignment.value(QStringLiteral("maintenanceState")).toString());
    }
    emit topologyChanged();
    emit assignmentsChanged();
    emit savedViewsChanged();
    emit baselinesChanged();
    emit inventoryChanged();
    emit stateChanged();
    setLastError({});
    audit(QStringLiteral("sites.import"), localPath(pathOrUrl), QStringLiteral("success"),
          QStringLiteral("merge=%1").arg(merge));
    return true;
}

bool FleetManager::exportSiteDefinitions(const QString &pathOrUrl) const
{
    if (!canManageTopology()) {
        setLastError(QStringLiteral("Administrator permission is required"));
        return false;
    }
    QVariantList assignments;
    for (auto it = m_assignments.cbegin(); it != m_assignments.cend(); ++it) {
        const int index = cameraIndexForKey(it.key());
        const Camera camera = m_cameraModel ? m_cameraModel->getCamera(index) : Camera{};
        QVariantMap assignment = it.value();
        assignment.insert(QStringLiteral("cameraRef"), QVariantMap{
            {QStringLiteral("id"), it.key()},
            {QStringLiteral("name"), camera.name},
            {QStringLiteral("ip"), camera.ip}
        });
        assignments.append(assignment);
    }
    QVariantList baselines;
    for (const QVariant &value : m_baselines) {
        QVariantMap baseline = value.toMap();
        baseline.insert(QStringLiteral("configuration"),
                        sanitizedConfiguration(baseline.value(QStringLiteral("configuration")).toMap()));
        baselines.append(baseline);
    }
    QString error;
    const bool ok = writeJsonObject(localPath(pathOrUrl), {
        {QStringLiteral("format"), QStringLiteral("openipc-dashboard-sites")},
        {QStringLiteral("version"), 1},
        {QStringLiteral("exportedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
        {QStringLiteral("credentialsIncluded"), false},
        {QStringLiteral("sites"), sanitizeVariant(m_sites, QStringLiteral("sites"))},
        {QStringLiteral("areas"), sanitizeVariant(m_areas, QStringLiteral("areas"))},
        {QStringLiteral("assignments"), assignments},
        {QStringLiteral("savedViews"), sanitizeVariant(m_savedViews, QStringLiteral("savedViews"))},
        {QStringLiteral("baselines"), baselines}
    }, &error);
    setLastError(ok ? QString() : error);
    audit(QStringLiteral("sites.export"), localPath(pathOrUrl),
          ok ? QStringLiteral("success") : QStringLiteral("failed"));
    return ok;
}

QJsonObject FleetManager::toJson() const
{
    QVariantList assignments;
    for (auto it = m_assignments.cbegin(); it != m_assignments.cend(); ++it) {
        QVariantMap value = it.value();
        value.insert(QStringLiteral("cameraId"), it.key());
        assignments.append(value);
    }
    QVariantList metadata;
    for (auto it = m_deviceMetadata.cbegin(); it != m_deviceMetadata.cend(); ++it) {
        QVariantMap value = sanitizedConfiguration(it.value());
        value.insert(QStringLiteral("cameraId"), it.key());
        metadata.append(value);
    }
    QVariantList configurations;
    for (auto it = m_deviceConfigurations.cbegin(); it != m_deviceConfigurations.cend(); ++it) {
        configurations.append(QVariantMap{
            {QStringLiteral("cameraId"), it.key()},
            {QStringLiteral("configuration"), sanitizedConfiguration(it.value())}
        });
    }
    return QJsonObject::fromVariantMap({
        {QStringLiteral("schemaVersion"), 1},
        {QStringLiteral("sites"), sanitizeVariant(m_sites, QStringLiteral("sites"))},
        {QStringLiteral("areas"), sanitizeVariant(m_areas, QStringLiteral("areas"))},
        {QStringLiteral("assignments"), assignments},
        {QStringLiteral("savedViews"), sanitizeVariant(m_savedViews, QStringLiteral("savedViews"))},
        {QStringLiteral("baselines"), sanitizeVariant(m_baselines, QStringLiteral("baselines"))},
        {QStringLiteral("deviceMetadata"), metadata},
        {QStringLiteral("deviceConfigurations"), configurations},
        {QStringLiteral("batchHistory"), m_batchHistory}
    });
}

void FleetManager::restoreJson(const QJsonObject &object)
{
    const QVariantMap map = object.toVariantMap();
    auto restoredDefinitions = [](const QVariant &source, int maximum) {
        QVariantList result;
        for (const QVariant &value : source.toList()) {
            QVariantMap item = FleetManager::sanitizedConfiguration(value.toMap());
            const QString id = FleetManager::normalizedId(item.value(QStringLiteral("id")).toString());
            const QString name = item.value(QStringLiteral("name")).toString().trimmed().left(120);
            if (id.isEmpty() || name.isEmpty()) continue;
            item.insert(QStringLiteral("id"), id);
            item.insert(QStringLiteral("name"), name);
            result.append(item);
            if (result.size() >= maximum) break;
        }
        return result;
    };
    m_sites = restoredDefinitions(map.value(QStringLiteral("sites")), kMaximumSites);
    QSet<QString> validSiteIds;
    for (const QVariant &value : m_sites)
        validSiteIds.insert(value.toMap().value(QStringLiteral("id")).toString());
    m_areas.clear();
    for (const QVariant &value : restoredDefinitions(map.value(QStringLiteral("areas")), kMaximumAreas)) {
        const QVariantMap area = value.toMap();
        if (validSiteIds.contains(area.value(QStringLiteral("siteId")).toString())) m_areas.append(area);
    }
    QSet<QString> validAreaIds;
    for (const QVariant &value : m_areas)
        validAreaIds.insert(value.toMap().value(QStringLiteral("id")).toString());
    m_savedViews = restoredDefinitions(map.value(QStringLiteral("savedViews")), kMaximumSavedViews);
    m_baselines.clear();
    for (const QVariant &value : map.value(QStringLiteral("baselines")).toList()) {
        QVariantMap baseline = value.toMap();
        const QString baselineSite = baseline.value(QStringLiteral("siteId")).toString();
        const QString baselineArea = baseline.value(QStringLiteral("areaId")).toString();
        if ((!baselineSite.isEmpty() && !validSiteIds.contains(baselineSite))
            || (!baselineArea.isEmpty() && !validAreaIds.contains(baselineArea))) continue;
        baseline.insert(QStringLiteral("configuration"),
                        sanitizedConfiguration(baseline.value(QStringLiteral("configuration")).toMap()));
        if (!baseline.value(QStringLiteral("id")).toString().isEmpty()
            && !baseline.value(QStringLiteral("name")).toString().isEmpty()) {
            m_baselines.append(baseline);
        }
        if (m_baselines.size() >= kMaximumBaselines) break;
    }
    m_assignments.clear();
    for (const QVariant &value : map.value(QStringLiteral("assignments")).toList()) {
        QVariantMap assignment = sanitizedConfiguration(value.toMap());
        const QString cameraId = normalizedId(assignment.take(QStringLiteral("cameraId")).toString());
        const QString siteId = normalizedId(assignment.value(QStringLiteral("siteId")).toString());
        const QString areaId = normalizedId(assignment.value(QStringLiteral("areaId")).toString());
        if (cameraId.isEmpty()
            || (!siteId.isEmpty() && !validSiteIds.contains(siteId))
            || (!areaId.isEmpty() && !validAreaIds.contains(areaId))) continue;
        assignment.insert(QStringLiteral("siteId"), siteId);
        assignment.insert(QStringLiteral("areaId"), areaId);
        assignment.insert(QStringLiteral("tags"),
                          normalizedTags(stringList(assignment.value(QStringLiteral("tags")))));
        const QString state = assignment.value(QStringLiteral("maintenanceState")).toString();
        assignment.insert(QStringLiteral("maintenanceState"),
                          state == QStringLiteral("maintenance") || state == QStringLiteral("retired")
                              ? state : QStringLiteral("active"));
        m_assignments.insert(cameraId, assignment);
    }
    m_deviceMetadata.clear();
    for (const QVariant &value : map.value(QStringLiteral("deviceMetadata")).toList()) {
        QVariantMap metadata = value.toMap();
        const QString cameraId = normalizedId(metadata.take(QStringLiteral("cameraId")).toString());
        if (!cameraId.isEmpty()) m_deviceMetadata.insert(cameraId, sanitizedConfiguration(metadata));
    }
    m_deviceConfigurations.clear();
    for (const QVariant &value : map.value(QStringLiteral("deviceConfigurations")).toList()) {
        const QVariantMap record = value.toMap();
        const QString cameraId = normalizedId(record.value(QStringLiteral("cameraId")).toString());
        if (!cameraId.isEmpty()) {
            m_deviceConfigurations.insert(cameraId, sanitizedConfiguration(
                record.value(QStringLiteral("configuration")).toMap()));
        }
    }
    m_batchHistory = map.value(QStringLiteral("batchHistory")).toList().mid(0, kMaximumBatchHistory);
    emit topologyChanged();
    emit assignmentsChanged();
    emit savedViewsChanged();
    emit baselinesChanged();
    emit batchHistoryChanged();
    emit inventoryChanged();
}
