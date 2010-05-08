// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__51353A8C_4FE8_4996_B9A6_EA2293CA582D__INCLUDED_)
#define AFX_STDAFX_H__51353A8C_4FE8_4996_B9A6_EA2293CA582D__INCLUDED_

#ifdef INCLUDE_VERID
 static char stdafx_h[]="@(#)$RCSfile: stdafx.h,v $$Revision: 1.3 $$Date: 2005/05/25 16:17:02Z $";
#endif

// Change these values to use different versions
#define WINVER      0x0400
#define _WIN32_IE   0x0400
#define _RICHEDIT_VER   0x0200

#define _WTL_USE_CSTRING

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atlcrack.h>
#include <atlmisc.h>
#include <atldlgs.h>
#include <atlctrlw.h>

#if _ATL_VER < 0x0700
#undef BEGIN_MSG_MAP
#define BEGIN_MSG_MAP(x) BEGIN_MSG_MAP_EX(x)
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__51353A8C_4FE8_4996_B9A6_EA2293CA582D__INCLUDED_)
