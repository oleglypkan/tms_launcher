// Options.h
//
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

extern CSettings Settings;
extern CString szWinName;
extern UINT LINK_MAX;
const int max_value_length = 2;
const char *HelpFileName = "TMS_Launcher.chm";

int CompareNoCaseCP1251(const char *string1, const char *string2);

//////////////////////////// General options page //////////////////////
class GeneralPage : public Mortimer::COptionPageImpl<GeneralPage,CPropPage>
{
public:
    enum { IDD = GENERAL_PAGE };
    CEdit PathToBrowser;

    BEGIN_MSG_MAP(GeneralPage)
        MSG_WM_INITDIALOG(OnInitDialog)
        COMMAND_ID_HANDLER_EX(IDC_BROWSE,OnBrowse)
    END_MSG_MAP()

    // called once when options dialog is opened
    LRESULT OnInitDialog(HWND hWnd, LPARAM lParam)
    {
        switch (Settings.RightClickAction)
        {
            case 0:
                SendDlgItemMessage(IDC_TASK_RADIO,BM_SETCHECK,BST_CHECKED,0);
                break;
            case 1:
                SendDlgItemMessage(IDC_CHILD_RADIO,BM_SETCHECK,BST_CHECKED,0);
                break;
        }
        if (Settings.Expand) SendDlgItemMessage(IDC_EXPANDED,BM_SETCHECK,BST_CHECKED,0);
        if (Settings.AutoRun) SendDlgItemMessage(IDC_AUTORUN,BM_SETCHECK,BST_CHECKED,0);
        if (Settings.Minimize) SendDlgItemMessage(IDC_MINIMIZE,BM_SETCHECK,BST_CHECKED,0);
        if (Settings.SingleClick) SendDlgItemMessage(IDC_SINGLE_CLICK,BM_SETCHECK,BST_CHECKED,0);
        if (Settings.DefaultBrowser) SendDlgItemMessage(IDC_DEFAULT_BROWSER,BM_SETCHECK,BST_CHECKED,0);
        PathToBrowser.Attach(GetDlgItem(IDC_BROWSER_PATH));
        PathToBrowser.LimitText(_MAX_PATH);
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

    // called every time when the page is deactivated
    bool OnKillActive(COptionItem *pItem)
    {
        return true;
    }

    // called every time when whole sheet is closed by clicking on OK button
    void OnOK()
    {
        Settings.AutoRun=(SendDlgItemMessage(IDC_AUTORUN,BM_GETCHECK,0,0)==BST_CHECKED);
        Settings.Expand=(SendDlgItemMessage(IDC_EXPANDED,BM_GETCHECK,0,0)==BST_CHECKED);
        Settings.Minimize=(SendDlgItemMessage(IDC_MINIMIZE,BM_GETCHECK,0,0)==BST_CHECKED);
        Settings.SingleClick=(SendDlgItemMessage(IDC_SINGLE_CLICK,BM_GETCHECK,0,0)==BST_CHECKED);
        Settings.DefaultBrowser=(SendDlgItemMessage(IDC_DEFAULT_BROWSER,BM_GETCHECK,0,0)==BST_CHECKED);
        if (SendDlgItemMessage(IDC_TASK_RADIO,BM_GETCHECK,0,0)==BST_CHECKED)
            Settings.RightClickAction = 0;
        else
            Settings.RightClickAction = 1;

        PathToBrowser.GetWindowText(Settings.BrowserPath.GetBuffer(_MAX_PATH+1),_MAX_PATH+1);
        Settings.BrowserPath.ReleaseBuffer();
        Settings.BrowserPath.TrimLeft();
        Settings.BrowserPath.TrimRight();
        Settings.SaveGeneralSettings();
    }

    // called every time when whole sheet is closed by clicking on Cancel button
    void OnCancel() {}
};

//////////////////////////// General options page //////////////////////
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
        EditControl.GetWindowText(temp.GetBuffer(max_value_length),max_value_length+1);
        temp.ReleaseBuffer();
        int i=0;
        while (i<temp.GetLength())
        {
            if (!isdigit(temp[i]))
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
        sMaxClientName.GetWindowText(temp.GetBuffer(max_value_length),max_value_length+1);
        temp.ReleaseBuffer();
        Settings.MaxClientName = atoi(temp);
        // %ID%
        sMaxIDName.GetWindowText(temp.GetBuffer(max_value_length),max_value_length+1);
        temp.ReleaseBuffer();
        Settings.MaxIDName = atoi(temp);
        // %EXT%
        sMaxExtName.GetWindowText(temp.GetBuffer(max_value_length),max_value_length+1);
        temp.ReleaseBuffer();
        Settings.MaxExt = atoi(temp);
        Settings.MinTaskName = Settings.MinClientName+Settings.MinIDName;
        Settings.MaxTaskName = Settings.MaxClientName+Settings.MaxIDName+Settings.MaxExt+2; // 2 - separators 

        sSeparators.GetWindowText(temp.GetBuffer(255),255+1);
        temp.ReleaseBuffer();
        Settings.RemoveDuplicateSeparators(temp);
        Settings.Separators = temp;
        sTasksSeparators.GetWindowText(temp.GetBuffer(255),255+1);
        temp.ReleaseBuffer();
        Settings.RemoveDuplicateSeparators(temp);
        Settings.TasksSeparators = temp;

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
        sMaxClientName.GetWindowText(temp.GetBuffer(max_value_length),max_value_length+1);
        temp.ReleaseBuffer();
        max_value = atoi(temp);
        if (max_value < Settings.MinClientName)
        {
            MessageBox("Maximum %CLIENT% length cannot be less than minimum one.\nPlease enter correct value",szWinName,MB_ICONERROR);
            return false;
        }
        // %ID%
        sMaxIDName.GetWindowText(temp.GetBuffer(max_value_length),max_value_length+1);
        temp.ReleaseBuffer();
        max_value = atoi(temp);
        if (max_value < Settings.MinIDName)
        {
            MessageBox("Maximum %ID% length cannot be less than minimum one.\nPlease enter correct value",szWinName,MB_ICONERROR);
            return false;
        }
        // %EXT%
        sMaxExtName.GetWindowText(temp.GetBuffer(max_value_length),max_value_length+1);
        temp.ReleaseBuffer();
        max_value = atoi(temp);
        if (max_value < Settings.MinExt)
        {
            MessageBox("Maximum %EXT% length cannot be less than minimum one.\nPlease enter correct value",szWinName,MB_ICONERROR);
            return false;
        }

        // checking for "Separators" and "TasksSeparators" fields are filled correctly
        sSeparators.GetWindowText(temp.GetBuffer(255),255+1);
        temp.ReleaseBuffer();
        if (temp.IsEmpty())
        {
            MessageBox("\"Separators\" field must not be empty.\nPlease enter correct value",szWinName,MB_ICONERROR);
            return false;
        }
        for (int i=0; i<temp.GetLength(); i++)
        {
            if (isalnum(temp[i]))
            {
                if (MessageBox("\"Separators\" field has character(s) that cannot be used as separators.\nWould you like the character(s) to be removed automatically?",szWinName,MB_YESNO|MB_ICONERROR)==IDYES)
                {
                    Settings.RemoveUnacceptableSeparators(temp);
                    sSeparators.SetWindowText(temp);
                }
                return false;
            }
        }

        sTasksSeparators.GetWindowText(temp2.GetBuffer(255),255+1);
        temp2.ReleaseBuffer();
        if (temp2.IsEmpty())
        {
            MessageBox("\"Tasks separators\" field must not be empty.\nPlease enter correct value",szWinName,MB_ICONERROR);
            return false;
        }
        for (i=0; i<temp2.GetLength(); i++)
        {
            if (isalnum(temp2[i]))
            {
                if (MessageBox("\"Tasks separators\" field has character(s) that cannot be used as separators.\nWould you like the character(s) to be removed automatically?",szWinName,MB_YESNO|MB_ICONERROR)==IDYES)
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
            MessageBox(Message,szWinName,MB_ICONERROR);
            return false;
        }
        
        return true;
    }

};

//////////////////////////// Links options page ///////////////////////
class URLsPage : public Mortimer::COptionPageImpl<URLsPage,CPropPage>
{
public:
    enum { IDD = URLS_PAGE };
    CComboBox CaptionsDropList;
    CComboBox CaptionsDropDown;
    CEdit CaptionEdit;
    CEdit TaskURL;
    CEdit ChildTasksURL;
    CWindow TaskHotkey;
    CWindow ChildTasksHotkey;
    CButton Default;
    CButton NewButton;
    CButton EditButton;
    CButton SaveButton;
    CButton DeleteButton;
    int MaxStringLength;
    int edit_pos;
    std::vector<link> temp_links;

    BEGIN_MSG_MAP(URLsPage)
        MSG_WM_INITDIALOG(OnInitDialog)
        COMMAND_CODE_HANDLER_EX(CBN_SELCHANGE, OnSelChanged)
        COMMAND_ID_HANDLER_EX(IDC_LINK_NEW,OnNewLink)
        COMMAND_ID_HANDLER_EX(IDC_LINK_EDIT,OnEditLink)
        COMMAND_ID_HANDLER_EX(IDC_LINK_SAVE,OnSaveLink)
        COMMAND_ID_HANDLER_EX(IDC_LINK_DELETE,OnDeleteLink)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    void sort_links()
    {
	    for (int i=0; i<temp_links.size()-1; i++)
	    {
		    bool exchange = false;
		    for (int j=1; j<temp_links.size()-i; j++)
		    {
			    if (CompareNoCaseCP1251(temp_links[j-1].Caption,temp_links[j].Caption)>0)
			    {
				    link temp = temp_links[j-1];
				    temp_links[j-1] = temp_links[j];
				    temp_links[j] = temp;
                    exchange = true;
			    }
		    }
		    if (!exchange) break;
	    }
    }

    // called once when options dialog is opened
    LRESULT OnInitDialog(HWND hWnd, LPARAM lParam)
    {
        MaxStringLength = 255;
        edit_pos = CB_ERR;
        NewButton.Attach(GetDlgItem(IDC_LINK_NEW));
        EditButton.Attach(GetDlgItem(IDC_LINK_EDIT));
        SaveButton.Attach(GetDlgItem(IDC_LINK_SAVE));
        DeleteButton.Attach(GetDlgItem(IDC_LINK_DELETE));

        CaptionsDropList.Attach(GetDlgItem(IDC_URL_CAPTION));
        CaptionsDropList.LimitText(MaxStringLength);
        
        CaptionsDropDown.Attach(GetDlgItem(IDC_URL_CAPTION2));
        CaptionsDropDown.LimitText(MaxStringLength);
        CaptionsDropDown.EnableWindow(FALSE);
        CaptionsDropDown.ShowWindow(SW_HIDE);

        CaptionEdit.Attach(GetDlgItem(IDC_URL_EDIT));
        CaptionEdit.LimitText(MaxStringLength);
        CaptionEdit.EnableWindow(FALSE);
        CaptionEdit.ShowWindow(SW_HIDE);

        TaskURL.Attach(GetDlgItem(IDC_TASK_URL));
        TaskURL.LimitText(MaxStringLength);
        ChildTasksURL.Attach(GetDlgItem(IDC_CHILD_TASKS_URL));
        ChildTasksURL.LimitText(MaxStringLength);
        Default.Attach(GetDlgItem(IDC_URL_DEFAULT));
        TaskHotkey.Attach(GetDlgItem(VIEW_TASK_HOTKEY));
        ChildTasksHotkey.Attach(GetDlgItem(VIEW_CHILD_TASKS_HOTKEY));
        UINT HotkeyID = 0;
        for (int i=0; i<Settings.links.size(); i++)
        {
            UnregisterHotKey(::GetParent(GetParent()),HotkeyID);
            HotkeyID++;
            UnregisterHotKey(::GetParent(GetParent()),HotkeyID);
            HotkeyID++;
            temp_links.push_back(Settings.links[i]);
            CaptionsDropList.AddString(Settings.links[i].Caption);
        }
        CaptionsDropList.SetCurSel(0);
        if (temp_links[0].Default)
        {
            Default.SetCheck(BST_CHECKED);
        }
        else
        {
            Default.SetCheck(BST_UNCHECKED);
        }
        Default.EnableWindow(FALSE);
        TaskURL.SetWindowText(temp_links[0].TaskURL);
        TaskURL.SetReadOnly(TRUE);
        ChildTasksURL.SetWindowText(temp_links[0].ChildTasksURL);
        ChildTasksURL.SetReadOnly(TRUE);
        TaskHotkey.SendMessage(HKM_SETHOTKEY,temp_links[0].ViewTaskHotKey,0);
        TaskHotkey.EnableWindow(FALSE);
        ChildTasksHotkey.SendMessage(HKM_SETHOTKEY,temp_links[0].ViewChildTasksHotKey,0);
        ChildTasksHotkey.EnableWindow(FALSE);
        SaveButton.EnableWindow(FALSE);
        
        return 0;
    }

    // called after selection in any ComboBox has been changed
    void OnSelChanged(UINT code, int ControlID, HWND ControlHandle)
    {
        CComboBox ComboBox = ControlHandle;
        int position = ComboBox.GetCurSel();
        if (position != CB_ERR)
        {
            if (temp_links[position].Default)
            {
                Default.SetCheck(BST_CHECKED);
            }
            else
            {
                Default.SetCheck(BST_UNCHECKED);
            }
            TaskURL.SetWindowText(temp_links[position].TaskURL);
            ChildTasksURL.SetWindowText(temp_links[position].ChildTasksURL);
            TaskHotkey.SendMessage(HKM_SETHOTKEY,temp_links[position].ViewTaskHotKey,0);
            ChildTasksHotkey.SendMessage(HKM_SETHOTKEY,temp_links[position].ViewChildTasksHotKey,0);
        }
    }

    void OnNewLink(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        for (int i=0; i<CaptionsDropList.GetCount(); i++)
        {
            CString temp;
            CaptionsDropList.GetLBText(i,temp);
            CaptionsDropDown.AddString(temp);
        }
        CaptionsDropList.ResetContent();
        CaptionsDropList.ShowWindow(SW_HIDE);
        CaptionsDropList.EnableWindow(FALSE);

        CaptionsDropDown.EnableWindow(TRUE);
        CaptionsDropDown.SetWindowText("");
        CaptionsDropDown.ShowWindow(SW_SHOW);
        
        NewButton.EnableWindow(FALSE);
        EditButton.EnableWindow(FALSE);
        DeleteButton.EnableWindow(FALSE);
        SaveButton.EnableWindow(TRUE);
        CaptionsDropList.SetWindowText("");
        Default.SetCheck(BST_UNCHECKED);
        Default.EnableWindow(TRUE);
        TaskURL.SetWindowText("");
        TaskURL.SetReadOnly(FALSE);
        ChildTasksURL.SetWindowText("");
        ChildTasksURL.SetReadOnly(FALSE);
        TaskHotkey.SendMessage(HKM_SETHOTKEY,0,0);
        TaskHotkey.EnableWindow(TRUE);
        ChildTasksHotkey.SendMessage(HKM_SETHOTKEY,0,0);
        ChildTasksHotkey.EnableWindow(TRUE);
        ::SetFocus(CaptionsDropDown);
    }

    void OnEditLink(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        edit_pos = CaptionsDropList.GetCurSel();
        if (edit_pos != CB_ERR)
        {
            CString temp;
            CaptionsDropList.GetLBText(edit_pos,temp);
            CaptionsDropList.ShowWindow(SW_HIDE);
            CaptionsDropList.EnableWindow(FALSE);

            CaptionEdit.SetWindowText(temp);
            CaptionEdit.EnableWindow(TRUE);
            CaptionEdit.ShowWindow(SW_SHOW);
            
            Default.EnableWindow(TRUE);
            TaskURL.SetReadOnly(FALSE);
            ChildTasksURL.SetReadOnly(FALSE);
            TaskHotkey.EnableWindow(TRUE);
            ChildTasksHotkey.EnableWindow(TRUE);
            NewButton.EnableWindow(FALSE);
            EditButton.EnableWindow(FALSE);
            DeleteButton.EnableWindow(FALSE);
            SaveButton.EnableWindow(TRUE);
            ::SetFocus(CaptionEdit);
        }
    }

    void OnDeleteLink(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        int position = CaptionsDropList.GetCurSel();
        if (position == CB_ERR)
        {
            return;
        }
        
        if (CaptionsDropList.GetCount()==1)
        {
            MessageBox("At least one record should stay in the list.\nIt cannot be deleted",szWinName,MB_ICONERROR);
            return;
        }
        
        bool wasDefault = temp_links[position].Default;
        CaptionsDropList.DeleteString(position);
        temp_links.erase(temp_links.begin()+position);

        if (wasDefault)
        {
            temp_links[0].Default = true;
            MessageBox("You are about to delete the default record.\nThe first record will be marked as default",szWinName,MB_ICONINFORMATION);
        }
        CaptionsDropList.SetCurSel(0);
        if (temp_links[0].Default)
        {
            Default.SetCheck(BST_CHECKED);
        }
        else
        {
            Default.SetCheck(BST_UNCHECKED);
        }
        TaskURL.SetWindowText(temp_links[0].TaskURL);
        ChildTasksURL.SetWindowText(temp_links[0].ChildTasksURL);
        TaskHotkey.SendMessage(HKM_SETHOTKEY,temp_links[0].ViewTaskHotKey,0);
        ChildTasksHotkey.SendMessage(HKM_SETHOTKEY,temp_links[0].ViewChildTasksHotKey,0);

        EditButton.EnableWindow(TRUE);
        DeleteButton.EnableWindow(TRUE);

        Default.EnableWindow(FALSE);
        TaskURL.SetReadOnly(TRUE);
        ChildTasksURL.SetReadOnly(TRUE);
        TaskHotkey.EnableWindow(FALSE);
        ChildTasksHotkey.EnableWindow(FALSE);

        SaveButton.EnableWindow(FALSE);
    }

    void OnSaveLink(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        // checking for unique and non-empty URLCaption
        bool bDefault = (Default.GetCheck() == BST_CHECKED);
        CString strURLCaption;
        CComboBox ComboBox;
        if (edit_pos == CB_ERR)
        {
            // trying to save new record
            ComboBox = CaptionsDropDown;
            ComboBox.GetWindowText(strURLCaption.GetBuffer(MaxStringLength),MaxStringLength+1);
        }
        else
        {
            //trying to save edited record
            ComboBox = CaptionsDropList;
            CaptionEdit.GetWindowText(strURLCaption.GetBuffer(MaxStringLength),MaxStringLength+1);
        }
        strURLCaption.ReleaseBuffer();
        
        if (strURLCaption.IsEmpty())
        {
            MessageBox("\"Open tasks in\" field must not be empty.",szWinName,MB_ICONERROR);
            return;
        }
        bool unique = true;
        int non_unique_pos = ComboBox.FindStringExact(-1,strURLCaption);
        if ((CB_ERR != non_unique_pos) && (non_unique_pos != edit_pos))
        {
            unique = false;
        }
        if (!unique)
        {
            MessageBox("The value entered in \"Open tasks in\" field is not unique.\nPlease use another one",szWinName,MB_ICONERROR);
            return;
        }
        // checking for non-empty links
        CString strTaskURL, strChildTasksURL;
        TaskURL.GetWindowText(strTaskURL.GetBuffer(MaxStringLength),MaxStringLength+1);
        strTaskURL.ReleaseBuffer();
        if (strTaskURL.IsEmpty())
        {
            MessageBox("URL to view Task cannot be empty.\nPlease enter correct URL",szWinName,MB_ICONERROR);
            return;
        }
        ChildTasksURL.GetWindowText(strChildTasksURL.GetBuffer(MaxStringLength),MaxStringLength+1);
        strChildTasksURL.ReleaseBuffer();
        if (strChildTasksURL.IsEmpty())
        {
            MessageBox("URL to view Child Tasks cannot be empty.\nPlease enter correct URL",szWinName,MB_ICONERROR);
            return;
        }

        // checking hotkeys
        UINT Hotkey1 = (UINT)TaskHotkey.SendMessage(HKM_GETHOTKEY,0,0);
        UINT Hotkey2 = (UINT)ChildTasksHotkey.SendMessage(HKM_GETHOTKEY,0,0);
        if (Hotkey1 && Hotkey1 == Hotkey2)
        {
            MessageBox("Entered hotkeys are the same.\nPlease use different ones.",szWinName,MB_ICONERROR);
            return;
        }
        for (int i=0; i<temp_links.size(); i++)
        {
            if (i == edit_pos)
            {
                continue;
            }
            if ((Hotkey1)&&((Hotkey1 == temp_links[i].ViewTaskHotKey)||
               (Hotkey1 == temp_links[i].ViewChildTasksHotKey)))
            {
                MessageBox("Hotkey entered in \"HotKey to View Task\" field is not unique.\nPlease enter another one",szWinName,MB_ICONERROR);
                return;
            }
            if ((Hotkey2)&&((Hotkey2 == temp_links[i].ViewTaskHotKey)||
               (Hotkey2 == temp_links[i].ViewChildTasksHotKey)))
            {
                MessageBox("Hotkey entered in \"HotKey to View Child Tasks\" field is not unique.\nPlease enter another one",szWinName,MB_ICONERROR);
                return;
            }
        }
        UINT HotKeyID;
        if (edit_pos == CB_ERR)
        {
            HotKeyID = (temp_links.size())*2;
        }
        else
        {
            HotKeyID = edit_pos*2;
        }
        if (Hotkey1)
        {
            if (!RegisterHotKey(::GetParent(GetParent()),HotKeyID,(!(Hotkey1&0x500)?
                           HIBYTE(LOWORD(Hotkey1)):((Hotkey1&0x500)<0x500?
                           HIBYTE(LOWORD(Hotkey1))^5:HIBYTE(LOWORD(Hotkey1)))),
                           LOBYTE(LOWORD(Hotkey1))))
            {
                MessageBox("Hotkey entered in \"HotKey to View Task\" field is already registered by another program.\nPlease enter another hotkey",szWinName,MB_ICONERROR);
                return;
            }
            UnregisterHotKey(::GetParent(GetParent()),HotKeyID);
        }
        HotKeyID++;
        if (Hotkey2)
        {
            if (!RegisterHotKey(::GetParent(GetParent()),HotKeyID,(!(Hotkey2&0x500)?
                            HIBYTE(LOWORD(Hotkey2)):((Hotkey2&0x500)<0x500?
                            HIBYTE(LOWORD(Hotkey2))^5:HIBYTE(LOWORD(Hotkey2)))),
                            LOBYTE(LOWORD(Hotkey2))))
            {
                MessageBox("Hotkey entered in \"HotKey to View Child Tasks\" field is already registered by another program.\nPlease enter another hotkey",szWinName,MB_ICONERROR);
                return;
            }
            UnregisterHotKey(::GetParent(GetParent()),HotKeyID);
        }
        
        // saving new record
        int RecordPosition;
        if (edit_pos == CB_ERR)
        {
            temp_links.push_back(link(strURLCaption,strTaskURL,strChildTasksURL,Hotkey1,Hotkey2,bDefault));
            ComboBox.AddString(strURLCaption);

            for (int i=0; i<ComboBox.GetCount(); i++)
            {
                CString temp;
                ComboBox.GetLBText(i,temp);
                CaptionsDropList.AddString(temp);
            }

            CaptionsDropList.SetCurSel(CaptionsDropList.GetCount()-1);
            RecordPosition = CaptionsDropList.GetCount()-1;
           
            CaptionsDropDown.ShowWindow(SW_HIDE);
            CaptionsDropDown.ResetContent();
            CaptionsDropDown.EnableWindow(FALSE);
        }
        else // saving edited record
        {
            temp_links[edit_pos].Caption = strURLCaption;
            temp_links[edit_pos].TaskURL = strTaskURL;
            temp_links[edit_pos].ChildTasksURL = strChildTasksURL;
            temp_links[edit_pos].ViewTaskHotKey = Hotkey1;
            temp_links[edit_pos].ViewChildTasksHotKey = Hotkey2;
            temp_links[edit_pos].Default = bDefault;
            RecordPosition = edit_pos;

            ComboBox.InsertString(edit_pos,strURLCaption);
            ComboBox.DeleteString(edit_pos+1);
            ComboBox.SetCurSel(edit_pos);

            CaptionEdit.ShowWindow(SW_HIDE);
            CaptionEdit.EnableWindow(FALSE);
            CaptionEdit.SetWindowText("");
        }
        CaptionsDropList.EnableWindow(TRUE);
        CaptionsDropList.ShowWindow(SW_SHOW);

        // unset default flag of previously default link
        bool IsDefault = bDefault;
        for (i=0; i<temp_links.size(); i++)
        {
            if (bDefault)
            {
                if (i != RecordPosition)
                {
                    temp_links[i].Default = false;
                }
            }
            else
            {
                if (temp_links[i].Default)
                {
                    IsDefault = true;
                    break;
                }
            }
        }
        if (!IsDefault)
        {
            MessageBox("None of records is marked as default.\nLast saved record will be marked as default",szWinName,MB_ICONINFORMATION);
            temp_links[RecordPosition].Default = true;
            Default.SetCheck(BST_CHECKED);
        }
        Default.EnableWindow(FALSE);
        TaskURL.SetReadOnly(TRUE);
        ChildTasksURL.SetReadOnly(TRUE);
        TaskHotkey.EnableWindow(FALSE);
        ChildTasksHotkey.EnableWindow(FALSE);

        EditButton.EnableWindow(TRUE);
        DeleteButton.EnableWindow(TRUE);

        SaveButton.EnableWindow(FALSE);
        NewButton.EnableWindow(TRUE);

        edit_pos = CB_ERR;
        ::SetFocus(NewButton);
    }

    void StopSavingURL()
    {
        // stop before saving new record
        int position;
        if (edit_pos == CB_ERR)
        {
            position = 0;
            for (int i=0; i<CaptionsDropDown.GetCount(); i++)
            {
                CString temp;
                CaptionsDropDown.GetLBText(i,temp);
                CaptionsDropList.AddString(temp);
            }

            CaptionsDropList.SetCurSel(0);
           
            CaptionsDropDown.ShowWindow(SW_HIDE);
            CaptionsDropDown.ResetContent();
            CaptionsDropDown.EnableWindow(FALSE);
        }
        else // stop before saving edited record
        {
            position = edit_pos;
            CaptionEdit.ShowWindow(SW_HIDE);
            CaptionEdit.EnableWindow(FALSE);
            CaptionEdit.SetWindowText("");
        }
        if (temp_links[position].Default)
        {
            Default.SetCheck(BST_CHECKED);
        }
        else
        {
            Default.SetCheck(BST_UNCHECKED);
        }
        TaskURL.SetWindowText(temp_links[position].TaskURL);
        ChildTasksURL.SetWindowText(temp_links[position].ChildTasksURL);
        TaskHotkey.SendMessage(HKM_SETHOTKEY,temp_links[position].ViewTaskHotKey,0);
        ChildTasksHotkey.SendMessage(HKM_SETHOTKEY,temp_links[position].ViewChildTasksHotKey,0);

        CaptionsDropList.EnableWindow(TRUE);
        CaptionsDropList.ShowWindow(SW_SHOW);

        Default.EnableWindow(FALSE);
        TaskURL.SetReadOnly(TRUE);
        ChildTasksURL.SetReadOnly(TRUE);
        TaskHotkey.EnableWindow(FALSE);
        ChildTasksHotkey.EnableWindow(FALSE);

        EditButton.EnableWindow(TRUE);
        DeleteButton.EnableWindow(TRUE);

        SaveButton.EnableWindow(FALSE);
        NewButton.EnableWindow(TRUE);

        edit_pos = CB_ERR;
        ::SetFocus(NewButton);
    }

    // called every time when the page is activated
    bool OnSetActive(COptionItem *pItem)
    {
        return true;
    }

    // called every time when the page is deactivated
    bool OnKillActive(COptionItem *pItem)
    {
        if (SaveButton.IsWindowEnabled())
        {
            if (MessageBox("You have not saved the record.\nContinue without saving?",szWinName,MB_ICONWARNING|MB_YESNO|MB_DEFBUTTON2)==IDYES)
            {
                StopSavingURL();
                return true;
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    void OnOK()
    {
        CaptionsDropList.ResetContent();
        Settings.links.clear();
        sort_links();
        UINT HotkeyID = 0;
        for (int i=0; i<temp_links.size(); i++)
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
                    MessageBox(message,szWinName,MB_ICONERROR);
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
                    MessageBox(message,szWinName,MB_ICONERROR);
                }
            }
            HotkeyID++;
        }

        Settings.SaveLinksSettings();
    }

    void OnCancel()
    {
        UINT HotkeyID = 0;
        for (int i=0; i<Settings.links.size(); i++)
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
                    MessageBox(message,szWinName,MB_ICONERROR);
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
                    MessageBox(message,szWinName,MB_ICONERROR);
                }
            }
            HotkeyID++;
        }
    };
};

//////////////////////////// Colors options page ///////////////////////
class DefectsPage : public Mortimer::COptionPageImpl<DefectsPage,CPropPage>
{
public:
    enum { IDD = DEFECTS_PAGE };

    CComboBox DefectsDropList; // linked to combobox with CBS_DROPDOWNLIST style
    CComboBox DefectsDropDown; // linked to combobox with CBS_DROPDOWN style
    CEdit DefectEdit;          // linked to editbox
    CEdit Project;
    CEdit Link;
    CButton NewButton;
    CButton EditButton;
    CButton SaveButton;
    CButton DeleteButton;
    int MaxStringLength;
    int edit_pos;
    std::vector<defect> projects;
    
    BEGIN_MSG_MAP(DefectsPage)
        MSG_WM_INITDIALOG(OnInitDialog)
        COMMAND_CODE_HANDLER_EX(CBN_SELCHANGE, OnSelChanged)
        COMMAND_ID_HANDLER_EX(IDC_DEFECT_NEW,OnNewDefect)
        COMMAND_ID_HANDLER_EX(IDC_DEFECT_EDIT,OnEditDefect)
        COMMAND_ID_HANDLER_EX(IDC_DEFECT_SAVE,OnSaveDefect)
        COMMAND_ID_HANDLER_EX(IDC_DEFECT_DELETE,OnDeleteDefect)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    void sort_defects()
    {
	    for (int i=0; i<projects.size()-1; i++)
	    {
		    bool exchange = false;
		    for (int j=1; j<projects.size()-i; j++)
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

    // called once when options dialog is opened
    LRESULT OnInitDialog(HWND hWnd, LPARAM lParam)
    {
        MaxStringLength = 255;
        edit_pos = CB_ERR;
        NewButton.Attach(GetDlgItem(IDC_DEFECT_NEW));
        EditButton.Attach(GetDlgItem(IDC_DEFECT_EDIT));
        SaveButton.Attach(GetDlgItem(IDC_DEFECT_SAVE));
        DeleteButton.Attach(GetDlgItem(IDC_DEFECT_DELETE));

        DefectsDropList.Attach(GetDlgItem(IDC_CLIENT_ID));
        DefectsDropList.LimitText(Settings.MaxClientName);
        
        DefectsDropDown.Attach(GetDlgItem(IDC_CLIENT_ID2));
        DefectsDropDown.LimitText(Settings.MaxClientName);
        DefectsDropDown.EnableWindow(FALSE);
        DefectsDropDown.ShowWindow(SW_HIDE);

        DefectEdit.Attach(GetDlgItem(IDC_CLIENT_EDIT));
        DefectEdit.LimitText(Settings.MaxClientName);
        DefectEdit.EnableWindow(FALSE);
        DefectEdit.ShowWindow(SW_HIDE);

        Project.Attach(GetDlgItem(IDC_DEFECT_PROJECT));
        Project.LimitText(MaxStringLength);
        Link.Attach(GetDlgItem(IDC_DEFECTS_LINK));
        Link.LimitText(LINK_MAX);
        Link.SetWindowText(Settings.DefectsLink);

        for (int i=0; i<Settings.defects.size(); i++)
        {
            DefectsDropList.AddString(Settings.defects[i].ClientID);
            projects.push_back(defect(Settings.defects[i].ClientID,Settings.defects[i].STProject));
        }
        if (Settings.defects.size()>0)
        {
            DefectsDropList.SetCurSel(0);
            Project.SetWindowText(projects[0].STProject);
        }
        else
        {
            EditButton.EnableWindow(FALSE);
            DeleteButton.EnableWindow(FALSE);
        }

        Project.SetReadOnly(TRUE);
        SaveButton.EnableWindow(FALSE);

        return 0;
    }

    // called after selection in any ComboBox has been changed
    void OnSelChanged(UINT code, int ControlID, HWND ControlHandle)
    {
        CComboBox ComboBox = ControlHandle;
        int position = ComboBox.GetCurSel();
        if (position != CB_ERR)
        {
            Project.SetWindowText(projects[position].STProject);
        }
    }

    void OnNewDefect(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        for (int i=0; i<DefectsDropList.GetCount(); i++)
        {
            CString temp;
            DefectsDropList.GetLBText(i,temp);
            DefectsDropDown.AddString(temp);
        }
        DefectsDropList.ResetContent();
        DefectsDropList.ShowWindow(SW_HIDE);
        DefectsDropList.EnableWindow(FALSE);

        DefectsDropDown.EnableWindow(TRUE);
        DefectsDropDown.ShowWindow(SW_SHOW);
        DefectsDropDown.SetWindowText("");
        
        Project.SetWindowText("");
        Project.SetReadOnly(FALSE);
        NewButton.EnableWindow(FALSE);
        EditButton.EnableWindow(FALSE);
        DeleteButton.EnableWindow(FALSE);
        SaveButton.EnableWindow(TRUE);
        ::SetFocus(DefectsDropDown);
    }

    void OnSaveDefect(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        // checking for duplicates and non-empty Project
        CComboBox ComboBox;
        CString strDefect, strProject;
        if (edit_pos == CB_ERR)
        {
            // trying to save new record
            ComboBox = DefectsDropDown;
            ComboBox.GetWindowText(strDefect.GetBuffer(MaxStringLength),MaxStringLength+1);
        }
        else
        {
            //trying to save edited record
            ComboBox = DefectsDropList;
            DefectEdit.GetWindowText(strDefect.GetBuffer(MaxStringLength),MaxStringLength+1);
        }
        strDefect.ReleaseBuffer();
        Project.GetWindowText(strProject.GetBuffer(MaxStringLength),MaxStringLength+1);
        strProject.ReleaseBuffer();
        
        int non_unique_pos = CB_ERR;
        for (int i=0; i<ComboBox.GetCount(); i++)
        {
            CString temp;
            ComboBox.GetLBText(i,temp);
            if (CompareNoCaseCP1251(strDefect,temp)==0)
            {
                non_unique_pos = i;
                break;
            }
        }
        bool unique = true;
        if ((CB_ERR != non_unique_pos) && (non_unique_pos != edit_pos))
        {
            unique = false;
        }
        if (!unique)
        {
            MessageBox("Entered client name is not unique.\nPlease use another one",szWinName,MB_ICONERROR);
            return;
        }
        if (strDefect.Find(';') != -1)
        {
            MessageBox("Symbol \';\' cannot be used as a part of Client name",szWinName,MB_ICONERROR);
            return;
        }
        if (strProject.IsEmpty())
        {
            MessageBox("Project field must not be empty.",szWinName,MB_ICONERROR);
            return;
        }
        // saving new record
        if (edit_pos == CB_ERR)
        {
            projects.push_back(defect(strDefect,strProject));
            ComboBox.AddString(strDefect);
            for (int i=0; i<ComboBox.GetCount(); i++)
            {
                CString temp;
                ComboBox.GetLBText(i,temp);
                DefectsDropList.AddString(temp);
            }
            DefectsDropList.SetCurSel(DefectsDropList.GetCount()-1);
            
            DefectsDropDown.ShowWindow(SW_HIDE);
            DefectsDropDown.ResetContent();
            DefectsDropDown.EnableWindow(FALSE);
        }
        else // saving edited record
        {
            projects[edit_pos].ClientID = strDefect;
            projects[edit_pos].STProject = strProject;

            ComboBox.InsertString(edit_pos,strDefect);
            ComboBox.DeleteString(edit_pos+1);
            ComboBox.SetCurSel(edit_pos);
            
            DefectEdit.ShowWindow(SW_HIDE);
            DefectEdit.EnableWindow(FALSE);
            DefectEdit.SetWindowText("");
        }
        DefectsDropList.EnableWindow(TRUE);
        DefectsDropList.ShowWindow(SW_SHOW);

        Project.SetReadOnly(TRUE);
        EditButton.EnableWindow(TRUE);
        DeleteButton.EnableWindow(TRUE);

        SaveButton.EnableWindow(FALSE);
        NewButton.EnableWindow(TRUE);
        
        edit_pos = CB_ERR;
        ::SetFocus(NewButton);
    }

    void OnEditDefect(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        edit_pos = DefectsDropList.GetCurSel();
        if (edit_pos != CB_ERR)
        {
            CString temp;
            DefectsDropList.GetLBText(edit_pos,temp);
            DefectsDropList.ShowWindow(SW_HIDE);
            DefectsDropList.EnableWindow(FALSE);

            DefectEdit.SetWindowText(temp);
            DefectEdit.EnableWindow(TRUE);
            DefectEdit.ShowWindow(SW_SHOW);

            Project.SetReadOnly(FALSE);
            NewButton.EnableWindow(FALSE);
            EditButton.EnableWindow(FALSE);
            DeleteButton.EnableWindow(FALSE);
            SaveButton.EnableWindow(TRUE);
            ::SetFocus(DefectEdit);
        }
    }

    void OnDeleteDefect(UINT wNotifyCode, INT wID, HWND hWndCtl)
    {
        int position = DefectsDropList.GetCurSel();
        if (position != CB_ERR)
        {
            DefectsDropList.DeleteString(position);
            projects.erase(projects.begin()+position);
        }
        if (DefectsDropList.GetCount()>0)
        {
            DefectsDropList.SetCurSel(0);
            Project.SetWindowText(projects[0].STProject);
            EditButton.EnableWindow(TRUE);
            DeleteButton.EnableWindow(TRUE);
        }
        else
        {
            DefectsDropList.ResetContent();
            Project.SetWindowText("");
            EditButton.EnableWindow(FALSE);
            DeleteButton.EnableWindow(FALSE);
        }
        Project.SetReadOnly(TRUE);
        SaveButton.EnableWindow(FALSE);
    }

    void StopSavingDefect()
    {
        // stop before saving new record
        if (edit_pos == CB_ERR)
        {
            for (int i=0; i<DefectsDropDown.GetCount(); i++)
            {
                CString temp;
                DefectsDropDown.GetLBText(i,temp);
                DefectsDropList.AddString(temp);
            }
            if (DefectsDropList.GetCount()>0)
            {
                DefectsDropList.SetCurSel(0);
                Project.SetWindowText(projects[0].STProject);
            }
            else
            {
                DefectsDropList.ResetContent();
                Project.SetWindowText("");
            }
            
            DefectsDropDown.ShowWindow(SW_HIDE);
            DefectsDropDown.ResetContent();
            DefectsDropDown.EnableWindow(FALSE);
        }
        else // stop before saving edited record
        {
            DefectEdit.ShowWindow(SW_HIDE);
            DefectEdit.EnableWindow(FALSE);
            DefectEdit.SetWindowText("");
            Project.SetWindowText(projects[edit_pos].STProject);
        }
        DefectsDropList.EnableWindow(TRUE);
        DefectsDropList.ShowWindow(SW_SHOW);

        Project.SetReadOnly(TRUE);
        EditButton.EnableWindow(TRUE);
        DeleteButton.EnableWindow(TRUE);

        SaveButton.EnableWindow(FALSE);
        NewButton.EnableWindow(TRUE);
        
        edit_pos = CB_ERR;
        ::SetFocus(NewButton);
    }

    bool OnSetActive(COptionItem *pItem)
    {
        return true;
    }
    
    bool OnKillActive(COptionItem *pItem)
    {
        if (SaveButton.IsWindowEnabled())
        {
            if (MessageBox("You have not saved the record.\nContinue without saving?",szWinName,MB_ICONWARNING|MB_YESNO|MB_DEFBUTTON2)==IDYES)
            {
                StopSavingDefect();
                return true;
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    void OnOK()
    {
        // reading defects settings
        DefectsDropList.ResetContent();
        Settings.defects.clear();
        sort_defects();
        for (int i=0; i<projects.size(); i++)
        {
            Settings.defects.push_back(projects[i]);
        }
        projects.clear();

        CString temp;
        Link.GetWindowText(temp.GetBuffer(LINK_MAX),LINK_MAX+1);
        temp.ReleaseBuffer();
        if (!temp.IsEmpty()) Settings.DefectsLink = temp;
        else
        {
            MessageBox("\"Link to open defects\" field cannot be empty.\nPrevious value will be used",szWinName,MB_ICONERROR);
        }
        Settings.SaveDefectsSettings();
    }

    // called every time when whole sheet is closed by clicking on Cancel button
    void OnCancel() {}
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

    bool DoInit(bool FirstTime)
    {
        m_PageCaption.Attach(GetDlgItem(IDC_PAGE_CAPTION));
        m_PageCaption.SetFont(PageCaptionFont);

        if (FirstTime)
        {
            m_PageGeneral.Create(this);
            m_PageFormat.Create(this);
            m_PageDefects.Create(this);
            m_PageURLs.Create(this);
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

        NewItem = new COptionSelectionTreeCtrl::CItem(1,1);
        NewItem->SetPage(&m_PageURLs);
        NewItem->SetCaption("URLs");
        // adding new page
        AddItem(NewItem);

        NewItem = new COptionSelectionTreeCtrl::CItem(2,2);
        NewItem->SetPage(&m_PageDefects);
        NewItem->SetCaption("Defects");
        // adding new page
        AddItem(NewItem);

        NewItem = new COptionSelectionTreeCtrl::CItem(3,3);
        NewItem->SetPage(&m_PageFormat);
        NewItem->SetCaption("Format");
        // adding new page
        AddItem(NewItem);
#else
        //In order not to use icons just use the following statements: 
        AddItem(new COptionItem("General", &m_PageGeneral));
        AddItem(new COptionItem("URLs", &m_PageURLs));
        AddItem(new COptionItem("Defects",&m_PageDefects));
        AddItem(new COptionItem("Format", &m_PageFormat));
#endif

        SetFlags(OSF_HASBUTTON_OK|OSF_HASBUTTON_CANCEL);

        return CPropSheet::DoInit(FirstTime);
    };

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
            MessageBox(ErrorMessage,szWinName,MB_ICONERROR);
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
    CStatic m_PageCaption;
    CMenu HelpMenu;
#if (USE_ICONS != 0)
    CImageList ImageList;
#endif
};

#endif