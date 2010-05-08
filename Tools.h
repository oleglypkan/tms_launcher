/*
    File name: Tools.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
*/

#ifndef TOOLS_H_INCLUDED
#define TOOLS_H_INCLUDED

#ifdef INCLUDE_VERID
 static char tools_h[]="@(#)$RCSfile: Tools.h,v $$Revision: 1.6 $$Date: 2009/03/27 12:34:56Z $";
#endif

class ModalStateClass
{
public:
    ModalStateClass()
    {
        Modal = 0;
    }
    bool IsModal()
    {
        return (Modal != 0);
    }
    void SetModal()
    {
        Modal++;
    }
    void UnsetModal()
    {
        Modal--;
    }
protected:
    DWORD Modal;
};

extern ModalStateClass ModalState;
int MyMessageBox(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
DWORD OpenLink(const char* MsgBoxCaption, HWND ParentWindow, const char* Operation, const char* Link, 
               const char* Parameters = NULL, const char* Directory = NULL, INT nShowCmd = SW_SHOWNORMAL);

#endif