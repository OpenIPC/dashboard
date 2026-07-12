pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

ColumnLayout {
    id: root

    property var controller: null
    property alias searchText: settingsSearch.text
    property alias livePreviewChecked: livePreview.checked

    Layout.fillWidth: true
    Layout.fillHeight: true
    spacing: 10

    Rectangle {
        Layout.fillWidth: true
        Layout.minimumHeight: 92
        Layout.preferredHeight: Math.max(Layout.minimumHeight, settingsToolbarContent.implicitHeight + 20)
        Layout.margins: 12
        color: Theme.metroSurface
        border.color: Theme.metroStroke
        radius: Theme.metroTileRadius

        ColumnLayout {
            id: settingsToolbarContent

            anchors.fill: parent
            anchors.margins: 10
            spacing: 8

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Text {
                        Layout.fillWidth: true
                        text: root.controller ? root.controller.selectedGroupLabel() : ""
                        color: Theme.textPrimary
                        font.family: Theme.metroFontFamily
                        font.bold: true
                        font.pixelSize: 14
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.fillWidth: true
                        text: I18n.t("%1 параметров · schema этой камеры", [
                                         root.controller ? root.controller.groupFieldCount(root.controller.selectedGroupId) : 0
                                     ])
                        color: Theme.textMuted
                        font.pixelSize: 11
                        elide: Text.ElideRight
                    }
                }

                Text {
                    text: I18n.t("Изменено: %1", [root.controller ? root.controller.dirtyCount : 0])
                    color: root.controller && root.controller.dirtyCount ? Theme.metroAmber : Theme.textMuted
                    font.family: Theme.metroFontFamily
                    font.pixelSize: 12
                }
            }

            MajesticTextField {
                id: settingsSearch
                Layout.fillWidth: true
                placeholderText: I18n.t("Поиск по имени, пути или описанию…")
            }

            Flow {
                Layout.fillWidth: true
                spacing: 8

                MajesticCheckBox {
                    id: livePreview
                    text: I18n.t("Live ISP")
                    checked: true
                    enabled: root.controller && root.controller.capabilities.liveImage === true
                }

                MajesticButton {
                    text: I18n.t("Отменить")
                    enabled: root.controller && root.controller.dirtyCount > 0
                    onClicked: root.controller.resetDraft()
                }

                MajesticButton {
                    text: I18n.t("Проверить и применить")
                    primary: true
                    enabled: root.controller
                             && root.controller.dirtyCount > 0
                             && root.controller.capabilities.configWrite === true
                             && !root.controller.loading
                    onClicked: root.controller.prepareApply(root.controller.editedConfig())
                }
            }
        }
    }

    Rectangle {
        visible: root.controller && root.controller.pipelineReloadNeeded
        Layout.fillWidth: true
        Layout.leftMargin: 12
        Layout.rightMargin: 12
        Layout.preferredHeight: visible ? 54 : 0
        color: Theme.warningSurface
        border.color: Theme.metroAmber
        radius: Theme.metroTileRadius

        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10

            Text {
                Layout.fillWidth: true
                text: I18n.t("Сохранено. Структурные изменения вступят в силу после reload pipeline; видеопотоки кратко мигнут.")
                color: Theme.warningText
                wrapMode: Text.WordWrap
                font.pixelSize: 11
            }

            MajesticButton {
                text: I18n.t("Применить reload")
                primary: true
                onClicked: root.controller.triggerPipelineReload("")
            }
        }
    }

    MajesticRollbackBanner {
        controller: root.controller
    }

    Rectangle {
        visible: root.controller && root.controller.capabilities.schema !== true && root.controller.fields.length > 0
        Layout.fillWidth: true
        Layout.leftMargin: 12
        Layout.rightMargin: 12
        Layout.preferredHeight: visible ? 48 : 0
        color: Theme.warningSurface
        border.color: Theme.metroAmber
        radius: Theme.metroTileRadius

        Text {
            anchors.fill: parent
            anchors.margins: 9
            text: I18n.t("Старая сборка Majestic: чтение доступно, schema-safe запись отключена.")
            color: Theme.warningText
            wrapMode: Text.WordWrap
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.margins: 12
        Layout.topMargin: 0
        spacing: 12

        Rectangle {
            Layout.preferredWidth: 230
            Layout.fillHeight: true
            color: Theme.metroSurface
            border.color: Theme.metroStroke
            radius: Theme.metroTileRadius

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                Text {
                    text: I18n.t("Разделы Majestic")
                    color: Theme.textPrimary
                    font.family: Theme.metroFontFamily
                    font.bold: true
                    font.pixelSize: 14
                }

                Text {
                    Layout.fillWidth: true
                    text: I18n.t("Группы берутся из schema камеры. Неподдерживаемые функции не показываются.")
                    color: Theme.textMuted
                    wrapMode: Text.WordWrap
                    font.pixelSize: 10
                }

                Repeater {
                    model: root.controller ? root.controller.groups : []

                    delegate: Button {
                        id: groupButton

                        required property var modelData

                        Layout.fillWidth: true
                        implicitHeight: 42
                        onClicked: root.controller.selectGroup(groupButton.modelData.id)

                        contentItem: RowLayout {
                            Text {
                                Layout.fillWidth: true
                                text: root.controller.localizedGroupLabel(groupButton.modelData)
                                color: root.controller.selectedGroupId === groupButton.modelData.id ? Theme.textPrimary : Theme.textSecondary
                                font.bold: root.controller.selectedGroupId === groupButton.modelData.id
                                font.pixelSize: 12
                                elide: Text.ElideRight
                            }

                            Rectangle {
                                Layout.preferredWidth: 42
                                Layout.preferredHeight: 22
                                radius: 11
                                color: root.controller.selectedGroupId === groupButton.modelData.id ? Theme.metroBlue : Theme.metroSurfaceAlt

                                Text {
                                    anchors.centerIn: parent
                                    text: root.controller.groupFieldCount(groupButton.modelData.id)
                                    color: Theme.textPrimary
                                    font.bold: true
                                    font.pixelSize: 10
                                }
                            }
                        }

                        background: Rectangle {
                            radius: Theme.metroTileRadius
                            color: root.controller.selectedGroupId === groupButton.modelData.id
                                   ? "#1e3a8a"
                                   : (groupButton.hovered ? Theme.metroTileHover : Theme.controlBackground)
                            border.color: root.controller.selectedGroupId === groupButton.modelData.id ? Theme.metroBlue : Theme.metroStroke
                        }
                    }
                }

                Item { Layout.fillHeight: true }

                Text {
                    Layout.fillWidth: true
                    text: I18n.t("Все поля сохраняются diff-ом через /api/v1/config.")
                    color: Theme.textFaint
                    wrapMode: Text.WordWrap
                    font.pixelSize: 10
                }
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.width
                spacing: 12

                RowLayout {
                    visible: root.controller && root.controller.liveFieldsForGroup(root.controller.selectedGroupId).length > 0
                    Layout.fillWidth: true
                    spacing: 12

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 292
                        color: Theme.metroSurface
                        border.color: Theme.metroStroke
                        radius: Theme.metroTileRadius

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 8

                            Text {
                                text: I18n.t("Live preview")
                                color: Theme.textMuted
                                font.pixelSize: 12
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: "#000000"
                                border.color: Theme.metroBlue
                                radius: Theme.metroTileRadius

                                Text {
                                    anchors.centerIn: parent
                                    text: I18n.t("Предпросмотр берётся из текущего видеопотока Dashboard.\nMajestic endpoint: /ws/video?stream=0")
                                    color: Theme.textFaint
                                    horizontalAlignment: Text.AlignHCenter
                                    font.pixelSize: 12
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.preferredWidth: 390
                        Layout.preferredHeight: 292
                        color: Theme.metroSurface
                        border.color: Theme.metroStroke
                        radius: Theme.metroTileRadius

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 8

                            RowLayout {
                                Layout.fillWidth: true

                                Text {
                                    Layout.fillWidth: true
                                    text: I18n.t("Live adjustments")
                                    color: Theme.textPrimary
                                    font.bold: true
                                    font.pixelSize: 18
                                }

                                MajesticButton {
                                    text: I18n.t("↺ Reset all")
                                    subtle: true
                                    enabled: root.controller
                                             && root.controller.liveFieldsForGroup(root.controller.selectedGroupId).length > 0
                                             && root.controller.capabilities.resetDefaults === true
                                    onClicked: {
                                        var paths = []
                                        var live = root.controller.liveFieldsForGroup(root.controller.selectedGroupId)
                                        for (var i = 0; i < live.length; ++i)
                                            paths.push(live[i].path)
                                        root.controller.requestResetMany(paths)
                                    }
                                }
                            }

                            Repeater {
                                model: root.controller ? root.controller.liveFieldsForGroup(root.controller.selectedGroupId) : []

                                delegate: MajesticSettingFieldEditor {
                                    id: liveFieldEditor

                                    required property var modelData

                                    controller: root.controller
                                    field: liveFieldEditor.modelData
                                    compact: true
                                }
                            }
                        }
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: width > 900 ? 2 : 1
                    rowSpacing: 12
                    columnSpacing: 12

                    Repeater {
                        model: root.controller ? root.controller.sectionCardsForGroup(root.controller.selectedGroupId) : []

                        delegate: Rectangle {
                            id: sectionCard

                            required property var modelData

                            Layout.fillWidth: true
                            Layout.preferredHeight: root.controller.cardHeight(sectionCard.modelData)
                            color: Theme.metroSurface
                            border.color: Theme.metroStroke
                            radius: Theme.metroTileRadius

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 14
                                spacing: 8

                                RowLayout {
                                    Layout.fillWidth: true

                                    Text {
                                        Layout.fillWidth: true
                                        text: sectionCard.modelData.label
                                        color: Theme.textPrimary
                                        font.bold: true
                                        font.pixelSize: 18
                                        elide: Text.ElideRight
                                    }

                                    Rectangle {
                                        Layout.preferredWidth: 44
                                        Layout.preferredHeight: 22
                                        radius: Theme.metroTileRadius
                                        color: Theme.metroSurfaceAlt

                                        Text {
                                            anchors.centerIn: parent
                                            text: sectionCard.modelData.fields.length
                                            color: Theme.textMuted
                                            font.pixelSize: 10
                                            font.bold: true
                                        }
                                    }
                                }

                                Repeater {
                                    model: sectionCard.modelData.fields

                                    delegate: MajesticSettingFieldEditor {
                                        id: sectionFieldEditor

                                        required property var modelData

                                        controller: root.controller
                                        field: sectionFieldEditor.modelData
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    visible: root.controller
                             && root.controller.sectionCardsForGroup(root.controller.selectedGroupId).length === 0
                             && root.controller.liveFieldsForGroup(root.controller.selectedGroupId).length === 0
                    Layout.fillWidth: true
                    Layout.preferredHeight: visible ? 120 : 0
                    color: Theme.metroSurface
                    border.color: Theme.metroStroke
                    radius: Theme.metroTileRadius

                    Text {
                        anchors.centerIn: parent
                        text: I18n.t("В этом разделе нет доступных полей для текущей schema или фильтра поиска.")
                        color: Theme.textMuted
                        font.pixelSize: 12
                    }
                }
            }
        }
    }
}

