/* $Header: modelist.h, 7, 10/11/00 8:51:28 PM, Brent$ */
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
** File name:   modelist.h
**
** Description: MODEINFO structure and supporting macros.
**
** $Revision: 7$ 
** $Date: 10/11/00 8:51:28 PM$
**
** $History: modelist.h $
** 
** *****************  Version 9  *****************
** User: Rbissell     Date: 8/07/99    Time: 3:11p
** Updated in $/devel/h5/Win9x/dx/dd16
** tvout merge from V3_OEM_100
** 
** *****************  Version 8  *****************
** User: Cshaw        Date: 7/19/99    Time: 10:35a
** Updated in $/devel/h5/Win9x/dx/dd16
** Took out "#if defined(h5)" for 2ppc modes.
** 
** *****************  Version 6  *****************
** User: Cshaw        Date: 4/20/99    Time: 12:24p
** Updated in $/devel/h3/Win95/dx/dd16
** Added Napalm's enabling of 2-pixel-per-clock rendering for 16bpp modes
** of "non-high" bandwidth requirements.  We'll need to tweak which modes
** are enabled after we get Silicon back.
** 
** *****************  Version 5  *****************
** User: Stb_jcampbel Date: 3/05/99    Time: 2:27p
** Updated in $/devel/h3/win95/dx/dd16
** Remove certain desktop modes from list of valid D3D modes.
** 
** *****************  Version 4  *****************
** User: Michael      Date: 12/29/98   Time: 2:36p
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB unified header.
** 
** 3     12/21/98 11:38a Stuartb
** Added initialization for tvout and flat panel modes.
** 
** 2     6/30/98 1:48p Ken
** 16-bit clean fix
** 
** 1     4/22/98 2:51p Andrew
** Mode Table data structure and defines
** 
*/

#ifndef _MODELIST_H_
#define _MODELIST_H_

#define IS_VALID_MODE    (0x80000000L)        // Mode + Monitor is Valid
#define SCANLINE_DBL     (0x40000000L)        // Modes uses Scan Line Double
#define IS_DDRAW_MODE  	 (0x20000000L)        // Mode is OK for DDRAW.   // DYNAMIC MODE TABLE 
#define TVOUT_DESKTOP  	 (0x10000000L)        // Mode is OK for TV, for DESKTOP 
#define TVOUT_DDRAW    	 (0x08000000L)        // Mode is OK for TV, but for DDRAW only.
#define TVOUT_NTSC     	 (0x04000000L)        // Mode is OK for NTSC 	// DYNAMIC MODE TABLE 
#define TVOUT_PAL      	 (0x02000000L)        // Mode is OK for PAL,	// DYNAMIC MODE TABLE 
#define LCD_DESKTOP    	 (0x01000000L)        // Mode is OK for LCD, for DESKTOP    // DYNAMIC MODE TABLE 
#define LCD_DDRAW      	 (0x00800000L)        // Mode is OK for LCD, for DDRAW only // DYNAMIC MODE TABLE 
#define INTERLACED     	 (0x00400000L)        // Mode is Interlaced,    // DYNAMIC MODE TABLE 
#define IS_LCD_MODE      (0x00200000L)        // Obsolete, kept in to prevent compile error // DYNAMIC MODE TABLE 
#define FAILS_ON_VOODOO3 (0x00100000L)        // Mode does not work on a Voodoo3
#define REND2PIX_PER_CLK (0x00080000L)        // Mode should be used with 2 pixel-per-clock rendering

typedef struct modeinfo
{
    DWORD       dwWidth;                // width (in pixels) of mode
    DWORD       dwHeight;               // height (in pixels) of mode
    LONG        lPitch;                 // pitch (in bytes) of mode
    DWORD       dwBPP;                  // bits per pixel
    DWORD       dwFlags;                // flags
    WORD        wHorz;                  // Horz Refresh Rate <KHz * 10>
    WORD        wVert;                  // Vert Refresh Rate <Hz>
    DWORD       dwDFPWidth;             // begin DYNAMIC MODE TABLE
    DWORD       dwDFPHeight;
	DWORD 		HTotal;    	 			
    DWORD 		HSyncStart;    		 
    DWORD 		HSyncEnd;    
    DWORD 		VTotal;    
    DWORD 		VSyncStart;    
    DWORD 		VSyncEnd;    
    DWORD 		CRTCflags;    
    DWORD 		PixelClock;    
    DWORD 		CharWidth;    
    DWORD 		UseGTF;    			   
    DWORD 		UseAltTiming;    			   // altTimings use by DFP is obsolete DanO 5/17/00
#ifdef REAL_NET
    WORD       wVert100;                  // Accurate Vertical Refresh * 100 for Real Networks
#endif
    BYTE        AltTiming[CRTC_TABLE_SIZE];			// End DYNAMIC MODE TABLE
 
} MODEINFO, * PMODEINFO, FAR * LPMODEINFO;

// STBNW JAC 3-4-99  Addresses PRS's 4641 & 4675 - Eliminate certain
//                   desktop modes from D3D valid modes.
//
#define DESKTOP_VALID	(0x00000001L)
#define DDRAW_VALID		(0x00000002L)

typedef struct desktopmodeinfo
{
    DWORD       dwWidth;                // width (in pixels) of mode
    DWORD       dwHeight;               // height (in pixels) of mode
    DWORD       dwFlags;                // flags
} DESKTOPMODEINFO, * PDESKTOPMODEINFO, FAR * LPDESKTOPMODEINFO;
// STBNW end

#define ADAPTER_DEFAULT (0)
#define OPTIMAL (-1)

#endif

extern MODEINFO FAR * ModeList;		  // DYNAMIC MODE TABLE
