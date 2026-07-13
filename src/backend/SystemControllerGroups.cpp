#include "SystemController.h"

void SystemController::addCameraGroup(const QString &groupName)
{
    const QString trimmed = groupName.trimmed();
    if (trimmed.isEmpty()) return;
    if (m_cameraGroups.contains(trimmed, Qt::CaseInsensitive)) return;
    m_cameraGroups.append(trimmed);
    emit cameraGroupsChanged();
    saveState();
}

void SystemController::setCameraGroup(int cameraIndex, const QString &groupName)
{
    if (cameraIndex < 0 || cameraIndex >= m_cameraModel->rowCount()) return;
    Camera cam = m_cameraModel->getCamera(cameraIndex);
    const QString trimmed = groupName.trimmed();
    if (cam.group == trimmed) return;
    cam.group = trimmed;
    m_cameraModel->setCamera(cameraIndex, cam);

    if (!trimmed.isEmpty() && !m_cameraGroups.contains(trimmed, Qt::CaseInsensitive)) {
        m_cameraGroups.append(trimmed);
        emit cameraGroupsChanged();
    }
    saveState();
}

void SystemController::removeCameraGroup(const QString &groupName)
{
    const QString trimmed = groupName.trimmed();
    if (trimmed.isEmpty()) return; // do not remove default empty group
    int idx = -1;
    for (int i = 0; i < m_cameraGroups.size(); ++i) {
        if (m_cameraGroups.at(i).compare(trimmed, Qt::CaseInsensitive) == 0) {
            idx = i; break;
        }
    }
    if (idx < 0) return;

    // Clear group on cameras
    for (int i = 0; i < m_cameraModel->rowCount(); ++i) {
        Camera cam = m_cameraModel->getCamera(i);
        if (cam.group.compare(trimmed, Qt::CaseInsensitive) == 0) {
            cam.group.clear();
            m_cameraModel->setCamera(i, cam);
        }
    }

    m_cameraGroups.removeAt(idx);
    emit cameraGroupsChanged();
    saveState();
}

void SystemController::renameCameraGroup(const QString &oldName, const QString &newName)
{
    const QString from = oldName.trimmed();
    const QString to = newName.trimmed();
    if (from.isEmpty() || to.isEmpty()) return; // default group cannot be renamed and empty target not allowed
    if (from.compare(to, Qt::CaseInsensitive) == 0) return;

    int idx = -1;
    for (int i = 0; i < m_cameraGroups.size(); ++i) {
        if (m_cameraGroups.at(i).compare(from, Qt::CaseInsensitive) == 0) {
            idx = i; break;
        }
    }
    if (idx < 0) return;

    // Update list, avoid duplicates
    if (m_cameraGroups.contains(to, Qt::CaseInsensitive)) {
        m_cameraGroups.removeAt(idx);
    } else {
        m_cameraGroups[idx] = to;
    }

    // Update cameras that belong to the old group
    for (int i = 0; i < m_cameraModel->rowCount(); ++i) {
        Camera cam = m_cameraModel->getCamera(i);
        if (cam.group.compare(from, Qt::CaseInsensitive) == 0) {
            cam.group = to;
            m_cameraModel->setCamera(i, cam);
        }
    }

    emit cameraGroupsChanged();
    saveState();
}
