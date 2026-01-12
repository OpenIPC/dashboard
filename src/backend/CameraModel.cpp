#include "CameraModel.h"

CameraModel::CameraModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int CameraModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_cameras.count();
}

QVariant CameraModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_cameras.count())
        return QVariant();

    const Camera &camera = m_cameras[index.row()];

    switch (role) {
    case IdRole:
        return camera.id;
    case NameRole:
        return camera.name;
    case IpRole:
        return camera.ip;
    case StreamUrlRole:
        return camera.streamUrl;
    case SdStreamUrlRole:
        return camera.sdStreamUrl;
    case HdStreamUrlRole:
        return camera.hdStreamUrl;
    case StatusRole:
        return camera.status;
    case PortRole:
        return camera.port;
    case OnvifPortRole:
        return camera.onvifPort;
    case LoginRole:
        return camera.login;
    case PasswordRole:
        return camera.password;
    case GroupRole:
        return camera.group;
    case IsRecordingRole:
        return camera.isRecording;
    case SpanRowsRole:
        return camera.spanRows;
    case SpanColsRole:
        return camera.spanCols;
    case SerialNumberRole:
        return camera.serialNumber;
    case ManufacturerRole:
        return camera.manufacturer;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> CameraModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "cameraId";
    roles[NameRole] = "cameraName";
    roles[IpRole] = "cameraIp";
    roles[StreamUrlRole] = "streamUrl";
    roles[SdStreamUrlRole] = "sdStreamUrl";
    roles[HdStreamUrlRole] = "hdStreamUrl";
    roles[StatusRole] = "status";
    roles[PortRole] = "cameraPort";
    roles[OnvifPortRole] = "cameraOnvifPort";
    roles[LoginRole] = "cameraLogin";
    roles[PasswordRole] = "cameraPassword";
    roles[GroupRole] = "cameraGroup";
    roles[IsRecordingRole] = "isRecording";
    roles[SpanRowsRole] = "spanRows";
    roles[SpanColsRole] = "spanCols";
    roles[SerialNumberRole] = "serialNumber";
    roles[ManufacturerRole] = "manufacturer";
    return roles;
}

void CameraModel::addCamera(const Camera &camera)
{
    beginInsertRows(QModelIndex(), m_cameras.count(), m_cameras.count());
    m_cameras.append(camera);
    endInsertRows();
}

void CameraModel::removeCamera(int index)
{
    if (index < 0 || index >= m_cameras.count())
        return;
        
    beginRemoveRows(QModelIndex(), index, index);
    m_cameras.removeAt(index);
    endRemoveRows();
}

void CameraModel::moveCamera(int from, int to)
{
    if (from < 0 || from >= m_cameras.count() || to < 0 || to >= m_cameras.count() || from == to)
        return;

    if (beginMoveRows(QModelIndex(), from, from, QModelIndex(), to > from ? to + 1 : to)) {
        m_cameras.move(from, to);
        endMoveRows();
    }
}

void CameraModel::swapCameras(int index1, int index2)
{
    if (index1 < 0 || index1 >= m_cameras.count() || index2 < 0 || index2 >= m_cameras.count() || index1 == index2)
        return;

    // Swap content but preserve spans (layout geometry belongs to the slot, not the camera)
    int spanRows1 = m_cameras[index1].spanRows;
    int spanCols1 = m_cameras[index1].spanCols;
    int spanRows2 = m_cameras[index2].spanRows;
    int spanCols2 = m_cameras[index2].spanCols;

    std::swap(m_cameras[index1], m_cameras[index2]);
    
    // Restore spans to their original indices
    m_cameras[index1].spanRows = spanRows1;
    m_cameras[index1].spanCols = spanCols1;
    m_cameras[index2].spanRows = spanRows2;
    m_cameras[index2].spanCols = spanCols2;
    
    QModelIndex idx1 = index(index1);
    QModelIndex idx2 = index(index2);
    emit dataChanged(idx1, idx1);
    emit dataChanged(idx2, idx2);
}

void CameraModel::setCamera(int index, const Camera &camera)
{
    if (index < 0)
        return;

    if (index < m_cameras.count()) {
        m_cameras[index] = camera;
        const QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex);
    } else {
        // If index is beyond current size, append
        addCamera(camera);
    }
}

void CameraModel::setStatus(int index, const QString &status)
{
    if (index < 0 || index >= m_cameras.count())
        return;

    if (m_cameras[index].status != status) {
        m_cameras[index].status = status;
        const QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex, {StatusRole});
    }
}

void CameraModel::setSpan(int index, int rows, int cols)
{
    if (index < 0 || index >= m_cameras.count())
        return;

    m_cameras[index].spanRows = rows;
    m_cameras[index].spanCols = cols;
    const QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {SpanRowsRole, SpanColsRole});
}

void CameraModel::clear()
{
    beginResetModel();
    m_cameras.clear();
    endResetModel();
}

Camera CameraModel::getCamera(int index) const
{
    if (index < 0 || index >= m_cameras.count()) {
        return Camera();
    }
    return m_cameras[index];
}

Camera CameraModel::findByIp(const QString &ip) const
{
    for (const auto &cam : m_cameras) {
        if (cam.ip == ip) {
            return cam;
        }
    }
    return Camera();
}

int CameraModel::findIndexByIp(const QString &ip) const
{
    for (int i = 0; i < m_cameras.count(); ++i) {
        if (m_cameras[i].ip == ip) {
            return i;
        }
    }
    return -1;
}

bool CameraModel::contains(const QString &ip)
{
    for (const auto &cam : m_cameras) {
        if (cam.ip == ip) return true;
    }
    return false;
}
