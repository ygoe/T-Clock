  //============================================================================
 //  PageQuicky.c  -  Stoic Joker 2006  ========================================
//==============================================================================
#include "tclock.h"
//------------------------------------------------------------------------------
/* Modified by Stoic Joker: Sunday, 03/14/2010 @ 10:48:18AM */

void AddListBoxRows(HWND);
static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
void EditQuickyMenuItem(char *, char *, char *, char *, int);

#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0)
 //================================================================================================
//----------------------------------------+++--> Dialog Procedure for Quicky Menus Dialog Messages:
BOOL CALLBACK PageQuickyProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) { //----+++-->
	char szText[TNY_BUFF] = {0};
	LVCOLUMN lvCol; 
	HWND hList;

  hList = FindWindowEx(hDlg, NULL, WC_LISTVIEW, NULL);
  switch(message) {
	int iCol = 0;

	case WM_INITDIALOG:
	  OnInit(hDlg);				// IF We Give IT a Window Caption ... Is IT Easier to Find??!?
//==================================================================================
	  hList = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD|WS_VSCROLL|LVS_REPORT|
								LVS_SINGLESEL, 17, 117, 430, 191, hDlg, NULL, 0, 0);
	  ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

	  		 lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;		 // Load the Column Headers.
					for(iCol = IDS_LIST_TASKNUMBER; iCol <= IDS_LIST_TASKSWITCHES; iCol++) {
						lvCol.iSubItem = iCol;								   // From the String Table
						lvCol.pszText = szText;							      // Into the Temporary Buffer.
						if(iCol == IDS_LIST_TASKNUMBER) {
						   lvCol.cx = 0;			   // Set Column Width in Pixels
						   lvCol.fmt = LVCFMT_CENTER; // center-aligned column
						}
						else if(iCol == IDS_LIST_TASKNAME) {
						   lvCol.cx = 100;		  // Set Column Width in Pixels
						lvCol.fmt = LVCFMT_LEFT; // left-aligned column
						}
						else if(iCol == IDS_LIST_TASKTARGET) {
						   lvCol.cx = 220;		  // Set Column Width in Pixels
						lvCol.fmt = LVCFMT_LEFT; // left-aligned column
						}else if(iCol == IDS_LIST_TASKSWITCHES) {
						   lvCol.cx = 110;		  // Set Column Width in Pixels
						lvCol.fmt = LVCFMT_LEFT; // left-aligned column
						}
						LoadString(0, iCol, szText, sizeof(szText));    // <-- String Loads Here.
						ListView_InsertColumn(hList, iCol, &lvCol); // <- Now It's a Column Header
					}
	  AddListBoxRows(hList);
	  ShowWindow(hList, SW_SHOW);
//==================================================================================
	  return TRUE;

	case WM_COMMAND: {
	  WORD id, code;
	  id = LOWORD(wParam);
	  code = HIWORD(wParam);
	  if((IDC_QMEN_EXITWIN <= id && id <= IDC_QMEN_DISPLAY) &&
		  ((code == BST_CHECKED) || (code == BST_UNCHECKED))) {
			  SendPSChanged(hDlg);
	  }
	  if(id == IDC_QMEM_REFRESH) {
		  AddListBoxRows(hList);
	  }
	  return TRUE;
	}

	case WM_NOTIFY: {
	  if(((NMHDR *)lParam)->code == PSN_APPLY) {
		  OnApply(hDlg);
		return TRUE;
	  } 
//-------------------------------------------------------------------------------------------------- 
	  if(((LPNMHDR)lParam)->code == NM_DBLCLK) {
			int iSel;
		  if((iSel = (int)SendMessage(hList, LVM_GETNEXTITEM, -1, LVNI_FOCUSED)) != -1) {
				  char TaskSwitches[LRG_BUFF] = {0};
				  char TaskTarget[LRG_BUFF] = {0};
				  char TaskName[TNY_BUFF] = {0};
				  char TaskNum[TNY_BUFF] = {0};  

			  ListView_GetItemText(hList, iSel, 0, TaskNum, TNY_BUFF);
			  ListView_GetItemText(hList, iSel, 1, TaskName, TNY_BUFF);
			  ListView_GetItemText(hList, iSel, 2, TaskTarget, LRG_BUFF);
			  ListView_GetItemText(hList, iSel, 3, TaskSwitches, LRG_BUFF);
			  EditQuickyMenuItem(TaskNum, TaskName, TaskTarget, TaskSwitches, iSel);
		  }
//--------------------------------------------------------------------------------------------------
	  }
	  return TRUE;
	}

	case WM_DESTROY:
	  DestroyWindow(hDlg);
	  break;
  }
 return FALSE;
}
/*--------------------------------------------------
---------- Initialize Quicky Menu Options Dialog Box
--------------------------------------------------*/
static void OnInit(HWND hDlg) { /*---------------*/
  CheckDlgButton(hDlg, IDC_QMEN_AUDIO, GetMyRegLong("QuickyMenu", "AudioProperties", TRUE));
  CheckDlgButton(hDlg, IDC_QMEN_DISPLAY, GetMyRegLong("QuickyMenu", "DisplayProperties", TRUE));
  CheckDlgButton(hDlg, IDC_QMEN_EXITWIN, GetMyRegLong("QuickyMenu", "ExitWindows", TRUE));
  CheckDlgButton(hDlg, IDC_QMEN_LAUNCH, GetMyRegLong("QuickyMenu", "QuickyMenu", TRUE));
  CheckDlgButton(hDlg, IDC_QMEN_NET, GetMyRegLong("QuickyMenu", "NetworkDrives", TRUE));

}
/*--------------------------------------------------
--------------------- When "Apply" Button is Clicked
--------------------------------------------------*/
void OnApply(HWND hDlg) { /*---------------------*/
  SetMyRegLong("QuickyMenu", "DisplayProperties", IsDlgButtonChecked(hDlg, IDC_QMEN_DISPLAY));
  SetMyRegLong("QuickyMenu", "AudioProperties",   IsDlgButtonChecked(hDlg, IDC_QMEN_AUDIO));
  SetMyRegLong("QuickyMenu", "NetworkDrives",     IsDlgButtonChecked(hDlg, IDC_QMEN_NET));
  SetMyRegLong("QuickyMenu", "ExitWindows",       IsDlgButtonChecked(hDlg, IDC_QMEN_EXITWIN));
  SetMyRegLong("QuickyMenu", "QuickyMenu",        IsDlgButtonChecked(hDlg, IDC_QMEN_LAUNCH));
}
 //================================================================================================
//------------------------------+++--> Populate ListView Control With Currently Configured Options:
void AddListBoxRows(HWND hList) { //--------------------------------------------------------+++-->
	LVITEM lvItem;
	int row;

  ListView_DeleteAllItems(hList); // Clear ListView Control (Refresh Function)

  for(row=0; row <= 11; row++) {
		char task[TNY_BUFF]={0};
		int col;

	  wsprintf(task, "Item %d", 12-row);
		 lvItem.mask = LVIF_TEXT;
		 lvItem.iItem = 0;
		 lvItem.iSubItem = 0;
		 lvItem.pszText = task;
	    ListView_InsertItem(hList, &lvItem); // FIRST Insert A New Row
	    for(col = 1; col <= 3; col++) { //-> THEN Populate Its COLUMNS
				char szEntry[TNY_BUFF]={0};
				char szValue[LRG_BUFF]={0};

			 lvItem.iSubItem = col;
			 switch(col) {
				 case 1:
					 wsprintf(szEntry, "MenuItem-%d-Text", 11-row);
					 GetMyRegStr("QuickyMenu\\MenuItems", szEntry, szValue, LRG_BUFF, "");
					 lvItem.pszText = szValue;
				   break;

				 case 2:
					 wsprintf(szEntry, "MenuItem-%d-Target", 11-row);
					 GetMyRegStr("QuickyMenu\\MenuItems", szEntry, szValue, LRG_BUFF, "");
					 lvItem.pszText = szValue;
				   break;

				 case 3:
					 wsprintf(szEntry, "MenuItem-%d-Switches", 11-row);
					 GetMyRegStr("QuickyMenu\\MenuItems", szEntry, szValue, LRG_BUFF, "");
					 lvItem.pszText = szValue;
				   break;
			 }
			 ListView_SetItem(hList, &lvItem);
		}
  }
}
 //================================================================================================
//--+++-->
void EditQuickyMenuItem(char *TskNum, char *TskName, char *TskTarget, char *TskSwitches, int iNdx) {
	HWND hNexTab;
	int i;

  //---------------+++--> First Set "Menu Item Details" as the Active Tab, Then...
  SendMessage(PropSheet_GetTabControl(g_hwndSheet), TCM_SETCURFOCUS, 6, 0);
  hNexTab = PropSheet_GetCurrentPageHwnd(g_hwndSheet); // Get Handle of Active Tab

  for(i = IDC_MID_MENUTEXT; i <= IDB_LIST_BROWSE; i++) {
      EnableDlgItem(hNexTab, i, TRUE);
  }

  SetDlgItemInt(hNexTab,  IDC_MID_INDEX, iNdx, FALSE);
  SetDlgItemText(hNexTab, IDC_MID_SWITCHES, TskSwitches);
  SetDlgItemText(hNexTab, IDC_MID_TARGET, TskTarget);
  SetDlgItemText(hNexTab, IDC_MID_MENUTEXT, TskName);
  SetDlgItemText(hNexTab, IDC_MID_TASKNUM, TskNum);
}