import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Rectangle {
    id: sideBar

    property bool expanded: true
    property int expandedWidth: 200
    property int collapsedWidth: 50

    property StackView stackView
    property var searchBridge

    color: "#f5f5f5"

    width: expanded ? expandedWidth : collapsedWidth

    Behavior on Layout.preferredWidth {
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    Behavior on width {
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 展开/收起按钮和搜索框
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                spacing: 8

                SearchTextField {
                    visible: sideBar.expanded
                    onSearch: function (searchText) {
                        console.log("搜索:", searchText);
                        // 这里可以添加搜索处理逻辑
                    }
                }

                Button {
                    Layout.rightMargin: sideBar.expanded ? 8 : 0
                    Layout.alignment: Qt.AlignRight
                    flat: true
                    onClicked: sideBar.expanded = !sideBar.expanded
                    icon.source: sideBar.expanded ? "qrc:/resources/icons/actions/toggle-left.svg" : "qrc:/resources/icons/actions/toggle-right.svg"
                    icon.color: "#707070"

                    background: HoverBackground {
                        isHovered: parent.hovered
                    }
                    icon.width: 20
                    icon.height: 20
                    display: AbstractButton.IconOnly
                }
            }
        }

        // 历史记录组件
        HistoryRecord {
            id: historyRecordComponent
            Layout.fillWidth: true
            Layout.preferredHeight: sideBar.expanded ? 300 : 0
            visible: sideBar.expanded
            searchBridge: sideBar.searchBridge
            stackView: sideBar.stackView
            
            // 添加高度过渡动画
            Behavior on Layout.preferredHeight {
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }
        }
        
        // 分隔线
        // Rectangle {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 1
        //     Layout.leftMargin: 8
        //     Layout.rightMargin: 8
        //     color: applicationWindow.isDarkTheme ? "#444444" : "#eeeeee"
        //     visible: sideBar.expanded
            
        //     // 添加颜色过渡动画
        //     Behavior on color {
        //         ColorAnimation { duration: 200 }
        //     }
        // }

        // 弹性空间，将设置按钮推到底部
        Item {
            Layout.fillHeight: true
        }

        Button {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            Layout.margins: 8
            flat: true

            icon.source: applicationWindow.isDarkTheme ? "qrc:/resources/icons/status/dark.svg" : "qrc:/resources/icons/status/light.svg"
            icon.color: "#333333"
            text: sideBar.expanded ? qsTr("主题") : ""

            icon.width: 20
            icon.height: 20
            display: sideBar.expanded ? AbstractButton.TextBesideIcon : AbstractButton.IconOnly

            contentItem: Row {
                spacing: 10
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter

                Image {
                    source: applicationWindow.isDarkTheme ? "qrc:/resources/icons/status/dark.svg" : "qrc:/resources/icons/status/light.svg"
                    width: 20
                    height: 20
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: qsTr("主题")
                    visible: sideBar.expanded
                    anchors.verticalCenter: parent.verticalCenter
                    color: applicationWindow.isDarkTheme ? "#FFFFFF" : "#000000"
                }
            }

            background: HoverBackground {
                isHovered: parent.hovered
            }

            onClicked: {
                applicationWindow.isDarkTheme = !applicationWindow.isDarkTheme;
            }
        }

        // 设置按钮
        Button {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            Layout.margins: 8
            flat: true

            // background: null

            icon.source: "qrc:/resources/icons/actions/setting.svg"
            icon.color: "#333333"
            text: sideBar.expanded ? qsTr("设置") : ""

            icon.width: 20
            icon.height: 20
            display: sideBar.expanded ? AbstractButton.TextBesideIcon : AbstractButton.IconOnly

            contentItem: Row {
                spacing: 10
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter

                Image {
                    source: "qrc:/resources/icons/actions/setting.svg"
                    width: 20
                    height: 20
                    anchors.verticalCenter: parent.verticalCenter
                    // color: "#333333"
                }

                Text {
                    text: qsTr("设置")
                    visible: sideBar.expanded
                    anchors.verticalCenter: parent.verticalCenter
                    color: applicationWindow.isDarkTheme ? "#FFFFFF" : "#000000"
                }
            }

            background: HoverBackground {
                isHovered: parent.hovered
            }

            onClicked: {
                console.log("打开设置");
                if (stackView.depth > 0 && stackView.currentItem.objectName === "settingPage") {
                    console.log("已经在设置页面，不再重复打开");
                    return;
                }
                stackView.push("qrc:/pages/SettingPage.qml", {
                    stackView: stackView
                });
            }
        }
    }
}
