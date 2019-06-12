/*++

    Copyright (c) Pai Financials LLC

Module Name:

    EarningsApi.h

Abstract:

    The header file for the main APIs exposed by the dll

Author:

    Navin Pai (navin.pai@outlook.com) - 06-Oct-2015

--*/
#pragma once

extern "C"
{

//
// The callback proc
//
typedef 
VOID (CALLBACK *LOGGERPROC)(
    _In_ LPCSTR Message
    );

//
// Get the earnings release date 
//
LPCSTR 
WINAPI 
GetEarningsReleaseDate(
    _In_ LPCSTR Symbol
    );

//
// Get the earnings release time
//
LPCSTR 
WINAPI 
GetEarningsReleaseTime(
    _In_ LPCSTR Symbol
    );

//
// Get the days to earnings release
//
LPCSTR 
WINAPI 
GetDaysToEarningsRelease(
    _In_ LPCSTR Symbol
    );

//
// Get the earnings release confirmation
//
INT 
WINAPI 
GetEarningsConfirmation(
    _In_ LPCSTR Symbol
    );

}
