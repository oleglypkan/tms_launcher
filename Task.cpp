/*
    File name: Task.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: February 7, 2006
*/

#include "stdafx.h"
#include "Task.h"
#include "settings.h"

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: Task.cpp,v $$Revision: 1.5 $$Date: 2006/02/07 16:28:46Z $"; 
#endif
 
extern CSettings Settings;
bool isalpha_cp1251(char ch);

bool TASK::IsTaskNameValid(CString &sTaskName, CString &sClientName, CString &sIDName)
{
//  Task name format: [%CLIENT%-]%ID%[-%EXT%]

    if (sTaskName.IsEmpty()) return false;

    sTaskName.TrimLeft();
    sTaskName.TrimRight();
    CString OriginalTask = sTaskName;

    if ((sTaskName.GetLength()<Settings.MinTaskName)||
        (sTaskName.GetLength()>Settings.MaxTaskName)) return false;

    for (int i=1; i<Settings.Separators.GetLength(); i++)
    {
        sTaskName.Replace(Settings.Separators[i],Settings.Separators[0]);
    }

    int pos = sTaskName.Find(Settings.Separators[0],0);

    // parsing task name
    if (pos > -1)
    {
        sClientName = sTaskName.Left(pos);
        sIDName = sTaskName.Right(sTaskName.GetLength()-pos-1);
        sIDName.TrimLeft(Settings.Separators[0]);
        pos = sIDName.Find(Settings.Separators[0],0);
        if (pos > -1)
        {
            int ext_len = sIDName.GetLength()-pos-1;
            if ((ext_len < Settings.MinExt) || (ext_len > Settings.MaxExt))
            {
                return false;
            }
            else
            {
                sIDName.Delete(pos,ext_len+1);
            }
        }
    }
    else // there are no separators in the task name
        if (sTaskName.GetLength()<=Settings.MaxIDName)
        {
            sClientName = "";
            sIDName = sTaskName;
        }
        else // wrong task name format
        {
            return false;
        }

    if ((sClientName.GetLength() < Settings.MinClientName) || 
        (sClientName.GetLength() > Settings.MaxClientName))
    {
        return false;
    }
    else
        if (!sClientName.IsEmpty())
        {
            // checking for correct Client name
            for (i=0; i<sClientName.GetLength()-1; i++)
            {
                if (isdigit((unsigned char)(sClientName[i]))) return false;
//                if (!isalpha_cp1251((unsigned int)(sClientName[i]))) return false;
            }
            // the last symbol in Client Name can be either an alpha character or a number (i.e. QARD3)
            if ((!isalpha_cp1251((unsigned char)(sClientName[sClientName.GetLength()-1]))) && (!isdigit((unsigned char)(sClientName[sClientName.GetLength()-1]))))
            {
                return false;
            }
        }

    if ((sIDName.GetLength() < Settings.MinIDName) || (sIDName.GetLength() > Settings.MaxIDName))
    {
        return false;
    }
    else
    {
        // checking for correct ID
        for (i=0; i<sIDName.GetLength(); i++)
        {
            if (!isdigit((unsigned int)sIDName[i])) return false;
        }
    }
    sTaskName = OriginalTask;
    return true;
}

void TASK::ParseTasks(const char *strTasks, std::vector<CString> &Tasks)
{
    CString sTasks = strTasks;
    Tasks.clear();
    for (int i=1; i<Settings.TasksSeparators.GetLength(); i++)
    {
        sTasks.Replace(Settings.TasksSeparators[i],Settings.TasksSeparators[0]);
    }
    int pos = -1;
    while ((pos = sTasks.Find(Settings.TasksSeparators[0],0)) != -1)
    {
        CString temp = sTasks.Left(pos);
        Tasks.push_back(temp);
        sTasks.Delete(0,pos);
        sTasks.TrimLeft(Settings.TasksSeparators[0]);
    }
    if (!sTasks.IsEmpty())
    {
        Tasks.push_back(sTasks);
    }
}

int TASK::ParseHTMLForParentTask(CString &TaskToFind, const CString &HTML) // 0 - parent found, 1 - no parent, 2 - no such task
{
    long pos = HTML.Find("Task "+TaskToFind+" not found...");
    if (pos != -1)
    {
        TaskToFind = "Task "+TaskToFind+" not found";
        return 2;
    }
    pos = HTML.Find("No parent</A>");
    if (pos != -1)
    {
        TaskToFind = "Task "+TaskToFind+" does not have parent task";
        return 1;
    }
    pos = HTML.Find("Parent Task</A>");
    if (pos == -1)
    {
        TaskToFind = "Cannot find parent task for "+TaskToFind;
        return 1;
    }
    while ((pos > 0) && (HTML.GetAt(pos) != '?'))
    {
        pos--;
    }
    if (pos == 0)
    {
        TaskToFind = "Cannot find parent task for "+TaskToFind;
        return 1;
    }
    else
    {
        TaskToFind = HTML.Right(HTML.GetLength()-pos-1-lstrlen("Client="));
        pos = TaskToFind.Find("ID=");
        TaskToFind = TaskToFind.Left(pos+lstrlen("ID=")+5); // 5 == lstrlen("XXXXX")
        TaskToFind.Replace("&ID=","-");
    }
    return 0;
}

 // 0 - parent found, 1 - no parent, 2 - no such defect
int TASK::ParseHTMLForParentDefect(CString &DefectToFind, const CString &HTML)
{
    long pos = HTML.Find("<title>R&amp;D / SoftTest / Defect Info / @");
    if (pos != -1)
    {
        DefectToFind = "Defect "+DefectToFind+" not found";
        return 2;
    }
    pos = HTML.Find("<title>R&amp;D / SoftTest / Defect Info / 0@");
    if (pos != -1)
    {
        DefectToFind = "Defect "+DefectToFind+" does not have parent";
        return 1;
    }
    pos = HTML.Find("</title>");
    if (pos == -1) return 2;
    DefectToFind = HTML.Left(pos);
    DefectToFind.Delete(0,DefectToFind.Find("<title>"));
    DefectToFind = DefectToFind.Left(DefectToFind.Find("@"));
    DefectToFind.Delete(0,DefectToFind.ReverseFind(' ')+1);
    return 0;
}

// 0 - child defect(s) found, 1 - no child defect(s)
int TASK::ParseHTMLForChildDefects(const CString &HTML, std::vector<CString> &Tasks)
{
    Tasks.clear();
    long pos = HTML.Find("</a><br />");
    if (pos == -1)
    {
        return 1; // no child defect(s)
    }
    while (pos != -1)
    {
        CString Child = "";
        Child = HTML.Left(pos);
        Child = Child.Right(Child.GetLength()-Child.ReverseFind('>')-1);
        Tasks.push_back(Child);
        pos = HTML.Find("</a><br />",pos+1);
    }
    return 0;
}

// 0 - child task(s) found, 1 - no child task(s)
int TASK::ParseHTMLForChildTasks(const CString &ParentTask, const CString &HTML, std::vector<CString> &Tasks)
{
    Tasks.clear();
    long pos = HTML.Find("No tasks found...<hr size=1>");
    if (pos != -1)
    {
        return 1;
    }
    pos = HTML.Find("<A target=task");
    while (pos != -1)
    {
        CString Child = "";
        Child = HTML.Left(HTML.Find("</a>",pos));
        Child = Child.Right(Child.GetLength()-Child.ReverseFind('>')-1);
        if (ParentTask.CompareNoCase(Child) != 0)
        {
            Tasks.push_back(Child);
        }
        pos = HTML.Find("<A target=task",pos+1);
    }
    return 0;
}
