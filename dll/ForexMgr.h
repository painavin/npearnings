/*++

    Copyright (c) Pai Financials LLC. All rights reserved.

Module Name:

    ForexEvents.h

Abstract:

    The header file for forex events. Extracted and added to expand the 
    earnings library to include next forex events as well.

Author:

    Navin Pai (navin.pai@outlook.com)

--*/
#pragma once

#include "FeedTime.h"
#include "HttpHelper.h"
#include "Lock.h"

#define FOREX_QUERY_INTERVAL    30


///////////////////////////////////////////////////////////////////////////////
//
// struct
//      FOREX_EVENT
//
// abstract
//      This structure holds the events related to the symbols
//
struct FOREX_EVENT
{
public:
    CFeedTime   EventDateTime;
    UINT        EventImportance;    // importance of the event
    char        StrCurrency[16];    // the currency for the event
    char        StrEventDate[16];   // date of the event
    char        StrEventTime[16];   // time of the event
    char        StrEventDesc[128];  // desc of the event

protected:
    char        StrDuration[16];    // Updated on every call

    // Constructors
public:
    FOREX_EVENT(
        CFeedTime Time,
        LPCSTR Currency,
        LPCSTR Description,
        LPCSTR Imp) :
            EventDateTime(Time)
    {
        // Copy the currency locally and convert to upper case
        strcpy_s(StrCurrency, Currency);
        strcpy_s(StrEventDesc, Description);
        _strupr_s(StrCurrency);

        // Save the date and time in string format
        EventDateTime.ToStringFxDate(StrEventDate);
        EventDateTime.ToStringFxTime(StrEventTime);

        // convert the importance to integer
        if (_strnicmp(Imp, "low", 3) == 0) EventImportance = 1;
        else if (_strnicmp(Imp, "medium", 6) == 0) EventImportance = 2;
        else EventImportance = 3;
    }


    LPSTR GetEventDuration() {
        CFeedTime       ftNow(FT_CURRENT);
        CFeedTimeSpan   ftDur = EventDateTime - ftNow;

        _snprintf_s(StrDuration, _countof(StrDuration), "%02d:%02d", 
            ftDur.GetHours(), (ftDur.GetTimeSpan() % 3600) / 60);

        return StrDuration;
    }

    void Dump() {
        printf("%s :: %s -> %s\n", StrEventTime, StrCurrency, StrEventDesc);
    }
};

//
// Data structure to hold the events
//
typedef FOREX_EVENT*                        PFOREX_EVENT;
typedef std::deque<PFOREX_EVENT>            FXEVENTS_QUEUE;
typedef std::deque<PFOREX_EVENT>::iterator  FXEVENTS_QUEUE_IT;



/*++

Class Name:

    CForexMgr

Class Description:

    This class is the main work horse. This class queue's up the events and 
    stores internally. It also flushes the queue periodically and downloads the
    data from the website

--*/
class CForexMgr
{
protected:
    CFeedTime       m_QueryTime;
    CHttp           m_DailyFx;
    FXEVENTS_QUEUE  m_FxEventsQueue;
    CLock           m_FxEventsQueueLock;

    // C'tor-D'tor
public:
    CForexMgr(void) :
        m_QueryTime(FT_CURRENT) { 

        CFeedTimeSpan duration(0, 0, FOREX_QUERY_INTERVAL + 1, 0);
        m_QueryTime -= duration;
    }

    ~CForexMgr(void) {
        Disconnect();
    }

    // Connection management
public:
    bool Connect(void);
    bool Disconnect(void) {
        m_DailyFx.Uninitialize();
        return true;
    }

private:
    CFeedTime ParseDateTime(LPCSTR StrDate, LPCSTR StrTime);
    void QueryForexEventsFromDailyFx(void);

    // Public interface for this class
public:
    void Dump() {
        for (FXEVENTS_QUEUE_IT fxIt = m_FxEventsQueue.begin(); 
            fxIt != m_FxEventsQueue.end(); fxIt++)
            (*fxIt)->Dump();
    }

    PFOREX_EVENT GetNextFxEvent(
        LPCSTR CurrencyPair
        );
};
