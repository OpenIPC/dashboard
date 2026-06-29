#ifndef CAMERAMODEL_H
#define CAMERAMODEL_H

#include <QAbstractListModel>
#include <QVector>

struct Camera {
    Q_GADGET
    Q_PROPERTY(QString cameraName MEMBER name)
    Q_PROPERTY(QString cameraIp MEMBER ip)
    Q_PROPERTY(int cameraPort MEMBER port)
    Q_PROPERTY(int cameraOnvifPort MEMBER onvifPort)
    Q_PROPERTY(QString cameraLogin MEMBER login)
    Q_PROPERTY(QString hdStreamUrl MEMBER hdStreamUrl)
    Q_PROPERTY(QString sdStreamUrl MEMBER sdStreamUrl)
    Q_PROPERTY(QString status MEMBER status)
    Q_PROPERTY(QString cameraGroup MEMBER group)
    Q_PROPERTY(bool isRecording MEMBER isRecording)
    Q_PROPERTY(QString discoveryMethods MEMBER discoveryMethods)
    Q_PROPERTY(QString discoveryEvidence MEMBER discoveryEvidence)
    Q_PROPERTY(int discoveryConfidence MEMBER discoveryConfidence)
    Q_PROPERTY(bool isOpenIpc MEMBER isOpenIpc)

public:
    QString id;
    QString name;
    QString ip;
    QString streamUrl;   // legacy main url
    QString sdStreamUrl; // low bitrate preview
    QString hdStreamUrl; // full-res stream
    QString status; // "Online", "Offline", "Auth Required"
    int port = 554; // default RTSP port
    int onvifPort = 80;
    QString login;
    QString password;
    QString group; // optional logical group name
    bool isRecording = false;
    int spanRows = 1;
    int spanCols = 1;
    QString serialNumber;
    QString manufacturer;
    QString discoveryMethods;
    QString discoveryEvidence;
    int discoveryConfidence = 0;
    bool isOpenIpc = false;
};

class CameraModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum CameraRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        IpRole,
        StreamUrlRole,
        SdStreamUrlRole,
        HdStreamUrlRole,
        StatusRole,
        PortRole,
        OnvifPortRole,
        LoginRole,
        GroupRole,
        IsRecordingRole,
        SpanRowsRole,
        SpanColsRole,
        SerialNumberRole,
        ManufacturerRole,
        DiscoveryMethodsRole,
        DiscoveryEvidenceRole,
        DiscoveryConfidenceRole,
        IsOpenIpcRole
    };

    explicit CameraModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addCamera(const Camera &camera);
    void removeCamera(int index);
    Q_INVOKABLE void moveCamera(int from, int to);
    Q_INVOKABLE void swapCameras(int index1, int index2);
    void setCamera(int index, const Camera &camera);
    Q_INVOKABLE void setStatus(int index, const QString &status);
    void setSpan(int index, int rows, int cols);
    void clear();
    Q_INVOKABLE Camera getCamera(int index) const;
    Camera findByIp(const QString &ip) const;
    Q_INVOKABLE int findIndexByIp(const QString &ip) const;
    
    // Helper to check if camera already exists
    bool contains(const QString &ip);

private:
    QVector<Camera> m_cameras;
};

#endif // CAMERAMODEL_H
