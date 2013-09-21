#include <Arduino.h>
#include <Ethernet.h>
#include <Streaming.h>
#include <SFEMP3Shield.h> 

// for debugging
// #define DEBUG_RADIO

// size of buffer
#define MP3BUF_SIZE 65536 // must be a multiple of 32
// bytes in buffer to begin the reproduction (50 to 100% of MP3BUF_SIZE)
#define MP3BUF_REFILL (65536/8)*7 
#define METABUF_SIZE 4096
#define TITLE_SIZE 100
// max number of bytes read from internet stream
#define CHUNK_SIZE 1024


class Radio : 
public EthernetClient
{
public:
  Radio( SFEMP3Shield mp3 );
  boolean startRadio( char * name, char * server, char * url, uint16_t port );
  void    stopRadio();
  boolean streamRadio();
  void  statistic( char * stat1, uint32_t * p_porcfill, uint32_t * p_fill );

private:
  int8_t connectRadio();
  int32_t fillBuf( int32_t p );
  void metadataTitle();

  char *   name;
  char *   server;
  char *   url;
  uint16_t port;

  SFEMP3Shield mp3;

  uint8_t mp3Buffer[ MP3BUF_SIZE ];
  char metadataBuffer[ METABUF_SIZE ];
  char titleBuffer[ TITLE_SIZE ];
  boolean refill, doStream;
  int32_t p_read, p_write, nb_fill, nb_read;
  int32_t m_emptyBuf;
  uint8_t n_tryConnect;
  uint8_t fase; 
  uint16_t metaint, metalen;
  int32_t b_read;
  int32_t t_fill, tnb_fill;
  int32_t filled, max_filled;
#ifdef DEBUG_RADIO
  int32_t t_read;
  int32_t min_filled;
  uint32_t micros0, t_micros;
#endif DEBUG_RADIO
};


