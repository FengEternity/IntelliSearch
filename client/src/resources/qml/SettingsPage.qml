import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    id: root
    color: "white"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 32  // 增加外边距
        spacing: 24  // 调整整体间距

        // 标题栏
        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            // 标题
            Text {
                text: "设置"
                font.pixelSize: 28
                font.weight: Font.Medium
                color: "#1a1a1a"
            }

            Item { Layout.fillWidth: true }  // 占位，使标题居左

            // 返回按钮移到右边
            RoundButton {
                width: 36  // 稍微调大一点
                height: 36
                flat: true
                icon.source: "qrc:/icons/back.svg"
                icon.width: 24
                icon.height: 24
                icon.color: hovered ? Material.primary : "#666666"  // 添加悬停效果
                
                onClicked: root.parent.state = ""

                // ToolTip {
                //     visible: parent.hovered
                //     text: "返回"
                //     delay: 500
                // }

                // 添加背景效果
                background: Rectangle {
                    radius: width / 2
                    color: parent.hovered ? Qt.rgba(Material.primary.r, Material.primary.g, Material.primary.b, 0.1) : "transparent"
                    Behavior on color { ColorAnimation { duration: 150 } }
                }
            }
        }

        // 选项卡
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            Layout.topMargin: 8
            
            background: Rectangle {
                color: "transparent"
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 2
                    color: "#e0e0e0"
                }
            }

            TabButton {
                text: "模型"
                icon.source: "qrc:/icons/model.svg"
                icon.width: 20
                icon.height: 20
                icon.color: checked ? Material.primary : "#666666"
                
                contentItem: RowLayout {
                    spacing: 8
                    Item {
                        Layout.preferredWidth: 20
                        Layout.preferredHeight: 20
                        Layout.alignment: Qt.AlignVCenter

                        Image {
                            anchors.fill: parent
                            source: parent.parent.parent.icon.source
                            sourceSize.width: parent.width
                            sourceSize.height: parent.height
                            opacity: parent.parent.parent.checked ? 1.0 : 0.7
                        }
                    }
                    Text {
                        text: parent.parent.text
                        color: parent.parent.checked ? Material.primary : "#666666"
                        font.pixelSize: 14
                    }
                }

                background: Rectangle {
                    color: "transparent"
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 2
                        color: parent.parent.checked ? Material.primary : "transparent"
                    }
                }
            }

            TabButton {
                text: "对话"
                icon.source: "qrc:/icons/chat.svg"
                icon.width: 20
                icon.height: 20
                icon.color: checked ? Material.primary : "#666666"
                
                contentItem: RowLayout {
                    spacing: 8
                    Item {
                        Layout.preferredWidth: 20
                        Layout.preferredHeight: 20
                        Layout.alignment: Qt.AlignVCenter

                        Image {
                            anchors.fill: parent
                            source: parent.parent.parent.icon.source
                            sourceSize.width: parent.width
                            sourceSize.height: parent.height
                            opacity: parent.parent.parent.checked ? 1.0 : 0.7
                        }
                    }
                    Text {
                        text: parent.parent.text
                        color: parent.parent.checked ? Material.primary : "#666666"
                        font.pixelSize: 14
                    }
                }

                background: Rectangle {
                    color: "transparent"
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 2
                        color: parent.parent.checked ? Material.primary : "transparent"
                    }
                }
            }

            TabButton {
                text: "其他"
                icon.source: "qrc:/icons/more.svg"
                icon.width: 20
                icon.height: 20
                icon.color: checked ? Material.primary : "#666666"
                
                contentItem: RowLayout {
                    spacing: 8
                    Item {
                        Layout.preferredWidth: 20
                        Layout.preferredHeight: 20
                        Layout.alignment: Qt.AlignVCenter

                        Image {
                            anchors.fill: parent
                            source: parent.parent.parent.icon.source
                            sourceSize.width: parent.width
                            sourceSize.height: parent.height
                            opacity: parent.parent.parent.checked ? 1.0 : 0.7
                        }
                    }
                    Text {
                        text: parent.parent.text
                        color: parent.parent.checked ? Material.primary : "#666666"
                        font.pixelSize: 14
                    }
                }

                background: Rectangle {
                    color: "transparent"
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 2
                        color: parent.parent.checked ? Material.primary : "transparent"
                    }
                }
            }
        }

        // 内容区域
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            // 模型设置页
            ColumnLayout {
                spacing: 32  // 增加间距

                GridLayout {
                    columns: 2
                    columnSpacing: 24
                    rowSpacing: 24
                    Layout.fillWidth: true

                    Text {
                        text: "模型提供方:"
                        font.pixelSize: 14
                        color: "#666666"
                        Layout.alignment: Qt.AlignVCenter
                    }

                    ComboBox {
                        id: modelProvider
                        Layout.preferredWidth: 300
                        model: ["SILICONFLOW API"]
                        Material.foreground: Material.Blue
                    }

                    Text {
                        text: "API 密钥"
                        font.pixelSize: 14
                        color: "#666666"
                        Layout.alignment: Qt.AlignVCenter
                    }

                    TextField {
                        id: apiKey
                        Layout.fillWidth: true
                        Layout.minimumWidth: 300
                        echoMode: TextInput.Password
                        placeholderText: "请输入 API 密钥"
                    }

                    Text {
                        text: "模型"
                        font.pixelSize: 14
                        color: "#666666"
                        Layout.alignment: Qt.AlignVCenter
                    }

                    ComboBox {
                        id: modelSelection
                        Layout.fillWidth: true
                        Layout.minimumWidth: 300
                        model: ["模型1", "模型2", "模型3"]
                    }
                }

                // 分隔线
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#e0e0e0"
                    Layout.topMargin: 8
                    Layout.bottomMargin: 8
                }

                Text {
                    text: "上下文的消息数量上限"
                    font.pixelSize: 14
                    color: "#666666"
                }

                RowLayout {
                    spacing: 16
                    Layout.fillWidth: true
                    
                    Slider {
                        id: contextLimit
                        from: 1
                        to: 50
                        value: 38  // 根据图片调整默认值
                        stepSize: 1
                        Layout.fillWidth: true
                        
                        background: Rectangle {
                            x: contextLimit.leftPadding
                            y: contextLimit.topPadding + contextLimit.availableHeight / 2 - height / 2
                            width: contextLimit.availableWidth
                            height: 4
                            radius: 2
                            color: "#e0e0e0"

                            Rectangle {
                                width: contextLimit.visualPosition * parent.width
                                height: parent.height
                                color: Material.primary
                                radius: 2
                            }
                        }
                    }

                    TextField {
                        text: contextLimit.value
                        Layout.preferredWidth: 60
                        horizontalAlignment: Text.AlignHCenter
                        validator: IntValidator { bottom: 1; top: 50 }
                        onTextChanged: if(text) contextLimit.value = parseInt(text)
                    }
                }

                Text {
                    text: "严谨与想象(Temperature)"
                    font.pixelSize: 14
                    color: "#666666"
                }

                RowLayout {
                    spacing: 16
                    Layout.fillWidth: true

                    Text {
                        text: "严谨细致"
                        font.pixelSize: 12
                        color: "#999999"
                    }

                    Slider {
                        id: temperature
                        from: 0
                        to: 1
                        value: 0.7
                        stepSize: 0.1
                        Layout.fillWidth: true
                        
                        background: Rectangle {
                            x: temperature.leftPadding
                            y: temperature.topPadding + temperature.availableHeight / 2 - height / 2
                            width: temperature.availableWidth
                            height: 4
                            radius: 2
                            color: "#e0e0e0"

                            Rectangle {
                                width: temperature.visualPosition * parent.width
                                height: parent.height
                                color: Material.primary
                                radius: 2
                            }
                        }
                    }

                    Text {
                        text: "想象发散"
                        font.pixelSize: 12
                        color: "#999999"
                    }

                    TextField {
                        text: temperature.value.toFixed(1)
                        Layout.preferredWidth: 60
                        horizontalAlignment: Text.AlignHCenter
                        validator: DoubleValidator { bottom: 0; top: 1; decimals: 1 }
                        onTextChanged: if(text) temperature.value = parseFloat(text)
                    }
                }

                Item { Layout.fillHeight: true }

                // 底部按钮
                RowLayout {
                    Layout.alignment: Qt.AlignRight
                    spacing: 16

                    Button {
                        text: "取消"
                        flat: true
                        onClicked: root.parent.state = ""
                        Material.foreground: "#666666"
                    }

                    Button {
                        text: "保存"
                        highlighted: true
                        Material.accent: Material.Blue
                        onClicked: {
                            // TODO: 保存设置
                            root.parent.state = ""
                        }
                    }
                }
            }

            // 对话设置页（占位）
            Item {
                Text {
                    anchors.centerIn: parent
                    text: "对话设置（开发中...）"
                    color: "#999999"
                }
            }

            // 其他设置页（占位）
            Item {
                Text {
                    anchors.centerIn: parent
                    text: "其他设置（开发中...）"
                    color: "#999999"
                }
            }
        }
    }
} 