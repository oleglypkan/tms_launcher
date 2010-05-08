/*
    File name: Tools.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: January 17, 2006
*/

#ifdef INCLUDE_VERID
 static char tools_h[]="@(#)$RCSfile: Tools.h,v $$Revision: 1.3 $$Date: 2006/01/17 15:58:34Z $";
#endif
 
BOOL OpenLink(const char* MsgBoxCaption, HWND ParentWindow, const char* Operation, const char* Link, 
              const char* Parameters = NULL, const char* Directory = NULL, INT nShowCmd = SW_SHOWNORMAL)
{
    if (32 >= (int)ShellExecute(ParentWindow,Operation,Link,Parameters,Directory,nShowCmd))
    {
        LPVOID lpMsgBuf;
        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR) &lpMsgBuf, 0, NULL))
        {
            ::MessageBox(ParentWindow,(LPCTSTR)lpMsgBuf,MsgBoxCaption,MB_ICONERROR);
            LocalFree(lpMsgBuf);
        }
        return FALSE;
    }
    else
        return TRUE;
}