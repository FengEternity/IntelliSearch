import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import IntelliSearch 1.0  

Dialog {
    id: linkInputDialog
    width: 400
    height: 400
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    anchors.centerIn: parent
    // title: "添加链接"
    standardButtons: Dialog.Ok | Dialog.Cancel

    SearchBridge {
        id: searchBridge
    }
    
    // 自定义属性
    property var linkList: []
    property bool isCrawling: false
    // property var searchBridge: null
    signal linksSubmitted(var links)
    signal crawlingStarted()
    signal crawlingError(string errorMessage)
    signal crawlingCompleted()

    // 对话框背景
    background: Rectangle {
        color: applicationWindow.isDarkTheme ? "#2A2A2A" : "#ffffff"
        border.color: applicationWindow.isDarkTheme ? "#444444" : "#E5E5EA"
        border.width: 1
        radius: 8
        
        // 添加颜色过渡动画
        Behavior on color {
            ColorAnimation {
                duration: 200
            }
        }
        
        Behavior on border.color {
            ColorAnimation {
                duration: 200
            }
        }
    }
    
    // 对话框内容
    contentItem: Rectangle {
        color: "transparent"
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 10
            
            // 顶部输入框
            Rectangle {
                Layout.fillWidth: true
                height: 50
                color: applicationWindow.isDarkTheme ? "#2A2A2A" : "#ffffff"
                border.color: linkTextField.focus ? 
                    (applicationWindow.isDarkTheme ? "#64B5F6" : "#007AFF") : 
                    (applicationWindow.isDarkTheme ? "#444444" : "#E5E5EA")
                border.width: 1
                radius: 8
                
                // 添加颜色过渡动画
                Behavior on border.color {
                    ColorAnimation {
                        duration: 200
                    }
                }
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8
                    
                    TextField {
                        id: linkTextField
                        Layout.fillWidth: true
                        placeholderText: "输入链接..."
                        color: applicationWindow.isDarkTheme ? "#FFFFFF" : "#000000"
                        placeholderTextColor: applicationWindow.isDarkTheme ? "#777777" : "#C7C7CC"
                        background: null
                        selectByMouse: true
                        font.pixelSize: 14
                        
                        // 添加颜色过渡动画
                        Behavior on color {
                            ColorAnimation {
                                duration: 200
                            }
                        }
                        
                        // 回车键添加链接
                        Keys.onReturnPressed: {
                            addLink()
                        }
                    }
                    
                    Button {
                        id: addButton
                        icon.source: "qrc:/resources/icons/navigations/add.svg"
                        icon.color: "#707070"
                        icon.width: 20
                        icon.height: 20
                        display: AbstractButton.IconOnly
                        // Layout.alignment: Qt.AlignHCenter
                        Layout.fillHeight: true
                        
                        background: HoverBackground {
                            isHovered: parent.hovered
                        }
                        
                        onClicked: {
                            addLink()
                        }
                    }
                }
            }
            
            // 链接列表
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: applicationWindow.isDarkTheme ? "#1E1E1E" : "#F5F5F5"
                radius: 8
                
                // 添加颜色过渡动画
                Behavior on color {
                    ColorAnimation {
                        duration: 200
                    }
                }
                
                ListView {
                    id: linkListView
                    anchors.fill: parent
                    anchors.margins: 8
                    clip: true
                    model: linkList
                    spacing: 8
                    
                    delegate: Rectangle {
                        width: linkListView.width
                        height: 40
                        color: applicationWindow.isDarkTheme ? "#333333" : "#FFFFFF"
                        radius: 6
                        
                        // 添加颜色过渡动画
                        Behavior on color {
                            ColorAnimation {
                                duration: 200
                            }
                        }
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 8
                            
                            Text {
                                Layout.fillWidth: true
                                text: modelData
                                color: applicationWindow.isDarkTheme ? "#FFFFFF" : "#000000"
                                elide: Text.ElideMiddle
                                font.pixelSize: 14
                                
                                // 添加颜色过渡动画
                                Behavior on color {
                                    ColorAnimation {
                                        duration: 200
                                    }
                                }
                            }
                            
                            Button {
                                icon.source: "qrc:/resources/icons/navigations/delete.svg"
                                icon.color: "#707070"
                                icon.width: 16
                                icon.height: 16
                                display: AbstractButton.IconOnly
                                // Layout.alignment: Qt.AlignHCenter
                                Layout.fillHeight: true
                                
                                background: HoverBackground {
                                    isHovered: parent.hovered
                                }
                                
                                onClicked: {
                                    removeLink(index)
                                }
                            }
                        }
                    }
                    
                    // 空列表提示
                    Text {
                        anchors.centerIn: parent
                        text: "暂无链接，请在上方输入框添加"
                        color: applicationWindow.isDarkTheme ? "#777777" : "#C7C7CC"
                        font.pixelSize: 14
                        visible: linkList.length === 0
                        
                        // 添加颜色过渡动画
                        Behavior on color {
                            ColorAnimation {
                                duration: 200
                            }
                        }
                    }
                }
            }
        }
    }
    
    // 确认按钮
    onAccepted: {
        if (linkList.length > 0) {
            // 验证所有链接
            let validLinks = linkList.filter(url => isValidUrl(url))
            if (validLinks.length > 0) {
                linksSubmitted(validLinks)
                crawlingStarted()
                
                // 如果有searchBridge属性，则调用其startCrawling方法
                if (searchBridge) {
                    searchBridge.startCrawling(validLinks)
                    console.log("URLs sent to crawler: " + validLinks.join(", "))
                } else {
                    console.warn("SearchBridge not available, cannot start crawling")
                }
            }
        }
    }
    
    // 取消按钮
    onRejected: {
        linkList = []
    }
    
    // 添加链接函数
    function addLink() {
        let url = linkTextField.text.trim()
        if (url !== "" && isValidUrl(url)) {
            // 检查是否已存在相同链接
            if (!linkList.includes(url)) {
                linkList.push(url)
                linkListView.model = linkList
                linkTextField.text = ""
            }
        }
    }
    
    // 移除链接函数
    function removeLink(index) {
        if (index >= 0 && index < linkList.length) {
            linkList.splice(index, 1)
            linkListView.model = linkList
        }
    }
    
    // 简单的URL验证函数
    function isValidUrl(url) {
        // 简单验证，可以根据需要增加更复杂的验证
        return url.startsWith("http://") || url.startsWith("https://")
    }
    
    // 打开对话框时清空输入框并聚焦
    onOpened: {
        linkTextField.text = ""
        linkTextField.forceActiveFocus()
    }
}
