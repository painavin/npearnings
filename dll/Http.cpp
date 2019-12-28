/*++

    Copyrights (c) Pai Financials

Module Name:

    Http.cpp

Abstract:

    This file contains the class implementation for http
    protocol

Author:

    Navin Pai (navinp)

--*/
#include "StdAfx.h"
#include "Http.h"


#pragma comment(lib, "WinInet.lib")

_Use_decl_annotations_
bool 
CHttpWinInet::InitializeW(
    LPCWSTR szUserAgent,
    LPCWSTR szServer
    )
{
    bool retVal = false;
    EnterFunc();

    _ASSERT(m_hSession == NULL);
    CHK_EXP(m_hSession != NULL);


    // Initialize the http
    m_hSession = InternetOpenW(szUserAgent, 
        INTERNET_OPEN_TYPE_PRECONFIG, 
        NULL, NULL, 0);
    CHK_EXP_ERR(m_hSession == NULL, "InternetOpenW");


    // Connect to the http server
    m_hConnection = InternetConnectW(m_hSession, szServer, 
        INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, 
        INTERNET_SERVICE_HTTP, 0, NULL);
    CHK_EXP_ERR(m_hConnection == NULL, "InternetConnectW");

    retVal = true;

Cleanup:

    LeaveFunc();
    return retVal;
}


_Use_decl_annotations_
bool
CHttpWinInet::InitializeA(
    LPCSTR szUserAgent,
    LPCSTR szServer
    )
{
    bool retVal = false;
    EnterFunc();

    _ASSERT(m_hSession == NULL);
    CHK_EXP(m_hSession != NULL) 

    // Initialize the http
    m_hSession = InternetOpenA(szUserAgent, INTERNET_OPEN_TYPE_PRECONFIG,
        NULL, NULL, 0);
    CHK_EXP_ERR(m_hSession == NULL, "InternetOpenA");


    // Connect to the http server
    m_hConnection = InternetConnectA(m_hSession, szServer, INTERNET_DEFAULT_HTTP_PORT,
        NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
    CHK_EXP_ERR(m_hConnection == NULL, "InternetConnectA");

    retVal = true;

Cleanup:

    LeaveFunc();
    return retVal;
}


_Use_decl_annotations_
bool
CHttpWinInet::RecvResponse(
    std::string& Response
    )
{
    CHAR chBuffer[4096];
    DWORD dwBytesRead = _countof(chBuffer);
    DWORD dwStatusCode = 0;

    EnterFunc();

    _ASSERT(m_hRequest != NULL);
    CHK_EXP(m_hRequest == NULL);

    Response.clear();

    // Query to make sure we succeeded
    if (HttpQueryInfoA(m_hRequest, HTTP_QUERY_STATUS_CODE, chBuffer, 
        &dwBytesRead, NULL) == FALSE)
    {
        LogErrorFn("HttpQueryInfoA");
        goto Cleanup;
    }

    dwStatusCode = (DWORD)atol(chBuffer);
    CHK_EXP(dwStatusCode != HTTP_STATUS_OK);


    // Read while there is still more data or request failed
    while (InternetReadFile(m_hRequest, (LPVOID)chBuffer, _countof(chBuffer) - 1, &dwBytesRead))
    {
        CHK_EXP(dwBytesRead == 0);
        chBuffer[dwBytesRead] = 0;
        Response.append(chBuffer);
    }

Cleanup:
    if (m_hRequest != NULL)
    {
        InternetCloseHandle(m_hRequest);
        m_hRequest = NULL;
    }

    LeaveFunc();
    return Response.length() > 0;
}


_Use_decl_annotations_
bool
CHttpWinInet::SendRequestW(
    LPCWSTR szVerb, 
    LPCWSTR szRequest, 
    LPCWSTR szReferrer, 
    LPCWSTR szHeaders, 
    DWORD dwHeaderLength, 
    LPVOID lpFormData, 
    DWORD dwFormDataLength)
{
    bool retVal = false;
    LPCWSTR acceptTypes[] = { L"*/*", NULL };

    EnterFunc();

    _ASSERT(m_hSession != NULL);
    _ASSERT(m_hConnection != NULL);
    _ASSERT(m_hRequest == NULL);

    CHK_EXP(m_hConnection == NULL);
    CHK_EXP(m_hSession == NULL);
    CHK_EXP(m_hRequest != NULL);

    
    // Create an HTTP request handle.
    m_hRequest = HttpOpenRequestW(m_hConnection, szVerb, szRequest, 
        NULL, szReferrer, acceptTypes, INTERNET_FLAG_EXISTING_CONNECT, NULL);
    CHK_EXP_ERR(m_hRequest == NULL, "HttpOpenRequestW");


    // Send the request.
    BOOL bResults = HttpSendRequestW(m_hRequest, szHeaders, 
        dwHeaderLength, lpFormData, dwFormDataLength);
    if (bResults == FALSE)
    {
        LogErrorFn("HttpSendRequestW");
        InternetCloseHandle(m_hRequest);
        m_hRequest = NULL;
        goto Cleanup;
    }

    retVal = true;

Cleanup:
    LeaveFunc();
    return retVal;
}


_Use_decl_annotations_
bool
CHttpWinInet::SendRequestA(
    LPCSTR szVerb, 
    LPCSTR szRequest, 
    LPCSTR szReferrer, 
    LPCSTR szHeaders, 
    DWORD dwHeaderLength, 
    LPVOID lpFormData, 
    DWORD dwFormDataLength)
{
    bool retVal = false;
    LPCSTR acceptTypes[] = { "*/*", NULL };

    EnterFunc();

    _ASSERT(m_hSession != NULL);
    _ASSERT(m_hConnection != NULL);
    _ASSERT(m_hRequest == NULL);

    CHK_EXP(m_hConnection == NULL);
    CHK_EXP(m_hSession == NULL);
    CHK_EXP(m_hRequest != NULL);

    
    // Create an HTTP request handle.
    m_hRequest = HttpOpenRequestA(m_hConnection, szVerb, szRequest, 
        NULL, szReferrer, acceptTypes, INTERNET_FLAG_EXISTING_CONNECT, NULL);
    CHK_EXP_ERR(m_hRequest == NULL, "HttpOpenRequestA");


    // Send the request.
    BOOL bResults = HttpSendRequestA(m_hRequest, szHeaders, dwHeaderLength,
        lpFormData, dwFormDataLength);
    if (bResults == FALSE)
    {
        LogErrorFn("HttpSendRequestA");
        InternetCloseHandle(m_hRequest);
        m_hRequest = NULL;
        goto Cleanup;
    }

    retVal = true;

Cleanup:
    LeaveFunc();
    return retVal;
}



///////////////////////////////////////////////////////////////////////////////
//
// class CHttpWinInetSecure
//
#pragma comment(lib, "WinInet.lib")


_Use_decl_annotations_
bool
CHttpWinInetSecure::InitializeA(
    LPCSTR szUserAgent,
    LPCSTR szServer
    )
{
    bool retVal = false;
    EnterFunc();

    _ASSERT(m_hSession == NULL);
    CHK_EXP(m_hSession != NULL);

    // Initialize the http
    m_hSession = InternetOpenA(szUserAgent, 
        INTERNET_OPEN_TYPE_PRECONFIG,
        NULL, NULL, 0);
    CHK_EXP_ERR(m_hSession == NULL, "InternetOpenA");


    // Connect to the http server
    m_hConnection = InternetConnectA(m_hSession, szServer, 
        INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, 
        INTERNET_SERVICE_HTTP, 0, NULL);
    CHK_EXP_ERR(m_hSession == NULL, "InternetConnectA");

    retVal = true;

Cleanup:

    LeaveFunc();
    return retVal;
}


_Use_decl_annotations_
bool
CHttpWinInetSecure::RecvResponse(
    std::string& Response
    )
{
    CHAR chBuffer[4096];
    DWORD dwBytesRead = _countof(chBuffer);
    DWORD dwStatusCode = 0;

    EnterFunc();

    _ASSERT(m_hRequest != NULL);
    CHK_EXP(m_hRequest == NULL);

    Response.clear();

    // Query to make sure we succeeded
    if (HttpQueryInfoA(m_hRequest, HTTP_QUERY_STATUS_CODE,
        chBuffer, &dwBytesRead, NULL) == FALSE)
    {
        LogErrorFn("HttpQueryInfoA");
        goto Cleanup;
    }

    dwStatusCode = (DWORD) atol(chBuffer);
    CHK_EXP(dwStatusCode != HTTP_STATUS_OK);

    // Read while there is still more data or request failed
    while (InternetReadFile(m_hRequest, (LPVOID)chBuffer, _countof(chBuffer) - 1, &dwBytesRead))
    {
        if (dwBytesRead == 0) break;
        chBuffer[dwBytesRead] = 0;
        Response.append(chBuffer);
    }

Cleanup:

    if (m_hRequest != NULL)
    {
        InternetCloseHandle(m_hRequest);
        m_hRequest = NULL;
    }

    LeaveFunc();
    return Response.length() > 0;
}


_Use_decl_annotations_
bool
CHttpWinInetSecure::SendRequestA(
    LPCSTR szVerb, 
    LPCSTR szRequest, 
    LPCSTR szReferrer, 
    LPCSTR szHeaders, 
    DWORD dwHeaderLength, 
    LPVOID lpFormData, 
    DWORD dwFormDataLength)
{
    bool retVal = false;
    LPCSTR acceptTypes[] = { "*/*", NULL };

    EnterFunc();

    _ASSERT(m_hSession != NULL);
    _ASSERT(m_hConnection != NULL);
    _ASSERT(m_hRequest == NULL);

    CHK_EXP(m_hConnection == NULL);
    CHK_EXP(m_hSession == NULL);
    CHK_EXP(m_hRequest != NULL);

    
    // Create an HTTP request handle.
    m_hRequest = HttpOpenRequestA(m_hConnection, szVerb, szRequest, 
        NULL, szReferrer, acceptTypes, INTERNET_FLAG_KEEP_CONNECTION | 
        INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_SECURE, NULL);
    CHK_EXP_ERR(m_hRequest == NULL, "HttpOpenRequestA");


    // Send the request.
    BOOL bResults = HttpSendRequestA(m_hRequest, szHeaders, dwHeaderLength,
        lpFormData, dwFormDataLength);
    if (bResults == FALSE)
    {
        LogErrorFn("HttpSendRequestA");
        InternetCloseHandle(m_hRequest);
        m_hRequest = NULL;
        goto Cleanup;
    }

    retVal = true;

Cleanup:

    LeaveFunc();
    return retVal;
}
