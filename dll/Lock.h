/*++

    Copyright (c) Pai Financials LLC. All rights reserved.

Module Name:

    Lock.h

Abstract:

    This file contains the class declaration for auto lock

Author:

    Navin Pai (navin.pai@outlook.com)

--*/
#pragma once

// Lock management
class CLock
{
private:
    CRITICAL_SECTION    m_cs;

public:
    CLock() { InitializeCriticalSection(&m_cs); }
    ~CLock() { DeleteCriticalSection(&m_cs); }

public:
    void Lock() { EnterCriticalSection(&m_cs);  }
    void Unlock() { LeaveCriticalSection(&m_cs); }
};


// Lock management
class CAutoLock
{
private:
    CLock&  m_Lock;

public:
    CAutoLock(CLock& Lock) : m_Lock(Lock) { m_Lock.Lock(); }
    ~CAutoLock() { m_Lock.Unlock(); }
};
