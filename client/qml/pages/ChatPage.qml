import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../components"

Rectangle {
    id: root
    anchors.fill: parent
    anchors.centerIn: parent
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
    
    // 组件加载完成后处理初始消息
    Component.onCompleted: {
        if (initialMessage.trim() !== "") {
            // 这里可以处理初始消息，例如添加到聊天记录中
            console.log("收到初始消息:", initialMessage)
            // 后续可以添加将消息添加到聊天记录的逻辑
        }
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
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    // 这里可以添加聊天记录的显示组件
                    Rectangle {
                        width: parent.width
                        height: parent.height
                        color: "transparent"
                        // 后续可以添加聊天气泡等组件
                    }
                }
                
                // 底部输入框
                ChatTextField {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    // Layout.bottomMargin: 16
                    
                    onTextSubmitted: function(text) {
                        // 处理发送消息的逻辑
                        console.log("发送消息:", text)
                        // 后续可以添加将消息添加到聊天记录的逻辑
                    }
                }
            }
        }
    }
}
