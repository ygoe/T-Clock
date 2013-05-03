/*-------------------------------------------
  soundselect.c - KAZUBON 1997-2001
  select a sound file with "Open" dialog
---------------------------------------------*/
// Last Modified by Stoic Joker: Sunday, 03/13/2011 @ 11:54:05am
#include "tclock.h"

static BOOL bPlaying = FALSE;

void GetMMFileExts(char* dst) {
	char s[1024], *sp, *dp;
	
  GetProfileString("mci extensions", NULL, "", s, 1024);

  sp = s; dp = dst;
  while(*sp) {
		if(dp != dst) *dp++ = ';';
		*dp++ = '*'; *dp++ = '.';
		while(*sp) *dp++ = *sp++;
		sp++;
  }
  *dp = 0;
}

/*------------------------------------------------------------------
---------------------------------- open dialog to browse sound files
------------------------------------------------------------------*/
BOOL BrowseSoundFile(HWND hDlg, const char *deffile, char *fname) {
	char filter[1024], mmfileexts[1024];
	char ftitle[MAX_PATH], initdir[MAX_PATH];
	
	OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(ofn)); // Initialize OPENFILENAME
	ofn.lStructSize = sizeof(ofn);

  filter[0] = filter[1] = 0;
  str0cat(filter, MyString(IDS_MMFILE));
  GetMMFileExts(mmfileexts);
  str0cat(filter, mmfileexts);
  str0cat(filter, MyString(IDS_ALLFILE));
  str0cat(filter, "*.*");
	
  if(deffile[0] == 0 || IsMMFile(deffile)) ofn.nFilterIndex = 1;
  else ofn.nFilterIndex = 2;
	
  strcpy(initdir, g_mydir);
  if(deffile[0]) {
	 WIN32_FIND_DATA fd;
	 HANDLE hfind;
	 hfind = FindFirstFile(deffile, &fd);
	 if(hfind != INVALID_HANDLE_VALUE) {
		FindClose(hfind);
		strcpy(initdir, deffile);
		del_title(initdir);
	 }
  }
	
  fname[0] = 0;

	ofn.hwndOwner = hDlg;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = fname;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = ftitle;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = initdir;
	ofn.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_FILEMUSTEXIST;

 return GetOpenFileName(&ofn);
}

BOOL IsMMFile(const char* fname) {
	char s[1024], *sp;
	
  if(lstrcmpi(fname, "cdaudio") == 0) return TRUE;
  GetProfileString("mci extensions", NULL, "", s, 1024);

  sp = s;
  while(*sp) {
		if(ext_cmp(fname, sp) == 0) return TRUE;
		if(ext_cmp(fname, "pcb") == 0) return TRUE;
		while(*sp) sp++; sp++;
  }
 return FALSE;
}

void OnInitDialog(HWND hDlg) {
	HWND hwndStatic;
	RECT rc1, rc2;
	POINT pt;
	int dx;
	
  SendDlgItemMessage(hDlg, IDC_TESTSOUND, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconPlay);
  EnableDlgItem(hDlg, IDC_TESTSOUND, FALSE);
	
  bPlaying = FALSE;

	// find "File Name:" Label
  hwndStatic = GetDlgItem(GetParent(hDlg), 0x442);
  if(hwndStatic == NULL) return;
  GetWindowRect(hwndStatic, &rc1);

	// move "Test:" Label
  GetWindowRect(GetDlgItem(hDlg, IDC_LABTESTSOUND), &rc2);
  dx = rc1.left - rc2.left;
  pt.x = rc2.left + dx; pt.y = rc2.top;
  ScreenToClient(hDlg, &pt);
  SetWindowPos(GetDlgItem(hDlg, IDC_LABTESTSOUND), NULL, pt.x, pt.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

	// move play button
  GetWindowRect(GetDlgItem(hDlg, IDC_TESTSOUND), &rc2);
  pt.x = rc2.left + dx; pt.y = rc2.top;
  ScreenToClient(hDlg, &pt);
  SetWindowPos(GetDlgItem(hDlg, IDC_TESTSOUND), NULL, pt.x, pt.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
}

void OnFileNameChanged(HWND hDlg) {
	char fname[MAX_PATH];
	WIN32_FIND_DATA fd;
	BOOL b = FALSE;

  HANDLE hfind = INVALID_HANDLE_VALUE;
	
  if(CommDlg_OpenSave_GetFilePath(GetParent(hDlg), fname, sizeof(fname)) <= sizeof(fname)) {
	 hfind = FindFirstFile(fname, &fd);
	 if(hfind != INVALID_HANDLE_VALUE) {
	    if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) b = TRUE;
		FindClose(hfind);
	 }
  }
 EnableDlgItem(hDlg, IDC_TESTSOUND, b);
}

void OnTestSound(HWND hDlg) {
	char fname[MAX_PATH];
	
  if(CommDlg_OpenSave_GetFilePath(GetParent(hDlg), fname, sizeof(fname)) <= sizeof(fname)) {
	 if((HICON)SendDlgItemMessage(hDlg, IDC_TESTSOUND, BM_GETIMAGE, IMAGE_ICON, 0) == g_hIconPlay) {
		if(PlayFile(hDlg, fname, 0)) {
		   SendDlgItemMessage(hDlg, IDC_TESTSOUND, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconStop);
		   InvalidateRect(GetDlgItem(hDlg, IDC_TESTSOUND), NULL, FALSE);
		   bPlaying = TRUE;
		}
	 }else{
	    StopFile(); bPlaying = FALSE;
	 }
  }
}