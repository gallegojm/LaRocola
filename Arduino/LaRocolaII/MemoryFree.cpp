#include "MemoryFree.h"

#ifdef __arm__
  // should use uinstd.h to define sbrk but Due causes a conflict
  extern "C" char* sbrk(int incr);
#else  // __arm__
  extern char *__brkval;
#endif  // __arm__

/*
  Amount of free RAM
  return the number of free bytes.
*/

int freeRam()
{
  char top;
  #ifdef __arm__
    return &top - reinterpret_cast<char*>( sbrk(0) );
  #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
  #else
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
  #endif  // __arm__
}

