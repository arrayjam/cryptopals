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
    //"49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d5a";
    "4d616e";

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

  // TODO(yuri): Implement padding
  uint8 Base64Padding = (ByteBufferSize % 3 != 0) ? (3 - (ByteBufferSize % 3)) : 0;
  uint8 PaddedByteBufferSize = ByteBufferSize + Base64Padding;
  uint8 Base64Length = PaddedByteBufferSize * 8 / 6;
  printf("Base64Length: %d, Base64Padding: %d\n",
         Base64Length, Base64Padding);

  uint8 *Base64Buffer = (uint8 *)malloc(sizeof(uint8) * Base64Length);
  if(!Base64Buffer) printf("Couldn't allocate Base64Buffer.\n");

  printf("ByteBufferSize: %d, Base64Remainder: %d\n",
         ByteBufferSize, Base64Padding);

  uint8 AsciiTriplet[3];
  uint8 Base64Sextet[4];
  uint8 Mask = 0xff;

  uint8 Base64LookupTable[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
  };

  for(int TripletIndex = 0;
      TripletIndex < PaddedByteBufferSize / 3;
      TripletIndex++)
  {
    for(int i = 0; i < ArrayCount(AsciiTriplet); i++)
    {
      // TODO(yuri): Does filling this out backward with 2-i give us the right
      // block of memory?
      AsciiTriplet[i] = ByteBuffer[TripletIndex*3 + i];
    }

    // TODO(yuri): There has to be a better way of doing this.
    Base64Sextet[0] = ((Mask << 2) & AsciiTriplet[0]) >> 2;
    PrintBits(&Base64Sextet[0], 1);

    Base64Sextet[1] =
      (((Mask >> 6) & AsciiTriplet[0]) << 4) |
      (((Mask << 4) & AsciiTriplet[1]) >> 4);
    PrintBits(&Base64Sextet[1], 1);

    Base64Sextet[2] =
      (((Mask >> 4) & AsciiTriplet[1]) << 2) |
      (((Mask << 6) & AsciiTriplet[2]) >> 6);
    PrintBits(&Base64Sextet[2], 1);

    Base64Sextet[3] = (Mask >> 2) & AsciiTriplet[2];
    PrintBits(&Base64Sextet[3], 1);


    for(int i = 0; i < ArrayCount(Base64Sextet); i++)
    {
      printf("%d: %c\n", Base64Sextet[i], Base64LookupTable[Base64Sextet[i]]);
    }

    //printf("%d, %d\n", TripletIndex, PaddedByteBufferSize / 3);
    printf("%c%c%c|\n", AsciiTriplet[0], AsciiTriplet[1], AsciiTriplet[2]);
  }


  free(ByteBuffer);
  free(Base64Buffer);
}
