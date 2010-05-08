/*
    File name: Tools.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
*/

#include "stdafx.h"
#include "Tools.h"

#ifndef NO_VERID
static char verid[]="@(#)$RCSfile: Tools.cpp,v $$Revision: 1.2 $$Date: 2009/03/27 12:34:47Z $"; 
#endif

ModalStateClass ModalState;

int MyMessageBox(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
    ModalState.SetModal();
    int res = ::MessageBox(hWnd, lpText, lpCaption, uType);
    ModalState.UnsetModal();
    return res;
}

DWORD OpenLink(const char* MsgBoxCaption, HWND ParentWindow, const char* Operation, const char* Link, 
               const char* Parameters, const char* Directory, INT nShowCmd)
{
    if (32 >= (int)ShellExecute(ParentWindow,Operation,Link,Parameters,Directory,nShowCmd))
    {
        DWORD LastError = GetLastError();
        if ((LastError != ERROR_FILE_NOT_FOUND) && (LastError != CO_E_APPNOTFOUND))
        {
            LPVOID lpMsgBuf;
            if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, LastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf, 0, NULL))
            {
                MyMessageBox(ParentWindow,(LPCTSTR)lpMsgBuf,MsgBoxCaption,MB_ICONERROR);
                LocalFree(lpMsgBuf);
            }
        }
        return LastError;
    }
    else
    {
        return 0;
    }
}
