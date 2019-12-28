/*++

    Copyright (c) Pai Financials LLC. All rights reserved.

Module Name:

    HttpHelper.h

Abstract:

    This file contains the class declaration for http
    protocol

Author:

    Navin Pai (navin.pai@outlook.com)

--*/
#pragma once

class CHttpWinInet
{
protected:
    HINTERNET   m_hSession;
    HINTERNET   m_hConnection;
    HINTERNET   m_hRequest;

public:
    CHttpWinInet(void) {
        m_hSession = m_hConnection = m_hRequest = NULL;
    }

    ~CHttpWinInet(void) {
        Uninitialize();
    }

public:
    //
    // un-initialize the variables
    //
    void Uninitialize(void) {

        if (m_hRequest != NULL) InternetCloseHandle(m_hRequest);
        if (m_hConnection != NULL) InternetCloseHandle(m_hConnection);
        if (m_hSession != NULL) InternetCloseHandle(m_hSession);

        m_hSession = m_hConnection = m_hRequest = NULL;
    }

    //
    // Initialize in unicode
    //
    bool InitializeW(_In_ LPCWSTR szUserAgent, _In_ LPCWSTR szServer);

    //
    // Initialize in ascii
    //
    bool InitializeA(_In_ LPCSTR szUserAgent, _In_ LPCSTR szServer);


public:
    //
    // Receive response for the request sent
    //
    bool RecvResponse(_Inout_ std::string& String);

    //
    // Send a request to the server in unicode
    //
    bool SendRequestW(_In_ LPCWSTR szVerb, _In_ LPCWSTR szRequest,
        _In_ LPCWSTR szReferrer, _In_ LPCWSTR szHeaders, 
        _In_ DWORD dwHeaderLength, _In_ LPVOID lpFormData, 
        _In_ DWORD dwFormDataLength);

    //
    // Send a GET request to the server in unicode
    //
    bool SendGetRequestW(_In_ LPCWSTR szRequest) 
    {
        return SendRequestW(L"GET", szRequest, NULL, NULL, 0, NULL, 0);
    }

    //
    // Send a POST request to the server in unicode
    //
    bool SendPostRequestW(_In_ LPCWSTR szRequest, _In_ LPCWSTR szReferrer, 
        _In_ LPCWSTR szHeaders, _In_ DWORD dwHeaderLength,
        _In_ LPVOID lpFormData, _In_ DWORD dwFormDataLength) 
    {
        return SendRequestW(L"POST", szRequest, szReferrer, 
            szHeaders, dwHeaderLength, lpFormData, dwFormDataLength);
    }
    
    //
    // Send a request to the server in ascii
    //
    bool SendRequestA(_In_ LPCSTR szVerb, _In_ LPCSTR szRequest,
        _In_ LPCSTR szReferrer, _In_ LPCSTR szHeaders, 
        _In_ DWORD dwHeaderLength, _In_ LPVOID lpFormData, _In_ DWORD dwFormDataLength);

    //
    // Send a GET request in ascii
    //
    bool SendGetRequestA(_In_ LPCSTR szRequest) 
    {
        return SendRequestA("GET", szRequest, NULL, 
            NULL, 0, NULL, 0);
    }

    //
    // Send a POST request in ascii
    //
    bool SendPostRequestA(_In_ LPCSTR szRequest, _In_ LPCSTR szReferrer, 
        _In_ LPCSTR szHeaders, _In_ DWORD dwHeaderLength, 
        _In_ LPVOID lpFormData, _In_ DWORD dwFormDataLength) 
    {
        return SendRequestA("POST", szRequest, szReferrer, 
            szHeaders, dwHeaderLength, lpFormData, dwFormDataLength);
    }
};



class CHttpWinInetSecure
{
protected:
    HINTERNET   m_hSession;
    HINTERNET   m_hConnection;
    HINTERNET   m_hRequest;

public:
    CHttpWinInetSecure(void) {
        m_hSession = m_hConnection = m_hRequest = NULL;
    }

    ~CHttpWinInetSecure(void) {
        Uninitialize();
    }

public:
    //
    // un-initialize the variables
    //
    void Uninitialize(void) {

        if (m_hRequest != NULL) InternetCloseHandle(m_hRequest);
        if (m_hConnection != NULL) InternetCloseHandle(m_hConnection);
        if (m_hSession != NULL) InternetCloseHandle(m_hSession);

        m_hSession = m_hConnection = m_hRequest = NULL;
    }

    //
    // Initialize in ascii
    //
    bool InitializeA(_In_ LPCSTR szUserAgent, _In_ LPCSTR szServer);

public:

    //
    // Receive response for the request sent
    //
    bool RecvResponse(_Inout_ std::string& String);

    //
    // Send a request to the server in unicode
    //
    bool SendRequestA(_In_ LPCSTR szVerb, _In_ LPCSTR szRequest,
        _In_ LPCSTR szReferrer, _In_ LPCSTR szHeaders, 
        _In_ DWORD dwHeaderLength, _In_ LPVOID lpFormData, 
        _In_ DWORD dwFormDataLength);

    //
    // Send a GET request to the server in ascii
    //
    bool SendGetRequestA(_In_ LPCSTR szRequest) {
        return SendRequestA("GET", szRequest, NULL, NULL, 0, NULL, 0);
    }

    //
    // Send a POST request to the server in ascii
    //
    bool SendPostRequestA(_In_ LPCSTR szRequest, _In_ LPCSTR szReferrer,
        _In_ LPCSTR szHeaders, _In_ DWORD dwHeaderLength,
        _In_ LPVOID lpFormData, _In_ DWORD dwFormDataLength) 
    {
        return SendRequestA("POST", szRequest, szReferrer, 
            szHeaders, dwHeaderLength,
            lpFormData, dwFormDataLength);
    }

};

typedef CHttpWinInet        CHttp;
typedef CHttpWinInetSecure  CHttpSecure;
