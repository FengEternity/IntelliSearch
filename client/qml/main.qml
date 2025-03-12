import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import Qt.labs.platform as Platform  // 添加平台特定功能
import IntelliSearch 1.0  // 导入IntelliSearch模块
import "pages"
import "components"

ApplicationWindow {
    id: applicationWindow
    width: 1080
    height: 720

    // 创建SearchBridge实例
    SearchBridge {
        id: searchBridge
    }

    // 修改系统主题检测方式
    property bool systemInDarkMode: {
        // macOS平台检测方式
        if (Qt.platform.os === "osx" || Qt.platform.os === "macos") {
            try {
                // 尝试直接使用ColorScheme
                return Platform.Theme.dark
            } catch(e) {
                // 回退到SystemPalette检测
                return systemPalette.window.hslLightness < 0.5
            }
        }
        // 对于其他平台
        return systemPalette.window.hslLightness < 0.5
    }
    
    // 使用系统主题或用户选择
    property bool isDarkTheme: systemInDarkMode

    // 添加系统调色板
    SystemPalette {
        id: systemPalette
        colorGroup: SystemPalette.Active
    }

    // 添加主题变化检测定时器
    Timer {
        interval: 1000  // 每秒检查一次
        running: true
        repeat: true
        onTriggered: {
            // 检测系统主题是否变化
            var currentDarkMode = (Qt.platform.os === "osx" || Qt.platform.os === "macos") ? 
                                  (Platform.Theme ? Platform.Theme.dark : systemPalette.window.hslLightness < 0.5) : 
                                  systemPalette.window.hslLightness < 0.5;
            
            // 如果用户没有手动设置主题，则跟随系统
            if (!userSetTheme && currentDarkMode !== systemInDarkMode) {
                systemInDarkMode = currentDarkMode;
                isDarkTheme = systemInDarkMode;
            }
        }
    }

    color: isDarkTheme ? "#121212" : "#dddddd"

    visible: true
    title: "IntelliSearch"
    
    // 设置Material主题
    Material.theme: isDarkTheme ? Material.Dark : Material.Light
    Material.accent: isDarkTheme ? "#64B5F6" : "#2196F3"
    Material.primary: isDarkTheme ? "#0D47A1" : "#1976D2"
    Material.background: isDarkTheme ? "#121212" : "#FFFFFF"
    Material.foreground: isDarkTheme ? "#FFFFFF" : "#333333"
    
    // 添加标题栏颜色设置
    background: Rectangle {
        color: isDarkTheme ? "#121212" : "#FFFFFF"
        
        // 添加颜色过渡动画
        Behavior on color {
            ColorAnimation { duration: 200 }
        }
    }

    // 主题切换时的过渡动画
    Behavior on color {
        ColorAnimation { duration: 200 }
    }

    // 添加属性标记用户是否手动设置了主题
    property bool userSetTheme: false
    property bool isSettingPage: false

    // 提供手动切换主题的方法
    function toggleTheme() {
        userSetTheme = true
        isDarkTheme = !isDarkTheme
    }
    
    // 组件加载完成后记录日志
    Component.onCompleted: {
        logger.info("应用程序窗口已加载")
    }

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
            color: isDarkTheme ? "#1E1E1E" : "#f5f5f5"
            searchBridge: searchBridge  // 使用SearchBridge实例
            
            // 添加颜色过渡动画
            Behavior on color {
                ColorAnimation { duration: 200 }
            }
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
        searchBridge: searchBridge  // 使用SearchBridge实例
        onSwitchToChatPage: function(initialMessage) {
            // 确保在创建 ChatPage 时传入 searchBridge
            var component = Qt.createComponent("qrc:/pages/ChatPage.qml")
            if (component.status === Component.Ready) {
                stackView.push(component, {
                    initialMessage: initialMessage,
                    searchBridge: searchBridge
                })
            } else {
                console.error("Error loading ChatPage:", component.errorString())
            }
        }
    }

    // 添加设置页面组件
    SettingPage {
        id: settingPage
        visible: false
    }
}

