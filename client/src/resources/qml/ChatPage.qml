import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    id: chatPage
    color: "white"
    
    // 属性：用于接收初始查询消息
    property string initialQuery: ""
    
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
            // 添加搜索结果到消息列表
            messageList.model.append({
                "message": results,
                "isUser": false
            })
            
            // 滚动到底部
            messageList.positionViewAtEnd()
        }
        
        function onSearchingChanged() {
            chatSearchBar.loading = searchBridge.searching
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

            delegate: Row {
                id: messageRow
                width: parent.width
                spacing: 8
                layoutDirection: isUser ? Qt.RightToLeft : Qt.LeftToRight

                Rectangle {
                    id: bubble
                    width: Math.min(messageContent.implicitWidth + 24, messageRow.width * 0.8)
                    height: messageContent.implicitHeight + 16
                    color: isUser ? "#e3f2fd" : "#f5f5f5"
                    radius: 8

                    Text {
                        id: messageContent
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
                        textFormat: Text.StyledText
                        onLinkActivated: Qt.openUrlExternally(link)
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
} 