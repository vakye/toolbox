
#pragma once

#include "shared.c"
#include "platform.c"

typedef s32 lock32;

#define IsLockReleased(Lock) ((Lock) == 0)
#define IsLockAcquired(Lock) ((Lock) == 1)

local b32  TryAcquireLock   (lock32* Lock);         // NOTE(vak): Tries to acquire a lock. Returns true if lock was acquired, otherwise returns false.
local void AcquireLockSpin  (lock32* Lock);         // NOTE(vak): Acquires a lock. If lock was already acquired then perform a spin-wait until lock has been released. Should be used for low-latency, high CPU utilization situations.
local void ReleaseLock      (lock32* Lock);         // NOTE(vak): Releases a lock. If lock was not acquired then do nothing.

#define AcquireLock(Lock) AcquireLockSpin(Lock)     // NOTE(vak): Default method to acquire a lock is the spin-wait version.

// NOTE(vak): Implementation

local b32 TryAcquireLock(lock32* Lock)
{
    b32 Result = false;

#if CompilerGCC
    s32 Expected = 0;
    Result =  __atomic_compare_exchange_n(Lock, &Expected, 1, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
#else
#   error TryAcquireLock() is not implemented for this compiler yet.
#endif

    return (Result);
}

local void AcquireLockSpin(lock32* Lock)
{
    if (!Lock)
        return;

    while (!TryAcquireLock(Lock));
}

local void ReleaseLock(lock32* Lock)
{
    if (!Lock)
        return;

#if CompilerGCC
    s32 Expected = 1;
    __atomic_compare_exchange_n(Lock, &Expected, 0, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
#else
#   error ReleaseLock() is not implemented for this compiler yet.
#endif    
}

