/*******************************************************************************
**                                                                            **
**                               La Rocola II                                 **
**      un lecteur MP3 et radio télécommandable depuis smartphone ou PC       **
**                                                                            **
**        Copyright (c) 2013 Jean-Michel Gallego. All rights reserved.        **
**                            http://larocola.net                             **
**                                                                            **
**    Ce programme est libre, vous pouvez le redistribuer et/ou le modifier   **
** selon les termes de la Licence Publique Générale GNU publiée par la Free   **
** Software Foundation (version 2 ou bien toute autre version ultérieure      **
** choisie par vous).                                                         **
**                                                                            **
**    Ce programme est distribué car potentiellement utile, mais SANS AUCUNE  **
** GARANTIE, ni explicite ni implicite, y compris les garanties de            **
** commercialisation ou d'adaptation dans un but spécifique. Reportez-vous à  **
** la Licence Publique Générale GNU pour plus de détails.                     **
**                                                                            **
**    Vous devez avoir reçu une copie de la Licence Publique Générale GNU en  **
** même  temps que ce programme; si ce n'est pas le cas, écrivez à la         **
** Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,        **
** MA 02111-1307, États-Unis.                                                 **
**                                                                            **
**    This programm is free software: you can redistribute it and/or modify   **
** it under the terms of the GNU General Public License as published by       **
** the Free Software Foundation, either version 3 of the License, or          **
** (at your option) any later version.                                        **
**                                                                            **
**    This library is distributed in the hope that it will be useful,         **
** but WITHOUT ANY WARRANTY; without even the implied warranty of             **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              **
** GNU General Public License for more details.                               **
**                                                                            **
**    You should have received a copy of the GNU General Public License       **
** along with this program.  If not, see <http://www.gnu.org/licenses/>.      **
**                                                                            **
*******************************************************************************/

#include <Streaming.h>
#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <SFEMP3Shield.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "Define.h"
#include "SdStream.h"
#include "Radio.h"
#include "MemoryFree.h"
#include "SdLongNameFile.h"

//char lmp3[] = "La Rocola II V-13.09.19";
char lmp3[] = "La Rocola II V-2.00";

// Objets et variables pour la comunication avec le module MP3

SFEMP3Shield mp3;
char PConfigFile[] = "/LaRocola.cfg";
uint8_t volume, volume0;
uint8_t mp3Status;
char cmd;
boolean isPlaying = false;       // indicate a radio or a file except bip or error is playing
boolean cmdError = false;
boolean mute = false;
char status = 'S';
uint32_t millisLastB  = 0L; // valeur de millis() au dernier appel à la commande 'b'
uint32_t millisLastM1 = 0L; // valeur de millis() au dernier appel à la commande '-'
uint32_t millisLastM2 = 0L; // valeur de millis() à l'avant dernier appel à la commande '-'
uint32_t millisLastP1 = 0L; // valeur de millis() au dernier appel à la commande '+'
uint32_t millisLastP2 = 0L; // valeur de millis() à l'avant dernier appel à la commande '+'
uint32_t millisec;
uint8_t deltaVolume = 1;
char bip[MAX_FILESOUND];
char erreur[MAX_FILESOUND];

// Objets et variables pour la comunication avec le lecteur de carte

SdFat sd;
SdBaseFile* dirFile;
char bufFile[ MAX_READBUF ];
char filePlaying[ MAX_READBUF ];
char * namePlaying;
char nameNext[ MAX_FILENAME ];

// Objet et variables pour l'ecoute radio

Radio clientRadio( mp3 );
char radioName[MAX_FILENAME];
char radioServer[MAX_FILENAME];
char radioUrl[MAX_FILENAME];
uint16_t radioPort;
char radioStat1[ 2 * MAX_FILENAME ];
uint32_t porcFill, bRate;

// Objet et variables pour la liaison UDP

EthernetUDP udp;
MacAddress mac;     // adresse MAC imprimée au dos du module Arduino Ethernet
IPAddress localIp, remoteIp;
IPAddress broadcastIp( 1, 2, 3, 255 );
uint16_t localPort, remotePort;
char packetBuffer[MAX_UDP_TX_PACKET];      //buffer pour les paquets UDP entrant
char infoStatus[MAX_FILENAME];

/*******************************************************************************
**                                                                            **
**                               INITIALISATION                               **
**                                                                            **
*******************************************************************************/

void setup()
{
  // Disable watchdog
  WDT_Disable( WDT );

  delay( 1000 );
  
  // configure la broche de la LED en sortie et éteind la LED
  pinMode( STAT_LED_PIN, OUTPUT );
  offStatusLed();

  // Control Select of SD card of MP3 shield set to high
  pinMode( SD_CS_PIN, OUTPUT );
  digitalWrite( SD_CS_PIN, HIGH );

  #ifdef DEBUG
    Serial.begin( 115200 );
    Serial << lmp3 << endl;
    #if defined(USE_MP3_REFILL_MEANS)
      #if USE_MP3_REFILL_MEANS == USE_MP3_Polled
        Serial << "MP3 shield data request is polled" << endl; 
      #else
        Serial << "Must define MP3 shield as polled" << endl;
        while( true ) ; 
      #endif
    #endif
  #endif

  // un clignotement indique le démarrage du programme
  okMsgLed( 1 );

  // initialisation du lecteur de carte du module MP3
  #ifdef DEBUG
    Serial << "Initialisation du lecteur de carte. Chip Select " << SD_SEL << endl;
  #endif
  if( ! sd.begin( SD_CS_PIN, SPI_HALF_SPEED ))
    errMsgLed( CODE_SDRD, true );
  okMsgLed( CODE_SDRD );
 
  // ouverture en lecture du fichier de configuration
  #ifdef DEBUG
    Serial << "Lecture du fichier de configuration" << endl;
  #endif
  if( ! sd.chdir( "/" ))
    errMsgLed( CODE_CONF, true );
  char welcome[MAX_FILESOUND];
  SdStream gsConfig( PConfigFile, bufFile, MAX_READBUF );
  if( ! gsConfig )
    errMsgLed( CODE_CONF, true );
    
  // lecture de l'adresse IP de La Rocola
  localIp = gsConfig.getIp();
  // Supprimer les // d'une des lignes suivantes pour fixer l'adresse IP ou la mac
  //   sans modifier le fichier /LaRocola.cfg de la carte mémoire microSD
  // mac = { 0x90, 0xA2, 0xDA, 0x0D, 0xA3, 0xA0 };
  // localIp = IPAddress( 192, 168, 1, 102 );
  // localIp = IPAddress( 0, 0, 0, 0 );
  
  // lecture de l'adresse MAC de l'Arduino Ethernet
  mac = gsConfig.getMac();
    
  // lecture du numéro du port UDP sur lequel La Rocola écoute
  localPort = gsConfig.getInt();
  
  // lecture de la valeur initiale du volume sonore
  volume = (uint8_t) gsConfig.getInt();
  
  // lecture du nom du fichier du son 'bip'
  gsConfig.getline( bip, MAX_FILESOUND );
  
  // lecture du nom du fichier du son 'erreur'
  gsConfig.getline( erreur, MAX_FILESOUND );

  // lecture du nom du fichier du son de bienvenue
  // comme c'est le dernier paramètre lu, on vérifie que cela se fait sans erreur
  if( gsConfig.getline( welcome, MAX_FILESOUND ) == 0)
    errMsgLed( CODE_CONF, true );
  okMsgLed( CODE_CONF );
  gsConfig.close();

  // initialisation du reproducteur MP3
  #ifdef DEBUG
    Serial << F( "Initialisation de MP3" ) << endl;
  #endif
  if( mp3.begin() != 0 )  
    errMsgLed( CODE_RMP3, true );
  okMsgLed( CODE_RMP3 );
  
  // initialisation du serveur UDP
  #ifdef DEBUG
    Serial << "Lancement du serveur udp" << endl;
  #endif
  delay( 1000 );
  if( localIp[0] == 0 && localIp[1] == 0 && localIp[2] == 0 && localIp[3] == 0 )
    if( Ethernet.begin( mac.b ) == 0 )
      errMsgLed( CODE_NET, true );
    else
      localIp = Ethernet.localIP();
  else
    Ethernet.begin( mac.b, localIp );
  okMsgLed( CODE_NET );

  if( udp.begin( localPort ) == 0 )
    errMsgLed( CODE_UDP, true );
  okMsgLed( CODE_UDP );

  // valeur initiale du volume et reproduction du son de bienvenue
  mp3.setVolume( volume, volume );
  mp3.playMP3( welcome );
  
  #ifdef DEBUG
    Serial << "Configuration ok!" << endl
           << "Adresse IP: " << localIp[0] << "." << localIp[1] << "." 
                                  << localIp[2] << "." << localIp[3] << endl
           << "Mac: " << mac[0] << "-" << mac[1] << "-" << mac[2] << "-" 
                           << mac[3] << "-" << mac[4] << "-" << mac[5] << endl
           << "Port: " << localPort << endl;
    Serial << "Volume: " << volume << endl
           << "Son de bip: " << bip << endl
           << "Son d'erreur: " << erreur << endl
           << "Son de bienvenue: " << welcome << endl;
    Serial << "RAM libre: " << freeRam() << endl;
  #endif
  
  millisec = millis();
}

/*******************************************************************************
**                                                                            **
**                              BOUCLE PRINCIPALE                             **
**                                                                            **
*******************************************************************************/

void loop()
{
  boolean sendinfo = false;
  
  mp3Status = mp3.getState();
  if( mp3Status == playback )
    mp3.available();
  else if( mp3Status == streaming )
    clientRadio.streamRadio();

  // Vérifie si un paquet de données est présent sur le port Udp
  int packetSize = udp.parsePacket();
  if( packetSize )
  {
    // mémorise l'adresse IP et le port distants
    onStatusLed();
    remoteIp = udp.remoteIP();
    remotePort = udp.remotePort();
    #ifdef DEBUG
      Serial << "Réception d'un paquet de " << packetSize << " octets" << endl
             << "Depuis " << remoteIp[0] << "." << remoteIp[1] << "." 
                          << remoteIp[2] << "." << remoteIp[3]
             << ", port " << remotePort << endl;
    #endif

    // lit le paquet
    udp.read( packetBuffer, MAX_UDP_TX_PACKET );
    if( packetSize > MAX_UDP_TX_PACKET )
      packetSize = MAX_UDP_TX_PACKET;
    packetBuffer[packetSize] = 0;
    #ifdef DEBUG
      Serial << "Contenu: " << packetBuffer << endl;
    #endif

    // vérifie l'intégrité du paquet
    // les commandes commencent toutes par '*x*' où x est l'abréviation de la commande
    if( packetBuffer[0] == '*' && packetBuffer[2] == '*' )
      cmd = packetBuffer[1];
    else
      cmd = ' ';
      
    #ifdef DEBUG
      Serial << "Status " << status << " Commande " << cmd<< endl;
    #endif

    // traitement des commandes
    cmdError = false;
    switch( cmd )
    {
      case 'f':  // joue un fichier
        stop();
        isPlaying = true;
        storeFilePlaying( packetBuffer + 3 );
        isPlaying = play( filePlaying );
        break;
      case 'p':  // pause
        // si est en train de reproduire un fichier...
        if( mp3Status == playback )
          mp3.pauseMusic();
        else if( mp3Status == paused_playback )
          mp3.resumeMusic();
        else if( mp3Status == streaming )
          clientRadio.stopRadio(); // mp3.stopStream(); 
        // else if( mp3Status == pause_streaming )
        //    
        break;
      case 's':  // stop
        stop();
        isPlaying = false;
        break;
      case '-':  // diminue le volume
        mute = false;
        if( millis() - millisLastM2 >= 2000 )   // si plus de 2 secondes entre 3 clics...
          deltaVolume = 1;                      //  ... incrément réinitialisé à 0,5 dB
        else if( deltaVolume < 8 )              // sinon...
          deltaVolume *= 2;                     //  ... incrément double jusqu'à 4 dB
        if( volume + deltaVolume < 254 )
          volume += deltaVolume;
        else
          volume = 254;
        millisLastM2 = millisLastM1;
        millisLastM1 = millis();
        millisLastP2 = 0;
        millisLastP1 = 0;
        mp3.setVolume( volume, volume );
        break;
      case '+':  // augmente le volume
        mute = false;
        if( volume == 0 )    // déjà au maximum
        {
          cmdError = true;
          break;
        }
        if( millis() - millisLastP2 >= 2000 )
          deltaVolume = 1;
        else if( deltaVolume < 8 )
          deltaVolume *= 2;
        if( volume - deltaVolume >= 0 )
          volume -= deltaVolume;
        else
          volume = 0;
        millisLastP2 = millisLastP1;
        millisLastP1 = millis();
        millisLastM2 = 0;
        millisLastM1 = 0;
        mp3.setVolume( volume, volume );
        break;
      case 'm':  // silencieux
        if( mute )
        {
          volume = volume0;
          mp3.setVolume( volume, volume );
          mute = false;
        }
        else
        {
          mute = true;
          volume0 = volume;
          volume += 30;
          mp3.setVolume( volume, volume );
        }
        break;
      case '<':  // recule de 5 secondes - jump back 5 seconds
        if(( mp3Status == playback || mp3Status == paused_playback ) && fileType( namePlaying ) == 2 )
          mp3.skip( -5000 );
        break;
      case '>':  // avance de 5 secondes - jump forward 5 seconds
        if(( mp3Status == playback || mp3Status == paused_playback ) && fileType( namePlaying ) == 2 )
          mp3.skip( 5000 );
        break;
      case 'b':  // jump back to begin of track. If send twice in less than 3 s, jump to previous track
        if( mp3Status == playback || mp3Status == paused_playback )
          if( millis() - millisLastB >= 3000 )
            mp3.skipTo( 0 );
          else
          {
            previousFile();
            mp3.stopTrack();
            playFile( filePlaying );
          }
        millisLastB = millis();
        break;
      case 'n':  // jump to next track
        if( mp3Status == playback || mp3Status == paused_playback )
        {
          mp3.stopTrack();
          if( strlen( nameNext ) > 0 )
          {
            strncpy( namePlaying, nameNext, MAX_READBUF - ( namePlaying - filePlaying )); 
            isPlaying = playFile( filePlaying );
          }
          else
            isPlaying = false;
        }
        break;
      case 'l':  // liste des fichiers
        cmdError = ! sendList( packetBuffer + 3 );
        break;
      case 'v':  // envoie la version
      case 'i':  // envoie les lignes d'information
        break;
      default:
        cmdError = true;
    }

    // envoie informations sauf en cas de commande 'liste'
    if( cmd != 'l' )
      sendinfo = true; // sendInfo();
    
    // Joue des sons pour indiquer l'état seulement
    //   si n'est pas en train de reproduire autre chose
    if( ! isPlaying )
      if( cmdError )
        mp3.playMP3( erreur );
      else
        mp3.playMP3( bip );
    
    offStatusLed();
    #ifdef DEBUG
      Serial << "RAM libre: " << freeRam() << endl;
    #endif

    // vide éventuellement le buffer de paquets udp
    while( udp.available() )
      udp.read();
  }

  // Si la reproduction est terminée, passe au fichier suivant
  mp3Status = mp3.getState();
  if( mp3Status == ready && isPlaying )
  {
    if( strlen( nameNext ) > 0 )
    {
      strncpy( namePlaying, nameNext, MAX_READBUF - ( namePlaying - filePlaying )); 
      isPlaying = playFile( filePlaying );
    }
    else
      isPlaying = false;
    sendinfo = true; // sendInfo();
  }
  
  // Envoie lignes d'information a la telecommande toutes les secondes
  // Si en mode radio, actualise les statistiques radio 
  if( sendinfo || (uint32_t) ( millis() - millisec ) > 1000 )
  {
    millisec = millis();
    if( mp3Status == streaming )
      clientRadio.statistic( radioStat1, & porcFill, & bRate );
    sendInfo();
  }
}

/*******************************************************************************
**                                                                            **
**                      LANCE LA REPRODUCTION D'UN FICHIER                    **
**                                                                            **
*******************************************************************************/

// lance la reproduction du fichier pointé par 'file' (nom complet)
// renvoie TRUE si la reproduction a bien démarré

boolean play( char * file )
{
  if( fileType( file ) == 1 )
    return playRadio( file );
  return playFile( file );
}

/*******************************************************************************
**                                                                            **
**                    LANCE LA REPRODUCTION D'UN FICHIER MP3                  **
**                                                                            **
*******************************************************************************/

// lance la reproduction du fichier pointé par 'file' (nom complet long)
// renvoie TRUE si la reproduction a bien démarré

boolean playFile( char * file )
{
  char shortName[ MAX_FILENAME ];
  
  if( ! long2shortPath( shortName, file, MAX_FILENAME ))
    return false;
  int8_t r = mp3.playMP3( shortName );
  if( r == 0 && isPlaying )
  {
    // tracklength = rmp3.gettracklength( file );
    storeNextFile( file );
  }
  return r == 0;
}

/*******************************************************************************
**                                                                            **
**                  LANCE LA REPRODUCTION D'UN FICHIER RADIO                  **
**                                                                            **
*******************************************************************************/

// lance la reproduction du fichier pointé par 'file' (nom complet)
// renvoie TRUE si la reproduction a bien démarré

boolean playRadio( char * file )
{
  char shortName[ MAX_FILENAME ];
  
  if( ! long2shortPath( shortName, file, MAX_FILENAME ))
    return false;
  // lit les donnees de la radio dans le fichier 'file'
  SdStream gsRadio( shortName, bufFile, MAX_READBUF );
  if( ! gsRadio )
    return false;
  gsRadio.getline( radioName, MAX_FILENAME );
  gsRadio.getline( radioServer, MAX_FILENAME );
  gsRadio.getline( radioUrl, MAX_FILENAME );
  radioPort = gsRadio.getInt();
  gsRadio.close();
  
  // informe de la prise en compte de la demande de connexion
  sendStatus( radioName );
  
  // se connecte au serveur
  return clientRadio.startRadio( radioName, radioServer, radioUrl, radioPort );
}

/*******************************************************************************
**                                                                            **
**                            STOP LA REPRODUCTION                            **
**                                                                            **
*******************************************************************************/

void stop()
{
  if( mp3Status == playback || mp3Status == paused_playback )
    mp3.stopTrack();
  else if( mp3Status == streaming )
    clientRadio.stopRadio();
}

/*******************************************************************************
**                                                                            **
**                 ENVOIE DES INFORMATIONS A LA TELECOMMANDE                  **
**                                                                            **
*******************************************************************************/

// la première ligne indique l'état du lecteur suivant la réponse a getplaybackstatus()
// la seconde envoie des infos sur le fichier joué et le volume sonore 

void udpSend( char * str )
{
  for( uint8_t i = 0; i < strlen( str ); i ++ )
    udp.write( str[ i ] );
}

// Envoie la ligne de status puis la ligne de statistiques
void sendInfo()
{
  char * ss;

  // envoie status
  if( cmd == 'v' )
    ss = lmp3;
  else if( ! isPlaying )
    ss = "Stopped";
  else switch( mp3Status = mp3.getState() )
  {
    case streaming:
      ss = radioStat1;
      break;
    case playback:
      ss = namePlaying;
      break;
    case paused_playback:
      ss = "Paused";
      break;
    default:
      ss = "Stopped";
  }      
  sendStatus( ss );
  // envoie statistiques    
  sendStatistics();
}

void sendStatus( char * ss )
{
  // udp.beginPacket( multicastIp, remotePort );
  udp.beginPacket( remoteIp, remotePort );
  udpSend( "i1" );                     // lettre 'i' suivie d'un chiffre 'un'
  udpSend( ss );
  udp.endPacket();
}

void sendStatistics()
{
  // udp.beginPacket( multicastIp, remotePort );
  udp.beginPacket( remoteIp, remotePort );
  udpSend( "i2" );
  if( cmd == 'v' )
    udpSend( "by Jean-Michel Gallego" );
  else
  {
    if( mp3Status == streaming )
    {
      udp.print( porcFill, DEC );
      udpSend( "% " );
      udp.print( bRate / 125, DEC );
      udpSend( " kbps " );
    }
    else if( mp3Status == playback )
    {
      uint32_t duration = mp3.getTrackLength();
      if( duration > 0 )
      {
        udp.print( mp3.getCurrentPosition(), DEC );
        udp.write( '/' );
        udp.print( duration , DEC );
        udpSend( " s " );
      }
      uint16_t byte_rate = mp3.getBitRate();
      if( byte_rate > 0 )
      {
        udp.print( byte_rate, DEC ); 
        udpSend( " kbps " );
      }
    }
    //  send volume in dB
    udp.write( '-' );
    union twobyte vol_rl; // create variable that can be both word and double byte of left and right.
    // vol_rl.word = mp3.getVolume(); // returns a double uint8_t of left and right packed into int16_t
    // uint8_t v = ( vol_rl.byte[0] + vol_rl.byte[1] ) / 2;
    // udp.print( v / 2, DEC );           // Envoie le volume en dB avec une seule décimale
    udp.print( volume / 2, DEC );           // Envoie le volume en dB avec une seule décimale
    udp.write( '.' );
    // udp.write( v % 2 == 0 ? '0' : '5' );
    udp.write( volume % 2 == 0 ? '0' : '5' );
    udpSend( " dB" );
  }
  udp.endPacket();
}

/*******************************************************************************
**                                                                            **
**               ENVOIE UNE LISTE DE FICHIERS (ou répertoires)                **
**                                                                            **
*******************************************************************************/

// le premier caractère de param indique le nombre de fichiers à envoyer
// param+1 pointe sur le dossier contenant les fichiers à envoyer
// si param fini par le caractère '/' la liste débute avec le premier fichier
// sinon, elle débute avec le fichier indiqué apres le dernier '/'
//
// examples:
//  8/ : envoie les 8 premiers fichiers du répertoire racine
//  8/Blues : envoie les 8 fichiers suivant 'Blues' du répertoire racine
//  6/Classique/Piano/ envoie les 6 premiers fichiers du répertoire /Classique/Piano
//  6/Classique/Piano/Brahms envoie les 6 fichiers suivant 'Brahms' du répertoire /Classique/Piano

boolean sendList( char * param )
{
  SdLongNameFile sdLn;
  sdLn.setHidePD( true );
  uint8_t nl = param[0] -'0';
  int8_t ft;
  char * dir0 = param + 1;                // pointe sur le dossier 
  char * pln = strrchr( dir0, '/' ) + 1;  // pointe sur le premier fichier
  
  boolean ok = sdLn.search( dir0 );
  while( ok && ! fileFilter( sdLn ))
 // && pln[0] == 0 &&
 //                  strcmp( pln, sdLn.longName() ) != 0 ))
    ok = sdLn.next();
  for( uint8_t i = 0; ok && i < nl; i++ )
  {
    udp.beginPacket( remoteIp, remotePort );
    udp.write( 'l' );        // c'est un 'el' comme dans Lolita, pas un 'un'
    udp.write( '0' + i );
    udpSend( sdLn.longName() );
    udp.endPacket();
    do
      ok = sdLn.next();
    while( ok && ! fileFilter( sdLn ));
  }
  return true;
}

boolean fileFilter( SdLongNameFile sdnf )
{
  return sdnf.isDir() || fileType( sdnf.longName()) > 0;
}

// Envoie un nombre suivant le genre de fichier
// -1 : erreur (nom nul)
// 1 : ".rad" radio
// 2 : ".mp3" 
// 3 : ".fla" ou ".flac"
// 4 : ".wav"
// 5 : ".wma"
// 0 : autre

int8_t fileType( char * name )
{
  char ext[5];
  uint8_t ln = strlen( name );
  if( ln == 0 )
    return -1;
  char * pext = strrchr( name, '.' );
  if( pext == 0 )
    return 0;
  if( ( strrchr( name, 0 ) - pext ) > 5 )
    return 0;
  strncpy( ext, pext + 1, 5 );
  strlwr( ext );
  if( ! strcmp( ext, "rad" ))
    return 1;
  if( ! strcmp( ext, "mp3" ))
    return 2;
  if( ! ( strcmp( ext, "fla" ) && strcmp( ext, "flac" )))
    return 3;
  if( ! strcmp( ext, "aac" ))
    return 4;
  if( ! strcmp( ext, "mid" ))
    return 5;
  if( ! strcmp( ext, "ogg" ))
    return 6;
  if( ! strcmp( ext, "wav" ))
    return 7;
  if( ! strcmp( ext, "wma" ))
    return 8;
  return 0;
}

// Memorise dans filePlaying le chemin/nom long du fichier
// Pointe namePlaying sur le nom long du fichier
// Entree: chemin et nom longs du fichier

boolean storeFilePlaying( char * fullname )
{
  // Memorise dans filePlaying le chemin/nom long du fichier
  strncpy( filePlaying, fullname, MAX_READBUF );
  if( strlen( filePlaying ) >= MAX_READBUF )
    return false;
  // Pointe namePlaying sur le nom long du fichier
  namePlaying = strrchr( filePlaying, '/' ) + 1;
  return true;
}

// Enregistre dans fileNext le nom long complet du fichier suivant
// Entree nom long complet du fichier actuel

boolean storeNextFile( char * file )
{
  SdLongNameFile sdLn;
  sdLn.setHidePD( true );
  
  boolean ok = sdLn.search( file );
  if( ok )
    do
      ok = sdLn.next();
    while( ok && ! fileFilter( sdLn ));
  if( ok )
    strncpy( nameNext, sdLn.longName(), MAX_FILENAME );
  else
    nameNext[0] = 0;
  return ok;
}


// Recherche le fichier precedent au fichier de nom long namePlaying
//   dans le repertoire pointe par filePlaying
// Enregistre dans filePlaying le nom complet court de ce fichier
// Enregistre dans namePlaying le nom long de ce fichier
// Si le fichier actuel est le premier, conserve celui-ci

void previousFile()
{
  char name0[ MAX_FILENAME ];
  SdLongNameFile sdLn;
  sdLn.setHidePD( true );
  
  name0[0] = 0;
  boolean ok = sdLn.first( filePlaying, namePlaying - filePlaying );
 // if(ok)Serial<<">"<<sdLn.longName()<<endl;else Serial<<")-:"<<endl;
  if( ok )
    while( ok && ! fileFilter( sdLn ))
      ok = sdLn.next();
  while( ok && strcmp( sdLn.longName(), namePlaying ) != 0 )
  {
    strncpy( name0, sdLn.longName(), MAX_FILENAME );
    do
      ok = sdLn.next();
    while( ok && ! fileFilter( sdLn ));
  }
  if( name0[0] == 0 || ! ok )
    return;
  strncpy( namePlaying, name0, MAX_READBUF - ( namePlaying - filePlaying )); 
  return;
}  

/*******************************************************************************
**                                                                            **
**                       FUNCTIONS POUR LA LED DE STATUS                      **
**                                                                            **
*******************************************************************************/

// Soft reset. Make a jump to address 0

#ifdef __arm__
  void softReset()
  {
    const int RSTC_KEY = 0xA5;
    RSTC->RSTC_CR = RSTC_CR_KEY(RSTC_KEY) | RSTC_CR_PROCRST | RSTC_CR_PERRST;
  }
#else
  void(* softReset) (void) = 0;   // declare reset function @ address 0
#endif

// Utilise la LED de status pour indiquer le code d'erreur
// Si 'sr' vaut TRUE, répète 10 fois le code d'erreur, puis fait un reset
//  sinon, répète indéfiniment le code d'erreur

void errMsgLed( uint8_t errNbr, boolean sr )
{
  if( sr )
  {
    for( uint8_t i = 0; i < 10; i++ )
      okMsgLed( errNbr );
    softReset();
  }
  else
    while( true )
      okMsgLed( errNbr );
}

// Fait clignoter 'code' fois la LED de status

void okMsgLed( uint8_t code )
{
  delay( 1000 );
  for( uint8_t i = 0; i < code; i ++ )
  {
    onStatusLed();
    delay( 150 );
    offStatusLed();
    delay( 300 );
  }
  delay( 200 );
}

// Allumme la LED de status

void onStatusLed()
{
  digitalWrite( STAT_LED_PIN, HIGH );
}

// Eteind la LED de status

void offStatusLed()
{
  digitalWrite( STAT_LED_PIN, LOW );
}

