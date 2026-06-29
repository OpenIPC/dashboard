#include "YoloDetector.h"
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QPainter>
#include <QThread>
#include <algorithm>
#include <cmath>
#include <cstring>

#define ORT_API_MANUAL_INIT
#include <onnxruntime_cxx_api.h>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

struct YoloDetector::Impl {
    std::unique_ptr<Ort::Env> env;
    std::unique_ptr<Ort::Session> session;
    std::unique_ptr<Ort::SessionOptions> sessionOptions;
    std::vector<const char*> inputNodeNames;
    std::vector<const char*> outputNodeNames;
    std::vector<std::string> inputNodeNamesStorage;
    std::vector<std::string> outputNodeNamesStorage;
    std::vector<int64_t> inputShape;
};

namespace {
void ensureOrtApiInitialized()
{
    static const bool initialized = []() {
        Ort::InitApi();
        return true;
    }();

    (void)initialized;
}
}

YoloDetector::YoloDetector(const Options &options)
    : m_options(options)
    , m_impl(std::make_unique<Impl>())
{
}

YoloDetector::~YoloDetector() = default;

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
        ensureOrtApiInitialized();

        m_impl->env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "OpenIPCDashboard");
        m_impl->sessionOptions = std::make_unique<Ort::SessionOptions>();
        m_impl->sessionOptions->SetIntraOpNumThreads(1);
        m_impl->sessionOptions->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

#ifdef Q_OS_WIN
        {
            using OrtSessionOptionsAppendExecutionProvider_DML_t = OrtStatus* (ORT_API_CALL *)(OrtSessionOptions*, int);
            HMODULE ortModule = GetModuleHandleW(L"onnxruntime.dll");
            if (!ortModule) {
                ortModule = LoadLibraryW(L"onnxruntime.dll");
            }
            OrtSessionOptionsAppendExecutionProvider_DML_t appendDml = nullptr;
            if (ortModule) {
                const FARPROC symbol = GetProcAddress(
                    ortModule, "OrtSessionOptionsAppendExecutionProvider_DML");
                static_assert(sizeof(appendDml) == sizeof(symbol));
                std::memcpy(&appendDml, &symbol, sizeof(appendDml));
            }

            if (appendDml) {
                Ort::UnownedSessionOptions unowned = m_impl->sessionOptions->GetUnowned();
                OrtSessionOptions* rawOptions = unowned;
                OrtStatus* status = appendDml(rawOptions, 0);
                if (status) {
                    const char* msg = Ort::GetApi().GetErrorMessage(status);
                    qWarning() << "DirectML EP not available, falling back to CPU:" << msg;
                    Ort::GetApi().ReleaseStatus(status);
                } else {
                    qInfo() << "DirectML EP enabled";
                }
            } else {
                qWarning() << "DirectML EP symbol not found, falling back to CPU";
            }
        }
#endif

        // Convert path to wstring for Windows
#ifdef Q_OS_WIN
        std::wstring modelPathW = modelPath.toStdWString();
        m_impl->session = std::make_unique<Ort::Session>(*m_impl->env, modelPathW.c_str(), *m_impl->sessionOptions);
#else
        std::string modelPathStr = modelPath.toStdString();
        m_impl->session = std::make_unique<Ort::Session>(*m_impl->env, modelPathStr.c_str(), *m_impl->sessionOptions);
#endif

        // Get input info
        Ort::AllocatorWithDefaultOptions allocator;
        size_t numInputNodes = m_impl->session->GetInputCount();
        m_impl->inputNodeNames.clear();
        m_impl->inputNodeNamesStorage.clear();

        for (size_t i = 0; i < numInputNodes; i++) {
            auto inputName = m_impl->session->GetInputNameAllocated(i, allocator);
            m_impl->inputNodeNamesStorage.emplace_back(inputName.get());
            m_impl->inputNodeNames.push_back(m_impl->inputNodeNamesStorage.back().c_str());
            
            auto typeInfo = m_impl->session->GetInputTypeInfo(i);
            auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
            m_impl->inputShape = tensorInfo.GetShape();
        }

        // Get output info
        size_t numOutputNodes = m_impl->session->GetOutputCount();
        m_impl->outputNodeNames.clear();
        m_impl->outputNodeNamesStorage.clear();

        for (size_t i = 0; i < numOutputNodes; i++) {
            auto outputName = m_impl->session->GetOutputNameAllocated(i, allocator);
            m_impl->outputNodeNamesStorage.emplace_back(outputName.get());
            m_impl->outputNodeNames.push_back(m_impl->outputNodeNamesStorage.back().c_str());
        }

        qInfo() << "YoloDetector: Loaded model from" << modelPath;
        if (m_impl->inputShape.size() >= 4) {
            qInfo() << "Input shape:" << m_impl->inputShape[0] << m_impl->inputShape[1] << m_impl->inputShape[2] << m_impl->inputShape[3];
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

std::vector<float> YoloDetector::preprocess(const QImage &img, PreprocessInfo &info)
{
    int targetW = 640;
    int targetH = 640;
    
    if (m_impl && m_impl->inputShape.size() == 4) {
        const int modelH = static_cast<int>(m_impl->inputShape[2]);
        const int modelW = static_cast<int>(m_impl->inputShape[3]);
        if (modelH > 0) {
            targetH = modelH;
        }
        if (modelW > 0) {
            targetW = modelW;
        }
    }

    info.inputW = targetW;
    info.inputH = targetH;
    info.sourceW = img.width();
    info.sourceH = img.height();

    const float scaleX = info.sourceW > 0 ? static_cast<float>(targetW) / static_cast<float>(info.sourceW) : 1.0f;
    const float scaleY = info.sourceH > 0 ? static_cast<float>(targetH) / static_cast<float>(info.sourceH) : 1.0f;
    info.scale = std::min(scaleX, scaleY);

    const int resizedW = std::max(1, static_cast<int>(std::round(info.sourceW * info.scale)));
    const int resizedH = std::max(1, static_cast<int>(std::round(info.sourceH * info.scale)));
    info.padX = (targetW - resizedW) / 2;
    info.padY = (targetH - resizedH) / 2;

    QImage canvas(targetW, targetH, QImage::Format_RGB888);
    canvas.fill(QColor(114, 114, 114));

    QImage scaled = img.scaled(resizedW, resizedH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
        .convertToFormat(QImage::Format_RGB888);
    QPainter painter(&canvas);
    painter.drawImage(info.padX, info.padY, scaled);
    painter.end();

    // Convert to float CHW
    std::vector<float> inputTensorValues(targetW * targetH * 3);
    
    const uchar* bits = canvas.bits();
    int stride = canvas.bytesPerLine();

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
    if (!m_loaded || frame.isNull() || !m_impl || !m_impl->session) return results;

    try {
        PreprocessInfo preprocessInfo;
        std::vector<float> inputTensorValues = preprocess(frame, preprocessInfo);
        
        // Create input tensor
        Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        std::vector<int64_t> inputShape = {1, 3, preprocessInfo.inputH, preprocessInfo.inputW};
        
        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo, inputTensorValues.data(), inputTensorValues.size(), inputShape.data(), inputShape.size());

        // Run inference
        auto outputTensors = m_impl->session->Run(
            Ort::RunOptions{nullptr}, 
            m_impl->inputNodeNames.data(), 
            &inputTensor, 
            1, 
            m_impl->outputNodeNames.data(), 
            m_impl->outputNodeNames.size()
        );

        // Get output data
        float* floatArr = outputTensors[0].GetTensorMutableData<float>();
        auto outputInfo = outputTensors[0].GetTensorTypeAndShapeInfo();
        auto outputShape = outputInfo.GetShape(); // commonly [1, channels, anchors] or [1, anchors, channels]

        // Important: Manual memory release logic for raw float vector is not needed as vector handles it.
        // But ONNX Runtime internal tensors are RAII.
        
        if (outputShape.size() < 3) {
            qWarning() << "Unexpected YOLO output shape rank:" << outputShape.size();
            return results;
        }

        const int dim1 = static_cast<int>(outputShape[1]);
        const int dim2 = static_cast<int>(outputShape[2]);
        const bool channelsFirst = dim1 <= dim2;
        const int channels = channelsFirst ? dim1 : dim2;
        const int anchors = channelsFirst ? dim2 : dim1;
        if (channels <= 4 || anchors <= 0) {
            qWarning() << "Unexpected YOLO output shape:" << dim1 << dim2;
            return results;
        }

        int numClasses = channels - 4;
        
        QVector<DetectionBox> candidates;
        auto outputValue = [floatArr, channelsFirst, anchors, channels](int channel, int anchor) {
            return channelsFirst
                ? floatArr[channel * anchors + anchor]
                : floatArr[anchor * channels + channel];
        };

        for (int i = 0; i < anchors; ++i) {
            // Find best class confidence
            float maxConf = 0.0f;
            int maxClassIdx = -1;

            for (int c = 0; c < numClasses; ++c) {
                float conf = outputValue(4 + c, i);
                if (conf > maxConf) {
                    maxConf = conf;
                    maxClassIdx = c;
                }
            }

            if (maxConf > m_options.confidenceThreshold) {
                float cx = outputValue(0, i);
                float cy = outputValue(1, i);
                float w = outputValue(2, i);
                float h = outputValue(3, i);

                if (cx <= 1.5f && cy <= 1.5f && w <= 1.5f && h <= 1.5f) {
                    cx *= preprocessInfo.inputW;
                    w *= preprocessInfo.inputW;
                    cy *= preprocessInfo.inputH;
                    h *= preprocessInfo.inputH;
                }

                float x1 = (cx - w / 2.0f - preprocessInfo.padX) / preprocessInfo.scale;
                float y1 = (cy - h / 2.0f - preprocessInfo.padY) / preprocessInfo.scale;
                float x2 = (cx + w / 2.0f - preprocessInfo.padX) / preprocessInfo.scale;
                float y2 = (cy + h / 2.0f - preprocessInfo.padY) / preprocessInfo.scale;

                x1 = std::clamp(x1, 0.0f, static_cast<float>(preprocessInfo.sourceW));
                y1 = std::clamp(y1, 0.0f, static_cast<float>(preprocessInfo.sourceH));
                x2 = std::clamp(x2, 0.0f, static_cast<float>(preprocessInfo.sourceW));
                y2 = std::clamp(y2, 0.0f, static_cast<float>(preprocessInfo.sourceH));

                const float boxW = x2 - x1;
                const float boxH = y2 - y1;
                if (boxW <= 1.0f || boxH <= 1.0f || preprocessInfo.sourceW <= 0 || preprocessInfo.sourceH <= 0) {
                    continue;
                }

                DetectionBox box;
                box.confidence = maxConf;
                box.bounds = QRectF(x1 / preprocessInfo.sourceW,
                                    y1 / preprocessInfo.sourceH,
                                    boxW / preprocessInfo.sourceW,
                                    boxH / preprocessInfo.sourceH);
                
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
            if (detections[i].label != detections[j].label) continue;

            // Calculate IoU
            QRectF rect1 = detections[i].bounds;
            QRectF rect2 = detections[j].bounds;
            
            QRectF intersect = rect1.intersected(rect2);
            float intersectArea = intersect.width() * intersect.height();
            
            if (intersect.isEmpty()) intersectArea = 0.0f;

            float unionArea = (rect1.width() * rect1.height()) + 
                              (rect2.width() * rect2.height()) - intersectArea;

            if (unionArea <= 0.0f) {
                continue;
            }

            float iou = intersectArea / unionArea;

            // Also suppress if one box is largely contained within another (e.g. > 80% overlap)
            float rect1Area = rect1.width() * rect1.height();
            float rect2Area = rect2.width() * rect2.height();
            float containment1 = rect1Area > 0.0f ? intersectArea / rect1Area : 0.0f;
            float containment2 = rect2Area > 0.0f ? intersectArea / rect2Area : 0.0f;

            if (iou > m_options.nmsThreshold || containment1 > 0.8f || containment2 > 0.8f) {
                suppressed[j] = true;
            }
        }
    }

    detections = result;
}
