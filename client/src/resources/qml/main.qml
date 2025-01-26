import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 1024
    height: 768
    visible: true
    title: "IntelliSearch"

    Material.accent: Material.Blue
    Material.primary: Material.Blue

    flags: Qt.Window
    color: "white"

    // 顶部工具栏
    Rectangle {
        id: toolbar
        anchors.top: parent.top
        anchors.right: parent.right
        width: 50
        height: 50
        color: "transparent"

        RoundButton {
            anchors.centerIn: parent
            width: 32
            height: 32
            flat: true
            icon.source: historyPanel.visible ? "qrc:/icons/chevron-left.svg" : "qrc:/icons/chevron-right.svg"
            icon.color: Material.foreground
            onClicked: historyPanel.visible = !historyPanel.visible
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // 左侧历史搜索记录
        Rectangle {
            id: historyPanel
            Layout.preferredWidth: 260
            Layout.fillHeight: true
            color: "#f5f5f5"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 20

                // 历史记录标题栏
                Text {
                    Layout.fillWidth: true
                    text: "历史搜索"
                    color: Material.foreground
                    font.pixelSize: 20
                    font.weight: Font.Medium
                }

                // 历史搜索记录列表
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 8
                    clip: true
                    model: ListModel {
                        // 示例数据
                        ListElement { 
                            query: "智能搜索引擎的发展趋势"
                            timestamp: "2024-01-20 15:30"
                        }
                        ListElement { 
                            query: "AI在搜索中的应用"
                            timestamp: "2024-01-20 14:20"
                        }
                        ListElement { 
                            query: "搜索引擎优化技术"
                            timestamp: "2024-01-20 11:45"
                        }
                    }
                    delegate: ItemDelegate {
                        width: parent.width
                        height: 60
                        
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 4

                            Text {
                                Layout.fillWidth: true
                                text: model.query
                                color: Material.foreground
                                font.pixelSize: 14
                                elide: Text.ElideRight
                                maximumLineCount: 2
                                wrapMode: Text.WordWrap
                            }

                            Text {
                                Layout.fillWidth: true
                                text: model.timestamp
                                color: Material.foreground
                                font.pixelSize: 12
                                opacity: 0.7
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
                anchors.margins: 20
                spacing: 20

                SearchBar {
                    id: searchBar
                    onSearch: function(query) {
                        // 模拟搜索结果数据
                        searchResults.model.clear()
                        searchResults.model.append({
                            title: "IntelliSearch：基于AI的智能搜索引擎",
                            description: "一个现代化的搜索引擎，结合了AI大语言模型，提供智能对话式搜索体验。"
                        })
                        searchResults.model.append({
                            title: "搜索引擎的工作原理",
                            description: "深入探讨搜索引擎的核心技术，包括索引、排序算法和相关性计算等关键概念。"
                        })
                        searchResults.model.append({
                            title: "AI技术在搜索中的应用",
                            description: "探索人工智能如何改变传统搜索方式，提升搜索准确性和用户体验。"
                        })
                    }
                }

                SearchResults {
                    id: searchResults
                }
            }
        }
    }
}