#include "AnalyticsModel.h"
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QDir>

AnalyticsModel::AnalyticsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int AnalyticsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_snapshots.size());
}

QVariant AnalyticsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_snapshots.size())
        return QVariant();

    const auto &snapshot = m_snapshots[index.row()];

    switch (role) {
    case IdRole:
        return snapshot.id;
    case ModuleIdRole:
        return snapshot.moduleId;
    case CameraIdRole:
        return snapshot.cameraId;
    case CapturedAtRole:
        return snapshot.capturedAt;
    case ConfidenceRole:
        return snapshot.confidence;
    case ImagePathRole:
        return snapshot.imagePath;
    case LabelRole:
        return snapshot.label;
    }

    return QVariant();
}

QHash<int, QByteArray> AnalyticsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[ModuleIdRole] = "moduleId";
    roles[CameraIdRole] = "cameraId";
    roles[CapturedAtRole] = "capturedAt";
    roles[ConfidenceRole] = "confidence";
    roles[ImagePathRole] = "imagePath";
    roles[LabelRole] = "label";
    return roles;
}

void AnalyticsModel::addSnapshot(const QString &moduleId, const QString &cameraId, const QString &imagePath, double confidence, const QString &label)
{
    AnalyticsSnapshot snapshot;
    snapshot.id = QString::number(QDateTime::currentMSecsSinceEpoch()) + "_" + QString::number(m_allSnapshots.size());
    snapshot.moduleId = moduleId;
    snapshot.cameraId = cameraId;
    snapshot.capturedAt = QDateTime::currentDateTime();
    snapshot.confidence = confidence;
    snapshot.imagePath = imagePath;
    snapshot.label = label;
    
    m_allSnapshots.push_back(snapshot);
    applyFilter();
}

void AnalyticsModel::clear()
{
    beginResetModel();
    m_snapshots.clear();
    m_allSnapshots.clear();
    endResetModel();
}

void AnalyticsModel::generateMockData()
{
    clear();
    
    // Generate some fake data
    for (int i = 0; i < 50; ++i) {
        QString moduleId = "face";
        if (i % 3 == 0) moduleId = "object";
        if (i % 3 == 1) moduleId = "plate";
        
        QString label;
        if (moduleId == "object") label = "person";
        if (moduleId == "plate") label = QString("ABC-%1").arg(i * 123);
        
        // Use a placeholder image or a real one if available
        QString imagePath = ""; 
        
        // Directly add to allSnapshots to avoid multiple applyFilter calls
        AnalyticsSnapshot snapshot;
        snapshot.id = QString::number(QDateTime::currentMSecsSinceEpoch()) + "_" + QString::number(i);
        snapshot.moduleId = moduleId;
        snapshot.cameraId = "Camera 1";
        snapshot.capturedAt = QDateTime::currentDateTime().addSecs(-i * 60);
        snapshot.confidence = 0.85 + (i * 0.001);
        snapshot.imagePath = imagePath;
        snapshot.label = label;
        m_allSnapshots.push_back(snapshot);
    }
    applyFilter();
}

void AnalyticsModel::setFilterType(const QString &type)
{
    if (m_filterType == type)
        return;
    m_filterType = type;
    emit filterTypeChanged();
    applyFilter();
}

void AnalyticsModel::applyFilter()
{
    beginResetModel();
    m_snapshots.clear();
    if (m_filterType.isEmpty()) {
        m_snapshots = m_allSnapshots;
    } else {
        for (const auto &s : m_allSnapshots) {
            if (s.moduleId == m_filterType) {
                m_snapshots.push_back(s);
            }
        }
    }
    endResetModel();
}
