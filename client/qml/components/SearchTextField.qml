import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

TextField {
    id: searchTextField
    
    signal search(string text)
    
    Layout.fillWidth: true
    Layout.leftMargin: 12
    placeholderText: "搜索..."
    
    onAccepted: {
        if (text.trim() !== "") {
            search(text)
            text = ""  // 清空输入框内容
        }
    }
    
    background: Rectangle {
        radius: 8
        border.width: 0
        border.color: "#e0e0e0"
        opacity: 0
    }
}