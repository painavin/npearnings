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

    EarningsMgr.h

Abstract:

    This file contains the declarations for earnings release manager.

Author:

    nabieasaurus

--*/
#pragma once
#include "FeedTime.h"
#include "HttpHelper.h"
#include "Lock.h"

extern bool gResetData;

/*++

Class:

    CEarningsData

Description:

    This structure stores the earnings information about the ticker. An instance
    of this class represents one ticker symbol.

--*/
class CEarningsData
{
public:
    bool        IsAvailable;            // If the earnings data is available on the website
    bool        IsConfirmed;            // If the earnings release time and date are confirmed as per website
    bool        ReQuery;                // If we have to query for the data again. By default we do not query again

public:
    String      StrTicker;              // The ticker symbol     
    String      StrEarningsDate;        // The earnings date in string format
    String      StrEarningsTime;        // The earnings time in string format
    String      StrEarningsDays;        // Days to earnings release
    String      StrEarningsNotes;       // The notes from the earnings file

private:
    CFeedTime   QueryDate;              // The date when we last retrieved the data from website
    CFeedTime   EarningsDate;           // The actual date of earnings release


    // Constructors
public:

    CEarningsData(_In_ LPCSTR Ticker, 
        _In_ bool Available = false,
        _In_ UINT32 QueryDt = 0,
        _In_ UINT32 EarningsDt = 0,
        _In_ LPCSTR EarningsTime = "",
        _In_ bool Confirmed = false,
        _In_ LPCSTR Note = "") :
            StrTicker(Ticker), 
            IsAvailable(Available), 
            QueryDate(TzUtc, QueryDt), 
            EarningsDate(TzUtc, EarningsDt), 
            StrEarningsTime(EarningsTime), 
            IsConfirmed(Confirmed), 
            StrEarningsNotes(Note) 
    {
        CHAR szTemp[128];
        ReQuery = false;
        EarningsDate.ToStringLong(szTemp);
        StrEarningsDate = szTemp;
    }

    void UpdateStrEarningsDays() 
    {
        CHAR            strEarningsDays[64];

        // Get local time and convert it to eastern time (Assumption here is that what is in local time
        // zone is also in the eastern timezone
        CFeedTime       ltToday(FT_CURRENT);
        CFeedTime       ltToday2(TzEastern, ltToday.GetLocalYear(), ltToday.GetLocalMonth(), ltToday.GetLocalDay());
        CFeedTimeSpan   diff = EarningsDate - ltToday2;

        sprintf_s(strEarningsDays, "%02d Days", diff.GetDays());
        StrEarningsDays = strEarningsDays;
    }

    inline void SetQueryDate(_In_ CFeedTime QDate) {
        QueryDate = QDate;
    }

    void SetEarningsDate(_In_ CFeedTime& EDate) {
        CHAR szTemp[128];
        EarningsDate = EDate;
        EarningsDate.ToStringLong(szTemp);
        StrEarningsDate = szTemp;
    }
    
    void CheckForRequery(_In_ INT LineCtr, _In_ INT EarningsQueryDays,
        _In_ INT PostEarningsDays, _In_ INT EarningsRandDays)
    {
        CFeedTime       todaysDate(FT_CURRENT);
        CFeedTimeSpan   modDiff = todaysDate - QueryDate;

        if (this->IsAvailable == false) { return; }

        //
        // If it has been long time since the last query then query again
        //
        if (modDiff.GetDays() > EarningsQueryDays + LineCtr % EarningsRandDays)
        {
            LogInfo("Stale data. Marked for requery: %s",
                this->StrTicker.c_str());
            this->ReQuery = true;
            return;
        }

        CFeedTimeSpan   earnDiff = this->EarningsDate - todaysDate;

        //
        // If it is N days past earnings, re-query for new data
        //
        if (earnDiff.GetDays() < (-1 * PostEarningsDays))
        {
            LogInfo("Past Earnings. Marked for requery: %s",
                this->StrTicker.c_str());
            this->ReQuery = true;
            return;
        }
    }

    template <size_t Size>
    int ToString(CHAR(&Buffer)[Size])
    {
        CHAR szQDate[64], szEDate[64];

        QueryDate.ToStringStd(szQDate);
        EarningsDate.ToStringStd(szEDate);

        return sprintf_s(Buffer, "%d,%s,%s,%s,%s,%d,%s\n",
            IsAvailable, StrTicker.c_str(), szQDate, szEDate,
            StrEarningsTime.c_str(), IsConfirmed,
            StrEarningsNotes.c_str());
    }
};

typedef CEarningsData*                      CEarningsDataPtr_t;
typedef std::map<String, CEarningsDataPtr_t>    EARNINGS_MAP;


/*++

Class Name:

    CEarningsMgr

Class Description:

    This class is the main work horse. It loads and saves the earnings
    cache file and if the data is not in the cache then queries the 
    internet site for all the earnings data.

--*/
class CEarningsMgr
{
protected:
    CHttp               m_EarningsSite;
    
    EARNINGS_MAP        m_EarningsCache;
    CLock               m_EarningsCacheLock;

public:
    bool                m_bConnected;
    bool                m_bCacheDirty;      // If true then we have to write the cache on exit

    // Internet functions
protected:

    //
    // Extract the value element for the attribute
    //
    bool ExtractAttributeValue(
        _In_ LPCSTR StrAttrib,
        _Inout_ String& StrInput,
        _Inout_ String& StrValue
        );

    //
    // Extract the attributes from the tags
    //
    bool ExtractAttribute(
        _In_ LPCSTR StrAttrib,
        _Inout_ String& StrInput,
        _Inout_ String& StrOutput
        );

    //
    // Extract the value between the tags
    //
    bool ExtractValue(
        _In_ String& StrInput, 
        _Out_ String& StrOutput
        );

    //
    // Extract the Tag value from the html source
    //
    bool ExtractTag(
        _In_ LPCSTR StrTag,
        _In_ String& StrInput,
        _Inout_ String::size_type& InOutPos,
        _Out_ String& StrOutput
        );

    //
    // Converts / = %2F etc
    //
    void ConvertToHex(
        _Inout_ String& Str
        );

    //
    // Retrieves the next earnings date from the html source
    //
    bool ParseHtmlForEarningsDate(
        _Inout_ String& HtmlPage,
        _Inout_ CEarningsDataPtr_t PtrEarningsData
        );

    //
    // Query the earnings data from datasource
    //
    void QueryEarningsFromWebsite(
        _Inout_ CEarningsDataPtr_t PtrEarningsData
        );

    // C'tor/D'tor
public:
    CEarningsMgr(void);
    ~CEarningsMgr(void);

    // Connection management
public:
    bool Connect(void);

    bool Disconnect(void) {
        m_EarningsSite.Uninitialize();
        return true;
    }


public:

    //
    // Load data from the cache file
    //
    bool LoadEarningsData(
        _In_ LPCSTR FileName,
        _In_ INT EarningsQueryDays,
        _In_ INT PostEarningsDays,
        _In_ INT EarningsRandDays
        );

    //
    // Save data to the cache file
    //
    bool SaveEarningsData(
        _In_ LPCSTR FileName
        );

    //
    // Retrieve the earnings data for the ticker
    //
    CEarningsDataPtr_t GetEarningsData(
        _In_ LPCSTR Ticker
        );

};


