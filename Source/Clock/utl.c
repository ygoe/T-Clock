/*-------------------------------------------
  utl.c - KAZUBON 1997-1998
---------------------------------------------*/
// Modified by Stoic Joker: Monday, 03/22/2010 @ 7:32:29pm
#include "tclock.h"
char mykey[] = "Software\\Stoic Joker's\\T-Clock 2010";

int atox(const char *p) {
	int r = 0;
  while(*p) {
	if('0' <= *p && *p <= '9') r = r * 16 + *p - '0';
	else if('A' <= *p && *p <= 'F') r = r * 16 + *p - 'A' + 10;
	else if('a' <= *p && *p <= 'f') r = r * 16 + *p - 'a' + 10;
	p++;
  }
 return r;
}

__inline int toupper(int c) {
  if('a' <= c && c <= 'z') c -= 'a' - 'A';
 return c;
}

void add_title(char *path, char *title)
{
	char *p;
	
	p = path;
	
	if(*p == 0) ;
	else if(*title && *(title + 1) == ':') ;
	else if(*title == '\\')
	{
		if(*p && *(p + 1) == ':') p += 2;
	}
	else
	{
		while(*p)
		{
			if((*p == '\\' || *p == '/') && *(p + 1) == 0)
			{
				break;
			}
			p = CharNext(p);
		}
		*p++ = '\\';
	}
	while(*title) *p++ = *title++;
	*p = 0;
}

void del_title(char *path)
{
	char *p, *ep;

	p = ep = path;
	while(*p)
	{
		if(*p == '\\' || *p == '/')
		{
			if(p > path && *(p - 1) == ':') ep = p + 1;
			else ep = p;
		}
		p = CharNext(p);
	}
	*ep = 0;
}

void get_title(char* dst, const char *path)
{
	const char *p, *ep;

	p = ep = path;
	while(*p)
	{
		if(*p == '\\' || *p == '/')
		{
			if(p > path && *(p - 1) == ':') ep = p + 1;
			else ep = p;
		}
		p = CharNext(p);
	}
	
	if(*ep == '\\' || *ep == '/') ep++;
	
	while(*ep) *dst++ = *ep++;
	*dst = 0;
}

int ext_cmp(const char *fname, const char *ext)
{
	const char* p, *sp;
	
	sp = NULL; p = fname;
	while(*p)
	{
		if(*p == '.') sp = p;
		else if(*p == '\\' || *p == '/') sp = NULL;
		p = CharNext(p);
	}
	
	if(sp == NULL) sp = p;
	if(*sp == '.') sp++;
	
	while(1)
	{
		if(*sp == 0 && *ext == 0) return 0;
		if(toupper(*sp) != toupper(*ext))
			return (toupper(*sp) - toupper(*ext));
		sp++; ext++;
	}
	return 0;
}

void parse(char *dst, char *src, int n)
{
	char *dp;
	int i;

	for(i = 0; i < n; i++)
	{
		while(*src && *src != ',') src++;
		if(*src == ',') src++;
	}
	if(*src == 0) 
	{
		*dst = 0; return;
	}
	
	while(*src == ' ') src++;
	
	dp = dst;
	while(*src && *src != ',') *dst++ = *src++;
	*dst = 0;
	
	while(dst != dp)
	{
		dst--;
		if(*dst == ' ') *dst = 0;
		else break;
	}
}

void parsechar(char *dst, char *src, char ch, int n)
{
	char *dp;
	int i;

	for(i = 0; i < n; i++)
	{
		while(*src && *src != ch) src++;
		if(*src == ch) src++;
	}
	if(*src == 0) 
	{
		*dst = 0; return;
	}
	
	while(*src == ' ') src++;
	
	dp = dst;
	while(*src && *src != ch) *dst++ = *src++;
	*dst = 0;
	
	while(dst != dp)
	{
		dst--;
		if(*dst == ' ') *dst = 0;
		else break;
	}
}

void str0cat(char* dst, const char* src)
{
	char* p;
	p = dst;
	while(*p) { while(*p) p++; p++; }
	strcpy(p, src);
	while(*p) p++; p++; *p = 0;
}

/*---------------------------------------------
--------------------- returns a resource string
---------------------------------------------*/
char* MyString(UINT id) {
	static char buf[80];
	
  buf[0] = 0;
  LoadString(GetModuleHandle(NULL), id, buf, 80);
 return buf;
}

int MyMessageBox(HWND hwnd, char* msg, char* title, UINT uType, UINT uBeep) {
	MSGBOXPARAMS mbp;
  memset(&mbp, 0, sizeof(MSGBOXPARAMS));
	mbp.cbSize = sizeof(MSGBOXPARAMS);
	mbp.hwndOwner = hwnd;
	mbp.hInstance = GetModuleHandle(NULL);
	mbp.lpszText = msg;
	mbp.lpszCaption = title;
	mbp.dwStyle = MB_USERICON | uType;
	mbp.lpszIcon = MAKEINTRESOURCE(IDI_ICON1);
	mbp.dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
  if(uBeep != 0xFFFFFFFF) MessageBeep(uBeep);
 return MessageBoxIndirect(&mbp);
}
 //================================================================================================
//----------------------------------------//--------------------------+++--> 32bit x 32bit = 64bit:
DWORDLONG M32x32to64(DWORD a, DWORD b) { //-------------------------------------------------+++-->
	ULARGE_INTEGER r;
	DWORD *p1, *p2, *p3;
	memset(&r, 0, 8);
	p1 = &r.LowPart;
	p2 = (DWORD*)((BYTE*)p1 + 2);
	p3 = (DWORD*)((BYTE*)p2 + 2);
	*p1 = LOWORD(a) * LOWORD(b);
	*p2 += LOWORD(a) * HIWORD(b) + HIWORD(a) * LOWORD(b);
	*p3 += HIWORD(a) * HIWORD(b);
	return *(DWORDLONG*)(&r);
}
 //================================================================================================
//-----------+++--> (Momentarily) Force Window X Into the Foreground So we Interact With IT (only):
void ForceForegroundWindow(HWND hWnd) { //---{ Required to Dismiss Context Menu Properly }--+++-->
	DWORD thread1, thread2, pid;

  thread1 = GetWindowThreadProcessId(GetForegroundWindow(), &pid);
  thread2 = GetCurrentThreadId();
  
  AttachThreadInput(thread2, thread1, TRUE);
  SetForegroundWindow(hWnd);
  
  AttachThreadInput(thread2, thread1, FALSE);
  BringWindowToTop(hWnd);
}
 //===============================================================================
//--+++-->
int GetMyRegStr(char* section, char* entry, char* val, int cbData, char* defval) {
	HKEY hkey;	char key[80];	DWORD regtype, size;	int r;
	
  strcpy(key, mykey);
	
  if(section && *section) {
	 strcat(key, "\\");
	 strcat(key, section);
  }

  if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == ERROR_SUCCESS) {
	 size = cbData;
	 if(RegQueryValueEx(hkey, entry, 0, &regtype, (LPBYTE)val, &size) == ERROR_SUCCESS) {
		if(size == 0) *val = 0;
		r = size;
	 }
	RegCloseKey(hkey);
  }
 return r;
}

int GetMyRegStrEx(char* section, char* entry, char* val, int cbData, char* defval) {
	HKEY hkey;	char key[80];	DWORD regtype, size;	BOOL b = FALSE;	int r;
	
  strcpy(key, mykey);
	
  if(section && *section) {
	 strcat(key, "\\");
	 strcat(key, section);
  }
	
  if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == ERROR_SUCCESS) {
	 size = cbData;
	 if(RegQueryValueEx(hkey, entry, 0, &regtype, (LPBYTE)val, &size) == ERROR_SUCCESS) {
		if(size == 0) *val = 0;
		   r = size;
		   b = TRUE;
	 }
	 RegCloseKey(hkey);
  }

  if(b == FALSE) {
	 SetMyRegStr(section, entry, defval);
	 strcpy(val, defval);
	 r = (int)strlen(defval);
  }
 return r;
}

LONG GetMyRegLong(char* section, char* entry, LONG defval) {
	HKEY hkey;	char key[80];	DWORD regtype, size;
	BOOL b = FALSE;	LONG r;
	
  strcpy(key, mykey);
	
  if(section && *section) {
	 strcat(key, "\\");
	 strcat(key, section);
  }
  
  if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == ERROR_SUCCESS) {
	 size = 4;
	 if(RegQueryValueEx(hkey, entry, 0, &regtype, (LPBYTE)&r, &size) == ERROR_SUCCESS) {
		if(size == 4) b = TRUE;
	 }
	 RegCloseKey(hkey);
  }

  if(b == FALSE) r = defval;
 return r;
}

LONG GetMyRegLongEx(char* section, char* entry, LONG defval) {
	HKEY hkey;	char key[80];	DWORD regtype, size;
	BOOL b = FALSE;	LONG r;
	
  strcpy(key, mykey);
	
  if(section && *section) {
	 strcat(key, "\\");
	 strcat(key, section);
  }
  
  if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == ERROR_SUCCESS) {
	 size = 4;
	 if(RegQueryValueEx(hkey, entry, 0, &regtype, (LPBYTE)&r, &size) == ERROR_SUCCESS) {
	    if(size == 4) b = TRUE;
	 }
	 RegCloseKey(hkey);
  }

  if(b == FALSE) {
	 r = defval;
	 SetMyRegLong(section, entry, defval);
  }
 return r;
}

/*------------------------------------------------
  get DWORD value from registry
--------------------------------------------------*/
LONG GetRegLong(HKEY rootkey, char *subkey, char *entry, LONG defval) {
	HKEY hkey;	DWORD regtype, size;	BOOL b = FALSE;	int r;
	
  if(RegOpenKey(rootkey, subkey, &hkey) == ERROR_SUCCESS) {
	 size = 4;
	 if(RegQueryValueEx(hkey, entry, 0, &regtype, (LPBYTE)&r, &size) == ERROR_SUCCESS) {
	    if(size == 4) b = TRUE;
	 }
	 RegCloseKey(hkey);
  }
  if(b == FALSE) r = defval;
 return r;
}

int GetRegStr(HKEY rootkey, char *subkey, char* entry, char* val, int cbData, char* defval) {
	HKEY hkey;	DWORD regtype, size;	BOOL b = FALSE;	int r;
	
  if(RegOpenKey(rootkey, subkey, &hkey) == ERROR_SUCCESS) {
	 size = cbData;
	 if(RegQueryValueEx(hkey, entry, 0, &regtype, (LPBYTE)val, &size) == ERROR_SUCCESS) {
	    if(size == 0) *val = 0;
		b = TRUE;
	 }
	 RegCloseKey(hkey);
  }
  
  if(b == FALSE) {
	 strcpy(val, defval);
	 r = (int)strlen(defval);
  }
 return r;
}

BOOL SetMyRegStr(char* section, char* entry, char* val) {
	HKEY hkey;	char key[80];	BOOL r;
	
  strcpy(key, mykey);
	
  if(section && *section) {
	 strcat(key, "\\");
	 strcat(key, section);
  }
  
  if(RegCreateKey(HKEY_CURRENT_USER, key, &hkey) == ERROR_SUCCESS) {
	 if(RegSetValueEx(hkey, entry, 0, REG_SZ, (CONST BYTE*)val, (DWORD)(int)strlen(val)) == ERROR_SUCCESS) {
		r = TRUE;
	 }
	 RegCloseKey(hkey);
  }
 return r;
}

BOOL SetRegStr(HKEY rootkey, char* subkey, char* entry, char* val) {
	HKEY hkey;	BOOL r = FALSE;

  if(RegCreateKey(rootkey, subkey, &hkey) == ERROR_SUCCESS) {
	 if(RegSetValueEx(hkey, entry, 0, REG_SZ, (CONST BYTE*)val, (DWORD)(int)strlen(val)) == ERROR_SUCCESS) {
		r = TRUE;
	 }
	 RegCloseKey(hkey);
  }
 return r;
}

BOOL SetMyRegLong(char* section, char* entry, DWORD val) {
	HKEY hkey;	char key[80];	BOOL r = FALSE;
	
  strcpy(key, mykey);
	
  if(section && *section) {
	 strcat(key, "\\");
	 strcat(key, section);
  }
  
  if(RegCreateKey(HKEY_CURRENT_USER, key, &hkey) == ERROR_SUCCESS) {
	 if(RegSetValueEx(hkey, entry, 0, REG_DWORD, (CONST BYTE*)&val, 4) == ERROR_SUCCESS) {
		r = TRUE;
	 }
	 RegCloseKey(hkey);
  }
 return r;
}

BOOL DelMyReg(char* section, char* entry) {
	HKEY hkey;	char key[80];	BOOL r = FALSE;
	
  strcpy(key, mykey);
	
  if(section && *section) {
	 strcat(key, "\\");
	 strcat(key, section);
  }
  
  if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == ERROR_SUCCESS) {
	 if(RegDeleteValue(hkey, entry) == ERROR_SUCCESS) r = TRUE;
	 RegCloseKey(hkey);
  }
 return r;
}

BOOL DelMyRegKey(char* section) {
	char key[80];	BOOL r = FALSE;
	
  strcpy(key, mykey);
	
  if(section && *section) {
	 strcat(key, "\\");
	 strcat(key, section);
  }
  
  if(RegDeleteKey(HKEY_CURRENT_USER, key) == ERROR_SUCCESS) r = TRUE;
 return r;
}

COLORREF GetMyRegColor(char* section, char* entry, COLORREF defval) {
	HKEY hkey;	char key[80];	DWORD regtype, size;
	BOOL b = FALSE;	LONG r;
	
  strcpy(key, mykey);
	
  if(section && *section) {
	 strcat(key, "\\");
	 strcat(key, section);
  }
  
  if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == ERROR_SUCCESS) {
	 size = 4;
	 if(RegQueryValueEx(hkey, entry, 0, &regtype, (LPBYTE)&r, &size) == ERROR_SUCCESS) {
		if(size == 4) b = TRUE;
	 }
	 RegCloseKey(hkey);
  }
  
  if(b == FALSE) r = defval;
  if(r & 0x80000000) r = GetSysColor(r & 0x00ffffff);
 return r;
}

void OnChooseColor(HWND hDlg, WORD id, WORD idCombo)
{
	CHOOSECOLOR cc;
	COLORREF col, colarray[16];
	int i;
	
	col = (COLORREF)CBGetItemData(hDlg, idCombo, CBGetCurSel(hDlg, idCombo));
	if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
	
	for(i = 0; i < 16; i++) colarray[i] = RGB(255,255,255);
	
	memset(&cc, 0, sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hDlg;
	cc.hInstance = (HWND)(HINSTANCE)GetModuleHandle(NULL);
	cc.rgbResult = col;
	cc.lpCustColors = colarray;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	
	if(!ChooseColor(&cc)) return;
	
	for(i = 0; i < 16; i++)
	{
		if(cc.rgbResult == (COLORREF)CBGetItemData(hDlg, idCombo, i))
			break;
	}
	if(i == 16) //Šî–{16F‚Å‚Í‚È‚¢‚Æ‚«
	{
		if(CBGetCount(hDlg, idCombo) == 20)
			CBAddString(hDlg, idCombo, cc.rgbResult);
		else
			CBSetItemData(hDlg, idCombo, 20, cc.rgbResult);
		i = 20;
	}
	CBSetCurSel(hDlg, idCombo, i);
}