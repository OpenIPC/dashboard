#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <vector>

struct AnalyticsSnapshot {
    QString id;
    QString moduleId; // "face", "object", "plate"
    QString cameraId;
    QDateTime capturedAt;
    double confidence;
    QString imagePath;
    QString label; // For objects/plates
};

class AnalyticsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString filterType READ filterType WRITE setFilterType NOTIFY filterTypeChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        ModuleIdRole,
        CameraIdRole,
        CapturedAtRole,
        ConfidenceRole,
        ImagePathRole,
        LabelRole
    };

    explicit AnalyticsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addSnapshot(const QString &moduleId, const QString &cameraId, const QString &imagePath, double confidence, const QString &label = QString());
    Q_INVOKABLE void clear();
    
    // Helper to generate mock data
    Q_INVOKABLE void generateMockData();

    Q_INVOKABLE QVariantMap getModuleConfig(const QString &moduleId);

    QString filterType() const { return m_filterType; }
    void setFilterType(const QString &type);

signals:
    void filterTypeChanged();

private:
    std::vector<AnalyticsSnapshot> m_snapshots;
    std::vector<AnalyticsSnapshot> m_allSnapshots;
    QString m_filterType;
    
    void applyFilter();
};
