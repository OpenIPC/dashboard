import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: root
    modal: true
    width: 800
    height: 600
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    closePolicy: Popup.NoAutoClose
    
    signal addCameraRequested(string name, string ip, int port, int onvifPort)

    onClosed: SystemController.stopNetworkScan()

    Dialog {
        id: batchLoginDialog
        title: I18n.t("Пакетное добавление")
        modal: true
        width: 400
        height: 250
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        
        background: Rectangle {
            color: Theme.metroSurface
            radius: Theme.metroTileRadius
            border.color: Theme.metroStroke
        }
        
        header: Rectangle {
            color: "transparent"
            height: 50
            Text {
                anchors.centerIn: parent
                text: batchLoginDialog.title
                color: "white"
                font.bold: true
                font.pixelSize: 16
            }
        }
        
        contentItem: ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            anchors.topMargin: 60
            spacing: 15
            
            Text {
                text: I18n.t("Введите логин и пароль для выбранных камер:")
                color: Theme.textSecondary
                font.pixelSize: 12
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            Text {
                text: "⚠️ " + I18n.t("Сторонние камеры могут быть добавлены с ошибками")
                color: Theme.metroAmber
                font.pixelSize: 12
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
            
            ColumnLayout {
                spacing: 5
                Layout.fillWidth: true
                Text { text: I18n.t("Логин"); color: Theme.textMuted; font.pixelSize: 12 }
                TextField {
                    id: batchLoginField
                    text: "root"
                    Layout.fillWidth: true
                    color: Theme.textPrimary
                    selectionColor: Theme.metroBlue
                    selectedTextColor: Theme.textPrimary
                    background: Rectangle { color: Theme.metroSurfaceAlt; radius: Theme.metroTileRadius; border.color: batchLoginField.activeFocus ? Theme.metroBlue : Theme.metroStroke }
                }
            }
            
            ColumnLayout {
                spacing: 5
                Layout.fillWidth: true
                Text { text: I18n.t("Пароль"); color: Theme.textMuted; font.pixelSize: 12 }
                TextField {
                    id: batchPasswordField
                    text: ""
                    echoMode: TextInput.Password
                    Layout.fillWidth: true
                    color: Theme.textPrimary
                    selectionColor: Theme.metroBlue
                    selectedTextColor: Theme.textPrimary
                    background: Rectangle { color: Theme.metroSurfaceAlt; radius: Theme.metroTileRadius; border.color: batchPasswordField.activeFocus ? Theme.metroBlue : Theme.metroStroke }
                }
            }
            
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 10
                spacing: 10
                
                Button {
                    text: I18n.t("Отмена")
                    Layout.fillWidth: true
                    hoverEnabled: true
                    background: Rectangle { color: parent.hovered ? Theme.metroTileHover : Theme.metroTile; radius: Theme.metroTileRadius; border.color: Theme.metroStroke }
                    contentItem: Text { text: parent.text; color: Theme.textPrimary; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.family: Theme.metroFontFamily }
                    onClicked: batchLoginDialog.close()
                }
                
                Button {
                    text: I18n.t("Добавить")
                    Layout.fillWidth: true
                    hoverEnabled: true
                    background: Rectangle { color: parent.hovered ? Theme.metroBlueHover : Theme.metroBlue; radius: Theme.metroTileRadius; border.color: Theme.metroBlue }
                    contentItem: Text { text: parent.text; color: Theme.textPrimary; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.bold: true; font.family: Theme.metroFontFamily }
                    onClicked: {
                        root.processBatchAdd(batchLoginField.text, batchPasswordField.text)
                        batchLoginDialog.close()
                    }
                }
            }
        }
    }

    function processBatchAdd(login, password) {
        SystemController.addDiscoveredCameras(root.selectedIndexList(), login, password, root.selectedProfile)
    }

    background: Rectangle {
        color: Theme.metroSidebarBackground
        radius: Theme.metroTileRadius
        border.color: Theme.metroStroke
    }
    
    header: Rectangle {
        color: "transparent"
        height: 60 // Increased height to prevent overlap
        
        Text {
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.top: parent.top
            anchors.topMargin: 20
            text: I18n.t("Найденные камеры")
            color: Theme.textPrimary
            font.family: Theme.metroFontFamily
            font.pixelSize: 18
            font.bold: true
        }
        
        MetroWindowButton {
            kind: "close"
            width: 34
            height: 34
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.top: parent.top
            anchors.topMargin: 16
            onClicked: root.close()
        }
    }
    
    property var interfaces: []
    property var selectedIndices: ({}) // Map to track selected indices
    property int selectedCount: 0
    property int validationCompleted: 0
    property int validationTotal: 0
    property int validationRevision: 0
    readonly property string selectedProfile: profileCombo.currentValue || "openipc"

    Connections {
        target: SystemController

        function onDiscoveryValidationProgress(completed, total) {
            root.validationCompleted = completed
            root.validationTotal = total
        }

        function onDiscoveryValidationFinished(okCount, failCount) {
            root.validationCompleted = 0
            root.validationTotal = 0
            root.validationRevision += 1
        }

        function onDiscoveryBatchAddFinished(addedCount, skippedCount) {
            if (addedCount > 0)
                root.close()
        }
    }
    
    onOpened: {
        refreshInterfaces()
        SystemController.refreshDiscoveryAddedFlags()
        selectedIndices = ({})
        selectedCount = 0
        validationCompleted = 0
        validationTotal = 0
        validationRevision += 1
    }
    
    function toggleSelection(index, isSelected) {
        if (isSelected) {
            selectedIndices[index] = true
        } else {
            delete selectedIndices[index]
        }
        // Update count
        var count = 0
        for (var key in selectedIndices) {
            count++
        }
        selectedCount = count
        validationRevision += 1
    }
    
    function refreshInterfaces() {
        interfaces = SystemController.getNetworkInterfaces()
        interfaceModel.clear()
        interfaceModel.append({text: I18n.t("Все интерфейсы"), value: ""})
        for (var i = 0; i < interfaces.length; i++) {
            var prefix = interfaces[i].prefixLength && interfaces[i].prefixLength > 0 ? "/" + interfaces[i].prefixLength : ""
            interfaceModel.append({
                text: interfaces[i].name + " (" + interfaces[i].ip + prefix + ")",
                value: interfaces[i].id
            })
        }
        interfaceCombo.currentIndex = 0
    }

    function selectedIndexList() {
        var indexes = []
        for (var key in selectedIndices) {
            if (selectedIndices[key] === true)
                indexes.push(parseInt(key))
        }
        indexes.sort(function(a, b) { return a - b })
        return indexes
    }

    function selectedReadyToAdd() {
        validationRevision
        var indexes = selectedIndexList()
        if (indexes.length === 0)
            return false
        for (var i = 0; i < indexes.length; ++i) {
            var cam = SystemController.discoveryModel.getCamera(indexes[i])
            if (cam.alreadyAdded)
                continue
            if (cam.validationStatus !== "ok")
                return false
        }
        return true
    }

    ListModel {
        id: profileModel
        ListElement { text: "OpenIPC / Majestic"; value: "openipc" }
        ListElement { text: "ONVIF"; value: "onvif" }
        ListElement { text: "RTSP manual"; value: "rtsp" }
    }

    contentItem: ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.bottomMargin: 20
        anchors.topMargin: 80 // Header height (60) + spacing (20)
        spacing: 15
        
        // Interface Selection Section
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 5
            
            RowLayout {
                Layout.fillWidth: true
                Text { 
                    text: I18n.t("Сетевой интерфейс")
                    color: Theme.textPrimary
                    font.family: Theme.metroFontFamily
                    font.bold: true
                    font.pixelSize: 14
                }
                
                Item { Layout.fillWidth: true }
                
                Text {
                    text: I18n.t("ОБНОВИТЬ ИНТЕРФЕЙСЫ")
                    color: Theme.metroBlue
                    font.pixelSize: 12
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.refreshInterfaces()
                    }
                }
                
                Rectangle {
                    width: 40
                    height: 24
                    color: "transparent"
                    border.color: Theme.metroStroke
                    radius: Theme.metroTileRadius
                    Text {
                        anchors.centerIn: parent
                        text: I18n.t("ВСЕ")
                        color: Theme.textMuted
                        font.pixelSize: 11
                    }
                }
            }
            
            Text {
                text: I18n.t("Выберите адаптер, который нужно сканировать.")
                color: Theme.textMuted
                font.pixelSize: 12
            }
            
            StyledComboBox {
                id: interfaceCombo
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                textRole: "text"
                valueRole: "value"
                model: ListModel { id: interfaceModel }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: width > 640 ? 3 : 1
                rowSpacing: 8
                columnSpacing: 10

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Text { text: I18n.t("Профиль добавления"); color: Theme.textMuted; font.pixelSize: 11 }
                    StyledComboBox {
                        id: profileCombo
                        Layout.fillWidth: true
                        Layout.preferredHeight: 38
                        textRole: "text"
                        valueRole: "value"
                        model: profileModel
                        currentIndex: 0
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Text { text: I18n.t("Логин для проверки"); color: Theme.textMuted; font.pixelSize: 11 }
                    TextField {
                        id: validationLoginField
                        Layout.fillWidth: true
                        implicitHeight: 38
                        text: "root"
                        color: Theme.textPrimary
                        selectionColor: Theme.metroBlue
                        selectedTextColor: Theme.textPrimary
                        background: Rectangle {
                            color: Theme.metroSurfaceAlt
                            radius: Theme.metroTileRadius
                            border.color: validationLoginField.activeFocus ? Theme.metroBlue : Theme.metroStroke
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Text { text: I18n.t("Пароль для проверки"); color: Theme.textMuted; font.pixelSize: 11 }
                    TextField {
                        id: validationPasswordField
                        Layout.fillWidth: true
                        implicitHeight: 38
                        echoMode: TextInput.Password
                        color: Theme.textPrimary
                        selectionColor: Theme.metroBlue
                        selectedTextColor: Theme.textPrimary
                        background: Rectangle {
                            color: Theme.metroSurfaceAlt
                            radius: Theme.metroTileRadius
                            border.color: validationPasswordField.activeFocus ? Theme.metroBlue : Theme.metroStroke
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                MajesticCheckBox {
                    id: deepScanCheck
                    text: I18n.t("Глубокий поиск OpenIPC")
                    ToolTip.visible: hovered
                    ToolTip.text: I18n.t("Быстрый режим проверяет локальный /24, глубокий расширяет поиск до /20. В обоих режимах проверяются Majestic HTTP и RTSP.")
                }
                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                    text: SystemController.networkDiscovery.running
                          ? I18n.t("Поиск: %1 · найдено %2", [I18n.t(SystemController.networkDiscovery.phase),
                                                             SystemController.networkDiscovery.foundCount])
                          : (SystemController.networkDiscovery.progress === 100
                             ? I18n.t("Поиск завершён · найдено %1", [SystemController.networkDiscovery.foundCount]) : "")
                    color: SystemController.networkDiscovery.running ? Theme.metroBlue : Theme.textMuted
                    font.pixelSize: 11
                    elide: Text.ElideLeft
                }
            }

            Text {
                Layout.fillWidth: true
                text: validationTotal > 0
                      ? I18n.t("Проверка выбранных: %1/%2", [validationCompleted, validationTotal])
                      : SystemController.discoverySessionSummary()
                color: validationTotal > 0 ? Theme.metroBlue : Theme.textMuted
                font.pixelSize: 11
                elide: Text.ElideRight
            }

            RowLayout {
                Layout.fillWidth: true
                visible: SystemController.networkDiscovery.running
                spacing: 10

                ProgressBar {
                    id: discoveryProgress
                    Layout.fillWidth: true
                    Layout.preferredHeight: 10
                    from: 0
                    to: 100
                    value: SystemController.networkDiscovery.progress
                    Behavior on value { NumberAnimation { duration: 260; easing.type: Easing.OutCubic } }
                    background: Rectangle {
                        implicitHeight: 10
                        radius: 5
                        color: Theme.metroSurfaceAlt
                        border.color: Theme.metroStroke
                    }
                    contentItem: Item {
                        Rectangle {
                            width: discoveryProgress.visualPosition * parent.width
                            height: parent.height
                            radius: 5
                            color: SystemController.networkDiscovery.running ? Theme.metroBlue : Theme.metroGreen
                        }
                    }
                }

                Text {
                    Layout.preferredWidth: 44
                    horizontalAlignment: Text.AlignRight
                    text: Math.round(SystemController.networkDiscovery.progress) + "%"
                    color: SystemController.networkDiscovery.running ? Theme.textSecondary : Theme.metroGreen
                    font.pixelSize: 12
                    font.bold: true
                }
            }
        }
        
        // Results Header
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: I18n.t("Найдено устройств: ") + resultsList.count
                color: Theme.textPrimary
                font.family: Theme.metroFontFamily
                font.bold: true
                font.pixelSize: 14
            }
            Item { Layout.fillWidth: true }
            Text {
                text: I18n.t("Найдено камер: ") + resultsList.count
                color: Theme.textMuted
                font.pixelSize: 12
            }
        }
        
        // Table Header
        Rectangle {
            Layout.fillWidth: true
            height: 30
            color: "transparent"
            
            RowLayout {
                anchors.fill: parent
                spacing: 10
                
                MetroCheckBox {
                    checked: false
                    enabled: false
                    Layout.preferredWidth: 22
                }
                
                Text { Layout.preferredWidth: 200; text: I18n.t("Устройство"); color: Theme.textMuted; font.pixelSize: 12 }
                Text { Layout.preferredWidth: 100; text: I18n.t("Сеть"); color: Theme.textMuted; font.pixelSize: 12 }
                Text { Layout.preferredWidth: 195; text: I18n.t("Порты"); color: Theme.textMuted; font.pixelSize: 12 }
                Text { Layout.fillWidth: true; text: I18n.t("Протокол"); color: Theme.textMuted; font.pixelSize: 12 }
            }
        }
        
        // Results List
        ListView {
            id: resultsList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: SystemController.discoveryModel
            spacing: 5
            
            delegate: Rectangle {
                width: resultsList.width
                height: 78
                color: model.alreadyAdded ? Theme.successSurface
                                           : (model.validationStatus === "fail" ? Theme.dangerSurface
                                              : (model.validationStatus === "ok" ? Theme.successSurface : Theme.metroTile))
                radius: Theme.metroTileRadius
                border.color: model.alreadyAdded ? Theme.metroGreen
                             : (model.validationStatus === "fail" ? Theme.metroRed
                                : (model.validationStatus === "ok" ? Theme.metroGreen : Theme.metroStroke))
                border.width: 1
                
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if (!model.alreadyAdded)
                            selectionCheck.checked = !selectionCheck.checked
                    }
                    onEntered: if (!model.alreadyAdded && model.validationStatus !== "fail" && model.validationStatus !== "ok") parent.color = Theme.metroTileHover
                    onExited: parent.color = model.alreadyAdded ? Theme.successSurface
                                           : (model.validationStatus === "fail" ? Theme.dangerSurface
                                              : (model.validationStatus === "ok" ? Theme.successSurface : Theme.metroTile))
                }
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10
                    
                    MetroCheckBox {
                        id: selectionCheck
                        checked: root.selectedIndices[index] === true
                        enabled: !model.alreadyAdded
                        Layout.preferredWidth: 22
                        onCheckedChanged: {
                            if (model.alreadyAdded && checked) {
                                checked = false
                                return
                            }
                            root.toggleSelection(index, checked)
                        }
                    }
                    
                    // Device Info
                    ColumnLayout {
                        Layout.preferredWidth: 200
                        spacing: 2
                        Text { 
                            text: model.cameraName 
                            color: Theme.textPrimary
                            font.family: Theme.metroFontFamily
                            font.bold: true
                            font.pixelSize: 13
                        }
                        Text { 
                            text: model.cameraIp
                            color: Theme.textSecondary
                            font.pixelSize: 12
                        }
                        Text {
                            text: (model.manufacturer ? model.manufacturer : "")
                                  + (model.discoveryMethods ? " · " + model.discoveryMethods : "")
                            color: model.isOpenIpc ? Theme.metroBlue : Theme.textMuted
                            font.pixelSize: 10
                            visible: text.length > 0
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Text {
                            text: model.validationMessage || model.discoveryEvidence || ""
                            color: model.validationStatus === "fail" ? Theme.danger
                                  : (model.validationStatus === "ok" || model.alreadyAdded ? Theme.success : Theme.textMuted)
                            font.pixelSize: 10
                            visible: text.length > 0
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                    
                    // Network Info
                    Row {
                        Layout.preferredWidth: 100
                        spacing: 5
                        Rectangle {
                            width: 60
                            height: 20
                            color: model.alreadyAdded ? Theme.successSurface : Theme.metroSurfaceAlt
                            border.color: model.alreadyAdded ? Theme.metroGreen : "transparent"
                            radius: 10
                            Text {
                                anchors.centerIn: parent
                                text: model.alreadyAdded ? I18n.t("В списке")
                                      : (model.isOpenIpc ? "OPENIPC" : (model.discoveryConfidence ? model.discoveryConfidence + "%" : "CAM"))
                                color: model.alreadyAdded ? Theme.success : Theme.textSecondary
                                font.pixelSize: 10
                            }
                        }
                    }
                    
                    // Ports Info
                    Row {
                        Layout.preferredWidth: 195
                        spacing: 5
                        
                        // SDK Tag
                        Rectangle {
                            width: 40
                            height: 20
                            color: "transparent"
                            border.color: Theme.metroAmber
                            radius: 10
                            visible: model.discoveryMethods
                                     && model.discoveryMethods.indexOf("Dahua") >= 0
                            Text {
                                anchors.centerIn: parent
                                text: "SDK"
                                color: Theme.metroAmber
                                font.pixelSize: 10
                                font.bold: true
                            }
                        }

                        // RTSP Tag
                        Rectangle {
                            width: 65
                            height: 20
                            color: "transparent"
                            border.color: Theme.metroGreen
                            radius: 10
                            visible: true
                            Text {
                                anchors.centerIn: parent
                                text: "RTSP " + (model.cameraPort ? model.cameraPort : 554)
                                color: Theme.metroGreen
                                font.pixelSize: 10
                            }
                        }
                        
                        // ONVIF Tag
                        Rectangle {
                            width: 65
                            height: 20
                            color: "transparent"
                            border.color: Theme.metroBlue
                            radius: 10
                            visible: true
                            Text {
                                anchors.centerIn: parent
                                text: "HTTP " + (model.cameraOnvifPort ? model.cameraOnvifPort : 80)
                                color: Theme.metroBlue
                                font.pixelSize: 10
                            }
                        }
                    }
                    
                    // Protocol Info
                    Row {
                        Layout.fillWidth: true
                        spacing: 5
                        Rectangle {
                            width: 72
                            height: 20
                            color: Theme.metroSurfaceAlt
                            border.color: Theme.metroStroke
                            border.width: 1
                            radius: 10
                            Text {
                                anchors.centerIn: parent
                                text: model.isOpenIpc ? "mdns/api" : "onvif/rtsp"
                                color: Theme.textPrimary
                                font.pixelSize: 10
                            }
                        }
                        Rectangle {
                            width: 84
                            height: 20
                            color: model.validationStatus === "running" ? Theme.metroDeepBlue
                                  : (model.validationStatus === "ok" || model.alreadyAdded ? Theme.successSurface
                                     : (model.validationStatus === "fail" ? Theme.dangerSurface : Theme.metroSurfaceAlt))
                            border.color: model.validationStatus === "ok" || model.alreadyAdded ? Theme.metroGreen
                                         : (model.validationStatus === "fail" ? Theme.metroRed : Theme.metroStroke)
                            border.width: 1
                            radius: 10
                            Text {
                                anchors.centerIn: parent
                                text: model.alreadyAdded ? I18n.t("Добавлена")
                                      : (model.validationStatus === "running" ? I18n.t("Проверка")
                                         : (model.validationStatus === "ok" ? "OK"
                                            : (model.validationStatus === "fail" ? I18n.t("Ошибка") : (model.onboardingProfile || root.selectedProfile))))
                                color: model.validationStatus === "fail" ? Theme.danger
                                      : (model.validationStatus === "ok" || model.alreadyAdded ? Theme.success : Theme.textPrimary)
                                font.pixelSize: 10
                                font.bold: model.validationStatus === "ok" || model.alreadyAdded
                            }
                        }
                    }
                }
            }
            
            // Empty State
            Text {
                anchors.centerIn: parent
                text: I18n.t("Нажмите \"Сканировать\", чтобы найти камеры.\nПроверьте, что сеть помечена как «Частная»...")
                color: Theme.textMuted
                horizontalAlignment: Text.AlignHCenter
                visible: resultsList.count === 0
            }
        }
        
        // Footer Actions
        RowLayout {
            Layout.fillWidth: true
            
            Button {
                text: SystemController.networkDiscovery.running
                      ? I18n.t("ОСТАНОВИТЬ") : I18n.t("СКАНИРОВАТЬ")
                Layout.preferredWidth: 140
                Layout.preferredHeight: 40
                hoverEnabled: true
                
                background: Rectangle {
                    color: parent.hovered ? Theme.metroBlueHover : Theme.metroBlue
                    radius: Theme.metroTileRadius
                    border.color: Theme.metroBlue
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                    font.family: Theme.metroFontFamily
                }
                
                onClicked: {
                    if (SystemController.networkDiscovery.running) {
                        SystemController.stopNetworkScan()
                    } else {
                        var iface = interfaceCombo.currentValue
                        SystemController.scanNetwork(iface, deepScanCheck.checked)
                    }
                }
            }

            Button {
                text: I18n.t("ОЧИСТИТЬ")
                Layout.preferredWidth: 110
                Layout.preferredHeight: 40
                enabled: !SystemController.networkDiscovery.running && resultsList.count > 0
                hoverEnabled: true

                background: Rectangle {
                    color: parent.enabled ? (parent.hovered ? Theme.metroTileHover : Theme.metroTile) : Theme.metroTileDisabled
                    radius: Theme.metroTileRadius
                    border.color: parent.enabled ? Theme.metroStroke : Theme.metroStroke
                }
                contentItem: Text {
                    text: parent.text
                    color: parent.enabled ? Theme.textPrimary : Theme.textMuted
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.family: Theme.metroFontFamily
                }
                onClicked: {
                    SystemController.clearDiscoveryResults()
                    selectedIndices = ({})
                    selectedCount = 0
                }
            }
            
            Item { Layout.fillWidth: true }

            Button {
                text: validationTotal > 0
                      ? I18n.t("ПРОВЕРКА %1/%2", [validationCompleted, validationTotal])
                      : I18n.t("ПРОВЕРИТЬ ВЫБРАННЫЕ")
                Layout.preferredWidth: 190
                Layout.preferredHeight: 40
                enabled: root.selectedCount > 0 && validationTotal === 0
                hoverEnabled: true

                background: Rectangle {
                    color: parent.enabled ? (parent.hovered ? Theme.metroTileHover : Theme.metroTile) : Theme.metroTileDisabled
                    radius: Theme.metroTileRadius
                    border.color: parent.enabled ? Theme.metroBlue : Theme.metroStroke
                }
                contentItem: Text {
                    text: parent.text
                    color: parent.enabled ? Theme.textPrimary : Theme.textMuted
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.bold: parent.enabled
                    font.family: Theme.metroFontFamily
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }
                onClicked: {
                    SystemController.validateDiscoverySelection(
                        root.selectedIndexList(),
                        validationLoginField.text,
                        validationPasswordField.text,
                        root.selectedProfile
                    )
                }
            }
            
            Button {
                text: root.selectedReadyToAdd()
                      ? I18n.t("ДОБАВИТЬ ВЫБРАННЫЕ (") + root.selectedCount + ")"
                      : I18n.t("ПРОВЕРИТЬ И ДОБАВИТЬ (") + root.selectedCount + ")"
                Layout.preferredWidth: 200
                Layout.preferredHeight: 40
                enabled: root.selectedCount > 0 && validationTotal === 0
                hoverEnabled: true
                
                background: Rectangle {
                    color: parent.enabled ? (parent.hovered ? Theme.metroBlueHover : Theme.metroBlue) : Theme.metroTileDisabled
                    radius: Theme.metroTileRadius
                    border.color: parent.enabled ? Theme.metroBlue : Theme.metroStroke
                }
                contentItem: Text {
                    text: parent.text
                    color: parent.enabled ? Theme.textPrimary : Theme.textMuted
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                    font.family: Theme.metroFontFamily
                }
                onClicked: {
                    if (!root.selectedReadyToAdd()) {
                        SystemController.validateDiscoverySelection(
                            root.selectedIndexList(),
                            validationLoginField.text,
                            validationPasswordField.text,
                            root.selectedProfile
                        )
                        return
                    }
                    SystemController.addDiscoveredCameras(
                        root.selectedIndexList(),
                        validationLoginField.text,
                        validationPasswordField.text,
                        root.selectedProfile
                    )
                }
            }
        }
    }
}
