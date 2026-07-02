import QtQuick
import QtQuick.Controls
import QtQuick.Window
import OpenIPC

ApplicationWindow {
    id: window
    width: 1280
    height: 720
    minimumWidth: 960
    minimumHeight: 540
    visible: true
    flags: Qt.Window | Qt.FramelessWindowHint
    title: I18n.t("Dashboard for OpenIPC")
    color: Theme.appBackground

    property string appLanguage: I18n.language
    property bool dashboardLoaded: SystemController.userManager.isLoggedIn

    Component.onCompleted: {
        console.info("Main.qml Component.onCompleted start")
        I18n.language = appLanguage
        if (!SystemController.userManager.isLoggedIn
                && SystemController.userManager.hasUsers
                && SystemController.userManager.rememberedUsername !== ""
                && SystemController.userManager.rememberedPassword !== "") {
            SystemController.userManager.loginWithRememberedCredentials()
        }
        window.dashboardLoaded = SystemController.userManager.isLoggedIn
        Qt.callLater(function() {
            window.showMaximized()
        })
    }

    Connections {
        target: SystemController.userManager

        function onIsLoggedInChanged() {
            authSwitchTimer.restart()
        }
    }

    Timer {
        id: authSwitchTimer
        interval: 0
        repeat: false
        onTriggered: window.dashboardLoaded = SystemController.userManager.isLoggedIn
    }

    Loader {
        id: pageLoader
        anchors.fill: parent
        clip: true
        sourceComponent: window.dashboardLoaded ? dashboardComponent : loginComponent
    }

    Binding {
        target: pageLoader.item
        property: "width"
        value: pageLoader.width
        when: pageLoader.item !== null
    }

    Binding {
        target: pageLoader.item
        property: "height"
        value: pageLoader.height
        when: pageLoader.item !== null
    }

    Component {
        id: loginComponent

        LoginView {
        }
    }

    Component {
        id: dashboardComponent

        DashboardView {
        }
    }
}
