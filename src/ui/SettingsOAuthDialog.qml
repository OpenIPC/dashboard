import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: dialog

    property var analyticsEngine: null
    property string oauthUrl: ""

    modal: true
    dim: true
    width: parent ? parent.width * 0.6 : 420
    height: parent ? parent.height * 0.3 : 220
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    onRejected: if (dialog.analyticsEngine) dialog.analyticsEngine.cancelOAuth()

    background: Rectangle { color: Theme.panelAltBackground; radius: Theme.radiusLg }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        Text {
            text: I18n.t("Авторизация открыта в браузере. Завершите вход и вернитесь в приложение.")
            color: Theme.textPrimary
            wrapMode: Text.Wrap
        }

        Text {
            text: "<a href=\"" + dialog.oauthUrl + "\">" + dialog.oauthUrl + "</a>"
            color: Theme.textMuted
            wrapMode: Text.Wrap
            font.pixelSize: 12
            textFormat: Text.RichText
            onLinkActivated: function(link) { Qt.openUrlExternally(link) }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 10

            Button {
                text: I18n.t("Открыть браузер")
                focusPolicy: Qt.StrongFocus
                onClicked: Qt.openUrlExternally(dialog.oauthUrl)
            }

            Button {
                text: I18n.t("Отмена")
                focusPolicy: Qt.StrongFocus
                onClicked: {
                    if (dialog.analyticsEngine) dialog.analyticsEngine.cancelOAuth()
                    dialog.close()
                }
            }
        }
    }
}
