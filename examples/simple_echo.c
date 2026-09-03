
#include "../shared.c"
#include "../platform.c"
#include "../print.c"

local s32 Main(main_info* Info)
{
    for (usize ArgIndex = 1; ArgIndex < Info->ArgCount; ArgIndex++)
    {
        Print           (StdOut(),Info->Args[ArgIndex]);
        PrintCharacter  (StdOut(), ' ');
    }

    PrintNewLine(StdOut());

    return (0);
}

