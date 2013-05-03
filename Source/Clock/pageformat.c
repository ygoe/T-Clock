/*-------------------------------------------
  pageformat.c
  "Format" page of properties
                       KAZUBON 1997-1998
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnLocale(HWND hDlg);
static void On12Hour(HWND hDlg);
static void OnCustom(HWND hDlg, BOOL bmouse);
static void OnFormatCheck(HWND hDlg, WORD id);

static HWND hwndPage;
static int ilang;  // language code. ex) 0x411 - Japanese
static int idate;  // 0: mm/dd/yy 1: dd/mm/yy 2: yy/mm/dd
static BOOL bDayOfWeekIsLast;   // yy/mm/dd ddd
static BOOL bTimeMarkerIsFirst; // AM/PM hh:nn:ss
static char *pCustomFormat = NULL;
static char sMon[11];  //


__inline void SendPSChanged(HWND hDlg) {
  g_bApplyClock = TRUE;
  g_bApplyTaskbar = TRUE;
  SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}
/*------------------------------------------------
   Dialog Procedure for the "Format" page
--------------------------------------------------*/
BOOL CALLBACK PageFormatProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg);
			return TRUE;
		case WM_COMMAND:
		{
			WORD id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			if(id == IDC_LOCALE && code == CBN_SELCHANGE)
				OnLocale(hDlg);
			// format textbox
			else if(id == IDC_FORMAT && code == EN_CHANGE)
				SendPSChanged(hDlg);
			// "Custumize Format"
			else if(id == IDC_CUSTOM)
				OnCustom(hDlg, TRUE);
			// "12H"
			else if(id == IDC_12HOUR)
				On12Hour(hDlg);
			// "year" -- "Internet Time"
			else if(IDC_YEAR4 <= id && id <= IDC_AMPM)
				OnFormatCheck(hDlg, id);
			else if(id == IDC_AMSYMBOL && code == CBN_SELCHANGE)
				SendPSChanged(hDlg);
			else if(id == IDC_PMSYMBOL && code == CBN_SELCHANGE)
				SendPSChanged(hDlg);
			else if(id == IDC_ZERO)
				SendPSChanged(hDlg);
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code) {
				case PSN_APPLY:
					OnApply(hDlg);
				  break;
			} return TRUE;

		case WM_DESTROY:
			if(pCustomFormat) {
				free(pCustomFormat);
				pCustomFormat = NULL;
			}
			DestroyWindow(hDlg);
		  break;
	}
  return FALSE;
}

char *entrydate[] = { "Year4", "Year", "Month", "MonthS", "Day", "Weekday",
	"Hour", "Minute", "Second", "Kaigyo", "InternetTime",
	"AMPM", "Hour12", "Custom",  };
#define ENTRY(id) entrydate[(id)-IDC_YEAR4]

/*------------------------------------------------
  Initialize Locale Infomation
--------------------------------------------------*/
void InitLocale(HWND hwnd)
{
	char s[21];
	int i, sel;
	int aLangDayOfWeekIsLast[] =
		{ LANG_JAPANESE, LANG_KOREAN, 0 };
	int aTimeMarkerIsFirst[] = 
		{ LANG_CHINESE, LANG_JAPANESE, LANG_KOREAN, 0 };
	
	if(hwnd)
	{
		sel = (int)(LRESULT)CBGetCurSel(hwnd, IDC_LOCALE);
		ilang = (int)(LRESULT)CBGetItemData(hwnd, IDC_LOCALE, sel);
	}
	else
	{
		ilang = GetMyRegLong("Format", "Locale", (int)GetUserDefaultLangID());
	}
	GetLocaleInfo(ilang, LOCALE_IDATE, s, 20);
	idate = atoi(s);
	GetLocaleInfo(ilang, LOCALE_SABBREVDAYNAME1, sMon, 10);
	
	bDayOfWeekIsLast = FALSE;
	for(i = 0; aLangDayOfWeekIsLast[i]; i++)
	{
		if((ilang & 0x00ff) == aLangDayOfWeekIsLast[i])
		{
			bDayOfWeekIsLast = TRUE; break;
		}
	}
	bTimeMarkerIsFirst = FALSE;
	for(i = 0; aTimeMarkerIsFirst[i]; i++)
	{
		if((ilang & 0x00ff) == aTimeMarkerIsFirst[i])
		{
			bTimeMarkerIsFirst = TRUE; break;
		}
	}
}

/*------------------------------------------------
  for EnumSystemLocales function
--------------------------------------------------*/
BOOL CALLBACK EnumLocalesProc(LPTSTR lpLocaleString)
{
	char s[81];
	int x, index;
	
	x = atox(lpLocaleString);
	if(GetLocaleInfo(x, LOCALE_SLANGUAGE, s, 80) > 0)
		index = (int)(LRESULT)CBAddString(hwndPage, IDC_LOCALE, (LPARAM)s);
	else
		index = (int)(LRESULT)CBAddString(hwndPage, IDC_LOCALE, (LPARAM)lpLocaleString);
	CBSetItemData(hwndPage, IDC_LOCALE, index, x);
	return TRUE;
}

/*------------------------------------------------
  Initialize the "Format" page
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	HFONT hfont;
	char s[MAX_BUFF];
	int i, count, nKaigyo;
	
	char s3[TNY_BUFF] = {0};
	char s2[TNY_BUFF] = {0};
	int ilang;
	
	//ilang = (int)lParam;

	hwndPage = hDlg;
	
	hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	if(hfont)
	{
		SendDlgItemMessage(hDlg, IDC_LOCALE, WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_AMSYMBOL, WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_PMSYMBOL, WM_SETFONT, (WPARAM)hfont, 0);
	}
	hfont = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
	if(hfont)
		SendDlgItemMessage(hDlg, IDC_FORMAT, WM_SETFONT, (WPARAM)hfont, 0);
	
	// Fill and select the "Locale" combobox
	EnumSystemLocales(EnumLocalesProc, LCID_INSTALLED);
	CBSetCurSel(hDlg, IDC_LOCALE, 0);
	ilang = GetMyRegLong("Format", "Locale", (int)GetUserDefaultLangID());
	count = (int)(LRESULT)CBGetCount(hDlg, IDC_LOCALE);
	for(i = 0; i < count; i++)
	{
		int x;
		x = (int)(LRESULT)CBGetItemData(hDlg, IDC_LOCALE, i);
		if(x == ilang)
		{
			CBSetCurSel(hDlg, IDC_LOCALE, i); break;
		}
	}
	
	InitLocale(hDlg);
	
	// "year" -- "second"
	for(i = IDC_YEAR4; i <= IDC_SECOND; i++)
	{
		CheckDlgButton(hDlg, i,
			GetMyRegLong("Format", ENTRY(i), TRUE));
	}
	
	if(IsDlgButtonChecked(hDlg, IDC_YEAR))
		CheckRadioButton(hDlg, IDC_YEAR4, IDC_YEAR, IDC_YEAR);
	if(IsDlgButtonChecked(hDlg, IDC_YEAR4))
		CheckRadioButton(hDlg, IDC_YEAR4, IDC_YEAR, IDC_YEAR4);
	
	if(IsDlgButtonChecked(hDlg, IDC_MONTH))
		CheckRadioButton(hDlg, IDC_MONTH, IDC_MONTHS, IDC_MONTH);
	if(IsDlgButtonChecked(hDlg, IDC_MONTHS))
		CheckRadioButton(hDlg, IDC_MONTH, IDC_MONTHS, IDC_MONTHS);
	
	nKaigyo = GetMyRegLong("Format", ENTRY(IDC_KAIGYO), -1);
	if(nKaigyo < 0)
	{
		RECT rc;
		HWND hwnd;
		nKaigyo = 1;
		hwnd = FindWindow("Shell_TrayWnd", NULL);
		if(hwnd != NULL)
		{
			GetClientRect(hwnd, &rc);
			// if the task bar is positioned horizontally
			if(rc.right > rc.bottom) nKaigyo = 0;
		}
	}
	CheckDlgButton(hDlg, IDC_KAIGYO, nKaigyo);
	
	// "Internet Time" -- "Customize format"
	for(i = IDC_INTERNETTIME; i <= IDC_CUSTOM; i++)
	{
		CheckDlgButton(hDlg, i,
			GetMyRegLong("Format", ENTRY(i), FALSE));
	}
	
	GetMyRegStr("Format", "Format", s, 1024, "");
	SetDlgItemText(hDlg, IDC_FORMAT, s);
	
	pCustomFormat = malloc(MAX_BUFF);
	if(pCustomFormat)
		GetMyRegStr("Format", "CustomFormat", pCustomFormat, MAX_BUFF, "");

	// "AM Symbol" and "PM Symbol"
	CBResetContent(hDlg, IDC_AMSYMBOL);
	GetMyRegStr("Format", "AMsymbol", s3, TNY_BUFF, "");
	if(s3[0]) CBAddString(hDlg, IDC_AMSYMBOL, (LPARAM)s3);
	GetLocaleInfo(ilang, LOCALE_S1159, s2, 10);
	if(s2[0] && strcmp(s, s2) != 0)
		CBAddString(hDlg, IDC_AMSYMBOL, (LPARAM)s2);
	if(strcmp(s, "AM") != 0 && strcmp(s2, "AM") != 0)
		CBAddString(hDlg, IDC_AMSYMBOL, (LPARAM)"AM");

	if(strcmp(s, "am") != 0 && strcmp(s2, "am") != 0)
		CBAddString(hDlg, IDC_AMSYMBOL, (LPARAM)"am");

	if(strcmp(s, "A") != 0 && strcmp(s2, "A") != 0)
		CBAddString(hDlg, IDC_AMSYMBOL, (LPARAM)"A");

	if(strcmp(s, "a") != 0 && strcmp(s2, "a") != 0)
		CBAddString(hDlg, IDC_AMSYMBOL, (LPARAM)"a");

	if(strcmp(s, " ") != 0 && strcmp(s2, " ") != 0)
		CBAddString(hDlg, IDC_AMSYMBOL, (LPARAM)" ");

	CBSetCurSel(hDlg, IDC_AMSYMBOL, 0);
	
	CBResetContent(hDlg, IDC_PMSYMBOL);
	GetMyRegStr("Format", "PMsymbol", s3, 80, "");
	if(s3[0]) CBAddString(hDlg, IDC_PMSYMBOL, (LPARAM)s3);
	GetLocaleInfo(ilang, LOCALE_S2359, s2, 10);
	if(s2[0] && strcmp(s, s2) != 0)
		CBAddString(hDlg, IDC_PMSYMBOL, (LPARAM)s2);
	if(strcmp(s, "PM") != 0 && strcmp(s2, "PM") != 0)
		CBAddString(hDlg, IDC_PMSYMBOL, (LPARAM)"PM");

	if(strcmp(s, "pm") != 0 && strcmp(s2, "pm") != 0)
		CBAddString(hDlg, IDC_PMSYMBOL, (LPARAM)"pm");

	if(strcmp(s, "P") != 0 && strcmp(s2, "P") != 0)
		CBAddString(hDlg, IDC_PMSYMBOL, (LPARAM)"P");

	if(strcmp(s, "p") != 0 && strcmp(s2, "p") != 0)
		CBAddString(hDlg, IDC_PMSYMBOL, (LPARAM)"p");

	CBSetCurSel(hDlg, IDC_PMSYMBOL, 0);
	
	CheckDlgButton(hDlg, IDC_ZERO,
		GetMyRegLong("Format", "HourZero", FALSE));

	On12Hour(hDlg);
	OnCustom(hDlg, FALSE);
}

 //================================================================================================
//---------------------------------------------------------------------------+++--> "Apply" button:
void OnApply(HWND hDlg) { //----------------------------------------------------------------+++-->
	char s[1024];
	int i;
	
  SetMyRegLong("Format", "Locale",
	  (DWORD)(LRESULT)CBGetItemData(hDlg, IDC_LOCALE, CBGetCurSel(hDlg, IDC_LOCALE)));
	
  for(i = IDC_YEAR4; i <= IDC_CUSTOM; i++) {
	  SetMyRegLong("Format", ENTRY(i), IsDlgButtonChecked(hDlg, i));
  }
	
  GetDlgItemText(hDlg, IDC_AMSYMBOL, s, 1024);
  SetMyRegStr("Format", "AMsymbol", s);
  GetDlgItemText(hDlg, IDC_PMSYMBOL, s, 1024);
  SetMyRegStr("Format", "PMsymbol", s);
	
  SetMyRegLong("Format", "HourZero", IsDlgButtonChecked(hDlg, IDC_ZERO));

  GetDlgItemText(hDlg, IDC_FORMAT, s, 1024);
  SetMyRegStr("Format", "Format", s);
	
  if(pCustomFormat) {
	  if(IsDlgButtonChecked(hDlg, IDC_CUSTOM)) {
		  strcpy(pCustomFormat, s);
		  SetMyRegStr("Format", "CustomFormat", pCustomFormat);
	  }
  }
}
 //================================================================================================
//-------------------------------------------+++--> When User's Location (Locale ComboBox) Changes:
void OnLocale(HWND hDlg) { //---------------------------------------------------------------+++-->
  InitLocale(hDlg);
  OnCustom(hDlg, FALSE);
}
 //================================================================================================
//-----------------------------------+++--> Handler for Enable/Disable "Customize format" CheckBox:
void OnCustom(HWND hDlg, BOOL bmouse) { //--------------------------------------------------+++-->
	BOOL b;
	int i;
	
  b = IsDlgButtonChecked(hDlg, IDC_CUSTOM);
  EnableDlgItem(hDlg, IDC_FORMAT, b);
	
  for(i = IDC_YEAR4; i <= IDC_AMPM; i++)
	  EnableDlgItem(hDlg, i, !b);

  EnableDlgItem(hDlg, IDC_ZERO, !b);
  EnableDlgItem(hDlg, IDC_LABAMSYMBOL, !b);
  EnableDlgItem(hDlg, IDC_LABPMSYMBOL, !b);

  if(pCustomFormat && bmouse) {
	  if(b) {
		  if(pCustomFormat[0])
			  SetDlgItemText(hDlg, IDC_FORMAT, pCustomFormat);
	  }else{
		  GetDlgItemText(hDlg, IDC_FORMAT, pCustomFormat, 1024);
	  }
  }
	
  if(!b) OnFormatCheck(hDlg, 0);
  SendPSChanged(hDlg);
}
 //================================================================================================
//-----------------------------------------+++--> Toggle Display Between 12 & 24 Hour Time Formats:
void On12Hour(HWND hDlg) { //---------------------------------------------------------------+++-->
	BOOL b;

  b = IsDlgButtonChecked(hDlg, IDC_12HOUR);
  if(!b) {
	  CheckDlgButton(hDlg, IDC_AMPM, 0);
	  if(!IsDlgButtonChecked(hDlg, IDC_CUSTOM)) OnFormatCheck(hDlg, 0);
  }
  SendPSChanged(hDlg);
}
/*------------------------------------------------
  When clicked "year" -- "am/pm"
--------------------------------------------------*/

#define CHECKS(a) checks[(a)-IDC_YEAR4]

void OnFormatCheck(HWND hDlg, WORD id)
{
	char s[1024];
	int checks[15];
	int i;
	
	for(i = IDC_YEAR4; i <= IDC_AMPM; i++)
	{
		CHECKS(i) = IsDlgButtonChecked(hDlg, i);
	}
	
	if(id == IDC_YEAR4 || id == IDC_YEAR)
	{
		if(id == IDC_YEAR4 && CHECKS(IDC_YEAR4))
		{
			CheckRadioButton(hDlg, IDC_YEAR4, IDC_YEAR, IDC_YEAR4);
			CHECKS(IDC_YEAR) = FALSE;
		}
		if(id == IDC_YEAR && CHECKS(IDC_YEAR))
		{
			CheckRadioButton(hDlg, IDC_YEAR4, IDC_YEAR, IDC_YEAR);
			CHECKS(IDC_YEAR4) = FALSE;
		}
	}
	
	if(id == IDC_MONTH || id == IDC_MONTHS)
	{
		if(id == IDC_MONTH && CHECKS(IDC_MONTH))
		{
			CheckRadioButton(hDlg, IDC_MONTH, IDC_MONTHS, IDC_MONTH);
			CHECKS(IDC_MONTHS) = FALSE;
		}
		if(id == IDC_MONTHS && CHECKS(IDC_MONTHS))
		{
			CheckRadioButton(hDlg, IDC_MONTH, IDC_MONTHS, IDC_MONTHS);
			CHECKS(IDC_MONTH) = FALSE;
		}
	}
	
	if(id == IDC_AMPM)
	{
		CheckDlgButton(hDlg, IDC_12HOUR, 1);
		On12Hour(hDlg);
	}
	
	CreateFormat(s, checks);
	SetDlgItemText(hDlg, IDC_FORMAT, s);
	SendPSChanged(hDlg);
}

/*------------------------------------------------
  Initialize a format string. Called from main.c
--------------------------------------------------*/
void InitFormat(void)
{
	char s[1024];
	int i, checks[15];
	RECT rc;
	HWND hwnd;
	BOOL b;
	
	if(GetMyRegLong("Format", ENTRY(IDC_CUSTOM), FALSE))
		return;
	
	InitLocale(NULL);
	
	for(i = IDC_YEAR4; i <= IDC_SECOND; i++)
	{
		CHECKS(i) = GetMyRegLong("Format", ENTRY(i), TRUE);
	}
	
	if(CHECKS(IDC_YEAR))  CHECKS(IDC_YEAR4) = FALSE;
	if(CHECKS(IDC_YEAR4)) CHECKS(IDC_YEAR) = FALSE;
	
	if(CHECKS(IDC_MONTH))  CHECKS(IDC_MONTHS) = FALSE;
	if(CHECKS(IDC_MONTHS)) CHECKS(IDC_MONTH) = FALSE;
	
	CHECKS(IDC_INTERNETTIME) = GetMyRegLong("Format",
		ENTRY(IDC_INTERNETTIME), FALSE);
	
	b = FALSE;
	hwnd = FindWindow("Shell_TrayWnd", NULL);
	if(hwnd != NULL)
	{
		GetClientRect(hwnd, &rc);
		if(rc.right < rc.bottom) b = TRUE;
	}
	CHECKS(IDC_KAIGYO) = 
		GetMyRegLong("Format", ENTRY(IDC_KAIGYO), b);
	CHECKS(IDC_AMPM) = GetMyRegLong("Format", ENTRY(IDC_AMPM), FALSE);
	
	CreateFormat(s, checks);
	SetMyRegStr("Format", "Format", s);
}

/*--------------------------------------------------
=============== Create a format string automatically
--------------------------------------------------*/
void CreateFormat(char* dst, int* checks) {
	BOOL bdate = FALSE, btime = FALSE;
	int i;
	
  for(i = IDC_YEAR4; i <= IDC_WEEKDAY; i++) {
	  if(CHECKS(i)) {
		 bdate = TRUE;
		 break;
	  }
  }

  for(i = IDC_HOUR; i <= IDC_AMPM; i++) {
	  if(CHECKS(i)) {
		 btime = TRUE;
		 break;
	  }
  }
	
  dst[0] = 0;
	
  if(!bDayOfWeekIsLast && CHECKS(IDC_WEEKDAY)) {
	 strcat(dst, "ddd");
	 for(i = IDC_YEAR4; i <= IDC_DAY; i++) {
		 if(CHECKS(i)) {
				if((ilang & 0x00ff) == LANG_CHINESE) strcat(dst, " ");
				else if(sMon[0] && sMon[ strlen(sMon) - 1 ] == '.')
					strcat(dst, " ");
				else strcat(dst, ", ");
				break;
			}
		}
	}
	
	if(idate == 0)
	{
		if(CHECKS(IDC_MONTH) || CHECKS(IDC_MONTHS))
		{
			if(CHECKS(IDC_MONTH)) strcat(dst, "mm");
			if(CHECKS(IDC_MONTHS)) strcat(dst, "mmm");
			if(CHECKS(IDC_DAY) || CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR))
			{
				if(CHECKS(IDC_MONTH)) strcat(dst, "/");
				else strcat(dst, " ");
			}
		}
		if(CHECKS(IDC_DAY))
		{
			strcat(dst, "dd");
			if(CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR))
			{
				if(CHECKS(IDC_MONTH)) strcat(dst, "/");
				else strcat(dst, ", ");
			}
		}
		if(CHECKS(IDC_YEAR4)) strcat(dst, "yyyy");
		if(CHECKS(IDC_YEAR)) strcat(dst, "yy");
	}
	else if(idate == 1)
	{
		if(CHECKS(IDC_DAY))
		{
			strcat(dst, "dd");
			if(CHECKS(IDC_MONTH) || CHECKS(IDC_MONTHS))
			{
				if(CHECKS(IDC_MONTH)) strcat(dst, "/");
				else strcat(dst, " ");
			}
			else if(CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR)) strcat(dst, "/");
		}
		if(CHECKS(IDC_MONTH) || CHECKS(IDC_MONTHS))
		{
			if(CHECKS(IDC_MONTH)) strcat(dst, "mm");
			if(CHECKS(IDC_MONTHS)) strcat(dst, "mmm");
			if(CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR))
			{
				if(CHECKS(IDC_MONTH)) strcat(dst, "/");
				else strcat(dst, " ");
			}
		}
		if(CHECKS(IDC_YEAR4)) strcat(dst, "yyyy");
		if(CHECKS(IDC_YEAR)) strcat(dst, "yy");
	}
	else
	{
		if(CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR))
		{
			if(CHECKS(IDC_YEAR4)) strcat(dst, "yyyy");
			if(CHECKS(IDC_YEAR)) strcat(dst, "yy");
			if(CHECKS(IDC_MONTH) || CHECKS(IDC_MONTHS)
				|| CHECKS(IDC_DAY))
			{
				if(CHECKS(IDC_MONTHS)) strcat(dst, " ");
				else strcat(dst, "/");
			}
		}
		if(CHECKS(IDC_MONTH) || CHECKS(IDC_MONTHS))
		{
			if(CHECKS(IDC_MONTH)) strcat(dst, "mm");
			if(CHECKS(IDC_MONTHS)) strcat(dst, "mmm");
			if(CHECKS(IDC_DAY))
			{
				if(CHECKS(IDC_MONTHS)) strcat(dst, " ");
				else strcat(dst, "/");
			}
		}
		if(CHECKS(IDC_DAY)) strcat(dst, "dd");
	}
	
	if(bDayOfWeekIsLast && CHECKS(IDC_WEEKDAY))
	{
		for(i = IDC_YEAR4; i <= IDC_DAY; i++)
		{
			if(CHECKS(i)) { strcat(dst, " "); break; }
		}
		strcat(dst, "ddd");
	}
	
	if(bdate && btime)
	{
		if(CHECKS(IDC_KAIGYO)) strcat(dst, "\\n");
		else
		{
			if(idate < 2 && CHECKS(IDC_MONTHS) &&
				(CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR)))
				strcat(dst, " ");
			strcat(dst, " ");
		}
	}
	
	if(bTimeMarkerIsFirst && CHECKS(IDC_AMPM))
	{
		strcat(dst, "tt");
		if(CHECKS(IDC_HOUR) || CHECKS(IDC_MINUTE) || 
			CHECKS(IDC_SECOND) || CHECKS(IDC_INTERNETTIME))
			strcat(dst, " ");
	}
	
	if(CHECKS(IDC_HOUR))
	{
		strcat(dst, "hh");
		if(CHECKS(IDC_MINUTE) || CHECKS(IDC_SECOND)) strcat(dst, ":");
		else if(CHECKS(IDC_INTERNETTIME) || 
			(!bTimeMarkerIsFirst && CHECKS(IDC_AMPM))) strcat(dst, " ");
	}
	if(CHECKS(IDC_MINUTE))
	{
		strcat(dst, "nn");
		if(CHECKS(IDC_SECOND)) strcat(dst, ":");
		else if(CHECKS(IDC_INTERNETTIME) || 
			(!bTimeMarkerIsFirst && CHECKS(IDC_AMPM))) strcat(dst, " ");
	}
	if(CHECKS(IDC_SECOND))
	{
		strcat(dst, "ss");
		if(CHECKS(IDC_INTERNETTIME) || 
			(!bTimeMarkerIsFirst && CHECKS(IDC_AMPM))) strcat(dst, " ");
	}
	
	if(!bTimeMarkerIsFirst && CHECKS(IDC_AMPM))
	{
		strcat(dst, "tt");
		if(CHECKS(IDC_INTERNETTIME)) strcat(dst, " ");
	}
	
	if(CHECKS(IDC_INTERNETTIME)) strcat(dst, "@@@");
}