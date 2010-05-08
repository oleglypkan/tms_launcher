/*
    File name: Request.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: January 17, 2006
*/

#ifndef REQUEST_H_INCLUDED
#define REQUEST_H_INCLUDED

// HTTPRequest: class that returns the HTTP headers and message from the request
class HTTPRequest
{
public:
    HTTPRequest()
    {
        headerSend = "";
        headerReceive ="";
        message = "";
    }
    CString headerSend;
    CString headerReceive;
    CString message;
};

class REQUEST  
{
public:
    REQUEST();
    virtual ~REQUEST();
private:
    DWORD       GetHostAddress(LPCSTR host);
    void        SendString(SOCKET sock,LPCSTR str);
    BOOL        ValidHostChar(char ch);
    void        ParseURL(const char *url, CString &protocol, CString &login, CString &password,
                         CString &host, CString &request, int &port);
    int         SendHTTP(LPCSTR url,LPCSTR headers,BYTE *post, DWORD postLength,HTTPRequest *req);
public:
    bool        SendRequest(bool IsPost, LPCSTR url, CString &psHeaderSend, CString &psHeaderReceive, CString &psMessage);
    static bool FindLoginPassword(const char * url);
    static void InsertLoginPassword(CString &url, const char *login, const char *password);
    static void Base64Encode(CString &string);
    static bool Base64Decode(CString &string);
};

#endif
