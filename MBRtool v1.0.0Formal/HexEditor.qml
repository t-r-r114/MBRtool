import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    width: 800
    height: 600

    signal backToMain()

    // 按钮样式复用组件
    Component {
        id: styledButton
        Rectangle {
            id: btn
            width: 120
            height: 40
            radius: 10
            color: hovered ? "#66ccff" : "#3399ff"
            border.color: "#007acc"
            opacity: pressed ? 0.7 : 1.0

            property alias text: label.text
            property bool hovered: false
            property bool pressed: false
            signal clicked()

            Text {
                id: label
                anchors.centerIn: parent
                color: "white"
                font.bold: true
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: btn.hovered = true
                onExited: btn.hovered = false
                onPressed: btn.pressed = true
                onReleased: btn.pressed = false
                onClicked: btn.clicked()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10

        // 顶部按钮栏
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Loader {
                sourceComponent: styledButton
                onLoaded: {
                    item.text = "← 返回"
                    item.clicked.connect(() => root.backToMain())
                }
            }

            Loader {
                sourceComponent: styledButton
                onLoaded: {
                    item.text = "保存"
                    item.clicked.connect(() => hexEditorBackend.saveFile(hexContentEdit.text))
                }
            }

            Loader {
                sourceComponent: styledButton
                onLoaded: {
                    item.text = "重新加载"
                    item.clicked.connect(() => {
                        hexEditorBackend.loadFile()
                        hexContentEdit.text = hexEditorBackend.hexContent
                    })
                }
            }

            Item { Layout.fillWidth: true }

            Label {
                text: "编辑器: " + hexEditorBackend.getFilePath()
                font.bold: true
                Layout.alignment: Qt.AlignRight
            }
        }

        // 滚动区域
        ScrollView {
            id: scrollArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            TextArea {
                id: hexContentEdit
                wrapMode: TextArea.WrapAnywhere
                selectByMouse: true
                font.family: "Courier New"
                font.pointSize: 10
                placeholderText: "Hex 数据将在此处显示"
                textFormat: TextEdit.PlainText

                Component.onCompleted: {
                    hexContentEdit.text = hexEditorBackend.hexContent
                }
            }
        }
    }
}
