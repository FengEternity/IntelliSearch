import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    id: chatPage
    color: "white"
    
    // 属性：用于接收初始查询消息
    property string initialQuery: ""
    // 添加状态属性
    property string currentStatus: ""
    
    // 监听 initialQuery 的变化
    onInitialQueryChanged: {
        if (initialQuery !== "") {
            // 清空现有消息
            messageList.model.clear()
            // 添加初始消息
            messageList.model.append({
                "message": initialQuery,
                "isUser": true
            })
        }
    }
    
    // 添加 Connections 来监听搜索结果
    Connections {
        target: searchBridge
        
        function onSearchResultsReady(results) {
            messageList.model.append({
                "message": results,
                "isUser": false
            })
            messageList.positionViewAtEnd()
        }
        
        function onSearchingChanged() {
            chatSearchBar.loading = searchBridge.searching
        }
    }
    
    // 将状态变更的处理移到单独的 Connections
    Connections {
        target: searchBridge
        function onSearchStatusChanged(status) {  // 确保 SearchBridge 中有对应的信号
            chatPage.currentStatus = status
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 聊天消息列表
        ListView {
            id: messageList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 16
            spacing: 16
            clip: true

            model: ListModel {
                id: messageModel
            }

            delegate: Column {
                width: parent.width
                spacing: 8
                property alias statusTextOpacity: statusText.opacity  // 添加属性别名

                Row {
                    id: messageRow
                    width: parent.width
                    spacing: 8
                    layoutDirection: isUser ? Qt.RightToLeft : Qt.LeftToRight

                    Rectangle {
                        id: bubble
                        width: Math.min(messageText.implicitWidth + 24, messageRow.width * 0.8)
                        height: messageText.implicitHeight + 16
                        color: isUser ? "#e3f2fd" : "#f5f5f5"
                        radius: 8

                        TextEdit {
                            id: messageText
                            anchors {
                                left: parent.left
                                right: parent.right
                                margins: 12
                                verticalCenter: parent.verticalCenter
                            }
                            text: message
                            wrapMode: TextEdit.Wrap
                            font.pixelSize: 14
                            color: "#303030"
                            readOnly: true
                            selectByMouse: true
                            textFormat: TextEdit.RichText
                            onLinkActivated: Qt.openUrlExternally(link)
                        }
                    }
                }

                // 非用户消息才显示操作按钮
                Row {
                    visible: !isUser
                    spacing: 8
                    x: isUser ? parent.width - bubble.width : 0
                    
                    Item {  // 包装复制按钮和状态提示
                        width: 40
                        height: 40
                        
                        Button {
                            id: copyButton
                            anchors.fill: parent
                            icon.source: "qrc:/icons/copy.svg"
                            flat: true
                            // ToolTip.visible: hovered
                            // ToolTip.text: "复制"
                            padding: 4
                            icon.width: 32
                            icon.height: 32
                            opacity: statusText.opacity === 0 ? 1 : 0
                            enabled: opacity > 0

                            Behavior on opacity {
                                NumberAnimation { duration: 200 }
                            }

                            onClicked: {
                                messageText.selectAll()
                                messageText.copy()
                                messageText.deselect()
                                parent.parent.parent.statusTextOpacity = 1  // 使用属性别名
                                statusTimer.restart()
                            }
                        }

                        Rectangle {
                            id: statusText
                            anchors.fill: parent
                            color: "#e3f2fd"
                            radius: 4
                            opacity: 0

                            Behavior on opacity {
                                NumberAnimation { duration: 200 }
                            }

                            Text {
                                anchors.centerIn: parent
                                text: "已复制"
                                font.pixelSize: 12
                                color: "#1976d2"
                            }
                        }
                    }

                    Button {
                        icon.source: "qrc:/icons/refresh.svg"
                        flat: true
                        // ToolTip.visible: hovered
                        // ToolTip.text: "重新生成"
                        width: 40
                        height: 40
                        padding: 4
                        icon.width: 32
                        icon.height: 32
                        onClicked: {
                            searchBridge.regenerateAnswer()
                        }
                    }
                }
            }

            // 添加滚动条
            ScrollBar.vertical: ScrollBar {
                active: true
            }
        }

        // 底部搜索栏
        SearchBar {
            id: chatSearchBar
            Layout.fillWidth: true
            Layout.margins: 16
            Layout.alignment: Qt.AlignHCenter
            
            property bool loading: false
            
            onSearch: function(query) {
                if (!loading) {
                    messageList.model.append({
                        "message": query,
                        "isUser": true
                    })
                    searchBridge.handleSearch(query)
                    messageList.positionViewAtEnd()
                }
            }
            
            // 添加加载状态指示器
            BusyIndicator {
                visible: parent.loading
                running: visible
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    rightMargin: 8
                }
                width: 24
                height: 24
            }
        }
    }

    // 修改计时器的处理
    Timer {
        id: statusTimer
        interval: 800
        onTriggered: {
            // 找到当前激活的状态文本并重置其不透明度
            for (let i = 0; i < messageList.count; i++) {
                let item = messageList.itemAtIndex(i)
                if (item && item.statusTextOpacity > 0) {
                    item.statusTextOpacity = 0
                    break
                }
            }
        }
    }
} 