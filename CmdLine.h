/*
    File name: CmdLine.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: March 8, 2008
*/

#ifndef CMDLINE_H_INCLUDED
#define CMDLINE_H_INCLUDED

#ifdef INCLUDE_VERID
 static char CmdLine_h[]="@(#)$RCSfile: CmdLine.h,v $$Revision: 1.8 $$Date: 2008/03/19 20:38:33Z $";
#endif

#include <vector>
#include <fstream.h>
#include "Task.h"

class CmdLine
{
public:
    void ParseCmdLine(const char *CommandLine);
    void GetTasksList(const char *InputFileName, const char *OutputFileName, bool parent);
    void GetActionFromTasks(const char *InputFileName, const char *OutputFileName, bool QB);
    void GetTimesheetsFromTasks(const char *InputFileName, const char *OutputFileName);
    CmdLine();
private:
    CString Parameters[6];
//  Parameters[0] - "-c/p"
//  Parameters[1] - "-name"
//  Parameters[2] - "-prod"
//  Parameters[3] - "-stat"
//  Parameters[4] - "-in"
//  Parameters[5] - "-out"
    void StringToArgv(const char *CommandLine, std::vector<CString> &params);
    void PrintTask(ofstream &OutFile, CHILD &ChildTask);
    bool FilterTask(const CString &string, const CString &mask);
//  true  - task should NOT be printed
//  false - task should be printed
};

#endif