
#include "../shared.c"
#include "../platform.c"
#include "../print.c"
#include "../character.c"

typedef struct
{
    usize WordCount;
    usize CharacterCount;
    usize WhitespaceCount;
    usize DigitCount;
    usize PunctuationCount;
} text_statistics;

local void AnalyzeText(string Text, text_statistics* ResultStats);

local s32 Main(main_info* Info)
{
    if (Info->ArgCount == 0)
        return (1);

    if (Info->ArgCount == 1)
    {
        Print           (StdOut(), Str("Usage: "));
        Print           (StdOut(), Info->Args[0]);
        Print           (StdOut(), Str(" <path_to_text_file>"));
        PrintNewLine    (StdOut());

        return (1);
    }

    string  FilePath = Info->Args[1];

    if (!DoesFileExist(FilePath))
    {
        Print       (StdOut(), Str("Error: The file `"));
        Print       (StdOut(), FilePath);
        Print       (StdOut(), Str("` doesn't exist."));
        PrintNewLine(StdOut());

        return (1);
    }

    usize FileSize  = ReadFileToBuffer(FilePath, 0, 0);
    void* FileData  = ReserveAndCommit(FileSize);
    usize BytesRead = ReadFileToBuffer(FilePath, FileData, FileSize);

    if (BytesRead != FileSize)
    {
        Print       (StdOut(), Str("Error: Failed to read from file `"));
        Print       (StdOut(), FilePath);
        Print       (StdOut(), Str("`."));
        PrintNewLine(StdOut());
        return (1);
    }

    string          Text  = StrData(FileData, FileSize);
    text_statistics Stats = {0};

    AnalyzeText(Text, &Stats);

    Print       (StdOut(), Str("Text statistics:"));
    PrintNewLine(StdOut());

    Print       (StdOut(), Str("    Word count:        "));
    PrintUSize  (StdOut(), Stats.WordCount);
    PrintNewLine(StdOut());

    Print       (StdOut(), Str("    Character count:   "));
    PrintUSize  (StdOut(), Stats.CharacterCount);
    PrintNewLine(StdOut());

    Print       (StdOut(), Str("    Whitespace count:  "));
    PrintUSize  (StdOut(), Stats.WhitespaceCount);
    PrintNewLine(StdOut());

    Print       (StdOut(), Str("    Digit count:       "));
    PrintUSize  (StdOut(), Stats.DigitCount);
    PrintNewLine(StdOut());

    Print       (StdOut(), Str("    Punctuation count: "));
    PrintUSize  (StdOut(), Stats.PunctuationCount);
    PrintNewLine(StdOut());

    return (0);
}

local void AnalyzeText(string Text, text_statistics* ResultStats)
{
    if (!ResultStats)
        return;

    ZeroType(ResultStats);

    b32 InsideWord = false;

    for (usize Index = 0; Index < Text.Size; Index++)
    {
        char Character = Text.Data[Index];

        ResultStats->CharacterCount     += IsAlphabet   (Character);
        ResultStats->WhitespaceCount    += IsWhitespace (Character);
        ResultStats->DigitCount         += IsDigit      (Character);
        ResultStats->PunctuationCount   += IsPunctuation(Character);

        b32 WasInsideWord   = InsideWord;
        b32 IsInsideWordNow = IsAlphabet(Character);
        b32 JustEnteredWord = (WasInsideWord == false) && (IsInsideWordNow == true);

        ResultStats->WordCount += JustEnteredWord;

        InsideWord = IsInsideWordNow;
    }
}

