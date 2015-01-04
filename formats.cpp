#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "portable_endian.h"

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

#define internal static
#define local_persist static
#define global_variable static

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

typedef uint16 flag;

// NOTE(yuri): Print Types
#define BYTE_BUFFER   (1 << 1)
#define HEX_STRING    (1 << 2)
#define BASE64_STRING (1 << 3)

// NOTE(yuri): Print Options
#define AS_STRING     (1 << 1)
#define AS_HEX_STRING (1 << 2)
#define AS_DUMP       (1 << 3)
#define AS_BASE64     (1 << 4)

void Print(void *Value, flag Type, flag PrintOptions);

struct base64_lookup {
  uint8 *LookupTable;
  uint8 PaddingByteIndex;
  uint8 *ReverseLookupTable;
};

global_variable base64_lookup GlobalBase64Lookup = {};

void
FillOutGlobalBase64Lookup()
{
  size_t Base64TableSize = 65;
  GlobalBase64Lookup.PaddingByteIndex = 64;
  GlobalBase64Lookup.LookupTable = (uint8 *)malloc(sizeof(uint8) * Base64TableSize);
  if(!GlobalBase64Lookup.LookupTable)
  {
    printf("Unable to malloc LookupTable in FillOutGlobalBase64Lookup");
  }

  uint8 *Base64Chars = (uint8 *)
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

  for(int CharacterIndex = 0;
      CharacterIndex < Base64TableSize;
      CharacterIndex++)
  {
    GlobalBase64Lookup.LookupTable[CharacterIndex] = Base64Chars[CharacterIndex];
  }

  int HighestCharCode = -1;
  for(int Base64TableIndex = 0;
      Base64TableIndex < Base64TableSize;
      Base64TableIndex++)
  {
    if(GlobalBase64Lookup.LookupTable[Base64TableIndex] > HighestCharCode)
    {
      HighestCharCode = GlobalBase64Lookup.LookupTable[Base64TableIndex];
    }
  }

  GlobalBase64Lookup.ReverseLookupTable =
    (uint8 *)malloc(sizeof(uint8) * HighestCharCode + 1);

  if(!GlobalBase64Lookup.ReverseLookupTable)
  {
    printf("Unable to malloc ReverseLookupTable in FillOutGlobalBase64Lookup");
  }

  uint8 CharCode = 0;
  for(int Base64TableIndex = 0;
      Base64TableIndex < Base64TableSize;
      Base64TableIndex++)
  {
    CharCode = GlobalBase64Lookup.LookupTable[Base64TableIndex];
    GlobalBase64Lookup.ReverseLookupTable[CharCode] = (CharCode != 0) ? Base64TableIndex : 0;
  }
}

void
FreeGlobalBase64Lookup()
{
  if(GlobalBase64Lookup.LookupTable)
  {
    free(GlobalBase64Lookup.LookupTable);
  }

  if(GlobalBase64Lookup.ReverseLookupTable)
  {
    free(GlobalBase64Lookup.ReverseLookupTable);
  }
}

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
    printf(" ");
  }
}

void
Print32Bits(uint32 *Value, size_t Size)
{
  uint8 Byte;
  int ByteIndex, BitIndex;

  for(ByteIndex = Size - 1; ByteIndex >= 0; ByteIndex--)
  {
    for(BitIndex = 31; BitIndex >= 0; BitIndex--)
    {
      Byte = Value[ByteIndex] & (1 << BitIndex);
      Byte >>= BitIndex;
      //printf("i: %d, j: %d, byte: %u\n", ByteIndex, BitIndex, Byte);
      printf("%u", Byte);
      if(BitIndex % 8 == 0) printf(" ");
    }
  }
  printf("\n");
}

struct byte_buffer
{
  uint8 *Buffer;
  size_t Size;
};

byte_buffer *
CreateByteBuffer(size_t ByteBufferSize)
{
  byte_buffer *ByteBuffer = (byte_buffer *)malloc(sizeof(byte_buffer));
  if(!ByteBuffer) printf("Failed to allocate ByteBuffer in CreateByteBuffer\n");

  ByteBuffer->Size = ByteBufferSize;
  ByteBuffer->Buffer = (uint8 *)malloc(sizeof(uint8) * ByteBuffer->Size);
  if(!ByteBuffer->Buffer) printf("Failed to allocate Buffer in ByteBuffer in CreateByteBuffer\n");

  return ByteBuffer;
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

byte_buffer *
DecodeHex(uint8 *HexString)
{
  byte_buffer *ByteBuffer = CreateByteBuffer(StringLength(HexString) / 2);

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

byte_buffer *
DecodeBase64(uint8 *Base64String)
{
  byte_buffer *ByteBuffer = CreateByteBuffer(10);
  for(int i = 0; i < ByteBuffer->Size; i++)
  {
    ByteBuffer->Buffer[i] = 65;
  }
  return ByteBuffer;
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

uint8 *
EncodeHex(byte_buffer *ByteBuffer)
{
  size_t HexStringLength = (ByteBuffer->Size * 2) + 1;
  uint8 *HexString = (uint8 *)malloc(sizeof(uint8) * HexStringLength);
  if(!HexString) printf("Failed to allocate HexString in EncodeHex\n");

  uint8 *HexStringStart = HexString;
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

bool32
StringsAreEqual(uint8 *Str1, uint8 *Str2)
{
  bool32 Result = 0;
  if(StringLength(Str1) != StringLength(Str2)) return Result;

  do {
    if(*Str1 != *Str2)
    {
      return Result;
    }
  } while (*Str1++ && *Str2++);

  return 1;
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

  uint8 Octet;
  uint32 QuadSextet = {0};
  QuadSextet = htobe32(QuadSextet);
  uint8 SixBitMask = 0xff >> 2;
  int Base64Index = 0;

  for(int TripletIndex = 0;
      TripletIndex < PaddedByteBufferSize / 3;
      TripletIndex++)
  {
    QuadSextet = 0;
    int ByteBufferIndex;
    for(int TriOctetIndex = 0; TriOctetIndex < 3; TriOctetIndex++)
    {
      ByteBufferIndex = TripletIndex*3 + TriOctetIndex;
      // TODO(yuri): Padding?
      Octet = ByteBuffer->Buffer[ByteBufferIndex];
      QuadSextet <<= 8;
      QuadSextet |= (uint32)Octet;
    }

    uint8 Sextet;
    for(int SextetIndex = 3; SextetIndex >= 0; SextetIndex--)
    {
      Sextet = (QuadSextet >> (SextetIndex * 6)) & SixBitMask;
      uint8 Base64Char = GlobalBase64Lookup.LookupTable[Sextet];
      Base64Buffer[Base64Index++] = Base64Char;
    }
    //for(int i = 0; i < ArrayCount(QuadSextet); i++)
    //{
      //Base64Buffer[Base64Index++] =
    //}









    // TODO(yuri): There has to be a better way of doing this.
    //QuadSextet[0] = ((Mask << 2) & TriOctet[0]) >> 2;

    //QuadSextet[1] =
      //(((Mask >> 6) & TriOctet[0]) << 4) |
      //(((Mask << 4) & TriOctet[1]) >> 4);

    //if(TriOctet[1] && TriOctet[2])
    //{
      //QuadSextet[2] =
        //(((Mask >> 4) & TriOctet[1]) << 2) |
        //(((Mask << 6) & TriOctet[2]) >> 6);
    //}
    //else
    //{
      //QuadSextet[2] = GlobalBase64Lookup.PaddingByteIndex;
    //}

    //if(TriOctet[2])
    //{
      //QuadSextet[3] = (Mask >> 2) & TriOctet[2];
    //}
    //else
    //{
      //QuadSextet[3] = GlobalBase64Lookup.PaddingByteIndex;
    //}

    //for(int i = 0; i < ArrayCount(QuadSextet); i++)
    //{
      //Base64Buffer[Base64Index++] = GlobalBase64Lookup.LookupTable[QuadSextet[i]];
    //}

  }
  Base64Buffer[Base64Index] = 0;

  return Base64Buffer;
}

void
PrintByteBufferAsBase64(byte_buffer *ByteBuffer)
{
  uint8 *Base64Buffer = EncodeBase64(ByteBuffer);
  printf("%s\n", Base64Buffer);
  free(Base64Buffer);
}


void
Print(void *Value, flag Type, flag PrintOptions)
{
  byte_buffer *ByteBuffer;
  if(Type & BYTE_BUFFER)
  {
    ByteBuffer = (byte_buffer *)Value;
  }
  else if(Type & HEX_STRING)
  {
    ByteBuffer = DecodeHex((uint8 *)Value);
  }
  else if(Type & BASE64_STRING)
  {
    ByteBuffer = DecodeBase64((uint8 *)Value);
  }

  if(PrintOptions & AS_STRING)
  {
    PrintByteBufferAsString(ByteBuffer);
  }

  if(PrintOptions & AS_HEX_STRING)
  {
    PrintByteBufferAsHexString(ByteBuffer);
  }

  if(PrintOptions & AS_DUMP)
  {
    PrintByteBufferAsArray(ByteBuffer);
  }

  if(PrintOptions & AS_BASE64)
  {
    PrintByteBufferAsBase64(ByteBuffer);
  }

  if(!(Type & BYTE_BUFFER))
  {
    FreeByteBuffer(ByteBuffer);
  }
}


int
main(int argc, char *argv[])
{
  FillOutGlobalBase64Lookup();

  uint8 *HexString = (uint8 *)
    //"49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d5a";
  "4d616e";

  //Print(HexString, HEX_STRING, AS_HEX_STRING|AS_STRING|AS_BASE64);
  byte_buffer *ByteBuffer = DecodeHex(HexString);
  uint8 *EncodedHexString = EncodeHex(ByteBuffer);
  Print(ByteBuffer, BYTE_BUFFER, AS_BASE64);

  printf("hurr");
  uint8 *Base64String = EncodeBase64(ByteBuffer);
  //Print(ByteBuffer, BYTE_BUFFER, AS_STRING);

  FreeByteBuffer(ByteBuffer);
  free(EncodedHexString);
  free(Base64String);
  FreeGlobalBase64Lookup();
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
