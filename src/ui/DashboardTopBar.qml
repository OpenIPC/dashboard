import QtQuick
import QtQuick.Window
import OpenIPC

Rectangle {
    id: topBar

    property var layoutsModel: null
    property int currentLayoutIndex: -1

    signal applyLayoutRequested(int index)
    signal closeLayoutRequested(int index)
    signal addLayoutRequested()
    signal editLayoutRequested()

    color: Theme.metroSidebarBackground
    clip: true

    MouseArea {
        anchors.fill: parent
        onPressed: {
            if (Window.window) Window.window.startSystemMove()
        }
        onDoubleClicked: {
            if (!Window.window) return
            if (Window.window.visibility === Window.Maximized)
                Window.window.showNormal()
            else
                Window.window.showMaximized()
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: Theme.metroStroke
    }

    DashboardLayoutToolbar {
        anchors.left: parent.left
        anchors.right: windowControls.left
        anchors.bottom: parent.bottom
        anchors.margins: 12
        anchors.rightMargin: 12
        anchors.bottomMargin: 8
        layoutsModel: topBar.layoutsModel
        currentLayoutIndex: topBar.currentLayoutIndex
        onApplyRequested: (index) => topBar.applyLayoutRequested(index)
        onCloseRequested: (index) => topBar.closeLayoutRequested(index)
        onAddRequested: topBar.addLayoutRequested()
        onEditRequested: topBar.editLayoutRequested()
    }

    DashboardWindowControls {
        id: windowControls

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 6
        anchors.rightMargin: 10
    }
}
