import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Window 2.1

import Qt3D 2.0
import Qt3D.Shapes 2.0

import arstudio 1.0

/**
  This component implements a window for displaying items from Repository.
*/

Window {
    id: window

    property url skyboxSource
    readonly property IWManager manager: iwmanager
    property bool cameraView: false
    property Item3D selectedModel: manager.selected_model
    property Skybox skybox: null

    title: "Item Window"
    flags: Qt.Tool | Qt.SubWindow
    Component.onCompleted: show()

    Effect {
        id: selected_effect
        color: "#ffffff"
    }

    Action {
        shortcut: "Ctrl+M"
        onTriggered: iwmanager.select_next_model()
    }

    IWManager {
        id: iwmanager
        window: window
        viewport: viewport
        bitmap_view: bitmapView
        camera: camera
        camera_view: cameraView
    }

    onSkyboxSourceChanged: skybox.visible = true

    // have to contain the Viewport in a rectangle,
    // or navigation does not work
    Rectangle {
        anchors.fill: parent
        color: "black"

        BitmapView {
            id: bitmapView
            anchors.fill: parent
            visible: (manager.scenegraph.locked_to === Scenegraph.BITMAP)
        }

        Viewport {
            id: viewport
            visible: !bitmapView.visible
            anchors.fill: parent
            light: Light {
                ambientColor: "#ffffff"
                position: Qt.vector3d(0, 0, 0)
            }
            camera: Camera { id: camera }
            focus: true
            picking: true
            Keys.onPressed: manager.viewport_keypress (event.key)

            Axis { objectName: "vp_ignore" }

            Skybox {
                id: skybox
                source: skyboxSource
                visible: false
            }
        }
    }
}
