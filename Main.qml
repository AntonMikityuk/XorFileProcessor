import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls.Material

ApplicationWindow {
    width: 650
    height: 750
    visible: true
    title: "XOR File Processor"

    Material.theme: Material.Light
    Material.accent: Material.Indigo

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 15

        GroupBox {
            title: "Configuration"
            Layout.fillWidth: true

            label: Text {
                text: parent.title
                color: "#333333"
                font.bold: true
                leftPadding: 10
            }

            GridLayout {
                columns: 3
                anchors.fill: parent
                rowSpacing: 12
                columnSpacing: 10

                Text { text: "Input Folder:"; color: "#2c3e50"; font.pixelSize: 13 }
                TextField {
                    id: inPath
                    placeholderText: "Select input folder"
                    Layout.fillWidth: true
                    selectByMouse: true
                }
                Button { text: "Browse"; onClicked: inDirDialog.open() }

                Text { text: "Output Folder:"; color: "#2c3e50"; font.pixelSize: 13 }
                TextField {
                    id: outPath
                    placeholderText: "Select destination folder"
                    Layout.fillWidth: true
                    selectByMouse: true
                }
                Button { text: "Browse"; onClicked: outDirDialog.open() }

                Text { text: "File Mask:"; color: "#2c3e50"; font.pixelSize: 13 }
                TextField {
                    id: maskStr
                    placeholderText: "*.txt"
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                }

                Text { text: "XOR Key (Hex):"; color: "#2c3e50"; font.pixelSize: 13 }
                TextField {
                    id: keyStr
                    placeholderText: "16 Hex symbols"
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    maximumLength: 16
                    validator: RegularExpressionValidator { regularExpression: /[0-9A-Fa-f]{0,16}/ }
                    onTextChanged: text = text.toUpperCase()
                }

                Text { text: "If file exists:"; color: "#2c3e50"; font.pixelSize: 13 }
                ComboBox {
                    id: modeSelect
                    model: ["Overwrite", "Add counter"]
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                }

                Text { text: "Run Mode:"; color: "#2c3e50"; font.pixelSize: 13 }
                ComboBox {
                    id: timerSelect
                    model: ["One Time Run", "Periodic Timer"]
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                }

                Text {
                    text: "Interval (sec):"; color: "#2c3e50"; font.pixelSize: 13
                    visible: timerSelect.currentIndex === 1
                }
                SpinBox {
                    id: timerVal
                    from: 1; to: 3600
                    value: 5
                    visible: timerSelect.currentIndex === 1
                    Layout.columnSpan: 2
                }

                Text { text: "Cleaning:"; color: "#2c3e50"; font.pixelSize: 13 }
                CheckBox {
                    id: delInput
                    text: "Delete original files"
                    Layout.columnSpan: 2
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            ProgressBar {
                id: progBar
                Layout.fillWidth: true
                value: backend.progress / 100
            }

            Text {
                text: "Status: " + backend.status
                font.bold: true
                font.pixelSize: 14
                color: backend.status === "Stopped" ? "#e74c3c" : "#2980b9"
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 20

            Button {
                text: "START"
                onClicked: backend.start(inPath.text, outPath.text, maskStr.text,
                                         keyStr.text, delInput.checked, modeSelect.currentIndex === 0,
                                         timerSelect.currentIndex === 1, timerVal.value)
                enabled: !backend.isBusy
                highlighted: true
            }

            Button {
                text: "PAUSE"
                enabled: backend.isBusy && !backend.isPaused
                onClicked: backend.pause()
            }

            Button {
                text: "RESUME"
                enabled: backend.isPaused
                onClicked: backend.resume()
            }

            Button {
                text: "STOP"
                enabled: backend.isBusy
                onClicked: backend.stop()
                Material.foreground: enabled ? Material.Red : Material.Grey
            }
        }
    }

    FolderDialog {
        id: inDirDialog
        title: "Choose Input Folder"
        onAccepted: inPath.text = selectedFolder
    }
    FolderDialog {
        id: outDirDialog
        title: "Choose Output Folder"
        onAccepted: outPath.text = selectedFolder
    }
}