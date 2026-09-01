
#pragma once

#include "shared.c"

typedef usize wall_clock;

local wall_clock    GetClockNow         (void);
local wall_clock    ClockElapsedSince   (wall_clock Reference);
local wall_clock    ClockDifference     (wall_clock From, wall_clock To);
local f64           ClockToSeconds      (wall_clock Clock);
local void          WaitNanoseconds     (usize Nanoseconds);
local usize         WriteStdOut         (void* Data, usize Size);
local usize         WriteStdErr         (void* Data, usize Size);
local void*         ReserveMemory       (usize Size);
local b32           CommitMemory        (void* Memory, usize Size);
local void          ReleaseMemory       (void* Memory, usize ReservedSize);
local void          Exit                (u8 ExitCode);

// NOTE(vak): Implementation

#if PlatformWindows
#   error "platform.c does not support Windows at the moment"
#elif PlatformLinux

local void Main(void);

__attribute__((force_align_arg_pointer))
void EntryPoint(void)
{
    Main();
    Exit(0);
}

typedef enum
{
#if ArchitectureX64
    LinuxSyscallNR_Write            = (1),
    LinuxSyscallNR_MMap             = (9),
    LinuxSyscallNR_MProtect         = (10),
    LinuxSyscallNR_MUnMap           = (11),
    LinuxSyscallNR_Nanosleep        = (35),
    LinuxSyscallNR_Exit             = (60),
    LinuxSyscallNR_Clock_GetTime    = (228),
    LinuxSyscallNR_Clock_GetRes     = (229),
#else
#   error Linux syscall numbers are not defined for this architecture yet.
#endif
} linux_syscall_nr;

typedef struct
{
    u64 Seconds;
    u64 Nanoseconds;
} linux_timespec;

#define CLOCK_MONOTONIC (0)

#define STDOUT_FILENO   (1)
#define STDERR_FILENO   (2)

#define PROT_NONE       (0x00)
#define PROT_READ       (0x01)
#define PROT_WRITE      (0x02)
#define PROT_EXEC       (0x04)

#define MAP_PRIVATE     (0x02)
#define MAP_ANONYMOUS   (0x20)

local usize LinuxSyscall(
    linux_syscall_nr SyscallNumber,
    usize Arg0, usize Arg1, usize Arg2,
    usize Arg3, usize Arg4, usize Arg5
)
{
    usize Result = 0;

#if ArchitectureX64
    register usize R10 __asm__("r10") = Arg3;
    register usize R8  __asm__("r8")  = Arg4;
    register usize R9  __asm__("r9")  = Arg5;

    __asm__ volatile (
        "syscall" :
        "=a"(Result) :
        "a"(SyscallNumber),
        "D"(Arg0),
        "S"(Arg1),
        "d"(Arg2),
        "r"(R10),
        "r"(R8),
        "r"(R9) :
        "memory", "rcx", "r11"
    );
#else
#   error Linux syscall is not implemented for this architecture yet.
#endif

    return (Result);
}

local wall_clock GetClockNow(void)
{
    linux_timespec Now = {0};
    LinuxSyscall(
        LinuxSyscallNR_Clock_GetTime,
        CLOCK_MONOTONIC,
        (usize)&Now,
        0, 0, 0, 0
    );

    u64 MaskedSeconds = Now.Seconds     & 0xFFFFFFFF;
    u64 Nanoseconds   = Now.Nanoseconds & 0xFFFFFFFF;

    wall_clock Result = (MaskedSeconds << 32) | Nanoseconds;
    return (Result);
}

local wall_clock ClockElapsedSince(wall_clock Reference)
{
    return ClockDifference(Reference, GetClockNow());
}

local wall_clock ClockDifference(wall_clock From, wall_clock To)
{
    wall_clock Result = To - From;
    return (Result);
}

local f64 ClockToSeconds(wall_clock Clock)
{
    u32 Seconds     = (Clock >> 32);
    u32 Nanoseconds = (Clock & 0xFFFFFFFF);
    f64 Result      = (f64)Seconds + (Nanoseconds * 1e-9);

    return (Result);
}

local void WaitNanoseconds(usize Nanoseconds)
{
    linux_timespec Duration =
    {
        .Seconds     = Nanoseconds / 1000000000,
        .Nanoseconds = Nanoseconds % 1000000000,
    };

    linux_timespec Remainder = {0};

    do
    {
        LinuxSyscall(
            LinuxSyscallNR_Nanosleep,
            (usize)&Duration,
            (usize)&Remainder,
            0, 0, 0, 0
        );
    } while (Remainder.Seconds || Remainder.Nanoseconds);
}

local usize WriteStdOut(void* Data, usize Size)
{
    ssize WriteResult = LinuxSyscall(
        LinuxSyscallNR_Write,
        STDOUT_FILENO,
        (usize)Data,
        Size,
        0, 0, 0
    );

    usize BytesWritten = Maximum(0, WriteResult);
    return (BytesWritten);
}

local usize WriteStdErr(void* Data, usize Size)
{
    ssize WriteResult = LinuxSyscall(
        LinuxSyscallNR_Write,
        STDERR_FILENO,
        (usize)Data,
        Size,
        0, 0, 0
    );

    usize BytesWritten = Maximum(0, WriteResult);
    return (BytesWritten);
}

local void* ReserveMemory(usize Size)
{
    ssize MapResult = LinuxSyscall(
        LinuxSyscallNR_MMap,
        0, Size, PROT_NONE,
        MAP_PRIVATE|MAP_ANONYMOUS,
        -1, 0
    );

    void* Result = (void*)Maximum(0, MapResult);
    return (Result);
}

local b32 CommitMemory(void* Memory, usize Size)
{
    ssize CommitResult = LinuxSyscall(
        LinuxSyscallNR_MProtect,
        (usize)Memory,
        Size,
        PROT_READ|PROT_WRITE,
        0, 0, 0
    );

    b32 Okay = (CommitResult >= 0);
    return (Okay);
}

local void ReleaseMemory(void* Memory, usize ReservedSize)
{
    LinuxSyscall(
        LinuxSyscallNR_MUnMap,
        (usize)Memory,
        ReservedSize,
        0, 0, 0, 0
    );
}

local void Exit(u8 ExitCode)
{
    LinuxSyscall(LinuxSyscallNR_Exit, ExitCode, 0, 0, 0, 0, 0);
}

#else
#   error "platform.c does not recognize this platform"
#endif

