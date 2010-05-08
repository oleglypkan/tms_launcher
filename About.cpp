// About.cpp: implementation of the CAbout class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "About.h"

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: About.cpp,v $$Revision: 1.3 $$Date: 2005/06/15 15:26:34Z $"; 
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
