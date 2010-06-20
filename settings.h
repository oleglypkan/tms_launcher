/*
    File name: settings.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
*/

#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

#ifdef INCLUDE_VERID
 static char settings_h[]="@(#)$RCSfile: settings.h,v $$Revision: 1.39 $$Date: 2009/03/22 21:49:36Z $";
#endif

#include "resource.h"
#include "Registry.h"
#include "maindlg.h"
#include <vector>

struct defect
{
    CString ClientID;
    CString STProject;
    CString DefectURL, ChildDefectsURL, ParentDefectURL, RelatedDefectsURL;
    
    defect(const CString& ID, const CString& Project, const CString& OpenDefectURL, 
           const CString& OpenChildDefectsURL, const CString& OpenParentDefectURL,
           const CString& OpenRelatedDefectsURL)
    {
        ClientID = ID;
        STProject = Project;
        DefectURL = OpenDefectURL;
        ChildDefectsURL = OpenChildDefectsURL;
        ParentDefectURL = OpenParentDefectURL;
        RelatedDefectsURL = OpenRelatedDefectsURL;
    }

    const defect& operator=(const defect& def)
    {
        if (this == &def) return *this;
        ClientID = def.ClientID;
        STProject = def.STProject;
        DefectURL = def.DefectURL;
        ChildDefectsURL = def.ChildDefectsURL;
        ParentDefectURL = def.ParentDefectURL;
        RelatedDefectsURL = def.RelatedDefectsURL;
        return *this;
    }
};

struct link
{
    CString Caption;
    CString TaskURL;
    CString ChildTasksURL;
    CString RelatedTasksURL;
    UINT ViewTaskHotKey;
    UINT ViewChildTasksHotKey;
    UINT ViewParentTaskHotKey;
    UINT ViewRelatedTasksHotKey;
    CString Login;
    CString Password;
    bool Default;
    bool DefectsInSoftTest;
    link(const CString& caption, const CString& task_url, const CString& child_tasks_url, const CString& related_tasks_url,
         const UINT TaskHotKey, const UINT ChildTasksHotKey, const UINT ParentTaskHotKey, const UINT RelatedTasksHotKey,
         bool UseByDefault, const CString& login, const CString& password, bool STDefects)
    {
        Caption = caption;
        TaskURL = task_url;
        ChildTasksURL = child_tasks_url;
        RelatedTasksURL = related_tasks_url;
        ViewTaskHotKey = TaskHotKey;
        ViewChildTasksHotKey = ChildTasksHotKey;
        ViewParentTaskHotKey = ParentTaskHotKey;
        ViewRelatedTasksHotKey = RelatedTasksHotKey;
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
        RelatedTasksURL = Link.RelatedTasksURL;
        ViewTaskHotKey = Link.ViewTaskHotKey;
        ViewChildTasksHotKey = Link.ViewChildTasksHotKey;
        ViewParentTaskHotKey = Link.ViewParentTaskHotKey;
        ViewRelatedTasksHotKey = Link.ViewRelatedTasksHotKey;
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
    CString HistoryTasks;
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
    CString RelatedDefectsLink;
    CString SifLink;
    CString BrowserPath;
    bool DefaultBrowser;
    bool EnableOpacity;
    BYTE InactiveOpacity;         // 0 - fully transparent; 100 - opaque
    BYTE ActiveOpacity;           // 0 - fully transparent; 100 - opaque
    UINT HotKey;                  // hotkey for fast window activation
    UINT WinKey;                  // MOD_WIN - use Win key, 0 - don't use it
    const UINT GlobalHotkeyID;    // ID of hotkey for fast window activation
    bool SetFocusToTaskName;      // if true, the cursor is placed to TaskName field after Main window is activated via hotkey or systray icon
    bool HighlightTaskName;       // if true and SetFocusToTaskName=true, text in Task Name field is highlighted after Main window is activated via hotkey or systray icon
    CString SoftTestPath;         // path to SoftTest binary file including the name of the file
    CString SoftTestLogin;        // user's login to SoftTest
    CString SoftTestPassword;     // user's password to SoftTest
    CString SoftTestFiltersPath;  // path to SoftTest filters
    CString SoftTestFilterName;   // name of TMS Launcher's filter for SoftTest
    CString DefectFilter;         // contents of TMS Launcher's filter for SoftTest for defect
    CString ChildDefectsFilter;   // contents of TMS Launcher's filter for SoftTest for child defects
    CString ParentDefectFilter;   // contents of TMS Launcher's filter for SoftTest for parent defect
    CString RelatedDefectsFilter; // contents of TMS Launcher's filter for SoftTest for related defects
    CString iTMSRtmRegEx;         // regular expression that describes requirements section in QC actions of tasks
    CString ActBodyRegEx;         // actually it is regex for action body
    CString ActHeaderRegEx;       // actually it is regex for action header
    CString AA_ID_RegEx;          // regular expression that describes AA_ID parameter in URL
    CString iTMSTimesheetsRegEx;  // regular expression that describes timesheeted hours in task
    const CString iTMSviewTask;   // URL to view task in iTMS
    const CString iTMSviewChildTasks;
    const CString iTMSviewRelatedTasks;
    const CString iTMSviewTimesheets;
    const CString& GetSoftTestCommandLine(const char *Project);
    CMainDlg *MainDialog; // pointer to main dialog window
    CSettings(const char* RegKey, const char* AutoRunRegKey, const char* AutoRunValName, 
              const char* DefectsSubKeyName, const char* TasksSubKeyName, const char* LinksSubKeyName,
              const char* SoftTestSubKeyName, const char* HistorySubKeyName, const char* OtherSubKeyName,
              const char* FlagsSubKeyName);
    void LoadSettings();
    void ImportSettings(LPCTSTR lpSubKey);
    void SaveGeneralSettings(bool AfterImporting = false);
    void SaveFormatSettings();
    void SaveDefectsSettings();
    void SaveLinksSettings();
    void SaveSoftTestSettings();
    void SaveHistorySettings();
    void SaveOtherSettings();
    bool SettingsAvailable();
    void ConvertSettings();
    void AddingNewURLs();
    int RemoveUnacceptableSeparators(CString &String);
    int RemoveDuplicateSeparators(CString &String);
    bool CorrectCRLF(CString &Separators, CString &TasksSeparators);
    int GetDefaultUrlIndex();
    bool IsDefect(const char *Client, CString *Project, int *index);
    bool OpenDefectsInSoftTest(INT wID);
    bool IsDefectInRegistry(const char *Client);
    bool IsSIF(int index);
protected:
    BYTE x;
    Registry Reg;
    CString RegistryKey;
    CString DefectsSubKey;
    CString FormatSubKey;
    CString LinksSubKey;
    CString SoftTestSubKey;
    CString HistorySubKey;
    CString OtherSubKey;
    CString FlagsSubKey;
    CString AutoRunRegistryKey;
    CString AutoRunValueName;
    CString SoftTestCommandLine; // command line that used to launch SoftTest
    void Crypt(CString &String);
    void SetRelatedDefectsFilter();
};

#endif