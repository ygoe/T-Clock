   /*-----------------------------------------------------
  // main.c : API, hook procedure -> KAZUBON 1997-2001
 //-------------------------------------------------------*/
// Modified by Stoic Joker: Tuesday, March 2 2010 - 10:42:42
#include "tcdll.h"

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
void InitClock(HWND hwnd);

/*------------------------------------------------
  shared data among processes
--------------------------------------------------*/
extern HWND hwndTClockMain;
extern HWND hwndClock;
extern HHOOK hhook;

/*------------------------------------------------
  globals
--------------------------------------------------*/
extern WNDPROC oldWndProc;
extern HANDLE hmod;

 //================================================================================================
//-------------------------------------+++--> Entry Point of This (SystemTray Clock ShellHook) DLL:
BOOL WINAPI DllMain(HANDLE hModule, DWORD dwFunction, LPVOID lpNot) { //--------------------+++-->
  hmod = hModule;
  switch(dwFunction) {
	case DLL_PROCESS_ATTACH:
	  break;
	
	case DLL_PROCESS_DETACH:
	  break;
	
	default: break;
  }
 return TRUE;
}
 //================================================================================================
//-------------------------------------------------------+++--> Install SystemTray Clock ShellHook:
void WINAPI HookStart(HWND hwnd) { //-------------------------------------------------------+++-->
	char classname[80] = {0};
	HWND hwndBar, hwndChild;
	HANDLE hThread;
	
  hwndTClockMain = hwnd;
	
	// find the taskbar
  hwndBar = FindWindow("Shell_TrayWnd", NULL);
  if(!hwndBar) {
	 SendMessage(hwnd, WM_USER+1, 0, 1);
	 return;
  }
	
	// get thread ID of taskbar (explorer) - Specal thanks to T.Iwata.
  hThread = (HANDLE)(DWORD_PTR)(DWORD)GetWindowThreadProcessId(hwndBar, NULL);
	  
  if(!hThread) {
	  SendMessage(hwnd, WM_USER+1, 0, 2);
	  return;
  }
	
	// install an hook to thread of taskbar
  hhook = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)CallWndProc, hmod, (DWORD)(DWORD_PTR)(HANDLE)hThread);
  if(!hhook) {
	 SendMessage(hwnd, WM_USER+1, 0, 3);
	 return;
  }
	
	// refresh the taskbar
  PostMessage(hwndBar, WM_SIZE, SIZE_RESTORED, 0);
	
	// find the clock window
  hwndChild = GetWindow(hwndBar, GW_CHILD);
  while(hwndChild) {
		GetClassName(hwndChild, classname, 80);
		if(lstrcmpi(classname, "TrayNotifyWnd") == 0) {
		   hwndChild = GetWindow(hwndChild, GW_CHILD);
		   while(hwndChild) {
				 GetClassName(hwndChild, classname, 80);
				 if(lstrcmpi(classname, "TrayClockWClass") == 0) {
					SendMessage(hwndChild, WM_NULL, 0, 0);
					break;
				 }
		   } break;
		}
	 hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
  }
}
 //================================================================================================
//--------------------------------------------------------+++--> Remove SystemTray Clock ShellHook:
void WINAPI HookEnd(void) { //--------------------------------------------------------------+++-->
	HWND hwnd;
	
	// force the clock to end cunstomizing
  if(hwndClock && IsWindow(hwndClock))
	 SendMessage(hwndClock, WM_COMMAND, 102, 0);
	// uninstall my hook
  if(hhook != NULL) UnhookWindowsHookEx(hhook);
  hhook = NULL;
	
	// refresh the clock
  if(hwndClock && IsWindow(hwndClock)) PostMessage(hwndClock, WM_TIMER, 0, 0);
  hwndClock = NULL;
	
	// refresh the taskbar
  hwnd = FindWindow("Shell_TrayWnd", NULL);
  if(hwnd) {
	 PostMessage(hwnd, WM_SIZE, SIZE_RESTORED, 0);
	 InvalidateRect(hwnd, NULL, TRUE);
  }
}
 //================================================================================================
//-----------------------------------------------------+++--> SystemTray Clock ShellHook Procedure:
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam) { //------------------+++-->
	LPCWPSTRUCT pcwps;
	char classname[80];

  pcwps = (LPCWPSTRUCT)lParam;
  if(nCode >= 0 && pcwps && pcwps->hwnd) { // if this message is sent to the clock
	 if(hwndClock == NULL && oldWndProc == NULL 
		 && GetClassName(pcwps->hwnd, classname, 80) > 0 
		 && lstrcmpi(classname, "TrayClockWClass") == 0) {
		InitClock(pcwps->hwnd); // initialize  cf. wndproc.c
	 }
  }
 return CallNextHookEx(hhook, nCode, wParam, lParam);
}