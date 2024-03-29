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

    Lock.h

Abstract:

    This file contains the class declaration for auto lock

Author:

    nabieasaurus

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
