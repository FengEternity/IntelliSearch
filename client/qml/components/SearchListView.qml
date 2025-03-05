import QtQuick
import QtQuick.Layouts

ListView {
    id: modelListView

    anchors.fill: parent

    model: ListModel {
        ListElement {
            name: "Bocha"
            icon: ""
        }

        ListElement {
            name: "Exa"
            icon: ""
        }
    }
}