#include "CameraHealthPolicy.h"

#include <QDateTime>

namespace {

QString statusLabel(const QString &status)
{
    if (status == QStringLiteral("ok")) return QStringLiteral("OK");
    if (status == QStringLiteral("warning")) return QStringLiteral("WARNING");
    if (status == QStringLiteral("error")) return QStringLiteral("ERROR");
    if (status == QStringLiteral("running")) return QStringLiteral("RUNNING");
    return QStringLiteral("UNKNOWN");
}

QString valueOrDash(const QVariantMap &map, const QString &key)
{
    const QString value = map.value(key).toString().trimmed();
    return value.isEmpty() ? QStringLiteral("-") : value;
}

bool probeFailed(const QVariantMap &probe, const QString &id)
{
    return probe.value(QStringLiteral("id")).toString() == id
        && probe.value(QStringLiteral("status")).toString() == QStringLiteral("error");
}

bool probeSucceeded(const QVariantMap &probe, const QString &id)
{
    return probe.value(QStringLiteral("id")).toString() == id
        && probe.value(QStringLiteral("status")).toString() == QStringLiteral("ok");
}

} // namespace

QString CameraHealthPolicy::normalizeProfile(const QString &profileId)
{
    const QString normalized = profileId.trimmed().toLower();
    if (normalized == QStringLiteral("quick")
        || normalized == QStringLiteral("deep")
        || normalized == QStringLiteral("openipc")
        || normalized == QStringLiteral("rtsp")) {
        return normalized;
    }
    return QStringLiteral("quick");
}

QVariantMap CameraHealthPolicy::probe(const QString &id, const QString &label,
                                      const QString &kind, bool required)
{
    return {
        {QStringLiteral("id"), id},
        {QStringLiteral("label"), label},
        {QStringLiteral("kind"), kind},
        {QStringLiteral("required"), required}
    };
}

QVariantList CameraHealthPolicy::profiles()
{
    return {
        QVariantMap{
            {QStringLiteral("id"), QStringLiteral("quick")},
            {QStringLiteral("label"), QStringLiteral("Quick")},
            {QStringLiteral("description"), QStringLiteral("RTSP main stream and snapshot readiness")}
        },
        QVariantMap{
            {QStringLiteral("id"), QStringLiteral("deep")},
            {QStringLiteral("label"), QStringLiteral("Deep")},
            {QStringLiteral("description"), QStringLiteral("All RTSP, Majestic, firmware, metrics and log probes")}
        },
        QVariantMap{
            {QStringLiteral("id"), QStringLiteral("openipc")},
            {QStringLiteral("label"), QStringLiteral("OpenIPC / Majestic")},
            {QStringLiteral("description"), QStringLiteral("Firmware WebUI and Majestic API diagnostics")}
        },
        QVariantMap{
            {QStringLiteral("id"), QStringLiteral("rtsp")},
            {QStringLiteral("label"), QStringLiteral("RTSP only")},
            {QStringLiteral("description"), QStringLiteral("Main and sub stream endpoint diagnostics")}
        }
    };
}

QVariantList CameraHealthPolicy::probePlan(const QString &profileId, bool hasSubStream)
{
    Q_UNUSED(hasSubStream);
    const QString profile = normalizeProfile(profileId);
    QVariantList result;

    const auto appendRtsp = [&result]() {
        result.append(probe(QStringLiteral("rtsp-main"), QStringLiteral("RTSP main"),
                            QStringLiteral("rtsp"), true));
        result.append(probe(QStringLiteral("rtsp-sub"), QStringLiteral("RTSP sub"),
                            QStringLiteral("rtsp"), false));
    };

    const auto appendOpenIpc = [&result]() {
        result.append(probe(QStringLiteral("majestic-config"), QStringLiteral("Majestic config"),
                            QStringLiteral("http"), true));
        result.append(probe(QStringLiteral("majestic-schema"), QStringLiteral("Majestic schema"),
                            QStringLiteral("http"), false));
        result.append(probe(QStringLiteral("firmware-status"), QStringLiteral("Firmware status"),
                            QStringLiteral("http"), false));
        result.append(probe(QStringLiteral("metrics"), QStringLiteral("Metrics"),
                            QStringLiteral("http"), false));
        result.append(probe(QStringLiteral("logs-readiness"), QStringLiteral("Live logs readiness"),
                            QStringLiteral("websocket"), false));
        result.append(probe(QStringLiteral("logs-sample"), QStringLiteral("Recent logs"),
                            QStringLiteral("http"), false));
        result.append(probe(QStringLiteral("snapshot"), QStringLiteral("Snapshot JPEG"),
                            QStringLiteral("http"), false));
    };

    if (profile == QStringLiteral("quick")) {
        result.append(probe(QStringLiteral("rtsp-main"), QStringLiteral("RTSP main"),
                            QStringLiteral("rtsp"), true));
        result.append(probe(QStringLiteral("snapshot"), QStringLiteral("Snapshot JPEG"),
                            QStringLiteral("http"), false));
    } else if (profile == QStringLiteral("rtsp")) {
        appendRtsp();
    } else if (profile == QStringLiteral("openipc")) {
        appendOpenIpc();
    } else {
        appendRtsp();
        appendOpenIpc();
    }
    return result;
}

QString CameraHealthPolicy::overallStatus(const QVariantList &probes)
{
    bool hasWarning = false;
    bool hasRunning = false;
    for (const QVariant &value : probes) {
        const QVariantMap probeResult = value.toMap();
        const QString status = probeResult.value(QStringLiteral("status")).toString();
        if (status == QStringLiteral("error")
            && probeResult.value(QStringLiteral("required"), true).toBool()) {
            return QStringLiteral("error");
        }
        if (status == QStringLiteral("error") || status == QStringLiteral("warning")) {
            hasWarning = true;
        } else if (status == QStringLiteral("pending") || status == QStringLiteral("running")) {
            hasRunning = true;
        }
    }
    if (hasRunning) return QStringLiteral("running");
    if (hasWarning) return QStringLiteral("warning");
    return probes.isEmpty() ? QStringLiteral("unknown") : QStringLiteral("ok");
}

QString CameraHealthPolicy::recommendation(const QVariantMap &cameraResult)
{
    const QVariantList probes = cameraResult.value(QStringLiteral("probes")).toList();
    bool authFailure = false;
    bool rtspFailure = false;
    bool majesticFailure = false;
    bool firmwareReachable = false;
    bool anyFailure = false;

    for (const QVariant &value : probes) {
        const QVariantMap item = value.toMap();
        const QString message = item.value(QStringLiteral("message")).toString().toLower();
        const QString status = item.value(QStringLiteral("status")).toString();
        if (status == QStringLiteral("error") || status == QStringLiteral("warning")) {
            anyFailure = true;
        }
        if (message.contains(QStringLiteral("401"))
            || message.contains(QStringLiteral("403"))
            || message.contains(QStringLiteral("authentication"))
            || message.contains(QStringLiteral("credentials"))) {
            authFailure = true;
        }
        rtspFailure = rtspFailure || probeFailed(item, QStringLiteral("rtsp-main"));
        majesticFailure = majesticFailure || probeFailed(item, QStringLiteral("majestic-config"));
        firmwareReachable = firmwareReachable
            || probeSucceeded(item, QStringLiteral("firmware-status"))
            || probeSucceeded(item, QStringLiteral("snapshot"));
    }

    if (authFailure) {
        return QStringLiteral("Check the camera username and password, then run the profile again.");
    }
    if (rtspFailure && firmwareReachable) {
        return QStringLiteral("The camera WebUI responds, but the RTSP stream is unavailable. Check Majestic video settings and the configured stream URL.");
    }
    if (majesticFailure && firmwareReachable) {
        return QStringLiteral("The firmware WebUI is available, but the Majestic API is unavailable. Check whether Majestic is running and its HTTP port.");
    }
    if (rtspFailure) {
        return QStringLiteral("The RTSP endpoint is unavailable. Check camera power, network route, RTSP port and stream path.");
    }
    if (anyFailure) {
        return QStringLiteral("Review the failed optional probes below; the primary camera endpoint remains available.");
    }
    if (!cameraResult.value(QStringLiteral("inGrid")).toBool()) {
        return QStringLiteral("The camera is healthy but is not currently used in the active layout.");
    }
    return QStringLiteral("No issues were detected.");
}

QString CameraHealthPolicy::reportText(const QVariantMap &run)
{
    if (run.isEmpty()) {
        return QStringLiteral("OpenIPC Camera Health report\nNo completed diagnostic run is available.");
    }

    QString report;
    report += QStringLiteral("OpenIPC Camera Health report\n");
    report += QStringLiteral("Run: %1\n").arg(valueOrDash(run, QStringLiteral("id")));
    report += QStringLiteral("Profile: %1\n").arg(valueOrDash(run, QStringLiteral("profileLabel")));
    report += QStringLiteral("Started: %1\n").arg(valueOrDash(run, QStringLiteral("startedAt")));
    report += QStringLiteral("Completed: %1\n").arg(valueOrDash(run, QStringLiteral("completedAt")));
    report += QStringLiteral("Summary: %1\n").arg(valueOrDash(run, QStringLiteral("summary")));

    const QVariantList cameras = run.value(QStringLiteral("cameras")).toList();
    for (qsizetype index = 0; index < cameras.size(); ++index) {
        const QVariantMap camera = cameras.at(index).toMap();
        report += QStringLiteral("\n%1. %2\n").arg(index + 1).arg(valueOrDash(camera, QStringLiteral("name")));
        report += QStringLiteral("   IP: %1\n").arg(valueOrDash(camera, QStringLiteral("ip")));
        report += QStringLiteral("   Status: %1\n")
                      .arg(statusLabel(camera.value(QStringLiteral("status")).toString()));
        report += QStringLiteral("   In layout: %1\n")
                      .arg(camera.value(QStringLiteral("inGrid")).toBool()
                               ? QStringLiteral("yes") : QStringLiteral("no"));
        report += QStringLiteral("   RTSP: %1\n").arg(valueOrDash(camera, QStringLiteral("mainStreamUrl")));
        report += QStringLiteral("   HTTP port: %1\n")
                      .arg(camera.value(QStringLiteral("httpPort"), 80).toInt());
        report += QStringLiteral("   Firmware: %1\n").arg(valueOrDash(camera, QStringLiteral("firmwareVersion")));
        report += QStringLiteral("   Majestic: %1\n").arg(valueOrDash(camera, QStringLiteral("majesticVersion")));
        if (camera.value(QStringLiteral("temperatureC")).isValid()) {
            report += QStringLiteral("   SoC temperature: %1 C\n")
                          .arg(camera.value(QStringLiteral("temperatureC")).toDouble(),
                               0, 'f', 1);
        }
        report += QStringLiteral("   Recommendation: %1\n")
                      .arg(valueOrDash(camera, QStringLiteral("recommendation")));

        const QVariantList probes = camera.value(QStringLiteral("probes")).toList();
        report += QStringLiteral("   Probes:\n");
        for (const QVariant &probeValue : probes) {
            const QVariantMap probeResult = probeValue.toMap();
            report += QStringLiteral("     - [%1] %2 (%3 ms): %4\n")
                          .arg(statusLabel(probeResult.value(QStringLiteral("status")).toString()),
                               valueOrDash(probeResult, QStringLiteral("label")))
                          .arg(probeResult.value(QStringLiteral("elapsedMs")).toInt())
                          .arg(valueOrDash(probeResult, QStringLiteral("message")));
        }

        const QString logs = camera.value(QStringLiteral("lastLogs")).toString().trimmed();
        if (!logs.isEmpty()) {
            report += QStringLiteral("   Recent logs:\n");
            const QStringList lines = logs.split(QLatin1Char('\n'));
            for (const QString &line : lines.mid(qMax(0, lines.size() - 20))) {
                report += QStringLiteral("     %1\n").arg(line.left(300));
            }
        }
    }
    return report;
}
