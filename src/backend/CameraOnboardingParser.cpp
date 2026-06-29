#include "CameraOnboardingParser.h"

#include <QUrl>
#include <QUrlQuery>

QVariantMap CameraOnboardingParser::parse(const QString &payload)
{
    const QUrl input(payload.trimmed());
    if (!input.isValid() || input.host().isEmpty()) {
        return {{QStringLiteral("valid"), false},
                {QStringLiteral("error"), QStringLiteral("Unsupported camera QR payload")}};
    }

    const QString scheme = input.scheme().toLower();
    if (scheme != QStringLiteral("openipc") && scheme != QStringLiteral("rtsp")
        && scheme != QStringLiteral("rtsps")) {
        return {{QStringLiteral("valid"), false},
                {QStringLiteral("error"), QStringLiteral("Unsupported camera QR scheme")}};
    }

    const QUrlQuery query(input);
    const int port = input.port(554);
    const int parsedOnvifPort = query.queryItemValue(QStringLiteral("onvifPort")).toInt();
    const QString path = input.path().isEmpty() ? QStringLiteral("/stream=0") : input.path();
    QUrl hdUrl;
    hdUrl.setScheme(scheme == QStringLiteral("rtsps") ? QStringLiteral("rtsps") : QStringLiteral("rtsp"));
    hdUrl.setHost(input.host());
    hdUrl.setPort(port);
    hdUrl.setPath(path);

    QUrl sdUrl = hdUrl;
    QString sdPath = query.queryItemValue(QStringLiteral("sdPath"));
    if (sdPath.isEmpty() && path.contains(QStringLiteral("stream=0"))) {
        sdPath = QString(path).replace(QStringLiteral("stream=0"), QStringLiteral("stream=1"));
    }
    if (!sdPath.isEmpty()) sdUrl.setPath(sdPath.startsWith('/') ? sdPath : '/' + sdPath);

    return {
        {QStringLiteral("valid"), true},
        {QStringLiteral("name"), query.queryItemValue(QStringLiteral("name"))},
        {QStringLiteral("ip"), input.host()},
        {QStringLiteral("port"), port},
        {QStringLiteral("onvifPort"), parsedOnvifPort > 0 ? parsedOnvifPort : 80},
        {QStringLiteral("login"), input.userName(QUrl::FullyDecoded)},
        {QStringLiteral("password"), input.password(QUrl::FullyDecoded)},
        {QStringLiteral("hdStreamUrl"), hdUrl.toString(QUrl::FullyEncoded)},
        {QStringLiteral("sdStreamUrl"), sdPath.isEmpty() ? QString() : sdUrl.toString(QUrl::FullyEncoded)}
    };
}
