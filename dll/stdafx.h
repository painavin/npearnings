/*++

    Copyright (c) Pai Financials LLC. All rights reserved.

Module Name:

    StdAfx.h

Abstract:

    Precompiled headers
    include file for standard system include files, or project specific 
    include files that are used frequently, but are changed infrequently

Author:

    Navin Pai (navin.pai@outlook.com)

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

