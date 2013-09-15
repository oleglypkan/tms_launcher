/*
    File name: AmHttpSocket.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
*/

//link with wininet.lib

#ifndef HTTPSOCKET_H_INCLUDED
#define HTTPSOCKET_H_INCLUDED

#ifdef INCLUDE_VERID
 static char AmHttpSocket_h[]="@(#)$RCSfile: AmHttpSocket.h,v $$Revision: 1.3 $$Date: 2008/09/28 17:16:34Z $";
#endif

#include <tchar.h>
#include <windows.h>
#include <wininet.h>

class CAmHttpSocket
{
public:
    CAmHttpSocket();
    ~CAmHttpSocket();
    void DownloadHtmlPage(const char *url, CString &HTML, const char* Login = "", const char* Password = "");
    static bool FindLoginPassword(const char * url);
    static void InsertLoginPassword(CString &url, const char *login, const char *password);
    static void Base64Encode(CString &string);
    static bool Base64Decode(CString &string);
protected:
    HINTERNET hInternet, hRequest, hConnection;
    static const char * const FormBoundary;
    int PerformRequest(const char *url, CString &HTML, const char *PostHeaders = NULL, const char *PostData = NULL);
    CString ReadHeaderLine(DWORD QueryInfoFlag); // read specific header line from response
    void ReadData(CString &HTML);
    CString GetPostFormHeaders(const CString &FormData);
    CString GetPostFormData(const char *DomainName, const char *UserName, const char *Password);
    void ParseURL(const char *url, CString &protocol, CString &login, CString &password, CString &host, INTERNET_PORT &port, CString &request);
    void ProcessCookies(const char *url);
};

#endif