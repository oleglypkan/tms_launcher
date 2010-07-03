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
 static char verid[]="@(#)$RCSfile: Task.cpp,v $$Revision: 1.25 $$Date: 2009/03/25 21:38:47Z $"; 
#endif

#ifdef _DEBUG
#include <crtdbg.h>
#include <stdlib.h>
#define _CRTDBG_MAP_ALLOC 
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

extern CSettings Settings;
bool isalpha_cp1251(unsigned char ch);
int CompareNoCaseCP1251(const char *string1, const char *string2);

void TASK::FillupTaskID(CString &ID)
{
    if (lstrlen(ID) < 5)
    {
        CString tempID = "00000";
        tempID += ID;
        tempID.Delete(0,tempID.GetLength()-5);
        ID = tempID;
    }
}

bool TASK::IsClientNameValid(const CString &ClientName)
{
    // checking for correct Client name
    // the first character in Client Name can not be a digit, the rest ones can be either alpha characters, '_' or numbers
    // [_A-Za-z]
    if (isdigit((unsigned char)(ClientName[0])))
    {
        return false;
    }
    // [_A-Za-z0-9]
    for (int i=0; i<ClientName.GetLength(); i++)
    {
        if (((unsigned char)(ClientName[i]) != '_') && (!isalpha_cp1251((unsigned char)(ClientName[i]))) && (!isdigit((unsigned char)(ClientName[i]))))
        {
            return false;
        }
    }
    return true;
}

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
    {
        if (!sClientName.IsEmpty())
        {
            if (!IsClientNameValid(sClientName)) return false;
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
            if (!isdigit((unsigned char)(sIDName[i]))) return false;
        }
    }
    
    if ((Ext.GetLength() < Settings.MinExt) || (Ext.GetLength() > Settings.MaxExt))
    {
        return false;
    }
    return true;
}

// separate items between tasks separators characters
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

// separates items between tasks separators characters, items are checked for correct task name,
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
    for (unsigned int i = 0; i < Defects.size() - 1; i++)
    {
        bool exchange = false;
        for (unsigned int j = 1; j < Defects.size() - i; j++)
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

// 0 - parent found, 1 - no parent, 2 - no such task
int TASK::ParseHTMLForParentTask(CString &TaskToFind, const CString &HTML)
{
    long pos = HTML.Find("Task "+TaskToFind+" does not exist");
    if (pos != -1)
    {
        TaskToFind = "Task "+TaskToFind+" does not exist";
        return 2;
    }
    RegEx expr; // regular expression object to be used to parse HTML
    // check if the task does not have parent
    try
    {
        expr.SetExpression("TMS Parent:.+____ ____",true);
    }
    catch (bad_expression)
    {
        TaskToFind = "Cannot find parent task for "+TaskToFind;
        return 1;
    }
    if (expr.Search(HTML,match_any))
    {
        TaskToFind = "Task "+TaskToFind+" does not have parent task";
        return 1;
    }
    // searching for parent task
    try
    {
        expr.SetExpression("TMS Parent:.+<A.+>([^[:space:]]+)</a>",true); // sub-expression ([^[:space:]]+) should contain parent task name)
    }
    catch (bad_expression)
    {
        TaskToFind = "Cannot find parent task for "+TaskToFind;
        return 1;
    }
    if (expr.Search(HTML,match_any)) // match_any - non greedy search
    {
        if (expr.Matched(1)) // first sub-expression ([^[:space:]]+) matched (it contains parent task name)
        {
            TaskToFind = expr.What(1).c_str();
            return 0;
        }
    }
    TaskToFind = "Cannot find parent task for "+TaskToFind;
    return 1;
}

 // 0 - parent found, 1 - no parent, 2 - no such defect
int TASK::ParseHTMLForParentDefect(CString &DefectToFind, const CString &HTML)
{
    RegEx expr; // regular expression object used to parse HTML page
    try
    {
        expr.SetExpression("<title>.+ / SoftTest / Defect Info / @",true);      
    }
    catch (bad_expression)
    {
        return 2;
    }
    if (expr.Search(HTML,match_any))
    {
        DefectToFind = "Defect "+DefectToFind+" not found";
        return 2;
    }
    try
    {
        expr.SetExpression("<title>.+ / SoftTest / Defect Info / 0@",true);      
    }
    catch (bad_expression)
    {
        return 2;
    }
    if (expr.Search(HTML,match_any))
    {
        DefectToFind = "Defect "+DefectToFind+" does not have parent";
        return 1;
    }
    long pos = HTML.Find("</title>");
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

    // check if task does not have child tasks
    long pos = HTML.Find("id=\"kids\"");
    if (pos == -1)
    {
        return 1;
    }

    RegEx expr; // regular expression object used to find tasks in HTML page
    try
    {
        expr.SetExpression("\"><A title='[ ]*Task:[^&]*&#13;CreateDate.+</td><td colspan=",true);      
    }
    catch (bad_expression)
    {
        return 1;
    }
    std::vector<std::string> temp; // to keep found tasks
    temp.clear();
    expr.Grep(temp,HTML,match_any);

    try
    {
        // regular expression used to find task's details (name, product, status)
        expr.SetExpression("\"><A title='[ ]*Task:[^&]*&#13;.+Product:(.+)&#13;Status:(.+)&#13;.+'>(.+)</a>.+</td><td colspan=",true);
    }
    catch (bad_expression)
    {
        return 1;
    }
    for (unsigned long i = 0; i < temp.size(); i++)
    {
        CString task = temp[i].c_str();
        if (expr.Search(task,match_any))
        {
            CHILD Child;
            Child.TaskName = expr.Matched(3) ? expr.What(3).c_str() : "";
            Child.TaskName.TrimLeft();
            Child.TaskName.TrimRight();
            if (Child.TaskName.IsEmpty())
            {
                continue;
            }
            Child.Product = expr.Matched(1) ? expr.What(1).c_str() : ""; 
            Child.Product.TrimLeft();
            Child.Product.TrimRight();

            Child.Status = expr.Matched(2) ? expr.What(2).c_str() : ""; 
            Child.Status.TrimLeft();
            Child.Status.TrimRight();
            
            Tasks.push_back(Child);
        }
    }
    if (Tasks.size()>0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

// 0 - SPC linked task(s) found, 1 - no SPC linked task(s)
int TASK::ParseHTMLForSPCtasks(const CString &HTML, std::vector<CHILD> &Tasks)
{
    Tasks.clear();

    RegEx expr; // regular expression object used to find tasks in HTML page
    try
    {
        expr.SetExpression("(<ul type=disc>)*<li><div.+(&nbsp;?){5,}.+</td></tr>",true);
    }
    catch (bad_expression)
    {
        return 1;
    }
    std::vector<std::string> temp; // to keep found tasks
    temp.clear();
    expr.Grep(temp,HTML,match_any);

    try
    {
        // regular expression object used to find task's details (level, name, product, status, MSP)
        expr.SetExpression("((?:<ul type=disc>)*)<li><div.+\"><b>(.+)</b></a>.+</td><td (?:bgcolor=\"#[0-9A-F]+\" )?nowrap >([^<]+)</td><td (?:bgcolor=\"#[0-9A-F]+\" )?nowrap >([^<]+)</td><td (?:bgcolor=\"#[0-9A-F]+\" )? nowrap align.+(?:&nbsp;?){5,}(.+)</td></tr>",true);
    }
    catch (bad_expression)
    {
        return 1;
    }
    for (unsigned long i = 0; i < temp.size(); i++)
    {
        CString task = temp[i].c_str();
        if (expr.Search(task,match_any))
        {
            CHILD Child;
            Child.TaskName = expr.Matched(2) ? expr.What(2).c_str() : "";
            Child.TaskName.TrimLeft();
            Child.TaskName.TrimRight();
            if (Child.TaskName.IsEmpty())
            {
                continue;
            }
            CString level = expr.Matched(1) ? expr.What(1).c_str() : "";
            Child.level = 0;
            int pos = 0;
            while ((pos = level.Find("<ul type=disc>")) != -1)
            {
                level.Delete(pos,lstrlen("<ul type=disc>"));
                Child.level++;
            }
            if (Child.level == 0)
            {
                Child.level = 1;
            }
            Child.Product = expr.Matched(3) ? expr.What(3).c_str() : "";
            Child.Product.TrimLeft();
            Child.Product.TrimRight();
            Child.Status = expr.Matched(4) ? expr.What(4).c_str() : "";
            Child.Status.TrimLeft();
            Child.Status.TrimRight();
            Child.MSP = expr.Matched(5) ? expr.What(5).c_str() : "";
            Child.MSP.TrimLeft();
            Child.MSP.TrimRight();
            Tasks.push_back(Child);
        }
    }
    if (Tasks.size()>0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void TASK::ParseHTMLForActions(const CString &HTML, std::vector<CString> &TaskActions, bool QB)
{
    TaskActions.clear();
    
    std::vector<std::string> RawActions;
    RawActions.clear();

    RegEx expr; // regular expression object to be used to parse HTML

    try
    {
        if (QB)
        {
            expr.SetExpression(Settings.ActHeaderRegEx,true);
        }
        else
        {
            expr.SetExpression(Settings.ActHeaderRegEx+Settings.ActBodyRegEx,true);
        }
    }
    catch (bad_expression)
    {
        return;
    }
    expr.Grep(RawActions,HTML,match_any);

    try
    {
        if (QB)
        {
            expr.SetExpression("<td[^>]*>([0-9]+)</td>[[:space:]]*<td[^>]*>(<a[^>]*>.+</a>|[NY])</td>[[:space:]]*<td[^>]*>([^<]+)</td>[[:space:]]*<td[^>]*>([^<]+)</td>[[:space:]]*<td[^>]*>[[:space:]]*<a[^>]*>(.+</a>.*)</td>[[:space:]]*<td[^>]*>(.+)</td>[[:space:]]*<td[^>]*>[[:space:]]*<a[^>]*>(.+</a>.*)</td>",true);
        }
        else
        {
            expr.SetExpression("<td[^>]*>([0-9]+)</td>[[:space:]]*<td[^>]*>(<a[^>]*>.+</a>|[NY])</td>[[:space:]]*<td[^>]*>([^<]+)</td>[[:space:]]*<td[^>]*>([^<]+)</td>[[:space:]]*<td[^>]*>[[:space:]]*<a[^>]*>(.+</a>.*)</td>[[:space:]]*<td[^>]*>(.+)</td>[[:space:]]*<td[^>]*>[[:space:]]*<a[^>]*>(.+</a>.*)</td>[[:space:]]*</tr>"+Settings.ActBodyRegEx,true);
        }
    }
    catch (bad_expression)
    {
        return;
    }
    for (unsigned long i = 0; i < RawActions.size(); i++)
    {
        CString Action = RawActions[i].c_str();
        if (expr.Search(Action,match_any))
        {
            CString tmp = expr.Matched(6) ? expr.What(6).c_str() : ""; // action code
            tmp.TrimLeft(); tmp.TrimRight();
            CString Requirements = "";
            if (QB)
            {
                if ((tmp.CompareNoCase("QB")!=0) && (tmp.CompareNoCase("QR")!=0)) continue;
            }
            else
            {
                if (tmp.CompareNoCase("QC")!=0) continue;
                tmp = expr.Matched(8) ? expr.What(8).c_str() : ""; // QC action body
                ParseActionForRequirements(tmp,Requirements);
            }
            CString Output = "", Result = "";
            tmp = expr.Matched(1) ? expr.What(1).c_str() : ""; // action #
            tmp.TrimLeft(); tmp.TrimRight();
            Result.Format("%-4s", tmp);
            tmp = expr.Matched(2) ? expr.What(2).c_str() : ""; // view action (Y/N)
            tmp = expr.Matched(3) ? expr.What(3).c_str() : ""; // action date
            tmp.TrimLeft(); tmp.TrimRight();
            Output.Format("%-12s", tmp);
            Result += Output;
            tmp = expr.Matched(4) ? expr.What(4).c_str() : ""; // action time
            tmp.TrimLeft(); tmp.TrimRight();
            Output.Format("%-7s", tmp);
            Result += Output;
            tmp = expr.Matched(5) ? expr.What(5).c_str() : ""; // action performer
            tmp.Replace("</a>",""); tmp.Replace("(",""); tmp.Replace(")","");
            tmp.TrimLeft(); tmp.TrimRight();
            Output.Format("%-18s", tmp);
            Result += Output;
            if (QB)
            {
                tmp = expr.Matched(7) ? expr.What(7).c_str() : ""; // assigned to
                tmp.Replace("</a>",""); tmp.Replace("(",""); tmp.Replace(")","");
                tmp.TrimLeft(); tmp.TrimRight();
                Result += tmp;
            }
            else
            {
                Result += Requirements;
            }
            TaskActions.push_back(Result);
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
    if (expr.Search(HTML,match_stop) && expr.Matched(1))
    // match_stop - greedy search but it stops after the first match is found
    // first sub-expression matched (it contains AA_ID number)
    {
        AA_ID = expr.What(1).c_str();
        return;
    }
    AA_ID = "";
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
        expr.SetExpression(Settings.iTMSTimesheetsRegEx,true);
    }
    catch (bad_expression)
    {
        return;
    }
    expr.Grep(v,HTML,match_any);

    for (unsigned int i = 0; i < v.size(); i++)
    {
        if (expr.Search(v[i].c_str(),match_any))
        {
            CString tmp = expr.Matched(2) ? expr.What(2).c_str() : ""; // Employee
            if (tmp.IsEmpty()) continue;
            if (FilterTask(tmp, mask)) // filtering by Employee
            {
                filtered = true;
                continue;
            }
            tmp.Format("%-16s%-18s%-12s%s", expr.Matched(1) ? expr.What(1).c_str() : "",  // Date
                                            expr.What(2).c_str(),                         // Employee
                                            expr.Matched(3) ? expr.What(3).c_str() : "",  // Time
                                            expr.Matched(4) ? expr.What(4).c_str() : ""); // Comment
            tmp.Replace("&nbsp","");
            tmp.TrimRight();
            Timesheets.insert(Timesheets.begin(),tmp);
        }
    }
}

void TASK::ParseActionForRequirements(const CString &Action, CString &Output)
{
    Output = "-------";
    
    RegEx expr;
    try
    {
        expr.SetExpression(Settings.iTMSRtmRegEx,true);
    }
    catch (bad_expression)
    {
        return;
    }
    
    if (expr.Search(Action,match_any))
    {
        Output = expr.Matched(2) ? expr.What(2).c_str() : "";
        RemoveSpaces(Output);
        if (Output.IsEmpty())
        {
            Output = "-------";
        }
    }
}

// true  - task should NOT be printed
// false - task should be printed
bool TASK::FilterTask(const CString &string, const CString &mask)
{
    if (mask.IsEmpty()) return false;
    
    // additional functionality to allow "!" to be used in regular expressions meaning NOT match
    CString temp = mask;
    bool exclude = false;

    if (temp[0] == '!')
    {
        temp.Delete(0);
        if (!temp.IsEmpty()) 
        {
            exclude = true;
        }
        else
        {
            return false;
        }
    }

    // filter that allows to use regular expressions
    RegEx expr;
    try
    {
        expr.SetExpression(temp,true);
    }
    catch (bad_expression)
    {
        return false;
    }

    if (expr.Search(string,match_any)) // string matches temp
    {
        return exclude;
    }
    else
    {
        return !exclude;
    }
}

void TASK::RemoveSpaces(CString &s)
{
    if (s.IsEmpty()) return;

    s.Replace("&nbsp;"," ");
    s.Replace("&nbsp"," ");
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