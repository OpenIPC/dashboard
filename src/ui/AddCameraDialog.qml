import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: root

    title: isEditMode ? I18n.t("Редактировать камеру") : I18n.t("Добавить новую камеру")
    modal: true
    width: Math.min(860, parent ? parent.width - 80 : 860)
    height: Math.min(760, parent ? parent.height - 80 : 760)
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    closePolicy: Popup.CloseOnEscape

    property string initialName: ""
    property string initialIp: ""
    property int initialPort: 554
    property int initialOnvifPort: 80
    property bool isEditMode: false
    property int editIndex: -1
    property string initialLogin: "root"
    property string initialPassword: ""
    property string initialHdUrl: ""
    property string initialSdUrl: ""

    property int stepIndex: 0
    readonly property int stepCount: 4
    property string formError: ""
    property string rtspProbeId: ""
    property string majesticProbeId: ""
    property string rtspProbeState: "idle"
    property string majesticProbeState: "idle"
    property string onvifProbeState: "idle"
    property string rtspProbeMessage: ""
    property string majesticProbeMessage: ""
    property string onvifProbeMessage: ""
    property bool internalUpdate: false
    readonly property var rtspTemplateModel: [
        "OpenIPC",
        "XM / Xiongmai",
        "XM / Sofia",
        "Hikvision",
        "Dahua",
        "Reolink",
        "TP-Link",
        "Uniview",
        "Custom"
    ]

    onOpened: {
        internalUpdate = true
        stepIndex = 0
        formError = ""
        qrPayloadField.text = ""
        nameField.text = initialName
        ipField.text = initialIp
        rtspPortField.text = (initialPort > 0 ? initialPort : 554).toString()
        httpOnvifPortField.text = (initialOnvifPort > 0 ? initialOnvifPort : 80).toString()
        loginField.text = initialLogin || "root"
        passwordField.text = initialPassword
        channelField.text = "1"
        hdProfileField.text = "0"
        sdProfileField.text = "1"
        urlTemplateCombo.currentIndex = detectTemplateIndex(initialHdUrl)

        if (isEditMode && initialHdUrl !== "" && urlTemplateCombo.currentText === "Custom") {
            hdUrlField.text = initialHdUrl
            sdUrlField.text = initialSdUrl
        } else {
            updateUrl()
        }

        internalUpdate = false
        resetProbeStates()
    }

    background: Rectangle {
        color: Theme.metroSurface
        border.color: Theme.metroStroke
        radius: Theme.metroTileRadius
    }

    header: Rectangle {
        color: Theme.metroSidebarBackground
        height: 70
        radius: Theme.metroTileRadius

        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.radius
            color: parent.color
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 24
            anchors.rightMargin: 14
            spacing: 14

            Rectangle {
                Layout.preferredWidth: 44
                Layout.preferredHeight: 44
                radius: Theme.metroTileRadius
                color: Theme.metroBlue

                Text {
                    anchors.centerIn: parent
                    text: "M"
                    color: Theme.textPrimary
                    font.family: Theme.metroFontFamily
                    font.bold: true
                    font.pixelSize: 20
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    Layout.fillWidth: true
                    text: root.title
                    color: Theme.textPrimary
                    font.bold: true
                    font.pixelSize: 21
                    elide: Text.ElideRight
                }

                Text {
                    Layout.fillWidth: true
                    text: I18n.t("Мастер подключения RTSP, Majestic API и ONVIF/PTZ")
                    color: Theme.textSecondary
                    font.pixelSize: 13
                    elide: Text.ElideRight
                }
            }

            MetroWindowButton {
                kind: "close"
                Layout.preferredWidth: 42
                Layout.preferredHeight: 42
                onClicked: root.close()
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
                color: Theme.metroBackground

            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                Rectangle {
                    Layout.preferredWidth: 230
                    Layout.fillHeight: true
                    color: Theme.metroSurface
                    border.color: Theme.metroStroke
                    radius: Theme.metroTileRadius

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 10

                        Text {
                            Layout.fillWidth: true
                            text: I18n.t("Шаги подключения")
                            color: Theme.textPrimary
                            font.bold: true
                            font.pixelSize: 16
                        }

                        Text {
                            Layout.fillWidth: true
                            text: I18n.t("Проверьте камеру до сохранения: так меньше сюрпризов в раскладке.")
                            color: Theme.textSecondary
                            wrapMode: Text.WordWrap
                            font.pixelSize: 12
                        }

                        Repeater {
                            model: [
                                { title: I18n.t("Данные"), hint: I18n.t("Имя, адрес и авторизация") },
                                { title: I18n.t("Подключение"), hint: I18n.t("Порты и шаблон URL") },
                                { title: I18n.t("Проверка"), hint: I18n.t("RTSP, Majestic и ONVIF") },
                                { title: I18n.t("Потоки"), hint: I18n.t("HD/SD RTSP URL") }
                            ]

                            delegate: Rectangle {
                                id: stepCard
                                required property int index
                                required property var modelData

                                Layout.fillWidth: true
                                height: 64
                                radius: 8
                                color: root.stepIndex === index ? Theme.cardHover : Theme.controlBackground
                                border.color: root.stepIndex === index ? Theme.accent : Theme.panelBorder

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 10

                                    Rectangle {
                                        Layout.preferredWidth: 30
                                        Layout.preferredHeight: 30
                                        radius: 15
                                        color: root.stepIndex === stepCard.index ? Theme.accent : Theme.panelSoftBackground
                                        border.color: Theme.panelBorder

                                        Text {
                                            anchors.centerIn: parent
                                            text: String(stepCard.index + 1)
                                            color: Theme.textPrimary
                                            font.bold: true
                                            font.pixelSize: 13
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 1

                                        Text {
                                            Layout.fillWidth: true
                                            text: stepCard.modelData.title
                                            color: Theme.textPrimary
                                            font.bold: true
                                            font.pixelSize: 13
                                            elide: Text.ElideRight
                                        }

                                        Text {
                                            Layout.fillWidth: true
                                            text: stepCard.modelData.hint
                                            color: Theme.textMuted
                                            font.pixelSize: 11
                                            elide: Text.ElideRight
                                        }
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.stepIndex = stepCard.index
                                }
                            }
                        }

                        Item { Layout.fillHeight: true }

                        Text {
                            Layout.fillWidth: true
                            text: I18n.t("Пароли не записываются в RTSP URL: они хранятся отдельно и подставляются только при подключении.")
                            color: Theme.textMuted
                            wrapMode: Text.WordWrap
                            font.pixelSize: 11
                        }
                    }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: root.stepIndex

                    ScrollView {
                        clip: true
                        contentWidth: availableWidth

                        ColumnLayout {
                            width: parent.width
                            spacing: 14

                            SectionHeader {
                                title: I18n.t("Основные данные")
                                subtitle: I18n.t("Можно вставить OpenIPC QR payload или RTSP URL, а можно заполнить поля вручную.")
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                color: Theme.cardBackground
                                border.color: Theme.panelBorder
                                radius: 10
                                implicitHeight: importLayout.implicitHeight + 24

                                ColumnLayout {
                                    id: importLayout
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 8

                                    Text {
                                        Layout.fillWidth: true
                                        text: I18n.t("Быстрый импорт")
                                        color: Theme.textPrimary
                                        font.bold: true
                                        font.pixelSize: 14
                                    }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 8

                                        TextField {
                                            id: qrPayloadField
                                            Layout.fillWidth: true
                                            placeholderText: I18n.t("OpenIPC payload или rtsp://...")
                                            color: Theme.textPrimary
                                            placeholderTextColor: Theme.textMuted
                                            selectByMouse: true
                                            background: FieldBackground {}
                                            onAccepted: root.importPayload()
                                        }

                                        WizardButton {
                                            text: I18n.t("Импорт")
                                            primary: true
                                            onClicked: root.importPayload()
                                        }
                                    }
                                }
                            }

                            FormGrid {
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 5
                                    FieldLabel { text: I18n.t("Название") }
                                    TextField {
                                        id: nameField
                                        Layout.fillWidth: true
                                        color: Theme.textPrimary
                                        placeholderText: I18n.t("Например: Двор, Подъезд, openipc-hi3516")
                                        placeholderTextColor: Theme.textMuted
                                        selectByMouse: true
                                        background: FieldBackground {}
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 5
                                    FieldLabel { text: I18n.t("IP / Host") }
                                    TextField {
                                        id: ipField
                                        Layout.fillWidth: true
                                        color: Theme.textPrimary
                                        placeholderText: "192.168.1.10"
                                        placeholderTextColor: Theme.textMuted
                                        selectByMouse: true
                                        background: FieldBackground {}
                                        onTextChanged: {
                                            if (!root.internalUpdate) {
                                                root.updateUrl()
                                                root.resetProbeStates()
                                            }
                                        }
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 5
                                    FieldLabel { text: I18n.t("Логин") }
                                    TextField {
                                        id: loginField
                                        Layout.fillWidth: true
                                        text: "root"
                                        color: Theme.textPrimary
                                        placeholderText: "root"
                                        placeholderTextColor: Theme.textMuted
                                        selectByMouse: true
                                        background: FieldBackground {}
                                        onTextChanged: {
                                            if (!root.internalUpdate) {
                                                root.updateUrl()
                                                root.resetProbeStates()
                                            }
                                        }
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 5
                                    FieldLabel { text: I18n.t("Пароль") }
                                    TextField {
                                        id: passwordField
                                        Layout.fillWidth: true
                                        echoMode: TextInput.Password
                                        color: Theme.textPrimary
                                        placeholderText: I18n.t("Пароль камеры")
                                        placeholderTextColor: Theme.textMuted
                                        selectByMouse: true
                                        background: FieldBackground {}
                                        onTextChanged: {
                                            if (!root.internalUpdate) {
                                                root.updateUrl()
                                                root.resetProbeStates()
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    ScrollView {
                        clip: true
                        contentWidth: availableWidth

                        ColumnLayout {
                            width: parent.width
                            spacing: 14

                            SectionHeader {
                                title: I18n.t("Параметры подключения")
                                subtitle: I18n.t("RTSP порт отвечает за видеопоток, HTTP/ONVIF порт — за Majestic WebUI, API и PTZ.")
                            }

                            FormGrid {
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 5
                                    FieldLabel { text: I18n.t("RTSP порт") }
                                    TextField {
                                        id: rtspPortField
                                        Layout.fillWidth: true
                                        text: "554"
                                        color: Theme.textPrimary
                                        selectByMouse: true
                                        validator: IntValidator { bottom: 1; top: 65535 }
                                        background: FieldBackground {}
                                        onTextChanged: {
                                            if (!root.internalUpdate) {
                                                root.updateUrl()
                                                root.resetProbeStates()
                                            }
                                        }
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 5
                                    FieldLabel { text: I18n.t("HTTP / Majestic / ONVIF порт") }
                                    TextField {
                                        id: httpOnvifPortField
                                        Layout.fillWidth: true
                                        text: "80"
                                        color: Theme.textPrimary
                                        selectByMouse: true
                                        validator: IntValidator { bottom: 1; top: 65535 }
                                        background: FieldBackground {}
                                        onTextChanged: {
                                            if (!root.internalUpdate) root.resetProbeStates()
                                        }
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 5
                                    FieldLabel { text: I18n.t("Шаблон RTSP URL") }
                                    StyledComboBox {
                                        id: urlTemplateCombo
                                        Layout.fillWidth: true
                                        model: root.rtspTemplateModel
                                        currentIndex: 0
                                        onCurrentIndexChanged: {
                                            if (!root.internalUpdate) {
                                                root.updateUrl()
                                                root.resetProbeStates()
                                            }
                                        }
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 5
                                    FieldLabel { text: I18n.t("Канал") }
                                    TextField {
                                        id: channelField
                                        Layout.fillWidth: true
                                        text: "1"
                                        color: Theme.textPrimary
                                        selectByMouse: true
                                        background: FieldBackground {}
                                        onTextChanged: {
                                            if (!root.internalUpdate) root.updateUrl()
                                        }
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 5
                                    FieldLabel { text: I18n.t("HD профиль / stream") }
                                    TextField {
                                        id: hdProfileField
                                        Layout.fillWidth: true
                                        text: "0"
                                        color: Theme.textPrimary
                                        selectByMouse: true
                                        background: FieldBackground {}
                                        onTextChanged: {
                                            if (!root.internalUpdate) root.updateUrl()
                                        }
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 5
                                    FieldLabel { text: I18n.t("SD профиль / stream") }
                                    TextField {
                                        id: sdProfileField
                                        Layout.fillWidth: true
                                        text: "1"
                                        color: Theme.textPrimary
                                        selectByMouse: true
                                        background: FieldBackground {}
                                        onTextChanged: {
                                            if (!root.internalUpdate) root.updateUrl()
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                color: Theme.cardBackground
                                border.color: Theme.panelBorder
                                radius: 10
                                implicitHeight: tipsLayout.implicitHeight + 24

                                ColumnLayout {
                                    id: tipsLayout
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 7

                                    Text {
                                        Layout.fillWidth: true
                                        text: I18n.t("Подсказка")
                                        color: Theme.textPrimary
                                        font.bold: true
                                        font.pixelSize: 14
                                    }

                                    Text {
                                        Layout.fillWidth: true
                                        text: I18n.t("OpenIPC: /stream=0 и /stream=1. XM/Xiongmai: user/password/channel/stream.sdp?real_stream, для XM Sofia пароль хэшируется. Hikvision, Dahua, Reolink, TP-Link и Uniview используют свои RTSP пути.")
                                        color: Theme.textSecondary
                                        wrapMode: Text.WordWrap
                                        font.pixelSize: 12
                                    }
                                }
                            }
                        }
                    }

                    ScrollView {
                        clip: true
                        contentWidth: availableWidth

                        ColumnLayout {
                            width: parent.width
                            spacing: 14

                            SectionHeader {
                                title: I18n.t("Проверка подключения")
                                subtitle: I18n.t("Проверки не сохраняют камеру — они только показывают, какие сервисы реально отвечают.")
                            }

                            ProbeCard {
                                title: "RTSP"
                                subtitle: I18n.t("Проверяет TCP-порт и отправляет RTSP OPTIONS по основному URL.")
                                state: root.rtspProbeState
                                message: root.rtspProbeMessage
                                buttonText: root.rtspProbeState === "running" ? I18n.t("Проверка…") : I18n.t("Проверить RTSP")
                                running: root.rtspProbeState === "running"
                                onRequested: root.probeRtsp()
                            }

                            ProbeCard {
                                title: "Majestic API"
                                subtitle: I18n.t("Проверяет HTTP endpoint /api/v1/config.json на порту камеры.")
                                state: root.majesticProbeState
                                message: root.majesticProbeMessage
                                buttonText: root.majesticProbeState === "running" ? I18n.t("Проверка…") : I18n.t("Проверить Majestic")
                                running: root.majesticProbeState === "running"
                                onRequested: root.probeMajestic()
                            }

                            ProbeCard {
                                title: "ONVIF / PTZ"
                                subtitle: I18n.t("Ищет ONVIF Device Service, Media profile и PTZ endpoint.")
                                state: root.onvifProbeState
                                message: root.onvifProbeMessage
                                buttonText: root.onvifProbeState === "running" ? I18n.t("Проверка…") : I18n.t("Проверить ONVIF/PTZ")
                                running: root.onvifProbeState === "running"
                                onRequested: root.probeOnvif()
                            }

                            Text {
                                Layout.fillWidth: true
                                text: I18n.t("Если RTSP доступен, но кадров потом нет — проверьте логин/пароль, путь потока и кодек. Для Majestic/OpenIPC полезно сразу открыть управление Majestic после сохранения.")
                                color: Theme.textMuted
                                wrapMode: Text.WordWrap
                                font.pixelSize: 12
                            }
                        }
                    }

                    ScrollView {
                        clip: true
                        contentWidth: availableWidth

                        ColumnLayout {
                            width: parent.width
                            spacing: 14

                            SectionHeader {
                                title: I18n.t("Потоки камеры")
                                subtitle: I18n.t("HD используется как основной поток, SD — как лёгкий поток для preview и плотных раскладок.")
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 6

                                FieldLabel { text: I18n.t("RTSP HD URL") }
                                TextField {
                                    id: hdUrlField
                                    Layout.fillWidth: true
                                    readOnly: urlTemplateCombo.currentText !== "Custom"
                                    color: readOnly ? Theme.textSecondary : Theme.textPrimary
                                    selectByMouse: true
                                    background: FieldBackground {}
                                    onTextChanged: {
                                        if (!root.internalUpdate) root.resetProbeStates()
                                    }
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 6

                                FieldLabel { text: I18n.t("RTSP SD URL") }
                                TextField {
                                    id: sdUrlField
                                    Layout.fillWidth: true
                                    readOnly: urlTemplateCombo.currentText !== "Custom"
                                    color: readOnly ? Theme.textSecondary : Theme.textPrimary
                                    selectByMouse: true
                                    background: FieldBackground {}
                                    onTextChanged: {
                                        if (!root.internalUpdate) root.resetProbeStates()
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                color: Theme.cardBackground
                                border.color: Theme.panelBorder
                                radius: 10
                                implicitHeight: summaryLayout.implicitHeight + 24

                                ColumnLayout {
                                    id: summaryLayout
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 8

                                    Text {
                                        Layout.fillWidth: true
                                        text: I18n.t("Итог")
                                        color: Theme.textPrimary
                                        font.bold: true
                                        font.pixelSize: 15
                                    }

                                    Text {
                                        Layout.fillWidth: true
                                        text: root.summaryText()
                                        color: Theme.textSecondary
                                        wrapMode: Text.WordWrap
                                        font.pixelSize: 12
                                    }

                                    Text {
                                        Layout.fillWidth: true
                                        text: I18n.t("После сохранения камеру можно добавить в раскладку, открыть Majestic и запустить проверку здоровья из сайдбара.")
                                        color: Theme.textMuted
                                        wrapMode: Text.WordWrap
                                        font.pixelSize: 11
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 62
            color: Theme.topBarBackground

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 18
                anchors.rightMargin: 18
                spacing: 10

                WizardButton {
                    text: I18n.t("Назад")
                    enabled: root.stepIndex > 0
                    onClicked: root.stepIndex = Math.max(0, root.stepIndex - 1)
                }

                WizardButton {
                    text: I18n.t("Далее")
                    visible: root.stepIndex < root.stepCount - 1
                    onClicked: root.stepIndex = Math.min(root.stepCount - 1, root.stepIndex + 1)
                }

                Item { Layout.fillWidth: true }

                Text {
                    Layout.maximumWidth: 360
                    text: root.formError
                    visible: text !== ""
                    color: Theme.danger
                    elide: Text.ElideRight
                    font.pixelSize: 12
                }

                WizardButton {
                    text: I18n.t("Отмена")
                    onClicked: root.close()
                }

                WizardButton {
                    text: root.isEditMode ? I18n.t("Сохранить") : I18n.t("Добавить камеру")
                    primary: true
                    enabled: root.canSave()
                    onClicked: root.saveCamera()
                }
            }
        }
    }

    Connections {
        target: SystemController

        function onCameraEndpointProbeFinished(requestId, kind, host, port, success, message, httpStatus, elapsedMs) {
            var text = I18n.t(message)
            if (httpStatus > 0) {
                text += " · HTTP " + httpStatus
            }
            if (elapsedMs > 0) {
                text += " · " + elapsedMs + " " + I18n.t("мс")
            }
            if (requestId === root.rtspProbeId) {
                root.rtspProbeState = success ? "ok" : "fail"
                root.rtspProbeMessage = text
                root.rtspProbeId = ""
            } else if (requestId === root.majesticProbeId) {
                root.majesticProbeState = success ? "ok" : "fail"
                root.majesticProbeMessage = text
                root.majesticProbeId = ""
            }
        }
    }

    Connections {
        target: SystemController.ptzController

        function onDiscoveryStarted(ip, port) {
            if (root.matchesCurrentCamera(ip, port)) {
                root.onvifProbeState = "running"
                root.onvifProbeMessage = I18n.t("Проверка ONVIF/PTZ…")
            }
        }

        function onDiscoveryFinished(ip, port, success, message, profileToken, ptzUrl, imagingUrl) {
            if (!root.matchesCurrentCamera(ip, port)) {
                return
            }
            root.onvifProbeState = success ? "ok" : "fail"
            root.onvifProbeMessage = success
                ? I18n.t(message) + (profileToken ? " · " + I18n.t("Профиль: ") + profileToken : "")
                : I18n.t(message)
        }
    }

    function canSave() {
        return ipField.text.trim().length > 0
            && parsePort(rtspPortField.text, 0) > 0
            && parsePort(httpOnvifPortField.text, 0) > 0
            && hdUrlField.text.trim().length > 0
    }

    function validateForm() {
        formError = ""
        if (ipField.text.trim() === "") {
            formError = I18n.t("Укажите IP или host камеры")
            stepIndex = 0
            return false
        }
        if (parsePort(rtspPortField.text, 0) <= 0) {
            formError = I18n.t("Укажите корректный RTSP порт")
            stepIndex = 1
            return false
        }
        if (parsePort(httpOnvifPortField.text, 0) <= 0) {
            formError = I18n.t("Укажите корректный HTTP/ONVIF порт")
            stepIndex = 1
            return false
        }
        if (hdUrlField.text.trim() === "") {
            formError = I18n.t("Укажите RTSP HD URL")
            stepIndex = 3
            return false
        }
        return true
    }

    function saveCamera() {
        if (!validateForm()) {
            return
        }

        var displayName = nameField.text.trim()
        if (displayName === "") {
            displayName = ipField.text.trim()
        }

        if (isEditMode) {
            SystemController.updateCamera(
                editIndex,
                displayName,
                ipField.text.trim(),
                hdUrlField.text.trim(),
                parsePort(rtspPortField.text, 554),
                parsePort(httpOnvifPortField.text, 80),
                loginField.text.trim(),
                passwordField.text,
                sdUrlField.text.trim()
            )
        } else {
            SystemController.addManualCamera(
                displayName,
                ipField.text.trim(),
                hdUrlField.text.trim(),
                parsePort(rtspPortField.text, 554),
                parsePort(httpOnvifPortField.text, 80),
                loginField.text.trim(),
                passwordField.text,
                sdUrlField.text.trim()
            )
        }
        root.close()
    }

    function importPayload() {
        var camera = SystemController.parseCameraQrPayload(qrPayloadField.text)
        if (!camera.valid) {
            formError = I18n.t(camera.error || "Invalid QR payload")
            return
        }

        internalUpdate = true
        formError = ""
        nameField.text = camera.name || nameField.text
        ipField.text = camera.ip
        rtspPortField.text = String(camera.port || 554)
        httpOnvifPortField.text = String(camera.onvifPort || 80)
        loginField.text = camera.login || "root"
        passwordField.text = camera.password || ""
        urlTemplateCombo.currentIndex = 3
        hdUrlField.text = camera.hdStreamUrl || ""
        sdUrlField.text = camera.sdStreamUrl || ""
        internalUpdate = false
        resetProbeStates()
        stepIndex = 2
    }

    function probeRtsp() {
        if (!validateProbeInputs()) {
            return
        }
        rtspProbeState = "running"
        rtspProbeMessage = I18n.t("Проверка RTSP…")
        rtspProbeId = SystemController.probeCameraEndpoint(
            "rtsp",
            ipField.text.trim(),
            parsePort(rtspPortField.text, 554),
            pathFromRtspUrl(hdUrlField.text),
            loginField.text.trim(),
            passwordField.text
        )
    }

    function probeMajestic() {
        if (!validateProbeInputs()) {
            return
        }
        majesticProbeState = "running"
        majesticProbeMessage = I18n.t("Проверка Majestic API…")
        majesticProbeId = SystemController.probeCameraEndpoint(
            "majestic",
            ipField.text.trim(),
            parsePort(httpOnvifPortField.text, 80),
            "/api/v1/config.json",
            loginField.text.trim(),
            passwordField.text
        )
    }

    function probeOnvif() {
        if (!validateProbeInputs()) {
            return
        }
        onvifProbeState = "running"
        onvifProbeMessage = I18n.t("Проверка ONVIF/PTZ…")
        SystemController.ptzController.probe(
            ipField.text.trim(),
            parsePort(httpOnvifPortField.text, 80),
            loginField.text.trim(),
            passwordField.text
        )
    }

    function validateProbeInputs() {
        if (ipField.text.trim() === "") {
            formError = I18n.t("Укажите IP или host камеры")
            stepIndex = 0
            return false
        }
        formError = ""
        return true
    }

    function resetProbeStates() {
        rtspProbeId = ""
        majesticProbeId = ""
        rtspProbeState = "idle"
        majesticProbeState = "idle"
        onvifProbeState = "idle"
        rtspProbeMessage = I18n.t("Проверка ещё не запускалась")
        majesticProbeMessage = I18n.t("Проверка ещё не запускалась")
        onvifProbeMessage = I18n.t("Проверка ещё не запускалась")
    }

    function matchesCurrentCamera(ip, port) {
        return ip === ipField.text.trim() && port === parsePort(httpOnvifPortField.text, 80)
    }

    function parsePort(value, fallback) {
        var parsed = parseInt(value)
        if (isNaN(parsed) || parsed < 1 || parsed > 65535) {
            return fallback
        }
        return parsed
    }

    function detectTemplateIndex(url) {
        var text = String(url || "")
        if (text.indexOf("user=") >= 0 && text.indexOf("real_stream") >= 0) return 1
        if (text.indexOf("/Streaming/Channels/") >= 0) return 3
        if (text.indexOf("/cam/realmonitor") >= 0) return 4
        if (text.indexOf("/h264Preview_") >= 0) return 5
        if (text.indexOf("/stream1") >= 0 || text.indexOf("/stream2") >= 0) return 6
        if (text.indexOf("/media/video") >= 0) return 7
        if (text.indexOf("/stream=") >= 0 || text === "") return 0
        return root.rtspTemplateModel.length - 1
    }

    function currentTemplate() {
        return urlTemplateCombo.currentText || "OpenIPC"
    }

    function xmSafe(value, fallback) {
        var text = String(value || "").trim()
        if (text.length === 0 && fallback !== undefined) {
            text = String(fallback)
        }
        return encodeURIComponent(text)
    }

    function sofiaHash(password) {
        return SystemController.xmSofiaPasswordHash(String(password || ""))
    }

    function generateRtspUrl(ip, port, stream, template) {
        var host = String(ip || "").trim()
        var rtspPort = parsePort(port, 554)
        var baseUrl = "rtsp://" + host + ":" + rtspPort

        if (template === "XM / Xiongmai" || template === "XM / Sofia") {
            var user = xmSafe(loginField.text, "admin")
            var password = template === "XM / Sofia"
                    ? sofiaHash(passwordField.text)
                    : xmSafe(passwordField.text, "")
            var channel = xmSafe(channelField.text, "1")
            var xmStream = xmSafe(stream, "0")
            return baseUrl + "/user=" + user
                    + "&password=" + password
                    + "&channel=" + channel
                    + "&stream=" + xmStream + ".sdp?real_stream"
        }

        if (template === "Hikvision") {
            var value = parseInt(stream)
            if (value > 9) {
                return baseUrl + "/Streaming/Channels/" + value
            }
            var channel = channelField.text || "1"
            var profile = isNaN(value) ? 1 : value + 1
            return baseUrl + "/Streaming/Channels/" + channel + "0" + profile
        }

        if (template === "Dahua") {
            var ch = channelField.text || "1"
            return baseUrl + "/cam/realmonitor?channel=" + ch + "&subtype=" + stream
        }

        if (template === "Reolink") {
            return baseUrl + (String(stream) === "1" ? "/h264Preview_01_sub" : "/h264Preview_01_main")
        }

        if (template === "TP-Link") {
            return baseUrl + (String(stream) === "1" ? "/stream2" : "/stream1")
        }

        if (template === "Uniview") {
            return baseUrl + (String(stream) === "1" ? "/media/video2" : "/media/video1")
        }

        return baseUrl + "/stream=" + stream
    }

    function updateUrl() {
        if (urlTemplateCombo.currentText === "Custom") {
            return
        }
        var template = currentTemplate()
        hdUrlField.text = generateRtspUrl(ipField.text, rtspPortField.text, hdProfileField.text, template)
        sdUrlField.text = generateRtspUrl(ipField.text, rtspPortField.text, sdProfileField.text, template)
    }

    function pathFromRtspUrl(value) {
        var text = String(value || "")
        var marker = text.indexOf("://")
        var pathStart = marker >= 0 ? text.indexOf("/", marker + 3) : text.indexOf("/")
        if (pathStart < 0) {
            return "/stream=0"
        }
        return text.slice(pathStart)
    }

    function summaryText() {
        var name = nameField.text.trim() || ipField.text.trim()
        return I18n.t("Камера: %1 · RTSP %2 · HTTP/ONVIF %3 · шаблон %4",
                      [name, parsePort(rtspPortField.text, 554), parsePort(httpOnvifPortField.text, 80), currentTemplate()])
    }

    function probeColor(state) {
        if (state === "ok") return Theme.success
        if (state === "fail") return Theme.danger
        if (state === "running") return Theme.warning
        return Theme.textMuted
    }

    function probeTitle(state) {
        if (state === "ok") return I18n.t("Готово")
        if (state === "fail") return I18n.t("Ошибка")
        if (state === "running") return I18n.t("Проверка…")
        return I18n.t("Не проверено")
    }

    component SectionHeader: ColumnLayout {
        property string title: ""
        property string subtitle: ""

        Layout.fillWidth: true
        spacing: 4

        Text {
            Layout.fillWidth: true
            text: parent.title
            color: Theme.textPrimary
            font.bold: true
            font.pixelSize: 22
        }

        Text {
            Layout.fillWidth: true
            text: parent.subtitle
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            font.pixelSize: 13
        }
    }

    component FieldLabel: Text {
        color: Theme.textSecondary
        font.family: Theme.metroFontFamily
        font.pixelSize: 12
        font.bold: true
    }

    component FieldBackground: Rectangle {
        color: Theme.controlBackground
        border.color: Theme.metroStroke
        radius: Theme.metroTileRadius
    }

    component FormGrid: GridLayout {
        columns: 2
        columnSpacing: 14
        rowSpacing: 12
        Layout.fillWidth: true
    }

    component WizardButton: Button {
        id: button
        property bool primary: false
        property bool danger: false

        implicitHeight: 38
        implicitWidth: Math.max(96, contentItem.implicitWidth + 28)
        font.bold: primary
        font.family: Theme.metroFontFamily

        background: Rectangle {
            radius: Theme.metroTileRadius
            color: !button.enabled
                   ? Theme.controlBackgroundAlt
                   : button.primary
                      ? (button.down ? Theme.metroBlueHover : (button.hovered ? Theme.metroBlueHover : Theme.metroBlue))
                     : button.danger
                        ? Theme.metroRed
                        : (button.hovered ? Theme.metroTileHover : Theme.metroTile)
            border.color: button.primary ? Theme.metroBlue : (button.hovered ? Theme.metroStrokeStrong : Theme.metroStroke)
            border.width: button.hovered || button.visualFocus ? 2 : 1
        }

        contentItem: Text {
            text: button.text
            color: button.enabled ? Theme.textPrimary : Theme.textMuted
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font: button.font
            elide: Text.ElideRight
        }
    }

    component ProbeCard: Rectangle {
        id: probeCard
        property string title: ""
        property string subtitle: ""
        property string state: "idle"
        property string message: ""
        property string buttonText: ""
        property bool running: false
        signal requested()

        Layout.fillWidth: true
        implicitHeight: probeLayout.implicitHeight + 24
        color: Theme.cardBackground
        border.color: probeCard.state === "ok" ? Theme.success
                    : probeCard.state === "fail" ? Theme.danger
                    : Theme.panelBorder
        radius: 10

        RowLayout {
            id: probeLayout
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            Rectangle {
                Layout.preferredWidth: 12
                Layout.preferredHeight: 12
                radius: 6
                color: root.probeColor(probeCard.state)
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: probeCard.title
                        color: Theme.textPrimary
                        font.bold: true
                        font.pixelSize: 15
                    }

                    Rectangle {
                        radius: 8
                        color: root.probeColor(probeCard.state)
                        opacity: 0.22
                        Layout.preferredHeight: 22
                        Layout.preferredWidth: probeStatusText.implicitWidth + 18

                        Text {
                            id: probeStatusText
                            anchors.centerIn: parent
                            text: root.probeTitle(probeCard.state)
                            color: Theme.textPrimary
                            font.bold: true
                            font.pixelSize: 10
                        }
                    }
                }

                Text {
                    Layout.fillWidth: true
                    text: probeCard.subtitle
                    color: Theme.textSecondary
                    wrapMode: Text.WordWrap
                    font.pixelSize: 12
                }

                Text {
                    Layout.fillWidth: true
                    text: probeCard.message
                    color: root.probeColor(probeCard.state)
                    wrapMode: Text.WordWrap
                    font.pixelSize: 11
                }
            }

            WizardButton {
                text: probeCard.buttonText
                enabled: !probeCard.running
                primary: probeCard.state === "idle" || probeCard.state === "fail"
                onClicked: probeCard.requested()
            }
        }
    }
}
