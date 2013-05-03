   //===============================================================================
  //--+++--> main.c - KAZUBON 1997-2001 ============================================
 //--+++--> WinMain, window procedure, and functions for initializing ==============
//==================== Last Modified by Stoic Joker: Sunday, 03/13/2011 @ 11:54:05am
#include "tclock.h" //---------------{ Stoic Joker 2006-2011 }---------------+++-->
#include <winver.h>
#include <wtsapi32.h>

// Application Global Window Handles
HWND	g_hwndClock;		// clock window
HWND	g_hwndSheet;		// property sheet window
HWND	g_hDlgTimer;		// Timer Dialog Handle
HWND	g_hDlgCalender;		// Calender Dialog Handle
HWND	g_hDlgStopWatch;	// Stopwatch Dialog Handle
HWND	g_hDlgTimerWatch;	// Timer Watch Dialog Handle
HWND	g_hWnd;	 // Main Window Anchor for HotKeys Only!

HICON	g_hIconTClock, g_hIconPlay, g_hIconStop, g_hIconDel, g_hIconLogo;
                            // icons to use frequently
char	g_mydir[MAX_PATH]; // path to tclock.exe

// Make Background of Desktop Icon Text Labels Transparent:
BOOL bTrans2kIcons; //-------+++--> (For Windows 2000 Only)
//-----------------//+++--> UnAvertized EasterEgg Function:

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

char szClassName[] = "TClockMainClass"; // window class name
char szWindowText[] = "TClock";        // caption of the window

void CheckCommandLine(HWND, BOOL);
static void OnTimerMain(HWND hwnd);
void FindTrayServer(HWND hwnd);
static void InitError(int n);
static BOOL CheckTCDLL(void);
static BOOL CheckDLL(char *fname);
static void CheckRegistry(void);
void SetDesktopIconTextBk(void);
static UINT s_uTaskbarRestart = 0;
static BOOL bStartTimer = FALSE;
static int nCountFindingClock = -1;
BOOL bMonOffOnLock = FALSE;
BOOL bV7up = FALSE;
BOOL b2000 = FALSE;

// alarm.c
extern BOOL bPlayingNonstop;

 //================================================================================================
//---------------------------//-----+++--> Find Out If it's Older Then Windows 2000 If it is, Die!:
BOOL CheckSystemVersion() { //--------------------------------------------------------------+++--> 
	OSVERSIONINFOEX osvi; 
  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  if(!GetVersionEx((OSVERSIONINFO *) &osvi)) return FALSE;
  if(osvi.dwMajorVersion >= 6) bV7up = TRUE;
  if((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 0)) b2000 = TRUE;
  if(osvi.dwMajorVersion >= 5) return TRUE;
 return FALSE;
}
 //================================================================================================
//------------------------------+++--> UnRegister the Clock For Login Session Change Notifications:
void UnregisterSession(HWND hwnd) { //--------{ Explicitly Linked for Windows 2000 }--------+++-->
    typedef DWORD (WINAPI *tWTSUnRegisterSessionNotification)(HWND, DWORD);

  tWTSUnRegisterSessionNotification pWTSUnRegisterSessionNotification=0;
  HINSTANCE handle = LoadLibrary("wtsapi32.dll"); // Windows 2000 Does Not Have This .dll
												 // ...Or Support This Feature.
  pWTSUnRegisterSessionNotification = (tWTSUnRegisterSessionNotification) 
	  GetProcAddress(handle,"WTSUnRegisterSessionNotification");

  if(pWTSUnRegisterSessionNotification) {
	  pWTSUnRegisterSessionNotification(hwnd, NOTIFY_FOR_THIS_SESSION);
	  bMonOffOnLock = FALSE;
  }
  FreeLibrary(handle);
}
 //================================================================================================
//--------------------------------+++--> Register the Clock For Login Session Change Notifications:
void RegisterSession(HWND hwnd) { //---------{ Explicitly Linked for Windows 2000 }---------+++-->
    typedef DWORD (WINAPI *tWTSRegisterSessionNotification)(HWND, DWORD);

  tWTSRegisterSessionNotification pWTSRegisterSessionNotification=0;
  HINSTANCE handle = LoadLibrary("wtsapi32.dll"); // Windows 2000 Does Not Have This .dll
												 // ...Or Support This Feature.
  pWTSRegisterSessionNotification = (tWTSRegisterSessionNotification)
	  GetProcAddress(handle,"WTSRegisterSessionNotification");
  
  if(pWTSRegisterSessionNotification) {
	  pWTSRegisterSessionNotification(hwnd, NOTIFY_FOR_THIS_SESSION);
	  bMonOffOnLock = TRUE;
  }
  FreeLibrary(handle);
}

 //================================================================================================
//--------------------------------------------------==-+++--> Entry Point of Program Using WinMain:
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
	BOOL SessionReged = FALSE; // IS Session Registered to Receive Session Change Notifications?
	BOOL bWin2000 = FALSE; // OS Must be Windows 2000 or Higher!
	WNDCLASS wndclass;
	HWND hwnd;	
	MSG msg;	
	
  bWin2000 = CheckSystemVersion(); // Make Sure We're Running Windows 2000 or Newer!
  if(!bWin2000) { //---//---------// If it's Older Then Windows 2000, it is too Old!
	 MessageBox(0, "T-Clock Requires Windows 2000 or Newer OS!\nSorry, Your Computer is To Old to Run This Program", "ERROR: Age Limit", MB_OK|MB_ICONERROR);
	 ExitProcess(1); //---------// Die Laughing...
	 return 0;
  }

	// make sure ObjectBar isn't running -> From Original Code/Unclear if This is Still a Conflict.
  if(FindWindow("ObjectBar Main", "ObjectBar") != NULL) { // However Nobody Has Ever Complained...
	 ExitProcess(1); 
	 return 0;
  }

	// Do Not Allow the Program to Execute Twice!
  hwnd = FindWindow(szClassName, szWindowText);
  FindTrayServer(hwnd);
  if(hwnd != NULL) { // This One Sends Commands to the Instance
	 CheckCommandLine(hwnd, TRUE); // That is Currently Running.
	 ExitProcess(1); return 0;
  }
	
	// get the path where .exe is positioned
  GetModuleFileName(hInstance, g_mydir, MAX_PATH);
  del_title(g_mydir);
	
  CheckRegistry();
  CancelAllTimersOnStartUp();
  if(!CheckTCDLL()) { ExitProcess(1); return 0; }
	
	// Message of the taskbar recreating - Special thanks to Mr.Inuya
  s_uTaskbarRestart = RegisterWindowMessage("TaskbarCreated");
  // Load ALL of the Global Resources
  g_hIconTClock = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
  g_hIconLogo   = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));
  g_hIconPlay = LoadImage(hInstance, MAKEINTRESOURCE(IDI_PLAY), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
  g_hIconStop = LoadImage(hInstance, MAKEINTRESOURCE(IDI_STOP), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
  g_hIconDel  = LoadImage(hInstance, MAKEINTRESOURCE(IDI_DEL), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
  g_hwndSheet = g_hDlgTimer = g_hDlgCalender = NULL;
	
	// register a window class
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = g_hIconTClock;
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = szClassName;
	RegisterClass(&wndclass);
	
	// create a hidden window
  hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, szClassName, szWindowText, 
								0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);

  CheckCommandLine(hwnd, FALSE); // This Checks for First Instance Startup Options
  g_hWnd = hwnd; // Main Window Anchor for HotKeys Only!

  GetHotKeyInfo(hwnd);
  
  if(!b2000) {
	  bMonOffOnLock = GetMyRegLongEx("Desktop", "MonOffOnLock", FALSE);
	  if(bMonOffOnLock) {
		  RegisterSession(hwnd);
		  SessionReged = TRUE;
	  }
  }

  while(GetMessage(&msg, NULL, 0, 0)) {
		if(g_hwndSheet && IsWindow(g_hwndSheet) && IsDialogMessage(g_hwndSheet, &msg)) ;
		else if(g_hDlgTimer && IsWindow(g_hDlgTimer) && IsDialogMessage(g_hDlgTimer, &msg)) ;
		else if(g_hDlgCalender && IsWindow(g_hDlgCalender) && IsDialogMessage(g_hDlgCalender, &msg)) ;
		else{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
  }

  UnregisterHotKey(hwnd, HOT_TIMER);
  UnregisterHotKey(hwnd, HOT_WATCH);
  UnregisterHotKey(hwnd, HOT_STOPW);
  UnregisterHotKey(hwnd, HOT_PROPR);
  UnregisterHotKey(hwnd, HOT_CALEN);
  UnregisterHotKey(hwnd, HOT_TSYNC);

  if((SessionReged) || (bMonOffOnLock)) UnregisterSession(hwnd);

 ExitProcess((UINT)msg.wParam);
}
         //========================================================================================
        //   /exit	: Exit T-Clock 2010
       //   /prop	: Show T-Clock 2010 Properties
      //   /Sync    : Synchronize the System Clock With an NTP Server
     //   /start	: Start the Stopwatch Counter (open as needed)
    //   /stop	: Stop (pause really) the Stopwatch Counter
   //   /reset	: Reset Stopwatch to 0 (stop as needed)
  //   /lap		: Record a (the current) Lap Time
 //================================================================================================
//---------------------------------------------//---------------+++--> T-Clock Command Line Option:
void CheckCommandLine(HWND hwnd, BOOL b2nd) { //--------------------------------------------+++-->
	char *p;

  p = GetCommandLine();
  while(*p) {
		if(*p == '/') {
		   p++;
		   if(_strnicmp(p, "prop", 4) == 0) {
			   PostMessage(hwnd, WM_COMMAND, IDC_SHOWPROP, 0);
			   p += 4;
		   }
		   else if(_strnicmp(p, "exit", 4) == 0) {
			   PostMessage(hwnd, WM_CLOSE, 0, 0);
			   p += 4;
		   }
		   else if(_strnicmp(p, "Start", 5) == 0) {
			   if(!IsWindow(g_hDlgStopWatch))
				   PostMessage(hwnd, WM_COMMAND, IDM_STOPWATCH, 0);
			   SendMessage(hwnd, WM_COMMAND, IDCM_SW_START, 0);
			   p += 5;
		   }
		   else if(_strnicmp(p, "Stop", 4) == 0) {
			   SendMessage(hwnd, WM_COMMAND, IDCB_SW_STOP, 0);
			   p += 4;
		   }
		   else if(_strnicmp(p, "Lap", 3) == 0) {
			   SendMessage(hwnd, WM_COMMAND, IDCB_SW_LAP, 0);
			   p += 3;
		   }
		   else if(_strnicmp(p, "Reset", 5) == 0) {
			   SendMessage(hwnd, WM_COMMAND, IDCB_SW_RESET, 0);
			   p += 5;
		   }
		   else if(_strnicmp(p, "SyncOpt", 7) == 0) {
			   NetTimeConfigDialog();
			   p += 7;
		   }
		   else if(_strnicmp(p, "Sync", 4) == 0) {
			   if(!b2nd) {
				   MessageBox(0, 
					   TEXT("T-Clock Must be Running for Time Synchronization to Succeed\n"
							"T-Clock Can Not be Started With the /Sync Switch"),
							"ERROR: Time Sync Failure", MB_OK|MB_ICONERROR);
				   SendMessage(hwnd, WM_COMMAND, IDC_EXIT, 0);
			   }else{
				   SyncTimeNow();
				   p += 4;
			   }
		   }
		}
		p++;
  }
}
 //================================================================================================
//--------------------------------------------------+++--> The Main Application "Window" Procedure:
LRESULT CALLBACK WndProc(HWND hwnd,	UINT message, WPARAM wParam, LPARAM lParam) { //--------+++-->
  switch(message) {
	case WM_CREATE:
		InitAlarm();  // initialize alarms
		InitFormat(); // initialize a Date/Time format
		SendMessage(hwnd, WM_TIMER, IDTIMER_START, 0);
		SetTimer(hwnd, IDTIMER_MAIN, 1000, NULL);
	  return 0;

	case WM_TIMER:
		if(wParam == IDTIMER_START) {
			if(bStartTimer) KillTimer(hwnd, wParam);
			bStartTimer = FALSE;
			HookStart(hwnd); // install a hook
			nCountFindingClock = 0;
			EmptyWorkingSet(GetCurrentProcess());
		}
		else if(wParam == IDTIMER_MAIN) OnTimerMain(hwnd);
		else if(wParam == IDTIMER_MOUSE) OnTimerMouse(hwnd);
	  return 0;
		
	case WM_DESTROY:
		EndAlarm();
		EndTimer();
		KillTimer(hwnd, IDTIMER_MAIN);
		if(bStartTimer) {
			KillTimer(hwnd, IDTIMER_START);
			bStartTimer = FALSE;
		}else{
			HookEnd();  // uninstall a hook
		}
		PostQuitMessage(0);
	  return 0;

	case WM_ENDSESSION:
		if(wParam) {
			EndAlarm();
			EndTimer();
			if(bStartTimer) {
				KillTimer(hwnd, IDTIMER_START);
				bStartTimer = FALSE;
			}else{
				HookEnd();  // uninstall a hook
			}
		} break;

	case WM_PAINT: {
		HDC hdc;
		PAINTSTRUCT ps;
		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
	  return 0;
	}
		
	case WM_HOTKEY: // Feature Requested From eweoma at DonationCoder.com
		switch(wParam) { // And a Damn Fine Request it Was... :-)
			case HOT_WATCH:
				PostMessage(hwnd, WM_COMMAND, IDM_TIMEWATCH, 0);
				return 0;

			case HOT_TIMER:
				PostMessage(hwnd, WM_COMMAND, IDC_TIMER, 0);
				return 0;

			case HOT_STOPW:
				PostMessage(hwnd, WM_COMMAND, IDM_STOPWATCH, 0);
				return 0;

			case HOT_PROPR:
				PostMessage(hwnd, WM_COMMAND, IDC_SHOWPROP, 0);
				return 0;

			case HOT_CALEN:
				PostMessage(hwnd, WM_COMMAND, IDC_SHOWCALENDER, 0);
				return 0;

			case HOT_TSYNC:
				SyncTimeNow();
				return 0;

		} return 0;
		
	//==================================================
	case WM_USER: // Messages sent/posted from TCDLL.dll
			nCountFindingClock = -1;
			g_hwndClock = (HWND)lParam;
			return 0;

		case (WM_USER+1):   // error
			nCountFindingClock = -1;
			InitError((int)lParam);
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;

		case (WM_USER+2):   // exit
			if(g_hwndSheet && IsWindow(g_hwndSheet))
				PostMessage(g_hwndSheet, WM_CLOSE, 0, 0);
			if(g_hDlgTimer && IsWindow(g_hDlgTimer))
				PostMessage(g_hDlgTimer, WM_CLOSE, 0, 0);
			if(g_hDlgCalender && IsWindow(g_hDlgCalender))
				PostMessage(g_hDlgCalender, WM_CLOSE, 0, 0);
			if(g_hDlgStopWatch && IsWindow(g_hDlgStopWatch))
				PostMessage(g_hDlgStopWatch, WM_CLOSE, 0, 0);
			g_hwndSheet = NULL;
			g_hDlgTimer = NULL;
			g_hDlgCalender = NULL;
			g_hDlgStopWatch = NULL;
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;

		case MM_MCINOTIFY:
			OnMCINotify(hwnd);
			return 0;

		case MM_WOM_DONE: // stop playing wave
		case (WM_USER+3):
			StopFile();
			return 0;

		case WM_WININICHANGE:
		{
				HWND hwndBar;
				HWND hwndChild;
				char classname[80];

				hwndBar = FindWindow("Shell_TrayWnd", NULL);

				// find the clock window
				hwndChild = GetWindow(hwndBar, GW_CHILD);
				while(hwndChild)
				{
					GetClassName(hwndChild, classname, 80);
					if(lstrcmpi(classname, "TrayNotifyWnd") == 0)
					{
						hwndChild = GetWindow(hwndChild, GW_CHILD);
						while(hwndChild)
						{
							GetClassName(hwndChild, classname, 80);
							if(lstrcmpi(classname, "TrayClockWClass") == 0)
							{
								SendMessage(hwndChild, CLOCKM_REFRESHTASKBAR, 0, 0);
								break;
							}
						}
						break;
					}
					hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
				}
			return 0;
		}
		case WM_SYSCOLORCHANGE:
			PostMessage(hwnd, WM_USER+10, 1,0);
			return 0;

		// context menu
		case WM_COMMAND:
			OnTClockCommand(hwnd, LOWORD(wParam), HIWORD(wParam)); // menu.c
			return 0;

	// messages transfered from the dll
	case WM_CONTEXTMENU: //-------------------------------------- menu.c
	  OnContextMenu(hwnd, (HWND)wParam, LOWORD(lParam), HIWORD(lParam));
	  return 0;

	case WM_DROPFILES: //------ mouse.c
	  OnDropFiles(hwnd, (HDROP)wParam);
	  return 0;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	  if(!bPlayingNonstop) PostMessage(hwnd, WM_USER+3, 0, 0);
	case WM_LBUTTONUP: // <^ Code is Designed to "Fall Through" Here.
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	  OnMouseMsg(hwnd, message, wParam, lParam); // mouse.c
	  return 0;

	case WM_WTSSESSION_CHANGE:
		switch(wParam) {
			case WTS_SESSION_LOCK:
				Sleep(500); // Eliminate user's interaction for 500 ms
				SendMessage(HWND_BROADCAST, WM_SYSCOMMAND,SC_MONITORPOWER, (LPARAM) 2);
				return 0;
        }
  }

  if(message == s_uTaskbarRestart) { // IF the Explorer Shell Crashes,
	 HookEnd();                     //  and the taskbar is recreated.
	 SetTimer(hwnd, IDTIMER_START, 1000, NULL);
	 bStartTimer = TRUE;
  }
 return DefWindowProc(hwnd, message, wParam, lParam);
}

/*---------------------------------------------------------
-- show a message when TClock failed to customize the clock
---------------------------------------------------------*/
void InitError(int n) {
	char s[160];
	
  wsprintf(s, "%s: %d", MyString(IDS_NOTFOUNDCLOCK), n);
  MyMessageBox(NULL, s, "Error", MB_OK, MB_ICONEXCLAMATION);
}
/*---------------------------------------------------------
---- Main Timer -------------------------------------------
---- synchronize, alarm, timer, execute Desktop Calendar...
---------------------------------------------------------*/
static int hourLast = -1, minuteLast = -1;
static int daySaved = -1;
 //================================================================================================
//-----------------------------------+++--> Values Above Are Required by Main Timer Function Below:
void OnTimerMain(HWND hwnd) { //------------------------------------------------------------+++-->
	SYSTEMTIME st;
	BOOL b = TRUE;
	
  GetLocalTime(&st); // Allow OnTimerAlarm(...) to Fire once Every 60 Seconds
  if(hourLast == (int)st.wHour && minuteLast == (int)st.wMinute) b = FALSE;
  
  hourLast = st.wHour;
  minuteLast = st.wMinute;
  if(daySaved >= 0 && st.wDay != daySaved) ;
  else daySaved = st.wDay;
	
  if(b) OnTimerAlarm(hwnd, &st); // alarm.c
  OnTimerTimer(hwnd, &st); // timer.c
  if(b) SetDesktopIconTextBk();
	
	// the clock window exists ?
  if(0 <= nCountFindingClock && nCountFindingClock < 20) nCountFindingClock++;
  else if(nCountFindingClock == 20) nCountFindingClock++;
}
 //================================================================================================  
//-----------------------//----------------------------+++--> Check the File Version of tClock.dll:
BOOL CheckTCDLL(void) { //------------------------------------------------------------------+++-->
	char fname[MAX_PATH];
  strcpy(fname, g_mydir); add_title(fname, "tClock.dll");
 return CheckDLL(fname);
}
 //================================================================================================  
//----------------------------//--------+++--> Verify the Correct Version of tClock.dll is Present:
BOOL CheckDLL(char *fname) { //-----------------------------{ 2.0.1.81 }--------------------+++-->
	DWORD size;
	char *pBlock;
	char szVersion[32] = {0};
	VS_FIXEDFILEINFO *pffi;
	BOOL br = FALSE;
	
	size = GetFileVersionInfoSize(fname, 0);
	if(size > 0) {
		pBlock = malloc(size);
		if(GetFileVersionInfo(fname, 0, size, pBlock)) {
			UINT tmp;
			if(VerQueryValue(pBlock, "\\\0", &pffi, &tmp)) {
				if(HIWORD(pffi->dwFileVersionMS) == 2 &&
				   LOWORD(pffi->dwFileVersionMS) == 0 &&
				   HIWORD(pffi->dwFileVersionLS) == 1 &&
				   LOWORD(pffi->dwFileVersionLS) == 81) {
					br = TRUE; //--+++--> Correct tClock.dll File Version Found!
				}else{
					wsprintf(szVersion, "Version: %d.%d.%d.%d",
						HIWORD(pffi->dwFileVersionMS), 
						LOWORD(pffi->dwFileVersionMS),
						HIWORD(pffi->dwFileVersionLS), 
						LOWORD(pffi->dwFileVersionLS));
				}
			}
		}
		free(pBlock);
	}
	if(!br) {
		char msg[MAX_PATH+30];
		strcpy(msg, "Invalid file version: ");
		get_title(msg + strlen(msg), fname);
		MyMessageBox(NULL, msg,
			szVersion, MB_OK, MB_ICONEXCLAMATION);
	}
  return br;
}
 //================================================================================================
//------------+++--> Initialize With (or set default) Clock.exe Path & Chosen Font Registy Entries:
void CheckRegistry(void) { //---------------------------------------------------------------+++-->
	char s[80];
	
  SetMyRegStr(NULL, "ExePath", g_mydir);
  GetMyRegStr("Clock", "Font", s, 80, "");

  //--------------+++--> This is For Windows 2000 Only - EasterEgg Function:
  bTrans2kIcons = GetMyRegLongEx("Desktop", "Transparent2kIconText", FALSE);

  if(s[0] == 0) {
	 HFONT hfont;
	 LOGFONT lf;
	 hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	 if(hfont) {
		GetObject(hfont, sizeof(lf),(LPVOID)&lf);
		SetMyRegStr("Clock", "Font", lf.lfFaceName);
	 }
  }
}
 //================================================================================================
//----------+++--> Make Background of Desktop Icon Text Labels Transparent (For Windows 2000 Only):
void SetDesktopIconTextBk(void) { //--------------------------------------------------------+++-->
	COLORREF col;
	HWND hwnd;

	if(bTrans2kIcons) {
		hwnd = FindWindow("Progman", "Program Manager");
		if(!hwnd) return;
		hwnd = GetWindow(hwnd, GW_CHILD);
		hwnd = GetWindow(hwnd, GW_CHILD);
		while(hwnd) {
			char s[80];
			GetClassName(hwnd, s, 80);
			if(lstrcmpi(s, "SysListView32") == 0) break;
			hwnd = GetWindow(hwnd, GW_HWNDNEXT);
		}
		if(!hwnd) return;
	}
	else return;
	
	if(bTrans2kIcons) {
		if(ListView_GetTextBkColor(hwnd) == CLR_NONE) return;
		col = CLR_NONE;
	}else{
		if(ListView_GetTextBkColor(hwnd) != CLR_NONE) return;
		col = GetSysColor(COLOR_DESKTOP);
	}

	ListView_SetTextBkColor(hwnd, col);
	ListView_RedrawItems(hwnd, 0, ListView_GetItemCount(hwnd));
	
	hwnd = GetParent(hwnd);
	hwnd = GetWindow(hwnd, GW_CHILD);
	while(hwnd) {
		InvalidateRect(hwnd, NULL, TRUE);
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
}
 //================================================================================================
//--------------------------------------------------+++--> Force a ReDraw of T-Clock & the TaskBar:
void RefreshUs(void) { //-------------------------------------------------------------------+++-->
	char classname[GEN_BUFF] = {0};
	HWND hwndBar, hwndChild;

  hwndBar = FindWindow("Shell_TrayWnd", NULL);

	// find the clock window
  hwndChild = GetWindow(hwndBar, GW_CHILD);
  while(hwndChild) {
		GetClassName(hwndChild, classname, 80);
		if(lstrcmpi(classname, "TrayNotifyWnd") == 0) {
		   hwndChild = GetWindow(hwndChild, GW_CHILD);
		   while(hwndChild) {
				 GetClassName(hwndChild, classname, 80);
				 if(lstrcmpi(classname, "TrayClockWClass") == 0) {
					SendMessage(hwndChild, CLOCKM_REFRESHCLOCK, 0, 0);
					SendMessage(hwndChild, CLOCKM_REFRESHTASKBAR, 0, 0);
					break;
				 }
		   } break;
		}
	  hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
  }
}
 //================================================================================================
//-----------------------+++--> Go Find the Default Windows Clock Window - So We Can Assimilate it:
void FindTrayServer(HWND hwnd) { //---------------------------------------------------------+++-->
  HWND  hwndTrayServer = FindWindow("Shell_TrayWnd", "CTrayServer");
  if(hwndTrayServer > 0) SendMessage(hwndTrayServer, WM_CLOSE, 0, 0);
}