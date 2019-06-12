/*++

    Copyright (c) 2015 Pai Financials LLC

Module Name:

    DateTime.h

Abstract:

    The time manipulation function declarations

Author:

    Navin Pai (navin.pai@outlook.com) - 06-Oct-2015

--*/
#pragma once

#define MAX_TIME_ZONES              (TzLocal + 1)
#define DATE(_Y, _M, _D)            _Y, _M, _D
#define TIME(_H, _M, _S)            _H, _M, _S
#define DAYS(_D, _H, _M, _S)        _D, _H, _M, _S

#define MINUTES_IN_TRADING_DAY      (6 * 60 + 30)       // 6hrs 30min
#define TIME_IN_SECS(_H, _M, _S)    ((((_H * 60) + _M) * 60) + _S)
#define NYSE_START_TIME             TIME_IN_SECS(9, 30, 0)
#define NYSE_END_TIME               TIME_IN_SECS(16, 00, 0)
#define FT_CURRENT_UTC              TzUtc, _time32(NULL)

// Supported timezones
enum ETimeZone
{
    TzUtc,
    TzEastern,
    TzPacific,
    TzIndia,
    TzLocal,    // Local should be last since it is also used as last count
};


///////////////////////////////////////////////////////////////////////////////
// class CTimeZoneInfo
//
//  The helper class to convert the timezone 
//
class CTimeZoneInfo
{
private:

    typedef struct _REG_TZI_FORMAT
    {
        LONG Bias;
        LONG StandardBias;
        LONG DaylightBias;
        SYSTEMTIME StandardDate;
        SYSTEMTIME DaylightDate;
    } REG_TZI_FORMAT;

    static LPCWSTR                  SZTimeZone[];
    static TIME_ZONE_INFORMATION    TimeZoneInfo[MAX_TIME_ZONES];
    static bool                     IsTimeZoneInit;
    
    static bool ReadTimeZoneInfoRegistry(LPCWSTR lpszTimeZone, 
        LPTIME_ZONE_INFORMATION lpTimeZoneInfo);

public:
    static bool InitializeTimezones(void);

private:
    ETimeZone   m_timeZone;

public:
    CTimeZoneInfo(ETimeZone TimeZone);

    bool FromGmtToLocal(__inout tm *pTm);
    bool FromLocalToGmt(__inout tm *pTm);
};



///////////////////////////////////////////////////////////////////////////////
// class DateTimeSpan
//
//  The helper class to do the time arithematic. Since the value is 
//  represented as the time difference, it is expressed in terms of units 
//  instead of regular format
//
class DateTimeSpan
{
private:
    __time32_t  m_timeSpan;

    // Constructors
public:
    DateTimeSpan(void) : m_timeSpan(0) { }

    DateTimeSpan(__time32_t time) : m_timeSpan(time) { }
 
    DateTimeSpan(int nDays, int nHours, int nMins, int nSec) {
        m_timeSpan = nSec + 60 * (nMins + 60 * (nHours + (24 * nDays)));
    }

    // Properties
public:
    inline __time32_t GetTimeSpan() const {
        return m_timeSpan;
    }

    inline int GetDays() {
        return (m_timeSpan / (24 * 3600));
    }

    inline int GetHours() {
        return (m_timeSpan / 3600);
    }

    inline int GetMins() {
        return (m_timeSpan / 60);
    }
};



///////////////////////////////////////////////////////////////////////////////
// class DateTimeOffset
//
//  Time manipulation class. The time is always stored in GMT time zone. If 
//  a local time or any other time zone is passed to the DateTimeOffset while 
//  construction, the time is converted to the GMT time zone.
//
class DateTimeOffset
{
public:
    enum {
        ftSunday = 1,
        ftMonday,
        ftTuesday,
        ftWednesday,
        ftThusday,
        ftFriday,
        ftSaturday
    };

private:
    __time32_t  m_gmtTime;  // Time in UTC

    // Constructors
public:
    DateTimeOffset(void) : m_gmtTime(0) { }

    DateTimeOffset(ETimeZone TimeZone, __time32_t Time);

    DateTimeOffset(ETimeZone TimeZone, int nYear, int nMonth, int nDay, 
        int nHour = 0, int nMin = 0, int nSec = 0);

    // Operators
public:
    inline DateTimeOffset& operator += (DateTimeSpan span) {
        m_gmtTime += span.GetTimeSpan();
        return *this;
    }

    inline DateTimeOffset& operator -= (DateTimeSpan span) {
        m_gmtTime -= span.GetTimeSpan();
        return *this;
    }
    
    inline DateTimeSpan operator - (DateTimeOffset time) {
        return DateTimeSpan(m_gmtTime - time.m_gmtTime);
    }

    inline DateTimeOffset operator - (DateTimeSpan span) {
        return DateTimeOffset(TzUtc, m_gmtTime - span.GetTimeSpan());
    }

    inline DateTimeOffset operator + (DateTimeSpan span) {
        return DateTimeOffset(TzUtc, m_gmtTime + span.GetTimeSpan());
    }

    inline bool operator <= (DateTimeOffset time) {
        return (m_gmtTime <= time.m_gmtTime);
    }

    inline bool operator < (DateTimeOffset time) {
        return (m_gmtTime < time.m_gmtTime);
    }

    inline bool operator >= (DateTimeOffset time) {
        return (m_gmtTime >= time.m_gmtTime);
    }

    inline bool operator > (DateTimeOffset time) {
        return (m_gmtTime > time.m_gmtTime);
    }

    inline bool operator == (DateTimeOffset time) {
        return (m_gmtTime == time.m_gmtTime);
    }

    // Properties
public:
    inline void SetUtcTime(__time32_t UtcTime) {
        m_gmtTime = UtcTime;
    }

    inline UINT32 GetUtcTime() {
        return m_gmtTime;
    }

    UINT32 GetTzTime(ETimeZone TimeZone);

    inline UINT32 GetUtcFloorMin() {
        struct tm   atm;
        _gmtime32_s(&atm, &m_gmtTime);
        atm.tm_sec = 0;
        return _mkgmtime32(&atm);
    }

    inline UINT32 GetUtcDayIndex() {
        //struct tm   tmIdx;
        //_gmtime32_s(&tmIdx, &m_gmtTime);
        //tmIdx.tm_hour = tmIdx.tm_min = tmIdx.tm_sec = 0;
        //return _mkgmtime32(&tmIdx);
        // Optimization of above code
        return m_gmtTime - (m_gmtTime % (1 * 24 * 60 * 60));
    }

    UINT32 GetUtcStartOfTradingDay();
    UINT32 GetUtcEndOfTradingDay();

    UINT32 GetUtcBeginOfWeek();
    UINT32 GetUtcEndOfWeek();

    inline UINT32 GetLocalTime() {
        return GetTzTime(TzLocal);
    }

    inline UINT32 GetNyseTime() {
        return GetTzTime(TzEastern);
    }

    inline int GetUtcYear() const { 
        struct tm   tmTemp;
        _gmtime32_s(&tmTemp, &m_gmtTime);
        return tmTemp.tm_year + 1900; 
    }

    inline int GetUtcMonth() const {
        struct tm   tmTemp;
        _gmtime32_s(&tmTemp, &m_gmtTime);
        return tmTemp.tm_mon + 1;
    }

    inline int GetUtcDay() const {
        struct tm   tmTemp;
        _gmtime32_s(&tmTemp, &m_gmtTime);
        return tmTemp.tm_mday;
    }

    inline int GetUtcDayOfWeek() const { 
        struct tm   tmTemp;
        _gmtime32_s(&tmTemp, &m_gmtTime);
        return tmTemp.tm_wday + 1;
    }

    inline int GetLocalYear() const { 
        struct tm   tmTemp;
        _localtime32_s(&tmTemp, &m_gmtTime);
        return tmTemp.tm_year + 1900; 
    }

    inline int GetLocalMonth() const {
        struct tm   tmTemp;
        _localtime32_s(&tmTemp, &m_gmtTime);
        return tmTemp.tm_mon + 1;
    }

    inline int GetLocalDay() const {
        struct tm   tmTemp;
        _localtime32_s(&tmTemp, &m_gmtTime);
        return tmTemp.tm_mday;
    }

    inline int GetLocalHour() const {
        struct tm   tmTemp;
        _localtime32_s(&tmTemp, &m_gmtTime);
        return tmTemp.tm_hour;
    }

    inline int GetLocalMinute() const {
        struct tm   tmTemp;
        _localtime32_s(&tmTemp, &m_gmtTime);
        return tmTemp.tm_min;
    }

    inline int GetLocalSecond() const {
        struct tm   tmTemp;
        _localtime32_s(&tmTemp, &m_gmtTime);
        return tmTemp.tm_sec;
    }

    inline int GetLocalDayOfWeek() const { 
        struct tm   tmTemp;
        _localtime32_s(&tmTemp, &m_gmtTime);
        return tmTemp.tm_wday + 1;
    }


    //
    // Conversion function
    //
protected:
    int FormatW(ETimeZone TimeZone, LPCWSTR pszFormat, LPWSTR TimeString, DWORD Length);
    int FormatA(ETimeZone TimeZone, LPCSTR pszFormat, LPSTR TimeString, DWORD Length);

public:
    template <size_t Length>
    int ToStringFxDate(CHAR(&DateString)[Length])
    {
        return FormatA(TzLocal, "%b %d", DateString, Length);
    }

    template <size_t Length>
    int ToStringFxTime(CHAR(&TimeString)[Length])
    {
        return FormatA(TzLocal, "%H:%M", TimeString, Length);
    }

    template <size_t Length>
    int ToStringLong(CHAR(&DateString)[Length])
    {
        return FormatA(TzEastern, "%a, %b %d, %y", DateString, Length);
    }

    template <size_t Length>
    int ToStringStd(CHAR (&DateString)[Length])
    {
        return FormatA(TzEastern, "%m/%d/%Y", DateString, Length);
    }

    //
    // Returns number of bytes written to the buffer
    //
    int ToStringLogFmt(LPSTR DateString, DWORD Length)
    {
        return FormatA(TzLocal, "[%Y/%m/%d %H:%M:%S] ", DateString, Length);
    }

    bool FromStringStd(__in LPCSTR DateString);

    bool FromStringWeb(__in LPCSTR DateString);

    //
    // Time check functions
    //
public:
    bool IsNyse930AM();
    bool IsNyse359PM();
    bool IsNyseClosed();
    bool IsNyseRegularHours(PUINT32 Index);

};

