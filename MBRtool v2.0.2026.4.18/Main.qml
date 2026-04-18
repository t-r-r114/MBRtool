import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    id: window1
    property bool showEditor: false

    width: 800
    height: 650
    visible: true
    title: qsTr("MBRtool - 专业版界面")
    color: "#F8FAFC"

    // 现代按钮组件
    component ModernButton: Button {
        id: control
        property color bgColor: "#2196F3"
        property color bgHoverColor: "#42A5F5"
        property color bgPressColor: "#1976D2"

        implicitWidth: 240
        implicitHeight: 64

        contentItem: Text {
            text: control.text
            font.pixelSize: 18
            font.bold: true
            color: "#FFFFFF"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
            color: control.pressed ? control.bgPressColor : (control.hovered ? control.bgHoverColor : control.bgColor)
            radius: 12

            layer.enabled: true
            opacity: control.enabled ? 1.0 : 0.5

            Behavior on color { ColorAnimation { duration: 120 } }

            scale: control.pressed ? 0.94 : (control.hovered ? 1.04 : 1.0)
            Behavior on scale {
                NumberAnimation { duration: 200; easing.type: Easing.OutBack }
            }
        }
    }

    // --- 主界面内容区（带有淡出动画） ---
    Item {
        id: mainContent
        anchors.fill: parent
        // 当打开编辑器时淡出主界面，返回时淡入
        opacity: showEditor ? 0 : 1
        visible: opacity > 0 // 完全透明时不渲染以节省性能
        Behavior on opacity { NumberAnimation { duration: 250 } }

        Label {
            id: titleLabel
            text: "MBR 磁盘工具控制台"
            font.pixelSize: 28
            font.bold: true
            color: "#1E293B"
            anchors.top: parent.top
            anchors.topMargin: 50
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // 1. 顶部操作按钮区
        RowLayout {
            id: buttonArea
            anchors.top: titleLabel.bottom
            anchors.topMargin: 35
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 80

            // 左侧按钮列
            ColumnLayout {
                spacing: 25
                Layout.alignment: Qt.AlignTop

                ModernButton {
                    text: "💾 保存 MBR"
                    onClicked: {
                        // 获取当前选择的磁盘号并作为参数传入
                        let targetDisk = diskNumberInput.value
                        let result = mbrsave.mBRSAVE(targetDisk)

                        if (result === 0) {
                            resultLabel.color = "#10B981"
                            resultLabel.text = "成功：磁盘 " + targetDisk + " 的 MBR 已安全备份"
                        } else {
                            resultLabel.color = "#EF4444"
                            resultLabel.text = "错误：磁盘 " + targetDisk + " 备份失败，请检查权限"
                        }
                    }
                }

                ModernButton {
                    text: "🛠️ 写入 MBR"
                    onClicked: {
                        let result = mbrwrite.mBRWRITE(diskNumberInput.value)
                        if (result === 0) {
                            resultLabel.color = "#10B981"
                            resultLabel.text = "成功：磁盘 " + diskNumberInput.value + " 写入完成"
                        } else {
                            resultLabel.color = "#EF4444"
                            resultLabel.text = "错误：操作被取消或写入失败"
                        }
                    }
                }

                ModernButton {
                    text: "🔍 反汇编窗口"
                    bgColor: "#8B5CF6"
                    bgHoverColor: "#A78BFA"
                    bgPressColor: "#6D28D9"
                    onClicked: {
                        let targetDisk = diskNumberInput.value

                        // 1. 强制提取最新 MBR 到 bin 文件中
                        let result = mbrsave.mBRSAVE(targetDisk)
                        if (result !== 0) {
                            resultLabel.color = "#EF4444"
                            resultLabel.text = "错误：提取磁盘 " + targetDisk + " 的 MBR 失败，请检查管理员权限"
                            return // 提取失败则终止
                        }

                        // 2. 告诉反汇编后端去读取哪个磁盘号的 bin 文件
                        disassembler.setTargetDisk(targetDisk)

                        // 3. 执行反汇编
                        disassembler.disassemblyWindows()

                        // 4. 打开反汇编界面
                        window.openDisassemblyWin()
                    }
                }
            }

            // 右侧按钮列
            ColumnLayout {
                spacing: 25
                Layout.alignment: Qt.AlignTop

                ModernButton {
                    text: "⌨️ 十六进制编辑"
                    bgColor: "#F59E0B"
                    bgHoverColor: "#FBBF24"
                    bgPressColor: "#D97706"
                    onClicked: {
                        let targetDisk = diskNumberInput.value

                        // 1. 强制提取最新 MBR 到 bin 文件中
                        let result = mbrsave.mBRSAVE(targetDisk)
                        if (result !== 0) {
                            resultLabel.color = "#EF4444"
                            resultLabel.text = "错误：提取磁盘 " + targetDisk + " 的 MBR 失败，请检查管理员权限"
                            return // 提取失败则终止
                        }

                        // 2. 告诉十六进制编辑器后端去读取哪个磁盘号的 bin 文件
                        hexEditorBackend.setTargetDisk(targetDisk)

                        // 3. 加载该文件数据
                        hexEditorBackend.loadFile()

                        // 4. 打开十六进制编辑器界面
                        showEditor = true
                    }
                }

                ModernButton {
                    text: "⚙️ 内核驱动写入"
                    // 使用红色系作为高危/底层操作的视觉反馈
                    bgColor: "#EF4444"
                    bgHoverColor: "#F87171"
                    bgPressColor: "#DC2626"
                    onClicked: {
                        resultLabel.color = "#F59E0B"
                        resultLabel.text = "系统提示：等待用户确认并执行底层写入..."

                        // 调用后端暴露的带弹窗确认的安全写入接口
                        let targetDisk = diskNumberInput.value;
                        let success = ntWriterBackend.executeSafeWrite(targetDisk, 0);

                        if (success) {
                            resultLabel.color = "#10B981"
                            resultLabel.text = "成功：内核驱动写入执行完成"
                        } else {
                            // 获取后端的错误码，如果是取消操作，我们可以优雅地提示
                            let errCode = ntWriterBackend.getLastError()
                            if (errCode === 1223) { // ERROR_CANCELLED 的系统错误码
                                resultLabel.color = "#64748B"
                                resultLabel.text = "已取消：用户中止了高危操作"
                            } else {
                                resultLabel.color = "#EF4444"
                                resultLabel.text = "错误：写入失败，系统错误码: " + errCode
                            }
                        }
                    }
                }
            }
        }

        // 2. 目标磁盘选择器
        RowLayout {
                    id: diskSelector
                    anchors.top: buttonArea.bottom
                    anchors.topMargin: 35
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 15

                    Label {
                        text: "目标磁盘"
                        font.pixelSize: 18
                        font.bold: true
                        color: "#475569"
                    }

                    SpinBox {
                        id: diskNumberInput
                        from: 0
                        to: 64
                        value: 1
                        editable: true

                        // 移除加减号指示器
                        up.indicator: null
                        down.indicator: null

                        // 自定义文本显示部分
                        contentItem: TextInput {
                            text: diskNumberInput.textFromValue(diskNumberInput.value, diskNumberInput.locale)
                            font.pixelSize: 16
                            font.bold: true
                            color: "#1E293B"
                            horizontalAlignment: Qt.AlignHCenter
                            verticalAlignment: Qt.AlignVCenter
                            readOnly: !diskNumberInput.editable
                            validator: diskNumberInput.validator
                            inputMethodHints: Qt.ImhDigitsOnly
                        }

                        // 自定义背景
                        background: Rectangle {
                            implicitWidth: 100 // 宽度缩小，因为没有加减按钮了
                            implicitHeight: 50
                            color: "#FFFFFF"
                            border.color: diskNumberInput.activeFocus ? "#2196F3" : "#E2E8F0"
                            border.width: 2
                            radius: 12

                            layer.enabled: true
                            layer.effect: Qt.createQmlObject("import QtGraphicalEffects 1.0; DropShadow { transparentBorder: true; horizontalOffset: 0; verticalOffset: 2; radius: 8; samples: 16; color: '#0D000000' }", parent)
                        }
                    }
                }

        // 3. 结果提示文本 (跟随在磁盘选择器下方)
        Label {
            id: resultLabel
            text: "系统就绪"
            anchors.top: diskSelector.bottom
            anchors.topMargin: 15
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 16
            font.italic: true
            color: "#64748B"
        }

        // 4. 底部警告框
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 40
            anchors.horizontalCenter: parent.horizontalCenter
            width: 500
            height: 80
            color: "#FFF1F2"
            radius: 12
            border.color: "#FECDD3"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 15

                Text { text: "⚠️"; font.pixelSize: 30 }

                Label {
                    text: "警告：谨慎操作。\n造成的任何损害与作者无关！"
                    font.pixelSize: 14
                    color: "#991B1B"
                    Layout.fillWidth: true
                }
            }
        }
    }

    // --- 十六进制编辑器动态加载器 ---
    Loader {
        id: hexEditorLoader
        source: showEditor ? "HexEditor.qml" : ""
        anchors.fill: parent
        // 只有组件准备好时才显示，防止白屏闪烁
        visible: status === Loader.Ready

        // 初始状态
        opacity: 0
        scale: 0.96

        onStatusChanged: {
            if (status === Loader.Ready) {
                fadeInAnimation.start()
            } else if (status === Loader.Null) {
                // 清理状态
                opacity = 0
                scale = 0.96
            }
        }

        ParallelAnimation {
            id: fadeInAnimation
            NumberAnimation {
                target: hexEditorLoader; property: "opacity"; from: 0; to: 1
                duration: 300; easing.type: Easing.OutCubic
            }
            NumberAnimation {
                target: hexEditorLoader; property: "scale"; from: 0.96; to: 1.0
                duration: 350; easing.type: Easing.OutBack
            }
        }
    }

    Connections {
        target: hexEditorLoader.item
        ignoreUnknownSignals: true
        function onBackToMain() {
            showEditor = false
        }
    }
}
