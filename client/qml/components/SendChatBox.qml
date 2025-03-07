import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Controls

Rectangle {
    id: sendChatBox
    width: parent.width
    height: contentText.contentHeight + 24
    color: "transparent"
    
    property string messageText: ""
    property int maxBubbleWidth: parent.width * 0.7
    
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
        
        Text {
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
        
        MouseArea {
            id: bubbleMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {}
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
        
        MouseArea {
            id: toolbarMouseArea
            anchors.fill: parent
            hoverEnabled: true
            propagateComposedEvents: true
            onClicked: function(mouse) {
                mouse.accepted = false;
            }
        }
        
        Row {
            id: bottomToolbar
            spacing: 8
            opacity: bubbleMouseArea.containsMouse || toolbarMouseArea.containsMouse || copyMouseArea.containsMouse || refreshMouseArea.containsMouse ? 1.0 : 0.0
            
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
                        // 使用QtQuick.Controls的Clipboard
                        Clipboard.text = messageText;
                        copySuccessMessage.visible = true;
                        copySuccessTimer.restart();
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
    
    Rectangle {
        id: copySuccessMessage
        anchors.horizontalCenter: messageBubble.horizontalCenter
        anchors.bottom: messageBubble.top
        anchors.bottomMargin: 8
        width: copySuccessText.width + 16
        height: copySuccessText.height + 8
        radius: 4
        color: applicationWindow.isDarkTheme ? "#424242" : "#333333"
        opacity: 0.9
        visible: false
        
        Text {
            id: copySuccessText
            text: "已复制到剪贴板"
            color: "#FFFFFF"
            font.pixelSize: 12
            anchors.centerIn: parent
        }
        
        Timer {
            id: copySuccessTimer
            interval: 2000
            onTriggered: copySuccessMessage.visible = false
        }
    }
}