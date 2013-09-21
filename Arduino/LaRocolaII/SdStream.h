#ifndef SD_STREAM_H
#define SD_STREAM_H

#include <SdStream.h>
#include <IPAddress.h>

class MacAddress
{
  public:
    MacAddress() { memset( b, 0, sizeof( b )); }
    MacAddress( uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6 )
      { b[0] = b1; b[1] = b2; b[2] = b3; b[3] = b4; b[4] = b5; b[5] = b6; }
    
    MacAddress& operator = ( const uint8_t * by )
      { memcpy( b, by, sizeof( b )); return *this; }
    uint8_t operator[](int index) const { return b[index]; };
    uint8_t& operator[](int index) { return b[index]; };
    byte b[6];
};

class SdStream : public ifstream
{
  public:
  
    SdStream() {}
    SdStream( const char* path, char* buf, uint16_t sbuf, openmode mode = in )
    {
      readBuf = buf;
      sizeReadBuf = sbuf;
      open( path, mode );
    }
  
    uint16_t   getInt();
    IPAddress  getIp();
    MacAddress getMac();
    
  private:
    uint16_t readInt( char ** ps );
    uint8_t readHex( char ** ps );
    
    char *   readBuf;
    uint16_t sizeReadBuf;
};

#endif // SD_STREAM_H

