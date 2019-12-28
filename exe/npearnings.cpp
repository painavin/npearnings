/*++

    Copyright (c) Pai Financials LLC. All rights reserved.

Module Name:

    npearnings.cpp

Abstract:

    This application tests the earnings dll and exposes some
    utility for testing

Author:

    Navin Pai (navin.pai@outlook.com)

--*/
#include <Windows.h>
#include <stdio.h>

#include "..\\dll\\EarningsApi.h"
#pragma comment(lib, "npearnings.lib")

VOID
CALLBACK
LoggerProc(
    _In_ LPCSTR Message
    )
{
    printf(Message);
}

void TestStocks()
{
    LPCSTR symList[] =
    {
        "MSFT",
        "AAPL",
        "RPAI",
        "RLYP",
        "RMAX",
        "RMBS",
        "RMTI",
        "AMD",
    };

    for (int i = 0; i < _countof(symList); i++)
    {
        printf(
            "%s: Earnings Date              = %s\n"
            "%s: Earnings Time              = %s\n"
            "%s: Earnings Days to Release   = %s\n"
            "%s: Earnings Confirmed         = %d\n\n\n\n",
            symList[i], GetEarningsReleaseDate(symList[i]),
            symList[i], GetEarningsReleaseTime(symList[i]),
            symList[i], GetDaysToEarningsRelease(symList[i]),
            symList[i], GetEarningsConfirmation(symList[i]));
    }
}

int main(/*int argc, char *argv[]*/)
{
    TestStocks();
}
