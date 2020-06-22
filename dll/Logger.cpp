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

    Logger.cpp

Abstract:

    The function definitions for logging functions

Author:

    nabieasaurus

--*/

#include "stdafx.h"
#include "FeedTime.h"
#include "EarningsApi.h"

//
// Logging is disabled by default
//
static DWORD gLogLevel = 0;
static LOGGERPROC gLoggerProc = NULL;
bool gResetData = false;


_Use_decl_annotations_
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
    int         iLen = 0;
    va_list     argPtr;
    CHAR        szMsg[1024];
    CFeedTime   ftNow(FT_CURRENT);

    if (LogLevel > gLogLevel || gLoggerProc == NULL) { return; }

    // iLen = ftNow.ToStringLogFmt(szMsg, sizeof(szMsg));

    va_start(argPtr, szFormat);
    iLen = _vsnprintf_s(&szMsg[iLen], _countof(szMsg) - iLen, _TRUNCATE, szFormat, argPtr);
    va_end(argPtr);

    gLoggerProc(szMsg);
}
