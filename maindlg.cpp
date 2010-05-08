// maindlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: maindlg.cpp,v $$Revision: 1.28 $$Date: 2005/07/07 11:09:32Z $"; 
#endif

#include "resource.h"
#include "maindlg.h"
#include "Systray.h"
#include "settings.h"
#include "tools.h"
#include <ctype.h>

#include "Options.h"
using Mortimer::COptionSheetDialogImpl;
using Mortimer::COptionSelectionTreeCtrl;

extern CString szWinName;
extern CSettings Settings;

bool isalpha_cp1251(char ch);
int CompareNoCaseCP1251(const char *string1, const char *string2);
void StringToUpperCase(CString &String);

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    UINT HotkeyID = 0;
    for (int i=0; i<Settings.links.size(); i++)
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
                MessageBox(message,szWinName,MB_ICONERROR);
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
                MessageBox(message,szWinName,MB_ICONERROR);
            }
        }
        HotkeyID++;
    }

    SetWindowText(szWinName);
    
    TaskNameControl.Attach(GetDlgItem(IDC_TASKNAME));
    TaskNameControl.SetTextMode(TM_PLAINTEXT);
    TaskNameControl.SetEventMask(ENM_MOUSEEVENTS|ENM_LINK);
    TaskNameControl.SetAutoURLDetect(true);

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
            if (!OpenLink(String))
            {
                MessageBox("The link is not accessible or incorrect",szWinName,MB_ICONERROR);
            }
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
    Settings.SaveGeneralSettings();

    DeleteSysTrayIcon(m_hWnd);
    DestroyMenu(SystraySubMenu);
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

bool CMainDlg::IsTaskNameValid(CString &sTaskName, CString &sClientName, CString &sIDName)
{
//  Task name format: [%CLIENT%-]%ID%[-%EXT%]

    if (sTaskName.IsEmpty()) return false;

    sTaskName.TrimLeft();
    sTaskName.TrimRight();
    if ((sTaskName.GetLength()<Settings.MinTaskName)||
        (sTaskName.GetLength()>Settings.MaxTaskName)) return false;

    for (int i=1; i<Settings.Separators.GetLength(); i++)
    {
        sTaskName.Replace(Settings.Separators[i],Settings.Separators[0]);
    }

    int pos = sTaskName.Find(Settings.Separators[0],0);

    // parsing task name
    if (pos > -1)
    {
        sClientName = sTaskName.Left(pos);
        sIDName = sTaskName.Right(sTaskName.GetLength()-pos-1);
        sIDName.TrimLeft(Settings.Separators[0]);
        pos = sIDName.Find(Settings.Separators[0],0);
        if (pos > -1)
        {
            int ext_len = sIDName.GetLength()-pos-1;
            if ((ext_len < Settings.MinExt) || (ext_len > Settings.MaxExt))
            {
                return false;
            }
            else
            {
                sIDName.Delete(pos,ext_len+1);
            }
        }
    }
    else // there are no separators in the task name
        if (sTaskName.GetLength()<=Settings.MaxIDName)
        {
            sClientName = "";
            sIDName = sTaskName;
        }
        else // wrong task name format
        {
            return false;
        }

    if ((sClientName.GetLength() < Settings.MinClientName) || 
        (sClientName.GetLength() > Settings.MaxClientName))
    {
        return false;
    }
    else
        if (!sClientName.IsEmpty())
        {
            // checking for correct Client name
            for (i=0; i<sClientName.GetLength()-1; i++)
            {
                if (!isalpha_cp1251(sClientName[i])) return false;
            }
            // the last symbol in Client Name can be either an alpha character or a number (i.e. QARD3)
            if ((!isalpha_cp1251(sClientName[sClientName.GetLength()-1])) && (!isdigit(sClientName[sClientName.GetLength()-1])))
            {
                return false;
            }
        }

    if ((sIDName.GetLength() < Settings.MinIDName) || (sIDName.GetLength() > Settings.MaxIDName))
    {
        return false;
    }
    else
    {
        // checking for correct ID
        for (i=0; i<sIDName.GetLength(); i++)
        {
            if (!isdigit(sIDName[i])) return false;
        }
    }

    return true;
}

bool CMainDlg::GetTaskNameFromRichEdit(CString &sTasks)
{
    UINT TaskNameLength = TaskNameControl.GetWindowTextLength();
    ::GetWindowText(TaskNameControl,sTasks.GetBuffer(TaskNameLength+1),TaskNameLength+1);
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

void CMainDlg::ParseTasks(CString &sTasks, std::vector<CString> &Tasks)
{
    Tasks.clear();
    for (int i=1; i<Settings.TasksSeparators.GetLength(); i++)
    {
        sTasks.Replace(Settings.TasksSeparators[i],Settings.TasksSeparators[0]);
    }
    int pos = -1;
    while ((pos = sTasks.Find(Settings.TasksSeparators[0],0)) != -1)
    {
        CString temp = sTasks.Left(pos);
        Tasks.push_back(temp);
        sTasks.Delete(0,pos);
        sTasks.TrimLeft(Settings.TasksSeparators[0]);
    }
    if (!sTasks.IsEmpty())
    {
        Tasks.push_back(sTasks);
    }
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

    // called after VIEW_TASK or VIEW_CHILD_TASKS button was pressed
    if ((wID == VIEW_TASK)||(wID == VIEW_CHILD_TASKS))
    {
        if (!GetTaskNameFromRichEdit(sTasks))
        {
            ShowModal = true;
            MessageBox("\"Task Name\" field is empty",szWinName,MB_ICONERROR);
            ShowModal = false;
            return;
        }
    }
    // called after systray menu items VIEW_TASK_HOTKEY or VIEW_CHILD_TASKS_HOTKEY are clicked
    // or hoykey is pressed
    else
    {
        if (!GetTaskNameFromClipboard(sTasks))
        {
            ShowModal = true;
            MessageBox("Clipboard does not contain data in text format",szWinName,MB_ICONERROR);
            ShowModal = false;
            return;
        }
        else TaskNameControl.SetWindowText(sTasks);
    }

    std::vector<CString> Tasks;
    busy = true;
    ParseTasks(sTasks, Tasks);
    bool correct = true;
    for (int i=0; i<Tasks.size(); i++)
    {
        if (!IsTaskNameValid(Tasks[i],sClientName,sIDName))
        {
            if (correct) correct = false;
            continue;
        }

        CreateRequest(sClientName, sIDName, Request, wID);

        if (!Request.IsEmpty())
        {
            if (Settings.DefaultBrowser)
            {
                if (Tasks.size() == 1)
                {
                    ::ShellExecute(NULL,"open",Request,NULL,"",SW_SHOWNORMAL);
                }
                else
                {
                    if (Settings.BrowserPath.IsEmpty())
                    {
                        ::ShellExecute(NULL,"open",Request,NULL,"",SW_SHOWNORMAL);
                    }
                    else
                    {
                        ::ShellExecute(NULL,"open",Settings.BrowserPath,Request,"",SW_SHOWNORMAL);
                    }
                }
            }
            else
            {
                if (Settings.BrowserPath.IsEmpty())
                {
                    ::ShellExecute(NULL,"open",Request,NULL,"",SW_SHOWNORMAL);
                }
                else
                {
                    ::ShellExecute(NULL,"open",Settings.BrowserPath,Request,"",SW_SHOWNORMAL);
                }
            }
        }
    }
    if (!correct)
    {
        if (Tasks.size() > 1)
        {
            MessageBox("Some tasks were not opened because of incorrect format",szWinName,MB_ICONWARNING);
        }
        else
        {
            MessageBox(ErrorString,szWinName,MB_ICONERROR);
        }
    }
    busy = false;
    Tasks.clear();
}

void CMainDlg::CreateRequest(const char *sClientName, const char *sIDName, CString &Request, INT wID)
{
    // opening defect
    for (int i=0; i<Settings.defects.size(); i++)
    {
        if (CompareNoCaseCP1251(sClientName,Settings.defects[i].ClientID)==0)
        {
            Request = Settings.DefectsLink;
            Request.Replace("%PROJECT%",Settings.defects[i].STProject);
            Request.Replace("%ID%",sIDName);
            return;
        }
    }
    // opening TMS task
    if (lstrcmp(sClientName,"")==0)
    {
        Request = "";
        MessageBox("Task without client name is entered.\nThere is no defect with empty client name defined.\nSo, the task cannot be opened.",szWinName,MB_ICONERROR);
        return;
    }
    if ((wID == VIEW_TASK)||(wID == VIEW_TASK_HOTKEY)||(wID == VIEW_CHILD_TASKS)||(wID == VIEW_CHILD_TASKS_HOTKEY))
    {   
        // use default URL to open task/child tasks
        for (int i=0; i<Settings.links.size(); i++)
        {
            if (Settings.links[i].Default)
            {
                // open task
                if ((wID == VIEW_TASK)||(wID == VIEW_TASK_HOTKEY))
                {
                    if (!Settings.links[i].TaskURL.IsEmpty())
                    {
                        Request = Settings.links[i].TaskURL;
                        Request.Replace("%CLIENT%",sClientName);
                        Request.Replace("%ID%",sIDName);
                    }
                    else
                    {
                        MessageBox("URL to open Task is not defined.\nPlease correct settings on URLs page",szWinName,MB_ICONERROR);
                        Request = "";
                    }
                    break;
                }
                // open child tasks
                else
                {
                    if (!Settings.links[i].ChildTasksURL.IsEmpty())
                    {
                        Request = Settings.links[i].ChildTasksURL;
                        Request.Replace("%CLIENT%",sClientName);
                        Request.Replace("%ID%",sIDName);
                    }
                    else
                    {
                        MessageBox("URL to open Child Tasks is not defined.\nPlease correct settings on URLs page",szWinName,MB_ICONERROR);
                        Request = "";
                    }
                    break;
                }
            }
        }
    }
    else
    {
        // use hotkey ID's dependent URL (wID == HotKeyID)
        int index = wID / 2;
        // open task
        if (wID % 2 == 0)
        {
            if (!Settings.links[index].TaskURL.IsEmpty())
            {
                Request = Settings.links[index].TaskURL;
                Request.Replace("%CLIENT%",sClientName);
                Request.Replace("%ID%",sIDName);
            }
            else
            {
                MessageBox("URL to open Task by the hotkey is not defined.\nPlease correct settings on URLs page",szWinName,MB_ICONERROR);
                Request = "";
            }
        }
        // open child tasks
        else
        {
            if (!Settings.links[index].ChildTasksURL.IsEmpty())
            {
                Request = Settings.links[index].ChildTasksURL;
                Request.Replace("%CLIENT%",sClientName);
                Request.Replace("%ID%",sIDName);
            }
            else
            {
                MessageBox("URL to open Child Tasks by the hotkey is not defined.\nPlease correct settings on URLs page",szWinName,MB_ICONERROR);
                Request = "";
            }
        }
    }
    if (!Request.IsEmpty())
    {
        Request.Insert(0,"\"");
        Request += "\"";
    }
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
    COptionSheetDialogImpl<COptionSelectionTreeCtrl, CMyPropSheet> Sheet(IDD_MYOPTIONSHEET);
    Sheet.SetTitle("TMS Launcher settings");
    SystrayMenu.EnableMenuItem(IDC_SETTINGS,MF_BYCOMMAND|MF_GRAYED);
    Sheet.DoModal();
    SystrayMenu.EnableMenuItem(IDC_SETTINGS,MF_BYCOMMAND|MF_ENABLED);
}

LRESULT CMainDlg::OnMsgFilter(LPNMHDR pnmh)
{
    MSGFILTER *msg = (MSGFILTER*)pnmh;
    if (msg->nmhdr.hwndFrom == GetDlgItem(IDC_TASKNAME))
    {
        if (msg->msg == WM_LBUTTONUP)
        {
            if (Settings.SingleClick) TaskNameControl.SetSelAll();
        }
        if (msg->msg == WM_RBUTTONDOWN)
        {
            OnViewTask(0,Settings.RightClickAction ? VIEW_CHILD_TASKS_HOTKEY:VIEW_TASK_HOTKEY,0);
        }
    }
    return 0;
}

bool isalpha_cp1251(char ch)
{
    if ((ch >= (char)0xC0) && (ch <= (char)0xFF))  // cyrilic characters in cp1251
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
    return str1.Compare(str2);
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

