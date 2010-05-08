/*
    File name: CmdLine.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: Match 9, 2008
*/

#include "stdafx.h"
#include <fstream.h>
#include "CmdLine.h"
#include "settings.h"
#include "MainDlg.h"
#include "Task.h"
#include "AmHttpSocket.h"
#include <boost/regex.hpp>
using namespace boost;

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: CmdLine.cpp,v $$Revision: 1.19 $$Date: 2008/03/19 20:38:41Z $"; 
#endif

extern CSettings Settings;
int CompareNoCaseCP1251(const char *string1, const char *string2);

CmdLine::CmdLine()
{
//  Parameters[0] - "-c/p/qb/qc/t"
//  Parameters[1] - "-name"
//  Parameters[2] - "-prod"
//  Parameters[3] - "-stat"
//  Parameters[4] - "-in"
//  Parameters[5] - "-out"
    Parameters[0] = "";
    Parameters[1] = "";
    Parameters[2] = "";
    Parameters[3] = "";
    Parameters[4] = "";
    Parameters[5] = "";
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

void CmdLine::ParseCmdLine(const char *CommandLine)
{
/*
    The following parameters are correct:
    
    TMS_Launcher.exe <-p> [<-name "PCC">] <input> <output>
      get the list of parent tasks for the given ones

    TMS_Launcher.exe <-c> [<-name "TCD">] [<-prod ".X">] [<-stat "C1">] <input> <output>
      get the list of child tasks for the given ones

    TMS_Launcher.exe <-qb> <input> <output>
      get the list of QB actions for the given tasks

    TMS_Launcher.exe <-qc> <input> <output>
      get the list of QC actions for the given tasks

    TMS_Launcher.exe <-t> [<-name "OLEG L">] <input> <output>
      get timesheets for the given tasks
*/
    std::vector<CString> argv;
    StringToArgv(CommandLine,argv);

    for (int i = 0; i < argv.size(); i++)
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
        if (Parameters[4].IsEmpty())
        {
            Parameters[4] = argv[i];
        }
        else
        {
            if (Parameters[5].IsEmpty())
            {
                Parameters[5] = argv[i];
            }
        }
    }

    // checking for correct parameters
    if (Parameters[0].IsEmpty() || Parameters[4].IsEmpty() || Parameters[5].IsEmpty())
    {
        return;
    }
    for (i = 1; i <= 3; i++)
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
                return;
            }
        }
    }

    switch (Parameters[0][0])
    {
        case 'p': // parent
            GetTasksList(Parameters[4],Parameters[5],true);
            break;
        case 'c': // child
            GetTasksList(Parameters[4],Parameters[5],false);
            break;
        case 'b': // qb
            GetActionFromTasks(Parameters[4],Parameters[5],true);
            break;
        case 'q': // qc
            GetActionFromTasks(Parameters[4],Parameters[5],false);
            break;
        case 't': // timesheets
            GetTimesheetsFromTasks(Parameters[4],Parameters[5]);
            break;
    }
}

void CmdLine::GetTimesheetsFromTasks(const char *InputFileName, const char *OutputFileName)
{
    int index = Settings.GetDefaultUrlIndex();
    if (index == -1) return;

    if (Settings.links[index].Login.IsEmpty() || Settings.links[index].Password.IsEmpty())
    {
        ofstream OutFile;
        OutFile.open(OutputFileName, ios::out);
        OutFile << "Both Login and Password must be specified in settings window for \""+Settings.links[index].Caption+"\" URL";
        OutFile.close();
        return;
    }
    CString TasksLine = "";
    ifstream InFile;
    InFile.open(InputFileName, ios::in);
    ofstream OutFile;
    OutFile.open(OutputFileName, ios::out);

    OutFile << "Task            Timesheets" << endl;
    OutFile << "-----------------------------------------------------------------------" << endl;

    while (!InFile.eof())
    {
        InFile.getline(TasksLine.GetBuffer(256),255);
        TasksLine.ReleaseBuffer();
        if (!TasksLine.IsEmpty())
        {
            std::vector<CString> Tasks;
            TASK task;
            task.SimpleParseTasks(TasksLine, Tasks);
            for (int i=0; i<Tasks.size(); i++)
            {
                CString Client, Sep, ID;
                if (!task.IsTaskNameValid(Tasks[i],Client,Sep,ID))
                {
                    OutFile << Tasks[i] << " - invalid task name format" << endl << endl;
                }
                else
                {
                    CAmHttpSocket Req;
                    CString Message = "", Request;
                    bool IsDefect = false;
                    int defect_index = -1;

                    // checking if a task is SoftTest defect
                    if (IsDefect = Settings.IsDefect(Client,NULL,&defect_index))
                    {
                        OutFile << Tasks[i] << " - is SoftTest defect" << endl << endl;
                        continue;
                    }
                    Request.Format("http://%s:%s@scc1.softcomputer.com/~alttms/tmsh.php?Client=%s&ID=%s",
                                   Settings.links[index].Login,Settings.links[index].Password,
                                   Client, ID);
                    Message = Req.GetPage(Request);
                    if (Message.IsEmpty())
                    {
                        OutFile << Tasks[i] << " - error occured during reading task" << endl;
                    }
                    else
                    {
                        OutFile.width(16);
                        OutFile.setf(OutFile.left);
                        
                        std::vector<CString> Timesheets;
                        bool filtered = false;
                        task.ParseHTMLForTimesheets(Message,Timesheets,Parameters[1],filtered);
                        if (Timesheets.size()==0)
                        {
                            OutFile << Tasks[i];
                            if (!filtered)
                            {
                                OutFile << "-----------";
                            }
                            OutFile << endl << endl;
                            continue;
                        }
                        OutFile << Tasks[i];
                        OutFile << "Date            Employee          Hours       Comments" << endl;
                        
                        for (int j=0; j<Timesheets.size(); j++)
                        {
                            OutFile.width(16);
                            OutFile.setf(OutFile.left);
                            OutFile << " " << Timesheets[j] << endl;
                        }
                        OutFile << endl;
                    }
                }
            }
        }
    }
    InFile.close();
    OutFile.close();
}

void CmdLine::GetActionFromTasks(const char *InputFileName, const char *OutputFileName, bool QB)
{
    int index = Settings.GetDefaultUrlIndex();
    if (index == -1) return;

    if (Settings.links[index].Login.IsEmpty() || Settings.links[index].Password.IsEmpty())
    {
        ofstream OutFile;
        OutFile.open(OutputFileName, ios::out);
        OutFile << "Both Login and Password must be specified in settings window for \""+Settings.links[index].Caption+"\" URL";
        OutFile.close();
        return;
    }
    CString TasksLine = "";
    ifstream InFile;
    InFile.open(InputFileName, ios::in);
    ofstream OutFile;
    OutFile.open(OutputFileName, ios::out);

    if (QB)
    {
        OutFile << "Task            QB/QR action(s)" << endl;
    }
    else
    {
        OutFile << "Task            QC action(s)" << endl;
    }
    OutFile << "-----------------------------------------------------------------------" << endl;

    while (!InFile.eof())
    {
        InFile.getline(TasksLine.GetBuffer(256),255);
        TasksLine.ReleaseBuffer();
        if (!TasksLine.IsEmpty())
        {
            std::vector<CString> Tasks;
            TASK task;
            task.SimpleParseTasks(TasksLine, Tasks);
            for (int i=0; i<Tasks.size(); i++)
            {
                CString Client, Sep, ID;
                if (!task.IsTaskNameValid(Tasks[i],Client,Sep,ID))
                {
                    OutFile << Tasks[i] << " - invalid task name format" << endl << endl;
                }
                else
                {
                    CAmHttpSocket Req;
                    CString Message = "", Request;
                    bool IsDefect = false;
                    int defect_index = -1;

                    // checking if a task is SoftTest defect
                    if (IsDefect = Settings.IsDefect(Client,NULL,&defect_index))
                    {
                        OutFile << Tasks[i] << " - is SoftTest defect" << endl << endl;
                        continue;
                    }
                    Request.Format("http://%s:%s@scc1.softcomputer.com/~alttms/viewtask.php?Client=%s&ID=%s",
                                   Settings.links[index].Login,Settings.links[index].Password,
                                   Client, ID);
                    Message = Req.GetPage(Request);
                    if (Message.IsEmpty())
                    {
                        OutFile << Tasks[i] << " - error occured during reading task" << endl;
                    }
                    else
                    {
                        OutFile.width(16);
                        OutFile.setf(OutFile.left);
                        
                        std::vector<CString> TaskActions;
                        task.ParseHTMLForActions(Message,TaskActions,QB);
                        if (TaskActions.size()==0)
                        {
                            OutFile << Tasks[i] << "-----------" << endl << endl;
                            continue;
                        }
                        OutFile << Tasks[i];
                        if (QB)
                        {
                            OutFile << "#   Date        Time   Person            Assigned to" << endl;
                        }
                        else
                        {
                            OutFile << "#   Date        Time   Person            Requirement(s)" << endl;
                        }
                        for (int j=0; j<TaskActions.size(); j++)
                        {
                            OutFile.width(16);
                            OutFile.setf(OutFile.left);
                            OutFile << " " << TaskActions[j] << endl;
                        }
                        OutFile << endl;
                    }
                }
            }
        }
    }
    InFile.close();
    OutFile.close();
}

void CmdLine::GetTasksList(const char *InputFileName, const char *OutputFileName, bool parent)
{
    int index = Settings.GetDefaultUrlIndex();
    if (index == -1) return;

    if (Settings.links[index].Login.IsEmpty() || Settings.links[index].Password.IsEmpty())
    {
        ofstream OutFile;
        OutFile.open(OutputFileName, ios::out);
        OutFile << "Both Login and Password must be specified in settings window for \""+Settings.links[index].Caption+"\" URL";
        OutFile.close();
        return;
    }
    CString TasksLine = "";
    ifstream InFile;
    InFile.open(InputFileName, ios::in);
    ofstream OutFile;
    OutFile.open(OutputFileName, ios::out);

    if (parent)
    {
        OutFile << "Task            Parent task" << endl;
        OutFile << "---------------------------" << endl;
    }
    else
    {
        OutFile << "Task            Child tasks     Product                 Status" << endl;
        OutFile << "--------------------------------------------------------------" << endl;
    }

    while (!InFile.eof())
    {
        InFile.getline(TasksLine.GetBuffer(256),255);
        TasksLine.ReleaseBuffer();
        if (!TasksLine.IsEmpty())
        {
            std::vector<CString> Tasks;
            TASK task;
            task.SimpleParseTasks(TasksLine, Tasks);
            for (int i=0; i<Tasks.size(); i++)
            {
                CString Client, Sep, ID;
                if (!task.IsTaskNameValid(Tasks[i],Client,Sep,ID))
                {
                    OutFile << Tasks[i] << " - invalid task name format" << endl;
                }
                else
                {
                    CAmHttpSocket Req;
                    CString Message = "", Request;
                    bool IsDefect = false;
                    int defect_index = -1;

                    // checking if a task is SoftTest defect
                    if (IsDefect = Settings.IsDefect(Client,NULL,&defect_index))
                    {
                        if (parent)
                        {
                            Request = Settings.defects[defect_index].ParentDefectURL;
                        }
                        else
                        {
                            Request = Settings.defects[defect_index].ChildDefectsURL;
                        }
                        Request.Replace("%PROJECT%",Settings.defects[defect_index].STProject);
                        Request.Replace("%ID%",ID);
                    }
                    
                    if (!IsDefect)
                    {
                        if (parent)
                        {
                            Request.Format("http://%s:%s@scc1.softcomputer.com/~alttms/viewtask.php?Client=%s&ID=%s",
                                           Settings.links[index].Login,Settings.links[index].Password,
                                           Client, ID);
                        }
                        else
                        {
                            
                            Request.Format("http://%s:%s@scc1.softcomputer.com/~alttms/showtasks.php?ParentClient=%s&ParentID=%s",
                                           Settings.links[index].Login,Settings.links[index].Password,
                                           Client, ID);
                        }
                    }
                    Message = Req.GetPage(Request);
                    if (Message.IsEmpty())
                    {
                        OutFile << Tasks[i] << " - error occured during reading task" << endl;
                    }
                    else
                    {
                        OutFile.width(16);
                        OutFile.setf(OutFile.left);
                        
                        CString ParentTask = Tasks[i];

                        // get parent/child for SoftTest defect
                        if (IsDefect)
                        {
                            if (parent)
                            {
                                switch (task.ParseHTMLForParentDefect(ParentTask, Message))
                                {
                                    case 0:
                                        if (!Client.IsEmpty()) Client += "-";
                                        OutFile << Tasks[i] << Client+ParentTask << endl;
                                        break;
                                    case 1:
                                    case 2:
                                        if (ParentTask.Find("does not have parent") != -1)
                                        {
                                            OutFile << Tasks[i] << "-----------" << endl;
                                        }
                                        else
                                        {
                                            OutFile << ParentTask << endl;
                                        }
                                        break;
                                }
                            }
                            else
                            {
                                std::vector<CString> ChildTasks;
                                switch(task.ParseHTMLForChildDefects(Message,ChildTasks))
                                {
                                    case 0:
                                        {
                                            if (!Client.IsEmpty()) Client += "-";
                                            OutFile << Tasks[i] << endl;
                                            for (int j = 0; j < ChildTasks.size(); j++)
                                            {
                                                OutFile.width(16);
                                                OutFile.setf(OutFile.left);
                                                OutFile << " " << Client+ChildTasks[j] << endl;
                                            }
                                        }
                                        break;
                                    case 1:
                                        OutFile << Tasks[i] << "-----------" << endl;
                                        break;
                                }
                            }
                        }
                        else // get parent/child for TMS task
                        {
                            if (parent)
                            {
                                switch (task.ParseHTMLForParentTask(ParentTask, Message))
                                {
                                    case 0:
                                        if (!FilterTask(ParentTask,Parameters[1]))
                                        {
                                            OutFile << Tasks[i] << ParentTask << endl;
                                        }
                                        else
                                        {
                                            OutFile << Tasks[i] << endl;
                                        }
                                        break;
                                    case 1:
                                    case 2:
                                        if (ParentTask.Find("does not have parent") != -1)
                                        {
                                            OutFile << Tasks[i] << "-----------" << endl;
                                        }
                                        else
                                        {
                                            OutFile << ParentTask << endl;
                                        }
                                        break;
                                }
                            }
                            else
                            {
                                std::vector<CHILD> ChildTasks;
                                switch(task.ParseHTMLForChildTasks(ParentTask,Message,ChildTasks))
                                {
                                    case 0:
                                        {
                                            OutFile << Tasks[i] << endl;
                                            for (int j = 0; j < ChildTasks.size(); j++)
                                            {
                                                if (!FilterTask(ChildTasks[j].TaskName,Parameters[1])) // filtering by task name
                                                {
                                                    if (!FilterTask(ChildTasks[j].Product,Parameters[2])) // filtering by product
                                                    {
                                                        if (!FilterTask(ChildTasks[j].Status,Parameters[3])) // filtering by status
                                                        {
                                                            PrintTask(OutFile, ChildTasks[j]);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        break;
                                    case 1:
                                        OutFile << Tasks[i] << "-----------" << endl;
                                        break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    InFile.close();
    OutFile.close();
    return;
}

// true  - task should NOT be printed
// false - task should be printed
bool CmdLine::FilterTask(const CString &string, const CString &mask)
{
    if (mask.IsEmpty()) return false;

    // filter that allows to use regular expressions
    RegEx expr;
    expr.SetExpression(mask,true);
    return !expr.Search(string,match_any);

/*
    // simple filter that allows "!" to be used as "NOT"
    CString temp = mask;
    bool exclude = false;

    if (temp[0] == '!')
    {
        temp.Delete(0);
        if (!temp.IsEmpty()) 
        {
            exclude = true;
        }
    }

    if (string.Find(temp) != -1) // temp is found in string
    {
        return exclude;
    }
    else
    {
        return !exclude;
    }
*/
}

void CmdLine::PrintTask(ofstream &OutFile, CHILD &ChildTask)
{
    OutFile.width(16);
    OutFile.setf(OutFile.left);
    OutFile << " ";
    OutFile.width(16);
    OutFile.setf(OutFile.left);
    OutFile << ChildTask.TaskName;
    OutFile.width(24);
    OutFile.setf(OutFile.left);
    OutFile << ChildTask.Product;
    OutFile.setf(OutFile.left);
    OutFile << ChildTask.Status << endl;
}
