   //---------------------------------------------------------
  //--------------------+++--> pageabout.c - KAZUBON 1997-1998
 //---------------------------------------------------------*/
// Modified by Stoic Joker: Wednesday, March 3 2010 - 7:17:33
#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnLinkClicked(HWND hDlg, UINT id);

LRESULT CALLBACK LabLinkProc(HWND, UINT, WPARAM, LPARAM);
static WNDPROC oldLabProc = NULL;
static HCURSOR hCurHand = NULL;
static HFONT hfontLink, hFontBold;

BOOL FileExists(HWND hDlg);
static void OnStartup(HWND hDlg);
static void RemoveStartup(HWND hDlg);
BOOL CreateLink(LPCSTR fname, LPCSTR dstpath, LPCSTR name);
#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0)
/////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK PageAboutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  switch(message) {
	case WM_INITDIALOG:
	  OnInit(hDlg);
	  return TRUE;

	case WM_CTLCOLORSTATIC: {
	  int id; HDC hdc;
	  hdc = (HDC)wParam;
	  id = GetDlgCtrlID((HWND)lParam);
	  if(id == IDC_MAILTO || id == IDC_HOMEPAGE) {
		 SetTextColor(hdc, RGB(0,0,255));
		 SetBkMode(hdc, OPAQUE);
		// return (BOOL)(INT_PTR)GetSysColorBrush(COLOR_3DFACE);
	  } break;
	}

	case WM_COMMAND: {
	  WORD id, code;
	  id = LOWORD(wParam);
	  code = HIWORD(wParam);
	  if((id == IDC_MAILTO || id == IDC_HOMEPAGE) && code == STN_CLICKED) {
		  OnLinkClicked(hDlg, id);
	  } 
	  if((id == IDC_STARTUP) && ((code == BST_CHECKED) || (code == BST_UNCHECKED))) {
		 SendPSChanged(hDlg);
	  } return TRUE;
	}

	case WM_NOTIFY:
	  switch(((NMHDR *)lParam)->code) {
		case PSN_APPLY: OnApply(hDlg); break;
	  } return TRUE;

	case WM_DESTROY:
	  DeleteObject(hfontLink);
	  DeleteObject(hFontBold);
	  DestroyWindow(hDlg);
	  break;
  }
 return FALSE;
}
 //================================================================================================
//--------------------+++--> Initialize Properties Dialog & Customize T-Clock Controls as Required:
static void OnInit(HWND hDlg) { //----------------------------------------------------------+++-->
	LOGFONT logfont, ftBold;
	
  SendDlgItemMessage(hDlg, IDC_ABOUTICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconTClock);
  SendDlgItemMessage(hDlg, 42666, STM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconLogo);

  SetDlgItemText(hDlg, IDC_ABT_TCLOCK, ABT_TCLOCK); //--+++--> Load String Data
  SetDlgItemText(hDlg, IDC_STARTUP, AUTO_START); //-+-> Based on Which Platform
  SetDlgItemText(hDlg, IDC_ABT_ME, ABT_ME); // Binary is Compiled for 32/64 Bit
	
  hfontLink = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
  hFontBold = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);

  GetObject(hfontLink, sizeof(LOGFONT), &logfont);
  GetObject(hFontBold, sizeof(LOGFONT), &ftBold);

  ftBold.lfWeight = FW_BOLD;
  logfont.lfWeight = FW_BOLD;
  logfont.lfUnderline = 1;

  hfontLink = CreateFontIndirect(&logfont);
  hFontBold = CreateFontIndirect(&ftBold);

  SendDlgItemMessage(hDlg, IDC_MAILTO, WM_SETFONT, (WPARAM)hfontLink, 0);
  SendDlgItemMessage(hDlg, IDC_HOMEPAGE, WM_SETFONT, (WPARAM)hfontLink, 0);
  SendDlgItemMessage(hDlg, IDC_ABT_TCLOCK, WM_SETFONT, (WPARAM)hFontBold, 0);
  SendDlgItemMessage(hDlg, IDC_ABT_MAIL, WM_SETFONT, (WPARAM)hFontBold, 0);
  SendDlgItemMessage(hDlg, IDC_ABT_WEB, WM_SETFONT, (WPARAM)hFontBold, 0);

  if(hCurHand == NULL) hCurHand = LoadCursor(NULL, IDC_HAND);

  oldLabProc = (WNDPROC)(LONG_PTR)GetWindowLongPtr(GetDlgItem(hDlg, IDC_MAILTO), GWL_WNDPROC);
//==================================================================================
#if defined _M_IX86 //---------------+++--> IF Compiling This as a 32-bit Clock Use:
  SetWindowLongPtr(GetDlgItem(hDlg, IDC_MAILTO), GWL_WNDPROC, (LONG)(LRESULT)LabLinkProc);
  SetWindowLongPtr(GetDlgItem(hDlg, IDC_HOMEPAGE), GWL_WNDPROC, (LONG)(LRESULT)LabLinkProc);

//==================================================================================
#else //-------------------+++--> ELSE Assume: _M_X64 - IT's a 64-bit Clock and Use:
  SetWindowLongPtr(GetDlgItem(hDlg, IDC_MAILTO), GWL_WNDPROC, (LONG_PTR)(LRESULT)LabLinkProc);
  SetWindowLongPtr(GetDlgItem(hDlg, IDC_HOMEPAGE), GWL_WNDPROC, (LONG_PTR)(LRESULT)LabLinkProc);

#endif
//==================================================================================

  if(FileExists(hDlg)) CheckDlgButton(hDlg, IDC_STARTUP, TRUE);
}
/*--------------------------------------------------
  "Apply" button ----------------- IS NOT USED HERE!
--------------------------------------------------*/
void OnApply(HWND hDlg) {
  if(IsDlgButtonChecked(hDlg, IDC_STARTUP)) OnStartup(hDlg);
  else RemoveStartup(hDlg);
}
/*--------------------------------------------------
 -- IF User Clicks eMail - Fire up their Mail Client
--------------------------------------------------*/
void OnLinkClicked(HWND hDlg, UINT id) {
	char str[1024], *p;
	BOOL bOutlook = FALSE;
	
  if(id == IDC_MAILTO) {
	 GetRegStr(HKEY_CLASSES_ROOT, "mailto\\shell\\open\\command", "", str, 1024, "");
	 p = str;
	 while(*p) {
	   if(_strnicmp(p, "MSIMN.EXE", 9) == 0) {
		   bOutlook = TRUE; break;
	   }
	 p++;
	 }
		
	 strcpy(str, "mailto:");
	 if(bOutlook) {
		strcat(str, "Stoic Joker <");
		GetDlgItemText(hDlg, id, str + strlen(str), 80);
		strcat(str, ">?subject=About ");
		strcat(str, CONF_START);
	 }else GetDlgItemText(hDlg, id, str + strlen(str), 80);
  }
  else GetDlgItemText(hDlg, id, str, 80);
  ShellExecute(hDlg, NULL, str, NULL, "", SW_SHOW);
}
 //================================================================================================
//-------{ Give me a Hand...(Icon) }------+++--> Change Curser to Hand When Mousing Over Web Links:
LRESULT CALLBACK LabLinkProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) { //----+++-->
  switch(message) {
	case WM_SETCURSOR:
	  SetCursor(hCurHand);
	  return TRUE;
  }
 return CallWindowProc(oldLabProc, hwnd, message, wParam, lParam);
}
 //================================================================================================
//---------------------------+++--> Verify Existance of Launch T-Clock on Windows Startup ShortCut:
BOOL FileExists(HWND hDlg) { //-------------------------------------------------------------+++-->
	LPITEMIDLIST pidl;
	char dstpath[MAX_PATH], path[MAX_PATH], path2[MAX_PATH];
	char *lpStr1, *lpStr2;
	int	retval;	

  if(SHGetSpecialFolderLocation(hDlg, CSIDL_STARTUP, &pidl) == NOERROR 
						&& SHGetPathFromIDList(pidl, dstpath) == TRUE);
  else return FALSE;

  strcpy(path, dstpath);
  strcat(path, "\\");
  strcat(path, CONF_START);
  strcat(path, ".lnk");
  lpStr1 = path;
  retval = PathFileExists(lpStr1);
  if(retval == 1) return TRUE;
  else{
	strcpy(path2, dstpath);
	strcat(path2, "\\");
	strcat(path2, CONF_START);
    strcat(path2, ".lnk");
	lpStr2 = path2;
	retval = PathFileExists(lpStr2);
	if(retval == 1) return TRUE;
  }
 return FALSE;
}
 //================================================================================================
//----------------------------------------+++--> Remove Launch T-Clock on Windows Startup ShortCut:
void RemoveStartup(HWND hDlg) { //----------------------------------------------------------+++-->
	LPITEMIDLIST pidl;
	char dstpath[MAX_PATH], path[MAX_PATH], path2[MAX_PATH];
	char *lpStr1, *lpStr2;
	int	retval;	

  if(!FileExists(hDlg)) return;

  if(SHGetSpecialFolderLocation(hDlg, CSIDL_STARTUP, &pidl) == NOERROR && SHGetPathFromIDList(pidl, dstpath) == TRUE);
  else return;

  if(MyMessageBox(hDlg, "Remove Shortcut From the Startup Folder.\nAre You Sure?", CONF_START, MB_YESNO, MB_ICONQUESTION) != IDYES) return;

  strcpy(path, dstpath);
  strcat(path, "\\");
  strcat(path, CONF_START);
  strcat(path, ".lnk");
  lpStr1 = path;
  retval = DeleteFile(lpStr1);
  if(retval == 1) return;
  else{
	strcpy(path2, dstpath);
	strcat(path2, "\\");
	strcat(path2, CONF_START);
    strcat(path2, ".lnk");
	lpStr2 = path2;
	retval = DeleteFile(lpStr2);
	if(retval == 1) return;
  }
 return;
}
 //======================================
//--+++-->
void OnStartup(HWND hDlg) {
	LPITEMIDLIST pidl;
	char dstpath[MAX_PATH], myexe[MAX_PATH];

  if(FileExists(hDlg)) return;

  if(SHGetSpecialFolderLocation(hDlg, CSIDL_STARTUP, &pidl) == NOERROR && SHGetPathFromIDList(pidl, dstpath) == TRUE);
  else return;
	
  if(MyMessageBox(hDlg, "Add Shortcut To the Startup Folder.\nAre You Sure?", CONF_START, MB_YESNO, MB_ICONQUESTION) != IDYES) return;

  GetModuleFileName(GetModuleHandle(NULL), myexe, MAX_PATH);
  CreateLink(myexe, dstpath, CONF_START);
}
 //==========================
//--+++--> Create Launch T-Clock on Windows Startup ShortCut:
BOOL CreateLink(LPCSTR fname, LPCSTR dstpath, LPCSTR name) {
	HRESULT hres;
	IShellLink* psl;
	
  CoInitialize(NULL);
	
  hres = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLink, &psl); 
  if(SUCCEEDED(hres)) {
	 IPersistFile* ppf;
	 char path[MAX_PATH];
		
	 psl->lpVtbl->SetPath(psl, fname);
	 psl->lpVtbl->SetDescription(psl, name);
	 strcpy(path, fname);
	 del_title(path);
	 psl->lpVtbl->SetWorkingDirectory(psl, path);
		
	 hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf);
		
	 if(SUCCEEDED(hres)) {
		WORD wsz[MAX_PATH]; 
		char lnkfile[MAX_PATH];
		strcpy(lnkfile, dstpath);
		add_title(lnkfile, (char*)name);
		strcat(lnkfile, ".lnk");
			
		MultiByteToWideChar(CP_ACP, 0, lnkfile, -1, wsz, MAX_PATH);
			
		hres = ppf->lpVtbl->Save(ppf, wsz, TRUE);
		ppf->lpVtbl->Release(ppf);
	 }
	 psl->lpVtbl->Release(psl);
  }
  CoUninitialize();
	
  if(SUCCEEDED(hres)) return TRUE;
  else return FALSE;
}