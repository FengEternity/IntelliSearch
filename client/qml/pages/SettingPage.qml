import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../components"

Rectangle {
    id: root
    anchors.fill: parent
    anchors.centerIn: parent
    color: applicationWindow.isDarkTheme ? "#1E1E1E" : "#ffffff"
    radius: 10
    
    // 添加颜色过渡动画
    Behavior on color {
        ColorAnimation { duration: 200 }
    }

    property StackView stackView

    RowLayout {
        anchors.fill: parent
        spacing: 0

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
            Layout.preferredWidth: 200
            Layout.fillHeight: true
            color: applicationWindow.isDarkTheme ? "#1E1E1E" : "#ffffff"
            
            // 添加颜色过渡动画
            Behavior on color {
                ColorAnimation { duration: 200 }
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                RowLayout {
                    anchors.fill: parent
                    spacing: 0

                    SearchTextField {
                        onSearch: function (searchText) {
                            console.log("搜索:", searchText);
                            // 这里可以添加搜索处理逻辑
                        }
                    }

                    Button {
                        Layout.fillWidth: true
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

        Rectangle {
            Layout.fillHeight: true  // 改为填充高度
            Layout.preferredWidth: 2  // 设置固定宽度
            color: applicationWindow.isDarkTheme ? "#444444" : "#eeeeee"
            opacity: 0.5
            
            // 添加颜色过渡动画
            Behavior on color {
                ColorAnimation { duration: 200 }
            }
        }

        // 右侧内容区域
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: applicationWindow.isDarkTheme ? "#1E1E1E" : "#ffffff"
            
            // 添加颜色过渡动画
            Behavior on color {
                ColorAnimation { duration: 200 }
            }

            StackLayout {
                id: rightContent
                anchors.fill: parent
                anchors.margins: 24  // 添加边距

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

        Rectangle {
            Layout.fillHeight: true  // 改为填充高度
            Layout.preferredWidth: 2  // 设置固定宽度
            color: applicationWindow.isDarkTheme ? "#444444" : "#eeeeee"
            opacity: 0.5
            
            // 添加颜色过渡动画
            Behavior on color {
                ColorAnimation { duration: 200 }
            }
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: applicationWindow.isDarkTheme ? "#1E1E1E" : "#ffffff"
            
            // 添加颜色过渡动画
            Behavior on color {
                ColorAnimation { duration: 200 }
            }
        }
    }
}
