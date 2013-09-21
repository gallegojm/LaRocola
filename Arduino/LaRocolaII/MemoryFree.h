// memoryFree header

#ifndef	MEMORY_FREE_H
  #define MEMORY_FREE_H
  
#include <Arduino.h>
#ifdef __AVR__
  #include <avr/pgmspace.h>
#endif

int freeRam();

#endif // MEMORY_FREE_H


