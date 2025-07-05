import QtQuick 2.15
import QtQuick.Controls 2.15
import FileReader 1.0

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: "Read File Example"

    Column {
        anchors.fill: parent
        spacing: 10
        padding: 20

        // 按钮：加载文件
        Button {
            text: "Load File"
            width: parent.width - 40
            onClicked: {
                fileReader.loadFile("./Disassembly.trrsw")
            }
        }

        // 提示文字（红色、斜体）
        Text {
            text: "注意：反汇编结果不一定准确！"
            color: "red"
            font.italic: true
            width: parent.width - 40
            horizontalAlignment: Text.AlignHCenter  // 水平居中
        }

        // 滚动视图包裹文本区域
        ScrollView {
            width: parent.width - 40
            height: parent.height - 120  // 减去按钮和提示文字的高度
            clip: true  // 确保内容不会溢出到视图外

            TextArea {
                width: parent.width  // 宽度设为与ScrollView相同
                text: fileReader.fileContent
                readOnly: true
                wrapMode: TextArea.Wrap
                font.pixelSize: 12
                background: Rectangle {  // 添加浅灰色背景
                    color: "#f0f0f0"
                    border.color: "#cccccc"
                }
            }
        }
    }

    FileReader {
        id: fileReader
    }
}
