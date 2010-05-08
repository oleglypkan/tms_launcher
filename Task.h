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
 static char Task_h[]="@(#)$RCSfile: Task.h,v $$Revision: 1.4 $$Date: 2006/02/06 17:07:30Z $";
#endif

#include <vector>

class TASK
{
public:
    bool IsTaskNameValid(CString &sTaskName, CString &sClientName, CString &sIDName);
    void ParseTasks(const char *strTasks, std::vector<CString> &Tasks);
    int ParseHTMLForParentTask(CString &TaskToFind, const CString &HTML); // 0 - parent found, 1 - no parent, 2 - no such task
    int ParseHTMLForParentDefect(CString &DefectToFind, const CString &HTML); // 0 - parent found, 1 - no parent, 2 - no such defect
    int ParseHTMLForChildDefects(const CString &HTML, std::vector<CString> &Tasks);
    int ParseHTMLForChildTasks(const CString &ParentTask, const CString &HTML, std::vector<CString> &Tasks);
};

#endif