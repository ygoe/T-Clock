/*-------------------------------------------
  TCDLL.H - KAZUBON 1997-2001
---------------------------------------------*/
// Modified by Stoic Joker: Thursday, 03/25/2010 @ 6:00:55pm
#define _CRT_SECURE_NO_DEPRECATE 1 // SHUT-UP About the New String Functions Already!!!

#include <windows.h>  // Required by the fact it runs on Windows.
#include <commctrl.h> // Required by most everything in TClock.c
#include <time.h>     // Required by time functions in Format.c
#include <math.h>     // Required by use of floor() in Format.c

#define AC_SRC_ALPHA	0x01
#define TZNAME_MAX		  256//10

#ifndef GWL_WNDPROC // Required for the x64 Edition
#define GWL_WNDPROC GWLP_WNDPROC
#endif

#define IDTIMER_DEKSTOPICONSTYLE 3

#define CLOCKM_REFRESHCLOCK   (WM_USER+1)
#define CLOCKM_REFRESHTASKBAR (WM_USER+2)
#define CLOCKM_BLINK          (WM_USER+3)
#define CLOCKM_COPY           (WM_USER+4)
#define CLOCKM_REFRESHDESKTOP (WM_USER+5)
#define CLOCKM_REFRESHCLEARTASKBAR	(WM_USER+6)
#define TBCDRF_NOEDGES              0x00010000  // Don't draw button edges
#define TBCDRF_HILITEHOTTRACK       0x00020000  // Use color of the button bk when hottracked
#define TBCDRF_NOOFFSET             0x00040000  // Don't offset button if pressed
#define TBCDRF_NOMARK               0x00080000  // Don't draw default highlight of image/text for TBSTATE_MARKED
#define TBCDRF_NOETCHEDEFFECT       0x00100000  // Don't draw etched effect for disabled items

#define LPTBBUTTONINFO LPTBBUTTONINFOA
#define TBBUTTONINFO TBBUTTONINFOA

#define DEFAULT_QUALITY				0
#define DRAFT_QUALITY				1
#define PROOF_QUALITY				2
#define NONANTIALIASED_QUALITY		3
#define ANTIALIASED_QUALITY			4
#define CLEARTYPE_QUALITY			5
#define CLEARTYPE_NATURAL_QUALITY	6

// tclock.c
extern BOOL bNoClock;
void DrawClock(HWND hwnd, HDC hdc);
void GetDisplayTime(SYSTEMTIME* pt, int* beat100);
void FillClock(HWND hwnd, HDC hdc, RECT *prc, int nblink);

//#pragma once
//extern char szTZone[]; //---+++--> TimeZone String Buffer, Also Used (as External) in Format.c

// utl.c
extern BOOL g_bIniSetting;
extern char g_inifile[];

char* MyString(UINT id);
void del_title(char *path);
int ext_cmp(char *fname, char *ext);
void add_title(char *path, char *title);
void parse(char *dst, char *src, int n);
int _strncmp(const char* d, const char* s, size_t n);
BOOL SetMyRegLong(char* subkey, char* entry, DWORD val);
LONG GetMyRegLong(char* section, char* entry, LONG defval);
LONG GetMyRegLongEx(char* section, char* entry, LONG defval);
COLORREF GetMyRegColor(char* section, char* entry, COLORREF defval);
LONG GetRegLong(HKEY rootkey, char*subkey, char* entry, LONG defval);
int GetMyRegStr(char* section, char* entry, char* val, int cbData, char* defval);
int GetMyRegStrEx(char* section, char* entry, char* val, int cbData, char* defval);
HFONT CreateMyFont(char* fontname, int fontsize, LONG weight, LONG italic, int angle);
int GetRegStr(HKEY rootkey, char*subkey, char* entry, char* val, int cbData, char* defval);
void VerticalTileBlt(HDC hdcDest, int xDest, int yDest, int cxDest, int cyDest, HDC hdcSrc, 
					 int xSrc, int ySrc, int cxSrc, int cySrc, BOOL ReverseBlt, BOOL useTrans);
void HorizontalTileBlt(HDC hdcDest, int xDest, int yDest, int cxDest, int cyDest, HDC hdcSrc, 
					   int xSrc, int ySrc, int cxSrc, int cySrc, DWORD rasterOp);
void FillTileBlt(HDC hdcDest, int xDest, int yDest, int cxDest, int cyDest, HDC hdcSrc, 
				 int xSrc, int ySrc, int cxSrc, int cySrc, DWORD rasterOp);
void TileBlt(HDC hdcDest, int xDest, int yDest, int cxDest, int cyDest, HDC hdcSrc,
			 int xSrc, int ySrc, int cxSrc, int cySrc, BOOL useTrans);
void GetFileAndOption(const char* command, char* fname, char* opt);
BOOL ConvertTip(char *destination, WCHAR *tip, UINT code);
BOOL SetMyRegStr(char* subkey, char* entry, char* val);
BOOL ExecFile(HWND hwnd, char* command);
void Pause(HWND hWnd, LPCTSTR pszArgs);
BOOL IsXPStyle();


// FORMAT.C
void InitFormat(SYSTEMTIME* lt);
void MakeFormat(char* s, SYSTEMTIME* pt, int beat100, char* fmt);
#define FORMAT_SECOND    0x0001
#define FORMAT_SYSINFO   0x0002
#define FORMAT_BEAT1     0x0004
#define FORMAT_BEAT2     0x0008
#define FORMAT_BATTERY   0x0010
#define FORMAT_MEMORY    0x0020
#define FORMAT_MOTHERBRD 0x0040
#define FORMAT_PERMON    0x0080
#define FORMAT_NET       0x0100
DWORD FindFormat(char* fmt);

// newapi.c
void EndNewAPI(HWND hwnd);
void SetLayeredTaskbar(HWND hwndClock);
void TransBlt(HDC dhdc, int dx, int dy, int dw, int dh, HDC shdc, int sx, int sy, int sw, int sh);
void TC2DrawBlt(HDC dhdc, int dx, int dy, int dw, int dh, HDC shdc, int sx, int sy, int sw, int sh, BOOL useTrans);
void DrawXPClockBackground(HWND hwnd, HDC hdc, RECT *prc);
void InitDrawThemeParentBackground(void);
void RefreshUs(void);

#ifndef CCM_FIRST
#define CCM_FIRST               0x2000
#endif

#ifndef CCM_SETCOLORSCHEME
#define CCM_SETCOLORSCHEME      (CCM_FIRST + 2)
#endif

#ifndef RB_SETBKCOLOR
#define RB_SETBKCOLOR   (WM_USER +  19)
#endif