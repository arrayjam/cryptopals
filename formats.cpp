#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;


typedef float real32;
typedef double real64;

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

typedef uint16 flag;

// NOTE(yuri): Print Types
#define BYTE_BUFFER   (1 << 1)
#define HEX_STRING    (1 << 2)

// NOTE(yuri): Print Options
#define AS_STRING     (1 << 1)
#define AS_HEX_STRING (1 << 2)
#define AS_ARRAY      (1 << 3)

size_t
StringLength(uint8 *String)
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
  int ByteIndex, BitIndex;

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
}

struct byte_buffer
{
  uint8 *Buffer;
  size_t Size;
};

byte_buffer *
DecodeHex(uint8 *HexString)
{
  byte_buffer *ByteBuffer = (byte_buffer *)malloc(sizeof(byte_buffer));
  ByteBuffer->Size = StringLength(HexString) / 2;
  //printf("HexString: %s\nStringLength(HexString): %lu, ByteBufferSize: %lu\n",
         //HexString, StringLength(HexString), ByteBuffer->Size);

  ByteBuffer->Buffer = (uint8 *)malloc(sizeof(uint8) * ByteBuffer->Size);
  if(!ByteBuffer->Buffer) printf("Couldn't allocate ByteBuffer.\n");

  int8 FirstHexChar, SecondHexChar;
  int8 HexByteBuffer[3];
  for(int HexIndex = 0; HexIndex < ByteBuffer->Size; HexIndex++)
  {
    FirstHexChar = HexString[HexIndex*2];
    SecondHexChar = HexString[(HexIndex*2)+1];
    HexByteBuffer[0] = FirstHexChar;
    HexByteBuffer[1] = SecondHexChar;
    HexByteBuffer[2] = 0;
    int8 Byte = strtol((char *)HexByteBuffer, 0, 16);
    ByteBuffer->Buffer[HexIndex] = Byte;
    //printf("%d\t%s\t%c\t%c\t%d\t%c\n",
    //HexIndex, HexByteBuffer, FirstHexChar, SecondHexChar,
    //Byte, Byte);
  }

  return ByteBuffer;
}

uint8 *
EncodeBase64(byte_buffer *ByteBuffer)
{
  uint8 Base64Padding = (ByteBuffer->Size % 3 != 0) ? (3 - (ByteBuffer->Size % 3)) : 0;
  uint8 PaddedByteBufferSize = ByteBuffer->Size + Base64Padding;
  // NOTE(yuri): +1 length for NUL
  uint8 Base64Length = (PaddedByteBufferSize * 8 / 6) + 1;
  printf("Base64Length: %d, Base64Padding: %d\n",
         Base64Length, Base64Padding);

  uint8 *Base64Buffer = (uint8 *)malloc(sizeof(uint8) * Base64Length);
  if(!Base64Buffer) printf("Couldn't allocate Base64Buffer.\n");

  printf("ByteBufferSize: %lu, Base64Remainder: %d\n",
         ByteBuffer->Size, Base64Padding);

  uint8 AsciiTriplet[3];
  uint8 Base64Sextet[4];
  uint8 Mask = 0xff;
  int Base64Index = 0;

  uint8 Base64LookupTable[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '='
  };

  for(int TripletIndex = 0;
      TripletIndex < PaddedByteBufferSize / 3;
      TripletIndex++)
  {
    for(int i = 0; i < ArrayCount(AsciiTriplet); i++)
    {
      // TODO(yuri): Does filling this out backward with 2-i give us the right
      // block of memory?
      AsciiTriplet[i] = ByteBuffer->Buffer[TripletIndex*3 + i];
    }

    // TODO(yuri): There has to be a better way of doing this.
    Base64Sextet[0] = ((Mask << 2) & AsciiTriplet[0]) >> 2;

    Base64Sextet[1] =
      (((Mask >> 6) & AsciiTriplet[0]) << 4) |
      (((Mask << 4) & AsciiTriplet[1]) >> 4);

    if (AsciiTriplet[1] && AsciiTriplet[2])
    {
      Base64Sextet[2] =
        (((Mask >> 4) & AsciiTriplet[1]) << 2) |
        (((Mask << 6) & AsciiTriplet[2]) >> 6);
    }
    else
    {
      Base64Sextet[2] = 64;
    }

    if (AsciiTriplet[2])
    {
      Base64Sextet[3] = (Mask >> 2) & AsciiTriplet[2];
    }
    else
    {
      Base64Sextet[3] = 64;
    }
    //PrintBits(&Base64Sextet[0], 1);
    //PrintBits(&Base64Sextet[1], 1);
    //PrintBits(&Base64Sextet[2], 1);
    //PrintBits(&Base64Sextet[3], 1);


    for(int i = 0; i < ArrayCount(Base64Sextet); i++)
    {
      //printf("%d: %c\n", Base64Sextet[i], Base64LookupTable[Base64Sextet[i]]);

      //Base64Buffer
      //printf("%c: TripletIndex: %d, i: %d %d\n", Base64LookupTable[Base64Sextet[i]], TripletIndex, i, Base64Length);
      //printf("%c", Base64LookupTable[Base64Sextet[i]]);
      Base64Buffer[Base64Index++] = Base64LookupTable[Base64Sextet[i]];
    }

    //printf("%d, %d\n", TripletIndex, PaddedByteBufferSize / 3);
    //printf("%c%c%c|\n", AsciiTriplet[0], AsciiTriplet[1], AsciiTriplet[2]);
  }
  Base64Buffer[Base64Index] = 0;

  return Base64Buffer;
}


void
PrintByteBufferAsString(byte_buffer *ByteBuffer)
{
  for(int i = 0; i < ByteBuffer->Size; i++)
  {
    printf("%c", ByteBuffer->Buffer[i]);
  }
  printf("\n");
}

uint8*
EncodeHex(byte_buffer *ByteBuffer)
{
  size_t HexStringLength = (ByteBuffer->Size * 2) + 1;
  uint8* HexString = (uint8 *)malloc(sizeof(uint8) * HexStringLength);
  if(!HexString) printf("Failed to allocate HexString in EncodeHex\n");

  uint8* HexStringStart = HexString;
  for(int i = 0; i < ByteBuffer->Size; i++)
  {
    sprintf((char *)HexString, "%02x", ByteBuffer->Buffer[i]);
    HexString += 2;
  }

  HexString = HexStringStart;
  //printf("HexString after EncodeHex: %s\n", HexString);

  return HexString;
}


void
PrintByteBufferAsHexString(byte_buffer *ByteBuffer)
{
  uint8 *HexString = EncodeHex(ByteBuffer);
  printf("%s\n", HexString);
  free(HexString);
}

void
PrintByteBufferAsArray(byte_buffer *ByteBuffer)
{
  printf("Char\tHex\tDec\tBin\n");
  for(int i = 0; i < ByteBuffer->Size; i++)
  {
    uint8 Val = ByteBuffer->Buffer[i];
    printf("%c\t0x%02x\t%d\t", Val, Val, Val);
    PrintBits(&Val, 1);
    printf("\n");
  }
}

void
FreeByteBuffer(byte_buffer *ByteBuffer)
{
  if(ByteBuffer)
  {
    if(ByteBuffer->Buffer)
    {
      free(ByteBuffer->Buffer);
    }
    free(ByteBuffer);
  }
}

void
Print(void *Value, flag Type, flag PrintOptions)
{
  byte_buffer *ByteBuffer;
  if(Type & BYTE_BUFFER)
  {
    ByteBuffer = (byte_buffer *)Value;
  }
  else if (Type & HEX_STRING)
  {
    ByteBuffer = DecodeHex((uint8 *)Value);
  }

  if(PrintOptions & AS_STRING)
  {
    PrintByteBufferAsString(ByteBuffer);
  }

  if(PrintOptions & AS_HEX_STRING)
  {
    PrintByteBufferAsHexString(ByteBuffer);
  }

  if (PrintOptions & AS_ARRAY)
  {
    PrintByteBufferAsArray(ByteBuffer);
  }

  FreeByteBuffer(ByteBuffer);
}

bool32
StringsAreEqual(uint8 *Str1, uint8 *Str2)
{
  bool32 Result = 0;
  if(StringLength(Str1) != StringLength(Str2)) return Result;

  do {
    if (*Str1 != *Str2)
    {
      return Result;
    }
  } while (*Str1++ && *Str2++);

  return 1;
}

int
main(int argc, char *argv[])
{
  //uint8 String;
  //String = 90;
  //PrintBits(&String, sizeof(uint8));
  //String = strtol("5a", 0, 16);
  //PrintBits(&String, sizeof(uint8));

  uint8 *HexString = (uint8 *)
    "49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d5a";
  //"4d616e";

  Print(HexString, HEX_STRING, AS_HEX_STRING|AS_STRING);
  byte_buffer *ByteBuffer = DecodeHex(HexString);
  uint8 *EncodedHexString = EncodeHex(ByteBuffer);
  FreeByteBuffer(ByteBuffer);
  free(EncodedHexString);
  //printf("here %d\n", StringsAreEqual(HexString, EncodedHexString));
#if 0
  Print(ByteBuffer, BYTE_BUFFER, AS_STRING);
  PrintByteBufferAsString(ByteBuffer);

  uint8 One = 0xff;
  PrintBits(&One, 1);

  printf("\n");

  uint8 *Base64Buffer = EncodeBase64(ByteBuffer);
  for(int i = 0; i < StringLength(Base64Buffer) + 1; i++)
  {
    //printf("%d|\t%d\n", Base64Buffer[i], i);
  }
  printf("\n");



  free(ByteBuffer->Buffer);
  free(ByteBuffer);
  free(Base64Buffer);
#endif
}
