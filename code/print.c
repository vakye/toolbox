
#pragma once

#include "shared.c"
#include "platform.c"

typedef usize print_write(void* Data, usize Size, void* UserData);      // NOTE(vak): User-provided function for writing to their own specified output. Returns number of bytes written.

typedef struct
{
    print_write*    Write;
    void*           UserData;
} print_out;

#define MakePrintOut(Write, UserData) (print_out){Write, UserData}      // NOTE(vak): Creates a print_out with the user-specified write function along with the user data.

#define StdOut(...) MakePrintOut((print_write*)&WriteStdOut, 0)         // NOTE(vak): Creates a print_out for writing to the system stdout.
#define StdErr(...) MakePrintOut((print_write*)&WriteStdErr, 0)         // NOTE(vak): Creates a print_out for writing to the system stderr.

local usize PrintWrite      (print_out Out, void* Data, usize Size);    // NOTE(vak): Writes a buffer to the specified output. Returns number of bytes written.
local usize PrintCharacter  (print_out Out, char Character);            // NOTE(vak): Prints character to the specified output. Returns number of bytes written.
local usize PrintNewLine    (print_out Out);                            // NOTE(vak): Prints new line to the specified output. Returns number of bytes written.
local usize Print           (print_out Out, string Message);            // NOTE(vak): Prints a string to the specified output. Returns number of bytes written.
local usize Println         (print_out Out, string Message);            // NOTE(vak): Prints a string followed by a new line to the specified output. Returns number of bytes written.
local usize PrintUSize      (print_out Out, usize Value);               // NOTE(vak): Prints an unsigned integer to the specified output. Returns number of bytes written.
local usize PrintSSize      (print_out Out, ssize Value);               // NOTE(vak): Prints a signed integer to the specified output. Returns number of bytes written.
local usize PrintF64        (print_out Out, f64 Value);                 // NOTE(vak): Prints a floating point value to the specified output. Returns number of bytes written.
local usize PrintBytes      (print_out Out, usize Value);               // NOTE(vak): Prints amount of bytes with tb/gb/mb/kb/bytes prefix depending on the value. Returns number of bytes written.

// NOTE(vak): Implementation

local usize PrintWrite(print_out Out, void* Data, usize Size)
{
    usize Result = Out.Write(Data, Size, Out.UserData);
    return (Result);
}

local usize PrintCharacter(print_out Out, char Character)
{
    usize Result = PrintWrite(Out, &Character, 1);
    return (Result);
}

local usize PrintNewLine(print_out Out)
{
    usize Result = PrintCharacter(Out, '\n');
    return (Result);
}

local usize Print(print_out Out, string Message)
{
    usize Result = PrintWrite(Out, Message.Data, Message.Size);
    return (Result);
}

local usize Println(print_out Out, string Message)
{
    usize Result = 0;
    Result += Print(Out, Message);
    Result += PrintNewLine(Out);
    return (Result);
}

local usize PrintUSize(print_out Out, usize Value)
{
    char DigitBuffer[64] = {0};
    usize WriteDigitAt = sizeof(DigitBuffer);
    usize DigitCount = 0;

    do
    {
        char Digit = '0' + (char)(Value % 10);
        Value /= 10;

        WriteDigitAt--;
        DigitCount++;

        DigitBuffer[WriteDigitAt] = Digit;
    } while (Value);

    usize Result = Print(Out, StrData(DigitBuffer + WriteDigitAt, DigitCount));
    return (Result);
}

local usize PrintSSize(print_out Out, ssize Value)
{
    usize Result = 0;

    if (Value < 0)
    {
        Result += PrintCharacter(Out, '-');
        Value = -Value;
    }

    Result += PrintUSize(Out, Value);
    return (Result);
}

local usize PrintF64(print_out Out, f64 Value)
{
    usize Result = 0;

    if (Value < 0)
    {
        Result += PrintCharacter(Out, '-');
        Value = -Value;
    }

    // NOTE(vak): Very inaccurate!

    usize IntegerPart = (usize)Value;
    f64   DecimalPart = Value - (f64)IntegerPart;

    Result += PrintUSize(Out, IntegerPart);
    Result += PrintCharacter(Out, '.');

    for (usize DecimalIndex = 0; DecimalIndex < 3; DecimalIndex++)
    {
        DecimalPart *= 10.0;
        char Digit = (char)DecimalPart;
        DecimalPart -= Digit;

        Result += PrintCharacter(Out, Digit + '0');
    }

    return (Result);
}

local usize PrintBytes(print_out Out, usize Value)
{
    string Postfix = Str(" bytes");

         if (Value >= TB(10)) { Value >>= 40; Postfix = Str(" tb"); }
    else if (Value >= GB(10)) { Value >>= 30; Postfix = Str(" gb"); }
    else if (Value >= MB(10)) { Value >>= 20; Postfix = Str(" mb"); }
    else if (Value >= KB(10)) { Value >>= 10; Postfix = Str(" kb"); }

    usize Result = 0;
    Result += PrintUSize(Out, Value);
    Result += Print(Out, Postfix);
    return (Result);
}

