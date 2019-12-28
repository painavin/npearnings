/*++

Copyright (c) 2015 Pai Financials LLC

Module Name:

    StdAfx.h

Abstract:

    Precompiled headers
    include file for standard system include files, or project specific 
    include files that are used frequently, but are changed infrequently

Author:

    Navin Pai (navinp) - 06-Oct-2015

--*/
#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <time.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdarg.h>

#include <windows.h>
#include <Shlwapi.h>

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
#include "Logger.h"

