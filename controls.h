/*
    File name: controls.h
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
*/

#ifndef CONTROLS_H_INCLUDED
#define CONTROLS_H_INCLUDED

#ifdef INCLUDE_VERID
 static char controls_h[]="@(#)$RCSfile: controls.h,v $$Revision: 1.2 $$Date: 2009/03/22 21:48:51Z $";
#endif

#include <vector>

class CMyListBox: public CListBox
{
public:
    void DeleteSelectedItems();
    bool GetSelectedItems(std::vector<CString> &SelItems);
};

#endif