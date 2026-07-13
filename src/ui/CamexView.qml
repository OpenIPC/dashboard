import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import OpenIPC

Dialog {
    id: root
    parent: Overlay.overlay
    modal: true
    dim: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    readonly property real hostWidth: parent && parent.width > 0 ? parent.width : 1280
    readonly property real hostHeight: parent && parent.height > 0 ? parent.height : 720
    readonly property real dialogMargin: hostWidth < 760 || hostHeight < 560 ? 24 : 48

    x: Math.max(0, (hostWidth - width) / 2)
    y: Math.max(0, (hostHeight - height) / 2)
    width: Math.max(320, Math.min(1320, hostWidth - dialogMargin))
    height: Math.max(360, Math.min(880, hostHeight - dialogMargin))

    property string binaryName: "camex"
    property string serverHost: "vpn.example.org"
    property int serverPort: 5800
    property string bindIp: "0.0.0.0"
    property string transport: "udp"
    property bool encrypt: true
    property string psk: "change-me-strong-key"
    property string clientPsk: ""
    property string serverCidr: "10.0.0.1/24"
    property string gatewayIp: "10.0.0.1"
    property string clientId: "OPENIPC-CAMERA-01"
    property string clientCidr: "10.0.0.2/24"
    property string routeCidrs: "192.168.100.0/24"
    property string configPath: "/etc/camex/camex.conf"
    property string tunDev: ""
    property string bindDev: ""
    property bool autoMode: true
    property int mtu: 1500
    property string checkMessage: ""
    property bool checkOk: false

    function settingsMap() {
        return {
            binary: binaryName,
            serverHost: serverHost,
            port: serverPort,
            bindIp: bindIp,
            transport: transport,
            encrypt: encrypt,
            psk: psk,
            clientPsk: clientPsk,
            serverCidr: serverCidr,
            gatewayIp: gatewayIp,
            clientId: clientId,
            clientCidr: clientCidr,
            routeCidrs: routeCidrs,
            configPath: configPath,
            tunDev: tunDev,
            bindDev: bindDev,
            autoMode: autoMode,
            mtu: mtu
        }
    }

    function serverCommand() {
        return SystemController.camexController.buildServerCommand(settingsMap())
    }

    function clientCommand() {
        return SystemController.camexController.buildClientCommand(settingsMap())
    }

    function serverConfig() {
        return SystemController.camexController.buildServerConfig(settingsMap())
    }

    function copyText(value) {
        SystemController.copyTextToClipboard(value)
    }

    function localizedTcpCheckMessage(result) {
        var raw = result.message || ""
        if (raw === "TCP check was not started.")
            return I18n.t("TCP-проверка не запускалась.")
        if (raw === "Specify a valid host and port.")
            return I18n.t("Укажите корректный host и port.")
        if (raw.indexOf("TCP port is reachable in ") === 0)
            return I18n.t("TCP-порт доступен за %1 мс.", [result.elapsedMs || raw.replace(/[^0-9]/g, "")])
        if (raw.indexOf("TCP port is not reachable: ") === 0)
            return I18n.t("TCP-порт недоступен: ") + raw.substring("TCP port is not reachable: ".length)
        return I18n.t(raw)
    }

    background: Rectangle {
        color: Theme.metroSidebarBackground
        radius: Theme.metroTileRadius
        border.color: Theme.metroStroke
        border.width: 1
    }

    header: Rectangle {
        height: 58
        color: "transparent"

        ColumnLayout {
            anchors.left: parent.left
            anchors.right: closeButton.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 16
            anchors.rightMargin: 12
            spacing: 2

            Text {
                text: "Camex"
                color: Theme.textPrimary
                font.pixelSize: 19
                font.bold: true
            }

            Text {
                text: I18n.t("Удаленный доступ к OpenIPC-камерам через легкий UDP/TCP-туннель")
                color: Theme.textMuted
                font.pixelSize: 11
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        MetroWindowButton {
            id: closeButton
            kind: "close"
            width: 32
            height: 32
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 10
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

    component PrimaryButton: Button {
        property color buttonColor: Theme.controlBackground
        property color hoverColor: Theme.cardHover

        implicitHeight: 34
        leftPadding: 14
        rightPadding: 14

        background: Rectangle {
            color: parent.enabled ? (parent.hovered ? parent.hoverColor : parent.buttonColor) : Theme.controlBackgroundAlt
            border.color: parent.hovered ? Theme.metroBlue : Theme.metroStroke
            border.width: 1
            radius: Theme.metroTileRadius
        }

        contentItem: Text {
            text: parent.text
            color: parent.enabled ? Theme.textPrimary : Theme.textMuted
            font.pixelSize: 12
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    component StyledCheckBox: MetroCheckBox {
        implicitHeight: 28
        leftPadding: 0
        rightPadding: 8
    }

    component FieldRow: ColumnLayout {
        property alias label: labelItem.text
        property alias text: input.text
        property string placeholderText: ""
        property bool readOnly: false

        spacing: 5
        Layout.fillWidth: true

        Text {
            id: labelItem
            color: Theme.textMuted
            font.pixelSize: 11
            Layout.fillWidth: true
            elide: Text.ElideRight
        }

        TextField {
            id: input
            Layout.fillWidth: true
            implicitHeight: 34
            readOnly: parent.readOnly
            placeholderText: parent.placeholderText
            color: Theme.textPrimary
            placeholderTextColor: Theme.textFaint
            selectionColor: Theme.accent
            selectedTextColor: Theme.textPrimary
            font.pixelSize: 13
            background: Rectangle {
                color: Theme.controlBackground
                border.color: parent.activeFocus ? Theme.metroBlue : Theme.metroStroke
                border.width: 1
                radius: Theme.metroTileRadius
            }
        }
    }

    component SectionPanel: Rectangle {
        property alias title: titleItem.text
        property alias subtitle: subtitleItem.text
        default property alias content: body.data

        Layout.fillWidth: true
        color: Theme.metroSurface
        border.color: Theme.metroStroke
        border.width: 1
        radius: Theme.metroTileRadius
        implicitHeight: headerColumn.implicitHeight + body.implicitHeight + 30

        ColumnLayout {
            id: panelLayout
            anchors.fill: parent
            anchors.margins: 12
            spacing: 10

            ColumnLayout {
                id: headerColumn
                Layout.fillWidth: true
                spacing: 2

                Text {
                    id: titleItem
                    color: Theme.textPrimary
                    font.pixelSize: 15
                    font.bold: true
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }

                Text {
                    id: subtitleItem
                    color: Theme.textMuted
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }

            ColumnLayout {
                id: body
                Layout.fillWidth: true
                spacing: 8
            }
        }
    }

    component CodePanel: Rectangle {
        property string title: ""
        property string value: ""

        Layout.fillWidth: true
        implicitHeight: 160
        color: Theme.controlBackgroundAlt
        border.color: Theme.controlBorder
        border.width: 1
        radius: Theme.radiusLg

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 8

            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: parent.parent.parent.title
                    color: Theme.textPrimary
                    font.pixelSize: 12
                    font.bold: true
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }

                PrimaryButton {
                    text: I18n.t("Копировать")
                    Layout.preferredWidth: 118
                    onClicked: root.copyText(parent.parent.parent.value)
                }
            }

            TextArea {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: parent.parent.value
                readOnly: true
                wrapMode: TextEdit.Wrap
                color: Theme.textSecondary
                selectedTextColor: Theme.textPrimary
                selectionColor: Theme.accent
                font.family: "Consolas"
                font.pixelSize: 11
                background: Rectangle {
                    color: Theme.controlBackground
                    border.color: Theme.panelBorder
                    border.width: 1
                    radius: Theme.radiusMd
                }
            }
        }
    }

    FileDialog {
        id: saveConfigDialog
        title: I18n.t("Сохранить camex.conf")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "conf"
        nameFilters: [I18n.t("Конфиг Camex (*.conf)"), I18n.t("Все файлы (*)")]
        onAccepted: {
            var ok = SystemController.camexController.saveTextFile(selectedFile.toString(), root.serverConfig())
            checkOk = ok
            checkMessage = ok ? I18n.t("Файл конфигурации сохранен.") : I18n.t("Не удалось сохранить файл конфигурации.")
        }
    }

    contentItem: ColumnLayout {
        spacing: 0

        TabBar {
            id: tabs
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            leftPadding: 8
            rightPadding: 8
            topPadding: 6
            bottomPadding: 6
            spacing: 6
            background: Rectangle {
                color: Theme.metroBackground
                border.color: Theme.metroStroke
                border.width: 1
                radius: Theme.metroTileRadius
            }

            component CamexTabButton: TabButton {
                id: tabButton

                implicitWidth: 130
                implicitHeight: 32

                background: Rectangle {
                    color: tabButton.checked ? Theme.metroSurfaceAlt : (tabButton.hovered ? Theme.metroTileHover : "transparent")
                    radius: Theme.metroTileRadius
                    border.color: tabButton.checked ? Theme.metroBlue : "transparent"
                    border.width: tabButton.checked ? 1 : 0

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: 2
                        radius: 1
                        color: tabButton.checked ? Theme.metroBlue : "transparent"
                    }
                }

                contentItem: Text {
                    text: tabButton.text
                    color: tabButton.checked ? Theme.metroBlue : Theme.textMuted
                    font.pixelSize: 13
                    font.bold: tabButton.checked
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }

            CamexTabButton { text: I18n.t("Мастер") }
            CamexTabButton { text: I18n.t("Сервер") }
            CamexTabButton { text: I18n.t("Камера") }
            CamexTabButton { text: I18n.t("Проверка") }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabs.currentIndex

            ScrollView {
                id: wizardScroll
                clip: true
                contentWidth: availableWidth
                ScrollBar.vertical: StyledScrollBar {}

                ColumnLayout {
                    x: 10
                    width: Math.max(0, wizardScroll.availableWidth - 20)
                    spacing: 10

                    SectionPanel {
                        title: I18n.t("Что делает Camex")
                        subtitle: I18n.t("Camex создает сетевой туннель между камерой и вашим сервером. Камера сама подключается наружу, а Dashboard затем может обращаться к ней через tunnel IP.")

                        Text {
                            Layout.fillWidth: true
                            color: Theme.textSecondary
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                            text: I18n.t("Типовой сценарий: у вас есть VPS с публичным IP, камера OpenIPC находится за NAT или LTE, а Camex связывает их в маленькую приватную сеть. После этого можно проверять SSH, Web UI, RTSP или ONVIF уже через адрес туннеля.")
                        }
                    }

                    SectionPanel {
                        title: I18n.t("Порядок настройки")
                        subtitle: I18n.t("Идите по шагам сверху вниз. Справа на вкладках приложение уже подготовит команды и конфиг под введенные параметры.")

                        Repeater {
                            model: [
                                I18n.t("1. Подготовьте VPS или сервер с публичным IP. Откройте порт %1 для %2.", [root.serverPort, root.transport.toUpperCase()]),
                                I18n.t("2. Скачайте Camex для архитектуры сервера и камеры из релизов OpenIPC/camex или соберите его командой make."),
                                I18n.t("3. На сервере создайте %1 из вкладки Сервер и запустите camex server.", [root.configPath]),
                                I18n.t("4. На камере запустите camex client из вкладки Камера. Для OpenIPC чаще всего нужен SSH-доступ к камере."),
                                I18n.t("5. Проверьте, что появился tunnel IP %1. После этого добавьте камеру в Dashboard по этому IP или проверьте RTSP/SSH вручную.", [root.clientCidr.split('/')[0]]),
                                I18n.t("6. Когда все работает, оформите camex как сервис автозапуска на сервере и на камере.")
                            ]

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Rectangle {
                                    Layout.preferredWidth: 8
                                    Layout.fillHeight: true
                                    radius: 4
                                    color: Theme.accent
                                }

                                Text {
                                    text: modelData
                                    color: Theme.textSecondary
                                    font.pixelSize: 13
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true

                            PrimaryButton {
                                text: I18n.t("Открыть релизы Camex")
                                buttonColor: Theme.accent
                                hoverColor: Theme.accentHover
                                onClicked: Qt.openUrlExternally("https://github.com/OpenIPC/camex/releases")
                            }

                            PrimaryButton {
                                text: I18n.t("Открыть репозиторий")
                                onClicked: Qt.openUrlExternally("https://github.com/OpenIPC/camex")
                            }
                        }
                    }

                    SectionPanel {
                        title: I18n.t("Общие параметры туннеля")
                        subtitle: I18n.t("Эти значения используются и для server-команды, и для client-команды.")

                        GridLayout {
                            Layout.fillWidth: true
                            columns: root.width > 1200 ? 3 : 2
                            columnSpacing: 10
                            rowSpacing: 10

                            FieldRow { label: I18n.t("Бинарный файл Camex"); text: root.binaryName; onTextChanged: root.binaryName = text }
                            FieldRow { label: I18n.t("Адрес сервера"); text: root.serverHost; onTextChanged: root.serverHost = text }
                            FieldRow {
                                label: I18n.t("Порт")
                                text: String(root.serverPort)
                                onTextChanged: {
                                    var v = parseInt(text)
                                    root.serverPort = isNaN(v) ? 5800 : Math.max(1, Math.min(65535, v))
                                }
                            }
                            FieldRow { label: I18n.t("CIDR туннеля сервера"); text: root.serverCidr; onTextChanged: root.serverCidr = text }
                            FieldRow { label: I18n.t("IP шлюза"); text: root.gatewayIp; onTextChanged: root.gatewayIp = text }
                            FieldRow { label: I18n.t("Путь к конфигу"); text: root.configPath; onTextChanged: root.configPath = text }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 16

                            Text {
                                text: I18n.t("Транспорт")
                                color: Theme.textMuted
                                font.pixelSize: 12
                            }

                            StyledComboBox {
                                model: ["udp", "tcp"]
                                currentIndex: root.transport === "tcp" ? 1 : 0
                                onCurrentTextChanged: root.transport = currentText
                                Layout.preferredWidth: 130
                            }

                            StyledCheckBox {
                                text: I18n.t("Шифрование")
                                checked: root.encrypt
                                onToggled: root.encrypt = checked
                            }

                            FieldRow {
                                label: "PSK"
                                text: root.psk
                                onTextChanged: root.psk = text
                            }
                        }
                    }
                }
            }

            ScrollView {
                id: serverScroll
                clip: true
                contentWidth: availableWidth
                ScrollBar.vertical: StyledScrollBar {}

                ColumnLayout {
                    x: 10
                    width: Math.max(0, serverScroll.availableWidth - 20)
                    spacing: 10

                    SectionPanel {
                        title: I18n.t("Настройка сервера")
                        subtitle: I18n.t("Сервер должен иметь публичный IP или DNS-имя. Он принимает подключения камер и раздает им параметры туннеля.")

                        GridLayout {
                            Layout.fillWidth: true
                            columns: 2
                            columnSpacing: 10
                            rowSpacing: 10

                            FieldRow { label: I18n.t("IP привязки"); text: root.bindIp; onTextChanged: root.bindIp = text }
                            FieldRow { label: I18n.t("Сетевой интерфейс привязки (необязательно)"); text: root.bindDev; placeholderText: "eth0"; onTextChanged: root.bindDev = text }
                            FieldRow { label: I18n.t("TUN-устройство (необязательно)"); text: root.tunDev; placeholderText: I18n.t("/dev/camex или /dev/net/tun"); onTextChanged: root.tunDev = text }
                            FieldRow {
                                label: "MTU"
                                text: String(root.mtu)
                                onTextChanged: {
                                    var v = parseInt(text)
                                    root.mtu = isNaN(v) ? 1500 : Math.max(576, Math.min(9000, v))
                                }
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            color: Theme.textMuted
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                            text: I18n.t("Если на сервере нет стандартного /dev/net/tun, можно собрать и загрузить camex.ko. В большинстве обычных VPS достаточно стандартного TUN.")
                        }
                    }

                    CodePanel {
                        title: I18n.t("Команда сервера")
                        value: root.serverCommand()
                    }

                    CodePanel {
                        title: "camex.conf"
                        implicitHeight: 260
                        value: root.serverConfig()
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        PrimaryButton {
                            text: I18n.t("Сохранить camex.conf")
                            buttonColor: Theme.accent
                            hoverColor: Theme.accentHover
                            onClicked: saveConfigDialog.open()
                        }
                        PrimaryButton {
                            text: I18n.t("Копировать конфиг")
                            onClicked: root.copyText(root.serverConfig())
                        }
                        Item { Layout.fillWidth: true }
                    }
                }
            }

            ScrollView {
                id: clientScroll
                clip: true
                contentWidth: availableWidth
                ScrollBar.vertical: StyledScrollBar {}

                ColumnLayout {
                    x: 10
                    width: Math.max(0, clientScroll.availableWidth - 20)
                    spacing: 10

                    SectionPanel {
                        title: I18n.t("Настройка камеры")
                        subtitle: I18n.t("Клиент запускается на камере. В auto mode камера получает адрес, gateway и маршруты из server config.")

                        GridLayout {
                            Layout.fillWidth: true
                            columns: 2
                            columnSpacing: 10
                            rowSpacing: 10

                            FieldRow {
                                label: "Client ID"
                                text: root.clientId
                                onTextChanged: root.clientId = SystemController.camexController.normalizeClientId(text)
                            }
                            FieldRow { label: I18n.t("CIDR туннеля камеры"); text: root.clientCidr; onTextChanged: root.clientCidr = text }
                            FieldRow { label: I18n.t("Маршруты за камерой"); text: root.routeCidrs; placeholderText: "192.168.100.0/24"; onTextChanged: root.routeCidrs = text }
                            FieldRow { label: I18n.t("PSK клиента (необязательно)"); text: root.clientPsk; placeholderText: I18n.t("оставьте пустым для общего PSK"); onTextChanged: root.clientPsk = text }
                        }

                        StyledCheckBox {
                            text: I18n.t("Auto mode: параметры камеры берутся с сервера")
                            checked: root.autoMode
                            onToggled: root.autoMode = checked
                        }

                        Text {
                            Layout.fillWidth: true
                            color: Theme.textMuted
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                            text: I18n.t("Client ID должен совпадать с секцией [client ID] в camex.conf. Удобно использовать MAC, серийный номер или понятное имя камеры без пробелов.")
                        }
                    }

                    CodePanel {
                        title: I18n.t("Команда клиента для камеры")
                        value: root.clientCommand()
                    }

                    SectionPanel {
                        title: I18n.t("Как запустить на камере")
                        subtitle: I18n.t("Минимальный ручной сценарий через SSH.")

                        Text {
                            Layout.fillWidth: true
                            color: Theme.textSecondary
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                            text: I18n.t("1. Скопируйте camex на камеру в /usr/bin/camex или /tmp/camex. 2. Дайте права chmod +x. 3. Выполните client command выше. 4. Если команда работает, добавьте ее в автозапуск OpenIPC или init-скрипт вашей прошивки.")
                        }
                    }
                }
            }

            ScrollView {
                id: checkScroll
                clip: true
                contentWidth: availableWidth
                ScrollBar.vertical: StyledScrollBar {}

                ColumnLayout {
                    x: 10
                    width: Math.max(0, checkScroll.availableWidth - 20)
                    spacing: 10

                    SectionPanel {
                        title: I18n.t("Проверка")
                        subtitle: I18n.t("Эти проверки помогают понять, где остановилась настройка. UDP напрямую не проверяется обычным TCP-connect, поэтому TCP-проверка полезна только если вы выбрали TCP transport или открыли TCP-порт для диагностики.")

                        GridLayout {
                            Layout.fillWidth: true
                            columns: 2
                            columnSpacing: 10
                            rowSpacing: 10

                            FieldRow { label: I18n.t("Адрес сервера"); text: root.serverHost; onTextChanged: root.serverHost = text }
                            FieldRow {
                                label: I18n.t("Порт сервера")
                                text: String(root.serverPort)
                                onTextChanged: {
                                    var v = parseInt(text)
                                    root.serverPort = isNaN(v) ? 5800 : Math.max(1, Math.min(65535, v))
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true

                            PrimaryButton {
                                text: I18n.t("Проверить TCP-порт")
                                buttonColor: Theme.accent
                                hoverColor: Theme.accentHover
                                onClicked: {
                                    var result = SystemController.camexController.checkTcpPort(root.serverHost, root.serverPort, 1800)
                                    root.checkOk = result.ok === true
                                    root.checkMessage = root.localizedTcpCheckMessage(result)
                                }
                            }

                            PrimaryButton {
                                text: I18n.t("Скопировать команды")
                                onClicked: root.copyText(root.serverCommand() + "\n\n" + root.clientCommand())
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: 46
                            color: root.checkMessage === "" ? Theme.controlBackgroundAlt : (root.checkOk ? "#132b1b" : "#2b1717")
                            border.color: root.checkMessage === "" ? Theme.controlBorder : (root.checkOk ? Theme.success : Theme.danger)
                            radius: Theme.radiusMd

                            Text {
                                anchors.fill: parent
                                anchors.margins: 12
                                text: root.checkMessage === "" ? I18n.t("Пока проверок не было.") : root.checkMessage
                                color: Theme.textSecondary
                                font.pixelSize: 12
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                            }
                        }
                    }

                    SectionPanel {
                        title: I18n.t("Что проверять после запуска")
                        subtitle: I18n.t("Если server и client запущены, проверьте туннельную сеть обычными сетевыми командами.")
                    }

                    CodePanel {
                        title: I18n.t("Команды диагностики")
                        implicitHeight: 210
                        value: "ip addr\nip route\nping " + root.clientCidr.split('/')[0] + "\nssh root@" + root.clientCidr.split('/')[0] + "\n# " + I18n.t("затем проверьте RTSP/Web UI камеры через tunnel IP")
                    }
                }
            }
        }
    }
}
