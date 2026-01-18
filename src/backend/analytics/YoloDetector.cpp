#include "YoloDetector.h"
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QThread>
#include <algorithm>
#include <cmath>

YoloDetector::YoloDetector(const Options &options)
    : m_options(options)
{
}

YoloDetector::~YoloDetector()
{
    // Smart pointers will handle cleanup
}

bool YoloDetector::load(const QString &moduleDir)
{
    QString modelPath = QDir(moduleDir).filePath(m_options.modelFile);
    
    QFileInfo fileInfo(modelPath);
    if (!fileInfo.exists() || fileInfo.size() == 0) {
        m_error = QString("Model file not found or empty: %1").arg(modelPath);
        qInfo() << "YoloDetector:" << m_error;
        m_loaded = false;
        return false;
    }

    try {
        m_env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "OpenIPCDashboard");
        m_sessionOptions = std::make_unique<Ort::SessionOptions>();
        m_sessionOptions->SetIntraOpNumThreads(1);
        m_sessionOptions->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        // Convert path to wstring for Windows
#ifdef Q_OS_WIN
        std::wstring modelPathW = modelPath.toStdWString();
        m_session = std::make_unique<Ort::Session>(*m_env, modelPathW.c_str(), *m_sessionOptions);
#else
        std::string modelPathStr = modelPath.toStdString();
        m_session = std::make_unique<Ort::Session>(*m_env, modelPathStr.c_str(), *m_sessionOptions);
#endif

        // Get input info
        Ort::AllocatorWithDefaultOptions allocator;
        size_t numInputNodes = m_session->GetInputCount();
        m_inputNodeNames.clear();
        m_inputNodeNamesAllocated.clear();

        for (size_t i = 0; i < numInputNodes; i++) {
            auto inputName = m_session->GetInputNameAllocated(i, allocator);
            m_inputNodeNamesAllocated.push_back(inputName.get());
            m_inputNodeNames.push_back(m_inputNodeNamesAllocated.back().c_str());
            
            auto typeInfo = m_session->GetInputTypeInfo(i);
            auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
            m_inputShape = tensorInfo.GetShape();
        }

        // Get output info
        size_t numOutputNodes = m_session->GetOutputCount();
        m_outputNodeNames.clear();
        m_outputNodeNamesAllocated.clear();

        for (size_t i = 0; i < numOutputNodes; i++) {
            auto outputName = m_session->GetOutputNameAllocated(i, allocator);
            m_outputNodeNamesAllocated.push_back(outputName.get());
            m_outputNodeNames.push_back(m_outputNodeNamesAllocated.back().c_str());
        }

        qInfo() << "YoloDetector: Loaded model from" << modelPath;
        if (m_inputShape.size() >= 4) {
            qInfo() << "Input shape:" << m_inputShape[0] << m_inputShape[1] << m_inputShape[2] << m_inputShape[3];
        }
        
        m_loaded = true;
        m_error.clear();
        return true;

    } catch (const Ort::Exception& e) {
        m_error = QString("ONNX Runtime Error: %1").arg(e.what());
        qWarning() << m_error;
        m_loaded = false;
        return false;
    } catch (const std::exception& e) {
        m_error = QString("Error loading model: %1").arg(e.what());
        qWarning() << m_error;
        m_loaded = false;
        return false;
    }
}

bool YoloDetector::isLoaded() const
{
    return m_loaded;
}

QString YoloDetector::getError() const
{
    return m_error;
}

std::vector<float> YoloDetector::preprocess(const QImage &img, int &outW, int &outH)
{
    // Target size from model input (usually 640x640)
    // m_inputShape is usually [1, 3, 640, 640]
    int targetW = 640;
    int targetH = 640;
    
    if (m_inputShape.size() == 4) {
        targetH = m_inputShape[2];
        targetW = m_inputShape[3];
    }

    outW = targetW;
    outH = targetH;

    // Resize image
    QImage scaled = img.scaled(targetW, targetH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    scaled = scaled.convertToFormat(QImage::Format_RGB888);

    // Convert to float CHW
    std::vector<float> inputTensorValues(targetW * targetH * 3);
    
    const uchar* bits = scaled.bits();
    int stride = scaled.bytesPerLine();

    // Optimize: Reduce implicit coercion and access
    // Pre-calculate inverse 255
    const float inv255 = 1.0f / 255.0f;
    const int offsetG = targetW * targetH;
    const int offsetB = offsetG * 2;

    for (int y = 0; y < targetH; ++y) {
        const uchar* line = bits + y * stride;
        int yOffset = y * targetW;
        for (int x = 0; x < targetW; ++x) {
            const uchar* pixel = line + x * 3;
            // Normalize 0-1
            inputTensorValues[yOffset + x] = pixel[0] * inv255;          // R
            inputTensorValues[offsetG + yOffset + x] = pixel[1] * inv255; // G
            inputTensorValues[offsetB + yOffset + x] = pixel[2] * inv255; // B
        }
    }

    return inputTensorValues;
}

QVector<DetectionBox> YoloDetector::detect(const QImage &frame)
{
    QVector<DetectionBox> results;
    if (!m_loaded || frame.isNull()) return results;

    try {
        int inputW, inputH;
        std::vector<float> inputTensorValues = preprocess(frame, inputW, inputH);
        
        // Create input tensor
        Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        std::vector<int64_t> inputShape = {1, 3, inputH, inputW};
        
        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo, inputTensorValues.data(), inputTensorValues.size(), inputShape.data(), inputShape.size());

        // Run inference
        auto outputTensors = m_session->Run(
            Ort::RunOptions{nullptr}, 
            m_inputNodeNames.data(), 
            &inputTensor, 
            1, 
            m_outputNodeNames.data(), 
            m_outputNodeNames.size()
        );

        // Get output data
        float* floatArr = outputTensors[0].GetTensorMutableData<float>();
        auto outputInfo = outputTensors[0].GetTensorTypeAndShapeInfo();
        auto outputShape = outputInfo.GetShape(); // [1, channels, anchors]

        // Important: Manual memory release logic for raw float vector is not needed as vector handles it.
        // But ONNX Runtime internal tensors are RAII.
        
        int batchSize = outputShape[0];
        int channels = outputShape[1]; // 4 + num_classes
        int anchors = outputShape[2];  // 8400

        int numClasses = channels - 4;
        
        QVector<DetectionBox> candidates;

        for (int i = 0; i < anchors; ++i) {
            // Find best class confidence
            float maxConf = 0.0f;
            int maxClassIdx = -1;

            for (int c = 0; c < numClasses; ++c) {
                float conf = floatArr[(4 + c) * anchors + i];
                if (conf > maxConf) {
                    maxConf = conf;
                    maxClassIdx = c;
                }
            }

            if (maxConf > m_options.confidenceThreshold) {
                float cx = floatArr[0 * anchors + i];
                float cy = floatArr[1 * anchors + i];
                float w = floatArr[2 * anchors + i];
                float h = floatArr[3 * anchors + i];

                float x = cx - w / 2.0f;
                float y = cy - h / 2.0f;

                DetectionBox box;
                box.confidence = maxConf;
                // Normalize coordinates to 0..1
                box.bounds = QRectF(x / inputW, y / inputH, w / inputW, h / inputH);
                
                if (maxClassIdx < m_options.classLabels.size()) {
                    box.label = m_options.classLabels[maxClassIdx];
                } else {
                    box.label = QString("Class %1").arg(maxClassIdx);
                }
                
                if (maxClassIdx < m_options.colorPalette.size()) {
                    box.color = QColor(m_options.colorPalette[maxClassIdx]);
                } else {
                    box.color = Qt::red;
                }
                
                candidates.append(box);
            }
        }

        applyNMS(candidates);
        results = candidates;

    } catch (const std::exception& e) {
        qWarning() << "Inference error:" << e.what();
    }

    return results;
}

void YoloDetector::applyNMS(QVector<DetectionBox> &detections)
{
    if (detections.isEmpty()) return;

    // Sort by confidence descending
    std::sort(detections.begin(), detections.end(), [](const DetectionBox &a, const DetectionBox &b) {
        return a.confidence > b.confidence;
    });

    QVector<DetectionBox> result;
    std::vector<bool> suppressed(detections.size(), false);

    for (int i = 0; i < detections.size(); ++i) {
        if (suppressed[i]) continue;

        result.append(detections[i]);

        for (int j = i + 1; j < detections.size(); ++j) {
            if (suppressed[j]) continue;

            // Calculate IoU
            QRectF rect1 = detections[i].bounds;
            QRectF rect2 = detections[j].bounds;
            
            QRectF intersect = rect1.intersected(rect2);
            float intersectArea = intersect.width() * intersect.height();
            
            if (intersect.isEmpty()) intersectArea = 0.0f;

            float unionArea = (rect1.width() * rect1.height()) + 
                              (rect2.width() * rect2.height()) - intersectArea;

            float iou = intersectArea / unionArea;

            // Also suppress if one box is largely contained within another (e.g. > 80% overlap)
            float rect1Area = rect1.width() * rect1.height();
            float rect2Area = rect2.width() * rect2.height();
            float containment1 = intersectArea / rect1Area;
            float containment2 = intersectArea / rect2Area;

            if (iou > m_options.nmsThreshold || containment1 > 0.8f || containment2 > 0.8f) {
                suppressed[j] = true;
            }
        }
    }

    detections = result;
}
