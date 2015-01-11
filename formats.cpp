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
#define BYTE_BUFFER   (1 << 1)
#define HEX_STRING    (1 << 2)
#define BASE64_STRING (1 << 3)
#define STRING        (1 << 4)

// NOTE(yuri): Print Options
#define AS_STRING     (1 << 1)
#define AS_HEX        (1 << 2)
#define AS_DUMP       (1 << 3)
#define AS_BASE64     (1 << 4)

void Challenge1();
void Challenge2();
void Challenge3();
void Challenge4();

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
CopyByteBuffer(byte_buffer *ByteBuffer)
{
    byte_buffer *Result = CreateByteBuffer(ByteBuffer->Size);

    for(int ByteBufferIndex = 0;
        ByteBufferIndex < ByteBuffer->Size;
        ByteBufferIndex++)
    {
        Result->Buffer[ByteBufferIndex] = ByteBuffer->Buffer[ByteBufferIndex];
    }

    return Result;
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
        if(Val == '\n')
        {
            printf("\\n");
        }
        else if(Val == '\r')
        {
            printf("\\r");
        }
        else if(Val == '\t')
        {
            printf("\\t");
        }
        else if(Val == 0)
        {
            printf("\\0");
        }
        else if(!isprint(Val))
        {
            printf(".");
        }
        else
        {
            printf("%c", Val);
        }
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

byte_buffer *
StringToByteBuffer(uint8 *String, bool32 IncludeNUL)
{
    size_t Length = StringLength(String);

    // NOTE(yuri): These byte_buffers will include the trailing NUL
    if(IncludeNUL)
    {
        Length += 1;
    }
    byte_buffer *ByteBuffer = CreateByteBuffer(Length);

    for(int ByteBufferIndex = 0;
        ByteBufferIndex < ByteBuffer->Size;
        ByteBufferIndex++)
    {
        ByteBuffer->Buffer[ByteBufferIndex] = *(String + ByteBufferIndex);
    }

    return ByteBuffer;
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
    else if(Type & STRING)
    {
        ByteBuffer = StringToByteBuffer((uint8 *)Value, 1);
    }

    if(PrintOptions & AS_STRING)
    {
        PrintByteBufferAsString(ByteBuffer);
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

byte_buffer *
XORBuffers(byte_buffer *A, byte_buffer *B)
{
    assert(A->Size == B->Size);
    byte_buffer *Result = CreateByteBuffer(A->Size);

    for(int ByteBufferIndex = 0;
        ByteBufferIndex < Result->Size;
        ByteBufferIndex++)
    {
        Result->Buffer[ByteBufferIndex] =
            A->Buffer[ByteBufferIndex] ^ B->Buffer[ByteBufferIndex];
    }

    return Result;
}

byte_buffer *
XORBufferSingleChar(byte_buffer *ByteBuffer, uint8 XORChar)
{
    byte_buffer *Result = CreateByteBuffer(ByteBuffer->Size);
    for(int ByteBufferIndex = 0;
        ByteBufferIndex < Result->Size;
        ByteBufferIndex++)
    {
        Result->Buffer[ByteBufferIndex] =
            ByteBuffer->Buffer[ByteBufferIndex] ^ XORChar;
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
ScoreBuffer(byte_buffer *ByteBuffer)
{
    real32 Result = 0;

    for(int ByteBufferIndex = 0;
        ByteBufferIndex < ByteBuffer->Size;
        ByteBufferIndex++)
    {
        Result += ScoreLetter(ByteBuffer->Buffer[ByteBufferIndex]);
    }

    Result /= ByteBuffer->Size;

    return Result;
}

struct scored_buffer
{
    byte_buffer *ByteBuffer;
    real32 Score;
};

scored_buffer
CreateEmptyScoredBuffer(void)
{
    scored_buffer Result = {0};
    return Result;
}

void
FreeScoreBuffer(scored_buffer ScoredBuffer)
{
    if(ScoredBuffer.ByteBuffer)
    {
        FreeByteBuffer(ScoredBuffer.ByteBuffer);
    }
}

scored_buffer
MaxBufferScore(scored_buffer ScoredBuffer, byte_buffer *ByteBuffer)
{
    real32 NewScore = ScoreBuffer(ByteBuffer);
    if(NewScore > ScoredBuffer.Score)
    {
        if(ScoredBuffer.ByteBuffer)
        {
            FreeByteBuffer(ScoredBuffer.ByteBuffer);
        }
        ScoredBuffer.Score = NewScore;
        ScoredBuffer.ByteBuffer = CopyByteBuffer(ByteBuffer);
    }
    return ScoredBuffer;
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

int
main(int argc, char *argv[])
{
    FillOutGlobalBase64Lookup();
    uint8 *PlainText = (uint8 *)"Burning 'em, if you ain't quick and nimble\nI go crazy when I hear a cymbal";
    uint8 *Key = (uint8 *)"ICE";
    byte_buffer *ByteBuffer = StringToByteBuffer(PlainText, 0);

    byte_buffer *XORBuffer = CreateByteBuffer(StringLength(PlainText));
    size_t KeyLength = StringLength(Key);

    for(int ByteBufferIndex = 0;
        ByteBufferIndex < ByteBuffer->Size;
        ByteBufferIndex++)
    {
        uint8 XORChar = Key[ByteBufferIndex % KeyLength];
        XORBuffer->Buffer[ByteBufferIndex] = ByteBuffer->Buffer[ByteBufferIndex] ^ XORChar;
    }
    Print(XORBuffer, BYTE_BUFFER, AS_HEX);

    if(StringsAreEqual((uint8 *)"0b3637272a2b2e63622c2e69692a23693a2a3c6324202d623d63343c2a26226324272765272a282b2f20430a652e2c652a3124333a653e2b2027630c692b20283165286326302e27282f", EncodeHex(XORBuffer)))
    {
        printf("Equal!\n");
    }
    FreeGlobalBase64Lookup();
}

void
Challenge4()
{
    uint8 *FileBuffer = 0;
    size_t FileContentsLength;
    FILE *File = fopen("data/4.txt", "r");

    if(File)
    {
        fseek(File, 0, SEEK_END);
        FileContentsLength = ftell(File);
        fseek(File, 0, SEEK_SET);
        FileBuffer = (uint8 *)malloc(sizeof(uint8) * FileContentsLength);
        if(FileBuffer)
        {
            fread(FileBuffer, 1, FileContentsLength, File);
        }
        fclose (File);
    }

    if (FileBuffer)
    {
        uint8 *FileBufferEnd = FileBuffer + FileContentsLength;
        bool32 TrailingNewLine = *(FileBufferEnd - 1) == '\n';

        // NOTE(yuri): Calculate the number of lines in the buffer
        int LinesCount = CountOccurancesInString(FileBuffer, '\n', FileContentsLength);
        if(!TrailingNewLine)
        {
            LinesCount++;
        }

        // NOTE(yuri): Allocate an appropriate 2D array
        uint8 **CipherTexts = (uint8 **)malloc(sizeof(uint8 *) * LinesCount);
        if(CipherTexts == 0) printf("Allocating CipherTexts failed\n");

        // printf("Allocated CipherTexts of size %d\n", LinesCount);

        // NOTE(yuri): Allocate arrays for strings and copy over strings
        uint8 *CurrentLinePointer = FileBuffer;
        for(int CipherTextIndex = 0;
            CipherTextIndex < LinesCount;
            CipherTextIndex++)
        {
            uint8 *NewLinePointer = SeekToChar(CurrentLinePointer, '\n', FileBufferEnd - CurrentLinePointer);
            size_t CharCount = NewLinePointer - CurrentLinePointer;
            CipherTexts[CipherTextIndex] = (uint8 *)malloc(sizeof(uint8) * (CharCount + 1));

            uint8 CharIndex = 0;
            // NOTE(yuri): Since NextLinePointer starts on the next line, we want to read up to it
            while(CharIndex < CharCount)
            {
                CipherTexts[CipherTextIndex][CharIndex] = *CurrentLinePointer;
                CurrentLinePointer++;
                CharIndex++;
            }
            CipherTexts[CipherTextIndex][CharIndex] = 0;
            CurrentLinePointer = NewLinePointer + 1;
        }

#if 0
        for(int i = 0;
            i < LinesCount;
            i++)
        {
            Print(CipherTexts[i], HEX_STRING, AS_DUMP);
        }
#endif

        scored_buffer ScoredBuffer = CreateEmptyScoredBuffer();

        for(int LineIndex = 0;
            LineIndex < LinesCount;
            LineIndex++)
        {
            byte_buffer *CandidateBuffer = DecodeHex(CipherTexts[LineIndex]);

            for(int Char = 0;
                Char < 900;
                Char++)
            {
                byte_buffer *XORBuffer = XORBufferSingleChar(CandidateBuffer, (uint8)Char);
                ScoredBuffer = MaxBufferScore(ScoredBuffer, XORBuffer);
                FreeByteBuffer(XORBuffer);
            }
            FreeByteBuffer(CandidateBuffer);
            free(CipherTexts[LineIndex]);
        }

        Print(ScoredBuffer.ByteBuffer, BYTE_BUFFER, AS_STRING);
        FreeScoreBuffer(ScoredBuffer);
        free(CipherTexts);
    }
    free(FileBuffer);
}

void
Challenge3()
{
    scored_buffer ScoredBuffer = CreateEmptyScoredBuffer();
    byte_buffer *CipherText =
        DecodeHex((uint8 *)"1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736");

    for(int Char = 0;
        Char < 255;
        Char++)
    {
        byte_buffer *XORBuffer = XORBufferSingleChar(CipherText, (uint8)Char);
        ScoredBuffer = MaxBufferScore(ScoredBuffer, XORBuffer);
        FreeByteBuffer(XORBuffer);
    }
    Print(ScoredBuffer.ByteBuffer, BYTE_BUFFER, AS_STRING);

    FreeScoreBuffer(ScoredBuffer);
    FreeByteBuffer(CipherText);
    printf("\n");
}

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

void
Challenge2()
{
    byte_buffer *A = DecodeHex((uint8 *)"1c0111001f010100061a024b53535009181c");
    byte_buffer *B = DecodeHex((uint8 *)"686974207468652062756c6c277320657965");
    Print(A, BYTE_BUFFER, AS_STRING);
    Print(B, BYTE_BUFFER, AS_STRING);
    byte_buffer *X = XORBuffers(A, B);
    Print(X, BYTE_BUFFER, AS_HEX|AS_STRING);

    FreeByteBuffer(A);
    FreeByteBuffer(B);
    FreeByteBuffer(X);
    printf("\n");
}
