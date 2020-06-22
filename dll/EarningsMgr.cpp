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

    EarningsMgr.cpp

Abstract:

    This file contains the implementation of the earnings release 
    module. It retrieves the earnings information from website
    and parses the information to be displayed in tradestation.

Author:

    nabieasaurus

--*/
#include "StdAfx.h"
#include "EarningsMgr.h"
#pragma comment(lib, "Shlwapi.lib")

//
// The user agent string and the website we are collecting data from
//
#define USER_AGENT_STRING           "UserAgent:  Mozilla/4.0 (compatible; MSIE 8.0)"
#define WWW_DATASOURCE              "www.earningswhispers.com"

//
// The earning file cache header and CSV header 
//
#define EARNINGS_DATAFILE_HDR       "Earnings Data File Ver 7.0 Copyright (c) Pai Financials LLC (Do not remove this line)\n"
#define EARNINGS_DATAFILE_ROW       "Available,Ticker,QueryDate,EarningsDate,EarningsTime,Confirmed,Notes\n"

//
// The constants required for the form submission 
//
#define WEBSITE_FORM_HEADER         "Content-Type: application/x-www-form-urlencoded"
#define WEBSITE_EARNING_URL         "stocks.asp?symbol=%s"

//
// The indexes of the csv line
//
enum EarningsIndex
{
    E_AVAILABLE         = 0,
    E_TICKER            = 1,
    E_QUERYDATE         = 2,
    E_EARNINGDATE       = 3,
    E_EARNINGTIME       = 4,
    E_EARNINGCONFIRMED  = 5,
    E_EARNINGNOTES      = 6,
    E_MAXCOLUMNS        = 7,
};



CEarningsMgr::CEarningsMgr(
    void
    )
/*++

Routine Description:

    This is the default constructor for the CEarningsMgr

--*/
{
    m_bCacheDirty = false;
    m_bConnected = false;
}


CEarningsMgr::~CEarningsMgr(
    void
    ) 
/*++

Routine Description:

    This is the default destructor for the CEarningsMgr.

--*/
{
    CAutoLock al(m_EarningsCacheLock);

    //
    // Delete all of the entries
    //
    for (EARNINGS_MAP::iterator itEarn = m_EarningsCache.begin();
        itEarn != m_EarningsCache.end();
        itEarn++)
    {
        CEarningsDataPtr_t pData = itEarn->second;
        delete pData;
    }

    //
    // Disconnect the http connection
    //
    Disconnect();
}


bool 
CEarningsMgr::Connect(
    void
    ) 
/*++

Routine Description:

    This function connects to website for sending queries

Return Value:

    true - if file load was successful
    false - if anything went wrong

--*/
{
    //
    // BUG: TODO: Hard code this dll to expire
    //
    CFeedTime   todaysDate(FT_CURRENT);
    CFeedTime   expireDate(TzUtc, 2021, 1, 1);

    EnterFunc();

    if (todaysDate > expireDate) { goto Cleanup; }

    if (m_bConnected == true) { goto Cleanup; }
    
    m_bConnected = m_EarningsSite.InitializeA(USER_AGENT_STRING, WWW_DATASOURCE);

Cleanup:

    LeaveFunc();
    return m_bConnected;
}


_Use_decl_annotations_
bool
CEarningsMgr::LoadEarningsData(
    LPCSTR FileName,
    INT EarningsQueryDays,
    INT PostEarningsDays,
    INT EarningsRandDays
    )
/*++

Routine Description:

    This function loads the earnings data from the cache file into 
    the memory for faster processing. It opens the file and reads
    the header information to verify we are reading the correct file
    and creates an archive object to pass it on for further reading.

Parameters:

    FileName - The name of the file from which to load the file.

    EarningsQueryDays - The number of days before earnings to query again

    PostEarningsDays - Number of days past earnings to query again

    EarningsRandDays - Number of days since last query query again

Return Value:

    true - if file load was successful
    false - if anything went wrong

--*/
{
    using namespace std;
    bool    bRet = false;
    int     lineCtr = 0;
    CHAR    szLine[1024];
    LPSTR   szHeaders[] = { EARNINGS_DATAFILE_HDR, EARNINGS_DATAFILE_ROW };

    EnterFunc();

    //
    // Use the lock function wide
    //
    CAutoLock al(m_EarningsCacheLock);

    LogInfo("Loading data file : %s", FileName);

    fstream inFile(FileName, ios::in);
    if (inFile.fail())
    {
        LogError("Unable to open the file : %s", FileName);
        goto Cleanup;
    }

    //
    // Read the file header and compare
    //
    for (int nCtr = 0; nCtr < _countof(szHeaders); nCtr++)
    {
        inFile.getline(szLine, _countof(szLine));
        if (inFile.fail())
        {
            LogError("Unable to read the file : %s", FileName);
            goto Cleanup;
        }

        if (strncmp(szLine, szHeaders[nCtr], strlen(szHeaders[nCtr]) - 1) != 0)
        {
            LogError("Header mismatch : %s", FileName);
            goto Cleanup;
        }

        LogTrace("Header row match: %d", nCtr);
    }

    //
    // Read the file data and Load the data into temp cache and remove
    // stale ones before moving to the cache
    //
    m_EarningsCache.clear();

    while (true)
    {
        LPSTR           szValue[E_MAXCOLUMNS];
        CFeedTime       ftQuery, ftEarnings;
        bool            bIsAvailable = true;

        //
        // Read a line and exit if there were no more lines to read
        //
        inFile.getline(szLine, _countof(szLine));
        if (inFile.fail()) { break; }
        lineCtr++;

        //
        // If the line was a blank, continue reading till we reach EOF
        //
        if (strlen(szLine) == 0) { continue; }

        //
        // Parse the line
        //
        szValue[E_AVAILABLE] = szLine;
        for (int nCtr = 1; nCtr < _countof(szValue); nCtr++)
        {
            LPSTR pNext = (LPSTR) strchr(szValue[nCtr - 1], L',');
            if (pNext == NULL) 
            {
                LogError("Unable to parse input line");
                continue;
            }

            *pNext++ = L'\0';

            //
            // Trim white-spaces around all the values that we read
            //
            StrTrimA(pNext, " \t\r\n");
            szValue[nCtr] = pNext;
        }

        //
        // Check if the line we read represents a valid entry
        //
        bIsAvailable = atoi(szValue[E_AVAILABLE]) == 0 ? false : true;

        if (ftQuery.FromStringStd(szValue[E_QUERYDATE]) == false)
        {
            LogError("Unable to parse the query date");
            continue;
        }

        if ((bIsAvailable == true) && 
            (ftEarnings.FromStringStd(szValue[E_EARNINGDATE]) == false))
        {
            LogError("Unable to parse the earnings date");
            continue;
        }

        //
        // Convert the symbol to uppercase
        //
        CHAR szSymbol[64];
        strcpy_s(szSymbol, szValue[E_TICKER]);
        _strupr_s(szSymbol);

        //
        // If all fields were successfully read then allocate an entry and add it to the cache
        //
        CEarningsDataPtr_t pData = new CEarningsData(szSymbol, bIsAvailable, 
            ftQuery.GetUtcTime(), ftEarnings.GetUtcTime(), szValue[E_EARNINGTIME], 
            atoi(szValue[E_EARNINGCONFIRMED]) == 0 ? false : true, 
            szValue[E_EARNINGNOTES]);
        if (pData == NULL)
        {
            //
            // allocation failed, continue with next iteration and see if that succeeds
            //
            LogError("Allocation failed for %s", pData->StrTicker.c_str());
            continue;
        }

        auto inserted = m_EarningsCache.insert(
            EARNINGS_MAP::value_type(pData->StrTicker, pData));
            
        _ASSERT(inserted.second == true);
        if (inserted.second == false)
        {
            //
            // insertion failed, continue with next iteration and see if that succeeds
            //
            LogError("Insertion failed for %s", pData->StrTicker.c_str());
            delete pData;
            continue;
        }

        LogTrace("Loaded earnings for %s", pData->StrTicker.c_str());

        //
        // For available data, check if we have to trigger a query again
        // to the server
        //
        pData->ReQuery = gResetData;
        if (pData->ReQuery == false)
        {
            pData->CheckForRequery(lineCtr, EarningsQueryDays,
                PostEarningsDays, EarningsRandDays);
        }
    }

    m_bCacheDirty = false;
    bRet = true;

Cleanup:

    inFile.close();
    
    LeaveFunc();
    return bRet;
}


_Use_decl_annotations_
bool
CEarningsMgr::SaveEarningsData(
    LPCSTR FileName
    )
/*++

Routine Description:

    This function saves the earnings data from the memory cache into 
    the file for later sessions. It opens the file and writes
    the header information and other earnings data.

Parameters:

    FileName - The name of the file to which the data will be saved

Return Value:

    true - if file load was successful
    false - if anything went wrong

--*/
{
    using namespace std;

    //
    // Use the lock function wide
    //
    CAutoLock al(m_EarningsCacheLock);

    //
    // If no modification then we have nothing to write
    //
    if (m_bCacheDirty == false)
    {
        LogTrace("Cache is not dirty. Nothing to write.");
        return true;
    }

    LogInfo("Saving data to file : %s", FileName);

    //
    // Open the output file in write mode
    //
    fstream    outFile(FileName, ios::out | ios::trunc);

    if (outFile.fail())
    {
        LogError("Unable to open the file : %s", FileName);
        return false;
    }

    //
    // Write the file header(s)
    //
    outFile.write(EARNINGS_DATAFILE_HDR, strlen(EARNINGS_DATAFILE_HDR));
    outFile.write(EARNINGS_DATAFILE_ROW, strlen(EARNINGS_DATAFILE_ROW));


    //
    // We write both valid and invalid entries to the file. Both valid and invalid
    // entries are queries every N days to make sure we have new data
    //
    for (EARNINGS_MAP::iterator itEarn = m_EarningsCache.begin();
        itEarn != m_EarningsCache.end(); itEarn++)
    {
        CHAR    szLine[1024];
        UINT    dwLen = itEarn->second->ToString(szLine);
        
        outFile.write(szLine, dwLen);
    }


    //
    // Close the file and reset the cache flag
    //
    outFile.flush();
    outFile.close();

    m_bCacheDirty = false;
    LogTrace("FileSaved");

    return true;
}


_Use_decl_annotations_
CEarningsDataPtr_t
CEarningsMgr::GetEarningsData(
    LPCSTR Ticker
    )
/*++

Routine Description:

    This function returns the earnings data for the Ticker symbol
    passed to the function. It first checks to see if the data is
    in the cache and if not found then queries website for
    data and puts it in the cache for future use

Parameters:

    Ticker - The ticker symbol

Return Value:

    Pointer to the earnings data if successful.
    NULL if there was error retrieving data.

--*/
{
    CHAR            strSymbol[128];
    CEarningsDataPtr_t  pData = NULL;

    //
    // Convert ticker to upper case
    //
    strcpy_s(strSymbol, Ticker);
    _strupr_s(strSymbol);
    String strTicker(strSymbol);

    //
    // Only one query active at any time
    //
    CAutoLock lock(m_EarningsCacheLock);

    //
    // Check to see if symbol is in cache
    //
    EARNINGS_MAP::iterator earnIt = m_EarningsCache.find(strTicker);
    if (earnIt == m_EarningsCache.end())
    {
        LogWarn("Symbol not in cache : %s", strSymbol);

        pData = new CEarningsData(strSymbol);
        if (pData != NULL)
        {
            auto inserted = m_EarningsCache.insert(
                EARNINGS_MAP::value_type(strTicker, pData));
            
            _ASSERT(inserted.second == true);
            if (inserted.second == false)
            {
                delete pData;
                pData = NULL;
            }
            else
            {
                QueryEarningsFromWebsite(pData);
                m_bCacheDirty = true;
            }
        }
    }
    else
    {
        //
        // Found in cache. Check if we have to requery the symbol
        //
        LogInfo("Symbol found in cache: %s", strSymbol);

        pData = earnIt->second;

        if (pData->ReQuery)
        {
            LogInfo("Symbol set for query: %s", pData->StrTicker.c_str());

            QueryEarningsFromWebsite(pData);
            pData->ReQuery = false;
            m_bCacheDirty = true;
        }
    }

    return pData;
}


_Use_decl_annotations_
bool 
CEarningsMgr::ExtractTag(
    LPCSTR StrTag, 
    String& StrInput, 
    String::size_type& InOutPos, 
    String& StrOutput
    )
/*++

Routine Description:

    This function extracts the Tag value from the html string.
    If input is <abc>.....</abc> then passing abc will get everything
    that is between the start and end of the tag.

Parameters:

    StrTag      - The tag value to extract
    StrInput    - The input string from which to extract the tag
    InOutPos    - The position of the string from which to look for tag, returns
                    the update position in the input string
    StrOutput   - The output string which contains the tag value

Return Value:

    true - if we were able to successfully parse the input string

--*/
{
    String::size_type startPos, endPos, nestedPos;
    String startTag("<");
    String endTag("</");

    startTag.append(StrTag);
    endTag.append(StrTag);

    //
    // Look for the start of the tag
    // TODO: Fix bug - look at the datasourceutils.cs
    //
    startPos = StrInput.find(startTag, InOutPos);
    if (startPos == String::npos) return false;

    //
    // While loop to take care of nested tags
    //
    endPos = startPos;
    nestedPos = startPos;
    while (true)
    {
        // Look for the end of the tag
        endPos = StrInput.find(endTag, endPos + 1);
        if (endPos == String::npos) return false; 

        // Check if there is a nested tag
        nestedPos = StrInput.find(startTag, nestedPos + 1);
        if ((nestedPos == String::npos) || (nestedPos > endPos)) break;

        // We have a nested entry. find the endPos again for this entry
    }

    
    InOutPos = endPos + endTag.length() + 1;
    StrOutput = StrInput.substr(startPos, InOutPos - startPos);

    return true;
}



_Use_decl_annotations_
bool
CEarningsMgr::ExtractAttributeValue(
    LPCSTR StrAttrib,
    String& StrInput,
    String& StrValue
    )
/*++
Routine Description:

    This function extracts the value from the string. If input is 
    <input type="ATRIB" ... value="asfsadfsafd"/> then the output 
    will be "asdfasfasdf"

Parameters:

    StrAttrib   - The attribute to look for in the file
    StrInput    - The input string from which to extract the tag
    StrValue    - The output string which contains the value

Return Value:

    true - if we were able to successfully parse the input string

--*/
{
    String::size_type startPos, endPos;

    // look for the attribute in the string
    startPos = StrInput.find(StrAttrib);
    if (startPos == String::npos) return false;

    // Get the value for this attribute
    startPos = StrInput.find("value", startPos);
    if (startPos == String::npos) return false;

    // Extract everything between quotes
    startPos = StrInput.find("\"", startPos);
    if (startPos == String::npos) return false;

    startPos++;
    endPos = StrInput.find("\"", startPos);
    if (endPos == String::npos) return false;

    StrValue = StrInput.substr(startPos, endPos - startPos);
    return true;
}


_Use_decl_annotations_
bool
CEarningsMgr::ExtractAttribute(
    LPCSTR StrAttrib,
    String& StrInput,
    String& StrOutput
    )
{
    String::size_type startPos, endPos;

    // Look for the start of the attrib
    startPos = StrInput.find(StrAttrib);
    if (startPos == String::npos) return false;

    // Extract everything between quotes
    startPos = StrInput.find("\"", startPos);
    if (startPos == String::npos) return false;

    startPos++;
    endPos = StrInput.find("\"", startPos);
    if (endPos == String::npos) return false;

    StrOutput = StrInput.substr(startPos, endPos - startPos);
    return true;
}


_Use_decl_annotations_
bool
CEarningsMgr::ExtractValue(
    String& StrInput, 
    String& StrOutput
    )
/*++
Routine Description:

    This function extracts the value from the tag string. If input is 
    <abc>Test<abc> then the output will be "Test"

Parameters:

    StrInput    - The input string from which to extract the tag
    StrOutput   - The output string which contains the value

Return Value:

    true - if we were able to successfully parse the input string

--*/
{
    String::size_type startPos, endPos;

    // Look for the start of the tag
    startPos = StrInput.find(">");
    if (startPos == String::npos) return false;

    // Look for the end of the table
    endPos = StrInput.find("<", startPos);
    if (endPos == String::npos) return false; 

    StrOutput = StrInput.substr(startPos + 1, endPos - startPos - 1);
    
    return true;
}


_Use_decl_annotations_
void
CEarningsMgr::ConvertToHex(
    String& Str
    )
{
    CHAR strBuffer[4*1024];
    PCHAR pStr = (PCHAR)Str.c_str();
    int nCtr = 0;

    for (int ctr = 0; pStr[ctr]; ctr++)
    {
        if (strchr("+@/=", pStr[ctr]) == NULL)
        {
            strBuffer[nCtr++] = pStr[ctr];
        }
        else
        {
            strBuffer[nCtr++] = '%';
            sprintf_s(strBuffer + nCtr, _countof(strBuffer) - nCtr - 1, 
                "%02X", pStr[ctr]);
            nCtr += 2;
        }
    }

    strBuffer[nCtr] = 0;
    Str = strBuffer;
}


_Use_decl_annotations_
bool
CEarningsMgr::ParseHtmlForEarningsDate(
    String& HtmlPage,
    CEarningsDataPtr_t PtrEarningsData
    )
/*++

Routine Description:

    This function returns the next earnings date of the ticker

Parameters:

    HtmlPage - The source for the html page

    PtrEarningsData - Fills up earnings information

Return Value:

    true - if everything was parsed correctly
    false - if there was error parsing data

--*/
{
    String szTemp;
    CFeedTime ftTemp;
    bool retVal = false;

    EnterFunc();

    //
    // Look for "datebox" in html page
    //
    String::size_type tableLoc = HtmlPage.find("id=\"datebox\"");
    if (tableLoc == String::npos)
    {
        LogTrace("datebox tag not found.");
        PtrEarningsData->IsAvailable = false;
        goto Cleanup;
    }

    //
    // Extract the day
    //
    CHK_RET(ExtractTag("div", HtmlPage, tableLoc, szTemp));

    //
    // Get the confirmation and parse
    //
    CHK_RET(ExtractTag("div", HtmlPage, tableLoc, szTemp));
    
    if (szTemp.find("color-yes") != String::npos)
    {
        PtrEarningsData->IsConfirmed = true;
    }

    //
    // Get the date and parse
    //
    CHK_RET(ExtractTag("div", HtmlPage, tableLoc, szTemp));
    CHK_RET(ExtractValue(szTemp, szTemp));
    CHK_RET(ftTemp.FromStringWeb(szTemp.c_str()));
    PtrEarningsData->SetEarningsDate(ftTemp);

    //
    // Get the time
    //
    CHK_RET(ExtractTag("div", HtmlPage, tableLoc, szTemp));
    CHK_RET(ExtractValue(szTemp, PtrEarningsData->StrEarningsTime));

    retVal = true;

Cleanup:

    LeaveFunc();
    return retVal;
}


_Use_decl_annotations_
void 
CEarningsMgr::QueryEarningsFromWebsite(
    CEarningsDataPtr_t PtrEarningsData
    )
/*++

Routine Description:

    This function does all the work of querying the data from 
    website and parses the data to save it for later use.

Parameters:

    Ticker - The ticker symbol

Return Value:

    Pointer to the earnings data if successful.
    NULL if there was error retrieving data.

--*/
{
    String      httpString;
    CHAR        chBuffer[1024];

    EnterFunc();

    //
    // If we are not connected, just return
    //
    CHK_EXP(m_bConnected == false);

    LogInfo("Query from website: %s", PtrEarningsData->StrTicker.c_str());

    //
    // Create the search query for the ticker and send the request
    //
    sprintf_s(chBuffer, WEBSITE_EARNING_URL, PtrEarningsData->StrTicker.c_str());

    LogInfo("Query URL = http://" WWW_DATASOURCE "/%s", chBuffer);

    if (m_EarningsSite.SendGetRequestA(chBuffer) == false)
    {
        LogError("Unable to send GET request");
        goto Cleanup;
    }

    //
    // Receive the response for our request
    //
    if (m_EarningsSite.RecvResponse(httpString) == false)
    {
        LogError("Failed to receive response");
        goto Cleanup;
    }

    //
    // Parse the http response and look for earnings date and if the parsing 
    // was successful then query for the estimates page
    //
    PtrEarningsData->SetQueryDate(CFeedTime(FT_CURRENT));
    PtrEarningsData->IsAvailable = ParseHtmlForEarningsDate(httpString, PtrEarningsData);
    
Cleanup:

    LeaveFunc();
    return;
}

