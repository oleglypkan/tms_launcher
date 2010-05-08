// TMS_Launcher.cpp : main source file for TMS_Launcher.exe
//

#include "stdafx.h"
#include "resource.h"
#include "maindlg.h"
#include "settings.h"

CAppModule _Module;

const char* szWinName = "TMS Launcher v1.4";
const char* MutexName = "TMS_Launcher";
const char* AutoRunKeyName = "TMS Launcher";
const UINT WM_TMS_LAUNCHER_ACTIVATE = ::RegisterWindowMessage("TMS_Launcher_Activate");
CSettings Settings("Software\\Winchester\\TMS_Launcher",
                   "Software\\Microsoft\\Windows\\CurrentVersion\\Run",AutoRunKeyName);

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
    // checking for previous instance of the program running
    HANDLE hMutex = CreateMutex(NULL, TRUE, MutexName);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(hMutex);
        PostMessage(HWND_BROADCAST,WM_TMS_LAUNCHER_ACTIVATE,0,0);
        return 0;
    }

    HINSTANCE RichEditLibrary = LoadLibrary (CRichEditCtrl::GetLibraryName());
    if (!RichEditLibrary)
    {
        MessageBox(NULL,"Cannot load Richedit library",szWinName,MB_OK);
        return 0;
    }

    HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//  HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    ATLASSERT(SUCCEEDED(hRes));

    // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
    ::DefWindowProc(NULL, 0, 0, 0L);

    AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);  // add flags to support other controls

    hRes = _Module.Init(NULL, hInstance);
    ATLASSERT(SUCCEEDED(hRes));
    
    if (Settings.SettingsAvailable())
    {
        Settings.LoadSettings();
    }
    else
    {
        Settings.SaveSettings();
    }
    
    CMainDlg dlgMain;

    HWND hWnd = dlgMain.Create(NULL);

	if (Settings.Minimize)
    {
        dlgMain.ShowWindow(SW_HIDE);
    }
    else
    {
        dlgMain.ShowWindow(SW_SHOWNORMAL);
    }

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0 ) > 0)
    {
        if (!IsDialogMessage(hWnd,&msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    _Module.Term();
    ::CoUninitialize();

    CloseHandle(hMutex);

    return 0;
}
