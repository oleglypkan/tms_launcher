/*
    File name: CmdLine.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
*/

#ifndef CMDLINE_H_INCLUDED
#define CMDLINE_H_INCLUDED

#ifdef INCLUDE_VERID
 static char CmdLine_h[]="@(#)$RCSfile: CmdLine.h,v $$Revision: 1.16 $$Date: 2009/03/16 23:29:19Z $";
#endif

#include <vector>
#include <fstream>
#include "Task.h"
using namespace std;

class CmdLine
{
public:
    void ParseCmdLine(const char *CommandLine);
    void ProcessInputFile(const char *InputFileName, const char *OutputFileName, const char type);
    CmdLine();
private:
    CString Parameters[7];
//  Parameters[0] - "-c/p"
//  Parameters[1] - "-name"
//  Parameters[2] - "-prod"
//  Parameters[3] - "-stat"
//  Parameters[4] - "-msp"
//  Parameters[5] - "-in"
//  Parameters[6] - "-out"
    void StringToArgv(const char *CommandLine, std::vector<CString> &params);
    void PrintTask(ofstream &OutFile, CHILD &ChildTask, bool first);
    void PrintSPCtask(ofstream &OutFile, CHILD &ChildTask, bool OrigTask, bool first);
    void GetActionsFromTasks(TASK &task, const CString &Message, const char *OriginalTask, ofstream &OutFile, bool QB);
    void GetTimesheetsFromTasks(TASK &task, const CString &Message, const char *OriginalTask, ofstream &OutFile);
    void GetParentTasksList(TASK &task, const CString &Message, const char *OriginalTask, ofstream &OutFile, CString &Client,  bool IsDefect);
    void GetChildTasksList(TASK &task, const CString &Message, const char *OriginalTask, ofstream &OutFile, CString &Client, const CString &ID, bool IsDefect, const char type);
    void GetInformationAboutTask(const CString &Message, const char *OriginalTask, ofstream &OutFile, const CString &Client, const CString &ID);
    void PrintSubExpressions(const CString &Message, const char *OriginalTask, ofstream &OutFile);
    void ShowUsage();
};

#endif