import QtQuick
import QtQuick.Controls

Rectangle {
    id: chatTextField
    width: parent.width
    height: 60  // 保持原来的高度

    // 将信号移到顶层
    property alias text: textArea.text
    property alias placeholder: placeholderText.text
    signal textSubmitted(string text)

    color: applicationWindow.isDarkTheme ? "#2A2A2A" : "#ffffff"
    border.color: textArea.focus ? 
                 (applicationWindow.isDarkTheme ? "#64B5F6" : "#007AFF") : 
                 (applicationWindow.isDarkTheme ? "#444444" : "#E5E5EA")
    border.width: 1
    radius: 8
    
    // 添加颜色过渡动画
    Behavior on color {
        ColorAnimation { duration: 200 }
    }
    
    Behavior on border.color {
        ColorAnimation { duration: 200 }
    }

    Rectangle {
        // 移除重复的属性声明
        anchors.fill: parent
        color: applicationWindow.isDarkTheme ? "#2A2A2A" : "#ffffff"
        border.color: textArea.focus ? 
                     (applicationWindow.isDarkTheme ? "#64B5F6" : "#007AFF") : 
                     (applicationWindow.isDarkTheme ? "#444444" : "#E5E5EA")
        border.width: 1
        radius: 8
        
        // 添加颜色过渡动画
        Behavior on color {
            ColorAnimation { duration: 200 }
        }
        
        Behavior on border.color {
            ColorAnimation { duration: 200 }
        }

        Button {
            id: sendButton
            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
                rightMargin: 8
            }
            width: 36
            height: 36
            icon.color: "#d1000000"
            font.kerning: false
            flat: false

            background: Image {
                source: "qrc:/resources/icons/actions/send.svg"
                opacity: 0.5
                sourceSize.width: 24
                sourceSize.height: 24
                fillMode: Image.PreserveAspectFit
            }

            onClicked: {
                if (textArea.text.trim() !== "") {
                    chatTextField.textSubmitted(textArea.text.trim());
                    textArea.text = "";
                }
            }
        }

        TextArea {
            id: textArea
            anchors {
                left: parent.left
                right: sendButton.left
                top: parent.top
                bottom: parent.bottom
                margins: 8
            }
            font.pixelSize: 14
            color: applicationWindow.isDarkTheme ? "#FFFFFF" : "#000000"
            wrapMode: TextArea.Wrap
            background: null
            verticalAlignment: TextEdit.AlignTop
            padding: 4
            topPadding: 4
            bottomPadding: 4
            
            // 添加颜色过渡动画
            Behavior on color {
                ColorAnimation { duration: 200 }
            }

            Text {
                id: placeholderText
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                    margins: 0
                }
                verticalAlignment: Text.AlignTop
                text: "你想了解些什么..."
                color: applicationWindow.isDarkTheme ? "#777777" : "#C7C7CC"
                visible: !textArea.text && !textArea.activeFocus
                font.pixelSize: 14
                
                // 添加颜色过渡动画
                Behavior on color {
                    ColorAnimation { duration: 200 }
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
                verticalCenter: parent.verticalCenter  // 通过 anchors 设置垂直居中
            }

            // 附件按钮
            ToolButton {
                id: attachmentButton
                width: 30
                height: 30
                icon.source: "qrc:/resources/icons/actions/model.svg"
                icon.color: "#666666"
                opacity: 0.8
                onClicked:
                // 处理附件功能
                {}
            }
        }
    }
}
