import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import QtQuick.Window 2.1

import arstudio 1.0

/**
  This component is responsible for displaying a list of sequences
  available for display in an ItemWindow and for adding user-selected
  sequences to ItemWindow's Scenegraph.
*/

RowLayout {
    id: root
    property int exclusiveSequenceIndex

    property alias title: item_window.title
    property alias scenegraph: itemView.scenegraph
    property alias window: item_window

    Window {
        id: item_window
        visible: true
        width: 400;
        height: 400;
        Component.onCompleted:
        {
            if((Screen.width - g_rootWindow.x - g_rootWindow.width) < width)
                x = g_rootWindow.x - width
            else
                x = g_rootWindow.x + g_rootWindow.width
            y = g_rootWindow.y;
        }

        onVisibleChanged: {
            if (visible) return;
            for (var i = 0; i < windowList.count; i++)
            {
                if (windowList.getTab(i).item.window === window)
                    windowList.removeTab(i)
            }
        }

        ItemView {
            id: itemView
            focus: true
            anchors.fill: parent
            show_camera_path: cameraPathCheckbox.checked
            show_item_positions: itemPositionsCheckbox.checked
            first_person_mode: firstPersonModeCheckBox.checked
        }

        ColumnLayout {
            id: clayout
            CheckBox {
                id: cameraPathCheckbox
                text: "Show Camera Path"
                checked: true
                visible: itemView.scenegraph.locked_to === Scenegraph.NORMAL
            }

            CheckBox {
                id: itemPositionsCheckbox
                text: "Show Item Positions"
                checked: true
                visible: itemView.scenegraph.locked_to === Scenegraph.NORMAL
            }
        }
            CheckBox {
                id: firstPersonModeCheckBox
                anchors.left: clayout.right
                text: "First Person Mode"
                visible: itemView.scenegraph.locked_to === Scenegraph.NORMAL
            }
    }

    TableView {
        id: tableView
        Layout.fillHeight: true
        Layout.fillWidth: true
        model: g_Repository

        TableViewColumn {
            role: "name"
            title: "Repository"
            width: tableView.width - 20
            delegate: CheckBox {
                anchors.fill: parent
                text: styleData.value
                enabled: {
                    if (!scenegraph || g_Repository.nodes.length === 0)
                        return false

                    var type = g_Repository.nodes[styleData.row].type
                    if (scenegraph.locked_to === Scenegraph.BITMAP)
                        return (type === Sequence.Bitmap)
                                && (styleData.row === exclusiveSequenceIndex)
                    else if (scenegraph.locked_to === Scenegraph.NORMAL)
                        return type !== Sequence.Bitmap

                    return true
                }

                onCheckedChanged: {
                    var sequence = g_Repository.nodes[styleData.row]
                    if (checked)
                        scenegraph.add_sequence(sequence)
                    else
                        scenegraph.remove_sequence(sequence)

                    if (scenegraph.locked_to === Scenegraph.BITMAP)
                        exclusiveSequenceIndex = styleData.row
                }
            }
        }

        TableViewColumn {
            role: "type"
            width: 16
            delegate: Image {
                fillMode: Image.Pad
                source: switch (styleData.value) {
                        case Sequence.Camera: return "qrc:camera-photo.png"
                        case Sequence.Bitmap: return "qrc:emblem-photos.png"
                        }
            }
        }
    }
}

