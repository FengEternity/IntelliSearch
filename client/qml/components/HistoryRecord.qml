import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

Rectangle {
    id: historyRecord
    color: "transparent"

    // 属性定义
    property var searchBridge
    property StackView stackView
    property string currentSessionId: ""

    // 信号定义
    signal sessionSelected(string sessionId)

    // 会话列表模型
    ListModel {
        id: sessionModel
    }

    // 组件初始化完成后加载会话历史
    Component.onCompleted: {
        // 只有当searchBridge存在时才加载会话历史
        if (historyRecord.searchBridge) {
            loadSessionHistory();
        } else {
            console.log("searchBridge is not available")
        }
    }

    // 监听会话历史变化
    Connections {
        target: historyRecord.searchBridge
        enabled: historyRecord.searchBridge !== null && historyRecord.searchBridge !== undefined

        // 使用正确的信号连接语法
        onSessionHistoryChanged: {
            loadSessionHistory();
        }

        onSessionCreated: function (sessionId) {
            // 当创建新会话时，更新当前会话ID
            currentSessionId = sessionId;
            loadSessionHistory();
        }

        onSessionUpdated: function (sessionId) {
            // 当会话更新时，刷新会话历史
            loadSessionHistory();
        }
    }

    // 加载会话历史数据
    function loadSessionHistory() {
        // 检查searchBridge是否存在
        if (!searchBridge) {
            console.warn("searchBridge is not available yet");
            return;
        }

        // 清空现有模型
        sessionModel.clear();
        try {
            // 获取会话历史数据
            var sessions = searchBridge.getSessionsList(100);

            // 将数据添加到模型中
            for (var i = 0; i < sessions.length; i++) {
                sessionModel.append({
                    sessionId: sessions[i].id,
                    title: sessions[i].title || "未命名会话",
                    lastUpdated: sessions[i].last_updated || "",
                    messageCount: sessions[i].message_count || 0,
                    lastQuery: sessions[i].last_query || ""
                });
            }
        } catch (e) {
            console.error("Error loading session history:", e);
        }
    }

    // 格式化日期时间
    function formatDateTime(dateTimeStr) {
        if (!dateTimeStr)
            return "";
        var date = new Date(dateTimeStr);
        var now = new Date();
        var yesterday = new Date(now);
        yesterday.setDate(yesterday.getDate() - 1);

        // 如果是今天
        if (date.toDateString() === now.toDateString()) {
            return "今天 " + date.getHours().toString().padStart(2, '0') + ":" + date.getMinutes().toString().padStart(2, '0');
        } else
        // 如果是昨天
        if (date.toDateString() === yesterday.toDateString()) {
            return "昨天 " + date.getHours().toString().padStart(2, '0') + ":" + date.getMinutes().toString().padStart(2, '0');
        } else
        // 其他日期
        {
            return (date.getMonth() + 1) + "月" + date.getDate() + "日";
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0


        // 分隔线
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: applicationWindow.isDarkTheme ? "#444444" : "#eeeeee"

            // 添加颜色过渡动画
            Behavior on color {
                ColorAnimation { duration: 200 }
            }
        }

        // 会话列表
        ListView {
            id: sessionListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: sessionModel

            // 当没有会话时显示的提示
            Rectangle {
                anchors.fill: parent
                color: "transparent"
                visible: sessionModel.count === 0

                Column {
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "暂无历史记录"
                        color: applicationWindow.isDarkTheme ? "#999999" : "#666666"
                    }
                }
            }

            delegate: ItemDelegate {
                width: parent.width
                height: 40

                // 高亮当前选中的会话
                Rectangle {
                    anchors.fill: parent
                    color: model.sessionId === currentSessionId ? (applicationWindow.isDarkTheme ? "#383838" : "#e8e8e8") : "transparent"
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 8
                    spacing: 4

                    // 会话信息
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Text {
                            text: model.title
                            font.pixelSize: 14
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                            color: applicationWindow.isDarkTheme ? "#FFFFFF" : "#333333"
                        }
                    }

                    // 时间和消息数
                    Column {
                        Layout.alignment: Qt.AlignRight
                        spacing: 4

                        Text {
                            text: formatDateTime(model.lastUpdated)
                            font.pixelSize: 10
                            color: applicationWindow.isDarkTheme ? "#AAAAAA" : "#666666"
                        }
                    }
                }

                background: HoverBackground {
                        isHovered: parent.hovered
                    }

                onClicked: {
                    // 设置当前会话ID
                    currentSessionId = model.sessionId;
                    // 切换到该会话
                    searchBridge.setCurrentSession(model.sessionId);
                    // 发出会话选择信号
                    sessionSelected(model.sessionId);

                    // 打开聊天页面
                    if (stackView.depth > 0 && stackView.currentItem.objectName === "chatPage") {
                        console.log("已经在聊天页面，不再重复打开");
                        return;
                    }
                    stackView.push("qrc:/pages/ChatPage.qml", {
                        stackView: stackView
                    });
                }
            }
        }
    }
}
