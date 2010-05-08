#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

#include "resource.h"
#include "Registry.h"

class CSettings
{
public:
    bool AutoRun;
    bool Expand;
    bool Minimize;
    bool SingleClick;
    int RightClickAction;
    int TMS; // 0 - usual TMS, 1 - alternative TMS
    UINT ViewTaskHotKey;
    UINT ViewChildTasksHotKey;
    CSettings(const char* RegKey, const char* AutoRunRegKey, const char* AutoRunValName);
    void LoadSettings();
    void SaveSettings();
    bool SettingsAvailable();
protected:
    Registry Reg;
    CString RegistryKey;
    CString AutoRunRegistryKey;
    CString AutoRunValueName;
};

class CSettingsDlg: public CDialogImpl<CSettingsDlg>
{
public:
    enum { IDD = SETTINGS_DIALOG};
    
    BEGIN_MSG_MAP(CSettingsDlg)
        MESSAGE_HANDLER_EX(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER_EX(IDOK,OnOK)
        COMMAND_ID_HANDLER_EX(IDCANCEL,OnCancel)
    END_MSG_MAP()

    CSettingsDlg(CSettings* TMS_Settings);
    CSettings* Settings;
    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    void OnOK(UINT wNotifyCode, INT wID, HWND hWndCtl);
    void OnCancel(UINT wNotifyCode, INT wID, HWND hWndCtl);
};

#endif