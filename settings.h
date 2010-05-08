#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

#ifdef INCLUDE_VERID
 static char settings_h[]="@(#)$RCSfile: settings.h,v $$Revision: 1.12 $$Date: 2005/07/04 14:08:44Z $";
#endif

#include "resource.h"
#include "Registry.h"
#include <vector>

struct defect
{
    CString ClientID;
    CString STProject;
    defect(const CString& ID, const CString& Project)
    {
        ClientID = ID;
        STProject = Project;
    }

    const defect& operator=(const defect& def)
    {
        if (this == &def) return *this;
        ClientID = def.ClientID;
        STProject = def.STProject;
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
    bool Default;
    link(const CString& caption, const CString& task_url, const CString& child_tasks_url,
         const UINT TaskHotKey, const UINT ChildTasksHotKey, bool UseByDefault)
    {
        Caption = caption;
        TaskURL = task_url;
        ChildTasksURL = child_tasks_url;
        ViewTaskHotKey = TaskHotKey;
        ViewChildTasksHotKey = ChildTasksHotKey;
        Default = UseByDefault;
    }
    const link& operator=(const link& Link)
    {
        if (this == &Link) return *this;
        Caption = Link.Caption;
        TaskURL = Link.TaskURL;
        ChildTasksURL = Link.ChildTasksURL;
        ViewTaskHotKey = Link.ViewTaskHotKey;
        ViewChildTasksHotKey = Link.ViewChildTasksHotKey;
        Default = Link.Default;
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
    int xPos;
    int yPos;
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
    CString DefectsLink;
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