import QtQuick
import QtQuick.Controls

Rectangle {
    id: chatTextField
    width: parent.width
    height: 40
    
    color: "#ffffff"
    border.color: textArea.focus ? "#007AFF" : "#E5E5EA"
    border.width: 1
    radius: 8

    property alias text: textArea.text
    property alias placeholder: placeholderText.text
    signal textSubmitted(string text)

    Button {
        id: sendButton
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            rightMargin: 8
        }
        width: 36
        height: 36
        icon.color: "#d1000000"
        font.kerning: false
        flat: false
        
        background: Image {
            source: "qrc:/resources/icons/actions/send.svg"
            opacity: 0.5
            sourceSize.width: 24
            sourceSize.height: 24
            fillMode: Image.PreserveAspectFit
        }

        onClicked: {
            if (textArea.text.trim() !== "") {
                chatTextField.textSubmitted(textArea.text.trim())
                // chatTextField.onStartChat()  // 删除这行，因为这个信号没有定义
                textArea.text = ""
            }
        }
    }

    TextArea {
        id: textArea
        anchors {
            left: parent.left
            right: sendButton.left
            top: parent.top
            bottom: parent.bottom
            margins: 8
        }
        font.pixelSize: 14
        color: "#000000"
        wrapMode: TextArea.Wrap
        background: null
        verticalAlignment: TextArea.AlignVCenter
        padding: 0
        topPadding: (height - font.pixelSize) / 2
        bottomPadding: (height - font.pixelSize) / 2
    
        
        Text {
            id: placeholderText
            anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
            }
            text: "你想了解些什么..."
            color: "#C7C7CC"
            visible: !textArea.text && !textArea.activeFocus
            font.pixelSize: 14
        }

        Keys.onReturnPressed: function(event) {
            if (event.modifiers & Qt.ShiftModifier) {
                event.accepted = false
            } else {
                event.accepted = true
                if (text.trim() !== "") {
                    chatTextField.textSubmitted(text.trim())
                    text = ""
                }
            }
        }
    }
}

