// maindlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINDLG_H__A8A891A0_D906_4BC5_BAF8_C17E58A38338__INCLUDED_)
#define AFX_MAINDLG_H__A8A891A0_D906_4BC5_BAF8_C17E58A38338__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifdef INCLUDE_VERID
 static char maindlg_h[]="@(#)$RCSfile: maindlg.h,v $$Revision: 1.9 $$Date: 2005/07/04 10:08:57Z $";
#endif

#include "Systray.h"
#include <vector>
extern const UINT WM_TMS_LAUNCHER_ACTIVATE;

class CMainDlg : public CDialogImpl<CMainDlg>
{
public:
    enum { IDD = TMS_LAUNCHER_DIALOG };

    BEGIN_MSG_MAP(CMainDlg)
        MESSAGE_HANDLER_EX(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER_EX(WM_MYICONNOTIFY,OnMyIconNotify)
        MESSAGE_HANDLER_EX(WM_TASKBARCREATED,OnTaskbarCreated)
        MESSAGE_HANDLER_EX(WM_TMS_LAUNCHER_ACTIVATE,OnTMSLauncherActivate)
        MESSAGE_HANDLER_EX(WM_HOTKEY,OnHotKey)
        MESSAGE_HANDLER_EX(WM_ENDSESSION,OnEndSession)
        MESSAGE_HANDLER_EX(WM_DISPLAYCHANGE,OnDisplayChange)
        NOTIFY_CODE_HANDLER_EX(EN_MSGFILTER,OnMsgFilter)
        NOTIFY_CODE_HANDLER_EX(EN_LINK,OnLink)
        COMMAND_ID_HANDLER_EX(VIEW_TASK,OnViewTask)
        COMMAND_ID_HANDLER_EX(VIEW_CHILD_TASKS,OnViewTask)
        COMMAND_ID_HANDLER_EX(VIEW_TASK_HOTKEY,OnViewTask)
        COMMAND_ID_HANDLER_EX(VIEW_CHILD_TASKS_HOTKEY,OnViewTask)
        COMMAND_ID_HANDLER_EX(IDC_SETTINGS,OnSettings)
        COMMAND_ID_HANDLER_EX(IDC_EXPAND,OnExpand)
        COMMAND_ID_HANDLER_EX(IDCANCEL,OnHide)
        COMMAND_ID_HANDLER_EX(IDC_CLOSE,OnClose)
        COMMAND_ID_HANDLER_EX(WM_ENDSESSION,OnClose)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    LRESULT OnMyIconNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    LRESULT OnTaskbarCreated(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    LRESULT OnTMSLauncherActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    LRESULT OnHotKey(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    LRESULT OnEndSession(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    LRESULT OnDisplayChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    LRESULT OnMsgFilter(LPNMHDR pnmh);
    LRESULT OnLink(LPNMHDR pnmh);
    void OnViewTask(UINT wNotifyCode, INT wID, HWND hWndCtl);
    void OnSettings(UINT wNotifyCode, INT wID, HWND hWndCtl);
    void OnExpand(UINT wNotifyCode, INT wID, HWND hWndCtl);
    void OnHide(UINT wNotifyCode, INT wID, HWND hWndCtl);
    void OnClose(UINT wNotifyCode, INT wID, HWND hWndCtl);
protected:
    CMenu SystrayMenu;
    CMenu SystraySubMenu;
    CRichEditCtrl TaskNameControl;
    bool Expanded;
    bool ShowModal;
    void ParseTasks(CString &sTasks, std::vector<CString> &Tasks);
    bool IsTaskNameValid(CString &sTaskName, CString &sClientName, CString &sIDName);
    bool GetTaskNameFromRichEdit(CString &sTasks);
    bool GetTaskNameFromClipboard(CString &sTasks);
    void CreateRequest(const char *sClientName, const char *sIDName, CString &Request, INT wID);
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINDLG_H__A8A891A0_D906_4BC5_BAF8_C17E58A38338__INCLUDED_)
