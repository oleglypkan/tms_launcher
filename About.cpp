// About.cpp: implementation of the CAbout class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "About.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern const char* szWinName;

LRESULT CAbout::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    CString Version = szWinName;
    Version.Replace('v', ' ');
    Version.Insert(Version.GetLength()-4,"version");
    SetDlgItemText(IDC_ABOUT_VERSION,Version);
	return TRUE;
}

LRESULT CAbout::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}
