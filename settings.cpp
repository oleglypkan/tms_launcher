/*
    File name: settings.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
*/

#include "stdafx.h"
#include "settings.h"
#include "About.h"
#include "Tools.h"

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: settings.cpp,v $$Revision: 1.58 $$Date: 2009/04/09 14:54:44Z $"; 
#endif

#ifdef _DEBUG
#include <crtdbg.h>
#include <stdlib.h>
#define _CRTDBG_MAP_ALLOC 
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

extern CString szWinName;
UINT LINK_MAX = 1024;

bool isalpha_cp1251(unsigned char ch);
int CompareNoCaseCP1251(const char *string1, const char *string2);
void StringToUpperCase(CString &String);

bool GetVersionInfo(CString &string, WORD Language, WORD CodePage,
                    const char* StringName = "ProductVersion", UINT VersionDigits = 2,
                    const CString &ModulePath = "");

CSettings::CSettings(const char* RegKey, const char* AutoRunRegKey, const char* AutoRunValName, 
                     const char* DefectsSubKeyName, const char* FormatSubKeyName,
                     const char* LinksSubKeyName, const char* SoftTestSubKeyName,
                     const char* HistorySubKeyName, const char* OtherSubKeyName,
                     const char* FlagsSubKeyName):Reg(HKEY_CURRENT_USER),x(138),GlobalHotkeyID(0xBFFF),
                     iTMSviewTask("https://www.softcomputer.com:443/itms/gentaskdetails.php?Client=%s&ID=%s"),
                     iTMSviewChildTasks("https://www.softcomputer.com:443/itms/gentaskdetails.php?Client=%s&ID=%s"),
                     iTMSviewRelatedTasks("https://www.softcomputer.com:443/itms/showall.php?Client=%s&ID=%s"),
                     iTMSviewTimesheets("https://www.softcomputer.com:443/itms/task_timesheets.php?Client=%s&ID=%s")
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
    OtherSubKey = OtherSubKeyName;
    FlagsSubKey = FlagsSubKeyName;
    AutoRunRegistryKey = AutoRunRegKey;
    AutoRunValueName = AutoRunValName;
    BrowserPath = "";
    DefaultBrowser = false;
    FillID = true;
    EnableOpacity = true;
    InactiveOpacity = 85;
    ActiveOpacity = 95;
    HotKey = 0;
    WinKey = 0;
    SetFocusToTaskName = true;
    HighlightTaskName = true;
    MainDialog = NULL;

    CString SpecialFolder;
    GetEnvironmentVariable("PROGRAMFILES",SpecialFolder.GetBuffer(MAX_PATH+1),MAX_PATH);
    SpecialFolder.ReleaseBuffer();
    if (SpecialFolder.IsEmpty())
    {
        SpecialFolder = "C:\\Program Files";
    }
    SoftTestPath = SpecialFolder+"\\SoftComputer\\SoftTest\\Bin\\SoftTest.exe";
    SoftTestLogin = "guest";
    SoftTestPassword = "";

    GetEnvironmentVariable("APPDATA",SpecialFolder.GetBuffer(MAX_PATH+1),MAX_PATH);
    SpecialFolder.ReleaseBuffer();
    if (SpecialFolder.IsEmpty())
    {
        SpecialFolder = "C:\\Documents and Settings\\All Users\\Application Data";
    }
    SoftTestFiltersPath = SpecialFolder+"\\SoftComputer\\SoftTest\\Filters\\";
    SoftTestFilterName = "TMS_Launcher~%PROJECT%.fil";
    DefectFilter = "SELECT * FROM BUG WHERE\nBG_BUG_ID IN (%ID%)";
    ChildDefectsFilter = "SELECT * FROM BUG WHERE\nBG_USER_HR_01 IN (%ID%)";
    ParentDefectFilter = "SELECT * FROM BUG WHERE\nBG_BUG_ID IN (SELECT BG_USER_HR_01 FROM BUG WHERE BG_BUG_ID IN (%ID%))";
    SetRelatedDefectsFilter();

    iTMSRtmRegEx = "(Requirement[^?]*[:#])(.*)(<br>$|<br><tr>|<br>[0-9][[:punct:]]|<br>[[:alpha:]]+\\b)";
    ActHeaderRegEx = "<tr bgcolor[^>]+>[[:space:]]*<td[^>]*>[0-9]+</td>.+</tr>"; // actually it is regex for action header
    ActBodyRegEx = "[[:space:]]*<tr bgcolor[^>]+>[[:space:]]*<td[^>]*>.*</td>[[:space:]]*<td colspan[^>]+>(.*)</td>[[:space:]]*</tr>"; // actually it is regex for action body
    AA_ID_RegEx = "AA_ID=([0-9]+)";
    iTMSTimesheetsRegEx = "<tr><td nowrap >([^<]+)</td><td nowrap >([^<]+)</td><td[^>]*>([^<]+)</td><td>([^<]+)</td></tr>";
    TasksSeparators = ";,\n\r";
    Separators = " —– -*+|:~#@$%^\t";
    HistoryTasks = "";
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
    RelatedDefectsLink = "http://qa.isd.dp.ua/softtest/related_defects/%PROJECT%/%ID%/";
    SifLink = "http://se-web.softcomputer.com/SE_DB_ENGINE/tools/forms/sif/saveToDb.do?&shortcut=true&actionS=Edit&id=%ID%";
    HfLinkActive = "http://se.softcomputer.com/CM/index.php?script=hotfix/index.php&search_product=all&search_limits=0&search_status[]=active&hotfixid=%ID%";
    HfLinkAll = "http://se.softcomputer.com/CM/index.php?script=hotfix/index.php&search_product=all&search_limits=0&hotfixid=%ID%";
    HfLinkRevision = "http://se.softcomputer.com/CM/hotfix/HFinformation.php?HF_mainpage=1&hotfixID=%ID%";
    defects.push_back(defect("","ST_LABGUI_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("CMN","ST_COMMONPROD_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("CMNA","ST_COMMONASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("LAB","ST_LABGUI_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("LABA","ST_LABASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("LABASC","ST_LABASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("LABQC","ST_LABQCASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("LABQCASC","ST_LABQCASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("LMM","ST_LM_MAYO_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("MIC","ST_MICGUI_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("MICA","ST_MICASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("MICASC","ST_MICASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("MICQC","ST_MICQCASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("MICQCASC","ST_MICQCASCII_SYNCH",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("SEC","ST_SECURITY",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("STO","ST_SOFTSTORE",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("STORE","ST_SOFTSTORE",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("SUP","ST_ISD_SUPPORT",DefectsLink, ChildDefectsLink, ParentDefectLink,RelatedDefectsLink));
    defects.push_back(defect("SIF","SIF",SifLink, SifLink, SifLink, SifLink));
    defects.push_back(defect("HF","HF",HfLinkActive, HfLinkAll, HfLinkAll, HfLinkAll));
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
    if ((RightClickAction < 0)||(RightClickAction > 4))
        RightClickAction = 0;

    DWordSize=sizeof(DWORD); // will be changed by ReadValue()
    Reg.ReadValue(RegistryKey,"RightClickAction2",REG_DWORD,(LPBYTE)&RightClickAction2,DWordSize);
    if ((RightClickAction2 < 0)||(RightClickAction2 > 3))
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
    DWORD SubKeyIndex = 0;
    CString SubKeyName = "";
    bool IsDefault = false;
    while (Reg.GetSubKeyName(RegistryKey+"\\"+LinksSubKey,SubKeyIndex,SubKeyName.GetBuffer(255)))
    {
        SubKeyName.ReleaseBuffer();
        // reading values of the sub-key
        const DWORD ValueLength = 16000;
        UINT ViewTaskHotKey = 0;
        UINT ViewChildTasksHotKey = 0;
        UINT ViewParentTaskHotKey = 0;
        UINT ViewRelatedTasksHotKey = 0;
        CString TaskURL = "";
        CString ChildTasksURL = "";
        CString RelatedTasksURL = "";
        CString Login = "";
        CString Password = "";
        DWORD Default = 0;
        DWORD STDefects = 0;
        
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
        DWordSize=sizeof(DWORD);
        if (!Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"ViewRelatedTasksHotkey",
            REG_DWORD,(LPBYTE)&ViewRelatedTasksHotKey,DWordSize))
        {
            ViewRelatedTasksHotKey = 0;
        }

        Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"TaskURL",
                      REG_SZ,(LPBYTE)TaskURL.GetBuffer(ValueLength),ValueLength);
        TaskURL.ReleaseBuffer();
        Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"ChildTasksURL",
                      REG_SZ,(LPBYTE)ChildTasksURL.GetBuffer(ValueLength),ValueLength);
        ChildTasksURL.ReleaseBuffer();
        Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"RelatedTasksURL",
                      REG_SZ,(LPBYTE)RelatedTasksURL.GetBuffer(ValueLength),ValueLength);
        RelatedTasksURL.ReleaseBuffer();

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

        links.push_back(link(SubKeyName,TaskURL,ChildTasksURL,RelatedTasksURL,ViewTaskHotKey,
                             ViewChildTasksHotKey,ViewParentTaskHotKey,ViewRelatedTasksHotKey,(Default == 1),
                             Login, Password, (STDefects == 1)));

        SubKeyIndex++;
    }
    if (links.empty()) // load default set of URLs
    {
        links.push_back(link("Alternative TMS","http://scc1.softcomputer.com/~alttms/viewtask.php?Client=%CLIENT%&ID=%ID%",
                             "http://scc1.softcomputer.com/~alttms/showtasks.php?ParentClient=%CLIENT%&ParentID=%ID%",
                             "http://scc1.softcomputer.com/~alttms/relatives.php?Client=%CLIENT%&ID=%ID%",
                             0,0,0,0,false,"","",false));
        links.push_back(link("iTMS","https://www.softcomputer.com/itms/gentaskdetails.php?Client=%CLIENT%&ID=%ID%",
                             "https://www.softcomputer.com/itms/tms_related.php?Client=%CLIENT%&ID=%ID%",
                             "https://www.softcomputer.com/itms/showall.php?Client=%CLIENT%&ID=%ID%",
                             0,0,0,0,true,"","",false));
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey,"iTMS",REG_SZ,(const BYTE*)LPCTSTR(""),0); // flag meaning 3.0 version or higher is used
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
        CString value_name = "", value = ""; // value_name == "Item[i]"; value == "LAB;ST_LABGUI_SYNCH;URL1;URL2;URL3;URL4";

        for (int i=0; i<defects_number; i++)
        {
            DWORD ValueNameLength = MaxValueNameLength;
            DWORD ValueLength = MaxValueLength;
            value_name = "", value = "";
            bool res = Reg.GetValueName(RegistryKey+"\\"+DefectsSubKey,i,value_name.GetBuffer(ValueNameLength),&ValueNameLength,&type);
            value_name.ReleaseBuffer();
            if (res && (type == REG_SZ))
            {
                Reg.ReadValue(RegistryKey+"\\"+DefectsSubKey,value_name,REG_SZ,(LPBYTE)value.GetBuffer(ValueLength),ValueLength);
                value.ReleaseBuffer();
                CString Client = "";
                CString Project = "";
                CString URL = "";
                CString ChildURL = "";
                CString ParentURL = "";
                CString RelatedURL = "";
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
                    defects.push_back(defect(Client,Project,DefectsLink,ChildDefectsLink,ParentDefectLink,RelatedDefectsLink));
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
                    defects.push_back(defect(Client,Project,URL,ChildDefectsLink,ParentDefectLink,RelatedDefectsLink));
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
                    defects.push_back(defect(Client,Project,URL,ChildURL,ParentDefectLink,RelatedDefectsLink));
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
                    defects.push_back(defect(Client,Project,URL,ChildURL,ParentURL,RelatedDefectsLink));
                    continue;
                }
                separator = value.Find(';');
                if (separator != -1)
                {
                    RelatedURL = value.Left(separator);
                }
                else 
                {
                    RelatedURL = value;
                }
                defects.push_back(defect(Client,Project,URL,ChildURL,ParentURL,RelatedURL));
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
        SoftTestFilterName = "TMS_Launcher~%PROJECT%.fil";        
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
    if (SEPARATORS.Find('.') != -1)
    {
        if (MyMessageBox(NULL,"\"Separators\" field has character(s) '.' that will prevent opening hotfixes.\nWould you like the character(s) to be removed automatically?",szWinName,MB_YESNO|MB_ICONWARNING)==IDYES)
        {
            SEPARATORS.Remove('.');
        }
    }
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"TasksSeparators",REG_SZ,(LPBYTE)TASKS_SEPARATORS.GetBuffer(255),255);
    TASKS_SEPARATORS.ReleaseBuffer();
    RemoveUnacceptableSeparators(TASKS_SEPARATORS);
    RemoveDuplicateSeparators(TASKS_SEPARATORS);
    if (TASKS_SEPARATORS.Find('.') != -1)
    {
        if (MyMessageBox(NULL,"\"Tasks separators\" field has character(s) '.' that will prevent opening hotfixes.\nWould you like the character(s) to be removed automatically?",szWinName,MB_YESNO|MB_ICONWARNING)==IDYES)
        {
            TASKS_SEPARATORS.Remove('.');
        }
    }
    CorrectCRLF(SEPARATORS,TASKS_SEPARATORS);
    if (SEPARATORS.FindOneOf(TASKS_SEPARATORS) == -1)
    {
        if (!SEPARATORS.IsEmpty()) Separators = SEPARATORS;
        if (!TASKS_SEPARATORS.IsEmpty()) TasksSeparators = TASKS_SEPARATORS;
    }
    if (Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"FillID",REG_DWORD,(LPBYTE)&DWbuf,DWordSize))
    {
        FillID = (DWbuf != 0);
    }
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"iTMS RTM regex",REG_SZ,(LPBYTE)iTMSRtmRegEx.GetBuffer(512),512);
    iTMSRtmRegEx.ReleaseBuffer();
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"ActionBody regex",REG_SZ,(LPBYTE)ActBodyRegEx.GetBuffer(512),512);
    ActBodyRegEx.ReleaseBuffer();
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"ActionHeader regex",REG_SZ,(LPBYTE)ActHeaderRegEx.GetBuffer(512),512);
    ActHeaderRegEx.ReleaseBuffer();
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"AA_ID regex",REG_SZ,(LPBYTE)AA_ID_RegEx.GetBuffer(512),512);
    AA_ID_RegEx.ReleaseBuffer();
    Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"iTMS Timesheets regex",REG_SZ,(LPBYTE)iTMSTimesheetsRegEx.GetBuffer(512),512);
    iTMSTimesheetsRegEx.ReleaseBuffer();

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

    // reading other settings
    DWordSize=sizeof(DWORD);
    DWORD Opacity = 0;
    if (Reg.ReadValue(RegistryKey+"\\"+OtherSubKey,"EnableOpacity",REG_DWORD,(LPBYTE)&Opacity,DWordSize))
    {
        EnableOpacity = (Opacity == 1);
    }

    DWordSize=sizeof(DWORD); // will be changed by ReadValue()
    Opacity = 101;
    if (Reg.ReadValue(RegistryKey+"\\"+OtherSubKey,"InactiveOpacity",REG_DWORD,(LPBYTE)&Opacity,DWordSize))
    {
        if (Opacity >= 0 && Opacity <= 100)
        {
            InactiveOpacity = (BYTE)Opacity;
        }
    }
    DWordSize=sizeof(DWORD); // will be changed by ReadValue()
    Opacity = 101;
    if (Reg.ReadValue(RegistryKey+"\\"+OtherSubKey,"ActiveOpacity",REG_DWORD,(LPBYTE)&Opacity,DWordSize))
    {
        if (Opacity >= 0 && Opacity <= 100)
        {
            ActiveOpacity = (BYTE)Opacity;
        }
    }
    DWordSize=sizeof(DWORD);
    Opacity = 0;
    if (Reg.ReadValue(RegistryKey+"\\"+OtherSubKey,"WinKey",REG_DWORD,(LPBYTE)&Opacity,DWordSize))
    {
        WinKey = Opacity ? MOD_WIN : 0;
    }

    DWordSize=sizeof(DWORD);
    Reg.ReadValue(RegistryKey+"\\"+OtherSubKey,"HotKey",REG_DWORD,(LPBYTE)&HotKey,DWordSize);

    DWordSize=sizeof(DWORD);
    Opacity = 0;
    if (Reg.ReadValue(RegistryKey+"\\"+OtherSubKey,"SetFocusToTaskName",REG_DWORD,(LPBYTE)&Opacity,DWordSize))
    {
        SetFocusToTaskName = (Opacity == 1);
    }

    DWordSize=sizeof(DWORD);
    Opacity = 0;
    if (Reg.ReadValue(RegistryKey+"\\"+OtherSubKey,"HighlightTaskName",REG_DWORD,(LPBYTE)&Opacity,DWordSize))
    {
        HighlightTaskName = (Opacity == 1);
    }

    // it is possible that some settings are not read and default ones should be saved
    SaveGeneralSettings();
    SaveDefectsSettings();
    SaveFormatSettings();
    SaveLinksSettings();
    SaveSoftTestSettings();
    SaveHistorySettings();
    SaveOtherSettings();
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

// convert some settings from previous to new version
void CSettings::ConvertSettings()
{
    DWORD DWordSize=sizeof(DWORD); // will be changed by ReadValue()
    int RightClickAction = 0;
    CString ISiTMSurl = "";
    if (!Reg.KeyPresent(RegistryKey+"\\"+LinksSubKey))
    {
        return;
    }
    if (!Reg.ReadValue(RegistryKey+"\\"+LinksSubKey,"iTMS",REG_SZ,(LPBYTE)ISiTMSurl.GetBuffer(1),1)) // this value was added in 3.0
    {
        // In versions prior to 3.0, RightClickAction == 3 means "Open context menu"
        // Since version 3.0, "Open context menu" is assigned to RightClickAction == 4
        Reg.ReadValue(RegistryKey,"RightClickAction",REG_DWORD,(LPBYTE)&RightClickAction,DWordSize);
        if (RightClickAction == 3)
        {
            RightClickAction = 4;
            Reg.AddValue(RegistryKey,"RightClickAction",REG_DWORD,(const BYTE*)&RightClickAction,sizeof(DWORD));
        }
    }
}

void CSettings::AddingNewURLs()
{
    // adding HF record to defects records list
    if (Reg.KeyPresent(RegistryKey+"\\"+DefectsSubKey))
    {
        char HFflag[2] = ""; // this flag was added in 3.1
        if (!Reg.ReadValue(RegistryKey+"\\"+FlagsSubKey,"HF",REG_SZ,(LPBYTE)HFflag,1))
        {
            if (!IsDefectInRegistry("HF;"))
            {
                CString value = "";
                value.Format("%s;%s;%s;%s;%s;%s", "HF", "HF", HfLinkActive, HfLinkAll, HfLinkAll, HfLinkAll);
                Reg.AddValue(RegistryKey+"\\"+DefectsSubKey,"HF",REG_SZ,(const BYTE*)LPCTSTR(value),value.GetLength()+1);
            }
            Reg.AddValue(RegistryKey+"\\"+FlagsSubKey,"HF",REG_SZ,(const BYTE*)LPCTSTR(""),0); // this flag was added in 3.1
        }
    }

    // adding SIF record to defects records list
    if (Reg.KeyPresent(RegistryKey+"\\"+DefectsSubKey))
    {
        char SIFflag[2] = ""; // this flag was added in 3.1
        if (!Reg.ReadValue(RegistryKey+"\\"+FlagsSubKey,"SIF",REG_SZ,(LPBYTE)SIFflag,1))
        {
            if (!IsDefectInRegistry("SIF;"))
            {
                CString value = "";
                value.Format("%s;%s;%s;%s;%s;%s", "SIF", "SIF", SifLink, SifLink, SifLink, SifLink);
                Reg.AddValue(RegistryKey+"\\"+DefectsSubKey,"SIF",REG_SZ,(const BYTE*)LPCTSTR(value),value.GetLength()+1);
            }
            Reg.AddValue(RegistryKey+"\\"+FlagsSubKey,"SIF",REG_SZ,(const BYTE*)LPCTSTR(""),0); // this flag was added in 3.1
        }
    }

    // adding new values to "Separators" field if these values were not added before
    // this is determined by existence of "NewSeparators" flag
    CString SEPARATORS = "";
    if (Reg.ReadValue(RegistryKey+"\\"+FormatSubKey,"Separators",REG_SZ,(LPBYTE)SEPARATORS.GetBuffer(255),255))
    {
        SEPARATORS.ReleaseBuffer();
        char NewSeparatorsFlag[2] = ""; // this flag was added in 3.1
        if (!Reg.ReadValue(RegistryKey+"\\"+FlagsSubKey,"NewSeparators",REG_SZ,(LPBYTE)NewSeparatorsFlag,1))
        {
            SEPARATORS.Insert(0," —–");
            Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"Separators",REG_SZ,(const BYTE*)LPCTSTR(SEPARATORS),SEPARATORS.GetLength()+1);
            Reg.AddValue(RegistryKey+"\\"+FlagsSubKey,"NewSeparators",REG_SZ,(const BYTE*)LPCTSTR(""),0);
        }       
    }
    
    //  if links sub-key does not exist, default URLs should be restored
    if (!Reg.KeyPresent(RegistryKey+"\\"+LinksSubKey))
    {
        return;
    }

    char ISiTMSURL[2] = "";
    if (!Reg.ReadValue(RegistryKey+"\\"+LinksSubKey,"iTMS",REG_SZ,(LPBYTE)ISiTMSURL,1)) // dealing with settings from version prior to 3.0
    {
        // adding "iTMS" URL
        if (!Reg.KeyPresent(RegistryKey+"\\"+LinksSubKey+"\\iTMS"))
        {
            CString Str = "https://www.softcomputer.com/itms/gentaskdetails.php?Client=%CLIENT%&ID=%ID%";
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\iTMS","TaskURL",REG_SZ,(const BYTE*)LPCTSTR(Str),Str.GetLength()+1);
            Str = "https://www.softcomputer.com/itms/tms_related.php?Client=%CLIENT%&ID=%ID%";
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\iTMS","ChildTasksURL",REG_SZ,(const BYTE*)LPCTSTR(Str),Str.GetLength()+1);
            Str = "https://www.softcomputer.com/itms/showall.php?Client=%CLIENT%&ID=%ID%";
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\iTMS","RelatedTasksURL",REG_SZ,(const BYTE*)LPCTSTR(Str),Str.GetLength()+1);
            DWORD def = 0;
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\iTMS","ViewTaskHotkey",REG_DWORD,(const BYTE*)&def,sizeof(DWORD));
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\iTMS","ViewChildTasksHotkey",REG_DWORD,(const BYTE*)&def,sizeof(DWORD));
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\iTMS","ViewParentTaskHotkey",REG_DWORD,(const BYTE*)&def,sizeof(DWORD));
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\iTMS","ViewRelatedTasksHotkey",REG_DWORD,(const BYTE*)&def,sizeof(DWORD));
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\iTMS","DefectsInSoftTest",REG_DWORD,(const BYTE*)&def,sizeof(DWORD));
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\iTMS","Login",REG_SZ,(const BYTE*)LPCTSTR(""),0);
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\iTMS","Password",REG_SZ,(const BYTE*)LPCTSTR(""),0);
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\iTMS","Encrypted",REG_SZ,(const BYTE*)LPCTSTR(""),0);

            if (MyMessageBox(NULL,"New URL \"iTMS\" has been added to the list of predefined URLs.\nWould you like to make it default URL?",szWinName,MB_YESNO|MB_ICONQUESTION)==IDYES)
            {
                // make previously default URL not default
                int SubKeyIndex = 0;
                CString SubKeyName = "";
                while (Reg.GetSubKeyName(RegistryKey+"\\"+LinksSubKey,SubKeyIndex,SubKeyName.GetBuffer(255)))
                {
                    SubKeyName.ReleaseBuffer();
                    DWORD Default = 0;
                    DWORD DWordSize = sizeof(DWORD);
                    Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"Default",REG_DWORD,(LPBYTE)&Default,DWordSize);
                    if (Default == 1)
                    {
                        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+SubKeyName,"Default",REG_DWORD,(const BYTE*)&def,sizeof(DWORD));
                    }
                    SubKeyIndex++;
                }
                def = 1;
            }
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\iTMS","Default",REG_DWORD,(const BYTE*)&def,sizeof(DWORD));
        }
        else // "iTMS" URL was previously defined by user, RelatedTasksURL needs to be added and probably ChildTasksURL should be corrected
        {
            const DWORD ValueLength = 16000;
            CString Str = "";
            Reg.ReadValue(RegistryKey+"\\"+LinksSubKey+"\\iTMS","ChildTasksURL",REG_SZ,(LPBYTE)Str.GetBuffer(ValueLength),ValueLength);
            Str.ReleaseBuffer();
            if (Str.Find("itms/showall.php") != -1)
            {
                if (MyMessageBox(NULL,"You already have \"iTMS\" URL defined.\nHowever, \"Open Child Tasks\" link should be corrected.\nWould you like the link to be corrected automatically?",szWinName,MB_YESNO|MB_ICONQUESTION)==IDYES)
                {
                    Str = "https://www.softcomputer.com/itms/tms_related.php?Client=%CLIENT%&ID=%ID%";
                    Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\iTMS","ChildTasksURL",REG_SZ,(const BYTE*)LPCTSTR(Str),Str.GetLength()+1);
                }
            }
            Str = "https://www.softcomputer.com/itms/showall.php?Client=%CLIENT%&ID=%ID%";
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\iTMS","RelatedTasksURL",REG_SZ,(const BYTE*)LPCTSTR(Str),Str.GetLength()+1);
        }
        // adding RelatedTasksURL to "Alternative TMS" and "Usual TMS"
        if (Reg.KeyPresent(RegistryKey+"\\"+LinksSubKey+"\\Alternative TMS"))
        {
            CString Str = "http://scc1.softcomputer.com/~alttms/relatives.php?Client=%CLIENT%&ID=%ID%";
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\Alternative TMS","RelatedTasksURL",REG_SZ,(const BYTE*)LPCTSTR(Str),Str.GetLength()+1);
        }
        if (Reg.KeyPresent(RegistryKey+"\\"+LinksSubKey+"\\Usual TMS"))
        {
            CString Str = "https://www.softcomputer.com/itms/showall.php?Client=%CLIENT%&ID=%ID%";
            Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\Usual TMS","RelatedTasksURL",REG_SZ,(const BYTE*)LPCTSTR(Str),Str.GetLength()+1);
        }
        // adding defect record for LMM defects
        if (Reg.KeyPresent(RegistryKey+"\\"+DefectsSubKey))
        {
            if (!IsDefectInRegistry("LMM;"))
            {
                CString value = "";
                value.Format("%s;%s;%s;%s;%s;%s", "LMM", "ST_LM_MAYO_SYNCH", DefectsLink, ChildDefectsLink, ParentDefectLink, RelatedDefectsLink);
                Reg.AddValue(RegistryKey+"\\"+DefectsSubKey,"LMM",REG_SZ,(const BYTE*)LPCTSTR(value),value.GetLength()+1);
            }
        }
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey,"iTMS",REG_SZ,(const BYTE*)LPCTSTR(""),0); // flag meaning 3.0 version or higher is used
    }
}

void CSettings::SaveLinksSettings()
{
//  deleting old settings if they exist
    if (Reg.KeyPresent(RegistryKey+"\\"+LinksSubKey))
    {
        if (!Reg.DeleteKeyIncludingSubKeys(HKEY_CURRENT_USER,RegistryKey+"\\"+LinksSubKey))
        {
            MyMessageBox(NULL, "Could not update Windows Registry with new URLs settings", szWinName, MB_OK | MB_ICONERROR);
            return;
        }
    }

//  restoring flag that was set in CSettings::AddingNewURLs() meaning 3.0 version or higher is used
    Reg.AddValue(RegistryKey+"\\"+LinksSubKey,"iTMS",REG_SZ,(const BYTE*)LPCTSTR(""),0);

//  saving new settings
    if (links.empty()) // load default set of URLs
    {
        links.push_back(link("Alternative TMS","http://scc1.softcomputer.com/~alttms/viewtask.php?Client=%CLIENT%&ID=%ID%",
            "http://scc1.softcomputer.com/~alttms/showtasks.php?ParentClient=%CLIENT%&ParentID=%ID%",
            "http://scc1.softcomputer.com/~alttms/relatives.php?Client=%CLIENT%&ID=%ID%",
            0,0,0,0,false,"","",false));
        links.push_back(link("iTMS","https://www.softcomputer.com/itms/gentaskdetails.php?Client=%CLIENT%&ID=%ID%",
            "https://www.softcomputer.com/itms/tms_related.php?Client=%CLIENT%&ID=%ID%",
            "https://www.softcomputer.com/itms/showall.php?Client=%CLIENT%&ID=%ID%",
            0,0,0,0,true,"","",false));
    }
    for (unsigned int i=0; i<links.size(); i++)
    {
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"TaskURL",REG_SZ,(const BYTE*)LPCTSTR(links[i].TaskURL),links[i].TaskURL.GetLength()+1);
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"ChildTasksURL",REG_SZ,(const BYTE*)LPCTSTR(links[i].ChildTasksURL),links[i].ChildTasksURL.GetLength()+1);
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"RelatedTasksURL",REG_SZ,(const BYTE*)LPCTSTR(links[i].RelatedTasksURL),links[i].RelatedTasksURL.GetLength()+1);
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"ViewTaskHotkey",REG_DWORD,(const BYTE*)&links[i].ViewTaskHotKey,sizeof(DWORD));
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"ViewChildTasksHotkey",REG_DWORD,(const BYTE*)&links[i].ViewChildTasksHotKey,sizeof(DWORD));
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"ViewParentTaskHotkey",REG_DWORD,(const BYTE*)&links[i].ViewParentTaskHotKey,sizeof(DWORD));
        Reg.AddValue(RegistryKey+"\\"+LinksSubKey+"\\"+links[i].Caption,"ViewRelatedTasksHotkey",REG_DWORD,(const BYTE*)&links[i].ViewRelatedTasksHotKey,sizeof(DWORD));
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
    Reg.AddValue(RegistryKey+"\\"+FlagsSubKey,"NewSeparators",REG_SZ,(const BYTE*)LPCTSTR(""),0); // new flag added in 3.1
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"TasksSeparators",REG_SZ,(const BYTE*)LPCTSTR(TasksSeparators),TasksSeparators.GetLength()+1);
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"FillID",REG_DWORD,(const BYTE*)&FillID,sizeof(DWORD));
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"iTMS RTM regex",REG_SZ,(const BYTE*)LPCTSTR(iTMSRtmRegEx),iTMSRtmRegEx.GetLength()+1);
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"ActionBody regex",REG_SZ,(const BYTE*)LPCTSTR(ActBodyRegEx),ActBodyRegEx.GetLength()+1);
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"ActionHeader regex",REG_SZ,(const BYTE*)LPCTSTR(ActHeaderRegEx),ActHeaderRegEx.GetLength()+1);
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"AA_ID regex",REG_SZ,(const BYTE*)LPCTSTR(AA_ID_RegEx),AA_ID_RegEx.GetLength()+1);
    Reg.AddValue(RegistryKey+"\\"+FormatSubKey,"iTMS Timesheets regex",REG_SZ,(const BYTE*)LPCTSTR(iTMSTimesheetsRegEx),iTMSTimesheetsRegEx.GetLength()+1);
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
    CString value_name, value; // value_name == "Item0"; value == "LAB;ST_LABGUI_SYNCH;URL1;URL2;URL3;URL4";
    for (int i=0; i<defects_number; i++)
    {
        value_name.Format("Item%d",i);
        value.Format("%s;%s;%s;%s;%s;%s",defects[i].ClientID,defects[i].STProject,defects[i].DefectURL,defects[i].ChildDefectsURL,defects[i].ParentDefectURL,defects[i].RelatedDefectsURL);
        Reg.AddValue(RegistryKey+"\\"+DefectsSubKey,value_name,REG_SZ,(const BYTE*)LPCTSTR(value),value.GetLength()+1);
    }
    Reg.AddValue(RegistryKey+"\\"+FlagsSubKey,"SIF",REG_SZ,(const BYTE*)LPCTSTR(""),0); // this flag was added in 3.1
    Reg.AddValue(RegistryKey+"\\"+FlagsSubKey,"HF",REG_SZ,(const BYTE*)LPCTSTR(""),0); // this flag was added in 3.1
}

void CSettings::SaveHistorySettings()
{
    Reg.DeleteKey(RegistryKey,HistorySubKey);
    Reg.AddValue(RegistryKey+"\\"+HistorySubKey,"MaxHistoryItems",REG_DWORD,(const BYTE*)&MaxHistoryItems,sizeof(DWORD));
    CString value_name, value;
    for (unsigned int i=0; i < History.size(); i++)
    {
        value_name.Format("Item%d",i);
        value = History[i];
        Reg.AddValue(RegistryKey+"\\"+HistorySubKey,value_name,REG_SZ,(const BYTE*)LPCTSTR(value),value.GetLength()+1);
    }
}

void CSettings::SaveOtherSettings()
{
    DWORD buf;
    buf = EnableOpacity ? 1:0;
    Reg.AddValue(RegistryKey+"\\"+OtherSubKey,"EnableOpacity",REG_DWORD,(const BYTE*)&buf,sizeof(DWORD));
    buf = InactiveOpacity;
    Reg.AddValue(RegistryKey+"\\"+OtherSubKey,"InactiveOpacity",REG_DWORD,(const BYTE*)&buf,sizeof(DWORD));
    buf = ActiveOpacity;
    Reg.AddValue(RegistryKey+"\\"+OtherSubKey,"ActiveOpacity",REG_DWORD,(const BYTE*)&buf,sizeof(DWORD));
    buf = WinKey;
    Reg.AddValue(RegistryKey+"\\"+OtherSubKey,"WinKey",REG_DWORD,(const BYTE*)&buf,sizeof(DWORD));
    buf = HotKey;
    Reg.AddValue(RegistryKey+"\\"+OtherSubKey,"HotKey",REG_DWORD,(const BYTE*)&buf,sizeof(DWORD));
    buf = SetFocusToTaskName ? 1: 0;
    Reg.AddValue(RegistryKey+"\\"+OtherSubKey,"SetFocusToTaskName",REG_DWORD,(const BYTE*)&buf,sizeof(DWORD));
    buf = HighlightTaskName ? 1: 0;
    Reg.AddValue(RegistryKey+"\\"+OtherSubKey,"HighlightTaskName",REG_DWORD,(const BYTE*)&buf,sizeof(DWORD));
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
        if (((unsigned char)(String[i]) == '_') || (isalpha_cp1251((unsigned char)(String[i]))) || (isdigit((unsigned char)(String[i]))))
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
    unsigned char duplicates[256];
    ZeroMemory(duplicates,256);
    int i = 0;
    while (i<String.GetLength())
    {
        if (duplicates[(unsigned char)(String[i])])
        {
            String.Delete(i);
        }
        else
        {
            duplicates[(unsigned char)(String[i])]++;
            i++;
        }
    }
    return String.GetLength();
}

void CSettings::ImportSettings(LPCTSTR lpSubKey)
{
    if (Reg.KeyPresent(lpSubKey))
    {
        if (MyMessageBox(NULL,"Settings of previous TMS Launcher version were detected.\nThey can be imported to this version and then removed.\n\nWould you like to import previous settings?",szWinName,MB_YESNO|MB_ICONQUESTION)==IDYES)
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
            if ((RightClickAction < 0)||(RightClickAction > 3)) // old versions do not have "View Related Tasks" item
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
        if (MyMessageBox(NULL,"Would you like to remove previous settings?",szWinName,MB_YESNO|MB_ICONQUESTION)==IDYES)
        {
            Reg.DeleteKeyIncludingSubKeys(HKEY_CURRENT_USER,lpSubKey);
        }
    }
}

int CSettings::GetDefaultUrlIndex()
{
    int index = -1;
    for (unsigned int i = 0; i < links.size(); i++)
    {
        if (links[i].Default)
        {
            index = i;
            break;
        }
    }
    return index;
}

// returns true if there is a record among defects with Client = ClientID
// additionally it determines if "SIF" or "HF" should be considered as regular defect or as special SIF or HF
// depending on URL used to open it
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

    for (unsigned int i=0; i < defects.size(); i++)
    {
        if (CompareNoCaseCP1251(Client,defects[i].ClientID)==0)
        {
            if (Project != NULL)
            {
                *Project = defects[i].STProject;
            }
            if (index != NULL)
            {
                *index = i;
            }
            if (IsSIF(i)) break; // in this case defect with ClientID == "SIF" is considered as special SIF but not as regular defect
            if (IsHF(i)) break;  // in this case defect with ClientID == "HF" is considered as special HF but not as regular defect
            result = true;       // this is regular defect
            break;
        }
    }

    return result;
}

// returns true if a defect should be considered as special SIF but not as regular defect
bool CSettings::IsSIF(int index)
{
    if (index < 0) return false;
    bool result = false;

    if (CompareNoCaseCP1251(defects[index].ClientID,"SIF")==0)
    {
        CString temp = defects[index].DefectURL;
        StringToUpperCase(temp);
        if (temp.Find("HTTP://SE-WEB")==0)
        {
            result = true;
        }
    }
    return result;
}

// returns true if a defect should be considered as special HF but not as regular defect
bool CSettings::IsHF(int index)
{
    if (index < 0) return false;
    bool result = false;

    if (CompareNoCaseCP1251(defects[index].ClientID,"HF")==0)
    {
        CString temp = defects[index].DefectURL;
        StringToUpperCase(temp);
        if (temp.Find("HTTP://SE")==0)
        {
            result = true;
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
        case VIEW_RELATED_TASKS:
        case VIEW_RELATED_TASKS_HOTKEY:
            return (links[GetDefaultUrlIndex()].DefectsInSoftTest);
        default:
            unsigned int index = wID / 4;
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
    for (unsigned int i=0; i<links_to_sort.size()-1; i++)
    {
        bool exchange = false;
        for (unsigned int j=1; j<links_to_sort.size()-i; j++)
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

// returns true if there is a value in RegistryKey\DefectsSubKey that begins with text specified in "Client" parameter
bool CSettings::IsDefectInRegistry(const char *Client)
{
    DWORD DWordSize=sizeof(DWORD);
    DWORD MaxValueNameLength = 255;
    DWORD MaxValueLength = 16000;
    int defects_number = 0;
    defects_number = Reg.GetNumberOfValues(RegistryKey+"\\"+DefectsSubKey,&MaxValueNameLength,&MaxValueLength);
    MaxValueNameLength++;
    MaxValueLength++;
    DWORD type;
    CString value_name = "", value = ""; // value_name == "Item[i]"; value == "LAB;ST_LABGUI_SYNCH;URL1;URL2;URL3;URL4";
    for (int i=0; i<defects_number; i++)
    {
        DWORD ValueNameLength = MaxValueNameLength;
        DWORD ValueLength = MaxValueLength;
        value_name = "", value = "";
        bool res = Reg.GetValueName(RegistryKey+"\\"+DefectsSubKey,i,value_name.GetBuffer(ValueNameLength),&ValueNameLength,&type);
        value_name.ReleaseBuffer();
        if (res && (type == REG_SZ))
        {
            Reg.ReadValue(RegistryKey+"\\"+DefectsSubKey,value_name,REG_SZ,(LPBYTE)value.GetBuffer(ValueLength),ValueLength);
            value.ReleaseBuffer();
            if (value.Find(Client)==0)
            {
                return true;
            }
        }
    }
    return false;
}

void CSettings::SetRelatedDefectsFilter()
{
CString RelatedDefectsFilter1 = 
"SELECT * FROM BUG WHERE \n\
bg_bug_id in \n\
( \n\
    SELECT to_number(ln_trg_id) FROM st_linking_sys.lnk_link  \n\
    WHERE  \n\
    ln_src_proj = (select SPJ_ID from st_linking_sys.STP_PROJECT where SPJ_NAME = user) AND \n\
    ln_trg_proj = (select SPJ_ID from st_linking_sys.STP_PROJECT where SPJ_NAME = user) AND \n\
    (  \n\
        ( \n\
            ln_trg_type = 5 AND ln_src_type = 3 AND  \n\
            /* All runs of all revisions of the TC from \"Test Reference\" field of the given defect(s) */ \n\
            ln_src_id in  \n\
            ( \n\
                select RN_RUN_ID from RUN where RN_TEST_ID in  \n\
                ( \n\
                    select TS_TEST_ID from TEST where TS_PARENT_ID in  \n\
                    ( \n\
                        select TS_PARENT_ID from TEST where TS_TEST_ID in  \n\
                        ( \n\
                            select BG_TEST_REFERENCE from BUG where BG_BUG_ID in (%ID%) \n\
                            and BG_CYCLE_REFERENCE is not NULL  \n\
                            and BG_BUG_VER_STAMP = 0 AND instr(BG_CYCLE_REFERENCE,CHR(127)) = 0 \n\
                        ) \n\
                    ) \n\
                ) \n\
            ) \n\
        ) \n\
        or  \n\
        ( \n\
            ln_trg_type = 5 AND ln_src_type = 1 AND  \n\
            /* All revisions of the TC from \"Test Reference\" field of the given defect(s) */ \n\
            ln_src_id in  \n\
            ( \n\
                select TS_TEST_ID from TEST where TS_PARENT_ID in  \n\
                ( \n\
                    select TS_PARENT_ID from TEST where TS_TEST_ID in  \n\
                    ( \n\
                        select BG_TEST_REFERENCE from BUG where BG_BUG_ID in (%ID%) \n\
                        and BG_CYCLE_REFERENCE is not NULL  \n\
                        and BG_BUG_VER_STAMP = 0 AND instr(BG_CYCLE_REFERENCE,CHR(127)) = 0 \n\
                    ) \n\
                ) \n\
            )  \n\
        )\n ";

CString RelatedDefectsFilter2 = " \
        or  \n\
        ( \n\
            /* All defects linked to the given defect(s) */ \n\
            ln_trg_type = 5 AND ln_src_type = 5 AND ln_src_id in (%ID%) \n\
        ) \n\
    ) \n\
    UNION \n\
    SELECT to_number(ln_src_id) FROM st_linking_sys.lnk_link  \n\
    WHERE  \n\
    ln_src_proj = (select SPJ_ID from st_linking_sys.STP_PROJECT where SPJ_NAME = user) AND  \n\
    ln_trg_proj = (select SPJ_ID from st_linking_sys.STP_PROJECT where SPJ_NAME = user) AND \n\
    (  \n\
        ( \n\
            ln_trg_type = 3 AND ln_src_type = 5 AND  \n\
            /* All runs of all revisions of the TC from \"Test Reference\" field of the given defect(s) */ \n\
            ln_trg_id in  \n\
            ( \n\
                select RN_RUN_ID from RUN where RN_TEST_ID in  \n\
                ( \n\
                    select TS_TEST_ID from TEST where TS_PARENT_ID in  \n\
                    ( \n\
                        select TS_PARENT_ID from TEST where TS_TEST_ID in  \n\
                        ( \n\
                            select BG_TEST_REFERENCE from BUG where BG_BUG_ID in (%ID%)  \n\
                            and BG_CYCLE_REFERENCE is not NULL  \n\
                            and BG_BUG_VER_STAMP = 0 AND instr(BG_CYCLE_REFERENCE,CHR(127)) = 0 \n\
                        ) \n\
                    ) \n\
                ) \n\
            ) \n\
        ) \n\
        or  \n\
        ( \n\
            ln_trg_type = 1 AND ln_src_type = 5 AND  \n\
            /* All revisions of the TC from \"Test Reference\" field of the given defect(s) */ \n\
            ln_trg_id in \n\
            ( \n\
                select TS_TEST_ID from TEST where TS_PARENT_ID in  \n\
                ( \n\
                    select TS_PARENT_ID from TEST where TS_TEST_ID in  \n\
                    (\n ";

CString RelatedDefectsFilter3 = " \
                        select BG_TEST_REFERENCE from BUG where BG_BUG_ID in (%ID%)  \n\
                        and BG_CYCLE_REFERENCE is not NULL  \n\
                        and BG_BUG_VER_STAMP = 0 AND instr(BG_CYCLE_REFERENCE,CHR(127)) = 0 \n\
                    ) \n\
                ) \n\
            ) \n\
        ) \n\
        or \n\
        ( \n\
            /* All defects linked to the given defect(s) */ \n\
            ln_trg_type = 5 AND ln_src_type = 5 AND ln_trg_id in (%ID%) \n\
        ) \n\
    ) \n\
    UNION \n\
    SELECT to_number(ln_trg_id) FROM st_linking_sys.lnk_link  \n\
    WHERE  \n\
    ln_src_proj = (select SPJ_ID from st_linking_sys.STP_PROJECT where SPJ_NAME = user) AND \n\
    ln_trg_proj = (select SPJ_ID from st_linking_sys.STP_PROJECT where SPJ_NAME = user) AND \n\
    (  \n\
        ( \n\
            ln_trg_type = 5 AND ln_src_type = 4 AND  \n\
            /* All runs of all revisions of the UTC from \"Test Reference\" field of the given defect(s) */ \n\
            ln_src_id in \n\
            ( \n\
                select URN_RUN_ID from UNIT_RUN where URN_TEST_ID in  \n\
                ( \n\
                    select UTS_TEST_ID from UNIT_TEST where UTS_PARENT_ID in  \n\
                    ( \n\
                        select UTS_PARENT_ID from UNIT_TEST where UTS_TEST_ID in  \n\
                        ( \n\
                            select BG_TEST_REFERENCE from BUG where BG_BUG_ID in (%ID%) \n\
                            and BG_CYCLE_REFERENCE is not NULL  \n\
                            and (BG_BUG_VER_STAMP = 1 OR instr(BG_CYCLE_REFERENCE,CHR(127)) <> 0) \n\
                        ) \n\
                    ) \n\
                ) \n\
            ) \n\
        )  \n\
        or  \n\
        ( \n\
            ln_trg_type = 5 AND ln_src_type = 2 AND  \n\
            /* All revisions of the UTC from \"Test Reference\" field of the given defect(s) */ \n\
            ln_src_id in \n\
            ( \n";

CString RelatedDefectsFilter4 = " \
                select UTS_TEST_ID from UNIT_TEST where UTS_PARENT_ID in  \n\
                ( \n\
                    select UTS_PARENT_ID from UNIT_TEST where UTS_TEST_ID in  \n\
                    ( \n\
                        select BG_TEST_REFERENCE from BUG where BG_BUG_ID in (%ID%) \n\
                        and BG_CYCLE_REFERENCE is not NULL  \n\
                        and (BG_BUG_VER_STAMP = 1 OR instr(BG_CYCLE_REFERENCE,CHR(127)) <> 0) \n\
                    ) \n\
                ) \n\
            )  \n\
        )  \n\
    ) \n\
    UNION \n\
    SELECT to_number(ln_src_id) FROM st_linking_sys.lnk_link  \n\
    WHERE  \n\
    ln_src_proj = (select SPJ_ID from st_linking_sys.STP_PROJECT where SPJ_NAME = user) AND \n\
    ln_trg_proj = (select SPJ_ID from st_linking_sys.STP_PROJECT where SPJ_NAME = user) AND \n\
    (  \n\
        ( \n\
            ln_trg_type = 4 AND ln_src_type = 5 AND  \n\
            /* All runs of all revisions of the UTC from \"Test Reference\" field of the given defect(s) */ \n\
            ln_trg_id in \n\
            ( \n\
                select URN_RUN_ID from UNIT_RUN where URN_TEST_ID in  \n\
                ( \n\
                    select UTS_TEST_ID from UNIT_TEST where UTS_PARENT_ID in  \n\
                    ( \n\
                        select UTS_PARENT_ID from UNIT_TEST where UTS_TEST_ID in  \n\
                        ( \n\
                            select BG_TEST_REFERENCE from BUG where BG_BUG_ID in (%ID%) \n\
                            and BG_CYCLE_REFERENCE is not NULL  \n\
                            and (BG_BUG_VER_STAMP = 1 OR instr(BG_CYCLE_REFERENCE,CHR(127)) <> 0) \n\
                        ) \n\
                    ) \n\
                ) \n\
            ) \n\
        ) \n\
        or  \n\
        ( \n\
            ln_trg_type = 2 AND ln_src_type = 5 AND  \n\
            /* All revisions of the UTC from \"Test Reference\" field of the given defect(s) */ \n\
            ln_trg_id in \n\
            ( \n";

CString RelatedDefectsFilter5 = " \
                select UTS_TEST_ID from UNIT_TEST where UTS_PARENT_ID in  \n\
                ( \n\
                    select UTS_PARENT_ID from UNIT_TEST where UTS_TEST_ID in  \n\
                    ( \n\
                        select BG_TEST_REFERENCE from BUG where BG_BUG_ID in (%ID%) \n\
                        and BG_CYCLE_REFERENCE is not NULL  \n\
                        and (BG_BUG_VER_STAMP = 1 OR instr(BG_CYCLE_REFERENCE,CHR(127)) <> 0) \n\
                    ) \n\
                ) \n\
            )  \n\
        )  \n\
    ) \n\
)";
RelatedDefectsFilter = RelatedDefectsFilter1 + RelatedDefectsFilter2 + 
                        RelatedDefectsFilter3 + RelatedDefectsFilter4 + 
                        RelatedDefectsFilter5;
}