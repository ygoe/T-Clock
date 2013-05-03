  //================================================================================
 //--+++--> BounceWind.c  -  Stoic Joker 2010  =====================================
//================= Last Modified by Stoic Joker: Wednesday, 12/22/2010 @ 11:29:24pm
#include "tclock.h" //---------------{ Stoic Joker 2006-2010 }---------------+++-->
//-----------------//-------------------------------------------------------+++-->
#include <time.h> // Required for Randomizing the Bounce Height -----------+++-->
char szCaption[TNY_BUFF];  // Alarm Name
char szMessage[MAX_BUFF]; // Window Text
char szSettings[TNY_BUFF]; // Hop Settings

static RECT rcMsg;
static HWND hWndBoing;
static void OnOK(HWND hDlg);
static void OnInit(HWND hDlg);
static void BounceWindow(HWND hWndBoing);
BOOL CALLBACK AlarmMsgProc(HWND, UINT, WPARAM, LPARAM);
int iDelta, iSpeed, iPause, iShort, iSkew, iPaws, iBounce;
static int iScreenW, iScreenH, iBallX, iBallY, iDeltaX, iDeltaY;
void ParseSettings(char*, int*, int*, int*, int*, int*, BOOL*);
static void CenterDoggie(HWND hWnd);
BOOL bRandHop;

#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0)
#define ID_DOGGIE 2419

 //----------------------------------------------------------------//---------+++-->
//---------------+++--> This is the Window That Will Bounce Wildly About the Screen:
void JackRusselWindow(HWND hJRWnd, char *szCapt, char *szText) { //----------+++-->
	MessageBox(hJRWnd, szText, szCapt, MB_OK);
}
 //-----------------------------//--------------------------------------------+++-->
//-----------------------------//----+++--> Open Window Bounce Control Options Here:
void OnMsgWindOpt(HWND hDlg){ //---------------------------------------------+++-->
  GetDlgItemText(hDlg, IDC_COMBOALARM, szCaption, TNY_BUFF);
  GetDlgItemText(hDlg, IDC_JRMSG_TEXT, szMessage, MAX_BUFF);
  GetDlgItemText(hDlg, IDC_JR_SETTINGS, szSettings, TNY_BUFF);
  if(DialogBox(0, MAKEINTRESOURCE(IDD_ALARMMSG), hDlg, (DLGPROC)AlarmMsgProc) == IDOK) {
  SetDlgItemText(hDlg, IDC_JR_SETTINGS, szSettings);
  SetDlgItemText(hDlg, IDC_JRMSG_TEXT, szMessage);
  SendPSChanged(hDlg);
  }
}
 //---------------------------------------------------------------------------+++-->
// -------------------------------------------+++--> Alarm Message Dialog Procedure:
BOOL CALLBACK AlarmMsgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {

  switch(message) {
	case WM_INITDIALOG:
	  OnInit(hDlg);
	  return TRUE;

	case WM_COMMAND: {
	  WORD id, code;
	  id = LOWORD(wParam); code = HIWORD(wParam);
	  switch(id) {
		case IDC_JRMSG_TEST:
			iBounce = GetDlgItemInt(hDlg, IDC_JRMSG_BOUN, NULL, TRUE); // Get Users Attention!!!
			iSkew = GetDlgItemInt(hDlg, IDC_JRMSG_SKEW, NULL, TRUE); // Hop vs. Ricochet deviation
			iPaws = GetDlgItemInt(hDlg, IDC_JRMSG_PAWS, NULL, TRUE); // Sit! & Wait for User Input

			iSpeed = GetDlgItemInt(hDlg, IDC_JRMSG_SPEED, NULL, TRUE); // SetTimer->Milliseconds
			iDelta = GetDlgItemInt(hDlg, IDC_JRMSG_DELTA, NULL, TRUE); // Move X Pixels per Loop
			GetDlgItemText(hDlg, IDC_JRMSG_CAPT, szCaption, TNY_BUFF);
			GetDlgItemText(hDlg, IDC_JRMSG_TEXT, szMessage, MAX_BUFF);
			bRandHop = IsDlgButtonChecked(hDlg, IDC_JRMSG_RAND);
			ReleaseTheHound(hDlg, FALSE);
			break;


		
		case IDOK: // Fall Through IS Intentional.
			OnOK(hDlg);
		
		case IDCANCEL: 
			EndDialog(hDlg, id);
			break;
	  }
	 return TRUE;
	}
  }
 return FALSE;
}
 //------------------------//-------------------------------------------------+++-->
// -----------------------//----------------------------+++--> Initialize Dialoggie:
void OnInit(HWND hDlg) { //--------------------------------------------------+++-->
/*	bRandHop = TRUE; // Enable Randomized Hop Height
	iBounce = 3;   // Get Users Attention!!!
	iDelta = 42;  // Move X Pixels per Loop
	iSpeed = 90; // SetTimer->Milliseconds
	iSkew = 4; // Hop vs. Ricochet deviation
	iPaws = 3; // Sit! & Wait for User Input
*/
  ParseSettings(szSettings, &iBounce, &iSkew, &iPaws, &iSpeed, &iDelta, &bRandHop);

  SendDlgItemMessage(hDlg, IDC_SPIN_JRSPD, UDM_SETRANGE, 0, MAKELONG(100, 0));
  SendDlgItemMessage(hDlg, IDC_SPIN_JRSPD, UDM_SETPOS, 0, iSpeed);  // Movement Timer Rate
  SendDlgItemMessage(hDlg, IDC_SPIN_JRDLT, UDM_SETRANGE, 0, MAKELONG(42, iSkew));
  SendDlgItemMessage(hDlg, IDC_SPIN_JRDLT, UDM_SETPOS, 0, iDelta); // Pixel Distance Moved

  SendDlgItemMessage(hDlg, IDC_SPIN_JRSKW, UDM_SETRANGE, 0, MAKELONG(6, 1));
  SendDlgItemMessage(hDlg, IDC_SPIN_JRSKW, UDM_SETPOS, 0, iSkew); //----+++--> Skew Factor
  SendDlgItemMessage(hDlg, IDC_SPIN_JRPAW, UDM_SETRANGE, 0, MAKELONG(10, 0));
  SendDlgItemMessage(hDlg, IDC_SPIN_JRPAW, UDM_SETPOS, 0, iPaws); //-+++--> Paws/Sit! Time
  SendDlgItemMessage(hDlg, IDC_SPIN_JRBNC, UDM_SETRANGE, 0, MAKELONG(10, 0));
  SendDlgItemMessage(hDlg, IDC_SPIN_JRBNC, UDM_SETPOS, 0, iBounce); //--+++--> Bounce Time
  SetDlgItemText(hDlg, IDC_JRMSG_CAPT, szCaption);
  SetDlgItemText(hDlg, IDC_JRMSG_TEXT, szMessage);
  CheckDlgButton(hDlg, IDC_JRMSG_RAND, bRandHop);
  iScreenW = GetSystemMetrics(SM_CXSCREEN);
  iScreenH = GetSystemMetrics(SM_CYSCREEN);

  iShort = 100;
}
 //-----------------------------//--------------------------------------------+++-->
//------------------+++--> ...And Here be the Part What Goes Boing, Boing, Boing!!!:
void BounceWindow(HWND hWnd){ //---------------------------------------------+++-->
	int iSizeW, iSizeH;

 iBallX += (iDeltaX / iSkew);
 iBallY += iDeltaY;
 iSizeW = rcMsg.right - rcMsg.left;
 iSizeH = rcMsg.bottom - rcMsg.top;

 if(iBallX < 0) {
   iBallX = 0;
   iDeltaX = iDelta;
 }
 else if((iBallX + iSizeW) > iScreenW) {
	 iBallX = iScreenW - iSizeW;
	 iDeltaX = -iDelta;
 }

 if(iBallY < iShort) {
   iBallY = iShort;
   iDeltaY = iDelta;
 }
 else if((iBallY + iSizeH) > iScreenH){
	 if(bRandHop) {
		 double dTmp;
		 srand((UINT)time(0));
		 dTmp = rand() % 42 + 0;
		 dTmp *= .01;
		 iShort = (int)(iScreenH * dTmp); // Make Shorter Random Height Hops
	 }else{
		 iShort = 0;
	 }
	 iBallY = iScreenH - iSizeH;
	 iDeltaY = -iDelta;
 }
 SetWindowPos(hWnd, HWND_TOP, iBallX, iBallY, 0, 0, SWP_NOSIZE);
}
 //------------------------------//-------------------------------------------+++-->
//------------+++--> Snap Dialoggie Window to Center Screen and Wait for User Input:
void CenterDoggie(HWND hWnd) { //--------------------------------------------+++-->
	int iSizeW, iSizeH;
	int x, y;

 iSizeW = rcMsg.right - rcMsg.left;
 iSizeH = rcMsg.bottom - rcMsg.top;
  x = (iScreenW / 2) - (iSizeW / 2);
  y = (iScreenH / 2) - (iSizeH / 2);
	SetWindowPos(hWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE);
}
 //----------------------//---------------------------------------------------+++-->
// ---------------------//------------------------------+++--> Initialize Dialoggie:
void OnOK(HWND hDlg) { //----------------------------------------------------+++-->
  iBounce = GetDlgItemInt(hDlg, IDC_JRMSG_BOUN, NULL, TRUE); // Get Users Attention!!!
  iSkew = GetDlgItemInt(hDlg, IDC_JRMSG_SKEW, NULL, TRUE); // Hop vs. Ricochet deviation
  iPaws = GetDlgItemInt(hDlg, IDC_JRMSG_PAWS, NULL, TRUE); // Sit! & Wait for User Input

  iSpeed = GetDlgItemInt(hDlg, IDC_JRMSG_SPEED, NULL, TRUE); // SetTimer->Milliseconds
  iDelta = GetDlgItemInt(hDlg, IDC_JRMSG_DELTA, NULL, TRUE); // Move X Pixels per Loop
//  GetDlgItemText(hDlg, IDC_JRMSG_CAPT, szCaption, TNY_BUFF);
  GetDlgItemText(hDlg, IDC_JRMSG_TEXT, szMessage, MAX_BUFF);
  bRandHop = IsDlgButtonChecked(hDlg, IDC_JRMSG_RAND);
  if(iSkew < 1) iSkew = 1; // Divide by Zero = Bad...

  wsprintf(szSettings, "%i,%i,%i,%i,%i,%i", iBounce, iSkew, iPaws, iSpeed, iDelta, bRandHop);								
}
 //--------------------------------------------------------//--------//-------+++-->
//----------------------------+++--> Parse the Dialoggie Settings Out of the String:
void ParseSettings(char *szString, int* iBnc, int *iSkw, //--------//--------+++-->
/*****************/int *iPws, int *iSpd, int *iDlt, BOOL *bRHp) { //--------+++-->
	char *szToken, *nxToken;
	char seps[] = ",";
	int i=0;

  if(!strlen(szString)) // Enforce the Defaults if String is Empty
	  strcpy(szString, "3,4,3,90,42,1");

  szToken = strtok_s(szString, seps, &nxToken);
  while(szToken != NULL) {
	  switch(i) {
		  case 0:
			  *iBnc = atoi(szToken);
			  break;
		  case 1:
			  *iSkw = atoi(szToken);
			  break;
		  case 2:
			  *iPws = atoi(szToken);
			  break;
		  case 3:
			  *iSpd = atoi(szToken);
			  break;
		  case 4:
			  *iDlt = atoi(szToken);
			  break;
		  case 5:
			  *bRHp = atoi(szToken);
			  break;
	  }
	  szToken = strtok_s(NULL, seps, &nxToken);
	  i++;
  }
}
 //---------------------------------------------------------------------------+++-->
//------------------------------------------+++--> Ricochet Doggie Window Procedure:
VOID CALLBACK DoggieProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) { //
  iPause += iSpeed;

  if(iPause >= (iBounce * 1000)) { // Check for Sit! Command.
	  if(iPause < ((iBounce * 1000) + 101)) CenterDoggie(hWndBoing);
	  iPause += iSpeed;
	  if(iPause >= ((iBounce * 1000)+(iPaws * 1000))) { // Give User X Seconds to Read
		  iPause = 0; //---------------------------+++--> and Respond to Alarm Message.
	  }
  }else{ // Bounce Wildly for X Seconds!
	  hWndBoing = FindWindow(NULL, szCaption);
	  GetWindowRect(hWndBoing, &rcMsg);
	  BounceWindow(hWndBoing);
  }
}
 //---------------------------------------------------------------------------+++-->
//-------------------------------------------+++--> The Alarm Just Let the Dogz Out:
void ReleaseTheHound(HWND hWnd, BOOL bLive) {

  iScreenW = GetSystemMetrics(SM_CXSCREEN);
  iScreenH = GetSystemMetrics(SM_CYSCREEN);
  iShort = 100;

  if(bLive) // If This is a Live Alarm & Not a Test, Parse the Settings Input String.
	  ParseSettings(szSettings, &iBounce, &iSkew, &iPaws, &iSpeed, &iDelta, &bRandHop);
  
  iDeltaX = iDeltaY = iDelta;
  iBallX = iBallY = 0;
  iPause = 0; // Hang Time Between Rompings...

  if(iSkew < 1) iSkew = 1; // Divide by Zero = Bad...
  
  if(iSpeed > 100){ // Timer Cannot be Set
	  iSpeed = 0;  // Any Faster Than Zero!
  }else{ // Convert High Number Into Short Time Slice.
	  iSpeed = 100 - iSpeed;
  } //-//-++-> Now Set the Converted Short Time Slice.

  SetTimer(hWnd, ID_DOGGIE, iSpeed, (TIMERPROC)DoggieProc);
  if(iSpeed < 10) iSpeed = 10; // Required to Ensure Paws.
  JackRusselWindow(hWnd, szCaption, szMessage);
  KillTimer(hWnd, ID_DOGGIE);
}