#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

int
StringLength(char *String) {
  int Count = 0;

  while(*String++) {
    Count++;
  }

  return Count;
}

uint8 *
DecStringToArray(char *String) {
  uint8 * Result = (uint8 *)malloc(sizeof(uint8) * StringLength(String));
  if (!Result) return 0;

  for(int i = 0; i < StringLength(String); i++) {
    Result[i] = (int)String[i];
  }

  return Result;
}

int main(int argc, char *argv[]) {
  char * String = (char *)"yuri";

  printf("%s: %d\n", String, StringLength(String));
  uint8 * Array = DecStringToArray(String);

  if(Array) free(Array);
}
