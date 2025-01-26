import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 1024
    height: 768
    visible: true
    title: "IntelliSearch"

    Material.theme: Material.Light
    Material.accent: Material.Blue

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