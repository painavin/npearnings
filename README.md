# Disclaimer
 
The DLL is provided as is and I or my company Pai Financials LLC accepts no liability for trading decisions that you make by using this DLL, or for the consequences of any actions taken on basis of the information provided by this DLL.

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

## How to use the DLL
 
1. Download the attachment from the Tradestation website, unzip the files to temporary location.
2. Copy NpEarnings.dll to "C:\Program Files (x86)\TradeStation 9.1\Program" OR "C:\Program Files\TradeStation 9.1\Program" depending on your OS is 32bit or 64 bit
3. Import the ELD into TradeStation. The indicator npearnings should be available to insert in the RadarScreen.
4. If you are seeing error, it means you have not copied the dll to the right location.
5. The CSV file which contains all the data downloaded from earnings whispers is located in the same directory as the dll. The CSV file location in the registry is incorrect.

## Update History

### Jul-1-2015

* Fixed the parsing code of earnings whispers
* The CSV file is now created in the same directory as the dll
* Added INI file to control the query

### Jan-19-2015

* Uploading again because of the server issue.
* Bug fix for parsing code of earnings whispers

### Jul-10-2014

* Bug fix for earningswhispers.com

### Jan-30-2014

* Using earningswhispers.com as the data source instead of briefing. Reason for this change was that too many users were seeing errors with briefing website
* It does not require username and password anymore
* Simplified installing of the indicator

### Aug-06-2013

* Briefing changed the login page again. Updated the dll and looking for new sources to get the earnings data from.I will be switching form briefing to a different website to get the earnings data.
* Removed the old deprecated function from the dll.

### Oct-25-2012

* Fixed the issue with Briefing.com after changes to their website. This will be my last update to the indicator. If I have to change the indicator again, I will have to charge a small fee to maintain the indicator.

### Apr-01-2012

* Fixed the bug that prevented login to briefing.com

### Oct-24-2011

* Fixed the release days calculation bug.
 
### Oct-14-2011

* Updated DLL to work with new briefing.com website.
* Added another API (DoBriefingLogin) to log into website to retrieve the information. The DLL now required you to have a briefing account before you can use the dll.

### Apr-14-2011

* Some minor bug fixes.
* DLL now auto saves the file every 15 minutes and loads the file every minute. Refresh the RadarScreen window to see the changes you made to the cache file.

### Apr-08-2011

* The earnings cache file is now saved on the disk as a text file in comma seperated value (CSV) format. This means you can open the NpEarnings.csv in Excel to view the file and make changes. Do not make changes to first two lines in CSV file.
* Removed dependency on MFC libraries. This makes the DLL lot faster and significantly smaller than all previous versions.
* Added three new functions to the DLL. It gives additional data about whether the earnings date is confirmed as per briefings.com. Also the new function allows you to add notes to every symbol and the notes are saved in the CSV cache file. This is far superior than the built-in Custom Notes indicator that Tradestation provides. Please scroll down for function documentation.
* To enter notes for symbol, simply double click in the notes column and input text that you want to show up in the Inputs tab. The notes will show up wherever you have the same symbol in the RadarScreen and will persist even if you restart Tradestation.
* The functions now does not return "Not Available," when earnings information is not found on briefings.com, instead it returns just empty string. I realize that this will break some of the existing indicators that use NpEarnings.dll but this change was necessary.
* Fixed the issue where the DLL was not able to parse the page because first line did not contain earnings information.

### Oct-12-2010

* Fixed a major bug that was crashing the RadarScreen. The crash happened because some of the tickers did not have a link associated with them that pointed to the earnings information.

### May-12-2010

* Added function to get the earnings forecast and earnings year ago for the same quarter. Earnings release are now prefixed with 0 if the release days are in single digit so that sorting via release date is easy in Tradestation.
 
### Feb-02-2010

* Updated the code to display the correct number of days left and updated the script to query only for stock symbols and ignore all the other symbols (eg. Options, future, etc).

## Function Documentation

### GetEarningsReleaseDate
 
Returns the date on which the earnings will be release for the symbol passed to the function.

```
LPCSTR 
WINAPI 
GetEarningsReleaseDate(
    LPCSTR Symbol
    );
```

#### Parameters

Symbol – The symbol for which the earnings date should be returned.

#### Return Value

The return value is a string that specifies the earning date for the symbol.

### GetEarningsReleaseTime
 
Returns the time when the earnings will be released for the symbol passed to the function.

```
LPCSTR 
WINAPI 
GetEarningsReleaseTime(
    LPCSTR Symbol
    );
```

#### Parameters

Symbol – The symbol for which the earnings release time should be returned.

#### Return Value

The return value is a string that specifies the earning time for the symbol. It could be before market, during market, after market or unknown.

### GetDaysToEarningsRelease
 
Returns the number of days from today when the earnings will be released.

```
LPCSTR 
WINAPI 
GetDaysToEarningsRelease(
    LPCSTR Symbol
    );
```

#### Parameters

Symbol – The symbol for which the days should be returned.

#### Return Value

The return value is a string that specifies the number of days from today when the earnings will be released.

### GetEarningsConfirmation
 
Returns boolean value indicating if the earnings date and time are confirmed.

```
INT 
WINAPI 
GetEarningsConfirmation(
    LPCSTR Symbol
    );
```

#### Parameters

Symbol – The symbol for which the earnings date should be returned.

#### Return Value

The return 1 if the earning date is confirmed; otherwise 0.

### GetEarningsNotes
 
Returns the text value saved in the earnings csv file.

```
LPCSTR 
WINAPI 
GetEarningsNotes(
    LPCSTR Symbol
    );
```

#### Parameters

Symbol – The symbol for which the saved notes should be returned.

#### Return Value
 
The return value is a string that contains the notes from the csv file.
 

### SetEarningsNotes
 
Sets the text note for the symbol in the csv file.

```
VOID 
WINAPI 
SetEarningsNotes(
    LPCSTR Symbol,
    LPCSTR Notes
    );
```

#### Parameters

Symbol – The symbol for which the notes should be saved in the file.

Notes – The text note to save in the csv file.

#### Return Value

This function has not return value.
