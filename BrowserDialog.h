/*
    File name: BrowserDialog.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
*/

#ifndef BROWSERDIALOG_H_INCLUDED
#define BROWSERDIALOG_H_INCLUDED

#ifdef INCLUDE_VERID
 static char tools_h[]="@(#)$RCSfile: BrowserDialog.h,v $$Revision: $$Date: $";
#endif

#include "resource.h"

class CBrowserDialog: public CFileDialogImpl<CBrowserDialog>
{
public:
    CString Parameters;

    CBrowserDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFileName = NULL, DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        LPCTSTR lpszFilter = NULL, HWND hWndParent = NULL): CFileDialogImpl<CBrowserDialog>(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, hWndParent)
    {
        // add custom child dialog with additional controls
        m_ofn.Flags |= OFN_EXPLORER | OFN_ENABLETEMPLATE;
        m_ofn.Flags &= ~OFN_ENABLESIZING; // turn off resizing in order not to do tricks with resizing of custom controls
        m_ofn.hInstance = _Module.GetModuleInstance();
        m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_PARAMETERS);
        Parameters = "";
        Title = "";
    }

    void OnInitDone(LPOFNOTIFY lpon)
    {
        m_EditParameters.Attach(GetDlgItem(IDC_PARAMETERS));
        m_EditParameters.LimitText(MAX_PATH-1);
        m_EditParameters.SetWindowText(Parameters);
    }

    BOOL OnFileOK(LPOFNOTIFY lpon)
    {
        m_EditParameters.GetWindowText(Parameters.GetBuffer(MAX_PATH),MAX_PATH);
        Parameters.ReleaseBuffer();
        return TRUE;
    }

    void SetTitle(const char *strTitle)
    {
        Title = strTitle;
        m_ofn.lpstrTitle = Title;
    }

protected:
    CEdit m_EditParameters;
    CString Title;
};

#endif