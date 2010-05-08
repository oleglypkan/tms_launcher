/*
    File name: Task.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: January 5, 2007
*/

#include "stdafx.h"
#include "Task.h"
#include "settings.h"
#include <boost/regex.hpp>
using namespace boost;

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: Task.cpp,v $$Revision: 1.15 $$Date: 2008/03/23 17:13:23Z $"; 
#endif
 
extern CSettings Settings;
bool isalpha_cp1251(char ch);
int CompareNoCaseCP1251(const char *string1, const char *string2);

bool TASK::IsTaskNameValid(const char *OriginalTask, CString &sClientName, CString &Sep, CString &sIDName)
{
//  Task name format: [%CLIENT%-]%ID%[-%EXT%]

    CString sTaskName = OriginalTask;
    if (sTaskName.IsEmpty()) return false;

    CString Ext = "";

    int pos = sTaskName.FindOneOf(Settings.Separators);

    // parsing task name
    if (pos > -1)
    {
        Sep = sTaskName[pos];
        sClientName = sTaskName.Left(pos);
        sIDName = sTaskName.Right(sTaskName.GetLength()-pos-1);
        sIDName.TrimLeft(Settings.Separators);
        pos = sIDName.FindOneOf(Settings.Separators);
        if (pos > -1)
        {
            Ext = sIDName.Right(sIDName.GetLength()-pos-1);
            Ext.TrimLeft(Settings.Separators);
            sIDName.Delete(pos,sIDName.GetLength()-pos);
        }
    }
    else // there are no separators in the task name
        if (sTaskName.GetLength()<=Settings.MaxIDName)
        {
            sClientName = "";
            Sep = "";
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
            for (int i=0; i<sClientName.GetLength()-1; i++)
            {
                if (isdigit((unsigned char)(sClientName[i]))) return false;
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
        for (int i=0; i<sIDName.GetLength(); i++)
        {
            if (!isdigit((unsigned int)sIDName[i])) return false;
        }
    }
    
    if ((Ext.GetLength() < Settings.MinExt) || (Ext.GetLength() > Settings.MaxExt))
    {
        return false;
    }
    return true;
}

// separate items between tasks separators symbols
void TASK::SimpleParseTasks(const char *strTasks, std::vector<CString> &Tasks)
{
    CString sTasks = strTasks;
    Tasks.clear();

    int pos = -1;
    while (!sTasks.IsEmpty())
    {
        pos = sTasks.FindOneOf(Settings.TasksSeparators);
        if (pos == -1)
        {
            pos = sTasks.GetLength();
        }
        CString temp = sTasks.Left(pos);
        temp.TrimLeft(Settings.Separators);
        temp.TrimRight(Settings.Separators);
        Tasks.push_back(temp);
        sTasks.Delete(0,pos);
        sTasks.TrimLeft(Settings.TasksSeparators);
    }
}

// separates items between tasks separators symbols, items are checked for correct task name,
// and saved with separated Client, Separator and ID parts.
// Also defects are additionaly saved to separate array and sorted
// PROJECT is saved instead of Client for defects
bool TASK::ComplexParseTasks(const char *strTasks, std::vector<TASKNAME> &Tasks, std::vector<TASKNAME> &Defects, int &items)
{
    Tasks.clear();
    Defects.clear();

    CString sTasks = strTasks;

    items = 0;
    int pos = -1;
    bool AllTasksValid = true;
    CString Client, Sep, ID, Project;

    while (!sTasks.IsEmpty())
    {
        pos = sTasks.FindOneOf(Settings.TasksSeparators);
        if (pos == -1)
        {
            pos = sTasks.GetLength();
        }

        CString temp = sTasks.Left(pos);
        temp.TrimLeft(Settings.Separators);
        temp.TrimRight(Settings.Separators);
        if (!temp.IsEmpty())
        {
            items++;
            if (IsTaskNameValid(temp,Client,Sep,ID))
            {
                Tasks.push_back(TASKNAME(Client,Sep,ID));
                if (Settings.IsDefect(Client, &Project, NULL))
                {
                    Project.MakeUpper();
                    Defects.push_back(TASKNAME(Project,Sep,ID));
                }
            }
            else
            {
                AllTasksValid = false;
            }
        }
        sTasks.Delete(0,pos);
        sTasks.TrimLeft(Settings.TasksSeparators);
    }
    
    sort_defects(Defects);

    return AllTasksValid;
}

void TASK::sort_defects(std::vector<TASKNAME> &Defects)
{
    for (int i = 0; i < Defects.size() - 1; i++)
    {
        bool exchange = false;
        for (int j = 1; j < Defects.size() - i; j++)
        {
            if (CompareNoCaseCP1251(Defects[j-1].Client,Defects[j].Client) > 0)
            {
                TASKNAME temp = Defects[j-1];
                Defects[j-1] = Defects[j];
                Defects[j] = temp;
                exchange = true;
            }
        }
        if (!exchange) break;
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
int TASK::ParseHTMLForChildTasks(const CString &ParentTask, const CString &HTML, std::vector<CHILD> &Tasks)
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
        // searching for task
        CHILD Child;
        Child.TaskName = "";
        Child.Product = "";
        Child.TaskName = HTML.Left(HTML.Find("</a>",pos));
        Child.TaskName = Child.TaskName.Right(Child.TaskName.GetLength()-Child.TaskName.ReverseFind('>')-1);
        if (ParentTask.CompareNoCase(Child.TaskName) != 0)
        {
            // searching for "Status" of the task
            for (int i = 0; i < 2; i++)
            {
                pos = HTML.Find("<td",pos+1);
            }
            if (pos != -1)
            {
                Child.Status = HTML.Left(HTML.Find("</td>",pos+1));
                Child.Status = Child.Status.Right(Child.Status.GetLength()-Child.Status.ReverseFind('>')-1);
                Child.Status.TrimRight(' ');
            }
            // searching for "Product" of the task
            pos = HTML.Find("<td",pos+1);
            if (pos != -1)
            {
                Child.Product = HTML.Left(HTML.Find("</td>",pos+1));
                Child.Product = Child.Product.Right(Child.Product.GetLength()-Child.Product.ReverseFind('>')-1);
            }
            // adding the task
            Tasks.push_back(Child);
        }

        // searching for next task
        pos = HTML.Find("<A target=task",pos+1);
    }
    return 0;
}

void TASK::ParseHTMLForActions(const CString &HTML, std::vector<CString> &TaskActions, bool QB)
{
    TaskActions.clear();
    
    std::vector<std::string> RawActions;
    RawActions.clear();

    RegEx expr; // regular expression object to be used to parse HTML

    if (QB) // QB or QR actions
    {
        try
        {
            expr.SetExpression(Settings.QbRegEx,true);
        }
        catch (bad_expression)
        {
            return;
        }
        expr.Grep(RawActions,HTML,match_any);
        for (long i = 0; i < RawActions.size(); i++)
        {
            CString Action = RawActions[i].c_str();
            Action = Action.Left(Action.ReverseFind('Q'));
            FormatActionOutput(Action,QB);
            TaskActions.push_back(Action);
        }
    }
    else // QC actions
    {
        try
        {
            expr.SetExpression(Settings.QcRegEx,true);
        }
        catch (bad_expression)
        {
            return;
        }
        expr.Grep(RawActions,HTML,match_any);
        for (long i = 0; i < RawActions.size(); i++)
        {
            CString Action = RawActions[i].c_str();
            CString Requirements = "";
            ParseActionForRequirements(Action,Requirements);
            FormatActionOutput(Action,QB);
            Action += Requirements;
            TaskActions.push_back(Action);
        }
    }
}

void TASK::ParseHTMLForAA_ID(const CString &HTML, CString &AA_ID)
{
    if (HTML.IsEmpty())
    {
        AA_ID = "";
        return;
    }

    std::vector<std::string> AA_IDs;
    AA_IDs.clear();
   
    RegEx expr;
    try
    {
        expr.SetExpression(Settings.AA_ID_RegEx,true);
    }
    catch (bad_expression)
    {
        AA_ID = "";
        return;
    }
    expr.Grep(AA_IDs,HTML,match_stop);

    if (AA_IDs[0].empty())
    {
        AA_ID = "";
    }
    else
    {
        AA_ID = AA_IDs[0].c_str();
        AA_ID.Replace("AA_ID=","");
    }
}

void TASK::ParseHTMLForTimesheets(const CString &HTML, std::vector<CString> &Timesheets, const CString &mask, bool &filtered)
{
    Timesheets.clear();
    filtered = false;
    
    std::vector<std::string> v;
    v.clear();
    
    RegEx expr;
    try
    {
        expr.SetExpression(Settings.TimesheetsRegEx,true);
    }
    catch (bad_expression)
    {
        return;
    }
    expr.Grep(v,HTML,match_any);

    for (int i = 0; i < v.size(); i++)
    {
        std::vector<std::string> v2;
        v2.clear();
        std::vector<CString> Result;
        Result.clear();
        CString part = "";

        expr.SetExpression(">[^<]+<",true);
        expr.Grep(v2,v[i].c_str(),match_any);

        for (int j = 0; j < v2.size(); j++)
        {
            part = v2[j].c_str();
            part.Remove('>');
            part.Remove('<');
            if (part.Find("&nbsp") != -1)
            {
                part = "";
            }
            Result.push_back(part);
        }
        if (Result.size() > 3)
        {
            part.Format("%-16s%-18s%-12s%s",Result[0],Result[1],Result[2],Result[3]);
            if (!mask.IsEmpty())
            {
                expr.SetExpression(mask,true);
                if (!expr.Search(Result[1],match_any))
                {
                    filtered = true;
                    continue;
                }
            }
            Timesheets.push_back(part);
        }
    }
}

void TASK::FormatActionOutput(CString &Action, bool QB)
{
    std::vector<std::string> v;
    v.clear();

    RegEx expr;
    expr.SetExpression(">[^<]+<",true);
    expr.Grep(v,Action,match_any);

    std::vector<CString> v2;
    v2.clear();
    
    int components = 0;
    (v.size() > 4) ? components = 5 : components = v.size();
    for (int i= 0; i < components; i++)
    {
        CString temp = v[i].c_str();
        temp.Remove('>');
        temp.Remove('<');
        v2.push_back(temp);
    }
    
    if (v2.size() > 4)
    {
        Action.Format("%-4s%-12s%-7s%-18s",v2[0],v2[1],v2[2],v2[3]);
        if (QB)
        {
            Action += v2[4];
        }
    }
}

void TASK::ParseActionForRequirements(const CString &Action, CString &Output)
{
    Output = "-------";
    
    RegEx expr;
    try
    {
        expr.SetExpression(Settings.RtmRegEx,true);
    }
    catch (bad_expression)
    {
        return;
    }
    
    if (expr.Match(Action,match_any))
    {
        Output = expr[2].c_str();
        RemoveSpaces(Output);
        if (Output.IsEmpty())
        {
            Output = "-------";
        }
    }
}

void TASK::RemoveSpaces(CString &s)
{
    if (s.IsEmpty()) return;

    s.TrimLeft("}>]):");
    s.TrimLeft();
    s.TrimRight();
    s.Replace("<br>","");
    s.Replace("<br/>","");
    s.Replace("&amp;","&");
    long pos = s.Find("  ");
    while (pos != -1)
    {
        s.Replace("  "," ");
        pos = s.Find("  ",pos);
    }
}