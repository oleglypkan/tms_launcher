/*
    File name: TMS_Launcher.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: November 24, 2006
*/

#include "stdafx.h"
#include "resource.h"
#include "maindlg.h"
#include "settings.h"
#include "CmdLine.h"

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: TMS_Launcher.cpp,v $$Revision: 1.23 $$Date: 2007/12/17 17:10:35Z $"; 
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
                   AutoRunKeyName, "Defects", "Format", "Links", "SoftTest", "History");
bool GetVersionInfo(CString &string, WORD Language, WORD CodePage,
                    const char* StringName = "ProductVersion", UINT VersionDigits = 2,
                    const CString &ModulePath = "");

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

    // setting correct current directory in case if the program was started from a shortcut or a registry key
    char CurrentDirName[MAX_PATH+1], *FilePart;
    GetModuleFileName(NULL,CurrentDirName,MAX_PATH);
    GetFullPathName(CurrentDirName,MAX_PATH,CurrentDirName,&FilePart);
    FilePart[0]='\0';
    SetCurrentDirectory(CurrentDirName);

    if (Settings.SettingsAvailable())
    {
        Settings.AddingNewURLs(); // Adding new URLs
        Settings.LoadSettings();
    }
    else
    {
        Settings.SaveGeneralSettings();
        Settings.SaveFormatSettings();
        Settings.SaveDefectsSettings();
        Settings.SaveLinksSettings();
        Settings.SaveSoftTestSettings();
        Settings.SaveHistorySettings();
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
                    const char* StringName, UINT VersionDigits, const CString &ModulePath)
{
    // reading version information from the resource
    DWORD Handle;
    CString FileName;

    if (ModulePath.IsEmpty())
    {
        GetModuleFileName(NULL,FileName.GetBuffer(MAX_PATH),MAX_PATH);
        FileName.ReleaseBuffer();
    }
    else
    {
        FileName = ModulePath;
    }

    DWORD VersionInfoSize = GetFileVersionInfoSize(FileName,&Handle);
    if (VersionInfoSize)
    {
        char *VersionInfo = new char[VersionInfoSize];
        if (GetFileVersionInfo(FileName.GetBuffer(MAX_PATH),0,VersionInfoSize,VersionInfo))
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
                    if (VersionDigits > 0 && VersionDigits < 4)
                    {
                        int pos = -1, i = 0;
                        do
                        {
                            pos = Version.Find('.',pos+1);
                            i++;
                        } while (i != VersionDigits);

                        Version = Version.Left(pos);
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