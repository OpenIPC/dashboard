
bool SystemController::exportConfiguration(const QString &path)
{
    if (path.isEmpty()) return false;
    
    // Fix path if it is a file URL
    QString targetPath = path;
    if (targetPath.startsWith("file:///")) {
        targetPath = targetPath.mid(8);
    } 
    else if (targetPath.startsWith("file://")) {
        targetPath = targetPath.mid(7);
    }

    QJsonObject root;
    QJsonArray cameras;
    for (int i = 0; i < m_cameraModel->rowCount(); ++i) {
        cameras.append(cameraToJson(m_cameraModel->getCamera(i)));
    }
    QJsonArray groups;
    for (const auto &g : m_cameraGroups) {
        groups.append(g);
    }
    QJsonArray grid;
    for (int i = 0; i < m_gridModel->rowCount(); ++i) {
        QJsonObject slot;
        const Camera cam = m_gridModel->getCamera(i);
        slot["ip"] = cam.ip;
        slot["camera"] = cameraToJson(cam);
        slot["spanRows"] = cam.spanRows;
        slot["spanCols"] = cam.spanCols;
        grid.append(slot);
    }
    root["cameras"] = cameras;
    root["grid"] = grid;
    root["analytics"] = QJsonObject::fromVariantMap(m_analyticsEngine->getSettings());
    root["appSettings"] = QJsonObject::fromVariantMap(m_appSettings);
    root["cameraGroups"] = groups;
    root["layoutTemplates"] = QJsonArray::fromVariantList(m_layoutTemplates);
    
    QFile f(targetPath);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        f.flush();
        f.close();
        qInfo() << "Configuration exported to" << targetPath;
        return true;
    }
    
    qWarning() << "Failed to export configuration to" << targetPath;
    return false;
}

bool SystemController::importConfiguration(const QString &path)
{
    if (path.isEmpty()) return false;
    
    QString targetPath = path;
    if (targetPath.startsWith("file:///")) {
        targetPath = targetPath.mid(8);
    }
    else if (targetPath.startsWith("file://")) {
        targetPath = targetPath.mid(7);
    }
    
    QFile f(targetPath);
    if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open config file for import:" << targetPath;
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &error);
    f.close();
    
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Invalid JSON in config import:" << error.errorString();
        return false;
    }
    
    QJsonObject root = doc.object();
    
    // 1. App Settings
    if (root.contains("appSettings")) {
        QVariantMap savedSettings = root.value("appSettings").toObject().toVariantMap();
        for (auto it = savedSettings.begin(); it != savedSettings.end(); ++it) {
            m_appSettings[it.key()] = it.value();
        }

        m_gridRows = m_appSettings.value("gridRows", 2).toInt();
        m_gridCols = m_appSettings.value("gridCols", 2).toInt();
        emit appSettingsChanged();
    }

    // 2. Analytics
    if (root.contains("analytics")) {
        m_analyticsEngine->setSettings(root.value("analytics").toObject().toVariantMap());
    }

    // 3. Layout Templates
    if (root.contains("layoutTemplates")) {
        m_layoutTemplates = root.value("layoutTemplates").toArray().toVariantList();
        emit layoutTemplatesChanged();
    }
    
    // 4. Camera Groups
    // Clear and rebuild
    m_cameraGroups.clear();
    if (root.contains("cameraGroups")) {
        const QJsonArray groups = root.value("cameraGroups").toArray();
        for (const auto &g : groups) {
             const QString name = g.toString();
             if (!name.isEmpty()) m_cameraGroups.append(name);
        }
    }
    emit cameraGroupsChanged();

    // 5. Cameras
    m_cameraModel->clear();
    const QJsonArray cameras = root.value("cameras").toArray();
    for (const auto &v : cameras) {
        Camera cam = cameraFromJson(v.toObject());
        m_cameraModel->addCamera(cam);
        
        // Ensure group exists in list if referenced
        if (!cam.group.isEmpty() && !m_cameraGroups.contains(cam.group, Qt::CaseInsensitive)) {
            m_cameraGroups.append(cam.group);
            emit cameraGroupsChanged();
        }
    }
    
    // 6. Grid
    m_gridModel->clear();
    QJsonArray grid = root.value("grid").toArray();
    
    if (grid.isEmpty()) {
        // Fallback or empty
    } else {
        for (int i = 0; i < grid.size(); ++i) {
            Camera cam;
            QJsonObject slotObj = grid.at(i).toObject();
            const QString ip = slotObj.value("ip").toString();
            
            // Try to link with known camera details
            if (!ip.isEmpty()) {
                cam = m_cameraModel->findByIp(ip);
                if (cam.ip.isEmpty()) {
                    // Not found in cameras list, use embedded data
                    cam = cameraFromJson(slotObj.value("camera").toObject());
                }
            }
            
            cam.spanRows = slotObj.value("spanRows").toInt(1);
            cam.spanCols = slotObj.value("spanCols").toInt(1);
            
            m_gridModel->addCamera(cam);
        }
    }
    
    emit gridLayoutChanged();
    
    // Persist immediately
    saveState();
    
    qInfo() << "Configuration imported successfully from" << targetPath;
    return true;
}
