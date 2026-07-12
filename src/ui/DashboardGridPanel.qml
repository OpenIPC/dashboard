pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: gridPanel

    property int gridRows: 1
    property int gridCols: 1
    property int activeGridIndex: -1
    property bool canLive: false
    property bool canPlayback: false
    property bool canPtz: false
    property bool canExport: false
    property bool canSettings: false
    property bool canAnalytics: false
    property bool emptyHintDismissed: false
    property real sidebarOpenProgress: 1.0
    property var previewBudgetRankProvider: null
    property var systemController: null

    signal selectedByUser(int index)
    signal permissionDenied()
    signal editCameraRequested(int cameraIndex)
    signal deleteCameraRequested(int cameraIndex)
    signal archiveRequested(string cameraIp)
    signal majesticRequested(string cameraName, string cameraIp, int cameraPort, string cameraLogin)
    signal searchRequested()
    signal addCameraRequested()
    signal analyticsRequested()
    signal settingsRequested()
    signal sidebarOpenRequested()
    signal emptyHintClosed(bool dontShowAgain)

    color: Theme.panelBackground
    radius: Theme.radiusLg
    border.color: Theme.panelBorder
    border.width: 1

    function previewBudgetRank(index) {
        if (!previewBudgetRankProvider)
            return -1
        return previewBudgetRankProvider(index)
    }

    GridLayout {
        id: cameraGrid
        anchors.fill: parent
        anchors.margins: cameraGrid.cellSpacing
        property int cellSpacing: 10
        columns: 1200
        rowSpacing: cameraGrid.cellSpacing
        columnSpacing: cameraGrid.cellSpacing
        flow: GridLayout.LeftToRight

        property real unitWidth: (width - (Math.max(1, gridPanel.gridCols) - 1) * columnSpacing) / 1200
        property real unitHeight: (height - (Math.max(1, gridPanel.gridRows) - 1) * rowSpacing) / 1200

        Repeater {
            model: gridPanel.systemController ? gridPanel.systemController.gridModel : null

            delegate: Item {
                id: gridCellDelegate

                required property int index
                required property string cameraIp
                required property string cameraLogin
                required property string cameraName
                required property int cameraOnvifPort
                required property int cameraPort
                required property string hdStreamUrl
                required property string manufacturer
                required property string sdStreamUrl
                required property int spanCols
                required property int spanRows
                required property string status
                required property string streamUrl

                Layout.preferredWidth: Math.floor(Math.max(1, gridCellDelegate.spanCols) * cameraGrid.unitWidth)
                Layout.preferredHeight: Math.floor(Math.max(1, gridCellDelegate.spanRows) * cameraGrid.unitHeight)
                Layout.rowSpan: Math.max(1, Math.round((gridCellDelegate.spanRows || 1) / (1200 / Math.max(1, gridPanel.gridRows || 2))))
                Layout.columnSpan: Math.max(1, gridCellDelegate.spanCols || 1)

                GridCell {
                    anchors.fill: parent

                    gridParent: cameraGrid
                    totalRows: 1200
                    totalCols: 1200
                    logicalRows: Math.max(1, gridPanel.gridRows || 1)
                    logicalCols: Math.max(1, gridPanel.gridCols || 1)
                    spanRows: gridCellDelegate.spanRows || 1
                    spanCols: gridCellDelegate.spanCols || 1
                    gridIndex: gridCellDelegate.index
                    previewBudgetRank: gridPanel.previewBudgetRank(gridCellDelegate.index)
                    isSelected: gridPanel.activeGridIndex === gridCellDelegate.index

                    unitWidth: cameraGrid.unitWidth
                    unitHeight: cameraGrid.unitHeight

                    cameraName: gridCellDelegate.cameraName
                    cameraIp: gridCellDelegate.cameraIp
                    cameraPort: gridCellDelegate.cameraPort
                    cameraOnvifPort: gridCellDelegate.cameraOnvifPort
                    cameraLogin: gridCellDelegate.cameraLogin
                    streamUrl: gridCellDelegate.streamUrl
                    sdStreamUrl: gridCellDelegate.sdStreamUrl || gridCellDelegate.streamUrl
                    hdStreamUrl: gridCellDelegate.hdStreamUrl || gridCellDelegate.streamUrl
                    status: gridCellDelegate.status
                    manufacturer: gridCellDelegate.manufacturer || ""

                    canLive: gridPanel.canLive
                    canPlayback: gridPanel.canPlayback
                    canPtz: gridPanel.canPtz
                    canExport: gridPanel.canExport
                    canSettings: gridPanel.canSettings

                    onSelectedByUser: gridPanel.selectedByUser(gridCellDelegate.index)
                    onPermissionDenied: gridPanel.permissionDenied()

                    onCloseClicked: {
                        if (!gridPanel.canSettings) {
                            gridPanel.permissionDenied()
                            return
                        }
                        gridPanel.systemController.removeCameraFromGrid(gridCellDelegate.index)
                    }

                    onEditRequested: {
                        if (!gridPanel.canSettings) {
                            gridPanel.permissionDenied()
                            return
                        }
                        var editIndex = gridPanel.systemController.cameraModel.findIndexByIp(gridCellDelegate.cameraIp)
                        if (editIndex >= 0)
                            gridPanel.editCameraRequested(editIndex)
                    }

                    onDeleteRequested: {
                        if (!gridPanel.canSettings) {
                            gridPanel.permissionDenied()
                            return
                        }
                        var deleteIndex = gridPanel.systemController.cameraModel.findIndexByIp(gridCellDelegate.cameraIp)
                        if (deleteIndex >= 0)
                            gridPanel.deleteCameraRequested(deleteIndex)
                    }

                    onArchiveRequested: {
                        if (!gridPanel.canPlayback) {
                            gridPanel.permissionDenied()
                            return
                        }
                        gridPanel.archiveRequested(gridCellDelegate.cameraIp)
                    }

                    onMajesticRequested: {
                        if (!gridPanel.canSettings) {
                            gridPanel.permissionDenied()
                            return
                        }
                        gridPanel.majesticRequested(gridCellDelegate.cameraName || gridCellDelegate.cameraIp,
                                                    gridCellDelegate.cameraIp,
                                                    gridCellDelegate.cameraOnvifPort || 80,
                                                    gridCellDelegate.cameraLogin || "root")
                    }
                }
            }
        }
    }

    DashboardEmptyHint {
        anchors.fill: parent
        anchors.margins: 24
        visible: gridPanel.systemController
                 && gridPanel.systemController.cameraModel.rowCount() === 0
                 && !gridPanel.emptyHintDismissed
        canSettings: gridPanel.canSettings
        canAnalytics: gridPanel.canAnalytics
        onCloseRequested: (dontShowAgain) => gridPanel.emptyHintClosed(dontShowAgain)
        onSearchRequested: gridPanel.searchRequested()
        onAddRequested: gridPanel.addCameraRequested()
        onAnalyticsRequested: gridPanel.analyticsRequested()
        onSettingsRequested: gridPanel.settingsRequested()
    }

    Button {
        id: revealSidebarButton

        visible: gridPanel.sidebarOpenProgress < 0.01
        width: 18
        height: 84
        anchors.right: parent.right
        anchors.rightMargin: -9
        anchors.verticalCenter: parent.verticalCenter
        padding: 0
        hoverEnabled: true
        focusPolicy: Qt.StrongFocus
        z: 5

        background: Rectangle {
            radius: 9
            color: revealSidebarButton.hovered || revealSidebarButton.visualFocus
                   ? Theme.cardHover
                   : Theme.cardBackground
            border.color: revealSidebarButton.visualFocus ? Theme.accent : Theme.controlBorderStrong
            border.width: revealSidebarButton.visualFocus ? 2 : 1
        }

        contentItem: Text {
            text: "«"
            color: Theme.textSecondary
            font.pixelSize: 16
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        ToolTip.visible: hovered || visualFocus
        ToolTip.text: I18n.t("Показать боковую панель")
        ToolTip.delay: 450
        onClicked: gridPanel.sidebarOpenRequested()
    }

}
