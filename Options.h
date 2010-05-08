/*
    File name: Options.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
*/

// Several pages should be created as dialogs templates with IDs == IDDs from pages classes.
// Make them with Style=Child|Control, Border=Thin and a titlebar (to have a caption). 
// These pages will be included to options sheet.
// To move the pages at the right position on the sheet, use the XPos and YPos fields
// in the 'Dialog Properties' from the resource editor.
//
// Dialog template for options sheet that will contain all pages should be created 
// with ID used in Sheet definition (IDD_MYOPTIONSHEET in the example below):
// COptionSheetDialogImpl<COptionSelectionTreeCtrl, CMyPropSheet> Sheet(IDD_MYOPTIONSHEET);
// TabCtrl or TreeCtrl should be placed on this dialog with ID == ATL_IDC_TAB_CONTROL.
// Also necessary buttons should be placed on the dialog template for the sheet.
//
// !!! /GR compiler option should be used !!!
//
// In OnInitDialog() function of each page you can initialize controls with values of settings
// In OnOK() function of each CPropPage you can read values of settings and save them
// In OnKillActive() function you can check for correct values entered by user and allow 
// or disallow the page to be changed
// define USE_ICONS as 1 if you want to add icons to the items in the tree control, as 0 otherwise

#ifndef OPTIONS_H_INCLUDED
#define OPTIONS_H_INCLUDED

#define USE_ICONS 1

#include "MOptionSheet.cpp"
#include "settings.h"
#include "About.h"
#include "htmlhelp.h"
#include "controls.h"

#ifdef _DEBUG
#include <crtdbg.h>
#include <stdlib.h>
#define _CRTDBG_MAP_ALLOC 
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

extern CSettings Settings;
extern CString szWinName;
extern UINT LINK_MAX;
const int max_value_length = 2;
const char *HelpFileName = "TMS_Launcher.chm";

bool isalpha_cp1251(char ch);
int CompareNoCaseCP1251(const char *string1, const char *string2);

//////////////////////////// General options page //////////////////////
class GeneralPage : public Mortimer::COptionPageImpl<GeneralPage,CPropPage>
{
public:
    enum { IDD = GENERAL_PAGE };
    CComboBox TaskNameControlSetup;
    CComboBox RightClickCombo;
    int RightClickAction;
    int RightClickAction2;
    bool SingleClick;
    CEdit PathToBrowser;
    CString sBrowser;
    bool sCheckMark;

    BEGIN_MSG_MAP(GeneralPage)
        MSG_WM_INITDIALOG(OnInitDialog)
        COMMAND_ID_HANDLER_EX(IDC_BROWSE,OnBrowse)
        COMMAND_ID_HANDLER_EX(IDC_SINGLE_CLICK,OnCheckBoxClick)
        COMMAND_CODE_HANDLER_EX(CBN_SELCHANGE, OnSelChanged)
    END_MSG_MAP()

    // called once when options dialog is opened
    LRESULT OnInitDialog(HWND hWnd, LPARAM lParam)
    {
        RightClickAction = Settings.RightClickAction;
        RightClickAction2 = Settings.RightClickAction2;

        TaskNameControlSetup.Attach(GetDlgItem(IDC_TASK_NAME_CONTROL));
        TaskNameControlSetup.AddString("RichEdit");
        TaskNameControlSetup.AddString("ComboBox");
        TaskNameControlSetup.SetCurSel(Settings.TaskNameControlType);

        RightClickCombo.Attach(GetDlgItem(IDC_RIGHT_CLICK));
        RightClickCombo.AddString("View Task");
        RightClickCombo.AddString("View Child Tasks");
        RightClickCombo.AddString("View Parent Task");
        RightClickCombo.AddString("View Related Tasks");

        switch (Settings.TaskNameControlType)
        {
            case 0:
                RightClickCombo.AddString("Open context menu");
                RightClickCombo.SetCurSel(RightClickAction);
                break;
            case 1:
                RightClickCombo.SetCurSel(RightClickAction2);
                break;
        }

        if (Settings.Expand) SendDlgItemMessage(IDC_EXPANDED,BM_SETCHECK,BST_CHECKED,0);
        if (Settings.AutoRun) SendDlgItemMessage(IDC_AUTORUN,BM_SETCHECK,BST_CHECKED,0);
        if (Settings.Minimize) SendDlgItemMessage(IDC_MINIMIZE,BM_SETCHECK,BST_CHECKED,0);

        SingleClick = Settings.SingleClick;
        if (Settings.TaskNameControlType == 1) // ComboBox control
        {
            ::EnableWindow(GetDlgItem(IDC_SINGLE_CLICK),FALSE);
        }
        else
        {
            if (Settings.SingleClick) SendDlgItemMessage(IDC_SINGLE_CLICK,BM_SETCHECK,BST_CHECKED,0);
        }
        if (Settings.DefaultBrowser) SendDlgItemMessage(IDC_DEFAULT_BROWSER,BM_SETCHECK,BST_CHECKED,0);
        sCheckMark = Settings.DefaultBrowser;
        sBrowser = Settings.BrowserPath;
        PathToBrowser.Attach(GetDlgItem(IDC_BROWSER_PATH));
        PathToBrowser.LimitText(MAX_PATH);
        PathToBrowser.SetWindowText(Settings.BrowserPath);
        return 0;
    }

    void OnBrowse(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        CFileDialog Browser(TRUE,NULL,NULL,OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES|OFN_PATHMUSTEXIST,
                            "Programs\0*.exe");
        CString temp = szWinName;
        temp += " - Select browser";
        Browser.m_ofn.lpstrTitle = temp;
        Browser.DoModal();
        if (lstrcmp(Browser.m_szFileName,"")!=0)
        {
            PathToBrowser.SetWindowText(Browser.m_szFileName);
        }
    }

    void OnCheckBoxClick(UINT code, int idFrom, HWND hwndFrom)
    {
        if (idFrom == IDC_SINGLE_CLICK)
        {
            SingleClick = (SendDlgItemMessage(IDC_SINGLE_CLICK,BM_GETCHECK,0,0) == BST_CHECKED);
        }
    }

    void OnSelChanged(UINT code, int ControlID, HWND ControlHandle)
    {
        switch (ControlID)
        {
            case IDC_TASK_NAME_CONTROL:
                switch (TaskNameControlSetup.GetCurSel())
                {
                    case 0: // RichEdit control
                        ::EnableWindow(GetDlgItem(IDC_SINGLE_CLICK),TRUE);
                        SendDlgItemMessage(IDC_SINGLE_CLICK,BM_SETCHECK,SingleClick?BST_CHECKED:BST_UNCHECKED,0);
                        RightClickCombo.AddString("Open context menu");
                        RightClickCombo.SetCurSel(RightClickAction);
                        break;
                    case 1: // ComboBox control
                        SendDlgItemMessage(IDC_SINGLE_CLICK,BM_SETCHECK,BST_UNCHECKED,0);
                        ::EnableWindow(GetDlgItem(IDC_SINGLE_CLICK),FALSE);
                        RightClickCombo.DeleteString(4);
                        RightClickCombo.SetCurSel(RightClickAction2);
                        break;
                }
                break;
            case IDC_RIGHT_CLICK:
                switch (TaskNameControlSetup.GetCurSel())
                {
                    case 0: // RichEdit control
                        RightClickAction = RightClickCombo.GetCurSel();
                        break;
                    case 1: // ComboBox control
                        RightClickAction2 = RightClickCombo.GetCurSel();
                        break;
                }
                break;
        }
    }

    // called every time when the page is deactivated
    bool OnKillActive(COptionItem *pItem)
    {
        sCheckMark = (SendDlgItemMessage(IDC_DEFAULT_BROWSER,BM_GETCHECK,0,0)==BST_CHECKED);
        PathToBrowser.GetWindowText(sBrowser.GetBuffer(MAX_PATH+1),MAX_PATH+1);
        sBrowser.ReleaseBuffer();
        return true;
    }

    // called every time when whole sheet is closed by clicking on OK button
    void OnOK()
    {
        Settings.AutoRun = (SendDlgItemMessage(IDC_AUTORUN,BM_GETCHECK,0,0)==BST_CHECKED);
        Settings.Expand = (SendDlgItemMessage(IDC_EXPANDED,BM_GETCHECK,0,0)==BST_CHECKED);
        Settings.Minimize = (SendDlgItemMessage(IDC_MINIMIZE,BM_GETCHECK,0,0)==BST_CHECKED);
        Settings.DefaultBrowser = (SendDlgItemMessage(IDC_DEFAULT_BROWSER,BM_GETCHECK,0,0)==BST_CHECKED);
        Settings.TaskNameControlType = TaskNameControlSetup.GetCurSel();
        Settings.RightClickAction = RightClickAction;
        Settings.RightClickAction2 = RightClickAction2;        
        Settings.SingleClick = SingleClick;
        PathToBrowser.GetWindowText(Settings.BrowserPath.GetBuffer(MAX_PATH+1),MAX_PATH+1);
        Settings.BrowserPath.ReleaseBuffer();
        Settings.BrowserPath.TrimLeft();
        Settings.BrowserPath.TrimRight();
        Settings.SaveGeneralSettings();
    }

    // called every time when whole sheet is closed by clicking on Cancel button
    void OnCancel() {}
};

//////////////////////////// Format options page //////////////////////
class FormatPage : public Mortimer::COptionPageImpl<FormatPage,CPropPage>
{
public:
    enum { IDD = TASKS_PAGE };
    CEdit sMinClientName;
    CEdit sMaxClientName;
    CEdit sMinIDName;
    CEdit sMaxIDName;
    CEdit sMinExtName;
    CEdit sMaxExtName;
    CEdit sSeparators;
    CEdit sTasksSeparators;

    BEGIN_MSG_MAP(FormatPage)
        MSG_WM_INITDIALOG(OnInitDialog)
        COMMAND_CODE_HANDLER_EX(EN_UPDATE, OnEditUpdate)
    END_MSG_MAP()

    LRESULT OnInitDialog(HWND hWnd, LPARAM lParam)
    {
        sMinClientName.Attach(GetDlgItem(IDC_MIN_CLIENT));
        sMaxClientName.Attach(GetDlgItem(IDC_MAX_CLIENT));
        sMinIDName.Attach(GetDlgItem(IDC_MIN_ID));
        sMaxIDName.Attach(GetDlgItem(IDC_MAX_ID));
        sMinExtName.Attach(GetDlgItem(IDC_MIN_EXT));
        sMaxExtName.Attach(GetDlgItem(IDC_MAX_EXT));
        sSeparators.Attach(GetDlgItem(IDC_SEPARATORS));
        sTasksSeparators.Attach(GetDlgItem(IDC_TASKS_SEPARATORS));
        
        CString temp;
        
        sMinClientName.LimitText(max_value_length);
        temp.Format("%d",Settings.MinClientName);
        sMinClientName.SetWindowText(temp);

        sMaxClientName.LimitText(max_value_length);
        temp.Format("%d",Settings.MaxClientName);
        sMaxClientName.SetWindowText(temp);

        sMinIDName.LimitText(max_value_length);
        temp.Format("%d",Settings.MinIDName);
        sMinIDName.SetWindowText(temp);
        
        sMaxIDName.LimitText(max_value_length);
        temp.Format("%d",Settings.MaxIDName);
        sMaxIDName.SetWindowText(temp);

        sMinExtName.LimitText(max_value_length);
        temp.Format("%d",Settings.MinExt);
        sMinExtName.SetWindowText(temp);

        sMaxExtName.LimitText(max_value_length);
        temp.Format("%d",Settings.MaxExt);
        sMaxExtName.SetWindowText(temp);

        sSeparators.LimitText(255);
        sSeparators.SetWindowText(Settings.Separators);
        sTasksSeparators.LimitText(255);
        sTasksSeparators.SetWindowText(Settings.TasksSeparators);

        if (Settings.FillID)
        {
            SendDlgItemMessage(IDC_FILL_ID,BM_SETCHECK,BST_CHECKED,0);
        }
        return 0;
    }

    void OnEditUpdate(UINT code, int ControlID, HWND ControlHandle)
    {
        if ((ControlID == IDC_SEPARATORS) || (ControlID == IDC_TASKS_SEPARATORS))
        {
            return;
        }
        CString temp;
        CEdit EditControl;
        EditControl.Attach(GetDlgItem(ControlID));
        EditControl.GetWindowText(temp.GetBuffer(max_value_length+1),max_value_length+1);
        temp.ReleaseBuffer();
        int i=0;
        while (i<temp.GetLength())
        {
            if (!isdigit((unsigned int)temp[i]))
            {
                temp.Delete(i);
                EditControl.SetWindowText(temp);
            }
            else
            {
                i++;
            }
        }
    }

    // called every time when whole sheet is closed by clicking on OK button
    void OnOK()
    {
        CString temp;
        // %CLIENT%
        sMaxClientName.GetWindowText(temp.GetBuffer(max_value_length+1),max_value_length+1);
        temp.ReleaseBuffer();
        Settings.MaxClientName = atoi(temp);
        // %ID%
        sMaxIDName.GetWindowText(temp.GetBuffer(max_value_length+1),max_value_length+1);
        temp.ReleaseBuffer();
        Settings.MaxIDName = atoi(temp);
        // %EXT%
        sMaxExtName.GetWindowText(temp.GetBuffer(max_value_length+1),max_value_length+1);
        temp.ReleaseBuffer();
        Settings.MaxExt = atoi(temp);

        sSeparators.GetWindowText(temp.GetBuffer(256),256);
        temp.ReleaseBuffer();
        Settings.RemoveDuplicateSeparators(temp);
        Settings.Separators = temp;
        sTasksSeparators.GetWindowText(temp.GetBuffer(256),256);
        temp.ReleaseBuffer();
        Settings.RemoveDuplicateSeparators(temp);
        Settings.TasksSeparators = temp;

        Settings.FillID = (SendDlgItemMessage(IDC_FILL_ID,BM_GETCHECK,0,0)==BST_CHECKED);

        Settings.SaveFormatSettings();
    }

    // called every time when whole sheet is closed by clicking on Cancel button
    void OnCancel() {}

    // called every time when the page is deactivated
    bool OnKillActive(COptionItem *pItem)
    {
        
        CString temp, temp2;
        int max_value;

        // %CLIENT%
        sMaxClientName.GetWindowText(temp.GetBuffer(max_value_length+1),max_value_length+1);
        temp.ReleaseBuffer();
        max_value = atoi(temp);
        if (max_value < Settings.MinClientName)
        {
            MyMessageBox(m_hWnd,"Maximum %CLIENT% length cannot be less than minimum one.\nPlease enter correct value",szWinName,MB_ICONERROR);
            return false;
        }
        // %ID%
        sMaxIDName.GetWindowText(temp.GetBuffer(max_value_length+1),max_value_length+1);
        temp.ReleaseBuffer();
        max_value = atoi(temp);
        if (max_value < Settings.MinIDName)
        {
            MyMessageBox(m_hWnd,"Maximum %ID% length cannot be less than minimum one.\nPlease enter correct value",szWinName,MB_ICONERROR);
            return false;
        }
        // %EXT%
        sMaxExtName.GetWindowText(temp.GetBuffer(max_value_length+1),max_value_length+1);
        temp.ReleaseBuffer();
        max_value = atoi(temp);
        if (max_value < Settings.MinExt)
        {
            MyMessageBox(m_hWnd,"Maximum %EXT% length cannot be less than minimum one.\nPlease enter correct value",szWinName,MB_ICONERROR);
            return false;
        }

        // checking for "Separators" and "TasksSeparators" fields are filled correctly
        sSeparators.GetWindowText(temp.GetBuffer(256),256);
        temp.ReleaseBuffer();
        if (temp.IsEmpty())
        {
            MyMessageBox(m_hWnd,"\"Separators\" field must not be empty.\nPlease enter correct value",szWinName,MB_ICONERROR);
            return false;
        }
        for (int i=0; i<temp.GetLength(); i++)
        {
            if (((unsigned char)(temp[i]) == '_') || (isalpha_cp1251((unsigned char)(temp[i]))) || (isdigit((unsigned char)(temp[i]))))
            {
                if (MyMessageBox(m_hWnd,"\"Separators\" field has character(s) that cannot be used as separators.\nWould you like the character(s) to be removed automatically?",szWinName,MB_YESNO|MB_ICONERROR)==IDYES)
                {
                    Settings.RemoveUnacceptableSeparators(temp);
                    sSeparators.SetWindowText(temp);
                }
                return false;
            }
        }

        sTasksSeparators.GetWindowText(temp2.GetBuffer(256),256);
        temp2.ReleaseBuffer();
        if (temp2.IsEmpty())
        {
            MyMessageBox(m_hWnd,"\"Tasks separators\" field must not be empty.\nPlease enter correct value",szWinName,MB_ICONERROR);
            return false;
        }
        for (i=0; i<temp2.GetLength(); i++)
        {
            if (((unsigned char)(temp2[i]) == '_') || (isalpha_cp1251((unsigned char)(temp2[i]))) || (isdigit((unsigned char)(temp2[i]))))
            {
                if (MyMessageBox(m_hWnd,"\"Tasks separators\" field has character(s) that cannot be used as separators.\nWould you like the character(s) to be removed automatically?",szWinName,MB_YESNO|MB_ICONERROR)==IDYES)
                {
                    Settings.RemoveUnacceptableSeparators(temp2);
                    sTasksSeparators.SetWindowText(temp2);
                }
                return false;
            }
        }
        int pos = temp.FindOneOf(temp2);
        if (pos != -1)
        {
            CString Message;
            Message.Format("Character '%c' is entered to both \"Separators\" and \"Tasks separators\" fields.\nPlease remove it from one of the fields",temp[pos]);
            MyMessageBox(m_hWnd,Message,szWinName,MB_ICONERROR);
            return false;
        }
        if (Settings.CorrectCRLF(temp,temp2)) // correction was done
        {
            sSeparators.SetWindowText(temp);
            sTasksSeparators.SetWindowText(temp2);
            MyMessageBox(m_hWnd,"Characters 'CR' and 'LF' should be used simultaneously as a separator.\nNecessary corrections have been made to the settings",szWinName,MB_ICONINFORMATION);
        }
        return true;
    }

};

//////////////////////////// Links options page ///////////////////////
class URLsPage;

class URLEditPage : public CDialogImpl<URLEditPage>
{
public:
    enum { IDD = URL_EDIT_PAGE };
    int Action; // 0 - new URL, 1 - edit URL, 2 - copy URL
    link URL;
    int MaxStringLength;
    std::vector<link> *Links;
    UINT *HotKey;
    UINT WinKey;
    bool IE;
    bool use_iexplore;

    CEdit CaptionEdit;
    CEdit TaskURL;
    CEdit ChildTasksURL;
    CEdit RelatedTasksURL;
    CEdit Login;
    CEdit Password;
    CWindow TaskHotkey;
    CWindow ChildTasksHotkey;
    CWindow ParentTaskHotkey;
    CWindow RelatedTasksHotkey;

    URLEditPage(int action, link url, std::vector<link> *links, UINT *ForbiddenHotkey, UINT *uWinKey, bool use_DEF, bool use_IE):URL(url)
    {
        Action = action;
        Links = links;
        MaxStringLength = 255;
        HotKey = NULL;
        if (ForbiddenHotkey != NULL)
        {
            HotKey = ForbiddenHotkey;
        }
        WinKey = 0;
        if (uWinKey != NULL)
        {
            WinKey = *uWinKey;
        }
        IE = use_DEF;
        use_iexplore = use_IE;
    }

    BEGIN_MSG_MAP(URLEditPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow();
        CaptionEdit.Attach(GetDlgItem(IDC_URL_EDIT));
        CaptionEdit.LimitText(MaxStringLength);
        TaskURL.Attach(GetDlgItem(IDC_TASK_URL));
        TaskURL.LimitText(LINK_MAX);
        ChildTasksURL.Attach(GetDlgItem(IDC_CHILD_TASKS_URL));
        ChildTasksURL.LimitText(LINK_MAX);
        RelatedTasksURL.Attach(GetDlgItem(IDC_RELATED_TASKS_URL));
        RelatedTasksURL.LimitText(LINK_MAX);
        Login.Attach(GetDlgItem(IDC_LOGIN));
        Login.LimitText(MaxStringLength);
        Password.Attach(GetDlgItem(IDC_PASSWORD));
        Password.LimitText(MaxStringLength);
        TaskHotkey.Attach(GetDlgItem(VIEW_TASK_HOTKEY));
        ChildTasksHotkey.Attach(GetDlgItem(VIEW_CHILD_TASKS_HOTKEY));
        ParentTaskHotkey.Attach(GetDlgItem(VIEW_PARENT_TASK_HOTKEY));
        RelatedTasksHotkey.Attach(GetDlgItem(VIEW_RELATED_TASKS_HOTKEY));

        CaptionEdit.SetWindowText(URL.Caption);
        TaskURL.SetWindowText(URL.TaskURL);
        ChildTasksURL.SetWindowText(URL.ChildTasksURL);
        RelatedTasksURL.SetWindowText(URL.RelatedTasksURL);
        Login.SetWindowText(URL.Login);
        Password.SetWindowText(URL.Password);
        TaskHotkey.SendMessage(HKM_SETHOTKEY,URL.ViewTaskHotKey,0);
        ChildTasksHotkey.SendMessage(HKM_SETHOTKEY,URL.ViewChildTasksHotKey,0);
        ParentTaskHotkey.SendMessage(HKM_SETHOTKEY,URL.ViewParentTaskHotKey,0);
        RelatedTasksHotkey.SendMessage(HKM_SETHOTKEY,URL.ViewRelatedTasksHotKey,0);
        SendDlgItemMessage(IDC_ST_DEFECTS,BM_SETCHECK,URL.DefectsInSoftTest ? BST_CHECKED : BST_UNCHECKED,0);

        switch(Action)
        {
            case 0: // new
                SetWindowText("New URL record");
                break;
            case 1: // edit
                SetWindowText("Edit URL record");
                break;
            case 2: // copy
                SetWindowText("New URL record");
                URL.Caption = "";
                URL.Default = false; // do not copy Default flag
                break;
        }
        return 0;
    }

    bool IExploreDefaultBrowserOnXP()
    {
        // checking if Microsoft Internet Explorer is default browser on Windows prior to Vista
        Registry ClassesRootKey(HKEY_CLASSES_ROOT);
        bool CLASSES_ROOT_HTTP = false;
        bool CLASSES_ROOT_HTTPS = false;
        CString Str = "";
        if (ClassesRootKey.ReadValue("http\\shell\\open\\command","",REG_SZ,(LPBYTE)Str.GetBuffer(MAX_PATH+1),MAX_PATH+1))
        {
            Str.ReleaseBuffer();
            Str.MakeUpper();
            if (Str.Find("IEXPLORE") != -1)
            {
                CLASSES_ROOT_HTTP = true;
            }
        }
        Str = "";
        if (ClassesRootKey.ReadValue("https\\shell\\open\\command","",REG_SZ,(LPBYTE)Str.GetBuffer(MAX_PATH+1),MAX_PATH+1))
        {
            Str.ReleaseBuffer();
            Str.MakeUpper();
            if (Str.Find("IEXPLORE") != -1)
            {
                CLASSES_ROOT_HTTPS = true;
            }
        }
        if (CLASSES_ROOT_HTTP && CLASSES_ROOT_HTTPS)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool IExploreDefaultBrowser()
    {
        // checking if Microsoft Internet Explorer is default browser
        if (CMainDlg::OsMajorVer()>=6) // OS is Windows Vista or higher
        {
            Registry CurrentUserKey(HKEY_CURRENT_USER);
            if (!CurrentUserKey.KeyPresent("Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice")
                && !CurrentUserKey.KeyPresent("Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\https\\UserChoice"))
            {
                return IExploreDefaultBrowserOnXP();
            }
            bool CURRENT_USER_HTTP = false;
            bool CURRENT_USER_HTTPS = false;
            CString Str = "";
            if (CurrentUserKey.ReadValue("Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice","Progid",REG_SZ,(LPBYTE)Str.GetBuffer(MAX_PATH+1),MAX_PATH+1))
            {
                Str.ReleaseBuffer();
                Str.MakeUpper();
                if (Str.Find("IE.HTTP") != -1)
                {
                    CURRENT_USER_HTTP = true;
                }
            }
            Str = "";
            if (CurrentUserKey.ReadValue("Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\https\\UserChoice","Progid",REG_SZ,(LPBYTE)Str.GetBuffer(MAX_PATH+1),MAX_PATH+1))
            {
                Str.ReleaseBuffer();
                Str.MakeUpper();
                if (Str.Find("IE.HTTPS") != -1)
                {
                    CURRENT_USER_HTTPS = true;
                }
            }
            if (CURRENT_USER_HTTP && CURRENT_USER_HTTPS)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else // other Operating systems
        {
            return IExploreDefaultBrowserOnXP();
        }
    }

    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        if (wID == IDOK)
        {
            // checking for unique and non-empty URLCaption
            CString strURLCaption;
            CaptionEdit.GetWindowText(strURLCaption.GetBuffer(MaxStringLength+1),MaxStringLength+1);
            strURLCaption.ReleaseBuffer();
        
            if (strURLCaption.IsEmpty())
            {
                MyMessageBox(m_hWnd,"\"URL caption\" field must not be empty.\nPlease enter a value in the field",szWinName,MB_ICONERROR);
                return 0;
            }
            if (GetPosByCaption(strURLCaption) != -1) // new caption is not unique
            {
                if (CompareNoCaseCP1251(URL.Caption,strURLCaption) != 0)
                {
                    MyMessageBox(m_hWnd,"The value entered in \"URL caption\" field is not unique.\nPlease use another one",szWinName,MB_ICONERROR);
                    return 0;
                }
            }
            // checking for non-empty links
            CString strTaskURL = "", strChildTasksURL = "", strRelatedTasksURL = "";
            TaskURL.GetWindowText(strTaskURL.GetBuffer(LINK_MAX+1),LINK_MAX+1);
            strTaskURL.ReleaseBuffer();
            if (strTaskURL.IsEmpty())
            {
                MyMessageBox(m_hWnd,"URL to view Task cannot be empty.\nPlease enter correct URL",szWinName,MB_ICONERROR);
                return 0;
            }
            ChildTasksURL.GetWindowText(strChildTasksURL.GetBuffer(LINK_MAX+1),LINK_MAX+1);
            strChildTasksURL.ReleaseBuffer();
            if (strChildTasksURL.IsEmpty())
            {
                MyMessageBox(m_hWnd,"URL to view Child Tasks cannot be empty.\nPlease enter correct URL",szWinName,MB_ICONERROR);
                return 0;
            }
            RelatedTasksURL.GetWindowText(strRelatedTasksURL.GetBuffer(LINK_MAX+1),LINK_MAX+1);
            strRelatedTasksURL.ReleaseBuffer();
/*
            if (strRelatedTasksURL.IsEmpty())
            {
                MyMessageBox(m_hWnd,"URL to view Related Tasks cannot be empty.\nPlease enter correct URL",szWinName,MB_ICONERROR);
                return 0;
            }
*/
            // reading login and password
            CString strLogin, strPassword;
            Login.GetWindowText(strLogin.GetBuffer(MaxStringLength+1),MaxStringLength+1);
            strLogin.ReleaseBuffer();
            Password.GetWindowText(strPassword.GetBuffer(MaxStringLength+1),MaxStringLength+1);
            strPassword.ReleaseBuffer();
            // checking hotkeys
            UINT Hotkey1 = (UINT)TaskHotkey.SendMessage(HKM_GETHOTKEY,0,0);
            UINT Hotkey2 = (UINT)ChildTasksHotkey.SendMessage(HKM_GETHOTKEY,0,0);
            UINT Hotkey3 = (UINT)ParentTaskHotkey.SendMessage(HKM_GETHOTKEY,0,0);
            UINT Hotkey4 = (UINT)RelatedTasksHotkey.SendMessage(HKM_GETHOTKEY,0,0);

            if ( 
                 ((Hotkey1)&&(Hotkey1 == Hotkey2)) || ((Hotkey1)&&(Hotkey1 == Hotkey3)) || ((Hotkey1)&&(Hotkey1 == Hotkey4)) ||
                 ((Hotkey2)&&(Hotkey2 == Hotkey3)) || ((Hotkey2)&&(Hotkey2 == Hotkey4)) || ((Hotkey3)&&(Hotkey3 == Hotkey4))
               )
            {
                MyMessageBox(m_hWnd,"At least two entered hotkeys are the same.\nPlease use different ones.",szWinName,MB_ICONERROR);
                return 0;
            }
            for (unsigned int i=0; i<(*Links).size(); i++)
            {
                if (CompareNoCaseCP1251(URL.Caption,(*Links)[i].Caption)==0)
                {
                    continue;
                }
                if (!CheckAndReassignHotkey(Hotkey1, 1, i))
                {
                    return 0;
                }
                if (!CheckAndReassignHotkey(Hotkey2, 2, i))
                {
                    return 0;
                }
                if (!CheckAndReassignHotkey(Hotkey3, 3, i))
                {
                    return 0;
                }
                if (!CheckAndReassignHotkey(Hotkey4, 4, i))
                {
                    return 0;
                }
            }
            if ((!WinKey) && (HotKey != NULL))
            {
                CString message = "";
                if (Hotkey1 && (Hotkey1 == *HotKey))
                {
                    message.Format("Hotkey entered in \"HotKey to View Task\" field\nis already used for fast window activation.\nYou can re-assign entered hotkey to current URL\nor choose another hotkey.\n\nWould you like to re-assign the hotkey?");
                }
                if (Hotkey2 && (Hotkey2 == *HotKey))
                {
                    message.Format("Hotkey entered in \"HotKey to View Child Tasks\" field\nis already used for fast window activation.\nYou can re-assign entered hotkey to current URL\nor choose another hotkey.\n\nWould you like to re-assign the hotkey?");
                }
                if (Hotkey3 && (Hotkey3 == *HotKey))
                {
                    message.Format("Hotkey entered in \"HotKey to View Parent Task\" field\nis already used for fast window activation.\nYou can re-assign entered hotkey to current URL\nor choose another hotkey.\n\nWould you like to re-assign the hotkey?");
                }
                if (Hotkey4 && (Hotkey4 == *HotKey))
                {
                    message.Format("Hotkey entered in \"HotKey to View Related Tasks\" field\nis already used for fast window activation.\nYou can re-assign entered hotkey to current URL\nor choose another hotkey.\n\nWould you like to re-assign the hotkey?");
                }
                if (!message.IsEmpty())
                {
                    if (MyMessageBox(m_hWnd,message,szWinName,MB_ICONQUESTION|MB_YESNO) == IDYES)
                    {
                        *HotKey = 0;
                    }
                    else
                    {
                        return 0;
                    }
                }
            }

            UINT HotKeyID = 0;
            if (Hotkey1)
            {
                if (!RegisterHotKey(::GetParent(::GetParent(GetParent())),HotKeyID,(!(Hotkey1&0x500)?
                               HIBYTE(LOWORD(Hotkey1)):((Hotkey1&0x500)<0x500?
                               HIBYTE(LOWORD(Hotkey1))^5:HIBYTE(LOWORD(Hotkey1)))),
                               LOBYTE(LOWORD(Hotkey1))))
                {
                    MyMessageBox(m_hWnd,"Hotkey entered in \"HotKey to View Task\" field is already registered by another program.\nPlease enter another hotkey",szWinName,MB_ICONERROR);
                    return 0;
                }
                UnregisterHotKey(::GetParent(::GetParent(GetParent())),HotKeyID);
            }
            HotKeyID++;
            if (Hotkey2)
            {
                if (!RegisterHotKey(::GetParent(::GetParent(GetParent())),HotKeyID,(!(Hotkey2&0x500)?
                                HIBYTE(LOWORD(Hotkey2)):((Hotkey2&0x500)<0x500?
                                HIBYTE(LOWORD(Hotkey2))^5:HIBYTE(LOWORD(Hotkey2)))),
                                LOBYTE(LOWORD(Hotkey2))))
                {
                    MyMessageBox(m_hWnd,"Hotkey entered in \"To View Child Tasks\" field is already registered by another program.\nPlease enter another hotkey",szWinName,MB_ICONERROR);
                    return 0;
                }
                UnregisterHotKey(::GetParent(::GetParent(GetParent())),HotKeyID);
            }
            HotKeyID++;
            if (Hotkey3)
            {
                if (!RegisterHotKey(::GetParent(::GetParent(GetParent())),HotKeyID,(!(Hotkey3&0x500)?
                               HIBYTE(LOWORD(Hotkey3)):((Hotkey3&0x500)<0x500?
                               HIBYTE(LOWORD(Hotkey3))^5:HIBYTE(LOWORD(Hotkey3)))),
                               LOBYTE(LOWORD(Hotkey3))))
                {
                    MyMessageBox(m_hWnd,"Hotkey entered in \"To View Parent Task\" field is already registered by another program.\nPlease enter another hotkey",szWinName,MB_ICONERROR);
                    return 0;
                }
                UnregisterHotKey(::GetParent(::GetParent(GetParent())),HotKeyID);
            }
            HotKeyID++;
            if (Hotkey4)
            {
                if (!RegisterHotKey(::GetParent(::GetParent(GetParent())),HotKeyID,(!(Hotkey4&0x500)?
                               HIBYTE(LOWORD(Hotkey4)):((Hotkey4&0x500)<0x500?
                               HIBYTE(LOWORD(Hotkey4))^5:HIBYTE(LOWORD(Hotkey4)))),
                               LOBYTE(LOWORD(Hotkey4))))
                {
                    MyMessageBox(m_hWnd,"Hotkey entered in \"To View Related Tasks\" field is already registered by another program.\nPlease enter another hotkey",szWinName,MB_ICONERROR);
                    return 0;
                }
                UnregisterHotKey(::GetParent(::GetParent(GetParent())),HotKeyID);
            }

            URL.Caption = strURLCaption;
            URL.TaskURL = strTaskURL;
            URL.ChildTasksURL = strChildTasksURL;
            URL.RelatedTasksURL = strRelatedTasksURL;
            URL.Login = strLogin;
            URL.Password = strPassword;
            URL.ViewTaskHotKey = Hotkey1;
            URL.ViewChildTasksHotKey = Hotkey2;
            URL.ViewParentTaskHotKey = Hotkey3;
            URL.ViewRelatedTasksHotKey = Hotkey4;
            URL.DefectsInSoftTest = (SendDlgItemMessage(IDC_ST_DEFECTS,BM_GETCHECK,0,0) == BST_CHECKED);
             // enabling FEATURE_HTTP_USERNAME_PASSWORD_DISABLE if needed
            if (!URL.Login.IsEmpty() && !URL.Password.IsEmpty())
            {
                if ((IExploreDefaultBrowser() && IE) || use_iexplore) // (Internet Explorer is default browser AND it is used to open tasks) OR Internet Explorer is used although it is not default
                {
                    // checking if FEATURE_HTTP_USERNAME_PASSWORD_DISABLE is disabled 
                    DWORD iexplore = 1; // assume that it is disabled
                    DWORD iexplore2 = 1; // assume that it is disabled
                    DWORD DWordSize = sizeof(DWORD);
                    Registry HTTP_USERNAME2(HKEY_CURRENT_USER);
                    bool r = HTTP_USERNAME2.ReadValue("SOFTWARE\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_HTTP_USERNAME_PASSWORD_DISABLE",
                                             "iexplore.exe",REG_DWORD,(LPBYTE)&iexplore2,DWordSize);
                    if (iexplore2 == 1) // either key is absent or "iexplore.exe" is equal to 1
                    {
                        DWordSize = sizeof(DWORD);
                        Registry HTTP_USERNAME(HKEY_LOCAL_MACHINE);
                        HTTP_USERNAME.ReadValue("SOFTWARE\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_HTTP_USERNAME_PASSWORD_DISABLE",
                                                "iexplore.exe",REG_DWORD,(LPBYTE)&iexplore,DWordSize);
                        if ((iexplore == 1) || ((iexplore == 0) && r)) // either key is absent or "iexplore.exe" is equal to 1
                        {
                            if (MyMessageBox(m_hWnd,"You have entered login and password to automatically login to the site,\nhowever, autologin feature is currently disabled for your browser.\n\nWould you like to enable autologin feature?",szWinName,MB_YESNO|MB_ICONQUESTION)==IDYES)
                            {
                                DWORD buf = 0;
                                HTTP_USERNAME2.AddValue("SOFTWARE\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_HTTP_USERNAME_PASSWORD_DISABLE",
                                                       "iexplore.exe",REG_DWORD,(const BYTE*)&buf,sizeof(DWORD));
                                HTTP_USERNAME2.AddValue("SOFTWARE\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_HTTP_USERNAME_PASSWORD_DISABLE",
                                                       "explorer.exe",REG_DWORD,(const BYTE*)&buf,sizeof(DWORD));
                            }
                        }
                    }
                }
            }
        }
        EndDialog(wID);
        return 0;
    }

    // checks if HotkeyToCheck entered to HotkeyField of current URL is already used in another URL
    // re-assigns the hotkey to current URL by user's request
    // HotkeyField: 1 - ViewTaskHotKey, 2 - ViewChildTasksHotKey, 3 - ViewParentTaskHotKey, 4 - ViewRelatedTasksHotKey
    // returns "false" if HotkeyToCheck is already used in another URL and user canceled re-assignment, otherwise returns "true"
    bool CheckAndReassignHotkey(const UINT HotkeyToCheck, const int HotkeyField, const unsigned int URLindex)
    {
        CString message = "";
        CString sHotkeyField = "";
        switch (HotkeyField)
        {
            case 1: // ViewTaskHotKey
                sHotkeyField = "HotKey to View Task";
                break;
            case 2: // ViewChildTasksHotKey
                sHotkeyField = "HotKey to View Child Tasks";
                break;
            case 3: // ViewParentTaskHotKey
                sHotkeyField = "HotKey to View Parent Task";
                break;
            case 4: // ViewRelatedTasksHotKey
                sHotkeyField = "HotKey to View Related Tasks";
                break;
        }
        if (HotkeyToCheck)
        {
            if (HotkeyToCheck == (*Links)[URLindex].ViewTaskHotKey)
            {
                message.Format("Hotkey entered in \"%s\" field is already used\nin \"%s\" URL as \"HotKey to View Task\".\nYou can re-assign entered hotkey to current URL or choose\nanother hotkey.\n\nWould you like to re-assign the hotkey?",sHotkeyField,(*Links)[URLindex].Caption);
                if (MyMessageBox(m_hWnd,message,szWinName,MB_ICONQUESTION|MB_YESNO)==IDYES)
                {
                    (*Links)[URLindex].ViewTaskHotKey = 0;
                }
                else
                {
                    return false;
                }
            }
            if (HotkeyToCheck == (*Links)[URLindex].ViewChildTasksHotKey)
            {
                message.Format("Hotkey entered in \"%s\" field is already used\nin \"%s\" URL as \"HotKey to View Child Tasks\".\nYou can re-assign entered hotkey to current URL or choose\nanother hotkey.\n\nWould you like to re-assign the hotkey?",sHotkeyField,(*Links)[URLindex].Caption);
                if (MyMessageBox(m_hWnd,message,szWinName,MB_ICONQUESTION|MB_YESNO)==IDYES)
                {
                    (*Links)[URLindex].ViewChildTasksHotKey = 0;
                }
                else
                {
                    return false;
                }
            }
            if (HotkeyToCheck == (*Links)[URLindex].ViewParentTaskHotKey)
            {
                message.Format("Hotkey entered in \"%s\" field is already used\nin \"%s\" URL as \"HotKey to View Parent Task\".\nYou can re-assign entered hotkey to current URL or choose\nanother hotkey.\n\nWould you like to re-assign the hotkey?",sHotkeyField,(*Links)[URLindex].Caption);
                if (MyMessageBox(m_hWnd,message,szWinName,MB_ICONQUESTION|MB_YESNO)==IDYES)
                {
                    (*Links)[URLindex].ViewParentTaskHotKey = 0;
                }
                else
                {
                    return false;
                }
            }
            if (HotkeyToCheck == (*Links)[URLindex].ViewRelatedTasksHotKey)
            {
                message.Format("Hotkey entered in \"%s\" field is already used\nin \"%s\" URL as \"HotKey to View Related Tasks\".\nYou can re-assign entered hotkey to current URL or choose\nanother hotkey.\n\nWould you like to re-assign the hotkey?",sHotkeyField,(*Links)[URLindex].Caption);
                if (MyMessageBox(m_hWnd,message,szWinName,MB_ICONQUESTION|MB_YESNO)==IDYES)
                {
                    (*Links)[URLindex].ViewRelatedTasksHotKey = 0;
                }
                else
                {
                    return false;
                }
            }
        }
        return true;
    }

    int GetPosByCaption(const char *caption)
    {
        for (unsigned int i=0; i<(*Links).size(); i++)
        {
            if (CompareNoCaseCP1251((*Links)[i].Caption,caption)==0)
            {
                return i;
            }
        }
        return -1;
    }
};

class URLsPage : public Mortimer::COptionPageImpl<URLsPage,CPropPage>
{
public:
    enum { IDD = URLS_PAGE };
    CListBox URLsList;
    CComboBox DefaultURL;
    CButton NewButton;
    CButton EditButton;
    CButton CopyButton;
    CButton DeleteButton;
    std::vector<link> temp_links;
    UINT *ForbiddenHotkey;
    UINT *uWinKey;
    CString *Browser;
    bool *CheckMark;

    BEGIN_MSG_MAP(URLsPage)
        MSG_WM_INITDIALOG(OnInitDialog)
        COMMAND_CODE_HANDLER_EX(CBN_SELCHANGE, OnSelChanged)
        COMMAND_ID_HANDLER_EX(IDC_LINK_NEW,OnNewLink)
        COMMAND_ID_HANDLER_EX(IDC_LINK_COPY,OnNewLink)
        COMMAND_ID_HANDLER_EX(IDC_LINK_EDIT,OnNewLink)
        COMMAND_ID_HANDLER_EX(IDC_LINK_DELETE,OnDeleteLink)
        COMMAND_ID_HANDLER_EX(IDC_URLS_LIST,OnListNotify)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    URLsPage()
    {
        ForbiddenHotkey = NULL;
        uWinKey = NULL;
        Browser = NULL;
        CheckMark = NULL;
    }

    int GetPosByCaption(const char *caption)
    {
        for (unsigned int i=0; i<temp_links.size(); i++)
        {
            if (CompareNoCaseCP1251(temp_links[i].Caption,caption)==0)
            {
                return i;
            }
        }
        return -1;
    }

    // called once when options dialog is opened
    LRESULT OnInitDialog(HWND hWnd, LPARAM lParam)
    {
        NewButton.Attach(GetDlgItem(IDC_LINK_NEW));
        EditButton.Attach(GetDlgItem(IDC_LINK_EDIT));
        CopyButton.Attach(GetDlgItem(IDC_LINK_COPY));
        DeleteButton.Attach(GetDlgItem(IDC_LINK_DELETE));

        URLsList.Attach(GetDlgItem(IDC_URLS_LIST));
        DefaultURL.Attach(GetDlgItem(IDC_URL_DEFAULT));

        UINT HotkeyID = 0;
        int Default = 0;
        for (unsigned int i=0; i<Settings.links.size(); i++)
        {
            UnregisterHotKey(::GetParent(GetParent()),HotkeyID);
            HotkeyID++;
            UnregisterHotKey(::GetParent(GetParent()),HotkeyID);
            HotkeyID++;
            UnregisterHotKey(::GetParent(GetParent()),HotkeyID);
            HotkeyID++;
            UnregisterHotKey(::GetParent(GetParent()),HotkeyID);
            HotkeyID++;
            temp_links.push_back(Settings.links[i]);
            if (Settings.links[i].Default)
            {
                Default = i;
            }
            URLsList.AddString(Settings.links[i].Caption);
            DefaultURL.AddString(Settings.links[i].Caption);
        }
        URLsList.SetCurSel(0);
        DefaultURL.SetCurSel(Default);
        return 0;
    }

    // called after selection in any ComboBox has been changed
    void OnSelChanged(UINT code, int ControlID, HWND ControlHandle)
    {
        if (ControlID != IDC_URL_DEFAULT) return;

        CComboBox ComboBox = ControlHandle;
        int position = ComboBox.GetCurSel();
        if (position == CB_ERR)
        {
            return;
        }
        for (unsigned int i=0; i<temp_links.size(); i++)
        {
            if (i == position)
            {
                temp_links[i].Default = true;
            }
            else
            {
                temp_links[i].Default = false;
            }
        }
    }

    void OnNewLink(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        int position, realpos;
        CString tmp = "";
        bool check = false;
        if (CheckMark != NULL)
        {
            check = *CheckMark;
        }
        if (Browser != NULL)
        {
            tmp = *Browser;
            tmp.MakeUpper();
        }
        URLEditPage URLEditDlg(0,link("","","","",0,0,0,0,false,"","",false),&temp_links,ForbiddenHotkey,uWinKey,(check || tmp.IsEmpty()), (tmp.Find("IEXPLORE")!=-1));

        if ((wID == IDC_LINK_COPY) || (wID == IDC_LINK_EDIT))
        {
            position = URLsList.GetCurSel();
            if (position == LB_ERR)
            {
                return;
            }
            CString SelectedItem;
            URLsList.GetText(position,SelectedItem);
            realpos = GetPosByCaption(SelectedItem);
            URLEditDlg.URL = temp_links[realpos];
            if (wID == IDC_LINK_COPY)
            {
                URLEditDlg.Action = 2;
            }
            else // wID == IDC_LINK_EDIT
            {
                URLEditDlg.Action = 1;
            }
        }

        if (URLEditDlg.DoModal() == IDOK)
        {
            if ((wID == IDC_LINK_NEW) || (wID == IDC_LINK_COPY))
            {
                URLsList.AddString(URLEditDlg.URL.Caption);
                DefaultURL.AddString(URLEditDlg.URL.Caption);
                temp_links.push_back(URLEditDlg.URL);
            }
            else // wID == IDC_LINK_EDIT
            {
                URLsList.DeleteString(position);
                URLsList.AddString(URLEditDlg.URL.Caption);
                int newpos = URLsList.FindStringExact(-1,URLEditDlg.URL.Caption);
                URLsList.SetCurSel(newpos);

                DefaultURL.DeleteString(position);
                DefaultURL.AddString(URLEditDlg.URL.Caption);
                if (temp_links[realpos].Default)
                {
                    DefaultURL.SetCurSel(newpos);
                }
                temp_links[realpos] = URLEditDlg.URL;
            }
            Settings.sort_links(temp_links);
        }
    }

    void OnListNotify(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        if (wNotifyCode == LBN_DBLCLK)
        {
            OnNewLink(0, IDC_LINK_EDIT, hWndCtl);
        }
    }

    void OnDeleteLink(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        if (URLsList.GetCount()==1)
        {
            MyMessageBox(m_hWnd,"At least one record should stay in the list.\nIt cannot be deleted",szWinName,MB_ICONERROR);
            return;
        }
        if (MyMessageBox(m_hWnd,"Are you sure you want to delete selected record?",szWinName,MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2) != IDYES)
        {
            return;
        }
        CString SelectedItem;
        int position = URLsList.GetCurSel();
        if (position == LB_ERR)
        {
            return;
        }
        else
        {
            URLsList.GetText(position,SelectedItem);
        }
        
        int realpos = GetPosByCaption(SelectedItem);
        if (realpos == -1) return;
        URLsList.DeleteString(position);
        DefaultURL.DeleteString(position);
        bool wasDefault = temp_links[realpos].Default;
        temp_links.erase(temp_links.begin()+realpos);

        if (wasDefault)
        {
            URLsList.GetText(0,SelectedItem);
            temp_links[GetPosByCaption(SelectedItem)].Default = true;
            MyMessageBox(m_hWnd,"You have just deleted the default record.\nThe first record will be marked as default",szWinName,MB_ICONINFORMATION);
            DefaultURL.SetCurSel(0);
        }
        URLsList.SetCurSel(0);
    }

    // called every time when the page is activated
    bool OnSetActive(COptionItem *pItem)
    {
        return true;
    }

    // called every time when the page is deactivated
    bool OnKillActive(COptionItem *pItem)
    {
        return true;
    }

    void OnOK()
    {
        URLsList.ResetContent();
        DefaultURL.ResetContent();
        Settings.links.clear();
        UINT HotkeyID = 0;
        for (unsigned int i=0; i<temp_links.size(); i++)
        {
            Settings.links.push_back(temp_links[i]);
            UINT HotKey = Settings.links[i].ViewTaskHotKey;
            if (HotKey)
            {
                if (!RegisterHotKey(::GetParent(GetParent()),HotkeyID,(!(HotKey&0x500)?
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
            HotKey = Settings.links[i].ViewChildTasksHotKey;
            if (HotKey)
            {
                if (!RegisterHotKey(::GetParent(GetParent()),HotkeyID,(!(HotKey&0x500)?
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
            HotKey = Settings.links[i].ViewParentTaskHotKey;
            if (HotKey)
            {
                if (!RegisterHotKey(::GetParent(GetParent()),HotkeyID,(!(HotKey&0x500)?
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
            HotKey = Settings.links[i].ViewRelatedTasksHotKey;
            if (HotKey)
            {
                if (!RegisterHotKey(::GetParent(GetParent()),HotkeyID,(!(HotKey&0x500)?
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

        temp_links.clear();

        Settings.SaveLinksSettings();
    }

    void OnCancel()
    {
        UINT HotkeyID = 0;
        for (unsigned int i=0; i<Settings.links.size(); i++)
        {
            UINT HotKey = Settings.links[i].ViewTaskHotKey;
            if (HotKey)
            {
                if (!RegisterHotKey(::GetParent(GetParent()),HotkeyID,(!(HotKey&0x500)?
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
            HotKey = Settings.links[i].ViewChildTasksHotKey;
            if (HotKey)
            {
                if (!RegisterHotKey(::GetParent(GetParent()),HotkeyID,(!(HotKey&0x500)?
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
            HotKey = Settings.links[i].ViewParentTaskHotKey;
            if (HotKey)
            {
                if (!RegisterHotKey(::GetParent(GetParent()),HotkeyID,(!(HotKey&0x500)?
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
            HotKey = Settings.links[i].ViewRelatedTasksHotKey;
            if (HotKey)
            {
                if (!RegisterHotKey(::GetParent(GetParent()),HotkeyID,(!(HotKey&0x500)?
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
    };
};

//////////////////////////// Defects options page ///////////////////////
class DefectsPage;

class DefectEditPage : public CDialogImpl<DefectEditPage>
{
public:
    enum { IDD = DEFECT_EDIT_PAGE };
    int Action; // 0 - new defect, 1 - edit defect, 2 - copy defect
    defect DEFECT;
    int MaxStringLength;
    std::vector<defect> *Defects;

    CEdit DefectEdit;
    CEdit ProjectEdit;
    CEdit Link;
    CEdit ChildLink;
    CEdit ParentLink;
    CEdit RelatedLink;

    DefectEditPage(int action, defect def, std::vector<defect> *defects):DEFECT(def)
    {
        Action = action;
        Defects = defects;
        MaxStringLength = 255;
    }

    BEGIN_MSG_MAP(DefectEditPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow();
        DefectEdit.Attach(GetDlgItem(IDC_CLIENT_EDIT));
        DefectEdit.LimitText(MaxStringLength);
        ProjectEdit.Attach(GetDlgItem(IDC_DEFECT_PROJECT));
        ProjectEdit.LimitText(MaxStringLength);
     
        Link.Attach(GetDlgItem(IDC_DEFECTS_LINK));
        Link.LimitText(LINK_MAX);
        ChildLink.Attach(GetDlgItem(IDC_DEFECTS_LINK2));
        ChildLink.LimitText(LINK_MAX);
        ParentLink.Attach(GetDlgItem(IDC_DEFECTS_LINK3));
        ParentLink.LimitText(LINK_MAX);
        RelatedLink.Attach(GetDlgItem(IDC_DEFECTS_LINK4));
        RelatedLink.LimitText(LINK_MAX);

        DefectEdit.SetWindowText(DEFECT.ClientID);
        ProjectEdit.SetWindowText(DEFECT.STProject);
        Link.SetWindowText(DEFECT.DefectURL);
        ChildLink.SetWindowText(DEFECT.ChildDefectsURL);
        ParentLink.SetWindowText(DEFECT.ParentDefectURL);
        RelatedLink.SetWindowText(DEFECT.RelatedDefectsURL);

        switch(Action)
        {
            case 0: // new
                SetWindowText("New defect record");
                DEFECT.ClientID = ";";
                break;
            case 1: // edit
                SetWindowText("Edit defect record");
                break;
            case 2: // copy
                SetWindowText("New defect record");
                DEFECT.ClientID = ";";
                break;
        }
        return 0;
    }

    int GetPosByCaption(const char *caption)
    {
        for (unsigned int i=0; i<(*Defects).size(); i++)
        {
            if (CompareNoCaseCP1251((*Defects)[i].ClientID,caption)==0)
            {
                return i;
            }
        }
        return -1;
    }

    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        if (wID == IDOK)
        {
            // checking for duplicates and non-empty Project
            CString strDefect, strProject;
            DefectEdit.GetWindowText(strDefect.GetBuffer(MaxStringLength+1),MaxStringLength+1);
            strDefect.ReleaseBuffer();
            ProjectEdit.GetWindowText(strProject.GetBuffer(MaxStringLength+1),MaxStringLength+1);
            strProject.ReleaseBuffer();
        
            if (!strDefect.IsEmpty() && !TASK::IsClientNameValid(strDefect))
            {
                MyMessageBox(m_hWnd,"Client name contains invalid character(s)",szWinName,MB_ICONERROR);
                return 0;
            }
            if (GetPosByCaption(strDefect) != -1) // ClientID is not unique
            {
                if (CompareNoCaseCP1251(DEFECT.ClientID,strDefect) != 0)
                {
                    MyMessageBox(m_hWnd,"Entered client name is not unique.\nPlease use another one",szWinName,MB_ICONERROR);
                    return 0;
                }
            }
            if (strProject.IsEmpty())
            {
                MyMessageBox(m_hWnd,"Project field must not be empty.",szWinName,MB_ICONERROR);
                return 0;
            }
            if (strProject.Find(';') != -1)
            {
                MyMessageBox(m_hWnd,"Character \';\' cannot be used as a part of Project name",szWinName,MB_ICONERROR);
                return 0;
            }
            CString DefectURL = "", ChildDefectsURL = "", ParentDefectURL = "", RelatedDefectsURL = "";
            Link.GetWindowText(DefectURL.GetBuffer(LINK_MAX+1),LINK_MAX+1);
            DefectURL.ReleaseBuffer();
            ChildLink.GetWindowText(ChildDefectsURL.GetBuffer(LINK_MAX+1),LINK_MAX+1);
            ChildDefectsURL.ReleaseBuffer();
            ParentLink.GetWindowText(ParentDefectURL.GetBuffer(LINK_MAX+1),LINK_MAX+1);
            ParentDefectURL.ReleaseBuffer();
            RelatedLink.GetWindowText(RelatedDefectsURL.GetBuffer(LINK_MAX+1),LINK_MAX+1);
            RelatedDefectsURL.ReleaseBuffer();
            bool Default = false;
            if (DefectURL.IsEmpty())
            {
                DefectURL = Settings.DefectsLink;
                Default = true;
            }
            else
            {
                if (DefectURL.Find(';') != -1)
                {
                    MyMessageBox(m_hWnd,"Character \';\' cannot be used as a part of an URL\nPlease correct \"URL to open defects\"",szWinName,MB_ICONERROR);
                    return 0;
                }
            }
            if (ChildDefectsURL.IsEmpty())
            {
                ChildDefectsURL = Settings.ChildDefectsLink;
                Default = true;
            }
            else
            {
                if (ChildDefectsURL.Find(';') != -1)
                {
                    MyMessageBox(m_hWnd,"Character \';\' cannot be used as a part of an URL\nPlease correct \"URL to open child defects\"",szWinName,MB_ICONERROR);
                    return 0;
                }
            }
            if (ParentDefectURL.IsEmpty())
            {
                ParentDefectURL = Settings.ParentDefectLink;
                Default = true;
            }
            else
            {
                if (ParentDefectURL.Find(';') != -1)
                {
                    MyMessageBox(m_hWnd,"Character \';\' cannot be used as a part of an URL.\nPlease correct \"URL to open parent defects\"",szWinName,MB_ICONERROR);
                    return 0;
                }
            }
            if (RelatedDefectsURL.IsEmpty())
            {
                RelatedDefectsURL = Settings.RelatedDefectsLink;
                Default = true;
            }
            else
            {
                if (RelatedDefectsURL.Find(';') != -1)
                {
                    MyMessageBox(m_hWnd,"Character \';\' cannot be used as a part of an URL.\nPlease correct \"URL to open related defects\"",szWinName,MB_ICONERROR);
                    return 0;
                }
            }
            if (Default)
            {
                MyMessageBox(m_hWnd,"Some URLs were not entered. Default values will be used",szWinName,MB_ICONINFORMATION);
            }

            DEFECT.ClientID = strDefect;
            DEFECT.STProject = strProject;
            DEFECT.DefectURL = DefectURL;
            DEFECT.ChildDefectsURL = ChildDefectsURL;
            DEFECT.ParentDefectURL = ParentDefectURL;
            DEFECT.RelatedDefectsURL = RelatedDefectsURL;
        }
        EndDialog(wID);
        return 0;
    }
};

class SoftTestPage : public Mortimer::COptionPageImpl<SoftTestPage,CPropPage>
{
public:
    enum { IDD = SOFTTEST_PAGE };
    CEdit SoftTestPath;
    CEdit SoftTestFilter;
    CEdit SoftTestLogin;
    CEdit SoftTestPassword;

    BEGIN_MSG_MAP(SoftTestPage)
        MSG_WM_INITDIALOG(OnInitDialog)
        COMMAND_ID_HANDLER_EX(IDC_SOFTTEST_BROWSE,OnBrowse)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    // called once when options dialog is opened
    LRESULT OnInitDialog(HWND hWnd, LPARAM lParam)
    {
        SoftTestPath.Attach(GetDlgItem(IDC_SOFTTEST_PATH));
        SoftTestPath.LimitText(MAX_PATH);
        SoftTestPath.SetWindowText(Settings.SoftTestPath);

        SoftTestFilter.Attach(GetDlgItem(IDC_SOFTTEST_FILTER));
        SoftTestFilter.LimitText(MAX_PATH);
        SoftTestFilter.SetWindowText(Settings.SoftTestFilterName);

        SoftTestLogin.Attach(GetDlgItem(IDC_SOFTTEST_LOGIN));
        SoftTestLogin.LimitText(1024);
        SoftTestLogin.SetWindowText(Settings.SoftTestLogin);

        SoftTestPassword.Attach(GetDlgItem(IDC_SOFTTEST_PASSWORD));
        SoftTestPassword.LimitText(1024);
        SoftTestPassword.SetWindowText(Settings.SoftTestPassword);

        return 0;
    }

    void OnBrowse(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        CFileDialog Browser(TRUE,NULL,NULL,OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES|OFN_PATHMUSTEXIST,
                            "Programs\0*.exe");
        CString temp = szWinName;
        temp += " - Select SoftTest executable";
        Browser.m_ofn.lpstrTitle = temp;
        Browser.DoModal();
        if (lstrcmp(Browser.m_szFileName,"")!=0)
        {
            SoftTestPath.SetWindowText(Browser.m_szFileName);
        }
    }

    bool OnSetActive(COptionItem *pItem)
    {
        return true;
    }
    
    bool OnKillActive(COptionItem *pItem)
    {
        CString temp = "";
        // checking for correct Path to SoftTest value
        SoftTestPath.GetWindowText(temp.GetBuffer(MAX_PATH+1),MAX_PATH+1);
        temp.ReleaseBuffer();
        temp.TrimLeft();
        temp.TrimRight();
        if (temp.IsEmpty())
        {
            MyMessageBox(m_hWnd,"\"Path to SoftTest\" field must not be empty.\nPlease fill it with correct value",szWinName,MB_ICONERROR);
            return false;
        }
        // checking for correct SoftTest filter name value
        temp = "";
        SoftTestFilter.GetWindowText(temp.GetBuffer(1025),1025);
        temp.ReleaseBuffer();
        if (temp.IsEmpty())
        {
            MyMessageBox(m_hWnd,"\"SoftTest filter name\" field must not be empty.\nPlease fill it with correct value",szWinName,MB_ICONERROR);
            return false;
        }
        if (temp.Find("%PROJECT%") == -1)
        {
            MyMessageBox(m_hWnd,"%PROJECT% variable must not be removed\nfrom filter name. Please add it to filter name",szWinName,MB_ICONERROR);
            return false;
        }
        CString login = "";
        SoftTestLogin.GetWindowText(login.GetBuffer(1025),1025);
        login.ReleaseBuffer();
        if (login.IsEmpty())
        {
            MyMessageBox(m_hWnd,"\"Login\" field must not be empty.\nPlease fill it with correct value",szWinName,MB_ICONERROR);
            return false;
        }
        CString password = "";
        SoftTestPassword.GetWindowText(password.GetBuffer(1025),1025);
        password.ReleaseBuffer();
        if (password.IsEmpty() && login.Compare("guest")!=0)
        {
            MyMessageBox(m_hWnd,"\"Password\" field must not be empty unless login is \"guest\".\nPlease fill it with correct value",szWinName,MB_ICONERROR);
            return false;
        }
        return true;
    }

    void OnOK()
    {
        // saving SoftTest settings
        SoftTestPath.GetWindowText(Settings.SoftTestPath.GetBuffer(MAX_PATH+1),MAX_PATH+1);
        Settings.SoftTestPath.ReleaseBuffer();
        Settings.SoftTestPath.TrimLeft();
        Settings.SoftTestPath.TrimRight();

        SoftTestFilter.GetWindowText(Settings.SoftTestFilterName.GetBuffer(1025),1025);
        Settings.SoftTestFilterName.ReleaseBuffer();

        SoftTestLogin.GetWindowText(Settings.SoftTestLogin.GetBuffer(1025),1025);
        Settings.SoftTestLogin.ReleaseBuffer();

        SoftTestPassword.GetWindowText(Settings.SoftTestPassword.GetBuffer(1025),1025);
        Settings.SoftTestPassword.ReleaseBuffer();

        Settings.SaveSoftTestSettings();
    }

    // called every time when whole sheet is closed by clicking on Cancel button
    void OnCancel() {}
};

class DefectsPage : public Mortimer::COptionPageImpl<DefectsPage,CPropPage>
{
public:
    enum { IDD = DEFECTS_PAGE };

    CListBox DefectsList;
    CButton NewButton;
    CButton EditButton;
    CButton CopyButton;
    CButton DeleteButton;
    std::vector<defect> projects;
    
    BEGIN_MSG_MAP(DefectsPage)
        MSG_WM_INITDIALOG(OnInitDialog)
        COMMAND_ID_HANDLER_EX(IDC_DEFECT_NEW,OnNewDefect)
        COMMAND_ID_HANDLER_EX(IDC_DEFECT_EDIT,OnNewDefect)
        COMMAND_ID_HANDLER_EX(IDC_DEFECT_COPY,OnNewDefect)
        COMMAND_ID_HANDLER_EX(IDC_DEFECT_DELETE,OnDeleteDefect)
        COMMAND_ID_HANDLER_EX(IDC_DEFECTS_LIST,OnListNotify)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    void sort_defects()
    {
        for (unsigned int i=0; i<projects.size()-1; i++)
        {
            bool exchange = false;
            for (unsigned int j=1; j<projects.size()-i; j++)
            {
                if (CompareNoCaseCP1251(projects[j-1].ClientID,projects[j].ClientID)>0)
                {
                    defect temp = projects[j-1];
                    projects[j-1] = projects[j];
                    projects[j] = temp;
                    exchange = true;
                }
            }
            if (!exchange) break;
        }
    }

    int GetPosByCaption(const char *caption)
    {
        for (unsigned int i=0; i<projects.size(); i++)
        {
            if (CompareNoCaseCP1251(projects[i].ClientID,caption)==0)
            {
                return i;
            }
        }
        return -1;
    }

    // called once when options dialog is opened
    LRESULT OnInitDialog(HWND hWnd, LPARAM lParam)
    {
        NewButton.Attach(GetDlgItem(IDC_DEFECT_NEW));
        EditButton.Attach(GetDlgItem(IDC_DEFECT_EDIT));
        CopyButton.Attach(GetDlgItem(IDC_DEFECT_COPY));
        DeleteButton.Attach(GetDlgItem(IDC_DEFECT_DELETE));

        DefectsList.Attach(GetDlgItem(IDC_DEFECTS_LIST));

        for (unsigned int i=0; i<Settings.defects.size(); i++)
        {
            DefectsList.AddString(Settings.defects[i].ClientID);
            projects.push_back(Settings.defects[i]);
        }

        DefectsList.SetCurSel(0);

        return 0;
    }

    void OnNewDefect(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        int position, realpos;
        DefectEditPage DefectEditDlg(0,defect("","","","","",""),&projects);

        if ((wID == IDC_DEFECT_COPY) || (wID == IDC_DEFECT_EDIT))
        {
            position = DefectsList.GetCurSel();
            if (position == LB_ERR)
            {
                return;
            }
            CString SelectedItem;
            DefectsList.GetText(position,SelectedItem);
            realpos = GetPosByCaption(SelectedItem);
            DefectEditDlg.DEFECT = projects[realpos];
            if (wID == IDC_DEFECT_COPY)
            {
                DefectEditDlg.Action = 2;
            }
            else // wID == IDC_DEFECT_EDIT
            {
                DefectEditDlg.Action = 1;
            }
        }

        if (DefectEditDlg.DoModal() == IDOK)
        {
            if ((wID == IDC_DEFECT_NEW) || (wID == IDC_DEFECT_COPY))
            {
                DefectsList.AddString(DefectEditDlg.DEFECT.ClientID);
                projects.push_back(DefectEditDlg.DEFECT);
            }
            else // wID == IDC_DEFECT_EDIT
            {
                DefectsList.DeleteString(position);
                DefectsList.AddString(DefectEditDlg.DEFECT.ClientID);
                int newpos = DefectsList.FindStringExact(-1,DefectEditDlg.DEFECT.ClientID);
                if ((newpos == -1) && (DefectEditDlg.DEFECT.ClientID.IsEmpty()))
                {
                    newpos = 0;
                }
                DefectsList.SetCurSel(newpos);

                projects[realpos] = DefectEditDlg.DEFECT;
            }
        }
    }

    void OnDeleteDefect(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        int position = DefectsList.GetCurSel();
        CString SelectedItem;
        if (position == LB_ERR)
        {
            return;
        }
        else
        {
            DefectsList.GetText(position,SelectedItem);
        }
        if (MyMessageBox(m_hWnd,"Are you sure you want to delete selected record?",szWinName,MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2) != IDYES)
        {
            return;
        }
        int realpos = GetPosByCaption(SelectedItem);
        if (realpos != -1)
        {
            DefectsList.DeleteString(position);
            projects.erase(projects.begin()+realpos);
        }
        DefectsList.SetCurSel(0);
    }

    void OnListNotify(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        if (wNotifyCode == LBN_DBLCLK)
        {
            OnNewDefect(0, IDC_DEFECT_EDIT, hWndCtl);
        }
    }

    bool OnSetActive(COptionItem *pItem)
    {
        return true;
    }
    
    bool OnKillActive(COptionItem *pItem)
    {
        return true;
    }

    void OnOK()
    {
        // saving defects settings
        sort_defects();
        DefectsList.ResetContent();
        Settings.defects.clear();
        for (unsigned int i=0; i<projects.size(); i++)
        {
            Settings.defects.push_back(projects[i]);
        }
        projects.clear();

        Settings.SaveDefectsSettings();
    }

    // called every time when whole sheet is closed by clicking on Cancel button
    void OnCancel() {}
};

//////////////////////////// History options page //////////////////////
class HistoryPage : public Mortimer::COptionPageImpl<HistoryPage,CPropPage>
{
public:
    enum { IDD = HISTORY_PAGE };
    CComboBox TaskNameCombo;
    CMyListBox HistoryList;
    CButton ViewButton;
    CButton CopyButton;
    CButton DeleteButton;
    CButton ClearButton;

    BEGIN_MSG_MAP(HistoryPage)
        MSG_WM_INITDIALOG(OnInitDialog)
        COMMAND_ID_HANDLER_EX(IDC_BUTTON_VIEW,OnViewButton)
        COMMAND_ID_HANDLER_EX(IDC_BUTTON_COPY,OnCopyButton)
        COMMAND_ID_HANDLER_EX(IDC_BUTTON_DELETE,OnItemDelete)
        COMMAND_ID_HANDLER_EX(IDC_BUTTON_CLEAR,OnListClear)
        COMMAND_CODE_HANDLER_EX(LBN_DBLCLK,OnListNotify)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    LRESULT OnInitDialog(HWND hWnd, LPARAM lParam)
    {
        HistoryList.Attach(GetDlgItem(IDC_HISTORY_LIST));
        ViewButton.Attach(GetDlgItem(IDC_BUTTON_VIEW));
        CopyButton.Attach(GetDlgItem(IDC_BUTTON_COPY));
        DeleteButton.Attach(GetDlgItem(IDC_BUTTON_DELETE));
        ClearButton.Attach(GetDlgItem(IDC_BUTTON_CLEAR));
        // copying tasks from Combobox to List
        TaskNameCombo = ::GetDlgItem(::GetParent(GetParent()),IDC_TASKNAME_COMBO);
        for (int i=0; i < TaskNameCombo.GetCount(); i++)
        {
            CString Item = "";
            TaskNameCombo.GetLBText(i,Item);
            HistoryList.AddString(Item);
        }
        if (HistoryList.GetCount() == 0)
        {
            ViewButton.EnableWindow(false);
            CopyButton.EnableWindow(false);
            DeleteButton.EnableWindow(false);
            ClearButton.EnableWindow(false);
        }
        else
        {
            HistoryList.SetSel(0);
        }
        // set minimum and maximum positions of spin button
        SendMessage(GetDlgItem(IDC_MAX_SPIN),(UINT)UDM_SETRANGE,0,
                    (LPARAM)MAKELONG((short)Settings.MaxPossibleHistory,(short)0));
        // set current position of spin button
        SendMessage(GetDlgItem(IDC_MAX_SPIN),(UINT)UDM_SETPOS,0,
                    (LPARAM) MAKELONG ((short)Settings.MaxHistoryItems, 0));
        return 0;
    }

    // called every time when whole sheet is closed by clicking on OK button
    void OnOK()
    {
        Settings.MaxHistoryItems = GetDlgItemInt(IDC_MAX_HISTORY);
        int items = HistoryList.GetCount(); // actual number of items in HistoryList control
        if (Settings.MaxHistoryItems == 0)
        {
            HistoryList.ResetContent();
        }
        else
        {
            for (int i = Settings.MaxHistoryItems; i < items; i++)
            {
                HistoryList.DeleteString(Settings.MaxHistoryItems);
            }
        }
        // copy tasks from list to combobox
        CString Task = "";
        UINT TaskNameLength = TaskNameCombo.GetWindowTextLength();
        if (TaskNameLength > 0)
        {
            ::GetWindowText(TaskNameCombo,Task.GetBuffer(TaskNameLength+1),TaskNameLength+1);
            Task.ReleaseBuffer();
        }
        TaskNameCombo.ResetContent();
        for (int i = 0; i < HistoryList.GetCount(); i++)
        {
            CString Item = "";
            HistoryList.GetText(i,Item);
            TaskNameCombo.AddString(Item);
        }
        HistoryList.ResetContent();
        if (!Task.IsEmpty())
        {
            TaskNameCombo.SetWindowText(Task);
        }
    }

    // called every time when whole sheet is closed by clicking on Cancel button
    void OnCancel() {}
 
    void OnViewButton(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        if (FillHistoryTasks() && (Settings.MainDialog != NULL))
        {
            Settings.MainDialog->OnViewTask(0,IDC_BUTTON_VIEW,0);
        }
    }

    void OnListNotify(UINT code, int ControlID, HWND ControlHandle)
    {
        if ((code == LBN_DBLCLK) && (ControlID == IDC_HISTORY_LIST))
        {
            if (FillHistoryTasks() && (Settings.MainDialog != NULL))
            {
                Settings.MainDialog->OnViewTask(0,IDC_BUTTON_VIEW,0);
            }
        }
    }

    bool FillHistoryTasks()
    {
        std::vector<CString> SelItems;
        bool res = HistoryList.GetSelectedItems(SelItems);
        if (res)
        {
            Settings.HistoryTasks = "";
            for (unsigned int i = 0; i < SelItems.size(); i++)
            {
                Settings.HistoryTasks += SelItems[i] + Settings.TasksSeparators[0];
            }
            if (!Settings.HistoryTasks.IsEmpty())
            {
                Settings.HistoryTasks.Delete(Settings.HistoryTasks.GetLength()-1, 1);
            }
        }
        return res;
    }

    void OnCopyButton(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        if (!FillHistoryTasks())
        {
            return;
        }
        Settings.HistoryTasks.Replace((CString)Settings.TasksSeparators[0], "\r\n");
        if (!::OpenClipboard(m_hWnd)) return;
        if (!::EmptyClipboard())
        {
            CloseClipboard();
            return;
        }
        HGLOBAL hglb = GlobalAlloc(GMEM_MOVEABLE,Settings.HistoryTasks.GetLength()+1);
        if (hglb == NULL)
        {
            CloseClipboard();
            return;
        }
        LPTSTR lptstr = (LPTSTR)GlobalLock(hglb); 
        memcpy(lptstr,Settings.HistoryTasks.GetBuffer(Settings.HistoryTasks.GetLength()),Settings.HistoryTasks.GetLength());
        lptstr[Settings.HistoryTasks.GetLength()] = '\0';
        Settings.HistoryTasks.ReleaseBuffer();
        GlobalUnlock(hglb);
        SetClipboardData(CF_TEXT, hglb);
        CloseClipboard();
    }

    void OnItemDelete(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        HistoryList.DeleteSelectedItems();
        if (HistoryList.GetCount() == 0)
        {
            ViewButton.EnableWindow(false);
            CopyButton.EnableWindow(false);
            DeleteButton.EnableWindow(false);
            ClearButton.EnableWindow(false);
        }
        else
        {
            HistoryList.SetSel(0);
        }
    }

    void OnListClear(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        if (MyMessageBox(m_hWnd,"Are you sure you want to clear the whole list?",
                       szWinName,MB_YESNO|MB_ICONQUESTION)==IDYES)
        {
            HistoryList.ResetContent();
            ViewButton.EnableWindow(false);
            CopyButton.EnableWindow(false);
            DeleteButton.EnableWindow(false);
            ClearButton.EnableWindow(false);
        }
    }
};

//////////////////////////// Other options page //////////////////////
class OtherPage : public Mortimer::COptionPageImpl<OtherPage,CPropPage>
{
public:
    enum { IDD = OTHER_PAGE };
    CTrackBarCtrl InactiveTrackBar;
    CTrackBarCtrl ActiveTrackBar;
    CStatic InactivePercentage;
    CStatic ActivePercentage;
    std::vector<link> *Links;
    UINT HotKey, WinKey;
    bool Highlight;

    BEGIN_MSG_MAP(OtherPage)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_HSCROLL(OnSliderMove)
        COMMAND_ID_HANDLER_EX(IDC_ENABLE_OPACITY,OnCheckBoxClick)
        COMMAND_ID_HANDLER_EX(IDC_SETFOCUS,OnCheckBoxClick)
        COMMAND_ID_HANDLER_EX(IDC_HIGHLIGHT,OnCheckBoxClick)
    END_MSG_MAP()

    OtherPage()
    {
        Links = NULL;
        HotKey = Settings.HotKey;
        WinKey = Settings.WinKey;
        Highlight = Settings.HighlightTaskName;
    }

    bool IsHotkeyUsed(UINT Hotkey)
    {
        if (Links == NULL)
        {
            return false;
        }
        for (unsigned int i=0; i<(*Links).size(); i++)
        {
            if (Hotkey)
            {
                CString message = "";
                if (Hotkey == (*Links)[i].ViewTaskHotKey)
                {
                    message.Format("Hotkey entered for fast window activation is already used\nin \"%s\" URL as \"HotKey to View Task\".\nYou can re-assign entered hotkey for fast window activation\nor choose another hotkey.\n\nWould you like to re-assign the hotkey?",(*Links)[i].Caption);
                    if (MyMessageBox(m_hWnd,message,szWinName,MB_ICONQUESTION|MB_YESNO) == IDYES)
                    {
                        (*Links)[i].ViewTaskHotKey = 0;
                    }
                    else
                    {
                        return true;
                    }
                }
                if (Hotkey == (*Links)[i].ViewChildTasksHotKey)
                {
                    message.Format("Hotkey entered for fast window activation is already used\nin \"%s\" URL as \"HotKey to View Child Tasks\".\nYou can re-assign entered hotkey for fast window activation\nor choose another hotkey.\n\nWould you like to re-assign the hotkey?",(*Links)[i].Caption);
                    if (MyMessageBox(m_hWnd,message,szWinName,MB_ICONQUESTION|MB_YESNO) == IDYES)
                    {
                        (*Links)[i].ViewChildTasksHotKey = 0;
                    }
                    else
                    {
                        return true;
                    }
                }
                if (Hotkey == (*Links)[i].ViewParentTaskHotKey)
                {
                    message.Format("Hotkey entered for fast window activation is already used\nin \"%s\" URL as \"HotKey to View Parent Task\".\nYou can re-assign entered hotkey for fast window activation\nor choose another hotkey.\n\nWould you like to re-assign the hotkey?",(*Links)[i].Caption);
                    if (MyMessageBox(m_hWnd,message,szWinName,MB_ICONQUESTION|MB_YESNO) == IDYES)
                    {
                        (*Links)[i].ViewParentTaskHotKey = 0;
                    }
                    else
                    {
                        return true;
                    }
                }
                if (Hotkey == (*Links)[i].ViewRelatedTasksHotKey)
                {
                    message.Format("Hotkey entered for fast window activation is already used\nin \"%s\" URL as \"HotKey to View Related Tasks\".\nYou can re-assign entered hotkey for fast window activation\nor choose another hotkey.\n\nWould you like to re-assign the hotkey?",(*Links)[i].Caption);
                    if (MyMessageBox(m_hWnd,message,szWinName,MB_ICONQUESTION|MB_YESNO) == IDYES)
                    {
                        (*Links)[i].ViewRelatedTasksHotKey = 0;
                    }
                    else
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    void EnableDisableControls(bool enable)
    {
        if (!enable)
        {
            ::EnableWindow(GetDlgItem(IDC_INACTIVE_STATIC),FALSE);
            ::EnableWindow(GetDlgItem(IDC_ACTIVE_STATIC),FALSE);
            ::EnableWindow(GetDlgItem(IDC_INACTIVE_SLIDER),FALSE);
            ::EnableWindow(GetDlgItem(IDC_ACTIVE_SLIDER),FALSE);
            ::EnableWindow(GetDlgItem(IDC_INOP_PER),FALSE);
            ::EnableWindow(GetDlgItem(IDC_AOP_PER),FALSE);            
        }
        else
        {
            ::EnableWindow(GetDlgItem(IDC_INACTIVE_STATIC),TRUE);
            ::EnableWindow(GetDlgItem(IDC_ACTIVE_STATIC),TRUE);
            ::EnableWindow(GetDlgItem(IDC_INACTIVE_SLIDER),TRUE);
            ::EnableWindow(GetDlgItem(IDC_ACTIVE_SLIDER),TRUE);
            ::EnableWindow(GetDlgItem(IDC_INOP_PER),TRUE);
            ::EnableWindow(GetDlgItem(IDC_AOP_PER),TRUE);            
        }
    }

    void OnCheckBoxClick(UINT code, int idFrom, HWND hwndFrom)
    {
        switch (idFrom)
        {
            case IDC_ENABLE_OPACITY:
                EnableDisableControls(SendDlgItemMessage(IDC_ENABLE_OPACITY,BM_GETCHECK,0,0) == BST_CHECKED);
                break;
            case IDC_SETFOCUS:
                if (SendDlgItemMessage(IDC_SETFOCUS,BM_GETCHECK,0,0)==BST_CHECKED)
                {
                    ::EnableWindow(GetDlgItem(IDC_HIGHLIGHT),TRUE);
                    SendDlgItemMessage(IDC_HIGHLIGHT,BM_SETCHECK,Highlight ? BST_CHECKED : BST_UNCHECKED,0);
                }
                else
                {
                    ::EnableWindow(GetDlgItem(IDC_HIGHLIGHT),FALSE);
                    SendDlgItemMessage(IDC_HIGHLIGHT,BM_SETCHECK,BST_UNCHECKED,0);
                }
                break;
            case IDC_HIGHLIGHT:
                Highlight = (SendDlgItemMessage(IDC_HIGHLIGHT,BM_GETCHECK,0,0)==BST_CHECKED);
                break;
        }
    }

    LRESULT OnSliderMove(int code, short pos, HWND hwndFrom)
    {
        CString str =  "";
        switch (::GetDlgCtrlID(hwndFrom))
        {
            case IDC_INACTIVE_SLIDER:
                str.Format("%d",InactiveTrackBar.GetPos());
                str += "%";
                InactivePercentage.SetWindowText(str);
                break;
            case IDC_ACTIVE_SLIDER:
                str.Format("%d",ActiveTrackBar.GetPos());
                str += "%";
                ActivePercentage.SetWindowText(str);
                break;
        }
        return 0;
    }

    LRESULT OnInitDialog(HWND hWnd, LPARAM lParam)
    {
        InactiveTrackBar.Attach(GetDlgItem(IDC_INACTIVE_SLIDER));
        ActiveTrackBar.Attach(GetDlgItem(IDC_ACTIVE_SLIDER));
        InactivePercentage.Attach(GetDlgItem(IDC_INOP_PER));
        ActivePercentage.Attach(GetDlgItem(IDC_AOP_PER));

        InactiveTrackBar.SetRange(0, 100, TRUE);
        InactiveTrackBar.SetPos(Settings.InactiveOpacity);
        InactiveTrackBar.SetPageSize(10);

        ActiveTrackBar.SetRange(0, 100, TRUE);
        ActiveTrackBar.SetPos(Settings.ActiveOpacity);
        ActiveTrackBar.SetPageSize(10);

        CString str =  "";
        str.Format("%d",Settings.InactiveOpacity);
        str += "%";
        InactivePercentage.SetWindowText(str);
        str.Format("%d",Settings.ActiveOpacity);
        str += "%";
        ActivePercentage.SetWindowText(str);
        
        if (CMainDlg::OsMajorVer()<5) // OS is lower than Win2000
        {
            ::EnableWindow(GetDlgItem(IDC_ENABLE_OPACITY),FALSE);
            EnableDisableControls(false);
        }
        else
        {
            if (Settings.EnableOpacity)
            {
                SendDlgItemMessage(IDC_ENABLE_OPACITY,BM_SETCHECK,BST_CHECKED,0);
            }
            EnableDisableControls(Settings.EnableOpacity);
        }
        SendDlgItemMessage(IDC_HOTKEY,HKM_SETHOTKEY,HotKey,0);
        if (WinKey == MOD_WIN)
        {
            SendDlgItemMessage(IDC_WINKEY,BM_SETCHECK,BST_CHECKED,0);
        }
        UnregisterHotKey(::GetParent(GetParent()),Settings.GlobalHotkeyID);

        if (Settings.SetFocusToTaskName)
        {
            SendDlgItemMessage(IDC_SETFOCUS,BM_SETCHECK,BST_CHECKED,0);
            if (Highlight)
            {
                SendDlgItemMessage(IDC_HIGHLIGHT,BM_SETCHECK,BST_CHECKED,0);
            }
        }
        else
        {
            ::EnableWindow(GetDlgItem(IDC_HIGHLIGHT),FALSE);
        }
        return 0;
    }

    // called every time when the page is activated
    bool OnSetActive(COptionItem *pItem)
    {
        SendDlgItemMessage(IDC_HOTKEY,HKM_SETHOTKEY,HotKey,0);
        return true;
    }

    // called every time when the page is deactivated
    bool OnKillActive(COptionItem *pItem)
    {
        WinKey = (SendDlgItemMessage(IDC_WINKEY,BM_GETCHECK,0,0)==BST_CHECKED) ? MOD_WIN : 0;
        HotKey = (UINT)SendDlgItemMessage(IDC_HOTKEY,HKM_GETHOTKEY,0,0);
        if (HotKey || WinKey)
        {
            if (!RegisterHotKey(::GetParent(GetParent()),Settings.GlobalHotkeyID,WinKey|(!(HotKey&0x500)?HIBYTE(LOWORD(HotKey)):
            ((HotKey&0x500)<0x500?HIBYTE(LOWORD(HotKey))^5:HIBYTE(LOWORD(HotKey)))),LOBYTE(LOWORD(HotKey)))) 
            {
                MyMessageBox(m_hWnd,"The hotkey is already registered by another program.\nPlease enter another hotkey",szWinName,MB_ICONERROR);
                return false;
            }
            else
            {
                UnregisterHotKey(::GetParent(GetParent()),Settings.GlobalHotkeyID);
            }
        }
        if (HotKey && !WinKey)
        {
            return !IsHotkeyUsed(HotKey);
        }
        return true;
    }

    // called every time when whole sheet is closed by clicking on Cancel button
    void OnCancel()
    {
        if (Settings.HotKey || Settings.WinKey)
        {
            if (!RegisterHotKey(::GetParent(GetParent()),Settings.GlobalHotkeyID,Settings.WinKey|(!(Settings.HotKey&0x500)?HIBYTE(LOWORD(Settings.HotKey)):
            ((Settings.HotKey&0x500)<0x500?HIBYTE(LOWORD(Settings.HotKey))^5:HIBYTE(LOWORD(Settings.HotKey)))),LOBYTE(LOWORD(Settings.HotKey)))) 
            {
                MyMessageBox(m_hWnd,"The hotkey entered on \"Other\" page is already registered and will not work as expected.\nPlease enter another hotkey in TMS Launcher Settings window",szWinName,MB_ICONERROR);
            }
        }
     }

    // called every time when whole sheet is closed by clicking on OK button
    void OnOK()
    {
        WinKey = (SendDlgItemMessage(IDC_WINKEY,BM_GETCHECK,0,0)==BST_CHECKED) ? MOD_WIN : 0;
        HotKey = (UINT)SendDlgItemMessage(IDC_HOTKEY,HKM_GETHOTKEY,0,0);
        if (HotKey)
        {
            if (!RegisterHotKey(::GetParent(GetParent()),Settings.GlobalHotkeyID,WinKey|(!(HotKey&0x500)?HIBYTE(LOWORD(HotKey)):
            ((HotKey&0x500)<0x500?HIBYTE(LOWORD(HotKey))^5:HIBYTE(LOWORD(HotKey)))),LOBYTE(LOWORD(HotKey)))) 
            {
                MyMessageBox(m_hWnd,"The hotkey entered on \"Other\" page is already registered and will not work as expected.\nPlease enter another hotkey in TMS Launcher Settings window",szWinName,MB_ICONERROR);
            }
            else
            {
                Settings.HotKey = HotKey;
                Settings.WinKey = WinKey;
            }
        }
        else
        {
            Settings.HotKey = HotKey;
            Settings.WinKey = WinKey;
        }

        Settings.EnableOpacity = (SendDlgItemMessage(IDC_ENABLE_OPACITY,BM_GETCHECK,0,0) == BST_CHECKED);
        Settings.InactiveOpacity = InactiveTrackBar.GetPos();
        Settings.ActiveOpacity = ActiveTrackBar.GetPos();
        Settings.SetFocusToTaskName = (SendDlgItemMessage(IDC_SETFOCUS,BM_GETCHECK,0,0)==BST_CHECKED);
        Settings.HighlightTaskName = Highlight;
        Settings.SaveOtherSettings();
    }
};

//////////////////////////// MyOptionSheet //////////////////////////////
class CMyPropSheet : public Mortimer::CPropSheet
{
public:
    BEGIN_MSG_MAP(CMyPropSheet)
        MSG_WM_CTLCOLORSTATIC(OnPageCaptionDraw)
        COMMAND_ID_HANDLER_EX(IDC_HELP_BUTTON,OnHelp)
        COMMAND_ID_HANDLER_EX(IDC_ABOUT,OnAbout)
        COMMAND_ID_HANDLER_EX(IDC_CONTENTS,OnContents)
        CHAIN_MSG_MAP(CPropSheet)
    END_MSG_MAP()

    CMyPropSheet()
    {
        PageCaptionFont.CreateFont(16,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,RUSSIAN_CHARSET,
                                   OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
                                   DEFAULT_PITCH,"MS Sans Serif");
        HelpMenu.CreatePopupMenu();
        HelpMenu.AppendMenu(MF_ENABLED,IDC_CONTENTS,"&Contents...");
        HelpMenu.AppendMenu(MF_ENABLED,IDC_ABOUT,"&About...");

#if (USE_ICONS != 0)
        HBITMAP hBitmap;
        ImageList.Create(16,16,ILC_COLOR24|ILC_MASK,2,0);
        hBitmap = LoadBitmap(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDB_BITMAP1));
        ImageList.Add(hBitmap, RGB(0,128,128));
        DeleteObject(hBitmap);
#endif
    }

    ~CMyPropSheet()
    {
        DestroyMenu(HelpMenu);

#if (USE_ICONS != 0)
        if (ImageList != NULL) ImageList.Destroy();
#endif
    }

    LRESULT OnPageCaptionDraw(HDC hDC, HWND hWnd)
    {
        if (hWnd == m_PageCaption)
        {
            ::SetBkMode(hDC,TRANSPARENT);
            ::SetTextColor(hDC,GetSysColor(COLOR_CAPTIONTEXT));
            return (BOOL)GetSysColorBrush(COLOR_INACTIVECAPTION);
        }
        return FALSE;
    }

    bool DoInit(bool FirstTime, LPARAM lParam)
    {
        m_PageCaption.Attach(GetDlgItem(IDC_PAGE_CAPTION));
        m_PageCaption.SetFont(PageCaptionFont);

        if (FirstTime)
        {
            m_PageGeneral.Create(this);
            m_PageFormat.Create(this);
            m_PageDefects.Create(this);
            m_PageURLs.Create(this);
            m_PageURLs.Browser = &(m_PageGeneral.sBrowser);
            m_PageURLs.CheckMark = &(m_PageGeneral.sCheckMark);
            m_PageSoftTest.Create(this);
            m_PageHistory.Create(this);
            m_PageOther.Create(this);
            m_PageOther.Links = &(m_PageURLs.temp_links);
            m_PageURLs.ForbiddenHotkey = &(m_PageOther.HotKey);
            m_PageURLs.uWinKey = &(m_PageOther.WinKey);
        }

#if (USE_ICONS != 0)
        // Adding icons to the items in the tree control
        dynamic_cast<COptionSelectionTreeCtrl*>(GetSelectionControl())->SetImageList(ImageList,TVSIL_NORMAL);
        // creating CItem with COptionPage
        COptionSelectionTreeCtrl::CItem *NewItem;
        
        NewItem = new COptionSelectionTreeCtrl::CItem(0,0);
        NewItem->SetPage(&m_PageGeneral);
        NewItem->SetCaption("General");
        // adding new page
        AddItem(NewItem);
        // checking if this page should be active and activate it if so
        if (lParam == 0) SetActiveItem(NewItem);

        NewItem = new COptionSelectionTreeCtrl::CItem(1,1);
        NewItem->SetPage(&m_PageURLs);
        NewItem->SetCaption("URLs");
        // adding new page
        AddItem(NewItem);
        // checking if this page should be active and activate it if so
        if (lParam == 1) SetActiveItem(NewItem);

        NewItem = new COptionSelectionTreeCtrl::CItem(2,2);
        NewItem->SetPage(&m_PageDefects);
        NewItem->SetCaption("Defects");
        // adding new page
        AddItem(NewItem);
        // checking if this page should be active and activate it if so
        if (lParam == 2) SetActiveItem(NewItem);

        NewItem = new COptionSelectionTreeCtrl::CItem(3,3);
        NewItem->SetPage(&m_PageFormat);
        NewItem->SetCaption("Format");
        // adding new page
        AddItem(NewItem);
        // checking if this page should be active and activate it if so
        if (lParam == 3) SetActiveItem(NewItem);

        NewItem = new COptionSelectionTreeCtrl::CItem(4,4);
        NewItem->SetPage(&m_PageSoftTest);
        NewItem->SetCaption("SoftTest");
        // adding new page
        AddItem(NewItem);
        // checking if this page should be active and activate it if so
        if (lParam == 4) SetActiveItem(NewItem);

        NewItem = new COptionSelectionTreeCtrl::CItem(5,5);
        NewItem->SetPage(&m_PageHistory);
        NewItem->SetCaption("History");
        // adding new page
        AddItem(NewItem);
        // checking if this page should be active and activate it if so
        if (lParam == 5) SetActiveItem(NewItem);

        NewItem = new COptionSelectionTreeCtrl::CItem(6,6);
        NewItem->SetPage(&m_PageOther);
        NewItem->SetCaption("Other");
        // adding new page
        AddItem(NewItem);
        // checking if this page should be active and activate it if so
        if (lParam == 6) SetActiveItem(NewItem);

#else
        //In order not to use icons just use the following statements: 
        AddItem(new COptionItem("General", &m_PageGeneral));
        AddItem(new COptionItem("URLs", &m_PageURLs));
        AddItem(new COptionItem("Defects",&m_PageDefects));
        AddItem(new COptionItem("Format", &m_PageFormat));
        AddItem(new COptionItem("SoftTest", &m_PageSoftTest));
        AddItem(new COptionItem("History", &m_PageHistory));
        AddItem(new COptionItem("Other", &m_PageOther));
#endif

        SetFlags(OSF_HASBUTTON_OK|OSF_HASBUTTON_CANCEL);

        return CPropSheet::DoInit(FirstTime, lParam);
    }

    void OnItemChange(COptionItem *pNewItem, COptionItem *pOldItem)
    {
        m_PageCaption.SetWindowText(pNewItem->GetCaption());
    }

    void OnAbout(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        CAbout dlg;
        dlg.DoModal();
    }

    void OnContents(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        if (HtmlHelp(NULL,HelpFileName,HH_DISPLAY_TOPIC,NULL)==NULL)
        {
            CString ErrorMessage;
            ErrorMessage.Format("Help file \"%s\" was not found\nor your system does not support HTML help",HelpFileName);
            MyMessageBox(m_hWnd,ErrorMessage,szWinName,MB_ICONERROR);
        }
    }

    void OnHelp(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        RECT Rect;
        ::GetWindowRect(GetDlgItem(IDC_HELP_BUTTON),&Rect);
        HelpMenu.TrackPopupMenu(TPM_LEFTBUTTON|TPM_LEFTALIGN|TPM_TOPALIGN,Rect.left,Rect.bottom,m_hWnd);
    }

protected:
    CFont PageCaptionFont;
    GeneralPage m_PageGeneral;
    FormatPage m_PageFormat;
    DefectsPage m_PageDefects;
    URLsPage m_PageURLs;
    SoftTestPage m_PageSoftTest;
    HistoryPage m_PageHistory;
    OtherPage m_PageOther;
    CStatic m_PageCaption;
    CMenu HelpMenu;
#if (USE_ICONS != 0)
    CImageList ImageList;
#endif
};

#endif