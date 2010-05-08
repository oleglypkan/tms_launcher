/*
    File name: settings.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: January 17, 2006
*/

#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

#ifdef INCLUDE_VERID
 static char settings_h[]="@(#)$RCSfile: settings.h,v $$Revision: 1.17 $$Date: 2006/02/08 13:37:56Z $";
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
    bool Default;
    CString Login;
    CString Password;
    link(const CString& caption, const CString& task_url, const CString& child_tasks_url,
         const UINT TaskHotKey, const UINT ChildTasksHotKey, const UINT ParentTaskHotKey,
         bool UseByDefault, const CString& login, const CString& password)
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
    // Task name format: [%CLIENT%-]%ID%[-%EXT%]
    CString Separators;
    CString TasksSeparators;
    int MinClientName;
    int MaxClientName;
    int MinIDName;
    int MaxIDName;
    int MinExt;
    int MaxExt;
    int MinTaskName;
    int MaxTaskName;
    int TaskNameControlType;
    CString DefectsLink;
    CString ChildDefectsLink;
    CString ParentDefectLink;
    CString BrowserPath;
    bool DefaultBrowser;
    CSettings(const char* RegKey, const char* AutoRunRegKey, const char* AutoRunValName, 
              const char* DefectsSubKeyName, const char* TasksSubKeyName, const char* LinksSubKeyName);
    void LoadSettings();
    void ImportSettings(LPCTSTR lpSubKey);
    void SaveGeneralSettings(bool AfterImporting = false);
    void SaveFormatSettings();
    void SaveDefectsSettings();
    void SaveLinksSettings();
    bool SettingsAvailable();
    int RemoveUnacceptableSeparators(CString &String);
    int RemoveDuplicateSeparators(CString &String);
protected:
    Registry Reg;
    CString RegistryKey;
    CString DefectsSubKey;
    CString FormatSubKey;
    CString LinksSubKey;
    CString AutoRunRegistryKey;
    CString AutoRunValueName;
};

#endif