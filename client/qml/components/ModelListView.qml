import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "."

// 使用SplitView替换ListView作为顶层组件，以支持可拖动分割线
SplitView {
    id: modelSplitView
    orientation: Qt.Horizontal

    // 设置分割线的样式
    handle: Rectangle {
        implicitWidth: 4
        implicitHeight: 4
        color: applicationWindow.isDarkTheme ? "#444444" : "#eeeeee"
        opacity: 0.5

        // 添加颜色过渡动画
        Behavior on color {
            ColorAnimation {
                duration: 200
            }
        }

        // 添加鼠标悬停效果
        Rectangle {
            anchors.centerIn: parent
            width: 2
            height: 30
            radius: 1
            color: SplitHandle.hovered ? (applicationWindow.isDarkTheme ? "#888888" : "#999999") : "transparent"

            Behavior on color {
                ColorAnimation {
                    duration: 100
                }
            }
        }
    }

    // 左侧模型列表
    ListView {
        id: modelListView
        SplitView.preferredWidth: 200
        SplitView.minimumWidth: 150
        SplitView.fillHeight: true

        model: ListModel {
            ListElement {
                name: "Kimi"
                icon: "qrc:/resources/icons/model/kimi.svg"
            }

            ListElement {
                name: "腾讯混元"
                icon: "qrc:/resources/icons/model/hunyuan.svg"
            }

            ListElement {
                name: "DeepSeek"
                icon: "qrc:/resources/icons/model/deepseek.svg"
            }

            ListElement {
                name: "通义千问"
                icon: "qrc:/resources/icons/model/qwen.svg"
            }
        }

        delegate: ItemDelegate {
            width: parent.width
            height: 60
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                spacing: 12

                Image {
                    source: model.icon
                    Layout.preferredWidth: 20
                    Layout.preferredHeight: 20
                    fillMode: Image.PreserveAspectFit
                }

                Label {
                    text: model.name
                    font.pixelSize: 14
                }

                Item {
                    Layout.fillWidth: true
                }
            }

            background: HoverBackground {
                isHovered: parent.hovered
            }

            // 添加点击事件，处理模型选择逻辑
            onClicked: {
                // 更新右侧内容区域，显示选中模型的详细信息
                console.log("选择模型:", model.name);

                // 更新右侧详情视图
                selectedModelIcon.source = model.icon;
                selectedModelName.text = model.name;

                // 根据不同模型设置不同的API地址示例
                if (model.name === "Kimi") {
                    apiUrlField.text = "https://api.moonshot.cn";
                    apiEndpointLabel.text = "https://api.moonshot.cn/v1/chat/completions";
                    modelSelector.model = ["moonshot-v1-8k", "moonshot-v1-32k", "moonshot-v1-128k"];
                } else if (model.name === "腾讯混元") {
                    apiUrlField.text = "https://hunyuan.cloud.tencent.com";
                    apiEndpointLabel.text = "https://hunyuan.cloud.tencent.com/hyllm/v1/chat/completions";
                    modelSelector.model = ["hunyuan-lite", "hunyuan-standard", "hunyuan-pro"];
                } else if (model.name === "DeepSeek") {
                    apiUrlField.text = "https://api.deepseek.com";
                    apiEndpointLabel.text = "https://api.deepseek.com/v1/chat/completions";
                    modelSelector.model = ["deepseek-chat", "deepseek-coder"];
                } else if (model.name === "通义千问") {
                    apiUrlField.text = "https://dashscope.aliyuncs.com";
                    apiEndpointLabel.text = "https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation";
                    modelSelector.model = ["qwen-turbo", "qwen-plus", "qwen-max"];
                }

                // 显示详情内容
                modelDetailContent.visible = true;
            }
        }
    }

    // 右侧内容区域 - 第三级界面空间
    Rectangle {
        id: modelDetailView
        SplitView.fillWidth: true
        SplitView.fillHeight: true
        SplitView.minimumWidth: 300
        color: applicationWindow.isDarkTheme ? "#1E1E1E" : "#ffffff"

        // 添加颜色过渡动画
        Behavior on color {
            ColorAnimation {
                duration: 200
            }
        }

        // 默认内容 - 当未选择模型时显示
        ColumnLayout {
            id: defaultContent
            anchors.centerIn: parent
            spacing: 20
            visible: !modelDetailContent.visible

            // Image {
            //     source: "qrc:/resources/icons/logo.svg"
            //     Layout.preferredWidth: 128
            //     Layout.preferredHeight: 128
            //     Layout.alignment: Qt.AlignHCenter
            //     fillMode: Image.PreserveAspectFit
            //     opacity: 0.5
            // }

            Label {
                text: "请选择左侧模型查看详情"
                font.pixelSize: 16
                color: applicationWindow.isDarkTheme ? "#CCCCCC" : "#666666"
                Layout.alignment: Qt.AlignHCenter

                // 添加颜色过渡动画
                Behavior on color {
                    ColorAnimation {
                        duration: 200
                    }
                }
            }
        }

        // 模型详情内容 - 当选择模型后显示
        ScrollView {
            id: modelDetailContent
            anchors.fill: parent
            anchors.margins: 20
            clip: true
            visible: false

            ColumnLayout {
                width: modelDetailContent.width - 40
                spacing: 20

                // 标题区域
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Image {
                        id: selectedModelIcon
                        source: ""
                        Layout.preferredWidth: 28
                        Layout.preferredHeight: 28
                        fillMode: Image.PreserveAspectFit
                    }

                    Label {
                        id: selectedModelName
                        text: ""
                        font.pixelSize: 18
                        font.bold: true
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Button {
                        text: "更新"
                        Layout.alignment: Qt.AlignRight
                        Layout.topMargin: 20
                        icon.source: "qrc:/resources/icons/navigations/refresh.svg"
                        display: AbstractButton.IconOnly
                        background: HoverBackground {
                            isHovered: parent.hovered
                        }
                        onClicked: {
                            console.log("更新模型配置");
                            // 这里可以添加保存配置的逻辑
                        }
                    }

                    // Button {
                    //     text: "删除"
                    //     icon.source: "qrc:/resources/icons/status/delete.svg"
                    //     flat: true
                    //     onClicked: {
                    //         console.log("删除模型配置:", selectedModelName.text);
                    //         // 这里可以添加删除模型配置的逻辑
                    //         modelDetailContent.visible = false;
                    //     }
                    // }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: applicationWindow.isDarkTheme ? "#444444" : "#e0e0e0"
                }

                // API密钥区域
                Label {
                    text: "API 密钥"
                    font.pixelSize: 14
                    font.bold: true
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    TextField {
                        id: apiKeyField
                        Layout.fillWidth: true
                        Layout.preferredHeight: 36
                        placeholderText: "请输入API密钥"
                        echoMode: TextInput.Password
                        font.pixelSize: 13
                        leftPadding: 8
                        rightPadding: 8
                    }

                    Button {
                        text: "获取"
                        onClicked: {
                            console.log("获取API密钥");
                            // 这里可以添加获取API密钥的逻辑
                        }
                    }

                    Button {
                        text: "检查"
                        onClicked: {
                            console.log("检查API密钥");
                            // 这里可以添加检查API密钥的逻辑
                        }
                    }
                }

                // Label {
                //     text: "多个密钥使用逗号分隔"
                //     font.pixelSize: 12
                //     color: applicationWindow.isDarkTheme ? "#AAAAAA" : "#888888"
                //     Layout.alignment: Qt.AlignRight
                // }

                // API地址区域
                Label {
                    text: "API 地址"
                    font.pixelSize: 14
                    font.bold: true
                    Layout.topMargin: 10
                }

                TextField {
                    id: apiUrlField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    placeholderText: "https://api.example.com"
                    font.pixelSize: 13
                    leftPadding: 8
                    rightPadding: 8
                }

                Label {
                    id: apiEndpointLabel
                    text: "https://api.example.com/v1/chat/completions"
                    font.pixelSize: 12
                    color: applicationWindow.isDarkTheme ? "#AAAAAA" : "#888888"
                }

                // 模型选择区域
                Label {
                    text: "模型"
                    font.pixelSize: 14
                    font.bold: true
                    Layout.topMargin: 10
                }

                ComboBox {
                    id: modelSelector
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    font.pixelSize: 13
                    leftPadding: 8
                    rightPadding: 8
                    model: ["模型1", "模型2", "模型3"]
                    onCurrentTextChanged: {
                        console.log("选择子模型:", currentText);
                    }

                    // 自定义下拉菜单样式
                    popup: Popup {
                        y: parent.height
                        width: parent.width
                        implicitHeight: contentItem.implicitHeight
                        padding: 1

                        // 添加阴影效果
                        // background: HoverBackground {
                        //     isHovered: parent.hovered
                        // }

                        contentItem: ListView {
                            clip: true
                            implicitHeight: contentHeight
                            model: modelSelector.popup.visible ? modelSelector.delegateModel : null
                            currentIndex: modelSelector.highlightedIndex

                            ScrollIndicator.vertical: ScrollIndicator {}
                        }
                    }
                }

                // 保存按钮
                // Button {
                //     text: "保存配置"
                //     Layout.alignment: Qt.AlignRight
                //     Layout.topMargin: 20
                //     onClicked: {
                //         console.log("保存模型配置");
                //         // 这里可以添加保存配置的逻辑
                //     }
                // }
            }
        }
    }
}
