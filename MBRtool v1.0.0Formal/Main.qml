import QtQuick.Window 2.12
import QtQuick.Controls 2.3
import QtQuick 2.15


Window {
    property bool showEditor: false

    width: 640
    height: 480
    visible: true
    title: qsTr("MBRtool")

    Column {
        visible: !showEditor
        id:tool
        x:50
        y:50
        spacing: 20

        Button {
            id: getBtn
            text: "保存MBR"
            width: 120
            height: 40

            background: Rectangle {
                color: getBtn.pressed ? "#1976D2" : "#2196F3"
                radius: 8

                Behavior on color {
                    ColorAnimation { duration: 100 }
                }

                scale: getBtn.pressed ? 1.06 : 1.0
                Behavior on scale {
                    NumberAnimation { duration: 100 }
                }
            }

            onClicked: {
                let result = mbrsave.mBRSAVE()
                if (result === 0) {
                    resultLabel.color = "#4CAF50"  // 成功时显示绿色
                    resultLabel.text = "MBR保存成功！"
                } else if(result === -1){
                    resultLabel.color = "#F44336"  // 失败时显示红色
                    resultLabel.text = "MBR保存失败！"
                }
            }
        }

        Button {
                    id: writeBtn
                    text: "写入MBR"
                    width: 120
                    height: 40

                    background: Rectangle {
                        color: writeBtn.pressed ? "#1976D2" : "#2196F3"
                        radius: 8

                        Behavior on color {
                            ColorAnimation { duration: 100 }
                        }

                        scale: writeBtn.pressed ? 1.06 : 1.0
                        Behavior on scale {
                            NumberAnimation { duration: 100 }
                        }
                    }

                    onClicked: {
                        let result = mbrwrite.mBRWRITE()
                        if (result === 0) {
                            resultLabel.color = "#4CAF50"
                            resultLabel.text = "MBR写入成功！"
                        } else {
                            resultLabel.color = "#F44336"
                            resultLabel.text = "MBR写入失败！"
                        }
                    }
                }

        Button {
                    id: disassembly
                    text: "查看反汇编窗口"
                    width: 120
                    height: 40

                    background: Rectangle {
                        color: disassembly.pressed ? "#1976D2" : "#2196F3"
                        radius: 8

                        Behavior on color {
                            ColorAnimation { duration: 100 }
                        }

                        scale: disassembly.pressed ? 1.06 : 1.0
                        Behavior on scale {
                            NumberAnimation { duration: 100 }
                        }
                    }

                    onClicked: {
                        disassembler.disassemblyWindows()
                        window.openDisassemblyWin()
                    }
        }

                Label {
                    id: resultLabel
                    text: "等待操作..."
                    font.pixelSize: 14
                    color: "#757575"  // 默认灰色
                }

        Label {
            x:200
            y:50
            text: "MBR工具 by:t-r-r\n\n\n警告！\n本软件不熟悉计算机的请勿使用！\n造成计算机损坏的本人概不负责！"
            font.pixelSize: 16
        }
    }

    Loader {
            source: showEditor ? "HexEditor.qml" : ""
            id: hexEditorLoader
            visible: showEditor
            anchors.fill: parent
    }

    Column {
        id:hexEdit
        visible: !showEditor
        x:300
        y:50
        spacing: 20

        Button {
            id:hexEditButton
            text: "打开十六进制编辑器"
            width: 120
            height: 40

            background: Rectangle {
                color: hexEditButton.pressed ? "#1976D2" : "#2196F3"
                radius: 8

                Behavior on color {
                    ColorAnimation { duration: 100 }
                }

                scale: hexEditButton.pressed ? 1.06 : 1.0
                Behavior on scale {
                    NumberAnimation { duration: 100 }
                }
            }

            onClicked: {
                hexEditorBackend.loadFile();
                showEditor = true
                hexEditorLoader.source = "HexEditor.qml"
                }
            }

        // 每次 source 更改后重新连接信号
        Connections {
            target: hexEditorLoader.item
            ignoreUnknownSignals: true
            function onBackToMain() {
                showEditor = false
                console.log("接收到返回信号")
                hexEditorLoader.source = ""
            }
        }
    }
}

