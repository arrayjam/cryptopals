#include "cryptopals.h"

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

base64_lookup GlobalBase64Lookup = {};
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
ByteBufferXOR(byte_buffer A, byte_buffer B)
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
SingleCharacterByteBufferXOR(byte_buffer ByteBuffer, uint8 XORChar)
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
RepeatingByteBufferXOR(byte_buffer ByteBuffer, byte_buffer Key)
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
        byte_buffer XORBuffer = SingleCharacterByteBufferXOR(ByteBuffer, Key.Buffer[0]);
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
        KeySizeBufferA.Buffer[KeySizeIndex] = ByteBuffer.Buffer[KeySizeIndex];
        KeySizeBufferB.Buffer[KeySizeIndex] = ByteBuffer.Buffer[KeySize + KeySizeIndex];
        KeySizeBufferC.Buffer[KeySizeIndex] = ByteBuffer.Buffer[KeySize*2 + KeySizeIndex];
        KeySizeBufferD.Buffer[KeySizeIndex] = ByteBuffer.Buffer[KeySize*3 + KeySizeIndex];
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

void
PrintState(uint8 State[4][4])
{
    for(int Row = 0;
        Row < 4;
        ++Row)
    {
        for(int Column = 0;
            Column < 4;
            ++Column)
        {
            printf("%02x\t", State[Row][Column]);
        }
        printf("\n");
    }
    printf("\n");
}

union word
{
    struct
    {
        uint32 Word;
    };
    uint8 Byte[4];
};

uint8
SubByte(uint8 Byte)
{
    uint8 Sbox[16][16] =
    {
        0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
        0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
        0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
        0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
        0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
        0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
        0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
        0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
        0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
        0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
        0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
        0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
        0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
        0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
        0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
        0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
    };
    uint8 Result = 0;

    Result = Sbox[Byte >> 4][Byte & 0x0f];

    return Result;
}

uint8 *
ShiftRow(uint8 Row[4], int Shift)
{
    word Word = {0};
    for(int Column = 0;
        Column < 4;
        ++Column)
    {
        Word.Byte[3 - Column] = Row[Column];
    }

    Word.Word = ((Word.Word << (Shift * 8)) |
                 (Word.Word >> (32 - (Shift * 8))));

    for(int Column = 0;
        Column < 4;
        ++Column)
    {
        Row[Column] = Word.Byte[3 - Column];
    }

    return Row;
}

word
SubWord(word Word)
{
    for(int ByteIndex = 0;
        ByteIndex < 4;
        ++ByteIndex)
    {
        Word.Byte[ByteIndex] = SubByte(Word.Byte[ByteIndex]);
    }

    return Word;
}

void
AES(void)
{
    uint8 InputBytes[16] = {
        0x32, 0x43, 0xf6, 0xa8,
        0x88, 0x5a, 0x30, 0x8d,
        0x31, 0x31, 0x98, 0xa2,
        0xe0, 0x37, 0x07, 0x34
    };

    uint8 Key[16] = {
        0x2b, 0x7e, 0x15, 0x16,
        0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88,
        0x09, 0xcf, 0x4f, 0x3c
    };

    // uint8 Key[24] = {
    //     0x8e, 0x73, 0xb0, 0xf7,
    //     0xda, 0x0e, 0x64, 0x52,
    //     0xc8, 0x10, 0xf3, 0x2b,
    //     0x80, 0x90, 0x79, 0xe5,
    //     0x62, 0xf8, 0xea, 0xd2,
    //     0x52, 0x2c, 0x6b, 0x7b
    // };

    // uint8 Key[32] = {
    //     0x60, 0x3d, 0xeb, 0x10,
    //     0x15, 0xca, 0x71, 0xbe,
    //     0x2b, 0x73, 0xae, 0xf0,
    //     0x85, 0x7d, 0x77, 0x81,
    //     0x1f, 0x35, 0x2c, 0x07,
    //     0x3b, 0x61, 0x08, 0xd7,
    //     0x2d, 0x98, 0x10, 0xa3,
    //     0x09, 0x14, 0xdf, 0xf4
    // };

    int Nb = 4; // Number of 32-bit words in Plaintext

    size_t KeySize = ArrayCount(Key);

    int Nr = 0; // Number of rounds;
    int Nk = KeySize / 4; // Number of columns
    if(KeySize * 8 == 128)
    {
        Nr = 10;
    }
    else if(KeySize * 8 == 192)
    {
        Nr = 12;
    }
    else if(KeySize * 8 == 256)
    {
        Nr = 14;
    }

    assert(ArrayCount(InputBytes) * 8 == 128);
    assert(Nb == 4);
    assert(Nr == 10 || Nr == 12 || Nr == 14);
    assert(Nk == 4 || Nk == 6 || Nk == 8);

    // NOTE(yuri): Row, Column
    uint8 State[4][4];

    for(int Row = 0;
        Row < 4;
        ++Row)
    {
        for(int Column = 0;
            Column < 4;
            ++Column)
        {
            State[Row][Column] = InputBytes[Row + (4 * Column)];
        }
    }

    PrintState(State);

    int ExpandedKeyLength = Nb * (Nr + 1);
    printf("ExpandedKeyLength: %d\n", ExpandedKeyLength);

    word *ExpandedKey = (word *)malloc(sizeof(word) * (Nb * (Nr + 1)));

    word Temp;

    int KeyExpansionIndex = 0;
    uint8 Rcon[] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

    while(KeyExpansionIndex < Nk)
    {
        printf("KeyExpansionIndex: %d\n", KeyExpansionIndex);
        ExpandedKey[KeyExpansionIndex].Word = ((Key[(4*KeyExpansionIndex) + 0] << 24) |
                                               (Key[(4*KeyExpansionIndex) + 1] << 16) |
                                               (Key[(4*KeyExpansionIndex) + 2] << 8) |
                                               (Key[(4*KeyExpansionIndex) + 3] << 0));
        printf("ExpandedKey word: %08x\n", ExpandedKey[KeyExpansionIndex].Word);
        KeyExpansionIndex++;
    }

    KeyExpansionIndex = Nk;

    while(KeyExpansionIndex < ExpandedKeyLength)
    {
        printf("i: %d\n", KeyExpansionIndex);
        Temp = ExpandedKey[KeyExpansionIndex - 1];
        printf("Temp: %08x\n", Temp.Word);

        if(KeyExpansionIndex % Nk == 0)
        {
            Temp.Word = (Temp.Word << 8) | (Temp.Word >> 24);
            printf("After RotWord: %08x\n", Temp.Word);

            Temp = SubWord(Temp);
            printf("After SubWord: %08x\n", Temp.Word);

            printf("Rcon[i/Nk]: %08x\n", (Rcon[KeyExpansionIndex / Nk] << 24));

            Temp.Word ^= (Rcon[KeyExpansionIndex / Nk] << 24);
            printf("After XOR with Rcon: %08x\n", Temp.Word);
        }
        else if(Nk > 6 && (KeyExpansionIndex % Nk) == 4)
        {
            Temp = SubWord(Temp);
            printf("After SubWord: %08x\n", Temp.Word);
        }

        ExpandedKey[KeyExpansionIndex].Word = ExpandedKey[KeyExpansionIndex - Nk].Word ^ Temp.Word;
        printf("After end XOR: %08x\n", ExpandedKey[KeyExpansionIndex].Word);
        KeyExpansionIndex++;
        printf("\n\n");
    }

    for(int ExpandedIndex = 0;
        ExpandedIndex < ExpandedKeyLength;
        ++ExpandedIndex)
    {
        printf("%08x\n", ExpandedKey[ExpandedIndex].Word);
    }

    PrintState(State);

    printf("Initial AddRoundKey: START\n");
    int Round = 0;
    for(int Column = 0;
        Column < 4;
        ++Column)
    {
        for(int Row = 0;
            Row < 4;
            ++Row)
        {

            uint8 OldState = State[Row][Column];
            uint8 XORByte = ExpandedKey[Column].Byte[3 - Row];
            uint8 NewState = OldState ^ XORByte;
            // printf("XORByte Index: %d, Row: %d\n", ExpandedKeyIndex + Column, Row);

            printf("OldState: %02x\tXORByte: %02x\tNewState: %02x\n", OldState, XORByte, NewState);
            State[Row][Column] = NewState;
        }
    }

    printf("Initial AddRoundKey: DONE\n");
    PrintState(State);

    int ExpandedKeyIndex;
    for(Round = 1;
        Round < 2; //Nr - 1;
        ++Round)
    {
        ExpandedKeyIndex = Round * Nb;
        for(int Row = 0;
            Row < 4;
            ++Row)
        {
            for(int Column = 0;
                Column < 4;
                ++Column)
            {
                State[Row][Column] = SubByte(State[Row][Column]);
            }
        }

        printf("After SubBytes Round Number: %d\n", Round);
        PrintState(State);

        for(int Row = 0;
            Row < 4;
            ++Row)
        {
            ShiftRow(State[Row], Row);
        }
        printf("After ShiftRow Round Number: %d\n", Round);
        PrintState(State);
    }
}

void
Initialize(void)
{
    FillOutGlobalBase64Lookup();
}

void
Terminate(void)
{
    FreeGlobalBase64Lookup();
}
