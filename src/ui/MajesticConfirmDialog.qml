import QtQuick
import QtQuick.Controls
import OpenIPC

Dialog {
    id: control

    property string message: ""
    property color messageColor: Theme.textSecondary
    property int dialogWidth: 560

    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 100 : dialogWidth, dialogWidth)
    standardButtons: Dialog.Ok | Dialog.Cancel

    contentItem: Label {
        text: control.message
        color: control.messageColor
        wrapMode: Text.WordWrap
        padding: 16
    }
}
