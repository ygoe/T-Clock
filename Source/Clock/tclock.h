   //===============================================================================
  //--+++--> tclock.h - KAZUBON  1997-1999 =========================================
 //=================== Last Modified by Stoic Joker: Sunday, 03/13/2011 @ 11:54:05am
//-------------------------{ Stoic Joker 2006-2010 }-------------------------+++-->
#define _CRT_SECURE_NO_DEPRECATE 1 // SHUT-UP About the New String Functions Already!!!

#pragma once

#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#if defined _M_IX86

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#define ABT_TCLOCK "T-Clock 2010 - build 98"
#define ABT_ME "T-Clock 2010 is my rewrite of their code which allows it to run on Windows 7, Vista, XP, && 2000. While I have removed some of T-Clock's previous functionality. I feel this makes it a more \"Administrator Friendly\" application as it no longer requires elevated privileges to run."
#define AUTO_START "Start T-Clock 2010 When Windows Starts"
#define CONF_START "Stoic Joker's T-Clock 2010"

#elif defined _M_IA64

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")

#elif defined _M_X64

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#define ABT_TCLOCK "T-Clock 2010 x64 - build 98"
#define ABT_ME "T-Clock 2010 x64 is my rewrite of their code which allows it to run on Windows XP, Vista, && 7 x64 Editions. While I have removed some of T-Clock's previous functionality. I feel this makes it a more \"Administrator Friendly\" application as it no longer requires elevated privileges to run."
#define AUTO_START "Start T-Clock 2010 x64 When Windows Starts"
#define CONF_START "Stoic Joker's T-Clock 2010 x64"

#else

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#endif

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <shlobj.h>
#include <stdlib.h>
#include <Shlwapi.h>
#include <psapi.h>
#include "resource.h"

#ifndef GWL_WNDPROC
#define GWL_WNDPROC GWLP_WNDPROC
#endif

// replacement of standard library's functions
int _strnicmp(const char* d, const char* s, size_t n);
int _stricmp(const char* d, const char* s);
#define malloc(s) GlobalAllocPtr(GHND,(s))
#define free(p) GlobalFreePtr(p);
int atoi(const char *p);

// IDs for timer
#define IDTIMER_START				2
#define IDTIMER_MAIN				3
#define IDTIMER_MOUSE				4
#define IDTIMER_DEKSTOPICON			5
#define IDTIMER_DESKTOPICONSTYLE	6


// messages to send the clock
#define CLOCKM_REFRESHCLOCK   (WM_USER+1)
#define CLOCKM_REFRESHTASKBAR (WM_USER+2)
#define CLOCKM_BLINK          (WM_USER+3)
#define CLOCKM_COPY           (WM_USER+4)
#define CLOCKM_REFRESHDESKTOP (WM_USER+5)
#define CLOCKM_REFRESHCLEARTASKBAR	(WM_USER+6)

// for mouse.c and pagemouse.c
#define MOUSEFUNC_NONE		   -1
#define MOUSEFUNC_TIMER			5
#define MOUSEFUNC_CLIPBOARD		6
#define MOUSEFUNC_SCREENSAVER	7
#define MOUSEFUNC_SHOWCALENDER	8
#define MOUSEFUNC_SHOWPROPERTY	9

// Global Buffer Size Labels
#define TNY_BUFF	32
#define MIN_BUFF	64
#define GEN_BUFF	128
#define LRG_BUFF	256
#define MAX_BUFF	1024

// System Global HotKey Identifiers
#define HOT_WATCH	200
#define HOT_TIMER	210
#define HOT_STOPW	220
#define HOT_PROPR	230
#define HOT_CALEN	240
#define HOT_TSYNC	250

// Jack Russel Message Windo Goes Boing!
#define JRMSG_BOING 15000

//--+++--> main.c - Application Global Values:
extern char      g_mydir[];         // Path to Clock.exe
extern HWND      g_hwndClock;       // Main Clock Window Handle
extern HWND      g_hDlgTimer;       // Timer Dialog Window Handle
extern HWND      g_hDlgCalender;    // Calender Dialog Window Handle
extern HWND		 g_hDlgStopWatch;	// Stopwatch Dialog Window Handle
extern HWND		 g_hDlgTimerWatch;  // Timwe Watch Dialog Window Handle
extern HWND      g_hwndSheet;       // (TCM Property Sheet Window Handle
extern HWND      g_hWnd;			// Main Window Anchor for HotKeys Only!
extern HICON     g_hIconTClock, g_hIconPlay, // Frequently Used Icon Handles
	  g_hIconStop, g_hIconDel, g_hIconLogo; // Frequently Used Icon Handles
extern BOOL bMonOffOnLock; //-+> Locking Workstation Turns Off Monitor(s).
extern BOOL bV7up; //--------------+++--> OS Version is Vista/7 or Better.
extern BOOL b2000; //--------------+++--> OS is Windows 2000.

void RegisterSession(HWND hwnd);
void UnregisterSession(HWND hwnd);
void RefreshUs(void);

// propsheet.c
extern BOOL g_bApplyClock;
void MyPropertySheet(void);
extern BOOL g_bApplyTaskbar;
void SetMyDialgPos(HWND hwnd);
BOOL SelectMyFile(HWND hDlg, const char *filter, DWORD nFilterIndex, const char *deffile, char *retfile);

// alarm.c
void StopFile(void);
void EndAlarm(void);
void InitAlarm(void);
void OnMCINotify(HWND hwnd);
BOOL ExecFile(HWND hwnd, char* command);
void OnTimerAlarm(HWND hwnd, SYSTEMTIME* st);
BOOL PlayFile(HWND hwnd, char *fname, DWORD dwLoops);
void GetFileAndOption(const char* command, char* fname, char* opt);
extern bKillPCBeep; // Declared in alarm.c

// alarmday.c
int SetAlarmDay(HWND hDlg, int n);

// soundselect.c
BOOL IsMMFile(const char* fname);
BOOL BrowseSoundFile(HWND hDlg, const char *deffile, char *fname);

// pageformat.c
void InitFormat(void);
void CreateFormat(char* s, int* checks);

// menu.c
void OnTClockCommand(HWND hwnd, WORD wID, WORD wCode);
void OnContextMenu(HWND hwnd, HWND hwndClicked, int xPos, int yPos);

// mouse.c
void OnTimerMouse(HWND hwnd);
void OnDropFiles(HWND hwnd, HDROP hdrop);
void OnMouseMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// deskcal.c
void DialogCalender(HWND hwnd);

// timer.c
#define MAX_TIMER  7
void WatchTimer(void);
void CancelAllTimersOnStartUp(void);

void EndTimer(void);
void DialogTimer(HWND hwnd);
void StopTimer(HWND hwnd, int n);
void OnTimerTimer(HWND hwnd, SYSTEMTIME* st);
int GetTimerInfo(char *dst, int num, BOOL bNameOnly);

// StopWatch.c
void DialogStopWatch(HWND);

// utl.c
int atox(const char *p);
void del_title(char *path);
void ForceForegroundWindow(HWND hWnd);
DWORDLONG M32x32to64(DWORD a, DWORD b);
void parse(char *dst, char *src, int n);
void add_title(char *path, char* titile);
void get_title(char* dst, const char *path);
int ext_cmp(const char *fname, const char *ext);
void parsechar(char *dst, char *src, char ch, int n);
COLORREF GetMyRegColor(char* section, char* entry, COLORREF defval);
int MyMessageBox(HWND hwnd, char* msg, char* title, UINT uType, UINT uBeep);
int GetMyRegStr(char* section, char* entry, char* val, int cbData, char* defval);
int GetMyRegStrEx(char* section, char* entry, char* val, int cbData, char* defval);
int GetRegStr(HKEY rootkey, char*subkey, char* entry, char* val, int cbData, char* defval);
LONG GetRegLong(HKEY rootkey, char*subkey, char* entry, LONG defval);
BOOL SetRegStr(HKEY rootkey, char* subkey, char* entry, char* val);
LONG GetMyRegLongEx(char* section, char* entry, LONG defval);
LONG GetMyRegLong(char* section, char* entry, LONG defval);
BOOL SetMyRegLong(char* subkey, char* entry, DWORD val);
BOOL SetMyRegStr(char* subkey, char* entry, char* val);
BOOL DelMyReg(char* subkey, char* entry);
void str0cat(char* dst, const char* src);
BOOL DelMyRegKey(char* subkey);
char* MyString(UINT id);

// ExitWindows.c
BOOL ShutDown(void);
BOOL ReBoot(void);
BOOL LogOff(void);

// TCDLL.DLL‚ÌAPI
void WINAPI HookStart(HWND hwnd);
void WINAPI HookEnd(void);

// PageHotKey.c
void GetHotKeyInfo(HWND hWnd);

// SNTP.c
void SyncTimeNow(void);
void NetTimeConfigDialog(void);

// BounceWind.c
void OnMsgWindOpt(HWND hDlg);
void ReleaseTheHound(HWND hWnd, BOOL);

// Macros
#define EnableDlgItem(hDlg,id,b) EnableWindow(GetDlgItem((hDlg),(id)),(b))
#define ShowDlgItem(hDlg,id,b) ShowWindow(GetDlgItem((hDlg),(id)),(b)?SW_SHOW:SW_HIDE)

#define CBFindStringExact(hDlg,id,s) SendDlgItemMessage((hDlg),(id),CB_FINDSTRINGEXACT,0,(LPARAM)(s))
#define CBInsertString(hDlg,id,i,s) SendDlgItemMessage((hDlg),(id),CB_INSERTSTRING,(i),(LPARAM)(s))
#define CBSetItemData(hDlg,id,i,lParam) SendDlgItemMessage((hDlg),(id),CB_SETITEMDATA,(i),(lParam))
#define CBGetLBText(hDlg,id,i,s) SendDlgItemMessage((hDlg),(id),CB_GETLBTEXT,(i),(LPARAM)(s))
#define CBAddString(hDlg,id,lParam) SendDlgItemMessage((hDlg),(id),CB_ADDSTRING,0,(lParam))
#define CBFindString(hDlg,id,s) SendDlgItemMessage((hDlg),(id),CB_FINDSTRING,0,(LPARAM)(s))
#define CBDeleteString(hDlg,id, i) SendDlgItemMessage((hDlg),(id),CB_DELETESTRING,(i),0)
#define CBGetItemData(hDlg,id,i) SendDlgItemMessage((hDlg),(id),CB_GETITEMDATA,(i),0)
#define CBResetContent(hDlg,id) SendDlgItemMessage((hDlg),(id),CB_RESETCONTENT,0,0)
#define CBSetCurSel(hDlg,id,i) SendDlgItemMessage((hDlg),(id),CB_SETCURSEL,(i),0)
#define CBGetCurSel(hDlg,id) SendDlgItemMessage((hDlg),(id),CB_GETCURSEL,0,0)
#define CBGetCount(hDlg,id) SendDlgItemMessage((hDlg),(id),CB_GETCOUNT,0,0)

typedef struct _tagAlarmStruct
{
	char name[TNY_BUFF];
	BOOL bAlarm;
	int hour;
	int minute;
	char fname[MAX_BUFF];
	char jrMessage[MAX_BUFF];
	char jrSettings[TNY_BUFF];
	BOOL jrMsgUsed;
	BOOL bHour12;
	BOOL bChimeHr;
	BOOL bRepeat;
	int iTimes;
	BOOL bBlink;
	BOOL bPM;
	int days;
} ALARMSTRUCT;
typedef ALARMSTRUCT* PALARMSTRUCT;

//----------------//--------------+++--> HotKey Configuration,
typedef struct { //--+++--> Manipulation, & Storage Structure.
	UINT vk;
	UINT fsMod;
	BOOL bValid;
	char szText[TNY_BUFF];
} TCHOTKEY;