/*++

    Copyright (c) Pai Financials LLC

Module Name:

    Tracer.cpp

Abstract:

    The function definitions for tracing 

Author:

    Navin Pai (navin.pai@outlook.com)
    Initial Revision - 06-Oct-2015

--*/
#include "pch.h"
#include "EarningsApi.h"
#include "Tracer.h"
#include "DateTime.h"

//
// Logging is disabled by default
//
static DWORD gLogLevel = 0;
static LOGGERPROC gLoggerProc = NULL;
bool gResetData = false;


VOID
WINAPI
InitLogger(
    DWORD LogLevel, 
    LOGGERPROC LoggerProc,
    bool ResetData
    )
/*++

Abstract:

    Initialize the logger level and logger callback

--*/
{
    gLogLevel = LogLevel;
    gLoggerProc = LoggerProc;
    gResetData = ResetData;
}


_Use_decl_annotations_
void 
LogPrintf(
    DWORD LogLevel, 
    LPCSTR szFormat,
    ...)
/*++

Abstract:

    The logging function checks to see if the logging is enabled
    and level is set correctly before printing the logging 
    information to the file

--*/
{
    int             iLen = 0;
    va_list         argPtr;
    CHAR            szMsg[1024];
    DateTimeOffset  ftNow(FT_CURRENT_UTC);

    if (LogLevel > gLogLevel || gLoggerProc == NULL) { return; }

    // iLen = ftNow.ToStringLogFmt(szMsg, sizeof(szMsg));

    va_start(argPtr, szFormat);
    iLen = _vsnprintf_s(&szMsg[iLen], _countof(szMsg) - iLen, _TRUNCATE, szFormat, argPtr);
    va_end(argPtr);

    gLoggerProc(szMsg);
}
