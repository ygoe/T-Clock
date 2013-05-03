/*-------------------------------------------
  pagealarm.c
  "Alarm" page of properties
  KAZUBON 1997-2001
---------------------------------------------*/
// Last Modified by Stoic Joker: Sunday, 03/13/2011 @ 11:54:05am
#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void ReadAlarmFromReg(PALARMSTRUCT pAS, int num);
static void SaveAlarmToReg(PALARMSTRUCT pAS, int num);
static void GetAlarmFromDlg(HWND hDlg, PALARMSTRUCT pAS);
static void SetAlarmToDlg(HWND hDlg, PALARMSTRUCT pAS);
static void OnChangeAlarm(HWND hDlg);
static void OnDropDownAlarm(HWND hDlg);
static void OnDay(HWND hDlg);
static void OnAlermJihou(HWND hDlg, WORD id);
static void OnSanshoAlarm(HWND hDlg, WORD id);
static void On12Hour(HWND hDlg);
static void OnDelAlarm(HWND hDlg);
static void OnFileChange(HWND hDlg, WORD id);
static void OnTest(HWND hDlg, WORD id);
static void OnMsgAlarm(HWND hDlg, WORD id);
static void FormatTimeText(HWND hDlg, WORD id);
static void EnableAMPM(HWND hDlg, BOOL bAP);
static void ToggleAMPMDisplay(HWND hDlg);

static int curAlarm;
static BOOL bPlaying = FALSE;
BOOL bFTT = TRUE;

#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0)

/*------------------------------------------------
  Dialog procedure
--------------------------------------------------*/
BOOL CALLBACK PageAlarmProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	WORD id, code;

	id = LOWORD(wParam);
	code = HIWORD(wParam);

	switch(message) {
		case WM_INITDIALOG:
			OnInit(hDlg);
			return TRUE;

		case WM_COMMAND:
		{
			// a combo-box to select alarm name
			if(id == IDC_COMBOALARM)
			{
				if(code == CBN_SELCHANGE) {
				   OnChangeAlarm(hDlg);
				   FormatTimeText(hDlg, IDC_COMBOALARM);
				   FormatTimeText(hDlg, IDC_MINUTEALARM);
				}
				else if(code == CBN_DROPDOWN) OnDropDownAlarm(hDlg);
			}
			// checked "active" or "Play sound every hour"
			else if(id == IDC_ALARM || id == IDC_JIHOU) OnAlermJihou(hDlg, id);

			// time setting changed
			else if((id == IDC_HOURALARM || id == IDC_MINUTEALARM) && code == EN_CHANGE) {
//			else if((id == IDC_SPINHOUR || id == IDC_SPINMINUTE) && code == EN_CHANGE) {
				if(bFTT) FormatTimeText(hDlg, id);
				SendPSChanged(hDlg);
				bFTT = FALSE;//TRUE;
			}
			else if(id == IDC_ALARMDAY) OnDay(hDlg); // Day...

			else if(id == IDCB_MSG_ALARM) OnMsgWindOpt(hDlg); // Message Window Options
				
			// file name changed
			else if((id == IDC_FILEALARM || id == IDC_FILEJIHOU) && code == EN_CHANGE) {
				OnFileChange(hDlg, id);
				SendPSChanged(hDlg);
			}
			// browse file
			else if(id == IDC_SANSHOALARM || id == IDC_SANSHOJIHOU) {
				OnSanshoAlarm(hDlg, id);
				OnFileChange(hDlg, (WORD)(id - 1));
				SendPSChanged(hDlg);
			}
			// checked "12 hour"
			else if(id == IDC_12HOURALARM) {
				FormatTimeText(hDlg, id);
				On12Hour(hDlg);
			}
			// Checked PM - Toggle Display of AM or PM 
			else if(id == IDC_AMPM_CHECK) {
				ToggleAMPMDisplay(hDlg);
				SendPSChanged(hDlg);
			}

			// checked other checkboxes
			else if(id == IDC_CHIMEALARM || id == IDC_REPEATALARM || id == IDC_REPEATJIHOU ||
					id == IDC_BLINKALARM || id == IDC_BLINKJIHOU) {
						if(id == IDC_CHIMEALARM) {
							CheckDlgButton(hDlg, IDC_REPEATALARM, FALSE);
							EnableDlgItem(hDlg, IDC_REPEATIMES, FALSE);
							EnableDlgItem(hDlg, IDC_SPINTIMES, FALSE);
						}
						if(id == IDC_REPEATALARM) {
							CheckDlgButton(hDlg, IDC_CHIMEALARM, FALSE);
							EnableDlgItem(hDlg, IDC_REPEATIMES, TRUE);
							EnableDlgItem(hDlg, IDC_SPINTIMES, TRUE);
						}
						SendPSChanged(hDlg);
			}

			// delete an alarm
			else if(id == IDC_DELALARM) OnDelAlarm(hDlg);

			// test sound
			else if(id == IDC_TESTALARM || id == IDC_TESTJIHOU) OnTest(hDlg, id);

			else if(id == IDC_MSG_ALARM) {
				OnMsgAlarm(hDlg, id);
				SendPSChanged(hDlg);
			}

		  return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code) {
				case PSN_APPLY:
				  OnApply(hDlg);
				 break;

				case UDN_DELTAPOS:
					bFTT = TRUE;
					break;

			} return TRUE; //--+++--> End Of Case WM_NOTIFY:

		case WM_DESTROY:
		  if(bPlaying) StopFile(); bPlaying = FALSE;
		  DestroyWindow(hDlg);
		  break;

		// playing sound ended
		case MM_MCINOTIFY:
		case MM_WOM_DONE:
			StopFile(); bPlaying = FALSE;
			SendDlgItemMessage(hDlg, IDC_TESTALARM, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)g_hIconPlay);
			SendDlgItemMessage(hDlg, IDC_TESTJIHOU, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)g_hIconPlay);
		return TRUE;
	}
  return FALSE;
}
/*------------------------------------------------
  initialize
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	char s[1024] = "";
	int i, count, index;
	HFONT hfont;
	HBITMAP hBMPJRPic;

	hBMPJRPic = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 64, 80, LR_DEFAULTCOLOR);
	SendDlgItemMessage(hDlg, IDC_BMPJACK, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBMPJRPic);
	
	hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	if(hfont)
	{
		SendDlgItemMessage(hDlg, IDC_COMBOALARM,
			WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_FILEALARM,
			WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_FILEJIHOU,
			WM_SETFONT, (WPARAM)hfont, 0);
	}
	
	index = (int)(LRESULT)CBAddString(hDlg, IDC_COMBOALARM, (LPARAM)MyString(IDS_ADDALARM));
	CBSetItemData(hDlg, IDC_COMBOALARM, index, 0);
	
	count = GetMyRegLong("", "AlarmNum", 0);
	if(count < 1) count = 0;
	for(i = 0; i < count; i++)
	{
		PALARMSTRUCT pAS;
		pAS = malloc(sizeof(ALARMSTRUCT));
		ReadAlarmFromReg(pAS, i);
		index = (int)(LRESULT)CBAddString(hDlg, IDC_COMBOALARM, (LPARAM)pAS->name);
		CBSetItemData(hDlg, IDC_COMBOALARM, index, (LPARAM)pAS);
		if(i == 0)  SetAlarmToDlg(hDlg, pAS);
	}
	if(count > 0)
	{
		CBSetCurSel(hDlg, IDC_COMBOALARM, 1);
		curAlarm = 1;
	}
	else
	{
		ALARMSTRUCT as;
		CBSetCurSel(hDlg, IDC_COMBOALARM, 0);
		curAlarm = -1;
		memset(&as, 0, sizeof(as));
		as.hour = 12;
		as.days = 0x7f;
		SetAlarmToDlg(hDlg, &as);
	}
	
	CheckDlgButton(hDlg, IDC_JIHOU,
		GetMyRegLong("", "Jihou", FALSE));
	
	GetMyRegStr("", "JihouFile", s, 1024, "");
	SetDlgItemText(hDlg, IDC_FILEJIHOU, s);
	
	CheckDlgButton(hDlg, IDC_REPEATJIHOU,
		GetMyRegLong("", "JihouRepeat", FALSE));
	
	CheckDlgButton(hDlg, IDC_BLINKJIHOU,
		GetMyRegLong("", "JihouBlink", FALSE));
	
	OnAlermJihou(hDlg, IDC_JIHOU);
	
	SendDlgItemMessage(hDlg, IDC_TESTALARM, BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)g_hIconPlay);
	OnFileChange(hDlg, IDC_FILEALARM);
	SendDlgItemMessage(hDlg, IDC_TESTJIHOU, BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)g_hIconPlay);
	OnFileChange(hDlg, IDC_FILEJIHOU);
	
	SendDlgItemMessage(hDlg, IDC_DELALARM, BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)g_hIconDel);
	
	bPlaying = FALSE;
	FormatTimeText(hDlg, 0);
	FormatTimeText(hDlg, IDC_MINUTEALARM);
}

/*------------------------------------------------
   apply - save settings
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char s[1024];
	int i, count, n_alarm;
	PALARMSTRUCT pAS;
	
	n_alarm = 0;
	
	if(curAlarm < 0)
	{
		char name[40];
		GetDlgItemText(hDlg, IDC_COMBOALARM, name, 40);
		if(name[0] && IsDlgButtonChecked(hDlg, IDC_ALARM))
		{
			pAS = malloc(sizeof(ALARMSTRUCT));
			if(pAS)
			{
				int index;
				GetAlarmFromDlg(hDlg, pAS);
				index = (int)(LRESULT)CBAddString(hDlg, IDC_COMBOALARM, (LPARAM)pAS->name);
				CBSetItemData(hDlg, IDC_COMBOALARM, index, (LPARAM)pAS);
				curAlarm = index;
				CBSetCurSel(hDlg, IDC_COMBOALARM, index);
				EnableDlgItem(hDlg, IDC_DELALARM, TRUE);
			}
		}
	}
	else
	{
		pAS = (PALARMSTRUCT)CBGetItemData(hDlg, IDC_COMBOALARM, curAlarm);
		if(pAS)
			GetAlarmFromDlg(hDlg, pAS);
	}
	
	count = (int)(LRESULT)CBGetCount(hDlg, IDC_COMBOALARM);
	for(i = 0; i < count; i++)
	{
		PALARMSTRUCT pAS;
		pAS = (PALARMSTRUCT)CBGetItemData(hDlg, IDC_COMBOALARM, i);
		if(pAS)
		{
			SaveAlarmToReg(pAS, n_alarm);
			n_alarm++;
		}
	}
	for(i = n_alarm; ; i++)
	{
		char subkey[20];
		wsprintf(subkey, "Alarm%d", i + 1);
		if(GetMyRegLong(subkey, "Hour", -1) >= 0)
			DelMyRegKey(subkey);
		else break;
	}
	
	SetMyRegLong("", "AlarmNum", n_alarm);
	
	SetMyRegLong("", "Jihou",
		IsDlgButtonChecked(hDlg, IDC_JIHOU));
	
	GetDlgItemText(hDlg, IDC_FILEJIHOU, s, 1024);
	SetMyRegStr("", "JihouFile", s);
	
	SetMyRegLong("", "JihouRepeat",
		IsDlgButtonChecked(hDlg, IDC_REPEATJIHOU));
	SetMyRegLong("", "JihouBlink",
		IsDlgButtonChecked(hDlg, IDC_BLINKJIHOU));
	
	InitAlarm(); // alarm.c
}
 //================================================================================================
//--------------------------------------------------//----+++--> Read Alarm Settings From Registry:
void ReadAlarmFromReg(PALARMSTRUCT pAS, int num) { //---------------------------------------+++-->
	char subkey[20];
	
	wsprintf(subkey, "Alarm%d", num + 1);
	
	GetMyRegStr(subkey, "Name", pAS->name, 40, "");
	pAS->bAlarm = GetMyRegLong(subkey, "Alarm", FALSE);
	pAS->hour = GetMyRegLong(subkey, "Hour", 12);
	pAS->minute = GetMyRegLong(subkey, "Minute", 0);
	GetMyRegStr(subkey, "File", pAS->fname, MAX_BUFF, "");

	GetMyRegStr(subkey, "jrMessage", pAS->jrMessage, MAX_BUFF, "");
	GetMyRegStr(subkey, "jrSettings", pAS->jrSettings, TNY_BUFF, "");
	pAS->jrMsgUsed = GetMyRegLong(subkey, "jrMsgUsed", FALSE);

	pAS->bHour12 = GetMyRegLong(subkey, "Hour12", TRUE);
	pAS->bChimeHr = GetMyRegLong(subkey, "ChimeHr", FALSE);
	pAS->bRepeat = GetMyRegLong(subkey, "Repeat", FALSE);
	pAS->iTimes = GetMyRegLong(subkey, "Times", 1);
	pAS->bBlink = GetMyRegLong(subkey, "Blink", FALSE);
	pAS->days = GetMyRegLong(subkey, "Days", 0x7f);
	pAS->bPM = GetMyRegLong(subkey, "PM", FALSE);
	
	if(pAS->name[0] == 0)
		wsprintf(pAS->name, "%02d:%02d", pAS->hour, pAS->minute);
}
 //================================================================================================
//------------------------------------------------//--------+++--> Save Alarm Settings in Registry:
void SaveAlarmToReg(PALARMSTRUCT pAS, int num) { //-----------------------------------------+++-->
	char subkey[20];
	
	wsprintf(subkey, "Alarm%d", num + 1);
	SetMyRegStr(subkey, "Name", pAS->name);
	SetMyRegLong(subkey, "Alarm", pAS->bAlarm);
	SetMyRegLong(subkey, "Hour", pAS->hour);
	SetMyRegLong(subkey, "Minute", pAS->minute);
	SetMyRegStr(subkey, "File", pAS->fname);

	SetMyRegStr(subkey, "jrMessage", pAS->jrMessage);
	SetMyRegStr(subkey, "jrSettings", pAS->jrSettings);
	SetMyRegLong(subkey, "jrMsgUsed", pAS->jrMsgUsed);

	SetMyRegLong(subkey, "Hour12", pAS->bHour12);
	SetMyRegLong(subkey, "ChimeHr", pAS->bChimeHr);
	SetMyRegLong(subkey, "Repeat", pAS->bRepeat);
	SetMyRegLong(subkey, "Times", pAS->iTimes);
	SetMyRegLong(subkey, "Blink", pAS->bBlink);
	SetMyRegLong(subkey, "Days", pAS->days);
	SetMyRegLong(subkey, "PM", pAS->bPM);
}
 //================================================================================================
//------------------------------------+++--> Load Current Alarm Setting From Dialog into Structure:
void GetAlarmFromDlg(HWND hDlg, PALARMSTRUCT pAS) { //--------------------------------------+++-->
	GetDlgItemText(hDlg, IDC_COMBOALARM, pAS->name, 40);
	pAS->bAlarm = IsDlgButtonChecked(hDlg, IDC_ALARM);

	pAS->hour = (int)(LRESULT)SendDlgItemMessage(hDlg, IDC_SPINHOUR, UDM_GETPOS, 0, 0);
	pAS->minute = (int)(LRESULT)SendDlgItemMessage(hDlg, IDC_SPINMINUTE, UDM_GETPOS, 0, 0);
	GetDlgItemText(hDlg, IDC_FILEALARM, pAS->fname, MAX_PATH);

	GetDlgItemText(hDlg, IDC_JRMSG_TEXT, pAS->jrMessage, MAX_BUFF);
	GetDlgItemText(hDlg, IDC_JR_SETTINGS, pAS->jrSettings, TNY_BUFF);
	pAS->jrMsgUsed = IsDlgButtonChecked(hDlg, IDC_MSG_ALARM);

	pAS->bHour12 = IsDlgButtonChecked(hDlg, IDC_12HOURALARM);
	pAS->bChimeHr = IsDlgButtonChecked(hDlg, IDC_CHIMEALARM);
	pAS->bRepeat = IsDlgButtonChecked(hDlg, IDC_REPEATALARM);
	pAS->iTimes = GetDlgItemInt(hDlg, IDC_REPEATIMES, NULL, FALSE);
	pAS->bBlink = IsDlgButtonChecked(hDlg, IDC_BLINKALARM);
	pAS->days = GetDlgItemInt(hDlg, IDC_HIDDENALARMDAY, NULL, FALSE);
	pAS->bPM = IsDlgButtonChecked(hDlg, IDC_AMPM_CHECK);
}
 //================================================================================================
//-------------------------------------------------+++--> Load Dialog With Settings for This Alarm:
void SetAlarmToDlg(HWND hDlg, PALARMSTRUCT pAS) { //----------------------------------------+++-->
	SetDlgItemText(hDlg, IDC_COMBOALARM, pAS->name);
	CheckDlgButton(hDlg, IDC_ALARM, pAS->bAlarm);

	SendDlgItemMessage(hDlg, IDC_SPINHOUR, UDM_SETRANGE, 0, MAKELONG(23, 0));
	SendDlgItemMessage(hDlg, IDC_SPINHOUR, UDM_SETPOS, 0, pAS->hour);
	SendDlgItemMessage(hDlg, IDC_SPINMINUTE, UDM_SETRANGE, 0, MAKELONG(59, 0));
	SendDlgItemMessage(hDlg, IDC_SPINMINUTE, UDM_SETPOS, 0, pAS->minute);
	SendDlgItemMessage(hDlg, IDC_SPINTIMES, UDM_SETRANGE, 0, MAKELONG(42, 1));
	SendDlgItemMessage(hDlg, IDC_SPINTIMES, UDM_SETPOS, 0, pAS->iTimes);
	SetDlgItemText(hDlg, IDC_FILEALARM, pAS->fname);

	SetDlgItemText(hDlg, IDC_JRMSG_TEXT, pAS->jrMessage);
	SetDlgItemText(hDlg, IDC_JR_SETTINGS, pAS->jrSettings);
	CheckDlgButton(hDlg, IDC_MSG_ALARM, pAS->jrMsgUsed);

	CheckDlgButton(hDlg, IDC_12HOURALARM, pAS->bHour12);
	CheckDlgButton(hDlg, IDC_CHIMEALARM, pAS->bChimeHr);
	CheckDlgButton(hDlg, IDC_REPEATALARM, pAS->bRepeat);
	CheckDlgButton(hDlg, IDC_BLINKALARM, pAS->bBlink);
	SetDlgItemInt(hDlg, IDC_HIDDENALARMDAY, pAS->days, FALSE);
	CheckDlgButton(hDlg, IDC_AMPM_CHECK, pAS->bPM);
	
	On12Hour(hDlg);
	OnFileChange(hDlg, IDC_FILEALARM);
	OnMsgAlarm(hDlg, IDC_MSG_ALARM);
	OnAlermJihou(hDlg, IDC_ALARM);
	ToggleAMPMDisplay(hDlg);

	EnableDlgItem(hDlg, IDC_REPEATIMES, pAS->bRepeat);
	EnableDlgItem(hDlg, IDC_SPINTIMES, pAS->bRepeat);
}
/*------------------------------------------------
   selected an alarm name by combobox
--------------------------------------------------*/
void OnChangeAlarm(HWND hDlg)
{
	PALARMSTRUCT pAS;
	int index;
	
	index = (int)(LRESULT)CBGetCurSel(hDlg, IDC_COMBOALARM);
	if(curAlarm >= 0 && index == curAlarm) return;
	
	if(curAlarm < 0)
	{
		char name[40];
		GetDlgItemText(hDlg, IDC_COMBOALARM, name, 40);
		if(name[0] && IsDlgButtonChecked(hDlg, IDC_ALARM))
		{
			pAS = malloc(sizeof(ALARMSTRUCT));
			if(pAS)
			{
				int index;
				GetAlarmFromDlg(hDlg, pAS);
				index = (int)(LRESULT)CBAddString(hDlg, IDC_COMBOALARM, (LPARAM)pAS->name);
				CBSetItemData(hDlg, IDC_COMBOALARM, index, (LPARAM)pAS);
				curAlarm = index;
			}
		}
	}
	else
	{
		pAS = (PALARMSTRUCT)CBGetItemData(hDlg, IDC_COMBOALARM, curAlarm);
		if(pAS) GetAlarmFromDlg(hDlg, pAS);
	}
	
	pAS = (PALARMSTRUCT)CBGetItemData(hDlg, IDC_COMBOALARM, index);
	if(pAS)
	{
		SetAlarmToDlg(hDlg, pAS);
		EnableDlgItem(hDlg, IDC_DELALARM, TRUE);
		curAlarm = index;
	}
	else
	{
		ALARMSTRUCT as;
		memset(&as, 0, sizeof(as));
		as.hour = 12;
		as.days = 0x7f;
		SetAlarmToDlg(hDlg, &as);
		EnableDlgItem(hDlg, IDC_DELALARM, FALSE);
		curAlarm = -1;
	}
}
/*------------------------------------------------
  combo box is about to be made visible
--------------------------------------------------*/
void OnDropDownAlarm(HWND hDlg)
{
	PALARMSTRUCT pAS;
	char name[40];
	int index;
	
	if(curAlarm < 0) return;
	pAS = (PALARMSTRUCT)CBGetItemData(hDlg, IDC_COMBOALARM, curAlarm);
	if(pAS == 0) return;
	GetDlgItemText(hDlg, IDC_COMBOALARM, name, 40);
	if(strcmp(name, pAS->name) != 0)
	{
		strcpy(pAS->name, name);
		CBDeleteString(hDlg, IDC_COMBOALARM, curAlarm);
		index = (int)(LRESULT)CBInsertString(hDlg, IDC_COMBOALARM, curAlarm, name);
		CBSetItemData(hDlg, IDC_COMBOALARM, index, (LPARAM)pAS);
		CBSetCurSel(hDlg, IDC_COMBOALARM, index);
		curAlarm = index;
	}
}

/*------------------------------------------------
  "Day..."
--------------------------------------------------*/
void OnDay(HWND hDlg)
{
	int nOld, nNew;
	
	nOld = GetDlgItemInt(hDlg, IDC_HIDDENALARMDAY, NULL, FALSE);
	nNew = SetAlarmDay(hDlg, nOld);
	if(nNew != nOld)
	{
		SetDlgItemInt(hDlg, IDC_HIDDENALARMDAY, nNew, FALSE);
		SendPSChanged(hDlg);
	}
}
/*------------------------------------------------
  checked "Enable" or "Play sound every hour"
--------------------------------------------------*/
void OnAlermJihou(HWND hDlg, WORD id)
{
	int s, e, i;
	BOOL b;
	
	b = IsDlgButtonChecked(hDlg, id);

	if(id == IDC_ALARM) {
		s = IDC_LABTIMEALARM; e = IDC_BLINKALARM;
		EnableDlgItem(hDlg, IDC_CHIMEALARM, b); // Lazy, I know...
		EnableDlgItem(hDlg, IDC_REPEATIMES, b); // Lazy, I know...
		EnableDlgItem(hDlg, IDC_SPINTIMES, b);  // Lazy, I know...
		//--+++--> ???????????????????????????????????????
		EnableWindow(GetDlgItem(hDlg, IDC_MSG_ALARM), b);
		if(b){
			OnMsgAlarm(hDlg, IDC_MSG_ALARM);
		}else{
			ShowWindow(GetDlgItem(hDlg, IDC_BMPJACK), SW_HIDE);
			EnableWindow(GetDlgItem(hDlg, IDCB_MSG_ALARM), FALSE);
		}
	}else{
		s = IDC_LABSOUNDJIHOU; e = IDC_BLINKJIHOU;
	}
	
	for(i = s; i <= e; i++)	EnableDlgItem(hDlg, i, b);
	
	if(id == IDC_ALARM)
		OnFileChange(hDlg, IDC_FILEALARM);
	else
		OnFileChange(hDlg, IDC_FILEJIHOU);
	
	if(b && id == IDC_ALARM)
	{
		char name[40];
		GetDlgItemText(hDlg, IDC_COMBOALARM, name, 40);
		if(strcmp(name, MyString(IDS_ADDALARM)) == 0)
			SetDlgItemText(hDlg, IDC_COMBOALARM, "");
		EnableAMPM(hDlg, IsDlgButtonChecked(hDlg, IDC_12HOURALARM));
	}
	
	SendPSChanged(hDlg);
}
 //-------------------------------------//------------------------------------+++-->
//-----------------------------+++--> Show/Hide the Bouncing Message Window Options:
void OnMsgAlarm(HWND hDlg, WORD id) { //-------------------------------------+++-->
	BOOL b; //--+++--> Here Begins the Jack Russel Terrier Window Feature!
	
  b = IsDlgButtonChecked(hDlg, id);
  if(b) {
	  ShowWindow(GetDlgItem(hDlg, IDC_BMPJACK), SW_SHOW);
	  EnableWindow(GetDlgItem(hDlg, IDCB_MSG_ALARM), TRUE);
  }else{
	  ShowWindow(GetDlgItem(hDlg, IDC_BMPJACK), SW_HIDE);
	  EnableWindow(GetDlgItem(hDlg, IDCB_MSG_ALARM), FALSE);
  }
}
/*------------------------------------------------
  browse sound file
--------------------------------------------------*/
void OnSanshoAlarm(HWND hDlg, WORD id)
{
	char deffile[MAX_PATH], fname[MAX_PATH];
	
	GetDlgItemText(hDlg, id - 1, deffile, MAX_PATH);
	
	if(!BrowseSoundFile(hDlg, deffile, fname)) // soundselect.c
		return;
	
	SetDlgItemText(hDlg, id - 1, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}

/*------------------------------------------------
  checked "12 hour"
--------------------------------------------------*/
void On12Hour(HWND hDlg) {
	BOOL bAMPM = FALSE;
	WORD h, u, l;
	
  h = (WORD)SendDlgItemMessage(hDlg, IDC_SPINHOUR, UDM_GETPOS, 0, 0);
  if(h > 23) h = 23;
  if(h <  0) h = 0;
	
	// set limits to spin controls
  u = 23; l = 0;
  if(IsDlgButtonChecked(hDlg, IDC_12HOURALARM)) {
     bAMPM = TRUE;
	 if(h > 12) h -= 12;
	 if(h == 0) h = 12;
	 u = 12; l = 1;
  }

  EnableAMPM(hDlg, bAMPM);
  SendDlgItemMessage(hDlg, IDC_SPINHOUR, UDM_SETPOS, 0, h);
  SendDlgItemMessage(hDlg, IDC_SPINHOUR, UDM_SETRANGE, 0, MAKELONG(u, l));
  SendPSChanged(hDlg);
}
/*------------------------------------------------
  delete an alarm
--------------------------------------------------*/
void OnDelAlarm(HWND hDlg)
{
	PALARMSTRUCT pAS;
	
	if(curAlarm < 0) return;
	
	pAS = (PALARMSTRUCT)CBGetItemData(hDlg, IDC_COMBOALARM, curAlarm);
	if(pAS)
	{
		PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
		CBDeleteString(hDlg, IDC_COMBOALARM, curAlarm);
		free(pAS);
		if(curAlarm > 0) curAlarm--;
		CBSetCurSel(hDlg, IDC_COMBOALARM, curAlarm);
		pAS = (PALARMSTRUCT)CBGetItemData(hDlg, IDC_COMBOALARM, curAlarm);
		if(pAS) SetAlarmToDlg(hDlg, pAS);
		else
		{
			ALARMSTRUCT as;
			memset(&as, 0, sizeof(as));
			as.hour = 14;
			as.days = 0x7f;
			SetAlarmToDlg(hDlg, &as);
			EnableDlgItem(hDlg, IDC_DELALARM, FALSE);
			curAlarm = -1;
		}
	}
}

/*------------------------------------------------
   file name changed - enable/disable controls
--------------------------------------------------*/
void OnFileChange(HWND hDlg, WORD id)
{
	char fname[MAX_PATH];
	BOOL b = FALSE;
	
	GetDlgItemText(hDlg, id, fname, MAX_PATH);
	if(IsWindowEnabled(GetDlgItem(hDlg, id)))
		b = TRUE;
	EnableDlgItem(hDlg, id + 3, b);
	
	EnableDlgItem(hDlg,
		(id==IDC_FILEALARM)?IDC_REPEATALARM:IDC_REPEATJIHOU,
		b?IsMMFile(fname):FALSE);

	if(id == IDC_FILEALARM)
		EnableDlgItem(hDlg, IDC_CHIMEALARM, b?IsMMFile(fname):FALSE); 
}

/*------------------------------------------------
  test sound
--------------------------------------------------*/
void OnTest(HWND hDlg, WORD id)
{
	char fname[MAX_PATH];
	
	GetDlgItemText(hDlg, id - 3, fname, MAX_PATH);
	if(fname[0] == 0) return;

	if((HICON)SendDlgItemMessage(hDlg, id, BM_GETIMAGE, IMAGE_ICON, 0)
		== g_hIconPlay)
	{
		if(PlayFile(hDlg, fname, 0)) {
			SendDlgItemMessage(hDlg, id, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconStop);
			InvalidateRect(GetDlgItem(hDlg, id), NULL, FALSE);
			bPlaying = TRUE;
		}
	}
	else
	{
		StopFile();
		bPlaying = FALSE;
	}
}
  //======================================*
 // Format 12/24hr Time Text 02:01 Properly
//========================================*
void FormatTimeText(HWND hDlg, WORD idc) {
	BOOL bAMPM;
	char txt[5];
	int iTxt;

  bAMPM = IsDlgButtonChecked(hDlg, IDC_12HOURALARM);
  if((idc == IDC_12HOURALARM)||(idc == IDC_COMBOALARM)||(idc == 0)) idc = IDC_HOURALARM;

  if((IsDlgButtonChecked(hDlg, IDC_12HOURALARM)) && (idc == IDC_HOURALARM)) {
	  GetDlgItemText(hDlg, idc, txt, 5);
     iTxt = atoi(txt);

	 if(iTxt > 12) iTxt = 0;

     wsprintf(txt, "%d", iTxt);
     bFTT = FALSE; // Kill the Update Loop.
     SetDlgItemText(hDlg, idc, txt);

  }else if((!IsDlgButtonChecked(hDlg, IDC_12HOURALARM)) && (idc == IDC_HOURALARM)) {
     GetDlgItemText(hDlg, idc, txt, 5);
     iTxt = atoi(txt);

	 if(iTxt > 23) iTxt = 0;

     wsprintf(txt, "%02d", iTxt);
     bFTT = FALSE; // Kill the Update Loop.
     SetDlgItemText(hDlg, idc, txt);
  }else{
     GetDlgItemText(hDlg, idc, txt, 5);
     iTxt = atoi(txt);

	 if(iTxt > 59) iTxt = 0;

     wsprintf(txt, "%02d", iTxt);
     bFTT = FALSE; // Kill the Update Loop.
     SetDlgItemText(hDlg, idc, txt);
	 keybd_event(VK_END, 0, 0, 0);
  }
  EnableAMPM(hDlg, bAMPM);
  ToggleAMPMDisplay(hDlg);
}
  //======================================*
 // Enable/Disable 12hr Controls as Needed
//========================================*
void EnableAMPM(HWND hDlg, BOOL bAP) {
  EnableDlgItem(hDlg, IDC_AMPM_CHECK, bAP);
//  EnableDlgItem(hDlg, IDC_AMPM_DISPLAY, bAP);
}
   //=============================*
  // -- Toggle Display of AM or PM
 // ---- Based on CheckBox State
//================================*
void ToggleAMPMDisplay(HWND hDlg) {
	char mBuf[5], hBuf[5];
	int imTxt, ihTxt;

  GetDlgItemText(hDlg, IDC_MINUTEALARM, mBuf, 5);
  GetDlgItemText(hDlg, IDC_HOURALARM, hBuf, 5);
  imTxt = atoi(mBuf);
  ihTxt = atoi(hBuf);

  if(IsDlgButtonChecked(hDlg, IDC_AMPM_CHECK)) {     //-------------- Assumes 12pm is Noon
     if((ihTxt == 12) && (imTxt == 0))     SetDlgItemText(hDlg, IDC_AMPM_DISPLAY, "Noon");
     else SetDlgItemText(hDlg, IDC_AMPM_DISPLAY, "PM");
   }else{									      //------------- Assumes 12am is Midnight
	 if((ihTxt == 12) && (imTxt == 0)) SetDlgItemText(hDlg, IDC_AMPM_DISPLAY, "Midnight");
	 else SetDlgItemText(hDlg, IDC_AMPM_DISPLAY, "AM");
  }
  if(!IsDlgButtonChecked(hDlg, IDC_12HOURALARM))
	  SetDlgItemText(hDlg, IDC_AMPM_DISPLAY, "24hr");
}