/*
    File name: settings.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: January 4, 2007
*/

#include "stdafx.h"
#include "settings.h"
#include "About.h"

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: settings.cpp,v $$Revision: 1.37 $$Date: 2007/12/17 17:09:46Z $"; 
#endif

extern CString szWinName;
UINT LINK_MAX = 1024;

int CompareNoCaseCP1251(const char *string1, const char *string2);

bool GetVersionInfo(CString &string, WORD Language, WORD CodePage,
                    const char* StringName = "ProductVersion", UINT VersionDigits = 2,
                    const CString &ModulePath = "");

CSettings::CSettings(const char* RegKey, const char* AutoRunRegKey, const char* AutoRunValName, 
                     const char* DefectsSubKeyName, const char* FormatSubKeyName,
                     const char* LinksSubKeyName, const char* SoftTestSubKeyName,
                     const char* HistorySubKeyName):Reg(HKEY_CURRENT_USER),x(138)
{
    AutoRun = false;
    Expand = false;
    Minimize = false;
    SingleClick = false;
    RightClickAction = 0;
    RightClickAction2 = 0;
    MaxHistoryItems = 25;
    MaxPossibleHistory = 32000;
    xPos = -1; // for centered window
    yPos = -1; // for centered window
    RegistryKey = RegKey;
    DefectsSubKey = DefectsSubKeyName;
    FormatSubKey = FormatSubKeyName;
    LinksSubKey = LinksSubKeyName;
    SoftTestSubKey = SoftTestSubKeyName;
    HistorySubKey = HistorySubKeyName;
    AutoRunRegistryKey = AutoRunRegKey;
    AutoRunValueName = AutoRunValName;
    BrowserPath = "";
    DefaultBrowser = false;
    FillID = true;
    
    CString SpecialFolder;
    GetEnvironmentVariable("PROGRAMFILES",SpecialFolder.GetBuffer(MAX_PATH+1),MAX_PATH);
    SpecialFolder.ReleaseBuffer();

    SoftTestPath = SpecialFolder+"\\SoftComputer\\SoftTest\\Bin\\SoftTest.exe";
    SoftTestLogin = "guest";
    SoftTestPassword = "";

    GetEnvironmentVariable("APPDATA",SpecialFolder.GetBuffer(MAX_PATH+1),MAX_PATH);
    SpecialFolder.ReleaseBuffer();
    SoftTestFiltersPath = SpecialFolder+"\\SoftComputer\\SoftTest\\Filters\\";
    SoftTestFilterName = "%PROJECT%-TMS_Launcher.fil";
    DefectFilter = "SELECT * FROM BUG WHERE\nBG_BUG_ID IN (%ID%)";
    ChildDefectsFilter = "SELECT * FROM BUG WHERE\nBG_USER_HR_01 IN (%ID%)";
    ParentDefectFilter = "SELECT * FROM BUG WHERE\nBG_BUG_ID IN (SELECT BG_USER_HR_01 FROM BUG WHERE BG_BUG_ID IN (%ID%))";

    RtmRegEx = "^.*(Requirement[^?]*[:#])(.*)(<br><tr>|<br>[0-9][[:punct:]]|<br>[[:alpha:]]+\\b).*$";
    QbRegEx = "\"[^\"]*Q[BR]</td><td";
    QcRegEx = "\"[^\"]*QC</td><td.*(<hr|</table>)";
    AA_ID_RegEx = "AA_ID=([0-9]+)";
    TasksSeparators = ";,\n\r";
    Separators = " _-*+|:~#@$%^\t";
    MinClientName = 0;
    MaxClientName = 8;
    MinIDName = 1;
    MaxIDName = 6;
    MinExt = 0;
    MaxExt = 1;
    TaskNameControlType = 1; // ComboBox control
    DefectsLink = "http://qa.isd.dp.ua/softtest/defect/%PROJECT%/%ID%/";
    ChildDefectsLink = "http://qa.isd.dp.ua/softtest/child_defects/%PROJECT%/%ID%/";
    ParentDefectLink = "http://qa.isd.dp.ua/softtest/parent_defect/%PROJECT%/%ID%/";
    defects.push_back(defect("","ST_LABGUI_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink));
    defects.push_back(defect("CMN","ST_COMMONPROD_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink));
    defects.push_back(defect("CMNA","ST_COMMONASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink));
    defects.push_back(defect("LAB","ST_LABGUI_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink));
    defects.push_back(defect("LABA","ST_LABASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink));
    defects.push_back(defect("LABASC","ST_LABASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink));
    defects.push_back(defect("LABQC","ST_LABQCASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink));
    defects.push_back(defect("LABQCASC","ST_LABQCASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink));
    defects.push_back(defect("MIC","ST_MICGUI_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink));
    defects.push_back(defect("MICA","ST_MICASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink));
    defects.push_back(defect("MICASC","ST_MICASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink));
    defects.push_back(defect("MICQC","ST_MICQCASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink));
    defects.push_back(defect("MICQCASC","ST_MICQCASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink));
    defects.push_back(defect("SEC","ST_SECURITY",DefectsLink, ChildDefectsLink, ParentDefectLink));
    defects.push_back(defect("STO","ST_SOFTSTORE",DefectsLink, ChildDefectsLink, ParentDefectLink));
    defects.push_back(defect("STORE","ST_SOFTSTORE",DefectsLink, ChildDefectsLink, ParentDefectLink));
    defects.push_back(defect("SUP","ST_ISD_SUPPORT",DefectsLink, ChildDefectsLink, ParentDefectLink));
}

const CString& CSettings::GetSoftTestCommandLine(const char *Project)
{
//  SoftTest.exe /nST_LABGUI_SYNCH /a"Track Defects" /uguest /p"" /f"TMS_Launcher"
    // check for SoftTest version
    CString SoftTestVer = "";
    GetVersionInfo(SoftTestVer, 0x0409, 0x04b0, "ProductVersion", 3, SoftTestPath);

    CString temp = SoftTestFilterName;
    if (SoftTestVer.Compare("1.5.1") < 0)
    {
        temp = temp.Left(temp.ReverseFind('.'));
    }
    SoftTestCommandLine = "/n%PROJECT% /a\"Track Defects\" /u"+SoftTestLogin+" /p\""+SoftTestPassword+"\" /f\""+temp+"\"";
    SoftTestCommandLine.Replace("%PROJECT%",Project);
    return SoftTestCommandLine;
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
    if ((RightClickAction < 0)||(RightClickAction > 3))
        RightClickAction = 0;

    DWordSize=sizeof(DWORD); // will be changed by ReadValue()
    Reg.ReadValue(RegistryKey,"RightClickAction2",REG_DWORD,(LPBYTE)&RightClickAction2,DWordSize);
    if ((RightClickAction2 < 0)||(RightClickAction2 > 2))
        RightClickAction2 = 0;

    DWordSize=sizeof(DWORD); // will be changed by ReadValue()
    Reg.ReadValue(RegistryKey,"TaskNameControlType",REG_DWORD,(LPBYTE)&TaskNameControlType,DWordSize);
    if ((TaskNameControlType < 0)||(TaskNameControlType > 1))
        TaskNameControlType = 1;

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

    Reg.ReadValue(RegistryKey,"PathToBrowser",REG_SZ,(LPBYTE)BrowserPath.GetBuffer(MAX_PATH+1),MAX_PATH+1);
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
        const DWORD ValueLength = 16000;
        UINT ViewTaskHotKey;
        UINT ViewChildTasksHotKey;
        UINT ViewParentTaskHotKey;
        CString TaskURL;
        CString ChildTasksURL;
        CString Login;
        CString Password;
        DWORD Default;
        DWORD STDefects;
        
        DWordSize=sizeof(DWORD);

        if (!Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"ViewTaskHotkey",
                           REG_DWORD,(LPBYTE)&ViewTaskHotKey,DWordSize))
        {
            ViewTaskHotKey = 0;
        }
        DWordSize=sizeof(DWORD);
        if (!Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"ViewChildTasksHotkey",
                           REG_DWORD,(LPBYTE)&ViewChildTasksHotKey,DWordSize))
        {
            ViewChildTasksHotKey = 0;
        }
        DWordSize=sizeof(DWORD);
        if (!Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"ViewParentTaskHotkey",
                           REG_DWORD,(LPBYTE)&ViewParentTaskHotKey,DWordSize))
        {
            ViewParentTaskHotKey = 0;
        }
        
        Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"TaskURL",
                      REG_SZ,(LPBYTE)TaskURL.GetBuffer(ValueLength),ValueLength);
        TaskURL.ReleaseBuffer();
        Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"ChildTasksURL",
                      REG_SZ,(LPBYTE)ChildTasksURL.GetBuffer(ValueLength),ValueLength);
        ChildTasksURL.ReleaseBuffer();
               
        bool res = Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"Login",
                                 REG_SZ,(LPBYTE)Login.GetBuffer(ValueLength),ValueLength);
        Login.ReleaseBuffer();

        if (!res)
        {
            Login = "";
        }
        res = Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"Password",
                            REG_SZ,(LPBYTE)Password.GetBuffer(ValueLength),ValueLength);
        Password.ReleaseBuffer();
        if (!res)
        {
            Password = "";
        }
        else // password is not empty
        {
            // determining if it is necessary to decrypt the password
            CString Encrypted = "";
            if (Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"Encrypted",
                REG_SZ,(LPBYTE)Encrypted.GetBuffer(1),1))
            {
                Crypt(Password);
            }
            Encrypted.ReleaseBuffer();
        }

        DWordSize=sizeof(DWORD);
        Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"Default",
                      REG_DWORD,(LPBYTE)&Default,DWordSize);
        if (Default == 1)
        {
            if (IsDefault) Default = 0;
            else IsDefault = true;
        }

        DWordSize=sizeof(DWORD);
        Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"DefectsInSoftTest",
                      REG_DWORD,(LPBYTE)&STDefects,DWordSize);

        links.push_back(link(SubKeyName,TaskURL,ChildTasksURL,ViewTaskHotKey,
                             ViewChildTasksHotKey,ViewParentTaskHotKey,(Default == 1),
                             Login, Password, (STDefects == 1)));

        SubKeyIndex++;
    }
    if (links.empty())
    {
        links.push_back(link("Alternative TMS","http://scc1.softcomputer.com/~alttms/viewtask.php?Client=%CLIENT%&ID=%ID%",
                             "http://scc1.softcomputer.com/~alttms/showtasks.php?ParentClient=%CLIENT%&ParentID=%ID%",0,0,0,false,"","",false));
        links.push_back(link("SPC linked tasks","http://www.softcomputer.com/spc/showall.php?Client=%CLIENT%&ID=%ID%",
                             "http://www.softcomputer.com/spc/showall.php?Client=%CLIENT%&ID=%ID%",0,0,0,false,"","",false));
        links.push_back(link("Usual TMS","http://www.softcomputer.com/tms/viewtask.php?Client=%CLIENT%&ID=%ID%",
                             "http://www.softcomputer.com/tms/showtasks.php?ParentClient=%CLIENT%&ParentID=%ID%",0,0,0,true,"","",false));
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey,"SPC linked tasks",REG_SZ,(const BYTE*)LPCTSTR(""),0);
    }
    else
    {
        sort_links(links);
        if (!IsDefault)
        {
            links[0].Default = true;
        }
    }

//  reading defects settings
    // reading defects link
    if (Reg.KeyPresent(RegistryKey+"\\"+DefectsSubKey))
    {
        defects.clear();
        DWordSize=sizeof(DWORD);
        DWORD MaxValueNameLength = 255;
        DWORD MaxValueLength = 16000;
        int defects_number = 0;
        defects_number = Reg.GetNumberOfValues(RegistryKey+"\\"+DefectsSubKey,&MaxValueNameLength,&MaxValueLength);
        MaxValueNameLength++;
        MaxValueLength++;

        DWORD type;
        CString value_name, value; // value_name == "Item[i]"; value == "LAB;ST_LABGUI_SYNCH;URL1;URL2;URL3";

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
                CString Client, Project, URL, ChildURL, ParentURL;
                // parse string
                int separator = value.Find(';');
                if (separator != -1)
                {
                    Client = value.Left(separator);
                    value.Delete(0,separator+1);
                }
                else
                {
                    continue;
                }
                separator = value.Find(';');
                if (separator != -1)
                {
                    Project = value.Left(separator);
                    value.Delete(0,separator+1);
                }
                else
                {
                    Project = value;
                    defects.push_back(defect(Client,Project,DefectsLink,ChildDefectsLink,ParentDefectLink));
                    continue;
                }
                separator = value.Find(';');
                if (separator != -1)
                {
                    URL = value.Left(separator);
                    value.Delete(0,separator+1);
                }
                else
                {
                    URL = value;
                    defects.push_back(defect(Client,Project,URL,ChildDefectsLink,ParentDefectLink));
                    continue;
                }
                separator = value.Find(';');
                if (separator != -1)
                {
                    ChildURL = value.Left(separator);
                    value.Delete(0,separator+1);
                }
                else
                {
                    ChildURL = value;
                    defects.push_back(defect(Client,Project,URL,ChildURL,ParentDefectLink));
                    continue;
                }
                separator = value.Find(';');
                if (separator != -1)
                {
                    ParentURL = value.Left(separator);
                    value.Delete(0,separator+1);
                }
                else
                {
                    ParentURL = value;
                }
                defects.push_back(defect(Client,Project,URL,ChildURL,ParentURL));
            }
        }
    }

    // reading SoftTest settings
    CString temp = "";
    Reg.ReadValue(RegistryKey+"\\"+SoftTestSubKey,"SoftTestPath",REG_SZ,(LPBYTE)temp.GetBuffer(MAX_PATH+1),MAX_PATH+1);
    temp.ReleaseBuffer();
    temp.TrimLeft();
    temp.TrimRight();
    if (!temp.IsEmpty()) SoftTestPath = temp;

    temp = "";
    Reg.ReadValue(RegistryKey+"\\"+SoftTestSubKey,"SoftTestFilter",REG_SZ,(LPBYTE)temp.GetBuffer(MAX_PATH+1),MAX_PATH+1);
    temp.ReleaseBuffer();
    if (!temp.IsEmpty()) SoftTestFilterName = temp;
    if (SoftTestFilterName.Find("%PROJECT%") == -1)
    {
        SoftTestFilterName.Insert(0,"%PROJECT%");
    }

    temp = "";
    Reg.ReadValue(RegistryKey+"\\"+SoftTestSubKey,"SoftTestLogin",REG_SZ,(LPBYTE)temp.GetBuffer(1025),1025);
    temp .ReleaseBuffer();
    if (!temp .IsEmpty()) SoftTestLogin = temp ;
    
    temp = "";
    Reg.ReadValue(RegistryKey+"\\"+SoftTestSubKey,"SoftTestPassword",REG_SZ,(LPBYTE)temp.GetBuffer(1025),1025);
    temp.ReleaseBuffer();
    if (!temp.IsEmpty() || SoftTestLogin.Compare("guest")==0)
    {
        SoftTestPassword = temp;
        // determining if it is necessary to decrypt the password
        CString Encrypted = "";
        if (Reg.ReadValue(RegistryKey+"\\"+SoftTestSubKey,"Encrypted",
            REG_SZ,(LPBYTE)Encrypted.GetBuffer(1),1))
        {
            Crypt(SoftTestPassword);
        }
        Encrypted.ReleaseBuffer();
    }

    // reading format settings
    DWordSize=sizeof(DWORD); // will be changed by ReadValue()
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"MaxClient",REG_DWORD,(LPBYTE)&MaxClientName,DWordSize);
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"MaxID",REG_DWORD,(LPBYTE)&MaxIDName,DWordSize);
    if (MaxIDName < 1) MaxIDName = 1;
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"MaxExt",REG_DWORD,(LPBYTE)&MaxExt,DWordSize);
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
    if (SEPARATORS.FindOneOf(TASKS_SEPARATORS) == -1)
    {
        if (!SEPARATORS.IsEmpty()) Separators = SEPARATORS;
        if (!TASKS_SEPARATORS.IsEmpty()) TasksSeparators = TASKS_SEPARATORS;
    }
    CorrectCRLF(SEPARATORS,TASKS_SEPARATORS);
    if (Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"FillID",REG_DWORD,(LPBYTE)&DWbuf,DWordSize))
    {
        FillID = (DWbuf != 0);
    }
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"RTM regex",REG_SZ,(LPBYTE)RtmRegEx.GetBuffer(512),512);
    RtmRegEx.ReleaseBuffer();
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"QC regex",REG_SZ,(LPBYTE)QcRegEx.GetBuffer(512),512);
    QcRegEx.ReleaseBuffer();
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"QB regex",REG_SZ,(LPBYTE)QbRegEx.GetBuffer(512),512);
    QbRegEx.ReleaseBuffer();
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"AA_ID regex",REG_SZ,(LPBYTE)AA_ID_RegEx.GetBuffer(512),512);
    AA_ID_RegEx.ReleaseBuffer();

    // reading history settings
    DWordSize=sizeof(DWORD); // will be changed by ReadValue()
    if (!Reg.ReadValue(RegistryKey+"\\"+HistorySubKey,"MaxHistoryItems",REG_DWORD,(LPBYTE)&MaxHistoryItems,DWordSize))
    {
        Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"MaxHistoryItems",REG_DWORD,(LPBYTE)&MaxHistoryItems,DWordSize);
        Reg.DeleteValue(RegistryKey+"\\"+FormatSubKey,"MaxHistoryItems");
    }
    if ((MaxHistoryItems < 0)||(MaxHistoryItems > MaxPossibleHistory))
    {
        MaxHistoryItems = 25;
    }
    DWORD MaxValueNameLength = 255;
    DWORD MaxValueLength = 16000;
    DWORD type = 0;
    int HistoryItems = 0;
    HistoryItems = Reg.GetNumberOfValues(RegistryKey+"\\"+HistorySubKey,&MaxValueNameLength,&MaxValueLength);
    MaxValueNameLength++;
    MaxValueLength++;
    History.clear();
    for (int i=0; i < HistoryItems; i++)
    {
        DWORD ValueNameLength = MaxValueNameLength;
        DWORD ValueLength = MaxValueLength;
        CString ValueName = "", Value = "";
        bool res = Reg.GetValueName(RegistryKey+"\\"+HistorySubKey,i,ValueName.GetBuffer(ValueNameLength),&ValueNameLength,&type);
        ValueName.ReleaseBuffer();
        if (res && (type == REG_SZ))
        {
            Reg.ReadValue(RegistryKey+"\\"+HistorySubKey,ValueName,REG_SZ,(LPBYTE)Value.GetBuffer(ValueLength),ValueLength);
            Value.ReleaseBuffer();
            History.push_back(Value);
        }
    }

    // it is possible that some settings are not read and default ones are saved
    SaveGeneralSettings();
    SaveDefectsSettings();
    SaveFormatSettings();
    SaveLinksSettings();
    SaveSoftTestSettings();
    SaveHistorySettings();
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

    Reg.AddValue(RegistryKey,"RightClickAction2",REG_DWORD,(const BYTE*)&RightClickAction2,sizeof(DWORD));

    Reg.AddValue(RegistryKey,"TaskNameControlType",REG_DWORD,(const BYTE*)&TaskNameControlType,sizeof(DWORD));

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

void CSettings::AddingNewURLs()
{
    // adding "SPC linked tasks" URL
    CString IsSPCURL = "";
    if (!Reg.ReadValue(RegistryKey+"\\"+LinksSubKey,"SPC linked tasks",REG_SZ,(LPBYTE)IsSPCURL.GetBuffer(1),1))
    {
        if (!Reg.KeyPresent(RegistryKey+"\\"+LinksSubKey+"\\SPC linked tasks"))
        {
            CString Str = "http://www.softcomputer.com/spc/showall.php?Client=%CLIENT%&ID=%ID%";
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\SPC linked tasks","TaskURL",REG_SZ,(const BYTE*)LPCTSTR(Str),Str.GetLength()+1);
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\SPC linked tasks","ChildTasksURL",REG_SZ,(const BYTE*)LPCTSTR(Str),Str.GetLength()+1);
            DWORD def = 0;
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\SPC linked tasks","ViewTaskHotkey",REG_DWORD,(const BYTE*)&def,sizeof(DWORD));
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\SPC linked tasks","ViewChildTasksHotkey",REG_DWORD,(const BYTE*)&def,sizeof(DWORD));
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\SPC linked tasks","ViewParentTaskHotkey",REG_DWORD,(const BYTE*)&def,sizeof(DWORD));
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\SPC linked tasks","Default",REG_DWORD,(const BYTE*)&def,sizeof(DWORD));
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\SPC linked tasks","DefectsInSoftTest",REG_DWORD,(const BYTE*)&def,sizeof(DWORD));
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\SPC linked tasks","Login",REG_SZ,(const BYTE*)LPCTSTR(""),0);
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\SPC linked tasks","Password",REG_SZ,(const BYTE*)LPCTSTR(""),0);
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\SPC linked tasks","Encrypted",REG_SZ,(const BYTE*)LPCTSTR(""),0);
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey,"SPC linked tasks",REG_SZ,(const BYTE*)LPCTSTR(""),0);
        }
    }
}

void CSettings::SaveLinksSettings()
{
//  deleting old settings
    Reg.DeleteAllSubKeys(RegistryKey+"\\"+LinksSubKey);

//  saving new settings
    if (links.empty())
    {
        links.push_back(link("Alternative TMS","http://scc1.softcomputer.com/~alttms/viewtask.php?Client=%CLIENT%&ID=%ID%",
                             "http://scc1.softcomputer.com/~alttms/showtasks.php?ParentClient=%CLIENT%&ParentID=%ID%",0,0,0,false,"","",false));
        links.push_back(link("SPC linked tasks","http://www.softcomputer.com/spc/showall.php?Client=%CLIENT%&ID=%ID%",
                             "http://www.softcomputer.com/spc/showall.php?Client=%CLIENT%&ID=%ID%",0,0,0,false,"","",false));
        links.push_back(link("Usual TMS","http://www.softcomputer.com/tms/viewtask.php?Client=%CLIENT%&ID=%ID%",
                             "http://www.softcomputer.com/tms/showtasks.php?ParentClient=%CLIENT%&ParentID=%ID%",0,0,0,true,"","",false));
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey,"SPC linked tasks",REG_SZ,(const BYTE*)LPCTSTR(""),0);

    }
    for (int i=0; i<links.size(); i++)
    {
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"TaskURL",REG_SZ,(const BYTE*)LPCTSTR(links[i].TaskURL),links[i].TaskURL.GetLength()+1);
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"ChildTasksURL",REG_SZ,(const BYTE*)LPCTSTR(links[i].ChildTasksURL),links[i].ChildTasksURL.GetLength()+1);
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"ViewTaskHotkey",REG_DWORD,(const BYTE*)&links[i].ViewTaskHotKey,sizeof(DWORD));
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"ViewChildTasksHotkey",REG_DWORD,(const BYTE*)&links[i].ViewChildTasksHotKey,sizeof(DWORD));
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"ViewParentTaskHotkey",REG_DWORD,(const BYTE*)&links[i].ViewParentTaskHotKey,sizeof(DWORD));
        DWORD def = links[i].Default ? 1:0;
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"Default",REG_DWORD,(const BYTE*)&def,sizeof(DWORD));
        def = links[i].DefectsInSoftTest ? 1:0;
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"DefectsInSoftTest",REG_DWORD,(const BYTE*)&def,sizeof(DWORD));
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"Login",REG_SZ,(const BYTE*)LPCTSTR(links[i].Login),links[i].Login.GetLength()+1);
        CString Encrypted = links[i].Password;
        Crypt(Encrypted);
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"Password",REG_SZ,(const BYTE*)LPCTSTR(Encrypted),Encrypted.GetLength()+1);
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"Encrypted",REG_SZ,(const BYTE*)LPCTSTR(""),0);
    }
}

void CSettings::SaveFormatSettings()
{
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"MaxClient",REG_DWORD,(const BYTE*)&MaxClientName,sizeof(DWORD));
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"MaxID",REG_DWORD,(const BYTE*)&MaxIDName,sizeof(DWORD));
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"MaxExt",REG_DWORD,(const BYTE*)&MaxExt,sizeof(DWORD));
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"Separators",REG_SZ,(const BYTE*)LPCTSTR(Separators),Separators.GetLength()+1);
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"TasksSeparators",REG_SZ,(const BYTE*)LPCTSTR(TasksSeparators),TasksSeparators.GetLength()+1);
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"FillID",REG_DWORD,(const BYTE*)&FillID,sizeof(DWORD));
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"RTM regex",REG_SZ,(const BYTE*)LPCTSTR(RtmRegEx),RtmRegEx.GetLength()+1);
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"QC regex",REG_SZ,(const BYTE*)LPCTSTR(QcRegEx),QcRegEx.GetLength()+1);
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"QB regex",REG_SZ,(const BYTE*)LPCTSTR(QbRegEx),QbRegEx.GetLength()+1);
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"AA_ID regex",REG_SZ,(const BYTE*)LPCTSTR(AA_ID_RegEx),AA_ID_RegEx.GetLength()+1);
}

void CSettings::SaveSoftTestSettings()
{
    Reg.AddValue(RegistryKey+"\\"+SoftTestSubKey,"SoftTestPath",REG_SZ,(const BYTE*)LPCTSTR(SoftTestPath),SoftTestPath.GetLength()+1);
    Reg.AddValue(RegistryKey+"\\"+SoftTestSubKey,"SoftTestFilter",REG_SZ,(const BYTE*)LPCTSTR(SoftTestFilterName),SoftTestFilterName.GetLength()+1);
    Reg.AddValue(RegistryKey+"\\"+SoftTestSubKey,"SoftTestLogin",REG_SZ,(const BYTE*)LPCTSTR(SoftTestLogin),SoftTestLogin.GetLength()+1);
    CString Encrypted = SoftTestPassword;
    Crypt(Encrypted);
    Reg.AddValue(RegistryKey+"\\"+SoftTestSubKey,"SoftTestPassword",REG_SZ,(const BYTE*)LPCTSTR(Encrypted),Encrypted.GetLength()+1);
    Reg.AddValue(RegistryKey+"\\"+SoftTestSubKey,"Encrypted",REG_SZ,(const BYTE*)LPCTSTR(""),0);
}

void CSettings::SaveDefectsSettings()
{
    Reg.DeleteKey(RegistryKey,DefectsSubKey);
   
    int defects_number = defects.size();
    CString value_name, value; // value_name == "Item0"; value == "LAB;ST_LABGUI_SYNCH;URL1;URL2;URL3";
    for (int i=0; i<defects_number; i++)
    {
        value_name.Format("Item%d",i);
        value.Format("%s;%s;%s;%s;%s",defects[i].ClientID,defects[i].STProject,defects[i].DefectURL,defects[i].ChildDefectsURL,defects[i].ParentDefectURL);
        Reg.AddValue(RegistryKey+"\\"+DefectsSubKey,value_name,REG_SZ,(const BYTE*)LPCTSTR(value),value.GetLength()+1);
    }
}

void CSettings::SaveHistorySettings()
{
    Reg.DeleteKey(RegistryKey,HistorySubKey);
    Reg.AddValue(RegistryKey+"\\"+HistorySubKey,"MaxHistoryItems",REG_DWORD,(const BYTE*)&MaxHistoryItems,sizeof(DWORD));
    CString value_name, value;
    for (int i=0; i < History.size(); i++)
    {
        value_name.Format("Item%d",i);
        value = History[i];
        Reg.AddValue(RegistryKey+"\\"+HistorySubKey,value_name,REG_SZ,(const BYTE*)LPCTSTR(value),value.GetLength()+1);
    }
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
            if ((RightClickAction < 0)||(RightClickAction > 3))
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

int CSettings::GetDefaultUrlIndex()
{
    int index = -1;
    for (int i = 0; i < links.size(); i++)
    {
        if (links[i].Default)
        {
            index = i;
            break;
        }
    }
    return index;
}

bool CSettings::IsDefect(const char *Client, CString *Project, int *index)
{
    bool result = false;
    if (Project != NULL)
    {
        *Project = CString("");
    }
    if (index != NULL)
    {
        *index = -1;
    }

    for (int i=0; i < defects.size(); i++)
    {
        if (CompareNoCaseCP1251(Client,defects[i].ClientID)==0)
        {
            result = true;
            if (Project != NULL)
            {
                *Project = defects[i].STProject;
            }
            if (index != NULL)
            {
                *index = i;
            }
            break;
        }
    }

    return result;
}

bool CSettings::OpenDefectsInSoftTest(INT wID)
{
    switch (wID)
    {
        case VIEW_TASK:
        case VIEW_TASK_HOTKEY:
        case VIEW_CHILD_TASKS:
        case VIEW_CHILD_TASKS_HOTKEY:
        case VIEW_PARENT_TASK:
        case VIEW_PARENT_TASK_HOTKEY:
            return (links[GetDefaultUrlIndex()].DefectsInSoftTest);
        default:
            int index = wID / 3;
            if (index < links.size())
            {
                return (links[index].DefectsInSoftTest);
            }
            else
            {
                return false;
            }
    }
}

bool CSettings::CorrectCRLF(CString &sSeparators, CString &sTasksSeparators)
{
    bool correction = false;

    if ((sTasksSeparators.Find('\n') != -1) && (sTasksSeparators.Find('\r') == -1))
    {
        sTasksSeparators += '\r';
        correction = true;
        int pos = -1;
        if ((pos = sSeparators.Find('\r')) != -1)
        {
            sSeparators.Delete(pos);
        }
    }
    if ((sSeparators.Find('\n') != -1) && (sSeparators.Find('\r') == -1))
    {
        sSeparators += '\r';
        correction = true;
        int pos = -1;
        if ((pos = sTasksSeparators.Find('\r')) != -1)
        {
            sTasksSeparators.Delete(pos);
        }
    }
    if ((sTasksSeparators.Find('\r') != -1) && (sTasksSeparators.Find('\n') == -1))
    {
        sTasksSeparators += '\n';
        correction = true;
    }
    if ((sSeparators.Find('\r') != -1) && (sSeparators.Find('\n') == -1))
    {
        sSeparators += '\n';
        correction = true;
    }
    return correction;
}

void CSettings::Crypt(CString &String)
{
    if (String.IsEmpty()) return;
    for (int i = 0; i < String.GetLength(); i++)
    {
        String.SetAt(i,String.GetAt(i)^(i+x));
    }
}

void CSettings::sort_links(std::vector<link> &links_to_sort)
{
    for (int i=0; i<links_to_sort.size()-1; i++)
    {
        bool exchange = false;
        for (int j=1; j<links_to_sort.size()-i; j++)
        {
            if (CompareNoCaseCP1251(links_to_sort[j-1].Caption,links_to_sort[j].Caption)>0)
            {
                link temp = links_to_sort[j-1];
                links_to_sort[j-1] = links_to_sort[j];
                links_to_sort[j] = temp;
                exchange = true;
            }
        }
        if (!exchange) break;
    }
}
