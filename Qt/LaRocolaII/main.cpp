#include <QtCore/QDebug>
#include <QtGui/QApplication>
#include <QtDeclarative>

#include "udpclient.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDeclarativeView view;

    qmlRegisterType<UdpClient>( "LaRocola", 1, 0, "UdpClient" );

    view.setAttribute( Qt::WA_LockPortraitOrientation );
    view.setSource( QUrl( "qrc:/main.qml" ));
    view.showFullScreen();

    return app.exec();
}
