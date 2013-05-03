/*-------------------------------------------------------------
  sntp.c
  KAZUBON 1998-1999
               Special thanks to Tomoaki Nakashima
---------------------------------------------------------------*/
// Modified by Stoic Joker: Monday, 04/12/2010 @ 7:42:04pm
#include "tclock.h"
#include <winsock.h>
#include "resource.h"

//===============================================================
struct NTP_Packet { // NTP (Network Time Protocol) Request Packet
	int Control_Word;
	int root_delay;
	int root_dispersion;
	int reference_identifier;
	__int64 reference_timestamp;
	__int64 originate_timestamp;
	__int64 receive_timestamp;
	int transmit_timestamp_seconds;
	int transmit_timestamp_fractions;
};
//===========================================================
typedef struct { // Close Socket on Request TimeOut Structure
	BOOL bComplete;
	DWORD dwSent;
	SOCKET soc;
}KILLSOC, *LPKILLSOC;

extern TCHOTKEY *tchk;

HWND hLogView;
static BOOL bSaveLog;
static BOOL bMessage;
static BOOL bGUI = FALSE;
static int nTimeout = 1000;
static DWORD dwTickCountOnSend = 0;

static void OnInit(HWND);
BOOL GetSetTimePermissions(void);
unsigned __stdcall KillSocketProc(void *);
static void OnSanshoAlarm(HWND hDlg, WORD id);
extern WNDPROC OldEditClassProc; // Default Procedure for Edit Controls
BOOL CALLBACK SNTPConfigProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY SubClassEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
 //================================================================================================
//---------------------------//----------------------------+++--> Save Request Results in SNTP.log:
void Log(const char* msg) { //--------------------------------------------------------------+++-->
	char s[GEN_BUFF] = {0};
	SYSTEMTIME st;
	
  GetLocalTime(&st);
  wsprintf(s, "%d/%02d/%02d %02d:%02d:%02d ", st.wYear,
	  st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
  strcat(s, msg);
	
  // save to file
  if(bSaveLog) {
	  char fname[MAX_PATH];
	  HFILE hf;
		
	  strcpy(fname, g_mydir);
	  add_title(fname, "SNTP.log");
	  hf = _lopen(fname, OF_WRITE);
	  if(hf == HFILE_ERROR)
		  hf = _lcreat(fname, 0);
	  if(hf == HFILE_ERROR) return;
	  _llseek(hf, 0, 2);
	  _lwrite(hf, s, lstrlen(s));
	  _lwrite(hf, "\x0d\x0a", 2);
	  _lclose(hf);
  }

  if(bGUI) { // IF Configure NTP Server is Open, Display Results in Sync History.
	  LVITEM lvItem; //-----+++--> Even if Activity is Not Saved to the Log File.
	  lvItem.mask = LVIF_TEXT;
	  lvItem.iSubItem = 0; // Hold These at Zero So the File Loads Backwards
	  lvItem.iItem = 0; //-----+++--> Which Puts the Most Recent Info on Top.
	  lvItem.pszText = s;
	  ListView_InsertItem(hLogView, &lvItem);
  }

  if(bMessage) {
	  MessageBox(0, s, "T-Clock Time Sync", MB_OK);
  }
}
 //================================================================================================
//-------------------------------------------------------+++--> Set System Time With Received Data:
void SynchronizeSystemTime(DWORD seconds, DWORD fractions) { //-----------------------------+++-->
	char szWave[MAX_BUFF] = {0};
	SYSTEMTIME st, st_dif;
	FILETIME ft, ftold;
	char s[GEN_BUFF];
	DWORD sr_time;
	DWORDLONG dif;
	BOOL b;
	
	// timeout ?
	sr_time = GetTickCount() - dwTickCountOnSend;
	if(sr_time >= (DWORD)nTimeout)
	{
		wsprintf(s, "timeout (%04d)", sr_time);
		Log(s); return;
	}
	
	// current time
	GetSystemTimeAsFileTime(&ftold);
	
	// NTP data -> FILETIME
	*(DWORDLONG*)&ft =
		// seconds from 1900/01/01 ¨ 100 nano-seconds from 1601/01/01
		M32x32to64(seconds, 10000000) + 94354848000000000i64;
/*	
	// difference
	if(nMinuteDif > 0)
		*(DWORDLONG*)&ft += M32x32to64(nMinuteDif * 60, 10000000);
	else if(nMinuteDif < 0)
		*(DWORDLONG*)&ft -= M32x32to64(-nMinuteDif * 60, 10000000);
*/	
	// set system time
	b = FileTimeToSystemTime(&ft, &st);
	if(b)
	{
		// 200 pico-seconds -> milli-seconds
		st.wMilliseconds = (WORD)(fractions / 5000000);
		b = SetSystemTime(&st);
	}
	if(!b) {
		Log("failed to set time"); return;
	}
/*	
	GetLocalTime(&lt);
	nLastDay = lt.wDay;
	SetMyRegLong("SNTP", "LastDay", nLastDay);
*/	
	SystemTimeToFileTime(&st, &ft);
	// delayed or advanced
	b = (*(DWORDLONG*)&ft > *(DWORDLONG*)&ftold);
	// get difference
	if(b) dif = *(DWORDLONG*)&ft - *(DWORDLONG*)&ftold;
	else  dif = *(DWORDLONG*)&ftold - *(DWORDLONG*)&ft;
	FileTimeToSystemTime((FILETIME*)&dif, &st_dif);
	
	// save log
	strcpy(s, "synchronized ");
	if(st_dif.wYear == 1601 && st_dif.wMonth == 1 &&
		st_dif.wDay == 1 && st_dif.wHour == 0)
	{
		strcat(s, b?"+":"-");
		wsprintf(s + strlen(s), "%02d:%02d.%03d ",
			st_dif.wMinute, st_dif.wSecond, st_dif.wMilliseconds);
	}
	GetMyRegStr("SNTP", "Sound", szWave, MAX_BUFF, "");
	PlayFile(g_hWnd, szWave, 0);
	if(strlen(szWave)) // IF There IS a Sound File Selected
		

	wsprintf(s + strlen(s), "(%04d)", sr_time);
	Log(s);
	Sleep(3000); //----+++--> Give it Time to Play.
}
 //================================================================================================
//--------------------------------------------------+++--> Close Socket, and the WinSOCK Interface:
void SocketClose(SOCKET Sntp, const char *msgbuf) { //--------------------------------------+++-->
	
  if(Sntp != -1) {
	  closesocket(Sntp);
  }

  WSACleanup();
  if(msgbuf) Log(msgbuf);
}
/*---------------------------------------------------
	get server name and port number from string
		buf: "ntp.xxxxx.ac.jp:123"
---------------------------------------------------*/
int GetServerPort(const char *buf, char *server)
{
	char *p;
	int port = 123;

	if(strcmp(buf, "") == 0) return -1;
	strcpy(server, buf);
	
	for(p = server; *p != ':' && *p != '\0'; p++);
	if(*p == ':')
	{
		*p = 0; p++; port = 0;
		while(*p)
		{
			if('0' <= *p && *p <= '9')
				port = port * 10 + *p - '0';
			else
			{
				port = -1; break;
			}
			p++;
		}
	}
	return port;
}
 //================================================================================================
//-------------------------+++--> Looks Like the Server Has SomeThing to Say - Find Out What it is:
void ReceiveSNTPReply(SOCKET Sntp) { //-----------------------------------------------------+++-->
	struct sockaddr_in FromAddr;
	struct NTP_Packet NTP_Recv;
	char szErr[MIN_BUFF] = {0};
	int sockaddr_Size;
	int nRet;
	
  //-----------------------------------------------+++--> Receive UpDated Time Data: 
  sockaddr_Size = sizeof(FromAddr);
  nRet = recvfrom(Sntp, (char *)&NTP_Recv, sizeof(NTP_Recv), 0,
								(struct sockaddr *)&FromAddr, &sockaddr_Size);
  if(nRet == SOCKET_ERROR) {
	  wsprintf(szErr, "Receive SOCKET ERROR: %d", WSAGetLastError());
	  MessageBox(0, szErr, "Time Sync Failed:", MB_OK|MB_ICONERROR);
	  SocketClose(Sntp, szErr);
	return;
  }
  
  //-------------------------------+++--> (Message Received) Now Set the System Time!
  SynchronizeSystemTime(ntohl(NTP_Recv.transmit_timestamp_seconds),
	  ntohl(NTP_Recv.transmit_timestamp_fractions));
  SocketClose(Sntp, 0);
}
 //================================================================================================
//-------------------------------------------------------------------+++--> Send SNTP Sync Request:
int SNTPSend(SOCKET Sntp, LPSOCKADDR_IN lpstToAddr) {
	struct NTP_Packet NTP_Send;
	char szErr[MIN_BUFF] = {0};
	int nRet;
	
	// init a packet
  memset(&NTP_Send, 0, sizeof(struct NTP_Packet));
  NTP_Send.Control_Word = htonl(0x0B000000);
	
	// send a packet
  nRet = sendto(Sntp, (const char *)&NTP_Send, sizeof(NTP_Send), 
					0, (LPSOCKADDR)lpstToAddr, sizeof(SOCKADDR_IN));

  if(nRet == SOCKET_ERROR) { // Tell Us if "We" Failed!
	  wsprintf(szErr, "Send SOCKET ERROR: %d", WSAGetLastError());
	  MessageBox(0, szErr, "Time Sync Failed:", MB_OK|MB_ICONERROR);
	  SocketClose(Sntp, szErr);
  }
	// save tickcount
  dwTickCountOnSend = GetTickCount();
 return(nRet);
}

 //================================================================================================
//-------------------------------------------------------------+++--> Open Socket for SNTP Session:
SOCKET OpenTimeSocket(char *szRegString) {
	struct sockaddr_in serveraddr;
	char szServer[256];
	char szErr[MIN_BUFF] = {0};
	int nRet, port;
	SOCKET Sntp;

	LPHOSTENT lpHost;

  port = GetServerPort(szRegString, szServer);
  lpHost = gethostbyname(szServer); // Verify that Host Name Exists

  if(lpHost == NULL) {
	  wsprintf(szErr, "Host not found: %s", szRegString);
	  MessageBox(0, szErr, "Time Sync Failed:", MB_OK|MB_ICONERROR);
	  return FALSE;
  }

	// make a socket
  Sntp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(Sntp == SOCKET_ERROR) {
	  wsprintf(szErr, "Create SOCKET Failed! ERROR: %d", WSAGetLastError());
	  MessageBox(0, szErr, "Time Sync Failed:", MB_OK|MB_ICONERROR);
	  SocketClose(Sntp, szErr);
	  return FALSE;
  }
	
	// Setup destination socket address
	serveraddr.sin_addr.s_addr = *((u_long FAR *) (lpHost->h_addr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);

	// send data
  nRet = SNTPSend(Sntp, &serveraddr);
  if(nRet == SOCKET_ERROR) {
	  wsprintf(szErr, "SendTo ERROR: %d", WSAGetLastError());
	  MessageBox(0, szErr, "Send Time Request Failed:", MB_OK|MB_ICONERROR);
	  SocketClose(Sntp, szErr);
	  return FALSE;
  }
 return Sntp;
}
   //====================//========================================================================
  // Required for SNTP/UDP Socket Operation TimeOut Thread Only ===================================
#include <process.h>   //--+++-->				 <--+++--<<<<< SNTP Code Starts Here >>>>>--+++-->
//====================//===========================================================================
void SyncTimeNow() { //============================================================================
    WORD wVersionRequested = MAKEWORD(2,2);
	WSADATA wsaData; // Okay...Now We Want WinSock v2.2
	char szServer[MIN_BUFF] = {0};
	char szErr[GEN_BUFF] = {0};
	DWORD dwTickCount = 0;
	SOCKET Sntp = 0;
	KILLSOC ks;
    int nRet;

  if(!bGUI) {
	    bSaveLog = GetMyRegLongEx("SNTP", "SaveLog", 0);
		bMessage = GetMyRegLongEx("SNTP", "MessageBox", 0);
  }
  GetMyRegStrEx("SNTP", "Server", szServer, MIN_BUFF, "");
  if(!strlen(szServer)) { //-------+++--> If SNTP Server is NOT Configured:
	  wsprintf(szErr, "No SNTP Server Specified!");
	  MessageBox(0, szErr, "Time Sync Failed:", MB_OK|MB_ICONERROR);
	  NetTimeConfigDialog();
	  return;
  }

  nRet = WSAStartup(wVersionRequested, &wsaData);
  if(nRet) { //-----------------------------------------+++--> If WinSock Startup Fails...
	  wsprintf(szErr, "Error initializing WinSock");
	  MessageBox(0, szErr, "Time Sync Failed:", MB_OK|MB_ICONERROR);
	 return;
  }

  if(wsaData.wVersion != wVersionRequested) { //-+++-> Check WinSOCKET's Version:
	  wsprintf(szErr, "WinSock version not supported");
	  MessageBox(0, szErr, "Time Sync Failed:", MB_OK|MB_ICONERROR);
	 return;
  }
  
  Sntp = OpenTimeSocket(szServer);
  if(!Sntp) return;
  
  dwTickCount = GetTickCount();
	
	ks.soc = Sntp;
	ks.dwSent = dwTickCount;
   //--------------------------------+++--> Start CutOff Timer:
  _beginthreadex(NULL, 0, KillSocketProc,(void *)&ks, 0, NULL);

  ReceiveSNTPReply(Sntp);
  ks.bComplete = TRUE;
}
 //================================================================================================
//---------------------------------------+++--> SNTP/UDP Socket Operation TimeOut Thread Procedure:
unsigned __stdcall KillSocketProc(void * param) { //----------------------------------------+++-->
	LPKILLSOC ks;
	DWORD dwNow = 0;
	DWORD dwKillTime;
	ks = (LPKILLSOC)param;

  dwKillTime = ks->dwSent + 1000;
  while(dwNow < dwKillTime) {
	  Sleep(10);
	  dwNow = GetTickCount();
	  if(ks->bComplete) dwNow = dwKillTime;
  }
  if(!ks->bComplete) {
	  closesocket(ks->soc);
	  WSACleanup();
  }
  _endthread();
 return 0;
}
//=================================================================================================
/* SetSystemTime(&st); Requires SE_SYSTEMTIME_NAME Priviledge: Are you running as a limited user?

If so, you'll need to use the group policy editor (Run gpedit.msc as Administrator) to assign the
rights here:

(Computer Configuration\Windows Settings\Security Settings\Local Policies\User Rights Assignments
and add your username to "Change the system time"). I don't know of any specific registry key. */

// Note: Changing Privileges in a Token - Only Works IF the Account Has Access to that Priviledge
//---------------------------------------------------+++--> e.g. IS An Administrator.
 //================================================================================================
//--------------------------------//---------------------+++--> Open the SNTP Configuration Dialog:
void NetTimeConfigDialog(void) { //---------------------------------------------------------+++-->
  bGUI = TRUE;    //-----------------------------------+++--> Start in Gooy Mode.
  DialogBox(0, MAKEINTRESOURCE(IDD_SNTPCONFIG), g_hWnd, (DLGPROC)SNTPConfigProc);
  bGUI = FALSE; //-------------------------------+++--> End of/Disable Gooy Mode.
}
 //================================================================================================
//--------------------------//--+++--> Save Network Time Server Configuration Settings to Registry:
void OkaySave(HWND hDlg) { //---------------------------------------------------------------+++-->
	char szServer[MIN_BUFF] = {0};
	char szSound[MAX_PATH] = {0};
	char entry[TNY_BUFF] = {0};
	char subkey[] = "SNTP";
	int i, index, count;

  SetMyRegLong(subkey, "SaveLog", IsDlgButtonChecked(hDlg, IDCBX_SNTPLOG));
  SetMyRegLong(subkey, "MessageBox", IsDlgButtonChecked(hDlg, IDCBX_SNTPMESSAGE));

  GetDlgItemText(hDlg, IDCE_SYNCSOUND, szSound, MAX_PATH);
  SetMyRegStr(subkey, "Sound", szSound);

  if(tchk[0].bValid) { // Synchronize System Clock With Remote Time Server
	  RegisterHotKey(g_hWnd, HOT_TSYNC, tchk[0].fsMod, tchk[0].vk);
  }else{						// I'm Calling This One Mouser's HotKey...
	  tchk[0].vk = 0;		   // I'm Not Explaining It You Either Already
	  tchk[0].fsMod = 0;	  // Understand Why or You're Not Going Too...
	  strcpy(tchk[0].szText, "None");
	  UnregisterHotKey(g_hWnd, HOT_TSYNC);
  }    
  SetMyRegLong("HotKeys\\HK5", "bValid", tchk[0].bValid);
  SetMyRegLong("HotKeys\\HK5", "fsMod",  tchk[0].fsMod);
  SetMyRegStr("HotKeys\\HK5", "szText", tchk[0].szText);
  SetMyRegLong("HotKeys\\HK5", "vk",  tchk[0].vk);




	GetDlgItemText(hDlg, IDCBX_NTPSERVER, szServer, MIN_BUFF);
	SetMyRegStr(subkey, "Server", szServer);
	
	if(szServer[0]) {
		index = (int)CBFindStringExact(hDlg, IDCBX_NTPSERVER, szServer);
		if(index != LB_ERR)
			CBDeleteString(hDlg, IDCBX_NTPSERVER, index);
		CBInsertString(hDlg, IDCBX_NTPSERVER, 0, szServer);
		CBSetCurSel(hDlg, IDCBX_NTPSERVER, 0);
	}
	count = (int)CBGetCount(hDlg, IDCBX_NTPSERVER);
	for(i = 0; i < count; i++) {
		CBGetLBText(hDlg, IDCBX_NTPSERVER, i, szServer);
		wsprintf(entry, "Server%d", i+1);
		SetMyRegStr(subkey, entry, szServer);
	}
	SetMyRegLong(subkey, "ServerNum", count);



}
 //================================================================================================
//------------------------------------------------------+++--> SNTP Configuration Dialog Procedure:
BOOL CALLBACK SNTPConfigProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) { 
  switch(msg)  {
	  case WM_INITDIALOG:
		  SetMyDialgPos(hDlg);
		  OnInit(hDlg);
		return TRUE;

	  case WM_COMMAND:
		  switch(LOWORD(wParam))  {
			  case IDCB_SYNCNOW:
				  SyncTimeNow();
			    return TRUE;

			  case IDCB_SYNCSOUNDBROWSE:
				  OnSanshoAlarm(hDlg, IDCE_SYNCSOUND);
				return TRUE;

			  case IDOK:
				  OkaySave(hDlg);
                 //--+++--+++--//--+++--> Fall Through IS Intentional. 
			  case IDCANCEL:
				  if(tchk) {
					  free(tchk);   // Free, and...? (Crash Unless You Include the Next Line)
					  tchk = NULL; //<--+++--> Thank You Don Beusee for reminding me to do this.
				  }
				  EndDialog(hDlg, /*wParam*/TRUE);
//				  return TRUE;
		  }
  } 
 return FALSE; 
}
   //=================//=====//>>>>>---------------------------------------------+++-->
  #include <stdio.h> //-----//--+++--> Required Here For Log FILE Open Functions Only.
 //=================//=====//======================================================================
//------------------------//---------------------------+++--> To-Do List for Dialog Initialization:
void OnInit(HWND hDlg) { //-----------------------------------------------------------------+++-->
	char server[MIN_BUFF] = {0};
	char szFile[MAX_PATH] = {0};
	char subkey[] = "SNTP";
	FILE *stReport;
	LVCOLUMN lvCol;
	LVITEM lvItem;
	int i, count;
										 // Get the List of Configured Time Servers:
//======================================//==========================================
	GetMyRegStr(subkey, "Server", server, 80, "");
	
	count = GetMyRegLong(subkey, "ServerNum", 0);
	for(i = 1; i <= count; i++) {
		char s[MAX_BUFF], entry[TNY_BUFF];

		wsprintf(entry, "Server%d", i);
		GetMyRegStr(subkey, entry, s, 80, "");
		if(s[0]) CBAddString(hDlg, IDCBX_NTPSERVER, (LPARAM)s);
	}

	if(server[0]) {
		i = (int)CBFindStringExact(hDlg, IDCBX_NTPSERVER, server);
		if(i == LB_ERR) {
			CBInsertString(hDlg, IDCBX_NTPSERVER, 0, (LPARAM)server);
			i = 0;
		}
		CBSetCurSel(hDlg, IDCBX_NTPSERVER, i);
	}

	if(!g_hIconDel) {
		g_hIconDel = LoadImage((HANDLE)GetModuleHandle(NULL),
								MAKEINTRESOURCE(IDI_DEL),
								IMAGE_ICON, 16, 16, 
								LR_DEFAULTCOLOR);
	}
	SendDlgItemMessage(hDlg, IDCB_DELSERVER, BM_SETIMAGE,
									 IMAGE_ICON, (LPARAM)g_hIconDel);

														 // Get the Sync Sound File:
//======================================================//==========================
  GetMyRegStr(subkey, "Sound", szFile, MAX_BUFF, "");
  SetDlgItemText(hDlg, IDCE_SYNCSOUND, szFile);
													// Get the Confirmation Options:
//=================================================//===============================
  bSaveLog = GetMyRegLongEx(subkey, "SaveLog", 0);
  CheckDlgButton(hDlg, IDCBX_SNTPLOG, bSaveLog);
  bMessage = GetMyRegLongEx(subkey, "MessageBox", 0);
  CheckDlgButton(hDlg, IDCBX_SNTPMESSAGE, bMessage);

							// Load & Display the Configured Synchronization HotKey:
//=========================//=======================================================

  tchk = malloc(sizeof(TCHOTKEY));
  tchk[0].bValid = GetMyRegLongEx("HotKeys\\HK5", "bValid", 0);
  GetMyRegStrEx("HotKeys\\HK5", "szText", tchk[0].szText, TNY_BUFF, "None");
  tchk[0].fsMod = GetMyRegLongEx("HotKeys\\HK5", "fsMod", 0);
  tchk[0].vk = GetMyRegLongEx("HotKeys\\HK5", "vk", 0);

  SetDlgItemText(hDlg, IDCE_SYNCHOTKEY, tchk[0].szText);
  
		// Subclass the Edit Controls
  OldEditClassProc  = (WNDPROC)(LONG_PTR)GetWindowLongPtr(GetDlgItem(hDlg, IDCE_SYNCHOTKEY), GWL_WNDPROC);
//==================================================================================
#if defined _M_IX86 //---------------+++--> IF Compiling This as a 32-bit Clock Use:
  SetWindowLongPtr(GetDlgItem(hDlg, IDCE_SYNCHOTKEY), GWL_WNDPROC, (LONG)(LRESULT)SubClassEditProc);

//==================================================================================
#else //-------------------+++--> ELSE Assume: _M_X64 - IT's a 64-bit Clock and Use:
  SetWindowLongPtr(GetDlgItem(hDlg, IDCE_SYNCHOTKEY), GWL_WNDPROC, (LONG_PTR)(LRESULT)SubClassEditProc);

#endif
								  //-+> Create & Show the Log File ListView Control:
//===============================//=================================================
  hLogView = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD|WS_VSCROLL|LVS_REPORT|
							 LVS_SINGLESEL, 19, 188, 428, 89, hDlg, NULL, 0, 0);
  ListView_SetExtendedListViewStyle(hLogView, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

  ShowWindow(hLogView, SW_SHOW);							 // Populate Its Column:
//==========================================================//======================
  lvCol.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
  lvCol.cx = 280; //-+-// Set Column Width in Pixels
  lvCol.iSubItem = 0; // This is the First & Only Column
  lvCol.pszText = "Synchronization History"; // Header Text
  ListView_InsertColumn(hLogView, 0, &lvCol);

         // Test For: SE_SYSTEMTIME_NAME Priviledge Before Enabling Sync Now Button:
//======//==========================================================================
  EnableWindow(GetDlgItem(hDlg, IDCB_SYNCNOW), GetSetTimePermissions());

										  // Load the Time Synchronization Log File:
//=======================================//=========================================
  strcpy(szFile, g_mydir);
  add_title(szFile, "SNTP.log");

  if(fopen_s(&stReport, szFile, "r") > 0) {
	  MessageBox(0, "Open SNTP.log Failed!", "ERROR:", MB_OK|MB_ICONERROR);
	  return;
  }else{
	  char *iGot;
	  lvItem.mask = LVIF_TEXT;
	  lvItem.iSubItem = 0; // Hold These at Zero So the File Loads Backwards
	  lvItem.iItem = 0; //-----+++--> Which Puts the Most Recent Info on Top.

	  for( ; ; ) { // (for) Ever Basically.
			char szLine[MAX_BUFF] = {0};
		  iGot = fgets(szLine, MAX_BUFF, stReport);
		  if(iGot) {
			  szLine[strcspn(szLine, "\n")] = '\0'; // Remove the Newline Character
			  lvItem.pszText = szLine;
			  ListView_InsertItem(hLogView, &lvItem);
		  }else{
			  fclose(stReport);
			  return; // Remember: Any Code Placed Below Here Will FAIL
		  } //--+++--//--------------+++--> ^^^ See Above ^^^
	  }
  }
}
 //================================================================================================
//----------------------------------------//---------------+++--> Browse for Sync Event Sound File:
void OnSanshoAlarm(HWND hDlg, WORD id) { //-------------------------------------------------+++-->
	char deffile[MAX_PATH], fname[MAX_PATH];
	
  GetDlgItemText(hDlg, id, deffile, MAX_PATH);
  if(!BrowseSoundFile(hDlg, deffile, fname)) // soundselect.c
	  return;
	
  SetDlgItemText(hDlg, id, fname);
  PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}