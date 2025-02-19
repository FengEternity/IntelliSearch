import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    id: chatPage
    color: "white"
    
    // 属性：用于接收初始查询消息
    property string initialQuery: ""
    // 添加状态属性
    property string currentStatus: ""
    
    // 添加加载会话对话历史的方法
    function loadSessionDialogues(sessionId) {
        // 清空现有消息
        messageList.model.clear()
        
        // 获取会话的对话历史
        let dialogues = searchBridge.getSessionDialogues(sessionId)
        
        // 添加每条对话记录
        for (let i = 0; i < dialogues.length; i++) {
            let dialogue = dialogues[i]
            
            // 添加用户查询
            messageList.model.append({
                "message": dialogue.user_query,
                "isUser": true
            })
            
            // 添加搜索结果
            messageList.model.append({
                "message": dialogue.search_result,
                "isUser": false
            })
        }
        
        // 滚动到底部
        scrollTimer.restart()
    }
    
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
            // 使用 Timer 确保在内容渲染后再滚动
            scrollTimer.restart()
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

                // 添加计时器到 delegate 中
                Timer {
                    id: statusTimer
                    interval: 800
                    onTriggered: parent.statusTextOpacity = 0
                }

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

                        Text {
                            id: messageText
                            anchors {
                                left: parent.left
                                right: parent.right
                                margins: 12
                                verticalCenter: parent.verticalCenter
                            }
                            text: message
                            wrapMode: Text.Wrap
                            font.pixelSize: 14
                            color: "#303030"
                            textFormat: Text.RichText
                            onLinkActivated: Qt.openUrlExternally(link)

                            // 使用 TextEdit 作为选择层
                            TextEdit {
                                anchors.fill: parent
                                text: parent.text
                                readOnly: true
                                selectByMouse: true
                                selectedTextColor: parent.color
                                selectionColor: "#b3d4fc"
                                color: "transparent"
                                font: parent.font
                                wrapMode: parent.wrapMode
                                textFormat: parent.textFormat
                                
                                // 防止鼠标事件穿透到底层Text
                                MouseArea {
                                    anchors.fill: parent
                                    acceptedButtons: Qt.NoButton
                                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
                                }
                            }
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
                                // 找到对应的 TextEdit
                                let textEdit = messageText.children[0]
                                textEdit.selectAll()
                                textEdit.copy()
                                textEdit.deselect()
                                parent.parent.parent.statusTextOpacity = 1
                                statusTimer.restart()  // 使用当前消息的计时器
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

    // 添加一个计时器来处理滚动
    Timer {
        id: scrollTimer
        interval: 100  // 100ms 延迟
        onTriggered: {
            messageList.positionViewAtEnd()
            // 再次检查并滚动，以处理可能的内容动态调整
            Qt.callLater(function() {
                messageList.positionViewAtEnd()
            })
        }
    }
} 