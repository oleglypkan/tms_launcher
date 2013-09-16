/*
    File name: Registry.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: January 17, 2006
*/

#ifndef REGISTRY_H_INCLUDED
#define REGISTRY_H_INCLUDED

#include <windows.h>

#ifdef INCLUDE_VERID
 static char Registry_h[]="@(#)$RCSfile: Registry.h,v $$Revision: 1.7 $$Date: 2007/12/17 17:09:34Z $";
#endif

class Registry 
{
    HKEY hMainKey,hKey;
    DWORD dwDisposition;
public:
    Registry(HKEY hMainKeyParam);
    void AddValue(LPCTSTR lpSubKey,LPCTSTR lpValueName,DWORD dwType,const BYTE *lpData,DWORD dwSizeOfData);
    void DeleteKey(LPCTSTR lpSubKey, LPCTSTR lpValueName); // dwType-REG_DWORD,REG_BINARY,REG_SZ,etc.
    bool DeleteKeyIncludingSubKeys(HKEY hKey, LPCTSTR lpSubKey);
    void DeleteAllSubKeys(LPCTSTR lpKeyName);
    void DeleteValue(LPCTSTR lpSubKey, LPCTSTR lpValueName);
    bool ReadValue(LPCTSTR lpSubKey,LPCTSTR lpValueName,DWORD dwType,LPBYTE lpData,DWORD dwSizeOfData);
    DWORD GetNumberOfValues(LPCTSTR lpSubKey, DWORD *lpcMaxValueNameLen = NULL, DWORD *lpcMaxValueLen = NULL);
    bool GetValueName(LPCTSTR lpSubKey, DWORD index, LPSTR ValueName, DWORD *ValueNameLength, DWORD *type = NULL);
    bool GetSubKeyName(LPCTSTR lpSubKey, DWORD index, LPSTR SubKeyName);
    bool KeyPresent(LPCTSTR lpSubKey);
};

#endif