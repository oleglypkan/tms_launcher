/*
    File name: Systray.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: January 17, 2006
*/

#ifndef SYSTRAY_H_INCLUDED
#define SYSTRAY_H_INCLUDED

#include <windows.h>

#ifdef INCLUDE_VERID
 static char Systray_h[]="@(#)$RCSfile: Systray.h,v $$Revision: 1.3 $$Date: 2006/01/17 15:56:50Z $";
#endif

const int WM_MYICONNOTIFY = WM_USER + 123;
extern const UINT WM_TASKBARCREATED;

void CreateSysTrayIcon(HWND hwnd, HICON IconHandle, const char *TipText);
void ChangeSysTrayIcon(HWND hwnd, HICON IconHandle, const char *TipText);
void DeleteSysTrayIcon(HWND hwnd);

#endif