import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import "pages"
import "components"

ApplicationWindow {
    width: 1080
    height: 720

    color: "#dddddd"

    visible: true
    title: "IntelliSearch"
    
    // 设置Material主题
    Material.theme: Material.Light
    Material.accent: "#2196F3"
    Material.primary: "#1976D2"
    Material.background: "#FFFFFF"
    Material.foreground: "#333333"

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // 添加侧边栏
        SideBar {
            id: sideBar
            expanded: false
            Layout.fillHeight: true
            Layout.preferredWidth: expanded ? expandedWidth : collapsedWidth
            Layout.maximumWidth: expanded ? expandedWidth : collapsedWidth
            Layout.minimumWidth: expanded ? expandedWidth : collapsedWidth
            stackView: stackView
        }

        // 主内容区域
        StackView {
            id: stackView
            Layout.fillWidth: true
            Layout.fillHeight: true
            initialItem: startPage
        }
    }

    StartPage {
        id: startPage
        visible: false
        onSwitchToChatPage: function(initialMessage) {
            stackView.push("qrc:/pages/ChatPage.qml", { initialMessage: initialMessage })
        }
    }

    // 添加设置页面组件
    SettingPage {
        id: settingPage
        visible: false
    }
}

