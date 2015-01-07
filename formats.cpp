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
PrintByteBufferAsDump(byte_buffer *ByteBuffer)
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

byte_buffer *
DecodeBase64(uint8 *Base64String)
{
    size_t Base64Length = StringLength(Base64String);

    int Base64Padding = 0;
    if(GlobalBase64Lookup.ReverseLookupTable[Base64String[Base64Length - 1]] == GlobalBase64Lookup.PaddingByteIndex)
    {
        Base64Padding++;
    }
    if(GlobalBase64Lookup.ReverseLookupTable[Base64String[Base64Length - 2]] == GlobalBase64Lookup.PaddingByteIndex)
    {
        Base64Padding++;
    }

    size_t ByteBufferSize = (Base64Length * 6 / 8) - Base64Padding;
    byte_buffer *ByteBuffer = CreateByteBuffer(ByteBufferSize);

    uint8 ByteMask = 0xff;
    int ByteBufferIndex = 0;


    for(int QuadrupletIndex = 0;
        QuadrupletIndex < Base64Length / 4;
        QuadrupletIndex++)
    {
        uint32 QuadSextet = 0;
        for(int QuadSextetIndex = 0;
            QuadSextetIndex < 4;
            QuadSextetIndex++)
        {
            int Base64Index = QuadrupletIndex*4 + QuadSextetIndex;
            uint8 Base64Char = Base64String[Base64Index];
            uint8 Base64CharIndex = GlobalBase64Lookup.ReverseLookupTable[Base64Char];

            QuadSextet |= Base64CharIndex;
            QuadSextet <<= 6;
        }
        // NOTE(yuri): Shift back by 6 because of extraneous shift above on last loop
        QuadSextet >>= 6;

        for(int TriOctetIndex = 0;
            TriOctetIndex < 3;
            TriOctetIndex++)
        {
            ByteBuffer->Buffer[ByteBufferIndex++] = (QuadSextet >> ((2 - TriOctetIndex) * 8)) & ByteMask;
        }
    }

    return ByteBuffer;
}

uint8 *
EncodeBase64(byte_buffer *ByteBuffer)
{
    int Base64Padding = (ByteBuffer->Size % 3 != 0) ? (3 - (ByteBuffer->Size % 3)) : 0;
    int PaddedByteBufferSize = ByteBuffer->Size + Base64Padding;
    size_t Base64Length = (PaddedByteBufferSize * 8 / 6);

    // NOTE(yuri): +1 length for NUL
    uint8 *Base64Buffer = (uint8 *)malloc(sizeof(uint8) * (Base64Length + 1));
    if(!Base64Buffer) printf("Couldn't allocate Base64Buffer.\n");

    uint8 Octet;
    uint8 Sextet;
    uint32 QuadSextet;

    // NOTE(yuri): This ensures that we're representing our 4 6-bit
    // values in memory as big-endian
    QuadSextet = htobe32(QuadSextet);

    int Base64Index = 0;
    uint8 SixBitMask = 0xff >> 2;

    for(int TripletIndex = 0;
        TripletIndex < PaddedByteBufferSize / 3;
        TripletIndex++)
    {
        QuadSextet = 0;
        int ByteBufferIndex;
        for(int TriOctetIndex = 0; TriOctetIndex < 3; TriOctetIndex++)
        {
            ByteBufferIndex = TripletIndex*3 + TriOctetIndex;
            Octet = ByteBuffer->Buffer[ByteBufferIndex];
            QuadSextet <<= 8;
            QuadSextet |= (uint32)Octet;
        }

        for(int QuadSextetIndex = 3; QuadSextetIndex >= 0; QuadSextetIndex--)
        {
            if((Base64Length - Base64Padding) > Base64Index)
            {
                Sextet = (QuadSextet >> (QuadSextetIndex * 6)) & SixBitMask;
            } else {
                Sextet = GlobalBase64Lookup.PaddingByteIndex;
            }
            uint8 Base64Char = GlobalBase64Lookup.LookupTable[Sextet];
            Base64Buffer[Base64Index++] = Base64Char;
        }
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
        PrintByteBufferAsDump(ByteBuffer);
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
        "49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d5a";
        // "4d616e";

    // Print(HexString, HEX_STRING, AS_HEX_STRING|AS_STRING|AS_BASE64);
    byte_buffer *ByteBuffer = DecodeHex(HexString);
    uint8 *EncodedHexString = EncodeHex(ByteBuffer);
    Print(ByteBuffer, BYTE_BUFFER, AS_BASE64);

    uint8 *Base64String = EncodeBase64(ByteBuffer);
    Print(Base64String, BASE64_STRING, AS_STRING);

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
