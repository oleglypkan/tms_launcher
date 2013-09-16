/*
    File name: Task.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
*/

#ifndef TASK_H_INCLUDED
#define TASK_H_INCLUDED

#ifdef INCLUDE_VERID
 static char Task_h[]="@(#)$RCSfile: Task.h,v $$Revision: 1.15 $$Date: 2009/03/16 13:49:46Z $";
#endif

#include <vector>

class TASKNAME
{
public:
    TASKNAME()
    {
        Client = "";
        Separator = "";
        ID = "";
        Ext = "";
    }
    TASKNAME(CString client, CString separator, CString id, CString ext)
    {
        Client = client;
        Separator = separator;
        ID = id;
        Ext = ext;
    }
    TASKNAME& operator=(const TASKNAME& task_name)
    {
        if (this == &task_name) return *this;
        Client = task_name.Client;
        Separator = task_name.Separator;
        ID = task_name.ID;
        Ext = task_name.Ext;
        return *this;
    }
    CString Client;
    CString Separator;
    CString ID;
    CString Ext;
};

struct CHILD
{
    CString TaskName;
    CString Product;
    CString Status;
    CString MSP;
    byte level;
    CHILD()
    {
        TaskName = "";
        Product = "";
        Status = "";
        MSP = "";
        level = 1;
    }
};

class TASK
{
public:
    bool IsTaskNameValid(const char *OriginalTask, CString &sClientName, CString &Sep, CString &sIDName, CString &Ext);
    bool IsIDValid(const CString &sClientName , const CString &sIDName);
    static bool IsClientNameValid(const CString &ClientName);
    static void FillupTaskID(CString &ID);
    void SimpleParseTasks(const char *strTasks, std::vector<CString> &Tasks);
    bool ComplexParseTasks(const char *strTasks, std::vector<TASKNAME> &Tasks, std::vector<TASKNAME> &Defects, int &count);
    int ParseHTMLForParentTask(CString &TaskToFind, const CString &HTML); // 0 - parent found, 1 - no parent, 2 - no such task
    int ParseHTMLForParentDefect(CString &DefectToFind, const CString &HTML); // 0 - parent found, 1 - no parent, 2 - no such defect
    int ParseHTMLForChildDefects(const CString &HTML, std::vector<CString> &Tasks);
    int ParseHTMLForChildTasks(const CString &ParentTask, const CString &HTML, std::vector<CHILD> &Tasks);
    int ParseHTMLForSPCtasks(const CString &HTML, std::vector<CHILD> &Tasks);
    void ParseHTMLForActions(const CString &HTML, std::vector<CString> &TaskActions, bool QB);
    void ParseHTMLForAA_ID(const CString &HTML, CString &AA_ID);
    void ParseHTMLForTimesheets(const CString &HTML, std::vector<CString> &Timesheets, const CString &mask, bool &filtered);
//  true  - task should NOT be printed
//  false - task should be printed
    static bool FilterTask(const CString &string, const CString &mask);
protected:
    void sort_defects(std::vector<TASKNAME> &Defects);
    void FormatActionOutput(CString &Action, bool QB);
    void ParseActionForRequirements(const CString &Action, CString &Output);
    void RemoveSpaces(CString &s);
};

#endif