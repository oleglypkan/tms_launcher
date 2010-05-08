/*
    File name: TMS_Launcher.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: January 17, 2006
*/

#include "stdafx.h"
#include "resource.h"
#include "maindlg.h"
#include "settings.h"
#include "CmdLine.h"

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: TMS_Launcher.cpp,v $$Revision: 1.19 $$Date: 2006/01/17 19:02:46Z $"; 
#endif

/* 
   The next 3 statements are used to track memory leaks in the program
   They work only in Debug mode
*/
#ifdef _DEBUG
    #define _CRTDBG_MAP_ALLOC 
    #include <stdlib.h>
    #include <crtdbg.h>
#endif

CAppModule _Module;

CString szWinName = "TMS Launcher";
const char* MutexName = "TMS_Launcher";
const char* AutoRunKeyName = "TMS Launcher";
const UINT WM_TMS_LAUNCHER_ACTIVATE = ::RegisterWindowMessage("TMS_Launcher_Activate");
CSettings Settings("Software\\Winchester\\TMS Launcher",
                   "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                   AutoRunKeyName, "Defects", "Format", "Links");
bool GetVersionInfo(CString &string, WORD Language, WORD CodePage,
                    const char* StringName = "ProductVersion", UINT VersionDigits = 2);

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
/* 
   The next statement is used to track memory leaks in the program
   It works only in Debug mode
*/
    #ifdef _DEBUG
        _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    #endif

    HANDLE hMutex;

    if (lstrlen(lpstrCmdLine) == 0)
    {
        // checking for previous instance of the program running
        hMutex = CreateMutex(NULL, TRUE, MutexName);
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            CloseHandle(hMutex);
            PostMessage(HWND_BROADCAST,WM_TMS_LAUNCHER_ACTIVATE,0,0);
            return 0;
        }

        szWinName += " ";
        GetVersionInfo(szWinName,0x0409,0x04b0);

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
    
        Settings.ImportSettings("Software\\Winchester\\TMS_Launcher");
    }

    if (Settings.SettingsAvailable())
    {
        Settings.LoadSettings();
    }
    else
    {
        Settings.SaveGeneralSettings();
        Settings.SaveFormatSettings();
        Settings.SaveDefectsSettings();
        Settings.SaveLinksSettings();
    }

    if (lstrlen(lpstrCmdLine) != 0)
    {
        CmdLine CommandLine;
        CommandLine.ParseCmdLine(lpstrCmdLine);
        return 0;
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

    HACCEL hAccelTable;
    hAccelTable = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCELERATORS));
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0 ) > 0)
    {
        if (!(TranslateAccelerator(hWnd, hAccelTable, &msg)||IsDialogMessage(hWnd,&msg)))
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

// function appends requested version infomation to the string
bool GetVersionInfo(CString &string, WORD Language, WORD CodePage,
                    const char* StringName, UINT VersionDigits)
{
    // reading version information from the resource
    DWORD Handle;
    DWORD VersionInfoSize = GetFileVersionInfoSize("TMS_Launcher.exe",&Handle);
    if (VersionInfoSize)
    {
        char *VersionInfo = new char[VersionInfoSize];
        CString FileName;
        GetModuleFileName(NULL,FileName.GetBuffer(_MAX_PATH),_MAX_PATH);
        FileName.ReleaseBuffer();
        if (GetFileVersionInfo(FileName.GetBuffer(_MAX_PATH),0,VersionInfoSize,VersionInfo))
        {
            char *SubBlockBuffer;
            UINT SubBlockSize;
            CString SubBlock;

            SubBlock.Format("\\StringFileInfo\\%04x%04x\\%s",Language,CodePage,StringName);

            if (VerQueryValue(VersionInfo,SubBlock.GetBuffer(SubBlock.GetLength()),(LPVOID *)&SubBlockBuffer,&SubBlockSize))
            {
                CString Version = SubBlockBuffer;
                if (lstrcmp(StringName,"ProductVersion")==0 || lstrcmp(StringName,"FileVersion")==0)
                {
                    Version.Remove(' ');
                    Version.Replace(',','.');
                    if (VersionDigits > 0 || VersionDigits < 4)
                    {
                        Version.Delete(VersionDigits*2-1,Version.GetLength()-(VersionDigits*2-1));
                    }
                }
                string += Version;
                delete [] VersionInfo;
                return true;
            }
            else
            {
                delete [] VersionInfo;
                return false;
            }
        }
        else
        {
            delete [] VersionInfo;
            return false;
        }
    }
    else
    {
        return false;
    }
}