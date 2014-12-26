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

uint16
StringLength(uint8 *String)
{
  int Count = 0;

  while(*String++) {
    Count++;
  }

  return Count;
}

void
PrintBits(void *Value, size_t Size)
{
  uint8 *Ptr = (uint8 *)Value;

  uint8 Byte;
  int16 ByteIndex, BitIndex;

  for (ByteIndex = Size - 1; ByteIndex >= 0; ByteIndex--) {
    for (BitIndex = 7; BitIndex >= 0; BitIndex--)
    {
      Byte = Ptr[ByteIndex] & (1 << BitIndex);
      Byte >>= BitIndex;
      //printf("i: %d, j: %d, byte: %u\n", ByteIndex, BitIndex, Byte);
      printf("%u", Byte);
    }
  }
  printf("\n");
}

int main(int argc, char *argv[])
{
  uint8 String;
  String = 90;
  PrintBits(&String, sizeof(uint8));
  String = strtol("5a", 0, 16);
  PrintBits(&String, sizeof(uint8));

  printf("\n");
}
