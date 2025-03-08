import QtQuick

Rectangle {
    id: sendChatBox
    width: parent.width
    height: contentText.contentHeight + 24
    color: "transparent"
    
    property string messageText: ""
    property int maxBubbleWidth: parent.width * 0.7
    
    // 添加编辑请求信号
    signal editRequested(string text)
    
    Rectangle {
        id: messageBubble
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.top: parent.top
        anchors.topMargin: 8
        width: Math.min(contentText.implicitWidth + 16, maxBubbleWidth)
        height: contentText.contentHeight + 16
        radius: 8
        color: applicationWindow.isDarkTheme ? "#333333" : "#e0e0e0"
        
        Behavior on color {
            ColorAnimation { duration: 200 }
        }
        
        TextEdit {
            id: contentText
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 8
            text: messageText
            color: applicationWindow.isDarkTheme ? "#FFFFFF" : "#000000"
            wrapMode: Text.Wrap
            width: Math.min(implicitWidth, maxBubbleWidth - 24)
            font.pixelSize: 14
            readOnly: true
            selectByMouse: true
            selectByKeyboard: true
            selectionColor: applicationWindow.isDarkTheme ? "#64B5F6" : "#69aef8"
            textFormat: TextEdit.PlainText
            
            Behavior on color {
                ColorAnimation { duration: 200 }
            }
            
            onImplicitWidthChanged: {
                if (implicitWidth > maxBubbleWidth - 24) {
                    width = maxBubbleWidth - 24
                } else {
                    width = implicitWidth
                }
            }
        }
        

    }
    
    Rectangle {
        id: toolbarBackground
        anchors.right: messageBubble.right
        anchors.top: messageBubble.bottom
        anchors.topMargin: 4
        width: bottomToolbar.width
        height: bottomToolbar.height
        color: "transparent"
        

        
        Row {
            id: bottomToolbar
            spacing: 8
            opacity: 1.0  // 工具栏始终显示
            
            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }
            
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
                        // 如果没有选中文本，则复制整个消息
                        if (contentText.selectedText === "") {
                            contentText.selectAll();
                            contentText.copy();
                            contentText.deselect();
                        } else {
                            // 如果已选中文本，直接复制选中部分
                            contentText.copy();
                        }
                        copySuccessMessage.visible = true;
                        copySuccessTimer.restart();
                    }
                }
            }

            Rectangle {
                id: editButton
                width: 20
                height: 20
                radius: 10
                color: editMouseArea.containsMouse?
                      (applicationWindow.isDarkTheme? "#444444" : "#E0E0E0") :
                      "transparent"

                Image {
                    anchors.centerIn: parent
                    source: "qrc:/resources/icons/actions/edit.svg"
                    width: 14
                    height: 14
                    sourceSize.width: 14
                    sourceSize.height: 14
                    opacity: applicationWindow.isDarkTheme? 0.7 : 0.6
                }
                MouseArea {
                    id: editMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        console.log("编辑消息: " + messageText);
                        // 触发编辑请求信号，将当前消息文本传递给父组件
                        sendChatBox.editRequested(messageText);
                    }
                }
            }
            
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
                        console.log("重新发送消息: " + messageText);
                    }
                }
            }
            
            Text {
                id: timeText
                text: Qt.formatDateTime(new Date(), "hh:mm")
                font.pixelSize: 10
                color: applicationWindow.isDarkTheme ? "#AAAAAA" : "#888888"
                anchors.verticalCenter: copyButton.verticalCenter
                
                Behavior on color {
                    ColorAnimation { duration: 200 }
                }
            }
        }
    }
}