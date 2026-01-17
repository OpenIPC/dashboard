import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import OpenIPC

Dialog {
    id: root
    title: I18n.t("Логи")
    modal: true
    width: 800
    height: 600
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    
    background: Rectangle {
        color: "#252526"
        radius: 8
        border.color: "#3c3c3c"
    }
    
    property bool showInfo: true
    property bool showWarning: true
    property bool showError: true // Critical and Fatal
    property bool showDebug: false

    component StyledCheckBox: CheckBox {
        hoverEnabled: false
        background: Item {}
        indicator: Rectangle {
            implicitWidth: 18
            implicitHeight: 18
            x: parent.leftPadding
            y: parent.height / 2 - height / 2
            radius: 3
            color: "#252526"
            border.color: parent.checked ? "#4caf50" : "#666666"
            
            Rectangle {
                width: 10
                height: 10
                anchors.centerIn: parent
                radius: 2
                color: "#4caf50"
                visible: parent.parent.checked
            }
        }
        contentItem: Text {
            text: parent.text
            font: parent.font
            opacity: parent.enabled ? 1.0 : 0.5
            color: "white"
            verticalAlignment: Text.AlignVCenter
            leftPadding: parent.indicator.width + parent.spacing
        }
    }

    header: Rectangle {
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
                text: I18n.t("Скачать")
                onClicked: saveDialog.open()
                background: Rectangle {
                    color: parent.down ? "#3e3e42" : "#2d2d30"
                    radius: 4
                    border.color: "#3c3c3c"
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                text: I18n.t("Очистить")
                onClicked: SystemController.logModel.clear()
                background: Rectangle {
                    color: parent.down ? "#3e3e42" : "#2d2d30"
                    radius: 4
                    border.color: "#3c3c3c"
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Text {
                text: "×"
                color: "#aaaaaa"
                font.pixelSize: 24
                Layout.alignment: Qt.AlignVCenter
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.close()
                }
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
                SystemController.logModel.saveLog(selectedFile)
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
                text: I18n.t("Предупреждение")
                checked: root.showWarning
                onCheckedChanged: root.showWarning = checked
            }
            StyledCheckBox {
                text: I18n.t("Ошибка")
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
            color: "#1e1e1e"
            border.color: "#3c3c3c"
            radius: 4
            
            ListView {
                id: logListView
                anchors.fill: parent
                anchors.margins: 5
                clip: true
                model: SystemController.logModel
                
                delegate: Item {
                    width: ListView.view.width
                    height: visible ? 20 : 0
                    visible: {
                        if (type === 0 && !root.showDebug) return false; // Debug
                        if (type === 1 && !root.showWarning) return false; // Warning
                        if (type === 2 && !root.showError) return false; // Critical
                        if (type === 3 && !root.showError) return false; // Fatal
                        if (type === 4 && !root.showInfo) return false; // Info
                        return true;
                    }

                    RowLayout {
                        anchors.fill: parent
                        spacing: 10
                        
                        Text {
                            text: formattedTime
                            color: "#888888"
                            font.family: "Consolas, monospace"
                            font.pixelSize: 12
                            Layout.preferredWidth: 140
                        }
                        
                        Text {
                            text: {
                                switch(type) {
                                    case 0: return "DBG";
                                    case 1: return "WRN";
                                    case 2: return "CRT";
                                    case 3: return "FTL";
                                    case 4: return "INF";
                                    default: return "UNK";
                                }
                            }
                            color: {
                                switch(type) {
                                    case 0: return "#aaaaaa"; // Debug
                                    case 1: return "#dcdcaa"; // Warning
                                    case 2: return "#f44747"; // Critical
                                    case 3: return "#f44747"; // Fatal
                                    case 4: return "#569cd6"; // Info
                                    default: return "white";
                                }
                            }
                            font.family: "Consolas, monospace"
                            font.pixelSize: 12
                            font.bold: true
                            Layout.preferredWidth: 30
                        }
                        
                        Text {
                            text: message
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
        }
    }
}
