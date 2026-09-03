
#pragma once

#include "shared.c"

local b32   IsPrintable     (char Character);       // NOTE(vak): Returns true if character is not a control code, false if character is a control code.
local b32   IsWhitespace    (char Character);       // NOTE(vak): Returns true if character is whitespace, otherwise returns false.
local b32   IsDigit         (char Character);       // NOTE(vak): Returns true if character is a digit, otherwise returns false.
local b32   IsLowercase     (char Character);       // NOTE(vak): Returns true if character is an lowercase alphabetical, otherwise returns false.
local b32   IsUppercase     (char Character);       // NOTE(vak): Returns true if character is an uppercase alphabetical, otherwise returns false.
local b32   IsAlphabet      (char Character);       // NOTE(vak): Returns true if character is an alphabetical, otherwise returns false.
local b32   IsPunctuation   (char Character);       // NOTE(vak): Returns true if character is a punctuation, otherwise returns false.

local char  ToLowercase     (char Character);       // NOTE(vak): Returns uppercase equivalent of character if character is lowercase, otherwise just return the character.
local char  ToUppercase     (char Character);       // NOTE(vak): Returns lowercase equivalent of character if character is uppercase, otherwise just return the character.

// NOTE(vak): Implementation

local b32 IsPrintable(char Character)
{
    b32 Result = ((Character >= 32) && (Character <= 126));
    return (Result);
}

local b32 IsWhitespace(char Character)
{
    b32 Result =
        (Character == ' ') ||
        (Character == '\r') ||
        (Character == '\t') ||
        (Character == '\n');

    return (Result);
}

local b32 IsDigit(char Character)
{
    b32 Result = (Character >= '0') && (Character <= '9');
    return (Result);
}

local b32 IsLowercase(char Character)
{
    b32 Result = (Character >= 'a') && (Character <= 'z');
    return (Result);
}

local b32 IsUppercase(char Character)
{
    b32 Result = (Character >= 'A') && (Character <= 'Z');
    return (Result);
}

local b32 IsAlphabet(char Character)
{
    b32 Result = IsLowercase(Character) || IsUppercase(Character);
    return (Result);
}

local b32 IsPunctuation(char Character)
{
    b32 Result =
        ((Character >=  33) && (Character <=  47)) ||
        ((Character >=  58) && (Character <=  64)) ||
        ((Character >=  91) && (Character <=  91)) ||
        ((Character >= 123) && (Character <= 126));

    return (Result);
}

local char ToLowercase(char Character)
{
    char Result = Character + 32*IsUppercase(Character);
    return (Result);
}

local char ToUppercase(char Character)
{
    char Result = Character - 32*IsLowercase(Character);
    return (Result);
}

