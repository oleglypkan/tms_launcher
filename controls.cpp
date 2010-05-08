/*
    File name: controls.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
    Date of last modification: November 28, 2006
*/

#include "stdafx.h"

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: controls.cpp,v $$Revision: 1.1 $$Date: 2006/11/28 22:53:41Z $"; 
#endif

#include "controls.h"

bool CMyListBox::DeleteSelItems()
{
    int SelCount = GetSelCount();
    if (SelCount <= 0)
    {
        return false;
    }
    int *SelNumbers = new int[SelCount];
    GetSelItems(SelCount, SelNumbers);
    std::vector<CString> SelItems;
    for (int i = 0; i < SelCount; i++)
    {
        CString Item = "";
        GetText(SelNumbers[i],Item);
        SelItems.push_back(Item);
    }
    delete [] SelNumbers;
    for (i = 0; i < SelItems.size(); i++)
    {
        DeleteString(FindStringExact(-1,SelItems[i]));
    }
    SelItems.clear();
    return true;
}