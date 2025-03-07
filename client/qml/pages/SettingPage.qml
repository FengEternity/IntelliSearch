import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../components"

Rectangle {
    id: root
    // 添加objectName属性，用于在SideBar中识别当前页面
    objectName: "settingPage"
    // 移除anchors.fill: parent，避免与StackView冲突
    // anchors.fill: parent
    Layout.fillWidth: true
    Layout.fillHeight: true
    color: applicationWindow.isDarkTheme ? "#1E1E1E" : "#ffffff"
    radius: 10
    
    // 添加颜色过渡动画
    Behavior on color {
        ColorAnimation { duration: 200 }
    }

    property StackView stackView

    // 使用SplitView替换RowLayout，以支持可拖动分割线
    SplitView {
        // 保留anchors.fill: parent，因为这是顶层布局
        anchors.fill: parent
        orientation: Qt.Horizontal
        // 设置分割线的样式
        handle: Rectangle {
            implicitWidth: 4
            implicitHeight: 4
            color: applicationWindow.isDarkTheme ? "#444444" : "#eeeeee"
            opacity: 0.5
            
            // 添加颜色过渡动画
            Behavior on color {
                ColorAnimation { duration: 200 }
            }
            
            // 添加鼠标悬停效果
            Rectangle {
                anchors.centerIn: parent
                width: 2
                height: 30
                radius: 1
                color: SplitHandle.hovered ? (applicationWindow.isDarkTheme ? "#888888" : "#999999") : "transparent"
                
                Behavior on color {
                    ColorAnimation { duration: 100 }
                }
            }
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 2
            color: applicationWindow.isDarkTheme ? "#444444" : "#eeeeee"
            opacity: 0.5
            
            // 添加颜色过渡动画
            Behavior on color {
                ColorAnimation { duration: 200 }
            }
        }

        // 左侧导航栏
        Rectangle {
            SplitView.preferredWidth: 200
            SplitView.minimumWidth: 150
            SplitView.fillHeight: true
            color: applicationWindow.isDarkTheme ? "#1E1E1E" : "#ffffff"
            
            // 添加颜色过渡动画
            Behavior on color {
                ColorAnimation { duration: 200 }
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                RowLayout {
                    // 移除冲突的anchors属性
                    Layout.fillWidth: true
                    Layout.preferredHeight: 48
                    spacing: 0

                    SearchTextField {
                        Layout.fillWidth: true
                        onSearch: function (searchText) {
                            console.log("搜索:", searchText);
                            // 这里可以添加搜索处理逻辑
                        }
                    }

                    Button {
                        Layout.preferredWidth: 48
                        Layout.preferredHeight: 48
                        flat: true

                        contentItem: RowLayout {
                            spacing: 10
                            Layout.leftMargin: 15

                            Image {
                                source: "qrc:/resources/icons/navigations/back.svg"
                                Layout.preferredWidth: 20
                                Layout.preferredHeight: 20
                                fillMode: Image.PreserveAspectFit
                                // 添加颜色滤镜使图标在暗黑模式下更明显
                                opacity: applicationWindow.isDarkTheme ? 0.9 : 0.7

                                HoverBackground {
                                    anchors.centerIn: parent
                                    implicitWidth: parent.width + 16
                                    implicitHeight: parent.height + 16
                                    isHovered: parent.parent.parent.hovered
                                    // 降低背景不透明度，避免完全覆盖图标
                                    opacity: 0.7
                                    // 确保背景在图标下方
                                    z: -1
                                }
                            }
                        }

                        background: null

                        onClicked: stackView.pop()
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    color: applicationWindow.isDarkTheme ? "#444444" : "#eeeeee"
                    
                    // 添加颜色过渡动画
                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }

                // 左侧导航栏的内容
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: ListModel {
                        ListElement {
                            name: "模型"
                            icon: "qrc:/resources/icons/actions/model.svg"
                            iconColor: "#707070"  // 修改为普通属性而非嵌套属性
                        }
                        ListElement {
                            name: "搜索"
                            icon: "qrc:/resources/icons/actions/search.svg"
                            iconColor: "#707070"  // 添加相同的属性保持一致性
                        }
                        // ListElement { name: "主题设置"; icon: "qrc:/resources/icons/settings/theme.svg" }
                        ListElement {
                            name: "关于"
                            icon: "qrc:/resources/icons/status/about.svg"
                            iconColor: "#707070"  // 添加相同的属性保持一致性
                        }
                    }

                    delegate: ItemDelegate {
                        width: parent.width
                        height: 48

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 16
                            spacing: 12

                            Image {
                                source: model.icon
                                Layout.preferredWidth: 24
                                Layout.preferredHeight: 24
                                fillMode: Image.PreserveAspectFit
                                // 使用iconColor属性
                                opacity: applicationWindow.isDarkTheme ? 0.9 : 0.7
                            }

                            Label {
                                text: model.name
                                font.pixelSize: 14
                                color: applicationWindow.isDarkTheme ? "#FFFFFF" : "#333333"

                                // 添加颜色过渡动画
                                Behavior on color {
                                    ColorAnimation { duration: 200 }
                                }
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                        }

                        background: HoverBackground {
                            isHovered: parent.hovered
                        }

                        onClicked: {
                            // 更新右侧内容区域
                            rightContent.currentIndex = model.index;
                        }
                    }
                }
            }
        }

        // 右侧内容区域
        Rectangle {
            SplitView.fillWidth: true
            SplitView.fillHeight: true
            SplitView.minimumWidth: 300
            color: applicationWindow.isDarkTheme ? "#1E1E1E" : "#ffffff"
            
            // 添加颜色过渡动画
            Behavior on color {
                ColorAnimation { duration: 200 }
            }

            StackLayout {
                id: rightContent
                // 移除冲突的anchors属性
                width: parent.width
                height: parent.height
                // 使用Layout属性设置边距
                Layout.margins: 24  // 添加边距

                ModelListView {
                    Layout.alignment: Qt.AlignTop  // 内容靠上对齐
                    Layout.fillWidth: true         // 宽度填充
                    Layout.preferredHeight: contentHeight  // 高度自适应内容
                }

                SearchListView {
                    Layout.alignment: Qt.AlignTop
                    Layout.fillWidth: true
                    Layout.preferredHeight: contentHeight
                }

                Rectangle {
                    // 关于页面
                    color: applicationWindow.isDarkTheme ? "#1E1E1E" : "#ffffff"
                    
                    // 添加颜色过渡动画
                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }
            }
        }

        // 使用第二个SplitView来处理第二个可拖动分割线
        SplitView {
            SplitView.fillWidth: true
            SplitView.fillHeight: true
            orientation: Qt.Horizontal
            
            // 设置分割线的样式
            handle: Rectangle {
                implicitWidth: 4
                implicitHeight: 4
                color: applicationWindow.isDarkTheme ? "#444444" : "#eeeeee"
                opacity: 0.5
                
                // 添加颜色过渡动画
                Behavior on color {
                    ColorAnimation { duration: 200 }
                }
                
                // 添加鼠标悬停效果
                Rectangle {
                    anchors.centerIn: parent
                    width: 2
                    height: 30
                    radius: 1
                    color: SplitHandle.hovered ? (applicationWindow.isDarkTheme ? "#888888" : "#999999") : "transparent"
                    
                    Behavior on color {
                        ColorAnimation { duration: 100 }
                    }
                }
            }
            
            // 空白区域
            Rectangle {
                SplitView.fillHeight: true
                SplitView.fillWidth: true
                color: applicationWindow.isDarkTheme ? "#1E1E1E" : "#ffffff"
                
                // 添加颜色过渡动画
                Behavior on color {
                    ColorAnimation { duration: 200 }
                }
            }
        }
    }
}
