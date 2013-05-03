   //-------------------------------------------------------------------
  //--+++--> PageMisc.c - Stoic Joker: Saturday, 06/05/2010 @ 10:46:02pm
 //------------------------------------------------------------------*/
// Last Modified by Stoic Joker: Sunday, 03/13/2011 @ 11:54:05am
#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);

#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0)
 //================================================================================================
//---------------------------------------------+++--> Dialog Procedure of Miscellaneous Tab Dialog:
BOOL CALLBACK PageMiscProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) { //------+++-->
  switch(message) {
	case WM_INITDIALOG:
	  OnInit(hDlg);
	  return TRUE;

	case WM_COMMAND: {
	  WORD id, code;
	  id = LOWORD(wParam);
	  code = HIWORD(wParam);

	  if(id == IDCB_CLOSECAL) {
		  if(IsDlgButtonChecked(hDlg, IDCB_CALTOPMOST))
			  CheckDlgButton(hDlg, IDCB_CALTOPMOST, FALSE);
	  }

	  if(id == IDCB_CALTOPMOST) {
		  if(IsDlgButtonChecked(hDlg, IDCB_CLOSECAL))
			  CheckDlgButton(hDlg, IDCB_CLOSECAL, FALSE);
	  }

	  if(((id == IDCB_CLOSECAL) || // IF Anything Happens to Anything,
		  (id == IDCB_SHOWWEEKNUMS) || //--+++--> Send Changed Message.
		  (id == IDCB_TRANS2KICONS) ||
		  (id == IDCB_SHOW_DOY) ||
		  (id == IDC_CAL_YEAR) ||
		  (id == IDC_CAL_3MON) ||
		  (id == IDC_CAL_1MON) ||
		  (id == IDCB_MONOFF_ONLOCK) ||
		  (id == IDCB_CALTOPMOST)) && ((code == BST_CHECKED) || (code == BST_UNCHECKED))) {
			  SendPSChanged(hDlg);
	  }
	  if(id == IDC_FIRSTWEEK && code == CBN_SELCHANGE) SendPSChanged(hDlg);

	 return TRUE;
	}

	case WM_NOTIFY:
	  switch(((NMHDR *)lParam)->code) {
		case PSN_APPLY: OnApply(hDlg); break;
	  } return TRUE;

	case WM_DESTROY:
	  DestroyWindow(hDlg);
	  break;
  }
 return FALSE;
}
    //-------+++--> Make Adjustable: HKEY_CURRENT_USER\Control Panel\International\iFirstWeekOfYear
   //-------+++--> Data Type: DWORD Possible Valid Values: 0, 1, or 2 - Else it Should Return Fail!
  //-------+++--> Purpose: Required for Reading and/or Correcting the Calendar Week Numbers Offset.
 //================================================================================================
//--------------------//----------+++--> This is to Access the System Level iFirstWeekOfYear Value:
int GetMySysWeek() { //---------------------------------------------------------------------+++-->
	HKEY hkey;  DWORD size;  char val[8]={0};

  if(RegOpenKey(HKEY_CURRENT_USER, "Control Panel\\International", &hkey) == ERROR_SUCCESS) {
	  size=8; // Thank you Pascal Aloy - For noticing I screwed this up. :-)
	  RegQueryValueEx(hkey, "iFirstWeekOfYear", 0, NULL, (LPBYTE)val, &size);
	  RegCloseKey(hkey);
  }
 return atoi(val);
}
 //================================================================================================
//------------------------------//---------------+++--> Set Value for iFirstWeekOfYear in Registry:
void SetMySysWeek(char *val) { //-----------------------------------------------------------+++-->
	HKEY hkey;

  if(RegCreateKey(HKEY_CURRENT_USER, "Control Panel\\International", &hkey) == ERROR_SUCCESS) {
	  RegSetValueEx(hkey, "iFirstWeekOfYear", 0, REG_SZ, (CONST BYTE*)val, (DWORD)(int)strlen(val));
	  RegCloseKey(hkey);
  }
}
 //================================================================================================
//--------------------+++--> Initialize Properties Dialog & Customize T-Clock Controls as Required:
static void OnInit(HWND hDlg) { //----------------------------------------------------------+++-->
	int ivMonths, iWeekOff;

  CheckDlgButton(hDlg, IDCB_CLOSECAL,
		GetMyRegLongEx("Calendar", "CloseCalendar", FALSE));
  CheckDlgButton(hDlg, IDCB_SHOWWEEKNUMS,
		GetMyRegLongEx("Calendar", "ShowWeekNums", FALSE));
  CheckDlgButton(hDlg, IDCB_CALTOPMOST,
		GetMyRegLongEx("Calendar", "CalendarTopMost", FALSE));
  CheckDlgButton(hDlg, IDCB_SHOW_DOY,
		GetMyRegLongEx("Calendar", "ShowDayOfYear", FALSE));

  ivMonths = GetMyRegLongEx("Calendar", "ViewMonths", 1);
  if(ivMonths == 12)     CheckDlgButton(hDlg, IDC_CAL_YEAR, TRUE);
  else if(ivMonths == 3) CheckDlgButton(hDlg, IDC_CAL_3MON, TRUE);
  else                   CheckDlgButton(hDlg, IDC_CAL_1MON, TRUE);

  iWeekOff = GetMySysWeek();
  CBResetContent(hDlg, IDC_FIRSTWEEK);
  CBAddString(hDlg, IDC_FIRSTWEEK, (LPARAM)"0");
  CBAddString(hDlg, IDC_FIRSTWEEK, (LPARAM)"1");
  CBAddString(hDlg, IDC_FIRSTWEEK, (LPARAM)"2");
  CBSetCurSel(hDlg, IDC_FIRSTWEEK, iWeekOff);

  if(b2000) {
	  EnableDlgItem(hDlg, IDCB_MONOFF_ONLOCK, FALSE);
	  CheckDlgButton(hDlg, IDCB_TRANS2KICONS, GetMyRegLongEx("Desktop", "Transparent2kIconText", FALSE));
  }else{
	  EnableDlgItem(hDlg, IDCB_TRANS2KICONS, FALSE);
	  CheckDlgButton(hDlg, IDCB_MONOFF_ONLOCK, bMonOffOnLock);
  }
}
 //================================================================================================
//-------------------------//-----------------------------+++--> Save Current Settings to Registry:
void OnApply(HWND hDlg) { //----------------------------------------------------------------+++-->
	int ivMonths;  char szWeek[8];

  SetMyRegLong("Calendar", "CloseCalendar",
		IsDlgButtonChecked(hDlg, IDCB_CLOSECAL));
  SetMyRegLong("Calendar", "ShowWeekNums",
		IsDlgButtonChecked(hDlg, IDCB_SHOWWEEKNUMS));
  SetMyRegLong("Calendar", "ShowDayOfYear",
		IsDlgButtonChecked(hDlg, IDCB_SHOW_DOY));
  SetMyRegLong("Calendar", "CalendarTopMost",
		IsDlgButtonChecked(hDlg, IDCB_CALTOPMOST));
  SetMyRegLong("Desktop", "Transparent2kIconText",
		IsDlgButtonChecked(hDlg, IDCB_TRANS2KICONS));

  if(IsDlgButtonChecked(hDlg, IDC_CAL_1MON)) ivMonths = 1;
  if(IsDlgButtonChecked(hDlg, IDC_CAL_3MON)) ivMonths = 3;
  if(IsDlgButtonChecked(hDlg, IDC_CAL_YEAR)) ivMonths = 12;
  SetMyRegLong("Calendar", "ViewMonths", ivMonths);

  GetDlgItemText(hDlg, IDC_FIRSTWEEK, szWeek, 8);
  SetMySysWeek(szWeek);

  if(!b2000) { // This Feature is Not For Windows 2000, It's Only XP and Above!
	  if((!bMonOffOnLock) &&(IsDlgButtonChecked(hDlg, IDCB_MONOFF_ONLOCK))) {
		  SetMyRegLong("Desktop", "MonOffOnLock", TRUE);
		  RegisterSession(g_hWnd); // Sets bMonOffOnLock to TRUE.
	  }else if((bMonOffOnLock) &&(IsDlgButtonChecked(hDlg, IDCB_MONOFF_ONLOCK))){
		  // RegisterSession() Already Set bMonOffOnLock So There is Nothing to do.
	  }else{
		  SetMyRegLong("Desktop", "MonOffOnLock", FALSE);
		  UnregisterSession(g_hWnd); // Sets bMonOffOnLock to FALSE.
	  }
  }
}