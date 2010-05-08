#ifndef REGISTRY_H_INCLUDED
#define REGISTRY_H_INCLUDED

#include <windows.h>

class Registry 
{
    HKEY hMainKey,hKey;
    DWORD dwDisposition;
public:
    Registry() {hMainKey=HKEY_CURRENT_USER; hKey=NULL;}
    void AddValue(LPCTSTR lpSubKey,LPCTSTR lpValueName,DWORD dwType,const BYTE *lpData,DWORD dwSizeOfData);
    void DeleteKey(LPCTSTR lpSubKey, LPCTSTR lpValueName); // dwType-REG_DWORD,REG_BINARY,REG_SZ,etc.
    void DeleteValue(LPCTSTR lpSubKey, LPCTSTR lpValueName);
    bool ReadValue(LPCTSTR lpSubKey,LPCTSTR lpValueName,DWORD dwType,LPBYTE lpData,DWORD dwSizeOfData);
    bool KeyPresent(LPCTSTR lpSubKey);
};

#endif