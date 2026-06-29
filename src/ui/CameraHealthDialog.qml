import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: root

    modal: true
    width: Math.min(parent ? parent.width - 80 : 1100, 1120)
    height: Math.min(parent ? parent.height - 80 : 720, 760)
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property int dataVersion: 0
    property string filterText: ""
    property string iconFontFamily: materialIcons.status === FontLoader.Ready ? materialIcons.name : "Material Icons"
    property bool reportCopied: false
    property double lastAutoRefreshMs: 0
    readonly property int autoRefreshCooldownMs: 15000

    signal editRequested(int cameraIndex)
    signal majesticRequested(int cameraIndex)
    signal addToGridRequested(int cameraIndex)

    onOpened: {
        root.reportCopied = false
        var now = Date.now()
        if (now - root.lastAutoRefreshMs >= root.autoRefreshCooldownMs) {
            root.lastAutoRefreshMs = now
            root.recheckAll()
        }
    }

    FontLoader {
        id: materialIcons
        source: "qrc:/OpenIPC/src/ui/fonts/MaterialIcons-Regular.ttf"
    }

    function isOnlineStatus(statusText) {
        if (typeof SystemController.isCameraOnline === "function")
            return SystemController.isCameraOnline("", statusText || "")
        return String(statusText || "").toLowerCase() === "online"
    }

    function effectiveCameraStatus(ip, fallbackStatus) {
        var version = dataVersion
        if (!ip || ip === "") return fallbackStatus || ""
        if (typeof SystemController.effectiveCameraStatus === "function")
            return SystemController.effectiveCameraStatus(ip, fallbackStatus || "")
        return fallbackStatus || ""
    }

    function cameraDetail(ip) {
        var version = dataVersion
        if (!ip || ip === "" || typeof SystemController.cameraStatusDetail !== "function") return ""
        return SystemController.cameraStatusDetail(ip) || ""
    }

    function cameraAttentionReason(ip, fallbackStatus) {
        var version = dataVersion
        if (!ip || ip === "") return ""
        if (typeof SystemController.cameraAttentionReason === "function")
            return SystemController.cameraAttentionReason(ip, fallbackStatus || "")
        return cameraDetail(ip)
    }

    function cameraStatusSearchText(ip, fallbackStatus) {
        var version = dataVersion
        if (!ip || ip === "") return fallbackStatus || ""
        if (typeof SystemController.cameraStatusSearchText === "function")
            return SystemController.cameraStatusSearchText(ip, fallbackStatus || "")
        return effectiveCameraStatus(ip, fallbackStatus) + " " + cameraAttentionReason(ip, fallbackStatus)
    }

    function isCameraInGrid(ip) {
        var version = dataVersion
        if (!ip || ip === "") return false
        if (typeof SystemController.isCameraInGrid === "function")
            return SystemController.isCameraInGrid(ip)
        return false
    }

    function onlineCount() {
        var version = dataVersion
        if (typeof SystemController.onlineCameraCount === "function")
            return SystemController.onlineCameraCount()
        return 0
    }

    function issueCount() {
        var version = dataVersion
        if (typeof SystemController.camerasNeedingAttentionCount === "function")
            return SystemController.camerasNeedingAttentionCount()
        return 0
    }

    function rowMatches(name, ip, statusText, detailText) {
        var query = filterText.trim().toLowerCase()
        if (query === "") return true
        return [name || "", ip || "", statusText || "", detailText || ""].join(" ").toLowerCase().indexOf(query) !== -1
    }

    function recheckAll() {
        for (var i = 0; i < SystemController.cameraModel.rowCount(); ++i) {
            var cam = SystemController.cameraModel.getCamera(i)
            if (cam && cam.cameraIp) SystemController.refreshCameraHealth(cam.cameraIp)
        }
    }

    function healthReport() {
        var report = I18n.t("Диагностический отчет камер") + " — " + Qt.formatDateTime(new Date(), "yyyy-MM-dd HH:mm:ss")
        report += "\n" + I18n.t("Всего") + ": " + SystemController.cameraModel.rowCount()
                + " | " + I18n.t("Онлайн") + ": " + root.onlineCount()
                + " | " + I18n.t("Требуют внимания") + ": " + root.issueCount()
        report += "\n"

        for (var i = 0; i < SystemController.cameraModel.rowCount(); ++i) {
            var cam = SystemController.cameraModel.getCamera(i)
            if (!cam) continue
            var name = cam.cameraName && cam.cameraName.trim() !== "" ? cam.cameraName : (I18n.t("Камера") + " " + cam.cameraIp)
            var statusText = root.effectiveCameraStatus(cam.cameraIp, cam.status)
            var detailText = root.cameraAttentionReason(cam.cameraIp, cam.status)
            report += "\n" + (i + 1) + ". " + name
            report += "\n   IP: " + (cam.cameraIp || "—")
            report += "\n   " + I18n.t("Статус") + ": " + I18n.t(statusText || "Неизвестно")
            report += "\n   " + I18n.t("Причина") + ": " + (detailText !== "" ? I18n.t(detailText) : I18n.t("Диагностика без замечаний"))
            report += "\n   " + I18n.t("В раскладке") + ": " + (root.isCameraInGrid(cam.cameraIp) ? I18n.t("Да") : I18n.t("Нет"))
        }

        return report
    }

    function copyHealthReport() {
        SystemController.copyTextToClipboard(root.healthReport())
        root.reportCopied = true
        copyReportFeedbackTimer.restart()
    }

    Timer {
        id: copyReportFeedbackTimer
        interval: 1800
        repeat: false
        onTriggered: root.reportCopied = false
    }

    component StatCard: Rectangle {
        id: statCard
        property string title: ""
        property string value: "0"
        property color accent: Theme.accent

        Layout.fillWidth: true
        implicitHeight: 64
        radius: Theme.radiusMd
        color: Theme.panelSoftBackground
        border.color: Theme.panelBorder

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 2
            Text {
                text: statCard.value
                color: statCard.accent
                font.pixelSize: 22
                font.bold: true
            }
            Text {
                text: I18n.t(statCard.title)
                color: Theme.textMuted
                font.pixelSize: 11
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }
    }

    Connections {
        target: SystemController.cameraModel
        ignoreUnknownSignals: true
        function onRowsInserted(parent, first, last) { root.dataVersion++ }
        function onRowsRemoved(parent, first, last) { root.dataVersion++ }
        function onModelReset() { root.dataVersion++ }
        function onDataChanged(topLeft, bottomRight, roles) { root.dataVersion++ }
    }

    Connections {
        target: SystemController.gridModel
        ignoreUnknownSignals: true
        function onRowsInserted(parent, first, last) { root.dataVersion++ }
        function onRowsRemoved(parent, first, last) { root.dataVersion++ }
        function onModelReset() { root.dataVersion++ }
        function onDataChanged(topLeft, bottomRight, roles) { root.dataVersion++ }
    }

    Connections {
        target: SystemController
        ignoreUnknownSignals: true
        function onCameraStatusDetailsChanged() { root.dataVersion++ }
    }

    background: Rectangle {
        color: Theme.panelBackground
        radius: Theme.radiusLg
        border.color: Theme.panelBorderStrong
        border.width: 1
    }

    header: Rectangle {
        height: 70
        color: Theme.topBarBackground
        radius: Theme.radiusLg

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 18
            anchors.rightMargin: 14
            spacing: 12

            Rectangle {
                Layout.preferredWidth: 40
                Layout.preferredHeight: 40
                radius: 20
                color: Theme.controlBackground
                border.color: Theme.controlBorder

                Text {
                    anchors.centerIn: parent
                    text: "health_and_safety"
                    color: Theme.accentHover
                    font.family: root.iconFontFamily
                    font.pixelSize: 22
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Text {
                    text: I18n.t("Здоровье камер")
                    color: Theme.textPrimary
                    font.pixelSize: 20
                    font.bold: true
                }
                Text {
                    text: I18n.t("Статусы, причины проблем и быстрые проверки RTSP/Majestic")
                    color: Theme.textMuted
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }

            Button {
                id: checkAllButton
                text: I18n.t("Проверить все")
                Layout.preferredHeight: 34
                onClicked: root.recheckAll()
                background: Rectangle {
                    color: checkAllButton.down ? Theme.accent : Theme.controlBackground
                    radius: Theme.radiusMd
                    border.color: Theme.accent
                }
                contentItem: Text {
                    text: checkAllButton.text
                    color: Theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 12
                    font.bold: true
                }
            }

            Button {
                id: copyReportButton
                text: root.reportCopied ? I18n.t("Скопировано") : I18n.t("Скопировать отчет")
                Layout.preferredHeight: 34
                onClicked: root.copyHealthReport()
                background: Rectangle {
                    color: copyReportButton.down ? Theme.accent : Theme.controlBackground
                    radius: Theme.radiusMd
                    border.color: root.reportCopied ? Theme.success : Theme.controlBorderStrong
                }
                contentItem: Text {
                    text: copyReportButton.text
                    color: root.reportCopied ? Theme.success : Theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 12
                    font.bold: true
                    leftPadding: 10
                    rightPadding: 10
                }
            }

            Button {
                id: closeButton
                Layout.preferredWidth: 38
                Layout.preferredHeight: 34
                onClicked: root.close()
                background: Rectangle {
                    color: closeButton.hovered ? Theme.cardHover : Theme.controlBackground
                    radius: Theme.radiusMd
                    border.color: Theme.controlBorder
                }
                contentItem: Text {
                    text: "close"
                    color: Theme.textPrimary
                    font.family: root.iconFontFamily
                    font.pixelSize: 20
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            StatCard {
                title: "Всего"
                value: String(SystemController.cameraModel.rowCount())
                accent: Theme.textPrimary
            }
            StatCard {
                title: "Онлайн"
                value: String(root.onlineCount())
                accent: Theme.success
            }
            StatCard {
                title: "Офлайн"
                value: String(Math.max(0, SystemController.cameraModel.rowCount() - root.onlineCount()))
                accent: Theme.danger
            }
            StatCard {
                title: "Требуют внимания"
                value: String(root.issueCount())
                accent: root.issueCount() > 0 ? Theme.warning : Theme.success
            }
        }

        TextField {
            Layout.fillWidth: true
            implicitHeight: 36
            text: root.filterText
            placeholderText: I18n.t("Фильтр по имени, IP, статусу или причине…")
            color: Theme.textPrimary
            placeholderTextColor: Theme.textMuted
            selectionColor: Theme.accent
            selectedTextColor: Theme.textPrimary
            leftPadding: 12
            rightPadding: 12
            background: Rectangle {
                color: Theme.controlBackground
                radius: Theme.radiusMd
                border.color: parent.activeFocus ? Theme.accent : Theme.controlBorder
            }
            onTextChanged: root.filterText = text
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 34
            radius: Theme.radiusMd
            color: Theme.panelAltBackground
            border.color: Theme.panelBorder

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                spacing: 10
                Text { text: I18n.t("Камера"); color: Theme.textMuted; font.pixelSize: 11; Layout.fillWidth: true }
                Text { text: "IP"; color: Theme.textMuted; font.pixelSize: 11; Layout.preferredWidth: 120 }
                Text { text: I18n.t("Статус"); color: Theme.textMuted; font.pixelSize: 11; Layout.preferredWidth: 92 }
                Text { text: I18n.t("Причина"); color: Theme.textMuted; font.pixelSize: 11; Layout.preferredWidth: 250 }
                Text { text: I18n.t("В раскладке"); color: Theme.textMuted; font.pixelSize: 11; Layout.preferredWidth: 82 }
                Text { text: I18n.t("Действия"); color: Theme.textMuted; font.pixelSize: 11; Layout.preferredWidth: 250 }
            }
        }

        ListView {
            id: cameraList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8
            model: SystemController.cameraModel

            delegate: Rectangle {
                id: row
                width: cameraList.width
                height: rowVisible ? 76 : 0
                visible: rowVisible
                radius: Theme.radiusMd
                color: hover.hovered ? Theme.cardHover : Theme.cardBackground
                border.color: row.online ? Theme.controlBorder : Theme.warning

                required property int index
                required property string cameraName
                required property string cameraIp
                required property string status

                property int cameraIndex: row.index
                property string rowName: row.cameraName && row.cameraName.trim() !== "" ? row.cameraName : (I18n.t("Камера") + " " + row.cameraIp)
                property string rowStatus: root.effectiveCameraStatus(row.cameraIp, row.status)
                property string rowDetail: root.cameraAttentionReason(row.cameraIp, row.status)
                property bool online: root.isOnlineStatus(rowStatus)
                property bool inGrid: root.isCameraInGrid(row.cameraIp)
                property bool rowVisible: root.rowMatches(rowName, row.cameraIp,
                                                          root.cameraStatusSearchText(row.cameraIp, row.status),
                                                          rowDetail)

                HoverHandler { id: hover }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14
                    spacing: 10

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 9
                        CameraStatusBadge {
                            Layout.preferredWidth: 10
                            Layout.preferredHeight: 10
                            dotSize: 10
                            showText: false
                            online: row.online
                            statusText: row.rowStatus
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                Layout.fillWidth: true
                                text: row.rowName
                                color: Theme.textPrimary
                                font.pixelSize: 13
                                font.bold: true
                                elide: Text.ElideRight
                            }
                            Text {
                                Layout.fillWidth: true
                                text: row.rowDetail !== "" ? I18n.t(row.rowDetail) : I18n.t("Диагностика без замечаний")
                                color: row.rowDetail !== "" ? Theme.warning : Theme.textMuted
                                font.pixelSize: 11
                                elide: Text.ElideRight
                            }
                        }
                    }

                    Text {
                        Layout.preferredWidth: 120
                        text: row.cameraIp
                        color: Theme.textSecondary
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }

                    CameraStatusBadge {
                        Layout.preferredWidth: 92
                        Layout.preferredHeight: 26
                        online: row.online
                        statusText: row.rowStatus || "РќРµРёР·РІРµСЃС‚РЅРѕ"
                        Text {
                            anchors.centerIn: parent
                            text: I18n.t(row.rowStatus || "Неизвестно")
                            color: Theme.textPrimary
                            font.pixelSize: 11
                            font.bold: true
                        }
                    }

                    Text {
                        Layout.preferredWidth: 250
                        text: row.rowDetail !== "" ? I18n.t(row.rowDetail) : "—"
                        color: row.rowDetail !== "" ? Theme.warning : Theme.textMuted
                        font.pixelSize: 11
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.preferredWidth: 82
                        text: row.inGrid ? I18n.t("Да") : I18n.t("Нет")
                        color: row.inGrid ? Theme.success : Theme.textMuted
                        font.pixelSize: 12
                        font.bold: row.inGrid
                        horizontalAlignment: Text.AlignHCenter
                    }

                    RowLayout {
                        Layout.preferredWidth: 250
                        spacing: 6

                        Button {
                            text: I18n.t("Проверить")
                            Layout.preferredHeight: 28
                            onClicked: SystemController.refreshCameraHealth(row.cameraIp)
                        }
                        Button {
                            text: I18n.t("Majestic")
                            Layout.preferredHeight: 28
                            onClicked: root.majesticRequested(row.cameraIndex)
                        }
                        Button {
                            text: row.inGrid ? I18n.t("В сетке") : I18n.t("В сетку")
                            enabled: !row.inGrid
                            Layout.preferredHeight: 28
                            onClicked: root.addToGridRequested(row.cameraIndex)
                        }
                        Button {
                            text: I18n.t("Правка")
                            Layout.preferredHeight: 28
                            onClicked: root.editRequested(row.cameraIndex)
                        }
                    }
                }
            }

            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
        }

        Text {
            Layout.fillWidth: true
            visible: SystemController.cameraModel.rowCount() === 0
            text: I18n.t("Камер пока нет. Добавьте камеру или запустите поиск.")
            color: Theme.textMuted
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 13
        }
    }
}
