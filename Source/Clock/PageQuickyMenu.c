  //============================================================================
 //  PageQuickyMenu.c  -  Stoic Joker 2006  ====================================
//==============================================================================
#include "tclock.h"
//------------------------------------------------------------------------------
/* Modified by Stoic Joker: Sunday, 03/14/2010 @ 10:48:18AM */

static void OnInit(HWND hDlg);
void DisableTabControls(HWND);
static void OnApply(HWND hDlg);
void BrowseForTargetFile(HWND);
void DeleteMenuItem(HWND, char *);
void SaveNewMenuOptions(HWND, char *, char *, char *, char *);

#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0)
 //================================================================================================
//-----------------------------------+++--> Dialog Procedure for Menu Item Details Dialog Messages:
BOOL CALLBACK PageQuickyMenuProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) { //-++-->
  switch(message) {
	case WM_INITDIALOG:
	  OnInit(hDlg);
	  return TRUE;

	case WM_COMMAND: {
	  WORD id, code;
	  id = LOWORD(wParam);
	  code = HIWORD(wParam);

	  if(id == IDC_MID_SAVE) {
			char szmText[TNY_BUFF]={0};
			char szIndex[TNY_BUFF]={0};
			char szmTarget[LRG_BUFF]={0};
			char szmSwitches[LRG_BUFF]={0};
	     GetDlgItemText(hDlg, IDC_MID_MENUTEXT, szmText,     MIN_BUFF);
	     GetDlgItemText(hDlg, IDC_MID_INDEX,    szIndex,     TNY_BUFF);
	     GetDlgItemText(hDlg, IDC_MID_TARGET,   szmTarget,   LRG_BUFF);
	     GetDlgItemText(hDlg, IDC_MID_SWITCHES, szmSwitches, LRG_BUFF);
		 SaveNewMenuOptions(hDlg, szmText, szmTarget, szmSwitches, szIndex);
		 SendMessage(PropSheet_IndexToHwnd(g_hwndSheet, 5), WM_COMMAND, IDC_QMEM_REFRESH, 0);
	  }

	  if(id == IDC_MID_CANCEL) {
	     DisableTabControls(hDlg);
		 SendMessage(PropSheet_GetTabControl(g_hwndSheet), TCM_SETCURFOCUS, 5, 0);
	  }

	  if(id == IDB_MID_DELETE) {
			char szIndex[TNY_BUFF]={0};
	     GetDlgItemText(hDlg, IDC_MID_INDEX, szIndex, TNY_BUFF);
		 DeleteMenuItem(hDlg, szIndex);
		 SendMessage(PropSheet_IndexToHwnd(g_hwndSheet, 5), WM_COMMAND, IDC_QMEM_REFRESH, 0);
	  }

	  if(id == IDB_LIST_BROWSE){
	     BrowseForTargetFile(hDlg);
	  }return TRUE;
	}

	case WM_NOTIFY:
	  if(((NMHDR *)lParam)->code == PSN_APPLY) {
		  OnApply(hDlg);
	  } return TRUE;

	case WM_DESTROY:
	  DestroyWindow(hDlg);
	  break;
  }
 return FALSE;
}
/*--------------------------------------------------
  initialize --------------------- IS NOT USED HERE!
--------------------------------------------------*/
static void OnInit(HWND hDlg) {
}
/*--------------------------------------------------
  "Apply" button ----------------- IS NOT USED HERE!
--------------------------------------------------*/
void OnApply(HWND hDlg) {
}
/*-------------------------------------------------
Disable & Clear ALL Menu Item Details Tab Controls.
-------------------------------------------------*/
void DisableTabControls(HWND hDlg) {
	int iShow = 0;

  SendMessage(PropSheet_GetTabControl(g_hwndSheet), TCM_SETCURFOCUS, 6, 0);

  for(iShow = IDC_MID_MENUTEXT; iShow <= IDB_LIST_BROWSE; iShow++) {
      EnableDlgItem(hDlg, iShow, FALSE);
  }

  SetDlgItemText(hDlg, IDC_MID_SWITCHES, "");
  SetDlgItemText(hDlg, IDC_MID_TARGET,   "");
  SetDlgItemText(hDlg, IDC_MID_MENUTEXT, "");
  SetDlgItemText(hDlg, IDC_MID_TASKNUM,  "");
  SetDlgItemText(hDlg, IDC_MID_INDEX,    "");
}
/*-------------------------------------------------------------
Save the New Menu Item Options - From the Menu Item Details Tab
-------------------------------------------------------------*/
void SaveNewMenuOptions(HWND hDlg, char *szmText, char *szmTarget, char *szmSwitches, char *szIndex) {
	char szItem[LRG_BUFF] = {0};

  if((strlen(szmText)) && (strlen(szmTarget))) {
	  wsprintf(szItem, "MenuItem-%s", szIndex);
	  SetMyRegLong("QuickyMenu\\MenuItems", szItem, TRUE);

	  wsprintf(szItem, "MenuItem-%s-Text", szIndex);
	  SetMyRegStr("QuickyMenu\\MenuItems", szItem, szmText);

	  wsprintf(szItem, "MenuItem-%s-Target", szIndex);
	  SetMyRegStr("QuickyMenu\\MenuItems", szItem, szmTarget);

	  wsprintf(szItem, "MenuItem-%s-Switches", szIndex);
	  SetMyRegStr("QuickyMenu\\MenuItems", szItem, szmSwitches);

	  DisableTabControls(hDlg);
	  SendMessage(PropSheet_GetTabControl(g_hwndSheet), TCM_SETCURFOCUS, 5, 0);
  }else{
	  wsprintf(szItem, "MenuItem-%s", szIndex);
	  SetMyRegLong("QuickyMenu\\MenuItems", szItem, FALSE);
	  if(!strlen(szmText))
		 MessageBox(0, "Menu Item Text Can Not be Left Blank!", "ERROR: Missing Information!", MB_OK|MB_ICONERROR);
	  if(!strlen(szmTarget))
		 MessageBox(0, "Target File Can Not be Left Blank!", "ERROR: Missing Information!", MB_OK|MB_ICONERROR);
  }
}
/*---------------------------------------------------------------------------
-------------- DELETE a Menu Item - From the ListBox Control and the Registry
---------------------------------------------------------------------------*/
void DeleteMenuItem(HWND hDlg, char *szIndex) {
	char szItem[TNY_BUFF] = {0};

  wsprintf(szItem, "MenuItem-%s", szIndex);
  DelMyReg("QuickyMenu\\MenuItems", szItem);

  wsprintf(szItem, "MenuItem-%s-Text", szIndex);
  DelMyReg("QuickyMenu\\MenuItems", szItem);

  wsprintf(szItem, "MenuItem-%s-Target", szIndex);
  DelMyReg("QuickyMenu\\MenuItems", szItem);

  wsprintf(szItem, "MenuItem-%s-Switches", szIndex);
  DelMyReg("QuickyMenu\\MenuItems", szItem);

  DisableTabControls(hDlg);
  SendMessage(PropSheet_GetTabControl(g_hwndSheet), TCM_SETCURFOCUS, 5, 0);
}
 //================================================================================================
//-------------------------------------//------+++--> Browse to the Quicky Menu Item's Target File:
void BrowseForTargetFile(HWND hBft) { //----------------------------------------------------+++-->
	char szFile[MAX_PATH];
	char *Filters = "Program Files (*.exe)\0*.exe\0" "All Files (*.*)\0*.*\0";
	OPENFILENAME ofn;

  ZeroMemory(szFile, MAX_PATH);
  ZeroMemory(&ofn, sizeof(ofn)); // Initialize OPENFILENAME

	ofn.lStructSize  = sizeof(ofn);
	ofn.hwndOwner    = hBft;
	ofn.hInstance	 = NULL;
	ofn.lpstrFilter  = Filters;
	ofn.lpstrFile	 = szFile;
	ofn.nMaxFile     = MAX_PATH;
	ofn.lpstrInitialDir = g_mydir;
	ofn.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_FILEMUSTEXIST;

  if(GetOpenFileName(&ofn)) {
     SetDlgItemText(hBft, IDC_MID_TARGET, szFile);
  }
}