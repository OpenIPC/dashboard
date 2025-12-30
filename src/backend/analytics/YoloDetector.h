#pragma once

#include "InferenceBackend.h"
#include <QObject>
#include <QMap>
#include <QMutex>

// Define ORT_API_MANUAL_INIT before including onnxruntime_cxx_api.h
// to prevent it from trying to use features not available in the C API struct
// or to force a specific initialization mode if needed.
// However, the error suggests missing members in OrtApi struct for the C++ wrapper.
// Let's try to define the API version before including.
#include <onnxruntime_cxx_api.h>

// This class wraps the ONNX Runtime C++ API
class YoloDetector : public InferenceBackend {
public:
    struct Options {
        QString modelFile;
        QStringList classLabels;
        QStringList colorPalette;
        float confidenceThreshold = 0.3f;
        float nmsThreshold = 0.45f;
    };

    explicit YoloDetector(const Options &options);
    ~YoloDetector() override;

    bool load(const QString &moduleDir) override;
    bool isLoaded() const override;
    QVector<DetectionBox> detect(const QImage &frame) override;
    QString getError() const override;

private:
    Options m_options;
    bool m_loaded = false;
    QString m_error;
    
    // ONNX Runtime resources
    std::unique_ptr<Ort::Env> m_env;
    std::unique_ptr<Ort::Session> m_session;
    std::unique_ptr<Ort::SessionOptions> m_sessionOptions;
    
    // Model input/output info
    std::vector<const char*> m_inputNodeNames;
    std::vector<const char*> m_outputNodeNames;
    std::vector<std::string> m_inputNodeNamesAllocated;
    std::vector<std::string> m_outputNodeNamesAllocated;
    std::vector<int64_t> m_inputShape;
    
    // Helper for NMS (Non-Maximum Suppression)
    void applyNMS(QVector<DetectionBox> &detections);
    
    // Helper for preprocessing
    std::vector<float> preprocess(const QImage &img, int &outW, int &outH);
};
