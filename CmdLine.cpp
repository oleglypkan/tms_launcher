/*
    File name: CmdLine.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
*/

#include "stdafx.h"
#include "CmdLine.h"
#include "settings.h"
#include "MainDlg.h"
#include "Task.h"
#include "AmHttpSocket.h"
#include <boost/cregex.hpp>
#include <boost/regex/pattern_except.hpp>
using namespace boost;
using namespace boost::regex_constants;

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: CmdLine.cpp,v $$Revision: 1.32 $$Date: 2009/04/09 14:54:26Z $"; 
#endif

#ifdef _DEBUG
#include <crtdbg.h>
#include <stdlib.h>
#define _CRTDBG_MAP_ALLOC 
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

extern CSettings Settings;
extern CString szWinName;
int CompareNoCaseCP1251(const char *string1, const char *string2);

CmdLine::CmdLine()
{
    Parameters[0] = ""; // "-c/p/qb/qc/t/spc/inf/exp"
    Parameters[1] = ""; // "-name"
    Parameters[2] = ""; // "-prod"
    Parameters[3] = ""; // "-stat"
    Parameters[4] = ""; // "-msp"
    Parameters[5] = ""; // "-in"
    Parameters[6] = ""; // "-out"
}

void CmdLine::StringToArgv(const char *CommandLine, std::vector<CString> &params)
{
    CString Line = CommandLine;

    params.clear();
    for (;;)
    {
        int pos = Line.Find(' '); // search for a separator between parameters
        if (pos == -1) // separator is not found (there is either 0 or 1 parameter)
        {
            if (Line.GetLength() > 0)
            {
                Line.TrimLeft('\"');
                Line.TrimRight('\"');
                params.push_back(Line);
                Line = "";
            }
            else
            {
                return;
            }
        }
        else // separator is found
        {
             // checking if separator is inside double quotes
            int start = Line.Find('\"');
            if ((start < pos) && (start > -1)) // separator is after opening double quote
            {
                // searching for closing double quote
                int end = Line.Find('\"',start+1);
                if (end == -1) return;
                CString temp = "";
                temp = Line.Left(end+1);
                temp.TrimLeft('\"');
                temp.TrimRight('\"');
                params.push_back(temp);
                Line.Delete(0,end+1);
                Line.TrimLeft(' ');
            }
            else // there are no double quotes before separator
            {
                params.push_back(Line.Left(pos));
                Line.Delete(0,pos+1);
                Line.TrimLeft(' ');
            }
        }
    }
}

void CmdLine::ShowUsage()
{
    CString Usage = "The following command line parameters are correct:\n\n\
TMS_Launcher -p [-name \"PCC\"] in out\n\
    - get the list of parent tasks for the given ones\n\n\
TMS_Launcher -c [-name \"TCD\"] [-prod \".X\"] [-stat \"C1\"] in out\n\
    - get the list of child tasks for the given ones\n\n\
TMS_Launcher -spc [-name \"PCC\"] [-prod \".X\"] [-stat \"C1\"] [-msp \"LABG4.0.4.5\"] in out\n\
    - get the list of SPC linked tasks with the given ones\n\n\
TMS_Launcher -qb in out\n\
    - get the list of QB actions for the given tasks\n\n\
TMS_Launcher -qc in out\n\
    - get the list of QC actions for the given tasks\n\n\
TMS_Launcher -t [-name \"OLEG L\"] in out\n\
    - get timesheets for the given tasks\n\n\
TMS_Launcher -inf [-name \"PCC\"] [-prod \".X\"] [-stat \"C1\"] [-msp \"LABG4.0.4.5\"] in out\n\
    - get information (product, status, MSP) for the given tasks\n\n\
TMS_Launcher -exp \"expression\" in out\n\
    - get timesheets for the given tasks";
    MessageBox(NULL, Usage, szWinName, MB_OK | MB_ICONINFORMATION);
}

void CmdLine::ParseCmdLine(const char *CommandLine)
{
    std::vector<CString> argv;
    StringToArgv(CommandLine,argv);

    for (unsigned int i = 0; i < argv.size(); i++)
    {
        if (argv[i].CompareNoCase("-c") == 0)
        {
            Parameters[0] = "c";
            continue;
        }
        if (argv[i].CompareNoCase("-p") == 0)
        {
            Parameters[0] = "p";
            continue;
        }
        if (argv[i].CompareNoCase("-qb") == 0)
        {
            Parameters[0] = "b";
            continue;
        }
        if (argv[i].CompareNoCase("-qc") == 0)
        {
            Parameters[0] = "q";
            continue;
        }
        if (argv[i].CompareNoCase("-t") == 0)
        {
            Parameters[0] = "t";
            continue;
        }
        if (argv[i].CompareNoCase("-spc") == 0)
        {
            Parameters[0] = "s";
            continue;
        }
        if (argv[i].CompareNoCase("-inf") == 0)
        {
            Parameters[0] = "i";
            continue;
        }
        if (argv[i].CompareNoCase("-exp") == 0)
        {
            if (argv.size() > i+1)
            {
                Parameters[0] = "e";
                Parameters[1] = argv[i+1];
                i++;
                continue;
            }
            else
            {
                ShowUsage();
                return;
            }
        }
        if (argv[i].CompareNoCase("-name") == 0)
        {
            if (argv.size() > i+1)
            {
                Parameters[1] = argv[i+1];
                i++;
            }
            continue;
        }
        if (argv[i].CompareNoCase("-prod") == 0)
        {
            if (argv.size() > i+1)
            {
                Parameters[2] = argv[i+1];
                i++;
            }
            continue;
        }
        if (argv[i].CompareNoCase("-stat") == 0)
        {
            if (argv.size() > i+1)
            {
                Parameters[3] = argv[i+1];
                i++;
            }
            continue;
        }
        if (argv[i].CompareNoCase("-msp") == 0)
        {
            if (argv.size() > i+1)
            {
                Parameters[4] = argv[i+1];
                i++;
            }
            continue;
        }
        if (Parameters[5].IsEmpty())
        {
            Parameters[5] = argv[i];
        }
        else
        {
            if (Parameters[6].IsEmpty())
            {
                Parameters[6] = argv[i];
            }
        }
    }

    // checking for correct parameters
    if (Parameters[0].IsEmpty() || Parameters[5].IsEmpty() || Parameters[6].IsEmpty())
    {
        ShowUsage();
        return;
    }
    for (int i = 1; i <= 4; i++)
    {
        if (!Parameters[i].IsEmpty())
        {
            RegEx expr;
            try
            {
                expr.SetExpression(Parameters[i],true);
            }
            catch (bad_expression)
            {
                CString Message = "", IncorrectParam = "";
                switch(i)
                {
                    case 1:
                        IncorrectParam = "-name/-exp";
                        break;
                    case 2:
                        IncorrectParam = "-prod";
                        break;
                    case 3:
                        IncorrectParam = "-stat";
                        break;
                    case 4:
                        IncorrectParam = "-msp";
                        break;
                }
                Message.Format("Regular expression passed with %s parameter is incorrect", IncorrectParam);
                MessageBox(NULL, Message, szWinName, MB_OK | MB_ICONERROR);
                return;
            }
        }
    }

    switch (Parameters[0][0])
    {
        case 'p': // parent
        case 'c': // child
        case 's': // SPC linked tasks
        case 'b': // qb
        case 'q': // qc
        case 't': // timesheets
        case 'i': // information about task
        case 'e': // check if task matches regular expression
            ProcessInputFile(Parameters[5],Parameters[6],Parameters[0][0]);
            break;
    }
}

void CmdLine::GetTimesheetsFromTasks(TASK &task, const CString &Message, const char *OriginalTask, ofstream &OutFile)
{
    std::vector<CString> Timesheets;
    bool filtered = false;
    task.ParseHTMLForTimesheets(Message,Timesheets,Parameters[1],filtered);
    if (Timesheets.size() == 0)
    {
        OutFile << OriginalTask;
        if (!filtered)
        {
            OutFile << "-----------";
        }
        OutFile << endl << endl;
        return;
    }
    OutFile << OriginalTask;
    OutFile << "Date            Employee          Time        Comments" << endl;

    for (unsigned int j = 0; j < Timesheets.size(); j++)
    {
        OutFile.width(16);
        OutFile.setf(OutFile.left);
        OutFile << " " << Timesheets[j] << endl;
    }
    OutFile << endl;
}

void CmdLine::PrintSubExpressions(const CString &Message, const char *OriginalTask, ofstream &OutFile)
{
    if (Parameters[1].IsEmpty()) return;

    // additional functionality to allow "!" to be used in regular expressions meaning NOT match
    CString temp = Parameters[1];
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
            return;
        }
    }

    // filter that allows to use regular expressions
    RegEx expr;
    expr.SetExpression(temp,true);
    if (!expr.Search(Message,match_any))
    {
        exclude = !exclude;
    }

    if (!exclude)
    {
        OutFile << OriginalTask;
        for (unsigned long i = 1; i < expr.Marks(); i++)
        {
            CString line = expr.Matched(i) ? expr.What(i).c_str() : "";
            OutFile << line << " ";
            if ((i % 2) == 0) OutFile << "    ";
        }
        OutFile << endl;
    }
}

void CmdLine::GetInformationAboutTask(const CString &Message, const char *OriginalTask, ofstream &OutFile, const CString &Client, const CString &ID)
{
    RegEx expr; // regular expression object to be used to parse HTML
    try
    {
        expr.SetExpression(": <a title=\"View Task\".+<b>"+Client+"-"+ID+"</b></a>.+\"Add Task To Timesheet\".+(<td[^>]*>[^<]*</td>){4}<td[^>]*>([^<]+)</td><td[^>]*>([^<]+)</td>(<td[^>]*>[^<]*</td>){3}<td[^>]*>((&nbsp;?){5}[^<]*)</td>",true);
    }
    catch (bad_expression)
    {
        return;
    }
    if (expr.Search(Message,match_any))
    {
        CHILD Task;
        Task.Product = expr.Matched(2) ? expr.What(2).c_str() : "";
        Task.Product.TrimLeft();
        Task.Product.TrimRight();

        Task.Status = expr.Matched(3) ? expr.What(3).c_str() : "";
        Task.Status.TrimLeft();
        Task.Status.TrimRight();

        Task.MSP = expr.Matched(5) ? expr.What(5).c_str() : "";
        Task.MSP.Replace("&nbsp;","");
        Task.MSP.Replace("&nbsp","");
        Task.MSP.TrimLeft();
        Task.MSP.TrimRight();

        // task filtering
        if (!TASK::FilterTask(Client+"-"+ID,Parameters[1])) // filtering by task name
        {
            if (!TASK::FilterTask(Task.Product,Parameters[2])) // filtering by product
            {
                if (!TASK::FilterTask(Task.Status,Parameters[3])) // filtering by status
                {
                    if (!TASK::FilterTask(Task.MSP,Parameters[4])) // filtering by MSP
                    {
                        // task printing
                        CString line = "";
                        line.Format("%-16s%-24s%-8s%s", OriginalTask, Task.Product, Task.Status, Task.MSP);
                        OutFile << line << endl;
                    }
                }
            }
        }
    }
    else
    {
        OutFile << OriginalTask << "- cannot get information about task" << endl;
    }
}

void CmdLine::GetActionsFromTasks(TASK &task, const CString &Message, const char *OriginalTask, ofstream &OutFile, bool QB)
{
    std::vector<CString> TaskActions;
    task.ParseHTMLForActions(Message,TaskActions,QB);
    if (TaskActions.size() == 0)
    {
        OutFile << OriginalTask << "-----------" << endl << endl;
        return;
    }
    OutFile << OriginalTask;
    if (QB)
    {
        OutFile << "#   Date        Time   Person            Assigned to" << endl;
    }
    else
    {
        OutFile << "#   Date        Time   Person            Requirement(s)" << endl;
    }
    for (unsigned int j = 0; j < TaskActions.size(); j++)
    {
        OutFile.width(16);
        OutFile.setf(OutFile.left);
        OutFile << " " << TaskActions[j] << endl;
    }
    OutFile << endl;
}

void CmdLine::GetParentTasksList(TASK &task, const CString &Message, const char *OriginalTask, ofstream &OutFile, CString &Client, bool IsDefect)
{                        
    CString ParentTask = OriginalTask;

    if (IsDefect) // get parent for SoftTest defect
    {
        switch (task.ParseHTMLForParentDefect(ParentTask, Message))
        {
            case 0:
                if (!Client.IsEmpty()) Client += "-";
                OutFile << OriginalTask << Client+ParentTask << endl;
                break;
            case 1:
            case 2:
                if (ParentTask.Find("does not have parent") != -1)
                {
                    OutFile << OriginalTask << "-----------" << endl;
                }
                else
                {
                    OutFile << ParentTask << endl;
                }
                break;
        }
    }
    else // get parent task for given TMS task
    {
        switch (task.ParseHTMLForParentTask(ParentTask, Message))
        {
            case 0:
                if (!task.FilterTask(ParentTask,Parameters[1]))
                {
                    OutFile << OriginalTask << ParentTask << endl;
                }
                else
                {
                    OutFile << OriginalTask << endl;
                }
                break;
            case 1:
            case 2:
                if (ParentTask.Find("does not have parent") != -1)
                {
                    OutFile << OriginalTask << "-----------" << endl;
                }
                else
                {
                    OutFile << ParentTask << endl;
                }
                break;
        }
    }
}

void CmdLine::GetChildTasksList(TASK &task, const CString &Message, const char *OriginalTask, ofstream &OutFile, CString &Client, const CString &ID, bool IsDefect, const char type)
{                        
    CString ParentTask = OriginalTask;

    if (IsDefect) // get children for SoftTest defect
    {
        std::vector<CString> ChildTasks;
        switch(task.ParseHTMLForChildDefects(Message,ChildTasks))
        {
            case 0:
                {
                    if (!Client.IsEmpty()) Client += "-";
                    OutFile << OriginalTask;
                    for (unsigned int j = 0; j < ChildTasks.size(); j++)
                    {
                        if (j != 0)
                        {
                            OutFile.width(16);
                            OutFile.setf(OutFile.left);
                            OutFile << " ";
                        }
                        OutFile << Client+ChildTasks[j] << endl;
                    }
                    OutFile << endl;
                }
                break;
            case 1:
                OutFile << OriginalTask << "-----------" << endl;
                OutFile << endl;
                break;
        }
    }
    else // get child/SPC linked tasks for given TMS task
    {
        std::vector<CHILD> ChildTasks;
        int result = -1;
        if (type == 'c') // find child tasks
        {
            result = task.ParseHTMLForChildTasks(Client+"-"+ID,Message,ChildTasks);
        }
        else // find SPC linked tasks
        {
            result = task.ParseHTMLForSPCtasks(Message,ChildTasks);
        }
        switch(result)
        {
            case 0:
                {
                    OutFile << OriginalTask;
                    unsigned int printed = 0;
                    for (unsigned int j = 0; j < ChildTasks.size(); j++)
                    {
                        if (!task.FilterTask(ChildTasks[j].TaskName,Parameters[1])) // filtering by task name
                        {
                            if (!task.FilterTask(ChildTasks[j].Product,Parameters[2])) // filtering by product
                            {
                                if (!task.FilterTask(ChildTasks[j].Status,Parameters[3])) // filtering by status
                                {
                                    if (type == 'c')
                                    {
                                        PrintTask(OutFile, ChildTasks[j], (printed==0));
                                        printed++;
                                    }
                                    else
                                    {
                                        if (!task.FilterTask(ChildTasks[j].MSP,Parameters[4])) // filtering by MSP
                                        {
                                            PrintSPCtask(OutFile, ChildTasks[j], (ChildTasks[j].TaskName.CompareNoCase(Client+"-"+ID) == 0), (printed==0));
                                            printed++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    OutFile << endl;
                    if (printed == 0)
                    {
                        OutFile << endl;
                    }
                }
                break;
            case 1:
                OutFile << OriginalTask << "-----------" << endl;
                OutFile << endl;
                break;
        }
    }
}

void CmdLine::ProcessInputFile(const char *InputFileName, const char *OutputFileName, const char type)
{
    int index = Settings.GetDefaultUrlIndex();
    if (index == -1) return;

    ifstream InFile;
    InFile.open(InputFileName, ios::in);
    if (InFile.fail())
    {
        CString Message = "";
        Message.Format("Input file \"%s\" does not exist. Please specify existing one", InputFileName);
        MessageBox(NULL, Message, szWinName, MB_OK | MB_ICONERROR);
        return;
    }

    ofstream OutFile;
    OutFile.open(OutputFileName, ios::out);
    if (OutFile.fail())
    {
        CString Message = "";
        Message.Format("Output file \"%s\" is read only or disk is write protected", OutputFileName);
        MessageBox(NULL, Message, szWinName, MB_OK | MB_ICONERROR);
        InFile.close();
        return;
    }

    if (Settings.links[index].Login.IsEmpty() || Settings.links[index].Password.IsEmpty())
    {
        OutFile << "Both Login and Password must be specified in settings window for \""+Settings.links[index].Caption+"\" URL";
        OutFile.close();
        InFile.close();
        return;
    }

    CAmHttpSocket Req;

    // Print request specific title
    switch (type)
    {
        case 'p': // parent task
            OutFile << "Task            Parent task" << endl;
            OutFile << "---------------------------" << endl;
            break;
        case 'c': // child tasks
            OutFile << "Task            Child tasks     Product                 Status" << endl;
            OutFile << "--------------------------------------------------------------" << endl;
            break;
        case 's': // SPC linked tasks
            OutFile << "Task             Linked tasks            Product                 Status  MSP" << endl;
            OutFile << "------------------------------------------------------------------------------------" << endl;
            break;
        case 'b': // qb actions
            OutFile << "Task            QB/QR action(s)" << endl;
            OutFile << "-----------------------------------------------------------------------" << endl;
            break;
        case 'q': // qc actions
            OutFile << "Task            QC action(s)" << endl;
            OutFile << "-----------------------------------------------------------------------" << endl;
            break;
        case 't': // timesheets
            OutFile << "Task            Timesheets" << endl;
            OutFile << "-----------------------------------------------------------------------" << endl;
            break;
        case 'i': // information about task
            OutFile << "Task            Product                 Status  MSP" << endl;
            OutFile << "-----------------------------------------------------------" << endl;
            break;
        case 'e': // check if task matches regular expression
            OutFile << "Task            Sub-expression(s)" << endl;
            OutFile << "-----------------------------------------------------------" << endl;
            break;
    }

    while (!InFile.eof())
    {
        CString TasksLine = "";
        InFile.getline(TasksLine.GetBuffer(256),255);
        TasksLine.ReleaseBuffer();
        if (!TasksLine.IsEmpty())
        {
            std::vector<CString> Tasks;
            TASK task;
            task.SimpleParseTasks(TasksLine, Tasks);
            for (unsigned int i=0; i<Tasks.size(); i++)
            {
                CString Client = "", Sep = "", ID = "", Ext = "";
                if (!task.IsTaskNameValid(Tasks[i],Client,Sep,ID,Ext))
                {
                    OutFile.width(16);
                    OutFile.setf(OutFile.left);
                    OutFile << Tasks[i] << "- invalid task name format" << endl;
                    if ((type != 'p') && (type != 'i') && (type != 'e')) OutFile << endl;
                }
                else
                {
                    CString Message = "", Request = "";
                    bool IsDefect = false;
                    int defect_index = -1;

                    // checking if a task is SoftTest defect or SIF
                    IsDefect = Settings.IsDefect(Client,NULL,&defect_index);
                    if (Settings.IsSIF(defect_index))
                    {
                        OutFile.width(16);
                        OutFile.setf(OutFile.left);
                        OutFile << Tasks[i] << "- is Software Installation Form" << endl;
                        if ((type != 'p') && (type != 'i') && (type != 'e')) OutFile << endl;
                        continue;
                    }
                    if (Settings.IsHF(defect_index))
                    {
                        OutFile.width(16);
                        OutFile.setf(OutFile.left);
                        OutFile << Tasks[i] << "- is Hotfix" << endl;
                        if ((type != 'p') && (type != 'i') && (type != 'e')) OutFile << endl;
                        continue;
                    }
                    if (IsDefect)
                    {
                        // build URL to open request specific page for SoftTest defects
                        switch (type)
                        {
                            case 'p': // parent task
                                Request = Settings.defects[defect_index].ParentDefectURL;
                                Request.Replace("%PROJECT%",Settings.defects[defect_index].STProject);
                                Request.Replace("%ID%",ID);
                                break;
                            case 'c': // child tasks
                                Request = Settings.defects[defect_index].ChildDefectsURL;
                                Request.Replace("%PROJECT%",Settings.defects[defect_index].STProject);
                                Request.Replace("%ID%",ID);
                                break;
                            case 's': // SPC linked tasks
                            case 'b': // qb actions
                            case 'q': // qc actions
                            case 't': // timesheets
                            case 'i': // information about task
                            case 'e': // check if task matches regular expression
                                OutFile.width(16);
                                OutFile.setf(OutFile.left);
                                OutFile << Tasks[i] << "- is SoftTest defect" << endl;
                                if ((type != 'i') && (type != 'e')) OutFile << endl;
                                continue;
                        }
                    }
                    else // task is TMS task
                    {
                        // build URL to open request specific page for TMS tasks
                        TASK::FillupTaskID(ID); // Settings.FillID is ignored because GetPage() request will not be successfully completed for not complete ID
                        switch (type)
                        {
                            case 'p': // parent task
                                Request.Format(Settings.iTMSviewTask, Client, ID);
                                break;
                            case 'c': // child tasks
                                Request.Format(Settings.iTMSviewChildTasks, Client, ID);
                                break;
                            case 's': // SPC linked tasks
                                Request.Format(Settings.iTMSviewRelatedTasks, Client, ID);
                                break;
                            case 'b': // qb actions
                            case 'q': // qc actions
                                Request.Format(Settings.iTMSviewTask, Client, ID);
                                break;
                            case 't': // timesheets
                                Request.Format(Settings.iTMSviewTimesheets, Client, ID);
                                break;
                            case 'i': // information about task
                                Request.Format(Settings.iTMSviewRelatedTasks, Client, ID); // use the same URL as for SPC linked tasks
                                break;
                            case 'e': // check if task matches regular expression
                                Request.Format(Settings.iTMSviewTask, Client, ID);
                                break;
                        }
                    }
#ifdef DEBUG
Req.OutFile = &OutFile;
OutFile << "GetHeaders started" << endl;
#endif
                    CString reply = Req.GetHeaders(Request);
#ifdef DEBUG
OutFile << "GetPageStatusCode: " << Req.GetPageStatusCode() << endl;
#endif
                    if (Req.GetPageStatusCode() == 401) // Authorization Required
                    {
                        CAmHttpSocket::InsertLoginPassword(Request,Settings.links[index].Login,Settings.links[index].Password);
#ifdef DEBUG
OutFile << "GetHeaders started with authorization" << endl;
#endif
                        reply = Req.GetHeaders(Request);
#ifdef DEBUG
OutFile << "GetPageStatusCode: " << Req.GetPageStatusCode() << endl;
#endif
                    }
#ifdef DEBUG
OutFile << "GetPage started" << endl;
#endif
                    Message = Req.GetPage(Request);
#ifdef DEBUG
OutFile << "GetPage finished" << endl;
#endif
                    OutFile.width(16);
                    OutFile.setf(OutFile.left);

                    if (Message.IsEmpty() || (Message.Find("You don't have access to this site") != -1))
                    {
                        OutFile << Tasks[i] << "- error occured during reading task, probably incorrect login/password or request was timed out" << endl;
                        if ((type != 'p') && (type != 'i') && (type != 'e')) OutFile << endl;
                    }
                    else
                    {
                        // request specific task processing
                        switch (type)
                        {
                            case 'p': // parent task
                                GetParentTasksList(task, Message, Tasks[i], OutFile, Client, IsDefect);
                                break;
                            case 'c': // child tasks
                            case 's': // SPC linked tasks
                                GetChildTasksList(task, Message, Tasks[i], OutFile, Client, ID, IsDefect, type);
                                break;
                            case 'b': // qb actions
                            case 'q': // qc actions
                                GetActionsFromTasks(task, Message, Tasks[i], OutFile, (type == 'b'));
                                break;
                            case 't': // timesheets
                                GetTimesheetsFromTasks(task, Message, Tasks[i], OutFile);
                                break;
                            case 'i': // information about task
                                GetInformationAboutTask(Message, Tasks[i], OutFile, Client, ID);
                                break;
                            case 'e': // check if task matches regular expression
                                PrintSubExpressions(Message, Tasks[i], OutFile);
                                break;
                        }
                    }
                }
            }
        }
    }
    InFile.close();
    OutFile.close();
}

void CmdLine::PrintTask(ofstream &OutFile, CHILD &ChildTask, bool first)
{
    CString line = "";
    if (first)
    {
        line.Format("%-16s%-24s%s", ChildTask.TaskName, ChildTask.Product, ChildTask.Status);
    }
    else
    {
        line.Format("%16s%-16s%-24s%s", " ", ChildTask.TaskName, ChildTask.Product, ChildTask.Status);
    }
    OutFile << line << endl;
}

void CmdLine::PrintSPCtask(ofstream &OutFile, CHILD &ChildTask, bool OrigTask, bool first)
{
    ChildTask.TaskName.Insert(0,"        ");
    ChildTask.TaskName.Delete(0,2*(6-ChildTask.level));

    CString line = "";
    if (first)
    {
        line.Format("%s%-24s%-24s%-8s%s", (OrigTask ? "*" : " "), ChildTask.TaskName, ChildTask.Product, ChildTask.Status, ChildTask.MSP);
    }
    else
    {
        line.Format("%17s%-24s%-24s%-8s%s", (OrigTask ? "*" : " "), ChildTask.TaskName, ChildTask.Product, ChildTask.Status, ChildTask.MSP);
    }
    OutFile << line << endl;
}