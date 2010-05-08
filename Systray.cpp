#include "stdafx.h"
#include "systray.h"

const UINT WM_TASKBARCREATED = ::RegisterWindowMessage("TaskbarCreated");

void CreateSysTrayIcon(HWND hwnd, HICON IconHandle, const char *TipText)
{
    NOTIFYICONDATA nidata;
    nidata.cbSize = sizeof(NOTIFYICONDATA);
    nidata.hWnd = hwnd;
    nidata.uID = 1;
    nidata.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nidata.uCallbackMessage = WM_MYICONNOTIFY;
    nidata.hIcon = IconHandle;
    strncpy(nidata.szTip,TipText,64);
    nidata.szTip[63] = '\0';
    Shell_NotifyIcon(NIM_ADD, &nidata);
}

void ChangeSysTrayIcon(HWND hwnd, HICON IconHandle, const char *TipText)
{
    NOTIFYICONDATA nidata;
    nidata.cbSize = sizeof(NOTIFYICONDATA);
    nidata.hWnd = hwnd;
    nidata.uID = 1;
    nidata.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nidata.uCallbackMessage = WM_MYICONNOTIFY;
    nidata.hIcon = IconHandle;
    strncpy(nidata.szTip,TipText,64);
    nidata.szTip[63] = '\0';
    Shell_NotifyIcon(NIM_MODIFY, &nidata);
}

void DeleteSysTrayIcon(HWND hwnd)
{
    NOTIFYICONDATA nidata;
    nidata.cbSize = sizeof(NOTIFYICONDATA);
    nidata.hWnd = hwnd;
    nidata.uID = 1;
    Shell_NotifyIcon(NIM_DELETE, &nidata);
}
