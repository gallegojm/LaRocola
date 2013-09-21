#include "Radio.h"

Radio::Radio( SFEMP3Shield mp3 )
{
  this->mp3 = mp3;
  doStream = false;
}

boolean Radio::startRadio( char * name, char * server, char * url, uint16_t port )
{
  doStream = false;
  this->name   = name;
  this->server = server;
  this->url    = url;
  this->port   = port;

  int8_t rc = connectRadio();
  if( rc == 1 )   
    rc = connectRadio();
  if( rc == -1 )
    return false;

  // start playback
  mp3.playStream();
  
  // initiate variables
  refill = true;
  doStream = true;
  m_emptyBuf = 0;
  b_read = 0;
  fase = 0;
  titleBuffer[0] = 0;

  // set pointers
  p_read = 0;
  p_write = fillBuf( 0 );
  
  #ifdef DEBUG_RADIO
    min_filled = MP3BUF_SIZE;
    t_read = 0;
    t_micros = 0;
  #endif DEBUG_RADIO 
  max_filled = 0;
  t_fill = 0;
  
  return true;
}
 
/*
  Se connecte a une radio
  Retour:
    -1 : erreur
     0 : ok, connecte
     1 : redirection
*/
  
int8_t Radio::connectRadio()
{
  int8_t rc = -1;
  
  if( connected() )
    stopRadio();

  #ifdef DEBUG_RADIO
    Serial << "Connecting to " << server << ":" << port << endl;
  #endif DEBUG_RADIO
  if( connect( server, port ) != 1 )
    return rc;
  
  #ifdef DEBUG_RADIO
    Serial << "GET " <<  url << " HTTP/1.1\r\n";
    Serial << "Host: " <<  server << ":" <<  port << "\r\n";
  #endif DEBUG_RADIO
  *this << "GET " <<  url << " HTTP/1.1\r\n";
  *this << "Host: " <<  server << ":" <<  port << "\r\n";
  *this << "Range: bytes=0-\r\n";
  *this << "Accept: audio/" << "*; q=0.2, audio/basic\r\n";
  *this << "Icy-MetaData: 1\r\n";
  *this << "Connection: close\r\n\r\n";
  char readLine[CHUNK_SIZE];
  uint16_t iRL = 0;
  metaint = 0;
  uint32_t millis0 = millis();
  // Attend la reponse pendant 3 secondes
  while( (uint32_t) ( millis() - millis0 ) < 3000 )
    if( available())
    {
      char c = read();
      #ifdef DEBUG_RADIO
        Serial << c ;
      #endif DEBUG_RADIO
      if( c != '\r' )
        if( c != '\n' )
        {
          if( iRL < CHUNK_SIZE )
            readLine[ iRL ++ ] = c;
        }
        else
        {
          // ligne vide? Fin de la reponse du serveur
          if( iRL == 0 )
          {
            rc = 0;
            break;
          }
          readLine[ iRL ] = 0;
          if( ! strncmp( readLine, "icy-metaint:", 12 ))
          {
            char * r1 = readLine + 12;
            if( r1[0] == ' ' )
              r1 ++;
            metaint = atoi( r1 );
            #ifdef DEBUG_RADIO
              Serial << "icy-metaint = " << metaint << endl;
            #endif DEBUG_RADIO
          }
          else if( ! strncmp( readLine, "Location:", 9 ))
          {
            char * r1 = readLine + 9;
            if( r1[0] == ' ' )
              r1 ++;
            if( ! strncmp( r1, "http://", 7 ))
              r1 += 7;
            char * r2 = strchr( r1, '/' );
            char * r3 = strchr( r1, ':' );
            // le port est indique?
            // if( r3 != 0 && r3 < r2 )
            //   r2 = r3;
            if( r2 != 0 )
            {
              strcpy( url, r2 );
              r2[0] = 0;
              strcpy( server, r1 );
              rc = 1;
              #ifdef DEBUG_RADIO
                Serial << "Location: " << server << " " << url << endl;
              #endif DEBUG_RADIO
            }
            break;
          }
          readLine[0] = 0;
          iRL = 0;
        }
    }
  if( rc != 0 )
  {
    flush();
    stop();
  }
  return rc;
}


void Radio::stopRadio()
{
  mp3.stopStream();
  //flush();
  stop(); 
  doStream = false;
  #ifdef DEBUG_RADIO
    Serial << "Stop playing " <<  name << endl << endl;
  #endif DEBUG_RADIO
}


// code to determine number of bytes actually in buffer
#define FILLED ( p_write - p_read >= 0 ? p_write - p_read : MP3BUF_SIZE - p_read + p_write )

boolean Radio::streamRadio()
{
  if( doStream )
  {
    // feed the VS10xx chip
    tnb_fill = 0;
    #ifdef DEBUG_RADIO
      micros0 = micros();
    #endif DEBUG_RADIO
    do
      // refilling th buffer or less than 32 bytes in buffer?
      if( refill  ||  FILLED < 32 )
      // yes. Start time counter m_emptyBuf if not yet started to know time
      //  ellapsed with empty buffer
      {
        nb_fill = 0;
        if( m_emptyBuf == 0 )
        {
          m_emptyBuf = millis();
          refill = true;
        }
      }
      // no. Fill mp3 buffer if needed
      // Variable tnb_fill store number of bytes sent to MP3 chip in this loop
      else 
      {
        m_emptyBuf = 0;
        nb_fill = mp3.fillFromStream( mp3Buffer + p_read );
        if( nb_fill != 0 )
        {
          #ifdef DEBUG_RADIO
            // Serial << " > " << p_read << " " << nb_fill << " " << p_read + nb_fill - 1 << " > ";
          #endif DEBUG_RADIO
          tnb_fill += nb_fill;
          p_read += nb_fill;
          if( p_read >= MP3BUF_SIZE )
            p_read = 0;
        }
      }
    while( nb_fill > 0 );
    
    // feed the buffer
    nb_read = 0;
    if( FILLED < MP3BUF_SIZE - CHUNK_SIZE )
    {
      nb_read = fillBuf( p_write );
      if( nb_read != 0 )
      {
        // Serial << " < " << p_write << " " << nb_read << " " << p_write + nb_read - 1 << " <";
        p_write += nb_read;
        if( p_write >= MP3BUF_SIZE )
          p_write = 0;
      }
      // buffer enough filled to begin playing?
      if( refill  &&  ( FILLED >= MP3BUF_REFILL ) )  // yes
        refill = false;
    }

    // calculate statistics
    filled = FILLED;
    #ifdef DEBUG_RADIO
      if( tnb_fill != 0  ||  nb_read != 0 )
        t_micros += micros() - micros0;
      if( filled < min_filled )
        min_filled = filled;
      t_read += nb_read;
    #endif DEBUG_RADIO
    if( filled > max_filled )
      max_filled = filled;
    t_fill += tnb_fill;
  
    // stop streaming if buffer empty for more than 20 seconds  
    doStream = m_emptyBuf == 0  || (uint32_t) ( millis() - m_emptyBuf ) < 20000;
  }
  if( ! doStream )
    stopRadio();
  return doStream;
}


void Radio::statistic( char * stat1, uint32_t * p_porcfill, uint32_t * p_fill )
{
  strcpy( stat1, name );
  if( strlen( titleBuffer ) > 0 )
    strcat( stat1, titleBuffer );
  * p_porcfill = ( 100 * max_filled ) >> 16;
  * p_fill = t_fill;
  //sprintf( stat2, "%d%% %d kbps ", ( 100 * max_filled ) >> 16, t_fill /125 );
  #ifdef DEBUG_RADIO
    Serial << min_filled << "  " << max_filled << "  " << t_read << "  " << t_fill
           << "  " << t_micros << endl;
  #endif DEBUG_RADIO
  
  // reset values
  #ifdef DEBUG_RADIO
    min_filled = MP3BUF_SIZE;
    t_read = 0;
    t_micros = 0;
  #endif DEBUG_RADIO
  max_filled = 0;
  t_fill = 0;
}


/*
  Input parameter p point to memory where data bytes must be written
  Output is number of written bytes 
*/

int32_t Radio::fillBuf( int32_t p )
{
  int32_t mr, nr, nrm;
  
  // nothing to read, nothing to do
  if( ! available() )
    return 0;
  
  // calcule nombre maximum d'octets a lire
  mr = min( CHUNK_SIZE, MP3BUF_SIZE - p );
  if( metaint == 0 )
    return read( (uint8_t *) mp3Buffer + p, mr );
    
  nr = 0;
  switch( fase )
  {
    case 0:  // read mp3 data
      mr = min( mr, metaint - b_read );
      nr = read( (uint8_t *) mp3Buffer + p, mr );
      b_read += nr;
      if( b_read >= metaint )
        fase = 1;
      break;
    case 1:  // read length of metadata
      metalen = read() << 4; // * 16 
      b_read = 0;
      if( metalen > 0 )
        fase = 2;
      else
        fase = 0;
      break;
    case 2:  // read metadata
      nrm = read( (uint8_t *) metadataBuffer + b_read, metalen - b_read ); 
      b_read += nrm;
      if( b_read >= metalen )
      {
        metadataBuffer[ b_read ] = 0;
        metadataTitle();
        b_read = 0;
        fase = 0;
      }
  }
  return nr;
}


void Radio::metadataTitle()
{
  char * st = "StreamTitle='";
  char * pst = strstr( metadataBuffer, st );
  if( pst == 0 )
    return;
  pst += strlen( st );
  char * pet = strstr( pst, "';" );
  if( pet <= pst )
    return;
  pet[0] = 0;
  titleBuffer[0] = 0;
  pet = strstr( pst, " - " );
  if( pet - pst > TITLE_SIZE )
    return;
  char * pet0 = strstr( pet + 3, " - " );
  if( pet0 - pst > TITLE_SIZE )
    pet[0] = 0;
  else
    pet0[0] = 0;
  if( strlen( pst ) > TITLE_SIZE )
    return;
  while( pst[0] == ' ' || pst[0] == '-' )
    pst ++;
  if( strlen( pst ) == 0 )
    return;
  strcat( titleBuffer, " - " );
  strcat( titleBuffer, pst );
  #ifdef DEBUG_RADIO
    Serial << "[" << titleBuffer << "]" << endl; 
  #endif DEBUG_RADIO
}


