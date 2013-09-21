
/*******************************************************************************
**                                                                            **
**                       LA ROCOLA pour PC ou Android                         **
**                                                                            **
**            une télécommandable depuis smartphone Android ou PC             **
**             pour La Rocola à base de Arduino Ethernet et rMP3              **
**                                                                            **
**      Copyright (c) 2012-2013 Jean-Michel Gallego. All rights reserved.     **
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

import hypermedia.net.*;

boolean debug = false; //true;

UDP udp;  

String version = "LaTelecommande V-2.00";
//String version = "LaTelecommande V-13.09.20";
String statusLine[] = { "", "" };
int etat     = 0;
int temps    = 0;
int depth    = 0;
int maxItm;
int x0;
int y2;
int apWidth;
int apWidth8;
int apWidth32;
int apHeight;
int imgHeight;
int nbCarreaux;
int nbCarreaux2;
int nbCarreaux4;
int carreau;
int carreau4;
int carreau8;
int sup1Cmd;
int sup2Cmd;
int infCmd;

String commandes = "b<ps>nxi m-+";    // commandes reconnues par La Rocola-Arduino
String[] listLine = new String[ 15 ]; // valeur pour écrans HD
String moreLines = ". . .";
PImage img;
PFont font;

String remoteIp;                      // adresse IP de LaRocola
int remotePort;                       // port sur lequel LaRocola écoute
int localPort;                        // port sur lequel la télécommande écoute

/*******************************************************************************
**                                                                            **
**                               INITIALISATION                               **
**                                                                            **
*******************************************************************************/

void setup()
{
  orientation( PORTRAIT );  // seulement pour Android mais ne gêne pas la compilation pour l'appli windows
  PImage titlebaricon;
  titlebaricon = loadImage( "LaRocolaIcon.gif" );
  // la ligne suivante doit être mise en commentaire pour Android, sinon on obtient une erreur de compilation
  frame.setIconImage( titlebaricon.getImage()); // seulement pour créer une application windows
  
  // Pour créer une application windows, choisir une taille
  // Pour Android, la taille est obtenue automatiquement suivant le modèle émulé
  //  et les 4 lignes suivantes doivent être mises en commentaire
  // size( 320, 480 );     // seulement pour tester HVGA (320 x 480 like Samsung Ace Plus)
  // size( 360, 480 );     // seulement pour tester QVGA (240 x 320)
  size( 360, 600 );     // seulement pour windows ou pour tester WVGA (480 x 800)
  // size( 360, 640 );     // seulement pour tester HD ou qHD (540 x 960 ou 720 x 1280)

  // calcule les paramètres de visualisation en fonction de la taille de l'écran
  //  et du rapport largeur / hauteur
  apWidth  = width;   // 320 pour HVGA (320 x 480)
  apHeight = height;  // 480 pour HVGA
  apWidth8 = apWidth / 8;
  apWidth32 = apWidth / 32;
  if( apWidth * 3 >= height * 2 )    // largeur/hauteur = 3/4 ou 2/3
  {
    nbCarreaux = 9;
    y2 = 0;
  }
  else
  { 
    if( apWidth * 5 == height * 3 )  // largeur/hauteur = 3/5
      nbCarreaux = 10;
    else                             // largeur/hauteur = 9/16
      nbCarreaux = 11;
    imgHeight = apHeight * 9 / nbCarreaux;
    y2 = apHeight - imgHeight;
  }
  maxItm = 2 * nbCarreaux - 7;      // nombre maximum d' items: 2 items par carreaux - 3 carreaux et demi
  nbCarreaux2 = 2 * nbCarreaux;
  nbCarreaux4 = 4 * nbCarreaux;
  carreau  = apWidth / 6;           // taille d'un carreau en pixel
  carreau4 = carreau / 4;
  carreau8 = carreau / 8;
  sup1Cmd  = apHeight * ( nbCarreaux - 3 ) / nbCarreaux;  // valeur superieur de la 1ere ligne de commande
  sup2Cmd  = apHeight * ( nbCarreaux - 2 ) / nbCarreaux;  // valeur superieur de la 2nde ligne de commande
  infCmd   = apHeight * ( nbCarreaux - 1 ) / nbCarreaux;  // valeur inférieur de la 2nde ligne de commande

  // Met à zéro le défilement des titres longs      
  x0 = apWidth8;

  // la taille de la font dépend aussi de la taille de l'écran
  font = createFont( "Segoe Print Bold.ttf", apWidth / 20 );  // 16 pour HVGA (320 x 480)
  textFont( font );
  
  // initialise la liste des fichiers ou répertoires
  for( int i = 0; i < maxItm; i++ )
    listLine[i] = "";

  frameRate( 30 );          // est-ce une bonne valeur?
  
  img = loadImage( "LaRocola320x480.jpg" );
  
  // récupère les paramètres réseau de LaRocola
  String lines[] = loadStrings( "LaRocola.dat" );
  remoteIp = lines[0];
  remotePort = int( lines[1] );
  localPort = int( lines[2] );
  
  // crée une connection sur le port 'localPort' et écoute les éventuels messages
  // create a new datagram connection on port 'localPort' and wait for incoming message
  udp = new UDP( this, localPort );
  udp.listen( true ); 
  
  //sollicite la version de LaRocola
  sendCmd( 'v' );
  
  // affiche la version de LaTelecommande
  listLine[maxItm-1] = version;
  
  //sollicite la liste des répertoires à la racine de la carte mémoire du module rMP3
  // sendCmdList( "/" );
}

/*******************************************************************************
**                                                                            **
**                           BOUCLE DE VISUALISATION                          **
**                                                                            **
*******************************************************************************/

void draw()
{
  // affiche le fond d'ecran
  if( y2 == 0 )
    image( img, 0, 0, apWidth, apHeight );
  else
  {
    image( img, 0, 0, apWidth, imgHeight );
    image( img, 0, y2, apWidth, imgHeight );
  }

  // affiche les lignes de status
  fill( 120, 20, 170 );
  if( textWidth( statusLine[0] ) < apWidth - apWidth32 )
    text( statusLine[0], apWidth32, ( nbCarreaux4 - 3 ) * apHeight / nbCarreaux4 );
  else
  {
    // Défilement des titres longs      
    text( statusLine[0], x0, ( nbCarreaux4 - 3 ) * apHeight / nbCarreaux4 );
    if( ( apWidth - ( -- x0 )) > textWidth( statusLine[0] ) + apWidth8 )
      x0 = apWidth8;
  }
  text( statusLine[1], apWidth32, ( nbCarreaux4 - 1 ) * apHeight / nbCarreaux4 );

  // affiche la ligne "répertoire racine"
  fill( 180, 50, 50 );
  text( "/", apWidth32, 2 + apHeight / ( nbCarreaux4 - 1 ) );

  // affiche la liste de fichiers ou répertoires 
  for( int i = 0; i < maxItm; i++ )
  {
    if( isMp3( listLine[i] ))
    {
      fill( 30, 150, 90 );
      text( clean( listLine[i] ), apWidth32 * ( 1 + depth ), 2 + ( 3 + 2 * i ) * apHeight / nbCarreaux4 );
    }
    else if( isRad( listLine[i] ))
    {
      fill( 112, 112, 255 );
      text( clean( listLine[i] ), apWidth32 * ( 1 + depth ), 2 + ( 3 + 2 * i ) * apHeight / nbCarreaux4 );
    }
    else
    {
      fill( 180, 50, 50 );
      text( cleanNum( listLine[i] ), apWidth32 * ( 1 +  min( i, depth )), 2 + ( 3 + 2 * i ) * apHeight / nbCarreaux4 );
    }
  }

  // peind un rectangle ou un cercle clair sur la zone cliquée
  if( mousePressed )
  {
    fill( 255, 255, 255, 128 );
    noStroke();
    if( mouseY < sup1Cmd )
    {
      int y = mouseY * nbCarreaux2 / apHeight;
      if( mouseY - carreau8 - ( y * apHeight / nbCarreaux2 ) < apHeight / nbCarreaux4 )
      {
        rectMode( CORNER );
        rect( carreau4, y * apHeight / nbCarreaux2 - carreau4 / 3, apWidth - 2 * carreau4, apHeight / nbCarreaux2,
              carreau4, carreau4, carreau4, carreau4 );
      }
    }
    else if( mouseY < infCmd )
    {
      int x = mouseX * 6 / apWidth;
      int y = mouseY * nbCarreaux / apHeight;
      ellipseMode( CORNER );
      ellipse( x * apWidth / 6, y * apHeight / nbCarreaux, carreau, carreau );
    }
  }
  
  if( etat == 0 )
  {
    etat = 1;
    temps = millis() + 3000;
    // print( etat ); println( temps );
  }
  else if( etat == 1 && temps < millis() )
  {
    etat = 2;
    //sollicite la liste des répertoires à la racine de la carte mémoire du module rMP3
    sendCmdList( "/" );
  }
  
  // Pour créer des images lorsque draw() est appelé
  // saveFrame( "output.png" );
}

/*******************************************************************************
**                                                                            **
**                          FONCTIONS D'INTEREACTION                          **
**                                                                            **
*******************************************************************************/

// Interaction avec un clic de souris ou en tapotant l'écran du smartphone

//void mouseClicked()
void mouseReleased()
{
  if( mouseY < sup1Cmd )                       // clic sur la liste de fichiers ou répertoires
  {
    int y = mouseY * nbCarreaux2 / apHeight;
    if( mouseY - carreau8 - ( y * apHeight / nbCarreaux2 ) < apHeight / nbCarreaux4 )
    {
      y -- ;
      if( y == -1 )                            // clic sur la première ligne: répertoire racine
      {
        depth = 0;
        for( int i = 0; i < maxItm; i++ )
          listLine[i] = "";
        sendCmdList( "/" );
      }
      else if( y < maxItm && listLine[y].length() > 0 && etat == 2 ) // lignes suivantes
      {
        String path = "";
        if( isMp3( listLine[y] ) || isRad( listLine[y] ) ) // Fichier reconnu ?
        {
          for( int i = 0; i < depth; i++ )
            path = path + "/" + listLine[i]; 
          path = path + "/" + listLine[y];
          sendCmdPlay( path );
        }
        else                                // Sous-répertoire
        {
          if( y < depth )
            depth = y;
          for( int i = 0; i < depth; i++ )
            path = path + "/" + listLine[i]; 
          if( listLine[y].equals( moreLines ))
            path = path + "/" + listLine[y-1];
          else
          {
            path = path + "/" + listLine[y] + "/";
            listLine[depth] = listLine[y];
            depth ++; 
          }
          for( int i = depth; i < maxItm; i++ )
            listLine[i] = "";
          sendCmdList( path );
        }
      }
    }
  }
  else
  {
     int x = mouseX * 6 / apWidth;
     if( mouseY < sup2Cmd )                    // clic sur la 1ère ligne de commandes
       sendCmd( commandes.charAt( x ));
     else if( mouseY < infCmd )                 // clic sur la 2nde ligne de commandes          
     {
       if( x == 0 )                             // commande 'fin du programme'
         exit();
       else if( x == 2 )                        // rien pour le moment...
         ;
       else                                     // commandes de volume
         sendCmd( commandes.charAt( x + 6 ));
     }
  }
}

// Interaction avec la réception d'un paquet UDP

void receive( byte[] data )
{
  if( debug )
  {
    for( int i = 0; i < data.length; i++ ) 
      print( char( data[ i ]));  
    println();
  }
  // Extrait le type de réponse dans 'c'
  char c = (char) data[ 0 ];
  // Extrait le paramètre (normalement un chiffre) dans n
  int n = data[ 1 ] - '0';
  
  // Réponse à une demande d'information (commande 'i')
  // n vaut 1 ou 2 : deux lignes d'informations 
  if( c == 'i' )
    if( n == 1 )
    {
      String s = new String( subset( data, 2 ));
      if( ! statusLine[ 0 ].equals( s ))
      {
        statusLine[ 0 ] = s;
        // Remet à zéro le défilement des titres longs
        //   seulement si la ligne est différente de l'antérieure      
        x0 = apWidth32;
      }
    }
    else if( n == 2 )
      statusLine[ 1 ] = new String( subset( data, 2 ));
      
  // Réponse à une demande de liste de fichiers (commande 'l' comme 'elle', pas 'un')
  // n vaut entre 0 et maxItm (défini en début de programme) 
  if( c == 'l' )
    if( n >= 0 )
      if( n + depth < maxItm )
        listLine[ n + depth ] = new String( subset( data, 2 ));
      else
        listLine[ maxItm - 1 ] = moreLines;
}

/*******************************************************************************
**                                                                            **
**                       FONCTIONS DE TRANSMISSION UDP                        **
**                                                                            **
*******************************************************************************/

// Envoie une commande simple (sans argument) sous le format '*x*' où x est la commande

void sendCmd( char cmd )
{
  char cs[] = { '*', cmd, '*', 0 };
  udpSend( new String( cs ));
}

// Envoie la commande 'exécute un morceau'
// Le format est '*f*/cccc/nnnn.mp3'
//   où /cccc/nnnn.mp3 est le chemin complet du fichier dans la carte mémoire du module rMP3

void sendCmdPlay( String dir )
{
  // String s = "*f*" + dir;
  udpSend( new String( "*f*" + dir ));
}

// Envoie la commande 'liste de fichiers ou répertoires'
// Le format est '*l*n' où n est le nombre de fichiers solicité
// Ce nombre dépend de la taille de l'écran représentée par maxItm et la profondeur du chemin
// On demande une ligne de plus que nécessaire pour savoir s'il faut afficher la ligne
//   'moreLignes' indiquant que la liste ne tient pas ter 'moreLignes'

void sendCmdList( String dir )
{
  // maxItm - depth suffit à remplir la liste.
  // mais on demande une ligne de plus pour savoir s'il y en a d'autres
  char cs[] = { '*', 'l', '*', (char) ( '0' + maxItm - depth + 1 ) };
  udpSend( new String( cs ) + dir );
}

// Envoie la commande s

void udpSend( String s )
{
  if( debug )
  {
    print( "Sending: " );
    println( s );
  }
  udp.send( s, remoteIp, remotePort );
}

/*******************************************************************************
**                                                                            **
**                      FONCTIONS SUR LES NOMS DE FICHIERS                    **
**                                                                            **
*******************************************************************************/

// Renvoie vrai si le fichier est audio (.aac, .flac, .mp3, .ogg, .wma, .wav) 

boolean isMp3( String file )
{
  int p = file.lastIndexOf( '.' );
  if( p > 0 )
  {
    String ext = file.substring( p + 1 );
    if( ext.equalsIgnoreCase( "aac" ) ||
        ext.equalsIgnoreCase( "mp3" ) ||
        ext.equalsIgnoreCase( "ogg" ) ||
        ext.equalsIgnoreCase( "wav" ) ||
        ext.equalsIgnoreCase( "wma" ) ||
        ext.equalsIgnoreCase( "fla" ) ||
        ext.equalsIgnoreCase( "flac" ))
      return true;
  }
  return false;
}

// Renvoie vrai si le fichier a pour extension .rad 

boolean isRad( String file )
{
  int p = file.lastIndexOf( '.' );
  if( p > 0 )
  {
    String ext = file.substring( p + 1 );
    if( ext.equalsIgnoreCase( "rad" ))
      return true;
  }
  return false;
}

// Renvoie le nom du fichier sans l'extension ni les chiffres, espaces et signes moins (-) du début du nom

String clean( String file )
{
  String name = file.substring( 0, file.lastIndexOf( '.' ));
  name = cleanNum( name );
  return name;
}

// Retire "01 - " ou "01-" du début du nom
// Ceci permet d'ordonner les fichiers

String cleanNum( String name )
{
  int i = 0;
  char c;
  while( name.length() > i )
  {
    c = name.charAt( i );
    if( c >= '0' && c <= '9' )
      i ++;
    else
      break;
  }
  while( i > 0 && name.length() > i )
  {
    c = name.charAt( i ++ );
    if( c == '-' )
      break;
    else if( c != ' ' )
      i = -1;
  }
  while( i > 1 && name.length() > i )
  {
    if( name.charAt( i ) != ' ' )
      break;
    i ++;
  }
  if( i > 0 )
    name = name.substring( i );
  return name;
}

