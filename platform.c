
#pragma once

#include "shared.c"

typedef struct
{
    string* Args;
    usize   ArgCount;
} main_info;

local s32 Main(main_info* Info); // NOTE(vak): Implement your main function like this and the platform implementation will call it.

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
local b32           DoesFileExist       (string FilePath);                              // NOTE(vak): Checks if a file exists. Returns true if file exists, else returns false.
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

#define LinuxMaxArgCount (1024)

void LinuxSetupAndCallMain(s32 ArgCount, char** Args)
{
    persist string ConvertedArgs[LinuxMaxArgCount] = {0};

    if (ArgCount >= 0)
    {
        for (s32 Index = 0; Index < ArgCount; Index++)
            ConvertedArgs[Index] = CString(Args[Index]);
    }

    s32 Returned = Main(&(main_info){
        .Args       = ConvertedArgs,
        .ArgCount   = (usize)Maximum(0, ArgCount),
    });

    ProcessExit(Returned);
}

// NOTE(vak): Unfortunately, we have to dabble in inline assembly on Linux. There are two places
// that require inline assembly right now:
//          + LinuxPerformClone():  Creates a new thread and calls thread entry point properly
//          + EntryPoint():         Retrieves ArgCount, Args and then calls LinuxSetupAndCallMain()

__attribute__((naked))
void EntryPoint(void)
{
#if ArchitectureX64
    // NOTE(vak): Layout of stack that Linux sets up for us on x86_64:

    //      rsp + 0  -> ArgCount            (s32)
    //      rsp + 8  -> Args[0]             (char*)
    //      rsp + 16 -> Args[1]             (char*)
    //      rsp + 24 -> Args[2]             (char*)
    //      rsp + .. -> Args[3]             (char*)
    //      rsp + .. -> Args[4]             (char*)
    //      ...         ....
    //      ...         ....
    //      rsp + .. -> Args[ArgCount - 1]  (char*)

    //      After the Args[] array is the array of
    //      environment variables. The last entry
    //      in the Envp[] array is set to 0, which
    //      terminates the array.

    //      rsp + .. -> Envp[0]             (char*)
    //      rsp + .. -> Envp[1]             (char*)
    //      rsp + .. -> Envp[2]             (char*)
    //      rsp + .. -> Envp[3]             (char*)
    //      rsp + .. -> Envp[...]           (char*)
    //      ...         ....
    //      rsp + .. -> Envp[...] = 0       (char*)

    // NOTE(vak): Keep in mind that the stack is misaligned (aligned to
    // 8-byte boundary instead of 16-byte boundary) when we're in EntryPoint().
    // However, the call to LinuxSetupAndCallMain() will push the return address
    // and align the stack to a 16-byte boundary, so it's all good.

    // NOTE(vak): This assembly code simply performs:
    //      LinuxSetupAndCallMain(ArgCount, Args)

    __asm__ volatile (
        "lea    8(%rsp), %rsi\n"            // NOTE(vak): Put base address of `Args` array into rsi
        "mov    0(%rsp), %edi\n"            // NOTE(vak): Put ArgCount into rdi
        "call   LinuxSetupAndCallMain\n"    // NOTE(vak): Perform the call
    );
#else
#   error Linux EntryPoint() is not implemented on this architecture yet.
#endif
}

typedef enum
{
#if ArchitectureX64
    LinuxSyscallNR_Read             = (0),
    LinuxSyscallNR_Write            = (1),
    LinuxSyscallNR_Open             = (2),
    LinuxSyscallNR_Close            = (3),
    LinuxSyscallNR_Stat             = (4),
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

// NOTE(vak): Taken from /usr/include/asm-generic/stat.h
// From the comment in the file, this should be the correct 64-bit version.
// We don't target 32-bit architectures so this should be the only version
// of `stat` that we use.

typedef struct
{
    u64                 Device;
    u64                 SerialNumber;
    u32                 Mode;
    u32                 LinkCount;
    u32                 UserID;
    u32                 GroupID;
    u64                 DeviceNumber;
    u64                 Pad0;
    s64                 SizeInBytes;
    s32                 OptimalBlockSize;
    s32                 Pad1;
    s64                 AllocatedBlocks512;
    s64                 LastAccessSeconds;
    u64                 LastAccessNanoseconds;
    s64                 LastModificationSeconds;
    u64                 LastModificationNanoseconds;
    s64                 LastStatusChangeSeconds;
    u64                 LastStatusChangeNanoseconds;
    u32                 Pad2;
    u32                 Pad3;
} linux_stat;

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

local b32 DoesFileExist(string FilePath)
{
    // NOTE(vak): `FilePath` string may not have a null terminator.
    static char PathBuffer[KB(1)] = {0};

    if (FilePath.Size >= sizeof(PathBuffer))
        return (0);

    CopyMemory(PathBuffer, FilePath.Data, FilePath.Size);
    PathBuffer[FilePath.Size] = '\0';

    linux_stat Stat = {0};
    ssize FStatResult = LinuxSyscall(
        LinuxSyscallNR_Stat,
        (usize)PathBuffer,
        (usize)&Stat,
        0, 0, 0, 0
    );

    b32 Result = (FStatResult >= 0);
    return (Result);
}

local usize ReadFileToBuffer(string FilePath, void* Buffer, usize Size)
{
    // NOTE(vak): `FilePath` string may not have a null terminator.
    static char PathBuffer[KB(1)] = {0};

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

