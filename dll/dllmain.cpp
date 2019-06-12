/*++

    Copyright (c) Pai Financials LLC

Module Name:

    dllmain.cpp

Abstract:

    Defines the entry point for the DLL Application

Author:

    Navin Pai (navinp) - 06-Oct-2015

--*/

#include "pch.h"

BOOL 
APIENTRY 
DllMain( 
    HMODULE ModuleHandle,
    DWORD  ReasonForCall,
    LPVOID Reserved
    )
/*++

Abstract:

    The main entrypoint into the dll. This function reads the registry
    settings and initializes the EarningsRelease class for use by
    TradeStation

--*/
{
    UNREFERENCED_PARAMETER(Reserved);

    switch (ReasonForCall)
    {
    case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls(ModuleHandle);
        }
        break;

    case DLL_PROCESS_DETACH:
        {
        }
        break;

    case DLL_THREAD_ATTACH: __fallthrough;
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}

