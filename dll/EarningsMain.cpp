/*++

    Copyright (c) Pai Financials LLC. All rights reserved.

Module Name:

    EarningsMain.cpp

Abstract:

    Defines the entry point for the DLL Application

Author:

    Navin Pai (navin.pai@outlook.com)

--*/
#include "stdafx.h"
#include "EarningsMain.h"

//
// The global singleton for the dll class object
//
CEarningsMain gEarningsMain;


inline 
bool 
operator == (
    FILETIME Ft1, 
    FILETIME Ft2
    ) 
/*++

Description:

    Inline function to compare two FILETIME structure

--*/
{
    return (Ft1.dwHighDateTime == Ft2.dwHighDateTime) &&
        (Ft1.dwLowDateTime == Ft2.dwLowDateTime);
}


String 
CEarningsMain::ReadString(
    LPCSTR KeyName, 
    LPCSTR Default
    )
/*++

Abstract:

    Reads string from the ini file

--*/
{
    CHAR    buffer[256];
    DWORD   dwRet;
    String  retStr(Default);

    //
    // Read the string from ini file
    //
    dwRet = GetPrivateProfileStringA("NpEarnings", KeyName, Default, buffer,
                                     _countof(buffer), m_sIniFile.c_str());
    if (dwRet == 0)
    {
        WritePrivateProfileStringA("NpEarnings", KeyName, Default, m_sIniFile.c_str());
    }
    else
    {
        retStr.assign(buffer);
    }

    return retStr;
}


DWORD 
CEarningsMain::ReadDWord(
    LPCSTR KeyName, 
    DWORD Default
    )
/*++

Abstract:

    Reads DWORD from the Ini file

--*/
{
    //
    // Read the dword from ini file
    //
    return GetPrivateProfileIntA(
        "NpEarnings", 
        KeyName, 
        Default, 
        m_sIniFile.c_str());
}


bool
CEarningsMain::Initialize(
    HMODULE hModule
    )
/*++

Abstract:

    This function is called by the framework when the dll is being 
    loaded in the memory. This function loads the options from
    the ini file and the earnings data from earnings file

--*/
{
    HANDLE  hThread = NULL;
    CHAR    szDllPath[MAX_PATH];

    EnterFunc();

    if (m_bInitialized == true) { goto Cleanup; }

    //
    // Get the default location for the csv file
    //
    if (GetModuleFileNameA(hModule, szDllPath, _countof(szDllPath)) != 0)
    {
        //
        // Remove the extension from path
        //
        PCHAR pExt = strrchr(szDllPath, '.');
        if (pExt != NULL) { *pExt = 0; }

        //
        // Create the name of the files
        //
        m_sEarningsFile.assign(szDllPath) += ".csv";
        m_sIniFile.assign(szDllPath) += ".ini";
    }

    //
    // Read the settings from the registry or use the default settings
    //
    m_nEarningsQueryDays = (int)gEarningsMain.ReadDWord("EarningsQueryDays", 5);
    m_nEarningsRandDays = (int)gEarningsMain.ReadDWord("EarningsRandDays", 5);
    m_nPostEarningsDays = (int)gEarningsMain.ReadDWord("PostEarningsDays", 3);

    //
    // Load the earnings file
    //
    if (!m_EarningsRelease.LoadEarningsData(m_sEarningsFile.c_str(), m_nEarningsQueryDays,
        m_nPostEarningsDays, m_nEarningsRandDays))
    {
        LogError("Unable to load earnings Data file");
    }
 
#ifdef MONITOR_CSV
    //
    // Read the cache monitor settings
    //
    m_nCacheMonitorMinutes = (int)gEarningsDll.ReadDWord("CacheMonitorMinutes", 1);
    m_nCacheSaveMinutes = (int)gEarningsDll.ReadDWord("CacheSaveMinutes", 15);

    //
    // Create thread that periodically writes the earnings file to disk
    //
    if ((m_hThreadExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
    {
        LogDebugA("Unable to create exit event\n");
        goto Cleanup;
    }

    if ((hThread = CreateThread(NULL, 0, CEarningsDll::WorkerThreadProc, this, 0, NULL)) == NULL)
    {
        LogDebugA("Unable to create monitor thread\n");
        CloseHandle(m_hThreadExitEvent);
        m_hThreadExitEvent = NULL;
        goto Cleanup;
    }

    CloseHandle(hThread);
#else
    UNREFERENCED_PARAMETER(hThread);
#endif

    //
    // Connect to the websites for queries
    //
    m_bInitialized = m_EarningsRelease.Connect();

#ifdef NPFOREX
#pragma message(__LOC__ "* * * * * * * * * FOREX ENABLED * * * * * * * *.")
    m_bInitialized = m_bInitialized && m_ForexEvents.Connect();
#endif

Cleanup:

    LeaveFunc();
    return m_bInitialized;
}


#ifdef MONITOR_CSV
DWORD 
CEarningsMain::LoadSaveFile(void)
/*++

Description:
    
    This function is called from worker thread that monitors the
    csv file for any changes and loads the earnings notes and
    dates from the file to be displayed in tradestation. It also
    periodically save the notes to the csv file.

--*/

{
    WIN32_FILE_ATTRIBUTE_DATA   lastFileAttribs;
    int                         minCtr = 0;

    LogDebugA("Entered file monitoring thread\n");

    if (!GetFileAttributesExA(m_sEarningsFile.c_str(), GetFileExInfoStandard, 
        &lastFileAttribs))
    {
        // Failed querying the attribs. something is wrong cannot proceed
        LogDebugA("Failed to query the attributes for file : %s\n", m_sEarningsFile.c_str());
        return 1;
    }

    //
    // Monitor every minute If the underlying file has changed, and load the file if changed
    //
    while (WaitForSingleObject(m_hThreadExitEvent, 
        m_nCacheMonitorMinutes * 60 * 1000) == WAIT_TIMEOUT)
    {
        WIN32_FILE_ATTRIBUTE_DATA curFileAttribs;

        minCtr++;

        LogDebugA("Loop ctr = %d\n", minCtr);

        if (!GetFileAttributesExA(m_sEarningsFile.c_str(), GetFileExInfoStandard, 
            &curFileAttribs))
        {
            LogDebugA("Failed to query the attributes for file : %s\n", 
                      m_sEarningsFile.c_str());
            continue;
        }

        if (lastFileAttribs.ftLastWriteTime == curFileAttribs.ftLastWriteTime)
        {
            //
            // There were not changes to the file from outside. Check if we have to
            // save the file. The file is saved every X minute and the lastFileAtrrib
            // is updated to reflect the current changes to the file
            //
            if (minCtr >= m_nCacheSaveMinutes)
            {
                LogDebugA("Saving the cache file : %s\n", m_sEarningsFile.c_str());
                m_EarningsRelease.SaveEarningsData(m_sEarningsFile.c_str());
                GetFileAttributesExA(m_sEarningsFile.c_str(), GetFileExInfoStandard, 
                                     &lastFileAttribs);
                minCtr = 0;
            }
        }
        else
        {
            //
            // Load the file from the disk and update the lastFileAttribs
            //
            LogDebugA("Loading the cache file : %s\n", m_sEarningsFile.c_str());
            m_EarningsRelease.LoadEarningsData(m_sEarningsFile.c_str(), m_nDaysPreEarnings,
                m_nDaysPastEarnings);

            lastFileAttribs = curFileAttribs;
        }
    }

    LogDebugA("Exited file monitoring thread\n");

    return 0;
}
#endif


bool
CEarningsMain::Uninitialize(
    void
    )
/*++

Abstract:

    This function is called by the framework when the dll is getting
    unloaded from memory.

--*/
{
    bool bRet = true;

    EnterFunc();

    if (m_bInitialized == false) { goto Cleanup; }

#ifdef MONITOR_CSV
    if (m_hThreadExitEvent != NULL)
    {
        LogDebugA("Waiting for thread to exit\n");

        // Signal the thread to terminate and wait until it terminates
        SetEvent(m_hThreadExitEvent);

        LogDebugA("Dont really know if thread ended\n");

        CloseHandle(m_hThreadExitEvent);
    }
#endif

    //
    // Save the earnings data back to the file
    //
    m_EarningsRelease.SaveEarningsData(m_sEarningsFile.c_str());

    // Disconnect from the internet
#ifdef NPFOREX
#pragma message(__LOC__ "* * * * * * * * * FOREX ENABLED * * * * * * * *.")
    m_ForexEvents.Disconnect();
#endif

    bRet = m_EarningsRelease.Disconnect();

Cleanup:

    LeaveFunc();
    return bRet;
}
