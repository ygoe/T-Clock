 // Written by Stoic Joker: Tuesday, 03/16/2010 @ 10:18:59pm
// Modified by Stoic Joker: Monday, 03/22/2010 @ 7:32:29pm
#include "tclock.h"

typedef struct {
    LARGE_INTEGER start;
    LARGE_INTEGER now;
} stopWatch;

stopWatch tcsw;  // Global Time Structure for Results.
stopWatch tcLap;// Global Time Structure for Lap Times.
BOOL bStopped; // Global Pause/Resume Displayed Counter.
 //================================================================================================
//--------------------------------+++--> Get the High-Resolution Performance Counter Starting Time:
void startTimer(stopWatch *timer) { //------------------------------------------------------+++-->
    QueryPerformanceCounter(&timer->start);
	tcLap.start = timer->start;
}
 //================================================================================================
//---------------------------------+++--> Get the High-Resolution Performance Counter Current Time:
void GetTimeNow(stopWatch *timer) { //------------------------------------------------------+++-->
    QueryPerformanceCounter(&timer->now);
}
 //================================================================================================
//------+++--> Calculate Seconds Passed Based on the High-Resolution Performance Counter Frequency:
double LIToSecs(LARGE_INTEGER *L) { //------------------------------------------------------+++-->
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
  return((double)L->QuadPart /(double)frequency.QuadPart);
}
 //================================================================================================
//--------------------------------+++--> Calculate the Time That Has Elapsed Between Start and Now:
double getElapsedTime(stopWatch *timer) { //------------------------------------------------+++-->
    LARGE_INTEGER time;
    time.QuadPart = timer->now.QuadPart - timer->start.QuadPart;
  return LIToSecs(&time);
}
 //================================================================================================
// ----------------------------------------------------+++--> Initialize Stopwatch Dialog Controls:
void OnInit(HWND hDlg, HWND hList) { //-----------------------------------------------------+++-->
	LVCOLUMN lvCol;

  SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)g_hIconTClock);

  hList = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD|WS_VSCROLL|LVS_REPORT|
							LVS_SINGLESEL, 9, 55, 261, 104, hDlg, NULL, 0, 0);

  ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

  lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
  lvCol.cx = 42;		 // Column Width
  lvCol.iSubItem = 0;      // Column Number
  lvCol.fmt = LVCFMT_CENTER; // Column Alignment
  lvCol.pszText = TEXT("Lap"); // Column Header Text
  ListView_InsertColumn(hList, 0, &lvCol);

  lvCol.cx = 121;
  lvCol.iSubItem = 1;
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.pszText = TEXT("Times");
  ListView_InsertColumn(hList, 1, &lvCol);

  ShowWindow(hList, SW_SHOW);

  SetDlgItemText(hDlg, IDCE_SW_ELAPSED, "00H: 00M: 00S: 000ms");
}
 //================================================================================================
//-------------------------//------------------+++--> Updates the Stopwatch's Elapsed Time Display:
void OnTimer(HWND hDlg) { //----------------------------------------------------------------+++-->
	char szElapsed[TNY_BUFF] = {0};
	int hrs, min, sec, ims; //-----------+++--> wsprintf Requires ALL Values be Integers.
	double dwElapsed; // Need to use a (Floating Point) Double to Get the "Change" Later.
	double ms; //------------+++--> This is/Will Be the MilliSecond "Change" We're After.

  GetTimeNow(&tcsw); //---------------//-+++--> Get the Current High Resolution Time.
  dwElapsed = getElapsedTime(&tcsw); // Compare it to the High Resolution Start Time.

  hrs =  (int)dwElapsed / 3600;
  min = (int)(dwElapsed - (hrs * 3600)) / 60;
  sec = (int)(dwElapsed - ((hrs * 3600) + (min * 60)));
  ms  = (dwElapsed - ((hrs * 3600) + (min * 60) + sec));
  // Now the ms double = .WhatWeAreAfter (e.g. The MilliSecond "Change".)
  ms *= 1000; //-+> The .Change * 1000 Converts it into an Interger That,
  ims = (int)ms; //-+-> We Can Now Use to Give Completely Precise Timing.
  wsprintf(szElapsed, "%02dH: %02dM: %02dS: %003dms", hrs, min, sec, ims);
  SetDlgItemText(hDlg, IDCE_SW_ELAPSED, szElapsed);
}
 //================================================================================================
//--------------------------+++--> Get Current Time as Lap Time and Add it to the ListView Control:
void InsertLapTime(HWND hList) { //---------------------------------------------------------+++-->
	char szLapTime[TNY_BUFF] = {0};
	char szLapNum[TNY_BUFF] = {0};
	int hrs, min, sec, ims; //-----------+++--> wsprintf Requires ALL Values be Integers.
	double dwElapsed; // Need to use a (Floating Point) Double to Get the "Change" Later.
	double ms; //------------+++--> This is/Will Be the MilliSecond "Change" We're After.
	LVITEM lvItem; // ListView Control Row Identifier
	int col = 0;  // ListView Control Column to Populate
	int iLap;

  GetTimeNow(&tcLap); //---------------//-+++--> Get the Current High Resolution Time.
  dwElapsed = getElapsedTime(&tcLap); // Compare it to the High Resolution Start Time.
  tcLap.start = tcLap.now; //-++--> Reset Starting Point to Current Time for Next Lap.

  hrs =  (int)dwElapsed / 3600;
  min = (int)(dwElapsed - (hrs * 3600)) / 60;
  sec = (int)(dwElapsed - ((hrs * 3600) + (min * 60)));
  ms  = (dwElapsed - ((hrs * 3600) + (min * 60) + sec));
  // Now the ms double = .WhatWeAreAfter (e.g. The MilliSecond "Change".)
  ms *= 1000; //-+> The .Change * 1000 Converts it into an Interger That,
  ims = (int)ms; //-+-> We Can Now Use to Give Completely Precise Timing.
  wsprintf(szLapTime, "%02d:%02d:%02d:%003dms", hrs, min, sec, ims);

  iLap = ListView_GetItemCount(hList);
  iLap++;

  wsprintf(szLapNum, "Lap %d", iLap);
  lvItem.mask = LVIF_TEXT;
  lvItem.iSubItem = 0;
  lvItem.iItem = 0;

  lvItem.pszText = szLapNum;
  ListView_InsertItem(hList, &lvItem);

  lvItem.iSubItem = 1;
  lvItem.pszText = szLapTime;
  ListView_SetItem(hList, &lvItem);
}
 //================================================================================================
// --------------------------------------------------+++--> Message Processor for Stopwatch Dialog:
BOOL CALLBACK DlgProcStopwatch(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) { //------+++-->
	WORD id, code; HWND hList;
	char szElapsed[TNY_BUFF] = {0};
	hList = FindWindowEx(hDlg, NULL, WC_LISTVIEW, NULL);

  id = LOWORD(wParam);
  code = HIWORD(wParam);

  switch(msg) {
	case WM_INITDIALOG:
	  OnInit(hDlg, hList);
	  SetMyDialgPos(hDlg);
	  return TRUE;

	case WM_TIMER:
		if(!bStopped) OnTimer(hDlg);
	  return TRUE;

	case WM_COMMAND: {
	  switch(id) {

		case IDOK: // Start
			if(!bStopped) {
				startTimer(&tcsw);
				SetTimer(hDlg, 1, 1, NULL);
			}
			bStopped = FALSE;
			EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDCB_SW_STOP), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDCB_SW_RESET), FALSE);
		  break;

		case IDCB_SW_STOP:
			EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDCB_SW_STOP), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDCB_SW_RESET), TRUE);
			bStopped = TRUE;
		  break;

		case IDCB_SW_RESET:
			KillTimer(hDlg, 1);
			EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDCB_SW_STOP), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDCB_SW_RESET), FALSE);
			SetDlgItemText(hDlg, IDCE_SW_ELAPSED, "00H: 00M: 00S: 000ms");
			ListView_DeleteAllItems(hList);
			bStopped = FALSE;
		  break;
		
		case IDCB_SW_LAP:
			InsertLapTime(hList);
		  break;
		
		case IDCANCEL:
			KillTimer(hDlg, 1);
		  EndDialog(hDlg, TRUE);
		  g_hDlgStopWatch = NULL;
	  }
	 return TRUE;
	}

  }
 return FALSE;
}
 //================================================================================================
// -------------------------------------------------------------------+++--> Open Stopwatch Dialog:
void DialogStopWatch(HWND hWnd) { //--------------------------------------------------------+++-->
  if(g_hDlgStopWatch && IsWindow(g_hDlgStopWatch));
  else g_hDlgStopWatch = CreateDialog(0, MAKEINTRESOURCE(IDD_STOPWATCH), NULL, (DLGPROC)DlgProcStopwatch);
  ForceForegroundWindow(g_hDlgStopWatch);
}