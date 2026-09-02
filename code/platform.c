
#pragma once

#include "shared.c"

typedef usize wall_clock;

typedef u32 thread_id;
#define NilThreadID (0)

typedef s32 thread_callback(void* UserData);

typedef struct
{
    thread_callback*    Callback;           // NOTE(vak): Function that will be called when the thread spawns.
    void*               UserData;           // NOTE(vak): User data that will be passed to the callback.
    void*               StackBase;          // NOTE(vak): Base address of stack space. Must be aligned on a 64 byte boundary.
    usize               StackSize;          // NOTE(vak): Size of stack space. Must be equal to or larger than 1KB. Must be aligned on a 64 byte boundary.
} spawn_thread_info;

local wall_clock    GetClockNow         (void);                                         // NOTE(vak): Get current value of the system wall clock.
local wall_clock    ClockElapsedSince   (wall_clock Reference);                         // NOTE(vak): Get the amount of time elapsed since the `Reference` point.
local wall_clock    ClockDifference     (wall_clock From, wall_clock To);               // NOTE(vak): Get the amount of time elapsed between two different points. `From` is assumed to be less than `To`.
local f64           ClockToSeconds      (wall_clock Clock);                             // NOTE(vak): Convert system wall clock value to seconds.
local void          WaitNanoseconds     (usize Nanoseconds);                            // NOTE(vak): Suspends calling thread with the duration being the specified `Nanoseconds`.
local usize         ReadFileToBuffer    (string FilePath, void* Buffer, usize Size);    // NOTE(vak): If Buffer=0, returns file size, otherwise read until there is nothing left or buffer has ran out then return the number of bytes read.
local usize         WriteStdOut         (void* Data, usize Size, ...);                  // NOTE(vak): Writes to stdout. Returns number of bytes written.
local usize         WriteStdErr         (void* Data, usize Size, ...);                  // NOTE(vak): Writes to sdterr. Returns number of bytes written.
local void*         ReserveMemory       (usize Size);                                   // NOTE(vak): Reserve a virtual address space whose size is equal to or larger than 'Size'. Returns 0 if failed, otherwise returns the base address.
local b32           CommitMemory        (void* Memory, usize Size);                     // NOTE(vak): Makes a virtual address range usable by mapping it to physical memory. Returns true on success, and false on failure.
local void*         ReserveAndCommit    (usize Size);                                   // NOTE(vak): Reserves and commits a region of memory whose size is equal to or larger than the specified `Size`.
local void          ReleaseMemory       (void* Memory, usize ReservedSize);             // NOTE(vak): Releases a virtual address space and all of its associated physical memory back to the system.
local thread_id     SpawnThread         (spawn_thread_info* Info);                      // NOTE(vak): Spawns a new thread with the specified options.
local s32           JoinThread          (thread_id ThreadID);                           // NOTE(vak): Waits for thread to exit. Returns exit code of thread.
local void          ThreadExit          (u8 ExitCode);                                  // NOTE(vak): Exits calling thread with `ExitCode`
local void          ProcessExit         (u8 ExitCode);                                  // NOTE(vak): Exits entire process with `ExitCode`

#define Exit(ExitCode) ThreadExit(ExitCode)

// NOTE(vak): Implementation

#if PlatformWindows
#   error "platform.c does not support Windows at the moment"
#elif PlatformLinux

local s32 Main(void);

__attribute__((force_align_arg_pointer))
void EntryPoint(void)
{
    s32 Returned = Main();
    ProcessExit(Returned);
}

typedef enum
{
#if ArchitectureX64
    LinuxSyscallNR_Read             = (0),
    LinuxSyscallNR_Write            = (1),
    LinuxSyscallNR_Open             = (2),
    LinuxSyscallNR_Close            = (3),
    LinuxSyscallNR_LSeek            = (8),
    LinuxSyscallNR_MMap             = (9),
    LinuxSyscallNR_MProtect         = (10),
    LinuxSyscallNR_MUnMap           = (11),
    LinuxSyscallNR_Nanosleep        = (35),
    LinuxSyscallNR_Exit             = (60),
    LinuxSyscallNR_Futex            = (202),
    LinuxSyscallNR_Clock_GetTime    = (228),
    LinuxSyscallNR_Clock_GetRes     = (229),
    LinuxSyscallNR_ExitGroup        = (231),
#else
#   error Linux syscall numbers are not defined for this architecture yet.
#endif
} linux_syscall_nr;

typedef struct
{
    u64 Seconds;
    u64 Nanoseconds;
} linux_timespec;

typedef struct
{
    void*               ThreadEntryAddress; // NOTE(vak): Always positioned first so ret instruction can pop this off.
    thread_callback*    Callback;
    void*               UserData;
    thread_id           ThreadID;
    u32                 Padding;
} linux_stack_header;

CTAssert(IsAligned(sizeof(linux_stack_header), 16));

typedef struct
{
    s32                 JoinFutex;
    s32                 Returned;
} linux_thread;

#define LinuxMaxThreadSpawnCount (4096)
#define LinuxIsValidThreadID(ThreadID) (((ThreadID) > 0) && ((ThreadID) < LinuxMaxThreadSpawnCount))

local linux_thread  LinuxAllThreads[LinuxMaxThreadSpawnCount]   = {0};
local u32           LinuxThreadCount                            = 1;

#define CLOCK_MONOTONIC (0)

#define O_RDONLY        (0)
#define SEEK_END        (2)

#define STDOUT_FILENO   (1)
#define STDERR_FILENO   (2)

#define PROT_NONE       (0x00)
#define PROT_READ       (0x01)
#define PROT_WRITE      (0x02)
#define PROT_EXEC       (0x04)

#define MAP_PRIVATE     (0x02)
#define MAP_ANONYMOUS   (0x20)

#define FUTEX_WAIT      (0)
#define FUTEX_WAKE      (1)

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

local usize ReadFileToBuffer(string FilePath, void* Buffer, usize Size)
{
    // NOTE(vak): `FilePath` string may not have a null terminator.
    static char PathBuffer[KB(4)] = {0};

    if (FilePath.Size >= sizeof(PathBuffer))
        return (0);

    CopyMemory(PathBuffer, FilePath.Data, FilePath.Size);
    PathBuffer[FilePath.Size] = '\0';

    s32 FileDescriptor = (s32)LinuxSyscall(
        LinuxSyscallNR_Open,
        (usize)PathBuffer, O_RDONLY,
        0, 0, 0, 0
    );

    if (FileDescriptor < 0)
        return (0);

    usize Result = 0;

    if (Buffer == 0)
    {
        ssize FileSize = (ssize)LinuxSyscall(
            LinuxSyscallNR_LSeek,
            FileDescriptor, 0, SEEK_END,
            0, 0, 0
        );

        Result = (usize)FileSize;
    }
    else
    {
        ssize BytesRead = (ssize)LinuxSyscall(
            LinuxSyscallNR_Read,
            FileDescriptor, (usize)Buffer, Size,
            0, 0, 0
        );

        Result = Maximum(0, BytesRead);
    }

    LinuxSyscall(
        LinuxSyscallNR_Close,
        FileDescriptor,
        0, 0, 0, 0, 0
    );

    return (Result);
}

local usize WriteStdOut(void* Data, usize Size, ...)
{
    ssize WriteResult = (ssize)LinuxSyscall(
        LinuxSyscallNR_Write,
        STDOUT_FILENO,
        (usize)Data,
        Size,
        0, 0, 0
    );

    usize BytesWritten = Maximum(0, WriteResult);
    return (BytesWritten);
}

local usize WriteStdErr(void* Data, usize Size, ...)
{
    ssize WriteResult = (ssize)LinuxSyscall(
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
    ssize MapResult = (ssize)LinuxSyscall(
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
    ssize CommitResult = (ssize)LinuxSyscall(
        LinuxSyscallNR_MProtect,
        (usize)Memory,
        Size,
        PROT_READ|PROT_WRITE,
        0, 0, 0
    );

    b32 Okay = (CommitResult >= 0);
    return (Okay);
}

local void* ReserveAndCommit(usize Size)
{
    void* Result = ReserveMemory(Size);
    if (!Result)
        return (0);

    if (!CommitMemory(Result, Size))
    {
        ReleaseMemory(Result, Size);
        return (0);
    }

    return (Result);
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

local void LinuxThreadEntry(linux_stack_header* Header)
{
    // TODO(vak): Investigate if `Header` might actually get overwritten because of
    // the `red zone` used for storing register arguments.. ?

    thread_id ThreadID = Header->ThreadID;
    linux_thread* Thread = LinuxAllThreads + ThreadID;

    s32 Returned = Header->Callback(Header->UserData);

    __atomic_store_n(&Thread->JoinFutex, 1, __ATOMIC_SEQ_CST);

    LinuxSyscall(
        LinuxSyscallNR_Futex,
        (usize)&Thread->JoinFutex,
        FUTEX_WAKE,
        S32Max,
        0, 0, 0
    );

    // TODO(vak): Do we need a write barrier here.. ??

    Thread->Returned = Returned;

    ThreadExit(Returned);
}

__attribute__((naked))
local ssize LinuxPerformClone(linux_stack_header* Header)
{
#if ArchitectureX64
    __asm__ volatile (
        "mov    %rdi,       %rsi\n"         // NOTE(vak): rsi = stack = Header
        "mov    $0x50F00,   %rdi\n"         // NOTE(vak): rdi = flags = CLONE_VM|CLONE_FILES|CLONE_FS|CLONE_THREAD|CLONE_SYSVMEM
        "mov    $0x38,      %rax\n"         // NOTE(vak): rax = __NR_clone = (56)
        "syscall\n"
        "mov    %rsp,       %rdi\n"         // NOTE(vak): Argument for LinuxThreadEntry(). Main thread will ignore this.
        "ret\n"                             // NOTE(vak): Main thread returns as normal. Spawned thread ends up in LinuxThreadEntry().
    );
#else
#   error LinuxPerformClone() is not implemented for this architecture.
#endif
}

local thread_id SpawnThread(spawn_thread_info* Info)
{
    if (!Info)                                  return (false);
    if (!Info->Callback)                        return (false);
    if (!Info->StackBase)                       return (false);
    if (Info->StackSize < KB(1))                return (false);
    if (!IsAligned((usize)Info->StackBase, 64)) return (false);
    if (!IsAligned(Info->StackSize, 64))        return (false);

    if (LinuxThreadCount == LinuxMaxThreadSpawnCount)
        return (false);

    thread_id ThreadID = LinuxThreadCount;

    linux_thread* Thread = LinuxAllThreads + ThreadID;
    Thread->JoinFutex = 0;

    ZeroMemory(Info->StackBase, Info->StackSize);

    linux_stack_header* Header = (linux_stack_header*)
        ((u8*)Info->StackBase + Info->StackSize - sizeof(linux_stack_header));

    Header->ThreadEntryAddress  = (void*)&LinuxThreadEntry;
    Header->Callback            = Info->Callback;
    Header->UserData            = Info->UserData;
    Header->ThreadID            = ThreadID;

    ssize CloneResult = LinuxPerformClone(Header);

    thread_id ResultID = NilThreadID;

    if (CloneResult >= 0)
    {
        ResultID = ThreadID;
        LinuxThreadCount++;
    }

    return (ResultID);
}

local s32 JoinThread(thread_id ThreadID)
{
    if (!LinuxIsValidThreadID(ThreadID))
        return (0);

    linux_thread* Thread = LinuxAllThreads + ThreadID;

    LinuxSyscall(
        LinuxSyscallNR_Futex,
        (usize)&Thread->JoinFutex,
        FUTEX_WAIT,
        0, 0,
        0, 0
    );

    s32 Returned = Thread->Returned;

    ZeroType(Thread);

    return (Returned);
}

local void ThreadExit(u8 ExitCode)
{
    LinuxSyscall(LinuxSyscallNR_Exit, ExitCode, 0, 0, 0, 0, 0);
}

local void ProcessExit(u8 ExitCode)
{
    LinuxSyscall(LinuxSyscallNR_ExitGroup, ExitCode, 0, 0, 0, 0, 0);
}

#else
#   error "platform.c does not recognize this platform"
#endif

