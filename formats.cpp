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
StringLength(int8 *String)
{
  int Count = 0;

  while(*String++)
  {
    Count++;
  }

  return Count;
}

void
PrintBits(uint8 *Value, size_t Size)
{
  uint8 Byte;
  int16 ByteIndex, BitIndex;

  for(ByteIndex = Size - 1; ByteIndex >= 0; ByteIndex--)
  {
    for(BitIndex = 7; BitIndex >= 0; BitIndex--)
    {
      Byte = Value[ByteIndex] & (1 << BitIndex);
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

  int8 *HexString = (int8 *)
    "49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d5a";

  uint8 ByteBufferSize = StringLength(HexString) / 2;
  printf("HexString: %s\nStringLength(HexString): %d, ByteBufferSize: %d\n",
         HexString, StringLength(HexString), ByteBufferSize);

  int8 *ByteBuffer = (int8 *)malloc(sizeof(int8) * ByteBufferSize);
  if(!ByteBuffer) printf("Couldn't allocate ByteBuffer.\n");

  int8 FirstHexChar, SecondHexChar;
  int8 HexByteBuffer[3];
  for(int HexIndex = 0; HexIndex < ByteBufferSize; HexIndex++)
  {
    FirstHexChar = HexString[HexIndex*2];
    SecondHexChar = HexString[(HexIndex*2)+1];
    HexByteBuffer[0] = FirstHexChar;
    HexByteBuffer[1] = SecondHexChar;
    HexByteBuffer[2] = 0;
    int8 Byte = strtol((char *)HexByteBuffer, 0, 16);
    ByteBuffer[HexIndex] = Byte;
    //printf("%d\t%s\t%c\t%c\t%d\t%c\n",
           //HexIndex, HexByteBuffer, FirstHexChar, SecondHexChar,
           //Byte, Byte);
  }

  for(int i = 0; i < ByteBufferSize; i++)
  {
    printf("%c", ByteBuffer[i]);
  }
  printf("\n");

  uint8 One = 0xff;
  PrintBits(&One, 1);

  uint8 Base64Padding = 3 - (ByteBufferSize % 3);
  uint8 Base64Length = (ByteBufferSize + Base64Padding) * 8 / 6;
  printf("Base64Length: %d, Base64Padding: %d\n",
         Base64Length, Base64Padding);

  uint8 *Base64Buffer = (uint8 *)malloc(sizeof(uint8) * Base64Length);
  if(!Base64Buffer) printf("Couldn't allocate Base64Buffer.\n");

  //for(int TripletIndex = 0; TripletIndex <



  free(ByteBuffer);
  free(Base64Buffer);
}
