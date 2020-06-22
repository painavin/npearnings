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

    EarningsApi.cpp

Abstract:

    The APIs exported by the DLL are implemented in this function

Author:

    nabieasaurus

--*/
#include "stdafx.h"
#include "EarningsApi.h"
#include "EarningsMain.h"

#define EARNINGS_NOT_AVAILABLE      ""
#define MAX_TICKER_LENGTH           10
#define MAX_CURRENCY_LENGTH         6


CEarningsDataPtr_t
GetEarningsData(
    LPCSTR Ticker
    )
/*

Abstract:

    This function was added as a wrapper to add exception handling for our function.
    This is required since the webpage may change anytime and the parsing code could
    fail crashing the application.

    The exception handling code will make sure even if the parsing fails, the app will
    not crash.

*/
{
    CEarningsDataPtr_t pEarningsData = NULL;
    EnterFunc();

    //
    // Validate the ticker symbol
    //
    if ((Ticker == NULL) || (Ticker[0] == _T('\0')) || 
        (strlen(Ticker) > MAX_TICKER_LENGTH))
    {
        LogError("Invalid parameters passed to the function");
        return NULL;
    }

    __try
    {
        if (gEarningsMain.Initialize(GetModuleHandle(NULL)))
        {
            pEarningsData = gEarningsMain.m_EarningsRelease.GetEarningsData(Ticker);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        LogError("Exception code = %x", GetExceptionCode());
        pEarningsData = NULL;
    }

    LeaveFunc();
    return pEarningsData;
}


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
    EnterFunc();
    LPCSTR retVal = EARNINGS_NOT_AVAILABLE;

    CEarningsDataPtr_t pData = GetEarningsData(Ticker);

    if ((pData != NULL) && (pData->IsAvailable == true))
    {
        retVal = pData->StrEarningsDate.c_str();
    }

    LeaveFunc();
    return retVal;
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
    EnterFunc();
    LPCSTR retVal = EARNINGS_NOT_AVAILABLE;

    CEarningsDataPtr_t pData = GetEarningsData(Ticker);

    if ((pData != NULL) && (pData->IsAvailable == true))
    {
        retVal = pData->StrEarningsTime.c_str();
    }

    LeaveFunc();
    return retVal;
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
    EnterFunc();
    LPCSTR retVal = EARNINGS_NOT_AVAILABLE;

    CEarningsDataPtr_t pData = GetEarningsData(Ticker);

    if ((pData != NULL) && (pData->IsAvailable == true))
    {
        pData->UpdateStrEarningsDays();
        retVal = pData->StrEarningsDays.c_str();
    }

    LeaveFunc();
    return retVal;
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
    EnterFunc();
    INT retVal = 0;
    CEarningsDataPtr_t pData = GetEarningsData(Ticker);

    if ((pData != NULL) && (pData->IsAvailable == true))
    {
        retVal = pData->IsConfirmed ? 1 : 0;
    }

    LeaveFunc();
    return retVal;
}


_Use_decl_annotations_
LPCSTR
WINAPI
GetEarningsNotes(
    LPCSTR Ticker
    )
/*++

Abstract:

    Returns the notes from the earnings file

--*/
{
    EnterFunc();
    LPCSTR retVal = "";
    CEarningsDataPtr_t pData = GetEarningsData(Ticker);

    if (pData != NULL)
    {
        LogTrace("GetEarningsNotes Returning[%s]: %s", 
            pData->StrTicker.c_str(),
            pData->StrEarningsNotes.c_str());
        retVal = pData->StrEarningsNotes.c_str();
    }

    LeaveFunc();
    return retVal;
}


_Use_decl_annotations_
void
WINAPI
SetEarningsNotes(
    LPCSTR Ticker,
    LPCSTR Notes
    )
/*++

Abstract:

    Sets the notes for the ticker

--*/
{
    EnterFunc();
    if (Notes == NULL) return;

    CEarningsDataPtr_t  pData = GetEarningsData(Ticker);

    if (pData != NULL)
    {
        //
        // TODO: Rare race condition
        //
        pData->StrEarningsNotes = Notes;
        gEarningsMain.m_EarningsRelease.m_bCacheDirty = true;

        LogTrace("SetEarningsNotes Setting[%s]: %s", 
            pData->StrTicker.c_str(), Notes);
    }

    LeaveFunc();
}


PFOREX_EVENT
GetForexEvent(
    LPCSTR CurrencyPair
    )
/*

Abstract:

    This function was added as a wrapper to add exception handling for our function.
    The exception handling code will make sure even if the parsing fails, the app will
    not crash.

*/
{
    EnterFunc();
    PFOREX_EVENT pForexEvent = NULL;

#ifdef NPFOREX
#pragma message(__LOC__ "* * * * * * * * * FOREX ENABLED * * * * * * * *.")

    //
    // Validate the ticker symbol
    //
    if ((CurrencyPair == NULL) || (CurrencyPair[0] == _T('\0')) || 
        (strlen(CurrencyPair) > MAX_CURRENCY_LENGTH))
    {
        LogError("Invalid parameters passed to the function");
        return NULL;
    }

    __try
    {
        pForexEvent = gEarningsMain.m_ForexEvents.GetNextFxEvent(CurrencyPair);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        LogError("Exception code = %x", GetExceptionCode());
        pForexEvent = NULL;
    }
#else
    UNREFERENCED_PARAMETER(CurrencyPair);
#endif

    LeaveFunc();
    return pForexEvent;
}


_Use_decl_annotations_
LPCSTR
WINAPI
GetNextFxEventDate(
    LPCSTR CurrencyPair
    )
/*++ 

Abstract:
    Returns the next forext event date related to the currency pair.

--*/
{
    EnterFunc();

    LPSTR retVal = "";
    PFOREX_EVENT pFxEvt = GetForexEvent(CurrencyPair);

    if (pFxEvt != NULL)
    {
        retVal = pFxEvt->StrEventDate;
    }

    LeaveFunc();
    return retVal;
}


_Use_decl_annotations_
LPCSTR
WINAPI
GetNextFxEventTime(
    LPCSTR CurrencyPair
    )
/*++

Abstract:
    Returns the next forex event time related to the currency pair.

--*/
{
    EnterFunc();

    LPSTR retVal = "";
    PFOREX_EVENT pFxEvt = GetForexEvent(CurrencyPair);

    if (pFxEvt != NULL)
    {
        retVal = pFxEvt->StrEventTime;
    }

    LeaveFunc();
    return retVal;
}


_Use_decl_annotations_
LPCSTR
WINAPI
GetNextFxEventDuration(
    LPCSTR CurrencyPair
    )
/*++

Abstract:
    Returns the duration in hrs::mins related to the currency pair.

--*/
{
    EnterFunc();

    LPSTR retVal = "";
    PFOREX_EVENT pFxEvt = GetForexEvent(CurrencyPair);
    
    if (pFxEvt != NULL)
    {
        retVal = pFxEvt->GetEventDuration();
    }

    LeaveFunc();
    return retVal;
}


_Use_decl_annotations_
LPCSTR
WINAPI
GetNextFxEventDesc(
    LPCSTR CurrencyPair
    )
/*++

Abstract:
    Returns the next forex event time related to the currency pair.

--*/
{
    EnterFunc();

    LPSTR retVal = "";
    PFOREX_EVENT pFxEvt = GetForexEvent(CurrencyPair);
    
    if (pFxEvt != NULL)
    {
        retVal = pFxEvt->StrEventDesc;
    }
    
    LeaveFunc();
    return retVal;
}


_Use_decl_annotations_
INT
WINAPI
GetNextFxEventImp(
    LPCSTR CurrencyPair
    )
/*++

Abstract:
    Returns the next forex event time related to the currency pair.

--*/
{
    EnterFunc();

    INT retVal = 0;
    PFOREX_EVENT pFxEvt = GetForexEvent(CurrencyPair);
    
    if (pFxEvt != NULL)
    {
        retVal = pFxEvt->EventImportance;
    }

    LeaveFunc();
    return retVal;
}
