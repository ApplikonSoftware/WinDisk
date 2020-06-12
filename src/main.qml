import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls.Universal 2.3
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import QtQuick.Dialogs 1.3

Window
{
    visible: true
    width: 480
    height: 400
    title: "Win Disk (Version: " + guiManager.version + ")"

    Universal.theme: Universal.Dark
    Universal.background: "#222222"
    Universal.accent: Universal.Cyan

    FileDialog
    {
        id: fileDialog
        title: "Please choose a image file"
        nameFilters: [ "Applikon Disk Image (*.adi)" ]
        folder: guiManager.homeDir
        sidebarVisible: true
        selectExisting: !createButton.checked
        selectFolder: false
        selectMultiple: false
        onAccepted:
        {
            guiManager.imageFileUrl = fileDialog.fileUrl;
        }
    }

    MessageDialog
    {
        id: confirmDialog
        title: "Overwrite?"
        icon: StandardIcon.Warning
        text: "All data on the selected device " + driveEdit.currentText + " will be erased! Do you want to proceed?"
        detailedText: "This operation is irreversible, please make sure no useful data is on the selected device!"
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: guiManager.restoreImage()
    }

    MessageDialog
    {
        id: errorDialog
        title: guiManager.errorType
        icon: StandardIcon.Critical
        text: guiManager.error
        standardButtons: StandardButton.Ok
        onYes:
        {
            errorDialog.close()
            guiManager.error = ""
            guiManager.errorType = ""
        }
    }

    Connections
    {
        target: guiManager
        onShowErrorMsg:
        {
            errorDialog.open();
        }
    }

    Pane
    {
        anchors.fill: parent
        Universal.background: "#222222"
        ColumnLayout
        {
            id: contentLayout
            anchors.fill: parent
            Item
            {
                id: funcSelection
                Layout.preferredHeight: 40
                Layout.fillWidth: true
                Layout.bottomMargin: 10
                ButtonGroup
                {
                    id: funcGroup
                    buttons: funcLayout.children
                }
                RowLayout
                {
                    id: funcLayout
                    anchors.fill: parent
                    spacing: 0
                    Button
                    {
                        id: createButton
                        enabled: !guiManager.busy
                        checkable: true
                        checked: true
                        text: "Create Image"
                        font.pixelSize: 16
                        Layout.fillWidth: true
                        Layout.preferredWidth: 0
                        Layout.fillHeight: true
                    }
                    Button
                    {
                        id: restoreButton
                        enabled: !guiManager.busy
                        checkable: true
                        text: "Restore Image"
                        font.pixelSize: 16
                        Layout.fillWidth: true
                        Layout.preferredWidth: 0
                        Layout.fillHeight: true
                        onClicked:
                        {
                            guiManager.cancel();
                        }
                    }
                }
            }
            Label
            {
                id: deviceLabel
                Layout.preferredHeight: 20
                Layout.fillWidth: true
                font.pixelSize: 16
                text: createButton.checked ? "Select source device" : "Select target device"
            }
            ComboBox
            {
                id: driveEdit
                Layout.preferredHeight: 40
                Layout.fillWidth: true
                Layout.bottomMargin: 10
                enabled: !guiManager.busy
                model: guiManager.devices
                textRole: "label"
                font.pixelSize: 16
                font.bold: true
                currentIndex: guiManager.deviceIndex
                delegate: ItemDelegate
                {
                    height: 40
                    width: parent.width
                    text: model.label
                    font.pixelSize: 16
                    font.bold: true
                    onClicked:
                    {
                        guiManager.deviceIndex = index;
                    }
                }
            }
            Label
            {
                id: imageLabel
                Layout.preferredHeight: 20
                Layout.fillWidth: true
                font.pixelSize: 16
                text: createButton.checked ? "Select target image file" : "Select source image file"
            }
            Item
            {
                id: fileSelecter
                enabled: !guiManager.busy
                Layout.preferredHeight: 40
                Layout.fillWidth: true
                Layout.bottomMargin: 10
                TextField
                {
                    id: filePathField
                    anchors { left: parent.left; top: parent.top; bottom: parent.bottom; right: openButton.left }
                    text: guiManager.imageFilePath
                    font.pixelSize: 16
                    onAccepted:
                    {
                        guiManager.imageFilePath = text;
                    }
                }
                ToolButton
                {
                    id: openButton
                    Universal.background: Universal.Steel
                    anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
                    width: height
                    icon.source: "qrc:/images/folder.png"
                    icon.color: Universal.color(Universal.Cyan)
                    onClicked:
                    {
                        fileDialog.open();
                    }
                }
            }
            CheckBox
            {
                id: verifyCheckBox
                checked: guiManager.verify
                text: createButton.checked ? "Verify created image file" : "Verify restored target device"
                font.pixelSize: 16
                Layout.preferredHeight: 30
                Layout.fillWidth: true
                Layout.bottomMargin: 10
                onClicked:
                {
                    guiManager.verify = !guiManager.verify
                }
            }
            Item
            {
                id: emptyItem
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
            Item
            {
                id: buttonsItem
                Layout.preferredHeight: 40
                Layout.fillWidth: true
                RowLayout
                {
                    anchors.fill: parent
                    Button
                    {
                        id: startButton
                        enabled: createButton.checked ? guiManager.canRead : guiManager.canWrite
                        text: createButton.checked ? "Start Creating" : "Start Restoring"
                        font.pixelSize: 16
                        Layout.fillWidth: true
                        Layout.preferredWidth: 0
                        Layout.fillHeight: true
                        onClicked:
                        {
                            createButton.checked ? guiManager.createImage() : confirmDialog.open()
                        }
                    }
                    Button
                    {
                        id: cancelButton
                        enabled: guiManager.canCancel
                        text: "Cancel"
                        font.pixelSize: 16
                        Layout.fillWidth: true
                        Layout.preferredWidth: 0
                        Layout.fillHeight: true
                        onClicked:
                        {
                            guiManager.cancel();
                        }
                    }
                }
            }
            ProgressBar
            {
                id: exportProgressBar
                from: 0
                to: 1
                value: guiManager.progress
                height: 5
                Layout.fillWidth: true
                Layout.topMargin: 10
            }
            Label
            {
                id: statusLabel
                Layout.preferredHeight: 40
                Layout.fillWidth: true
                text: guiManager.message
                font.pixelSize: 16
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
