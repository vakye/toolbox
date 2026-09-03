
#include "../shared.c"
#include "../platform.c"
#include "../print.c"

local s32 Main(main_info* Info)
{
    Println(StdOut(), Str("Hello, world!"));
    return (0);
}

