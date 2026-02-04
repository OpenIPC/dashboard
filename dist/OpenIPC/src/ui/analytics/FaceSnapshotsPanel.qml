import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Item {
    id: root
    property var model // AnalyticsEngine
    property string snapshotsDirOverride: ""
    property string clipsDirOverride: ""
    
    // Get the snapshots directory from the engine
    property string snapshotsDir: {
        if (root.snapshotsDirOverride && root.snapshotsDirOverride !== "") return root.snapshotsDirOverride
        if (root.model) {
            var config = root.model.getModuleConfig(0); // 0 = FaceDetector
            if (config && config.snapshotsDir) {
                return config.snapshotsDir;
            }
        }
        return "";
    }

    SnapshotBrowser {
        anchors.fill: parent
        snapshotsDir: root.snapshotsDir
        clipsDir: root.clipsDirOverride
        moduleBadgeText: I18n.t("Лица")
    }
}
