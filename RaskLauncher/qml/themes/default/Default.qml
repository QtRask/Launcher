import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0

import QtRask.Launcher 1.0

Page {
    id: page

    padding: 30
    leftPadding: 10
    rightPadding: 10

    property alias applications: appGrid.model
    property alias applicationsHidden: appHidden.model

    background: Item {}

    AppGrid {
        id: appGrid

        width: parent.width
        height: parent.height

        model: page.applications

        onClicked: function (packageName) {
            RaskLauncher.launchApplication(packageName)
            AndroidVibrate.vibrate(50, AndroidVibrate.EFFECT_TICK)
        }

        children: [
            Item {
                id: itemChildren

                Item {
                    id: itemSearch

                    y: appGrid.contentY >= 0 ? -height - 30 : -height - appGrid.contentY - 30

                    parent: itemChildren.parent
                    width: parent.width
                    height: buttonSearchPage.height * 1.2

                    Button {
                        id: buttonSearchPage

                        anchors.centerIn: parent
                        flat: true

                        icon.name: "search"
                        text: qsTr("Search Apps")
                    }
                }

                Item {
                    y: (Screen.height
                        > appGrid.contentHeight ? Screen.height : appGrid.contentHeight)
                       + height - appGrid.contentY

                    parent: itemChildren.parent
                    width: parent.width
                    height: buttonHiddenApps.height * 1.2

                    Button {
                        id: buttonHiddenApps

                        anchors.centerIn: parent
                        flat: true

                        icon.name: "visibility"
                        text: qsTr("Hidden Apps")
                    }
                }

                QtObject {
                    id: flickData

                    property int beforeHeight: -70
                    property int afterHeight: 150
                    property bool flickedStart: false
                    property bool flickedEnd: false
                }

                Timer {
                    id: flickAndHoldStart
                    interval: 1000

                    onTriggered: {
                        if (appGrid.contentY <= flickData.beforeHeight) {
                            appGrid.flickBeforeStart()
                            AndroidVibrate.vibrate(50,
                                                   AndroidVibrate.EFFECT_TICK)
                        }
                    }
                }

                Timer {
                    id: flickAndHoldEnd
                    interval: 1000

                    onTriggered: {
                        if (appGrid.contentY >= flickData.afterHeight) {
                            appGrid.flickAfterEnd()
                            AndroidVibrate.vibrate(50,
                                                   AndroidVibrate.EFFECT_TICK)
                        }
                    }
                }

                Connections {
                    target: appGrid

                    function onContentYChanged() {
                        if (!flickData.flickedStart && target.atYBeginning
                                && target.contentY < flickData.beforeHeight) {
                            flickData.flickedStart = true
                            flickAndHoldStart.running = true
                            return
                        }

                        if (!flickData.flickedEnd && target.atYEnd
                                && (target.contentY > flickData.afterHeight)) {
                            flickData.flickedEnd = true
                            flickAndHoldEnd.running = true
                        }
                    }

                    function onMovementEnded() {
                        flickData.flickedStart = false
                        flickData.flickedEnd = false
                        flickAndHoldStart.running = false
                        flickAndHoldEnd.running = false
                    }
                }
            }
        ]

        actions: AppActions {
            id: actions

            name: modelData ? modelData.name : ""
            options: ListModel {
                ListElement {
                    label: qsTr("Add to Dock")
                    iconName: "bookmark"

                    property var func: function () {}
                }

                ListElement {
                    label: qsTr("Hide App")
                    iconName: "visibility-off"

                    property var func: function () {
                        Applications.hideApplication(
                                    actions.modelData.packageName)
                    }
                }

                ListElement {
                    label: qsTr("Information")
                    iconName: "info"

                    property var func: function () {
                        RaskLauncher.openApplicationDetailsSettings(
                                    actions.modelData.packageName)
                    }
                }

                ListElement {
                    label: qsTr("Uninstall")
                    iconName: "delete"

                    property var func: function () {
                        RaskLauncher.uninstallApplication(
                                    actions.modelData.packageName)
                    }
                }
            }
        }

        onFlickBeforeStart: console.log("Show Search Page")

        onFlickAfterEnd: {
            if (appHidden.model.length > 0)
                appHidden.open()
        }
    }

    AppHidden {
        id: appHidden

        model: page.applications
    }

    //footer: AppDock {
    //    visible: model.length > 0
    //    width: parent.width
    //    height: 100
    //    anchors.bottom: parent.bottom
    //    shadderSource: appGrid
    //    //model: page.applications.splice(10, 10)
    //}

    //background: Rectangle {
    //    Image {
    //        anchors.fill: parent
    //        source: "file:///home/marssola/Pictures/P00613-120151.jpg"
    //        fillMode: Image.PreserveAspectCrop
    //    }
    //}
    property var applications: [{
            "adaptativeIcon": false,
            "name": "0ad",
            "packageName": "0ad"
        }, {
            "adaptativeIcon": false,
            "name": "Adobe Acrobat Reader",
            "packageName": "acroread"
        }, {
            "adaptativeIcon": false,
            "name": "Adobe After Effects",
            "packageName": "AdobeAfterEffect"
        }, {
            "adaptativeIcon": false,
            "name": "Adobe Photoshop",
            "packageName": "AdobePhotoshop"
        }, {
            "adaptativeIcon": false,
            "name": "Anatine",
            "packageName": "anatine"
        }, {
            "adaptativeIcon": false,
            "name": "Android Studio",
            "packageName": "android-studio"
        }, {
            "adaptativeIcon": true,
            "name": "Anjuta",
            "packageName": "anjuta"
        }, {
            "adaptativeIcon": false,
            "name": "Calligra",
            "packageName": "calligrastage"
        }, {
            "adaptativeIcon": false,
            "name": "Chromium",
            "packageName": "chromium-browser"
        }, {
            "adaptativeIcon": true,
            "name": "CPU AMD",
            "packageName": "cpu-amd"
        }, {
            "adaptativeIcon": true,
            "name": "CPU Info",
            "packageName": "cpuinfo"
        }, {
            "adaptativeIcon": false,
            "name": "Display",
            "packageName": "display"
        }, {
            "adaptativeIcon": false,
            "name": "Finder",
            "packageName": "file-manager"
        }, {
            "adaptativeIcon": false,
            "name": "Firefox",
            "packageName": "firefox"
        }, {
            "adaptativeIcon": false,
            "name": "Geany",
            "packageName": "geany"
        }, {
            "adaptativeIcon": false,
            "name": "Gnome Books",
            "packageName": "gnome-books"
        }, {
            "adaptativeIcon": false,
            "name": "Kodi",
            "packageName": "kodi"
        }, {
            "adaptativeIcon": false,
            "name": "Libre Office Calc",
            "packageName": "libreoffice-calc"
        }, {
            "adaptativeIcon": false,
            "name": "Libre Office Draw",
            "packageName": "libreoffice-draw"
        }, {
            "adaptativeIcon": false,
            "name": "Libre Office Writer",
            "packageName": "libreoffice-writer"
        }, {
            "adaptativeIcon": false,
            "name": "Luminance HDR",
            "packageName": "luminance-hdr"
        }, {
            "adaptativeIcon": false,
            "name": "Message",
            "packageName": "message"
        }, {
            "adaptativeIcon": false,
            "name": "Microsoft Office",
            "packageName": "ms-office"
        }, {
            "adaptativeIcon": false,
            "name": "Microsoft Office Excel",
            "packageName": "ms-excel"
        }, {
            "adaptativeIcon": false,
            "name": "Microsoft Office One Note",
            "packageName": "ms-onenote"
        }, {
            "adaptativeIcon": false,
            "name": "Microsoft Office Outlook",
            "packageName": "ms-outlook"
        }, {
            "adaptativeIcon": false,
            "name": "Microsoft Office PowerPoint",
            "packageName": "ms-powerpoint"
        }, {
            "adaptativeIcon": false,
            "name": "Microsoft Office Word",
            "packageName": "ms-word"
        }, {
            "adaptativeIcon": false,
            "name": "Minitube",
            "packageName": "minitube"
        }, {
            "adaptativeIcon": false,
            "name": "mongodb Compass",
            "packageName": "mongodb-compass"
        }, {
            "adaptativeIcon": false,
            "name": "Okular",
            "packageName": "okular"
        }, {
            "adaptativeIcon": true,
            "name": "Opera",
            "packageName": "opera"
        }, {
            "adaptativeIcon": false,
            "name": "PCSXR",
            "packageName": "pcsxr"
        }, {
            "adaptativeIcon": true,
            "name": "pgadmin3",
            "packageName": "pgadmin3"
        }, {
            "adaptativeIcon": false,
            "name": "Pitivi",
            "packageName": "pitivi"
        }, {
            "adaptativeIcon": true,
            "name": "Plasma",
            "packageName": "plasma"
        }, {
            "adaptativeIcon": false,
            "name": "Postman",
            "packageName": "postman"
        }, {
            "adaptativeIcon": false,
            "name": "Preferences System Privacy",
            "packageName": "preferences-system-privacy"
        }, {
            "adaptativeIcon": false,
            "name": "Preferences Tweaks",
            "packageName": "preferences-tweaks-anim"
        }, {
            "adaptativeIcon": false,
            "name": "qTransmission",
            "packageName": "qtransmission"
        }]
}
