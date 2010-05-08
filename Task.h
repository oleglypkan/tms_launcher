/*
    File name: Task.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: January 20, 2006
*/

#ifndef TASK_H_INCLUDED
#define TASK_H_INCLUDED

#ifdef INCLUDE_VERID
 static char Task_h[]="@(#)$RCSfile: Task.h,v $$Revision: 1.6 $$Date: 2006/03/15 16:19:18Z $";
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
    }
    TASKNAME(CString client, CString separator, CString id)
    {
        Client = client;
        Separator = separator;
        ID = id;
    }
    const TASKNAME& operator=(const TASKNAME& task_name)
    {
        if (this == &task_name) return *this;
        Client = task_name.Client;
        Separator = task_name.Separator;
        ID = task_name.ID;
        return *this;
    }
    CString Client;
    CString Separator;
    CString ID;
};

struct CHILD
{
    CString TaskName;
    CString Product;
};

class TASK
{
public:
    bool IsTaskNameValid(const char *OriginalTask, CString &sClientName, CString &Sep, CString &sIDName);
    void SimpleParseTasks(const char *strTasks, std::vector<CString> &Tasks);
    bool ComplexParseTasks(const char *strTasks, std::vector<TASKNAME> &Tasks, std::vector<TASKNAME> &Defects, int &count);
    int ParseHTMLForParentTask(CString &TaskToFind, const CString &HTML); // 0 - parent found, 1 - no parent, 2 - no such task
    int ParseHTMLForParentDefect(CString &DefectToFind, const CString &HTML); // 0 - parent found, 1 - no parent, 2 - no such defect
    int ParseHTMLForChildDefects(const CString &HTML, std::vector<CString> &Tasks);
    int ParseHTMLForChildTasks(const CString &ParentTask, const CString &HTML, std::vector<CHILD> &Tasks);
protected:
    void sort_defects(std::vector<TASKNAME> &Defects);
};

#endif