#include "stdafx.h"
#include "settings.h"

extern const char* szWinName;
UINT ViewTaskHotKeyID = 1;
UINT ViewChildTasksHotKeyID = 2;

CSettingsDlg::CSettingsDlg(CSettings* TMS_Settings)
{
    Settings = TMS_Settings;
}

LRESULT CSettingsDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    switch (Settings->TMS)
    {
        case 0:
            SendDlgItemMessage(IDC_USUAL_TMS,BM_SETCHECK,BST_CHECKED,0);
            break;
        case 1:
            SendDlgItemMessage(IDC_ALT_TMS,BM_SETCHECK,BST_CHECKED,0);
            break;
    }
    switch (Settings->RightClickAction)
    {
        case 0:
            SendDlgItemMessage(IDC_TASK_RADIO,BM_SETCHECK,BST_CHECKED,0);
            break;
        case 1:
            SendDlgItemMessage(IDC_CHILD_RADIO,BM_SETCHECK,BST_CHECKED,0);
            break;
    }
    if (Settings->Expand) SendDlgItemMessage(IDC_EXPANDED,BM_SETCHECK,BST_CHECKED,0);
    if (Settings->AutoRun) SendDlgItemMessage(IDC_AUTORUN,BM_SETCHECK,BST_CHECKED,0);
    if (Settings->Minimize) SendDlgItemMessage(IDC_MINIMIZE,BM_SETCHECK,BST_CHECKED,0);
    if (Settings->SingleClick) SendDlgItemMessage(IDC_SINGLE_CLICK,BM_SETCHECK,BST_CHECKED,0);
    SendDlgItemMessage(VIEW_TASK_HOTKEY,HKM_SETHOTKEY,Settings->ViewTaskHotKey,0);
    UnregisterHotKey(GetParent(),ViewTaskHotKeyID);
    SendDlgItemMessage(VIEW_CHILD_TASKS_HOTKEY,HKM_SETHOTKEY,Settings->ViewChildTasksHotKey,0);
    UnregisterHotKey(GetParent(),ViewChildTasksHotKeyID);

    return TRUE;
}

void CSettingsDlg::OnOK(UINT wNotifyCode, INT wID, HWND hWndCtl)
{
    Settings->AutoRun=(SendDlgItemMessage(IDC_AUTORUN,BM_GETCHECK,0,0)==BST_CHECKED);
    Settings->Expand=(SendDlgItemMessage(IDC_EXPANDED,BM_GETCHECK,0,0)==BST_CHECKED);
    Settings->Minimize=(SendDlgItemMessage(IDC_MINIMIZE,BM_GETCHECK,0,0)==BST_CHECKED);
    Settings->SingleClick=(SendDlgItemMessage(IDC_SINGLE_CLICK,BM_GETCHECK,0,0)==BST_CHECKED);

    if (SendDlgItemMessage(IDC_USUAL_TMS,BM_GETCHECK,0,0)==BST_CHECKED)
        Settings->TMS = 0;
    else
        Settings->TMS = 1;

    if (SendDlgItemMessage(IDC_TASK_RADIO,BM_GETCHECK,0,0)==BST_CHECKED)
        Settings->RightClickAction = 0;
    else
        Settings->RightClickAction = 1;
    
    UINT ViewTaskHotKey=(UINT)SendDlgItemMessage(VIEW_TASK_HOTKEY,HKM_GETHOTKEY,0,0);
    UINT ViewChildTasksHotKey=(UINT)SendDlgItemMessage(VIEW_CHILD_TASKS_HOTKEY,HKM_GETHOTKEY,0,0);
    
    if ((ViewTaskHotKey == ViewChildTasksHotKey)&&(ViewTaskHotKey))
    {
        MessageBox("Hotkeys have to be different",szWinName,MB_ICONERROR);
        return;
    }

    BOOL RegSuccess[2] = {true, true};
    if (ViewTaskHotKey)
    {
        RegSuccess[0] = RegisterHotKey(GetParent(),ViewTaskHotKeyID, (!(ViewTaskHotKey&0x500)?
                        HIBYTE(LOWORD(ViewTaskHotKey)):((ViewTaskHotKey&0x500)<0x500?
                        HIBYTE(LOWORD(ViewTaskHotKey))^5:HIBYTE(LOWORD(ViewTaskHotKey)))),
                        LOBYTE(LOWORD(ViewTaskHotKey)));
        if(!RegSuccess[0])
        {
            MessageBox("Hotkey entered in \"HotKey for View Task\" field\nis already registered by another program.\nPlease choose another one",szWinName,MB_ICONERROR);
        }        
    }
    if (ViewChildTasksHotKey)
    {
        RegSuccess[1] = RegisterHotKey(GetParent(),ViewChildTasksHotKeyID, (!(ViewChildTasksHotKey&0x500)?
                        HIBYTE(LOWORD(ViewChildTasksHotKey)):((ViewChildTasksHotKey&0x500)<0x500?
                        HIBYTE(LOWORD(ViewChildTasksHotKey))^5:HIBYTE(LOWORD(ViewChildTasksHotKey)))),
                        LOBYTE(LOWORD(ViewChildTasksHotKey)));
        if(!RegSuccess[1])
        {
            MessageBox("Hotkey entered in \"HotKey for View Child Task\" field\nis already registered by another program.\nPlease choose another one",szWinName,MB_ICONERROR);
        }
    }
    if ((!RegSuccess[0])||(!RegSuccess[1]))
    {
        UnregisterHotKey(GetParent(),ViewTaskHotKeyID);
        UnregisterHotKey(GetParent(),ViewChildTasksHotKeyID);
        return;
    }

    Settings->ViewTaskHotKey = ViewTaskHotKey;
    Settings->ViewChildTasksHotKey = ViewChildTasksHotKey;

    Settings->SaveSettings();
    EndDialog(wID);
}

void CSettingsDlg::OnCancel(UINT wNotifyCode, INT wID, HWND hWndCtl)
{
    if (Settings->ViewTaskHotKey)
    {
        if (!RegisterHotKey(GetParent(),ViewTaskHotKeyID,(!(Settings->ViewTaskHotKey&0x500)?
                       HIBYTE(LOWORD(Settings->ViewTaskHotKey)):((Settings->ViewTaskHotKey&0x500)<0x500?
                       HIBYTE(LOWORD(Settings->ViewTaskHotKey))^5:HIBYTE(LOWORD(Settings->ViewTaskHotKey)))),
                       LOBYTE(LOWORD(Settings->ViewTaskHotKey))))
            MessageBox("Hotkey used to View Task is already registered by another program.\nPlease enter another hotkey in TMS Launcher Settings window",szWinName,MB_OK|MB_ICONERROR);
    }
    if (Settings->ViewChildTasksHotKey)
    {
        if (!RegisterHotKey(GetParent(),ViewChildTasksHotKeyID,(!(Settings->ViewChildTasksHotKey&0x500)?
                        HIBYTE(LOWORD(Settings->ViewChildTasksHotKey)):((Settings->ViewChildTasksHotKey&0x500)<0x500?
                        HIBYTE(LOWORD(Settings->ViewChildTasksHotKey))^5:HIBYTE(LOWORD(Settings->ViewChildTasksHotKey)))),
                        LOBYTE(LOWORD(Settings->ViewChildTasksHotKey))))
            MessageBox("Hotkey used to View Child Tasks is already registered by another program.\nPlease enter another hotkey in TMS Launcher Settings window",szWinName,MB_OK|MB_ICONERROR);
    }

    EndDialog(wID);
}

CSettings::CSettings(const char* RegKey, const char* AutoRunRegKey, const char* AutoRunValName)
{
    AutoRun = false;
    Expand = false;
    Minimize = false;
    SingleClick = false;
    RightClickAction = 0;
    TMS = 0;
    ViewTaskHotKey = 0;
    ViewChildTasksHotKey = 0;
    RegistryKey = RegKey;
    AutoRunRegistryKey = AutoRunRegKey;
    AutoRunValueName = AutoRunValName;    
}

void CSettings::LoadSettings()
{
    DWORD DWbuf;
    DWORD DWordSize;
    
    DWordSize=sizeof(DWORD); // will be changed by ReadValue()
    Reg.ReadValue(RegistryKey,"Use TMS",REG_DWORD,(LPBYTE)&TMS,DWordSize);
    if ((TMS < 0)||(TMS > 1)) TMS = 0;

    DWordSize=sizeof(DWORD); // will be changed by ReadValue()
    Reg.ReadValue(RegistryKey,"Expand at start",REG_DWORD,(LPBYTE)&DWbuf,DWordSize);
    Expand = (DWbuf != 0);

    DWordSize=sizeof(DWORD); // will be changed by ReadValue()
    Reg.ReadValue(RegistryKey,"Minimize at start",REG_DWORD,(LPBYTE)&DWbuf,DWordSize);
    Minimize = (DWbuf != 0);

    DWordSize=sizeof(DWORD); // will be changed by ReadValue()
    Reg.ReadValue(RegistryKey,"Single click",REG_DWORD,(LPBYTE)&DWbuf,DWordSize);
    SingleClick = (DWbuf != 0);

    DWordSize=sizeof(DWORD); // will be changed by ReadValue()
    Reg.ReadValue(RegistryKey,"RightClickAction",REG_DWORD,(LPBYTE)&RightClickAction,DWordSize);
    if ((RightClickAction < 0)||(RightClickAction > 1))
        RightClickAction = 0;

    char lpFileName[MAX_PATH+3];
    DWORD size = MAX_PATH+2;
    AutoRun = Reg.ReadValue(AutoRunRegistryKey,AutoRunValueName,REG_SZ,(LPBYTE)lpFileName,size);
    if (AutoRun)
    {
        strcpy(lpFileName,"\"");
        GetModuleFileName(NULL,lpFileName+1,MAX_PATH);
        strcat(lpFileName,"\"");
        Reg.AddValue(AutoRunRegistryKey,AutoRunValueName,REG_SZ,(const BYTE*)lpFileName,lstrlen(lpFileName)+1);
    }

    DWordSize=sizeof(DWORD);
    Reg.ReadValue(RegistryKey,"ViewTaskHotkey",REG_DWORD,(LPBYTE)&ViewTaskHotKey,DWordSize);

    DWordSize=sizeof(DWORD);
    Reg.ReadValue(RegistryKey,"ViewChildTasksHotkey",REG_DWORD,(LPBYTE)&ViewChildTasksHotKey,DWordSize);
}

void CSettings::SaveSettings()
{
    DWORD buf;
    buf = Expand ? 1:0;
    Reg.AddValue(RegistryKey,"Expand at start",REG_DWORD,(const BYTE*)&buf,sizeof(DWORD));

    buf = Minimize ? 1:0;
    Reg.AddValue(RegistryKey,"Minimize at start",REG_DWORD,(const BYTE*)&buf,sizeof(DWORD));

    buf = SingleClick ? 1:0;
    Reg.AddValue(RegistryKey,"Single click",REG_DWORD,(const BYTE*)&buf,sizeof(DWORD));

    Reg.AddValue(RegistryKey,"RightClickAction",REG_DWORD,(const BYTE*)&RightClickAction,sizeof(DWORD));

    Reg.AddValue(RegistryKey,"Use TMS",REG_DWORD,(const BYTE*)&TMS,sizeof(DWORD));

    if (AutoRun)
    {
        char lpFileName[MAX_PATH+3];
        strcpy(lpFileName,"\"");
        GetModuleFileName(NULL,lpFileName+1,MAX_PATH);
        strcat(lpFileName,"\"");
        Reg.AddValue(AutoRunRegistryKey,AutoRunValueName,REG_SZ,(const BYTE*)lpFileName,lstrlen(lpFileName)+1);
    }
    else Reg.DeleteValue(AutoRunRegistryKey,AutoRunValueName);

    Reg.AddValue(RegistryKey,"ViewTaskHotkey",REG_DWORD,(const BYTE*)&ViewTaskHotKey,sizeof(DWORD));
    Reg.AddValue(RegistryKey,"ViewChildTasksHotkey",REG_DWORD,(const BYTE*)&ViewChildTasksHotKey,sizeof(DWORD));
}

bool CSettings::SettingsAvailable()
{
    if (Reg.KeyPresent(RegistryKey))
    {
        return true;
    }
    else
    {
        return false;
    }
}