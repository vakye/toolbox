
#pragma once

#if defined(_WIN32) || defined(_WIN64)
#   define PlatformWindows  (1)
#elif defined(__linux__)
#   define PlatformLinux    (1)
#else
#   error "Unknown platform"
#endif

#if !defined(PlatformWindows)
#   define PlatformWindows  (0)
#endif

#if !defined(PlatformLinux)
#   define PlatformLinux    (0)
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || (_M_X64) || defined(_M_AMD64)
#   define ArchitectureX64      (1)
#elif defined(__aarch64__) || defined(_M_ARM64)
#   define ArchitectureARM64    (1)
#elif defined(__riscv) && (__riscv_xlen == 64)
#   define ArchitectureRV64     (1)
#else
#   error "Unknown architecture"
#endif

#if !defined(ArchitectureX64)
#   define ArchitectureX64      (0)
#endif

#if !defined(ArchitectureARM64)
#   define ArchitectureARM64    (0)
#endif

#if !defined(ArchitectureRV64)
#   define ArchitectureRV64     (0)
#endif

#define local   static
#define persist static

#define CTAssert(Expression) _Static_assert(Expression, "Compile-time assertion failed")

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define OffsetOf(Structure, Member) ((usize)(&(((Structure*)0)->Member)))

#define Minimum(A, B) ((A) < (B) ? (A) : (B))
#define Maximum(A, B) ((A) > (B) ? (A) : (B))

#define AlignDown(Value, PowerOf2) ((Value) & ~((PowerOf2) - 1))
#define AlignUp(Value, PowerOf2) (((Value) + (PowerOf2) - 1) & ~((PowerOf2) - 1))

#define IsAligned(Value, PowerOf2) (((Value) & ((PowerOf2) - 1)) == 0)

#define KB(Amount) ((ssize)(Amount) << 10)
#define MB(Amount) ((ssize)(Amount) << 20)
#define GB(Amount) ((ssize)(Amount) << 30)
#define TB(Amount) ((ssize)(Amount) << 40)

typedef signed char         s8;
typedef signed short        s16;
typedef signed int          s32;
typedef signed long long    s64;

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

typedef s64 ssize;
typedef u64 usize;

typedef float f32;
typedef double f64;

typedef u8  b8;
typedef u16 b16;
typedef u32 b32;
typedef u64 b64;

CTAssert(sizeof(s8)  == 1);
CTAssert(sizeof(s16) == 2);
CTAssert(sizeof(s32) == 4);
CTAssert(sizeof(s64) == 8);

CTAssert(sizeof(u8)  == 1);
CTAssert(sizeof(u16) == 2);
CTAssert(sizeof(u32) == 4);
CTAssert(sizeof(u64) == 8);

CTAssert(sizeof(f32) == 4);
CTAssert(sizeof(f64) == 8);

CTAssert(sizeof(ssize) == sizeof(void*));
CTAssert(sizeof(usize) == sizeof(void*));

#define true  (1)
#define false (0)

#define S8Min  ((s8 )(-128))
#define S16Min ((s16)(-32768))
#define S32Min ((s32)(-2147483648))
#define S64Min ((s64)(-9223372036854775808ll))

#define S8Max  ((s8 )(+127))
#define S16Max ((s16)(+32767))
#define S32Max ((s32)(+2147483647))
#define S64Max ((s64)(+9223372036854775807ll))

#define U8Max  ((u8 )(+255))
#define U16Max ((u16)(+65535))
#define U32Max ((u32)(+4294967295))
#define U64Max ((u64)(+18446744073709551615ull))

#define SSizeMin S64Min
#define SSizeMax S64Max
#define USizeMax U64Max

void* memset(void* DestInit, s32 Byte, usize Size)
{
    u8* Dest = (u8*)DestInit;
    while (Size--)
        *Dest++ = 0;

    return (DestInit);
}

void* memcpy(void* DestInit, void* SourceInit, usize Size)
{
    u8* Dest = (u8*)DestInit;
    u8* Source = (u8*)SourceInit;
    while (Size--)
        *Dest++ = *Source++;

    return (DestInit);
}

local void ZeroMemory(void* DestInit, usize Size)                   { memset(DestInit, 0, Size); }
local void FillMemory(void* DestInit, u8 Byte, usize Size)          { memset(DestInit, 0, Size); }
local void CopyMemory(void* DestInit, void* SourceInit, usize Size) { memcpy(DestInit, SourceInit, Size); }

#define ZeroType(Pointer)           ZeroMemory(Pointer, sizeof(*(Pointer)))
#define ZeroArray(Pointer, Count)   ZeroMemory(Pointer, sizeof(*(Pointer)) * (Count))

typedef struct
{
    char* Data;
    usize Size;
} string;

#define Str(Literal)            (string){Literal, sizeof(Literal) - 1}
#define StrData(Data, Size)     (string){Data, Size}

