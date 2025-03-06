import QtQuick

Rectangle {
    id: sendChatBox
    width: parent.width
    height: contentText.contentHeight + 24 // 动态高度，根据文本内容调整
    // 设置背景为透明
    color: "transparent"
    
    property string messageText: ""
    property int maxBubbleWidth: parent.width * 0.7 // 气泡最大宽度为父容器的70%
    
    // 消息气泡
    Rectangle {
        id: messageBubble
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.top: parent.top
        anchors.topMargin: 8
        width: Math.min(contentText.implicitWidth + 24, maxBubbleWidth)
        height: contentText.contentHeight + 16
        radius: 12
        color: applicationWindow.isDarkTheme ? "#333333" : "#e0e0e0" // 蓝色气泡
        
        // 添加颜色过渡动画
        Behavior on color {
            ColorAnimation { duration: 200 }
        }
        
        // 消息文本
        Text {
            id: contentText
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 8
            text: messageText
            color: applicationWindow.isDarkTheme ? "#FFFFFF" : "#000000" // 根据主题调整文字颜色
            wrapMode: Text.Wrap
            width: Math.min(implicitWidth, maxBubbleWidth - 24)
            font.pixelSize: 14
            
            // 添加颜色过渡动画
            Behavior on color {
                ColorAnimation { duration: 200 }
            }
            
            // 限制最大宽度
            onImplicitWidthChanged: {
                if (implicitWidth > maxBubbleWidth - 24) {
                    width = maxBubbleWidth - 24
                } else {
                    width = implicitWidth
                }
            }
        }
    }
    
    // 发送时间（可选）
    Text {
        id: timeText
        anchors.right: messageBubble.right
        anchors.top: messageBubble.bottom
        anchors.topMargin: 4
        text: Qt.formatDateTime(new Date(), "hh:mm")
        font.pixelSize: 10
        color: applicationWindow.isDarkTheme ? "#AAAAAA" : "#888888"
        
        // 添加颜色过渡动画
        Behavior on color {
            ColorAnimation { duration: 200 }
        }
    }
}