import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import OpenIPC

ApplicationWindow {
    id: window
    width: Math.min(1280, Screen.desktopAvailableWidth || 1280)
    height: Math.min(720, Screen.desktopAvailableHeight || 720)
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
