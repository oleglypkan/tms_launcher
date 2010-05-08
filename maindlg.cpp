/*
    File name: maindlg.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: January 18, 2006
*/

#include "stdafx.h"

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: maindlg.cpp,v $$Revision: 1.48 $$Date: 2006/05/04 14:53:44Z $"; 
#endif

#include <fstream.h>
#include "resource.h"
#include "maindlg.h"
#include "Systray.h"
#include "settings.h"
#include "tools.h"
#include "request.h"
#include "Options.h"
#include "Task.h"
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

    TaskNameCombo.Attach(GetDlgItem(IDC_TASKNAME_COMBO));

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
    
    ShowModal = false;
    
    ClipboardSubMenu.CreatePopupMenu();
    ClipboardSubMenu.AppendMenu(MF_ENABLED,VIEW_TASK_HOTKEY,"&View Task");
    ClipboardSubMenu.AppendMenu(MF_ENABLED,VIEW_CHILD_TASKS_HOTKEY,"View &Child Tasks");
    ClipboardSubMenu.AppendMenu(MF_ENABLED,VIEW_PARENT_TASK_HOTKEY,"View &Parent Task");
    
    SettingsSubMenu.CreatePopupMenu();
    SettingsSubMenu.AppendMenu(MF_ENABLED,ID_SETTINGS_GENERAL,"&General");
    SettingsSubMenu.AppendMenu(MF_ENABLED,ID_SETTINGS_URLS,"&URLs");
    SettingsSubMenu.AppendMenu(MF_ENABLED,ID_SETTINGS_DEFECTS,"&Defects");
    SettingsSubMenu.AppendMenu(MF_ENABLED,ID_SETTINGS_FORMAT,"&Format");
    SettingsSubMenu.AppendMenu(MF_ENABLED,ID_SETTINGS_SOFTTEST,"&SoftTest");

    SystrayMenu.CreatePopupMenu();
    SystrayMenu.AppendMenu(MF_ENABLED,ID_SHOW_HIDE_WINDOW,"Show &window");
    SystrayMenu.SetMenuDefaultItem(ID_SHOW_HIDE_WINDOW);
    SystrayMenu.AppendMenu(MF_SEPARATOR);
    SystrayMenu.AppendMenu(MF_ENABLED,VIEW_TASK,"&View Task");
    SystrayMenu.AppendMenu(MF_ENABLED,VIEW_CHILD_TASKS,"View &Child Tasks");
    SystrayMenu.AppendMenu(MF_ENABLED,VIEW_PARENT_TASK,"View &Parent Task");
    SystrayMenu.AppendMenu(MF_SEPARATOR);
    SystrayMenu.AppendMenu(MF_POPUP,(UINT)(HMENU)ClipboardSubMenu,"Clip&board");
    SystrayMenu.AppendMenu(MF_POPUP,(UINT)(HMENU)SettingsSubMenu,"&Settings");
    SystrayMenu.AppendMenu(MF_SEPARATOR);
    SystrayMenu.AppendMenu(MF_ENABLED,ID_CLEAR_HISTORY,"C&lear history");
    SystrayMenu.AppendMenu(MF_SEPARATOR);
    SystrayMenu.AppendMenu(MF_ENABLED,IDC_CLOSE,"&Exit");
    CreateSysTrayIcon(m_hWnd,LoadIcon(_Module.GetModuleInstance(),MAKEINTRESOURCE(IDI_SYSTRAY_ICON)),szWinName);

    return TRUE;
}

void CMainDlg::OnContents(UINT wNotifyCode, INT wID, HWND hWndCtl)
{
    if (HtmlHelp(NULL,HelpFileName,HH_DISPLAY_TOPIC,NULL)==NULL)
    {
        CString ErrorMessage;
        ErrorMessage.Format("Help file \"%s\" was not found\nor your system does not support HTML help",HelpFileName);
        ShowModal = true;
        MessageBox(ErrorMessage,szWinName,MB_ICONERROR);
        ShowModal = false;
    }
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
    DestroyMenu(ContextMenu);
    DestroyMenu(ClipboardSubMenu);
    DestroyMenu(SettingsSubMenu);
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
        ::EnableWindow(GetDlgItem(VIEW_PARENT_TASK),FALSE);
        ::EnableWindow(GetDlgItem(IDC_SETTINGS),FALSE);
    }
    else
    {
        WindowRect.bottom += ButtonRect.bottom - ButtonRect.top + 6;
        SetDlgItemText(IDC_EXPAND,"<<");
        ::EnableWindow(GetDlgItem(VIEW_CHILD_TASKS),TRUE);
        ::EnableWindow(GetDlgItem(VIEW_PARENT_TASK),TRUE);
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

void CMainDlg::AddToHistory(const char *item)
{
    if (Settings.MaxHistoryItems == 0)
    {
        return;
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

    // called after VIEW_TASK, VIEW_CHILD_TASKS or VIEW_PARENT_TASK button was pressed
    if ((wID == VIEW_TASK)||(wID == VIEW_CHILD_TASKS)||(wID == VIEW_PARENT_TASK)||(wNotifyCode==1))
    {
        if (!GetTaskNameFromRichEdit(sTasks))
        {
            ShowModal = true;
            MessageBox("\"Task Name\" field is empty",szWinName,MB_ICONERROR);
            ShowModal = false;
            return;
        }
    }
    // called after systray menu items VIEW_TASK_HOTKEY, VIEW_CHILD_TASKS_HOTKEY or VIEW_PARENT_TASK_HOTKEY 
    // are clicked or hoykey is pressed
    else
    {
        if (!GetTaskNameFromClipboard(sTasks))
        {
            ShowModal = true;
            MessageBox("Clipboard does not contain data in text format",szWinName,MB_ICONERROR);
            ShowModal = false;
            return;
        }
        else
        {
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
    }

    busy = true;

    std::vector<TASKNAME> Tasks;
    std::vector<TASKNAME> Defects;
    TASK task;
    int items = 0;
    bool correct = task.ComplexParseTasks(sTasks, Tasks, Defects, items);
    bool InSoftTest = Settings.OpenDefectsInSoftTest(wID);

    for (int i = 0; i < Tasks.size(); i++)
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
                OpenTask(Request, (Tasks.size() == 1) ? true : false);
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

    if (!correct)
    {
        if (items > 1)
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
    Defects.clear();
}

void CMainDlg::CreateRequest(const char *sClientName, const char *sIDName, CString &Request, INT wID)
{
    int index = -1;

    Request = "";

    // opening defect
    if (Settings.IsDefect(sClientName, NULL, &index))
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
            default:
                switch (wID % 3)
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
                }
        }
        Request.Replace("%PROJECT%",Settings.defects[index].STProject);
        Request.Replace("%ID%",sIDName);
        return;
    }

    // opening TMS task
    if (lstrcmp(sClientName,"")==0)
    {
        MessageBox("Task without client name is entered.\nThere is no defect with empty client name defined.\nSo, the task cannot be opened.",szWinName,MB_ICONERROR);
        return;
    }
    CString tempID = "00000";
    if (lstrlen(sIDName)<5)
    {
        tempID += sIDName;
        tempID.Delete(0,tempID.GetLength()-5);
    }
    else
    {
        tempID = sIDName;
    }
    
    switch (wID)
    {
        case VIEW_TASK:
        case VIEW_TASK_HOTKEY:
        case VIEW_CHILD_TASKS:
        case VIEW_CHILD_TASKS_HOTKEY:
        case VIEW_PARENT_TASK:
        case VIEW_PARENT_TASK_HOTKEY:
            // use default URL to open task/child/parent task(s)
            index = Settings.GetDefaultUrlIndex();
            break;
        default:
            // use hotkey ID's dependent URL (wID == HotKeyID)
            index = wID / 3;
            switch (wID % 3)
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
            }
    }

    switch (wID)
    {
        case VIEW_PARENT_TASK:
        case VIEW_PARENT_TASK_HOTKEY:
        {
            CString HeaderSend, HeaderReceive, Message;
            REQUEST Req;
            if (Settings.links[index].Login.IsEmpty() || Settings.links[index].Password.IsEmpty())
            {
                MessageBox("Both Login and Password must be specified\nin settings window for \""+Settings.links[index].Caption+"\" URL\nto open parent task",szWinName,MB_ICONERROR);
                break;
            }
            Request.Format("http://%s:%s@scc1/~alttms/viewtask.php?Client=%s&ID=%s",
                           Settings.links[index].Login,Settings.links[index].Password,
                           sClientName, tempID);
            if (!Req.SendRequest(false, Request, HeaderSend, HeaderReceive, Message))
            {
                MessageBox("Error while opening parent task",szWinName,MB_ICONERROR);
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
                    MessageBox(Request,szWinName,MB_ICONERROR);
                    Request = "";
                    return;
            }
        }
        case VIEW_TASK:
        case VIEW_TASK_HOTKEY:
            if (!Settings.links[index].TaskURL.IsEmpty())
            {
                if (!Request.IsEmpty())
                {
                    CString Client, ID;
                    int pos = Request.Find('-');
                    Client = Request.Left(pos);
                    ID = Request.Right(Request.GetLength()-pos-1);
                    Request = Settings.links[index].TaskURL;
                    Request.Replace("%CLIENT%",Client);
                    Request.Replace("%ID%",ID);
                }
                else
                {
                    Request = Settings.links[index].TaskURL;
                    Request.Replace("%CLIENT%",sClientName);
                    Request.Replace("%ID%",tempID);
                }
            }
            else
            {
                MessageBox("URL to open Task is not defined.\nPlease correct settings on URLs page",szWinName,MB_ICONERROR);
            }
            break;
        case VIEW_CHILD_TASKS:
        case VIEW_CHILD_TASKS_HOTKEY:
            if (!Settings.links[index].ChildTasksURL.IsEmpty())
            {
                Request = Settings.links[index].ChildTasksURL;
                Request.Replace("%CLIENT%",sClientName);
                Request.Replace("%ID%",tempID);
            }
            else
            {
                MessageBox("URL to open Child Tasks is not defined.\nPlease correct settings on URLs page",szWinName,MB_ICONERROR);
            }
            break;
    }

    if (!Request.IsEmpty())
    {
        if (!Settings.links[index].Login.IsEmpty() && !Settings.links[index].Password.IsEmpty())
        {
            REQUEST::InsertLoginPassword(Request,Settings.links[index].Login,Settings.links[index].Password);
        }
        Request.Insert(0,"\"");
        Request += "\"";
    }
}

void CMainDlg::OpenTask(const char *Request, bool SingleTask)
{
    if (Settings.DefaultBrowser)
    {
        if (SingleTask)
        {
            OpenLink(szWinName,m_hWnd,"open",Request);
        }
        else
        {
            if (Settings.BrowserPath.IsEmpty())
            {
                OpenLink(szWinName,m_hWnd,"open",Request);
            }
            else
            {
                OpenLink(szWinName,m_hWnd,"open",Settings.BrowserPath,Request);
            }
        }
    }
    else
    {
        if (Settings.BrowserPath.IsEmpty())
        {
            OpenLink(szWinName,m_hWnd,"open",Request);
        }
        else
        {
            OpenLink(szWinName,m_hWnd,"open",Settings.BrowserPath,Request);
        }
    }
}

void CMainDlg::OpenDefects(const std::vector<TASKNAME> &Defects, INT wID)
{
    if (Defects.size() == 0) return;

    int start = 0;
    CString DEFECTS = Defects[start].ID;

    int i = 1;
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
        default:
            switch (wID % 3)
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
            }
    }
    Filter.Replace("%ID%",IDs);

    ofstream OutFile;
    CString OutputFileName = Settings.SoftTestFiltersPath+Settings.SoftTestFilterName;
    OutputFileName.Replace("%PROJECT%",Project);
    
    OutFile.open(OutputFileName, ios::out);
    OutFile << Filter;
    OutFile.close();

    OpenLink(szWinName,m_hWnd,"open",Settings.SoftTestPath,Settings.GetSoftTestCommandLine(Project));
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
    
    SystrayMenu.EnableMenuItem(7,MF_BYPOSITION|MF_GRAYED);

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
        default:
            Sheet.DoModal(::GetActiveWindow(),NULL);
    }

    if (OldControlType != Settings.TaskNameControlType)
    {
        SwitchControls();
    }

    int items = TaskNameCombo.GetCount(); // actual number of items in ComboBox control
    if (items > Settings.MaxHistoryItems)
    {
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
        if (Settings.MaxHistoryItems == 0)
        {
            TaskNameCombo.ResetContent();
        }
        else
        {
            for (int i = Settings.MaxHistoryItems; i < items; i++)
            {
                TaskNameCombo.DeleteString(Settings.MaxHistoryItems);
            }
        }
        if (!Task.IsEmpty())
        {
            TaskNameCombo.SetWindowText(Task);
        }
    }

    SystrayMenu.EnableMenuItem(7,MF_BYPOSITION|MF_ENABLED);
}

void CMainDlg::OnClearHistory(UINT wNotifyCode, INT wID, HWND hWndCtl)
{
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
            }
        }
        else  // processing context menus for main window buttons
        {
            CMenu ButtonMenu;
            ButtonMenu = CreatePopupMenu();
            for (int i=0; i < Settings.links.size(); i++)
            {
                ButtonMenu.AppendMenu(MF_ENABLED,i+1,Settings.links[i].Caption);
                if (Settings.links[i].Default)
                {
                    ButtonMenu.SetMenuDefaultItem(i+1);
                }
            }
            CButton Button = hwndFrom;
            RECT Rect;
            Button.GetWindowRect(&Rect);
            UINT Command = 0;

            if (hwndFrom == GetDlgItem(VIEW_TASK))
            {
                Command = TrackPopupMenuEx(ButtonMenu,TPM_RETURNCMD|TPM_LEFTBUTTON|TPM_LEFTALIGN|TPM_TOPALIGN,Rect.left,Rect.bottom,m_hWnd,NULL);
                if (Command != 0)
                {
                    OnViewTask(1,(Command-1)*3,0);
                }
            }
            else
            {
                if (hwndFrom == GetDlgItem(VIEW_CHILD_TASKS))
                {
                    Command = TrackPopupMenuEx(ButtonMenu,TPM_RETURNCMD|TPM_LEFTBUTTON|TPM_LEFTALIGN|TPM_TOPALIGN,Rect.left,Rect.bottom,m_hWnd,NULL);
                    if (Command != 0)
                    {
                        OnViewTask(1,(Command-1)*3+1,0);
                    }
                }
                else
                {
                    if (hwndFrom == GetDlgItem(VIEW_PARENT_TASK))
                    {
                        Command = TrackPopupMenuEx(ButtonMenu,TPM_RETURNCMD|TPM_LEFTBUTTON|TPM_LEFTALIGN|TPM_TOPALIGN,Rect.left,Rect.bottom,m_hWnd,NULL);
                        if (Command != 0)
                        {
                            OnViewTask(1,(Command-1)*3+2,0);
                        }
                    }
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

