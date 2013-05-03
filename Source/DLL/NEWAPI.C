/*-------------------------------------------
  newapi.c - Kazubon 1999
  GradientFill and Layerd Window
---------------------------------------------*/
#include "tcdll.h"

//#define WS_EX_LAYERED 0x80000
//#define LWA_ALPHA     2
//#define LWA_COLORKEY  1

HMODULE hmodMSIMG32 = NULL;
HMODULE hmodUSER32 = NULL;
HMODULE hmodUxTheme = NULL;

BOOL (WINAPI *pGradientFill)(HDC,PTRIVERTEX,ULONG,PVOID,ULONG,ULONG) = NULL;

BOOL (WINAPI *pSetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD) = NULL;

BOOL (WINAPI *pTransparentBlt)(HDC,int,int,int,int,HDC,int,int,int,int,UINT) = NULL;

BOOL (WINAPI *pDrawThemeParentBackground)(HWND hwnd, HDC hdc, RECT *prc) = NULL;

static BOOL bInitGradientFill = FALSE;
static BOOL bInitLayeredWindow = FALSE;
static BOOL bInitTransparentBlt = FALSE;
static BOOL bInitDrawThemeParentBackground = FALSE;

static void RefreshRebar(HWND hwndBar);
//static void RefreshUs(void);

int nAlphaDesktop = 255;
BOOL bClearTaskbar = FALSE;
extern BOOL bRefreshClearTaskbar;

void InitGradientFill(void)
{
	if(bInitGradientFill) return;
	
	hmodMSIMG32 = LoadLibrary("msimg32.dll");
	if(hmodMSIMG32 != NULL)
	{
		(FARPROC)pGradientFill = GetProcAddress(hmodMSIMG32, "GradientFill");
		if(pGradientFill == NULL)
		{
			FreeLibrary(hmodMSIMG32); hmodMSIMG32 = NULL;
		}
	}
	bInitGradientFill = TRUE;
}

void InitTransparentBlt(void)
{
	if(bInitTransparentBlt) return;
	
	hmodMSIMG32 = LoadLibrary("msimg32.dll");
	if(hmodMSIMG32 != NULL)
	{
		(FARPROC)pTransparentBlt = GetProcAddress(hmodMSIMG32, "TransparentBlt");
		if(pTransparentBlt == NULL)
		{
			FreeLibrary(hmodMSIMG32); hmodMSIMG32 = NULL;
		}
	}
	bInitTransparentBlt = TRUE;
}

void InitLayeredWindow(void)
{
	if(bInitLayeredWindow) return;
	
	hmodUSER32 = LoadLibrary("user32.dll");
	if(hmodUSER32 != NULL)
	{
		(FARPROC)pSetLayeredWindowAttributes = 
			GetProcAddress(hmodUSER32, "SetLayeredWindowAttributes");
		if(pSetLayeredWindowAttributes == NULL)
		{
			FreeLibrary(hmodUSER32); hmodUSER32 = NULL;
		}
	}
	bInitLayeredWindow = TRUE;
}

void EndNewAPI(HWND hwndClock)
{
	if(hmodMSIMG32 != NULL) FreeLibrary(hmodMSIMG32);
	hmodMSIMG32 = NULL; pGradientFill = NULL; pTransparentBlt = NULL;
	
	if(pSetLayeredWindowAttributes)
	{
		HWND hwnd;
		LONG exstyle;
		
		hwnd = GetParent(GetParent(hwndClock));
		exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
		if(exstyle & WS_EX_LAYERED)
		{
			exstyle &= ~WS_EX_LAYERED;
			SetWindowLong(hwnd, GWL_EXSTYLE, exstyle);
			bClearTaskbar = FALSE;
			RefreshRebar(hwnd);
		}
	}
	
	if(hmodUSER32 != NULL) FreeLibrary(hmodUSER32);
	hmodUSER32 = NULL;
	pSetLayeredWindowAttributes = NULL;

	if(hmodUxTheme != NULL) FreeLibrary(hmodUxTheme);
	hmodUxTheme = NULL;
	pDrawThemeParentBackground = NULL;

}
/*
void GradientFillClock(HDC hdc, RECT* prc, COLORREF col1, COLORREF col2) {
	TRIVERTEX vert[2];
	GRADIENT_RECT gRect;
	
  if(!pGradientFill) InitGradientFill();
  if(!pGradientFill) {
	 return;
  }
	
	vert[0].x      = prc->left;
	vert[0].y      = prc->top;
	vert[0].Red    = (COLOR16)GetRValue(col1) * 256;
	vert[0].Green  = (COLOR16)GetGValue(col1) * 256;
	vert[0].Blue   = (COLOR16)GetBValue(col1) * 256;
	vert[0].Alpha  = 0x0000;
	vert[1].x      = prc->right;
	vert[1].y      = prc->bottom; 
	vert[1].Red    = (COLOR16)GetRValue(col2) * 256;
	vert[1].Green  = (COLOR16)GetGValue(col2) * 256;
	vert[1].Blue   = (COLOR16)GetBValue(col2) * 256;
	vert[1].Alpha  = 0x0000;
	gRect.UpperLeft  = 0;
	gRect.LowerRight = 1;

  pGradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
}
*/
void TransBlt(HDC dhdc, int dx, int dy, int dw, int dh, HDC shdc, int sx, int sy, int sw, int sh) {
  if(!pTransparentBlt) InitTransparentBlt();
  if(!pTransparentBlt) {
	 return;
  }
  pTransparentBlt(dhdc, dx, dy, dw, dh, shdc, sx, sy, sw, sh, RGB(255, 0, 255));
}

void SetLayeredTaskbar(HWND hwndClock)
{
	LONG exstyle;
	HWND hwnd;
	int alpha;
	
	alpha = GetMyRegLongEx("Taskbar", "AlphaTaskbar", 0);
	bClearTaskbar = GetMyRegLongEx("Taskbar", "ClearTaskbar", FALSE);
	alpha = 255 - (alpha * 255 / 100);
	if(alpha < 8) alpha = 8; else if(alpha > 255) alpha = 255;
	
	if(!pSetLayeredWindowAttributes && (alpha < 255 || bClearTaskbar)) InitLayeredWindow();
	if(!pSetLayeredWindowAttributes) return;
	
	hwnd = GetParent(GetParent(hwndClock));
	
	exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	if(alpha < 255 || bClearTaskbar) exstyle |= WS_EX_LAYERED;
	else exstyle &= ~WS_EX_LAYERED;
	SetWindowLong(hwnd, GWL_EXSTYLE, exstyle);
	if(bRefreshClearTaskbar)
	{
		bRefreshClearTaskbar = FALSE;
		RefreshUs();
	}
	RefreshRebar(hwnd);
	
	if(alpha < 255 && !bClearTaskbar)
		pSetLayeredWindowAttributes(hwnd, 0, (BYTE)alpha, LWA_ALPHA);
	else if(bClearTaskbar)
		pSetLayeredWindowAttributes(hwnd, GetSysColor(COLOR_3DFACE), (BYTE)alpha, LWA_COLORKEY|LWA_ALPHA);

}

/*--------------------------------------------------
    redraw ReBarWindow32 forcely
----------------------------------------------------*/
void RefreshRebar(HWND hwndBar)
{
	HWND hwnd;
	char classname[80];
	
	hwnd = GetWindow(hwndBar, GW_CHILD);
	while(hwnd)
	{
		GetClassName(hwnd, classname, 80);
		if(lstrcmpi(classname, "ReBarWindow32") == 0)
		{
			InvalidateRect(hwnd, NULL, TRUE);
			hwnd = GetWindow(hwnd, GW_CHILD);
			while(hwnd)
			{
				InvalidateRect(hwnd, NULL, TRUE);
				hwnd = GetWindow(hwnd, GW_HWNDNEXT);
			}
			break;
		}
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
}

//force ourselves to redraw all our settings
void RefreshUs(void)
{
	HWND hwndBar;
	HWND hwndChild;
	char classname[80];

	hwndBar = FindWindow("Shell_TrayWnd", NULL);

	// find the clock window
	hwndChild = GetWindow(hwndBar, GW_CHILD);
	while(hwndChild)
	{
		GetClassName(hwndChild, classname, 80);
		//WriteDebug(classname);
		if(lstrcmpi(classname, "TrayNotifyWnd") == 0)
		{
			hwndChild = GetWindow(hwndChild, GW_CHILD);
			while(hwndChild)
			{
				GetClassName(hwndChild, classname, 80);
				//WriteDebug(classname);
				if(lstrcmpi(classname, "TrayClockWClass") == 0)
				{
					SendMessage(hwndChild, CLOCKM_REFRESHCLOCK, 0, 0);
					SendMessage(hwndChild, CLOCKM_REFRESHTASKBAR, 0, 0);
					break;
				}
			}
			break;
		}
		hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
	}
}

void TC2DrawBlt(HDC dhdc, int dx, int dy, int dw, int dh, HDC shdc, int sx, int sy, int sw, int sh, BOOL useTrans) {
  if(useTrans) TransBlt(dhdc, dx, dy, dw, dh, shdc, sx, sy, sw, sh);
  else StretchBlt(dhdc, dx, dy, dw, dh, shdc, sx, sy, sw, sh, SRCCOPY);
}

void InitDrawThemeParentBackground(void) {
  if(bInitDrawThemeParentBackground) return;
	
  hmodUxTheme = LoadLibrary("UxTheme.dll");
  if(hmodUxTheme != NULL) {
	 (FARPROC)pDrawThemeParentBackground = GetProcAddress(hmodUxTheme, "DrawThemeParentBackground");
	 if(pDrawThemeParentBackground == NULL) {
		FreeLibrary(hmodUxTheme); hmodUxTheme = NULL;
	 }
  }
 bInitDrawThemeParentBackground = TRUE;
}

void DrawXPClockBackground(HWND hwnd, HDC hdc, RECT *prc) {
  if(!pDrawThemeParentBackground) InitDrawThemeParentBackground();
  if(!pDrawThemeParentBackground)return;
 pDrawThemeParentBackground(hwnd, hdc, prc);
}