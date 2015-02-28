#include "cryptopals.cpp"
#include "challenges.h"

void
Challenges(void)
{
    // AESAllTests();
    // Challenge1();
    // Challenge2();
    // Challenge3();
    // Challenge4();
    // Challenge5();
    // Challenge6();
    // Challenge7();
    // Challenge8();
    // Challenge9();
    // Challenge10();
    // Challenge11();
    Challenge12();
}

int
main(int argc, char *argv[])
{
    Initialize();
    Challenges();
    Terminate();
}

int
CountEqualBlocks(byte_buffer ByteBuffer, size_t BlockSize)
{
    int Result = 0;
    byte_buffers Blocks = ChunkBuffer(ByteBuffer, BlockSize);
    for(int BlockAIndex = 0;
        BlockAIndex < Blocks.Size - 1;
        ++BlockAIndex)
    {
        for(int BlockBIndex = BlockAIndex + 1;
            BlockBIndex < Blocks.Size;
            ++BlockBIndex)
        {
            byte_buffer BlockA = Blocks.Buffers[BlockAIndex];
            byte_buffer BlockB = Blocks.Buffers[BlockBIndex];
            if(ByteBuffersEqual(BlockA, BlockB))
            {
                Result++;
            }
        }
    }

    FreeByteBuffers(Blocks);

    return Result;
}

int
BadRandomNumberBetween(int Min, int Max)
{
    assert(Max > Min);
    return (rand() % (Max - Min + 1)) + Min;
}

byte_buffer
GenerateRandomByteBuffer(size_t Size)
{
    byte_buffer ByteBuffer = CreateByteBuffer(Size);
    for(int ByteBufferIndex = 0;
        ByteBufferIndex < ByteBuffer.Size;
        ++ByteBufferIndex)
    {
        ByteBuffer.Buffer[ByteBufferIndex] = BadRandomNumberBetween(0, 255);
    }

    return ByteBuffer;
}

byte_buffer
RandomAESKey(void)
{
    return GenerateRandomByteBuffer(AES_BLOCK_SIZE);
}

byte_buffer
EncryptionOracle(byte_buffer Input)
{
    byte_buffer Prepend = GenerateRandomByteBuffer(BadRandomNumberBetween(5, 10));
    byte_buffer Prepended = CatBuffers(Prepend, Input, true);

    byte_buffer Append = GenerateRandomByteBuffer(BadRandomNumberBetween(5, 10));
    byte_buffer ByteBuffer = CatBuffers(Prepended, Append, true);
    FreeByteBuffer(Append);

    int ChooseCBC = BadRandomNumberBetween(0, 1);
    byte_buffer KeyBuffer = RandomAESKey();
    byte_buffer Result;

    if(ChooseCBC)
    {
        byte_buffer IV = GenerateRandomByteBuffer(16);
        Result = AESEncryptCBC(ByteBuffer, KeyBuffer, IV);
    }
    else
    {
        Result = AESEncryptECB(ByteBuffer, KeyBuffer);
    }
    FreeByteBuffer(ByteBuffer);
    FreeByteBuffer(KeyBuffer);

    return Result;
}

byte_buffer
AESOracle(byte_buffer PlainText, byte_buffer SecretText, byte_buffer KeyBuffer)
{
    byte_buffer ByteBuffer  = CatBuffers(PlainText, SecretText);
    byte_buffer Result = AESEncryptECB(ByteBuffer, KeyBuffer);

    return Result;
}

int
DetectBlockSize(byte_buffer SecretBuffer, byte_buffer KeyBuffer)
{
    int Result = 0;
    int LastCipherTextSize = 0;

    for(int BlockSize = 0;
        BlockSize < 256;
        ++BlockSize)
    {
        byte_buffer PlainText = CreateByteBuffer(BlockSize);
        PlainText = FillBuffer(PlainText, 'X');

        byte_buffer CipherText = AESOracle(PlainText, SecretBuffer, KeyBuffer);

        if(LastCipherTextSize &&
           ((CipherText.Size - LastCipherTextSize) > 0))
        {
            Result = CipherText.Size - LastCipherTextSize;
            FreeByteBuffer(CipherText);
            FreeByteBuffer(PlainText);
            break;
        }
        LastCipherTextSize = CipherText.Size;
        FreeByteBuffer(CipherText);
        FreeByteBuffer(PlainText);
    }

    return Result;
}

bool32
DetectECB(byte_buffer SecretBuffer, byte_buffer KeyBuffer, size_t BlockSize)
{
    bool32 Result = false;

    byte_buffer PlainText = CreateByteBuffer(1024);
    PlainText = FillBuffer(PlainText, 0);

    byte_buffer CipherText = AESOracle(PlainText, SecretBuffer, KeyBuffer);

    int EqualBlocks = CountEqualBlocks(CipherText, BlockSize);
    if(EqualBlocks > 1)
    {
        Result = true;
    }

    FreeByteBuffer(CipherText);
    FreeByteBuffer(PlainText);

    return Result;
}

void
Challenge12()
{
    byte_buffer SecretBuffer = FileToBase64Buffer("data/12.txt");
    byte_buffer KeyBuffer = RandomAESKey();

    int BlockSize = DetectBlockSize(SecretBuffer, KeyBuffer);
    printf("DetectedBlockSize: %d\n", BlockSize);

    int ECBMode = DetectECB(SecretBuffer, KeyBuffer, BlockSize);

    if(ECBMode)
    {
        printf("Detected ECB mode\n");
    }

}

// An ECB/CBC detection oracle
void
Challenge11()
{
    byte_buffer EasyBuffer = CreateByteBuffer(1024);
    EasyBuffer = FillBuffer(EasyBuffer, 0);
    byte_buffer CipherText = EncryptionOracle(EasyBuffer);
    Print(&CipherText, BYTE_BUFFER, AS_NICE_STRING);

    int EqualBlocks = CountEqualBlocks(CipherText, AES_BLOCK_SIZE);
    printf("%d Equal blocks found\n", EqualBlocks);
    if(EqualBlocks > 1)
    {
        printf("ECB Mode detected.\n");
    }
    else
    {
        printf("CBC Mode detected.\n");
    }
    FreeByteBuffer(EasyBuffer);
    FreeByteBuffer(CipherText);
}

// Implement CBC mode
void
Challenge10()
{
    byte_buffer ByteBuffer = FileToBase64Buffer("data/10.txt");
    byte_buffer KeyBuffer = StringToByteBuffer("YELLOW SUBMARINE", 0);
    byte_buffer IV = CreateByteBuffer(AES_BLOCK_SIZE);
    // Print(&ByteBuffer, BYTE_BUFFER, AS_DUMP);
    IV = FillBuffer(IV, 0);

    byte_buffer PlainText = AESDecryptCBC(ByteBuffer, KeyBuffer, IV);

    Print(&PlainText, BYTE_BUFFER, AS_NICE_STRING);

    FreeByteBuffer(ByteBuffer);
    FreeByteBuffer(KeyBuffer);
    FreeByteBuffer(PlainText);
}

// Implement PKCS#7 padding
void
Challenge9()
{
    byte_buffer ByteBuffer = StringToByteBuffer("YELLOW SUBMARINE", 0);
    byte_buffer Padded = PKCS7PaddedBuffer(ByteBuffer, 20);

    Print(&ByteBuffer, BYTE_BUFFER, AS_STRING);
    Print(&Padded, BYTE_BUFFER, AS_HEX_STRING);

    FreeByteBuffer(ByteBuffer);
    FreeByteBuffer(Padded);
}

// Detect AES in ECB mode
void
Challenge8()
{
    string_buffers StringsBuffer = FileToStringBuffers("data/8.txt");

    byte_buffer BestBuffer;
    int MaxEqualBlocksCount = 0;
    int MaxEqualBlocksIndex = 0;
    for(int StringIndex = 0;
        StringIndex < StringsBuffer.Size;
        ++StringIndex)
    {
        // Print(StringsBuffer.Strings[StringIndex], STRING, AS_NICE_STRING);
        byte_buffer CipherTextBuffer = DecodeHex(StringsBuffer.Strings[StringIndex]);
        int EqualBlocksCount = CountEqualBlocks(CipherTextBuffer, AES_BLOCK_SIZE);

        if(EqualBlocksCount > MaxEqualBlocksCount)
        {
            printf("New top found at index %d, from %d to %d\n", StringIndex, MaxEqualBlocksCount, EqualBlocksCount);
            // FreeByteBuffer(BestBuffer);
            BestBuffer = CopyByteBuffer(CipherTextBuffer);
            MaxEqualBlocksCount = EqualBlocksCount;
            MaxEqualBlocksIndex = StringIndex;
        }
        FreeByteBuffer(CipherTextBuffer);
    }

    FreeStringBuffers(StringsBuffer);

    printf("Found %d Equal blocks at string %d\n", MaxEqualBlocksCount, MaxEqualBlocksIndex);

    // byte_buffers BestBuffers = ChunkBuffer(BestBuffer, AES_BLOCK_SIZE);
    // Print(&BestBuffers, BYTE_BUFFERS, AS_HEX);
    FreeByteBuffer(BestBuffer);
}

// Decrypt ECB AES
void
Challenge7()
{
    byte_buffer ByteBuffer = FileToBase64Buffer("data/7.txt");
    byte_buffer KeyBuffer = StringToByteBuffer("YELLOW SUBMARINE", 0);
    byte_buffer PlainTextBuffer = AESDecryptECB(ByteBuffer, KeyBuffer);
    Print(&PlainTextBuffer, BYTE_BUFFER, AS_STRING);

    FreeByteBuffer(ByteBuffer);
    FreeByteBuffer(KeyBuffer);
    FreeByteBuffer(PlainTextBuffer);
}

// Break repeating-key XOR
void
Challenge6()
{
    byte_buffer ByteBuffer = FileToBase64Buffer("data/6.txt");

    real32 SmallestEditDistance = 10000;
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

    int PaddedSize = BlockPaddedSize(ByteBuffer.Size, KeySizeGuess);
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

    byte_buffer PlainText = RepeatingByteBufferXOR(ByteBuffer, KeyByteBuffer);
    Print(&PlainText, BYTE_BUFFER, AS_STRING);
    FreeByteBuffer(PlainText);

    FreeByteBuffer(KeyByteBuffer);
    FreeByteBuffer(ByteBuffer);
}

// Implement repeating-key XOR
void
Challenge5()
{
    const char *PlainText = "Burning 'em, if you ain't quick and nimble\nI go crazy when I hear a cymbal";
    const char *Key = "ICE";
    byte_buffer PlainTextByteBuffer = StringToByteBuffer(PlainText, 0);
    byte_buffer KeyByteBuffer = StringToByteBuffer(Key, 0);
    byte_buffer Ciphered = RepeatingByteBufferXOR(PlainTextByteBuffer, KeyByteBuffer);

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
    byte_buffer FileBuffer = OpenFileBuffer("data/4.txt");

    if(FileBuffer.Buffer)
    {
        string_buffers StringBuffers = ReadFileIntoStringBuffers(FileBuffer);
        scored_buffer ScoredBuffer = CreateEmptyScoredBuffer();

        for(int StringIndex = 0;
            StringIndex < StringBuffers.Size;
            StringIndex++)
        {
            byte_buffer ByteBuffer = DecodeHex(StringBuffers.Strings[StringIndex]);
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
    byte_buffer X = ByteBufferXOR(A, B);
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
