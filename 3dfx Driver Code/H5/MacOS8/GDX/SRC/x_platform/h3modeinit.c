/*
** Copyright (c) 1996-1999, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   H3modeinit.c
**
** Description: h3 mode initialization code (derived from h3vdd in win9x)
**
**
*/


#include "h3modeinit.h"
#include "math.h"

FxU16 crtc_table[CRTC_TABLE_SIZE] = { 0, 0, 0 };


/*----------------------------------------------------------------------
Function name:  ds_Calc_CRTC_table

Description:    Calculate and fill the crtc_table[] with values taken from 
				the VESA BIOS EXTENSION 3 parameters or use detailed 
				timings if an alternate timing is specified.

Information:

Return:         VOID
----------------------------------------------------------------------*/
VOID ds_Calc_CRTC_table( VidProcConfig *pVpc, PDEVTABLE pDev )
{
TIMING_PARAMS *pVprm = &pVpc->TimingParams;
BYTE byHTotal, byHorDispEnEnd, byHBlankStart;
BYTE byHBlankEnd, byHSyncStart, byHSyncEnd;
BYTE byVTotal, byOverflow, byMaxLineScan;
BYTE byVSyncStart, byVSyncEnd, byVertDispEnEnd;
BYTE byVBlankStart, byVBlankEnd;
BYTE byHExtensions, byVExtensions;
BYTE byMiscOutput, byDacMode;
BYTE byScanLineDoubled;
BYTE byHSyncPolarity;
BYTE byVSyncPolarity;
BYTE byCRTCflags;
BYTE bySeqDotClk;
WORD wHVisible;
WORD wHTotal;
WORD wHBlankStart;
WORD wHBlankTime;
WORD wHSyncStart;
WORD wHSyncTime;
WORD wVVisible;
WORD wVTotal;
WORD wVBlankStart;
WORD wVBlankTime;
WORD wVSyncStart;
WORD wVSyncTime;
DWORD bClockDouble;
double dbPixelClock;
// variables used for finding the best pll #s
int m, k, bestm, bestn, bestk;
double test, rndtest, newoverflow, oldoverflow;
WORD wPllCtrl0;
DWORD bBigger256 = FALSE;
#ifdef DEBUG
float answer;
WORD wPossiblePllCtrl0values[256];
WORD wPossiblePllCount = 0;
int iGoodTimings = 0;
#endif

	if ( pVprm->UseAltTiming )
	{
		for	( m = 0; m < sizeof( crtc_table ); m++ )
			crtc_table[m] = (WORD)pVprm->AltTiming[m];
		return;
	}
	 
	FPU_State( FPU_FUNCTION_SAVE );

	byCRTCflags = (BYTE)pVprm->CRTCflags;
	byScanLineDoubled = ( byCRTCflags & 1 );
	byHSyncPolarity = ( ( byCRTCflags << 4 ) & 0x40 );
	byVSyncPolarity = ( ( byCRTCflags << 4 ) & 0x80 );

	wHVisible = (WORD)( pVprm->width / pVprm->CharWidth ); // HVisible same as Hor Addr Time
	wHBlankStart = wHVisible; 	// We can assume this because there are no borders
	wHTotal = (WORD)( pVprm->HTotal / pVprm->CharWidth ); 
	wHBlankTime = ( wHTotal - wHVisible );
	wHSyncStart	= (WORD)( pVprm->HSyncStart / pVprm->CharWidth );
	wHSyncTime	= (WORD)( ( pVprm->HSyncEnd - pVprm->HSyncStart ) / pVprm->CharWidth );

	wVVisible = (WORD)pVprm->height;	 // VVisible same as Ver Addr Time
	if ( byScanLineDoubled )
		wVVisible *= 2;
	wVBlankStart = wVVisible;	// We can assume this because there are no borders
	wVTotal = (WORD)pVprm->VTotal;
	wVBlankTime = ( wVTotal - wVBlankStart );
	wVSyncStart	= (WORD)pVprm->VSyncStart;
	wVSyncTime  = (WORD)( pVprm->VSyncEnd - pVprm->VSyncStart );

	dbPixelClock = (double)pVprm->PixelClock;   
	dbPixelClock = ( dbPixelClock / 1000000 );	// Convert PixelClock from Hz to double

    if(pVpc->width != pVprm->width)  //Test for a dfp centered mode
    {
        WORD H2, V2;

        H2           = (WORD)(((pVprm->width - pVpc->width) / 2) / pVprm->CharWidth);
        wHVisible    = (WORD)( pVpc->width / pVprm->CharWidth );
        wHBlankStart -= H2;
        wHSyncStart  -= H2;

        V2           = (WORD)((pVprm->height - pVpc->height) / 2);
        wVVisible    = (WORD)pVpc->height;
        wVBlankStart -= V2;
        wVSyncStart  -= V2;
    }

	// Dacmode Bit0 set to 0 for 1:1 mode, 1 for 2:1 mode
	byDacMode = 0;

	// Test for DoubleDACRate
   bClockDouble = FALSE;
   if (IS_NAPALM(pDev->dwVendorDeviceID))
      {
      // The last or clause is needed since HBlank End is only 6 bits and it would need to be 9
      // this cause problem when switch between DOS and Hi-Rez
   	if(((dbPixelClock > 250.0) && ( pVprm->width >= 1280)) || (wHTotal >= 256))
//   	if ((dbPixelClock > 250.0) && ( pVprm->width >= 1280))
         bClockDouble = TRUE;
      }
   else
      {
	   if((dbPixelClock > 160.0) && ( pVprm->width >= 1280))
         bClockDouble = TRUE;
      }

   if (bClockDouble)
	{
		byDacMode = 1;
		wHVisible >>= 1;		
		wHBlankTime	= (wHBlankTime & 0x1) + (wHBlankTime>>1);
		wHSyncStart	= (wHSyncStart & 0x1) + (wHSyncStart>>1);
		wHSyncTime	= (wHSyncTime & 0x1) + (wHSyncTime>>1);
		wHTotal		= wHVisible + wHBlankTime;
      wHBlankStart = (wHBlankStart & 0x1) + (wHBlankStart>>1);
	}
    
   if ((IS_NAPALM(pDev->dwVendorDeviceID)) && (wHTotal >= 256))
      bBigger256 = TRUE;

	// Lower 8 bits of Horizontal Total
	byHTotal = 0xff & (wHTotal - 5);

	// Lower 8 bits of the horizontal display enable end
	byHorDispEnEnd = 0xff & (wHVisible - 1);

	// Lower 8 bits of the horizontal Blanking start
	byHBlankStart = 0xff & (wHBlankStart-1);

	// Finishing the Horizontal Blank End time
	// Assuming DisplayEnableSkew is 0 and Compatibility Read is on

   // Use a wrap trick to get max HBlank End
   if (bBigger256)
      {
   	byHBlankEnd = 0x80;
	   byHSyncEnd = 0x0;
      }
   else
      {
   	byHBlankEnd = 0x1f&(wHBlankTime + ((wHBlankStart-1) &0x3f)) | 0x80;
   	byHSyncEnd = (0x20&(wHBlankTime + ((wHBlankStart-1)&0x3f)))<<2;
      }

	// Filling in the lower 8 bits of the Horizontal Sync start value
	byHSyncStart = 0xff & (wHSyncStart-1);

	// Finishing the Horzontal Sync End time
	// Assuming HorizontalSyncSkew is 0
	byHSyncEnd |= (0x1f&(wHSyncTime + wHSyncStart-1));

	// Getting the lower 8 bits of the total # of vertical lines
	byVTotal = 0xff & (wVTotal - 2);

	// Filling in the Overflow register
	// Assuming the LineComp bit 8 is 1
	byOverflow = ((0x100 & (wVTotal-2))>>8) | ((0x100 & (wVVisible-1))>>7) |
		((0x100 & (wVSyncStart-1))>>6) | ((0x100 & (wVBlankStart-1))>>5) |
		0x10 | ((0x200 & (wVTotal-2))>>4) | ((0x200 & (wVVisible-1))>>3) |
		((0x200 & (wVSyncStart-1))>>2);

	// Filling in the MaxScanLine register
	// Assuming the LineComp bit 9 is 1
	byMaxLineScan = ((0x200 & (wVBlankStart-1))>>4) | 0x40 | 
		(byScanLineDoubled ? 0x80 : 0x0);

	// Filling in the lower 8 bits of Vertical Sync Start
	byVSyncStart = 0xff & (wVSyncStart-1);

	// Filling in the Vertical Sync End time
	// Assuming that we are enabling the Vertical Interrupt, allowing access to CR0-7,
	// and not clearing the interrupt
	byVSyncEnd = (0x0f & (wVSyncTime + wVSyncStart-1)) | 0x20;

	// Filling in the in lower 8 bits of the visible vertical lines
	byVertDispEnEnd = 0xff & (wVVisible-1);

	// Filling in the lower 8 bits of Vertical Blank Start
	byVBlankStart = 0xff & (wVBlankStart - 1);

	// Filling in the Vertical Blank End time
	byVBlankEnd = 0xff & (wVBlankTime + wVBlankStart - 1);

	// Filling in the Horizontal Extensions
   // if Bigger then 256 use wrap trick to set HBlank to zero
   if (bBigger256)
      {
   	byHExtensions = ((0x100 & (wHTotal-5))>>8) |
	   	((0x100 & (wHVisible-1))>>6) | ((0x100 & (wHBlankStart-1))>>4) |
		   ((0x100 & (wHSyncStart-1))>>2) |
		   ((0x20&(wHSyncTime + wHSyncStart-1))<<2);
      }
   else
      {
   	byHExtensions = ((0x100 & (wHTotal-5))>>8) |
	   	((0x100 & (wHVisible-1))>>6) | ((0x100 & (wHBlankStart-1))>>4) |
		   ((0x40&(wHBlankTime + ((wHBlankStart-1)&0x3f)))>>1) |
		   ((0x100 & (wHSyncStart-1))>>2) |
		   ((0x20&(wHSyncTime + wHSyncStart-1))<<2);
      }

	// Filling in the Vertical Extensions
	byVExtensions = ((0x400 & (wVTotal-2))>>10) |
		((0x400 & (wVVisible-1))>>8) | ((0x400 & (wVBlankStart-1))>>6) | 
		((0x400 & (wVSyncStart-1))>>4);

	// Filling in Miscellaneous Output register
	// Asuming Clock Select is dictated by the Programmable PLL,
	// RAM is enabled, and using color mode CRTC addressing
	byMiscOutput = 0xf | byHSyncPolarity | byVSyncPolarity;

	if ( pVprm->CharWidth == 9 )
		bySeqDotClk = 0x20;
	else
		bySeqDotClk = 0x21;

	oldoverflow = 1000.0;  // big number
	bestn = bestm = bestk = 0;

	if(dbPixelClock > 150.0)
		k=1;
	else if (dbPixelClock > 65.0)
		k=2;
	else
		k=3;

	// Find the correct pllTable value for the pixel clock
	for(m=1 ; m<64; m++)
	{
		// m should not start at 0 (Found empirically that m=0 will cause the
		// equation to be 14.31818*(n+2)/(1*2^k) So m adds 1 but not 2 here.)
		// As per Yancy's email on Feb. 8, 1999, use only m values >=10
		if ( (dbPixelClock > 36.0) && (m>10))
			break;
		if ( (dbPixelClock > 200.0) && (m>5))
			break;

		test = (dbPixelClock) * ((double) m + 2.0) * 
			((double) (8>>(3-k))) / 14.31818;

		if(test>257.0)
			continue;

		rndtest = Round(test);

		newoverflow = dbPixelClock - 14.31818*rndtest/(((double) m + 2.0) * ((double) (8>>(3-k))));
		if(newoverflow < 0.0)
			newoverflow *= -1.0;

#ifdef DEBUG
		if((newoverflow / dbPixelClock < 0.005) && (wPossiblePllCount < 256))
		{
			wPossiblePllCtrl0values[wPossiblePllCount++] = ( (FloatToInt(rndtest-2.0)) <<8) | (m<<2) | k;
		}

		if(newoverflow == oldoverflow)
			iGoodTimings++;
#endif
			
		// The first values found will give better results than the older values
		if(newoverflow < oldoverflow)
		{
#ifdef DEBUG
			if(newoverflow != oldoverflow)
				iGoodTimings = 0;
#endif
			bestm = m;
			bestn = FloatToInt(rndtest - 2.0);
			bestk = k;
			oldoverflow = newoverflow;
		}
	}


#ifdef DEBUG
	answer = (14.31818f * ( ((float) bestn) + 2.0f)) / 
		( (((float) bestm) + 2.0f) * ((float) (8>>(3-bestk))) );
#endif

	wPllCtrl0 = (bestn<<8) | (bestm<<2) | bestk;

//  Define this if you want to see mode/pll information
//	Deubg_Printf("\n%dx%d @ %d Hz\n", pVpc->width, pVpc->height, pVpc->refresh);
//	Deubg_Printf("0x%04x, n=%d, m=%d, k=%d, 10*VCO=%d\n", wPllCtrl0, bestn, bestm, bestk, ((1431818*(bestn+2))/(bestm+2))/10000 );

	crtc_table[0]  = byHTotal;
	crtc_table[1]  = byHorDispEnEnd;
	crtc_table[2]  = byHBlankStart;
	crtc_table[3]  = byHBlankEnd;
	crtc_table[4]  = byHSyncStart;
	crtc_table[5]  = byHSyncEnd;
	crtc_table[6]  = byVTotal;
	crtc_table[7]  = byOverflow;
	crtc_table[8]  = byMaxLineScan;
	crtc_table[9]  = byVSyncStart;
	crtc_table[10] = byVSyncEnd;
	crtc_table[11] = byVertDispEnEnd;
	crtc_table[12] = byVBlankStart;
	crtc_table[13] = byVBlankEnd;
	crtc_table[14] = byHExtensions;
	crtc_table[15] = byVExtensions;
	crtc_table[16] = byMiscOutput;
	crtc_table[17] = bySeqDotClk;	// SR1
	crtc_table[18] = wPllCtrl0 & 0xff;
	crtc_table[19] = wPllCtrl0 >> 8;
	crtc_table[20] = byDacMode;

	FPU_State( FPU_FUNCTION_RESTORE );


}



