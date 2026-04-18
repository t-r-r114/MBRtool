import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import FileReader 1.0
import DisassemblyWindows 1.0

ApplicationWindow {
    id: window2
    visible: true
    width: 800
    height: 600
    title: "反汇编查看器 - MBRtool"
    color: "#F8FAFC"

    property int currentViewMode: 16
    property bool isProcessing: false

    component ModernButton: Button {
        id: control
        property color bgColor: "#2196F3"
        property color bgHoverColor: "#42A5F5"
        property color bgPressColor: "#1976D2"
        property bool isActive: false

        implicitWidth: 140
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
            color: control.isActive ? control.bgPressColor :
                  (control.pressed ? control.bgPressColor :
                  (control.hovered ? control.bgHoverColor : control.bgColor))
            radius: 8
            border.color: control.isActive ? "#FFFFFF" : "transparent"
            border.width: control.isActive ? 2 : 0

            Behavior on color { ColorAnimation { duration: 150 } }
            scale: control.pressed ? 0.95 : (control.hovered ? 1.02 : 1.0)
            Behavior on scale { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        RowLayout {
            Layout.fillWidth: true
            spacing: 15

            ModernButton {
                text: "16位 模式"
                bgColor: currentViewMode === 16 ? "#3B82F6" : "#94A3B8"
                bgHoverColor: currentViewMode === 16 ? "#60A5FA" : "#CBD5E1"
                bgPressColor: "#2563EB"
                isActive: currentViewMode === 16
                enabled: !window2.isProcessing
                onClicked: {
                    if(currentViewMode !== 16) {
                        currentViewMode = 16
                        fileReader.loadFile("./Disassembly_16.trrsw")
                    }
                }
            }

            ModernButton {
                text: "32位 模式"
                bgColor: currentViewMode === 32 ? "#3B82F6" : "#94A3B8"
                bgHoverColor: currentViewMode === 32 ? "#60A5FA" : "#CBD5E1"
                bgPressColor: "#2563EB"
                isActive: currentViewMode === 32
                enabled: !window2.isProcessing
                onClicked: {
                    if(currentViewMode !== 32) {
                        currentViewMode = 32
                        fileReader.loadFile("./Disassembly_32.trrsw")
                    }
                }
            }

            Item { Layout.fillWidth: true }

            ModernButton {
                text: window2.isProcessing ? "⚙️ 反汇编处理中..." : "🚀 开始反汇编"
                implicitWidth: 200
                bgColor: window2.isProcessing ? "#94A3B8" : "#8B5CF6"
                bgHoverColor: window2.isProcessing ? "#94A3B8" : "#A78BFA"
                bgPressColor: "#6D28D9"
                enabled: !window2.isProcessing

                onClicked: {
                    window2.isProcessing = true;
                    processTimer.start()
                }
            }
        }

        Label {
            text: "⚠️ 注意：反汇编结果仅供参考，不一定完全准确！"
            color: "#EF4444"
            font.italic: true
            font.pixelSize: 14
            Layout.alignment: Qt.AlignHCenter
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#FFFFFF"
            radius: 10
            border.color: "#CBD5E1"
            border.width: 1

            // 加载遮罩
            Rectangle {
                anchors.fill: parent
                color: "#FFFFFF"
                opacity: 0.8
                visible: window2.isProcessing
                radius: 10
                z: 2
            }

            ColumnLayout {
                anchors.centerIn: parent
                visible: window2.isProcessing
                z: 3
                spacing: 15

                BusyIndicator {
                    Layout.alignment: Qt.AlignHCenter
                    running: window2.isProcessing
                }

                Label {
                    text: "正在执行引擎分析..."
                    font.pixelSize: 16
                    color: "#475569"
                    font.bold: true
                }
            }

            ScrollView {
                anchors.fill: parent
                anchors.margins: 15
                clip: true

                TextArea {
                    width: parent.width
                    // 动态绑定文本，避免修改C++只读属性报错
                    text: window2.isProcessing ? "正在分析二进制数据，请稍候..." : fileReader.fileContent
                    readOnly: true
                    wrapMode: TextArea.Wrap
                    font.family: Qt.platform.os === "windows" ? "Consolas" : "Courier New"
                    font.pixelSize: 14
                    color: "#1E293B"
                    background: Item {}
                }
            }
        }
    }

    FileReader { id: fileReader }
    DisassemblyWindows { id: disassemblyWindows }

    Timer {
        id: processTimer
        interval: 300
        repeat: false
        onTriggered: {
            try {
                disassemblyWindows.startDualDisassembly("")
                if (currentViewMode === 16) {
                    fileReader.loadFile("./Disassembly_16.trrsw")
                } else {
                    fileReader.loadFile("./Disassembly_32.trrsw")
                }
            } catch(e) {
                console.log("执行出错: ", e)
            } finally {
                // 确保无论发生什么，动画都会关闭
                window2.isProcessing = false
            }
        }
    }
}
