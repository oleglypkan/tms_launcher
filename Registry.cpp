/*
    File name: Registry.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: January 17, 2006
*/

#include "stdafx.h"
#include <vector>
#include "registry.h"

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: Registry.cpp,v $$Revision: 1.7 $$Date: 2007/12/17 17:09:23Z $"; 
#endif

void Registry::AddValue(LPCTSTR lpSubKey,LPCTSTR lpValueName,DWORD dwType,const BYTE *lpData,DWORD dwSizeOfData)
{
    RegCreateKeyEx(hMainKey,                   // handle of an open key 
                   lpSubKey,                   // address of subkey name 
                   0,                          // reserved 
                   "",                         // address of class string 
                   REG_OPTION_NON_VOLATILE ,   // special options flag 
                   KEY_WRITE,                  // desired security access 
                   NULL,                       // address of key security structure 
                   &hKey,                      // address of buffer for opened handle  
                   &dwDisposition);            // address of disposition value buffer 

    RegSetValueEx(hKey,                        // handle of key to set value for  
                  lpValueName,                 // address of value to set 
                  0,                           // reserved 
                  dwType,                      // flag for value type 
                  lpData,                      // address of value data 
                  dwSizeOfData);               // size of value data 
    RegCloseKey(hKey);
};

void Registry::DeleteKey(LPCTSTR lpSubKey, LPCTSTR lpValueName)
{
    RegCreateKeyEx(hMainKey,                   // handle of an open key 
                   lpSubKey,                   // address of subkey name 
                   0,                          // reserved 
                   "",                         // address of class string 
                   REG_OPTION_NON_VOLATILE ,   // special options flag 
                   DELETE,                     // desired security access 
                   NULL,                       // address of key security structure 
                   &hKey,                      // address of buffer for opened handle  
                   &dwDisposition);            // address of disposition value buffer 
    RegDeleteKey(hKey, lpValueName);
    RegCloseKey(hKey);
};

bool Registry::DeleteKeyIncludingSubKeys(HKEY hKey, LPCTSTR lpSubKey)
{
    if (SHDeleteKey(hKey, lpSubKey) == ERROR_SUCCESS)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Registry::DeleteAllSubKeys(LPCTSTR lpKeyName)
{
    DWORD SubKeyIndex = 0;
    CString SubKeyName = "";
    std::vector<CString> SubKeys;

    while (GetSubKeyName(lpKeyName,SubKeyIndex,SubKeyName.GetBuffer(255)))
    {
        SubKeyName.ReleaseBuffer();
        SubKeys.push_back(SubKeyName);
        SubKeyIndex++;
    }
    for (SubKeyIndex = 0; SubKeyIndex < SubKeys.size(); SubKeyIndex++)
    {
        DeleteKey(lpKeyName,SubKeys[SubKeyIndex]);
    }
}

void Registry::DeleteValue(LPCTSTR lpSubKey, LPCTSTR lpValueName)
{
    RegCreateKeyEx(hMainKey,                           // handle of an open key 
                   lpSubKey,                           // address of subkey name 
                   0,                                  // reserved 
                   "",                                 // address of class string 
                   REG_OPTION_NON_VOLATILE,            // special options flag 
                   KEY_SET_VALUE,                      // desired security access 
                   NULL,                               // address of key security structure 
                   &hKey,                              // address of buffer for opened handle  
                   &dwDisposition);                    // address of disposition value buffer 
    RegDeleteValue(hKey, lpValueName);
    RegCloseKey(hKey);
}

bool Registry::KeyPresent(LPCTSTR lpSubKey)
{
    bool Res = (RegOpenKeyEx(hMainKey,                         // handle of an open key 
                            lpSubKey,                         // address of subkey name 
                            0,                                // reserved 
                            KEY_READ,                         // desired security access 
                            &hKey)==ERROR_SUCCESS);
    RegCloseKey(hKey);
    return Res;
}

bool Registry::ReadValue(LPCTSTR lpSubKey,LPCTSTR lpValueName,DWORD dwType,LPBYTE lpData,DWORD dwSizeOfData)
{
    RegCreateKeyEx(hMainKey,                               // handle of an open key 
                   lpSubKey,                               // address of subkey name 
                   0,                                      // reserved 
                   "",                                     // address of class string 
                   REG_OPTION_NON_VOLATILE,                // special options flag 
                   KEY_READ,                               // desired security access 
                   NULL,                                   // address of key security structure 
                   &hKey,                                  // address of buffer for opened handle  
                   &dwDisposition);                        // address of disposition value buffer 

    if (RegQueryValueEx(hKey,                              // handle of key to query 
                        lpValueName,                       // address of name of value to query 
                        NULL,                              // reserved 
                        &dwType,                           // address of buffer for value type 
                        lpData,                            // address of data buffer 
                        &dwSizeOfData)                     // address of data buffer size 
                        !=ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return false;
    }
    RegCloseKey(hKey);
    return true;
};

DWORD Registry::GetNumberOfValues(LPCTSTR lpSubKey, DWORD *lpcMaxValueNameLen, DWORD *lpcMaxValueLen)
{
    DWORD values;
    RegCreateKeyEx(hMainKey,lpSubKey,0,"",REG_OPTION_NON_VOLATILE,KEY_QUERY_VALUE,NULL,&hKey,
                   &dwDisposition);
    if (RegQueryInfoKey(hKey,NULL,NULL,NULL,NULL,NULL,NULL,&values,lpcMaxValueNameLen,lpcMaxValueLen,NULL,NULL) == ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return values;
    }
    else
    {
        RegCloseKey(hKey);
        return 0;
    }
}

bool Registry::GetValueName(LPCTSTR lpSubKey, DWORD index, LPSTR ValueName, DWORD *ValueNameLength,
                            DWORD *type)
{
    RegCreateKeyEx(hMainKey,lpSubKey,0,"",REG_OPTION_NON_VOLATILE,KEY_QUERY_VALUE,
                   NULL,&hKey,&dwDisposition);

    if (RegEnumValue(hKey,index,ValueName,ValueNameLength,NULL,type,NULL,NULL) == ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return true;
    }
    else
    {
        RegCloseKey(hKey);
        return false;
    }    
}

bool Registry::GetSubKeyName(LPCTSTR lpSubKey, DWORD index, LPSTR SubKeyName)
{
    RegCreateKeyEx(hMainKey,lpSubKey,0,"",REG_OPTION_NON_VOLATILE,KEY_ENUMERATE_SUB_KEYS,
                   NULL,&hKey,&dwDisposition);
    DWORD lpcName;
    FILETIME FileTime;
    LONG Res = RegEnumKeyEx(hKey,index,SubKeyName,&lpcName,NULL,NULL,NULL,&FileTime);
    RegCloseKey(hKey);
    if (Res == ERROR_SUCCESS)
    {
        return true;
    }
    else
    {
        return false;
    }
}
