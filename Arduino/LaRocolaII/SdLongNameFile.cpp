#include "SdLongNameFile.h"

// Search for the first file in a directory given by is base class
//
// parameters:
//   SdBaseFile * dirPath : the directory we want to scan
//
// return:
//   true if found, false if not

boolean SdLongNameFile::first( SdBaseFile * dirPath )
{
  p_sdbf = dirPath;
  p_sdbf->rewind();
  return next();
}

// Search for the first file in a directory given by his long name
//
// parameters:
//   char * lnPath : the long name of directory we want to scan
//
// return:
//   true if found, false if not

boolean SdLongNameFile::first( char * lnPath, size_t lenLnPath )
{
  char shortPath[MAX_SHORTPATH];
  if( ! long2shortPath( shortPath, lnPath, MAX_SHORTPATH, lenLnPath ))
    return false;
  sd.chdir( shortPath );
  p_sdbf = sd.vwd();
  p_sdbf->rewind();
  return next();
}

boolean SdLongNameFile::next()
{
  uint8_t offset[] = {1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30};
  uint8_t lfnIn = 130;
  uint8_t i;
  uint8_t ndir;
  uint8_t sum;
  uint8_t test;
  bool haveLong = false;
  
  while( p_sdbf->read( &dir_ln, 32 ) == 32 )
  {
    if( DIR_IS_LONG_NAME( &dir_ln ) )
    {
      if( ! haveLong )
      {
        if(( dir_ln.name[0] & 0XE0 ) != 0X40 )
          continue;
        ndir = dir_ln.name[0] & 0X1F;
        test = dir_ln.creationTimeTenths;
        haveLong = true;
        lfnIn = 130;
        long_name[ lfnIn ] = 0;
      }
      else if( dir_ln.name[0] != --ndir || test != dir_ln.creationTimeTenths )
      {
        haveLong = false;
        continue;
      }
      char *p = (char*) & dir_ln;
      if( lfnIn > 0 )
      {
        lfnIn -= 13;
        for( i = 0; i < 13; i++ )
          long_name[ lfnIn + i ] = p[ offset[ i ]];
      }
    }
    else if( DIR_IS_FILE_OR_SUBDIR( &dir_ln ) 
             && dir_ln.name[0] != DIR_NAME_DELETED 
             && dir_ln.name[0] != DIR_NAME_FREE
             && ( ! hideP || dir_ln.name[0] != 0x2E ))
    {
      if( haveLong )
      {
        for( sum = i = 0; i < 11; i++ )
           sum = (((sum & 1) << 7) | ((sum & 0xfe) >> 1)) + dir_ln.name[i];
        if( sum != test || ndir != 1 )
        haveLong = false;
      }
      if( haveLong )
      {
        for( i = 0; lfnIn + i <= 130 ; i++ )
          long_name[i] = long_name[lfnIn + i];
        return true;
      }
      // else if( dir.reservedNT )
      //  return "Reserved NT";
      else
      {
        SdFile::dirName( dir_ln, long_name );
        return true;  
      }
    }
    else
    {
      if( dir_ln.name[0] == DIR_NAME_FREE )
        break;
      haveLong = false;
    }
  }
  long_name[ 0 ] = 0;
  return false;
}

// Get the long name of the file
//
// return:
//   pointer to the string that contain the long name

char * SdLongNameFile::longName()
{
  return long_name;
}

// Search for a file given by his long name
//   in a directory given by is short name
//
// parameters:
//   SdBaseFile * dirPath : the directory (short name) we want to scan
//   char * lnFile : the long name of the file
//   size_t lLnFile : the length of the long name file. If 0 (default),
//                    calculated with strlen( lnFile )
//
// return:
//   true if found, false if not

boolean SdLongNameFile::search( SdBaseFile * dirPath,
                                char * lnFile, size_t lLnFile )
{
  if( lLnFile == 0 )
    lLnFile = strlen( lnFile );
  boolean ok = first( dirPath );
  while( ok )
  {
    if( strncmp( long_name, lnFile, lLnFile ) == 0 )
      return true;
    ok = next();
  }
  return false;
}

boolean SdLongNameFile::search( char * lfnFile )
{
  char * psep = strrchr( lfnFile, '/' );
  // if lfnFile ends with '/', this is a path without file name,
  //   so point to first file
  if( psep == strrchr( lfnFile, 0 ) - 1 ) 
    return first( lfnFile );
  boolean ok = first( lfnFile, psep - lfnFile + 1 );
  while( ok )
  {
    if( strcmp( long_name, psep + 1 ) == 0 )
      return true;
    ok = next();
  }
  return false;
}

// Convert a 'long name' path to a 'short name' path
//
// parameters:
//   char * snPath (out) : where to store the 'short name' path
//   char * lnPath : the 'long name' path we want to convert
//                   Must be absolute path (begin with /)
//   size_t lSnPath : the length of snPath string
//   size_t lLnPath : the length of the long name path. If 0 (default),
//                    calculated with strlen( lnPath )
//
// return:
//   true, if convertion is done

boolean long2shortPath( char * snPath, char * lnPath,
                        size_t lSnPath, size_t lLnPath )
{
  SdLongNameFile sdlnf;
  char * dir0 = lnPath + 1;
  char * dir1;
  char * dir2 = lnPath + ( lLnPath == 0 ? strlen( lnPath ) : lLnPath );
  char * snPathEnd;
  
  strcpy( snPath, "/" );
  while( dir0 < dir2 ) // strlen( dir0 ) > 0 )
  {
    dir1 = strchr( dir0, '/' );
    if( dir1 == 0 || dir1 > dir2 )
    {
      dir1 = dir2; // strrchr( dir0, 0 );
      if( dir1 == dir0 )
        return true;
    }
    if( lSnPath <= strlen( snPath ) + 8 ) // not enough space in snPath
      return false;
    snPathEnd = strrchr( snPath, 0 ); // point to the end of snPath
    sd.chdir( snPath );
    if( ! sdlnf.search( sd.vwd(), dir0, dir1 - dir0 ))
      return false;
    SdFile::dirName( sdlnf.dir(), snPathEnd ); // copy short name to snPathEnd
    if( dir1 >= dir2 || dir1[0] != '/' )
      return true;
    strcat( snPath, "/" );
    dir0 = dir1 + 1;
   }
  return true;
}


