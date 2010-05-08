/*
    File name: CmdLine.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: February 1, 2006
*/

#include "stdafx.h"
#include "CmdLine.h"
#include "settings.h"
#include "MainDlg.h"
#include "Request.h"
#include "Task.h"
#include <fstream.h>

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: CmdLine.cpp,v $$Revision: 1.13 $$Date: 2006/03/23 14:17:29Z $"; 
#endif

extern CSettings Settings;
int CompareNoCaseCP1251(const char *string1, const char *string2);

CmdLine::CmdLine()
{
//  Parameters[0] - "-c/p"
//  Parameters[1] - "-name"
//  Parameters[2] - "-prod"
//  Parameters[3] - "-in"
//  Parameters[4] - "-out"
    Parameters[0] = "";
    Parameters[1] = "";
    Parameters[2] = "";
    Parameters[3] = "";
    Parameters[4] = "";
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
    
    TMS_Launcher.exe <-p> <input> <output>
      get the list of parent tasks for the given ones

    TMS_Launcher.exe <-c> [<-name "TCD">] [<-prod ".X">] <input> <output>
      get the list of child tasks for the given ones
*/
    std::vector<CString> argv;
    StringToArgv(CommandLine,argv);

    for (int i = 0; i < argv.size(); i++)
    {
        if (argv[i].CompareNoCase("-c") == 0)
        {
            Parameters[0] = "c";
        }
        else
        {
            if (argv[i].CompareNoCase("-p") == 0)
            {
                Parameters[0] = "p";
            }
            else
            {
                if (argv[i].CompareNoCase("-name") == 0)
                {
                    if (argv.size() > i+1)
                    {
                        Parameters[1] = argv[i+1];
                        i++;
                    }
                }
                else
                {
                    if (argv[i].CompareNoCase("-prod") == 0)
                    {
                        if (argv.size() > i+1)
                        {
                            Parameters[2] = argv[i+1];
                            i++;
                        }
                    }
                    else
                    {
                        if (Parameters[3].IsEmpty())
                        {
                            Parameters[3] = argv[i];
                        }
                        else
                        {
                            if (Parameters[4].IsEmpty())
                            {
                                Parameters[4] = argv[i];
                            }
                        }
                    }
                }
            }
        }
    }

    // checking for correct parameters
    if (Parameters[0].IsEmpty() || Parameters[3].IsEmpty() || Parameters[4].IsEmpty())
    {
        return;
    }

    switch (Parameters[0][0])
    {
        case 'p':
            GetTasksList(Parameters[3],Parameters[4],true);
            break;
        case 'c':
            GetTasksList(Parameters[3],Parameters[4],false);
            break;
    }
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
        OutFile << "Task            Child tasks     Product" << endl;
        OutFile << "-------------------------------------------------" << endl;
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
                    REQUEST Req;
                    CString HeaderSend, HeaderReceive, Message, Request;
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
                            Request.Format("http://%s:%s@scc1/~alttms/viewtask.php?Client=%s&ID=%s",
                                           Settings.links[index].Login,Settings.links[index].Password,
                                           Client, ID);
                        }
                        else
                        {
                            
                            Request.Format("http://%s:%s@scc1/~alttms/showtasks.php?ParentClient=%s&ParentID=%s",
                                           Settings.links[index].Login,Settings.links[index].Password,
                                           Client, ID);
                        }
                    }
                    if (!Req.SendRequest(false, Request, HeaderSend, HeaderReceive, Message))
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
                                        OutFile << Tasks[i] << ParentTask << endl;
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
                                                if (Parameters[1].IsEmpty()) // filtering by task name is NOT needed 
                                                {
                                                    if (Parameters[2].IsEmpty()) // filtering by product is NOT needed
                                                    {
                                                        PrintTask(OutFile, ChildTasks[j]);
                                                    }
                                                    else // filtering by product is needed 
                                                    {
                                                        if (ChildTasks[j].Product.Find(Parameters[2]) != -1)
                                                        {
                                                            PrintTask(OutFile, ChildTasks[j]);
                                                        }
                                                    }
                                                }
                                                else // filtering by task name is needed 
                                                {
                                                    if (ChildTasks[j].TaskName.Find(Parameters[1]) != -1)
                                                    {
                                                        if (Parameters[2].IsEmpty()) // filtering by product is NOT needed
                                                        {
                                                            PrintTask(OutFile, ChildTasks[j]);
                                                        }
                                                        else // filtering by product is needed
                                                        {
                                                            if (ChildTasks[j].Product.Find(Parameters[2]) != -1)
                                                            {
                                                                PrintTask(OutFile, ChildTasks[j]);
                                                            }
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

void CmdLine::PrintTask(ofstream &OutFile, CHILD &ChildTask)
{
    OutFile.width(16);
    OutFile.setf(OutFile.left);
    OutFile << " ";
    OutFile.width(16);
    OutFile.setf(OutFile.left);
    OutFile << ChildTask.TaskName;
    OutFile.width(16);
    OutFile.setf(OutFile.left);
    OutFile << ChildTask.Product << endl;
}
