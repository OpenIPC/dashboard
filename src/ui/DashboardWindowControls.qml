import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import OpenIPC

RowLayout {
    id: root

    spacing: 4

    MetroWindowButton {
        kind: "minimize"
        Layout.preferredWidth: 36
        Layout.preferredHeight: 32
        onClicked: Window.window.showMinimized()
    }

    MetroWindowButton {
        kind: "maximize"
        maximized: Window.window && Window.window.visibility === Window.Maximized
        Layout.preferredWidth: 36
        Layout.preferredHeight: 32
        onClicked: {
            if (Window.window.visibility === Window.Maximized)
                Window.window.showNormal()
            else
                Window.window.showMaximized()
        }
    }

    MetroWindowButton {
        kind: "close"
        Layout.preferredWidth: 36
        Layout.preferredHeight: 32
        onClicked: Window.window.close()
    }
}
