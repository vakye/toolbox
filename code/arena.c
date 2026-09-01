
#pragma once

#include "shared.c"
#include "platform.c"

typedef u32 arena_id;
#define NilArenaID (0)

local arena_id  MakeArena       (usize MinCommited, usize MinReserved);
local void      DeleteArena     (arena_id ArenaID);
local void      DeleteAllArenas (void);

local void      ResetArena      (arena_id ArenaID);
local void*     PushArenaSize   (arena_id ArenaID, usize Size);

local usize     ArenaUsed       (arena_id ArenaID);
local void*     ArenaBaseAt     (arena_id ArenaID);
local void*     ArenaAllocAt    (arena_id ArenaID);

#define PushArena(ArenaID, Type)                (Type*)PushArenaSize(ArenaID, sizeof(Type))
#define PushArenaArray(ArenaID, Type, Count)    (Type*)PushArenaSize(ArenaID, sizeof(Type) * (Count))

// NOTE(vak): Implementation

typedef struct
{
    void* Base;
    usize Used;
    usize Commited;
    usize Reserved;
} arena;

#define MaxArenaCount (64)

local arena AllArenas[MaxArenaCount] = {0};

#define ArenaGranuleSize KB(256)
#define IsValidArenaID(ArenaID) ((ArenaID) > 0) && ((ArenaID) < MaxArenaCount)

local arena_id MakeArena(usize MinCommited, usize MinReserved)
{
    if (MinReserved == 0)           return (NilArenaID);
    if (MinReserved <  MinCommited) return (NilArenaID);

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

    Arena->Reserved = AlignUp(MinReserved, ArenaGranuleSize);
    Arena->Commited = AlignUp(MinCommited, ArenaGranuleSize);

    Arena->Base = ReserveMemory(Arena->Reserved);

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

    if (Arena->Used + Size > Arena->Commited)
    {
        usize ExpandSize = (Arena->Used + Size) - Arena->Commited;
        usize CommitSize = AlignUp(ExpandSize, ArenaGranuleSize);
        void* CommitAt   = (u8*)Arena->Base + Arena->Commited;

        if (Arena->Commited + CommitSize > Arena->Reserved)
            return (0);

        if (!CommitMemory(CommitAt, CommitSize))
            return (0);

        Arena->Commited += CommitSize;
    }

    void* Result = (u8*)Arena->Base + Arena->Used;
    Arena->Used += Size;

    return (Result);
}

local usize ArenaUsed(arena_id ArenaID)
{
    if (!IsValidArenaID(ArenaID))
        return (0);

    arena* Arena = AllArenas + ArenaID;
    usize Result = Arena->Used;

    return (Result);
}

local void* ArenaBaseAt(arena_id ArenaID)
{
    if (!IsValidArenaID(ArenaID))
        return (0);

    arena* Arena = AllArenas + ArenaID;
    void* Result = Arena->Base;

    return (Result);
}

local void* ArenaAllocAt(arena_id ArenaID)
{
    if (!IsValidArenaID(ArenaID))
        return (0);

    arena* Arena = AllArenas + ArenaID;
    void* Result = (u8*)Arena->Base + Arena->Used;

    return (Result);
}

