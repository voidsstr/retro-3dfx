/* -*-c++-*- */
/* $Header: thunk32.c, 3, 10/11/00 8:55:16 PM, Brent$ */
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
** File name:   thunk32.c
**
** Description: the 32-bit side of the 16->32 GDI thunker.
**
** $Revision: 3$
** $Date: 10/11/00 8:55:16 PM$
**
** $History: thunk32.c $
** 
** *****************  Version 33  *****************
** User: Cwilcox      Date: 1/22/99    Time: 2:09p
** Updated in $/devel/h3/Win95/dx/minivdd
** Minor revisions to clean up compiler warnings.
** 
** *****************  Version 32  *****************
** User: Michael      Date: 1/15/99    Time: 9:25a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 31  *****************
** User: Ken          Date: 7/24/98    Time: 10:38p
** Updated in $/devel/h3/win95/dx/minivdd
** changes to allow 2d driver to run properly synchronized with an AGP
** command fifo (although video memory fifo is still used when the desktop
** has the focus, e.g., a fullscreen 3d app isn't in the foreground)
** 
** *****************  Version 30  *****************
** User: Andrew       Date: 7/23/98    Time: 12:23p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Some flags for BitmapBits
** 
** *****************  Version 29  *****************
** User: Ken          Date: 7/09/98    Time: 11:55a
** Updated in $/devel/h3/win95/dx/minivdd
** added some NULLDRIVER ability as well as a performance measuring 
** technique PERFNOP (PERFNOP was here before, I'm just adding it
** officially to the makefile).
** 
** *****************  Version 28  *****************
** User: Michael      Date: 6/22/98    Time: 7:19a
** Updated in $/devel/h3/Win95/dx/minivdd
** ChrisE's (IGX) addition of code for FxStretchDIBits.  Fixes 1893.
** Also, modifies THUNK_PROLOG to use EBX instead of EDX.
** 
** *****************  Version 27  *****************
** User: Michael      Date: 5/15/98    Time: 4:34p
** Updated in $/devel/h3/Win95/dx/minivdd
** Initialize a flat32 pointer to the screen's pdevice in InitThunks32
** 
** *****************  Version 26  *****************
** User: Ken          Date: 5/13/98    Time: 8:08a
** Updated in $/devel/h3/win95/dx/minivdd
** fast enter/leave turned on, working with SSB.
** also, bumped up the max # of device bitmap allocations by 4x
** 
** *****************  Version 25  *****************
** User: Ken          Date: 4/30/98    Time: 11:26a
** Updated in $/devel/h3/win95/dx/minivdd
** updated enter/leave to always check/set the busy bit on the 
** screen's pdevice, not the destination bitmap's pdevice ('cuz it might
** be a device bitmap.
** 
** *****************  Version 24  *****************
** User: Ken          Date: 4/27/98    Time: 5:35p
** Updated in $/devel/h3/win95/dx/minivdd
** complete (well, almost) enter/leave implementation for gdi32
** 
** *****************  Version 23  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/


//: thunk32.c


#include "thunk32.h"

VOID __stdcall InitThunks32( DWORD, DWORD, DWORD, DWORD, DWORD );
VOID __stdcall BitBlt32( DWORD, VOID* );
VOID __stdcall ColorInfo32( VOID*, VOID* );
VOID __stdcall Control32( VOID*, VOID* );
VOID __stdcall Disable32( VOID*, VOID* );
VOID __stdcall Enable32( VOID*, VOID* );
VOID __stdcall EnumDFonts32( VOID*, VOID* );
VOID __stdcall EnumObj32( VOID*, VOID* );
VOID __stdcall Output32( DWORD, VOID* );
VOID __stdcall Pixel32( VOID*, VOID* );
VOID __stdcall RealizeObject32( VOID*, VOID* );
VOID __stdcall StrBlt32( VOID*, VOID* );
VOID __stdcall ScanLR32( VOID*, VOID* );
VOID __stdcall DeviceMode32( VOID*, VOID* );
VOID __stdcall ExtTextOut32( DWORD, VOID* );
VOID __stdcall GetCharWidth32( VOID*, VOID* );
VOID __stdcall DeviceBitmap32( VOID*, VOID* );
VOID __stdcall FastBorder32( VOID*, VOID* );
VOID __stdcall SetAttribute32( VOID*, VOID* );
VOID __stdcall DibBlt32( VOID*, VOID* );
VOID __stdcall DeviceBitmapBits32( DWORD, VOID* );
VOID __stdcall CreateDIBitmap32( VOID*, VOID* );
VOID __stdcall DibToDevice32( VOID*, VOID* );
VOID __stdcall SetDIBitsToDevice32( DWORD, VOID* );
VOID __stdcall SetPalette32( VOID*, VOID* );
VOID __stdcall GetPalette32( VOID*, VOID* );
VOID __stdcall SetPaletteTranslate32( VOID*, VOID* );
VOID __stdcall GetPaletteTranslate32( VOID*, VOID* );
VOID __stdcall UpdateColors32( VOID*, VOID* );
VOID __stdcall StretchBlt32( DWORD, VOID* );
VOID __stdcall StretchDIBits32( DWORD, VOID* );
VOID __stdcall SelectBitmap32( DWORD, VOID* );
VOID __stdcall BitmapBits32( DWORD, VOID* );
VOID __stdcall ReEnable32( VOID*, VOID* );
VOID __stdcall MapSharedData32( DWORD, VOID* );
VOID __stdcall initFifo32( DWORD , VOID*  );
extern DWORD dwNumDevices;
extern DWORD dwDevNode;
void SetLPFromDevNode(DWORD, GLOBALDATA * );
GLOBALDATA * FindLPFromDevNode(DWORD);

DWORD func_table[] =
{
    (DWORD) InitThunks32,               // 00
    (DWORD) BitBlt32,                   // 01
    (DWORD) 0, //ColorInfo32,           // 02
    (DWORD) 0, //Control32,             // 03
    (DWORD) 0, //Disable32,             // 04
    (DWORD) 0, //Enable32,              // 05
    (DWORD) 0, //EnumDFonts32,          // 06
    (DWORD) 0, //EnumObj32,             // 07
    (DWORD) Output32,                   // 08
    (DWORD) 0, //Pixel32,               // 09
    (DWORD) 0, //RealizeObject32,       // 10
    (DWORD) 0, //StrBlt32,              // 11
    (DWORD) 0, //ScanLR32,              // 12
    (DWORD) 0, //DeviceMode32,          // 13
    (DWORD) ExtTextOut32,               // 14
    (DWORD) 0, //GetCharWidth32,        // 15
    (DWORD) 0, //DeviceBitmap32,        // 16
    (DWORD) 0, //FastBorder32,          // 17
    (DWORD) 0, //SetAttribute32,        // 18
    (DWORD) 0, //DibBlt32,              // 19
    (DWORD) DeviceBitmapBits32,         // 20
    (DWORD) 0, //CreateDIBitmap32,      // 21
    (DWORD) 0, //DibToDevice32,         // 22
    (DWORD) SetDIBitsToDevice32,        // 23
    (DWORD) 0, //SetPalette32,          // 24
    (DWORD) 0, //GetPalette32,          // 25
    (DWORD) 0, //SetPaletteTranslate32, // 26
    (DWORD) 0, //GetPaletteTranslate32, // 27
    (DWORD) 0, //UpdateColors32,        // 28
    (DWORD) StretchBlt32,               // 29
    (DWORD) StretchDIBits32,            // 30
    (DWORD) SelectBitmap32,             // 31
    (DWORD) BitmapBits32,               // 32
    (DWORD) 0, //ReEnable32,            // 33
	(DWORD) MapSharedData32 ,           // 34
	(DWORD) initFifo32                  // 35
};

#ifdef DEBUG
DWORD UseDibEng = 0;		// =1, call dib eng, =0, use 3Dfx code
#endif

DWORD dwLdtPtr;
BOOL  bPunt;
DWORD count;
GLOBALDATA * lpDriverData ;
GLOBALDATA  DriverData ;
DWORD dwDevNode;

DWORD hwAll=1;    
DWORD hwBlt=1; hwBitBltSS=1, hwBitBltPS=1, hwBitBltHS=1 ;
DWORD hwStretchBlt=1; hwStretchBltSS=1; hwStretchBltHS=1; hwStretchBltPS=1;

DWORD hwText=1, hwTextLPDX=1; hwTextNoLPDX=1;
DWORD hwOutput =1;
DWORD hwAltPolygon=1, hwWindPolygon=0;
DWORD hwPolyline=1; 
DWORD hwRect=1;
DWORD hwPolyScanline=1, hwScanline =1 ;
DWORD hwSetDIBitsToDevice=1;
DWORD hwDeviceBitmapBits=1;
DWORD hwStretchDIBits=1;
DWORD hwDibBlt =1 ; hwDibToDevice=1;
DWORD hwpollStall=0, hwbreakStall=0;
DWORD hwpciFree=0;
DWORD pollCount=0xfffffffd;
DWORD hwBitmapBits = 0;
DWORD hwBitmapBitsHS = 0;
DWORD hwBitmapBitsSS = 0;
DWORD hwBitmapBitsSH = 0;


VOID InitDebugLevel(VOID);


/*----------------------------------------------------------------------
Function name:  InitThunks32

Description:    Do initialization for thunking on the 32-bit side.
                
Information:    

Return:         VOID
----------------------------------------------------------------------*/
__declspec( naked ) 
VOID WINAPI InitThunks32
(
    DWORD dwDummy,
    DWORD dwLocalDevNode,
    DWORD dwPfnGlobalData,
    DWORD dwLdtAliasPtr,
    DWORD dwPfnCSIM
)
{
    VOID* pParams;  // this makes THUNK_PROLOG work
    DWORD lpPDevice32;

    THUNK_PROLOG;

    dwLdtPtr = dwLdtAliasPtr;
    pfnCSIM = dwPfnCSIM;

    FarToFlat(dwPfnGlobalData,  lpDriverData);

    // make this screen's pdevice conveniently accessible to GDI32 functions
    //
    FarToFlat(lpDriverData->lpPDevice, lpPDevice32);
    lpDriverData->lpDeFlags = (DWORD *) &((LPDIBENGINE)lpPDevice32)->deFlags;

    // set up a flat32 screen pdevice accessible to GDI32 functions
    //
    lpDriverData->lpPDevice32 = lpPDevice32;

    // set up the mapping from this devnode to its lpDriverData
    // 
    SetLPFromDevNode(dwLocalDevNode, lpDriverData);

    // put the 2d regs in a known state
    //
    InitRegs();

    // get the command fifo up & running
    // 
    InitFifo(lpDriverData->fifoStart, lpDriverData->fifoSize);

#ifdef TRACE
    InitDebugLevel();
#endif

    THUNK_EPILOG;
}

#ifdef NULLDRIVER
FxU32 nullAll = 0;
FxU32 nullBitBlt = 0;
FxU32 nullStretchBlt = 0;
FxU32 nullText = 0;
FxU32 nullOutput = 0;
FxU32 nullDevBit = 0;
FxU32 nullDibToDev = 0;
FxU32 nullStretchDib = 0;
FxU32 nullSelectBitmap = 0;
FxU32 nullBitmapBits = 0;
#endif // #ifdef NULLDRIVER


/*----------------------------------------------------------------------
Function name:  BitBlt32

Description:    Perform THUNK_PROLOG, dispatch to FxBitBlt, and
                perform THUNK_EPILOG.
                
Information:    Sets lpDriverData for multimonitor.

Return:         VOID
----------------------------------------------------------------------*/
#pragma optimize("", off)

__declspec( naked )
VOID WINAPI BitBlt32
(
    DWORD         dwDummy,
    BitBltParams* pParams
)
{
    THUNK_PROLOG

    // This is how we find which device to use in a Multi-Monitor Situation
    if (dwNumDevices > 1)
      lpDriverData = (GLOBALDATA *)FindLPFromDevNode(dwDevNode);

    bPunt = FALSE;

#ifdef NULLDRIVER
    if (!nullAll && !nullBitBlt)
#endif // #ifdef NULLDRIVER
	(DWORD) pParams = FxBitBlt( pParams );

    if (bPunt)
        dwDummy |= PUNT_BIT;

    THUNK_EPILOG
}


/*----------------------------------------------------------------------
Function name:  StretchBlt32

Description:    Perform THUNK_PROLOG, dispatch to FxStretchBlt, and
                perform THUNK_EPILOG.
                
Information:    Sets lpDriverData for multimonitor.

Return:         VOID
----------------------------------------------------------------------*/
__declspec( naked )
VOID WINAPI StretchBlt32
(
    DWORD         dwDummy,
    StretchBltParams* pParams
)
{
    THUNK_PROLOG

    // This is how we find which device to use in a Multi-Monitor Situation
    if (dwNumDevices > 1)
      lpDriverData = (GLOBALDATA *)FindLPFromDevNode(dwDevNode);

    bPunt = FALSE;

#ifdef NULLDRIVER
    if (!nullAll && !nullStretchBlt)
#endif // #ifdef NULLDRIVER
	(DWORD) pParams = FxStretchBlt( pParams );

    if (bPunt)
        dwDummy |= PUNT_BIT;

    THUNK_EPILOG
}


/*----------------------------------------------------------------------
Function name:  ExtTextOut32

Description:    Perform THUNK_PROLOG, dispatch to FxExtTextOut, and
                perform THUNK_EPILOG.
                
Information:    Sets lpDriverData for multimonitor.

Return:         VOID
----------------------------------------------------------------------*/
__declspec( naked )
VOID WINAPI ExtTextOut32
(
    DWORD             dwDummy,
    ExtTextOutParams* pParams
)
{
    THUNK_PROLOG

    // This is how we find which device to use in a Multi-Monitor Situation
    if (dwNumDevices > 1)
      lpDriverData = (GLOBALDATA *)FindLPFromDevNode(dwDevNode);

    bPunt = FALSE;

#ifdef NULLDRIVER
    if (!nullAll && !nullText)
#endif // #ifdef NULLDRIVER
	(DWORD) pParams = FxExtTextOut( pParams );

    if (bPunt)
        dwDummy |= PUNT_BIT;

    THUNK_EPILOG
}


/*----------------------------------------------------------------------
Function name:  Output32

Description:    Perform THUNK_PROLOG, dispatch to FxOutput, and
                perform THUNK_EPILOG.
                
Information:    Sets lpDriverData for multimonitor.

Return:         VOID
----------------------------------------------------------------------*/
__declspec( naked )
VOID WINAPI Output32
(
    DWORD         dwDummy,
    OutputParams * pParams
)
{
    THUNK_PROLOG

    // This is how we find which device to use in a Multi-Monitor Situation
    if (dwNumDevices > 1)
      lpDriverData = (GLOBALDATA *)FindLPFromDevNode(dwDevNode);

    bPunt = FALSE;

#ifdef NULLDRIVER
    if (!nullAll && !nullOutput)
#endif // #ifdef NULLDRIVER
	(DWORD) pParams = FxOutput( pParams );

    if (bPunt)
        dwDummy |= PUNT_BIT;

    THUNK_EPILOG
}


/*----------------------------------------------------------------------
Function name:  DeviceBitmapBits32

Description:    Perform THUNK_PROLOG, dispatch to FxDeviceBitmapBits,
                and perform THUNK_EPILOG.
                
Information:    Sets lpDriverData for multimonitor.

Return:         VOID
----------------------------------------------------------------------*/
#define DAC_DPMS_BITS (SST_DAC_DPMS_ON_VSYNC | SST_DAC_FORCE_VSYNC | SST_DAC_DPMS_ON_HSYNC | SST_DAC_FORCE_HSYNC)

__declspec( naked ) VOID WINAPI 
DeviceBitmapBits32(DWORD                   dwDummy,
		   DeviceBitmapBitsParams* pParams)
{
    THUNK_PROLOG;
    
    // This is how we find which device to use in a Multi-Monitor Situation
    if (dwNumDevices > 1)
      lpDriverData = (GLOBALDATA *)FindLPFromDevNode(dwDevNode);

    bPunt = FALSE;

    (DWORD) pParams = FxDeviceBitmapBits( pParams );
    
    if (bPunt)
    {
        dwDummy |= PUNT_BIT;

    if  (!(GET(_FF(lpIOregs)->dacMode) & DAC_DPMS_BITS)) // ignore wait for idle if we are in DPMS power save.
     	FXWAITFORIDLE();
    }
    
    THUNK_EPILOG;
}


/*----------------------------------------------------------------------
Function name:  SetDIBitsToDevice32

Description:    Perform THUNK_PROLOG, dispatch to FxSetDIBitsToDevice,
                and perform THUNK_EPILOG.
                
Information:    Sets lpDriverData for multimonitor.

Return:         VOID
----------------------------------------------------------------------*/
__declspec( naked )
VOID WINAPI SetDIBitsToDevice32
(
    DWORD                    dwDummy,
    SetDIBitsToDeviceParams* pParams
)
{
    THUNK_PROLOG

    // This is how we find which device to use in a Multi-Monitor Situation
    if (dwNumDevices > 1)
      lpDriverData = (GLOBALDATA *)FindLPFromDevNode(dwDevNode);

    bPunt = FALSE;

#ifdef NULLDRIVER
    if (!nullAll && !nullDibToDev)
#endif // #ifdef NULLDRIVER
	(DWORD) pParams = FxSetDIBitsToDevice( pParams );

    if (bPunt)
        dwDummy |= PUNT_BIT;

    THUNK_EPILOG
}


/*----------------------------------------------------------------------
Function name:  StretchDIBits32

Description:    Perform THUNK_PROLOG, dispatch to FxStretchDIBits,
                and perform THUNK_EPILOG.
                
Information:    Sets lpDriverData for multimonitor.

Return:         VOID
----------------------------------------------------------------------*/
__declspec( naked )
VOID WINAPI StretchDIBits32
(
    DWORD                    dwDummy,
    StretchDIBitsParams* pParams
)
{
    THUNK_PROLOG

    // This is how we find which device to use in a Multi-Monitor Situation
    if (dwNumDevices > 1)
      lpDriverData = (GLOBALDATA *)FindLPFromDevNode(dwDevNode);

    bPunt = FALSE;

#ifdef NULLDRIVER
    if (!nullAll && !nullStretchDib)
#endif // #ifdef NULLDRIVER
	(DWORD) pParams = FxStretchDIBits( pParams );

    if (bPunt)
        dwDummy |= PUNT_BIT;

    THUNK_EPILOG
}


/*----------------------------------------------------------------------
Function name:  SelectBitmap32

Description:    Perform THUNK_PROLOG, dispatch to FxSelectBitmap,
                and perform THUNK_EPILOG.
                
Information:    Sets lpDriverData for multimonitor.

Return:         VOID
----------------------------------------------------------------------*/
__declspec( naked )
VOID WINAPI SelectBitmap32
(
    DWORD               dwDummy,
    SelectBitmapParams* pParams
)
{
    THUNK_PROLOG

    // This is how we find which device to use in a Multi-Monitor Situation
    if (dwNumDevices > 1)
      lpDriverData = (GLOBALDATA *)FindLPFromDevNode(dwDevNode);

    bPunt = FALSE;

#ifdef NULLDRIVER
    if (!nullSelectBitmap)
#endif // #ifdef NULLDRIVER
	(DWORD) pParams = FxSelectBitmap( pParams );

    if (bPunt)
        dwDummy |= PUNT_BIT;

    THUNK_EPILOG
}


/*----------------------------------------------------------------------
Function name:  BitmapBits32

Description:    Perform THUNK_PROLOG, dispatch to FxBitmapBits, and
                perform THUNK_EPILOG.
                
Information:    Sets lpDriverData for multimonitor.

Return:         VOID
----------------------------------------------------------------------*/
__declspec( naked )
VOID WINAPI BitmapBits32
(
    DWORD             dwDummy,
    BitmapBitsParams* pParams
)
{
    THUNK_PROLOG

    // This is how we find which device to use in a Multi-Monitor Situation
    if (dwNumDevices > 1)
      lpDriverData = (GLOBALDATA *)FindLPFromDevNode(dwDevNode);

    bPunt = FALSE;

#ifdef NULLDRIVER
    if (!nullBitmapBits)
#endif // #ifdef NULLDRIVER
	(DWORD) pParams = FxBitmapBits( pParams );

    if (bPunt)
        dwDummy |= PUNT_BIT;

    THUNK_EPILOG
}


/*----------------------------------------------------------------------
Function name:  MapSharedData32

Description:    Perform THUNK_PROLOG, perform a FarToFlat on
                lpDriverData and perform THUNK_EPILOG.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
#pragma optimize("", on)

__declspec( naked )
VOID WINAPI MapSharedData32
(
    DWORD             dwDummy,
    MapSharedDataParams * pParams
)
{
    THUNK_PROLOG

	DEBUG_FIX

	
	FarToFlat(pParams->lpDriverData,lpDriverData);


    THUNK_EPILOG
}


/*----------------------------------------------------------------------
Function name:  initFifo32

Description:    Dummy call.  This call is perfromed so the mode set
                can init the fifo.
Information:    

Return:         VOID
----------------------------------------------------------------------*/
__declspec( naked )
VOID WINAPI initFifo32
(
    DWORD             dwDummy,
    initFifoParams * pParams
)
{
	DWORD temp;

    THUNK_PROLOG

	DEBUG_FIX

	temp = pParams->size;
	

    THUNK_EPILOG
}



