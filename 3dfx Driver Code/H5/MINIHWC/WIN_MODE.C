/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
*/
#include <stdio.h>
#include <3dfx.h>
#include <gdebug.h>
#define WIN32_LEAN_AND_MEAN 
#include <windows.h> 
#include <ddraw.h> 
#include <sst1vid.h>
#include "qmodes.h"
#define IS_32
#define Not_VxD
#include <minivdd.h>
#include <vmm.h>
#include <configmg.h>

#define  KERNEL 1 /* this preventes a strange compile time error in fxpci.h */
#include <minihwc.h>

#ifdef GETENV
#undef GETENV
#endif

#define GETENV hwcGetenv


/* XXXTACOHACK -- The required header file isn't shipping yet - decls will be removed at some 
   future time */

#ifndef HMONITOR_DECLARED // AJB- Make def compatible w/ vc6 headers
typedef void *HMONITOR;
#define HMONITOR_DECLARED
#endif
typedef BOOL (FAR PASCAL * LPDDENUMCALLBACKEXA)(GUID FAR *, LPSTR, LPSTR, LPVOID, HMONITOR); 
extern HRESULT WINAPI DirectDrawEnumerateExA( LPDDENUMCALLBACKEXA lpCallback, 
                                              LPVOID lpContext, DWORD dwFlags); 
typedef HRESULT (WINAPI * LPDIRECTDRAWENUMERATEEXA)( LPDDENUMCALLBACKEXA lpCallback, 
                                                     LPVOID lpContext, 
                                                     DWORD dwFlags); 
#define DDENUM_ATTACHEDSECONDARYDEVICES     0x00000001L 
/* XXXTACOHACK -- The required header file isn't shipping yet - decls will be removed at some 
   future time */


static GUID          fooGuid;
static LPDIRECTDRAW  lpDD1;
static LPDIRECTDRAW2 lpDD;
static HWND          hwndApp;
static DEVMODE       theDM ;
static WNDPROC       oldWindowProc = NULL ;
static HWND          patchedHwnd = NULL ;
static int           OpenGLEnabled = 0;

/* Windows */
#define SEPARATOR '\\'
/* UNIX */
#define SEPARATOR2 '/'

/*
 * parseFilename
 *
 *      Return the file name portion of a filename/path.
 */
static char *
_parseFilename(char *name)
{
    int i;

    if (name == NULL) 
      return NULL;
    for(i = strlen(name); i >= 0; i--)
        if ((name[i] == SEPARATOR) ||
            (name[i] == SEPARATOR2))
            return (name + i + 1);
    return name;
}  /* End of parseFilename*/


#define MAX_SPECIAL_LIST 8
static int _special_list[MAX_SPECIAL_LIST][50] = 
{{13, 7, 11, -8, -9, 3, 4, 12},  /* nhl98dem */
 {45, 39, 43, -8, -9, 35, 36, 44}, /* lower */
  {13, 7, 11, -8, -9}, /*NHL98 */
 {45, 39, 43, -8, -9}, /* lower */
 {30, 6, 11, 8, 3, 4}, /* _GLIDE excalibur*/
 {30, 38, 43, 40, 35, 36}, /*lower */
 {5, 11, 24}, /* fly */ 
 {37, 43, 56}, /*lower */
};

static int
_eadecrypt(char *name)
{
  int i, j;

  for (j = 0; j < MAX_SPECIAL_LIST; j++){
    int success = 1;
    for (i = 0; i < (int)strlen(name)-4; i++){
      if (name[i] != _special_list[j][i] + 65){
        success = 0;
        break;
      }
    }
    if (success) {
      return 1;
    }
  }
  return 0;
}

static int _set_exclusive_relaxed;
static int _set_vidmode_relaxed;

void
checkSpecialList()
{
#if defined( __WATCOMC__ )
  /* Nothing for DOS */
#else 
/* windows only */
  HMODULE module;
  module = GetModuleHandle(NULL);
  _set_exclusive_relaxed = 0;
  if (module) {
    char pathname[256];
    char *fname;
    GDBG_INFO(80,"Got module handle\n");
    strcpy(pathname, "deadbeef");
    GetModuleFileName(module, pathname, 256);
    fname = _parseFilename(pathname);
    if (fname != NULL){
      GDBG_INFO(80,"module name %s\n", fname);
      if (_eadecrypt(fname)){
        GDBG_INFO(80,"Special found\n");
        _set_exclusive_relaxed = 1;
      }
    }
  }
#endif
}

static BOOL FAR PASCAL
ddEnumCbEx( GUID FAR *guid, LPSTR desc, LPSTR name, LPVOID ctx, HMONITOR hmon ) 
{
  DWORD    *data  = (DWORD*)ctx;
  HMONITOR target = (HMONITOR)data[0];
  BOOL     rv     = DDENUMRET_OK;
  
  if ( target == hmon ) {
    if ( guid ) {
      fooGuid = *guid;
      data[1] = (DWORD)&fooGuid;
    } else {
      /* guid for primary display device */
      data[1] = 0;
    }
    rv     = DDENUMRET_CANCEL;
  }
  return rv;
}


/*
 * To answer your question:
 *
 * This function is patched into the windows message handler of the
 * calling app when the app requests a fullscreen mode that is the
 * same as the desktop mode.  When this handler gets called with
 * WM_ACTIVATEAPP (FALSE) then our window is being alt-tab'd away
 * from and we need to bail out & make things safe for the display
 * driver.  The easiest way to do this is to call our own resetVideo
 * function.  The resetVideo function will give up its DDraw exclusive
 * lock, close DDraw, ChangeDisplaySettings back to the original desktop
 * mode and, most importantly, unhook this message handler from the call
 * chain.
 */
void resetVideo(void) ;

LRESULT CALLBACK MyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_ACTIVATEAPP:
      if (wParam == 0)
        resetVideo() ;
      break ;
    default:
      break ;
  } ;

  return CallWindowProc(oldWindowProc, hwnd, uMsg, wParam, lParam) ;
}

FxBool 
setVideoMode(HWND hwnd, int xRes, int yRes, int pixelSize, int refresh, void *hmon) 
{
  LPGUID  ddGuid = NULL ;
  HMODULE ddraw = NULL ; 
  HRESULT hResult ;
  DEVMODE devMode ;
  FxU32   bpp = pixelSize << 3 ;

  GDBG_INFO(80, "setVideoMode sees hwnd %x\n", hwnd) ;
  hwndApp = (hwnd == NULL) ? GetActiveWindow() : hwnd ;
 
  if (hwndApp == NULL)
    GDBG_INFO( 80, "Couldn't get a valid window handle\n" ) ;
  
  ddGuid = NULL ;
  ddraw = GetModuleHandle("ddraw.dll") ;

  if (ddraw)
  {
    LPDIRECTDRAWENUMERATEEXA ddEnumEx ;
    
    ddEnumEx = (void*)GetProcAddress(ddraw, "DirectDrawEnumerateExA") ;

    if (ddEnumEx)
    {
      DWORD   data[2] ;
      
      data[0] = (DWORD)hmon ;
      data[1] = 0 ;
      
      ddEnumEx(ddEnumCbEx, data, DDENUM_ATTACHEDSECONDARYDEVICES) ;

      if (data[1])
        ddGuid = (LPGUID)data[1] ;
    }
  }
  

  /*
  **  Oh, this is lovely.  What we have here is a failure to
  **  communicate.  If the current mode is exactly the same as the
  **  mode in which Glide wishes to run, then the DirecDraw driver
  **  doesn't get called when we restore the video (or on an alt-tab)
  **  this would be fine, except Glide has its way with the video
  **  overlay registers.  So, since Glide *never* wants 8 bits per
  **  pixel, we just force it here, and our DirectDraw driver always
  **  be called when Glide gives up video exclusivity. -CHD
  */
  /* Lovely my foot. What about when we run from an 8bpp desktop?
   * Let's only be stupid when we absolutely have to be. -AJB
   */
  /* The latest stupidity: When the desktop is in the same video mode
   * as the app has requested we can infer that an alt-tab is happening
   * by patching the message handler for the app and looking for the
   * WM_ACTIVATEAPP message with a FALSE parameter (which is Windows-ese
   * for "you are being alt-tab'd away from).  This sounds scary but
   * it is actually exactly what goes on under the hood of direct draw.
   */
  {
    HDC hdc = GetDC(NULL) ;

    /* make sure we only patch the wndproc once */
    /* this is a problem with the minigl in half-life */
    if (hwndApp != patchedHwnd) 
    {
      if ((xRes == GetSystemMetrics(SM_CXSCREEN)) &&
          (yRes == GetSystemMetrics(SM_CYSCREEN)))
      {
        if (bpp == (DWORD)GetDeviceCaps(hdc, BITSPIXEL))
        {
		  if ( !OpenGLEnabled )
		  {
            oldWindowProc = (WNDPROC)GetWindowLong(hwndApp, GWL_WNDPROC) ;
            SetWindowLong(hwndApp, GWL_WNDPROC, (DWORD)&MyWindowProc) ;
            patchedHwnd = hwndApp ;
		  }
        }
      }
    }

    EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &theDM) ;

    /* EnumDisplaySettings doesn't enum current or registry
     * settings in win95. If we get wacky data back, make
     * a struct manually.
     */
    if ((theDM.dmBitsPerPel < 8) ||
        (theDM.dmBitsPerPel > 32)) 
    {
      /*
       * AJB- copy the current device mode so we can restore it upon
       * exit.  It seems that in win95 after an OpenGL app (Quake3)
       * has set the mode w/ ChangeDisplaySettings, the DirectX call
       * to restore the video mode doesn't.  Doing it manually seems
       * to work, tho.
       */
      theDM.dmSize = sizeof(DEVMODE) ;
      theDM.dmPelsWidth = GetSystemMetrics(SM_CXSCREEN) ;
      theDM.dmPelsHeight = GetSystemMetrics(SM_CYSCREEN) ;
      theDM.dmBitsPerPel = (DWORD)GetDeviceCaps(hdc, BITSPIXEL) ;
      theDM.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT ;
    }
    ReleaseDC(NULL, hdc) ;
  }

  checkSpecialList() ;

  if (lpDD == NULL) 
  {
    /* only create directdraw object once */
    if (DirectDrawCreate(ddGuid, &lpDD1, NULL) != DD_OK)
    {
      GDBG_INFO(80, "DDraw Obj Create Failed!\n") ;
    }
     else
      GDBG_INFO(80, "DDraw Obj created!\n") ;

    if (IDirectDraw_QueryInterface(lpDD1, &IID_IDirectDraw2, (LPVOID*)&lpDD) != DD_OK)
    {
      IDirectDraw_Release(lpDD1) ;
      lpDD1 = NULL ;
      lpDD  = NULL ;
      GDBG_INFO(80, "DDraw Obj Create Failed!\n") ;

      return FXFALSE ;
    }
     else
       GDBG_INFO(80, "DDraw2 Obj created!\n") ;
  }
  
  /* Set Exclusive Mode, change resolution,  */
  GDBG_INFO(80, "Setting Full screen exclusive mode!\n");
  GDBG_INFO(80, "Calling IDD2_SetCoop: 0x%x, 0x%x, 0x%x\n", lpDD, hwndApp, 
            DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
 
  hResult = IDirectDraw2_SetCooperativeLevel(lpDD, hwndApp, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN) ;

  if (hResult != DD_OK)
  {
    GDBG_INFO(80, "Couldn't set cooperative level:  " );

    if (hResult & DDERR_EXCLUSIVEMODEALREADYSET)
      GDBG_INFO_MORE(80, "DDERR_EXCLUSIVEMODEALREADYSET\n" ); 
    
    if (hResult & DDERR_HWNDALREADYSET) 
    {
      GDBG_INFO_MORE(80, "DDERR_HWNDALREADYSET\n" );
      
      if (hResult == DDERR_HWNDALREADYSET)
        _set_exclusive_relaxed = 1;
    }

    if (hResult & DDERR_HWNDSUBCLASSED)
      GDBG_INFO_MORE(80, "DDERR_HWNDSUBCLASSED\n" );
    
    if (hResult & DDERR_INVALIDOBJECT)
      GDBG_INFO_MORE(80, "DDERR_INVALIDOBJECT\n" );
    
    if (hResult & DDERR_INVALIDPARAMS)
      GDBG_INFO_MORE(80, "DDERR_INVALIDPARAMS\n" );
    
    if (hResult & DDERR_OUTOFMEMORY)
      GDBG_INFO_MORE(80, "DDERR_OUTOFMEMORY\n" );
    
    if (!_set_exclusive_relaxed)
      return FXFALSE;
  }
  
  GDBG_INFO(80, "FSEM Set\n" );
  GDBG_INFO(80, "Enumerating Display Modes.\n");
  
  /* Figure out if we can support the requested display mode.  If not,
     try to use the same x & y res, but the default refresh rate.*/
  
  devMode.dmSize = sizeof(DEVMODE) ;
  devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY ;
  devMode.dmPelsWidth = xRes ;
  devMode.dmPelsHeight = yRes ;
  devMode.dmBitsPerPel = bpp ;
  devMode.dmDisplayFrequency = refresh ;

  GDBG_INFO(80, "Setting Display Mode\n");
  GDBG_INFO(80, "Attempting mode %dx%d at %dHz\n", xRes, yRes, refresh) ;

  /*
   * if FX_GLIDE_APP_REFRESH_RATE is set, allow the app the chance
   * to override the default refresh rate as set by the display
   * properties tab. Note that this conditional relies heavily
   * upon left->right evaluation.
   */
  if ((!GETENV("FX_GLIDE_USE_APP_REFRESH") ||
     (atoi(GETENV("FX_GLIDE_USE_APP_REFRESH"))) == 0) || 
     (ChangeDisplaySettings(&devMode, 0) != DISP_CHANGE_SUCCESSFUL))
  {
    GDBG_INFO(80, "Couldn't set display mode\n" );
    GDBG_INFO(80, "Retrying at default refresh\n");

    devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT ;

    if (ChangeDisplaySettings(&devMode, 0) != DISP_CHANGE_SUCCESSFUL)
    {
      GDBG_INFO(80, "Setting video mode %dx%d@default refresh failed!\n",
                xRes, yRes) ;

      if (!_set_vidmode_relaxed)
      {
        GDBG_INFO(80, "Returning FXFALSE\n") ;
        return FXFALSE ;
      } 
       else
      {
        GDBG_INFO(80, "Continuing operation due to relaxation condition\n") ;
      }
    }
  }

  return FXTRUE;
  
} /* setVideoMode */


void resetVideo(void) 
{
#define FN_NAME "resetVideo"
  if (lpDD)
  {
    IDirectDraw2_SetCooperativeLevel(lpDD, hwndApp, DDSCL_NORMAL) ;
    GDBG_INFO(80, "%s:  Set cooperative level!\n", FN_NAME) ;
    IDirectDraw2_Release(lpDD) ;
    GDBG_INFO(80, "%s:  Released lpDD!\n", FN_NAME) ;

    if (lpDD1)
    {
      IDirectDraw_Release(lpDD1) ;
      GDBG_INFO(80, "%s:  Released lpDD1!\n", FN_NAME) ;
    }
  }
  lpDD = NULL ;
  lpDD1 = NULL ;

  /*
   * once upon a time we would un-install our wndproc here
   * but that was conflicting with the minigl's way of thinking
   * and it would be a damn shame if the minigl didn't work right.
   */
	
  /*
   * Restore the video mode that was set before we did our
   * setVideoMode.  The CDS_RESET flag forces a mode change
   * to take place even if the mode is changed to the same
   * one we are already in.
   */
  ChangeDisplaySettings(&theDM, CDS_RESET) ;
  
  return;
#undef FN_NAME
} /* resetVideo */

typedef struct WidthHeight_s 
{
  FxU32 width; 
  FxU32 height;
} WidthHeight_t;

static  WidthHeight_t widthHeightByResolution[] = {
  {320, 200},                 /* GR_RESOLUTION_320x200 */
  {320, 240},                 /* GR_RESOLUTION_320x240 */
  {400, 256},                 /* GR_RESOLUTION_400x256 */
  {512, 384},                 /* GR_RESOLUTION_512x384 */
  {640, 200},                 /* GR_RESOLUTION_640x200 */
  {640, 350},                 /* GR_RESOLUTION_640x350 */
  {640, 400},                 /* GR_RESOLUTION_640x400 */
  {640, 480},                 /* GR_RESOLUTION_640x480 */
  {800, 600},                 /* GR_RESOLUTION_800x600 */
  {960, 720},                 /* GR_RESOLUTION_960x720 */
  {856, 480},                 /* GR_RESOLUTION_856x480 */
  {512, 256},                 /* GR_RESOLUTION_512x256 */
  {1024, 768},                /* GR_RESOLUTION_1024x768 */
  {1280, 1024},               /* GR_RESOLUTION_1280x1024 */
  {1600, 1200},               /* GR_RESOLUTION_1600x1200 */
  {400, 300},                 /* GR_RESOLUTION_400x300 */
  {1152, 864},                /* GR_RESOLUTION_1152x864 */
  {1280, 960},                /* GR_RESOLUTION_1280x960 */
  {1600, 1024},               /* GR_RESOLUTION_1600x1024 */
  {1792, 1344},               /* GR_RESOLUTION_1792x1344 */
  {1856, 1392},               /* GR_RESOLUTION_1856x1392 */
  {1920, 1440},               /* GR_RESOLUTION_1920x1440 */
  {2048, 1536},               /* GR_RESOLUTION_2048x1536 */
  {2048, 2048}                /* GR_RESOLUTION_2048x2048 */
};

static FxU32 refresh[] = {
    60,	 /* GR_REFRESH_60Hz	 */
    70,	 /* GR_REFRESH_70Hz	 */
    72,  /* GR_REFRESH_72Hz	 */
    75,	 /* GR_REFRESH_75Hz	 */
    80,  /* GR_REFRESH_80Hz	 */
    90,  /* GR_REFRESH_90Hz	 */
    100, /* GR_REFRESH_100Hz */
    85,  /* GR_REFRESH_85Hz	 */ 
    120  /* GR_REFRESH_120Hz */
};


/*
**  checkResolution - check to see if a given resolution is available
**
**  since we no longer use ddraw to change modes, we no longer enum
**  modes with ddraw either.
*/
FxBool
checkResolutions(FxBool *supportedByResolution, FxU32 stride, void *hmon) 
{
#define FN_NAME "checkResolution"  
  DEVMODE testDM ;
  DWORD   theRes, theFreq ;

  testDM.dmSize = sizeof(DEVMODE) ;
  testDM.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY ;

  for (theRes = 0 ;
       theRes < (sizeof(widthHeightByResolution) / sizeof(WidthHeight_t)) ;
       theRes++) {
	  for (theFreq = 0 ;
	       theFreq < (sizeof(refresh) / sizeof(FxU32)) ;
		     theFreq++) {

		  testDM.dmPelsWidth         = widthHeightByResolution[theRes].width ;
		  testDM.dmPelsHeight        = widthHeightByResolution[theRes].height ;
      testDM.dmDisplayFrequency  = refresh[theFreq] ;

		  /* This is slightly flaky:
		   * Glide wasn't really designed to support anything other than 16bpp
		   * so we have no way of differentiating between 16 and 32 bpp in our
		   * supported mode list.  Hopefully that will never be an issue.
		   */
		  testDM.dmBitsPerPel        = 0x10 ;
      supportedByResolution[theRes * stride + theFreq] = (ChangeDisplaySettings(&testDM, CDS_TEST) != DISP_CHANGE_BADMODE) ;
	  }
  }

  return FXTRUE;
#undef FN_NAME
} /* checkResolutions */

void EnableOpenGL ( void )
{
  OpenGLEnabled = 1;
}
  