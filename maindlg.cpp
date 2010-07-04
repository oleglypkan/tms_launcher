/*
    File name: maindlg.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
*/

#include "stdafx.h"

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: maindlg.cpp,v $$Revision: 1.72 $$Date: 2009/03/29 22:43:31Z $"; 
#endif

#include <fstream>
#include "resource.h"
#include "maindlg.h"
#include "Systray.h"
#include "settings.h"
#include "tools.h"
#include "Options.h"
#include "Task.h"
#include "AmHttpSocket.h"
using Mortimer::COptionSheetDialogImpl;
using Mortimer::COptionSelectionTreeCtrl;
using namespace std;
extern CString szWinName;
extern CSettings Settings;

#ifdef _DEBUG
#include <crtdbg.h>
#include <stdlib.h>
#define _CRTDBG_MAP_ALLOC 
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

bool isalpha_cp1251(unsigned char ch);
int CompareNoCaseCP1251(const char *string1, const char *string2);
void StringToUpperCase(CString &String);

#define TrackMouseEventTimer 101
#define TimerDelay 100

CMainDlg::CMainDlg()
{
    TimerIsSet = 0;
    user32 = NULL;
    SetLayeredWindowAttributes = NULL;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    UINT HotkeyID = 0;
    for (unsigned int i=0; i<Settings.links.size(); i++)
    {
        if (Settings.links[i].ViewTaskHotKey)
        {
            UINT HotKey = Settings.links[i].ViewTaskHotKey;
            if (!RegisterHotKey(m_hWnd,HotkeyID,(!(HotKey&0x500)?
                           HIBYTE(LOWORD(HotKey)):((HotKey&0x500)<0x500?
                           HIBYTE(LOWORD(HotKey))^5:HIBYTE(LOWORD(HotKey)))),
                           LOBYTE(LOWORD(HotKey))))
            {
                CString message;
                message.Format("Hotkey used to View Task in \"%s\" is already registered and will not work as expected.\nPlease enter another hotkey in TMS Launcher Settings window",
                               Settings.links[i].Caption);
                MyMessageBox(m_hWnd,message,szWinName,MB_ICONERROR);
            }
        }
        HotkeyID++;
        if (Settings.links[i].ViewChildTasksHotKey)
        {
            UINT HotKey = Settings.links[i].ViewChildTasksHotKey;
            if (!RegisterHotKey(m_hWnd,HotkeyID,(!(HotKey&0x500)?
                           HIBYTE(LOWORD(HotKey)):((HotKey&0x500)<0x500?
                           HIBYTE(LOWORD(HotKey))^5:HIBYTE(LOWORD(HotKey)))),
                           LOBYTE(LOWORD(HotKey))))
            {
                CString message;
                message.Format("Hotkey used to View Child Tasks in \"%s\" is already registered and will not work as expected.\nPlease enter another hotkey in TMS Launcher Settings window",
                               Settings.links[i].Caption);
                MyMessageBox(m_hWnd,message,szWinName,MB_ICONERROR);
            }
        }
        HotkeyID++;
        if (Settings.links[i].ViewParentTaskHotKey)
        {
            UINT HotKey = Settings.links[i].ViewParentTaskHotKey;
            if (!RegisterHotKey(m_hWnd,HotkeyID,(!(HotKey&0x500)?
                           HIBYTE(LOWORD(HotKey)):((HotKey&0x500)<0x500?
                           HIBYTE(LOWORD(HotKey))^5:HIBYTE(LOWORD(HotKey)))),
                           LOBYTE(LOWORD(HotKey))))
            {
                CString message;
                message.Format("Hotkey used to View Parent Task in \"%s\" is already registered and will not work as expected.\nPlease enter another hotkey in TMS Launcher Settings window",
                               Settings.links[i].Caption);
                MyMessageBox(m_hWnd,message,szWinName,MB_ICONERROR);
            }
        }
        HotkeyID++;
        if (Settings.links[i].ViewRelatedTasksHotKey)
        {
            UINT HotKey = Settings.links[i].ViewRelatedTasksHotKey;
            if (!RegisterHotKey(m_hWnd,HotkeyID,(!(HotKey&0x500)?
                HIBYTE(LOWORD(HotKey)):((HotKey&0x500)<0x500?
                HIBYTE(LOWORD(HotKey))^5:HIBYTE(LOWORD(HotKey)))),
                LOBYTE(LOWORD(HotKey))))
            {
                CString message;
                message.Format("Hotkey used to View Related Tasks in \"%s\" is already registered and will not work as expected.\nPlease enter another hotkey in TMS Launcher Settings window",
                               Settings.links[i].Caption);
                MyMessageBox(m_hWnd,message,szWinName,MB_ICONERROR);
            }
        }
        HotkeyID++;
    }
    if (Settings.HotKey || Settings.WinKey)
    {
        if (!RegisterHotKey(m_hWnd,Settings.GlobalHotkeyID,Settings.WinKey|(!(Settings.HotKey&0x500)?HIBYTE(LOWORD(Settings.HotKey)):
        ((Settings.HotKey&0x500)<0x500?HIBYTE(LOWORD(Settings.HotKey))^5:HIBYTE(LOWORD(Settings.HotKey)))),LOBYTE(LOWORD(Settings.HotKey)))) 
        {
            MyMessageBox(m_hWnd,"The hotkey entered on \"Other\" page is already registered and will not work as expected.\nPlease enter another hotkey in TMS Launcher Settings window",szWinName,MB_ICONERROR);
        }
    }

    SetWindowText(szWinName);
    
    TaskNameControl.Attach(GetDlgItem(IDC_TASKNAME));
    TaskNameControl.SetTextMode(TM_PLAINTEXT);
    TaskNameControl.SetEventMask(ENM_MOUSEEVENTS|ENM_LINK);
    TaskNameControl.SetAutoURLDetect(true);

    TaskNameCombo.Attach(GetDlgItem(IDC_TASKNAME_COMBO));
    LoadHistory();

    SwitchControls();

    // creation of RichEdit control context menu
    ContextMenu.CreatePopupMenu();
    ContextMenu.AppendMenu(MF_ENABLED,ID_MENU_UNDO,"&Undo");
    ContextMenu.AppendMenu(MF_SEPARATOR,0,"");
    ContextMenu.AppendMenu(MF_ENABLED,ID_MENU_CUT,"Cu&t");
    ContextMenu.AppendMenu(MF_ENABLED,ID_MENU_COPY,"&Copy");
    ContextMenu.AppendMenu(MF_ENABLED,ID_MENU_PASTE,"&Paste");
    ContextMenu.AppendMenu(MF_ENABLED,ID_MENU_DELETE,"&Delete");
    ContextMenu.AppendMenu(MF_SEPARATOR,0,"");
    ContextMenu.AppendMenu(MF_ENABLED,ID_MENU_SELECT,"Select &All");

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
    
    ClipboardSubMenu.CreatePopupMenu();
    ClipboardSubMenu.AppendMenu(MF_ENABLED,VIEW_TASK_HOTKEY,"&View Task");
    ClipboardSubMenu.AppendMenu(MF_ENABLED,VIEW_CHILD_TASKS_HOTKEY,"View &Child Tasks");
    ClipboardSubMenu.AppendMenu(MF_ENABLED,VIEW_PARENT_TASK_HOTKEY,"View &Parent Task");
    ClipboardSubMenu.AppendMenu(MF_ENABLED,VIEW_RELATED_TASKS_HOTKEY,"View &Related Tasks");
    
    SettingsSubMenu.CreatePopupMenu();
    SettingsSubMenu.AppendMenu(MF_ENABLED,ID_SETTINGS_GENERAL,"&General");
    SettingsSubMenu.AppendMenu(MF_ENABLED,ID_SETTINGS_URLS,"&URLs");
    SettingsSubMenu.AppendMenu(MF_ENABLED,ID_SETTINGS_DEFECTS,"&Defects");
    SettingsSubMenu.AppendMenu(MF_ENABLED,ID_SETTINGS_FORMAT,"&Format");
    SettingsSubMenu.AppendMenu(MF_ENABLED,ID_SETTINGS_SOFTTEST,"&SoftTest");
    SettingsSubMenu.AppendMenu(MF_ENABLED,ID_SETTINGS_HISTORY,"&History");
    SettingsSubMenu.AppendMenu(MF_ENABLED,ID_SETTINGS_OTHER,"&Other");

    SystrayMenu.CreatePopupMenu();
    SystrayMenu.AppendMenu(MF_ENABLED,ID_SHOW_HIDE_WINDOW,"Show &window");
    SystrayMenu.SetMenuDefaultItem(ID_SHOW_HIDE_WINDOW);
    SystrayMenu.AppendMenu(MF_SEPARATOR);
    SystrayMenu.AppendMenu(MF_ENABLED,VIEW_TASK,"&View Task");
    SystrayMenu.AppendMenu(MF_ENABLED,VIEW_CHILD_TASKS,"View &Child Tasks");
    SystrayMenu.AppendMenu(MF_ENABLED,VIEW_PARENT_TASK,"View &Parent Task");
    SystrayMenu.AppendMenu(MF_ENABLED,VIEW_RELATED_TASKS,"View &Related Tasks");
    SystrayMenu.AppendMenu(MF_SEPARATOR);
    SystrayMenu.AppendMenu(MF_POPUP,(UINT)(HMENU)ClipboardSubMenu,"Clip&board");
    SystrayMenu.AppendMenu(MF_POPUP,(UINT)(HMENU)SettingsSubMenu,"&Settings");
    SystrayMenu.AppendMenu(MF_SEPARATOR);
    if (TaskNameCombo.GetCount() == 0)
    {
        SystrayMenu.AppendMenu(MF_GRAYED,ID_CLEAR_HISTORY,"C&lear history");
    }
    else
    {
        SystrayMenu.AppendMenu(MF_ENABLED,ID_CLEAR_HISTORY,"C&lear history");
    }
    SystrayMenu.AppendMenu(MF_SEPARATOR);
    SystrayMenu.AppendMenu(MF_ENABLED,IDC_CLOSE,"&Exit");
    CreateSysTrayIcon(m_hWnd,LoadIcon(_Module.GetModuleInstance(),MAKEINTRESOURCE(IDI_SYSTRAY_ICON)),szWinName);

    if (OsMajorVer()>=5) // OS is Win2000 or higher
    {
        user32 = LoadLibrary("User32.dll");
        if (user32 != NULL)
        {
            if (Settings.EnableOpacity)
            {
                SetLayeredWindowAttributes = (MYPROC)GetProcAddress(user32, "SetLayeredWindowAttributes");
                if (SetLayeredWindowAttributes != NULL)
                {
                    SetWindowLong(GWL_EXSTYLE,GetWindowLong(GWL_EXSTYLE) | WS_EX_LAYERED);
                    SetLayeredWindowAttributes(m_hWnd,RGB(0,0,0),(255*Settings.ActiveOpacity)/100,LWA_ALPHA);  // 0 - fully transparent; 255 - opaque
                }
            }
        }
    }
    else
    {
        Settings.EnableOpacity = false;
    }

    // subclassing Settings button to draw icon on the button with Visual XP style
    if (IconButton.IsWinVerXPor2003())
    {
        IconButton.SubclassWindow(GetDlgItem(IDC_SETTINGS));
        IconButton.LoadThemeData();
    }
    SendDlgItemMessage(IDC_SETTINGS, BM_SETIMAGE, IMAGE_ICON, 
        (LPARAM)(HICON)::LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDI_SETTINGS),IMAGE_ICON,
        ::GetSystemMetrics(SM_CXSMICON),::GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR));
    
    return TRUE;
}

LRESULT CMainDlg::OnNCHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (SetLayeredWindowAttributes != NULL)
    {
        if (GetForegroundWindow() != m_hWnd)
        {
            if (TimerIsSet == 0)
            {
                TimerIsSet = SetTimer(TrackMouseEventTimer,TimerDelay);
                if (Settings.EnableOpacity) SetLayeredWindowAttributes(m_hWnd,RGB(0,0,0),(255*Settings.ActiveOpacity)/100,LWA_ALPHA);  // 0 - fully transparent; 255 - opaque
            }
        }
    }
    return ::DefWindowProc(m_hWnd,uMsg,wParam,lParam);
}

LRESULT CMainDlg::OnMouseLeave(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    if (TimerIsSet != 0)
    {
        KillTimer(TrackMouseEventTimer);
        TimerIsSet = 0;
    }
    if (SetLayeredWindowAttributes != NULL)
    {
        if (Settings.EnableOpacity) SetLayeredWindowAttributes(m_hWnd,RGB(0,0,0),(255*Settings.InactiveOpacity)/100,LWA_ALPHA);
    }
    return 0;
}

LRESULT CMainDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
        case TrackMouseEventTimer:
            if ((GetForegroundWindow() == m_hWnd) || (!IsWindowVisible()))
            {
                KillTimer(TrackMouseEventTimer);
                TimerIsSet = 0;
                break;
            }
            if (!CursorOverWindow())
            {
                PostMessage(WM_MOUSELEAVE, 0, 0L);
            }
            break;
    }
    return 0;
}

bool CMainDlg::CursorOverWindow()
{
    RECT rc;
    POINT pt;
    GetWindowRect(&rc);
    GetCursorPos(&pt);
    if (!PtInRect(&rc, pt)) 
    {
        return false;
    }
    else
    {
        return true;
    }
}

DWORD CMainDlg::OsMajorVer(void)
{
    OSVERSIONINFO vi;
    vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&vi);
    return vi.dwMajorVersion;
}

void CMainDlg::LoadHistory()
{
    int size = Settings.History.size();
    if (size > Settings.MaxHistoryItems)
    {
        size = Settings.MaxHistoryItems;
    }
    for (int i=0; i < size; i++)
    {
        TaskNameCombo.AddString(Settings.History[i]);
    }
    Settings.History.clear();
}

void CMainDlg::SaveHistory()
{
    Settings.History.clear();
    for (int i=0; i < TaskNameCombo.GetCount(); i++)
    {
        CString Item = "";
        TaskNameCombo.GetLBText(i,Item);
        Settings.History.push_back(Item);
    }
}

void CMainDlg::OnContents(UINT wNotifyCode, INT wID, HWND hWndCtl)
{
    if (HtmlHelp(NULL,HelpFileName,HH_DISPLAY_TOPIC,NULL)==NULL)
    {
        CString ErrorMessage;
        ErrorMessage.Format("Help file \"%s\" was not found\nor your system does not support HTML help",HelpFileName);
        MyMessageBox(m_hWnd,ErrorMessage,szWinName,MB_ICONERROR);
    }
}

LRESULT CMainDlg::OnMyIconNotify(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // if at least 1 modal window is opened (Settings window, MessageBox), prohibit opening systray menu
    if (ModalState.IsModal()) return 0;

    switch (lParam)
    {
        POINT Point;
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONDOWN:
            if (IsWindowVisible())
            {
                OnHide(0,0,0);
            }
            else
            {
                OnTMSLauncherActivate(0,0,0);
            }
            break;
        case WM_RBUTTONDOWN:
            if (IsWindowVisible())
            {
                ModifyMenu(SystrayMenu,ID_SHOW_HIDE_WINDOW,MF_BYCOMMAND,ID_SHOW_HIDE_WINDOW,"Hide &window");
            }
            else
            {
                ModifyMenu(SystrayMenu,ID_SHOW_HIDE_WINDOW,MF_BYCOMMAND,ID_SHOW_HIDE_WINDOW,"Show &window");
            }
            GetCursorPos(&Point);
            SetForegroundWindow(m_hWnd);
            TrackPopupMenuEx(SystrayMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON,Point.x,Point.y,m_hWnd,NULL);
            break;
    }
    return 0;
}

void CMainDlg::OnShowHide(UINT wNotifyCode, INT wID, HWND hWndCtl)
{
    OnMyIconNotify((UINT)0, (WPARAM)0, (LPARAM)WM_LBUTTONDOWN);
}

LRESULT CMainDlg::OnLink(LPNMHDR pnmh)
{
    ENLINK *msg = (ENLINK*)pnmh;
    if ((msg->msg == WM_LBUTTONUP)&&(msg->wParam == MK_CONTROL))
    {
        if (msg->chrg.cpMax > msg->chrg.cpMin)
        {
            DWORD LinkLength = msg->chrg.cpMax - msg->chrg.cpMin + 1;
            CString String;
            CRichEditCtrl Temp = msg->nmhdr.hwndFrom;
            Temp.GetTextRange(msg->chrg.cpMin,msg->chrg.cpMax,String.GetBuffer(LinkLength));
            String.ReleaseBuffer();
            OpenLink(szWinName,m_hWnd,"open",String);
        }
    }
    return 0;
}

LRESULT CMainDlg::OnTaskbarCreated(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    DeleteSysTrayIcon(m_hWnd);
    CreateSysTrayIcon(m_hWnd,LoadIcon(_Module.GetModuleInstance(),MAKEINTRESOURCE(IDI_SYSTRAY_ICON)),szWinName);
    return 0;
}

LRESULT CMainDlg::OnActivateApp(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
    if (SetLayeredWindowAttributes != NULL)
    {
        if (wParam)
        {
            if (TimerIsSet != 0)
            {
                KillTimer(TrackMouseEventTimer);
                TimerIsSet = 0;
            }
            if (Settings.EnableOpacity) SetLayeredWindowAttributes(m_hWnd,RGB(0,0,0),(255*Settings.ActiveOpacity)/100,LWA_ALPHA);  // 0 - fully transparent; 255 - opaque
        }
        else
        {
            if (!CursorOverWindow())
            {
                if (Settings.EnableOpacity) SetLayeredWindowAttributes(m_hWnd,RGB(0,0,0),(255*Settings.InactiveOpacity)/100,LWA_ALPHA);
            }
            else
            {
                if (TimerIsSet == 0)
                {
                    TimerIsSet = SetTimer(TrackMouseEventTimer,TimerDelay);
                    if (Settings.EnableOpacity) SetLayeredWindowAttributes(m_hWnd,RGB(0,0,0),(255*Settings.ActiveOpacity)/100,LWA_ALPHA);  // 0 - fully transparent; 255 - opaque
                }
            }
        }
    }
    return 0;
}

LRESULT CMainDlg::OnTMSLauncherActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    if (Settings.SetFocusToTaskName)
    {
        switch(Settings.TaskNameControlType)
        {
            case 0: // rich edit control
                TaskNameControl.SetFocus();
                Settings.HighlightTaskName ? TaskNameControl.SetSelAll() : TaskNameControl.SetSelNone();
        	    break;
            case 1: // combobox control
                TaskNameCombo.SetFocus();
                Settings.HighlightTaskName ? TaskNameCombo.SetEditSel(0, -1) : TaskNameCombo.SetEditSel(-1, 0);
        	    break;
        }
    }
    ShowWindow(SW_SHOWNORMAL);
    if (!SetForegroundWindow(m_hWnd))
    {
        if (SetLayeredWindowAttributes != NULL)
        {
            if (Settings.EnableOpacity) SetLayeredWindowAttributes(m_hWnd,RGB(0,0,0),(255*Settings.InactiveOpacity)/100,LWA_ALPHA);
        }
    }
    else
    {
        if (SetLayeredWindowAttributes != NULL)
        {
            if (Settings.EnableOpacity) SetLayeredWindowAttributes(m_hWnd,RGB(0,0,0),(255*Settings.ActiveOpacity)/100,LWA_ALPHA);
        }
    }
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
    Settings.SaveGeneralSettings();
    
    SaveHistory(); // copies values from TaskNameCombo control to internal list
    Settings.SaveHistorySettings(); // saves values from internal list to Registry

    UnregisterHotKey(m_hWnd,Settings.GlobalHotkeyID);
    UINT HotkeyID = 0;
    for (unsigned int i=0; i<Settings.links.size(); i++)
    {
        UnregisterHotKey(m_hWnd,HotkeyID);
        HotkeyID++;
        UnregisterHotKey(m_hWnd,HotkeyID);
        HotkeyID++;
        UnregisterHotKey(m_hWnd,HotkeyID);
        HotkeyID++;
        UnregisterHotKey(m_hWnd,HotkeyID);
        HotkeyID++;
    }
    DeleteSysTrayIcon(m_hWnd);
    DestroyMenu(ContextMenu);
    DestroyMenu(ClipboardSubMenu);
    DestroyMenu(SettingsSubMenu);
    DestroyMenu(SystrayMenu);
    
    if (user32 != NULL) FreeLibrary(user32);
    
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

LRESULT CMainDlg::OnThemeChanged(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ((uMsg == WM_SETTINGCHANGE) && (wParam != SPI_SETNONCLIENTMETRICS))
    {
        return 0;
    }
    RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_ERASE|RDW_UPDATENOW|RDW_ALLCHILDREN);
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
        ::EnableWindow(GetDlgItem(VIEW_PARENT_TASK),FALSE);
        ::EnableWindow(GetDlgItem(VIEW_RELATED_TASKS),FALSE);
        ::EnableWindow(GetDlgItem(IDC_SETTINGS),FALSE);
    }
    else
    {
        WindowRect.bottom += ButtonRect.bottom - ButtonRect.top + 6;
        SetDlgItemText(IDC_EXPAND,"<<");
        ::EnableWindow(GetDlgItem(VIEW_CHILD_TASKS),TRUE);
        ::EnableWindow(GetDlgItem(VIEW_PARENT_TASK),TRUE);
        ::EnableWindow(GetDlgItem(VIEW_RELATED_TASKS),TRUE);
        ::EnableWindow(GetDlgItem(IDC_SETTINGS),TRUE);
    }
    SetWindowPos(HWND_TOPMOST,WindowRect.left,WindowRect.top,
                 WindowRect.right-WindowRect.left,WindowRect.bottom-WindowRect.top,0);
    Expanded = !Expanded;
}

bool CMainDlg::GetTaskNameFromRichEdit(CString &sTasks)
{
    UINT TaskNameLength;
    switch (Settings.TaskNameControlType)
    {
        case 0: // RichEdit control
            TaskNameLength = TaskNameControl.GetWindowTextLength();
            ::GetWindowText(TaskNameControl,sTasks.GetBuffer(TaskNameLength+1),TaskNameLength+1);
            break;
        case 1: // ComboBox control
            TaskNameLength = TaskNameCombo.GetWindowTextLength();
            ::GetWindowText(TaskNameCombo,sTasks.GetBuffer(TaskNameLength+1),TaskNameLength+1);
            break;
    }
    sTasks.ReleaseBuffer();
    return (!sTasks.IsEmpty());
}

bool CMainDlg::GetTaskNameFromClipboard(CString &sTasks)
{
    if (!::OpenClipboard(m_hWnd)) return false;
    if (IsClipboardFormatAvailable(CF_TEXT))
    {
        HANDLE ClipboardData = GetClipboardData(CF_TEXT);
        if (ClipboardData)
        {
            LPSTR TaskName = (LPSTR)GlobalLock(ClipboardData);
            sTasks = TaskName;
            GlobalUnlock(ClipboardData);
        }
    }
    CloseClipboard();
    return (!sTasks.IsEmpty());
}

BOOL CALLBACK AddToHistoryList(HWND hwnd, LPARAM lParam)
{
    CListBox HistoryList = ::GetDlgItem(hwnd,IDC_HISTORY_LIST);
    CButton ViewButton = ::GetDlgItem(hwnd,IDC_BUTTON_VIEW);
    CButton CopyButton = ::GetDlgItem(hwnd,IDC_BUTTON_COPY);
    CButton DeleteButton = ::GetDlgItem(hwnd,IDC_BUTTON_DELETE);
    CButton ClearButton = ::GetDlgItem(hwnd,IDC_BUTTON_CLEAR);
    if (HistoryList != NULL && DeleteButton != NULL && ClearButton != NULL && ViewButton != NULL && CopyButton != NULL)
    {
        int pos = HistoryList.FindStringExact(-1,(const char *)lParam);
        if (pos != CB_ERR)
        {
            HistoryList.DeleteString(pos);
        }
        else
        {
            UINT ItemsNumber = HistoryList.GetCount();
            if (ItemsNumber == Settings.MaxHistoryItems)
            {
                HistoryList.DeleteString(ItemsNumber - 1);
            }
        }
        HistoryList.InsertString(0,(const char *)lParam);
        HistoryList.SetSel(0);
        ViewButton.EnableWindow(true);
        CopyButton.EnableWindow(true);
        DeleteButton.EnableWindow(true);
        ClearButton.EnableWindow(true);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

void CMainDlg::AddToHistory(const char *item)
{
    if (Settings.MaxHistoryItems == 0)
    {
        return;
    }
    HWND SettingsWindow = NULL;
    SettingsWindow = FindWindow(NULL,"TMS Launcher settings");
    if (SettingsWindow != NULL)
    {
        EnumChildWindows(SettingsWindow,(WNDENUMPROC)AddToHistoryList,(LPARAM)item);
    }

    int pos = TaskNameCombo.FindStringExact(-1,item);
    if (pos != CB_ERR)
    {
        TaskNameCombo.DeleteString(pos);
    }
    else
    {
        UINT ItemsNumber = TaskNameCombo.GetCount();
        if (ItemsNumber == Settings.MaxHistoryItems)
        {
            TaskNameCombo.DeleteString(ItemsNumber - 1);
        }
    }

    TaskNameCombo.InsertString(0,item);
    SystrayMenu.EnableMenuItem(ID_CLEAR_HISTORY,MF_BYCOMMAND|MF_ENABLED);
}

void CMainDlg::OnViewTask(UINT wNotifyCode, INT wID, HWND hWndCtl)
{
    CString sTasks = "";
    CString sClientName = "";
    CString sIDName = "";
    CString Request = "";
    CString ErrorString = "Invalid task name format";

    static bool busy = false;
    if (busy) return;

    // called after VIEW_TASK, VIEW_CHILD_TASKS, VIEW_PARENT_TASK or VIEW_RELATED_TASKS button was pressed
    if ((wID == VIEW_TASK)||(wID == VIEW_CHILD_TASKS)||(wID == VIEW_PARENT_TASK)||(wID == VIEW_RELATED_TASKS)||(wNotifyCode==1))
    {
        if (!GetTaskNameFromRichEdit(sTasks))
        {
            MyMessageBox(m_hWnd,"\"Task Name\" field is empty",szWinName,MB_ICONERROR);
            return;
        }
    }
    // called after systray menu items VIEW_TASK_HOTKEY, VIEW_CHILD_TASKS_HOTKEY or VIEW_PARENT_TASK_HOTKEY 
    // are clicked or hoykey is pressed or IDC_BUTTON_VIEW was pressed on History page of Settings window
    else
    {
        // IDC_BUTTON_VIEW was pressed on History page of Settings window
        if (wID == IDC_BUTTON_VIEW)
        {
            sTasks = Settings.HistoryTasks;
            wID = VIEW_TASK;
        }
        else
        {
            if (!GetTaskNameFromClipboard(sTasks))
            {
                MyMessageBox(m_hWnd,"Clipboard does not contain data in text format",szWinName,MB_ICONERROR);
                return;
            }
        }
        switch (Settings.TaskNameControlType)
        {
            case 0: // RichEdit control
                TaskNameControl.SetWindowText(sTasks);
                break;
            case 1: // ComboBox control
                TaskNameCombo.SetWindowText(sTasks);
                break;
        }
    }

    busy = true;

    std::vector<TASKNAME> Tasks;
    std::vector<TASKNAME> Defects;
    TASK task;
    int items = 0;
    bool correct = task.ComplexParseTasks(sTasks, Tasks, Defects, items);
    bool InSoftTest = Settings.OpenDefectsInSoftTest(wID);

    for (unsigned int i = 0; i < Tasks.size(); i++)
    {
        // skip opening defects if they need to be opened in SoftTest
        if (InSoftTest && Settings.IsDefect(Tasks[i].Client, NULL, NULL))
        {
            AddToHistory(Tasks[i].Client+Tasks[i].Separator+Tasks[i].ID);
        }
        else
        {
            CreateRequest(Tasks[i].Client, Tasks[i].ID, Request, wID);
            if (!Request.IsEmpty())
            {
                OpenTask(Request, (Tasks.size() == 1));
                AddToHistory(Tasks[i].Client+Tasks[i].Separator+Tasks[i].ID);
            }
        }
    }

    // open defects in SoftTest
    if (InSoftTest)
    {
        OpenDefects(Defects, wID);
    }

    if (Settings.TaskNameControlType == 1)
    {
        TaskNameCombo.SetWindowText(sTasks);
    }

    switch(items)
    {
        case 0:
            if (!sTasks.IsEmpty())
            {
                MyMessageBox(m_hWnd,ErrorString,szWinName,MB_ICONERROR);
            }
    	    break;
        case 1:
            if (!correct)
            {
                MyMessageBox(m_hWnd,ErrorString,szWinName,MB_ICONERROR);
            }
    	    break;
        default:
            if (!correct)
            {
                MyMessageBox(m_hWnd,"Some tasks were not opened because of incorrect format",szWinName,MB_ICONWARNING);
            }
    	    break;
    }

    busy = false;
    Tasks.clear();
    Defects.clear();
}

void CMainDlg::Replace_AA_ID(CString &Request, CString &Message, int index)
{
    if (Request.Find("%AA_ID%") == -1) return;

    if (Settings.links[index].Login.IsEmpty() || Settings.links[index].Password.IsEmpty())
    {
        MyMessageBox(m_hWnd,"Both Login and Password must be specified\nin settings window for \""+Settings.links[index].Caption+"\" URL\nto open task with AA_ID parameter in URL",szWinName,MB_ICONERROR);
        Request = "";
        return;
    }
    CAmHttpSocket Req;
    CString reply = Req.GetHeaders(Message);
    if (Req.GetPageStatusCode() == 401) // Authorization Required
    {
        CAmHttpSocket::InsertLoginPassword(Message,Settings.links[index].Login,Settings.links[index].Password);
        reply = Req.GetHeaders(Message);
    }
    Message = Req.GetPage(Message);
    TASK task;
    task.ParseHTMLForAA_ID(Message,Message);
    if (Message.IsEmpty())
    {
        MyMessageBox(m_hWnd,"Error while getting AA_ID parameter",szWinName,MB_ICONERROR);
        Request = "";
        return;
    }
    else
    {
        Request.Replace("%AA_ID%",Message);
    }
}

void CMainDlg::CreateRequest(const char *sClientName, const char *sIDName, CString &Request, INT wID)
{
    int index = -1;

    Request = "";

    // opening defect
    if (Settings.IsDefect(sClientName, NULL, &index) || Settings.IsSIF(index) || Settings.IsHF(index))
    {
        switch (wID)
        {
            case VIEW_TASK:
            case VIEW_TASK_HOTKEY:
                Request = Settings.defects[index].DefectURL;
                break;
            case VIEW_CHILD_TASKS:
            case VIEW_CHILD_TASKS_HOTKEY:
                Request = Settings.defects[index].ChildDefectsURL;
                break;
            case VIEW_PARENT_TASK:
            case VIEW_PARENT_TASK_HOTKEY:
                Request = Settings.defects[index].ParentDefectURL;
                break;
            case VIEW_RELATED_TASKS:
            case VIEW_RELATED_TASKS_HOTKEY:
                Request = Settings.defects[index].RelatedDefectsURL;
                break;
            default:
                switch (wID % 4)
                {
                    case 0: // open task
                        Request = Settings.defects[index].DefectURL;
                        break;
                    case 1: // open child tasks
                        Request = Settings.defects[index].ChildDefectsURL;
                        break;
                    case 2: // open parent task
                        Request = Settings.defects[index].ParentDefectURL;
                        break;
                    case 3: // open related tasks
                        Request = Settings.defects[index].RelatedDefectsURL;
                        break;
                }
        }
        Request.Replace("%PROJECT%",Settings.defects[index].STProject);
        Request.Replace("%ID%",sIDName);
        return;
    }

    // opening TMS task
    if (lstrcmp(sClientName,"")==0)
    {
        MyMessageBox(m_hWnd,"Task without client name is entered.\nThere is no defect with empty client name defined.\nSo, the task cannot be opened.",szWinName,MB_ICONERROR);
        return;
    }

    CString tempID = sIDName;
    if (Settings.FillID) TASK::FillupTaskID(tempID);

    switch (wID)
    {
        case VIEW_TASK:
        case VIEW_TASK_HOTKEY:
        case VIEW_CHILD_TASKS:
        case VIEW_CHILD_TASKS_HOTKEY:
        case VIEW_PARENT_TASK:
        case VIEW_PARENT_TASK_HOTKEY:
        case VIEW_RELATED_TASKS:
        case VIEW_RELATED_TASKS_HOTKEY:
            // use default URL to open task/child/parent/related task(s)
            index = Settings.GetDefaultUrlIndex();
            break;
        default:
            // use hotkey ID's dependent URL (wID == HotKeyID)
            index = wID / 4;
            switch (wID % 4)
            {
                case 0: // open task
                    wID = VIEW_TASK_HOTKEY;
                    break;
                case 1: // open child tasks
                    wID = VIEW_CHILD_TASKS_HOTKEY;
                    break;
                case 2: // open parent task
                    wID = VIEW_PARENT_TASK_HOTKEY;
                    break;
                case 3: // open related tasks
                    wID = VIEW_RELATED_TASKS_HOTKEY;
                    break;
            }
    }

    switch (wID)
    {
        case VIEW_PARENT_TASK:
        case VIEW_PARENT_TASK_HOTKEY:
        {
            if (Settings.links[index].Login.IsEmpty() || Settings.links[index].Password.IsEmpty())
            {
                MyMessageBox(m_hWnd,"Both Login and Password must be specified\nin settings window for \""+Settings.links[index].Caption+"\" URL\nto open parent task",szWinName,MB_ICONERROR);
                Request = "";
                break;
            }
            CString Message = "";
            CAmHttpSocket Req;
            Request.Format(Settings.iTMSviewTask, sClientName, tempID);
            CString reply = Req.GetHeaders(Request);
            if (Req.GetPageStatusCode() == 401) // Authorization Required
            {
                CAmHttpSocket::InsertLoginPassword(Request,Settings.links[index].Login,Settings.links[index].Password);
                reply = Req.GetHeaders(Request);
            }
            Message = Req.GetPage(Request);
            if (Message.IsEmpty() || (Message.Find("You don't have access to this site") != -1))
            {
                MyMessageBox(m_hWnd,"Error while opening parent task",szWinName,MB_ICONERROR);
                Request = "";
                break;
            }
            Request = CString(sClientName)+"-"+tempID;
            TASK task;
            switch (task.ParseHTMLForParentTask(Request, Message))
            {
                case 0:
                    break;
                case 1:
                case 2:
                    MyMessageBox(m_hWnd,Request,szWinName,MB_ICONERROR);
                    Request = "";
                    return;
            }
        }
        case VIEW_TASK:
        case VIEW_TASK_HOTKEY:
            if (!Settings.links[index].TaskURL.IsEmpty())
            {
                CString Message = "";
                if (!Request.IsEmpty())
                {
                    CString Client, ID;
                    int pos = Request.Find('-');
                    Client = Request.Left(pos);
                    ID = Request.Right(Request.GetLength()-pos-1);
                    Request = Settings.links[index].TaskURL;
                    Request.Replace("%CLIENT%",Client);
                    Request.Replace("%ID%",ID);
                    Message.Format(Settings.iTMSviewTask,Client,ID);
                }
                else
                {
                    Request = Settings.links[index].TaskURL;
                    Request.Replace("%CLIENT%",sClientName);
                    Request.Replace("%ID%",tempID);
                    Message.Format(Settings.iTMSviewTask,sClientName,tempID);
                }
                Replace_AA_ID(Request, Message, index);
            }
            else
            {
                MyMessageBox(m_hWnd,"URL to open Task is not defined.\nPlease correct settings on URLs page",szWinName,MB_ICONERROR);
            }
            break;
        case VIEW_CHILD_TASKS:
        case VIEW_CHILD_TASKS_HOTKEY:
            if (!Settings.links[index].ChildTasksURL.IsEmpty())
            {
                Request = Settings.links[index].ChildTasksURL;
                Request.Replace("%CLIENT%",sClientName);
                Request.Replace("%ID%",tempID);
                CString Message = "";
                Message.Format(Settings.iTMSviewTask,sClientName,tempID);
                Replace_AA_ID(Request, Message, index);
            }
            else
            {
                MyMessageBox(m_hWnd,"URL to open Child Tasks is not defined.\nPlease correct settings on URLs page",szWinName,MB_ICONERROR);
            }
            break;
        case VIEW_RELATED_TASKS:
        case VIEW_RELATED_TASKS_HOTKEY:
            if (!Settings.links[index].RelatedTasksURL.IsEmpty())
            {
                Request = Settings.links[index].RelatedTasksURL;
                Request.Replace("%CLIENT%",sClientName);
                Request.Replace("%ID%",tempID);
                CString Message = "";
                Message.Format(Settings.iTMSviewTask,sClientName,tempID);
                Replace_AA_ID(Request, Message, index);
            }
            else
            {
                MyMessageBox(m_hWnd,"URL to open Related Tasks is not defined.\nPlease correct settings on URLs page.\nAt least you can use the same URL as for Child tasks",szWinName,MB_ICONERROR);
            }
            break;
    }

    if (!Request.IsEmpty())
    {
        if (!Settings.links[index].Login.IsEmpty() && !Settings.links[index].Password.IsEmpty())
        {
            CAmHttpSocket::InsertLoginPassword(Request,Settings.links[index].Login,Settings.links[index].Password);
        }
        Request.Insert(0,"\"");
        Request += "\"";
    }
}

void CMainDlg::OpenTask(const char *Request, bool SingleTask)
{
    DWORD Result = 0;
    if (Settings.DefaultBrowser)
    {
        if (SingleTask)
        {
            Result = OpenLink(szWinName,m_hWnd,"open",Request);
        }
        else
        {
            if (Settings.BrowserPath.IsEmpty())
            {
                Result = OpenLink(szWinName,m_hWnd,"open",Request);
            }
            else
            {
                Result = OpenLink(szWinName,m_hWnd,"open",Settings.BrowserPath,Request);
            }
        }
    }
    else
    {
        if (Settings.BrowserPath.IsEmpty())
        {
            Result = OpenLink(szWinName,m_hWnd,"open",Request);
        }
        else
        {
            Result = OpenLink(szWinName,m_hWnd,"open",Settings.BrowserPath,Request);
        }
    }
    if ((Result == ERROR_FILE_NOT_FOUND) || (Result == CO_E_APPNOTFOUND))
    {
        MyMessageBox(m_hWnd,"System could not find browser you are using to open task(s).\n\nPlease make sure default browser is correctly defined in your system\nor specify \"Path to browser\" explicitly on General page of Settings window",
                     szWinName, MB_OK | MB_ICONERROR);
    }
}

void CMainDlg::OpenDefects(const std::vector<TASKNAME> &Defects, INT wID)
{
    if (Defects.size() == 0) return;

    unsigned int start = 0;
    CString DEFECTS = Defects[start].ID;

    unsigned int i = 1;
    while (i < Defects.size())
    {
        if (Defects[start].Client.CompareNoCase(Defects[i].Client)==0)
        {
            DEFECTS += ", " + Defects[i].ID;
            i++;
        }
        else
        {
            OpenDefectsInOneProject(Defects[start].Client,DEFECTS,wID);
            start = i;
            DEFECTS = Defects[start].ID;
            i++;
        }
    }
    OpenDefectsInOneProject(Defects[start].Client,DEFECTS,wID);
}

void CMainDlg::OpenDefectsInOneProject(const char *Project, const char *IDs, INT wID)
{
    CString Filter = "";

    switch (wID)
    {
        case VIEW_TASK:
        case VIEW_TASK_HOTKEY:
            Filter = Settings.DefectFilter;
            break;
        case VIEW_CHILD_TASKS:
        case VIEW_CHILD_TASKS_HOTKEY:
            Filter = Settings.ChildDefectsFilter;
            break;
        case VIEW_PARENT_TASK:
        case VIEW_PARENT_TASK_HOTKEY:
            Filter = Settings.ParentDefectFilter;
            break;
        case VIEW_RELATED_TASKS:
        case VIEW_RELATED_TASKS_HOTKEY:
            Filter = Settings.RelatedDefectsFilter;
            break;
        default:
            switch (wID % 4)
            {
                case 0: // open defect
                    Filter = Settings.DefectFilter;
                    break;
                case 1: // open child defect
                    Filter = Settings.ChildDefectsFilter;
                    break;
                case 2: // open parent defect
                    Filter = Settings.ParentDefectFilter;
                    break;
                case 3: // open related defects
                    Filter = Settings.RelatedDefectsFilter;
                    break;
            }
    }
    Filter.Replace("%ID%",IDs);

    ofstream OutFile;
    CString OutputFileName = Settings.SoftTestFiltersPath+Settings.SoftTestFilterName;
    OutputFileName.Replace("%PROJECT%",Project);
    
    OutFile.open(OutputFileName, ios::out);
    if (OutFile.fail())
    {
        CString Message = "";
        Message.Format("Cannot create/re-write SoftTest filter \"%s\".\n\nDirectory does not exist or disk is write protected.\nPlease check if SoftTest is installed correctly", OutputFileName);
        MyMessageBox(m_hWnd,Message, szWinName, MB_OK | MB_ICONERROR);
        return;
    }
    OutFile << Filter;
    OutFile.close();

    if (ERROR_FILE_NOT_FOUND == OpenLink(szWinName,m_hWnd,"open",Settings.SoftTestPath,Settings.GetSoftTestCommandLine(Project)))
    {
        MyMessageBox(m_hWnd,"SoftTest executable file was not found.\n\nPlease make sure that SoftTest is installed\nand \"Path to SoftTest\" is defined properly in Settings window", szWinName, MB_OK | MB_ICONERROR);
    }
}

LRESULT CMainDlg::OnHotKey(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/)
{
    // if at least 1 modal window is opened (Settings window, MessageBox), prohibit using hotkeys
    if (ModalState.IsModal()) return 0;

    if (wParam == Settings.GlobalHotkeyID)
    {
        OnTMSLauncherActivate(0,0,0);
    }
    else
    {
        OnViewTask(0,wParam,0);
    }
    return 0;
}

void CMainDlg::OnSettings(UINT wNotifyCode, INT wID, HWND hWndCtl)
{
    COptionSheetDialogImpl<COptionSelectionTreeCtrl, CMyPropSheet> Sheet(IDD_MYOPTIONSHEET);
    Sheet.SetTitle("TMS Launcher settings");

    ModalState.SetModal();
    SystrayMenu.EnableMenuItem(8,MF_BYPOSITION|MF_GRAYED);

    int OldControlType = Settings.TaskNameControlType;
    
    switch (wID)
    {
        case IDC_SETTINGS:
            Sheet.DoModal(::GetActiveWindow(),NULL);
            break;
        case ID_SETTINGS_GENERAL:
            Sheet.DoModal(::GetActiveWindow(),0);
            break;
        case ID_SETTINGS_URLS:
            Sheet.DoModal(::GetActiveWindow(),1);
            break;
        case ID_SETTINGS_DEFECTS:
            Sheet.DoModal(::GetActiveWindow(),2);
            break;
        case ID_SETTINGS_FORMAT:
            Sheet.DoModal(::GetActiveWindow(),3);
            break;
        case ID_SETTINGS_SOFTTEST:
            Sheet.DoModal(::GetActiveWindow(),4);
            break;
        case ID_SETTINGS_HISTORY:
            Sheet.DoModal(::GetActiveWindow(),5);
            break;
        case ID_SETTINGS_OTHER:
            Sheet.DoModal(::GetActiveWindow(),6);
            break;
        default:
            Sheet.DoModal(::GetActiveWindow(),NULL);
    }

    ModalState.UnsetModal();

    if (OldControlType != Settings.TaskNameControlType)
    {
        SwitchControls();
    }

    SystrayMenu.EnableMenuItem(8,MF_BYPOSITION|MF_ENABLED);
    if (TaskNameCombo.GetCount() == 0)
    {
        SystrayMenu.EnableMenuItem(ID_CLEAR_HISTORY,MF_BYCOMMAND|MF_GRAYED);
    }
    else
    {
        SystrayMenu.EnableMenuItem(ID_CLEAR_HISTORY,MF_BYCOMMAND|MF_ENABLED);
    }

    if (Settings.EnableOpacity)
    {
        if (user32 != NULL)
        {
            if (SetLayeredWindowAttributes == NULL)
            {
                SetLayeredWindowAttributes = (MYPROC)GetProcAddress(user32, "SetLayeredWindowAttributes");
                if (SetLayeredWindowAttributes != NULL)
                {
                    SetWindowLong(GWL_EXSTYLE,GetWindowLong(GWL_EXSTYLE) | WS_EX_LAYERED);
                    SetLayeredWindowAttributes(m_hWnd,RGB(0,0,0),(255*Settings.ActiveOpacity)/100,LWA_ALPHA);  // 0 - fully transparent; 255 - opaque
                }
            }
            else
            {
                SetLayeredWindowAttributes(m_hWnd,RGB(0,0,0),(255*Settings.ActiveOpacity)/100,LWA_ALPHA);  // 0 - fully transparent; 255 - opaque
            }
        }
    }
    else
    {
        if (SetLayeredWindowAttributes != NULL)
        {
            SetWindowLong(GWL_EXSTYLE,GetWindowLong(GWL_EXSTYLE) & ~WS_EX_LAYERED);
            RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW| RDW_ALLCHILDREN);
            SetLayeredWindowAttributes = NULL;
        }
    }
}

BOOL CALLBACK ClearHistoryList(HWND hwnd, LPARAM lParam)
{
    CListBox HistoryList = ::GetDlgItem(hwnd,IDC_HISTORY_LIST);
    CButton ViewButton = ::GetDlgItem(hwnd,IDC_BUTTON_VIEW);
    CButton CopyButton = ::GetDlgItem(hwnd,IDC_BUTTON_COPY);
    CButton DeleteButton = ::GetDlgItem(hwnd,IDC_BUTTON_DELETE);
    CButton ClearButton = ::GetDlgItem(hwnd,IDC_BUTTON_CLEAR);
    if (HistoryList != NULL && DeleteButton != NULL && ClearButton != NULL && ViewButton != NULL && CopyButton != NULL)
    {
        HistoryList.ResetContent();
        ViewButton.EnableWindow(false);
        CopyButton.EnableWindow(false);
        DeleteButton.EnableWindow(false);
        ClearButton.EnableWindow(false);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

void CMainDlg::OnClearHistory(UINT wNotifyCode, INT wID, HWND hWndCtl)
{
    HWND SettingsWindow = NULL;
    SettingsWindow = FindWindow(NULL,"TMS Launcher settings");
    if (SettingsWindow != NULL)
    {
        EnumChildWindows(SettingsWindow,(WNDENUMPROC)ClearHistoryList,(LPARAM)0);
    }
    CString Task = "";
    if (Settings.TaskNameControlType == 1) // ComboBox control
    {
        UINT TaskNameLength = TaskNameCombo.GetWindowTextLength();
        if (TaskNameLength > 0)
        {
            ::GetWindowText(TaskNameCombo,Task.GetBuffer(TaskNameLength+1),TaskNameLength+1);
            Task.ReleaseBuffer();
        }
    }
    TaskNameCombo.ResetContent();
    if (!Task.IsEmpty())
    {
        TaskNameCombo.SetWindowText(Task);
    }
    SystrayMenu.EnableMenuItem(ID_CLEAR_HISTORY,MF_BYCOMMAND|MF_GRAYED);
}

void CMainDlg::SwitchControls()
{
    UINT TaskNameLength = 0;
    CString Task = "";
    switch(Settings.TaskNameControlType)
    {
        case 0: // RichEdit control
            TaskNameLength = TaskNameCombo.GetWindowTextLength();
            if (TaskNameLength > 0)
            {
                ::GetWindowText(TaskNameCombo,Task.GetBuffer(TaskNameLength+1),TaskNameLength+1);
                Task.ReleaseBuffer();
                TaskNameControl.SetWindowText(Task);
            }
            else
            {
                TaskNameControl.SetWindowText("");
            }
            TaskNameControl.EnableWindow(TRUE);
            TaskNameControl.ShowWindow(SW_SHOW);
            TaskNameCombo.EnableWindow(FALSE);
            TaskNameCombo.ShowWindow(SW_HIDE);
        	break;
        case 1: // ComboBox control
            TaskNameLength = TaskNameControl.GetWindowTextLength();
            if (TaskNameLength > 0)
            {
                ::GetWindowText(TaskNameControl,Task.GetBuffer(TaskNameLength+1),TaskNameLength+1);
                Task.ReleaseBuffer();
                TaskNameCombo.SetWindowText(Task);
            }
            else
            {
                TaskNameCombo.SetWindowText("");
            }
            TaskNameCombo.EnableWindow(TRUE);
            TaskNameCombo.ShowWindow(SW_SHOW);
            TaskNameControl.EnableWindow(FALSE);
            TaskNameControl.ShowWindow(SW_HIDE);
        	break;
    }
}

LRESULT CMainDlg::OnMsgFilter(LPNMHDR pnmh)
{
    MSGFILTER *msg = (MSGFILTER*)pnmh;
    if (msg->nmhdr.hwndFrom == GetDlgItem(IDC_TASKNAME))
    {
        if (msg->msg == WM_LBUTTONUP)
        {
            if (Settings.SingleClick)
            {
                switch (Settings.TaskNameControlType)
                {
                    case 0: // RichEdit control
                        TaskNameControl.SetSelAll();
                        break;
                }
            }
        }
    }
    return 0;
}

void CMainDlg::OnContextMenu(HWND hwndFrom, CPoint CursorPos)
{
    if (hwndFrom == TaskNameControl)
    {
        switch (Settings.RightClickAction)
        {
            case 0:
                OnViewTask(0,VIEW_TASK_HOTKEY,0);
                break;
            case 1:
                OnViewTask(0,VIEW_CHILD_TASKS_HOTKEY,0);
                break;
            case 2:
                OnViewTask(0,VIEW_PARENT_TASK_HOTKEY,0);
                break;
            case 3:
                OnViewTask(0,VIEW_RELATED_TASKS_HOTKEY,0);
                break;
            case 4:
                RECT Rect;
                CWindow ChildWindow = hwndFrom;
                ChildWindow.GetWindowRect(&Rect);
                if ((CursorPos.x == -1)||(CursorPos.y == -1)) // WM_CONTEXTMENU was generated by VK_APPS key
                {
                    CursorPos.x = Rect.left;
                    CursorPos.y = Rect.top;
                }
                else // WM_CONTEXTMENU was generated by right mouse button click
                {
                    // check if cursor position is outside the window
                    if ((CursorPos.x < Rect.left)||(CursorPos.x > Rect.right)||
                        (CursorPos.y < Rect.top)||(CursorPos.y > Rect.bottom))
                        return; // do not show context menu in this case
                }
                CString SelectedText;
                TaskNameControl.GetSelText(SelectedText);
                ContextMenu.EnableMenuItem(ID_MENU_UNDO,TaskNameControl.CanUndo()?MF_ENABLED:MF_GRAYED);
                ContextMenu.EnableMenuItem(ID_MENU_PASTE,TaskNameControl.CanPaste()?MF_ENABLED:MF_GRAYED);
                ContextMenu.EnableMenuItem(ID_MENU_SELECT,TaskNameControl.GetWindowTextLength()?MF_ENABLED:MF_GRAYED);
                ContextMenu.EnableMenuItem(ID_MENU_CUT,SelectedText.IsEmpty()?MF_GRAYED:MF_ENABLED);
                ContextMenu.EnableMenuItem(ID_MENU_COPY,SelectedText.IsEmpty()?MF_GRAYED:MF_ENABLED);
                ContextMenu.EnableMenuItem(ID_MENU_DELETE,SelectedText.IsEmpty()?MF_GRAYED:MF_ENABLED);
                TrackPopupMenuEx(ContextMenu,TPM_LEFTBUTTON|TPM_LEFTALIGN|TPM_TOPALIGN,CursorPos.x,CursorPos.y,m_hWnd,NULL);
                break;
        }
    }
    else
    {
        if (hwndFrom == TaskNameCombo)
        {
            switch (Settings.RightClickAction2)
            {
                case 0:
                    OnViewTask(0,VIEW_TASK_HOTKEY,0);
                    break;
                case 1:
                    OnViewTask(0,VIEW_CHILD_TASKS_HOTKEY,0);
                    break;
                case 2:
                    OnViewTask(0,VIEW_PARENT_TASK_HOTKEY,0);
                    break;
                case 3:
                    OnViewTask(0,VIEW_RELATED_TASKS_HOTKEY,0);
                    break;
            }
        }
        else  // processing context menus for main window buttons
        {
            CMenu ButtonMenu;
            ButtonMenu = CreatePopupMenu();
            UINT Command = 0;

            if ((hwndFrom == GetDlgItem(VIEW_TASK)) || (hwndFrom == GetDlgItem(VIEW_CHILD_TASKS)) ||
                (hwndFrom == GetDlgItem(VIEW_PARENT_TASK)) || (hwndFrom == GetDlgItem(VIEW_RELATED_TASKS)))
            {
                for (unsigned int i=0; i < Settings.links.size(); i++)
                {
                    ButtonMenu.AppendMenu(MF_ENABLED,i+1,Settings.links[i].Caption);
                    if (Settings.links[i].Default)
                    {
                        ButtonMenu.SetMenuDefaultItem(i+1);
                    }
                }
                RECT Rect;
                ::GetWindowRect(hwndFrom,&Rect);
                Command = TrackPopupMenuEx(ButtonMenu,TPM_RETURNCMD|TPM_LEFTBUTTON|TPM_LEFTALIGN|TPM_TOPALIGN,Rect.left,Rect.bottom,m_hWnd,NULL);
            }
            if (Command != 0)
            {
                if (hwndFrom == GetDlgItem(VIEW_TASK))
                {
                    OnViewTask(1,(Command-1)*4,0);
                }
                if (hwndFrom == GetDlgItem(VIEW_CHILD_TASKS))
                {
                    OnViewTask(1,(Command-1)*4+1,0);
                }
                if (hwndFrom == GetDlgItem(VIEW_PARENT_TASK))
                {
                    OnViewTask(1,(Command-1)*4+2,0);
                }
                if (hwndFrom == GetDlgItem(VIEW_RELATED_TASKS))
                {
                    OnViewTask(1,(Command-1)*4+3,0);
                }
            }
            DestroyMenu(ButtonMenu);
        }
    }
}

void CMainDlg::OnContextMenuCommand(UINT code, int idFrom, HWND hwndFrom)
{
    switch(idFrom)
    {
        case ID_MENU_UNDO:
            TaskNameControl.Undo();
            ::SetFocus(TaskNameControl);
            break;
        case ID_MENU_CUT:
            TaskNameControl.Cut();
            ::SetFocus(TaskNameControl);
            break;
        case ID_MENU_COPY:
            TaskNameControl.Copy();
            ::SetFocus(TaskNameControl);
            break;
        case ID_MENU_PASTE:
            TaskNameControl.Paste();
            ::SetFocus(TaskNameControl);
            break;
        case ID_MENU_DELETE:
            TaskNameControl.Clear();
            ::SetFocus(TaskNameControl);
            break;
        case ID_MENU_SELECT:
            TaskNameControl.SetSelAll();
            ::SetFocus(TaskNameControl);
            break;
    }
}

bool isalpha_cp1251(unsigned char ch)
{
    if ((ch >= (unsigned char)0xC0) && (ch <= (unsigned char)0xFF))  // cyrilic characters in cp1251
    {
        return true;
    }
    else
    {
        return (isalpha(ch) != 0);
    }
}

int CompareNoCaseCP1251(const char *string1, const char *string2)
{
    CString str1 = string1;
    StringToUpperCase(str1);
    CString str2 = string2;
    StringToUpperCase(str2);
    return StrCmp(str1, str2);
}

// This function converts lowercase characters to uppercase ones in passed string
// For russian characters it works correctly with Cyrilic (Windows 1251) code page only
void StringToUpperCase(CString &String)
{
    for (INT i=0; i<lstrlen(String); i++)
    {
        if ((String[i] >= (char)0x61) && (String[i] <= (char)0x7A))   // small latin characters
            String.SetAt(i,String[i]-(char)0x20);
        else
            if ((String[i] >= (char)0xE0) && (String[i] <= (char)0xFF)) // small cyrilic characters
                String.SetAt(i,String[i]-(char)0x20);
        else
            switch((char)String[i])
            {
                case (char)0xB3: // small ukrainian "i"
                    String.SetAt(i,(char)0xB2);
                    break;
                case (char)0xB4: // small ukrainian "_"
                    String.SetAt(i,(char)0xA5);
                    break;
                case (char)0xB8: // small russian "¸"
                    String.SetAt(i,(char)0xA8);
                    break;
                case (char)0xBA: // small russian "º"
                    String.SetAt(i,(char)0xAA);
                    break;
                case (char)0xBF: // small russian "¿"
                    String.SetAt(i,(char)0xAF);
                    break;
            }
    }
}
