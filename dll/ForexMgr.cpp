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

    ForexMgr.cpp

Abstract:

    All forex related functions are implemented in this file

Author:

    nabieasaurus

--*/
#include "StdAfx.h"
#include "ForexMgr.h"

#define USER_AGENT_STRING       "UserAgent:  Mozilla/4.0 (compatible; MSIE 8.0)"
#define WWW_DAILYFX             "www.dailyfx.com"
#define DAILYFX_CALENDAR        "/files/Calendar-%02d-%02d-%04d.xls"


static
LPCSTR  StrMonths[] = { 
    "Jan", "Feb", "Mar", "Apr", 
    "May", "Jun", "Jul", "Aug", 
    "Sep", "Oct", "Nov", "Dec", 
};

static
LPCSTR  StrDays[] = {
    "Sun", "Mon", "Tue", "Wed",
    "Thu", "Fri", "Sat"
};


bool
CForexMgr::Connect(void)
/*++

Abstract:

    Connects to the DailyFx.com website and initializes the http connection

--*/
{
    return m_DailyFx.InitializeA(USER_AGENT_STRING, WWW_DAILYFX);
}



CFeedTime
CForexMgr::ParseDateTime(
    LPCSTR StrDate,
    LPCSTR StrTime
    )
/*++

Abstract:

    Parses the date and time that is received from DailyFx in CSV format

Parameters:

    StrDate -   Date in format DDD MMM DD eg. (Mon Mar 04)

    StrTime -   Time in 24 hrs format. eg. 01:23 or 23:12 etc

--*/
{
    CHAR        szDay[32], szMon[32];
    UINT        nMon = 0, nDay, nHrs, nMins, nDofW = 0;
    CFeedTime   ftRet(TzUtc, 0);

    //
    // Parse the date
    //
    if (sscanf_s(StrDate, "%s %s %d", szDay, _countof(szDay), 
        szMon, _countof(szMon), &nDay) != 3)
    {
        goto Cleanup;
    }

    for (int nCtr = 0; nCtr < _countof(StrMonths); nCtr++)
    {
        if (_strnicmp(StrMonths[nCtr], szMon, 3) == 0)
        {
            nMon = nCtr + 1;
            break;
        }
    }

    if (nMon == 0) goto Cleanup;

    for (int nCtr = 0; nCtr < _countof(StrDays); nCtr++)
    {
        if (_strnicmp(StrDays[nCtr], szDay, 3) == 0)
        {
            nDofW = nCtr;
            break;
        }
    }

    //
    // Parse the time
    //
    if (sscanf_s(StrTime, "%d:%d", &nHrs, &nMins) != 2)
    {
        //
        // If time is not specified then there will be error in 
        // time parsing, In that case just use the date and 
        // and mark time as 00:01
        //
        nHrs = 0;
        nMins = 1;
    }

    {
        //
        // Calculate the date/time of the event from the start of the week.
        // we are calculating to prevent wrap-around date calculations when the
        // week is split between two years
        //
        CFeedTime       startOfWeek(FT_CURRENT);
        CFeedTimeSpan   diffTime(startOfWeek.GetUtcDayOfWeek() - 1, 0, 0, 0);
        startOfWeek     -= diffTime;
        ftRet           = CFeedTime(TzUtc, startOfWeek.GetUtcYear(), startOfWeek.GetUtcMonth(), startOfWeek.GetUtcDay());
        CFeedTimeSpan   eventTime(nDofW, nHrs, nMins, 0);
        ftRet           += eventTime;
    }

Cleanup:

    return ftRet;
}


void
CForexMgr::QueryForexEventsFromDailyFx(
    void
    )
/*++

Abstract:
    Queries DailyFx.com and retieves the CSV format events for the entire week
    and then parses and creates a queue of the events.

    
--*/
{
    String      httpString;
    CHAR        chBuffer[512];
    CFeedTime   currentTime(FT_CURRENT);
    CFeedTime   startOfWeek(currentTime);


    //
    // Create the query for the weekly events
    //
    CFeedTimeSpan   diffTime(startOfWeek.GetUtcDayOfWeek() - 1, 0, 0, 0);
    startOfWeek     -= diffTime;

    sprintf_s(chBuffer, DAILYFX_CALENDAR, startOfWeek.GetUtcMonth(),
        startOfWeek.GetUtcDay(), startOfWeek.GetUtcYear());

    //
    // Send the get request
    //
    if (m_DailyFx.SendGetRequestA(chBuffer) == false)
    {
        LogError("Unable to send request");
        goto Cleanup;
    }

    //
    // Receive the response for our request
    //
    if (m_DailyFx.RecvResponse(httpString) == false)
    {
        LogError("Failed to receive response");
        goto Cleanup;
    }

    //
    // Clear the cache, Parse the response and populate the earnings queue
    //
    LPSTR       szHttpData = (LPSTR) httpString.c_str();
    int         lineCtr = 0;
    
    m_FxEventsQueue.clear();

    while (szHttpData != NULL)
    {
        LPSTR szDataLine = szHttpData;
        LPSTR szNextLine = strchr(szHttpData, 0x0a);

        if (szNextLine != NULL) *szNextLine++ = '\0';
        
        lineCtr++;
        szHttpData = szNextLine;
        
        // Skip first line and lines with few chars
        if ((lineCtr == 1) || (strlen(szDataLine) < 15))
        {
            continue;
        }

        //
        // Parse the extracted data line
        //
        LPSTR           szValue[7];
        bool            bLineParsed = true;

        szValue[0] = szDataLine;
        for (int nCtr = 1; nCtr < _countof(szValue); nCtr++)
        {
            LPSTR pNext = (LPSTR) strchr(szValue[nCtr - 1], ',');
            if (pNext == NULL)
            {
                LogError("Unable to parse input line");
                bLineParsed = false;
                break;
            }

            *pNext++ = '\0';
            StrTrimA(pNext, " \t\r\n");
            szValue[nCtr] = pNext;
        }

        //
        // The line was not parsed correctly, continue to next line
        //
        if (bLineParsed == false) continue;

        // 
        // The line was parsed successfuly, Create forex event from the
        // parsed data value only if the event time is greater than the current time 
        // value (there is no need to save the old event in the queue)
        //
        CFeedTime fxDate = ParseDateTime(szValue[0], szValue[1]);

        if (fxDate >= currentTime)
        {
            PFOREX_EVENT ptrFxEvent = new FOREX_EVENT(fxDate, szValue[3], szValue[4], szValue[5]);
            if (ptrFxEvent != NULL)
            {
                //ptrFxEvent->Dump();
                m_FxEventsQueue.push_back(ptrFxEvent);
            }
        }
    }

    //
    // Parsing successful. Update the query time
    //
    m_QueryTime = currentTime;

Cleanup:

    return;
}



PFOREX_EVENT
CForexMgr::GetNextFxEvent(
    LPCSTR CurrencyPair
    )
/*++

Abstract:
    Retrieves the next event that is related to the currency pair from the events list.
    Also periodically downloads the events from DailyFx even if the data is cached. This is
    done so that we get fresh data every interval specified.

--*/
    
{
    CFeedTime       currentTime(FT_CURRENT);
    CFeedTimeSpan   timeDiff = currentTime - m_QueryTime;
    CHAR            szCurPair[12];
    
    CAutoLock   al(m_FxEventsQueueLock);

    //
    // If the last query has expired, requery
    //
    if (timeDiff.GetMins() > FOREX_QUERY_INTERVAL)
    {
        LogTrace("Previous query expired: Requery.");

        //
        // Requery. The data that we receive from DailyFx 
        // is already sorted so no need to sort
        //
        QueryForexEventsFromDailyFx();
    }

    //
    // Return the next event from the queue that matches
    // the currency pair
    //
    strcpy_s(szCurPair, CurrencyPair);
    _strupr_s(szCurPair);

    PFOREX_EVENT retVal = NULL;

    for (FXEVENTS_QUEUE_IT fxIt = m_FxEventsQueue.begin(); fxIt != m_FxEventsQueue.end(); fxIt++)
    {
        if (retVal == NULL)
        {
            //
            // Try to find one event that corresponds with currency pair
            //
            if ((strstr(szCurPair, (*fxIt)->StrCurrency) != NULL) &&
                ((*fxIt)->EventDateTime >= currentTime))
            {
                retVal = *fxIt;
                if (retVal->EventImportance >= 3)
                    break;
            }
        }
        else
        {
            //
            // Check if there are events that are at the same time with
            // higher priority
            //
            if ((strstr(szCurPair, (*fxIt)->StrCurrency) != NULL) &&
                (retVal->EventDateTime == (*fxIt)->EventDateTime) &&
                (retVal->EventImportance < (*fxIt)->EventImportance))
            {
                retVal = *fxIt;
            }
            else
            {
                break;
            }
        }
    }


    return retVal;
}