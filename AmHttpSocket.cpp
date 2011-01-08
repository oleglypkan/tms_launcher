/*
    File name: CmdLine.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: September 5, 2006
*/

#include "StdAfx.h"
#include "AmHttpSocket.h"
#include <atlbase.h>
#include <limits.h>
#include <vector>

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: AmHttpSocket.cpp,v $$Revision: 1.3 $$Date: 2008/09/28 17:16:48Z $"; 
#endif

#define AgentName _T("Mozilla/4.0")

//case insensitive search functions...
#ifdef UNICODE
#define _tcsustr wcsustr
#else
#define _tcsustr strustr
#endif
char* strustr(char *source, char *s);
wchar_t* wcsustr(wchar_t *source, wchar_t *s);

char* strustr(char *source, char *s)
{
    //make an uppercase copy of source and s
    char *csource = _strdup(source);
    char *cs = _strdup(s);
    _strupr(csource);
    _strupr(cs);
    //find cs in csource...
    char *result = strstr(csource, cs);
    if (result != NULL)
    {
        //cs is somewhere in csource
        int pos = result - csource;
        result = source;
        result += pos;
    }
    //clean up
    free(csource);
    free(cs);
    return result;
}

wchar_t* wcsustr(wchar_t *source, wchar_t *s)
{
    //make an uppercase copy af source and s
    wchar_t *csource = _wcsdup(source);
    wchar_t *cs = _wcsdup(s);
    _wcsupr(csource);
    _wcsupr(cs);
    //find cs in csource...
    wchar_t *result = wcsstr(csource, cs);
    if (result != NULL)
    {
        //cs is somewhere in csource
        int pos = result - csource;
        result = source;
        result += pos;
    }
    //clean up
    free(csource);
    free(cs);
    return result;
}

// find 'ch' character in 'buffer' and returns its index
DWORD strnpos(const char *buffer, const char ch, DWORD size)
{
    for (DWORD i=0; i<size-1; i++)
    {
        if (buffer[i] == ch)
        {
            return i;
        }
    }
    return -1;
}

CAmHttpSocket::CAmHttpSocket()
{
#ifdef DEBUG
    OutFile = NULL; // only for DEBUG
#endif
    LastError = 0;
    ReceivedData = NULL;
    Headers = NULL;
    hIO = NULL;
    hIS = NULL;
    hCO = NULL;
    hIO = InternetOpen(AgentName, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

    // set new options if necessary
    DWORD dwSize = sizeof(DWORD);
    DWORD Timeout = 0;
    InternetQueryOption(hIO,INTERNET_OPTION_CONNECT_TIMEOUT,&Timeout,&dwSize);
    if (Timeout < 300000) // timeout is less than 5 minutes
    {
        Timeout = 300000;
        InternetSetOption(hIO,INTERNET_OPTION_CONNECT_TIMEOUT,&Timeout,dwSize);
    }
    Timeout = 0;
    InternetQueryOption(hIO,INTERNET_OPTION_RECEIVE_TIMEOUT,&Timeout,&dwSize);
    if (Timeout < 300000) // timeout is less than 5 minutes
    {
        Timeout = 300000;
        InternetSetOption(hIO,INTERNET_OPTION_RECEIVE_TIMEOUT,&Timeout,dwSize);
    }
}

CAmHttpSocket::~CAmHttpSocket()
{
    if (ReceivedData != NULL) free(ReceivedData);
    if (Headers != NULL) free(Headers);
    if (hIO != NULL) InternetCloseHandle(hIO);
    if (hIS != NULL) InternetCloseHandle(hIS);
    if (hCO != NULL) InternetCloseHandle(hCO);
}

bool CAmHttpSocket::OpenUrl(const TCHAR *url)
{
    if (hIS != NULL)
    {
        InternetCloseHandle(hIS);
        hIS = NULL;
    }

    CString protocol, host, request, port, login, password;
    ParseURL(url,protocol,login,password,host,request,port);
    CString headers = "";
    headers = GetHeaderLine("WWW-Authenticate");
    if (!headers.IsEmpty() && !login.IsEmpty() && !password.IsEmpty())
    {
        CString AuthString = login + ":" + password;
        Base64Encode(AuthString);
        headers.Replace("WWW-Authenticate","Authorization");
        headers.Insert(headers.Find("realm"),AuthString+" ");
        headers += "\r\n";
    }
    else
    {
        headers = "";
    }

#ifdef DEBUG
if (OutFile != NULL) *OutFile << "    InternetOpenUrl started...";
#endif

    hIS = InternetOpenUrl(hIO, protocol+"://"+host+":"+port+request, headers, -1L, INTERNET_FLAG_RAW_DATA, 0);

    if (hIS != NULL)
    {
#ifdef DEBUG
if (OutFile != NULL) *OutFile << "finished successfully" << endl;
#endif
        return true;
    }
    else
    {
        LastError = GetLastError();
#ifdef DEBUG
if (OutFile != NULL) *OutFile << "finished with error - " << LastError << endl;
#endif
        return false;
    }
}

bool CAmHttpSocket::PostUrl(const TCHAR *url, const char *PostData, int PostDataLength)
{
    //check length of postdata
    if (PostDataLength == -1)
        PostDataLength = strlen(PostData);
    //some variable that we need...
    URL_COMPONENTS uc;
    //let's split the url...
    uc.dwStructSize = sizeof(uc);
    uc.lpszScheme = NULL;
    uc.dwSchemeLength = 0;
    uc.lpszHostName = NULL;
    uc.dwHostNameLength = 1;
    uc.nPort = 0;
    uc.lpszUserName = NULL;
    uc.dwUserNameLength = 0;
    uc.lpszPassword = NULL;
    uc.dwPasswordLength = 0;
    uc.lpszUrlPath = NULL;
    uc.dwUrlPathLength = 1;
    uc.lpszExtraInfo = NULL;
    uc.dwExtraInfoLength = 0;
    InternetCrackUrl(url, _tcslen(url), 0, &uc);
    //post the data...
    if (hCO != NULL) InternetCloseHandle(hCO);
    TCHAR *HostName = _tcsdup(uc.lpszHostName);
    HostName[uc.dwHostNameLength] = '\0';
    TCHAR *FileName = _tcsdup(uc.lpszUrlPath);
    FileName[uc.dwUrlPathLength] = '\0';
    if (hIS != NULL) InternetCloseHandle(hIS); //if open, close the handle to the connection
    DWORD flags;
    if (uc.nPort == 80)
    {
        //we are talking plain http
        flags = INTERNET_FLAG_NO_CACHE_WRITE;
    }
    else
    {
        //we are talking secure https
        flags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE |
            INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
    }
    TCHAR headers[] = _T("Content-Type: application/x-www-form-urlencoded"); //content type for post...
    TCHAR szAccept[] = _T("*/*"); //we accept everything...
    LPTSTR AcceptTypes[2]={0};
    AcceptTypes[0]=szAccept;
    hCO = InternetConnect(hIO, HostName, uc.nPort, _T(""), _T(""), INTERNET_SERVICE_HTTP, INTERNET_FLAG_NO_CACHE_WRITE, 0);
    hIS = HttpOpenRequest(hCO, _T("POST"), FileName, NULL, NULL, (LPCTSTR*)AcceptTypes, flags, 0);
    if (!HttpSendRequest(hIS, headers, _tcslen(headers), (TCHAR*)PostData, PostDataLength))
    {
        LastError = GetLastError();
        free(HostName);
        free(FileName);
        return false;
    }
    free(HostName);
    free(FileName);
    return true;
}

TCHAR* CAmHttpSocket::GetHeaders(const TCHAR *url)
{
    //did we get an url?
    if (url == NULL)
    {
        LastError = -1;
        return NULL;
    }
    //open the url...
    if (!OpenUrl(url)) return NULL;
    //delete old headers...
    if (Headers != NULL) free(Headers);
    Headers = (TCHAR*)calloc(1, sizeof(TCHAR));
    //get the size headers
    DWORD d = 1, d2 = 0;
    int i = HttpQueryInfo(hIS, HTTP_QUERY_RAW_HEADERS, Headers, &d, &d2);
    //alloc some space for the headers
    Headers = (TCHAR*)realloc(Headers, d * sizeof(TCHAR));
    if (!HttpQueryInfo(hIS, HTTP_QUERY_RAW_HEADERS, Headers, &d, &d2)) return NULL;
    return Headers;
}

char* CAmHttpSocket::GetPage(const TCHAR *url, bool Post, const char *PostData, int PostDataLength)
{
    // did we get an url?
    if (url == NULL)
    {
        LastError = -1;
        return NULL;
    }
    // get the page and store it in ReceivedData...
    if (Post)
    {
        // use http post...
        if (!PostUrl(url, PostData, PostDataLength)) return NULL;
    }
    else
    {
        // use http get
        //if (!OpenUrl(url)) return NULL; // OpenUrl() function was already called in GetHeaders() and no need to call it again
    }
    // initializing variables
    char *mr = NULL;    
    DWORD rd = 0;
    DWORD curpos = 0;
    DWORD dwNumberOfBytesAvailable = 0;
    if (ReceivedData != NULL)
    {
        free(ReceivedData); ReceivedData = NULL; 
    }

#ifdef DEBUG
if (OutFile != NULL) *OutFile << "    InternetQueryDataAvailable started" << endl;
#endif

    // get amount of available data
    if (!InternetQueryDataAvailable(hIS, &dwNumberOfBytesAvailable, 0, 0))
    {
        LastError = GetLastError();
        return NULL;
    }
    if (dwNumberOfBytesAvailable != 0)
    {
        mr = new char[dwNumberOfBytesAvailable];
    }
    else
    {
        return NULL;
    }
    // receiving data...
#ifdef DEBUG
if (OutFile != NULL) *OutFile << "    InternetReadFile started in loop";
#endif

    while (InternetReadFile(hIS, mr, dwNumberOfBytesAvailable, &rd))
    {

#ifdef DEBUG
if (OutFile != NULL)
{
    *OutFile << ".";
    OutFile->flush();
}
#endif

        if (rd == 0) break;
        ReceivedData = (char*)realloc(ReceivedData, curpos + rd + 1);
        memcpy(ReceivedData+curpos, mr, rd);
        curpos += rd;
        if (mr != NULL) 
        {
            delete [] mr; 
            mr = NULL;
        }
        dwNumberOfBytesAvailable = 0;
        if (!InternetQueryDataAvailable(hIS, &dwNumberOfBytesAvailable, 0, 0))
        {
            LastError = GetLastError();
            return NULL;
        }
        if (dwNumberOfBytesAvailable != 0)
        {
            mr = new char[dwNumberOfBytesAvailable];
        }
        else
        {
            break;
        }

    }
#ifdef DEBUG
if (OutFile != NULL) *OutFile << endl << "    InternetReadFile finished" << endl;
#endif

    if (mr != NULL)
    {
        delete [] mr;
    }
    // replacing '\0' character in received data with ' ' character
    if (ReceivedData != NULL)
    {
        ReceivedData[curpos] = '\0';
        DWORD pos = -1;
        while ((pos = strnpos(ReceivedData,'\0',curpos)) != -1)
        {
            ReceivedData[pos] = ' ';
        }
    }
    return ReceivedData;
}

TCHAR* CAmHttpSocket::GetHeaderLine(TCHAR *s)
{
    //find a line in the headers that contains s, and return a pointer to the line...
    if (Headers == NULL) return NULL;
    TCHAR *ts = Headers;
    if (_tcsustr(ts, s) != NULL) return ts;
    while (1)
    {
        if (*ts == '\0' && ts[1] == '\0') break;
        if (*ts == '\0')
        {
            ts++;
            if (_tcsustr(ts, s) != NULL) return ts;
        }
        else ts++;
    }
    return NULL;
}

int CAmHttpSocket::GetPageStatusCode()
{
    //get the correct header line
    TCHAR *s = GetHeaderLine(_T("http"));
    if (s == NULL) return 0; //no headers
    //find the 3 digit code...
    if (_tcslen(s) < 3) return 0; //some error,  the string is too short...
    while (!(isdigit(s[0]) && isdigit(s[1]) && isdigit(s[2])))
    {
        if (s[3] == '\0') return 0; //we have reached the end of the string, without finding the number...
        s++;
    }
    //make a copy of s, and return the code
    TCHAR *code = _tcsdup(s);
    code[3] = '\0'; //remove all text after the 3 digit response code
    int result = _ttoi(code);
    free(code);
    return result;
}

//  Used to break apart an URL such as 
//  http://login:password@www.localhost.com:80/TestPost.htm 
//  into protocol, login, password, port, host and request
void CAmHttpSocket::ParseURL(const char *url, CString &protocol, CString &login, CString &password,
                             CString &host, CString &request, CString &port)
{
    CString URL = url;

    // find protocol
    int pos = URL.Find("://");
    if (pos != -1)
    {
        protocol = URL.Left(pos);
        protocol.MakeUpper();
        URL.Delete(0,pos+3);
    }
    else
    {
        protocol = "HTTP";
    }
    // find login and password
    pos = URL.Find("@");
    if (pos != -1)
    {
        password = URL.Left(pos);
        URL.Delete(0,pos+1);
        pos = password.Find(":");
        if (pos != -1)
        {
            login = password.Left(pos);
            password.Delete(0,pos+1);
        }
        else
        {
            login = "";
            password = "";
        }
    }
    else
    {
        login = "";
        password = "";
    }
    // find port, host, request
    port = "80";
    pos = URL.Find(":");
    if (pos != -1)
    {
        host = URL.Left(pos);
        URL.Delete(0,pos+1);
        pos = URL.Find("/");
        if (pos != -1)
        {
            port = URL.Left(pos);
            URL.Delete(0,pos);
            request = URL;
        }
        else
        {
            port = URL;
            request = "";
        }
    }
    else
    {
        pos = URL.Find("/");
        if (pos != -1)
        {
            host = URL.Left(pos);
            URL.Delete(0,pos);
            request = URL;
        }
        else
        {
            host = URL;
            request = "";
        }
    }
}

bool CAmHttpSocket::FindLoginPassword(const char * url)
{
    CString URL = url;

    // find protocol
    int pos = URL.Find("://");
    if (pos != -1)
    {
        URL.Delete(0,pos+3);
    }
    // find login and password
    pos = URL.Find("@");
    if (pos != -1)
    {
        URL = URL.Left(pos);
        pos = URL.Find(":");
        if (pos != -1)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

void CAmHttpSocket::InsertLoginPassword(CString &url, const char *login, const char *password)
{
    if (FindLoginPassword(url)) return;
    
    // find protocol
    int pos = url.Find("://");
    if (pos == -1)
    {
        pos = 0;
    }
    else
    {
        pos += 3;
    }
    url.Insert(pos,login+CString(":")+password+CString("@"));
}

void CAmHttpSocket::Base64Encode(CString &string)
{
    const char Base64Table[64] = {'A','B','C','D','E','F','G','H',
                                  'I','J','K','L','M','N','O','P',
                                  'Q','R','S','T','U','V','W','X',
                                  'Y','Z','a','b','c','d','e','f',
                                  'g','h','i','j','k','l','m','n',
                                  'o','p','q','r','s','t','u','v',
                                  'w','x','y','z','0','1','2','3',
                                  '4','5','6','7','8','9','+','/'};
    CString Binary = "";
    std::vector<int> Base64Digits;

    for (int i = 0; i < string.GetLength(); i++)
    {
        unsigned char ch = (unsigned char)string[i];
        // decimal to binary
        CString temp = "";
        while (ch > 0)
        {
            char c[2] = "";
            temp.Insert(0,_itoa(ch % 2,c,10));
            ch = ch / 2;
        }
        while (temp.GetLength() < 8)
        {
            temp.Insert(0,"0");
        }
        Binary += temp;
    }

    int PaddingChars = 0;
    switch (string.GetLength() % 3)
    {
        case 1:
            Binary += "0000000000000000";
            PaddingChars = 2;
            break;
        case 2:
            Binary += "00000000";
            PaddingChars = 1;
            break;
    }
    int count = Binary.GetLength() / 6 - PaddingChars;
    for (int i = 0; i < count; i++)
    {
        CString temp = "";
        temp = Binary.Left(6);
        Binary.Delete(0,6);
        // binary to decimal
        int Result = 0;
        int pow = 1;
        for (int j = 0; j < 6; j++)
        {
            if (j != 0) pow *= 2;
            char c[2];
            c[0] = temp[5-j];
            c[1] = '\0';
            Result += atoi(c) * pow;
        }
        Base64Digits.push_back(Result);
    }
    string = "";
    for (unsigned int j = 0; j < Base64Digits.size(); j++)
    {
        string += Base64Table[Base64Digits[j]];
    }
    for (int i = 0; i < PaddingChars; i++)
    {
        string += '=';
    }
}

bool CAmHttpSocket::Base64Decode(CString &string)
{
    const char Base64Table[64] = {'A','B','C','D','E','F','G','H',
                                  'I','J','K','L','M','N','O','P',
                                  'Q','R','S','T','U','V','W','X',
                                  'Y','Z','a','b','c','d','e','f',
                                  'g','h','i','j','k','l','m','n',
                                  'o','p','q','r','s','t','u','v',
                                  'w','x','y','z','0','1','2','3',
                                  '4','5','6','7','8','9','+','/'};
    CString Binary = "";
    std::vector<int> Base64Digits;
    int PaddingChars = 0;

    for (int i = 0; i < string.GetLength(); i++)
    {
        if (string[i] != '=')
        {
            int x = 64;
            for (int j = 0; j < 64; j++)
            {
                if (Base64Table[j] == string[i])
                {
                    x = j;
                    break;
                }
            }
            if (x == 64) return false;
            Base64Digits.push_back(x);
        }
        else
        {
            PaddingChars++;
        }
    }
    for (unsigned int h = 0; h < Base64Digits.size(); h++)
    {
        unsigned char ch = (unsigned char)Base64Digits[h];
        // decimal to binary
        CString temp = "";
        while (ch > 0)
        {
            char c[2] = "";
            temp.Insert(0,_itoa(ch % 2,c,10));
            ch = ch / 2;
        }
        while (temp.GetLength() < 6)
        {
            temp.Insert(0,"0");
        }
        Binary += temp;
    }
    int count = Binary.GetLength() / 8;
    string = "";
    for (int i = 0; i < count; i++)
    {
        CString temp = "";
        temp = Binary.Left(8);
        Binary.Delete(0,8);
        // binary to decimal
        int Result = 0;
        int pow = 1;
        for (int j = 0; j < 8; j++)
        {
            if (j != 0) pow *= 2;
            char c[2];
            c[0] = temp[7-j];
            c[1] = '\0';
            Result += atoi(c) * pow;
        }
        string += (unsigned char)Result;
    }
    return true;
}
