#include "stdafx.h"
#include "settings.h"
#include "About.h"

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: settings.cpp,v $$Revision: 1.17 $$Date: 2005/07/08 10:02:33Z $"; 
#endif

extern CString szWinName;
UINT LINK_MAX = 1024;

bool GetVersionInfo(CString &string, WORD Language, WORD CodePage,
                    const char* StringName = "ProductVersion", UINT VersionDigits = 2);

CSettings::CSettings(const char* RegKey, const char* AutoRunRegKey, const char* AutoRunValName, 
                     const char* DefectsSubKeyName, const char* FormatSubKeyName,
                     const char* LinksSubKeyName):Reg(HKEY_CURRENT_USER)
{
    AutoRun = false;
    Expand = false;
    Minimize = false;
    SingleClick = false;
    RightClickAction = 0;
    xPos = -1; // for centered window
    yPos = -1; // for centered window
    RegistryKey = RegKey;
    DefectsSubKey = DefectsSubKeyName;
    FormatSubKey = FormatSubKeyName;
    LinksSubKey = LinksSubKeyName;
    AutoRunRegistryKey = AutoRunRegKey;
    AutoRunValueName = AutoRunValName;
    BrowserPath = "";
    DefaultBrowser = false;
    TasksSeparators = ";,\n";
    Separators = " _-*+|:~#@$%^\t";
    MinClientName = 0;
    MaxClientName = 8;
    MinIDName = 1;
    MaxIDName = 6;
    MinExt = 0;
    MaxExt = 1;
    MinTaskName = MinClientName+MinIDName;
    MaxTaskName = MaxClientName+MaxIDName+MaxExt+2; // 2 - separators
    DefectsLink = "http://qa.isd.dp.ua/softtest/defect/%PROJECT%/%ID%/";
    defects.push_back(defect("","ST_LABGUI_SYNCH"));
    defects.push_back(defect("CMN","ST_COMMONPROD_SYNCH"));
    defects.push_back(defect("CMNA","ST_COMMONASCII_SYNCH"));
    defects.push_back(defect("LAB","ST_LABGUI_SYNCH"));
    defects.push_back(defect("LABA","ST_LABASCII_SYNCH"));
    defects.push_back(defect("LABASC","ST_LABASCII_SYNCH"));
    defects.push_back(defect("LABQC","ST_LABQCASCII_SYNCH"));
    defects.push_back(defect("LABQCASC","ST_LABQCASCII_SYNCH"));
    defects.push_back(defect("MIC","ST_MICGUI_SYNCH"));
    defects.push_back(defect("MICA","ST_MICASCII_SYNCH"));
    defects.push_back(defect("MICASC","ST_MICASCII_SYNCH"));
    defects.push_back(defect("MICQC","ST_MICQCASCII_SYNCH"));
    defects.push_back(defect("MICQCASC","ST_MICQCASCII_SYNCH"));
    defects.push_back(defect("SEC","ST_SECURITY"));
    defects.push_back(defect("STO","ST_SOFTSTORE"));
    defects.push_back(defect("STORE","ST_SOFTSTORE"));
    defects.push_back(defect("SUP","ST_ISD_SUPPORT"));
}

void CSettings::LoadSettings()
{
    DWORD DWbuf;
    DWORD DWordSize;

//  reading general settings
    DWordSize=sizeof(DWORD); // will be changed by ReadValue()
    Reg.ReadValue(RegistryKey,"Expand at start",REG_DWORD,(LPBYTE)&DWbuf,DWordSize);
    Expand = (DWbuf != 0);

    DWordSize=sizeof(DWORD); // will be changed by ReadValue()
    Reg.ReadValue(RegistryKey,"Minimize at start",REG_DWORD,(LPBYTE)&DWbuf,DWordSize);
    Minimize = (DWbuf != 0);

    DWordSize=sizeof(DWORD); // will be changed by ReadValue()
    Reg.ReadValue(RegistryKey,"Single click",REG_DWORD,(LPBYTE)&DWbuf,DWordSize);
    SingleClick = (DWbuf != 0);

    DWordSize=sizeof(DWORD); // will be changed by ReadValue()
    Reg.ReadValue(RegistryKey,"RightClickAction",REG_DWORD,(LPBYTE)&RightClickAction,DWordSize);
    if ((RightClickAction < 0)||(RightClickAction > 1))
        RightClickAction = 0;

    DWordSize=sizeof(DWORD);
    Reg.ReadValue(RegistryKey,"xPos",REG_DWORD,(LPBYTE)&xPos,DWordSize);

    DWordSize=sizeof(DWORD);
    Reg.ReadValue(RegistryKey,"yPos",REG_DWORD,(LPBYTE)&yPos,DWordSize);
    
    RECT Rect;
    GetWindowRect(GetDesktopWindow(),&Rect);

    if ((xPos < Rect.left)||(xPos > Rect.right)||(yPos < Rect.top)||(yPos > Rect.bottom))
    {
        xPos = -1;
        yPos = -1;
    }

    Reg.ReadValue(RegistryKey,"PathToBrowser",REG_SZ,(LPBYTE)BrowserPath.GetBuffer(_MAX_PATH+1),_MAX_PATH+1);
    BrowserPath.ReleaseBuffer();
    BrowserPath.TrimLeft();
    BrowserPath.TrimRight();

    DWordSize=sizeof(DWORD);
    Reg.ReadValue(RegistryKey,"DefaultBrowser",REG_DWORD,(LPBYTE)&DWbuf,DWordSize);
    DefaultBrowser = (DWbuf != 0);

    char lpFileName[MAX_PATH+3];
    DWORD size = MAX_PATH+2;
    AutoRun = Reg.ReadValue(AutoRunRegistryKey,AutoRunValueName,REG_SZ,(LPBYTE)lpFileName,size);
    if (AutoRun)
    {
        strcpy(lpFileName,"\"");
        GetModuleFileName(NULL,lpFileName+1,MAX_PATH);
        strcat(lpFileName,"\"");
        Reg.AddValue(AutoRunRegistryKey,AutoRunValueName,REG_SZ,(const BYTE*)lpFileName,lstrlen(lpFileName)+1);
    }

//  reading links settings
    int SubKeyIndex = 0;
    CString SubKeyName;
    bool IsDefault = false;
    while (Reg.GetSubKeyName(RegistryKey+"\\"+LinksSubKey,SubKeyIndex,SubKeyName.GetBuffer(255)))
    {
        SubKeyName.ReleaseBuffer();
        // reading values of the subkey
        const DWORD ValueLength = 255;
        UINT ViewTaskHotKey;
        UINT ViewChildTasksHotKey;
        CString TaskURL;
        CString ChildTasksURL;
        DWORD Default;
        DWordSize=sizeof(DWORD);
        Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"ViewTaskHotkey",
                      REG_DWORD,(LPBYTE)&ViewTaskHotKey,DWordSize);
        DWordSize=sizeof(DWORD);
        Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"ViewChildTasksHotkey",
                      REG_DWORD,(LPBYTE)&ViewChildTasksHotKey,DWordSize);
        
        Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"TaskURL",
                      REG_SZ,(LPBYTE)TaskURL.GetBuffer(ValueLength),ValueLength);
        TaskURL.ReleaseBuffer();
        Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"ChildTasksURL",
                      REG_SZ,(LPBYTE)ChildTasksURL.GetBuffer(ValueLength),ValueLength);
        ChildTasksURL.ReleaseBuffer();
               
        DWordSize=sizeof(DWORD);
        Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"Default",
                      REG_DWORD,(LPBYTE)&Default,DWordSize);
        if (Default == 1)
        {
            if (IsDefault) Default = 0;
            else IsDefault = true;
        }

        links.push_back(link(SubKeyName,TaskURL,ChildTasksURL,ViewTaskHotKey,
                             ViewChildTasksHotKey,(Default == 1)));

        SubKeyIndex++;
    }
    if (links.empty())
    {
        links.push_back(link("Alternative TMS","http://scc1/~alttms/viewtask.php?Client=%CLIENT%&ID=%ID%",
                             "http://scc1/~alttms/showtasks.php?ParentClient=%CLIENT%&ParentID=%ID%",0,0,false));
        links.push_back(link("Usual TMS","http://www.softcomputer.com/tms/viewtask.php?Client=%CLIENT%&ID=%ID%",
                             "http://www.softcomputer.com/tms/showtasks.php?ParentClient=%CLIENT%&ParentID=%ID%",0,0,true));
    }
    else
    {
        if (!IsDefault)
        {
            links[0].Default = true;
        }
    }

//  reading defects settings
    // reading defects link
    CString temp;
    Reg.ReadValue(RegistryKey+"\\"+LinksSubKey,"DefectsLink",REG_SZ,(LPBYTE)temp.GetBuffer(LINK_MAX+1),LINK_MAX+1);
    temp.ReleaseBuffer();
    if (!temp.IsEmpty() &&  (temp.GetLength() <= LINK_MAX)) DefectsLink = temp;

    if (Reg.KeyPresent(RegistryKey+"\\"+DefectsSubKey))
    {
        defects.clear();
        DWordSize=sizeof(DWORD);
        DWORD MaxValueNameLength = 260;
        DWORD MaxValueLength = 255;
        int defects_number = 0;
        defects_number = Reg.GetNumberOfValues(RegistryKey+"\\"+DefectsSubKey,&MaxValueNameLength,&MaxValueLength);
        MaxValueNameLength++;
        MaxValueLength++;
    
        DWORD type;
        CString value_name, value; // value_name == "Item[i]"; value == "LAB;ST_LABGUI_SYNCH";
    
        for (int i=0; i<defects_number; i++)
        {
            DWORD ValueNameLength = MaxValueNameLength;
            DWORD ValueLength = MaxValueLength;
            bool res = Reg.GetValueName(RegistryKey+"\\"+DefectsSubKey,i,value_name.GetBuffer(ValueNameLength),&ValueNameLength,&type);
            value_name.ReleaseBuffer();
            if (res && (type == REG_SZ))
            {
                Reg.ReadValue(RegistryKey+"\\"+DefectsSubKey,value_name,REG_SZ,(LPBYTE)value.GetBuffer(ValueLength),ValueLength);
                value.ReleaseBuffer();
                int separator = value.Find(';');
                if (separator != -1)
                {
                    defects.push_back(defect(value.Left(separator),value.Right(value.GetLength()-separator-1)));
                }
            }
        }
    }
    // reading format settings
    DWordSize=sizeof(DWORD);
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"MaxClient",REG_DWORD,(LPBYTE)&MaxClientName,DWordSize);
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"MaxID",REG_DWORD,(LPBYTE)&MaxIDName,DWordSize);
    if (MaxIDName < 1) MaxIDName = 1;
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"MaxExt",REG_DWORD,(LPBYTE)&MaxExt,DWordSize);
    MinTaskName = MinClientName+MinIDName;
    MaxTaskName = MaxClientName+MaxIDName+MaxExt+2; // 2 - separators
    CString SEPARATORS = "";
    CString TASKS_SEPARATORS = "";
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"Separators",REG_SZ,(LPBYTE)SEPARATORS.GetBuffer(255),255);
    SEPARATORS.ReleaseBuffer();
    RemoveUnacceptableSeparators(SEPARATORS);
    RemoveDuplicateSeparators(SEPARATORS);
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"TasksSeparators",REG_SZ,(LPBYTE)TASKS_SEPARATORS.GetBuffer(255),255);
    TASKS_SEPARATORS.ReleaseBuffer();
    RemoveUnacceptableSeparators(TASKS_SEPARATORS);
    RemoveDuplicateSeparators(TASKS_SEPARATORS);
    if (!SEPARATORS.FindOneOf(TASKS_SEPARATORS))
    {
        if (!SEPARATORS.IsEmpty()) Separators = SEPARATORS;
        if (!TASKS_SEPARATORS.IsEmpty()) TasksSeparators = TASKS_SEPARATORS;
    }
    // it is possible that some settings are not read and default ones are saved
    SaveGeneralSettings();
    SaveDefectsSettings();
    SaveFormatSettings();
    SaveLinksSettings();
}

void CSettings::SaveGeneralSettings(bool AfterImporting)
{
    DWORD buf;
    buf = Expand ? 1:0;
    Reg.AddValue(RegistryKey,"Expand at start",REG_DWORD,(const BYTE*)&buf,sizeof(DWORD));

    buf = Minimize ? 1:0;
    Reg.AddValue(RegistryKey,"Minimize at start",REG_DWORD,(const BYTE*)&buf,sizeof(DWORD));

    buf = SingleClick ? 1:0;
    Reg.AddValue(RegistryKey,"Single click",REG_DWORD,(const BYTE*)&buf,sizeof(DWORD));

    Reg.AddValue(RegistryKey,"RightClickAction",REG_DWORD,(const BYTE*)&RightClickAction,sizeof(DWORD));

    if (AutoRun)
    {
        char lpFileName[MAX_PATH+3];
        strcpy(lpFileName,"\"");
        GetModuleFileName(NULL,lpFileName+1,MAX_PATH);
        strcat(lpFileName,"\"");
        Reg.AddValue(AutoRunRegistryKey,AutoRunValueName,REG_SZ,(const BYTE*)lpFileName,lstrlen(lpFileName)+1);
    }
    else Reg.DeleteValue(AutoRunRegistryKey,AutoRunValueName);

    Reg.AddValue(RegistryKey,"xPos",REG_DWORD,(const BYTE*)&xPos,sizeof(DWORD));
    Reg.AddValue(RegistryKey,"yPos",REG_DWORD,(const BYTE*)&yPos,sizeof(DWORD));
    if (!AfterImporting)
    {
        buf = DefaultBrowser ? 1:0;
        Reg.AddValue(RegistryKey,"DefaultBrowser",REG_DWORD,(const BYTE*)&buf,sizeof(DWORD));
        Reg.AddValue(RegistryKey,"PathToBrowser",REG_SZ,(const BYTE*)LPCTSTR(BrowserPath),BrowserPath.GetLength()+1);
    }
}

void CSettings::SaveLinksSettings()
{
//  deleting old settings
    int SubKeyIndex = 0;
    CString SubKeyName;
    std::vector<CString> SubKeys;

    while (Reg.GetSubKeyName(RegistryKey+"\\"+LinksSubKey,SubKeyIndex,SubKeyName.GetBuffer(255)))
    {
        SubKeyName.ReleaseBuffer();
        SubKeys.push_back(SubKeyName);
        SubKeyIndex++;
    }
    for (int i=0; i<SubKeys.size(); i++)
    {
        Reg.DeleteKeyIncludingSubKeys(HKEY_CURRENT_USER,RegistryKey+"\\"+LinksSubKey+"\\"+SubKeys[i]);
    }
    SubKeys.clear();

//  saving new settings
    if (links.empty())
    {
        links.push_back(link("Alternative TMS","http://scc1/~alttms/viewtask.php?Client=%CLIENT%&ID=%ID%",
                             "http://scc1/~alttms/showtasks.php?ParentClient=%CLIENT%&ParentID=%ID%",0,0,false));
        links.push_back(link("Usual TMS","http://www.softcomputer.com/tms/viewtask.php?Client=%CLIENT%&ID=%ID%",
                             "http://www.softcomputer.com/tms/showtasks.php?ParentClient=%CLIENT%&ParentID=%ID%",0,0,true));
    }
    for (i=0; i<links.size(); i++)
    {
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"TaskURL",REG_SZ,(const BYTE*)LPCTSTR(links[i].TaskURL),links[i].TaskURL.GetLength()+1);
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"ChildTasksURL",REG_SZ,(const BYTE*)LPCTSTR(links[i].ChildTasksURL),links[i].ChildTasksURL.GetLength()+1);
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"ViewTaskHotkey",REG_DWORD,(const BYTE*)&links[i].ViewTaskHotKey,sizeof(DWORD));
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"ViewChildTasksHotkey",REG_DWORD,(const BYTE*)&links[i].ViewChildTasksHotKey,sizeof(DWORD));
        DWORD def = links[i].Default ? 1:0;
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"Default",REG_DWORD,(const BYTE*)&def,sizeof(DWORD));
    }
}

void CSettings::SaveFormatSettings()
{
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"MaxClient",REG_DWORD,(const BYTE*)&MaxClientName,sizeof(DWORD));
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"MaxID",REG_DWORD,(const BYTE*)&MaxIDName,sizeof(DWORD));
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"MaxExt",REG_DWORD,(const BYTE*)&MaxExt,sizeof(DWORD));
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"Separators",REG_SZ,(const BYTE*)LPCTSTR(Separators),Separators.GetLength()+1);
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"TasksSeparators",REG_SZ,(const BYTE*)LPCTSTR(TasksSeparators),TasksSeparators.GetLength()+1);
}

void CSettings::SaveDefectsSettings()
{
    Reg.DeleteKey(RegistryKey,DefectsSubKey);
    int defects_number = defects.size();
    CString value_name, value; // value_name == "Item0"; value == "LAB;ST_LABGUI_SYNCH";
    for (int i=0; i<defects_number; i++)
    {
        value_name.Format("Item%d",i);
        value.Format("%s;%s",defects[i].ClientID,defects[i].STProject);
        Reg.AddValue(RegistryKey+"\\"+DefectsSubKey,value_name,REG_SZ,(const BYTE*)LPCTSTR(value),value.GetLength()+1);
    }
    Reg.AddValue(RegistryKey+"\\"+LinksSubKey,"DefectsLink",REG_SZ,(const BYTE*)LPCTSTR(DefectsLink),DefectsLink.GetLength()+1);
}

bool CSettings::SettingsAvailable()
{
    // checking for new registry key
    if (Reg.KeyPresent(RegistryKey))
    {
        return true;
    }
    else
    {
        return false;
    }
}

// removes characters that cannot be separators and returns the number of characters left in string
int CSettings::RemoveUnacceptableSeparators(CString &String)
{
    int i = 0;
    while (i<String.GetLength())
    {
        if (isalnum(String[i]))
        {
            String.Delete(i);
        }
        else
        {
            i++;
        }
    }
    return String.GetLength();
}

// removes duplicate separators and returns the number of characters left in string
int CSettings::RemoveDuplicateSeparators(CString &String)
{
    char duplicates[255];
    ZeroMemory(duplicates,255);
    int i = 0;
    while (i<String.GetLength())
    {
        if (duplicates[String[i]])
        {
            String.Delete(i);
        }
        else
        {
            duplicates[String[i]]++;
            i++;
        }
    }
    return String.GetLength();
}

void CSettings::ImportSettings(LPCTSTR lpSubKey)
{
    if (Reg.KeyPresent(lpSubKey))
    {
        if (MessageBox(NULL,"Settings of previous TMS Launcher version were detected.\nThey can be imported to this version and then removed.\n\nWould you like to import previous settings?",szWinName,MB_YESNO|MB_ICONQUESTION)==IDYES)
        {
            // reading old settings
            DWORD DWbuf;
            DWORD DWordSize;

            // reading general settings
            DWordSize=sizeof(DWORD); // will be changed by ReadValue()
            Reg.ReadValue(lpSubKey,"Expand at start",REG_DWORD,(LPBYTE)&DWbuf,DWordSize);
            Expand = (DWbuf != 0);

            DWordSize=sizeof(DWORD); // will be changed by ReadValue()
            Reg.ReadValue(lpSubKey,"Minimize at start",REG_DWORD,(LPBYTE)&DWbuf,DWordSize);
            Minimize = (DWbuf != 0);

            DWordSize=sizeof(DWORD); // will be changed by ReadValue()
            Reg.ReadValue(lpSubKey,"Single click",REG_DWORD,(LPBYTE)&DWbuf,DWordSize);
            SingleClick = (DWbuf != 0);

            DWordSize=sizeof(DWORD); // will be changed by ReadValue()
            Reg.ReadValue(lpSubKey,"RightClickAction",REG_DWORD,(LPBYTE)&RightClickAction,DWordSize);
            if ((RightClickAction < 0)||(RightClickAction > 1))
                RightClickAction = 0;

            DWordSize=sizeof(DWORD);
            Reg.ReadValue(lpSubKey,"xPos",REG_DWORD,(LPBYTE)&xPos,DWordSize);

            DWordSize=sizeof(DWORD);
            Reg.ReadValue(lpSubKey,"yPos",REG_DWORD,(LPBYTE)&yPos,DWordSize);
    
            RECT Rect;
            GetWindowRect(GetDesktopWindow(),&Rect);

            if ((xPos < Rect.left)||(xPos > Rect.right)||(yPos < Rect.top)||(yPos > Rect.bottom))
            {
                xPos = -1;
                yPos = -1;
            }
            char lpFileName[MAX_PATH+3];
            DWORD size = MAX_PATH+2;
            AutoRun = Reg.ReadValue(AutoRunRegistryKey,AutoRunValueName,REG_SZ,(LPBYTE)lpFileName,size);

            // saving old settings as new settings
            SaveGeneralSettings(true);
        }
        // removing old registry key
        if (MessageBox(NULL,"Would you like to remove previous settings?",szWinName,MB_YESNO|MB_ICONQUESTION)==IDYES)
        {
            Reg.DeleteKeyIncludingSubKeys(HKEY_CURRENT_USER,lpSubKey);
        }
    }
}