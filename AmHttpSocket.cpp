/*
    File name: AmHttpSocket.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
*/

#include "StdAfx.h"
#include "AmHttpSocket.h"
#include <vector>
#include <string>
#include <algorithm>

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: AmHttpSocket.cpp,v $$Revision: 1.3 $$Date: 2008/09/28 17:16:48Z $"; 
#endif

const char * const CAmHttpSocket::FormBoundary = "---------------------------TMS_Launcher";

CAmHttpSocket::CAmHttpSocket()
{
    hInternet = NULL;
    hRequest = NULL;
    hConnection = NULL;
    hInternet = InternetOpen(_T("Mozilla/4.0 (compatible; MSIE 9.0)"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

    // set new options if necessary
    DWORD dwSize = sizeof(DWORD);
    DWORD Timeout = 0;
    InternetQueryOption(hInternet,INTERNET_OPTION_CONNECT_TIMEOUT,&Timeout,&dwSize);
    if (Timeout < 300000) // timeout is less than 5 minutes
    {
        Timeout = 300000;
        InternetSetOption(hInternet,INTERNET_OPTION_CONNECT_TIMEOUT,&Timeout,dwSize);
    }
    Timeout = 0;
    InternetQueryOption(hInternet,INTERNET_OPTION_RECEIVE_TIMEOUT,&Timeout,&dwSize);
    if (Timeout < 300000) // timeout is less than 5 minutes
    {
        Timeout = 300000;
        InternetSetOption(hInternet,INTERNET_OPTION_RECEIVE_TIMEOUT,&Timeout,dwSize);
    }
}

CAmHttpSocket::~CAmHttpSocket()
{
    if (hInternet != NULL) InternetCloseHandle(hInternet);
    if (hRequest != NULL) InternetCloseHandle(hRequest);
    if (hConnection != NULL) InternetCloseHandle(hConnection);
}

void CAmHttpSocket::DownloadHtmlPage(const char *url, CString &HTML, const char* Login, const char* Password)
{
    if (PerformRequest(url, HTML) != 200)
    {
        HTML.Empty();
        return;
    }

    // check if this is iTMS login page
    if (HTML.Find("/itms/login/login.php?r=") == -1) return;

    // download iTMS page
    CString UserName = Login;
    CString DomainName;
    // Login can be in formats: UserName@DomainName or DomainName\UserName or UserName
    int pos = -1;
    if ((pos = UserName.Find('\\')) != -1 && pos < (UserName.GetLength()-1))
    {
        DomainName = UserName.Left(pos);
        UserName = UserName.Mid(pos + 1);
    }
    if ((pos = UserName.Find('@')) != -1 && pos < (UserName.GetLength()-1))
    {
        DomainName = UserName.Mid(pos + 1);
        UserName = UserName.Left(pos);
    }
    CString FormData = GetPostFormData(DomainName, UserName, Password);
    CString FormHeaders = GetPostFormHeaders(FormData);
    if (PerformRequest("https://www.softcomputer.com/itms/login/login.php", HTML, FormHeaders, FormData) != 200)
    {
        HTML.Empty();
        return;
    }

    if (PerformRequest(url, HTML) != 200 || HTML.Find("/itms/login/login.php?r=") != -1)
    {
        HTML.Empty();
        return;
    }
}

int CAmHttpSocket::PerformRequest(const char *url, CString &HTML, const char *PostHeaders, const char *PostData)
{
    // close previous connection if it was not closed
    if (hRequest != NULL) InternetCloseHandle(hRequest);
    if (hConnection != NULL) InternetCloseHandle(hConnection);

    CString protocol, login, password, host, request;
    INTERNET_PORT iPort = 80;
    ParseURL(url, protocol, login, password, host, iPort, request);

    DWORD flags = 0;
    if (protocol == "http")
    {
        //we are talking plain http
        flags = INTERNET_FLAG_NO_CACHE_WRITE;
    }
    else
    {
        //we are talking secure https
        flags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
    }

    // connect to the server
    hConnection = InternetConnect(hInternet, host, iPort, NULL, NULL, INTERNET_SERVICE_HTTP, INTERNET_FLAG_NO_CACHE_WRITE, 0);
    LPCTSTR AcceptTypes[] = {_T("*/*"), NULL};

    // prepare request to be sent to the server
    CString Method = (PostHeaders == NULL) ? _T("GET") : _T("POST");
    hRequest = HttpOpenRequest(hConnection, Method, request, NULL, NULL, AcceptTypes, flags, 0);

    // send the request to the server and get the response
    if (!HttpSendRequest(hRequest, PostHeaders, lstrlen(PostHeaders), (LPVOID)PostData, lstrlen(PostData)))
    {
        HTML.Empty();
        return 0;
    }
    // response is received, read HTTP headers and returned data (usually HTML page)
    int RetCode = atoi(ReadHeaderLine(HTTP_QUERY_STATUS_CODE));
    ReadData(HTML);

    // process cookies received from the server
    ProcessCookies(url);

    // close connection
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnection);

    return RetCode;
}

void CAmHttpSocket::ProcessCookies(const char *url)
{
    // set cookies received from the server to be used in the next session
    CString AllCookies = ReadHeaderLine(HTTP_QUERY_SET_COOKIE);
    if (!AllCookies.IsEmpty())
    {
        AllCookies.Replace("\r\n", "; ");
        InternetSetCookie(url, NULL, AllCookies);
    }
}

CString CAmHttpSocket::GetPostFormData(const char *DomainName, const char *UserName, const char *Password)
{
    CString FormData = CString("--") + CAmHttpSocket::FormBoundary;
    FormData += "\r\n";
    FormData += "Content-Disposition: form-data; name=\"login\"";
    FormData += "\r\n\r\n";
    FormData += UserName;
    FormData += "\r\n";

    FormData += CString("--") + CAmHttpSocket::FormBoundary;
    FormData += "\r\n";
    FormData += "Content-Disposition: form-data; name=\"provider\"";
    FormData += "\r\n\r\n";
    FormData += DomainName;
    FormData += "\r\n";

    FormData += CString("--") + CAmHttpSocket::FormBoundary;
    FormData += "\r\n";
    FormData += "Content-Disposition: form-data; name=\"password\"";
    FormData += "\r\n\r\n";
    FormData += Password;
    FormData += "\r\n";

    FormData += CString("--") + CAmHttpSocket::FormBoundary;
    FormData += "--\r\n";

    return FormData;
}

CString CAmHttpSocket::GetPostFormHeaders(const CString &FormData)
{
    CString FormHeader = "Content-Type: multipart/form-data; boundary=";
    FormHeader += CAmHttpSocket::FormBoundary;
    FormHeader += "\r\n";

    CString FormDataSize;
    FormDataSize.Format("%d", FormData.GetLength());
    FormHeader += "Content-Length: " + FormDataSize+ "\r\n";

    return FormHeader;
}

CString CAmHttpSocket::ReadHeaderLine(DWORD QueryInfoFlag)
{
    CString HeaderLine;
    CString AllHeaderLines;
    DWORD dwIndex = 0;
    DWORD LastError = 0;

    while (LastError != ERROR_HTTP_HEADER_NOT_FOUND)
    {
        DWORD dwSizeOfHeaderLine = 512;
        if (HttpQueryInfo(hRequest, QueryInfoFlag, HeaderLine.GetBufferSetLength(dwSizeOfHeaderLine), &dwSizeOfHeaderLine, &dwIndex))
        {
            HeaderLine.ReleaseBuffer();
            AllHeaderLines += (AllHeaderLines.IsEmpty() ? HeaderLine : "\r\n" + HeaderLine);
            if (dwIndex == 0) dwIndex++;
        }
        else
        {
            LastError = GetLastError();
            HeaderLine.ReleaseBuffer();
            if (LastError == ERROR_INSUFFICIENT_BUFFER)
            {
                if (HttpQueryInfo(hRequest, QueryInfoFlag, HeaderLine.GetBufferSetLength(dwSizeOfHeaderLine), &dwSizeOfHeaderLine, &dwIndex))
                {
                    HeaderLine.ReleaseBuffer();
                    AllHeaderLines += (AllHeaderLines.IsEmpty() ? HeaderLine : "\r\n" + HeaderLine);
                    if (dwIndex == 0) dwIndex++;
                }
                else
                {
                    LastError = GetLastError();
                    HeaderLine.ReleaseBuffer();
                }
            }
        }
    }
    return AllHeaderLines;
}

void CAmHttpSocket::ReadData(CString &HTML)
{
    HTML.Empty();
    std::string buffer;
    DWORD dwNumberOfBytesAvailable = 0;
    while (InternetQueryDataAvailable(hRequest, &dwNumberOfBytesAvailable, 0, 0))
    {
        if (dwNumberOfBytesAvailable == 0) return;
        buffer.resize(dwNumberOfBytesAvailable + 1);
        DWORD dwNumberOfBytesRead = 0;
        if (!InternetReadFile(hRequest, &buffer[0], dwNumberOfBytesAvailable, &dwNumberOfBytesRead) || dwNumberOfBytesRead == 0)
        {
            HTML.Empty();
            return;
        }
        std::string portion(buffer.c_str(), dwNumberOfBytesRead);
        std::replace(portion.begin(), portion.end(), '\0', ' ');
        HTML += portion.c_str();
        dwNumberOfBytesAvailable = 0;
    }
}

//  Used to break apart an URL such as
//  http://login:password@www.localhost.com:80/TestPost.htm
//  into protocol, login, password, host, port and request
void CAmHttpSocket::ParseURL(const char *url, CString &protocol, CString &login, CString &password, CString &host, INTERNET_PORT &port, CString &request)
{
    URL_COMPONENTS uc = {0};
    uc.dwStructSize = sizeof(uc);
    uc.dwSchemeLength = 1;
    uc.dwHostNameLength = 1;
    uc.dwUserNameLength = 1;
    uc.dwPasswordLength = 1;
    uc.dwUrlPathLength = 1;
    uc.dwExtraInfoLength = 1;

    CString URL = url;
    if (!InternetCrackUrl(URL, URL.GetLength(), 0, &uc))
    {
        return;
    }

    protocol = CString(uc.lpszScheme, uc.dwSchemeLength);
    login = CString(uc.lpszUserName, uc.dwUserNameLength);
    password = CString(uc.lpszPassword, uc.dwPasswordLength);
    host = CString(uc.lpszHostName, uc.dwHostNameLength);
    port= uc.nPort;
    request = CString(uc.lpszUrlPath, uc.dwUrlPathLength);
    request += CString(uc.lpszExtraInfo, uc.dwExtraInfoLength);
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
