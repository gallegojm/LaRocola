#include "SdStream.h"

uint16_t SdStream::getInt()
{
  getline( readBuf, sizeReadBuf );
  char * pb = readBuf;
  return readInt( &pb );
}

IPAddress SdStream::getIp()
{
  IPAddress ipa;
  
  getline( readBuf, sizeReadBuf );
  char * pb = readBuf;
  for( uint8_t i = 0; i < 4; i ++ )
    ipa[i] = readInt( &pb );
  return ipa;
}

MacAddress SdStream::getMac()
{
  MacAddress maca;
  
  getline( readBuf, sizeReadBuf );
  char * pb = readBuf;
  for( uint8_t i = 0; i < 6; i ++ )
    maca[i] = readHex( &pb );
  return maca;
}

uint16_t SdStream::readInt( char ** ps )
{
  uint16_t i = 0;
  char c;
  // saute les caractères qui ne sont pas des chiffres 
  do
    c = *(( *ps )++ );
  while( c >= 0 && ! isdigit( c ));
  while( isdigit( c ))
  {
    i = 10 * i + c - '0';
    c = *(( *ps )++ );
  }
  return i;
}

uint8_t SdStream::readHex( char ** ps )
{
  uint8_t i = 0;
  char c;
  // saute les caractères qui ne sont pas des chiffres hexadécimaux
  do
    c = *(( *ps )++ );
  while( c >= 0 && ! isxdigit( c ));
  while( isxdigit( c ))
  {
    i = 16 * i + c;
    if( isdigit( c ))
      i -= '0';
    else
    {
      i += 10;
      if( c < 'F' )
        i -= 'A';
      else
        i -= 'a';
    }
    c = *(( *ps )++ );
  }
  return i;
}

