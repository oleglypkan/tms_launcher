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
 static char verid[]="@(#)$RCSfile: CmdLine.cpp,v $$Revision: 1.5 $$Date: 2006/02/06 17:07:13Z $"; 
#endif

extern CSettings Settings;
int CompareNoCaseCP1251(const char *string1, const char *string2);

CmdLine::CmdLine()
{
    Parameter = 0;
}

void CmdLine::ParseCmdLine(const char *CommandLine)
{
/*
    The following parameters are correct:
    
    TMS_Launcher.exe -p input output
      get the list of parent tasks for the given ones

    TMS_Launcher.exe -c input output
      get the list of child tasks for the given ones
*/
    CString Line = CommandLine;
    CString Input = "";
    CString Output = "";

    int pos = Line.Find("-p ");
    if (pos == -1) // no "-p" parameter in command line
    {
        pos = Line.Find("-c ");
        if (pos == -1)
        {
            PrintInfo();
            return;
        }
        else
        {
            Parameter = 2;
        }
    }
    else
    {
        Parameter = 1;
    }
    Line.Delete(pos,3);
    if (Line[0] == '"') // first file is quoted
    {
        Line.Delete(0,1);
        pos = Line.Find("\"");
        if (pos == -1)
        {
            PrintInfo();
            return;
        }
        Input = Line.Left(pos);
        Line.Delete(0,pos+1);
    }
    else // first file is not quoted
    {
        pos = Line.Find(" ");
        if (pos == -1)
        {
            PrintInfo();
            return;
        }
        Input = Line.Left(pos);
        Input.TrimLeft(" \"");
        Input.TrimRight(" \"");
        Line.Delete(0,pos+1);
    }
    Output = Line;
    Output.TrimLeft(" \"");
    Output.TrimRight(" \"");
    switch (Parameter)
    {
        case 1:
            GetTasksList(Input,Output, true);
            break;
        case 2:
            GetTasksList(Input,Output, false);
            break;
    }
}

void CmdLine::GetTasksList(const char *InputFileName, const char *OutputFileName, bool parent)
{
    int index = 0;
    for (int i = 0; i < Settings.links.size(); i++)
    {
        if (Settings.links[i].Default)
        {
            index = i;
            break;
        }
    }
    if (Settings.links[index].Login.IsEmpty() || Settings.links[index].Password.IsEmpty())
    {
        ofstream OutFile;
        OutFile.open(OutputFileName, ios::out);
        OutFile << "Both Login and Password must be defined in TMS Launcher settings window for \""+Settings.links[index].Caption+"\" URL";
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
    }
    else
    {
        OutFile << "Task            Child tasks" << endl;
    }
    OutFile << "---------------------------" << endl;

    while (!InFile.eof())
    {
        InFile.getline(TasksLine.GetBuffer(256),255);
        TasksLine.ReleaseBuffer();
        if (!TasksLine.IsEmpty())
        {
            std::vector<CString> Tasks;
            TASK task;
            task.ParseTasks(TasksLine, Tasks);
            for (int i=0; i<Tasks.size(); i++)
            {
                CString Client, ID;
                if (!task.IsTaskNameValid(Tasks[i],Client,ID))
                {
                    OutFile << Tasks[i] << " - invalid task name format" << endl;
                }
                else
                {
                    REQUEST Req;
                    CString HeaderSend, HeaderReceive, Message, Request;
                    bool IsDefect = false;

                    // checking if a task is SoftTest defect
                    for (int j=0; j<Settings.defects.size(); j++)
                    {
                        if (CompareNoCaseCP1251(Client,Settings.defects[j].ClientID)==0)
                        {
                            if (parent)
                            {
                                Request = Settings.defects[j].ParentDefectURL;
                            }
                            else
                            {
                                Request = Settings.defects[j].ChildDefectsURL;
                            }
                            Request.Replace("%PROJECT%",Settings.defects[j].STProject);
                            Request.Replace("%ID%",ID);
                            IsDefect = true;
                            break;
                        }
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
                                std::vector<CString> ChildTasks;
                                switch(task.ParseHTMLForChildTasks(ParentTask,Message,ChildTasks))
                                {
                                    case 0:
                                        {
                                            OutFile << Tasks[i] << endl;
                                            for (int j = 0; j < ChildTasks.size(); j++)
                                            {
                                                OutFile.width(16);
                                                OutFile.setf(OutFile.left);
                                                OutFile << " " << ChildTasks[j] << endl;
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

void CmdLine::PrintInfo()
{
    cout << "Parameters are incorrect." << endl;
    cout << "Correct parameters are the following:" << endl;
    cout << "TMS_Launcher.exe -p InputFile OutputFile" << endl;
    cout << "     get the list of parent tasks for the given ones" << endl << endl;
    cout << "TMS_Launcher.exe -c InputFile OutputFile" << endl;
    cout << "     get the list of child tasks for the given ones";
}