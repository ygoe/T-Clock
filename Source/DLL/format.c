     //---[s]--- For InternetTime 99/03/16@211 M.Takemura -----

   /*------------------------------------------------------------------------
  // format.c : to make a string to display in the clock -> KAZUBON 1997-1998
 //-----------------------------------------------------------------------*/
// Last Modified by Stoic Joker: Friday, 12/16/2011 @ 3:36:00pm
#include "tcdll.h"

int codepage = CP_ACP;
static char MonthShort[11], MonthLong[31];
static char DayOfWeekShort[11], DayOfWeekLong[31];
static char *DayOfWeekEng[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char *MonthEng[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static char AM[11], PM[11], SDate[5], STime[5];
static char EraStr[11];
static int AltYear;

extern BOOL bHour12, bHourZero;

 //================================================================================================
//---------------------------------//+++--> load Localized Strings for Month, Day, & AM/PM Symbols:
void InitFormat(SYSTEMTIME* lt) { //--------------------------------------------------------+++-->
	char s[80], *p;
	int i, ilang, ioptcal;
	
  ilang = GetMyRegLong(NULL, "Locale", (int)GetUserDefaultLangID());
  codepage = CP_ACP;

  if(GetLocaleInfo((WORD)ilang, LOCALE_IDEFAULTANSICODEPAGE, s, 10) > 0) {
	  p = s; codepage = 0;
	  while('0' <= *p && *p <= '9') codepage = codepage * 10 + *p++ - '0';
	  if(!IsValidCodePage(codepage)) codepage = CP_ACP;
  }
	
  i = lt->wDayOfWeek; i--; if(i < 0) i = 6;

  GetLocaleInfo((WORD)ilang, LOCALE_SABBREVDAYNAME1 + i, DayOfWeekShort, 10);
  GetLocaleInfo((WORD)ilang, LOCALE_SDAYNAME1 + i, DayOfWeekLong, 30);

  i = lt->wMonth; i--;
  GetLocaleInfo((WORD)ilang, LOCALE_SABBREVMONTHNAME1 + i, MonthShort, 10);
  GetLocaleInfo((WORD)ilang, LOCALE_SMONTHNAME1 + i, MonthLong, 30);
	
  GetLocaleInfo((WORD)ilang, LOCALE_S1159, AM, 10);
  GetMyRegStr("Format", "AMsymbol", s, 80, AM);
  if(s[0] == 0) strcpy(s, "AM");
  strcpy(AM, s);

  GetLocaleInfo((WORD)ilang, LOCALE_S2359, PM, 10);
  GetMyRegStr("Format", "PMsymbol", s, 80, PM);
  if(s[0] == 0) strcpy(s, "PM");
  strcpy(PM, s);
	
  GetLocaleInfo((WORD)ilang, LOCALE_SDATE, SDate, 4);
  GetLocaleInfo((WORD)ilang, LOCALE_STIME, STime, 4);
	
  EraStr[0] = 0;
  AltYear = -1;
	
  ioptcal = 0;
  if(GetLocaleInfo((WORD)ilang, LOCALE_IOPTIONALCALENDAR, s, 10)) {
	  ioptcal = 0;
	  p = s;
	  
	  while('0' <= *p && *p <= '9') ioptcal = ioptcal * 10 + *p++ - '0';
  }
  
  if(ioptcal < 3) ilang = LANG_USER_DEFAULT;

  if(GetDateFormat((WORD)ilang, DATE_USE_ALT_CALENDAR, lt, "gg", s, 12) != 0) strcpy(EraStr, s);
	
  if(GetDateFormat((WORD)ilang, DATE_USE_ALT_CALENDAR, lt, "yyyy", s, 6) != 0) {
	  if(s[0]) {
		  p = s;
		  AltYear = 0;
		  while('0' <= *p && *p <= '9') AltYear = AltYear * 10 + *p++ - '0';
	  }
  }
}
 //================================================================================================
//--+++-->
BOOL GetNumFormat(char **sp, char x, int *len, int *slen)
{
	char *p;
	int n, ns;

	p = *sp;
	n = 0;
	ns = 0;

	while (*p == '_')
	{
		ns++;
		p++;
	}
	if (*p != x) return FALSE;
	while (*p == x)
	{
		n++;
		p++;
	}

	*len = n+ns;
	*slen = ns;
	*sp = p;
	return TRUE;
}
 //================================================================================================
//--+++-->
void SetNumFormat(char **dp, int n, int len, int slen)
{
	char *p;
	int minlen,i;

	p = *dp;

	for (i=10,minlen=1; i<1000000000; i*=10,minlen++)
		if (n < i) break;

	while (minlen < len)
	{
		if (slen > 0) { *p++ = ' '; slen--; }
		else { *p++ = '0'; }
		len--;
	}

	for (i=minlen-1; i>=0; i--)
	{
		*(p+i) = (char)((n%10)+'0');
		n/=10;
	}
	p += minlen;

	*dp = p;
}
 //================================================================================================
//-------------+++--> Format T-Clock's OutPut String From Current Date, Time, & System Information:
void MakeFormat(char* s, SYSTEMTIME* pt, int beat100, char* fmt) { //-----------------------+++-->
	char *sp, *dp, *p;
	DWORD TickCount = 0;
	
	sp = fmt; dp = s;
	while(*sp)
	{
		if(*sp == '\"')
		{
			sp++;
			while(*sp != '\"' && *sp)
			{
				p = CharNext(sp);
				while(sp != p) *dp++ = *sp++;
			}
			if(*sp == '\"') sp++;
		}
		else if(*sp == '/')
		{
			p = SDate; while(*p) *dp++ = *p++;
			sp++;
		}
		else if(*sp == ':')
		{
			p = STime; while(*p) *dp++ = *p++;
			sp++;
		}
		
		// for testing
		else if(*sp == 'S' && *(sp + 1) == 'S' && *(sp + 2) == 'S')
		{
			SetNumFormat(&dp, (int)pt->wMilliseconds, 3, 0);
			sp += 3;
		}
		
		else if(*sp == 'y' && *(sp + 1) == 'y')
		{
			int len;
			len = 2;
			if (*(sp + 2) == 'y' && *(sp + 3) == 'y') len = 4;

			SetNumFormat(&dp, (len==2)?(int)pt->wYear%100:(int)pt->wYear, len, 0);
			sp += len;
		}
		else if(*sp == 'm')
		{
			if(*(sp + 1) == 'm' && *(sp + 2) == 'e')
			{
				*dp++ = MonthEng[pt->wMonth-1][0];
				*dp++ = MonthEng[pt->wMonth-1][1];
				*dp++ = MonthEng[pt->wMonth-1][2];
				sp += 3;
			}
			else if(*(sp + 1) == 'm' && *(sp + 2) == 'm')
			{
				if(*(sp + 3) == 'm')
				{
					p = MonthLong;
					while(*p) *dp++ = *p++;
					sp += 4;
				}
				else
				{
					p = MonthShort;
					while(*p) *dp++ = *p++;
					sp += 3;
				}
			}
			else
			{
				if(*(sp + 1) == 'm')
				{
					*dp++ = (char)((int)pt->wMonth / 10) + '0';
					sp += 2;
				}
				else
				{
					if(pt->wMonth > 9)
						*dp++ = (char)((int)pt->wMonth / 10) + '0';
					sp++;
				}
				*dp++ = (char)((int)pt->wMonth % 10) + '0';
			}
		}
		else if(*sp == 'a' && *(sp + 1) == 'a' && *(sp + 2) == 'a')
		{
			if(*(sp + 3) == 'a')
			{
				p = DayOfWeekLong;
				while(*p) *dp++ = *p++;
				sp += 4;
			}
			else
			{
				p = DayOfWeekShort;
				while(*p) *dp++ = *p++;
				sp += 3;
			}
		}
		else if(*sp == 'd')
		{
			if(*(sp + 1) == 'd' && *(sp + 2) == 'e')
			{
				p = DayOfWeekEng[pt->wDayOfWeek];
				while(*p) *dp++ = *p++;
				sp += 3;
			}
			else if(*(sp + 1) == 'd' && *(sp + 2) == 'd')
			{
				if(*(sp + 3) == 'd')
				{
					p = DayOfWeekLong;
					while(*p) *dp++ = *p++;
					sp += 4;
				}
				else
				{
					p = DayOfWeekShort;
					while(*p) *dp++ = *p++;
					sp += 3;
				}
			}
			else
			{
				if(*(sp + 1) == 'd')
				{
					*dp++ = (char)((int)pt->wDay / 10) + '0';
					sp += 2;
				}
				else
				{
					if(pt->wDay > 9)
						*dp++ = (char)((int)pt->wDay / 10) + '0';
					sp++;
				}
				*dp++ = (char)((int)pt->wDay % 10) + '0';
			}
		}
		else if(*sp == 'h')
		{
			int hour;
			hour = pt->wHour;
			if(bHour12)
			{
				if(hour > 12) hour -= 12;
				else if(hour == 0) hour = 12;
				if(hour == 12 && bHourZero) hour = 0;
			}
			if(*(sp + 1) == 'h')
			{
				*dp++ = (char)(hour / 10) + '0';
				sp += 2;
			}
			else
			{
              if(hour > 9) {
					*dp++ = (char)(hour / 10) + '0';
              }
				sp++;
			}
			*dp++ = (char)(hour % 10) + '0';
		}
        else if (*sp == 'w' )
		{
          char xs_diff[3];
          int xdiff;
          int hour;

          xs_diff[0] = (char)(*(sp+2));
          xs_diff[1] = (char)(*(sp+3));
          xs_diff[2] = (char)'\x0';
          xdiff = atoi( xs_diff );
          if ( *(sp+1) == '-' ) xdiff = -xdiff;
          hour = ( pt->wHour + xdiff )%24;
          if ( hour < 0 ) hour += 24;
          if ( bHour12 ) {
            if(hour > 12) hour -= 12;
            else if(hour == 0) hour = 12;
            if(hour == 12 && bHourZero) hour = 0;
          }
          *dp++ = (char)(hour / 10) + '0';
          *dp++ = (char)(hour % 10) + '0';
          sp += 4;
		}
		else if(*sp == 'n')
		{
			if(*(sp + 1) == 'n')
			{
				*dp++ = (char)((int)pt->wMinute / 10) + '0';
				sp += 2;
			}
			else
			{
				if(pt->wMinute > 9)
					*dp++ = (char)((int)pt->wMinute / 10) + '0';
				sp++;
			}
			*dp++ = (char)((int)pt->wMinute % 10) + '0';
		}
		else if(*sp == 's')
		{
			if(*(sp + 1) == 's')
			{
				*dp++ = (char)((int)pt->wSecond / 10) + '0';
				sp += 2;
			}
			else
			{
				if(pt->wSecond > 9)
					*dp++ = (char)((int)pt->wSecond / 10) + '0';
				sp++;
			}
			*dp++ = (char)((int)pt->wSecond % 10) + '0';
		}
		else if(*sp == 't' && *(sp + 1) == 't')
		{
			if(pt->wHour < 12) p = AM; else p = PM;
			while(*p) *dp++ = *p++;
			sp += 2;
		}
		else if(*sp == 'A' && *(sp + 1) == 'M')
		{
			if(*(sp + 2) == '/' &&
				*(sp + 3) == 'P' && *(sp + 4) == 'M')
			{
				if(pt->wHour < 12) *dp++ = 'A'; //--+++--> 2010 - Noon / MidNight Decided Here!
				else *dp++ = 'P';
				*dp++ = 'M'; sp += 5;
			}
			else if(*(sp + 2) == 'P' && *(sp + 3) == 'M')
			{
				if(pt->wHour < 12) p = AM; else p = PM;
				while(*p) *dp++ = *p++;
				sp += 4;
			}
			else *dp++ = *sp++;
		}
		else if(*sp == 'a' && *(sp + 1) == 'm' && *(sp + 2) == '/' &&
			*(sp + 3) == 'p' && *(sp + 4) == 'm')
		{
			if(pt->wHour < 12) *dp++ = 'a';
			else *dp++ = 'p';
			*dp++ = 'm'; sp += 5;
		}
		else if(*sp == '\\' && *(sp + 1) == 'n')
		{
			*dp++ = 0x0d; *dp++ = 0x0a;
			sp += 2;
		}
		// internet time
		else if (*sp == '@' && *(sp + 1) == '@' && *(sp + 2) == '@')
		{
			*dp++ = '@';
			*dp++ = beat100 / 10000 + '0';
			*dp++ = (beat100 % 10000) / 1000 + '0';
			*dp++ = (beat100 % 1000) / 100 + '0';
			sp += 3;
			if(*sp == '.' && *(sp + 1) == '@')
			{
				*dp++ = '.';
				*dp++ = (beat100 % 100) / 10 + '0';
				sp += 2;
			}
		}
		// alternate calendar
		else if(*sp == 'Y' && AltYear > -1)
		{
			int n = 1;
			while(*sp == 'Y') { n *= 10; sp++; }
			if(n < AltYear)
			{
				n = 1; while(n < AltYear) n *= 10;
			}
			while(1)
			{
				*dp++ = (AltYear % n) / (n/10) + '0';
				if(n == 10) break;
				n /= 10;
			}
		}
		else if(*sp == 'g')
		{
			char *p2;
			p = EraStr;
			while(*p && *sp == 'g')
			{
				p2 = CharNextExA((WORD)codepage, p, 0);
				while(p != p2) *dp++ = *p++;
				sp++;
			}
			while(*sp == 'g') sp++;
		}

		else if(*sp == 'L' && _strncmp(sp, "LDATE", 5) == 0)
		{
			char s[80], *p;
			GetDateFormat(MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
				DATE_LONGDATE, pt, NULL, s, 80);
			p = s;
			while(*p) *dp++ = *p++;
			sp += 5;
		}

		else if(*sp == 'D' && _strncmp(sp, "DATE", 4) == 0)
		{
			char s[80], *p;
			GetDateFormat(MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
				DATE_SHORTDATE, pt, NULL, s, 80);
			p = s;
			while(*p) *dp++ = *p++;
			sp += 4;
		}

		else if(*sp == 'T' && _strncmp(sp, "TIME", 4) == 0)
		{
			char s[80], *p;
			GetTimeFormat(MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
				TIME_FORCE24HOURFORMAT, pt, NULL, s, 80);
			p = s;
			while(*p) *dp++ = *p++;
			sp += 4;
		}

		else if(*sp == 'S') // <--+++--<<<<< WTF (If AnyThing) Does This Do?!?!?!?
		{
			int len, slen, st;
			sp++;
			if(GetNumFormat(&sp, 'd', &len, &slen) == TRUE)
			{
				if (!TickCount) TickCount = GetTickCount();
				st = TickCount/86400000;		//day
				SetNumFormat(&dp, st, len, slen);
			}
			else if(GetNumFormat(&sp, 'a', &len, &slen) == TRUE)
			{
				if (!TickCount) TickCount = GetTickCount();
				st = TickCount/3600000;		//hour
				SetNumFormat(&dp, st, len, slen);
			}
			else if(GetNumFormat(&sp, 'h', &len, &slen) == TRUE)
			{
				if (!TickCount) TickCount = GetTickCount();
				st = (TickCount/3600000)%24;
				SetNumFormat(&dp, st, len, slen);
			}
			else if(GetNumFormat(&sp, 'n', &len, &slen) == TRUE)
			{
				if (!TickCount) TickCount = GetTickCount();
				st = (TickCount/60000)%60;
				SetNumFormat(&dp, st, len, slen);
			}
			else if(GetNumFormat(&sp, 's', &len, &slen) == TRUE)
			{
				if (!TickCount) TickCount = GetTickCount();
				st = (TickCount/1000)%60;
				SetNumFormat(&dp, st, len, slen);
			}
			else if(*sp == 'T')
			{
				DWORD dw;
				int sth, stm, sts;
				if (!TickCount) TickCount = GetTickCount();
				dw = TickCount;
				dw /= 1000;
				sts = dw%60; dw /= 60;
				stm = dw%60; dw /= 60;
				sth = dw;

				SetNumFormat(&dp, sth, 1, 0);
				*dp++ = ':';
				SetNumFormat(&dp, stm, 2, 0);
				*dp++ = ':';
				SetNumFormat(&dp, sts, 2, 0);

				sp++;
			} // <--+++--<<<<< END of - WTF (If AnyThing) Does This Do?!?!?!?
			else
				*dp++ = 'S';
		}
		else if(*sp == 'W') { //----//--+++--> 3/21/2010 is 80th day of year
			char szWkNum[8] = {0}; //-----+++--> WEEK NUMBER CODE IS HERE!!!
			char *Wk;
			struct tm today;
			time_t ltime;
			time(&ltime);
			_localtime64_s(&today, &ltime);
			if(*(sp + 1) == 's') { // Week-Of-Year Starts Sunday
				strftime(szWkNum, 8, "%U", &today);
				Wk = szWkNum;
				while(*Wk) *dp++ = *Wk++;
				sp++;
			}
			else if(*(sp + 1) == 'm') { // Week-Of-Year Starts Monday
				strftime(szWkNum, 8, "%W", &today);
				Wk = szWkNum;
				while(*Wk) *dp++ = *Wk++;
				sp++;
			}
			// Need DOY + 6 / 7 (as float) DO NOT ROUND - Done!
			else if(*(sp + 1) == 'w') { // SWN (Simple Week Number)
					double dy; int d, s;
				 //------+++--> Stoic Joker's (Pipe Bomb Crude) Simple Week Number Calculation!
				strftime(szWkNum, 8, "%j", &today);   // Day-Of-Year as Decimal Number (1 - 366)
				dy = floor((atof(szWkNum) + 6) / 7); // DoY + 6 / 7 with the Fractional Part... 
													//-------------------------+++--> Truncated.
				Wk = _fcvt(dy, 0, &d, &s); // Make it a String	
				while(*Wk) *dp++ = *Wk++; // Done!
				sp++;
			}
			sp++; // Might Not be Needed!!!
		}
 //================================================================================================
//======================================= JULIAN DATE Code ========================================
		else if(*sp == 'J' && *(sp + 1) == 'D') {
				double y, M, d, h, m, s, bc, JD;
				struct tm Julian;
				int id, is, i=0;
				char *szJulian;
				time_t UTC;

			time(&UTC);
			_gmtime64_s(&Julian, &UTC);

			y = Julian.tm_year +1900;	// Year
			M = Julian.tm_mon +1;		// Month
			d = Julian.tm_mday;			// Day
			h = Julian.tm_hour;			// Hours
			m = Julian.tm_min;			// Minutes
			s = Julian.tm_sec;			// Seconds
			// This Handles the January 1, 4713 B.C up to
			bc = 100.0 * y + M - 190002.5; // Year 0 Part.
			JD = 367.0 * y;

			JD -= floor(7.0*(y + floor((M+9.0)/12.0))/4.0);
			JD += floor(275.0*M/9.0);
			JD += d;
			JD += (h + (m + s/60.0)/60.0)/24.0;
			JD += 1721013.5; // BCE 2 November 18 00:00:00.0 UT - Tuesday
			JD -= 0.5*bc/fabs(bc);
			JD += 0.5;

			szJulian = _fcvt(JD, 4, &id, &is); // Make it a String
			while(*szJulian) {
				if(i == id) { //--//-++-> id = Decimal Point Precision/Position
					*dp++ = '.'; // ReInsert the Decimal Point Where it Belongs.
				}else{
					*dp++ = *szJulian++; //--+++--> Done!
				}
				i++;
			}
			sp +=2;
		}
 //================================================================================================
//======================================= ORDINAL DATE Code =======================================
		else if(*sp == 'O' && *(sp + 1) == 'D') { //--------+++--> Ordinal Date UTC:
				char szOD[16] = {0};
				struct tm today;
				time_t UTC;
				char *od;

			time(&UTC);
			_gmtime64_s(&today, &UTC);
			strftime(szOD, 16, "%Y-%j", &today);
			od = szOD;
			while(*od) *dp++ = *od++;
			sp +=2;
		}
		//==========================================================================
		else if(*sp == 'O' && *(sp + 1) == 'd') { //------+++--> Ordinal Date Local:
				char szOD[16] = {0};
				struct tm today;
				time_t ltime;
				char *od;

			time(&ltime);
			_localtime64_s(&today, &ltime);
			strftime(szOD, 16, "%Y-%j", &today);
			od = szOD;
			while(*od) *dp++ = *od++;
			sp +=2;
		}
		//==========================================================================
		else if(*sp == 'D' && _strncmp(sp, "DOY", 3) == 0) { //--+++--> Day-Of-Year:
				char szDoy[8] = {0};
				struct tm today;
				time_t ltime;
				char *doy;

			time(&ltime);
			_localtime64_s(&today, &ltime);
			strftime(szDoy, 8, "%j", &today);
			doy = szDoy;
			while(*doy) *dp++ = *doy++;
			sp +=3;
		}
		//==========================================================================
		else if(*sp == 'P' && _strncmp(sp, "POSIX", 5) == 0) { //-> Posix/Unix Time:
				char szPosix[16] = {0}; // This will Give the Number of Seconds That Have
				char *posix; //--+++--> Elapsed Since the Unix Epoch: 1970-01-01 00:00:00

			wsprintf(szPosix, "%ld", time(NULL));
			posix = szPosix;
			while(*posix) *dp++ = *posix++;
			sp +=5;
		}
		//==========================================================================
		else if(*sp == 'T' && _strncmp(sp, "TZN", 3) == 0) { //--++-> TimeZone Name:
				char szTZName[TZNAME_MAX] = {0};
				size_t lRet;
				char *tzn;
				int iDST;

			_get_daylight(&iDST);
			if(iDST) {
				_get_tzname(&lRet, szTZName, TZNAME_MAX, 1);
			}else{
				_get_tzname(&lRet, szTZName, TZNAME_MAX, 0);
			}

			tzn = szTZName;
			while(*tzn) *dp++ = *tzn++;
			sp +=3;
		}
//=================================================================================================
		else
		{
			p = CharNext(sp);
			while(sp != p) *dp++ = *sp++;
		}
	}
	*dp = 0;
}

/*--------------------------------------------------
--------------------------------------- Check Format
--------------------------------------------------*/
DWORD FindFormat(char* fmt) {
	DWORD ret = 0;
	char *sp;
	
  sp = fmt;
  while(*sp) {
	if(*sp == '\"') {
	   sp++;
	   while(*sp != '\"' && *sp) sp++;
	   if(*sp == '\"') sp++;
	}
	
	else if(*sp == 's') {
	   sp++;
	   ret |= FORMAT_SECOND;
	}
	
	else if(*sp == '@' && *(sp + 1) == '@' && *(sp + 2) == '@') {
	   sp += 3;
	   if(*sp == '.' && *(sp + 1) == '@') {
		  ret |= FORMAT_BEAT2;
		  sp += 2;
	   }
	   else ret |= FORMAT_BEAT1;
	}

	else sp = CharNext(sp);
  }
 return ret;
}