/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** $Revision: 6$
** $Date: 10/11/00 7:40:12 PM$
*/

#ifdef HWC_GUI
#define WIN32_LEAN_AND_MEAN
#include <stdlib.h>
#include <windows.h>
#include <commctrl.h>
#include <ctl3d.h>
#include <afxres.h>
#include <stdio.h>
#include <fxhwc.h>
#include <fximg.h>
#include "gui.h"
#include "guimv.h"

extern char szMainClassName[];
extern char szViewClassName[];
char *szMainName = "Main Control";
static int qhead = 0;
static int qtail = 0;
static int queue[256] = {0};

typedef struct {
  BITMAPINFOHEADER bmiHeader;
  RGBQUAD bmiColors[256];
} BITMAPINFO256;

BITMAPINFO256 bmi;
void *pvbits = NULL;
HBITMAP hBM = NULL;
HDC bmDC;
ATOM _hwcInstAtom;
static HwcWindow *lastHwcWindow = NULL;
static void *lastWin = NULL;

char *
hwcWindowName(const HwcSimulator *hws, const char *name);

HwcWindow *
hwcNewWindow(HwcSimulator *hws, CHAR *className, const char *name, int w, int h);

FxBool 
hwcReadWindowPlacement( const char *name, HWND hWnd );

FxBool
hwcWriteWindowPlacement( const char *name, HWND hWnd);

void
hwcBindWindow(void *hWnd, HwcWindow *pWnd) {
    SetProp( (HWND)hWnd, (LPCTSTR)_hwcInstAtom, (HANDLE)pWnd);
}

static void
hwcCheckBuffer(struct _HwcBuffer *pBuff)
{ 
  if ( pBuff->hPen == NULL ) {
    pBuff->hPen = CreatePen( PS_SOLID, 1, 0xffffffff );
    pBuff->hDC = CreateCompatibleDC( NULL );
    pBuff->hBitmap = CreateCompatibleBitmap( (HDC) pBuff->hDC, pBuff->width, pBuff->height );
    SelectObject( (HDC) pBuff->hDC, pBuff->hBitmap );
    SelectObject( (HDC) pBuff->hDC, pBuff->hPen );
    hwcFillRectangle(pBuff, 0, 0, pBuff->width, pBuff->height);
  }
}

HwcWindow *
hwcWndToWindow(void *hWnd) {
    HwcWindow *pHwc = NULL;

    if ( hWnd == lastWin ) {
        return lastHwcWindow;
    }

    pHwc = (HwcWindow *) GetProp((HWND) hWnd, (LPCTSTR)_hwcInstAtom );

    if ( pHwc == NULL ) {
        GDBG_ERROR("hwcWndToWindow", "window not bound\n");
    }

    lastWin = hWnd;
    lastHwcWindow = pHwc;

    return pHwc;
}

/*---------------------------------------------------------------------------
   draw a pixel in each of the open windows attached to a device
  ---------------------------------------------------------------------------*/
void
_hwcGUIDrawPixel(HwcSimulator *hws, int x, int y, FxU32 addr, HwcPixel pix, HwcBufferType which)
{
  HwcWindow *cw;
  FxU8 *dest;
  FxU8 r, g, b;

  /* Since thing can happen asynchronously here due to PSIM execution we need 
     to ensure we are not being called after we have deleted the DIB check 
     we have a valid DIB */

  if ( hBM == NULL )
    return;

  /* check pixel is within bitmap */

  if (( x < 0 ) || ( y < 0 ) || 
      (x >= bmi.bmiHeader.biWidth) || (y >= -bmi.bmiHeader.biHeight)) {
      return;
  }

  /* convert and copy the framebuffer data to the bitmap */

  dest = (FxU8*) pvbits;
  dest += 3 * (y * bmi.bmiHeader.biWidth + x);
  
  /* amazingly BitBlt is as fast as FillREct! */

  for (cw = hws->windows; cw != NULL; cw = cw->next) {
    /* if addr falls within this window */
    if (addr >= cw->buffer->physicalBase && addr < (int)cw->buffer->physicalEnd) {

      if (!( cw->buffer->hints & HWC_BUFFER_SHOW_UPDATES ))
          continue;
		
      // hwcPixelToRGB(cw->buffer, pix, &r, &g, &b);

      r = (FxU8)((pix >> 16)&0xff);
      g = (FxU8)((pix >> 8) &0xff);
      b = (FxU8)(pix & 0xff);
      dest[0] = (FxU8)b;
      dest[1] = (FxU8)g;
      dest[2] = (FxU8)r;
      BitBlt((HDC) cw->hDC,x,y,1,1,bmDC,x,y,SRCCOPY);
      /* this is WAY too slow.... */
      /* we need this because otherwise BitBlt delays and the AUX */
      /* and color buffers share the same DibSection!! */
      /* GdiFlush(); */
    }
  }
}

void
_hwcGUIDrawPixelBuffer(HwcSimulator *hws, int x, int y, FxU32 buffer, HwcPixel pix, HwcBufferType which)
{
  HwcWindow *cw;
  FxU8 *dest;
  FxU8 r, g, b;

  /* Since thing can happen asynchronously here due to PSIM execution we need 
     to ensure we are not being called after we have deleted the DIB check 
     we have a valid DIB */

  if ( hBM == NULL )
    return;
 
  /* check pixel is within bitmap */

  if (( x < 0 ) || ( y < 0 ) || 
      (x >= bmi.bmiHeader.biWidth) || (y >= -bmi.bmiHeader.biHeight)) {
      return;
  }

  /* convert and copy the framebuffer data to the bitmap */

  dest = (FxU8*) pvbits;
  dest += 3 * (y * bmi.bmiHeader.biWidth + x);
  
  /* amazingly BitBlt is as fast as FillREct! */

  for (cw = hws->windows; cw != NULL; cw = cw->next) {
    /* if addr falls within this window */
    if ( buffer == cw->buffer->type ) {
      hwcPixelToRGB(cw->buffer, pix, &r, &g, &b);
	   dest[0] = (FxU8)b;
      dest[1] = (FxU8)g;
      dest[2] = (FxU8)r;
      BitBlt((HDC) cw->hDC,x,y,1,1,bmDC,x,y,SRCCOPY);
      /* this is WAY too slow.... */
      /* we need this because otherwise BitBlt delays and the AUX */
      /* and color buffers share the same DibSection!! */
      /* GdiFlush(); */
    }
  }
}

FX_EXPORT void FX_CSTYLE 
hwcGUIDrawPixel(HwcSimulator *hws, int x, int y, FxU32 addr, HwcPixel pix, HwcBufferType which)
{
    _hwcGUIDrawPixel(hws, x, y, addr, pix, which);
}

/* process accelerator keys */

FX_EXPORT int FX_CSTYLE 
hwcTranslateAccelerator(HWND hWnd, MSG *msg)
{
  if (TranslateAccelerator(msg->hwnd, (HACCEL) hwcInfo.hAccel, msg)) return TRUE;
  if (IsDialogMessage(hwndCsimMainControl,msg)) return TRUE;
  /* the parent turns out to be the csim property dialog box */
  if (IsDialogMessage(GetParent(msg->hwnd),msg)) return TRUE;
  return FALSE;
}

#define     CTRL( x, y) GetDlgItem( x, y)

static char errMsgBuf[1024];

static BOOL WINAPI 
ErrorProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    HWND hWndErrorIcon = CTRL(hWnd, IDC_ERROR_ICON);
    HWND hWndMessage = CTRL(hWnd, IDC_ERROR_MESSAGE);
    HWND hWndIgnoreErrors = CTRL(hWnd, IDC_IGNORE_ERRORS);
    HICON icon;

    switch( msg) {
        case WM_INITDIALOG:
        {
            icon = LoadIcon( NULL, IDI_EXCLAMATION);
    
            SendMessage(hWndErrorIcon, STM_SETIMAGE, IMAGE_ICON, (LPARAM)icon);
            SetWindowText(hWnd, "HWC Error");
            CheckDlgButton(hWndIgnoreErrors, IDC_IGNORE_ERRORS, hwcInfo.ignoreErrors);
            SendMessage( hWndMessage, WM_SETTEXT, (WPARAM)0, (LPARAM)errMsgBuf);

        } 
        return TRUE ;

        case WM_COMMAND:
        {
            switch( LOWORD( wParam)) {
                case ID_END_TASK:
                    EndDialog( hWnd, TRUE) ;
                    exit(1);
                    return TRUE ;
                case IDOK:
                    EndDialog( hWnd, TRUE) ;
                    return TRUE ;
                case IDC_DEBUG: // invoke debugger
                    hwcInfo.debug = FXTRUE;
                    EndDialog( hWnd, TRUE) ;
                    return TRUE ;
                case IDC_IGNORE_ERRORS:
                  hwcInfo.ignoreErrors = !hwcInfo.ignoreErrors;
                  return TRUE;
                default:
                    return FALSE ;
            }
        }
        default:
            return FALSE ;
    } 
    return FALSE ;
}

// callback routine from GDBG to HWC upon a GDBG_ERROR
void
hwcGDIGdbgErrorCallback(const char* const procName,
                 const char* const format,
                 va_list           args)
{
    hwcInfo.debug = FXFALSE;
    if ( !hwcInfo.ignoreErrors ) {
        vsprintf(errMsgBuf, format, args);
        DialogBoxParam( (HINSTANCE) hwcInfo.hInstance, MAKEINTRESOURCE(IDD_ERROR),
                        NULL /* hwndCsimMainControl */, ErrorProc, 0) ;
    }
    if ( hwcInfo.debug ) {
        hwcInvokeDebugger();
    }

}

/*---------------------------------------------------------------------------
   update dynamic text boxes, this is fairly slow so we don't want to do it
   all the time
  ---------------------------------------------------------------------------*/
void
hwcGDIUpdateDynamicText(HwcSimulator *hws)
{
    char buf[80];

    if (hws->environment.valid) return;
    hws->environment.valid = FXTRUE;
    /* XXX: need to get statistics */
    sprintf(buf,"%d",hws->stats.framesDrawn);
    SetDlgItemText(hwndCsimMainControl,IDC_CURRENT_FRAME_NUM,buf);
    sprintf(buf,"%d",hws->stats.trisDrawnPerFrame);
    SetDlgItemText(hwndCsimMainControl,IDC_TRIANGLE_COUNT,buf);
    sprintf(buf,"%d",hws->stats.trisDrawnPerFrame);
    SetDlgItemText(hwndCsimMainControl,IDC_FRAME_TRIANGLE_COUNT,buf);
    sprintf(buf,"%d",hws->stats.pixelsIn);
    SetDlgItemText(hwndCsimMainControl,IDC_PIXELS_IN,buf);
    sprintf(buf,"%d", hws->vecInfo.nextVector);
    SetDlgItemText(hwndCsimMainControl,IDC_TRACE_NUMBER,buf);
    sprintf(buf,"%d",hws->stats.texDownloads);
    SetDlgItemText(hwndCsimMainControl,IDC_TEX_DOWNLOADS,buf);
    sprintf(buf,"%d",hws->stats.texBytes);
    SetDlgItemText(hwndCsimMainControl,IDC_TEX_BYTES,buf);
    sprintf(buf,"%d",hws->stats.palDownloads+hws->stats.nccDownloads);
    SetDlgItemText(hwndCsimMainControl,IDC_NCC_DOWNLOADS,buf);
}

/*---------------------------------------------------------------------------
   generic proc for about box dialogs, this one is local to this DLL
   NOTE: we count on FX_CSTYLE to be the same as CALLBACK (__stdcall)
  ---------------------------------------------------------------------------*/
BOOL FX_EXPORT FX_CSTYLE
hwcGDIAboutBoxDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  switch(uMsg) {
  case WM_INITDIALOG:
    return TRUE;

  case WM_COMMAND:
    switch(LOWORD(wParam)) {
    case IDOK:
    case IDCANCEL:
      EndDialog(hwndDlg,wParam);
      return TRUE;
     }
     break;
  }
  return FALSE;
}

/* HACK: global combo text list for the GDBG_LEVEL control */
static char comboText[64][80];
static int comboTextN;

/*---------------------------------------------------------------------------
   the properties/control dialog window proc
  ---------------------------------------------------------------------------*/
static
BOOL CALLBACK 
hwcMainControl( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  int temp=0;
  HWND hWnd = GetParent(hwndDlg);
  HWND gdbgCombo = GetDlgItem(hwndDlg, IDC_COMBO_GDBG_LEVEL);
  HwcWindow *cw = (HwcWindow *)GetWindowLong(hWnd,GWL_USERDATA);
  HwcSimulator *hws = cw->hws;

  switch(uMsg) {
  case WM_INITDIALOG:
    CheckDlgButton(hwndDlg, IDC_FLUSH_AFTER_COMMANDS, hws->environment.flushOnCommands);
    CheckDlgButton(hwndDlg, IDC_PAUSE, hws->environment.paused);
    CheckDlgButton(hwndDlg, IDC_SAVE_AFTER_SWAP, hws->environment.saveAfterSwap);
    CheckDlgButton(hwndDlg, IDC_RESET_PIXELS_IN, hws->environment.resetPixelsIn);
    CheckDlgButton(hwndDlg, IDC_IEEE_MATH, hws->environment.ieeeMath);
    CheckDlgButton(hwndDlg, IDC_BIT_ACCURATE, hws->environment.bitAccurate);
    CheckDlgButton(hwndDlg, IDC_SKIP_RENDERING, hws->environment.skipRendering);
    CheckDlgButton(hwndDlg, IDC_ANALYTICAL_RASTER, hws->environment.rasterAlg);
    CheckRadioButton(hwndDlg,IDC_PAUSE_AFTER_NEXT_SWAP,IDC_PAUSE_BEFORE_FRAME,
         hws->environment.pauseAfterNextSwap ? IDC_PAUSE_AFTER_NEXT_SWAP : IDC_PAUSE_BEFORE_FRAME);
    CheckRadioButton(hwndDlg,IDC_PAUSE_AFTER_NEXT_TRIANGLE,IDC_PAUSE_BEFORE_TRIANGLE,
         hws->environment.pauseAfterNextTriangle ? IDC_PAUSE_AFTER_NEXT_TRIANGLE : IDC_PAUSE_BEFORE_TRIANGLE);
    /* add all the items into the combo box */
    for (temp=0; temp<comboTextN; temp++) {
       SendMessage(gdbgCombo, CB_ADDSTRING, 0, (LONG)comboText[temp]); 
    }
    SendMessage(gdbgCombo, CB_SETCURSEL, 0, 0);

    return TRUE;

  case WM_TIMER:
    GDBG_INFO(30,"csim main control dialog: WM_TIMER: 0x%x 0x%x %d\n",
              HIWORD(wParam),LOWORD(wParam),lParam);
    ASSERT(hwndDlg == hwndCsimMainControl);
    hwcGUIReadMessageQueue();
    hwcGDIUpdateDynamicText(hws);
            return TRUE;


    case WM_COMMAND:
      GDBG_INFO(7,"csim main control dialog: WM_COMMAND: 0x%x 0x%x %d\n",
                HIWORD(wParam),LOWORD(wParam),lParam);
      ASSERT(hwndDlg == hwndCsimMainControl);
      switch(LOWORD(wParam)) {
      case IDC_FLUSH_AFTER_COMMANDS:
        hws->environment.flushOnCommands = !hws->environment.flushOnCommands;
        return TRUE;

      case IDC_PAUSE:
        if (hws->windows)
          SendMessage((HWND) hws->windows->hWnd, WM_COMMAND,
                      hws->environment.paused ? ID_DEBUG_GO : ID_DEBUG_BREAK, 0);
          return TRUE;

      case IDC_SAVE_AFTER_SWAP:
        hws->environment.saveAfterSwap = !hws->environment.saveAfterSwap;
        return TRUE;

       case IDC_RESET_PIXELS_IN:
          hws->environment.resetPixelsIn = !hws->environment.resetPixelsIn;
          return TRUE;

      case IDC_IEEE_MATH:
        hws->environment.ieeeMath = !hws->environment.ieeeMath;
        return TRUE;

      case IDC_BIT_ACCURATE:
        hws->environment.bitAccurate = !hws->environment.bitAccurate;
        return TRUE;

      case IDC_SKIP_RENDERING:
        hws->environment.skipRendering = !hws->environment.skipRendering;
        return TRUE;

      case IDC_ANALYTICAL_RASTER:
        hws->environment.rasterAlg = !hws->environment.rasterAlg;
        return TRUE;

      case IDC_PAUSE_AFTER_NEXT_SWAP:
        hws->environment.pauseAfterNextSwap = 1;
        return TRUE;

      case IDC_PAUSE_BEFORE_FRAME:
        hws->environment.pauseAfterNextSwap = 0;
        return TRUE;

      case IDC_PAUSE_AFTER_NEXT_TRIANGLE:
        hws->environment.pauseAfterNextTriangle = 1;
        return TRUE;
      case IDC_PAUSE_BEFORE_TRIANGLE:
        hws->environment.pauseAfterNextTriangle = 0;
        return TRUE;

      CASE_UPDOWN(hwndDlg,IDC_FLUSH_COUNT,IDC_UPDOWN1,hws->environment.flushCount,0);
      CASE_UPDOWN(hwndDlg,IDC_POLL_LIMIT,IDC_UPDOWN2,hwcInfo.pollLimit,0);
      CASE_UPDOWN(hwndDlg,IDC_EDIT3,IDC_UPDOWN3,temp,0);

      case IDC_PAUSE_BEFORE_FRAME_NUM:
        if (HIWORD(wParam) == EN_KILLFOCUS) {
          char buf[80];
          GetDlgItemText(hwndDlg,IDC_PAUSE_BEFORE_FRAME_NUM,buf,sizeof(buf));
          sscanf(buf,"%i",&hws->environment.pauseAfterSwapNum);
          sprintf(buf,"%d",hws->environment.pauseAfterSwapNum);
          SetDlgItemText(hwndDlg,IDC_PAUSE_BEFORE_FRAME_NUM,buf);
          return TRUE;
        }
        break;

        case IDC_PAUSE_BEFORE_TRIANGLE_NUM:
            if (HIWORD(wParam) == EN_KILLFOCUS) {
                char buf[80];
                GetDlgItemText(hwndDlg,IDC_PAUSE_BEFORE_TRIANGLE_NUM,buf,sizeof(buf));
                sscanf(buf,"%i",&hws->environment.pauseAfterTriangleNum);
                sprintf(buf,"%d",hws->environment.pauseAfterTriangleNum);
                SetDlgItemText(hwndDlg,IDC_PAUSE_BEFORE_TRIANGLE_NUM,buf);
                return TRUE;
            }
            break;

      case IDC_BUTTON_GDBG:
        {
           char *cp = comboText[0];

           /* accept the current text entry, first get the text */
           SendMessage(gdbgCombo, WM_GETTEXT, sizeof(comboText[0]), (LPARAM)cp);
           while (isspace(*cp)) cp++;      /* skip over white space */
             if (*cp == '\0') return TRUE; /* if no tokens return */

             /* if not already in the combo box then add it */
             if (CB_ERR == SendMessage(gdbgCombo, CB_FINDSTRINGEXACT, 
                                       (WPARAM)-1, (LPARAM)cp)) {
               GDBG_INFO(7,"combo box: add '%s'\n", comboText[0]);
                         SendMessage(gdbgCombo, CB_INSERTSTRING, (WPARAM)0, 
                                     (LONG)cp); 
              }
              /* now */
              GDBG_INFO(7,"combo box: '%s'\n", cp);
              gdbg_parse(cp);
              return TRUE;
        }

      case IDCANCEL:
          /* kill any timers associated with this */
          KillTimer(hwndCsimMainControl,WM_TIMER_ID);
          /* save all the combo box text entries in our global array */
          temp = SendMessage(gdbgCombo, CB_GETCOUNT, 0, 0);
          if (temp > 64) temp = 64;
          for (comboTextN=0; comboTextN<temp; comboTextN++)
            SendMessage(gdbgCombo, CB_GETLBTEXT, comboTextN, 
                        (LPARAM)comboText[comboTextN]);
            DestroyWindow(hwndDlg);
            hwndCsimMainControl = NULL;
            return TRUE;
      }
      break;
  }
  return FALSE;
}

/*---------------------------------------------------------------------------
   create a spin control next to a buddy window
   NOTE: we do not use UDS_ALIGNRIGHT as that makes the spinner overap the
        3D controls a little, so we place it on the right side ourselves
  ---------------------------------------------------------------------------*/
HWND
hwcGDICreateSpin(
        HWND parent,            /* the parent window */
        int spinID,             /* the ID for this control */
        int buddyID,            /* the ID for the buddy control */
        int min,                /* the minimum pos value */
        int max,                /* the maximum pos value */
        int pos                 /* the initial pos value */
        )
{
  RECT rect;
  HWND buddyHWND, spin;

  buddyHWND = GetDlgItem(parent,buddyID);             /* get buddy window */
  GetClientRect(buddyHWND,&rect);                     /* and its coords */
  MapWindowPoints(buddyHWND,parent,(LPPOINT)&rect,2); /* convert to parent coords */
  spin = CreateUpDownControl(WS_CHILD | WS_BORDER | WS_VISIBLE |
                             UDS_ARROWKEYS | UDS_SETBUDDYINT,
                             rect.right+2,rect.top-2,12,rect.bottom-rect.top+4,
                             parent, spinID, GetModuleHandle(NULL),
                             buddyHWND,max,min,pos);
  if (spin == NULL)
    GDBG_ERROR("hwcGDICreateSpin", "CreateUpDownControl failed\n");
  return spin;
}

/*---------------------------------------------------------------------------
   the main window proc, it processes menu commands
  ---------------------------------------------------------------------------*/
LRESULT CALLBACK 
hwcMainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  HwcWindow *cw = (HwcWindow *)GetWindowLong(hWnd,GWL_USERDATA);
  HwcSimulator *hws = NULL;

  if ( cw )
	  hws = cw->hws;

  switch (msg)
  {
    case WM_CREATE:
      /* load up the accelerator table for CSIM */
      hwcInfo.hAccel = LoadAccelerators((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE),
                                        MAKEINTRESOURCE(IDR_ACCELERATOR1));
      break;
    case WM_COMMAND:
    switch(LOWORD(wParam)) {
      case ID_FILE_EXIT:
        GDBG_INFO(7,"ID_FILE_EXIT main\n");
        if (IDYES ==MessageBox(NULL, "Are you sure you want to exit?", "Csim",
                           MB_SETFOREGROUND | MB_TASKMODAL | MB_ICONQUESTION | 
                           MB_YESNO | MB_DEFBUTTON2))
           PostQuitMessage(0);
         break;

       case ID_WINDOW_NEWWINDOW:
         GDBG_INFO(7,"ID_WINDOW_NEWWINDOW 0x%x\n",hWnd);
         if ( hws )
             hwcNewSimViewWindow( hws, hws->hwc->stdBuffers[HWC_BUF_3D_RENDER]);
         break;

       case ID_NEW_MSWIN:
         GDBG_INFO(7,"ID_WINDOW_NEWWINDOW 0x%x\n",hWnd);
         if ( hws )
             hwcNewSimViewWindow( hws, hws->hwc->stdBuffers[HWC_BUF_MS]);
         break;

       case ID_VIEW_PROPERTIES:
       case ID_WINDOW_PROPERTIES:
         GDBG_INFO(7,"ID_WINDOW_PROPERTIES 0x%x\n",hWnd);
         if (hws && (hwndCsimMainControl == NULL)) {
            char buf[80];
            hwndCsimMainControl = CreateDialog((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE),
                                                MAKEINTRESOURCE(IDD_CSIM_MAIN),
                                                hWnd,
                                                hwcMainControl);
          /* initialize the spin controls */
          hwcGDICreateSpin(hwndCsimMainControl,IDC_UPDOWN1, IDC_FLUSH_COUNT, -1,9999,
                           hws->environment.flushCount);
          hwcGDICreateSpin(hwndCsimMainControl,IDC_UPDOWN2, IDC_POLL_LIMIT, 1,10000,
                       hwcInfo.pollLimit);
          hwcGDICreateSpin(hwndCsimMainControl,IDC_UPDOWN3, IDC_EDIT3, 1,16,1);
          { 
            sprintf(buf,"%d",hws->environment.pauseAfterSwapNum);
            SetDlgItemText(hwndCsimMainControl,IDC_PAUSE_BEFORE_FRAME_NUM,buf);
            sprintf(buf,"%d",hws->environment.pauseAfterTriangleNum);
            SetDlgItemText(hwndCsimMainControl,IDC_PAUSE_BEFORE_TRIANGLE_NUM,buf);
            SetTimer(hwndCsimMainControl,WM_TIMER_ID,WM_TIMER_MSECS,NULL);
            hws->environment.valid = 0;
          }
        }
        else 
          SetActiveWindow(hwndCsimMainControl);
          
             break;

    case ID_WINDOW_CLOSE:
      GDBG_INFO(7,"ID_WINDOW_CLOSE 0x%x main\n",hWnd);
      SendMessage(hWnd,WM_CLOSE,0,0);
      break;

    case ID_WINDOW_CLOSE_ALL:
      GDBG_INFO(7,"ID_WINDOW_CLOSE_ALL 0x%x\n",hWnd);
      if ( hws )
          hwcDeleteAllWindows(hws);
      break;

    case ID_HELP_CONTENTS:
      GDBG_INFO(7,"ID_HELP_CONTENTS\n");
      break;
    case ID_HELP_ABOUT:
      GDBG_INFO(7,"ID_HELP_ABOUT main\n");
      DialogBox((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE),
                MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, hwcGDIAboutBoxDialogProc);
      break;
    
    default:
      return(DefWindowProc(hWnd, msg, wParam, lParam));
    }
    break;

    case WM_SYSCOLORCHANGE:                 /* CTL3D hooks */
      break;

    case WM_KEYDOWN:
      switch(wParam) {
      case VK_LEFT:  
        queue[qhead++] = '4';
        qhead &= 255;
        break;
      case VK_RIGHT:  
        queue[qhead++] = '6';
        qhead &= 255;
        break;
      case VK_UP:
        queue[qhead++] = '8';
        qhead &= 255;
        break;
      case VK_DOWN:
        queue[qhead++] = '2';
        qhead &= 255;
        break;
      }
      break;

    case WM_CHAR:
      // printf("char: got here %d\n", wParam);
      queue[qhead++] = wParam;
      qhead &= 255;
      break;

    case WM_CLOSE:                          /* close the window */
      GDBG_INFO(7,"WM_CLOSE csim Main 0x%x\n",hWnd);
      hwcDeleteWindow(cw);
    break;

    case WM_MOVE:
    case WM_SIZE:
      if ( cw && cw->hws)
          hwcWriteWindowPlacement( hwcWindowName(cw->hws, szMainName), hWnd );
      break;
    
    default:
      return(DefWindowProc(hWnd, msg, wParam, lParam));
    }
    return(0);
}

void 
hwcGDIUnrealizeWindow(HwcWindow *cw)
{
  if ( !cw->clientWindow ) {
    DestroyWindow((HWND) cw->hWnd);
  }
}


#if 0
// TODO: either fix this up and use it or delete it.  GJS

HWND invalidWnd;
RECT invalidRect;

void
_hwcGDIInvalidateHack(HwcSimulator* hws)
{
    RECT zeroRect = {0, 0, 0, 0};
    RECT tmpRect = invalidRect;
    HWND tmpWnd = invalidWnd;

    if ( tmpWnd ) {
        invalidRect = zeroRect;
        invalidWnd = 0;
        InvalidateRect(tmpWnd, &tmpRect, FALSE);    /* invalidate the rect */
        (*hws->GUIKeepAlive)(hws, (tmpRect.bottom - tmpRect.top) * (tmpRect.right - tmpRect.left));

    }
}
#endif

/*---------------------------------------------------------------------------
   redisplay a rectangle of pixels in a buffer
  ---------------------------------------------------------------------------*/
void 
_hwcGDIUpdateRect(HwcSimulator *hws, int xmin, int ymin, int w, int h, FxU32 addr, FxU32 memType, HwcBufferType which)
{
  HwcWindow *cw;
  RECT rect;

  /* Since thing can happen asynchronously here due to PSIM execution we need 
     to ensure we are not being called after we have deleted the DIB check 
     we have a valid DIB */

  if ( hBM == NULL )
    return;

  /* repaint all windows showing this buffer */

  for (cw = hws->windows; cw != NULL; cw = cw->next) {
     /* if addr falls within this window */
     if (addr >= cw->buffer->physicalBase && addr < (int)cw->buffer->physicalEnd) {
         int xOffset = GetScrollPos ((HWND) cw->hWnd, SB_HORZ);
         int yOffset = GetScrollPos ((HWND) cw->hWnd, SB_VERT);

         rect.left   = xmin-xOffset;
         rect.top    = ymin-yOffset;
         rect.right  = rect.left + w;
         rect.bottom = rect.top +h;
       
         /* clip update rectangle to bitmap */

         if ( rect.left < 0 )
             rect.left = 0;
       
         if ( rect.top < 0 )
             rect.top = 0;

         if ( rect.right > bmi.bmiHeader.biWidth )
           rect.right = bmi.bmiHeader.biWidth;

         if ( rect.bottom > -bmi.bmiHeader.biHeight )
           rect.bottom = -bmi.bmiHeader.biHeight;

         /* see if anything remains to be drawn */

         if (( rect.left >= rect.right ) || ( rect.top >= rect.bottom ))
             return;

         InvalidateRect((HWND) cw->hWnd,&rect,FALSE);    /* invalidate the rect */

         // invalidWnd = (HWND) cw->hWnd;
         // UnionRect(&invalidRect, &invalidRect, &rect);
     }
  }
}


/*---------------------------------------------------------------------------
   redisplay the contents of the window
  ---------------------------------------------------------------------------*/
void
hwcGDIRefreshWindow(HwcWindow *cw)
{
  GDBG_INFO(7,"hwcRefreshWindow(0x%x)\n",cw);
  InvalidateRgn((HWND) cw->hWnd, NULL, FALSE);
}

LRESULT CALLBACK clientWinProc(
    HWND hWnd,	// handle of window
    UINT uMsg,	// message identifier
    WPARAM wParam,	// first message parameter
    LPARAM lParam 	// second message parameter
   )
{
  HwcWindow *cw = (HwcWindow *)GetWindowLong(hWnd,GWL_USERDATA);

  if ( cw == NULL )
      return 0;

  switch (uMsg) {
  case WM_PAINT:
      return csimViewWndProc( hWnd, uMsg, wParam, lParam);
  default:
      return CallWindowProc((WNDPROC) cw->clientWndProc, (HWND) hWnd, uMsg, wParam, lParam);
  }
}

char *
hwcWindowName(const HwcSimulator *hws, const char *name)
{
  static char title[80];

  if ( hws && name ) {
      sprintf(title, "%s.%d  %s", hws->name, hws->hwc->bn,name);
  } else {
      strcpy(title, "no name");
  }

  return title;
}

/*---------------------------------------------------------------------------
   Open up a generic window
  ---------------------------------------------------------------------------*/
FxBool
hwcGDIRealizeWindow(HwcWindow *cw, CHAR *className, const char *name, int w, int h)
{
  char *title;
  int xb,yb, xs, ys, cap;
  HWND hWnd;
  HwcSimulator *hws = cw->hws;
  DWORD style = WS_OVERLAPPEDWINDOW;
  int screenWidth =  GetSystemMetrics(SM_CXSCREEN);
  int screenHeight =  GetSystemMetrics(SM_CYSCREEN);

  if ( cw->clientWindow ) {
      cw->clientWndProc = (void *)SetWindowLong((HWND) cw->hWnd, GWL_WNDPROC, (LONG)clientWinProc);
  } else {
    // limit size of window to display buffer in
    if ( w > ((2*screenWidth)/3)) w = (2*screenWidth)/3;
    if ( h > ((2*screenHeight)/3)) h = (2*screenHeight)/3;
    if ( className == szViewClassName ) {
        h += GetSystemMetrics(SM_CYCAPTION);
        style |= WS_VSCROLL | WS_HSCROLL;
    }
    xb = GetSystemMetrics(SM_CXFRAME);
    yb = GetSystemMetrics(SM_CYFRAME);
    xs = GetSystemMetrics(SM_CXVSCROLL);
    ys = GetSystemMetrics(SM_CXHSCROLL);
    cap = GetSystemMetrics(SM_CYCAPTION);

    title = hwcWindowName(hws, name);

    hWnd = CreateWindow(className, title, style,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        w + xb + xb + xs, // allow room for vertical scroll bar
                        h + yb + yb + ys + cap + cap+1,
                        NULL, NULL, (HINSTANCE) hwcInfo.hInstance, NULL);
    cw->hWnd = hWnd;
    hwcReadWindowPlacement( title, hWnd );
  }

  cw->hDC = GetDC((HWND) cw->hWnd);
  cw->hWndDisplaySettings = NULL;
  SetWindowLong((HWND) cw->hWnd, GWL_USERDATA, (FxU32)cw);
  if ( !cw->clientWindow ) {
    if (hws->environment.useClientWindows) {
      SetWindowPos((HWND) cw->hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW|SWP_NOACTIVATE);
    } else {
      ShowWindow((HWND) cw->hWnd, SW_SHOWNOACTIVATE);
    }
  }
  InvalidateRgn((HWND) cw->hWnd, NULL, FALSE);  /* force a WM_PAINT message */
  GDBG_INFO(102,"hwcNewWindow(%s, %d,%s), %d windows open\n", 
            hws->name, hws->hwc->bn, name, hws->hWndCount);
  return FXTRUE;
}

void
hwcGDIUpdateScroll(HwcWindow *pWnd, FxBool reset)
{
  RECT rect;
  int cap = GetSystemMetrics(SM_CYCAPTION);
  SCROLLINFO scrollInfo;
  char szBuf[256];
  char *title;
  HwcBuffer *pBuff = pWnd->buffer;
  int x = GetScrollPos ((HWND) pWnd->hWnd, SB_HORZ);
  int y = GetScrollPos ((HWND) pWnd->hWnd, SB_VERT);
  static int i = 0;

  if (( pBuff == NULL ) || pWnd->clientWindow )
    return;

  if ( reset )
    x = y = 0;

  GetClientRect ((HWND) pWnd->hWnd, &rect);

  if ( rect.right >= (FxI32)pBuff->width)
    x = 0;
  
  if ( rect.bottom >= (FxI32)pBuff->height)
    y = 0;
  
  scrollInfo.cbSize = sizeof(scrollInfo);
  scrollInfo.fMask = SIF_ALL|SIF_DISABLENOSCROLL;
  scrollInfo.nMin = 0;
  scrollInfo.nMax = pBuff->width-1;
  scrollInfo.nPage = rect.right;
  scrollInfo.nPos = x;
  SetScrollInfo((HWND) pWnd->hWnd, SB_HORZ, &scrollInfo, TRUE);

  scrollInfo.cbSize = sizeof(scrollInfo);
  scrollInfo.fMask = SIF_ALL|SIF_DISABLENOSCROLL;
  scrollInfo.nMin = 0;
  scrollInfo.nMax = pBuff->height-1;
  scrollInfo.nPage = rect.bottom-cap;
  scrollInfo.nPos = y;
  SetScrollInfo((HWND) pWnd->hWnd, SB_VERT, &scrollInfo, TRUE);
  
  title = hwcWindowName(pWnd->hws, pBuff ? pBuff->name:"");
  SetWindowText((HWND) pWnd->hWnd, title);
  sprintf(szBuf, "%s (%d, %d)", title, x, y);
  SetWindowText((HWND) pWnd->hWnd, szBuf);

  InvalidateRect((HWND) pWnd->hWnd, NULL, FALSE);    /* invalidate the client area */
}

FxBool 
hwcGDIWindowSetBuffer(void *hWnd, HwcWindow *pWnd, HwcBuffer *pBuff)
{
  char *title;
  HwcSimulator *hws;
  char buff[80];

  // see if we are processing a client window, if so get its window data
  if ( pWnd == NULL ) {
      if ( hWnd != NULL ) {
          pWnd = (HwcWindow *)GetWindowLong((HWND) hWnd, GWL_USERDATA);

          // no window structure, try to allocate one
          if ( pWnd == NULL ) 
              return ( hwcNewClientWindow(hWnd, pBuff) != NULL);
      }
  }

  if ( pWnd == NULL ) 
      return FXFALSE;

  hws = pWnd->hws;
  pWnd->buffer = pBuff;

  /* toggle the hex display and reformat the status bar text */
  if ( !pWnd->clientWindow ) {
    SendMessage((HWND) pWnd->hWnd,WM_COMMAND,ID_VIEW_HEXADECIMAL,0);
    SendMessage((HWND) pWnd->hWnd,WM_USER_RESIZE,0,0);   /* recompute client clipping */
    SendMessage((HWND) pWnd->hWnd,WM_COMMAND,ID_WINDOW_REFRESH,0);
    if ( pBuff ) {
        hwcGDIUpdateScroll(pWnd, FXTRUE);
    }
    title = hwcWindowName(hws, pBuff ? pBuff->name:"");
    SetWindowText((HWND) pWnd->hWnd, title);
  } else {
      GetWindowText((HWND) pWnd->hWnd, buff, sizeof(buff));
      title = buff;
  }

  hwcWriteWindowPlacement( title, (HWND) pWnd->hWnd );
  return FXTRUE;
}

/*---------------------------------------------------------------------------
   initialize the display DIBs
  ---------------------------------------------------------------------------*/
static void InitDibs(HWND hWnd)
{
  // LOOOK make bitmap as large as the largest possible client area
  int screenWidth =  GetSystemMetrics(SM_CXSCREEN);
  int screenHeight =  GetSystemMetrics(SM_CYSCREEN);

  if ( hBM != NULL )
      return;
  bmi.bmiHeader.biSize    = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth    = 1280;
  bmi.bmiHeader.biHeight    = -1024;  /* top-down bitmap */
  bmi.bmiHeader.biPlanes    = 1;
  bmi.bmiHeader.biBitCount    = 24;
  bmi.bmiHeader.biCompression    = BI_RGB;
  bmi.bmiHeader.biSizeImage    = 0;
  bmi.bmiHeader.biXPelsPerMeter  = 0;
  bmi.bmiHeader.biYPelsPerMeter  = 0;
  bmi.bmiHeader.biClrUsed    = 3;
  bmi.bmiHeader.biClrImportant  = 0;
  ((DWORD *)bmi.bmiColors)[0] = 0xff0000;
  ((DWORD *)bmi.bmiColors)[1] = 0x00ff00;
  ((DWORD *)bmi.bmiColors)[2] = 0x0000ff;
  hBM = CreateDIBSection(GetDC(hWnd), (BITMAPINFO *) &bmi, DIB_RGB_COLORS,
                         &pvbits, NULL, 0);
  if (!hBM) {
    GDBG_ERROR("InitDibs", "CreateDIBSection failed %d\n",GetLastError());
  }
  /* NOTE: we use the main control window's DC (ok, a hack) */
  bmDC = CreateCompatibleDC(GetDC(hWnd));
  SelectObject(bmDC,hBM);
}

/*---------------------------------------------------------------------------
   redisplay the contents of the window
   XXX the BITMAP pointed to by pvbits should be treated as a cache
   if we are repainting a window with the same VSD formt data, then we
   should take advantage of that fact and not recopy/reconvert data
  ---------------------------------------------------------------------------*/
void
hwcGDIRepaintWindow(HwcWindow *cw, void *hDc, HwcRect *prect)
{
  int x,y, tx, ty, txmax, tymax, xOff, yOff;
  int save199 = GDBG_GET_DEBUGLEVEL(199);
  FxU8 *dest;
  HwcBuffer *buffer= cw->buffer;
  HwcSimulator *hws = cw->hws;
  HwcContext *hwc = hws->hwc;
  void *hSrcDC = bmDC; 

  // printf("repaint window %d, %d, %d, %d\n", prect->left, prect->top, prect->right, prect->bottom);

  ASSERT(cw != NULL);

  xOff = GetScrollPos((HWND) cw->hWnd, SB_HORZ);
  yOff = GetScrollPos((HWND) cw->hWnd, SB_VERT);

  /* Since thing can happen asynchronously here due to windows creation or PSIM 
     execution we need to ensure we are not being called before we have created 
     the buffer or after we have deleted the DIB check we have a valid buffer and DIB */

  if (( buffer == NULL ) || ( hBM == NULL ))
	return;

  if ((FxU32)prect->bottom > ( cw->buffer->height - yOff))  /* don't overwrite status bar */
    prect->bottom = cw->buffer->height - yOff;

  GDBG_SET_DEBUGLEVEL(199,0);

  if ( buffer->hints & HWC_BUFFER_WIREFRAME ) {
    hwcCheckBuffer(buffer);
    hSrcDC = buffer->hDC; 
  } else {
    /* convert and copy the framebuffer data to the bitmap */
    /* walk in 2x4 blocks to optimize decompression */
    for (y=prect->top; y<prect->bottom; y += 2) {
      tymax = MIN(y+2, prect->bottom);
      for (x=prect->left; x<prect->right; x += 4) {
        txmax = MIN(x+4, prect->right);
        for (ty=y; ty<tymax; ty++) {
          for (tx=x; tx<txmax; tx++) {
            FxU8 a,r,g,b;
      
            dest = (FxU8*) pvbits;
            dest += 3 * (ty * bmi.bmiHeader.biWidth + tx);
  
            hwcReadColor(cw->hws, buffer, tx+xOff, ty+yOff, &a, &r, &g, &b);
      
            switch (cw->videoFilter) {
            case 0:      /* none */
              break;
            case 1:      /* expand */
              r |= r >> 5;
              g |= g >> 6;
              b |= b >> 5;
              break;
            case 2:
              break;
            case 3:
              break;
            default:
              GDBG_ERROR("hwcRepaint","invalid videoFilter = %d\n", cw->videoFilter);
            }
  
            r = (FxU8)MIN(255, hwc->gamma.tableR[r]);
            g = (FxU8)MIN(255, hwc->gamma.tableR[g]);
            b = (FxU8)MIN(255, hwc->gamma.tableR[b]);
            dest[0] = b;
            dest[1] = g;
            dest[2] = r;
          }
        }  
      }
    }
  }

  GDBG_SET_DEBUGLEVEL(199,save199);

  /* now display it */

  x = BitBlt((HDC) hDc, prect->left, prect->top, prect->right - prect->left,
             prect->bottom - prect->top, (HDC) hSrcDC, prect->left, prect->top,SRCCOPY);
  GdiFlush();
  if (x == 0)
    GDBG_ERROR("hwcRepaint", "BitBlt failed  = %d\n",GetLastError());
}

/*---------------------------------------------------------------------------
   read and process all messages, this gets called every so many pixels
   exit upon a WM_QUIT message because we assume we are running a console app
  ---------------------------------------------------------------------------*/
void
hwcGDIReadMessageQueue(void)
{
  MSG msg;

  hwcInfo.debug = FXFALSE;
  while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
    if (!hwcTranslateAccelerator(NULL, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    if (msg.message == WM_QUIT) {
#if !defined(FX_DLL_ENABLE)
      hwcShutdown();
#endif
      exit(0);
    }
    if ( hwcInfo.debug == FXTRUE )
       hwcInvokeDebugger();
  }
}

/*-------------------------------------------------------------------
  Function: hwcGDIKbHit
  Date: 10/28/98
  Implementor(s): jdt
  Library: test library
  Description:
  Returns true if there are pending characters in the input queue
  Arguments:
  none
  Return:
  nonzero if keys in queue
  -------------------------------------------------------------------*/
extern HwcContext *lastContext;
int
hwcGDIKbHit( void )
{
  MSG msg;

  if (qhead != qtail) {
    return 1;
  }

  // LOOOK XXXX this is a hack, how should we figure out which context
  // to use for kbd input, perhaps input should take a context

  if ( lastContext && lastContext->hws && lastContext->hws->mainWindow )
    SetFocus( (HWND) lastContext->hws->mainWindow->hWnd);

  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);      /* this might change qhead */
    if (qhead != qtail) {
      return 1;
    }
  }
  return 0;
}

/*-------------------------------------------------------------------
  Function: hwcGDIGetCH
  Date: 10/28/98
  Implementor(s): jdt
  Library: test library
  Description:
  Returns character from top of input fifo, blocks if fifo is empty
  Arguments:
  none
  Return:
  character
  -------------------------------------------------------------------*/
char
hwcGDIGetCH( void )
{
  MSG     msg;
  char    rv;

  if (qtail != qhead) {
    rv = queue[qtail++];
    qtail &= 255;
    return rv;
  }

  // LOOOK XXXX this is a hack, how should we figure out which context
  // to use for kbd input, perhaps input should take a context

  if ( lastContext && lastContext->hws && lastContext->hws->mainWindow )
    SetFocus( (HWND) lastContext->hws->mainWindow->hWnd);

  while (GetMessage( &msg, NULL, 0, 0 )) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);

    if (qtail != qhead) {
      rv = queue[qtail++];
      qtail &= 255;
      return rv;
    }
  }

  /* Should never get here!! */
  /* printf("Bad exit..\n"); */
  /* fflush(stdout); */
  return 0;
}

/*---------------------------------------------------------------------------
  one time init routine
  ---------------------------------------------------------------------------*/
FxBool 
hwcGDIInit( void )
{
  WNDCLASS wc;


#if !defined(FX_DLL_ENABLE)
  GDBG_INIT();
#endif
  hwcInfo.hInstance = GetModuleHandle("hwc");
  GDBG_INFO(101,"hwcGUIInit(): hInstance=0x%x\n", hwcInfo.hInstance);

  if ((_hwcInstAtom = GlobalAddAtom("HWC")) == 0) {
    GDBG_ERROR("hwcGDIInit", "Could not create instance atom error = %d", GetLastError());
    return FXFALSE;
  }

  wc.style = CS_NOCLOSE | CS_DBLCLKS | CS_OWNDC; /* | CS_HREDRAW | CS_VREDRAW; */
  wc.lpfnWndProc   = hwcMainWndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = sizeof(HwcWindow *);
  wc.hInstance     = (HINSTANCE) hwcInfo.hInstance;
  wc.hIcon         = LoadIcon((HINSTANCE) hwcInfo.hInstance,MAKEINTRESOURCE(IDI_HWC));
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
  wc.lpszMenuName  = MAKEINTRESOURCE(IDR_APPMENU);
  wc.lpszClassName = szMainClassName;
  if (!RegisterClass(&wc)) {
    GDBG_ERROR("RegisterClass (%s)", "error = %d\n", wc.lpszClassName, 
               GetLastError());
    return FXFALSE;
  }
  /* now register the views */
  wc.style         = CS_DBLCLKS | CS_OWNDC; /* | CS_HREDRAW | CS_VREDRAW; */
  wc.lpfnWndProc   = csimViewWndProc;
  wc.lpszMenuName  = MAKEINTRESOURCE(IDR_APPMENU1);
  wc.lpszClassName = szViewClassName;
  if (!RegisterClass(&wc)) {
    GDBG_ERROR("RegisterClass (%s)", "error = %d\n", wc.lpszClassName, 
               GetLastError());
    return FXFALSE;
  }

  InitCommonControls();

  return FXTRUE;
}

/*---------------------------------------------------------------------------
   open a CSIM gui for a board
  ---------------------------------------------------------------------------*/
FxBool
hwcGDIGUIOpen( HwcSimulator *hws)
{
  if (hws->mainWindow == NULL) {
    hws->mainWindow = hwcNewWindow(hws,szMainClassName, szMainName, 320, 240);
  }
  InitDibs((HWND) hws->mainWindow->hWnd);
  return FXTRUE;
}

/*---------------------------------------------------------------------------
   shut down the CSIM gui environment
  ---------------------------------------------------------------------------*/

void
hwcGDIShutdown( void )
{
  GDBG_INFO(101,"hwcGUIShutdown()\n");
  return;
  DeleteDC(bmDC);
  DeleteObject(hBM); hBM = NULL;
  hwndCsimMainControl = NULL;
}

/* find a Csim window that contains hWnd and move it to front of list */
HwcWindow *
hwcFindWindow(HwcSimulator *hws, HWND hWnd)
{
  HwcWindow *cw;
  HwcWindow *wlast = NULL;

  if ( hws == NULL )
	  return NULL;

  cw = hws->windows;

  if (hws->mainWindow) {  /* first check the main window */
    if (hws->mainWindow->hWnd == hWnd)
      return hws->mainWindow;
  }
  while (cw) {                     /* search thru the entire list */
    if (cw->hWnd == hWnd) {        /* if found it */
      if (wlast) {                    /* if not the first entrys */
        wlast->next = cw->next;    /* then unlink it */
        cw->next = hws->windows;   /* and make it the head */
        hws->windows = cw;
      }
      return cw;
    }
    wlast = cw;
    cw = cw->next;
  }
  return cw;
}

static char szKey[] = "Software\\3dfx\\hwc" ;

FxBool 
hwcReadWindowPlacement( const char *name, HWND hWnd ) 
{
    HKEY hkey ;
    WINDOWPLACEMENT wPlace;
    DWORD dwType, dwSize = sizeof(wPlace);

    /* open/create the Software\\3dfx\\view key */
    
    if ( RegOpenKeyEx( HKEY_CURRENT_USER, szKey, 0, KEY_ALL_ACCESS, &hkey) != ERROR_SUCCESS) {
        return FXFALSE ;
    }

    /* read the defaults */

    if( RegQueryValueEx( hkey, name, NULL, &dwType, 
                         (BYTE *)&wPlace, &dwSize) !=ERROR_SUCCESS){
        return FXFALSE ;
    }

    /* set window placement */

    wPlace.showCmd = SW_SHOWNA;

    if (!SetWindowPlacement(hWnd, &wPlace)) {
        GDBG_ERROR("hwcReadDefault", "Error setting placement data\n");
        return FXFALSE ;
    }

    /* close the key */

    RegCloseKey( hkey) ;

    return FXTRUE ;
}

FxBool
hwcWriteWindowPlacement( const char *name, HWND hWnd) 
{
    HKEY hkey ;
    DWORD dwAction ;
    WINDOWPLACEMENT wPlace;

    if ( hWnd == NULL )
        return FXFALSE;

    /* open/create the Software\\3dfx\\view key */

    if( RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0L, "HWC", REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS, NULL, &hkey, &dwAction) != ERROR_SUCCESS) {
        GDBG_ERROR("hwcWriteDefault", "Error creating/opening the key\n") ;
        return FXFALSE ;
    }

    /* lets store the data */
    wPlace.length = sizeof(WINDOWPLACEMENT);
    if (!GetWindowPlacement(hWnd, &wPlace)) {
        GDBG_ERROR("hwcWriteWindowPlacement", "Error getting placement data\n") ;
        return FXFALSE ;
    }

    if( RegSetValueEx(  hkey, name, 0L, REG_BINARY, 
                       (BYTE *)&wPlace, sizeof(wPlace)) != ERROR_SUCCESS) {
        GDBG_ERROR("hwcWriteWindowPlacement", "Error saving placement data\n") ;
        return FXFALSE ;
    }

    /* close the key */

    RegCloseKey(hkey) ;
    return FXTRUE ;
}

void FX_EXPORT FX_CSTYLE
hwcDrawLine(struct _HwcBuffer *pBuff, HwcVertex *a, HwcVertex *b)
{
  POINT pts[2];

  hwcCheckBuffer(pBuff);

  pts[0].x = (int)a->x;
  pts[0].y = (int)a->y;
  pts[1].x = (int)b->x;
  pts[1].y = (int)b->y;
  Polyline( (HDC) pBuff->hDC, pts, 2); 
}

void FX_EXPORT FX_CSTYLE
hwcDrawTriangle(struct _HwcBuffer *pBuff, HwcVertex *a, HwcVertex *b, HwcVertex *c)
{
  POINT pts[4];

  hwcCheckBuffer(pBuff);

  pts[0].x = (int)a->x;
  pts[0].y = (int)a->y;
  pts[1].x = (int)b->x;
  pts[1].y = (int)b->y;
  pts[2].x = (int)c->x;
  pts[2].y = (int)c->y;
  pts[3].x = (int)a->x;
  pts[3].y = (int)a->y;
  Polyline((HDC) pBuff->hDC, pts, 4); 
}

void FX_EXPORT FX_CSTYLE
hwcFillRectangle(struct _HwcBuffer *pBuff, FxI32 x, FxI32 y, FxI32 w, FxI32 h)
{
  RECT rect;

  hwcCheckBuffer(pBuff);

  rect.left   = x;
  rect.top    = y; 
  rect.right  = x+w;
  rect.bottom = y+h;

  FillRect((HDC) pBuff->hDC, &rect, (HBRUSH) GetStockObject(BLACK_BRUSH));
}

#endif
