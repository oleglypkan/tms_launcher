/*
    File name: maindlg.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: January 17, 2006
*/

#if !defined(AFX_MAINDLG_H__A8A891A0_D906_4BC5_BAF8_C17E58A38338__INCLUDED_)
#define AFX_MAINDLG_H__A8A891A0_D906_4BC5_BAF8_C17E58A38338__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifdef INCLUDE_VERID
 static char maindlg_h[]="@(#)$RCSfile: maindlg.h,v $$Revision: 1.18 $$Date: 2006/03/21 14:56:08Z $";
#endif

#include "Systray.h"
#include "Task.h"
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
        MSG_WM_CONTEXTMENU(OnContextMenu);
        NOTIFY_CODE_HANDLER_EX(EN_MSGFILTER,OnMsgFilter)
        NOTIFY_CODE_HANDLER_EX(EN_LINK,OnLink)
        COMMAND_ID_HANDLER_EX(VIEW_TASK,OnViewTask)
        COMMAND_ID_HANDLER_EX(VIEW_CHILD_TASKS,OnViewTask)
        COMMAND_ID_HANDLER_EX(VIEW_PARENT_TASK,OnViewTask)
        COMMAND_ID_HANDLER_EX(VIEW_TASK_HOTKEY,OnViewTask)
        COMMAND_ID_HANDLER_EX(VIEW_CHILD_TASKS_HOTKEY,OnViewTask)
        COMMAND_ID_HANDLER_EX(VIEW_PARENT_TASK_HOTKEY,OnViewTask)
        COMMAND_ID_HANDLER_EX(IDC_SETTINGS,OnSettings)
        COMMAND_ID_HANDLER_EX(ID_SETTINGS_GENERAL,OnSettings)
        COMMAND_ID_HANDLER_EX(ID_SETTINGS_URLS,OnSettings)
        COMMAND_ID_HANDLER_EX(ID_SETTINGS_DEFECTS,OnSettings)
        COMMAND_ID_HANDLER_EX(ID_SETTINGS_FORMAT,OnSettings)
        COMMAND_ID_HANDLER_EX(ID_SETTINGS_SOFTTEST,OnSettings)
        COMMAND_ID_HANDLER_EX(ID_SHOW_HIDE_WINDOW,OnShowHide)
        COMMAND_ID_HANDLER_EX(ID_CLEAR_HISTORY,OnClearHistory)
        COMMAND_ID_HANDLER_EX(IDC_EXPAND,OnExpand)
        COMMAND_ID_HANDLER_EX(IDC_CONTENTS,OnContents)
        COMMAND_ID_HANDLER_EX(ID_MENU_UNDO,OnContextMenuCommand)
        COMMAND_ID_HANDLER_EX(ID_MENU_CUT,OnContextMenuCommand)
        COMMAND_ID_HANDLER_EX(ID_MENU_COPY,OnContextMenuCommand)
        COMMAND_ID_HANDLER_EX(ID_MENU_PASTE,OnContextMenuCommand)
        COMMAND_ID_HANDLER_EX(ID_MENU_DELETE,OnContextMenuCommand)
        COMMAND_ID_HANDLER_EX(ID_MENU_SELECT,OnContextMenuCommand)
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
    void OnContextMenu(HWND hwndFrom, CPoint CursorPos);
    void OnContextMenuCommand(UINT code, int idFrom, HWND hwndFrom);
    void OnViewTask(UINT wNotifyCode, INT wID, HWND hWndCtl);
    void OnSettings(UINT wNotifyCode, INT wID, HWND hWndCtl);
    void OnShowHide(UINT wNotifyCode, INT wID, HWND hWndCtl);
    void OnClearHistory(UINT wNotifyCode, INT wID, HWND hWndCtl);
    void OnExpand(UINT wNotifyCode, INT wID, HWND hWndCtl);
    void OnContents(UINT wNotifyCode, INT wID, HWND hWndCtl);
    void OnHide(UINT wNotifyCode, INT wID, HWND hWndCtl);
    void OnClose(UINT wNotifyCode, INT wID, HWND hWndCtl);
    CRichEditCtrl TaskNameControl;
    CComboBox TaskNameCombo;
    void OpenTask(const char *Request, bool SingleTask);
    void OpenDefects(const std::vector<TASKNAME> &Defects, INT wID);
protected:
    CMenu SystrayMenu;
    CMenu ClipboardSubMenu;
    CMenu SettingsSubMenu;
    CMenu ContextMenu;
    bool Expanded;
    bool ShowModal;
    void SwitchControls();
    bool GetTaskNameFromRichEdit(CString &sTasks);
    bool GetTaskNameFromClipboard(CString &sTasks);
    void CreateRequest(const char *sClientName, const char *sIDName, CString &Request, INT wID);
    void AddToHistory(const char *item);
    void OpenDefectsInOneProject(const char *Project, const char *IDs, INT wID);
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINDLG_H__A8A891A0_D906_4BC5_BAF8_C17E58A38338__INCLUDED_)
