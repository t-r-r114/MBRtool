import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    width: 800
    height: 600

    signal backToMain()

    component ModernButton: Button {
        id: control
        property color bgColor: "#2196F3"
        property color bgHoverColor: "#42A5F5"
        property color bgPressColor: "#1976D2"

        implicitWidth: 130
        implicitHeight: 44

        contentItem: Text {
            text: control.text
            font.pixelSize: 15
            font.bold: true
            color: "#FFFFFF"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
            color: control.pressed ? control.bgPressColor : (control.hovered ? control.bgHoverColor : control.bgColor)
            radius: 8
            Behavior on color { ColorAnimation { duration: 120 } }
            scale: control.pressed ? 0.95 : (control.hovered ? 1.03 : 1.0)
            Behavior on scale { NumberAnimation { duration: 200; easing.type: Easing.OutBack } }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent" // 使用外层的背景色
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        RowLayout {
            Layout.fillWidth: true
            spacing: 15

            ModernButton {
                text: "⬅️ 返回主页"
                bgColor: "#64748B"; bgHoverColor: "#94A3B8"; bgPressColor: "#475569"
                onClicked: root.backToMain()
            }

            ModernButton {
                text: "💾 保存修改"
                bgColor: "#10B981"; bgHoverColor: "#34D399"; bgPressColor: "#059669"
                onClicked: hexEditorBackend.saveFile(hexContentEdit.text)
            }

            ModernButton {
                text: "🔄 重新加载"
                bgColor: "#3B82F6"; bgHoverColor: "#60A5FA"; bgPressColor: "#2563EB"
                onClicked: {
                    hexEditorBackend.loadFile()
                    hexContentEdit.text = hexEditorBackend.hexContent
                }
            }

            Item { Layout.fillWidth: true }

            ColumnLayout {
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                spacing: 4

                Label {
                    text: "十六进制编辑器"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#1E293B"
                    Layout.alignment: Qt.AlignRight
                }
                Label {
                    text: "当前文件: " + hexEditorBackend.getFilePath()
                    font.pixelSize: 13
                    color: "#64748B"
                    Layout.alignment: Qt.AlignRight
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#FFFFFF"
            radius: 10
            border.color: "#CBD5E1"
            border.width: 1
            layer.enabled: true

            ScrollView {
                id: scrollArea
                anchors.fill: parent
                anchors.margins: 15
                clip: true

                TextArea {
                    id: hexContentEdit
                    wrapMode: TextArea.WrapAnywhere
                    selectByMouse: true
                    font.pixelSize: 15
                    color: "#334155"
                    placeholderText: "Hex 数据将在此处显示..."
                    placeholderTextColor: "#94A3B8"
                    textFormat: TextEdit.PlainText
                    background: Item {}

                    Component.onCompleted: {
                        font.family = Qt.platform.os === "windows" ? "Consolas" : "Courier New"
                        hexContentEdit.text = hexEditorBackend.hexContent
                    }
                }
            }
        }
    }
}
