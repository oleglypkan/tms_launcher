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
 static char CmdLine_h[]="@(#)$RCSfile: CmdLine.h,v $$Revision: 1.3 $$Date: 2006/02/03 16:18:23Z $";
#endif

class CmdLine
{
public:
    void ParseCmdLine(const char *CommandLine);
    void GetTasksList(const char *InputFileName, const char *OutputFileName, bool parent);
    CmdLine();
private:
    int Parameter;
    void PrintInfo();
};

#endif