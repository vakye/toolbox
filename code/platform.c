
#pragma once

local void Exit(u8 ExitCode);

local void Main(void);

#if PlatformWindows
#   error "platform.c does not support Windows at the moment"
#elif PlatformLinux

__attribute__((force_align_arg_pointer))
void EntryPoint(void)
{
    Main();
    Exit(0);
}

typedef enum
{
#if ArchitectureX64
    LinuxSyscallNR_Exit     = (60),
#else
#   error Linux syscall numbers are not defined for this architecture yet.
#endif
} linux_syscall_nr;

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

local void Exit(u8 ExitCode)
{
    LinuxSyscall(LinuxSyscallNR_Exit, ExitCode, 0, 0, 0, 0, 0);
}

#else
#   error "platform.c does not recognize this platform"
#endif

