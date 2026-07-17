/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
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
** $Revision: 2$
** $Date: 10/11/00 8:41:30 PM$
*/

#include <windows.h>
#include <wing.h>
#include <3dfx.h>
#include <gdebug.h>
#include <ui.h>

static HDC wgDC;   		// WinG-DC
static HBITMAP wgBM;		// WinG bitmap

static void
wingSetup ( HWND hWnd, int w, int h )
{
    static PBITMAPINFO pbmiDib;
    static FxU8 *fbuffer;	// bits associated with wgBM
    FxU8 *data8;
    FxU32 *data32;
    int x;
    const FxU32 *lfb;

    // if necessary, malloc the BITMAPINFO structure and initialize
    // any mode independent data
    if (pbmiDib == NULL) {
	// always malloc 256 entry colormap
	pbmiDib = (PBITMAPINFO)malloc(sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD));
	if (pbmiDib == NULL) {
	    GDBG_ERROR("wingSetup","Out of memory!\n");
	    return;
	}
	if (GetDeviceCaps(GetDC(hWnd), BITSPIXEL) < 16) {
	    GDBG_ERROR("wingSetup", "display must have >= 16 bits per pixel\n");
	    exit(1);
	}
	// setup width and height as maximum SST framebuffer size
	pbmiDib->bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	pbmiDib->bmiHeader.biWidth		= grSstScreenWidth();
	pbmiDib->bmiHeader.biHeight		= -grSstScreenHeight();
	pbmiDib->bmiHeader.biPlanes		= 1;
	pbmiDib->bmiHeader.biSizeImage		= 0; /* not compressed */
	pbmiDib->bmiHeader.biXPelsPerMeter	= 0;
	pbmiDib->bmiHeader.biYPelsPerMeter	= 0;
	pbmiDib->bmiHeader.biClrUsed		= 3;
	pbmiDib->bmiHeader.biClrImportant	= 0;
	if (GetDeviceCaps(GetDC(hWnd), BITSPIXEL) == 16) {
	    pbmiDib->bmiHeader.biCompression	= BI_BITFIELDS;
	    pbmiDib->bmiHeader.biBitCount	= 16;
	    ((DWORD *)pbmiDib->bmiColors)[0] = 0xF800;
	    ((DWORD *)pbmiDib->bmiColors)[1] = 0x07E0;
	    ((DWORD *)pbmiDib->bmiColors)[2] = 0x001f;
	}
	else {
	    pbmiDib->bmiHeader.biCompression	= BI_RGB;
	    pbmiDib->bmiHeader.biBitCount	= 24;
	    // NOTE: BGR format runs 10 times faster than RGB!!!
	    ((DWORD *)pbmiDib->bmiColors)[0] = 0xFF0000;
	    ((DWORD *)pbmiDib->bmiColors)[1] = 0x00FF00;
	    ((DWORD *)pbmiDib->bmiColors)[2] = 0x0000FF;
	}

	wgDC = WinGCreateDC();		// create the WinG DC
	if (wgDC == NULL)
	    GDBG_ERROR("wingSetup","WinGCreateDC failed %d\n",GetLastError());
	wgBM = WinGCreateBitmap(wgDC, (BITMAPINFO far *)pbmiDib, &fbuffer);
	if (wgBM == NULL)
	    GDBG_ERROR("wingSetup","WinGCreateBitmap failed %d\n",GetLastError());
	SelectObject(wgDC,wgBM);
    }
    // now for as large as the GUI window is, read the framebuffer into fbuffer 
    // and expand from 16-bits to 32 bits

//    lfb = grLfbGetReadPtr( GR_BUFFER_FRONTBUFFER );	// read FRONT buffer
    data8 = fbuffer;
    data32 = (FxU32 *)data8;

  if (pbmiDib->bmiHeader.biBitCount == 16) {
    // NOTE: this blows out the 2nd level cache
    while (h-- > 0) {
	for (x = 0; x < w/2; x+=2) {
#if 0
	    FxU32 a,b;
	    a = lfb[x];
	    b = lfb[x+1];
	    data32[x] = a;
	    data32[x+1] = b;
#else
	    data32[x] = lfb[x];
	    data32[x+1] = lfb[x+1];
#endif
	}
	lfb += (1024)>>1;		// advance to next scanline
	data32 += (pbmiDib->bmiHeader.biWidth) >> 1;
    }
  }
  else {
    while (h-- > 0) {
        FxU8 *xdata = data8;

	for (x = 0; x < w/2; x++) {
	    FxU32 rgb2 = lfb[x];		// holds 2 pixels

	    // NOTE: this is really slow
	    rgb2 & 0xFFFF;
	    *xdata++ = (FxU8)(rgb2<<3);
	    *xdata++ = (FxU8)(rgb2>>3);
	    *xdata++ = (FxU8)(rgb2>>8);
	    rgb2 >>= 16;
	    *xdata++ = (FxU8)(rgb2<<3);
	    *xdata++ = (FxU8)(rgb2>>3);
	    *xdata++ = (FxU8)(rgb2>>8);
	}
	lfb += 1024>>1;				// advance to next scanline
//	data8 += (pbmiDib->bmiHeader.biWidth * 3 + 3) & ~3;
	data8 += 640 *3;
    }
  }
}

// copy the window from the SST framebuffer to the GUI window
void FX_EXPORT FX_CSTYLE
uiSwapBuffers( HWND hWnd , HWND hwndStatusBar )
{
    int w,h;
    RECT rect;

    w = grSstScreenWidth();
    h = grSstScreenHeight();
    GDBG_INFO(58,"uiSwapBuffers(0x%x): %d x %d\n",hWnd,w,h);

    // take minimum of window and SST framebuffer
    GetClientRect(hWnd,&rect);
    if (hwndStatusBar)
    if (GetWindowLong(hwndStatusBar,GWL_STYLE) & WS_VISIBLE) {
	RECT statRect;
	GetClientRect(hwndStatusBar,&statRect);	// if status bar is visible
	rect.bottom -= statRect.bottom;		// decrease window size
    }
    if (rect.right < w) w = rect.right;
    if (rect.bottom < h) h = rect.bottom;
    wingSetup(hWnd,w,h);			// setup WING

    WinGBitBlt(GetDC(hWnd),0,0,w,h,wgDC,0,0);
    GdiFlush();
}

#include <fxos.h>

//---------------------------------------------------------------------------
// A SIMPLE TIMING PACKAGE
//---------------------------------------------------------------------------
static int timing, tickCounter;
static int TICKS = 60;

//---------------------------------------------------------------------------
// set the number of ticks in the timing loop (if ticks > 0)
// returns the number of ticls
//---------------------------------------------------------------------------
int FX_EXPORT FX_CSTYLE
uiTimingTicks( int ticks )
{
    if (ticks > 0) TICKS = ticks;
    return TICKS;
}

//---------------------------------------------------------------------------
// enable/disable timing reports
//---------------------------------------------------------------------------
void FX_EXPORT FX_CSTYLE
uiTimingEnable( int flag )
{
    timing = flag;
    tickCounter = TICKS;
    timer(0);
}

int FX_EXPORT FX_CSTYLE
uiTimingIsEnabled( void )
{
    return timing;
}

//---------------------------------------------------------------------------
// update the tick counter and display timing info if enabled
//---------------------------------------------------------------------------
void FX_EXPORT FX_CSTYLE
uiTimingLoop( void )
{
    tickCounter--;			// decrement the tick counter
    if (tickCounter == 0) {		// once every TICKS frames do ...
	tickCounter = TICKS;		// reset the tick counter
	if (timing) {			// if enabled then display the stats
	    float t = timer(1);		// get the elapsed time
	    printf("%d frames in %.2f seconds, %.1f frames per second\n",
			TICKS, t, TICKS/t);
	    timer(0);			// restart the timer
	}
    }
}
