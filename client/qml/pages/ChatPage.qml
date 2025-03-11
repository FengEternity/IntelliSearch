import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../components"

Rectangle {
    id: root
    width: parent.width
    height: parent.height
    color: applicationWindow.isDarkTheme ? "#1E1E1E" : "#ffffff"
    radius: 10
    border.color: applicationWindow.isDarkTheme ? "#333333" : "#dddddd"
    border.width: 1
    
    Behavior on color {
        ColorAnimation { duration: 200 }
    }
    
    Behavior on border.color {
        ColorAnimation { duration: 200 }
    }
    
    property string initialMessage: ""
    property bool isSearching: searchBridge.isSearching
    
    ListModel {
        id: chatModel
    }
    
    // 组件加载完成后处理初始消息
    Component.onCompleted: {
        if (initialMessage.trim() !== "") {
            console.log("收到初始消息:", initialMessage)
            addMessage(initialMessage, true)
            // 发送初始消息到后端处理
            searchBridge.handleSearch(initialMessage)
        }
        
        // 连接搜索结果信号
        searchBridge.searchResultsReady.connect(function(results) {
            console.log("收到搜索结果:", results)
            try {
                var jsonResult = JSON.parse(results)
                addMessage(JSON.stringify(jsonResult, null, 2), false)
            } catch (e) {
                console.error("解析搜索结果出错:", e)
                addMessage("抱歉，处理您的请求时出现错误。", false)
            }
        })
    }
    
    // 添加消息到聊天记录的函数
    function addMessage(text, isUserMessage) {
        chatModel.append({
            "messageText": text,
            "isUserMessage": isUserMessage
        })
        // 滚动到底部
        chatListView.positionViewAtEnd()
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 0
        color: applicationWindow.isDarkTheme ? "#121212" : "#F9F9F9"
        
        Behavior on color {
            ColorAnimation { duration: 200 }
        }
        
        Item {
            anchors.fill: parent
            anchors.margins: 16
            
            ScrollView {
                id: chatScrollView
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    bottom: chatTextField.top
                    bottomMargin: 16
                }
                clip: true
                
                ListView {
                    id: chatListView
                    anchors.fill: parent
                    model: chatModel
                    spacing: 8
                    delegate: Rectangle {
                        width: ListView.view.width
                        height: messageItem.height
                        color: "transparent"
                        
                        Loader {
                            id: messageItem
                            width: parent.width
                            sourceComponent: model.isUserMessage ? userMessageComponent : botMessageComponent
                            onLoaded: {
                                item.messageText = model.messageText
                                item.maxBubbleWidth = chatListView.width * 0.7
                            }
                        }
                    }
                    
                    onCountChanged: {
                        positionViewAtEnd()
                    }
                }
            }
            
            ChatTextField {
                id: chatTextField
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                
                onTextSubmitted: function(text) {
                    console.log("发送消息:", text)
                    addMessage(text, true)
                    searchBridge.handleSearch(text)
                }
            }
        }
    }
    
    // 用户消息组件
    Component {
        id: userMessageComponent
        SendChatBox {
            onEditRequested: function(text) {
                // 处理编辑请求，将文本设置到输入框
                console.log("接收到编辑请求:", text);
                chatTextField.text = text;
            }
        }
    }
    
    // 机器人消息组件
    Component {
        id: botMessageComponent
        ReplyChatBox {
            messageText: model.messageText
            maxBubbleWidth: chatListView.width * 0.7
        }
    }
}
