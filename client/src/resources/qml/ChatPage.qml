import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    id: chatPage
    color: "white"
    
    // 属性：用于接收初始查询消息
    property string initialQuery: ""
    
    // 监听 initialQuery 的变化
    onInitialQueryChanged: {
        if (initialQuery !== "") {
            // 清空现有消息
            messageList.model.clear()
            // 添加初始消息
            messageList.model.append({
                "message": initialQuery,
                "isUser": true
            })
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 聊天消息列表
        ListView {
            id: messageList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 16
            spacing: 16
            clip: true

            model: ListModel {
                id: messageModel
            }

            delegate: Row {
                id: messageRow
                width: parent.width
                spacing: 8
                layoutDirection: isUser ? Qt.RightToLeft : Qt.LeftToRight

                Rectangle {
                    id: bubble
                    width: Math.min(messageContent.implicitWidth + 24, messageRow.width * 0.8)
                    height: messageContent.implicitHeight + 16
                    color: isUser ? "#e3f2fd" : "#f5f5f5"
                    radius: 8

                    Text {
                        id: messageContent
                        anchors.centerIn: parent
                        width: bubble.width - 24
                        text: message
                        wrapMode: Text.Wrap
                        font.pixelSize: 14
                        color: "#303030"
                        horizontalAlignment: Text.AlignLeft
                    }
                }
            }
        }

        // 底部搜索栏
        SearchBar {
            id: chatSearchBar
            Layout.fillWidth: true
            Layout.margins: 16
            Layout.alignment: Qt.AlignHCenter
            
            onSearch: function(query) {
                messageList.model.append({
                    "message": query,
                    "isUser": true
                })
                // TODO: 处理消息发送逻辑
                searchBridge.handleSearch(query) 
            }
        }
    }
} 