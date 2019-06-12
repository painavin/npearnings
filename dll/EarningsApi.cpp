/*++

    Copyright (c) Pai Financials LLC

Module Name:

    EarningsApi.cpp

Abstract:

    The APIs exported by the DLL are implemented in this function

Author:

    Navin Pai (navin.pai@outlook.com) - 06-Oct-2015

--*/
#include "pch.h"
#include "EarningsApi.h"

_Use_decl_annotations_
LPCSTR
WINAPI
GetEarningsReleaseDate(
    LPCSTR Ticker
    )
/*++

Abstract:

    Returns the date of the next earnings release

--*/
{
    return Ticker;
}


_Use_decl_annotations_
LPCSTR
WINAPI
GetEarningsReleaseTime(
    LPCSTR Ticker
    )
/*++

Abstract:

    Returns the time when the earnings will be released

--*/
{
    return Ticker;
}


_Use_decl_annotations_
LPCSTR
WINAPI
GetDaysToEarningsRelease(
    LPCSTR Ticker
    )
/*++

Abstract:

    Returns Number of days to next earnings release as a string.

--*/
{
    return Ticker;
}


_Use_decl_annotations_
INT
WINAPI
GetEarningsConfirmation(
    LPCSTR Ticker
    )
/*++

Abstract:

    Returns the notes from the earnings file

--*/
{
    return 0;
}
