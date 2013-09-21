#ifndef UDPGET_H
#define UDPGET_H

#include <QObject>
#include <QtNetwork>

#include "config.h"

class UdpClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QStringList lines READ lines NOTIFY linesChanged )
    Q_PROPERTY( QString status1 READ status1 NOTIFY statusChanged )
    Q_PROPERTY( QString status2 READ status2 NOTIFY statusChanged )

public:
    explicit UdpClient( QObject *parent = 0 );
    ~UdpClient();
    QStringList lines();
    QString status1();
    QString status2();

signals:
    void linesChanged();
    void statusChanged();

public slots:
    void sendCmd( int cmd );
    QString colorFile( int ind );
    void sendCmdListRoot() { sendCmdList( "/" ); };

private slots:
    void getData();

private:
    void sendDatagram( char *cmd );
    void sendCmdPlay( QByteArray play );
    void sendCmdList( char * dir );
    void clearList( int d );
    int type( QByteArray file );
    QByteArray format( QByteArray l, int d );
    QByteArray ident( QByteArray l, int d );

    QUdpSocket udpSocket;
    QHostAddress hostIp;
    QByteArray statusline1;
    QByteArray statusline2;
    QByteArray moreLines;
    QList<QByteArray> filesList;
    QStringList linesList;
    int depth;
};

#endif // UDPGET_H
