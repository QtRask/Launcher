import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Material.impl 2.12
import QtGraphicalEffects 1.0
import QtRask.Launcher 1.0

Item {
    id: root

    property string applicationName
    property string packageName
    property string icon
    property int iconSize: 60
    property bool adaptativeIcon

    property bool showAppName: true
    property alias click: areaClick
    property double scaleForClick: 1.5

    property bool textNegative: false

    width: 60
    height: width + (root.showAppName ? 36 : 0)

    Column {
        anchors.fill: parent

        Rectangle {
            id: iconGlass

            width: root.iconSize
            height: width
            z: 2

            anchors.horizontalCenter: parent.horizontalCenter

            color: RaskTheme.iconBackground
            border.color: RaskTheme.iconBorderColor
            border.width: 1
            radius: raskSettings.iconRadius

            scale: areaClick.pressed ? root.scaleForClick : 1
            Behavior on scale {
                NumberAnimation {
                    duration: 200
                }
            }

            Rectangle {
                id: iconBackground

                width: parent.width
                height: width
                z: 3

                color: "transparent"
                anchors.centerIn: parent

                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Rectangle {
                        x: iconBackground.x
                        y: iconBackground.y
                        width: iconBackground.width
                        height: iconBackground.height
                        radius: iconGlass.radius
                    }
                }

                Image {
                    width: parent.width * (root.adaptativeIcon ? 1.5 : 1)
                    height: width

                    asynchronous: true
                    anchors.centerIn: parent

                    sourceSize.width: width
                    sourceSize.height: height
                    source: root.icon
                }
            }

            Rectangle {
                id: iconPressedBackground
                z: 4

                anchors.fill: parent
                radius: parent.radius
                color: "transparent"

                Ripple {

                    anchors.fill: parent
                    clipRadius: iconPressedBackground.radius
                    anchor: iconPressedBackground
                    active: areaClick.pressed
                    pressed: areaClick.pressed

                    color: RaskTheme.iconPressed
                }

                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Rectangle {
                        x: iconPressedBackground.x
                        y: iconPressedBackground.y
                        width: iconPressedBackground.width
                        height: iconPressedBackground.height
                        radius: iconPressedBackground.radius
                    }
                }
            }

            MouseArea {
                id: areaClick
                anchors.fill: parent
            }
        }

        Label {
            visible: root.showAppName

            width: parent.width
            height: 36
            z: 1

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 5
            anchors.rightMargin: 5

            text: root.applicationName
            font.pixelSize: 11
            color: RaskTheme.getColor(
                       root.textNegative ? RaskTheme.Black : RaskTheme.White)

            elide: Label.ElideRight
            wrapMode: Label.WordWrap
            horizontalAlignment: Label.AlignHCenter
            verticalAlignment: Label.AlignVCenter

            layer.enabled: !root.textNegative
            layer.effect: DropShadow {
                verticalOffset: 1
                horizontalOffset: 1
                color: RaskTheme.getColor(RaskTheme.Black, RaskTheme.Alpha75)
                radius: 1
                samples: 3
            }
        }
    }
}
