/*
    File name: Request.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: January 17, 2006
*/

#include "stdafx.h"
#include "Request.h"
#include <vector>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

REQUEST::REQUEST()
{

}

REQUEST::~REQUEST()
{

}

//*******************************************************************************************************
// GetHostAddress: 
//                  Resolve using DNS or similar(WINS,etc) the IP 
//                   address for a domain name such as www.wdj.com. 
//*******************************************************************************************************
DWORD REQUEST::GetHostAddress(LPCSTR host)
{
    struct hostent *phe;
    char *p;

    phe = gethostbyname( host );
            
    if(phe==NULL)
        return 0;
    
    p = *phe->h_addr_list;
    return *((DWORD*)p);
}

//*******************************************************************************************************
// SendString: 
//                  Send a string(null terminated) over the specified socket.
//*******************************************************************************************************
void REQUEST::SendString(SOCKET sock,LPCSTR str)
{
    send(sock,str,strlen(str),0);
}

//*******************************************************************************************************
// ValidHostChar: 
//                  Return TRUE if the specified character is valid
//                  for a host name, i.e. A-Z or 0-9 or -.: 
//*******************************************************************************************************
BOOL REQUEST::ValidHostChar(char ch)
{
    return( isalpha(ch) || isdigit(ch) || ch=='-' || ch=='.' || ch==':' );
}

//*******************************************************************************************************
// ParseURL: 
//                  Used to break apart an URL such as 
//                  http://login:password@www.localhost.com:80/TestPost.htm 
//                  into protocol, login, password, port, host and request
//*******************************************************************************************************
void REQUEST::ParseURL(const char *url, CString &protocol, CString &login, CString &password,
                       CString &host, CString &request, int &port)
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
    port = 80;
    pos = URL.Find(":");
    if (pos != -1)
    {
        host = URL.Left(pos);
        URL.Delete(0,pos+1);
        pos = URL.Find("/");
        if (pos != -1)
        {
            port = atoi(URL.Left(pos));
            URL.Delete(0,pos);
            request = URL;
        }
        else
        {
            port = atoi(URL);
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

bool REQUEST::FindLoginPassword(const char * url)
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

void REQUEST::InsertLoginPassword(CString &url, const char *login, const char *password)
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

//*******************************************************************************************************
// SendHTTP: 
//                  Main entry point for this code.  
//                    url           - The URL to GET/POST to/from.
//                    headerSend    - Headers to be sent to the server.
//                    post          - Data to be posted to the server, NULL if GET.
//                    postLength    - Length of data to post.
//                    req           - Contains the message and headerSend sent by the server.
//
//                    returns 1 on failure, 0 on success.
//*******************************************************************************************************
int REQUEST::SendHTTP(const char *url,const char *headerReceive,BYTE *post,
                      DWORD postLength,HTTPRequest *req)
{
    WSADATA         WsaData;
    SOCKADDR_IN     sin;
    SOCKET          sock;
    char            buffer[1024];
    CString         protocol, host, request, login, password;
    int             port;
    CString         headerSend;
    
    ParseURL(url,protocol,login,password,host,request,port);

    if (protocol.CompareNoCase("HTTP") != 0)
    {
        return 1;
    }

    if (WSAStartup(0x0101, &WsaData) != 0) return 1;               // Init Winsock

    sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) return 1;
    
    sin.sin_family = AF_INET;                                       // Connect to web sever
    sin.sin_port = htons( (unsigned short)port );
    sin.sin_addr.s_addr = GetHostAddress(host);

    if (connect(sock,(LPSOCKADDR)&sin, sizeof(SOCKADDR_IN)))
    {
        return 1;
    }

    // Sending request
    if (request.IsEmpty())
    {
        request = "/";
    }

    if (post == NULL)
    {
        SendString(sock,"GET ");
        headerSend += "GET ";
    }
    else 
    {
        SendString(sock,"POST ");
        headerSend += "POST ";
    }

    SendString(sock,request);
        headerSend += request;

    SendString(sock," HTTP/1.0\r\n");
        headerSend += " HTTP/1.0\r\n";

    if ((post == NULL) && (!login.IsEmpty()) && (!password.IsEmpty()))
    {
        CString AuthString = login + ":" + password;
        Base64Encode(AuthString);

        SendString(sock,"Authorization: Basic "+AuthString+" realm=\"Enter your TMS EMPLOYEE ID and TMS password\"\r\n");
            headerSend += "Authorization: Basic "+AuthString+" realm=\"Enter your TMS EMPLOYEE ID and TMS password\"\r\n";
    }

    SendString(sock,"Accept: image/gif, image/x-xbitmap,"
        " image/jpeg, image/pjpeg, application/vnd.ms-excel,"
        " application/msword, application/vnd.ms-powerpoint,"
        " */*\r\n");
        headerSend += "Accept: image/gif, image/x-xbitmap,"
        " image/jpeg, image/pjpeg, application/vnd.ms-excel,"
        " application/msword, application/vnd.ms-powerpoint,"
        " */*\r\n";

    SendString(sock,"Accept-Language: en-us\r\n");
        headerSend += "Accept-Language: en-us\r\n";

    SendString(sock,"Accept-Encoding: gzip, deflate\r\n");
        headerSend += "Accept-Encoding: gzip, deflate\r\n";

    SendString(sock,"User-Agent: Mozilla/4.0\r\n");
        headerSend += "User-Agent: Mozilla/4.0\r\n";

    if ((postLength > 0) && (postLength <= 1000))
    {
        CString ContentLength;
        ContentLength.Format("Content-Length: %ld\r\n",postLength);
        SendString(sock,ContentLength);
        headerSend += ContentLength;
    }

    SendString(sock,"Host: " + host);
        headerSend += "Host: " + host;

    SendString(sock,"\r\n");
        headerSend += "\r\n";

    if ((headerReceive != NULL) && *headerReceive)
    {
        SendString(sock,headerReceive);
        headerSend += headerReceive;
    }
    
    SendString(sock,"\r\n");    // Send a blank line to signal end of HTTP headerReceive
        headerSend += "\r\n";

    if ((post != NULL) && postLength)
    {
        send(sock,(const char*)post,postLength,0);
        post[postLength] = '\0';
        headerSend += (const char*)post;
    }
    
    req->headerSend += headerSend;

    // Read the result (First read HTTP headerReceive)
    int chars = 0;
    bool done = false;
    int l = 0;

    while (!done)
    {
        l = recv(sock,buffer,1,0);

        if (l<0) done = true;

        switch(*buffer)
        {
            case '\r':
                break;
            case '\n':
                if (chars == 0) done = true;
                chars = 0;
                break;
            default:
                chars++;
                break;
        }

        req->headerReceive += buffer;
    }

    // Reading the HTTP body
    do
    {
        l = recv(sock,buffer,sizeof(buffer)-1,0);
        if (l<0) break;
        *(buffer+l)=0;
        req->message += buffer;
    } while(l>0);

    closesocket(sock);                                          // Cleanup

    return 0;
}

void REQUEST::Base64Encode(CString &string)
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
            temp.Insert(0,itoa(ch % 2,c,10));
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
    for (i = 0; i < count; i++)
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
    for (i = 0; i < Base64Digits.size(); i++)
    {
        string += Base64Table[Base64Digits[i]];
    }
    for (i = 0; i < PaddingChars; i++)
    {
        string += '=';
    }
}

bool REQUEST::Base64Decode(CString &string)
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
    for (i = 0; i < Base64Digits.size(); i++)
    {
        unsigned char ch = (unsigned char)Base64Digits[i];
        // decimal to binary
        CString temp = "";
        while (ch > 0)
        {
            char c[2] = "";
            temp.Insert(0,itoa(ch % 2,c,10));
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
    for (i = 0; i < count; i++)
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

//*******************************************************************************************************
// SendRequest
//
//*******************************************************************************************************
bool REQUEST::SendRequest(bool IsPost, const char *url, CString &psHeaderSend, CString &psHeaderReceive, CString &psMessage)
{
    HTTPRequest req;
    int res = -1;

    if (IsPost)
    {                                                   /* POST */
        DWORD i = psHeaderSend.GetLength();
        char *buffer = new char[i+1];
        strcpy(buffer, psHeaderSend);
        res = SendHTTP(url,"Content-Type: application/x-www-form-urlencoded\r\n",(BYTE*)buffer,i,&req);
        delete [] buffer;
    }
    else                                                /* GET */
    {
        res = SendHTTP(url,NULL,NULL,0,&req);
    }

    if (!res)                                           // Output message and/or headerSend 
    {
        psHeaderSend = req.headerSend;
        psHeaderReceive = req.headerReceive;
        psMessage = req.message;
        return true;
    }
    else
    {
        return false;
    }
}

