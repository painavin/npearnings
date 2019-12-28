/*++

    Copyright (c) 2015 Pai Financials LLC

Module Name:

    EarningsApi.h

Abstract:

    The API header file

Author:

    Navin Pai (navinp) 

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
// Initialize the logger
//
VOID 
WINAPI 
InitLogger(
    _In_ DWORD LogLevel, 
    _In_ LOGGERPROC LoggerProc,
    _In_ bool ResetData
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

//
// Get the notes for the earnings
//
LPCSTR 
WINAPI 
GetEarningsNotes(
    _In_ LPCSTR Symbol
    );

//
// Set the earnings notes
//
VOID 
WINAPI 
SetEarningsNotes(
    _In_ LPCSTR Symbol, 
    _In_ LPCSTR Notes
    );


//
// Exported function for Forex from DailyFx.com
//
LPCSTR 
WINAPI 
GetNextFxEventDate(
    _In_ LPCSTR CurrencyPair
    );

LPCSTR 
WINAPI 
GetNextFxEventTime(
    _In_ LPCSTR CurrencyPair
    );

LPCSTR 
WINAPI 
GetNextFxEventDuration(
    _In_ LPCSTR CurrencyPair
    );

LPCSTR 
WINAPI 
GetNextFxEventDesc(
    _In_ LPCSTR CurrencyPair
    );

INT 
WINAPI 
GetNextFxEventImp(
    _In_ LPCSTR CurrencyPair
    );

} // Extern C
