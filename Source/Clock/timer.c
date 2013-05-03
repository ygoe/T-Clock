/*-------------------------------------------
  timer.c - Kazubon 1998-1999
---------------------------------------------*/
// Last Modified by Stoic Joker: Sunday, 03/13/2011 @ 11:54:05am
#include "tclock.h"

char szTimersSubKey[] = "Timers";

// Structure for Timer Setting
typedef struct _tagTimerStruct
{
	char name[GEN_BUFF];
	int id;
	int second;
	int minute;
	int hour;
	int day;
	BOOL bActive;
	char fname[MAX_BUFF];
	BOOL bRepeat;
	BOOL bBlink;
} TIMERSTRUCT;
typedef TIMERSTRUCT* PTIMERSTRUCT;

// Structure for Active Timers
typedef struct _tagTimerStruct2
{						  // Note: ALL Time Increment Fields Are Converted
	char name[GEN_BUFF]; // to Seconds Before Being Set as an Active Timer
	int id;				// Second  = 1 Second
	BOOL bHomeless;		// Minute = 60 Seconds
	DWORD seconds;		// Hour = 3600 Seconds
	DWORD tickonstart;	// Day = 86400 Seconds
} TIMERSTRUCT2;
typedef TIMERSTRUCT2* PTIMERSTRUCT2;

static void OnOK(HWND hDlg);
static void OnDel(HWND hDlg);
static void OnInit(HWND hDlg);
static void OnDestroy(HWND hDlg);
static void OnStopTimer(HWND hDlg);
static void OnTimerName(HWND hDlg);
static void Ring(HWND hwnd, int id);
static void OnTest(HWND hDlg, WORD id);
static void OnSanshoAlarm(HWND hDlg, WORD id);

static int nTimerCount = 0;
static PTIMERSTRUCT2 pTimersWorking = NULL; // Array of Currently Active Timers

void UpdateNextCtrl(HWND, int, int, BOOL);
void ValidateTimerInput(int seconds, int minutes, int hours, int days);
BOOL CALLBACK DlgProcTimer(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
  //=========================================================================================*
 // ------------------------------------------------------------- Open Add/Edit Timers Dialog
//===========================================================================================*
void DialogTimer(HWND hwnd) {
  if(g_hDlgTimer && IsWindow(g_hDlgTimer));
  else g_hDlgTimer = CreateDialog(0, MAKEINTRESOURCE(IDD_TIMER), NULL, (DLGPROC)DlgProcTimer);
  ForceForegroundWindow(g_hDlgTimer);
}
  //==============================================================================*
 // ---------------------------------- Dialog Procedure for Add/Edit Timers Dialog
//================================================================================*
BOOL CALLBACK DlgProcTimer(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	WORD id, code;

  id = LOWORD(wParam);
  code = HIWORD(wParam);

  switch(message) {
	case WM_INITDIALOG:
	  SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)g_hIconTClock);
	  OnInit(hDlg);
	  SetMyDialgPos(hDlg);
	  return TRUE;

	case WM_COMMAND: {
	  switch(id) {
		case IDC_TIMERNAME:
		  if(code == CBN_EDITCHANGE) OnTimerName(hDlg);
		  else if(code == CBN_SELCHANGE) PostMessage(hDlg, WM_COMMAND, MAKELONG(id, CBN_EDITCHANGE), 0);
		  break;

		case IDC_TIMERDEL:
		  OnDel(hDlg);
		  break;

		case IDCB_STOPTIMER:
		  OnStopTimer(hDlg);
		  break;

		case IDC_TIMERSANSHO:
		  OnSanshoAlarm(hDlg, id);
		  break;
		
		case IDC_TIMERTEST:
		  OnTest(hDlg, id);
		  break;
		
		case IDOK:
		  OnOK(hDlg);
		
		case IDCANCEL:
		  DestroyWindow(hDlg);
	  }
	 return TRUE;
	}

  //--------------------------------------------------------------------------+++-->
	case WM_NOTIFY: { //========================================== BEGIN WM_NOTIFY:
//----------------------------------------------------------------------------+++-->
	  if(((LPNMHDR)lParam)->code == UDN_DELTAPOS) {
			char szTime[TNY_BUFF] = {0};
			LPNMUPDOWN lpnmud;
			int i;

		  lpnmud = (LPNMUPDOWN)lParam;
		  if(lpnmud->iDelta > 0) { // User Selected the Up Arrow
			switch(id) { //--+++--> on One of the Timer Controls.
				case IDC_TIMERSECSPIN:
					i = GetDlgItemInt(hDlg, IDC_TIMERSECOND, NULL, TRUE);
					if(i == 59)
						UpdateNextCtrl(hDlg, IDC_TIMERMINSPIN, IDC_TIMERMINUTE, TRUE); 
					break;

				case IDC_TIMERMINSPIN:
					i = GetDlgItemInt(hDlg, IDC_TIMERMINUTE, NULL, TRUE);
					if(lpnmud->iDelta == 4) {
						if(i < 59)
							SetDlgItemInt(hDlg, IDC_TIMERMINUTE, i+1, TRUE);
					}
					if(i == 59)
						UpdateNextCtrl(hDlg, IDC_TIMERHORSPIN, IDC_TIMERHOUR, TRUE);
					break;

				case IDC_TIMERHORSPIN:
					i = GetDlgItemInt(hDlg, IDC_TIMERHOUR, NULL, TRUE);
					if(lpnmud->iDelta == 4) {
						if(i < 23)
							SetDlgItemInt(hDlg, IDC_TIMERHOUR, i+1, TRUE);
					}
					if(i == 23)
						UpdateNextCtrl(hDlg, IDC_TIMERDAYSPIN, IDC_TIMERDAYS, TRUE);
					break;

				case IDC_TIMERDAYSPIN:
					if(lpnmud->iDelta == 4) {
						i = GetDlgItemInt(hDlg, IDC_TIMERDAYS, NULL, TRUE);
						if(i < 7)
							SetDlgItemInt(hDlg, IDC_TIMERDAYS, i+1, TRUE);
					} break;
			}
		  }else{ //--+++--> User Selected the Down Arrow
			switch(id) { // on One of the Timer Controls.
				case IDC_TIMERSECSPIN:
					if(lpnmud->iDelta == -4) {
						i = GetDlgItemInt(hDlg, IDC_TIMERSECOND, NULL, TRUE);
						if(i > 0)
							SetDlgItemInt(hDlg, IDC_TIMERSECOND, i -1, TRUE);
					} break;

				case IDC_TIMERMINSPIN:
					i = GetDlgItemInt(hDlg, IDC_TIMERMINUTE, NULL, TRUE);
					if(lpnmud->iDelta == -4) {
						if(i > 0)
							SetDlgItemInt(hDlg, IDC_TIMERMINUTE, i -1, TRUE);
					}
					if(i == 0)
						UpdateNextCtrl(hDlg, IDC_TIMERSECSPIN, IDC_TIMERSECOND, FALSE);
					break;

				case IDC_TIMERHORSPIN:
					i = GetDlgItemInt(hDlg, IDC_TIMERHOUR, NULL, TRUE);
					if(lpnmud->iDelta == -4) {
						if(i > 0)
							SetDlgItemInt(hDlg, IDC_TIMERHOUR, i -1, TRUE);
					}
					if(i == 0)
						UpdateNextCtrl(hDlg, IDC_TIMERMINSPIN, IDC_TIMERMINUTE, FALSE);
					break;

				case IDC_TIMERDAYSPIN:
					i = GetDlgItemInt(hDlg, IDC_TIMERDAYS, NULL, TRUE);
					if (i == 0)
						UpdateNextCtrl(hDlg, IDC_TIMERHORSPIN, IDC_TIMERHOUR, FALSE);
					break;
			}
		  }
	  }
//----------------------------------------------------------------------------+++-->
	  return TRUE; //=============================================== END WM_NOTIFY:
	} //----------------------------------------------------------------------+++-->

	case WM_DESTROY:
	  StopFile();
	  OnDestroy(hDlg);
	  g_hDlgTimer = NULL;
	  break;
	
	case MM_MCINOTIFY:
	case MM_WOM_DONE:
	  StopFile();
	  SendDlgItemMessage(hDlg, IDC_TIMERTEST, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconPlay);
	return TRUE;
  }
 return FALSE;
}
 //================================================================================================
//------------------------//----------------------------------+++--> Initialize the "Timer" Dialog:
void OnInit(HWND hDlg) { //-----------------------------------------------------------------+++-->
	HFONT hfont;
	char subkey[TNY_BUFF];
	int i, count;
	
	hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	if(hfont) {
	   SendDlgItemMessage(hDlg, IDC_TIMERNAME, WM_SETFONT, (WPARAM)hfont, 0);
	   SendDlgItemMessage(hDlg, IDC_TIMERFILE, WM_SETFONT, (WPARAM)hfont, 0);
	}
	
	SendDlgItemMessage(hDlg, IDC_TIMERSECSPIN, UDM_SETRANGE, 0, MAKELONG(59, 0)); // 60 Seconds Max
	SendDlgItemMessage(hDlg, IDC_TIMERMINSPIN, UDM_SETRANGE, 0, MAKELONG(59, 0)); // 60 Minutes Max
	SendDlgItemMessage(hDlg, IDC_TIMERHORSPIN, UDM_SETRANGE, 0, MAKELONG(23, 0)); // 24 Hours Max
	SendDlgItemMessage(hDlg, IDC_TIMERDAYSPIN, UDM_SETRANGE, 0,  MAKELONG(7, 0)); //  7 Days Max
	
	count = GetMyRegLong(szTimersSubKey, "NumberOfTimers", 0);
	for(i = 0; i <= count; i++)
	{ // count is +1'ed to Make Room for the Dummy Item.
		PTIMERSTRUCT pts;
		int index;
		
		pts = (PTIMERSTRUCT)malloc(sizeof(TIMERSTRUCT));
		wsprintf(subkey, "%s\\Timer%d", szTimersSubKey, i + 1);
		if(i < count) // Wait for the last (extra) Pass, and Then...
			GetMyRegStr(subkey, "Name", pts->name, GEN_BUFF, "");
		else //-----//----------------------+++--> Insert the Dummy!
			strcpy(pts->name, "New Timer");
		pts->id = GetMyRegLong(subkey, "ID", 0);
		pts->second = GetMyRegLong(subkey, "Seconds",  0);
		pts->minute = GetMyRegLong(subkey, "Minutes", 10);
		pts->hour   = GetMyRegLong(subkey, "Hours",    0);
		pts->day    = GetMyRegLong(subkey, "Days",     0);
		GetMyRegStr(subkey, "File", pts->fname, MAX_BUFF, "");
		pts->bBlink = GetMyRegLong(subkey, "Blink", FALSE);
		pts->bRepeat = GetMyRegLong(subkey, "Repeat", FALSE);
		if(i < count) // Waiting for the (Dummy) Last One...
			pts->bActive = GetMyRegLong(subkey, "Active", FALSE);
		else // The New Timer Dummy Menu Item MUST Always Remain FALSE!
			pts->bActive = FALSE;
		index = (int)(LRESULT)CBAddString(hDlg, IDC_TIMERNAME, (LPARAM)pts->name);
		CBSetItemData(hDlg, IDC_TIMERNAME, index, (LPARAM)pts);
	}
	if(count > 0)
		CBSetCurSel(hDlg, IDC_TIMERNAME, 0);
	else
		SendDlgItemMessage(hDlg, IDC_TIMERMINSPIN, UDM_SETPOS, 0, 10);
	OnTimerName(hDlg);
	SendDlgItemMessage(hDlg, IDC_TIMERTEST, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconPlay);
	SendDlgItemMessage(hDlg, IDC_TIMERDEL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconDel);
}
/*------------------------------------------------
  free memories associated with combo box.
--------------------------------------------------*/
void OnDestroy(HWND hDlg) {
	int i, count;
	
  count = (int)(LRESULT)CBGetCount(hDlg, IDC_TIMERNAME);
  for(i = 0; i < count; i++) {
	  PTIMERSTRUCT pts;
	  pts = (PTIMERSTRUCT)CBGetItemData(hDlg, IDC_TIMERNAME, i);
	  free(pts);
  }
}
 //================================================================================================
//--{ START TIMER }-----//-----------------+++--> Called When "OK" Button is Clicked (Start Timer):
void OnOK(HWND hDlg) { //-------------------------------------------------------------------+++-->
	int i, j, id, count, seconds, minutes, hours, days, iTimer;
	char subkey[TNY_BUFF] = {0};
	char name[GEN_BUFF] = {0};
	char s[MAX_BUFF] = {0};
	PTIMERSTRUCT2 temp;
	
  GetDlgItemText(hDlg, IDC_TIMERNAME, name, GEN_BUFF);
	
  // Save Settings as Needed - IF Needed.
  j = 1; id = -1;
  
  count = (int)(LRESULT)CBGetCount(hDlg, IDC_TIMERNAME);
  count -=1; // Skip the Last One Because It's the New Timer Dummy Item
  
  for(i = 0; i < count; i++) {
		PTIMERSTRUCT pts;

	  pts = (PTIMERSTRUCT)CBGetItemData(hDlg, IDC_TIMERNAME, i);
	  if(strcmp(pts->name, name) != 0) { //----//++--> Create Timer Named X in Registry
											  //--+++--> if it Does Not Currently Exist.
		  
		  wsprintf(subkey, "%s\\Timer%d", szTimersSubKey, j + 1); // Timers Get renumbered 
											 // So the Timer Activated Last Becomes Timer1

		  SetMyRegStr(subkey, "Name",     pts->name);		// Transfer Configuration Info
		  SetMyRegStr(subkey, "File",     pts->fname);		// From Timer X to Timer Y, so
		  SetMyRegLong(subkey, "ID",      pts->id);			// The Timer we Just Activated
		  SetMyRegLong(subkey, "Seconds", pts->second);		// Can "Bubble", to the top of
		  SetMyRegLong(subkey, "Minutes", pts->minute);		// the ComboBox's Timers List.
		  SetMyRegLong(subkey, "Hours",   pts->hour);
		  SetMyRegLong(subkey, "Days",    pts->day);
		  SetMyRegLong(subkey, "Repeat",  pts->bRepeat);
		  SetMyRegLong(subkey, "Blink",   pts->bBlink);
		  SetMyRegLong(subkey, "Active",  pts->bActive);
		  j++;
	  }else{
		  id = pts->id; //--++--// ID of the Timer we are Activating
		  pts->bActive = TRUE; // Timer B Active (As Far as the ComboBox is Concerned Only)
	  }
  }
  
  SetMyRegLong(szTimersSubKey, "NumberOfTimers", j);
	
  if(id < 0) {
	  id = 0;
	  for(i = 0; i < count; i++) {
		  PTIMERSTRUCT pts;
		  pts = (PTIMERSTRUCT)CBGetItemData(hDlg, IDC_TIMERNAME, i);
		  if(pts->id >= id) id = pts->id + 1;
	  }
  }

  wsprintf(subkey, "%s\\Timer1", szTimersSubKey);
  SetMyRegStr(subkey, "Name", name);
  SetMyRegLong(subkey, "ID", id);

  seconds = GetDlgItemInt(hDlg, IDC_TIMERSECOND, 0, FALSE);
  minutes = GetDlgItemInt(hDlg, IDC_TIMERMINUTE, 0, FALSE);
  hours   = GetDlgItemInt(hDlg, IDC_TIMERHOUR,   0, FALSE);
  days    = GetDlgItemInt(hDlg, IDC_TIMERDAYS,   0, FALSE);

  ValidateTimerInput(seconds, minutes, hours, days);

  SetMyRegLong(subkey, "Seconds", seconds);
  SetMyRegLong(subkey, "Minutes", minutes);
  SetMyRegLong(subkey, "Hours",   hours);
  SetMyRegLong(subkey, "Days",    days);

  GetDlgItemText(hDlg, IDC_TIMERFILE, s, MAX_PATH);
  SetMyRegStr(subkey, "File", s);
  
  SetMyRegLong(subkey, "Repeat", IsDlgButtonChecked(hDlg, IDC_TIMERREPEAT));
  SetMyRegLong(subkey, "Blink",  IsDlgButtonChecked(hDlg, IDC_TIMERBLINK));
  SetMyRegLong(subkey, "Active",  TRUE);
	
	// start timer
	temp = pTimersWorking;
	pTimersWorking = (PTIMERSTRUCT2)malloc(sizeof(TIMERSTRUCT2)*(nTimerCount + 1));
	for(i = 0; i < nTimerCount; i++) {
		pTimersWorking[i] = temp[i];
	}
	if(temp) free(temp);

	iTimer = seconds;
	iTimer += minutes * 60;
	iTimer += hours * 3600;
	iTimer += days * 86400;

	strcpy(pTimersWorking[i].name, name);
	pTimersWorking[i].id = id;
	pTimersWorking[i].seconds = iTimer;
	pTimersWorking[i].bHomeless = FALSE;
	pTimersWorking[i].tickonstart = GetTickCount();
	
	nTimerCount++;
}
 //================================================================================================
//-----------------------------//-----+++--> Load the Data Set For Timer X When Its Name is Called:
void OnTimerName(HWND hDlg) { //------------------------------------------------------------+++-->
	char s[TNY_BUFF];
	int i, count;
	
  GetDlgItemText(hDlg, IDC_TIMERNAME, s, TNY_BUFF);
  count = (int)(LRESULT)CBGetCount(hDlg, IDC_TIMERNAME);
  for(i = 0; i < count; i++) {
		PTIMERSTRUCT pts;
	  pts = (PTIMERSTRUCT)CBGetItemData(hDlg, IDC_TIMERNAME, i);
	  if(strcmp(s, pts->name) == 0) {
		  SetDlgItemInt(hDlg, IDC_TIMERSECOND, pts->second, FALSE);
		  SetDlgItemInt(hDlg, IDC_TIMERMINUTE, pts->minute, FALSE);
		  SetDlgItemInt(hDlg, IDC_TIMERHOUR,   pts->hour,   FALSE);
		  SetDlgItemInt(hDlg, IDC_TIMERDAYS,   pts->day,    FALSE);
		  SetDlgItemText(hDlg, IDC_TIMERFILE, pts->fname);
		  CheckDlgButton(hDlg, IDC_TIMERREPEAT, pts->bRepeat);
		  CheckDlgButton(hDlg, IDC_TIMERBLINK, pts->bBlink);
		  if(pts->bActive) {
			  EnableWindow(GetDlgItem(hDlg, IDCB_STOPTIMER), TRUE);
			  EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
		  }else{
			  EnableWindow(GetDlgItem(hDlg, IDCB_STOPTIMER), FALSE);
			  EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
		  } break;
	  }
  }
  EnableDlgItem(hDlg, IDC_TIMERDEL, i < count);
}
/*------------------------------------------------
  browse sound file
--------------------------------------------------*/
void OnSanshoAlarm(HWND hDlg, WORD id) {
    char deffile[MAX_PATH], fname[MAX_PATH];
	
  GetDlgItemText(hDlg, id - 1, deffile, MAX_PATH);
  if(!BrowseSoundFile(hDlg, deffile, fname)) // soundselect.c
	 return;
  SetDlgItemText(hDlg, id - 1, fname);
  PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}

 //================================================================================================
//-----------------------//-----------------------------+++--> Delete One of the Configured Timers:
void OnDel(HWND hDlg) { //------------------------------------------------------------------+++-->
	char s[TNY_BUFF], subkey[TNY_BUFF];
	int i, k, count;
	
	GetDlgItemText(hDlg, IDC_TIMERNAME, s, TNY_BUFF);
	count = (int)(LRESULT)CBGetCount(hDlg, IDC_TIMERNAME);
	for(i = 0; i < count; i++)
	{
		PTIMERSTRUCT pts;
		pts = (PTIMERSTRUCT)CBGetItemData(hDlg, IDC_TIMERNAME, i);
		if(strcmp(s, pts->name) == 0) {
			//--+++--> Stop Timer on KiLL: Suggested by ewemoa @ DonationCoder.com
			for(k = 0; k < nTimerCount; k++) {
				if(strcmp(s, pTimersWorking[k].name) == 0) {
					StopTimer(hDlg, k);
					MessageBox(hDlg, s, "Timer Stopped!", MB_OK|MB_ICONINFORMATION);
				}
			}
			//--+++--> End of Stop Timer on KiLL: Simple Really - Shoulda Been There Already
			free(pts); 
			break; 
		}
	}
	if(i >= count) return;
	
	CBDeleteString(hDlg, IDC_TIMERNAME, i);
	if(count > 1)
		CBSetCurSel(hDlg, IDC_TIMERNAME, (i>0)?(i-1):i);
	else
		SetDlgItemText(hDlg, IDC_TIMERNAME, "");
	OnTimerName(hDlg);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	
	wsprintf(subkey, "%s\\Timer%d", szTimersSubKey, count);
	DelMyRegKey(subkey);
	
	for(i = 0; i < count - 1; i++)
	{
		PTIMERSTRUCT pts;
		pts = (PTIMERSTRUCT)CBGetItemData(hDlg, IDC_TIMERNAME, i);
		wsprintf(subkey, "%s\\Timer%d", szTimersSubKey, i + 1);
		SetMyRegStr(subkey, "Name", pts->name);
		SetMyRegLong(subkey, "ID", pts->id);
		SetMyRegLong(subkey, "Seconds", pts->second);
		SetMyRegLong(subkey, "Minutes", pts->minute);
		SetMyRegLong(subkey, "Hours",   pts->hour);
		SetMyRegLong(subkey, "Days",    pts->day);
		SetMyRegStr(subkey, "File", pts->fname);
		SetMyRegLong(subkey, "Repeat", pts->bRepeat);
		SetMyRegLong(subkey, "Blink", pts->bBlink);
		SetMyRegLong(subkey, "Active",  pts->bActive);
	}
	SetMyRegLong(szTimersSubKey, "NumberOfTimers", count - 1);
}
 //================================================================================================
//---------------------------------//--------------------+++--> Test -> Play/Stop Alarm Sound File:
void OnTest(HWND hDlg, WORD id) { //--------------------------------------------------------+++-->
    char fname[MAX_PATH];
	
  GetDlgItemText(hDlg, id - 2, fname, MAX_PATH);
  if(fname[0] == 0) return;

  if((HICON)SendDlgItemMessage(hDlg, id, BM_GETIMAGE, IMAGE_ICON, 0) == g_hIconPlay) {
	 if(PlayFile(hDlg, fname, 0)) {
		 SendDlgItemMessage(hDlg, id, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconStop);
		 InvalidateRect(GetDlgItem(hDlg, id), NULL, FALSE);
	 }
  }
 else StopFile();
}
 //================================================================================================
//------+++--> Called When Main Window Receives WM_TIMER - Sound the Alarm if Clock has Run Out...:
void OnTimerTimer(HWND hwnd, SYSTEMTIME* st) { //-------------------------------------------+++-->
	DWORD tick;
	int i;

  if(nTimerCount == 0) return;

  tick = GetTickCount();
  for(i = 0; i < nTimerCount; i++) {
		DWORD seconds;
	  seconds = (tick - pTimersWorking[i].tickonstart) / 1000;
	  if(seconds >= pTimersWorking[i].seconds) {
		  Ring(hwnd, pTimersWorking[i].id);
		  StopTimer(hwnd, i);
		  i--;
	  }
  }
}
 //================================================================================================
//------------------------------//---------------------------+++--> Sound Alarm or Open Timer File:
void Ring(HWND hwnd, int id) { //-----------------------------------------------------------+++-->
	char subkey[TNY_BUFF], fname[MAX_BUFF];
	int i, count;
	
  count = GetMyRegLong(szTimersSubKey, "NumberOfTimers", 0);
  for(i = 0; i < count; i++) {
	  wsprintf(subkey, "%s\\Timer%d", szTimersSubKey, i + 1);

	  if(id == GetMyRegLong(subkey, "ID", 0)) {
		 GetMyRegStr(subkey, "File", fname, MAX_BUFF, "");
		 SetMyRegLong(subkey, "Active",  FALSE);
		 PlayFile(hwnd, fname, GetMyRegLong(subkey, "Repeat", FALSE)?(-1):0);
		 if(GetMyRegLong(subkey, "Blink", FALSE)) PostMessage(g_hwndClock, CLOCKM_BLINK, FALSE, 0);
		break;
	  }
  }
}
 //================================================================================================
//---------------------//---------------------------------------------------+++--> Clear All Timer:
void EndTimer(void) { //--------------------------------------------------------------------+++-->
  if(pTimersWorking) free(pTimersWorking);
  nTimerCount = 0;
}
 //================================================================================================
//---------+++--> Get Active Timer Name(s) to Populate Menu -or- Mark Selected Timer as "Homeless":
int GetTimerInfo(char *dst, int num, BOOL bNameOnly) { //-----------------------------------+++-->
	DWORD tick;
	int iTCount, days, hours, minutes, seconds;
	
  if(num < nTimerCount) {
	 if(bNameOnly) {
		 wsprintf(dst, " %s", pTimersWorking[num].name);
	 }else{
		 tick = GetTickCount();
		 iTCount = (tick - pTimersWorking[num].tickonstart) / 1000;
		 iTCount = pTimersWorking[num].seconds - iTCount;
		 pTimersWorking[num].bHomeless = TRUE; // Homeless Timers Are Automatically Picked-Up
		 if(iTCount <= 0) {					  //-++--> By the Timer Watch Window for Display.
			 wsprintf(dst, " <- Time Expired!");
			 return -1;
		 }
							
		 days = iTCount / 86400;			 // The Watch Window Then Becomes Their New Home.
		 hours   = (iTCount -  (days * 86400)) / 3600; // Don't Ya Just Love Happy Endings...
		 minutes = (iTCount - ((days * 86400) + (hours * 3600))) / 60;
		 seconds = (iTCount - ((days * 86400) + (hours * 3600) + (minutes * 60)));
		 wsprintf(dst, "%02dD:%02dH:%02dM:%02dS", days, hours, minutes, seconds);
	 }
	 return (int)strlen(dst);
  }
 return 0;
}
 //================================================================================================
//---------------------------------------//--------------------+++--> Free Memory to Clear a Timer:
void StopTimer(HWND hwnd, int tostop) { //--------------------------------------------------+++-->
	PTIMERSTRUCT2 temp;
	int i, j;
	
  if(tostop >= nTimerCount) return;
	
  temp = pTimersWorking;
  if(nTimerCount > 1) {
	  pTimersWorking = (PTIMERSTRUCT2)malloc(sizeof(TIMERSTRUCT2)*(nTimerCount - 1));
	  
	  for(i = 0, j = 0; i < nTimerCount; i++) {
		  if(tostop != i) pTimersWorking[j++] = temp[i];
	  }
  }
  else pTimersWorking = NULL;
  if(temp) free(temp);
  nTimerCount--;
}
 //================================================================================================
//-------------------------------------+++--> Spoof Control Message to Force Update of Nexe Window:
void UpdateNextCtrl(HWND hWnd, int iSpin, int iEdit, BOOL bGoUp) { //-----------------------+++-->
	NMUPDOWN nmud;

  nmud.hdr.hwndFrom = GetDlgItem(hWnd, iSpin);
  nmud.hdr.idFrom = iSpin;
  nmud.hdr.code = UDN_DELTAPOS;
  if(bGoUp)nmud.iDelta = 4; // Fake Message Forces Update of Next Control!
  else nmud.iDelta = -4;   // Fake Message Forces Update of Next Control!
  nmud.iPos = GetDlgItemInt(hWnd, iEdit, NULL, TRUE);
  
  SendMessage(hWnd, WM_NOTIFY, iSpin, (LPARAM)&nmud);
}
 //================================================================================================
//-----------------------------//-------------------+++--> Stop & Cancel a Currently Running Timer:
void OnStopTimer(HWND hWnd) { //------------------------------------------------------------+++-->
	char subkey[TNY_BUFF];
	char s[GEN_BUFF];
	int i, count;
	
  GetDlgItemText(hWnd, IDC_TIMERNAME, s, TNY_BUFF);
  count = GetMyRegLong(szTimersSubKey, "NumberOfTimers", 0);
  
  for(i = 0; i < nTimerCount; i++) {

	  if(strcmp(s, pTimersWorking[i].name) == 0) {
		  StopTimer(hWnd, i);
		  MessageBox(hWnd, s, "Timer Stopped!", MB_OK|MB_ICONINFORMATION);

		  for(i = 0; i < count; i++) {
			  char szName[GEN_BUFF] = {0};
			  PTIMERSTRUCT pts;
			  pts = (PTIMERSTRUCT)CBGetItemData(hWnd, IDC_TIMERNAME, i);
			  wsprintf(subkey, "%s\\Timer%d", szTimersSubKey, i +1);
			  GetMyRegStr(subkey, "Name", szName, GEN_BUFF, "");
			  if(strcmp(s, szName) == 0) {
				  SetMyRegLong(subkey, "Active",  FALSE);
				  pts->bActive = FALSE;
				  break;
			  }
		  }
	  
		  EnableWindow(GetDlgItem(hWnd, IDOK), TRUE);
		  EnableWindow(GetDlgItem(hWnd, IDCB_STOPTIMER), FALSE);
		  return;
	  }
  }
}
 //================================================================================================
//--{ DC - ewemoa }------+++--> Validate Values as Being Within Expected Ranges (Adjust as Needed):
void ValidateTimerInput(int sec, int min, int hrs, int day) { // Second  = 1 Second --------+++-->
															  // Minute = 60 Seconds
															  // Hour = 3600 Seconds
  if(sec > 59) {											  // Day = 86400 Seconds
	  for(sec; sec > 59; sec -= 60) {
		  min += 1;
	  }
  }
  
  if(min > 59) {
	  for(min; min > 59; min -= 60) {
		  hrs += 1;
	  }
  }

  if(hrs > 23) {
	  for(hrs; hrs > 23; hrs -= 24) {
		  day += 1;
	  }
  }

  if(day > 42) day = 7;
}
 //================================================================================================
//-----------------------------+++--> When T-Clock Starts, Make Sure ALL Timer Are Set as INActive:
void CancelAllTimersOnStartUp() { //--------------------------------------------------------+++-->
	char subkey[TNY_BUFF];
	int i, count;
	
  count = GetMyRegLong(szTimersSubKey, "NumberOfTimers", 0);

  for(i = 0; i < count; i++) {
	  wsprintf(subkey, "%s\\Timer%d", szTimersSubKey, i + 1);
	  SetMyRegLong(subkey, "Active", FALSE);
  }
}
 //================================================================================================
// -------------------------------------------//+++--> Initialize Timer View/Watch Dialog Controls:
void OnInitTimeView(HWND hDlg, HWND hList) { //---------------------------------------------+++-->
	LVCOLUMN lvCol;
	RECT rc;

  SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)g_hIconTClock);

  hList = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD|WS_VSCROLL|LVS_REPORT|
							LVS_SINGLESEL, 0, 0, 261, 104, hDlg, NULL, 0, 0);

  ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

  lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
  lvCol.cx = 125;		 // Column Width
  lvCol.iSubItem = 0;      // Column Number
  lvCol.fmt = LVCFMT_CENTER; // Column Alignment
  lvCol.pszText = TEXT("Timer"); // Column Header Text
  ListView_InsertColumn(hList, 0, &lvCol);

  lvCol.cx = 150;
  lvCol.iSubItem = 1;
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.pszText = TEXT("Remaining");
  ListView_InsertColumn(hList, 1, &lvCol);

  GetClientRect(hDlg, &rc);
  SetWindowPos(hList, NULL, 0, 0, rc.right-rc.left, rc.bottom-rc.top, SWP_SHOWWINDOW);
}
 //================================================================================================
//-------------------------------------+++--> Gather Status Info About the Timers User is Watching:
BOOL OnWatchTimer(HWND hDlg, HWND hList) { //-----------------------------------------------+++-->
	char szStatus[MIN_BUFF] = {0};
	BOOL bNeeded = FALSE;
	LVFINDINFO lvFind;
	int iTc, iTn, iF;
	LVITEM lvItem;

  iTc = nTimerCount;
  for(iTc; iTc > 0; iTc--) {
		iTn = (iTc - 1);
		if(pTimersWorking[iTn].bHomeless) {
			GetTimerInfo(szStatus, iTn, FALSE);

			lvFind.flags = LVFI_STRING;
			lvFind.psz = pTimersWorking[iTn].name;
			if((iF = ListView_FindItem(hList, -1, &lvFind)) != -1) {
				ListView_SetItemText(hList, iF, 1, szStatus); // IF Timer Pre-Exists,
				bNeeded = TRUE; //------------+++--> Update the Existing Timer Entry.
			}else{
			//---------------------+++--> ELSE Add the New Timer Entry to Watch List.
				  lvItem.mask = LVIF_TEXT;
				  lvItem.iSubItem = 0;
				  lvItem.iItem = 0;

				lvItem.pszText = pTimersWorking[iTn].name;
				ListView_InsertItem(hList, &lvItem);

				  lvItem.iSubItem = 1;
				  lvItem.pszText = szStatus;
				ListView_SetItem(hList, &lvItem);
				bNeeded = TRUE;
			}
		}
  }
 return bNeeded;
}
 //================================================================================================
//-----------------------------+++--> Remove Timer X From Timer Watch List (Set Homeless to FALSE):
void RemoveFromWatch(HWND hWnd, HWND hList, char *szTimer, int iLx) {
	char szCaption[GEN_BUFF] = {0}; int i; BOOL bExpired = TRUE;
	char szMessage[] = TEXT("Yes will Cancel the Timer & Remove it from the Watch List.\r\n"
							"No will Remove Timer from Watch List Only (Timer continues).\r\n"
							"Cancel will Assume you hit Delete Accidentally (and do nothing).");

  for(i = 0; i < nTimerCount; i++) {
	  if(strcmp(szTimer, pTimersWorking[i].name) == 0) {
		  bExpired = FALSE; // Timer is/Was Still Running
		  break; //--------//---+++--> at Time of Request.
	  }
  }

  if(bExpired) { //----+++--> IF the Timer Has Expired...
	  ListView_DeleteItem(hList, iLx); // Just Delete it.
	  return;
  }

  wsprintf(szCaption, "Cancel Timer (%s) Also?", szTimer);

  switch(MessageBox(hWnd, szMessage, szCaption, MB_YESNOCANCEL|MB_ICONQUESTION)) {
	  case IDYES:
		  for(i = 0; i < nTimerCount; i++) {
			  if(strcmp(szTimer, pTimersWorking[i].name) == 0) {
					int count, k;

//===========================================================================
				  count = GetMyRegLong(szTimersSubKey, "NumberOfTimers", 0);
				  for(k = 0; k < count; k++) {
						  char szName[GEN_BUFF] = {0};
						  char subkey[TNY_BUFF] = {0};

					  wsprintf(subkey, "%s\\Timer%d", szTimersSubKey, k + 1);
					  GetMyRegStr(subkey, "Name", szName, GEN_BUFF, "");
					  if(strcmp(szName, szTimer) == 0) {
						  SetMyRegLong(subkey, "Active", FALSE);
						  pTimersWorking[i].bHomeless = FALSE;
						  ListView_DeleteItem(hList, iLx);
						  StopTimer(hWnd, i); // Does Not Reset Active Flag!
						return; // The End...
					  }
				  }
//===========================================================================
				  break;
			  }
		  } break;

	  case IDNO:
		  for(i = 0; i < nTimerCount; i++) {
			  if(strcmp(szTimer, pTimersWorking[i].name) == 0) {
				  pTimersWorking[i].bHomeless = FALSE;
				  ListView_DeleteItem(hList, iLx);
				  break;
			  }
		  }  break;

	  default: return;
  }
}
 //================================================================================================
// -----------------------+++--> Message Processor for the Selected Running Timers Watching Dialog:
BOOL CALLBACK DlgTimerViewProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) { //------+++-->
	WORD id, code; HWND hList;
	hList = FindWindowEx(hDlg, NULL, WC_LISTVIEW, NULL);

  id = LOWORD(wParam);
  code = HIWORD(wParam);

  switch(msg) {
	case WM_INITDIALOG:
		OnInitTimeView(hDlg, hList);
		SetTimer(hDlg, 3, 285, NULL); // Timer Refresh Times Above 400ms Make
		SetMyDialgPos(hDlg); //-----+++--> Timer Watch Dialog Appear Sluggish.
	  return TRUE; //-------------------------------+++--> END of Case WM_INITDOALOG
//================//================================================================
	case WM_TIMER:
		if(!OnWatchTimer(hDlg, hList)) { // When the Last Monitored Timer
			KillTimer(hDlg, 3);			 // Expires, Close the Now UnNeeded
			EndDialog(hDlg, TRUE);		 // Timer Watch/View Dialog Window.
			g_hDlgTimerWatch = NULL;
		} return TRUE; //--------------------------------+++--> END of Case WM_TIMER
//====================//============================================================
	case WM_COMMAND: {
	  switch(id) {
		case IDCANCEL:
			KillTimer(hDlg, 3);
			EndDialog(hDlg, TRUE);
			g_hDlgTimerWatch = NULL;
		  return TRUE;
	  } return FALSE;
	} //-----------------------------------------------+++--> END of Case WM_COMMAND
//===//=============================================================================
	case WM_NOTIFY: {
		//--------------------------------------------------------------------+++-->
		if(((LPNMHDR)lParam)->code == LVN_KEYDOWN) { //-+> Capture Key Strokes Here.
				LPNMLVKEYDOWN nmkey = (LPNMLVKEYDOWN)lParam;
				char szTimer[GEN_BUFF] = {0};	int i;
			switch(nmkey->wVKey) {
				case VK_DELETE:
					if((i = (int)SendMessage(hList, LVM_GETNEXTITEM, -1, LVNI_SELECTED)) != -1) {
						ListView_GetItemText(hList, i, 0, szTimer, GEN_BUFF);
						RemoveFromWatch(hDlg, hList, szTimer, i);
					} return TRUE; // Delete Key Handled
				return FALSE; // ALL Other Keys Ignored!
			}
		} break;
	} //------------------------------------------------+++--> END of Case WM_NOTIFY
//===//=============================================================================
  }
 return FALSE;
}
 //================================================================================================
// ------------------//---------------------------------------------+++--> Open Timer Watch Dialog:
void WatchTimer() { //----------------------------------------------------------------------+++-->
  if(g_hDlgTimerWatch && IsWindow(g_hDlgTimerWatch)) {
	  ForceForegroundWindow(g_hDlgTimerWatch);
  }else{
	  g_hDlgTimerWatch = CreateDialog(0, MAKEINTRESOURCE(IDD_TIMERVIEW), NULL, (DLGPROC)DlgTimerViewProc);
  }
}