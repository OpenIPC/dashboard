import QtQuick
import QtQuick.Controls
import OpenIPC

MenuSeparator {
    id: separator

    topPadding: 4
    bottomPadding: 4

    contentItem: Rectangle {
        implicitWidth: 1
        implicitHeight: 1
        color: Theme.metroStroke
        opacity: 0.8
    }
}
