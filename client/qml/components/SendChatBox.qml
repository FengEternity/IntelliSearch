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
        width: Math.min(contentText.implicitWidth + 16, maxBubbleWidth)
        height: contentText.contentHeight + 16
        radius: 8
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
        
        // 添加鼠标区域来检测悬停
        MouseArea {
            id: bubbleMouseArea
            anchors.fill: parent
            hoverEnabled: true
            // 不处理点击事件，只用于检测悬停
            onClicked: {}
        }
    }
    
    // 底部工具栏（包含时间和按钮）
    Rectangle {
        id: toolbarBackground
        anchors.right: messageBubble.right
        anchors.top: messageBubble.bottom
        anchors.topMargin: 4
        width: bottomToolbar.width
        height: bottomToolbar.height
        color: "transparent"
        
        // 添加鼠标区域来检测工具栏悬停，但不拦截子组件的鼠标事件
        MouseArea {
            id: toolbarMouseArea
            anchors.fill: parent
            hoverEnabled: true
            propagateComposedEvents: true // 允许事件传递给子组件
            // 不处理点击事件，只用于检测悬停
            onClicked: function(mouse) {
                mouse.accepted = false; // 不消费事件，允许传递给子组件
            }
        }
        
        Row {
            id: bottomToolbar
            spacing: 8
            opacity: bubbleMouseArea.containsMouse || toolbarMouseArea.containsMouse || copyMouseArea.containsMouse || refreshMouseArea.containsMouse ? 1.0 : 0.0 // 根据所有鼠标区域的悬停状态控制可见性
            
            // 添加透明度过渡动画
            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }
            
            // 复制按钮
            Rectangle {
                id: copyButton
                width: 20
                height: 20
                radius: 10
                color: copyMouseArea.containsMouse ? 
                      (applicationWindow.isDarkTheme ? "#444444" : "#E0E0E0") : 
                      "transparent"
                
                Image {
                    anchors.centerIn: parent
                    source: "qrc:/resources/icons/actions/copy.svg"
                    width: 14
                    height: 14
                    sourceSize.width: 14
                    sourceSize.height: 14
                    opacity: applicationWindow.isDarkTheme ? 0.7 : 0.6
                }
                
                MouseArea {
                    id: copyMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        // 复制消息内容到剪贴板
                        var clipboard = Qt.createQmlObject('import QtQuick; QtObject { function setText(text) { console.log("复制到剪贴板: " + text) } }', parent, 'Clipboard');
                        clipboard.setText(messageText);
                        // 可以添加一个提示，表示复制成功
                        console.log("消息已复制到剪贴板");
                    }
                }
            }
            
            // 刷新按钮
            Rectangle {
                id: refreshButton
                width: 20
                height: 20
                radius: 10
                color: refreshMouseArea.containsMouse ? 
                      (applicationWindow.isDarkTheme ? "#444444" : "#E0E0E0") : 
                      "transparent"
                
                Image {
                    anchors.centerIn: parent
                    source: "qrc:/resources/icons/navigations/refresh.svg"
                    width: 14
                    height: 14
                    sourceSize.width: 14
                    sourceSize.height: 14
                    opacity: applicationWindow.isDarkTheme ? 0.7 : 0.6
                }
                
                MouseArea {
                    id: refreshMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        // 重新发送消息
                        console.log("重新发送消息: " + messageText);
                        // 这里可以添加重新发送消息的逻辑
                    }
                }
            }
            
            // 发送时间
            Text {
                id: timeText
                text: Qt.formatDateTime(new Date(), "hh:mm")
                font.pixelSize: 10
                color: applicationWindow.isDarkTheme ? "#AAAAAA" : "#888888"
                anchors.verticalCenter: copyButton.verticalCenter
                
                // 添加颜色过渡动画
                Behavior on color {
                    ColorAnimation { duration: 200 }
                }
            }
        }
    }
}