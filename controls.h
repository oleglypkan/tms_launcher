/*
    File name: controls.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: November 28, 2006
*/

#ifndef CONTROLS_H_INCLUDED
#define CONTROLS_H_INCLUDED

#ifdef INCLUDE_VERID
 static char controls_h[]="@(#)$RCSfile: controls.h,v $$Revision: 1.1 $$Date: 2006/11/28 22:53:56Z $";
#endif

#include <vector>

class CMyListBox: public CListBox
{
public:
    bool DeleteSelItems();
};

#endif