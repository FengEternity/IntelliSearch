import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import "qrc:/qml"

// 主应用窗口
ApplicationWindow {
    id: window
    width: 1024
    height: 768
    visible: true
    minimumWidth: 800    // 最小宽度确保UI布局不会过于拥挤
    minimumHeight: 600   // 最小高度确保所有内容可见

    // 设置Material主题颜色
    Material.accent: Material.Blue
    Material.primary: Material.Blue

    // 主布局：左侧历史面板 + 右侧内容区
    RowLayout {
        anchors.fill: parent
        spacing: 0

        // 左侧历史搜索记录面板
        Rectangle {
            id: historyPanel
            // 动态计算面板宽度：展开时25%（200-400px），收起时5%（45-60px）
            Layout.preferredWidth: expanded ? Math.min(Math.max(window.width * 0.25, 200), 400) : Math.min(Math.max(window.width * 0.05, 45), 60)
            Layout.fillHeight: true
            color: "#f5f5f5"
            property bool expanded: false

            // 收起状态的按钮
            RoundButton {
                anchors {
                    left: parent.left
                    top: parent.top
                    margins: 5
                }
                width: 40
                height: 40
                flat: true
                icon.source: "qrc:/icons/toggle-right.svg"
                icon.color: "#303030"
                icon.width: 24
                icon.height: 24
                visible: !historyPanel.expanded
                onClicked: historyPanel.expanded = !historyPanel.expanded
            }
            
            // 添加设置按钮
            RoundButton {
                id: settingsButton
                anchors {
                    right: parent.right
                    bottom: parent.bottom
                    margins: 8
                }
                width: 40
                height: 40
                flat: true
                icon.source: "qrc:/icons/settings.svg"
                icon.color: "#303030"
                icon.width: 24
                icon.height: 24
                visible: !historyPanel.expanded
                
                // ToolTip {
                //     visible: parent.hovered
                //     text: "设置"
                //     delay: 500
                // }

                onClicked: {
                    contentArea.state = contentArea.state === "settings" ? "" : "settings"
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: historyPanel.expanded ? 10 : 0
                spacing: 20
                visible: historyPanel.expanded  // 展开时才显示内容

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 15

                    Text {
                        text: "  历史搜索"
                        color: Material.foreground
                        font.pixelSize: 20
                        font.weight: Font.Medium
                    }

                    Item { Layout.fillWidth: true }

                    RoundButton {
                        width: 32
                        height: 32
                        flat: true
                        icon.source: "qrc:/icons/toggle-left.svg"
                        icon.color: "#303030"
                        icon.width: 20
                        icon.height: 20
                        onClicked: historyPanel.expanded = !historyPanel.expanded
                    }
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: historyPanel.expanded
                    spacing: 2
                    clip: true
                    model: searchBridge.searchHistory
                    delegate: ItemDelegate {
                        width: parent.width
                        height: 60
                        
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 0

                            Text {
                                Layout.fillWidth: true
                                text: modelData.query ?? "未知查询"
                                color: Material.foreground
                                font.pixelSize: 14
                                elide: Text.ElideRight
                                maximumLineCount: 0
                                wrapMode: Text.WordWrap
                            }
                        }

                        background: Rectangle {
                            color: parent.hovered ? Qt.rgba(0, 0, 0, 0.05) : "transparent"
                            Behavior on color { ColorAnimation { duration: 150 } }
                        }

                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.RightButton

                            onClicked: function(event) {
                                if(event.button == Qt.RightButton) {
                                    contextMenu.recordId = modelData.id;
                                    contextMenu.popup();
                                }
                            }
                        }
                    }

                    Menu {
                        id: contextMenu
                        property string recordId: ""
                        Material.elevation: 8
                        Material.background: Material.dialogColor
                        closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnEscape
                        width: historyPanel.Layout.preferredWidth / 2
                        Timer {
                            id: hideTimer
                            interval: 1000
                            onTriggered: contextMenu.close()
                        }
                        onClosed: hideTimer.stop()
                        onOpened: hideTimer.stop()
                        MenuItem {
                            text: "显示详细信息"
                            icon.source: "qrc:/icons/info.svg"
                            icon.color: Material.foreground
                            icon.width: 16
                            icon.height: 16
                            height: 40
                            padding: 4
                            font.pixelSize: 13  // 设置文字大小
                            // onTriggered: {
                            //     searchBridge.deleteSearchHistory(contextMenu.recordId);
                            // }
                            onHoveredChanged: {
                                if (hovered) {
                                    hideTimer.stop()
                                } else {
                                    hideTimer.start()
                                }
                            }
                        }
                        MenuItem {
                            text: "删除搜索记录"
                            icon.source: "qrc:/icons/delete.svg"
                            icon.color: Material.foreground
                            icon.width: 16
                            icon.height: 16
                            height: 40
                            padding: 4
                            font.pixelSize: 13  // 设置文字大小
                            onTriggered: {
                                searchBridge.deleteSearchHistory(contextMenu.recordId);
                            }
                            onHoveredChanged: {
                                if (hovered) {
                                    hideTimer.stop()
                                } else {
                                    hideTimer.start()
                                }
                            }
                        }
                    }
                }
            }
        }

        // 右侧主内容区
        Rectangle {
            id: contentArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"

            states: [
                State {
                    name: "settings"
                    PropertyChanges { target: welcomeContent; visible: false }
                    PropertyChanges { target: settingsPage; visible: true }
                }
            ]

            // 欢迎内容
            ColumnLayout {
                id: welcomeContent
                anchors.fill: parent
                spacing: 20

                Item { Layout.fillHeight: true }  // 顶部弹性空间

                // 欢迎界面：当没有搜索结果时显示
                ColumnLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 20
                    visible: !mainSearchResults.model || mainSearchResults.model.count === 0

                    Image {
                        Layout.alignment: Qt.AlignHCenter
                        source: "qrc:/icons/logo.svg"
                        width: 80
                        height: 80
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: "我是 IntelliSearch，很高兴见到你！"
                        font.pixelSize: 24
                        color: "#1a1a1a"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: "我可以帮你搜索各种内容，请把你的任务交给我吧～"
                        font.pixelSize: 14
                        color: "#666666"
                    }

                    Item { height: 40 }  // 搜索框上方的间距
                }

                // 搜索功能区域
                ColumnLayout {
                    Layout.fillWidth: true
                    // 根据是否有搜索结果动态调整边距
                    Layout.margins: mainSearchResults.model && mainSearchResults.model.count > 0 ? 20 : 0
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 20

                    // 搜索栏组件
                    SearchBar {
                        id: mainSearchBar
                        Layout.fillWidth: true
                        // 有搜索结果时占满宽度，否则最大600px
                        Layout.maximumWidth: mainSearchResults.model && mainSearchResults.model.count > 0 ? parent.width : 600
                        Layout.alignment: Qt.AlignHCenter
                        // 搜索信号处理
                        onSearch: function(query) {
                            searchBridge.handleSearch(query);
                        }
                    }

                    // 搜索结果显示组件
                    SearchResults {
                        id: mainSearchResults
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        visible: model && model.count > 0
                    }
                }

                Item { Layout.fillHeight: true }  // 底部弹性空间
            }

            // 设置页面
            SettingsPage {
                id: settingsPage
                anchors.fill: parent
                visible: false
            }
        }
    }
}