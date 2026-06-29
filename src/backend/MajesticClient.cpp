#include "MajesticClient.h"

#include "PathUtils.h"

#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMetaObject>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>

#include <algorithm>

namespace {

constexpr int kTransferTimeoutMs = 15000;
constexpr qsizetype kMaximumConfigBytes = 1024 * 1024;

QVariantMap variantMap(const QVariant &value)
{
    return value.canConvert<QVariantMap>() ? value.toMap() : QVariantMap{};
}

QString endpointKey(const QString &host, int port)
{
    const QString raw = host.trimmed();
    const QUrl url(raw.contains(QStringLiteral("://")) ? raw : QStringLiteral("http://") + raw);
    const int defaultPort = url.scheme() == QStringLiteral("https") ? 443 : 80;
    return url.host().toLower() + QLatin1Char('|')
        + QString::number(port > 0 ? port : url.port(defaultPort));
}

bool sensitivePath(const QString &path)
{
    return QRegularExpression(QStringLiteral("(^|\\.)(password|passwd|secret|token)(\\.|$)"),
                              QRegularExpression::CaseInsensitiveOption)
               .match(path)
               .hasMatch()
        || path == QStringLiteral("outgoing.server");
}

bool variantsEqual(const QVariant &left, const QVariant &right)
{
    return QJsonValue::fromVariant(left) == QJsonValue::fromVariant(right);
}

QVariantMap buildPatchRecursive(const QVariantMap &original, const QVariantMap &edited)
{
    QVariantMap patch;
    for (auto it = edited.cbegin(); it != edited.cend(); ++it) {
        const QVariant before = original.value(it.key());
        const QVariantMap beforeMap = variantMap(before);
        const QVariantMap afterMap = variantMap(it.value());
        if (!afterMap.isEmpty() || it.value().metaType().id() == QMetaType::QVariantMap) {
            const QVariantMap childPatch = buildPatchRecursive(beforeMap, afterMap);
            if (!childPatch.isEmpty()) {
                patch.insert(it.key(), childPatch);
            }
        } else if (!original.contains(it.key()) || !variantsEqual(before, it.value())) {
            patch.insert(it.key(), it.value());
        }
    }
    return patch;
}

void describeRecursive(const QVariantMap &original, const QVariantMap &edited,
                       const QString &prefix, QVariantList *changes)
{
    for (auto it = edited.cbegin(); it != edited.cend(); ++it) {
        const QString path = prefix.isEmpty() ? it.key() : prefix + QLatin1Char('.') + it.key();
        const QVariant before = original.value(it.key());
        const QVariantMap beforeMap = variantMap(before);
        const QVariantMap afterMap = variantMap(it.value());
        if (!afterMap.isEmpty() || it.value().metaType().id() == QMetaType::QVariantMap) {
            describeRecursive(beforeMap, afterMap, path, changes);
            continue;
        }
        if (original.contains(it.key()) && variantsEqual(before, it.value())) {
            continue;
        }
        const bool sensitive = sensitivePath(path);
        QVariantMap change;
        change.insert(QStringLiteral("path"), path);
        change.insert(QStringLiteral("before"), sensitive ? QVariant(QStringLiteral("••••")) : before);
        change.insert(QStringLiteral("after"), sensitive ? QVariant(QStringLiteral("••••")) : it.value());
        change.insert(QStringLiteral("sensitive"), sensitive);
        changes->append(change);
    }
}

void collectSchemaPaths(const QVariantMap &node, const QString &prefix, QSet<QString> *paths)
{
    const QVariantMap properties = node.value(QStringLiteral("properties")).toMap();
    for (auto it = properties.cbegin(); it != properties.cend(); ++it) {
        const QString path = prefix.isEmpty() ? it.key() : prefix + QLatin1Char('.') + it.key();
        const QVariantMap child = it.value().toMap();
        if (child.value(QStringLiteral("type")).toString() == QStringLiteral("object")
            && !child.value(QStringLiteral("properties")).toMap().isEmpty()) {
            collectSchemaPaths(child, path, paths);
        } else {
            paths->insert(path);
        }
    }
}

void collectSchemaDescriptors(const QVariantMap &node, const QString &prefix,
                              QHash<QString, QVariantMap> *descriptors)
{
    const QVariantMap properties = node.value(QStringLiteral("properties")).toMap();
    for (auto it = properties.cbegin(); it != properties.cend(); ++it) {
        const QString path = prefix.isEmpty() ? it.key() : prefix + QLatin1Char('.') + it.key();
        const QVariantMap child = it.value().toMap();
        if (child.value(QStringLiteral("type")).toString() == QStringLiteral("object")
            && !child.value(QStringLiteral("properties")).toMap().isEmpty()) {
            collectSchemaDescriptors(child, path, descriptors);
        } else {
            descriptors->insert(path, child);
        }
    }
}

QVariant patchValueAtPath(const QVariantMap &patch, const QString &path)
{
    QVariant current = patch;
    for (const QString &part : path.split(QLatin1Char('.'), Qt::SkipEmptyParts)) {
        const QVariantMap map = current.toMap();
        if (!map.contains(part)) return {};
        current = map.value(part);
    }
    return current;
}

void collectPatchPaths(const QVariantMap &patch, const QString &prefix, QStringList *paths)
{
    for (auto it = patch.cbegin(); it != patch.cend(); ++it) {
        const QString path = prefix.isEmpty() ? it.key() : prefix + QLatin1Char('.') + it.key();
        const QVariantMap child = variantMap(it.value());
        if (!child.isEmpty() || it.value().metaType().id() == QMetaType::QVariantMap) {
            collectPatchPaths(child, path, paths);
        } else {
            paths->append(path);
        }
    }
}

QString inferredType(const QVariant &value)
{
    switch (value.metaType().id()) {
    case QMetaType::Bool:
        return QStringLiteral("boolean");
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
        return QStringLiteral("integer");
    case QMetaType::Double:
        return QStringLiteral("number");
    case QMetaType::QVariantList:
    case QMetaType::QStringList:
        return QStringLiteral("array");
    default:
        return QStringLiteral("string");
    }
}

void appendFields(const QVariantMap &schemaNode, const QVariantMap &configNode,
                  const QString &prefix, const QString &section,
                  const QString &sectionLabel, const QString &groupId, const QString &groupLabel,
                  QVariantList *fields)
{
    QVariantMap properties = schemaNode.value(QStringLiteral("properties")).toMap();
    if (properties.isEmpty()) {
        for (auto it = configNode.cbegin(); it != configNode.cend(); ++it) {
            QVariantMap inferred;
            const QVariantMap childMap = variantMap(it.value());
            inferred.insert(QStringLiteral("type"), childMap.isEmpty()
                                ? inferredType(it.value()) : QStringLiteral("object"));
            properties.insert(it.key(), inferred);
        }
    }

    for (auto it = properties.cbegin(); it != properties.cend(); ++it) {
        const QVariantMap descriptor = it.value().toMap();
        const QString path = prefix.isEmpty() ? it.key() : prefix + QLatin1Char('.') + it.key();
        const QVariantMap nested = descriptor.value(QStringLiteral("properties")).toMap();
        if (!nested.isEmpty() || descriptor.value(QStringLiteral("type")).toString() == QStringLiteral("object")) {
            appendFields(descriptor, configNode.value(it.key()).toMap(), path, section,
                         sectionLabel, groupId, groupLabel, fields);
            continue;
        }

        QVariantMap field;
        field.insert(QStringLiteral("path"), path);
        field.insert(QStringLiteral("key"), it.key());
        field.insert(QStringLiteral("section"), section);
        field.insert(QStringLiteral("sectionLabel"), sectionLabel.isEmpty() ? section : sectionLabel);
        field.insert(QStringLiteral("groupId"), groupId);
        field.insert(QStringLiteral("groupLabel"), groupLabel);
        field.insert(QStringLiteral("title"), descriptor.value(QStringLiteral("title"), it.key()));
        field.insert(QStringLiteral("description"), descriptor.value(QStringLiteral("description")));
        field.insert(QStringLiteral("hint"), descriptor.value(QStringLiteral("hint")));
        field.insert(QStringLiteral("type"), descriptor.value(QStringLiteral("type"),
                                                               inferredType(configNode.value(it.key()))));
        field.insert(QStringLiteral("value"), configNode.value(it.key()));
        field.insert(QStringLiteral("defaultValue"), descriptor.value(QStringLiteral("default")));
        field.insert(QStringLiteral("hasDefault"), descriptor.contains(QStringLiteral("default")));
        field.insert(QStringLiteral("enumValues"), descriptor.value(QStringLiteral("enum")).toList());
        field.insert(QStringLiteral("minimum"), descriptor.value(QStringLiteral("minimum")));
        field.insert(QStringLiteral("maximum"), descriptor.value(QStringLiteral("maximum")));
        field.insert(QStringLiteral("step"), descriptor.value(QStringLiteral("multipleOf")));
        field.insert(QStringLiteral("live"), descriptor.value(QStringLiteral("x-live"), false));
        field.insert(QStringLiteral("resolution"), descriptor.value(QStringLiteral("x-resolution"), false));
        field.insert(QStringLiteral("xMin"), descriptor.value(QStringLiteral("x-min")));
        field.insert(QStringLiteral("xMax"), descriptor.value(QStringLiteral("x-max")));
        field.insert(QStringLiteral("xNative"), descriptor.value(QStringLiteral("x-native")));
        field.insert(QStringLiteral("visibleWhen"), descriptor.value(QStringLiteral("visibleWhen")).toMap());
        const bool sensitive = sensitivePath(path);
        field.insert(QStringLiteral("sensitive"), sensitive);
        fields->append(field);
    }
}

QByteArray limitedBody(const QByteArray &body)
{
    constexpr qsizetype maximum = 4096;
    return body.size() <= maximum ? body : body.left(maximum) + QByteArrayLiteral("…");
}

bool findJsonNull(const QJsonValue &value, const QString &prefix, QString *path)
{
    if (value.isNull() || value.isUndefined()) {
        if (path) *path = prefix.isEmpty() ? QStringLiteral("$") : prefix;
        return true;
    }
    if (value.isArray()) {
        const QJsonArray array = value.toArray();
        for (qsizetype i = 0; i < array.size(); ++i) {
            const QString childPath = prefix + QStringLiteral("[%1]").arg(i);
            if (findJsonNull(array.at(i), childPath, path)) return true;
        }
    }
    if (value.isObject()) {
        const QJsonObject object = value.toObject();
        for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
            const QString childPath = prefix.isEmpty()
                ? it.key()
                : prefix + QLatin1Char('.') + it.key();
            if (findJsonNull(it.value(), childPath, path)) return true;
        }
    }
    return false;
}

} // namespace

MajesticClient::MajesticClient(QObject *parent)
    : QObject(parent)
{
}

QString MajesticClient::newRequestId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QNetworkRequest MajesticClient::makeRequest(const QString &host, int port, const QString &path,
                                             const QString &username, const QString &password) const
{
    QUrl relative;
    relative.setPath(path);
    return makeRequest(host, port, relative, username, password);
}

QNetworkRequest MajesticClient::makeRequest(const QString &host, int port, const QUrl &relativeUrl,
                                             const QString &username, const QString &password) const
{
    const QString trimmedHost = host.trimmed();
    QUrl url(trimmedHost.contains(QStringLiteral("://"))
                 ? trimmedHost : QStringLiteral("http://") + trimmedHost);
    if (url.scheme().isEmpty()) {
        url.setScheme(QStringLiteral("http"));
    }
    if (port > 0) {
        url.setPort(port);
    } else if (url.port() < 0) {
        url.setPort(url.scheme() == QStringLiteral("https") ? 443 : 80);
    }
    url.setPath(relativeUrl.path());
    url.setQuery(relativeUrl.query());

    QNetworkRequest request(url);
    request.setTransferTimeout(kTransferTimeoutMs);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", "OpenIPC-Dashboard/0.2");
    request.setRawHeader("Accept", "application/json, text/plain, */*");
    if (!username.isEmpty()) {
        request.setRawHeader("Authorization", "Basic "
            + (username + QLatin1Char(':') + password).toUtf8().toBase64());
    }
    return request;
}

bool MajesticClient::parseJsonMap(const QByteArray &data, QVariantMap *result, QString *error)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (error) {
            *error = parseError.errorString();
        }
        return false;
    }
    if (!document.isObject()) {
        if (error) {
            *error = QStringLiteral("Expected a JSON object");
        }
        return false;
    }
    if (result) {
        *result = document.object().toVariantMap();
    }
    return true;
}

QString MajesticClient::loadConfiguration(const QString &host, int port,
                                           const QString &username, const QString &password)
{
    const QString requestId = newRequestId();
    QNetworkReply *configReply = m_networkManager.get(
        makeRequest(host, port, QStringLiteral("/api/v1/config.json"), username, password));
    connect(configReply, &QNetworkReply::finished, this,
            [this, configReply, requestId, host, port, username, password]() {
        const QByteArray body = configReply->readAll();
        if (configReply->error() != QNetworkReply::NoError) {
            emitFailure(requestId, QStringLiteral("load-config"), configReply, body);
            configReply->deleteLater();
            return;
        }

        QVariantMap config;
        QString parseError;
        if (!parseJsonMap(body, &config, &parseError)) {
            emit operationFailed(requestId, QStringLiteral("load-config"),
                                 QStringLiteral("Invalid config JSON: %1").arg(parseError), 0);
            emit requestFailed(QStringLiteral("load-config"), parseError);
            configReply->deleteLater();
            return;
        }
        configReply->deleteLater();

        QNetworkReply *schemaReply = m_networkManager.get(
            makeRequest(host, port, QStringLiteral("/api/v1/config.schema.json"),
                        username, password));
        connect(schemaReply, &QNetworkReply::finished, this,
                [this, schemaReply, requestId, config]() {
            const QByteArray schemaBody = schemaReply->readAll();
            QVariantMap schema;
            QString parseError;
            const bool schemaAvailable = schemaReply->error() == QNetworkReply::NoError
                && parseJsonMap(schemaBody, &schema, &parseError);
            if (!schemaAvailable) {
                schema.clear(); // Older Majestic builds have config.json but no schema endpoint.
            }
            m_schemaByEndpoint.insert(endpointKey(schemaReply->url().host(), schemaReply->url().port(80)),
                                      schema);
            const QVariantList fields = flattenFieldsForTest(schema, config);
            const QVariantMap caps = capabilities(schema, config);
            emit configurationLoaded(requestId, config, schema, fields, caps);
            emit operationSucceeded(requestId, QStringLiteral("load-config"),
                                    schemaAvailable ? QStringLiteral("schema-v1")
                                                    : QStringLiteral("legacy-config"));
            emit requestSucceeded(QStringLiteral("load-config"),
                                  schemaAvailable ? QStringLiteral("schema-v1")
                                                  : QStringLiteral("legacy-config"));
            schemaReply->deleteLater();
        });
    });
    return requestId;
}

QString MajesticClient::applyConfiguration(const QString &host, int port,
                                            const QString &username, const QString &password,
                                            const QVariantMap &patch)
{
    const QString requestId = newRequestId();
    if (patch.isEmpty()) {
        emitFailureLater(requestId, QStringLiteral("apply-config"),
                         QStringLiteral("Configuration patch is empty"));
        return requestId;
    }

    QString validationError;
    const QVariantMap schema = m_schemaByEndpoint.value(endpointKey(host, port));
    if (!validatePatch(patch, schema, &validationError)) {
        emitFailureLater(requestId, QStringLiteral("apply-config"), validationError);
        return requestId;
    }

    const QByteArray body = QJsonDocument::fromVariant(patch).toJson(QJsonDocument::Compact);
    if (body.size() > kMaximumConfigBytes) {
        const QString message = QStringLiteral("Configuration patch exceeds Majestic's 1 MiB limit");
        emitFailureLater(requestId, QStringLiteral("apply-config"), message);
        return requestId;
    }

    QNetworkRequest request = makeRequest(host, port, QStringLiteral("/api/v1/config"),
                                          username, password);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    QNetworkReply *reply = m_networkManager.post(request, body);
    handleSimpleReply(reply, requestId, QStringLiteral("apply-config"),
                      [this, requestId](const QByteArray &) {
        emit configurationApplied(requestId);
    });
    return requestId;
}

QString MajesticClient::resetConfigurationFields(const QString &host, int port,
                                                  const QString &username, const QString &password,
                                                  const QStringList &fieldPaths)
{
    const QString requestId = newRequestId();
    static const QRegularExpression validPath(QStringLiteral("^[A-Za-z0-9_.-]+$"));
    QStringList normalized;
    for (const QString &path : fieldPaths) {
        const QString trimmed = path.trimmed();
        if (trimmed.isEmpty() || !validPath.match(trimmed).hasMatch()) {
            const QString message = QStringLiteral("Invalid configuration path: %1").arg(path);
            emitFailureLater(requestId, QStringLiteral("reset-config"), message);
            return requestId;
        }
        if (!normalized.contains(trimmed)) {
            normalized.append(trimmed);
        }
    }
    if (normalized.isEmpty() || normalized.size() > 64) {
        const QString message = QStringLiteral("Reset requires between 1 and 64 fields");
        emitFailureLater(requestId, QStringLiteral("reset-config"), message);
        return requestId;
    }

    QUrl relative;
    relative.setPath(QStringLiteral("/api/v1/reset"));
    QUrlQuery query;
    for (const QString &path : std::as_const(normalized)) {
        query.addQueryItem(QStringLiteral("key"), path);
    }
    relative.setQuery(query);
    QNetworkReply *reply = m_networkManager.get(makeRequest(host, port, relative, username, password));
    handleSimpleReply(reply, requestId, QStringLiteral("reset-config"),
                      [this, requestId, normalized](const QByteArray &) {
        emit configurationFieldsReset(requestId, normalized);
    });
    return requestId;
}

QString MajesticClient::applyLiveImage(const QString &host, int port,
                                       const QString &username, const QString &password,
                                       const QVariantMap &values)
{
    const QString requestId = newRequestId();
    const QSet<QString> allowed{QStringLiteral("luminance"), QStringLiteral("contrast"),
                                QStringLiteral("saturation"), QStringLiteral("hue"),
                                QStringLiteral("mirror"), QStringLiteral("flip")};
    QUrlQuery query;
    for (auto it = values.cbegin(); it != values.cend(); ++it) {
        if (!allowed.contains(it.key())) {
            const QString message = QStringLiteral("Unsupported live image key: %1").arg(it.key());
            emitFailureLater(requestId, QStringLiteral("live-image"), message);
            return requestId;
        }
        const QString value = it.value().metaType().id() == QMetaType::Bool
            ? (it.value().toBool() ? QStringLiteral("1") : QStringLiteral("0"))
            : it.value().toString();
        query.addQueryItem(it.key(), value);
    }
    if (query.isEmpty()) {
        const QString message = QStringLiteral("Live image patch is empty");
        emitFailureLater(requestId, QStringLiteral("live-image"), message);
        return requestId;
    }

    QUrl relative;
    relative.setPath(QStringLiteral("/api/v1/image"));
    relative.setQuery(query);
    QNetworkReply *reply = m_networkManager.post(
        makeRequest(host, port, relative, username, password), QByteArray{});
    handleSimpleReply(reply, requestId, QStringLiteral("live-image"));
    return requestId;
}

QString MajesticClient::reloadPipeline(const QString &host, int port,
                                       const QString &username, const QString &password)
{
    const QString requestId = newRequestId();
    QNetworkReply *reply = m_networkManager.get(makeRequest(
        host, port, QStringLiteral("/cgi-bin/j/mj-apply.cgi"), username, password));
    handleSimpleReply(reply, requestId, QStringLiteral("reload-pipeline"));
    return requestId;
}

QString MajesticClient::loadMetrics(const QString &host, int port,
                                    const QString &username, const QString &password)
{
    const QString requestId = newRequestId();
    QNetworkReply *reply = m_networkManager.get(
        makeRequest(host, port, QStringLiteral("/metrics"), username, password));
    handleSimpleReply(reply, requestId, QStringLiteral("metrics"),
                      [this, requestId](const QByteArray &body) {
        const QString raw = QString::fromUtf8(body);
        emit metricsLoaded(requestId, metricsFromText(raw), raw);
    });
    return requestId;
}

QString MajesticClient::takeSnapshot(const QString &host, int port, const QString &username,
                                     const QString &password, const QString &destinationPath,
                                     int width, int height, int quality, bool grayscale)
{
    const QString requestId = newRequestId();
    const QString path = PathUtils::localPathFromUserInput(destinationPath);
    if (path.isEmpty()) {
        emit operationFailed(requestId, QStringLiteral("snapshot"),
                             QStringLiteral("Snapshot destination is empty"), 0);
        return requestId;
    }
    QUrl relative;
    relative.setPath(QStringLiteral("/image.jpg"));
    QUrlQuery query;
    if (width > 0) query.addQueryItem(QStringLiteral("width"), QString::number(width));
    if (height > 0) query.addQueryItem(QStringLiteral("height"), QString::number(height));
    if (quality > 0) query.addQueryItem(QStringLiteral("qfactor"), QString::number(std::clamp(quality, 1, 100)));
    if (grayscale) query.addQueryItem(QStringLiteral("color2gray"), QStringLiteral("1"));
    relative.setQuery(query);

    QNetworkReply *reply = m_networkManager.get(makeRequest(host, port, relative, username, password));
    connect(reply, &QNetworkReply::finished, this, [this, reply, requestId, path]() {
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            emitFailure(requestId, QStringLiteral("snapshot"), reply, body);
        } else {
            QSaveFile file(path);
            if (!file.open(QIODevice::WriteOnly) || file.write(body) != body.size() || !file.commit()) {
                const QString message = file.errorString();
                emit operationFailed(requestId, QStringLiteral("snapshot"), message, 0);
                emit requestFailed(QStringLiteral("snapshot"), message);
            } else {
                emit snapshotSaved(path);
                emit operationSucceeded(requestId, QStringLiteral("snapshot"), path);
                emit requestSucceeded(QStringLiteral("snapshot"), path);
            }
        }
        reply->deleteLater();
    });
    return requestId;
}

QString MajesticClient::setNightMode(const QString &host, int port, const QString &username,
                                     const QString &password, const QString &mode)
{
    const QString requestId = newRequestId();
    const QString normalized = mode.trimmed().toLower();
    const QSet<QString> allowed{QStringLiteral("on"), QStringLiteral("off"),
                                QStringLiteral("toggle"), QStringLiteral("ircut"),
                                QStringLiteral("light")};
    if (!allowed.contains(normalized)) {
        const QString message = QStringLiteral("Invalid night action: %1").arg(mode);
        emit operationFailed(requestId, QStringLiteral("night-mode"), message, 0);
        emit requestFailed(QStringLiteral("night-mode"), message);
        return requestId;
    }
    QNetworkReply *reply = m_networkManager.get(makeRequest(
        host, port, QStringLiteral("/night/") + normalized, username, password));
    handleSimpleReply(reply, requestId, QStringLiteral("night-mode"));
    return requestId;
}

QString MajesticClient::playPcmFile(const QString &host, int port, const QString &username,
                                    const QString &password, const QString &filePath)
{
    const QString requestId = newRequestId();
    QFile file(PathUtils::localPathFromUserInput(filePath));
    if (!file.open(QIODevice::ReadOnly)) {
        emit operationFailed(requestId, QStringLiteral("play-audio"), file.errorString(), 0);
        emit requestFailed(QStringLiteral("play-audio"), file.errorString());
        return requestId;
    }
    if (file.size() > 32 * 1024 * 1024) {
        const QString message = QStringLiteral("PCM file is larger than the 32 MiB safety limit");
        emit operationFailed(requestId, QStringLiteral("play-audio"), message, 0);
        emit requestFailed(QStringLiteral("play-audio"), message);
        return requestId;
    }

    QNetworkRequest request = makeRequest(host, port, QStringLiteral("/play_audio"), username, password);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/octet-stream"));
    QNetworkReply *reply = m_networkManager.post(request, file.readAll());
    handleSimpleReply(reply, requestId, QStringLiteral("play-audio"));
    return requestId;
}

QString MajesticClient::saveConfigurationBackup(const QVariantMap &config,
                                                 const QVariantMap &schema,
                                                 const QString &destinationPath)
{
    const QString requestId = newRequestId();
    const QString path = PathUtils::localPathFromUserInput(destinationPath);
    QVariantMap backup;
    backup.insert(QStringLiteral("format"), QStringLiteral("openipc-dashboard-majestic-backup"));
    backup.insert(QStringLiteral("version"), 1);
    backup.insert(QStringLiteral("createdAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    backup.insert(QStringLiteral("config"), config);
    backup.insert(QStringLiteral("schema"), schema);
    const QByteArray data = QJsonDocument::fromVariant(backup).toJson(QJsonDocument::Indented);

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size() || !file.commit()) {
        emit operationFailed(requestId, QStringLiteral("backup-save"), file.errorString(), 0);
        emit requestFailed(QStringLiteral("backup-save"), file.errorString());
        return requestId;
    }
    emit backupSaved(requestId, path);
    emit operationSucceeded(requestId, QStringLiteral("backup-save"), path);
    emit requestSucceeded(QStringLiteral("backup-save"), path);
    return requestId;
}

QString MajesticClient::loadConfigurationBackup(const QString &sourcePath)
{
    const QString requestId = newRequestId();
    const QString path = PathUtils::localPathFromUserInput(sourcePath);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        emit operationFailed(requestId, QStringLiteral("backup-load"), file.errorString(), 0);
        emit requestFailed(QStringLiteral("backup-load"), file.errorString());
        return requestId;
    }
    if (file.size() > 4 * 1024 * 1024) {
        const QString message = QStringLiteral("Backup is larger than the 4 MiB safety limit");
        emit operationFailed(requestId, QStringLiteral("backup-load"), message, 0);
        emit requestFailed(QStringLiteral("backup-load"), message);
        return requestId;
    }
    QVariantMap document;
    QString error;
    if (!parseJsonMap(file.readAll(), &document, &error)) {
        emit operationFailed(requestId, QStringLiteral("backup-load"), error, 0);
        emit requestFailed(QStringLiteral("backup-load"), error);
        return requestId;
    }

    QVariantMap config;
    QVariantMap schema;
    if (document.value(QStringLiteral("format")).toString()
        == QStringLiteral("openipc-dashboard-majestic-backup")) {
        config = document.value(QStringLiteral("config")).toMap();
        schema = document.value(QStringLiteral("schema")).toMap();
    } else {
        config = document; // Also accept a plain config.json export.
    }
    if (config.isEmpty()) {
        const QString message = QStringLiteral("Backup does not contain a Majestic configuration");
        emit operationFailed(requestId, QStringLiteral("backup-load"), message, 0);
        emit requestFailed(QStringLiteral("backup-load"), message);
        return requestId;
    }
    emit backupLoaded(requestId, config, schema, path);
    emit operationSucceeded(requestId, QStringLiteral("backup-load"), path);
    emit requestSucceeded(QStringLiteral("backup-load"), path);
    return requestId;
}

QVariantMap MajesticClient::buildPatch(const QVariantMap &original, const QVariantMap &edited) const
{
    return buildPatchForTest(original, edited);
}

QVariantList MajesticClient::describeChanges(const QVariantMap &original,
                                              const QVariantMap &edited) const
{
    return describeChangesForTest(original, edited);
}

QVariantList MajesticClient::flattenFields(const QVariantMap &schema,
                                            const QVariantMap &config) const
{
    return flattenFieldsForTest(schema, config);
}

QVariantMap MajesticClient::parseJsonObject(const QString &json) const
{
    QVariantMap result;
    QString error;
    QVariantMap value;
    const bool ok = parseJsonMap(json.toUtf8(), &value, &error);
    result.insert(QStringLiteral("ok"), ok);
    result.insert(QStringLiteral("error"), error);
    result.insert(QStringLiteral("value"), value);
    return result;
}

QVariantMap MajesticClient::buildPatchForTest(const QVariantMap &original,
                                               const QVariantMap &edited)
{
    return buildPatchRecursive(original, edited);
}

QVariantList MajesticClient::describeChangesForTest(const QVariantMap &original,
                                                     const QVariantMap &edited)
{
    QVariantList changes;
    describeRecursive(original, edited, {}, &changes);
    return changes;
}

QVariantList MajesticClient::flattenFieldsForTest(const QVariantMap &schema,
                                                   const QVariantMap &config)
{
    QVariantList fields;
    const QVariantMap properties = schema.value(QStringLiteral("properties")).toMap();
    const QVariantList groups = schema.value(QStringLiteral("x-groups")).toList();
    QSet<QString> groupedSections;

    for (const QVariant &groupValue : groups) {
        const QVariantMap group = groupValue.toMap();
        const QString groupId = group.value(QStringLiteral("id")).toString();
        const QString groupLabel = group.value(QStringLiteral("label"), groupId).toString();
        for (const QVariant &sectionValue : group.value(QStringLiteral("sections")).toList()) {
            const QString section = sectionValue.toString();
            if (!properties.contains(section) && !config.contains(section)) {
                continue;
            }
            groupedSections.insert(section);
            const QVariantMap sectionSchema = properties.value(section).toMap();
            const QString sectionLabel = sectionSchema.value(QStringLiteral("title"), section).toString();
            appendFields(properties.value(section).toMap(), config.value(section).toMap(), section,
                         section, sectionLabel, groupId, groupLabel, &fields);
        }
    }

    QSet<QString> sections;
    for (auto it = properties.cbegin(); it != properties.cend(); ++it) sections.insert(it.key());
    for (auto it = config.cbegin(); it != config.cend(); ++it) sections.insert(it.key());
    QStringList sortedSections = sections.values();
    std::sort(sortedSections.begin(), sortedSections.end());
    for (const QString &section : std::as_const(sortedSections)) {
        if (groupedSections.contains(section)) continue;
        const QVariantMap sectionSchema = properties.value(section).toMap();
        const QString sectionLabel = sectionSchema.value(QStringLiteral("title"), section).toString();
        appendFields(properties.value(section).toMap(), config.value(section).toMap(), section,
                     section, sectionLabel, section, section, &fields);
    }
    return fields;
}

QVariantMap MajesticClient::capabilities(const QVariantMap &schema, const QVariantMap &config)
{
    Q_UNUSED(config)
    QVariantMap result;
    result.insert(QStringLiteral("configRead"), true);
    result.insert(QStringLiteral("schema"), !schema.isEmpty());
    result.insert(QStringLiteral("configWrite"), !schema.isEmpty());
    result.insert(QStringLiteral("resetDefaults"), !schema.isEmpty());
    result.insert(QStringLiteral("snapshot"), true);
    result.insert(QStringLiteral("nightControl"), true);
    result.insert(QStringLiteral("metrics"), true);
    result.insert(QStringLiteral("playAudio"), true);
    result.insert(QStringLiteral("pipelineReload"), true);

    bool liveImage = false;
    const QVariantList fields = flattenFieldsForTest(schema, config);
    for (const QVariant &field : fields) {
        if (field.toMap().value(QStringLiteral("live")).toBool()) {
            liveImage = true;
            break;
        }
    }
    result.insert(QStringLiteral("liveImage"), liveImage);
    result.insert(QStringLiteral("fieldCount"), fields.size());
    return result;
}

QVariantMap MajesticClient::metricsFromText(const QString &text)
{
    QVariantMap metrics;
    const QStringList lines = text.split(QLatin1Char('\n'));
    for (const QString &lineValue : lines) {
        const QString line = lineValue.trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) continue;
        const int separator = line.lastIndexOf(QRegularExpression(QStringLiteral("\\s+")));
        if (separator <= 0) continue;
        const QString key = line.left(separator).trimmed();
        const QString rawValue = line.mid(separator).trimmed();
        bool ok = false;
        const double number = rawValue.toDouble(&ok);
        metrics.insert(key, ok ? QVariant(number) : QVariant(rawValue));
    }
    return metrics;
}

bool MajesticClient::validatePatch(const QVariantMap &patch, const QVariantMap &schema,
                                   QString *error)
{
    QString nullPath;
    if (findJsonNull(QJsonObject::fromVariantMap(patch), {}, &nullPath)) {
        if (error) {
            *error = QStringLiteral("Configuration patch contains a null value at %1").arg(nullPath);
        }
        return false;
    }
    QStringList patchPaths;
    collectPatchPaths(patch, {}, &patchPaths);
    if (patchPaths.isEmpty()) {
        if (error) *error = QStringLiteral("Configuration patch has no leaf values");
        return false;
    }
    for (const QString &path : std::as_const(patchPaths)) {
        if (path.contains(QStringLiteral("..")) || path.startsWith(QLatin1Char('.'))
            || path.endsWith(QLatin1Char('.'))) {
            if (error) *error = QStringLiteral("Invalid configuration path: %1").arg(path);
            return false;
        }
    }
    if (!schema.isEmpty()) {
        QSet<QString> schemaPaths;
        collectSchemaPaths(schema, {}, &schemaPaths);
        QHash<QString, QVariantMap> descriptors;
        collectSchemaDescriptors(schema, {}, &descriptors);
        for (const QString &path : std::as_const(patchPaths)) {
            if (!schemaPaths.contains(path)) {
                if (error) *error = QStringLiteral("Field is not supported by this camera: %1").arg(path);
                return false;
            }
            const QVariantMap descriptor = descriptors.value(path);
            const QVariant value = patchValueAtPath(patch, path);
            const QString type = descriptor.value(QStringLiteral("type")).toString();
            bool conversionOk = true;
            double numericValue = 0.0;
            if (type == QStringLiteral("boolean")) {
                const QString text = value.toString().toLower();
                conversionOk = value.metaType().id() == QMetaType::Bool
                    || text == QStringLiteral("true") || text == QStringLiteral("false")
                    || text == QStringLiteral("1") || text == QStringLiteral("0");
            } else if (type == QStringLiteral("integer")) {
                value.toLongLong(&conversionOk);
                numericValue = value.toDouble();
            } else if (type == QStringLiteral("number")) {
                numericValue = value.toDouble(&conversionOk);
            } else if (type == QStringLiteral("array")) {
                conversionOk = value.metaType().id() == QMetaType::QVariantList
                    || value.metaType().id() == QMetaType::QStringList;
            }
            if (!conversionOk) {
                if (error) *error = QStringLiteral("Invalid %1 value for %2").arg(type, path);
                return false;
            }
            if ((type == QStringLiteral("integer") || type == QStringLiteral("number"))
                && descriptor.contains(QStringLiteral("minimum"))
                && numericValue < descriptor.value(QStringLiteral("minimum")).toDouble()) {
                if (error) *error = QStringLiteral("Value for %1 is below the minimum").arg(path);
                return false;
            }
            if ((type == QStringLiteral("integer") || type == QStringLiteral("number"))
                && descriptor.contains(QStringLiteral("maximum"))
                && numericValue > descriptor.value(QStringLiteral("maximum")).toDouble()) {
                if (error) *error = QStringLiteral("Value for %1 exceeds the maximum").arg(path);
                return false;
            }
            const QVariantList enumValues = descriptor.value(QStringLiteral("enum")).toList();
            if (!enumValues.isEmpty()) {
                const bool enumMatch = std::any_of(enumValues.cbegin(), enumValues.cend(),
                                                   [&value](const QVariant &candidate) {
                    return variantsEqual(candidate, value)
                        || candidate.toString() == value.toString();
                });
                if (!enumMatch) {
                    if (error) *error = QStringLiteral("Value for %1 is not in the supported enum").arg(path);
                    return false;
                }
            }
        }
    }
    return true;
}

void MajesticClient::handleSimpleReply(
    QNetworkReply *reply, const QString &requestId, const QString &operation,
    const std::function<void(const QByteArray &)> &onSuccess)
{
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, requestId, operation, onSuccess]() {
        const QByteArray body = reply->readAll();
        if (reply->error() == QNetworkReply::NoError) {
            if (onSuccess) onSuccess(body);
            const QString result = QString::fromUtf8(body).trimmed();
            emit operationSucceeded(requestId, operation, result);
            emit requestSucceeded(operation, result);
        } else {
            emitFailure(requestId, operation, reply, body);
        }
        reply->deleteLater();
    });
}

void MajesticClient::emitFailure(const QString &requestId, const QString &operation,
                                 QNetworkReply *reply, const QByteArray &body)
{
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString message = reply->errorString();
    const QString response = QString::fromUtf8(limitedBody(body)).trimmed();
    if (!response.isEmpty()) {
        message += QStringLiteral(": ") + response;
    }
    emit operationFailed(requestId, operation, message, status);
    emit requestFailed(operation, message);
}

void MajesticClient::emitFailureLater(const QString &requestId, const QString &operation,
                                      const QString &message, int httpStatus)
{
    QMetaObject::invokeMethod(this, [this, requestId, operation, message, httpStatus]() {
        emit operationFailed(requestId, operation, message, httpStatus);
        emit requestFailed(operation, message);
    }, Qt::QueuedConnection);
}
