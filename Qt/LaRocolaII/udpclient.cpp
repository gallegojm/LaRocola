#include "udpclient.h"

// liste des commandes
char scmd[] = "b<ps>nxi m-+v";

UdpClient::UdpClient( QObject *parent ) : QObject( parent )
{
    hostIp.setAddress( QString( LAROCOLA_IP ));

    #ifdef DEBUG
        if( udpSocket.bind( SERVER_PORT ))
            qDebug() << "Ready to read";
        else
            qDebug() << "Not ready to read";
    #else
        udpSocket.bind( SERVER_PORT );
    #endif
    QObject::connect( &udpSocket, SIGNAL( readyRead() ), this, SLOT( getData() ));

    moreLines = ". . .";
    for( int i = 0; i < NUM_LINE; i++ )
    {
        linesList.append( " " );
        filesList.append( " " );
    }
    // visualise la version de la telecommande sur la derniere ligne de la liste
    linesList.replace( NUM_LINE - 1, VERSION );

    // demande la version de LaRocola (commande 'v')
    sendCmd( 32 );

    // attend 2 secondes et demande la liste des fichiers
    depth = 0;
    QTimer::singleShot( 2000, this, SLOT( sendCmdListRoot()));
}

UdpClient::~UdpClient()
{
    udpSocket.close();
}

void UdpClient::sendDatagram( char * cmd )
{
    QByteArray datagram( cmd );

    udpSocket.writeDatagram( datagram, hostIp, 8888 );
    #ifdef DEBUG
        qDebug() << "Sending: " << datagram;
    #endif
}

void UdpClient::sendCmd( int cmd )
{
    #ifdef DEBUG
        qDebug() << "Command: " << cmd;
    #endif
    if( cmd == 0 )
    {
        clearList( 0 );
        sendCmdList( "/" );
    }
    else if( cmd <= NUM_LINE )
    {
        cmd --;
        QByteArray fn = filesList.at( cmd );
        if( fn.length() > 0 )
        {
            QByteArray path;
            if( type( fn ) >= 1 ) // fichier mp3
            {
                for( int i = 0; i < depth; i++ )
                    path.append( "/" + filesList.at( i ));
                path.append( "/" + filesList.at( cmd ));
                sendCmdPlay( path );
            }
            else
            {
                if( cmd < depth )
                    depth = cmd;
                for( int i = 0; i < depth; i++ )
                    path.append( "/" + filesList.at( i ));
                if( filesList.at( cmd ) == moreLines )
                    path.append( "/" + filesList.at( cmd - 1 ));
                else
                {
                    path.append( "/" + filesList.at( cmd ) + "/" );
                    filesList.replace( depth, filesList.at( cmd ));
                    linesList.replace( depth, linesList.at( cmd ));
                    // typesList.replace( depth, typesList.at( cmd ));
                    depth ++;
                }
                clearList( depth );
                sendCmdList( path.data() );
            }
        }
    }
    else if( cmd == 28 )
        ; //sendCmdPlay( "/The Beatles/Rubber Soul/01 - Drive My Car.mp3" );
    else if( cmd >= 20 )
    {
        // send *x* where x is the command
        cmd -= 20;
        char c[4];
        c[0] = '*';
        c[1] = scmd[ cmd ];
        c[2] = '*';
        c[3] = 0;
        sendDatagram( c );
    }
}

void UdpClient::sendCmdList( char * dir )
{
    // maxline - depth suffit à remplir la liste.
    // mais on demande une ligne de plus pour savoir s'il y en a d'autre
    QByteArray cmd;
    cmd.append( "*l*" );
    cmd.append( (char) ('0' + NUM_LINE - depth + 1 ));
    cmd.append( dir );
    sendDatagram( cmd.data());
}

void UdpClient::sendCmdPlay( QByteArray play )
{
    QByteArray cmd;
    cmd.append( "*f*" );
    cmd.append( play );
    sendDatagram( cmd.data());
}

void UdpClient::getData()
{
    QByteArray datagram;

    #ifdef DEBUG
        qDebug() << "Receiving:";
    #endif
    do
    {
        datagram.resize( udpSocket.pendingDatagramSize() );
        udpSocket.readDatagram( datagram.data(), datagram.size() );
        #ifdef DEBUG
            qDebug() << datagram;
        #endif
        char c = datagram.at(0);
        int n = datagram.at(1) - '0';
        if( c == 'i' )
        {
            if( n == 1 )
                statusline1 = datagram.mid( 2 );
            else if( n == 2 )
                statusline2 = datagram.mid( 2 );
            emit statusChanged();
        }
        else if( c == 'l' )  // l premiere lettre de list, pas "un"
            if( n >= 0 )
            {
                if( n + depth < NUM_LINE )
                {
                    linesList.replace( n + depth, format( datagram.mid( 2 ), depth ));
                    filesList.replace( n + depth, datagram.mid( 2 ));
                }
                else
                {
                    linesList.replace( NUM_LINE - 1, ident( moreLines, depth ));
                    filesList.replace( NUM_LINE - 1, moreLines );
                }
                emit linesChanged();
            }
    }
    while( udpSocket.hasPendingDatagrams() );
}

QStringList UdpClient::lines()
{
    // return filesList;
    return linesList;
}

QString UdpClient::status1()
{
    return statusline1.data();
}

QString UdpClient::status2()
{
    return statusline2.data();
}

void UdpClient::clearList( int d )
{
    for( int i = d; i < NUM_LINE; i++ )
    {
        linesList.replace( i, "" );
        filesList.replace( i, "" );
    }
    depth = d;
    emit linesChanged();
}

int UdpClient::type( QByteArray file )
{
    int lf = file.length();
    int pp = file.lastIndexOf( '.' );
    if( pp == -1 || lf - pp > 5 )
        return 0;
    QByteArray ext = file.right( lf - pp - 1 );
    ext = ext.toLower();
    if( ext == "mp3" || ext == "aac" || ext == "flac" ||
        ext == "ogg" || ext == "wav" || ext == "wma" )
        return 1;
    else if( ext == "rad" )
        return 2;
    return 0;
}

QString UdpClient::colorFile( int ind )
{
    switch( type( filesList.at( ind )))
    {
      case 1:
        return "#1E9A5A";
      case 2:
        return "#7070FF";
      default:
        return "#943232";
    }
}

QByteArray UdpClient::format( QByteArray l, int d )
{
    QByteArray f = l;
    // retire "01 - " ou "01-"
    int i = 0;
    while( f.length() > i )
    {
        char c = f.at( i );
        if( c >= '0' && c <= '9')
            i++;
        else
            break;
    }
    while( i > 0 && f.length() > i )
    {
        char c = f.at( i++ );
        if( c == '-' )
            break;
        else if( c != ' ' )
            i = -1;
    }
    while( i > 1 && f.length() > i )
    {
        if( f.at( i ) != ' ' )
            break;
        i++;
    }
    if( i > 0 )
        f.remove( 0, i );

    // retire l'extension ( 4 ou 5 caracteres )
    if( f.length() > 4 && type( l ) >= 1 )
    {
        int lc = f.length() - f.lastIndexOf( '.' );
        if( lc <= 5 )
            f.chop( lc );
    }
    f = ident( f, d );
    return f;
}

QByteArray UdpClient::ident( QByteArray l, int d )
{
    QByteArray f = l;
    if( d > 0 )
        f.insert( 0, QByteArray( 3*d, ' ' ));
    return f;
}
