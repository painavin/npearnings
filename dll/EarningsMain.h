/*++

    Copyright (c) Pai Financials LLC. All rights reserved.

Module Name:

    EarningsMain.h

Abstract:

    The main header file for the earnings dll

Author:

    Navin Pai (navin.pai@outlook.com)

--*/
#pragma once
#include "EarningsMgr.h"
#include "ForexMgr.h"



//
// Abstract
//
//      This class is the main class that is called from the
//      dll entry/exit point. The class implements the functions
//      to read and write the settings from registry/ini file.
//
class CEarningsMain
{
public:
    CEarningsMgr    m_EarningsRelease;
    CForexMgr       m_ForexEvents;


protected:
    bool    m_bInitialized;                 // If this is initialized already
    String  m_sEarningsFile;                // This string stores the name of earnings csv file
    String  m_sIniFile;                     // This string stores the name of ini file.

    int     m_nPostEarningsDays = 0;        // days past earnings
    int     m_nEarningsQueryDays = 0;       // days pre earnings 
    int     m_nEarningsRandDays = 0;        // randomly distribute


protected:
    String ReadString(LPCSTR KeyName, LPCSTR Default);
    DWORD ReadDWord(LPCSTR KeyName, DWORD Default);


#ifdef MONITOR_CSV
    int     m_nCacheMonitorMinutes;         // interval to monitor the file
    int     m_nCacheSaveMinutes;            // interval to save the csv file
    HANDLE  m_hThreadExitEvent;             // Handle to the monitor thread
    
    static DWORD WINAPI WorkerThreadProc(LPVOID This)
    {
        CEarningsDll *pApp = (CEarningsDll*)This;
		return pApp->LoadSaveFile();
	}

	DWORD LoadSaveFile();
#endif

public:
    CEarningsMain()
    {
#ifdef MONITOR_CSV
        m_hThreadExitEvent = NULL;
#endif
        m_bInitialized = false;

        //
        // Set the file names
        //
        m_sEarningsFile.assign("NpEarnings.csv");
        m_sIniFile.assign("NpEarnings.ini");
    }

    bool Initialize(HMODULE hModule);
    bool Uninitialize(void);
};


//
// The global earnings dll instance
//
extern CEarningsMain gEarningsMain;


