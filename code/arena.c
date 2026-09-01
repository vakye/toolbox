
#pragma once

#include "shared.c"
#include "platform.c"

typedef u32 arena_id;
#define NilArenaID (0)

typedef struct
{
    usize MinCommited;
    usize MinReserved;
    usize Alignment;
} make_arena_info;

local arena_id  MakeArena       (make_arena_info* Info);
local void      DeleteArena     (arena_id ArenaID);
local void      DeleteAllArenas (void);

local void      ResetArena      (arena_id ArenaID);
local void*     PushArenaSize   (arena_id ArenaID, usize Size);

local usize     GetArenaUsed    (arena_id ArenaID);
local void*     GetArenaBaseAt  (arena_id ArenaID);
local void*     GetArenaAllocAt (arena_id ArenaID);

#define PushArena(ArenaID, Type)                (Type*)PushArenaSize(ArenaID, sizeof(Type))
#define PushArenaArray(ArenaID, Type, Count)    (Type*)PushArenaSize(ArenaID, sizeof(Type) * (Count))

// NOTE(vak): Implementation

typedef struct
{
    void* Base;
    usize Used;
    usize Commited;
    usize Reserved;
    usize Alignment;
} arena;

#define MaxArenaCount (64)

local arena AllArenas[MaxArenaCount] = {0};

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

