import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Item {
    id: root
    property var model // AnalyticsEngine
    property string snapshotsDirOverride: ""
    property string clipsDirOverride: ""
    SnapshotBrowser {
        anchors.fill: parent
        snapshotsDir: root.snapshotsDirOverride
        clipsDir: root.clipsDirOverride
        moduleBadgeText: I18n.t("Объекты")
    }
}
