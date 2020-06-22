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

    npearnings.cpp

Abstract:

    This application tests the earnings dll and exposes some
    utility for testing

Author:

    nabieasaurus

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


void 
TestStocks()
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


int 
main(/*int argc, char *argv[]*/)
{
    TestStocks();
}
