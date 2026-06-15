#include "GstPlayer.h"
#include "../analytics/AnalyticsEngine.h"

#include <QQuickWindow>
#include <QSGSimpleTextureNode>
#include <QDebug>
#include <QDateTime>
#include <gst/app/gstappsink.h>

// Ensure gst_init is called once
static void ensureGstInit() {
    static bool initialized = false;
    if (!initialized) {
        gst_init(nullptr, nullptr);
        initialized = true;
    }
}

GstPlayer::GstPlayer(QQuickItem *parent) : QQuickItem(parent)
{
    ensureGstInit();
    setFlag(ItemHasContents, true);

    m_statsTimer = new QTimer(this);
    m_statsTimer->setInterval(1000);
    connect(m_statsTimer, &QTimer::timeout, this, &GstPlayer::updateStats);

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    m_reconnectTimer->setInterval(2000);
    connect(m_reconnectTimer, &QTimer::timeout, this, [this]() {
        if (!m_running || m_url.isEmpty()) return;
        qInfo() << "Reconnecting to stream:" << m_url;
        restartPipeline();
    });
}

GstPlayer::~GstPlayer()
{
    stopPipeline();
}

void GstPlayer::setUrl(const QString &url)
{
    if (m_url != url) {
        m_url = url;
        emit urlChanged();
        if (m_running) {
            restartPipeline();
        }
    }
}

void GstPlayer::setRunning(bool running)
{
    if (m_running != running) {
        m_running = running;
        emit runningChanged();
        if (m_running) {
            startPipeline();
        } else {
            stopPipeline();
        }
        emit mediaStatusChanged(m_running ? 1 : 0);
    }
}

void GstPlayer::setRtspTransport(const QString &transport)
{
    if (m_rtspTransport != transport) {
        m_rtspTransport = transport;
        emit rtspTransportChanged();
        if (m_running) restartPipeline();
    }
}

void GstPlayer::setHwDecoding(const QString& value)
{
    QString normalized = value.trimmed().toLower();
    if (normalized.isEmpty()) normalized = "auto";
    if (m_hwDecoding != normalized) {
        m_hwDecoding = normalized;
        emit hwDecodingChanged();
        if (m_running) restartPipeline();
    }
}

void GstPlayer::setHwDecoders(bool value)
{
    const QString desired = value ? "auto" : "none";
    if (m_hwDecoding != desired) {
        m_hwDecoding = desired;
        emit hwDecodersChanged();
        emit hwDecodingChanged();
        if (m_running) restartPipeline();
    }
}

void GstPlayer::setBufferMode(int mode)
{
    if (m_bufferMode != mode) {
        m_bufferMode = mode;
        emit bufferModeChanged();
        if (m_running) restartPipeline();
    }
}

void GstPlayer::setVolume(double volume)
{
    if (m_volume != volume) {
        m_volume = volume;
        if (m_volumeElement) {
            g_object_set(m_volumeElement, "volume", m_volume, NULL);
        }
        emit volumeChanged();
    }
}

void GstPlayer::setMuted(bool muted)
{
    if (m_muted != muted) {
        m_muted = muted;
        if (m_pipeline) {
             // For playbin, it implements GstStreamVolume
             // But GstStreamVolume interface usage in C++ without wrappers is verbose.
             // Simpler: set "mute" property on playbin
             g_object_set(G_OBJECT(m_pipeline), "mute", m_muted, NULL);
             updatePlaybinAudioFlags();
        }
        emit mutedChanged();
    }
}

// --- Video Adjustments ---

void GstPlayer::setBrightness(float value) {
    // UI likely sends 0.0 to 2.0 (default 1.0). videobalance expects -1.0 to 1.0 (default 0).
    // Let's assume the UI sends the value relative to 1.0 being neutral.
    // video_balance brightness: -1.0 (black) ... 0.0 (normal) ... 1.0 (white).
    // Map: (val - 1.0)
    
    // Check what range GridCell sends. 
    // Usually standard sliders are 0..1 or 0..2. Assuming 0..2 with 1.0 default:
    float gstVal = value - 1.0f; 
    // Clamp
    if (gstVal < -1.0f) gstVal = -1.0f;
    if (gstVal > 1.0f) gstVal = 1.0f;
    
    if (m_brightness != gstVal) {
        m_brightness = gstVal;
        if (m_videoBalance) {
            g_object_set(m_videoBalance, "brightness", (double)m_brightness, NULL);
        }
    }
}

void GstPlayer::setContrast(float value) {
    // videobalance: 0.0 to 2.0. matches typical UI.
    if (m_contrast != value) {
        m_contrast = value;
        if (m_videoBalance) {
            g_object_set(m_videoBalance, "contrast", (double)m_contrast, NULL);
        }
    }
}

void GstPlayer::setHue(int value) {
    // videobalance: -1.0 to 1.0. UI sends int degrees? (-180 to 180?)
    // If int is degrees:
    float gstVal = (float)value / 180.0f; // map -180..180 to -1..1?
    // GridCell doesn't specify range but usually it is angle.
    // videobalance docs say: hue -1 to 1.
    if (gstVal < -1.0f) gstVal = -1.0f;
    if (gstVal > 1.0f) gstVal = 1.0f;

    if (m_hue != value) {
        m_hue = value;
        if (m_videoBalance) {
            g_object_set(m_videoBalance, "hue", (double)gstVal, NULL);
        }
    }
}

void GstPlayer::setSaturation(float value) {
    // videobalance: 0.0 to 2.0
    // Clamp to ensure we don't exceed plugin limits (e.g. saving old state)
    if (value > 2.0f) value = 2.0f;
    if (value < 0.0f) value = 0.0f;

    if (m_saturation != value) {
        m_saturation = value;
        if (m_videoBalance) {
            g_object_set(m_videoBalance, "saturation", (double)m_saturation, NULL);
        }
    }
}

void GstPlayer::setGamma(float value) {
    if (m_gamma != value) {
        m_gamma = value;
        if (m_videoGamma) {
             g_object_set(m_videoGamma, "gamma", (double)m_gamma, NULL);
        }
        emit videoAdjustmentsChanged();
    }
}

// --- Geometry ---

void GstPlayer::setOrientation(int angle) {
    if (m_orientation != angle) {
        m_orientation = angle;
        updateFlipMethod();
        emit orientationChanged();
    }
}

void GstPlayer::setMirror(bool value) {
    if (m_mirror != value) {
        m_mirror = value;
        updateFlipMethod();
        emit mirrorChanged();
    }
}

void GstPlayer::updateFlipMethod() {
    if (!m_videoFlip) return;
    
    // Method enum:
    // 0=none, 1=cw, 2=rotate-180, 3=ccw
    // 4=horiz, 5=vert, 6=upper-left, 7=upper-right
    
    int method = 0;
    // Simplify angle to 0, 90, 180, 270
    int angle = m_orientation % 360;
    if (angle < 0) angle += 360;
    
    // Check for weird orientation issue described by user: preview is OK, fullscreen upside down?
    // Actually both use the same GstPlayer logic. Could be "rotate-180" metadata causing confusion.
    // If the image is upside down when expected upright, it's 180 degrees off.
    
    // Standard rotation steps
    int rot = 0; // 0=0, 1=90, 2=180, 3=270
    if (angle >= 45 && angle < 135) rot = 1;
    else if (angle >= 135 && angle < 225) rot = 2;
    else if (angle >= 225 && angle < 315) rot = 3;
    
    if (!m_mirror) {
        method = rot; 
    } else {
        // Mirror logic
        switch (rot) {
            case 0: method = 4; break;
            case 1: method = 7; break;
            case 2: method = 5; break;
            case 3: method = 6; break;
        }
    }
    
    // Use "method" property, not "video-direction". 
    // "video-direction" is an enum alias but "method" is the canonical property name for videoflip.
    // Also, if the user sees upside down image by default, and rotation is 0,
    // they can just set rotation to 180 in UI. 
    // But if the issue is that internal metadata is flipping it, we might need to override.
    
    g_object_set(m_videoFlip, "method", method, NULL);
}

void GstPlayer::setCameraId(const QString& id) {
    if (m_cameraId != id) {
        m_cameraId = id;
        emit cameraIdChanged();
    }
}

void GstPlayer::setAnalyticsEngine(QObject* engine) {
    if (m_analyticsEngine != engine) {
        m_analyticsEngine = engine;
        emit analyticsEngineChanged();
    }
}

GstFlowReturn GstPlayer::onNewSample(GstElement *sink, GstPlayer *player)
{
    GstSample *sample;
    GstBuffer *buffer;
    GstMapInfo map;

    g_signal_emit_by_name(sink, "pull-sample", &sample);
    if (sample) {
        buffer = gst_sample_get_buffer(sample);
        gst_buffer_map(buffer, &map, GST_MAP_READ);
        
        GstCaps *caps = gst_sample_get_caps(sample);
        GstStructure *s = gst_caps_get_structure(caps, 0);
        int width, height;
        gst_structure_get_int(s, "width", &width);
        gst_structure_get_int(s, "height", &height);
        
        {
            // Update QObject-owned fields on GUI thread only
            if (width != player->m_videoWidth || height != player->m_videoHeight) {
                const int w = width;
                const int h = height;
                QMetaObject::invokeMethod(player, [player, w, h]() {
                    if (!player) return;
                    if (player->m_videoWidth != w || player->m_videoHeight != h) {
                        player->m_videoWidth = w;
                        player->m_videoHeight = h;
                        emit player->videoStatsChanged();
                    }
                }, Qt::QueuedConnection);
            }

            // Estimate FPS from timestamp difference if duration is not available
            // Note: This is a rough estimation. Better to use gst_util_uint64_scale
        }

        // Copy data to QImage
        // Format is RGBA (bytes per pixel = 4)
        {
            // Collect stats
            player->m_frameCountInst++;
            player->m_lastFrameMs = QDateTime::currentMSecsSinceEpoch();
            // Note: Byte counting moved to source pad probe for correct network bitrate
            
            QMutexLocker locker(&player->m_frameMutex);
            // Create a deep copy because GStreamer buffer will be unmapped/freed
            QImage img(map.data, width, height, QImage::Format_RGBA8888);
            player->m_currentFrame = img.copy(); 
            player->m_frameReady = true;
        }

        // Analytics processing (queued to main thread)
        if (player->m_analyticsEngine && !player->m_cameraId.isEmpty()) {
            auto *engine = qobject_cast<AnalyticsEngine *>(player->m_analyticsEngine);
            if (engine && engine->hasActiveModules(player->m_cameraId) && !engine->isBusy(player->m_cameraId)) {
                QImage analyticsFrame;
                {
                    QMutexLocker locker(&player->m_frameMutex);
                    analyticsFrame = player->m_currentFrame;
                }
                QMetaObject::invokeMethod(engine, "processFrame", Qt::QueuedConnection,
                                          Q_ARG(QImage, analyticsFrame),
                                          Q_ARG(QString, player->m_cameraId));
            }
        }

        gst_buffer_unmap(buffer, &map);
        gst_sample_unref(sample);

        // Notify UI thread to redraw
        QMetaObject::invokeMethod(player, "update", Qt::QueuedConnection);
        
        emit player->frameReady();
        
        return GST_FLOW_OK;
    }
    return GST_FLOW_ERROR;
}


void GstPlayer::onSourcePadAdded(GstElement *, GstPad *pad, gpointer user_data) {
    GstPlayer *self = static_cast<GstPlayer *>(user_data);
    gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, &GstPlayer::onSourcePadProbe, self, NULL);

    // Initial Codec Detection from Caps
    GstCaps *caps = gst_pad_get_current_caps(pad);
    if (caps) {
        GstStructure *s = gst_caps_get_structure(caps, 0);
        if (s) {
            const gchar *encoding = gst_structure_get_string(s, "encoding-name");
            if (encoding) {
                QString codec = QString::fromUtf8(encoding);
                QMetaObject::invokeMethod(self, [self, codec]() {
                    if (!self) return;
                    if (self->m_videoCodec != codec) {
                        self->m_videoCodec = codec;
                        emit self->videoStatsChanged();
                    }
                }, Qt::QueuedConnection);
            }
        }
        gst_caps_unref(caps);
    }
}

void GstPlayer::onRecordingPadAdded(GstElement *element, GstPad *pad, gpointer user_data) {
    GstPlayer *self = static_cast<GstPlayer *>(user_data);
    
    // Ensure we are on the right thread or locked? 
    // Usually signals are called from streaming thread.
    
    GstCaps *caps = gst_pad_get_current_caps(pad);
    if (!caps) caps = gst_pad_query_caps(pad, NULL);

    if (caps) {
        gchar *capsStr = gst_caps_to_string(caps);
        qInfo() << "Recording Pad Added:" << capsStr;
        g_free(capsStr);
        
        GstStructure *s = gst_caps_get_structure(caps, 0);
        const gchar *name = gst_structure_get_name(s);
        const gchar *encoding = gst_structure_get_string(s, "encoding-name");
        
        qInfo() << "Recording Structure:" << name << "Encoding:" << encoding;

        // Check for video
        if (g_str_has_prefix(name, "application/x-rtp") && g_strcmp0(gst_structure_get_string(s, "media"), "video") == 0) {
            
            // Build depay/parse chain
            GstElement *depay = nullptr;
            GstElement *parse = nullptr;
            
            if (g_strcmp0(encoding, "H264") == 0) {
                depay = gst_element_factory_make("rtph264depay", "depay");
                parse = gst_element_factory_make("h264parse", "parse");
            } else if (g_strcmp0(encoding, "H265") == 0) {
                depay = gst_element_factory_make("rtph265depay", "depay");
                parse = gst_element_factory_make("h265parse", "parse");
            }
            
            if (depay && parse) {
                qInfo() << "Linking Recording elements for" << encoding;
                
                GstElement *queue = gst_element_factory_make("queue", NULL);
                if (queue) {
                   gst_bin_add_many(GST_BIN(self->m_pipeline), depay, parse, queue, NULL);
                   gst_element_sync_state_with_parent(queue);
                } else {
                   gst_bin_add_many(GST_BIN(self->m_pipeline), depay, parse, NULL);
                }
                
                // Link src->depay->parse
                gst_element_sync_state_with_parent(depay);
                gst_element_sync_state_with_parent(parse);
                
                GstPad *sinkPad = gst_element_get_static_pad(depay, "sink");
                if (gst_pad_link(pad, sinkPad) != GST_PAD_LINK_OK) {
                     qWarning() << "Failed to link RTSP pad to Depayloader";
                }
                gst_object_unref(sinkPad);
                
                if (!gst_element_link(depay, parse)) {
                     qWarning() << "Failed to link Depayloader to Parser";
                }
                
                GstElement *lastElement = parse;
                if (queue) {
                    if (!gst_element_link(parse, queue)) {
                        qWarning() << "Failed to link Parser to Queue";
                    }
                    lastElement = queue;
                }

                // Link lastElement->muxer
                if (self->m_muxer) {
                    // Use pad template request if simple fails? No, simple is for request pads.
                    // mp4mux uses 'video_%u'
                    GstPad *muxPad = gst_element_request_pad_simple(self->m_muxer, "video_%u");
                    if (muxPad) {
                         GstPad *srcPad = gst_element_get_static_pad(lastElement, "src");
                         if (gst_pad_link(srcPad, muxPad) != GST_PAD_LINK_OK) {
                              qWarning() << "Failed to link to Muxer";
                         } else {
                              qInfo() << "Successfully linked video to Muxer";
                              
                              // Workaround: H264/H265 might need config-interval for mp4mux to write headers initially?
                              // mp4mux usually waits for headers.
                         }
                         gst_object_unref(srcPad);
                         gst_object_unref(muxPad);
                    } else {
                         qWarning() << "Failed to request pad from Muxer";
                    }
                }
            } else {
               qWarning() << "Unsupported codec for recording:" << encoding << "or missing plugins";
            }
        }
        // Check for audio
        else if (g_str_has_prefix(name, "application/x-rtp") && g_strcmp0(gst_structure_get_string(s, "media"), "audio") == 0) {
            
            bool needsTranscoding = true;
            GstElement *depay = nullptr;
            GstElement *decoder = nullptr;
            
            if (g_strcmp0(encoding, "MPEG4-GENERIC") == 0) {
                depay = gst_element_factory_make("rtpmp4gdepay", "adepay");
                needsTranscoding = false; 
                
                // Check if aacparse is available. If not, we MUST transcode or drop audio.
                // Passthrough without parse leads to 0KB files (stalled muxer).
                GstElement *testParse = gst_element_factory_make("aacparse", NULL);
                if (testParse) {
                    gst_object_unref(testParse);
                } else {
                    qInfo() << "aacparse missing for AAC Passthrough. Attempting transcoding.";
                    needsTranscoding = true;
                    // Try to find a decoder for AAC
                    decoder = gst_element_factory_make("avdec_aac", "adec");
                    if (!decoder) decoder = gst_element_factory_make("faad", "adec");
                }
            } 
            else if (g_strcmp0(encoding, "PCMA") == 0) {
                depay = gst_element_factory_make("rtppcmadepay", "adepay");
                decoder = gst_element_factory_make("alawdec", "adec");
            }
            else if (g_strcmp0(encoding, "PCMU") == 0) {
                depay = gst_element_factory_make("rtppcmudepay", "adepay");
                decoder = gst_element_factory_make("mulawdec", "adec");
            }
            else if (g_strcmp0(encoding, "OPUS") == 0) {
                depay = gst_element_factory_make("rtpopusdepay", "adepay");
                // Opus in MP4 is supported but let's transcode to AAC for max compatibility
                decoder = gst_element_factory_make("opusdec", "adec");
            }
            else if (g_strcmp0(encoding, "L16") == 0) {
                depay = gst_element_factory_make("rtpL16depay", "adepay");
                // L16 is raw, so we treat it as decoded input for the converter
            }

            if (depay) {
                qInfo() << "Configuring Audio path for" << encoding << (needsTranscoding ? "(Transcoded to AAC)" : "(AAC Passthrough)");

                // Request Muxer Pad *EARLY* to avoid race conditions with Video stream effectively locking the muxer
                GstPad *muxPad = nullptr;
                if (self->m_muxer) {
                     muxPad = gst_element_request_pad_simple(self->m_muxer, "audio_%u");
                     if (!muxPad) {
                         qWarning() << "Failed to EARLY request Audio pad from Muxer. Audio will strictly not be recorded.";
                     }
                }
                
                GstElement *parse = gst_element_factory_make("aacparse", "aparse");
                GstElement *queue = gst_element_factory_make("queue", "aqueue");
                
                if (needsTranscoding) {
                    GstElement *converter = gst_element_factory_make("audioconvert", "aconv");
                    GstElement *resampler = gst_element_factory_make("audioresample", "aresample");
                    // Configure encoder: avenc_aac is good, voaacenc is alternative
                    GstElement *encoder = gst_element_factory_make("avenc_aac", "aenc");
                    if (!encoder) {
                        qInfo() << "avenc_aac not found, trying voaacenc...";
                        encoder = gst_element_factory_make("voaacenc", "aenc");
                    }
                    if (!encoder) {
                        qInfo() << "voaacenc not found, trying faac...";
                        encoder = gst_element_factory_make("faac", "aenc");
                    }

                    // CRITICAL: If we don't have a decoder but needed one (e.g. AAC input without parse), we can't proceed.
                    bool hasDecoderOrRaw = (decoder != nullptr) || (g_strcmp0(encoding, "L16") == 0);

                    if (converter && resampler && encoder && queue && hasDecoderOrRaw && muxPad) {
                         // Build chain: depay -> [decoder] -> converter -> resampler -> encoder -> [parse] -> queue
                         gst_bin_add_many(GST_BIN(self->m_pipeline), depay, converter, resampler, encoder, queue, NULL);
                         if (parse) gst_bin_add(GST_BIN(self->m_pipeline), parse);
                         if (decoder) gst_bin_add(GST_BIN(self->m_pipeline), decoder);
                         
                         bool linked = false;
                         if (decoder) {
                             if (parse) linked = gst_element_link_many(depay, decoder, converter, resampler, encoder, parse, queue, NULL);
                             else linked = gst_element_link_many(depay, decoder, converter, resampler, encoder, queue, NULL);
                         } else {
                             if (parse) linked = gst_element_link_many(depay, converter, resampler, encoder, parse, queue, NULL);
                             else linked = gst_element_link_many(depay, converter, resampler, encoder, queue, NULL);
                         }
                         
                         if (linked) {
                             gst_element_sync_state_with_parent(depay);
                             if (decoder) gst_element_sync_state_with_parent(decoder);
                             gst_element_sync_state_with_parent(converter);
                             gst_element_sync_state_with_parent(resampler);
                             gst_element_sync_state_with_parent(encoder);
                             if (parse) gst_element_sync_state_with_parent(parse);
                             gst_element_sync_state_with_parent(queue);
                             
                             // Link to source pad
                             GstPad *sinkPad = gst_element_get_static_pad(depay, "sink");
                             if (gst_pad_link(pad, sinkPad) != GST_PAD_LINK_OK) {
                                  qWarning() << "Failed to link RTSP Audio pad to Depayloader (" << encoding << ")";
                             }
                             gst_object_unref(sinkPad);
                             
                             // Link to Muxer (Using pre-requested pad)
                             GstPad *srcPad = gst_element_get_static_pad(queue, "src");
                             if (gst_pad_link(srcPad, muxPad) != GST_PAD_LINK_OK) {
                                  qWarning() << "Failed to link Transcoded Audio to Muxer";
                             } else {
                                  qInfo() << "Successfully linked Transcoded Audio to Muxer";
                             }
                             gst_object_unref(srcPad);
      
                         } else {
                             qWarning() << "Failed to link audio transcoding chain for" << encoding;
                         }
                    } else {
                        qWarning() << "Missing elements for audio transcoding or MuxPad failed" 
                                   << "Converter:" << (converter ? "OK" : "MISSING")
                                   << "Decoder:" << (hasDecoderOrRaw ? "OK" : "MISSING")
                                   << "MuxPad:" << (muxPad ? "OK" : "MISSING");
                        if (muxPad) {
                            gst_element_release_request_pad(self->m_muxer, muxPad);
                            gst_object_unref(muxPad);
                        }
                    }
                } else {
                    // Direct AAC path: depay -> [parse] -> queue
                    if (queue && muxPad) {
                        gst_bin_add_many(GST_BIN(self->m_pipeline), depay, queue, NULL);
                        if (parse) gst_bin_add(GST_BIN(self->m_pipeline), parse);
                        
                        gst_element_sync_state_with_parent(depay);
                        if (parse) gst_element_sync_state_with_parent(parse);
                        gst_element_sync_state_with_parent(queue);
                        
                        bool linked = false;
                        if (parse) linked = gst_element_link_many(depay, parse, queue, NULL);
                        else linked = gst_element_link_many(depay, queue, NULL);
                        
                        if (linked) {
                             GstPad *sinkPad = gst_element_get_static_pad(depay, "sink");
                             if (gst_pad_link(pad, sinkPad) != GST_PAD_LINK_OK) {
                                  qWarning() << "Failed to link RTSP Audio pad to Depayloader";
                             }
                             gst_object_unref(sinkPad);
                             
                             // Link to Muxer
                             GstPad *srcPad = gst_element_get_static_pad(queue, "src");
                             if (gst_pad_link(srcPad, muxPad) != GST_PAD_LINK_OK) {
                                  qWarning() << "Failed to link Audio to Muxer";
                             } else {
                                  qInfo() << "Successfully linked Audio to Muxer";
                             }
                             gst_object_unref(srcPad);
                        } else {
                             qWarning() << "Failed to link direct audio chain";
                        }
                    } else {
                        qWarning() << "Missing elements for AAC Passthrough or MuxPad failed:" 
                                   << "queue=" << (queue ? "OK" : "MISSING")
                                   << "MuxPad=" << (muxPad ? "OK" : "MISSING");
                         if (muxPad) {
                            gst_element_release_request_pad(self->m_muxer, muxPad);
                            gst_object_unref(muxPad);
                        }
                    }
                }
                if (muxPad) gst_object_unref(muxPad); // Unref the initial reference from request_pad
            } else {
                 qWarning() << "Unsupported audio recording codec:" << encoding;
            }
            // Done with audio branch
            gst_caps_unref(caps);
            return; 
        }
    } else {
        qWarning() << "Recording Pad Added but NO CAPS";
    }
}

GstPadProbeReturn GstPlayer::onSourcePadProbe(GstPad *, GstPadProbeInfo *info, gpointer user_data) {
    GstPlayer *self = static_cast<GstPlayer *>(user_data);
    if (GST_PAD_PROBE_INFO_TYPE(info) & GST_PAD_PROBE_TYPE_BUFFER) {
        GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER(info);
        if (buffer) {
             self->m_byteCountInst += gst_buffer_get_size(buffer);
        }
    }
    return GST_PAD_PROBE_OK;
}

void GstPlayer::updateStats() {
    int frames = m_frameCountInst.exchange(0);
    // bytes per second * 8 = bits per second. / 1000 = kbps.
    long long bytes = m_byteCountInst.exchange(0);
    int bitrate = (int)((bytes * 8) / 1000);

    // Only emit if changed significantly or if it was 0
    bool changed = false;
    if (m_videoFps != frames) {
        m_videoFps = frames;
        changed = true;
    }
    // Smooth bitrate a bit or just report it? Let's report raw for now.
    // If bitrate fluctuates wildly, maybe add smoothing.
    if (abs(m_videoBitrate - bitrate) > 50 || (bitrate == 0 && m_videoBitrate != 0) || (bitrate > 0 && m_videoBitrate == 0)) {
         m_videoBitrate = bitrate;
         changed = true;
    }
    
    if (changed) {
        emit videoStatsChanged();
    }

    // Reconnect watchdog: if no frames for a while, restart pipeline
    if (m_running && !m_url.isEmpty()) {
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        const qint64 sinceStart = now - m_pipelineStartMs;
        const qint64 sinceFrame = (m_lastFrameMs > 0) ? (now - m_lastFrameMs) : sinceStart;
        if (sinceStart > 8000 && sinceFrame > 8000) {
            QMetaObject::invokeMethod(this, "scheduleReconnect", Qt::QueuedConnection);
        }
    }

    // Update Position & Duration
    if (m_pipeline && m_running && m_recordingPath.isEmpty()) { // Only for playback
        qint64 dur = duration();
        qint64 pos = position();
        
        if (m_lastDuration != dur) {
            m_lastDuration = dur;
            emit durationChanged();
        }
        
        // Always emit position changed if running, or check diff
        if (abs(m_lastPosition - pos) >= 0) { // Emit always so UI slider moves smoothly? Or at least every sec
            m_lastPosition = pos;
            emit positionChanged();
        }
    }
}

void GstPlayer::updateMediaInfo() {
    if (!m_pipeline) return;

    // Use tag events to find codec and bitrate
    // NOTE: This usually requires a valid stream to be running.
    
    // We can also query the pad caps from the appsink to verify the format, but that's RGBA.
    // We want the codec *before* decoding. 
    // In playbin, we can try to look at signals on the bus for TAG messages.
    // But since this is called periodically, let's assume we handle tags in the bus callback.
    // However, for playbin, simple stats can be retrieved by looking at the stream-info property (deprecated) or signal.
    // Let's implement this lightly by assuming tag handling on bus is better.
    
    // But for a quick update:
    // Try to get video frame rate from the last sample caps if possible.
    // Actually, m_videoWidth/Height are updated in onNewSample.

    // Let's update bitrate/codec from TAG messages (see onBusMessage)
}

void GstPlayer::onBusMessage(GstBus *, GstMessage *msg, gpointer data) { // Moved to GstPlayer scope for private access

    GstPlayer *self = static_cast<GstPlayer*>(data);
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_TAG: {
            GstTagList *tags = nullptr;
            gst_message_parse_tag(msg, &tags);
            if (tags) {
                gchar *codec = nullptr;
                if (gst_tag_list_get_string(tags, GST_TAG_VIDEO_CODEC, &codec)) {
                    QString codecStr = QString::fromUtf8(codec).trimmed();
                    // Remove profile suffix like " (Main Profile)" if present
                    int parenIdx = codecStr.indexOf(" (");
                    if (parenIdx > 0 && codecStr.endsWith(')')) {
                        codecStr = codecStr.left(parenIdx).trimmed();
                    }
                    QMetaObject::invokeMethod(self, [self, codecStr]() {
                        if (!self) return;
                        if (self->m_videoCodec != codecStr) {
                            self->m_videoCodec = codecStr;
                            emit self->videoStatsChanged();
                        }
                    }, Qt::QueuedConnection);
                    g_free(codec);
                }
                
                guint bitrate = 0;
                if (gst_tag_list_get_uint(tags, GST_TAG_BITRATE, &bitrate) || 
                    gst_tag_list_get_uint(tags, GST_TAG_NOMINAL_BITRATE, &bitrate)) {
                    // GStreamer tag bitrate is in bits per second, convert to kbps
                    int bitrateKbps = (int)qRound((double)bitrate / 1000.0);
                    // Ignore clearly bogus values (fallback to measured bitrate)
                    if (bitrateKbps > 0 && bitrateKbps <= 200000) {
                        QMetaObject::invokeMethod(self, [self, bitrateKbps]() {
                            if (!self) return;
                            if (self->m_videoBitrate != bitrateKbps) {
                                self->m_videoBitrate = bitrateKbps;
                                emit self->videoStatsChanged();
                            }
                        }, Qt::QueuedConnection);
                    }
                }
                gst_tag_list_unref(tags);
            }
            break;
        }
        case GST_MESSAGE_ERROR: {
            GError *err;
            gchar *debug;
            gst_message_parse_error(msg, &err, &debug);
            qWarning() << "GStreamer Error:" << err->message;
            if (debug) qWarning() << "Debug Info:" << debug;
            emit self->errorOccurred(QString::fromUtf8(err->message));
            g_error_free(err);
            g_free(debug);
            QMetaObject::invokeMethod(self, "scheduleReconnect", Qt::QueuedConnection);
            break;
        }
        case GST_MESSAGE_EOS: {
            qWarning() << "GStreamer EOS received. Scheduling reconnect.";
            QMetaObject::invokeMethod(self, "scheduleReconnect", Qt::QueuedConnection);
            break;
        }
        case GST_MESSAGE_WARNING: {
            GError *err;
            gchar *debug;
            gst_message_parse_warning(msg, &err, &debug);
            qWarning() << "GStreamer Warning:" << err->message;
            if (debug) qWarning() << "Debug Info:" << debug;
            g_error_free(err);
            g_free(debug);
            break;
        }
        case GST_MESSAGE_STATE_CHANGED: {
             GstState old_state, new_state, pending_state;
             gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
             // Log state change for the pipeline
             if (GST_MESSAGE_SRC(msg) == GST_OBJECT(self->pipeline())) {
                qDebug() << "Pipeline state changed from" << gst_element_state_get_name(old_state) 
                         << "to" << gst_element_state_get_name(new_state);
             }
             break;
        }
        default:
            break;
    }
}

GstElement* GstPlayer::createVideoFilterBin() {
    // Bin to hold balance and flip
    GstElement *bin = gst_bin_new("video_filter_bin");
    if (!bin) return nullptr;

    m_videoBalance = gst_element_factory_make("videobalance", "balance");
    if (!m_videoBalance) {
        qWarning() << "Failed to create 'videobalance' element. Check if gst-plugins-good is installed properly.";
        // Check availability
        GstElementFactory *factory = gst_element_factory_find("videobalance");
        if (!factory) qWarning() << "Factory 'videobalance' not found in registry.";
        else gst_object_unref(factory);
    }
    
    m_videoFlip = gst_element_factory_make("videoflip", "flip");
    if (!m_videoFlip) {
        qWarning() << "Failed to create 'videoflip' element.";
    }
    
    // Attempt to create 'gamma' element
    m_videoGamma = gst_element_factory_make("gamma", "gamma");
    if (!m_videoGamma) {
        // Fallback or just ignore if missing. It is part of gst-plugins-good.
        // If missing, gamma slider won't work but stream will play.
        // qWarning() << "Failed to create 'gamma' element."; 
    }

    if (!m_videoBalance && !m_videoFlip && !m_videoGamma) {
        gst_object_unref(bin);
        return nullptr;
    }
    
    // Create converters
    // Revert to standard videoconvert to avoid d3d11upload/glcolorconvert linking errors seen with autovideoconvert
    GstElement *conv1 = gst_element_factory_make("videoconvert", "conv1");
    GstElement *conv2 = gst_element_factory_make("videoconvert", "conv2");
    
    if (!conv1 || !conv2) {
         qWarning() << "Failed to create videoconvert elements";
         // Proceed without convert? Unlikely to work but let's try
    }
    
    // Construct pipeline: conv1 -> balance (opt) -> flip (opt) -> conv2
    
    gst_bin_add(GST_BIN(bin), conv1);
    GstElement *prev = conv1;
    
    if (m_videoBalance) {
        gst_bin_add(GST_BIN(bin), m_videoBalance);
        if (!gst_element_link(prev, m_videoBalance)) qWarning() << "Failed to link to balance";
        prev = m_videoBalance;
    }
    
    if (m_videoFlip) {
        gst_bin_add(GST_BIN(bin), m_videoFlip);
        if (!gst_element_link(prev, m_videoFlip)) qWarning() << "Failed to link to flip";
        prev = m_videoFlip;
    }
    
    if (m_videoGamma) {
        gst_bin_add(GST_BIN(bin), m_videoGamma);
        if (!gst_element_link(prev, m_videoGamma)) qWarning() << "Failed to link to gamma";
        prev = m_videoGamma;
    }

    gst_bin_add(GST_BIN(bin), conv2);
    if (!gst_element_link(prev, conv2)) qWarning() << "Failed to link to conv2";
    
    // Helper to add ghost pads
    auto addGhostPad = [&](GstElement* element, const gchar* padName) {
        GstPad *pad = gst_element_get_static_pad(element, padName);
        if (pad) {
            gst_element_add_pad(bin, gst_ghost_pad_new(padName, pad));
            gst_object_unref(pad);
        } else {
            qWarning() << "Failed to get pad" << padName << "from" << GST_ELEMENT_NAME(element);
        }
    };

    addGhostPad(conv1, "sink");
    addGhostPad(conv2, "src");

    // Configure initial values
    if (m_videoBalance) {
        // m_brightness is already stored in GStreamer range (-1..1) by setBrightness
        // m_contrast and m_saturation are stored in UI range (0..2)
        // m_hue is stored in UI range (-180..180 or similar)

        double gstBrightness = (double)m_brightness;
        
        // Clamp saturation to 0..2 range supported by videobalance
        double gstSaturation = (double)m_saturation;
        if (gstSaturation > 2.0) gstSaturation = 2.0;
        if (gstSaturation < 0.0) gstSaturation = 0.0;
        
        g_object_set(m_videoBalance, "brightness", gstBrightness, 
                                     "contrast", (double)m_contrast,
                                     "saturation", gstSaturation,
                                     "hue", (double)((float)m_hue/180.0f), NULL);
    }
    
    if (m_videoGamma) {
        g_object_set(m_videoGamma, "gamma", (double)m_gamma, NULL);
    }

    updateFlipMethod(); 

    return bin;
}

void GstPlayer::startPipeline()
{
    stopPipeline();

    if (m_url.isEmpty()) return;

    m_pipelineStartMs = QDateTime::currentMSecsSinceEpoch();
    m_lastFrameMs = 0;

#ifdef Q_OS_WIN
    applyHwDecodingPreference();
#endif

    GError *error = nullptr;

    // --- Recording Mode ---
    if (!m_recordingPath.isEmpty()) {
        qInfo() << "Recording Mode Started. URL:" << m_url << "Path:" << m_recordingPath;

        // Construct a dedicated recording pipeline
        // rtspsrc -> depay -> parse -> mp4mux -> filesink
        // We use 'rtspsrc' and dynamic pad linking.
        
        // Escape backslashes in path for GStreamer string
        QString safePath = m_recordingPath;
        safePath.replace("\\", "/");
        
        QString pipelineDesc = QString("rtspsrc location=\"%1\" name=src protocols=tcp mp4mux name=mux reserved-max-duration=3600000000000 reserved-moov-update-period=2000000000 ! filesink location=\"%2\"")
                               .arg(m_url)
                               .arg(safePath);

        qDebug() << "Recording Pipeline:" << pipelineDesc;

        m_pipeline = gst_parse_launch(pipelineDesc.toUtf8().constData(), &error);
        
        if (error) {
            qWarning() << "Recording Pipeline Error:" << error->message;
            g_error_free(error);
            return;
        }
        
        // Setup dynamic linking
        GstElement *src = gst_bin_get_by_name(GST_BIN(m_pipeline), "src");
        m_muxer = gst_bin_get_by_name(GST_BIN(m_pipeline), "mux");
        
        if (src && m_muxer) {
            g_signal_connect(src, "pad-added", G_CALLBACK(&GstPlayer::onRecordingPadAdded), this);
        }
        
        if (src) gst_object_unref(src);
        
    } else {
        // --- Playback Mode ---
        // "sync=false" for low latency modes to render immediately
        // "emit-signals=true drop=true max-buffers=1" ensures we only get the latest frame
        bool isFile = m_url.startsWith("file://") || m_url.startsWith("/"); 
        bool sync = (m_bufferMode == 0) || isFile; // Always sync for files to prevent fast-forward
        QString pipelineStr = QString("playbin uri=\"%1\" video-sink=\"appsink name=sink emit-signals=true drop=true max-buffers=1 sync=%2\"")
                                  .arg(m_url)
                                  .arg(sync ? "true" : "false");

        m_pipeline = gst_parse_launch(pipelineStr.toUtf8().constData(), &error);

        if (error) {
            qWarning() << "GStreamer Parse Error:" << error->message;
            emit errorOccurred(QString::fromUtf8(error->message));
            g_error_free(error);
            return;
        }

        GstElement *filterBin = createVideoFilterBin();
        if (filterBin) {
             // IMPORTANT: Set "video-filter" on playbin, not "video-sink"
         g_object_set(G_OBJECT(m_pipeline), "video-filter", filterBin, NULL);
         // Filter bin ownership is transferred to playbin
         
         // Store references - they are valid as long as pipeline exists
         m_videoBalance = gst_bin_get_by_name(GST_BIN(filterBin), "balance");
         m_videoFlip = gst_bin_get_by_name(GST_BIN(filterBin), "flip");
         m_videoGamma = gst_bin_get_by_name(GST_BIN(filterBin), "gamma");
         
         if (m_videoBalance) gst_object_unref(m_videoBalance); // get_by_name adds ref
         if (m_videoFlip) gst_object_unref(m_videoFlip);
         if (m_videoGamma) gst_object_unref(m_videoGamma);
    }

        // Configure low latency via source-setup
        g_signal_connect(m_pipeline, "source-setup", G_CALLBACK(+[](GstElement* pipeline, GstElement* source, GstPlayer* self){
             // Set latency
             if (g_object_class_find_property(G_OBJECT_GET_CLASS(source), "latency")) {
                 int latency = 2000;
                 if (self->m_bufferMode == 1) latency = 200;
                 else if (self->m_bufferMode == 2) latency = 0;
                 g_object_set(source, "latency", latency, NULL);
             }
             // Set RTSP protocols if applicable (rtspsrc)
             if (g_object_class_find_property(G_OBJECT_GET_CLASS(source), "protocols")) {
                 // GstRTSPLowerTrans: UDP=1, UDP_MCAST=2, TCP=4
                 int protocols = 0x00000007; // UDP|UDP_MCAST|TCP (default)
                 if (self->m_rtspTransport == "tcp") protocols = 0x00000004;
                 else if (self->m_rtspTransport == "udp") protocols = 0x00000001;
                 else if (self->m_rtspTransport == "udp_mcast") protocols = 0x00000002;
                 else if (self->m_rtspTransport == "http") protocols = 0x00000004;
                 g_object_set(source, "protocols", protocols, NULL);
             }
             // Drop on latency for ultra low latency
             if (self->m_bufferMode == 2 && g_object_class_find_property(G_OBJECT_GET_CLASS(source), "drop-on-latency")) {
                 g_object_set(source, "drop-on-latency", TRUE, NULL);
             }
             
             // Setup stats probing on source pads (Network Bitrate & Codec Detection)
             g_signal_connect(source, "pad-added", G_CALLBACK(&GstPlayer::onSourcePadAdded), self);
             
             // Also iterate existing pads (for non-dynamic sources)
             GstIterator *it = gst_element_iterate_src_pads(source);
             GValue item = G_VALUE_INIT;
             bool done = false;
             while (!done) {
                switch (gst_iterator_next(it, &item)) {
                    case GST_ITERATOR_OK: {
                        GstPad *pad = (GstPad *)g_value_get_object(&item);
                        GstPlayer::onSourcePadAdded(source, pad, self);
                        g_value_reset(&item);
                        break;
                    }
                    case GST_ITERATOR_RESYNC:
                        gst_iterator_resync(it);
                        break;
                    case GST_ITERATOR_ERROR:
                    case GST_ITERATOR_DONE:
                        done = true;
                        break;
                }
             }
             gst_iterator_free(it);
        }), this);

        // Get the appsink (playbin video-sink property)
        GstElement *sink = nullptr;
        g_object_get(m_pipeline, "video-sink", &sink, NULL);
        
        if (sink) {
            // Enforce RGBA caps
            GstCaps *caps = gst_caps_from_string("video/x-raw, format=RGBA");
            g_object_set(sink, "caps", caps, NULL);
            gst_caps_unref(caps);

            g_signal_connect(sink, "new-sample", G_CALLBACK(onNewSample), this);
            gst_object_unref(sink);
        } else {
            // qWarning() << "Could not find appsink in pipeline!"; 
            // Might be harmless if error occurred earlier
        }
        
        // playbin implements GstStreamVolume
        m_volumeElement = m_pipeline; 
        
        if (m_volumeElement) {
            g_object_set(m_volumeElement, "volume", m_volume, NULL);
            g_object_set(m_volumeElement, "mute", m_muted, NULL);
        }

        // Disable audio chain when muted to avoid wasapi sink errors
        updatePlaybinAudioFlags();
    } // End Playback Mode

    GstStateChangeReturn ret = gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qWarning() << "Unable to set the pipeline to the playing state.";
        stopPipeline();
    } else {
        // Add bus watch for errors
        GstBus *bus = gst_element_get_bus(m_pipeline);
        gst_bus_add_signal_watch(bus);
        g_signal_connect(bus, "message", G_CALLBACK(&GstPlayer::onBusMessage), this);
        gst_object_unref(bus);
        
        m_frameCountInst = 0;
        m_byteCountInst = 0;
        m_statsTimer->start();
    }
}

void GstPlayer::updatePlaybinAudioFlags()
{
    if (!m_pipeline) return;

    if (!g_object_class_find_property(G_OBJECT_GET_CLASS(m_pipeline), "flags")) return;

    guint flags = 0;
    g_object_get(G_OBJECT(m_pipeline), "flags", &flags, NULL);

    // playbin flags: video=0x1, audio=0x2, text=0x4
    const guint GST_PLAY_FLAG_AUDIO = 0x2;

    if (m_muted) flags &= ~GST_PLAY_FLAG_AUDIO;
    else flags |= GST_PLAY_FLAG_AUDIO;

    g_object_set(G_OBJECT(m_pipeline), "flags", flags, NULL);
}

void GstPlayer::applyHwDecodingPreference()
{
    auto setRank = [](const char* name, guint rank) {
        GstRegistry* registry = gst_registry_get();
        GstPluginFeature* feature = gst_registry_find_feature(registry, name, GST_TYPE_ELEMENT_FACTORY);
        if (feature) {
            gst_plugin_feature_set_rank(feature, rank);
            gst_object_unref(feature);
        }
    };

    const bool disableHw = (m_hwDecoding == "none");
    const bool preferHw = (m_hwDecoding == "auto" || m_hwDecoding == "d3d11" || m_hwDecoding == "dxva2");

    if (disableHw) {
        // Prefer software decoders
        setRank("d3d11h264dec", GST_RANK_NONE);
        setRank("d3d11h265dec", GST_RANK_NONE);
        setRank("d3d11vp9dec", GST_RANK_NONE);
        setRank("d3d11av1dec", GST_RANK_NONE);
        setRank("avdec_h264", GST_RANK_PRIMARY + 50);
        setRank("avdec_h265", GST_RANK_PRIMARY + 50);
        setRank("avdec_vp9", GST_RANK_PRIMARY + 50);
        setRank("avdec_av1", GST_RANK_PRIMARY + 50);
        setRank("openh264dec", GST_RANK_PRIMARY + 40);
        return;
    }

    if (preferHw) {
        // Prefer D3D11/MFT (DXVA) decoders when available
        setRank("d3d11h264dec", GST_RANK_PRIMARY + 200);
        setRank("d3d11h265dec", GST_RANK_PRIMARY + 200);
        setRank("d3d11vp9dec", GST_RANK_PRIMARY + 150);
        setRank("d3d11av1dec", GST_RANK_PRIMARY + 150);

        // Keep software decoders as fallback
        setRank("avdec_h264", GST_RANK_SECONDARY);
        setRank("avdec_h265", GST_RANK_SECONDARY);
        setRank("avdec_vp9", GST_RANK_SECONDARY);
        setRank("avdec_av1", GST_RANK_SECONDARY);
        setRank("openh264dec", GST_RANK_SECONDARY);
    }
}

void GstPlayer::stopPipeline()
{
    if (m_statsTimer) m_statsTimer->stop();
    if (m_reconnectTimer) m_reconnectTimer->stop();

    if (m_pipeline) {
        // If recording, we must send EOS to finalize the MP4 file
        if (!m_recordingPath.isEmpty() && m_running) {
             qDebug() << "Stopping recording, sending EOS...";
             
             // Send EOS from the source element
             GstElement *src = gst_bin_get_by_name(GST_BIN(m_pipeline), "src");
             if (src) {
                 gst_element_send_event(src, gst_event_new_eos());
                 gst_object_unref(src);
                 
                 // Wait for EOS message on the bus (up to 2 seconds)
                 GstBus *bus = gst_element_get_bus(m_pipeline);
                 GstMessage *msg = gst_bus_timed_pop_filtered(bus, 2000 * GST_MSECOND, (GstMessageType)(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));
                 
                 if (msg) {
                     if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
                         qDebug() << "EOS received, recording finished successfully.";
                     } else {
                         qWarning() << "Error received while waiting for EOS.";
                     }
                     gst_message_unref(msg);
                 } else {
                     qWarning() << "Timeout waiting for EOS (2s). Force stopping.";
                 }
                 gst_object_unref(bus);
             }
        }

        gst_element_set_state(m_pipeline, GST_STATE_NULL);
        gst_object_unref(m_pipeline);
        m_pipeline = nullptr;
    }
    m_volumeElement = nullptr;
    m_muxer = nullptr;
    m_videoBalance = nullptr;
    m_videoFlip = nullptr;
    m_videoGamma = nullptr;
    
    // Reset stats
    m_videoFps = 0; 
    m_videoBitrate = 0;
    emit videoStatsChanged();
}

void GstPlayer::scheduleReconnect()
{
    if (!m_running || m_url.isEmpty()) return;
    if (m_reconnectTimer && !m_reconnectTimer->isActive()) {
        m_reconnectTimer->start();
    }
}

void GstPlayer::restartPipeline()
{
    stopPipeline();
    startPipeline();
}

QImage GstPlayer::getLastFrame()
{
    QMutexLocker locker(&m_frameMutex);
    return m_currentFrame;
}

QImage GstPlayer::takeFrameCopy()
{
    return getLastFrame().copy(); 
}

bool GstPlayer::saveSnapshot(const QString &path)
{
    QImage img = getLastFrame();
    if (img.isNull()) return false;
    
    QString localPath = path;
    if (localPath.startsWith("file://")) {
        localPath = localPath.mid(7);
    }
#ifdef Q_OS_WIN
    if (localPath.startsWith("/")) localPath = localPath.mid(1);
#endif

    return img.save(localPath);
}

QSGNode *GstPlayer::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode *>(oldNode);

    if (!node) {
        node = new QSGSimpleTextureNode();
    }

    QImage img = getLastFrame();
    if (!img.isNull()) {
        QSGTexture *texture = window()->createTextureFromImage(img);
        
        if (node->texture()) {
            delete node->texture();
        }
        node->setTexture(texture);
        node->setRect(boundingRect());
    }

    return node;
}

qint64 GstPlayer::duration() const
{
    if (!m_pipeline) return 0;
    gint64 duration = 0;
    if (gst_element_query_duration(m_pipeline, GST_FORMAT_TIME, &duration)) {
        return duration / GST_MSECOND;
    }
    return 0;
}

qint64 GstPlayer::position() const
{
    if (!m_pipeline) return 0;
    gint64 position = 0;
    if (gst_element_query_position(m_pipeline, GST_FORMAT_TIME, &position)) {
        return position / GST_MSECOND;
    }
    return 0;
}

void GstPlayer::setPosition(qint64 pos)
{
    if (!m_pipeline) return;
    gint64 nanosp = pos * GST_MSECOND;
    if (!gst_element_seek_simple(m_pipeline, GST_FORMAT_TIME, (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), nanosp)) {
        qWarning() << "Seek failed!";
    }
}


void GstPlayer::setPlaybackRate(double rate)
{
    if (m_playbackRate == rate) return;
    m_playbackRate = rate;
    emit playbackRateChanged();

    if (m_pipeline && m_running && m_recordingPath.isEmpty()) {
        gint64 position = 0;
        if (gst_element_query_position(m_pipeline, GST_FORMAT_TIME, &position)) {
             GstSeekFlags flags = (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT);
             if (rate > 0) {
                 gst_element_seek(m_pipeline, rate, GST_FORMAT_TIME, flags,
                                  GST_SEEK_TYPE_SET, position, 
                                  GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
             }
        }
    }
}

