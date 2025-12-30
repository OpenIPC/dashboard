#pragma once

#include <QString>
#include <QVector>
#include <QRectF>
#include <QColor>
#include <QImage>
#include <memory>

struct DetectionBox {
    QString id;
    QString label;
    float confidence;
    QRectF bounds; // Normalized 0..1
    QColor color;
    QString trackId;
};

class InferenceBackend {
public:
    virtual ~InferenceBackend() = default;
    
    virtual bool load(const QString &modelPath) = 0;
    virtual bool isLoaded() const = 0;
    virtual QVector<DetectionBox> detect(const QImage &frame) = 0;
    virtual QString getError() const = 0;
};
