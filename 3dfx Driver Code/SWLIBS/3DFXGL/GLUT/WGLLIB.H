/*
	FILE:	wgllib.h

	Defines functions and data structures for creating graphics
	with wgl.

	Paul Mayfield, 7/3/96
*/

#ifndef __wgllib_h
#define __wgllib_h

#include <windows.h>
#include <stdio.h>


// CALLBACK DEFINITIONS
typedef void (*WLBinitCB)		(void);
typedef void (*WLBdisplayCB)	(void);
typedef void (*WLBreshapeCB)	(int, int);
typedef void (*WLBkeyboardCB)	(unsigned char, int, int);
typedef void (*WLBmouseCB)		(int, int, int, int);
typedef void (*WLBpfdsetupCB)	(HDC);

class wlbInfo {
  public:
	wlbInfo();
   ~wlbInfo();

	WLBinitCB		init;
	WLBdisplayCB	display;
	WLBreshapeCB	reshape;
	WLBkeyboardCB	keyboard;
	WLBmouseCB		mouse;
	WLBpfdsetupCB	pfdsetup;
};	


// GLOBALS AND THE SUCH
#define WM_INIT (WM_USER + 0x0001)
LONG WINAPI	wlbWindowProc (HWND, UINT, WPARAM, LPARAM);
HPALETTE WINAPI	wlbCreateRGBPalette(HDC hDC);
void WINAPI	wlbPfdSetup (HDC hDC);

extern "C" {
	extern RECT oldrect;
};	

// ==================================================================
// ==================================================================
// ==================================================================
//		LIBRARY ENTRY POINTS
// ==================================================================
// ==================================================================
// ==================================================================
// OUTPUTS INFORMATION CONCERNING THE PFD
void	wlbOutputPFD				(FILE * file, HDC hdc, int pfdIndex);
void	wlbInitPFD					(PIXELFORMATDESCRIPTOR * pfd);

// FUNCTIONS TO QUERY STATE OF A PFD
WORD	wlbGetPFDVersion			(HDC hdc, int pfdIndex);
BOOL	wlbCanDrawToWindow			(HDC hdc, int pfdIndex);
BOOL	wlbCanDrawToBitmap			(HDC hdc, int pfdIndex);
BOOL	wlbSupportsGDI				(HDC hdc, int pfdIndex);
BOOL	wlbSupportsOpenGL			(HDC hdc, int pfdIndex);
BOOL	wlbIsDriverPFD				(HDC hdc, int pfdIndex);
BOOL	wlbNeedsPalette				(HDC hdc, int pfdIndex);
BOOL	wlbNeedsSystemPalette		(HDC hdc, int pfdIndex);
BOOL	wlbIsDoubleBuffered			(HDC hdc, int pfdIndex);
BOOL	wlbIsStereo					(HDC hdc, int pfdIndex);
BOOL	wlbCanSwapIndividualLayers	(HDC hdc, int pfdIndex);
BOOL	wlbSwapCopyExtensionSet		(HDC hdc, int pfdIndex);
BOOL	wlbSwapExchangeSet			(HDC hdc, int pfdIndex);
BOOL	wlbIsRGBA					(HDC hdc, int pfdIndex);
BOOL	wlbIsColorIndex				(HDC hdc, int pfdIndex);
BYTE	wlbGetColorDepth			(HDC hdc, int pfdIndex);
BYTE	wlbGetRedBits				(HDC hdc, int pfdIndex);
BYTE	wlbGetGreenBits				(HDC hdc, int pfdIndex);
BYTE	wlbGetBlueBits				(HDC hdc, int pfdIndex);
BYTE	wlbGetAlphaBits				(HDC hdc, int pfdIndex);
BYTE	wlbGetAccumBits				(HDC hdc, int pfdIndex);
BYTE	wlbGetAccumRedBits			(HDC hdc, int pfdIndex);
BYTE	wlbGetAccumGreenBits		(HDC hdc, int pfdIndex);
BYTE	wlbGetAccumBlueBits			(HDC hdc, int pfdIndex);
BYTE	wlbGetAccumAlphaBits		(HDC hdc, int pfdIndex);
BYTE	wlbGetDepthBits				(HDC hdc, int pfdIndex);
BYTE	wlbGetStencilBits			(HDC hdc, int pfdIndex);
BYTE	wlbGetAuxBufferCount		(HDC hdc, int pfdIndex);
BYTE	wlbGetLayerPlaneCount		(HDC hdc, int pfdIndex);
DWORD	wlbGetUnderlayTransparentClr(HDC hdc, int pfdIndex);

// VERSIONS OF FUNCTIONS TO QUERY STATE OF PFD THAT TAKE PFD'S AS PARAMS
WORD	wlbGetPFDVersion			(PIXELFORMATDESCRIPTOR * pfd);
BOOL	wlbCanDrawToWindow			(PIXELFORMATDESCRIPTOR * pfd);
BOOL	wlbCanDrawToBitmap			(PIXELFORMATDESCRIPTOR * pfd);
BOOL	wlbSupportsOpenGL			(PIXELFORMATDESCRIPTOR * pfd);
BOOL	wlbSupportsGDI				(PIXELFORMATDESCRIPTOR * pfd);
BOOL	wlbIsDriverPFD				(PIXELFORMATDESCRIPTOR * pfd);
BOOL	wlbNeedsPalette				(PIXELFORMATDESCRIPTOR * pfd);
BOOL	wlbNeedsSystemPalette		(PIXELFORMATDESCRIPTOR * pfd);
BOOL	wlbIsDoubleBuffered			(PIXELFORMATDESCRIPTOR * pfd);
BOOL	wlbIsStereo					(PIXELFORMATDESCRIPTOR * pfd);
BOOL	wlbCanSwapIndividualLayers	(PIXELFORMATDESCRIPTOR * pfd);
BOOL	wlbSwapCopyExtensionSet		(PIXELFORMATDESCRIPTOR * pfd);
BOOL	wlbSwapExchangeSet			(PIXELFORMATDESCRIPTOR * pfd);
BOOL	wlbIsRGBA					(PIXELFORMATDESCRIPTOR * pfd);
BOOL	wlbIsColorIndex				(PIXELFORMATDESCRIPTOR * pfd);
BYTE	wlbGetColorDepth			(PIXELFORMATDESCRIPTOR * pfd);
BYTE	wlbGetRedBits				(PIXELFORMATDESCRIPTOR * pfd);
BYTE	wlbGetGreenBits				(PIXELFORMATDESCRIPTOR * pfd);
BYTE	wlbGetBlueBits				(PIXELFORMATDESCRIPTOR * pfd);
BYTE	wlbGetAlphaBits				(PIXELFORMATDESCRIPTOR * pfd);
BYTE	wlbGetAccumBits				(PIXELFORMATDESCRIPTOR * pfd);
BYTE	wlbGetAccumRedBits			(PIXELFORMATDESCRIPTOR * pfd);
BYTE	wlbGetAccumGreenBits		(PIXELFORMATDESCRIPTOR * pfd);
BYTE	wlbGetAccumBlueBits			(PIXELFORMATDESCRIPTOR * pfd);
BYTE	wlbGetAccumAlphaBits		(PIXELFORMATDESCRIPTOR * pfd);
BYTE	wlbGetDepthBits				(PIXELFORMATDESCRIPTOR * pfd);
BYTE	wlbGetStencilBits			(PIXELFORMATDESCRIPTOR * pfd);
BYTE	wlbGetAuxBufferCount		(PIXELFORMATDESCRIPTOR * pfd);
BYTE	wlbGetLayerPlaneCount		(PIXELFORMATDESCRIPTOR * pfd);
DWORD	wlbGetUnderlayTransparentClr(PIXELFORMATDESCRIPTOR * pfd);

// FUNCTIONS TO SET THE STATE OF A PFD
BOOL	wlbSetPFDVersion			(PIXELFORMATDESCRIPTOR * pfd, WORD version);
BOOL	wlbSetDrawToWindow			(PIXELFORMATDESCRIPTOR * pfd, BOOL state=1);
BOOL	wlbSetDrawToBitmap			(PIXELFORMATDESCRIPTOR * pfd, BOOL state=1);
BOOL	wlbSetSupportGDI			(PIXELFORMATDESCRIPTOR * pfd, BOOL state=1);
BOOL	wlbSetSupportOpenGL			(PIXELFORMATDESCRIPTOR * pfd, BOOL state=1);
BOOL	wlbSetDriverPFD				(PIXELFORMATDESCRIPTOR * pfd, BOOL state=1);
BOOL	wlbSetNeedsPalette			(PIXELFORMATDESCRIPTOR * pfd, BOOL state=1);
BOOL	wlbSetNeedsSystemPalette	(PIXELFORMATDESCRIPTOR * pfd, BOOL state=1);
BOOL	wlbSetDoubleBuffer			(PIXELFORMATDESCRIPTOR * pfd, BOOL state=1);
BOOL	wlbSetSingleBuffer			(PIXELFORMATDESCRIPTOR * pfd, BOOL state=1);
BOOL	wlbSetStereo				(PIXELFORMATDESCRIPTOR * pfd, BOOL state=1);
BOOL	wlbSetDBDontCare			(PIXELFORMATDESCRIPTOR * pfd, BOOL state=1);
BOOL	wlbSetStereoDontCare		(PIXELFORMATDESCRIPTOR * pfd, BOOL state=1);
BOOL	wlbSetCanSwapIndLayers		(PIXELFORMATDESCRIPTOR * pfd, BOOL state=1);
BOOL	wlbSetSwapCopyExtension		(PIXELFORMATDESCRIPTOR * pfd, BOOL state=1);
BOOL	wlbSetSwapExchange			(PIXELFORMATDESCRIPTOR * pfd, BOOL state=1);
BOOL	wlbSetRGBA					(PIXELFORMATDESCRIPTOR * pfd, BOOL state=1);
BOOL	wlbSetColorIndex			(PIXELFORMATDESCRIPTOR * pfd, BOOL state=1);
BOOL	wlbSetColorDepth			(PIXELFORMATDESCRIPTOR * pfd, BYTE value);
BOOL	wlbSetRedBits				(PIXELFORMATDESCRIPTOR * pfd, BYTE value);
BOOL	wlbSetGreenBits				(PIXELFORMATDESCRIPTOR * pfd, BYTE value);
BOOL	wlbSetBlueBits				(PIXELFORMATDESCRIPTOR * pfd, BYTE value);
BOOL	wlbSetAlphaBits				(PIXELFORMATDESCRIPTOR * pfd, BYTE value);
BOOL	wlbSetAccumBits				(PIXELFORMATDESCRIPTOR * pfd, BYTE value);
BOOL	wlbSetAccumRedBits			(PIXELFORMATDESCRIPTOR * pfd, BYTE value);
BOOL	wlbSetAccumGreenBits		(PIXELFORMATDESCRIPTOR * pfd, BYTE value);
BOOL	wlbSetAccumBlueBits			(PIXELFORMATDESCRIPTOR * pfd, BYTE value);
BOOL	wlbSetAccumAlphaBits		(PIXELFORMATDESCRIPTOR * pfd, BYTE value);
BOOL	wlbSetDepth					(PIXELFORMATDESCRIPTOR * pfd, BYTE value);
BOOL	wlbSetStencilBits			(PIXELFORMATDESCRIPTOR * pfd, BYTE value);
BOOL	wlbSetAuxBufferCount		(PIXELFORMATDESCRIPTOR * pfd, BYTE value);
BOOL	wlbSetLayerPlaneCount		(PIXELFORMATDESCRIPTOR * pfd, BYTE value);
BOOL	wlbSetUnderlayTransparentClr(PIXELFORMATDESCRIPTOR * pfd, DWORD value);

#endif