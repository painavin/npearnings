/*++

    Copyright (c) Pai Financials LLC. All rights reserved.

Module Name:

    dllmain.cpp

Abstract:

    Defines the entry point for the DLL Application

Author:

    Navin Pai (navin.pai@outlook.com)

--*/
#include "stdafx.h"
#include "EarningsMain.h"

BOOL
APIENTRY 
DllMain(
    HMODULE hModule,
    DWORD  dwReason,
    LPVOID lpReserved
    )
/*++

Abstract:

    The main entrypoint into the dll. This function reads the registry
    settings and initializes the EarningsRelease class for use by
    TradeStation

--*/
{
    UNREFERENCED_PARAMETER(lpReserved);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls(hModule);
        }
        break;

    case DLL_PROCESS_DETACH:
        {
            gEarningsMain.Uninitialize();
        }
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}
