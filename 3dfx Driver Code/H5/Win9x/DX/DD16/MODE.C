/* $Header: mode.c, 12, 10/11/00 8:51:27 PM, Brent$ */
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
** File name:   mode.c
**
** Description: Misc support functions for modes.
**
** $Revision: 12$ 
** $Date: 10/11/00 8:51:27 PM$
**
** $History: mode.c $
** 
** *****************  Version 4  *****************
** User: Cwilcox      Date: 1/25/99    Time: 11:37a
** Updated in $/devel/h3/Win95/dx/dd16
** Minor modifications to remove compiler warnings.
** 
** *****************  Version 3  *****************
** User: Michael      Date: 12/29/98   Time: 2:35p
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB unified header.
** 
** 2     5/01/98 10:50a Andrew
** Added function GetOverRide to read override flag.
** 
** 1     4/22/98 2:49p Andrew
** Refresh rate support routines
** 
*/

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

#include "modelist.h"
#include "monitor.h"
#include "dfpapi.h"


extern DISPLAYINFO DisplayInfo;
extern DWORD dwDeviceHandle;
extern DWORD dwDevNode;
extern MONITOR Monitor;
extern REF_DATA Ref_Data[MAX_REF_DATA];
extern int nCount;
extern BOOL bUseEDID;

MODEINFO FAR * ModeList;
int nNumModes = 0;

int fatoi(char FAR * lpStr);
int atoi( char * pStr );
long atol( char * pStr );
DWORD atoul( char * pStr );

extern void DoEDID(void);
extern WORD GetMaxEDID_Refresh( WORD wWidth, WORD wHeight );
extern int di_ValidateModeList(void);
BOOL di_GetRegistryOptRefreshLimit( int * OptRefreshLim, int * OptNonEDIDLim ); //DYNAMIC MODE TABLE

//=======================================================================================
// Function name:  ds_CalculatePitch
//
// Description:    Device specific function for calculating the pitch at a given bit depth
//				
// Information:    
//
// Return:         DWORD 	Calculated pitch value
//
//=======================================================================================
DWORD ds_CalculatePitch( DWORD Width, DWORD Bpp )
{
DWORD pitch;


	pitch =	Width;

	if ( pitch == 2046 )	// Pitch of 2046 modes is really 2048
		pitch += 2;
	
	switch ( Bpp )
	{
		case 16:
			pitch *= 2;
			pitch += ( pitch % 128 );
			break;
		case 24:
			pitch *= 3;
			break;
		case 32:
			pitch *= 4;
			break;
	}

	return ( pitch );
}

//=======================================================================================
// Function name:  di_DoStr
// 
// Description:    Cheesy atoi for X,Y & BPP.
// 
// Information:
// 
// Return:         INT     0 is always returned.
//=======================================================================================

int nDim[] = {1000, 100, 10, 1};
int di_DoStr(char FAR * pStr, int nNum, int nStart)
{
	int i;
	int nDiv;
   BOOL bKeepZero = FALSE;

	for (i=nStart; i < sizeof(nDim)/sizeof(int); i++)
		{
		nDiv = nNum / nDim[i];
		nNum = nNum - nDiv * nDim[i];
		if (FALSE == bKeepZero)
			{
			if (nDiv)
            {
            bKeepZero = TRUE;
				*pStr++ = '0' + nDiv;
            }
			}
		else
			*pStr++ = '0' + nDiv;
		}

	*pStr++ = '\0';

	return 0;
}

//=======================================================================================
// Function name:  	di_xtoa
//
// Description:    	Converts an long to a character string. This function is called by
//				   	di_itoa(), di_ultoa(), and di_itoa().
// Information:    
//       			val - number to be converted (int, long or unsigned long)
//       			int radix - base to convert into
//       			char *buf - ptr to buffer to place result
//
// Return:         	VOID
//
//=======================================================================================
void di_xtoa ( unsigned long val, char *buf, unsigned radix, int is_neg	)
{
char *p;                // pointer to traverse string 
char *firstdig;         // pointer to first digit 
char temp;              // temp char 
unsigned digval;        // value of digit 


    p = buf;

    if (is_neg) 
    {
        // negative, so output '-' and negate 
        *p++ = '-';
        val = (unsigned long)(-(long)val);
    }

    firstdig = p;           // save pointer to first digit 

    do 
    {
        digval = (unsigned) (val % radix);
        val /= radix;       // get next digit 

        // convert to ascii and store 
        if (digval > 9)
            *p++ = (char) (digval - 10 + 'a');  // a letter 
        else
            *p++ = (char) (digval + '0');       // a digit 
    } while (val > 0);

        // We now have the digit of the number in the buffer, but in reverse
        //   order.  Thus we reverse them now. 

    *p-- = '\0';            // terminate string; p points to last digit 

    do 
    {
        temp = *p;
        *p = *firstdig;
        *firstdig = temp;   // swap *p and *firstdig 
        --p;
        ++firstdig;         // advance to next two digits 
    } while (firstdig < p); // repeat until halfway 
}

//=======================================================================================
// Function name:  	di_itoa
//
// Description:    	Converts an int to a character string. This function calls 
//					di_xtoa() with neg flag set correctly.
// Information:    
//       			val - number to be converted (integer )
//       			int radix - base to convert into
//       			char *buf - ptr to buffer to place result
//
// Return:         	VOID
//
//=======================================================================================
char * di_itoa ( int val, char *buf, int radix )
{
    if (radix == 10 && val < 0)
        di_xtoa((unsigned long)val, buf, radix, 1);
    else
        di_xtoa((unsigned long)(unsigned int)val, buf, radix, 0);
    return buf;
}

//=======================================================================================
// Function name:  	di_ltoa
//
// Description:    	Converts an long to a character string. This function calls 
//					di_xtoa() with neg flag set correctly.
// Information:    
//       			val - number to be converted ( long )
//       			int radix - base to convert into
//       			char *buf - ptr to buffer to place result
//
// Return:         	VOID
//
//=======================================================================================
char * di_ltoa ( long val, char *buf, int radix )
{
	di_xtoa(( unsigned long )val, buf, radix, (radix == 10 && val < 0));
    return buf;
}

//=======================================================================================
// Function name:  	di_ultoa
//
// Description:    	Converts an unsigned long to a character string. This function calls 
//					di_xtoa() with neg flag set correctly.
// Information:    
//       			val - number to be converted ( unsigned long )
//       			int radix - base to convert into
//       			char *buf - ptr to buffer to place result
//
// Return:         	VOID
//
//=======================================================================================
char * di_ultoa ( unsigned long val, char *buf,	int radix )
{
    di_xtoa(val, buf, radix, 0);
    return buf;
}

//=======================================================================================
// The following tables are used for conversion of ASCII chars to numeric values 
//=======================================================================================

unsigned char HexCovertTable[] =	{ 
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 0
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 16
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 32
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,255,255,255,255,255,255,		// 48
255, 10, 11, 12, 13, 14, 15,255,255,255,255,255,255,255,255,255,		// 64
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 80
255, 10, 11, 12, 13, 14, 15,255,255,255,255,255,255,255,255,255,		// 96
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 112
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 128
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 144
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 160
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 176
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 192
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 208
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 224
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255			// 240
};

DWORD HexMultiplyTable[] = {
0x1,
0x10,
0x100,
0x1000,
0x10000,
0x100000,
0x1000000,
0x10000000	
};

// #pragma optimize("",off)

//=======================================================================================
// Function name:  	di_atox
//
// Description:    	Converts a character string value to a ulong. This function also determines
//	 				if the passed string value is in hex format, and returns a numeric ulong  
// 					value. If the string value is not preceeded by "0x", then this function
//					simply returns the result of a call to the standard library function atol().
// Information:    
//       			char *buf - ptr to buffer that holds the string value.
//
// Return:         	ulong value interpreted from the character string.
//
//=======================================================================================
long di_atox( char *buf )
{
DWORD retval = 0;
DWORD mult;
int i;
int j;


	// Search through non-numerical chars until we reach one we do recognize.
	for ( i = 0; HexCovertTable[buf[i]] == 255; i++ )  
	{								    
		if ( buf[i] == 0 )
			return( retval );
	}

	// If we find a "0x" in front of a string, we assume it is in HEX format.
	if ( buf[i] == '0' && ( buf[i+1] == 'x' || buf[i+1] == 'X') )
		i += 2;
	else
		return( atol( buf ) );	// Call the standard library atol function 

	// Search through numerical chars until we reach one that is non numerical.
	for ( j = i; HexCovertTable[buf[j]] != 255; j++ )  
		;

	// Limit the number of characters read to 8 so we do not overflow a dword and crash.
	if ( ( j - i ) > 8 )
		j = i + 8;
			
	for ( j--; i <= j; i++ )
	{
		mult = (DWORD)HexCovertTable[buf[i]]; 
		retval += ( HexMultiplyTable[j-i] * mult );	
	}

    return ( retval );
}

//=======================================================================================
// Function name:  	di_atoi
//
// Description:    	Converts a character string value to a integer. This function calls 
//					the main worker function di_atox(). 
// Information:    
//       			char *buf - ptr to buffer that holds the string value.
//
// Return:         	integer value interpreted from the character string.
//
//=======================================================================================
int di_atoi( char *buf )
{
	return( (int)di_atox( buf ) );
}

//=======================================================================================
// Function name:  	di_atol
//
// Description:    	Converts a character string value to a long. This function calls 
//					the main worker function di_atox(). 
// Information:    
//       			char *buf - ptr to buffer that holds the string value.
//
// Return:         	long value interpreted from the character string.
//
//=======================================================================================
long di_atol( char *buf )
{
	return( (long)di_atox( buf ) );
}

//=======================================================================================
// Function name:  	di_atoul
//
// Description:    	Converts a character string value to a ulong. This function calls 
//					the main worker function di_atox(). 
// Information:    
//       			char *buf - ptr to buffer that holds the string value.
//
// Return:         	ulong value interpreted from the character string.
//
//=======================================================================================
DWORD di_atoul( char *buf )
{
	return( di_atox( buf ) );
}

//=======================================================================================
// Function name:  di_GetRefreshRate
//
// Description:    Reads RefreshRate from the registry.  Looks in two
//                 places:  Modes\BPP\X,Y & Default.  Skips if
//                 dwDevNode is zero which is just a call from validate.
// Information:
// 
// Return:         INT     the value of the refresh rate.
//=======================================================================================
int di_GetRefreshRate(int nFlag, int nX, int nY, int nBPP)
{
int nRefreshRate = ADAPTER_DEFAULT;
LONG cbValue;
char szModes[32];
char szResult[32];
char szRes[6];


  if (0 == dwDevNode)
   return nRefreshRate;

  _fstrcpy(szModes,"MODES\\8\\");

  // this will put 15,16,24,32 bpp into the Mode Lookup
  if (8 != nBPP)
	  {
	  di_DoStr(szRes, nBPP, 2);
	  szRes[2]='\\';
	  szRes[3]='\0';
	  // Replace old BPP with new
	  _fstrcpy(&szModes[6], szRes);
	  }

	di_DoStr(szRes, nX, 0);
	_fstrcat(szModes,szRes);
	szRes[0]=',';
	di_DoStr(&szRes[1], nY, 0);
	_fstrcat(szModes,szRes);

	cbValue = sizeof(szResult);
	if (CM_Read_Registry_Value(DisplayInfo.diDevNodeHandle,
							szModes,
							"RefreshRate",
							REG_SZ,
							(LPBYTE)&szResult,
							&cbValue,
							CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
         {
         nRefreshRate = fatoi(szResult);
         }
	 else if (CM_Read_Registry_Value(DisplayInfo.diDevNodeHandle,
							"DEFAULT",
							"RefreshRate",
							REG_SZ,
							(LPBYTE)&szResult,
							&cbValue,
							CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
         {
		   nRefreshRate = fatoi(szResult);
         }
      
   di_ValidateModeList();  	 

   // If the monitor is unknown then OPTIMAL should be ADAPTER_DEFAULT
   if ((OPTIMAL == nRefreshRate) && (MONITOR_UNKNOWN == Monitor.nType))
      nRefreshRate = ADAPTER_DEFAULT;

   return nRefreshRate;
}

//=======================================================================================
// Function name:  di_GetOverRide
//
// Description:    Reads OverRide from the registry. Looks in at
//                 Modes\BPP\X,Y.  Skips if dwDevNode is zero.
// Information:

// 
// Return:         INT     Override value
//=======================================================================================
int di_GetOverRide(int nX, int nY, int nBPP)
{
  int nOverRide = 0x0;
  LONG cbValue;
char szModes[32];
char szResult[32];
char szRes[6];


  if (0 == dwDevNode)
   return nOverRide;

  _fstrcpy(szModes,"MODES\\8\\");

  // this will put 15,16,24,32 bpp into the Mode Lookup
  if (8 != nBPP)
	  {
	  di_DoStr(szRes, nBPP, 2);
	  szRes[2]='\\';
	  szRes[3]='\0';
	  // Replace old BPP with new
	  _fstrcpy(&szModes[6], szRes);
	  }

	di_DoStr(szRes, nX, 0);
	_fstrcat(szModes,szRes);
	szRes[0]=',';
	di_DoStr(&szRes[1], nY, 0);
	_fstrcat(szModes,szRes);

	cbValue = sizeof(szResult);
	if (CM_Read_Registry_Value(DisplayInfo.diDevNodeHandle,
							szModes,
							"OverRide",
							REG_SZ,
							(LPBYTE)&szResult,
							&cbValue,
							CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
         nOverRide = fatoi(szResult);
      
   return nOverRide;
}

//=======================================================================================
// Function name:  di_FindMax
// 
// Description:    Search through the mode table and find a mode that
//                 matches.  Call the HWTestMode function in setmode.c
//                 to make sure the mode is valid.
// Information:	   Modified by jmccartney to select an optimal mode defined in the EDID,
//				   if possible.
// 
// Return:         INT     Mode number (0-n) if a valid mode was found or,
//                        -1 if no valid mode was found.
//=======================================================================================
int di_FindMax(int i, DWORD dwWidth, DWORD dwHeight, DWORD dwBPP)
{
int nOldPossible = -1,nPossible = -1, nNoEDIDMode = -1, nMatchRef = 0;
//WORD MaxEDID_Refresh;
int RegistryRefreshLimit; 			
int RegistryNonEDIDLimit; 
BOOL bLocal_UseEDID = bUseEDID;	 // do modify bUseEDID in this func; copy its value into a local var
int j = 0;
WORD refRates[MAX_REF_DATA], wTmpVRef;
DWORD dwTmpWidth, dwTmpHeight;
REF_DATA Ref_Cache;

	di_GetRegistryOptRefreshLimit( &RegistryRefreshLimit, &RegistryNonEDIDLimit );  // DYNAMIC MODE TABLE

	// are we going to use the EDID data
	if (bLocal_UseEDID)
	{	// find for this mode all the refresh rates in the EDID mode list
		dwTmpWidth  = ModeList[i].dwWidth;
		dwTmpHeight = ModeList[i].dwHeight;	
	
		while ( (j < nCount) && (j < MAX_REF_DATA) )
		{
			Ref_Cache = Ref_Data[j];
			if ( (dwTmpWidth  == (DWORD)Ref_Cache.nHor) &&
				 (dwTmpHeight == (DWORD)Ref_Cache.nVer) )
			{ // if the mode from the EDID matches store its vertical refresh rate in the array refRates
				refRates[nMatchRef] = Ref_Cache.nRef;
				nMatchRef++;
			}
			j++;
		}
		
		if (!nMatchRef)	  // if none match set bLocal_UseEDID to FALSE so that the code later on will be skipped
			bLocal_UseEDID = FALSE;
	}

    for (; ModeList[i].dwWidth == dwWidth &&
           ModeList[i].dwHeight == dwHeight &&
           ModeList[i].dwBPP == dwBPP; i++)
   	{
        if ((IS_VALID_MODE == (ModeList[i].dwFlags & IS_VALID_MODE)) &&
        	HWTestMode(i))
		{
			// are we going to use the EDID data
			if (bLocal_UseEDID)
			{	// check if the current ModeList mode matches any of the refresh rate(s) of the mode we pulled from the EDID
				wTmpVRef = ModeList[i].wVert;
											   // the rates are sorted so if we aren't going to find it any more don't go round the loop
				for (j=0; (j < nMatchRef) && (wTmpVRef >= refRates[j]); j++)
					if (wTmpVRef == refRates[j])
					{
						nPossible = i;	  // we have found a mode in the EDID that matches
						break;			  // don't bother going through the rest of the refresh rates
					}
			}
			nNoEDIDMode = i;	   		  // update this incase we can't find a match or we aren't using the EDID
		}

		// if the Vert Refresh of this mode is equal to the limit break out of the for loop
		if (ModeList[i].wVert == (WORD)RegistryRefreshLimit)
			break;
		else if (ModeList[i].wVert > (WORD)RegistryRefreshLimit)		   // if the Vert Refresh of this mode is greater than the limit break out
		{																   // but first set nPossible to the old value from the prev mode
			nPossible = nOldPossible;
			break;
		} 
		nOldPossible = (-1 == nPossible) ? nNoEDIDMode : nPossible;

    }
	// couldn't find a mode with a matching description in the EDID	or we aren't using the EDID
	// therefore just return the highest valid refresh rate mode for this resolution (maybe none ie -1)
	if (-1 == nPossible)
		return nNoEDIDMode;

   return nPossible;
}

//=======================================================================================
// Function name:  di_FindClosest
// 
// Description:    Search through the mode table and find a mode that
//                 matches.  Call the HWTestMode function in setmode.c
//                 to make sure the mode is valid.
// Information:
// 
// Return:         INT     Mode number (0-n) if a valid mode was found or,
//                         -1 if no valid mode was found.
//=======================================================================================
int di_FindClosest(int i, DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, int nRefreshRate)
{
   int nPossible = -1;
   int nDiff;
   int nOldDiff = 999;
   int nOverRide = di_GetOverRide((int)dwWidth, (int)dwHeight, (int)dwBPP);

   for (; ModeList[i].dwWidth == dwWidth &&
                 ModeList[i].dwHeight == dwHeight &&
                 ModeList[i].dwBPP == dwBPP; i++)
         {
            if ((WORD)nRefreshRate > ModeList[i].wVert)
               nDiff = nRefreshRate - ModeList[i].wVert;
            else
               nDiff = ModeList[i].wVert - nRefreshRate;

            if ((1 == nOverRide) || (IS_VALID_MODE == (ModeList[i].dwFlags & IS_VALID_MODE)))
               {
               if (nDiff < nOldDiff)
                  {
                  if (HWTestMode(i))
                     {
                     nPossible = i;
                     nOldDiff = nDiff;
                     }
                  }
               }
         }

   return nPossible;
}

//=======================================================================================
// Function name:  di_FindMode
// 
// Description:    Search through the mode table and find a mode that
//                 matches.  Call the HWTestMode function in setmode.c
//                 to make sure the mode is valid.
// Information:
// 
// Return:         INT     Mode number (0-n) if a valid mode was found or,
//                        -1 if no valid mode was found.
//=======================================================================================
int di_FindMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwFlags)
{
    int nRefreshRate = di_GetRefreshRate((WORD)dwFlags, (WORD)dwWidth, (WORD)dwHeight, (WORD)dwBPP);
    int i;

    for (i=0; ModeList[i].dwWidth!=0; i++)
    {
        if (ModeList[i].dwWidth == dwWidth &&
            ModeList[i].dwHeight == dwHeight &&
            ModeList[i].dwBPP == dwBPP)
            {
            // Three way case here:
            // 1.) Default -- Get First
            // 2.) Optimal -- Get Last
            // 3.) Rate -- Get Closest
            if (ADAPTER_DEFAULT == nRefreshRate)
               {
               if ((ModeList[i].dwFlags & IS_VALID_MODE) && (ModeList[i].wVert >= 60) && HWTestMode(i))
                  return i;
               }
            else if (OPTIMAL == nRefreshRate)
               return di_FindMax(i, dwWidth, dwHeight, dwBPP);
            else
               return di_FindClosest(i, dwWidth, dwHeight, dwBPP, nRefreshRate);
            }
    }

    return -1;
}

//=======================================================================================
static defaultDFPtiming[CRTC_TABLE_SIZE] = {
0xa3,0x4f,0x60,0x8c,0x68,0x1c,0x20,0xb3,0x60,0x70,0x01,0xdf,0x67,0x88,0x20,0x00,0xcf,0x21,0x29,0x6b,0x00
};
//=======================================================================================

//=======================================================================================
// The baseline timings contain VESA and Industry Standard timing values.
//=======================================================================================

TIMING_PARAMS baseline_timings[] = {
//
// 	x ,   y ,ref, Htot,HsynS,HsynE,	Vtot,VsynS,VsynE,flg,  PixClock,CW,GT,CK
// 
{  320,  200, 70,  400,  328,  376,  449,  413,  415,  5,  12587500, 8, 0, 0 },
{  320,  400, 70,  400,  328,  376,  449,  413,  415,  4,  12587500, 8, 0, 0 },
{  360,  200, 70,  450,  369,  423,  449,  413,  415,  5,  14161000, 8, 0, 0 },
{  360,  400, 70,  450,  369,  423,  449,  413,  415,  4,  14161000, 8, 0, 0 },
{  640,  200, 70,  800,  656,  752,  449,  413,  415,  5,  25175000, 8, 0, 0 },
{  640,  350, 70,  800,  656,  752,  449,  387,  389,  8,  25175000, 8, 0, 0 },
{  640,  350, 85,  832,  672,  736,  445,  382,  385,  8,  31500000, 8, 0, 0 },
{  640,  400, 70,  800,  656,  752,  449,  413,  415,  4,  25175000, 8, 0, 0 },
{  640,  400, 85,  832,  672,  736,  445,  401,  404,  4,  31500000, 8, 0, 0 },
{  640,  480, 60,  800,  656,  752,  525,  490,  492, 12,  25175000, 8, 0, 0 },
{  640,  480, 72,  832,  664,  704,  520,  489,  492, 12,  31500000, 8, 0, 0 },
{  640,  480, 75,  840,  656,  720,  500,  481,  484, 12,  31500000, 8, 0, 0 },
{  640,  480, 85,  832,  696,  752,  509,  481,  484, 12,  36000000, 8, 0, 0 },
{  720,  400, 70,  900,  738,  846,  449,  413,  415,  4,  28322000, 9, 0, 0 },
{  720,  400, 85,  936,  756,  828,  446,  401,  404,  4,  35500000, 9, 0, 0 },
{  800,  600, 56, 1024,  824,  896,  625,  601,  603,  0,  36000000, 8, 0, 0 },
{  800,  600, 60, 1056,  840,  968,  628,  601,  605,  0,  40000000, 8, 0, 0 },
{  800,  600, 72, 1040,  856,  976,  666,  637,  643,  0,  50000000, 8, 0, 0 },
{  800,  600, 75, 1056,  816,  896,  625,  601,  604,  0,  49500000, 8, 0, 0 },
{  800,  600, 85, 1048,  832,  896,  631,  601,  604,  0,  56250000, 8, 0, 0 },
{ 1024,  768, 60, 1344, 1048, 1184,  806,  771,  777, 12,  65000000, 8, 0, 0 },
{ 1024,  768, 70, 1328, 1048, 1184,  806,  771,  777, 12,  75000000, 8, 0, 0 },
{ 1024,  768, 75, 1312, 1040, 1136,  800,  769,  772,  0,  78750000, 8, 0, 0 },
{ 1024,  768, 85, 1376, 1072, 1168,  808,  769,  772,  0,  94500000, 8, 0, 0 },
{ 1152,  864, 75, 1600, 1216, 1344,  900,  865,  868,  0, 108000000, 8, 0, 0 },
{ 1280,  960, 60, 1800, 1376, 1488, 1000,  961,  964,  0, 108000000, 8, 0, 0 },
{ 1280,  960, 85, 1728, 1344, 1504, 1011,  961,  964,  0, 148500000, 8, 0, 0 },
{ 1280, 1024, 60, 1688, 1328, 1440, 1066, 1025, 1028,  0, 108000000, 8, 0, 0 },
{ 1280, 1024, 75, 1688, 1296, 1440, 1066, 1025, 1028,  0, 135000000, 8, 0, 0 },
{ 1280, 1024, 85, 1728, 1344, 1504, 1072, 1025, 1028,  0, 157500000, 8, 0, 0 },
{ 1600, 1200, 60, 2160, 1664, 1856, 1250, 1201, 1204,  0, 162000000, 8, 0, 0 },
{ 1600, 1200, 65, 2160, 1664, 1856, 1250, 1201, 1204,  0, 175500000, 8, 0, 0 },
{ 1600, 1200, 70, 2160, 1664, 1856, 1250, 1201, 1204,  0, 189000000, 8, 0, 0 },
{ 1600, 1200, 75, 2160, 1664, 1856, 1250, 1201, 1204,  0, 202500000, 8, 0, 0 },
{ 1600, 1200, 85, 2160, 1664, 1856, 1250, 1201, 1204,  0, 229500000, 8, 0, 0 },
{ 1792, 1344, 60, 2448, 1920, 2120, 1394, 1345, 1348,  4, 204750000, 8, 0, 0 },
{ 1792, 1344, 75, 2456, 1888, 2104, 1417, 1345, 1348,  4, 261000000, 8, 0, 0 },
{ 1856, 1392, 60, 2528, 1952, 2176, 1439, 1393, 1396,  4, 218250000, 8, 0, 0 },
{ 1856, 1392, 75, 2560, 1984, 2208, 1500, 1393, 1396,  4, 288000000, 8, 0, 0 },
{ 1920, 1440, 60, 2600, 2048, 2256, 1500, 1441, 1444,  4, 234000000, 8, 0, 0 },
{ 1920, 1440, 75, 2640, 2064, 2288, 1500, 1441, 1444,  4, 297000000, 8, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};


//=======================================================================================
// Function name:  di_GetNumParamsInString
//
// Description:    Search a string for fields separated by commas and return the field count
//
// Information:    
//
// Return:         INT     Number of fields in the passed string
//
//=======================================================================================
int di_GetNumParamsInString( char * paramstr )
{
int fieldcount = 0;
int i;


	for ( i = 0; paramstr[i] != 0 ; i++ )	// count the commas in the parameter string
	{										// to determine if we are opting to use detailed
		if ( paramstr[i] == ',' )			// alternate timings or the simpler VESA 3 based timings.
			fieldcount++;
	}

	fieldcount++;

	return ( fieldcount );
}

//=======================================================================================
// Function name:  di_ParseStringForInteger
//
// Description:    Search a string for an int with fields separated by commas
//
// Information:    
//
// Return:         INT     Result returned by the call or,
//                        -1L if failure.
//=======================================================================================
int di_ParseStringForInteger( char *buf, int fieldnum )
{
char * chptr = buf;


	if ( fieldnum < 1 )	   	// Handle invalid field number
		return( -1 );

	fieldnum--;

	while ( fieldnum-- )
		{
			chptr = strchr( chptr, ',' );

			if ( chptr == NULL )
				return(	-1 );

			chptr++;
		}
	
	return( di_atoi( chptr ) );
}

//=======================================================================================
// Function name:  di_ParseStringForLong
//
// Description:    Search a string for a long with fields separated by commas
//
// Information:    
//
// Return:         LONG    Result returned by the call or,
//                        -1L if failure.
//=======================================================================================
long di_ParseStringForLong( char *buf, int fieldnum )
{
char * chptr = buf;


	if ( fieldnum < 1 )	   	// Handle invalid field number
		return( -1 );

	fieldnum--;

	while ( fieldnum-- )
		{
			chptr = strchr( chptr, ',' );

			if ( chptr == NULL )
				return(	-1 );

			chptr++;
		}
	
	return( di_atol( chptr ) );
}

//=======================================================================================
// Function name:  di_ParseStringForULong
//
// Description:    Search a string for a DWORD with fields separated by commas
//
// Information:    
//
// Return:         DWORD   Result returned by the call or,
//                         0xFFFFFFFF if failure.
//=======================================================================================
DWORD di_ParseStringForULong( char *buf, int fieldnum )
{
char * chptr = buf;


	if ( fieldnum < 1 )	   	// Handle invalid field number
		return( 0xFFFFFFFF );

	fieldnum--;

	while ( fieldnum-- )
		{
			chptr = strchr( chptr, ',' );

			if ( chptr == NULL )
				return(	0xFFFFFFFF );

			chptr++;
		}
	
	return( di_atoul( chptr ) );
}

//=======================================================================================
// Function name:  di_ParseBitDepthString
//
// Description:    Return a value that represents which color depths are available
//				   within a capability string.
// Information:    
//
// Return:         DWORD   Bits that represent each supported bit depth.
//                         
//=======================================================================================
DWORD di_ParseBitDepthString( char * capstr )
{
int i;
DWORD retval = 0;
char bppstr[MAX_CAP_STRING_SIZE];
char * bpp_ptr;


	strcpy( bppstr, capstr );

	bpp_ptr = strstr( bppstr, BPP_CAP_STR );

	if ( bpp_ptr )
	{
		// Move to the start of the bitdepth portion of the string
		bpp_ptr += sizeof(BPP_CAP_STR);	
		
		for ( i = 0; bppstr[i] != ',', bppstr[i] != 0; i++ )
			;
			
		bppstr[i] = 0;	// Append NULL to end of the string we are interested in
		
		if ( strstr( bppstr, "+8" ) )	 
			retval |= BPP_8_CAP_FLAG;
		if ( strstr( bppstr, "+16" ) )	 
			retval |= BPP_16_CAP_FLAG;
		if ( strstr( bppstr, "+24" ) )	 
			retval |= BPP_24_CAP_FLAG;
		if ( strstr( bppstr, "+32" ) )	 
			retval |= BPP_32_CAP_FLAG;
	}

	return( retval );
}

//=======================================================================================
// Function name:  di_QueryNumBitDepthsInString
//
// Description:    Return a value that represents how many color depths are available
//				   within a capability string.
// Information:    
//
// Return:         int   Number of color depths.
//                         
//=======================================================================================
int di_QueryNumBitDepthsInString( char * capstr )
{
int retval = 0;
DWORD bpp_support;


	bpp_support = di_ParseBitDepthString( capstr );

	if ( bpp_support & BPP_8_CAP_FLAG )
		retval++;
	if ( bpp_support & BPP_16_CAP_FLAG )
		retval++;
	if ( bpp_support & BPP_24_CAP_FLAG )
		retval++;
	if ( bpp_support & BPP_32_CAP_FLAG )
		retval++;

	return ( retval );
}

//=======================================================================================
// Function name:  di_ParseStringForCaps
//
// Description:    Parse the capabilities string for fields separated by commas, and
//				   return the appropriate bits for each capabliity.
// Information:    
//
// Return:         DWORD   Bits that represent each supported capability.
//                         
//=======================================================================================
DWORD di_ParseStringForCaps( char * capstr )
{
DWORD retval;


	retval = di_ParseBitDepthString( capstr );

	if ( strstr( capstr, DDRAW_CAPABLE_STR ) )
		retval |= DDRAW_CAPABLE;
	if ( strstr( capstr, TVOUT_DESKTOP_CAPABLE_STR ) )
		retval |= TVOUT_DESKTOP_CAPABLE;
	if ( strstr( capstr, TVOUT_DDRAW_CAPABLE_STR ) )
		retval |= TVOUT_DDRAW_CAPABLE;
	if ( strstr( capstr, DFP_DESKTOP_CAPABLE_STR ) )
		retval |= DFP_DESKTOP_CAPABLE;
	if ( strstr( capstr, DFP_DDRAW_CAPABLE_STR ) )
		retval |= DFP_DDRAW_CAPABLE;
	if ( strstr( capstr, NTSC_CAPABLE_STR ) )
		retval |= NTSC_CAPABLE;
	if ( strstr( capstr, PAL_CAPABLE_STR ) )
		retval |= PAL_CAPABLE;

	return( retval );	
}

//=======================================================================================
// Function name:  di_VerifyChecksumParam
//
// Description:    Calculate checksum from fields in the timings parameter string, and
//				   indicate pass or fail.
// Information:    
//
// Return:         BOOL   TRUE if the checksum matches or,
//                        FALSE if failure.
//=======================================================================================
BOOL di_VerifyChecksumParam( HKEY hkey, char *paramstr )
{
char * checkptr;
char nbrstr[MAX_NUMERIC_STRING_SIZE];
DWORD checksum;
BYTE bytechecksum;
int fieldnum;


	// If we are to use Generalized Timing Formulas, then no verification is required
	if ( strstr( paramstr, "GTF" ) || strstr( paramstr, "gtf" ) )
		return( TRUE );	

	if ( di_GetNumParamsInString( paramstr ) <= CHECKSUM_FIELD )
	{
		// Detailed refresh parameter is not needed for timing calculations, but it is in our
		// parameter list for our convienience... However, it is still needed for the checksum. 
		checksum = di_ParseStringForULong( paramstr, REFRESH_FIELD ); 
		
		checksum += di_ParseStringForInteger( paramstr, HTOTAL_FIELD );
		checksum += di_ParseStringForInteger( paramstr, HSYNCSTART_FIELD );
		checksum += di_ParseStringForInteger( paramstr, HSYNCEND_FIELD );
		checksum += di_ParseStringForInteger( paramstr, VTOTAL_FIELD );
		checksum += di_ParseStringForInteger( paramstr, VSYNCSTART_FIELD );
		checksum += di_ParseStringForInteger( paramstr, VSYNCEND_FIELD );
		checksum += di_ParseStringForInteger( paramstr, CRTCFLAG_FIELD );
		checksum += ( di_ParseStringForULong( paramstr, PIXCLOCK_FIELD ) / 10000 );
		checksum += di_ParseStringForInteger( paramstr, CHARWIDTH_FIELD );

		checkptr = strstr( paramstr, "check" );

		if ( checkptr )
		{
			checkptr[0] = 0;
			di_ultoa( checksum, nbrstr, DECIMAL );
			strcat( paramstr, nbrstr );

			RegSetValueEx(hkey,
	                      TIMINGS_VALUE_STR,	// NULL ptr = Set Default value
	                      0,
	                      REG_SZ,
	                      (LPBYTE)paramstr,
	                      strlen(paramstr));
		}

		// If the checksum does not match, than the registry may be corrupt,
		// or the user has tampered with these values.
		if ( checksum != di_ParseStringForULong( paramstr, CHECKSUM_FIELD ) )
			return( FALSE );
	}
	else   	// use detailed CRTC table style timings
	{
		bytechecksum = 0;
		for ( fieldnum = 1; fieldnum < ALT_CHECKSUM_FIELD; fieldnum++ )
			bytechecksum += di_ParseStringForInteger( paramstr,	fieldnum );

		checkptr = strstr( paramstr, "check" );

		if ( checkptr )
		{
			checkptr[0] = 0;
			di_ultoa( bytechecksum, nbrstr, DECIMAL );
			strcat( paramstr, nbrstr );

			RegSetValueEx(hkey,
	                      DFP_TIMINGS_VALUE_STR,
	                      0,
	                      REG_SZ,
	                      (LPBYTE)paramstr,
	                      strlen(paramstr));
		}

		// If the checksum does not match, than the registry may be corrupt,
		// or the user has tampered with these values.
		if ( bytechecksum != di_ParseStringForInteger( paramstr, ALT_CHECKSUM_FIELD ) )
			return( FALSE );
	}
			
	return( TRUE );	
}

//=======================================================================================
// Function name:  di_EnumBitDepth
//
// Description:    
//				
// Information:    
//
// Return:         
//				
//=======================================================================================
int di_EnumBitDepth( char * capstr, int index )
{
int i;
int retval = 0;
char bppstr[MAX_CAP_STRING_SIZE];
char * bpp_ptr;


	strcpy( bppstr, capstr );

	bpp_ptr = strstr( bppstr, BPP_CAP_STR );

	if ( bpp_ptr )
	{
		// Move to the start of the bitdepth portion of the string
		bpp_ptr += sizeof(BPP_CAP_STR);	
		
		for ( i = 0; bppstr[i] != ',', bppstr[i] != 0; i++ )
			;
			
		bppstr[i] = 0;	// Append NULL to end of the string we are interested in
		
		i = index - 1;

		if ( strstr( bppstr, "+32" ) )	 
		{
			if ( i-- <= 0 ) 
				return( 32 );
		}
		if ( strstr( bppstr, "+24" ) )
		{	 
			if ( i-- <= 0 ) 
				return( 24 );
		}
		if ( strstr( bppstr, "+16" ) )
		{	 
			if ( i-- <= 0 ) 
				return( 16 );
		}
		if ( strstr( bppstr, "+8" ) )
		{		 
			if ( i-- <= 0 ) 
				return( 8 );
		}
	}

	return( 0 );
}

//=======================================================================================
// Function name:  di_RegOpenTimingsKey
//
// Description:    Opens the main timings key that branches of of the devnode. 
//				
// Information:    
//
// Return:         int CR_SUCCESS - If successful,	
//					   CR_FAILURE - If failure
//=======================================================================================
int	di_RegOpenTimingsKey( char * DevNodeKey, HKEY * hkey )
{
  	// convert devnode to a registry key
  	// when successful, this function returns a string something like
  	// "System\CurrentControlSet\Services\Class\DISPLAY\XXXX"
  	if (CR_SUCCESS != CM_Get_DevNode_Key(DisplayInfo.diDevNodeHandle,
                                        NULL,
                                        (PFARVOID)DevNodeKey,
                                        MAX_TIMINGS_REG_KEY_LEN,
                                        CM_REGISTRY_SOFTWARE))
  	{
		return CR_FAILURE;
  	}
	
	strcat( DevNodeKey, TIMINGS_KEY_STR );

    // attempt to open the main key
    if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE,
                                        DevNodeKey,
                                        hkey))
    {
		return CR_FAILURE;
	}

	return CR_SUCCESS;
}

//=======================================================================================
// Function name:  di_GetBaselineTiming
//
// Description:    Searches baseline timing table for a match in width, height,
//				   and	refresh and loads the parameters into a timing struct.
//
// Information:    This function should be called only when the User has tampered with
//                 the TIMINGS registry entries, or the registry is corrupt.
//
// Return:         TRUE  - If an exact match is found in the baseline table
//  			   FALSE - If match is not found
//=======================================================================================
BOOL di_GetBaselineTiming( TIMING_PARAMS *pTprm )
{
int i;
TIMING_PARAMS *pBaseline = &baseline_timings[0];


	for ( i = 0; pBaseline->width != 0; i++ )
	{
		if ( pBaseline->width == pTprm->width &&
			 pBaseline->height == pTprm->height &&
			 pBaseline->refresh == pTprm->refresh )
		{

			pTprm->HTotal = pBaseline->HTotal;
			pTprm->HSyncStart = pBaseline->HSyncStart;
			pTprm->HSyncEnd = pBaseline->HSyncEnd;
			pTprm->VTotal = pBaseline->VTotal;
			pTprm->VSyncStart = pBaseline->VSyncStart;
			pTprm->VSyncEnd = pBaseline->VSyncEnd;
			pTprm->CRTCflags = pBaseline->CRTCflags;
			pTprm->PixelClock = pBaseline->PixelClock;
			pTprm->CharWidth = pBaseline->CharWidth;
			pTprm->UseGTF = FALSE;
			pTprm->UseAltTiming = FALSE;
			return TRUE;
		}

		pBaseline = &baseline_timings[i];
	}

	pTprm->UseGTF = TRUE;	  	// Use GTF timing as a safety if we cannot find a matching entry
	pTprm->CharWidth = DEFAULT_CHAR_WIDTH;
	pTprm->CRTCflags = DEFAULT_CRTC_FLAGS;
	
	return FALSE;
}

//=======================================================================================
// Function name:  di_GetRegistryTiming
//
// Description:    Parses the TIMINGS Default key in the registry and 
//				   loads parameters into a timing struct.
// Information:    
//
// Return:         VOID
//
//=======================================================================================
void di_GetRegistryTiming( TIMING_PARAMS *pTprm, char * paramstr )
{
int i;
DWORD fout;
DWORD N, M, K;


	pTprm->UseGTF = FALSE;

	if ( di_GetNumParamsInString( paramstr ) <= CHECKSUM_FIELD )		
	{
		pTprm->HTotal = di_ParseStringForInteger( paramstr, HTOTAL_FIELD );
		pTprm->HSyncStart = di_ParseStringForInteger( paramstr, HSYNCSTART_FIELD );
		pTprm->HSyncEnd = di_ParseStringForInteger( paramstr, HSYNCEND_FIELD );
		pTprm->VTotal = di_ParseStringForInteger( paramstr, VTOTAL_FIELD );
		pTprm->VSyncStart = di_ParseStringForInteger( paramstr, VSYNCSTART_FIELD );
		pTprm->VSyncEnd = di_ParseStringForInteger( paramstr, VSYNCEND_FIELD );
		pTprm->CRTCflags = di_ParseStringForInteger( paramstr, CRTCFLAG_FIELD );
		pTprm->PixelClock = di_ParseStringForULong( paramstr, PIXCLOCK_FIELD );
		pTprm->CharWidth = di_ParseStringForInteger( paramstr, CHARWIDTH_FIELD );
		pTprm->UseAltTiming = FALSE;
	}
	else	// use detailed CRTC table style timings
	{
		for ( i = 0; i < ALT_CHECKSUM_FIELD; i++ )
			pTprm->AltTiming[i] = di_ParseStringForInteger( paramstr, i + 1 );

		if ( pTprm->AltTiming[ALT_SR1] & 1 )
			pTprm->CharWidth = 8;
		else
			pTprm->CharWidth = 9;

		pTprm->HTotal = pTprm->AltTiming[ALT_HTOTAL] + 5;
		pTprm->HTotal *= 8;		// Convert chars to pixels

		pTprm->CRTCflags = 0;

		if ( pTprm->AltTiming[ALT_MISC_OUTPUT] & 0x80 )	// Setup polarity flags
			pTprm->CRTCflags |= 8;
		if ( pTprm->AltTiming[ALT_MISC_OUTPUT] & 0x40 )
			pTprm->CRTCflags |= 4;
		if ( pTprm->width < 640 )	  // Assume Scanline doubling if width is less than 640
			pTprm->CRTCflags |= 1;

		// Bit	Description
		// 1:0	K, Post divider value
		// 7:2	M, PLL input divider
		// 15:8	N, PLL multiplier

		// fout = 14.31818 * (N + 2) / (M + 2) / (2 ^ K).
		N = (DWORD)(pTprm->AltTiming[ALT_PLLCTRL0_HIGH] + 2 );
		M = (DWORD)(pTprm->AltTiming[ALT_PLLCTRL0_LOW] >> 2 ) + 2;
		K = (DWORD)(pTprm->AltTiming[ALT_PLLCTRL0_LOW] & 3) - 1;

		K = ( 2 << K );

		fout = ( 1431818 * N ) / M; 
		fout = fout / K;
		pTprm->PixelClock = fout * 10;

		pTprm->UseAltTiming = TRUE;
	}

	if ( strstr( paramstr, "GTF" ) || strstr( paramstr, "gtf" ) )
	{
		pTprm->UseGTF = TRUE;

		if ( pTprm->CharWidth == -1 )  	// GTF character width defaults to 8
			pTprm->CharWidth = DEFAULT_CHAR_WIDTH;

		if ( pTprm->CRTCflags == -1 )  	// GTF flags defaults to H-NEG, V-POS
			pTprm->CRTCflags = DEFAULT_CRTC_FLAGS;
	}
}

//=======================================================================================
// Function name:  di_QueryNumRegModes
//
// Description:    Counts the TIMINGS\XRES\YRES\REF keys in the registry for the 
//				   total number of modes.
//
// Information:    
//
// Return:         INT - Total number of modes found in registry.
//=======================================================================================
int di_QueryNumRegistryModes( void )
{
HKEY hkey1 = 0;
HKEY hkey2 = 0;
HKEY hkey3 = 0;
int index1 = 0;
int index2 = 0;
int total = 0;
char subkey[MAX_NUMERIC_STRING_SIZE];
char subkey2[MAX_NUMERIC_STRING_SIZE];
char capstr[MAX_CAP_STRING_SIZE];
char DevNodeKey[MAX_TIMINGS_REG_KEY_LEN];
char DevNodeModeKey[sizeof(DevNodeKey)+sizeof(subkey)];
char DevNodeRefrKey[sizeof(DevNodeModeKey)+16];
DWORD length = sizeof(subkey);
DWORD caplength = sizeof(capstr);
DWORD type;


	if ( CR_FAILURE == di_RegOpenTimingsKey( DevNodeKey, &hkey1 ))
		return( 0 );
	
	// enumerate the list of mode entries
    while (ERROR_SUCCESS == RegEnumKey( hkey1, index1, subkey, length ))
    {
		strcpy( DevNodeModeKey, DevNodeKey );
		strcat( DevNodeModeKey, "\\" );
		strcat( DevNodeModeKey, subkey );
		index2 = 0;

      	// attempt to open the mode sub key
    	if ( ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE, DevNodeModeKey, &hkey2))
			break;

		// enumerate the list of refresh rate entries
    	while ( ERROR_SUCCESS == RegEnumKey( hkey2, index2, subkey2, length ))
		{
			strcpy( DevNodeRefrKey, DevNodeModeKey );
			strcat( DevNodeRefrKey, "\\" );
			strcat( DevNodeRefrKey, subkey2 );

      		// attempt to open the refresh rate sub key
    		if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE, DevNodeRefrKey, &hkey3))
				break;

			index2++;
			caplength = sizeof(capstr);

			// the key exists so attempt to read the value of varname
    		if (ERROR_SUCCESS != RegQueryValueEx(hkey3,
                                         SUPPORT_VALUE_STR,	
                                         0,
                                         &type,
                                         (LPBYTE)capstr,
                                         &caplength))
    		{
				continue;  // Oops,  no "Supported" entry was found
			}

			RegCloseKey(hkey3);

			total += di_QueryNumBitDepthsInString( capstr );

		}

		index1++;
		RegCloseKey(hkey2);
	}

	RegCloseKey(hkey1);

	return(total);
}

//=======================================================================================
// Function name:  di_EnumRegistryMode
//
// Description:    Enumerates the TIMINGS\XRES,YRES\REF keys in the registry,
//                 and fills in the appropriate MODEINFO structure.
// Information:    
//
// Return:         TRUE - If a mode was found according to the given index.
// 				   FALSE - If a mode was NOT found	according to the given index.
//=======================================================================================
BOOL di_EnumRegistryMode( MODEINFO FAR * ModeInfo, int index )
{
TIMING_PARAMS timingparams;
HKEY hkey1 = 0;
HKEY hkey2 = 0;
HKEY hkey3 = 0;
int index1 = 0;
int index2 = 0;
int total = 0;
long pixelclock;
long htotal;
char subkey[MAX_NUMERIC_STRING_SIZE];
char subkey2[MAX_NUMERIC_STRING_SIZE];
char paramstr[MAX_TIMING_STRING_SIZE];
char capstr[MAX_CAP_STRING_SIZE];
char DevNodeKey[MAX_TIMINGS_REG_KEY_LEN];
char DevNodeModeKey[MAX_TIMINGS_REG_KEY_LEN+sizeof(subkey)];
char DevNodeRefrKey[sizeof(DevNodeModeKey)+16];
DWORD type;
DWORD capflag;
DWORD length = sizeof(subkey);
DWORD length2 = sizeof(subkey2);
DWORD caplength = sizeof(capstr);
DWORD paramlength = sizeof(paramstr);
BOOL modefound = FALSE;
BOOL passchecksum;
BOOL read_alt_timings;


	if ( CR_FAILURE == di_RegOpenTimingsKey( DevNodeKey, &hkey1 ))
		return( FALSE );
	
	// enumerate the list of mode entries
    while (ERROR_SUCCESS == RegEnumKey( hkey1, index1, subkey, length ))
    {
		strcpy( DevNodeModeKey, DevNodeKey );
		strcat( DevNodeModeKey, "\\" );
		strcat( DevNodeModeKey, subkey );
		index2 = 0;

      	// attempt to open the mode sub key
    	if ( ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE, DevNodeModeKey, &hkey2))
			break;

		// enumerate the list of refresh rate entries
    	while ( ERROR_SUCCESS == RegEnumKey( hkey2, index2, subkey2, length2 ))
		{
			strcpy( DevNodeRefrKey, DevNodeModeKey );
			strcat( DevNodeRefrKey, "\\" );
			strcat( DevNodeRefrKey, subkey2 );

      		// attempt to open the refresh rate sub key
    		if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE, DevNodeRefrKey, &hkey3))
				break;

			index2++;
			caplength = sizeof(capstr);

			// the key exists so attempt to read the value of varname
    		if (ERROR_SUCCESS != RegQueryValueEx(hkey3,
                                         SUPPORT_VALUE_STR,	
                                         0,
                                         &type,
                                         (LPBYTE)capstr,
                                         &caplength))
    		{
				RegCloseKey(hkey3);
				continue;  // Oops,  no "Supported" entry
			}

			paramlength = sizeof(paramstr);

			// the key exists so attempt to read the parameter string

            //alt_timings are obsolete, don't process these lines in .inf
			read_alt_timings = FALSE;

			if ( read_alt_timings == FALSE )
			{
	    		if (ERROR_SUCCESS != RegQueryValueEx(hkey3,
	                                         TIMINGS_VALUE_STR,	
	                                         0,
	                                         &type,
	                                         (LPBYTE)paramstr,
	                                         &paramlength))
	    		{
					RegCloseKey(hkey3);
					continue;  // Oops,  no "Default" entry
				}
			}

			capflag = di_ParseStringForCaps( capstr );

			total += di_QueryNumBitDepthsInString( capstr );

			if ( total > index )
			{
				timingparams.width = di_ParseStringForInteger( subkey, 1 );
				timingparams.height = di_ParseStringForInteger( subkey, 2 );
				timingparams.refresh = di_ParseStringForInteger( subkey2, 1 );
	
				// check for user tampering
				passchecksum = di_VerifyChecksumParam( hkey3, paramstr );

				if ( passchecksum )	// If there is no evidence of user tampering
					di_GetRegistryTiming( &timingparams, paramstr );
				else 				// User has tampered with registry!
					di_GetBaselineTiming( &timingparams );	

				if ( timingparams.UseGTF == TRUE || ModeInfo->UseGTF == TRUE )
				{
					if ( !( timingparams.CRTCflags & 1 ) )		// If not Double Scanned
						timingparams.CRTCflags = DEFAULT_CRTC_FLAGS;

					VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
							H3VDD_GET_GTF_TIMINGS, 0, &timingparams);
#ifdef REAL_NET
               ModeInfo->wVert100 = (WORD)timingparams.refresh100;
#endif
				}
#ifdef REAL_NET
            else
               ModeInfo->wVert100 = (WORD)di_ParseStringForULong( paramstr, REFRESH_FIELD );
#endif


				pixelclock = timingparams.PixelClock;
				htotal = timingparams.HTotal;

				// Transfer timingparam info to ModeInfo structure
				ModeInfo->dwWidth  = timingparams.width;
				ModeInfo->dwHeight = timingparams.height;
				ModeInfo->wVert = (WORD)timingparams.refresh;
				ModeInfo->dwBPP = di_EnumBitDepth( capstr, total - index );
				ModeInfo->lPitch = ds_CalculatePitch( ModeInfo->dwWidth, ModeInfo->dwBPP );
				ModeInfo->wHorz = (WORD)(( pixelclock / 100 ) / htotal );
				ModeInfo->dwFlags = 0;
				ModeInfo->HTotal = htotal;
				ModeInfo->HSyncStart = timingparams.HSyncStart;
				ModeInfo->HSyncEnd = timingparams.HSyncEnd;
				ModeInfo->VTotal =  timingparams.VTotal;
				ModeInfo->VSyncStart = timingparams.VSyncStart;
				ModeInfo->VSyncEnd = timingparams.VSyncEnd;
				ModeInfo->CRTCflags = timingparams.CRTCflags;
				ModeInfo->PixelClock = pixelclock;
				ModeInfo->CharWidth = timingparams.CharWidth;
				ModeInfo->UseAltTiming = FALSE;  //Note: use of altTimings for DFPs is obsolete. DanO 6/7/00
                ModeInfo->dwDFPWidth = 0;
                ModeInfo->dwDFPHeight = 0;

				if ( ModeInfo->UseAltTiming )
 					_fmemcpy( ModeInfo->AltTiming, timingparams.AltTiming, CRTC_TABLE_SIZE );

				if ( timingparams.CRTCflags & 1 )
					ModeInfo->dwFlags |= SCANLINE_DBL;	// Translate the Scanline Double flag to ModeInfo struct

				if ( timingparams.CRTCflags & 2 )
					ModeInfo->dwFlags |= INTERLACED;	// Translate the Interlace flag to ModeInfo struct

				if ( capflag & DDRAW_CAPABLE )
					ModeInfo->dwFlags |= IS_DDRAW_MODE;

				if ( capflag & TVOUT_DESKTOP_CAPABLE )
					ModeInfo->dwFlags |= TVOUT_DESKTOP;

				if ( capflag & TVOUT_DDRAW_CAPABLE )
					ModeInfo->dwFlags |= TVOUT_DDRAW;

				if ( capflag & NTSC_CAPABLE )
					ModeInfo->dwFlags |= TVOUT_NTSC;

				if ( capflag & PAL_CAPABLE )
					ModeInfo->dwFlags |= TVOUT_PAL;

				if ( capflag & DFP_DESKTOP_CAPABLE )
					ModeInfo->dwFlags |= LCD_DESKTOP;

				if ( capflag & DFP_DDRAW_CAPABLE )
					ModeInfo->dwFlags |= LCD_DDRAW;

				// 2ppc is still mutually exclusive with multi-texturing and trilinear filtering.
				if ( ( ModeInfo->dwBPP <= 16 ) || ( ModeInfo->dwBPP == 32 ) )
				{
					ModeInfo->dwFlags |= REND2PIX_PER_CLK;
				}

				RegCloseKey(hkey3);
				modefound = TRUE;
				break;
			}

			RegCloseKey(hkey3);
		}

		if ( modefound )
			break;

		index1++;
		RegCloseKey(hkey2);
	}

	RegCloseKey(hkey1);

	return( modefound );
}

//=======================================================================================
// Function name:  di_GetRegistryGTFOverride
//
// Description:    Reads the TIMINGS\UseGTF entry in the registry,
// 				   and fills a BOOL with TRUE or FALSE according to the value.
// Information:    
//
// Return:         TRUE - If the registry entry was found and read.
//				   FALSE - If the registry entry was NOT read.
//=======================================================================================
BOOL di_GetRegistryGTFOverride( BOOL * UseGTF )
{
HKEY hkey;
char subkey[] = USE_GTF_VALUE_STR;
char nbrstr[MAX_NUMERIC_STRING_SIZE];
char DevNodeKey[MAX_TIMINGS_REG_KEY_LEN];
DWORD type;
DWORD length = sizeof(nbrstr);


	if ( CR_FAILURE == di_RegOpenTimingsKey( DevNodeKey, &hkey ))
		return( FALSE );
	
    // the key exists so attempt to read the value of varname
    if (ERROR_SUCCESS != RegQueryValueEx(hkey,
                                         subkey,	// NULL = Get Default value
                                         0,
                                         &type,
                                         (LPBYTE)nbrstr,
                                         &length))
    {
		RegCloseKey(hkey);
		return FALSE;
	}

	RegCloseKey(hkey);

	if ( atoi(&nbrstr[0]) )
		*UseGTF = TRUE;
	else
		*UseGTF = FALSE;

	return TRUE;
}

//=======================================================================================
// Function name:  di_GetRegistryOptRefreshLimit
//
// Description:    Reads the TIMINGS\OptimalRefreshLimit entry in the registry,
//				   and fills a integer with according to the value. Also reads the
//				   TIMINGS\OptimalNonEDIDLimit entry and fills that integer with that value.
// Information:    
//
// Return:         TRUE - If the registry entry was found and read.
//				   FALSE - If the registry entry was NOT read.
//=======================================================================================
BOOL di_GetRegistryOptRefreshLimit( int * OptRefreshLim, int * OptNonEDIDLim )
{
HKEY hkey;
char subkey1[] = OPT_REFRESH_LIMIT_STR;
char subkey2[] = OPT_NONEDID_LIMIT_STR;
char nbrstr[MAX_NUMERIC_STRING_SIZE];
char DevNodeKey[MAX_TIMINGS_REG_KEY_LEN];
DWORD type;
DWORD length = sizeof(nbrstr);


	*OptRefreshLim = MAX_REFRESH_RATE;
	*OptNonEDIDLim = MAX_NONEDID_RATE;

	if ( CR_FAILURE == di_RegOpenTimingsKey( DevNodeKey, &hkey ))
		return( FALSE );
	
    // the key exists so attempt to read the value of varname
    if (ERROR_SUCCESS != RegQueryValueEx(hkey,
                                         subkey1,	// NULL = Get Default value
                                         0,
                                         &type,
                                         (LPBYTE)nbrstr,
                                         &length))
    {
		RegCloseKey(hkey);
		return FALSE;
	}

	if ( strstr( nbrstr, "MAX_EDID" ) || strstr( nbrstr, "Max_EDID" ) )
		*OptRefreshLim = MAX_EDID_LIMIT;
	else
		*OptRefreshLim = atoi(&nbrstr[0]);

    // the key exists so attempt to read the value of varname
    if (ERROR_SUCCESS != RegQueryValueEx(hkey,
                                         subkey2,	// NULL = Get Default value
                                         0,
                                         &type,
                                         (LPBYTE)nbrstr,
                                         &length))
    {
		RegCloseKey(hkey);
		return FALSE;
	}

	RegCloseKey(hkey);

	*OptNonEDIDLim = atoi(&nbrstr[0]);

	return TRUE;
}

//=======================================================================================
// Function name:  di_BuildDefaultMode
//
// Description:    Generate a single definition of a default mode in the event of a catastropic
//				   registry read failure to prevent coming up in standard VGA mode.
//				   
// Information:    
//
// Return:         VOID.
//=======================================================================================
void di_BuildDefaultMode( MODEINFO FAR * ModeInfo )
{

			ModeInfo->dwWidth  = 640;
			ModeInfo->dwHeight = 480;
			ModeInfo->wVert = 60;
			ModeInfo->dwBPP = 8;
			ModeInfo->lPitch = 640;
			ModeInfo->dwFlags = ( TVOUT_DESKTOP | TVOUT_DDRAW | TVOUT_NTSC | TVOUT_PAL | LCD_DESKTOP | LCD_DDRAW );
			ModeInfo->dwFlags |= ( IS_DDRAW_MODE | REND2PIX_PER_CLK );
			ModeInfo->HTotal = 800;
			ModeInfo->HSyncStart = 656;
			ModeInfo->HSyncEnd = 752;
			ModeInfo->VTotal =  525;
			ModeInfo->VSyncStart = 490;
			ModeInfo->VSyncEnd = 492;
			ModeInfo->CRTCflags = 12;
			ModeInfo->PixelClock = 25175000;
			ModeInfo->CharWidth = 8;
			ModeInfo->wHorz = (WORD)(( ModeInfo->PixelClock / 100 ) / ModeInfo->HTotal );
			ModeInfo->UseAltTiming = FALSE;

			_fmemcpy( ModeInfo->AltTiming, defaultDFPtiming, CRTC_TABLE_SIZE );

}

//=======================================================================================
// Function name:  di_AllocModeTable
//
// Description:    Allocates space for the master mode list array, and fills the passed
//				   integer address with the total number of available modes.
//
// Information:    
//
// Return:         VOID	FAR *  Non Null if successful	
//							   Null if failure
//=======================================================================================
void FAR * di_AllocModeTable( int * nummodes )
{
HGLOBAL hgbl;
DWORD bufsize;
void FAR * bufptr;
ULONG numfoundmodes;

	
	*nummodes = di_QueryNumRegistryModes( );

	numfoundmodes = *nummodes;

    if ( numfoundmodes == 0 )
		numfoundmodes = 1;		// Allow space for 1 default mode if registry fails us.

	// The reason why we add one to the allocation is that many support functions
	// detect the end of the mode list by comparing the width to 0, so we need the
	// last structure entry to be empty and initialized to zero.
	bufsize = ( ( numfoundmodes + 1 ) * sizeof( MODEINFO ) );

	hgbl = GlobalAlloc( GMEM_FIXED | GMEM_SHARE | GMEM_ZEROINIT, bufsize );

    if ( hgbl == 0 )
		return NULL;

    bufptr = GlobalLock( hgbl );

	if ( bufptr == NULL )
      	GlobalFree(hgbl);

	return ( bufptr );
}

//=======================================================================================
// Function name:  di_SortModeTable
//
// Description:    Examines all Mode entrys in the Mode Table,	and places them
//				   in ascending order by BPP first, then width, then height, then refresh.
//
// Information:    Several of the routines in this driver require a sorted mode list.
//
// Return:         VOID.
//=======================================================================================
void di_SortModeTable( MODEINFO FAR * modeinfo, int nummodes )
{
MODEINFO tempmodeinfo;
int i,j;


	for (i = 0; i < nummodes; i++)
		for (j = i; j < nummodes; j++)
 		{
 			if ( modeinfo[i].dwBPP > modeinfo[j].dwBPP )
 			{
 				_fmemcpy( &tempmodeinfo, &modeinfo[j], sizeof(MODEINFO) );
 				_fmemcpy( &modeinfo[j], &modeinfo[i], sizeof(MODEINFO) );
 				_fmemcpy( &modeinfo[i], &tempmodeinfo, sizeof(MODEINFO) );
 			}
 		}

	for (i = 0; i < nummodes; i++)
		for (j = i; j < nummodes; j++)
 		{
 			if ( modeinfo[i].dwBPP == modeinfo[j].dwBPP &&
 			     modeinfo[i].dwWidth > modeinfo[j].dwWidth )
 			{
 				_fmemcpy( &tempmodeinfo, &modeinfo[j], sizeof(MODEINFO) );
 				_fmemcpy( &modeinfo[j], &modeinfo[i], sizeof(MODEINFO) );
 				_fmemcpy( &modeinfo[i], &tempmodeinfo, sizeof(MODEINFO) );
 			}
 		}

	for (i = 0; i < nummodes; i++)
		for (j = i; j < nummodes; j++)
 		{
 			if ( modeinfo[i].dwBPP == modeinfo[j].dwBPP &&
 			     modeinfo[i].dwWidth == modeinfo[j].dwWidth && 
 			     modeinfo[i].dwHeight > modeinfo[j].dwHeight )
 			{
 				_fmemcpy( &tempmodeinfo, &modeinfo[j], sizeof(MODEINFO) );
 				_fmemcpy( &modeinfo[j], &modeinfo[i], sizeof(MODEINFO) );
 				_fmemcpy( &modeinfo[i], &tempmodeinfo, sizeof(MODEINFO) );
 			}
 		}

	for (i = 0; i < nummodes; i++)
		for (j = i; j < nummodes; j++)
 		{
 			if ( modeinfo[i].dwBPP == modeinfo[j].dwBPP &&
 			     modeinfo[i].dwWidth == modeinfo[j].dwWidth && 
 			     modeinfo[i].dwHeight == modeinfo[j].dwHeight &&
 			     modeinfo[i].wVert > modeinfo[j].wVert )
 			{
 				_fmemcpy( &tempmodeinfo, &modeinfo[j], sizeof(MODEINFO) );
 				_fmemcpy( &modeinfo[j], &modeinfo[i], sizeof(MODEINFO) );
 				_fmemcpy( &modeinfo[i], &tempmodeinfo, sizeof(MODEINFO) );
 			}
 		}
}

//=======================================================================================
// Function name:  di_FillModeTable
//
// Description:    Reads in all TIMINGS mode entrys in the registry,
// 				   and places them in the Mode Info Table ( unsorted ).
// Information:    
//
// Return:         VOID.
//=======================================================================================
void di_FillModeTable( MODEINFO FAR * modeinfo, int nummodes )
{
int index = 0;
BOOL UseGTF_AllModes = FALSE;


	di_GetRegistryGTFOverride( &UseGTF_AllModes ); // Force use of GTF for all modes?

	if ( nummodes == 0 )
	{
		di_BuildDefaultMode( modeinfo );
		nNumModes = 1;
		return;
	}

	while ( index < nummodes )
	{
		if ( UseGTF_AllModes )
			modeinfo->UseGTF = TRUE;

		di_EnumRegistryMode( modeinfo, index );

		index++;

		modeinfo++;
	}

}

// #pragma optimize("",on)
