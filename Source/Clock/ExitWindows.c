  //============================================================================
 //  ExitWindows.c  -  Stoic Joker 2006  =======================================
//==============================================================================
#include "tclock.h"
//------------------------------------------------------------------------------
BOOL ShutDown() {
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

  if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken)) return FALSE;

  LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,&tkp.Privileges[0].Luid);
  tkp.PrivilegeCount = 1;
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,(PTOKEN_PRIVILEGES)NULL, 0);
  if(GetLastError() != ERROR_SUCCESS) return FALSE;

  if(!ExitWindowsEx(EWX_SHUTDOWN|EWX_FORCE,0)) return FALSE;
 return TRUE;
}

//------------------------------------------------------------------------------
BOOL ReBoot() {
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

  if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken)) return FALSE;

  LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,&tkp.Privileges[0].Luid);
  tkp.PrivilegeCount = 1;
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,(PTOKEN_PRIVILEGES)NULL, 0);
  if(GetLastError() != ERROR_SUCCESS) return FALSE;

  if(!ExitWindowsEx(EWX_REBOOT|EWX_FORCE,0)) return FALSE;
 return TRUE;
}

//------------------------------------------------------------------------------
BOOL LogOff() {
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

  if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken)) return FALSE;

  LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,&tkp.Privileges[0].Luid);
  tkp.PrivilegeCount = 1;
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,(PTOKEN_PRIVILEGES)NULL, 0);
  if(GetLastError() != ERROR_SUCCESS) return FALSE;

  if(!ExitWindowsEx(EWX_LOGOFF|EWX_FORCE,0)) return FALSE;
 return TRUE;
}