// maindlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINDLG_H__A8A891A0_D906_4BC5_BAF8_C17E58A38338__INCLUDED_)
#define AFX_MAINDLG_H__A8A891A0_D906_4BC5_BAF8_C17E58A38338__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Systray.h"
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
    void OnViewTask(UINT wNotifyCode, INT wID, HWND hWndCtl);
    void OnSettings(UINT wNotifyCode, INT wID, HWND hWndCtl);
    void OnExpand(UINT wNotifyCode, INT wID, HWND hWndCtl);
    void OnHide(UINT wNotifyCode, INT wID, HWND hWndCtl);
    void OnClose(UINT wNotifyCode, INT wID, HWND hWndCtl);
protected:
    CMenu SystrayMenu;
    CRichEditCtrl TaskNameControl;
    bool Expanded;
    bool ShowModal;
    bool IsTaskNameValid(CString &sTaskName);
    bool GetTaskNameFromRichEdit(CString &sTaskName);
    bool GetTaskNameFromClipboard(CString &sTaskName);
    void CreateRequest(const char *Text, CString &Request, bool ViewChildTasks, bool AltTMS);
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINDLG_H__A8A891A0_D906_4BC5_BAF8_C17E58A38338__INCLUDED_)
