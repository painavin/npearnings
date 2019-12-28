/*++

Copyright (c) 2015 Pai Financials LLC

Module Name:

    Logger.h

Abstract:

    The function declarations for logging functions

Author:

    Navin Pai (navinp) - 06-Oct-2015

--*/
#pragma once


//
// Logging functions and variables
//
void LogPrintf(_In_ DWORD LogLevel, _In_ LPCSTR szFormat, ...);


//
// Logging levels
//
#define NP_LOG_ERROR            1
#define NP_LOG_WARNING          2
#define NP_LOG_INFO             3
#define NP_LOG_TRACE            4

//
// Logging macros
//
#define LogTrace(_msg, ...)     LogPrintf(NP_LOG_TRACE,   "[" __FUNCTION__ "]:" _msg "\n", __VA_ARGS__)
#define LogInfo(_msg, ...)      LogPrintf(NP_LOG_INFO,    "[" __FUNCTION__ "]:" _msg "\n", __VA_ARGS__)
#define LogWarn(_msg, ...)      LogPrintf(NP_LOG_WARNING, "[" __FUNCTION__ "]:" _msg "\n", __VA_ARGS__)
#define LogError(_msg, ...)     LogPrintf(NP_LOG_ERROR,   "[" __FUNCTION__ "]:" _msg "\n", __VA_ARGS__)
#define LogErrorFn(_fun)        LogPrintf(NP_LOG_ERROR,   "[" __FUNCTION__ "]:" _fun " Error. Code = %d\n", GetLastError())

#define EnterFunc()          \
do {                        \
    LogTrace("===>Enter");  \
}while(0)


#define LeaveFunc()         \
do {                        \
    LogTrace("<===Exit");   \
}while(0)
