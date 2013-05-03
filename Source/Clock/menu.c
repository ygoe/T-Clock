  //================================================================================
 //--+++--> menu.c - pop-up menu on right button click - KAZUBON 1997-2001 =========
//================= Last Modified by Stoic Joker: Wednesday, 12/22/2010 @ 11:29:24pm
#include "tclock.h" //---------------{ Stoic Joker 2006-2010 }---------------+++-->

#define QM_COMMAND 3800
void UpdateTimerMenu(HMENU hMenu);

/*-----------------------------------------------------------------
 ----------------  when the clock is right-clicked show pop-up menu
-----------------------------------------------------------------*/
void OnContextMenu(HWND hWnd, HWND hwndClicked, int xPos, int yPos) {
	BOOL g_bQMDisplay = TRUE;
	BOOL g_bQMExitWin = TRUE;
	BOOL g_bQMLaunch = TRUE;
	HMENU hPopupMenu = NULL;
	BOOL g_bQMAudio = TRUE;
	BOOL g_bQMNet = TRUE;
	HMENU hMenu = NULL;

  g_bQMDisplay = GetMyRegLong("QuickyMenu", "DisplayProperties", TRUE);
  g_bQMExitWin = GetMyRegLong("QuickyMenu", "ExitWindows",       TRUE);
  g_bQMLaunch  = GetMyRegLong("QuickyMenu", "QuickyMenu",        TRUE);
  g_bQMAudio   = GetMyRegLong("QuickyMenu", "AudioProperties",   TRUE);
  g_bQMNet     = GetMyRegLong("QuickyMenu", "NetworkDrives",     TRUE);

  hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MENU));
  hPopupMenu = GetSubMenu(hMenu, 0);

  if(!g_bQMDisplay) DeleteMenu(hPopupMenu, 16, MF_BYPOSITION);
  if(!g_bQMExitWin) DeleteMenu(hPopupMenu, 13, MF_BYPOSITION);
	  // Timers Menu Item y/n Goes HERE!!!
  if(!g_bQMLaunch)  DeleteMenu(hPopupMenu, 11, MF_BYPOSITION);
  if(!g_bQMNet)	    DeleteMenu(hPopupMenu, 10, MF_BYPOSITION);
  if(!g_bQMAudio)   DeleteMenu(hPopupMenu,  9, MF_BYPOSITION);

  UpdateTimerMenu(hPopupMenu); // Get the List of Active Timers.

  if(g_bQMLaunch) {
	  char szmItem[TNY_BUFF] = {0};
	  UINT uItemID = QM_COMMAND;
	  char s[TNY_BUFF] = {0};
	  int iMenu;

	  MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
	  mii.fMask = MIIM_STRING | MIIM_ID;

	  for(iMenu = 0; iMenu <= 11; iMenu++) {
		  wsprintf(szmItem, "MenuItem-%d", iMenu);

		  if(GetMyRegLong("QuickyMenu\\MenuItems", szmItem, FALSE)) {
			  wsprintf(szmItem, "MenuItem-%d-Text", iMenu);
			  GetMyRegStr("QuickyMenu\\MenuItems", szmItem, s, TNY_BUFF, "");
			  mii.dwTypeData = s;
			  mii.wID = uItemID;
			  InsertMenuItem(hPopupMenu, IDC_SHOWCALENDER, FALSE, &mii);
		  }
		  uItemID++;
	  }
  }

  ForceForegroundWindow(hWnd);
  TrackPopupMenu(hPopupMenu, TPM_NONOTIFY|TPM_LEFTBUTTON, xPos, yPos, 0, hWnd, NULL);
  DestroyMenu(hMenu); // Starting Over is Simpler & Recommended
}
 //================================================================================================
//--------------------------------------+++--> Show/Hide Desktop (e.g. Show/Hide all Open Windows):
void ToggleDesk() { //----------------------------------------------------------------------+++-->
	IShellDispatch4* pDisp;
	HRESULT hres;

  CoInitialize(NULL);

  hres = CoCreateInstance(&CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, &IID_IShellDispatch4, &pDisp);

  if(SUCCEEDED(hres)) {
	pDisp->lpVtbl->ToggleDesktop(pDisp);
    pDisp->lpVtbl->Release(pDisp);

  }
  CoUninitialize();
}
 //================================================================================================
//-----------------------------------------------------+++--> T-Clock Menu Command Message Handler:
void OnTClockCommand(HWND hwnd, WORD wID, WORD wCode) { //----------------------------------+++-->
  switch(wID) {
	case IDC_REFRESHTCLOCK: //-----+++--> RePaint & Size the T-Clock Display Window
	  RefreshUs();
	  return;
	
	case IDC_SHOWPROP: //---------------------+++--> Show T-Clock Properties Dialog
	  MyPropertySheet();
	  return;

	case IDM_SYNCTIME:
		SyncTimeNow();
	  return;

	case JRMSG_BOING:
		ReleaseTheHound(hwnd, TRUE);
		return;
	
	case IDC_EXIT: //--------------------------------------+++--> Exit T-Clock 2010
	  PostMessage(g_hwndClock, WM_COMMAND, IDC_EXIT, 0);
	  return;
	
	case IDC_SHOWCALENDER: //-------------------------------+++--> Display Calender
	  DialogCalender(hwnd);
	  return;
	
	case IDC_DISPLAYPROP: //------------------------------+++--> Display Properties
	  WinExec(("control.exe desk.cpl, display,1"),SW_SHOW);
	  return;
//========================================================================
#if defined _M_IX86 //-----+++--> IF Compiling This as a 32-bit Clock Use:
  #define OPEN_VOLUME "sndvol32.exe"

#else //---------+++--> ELSE Assume: _M_X64 - IT's a 64-bit Clock and Use:
  #define OPEN_VOLUME "sndvol.exe"

#endif
//========================================================================
	case IDC_VOLUMECONTROL: //-------------------------------+++--> Volume Controls
	  WinExec((OPEN_VOLUME),SW_SHOW);
	  return;
	
	case IDC_AUDIOPROP: //----------------------------------+++--> Audio Properties
	  WinExec(("control.exe mmsys.cpl"),SW_SHOW);
	  return;
	
	case IDC_MAPDRIVE: //----------------------------------+++--> Map Network Drive
	  WNetConnectionDialog(hwnd, RESOURCETYPE_DISK);
	  return;
	
	case IDC_DISCONNECT: //-------------------------+++--> Disconnect Network Drive
	  WNetDisconnectDialog(hwnd, RESOURCETYPE_DISK);
	  return;
	
	case IDC_TOGGLE_DT: //---------------------------+++--> Show / Hide the Desktop
	  ToggleDesk();
	  return;
	
	case IDC_QUICKY_WINEXP: { //-----------------//--+++--> Windows Explorer Opened
	  ShellExecute(hwnd, "open","Explorer.exe", //-> Correctly at My Computer Level
				"/e, ::{20D04FE0-3AEA-1069-A2D8-08002B30309D}",NULL,SW_SHOWNORMAL);
	  return;
	}

	case IDC_QUICKY_DOS: { // Command Prompt
	  char RooT[MAX_PATH];
	  GetWindowsDirectory(RooT,MAX_PATH);
	  ShellExecute(hwnd, "open","cmd.exe", "/f:on /t:0a", RooT, SW_SHOWNORMAL);
	  return;
	}
	
	case QM_COMMAND:
	case QM_COMMAND+1:
	case QM_COMMAND+2:
	case QM_COMMAND+3:
	case QM_COMMAND+4:
	case QM_COMMAND+5:
	case QM_COMMAND+6:
	case QM_COMMAND+7:
	case QM_COMMAND+8:
	case QM_COMMAND+9:
	case QM_COMMAND+10:
	case QM_COMMAND+11: {
	  char szQM_Temp[260]="";
	  char szQM_Target[260]="";
	  char szQM_Switch[260]="";
	  UINT uQM_cID = (wID - QM_COMMAND);

	  wsprintf(szQM_Temp, "MenuItem-%d-Target", uQM_cID);
	  GetMyRegStr("QuickyMenu\\MenuItems", szQM_Temp, szQM_Target, 260, "");

	  wsprintf(szQM_Temp, "MenuItem-%d-Switches", uQM_cID);
	  GetMyRegStr("QuickyMenu\\MenuItems", szQM_Temp, szQM_Switch, 260, "");

	  ShellExecute(hwnd, "open", szQM_Target, szQM_Switch, NULL, SW_SHOWNORMAL);
	  return;
	}
	
	case IDC_QUICKY_EMPTYRB:
		SHEmptyRecycleBin(0, NULL, SHERB_NOCONFIRMATION);
	  return;
//-----------------------//--------------------------------------------+++-->
	case IDCM_SW_START: //-> These Messages are Bounced From the Command Line
		SendMessage(g_hDlgStopWatch, WM_COMMAND, IDOK, 0); //-+> Through Here
	  return; //-+-> Then to the StopWatch Window - IF/When it is/Gets Opened

	case IDCB_SW_RESET:
		SendMessage(g_hDlgStopWatch, WM_COMMAND, IDCB_SW_RESET, 0);
	  return;

	case IDCB_SW_STOP:
		SendMessage(g_hDlgStopWatch, WM_COMMAND, IDCB_SW_STOP, 0);
	  return;

	case IDCB_SW_LAP:
		SendMessage(g_hDlgStopWatch, WM_COMMAND, IDCB_SW_LAP, 0);
	  return; //------------------------+++--> End of Bounce Through Messages
//-----------//--------------------------------------------------------+++-->
	case IDC_SHUTDOWN:
		if(!ShutDown()) 
			MessageBox(0, "Shutdown Request Failed!", "ERROR:", MB_OK|MB_ICONERROR);
	  return;

	case IDC_REBOOT:
	  if(!ReBoot()) 
		  MessageBox(0, "Reboot Request Failed!", "ERROR:", MB_OK|MB_ICONERROR);
	  return;

	case IDC_LOGOFF:
	  if(!LogOff()) 
		  MessageBox(0, "Logoff Request Failed!", "ERROR:", MB_OK|MB_ICONERROR);
	  return;

	case IDC_TIMER: // Timer
	  DialogTimer(hwnd);
	  return;

	case IDM_STOPWATCH:
		DialogStopWatch(hwnd);
	
	case IDC_MINALL:
	case IDC_DATETIME:
	case IDC_CASCADE:
	case IDC_TILEHORZ:
	case IDC_TILEVERT:
	case IDC_TASKMAN:
	case IDC_TASKBARPROP: {
	  HWND hwndTray = FindWindow("Shell_TrayWnd", NULL);
	  if(hwndTray) PostMessage(hwndTray, WM_COMMAND, (WPARAM)wID, 0);
	  return;
	}

	case IDM_TIMEWATCH:
		WatchTimer(); // HotKey Open/View - It Will However,
	  return; //-++--> Auto-Close if it Isn't Really Needed.

	case ID_T_TIMER1:
	case ID_T_TIMER2:
	case ID_T_TIMER3:
	case ID_T_TIMER4:
	case ID_T_TIMER5:
	case ID_T_TIMER6:
	case ID_T_TIMER7: {
		char szTime[GEN_BUFF] = {0};
		int iFree = 0;
		wID -= ID_T_TIMER1;
		GetTimerInfo(szTime, wID, FALSE);
		WatchTimer(); // Shelter All the Homeless Timers.
		return;
	}
  }
	
  if((IDC_STOPTIMER <= wID && wID < IDC_STOPTIMER + MAX_TIMER)) {
	  StopTimer(hwnd, wID - IDC_STOPTIMER); //-+-> Stop Timer X!
  }
 return;
}
 //================================================================================================
//----------+++--> Enumerate & Display ALL Currently Active Timers on The Running Timers Menu List:
void UpdateTimerMenu(HMENU hMenu) { //------------------------------------------------------+++-->
	int i, t; char s[GEN_BUFF];

  i = ID_T_TIMER1; //--+++--> All Timers Are Inserted (just) Above (ID_T_TIMER1)
  for(t = 0; t <= 7; t++) { //---------+++--> the Running Timers List Menu Item.
	  if(GetTimerInfo(s, t, TRUE) == 0) break; // Get the Timer's Name Only
	  InsertMenu(hMenu, i, MF_BYCOMMAND|MF_STRING, ID_T_TIMER1 + t, s);
  }
}