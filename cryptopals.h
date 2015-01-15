#if !defined(CRYPTOPALS_H)

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

#define CRYPTOPALS_H
#endif
