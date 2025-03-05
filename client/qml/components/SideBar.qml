import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sideBar

    property bool expanded: true
    property int expandedWidth: 200
    property int collapsedWidth: 50

    color: "#f5f5f5"
    
    width: expanded ? expandedWidth : collapsedWidth
    
    Behavior on Layout.preferredWidth {
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    Behavior on width {
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 展开/收起按钮和搜索框
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: "transparent"
        
            RowLayout {
                anchors.fill: parent
                spacing: 8
                
                TextField {
                    Layout.fillWidth: true
                    Layout.leftMargin: 12
                    visible: sideBar.expanded
                    placeholderText: "搜索..."
                    onAccepted: {
                        if (text.trim() !== "") {
                            console.log("搜索:", text)
                            // 这里可以添加搜索处理逻辑
                            text = ""  // 清空输入框内容
                        }
                    }
                    background: Rectangle {
                        radius: 8
                        border.width: 0
                        border.color: "#e0e0e0"
                        opacity : 0
                    }
                }
        
                Button {
                    Layout.rightMargin: sideBar.expanded ? 8 : 0
                    Layout.alignment: Qt.AlignRight
                    flat: true
                    onClicked: sideBar.expanded = !sideBar.expanded
                    icon.source: sideBar.expanded ? "qrc:/resources/icons/actions/toggle-left.svg" :
                                    "qrc:/resources/icons/actions/toggle-right.svg"
                    
                    background: null
                    icon.width: 20
                    icon.height: 20
                    display: AbstractButton.IconOnly
                }
            }
        }
        
        // 添加菜单项按钮
        Repeater {
            model: [
                // {icon: "qrc:/resources/icons/actions/home.svg", text: "首页"},
                // {icon: "qrc:/resources/icons/actions/search.svg", text: "搜索"},
                // {icon: "qrc:/resources/icons/actions/settings.svg", text: "设置"}
            ]
            
            delegate: Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                flat: true
                
                icon.source: modelData.icon
                text: sideBar.expanded ? modelData.text : ""
                
                icon.width: 20
                icon.height: 20
                display: sideBar.expanded ? AbstractButton.TextBesideIcon : AbstractButton.IconOnly
                
                contentItem: Row {
                    spacing: 10
                    anchors.centerIn: parent
                    
                    Image {
                        source: modelData.icon
                        width: 20
                        height: 20
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    
                    Text {
                        text: modelData.text
                        visible: sideBar.expanded
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#333333"
                    }
                }
                
                background: Rectangle {
                    color: parent.hovered ? "#e0e0e0" : "transparent"
                }
            }
        }
        
        // 菜单项列表
        Repeater {
            model: [
                // {icon: "qrc:/resources/icons/actions/home.svg", text: "首页"},
                // {icon: "qrc:/resources/icons/actions/search.svg", text: "搜索"},
                // {icon: "qrc:/resources/icons/actions/settings.svg", text: "设置"}
            ]
            
            delegate: Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                flat: true
                
                icon.source: modelData.icon
                text: sideBar.expanded ? modelData.text : ""
                
                icon.width: 20
                icon.height: 20
                display: sideBar.expanded ? AbstractButton.TextBesideIcon : AbstractButton.IconOnly
                
                contentItem: Row {
                    spacing: 10
                    anchors.centerIn: parent
                    
                    Image {
                        source: modelData.icon
                        width: 20
                        height: 20
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    
                    Text {
                        text: modelData.text
                        visible: sideBar.expanded
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#333333"
                    }
                }
                
                background: Rectangle {
                    color: parent.hovered ? "#e0e0e0" : "transparent"
                }
            }
        }
        
        // 弹性空间，将设置按钮推到底部
        Item {
            Layout.fillHeight: true
        }
        
        // 设置按钮
        Button {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            Layout.margins: 8
            flat: true
            // background: null
            
            icon.source: "qrc:/resources/icons/actions/setting.svg"
            text: sideBar.expanded ? qsTr("设置") : ""
            
            icon.width: 20
            icon.height: 20
            display: sideBar.expanded ? AbstractButton.TextBesideIcon : AbstractButton.IconOnly
            
            contentItem: Row {
                spacing: 10
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                
                Image {
                    source: "qrc:/resources/icons/actions/setting.svg"
                    width: 20
                    height: 20
                    anchors.verticalCenter: parent.verticalCenter
                }
                
                Text {
                    text: qsTr("设置")
                    visible: sideBar.expanded
                    anchors.verticalCenter: parent.verticalCenter
                    color: "#333333"
                }
            }
            
            background: Rectangle {
                color: parent.hovered ? "#e0e0e0" : "transparent"
            }
            
            onClicked: {
                console.log("打开设置")
                // 这里添加打开设置的逻辑
            }
        }
    }
}