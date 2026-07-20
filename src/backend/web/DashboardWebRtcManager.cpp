#include "DashboardWebRtcManager.h"

#include "CameraModel.h"
#include "SystemController.h"

#include <QDebug>
#include <QMetaObject>
#include <QPointer>
#include <QStringList>

#include <memory>

#include <gst/sdp/sdp.h>

#ifndef GST_USE_UNSTABLE_API
#define GST_USE_UNSTABLE_API
#endif
#include <gst/webrtc/webrtc.h>

namespace {

QString normalizedQuality(const QString &quality)
{
    return quality.compare(QStringLiteral("hd"), Qt::CaseInsensitive) == 0
        ? QStringLiteral("hd") : QStringLiteral("sd");
}

QString previewSource(const Camera &camera, const QString &quality)
{
    if (quality == QStringLiteral("hd")) {
        if (!camera.hdStreamUrl.trimmed().isEmpty()) return camera.hdStreamUrl;
        if (!camera.streamUrl.trimmed().isEmpty()) return camera.streamUrl;
        return camera.sdStreamUrl;
    }
    if (!camera.sdStreamUrl.trimmed().isEmpty()) return camera.sdStreamUrl;
    if (!camera.streamUrl.trimmed().isEmpty()) return camera.streamUrl;
    return camera.hdStreamUrl;
}

bool hasFactory(const char *name)
{
    GstElementFactory *factory = gst_element_factory_find(name);
    if (!factory) return false;
    gst_object_unref(factory);
    return true;
}

QString gstErrorText(const GError *error)
{
    return error && error->message ? QString::fromUtf8(error->message)
                                   : QStringLiteral("Unknown GStreamer error");
}

} // namespace

struct DashboardWebRtcManager::Peer {
    DashboardWebRtcManager *owner = nullptr;
    QString id;
    QString cameraIp;
    QString quality;
    QString codec;
    GstElement *pipeline = nullptr;
    GstElement *source = nullptr;
    GstElement *webrtc = nullptr;
    GstElement *videoBranch = nullptr;
    GstElement *audioBranch = nullptr;
    GstPad *videoWebRtcSinkPad = nullptr;
    GstPad *audioWebRtcSinkPad = nullptr;
    bool videoLinked = false;
    bool audioLinked = false;
};

struct DashboardWebRtcManager::PromiseContext {
    QPointer<DashboardWebRtcManager> owner;
    QString peerId;
    GstElement *webrtc = nullptr;
};

DashboardWebRtcManager::DashboardWebRtcManager(SystemController *systemController,
                                               QObject *parent)
    : QObject(parent)
    , m_systemController(systemController)
{
    if (!gst_is_initialized()) gst_init(nullptr, nullptr);
    const QStringList required{
        QStringLiteral("rtspsrc"), QStringLiteral("webrtcbin"),
        QStringLiteral("nicesrc"), QStringLiteral("nicesink"),
        QStringLiteral("dtlssrtpenc"), QStringLiteral("srtpenc"),
        QStringLiteral("rtph264depay"), QStringLiteral("h264parse"),
        QStringLiteral("rtph264pay")
    };
    QStringList missing;
    for (const QString &name : required) {
        if (!hasFactory(name.toUtf8().constData())) missing.append(name);
    }
    if (!missing.isEmpty()) {
        m_availabilityError = tr("Missing GStreamer WebRTC elements: %1")
            .arg(missing.join(QStringLiteral(", ")));
    }
    m_audioAvailable = hasFactory("audioconvert") && hasFactory("audioresample")
        && hasFactory("opusenc") && hasFactory("rtpopuspay");
}

DashboardWebRtcManager::~DashboardWebRtcManager()
{
    stopAll();
}

bool DashboardWebRtcManager::available() const
{
    return m_availabilityError.isEmpty();
}

QString DashboardWebRtcManager::availabilityError() const
{
    return m_availabilityError;
}

bool DashboardWebRtcManager::startPeer(const QString &peerId, int cameraIndex,
                                       const QString &requestedQuality, QString *error)
{
    if (!available()) {
        if (error) *error = m_availabilityError;
        return false;
    }
    if (!m_systemController || cameraIndex < 0
        || cameraIndex >= m_systemController->cameraModel()->rowCount()) {
        if (error) *error = tr("Camera not found");
        return false;
    }

    const Camera camera = m_systemController->cameraModel()->getCamera(cameraIndex);
    const QString quality = normalizedQuality(requestedQuality);
    const QString sourceUrl = previewSource(camera, quality).trimmed();
    if (sourceUrl.isEmpty()) {
        if (error) *error = tr("Camera has no configured stream");
        return false;
    }

    removePeer(peerId, false);
    auto *peer = new Peer;
    peer->owner = this;
    peer->id = peerId;
    peer->cameraIp = camera.ip;
    peer->quality = quality;
    peer->pipeline = gst_pipeline_new(nullptr);
    peer->source = gst_element_factory_make("rtspsrc", nullptr);
    peer->webrtc = gst_element_factory_make("webrtcbin", nullptr);
    if (!peer->pipeline || !peer->source || !peer->webrtc) {
        if (error) *error = tr("Could not create the GStreamer WebRTC pipeline");
        if (peer->pipeline) gst_object_unref(peer->pipeline);
        if (peer->source) gst_object_unref(peer->source);
        if (peer->webrtc) gst_object_unref(peer->webrtc);
        delete peer;
        return false;
    }

    const QString authenticatedUrl = m_systemController->authenticatedStreamUrl(sourceUrl, camera.ip);
    g_object_set(peer->source,
                 "location", authenticatedUrl.toUtf8().constData(),
                 "protocols", 4,
                 "latency", 80u,
                 "drop-on-latency", TRUE,
                 nullptr);
    g_object_set(peer->webrtc,
                 "bundle-policy", GST_WEBRTC_BUNDLE_POLICY_MAX_BUNDLE,
                 "latency", 0u,
                 nullptr);

    gst_bin_add_many(GST_BIN(peer->pipeline), peer->source, peer->webrtc, nullptr);
    g_signal_connect(peer->source, "pad-added", G_CALLBACK(onSourcePadAdded), peer);
    g_signal_connect(peer->webrtc, "on-negotiation-needed",
                     G_CALLBACK(onNegotiationNeeded), peer);
    g_signal_connect(peer->webrtc, "on-ice-candidate",
                     G_CALLBACK(onIceCandidate), peer);

    GstBus *bus = gst_element_get_bus(peer->pipeline);
    gst_bus_add_signal_watch(bus);
    g_signal_connect(bus, "message", G_CALLBACK(onBusMessage), peer);
    gst_object_unref(bus);

    m_peers.insert(peerId, peer);
    if (gst_element_set_state(peer->pipeline, GST_STATE_PLAYING)
        == GST_STATE_CHANGE_FAILURE) {
        if (error) *error = tr("Could not start the GStreamer WebRTC pipeline");
        removePeer(peerId, false);
        return false;
    }

    qInfo().noquote() << "WebRTC peer started for" << camera.ip << quality
                      << "peer" << peerId;
    postMessage(peerId, {
        {QStringLiteral("type"), QStringLiteral("webrtc-status")},
        {QStringLiteral("state"), QStringLiteral("starting")}
    });
    return true;
}

bool DashboardWebRtcManager::setAnswer(const QString &peerId, const QString &sdp,
                                       QString *error)
{
    Peer *peer = m_peers.value(peerId, nullptr);
    if (!peer || !peer->webrtc) {
        if (error) *error = tr("WebRTC peer not found");
        return false;
    }

    GstSDPMessage *message = nullptr;
    const QByteArray sdpBytes = sdp.toUtf8();
    if (gst_sdp_message_new(&message) != GST_SDP_OK
        || gst_sdp_message_parse_buffer(
               reinterpret_cast<const guint8 *>(sdpBytes.constData()),
               static_cast<guint>(sdpBytes.size()), message) != GST_SDP_OK) {
        if (message) gst_sdp_message_free(message);
        if (error) *error = tr("Invalid WebRTC answer");
        return false;
    }

    GstWebRTCSessionDescription *answer = gst_webrtc_session_description_new(
        GST_WEBRTC_SDP_TYPE_ANSWER, message);
    GstPromise *promise = gst_promise_new();
    g_signal_emit_by_name(peer->webrtc, "set-remote-description", answer, promise);
    gst_promise_interrupt(promise);
    gst_promise_unref(promise);
    gst_webrtc_session_description_free(answer);
    postMessage(peerId, {
        {QStringLiteral("type"), QStringLiteral("webrtc-status")},
        {QStringLiteral("state"), QStringLiteral("connecting")}
    });
    return true;
}

bool DashboardWebRtcManager::addIceCandidate(const QString &peerId, int mlineIndex,
                                             const QString &candidate, QString *error)
{
    Peer *peer = m_peers.value(peerId, nullptr);
    if (!peer || !peer->webrtc) {
        if (error) *error = tr("WebRTC peer not found");
        return false;
    }
    if (mlineIndex < 0 || candidate.size() > 8192) {
        if (error) *error = tr("Invalid ICE candidate");
        return false;
    }
    g_signal_emit_by_name(peer->webrtc, "add-ice-candidate",
                          static_cast<guint>(mlineIndex), candidate.toUtf8().constData());
    return true;
}

void DashboardWebRtcManager::stopPeer(const QString &peerId)
{
    removePeer(peerId, true);
}

void DashboardWebRtcManager::stopAll()
{
    const QStringList peerIds = m_peers.keys();
    for (const QString &peerId : peerIds) removePeer(peerId, false);
}

void DashboardWebRtcManager::removePeer(const QString &peerId, bool notify)
{
    Peer *peer = m_peers.take(peerId);
    if (!peer) return;
    if (peer->pipeline) {
        gst_element_set_state(peer->pipeline, GST_STATE_NULL);
        GstBus *bus = gst_element_get_bus(peer->pipeline);
        gst_bus_remove_signal_watch(bus);
        gst_object_unref(bus);
    }
    if (peer->webrtc && peer->videoWebRtcSinkPad) {
        gst_element_release_request_pad(peer->webrtc, peer->videoWebRtcSinkPad);
        gst_object_unref(peer->videoWebRtcSinkPad);
        peer->videoWebRtcSinkPad = nullptr;
    }
    if (peer->webrtc && peer->audioWebRtcSinkPad) {
        gst_element_release_request_pad(peer->webrtc, peer->audioWebRtcSinkPad);
        gst_object_unref(peer->audioWebRtcSinkPad);
        peer->audioWebRtcSinkPad = nullptr;
    }
    if (peer->pipeline) gst_object_unref(peer->pipeline);
    qInfo().noquote() << "WebRTC peer stopped for" << peer->cameraIp
                      << "peer" << peerId;
    delete peer;
    if (notify) emit peerStopped(peerId);
}

void DashboardWebRtcManager::onSourcePadAdded(GstElement *, GstPad *pad,
                                              gpointer userData)
{
    auto *peer = static_cast<Peer *>(userData);
    if (!peer || !peer->pipeline || !peer->webrtc) return;

    GstCaps *caps = gst_pad_get_current_caps(pad);
    if (!caps) caps = gst_pad_query_caps(pad, nullptr);
    if (!caps || gst_caps_is_empty(caps)) {
        if (caps) gst_caps_unref(caps);
        return;
    }
    const GstStructure *structure = gst_caps_get_structure(caps, 0);
    const gchar *media = gst_structure_get_string(structure, "media");
    const gchar *encoding = gst_structure_get_string(structure, "encoding-name");
    if (!media || !encoding) {
        gst_caps_unref(caps);
        return;
    }

    const QString codec = QString::fromUtf8(encoding).toUpper();
    QString branchDescription;
    QString mode;
    const bool video = g_ascii_strcasecmp(media, "video") == 0;
    const bool audio = g_ascii_strcasecmp(media, "audio") == 0;
    if ((!video && !audio) || (video && peer->videoLinked) || (audio && peer->audioLinked)) {
        gst_caps_unref(caps);
        return;
    }

    if (video && codec == QStringLiteral("H264")) {
        branchDescription = QStringLiteral(
            "queue max-size-buffers=2 max-size-bytes=0 max-size-time=0 leaky=downstream "
            "! rtph264depay ! h264parse ! rtph264pay config-interval=-1 "
            "aggregate-mode=zero-latency pt=96 "
            "! application/x-rtp,media=video,encoding-name=H264,payload=96");
        mode = QStringLiteral("passthrough");
    } else if (video && (codec == QStringLiteral("H265") || codec == QStringLiteral("HEVC"))) {
        if (!hasFactory("rtph265depay") || !hasFactory("avdec_h265")
            || !hasFactory("openh264enc")) {
            peer->owner->postError(peer->id,
                                   peer->owner->tr("H.265 WebRTC transcoder is unavailable"));
            peer->owner->scheduleStop(peer->id);
            gst_caps_unref(caps);
            return;
        }
        const int bitrate = peer->quality == QStringLiteral("hd") ? 4000000 : 1800000;
        const int maxRate = peer->quality == QStringLiteral("hd") ? 20 : 15;
        branchDescription = QStringLiteral(
            "queue max-size-buffers=2 max-size-bytes=0 max-size-time=0 leaky=downstream "
            "! rtph265depay ! h265parse ! avdec_h265 "
            "! videoconvert ! videorate drop-only=true max-rate=%1 "
            "! video/x-raw,format=I420 "
            "! openh264enc bitrate=%2 complexity=low gop-size=%3 multi-thread=4 "
            "! h264parse ! rtph264pay config-interval=-1 aggregate-mode=zero-latency pt=96 "
            "! application/x-rtp,media=video,encoding-name=H264,payload=96")
            .arg(maxRate).arg(bitrate).arg(maxRate * 2);
        mode = QStringLiteral("h265-to-h264");
    } else if (audio && codec == QStringLiteral("OPUS")
               && hasFactory("rtpopusdepay") && hasFactory("opusparse")
               && hasFactory("rtpopuspay")) {
        branchDescription = QStringLiteral(
            "queue max-size-buffers=8 max-size-bytes=0 max-size-time=0 leaky=downstream "
            "! rtpopusdepay ! opusparse ! rtpopuspay pt=111 "
            "! application/x-rtp,media=audio,encoding-name=OPUS,payload=111");
        mode = QStringLiteral("audio-opus-passthrough");
    } else if (audio && codec == QStringLiteral("PCMA")
               && hasFactory("rtppcmadepay") && hasFactory("alawdec")
               && peer->owner->audioAvailable()) {
        branchDescription = QStringLiteral(
            "queue max-size-buffers=8 max-size-bytes=0 max-size-time=0 leaky=downstream "
            "! rtppcmadepay ! alawdec ! audioconvert ! audioresample "
            "! opusenc bitrate=64000 audio-type=voice ! rtpopuspay pt=111 "
            "! application/x-rtp,media=audio,encoding-name=OPUS,payload=111");
        mode = QStringLiteral("audio-pcma-to-opus");
    } else if (audio && codec == QStringLiteral("PCMU")
               && hasFactory("rtppcmudepay") && hasFactory("mulawdec")
               && peer->owner->audioAvailable()) {
        branchDescription = QStringLiteral(
            "queue max-size-buffers=8 max-size-bytes=0 max-size-time=0 leaky=downstream "
            "! rtppcmudepay ! mulawdec ! audioconvert ! audioresample "
            "! opusenc bitrate=64000 audio-type=voice ! rtpopuspay pt=111 "
            "! application/x-rtp,media=audio,encoding-name=OPUS,payload=111");
        mode = QStringLiteral("audio-pcmu-to-opus");
    } else if (audio && (codec == QStringLiteral("MPEG4-GENERIC")
                         || codec == QStringLiteral("AAC"))
               && hasFactory("rtpmp4gdepay") && hasFactory("avdec_aac")
               && peer->owner->audioAvailable()) {
        branchDescription = QStringLiteral(
            "queue max-size-buffers=8 max-size-bytes=0 max-size-time=0 leaky=downstream "
            "! rtpmp4gdepay ! avdec_aac ! audioconvert ! audioresample "
            "! opusenc bitrate=64000 audio-type=voice ! rtpopuspay pt=111 "
            "! application/x-rtp,media=audio,encoding-name=OPUS,payload=111");
        mode = QStringLiteral("audio-aac-to-opus");
    } else if (audio) {
        qInfo().noquote() << "Skipping unsupported WebRTC audio codec" << codec
                          << "for peer" << peer->id;
        gst_caps_unref(caps);
        return;
    } else {
        peer->owner->postError(peer->id,
                               peer->owner->tr("Unsupported WebRTC source codec: %1").arg(codec));
        peer->owner->scheduleStop(peer->id);
        gst_caps_unref(caps);
        return;
    }
    gst_caps_unref(caps);

    GError *parseError = nullptr;
    GstElement *branch = gst_parse_bin_from_description(
        branchDescription.toUtf8().constData(), TRUE, &parseError);
    if (!branch) {
        if (video) peer->owner->postError(peer->id, gstErrorText(parseError));
        else qWarning().noquote() << "Could not create the optional WebRTC audio branch:"
                                  << gstErrorText(parseError);
        if (parseError) g_error_free(parseError);
        if (video) peer->owner->scheduleStop(peer->id);
        return;
    }
    if (parseError) g_error_free(parseError);

    gst_bin_add(GST_BIN(peer->pipeline), branch);
    GstPad *branchSink = gst_element_get_static_pad(branch, "sink");
    GstPad *branchSource = gst_element_get_static_pad(branch, "src");
    GstPad *webrtcSink = gst_element_request_pad_simple(peer->webrtc, "sink_%u");
    const bool linked = branchSink && branchSource && webrtcSink
        && gst_pad_link(pad, branchSink) == GST_PAD_LINK_OK
        && gst_pad_link(branchSource, webrtcSink) == GST_PAD_LINK_OK;
    if (branchSink) gst_object_unref(branchSink);
    if (branchSource) gst_object_unref(branchSource);
    if (!linked) {
        if (webrtcSink) {
            gst_element_release_request_pad(peer->webrtc, webrtcSink);
            gst_object_unref(webrtcSink);
        }
        gst_bin_remove(GST_BIN(peer->pipeline), branch);
        if (video) {
            peer->owner->postError(peer->id,
                                   peer->owner->tr("Could not link the WebRTC video branch"));
            peer->owner->scheduleStop(peer->id);
        } else {
            qWarning().noquote() << "Could not link the optional WebRTC audio branch for peer"
                                 << peer->id;
        }
        return;
    }

    if (video) {
        peer->codec = codec;
        peer->videoBranch = branch;
        peer->videoWebRtcSinkPad = webrtcSink;
        peer->videoLinked = true;
    } else {
        peer->audioBranch = branch;
        peer->audioWebRtcSinkPad = webrtcSink;
        peer->audioLinked = true;
    }
    gst_element_sync_state_with_parent(branch);
    peer->owner->postMessage(peer->id, {
        {QStringLiteral("type"), QStringLiteral("webrtc-status")},
        {QStringLiteral("state"), video ? QStringLiteral("negotiating")
                                         : QStringLiteral("audio-ready")},
        {QStringLiteral("codec"), codec},
        {QStringLiteral("mode"), mode}
    });
}

void DashboardWebRtcManager::onNegotiationNeeded(GstElement *webrtc,
                                                 gpointer userData)
{
    auto *peer = static_cast<Peer *>(userData);
    if (!peer || !peer->owner) return;
    auto *context = new PromiseContext{
        QPointer<DashboardWebRtcManager>(peer->owner), peer->id,
        GST_ELEMENT(gst_object_ref(webrtc))
    };
    GstPromise *promise = gst_promise_new_with_change_func(
        onOfferCreated, context, nullptr);
    g_signal_emit_by_name(webrtc, "create-offer", nullptr, promise);
}

void DashboardWebRtcManager::onOfferCreated(GstPromise *promise, gpointer userData)
{
    std::unique_ptr<PromiseContext> context(static_cast<PromiseContext *>(userData));
    const GstStructure *reply = gst_promise_get_reply(promise);
    GstWebRTCSessionDescription *offer = nullptr;
    if (reply) {
        gst_structure_get(reply, "offer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION,
                          &offer, nullptr);
    }
    gst_promise_unref(promise);
    if (!offer || !context->webrtc) {
        if (context->owner) context->owner->postError(
            context->peerId, context->owner->tr("Could not create a WebRTC offer"));
        if (offer) gst_webrtc_session_description_free(offer);
        if (context->webrtc) gst_object_unref(context->webrtc);
        return;
    }

    GstPromise *localPromise = gst_promise_new();
    g_signal_emit_by_name(context->webrtc, "set-local-description", offer, localPromise);
    gst_promise_interrupt(localPromise);
    gst_promise_unref(localPromise);
    gchar *sdpText = gst_sdp_message_as_text(offer->sdp);
    if (context->owner) {
        context->owner->postMessage(context->peerId, {
            {QStringLiteral("type"), QStringLiteral("webrtc-offer")},
            {QStringLiteral("sdp"), QString::fromUtf8(sdpText ? sdpText : "")}
        });
    }
    g_free(sdpText);
    gst_webrtc_session_description_free(offer);
    gst_object_unref(context->webrtc);
}

void DashboardWebRtcManager::onIceCandidate(GstElement *, guint mlineIndex,
                                            gchar *candidate, gpointer userData)
{
    auto *peer = static_cast<Peer *>(userData);
    if (!peer || !peer->owner || !candidate) return;
    peer->owner->postMessage(peer->id, {
        {QStringLiteral("type"), QStringLiteral("webrtc-ice")},
        {QStringLiteral("mlineIndex"), static_cast<int>(mlineIndex)},
        {QStringLiteral("candidate"), QString::fromUtf8(candidate)}
    });
}

void DashboardWebRtcManager::onBusMessage(GstBus *, GstMessage *message,
                                          gpointer userData)
{
    auto *peer = static_cast<Peer *>(userData);
    if (!peer || !peer->owner) return;
    if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR) {
        GError *error = nullptr;
        gchar *debug = nullptr;
        gst_message_parse_error(message, &error, &debug);
        const QString text = gstErrorText(error);
        g_clear_error(&error);
        g_free(debug);
        peer->owner->postError(peer->id, text);
        peer->owner->scheduleStop(peer->id);
    } else if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_EOS) {
        peer->owner->postError(peer->id, peer->owner->tr("WebRTC source stream ended"));
        peer->owner->scheduleStop(peer->id);
    }
}

void DashboardWebRtcManager::postMessage(const QString &peerId,
                                         const QVariantMap &message)
{
    QPointer<DashboardWebRtcManager> guard(this);
    QMetaObject::invokeMethod(this, [guard, peerId, message]() {
        if (!guard) return;
        QVariantMap payload = message;
        payload.insert(QStringLiteral("peerId"), peerId);
        emit guard.data()->messageReady(peerId, payload);
    }, Qt::QueuedConnection);
}

void DashboardWebRtcManager::postError(const QString &peerId,
                                       const QString &message)
{
    postMessage(peerId, {
        {QStringLiteral("type"), QStringLiteral("webrtc-error")},
        {QStringLiteral("error"), message}
    });
}

void DashboardWebRtcManager::scheduleStop(const QString &peerId)
{
    QPointer<DashboardWebRtcManager> guard(this);
    QMetaObject::invokeMethod(this, [guard, peerId]() {
        if (guard) guard->stopPeer(peerId);
    }, Qt::QueuedConnection);
}
