/*
    File name: About.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: January 17, 2006
*/

#include "stdafx.h"
#include "About.h"

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: About.cpp,v $$Revision: 1.4 $$Date: 2006/01/17 15:42:08Z $"; 
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern CString szWinName;

LRESULT CAbout::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    SetDlgItemText(IDC_ABOUT_VERSION,szWinName);
    return TRUE;
}

LRESULT CAbout::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}
