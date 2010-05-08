/*
    File name: controls.cpp
    Purpose:   This module is a part of TMS Launcher source code
    Author:    Oleg Lypkan
    Copyright: Information Systems Development
*/

#include "stdafx.h"

#ifndef NO_VERID
 static char verid[]="@(#)$RCSfile: controls.cpp,v $$Revision: 1.3 $$Date: 2009/03/22 21:48:34Z $"; 
#endif

#include "controls.h"

void CMyListBox::DeleteSelectedItems()
{
    std::vector<CString> SelItems;
    if (!GetSelectedItems(SelItems)) return;

    for (unsigned int j = 0; j < SelItems.size(); j++)
    {
        DeleteString(FindStringExact(-1,SelItems[j]));
    }
    SelItems.clear();
}

bool CMyListBox::GetSelectedItems(std::vector<CString> &SelItems)
{
    int SelCount = GetSelCount();
    if (SelCount <= 0)
    {
        return false;
    }
    int *SelNumbers = new int[SelCount];
    GetSelItems(SelCount, SelNumbers);
    SelItems.clear();
    for (int i = 0; i < SelCount; i++)
    {
        CString Item = "";
        GetText(SelNumbers[i],Item);
        SelItems.push_back(Item);
    }
    delete [] SelNumbers;    
    return true;
}