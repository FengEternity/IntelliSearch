import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: chatTextField
    width: parent.width
    height: Math.min(textArea.contentHeight + 50, 150)  // 根据内容自适应高度，但最大高度降低到150

    // 将信号移到顶层
    property alias text: textArea.text
    property alias placeholder: placeholderText.text
    signal textSubmitted(string text)

    color: applicationWindow.isDarkTheme ? "#2A2A2A" : "#ffffff"
    border.color: textArea.focus ? (applicationWindow.isDarkTheme ? "#64B5F6" : "#007AFF") : (applicationWindow.isDarkTheme ? "#444444" : "#E5E5EA")
    border.width: 1
    radius: 8

    // 添加颜色过渡动画
    Behavior on color {
        ColorAnimation {
            duration: 200
        }
    }

    Behavior on border.color {
        ColorAnimation {
            duration: 200
        }
    }

    // 添加高度过渡动画
    Behavior on height {
        NumberAnimation {
            duration: 100
        }
    }

    Rectangle {
        anchors.fill: parent
        color: applicationWindow.isDarkTheme ? "#2A2A2A" : "#ffffff"
        border.color: textArea.focus ? (applicationWindow.isDarkTheme ? "#64B5F6" : "#007AFF") : (applicationWindow.isDarkTheme ? "#444444" : "#E5E5EA")
        border.width: 1
        radius: 8
        clip: true  // 防止内容溢出

        // 添加颜色过渡动画
        Behavior on color {
            ColorAnimation {
                duration: 200
            }
        }

        Behavior on border.color {
            ColorAnimation {
                duration: 200
            }
        }

        // 工具栏放在内部矩形中
        Rectangle {
            id: toolBar
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                margins: 8
            }
            height: 20
            color: "transparent"

            Row {
                spacing: 8
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }

                ToolButton {
                    id: attachmentButton
                    width: 30
                    height: 30
                    icon.source: "qrc:/resources/icons/actions/model.svg"
                    icon.color: "#666666"
                    opacity: 0.8
                    onClicked: {}
                }

                ToolButton {
                    id: linkButton
                    width: 30
                    height: 30
                    icon.source: "qrc:/resources/icons/actions/link.svg"
                    icon.color: "#666666"
                    opacity: 0.8
                    onClicked: {
                        // 创建并打开链接输入对话框
                        var component = Qt.createComponent("LinkInputDialog.qml");
                        if (component.status === Component.Ready) {
                            // 获取SearchBridge实例
                            var searchBridgeInstance = null;
                            if (applicationWindow.searchBridge) {
                                searchBridgeInstance = applicationWindow.searchBridge;
                            }

                            var dialog = component.createObject(applicationWindow, {
                                "searchBridge": searchBridgeInstance
                            });
                            dialog.open();
                            dialog.linksSubmitted.connect(function (links) {
                                // 为每个链接创建一个标签组件
                                links.forEach(function (link, index) {
                                    var tagComponent = Qt.createComponent("LinkTag.qml");
                                    if (tagComponent.status === Component.Ready) {
                                        // 从预定义的颜色列表中随机选择一个颜色
                                        var colors = ["#64B5F6", "#4CAF50", "#FF9800", "#9C27B0", "#F44336", "#009688"];
                                        var randomColor = colors[Math.floor(Math.random() * colors.length)];

                                        // 在当前光标位置插入空格
                                        var pos = textArea.cursorPosition;
                                        if (pos > 0 && textArea.text.charAt(pos - 1) !== ' ') {
                                            textArea.insert(pos, ' ');
                                            pos += 1;
                                        }

                                        // 创建并插入标签
                                        var tag = tagComponent.createObject(textArea, {
                                            "text": link,
                                            "tagColor": randomColor,
                                            "linkIndex": index + 1,
                                            "parent": textArea
                                        });

                                        // 插入标签文本并添加结尾空格
                                        // var tagText = "#link" + (index + 1);
                                        // textArea.insert(pos, tagText + ' ');
                                        // textArea.cursorPosition = pos + tagText.length + 1;
                                    } else {
                                        console.error("Error loading LinkTag:", tagComponent.errorString());
                                    }
                                });
                            });
                        } else {
                            console.error("Error loading LinkInputDialog:", component.errorString());
                        }
                    }
                }
            }
        }

        Button {
            id: sendButton
            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
                rightMargin: 8
            }
            icon.source: "qrc:/resources/icons/actions/send.svg"
            icon.color: "#707070"
            icon.width: 20
            icon.height: 20
            display: AbstractButton.IconOnly

            background: HoverBackground {
                isHovered: parent.hovered
            }

            font.kerning: false
            flat: false

            onClicked: {
                if (textArea.text.trim() !== "") {
                    chatTextField.textSubmitted(textArea.text.trim());
                    textArea.text = "";
                }
            }
        }

        ScrollView {
            id: scrollView
            anchors {
                left: parent.left
                right: sendButton.left
                top: parent.top
                bottom: toolBar.top
                margins: 8
            }
            clip: true
            ScrollBar.vertical.policy: ScrollBar.AsNeeded
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            TextArea {
                id: textArea
                width: scrollView.width
                font.pixelSize: 14
                color: applicationWindow.isDarkTheme ? "#FFFFFF" : "#000000"
                wrapMode: TextArea.Wrap
                background: null
                verticalAlignment: TextEdit.AlignTop
                padding: 4
                topPadding: 4
                bottomPadding: 4

                Behavior on color {
                    ColorAnimation {
                        duration: 200
                    }
                }

                Text {
                    id: placeholderText
                    anchors {
                        left: parent.left
                        right: parent.right
                        top: parent.top
                        margins: 0
                    }
                    verticalAlignment: Text.AlignTop
                    text: "你想了解些什么..."
                    color: applicationWindow.isDarkTheme ? "#777777" : "#C7C7CC"
                    visible: !textArea.text && !textArea.activeFocus
                    font.pixelSize: 14

                    Behavior on color {
                        ColorAnimation {
                            duration: 200
                        }
                    }
                }

                Keys.onReturnPressed: function (event) {
                    if (event.modifiers & Qt.ShiftModifier) {
                        event.accepted = false;
                    } else {
                        event.accepted = true;
                        if (text.trim() !== "") {
                            chatTextField.textSubmitted(text.trim());
                            text = "";
                        }
                    }
                }
            }
        }
    }
}
