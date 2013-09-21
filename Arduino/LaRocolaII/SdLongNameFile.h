#include <SdFat.h>

extern SdFat sd;

#define MAX_SHORTPATH 128 // Max length for short path names

boolean long2shortPath( char * snPath, char * lnPath,
                        size_t lSnPath, size_t lLnPath = 0 );

class SdLongNameFile : public SdBaseFile
{
 public:
  SdLongNameFile() { hideP = false; }

#if DESTRUCTOR_CLOSES_FILE
  ~SdLongNameFile() {}
#endif  // DESTRUCTOR_CLOSES_FILE

  boolean first( SdBaseFile * dirPath );
  boolean first( char * lnPath, size_t lenLnPath = 0 );
  boolean next();
  boolean search( SdBaseFile * dirPath, char * lnFile, size_t lLnFile = 0 );
  boolean search( char * lfnFile );
  char *  longName();
  dir_t   dir() { return dir_ln; }
  boolean isDir()
   { return ( dir_ln.attributes & DIR_ATT_FILE_TYPE_MASK ) == DIR_ATT_DIRECTORY ; }
  boolean isFile() { return DIR_IS_FILE( &dir_ln ); }
  uint32_t fileSize() { return dir_ln.fileSize; }
  void setHidePD( boolean hideP ) { this->hideP = hideP; }
  
 private:
  SdBaseFile * p_sdbf;
  dir_t dir_ln;
  char long_name[ 131 ];
  boolean hideP;
};

