#include "stdafx.h"
#include "registry.h"

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
                        !=ERROR_SUCCESS) return false;
    RegCloseKey(hKey);
    return true;
};
