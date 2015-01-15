#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
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
#define BYTE_BUFFER    (1 << 1)
#define HEX_STRING     (1 << 2)
#define BASE64_STRING  (1 << 3)
#define STRING         (1 << 4)

// NOTE(yuri): Print Options
#define AS_STRING      (1 << 1)
#define AS_NICE_STRING (1 << 2)
#define AS_HEX         (1 << 3)
#define AS_DUMP        (1 << 4)
#define AS_BASE64      (1 << 5)

void Challenge1();
void Challenge2();
void Challenge3();
void Challenge4();
void Challenge5();
void Challenge6();

void Print(void *Value, flag Type, flag PrintOptions);

struct byte_buffer
{
    uint8 *Buffer;
    size_t Size;
};

struct base64_lookup
{
    uint8 *LookupTable;
    uint8 PaddingByteIndex;
    uint8 *ReverseLookupTable;
};

struct scored_buffer
{
    byte_buffer ByteBuffer;
    real32 Score;
    byte_buffer Key;
};

struct string_buffers
{
    uint8 **String;
    size_t Size;
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

uint8 *
SeekToChar(uint8 *String, uint8 Char, size_t MaxSeek)
{
    uint8 *SeekPtr = String;
    int Seek = 0;
    while(Seek < MaxSeek && *SeekPtr != Char)
    {
        Seek++;
        SeekPtr++;
    }

    return SeekPtr;
}

size_t
StringLength(uint8 *String)
{
    int Result = 0;

    while(*String++)
    {
        Result++;
    }

    return Result;
}

void
PrintNiceChar(uint8 Char)
{
    if(Char == '\n')
    {
        printf("\\n");
    }
    else if(Char == '\r')
    {
        printf("\\r");
    }
    else if(Char == '\t')
    {
        printf("\\t");
    }
    else if(Char == 0)
    {
        printf("\\0");
    }
    else if(!isprint(Char))
    {
        printf(".");
    }
    else
    {
        printf("%c", Char);
    }
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

byte_buffer
CreateByteBuffer(size_t ByteBufferSize)
{
    byte_buffer ByteBuffer = {0};

    ByteBuffer.Size = ByteBufferSize;
    ByteBuffer.Buffer = (uint8 *)malloc(sizeof(uint8) * ByteBuffer.Size);
    if(!ByteBuffer.Buffer) printf("Failed to allocate Buffer in ByteBuffer in CreateByteBuffer\n");

    return ByteBuffer;
}

void
FreeByteBuffer(byte_buffer ByteBuffer)
{
    if(ByteBuffer.Buffer)
    {
        free(ByteBuffer.Buffer);
    }
}

byte_buffer
CopyByteBuffer(byte_buffer ByteBuffer)
{
    byte_buffer Result = CreateByteBuffer(ByteBuffer.Size);

    for(int ByteBufferIndex = 0;
        ByteBufferIndex < ByteBuffer.Size;
        ByteBufferIndex++)
    {
        Result.Buffer[ByteBufferIndex] = ByteBuffer.Buffer[ByteBufferIndex];
    }

    return Result;
}

byte_buffer
DecodeHex(uint8 *HexString)
{
    byte_buffer ByteBuffer = CreateByteBuffer(StringLength(HexString) / 2);

    int8 FirstHexChar, SecondHexChar;
    int8 HexByteBuffer[3];
    for(int HexIndex = 0; HexIndex < ByteBuffer.Size; HexIndex++)
    {
        FirstHexChar = HexString[HexIndex*2];
        SecondHexChar = HexString[(HexIndex*2)+1];
        HexByteBuffer[0] = FirstHexChar;
        HexByteBuffer[1] = SecondHexChar;
        HexByteBuffer[2] = 0;
        int8 Byte = strtol((char *)HexByteBuffer, 0, 16);
        ByteBuffer.Buffer[HexIndex] = Byte;
    }

    return ByteBuffer;
}

void
PrintByteBufferAsNiceString(byte_buffer ByteBuffer)
{
    for(int i = 0; i < ByteBuffer.Size; i++)
    {
        PrintNiceChar(ByteBuffer.Buffer[i]);
    }
    printf("\n");
}

void
PrintByteBufferAsString(byte_buffer ByteBuffer)
{
    for(int i = 0; i < ByteBuffer.Size; i++)
    {
        printf("%c", ByteBuffer.Buffer[i]);
    }
    printf("\n");
}

uint8 *
EncodeHex(byte_buffer ByteBuffer)
{
    size_t HexStringLength = (ByteBuffer.Size * 2) + 1;
    uint8 *HexString = (uint8 *)malloc(sizeof(uint8) * HexStringLength);
    if(!HexString) printf("Failed to allocate HexString in EncodeHex\n");

    uint8 *HexStringStart = HexString;
    for(int i = 0; i < ByteBuffer.Size; i++)
    {
        sprintf((char *)HexString, "%02x", ByteBuffer.Buffer[i]);
        HexString += 2;
    }

    HexString = HexStringStart;
    //printf("HexString after EncodeHex: %s\n", HexString);

    return HexString;
}

void
PrintByteBufferAsHexString(byte_buffer ByteBuffer)
{
    uint8 *HexString = EncodeHex(ByteBuffer);
    printf("%s\n", HexString);
    free(HexString);
}

void
PrintByteBufferAsDump(byte_buffer ByteBuffer)
{
    printf("Char\tHex\tDec\tBin\n");
    for(int i = 0; i < ByteBuffer.Size; i++)
    {
        uint8 Val = ByteBuffer.Buffer[i];
        PrintNiceChar(Val);
        printf("\t0x%02x\t%d\t", Val, Val);
        PrintBits(&Val, 1);
        printf("\n");
    }
    printf("\n");
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

byte_buffer
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
    byte_buffer ByteBuffer = CreateByteBuffer(ByteBufferSize);

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
            if(ByteBufferIndex < ByteBufferSize)
            {
                ByteBuffer.Buffer[ByteBufferIndex++] =
                    (QuadSextet >> ((2 - TriOctetIndex) * 8)) & ByteMask;
            }
        }
    }

    return ByteBuffer;
}

uint8 *
EncodeBase64(byte_buffer ByteBuffer)
{
    int Base64Padding = (ByteBuffer.Size % 3 != 0) ? (3 - (ByteBuffer.Size % 3)) : 0;
    int PaddedByteBufferSize = ByteBuffer.Size + Base64Padding;
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
            Octet = ByteBuffer.Buffer[ByteBufferIndex];
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
PrintByteBufferAsBase64(byte_buffer ByteBuffer)
{
    uint8 *Base64Buffer = EncodeBase64(ByteBuffer);
    printf("%s\n", Base64Buffer);
    free(Base64Buffer);
}

byte_buffer
StringToByteBuffer(uint8 *String, bool32 IncludeNUL)
{
    size_t Length = StringLength(String);

    // NOTE(yuri): These byte_buffers will include the trailing NUL
    if(IncludeNUL)
    {
        Length += 1;
    }
    byte_buffer ByteBuffer = CreateByteBuffer(Length);

    for(int ByteBufferIndex = 0;
        ByteBufferIndex < ByteBuffer.Size;
        ByteBufferIndex++)
    {
        ByteBuffer.Buffer[ByteBufferIndex] = *(String + ByteBufferIndex);
    }

    return ByteBuffer;
}


void
Print(void *Value, flag Type, flag PrintOptions)
{
    byte_buffer ByteBuffer;
    if(Type & BYTE_BUFFER)
    {
        ByteBuffer = *(byte_buffer *)Value;
    }
    else if(Type & HEX_STRING)
    {
        ByteBuffer = DecodeHex((uint8 *)Value);
    }
    else if(Type & BASE64_STRING)
    {
        ByteBuffer = DecodeBase64((uint8 *)Value);
    }
    else if(Type & STRING)
    {
        ByteBuffer = StringToByteBuffer((uint8 *)Value, 1);
    }

    if(PrintOptions & AS_STRING)
    {
        PrintByteBufferAsString(ByteBuffer);
    }

    if(PrintOptions & AS_NICE_STRING)
    {
        PrintByteBufferAsNiceString(ByteBuffer);
    }

    if(PrintOptions & AS_HEX)
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

byte_buffer
XORBuffers(byte_buffer A, byte_buffer B)
{
    assert(A.Size == B.Size);
    byte_buffer Result = CreateByteBuffer(A.Size);

    for(int ByteBufferIndex = 0;
        ByteBufferIndex < Result.Size;
        ByteBufferIndex++)
    {
        Result.Buffer[ByteBufferIndex] =
            A.Buffer[ByteBufferIndex] ^ B.Buffer[ByteBufferIndex];
    }

    return Result;
}

byte_buffer
XORBufferSingleChar(byte_buffer ByteBuffer, uint8 XORChar)
{
    byte_buffer Result = CreateByteBuffer(ByteBuffer.Size);
    for(int ByteBufferIndex = 0;
        ByteBufferIndex < Result.Size;
        ByteBufferIndex++)
    {
        Result.Buffer[ByteBufferIndex] =
            ByteBuffer.Buffer[ByteBufferIndex] ^ XORChar;
    }

    return Result;
}

byte_buffer
XORBufferRepeating(byte_buffer ByteBuffer, byte_buffer Key)
{
    byte_buffer Result = CreateByteBuffer(ByteBuffer.Size);

    for(int ByteBufferIndex = 0;
        ByteBufferIndex < ByteBuffer.Size;
        ByteBufferIndex++)
    {
        Result.Buffer[ByteBufferIndex] =
            ByteBuffer.Buffer[ByteBufferIndex] ^ Key.Buffer[ByteBufferIndex % Key.Size];
    }

    return Result;
}

real32
ScoreLetter(uint8 Letter)
{
    real32 Result = 0.0f;
    real32 EnglishLetterFrequencies[] = {
        8.167, 1.492, 2.782, 4.253, 2.702, 2.228, 2.015, 6.094, 6.966, 0.153, 0.772, 4.025, 2.406,
        6.749, 7.507, 1.929, 0.095, 5.987, 6.327, 9.056, 2.758, 0.978, 2.360, 0.150, 1.974, 0.074
    };

    int FrequencyTableIndex = tolower(Letter) - 97;
    if(FrequencyTableIndex >= 0 && FrequencyTableIndex < 26)
    {
        Result = EnglishLetterFrequencies[FrequencyTableIndex];
    }
    else if(Letter == ' ')
    {
        Result = 0.5;
    }
    return Result;
}

real32
ScoreBuffer(byte_buffer ByteBuffer)
{
    real32 Result = 0;

    for(int ByteBufferIndex = 0;
        ByteBufferIndex < ByteBuffer.Size;
        ByteBufferIndex++)
    {
        Result += ScoreLetter(ByteBuffer.Buffer[ByteBufferIndex]);
    }

    Result /= ByteBuffer.Size;

    return Result;
}

scored_buffer
CreateEmptyScoredBuffer(void)
{
    scored_buffer Result = {0};
    return Result;
}

void
FreeScoreBuffer(scored_buffer ScoredBuffer)
{
    FreeByteBuffer(ScoredBuffer.ByteBuffer);
    FreeByteBuffer(ScoredBuffer.Key);
}

scored_buffer
MaxBufferScore(scored_buffer ScoredBuffer, byte_buffer ByteBuffer, byte_buffer Key)
{
    real32 NewScore = ScoreBuffer(ByteBuffer);
    if(NewScore > ScoredBuffer.Score)
    {
        FreeByteBuffer(ScoredBuffer.ByteBuffer);
        FreeByteBuffer(ScoredBuffer.Key);
        ScoredBuffer.Score = NewScore;
        ScoredBuffer.ByteBuffer = CopyByteBuffer(ByteBuffer);
        ScoredBuffer.Key = CopyByteBuffer(Key);
    }
    return ScoredBuffer;
}

byte_buffer
OpenFileBuffer(uint8 *Filename)
{
    byte_buffer FileBuffer = {0};
    FILE *File = fopen((char *)Filename, "r");

    if(File)
    {
        fseek(File, 0, SEEK_END);
        FileBuffer.Size = ftell(File);
        fseek(File, 0, SEEK_SET);
        FileBuffer.Buffer = (uint8 *)malloc(sizeof(uint8) * FileBuffer.Size);
        if(FileBuffer.Buffer)
        {
            fread(FileBuffer.Buffer, 1, FileBuffer.Size, File);
        }
        fclose (File);
    }

    return FileBuffer;
}

void
FreeFileBuffer(byte_buffer FileBuffer)
{
    if(FileBuffer.Buffer)
    {
        free(FileBuffer.Buffer);
    }
}

int
CountOccurancesInString(uint8 *String, uint8 Char, size_t Length)
{
    int Result = 0;
    uint8 *CharPointer = String;

    size_t ReadLength = 0;

    while(ReadLength < Length)
    {
        if(*CharPointer == Char)
        {
            Result++;
        }
        CharPointer++;
        ReadLength++;
    }

    return Result;
}


string_buffers
ReadFileIntoStringBuffers(byte_buffer FileBuffer)
{
    string_buffers StringBuffers = {0};

    uint8 *FileBufferEnd = FileBuffer.Buffer + FileBuffer.Size;
    bool32 TrailingNewLine = *(FileBufferEnd - 1) == '\n';

    StringBuffers.Size = CountOccurancesInString(FileBuffer.Buffer, '\n', FileBuffer.Size);
    if(!TrailingNewLine)
    {
        StringBuffers.Size++;
    }

    // NOTE(yuri): Allocate an appropriate 2D array
    StringBuffers.String = (uint8 **)malloc(sizeof(uint8 *) * StringBuffers.Size);
    if(StringBuffers.String == 0) printf("Allocating StringBuffers.String failed\n");

    // printf("Allocated StringBuffers.String of size %d\n", StringBuffers.Size);

    // NOTE(yuri): Allocate arrays for strings and copy over strings
    uint8 *CurrentLinePointer = FileBuffer.Buffer;
    for(int StringIndex = 0;
        StringIndex < StringBuffers.Size;
        StringIndex++)
    {
        uint8 *NewLinePointer = SeekToChar(CurrentLinePointer, '\n', FileBufferEnd - CurrentLinePointer);
        size_t CharCount = NewLinePointer - CurrentLinePointer;
        StringBuffers.String[StringIndex] = (uint8 *)malloc(sizeof(uint8) * (CharCount + 1));

        uint8 CharIndex = 0;
        // NOTE(yuri): Since NextLinePointer starts on the next line, we want to read up to it
        while(CharIndex < CharCount)
        {
            StringBuffers.String[StringIndex][CharIndex] = *CurrentLinePointer;
            CurrentLinePointer++;
            CharIndex++;
        }
        StringBuffers.String[StringIndex][CharIndex] = 0;
        CurrentLinePointer = NewLinePointer + 1;
    }

    return StringBuffers;
}

void
FreeStringBuffers(string_buffers StringBuffers)
{
    if(StringBuffers.String)
    {
        for(int StringIndex = 0;
            StringIndex < StringBuffers.Size;
            StringIndex++)
        {
            if(StringBuffers.String[StringIndex])
            {
                free(StringBuffers.String[StringIndex]);
            }
        }
        free(StringBuffers.String);
    }
}

int
HammingWeight(uint8 Number)
{
    int Result = __builtin_popcount(Number);
    return Result;
}

int
HammingDistance(byte_buffer TestA, byte_buffer TestB)
{
    int Result = 0;

    for(int ByteBufferIndex = 0;
        ByteBufferIndex < TestA.Size;
        ByteBufferIndex++)
    {
        Result += HammingWeight(TestA.Buffer[ByteBufferIndex] ^
                                TestB.Buffer[ByteBufferIndex]);
    }

    return Result;
}

byte_buffer
ReadFileAsWrappedBase64String(byte_buffer FileBuffer)
{
    int NewlineCount = CountOccurancesInString(FileBuffer.Buffer, '\n', FileBuffer.Size);
    printf("Count: %d, Size: %zu\n", NewlineCount, FileBuffer.Size);
    size_t Base64StringSize = FileBuffer.Size - NewlineCount;
    uint8 *Base64String = (uint8 *)malloc(sizeof(uint8) * Base64StringSize + 1);

    int Base64Index = 0;
    for(int FileBufferIndex = 0;
        FileBufferIndex < FileBuffer.Size;
        FileBufferIndex++)
    {
        uint8 Char = FileBuffer.Buffer[FileBufferIndex];
        if(Char != '\n')
        {
            Base64String[Base64Index] = Char;
            Base64Index++;
        }
    }
    Base64String[Base64Index] = 0;

    byte_buffer ByteBuffer = DecodeBase64(Base64String);
    free(Base64String);

    return ByteBuffer;
}

scored_buffer
BreakSingleCharacterXOR(byte_buffer ByteBuffer, scored_buffer ScoredBuffer)
{
    for(int Char = 0;
        Char < 256;
        Char++)
    {
        byte_buffer Key = CreateByteBuffer(sizeof(uint8));
        Key.Buffer[0] = (uint8)Char;
        byte_buffer XORBuffer = XORBufferSingleChar(ByteBuffer, Key.Buffer[0]);
        ScoredBuffer = MaxBufferScore(ScoredBuffer, XORBuffer, Key);
        FreeByteBuffer(Key);
        FreeByteBuffer(XORBuffer);
    }

    return ScoredBuffer;
}

real32
AverageEditDistance(byte_buffer ByteBuffer, int KeySize)
{
    real32 Result = 0;

    byte_buffer KeySizeBufferA = CreateByteBuffer(KeySize);
    byte_buffer KeySizeBufferB = CreateByteBuffer(KeySize);
    byte_buffer KeySizeBufferC = CreateByteBuffer(KeySize);
    byte_buffer KeySizeBufferD = CreateByteBuffer(KeySize);
    for(int KeySizeIndex = 0;
        KeySizeIndex < KeySize;
        KeySizeIndex++)
    {
        uint8 CharA = ByteBuffer.Buffer[KeySizeIndex];
        KeySizeBufferA.Buffer[KeySizeIndex] = CharA ? CharA : 0;

        uint8 CharB = ByteBuffer.Buffer[KeySize + KeySizeIndex];
        KeySizeBufferB.Buffer[KeySizeIndex] = CharB ? CharB : 0;

        uint8 CharC = ByteBuffer.Buffer[KeySize*2 + KeySizeIndex];
        KeySizeBufferC.Buffer[KeySizeIndex] = CharC ? CharC : 0;

        uint8 CharD = ByteBuffer.Buffer[KeySize*3 + KeySizeIndex];
        KeySizeBufferD.Buffer[KeySizeIndex] = CharD ? CharD : 0;
    }
    Result = (((real32)HammingDistance(KeySizeBufferA, KeySizeBufferB) +
               (real32)HammingDistance(KeySizeBufferB, KeySizeBufferC) +
               (real32)HammingDistance(KeySizeBufferC, KeySizeBufferD) +
               (real32)HammingDistance(KeySizeBufferA, KeySizeBufferC) +
               (real32)HammingDistance(KeySizeBufferA, KeySizeBufferD) +
               (real32)HammingDistance(KeySizeBufferB, KeySizeBufferD))
              / 6) / KeySize;

    FreeByteBuffer(KeySizeBufferA);
    FreeByteBuffer(KeySizeBufferB);
    FreeByteBuffer(KeySizeBufferC);
    FreeByteBuffer(KeySizeBufferD);

    return Result;
}

int
main(int argc, char *argv[])
{
    FillOutGlobalBase64Lookup();

    // Challenge1();
    // Challenge2();
    // Challenge3();
    // Challenge4();
    // Challenge5();
    // Challenge6();

    FreeGlobalBase64Lookup();
}

// Break repeating-key XOR
void
Challenge6()
{
    byte_buffer FileBuffer = OpenFileBuffer((uint8 *)"data/6.txt");
    byte_buffer ByteBuffer = ReadFileAsWrappedBase64String(FileBuffer);
    FreeFileBuffer(FileBuffer);

    real32 SmallestEditDistance = 1000;
    int KeySizeGuess = 0;
    for(int KeySize = 2;
        KeySize < 40;
        KeySize++)
    {
        real32 EditDistance = AverageEditDistance(ByteBuffer, KeySize);
        if(EditDistance < SmallestEditDistance)
        {
            SmallestEditDistance = EditDistance;
            KeySizeGuess = KeySize;
        }
    }
    printf("Smallest edit distance is %f from keysize %d\n", SmallestEditDistance, KeySizeGuess);

    int PaddedSize = ByteBuffer.Size + (KeySizeGuess - (ByteBuffer.Size % KeySizeGuess));
    int TransposedSize = PaddedSize / KeySizeGuess;
    printf("PaddedSize: %d, TransposedSize: %d\n", PaddedSize, TransposedSize);
    byte_buffer KeyByteBuffer = CreateByteBuffer(KeySizeGuess);

    for(int KeySizeIndex = 0;
        KeySizeIndex < KeySizeGuess;
        KeySizeIndex++)
    {
        byte_buffer SingleCharXORBuffer = CreateByteBuffer(TransposedSize);

        for(int TransposedBufferIndex = 0;
            TransposedBufferIndex < TransposedSize;
            TransposedBufferIndex++)
        {
            int ByteBufferIndex = (TransposedBufferIndex * KeySizeGuess) + KeySizeIndex;
            uint8 Char = ByteBufferIndex < ByteBuffer.Size ? ByteBuffer.Buffer[ByteBufferIndex] : 0;
            SingleCharXORBuffer.Buffer[TransposedBufferIndex] = Char;
        }

        scored_buffer ScoredBuffer = BreakSingleCharacterXOR(SingleCharXORBuffer, CreateEmptyScoredBuffer());
        KeyByteBuffer.Buffer[KeySizeIndex] = ScoredBuffer.Key.Buffer[0];
        FreeScoreBuffer(ScoredBuffer);

        FreeByteBuffer(SingleCharXORBuffer);
    }
    Print(&KeyByteBuffer, BYTE_BUFFER, AS_NICE_STRING);

    byte_buffer PlainText = XORBufferRepeating(ByteBuffer, KeyByteBuffer);
    Print(&PlainText, BYTE_BUFFER, AS_STRING);

    FreeByteBuffer(KeyByteBuffer);
    FreeByteBuffer(ByteBuffer);
}

// Implement repeating-key XOR
void
Challenge5()
{
    uint8 *PlainText = (uint8 *)"Burning 'em, if you ain't quick and nimble\nI go crazy when I hear a cymbal";
    uint8 *Key = (uint8 *)"ICE";
    byte_buffer PlainTextByteBuffer = StringToByteBuffer(PlainText, 0);
    byte_buffer KeyByteBuffer = StringToByteBuffer(Key, 0);
    byte_buffer Ciphered = XORBufferRepeating(PlainTextByteBuffer, KeyByteBuffer);

    uint8 *Encoded = EncodeHex(Ciphered);
    if(StringsAreEqual((uint8 *)"0b3637272a2b2e63622c2e69692a23693a2a3c6324202d623d63343c2a26226324272765272a282b2f20430a652e2c652a3124333a653e2b2027630c692b20283165286326302e27282f", Encoded))
    {
        printf("Repeating-Key XOR buffers are equal!\n");
    }
    free(Encoded);

    FreeByteBuffer(PlainTextByteBuffer);
    FreeByteBuffer(KeyByteBuffer);
    FreeByteBuffer(Ciphered);
}

// Detect single-character XOR
void
Challenge4()
{
    uint8 *Filename = (uint8 *)"data/4.txt";
    byte_buffer FileBuffer = OpenFileBuffer(Filename);

    if(FileBuffer.Buffer)
    {
        string_buffers StringBuffers = ReadFileIntoStringBuffers(FileBuffer);
        scored_buffer ScoredBuffer = CreateEmptyScoredBuffer();

        for(int StringIndex = 0;
            StringIndex < StringBuffers.Size;
            StringIndex++)
        {
            byte_buffer ByteBuffer = DecodeHex(StringBuffers.String[StringIndex]);
            ScoredBuffer = BreakSingleCharacterXOR(ByteBuffer, ScoredBuffer);
            FreeByteBuffer(ByteBuffer);
        }

        Print(&ScoredBuffer.ByteBuffer, BYTE_BUFFER, AS_STRING);
        FreeScoreBuffer(ScoredBuffer);
        FreeStringBuffers(StringBuffers);
    }
    FreeFileBuffer(FileBuffer);
}

// Single-byte XOR cipher
void
Challenge3()
{
    byte_buffer CipherText =
        DecodeHex((uint8 *)"1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736");

    scored_buffer ScoredBuffer = BreakSingleCharacterXOR(CipherText, CreateEmptyScoredBuffer());
    Print(&ScoredBuffer.ByteBuffer, BYTE_BUFFER, AS_STRING);

    FreeScoreBuffer(ScoredBuffer);
    FreeByteBuffer(CipherText);
    printf("\n");
}

// Fixed XOR
void
Challenge2()
{
    byte_buffer A = DecodeHex((uint8 *)"1c0111001f010100061a024b53535009181c");
    byte_buffer B = DecodeHex((uint8 *)"686974207468652062756c6c277320657965");
    Print(&A, BYTE_BUFFER, AS_STRING);
    Print(&B, BYTE_BUFFER, AS_STRING);
    byte_buffer X = XORBuffers(A, B);
    Print(&X, BYTE_BUFFER, AS_HEX|AS_STRING);

    FreeByteBuffer(A);
    FreeByteBuffer(B);
    FreeByteBuffer(X);
    printf("\n");
}

// Convert hex to base64
void
Challenge1()
{
    uint8 *HexString = (uint8 *)
        "49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d";

    uint8 *Base64String = (uint8 *)
        "SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t";

    Print(HexString, HEX_STRING, AS_BASE64);
    Print(Base64String, BASE64_STRING, AS_HEX);
    printf("\n");
}

