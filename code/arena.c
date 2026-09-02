
#pragma once

#include "shared.c"
#include "platform.c"

typedef u32 arena_id;
#define NilArenaID (0)

typedef struct
{
    usize MinCommited;  // NOTE(vak): Minimum amount of memory that have been mapped to physical pages
    usize MinReserved;  // NOTE(vak): Minimum size of the virtual address space that the user expects to have
    usize Alignment;    // NOTE(vak): Alignment for `PushArenaSize` to follow. Has to be a power of 2. If 0 then MakeArena() defaults the alignment to 1.
} make_arena_info;

local arena_id  MakeArena       (make_arena_info* Info);            // NOTE(vak): Allocates a new arena with the specified options
local void      DeleteArena     (arena_id ArenaID);                 // NOTE(vak): Releases an arena's memory
local void      DeleteAllArenas (void);                             // NOTE(vak): Deletes all arenas that have been created

local void      ResetArena      (arena_id ArenaID);                 // NOTE(vak): Resets allocation pointer back to base address (set Used = 0)
local void*     PushArenaSize   (arena_id ArenaID, usize Size);     // NOTE(vak): Returns current allocation pointer and bump allocation pointer up by `Size`. (Used += Size)

local usize     GetArenaUsed    (arena_id ArenaID);                 // NOTE(vak): Get the amount of memory used so far.
local void*     GetArenaBaseAt  (arena_id ArenaID);                 // NOTE(vak): Get the base address of the virtual address space.
local void*     GetArenaAllocAt (arena_id ArenaID);                 // NOTE(vak): Get the allocation pointer: (u8*)Base + Used

#define PushArena(ArenaID, Type)                (Type*)PushArenaSize(ArenaID, sizeof(Type))
#define PushArenaArray(ArenaID, Type, Count)    (Type*)PushArenaSize(ArenaID, sizeof(Type) * (Count))

// NOTE(vak): Implementation

typedef struct
{
    void* Base;         // NOTE(vak): Base address of the virtual address space
    usize Used;         // NOTE(vak): Allocation pointer
    usize Commited;     // NOTE(vak): Amount of memory that has been commited. Memory within the range [Base, Base+Commited) is considered usable.
    usize Reserved;     // NOTE(vak): Size of the virtual address space
    usize Alignment;    // NOTE(vak): Alignment for all `PushArenaSize` calls to follow.
} arena;

#define MaxArenaCount (64)

local arena AllArenas[MaxArenaCount] = {0};

// NOTE(vak): Alignment for the `Commited` and `Reserved` fields of the arena. Should be
// reasonably large but not too large as to not make too many system calls and to save
// system memory.
#define ArenaGranuleSize KB(256)

#define IsValidArenaID(ArenaID) ((ArenaID) > 0) && ((ArenaID) < MaxArenaCount)

local arena_id MakeArena(make_arena_info* Info)
{
    if (!Info)                                  return (NilArenaID);
    if (Info->MinReserved == 0)                 return (NilArenaID);
    if (Info->MinReserved <  Info->MinCommited) return (NilArenaID);
    if (Info->MinReserved <  Info->Alignment)   return (NilArenaID);

    arena_id ArenaID = NilArenaID;

    for (arena_id SearchID = 1; SearchID < MaxArenaCount; SearchID++)
    {
        arena* Candidate = AllArenas + SearchID;
        if (!Candidate->Base)
        {
            ArenaID = SearchID;
            break;
        }
    }

    if (ArenaID == NilArenaID)
        return (NilArenaID);

    arena* Arena = AllArenas + ArenaID;

    Arena->Reserved     = AlignUp(Info->MinReserved, ArenaGranuleSize);
    Arena->Commited     = AlignUp(Info->MinCommited, ArenaGranuleSize);
    Arena->Alignment    = Maximum(1, Info->Alignment);
    Arena->Base         = ReserveMemory(Arena->Reserved);

    if (!Arena->Base)
        return (NilArenaID);

    b32 CommitOkay = true;

    if (Arena->Commited)
        CommitOkay = CommitMemory(Arena->Base, Arena->Commited);

    if (!CommitOkay)
    {
        DeleteArena(ArenaID);
        ArenaID = NilArenaID;
    }

    return (ArenaID);
}

local void DeleteArena(arena_id ArenaID)
{
    if (!IsValidArenaID(ArenaID))
        return;

    arena* Arena = AllArenas + ArenaID;

    if (Arena->Base)
        ReleaseMemory(Arena->Base, Arena->Reserved);

    ZeroType(Arena);
}

local void DeleteAllArenas(void)
{
    for (arena_id DeleteAtID = 1; DeleteAtID < MaxArenaCount; DeleteAtID++)
        DeleteArena(DeleteAtID);
}

local void ResetArena(arena_id ArenaID)
{
    if (!IsValidArenaID(ArenaID))
        return;

    arena* Arena = AllArenas + ArenaID;
    Arena->Used = 0;
}

local void* PushArenaSize(arena_id ArenaID, usize Size)
{
    if (!IsValidArenaID(ArenaID))
        return (0);

    arena* Arena = AllArenas + ArenaID;

    usize AlignedSize = AlignUp(Size, Arena->Alignment);

    if (Arena->Used + AlignedSize > Arena->Commited)
    {
        usize ExpandSize = (Arena->Used + AlignedSize) - Arena->Commited;
        usize CommitSize = AlignUp(ExpandSize, ArenaGranuleSize);
        void* CommitAt   = (u8*)Arena->Base + Arena->Commited;

        if (Arena->Commited + CommitSize > Arena->Reserved)
            return (0);

        if (!CommitMemory(CommitAt, CommitSize))
            return (0);

        Arena->Commited += CommitSize;
    }

    void* Result = (u8*)Arena->Base + Arena->Used;
    Arena->Used += AlignedSize;

    return (Result);
}

local usize GetArenaUsed(arena_id ArenaID)
{
    if (!IsValidArenaID(ArenaID))
        return (0);

    arena* Arena = AllArenas + ArenaID;
    usize Result = Arena->Used;

    return (Result);
}

local void* GetArenaBaseAt(arena_id ArenaID)
{
    if (!IsValidArenaID(ArenaID))
        return (0);

    arena* Arena = AllArenas + ArenaID;
    void* Result = Arena->Base;

    return (Result);
}

local void* GetArenaAllocAt(arena_id ArenaID)
{
    if (!IsValidArenaID(ArenaID))
        return (0);

    arena* Arena = AllArenas + ArenaID;
    void* Result = (u8*)Arena->Base + Arena->Used;

    return (Result);
}

