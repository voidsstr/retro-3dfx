/* -*-c++-*- */
/* $Header: dibblt.h, 9, 10/11/00 8:54:14 PM, Brent$ */
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
** File name:   dibblt.h
**
** Description: Macros and support for DibBlt functions.
**
** $Revision: 9$
** $Date: 10/11/00 8:54:14 PM$
**
** $History: dibblt.h $
** 
** *****************  Version 14  *****************
** User: Cwilcox      Date: 2/03/99    Time: 5:17p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added missing 32bpp source cases to fxStretchDIBits.
** 
** *****************  Version 13  *****************
** User: Stb_srogers  Date: 1/29/99    Time: 8:02a
** Updated in $/devel/h3/win95/dx/minivdd
** 
** *****************  Version 12  *****************
** User: Andrew       Date: 1/20/99    Time: 4:35p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added New function prototype to Handle 555 to 565 conversion
** 
** *****************  Version 11  *****************
** User: Michael      Date: 1/05/99    Time: 7:51a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 10  *****************
** User: Michael      Date: 12/24/98   Time: 10:28a
** Updated in $/devel/h3/Win95/dx/minivdd
** ChrisE's (igx) optimization for FxStretchDIBits.  High-End Graphics
** WB9x tests have all 8bpp StretchDIB calls as one-to-one stretches.
** Test for this case and perform a screen blt.  For High-End Graphics,
** gains 26 Winmarks.  (About a 15.2% increase on my PII-300.)
** 
** *****************  Version 9  *****************
** User: Michael      Date: 8/21/98    Time: 12:56p
** Updated in $/devel/h3/Win95/dx/minivdd
** Add MarkL's (igx) Host2Scrn8To16ConvBlt functionality.  Needed to
** fix #1209.
** 
** *****************  Version 8  *****************
** User: Michael      Date: 6/22/98    Time: 7:15a
** Updated in $/devel/h3/Win95/dx/minivdd
** ChrisE's (IGX) addition of FxStretchDIBits and supporting
** functionality.  Fixes 1893.
** 
** *****************  Version 7  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
** 
** *****************  Version 6  *****************
** User: Ken          Date: 4/15/98    Time: 10:39a
** Updated in $/devel/h3/win95/dx/minivdd
** 24 and 32bpp 4-bit source dib conversion routine added
**
*/


/***************************************************************************
 *
 * functions in dibblt.c
 *
 ***************************************************************************/
#ifdef PERF_NEWDIB

int StretchHost2Scrn8To32ConvBlt(  LPDIBENGINE     lpDst,
                                   LPBITMAPINFO    lpSrc,
                                   StretchDIBitsParams *lpParams ); 

int StretchHost2Scrn8To16ConvBlt(  LPDIBENGINE     lpDst,
                                   LPBITMAPINFO    lpSrc,
                                   StretchDIBitsParams *lpParams ); 
                                   
int StretchHost2Scrn16To08ConvBlt(  LPDIBENGINE     lpDst,
                           RECT FAR *      lpClipRect,
                           WORD            cScans, 
                           int             DstX,
                           int             DstY,
                           int             SrcX,
                           int             SrcY,
                           DWORD *         lpSrcData,   // buffer
                           LPBITMAPINFO    lpSrc,   
                           DWORD           lpDrawMode,
                           DWORD           lpPBrush,
                           DWORD           lpTranslate,
                           DWORD           dwROP);   
 
int Host2Scrn8bppConvBlt( LPDIBENGINE     lpDst,              // device dib
                          RECT FAR *      lpClipRect,
                          WORD            cScans, 
                          int             DstX,
                          int             DstY,                                                         
                          DWORD *         lpSrcData,          // buffer
                          LPBITMAPINFO    lpSrc );

int Host2Scrn8To16ConvBlt( LPDIBENGINE     lpDst,       // device dib
                           RECT FAR *      lpClipRect,
                           WORD            cScans, 
                           int             DstX,
                           int             DstY,
                           DWORD *         lpSrcData,   // buffer
                           LPBITMAPINFO    lpSrc );   
									
int Host2Scrn4to16ConvBlt( LPDIBENGINE     lpDst,              // device dib
                           RECT FAR *      lpClipRect,
                           WORD            cScans, 
                           int             DstX,
                           int             DstY,
		                     int             SrcX,
		                     int             SrcY,                                        
                           DWORD *         lpSrcData,          // buffer
                           LPBITMAPINFO    lpSrc, 
                           DIB_Brush8 FAR* lpPBrush,
                           DRAWMODE FAR*   lpDrawMode,
                           DWORD           dwROP );
                           
int Host2Scrn4to2432ConvBlt( LPDIBENGINE     lpDst,              // device dib
                             RECT FAR *      lpClipRect,
                             WORD            cScans, 
                             int             DstX,
                             int             DstY,
		                       int             SrcX,
		                       int             SrcY,                                        
                             DWORD *         lpSrcData,          // buffer
                             LPBITMAPINFO    lpSrc,
                             DIB_Brush8 FAR* lpPBrush,
                             DRAWMODE FAR*   lpDrawMode,
                             DWORD           dwROP );
#else    // PERF_NEWDIB

int Host2Scrn8bppConvBlt( LPDIBENGINE     lpDst,              // device dib
                          RECT FAR *      lpClipRect,
                          WORD            cScans, 
                          int             X,
                          int             Y,
                          DWORD *         lpSrcData,          // buffer
                          LPBITMAPINFO    lpSrc );   

int Host2Scrn8bppTo16bppConvBlt( LPDIBENGINE     lpDst,       // device dib
                                 RECT FAR *      lpClipRect,
                                 WORD            cScans, 
                                 int             X,
                                 int             Y,
                                 DWORD *         lpSrcData,   // buffer
                                 LPBITMAPINFO    lpSrc );   
									
int Host2Scrn4to16ConvBlt( LPDIBENGINE     lpDst,              // device dib
                          RECT FAR *      lpClipRect,
                          WORD            cScans, 
                          int             X,
                          int             Y,
                          DWORD *         lpSrcData,          // buffer
                          LPBITMAPINFO    lpSrc );

int Host2Scrn4to2432ConvBlt( LPDIBENGINE     lpDst,              // device dib
                          RECT FAR *      lpClipRect,
                          WORD            cScans, 
                          int             X,
                          int             Y,
                          DWORD *         lpSrcData,          // buffer
                          LPBITMAPINFO    lpSrc );
#endif   // PERF_NEWDIB
                            
int Host2ScrnMonoBlt( LPDIBENGINE     lpDst,              // device dib
                      RECT FAR *      lpClipRect,
                      WORD            cScans, 
                      int             X,
                      int             Y,
                      DWORD *         lpSrcData,          // buffer
                      LPBITMAPINFO    lpSrc ); 
                          
int Host2Scrn16to16StrBlt( LPDIBENGINE     lpDst,              // device dib
                          RECT FAR *      lpClipRect,
                          int             dstX,
                          int             dstY,
                          int             dstW,
                          int             dstH,
                          int             srcX,
                          int             srcY,
                          int             srcW,
                          int             srcH,
                          DWORD *         lpSrcData,          // buffer
                          LPBITMAPINFO    lpSrc );

int LoadNativeDIBScan(DWORD *lpData, int srcX, int srcW, DWORD srcDataFormat, DWORD dwordsInScanline, DWORD dwExtra, DWORD biCompression);
int LoadShrinkNativeDIBScan(DWORD *lpData, int srcX, int dstW, int xWhole, int xFrac, int xff, int savW, DWORD biCompression);

//
// Extern functions
//

extern DWORD HandleBrush(DIB_Brush8 * lpBrush, DWORD Rop, 
					DRAWMODE  * lpDrawMode);
                       

// 4k byte page
#define MAXCOLORDWORDS 1024
#define MAXMONODWORDS  1024

/* Local variable whose address will be taken */
#define TAKE_ADDRESS static


//  NUM_SCANS defines the number of scanlines to request from Makeroom 
//  function.

#define NUM_SCANS 4
extern DWORD hwSetDIBitsToDevice;
extern DWORD hwDeviceBitmapBits;

// the following defines are used to create a switch table code 
// based on the pixel format of the source and destination
// 0 and 1 have the same value because we don't handle DIBs of 
// less than 4bpp. 24 and 32 share the same code as we don't need
// to distinguish between them.

// SteveH - this is different to the pervious version where 0, 1 and 4
// all shared the same code as we didn't support DIBs below 8bpp. It
// turns out that 4bpp sources are used a lot by applications such 
// as Internet Explorer and so for performance reasons can't be punted.

#define MAKE_PIX_CODE(SrcBpp,DstBpp) (BYTE)(LookupPixCode[SrcBpp]+((LookupPixCode[DstBpp])<<DST_BPP_SHIFT))

#ifdef PERF_NEWDIB

#define BPP00   0x00
#define BPP01   0x00
#define BPP04   0x01
#define BPP08   0x02
#define BPP16   0x04
#define BPP24   0x08
#define BPP32   0x08

#else    // PERF_NEWDIB

#define BPP00   0x00
#define BPP01   0x00
#define BPP04   0x00
#define BPP08   0x01
#define BPP16   0x02
#define BPP24   0x04
#define BPP32   0x08

#endif

#define DST_BPP_SHIFT   4

#define DIB_01TO08  (BPP01+(BPP08<<DST_BPP_SHIFT))
#define DIB_04TO08  (BPP04+(BPP08<<DST_BPP_SHIFT)) 
#define DIB_08TO08  (BPP08+(BPP08<<DST_BPP_SHIFT)) 
#define DIB_16TO08  (BPP16+(BPP08<<DST_BPP_SHIFT)) 
#define DIB_24TO08  (BPP24+(BPP08<<DST_BPP_SHIFT)) 
#define DIB_32TO08  (BPP32+(BPP08<<DST_BPP_SHIFT)) 

#define DIB_01TO16  (BPP01+(BPP16<<DST_BPP_SHIFT))
#define DIB_04TO16  (BPP04+(BPP16<<DST_BPP_SHIFT)) 
#define DIB_08TO16  (BPP08+(BPP16<<DST_BPP_SHIFT)) 
#define DIB_16TO16  (BPP16+(BPP16<<DST_BPP_SHIFT)) 
#define DIB_24TO16  (BPP24+(BPP16<<DST_BPP_SHIFT)) 
#define DIB_32TO16  (BPP32+(BPP16<<DST_BPP_SHIFT)) 

#define DIB_01TO24  (BPP01+(BPP24<<DST_BPP_SHIFT))
#define DIB_04TO24  (BPP04+(BPP24<<DST_BPP_SHIFT)) 
#define DIB_08TO24  (BPP08+(BPP24<<DST_BPP_SHIFT)) 
#define DIB_16TO24  (BPP16+(BPP24<<DST_BPP_SHIFT)) 
#define DIB_24TO24  (BPP24+(BPP24<<DST_BPP_SHIFT)) 
#define DIB_32TO24  (BPP32+(BPP24<<DST_BPP_SHIFT)) 

#define DIB_01TO32  (BPP01+(BPP32<<DST_BPP_SHIFT))
#define DIB_04TO32  (BPP04+(BPP32<<DST_BPP_SHIFT)) 
#define DIB_08TO32  (BPP08+(BPP32<<DST_BPP_SHIFT)) 
#define DIB_16TO32  (BPP16+(BPP32<<DST_BPP_SHIFT)) 
#define DIB_24TO32  (BPP24+(BPP32<<DST_BPP_SHIFT)) 
#define DIB_32TO32  (BPP32+(BPP32<<DST_BPP_SHIFT)) 

