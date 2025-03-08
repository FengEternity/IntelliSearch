import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../components"

Rectangle {
    id: root
    // 移除冲突的anchors属性，使用width和height替代
    width: parent.width
    height: parent.height
    color: applicationWindow.isDarkTheme ? "#1E1E1E" : "#ffffff"
    radius: 10
    border.color: applicationWindow.isDarkTheme ? "#333333" : "#dddddd"
    border.width: 1
    
    // 添加颜色过渡动画
    Behavior on color {
        ColorAnimation { duration: 200 }
    }
    
    Behavior on border.color {
        ColorAnimation { duration: 200 }
    }
    
    // 添加属性接收初始消息
    property string initialMessage: ""
    
    // 添加消息列表模型
    ListModel {
        id: chatModel
    }
    
    // 组件加载完成后处理初始消息
    Component.onCompleted: {
        if (initialMessage.trim() !== "") {
            // 这里可以处理初始消息，例如添加到聊天记录中
            console.log("收到初始消息:", initialMessage)
            addMessage(initialMessage, true)
        }
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

    RowLayout {
        anchors.fill: parent
        spacing: 0
        
        // 聊天主区域
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 0
            color: applicationWindow.isDarkTheme ? "#121212" : "#F9F9F9"
            
            // 添加颜色过渡动画
            Behavior on color {
                ColorAnimation { duration: 200 }
            }
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 10
                
                // 聊天记录区域
                ScrollView {
                    id: chatScrollView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    background: Rectangle {
                        color: "transparent"
                    }
                    
                    ListView {
                        id: chatListView
                        anchors.fill: parent
                        model: chatModel
                        spacing: 8
                        delegate: Rectangle {  // 将 Item 改为 Rectangle
                            width: ListView.view.width
                            height: messageItem.height
                            // 确保背景透明
                            color: "transparent"
                            
                            // 根据消息类型选择不同的组件
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
                        
                        // 自动滚动到底部
                        onCountChanged: {
                            positionViewAtEnd()
                        }
                    }
                }
                
                // 底部输入框
                ChatTextField {
                    id: chatTextField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    
                    onTextSubmitted: function(text) {
                        // 处理发送消息的逻辑
                        console.log("发送消息:", text)
                        // 添加用户消息到聊天记录
                        addMessage(text, true)
                        // 这里可以添加发送到后端的逻辑
                    }
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
    
    // 机器人消息组件（后续实现）
    Component {
        id: botMessageComponent
        Rectangle {
            property string messageText: ""
            property int maxBubbleWidth: 0
            width: parent.width
            height: 40
            color: "transparent"
            // 这里后续实现机器人回复的气泡
        }
    }
}
