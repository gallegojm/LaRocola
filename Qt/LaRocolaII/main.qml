// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.symbian 1.1

Window
{
    id: window

    StatusBar
    {
        id: statusBar
        anchors.top: window.top
    }

    PageStack
    {
        id: pageStack
        anchors { left: parent.left; right: parent.right; top: statusBar.bottom; bottom: toolBar.top }
    }

    ToolBar
    {
        id: toolBar
        anchors.bottom: window.bottom
        tools: ToolBarLayout
        {
            id: toolBarLayout
            /*
            ToolButton
            {
                flat: true
                iconSource: "toolbar-back"
                onClicked: pageStack.depth <= 1 ? Qt.quit() : pageStack.pop()
            }
            */
        }
    }

    Component.onCompleted:
    {
        pageStack.push( Qt.resolvedUrl( "MainPage.qml" ))
    }
}

