/*
    File name: CmdLine.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: February 1, 2006
*/

#ifndef CMDLINE_H_INCLUDED
#define CMDLINE_H_INCLUDED

#ifdef INCLUDE_VERID
 static char CmdLine_h[]="@(#)$RCSfile: CmdLine.h,v $$Revision: 1.6 $$Date: 2006/03/23 14:17:23Z $";
#endif

#include <vector>
#include <fstream.h>
#include "Task.h"

class CmdLine
{
public:
    void ParseCmdLine(const char *CommandLine);
    void GetTasksList(const char *InputFileName, const char *OutputFileName, bool parent);
    CmdLine();
private:
    CString Parameters[5];
//  Parameters[0] - "-c/p"
//  Parameters[1] - "-name"
//  Parameters[2] - "-prod"
//  Parameters[3] - "-in"
//  Parameters[4] - "-out"
    void StringToArgv(const char *CommandLine, std::vector<CString> &params);
    void PrintTask(ofstream &OutFile, CHILD &ChildTask);
};

#endif