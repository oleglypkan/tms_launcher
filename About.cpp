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
 static char verid[]="@(#)$RCSfile: About.cpp,v $$Revision: 1.5 $$Date: 2008/03/19 20:38:50Z $"; 
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern CString szWinName;

bool GetVersionInfo(CString &string, WORD Language, WORD CodePage,
                    const char* StringName = "ProductVersion", UINT VersionDigits = 2,
                    const CString &ModulePath = "");

LRESULT CAbout::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    SetDlgItemText(IDC_ABOUT_VERSION,szWinName);
    CString temp = "";
    GetVersionInfo(temp,0x0409,0x04b0,"CompanyName");
    SetDlgItemText(IDC_COMPANY,temp);
    temp = "";
    GetVersionInfo(temp,0x0409,0x04b0,"LegalCopyright");
    SetDlgItemText(IDC_COPYRIGHT,temp);
    return TRUE;
}

LRESULT CAbout::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}
