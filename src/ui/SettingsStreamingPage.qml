pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Item {
    id: page

    required property var settings
    readonly property bool compactLayout: width < 560
    readonly property bool layoutReady: width > 0
                                        && height > 0
                                        && scrollView.availableWidth > 0
                                        && streamingGrid.width <= scrollView.availableWidth + 1
                                        && adjustmentsGrid.width <= scrollView.availableWidth + 1

    component SectionTitle: Text {
        color: Theme.textPrimary
        font.pixelSize: 16
        font.bold: true
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
    }

    component FieldLabel: Text {
        color: Theme.textMuted
        font.pixelSize: 14
        wrapMode: Text.WordWrap
        verticalAlignment: Text.AlignVCenter
        Layout.fillWidth: page.compactLayout
        Layout.preferredWidth: page.compactLayout ? implicitWidth : 190
        Layout.minimumWidth: 0
    }

    component Separator: Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 1
        color: Theme.metroStroke
    }

    ScrollView {
        id: scrollView

        anchors.fill: parent
        leftPadding: page.compactLayout ? 12 : 20
        rightPadding: page.compactLayout ? 12 : 20
        topPadding: 16
        bottomPadding: 16
        contentWidth: availableWidth
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical: StyledScrollBar {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom
        }

        ColumnLayout {
            width: scrollView.availableWidth
            spacing: 14

            SectionTitle {
                text: I18n.t("Трансляция (GStreamer)")
            }

            GridLayout {
                id: streamingGrid

                columns: page.compactLayout ? 1 : 2
                columnSpacing: 14
                rowSpacing: page.compactLayout ? 6 : 10
                Layout.fillWidth: true

                FieldLabel {
                    text: I18n.t("Режим буферизации (Latency)")
                }

                StyledComboBox {
                    id: bufferModeCombo

                    model: [
                        I18n.t("Стандартная (Stable / 2s)"),
                        I18n.t("Минимальная (Low Latency / 200ms)"),
                        I18n.t("Ультра-низкая (Realtime / 0ms)")
                    ]
                    currentIndex: page.settings.playerBufferMode
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    onUserSelected: function(index) {
                        page.settings.playerBufferMode = index
                    }
                }

                FieldLabel {
                    text: I18n.t("Протокол RTSP")
                }

                StyledComboBox {
                    id: rtspTransportCombo

                    model: ["TCP (Interleaved)", "UDP (Unicast)", "UDP (Multicast)", "HTTP (Tunneling)"]
                    currentIndex: page.settings.playerRtspTransport === "tcp"
                                  ? 0
                                  : page.settings.playerRtspTransport === "udp"
                                    ? 1
                                    : page.settings.playerRtspTransport === "udp_mcast" ? 2 : 3
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    onUserSelected: function(index) {
                        if (index === 0) page.settings.playerRtspTransport = "tcp"
                        else if (index === 1) page.settings.playerRtspTransport = "udp"
                        else if (index === 2) page.settings.playerRtspTransport = "udp_mcast"
                        else page.settings.playerRtspTransport = "http"
                    }
                }

                FieldLabel {
                    text: I18n.t("Аппаратное декодирование")
                }

                StyledComboBox {
                    id: hwDecodingCombo

                    model: [I18n.t("Авто"), "D3D11", "DXVA2", "Off (None)"]
                    currentIndex: {
                        if (page.settings.playerHwDecoding === "d3d11") return 1
                        if (page.settings.playerHwDecoding === "dxva2") return 2
                        if (page.settings.playerHwDecoding === "none") return 3
                        return 0
                    }
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    onUserSelected: function(index) {
                        if (index === 1) page.settings.playerHwDecoding = "d3d11"
                        else if (index === 2) page.settings.playerHwDecoding = "dxva2"
                        else if (index === 3) page.settings.playerHwDecoding = "none"
                        else page.settings.playerHwDecoding = "auto"
                    }
                }

                FieldLabel {
                    text: I18n.t("Предпочтительный поток")
                }

                StyledComboBox {
                    id: preferredStreamCombo

                    model: [I18n.t("Авто"), "HD", "SD"]
                    currentIndex: page.settings.preferredStream === "hd"
                                  ? 1
                                  : page.settings.preferredStream === "sd" ? 2 : 0
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    onUserSelected: function(index) {
                        page.settings.preferredStream = index === 1 ? "hd" : index === 2 ? "sd" : "auto"
                    }
                }

                FieldLabel {
                    text: I18n.t("Отображать статистику")
                }

                MetroCheckBox {
                    text: I18n.t("Показывать codec/res/bitrate/fps")
                    checked: page.settings.showStatsOverlay
                    Layout.fillWidth: true
                    onToggled: page.settings.showStatsOverlay = checked
                }

                FieldLabel {
                    text: I18n.t("Умный бюджет превью")
                }

                MetroCheckBox {
                    text: I18n.t("Ограничивать одновременные live-preview")
                    checked: page.settings.smartStreamBudget
                    Layout.fillWidth: true
                    onToggled: page.settings.smartStreamBudget = checked
                }

                FieldLabel {
                    text: I18n.t("Максимум активных превью")
                    color: page.settings.smartStreamBudget ? Theme.textMuted : Theme.textFaint
                }

                SettingsSpinBox {
                    from: 1
                    to: 64
                    value: page.settings.maxPreviewStreams
                    enabled: page.settings.smartStreamBudget
                    Layout.preferredWidth: 120
                    Layout.minimumWidth: 100
                    onValueModified: page.settings.maxPreviewStreams = value

                    ToolTip.visible: hovered && !enabled
                    ToolTip.text: I18n.t("Сначала включите умный бюджет превью")
                    ToolTip.delay: 450
                }

                Text {
                    text: I18n.t("Fullscreen, запись и аналитика не ограничиваются.")
                    color: Theme.textMuted
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                    Layout.columnSpan: streamingGrid.columns
                    Layout.fillWidth: true
                }
            }

            Separator {}

            SectionTitle {
                text: I18n.t("Настройки изображения")
            }

            GridLayout {
                id: adjustmentsGrid

                columns: page.compactLayout ? 1 : 2
                columnSpacing: 14
                rowSpacing: page.compactLayout ? 6 : 10
                Layout.fillWidth: true

                FieldLabel {
                    text: I18n.t("Поворот / Зеркало")
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    spacing: 10

                    StyledComboBox {
                        id: imageTransformCombo

                        model: ["0°", "90°", "180°", "270°"]
                        currentIndex: {
                            if (page.settings.playerOrientation === 90) return 1
                            if (page.settings.playerOrientation === 180) return 2
                            if (page.settings.playerOrientation === 270) return 3
                            return 0
                        }
                        Layout.preferredWidth: 110
                        onUserSelected: function(index) {
                            page.settings.playerOrientation = index * 90
                        }
                    }

                    MetroCheckBox {
                        text: I18n.t("Зеркально")
                        checked: page.settings.playerMirror
                        Layout.fillWidth: true
                        onToggled: page.settings.playerMirror = checked
                    }
                }

                FieldLabel {
                    text: I18n.t("Яркость") + " (" + page.settings.playerBrightness.toFixed(2) + ")"
                }

                MetroSlider {
                    from: 0.0
                    to: 2.0
                    value: page.settings.playerBrightness
                    Layout.fillWidth: true
                    onMoved: page.settings.playerBrightness = value
                    onPressedChanged: if (!pressed) page.settings.applyCurrentSettings()
                }

                FieldLabel {
                    text: I18n.t("Контраст") + " (" + page.settings.playerContrast.toFixed(2) + ")"
                }

                MetroSlider {
                    from: 0.0
                    to: 2.0
                    value: page.settings.playerContrast
                    Layout.fillWidth: true
                    onMoved: page.settings.playerContrast = value
                    onPressedChanged: if (!pressed) page.settings.applyCurrentSettings()
                }

                FieldLabel {
                    text: I18n.t("Насыщенность") + " (" + page.settings.playerSaturation.toFixed(2) + ")"
                }

                MetroSlider {
                    from: 0.0
                    to: 2.0
                    value: page.settings.playerSaturation
                    Layout.fillWidth: true
                    onMoved: page.settings.playerSaturation = value
                    onPressedChanged: if (!pressed) page.settings.applyCurrentSettings()
                }

                FieldLabel {
                    text: I18n.t("Гамма") + " (" + page.settings.playerGamma.toFixed(2) + ")"
                }

                MetroSlider {
                    from: 0.01
                    to: 3.0
                    value: page.settings.playerGamma
                    Layout.fillWidth: true
                    onMoved: page.settings.playerGamma = value
                    onPressedChanged: if (!pressed) page.settings.applyCurrentSettings()
                }

                FieldLabel {
                    text: I18n.t("Оттенок") + " (" + page.settings.playerHue + ")"
                }

                MetroSlider {
                    from: -180
                    to: 180
                    stepSize: 1
                    value: page.settings.playerHue
                    showCenterMarker: true
                    Layout.fillWidth: true
                    onMoved: page.settings.playerHue = value
                    onPressedChanged: if (!pressed) page.settings.applyCurrentSettings()
                }

                Button {
                    id: resetImageButton

                    text: I18n.t("Сбросить настройки изображения")
                    focusPolicy: Qt.StrongFocus
                    Layout.columnSpan: adjustmentsGrid.columns
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredHeight: 34
                    Layout.preferredWidth: Math.min(280, adjustmentsGrid.width)
                    background: Rectangle {
                        color: resetImageButton.down
                               ? Theme.metroTilePressed
                               : resetImageButton.hovered ? Theme.metroTileHover : Theme.metroTile
                        border.color: resetImageButton.visualFocus ? Theme.metroStrokeStrong : Theme.metroStroke
                        border.width: resetImageButton.visualFocus ? 2 : 1
                        radius: Theme.metroTileRadius
                    }
                    contentItem: Text {
                        text: resetImageButton.text
                        color: Theme.textPrimary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                    onClicked: {
                        page.settings.playerBrightness = 1.0
                        page.settings.playerContrast = 1.0
                        page.settings.playerHue = 0
                        page.settings.playerSaturation = 1.0
                        page.settings.playerGamma = 1.0
                        page.settings.playerOrientation = 0
                        page.settings.playerMirror = false
                        page.settings.applyCurrentSettings()
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
        }
    }
}
