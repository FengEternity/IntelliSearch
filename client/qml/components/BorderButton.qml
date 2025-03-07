import QtQuick
import QtQuick.Controls

Button {
    id: borderButton
    
    // 自定义属性
    property color borderColor: applicationWindow.isDarkTheme ? "#444444" : "#000000"
    property color hoverBorderColor: applicationWindow.isDarkTheme ? "#64B5F6" : "#007AFF"
    property color textColor: applicationWindow.isDarkTheme ? "#FFFFFF" : "#000000"
    property color hoverTextColor: applicationWindow.isDarkTheme ? "#64B5F6" : "#007AFF"
    property int borderWidth: 1
    property int borderRadius: 4
    
    // 默认文本颜色
    contentItem: Text {
        text: borderButton.text
        font: borderButton.font
        color: borderButton.hovered ? hoverTextColor : textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        
        // 添加颜色过渡动画
        Behavior on color {
            ColorAnimation { duration: 200 }
        }
    }
    
    // 自定义背景
    background: Rectangle {
        implicitWidth: 60
        implicitHeight: 36
        color: "transparent" // 透明背景
        border.width: borderWidth
        border.color: borderButton.hovered ? hoverBorderColor : borderColor
        radius: borderRadius
        
        // 添加颜色过渡动画
        Behavior on border.color {
            ColorAnimation { duration: 200 }
        }
    }
}