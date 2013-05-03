   /*---------------------------------------------------
  // font.c : create a font of the clock -> KAZUBON 1999
 //--------------------------------------------------*/
// Modified by Stoic Joker: Wednesday, 03/24/2010 @ 10:13:25pm
#include "tcdll.h"

struct {
	int cp;
	BYTE charset;
} codepage_charset[] = {
	{ 932,  SHIFTJIS_CHARSET },
	{ 936,  GB2312_CHARSET },
	{ 949,  HANGEUL_CHARSET },
	{ 950,  CHINESEBIG5_CHARSET },
	{ 1250, EASTEUROPE_CHARSET },
	{ 1251, RUSSIAN_CHARSET },
	{ 1252, ANSI_CHARSET },
	{ 1253, GREEK_CHARSET },
	{ 1254, TURKISH_CHARSET },
	{ 1257, BALTIC_CHARSET },
	{ 0, 0}
};

 //================================================================================================
//----------------------+++--> CallBack Function for EnumFontFamiliesEx, to Find a Designated Font:
BOOL CALLBACK EnumFontFamExProc(ENUMLOGFONTEX* pelf, //-------------------------------------+++-->
								NEWTEXTMETRICEX* lpntm, int FontType, LPARAM fontname) {
  if(strcmp((LPSTR)fontname, pelf->elfLogFont.lfFaceName) == 0) return FALSE;
  return TRUE;
}
 //================================================================================================
//--------------------------------------------------------------+++--> Create a Font For the Clock:
HFONT CreateMyFont(char* fontname, int fontsize, LONG weight, LONG italic, int angle) { //--+++-->
	LOGFONT lf;	POINT pt;	HDC hdc;	WORD langid;
	char s[11];	int cp, i;	BYTE charset; int FontQty;
	
  memset(&lf, 0, sizeof(LOGFONT));
  langid = (WORD)GetMyRegLong("Format", "Locale", (int)GetUserDefaultLangID());
  FontQty = GetMyRegLong("Clock", "FontQuality", CLEARTYPE_QUALITY);
  cp = CP_ACP;

  if(GetLocaleInfo(langid, LOCALE_IDEFAULTANSICODEPAGE, s, 10) > 0) {
	 char *p;
	 p = s;
	 cp = 0;
	 while('0' <= *p && *p <= '9') cp = cp * 10 + *p++ - '0';
	 if(!IsValidCodePage(cp)) cp = CP_ACP;
  }
	
  charset = 0;
  for(i = 0; codepage_charset[i].cp; i++) {
	  if(cp == codepage_charset[i].cp) {
		 charset = codepage_charset[i].charset; break;
	  }
  }
	
  hdc = GetDC(NULL);
	
	// find a font named "fontname"
  if(charset == 0) charset = GetTextCharset(hdc);
  lf.lfCharSet = charset;
  if(EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)fontname, 0)) {
	 lf.lfCharSet = OEM_CHARSET;
	 if(EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)fontname, 0)) {
		lf.lfCharSet = ANSI_CHARSET;
		EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)fontname, 0);
	 }
  }
	
	pt.x = 0;
	pt.y = MulDiv(fontsize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	DPtoLP(hdc, &pt, 1);
	lf.lfHeight = -pt.y;
	
	ReleaseDC(NULL, hdc);
	
	lf.lfWidth = lf.lfEscapement = lf.lfOrientation = 0;
	lf.lfWeight = weight;
	lf.lfItalic = (BYTE)italic;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	if(angle >0) lf.lfEscapement = angle;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;

	lf.lfQuality = FontQty; // This Just HAD To be Adjustable.

	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	strcpy(lf.lfFaceName, fontname);
	
 return CreateFontIndirect(&lf);
}
/*--+++--> ClearType: <--+++--<<<<< NEED TO TRY THIS!!!!!!!!!!!!
ClearType support depends on your system supporting it. 
Please check with your vendor if your device supports ClearType.

Creating a font with the ClearType property is as simple as 
specifying 5 as the lfQuality member of the LOGFONT structure.
*/
//	== ALL Currently Available Font Quality Options ==
//	lf.lfQuality = CLEARTYPE_NATURAL_QUALITY;	// = 6
//	lf.lfQuality = CLEARTYPE_QUALITY;			// = 5
//	lf.lfQuality = ANTIALIASED_QUALITY;  		// = 4
//	lf.lfQuality = NONANTIALIASED_QUALITY;		// = 3
//	lf.lfQuality = PROOF_QUALITY;				// = 2
//	lf.lfQuality = DRAFT_QUALITY;				// = 1
//	lf.lfQuality = DEFAULT_QUALITY;				// = 0
/*
Okay... So for "Historical Acuracy" (so i don't forget) 
I'm going to include the Whole (font.lfQuality) Story...

The original Kazubon builds used: lf.lfQuality = DEFAULT_QUALITY;
But weren't really crisp on Vista with themes/transparency/etc.

So, I switched to: lf.lfQuality = CLEARTYPE_NATURAL_QUALITY;
Whiched proved to be fine unless using (Win7) Aero with a light
colored clock font and wallpaper (which made it fuzzy as hell).

Now after a bit of futzing around, I discovered that apparently:
	lf.lfQuality = ANTIALIASED_QUALITY; (= 4) Was best for Win7
	lf.lfQuality = CLEARTYPE_QUALITY;  (= 5) Was best for WinXP
	lf.lfQuality = DEFAULT_QUALITY;  (= 0) Was best for Win2000

Which is why I decided (was forced) to make Font Quality Adjustable.
*/