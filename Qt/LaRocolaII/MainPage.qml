// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.symbian 1.1

import LaRocola 1.0

Page
{
    id: mainPage

    FontLoader { id: myFont; name: "Arial" }
    // FontLoader { id: myFont; source: "SegoePrintBold.ttf" }

    Rectangle
    {
        id: mainRectangle
        width: 360; height: 554

        Image
        {
            id: screen
            source: "Screen.jpg"
            width: parent.width; height: parent.height
            //anchors.right: parent.right
            //anchors.verticalCenter: parent.verticalCenter
        }

        MouseArea
        {
            anchors.fill: parent
            property int mx
            property int my

            onPressed:
            {
                if( mouseY < 360 && mouseY % 30 > 12 )
                {
                    clicList.opacity = 0.7;
                    clicList.y = 30 * Math.floor(( mouseY - 5 ) / 30 ) + 5;
                }
                else if( mouseY >= 380 && mouseY < 500 )
                {
                    clicCmd.opacity = 0.7;
                    clicCmd.x = 60 * Math.floor( mouseX / 60 ) - 15;
                    clicCmd.y = 60 * Math.floor(( mouseY - 20 ) / 60 ) + 5;
                }
            }

            onReleased:
            {
                clicCmd.opacity = 0.0;
                clicList.opacity = 0.0;
            }

            onClicked:
            {
                mx = Math.floor( mouseX / 60 )
                my = Math.floor( mouseY / 30 )
                if( mouseY < 360 )
                {
                    if( mouseY % 30 > 12 )
                        udp.sendCmd( my );
                }
                else if( mouseY < 380 )
                    ;
                else if( mouseY < 440 )
                    udp.sendCmd( mx + 20 );
                else if( mouseY < 500 )
                    if( mouseX < 60 )
                        Qt.quit();
                    else
                        udp.sendCmd( mx + 26 );
            }
        }

        Rectangle
        {
            id: clicCmd
            width: 90; height: 90
            radius: 45
            opacity: 0.0
        }

        Rectangle
        {
            id: clicList
            x: 5
            width: 350; height: 30
            radius: 15
            opacity: 0.0
        }
    }

    // Root line
    ListText
    {
        x: 10; y: 6
        color: "#943232"
        text: '/'
    }

    // List lines
    ListView
    {
        x: 10; y: 36
        height: 330
        model: udp.lines
        delegate: Rectangle
        {
            height: 29.5; anchors.margins: 0
            ListText
            {
                color: udp.colorFile( index )
                text: modelData
            }
        }
    }

    // Status line 1
    ListText
    {
        id: statusText
        x: 10
        y: 494
        width: mainRectangle.width - 20
        height: 30
        wrapMode: Text.NoWrap
        color: "#7814AA"
        text: udp.status1
        property int doAnim: -1

        onTextChanged:
        {
            x = 10;
            doAnim = paintedWidth - width;
            statusAnim.running = doAnim > 0;
            statusAnim.duration = 4000 * ( 1 + ( doAnim > 0 ? doAnim / width : 0 ));
        }

        NumberAnimation
        {
            id: statusAnim
            alwaysRunToEnd: false
            targets: statusText
            property: "x"
            from: 10
            to: statusText.width - statusText.paintedWidth - 20
            // duration: 4000 * ( 1 + ( statusText.paintedWidth - statusText.width > 0 ? ( statusText.paintedWidth - statusText.width ) / statusText.width : 0 ))
            Component.onCompleted:
                if( statusText.doAnim > 0 )
                    running = true;
            onRunningChanged:
                if( ! running )
                    if( statusText.doAnim > 0 )
                        running = true;
                    else
                        x = 10;
        }
    }

    // Status line 2
    ListText
    {
        x: 10; y: 524
        color: "#7814AA"
        text: udp.status2
    }

    UdpClient
    {
        id: udp
    }
}
