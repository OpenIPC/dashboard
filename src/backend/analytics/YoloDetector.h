#pragma once

#include "InferenceBackend.h"
#include <QObject>
#include <QMap>
#include <QMutex>

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
    struct Impl;
    struct PreprocessInfo {
        int inputW = 640;
        int inputH = 640;
        int sourceW = 0;
        int sourceH = 0;
        float scale = 1.0f;
        int padX = 0;
        int padY = 0;
    };

    Options m_options;
    bool m_loaded = false;
    QString m_error;
    std::unique_ptr<Impl> m_impl;
    
    // Helper for NMS (Non-Maximum Suppression)
    void applyNMS(QVector<DetectionBox> &detections);
    
    // Helper for preprocessing
    std::vector<float> preprocess(const QImage &img, PreprocessInfo &info);
};
