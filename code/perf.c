
#pragma once

#include "shared.c"
#include "platform.c"

typedef void perf_target_function(void* UserData);

typedef struct
{
    wall_clock  MinTimeTaken;   // NOTE(vak): Minimum amount of time taken for an iteration
    wall_clock  MaxTimeTaken;   // NOTE(vak): Maximum amount of time taken for an iteration
    wall_clock  AvgTimeTaken;   // NOTE(vak): Average amount of time taken per iteration
    wall_clock  TotalTimeTaken; // NOTE(vak): Total amount of time taken overall
} perf_statistics;

typedef struct
{
    perf_target_function*   TargetFunction; // NOTE(vak): Target function to start measuring. Required to be set.
    void*                   UserData;       // NOTE(vak): User data to pass to target function
    usize                   Iterations;     // NOTE(vak): Amount of iterations to repeatedly measure the target function. Must be equal to or larger than 1.
    wall_clock*             ResultTimings;  // NOTE(vak): Where the resulting amount of time taken for each iteration will be written to. Optional. Array must be the same size or larger than the specified iteration count.
    perf_statistics*        ResultStats;    // NOTE(vak): Where the resulting statistics will be written to. Required to be set.
} perf_measure_info;

local void PerfMeasure(perf_measure_info* Info);    // NOTE(vak): Perform a performance measurement with the specified options.

// NOTE(vak): Implementation

local void PerfMeasure(perf_measure_info* Info)
{
    if (!Info) return;
    if (!Info->TargetFunction)  return;
    if (!Info->Iterations)      return;
    if (!Info->ResultStats)     return;

    wall_clock MinTimeTaken     = USizeMax;
    wall_clock MaxTimeTaken     = 0;
    wall_clock AvgTimeTaken     = 0;
    wall_clock TotalTimeTaken   = 0;

    for (usize Index = 0; Index < Info->Iterations; Index++)
    {
        wall_clock Begin = GetClockNow();

        Info->TargetFunction(Info->UserData);

        wall_clock TimeTaken = ClockElapsedSince(Begin);

        if (Info->ResultTimings)
            Info->ResultTimings[Index] = TimeTaken;

        MinTimeTaken    = Minimum(MinTimeTaken, TimeTaken);
        MaxTimeTaken    = Maximum(MinTimeTaken, TimeTaken);
        TotalTimeTaken  += TimeTaken;
    }

    AvgTimeTaken = TotalTimeTaken / Info->Iterations;

    perf_statistics* Result = Info->ResultStats;

    Result->MinTimeTaken    = MinTimeTaken;
    Result->MaxTimeTaken    = MaxTimeTaken;
    Result->AvgTimeTaken    = AvgTimeTaken;
    Result->TotalTimeTaken  = TotalTimeTaken;
}

