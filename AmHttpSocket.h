/*
    File name: CmdLine.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: September 5, 2006
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

#ifdef DEBUG
#include <fstream> // to be removed
using namespace std;
#endif

/*
    custom errorcodes:
    -1: bad url...
*/

class CAmHttpSocket
{
public:
  #ifdef DEBUG
    ofstream *OutFile; // only for DEBUG
  #endif
    int GetPageStatusCode(); //get the HTTP statuscode for the last received page
    TCHAR* GetHeaders(const TCHAR *url); //return a pointer to the headers from an url
    CAmHttpSocket();
    ~CAmHttpSocket();
    char* GetPage(const TCHAR *url, bool Post = false, const char *PostData = NULL, int PostDataLength = -1); //get a page, if post is false, HTTP GET is used othervise HTTP POST is used. if PostDataLength is -1 the data must be NULL terminated...
    static bool FindLoginPassword(const char * url);
    static void InsertLoginPassword(CString &url, const char *login, const char *password);
    static void Base64Encode(CString &string);
    static bool Base64Decode(CString &string);
protected:
    void ParseURL(const char *url, CString &protocol, CString &login, CString &password,
                  CString &host, CString &request, CString &port);
    bool PostUrl(const TCHAR *url, const char *PostData, int PostDataLength = -1); //open a page using http post
    TCHAR* GetHeaderLine(TCHAR *s); //get a specific line from the headers
    bool OpenUrl(const TCHAR *url); //open a page using http get
    HINTERNET hIO, hIS, hCO;
    char *ReceivedData; //the internal databuffer
    TCHAR *Headers; //the internal headerbuffer
    int LastError; //internal statuscode...
};

#endif