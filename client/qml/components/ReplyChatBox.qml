import QtQuick

Rectangle {
    id: replyChatBox
    width: parent.width
    height: contentText.contentHeight + 24
    color: "transparent"
    
    property string messageText: ""
    property int maxBubbleWidth: parent.width * 0.7
    
    Rectangle {
        id: messageBubble
        anchors.left: parent.left
        anchors.leftMargin: 16
        anchors.top: parent.top
        anchors.topMargin: 8
        width: Math.min(contentText.implicitWidth + 16, maxBubbleWidth)
        height: contentText.contentHeight + 16
        radius: 8
        color: applicationWindow.isDarkTheme ? "#1E1E1E" : "#ffffff"
        border.color: applicationWindow.isDarkTheme ? "#333333" : "#3ee0e0e0"
        border.width: 1
        
        Behavior on color {
            ColorAnimation { duration: 200 }
        }
        
        Behavior on border.color {
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
        anchors.left: messageBubble.left
        anchors.top: messageBubble.bottom
        anchors.topMargin: 4
        width: bottomToolbar.width
        height: bottomToolbar.height
        color: "transparent"
        
        Row {
            id: bottomToolbar
            spacing: 8
            opacity: 1.0
            
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
                        if (contentText.selectedText === "") {
                            contentText.selectAll()
                            contentText.copy()
                            contentText.deselect()
                        } else {
                            contentText.copy()
                        }
                    }
                }
            }
            
            Rectangle {
                id: regenerateButton
                width: 20
                height: 20
                radius: 10
                color: regenerateMouseArea.containsMouse ? 
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
                    id: regenerateMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        console.log("重新生成回复: " + messageText)
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