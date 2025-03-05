import QtQuick

Rectangle {
    property bool isHovered: false
    property int marginSize: 2
    
    anchors {
        left: parent.left
        right: parent.right
        margins: marginSize
    }
    color: isHovered ? "#b6e5e5e5" : "transparent"
    radius: 8
}