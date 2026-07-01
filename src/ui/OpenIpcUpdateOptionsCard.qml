import QtQuick
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property bool updateKernel: true
    property bool updateRootfs: true
    property bool updateReset: false
    property bool updateForce: false
    property string optionsSummary: ""

    signal kernelToggled(bool checked)
    signal rootfsToggled(bool checked)
    signal resetToggled(bool checked)
    signal forceToggled(bool checked)

    Layout.fillWidth: true
    Layout.leftMargin: 16
    Layout.rightMargin: 16
    Layout.preferredHeight: 122
    color: Theme.cardBackground
    border.color: (root.updateReset || root.updateForce || (!root.updateKernel && !root.updateRootfs)) ? Theme.warning : Theme.cardBorder
    radius: Theme.radiusLg

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 8

        RowLayout {
            Layout.fillWidth: true

            Text {
                Layout.fillWidth: true
                text: I18n.t("Опции прошивки")
                color: Theme.textPrimary
                font.bold: true
                font.pixelSize: 16
            }

            Text {
                text: root.optionsSummary
                color: Theme.accentHover
                font.family: "Consolas"
                font.pixelSize: 12
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: width > 760 ? 4 : 2
            rowSpacing: 8
            columnSpacing: 12

            MajesticCheckBox {
                text: "Kernel"
                checked: root.updateKernel
                onToggled: root.kernelToggled(checked)
            }

            MajesticCheckBox {
                text: "RootFS"
                checked: root.updateRootfs
                onToggled: root.rootfsToggled(checked)
            }

            MajesticCheckBox {
                text: I18n.t("Reset config")
                checked: root.updateReset
                onToggled: root.resetToggled(checked)
            }

            MajesticCheckBox {
                text: I18n.t("Force reflash")
                checked: root.updateForce
                onToggled: root.forceToggled(checked)
            }
        }

        Text {
            Layout.fillWidth: true
            text: (!root.updateKernel && !root.updateRootfs)
                  ? I18n.t("Выберите хотя бы kernel или rootfs.")
                  : (root.updateReset || root.updateForce)
                    ? I18n.t("Reset/Force — опасные опции. Используйте их только если понимаете последствия.")
                    : I18n.t("Стандартный безопасный режим: kernel + rootfs, без reset/force.")
            color: (!root.updateKernel && !root.updateRootfs) || root.updateReset || root.updateForce ? Theme.warning : Theme.textMuted
            wrapMode: Text.WordWrap
            font.pixelSize: 11
        }
    }
}
