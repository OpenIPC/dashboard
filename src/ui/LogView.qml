import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import OpenIPC

Dialog {
    id: root
    title: I18n.t("Логи")
    modal: true
    width: Math.max(320, Math.min(800, (parent ? parent.width : 832) - 32))
    height: Math.max(320, Math.min(600, (parent ? parent.height : 632) - 32))
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    readonly property bool layoutReady: width > 0
                                        && height > 0
                                        && (!parent || (width <= parent.width && height <= parent.height))
    
    background: Rectangle {
        color: Theme.metroSidebarBackground
        radius: 8
        border.color: Theme.metroStroke
    }
    
    property bool showInfo: true
    property bool showWarning: true
    property bool showError: true // Critical and Fatal
    property bool showDebug: false
    required property var logModel

    function scrollToEnd() { scrollTimer.restart() }

    onOpened: {
        root.logModel.reloadFromFile()
        root.scrollToEnd()
    }

    Timer {
        id: scrollTimer
        interval: 0
        onTriggered: logListView.positionViewAtEnd()
    }

    component StyledCheckBox: MetroCheckBox {
    }

    header: Rectangle {
        id: headerBar
        height: 50
        color: "transparent"
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            
            Text {
                text: root.title
                color: "white"
                font.pixelSize: 18
                font.bold: true
                Layout.alignment: Qt.AlignVCenter
            }
            
            Item { Layout.fillWidth: true }
            
            Button {
                id: exportButton
                text: I18n.t("Экспорт")
                enabled: root.logModel.count > 0
                onClicked: saveDialog.open()
                background: Rectangle {
                    color: exportButton.down ? Theme.metroTileHover : Theme.metroSurface
                    radius: 4
                    border.color: Theme.metroStroke
                }
                contentItem: Text {
                    text: exportButton.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                id: refreshButton
                text: I18n.t("Обновить")
                onClicked: {
                    root.logModel.reloadFromFile()
                    root.scrollToEnd()
                }
                background: Rectangle {
                    color: refreshButton.down ? Theme.metroTileHover : Theme.metroSurface
                    radius: 4
                    border.color: Theme.metroStroke
                }
                contentItem: Text {
                    text: refreshButton.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                id: clearButton
                text: I18n.t("Очистить")
                enabled: root.logModel.count > 0
                onClicked: root.logModel.clear()
                background: Rectangle {
                    color: clearButton.down ? Theme.metroTileHover : Theme.metroSurface
                    radius: 4
                    border.color: Theme.metroStroke
                }
                contentItem: Text {
                    text: clearButton.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            MetroWindowButton {
                kind: "close"
                Layout.preferredWidth: 38
                Layout.preferredHeight: 34
                onClicked: root.close()
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: 10
        
        FileDialog {
            id: saveDialog
            title: I18n.t("Сохранить логи")
            fileMode: FileDialog.SaveFile
            nameFilters: [I18n.t("Текстовые файлы (*.txt)"), I18n.t("Все файлы (*)")]
            defaultSuffix: "txt"
            // currentFile: "logs.txt"
            onAccepted: {
                root.logModel.saveLog(selectedFile)
            }
        }
        
        // Filters
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 10
            Layout.rightMargin: 10
            
            StyledCheckBox {
                text: I18n.t("Инфо")
                checked: root.showInfo
                onCheckedChanged: root.showInfo = checked
            }
            StyledCheckBox {
                text: I18n.t("Предупреждения")
                checked: root.showWarning
                onCheckedChanged: root.showWarning = checked
            }
            StyledCheckBox {
                text: I18n.t("Ошибки")
                checked: root.showError
                onCheckedChanged: root.showError = checked
            }
            StyledCheckBox {
                text: I18n.t("Отладка")
                checked: root.showDebug
                onCheckedChanged: root.showDebug = checked
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.metroBackground
            border.color: Theme.metroStroke
            radius: 4
            
            ListView {
                id: logListView
                anchors.fill: parent
                anchors.margins: 5
                clip: true
                model: root.logModel
                
                delegate: Item {
                    id: logDelegate

                    required property int type
                    required property string formattedTime
                    required property string message

                    width: ListView.view.width
                    height: visible ? 20 : 0
                    visible: {
                        if (logDelegate.type === 0 && !root.showDebug) return false; // Debug
                        if (logDelegate.type === 1 && !root.showWarning) return false; // Warning
                        if (logDelegate.type === 2 && !root.showError) return false; // Critical
                        if (logDelegate.type === 3 && !root.showError) return false; // Fatal
                        if (logDelegate.type === 4 && !root.showInfo) return false; // Info
                        return true;
                    }

                    RowLayout {
                        anchors.fill: parent
                        spacing: 10
                        
                        Text {
                            text: logDelegate.formattedTime
                            color: Theme.textMuted
                            font.family: "Consolas, monospace"
                            font.pixelSize: 12
                            Layout.preferredWidth: 140
                        }
                        
                        Text {
                            text: {
                                switch(logDelegate.type) {
                                    case 0: return "DBG";
                                    case 1: return "WRN";
                                    case 2: return "CRT";
                                    case 3: return "FTL";
                                    case 4: return "INF";
                                    default: return "UNK";
                                }
                            }
                            color: {
                                switch(logDelegate.type) {
                                    case 0: return Theme.textMuted; // Debug
                                    case 1: return Theme.metroAmber; // Warning
                                    case 2: return Theme.metroRed; // Critical
                                    case 3: return Theme.metroRed; // Fatal
                                    case 4: return Theme.metroBlue; // Info
                                    default: return "white";
                                }
                            }
                            font.family: "Consolas, monospace"
                            font.pixelSize: 12
                            font.bold: true
                            Layout.preferredWidth: 30
                        }
                        
                        Text {
                            text: logDelegate.message
                            color: "white"
                            font.family: "Consolas, monospace"
                            font.pixelSize: 12
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }
                }
                
                ScrollBar.vertical: ScrollBar { }
            }

            Text {
                anchors.centerIn: parent
                visible: root.logModel.count === 0
                text: I18n.t("Логи пока пусты")
                color: Theme.textMuted
                font.pixelSize: 15
            }

            Connections {
                target: root.logModel
                function onCountChanged() {
                    if (root.visible) {
                        root.scrollToEnd()
                    }
                }
            }
        }
    }
}
