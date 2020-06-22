/*++

    Copyright (c) Pai Financials LLC.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

Module Name:

    EarningsApi.h

Abstract:

    The API header file

Author:

    nabieasaurus

--*/
#pragma once


extern "C" 
{
    //
    // Logging levels
    //
#define NP_LOG_ERROR            1
#define NP_LOG_WARNING          2
#define NP_LOG_INFO             3
#define NP_LOG_TRACE            4

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
