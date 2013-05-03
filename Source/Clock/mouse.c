/*-------------------------------------------
  mouse.c - KAZUBON 1997-2001
  mouse operation
---------------------------------------------*/
// Last Modified by Stoic Joker: Sunday, 01/09/2011 @ 4:34:56pm
#include "tclock.h"
/*
// XButton Messages
#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN 0x020B
#define WM_XBUTTONUP   0x020C
#define XBUTTON1       0x0001
#define XBUTTON2       0x0002
#endif
*/
static char reg_section[] = "Mouse";
static UINT last_mousedown  = 0;
static WORD last_xmousedown = 0;
static DWORD last_tickcount;
static int num_click = 0;
static int exec_button = -1;
static BOOL timer = FALSE;

static int GetMouseFuncNum(int button, int nclick);

/*------------------------------------------------
   when files dropped to the clock
--------------------------------------------------*/
void OnDropFiles(HWND hwnd, HDROP hdrop)
{
	char fname[MAX_PATH], sname[MAX_PATH];
	char app[1024];
	SHFILEOPSTRUCT shfos;
	char *buf, *p;
	int i, num;
	int nType;
	
	nType = GetMyRegLong(reg_section, "DropFiles", 0);
	
	num = DragQueryFile(hdrop, (UINT)-1, NULL, 0);
	if(num <= 0) return;
	buf = malloc(num*MAX_PATH);
	if(buf == NULL) return;
	p = buf;
	for(i = 0; i < num; i++)
	{
		DragQueryFile(hdrop, i, fname, MAX_PATH);
		if(nType == 1 || nType == 3 || nType == 4)
		{                           
			strcpy(p, fname); p += strlen(p) + 1;
		}
		else if(nType == 2)
		{                
			if(num > 1) GetShortPathName(fname, sname, MAX_PATH);
			else strcpy(sname, fname);
			strcpy(p, sname);
			p += strlen(p);
			if(num > 1 && i < num - 1) { *p = ' '; p++; }
		}
	}
	*p = 0;
	DragFinish(hdrop);
	
	GetMyRegStr(reg_section, "DropFilesApp", app, 1024, "");
	
	if(nType == 1 || nType == 3 || nType == 4)
	{
		memset(&shfos, 0, sizeof(SHFILEOPSTRUCT));
		shfos.hwnd = NULL;
		if(nType == 1) shfos.wFunc = FO_DELETE;
		else if(nType == 3) shfos.wFunc = FO_COPY;
		else if(nType == 4) shfos.wFunc = FO_MOVE;
		shfos.pFrom = buf;
		if(nType == 3 || nType == 4) shfos.pTo = app;
		shfos.fFlags = FOF_ALLOWUNDO|FOF_NOCONFIRMATION;
		SHFileOperation(&shfos);
	}
	else if(nType == 2)
	{
		char command[MAX_PATH*2];
		//	<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< MODIFY Run With Swithces HERE!!!
		strcpy(command, app);
		strcat(command, " ");
		strcat(command, buf);
		ExecFile(hwnd, command);
///////////////////////////////////////////////////////////////////////////////////////////////////////
/*-----------------------------------------------------------+++--> Will Need to Modify This Function:
BOOL ExecFile(HWND hwnd, char* command) {
	char fname[MAX_PATH], opt[MAX_PATH]; int FUCK = 0;
	
  if(*command == 0) return FALSE;
  GetFileAndOption(command, fname, opt);
  FUCK = (int)(UINT_PTR)(HINSTANCE)ShellExecute(hwnd, NULL, fname, opt[0]?opt:NULL, "", SW_SHOW);
  if(FUCK <= 32) return FALSE;
 return TRUE;
}
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////
	}
	free(buf);
}

/*------------------------------------------------------------
   when the clock clicked
   
   registry format 
   name    value
   03      3           left button triple click -> Minimize All
   32      100         x-1 button  double click -> Run Notepad
   32File  C:\Windows\notepad.exe
--------------------------------------------------------------*/
void OnMouseMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	UINT doubleclick_time;
	BOOL bDown = FALSE;
	int button, i;
	LONG n_func;
	
  if(timer) KillTimer(hwnd, IDTIMER_MOUSE);
  timer = FALSE;

  switch(message) {
	case WM_LBUTTONDOWN: 
	case WM_LBUTTONUP:   button = 0; break;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:   button = 1; break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:   button = 2; break;
   default: return;
  }
	
  switch(message) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	  if(last_mousedown != message) num_click = 0;
		 last_mousedown = message;
		 bDown = TRUE;
	  break;

	case WM_LBUTTONUP:
	  if(last_mousedown != WM_LBUTTONDOWN) last_mousedown = 0;
	  break;

	case WM_RBUTTONUP:
	  if(last_mousedown != WM_RBUTTONDOWN) last_mousedown = 0;
	  break;

	case WM_MBUTTONUP:
	  if(last_mousedown != WM_MBUTTONDOWN) last_mousedown = 0;
	  break;
  }
	
  if(last_mousedown == 0) {
	 num_click = 0;
	 return;
  }
	
	// Mouse double click speed
  doubleclick_time = GetDoubleClickTime();
  if(doubleclick_time < (GetTickCount() - last_tickcount)) num_click = 0;
  last_tickcount = GetTickCount();
	
  if(bDown) {
	 n_func = GetMouseFuncNum(button, num_click + 1);
	 if(n_func >= 0 && n_func != MOUSEFUNC_SCREENSAVER) {
		for(i = num_click + 1; i <= 2; i++) {
			n_func = GetMouseFuncNum(button, i);
			if(n_func >= 0) return;
		}
		num_click++;
		exec_button = button;
		OnTimerMouse(hwnd);
	 }
	return;
  }
	
  num_click++;
  n_func = GetMouseFuncNum(button, num_click);
  if(n_func < 0) return;
	
  for(i = num_click + 1; i <= 2; i++) {
	  n_func = GetMouseFuncNum(button, i);
	  if(n_func >= 0) {
		 exec_button = button;
		 timer = TRUE;
		 SetTimer(hwnd, IDTIMER_MOUSE, doubleclick_time, 0);
		 return;
	  }
  }

  exec_button = button;
  OnTimerMouse(hwnd);
}

/*--------------------------------------------------
----------------------------- Execute Mouse Function
--------------------------------------------------*/
void OnTimerMouse(HWND hwnd) {
	int button;
	LONG n_func;
	
	button = exec_button;
	if(timer) KillTimer(hwnd, IDTIMER_MOUSE); timer = FALSE;
	
	n_func = GetMouseFuncNum(button, num_click);
	
	if(n_func < 0) return;
	
	switch (n_func)
	{
		case MOUSEFUNC_TIMER:
		  DialogTimer(hwnd);
		  break;

		case MOUSEFUNC_SHOWCALENDER:
		  DialogCalender(hwnd);
		  break;

		case MOUSEFUNC_SHOWPROPERTY:
		   MyPropertySheet();
		  break;

		case MOUSEFUNC_CLIPBOARD:
		{	LPARAM lParam;
		  lParam = MAKELONG((WORD)button, (WORD)num_click);
		  PostMessage(g_hwndClock, CLOCKM_COPY, 0, lParam);
		  break;
		}

		case MOUSEFUNC_SCREENSAVER: {
		  SendMessage(GetDesktopWindow(), WM_SYSCOMMAND, SC_SCREENSAVE, 0);
		  break;
		}
	}
}

int GetMouseFuncNum(int button, int nclick) {
	char entry[20];
  wsprintf(entry, "%d%d", button, nclick);
 return GetMyRegLong(reg_section, entry, -1);
}