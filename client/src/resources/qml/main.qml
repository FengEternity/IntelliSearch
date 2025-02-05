import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 1024
    height: 768
    visible: true
    minimumWidth: 800
    minimumHeight: 600
    //title: "IntelliSearch"

    Material.accent: Material.Blue
    Material.primary: Material.Blue

    // 删除整个 header: ToolBar 部分

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // 左侧历史搜索记录
        Rectangle {
            id: historyPanel
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
                    }
                }
            }
        }

        // 右侧内容区
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"

            ColumnLayout {
                anchors.fill: parent
                spacing: 20

                Item { Layout.fillHeight: true }  // 顶部弹性空间

                // 欢迎内容
                ColumnLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 20
                    visible: !searchResults.model || searchResults.model.count === 0

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

                // 搜索区域
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.margins: searchResults.model && searchResults.model.count > 0 ? 20 : 0
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 20

                    SearchBar {
                        id: searchBar
                        Layout.fillWidth: true
                        Layout.maximumWidth: searchResults.model && searchResults.model.count > 0 ? parent.width : 600
                        Layout.alignment: Qt.AlignHCenter
                        onSearch: function(query) {
                            searchBridge.handleSearch(query);
                        }
                    }

                    SearchResults {
                        id: searchResults
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        visible: model && model.count > 0
                    }
                }

                Item { Layout.fillHeight: true }  // 底部弹性空间
            }
        }
    }
}