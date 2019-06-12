/*++

    Copyright (c) Pai Financials LLC

Module Name:

    DateTime.cpp

Abstract:

    The time manipulation function implementation

Author:

    Navin Pai (navinp) - 06-Oct-2015

--*/
#include "pch.h"
#include "DateTime.h"


///////////////////////////////////////////////////////////////////////////////
//
// CTimeZoneInfo
//
#define TIMEZONE_PATH               L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\"

LPCWSTR CTimeZoneInfo::SZTimeZone[] = {
    L"GMT Standard Time",
    L"Eastern Standard Time",
    L"Pacific Standard Time",
    L"India Standard Time",
};

bool CTimeZoneInfo::IsTimeZoneInit = false;
TIME_ZONE_INFORMATION CTimeZoneInfo::TimeZoneInfo[MAX_TIME_ZONES];

bool
CTimeZoneInfo::InitializeTimezones(
    void
    )
/*++
Description:
    Moved into Static function to be called before using the timzone
    class as the part of optimization. By moving this one function
    out of constructor, the test init time went from 3600 to 16
--*/
{
    if (IsTimeZoneInit)
    {
        // already initialized
        return true;
    }

    // Get timezone info for all the supported timezones
    for (int nCtr = 0; nCtr < MAX_TIME_ZONES; nCtr++)
    {
        if (nCtr == TzLocal)
        {
            if (::GetTimeZoneInformation(&TimeZoneInfo[nCtr]) == 
                TIME_ZONE_ID_INVALID)
            {
                return false;
            }
        }
        else
        {
            if (! ReadTimeZoneInfoRegistry(SZTimeZone[nCtr],
                &TimeZoneInfo[nCtr]))
            {
                return false;
            }
        }
    }

    // Everything went okay
    IsTimeZoneInit = true;
    return true;
}


bool
CTimeZoneInfo::ReadTimeZoneInfoRegistry(
    LPCWSTR lpszTimeZone,
    LPTIME_ZONE_INFORMATION lpTimeZoneInfo
    )
{
    HKEY            hKey;
    REG_TZI_FORMAT  regTzi;
    DWORD           regTziCount = sizeof(regTzi);
    WCHAR           szTimeKey[512] = TIMEZONE_PATH;
    DWORD           retVal;

    wcscat_s(szTimeKey, _countof(szTimeKey), lpszTimeZone);
    
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, szTimeKey, 
        0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        return false;
    }

    retVal = RegQueryValueExW(hKey, L"TZI", NULL, 
        NULL, (PBYTE)&regTzi, &regTziCount);
    RegCloseKey(hKey);

    if (retVal != ERROR_SUCCESS)
    {
        SetLastError(retVal);
        return false;
    }

    lpTimeZoneInfo->Bias         = regTzi.Bias;
    lpTimeZoneInfo->DaylightBias = regTzi.DaylightBias;
    lpTimeZoneInfo->DaylightDate = regTzi.DaylightDate;
    lpTimeZoneInfo->StandardBias = regTzi.StandardBias;
    lpTimeZoneInfo->StandardDate = regTzi.StandardDate;
    lpTimeZoneInfo->DaylightName[0] = L'0';
    lpTimeZoneInfo->StandardName[0] = L'0';

    return true;
}


CTimeZoneInfo::CTimeZoneInfo(ETimeZone TimeZone)
{
    if (! IsTimeZoneInit)
    {
        InitializeTimezones();
    }

    m_timeZone = TimeZone;
}


bool
CTimeZoneInfo::FromGmtToLocal(
    tm *pTm
    )
{
    SYSTEMTIME  universalTime;
    SYSTEMTIME  localTime;

    // Set universal time
    universalTime.wYear = (WORD) (1900 + pTm->tm_year);
    universalTime.wMonth = (WORD) (1 + pTm->tm_mon);
    universalTime.wDayOfWeek = (WORD) pTm->tm_wday;
    universalTime.wDay = (WORD) pTm->tm_mday;
    universalTime.wHour = (WORD) pTm->tm_hour;
    universalTime.wMinute = (WORD) pTm->tm_min;
    universalTime.wSecond = (WORD) pTm->tm_sec;
    universalTime.wMilliseconds = 0;

    // Convert to the timezone
    if (! SystemTimeToTzSpecificLocalTime(
        &TimeZoneInfo[m_timeZone], 
        &universalTime, 
        &localTime))
    {
        return false;
    }

    // Save it back
    pTm->tm_year = localTime.wYear - 1900;
    pTm->tm_mon = localTime.wMonth - 1;
    pTm->tm_mday = localTime.wDay;
    pTm->tm_wday = localTime.wDayOfWeek;
    pTm->tm_hour = localTime.wHour;
    pTm->tm_min = localTime.wMinute;
    pTm->tm_sec = localTime.wSecond;

    return true;
}


bool
CTimeZoneInfo::FromLocalToGmt(
    tm *pTm
    )
{
    SYSTEMTIME  universalTime;
    SYSTEMTIME  localTime;

    // We have the local time in pTm
    localTime.wYear = (WORD) (1900 + pTm->tm_year);
    localTime.wMonth = (WORD) (1 + pTm->tm_mon);
    localTime.wDayOfWeek = (WORD) pTm->tm_wday;
    localTime.wDay = (WORD) pTm->tm_mday;
    localTime.wHour = (WORD) pTm->tm_hour;
    localTime.wMinute = (WORD) pTm->tm_min;
    localTime.wSecond = (WORD) pTm->tm_sec;
    localTime.wMilliseconds = 0;

    // Convert to the timezone
    if (! TzSpecificLocalTimeToSystemTime(
        &TimeZoneInfo[m_timeZone], 
        &localTime, 
        &universalTime))
    {
        return false;
    }

    // Save it back
    pTm->tm_year = universalTime.wYear - 1900;
    pTm->tm_mon = universalTime.wMonth - 1;
    pTm->tm_mday = universalTime.wDay;
    pTm->tm_wday = universalTime.wDayOfWeek;
    pTm->tm_hour = universalTime.wHour;
    pTm->tm_min = universalTime.wMinute;
    pTm->tm_sec = universalTime.wSecond;

    return true;
}



///////////////////////////////////////////////////////////////////////////////
//
// DateTimeOffset Constructors
//
DateTimeOffset::DateTimeOffset(
    ETimeZone TimeZone, 
    __time32_t Time
    )
{
    // Optimization
    if (TimeZone == TzUtc)
    {
        m_gmtTime = Time;
        return;
    }

    // Check what time zone and initialize as per
    switch (TimeZone)
    {
    case TzLocal:
        {
            struct tm   tmTemp;
            _gmtime32_s(&tmTemp, &Time);        // convert to gmtime 
            m_gmtTime = _mkgmtime32(&tmTemp);
        }
        break;

    case TzEastern:
    case TzIndia:
    case TzPacific:
        {
            CTimeZoneInfo  newTz(TimeZone);
            struct tm   tmTemp;

            _gmtime32_s(&tmTemp, &Time);
            newTz.FromLocalToGmt(&tmTemp);
            m_gmtTime = _mkgmtime32(&tmTemp);
        }
        break;

    default:
        _ASSERT(true);
    }
}


DateTimeOffset::DateTimeOffset(
    ETimeZone TimeZone,
    int nYear, int nMonth, int nDay, 
    int nHour, int nMin, int nSec)
{
    struct tm   tmTemp;

    _ASSERT(nYear >= 1900);
    _ASSERT(nMonth >= 1 && nMonth <= 12);
    _ASSERT(nDay >= 1 && nDay <= 31);
    _ASSERT(nHour >= 0 && nHour <= 23);
    _ASSERT(nMin >= 0 && nMin <= 59);
    _ASSERT(nSec >= 0 && nSec <= 59);

    tmTemp.tm_sec = nSec;
    tmTemp.tm_min = nMin;
    tmTemp.tm_hour = nHour;
    tmTemp.tm_mday = nDay;
    tmTemp.tm_mon = nMonth - 1;        // tm_mon is 0 based
    tmTemp.tm_year = nYear - 1900;     // tm_year is 1900 based
    tmTemp.tm_isdst = -1;

    // Optimization
    if (TimeZone == TzUtc)
    {
        m_gmtTime = _mkgmtime32(&tmTemp);
        _ASSERT(m_gmtTime != -1);
        return;
    }

    switch (TimeZone)
    {
    case TzLocal:
        {
            m_gmtTime = _mktime32(&tmTemp);
            _gmtime32_s(&tmTemp, &m_gmtTime);   // convert to gmtime 
            m_gmtTime = _mkgmtime32(&tmTemp);
        }
        break;

    case TzEastern:
    case TzIndia:
    case TzPacific:
        {
            CTimeZoneInfo  newTz(TimeZone);

            newTz.FromLocalToGmt(&tmTemp);
            m_gmtTime = _mkgmtime32(&tmTemp);
        }
        break;

    default:
        _ASSERT(true);
    }
    
    _ASSERT(m_gmtTime != -1);
}


///////////////////////////////////////////////////////////////////////////////
//
// DateTimeOffset Properties
//
UINT32
DateTimeOffset::GetTzTime(
    ETimeZone TimeZone
    )
{
    struct tm   tmTemp;

    if (TimeZone == TzLocal)
    {
        _localtime32_s(&tmTemp, &m_gmtTime);
        return _mktime32(&tmTemp);
    }
    else
    {
        CTimeZoneInfo  newTz(TimeZone);

        _gmtime32_s(&tmTemp, &m_gmtTime);
        newTz.FromGmtToLocal(&tmTemp);
        return _mkgmtime32(&tmTemp);
    }
}


UINT32
DateTimeOffset::GetUtcStartOfTradingDay(
    void
    )
{
    // BUG:
    struct tm       tmTemp;
    CTimeZoneInfo   nyseTz(TzEastern);
    __time32_t      easternTime;
    
    // Get the eastern time
    easternTime = GetTzTime(TzEastern);

    // point it to the start of day
    easternTime -= (easternTime % TIME_IN_SECS(24, 0, 0));
    easternTime += TIME_IN_SECS(9, 30, 0);

    // Convert eastern to utc time
    _gmtime32_s(&tmTemp, &easternTime);
    nyseTz.FromLocalToGmt(&tmTemp);

    return _mkgmtime32(&tmTemp);
}


UINT32
DateTimeOffset::GetUtcEndOfTradingDay(
    void
    )
{
    // BUG:
    struct tm       tmTemp;
    CTimeZoneInfo   nyseTz(TzEastern);
    __time32_t      easternTime;
    
    // Get the eastern time
    easternTime = GetTzTime(TzEastern);

    // point it to the start of day
    easternTime -= (easternTime % TIME_IN_SECS(24, 0, 0));
    easternTime += TIME_IN_SECS(16, 00, 0);

    // Convert eastern to utc time
    _gmtime32_s(&tmTemp, &easternTime);
    nyseTz.FromLocalToGmt(&tmTemp);

    return _mkgmtime32(&tmTemp);
}


UINT32
DateTimeOffset::GetUtcBeginOfWeek(
    void
    )
/*+

Abstract

    This function returns the monday as the begin of week

-*/
{
    struct tm   tmTemp;

    _localtime32_s(&tmTemp, &m_gmtTime);

    if (tmTemp.tm_wday == 0)
    {
        // its sunday, add 24 hrs to make it monday
        return m_gmtTime + TIME_IN_SECS(24, 0 , 0);
    }
    else if (tmTemp.tm_wday == 1)
    {
        // its monday, do nothing
        return m_gmtTime;
    }
    else 
    {
        // subtract 1 day for tues, 2 for wed,--- 5 days for sat
        return m_gmtTime - (TIME_IN_SECS(24, 0, 0) * (tmTemp.tm_wday - 1));
    }
}


UINT32
DateTimeOffset::GetUtcEndOfWeek(
    void
    )
{
    struct tm   tmTemp;

    _localtime32_s(&tmTemp, &m_gmtTime);

    if (tmTemp.tm_wday == 6)
    {
        // its saturday, sub 24 hrs to make it friday
        return m_gmtTime - TIME_IN_SECS(24, 0 , 0);
    }
    else if (tmTemp.tm_wday == 5)
    {
        // its friday, do nothing
        return m_gmtTime;
    }
    else 
    {
        // add 5 day for sunday, 4 for monday,--- 1 day for thus
        return m_gmtTime + (TIME_IN_SECS(24, 0, 0) * (5 - tmTemp.tm_wday));
    }
}


bool
DateTimeOffset::IsNyseClosed()
{
    struct tm   tmTemp;
    CTimeZoneInfo  nyseTz(TzEastern);

    _gmtime32_s(&tmTemp, &m_gmtTime);
    nyseTz.FromGmtToLocal(&tmTemp);

    // Closed on saturday and sunday. Add more holidays
    // 0 = sunday, 1 = monday, ... 6 = saturday
    if ((tmTemp.tm_wday == 0) || (tmTemp.tm_wday == 6)) 
        return true;

    // New year's day (Jan 1) Closed every year
    // If new years day on Sunday then we observe it on monday
    // If its on saturday then its not observed on 31st
    if ((tmTemp.tm_mon == 0) && (tmTemp.tm_mday == 1)) 
        return true;
    if ((tmTemp.tm_mon == 0) && (tmTemp.tm_mday == 2) && (tmTemp.tm_wday == 1))
        return true;

    // Martin luther king, Jr. day (Jan 15) Closed for 1 min of silence 
    // at noon since 1986. Closed all day beginning in 1998
    // observed on third monday of jan
    if ((tmTemp.tm_year >= 98) && (tmTemp.tm_mon == 0) && (tmTemp.tm_wday == 1) && 
        ((tmTemp.tm_mday - 14) >= 1) && ((tmTemp.tm_mday - 14) <= 7)) 
        return true;

    // Washington's bday/President's day. Closed every year. Observed on
    // third mondays since 1971
    if ((tmTemp.tm_year >= 71) && (tmTemp.tm_mon == 1) && (tmTemp.tm_wday == 1) &&
        ((tmTemp.tm_mday - 14) >= 1) && ((tmTemp.tm_mday - 14) <= 7))
        return true;

    //// Good friday. Closed every year except 1898, 1906 and 1907
    //// Good friday calculation is crazy
    {
        int goldenNumber = (tmTemp.tm_year + 1900) % 19;    // position of yr in 19 yr lunar cycle
        int currCentury = (tmTemp.tm_year + 1900) / 100;    // century we are in
        int yearNumber = (tmTemp.tm_year + 1900) % 100;     // year within the current century
        int leapCentury = currCentury / 4;                  // leap centuries
        int leapCenturyNumber = currCentury % 4;            // year within the leap centuries
        int leapYear = yearNumber / 4;                      
        int leapYearNumber = yearNumber % 4;

        int correctionFactor = (8 * currCentury + 13) / 25;
        int daysAfterEquinox = ((19 * goldenNumber) + currCentury - leapCentury - correctionFactor + 15) % 30;
        int fullmoonDayAfterSunday = ((2 * leapCenturyNumber) + (2 * leapYear) - leapYearNumber + 32 - daysAfterEquinox) % 7;
        int fudgeFactor = (goldenNumber + (11 * daysAfterEquinox) + (19 * fullmoonDayAfterSunday)) / 433;

        int daysToGoodFriday = daysAfterEquinox + fullmoonDayAfterSunday - (7 * fudgeFactor) - 2;
        int goodFridayMonth = (daysToGoodFriday + 90) / 25;
        int goodFridayDay = (daysToGoodFriday + (33 * goodFridayMonth) + 19) % 32;

        if ((tmTemp.tm_mon == goodFridayMonth - 1) && (tmTemp.tm_mday == goodFridayDay))
            return true;
    }

    // Decoration/Memorial day (May 30). Closed every year since 1873
    // Observed on last monday of May
    if ((tmTemp.tm_mon == 4) && (tmTemp.tm_wday == 1) &&
        ((tmTemp.tm_mday - 21) >= 4) && ((tmTemp.tm_mday - 21) <= 10))
        return true;

    // Independence day (July 4). Closed every year
    // if Jul 4 is on saturday then we move it to friday, 
    // if on sunday, we move it to monday
    if ((tmTemp.tm_mon == 6) && (tmTemp.tm_mday == 3) && (tmTemp.tm_wday == 5))
        return true;
    if ((tmTemp.tm_mon == 6) && (tmTemp.tm_mday == 5) && (tmTemp.tm_wday == 1))
        return true;
    if ((tmTemp.tm_mon == 6) && (tmTemp.tm_mday == 4))
        return true;

    // Labor day. Closed every year 1887 - date. First monday
    // of September
    if ((tmTemp.tm_mon == 8) && (tmTemp.tm_wday == 1) && (tmTemp.tm_mday <= 7))
        return true;

    //// Columbus day (Oct 12) Closed every year 1909 - 1953. 
    //// Not implementing. Too old

    //// Election day. Closed every year thru 1968.
    //// Closed presidential election years only 1972-1980.
    //// Not implementing. Too old

    //// Veteran's day (Nov 11). Closed all day 1918-1921
    //// Closed for 2 mins 1922-1933. Closed all day 1934-1953
    //// Closed for 2 mins 1954 - present
    //// Not implementing. Too old

    // Thanksgiving day. fourth thusday of November
    // Closed every year
    if ((tmTemp.tm_mon == 10) && (tmTemp.tm_wday == 4) && 
        ((tmTemp.tm_mday - 21) >= 1) && ((tmTemp.tm_mday - 21) <= 7))
        return true;

    // Christmas day (Dec 25). Closed every year
    // if Dec 25 is on saturday then we move it to friday, 
    // if on sunday, we move it to monday
    if ((tmTemp.tm_mon == 11) && (tmTemp.tm_mday == 24) && (tmTemp.tm_wday == 5))
        return true;
    if ((tmTemp.tm_mon == 11) && (tmTemp.tm_mday == 26) && (tmTemp.tm_wday == 1))
        return true;
    if ((tmTemp.tm_mon == 11) && (tmTemp.tm_mday == 25))
        return true;

    // Some special cases
    // Day of mourning - Gerald Ford
    if ((tmTemp.tm_year == 107) && (tmTemp.tm_mon == 0) && (tmTemp.tm_mday == 2))
        return true;
    // Presidential funeral - Ronald  Reagan
    if ((tmTemp.tm_year == 104) && (tmTemp.tm_mon == 5) && (tmTemp.tm_mday == 11))
        return true;
    // WTC 9/11
    if ((tmTemp.tm_year == 101) && (tmTemp.tm_mon == 8) && (tmTemp.tm_mday >= 11) && (tmTemp.tm_mday <= 14))
        return true;
    // Presidential funeral - Richard Nixon
    if ((tmTemp.tm_year == 94) && (tmTemp.tm_mon == 3) && (tmTemp.tm_mday == 27))
        return true;

    return false;
}


bool
DateTimeOffset::IsNyseRegularHours(
    PUINT32 Index
    )
/*++
Description:

    Index is valid only if return is true.

--*/
{
    UINT32 easternTime;
    UINT32 secondsSinceMidnight;

    easternTime = GetTzTime(TzEastern);
    secondsSinceMidnight = easternTime % TIME_IN_SECS(24, 0, 0);

    if (Index != NULL) *Index = (secondsSinceMidnight - NYSE_START_TIME) / 60;

    return (secondsSinceMidnight >= NYSE_START_TIME && 
        secondsSinceMidnight < NYSE_END_TIME);
}


bool
DateTimeOffset::IsNyse930AM(
    void
    )
/*++
Description:

--*/
{
    UINT32 easternTime;
    UINT32 secondsSinceMidnight;

    easternTime = GetTzTime(TzEastern);
    secondsSinceMidnight = easternTime % TIME_IN_SECS(24, 0, 0);

    return secondsSinceMidnight == NYSE_START_TIME;
}


bool
DateTimeOffset::IsNyse359PM(
    void
    )
/*++
Description:

--*/
{
    UINT32 easternTime;
    UINT32 secondsSinceMidnight;

    easternTime = GetTzTime(TzEastern);
    secondsSinceMidnight = easternTime % TIME_IN_SECS(24, 0, 0);

    return secondsSinceMidnight == TIME_IN_SECS(15, 59, 0);
}


int 
DateTimeOffset::FormatW(
    ETimeZone TimeZone, 
    LPCWSTR pszFormat, 
    LPWSTR TimeString, 
    DWORD Length)
{
    struct tm       tmTemp;

    TimeString[0] = L'\0';
    switch (TimeZone)
    {
    case TzLocal:
        _localtime32_s(&tmTemp, &m_gmtTime);
        break;

    case TzUtc:
        _gmtime32_s(&tmTemp, &m_gmtTime);
        break;

    case TzIndia:
    case TzEastern:
    case TzPacific:
        {
            CTimeZoneInfo  newTz(TimeZone);

            _gmtime32_s(&tmTemp, &m_gmtTime);
            newTz.FromGmtToLocal(&tmTemp);
        }
        break;

    default:
        _ASSERT(true);
        break;
    }

    return (int)wcsftime(TimeString, Length, pszFormat, &tmTemp);
}


int 
DateTimeOffset::FormatA(
    ETimeZone TimeZone, 
    LPCSTR pszFormat,
    LPSTR TimeString, 
    DWORD Length)
{
    struct tm   tmTemp;

    switch (TimeZone)
    {
    case TzLocal:
        _localtime32_s(&tmTemp, &m_gmtTime);
        break;

    case TzUtc:
        _gmtime32_s(&tmTemp, &m_gmtTime);
        break;

    case TzIndia:
    case TzEastern:
    case TzPacific:
        {
            CTimeZoneInfo  newTz(TimeZone);

            _gmtime32_s(&tmTemp, &m_gmtTime);
            newTz.FromGmtToLocal(&tmTemp);
        }
        break;

    default:
        _ASSERT(true);
        break;
    }

    return (int)strftime(TimeString, Length, pszFormat, &tmTemp);
}

bool
DateTimeOffset::FromStringWeb(
    __in LPCSTR DateString)
/*++

Routine Description:

    This function parses the date and time string passed and 
    convert it into CTime object

Parameters:

    DateString - The date in "MMM DD" format

Return Value:

    true - if everything was parsed correctly
    false - if there was error parsing data

--*/
{
    static const char* sMonths[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
    "Aug", "Sep", "Oct", "Nov", "Dec" };
    UINT day, month = 0;

    //
    // Convert month to number
    //
    for (int nCtr = 0; nCtr < _countof(sMonths); nCtr++)
    {
        if (_strnicmp(DateString, sMonths[nCtr], 3) == 0)
        {
            month = nCtr + 1;
            break;
        }
    }

    if (month == 0) return false;

    //
    // Convert the date
    //
    day = atoi(DateString + 3);

    //
    // Year is tricky. 
    //
    DateTimeOffset ftToday(FT_CURRENT_UTC);
    DateTimeOffset ftEarnings(TzEastern, ftToday.GetLocalYear() + 1, month, day);

    DateTimeSpan diff = ftEarnings - ftToday;
    if (diff.GetDays() > 100)
    {
        ftEarnings = DateTimeOffset(TzEastern, ftToday.GetLocalYear(), month, day);
    }

    //
    // Assign the parsed date
    //
    *this = ftEarnings;

    return true;
}


bool
DateTimeOffset::FromStringStd(
    __in LPCSTR DateString)
/*++

Routine Description:

    This function parses the date and time string passed and 
    convert it into CTime object

Parameters:

    DateString - The date in "MM/DD/YYYY" format eg. 10-11-1998
    TimeString - Contains before, after, during
    EarningsDate - Return the date

Return Value:

    true - if everything was parsed correctly
    false - if there was error parsing data

--*/
{
    int nMonth = 0, nDate = 0, nYear = 0;

    // Length of date string not correct
    if (strlen(DateString) < 5) return false;

    // Initialize the return date
    if (sscanf_s(DateString, "%02d/%02d/%04d", &nMonth, &nDate, &nYear) != 3)
        return false;

    if (nMonth < 1 || nMonth > 12) return false;
    if (nDate < 1 || nDate > 31) return false;
    if (nYear < 1900) return false;

    // Assign the parsed date, The parsed date is in eastern timezone
    // so we convert it to GMT
    *this = DateTimeOffset(TzEastern, nYear, nMonth, nDate);

    return true;
}
