import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

ApplicationWindow {
    id: window
    width: 1280
    height: 720
    visible: true
    visibility: Window.Maximized
    flags: Qt.Window | Qt.FramelessWindowHint
    title: I18n.t("Dashboard for OpenIPC")
    color: "#1e1e1e" // Main background

    property string appLanguage: I18n.language

    Component.onCompleted: {
        console.info("Main.qml Component.onCompleted start")
        I18n.language = appLanguage
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: SystemController.userManager.isLoggedIn ? 1 : 0
        
        LoginView {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        
        DashboardView {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
