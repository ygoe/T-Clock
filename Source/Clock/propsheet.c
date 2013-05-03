/*-------------------------------------------
  propsheet.c - Kazubon 1997-2001
  show "properties for TClock"
---------------------------------------------*/
// Modified by Stoic Joker: Monday, 03/22/2010 @ 7:32:29pm
#include "tclock.h"

#define MAX_PAGE  9

void SetMyDialgPos(HWND hwnd);
int CALLBACK PropSheetProc(HWND hDlg, UINT uMsg, LPARAM  lParam);
LRESULT CALLBACK SubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// dialog procedures of each page
BOOL CALLBACK PageAboutProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageAlarmProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageMouseProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageColorProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageQuickyProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageFormatProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageHotKeyProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageQuickyMenuProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageMiscProc(HWND, UINT, WPARAM, LPARAM);

void SetPropSheetPos(HWND hwnd);

static WNDPROC oldWndProc; // to save default window procedure
static int startpage = 0; // page to open first

BOOL g_bApplyTaskbar = FALSE;
BOOL g_bApplyClock = FALSE;
BOOL g_bApplyClear = FALSE;

 //================================================================================================
//----------------------------//-----------------------+++--> Show the (Tab Dialog) Property Sheet:
void MyPropertySheet(void) { //-------------------------------------------------------------+++-->
	PROPSHEETPAGE psp[MAX_PAGE];
	PROPSHEETHEADER psh;  int i;
	DLGPROC PageProc[MAX_PAGE] = { (DLGPROC)PageAboutProc, (DLGPROC)PageAlarmProc,
		  (DLGPROC)PageColorProc, (DLGPROC)PageFormatProc, (DLGPROC)PageMouseProc,
		  (DLGPROC)PageQuickyProc, (DLGPROC)PageQuickyMenuProc, 
		  (DLGPROC)PageHotKeyProc, (DLGPROC)PageMiscProc };

  if(g_hwndSheet && IsWindow(g_hwndSheet)) { // IF Already Open...
	 ForceForegroundWindow(g_hwndSheet); // <--+++--> Stick it on Top!
	 return;
  }
	
   // Allocate Clean Memory for Each Page
  for(i = 0; i < MAX_PAGE; i++) {
	  memset(&psp[i], 0, sizeof(PROPSHEETPAGE));
	  psp[i].dwSize = sizeof(PROPSHEETPAGE);
	  psp[i].pfnDlgProc = PageProc[i];
  }

  psp[0].pszTemplate = MAKEINTRESOURCE(IDD_PAGEABOUT);		// - PageAboutProc
  psp[1].pszTemplate = MAKEINTRESOURCE(IDD_PAGEALARM);		// - PageAlarmProc
  psp[2].pszTemplate = MAKEINTRESOURCE(IDD_PAGECLOCKTEXT);	// - PageColorProc
  psp[3].pszTemplate = MAKEINTRESOURCE(IDD_PAGEFORMAT);		// - PageFormatProc
  psp[4].pszTemplate = MAKEINTRESOURCE(IDD_PAGEMOUSE);		// - PageMouseProc
  psp[5].pszTemplate = MAKEINTRESOURCE(IDD_PAGEQUICKY);		// - PageQuickyProc
  psp[6].pszTemplate = MAKEINTRESOURCE(IDD_PAGETARGETFILE);	// - PageQuickyMenuProc
  psp[7].pszTemplate = MAKEINTRESOURCE(IDD_PAGEHOTKEY);		// - PageHotKeyProc
  psp[8].pszTemplate = MAKEINTRESOURCE(IDD_PAGEMISC);		// - PageMiscProc
  // Need to Add: Miscellaneous Tab.

	// set data of property sheet
  memset(&psh, 0, sizeof(PROPSHEETHEADER));
	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE | PSH_MODELESS | PSH_USECALLBACK;
	psh.hInstance = GetModuleHandle(NULL);
	psh.pszIcon = MAKEINTRESOURCE(IDI_ICON1);
	psh.pszCaption = "T-Clock 2010 Properties";
	psh.nPages = MAX_PAGE;
	psh.nStartPage = startpage;
	psh.ppsp = psp;
	psh.pfnCallback = PropSheetProc;
	
  g_bApplyClock = FALSE;
  g_bApplyTaskbar = FALSE;
	
	// show it !
  g_hwndSheet = (HWND)PropertySheet(&psh);
  ForceForegroundWindow(g_hwndSheet);
}
 //================================================================================================
//--------------------------------------------------------+++--> Property Sheet Callback Procedure:
int CALLBACK PropSheetProc(HWND hDlg, UINT uMsg, LPARAM  lParam) { //-----------------------+++-->
	LONG style;
	
  if(uMsg == PSCB_INITIALIZED) {
	 style = GetWindowLong(hDlg, GWL_EXSTYLE);
	 style ^= WS_EX_CONTEXTHELP; // Hide ? Button
	 SetWindowLong(hDlg, GWL_EXSTYLE, style);
		
		// subclass the window
  oldWndProc = (WNDPROC)(LONG_PTR)GetWindowLongPtr(hDlg, GWL_WNDPROC);
//==================================================================================
#if defined _M_IX86 //---------------+++--> IF Compiling This as a 32-bit Clock Use:
  SetWindowLongPtr(hDlg, GWL_WNDPROC, (LONG)(LRESULT)SubclassProc);

//==================================================================================
#else //-------------------+++--> ELSE Assume: _M_X64 - IT's a 64-bit Clock and Use:
  SetWindowLongPtr(hDlg, GWL_WNDPROC, (LONG_PTR)(LRESULT)SubclassProc);

#endif
//==================================================================================
  SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)g_hIconTClock);
  }
 return 0;
}
/*--------------------------------------------------------
   window procedure of subclassed property sheet
---------------------------------------------------------*/
LRESULT CALLBACK SubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT l;
  switch(message) {

	case WM_SHOWWINDOW: // adjust the window position
	  SetMyDialgPos(hwnd);
	  return FALSE; // Returning FALSE Allows it to Maintain Caret Focus
 }
	
	// default
	l = CallWindowProc(oldWndProc, hwnd, message, wParam, lParam);
	
	switch(message)
	{
		case WM_COMMAND:
		{
			WORD id;
			id = LOWORD(wParam);
			// close the window by "OK" or "Cancel"
			if(id == IDOK || id == IDCANCEL)
			{
				// MyHelp(hwnd, -1);
				startpage = (int)SendMessage(
					(HWND)SendMessage(hwnd, PSM_GETTABCONTROL, 0, 0), TCM_GETCURSEL, 0, 0);
				if(startpage < 0) startpage = 0;
				DestroyWindow(hwnd);
				g_hwndSheet = NULL;
			}
			// apply settings
			if(id == IDOK || id == 0x3021)
			{
				if(g_bApplyClock)
				{
					SendMessage(g_hwndClock, CLOCKM_REFRESHCLOCK, 0, 0);
					g_bApplyClock = FALSE;
				}
				if(g_bApplyClear)
				{
					SendMessage(g_hwndClock, CLOCKM_REFRESHCLEARTASKBAR, 0, 0);
					g_bApplyClear = FALSE;
				}
				if(g_bApplyTaskbar)
				{
					SendMessage(g_hwndClock, CLOCKM_REFRESHTASKBAR, 0, 0);
					g_bApplyTaskbar = FALSE;
				}
			}
			if(id == IDOK || id == IDCANCEL)
			{
						if(g_hDlgTimer && IsWindow(g_hDlgTimer))
							PostMessage(g_hDlgTimer, WM_CLOSE, 0, 0);
						if(g_hDlgCalender && IsWindow(g_hDlgCalender))
							PostMessage(g_hDlgCalender, WM_CLOSE, 0, 0);
			}
			EmptyWorkingSet(GetCurrentProcess());
			break;
		}
		// close by "x" button
		case WM_SYSCOMMAND:
		{
			if((wParam & 0xfff0) == SC_CLOSE)
				PostMessage(hwnd, WM_COMMAND, IDCANCEL, 0);
			break;
		}
	}
	return l;
}

 //================================================================================================
//------------------------------+++--> Adjust the Window Position Based on Taskbar Size & Location:
void SetMyDialgPos(HWND hwnd) { //----------------------------------------------------------+++-->
	int wscreen, hscreen, wProp, hProp;
	int wTray, hTray, x, y;
	RECT rc, rcTray;
	HWND hwndTray;

  GetWindowRect(hwnd, &rc); // Properties Dialog Dimensions
  wProp = rc.right - rc.left;  //----------+++--> Width
  hProp = rc.bottom - rc.top; //----------+++--> Height

  wscreen = GetSystemMetrics(SM_CXSCREEN);  // Desktop Width
  hscreen = GetSystemMetrics(SM_CYSCREEN); // Desktop Height
	
  hwndTray = FindWindow("Shell_TrayWnd", NULL);
  if(hwndTray == NULL) return;

  GetWindowRect(hwndTray, &rcTray);
  wTray = rcTray.right - rcTray.left;
  hTray = rcTray.bottom - rcTray.top;

  if(wTray > hTray) { // IF Width is Greater Than Height, Taskbar is
	  x = wscreen - wProp - 21; // at Either Top or Bottom of Screen
	  if(rcTray.top < hscreen / 2)
		  y = rcTray.bottom + 21; // Taskbar is on Top of Screen
	  else // ELSE Taskbar is Where it Belongs! (^^^Mac Fag?^^^)
		  y = rcTray.top - hProp - 21;
	  if(y < 0) y = 0;
  }else{ //---+++--> ELSE Taskbar is on Left or Right Side of Screen
	  y = hscreen - hProp - 21; // Down is a Fixed Position
	  if(rcTray.left < wscreen / 2)
		  x = rcTray.right + 21; //--+++--> Taskbar is on Left Side of Screen
	  else
		  x = rcTray.left - wProp - 21; // Taskbar is on Right Side of Screen
	  if(x < 0) x = 0;
  }

  MoveWindow(hwnd, x, y, wProp, hProp, FALSE);
}
/*------------------------------------------------
   select file
--------------------------------------------------*/
BOOL SelectMyFile(HWND hDlg, const char *filter, DWORD nFilterIndex, const char *deffile, char *retfile) {
	char fname[MAX_PATH], ftitle[MAX_PATH], initdir[MAX_PATH];
	OPENFILENAME ofn;
	BOOL r;
	
	memset(&ofn, '\0', sizeof(OPENFILENAME));
	
	strcpy(initdir, g_mydir);
	if(deffile[0])
	{
		WIN32_FIND_DATA fd;
		HANDLE hfind;
		hfind = FindFirstFile(deffile, &fd);
		if(hfind != INVALID_HANDLE_VALUE)
		{
			FindClose(hfind);
			strcpy(initdir, deffile);
			del_title(initdir);
		}
	}
	
	fname[0] = 0;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hDlg;
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = nFilterIndex;
	ofn.lpstrFile= fname;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = ftitle;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = initdir;
	ofn.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_FILEMUSTEXIST;
	
	r = GetOpenFileName(&ofn);
	if(!r) return r;
	
	strcpy(retfile, ofn.lpstrFile);
	
 return r;
}