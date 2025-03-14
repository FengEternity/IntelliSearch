import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    property string text: ""
    property int linkIndex: 1  // 新增链接序号属性
    property string tagColor: "#64B5F6"
    property bool clickable: true
    
    height: parent.height - 5
    width: displayTextItem.width + 10
    radius: 8
    
    border.width: 1
    border.color: Qt.darker(tagColor, 1.1)
    color: mouseArea.containsMouse ? Qt.alpha(tagColor, 0.3) : Qt.alpha(tagColor, 0.1)
    
    Behavior on color {
        ColorAnimation { duration: 150 }
    }

    Text {
        id: displayTextItem
        anchors.centerIn: parent
        text: "link" + root.linkIndex  // 显示 link + 序号
        color: root.tagColor
        font.pixelSize: 13
        font.weight: Font.Medium
    }

    ToolTip {
        id: toolTip
        text: root.text
        delay: 500
        timeout: 5000
        visible: mouseArea.containsMouse
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: clickable ? Qt.PointingHandCursor : Qt.ArrowCursor
    }
}