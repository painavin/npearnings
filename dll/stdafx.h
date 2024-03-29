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

    StdAfx.h

Abstract:

    Precompiled headers
    include file for standard system include files, or project specific 
    include files that are used frequently, but are changed infrequently

Author:

    nabieasaurus

--*/
#pragma once

// Including SDKDDKVer.h defines the highest available Windows platform.
// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <Shlwapi.h>
#include <WinInet.h>

#include <time.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdarg.h>

#include <map>
#include <deque>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>


typedef std::wstring WString;
typedef std::string String;


#define __STR2__(x)     #x
#define __STR1__(x)     __STR2__(x)
#define __LOC__         __FILE__ "("__STR1__(__LINE__)") : NOTE : "

//
// If enabled, we provide Forex functionality as well
//
#define NPFOREX_DISABLED

//
// If enabled, we monitor the csv file for runtime changes
//
#define MONITOR_CSV_

#ifndef NPFOREX
#pragma message(__LOC__ "* * * * * * * * * FOREX DISABLED * * * * * * * *.")
#endif

#ifndef MONITOR_CSV
#pragma message(__LOC__ "* * * * * * * * * MONITORING DISABLED * * * * * * * *.")
#endif


#define CHK_RET(_expr_)         \
    {                           \
        retVal = (_expr_);      \
        if (retVal == false)    \
        {                       \
            goto Cleanup;       \
        }                       \
    }


#define CHK_EXP(_expr_)         \
    {                           \
        bool _r = (_expr_);     \
        if (_r)                 \
        {                       \
            goto Cleanup;       \
        }                       \
    }

#define CHK_EXP_ERR(_e, _f)                                     \
    {                                                           \
        bool _r = (_e);                                         \
        if (_r)                                                 \
        {                                                       \
            LogError(_f " error. Code = %d", GetLastError());   \
            goto Cleanup;                                       \
        }                                                       \
    }


//
// Add local files
//
#include "EarningsApi.h"
#include "Logger.h"

