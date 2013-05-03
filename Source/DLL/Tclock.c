   /*--------------------------------------------------------
  // tclock.c : customize the tray clock -> KAZUBON 1997-2001
 //--------------------------------------------------------*/
// Modified by Stoic Joker: Tuesday, March 2 2010 - 10:42:42
#include "tcdll.h"

void EndClock(void);
void OnTimer(HWND hwnd);
void ReadData(HWND hwnd);
void InitClock(HWND hwnd);
void CreateTip(HWND hwnd);
void CreateClockDC(HWND hwnd);
LRESULT OnCalcRect(HWND hwnd);
void InitDaylightTimeTransition(void);
void OnCopy(HWND hwnd, LPARAM lParam);
BOOL CheckDaylightTimeTransition(SYSTEMTIME *lt);
void OnTooltipNeedText(UINT code, LPARAM lParam);
void DrawClockSub(HWND hwnd, HDC hdc, SYSTEMTIME* pt, int beat100);
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
 //================================================================================================
//----------------------------------------+++--> Definition of Data Segment Shared Among Processes:
#pragma data_seg(".MYDATA") //--------------------------------------------------------------+++-->
//char szShareBuf[81] = { 0 }; WTF DOES THIS DO??!!?? 2010
HWND hwndTClockMain = NULL;
HWND hwndClock = NULL;
HHOOK hhook = 0;
#pragma data_seg()

/*------------------------------------------------
  globals
--------------------------------------------------*/
HANDLE hmod = 0;
WNDPROC oldWndProc = NULL;
BOOL bTimer = FALSE;
BOOL bTimerTesting = FALSE;
HDC hdcClock = NULL;
HBITMAP hbmpClock = NULL;
HBITMAP hbmpClockSkin = NULL;
HFONT hFon = NULL;
HWND hwndTip = NULL;
COLORREF colback, colback2, colfore;
char format[1024];
BOOL bHour12, bHourZero;
SYSTEMTIME LastTime;
int beatLast = -1;
int bDispSecond = FALSE;
int nDispBeat = 0;
BOOL bNoClock = FALSE;
int nBlink = 0;
int dwidth = 0, dheight = 0, dvpos = 0, dlineheight = 0, dhpos = 0;
int iClockWidth = -1;
BOOL bPlaying = FALSE;

extern HWND hwndStartMenu;
extern int codepage;

int tEdgeTop;
int tEdgeLeft;
int tEdgeBottom;
int tEdgeRight;

BOOL bRefreshClearTaskbar = FALSE;
//static BOOL bClockUseTrans = FALSE;
 //================================================================================================
//---------------------------------------------------------------------+++--> Initialize the Clock:
void InitClock(HWND hWnd) { //--------------------------------------------------------------+++-->
	BOOL b;

  hwndClock = hWnd;
  PostMessage(hwndTClockMain, WM_USER, 0, (LPARAM)hWnd);
	
  ReadData(hwndClock); //-+-> Get Configuration Information From Registry
  InitDaylightTimeTransition(); // Get User's Local Time-Zone Information

/*------------------{ This Code Will Crash After MSVS2005 SP1 }-------------------+++--> S-h-i-t...!
  oldWndProc = (WNDPROC)(LONG_PTR)GetWindowLongPtr(hwndClock, GWL_WNDPROC); // The x64 Bugg Was Here
  // SetWindowLongPtr(hwndClock, GWL_WNDPROC, (LONG)(LONG_PTR)WndProc); <--+++----<<<<< FAIL Code!!!
  SetWindowLongPtr(hwndClock, GWL_WNDPROC, (LONG_PTR)(LRESULT)WndProc); //----+++--> This Fixed IT!!
  SetClassLong(hwndClock, GCL_STYLE, GetClassLong(hwndClock, GCL_STYLE) & ~CS_DBLCLKS);
 <--+++-----------------------------------------------------------------------------*/

  oldWndProc = (WNDPROC)(LONG_PTR)GetWindowLongPtr(hwndClock, GWL_WNDPROC);
//==================================================================================
#if defined _M_IX86 //---------------+++--> IF Compiling This as a 32-bit Clock Use:
  SetWindowLongPtr(hwndClock, GWL_WNDPROC, (LONG)(LRESULT)WndProc);

//==================================================================================
#else //-------------------+++--> ELSE Assume: _M_X64 - IT's a 64-bit Clock and Use:
  SetWindowLongPtr(hwndClock, GWL_WNDPROC, (LONG_PTR)(LRESULT)WndProc);

#endif
//==================================================================================
 SetClassLong(hwndClock, GCL_STYLE, GetClassLong(hwndClock, GCL_STYLE) & ~CS_DBLCLKS);

  CreateTip(hwndClock); // Create Mouse-Over ToolTip Window & Contents
	
  b = GetMyRegLong(NULL, "DropFiles", FALSE);
  DragAcceptFiles(hWnd, b); // Enable/Disable DropFiles on Clock Based on Reg Info.
	
  SetLayeredTaskbar(hwndClock); // Strangely Not Required for XP Themes... WTF is it For?? 2010

  PostMessage(GetParent(GetParent(hwndClock)), WM_SIZE, SIZE_RESTORED, 0);
  InvalidateRect(GetParent(GetParent(hwndClock)), NULL, TRUE);
}
 //================================================================================================
//-------------------------------------+++--> Delete ALL T-Clock Created (Font & BitMap) Resources:
void DeleteClockRes(void) { //--------------------------------------------------------------+++-->
  if(hFon) DeleteObject(hFon); hFon = NULL;
  if(hdcClock) DeleteDC(hdcClock); hdcClock = NULL;
  if(hbmpClock) DeleteObject(hbmpClock); hbmpClock = NULL;
  if(hbmpClockSkin) DeleteObject(hbmpClockSkin); hbmpClockSkin = NULL;
}
 //================================================================================================
//----------------------------------+++--> End Clock Procedure (WndProc) - (Before?) Removing Hook:
void EndClock(void) { //--------------------------------------------------------------------+++-->
  DragAcceptFiles(hwndClock, FALSE);
  if(hwndTip) DestroyWindow(hwndTip); hwndTip = NULL;
	
  DeleteClockRes();
  EndNewAPI(hwndClock);
  if(hwndClock && IsWindow(hwndClock)) {
	 if(bTimer) KillTimer(hwndClock, 1); bTimer = FALSE;
//     SetWindowLongPtr(hwndClock, GWL_WNDPROC, (LONG)(LONG_PTR)oldWndProc);

//==================================================================================
#if defined _M_IX86 //---------------+++--> IF Compiling This as a 32-bit Clock Use:
  SetWindowLongPtr(hwndClock, GWL_WNDPROC, (LONG)(LRESULT)oldWndProc);

//==================================================================================
#else //-------------------+++--> ELSE Assume: _M_X64 - IT's a 64-bit Clock and Use:
  SetWindowLongPtr(hwndClock, GWL_WNDPROC, (LONG_PTR)(LRESULT)oldWndProc);

#endif
//==================================================================================

	 oldWndProc = NULL;
  }

  if(IsWindow(hwndTClockMain)) PostMessage(hwndTClockMain, WM_USER+2, 0, 0);
//  bClockUseTrans = FALSE;
}
/*------------------------------------------------
  subclass procedure of the clock
--------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message) {
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_MOUSEMOVE:
		{
			MSG msg;
			msg.hwnd = hwnd;
			msg.message = message;
			msg.wParam = wParam;
			msg.lParam = lParam;
			msg.time = GetMessageTime();
			msg.pt.x = LOWORD(GetMessagePos());
			msg.pt.y = HIWORD(GetMessagePos());
			if(hwndTip)
			{
				SendMessage(hwndTip, TTM_RELAYEVENT, 0, (LPARAM)&msg);
			}
		}
	}
	
	switch(message)
	{
		case WM_DESTROY:
			DeleteClockRes();
			break;

		case (WM_USER+100):
			if(bNoClock) break;
			return OnCalcRect(hwnd);

		case WM_SYSCOLORCHANGE:
			CreateClockDC(hwnd); 
		case WM_TIMECHANGE:

		case (WM_USER+101):
		{
			HDC hdc;
			if(bNoClock) break;
			hdc = GetDC(hwnd);
			DrawClock(hwnd, hdc);
			ReleaseDC(hwnd, hdc);
			return 0;
		}
		case WM_SIZE:
			CreateClockDC(hwnd);
			break;

		case WM_ERASEBKGND:
			return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			if(bNoClock) break;
			hdc = BeginPaint(hwnd, &ps);
			DrawClock(hwnd, hdc);
			EndPaint(hwnd, &ps);
			return 0;
		}
		case WM_TIMER:
			if(wParam == 1) 
				OnTimer(hwnd);
			else
			{
				if(bNoClock) break;
			}
			return 0;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			if(nBlink)
			{
				nBlink = 0; InvalidateRect(hwnd, NULL, TRUE);
			}

			PostMessage(hwndTClockMain, message, wParam, lParam);
			return 0;

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			PostMessage(hwndTClockMain, message, wParam, lParam);
			if(message == WM_RBUTTONUP) break;
			return 0;
		case WM_MOUSEMOVE:
			return 0;
		case WM_CONTEXTMENU: 
			PostMessage(hwndTClockMain, message, wParam, lParam);
			return 0;
		case WM_NCHITTEST: // oldWndProc
			return DefWindowProc(hwnd, message, wParam, lParam);
		case WM_MOUSEACTIVATE:
			return MA_ACTIVATE;

		case WM_DROPFILES:
			PostMessage(hwndTClockMain, WM_DROPFILES, wParam, lParam);
			return 0;
		case WM_NOTIFY: 
		{
			UINT code;
			code = ((LPNMHDR)lParam)->code;
			if(code == TTN_NEEDTEXT || code == TTN_NEEDTEXTW)
			{
				OnTooltipNeedText(code, lParam);
				return 0;
			}
			return 0;
		}
		case WM_COMMAND:
			if(LOWORD(wParam) == 102) EndClock();
			return 0;
		case CLOCKM_REFRESHCLOCK: // refresh the clock
		{
			BOOL b;
			ReadData(hwnd);
			CreateClockDC(hwnd);
			b = GetMyRegLong(NULL, "DropFiles", FALSE);
			DragAcceptFiles(hwnd, b);
			InvalidateRect(hwnd, NULL, FALSE);
			InvalidateRect(GetParent(hwndClock), NULL, TRUE);
			return 0;
		}
		case CLOCKM_REFRESHTASKBAR: // refresh other elements than clock
			CreateClockDC(hwnd);
			SetLayeredTaskbar(hwndClock);
			PostMessage(GetParent(GetParent(hwnd)), WM_SIZE, SIZE_RESTORED, 0);
			InvalidateRect(GetParent(GetParent(hwndClock)), NULL, TRUE);
			return 0;
		case CLOCKM_BLINK: // blink the clock
			if(wParam) { if(nBlink == 0) nBlink = 4; }
			else nBlink = 2;
			return 0;
		case CLOCKM_COPY: // copy format to clipboard
			OnCopy(hwnd, lParam);
			return 0;
		case CLOCKM_REFRESHCLEARTASKBAR:
		{
			bRefreshClearTaskbar = TRUE;
			SetLayeredTaskbar(hwndClock);
			return 0;
		}
		case WM_WINDOWPOSCHANGING:
		{
			LPWINDOWPOS pwp;
			if(bNoClock) break;
			pwp = (LPWINDOWPOS)lParam;
			if(IsWindowVisible(hwnd) && !(pwp->flags & SWP_NOSIZE))
			{
				int h;
				h = (int)HIWORD(OnCalcRect(hwnd));
				if(pwp->cy > h) pwp->cy = h;
				
			}
			break;
		}
	}
  return CallWindowProc(oldWndProc, hwnd, message, wParam, lParam);
}
 //================================================================================================
//---------------------------------+++--> Retreive T-Clock Configuration Information From Registry:
void ReadData(HWND hwnd) { //---------------------------------------------------------------+++-->
	char FontRotateDirection[1024] = {0};
	char fontname[80] = {0};
	LONG weight, italic;
	int angle, fontsize;
	DWORD dwInfoFormat;
	SYSTEMTIME lt;

  colfore = GetMyRegColor("Clock", "ForeColor", 0x00ffffff);//GetSysColor(COLOR_BTNTEXT));
						// <--+++--<<<< THIS IS WHERE THE BLINK COLOR ISSUE IS/STARTS/NEEDS TO GO!!!
  colback = GetMyRegColor("Clock", "BackColor", GetSysColor(COLOR_3DFACE));
  if(GetMyRegLong("Clock", "UseBackColor2", TRUE)) 
	  colback2 = GetMyRegColor("Clock", "BackColor2", colback);
  else colback2 = colback;
	
  GetMyRegStr("Clock", "Font", fontname, 80, "Arial");
	
  fontsize = GetMyRegLong("Clock", "FontSize", 10);
  italic = GetMyRegLong("Clock", "Italic", 0);
  weight = GetMyRegLong("Clock", "Bold", 1);
  if(weight) weight = FW_BOLD;
  else weight = 0;
	
  GetMyRegStr("Clock", "FontRotateDirection", FontRotateDirection, 1024, "None");
  if(_strnicmp(FontRotateDirection, "RIGHT", 5) == 0) angle = 2700;
  else if(_strnicmp(FontRotateDirection, "LEFT", 4) == 0) angle = 900;
  else angle = 0;

  if(hFon) DeleteObject(hFon);
  hFon = CreateMyFont(fontname, fontsize, weight, italic, angle);

  dlineheight = (int)(short)GetMyRegLong("Clock", "LineHeight", 0);
  dheight = (int)(short)GetMyRegLong("Clock", "ClockHeight", 0);
  dwidth = (int)(short)GetMyRegLong("Clock", "ClockWidth", 0);
  dhpos = (int)(short)GetMyRegLong("Clock", "HorizPos", 0);
  dvpos = (int)(short)GetMyRegLong("Clock", "VertPos", 0);
	
  bNoClock = GetMyRegLong("Clock", "NoClockCustomize", FALSE);
	
  if(!GetMyRegStr("Format", "Format", format, 1024, "") || !format[0]) {
	 bNoClock = TRUE;
  }
	
  dwInfoFormat = FindFormat(format);
  bDispSecond = (dwInfoFormat&FORMAT_SECOND)? TRUE:FALSE;
  nDispBeat = dwInfoFormat & (FORMAT_BEAT1 | FORMAT_BEAT2);
  if(!bTimer) SetTimer(hwndClock, 1, 1000, NULL);
  bTimer = TRUE;
	
  bHour12 = GetMyRegLong("Format", "Hour12", FALSE);
  bHourZero = GetMyRegLong("Format", "HourZero", 0);
	
  GetLocalTime(&lt);
  LastTime.wDay = lt.wDay;
	
  InitFormat(&lt);      // format.c
	
  iClockWidth = -1;

//  bClockUseTrans = GetMyRegLong("Clock", "ClockUseTrans", FALSE);
}
 //================================================================================================
//---------------------------------------------------------+++--> Create Mouse-Over ToolTip Window:
void CreateTip(HWND hwnd) { //--------------------------------------------------------------+++-->
	TOOLINFO ti;
	
  hwndTip = CreateWindow(TOOLTIPS_CLASS, (LPSTR)NULL, TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hmod, NULL);
  if(!hwndTip) return;

	ti.cbSize = sizeof(TOOLINFO); 
	ti.uFlags = 0;
	ti.hwnd = hwnd;
	ti.hinst = NULL;
	ti.uId = 1;
	ti.lpszText = LPSTR_TEXTCALLBACK;
	ti.rect.left = 0;
	ti.rect.top = 0;
	ti.rect.right = 480; 
	ti.rect.bottom = 480;
	
  SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO)&ti);
  SendMessage(hwndTip, TTM_ACTIVATE, TRUE, 0);
  SendMessage(hwndTip, TTM_SETMAXTIPWIDTH, 0, 300);
}
 //=========================================================
void CreateClockDC(HWND hwnd) {
	COLORREF col;
	RECT rc;
	HDC hdc;

  if(hdcClock) DeleteDC(hdcClock); hdcClock = NULL;
  if(hbmpClock) DeleteObject(hbmpClock); hbmpClock = NULL;
  if(hbmpClockSkin) DeleteObject(hbmpClockSkin); hbmpClockSkin = NULL;
	
  if(bNoClock) return;

  GetClientRect(hwnd, &rc);
	
  hdc = GetDC(NULL);
  hdcClock = CreateCompatibleDC(hdc);
  if(!hdcClock) {
	 ReleaseDC(NULL, hdc);
	 return;
  }
  
  hbmpClock = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);

  if(!hbmpClock) {
	 DeleteDC(hdcClock); hdcClock = NULL;
	 ReleaseDC(NULL, hdc);
	 return;
  }
  
  SelectObject(hdcClock, hbmpClock);
  SelectObject(hdcClock, hFon);
  
  SetBkMode(hdcClock, TRANSPARENT);
  SetTextAlign(hdcClock, TA_CENTER|TA_TOP);
  col = colfore;
  
  if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
  SetTextColor(hdcClock, col);

  FillClock(hwnd, hdcClock, &rc, 0);
  ReleaseDC(NULL, hdc);
}

/*------------------------------------------------
   get date/time and beat to display
--------------------------------------------------*/
void GetDisplayTime(SYSTEMTIME* pt, int* beat100)
{
	FILETIME ft, lft;
	SYSTEMTIME lt;
	
	GetSystemTimeAsFileTime(&ft);
	
	if(beat100)
	{
		DWORDLONG dl;
		SYSTEMTIME st;
		int sec;
		
		dl = *(DWORDLONG*)&ft + 36000000000;
		FileTimeToSystemTime((FILETIME*)&dl, &st);
		
		sec = st.wHour * 3600 + st.wMinute * 60 + st.wSecond;
		*beat100 = (sec * 1000) / 864;
	}
	
	FileTimeToLocalFileTime(&ft, &lft);
	FileTimeToSystemTime(&lft, &lt);
	memcpy(pt, &lt, sizeof(lt));
}

/*--------------------------------------------------
------------------------------------------- WM_TIMER
--------------------------------------------------*/
void OnTimer(HWND hwnd)
{
	SYSTEMTIME t;
	int beat100;
	HDC hdc;
	BOOL bRedraw;
	
	GetDisplayTime(&t, nDispBeat?(&beat100):NULL);
	
	if(t.wMilliseconds > 200)
	{
		KillTimer(hwnd, 1);
		bTimerTesting = TRUE;
		SetTimer(hwnd, 1, 1001 - t.wMilliseconds, NULL);
	}
	else if(bTimerTesting)
	{
		KillTimer(hwnd, 1);
		bTimerTesting = FALSE;
		SetTimer(hwnd, 1, 1000, NULL);
	}
	
	if(CheckDaylightTimeTransition(&t))
	{
		CallWindowProc(oldWndProc, hwnd, WM_TIMER, 0, 0);
		GetDisplayTime(&t, nDispBeat?(&beat100):NULL);
	}
	
	bRedraw = FALSE;
	if(nBlink > 0) {
//			APPBARDATA abd;
		bRedraw = TRUE;
/* --+++--> This Will Disable the AutoHide...
		abd.cbSize = sizeof(APPBARDATA);
		abd.hWnd = FindWindow("Shell_TrayWnd","");
		abd.lParam = ABS_ALWAYSONTOP;
		SHAppBarMessage(ABM_SETSTATE, &abd); ...Which Ain't What We're After! <+-*/

	}

	else if(bDispSecond) bRedraw = TRUE;
	else if(nDispBeat == 1 && beatLast != (beat100/100)) bRedraw = TRUE;
	else if(nDispBeat == 2 && beatLast != beat100) bRedraw = TRUE;
//	else if(bDispSysInfo) bRedraw = TRUE;
	else if(LastTime.wHour != (int)t.wHour 
		|| LastTime.wMinute != (int)t.wMinute) bRedraw = TRUE;
	
	if(bNoClock) bRedraw = FALSE;

	if((LastTime.wHour != (int)t.wHour) && (bRedraw)) RefreshUs();
	
	if(LastTime.wDay != t.wDay || LastTime.wMonth != t.wMonth ||
		LastTime.wYear != t.wYear)
	{
		InitFormat(&t); // format.c
		InitDaylightTimeTransition();
	}
	
	hdc = NULL;
	if(bRedraw) hdc = GetDC(hwnd);
	
	memcpy(&LastTime, &t, sizeof(t));
	
	if(nDispBeat == 1) beatLast = beat100/100;
	else if(nDispBeat > 1) beatLast = beat100;
	
	if(nBlink >= 3 && t.wMinute == 1) nBlink = 0;
	
	if(hdc)
	{
		DrawClockSub(hwnd, hdc, &t, beat100); //•`‰æ
		ReleaseDC(hwnd, hdc);
	}
	
	if(nBlink)
	{
		if(nBlink % 2) nBlink++;
		else nBlink--;
	}
}

/*------------------------------------------------
--------------------------------------------------*/
void DrawClock(HWND hwnd, HDC hdc) {
	SYSTEMTIME t;
	int beat100;
	
  GetDisplayTime(&t, nDispBeat?(&beat100):NULL);
  DrawClockSub(hwnd, hdc, &t, beat100);
}

/*------------------------------------------------
  draw the clock
--------------------------------------------------*/
void DrawClockSub(HWND hwnd, HDC hdc, SYSTEMTIME* pt, int beat100)
{
	BITMAP bmp;
	RECT rcFill,  rcClock;
	TEXTMETRIC tm;
	int hf, y, w;
	char s[1024], *p, *sp;
	SIZE sz;
	int xclock, yclock, wclock, hclock, xsrc, ysrc, wsrc, hsrc;
	int xcenter;
	
	if(!hdcClock) CreateClockDC(hwnd);
	
	if(!hdcClock || !hbmpClock) return;
	
	GetObject(hbmpClock, sizeof(BITMAP), (LPVOID)&bmp);
	rcFill.left = rcFill.top = 0;
	rcFill.right = bmp.bmWidth; rcFill.bottom = bmp.bmHeight;
	
	FillClock(hwnd, hdcClock, &rcFill, nBlink);
	
	MakeFormat(s, pt, beat100, format);
	
	GetClientRect(hwndClock, &rcClock);

	xclock = 0;
	yclock = 0;

	wclock = rcClock.right;  hclock = rcClock.bottom;
	
	GetTextMetrics(hdcClock, &tm);
	
	hf = tm.tmHeight - tm.tmInternalLeading;
	p = s;
	y = hf / 4 - tm.tmInternalLeading / 2;
	xcenter = wclock / 2;
	w = 0;
	while(*p)
	{
		sp = p;
		while(*p && *p != 0x0d) p++;
		if(*p == 0x0d) { *p = 0; p += 2; }
		if(*p == 0 && sp == s)
		{
			y = (hclock - tm.tmHeight) / 2  - tm.tmInternalLeading / 4;
		}
		TextOut(hdcClock, xcenter + dhpos, y + dvpos, sp, (int)strlen(sp));
		
		if(GetTextExtentPoint32(hdcClock, sp, (int)strlen(sp), &sz) == 0)
			sz.cx = (LONG)(int)strlen(sp) * tm.tmAveCharWidth;
		if(w < sz.cx) w = sz.cx;
		
		y += hf; if(*p) y += 2 + dlineheight;
	}
	
	xsrc = 0; ysrc = 0; wsrc = rcFill.right; hsrc = rcFill.bottom;

	BitBlt(hdc, 0, 0, wsrc, hsrc, hdcClock, xsrc, ysrc, SRCCOPY);
	
	w += tm.tmAveCharWidth * 2;
	w += dwidth;
	if(w > iClockWidth)
	{
		iClockWidth = w;
		PostMessage(GetParent(GetParent(hwndClock)), WM_SIZE,
			SIZE_RESTORED, 0);
	}
}

/*--------------------------------------------------
-------------------------- paint background of clock
--------------------------------------------------*/
void FillClock(HWND hwnd, HDC hdc, RECT *prc, int nblink) {
	HBRUSH hbr;
	COLORREF col;

	if(nblink == 0 || (nblink % 2)) col = colfore;
	else col = colback; // <--+++--<<<< THIS IS WHERE THE BLINK COLOR ISSUE IS/STARTS/NEEDS TO GO!!!
	SetTextColor(hdc, col);
	
  if(IsXPStyle()) {
	 DrawXPClockBackground(hwnd, hdc, 0);
  }else{ // -------------- fill the clock/tray with simple colors
     if(nblink || colback == colback2) { // - only a single color
		if(nblink == 0 || (nblink % 2)) col = colback;
		else col = colfore;
		hbr = CreateSolidBrush(col);
		FillRect(hdc, prc, hbr);
		DeleteObject(hbr);
	 }
  }
}

/*------------------------------------------------
--------------------------------------------------*/
LRESULT OnCalcRect(HWND hwnd)
{
	SYSTEMTIME t;
	int beat100;
	LRESULT w, h;
	HDC hdc;
	TEXTMETRIC tm;
	char s[1024], *p, *sp;
	SIZE sz;
	int hf;
	
	if(!(GetWindowLong(hwnd, GWL_STYLE)&WS_VISIBLE)) return 0;
	
	hdc = GetDC(hwnd);
	
	if(hFon) SelectObject(hdc, hFon);
	GetTextMetrics(hdc, &tm);
	
	GetDisplayTime(&t, nDispBeat?(&beat100):NULL);
	MakeFormat(s, &t, beat100, format);
	
	p = s; w = 0; h = 0;
	hf = tm.tmHeight - tm.tmInternalLeading;
	while(*p)
	{
		sp = p;
		while(*p && *p != 0x0d) p++;
		if(*p == 0x0d) { *p = 0; p += 2; }
		if(GetTextExtentPoint32(hdc, sp, (int)strlen(sp), &sz) == 0)
			sz.cx = (LONG)strlen(sp) * tm.tmAveCharWidth;
		if(w < sz.cx) w = sz.cx;
		h += hf; if(*p) h += 2 + dlineheight;
	}
	w += tm.tmAveCharWidth * 2;
	if(iClockWidth < 0) iClockWidth = (int)(LRESULT)w;
	else w = iClockWidth;
	w += dwidth;
	
	h += hf / 2 + dheight;
	if(h < 4) h = 4;
	
	ReleaseDC(hwnd, hdc);
	
	return (h << 16) + w;
}

void OnTooltipNeedText(UINT code, LPARAM lParam) {
	SYSTEMTIME t;
	int beat100;
	char fmt[1024], s[1024];

  GetMyRegStr("Tooltip", "Tooltip", fmt, 1024, "");
  if(fmt[0] == 0) strcpy(fmt, "\"TClock\" LDATE");

  GetDisplayTime(&t, &beat100);
  MakeFormat(s, &t, beat100, fmt);

  if(code == TTN_NEEDTEXT) strcpy(((LPTOOLTIPTEXT)lParam)->szText, s);
  else {
	MultiByteToWideChar(CP_ACP, 0, s, -1, ((LPTOOLTIPTEXTW)lParam)->szText, 80);
  }
 SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
}

/*--------------------------------------------------
------------------- copy date/time text to clipboard
--------------------------------------------------*/
void OnCopy(HWND hwnd, LPARAM lParam) {
	SYSTEMTIME t;	HGLOBAL hg;
	char entry[20], fmt[256], s[1024], *pbuf;
	int beat100;
	
  GetDisplayTime(&t, &beat100);
  wsprintf(entry, "%d%dClip", (int)LOWORD(lParam), (int)HIWORD(lParam));
  GetMyRegStr("Mouse", entry, fmt, 256, "");
  if(fmt[0] == 0) strcpy(fmt, format);
	
	MakeFormat(s, &t, beat100, fmt);
	
	if(!OpenClipboard(hwnd)) return;
	EmptyClipboard();
	hg = GlobalAlloc(GMEM_DDESHARE, strlen(s) + 1);
	pbuf = (char*)GlobalLock(hg);
	strcpy(pbuf, s);
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard(); 
}
	 //============================================================================================
//	char szTZone[32] = {0}; //---+++--> TimeZone String Buffer, Also Used (as External) in Format.c
   //==============================================================================================
  int iHourTransition = -1, iMinuteTransition = -1; //--++--> Used Only in the Two Functions Below!
 //================================================================================================
//-----------------------------+++--> Initialize Clock With the User's Local Time-Zone Information:
void InitDaylightTimeTransition(void) { //--------------------------------------------------+++-->
	TIME_ZONE_INFORMATION tzi;
	SYSTEMTIME lt, *plt;
	DWORD dw;
	BOOL b;
	
	iHourTransition = iMinuteTransition = -1;
	
	GetLocalTime(&lt);
	
	b = FALSE;
	memset(&tzi, 0, sizeof(tzi));
	dw = GetTimeZoneInformation(&tzi);
	if(dw == TIME_ZONE_ID_STANDARD // This Will Only Apply in the Fall/Winter Months When DST is NOT in Effect.
	  && tzi.DaylightDate.wMonth == lt.wMonth
	  && tzi.DaylightDate.wDayOfWeek == lt.wDayOfWeek)
	{
		b = TRUE; plt = &(tzi.DaylightDate);
//		strcpy(szTZone, (char *)tzi.StandardName);
//		wcstombs(szTZone, tzi.StandardName, 32);
//		wsprintf(szTZone, "%S", tzi.StandardName);
	}
	if(dw == TIME_ZONE_ID_DAYLIGHT // This Will Only Apply in the Spring/Summer Months When DST IS in Effect.
	  && tzi.StandardDate.wMonth == lt.wMonth
	  && tzi.StandardDate.wDayOfWeek == lt.wDayOfWeek)
	{
		b = TRUE; plt = &(tzi.StandardDate);
//		strcpy(szTZone, tzi.DaylightName);
//		wcstombs(szTZone, tzi.DaylightName, 32);
//		wsprintf(szTZone, "%S", tzi.DaylightName);
	}
	
	if(b && plt->wDay < 5) {
		if(((lt.wDay - 1) / 7 + 1) == plt->wDay) {
			iHourTransition = plt->wHour;
			iMinuteTransition = plt->wMinute;
		}
	}
	else if(b && plt->wDay == 5) {
		FILETIME ft;
		SystemTimeToFileTime(&lt, &ft);
		*(DWORDLONG*)&ft += 6048000000000i64;
		FileTimeToSystemTime(&ft, &lt);
		if(lt.wDay < 8) {
			iHourTransition = plt->wHour;
			iMinuteTransition = plt->wMinute;
		}
	}	
//	wsprintf(szTZone, "Day: %S, Std: %S", tzi.DaylightName, TEXT(tzi.StandardName));
//		MessageBox(0, szTZone, "is TimeZone??", MB_OK);
}
 //================================================================================================
//-+++--> iHourTransition & iMinuteTransition Are Now Used to Pass Local Time-Zone Offset to Clock:
BOOL CheckDaylightTimeTransition(SYSTEMTIME *plt) { //--------------------------------------+++-->
  if((int)plt->wHour == iHourTransition &&
	 (int)plt->wMinute >= iMinuteTransition) {
		iHourTransition = iMinuteTransition = -1;
		return TRUE;
  }
 else return FALSE;
}