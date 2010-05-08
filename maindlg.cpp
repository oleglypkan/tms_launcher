// maindlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "maindlg.h"
#include "Systray.h"
#include "settings.h"
#include <ctype.h>

extern const char* szWinName;
const char* SEPARATORS = " _-*+|:~#@$%^\t";
extern UINT ViewTaskHotKeyID;
extern UINT ViewChildTasksHotKeyID;
extern CSettings Settings;

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    if (Settings.ViewTaskHotKey)
    {
        if (!RegisterHotKey(m_hWnd,ViewTaskHotKeyID,(!(Settings.ViewTaskHotKey&0x500)?
                       HIBYTE(LOWORD(Settings.ViewTaskHotKey)):((Settings.ViewTaskHotKey&0x500)<0x500?
                       HIBYTE(LOWORD(Settings.ViewTaskHotKey))^5:HIBYTE(LOWORD(Settings.ViewTaskHotKey)))),
                       LOBYTE(LOWORD(Settings.ViewTaskHotKey))))
            MessageBox("Hotkey used to View Task is already registered by another program.\nPlease enter another hotkey in TMS Launcher Settings window",szWinName,MB_OK|MB_ICONERROR);
    }
    if (Settings.ViewChildTasksHotKey)
    {
        if (!RegisterHotKey(m_hWnd,ViewChildTasksHotKeyID,(!(Settings.ViewChildTasksHotKey&0x500)?
                        HIBYTE(LOWORD(Settings.ViewChildTasksHotKey)):((Settings.ViewChildTasksHotKey&0x500)<0x500?
                        HIBYTE(LOWORD(Settings.ViewChildTasksHotKey))^5:HIBYTE(LOWORD(Settings.ViewChildTasksHotKey)))),
                        LOBYTE(LOWORD(Settings.ViewChildTasksHotKey))))
            MessageBox("Hotkey used to View Child Tasks is already registered by another program.\nPlease enter another hotkey in TMS Launcher Settings window",szWinName,MB_OK|MB_ICONERROR);
    }
    SetWindowText(szWinName);
    
    TaskNameControl.Attach(GetDlgItem(IDC_TASKNAME));
    TaskNameControl.SetTextMode(TM_PLAINTEXT);
    TaskNameControl.LimitText(1024);
    TaskNameControl.SetEventMask(ENM_MOUSEEVENTS);

    if ((Settings.xPos < 0) && (Settings.yPos < 0))
    {
        CenterWindow();
    }
    else
    {
        SetWindowPos(HWND_TOPMOST,Settings.xPos,Settings.yPos,0,0,SWP_NOSIZE);
    }

    Expanded = Settings.Expand;
    if (!Expanded)
    {
        Expanded = !Expanded;
        OnExpand(0,0,0);
    }
    else
    {
        SetWindowPos(HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
    }
    
    ShowModal = false;
    CMenu SystraySubMenu;
    SystraySubMenu.CreatePopupMenu();
    SystraySubMenu.AppendMenu(MF_ENABLED,VIEW_TASK_HOTKEY,"&View Task");
    SystraySubMenu.AppendMenu(MF_ENABLED,VIEW_CHILD_TASKS_HOTKEY,"View &Child Tasks");
    SystrayMenu.CreatePopupMenu();
    SystrayMenu.AppendMenu(MF_ENABLED,VIEW_TASK,"&View Task");
    SystrayMenu.AppendMenu(MF_ENABLED,VIEW_CHILD_TASKS,"View &Child Tasks");
    SystrayMenu.AppendMenu(MF_SEPARATOR);
    SystrayMenu.AppendMenu(MF_POPUP,(UINT)(HMENU)SystraySubMenu,"Clip&board");
    SystrayMenu.AppendMenu(MF_SEPARATOR);
    SystrayMenu.AppendMenu(MF_ENABLED,IDC_SETTINGS,"&Settings");
    SystrayMenu.AppendMenu(MF_SEPARATOR);
    SystrayMenu.AppendMenu(MF_ENABLED,IDC_CLOSE,"&Exit");
    CreateSysTrayIcon(m_hWnd,LoadIcon(_Module.GetModuleInstance(),MAKEINTRESOURCE(IDI_SYSTRAY_ICON)),szWinName);

    return TRUE;
}

LRESULT CMainDlg::OnMyIconNotify(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (lParam)
    {
        POINT Point;
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONDOWN:
            if (IsWindowVisible())
            {
                ShowWindow(SW_HIDE);
            }
            else
            {
                ShowWindow(SW_SHOWNORMAL);
                SetForegroundWindow(m_hWnd);
            }
            break;
        case WM_RBUTTONDOWN:
            if (ShowModal) return 1;
            GetCursorPos(&Point);
            SetForegroundWindow(m_hWnd);
            TrackPopupMenuEx(SystrayMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON,Point.x,Point.y,m_hWnd,NULL);
            break;
    }
    return 0;
}

LRESULT CMainDlg::OnTaskbarCreated(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    DeleteSysTrayIcon(m_hWnd);
    CreateSysTrayIcon(m_hWnd,LoadIcon(_Module.GetModuleInstance(),MAKEINTRESOURCE(IDI_SYSTRAY_ICON)),szWinName);
    return 0;
}

LRESULT CMainDlg::OnTMSLauncherActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    ShowWindow(SW_SHOWNORMAL);
    SetForegroundWindow(m_hWnd);
    return 0;
}

void CMainDlg::OnHide(UINT wNotifyCode, INT wID, HWND hWndCtl)
{
    ShowWindow(SW_HIDE);
}

void CMainDlg::OnClose(UINT wNotifyCode, INT wID, HWND hWndCtl)
{
    RECT Rect;
    GetWindowRect(&Rect);
    Settings.xPos = Rect.left;
    Settings.yPos = Rect.top;
    Settings.SaveSettings();

    DeleteSysTrayIcon(m_hWnd);
    DestroyMenu(SystrayMenu);

    if (wID == IDC_CLOSE)
    {
        DestroyWindow();
        ::PostQuitMessage(0);
    }
}

LRESULT CMainDlg::OnEndSession(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/)
{
    if (wParam) // the session is being ended
    {
        OnClose(0,0,0);
    }
    return 0;
}

LRESULT CMainDlg::OnDisplayChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    RECT DesktopRect, WindowRect;
    ::GetWindowRect(GetDesktopWindow(),&DesktopRect);
    GetWindowRect(&WindowRect);
    Settings.xPos = WindowRect.left;
    Settings.yPos = WindowRect.top;

    if ((Settings.xPos < DesktopRect.left)||(Settings.xPos > DesktopRect.right)||
        (Settings.yPos < DesktopRect.top)||(Settings.yPos > DesktopRect.bottom))
    {
        CenterWindow();
    }
    return 0;
}

void CMainDlg::OnExpand(UINT wNotifyCode, INT wID, HWND hWndCtl)
{
    RECT WindowRect;
    RECT ButtonRect;
    GetWindowRect(&WindowRect);
    ::GetWindowRect(GetDlgItem(VIEW_CHILD_TASKS),&ButtonRect);
    if (Expanded) // window is expanded, make it collapsed
    {
        WindowRect.bottom -= ButtonRect.bottom - ButtonRect.top + 6;
        SetDlgItemText(IDC_EXPAND,">>");
        ::EnableWindow(GetDlgItem(VIEW_CHILD_TASKS),FALSE);
        ::EnableWindow(GetDlgItem(IDC_SETTINGS),FALSE);
    }
    else
    {
        WindowRect.bottom += ButtonRect.bottom - ButtonRect.top + 6;
        SetDlgItemText(IDC_EXPAND,"<<");
        ::EnableWindow(GetDlgItem(VIEW_CHILD_TASKS),TRUE);
        ::EnableWindow(GetDlgItem(IDC_SETTINGS),TRUE);
    }
    SetWindowPos(HWND_TOPMOST,WindowRect.left,WindowRect.top,
                 WindowRect.right-WindowRect.left,WindowRect.bottom-WindowRect.top,0);
    Expanded = !Expanded;
}

bool CMainDlg::IsTaskNameValid(CString &sTaskName)
{
    const UINT MinTaskName = 7;
    const UINT MaxTaskName = 11;
    const UINT MinClientName = 3;
    const UINT MaxClientName = 5;
    const UINT MinIDName = 3;
    const UINT MaxIDName = 5;

    if (sTaskName.IsEmpty()) return false;

    sTaskName.TrimLeft();
    sTaskName.TrimRight();
    if ((sTaskName.GetLength()<MinTaskName)||(sTaskName.GetLength()>MaxTaskName)) return false;
    
    int Separator = sTaskName.FindOneOf(SEPARATORS);
    if ((Separator == -1)||(Separator < MinClientName)||(Separator > MaxClientName)||
        (sTaskName.GetLength()-Separator-1 < MinIDName)||(sTaskName.GetLength()-Separator-1 > MaxIDName)) return false;
    sTaskName.SetAt(Separator,'-');
    for (int i=0; i<Separator-1; i++)
    {
        if (!isalpha(sTaskName[i])) return false;
    }
    // the last symbol in Client Name can be either an alpha character or a number (i.e. QARD3)
    if ((!isalpha(sTaskName[Separator-1])) && (!isdigit(sTaskName[Separator-1]))) return false;

    for (int j=Separator+1; j<sTaskName.GetLength(); j++)
    {
        if (!isdigit(sTaskName[j])) return false;
    }
    return true;
}

bool CMainDlg::GetTaskNameFromRichEdit(CString &sTaskName)
{
    UINT TaskNameLength = TaskNameControl.GetWindowTextLength();
    ::GetWindowText(TaskNameControl,sTaskName.GetBuffer(TaskNameLength+1),TaskNameLength+1);
    sTaskName.ReleaseBuffer();
    return IsTaskNameValid(sTaskName);
}

bool CMainDlg::GetTaskNameFromClipboard(CString &sTaskName)
{
    if (!::OpenClipboard(m_hWnd)) return false;
    if (IsClipboardFormatAvailable(CF_TEXT))
    {
        HANDLE ClipboardData = GetClipboardData(CF_TEXT);
        if (ClipboardData)
        {
            LPSTR TaskName = (LPSTR)GlobalLock(ClipboardData);
            sTaskName = TaskName;
            GlobalUnlock(ClipboardData);
        }
    }
    CloseClipboard();
    return IsTaskNameValid(sTaskName);
}

void CMainDlg::OnViewTask(UINT wNotifyCode, INT wID, HWND hWndCtl)
{
    CString sTaskName = "";
    CString Request = "";
    CString ErrorString = "Invalid task name format";

    // called after VIEW_TASK or VIEW_CHILD_TASKS button was pressed
    if ((wID == VIEW_TASK)||(wID == VIEW_CHILD_TASKS))
    {
        if (!GetTaskNameFromRichEdit(sTaskName))
        {
            ShowModal = true;
            MessageBox(ErrorString,szWinName,MB_OK|MB_ICONERROR);
            ShowModal = false;
            return;
        }
    }
    else // called after pressing ViewTaskHotKey or ViewChildTasksHotKey
    {
        if (!GetTaskNameFromClipboard(sTaskName))
        {
            ShowModal = true;
            MessageBox(ErrorString+" in clipboard",szWinName,MB_OK|MB_ICONERROR);
            ShowModal = false;
            return;
        }
        else TaskNameControl.SetWindowText(sTaskName);
    }

    bool ViewChildTasks = ((wID == VIEW_CHILD_TASKS)||(static_cast<UINT>(wID) == ViewChildTasksHotKeyID)||
                           (wID == VIEW_CHILD_TASKS_HOTKEY));

    CreateRequest(sTaskName, Request, ViewChildTasks, (Settings.TMS==1));
    ::ShellExecute(NULL,"open",Request,NULL,"",SW_SHOWNORMAL);
}

void CMainDlg::CreateRequest(const char *Text, CString &Request, bool ViewChildTasks, bool AltTMS)
{
    CString Temp = Text;

    if (ViewChildTasks)
    {
        Temp.Replace("-","&ParentID=");
        if (AltTMS)
            Request = "http://scc1/~alttms/showtasks.php?ParentClient=";
        else
            Request = "http://www.softcomputer.com/tms/showtasks.php?ParentClient=";
    }
    else
    {
        Temp.Replace("-","&ID=");
        if (AltTMS)
            Request = "http://scc1/~alttms/viewtask.php?Client=";
        else
            Request = "http://www.softcomputer.com/tms/viewtask.php?Client=";
    }
    Request += Temp;
}

LRESULT CMainDlg::OnHotKey(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/)
{
    if (!ShowModal)
    {
        OnViewTask(0,wParam,0);
    }
    return 0;
}

void CMainDlg::OnSettings(UINT wNotifyCode, INT wID, HWND hWndCtl)
{
    CSettingsDlg SettingsDlg(&Settings);
    ShowModal = true;
    SettingsDlg.DoModal();
    ShowModal = false;
}

LRESULT CMainDlg::OnMsgFilter(LPNMHDR pnmh)
{
    MSGFILTER *msg = (MSGFILTER*)pnmh;
    if (msg->nmhdr.hwndFrom == GetDlgItem(IDC_TASKNAME))
        if (msg->msg == WM_LBUTTONUP)
        {
            if (Settings.SingleClick) TaskNameControl.SetSelAll();
        }
        if (msg->msg == WM_RBUTTONDOWN)
        {
            OnViewTask(0,Settings.RightClickAction ? VIEW_CHILD_TASKS_HOTKEY:VIEW_TASK_HOTKEY,0);
        }
    return 0;
}
