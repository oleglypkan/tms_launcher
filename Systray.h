#ifndef SYSTRAY_H_INCLUDED
#define SYSTRAY_H_INCLUDED

#include <windows.h>

#ifdef INCLUDE_VERID
 static char Systray_h[]="@(#)$RCSfile: Systray.h,v $$Revision: 1.2 $$Date: 2005/05/25 16:18:59Z $";
#endif

const int WM_MYICONNOTIFY = WM_USER + 123;
extern const UINT WM_TASKBARCREATED;

void CreateSysTrayIcon(HWND hwnd, HICON IconHandle, const char *TipText);
void ChangeSysTrayIcon(HWND hwnd, HICON IconHandle, const char *TipText);
void DeleteSysTrayIcon(HWND hwnd);

#endif