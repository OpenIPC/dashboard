pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import OpenIPC

Item {
    id: root

    width: 1280
    height: 720
    visible: false

    signal smokeFinished(bool ok, string message)

    property bool smokeOk: false
    property string smokeMessage: ""
    property int caseIndex: -1
    property var currentObject: null

    QtObject {
        id: streamingSettingsStub

        property string preferredStream: "auto"
        property bool showStatsOverlay: true
        property bool smartStreamBudget: true
        property int maxPreviewStreams: 16
        property int playerBufferMode: 1
        property int playerFillMode: 0
        property string playerRtspTransport: "tcp"
        property string playerHwDecoding: "auto"
        property real playerBrightness: 1.0
        property real playerContrast: 1.0
        property int playerHue: 0
        property real playerSaturation: 1.0
        property real playerGamma: 1.0
        property int playerOrientation: 0
        property bool playerMirror: false

        function applyCurrentSettings() {}
    }

    QtObject {
        id: downloadingUpdateStub

        property string currentVersion: "0.2.6.1"
        property string latestVersion: "0.2.8"
        property string latestName: "OpenIPC Dashboard v0.2.8"
        property string releaseNotes: "A deliberately long release note verifies that wrapped text stays inside the updater window."
        property bool latestPrerelease: false
        property bool downloadAvailable: true
        property bool downloading: true
        property bool installing: false
        property int downloadProgress: 42
        property real downloadReceivedBytes: 52428800
        property real downloadTotalBytes: 126667980
        property string downloadedFilePath: ""
        property string assetName: "OpenIPC-Dashboard-Installer.exe"
        property string errorString: ""
    }

    ListModel {
        id: toolbarLayoutsModel

        ListElement { name: "Layout 01"; isDefault: false }
        ListElement { name: "Very long layout 02"; isDefault: false }
        ListElement { name: "Layout 03"; isDefault: false }
        ListElement { name: "Very long layout 04"; isDefault: false }
        ListElement { name: "Layout 05"; isDefault: false }
        ListElement { name: "Very long layout 06"; isDefault: false }
        ListElement { name: "Layout 07"; isDefault: false }
        ListElement { name: "Very long layout 08"; isDefault: false }
    }

    Item {
        id: host
        width: root.width
        height: root.height
        visible: false
    }

    Timer {
        id: smokeStepTimer

        interval: 16
        repeat: false
        onTriggered: root.runNextCase()
    }

    Component.onCompleted: smokeStepTimer.start()

    function cases() {
        return [
            {
                name: "Login",
                component: loginComponent,
                parentObject: host,
                validate: function(object) {
                    return object.keyboardNavigationReady
                        ? ""
                        : "login keyboard navigation is not configured"
                }
            },
            {
                name: "Dashboard",
                component: dashboardComponent,
                parentObject: host,
                validate: function(object) {
                    return object.layoutReady ? "" : "dashboard layout has invalid geometry"
                }
            },
            {
                name: "Dashboard Compact",
                component: compactDashboardComponent,
                parentObject: host,
                validate: function(object) {
                    return object.layoutReady ? "" : "compact dashboard layout has invalid geometry"
                }
            },
            {
                name: "Sidebar Tools Collapse",
                component: collapsedSidebarComponent,
                parentObject: host,
                validate: function(object) {
                    return object.layoutReady
                        ? ""
                        : "sidebar tools did not collapse cleanly"
                }
            },
            {
                name: "Layout Toolbar Overflow",
                component: compactLayoutToolbarComponent,
                parentObject: host,
                validate: function(object) {
                    return object.layoutReady ? "" : "layout toolbar actions are clipped"
                }
            },
            { name: "Grid Cell", component: gridCellComponent, parentObject: host },
            { name: "Grid Cell Compact", component: compactGridCellComponent, parentObject: host },
            {
                name: "Sidebar Camera Card",
                component: sidebarCameraCardComponent,
                parentObject: host,
                validate: function(object) {
                    return object.layoutReady
                        ? ""
                        : "sidebar camera metadata is clipped"
                }
            },
            {
                name: "Settings",
                component: settingsComponent,
                parentObject: null,
                showForLayout: true,
                validate: function(object) {
                    return object.contentLayoutReady
                        ? ""
                        : "main settings content has invalid geometry"
                }
            },
            {
                name: "Settings Compact",
                component: compactSettingsComponent,
                parentObject: null,
                showForLayout: true,
                validate: function(object) {
                    return object.contentLayoutReady
                        ? ""
                        : "compact settings content has invalid geometry"
                }
            },
            {
                name: "Updater Download Compact",
                component: compactUpdaterDownloadComponent,
                parentObject: host,
                validate: function(object) {
                    return object.layoutReady
                        ? ""
                        : "downloading updater content exceeds its window"
                }
            },
            {
                name: "Settings Streaming Compact",
                component: compactStreamingSettingsComponent,
                parentObject: host,
                validate: function(object) {
                    return object.layoutReady
                        ? ""
                        : "compact streaming settings layout overflow"
                }
            },
            { name: "Camera Search", component: cameraSearchComponent, parentObject: host },
            {
                name: "Fleet Management",
                component: fleetManagementComponent,
                parentObject: host,
                showForLayout: true,
                validate: function(object) {
                    return object.contentLayoutReady ? "" : "fleet dialog exceeds its host"
                }
            },
            {
                name: "Health Center",
                component: healthCenterComponent,
                parentObject: host,
                showForLayout: true,
                validate: function(object) {
                    return object.layoutReady && object.diagnosticProfileCount === 4
                        ? ""
                        : "health v2 layout or profiles are invalid (top="
                          + object.statsTopInDialog + ", headerBottom="
                          + object.headerBottomInDialog + ", profiles="
                          + object.diagnosticProfileCount + ")"
                }
            },
            {
                name: "Analytics",
                component: analyticsComponent,
                parentObject: host,
                showForLayout: true,
                validate: function(object) {
                    return object.layoutReady ? "" : "analytics tabs or content overflow"
                }
            },
            {
                name: "Application Logs",
                component: logViewComponent,
                parentObject: host,
                showForLayout: true,
                validate: function(object) {
                    return object.layoutReady ? "" : "log dialog exceeds its host"
                }
            },
            { name: "Camex", component: camexComponent, parentObject: host, showForLayout: true },
            {
                name: "Dashboard Camex Open",
                component: dashboardCamexOpenComponent,
                parentObject: null,
                showForLayout: true,
                validate: function(object) {
                    return object.camexOpened ? "" : "Camex dialog was not opened from Dashboard"
                }
            },
            { name: "Majestic/OpenIPC Control Center", component: majesticComponent, parentObject: host }
        ]
    }

    function cleanupCurrent() {
        if (currentObject) {
            currentObject.destroy()
            currentObject = null
        }
    }

    function finish(ok, message) {
        cleanupCurrent()
        smokeOk = ok
        smokeMessage = message
        smokeFinished(ok, message)
    }

    function runNextCase() {
        if (currentObject && caseIndex >= 0) {
            var completedCase = cases()[caseIndex]
            if (completedCase.validate) {
                var validationError = completedCase.validate(currentObject)
                if (validationError) {
                    finish(false, completedCase.name + ": " + validationError)
                    return
                }
            }
        }

        cleanupCurrent()

        var allCases = cases()
        caseIndex += 1
        if (caseIndex >= allCases.length) {
            finish(true, "QML smoke passed: " + allCases.length + " components")
            return
        }

        var smokeCase = allCases[caseIndex]
        if (smokeCase.component.status === Component.Error) {
            finish(false, smokeCase.name + ": " + smokeCase.component.errorString())
            return
        }

        var created = smokeCase.component.createObject(smokeCase.parentObject)
        if (!created) {
            finish(false, smokeCase.name + ": createObject returned null")
            return
        }

        currentObject = created
        if (currentObject.hasOwnProperty("visible")) {
            currentObject.visible = smokeCase.showForLayout === true
        }

        smokeStepTimer.restart()
    }

    Component {
        id: loginComponent

        LoginView {
            width: 1280
            height: 720
        }
    }

    Component {
        id: dashboardComponent

        DashboardView {
            width: 1280
            height: 720
        }
    }

    Component {
        id: compactDashboardComponent

        DashboardView {
            width: 960
            height: 540
            isSidebarVisible: true
        }
    }

    Component {
        id: compactLayoutToolbarComponent

        DashboardLayoutToolbar {
            width: 480
            height: 32
            layoutsModel: toolbarLayoutsModel
            currentLayoutIndex: 0
        }
    }

    Component {
        id: collapsedSidebarComponent

        Item {
            id: collapsedSidebarHost

            width: 300
            height: 540
            property bool toolsExpanded: true
            readonly property bool layoutReady: !toolsExpanded
                                                && !testSidebar.toolsContentVisible

            Component.onCompleted: testSidebar.toolsExpandedToggleRequested()

            DashboardSidebar {
                id: testSidebar

                width: collapsedSidebarHost.width
                height: collapsedSidebarHost.height
                sidebarWidth: collapsedSidebarHost.width
                systemController: SystemController
                toolsExpanded: collapsedSidebarHost.toolsExpanded
                onToolsExpandedToggleRequested: collapsedSidebarHost.toolsExpanded =
                                                    !collapsedSidebarHost.toolsExpanded
            }
        }
    }

    Component {
        id: gridCellComponent

        GridCell {
            width: 320
            height: 180
            cameraName: "Smoke Cell"
            status: "Offline"
            streamUrl: ""
            sdStreamUrl: ""
            hdStreamUrl: ""
            recordingOwner: "manual"
            canLive: false
            canPlayback: false
            canPtz: false
            canExport: false
            canSettings: false
        }
    }

    Component {
        id: compactGridCellComponent

        GridCell {
            width: 160
            height: 90
            cameraName: "Smoke Compact Cell"
            status: "Offline"
            streamUrl: ""
            sdStreamUrl: ""
            hdStreamUrl: ""
            canLive: false
            canPlayback: false
            canPtz: false
            canExport: false
            canSettings: false
        }
    }

    Component {
        id: sidebarCameraCardComponent

        Item {
            width: 300
            height: 96

            readonly property bool layoutReady: cardRepeater.count > 0
                                                && cardRepeater.itemAt(0)
                                                && cardRepeater.itemAt(0).cameraIp === "192.168.1.219"

            ListModel {
                id: cardModel
                ListElement {
                    cameraName: "OpenIPC smoke camera"
                    cameraIp: "192.168.1.219"
                    cameraPort: 554
                }
            }

            Repeater {
                id: cardRepeater
                model: cardModel

                delegate: DeviceListItem {
                    width: 300
                    effectiveStatus: "Online"
                    effectiveDetail: "Optional probe warning"
                    online: true
                    canSettings: true
                    systemController: SystemController
                }
            }
        }
    }

    Component {
        id: settingsComponent

        SettingsDialog {
        }
    }

    Component {
        id: compactSettingsComponent

        SettingsDialog {
            width: 560
            height: 480
            language: "en"
        }
    }

    Component {
        id: compactStreamingSettingsComponent

        SettingsStreamingPage {
            width: 520
            height: 420
            settings: streamingSettingsStub
        }
    }

    Component {
        id: compactUpdaterDownloadComponent

        AppUpdateContent {
            width: 600
            height: 480
            updateChecker: downloadingUpdateStub
        }
    }

    Component {
        id: cameraSearchComponent

        CameraSearchDialog {
        }
    }

    Component {
        id: healthCenterComponent

        Window {
            id: healthTestWindow

            width: 1280
            height: 720
            visible: false

            property alias layoutReady: healthDialog.layoutReady
            property alias diagnosticProfileCount: healthDialog.diagnosticProfileCount
            property alias statsTopInDialog: healthDialog.statsTopInDialog
            property alias headerBottomInDialog: healthDialog.headerBottomInDialog

            onVisibleChanged: {
                if (visible)
                    Qt.callLater(function() { healthDialog.open() })
            }

            CameraHealthDialog {
                id: healthDialog

                autoRefreshOnOpen: false
            }
        }
    }

    Component {
        id: fleetManagementComponent

        FleetManagementDialog {
        }
    }

    Component {
        id: analyticsComponent

        AnalyticsView {
        }
    }

    Component {
        id: majesticComponent

        MajesticControlDialog {
            cameraName: "Smoke Camera"
            cameraHost: "127.0.0.1"
            cameraPort: 80
            cameraUser: "root"
            cameraPassword: ""
        }
    }

    Component {
        id: logViewComponent

        LogView {
            logModel: SystemController.logModel
        }
    }

    Component {
        id: camexComponent

        CamexView {
        }
    }

    Component {
        id: dashboardCamexOpenComponent

        Window {
            id: camexOpenWindow

            width: 1280
            height: 720
            visible: false
            property bool camexOpened: false

            onVisibleChanged: {
                if (visible) {
                    Qt.callLater(function() {
                        dashboard.openCamexDialog()
                        camexOpened = true
                    })
                }
            }

            DashboardView {
                id: dashboard

                anchors.fill: parent
            }
        }
    }
}
