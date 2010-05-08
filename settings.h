/*
    File name: settings.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: August 30, 2006
*/

#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

#ifdef INCLUDE_VERID
 static char settings_h[]="@(#)$RCSfile: settings.h,v $$Revision: 1.27 $$Date: 2007/01/10 18:12:42Z $";
#endif

#include "resource.h"
#include "Registry.h"
#include <vector>

struct defect
{
    CString ClientID;
    CString STProject;
    CString DefectURL, ChildDefectsURL, ParentDefectURL;
    
    defect(const CString& ID, const CString& Project, const CString& OpenDefectURL, 
           const CString& OpenChildDefectsURL, const CString& OpenParentDefectURL)
    {
        ClientID = ID;
        STProject = Project;
        DefectURL = OpenDefectURL;
        ChildDefectsURL = OpenChildDefectsURL;
        ParentDefectURL = OpenParentDefectURL;
    }

    const defect& operator=(const defect& def)
    {
        if (this == &def) return *this;
        ClientID = def.ClientID;
        STProject = def.STProject;
        DefectURL = def.DefectURL;
        ChildDefectsURL = def.ChildDefectsURL;
        ParentDefectURL = def.ParentDefectURL;
        return *this;
    }
};

struct link
{
    CString Caption;
    CString TaskURL;
    CString ChildTasksURL;
    UINT ViewTaskHotKey;
    UINT ViewChildTasksHotKey;
    UINT ViewParentTaskHotKey;
    CString Login;
    CString Password;
    bool Default;
    bool DefectsInSoftTest;
    link(const CString& caption, const CString& task_url, const CString& child_tasks_url,
         const UINT TaskHotKey, const UINT ChildTasksHotKey, const UINT ParentTaskHotKey,
         bool UseByDefault, const CString& login, const CString& password, bool STDefects)
    {
        Caption = caption;
        TaskURL = task_url;
        ChildTasksURL = child_tasks_url;
        ViewTaskHotKey = TaskHotKey;
        ViewChildTasksHotKey = ChildTasksHotKey;
        ViewParentTaskHotKey = ParentTaskHotKey;
        Default = UseByDefault;
        Login = login;
        Password = password;
        DefectsInSoftTest = STDefects;
    }
    const link& operator=(const link& Link)
    {
        if (this == &Link) return *this;
        Caption = Link.Caption;
        TaskURL = Link.TaskURL;
        ChildTasksURL = Link.ChildTasksURL;
        ViewTaskHotKey = Link.ViewTaskHotKey;
        ViewChildTasksHotKey = Link.ViewChildTasksHotKey;
        ViewParentTaskHotKey = Link.ViewParentTaskHotKey;
        Default = Link.Default;
        Login = Link.Login;
        Password = Link.Password;
        DefectsInSoftTest = Link.DefectsInSoftTest;
        return *this;
    }
};

class CSettings
{
public:
    bool AutoRun;
    bool Expand;
    bool Minimize;
    bool SingleClick;
    int RightClickAction;
    int RightClickAction2;
    int xPos;
    int yPos;
    int MaxHistoryItems;
    int MaxPossibleHistory;
    std::vector<link>links;
    std::vector<defect> defects;
    std::vector<CString> History;
    void sort_links(std::vector<link> &links_to_sort);
    // Task name format: [%CLIENT%-]%ID%[-%EXT%]
    CString Separators;
    CString TasksSeparators;
    bool FillID;
    int MinClientName;
    int MaxClientName;
    int MinIDName;
    int MaxIDName;
    int MinExt;
    int MaxExt;
    int TaskNameControlType;
    CString DefectsLink;
    CString ChildDefectsLink;
    CString ParentDefectLink;
    CString BrowserPath;
    bool DefaultBrowser;
    CString SoftTestPath;        // path to SoftTest binary file including the name of the file
    CString SoftTestLogin;       // user's login to SoftTest
    CString SoftTestPassword;    // user's password to SoftTest
    CString SoftTestFiltersPath; // path to SoftTest filters
    CString SoftTestFilterName;  // name of TMS Launcher's filter for SoftTest
    CString DefectFilter;        // contents of TMS Launcher's filter for SoftTest for defect
    CString ChildDefectsFilter;  // contents of TMS Launcher's filter for SoftTest for child defects
    CString ParentDefectFilter;  // contents of TMS Launcher's filter for SoftTest for parent defect
    CString RtmRegEx;            // regular expression describing requirements section in QC actions of tasks
    CString QcRegEx;             // regular expression describing QC actions of tasks
    CString QbRegEx;             // regular expression describing QB/QR actions of tasks
    const CString& GetSoftTestCommandLine(const char *Project);
    CSettings(const char* RegKey, const char* AutoRunRegKey, const char* AutoRunValName, 
              const char* DefectsSubKeyName, const char* TasksSubKeyName, const char* LinksSubKeyName,
              const char* SoftTestSubKeyName, const char* HistorySubKeyName);
    void LoadSettings();
    void ImportSettings(LPCTSTR lpSubKey);
    void SaveGeneralSettings(bool AfterImporting = false);
    void SaveFormatSettings();
    void SaveDefectsSettings();
    void SaveLinksSettings();
    void SaveSoftTestSettings();
    void SaveHistorySettings();
    bool SettingsAvailable();
    int RemoveUnacceptableSeparators(CString &String);
    int RemoveDuplicateSeparators(CString &String);
    bool CorrectCRLF(CString &Separators, CString &TasksSeparators);
    int GetDefaultUrlIndex();
    bool IsDefect(const char *Client, CString *Project, int *index);
    bool OpenDefectsInSoftTest(INT wID);
protected:
    BYTE x;
    Registry Reg;
    CString RegistryKey;
    CString DefectsSubKey;
    CString FormatSubKey;
    CString LinksSubKey;
    CString SoftTestSubKey;
    CString HistorySubKey;
    CString AutoRunRegistryKey;
    CString AutoRunValueName;
    CString SoftTestCommandLine; // command line that used to launch SoftTest
    void Crypt(CString &String);
};

#endif