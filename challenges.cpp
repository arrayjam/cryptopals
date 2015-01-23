#include "cryptopals.cpp"
#include "challenges.h"

void
Challenges(void)
{
    AES();
    // Challenge1();
    // Challenge2();
    // Challenge3();
    // Challenge4();
    // Challenge5();
    // Challenge6();
}

int
main(int argc, char *argv[])
{
    Initialize();
    Challenges();
    Terminate();
}

// Break repeating-key XOR
void
Challenge6()
{
    byte_buffer FileBuffer = OpenFileBuffer((uint8 *)"data/6.txt");
    byte_buffer ByteBuffer = ReadFileAsWrappedBase64String(FileBuffer);
    FreeFileBuffer(FileBuffer);

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
    uint8 *PlainText = (uint8 *)"Burning 'em, if you ain't quick and nimble\nI go crazy when I hear a cymbal";
    uint8 *Key = (uint8 *)"ICE";
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
