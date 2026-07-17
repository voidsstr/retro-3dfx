/* $Header: monitor.c, 24, 10/11/00 8:51:30 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   monitor.c
**
** Description: functions to handle interfacing to the monitor.
**
** $Revision: 24$ 
** $Date: 10/11/00 8:51:30 PM$
**
** $History: monitor.c $
** 
** *****************  Version 48  *****************
** User: Andrew       Date: 8/16/99    Time: 2:46p
** Updated in $/devel/h5/Win9x/dx/dd16
** Removed MONITOR_DEVNODE_NOT_ACTIVE check from DoMonitor as this bit was
** getting set on the Primary Monitor when booting with Multi-Monitor and
** the secondary had no monitor attached.
** 
** *****************  Version 47  *****************
** User: Rbissell     Date: 8/07/99    Time: 3:11p
** Updated in $/devel/h5/Win9x/dx/dd16
** tvout merge from V3_OEM_100
** 
** *****************  Version 46  *****************
** User: Edwin        Date: 6/03/99    Time: 5:29p
** Updated in $/devel/h5/Win9x/dx/dd16
** Cleanup external reference declaration for DesktopModeList[] so the
** compiler generates the desired instructions.
** 
** 
** *****************  Version 45  *****************
** User: Michael      Date: 6/02/99    Time: 2:42p
** Updated in $/devel/h5/Win9x/dx/dd16
** Add -WX to the makefile and clean up all warnings in the DD16
** subdirectory.
** 
** *****************  Version 43  *****************
** User: Andrew       Date: 5/18/99    Time: 1:53p
** Updated in $/devel/h3/Win95/dx/dd16
** Added code to allow 8x6 modes if no modes are available
** 
** *****************  Version 42  *****************
** User: Andrew       Date: 5/18/99    Time: 9:54a
** Updated in $/devel/h3/Win95/dx/dd16
** Changed the fix for 5825 and 5552.  I did this by removing change 41.
** This fixes 6130 and then added a fix for 5825 and 5552 which if no
** Valid Resolution information is available to only allow 640x480.
** 
** *****************  Version 40  *****************
** User: Pratt        Date: 2/28/99    Time: 4:50p
** Updated in $/devel/h3/Win95/dx/dd16
** added debug statements for tracing flow of LCD code.
** 
** *****************  Version 39  *****************
** User: Stb_srogers  Date: 3/16/99    Time: 10:12a
** Updated in $/devel/h3/win95/dx/dd16
** Fix for PRS #5153, removes 1280x960 mode for Gateway drivers
** 
** *****************  Version 38  *****************
** User: Stb_srogers  Date: 3/12/99    Time: 1:59p
** Updated in $/devel/h3/win95/dx/dd16
** Added low-res modes back into Dell and Gateway Driver
** 
** *****************  Version 37  *****************
** User: Stb_srogers  Date: 3/09/99    Time: 2:19p
** Updated in $/devel/h3/win95/dx/dd16
** Removed 1152 mode from Dell list of modes
** 
** *****************  Version 36  *****************
** User: Stb_srogers  Date: 3/09/99    Time: 1:29p
** Updated in $/devel/h3/win95/dx/dd16
** Removed 960x720 and 1280x960 modes for Dell Driver.
** 
** *****************  Version 35  *****************
** User: Stb_srogers  Date: 3/06/99    Time: 7:38a
** Updated in $/devel/h3/win95/dx/dd16
** Removing 24 bit modes for Dell
** 
** *****************  Version 34  *****************
** User: Stb_srogers  Date: 3/02/99    Time: 10:18p
** Updated in $/devel/h3/Win95/dx/dd16
** Removed "#ifdef INCSTBCUST" so that nightly builds allow OEM mode
** culling
** 
** *****************  Version 33  *****************
** User: Stb_srogers  Date: 2/23/99    Time: 2:26p
** Updated in $/devel/h3/win95/dx/dd16
** Removed Gateway specific modes
** 
** *****************  Version 32  *****************
** User: Andrew       Date: 2/22/99    Time: 11:24a
** Updated in $/devel/h3/Win95/dx/dd16
** Added Back in fixes for EV700
** 
** *****************  Version 31  *****************
** User: Stb_srogers  Date: 2/16/99    Time: 8:35p
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 30  *****************
** User: Stb_srogers  Date: 2/16/99    Time: 4:16p
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 29  *****************
** User: Stb_srogers  Date: 2/11/99    Time: 6:57p
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 28  *****************
** User: Andrew       Date: 2/11/99    Time: 4:29p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed WriteRegistry to check to see if a mode exists before
** attempting to write the refresh rate.  The failure to do this allows
** the code to create keys for modes.
** 
** *****************  Version 27  *****************
** User: Andrew       Date: 2/11/99    Time: 3:00p
** Updated in $/devel/h3/Win95/dx/dd16
** Removed EV700 workaround
** 
** *****************  Version 26  *****************
** User: Stb_srogers  Date: 2/09/99    Time: 12:17p
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 25  *****************
** User: Andrew       Date: 2/04/99    Time: 11:17a
** Updated in $/devel/h3/Win95/dx/dd16
** Changed EV700 fudge factor from 30 to 40 to drop 1152x864 @ 75 Hz
** 
** *****************  Version 24  *****************
** User: Andrew       Date: 2/03/99    Time: 4:51p
** Updated in $/devel/h3/Win95/dx/dd16
** Added a fix for EV700 Monitor.  It appears that there are two types of
** EV700 monitors.  The have different EDID data but the same ID.  One can
** handle 1024x768 @ 85 Hz and one cannot.  The fixes tweaks for this
** monitor and lowers the refresh rate.
** 
** *****************  Version 23  *****************
** User: Stb_srogers  Date: 1/29/99    Time: 7:02a
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 22  *****************
** User: Cwilcox      Date: 1/25/99    Time: 11:37a
** Updated in $/devel/h3/Win95/dx/dd16
** Minor modifications to remove compiler warnings.
** 
** *****************  Version 21  *****************
** User: Stuartb      Date: 1/20/99    Time: 5:15p
** Updated in $/devel/h3/Win95/dx/dd16
** Removed extraneous #if 0 code and superfluous calls to
** broadcastMonitorChange.
** 
** *****************  Version 20  *****************
** User: Andrew       Date: 1/15/99    Time: 9:19a
** Updated in $/devel/h3/Win95/dx/dd16
** Fix for Gateway Monitor
** 
** *****************  Version 19  *****************
** User: Michael      Date: 1/12/99    Time: 1:55p
** Updated in $/devel/h3/Win95/dx/dd16
** Add the STB customer specific code.  All STB added code is surrounded
** by "#ifdef INCSTBCUST/#endif"
** 
** *****************  Version 18  *****************
** User: Stuartb      Date: 1/12/99    Time: 11:23a
** Updated in $/devel/h3/Win95/dx/dd16
** For the interim, the flat panel must be enabled by an ExtEscape
** (QUERY_LCDCTRL) call even if we booted to it.
** 
** *****************  Version 17  *****************
** User: Stuartb      Date: 1/08/99    Time: 5:11p
** Updated in $/devel/h3/Win95/dx/dd16
** Enable mode parsing for LCD if it was present at boot time only.
** 
** *****************  Version 16  *****************
** User: Michael      Date: 12/29/98   Time: 2:36p
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB unified header.
** 
** 15    12/23/98 5:18p Stuartb
** Added code with Andrew S' help to dynamically refresh mode list on
** enabling/disabling tvout or lcd panel.
** 
** 14    12/18/98 9:40p Andrew
** Fixed a bug with the Iiyama Monitor in which 1024x768 @ 120 Hz was
** consider valid
** 
** 13    11/17/98 6:24p Andrew
** Added Code to write to registry only the modes that we think that the
** monitor can do so that there is less confusion with the Property Page
** 
** 12    10/26/98 1:00p Andrew
** Changed the default resolution when the Monitor is unknown to 800x600.
** 
** 11    8/28/98 1:03p Andrew
** Added ability to trim the number of refreshes if it is a PNP with no
** range information
** 
** 3     8/27/98 11:25p Andrew
** Added a loop to parse throught the EDID data and reduce modes that
** don't have entries in the EDID.
** 
** 2     8/26/98 12:30a Andrew
** Added MonitorType registry key so that we will try the monitor as
** either a Multi-Sync or a Fixed Frequency
** 
** 9     7/13/98 1:42p Andrew
** Changed the Microsoft 75 Hz fix to fixup the bogus horizontal refresh
** rates instead of expanding the fudge factor.
** 
** 8     7/11/98 8:13a Andrew
** Added old fix back in
** 
** 7     7/07/98 3:08p Michael
** Backout Andrews changes for version 5.
** 
** 6     7/07/98 1:51p Michael
** In IsValidMode(), change HYSNC_FUDGE(80) back to HSYNC_FUDGE(10) to fix
** problem with 75hz detection on some monitors.
** 
** 5     7/03/98 10:25a Andrew
** Changed horizontal fudge factor to enable us to work with Win '98
** 1024x768 @ 75 Hz monitor.  Added code if no-modes are deemed valid to
** enable 640x480xxbpp
** 
** 4     5/21/98 5:39p Andrew
** Added code to look for and use MAX_REZ if available
** 
** 3     4/28/98 3:58p Michael
** change Msg() to include a new parameter that will allow for selective
** debug message output.
** 
** 2     4/23/98 4:43p Andrew
** Had a problem where EDID VRef with a range was being multiplied by 10.
** This caused no modes to be valid and the dib engine to crash as no mode
** would be set right.
** 
** 1     4/22/98 2:50p Andrew
** Routines to support finding the monitor data and joining the mode table
** with the monitor cap
** 
** 
*/


/***************************************************************************
* I N C L U D E S
****************************************************************************/

#include <string.h>
#include "header.h"

#define Not_VxD
#include "minivdd.h"

#define Not_VxD
#include <vmm.h>

#define MIDL_PASS     // suppress 32-bit only #pragma pack(push)

#pragma warning (disable: 4047 4704)
#include <configmg.h>
#pragma warning (default: 4047 4704)

// Fix for building with Windows 98 DDK (Must have WindowsX definitions)
#include <windowsx.h>

#include "modelist.h"
#include "monitor.h"

#include "qmodes.h"	/* for LPQIN Structure */
#include "tv.h"
#include "fxtvout.h"
#include "dfpapi.h"					
 
extern DISPLAYINFO DisplayInfo;
extern DWORD dwDevNode;
extern DWORD dwDeviceHandle;
extern int nNumModes;

void DoMonitor(void);
int ParseMode(LPMONINFO lpMonInfo, char FAR *lpszString);
BOOL FillRange(LPRANGE pRange, char FAR *lpTok);
LPMONINFO GetMonitorStruc(int nType);
DWORD IsValidMode(LPMODEINFO lpModeInfo, int nFirst);
void InsertEntry(LPMONINFO lpMonInfo);
void FreeMon(LPMONTYPE lpMonType);
void DoEDID(void);
int fatoi(char FAR * lpStr);
long int fhtol(char FAR * lpStr);
long int fatol10x(char FAR * lpStr);
int fatoi10x(char FAR * lpStr);
long int fatol(char FAR * lpStr);
DWORD VDDCall(DWORD myEAX, DWORD VDDmagicNumber, DWORD myECX, DWORD myEDX, LPVOID esdi);
int LookUpRate(WORD wX, WORD wY, WORD wVert);
int ConvertToMultiSync(void);
void GetBinary(DWORD dwDevNodeHandle, WORD FAR * lpValue, char FAR * lpStr, WORD nDefault);
int NotInEdid(LPMONTYPE lpTimmer, DWORD dwOldX, DWORD dwOldY);
int di_WriteRegistry(WORD wWidth, WORD wHeight, int * pRefreshRate, int nCount);
int di_DoStr(char FAR * pStr, int nNum, int nStart);
extern void TVOutGetStandard( LPQIN lpQIN, LPTVGETSTANDARD lpOutput );
extern void di_FillModeTable( MODEINFO FAR * modeinfo, int nummodes );
extern void di_SortModeTable( MODEINFO FAR * modeinfo, int nummodes );
 
MONITOR Monitor = {NULL, MONITOR_UNKNOWN, 0x0, 'A', '\0'};
LPMONTYPE lpTrimmer = NULL;
BOOL MonitorIsGTF = FALSE;	  // GTF monitor detection flag

#define VSYNC_FUDGE (2)       // Fudge the numbers... Hey this is not rocket science
#define HSYNC_FUDGE (10)      // Fudge the numbers... Hey this is not rocket science

#define TVO_DEV_NAME   "TV Encoder"
#define TVO_HDWE_NAME  "TV Encoder"
#define LCD_DEV_NAME  "Flat Panel"
// The following are monitor ID's specified by Microsoft.
// There horizontal refresh rates are bogus as they are specified to high
// and outside of the range of our fudge factor
char * FixUpIds[] = {
      "MonID_640@75",
      "MonID_800@75",
      "MonID_1024@75",
      "MonID_1280@75",
       };

// These are the fixups for Microsoft 75 Hz modes
WORD FixUpValue[] = {
      640, 25,            
      800, 21,
      1024, 50,
      1152, 25,
      1280, 40,
      };

// This is to Fixup for the Iiyama A701GT
// The problem is that are fudge factor allows 1024,768
char * FixUpId2[] = {
   "Monitor\\IVM1711",
   "Monitor\\GWY7658",
   };

WORD FixUpValue2[] = {
   0, HSYNC_FUDGE,         // Fudges for Iiyama A701GT Monitor
   0, 40,                  // Fudge for EV700 from 70 Khz to 67 Khz
   };

#define COMPAQ_FP730_ID			"Monitor\\CPQ3044"
#define COMPAQ_FP730_DEVICEID  	"COMPAQ FP730A Analog Flat Panel Monitor"
#define COMPAQ_FP730_EDID_PRODID 0x4430
#define COMPAQ_FP730_EDID_MANUFACTID 0x0E11 // fix this
#define COMPAQ_FP730_MAX_HREF 0x1E4 // taken from the inf file for the monitor
#define COMPAQ_FP730_MIN_HREF 0x136 // taken from the inf file for the monitor
  
char szBuffer[512];


/*----------------------------------------------------------------------
Function name:  DoMonitor

Description:    This routine reads in a monitor data structure and
                builds a data structure to describe it.  If it
                fails to get the information that it needs then it
                parses the EDID data.
Information:

Return:         VOID
----------------------------------------------------------------------*/
void DoMonitor(void)
{
   char FAR * lpStr;
	HKEY hKey;
	HKEY hKey1;
	CONFIGRET nReturn1;
	DWORD nReturn2;
	DWORD nReturn3;
	DWORD nKeyNum;
	DWORD nValueNum;
	DWORD dwType;
	DWORD cbValue;
	DWORD cbValue2;
	LPMONINFO lpMonInfo;
	int nTimes;
	int nX;
	int nY;
	int nKeyOpen;
	int k;
   char szToken[64];
   int nSize;

	// Initialize the Monitor Structure....
	Monitor.nType = MONITOR_UNKNOWN;
	FreeMon(Monitor.lpNext);
	Monitor.lpNext = NULL;

	// Check if the Info Is Valid...
	// Note that there is also a IS_STALE  flag but who cares???
	//
	// Assume monitor data is junk
	Monitor.IsValid &= ~(ALL_VALID);
   if (DisplayInfo.diMonitorDevNodeHandle)
		{
		Monitor.IsValid |= DISPLAY_INFO_VALID;

		// Here we enter the three way tree walk
		// At the first level we are looking for the keyword
		// Modes
		// At the second we will find the X and Y for this level
		// At the third we will find the ModeX string
		// This should be either a RANGE for Multi-Sync or a 
		// Fixed Frequency for Fixed Frequency Monitors.
		nReturn1 = CM_Get_DevNode_Key(DisplayInfo.diMonitorDevNodeHandle,
				"MODES", (PFARVOID)&szBuffer, sizeof(szBuffer), CM_REGISTRY_SOFTWARE);

		if (CR_SUCCESS == nReturn1)
			{
			// Ok.. Here's the One that we want to Open 
			if (RegOpenKey(HKEY_LOCAL_MACHINE, (LPSTR)&szBuffer, (LPHKEY)&hKey1) == ERROR_SUCCESS)
				{
				// Ok only two more levels to go
				// At this point the tree should look like
				// X,Y\\ModeX
				// The question is how many X,Y's are there..
				nReturn2 = ERROR_SUCCESS;
				nKeyNum = 0;
				for (;ERROR_SUCCESS == nReturn2;)
					{
					lpMonInfo = NULL;
					cbValue = sizeof(szBuffer);
					nReturn2 = RegEnumKey(hKey1, nKeyNum++, (LPSTR)&szBuffer, cbValue);
					if (ERROR_SUCCESS == nReturn2)
						{
                  nSize = strlen(szBuffer);
                  if (nSize > sizeof(szToken) - 1)
                     nSize = sizeof(szToken) - 1;
                  _fmemcpy(szToken, szBuffer, nSize);
                  szToken[nSize]='\0';
                  lpStr = _fstrtok((char FAR *)szToken, "\\,");
                  if (NULL == lpStr)
                     continue;
					   nX = fatoi(lpStr);

                  lpStr = _fstrtok(NULL,"\\,");
                  if (NULL == lpStr)
                     continue;
						nY = fatoi(lpStr);

					   // No point in allocating one until we have a X & Y
						lpMonInfo = GetMonitorStruc(MONINFO_REGISTRY);
						lpMonInfo->nResX = nX;
						lpMonInfo->nResY = nY;
						lpMonInfo->IsValid |= MAX_REZ_IS_VALID;
						Monitor.IsValid |= MAX_REZ_IS_VALID;
						nKeyOpen = 0;
						nReturn3 = RegOpenKey(hKey1, (LPSTR)&szBuffer, (LPHKEY)&hKey);
						nTimes = 0;
						nValueNum = 0;
						for (;ERROR_SUCCESS == nReturn3;)
						   {
							nKeyOpen = 1;
							cbValue = sizeof(szBuffer);
							cbValue2 = sizeof(szToken);
							nReturn3 = RegEnumValue(hKey, nValueNum++, (LPCSTR)&szBuffer, (LONG FAR *)&cbValue,
										0x0, (LONG FAR *)&dwType, (LPBYTE)&szToken, (LONG FAR *)&cbValue2);
							if (ERROR_SUCCESS == nReturn3)
							   {
								// Do the comparison in Upper Case
								for (k=0; k<4; k++)
								   if (RANGE(szBuffer[k],'a','z'))
									   szBuffer[k] = szBuffer[k] - 'a' + 'A';

								// Does this string have MODE in it?
								if ((memcmp(szBuffer,"MODE", 4) == 0) &&
									 (REG_SZ == dwType))
								   {
									// More then one Mode String???
									if (nTimes++)
										{
										InsertEntry(lpMonInfo);
										lpMonInfo = GetMonitorStruc(MONINFO_REGISTRY);
										lpMonInfo->nResX = nX;
										lpMonInfo->nResY = nY;
										lpMonInfo->IsValid |= MAX_REZ_IS_VALID;
										}
									Monitor.IsValid |= ParseMode(lpMonInfo, szToken);
									lpMonInfo->IsValid |= Monitor.IsValid;
									if ((lpMonInfo->rHort.nMin == lpMonInfo->rHort.nMax) ||
									    (lpMonInfo->rVert.nMin == lpMonInfo->rVert.nMax))
										Monitor.nType = MONITOR_FIXED_FREQUENCY;
									else if (Monitor.nType == MONITOR_UNKNOWN)
										Monitor.nType = MONITOR_MULTI_SYNC;
									} // It's a "MODE"
								} // Got a Value 
							} // Opened a Key & For EnumValue
						if (nKeyOpen)
							RegCloseKey(hKey);
						if (lpMonInfo)
							InsertEntry(lpMonInfo);
						} // Found a Key..
					} // RegEnumKey For loop
				RegCloseKey(hKey1);
				} // Opened "MODES" SubKey
         else
            {
            cbValue = sizeof(szBuffer);
            nReturn1 = CM_Read_Registry_Value(DisplayInfo.diMonitorDevNodeHandle,
   			   				NULL,
   				   			"MaxResolution",
   					   		REG_SZ,
   						   	(LPBYTE)szBuffer,
   							   &cbValue,
   							   CM_REGISTRY_SOFTWARE);
            if (CR_SUCCESS == nReturn1)
               {
               nSize = strlen(szBuffer);
               if (nSize > sizeof(szToken) - 1)
                  nSize = sizeof(szToken) - 1;
               _fmemcpy(szToken, szBuffer, nSize);
               szToken[nSize]='\0';
               lpStr = _fstrtok((char FAR *)szToken, "\\,");
               if (NULL != lpStr)
                  {
      				nX = fatoi(lpStr);
         
                  lpStr = _fstrtok(NULL,"\\,");
                  if (NULL != lpStr)
                     {
      		   		nY = fatoi(lpStr);
   
         				// No point in allocating one until we have a X & Y
      	   			lpMonInfo = GetMonitorStruc(MONINFO_REGISTRY);
         				lpMonInfo->nResX = nX;
         				lpMonInfo->nResY = nY;
         				lpMonInfo->IsValid |= MAX_REZ_IS_VALID;
        					Monitor.IsValid |= MAX_REZ_IS_VALID;
                     lpMonInfo->rHort.nMin = 0;
                     lpMonInfo->rHort.nMax = 0;
                     lpMonInfo->rVert.nMin = 0;
                     lpMonInfo->rVert.nMax = 0;
         			   InsertEntry(lpMonInfo);
                     }
                  }
               }
            }
         } // Found SubKey "MODES"
		}  // DevNode Handle Not Null 
}


/*----------------------------------------------------------------------
Function name:  ParseMode

Description:    This routine parses the ModeX string.

Information:

Return:         INT     REFRESH_IS_VALID if success or,
                        0 if failure.
----------------------------------------------------------------------*/
int ParseMode(LPMONINFO lpMonInfo, char FAR *lpStr)
{
   char FAR * lpTok;
   BOOL bReturn = FALSE;
   int nReturn = 0;

   /* Get the horizontal/vertical sync. range value(s) */
   lpTok = _fstrtok(lpStr,",;");
   bReturn = FillRange(&lpMonInfo->rHort, lpTok);
   lpTok = _fstrtok(NULL,",;");
   bReturn &= FillRange(&lpMonInfo->rVert, lpTok);

   // These are ten to large
   lpMonInfo->rVert.nMin /= 10; 
   lpMonInfo->rVert.nMax /= 10;
 
   if (bReturn)
      nReturn = REFRESH_IS_VALID;

   return nReturn;
}


/*----------------------------------------------------------------------
Function name:  FillRange

Description:    This routine is used to determine if a string is
                of the form xx.y-xx.y or xx.y.  The former is a
                range and the latter a broken range.
Information:

Return:         BOOL    TRUE  if success or,
                        FALSE if failure.
----------------------------------------------------------------------*/
BOOL FillRange(LPRANGE lpRange, char FAR *lpTok)
{
   BOOL bReturn = FALSE;
   BOOL bFound;
   int nPos;

   // NULL --> You ain't nothing
   if (NULL == lpTok)   
      {
      lpRange->nMin = 0;
      lpRange->nMax = 0;
      }
   else
      {
      bFound = FALSE;
      bReturn = TRUE;
      // Range or Single # ??
      for (nPos = 0; '\0' != lpTok[nPos]; nPos++)
         if ('-' == lpTok[nPos])
            {
            bFound = TRUE;
            lpTok[nPos] = '\0';
            break;
            }

      // Get Minimum
      lpRange->nMin = fatoi10x(lpTok);
      if (bFound)
         {
         // Get Maximum
         lpRange->nMax = fatoi10x(&lpTok[nPos+1]);
         lpTok[nPos] = '-';
         }
      else
         // Max = Min is a kind of Range
         lpRange->nMax = lpRange->nMin;
      }

   return bReturn;
}

#define HEADER_START 0x00
#define HEADER_SIZE 0x08

#define VENDOR_START 0x08
#define VENDOR_SIZE 0x0A

#define VERSION_START 0x12
#define VERSION_SIZE  0x02

#define DISPLAY_START 0x14
#define DISPLAY_SIZE  0x5

#define COLOR_START 0x19
#define COLOR_SIZE 0x0A

#define EST_START 0x23
#define EST_SIZE 0x03
#define E_720x400x70 0x80
#define E_720x400x88 0x40
#define E_640x480x60 0x20
#define E_640x480x67 0x10
#define E_640x480x72 0x08
#define E_640x480x75 0x04
#define E_800x600x56 0x02
#define E_800x600x60 0x01
#define E_800x600x72 0x80
#define E_800x600x75 0x40
#define E_832x624x75 0x20
#define E_1024x768x87i 0x10
#define E_1024x768x60 0x08
#define E_1024x768x70 0x04
#define E_1024x768x75 0x02
#define E_1280x1024x75 0x01
#define E_1152x864x75 0x80

#define STANDARD_START 0x26
#define STANDARD_SIZE 0x10

#define DETAIL_START 0x36
#define DETAIL_SIZE 0x48

#define MON_DATA_TYPE 0x03
#define MON_DATA_START 0x05
#define MON_DATA_VREF_MIN 0x05
#define MON_DATA_VREF_MAX 0x06
#define MON_DATA_HREF_MIN 0x07
#define MON_DATA_HREF_MAX 0x08

#define EXT_START 0x7E
#define EXT_SIZE 0x01

#define CHKSUM_START 0x7F
#define CHKSUM_SIZE 0x01

typedef struct est_ref {
	int nVRef;
	int nOffset;
	int nResIndex;
	BYTE bFlag;
} EST_REF, * PEST_REF, FAR * LPEST_REF;

int nCount = 0;
BOOL bUseEDID = FALSE;			 // did we get correct mode data from the EDID
BOOL bUseAltValidation = FALSE;  // fix for Compaq flat panel FP730

REF_DATA Ref_Data[MAX_REF_DATA];

EST_REF Est_Ref[] = {
	{60, 0x00, 0x00, E_640x480x60,}, /* 640x480x60 */
	{67, 0x00, 0x00, E_640x480x67,}, /* 640x480x67 */
	{72, 0x00, 0x00, E_640x480x72,}, /* 640x480x72 */
	{75, 0x00, 0x00, E_640x480x75,}, /* 640x480x75 */
	{70, 0x00, 0x01, E_720x400x70,}, /* 720x400x70 */
	{88, 0x00, 0x01, E_720x400x88,}, /* 720x400x88 */
	{56, 0x00, 0x02, E_800x600x56,}, /* 800x600x56 */
	{60, 0x00, 0x02, E_800x600x60,}, /* 800x600x60 */
	{72, 0x01, 0x02, E_800x600x72,}, /* 800x600x72 */
	{75, 0x01, 0x02, E_800x600x75,}, /* 800x600x75 */
	{75, 0x01, 0x03, E_800x600x75,}, /* 832x624x75 */
	{43, 0x01, 0x04, E_1024x768x87i,}, /* 1024x768x87i */
	{60, 0x01, 0x04, E_1024x768x60,}, /* 1024x768x60 */
	{70, 0x01, 0x04, E_1024x768x70,}, /* 1024x768x70 */
	{75, 0x01, 0x04, E_1024x768x75,}, /* 1024x768x75 */
	{75, 0x02, 0x05, E_1152x864x75,}, /* 1152x864x75 */
	{75, 0x01, 0x06, E_1280x1024x75,}, /* 1280x1024x75 */
	};

typedef struct est_res {
	WORD nHor;			/* Horizontal Pixels */
	WORD nVer;			/* Vertical Lines */
} EST_RES, * PEST_RES, FAR * LPEST_RES;

EST_RES Est_Res[] = {
	640, 480,  			/* 0x00 */
	720, 400,			/* 0x01 */
	800, 600,			/* 0x02 */
	832, 624,			/* 0x03 */
	1024, 768,			/* 0x04 */
	1152, 864,			/* 0x05 */
	1280, 1024,			/* 0x06 */
	};

int CheckSum(BYTE * pData, int nSize);
int SortRefData(LPREF_DATA lpRef_Data, int nCount);
int AddRefData(LPREF_DATA lpRef_Data, int FAR * lpCount, WORD nHor, WORD nVer, WORD nRef);

#define EDID_20_SIZE 128
#define MAX_EDID_SIZE (EDID_20_SIZE + 2)
BYTE EDID[MAX_EDID_SIZE];

int nMult[] =  {1,3,4,9};
int nDiv[] = {1,4,5,16};


/*----------------------------------------------------------------------
Function name:  DoEDID

Description:    Extract the EDID data.

Information:
  Notes:
   	The compiler does not assemble this line correctly. 
  	Hact = (WORD)EDID[DETAIL_START + i + 2] | 
                (((WORD)EDID[DETAIL_START + i + 4] & 0xF0) << 4);
    For some reason it does something like this:
    mov	ch, byte ptr EDID[]
	and	cx, F000h
	shl	cx, 4		<==  This is bad

Therefore I added the pragma to give it some help..

Return:         VOID
----------------------------------------------------------------------*/
#pragma optimize("",off)
void DoEDID(void)
{
	DWORD Pixel_Clock;
	LONG nLen;
	WORD HRefMax;
	WORD HRefMin;
	LPMONINFO lpMonInfo;
	WORD HTotal;
	WORD Hact;
	WORD HBlk;
	WORD VTotal;
	WORD Vact;
	WORD VBlk;
	WORD nVersion;
	WORD VRefMax;
	WORD VRefMin;
	WORD nRange;
	WORD nMonType;
	WORD nProdID = 0x0;
	WORD nManufactID = 0x0;
	int VRef;
	int i;
#ifdef EXTENDED_STANDARD_MONITOR
	int j;
#endif
	int nIndex;
    int ret;

	memset(&Ref_Data[0], 0, sizeof(Ref_Data));	// Clear the array to avoid junk values

	MonitorIsGTF = FALSE; 	
	nRange = 0;
	nCount = 0;
	nLen = sizeof(EDID);
	bUseEDID = FALSE;

	Monitor.nType = MONITOR_UNKNOWN;
	FreeMon(Monitor.lpNext);
	Monitor.lpNext = NULL;
	Monitor.IsValid &= ~(ALL_VALID);		  
	
   
    if (DFPisPanelActive())
    {
        nLen = EDID_20_SIZE;

        ret = !VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
                       H3VDD_FLATPNL_EDID, 0, &EDID[0]);
    }
    else
    {
        ret = CM_Read_Registry_Value(DisplayInfo.diMonitorDevNodeHandle,
			                         NULL,
			                         "EDID",
			                         REG_BINARY,
			                         (LPBYTE)&EDID[0],
			                         &nLen,
			                         CM_REGISTRY_HARDWARE) == CR_SUCCESS;
    }

	if (ret)
		{
		// Make sure size is what we expect
		if (EDID_20_SIZE == nLen)
			{

			// Is CheckSum ok ?
			if (CheckSum(EDID, EDID_20_SIZE) == 0x0)
				{
				// At this Point since we are going to use the EDID
				// free anything that we got from the registry
				FreeMon(Monitor.lpNext);
				Monitor.lpNext = NULL;
				nCount=0;

				nManufactID = (EDID[VENDOR_START] << 8) | EDID[VENDOR_START+1];
				nProdID = (EDID[VENDOR_START+2] << 8) | EDID[VENDOR_START+3];

				if (nProdID == COMPAQ_FP730_EDID_PRODID && nManufactID == COMPAQ_FP730_EDID_MANUFACTID)
					bUseAltValidation = TRUE;
				else
					bUseAltValidation = FALSE;

				nVersion = EDID[VERSION_START]*10 + EDID[VERSION_START+1];
				// Add Established Refresh Rates to Table
				for (i=0; i<sizeof(Est_Ref)/sizeof(EST_REF); i++)
					if (EDID[EST_START + Est_Ref[i].nOffset] & Est_Ref[i].bFlag)
						AddRefData((LPREF_DATA)&Ref_Data[0], (int FAR *)&nCount, Est_Res[Est_Ref[i].nResIndex].nHor, Est_Res[Est_Ref[i].nResIndex].nVer, Est_Ref[i].nVRef);

				// Add Standard Refresh Rates to Table
				for (i=0; i<16; i+=2)
					{
					if ((EDID[STANDARD_START + i] == 0) ||
						 (EDID[STANDARD_START + i] == 1))
						continue;

					Hact=((WORD)EDID[STANDARD_START + i]) + 31 << 3;
					nIndex = ((EDID[STANDARD_START + i + 1] & 0xC0) >> 6);
					Vact= Hact * nMult[nIndex]/nDiv[nIndex];
					VRef = (EDID[STANDARD_START + i + 1] & 0x3F) + 60;
					AddRefData((LPREF_DATA)&Ref_Data[0], (int FAR *)&nCount, Hact, Vact, VRef);
					}

				// Add Detail Refresh Rates to Table
				for (i=0; i<DETAIL_SIZE; i+=18)
					{
					Pixel_Clock = EDID[DETAIL_START + i] | (((WORD)EDID[DETAIL_START + i + 1]) << 8);
					if (Pixel_Clock == 0x0)
						{
						// If version is greater then 10 then you could have a monitor
						// descriptor
						if (nVersion > 10)
							{
							nMonType = EDID[DETAIL_START + i + MON_DATA_TYPE];
							switch (nMonType)
								{
								case 0xFF:		// Serial Number
									break;

								case 0xFE:		// ASCII String
									break;

								case 0xFD:
									nRange = 1;
									HRefMin = EDID[DETAIL_START + i + MON_DATA_HREF_MIN] * 10;
									HRefMax = EDID[DETAIL_START + i + MON_DATA_HREF_MAX] * 10;
									VRefMin = EDID[DETAIL_START + i + MON_DATA_VREF_MIN];
									VRefMax = EDID[DETAIL_START + i + MON_DATA_VREF_MAX];
									break;

								case 0xFC:  	// Monitor Name
									break;
	
								case 0xFB:		// Color Point
									break;

								case 0xFA:		// This is a standard timing
							#ifdef EXTENDED_STANDARD_MONITOR
									// need a monitor to test this
									// with
									for (j=0; j<12; j+=2)
										{
										if ((EDID[DETAIL_START + MON_DATA_START + i + j] == 0) ||
											 (EDID[DETAIL_START + MON_DATA_START + i + j] == 1))
											break;

										Hact=((WORD)EDID[DETAIL_START + MON_DATA_START + i + j]) + 31 << 3;
										nIndex = ((EDID[DETAIL_START + MON_DATA_START + i + j + 1] & 0xC0) >> 6);
										Vact= Hact * nMult[nIndex]/nDiv[nIndex];
										VRef = (EDID[DETAIL_START + MON_DATA_START + i + j + 1] & 0x3F) + 60;
										AddRefData((LPREF_DATA)&Ref_Data[0], (int FAR *)&nCount, Hact, Vact, VRef);
										}
							#endif
									break;
	
								default:			// Punt
									break;
								}
							}
						}
					else
						{
						Pixel_Clock	*= 10000;
   					Hact = (WORD)EDID[DETAIL_START + i + 2] | (((WORD)EDID[DETAIL_START + i + 4] & 0xF0) << 4);
						HBlk = (WORD)EDID[DETAIL_START + i + 3] | (((WORD)EDID[DETAIL_START + i + 4] & 0x0F) << 8);
						HTotal = Hact + HBlk;
						Vact = (WORD)EDID[DETAIL_START + i + 5] | (((WORD)EDID[DETAIL_START + i + 7] & 0xF0) << 4);
						VBlk = (WORD)EDID[DETAIL_START + i + 6] | (((WORD)EDID[DETAIL_START + i + 7] & 0x0F) << 8);
						VTotal = Vact + VBlk;
						VRef = (int)(Pixel_Clock/((DWORD)HTotal * (DWORD)VTotal));
						AddRefData((LPREF_DATA)&Ref_Data[0], (int FAR *)&nCount, Hact, Vact, VRef);
						}
					}

				// Sort the Rates Lowest to Highest
				SortRefData(&Ref_Data[0], nCount);
				// Ok if we have a Range then let's make this a 
				// Multi-Sync Monitor else it is a Fix Frequency Monitor
				if (nRange)
					{
					FreeMon(Monitor.lpNext);
					Monitor.lpNext = NULL;
					lpMonInfo = GetMonitorStruc(MONINFO_REGISTRY);
					lpMonInfo->nResX = Ref_Data[nCount-1].nHor;
					lpMonInfo->nResY = Ref_Data[nCount-1].nVer;
					lpMonInfo->IsValid |= MAX_REZ_IS_VALID;
					lpMonInfo->rHort.nMin = HRefMin;
					lpMonInfo->rHort.nMax = HRefMax;
					lpMonInfo->rVert.nMin = VRefMin;
					lpMonInfo->rVert.nMax = VRefMax;
					lpMonInfo->IsValid |= (MAX_REZ_IS_VALID | REFRESH_IS_VALID);
					InsertEntry(lpMonInfo);
					Monitor.nType = MONITOR_MULTI_SYNC;
					if ( EDID[24] & 1 )			 	// If GTF monitor is detected
						MonitorIsGTF = TRUE;
					bUseEDID = TRUE;
					Monitor.IsValid |= (MAX_REZ_IS_VALID | REFRESH_IS_VALID);
					}
				else if (nCount)
					{
					bUseEDID = TRUE;
					Monitor.nType = MONITOR_FIXED_FREQUENCY;
					Monitor.IsValid |= (MAX_REZ_IS_VALID | REFRESH_IS_VALID);
					}
				}	// checksum is ok
			}	// size is 128 bytes
		}	// edid in registry
		if (!bUseEDID)
			DoMonitor();
}
#if 0
// ??? use this line if it is necessary to force VESA timings for DFP
extern BOOL di_GetBaselineTiming( TIMING_PARAMS *pTprm );
#endif
/*----------------------------------------------------------------------
Return:         VOID
----------------------------------------------------------------------*/
void UpdateModeFromEdid( MODEINFO FAR * ModeInfo )
{
    DWORD Pixel_Clock;
    LONG nLen;
    WORD HRefMax;
    WORD HRefMin;
    WORD HTotal;
    WORD Hact;
    WORD HBlk;
    WORD VTotal;
    WORD Vact;
    WORD VBlk;
    WORD nVersion;
    WORD VRefMax;
    WORD VRefMin;
    WORD nRange;
    WORD nMonType;
    int VRef;
    int i;
#ifdef EXTENDED_STANDARD_MONITOR
    int j;
#endif
    int nIndex;
    int ret;
    int found = FALSE;
    TIMING_PARAMS localTimings;
	FxU16    hsyncWidth;
	FxU16    hsyncOfst;
	FxU16    vsyncWidth;
	FxU16    vsyncOfst;
	WORD    matchWidth;
	WORD    matchHeight;
	int    matchRefresh;

    localTimings.width = ModeInfo->dwWidth;
    localTimings.height = ModeInfo->dwHeight;
    localTimings.refresh = ModeInfo->wVert;

	matchWidth = 0;
	matchHeight= 1;
	matchRefresh= 1;


    nRange = 0;
    nCount = 0;
    nLen = sizeof(EDID);

    nLen = EDID_20_SIZE;

    ret = !VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
            H3VDD_FLATPNL_EDID, 0, &EDID[0]);
    if (ret)
    {
        // Make sure size is what we expect
        if (EDID_20_SIZE == nLen)
        {

            // Is CheckSum ok ?
            if (CheckSum(EDID, EDID_20_SIZE) == 0x0)
            {
                nVersion = EDID[VERSION_START]*10 + EDID[VERSION_START+1];
                // Search Established Refresh Rates
                for (i=0; i<sizeof(Est_Ref)/sizeof(EST_REF); i++)
                    if (EDID[EST_START + Est_Ref[i].nOffset] & Est_Ref[i].bFlag)
                        if ((Est_Res[Est_Ref[i].nResIndex].nHor == localTimings.width) &&
                            (Est_Res[Est_Ref[i].nResIndex].nVer == localTimings.height) &&
                            ((WORD)Est_Ref[i].nVRef == localTimings.refresh))
                        {
#if 0
// ??? use this line if it is necessary to force VESA timings for DFP
                            found = di_GetBaselineTiming( &localTimings );
                            break;
#else
                            return;  //don't touch timing
#endif
                        }

                        // Search Standard Refresh Rates
                        for (i=0; i<16; i+=2)
                        {
                            if ((EDID[STANDARD_START + i] == 0) ||
                                    (EDID[STANDARD_START + i] == 1))
                                continue;

                            Hact=((WORD)EDID[STANDARD_START + i]) + 31 << 3;
                            nIndex = ((EDID[STANDARD_START + i + 1] & 0xC0) >> 6);
                            Vact= Hact * nMult[nIndex]/nDiv[nIndex];
                            VRef = (EDID[STANDARD_START + i + 1] & 0x3F) + 60;
                            if ((Hact == localTimings.width) &&
                                (Vact == localTimings.height) &&
                                ((WORD)VRef == localTimings.refresh))
                            {
#if 0
// ??? use this line if it is necessary to force VESA timings for DFP
                                found = di_GetBaselineTiming( &localTimings );
                                break;
#else
                                return;  //don't touch timing
#endif
                            }
                        }

                        // Search Detail Refresh Rates
                        for (i=0; i<DETAIL_SIZE; i+=18)
                        {
                            Pixel_Clock = EDID[DETAIL_START + i] | (((WORD)EDID[DETAIL_START + i + 1]) << 8);
                            if (Pixel_Clock == 0x0)
                            {
                                // If version is greater then 10 then you could have a monitor
                                // descriptor
                                if (nVersion > 10)
                                {
                                    nMonType = EDID[DETAIL_START + i + MON_DATA_TYPE];
                                    switch (nMonType)
                                    {
                                    case 0xFF:		// Serial Number
                                        break;

                                    case 0xFE:		// ASCII String
                                        break;

                                    case 0xFD:
                                        nRange = 1;
                                        HRefMin = EDID[DETAIL_START + i + MON_DATA_HREF_MIN] * 10;
                                        HRefMax = EDID[DETAIL_START + i + MON_DATA_HREF_MAX] * 10;
                                        VRefMin = EDID[DETAIL_START + i + MON_DATA_VREF_MIN];
                                        VRefMax = EDID[DETAIL_START + i + MON_DATA_VREF_MAX];
                                        break;

                                    case 0xFC:  	// Monitor Name
                                        break;

                                    case 0xFB:		// Color Point
                                        break;

                                    case 0xFA:		// This is a standard timing
#ifdef EXTENDED_STANDARD_MONITOR
                                        // need a monitor to test this
                                        // with
                                        for (j=0; j<12; j+=2)
                                        {
                                            if ((EDID[DETAIL_START + MON_DATA_START + i + j] == 0) ||
                                                    (EDID[DETAIL_START + MON_DATA_START + i + j] == 1))
                                                break;

                                        Hact=((WORD)EDID[DETAIL_START + MON_DATA_START + i + j]) + 31 << 3;
                                        nIndex = ((EDID[DETAIL_START + MON_DATA_START + i + j + 1] & 0xC0) >> 6);
                                        Vact= Hact * nMult[nIndex]/nDiv[nIndex];
                                        VRef = (EDID[DETAIL_START + MON_DATA_START + i + j + 1] & 0x3F) + 60;
                                        }
                                        ???not modified for DFP
#endif
                                                break;

                                    default:			// Punt
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                Pixel_Clock	*= 10000;
                                Hact = (WORD)EDID[DETAIL_START + i + 2] | (((WORD)EDID[DETAIL_START + i + 4] & 0xF0) << 4);
                                HBlk = (WORD)EDID[DETAIL_START + i + 3] | (((WORD)EDID[DETAIL_START + i + 4] & 0x0F) << 8);
                                HTotal = Hact + HBlk;
                                hsyncOfst = (((WORD)EDID[DETAIL_START + i + 11] << 2) & 0x300) | (WORD)EDID[DETAIL_START + i + 8];
                                hsyncWidth = (((WORD)EDID[DETAIL_START + i + 11] << 4) & 0x300) | (WORD)EDID[DETAIL_START + i + 9];

                                Vact = (WORD)EDID[DETAIL_START + i + 5] | (((WORD)EDID[DETAIL_START + i + 7] & 0xF0) << 4);
                                VBlk = (WORD)EDID[DETAIL_START + i + 6] | (((WORD)EDID[DETAIL_START + i + 7] & 0x0F) << 8);
                                VTotal = Vact + VBlk;
                                vsyncOfst = (((WORD)EDID[DETAIL_START + i + 11] << 2) & 0x30) | ((WORD)EDID[DETAIL_START + i + 10] >> 4);
                                vsyncWidth = (((WORD)EDID[DETAIL_START + i + 11] << 4) & 0x30) | ((WORD)EDID[DETAIL_START + i + 10] & 15);

                                VRef = (int)(Pixel_Clock/((DWORD)HTotal * (DWORD)VTotal));
                                if ((Hact == localTimings.width) &&
                                    (Vact == localTimings.height) &&
                                    ((WORD)VRef == localTimings.refresh))
                                {
                                    localTimings.HSyncStart = Hact + hsyncOfst;
                                    localTimings.HSyncEnd = Hact + hsyncOfst + hsyncWidth;
                                    localTimings.HTotal = HTotal;
                                    localTimings.VSyncStart = Vact + vsyncOfst;
                                    localTimings.VSyncEnd = Vact + vsyncOfst + vsyncWidth;
                                    localTimings.VTotal = VTotal;
                                    localTimings.PixelClock = Pixel_Clock;



                                    found = TRUE;
                                    break;

                                }

                            }
                        }
            }
        }
        if (found)
        {
            // Update mode in ModeInfo structure
            ModeInfo->wHorz = (WORD)(( localTimings.PixelClock / 100 ) / localTimings.HTotal );
            ModeInfo->HTotal = localTimings.HTotal;
            ModeInfo->HSyncStart = localTimings.HSyncStart;
            ModeInfo->HSyncEnd = localTimings.HSyncEnd;
            ModeInfo->VTotal =  localTimings.VTotal;
            ModeInfo->VSyncStart = localTimings.VSyncStart;
            ModeInfo->VSyncEnd = localTimings.VSyncEnd;
            ModeInfo->PixelClock = localTimings.PixelClock;
        }
    }
}
/*----------------------------------------------------------------------
Function name:  GetMaxEDID_Refresh

Description:    Return the maximum refresh described in EDID block

Information:

Notes:			The Ref_Data array refresh rates are not sorted

Return:         INT     Maximum refresh described in EDID block
----------------------------------------------------------------------*/

int GetMaxEDID_Refresh( WORD wWidth, WORD wHeight )
{
    int i;
    WORD maxref = 0;

    for (i = 0; i < MAX_REF_DATA; i++)
    {
        if ( Ref_Data[i].nHor == wWidth && Ref_Data[i].nVer == wHeight )
        {
            while ( Ref_Data[i].nHor == wWidth && Ref_Data[i].nVer == wHeight )
            {
                if ( Ref_Data[i].nRef > maxref )
                    maxref = Ref_Data[i].nRef;
                i++;
		    }
            return( maxref );
        }
 	}

    return 0;
}
#pragma optimize("",on)


/*----------------------------------------------------------------------
Function name:  CheckSum

Description:    Perform a Checksum.

Information:
  Where:
 pData is Pointer to EDID data
  nSize is in bytes

  Notes:
   The value should checksum to zero if nSize = 128 is passed or to
   EDID[127] if nSize=127.

Return:         INT     Value of the checksum
----------------------------------------------------------------------*/
int CheckSum(BYTE * pData, int nSize)
{
	int i;
	BYTE bSum;

	bSum=0;
	for (i=0; i<nSize; i++)
		bSum+= *pData++;

	return (int)bSum;
}


/*----------------------------------------------------------------------
Function name:  AddRefData

Description:    Add the refresh data.

Information:
  Where:
 lpRef_Data is a FAR pointer to Ref_Data
  lpCount is a FAR pointer to number of elements
  nHor is Horizontal Size
  nVer is Vertical Size
  nRef is Refresh Rate

  Notes:
 Ref Data has been size for DDC ver 1.10 rev 0. This function only adds a
  new element to Ref_Data if the entry is not already in the table.  If it
  already exists then the refresh rate may be updated if it is larger.

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int AddRefData(LPREF_DATA lpRef_Data, int FAR * lpCount, WORD nHor, WORD nVer, WORD nRef)
{
	LPMONEDID lpMonEDID;
	int i;
	int nFound;

	nFound = 0;
	for (i=0; i<*lpCount; i++)
		if ((lpRef_Data[i].nHor == nHor) && (lpRef_Data[i].nVer == nVer) &&
			 (lpRef_Data[i].nRef == nRef))
			{
			nFound = 1;
			break;
			}

	if (!nFound)
		{
		lpRef_Data[*lpCount].nHor =  nHor;
		lpRef_Data[*lpCount].nVer =  nVer;
		lpRef_Data[*lpCount].nRef =  nRef;
		*lpCount = *lpCount + 1;
		lpMonEDID = (LPMONEDID)GetMonitorStruc(MONINFO_EDID);
		if (NULL != lpMonEDID)
			{
			lpMonEDID->nHor = nHor;
			lpMonEDID->nVer = nVer;
         lpMonEDID->nVRef = nRef; 
			lpMonEDID->IsValid |= (MAX_REZ_IS_VALID | REFRESH_IS_VALID);
			InsertEntry((LPMONINFO)lpMonEDID);
			}
		}

	return 0;
}


/*----------------------------------------------------------------------
Function name:  SortRefData

Description:    Sort the refresh data.

Information:
  Where:
 lpRef_Data is a FAR pointer to Ref_Data
  nCount is the number of elements

  Notes:
 Use a simple bubble sort to order the data on Key X first and then Key Y.

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int SortRefData(LPREF_DATA lpRef_Data, int nCount)
{
	REF_DATA Ref;
	int i;
	int j;

	for (i=0; i<nCount; i++)
		for (j=i; j<nCount; j++)
			if ((lpRef_Data[i].nHor > lpRef_Data[j].nHor) ||
				 ((lpRef_Data[i].nHor == lpRef_Data[j].nHor) &&
				  (lpRef_Data[i].nVer > lpRef_Data[j].nVer)) ||

				  ((lpRef_Data[i].nHor == lpRef_Data[j].nHor) &&   // sort also based by refresh rate jmccartney
				  (lpRef_Data[i].nVer == lpRef_Data[j].nVer)  &&
				  (lpRef_Data[i].nRef > lpRef_Data[j].nRef)))
				{
				_fmemcpy((LPREF_DATA)&Ref, &lpRef_Data[j], sizeof(REF_DATA));
				_fmemcpy(&lpRef_Data[j], &lpRef_Data[i], sizeof(REF_DATA));
				_fmemcpy(&lpRef_Data[i], (LPREF_DATA)&Ref, sizeof(REF_DATA));
				}

	return 0;
}


/*----------------------------------------------------------------------
Function name:  GetMonitorStruct

Description:    Allocate a MONINFO structure.

Information:
  Where:
   nType -- Either MONINFO_REGISTRY or MONINFO_EDID

Return:         LPMONINFO   Valid pointer or,
                            NULL if failure
----------------------------------------------------------------------*/
LPMONINFO GetMonitorStruc(int nType)
{
	LPMONINFO lpMonInfo = NULL;
	DWORD dwSize;

	// Determine Size of Structure
	if (MONINFO_REGISTRY == nType)
		dwSize = sizeof(MONINFO);
	else
		dwSize = sizeof(MONEDID);

	// Allocate and Lock
	lpMonInfo = (LPMONINFO)GlobalLock(GlobalAlloc((GMEM_MOVEABLE|GMEM_SHARE), (dwSize)));

	// If it works.. fill out what we know
	if (NULL != lpMonInfo)
		{
		lpMonInfo->lpNext = NULL;
		lpMonInfo->nType = nType;
		lpMonInfo->IsValid = DISPLAY_INFO_VALID;
		}
	else
		{
	   DPF(DBGLVL_NORMAL, "GlobalAlloc for GetMonitorStruc Failed");
    	DPF(DBGLVL_NORMAL, "GetMonitorStruc returns Zero");
		}


	DPF(DBGLVL_NORMAL, "Get Entry %lx", lpMonInfo);

	return lpMonInfo;
}


/*----------------------------------------------------------------------
Function name:  FreeMon

Description:    Release a monitor node.

Information:
   Beware: without optimization off windows will free pointer
   then dereference.  This results in a page fault....

   Where:
    nType -- Either MONINFO_REGISTRY or MONINFO_EDID

Return:         VOID
----------------------------------------------------------------------*/
#pragma optimize("", off)
void FreeMon(LPMONTYPE lpMonType)
{
	LPMONTYPE lpMonNext;

	DPF(DBGLVL_NORMAL, "FreeMon");
	// Walk the link list and free unsed entries
	for (;NULL != lpMonType;)
		{
		lpMonNext = lpMonType->lpNext;
		DPF(DBGLVL_NORMAL, "Free %lx", lpMonType);
		GlobalUnlockPtr(lpMonType);
		GlobalFree(GlobalPtrHandle(lpMonType));
		lpMonType = lpMonNext;
		}
}
#pragma optimize("", on)


/*----------------------------------------------------------------------
Function name:  InsertEntry

Description:    Insert a monitor entry into the linked list.

Information:
  Where:
   lpMonInfo -- Insert a MonInfoEntry

Return:         VOID
----------------------------------------------------------------------*/
void InsertEntry(LPMONINFO lpMonInfo)
{
	LPMONTYPE lpMonFirst;
	LPMONTYPE lpMonLast;
	LPMONINFO lpMonOld;
	LPMONINFO lpMonNew;
	LPMONEDID lpEDIDOld;
	LPMONEDID lpEDIDNew;
	int nFound = 0;

	// Walk the link list

	DPF(DBGLVL_NORMAL, "Insert Entry %lx", lpMonInfo);
	// At the Head
	if (NULL == Monitor.lpNext)
		{
		Monitor.lpNext = (LPMONTYPE)lpMonInfo;
	   DPF(DBGLVL_NORMAL, "At Head");
		}
	else
		{
		// Insert in the right spot
		lpMonLast = lpMonFirst = Monitor.lpNext;
		for (; (0 == nFound) && (NULL != lpMonFirst); )
			{
			// First Parse on X & Y's
			if ((lpMonInfo->nResX < lpMonFirst->nResX) &&
				 (lpMonInfo->nResY < lpMonFirst->nResY))
				{
				nFound = 1;
	   		DPF(DBGLVL_NORMAL, "(%d %d) < (%d %d)", lpMonInfo->nResX, lpMonInfo->nResY, lpMonFirst->nResX, lpMonFirst->nResY);
				}
			else if ((lpMonInfo->nResX == lpMonFirst->nResX) &&
						(lpMonInfo->nResY < lpMonFirst->nResY))
				{
				nFound = 1;
	   		DPF(DBGLVL_NORMAL, "X==X Y < Y(%d %d) < (%d %d)", lpMonInfo->nResX, lpMonInfo->nResY, lpMonFirst->nResX, lpMonFirst->nResY);
				}
			else if ((lpMonInfo->nResX == lpMonFirst->nResX) &&
						(lpMonInfo->nResY == lpMonFirst->nResY))
				{
				// X==X and Y==Y so now use refresh
				if (lpMonInfo->nType == lpMonFirst->nType)
					{
					if (MONINFO_REGISTRY == lpMonInfo->nType)
						{
						lpMonNew = (LPMONINFO)lpMonInfo;
						lpMonOld = (LPMONINFO)lpMonFirst;
						if ((lpMonOld->rHort.nMin <= lpMonNew->rHort.nMin) &&
							 (lpMonOld->rHort.nMax <= lpMonNew->rHort.nMax) &&
							 (lpMonOld->rVert.nMin <= lpMonNew->rVert.nMin) &&
							 (lpMonOld->rVert.nMax <= lpMonNew->rVert.nMax))
							{
							nFound = 1;
		   		      DPF(DBGLVL_NORMAL, "Range ?? (%d %d) (%d %d) (%d %d) (%d %d)",
								lpMonOld->rHort.nMin,
							   lpMonOld->rHort.nMax,
							   lpMonOld->rVert.nMin,
							   lpMonOld->rVert.nMax,
								lpMonNew->rHort.nMin,
							   lpMonNew->rHort.nMax,
							 	lpMonNew->rVert.nMin,
							   lpMonNew->rVert.nMax);
							}
						}
					else
						{
						lpEDIDNew = (LPMONEDID)lpMonInfo;
						lpEDIDOld = (LPMONEDID)lpMonFirst;
						if	(lpEDIDNew->nVRef < lpEDIDOld->nVRef)
							{
							nFound = 1;
		   		      DPF(DBGLVL_NORMAL, "Vref ?? %d %d", lpEDIDNew->nVRef, lpEDIDOld->nVRef);
							}
						}
					}
				else
					{
					nFound = 1;
		   		DPF(DBGLVL_NORMAL, "Types ?? %d %d", lpMonInfo->nType, lpMonFirst->nType);
					}
				}

			if (0 == nFound)
				{
				lpMonLast = lpMonFirst;
				lpMonFirst = lpMonFirst->lpNext;
				}
			}

		// If last == first then we are at the end
		// so insert at the head
		if (lpMonLast == lpMonFirst)
			Monitor.lpNext = (LPMONTYPE)lpMonInfo;
		else
			lpMonLast->lpNext = (LPMONTYPE)lpMonInfo;

		lpMonInfo->lpNext = (LPMONINFO)lpMonFirst;
		}
}


/*----------------------------------------------------------------------
Function name:  GetMonitorID

Description:    This routine is reads the registry string
                "HardwareID" to find the ID of the monitor.  This
                is what we used to determine if the monitor has
                changed.
Information:

Return:         INT     value from registry.
----------------------------------------------------------------------*/
int GetMonitorID(char FAR * pStr, int nSize, char FAR * pReadStr)
{
   int nReturn;

   LONG cbValue = nSize;

   *pStr = '\0';
	nReturn = CM_Read_Registry_Value(DisplayInfo.diMonitorDevNodeHandle,
							NULL,
							pReadStr,
							REG_SZ,
							(LPBYTE)pStr,
							&cbValue,
							CM_REGISTRY_HARDWARE);

   if (CR_SUCCESS == nReturn)
      pStr[cbValue] = '\0';
   else if (CR_INVALID_DEVNODE == nReturn)
      {
      // if bad devnode refresh and try again
      VDDCall(VDD_GET_DISPLAY_CONFIG, dwDeviceHandle, sizeof(DisplayInfo), 0,
              &DisplayInfo);
      dwDevNode = DisplayInfo.diDevNodeHandle;

      *pStr = '\0';
      cbValue = nSize;
	   nReturn = CM_Read_Registry_Value(DisplayInfo.diMonitorDevNodeHandle,
		   					NULL,
			   				pReadStr,
				   			REG_SZ,
					   		(LPBYTE)pStr,
						   	&cbValue,
							   CM_REGISTRY_HARDWARE);

      if (CR_SUCCESS == nReturn)
         pStr[cbValue] = '\0';
      }

   return nReturn;
}

/*----------------------------------------------------------------------
Function name:  IsValidMode

Description:    This is where we determine if we think that the
                monitor can handle the mode.
Information:

Return:         DWORD   IS_VALID_MODE for success or,
                        0 for failure.
----------------------------------------------------------------------*/
DWORD IsValidMode(LPMODEINFO lpModeInfo, int nFirst)
{
   DWORD dwReturn = 0;
	LPMONINFO lpMonRange;
	LPMONEDID lpMonEDID;
	LPMONTYPE lpMonData;
	int nFound;
   WORD nActualX;
   WORD nActualY;
   int intlaceadj = 1; // DYNAMIC MODE TABLE 


   if ( lpModeInfo->dwFlags & INTERLACED ) // DYNAMIC MODE TABLE 
   		intlaceadj = 2;

   // If the mode uses Scan Line Doubling then the mode has the 
   // dot clock cut in 1/2 and the CRT double the "Y".  This
   // has the effect of making the mode look like another mode to the
   // monitor.  The other mode is a doubling of the "X" & "Y".
   if (SCANLINE_DBL == (lpModeInfo->dwFlags & SCANLINE_DBL))
      {  
      nActualX = (WORD)(lpModeInfo->dwWidth << 1);
      nActualY = (WORD)(lpModeInfo->dwHeight << 1);
      }
   else
      {
      nActualX = (WORD)lpModeInfo->dwWidth;
      nActualY = (WORD)lpModeInfo->dwHeight;
      }

   // If we know nothing then everything is valid
   if (MONITOR_UNKNOWN == Monitor.nType)
      {
      if (MAX_REZ_IS_VALID != (MAX_REZ_IS_VALID & Monitor.IsValid))
         {
         dwReturn = 0x0;
         }
      else
         {
         nFound = 0;
         if (nFirst)
            {
      		for (lpMonData = Monitor.lpNext; (NULL != lpMonData) && (!nFound); lpMonData = lpMonData->lpNext)
	      		{
   				lpMonRange = (LPMONINFO)lpMonData;
     	      	if ((nActualX <= lpMonRange->nResX) &&
			      	 (nActualY <= lpMonRange->nResY))
				   	{
   					/* This mode will work, set it to be valid */
	   				dwReturn = IS_VALID_MODE;
   					nFound = 1;
   					}
               }
            }
         }
      }
	else if (MONITOR_MULTI_SYNC == Monitor.nType)
      {
		nFound = 0;
		for (lpMonData = Monitor.lpNext; (NULL != lpMonData) && (!nFound); lpMonData = lpMonData->lpNext)
			{

			// What type of entry is this
			if (MONINFO_REGISTRY == lpMonData->nType)
				{
				lpMonRange = (LPMONINFO)lpMonData;

            if (RANGE(lpModeInfo->wHorz, lpMonRange->rHort.nMin - HSYNC_FUDGE, lpMonRange->rHort.nMax + HSYNC_FUDGE) &&
                RANGE(lpModeInfo->wVert, ( lpMonRange->rVert.nMin / intlaceadj ) - VSYNC_FUDGE, lpMonRange->rVert.nMax + VSYNC_FUDGE) &&
     				 (nActualX <= lpMonRange->nResX) &&
				    (nActualY <= lpMonRange->nResY))
					{
					/* This mode will work, set it to be valid */
					dwReturn = IS_VALID_MODE;
					nFound = 1;
					}
				}
			else
				{
				lpMonEDID = (LPMONEDID)lpMonData;
				if ((nActualX <= lpMonEDID->nHor) &&
					 (nActualY <= lpMonEDID->nVer) &&
                 RANGE(lpModeInfo->wVert, lpMonEDID->nVRef - VSYNC_FUDGE, lpMonEDID->nVRef + VSYNC_FUDGE))
					{
					/* This mode will work, return pointer to it */
					dwReturn = IS_VALID_MODE;
					nFound = 1;
					}
				}
			}
		}
	else if (MONITOR_FIXED_FREQUENCY == Monitor.nType)
		{
		BOOL bFoundXYMatch = FALSE; // does the x, y of the mode being tested fall within the monitor's range
		nFound = 0;
		for (lpMonData = Monitor.lpNext; (NULL != lpMonData) && (!nFound); lpMonData = lpMonData->lpNext)
			{

			lpMonRange = (LPMONINFO)lpMonData;
			
			// if the monitor is a FP730 CPQ model compare with <= to fix problem with FP730 monitor
			// this is detected from the EDID product ID and manufacturer ID or from the inf file monitor name
			if (!bUseAltValidation)
				bFoundXYMatch = (nActualX == lpMonRange->nResX) && (nActualY == lpMonRange->nResY);
			else
			 	bFoundXYMatch = (nActualX <= lpMonRange->nResX) && (nActualY <= lpMonRange->nResY);


			// What type of entry is this
			if (MONINFO_REGISTRY == lpMonData->nType)
				{
				lpMonRange = (LPMONINFO)lpMonData;

            if (RANGE(lpModeInfo->wHorz, lpMonRange->rHort.nMin - HSYNC_FUDGE, lpMonRange->rHort.nMax + HSYNC_FUDGE) &&
                RANGE(lpModeInfo->wVert, ( lpMonRange->rVert.nMin / intlaceadj )  - VSYNC_FUDGE, lpMonRange->rVert.nMax + VSYNC_FUDGE) &&
		   	    bFoundXYMatch)
					{
					/* This mode will work, return pointer to it */
					dwReturn = IS_VALID_MODE;
					nFound = 1;
					}
				}
			else
				{
				lpMonEDID = (LPMONEDID)lpMonData;

				if (bFoundXYMatch &&
                RANGE(lpModeInfo->wVert, lpMonEDID->nVRef - VSYNC_FUDGE, lpMonEDID->nVRef + VSYNC_FUDGE))
					{
					/* This mode will work, return pointer to it */
					dwReturn = IS_VALID_MODE;
					nFound = 1;
					}
				}
			}
		}

   return dwReturn;
}


/*----------------------------------------------------------------------
Function name:  ConvertToMultiSync

Description:    Convert a bunch of discrete data points into a
                continuous function.
Information:	

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int ConvertToMultiSync(void)
{
   LPMONTYPE lpMonData;
   LPMONINFO lpMonInfo;
   LPMONEDID lpMonEDID;
   WORD nMinHorz=999;
   WORD nMaxHorz=0;
   WORD nMinVert=999;
   WORD nMaxVert=0;
   WORD nMaxX=0;
   WORD nMaxY=0;
   WORD nConvert=0;
   WORD nHorz;

   for (lpMonData = Monitor.lpNext; (NULL != lpMonData); lpMonData = lpMonData->lpNext)
      {
      if (MONINFO_EDID == lpMonData->nType)
         {
         lpMonEDID = (LPMONEDID)lpMonData;
         nConvert = 1;
         
         // Throw away bogus entries
         if (lpMonEDID->nHor < 640)
            continue;
         if (lpMonEDID->nVer < 300)
            continue;
         nHorz = LookUpRate(lpMonEDID->nHor, lpMonEDID->nVer, lpMonEDID->nVRef);
         nMinHorz = MIN(nMinHorz, nHorz);
         nMaxHorz = MAX(nMaxHorz, nHorz);
         nMinVert = MIN(nMinVert, lpMonEDID->nVRef);
         nMaxVert = MAX(nMaxVert, lpMonEDID->nVRef);
         nMaxX = MAX(nMaxX, lpMonEDID->nHor);
         nMaxY = MAX(nMaxY, lpMonEDID->nVer);
         }
      }

   if (1 == nConvert)
      {
      lpTrimmer = Monitor.lpNext;
      Monitor.lpNext = NULL;
      lpMonInfo = GetMonitorStruc(MONINFO_REGISTRY);
      Monitor.IsValid |= MAX_REZ_IS_VALID;
      Monitor.IsValid |= REFRESH_IS_VALID;
      Monitor.nType = MONITOR_MULTI_SYNC;
      lpMonInfo->nResX = nMaxX;
      lpMonInfo->nResY = nMaxY;
      lpMonInfo->rHort.nMin = nMinHorz;
      lpMonInfo->rHort.nMax = nMaxHorz;
	  if (bUseAltValidation)	// if we detected the Compaq FP730 monitor from the EDID data
	  {	
		lpMonInfo->rHort.nMin = COMPAQ_FP730_MIN_HREF;	  // fix for Compaq FP730 flat panel 
	  	lpMonInfo->rHort.nMax = COMPAQ_FP730_MAX_HREF;	  // the href values are taken from the monitor's inf file
	  }
      lpMonInfo->rVert.nMin = nMinVert;
      lpMonInfo->rVert.nMax = nMaxVert;
      lpMonInfo->IsValid |= Monitor.IsValid;
 	   InsertEntry(lpMonInfo);
     }

   return 0;
}


/*----------------------------------------------------------------------
Function name:  LookUpRate

Description:    Convert X,Y and VSync into a Hsync Value.
                
Information:	

Return:         INT     the Horizontal rate or,
                        -1 if failuer.
----------------------------------------------------------------------*/
int LookUpRate(WORD wX, WORD wY, WORD wVert)
{
   int i;
   WORD wHort = 0xFFFF;
   
   for (i=0; 0 != ModeList[i].dwWidth; i++)
      if ((wX == (WORD)ModeList[i].dwWidth) && (wY == (WORD)ModeList[i].dwHeight) &&
            RANGE(wVert, ModeList[i].wVert - VSYNC_FUDGE, ModeList[i].wVert + VSYNC_FUDGE))
            {
            wHort = ModeList[i].wHorz;
            break;
            } 

   if (-1 == wHort)
      wHort = (WORD)((DWORD)wY * (DWORD)wVert/(DWORD)100L);   

   // Make sure we don't go below our min or over are maximum
   // This may cause some math problems in out comparision
   // Our current min is 640x400 @ 70 or 31.2
   // Our current max is 1792x1344 @ 75 or 106.5
   wHort = MAX(200, wHort);
   wHort = MIN(3000, wHort);

   return wHort;
}


/*----------------------------------------------------------------------
Function name:  NotInEdid

Description:    This function determines if the mode is an exact
                match.
Information:

Return:         INT     TRUE for success, FALSE for failure.
----------------------------------------------------------------------*/
int NotInEdid(LPMONTYPE lpTimmer, DWORD dwOldX, DWORD dwOldY)
{
   LPMONTYPE lpMonData;
   int nReturn = TRUE;

   for (lpMonData = lpTimmer; (NULL != lpMonData) && (nReturn); lpMonData = lpMonData->lpNext)
      {
      if (((WORD)dwOldX == lpMonData->nResX) && ((WORD)dwOldY == lpMonData->nResY))
         {
         nReturn = FALSE;
         break;
         }
      }

   return nReturn;
}

/*----------------------------------------------------------------------
Function name:  MaxWidthInEdid

Description:    This function returns the highest X found in the EDID
                
Information:

Return:         INT     Largest X res dimension mentioned in EDID.
----------------------------------------------------------------------*/
DWORD MaxWidthInEdid(LPMONTYPE lpTimmer)
{
   LPMONTYPE lpMonData;
   DWORD nReturn = 640; // Assume all monitors will do at least 640

   for (lpMonData = lpTimmer; (NULL != lpMonData); lpMonData = lpMonData->lpNext)
      {
      if ( lpMonData->nResX > nReturn ) 
         nReturn = lpMonData->nResX;
      }

   return nReturn;
}

/*----------------------------------------------------------------------
Function name:  VX900T_Fixup

Description:    Adjust 640x480x75Hz for VX900T
                
Information:    This function fixes a problem with the VX900T Monitor.
 It cannot handle 75Hz and goes into some sorta of double line mode
 which look very, very bad.
                

Return:         void
----------------------------------------------------------------------*/
void VX900T_Fixup(int ModeNumber)
{
	CONFIGRET nReturn1;
   int nLen;

   // Right Mode ??
   if ((640 == ModeList[ModeNumber].dwWidth) &&
       (480 == ModeList[ModeNumber].dwHeight) &&
       (75 == ModeList[ModeNumber].wVert))
      {
      nReturn1 = CM_Get_DevNode_Key(DisplayInfo.diMonitorDevNodeHandle,
					"", (PFARVOID)&szBuffer, sizeof(szBuffer), CM_REGISTRY_HARDWARE);

      if (CR_SUCCESS == nReturn1)
         {
         nLen = strlen(szBuffer);

         if (nLen > 19)
            {
            // Right Monitor ???
            if (_fmemcmp(&szBuffer[13],"GWY00C0", 6) == 0)
               {
               // This is about 75.5 Hz and 37.7 Khz which is almost 75 Hz
               SETDW(lph3IORegs->pllCtrl0, 0x5BA0L); 
               }
            }
         }
      }

}

/*----------------------------------------------------------------------
Function name:  di_WriteRegistry

Description:    This routine writes the refresh rate string to
                the registry.
Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int di_WriteRegistry(WORD wWidth, WORD wHeight, int * pRefreshRate, int nCount)
{
CONFIGRET nReturn;
LONG cbValue;
int i;
char szModes[32];
char szRes[6];


   // Build the key
   _fstrcpy(szModes,"MODES\\8\\");
	di_DoStr(szRes, wWidth, 0);
	_fstrcat(szModes,szRes);
	szRes[0]=',';
	di_DoStr(&szRes[1], wHeight, 0);
	_fstrcat(szModes,szRes);
   
   // Build the refresh rate string
   di_DoStr(szBuffer, pRefreshRate[0], 0);
   for (i=1; i<nCount; i++)
      {
   	szRes[0]=',';
	   di_DoStr(&szRes[1], (WORD)pRefreshRate[i], 0);
	   _fstrcat(szBuffer,szRes);
      }

   // Make sure the key exists before writing else we will create
   // a new one !!!!!
   cbValue=sizeof(szRes) - 1;

   nReturn = CM_Read_Registry_Value(DisplayInfo.diDevNodeHandle,
							szModes,
							"",
							REG_SZ,
							(LPBYTE)&szRes,
							&cbValue,
							CM_REGISTRY_SOFTWARE);

   if ((CR_SUCCESS == nReturn) || (CR_BUFFER_SMALL == nReturn))
      {
      // Write the rate string
      cbValue=strlen(szBuffer);

   	CM_Write_Registry_Value(DisplayInfo.diDevNodeHandle,
	   						szModes,
		   					"",
			   				REG_SZ,
				   			(LPBYTE)&szBuffer,
					   		cbValue,
						   	CM_REGISTRY_SOFTWARE);
      }
   
   return 0;
}

/*----------------------------------------------------------------------
Function name:  di_FixupRegistryRefresh

Description:    This routine makes sure that the rates in the
                registry match the rates that we think that we can do.
Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
#define MAX_REFRESH 10
int di_FixupRegistryRefresh(void)
{
   DWORD dwOldX;
   DWORD dwOldY;
   int i;
   int nCount;
   int nHaveList = FALSE;
   int nRefreshRate[MAX_REFRESH];
   
   dwOldX = 0;
   dwOldY = 0;
   nCount = 0;
   for (i=0; 0 != ModeList[i].dwWidth; i++)
      {
      if (ModeList[i].dwFlags & IS_VALID_MODE)
         {
         if (8 == ModeList[i].dwBPP)
            {
            if ((ModeList[i].dwWidth >= 640) && (ModeList[i].dwHeight >=480))
               {
               if ((dwOldX == ModeList[i].dwWidth) && (dwOldY == ModeList[i].dwHeight))
                 {  
                 if (nCount < MAX_REFRESH)
                     nRefreshRate[nCount++] = ModeList[i].wVert;
                  }
               else
                  {
                  if (nHaveList)
                     di_WriteRegistry((WORD)dwOldX, (WORD)dwOldY, nRefreshRate, nCount);
                  nHaveList = TRUE;
                  nCount = 0;
                  dwOldX = ModeList[i].dwWidth;
                  dwOldY = ModeList[i].dwHeight;
                  nRefreshRate[nCount++] = ModeList[i].wVert;
                  }
               }
            }
         }
   
      // Only do 8 BPP
      if (ModeList[i].dwBPP > 8)
         break;
      }
   
   if (nHaveList)
      di_WriteRegistry((WORD)dwOldX, (WORD)dwOldY, nRefreshRate, nCount);

   return 0;
}

#pragma optimize("",off)

//=======================================================================================
// Function name:  di_GetTvOutMaxRes
//
// Description:    Search the mode list for TVOUT modes and fill the MaxX and MaxY values
// 				   with the highest TVOUT capable resolution depending on the   
//				   current TV standard.	
// Information:    
//
// Return:         VOID
//							
//=======================================================================================
void di_GetTvOutMaxRes( DWORD * MaxX, DWORD * MaxY, DWORD TvStandard )
{
int i;


	*MaxX = 0;
	*MaxY = 0;

	for (i = 0; ModeList[i].dwWidth; i++)
	{
    	if ( TvStandard == VP_TV_STANDARD_NTSC_M )	    
		{												
			if ( ModeList[i].dwFlags & TVOUT_NTSC )
			{
				if ( ModeList[i].dwFlags & TVOUT_DESKTOP || ModeList[i].dwFlags & TVOUT_DDRAW )
				{
					if ( ModeList[i].dwWidth > *MaxX )
						*MaxX = ModeList[i].dwWidth;
					if ( ModeList[i].dwHeight > *MaxY )
						*MaxY = ModeList[i].dwHeight;
				}
			}
		}
		else									   	   	
		{
			if ( ModeList[i].dwFlags & TVOUT_PAL )
			{
				if ( ModeList[i].dwFlags & TVOUT_DESKTOP || ModeList[i].dwFlags & TVOUT_DDRAW )
				{
					if ( ModeList[i].dwWidth > *MaxX )
						*MaxX = ModeList[i].dwWidth;
					if ( ModeList[i].dwHeight > *MaxY )
						*MaxY = ModeList[i].dwHeight;
				}
			}
		}
	}
}

//=======================================================================================
// Function name:  di_ConvertTimingToGTF
//
// Description:    Refill a Modeinfo struct with GTF timing information
//
// Information:    This function calls the minivdd to fill the timing params structure
//				   using the GTF formula.
//
// Return:         VOID
//=======================================================================================
void di_ConvertTimingToGTF( MODEINFO FAR * ModeInfo )
{
TIMING_PARAMS timingparams;

	
	timingparams.width = ModeInfo->dwWidth;
	timingparams.height = ModeInfo->dwHeight;
	timingparams.refresh = ModeInfo->wVert;
	timingparams.CharWidth = ModeInfo->CharWidth;
	timingparams.CRTCflags = DEFAULT_CRTC_FLAGS;

	if (  ModeInfo->dwFlags & SCANLINE_DBL  )		// If Double Scanned mode
	   	timingparams.CRTCflags |= 1;
			 
	VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
	    	H3VDD_GET_GTF_TIMINGS, 0, &timingparams);
				
	// Transfer timingparam info to ModeInfo structure
	ModeInfo->wHorz = (WORD)(( timingparams.PixelClock / 100 ) / timingparams.HTotal );
	ModeInfo->HTotal = timingparams.HTotal;
	ModeInfo->HSyncStart = timingparams.HSyncStart;
	ModeInfo->HSyncEnd = timingparams.HSyncEnd;
	ModeInfo->VTotal =  timingparams.VTotal;
	ModeInfo->VSyncStart = timingparams.VSyncStart;
	ModeInfo->VSyncEnd = timingparams.VSyncEnd;
	ModeInfo->CRTCflags = timingparams.CRTCflags;
	ModeInfo->PixelClock = timingparams.PixelClock;
   	ModeInfo->UseGTF = TRUE;
}

//=======================================================================================
// Function name:  di_ValidateTvOutModeList
//
// Description:    Validate entries in the primary Mode List structure according to
// 				   what the DFP can do.	These validations affect what modes
//				   eventually appear in Windows and Direct Draw.	
// Information:    
//
// Return:         BOOL		TRUE - Pass
//							FALSE - Fail
//=======================================================================================
BOOL di_ValidateTvOutModeList(void)
{
int i;
TVSETSTANDARD Std;
LPTVGETSTANDARD lpStd = (LPTVGETSTANDARD)&Std;


	Std.dwSubFunc = QUERYGETSTANDARD;
	TVOutGetStandard((LPQIN)&Std, lpStd);  	// Initialize dwTvoStd variable!

	strcpy (Monitor.szHardwareID, TVO_HDWE_NAME);
	strcpy (Monitor.szDeviceID, TVO_DEV_NAME);

	for (i = 0; ModeList[i].dwWidth; i++)
	{
      	ModeList[i].dwFlags &= ~IS_VALID_MODE;			// Start by assuming mode is invalid
        ModeList[i].dwFlags &= ~IS_DDRAW_MODE;	  		// Invalidate ALL DDRAW modes

    	if ( _FF(dwTvoStd) == VP_TV_STANDARD_NTSC_M )	// Validate NTSC modes if NTSC is detected
		{												// Add new NTSC types to this if statement
			if ( ModeList[i].dwFlags & TVOUT_NTSC )
			{
				if ( ModeList[i].dwFlags & TVOUT_DESKTOP )
					 ModeList[i].dwFlags |= IS_VALID_MODE;
				if ( ModeList[i].dwFlags & TVOUT_DDRAW )	// Validate TVOUT DDRAW only modes
				{
					 ModeList[i].dwFlags |= IS_DDRAW_MODE;   
					 ModeList[i].dwFlags |= IS_VALID_MODE;
				}
			}
		}
		else									   	   	// Validate PAL modes if PAL is detected
		{
			if ( ModeList[i].dwFlags & TVOUT_PAL )
			{
				if ( ModeList[i].dwFlags & TVOUT_DESKTOP )
			   		 ModeList[i].dwFlags |= IS_VALID_MODE;
				if ( ModeList[i].dwFlags & TVOUT_DDRAW )	// Validate TVOUT DDRAW only modes
				{
					 ModeList[i].dwFlags |= IS_DDRAW_MODE;   
			   		 ModeList[i].dwFlags |= IS_VALID_MODE;
				}
			}
		}
	}

	return TRUE;
}

//=======================================================================================
// Function name:  di_ValidateAndFixupDfpModeList
//
// Description:    Adds centered modes to the mode list. Centered modes are added for
//                 monitors which support at least one mode at a given refresh rate. This
//                 function overwrites the centered mode's parameters with those of the
//                 reference mode. The minivdd then then uses the relative widths and
//                 heights to modify the reference mode's parameters to achieve
//                 centering.
//
// Information:    
//
// Return:         void
//=======================================================================================
#define MAX_DFP_REF 10

void di_ValidateAndFixupDfpModeList(void)
{
    DWORD i, j, ref;
    DWORD RefTable[MAX_DFP_REF][2];

    for(i=0; i<MAX_DFP_REF; i++)                        //Zero the refresh rate table
        RefTable[i][0] = 0;

    for(i=(nNumModes-1); i<(DWORD)nNumModes; i--)       //Go through the mode list backwards
    {
        if((ModeList[i].dwFlags & IS_VALID_MODE) &&     //When we find a valid full mode
           (ModeList[i].dwWidth >= 640))
        {
            ModeList[i].dwFlags |= IS_DDRAW_MODE;

            for(j=0; j<MAX_DFP_REF; j++)                //Go through the refresh rate table
            {
                                                        //Overwrite the previous index for that ref rate
                if(RefTable[j][0] == ModeList[i].wVert)
                {
                    RefTable[j][1] = i;
                    break;
                }
                else if(RefTable[j][0] == 0)
                {
                                                        //Or enter the index into a free slot
                    RefTable[j][0] = ModeList[i].wVert;
                    RefTable[j][1] = i;
                    break;
                }
            }
        }
        else                                            //When we find an invalid mode
        {
            WORD bDDraw = (ModeList[i].dwFlags & IS_DDRAW_MODE) ? 1 : 0;

            ModeList[i].dwFlags &= ~IS_DDRAW_MODE;

            for(j=0; j<MAX_DFP_REF; j++)                //Go through the refrsh rate table
            {
                if(RefTable[j][0] == ModeList[i].wVert) //If we find a matching index
                {
                    ref = RefTable[j][1];
                                                        //and the mode is smaller than the reference mode and the bpps match
                    if((ModeList[i].dwWidth  <= ModeList[ref].dwWidth) &&
                       (ModeList[i].dwHeight <= ModeList[ref].dwHeight) &&
                       (ModeList[i].dwBPP    == ModeList[ref].dwBPP))
                    {
                        //Overwrite the mode's parameters with those of the reference mode
                        ModeList[i].dwDFPWidth  = ModeList[ref].dwWidth;
                        ModeList[i].dwDFPHeight = ModeList[ref].dwHeight;
                        ModeList[i].HTotal      = ModeList[ref].HTotal;
                        ModeList[i].HSyncStart  = ModeList[ref].HSyncStart;
                        ModeList[i].HSyncEnd    = ModeList[ref].HSyncEnd;
                        ModeList[i].VTotal      = ModeList[ref].VTotal;
                        ModeList[i].VSyncStart  = ModeList[ref].VSyncStart;
                        ModeList[i].VSyncEnd    = ModeList[ref].VSyncEnd;
                        ModeList[i].CRTCflags   = ModeList[ref].CRTCflags;
                        ModeList[i].PixelClock  = ModeList[ref].PixelClock;
                        ModeList[i].CharWidth   = ModeList[ref].CharWidth;

                        //Remove the scanline double bit
                        ModeList[i].dwFlags &= ~SCANLINE_DBL;

                        if(bDDraw)
                            ModeList[i].dwFlags |= IS_DDRAW_MODE;

                        ModeList[i].dwFlags |= IS_VALID_MODE;
                    }

                    break;
                }
                else if(RefTable[j][0] == 0)            //If we reach the end of the list
                    break;
            }
        }
    }
}

//=======================================================================================
// Function name:  di_ValidateModeList
//
// Description:    Validate entries in the primary Mode List structure according to
// 				   what the monitor/TV/DFP and hardware can do.	These validations
//				   affect what modes eventually appear in Windows and Direct Draw.	
// Information:    
//
// Return:         INT	
//=======================================================================================
int di_ValidateModeList(void)
{
DWORD dwOldX;
DWORD dwOldY;
LPMONINFO lpMonRange;
WORD wMonType;
int nFirst;
int i;
int j;
int nCount;
int nTrim;
char szNewId[32];
char szNewDeviceId[64];
 
	  
   	if (_FF(dwTvoActive))	// Validate mode list for TV OUT and return;
   	{
 	   	if ( di_ValidateTvOutModeList() )
		{
	   		di_FixupRegistryRefresh();	// Retouch available refreshes in the registry
	   		return ( 1 );
		}
   	}

	if (DFPisPanelActive())
   	{
		// Get current DFP ID info.
        DFPgetUniqueName(szNewId);
	    _fstrcpy (szNewDeviceId, LCD_DEV_NAME);
   	}
   	else
   	{
		// Get current CRT monitor ID info.
       	GetMonitorID(szNewId, sizeof(szNewId) - 1, "HardwareID");
       	GetMonitorID(szNewDeviceId, sizeof(szNewDeviceId) - 1, "DeviceDesc");
   	}

   	// If the monitor has not changed then just return
   	if ((_fstrcmp( szNewId, Monitor.szHardwareID ) == 0) &&
       	(_fstrcmp( szNewDeviceId, Monitor.szDeviceID ) == 0))
      	return( 1 );

   	// Update IDs
   	_fstrcpy(Monitor.szHardwareID, szNewId);
   	_fstrcpy(Monitor.szDeviceID, szNewDeviceId);

	bUseAltValidation = FALSE;
	// if we are dealing with the Compaq FP730 then change the validation method
	if (_fstrcmp(szNewId, COMPAQ_FP730_ID) == 0)
	  	if (_fstrcmp(szNewDeviceId, COMPAQ_FP730_DEVICEID) == 0)
			bUseAltValidation = TRUE;

   	DoEDID();  // Fill the monitor information structure

    // Assuming device has changed, for example TvOut to CRT, or one type of DFP to a different type of DFP.
    // The mode tables need to be regenerated.
   	di_FillModeTable( ModeList, nNumModes );		// Parse registry & fill ModeList
   	di_SortModeTable( ModeList, nNumModes );		// Sort the ModeList

    // This code is to fixup the "Standard Microsoft Monitors for Win '98 which are bogus"
   	for (i=0; i<sizeof(FixUpIds)/sizeof(char *); i++)
    {
      	if (_fstrcmp(FixUpIds[i], Monitor.szHardwareID) == 0)
        {
         	for (lpMonRange = (LPMONINFO)Monitor.lpNext; NULL != lpMonRange; lpMonRange = lpMonRange->lpNext)
            {
            	for (j=0; j<sizeof(FixUpValue)/sizeof(WORD); j+=2)
               	{
               		if (lpMonRange->nResX == FixUpValue[j])
                	{
                  		lpMonRange->rHort.nMin = lpMonRange->rHort.nMin - FixUpValue[j+1];
                  		lpMonRange->rHort.nMax = lpMonRange->rHort.nMax - FixUpValue[j+1];
                  		break;
                  	}
               	}
        	}         
        break;
    	}
    }

    // This code is to fixup the Iiyama A701GT Monitor ranges
   	for (i=0; i<sizeof(FixUpId2)/sizeof(char *); i++)
   	{
      if (_fstrcmp(FixUpId2[i], Monitor.szHardwareID) == 0)
         {
         for (lpMonRange = (LPMONINFO)Monitor.lpNext; NULL != lpMonRange; lpMonRange = lpMonRange->lpNext)
            {
            lpMonRange->rHort.nMin = lpMonRange->rHort.nMin - FixUpValue2[i*2];
            lpMonRange->rHort.nMax = lpMonRange->rHort.nMax - FixUpValue2[i*2+1];
            }         
         break;
   		}
   	}

   	// Override the Unknown Monitor to support 8x6 and 6x4
   	if (_fstrcmp(Monitor.szDeviceID,"(Unknown Monitor)") == 0)
    {
    	if (NULL != Monitor.lpNext)
        {
        	Monitor.lpNext->nResX = 800;
         	Monitor.lpNext->nResY = 600;
        }
    }

    for (i=0; 0 != ModeList[i].dwWidth; i++)	  // Start by invalidating ALL modes
      	ModeList[i].dwFlags &= ~IS_VALID_MODE;

	if (DFPisPanelActive())
    {
        wMonType = 0x01;  // this has the affect of allowing ConvertToMultiSync to be called for DFPs
    }
   	else
   		GetBinary(DisplayInfo.diDevNodeHandle, &wMonType, "MonitorType", 0x1);

   	// Here is where we Decide if we should try this monitor as a Multi-Sync
   	// or a Fixed Frequency
   	if ((0x01 == wMonType) && (MONITOR_FIXED_FREQUENCY == Monitor.nType))
      	ConvertToMultiSync();
   
   	dwOldX = 0;
   	dwOldY = 0;
   	for (i=0; 0 != ModeList[i].dwWidth; i++)
    {
		if (((ModeList[i].dwWidth != dwOldX) ||
              (ModeList[i].dwHeight != dwOldY)) &&
			  (ModeList[i].wVert >= 60))
        {
            dwOldX = ModeList[i].dwWidth;
            dwOldY = ModeList[i].dwHeight;
            nFirst = TRUE;
        }
        else
        	nFirst = FALSE;

         // If this mode does not work on a V3 then just continue
         //if ((IS_VOODOO3) && (ModeList[i].dwFlags & FAILS_ON_VOODOO3))
         //   continue;

        ModeList[i].dwFlags |= IsValidMode(&ModeList[i], nFirst);
    }

   	// Trim the modelist down if we are doing a EDID expand to better match	Windows 
   	if (NULL != lpTrimmer)
    {        
    	dwOldX = 0;
      	dwOldY = 0;
      	for (i=0; 0 != ModeList[i].dwWidth; i++)
        {
        	if ((ModeList[i].dwWidth != dwOldX) ||
                (ModeList[i].dwHeight != dwOldY))
            {
               	dwOldX = ModeList[i].dwWidth;
               	dwOldY = ModeList[i].dwHeight;
               	if ((dwOldX >= 640) && (dwOldY >=480))
                  	nTrim = NotInEdid(lpTrimmer, dwOldX, dwOldY);
               	else
                  	nTrim = FALSE;
            }
            if (nTrim)
			{
				// Some EDIDs may not list modes that are between the  
				// lowest and the highest resolutions, so let's keep
				// those modes from disappearing from user selection.
                // Don't do above special case if it's a DFP.
                if ( dwOldX > MaxWidthInEdid(lpTrimmer) || DFPisPanelActive())
                  		ModeList[i].dwFlags &= ~IS_VALID_MODE;
			}
    	}
    } 

   	// If the HARDWARE cannot do a mode or refresh, make sure that it does 
   	// not show up in the selection list.
	for (i=0; 0 != ModeList[i].dwWidth; i++)
    {
		if ( HWTestMode(i) == FALSE )	
   			ModeList[i].dwFlags &= ~IS_VALID_MODE;
    }

	// If a genuine GTF monitor was detected, assume the use of GTF timings for ALL modes
	for (i=0; 0 != ModeList[i].dwWidth; i++)
    {
		if ( MonitorIsGTF == TRUE && ModeList[i].UseGTF == FALSE )	
		{
			di_ConvertTimingToGTF( &ModeList[i] );
		}
    }


	// if the DFP is active fix up modes with timing information from the EDID
	if (DFPisPanelActive())
	{
	    for (i=0; 0 != ModeList[i].dwWidth; i++)
        {
            UpdateModeFromEdid( &ModeList[i] );
		}

        // If the dfp is not scaling fix up the mode list to add centered modes
	    if (DFPisPanelActive() && !DFPisPanelScaling())
            di_ValidateAndFixupDfpModeList();
    }

   	// Count the number of valid modes
   	nCount=0;   
   	for (i=0; 0 != ModeList[i].dwWidth; i++)
    {
        if ((ModeList[i].dwWidth >= 640) &&
            (ModeList[i].dwHeight >= 480) &&
            (ModeList[i].dwFlags & IS_VALID_MODE))
        {
        	nCount++;
    	}
    } 

   	// If all modes are invalid make some of them valid
   	// Find the 640x480 & 800x600 modes at 60Hz and make them valid at all Pixel Depths
   	if (0 == nCount)
    {
        for (i=0; 0 != ModeList[i].dwWidth; i++)
        {
            if (( ModeList[i].dwWidth == 640 ) && 
                ( ModeList[i].dwHeight == 480 ) && 
                ( ModeList[i].wVert == 60 ))
            {
            	ModeList[i].dwFlags |= IS_VALID_MODE;
            }
            if (( ModeList[i].dwWidth == 800 ) && 
                ( ModeList[i].dwHeight == 600 ) && 
                ( ModeList[i].wVert == 60 ))
            {
            	ModeList[i].dwFlags |= IS_VALID_MODE;
            }
   		}
    }

   	// Free this stuff cause we no longer need it!!!!!
   	FreeMon(Monitor.lpNext);
   	Monitor.lpNext = NULL;   

   	if (NULL != lpTrimmer)
    {
    	FreeMon(lpTrimmer);
      	lpTrimmer = NULL;
    }

   	di_FixupRegistryRefresh();	// Retouch available refreshes in the registry

   	return 1;
}

#pragma optimize("",on)
