/*
    File name: About.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: January 17, 2006
*/

#if !defined(AFX_ABOUT_H__3703822D_29A4_44ED_A6A8_FC738830A235__INCLUDED_)
#define AFX_ABOUT_H__3703822D_29A4_44ED_A6A8_FC738830A235__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef INCLUDE_VERID
 static char About_h[]="@(#)$RCSfile: About.h,v $$Revision: 1.3 $$Date: 2006/01/17 15:42:56Z $";
#endif

#include "resource.h"

class CAbout : public CDialogImpl<CAbout>
{
public:
    enum { IDD = IDD_ABOUTBOX };

    BEGIN_MSG_MAP(CAbout)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

#endif // !defined(AFX_ABOUT_H__3703822D_29A4_44ED_A6A8_FC738830A235__INCLUDED_)
