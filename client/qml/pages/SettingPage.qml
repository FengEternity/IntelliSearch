import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../components"

Rectangle {
    id: root
    anchors.fill: parent
    anchors.centerIn: parent
    color: "#ffffff"
    radius: 10

    property StackView stackView

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 2
            opacity: 0.5
        }

        // 左侧导航栏
        Rectangle {
            Layout.preferredWidth: 200
            Layout.fillHeight: true
            color: "#ffffff"

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

                                HoverBackground {
                                    anchors.centerIn: parent
                                    implicitWidth: parent.width + 16  // 使用 implicitWidth 替代 width
                                    implicitHeight: parent.height + 16 // 使用 implicitHeight 替代 height
                                    isHovered: parent.parent.parent.hovered
                                }
                            }
                        }

                        background: null

                        onClicked: stackView.pop()
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
                        }
                        ListElement {
                            name: "搜索"
                            icon: "qrc:/resources/icons/actions/search.svg"
                        }
                        // ListElement { name: "主题设置"; icon: "qrc:/resources/icons/settings/theme.svg" }
                        ListElement {
                            name: "关于"
                            icon: "qrc:/resources/icons/status/about.svg"
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
                            }

                            Label {
                                text: model.name
                                font.pixelSize: 14
                                color: "#333333"
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
                            rightContent.currentIndex = model.index
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 2
            opacity: 0.5
        }

        // 右侧内容区域
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#ffffff"

            StackLayout {
                id: rightContent
                anchors.fill: parent

                ModelListView {}  // 模型设置页面

                Rectangle {  // 搜索设置页面
                    color: "#ffffff"
                }

                Rectangle {  // 关于页面
                    color: "#ffffff"
                }
            }
        }
    }
}
