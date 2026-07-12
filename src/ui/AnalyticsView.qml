import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC
import "analytics"

Dialog {
    id: root
    modal: true
    dim: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    
    // Center in parent
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    width: parent.width * 0.9
    height: parent.height * 0.9
    
    background: Rectangle {
        color: Theme.metroSidebarBackground
        radius: Theme.metroTileRadius
        border.color: Theme.metroStroke
        border.width: 1
    }
    
    header: Rectangle {
        height: 56
        color: "transparent"
        
        ColumnLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.leftMargin: 14
            anchors.rightMargin: 54
            anchors.topMargin: 8
            anchors.bottomMargin: 8
            spacing: 0
            
            Text {
                text: I18n.t("Аналитика")
                color: Theme.textPrimary
                font.pixelSize: 18
                font.bold: true
            }

            Text {
                text: I18n.t("Управление ИИ, событиями и правилами")
                color: Theme.textMuted
                font.pixelSize: 11
            }
        }

        MetroWindowButton {
            kind: "close"
            width: 32
            height: 32
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 8
            anchors.rightMargin: 10
            onClicked: root.close()
        }
        
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: Theme.metroStroke
        }
    }
    
    property string evidenceSnapshotsDir: ""
    property string evidenceClipsDir: ""
    property string faceSnapshotsDir: ""
    property string objectSnapshotsDir: ""
    property string plateSnapshotsDir: ""
    property var analyticsDiagnosticsData: ({})
    readonly property bool layoutReady: bar.width > 0
                                        && bar.height >= 40
                                        && bar.count === 7
                                        && overviewPage.layoutReady
    readonly property int summaryColumns: width >= 1500 ? 6 : width >= 1100 ? 3 : 2
    property int rulesModuleType: 1

    function rulesModuleName(type) {
        if (type === 0) return I18n.t("Лица")
        if (type === 1) return I18n.t("Объекты")
        if (type === 2) return I18n.t("Номера")
        return I18n.t("Модуль")
    }

    function refreshEvidenceDir() {
        var settings = SystemController.analyticsEngine.getSettings()
        if (settings && settings.evidence && settings.evidence.snapshotsDir) {
            evidenceSnapshotsDir = settings.evidence.snapshotsDir
        } else {
            evidenceSnapshotsDir = ""
        }
        if (settings && settings.evidence && settings.evidence.clipsDir) {
            evidenceClipsDir = settings.evidence.clipsDir
        } else {
            evidenceClipsDir = ""
        }
        refreshModuleDirs()
    }

    function moduleSnapshotsDir(type, fallbackFolder) {
        var cfg = SystemController.analyticsEngine.getModuleConfig(type)
        if (cfg && cfg.snapshotsDir) return cfg.snapshotsDir
        if (evidenceSnapshotsDir && evidenceSnapshotsDir !== "") {
            return evidenceSnapshotsDir + "/" + fallbackFolder
        }
        return ""
    }

    function refreshModuleDirs() {
        faceSnapshotsDir = moduleSnapshotsDir(0, "Face_Detector")
        objectSnapshotsDir = moduleSnapshotsDir(1, "Object_Counter")
        plateSnapshotsDir = moduleSnapshotsDir(2, "License_Plate")
    }

    function refreshAnalyticsDiagnostics() {
        if (SystemController.analyticsEngine && SystemController.analyticsEngine.analyticsDiagnostics) {
            analyticsDiagnosticsData = SystemController.analyticsEngine.analyticsDiagnostics
        } else {
            analyticsDiagnosticsData = ({})
        }
    }

    Component.onCompleted: {
        refreshEvidenceDir()
        refreshAnalyticsDiagnostics()
    }

    Connections {
        target: SystemController.analyticsEngine
        function onSettingsChanged() { refreshEvidenceDir() }
        function onModuleConfigChanged(type) { refreshModuleDirs() }
        function onAnalyticsTelemetryChanged() { refreshAnalyticsDiagnostics() }
        function onAnalyticsEventsChanged() { refreshAnalyticsDiagnostics() }
    }
    
    contentItem: ColumnLayout {
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 0
            Layout.minimumHeight: 0
            Layout.maximumHeight: 0
            color: Theme.panelBackground
            radius: Theme.radiusLg
            border.color: Theme.panelBorder
            border.width: 1
            Layout.bottomMargin: 0
            visible: false

            GridLayout {
                id: summaryGrid
                anchors.fill: parent
                anchors.margins: 10
                columns: root.summaryColumns
                rowSpacing: 8
                columnSpacing: 8

                Rectangle {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 160
                    implicitHeight: 60
                    color: Theme.cardBackground
                    radius: Theme.radiusMd
                    border.color: Theme.cardBorder

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 2

                        Text {
                            text: I18n.t("Обработано кадров")
                            color: Theme.textMuted
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                            Layout.fillWidth: true
                        }
                        Text { text: String(analyticsDiagnosticsData.processedFrames || 0); color: Theme.textPrimary; font.pixelSize: 18; font.bold: true }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 160
                    implicitHeight: 60
                    color: Theme.cardBackground
                    radius: Theme.radiusMd
                    border.color: Theme.cardBorder

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 2

                        Text {
                            text: I18n.t("Пропущено кадров")
                            color: Theme.textMuted
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                            Layout.fillWidth: true
                        }
                        Text { text: String(analyticsDiagnosticsData.skippedFrames || 0); color: Theme.textPrimary; font.pixelSize: 18; font.bold: true }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 160
                    implicitHeight: 60
                    color: Theme.cardBackground
                    radius: Theme.radiusMd
                    border.color: Theme.cardBorder

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 2

                        Text {
                            text: I18n.t("Детекции")
                            color: Theme.textMuted
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                            Layout.fillWidth: true
                        }
                        Text { text: String(analyticsDiagnosticsData.detections || 0); color: Theme.textPrimary; font.pixelSize: 18; font.bold: true }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 160
                    implicitHeight: 60
                    color: Theme.cardBackground
                    radius: Theme.radiusMd
                    border.color: Theme.cardBorder

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 2

                        Text {
                            text: I18n.t("События")
                            color: Theme.textMuted
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                            Layout.fillWidth: true
                        }
                        Text { text: String(analyticsDiagnosticsData.events || 0); color: Theme.textPrimary; font.pixelSize: 18; font.bold: true }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 160
                    implicitHeight: 60
                    color: Theme.cardBackground
                    radius: Theme.radiusMd
                    border.color: Theme.cardBorder

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 2

                        Text {
                            text: I18n.t("Средняя задержка")
                            color: Theme.textMuted
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                            Layout.fillWidth: true
                        }
                        Text {
                            text: Number(analyticsDiagnosticsData.averageInferenceMs || 0).toFixed(1) + " ms"
                            color: Theme.textPrimary
                            font.pixelSize: 18
                            font.bold: true
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 160
                    implicitHeight: 60
                    color: Theme.cardBackground
                    radius: Theme.radiusMd
                    border.color: Theme.cardBorder

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 2

                        Text {
                            text: I18n.t("Активные треки")
                            color: Theme.textMuted
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                            Layout.fillWidth: true
                        }
                        Text { text: String(analyticsDiagnosticsData.activeTracks || 0); color: Theme.textPrimary; font.pixelSize: 18; font.bold: true }
                    }
                }
            }
        }
        
        // Custom Tab Bar
        TabBar {
            id: bar
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            spacing: 6
            background: Rectangle {
                color: "transparent"
            }
            
            component CustomTabButton: TabButton {
                id: tabBtn
                implicitWidth: Math.max(92, tabLabel.implicitWidth + 28)
                implicitHeight: 38
                width: implicitWidth
                height: implicitHeight
                focusPolicy: Qt.StrongFocus
                background: Rectangle {
                    color: tabBtn.checked
                           ? Theme.metroSurfaceAlt
                           : tabBtn.hovered ? Theme.metroTileHover : Theme.metroTile
                    radius: Theme.metroTileRadius
                    border.color: tabBtn.visualFocus || tabBtn.checked
                                  ? Theme.metroStrokeStrong
                                  : Theme.metroStroke
                    border.width: tabBtn.visualFocus || tabBtn.checked ? 2 : 1

                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: 6
                        anchors.rightMargin: 6
                        height: 2
                        color: tabBtn.checked ? Theme.metroBlue : "transparent"
                    }
                }
                contentItem: Text {
                    id: tabLabel

                    text: tabBtn.text
                    color: tabBtn.checked ? Theme.metroBlue : Theme.textMuted
                    font.pixelSize: 13
                    font.bold: tabBtn.checked
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }
            
            CustomTabButton { text: I18n.t("Обзор") }
            CustomTabButton { text: I18n.t("Камеры") }
            CustomTabButton { text: I18n.t("Модули") }
            CustomTabButton { text: I18n.t("События") }
            CustomTabButton { text: I18n.t("Правила") }
            CustomTabButton { text: I18n.t("Архив") }
            CustomTabButton { text: I18n.t("Диагностика") }
        }
        
        // Content
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.metroBackground
            radius: Theme.metroTileRadius
            border.color: Theme.metroStroke
            border.width: 1

            Component {
                id: eventsPageComponent

                EventsPanel {
                    model: SystemController.analyticsEngine
                    moduleType: -1
                    snapshotsDir: root.evidenceSnapshotsDir
                    clipsDir: root.evidenceClipsDir
                    moduleBadgeText: I18n.t("Лента событий")
                }
            }

            Component {
                id: archivePageComponent

                Item {
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 8

                        TabBar {
                            id: archiveBar
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            background: Rectangle {
                                color: Theme.panelSoftBackground
                                radius: Theme.radiusMd
                                border.color: Theme.cardBorder
                            }

                            component ArchiveTabButton: TabButton {
                                id: archiveTabBtn
                                width: implicitWidth + 28
                                background: Rectangle {
                                    color: archiveTabBtn.checked ? Theme.metroSurfaceAlt : Theme.metroTile
                                    Rectangle {
                                        anchors.bottom: parent.bottom
                                        width: parent.width
                                        height: 2
                                        color: archiveTabBtn.checked ? Theme.metroBlue : "transparent"
                                    }
                                }
                                contentItem: Text {
                                    text: archiveTabBtn.text
                                    color: archiveTabBtn.checked ? Theme.metroBlue : Theme.textMuted
                                    font.pixelSize: 13
                                    font.bold: archiveTabBtn.checked
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }

                            ArchiveTabButton { text: I18n.t("Лица") }
                            ArchiveTabButton { text: I18n.t("Объекты") }
                            ArchiveTabButton { text: I18n.t("Номера") }
                        }

                        StackLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            currentIndex: archiveBar.currentIndex

                            FaceSnapshotsPanel {
                                model: SystemController.analyticsEngine
                                snapshotsDirOverride: root.faceSnapshotsDir
                                clipsDirOverride: root.evidenceClipsDir
                            }

                            ObjectCounterPanel {
                                model: SystemController.analyticsEngine
                                snapshotsDirOverride: root.objectSnapshotsDir
                                clipsDirOverride: root.evidenceClipsDir
                            }

                            LicensePlatePanel {
                                model: SystemController.analyticsEngine
                                snapshotsDirOverride: root.plateSnapshotsDir
                                clipsDirOverride: root.evidenceClipsDir
                            }
                        }
                    }
                }
            }

            StackLayout {
                anchors.fill: parent
                anchors.margins: 8
                currentIndex: bar.currentIndex

                AnalyticsOverviewPanel {
                    id: overviewPage

                    diagnostics: analyticsDiagnosticsData
                }

                AnalyticsCamerasPanel {
                    diagnostics: analyticsDiagnosticsData
                }

                AnalyticsModulesPanel {
                }

                Loader {
                    active: StackLayout.isCurrentItem
                    sourceComponent: eventsPageComponent
                }

                Item {
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                text: I18n.t("Модуль")
                                color: Theme.textMuted
                                font.pixelSize: 12
                                Layout.alignment: Qt.AlignVCenter
                            }

                            StyledComboBox {
                                Layout.preferredWidth: 220
                                model: [I18n.t("Лица"), I18n.t("Объекты"), I18n.t("Номера")]
                                currentIndex: root.rulesModuleType
                                onUserSelected: root.rulesModuleType = currentIndex
                            }

                            Item { Layout.fillWidth: true }
                        }

                        RulesPanel {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            model: SystemController.analyticsEngine
                            moduleType: root.rulesModuleType
                            moduleName: root.rulesModuleName(root.rulesModuleType)
                        }
                    }
                }

                Loader {
                    active: StackLayout.isCurrentItem
                    sourceComponent: archivePageComponent
                }

                ScrollView {
                    id: diagnosticsScroll
                    clip: true
                    contentWidth: availableWidth
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                    ColumnLayout {
                        width: diagnosticsScroll.availableWidth
                        spacing: 10

                        GridLayout {
                            Layout.fillWidth: true
                            columns: diagnosticsScroll.availableWidth >= 980 ? 4 : diagnosticsScroll.availableWidth >= 620 ? 2 : 1
                            columnSpacing: 8
                            rowSpacing: 8

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.minimumWidth: 180
                                implicitHeight: 76
                                radius: Theme.radiusLg
                                color: Theme.cardBackground
                                border.color: Theme.cardBorder

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 3
                                    Text { text: I18n.t("Event store"); color: Theme.textMuted; font.pixelSize: 11 }
                                    Text {
                                        text: analyticsDiagnosticsData.eventStoreReady ? I18n.t("Готов") : I18n.t("Недоступен")
                                        color: analyticsDiagnosticsData.eventStoreReady ? Theme.success : Theme.danger
                                        font.pixelSize: 18
                                        font.bold: true
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.minimumWidth: 180
                                implicitHeight: 76
                                radius: Theme.radiusLg
                                color: Theme.cardBackground
                                border.color: Theme.cardBorder

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 3
                                    Text { text: I18n.t("Буфер событий"); color: Theme.textMuted; font.pixelSize: 11 }
                                    Text { text: String(analyticsDiagnosticsData.eventBufferSize || 0); color: Theme.textPrimary; font.pixelSize: 18; font.bold: true }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.minimumWidth: 180
                                implicitHeight: 76
                                radius: Theme.radiusLg
                                color: Theme.cardBackground
                                border.color: Theme.cardBorder

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 3
                                    Text { text: I18n.t("Очередь выгрузки"); color: Theme.textMuted; font.pixelSize: 11 }
                                    Text { text: String(analyticsDiagnosticsData.uploadQueueDepth || 0); color: Theme.textPrimary; font.pixelSize: 18; font.bold: true }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.minimumWidth: 180
                                implicitHeight: 76
                                radius: Theme.radiusLg
                                color: Theme.cardBackground
                                border.color: Theme.cardBorder

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 3
                                    Text { text: I18n.t("Камер в телеметрии"); color: Theme.textMuted; font.pixelSize: 11 }
                                    Text {
                                        text: String(analyticsDiagnosticsData.cameraStats ? analyticsDiagnosticsData.cameraStats.length : 0)
                                        color: Theme.textPrimary
                                        font.pixelSize: 18
                                        font.bold: true
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.minimumWidth: 180
                                implicitHeight: 76
                                radius: Theme.radiusLg
                                color: Theme.cardBackground
                                border.color: Theme.cardBorder

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 3
                                    Text { text: I18n.t("AI FPS"); color: Theme.textMuted; font.pixelSize: 11 }
                                    Text {
                                        text: String(analyticsDiagnosticsData.analyticsTargetFps || 0)
                                        color: Theme.textPrimary
                                        font.pixelSize: 18
                                        font.bold: true
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.minimumWidth: 180
                                implicitHeight: 76
                                radius: Theme.radiusLg
                                color: Theme.cardBackground
                                border.color: Theme.cardBorder

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 3
                                    Text { text: I18n.t("AI-задачи"); color: Theme.textMuted; font.pixelSize: 11 }
                                    Text {
                                        text: String(analyticsDiagnosticsData.analyticsActiveJobs || 0)
                                              + " / "
                                              + String(analyticsDiagnosticsData.analyticsMaxParallelJobs || 0)
                                        color: Number(analyticsDiagnosticsData.analyticsActiveJobs || 0) > 0 ? Theme.success : Theme.textPrimary
                                        font.pixelSize: 18
                                        font.bold: true
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: storePathColumn.implicitHeight + 20
                            radius: Theme.radiusLg
                            color: Theme.cardBackground
                            border.color: Theme.cardBorder

                            ColumnLayout {
                                id: storePathColumn
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 6

                                Text {
                                    text: I18n.t("Хранилище событий")
                                    color: Theme.textPrimary
                                    font.pixelSize: 13
                                    font.bold: true
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: analyticsDiagnosticsData.eventStorePath || I18n.t("Путь не задан")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    wrapMode: Text.WrapAnywhere
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
