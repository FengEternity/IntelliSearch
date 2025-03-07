import QtQuick

Rectangle {
    property bool isHovered: false
    property int marginSize: 2
    
    anchors {
        left: parent.left
        right: parent.right
        margins: marginSize
    }
    color: isHovered ? 
           (applicationWindow.isDarkTheme ? "#444444" : "#b6e5e5e5") : 
           "transparent"
    radius: 8
    
    // 添加颜色过渡动画
    // Behavior on color {
    //     ColorAnimation { duration: 200 }
    // }
}