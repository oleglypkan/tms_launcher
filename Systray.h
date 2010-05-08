#ifndef SYSTRAY_H_INCLUDED
#define SYSTRAY_H_INCLUDED

#include <windows.h>

const int WM_MYICONNOTIFY = WM_USER + 123;
extern const UINT WM_TASKBARCREATED;

void CreateSysTrayIcon(HWND hwnd, HICON IconHandle, const char *TipText);
void ChangeSysTrayIcon(HWND hwnd, HICON IconHandle, const char *TipText);
void DeleteSysTrayIcon(HWND hwnd);

#endif