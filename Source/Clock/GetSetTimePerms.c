#include "TClock.h"
//BOOL RequestAdminRights(); <- This Just Ain't Gonna Happen!)
void SyncTimeNow();
 //================================================================================================
//-----------------------+++--> Verify That User Has or Can Get Permission to Set the System Clock:
BOOL GetSetTimePermissions() { //-----------------------------------------------------------+++-->
	HANDLE      hToken;     // process token
	TOKEN_PRIVILEGES tp;    // token provileges
	TOKEN_PRIVILEGES oldtp; // old token privileges
	DWORD dwSize = sizeof(TOKEN_PRIVILEGES);
	char szErr[MIN_BUFF] = {0};
	LUID     luid;          

   // now, set the SE_SYSTEMTIME_NAME privilege to our
  // current process, so we can (hopefully) call SetSystemTime()
  if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
//  if(!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, TRUE, &hToken)) { 
	  // ^^^This^^^ FAILS With No Token Error - Much Like it Did for Everyone Else That Tried it.
	  wsprintf(szErr, "OpenProcessToken() failed with code %d\n", GetLastError());
	  MessageBox(0, szErr, "User Fail!", MB_OK|MB_ICONERROR);
    return FALSE;
  }
  
  if(!LookupPrivilegeValue(NULL, SE_SYSTEMTIME_NAME, &luid)) {
	  wsprintf(szErr, "LookupPrivilege() failed with code %d\n", GetLastError());
	  MessageBox(0, szErr, "User Fail!", MB_OK|MB_ICONERROR);
	  CloseHandle(hToken);
    return FALSE;
  }

  ZeroMemory(&tp, sizeof(tp));
  tp.PrivilegeCount = 1;
  tp.Privileges[0].Luid = luid;
  tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  // Adjust Token privileges
  AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &oldtp, &dwSize);
  if(GetLastError() != ERROR_SUCCESS) {
	  CloseHandle(hToken);
	  MessageBox(0, "SE_SYSTEMTIME_NAME Privilege\n Cannot be Enabled!", 
		  "Privilege Failure", MB_OK|MB_ICONERROR);
//    return RequestAdminRights(); - This Just Ain't Gonna Work...
    return FALSE;
  }
      
  // Set time - Pointless Exorcise IF Not Moved Elsewhere
//  if(!SetSystemTime(&stCurrentTime)) {
//   printf ("SetSystemTime() failed with code %d\n", GetLastError());
//	  CloseHandle(hToken);
//    return FALSE;
//  }

  // disable SE_SYSTEMTIME_NAME again  - Pointless Exorcise IF Not Moved Elsewhere
//  AdjustTokenPrivileges(hToken, FALSE, &oldtp, dwSize, NULL, NULL);
//  if(GetLastError() != ERROR_SUCCESS) {
//   printf ("AdjustTokenPrivileges() failed with code %d\n", GetLastError());
//	  CloseHandle(hToken);
//    return FALSE;
//  }

  CloseHandle(hToken); // IF WE Get Here - Nothing Went Wrong.
//  MessageBox(0, "Nailed IT!!!", "Sync Time Perms", MB_OK);
 return TRUE; // Enable The Time Sync Button/Feature.
}
/* I'm Leaving This Code Here Only as a Point Of Reference on Where I Gave-up on doing Mid-Process Login.
#pragma comment(lib, "Credui.lib")
  //===============================================================================================
 #include <wincred.h> //===========================================================================
//-----------+++--> Prompt User For Administrative Rights to (Set the System Time) Sync the Clock):
BOOL RequestAdminRights() { //--------------------------------------------------------------+++-->
	TCHAR szDomain[CREDUI_MAX_DOMAIN_TARGET_LENGTH+1]; // Parsed Out Domain Name for Login
	TCHAR pszName[CREDUI_MAX_USERNAME_LENGTH+1]; // Full User@Domain Name From Dialog
	TCHAR szUser[CREDUI_MAX_USERNAME_LENGTH+1]; // Parsed Out UserName for Login
	TCHAR pszPwd[CREDUI_MAX_PASSWORD_LENGTH+1];// PassWord for Login
	char szErr[MIN_BUFF] = {0};
	CREDUI_INFO cui;
	DWORD dwErr;
	BOOL fSave;


 

  cui.cbSize = sizeof(CREDUI_INFO);
  cui.hwndParent = NULL;

  //  Ensure that MessageText and CaptionText identify what credentials to use and which application requires them.
  cui.pszMessageText = TEXT("Enter administrator account information");
  cui.pszCaptionText = TEXT("CredUITest");
  cui.hbmBanner = NULL;
  fSave = FALSE;

  SecureZeroMemory(szDomain, sizeof(szDomain));
  SecureZeroMemory(pszName, sizeof(pszName));
  SecureZeroMemory(szUser, sizeof(szUser));
  SecureZeroMemory(pszPwd, sizeof(pszPwd));

  dwErr = CredUIPromptForCredentials(
//  dwErr = CredUIPromptForWindowsCredentials( // Not Supported in Current Headers.
                 &cui,				 // CREDUI_INFO structure
                 TEXT("T-Clock 2010"), // Target for credentials, usually a server
                 NULL,			   // Reserved
                 0,				  // Reason
                 pszName,		 // User name
                 CREDUI_MAX_USERNAME_LENGTH+1,  // Max number of char for user name
                 pszPwd,	   // Password
                 CREDUI_MAX_PASSWORD_LENGTH+1, // Max number of char for password
                 &fSave,	 // State of save check box
                 CREDUI_FLAGS_GENERIC_CREDENTIALS |  // flags
                 CREDUI_FLAGS_ALWAYS_SHOW_UI |
                 CREDUI_FLAGS_DO_NOT_PERSIST);  

  if(!dwErr) {
	  if(CredUIParseUserName(pszName, szUser, CREDUI_MAX_USERNAME_LENGTH+1, szDomain, CREDUI_MAX_DOMAIN_TARGET_LENGTH+1)) {
		  	  wsprintf(szErr, "CredUIParseUserName() failed with code %d\n", GetLastError());
			  MessageBox(0, szErr, "User Fail!", MB_OK|MB_ICONERROR);
		  return FALSE;
	  }else{
		  HANDLE hToken;					   // LOGON32_LOGON_BATCH - IS NOT a Viable Option.
		  if(!LogonUser(szUser, szDomain, pszPwd, LOGON32_LOGON_NETWORK, LOGON32_PROVIDER_DEFAULT, &hToken)) {
		  	  wsprintf(szErr, "LogonUser() failed with code %d\n", GetLastError());
			  MessageBox(0, szErr, "User Fail!", MB_OK|MB_ICONERROR);
		  return FALSE;
		  }else{







						TOKEN_PRIVILEGES tp;    // token provileges
						TOKEN_PRIVILEGES oldtp; // old token privileges
						DWORD dwSize = sizeof(TOKEN_PRIVILEGES);
						LUID     luid;          

					   // now, set the SE_SYSTEMTIME_NAME privilege to our
					  //  current process, so we can call SetSystemTime()
					 
					  if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
					//  if(!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, TRUE, &hToken)) { FAILS - No Token
						  wsprintf(szErr, "OpenProcessToken() failed with code %d\n", GetLastError());
						  MessageBox(0, szErr, "User Fail!", MB_OK|MB_ICONERROR);
						return FALSE;
					  }
					  
					  if(!LookupPrivilegeValue(NULL, SE_SYSTEMTIME_NAME, &luid)) {
						  wsprintf(szErr, "LookupPrivilege() failed with code %d\n", GetLastError());
						  MessageBox(0, szErr, "User Fail!", MB_OK|MB_ICONERROR);
						  CloseHandle(hToken);
						return FALSE;
					  }

					  ZeroMemory(&tp, sizeof(tp));
					  tp.PrivilegeCount = 1;
					  tp.Privileges[0].Luid = luid;
					  tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED|TOKEN_IMPERSONATE;

					  // Adjust Token privileges
					  AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &oldtp, &dwSize);
					  if(GetLastError() != ERROR_SUCCESS) {
						  CloseHandle(hToken);
						  MessageBox(0, "SE_SYSTEMTIME_NAME Privilege\n Cannot be Enabled!", 
							  "Privilege Failure", MB_OK|MB_ICONERROR);
					//	  RequestAdminRights();
						return FALSE;
					  }





		  }
//	  SetThreadToken(GetCurrentThread(), hToken);
		  if(!ImpersonateLoggedOnUser(hToken)) {
			  wsprintf(szErr, "ImpersonateLoggedOnUser() failed with code %d\n", GetLastError());
			  MessageBox(0, szErr, "User Fail!", MB_OK|MB_ICONERROR);
		  }else{
//			  SyncTimeNow();
//			  MessageBox(0, "Successfully!", "Authenticated:", MB_OK);
		  }
	  }
//	  GetSetTimePermissions();
//	  SyncTimeNow();
	   // TODO: Put code that uses the credentials here.
	  //  When you have finished using the credentials, erase them from memory.
	  SecureZeroMemory(pszName, sizeof(pszName));
	  SecureZeroMemory(pszPwd, sizeof(pszPwd));
    return TRUE;
  }else{
	  MessageBox(0, "There Was a dwError", "Oopsy!", MB_OK);
  }
  return FALSE;
}*/