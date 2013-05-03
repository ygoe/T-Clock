   /*----------------------------------------------------------------------
  // deskcal.c : Update Desktop Calendar automatically -> KAZUBON 1997-1999
 //---------------------------------------------------------------------*/
// Last Modified by Stoic Joker: Saturday, 06/05/2010 @ 3:48:15pm
#include "tclock.h"
#include <time.h>
//#pragma comment(lib, "comctl32.lib")
static BOOL bAutoClose;
void SetMyDialgPos(HWND hwnd); // propsheet.c
 //================================================================================================
//----------------------------------------------------------+++--> Get the Current Day of the Year:
void GetDayOfYearTitle(char *szTitle, int ivMonths) {
	struct tm today;
	char szDoY[8];
	time_t ltime;

  time(&ltime);
  _localtime64_s(&today, &ltime);
//  strftime(szDoY, 8, "%#j", &today); // <--{OutPut}--> Day 95
  strftime(szDoY, 8, "%j", &today);   // <--{OutPut}--> Day 095

  if(!bV7up && (ivMonths = 1)) {
	  wsprintf(szTitle, "Calendar:  Day: %s", szDoY);
  }else{
	  wsprintf(szTitle, "T-Clock: Calendar  Day: %s", szDoY);
  }
}
 //================================================================================================
//----------------------------------------------------+++--> Dialog Procedure of "Calender" Dialog:
BOOL CALLBACK DlgProcCalender(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) { //---+++-->
	int uiHigh, uiWide, ivMonths;
	int iWd, iHt;
	WORD id, code;
	DWORD dwCalStyle;
	RECT rc; HWND hCal;
	id = LOWORD(wParam);
	code = HIWORD(wParam);		// ADD Day of Year to Caption Bar.


  switch(message) {
	case WM_INITDIALOG:
	  ivMonths = GetMyRegLongEx("Calendar", "ViewMonths", 1);
	  SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)g_hIconTClock);

	  if(GetMyRegLong("Calendar", "ShowDayOfYear", FALSE)) {
			char szTitle[TNY_BUFF];
		  GetDayOfYearTitle(szTitle, ivMonths);
		  SetWindowText(hDlg, szTitle);
	  }

	  if(GetMyRegLong("Calendar", "ShowWeekNums", FALSE)) {
		  dwCalStyle = WS_BORDER|WS_CHILD|WS_VISIBLE|MCS_NOTODAYCIRCLE|MCS_WEEKNUMBERS;
	  }else{
		  dwCalStyle = WS_BORDER|WS_CHILD|WS_VISIBLE|MCS_NOTODAYCIRCLE;
	  }

	  hCal = CreateWindowEx(0, MONTHCAL_CLASS, "", dwCalStyle, 0,0,0,0, hDlg, 0, 0, NULL);

	  if(bV7up) { // OS is Windows Vista, 7, or Above
		  if(ivMonths < 12) {
			  uiHigh = 22;
		  }else{
			  uiHigh = -53;
		  }
		  uiWide = -8;
	  }else{ // OS is either Windows 2000 or Windows XP
		  if(ivMonths < 12) {
			  uiHigh = 34; // +12 Allows for Today mm/dd/yyyy Text Label.
		  }else{
			  uiHigh = -6;
		  }
		  uiWide = 14;
	  }

	  // Get the size required to show an entire month.
	  MonthCal_GetMinReqRect(hCal, &rc);

	  iWd = (((rc.right - rc.left) * 3) + uiWide);
	  if(ivMonths == 12) {
		  iHt = (((rc.bottom - rc.top) * 4) + uiHigh);
	  }else{
		  iHt = ((rc.bottom - rc.top) + uiHigh);
	  }

	  if(ivMonths > 1) {
		  SetWindowPos(hDlg, NULL, rc.left, rc.top, iWd, iHt, SWP_NOMOVE);
	  }else{
		  // This is the Original 1 Month Calendar Code
		  SetWindowPos(hDlg, NULL, rc.left, rc.top, rc.right+6, rc.bottom+uiHigh, SWP_NOMOVE);
	  }

	  GetClientRect(hDlg, &rc);
	  SetWindowPos(hCal, NULL, rc.left, rc.top, rc.right, rc.bottom, SWP_NOZORDER);

	  SetMyDialgPos(hDlg);
	  ForceForegroundWindow(hDlg); // <--+++--> Stick it on Top!
	  if(GetMyRegLong("Calendar", "CalendarTopMost", FALSE)) {
		  SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	  } return TRUE; //-------------------------------+++--> END of Case WM_INITDIALOG:
	//=================================================================================

	case WM_ACTIVATE:
	  if((bAutoClose) && (LOWORD(wParam) == WA_INACTIVE)) {
		  SendMessage(hDlg, WM_CLOSE, 0, 0);
	  } return TRUE;

	case WM_CLOSE:
	  g_hDlgCalender = NULL;
	  DestroyWindow(hDlg);
	  return TRUE;
  }
 return FALSE;
}
 //================================================================================================
//--------------------------------//---------------------------------+++--> Open "Calendar" Dialog:
void DialogCalender(HWND hWnd) { //---------------------------------------------------------+++-->
  bAutoClose = GetMyRegLong("Calendar", "CloseCalendar", FALSE);

  if(g_hDlgCalender && IsWindow(g_hDlgCalender)) { //-+++--> If it Already Exists...
	  ForceForegroundWindow(g_hDlgCalender); // Bring it Forward (to Top of Z Axis).
  }else{
	  g_hDlgCalender = CreateDialog(0, MAKEINTRESOURCE(IDD_CALENDAR), hWnd, (DLGPROC)DlgProcCalender);
	  SetFocus(g_hDlgCalender); // Resolves Calendar (doesn't consistently) Close on Loose Focus Bugg.
  }
}