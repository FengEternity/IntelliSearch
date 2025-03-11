import QtQuick
import QtQuick.Controls

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
