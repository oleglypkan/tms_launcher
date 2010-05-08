#ifdef INCLUDE_VERID
 static char tools_h[]="@(#)$RCSfile: Tools.h,v $$Revision: 1.1 $$Date: 2005/07/04 10:04:13Z $";
#endif
 
BOOL OpenLink(const char* Link, HWND ParentWindow = NULL, const char* Operation = "open",
              const char* Parameters = NULL, const char* Directory = NULL,
			  INT nShowCmd = SW_SHOWNORMAL)
{
	if (32 > (int)ShellExecute(ParentWindow,Operation,Link,Parameters,Directory,nShowCmd))
		return FALSE;
	else
	    return TRUE;
}