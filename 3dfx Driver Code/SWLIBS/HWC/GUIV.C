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
** $Revision: 5$
** $Date: 10/11/00 7:40:14 PM$
*/

#ifdef GUI

#include <windows.h>
#include <commctrl.h>
#include <ctl3d.h>

#include <stdio.h>
#include <fxhwc.h>
#include "gui.h"
#include "guimv.h"
#include <afxres.h>
#include <fximg.h>

HWND hwndCsimMainControl;

char *
hwcWindowName(const HwcSimulator *hws, const char *name);

void
hwcGDIUpdateScroll(HwcWindow *pWnd, FxBool bResize);

FxBool 
hwcReadWindowPlacement( const char *name, HWND hWnd );

FxBool
hwcWriteWindowPlacement( const char *name, HWND hWnd);

#define SIGN_EXTEND(x,nbits) ((((signed)(x))<<(32-(nbits)))>>(32-(nbits)))

/* XXX this goes somewhere else and really should do something!!! */
static void fileSave(HwcWindow *pWnd)
{
  HwcBuffer *pBuff = pWnd->buffer;

  GDBG_INFO(7,"save as %s \n",pWnd->fileSaveName);
  if (!hwcBufferSave(NULL, pBuff, pWnd->fileSaveName,IMG_UNKNOWN))
    MessageBox(NULL,
                "File Save Failed, see console output for more details\n"
                "The probable cause is an invalid file extension.\n"
                "Valid file extensions are .ppm, .sbi, or .tga.\n",
                "Csim",
                MB_SETFOREGROUND | MB_TASKMODAL | MB_OK | MB_ICONSTOP);
}

/*---------------------------------------------------------------------------
   common dialog control for doing a Save As
  ---------------------------------------------------------------------------*/
static void doSaveAs(HWND hWnd, HwcSimulator *hws)
{
  static OPENFILENAME ofn;
  char szFileTitle[256];
  char szFilter[] =  "PPM files\0*.ppm\0"
                     "Targa files\0*.tga\0"
                     "SBI files\0*.sbi\0"
                     "All files\0*.*\0";
  HwcWindow *pWnd = (HwcWindow *)GetWindowLong(hWnd,GWL_USERDATA);

  ASSERT(pWnd != NULL);
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hWnd;
  ofn.lpstrFilter = szFilter;
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = pWnd->fileSaveName;
  ofn.nMaxFile = sizeof(pWnd->fileSaveName);
  ofn.lpstrFileTitle = szFileTitle;
  ofn.nMaxFileTitle = sizeof(szFileTitle);
  ofn.lpstrInitialDir = NULL;
  ofn.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT;

  if (GetSaveFileName(&ofn)) {
    strcpy(pWnd->fileSaveName,ofn.lpstrFile);
    fileSave(pWnd);
  } else {
    printf("save as CANCELLED: error code = %d 0x%x\n",
    CommDlgExtendedError(), CommDlgExtendedError());
  }
}

/*---------------------------------------------------------------------------
   the properties/control dialog window proc
  ---------------------------------------------------------------------------*/
static
BOOL CALLBACK csimPropsProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  char buf[80];
  HWND hWnd = GetParent(hwndDlg);
  HwcWindow *pWnd = (HwcWindow *)GetWindowLong(hWnd,GWL_USERDATA);
  HwcSimulator *hws = pWnd->hws;
  HwcContext *hwc = hws->hwc;
  HwcBuffer *pBuff;

  ASSERT(pWnd != NULL);
  switch(uMsg) {
  case WM_INITDIALOG:
  {
    int temp;
    CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO4,IDC_RADIO1+pWnd->videoFilter);
    switch(pWnd->buffer->format) {
    case HWC_PIXFMT_P_8:  temp = IDC_RADIO5;  break;
    case HWC_PIXFMT_RGB_565:  temp = IDC_RADIO6;  break;
    case HWC_PIXFMT_RGB_888:  temp = IDC_RADIO7;  break;
    case HWC_PIXFMT_ARGB_8888:  temp = IDC_RADIO8;  break;
    }
    CheckRadioButton(hwndDlg,IDC_RADIO5,IDC_RADIO8,temp);
    return TRUE;
  }
#if 0
  case WM_ACTIVATE:
      GDBG_INFO(7,"csim props dialog: WM_ACTIVATE\n");
      hwndCsimProps = hwndDlg;
      return TRUE;
#endif
  case WM_COMMAND:
    ASSERT( hwndDlg == pWnd->hWndDisplaySettings );
    GDBG_INFO(7,"csim props dialog: WM_COMMAND: %d %d %d\n",LOWORD(wParam),wParam,lParam);
    switch(LOWORD(wParam)) {
    case IDC_RADIO1:
    case IDC_RADIO2:
    case IDC_RADIO3:
    case IDC_RADIO4:
      pWnd->videoFilter = LOWORD(wParam)-IDC_RADIO1;
    refresh:
      SendMessage(hWnd,WM_COMMAND,ID_WINDOW_REFRESH,0);
      return TRUE;

    case IDC_RADIO5:
      pWnd->buffer->format = HWC_PIXFMT_P_8;
      goto refresh;
    case IDC_RADIO6:
      pWnd->buffer->format = HWC_PIXFMT_RGB_565;
      goto refresh;
    case IDC_RADIO7:
      pWnd->buffer->format = HWC_PIXFMT_RGB_888;
      goto refresh;
    case IDC_RADIO8:
      pWnd->buffer->format = HWC_PIXFMT_ARGB_8888;
      goto refresh;

    case IDC_EDIT1:
      if (HIWORD(wParam) == EN_KILLFOCUS) {
          GetDlgItemText(hwndDlg,IDC_EDIT1,buf,sizeof(buf));
          sscanf(buf,"%i",&pWnd->buffer->physicalBase);
          pWnd->buffer->physicalBase &= ~15;
          sprintf(buf,"0x%x",pWnd->buffer->physicalBase);
          SetDlgItemText(hwndDlg,IDC_EDIT1,buf);
          goto refresh;
      }
      break;
    case IDC_EDIT2:
      if (HIWORD(wParam) == EN_KILLFOCUS) {
          GetDlgItemText(hwndDlg,IDC_EDIT2,buf,sizeof(buf));
          sscanf(buf,"%i",&pWnd->buffer->stride);
          pWnd->buffer->stride &= ~15;
          sprintf(buf,"0x%x",pWnd->buffer->stride);
          SetDlgItemText(hwndDlg,IDC_EDIT2,buf);
          goto refresh;
      }
      break;
    CASE_UPDOWN(hwndDlg,IDC_EDIT3,IDC_UPDOWN3,pWnd->zoom,
      SendMessage(GetParent(hwndDlg),WM_COMMAND,ID_VIEW_ZOOM_11-1+pWnd->zoom,0));
    case IDC_BUTTON_FRONT:
      pBuff = hwc->stdBuffers[HWC_BUF_3D_FRONT];
      goto changed_buffer;
    case IDC_BUTTON_BACK:
      pBuff = hwc->stdBuffers[HWC_BUF_3D_BACK];
      goto changed_buffer;
    case IDC_BUTTON_AUX:
      pBuff = hwc->stdBuffers[HWC_BUF_3D_AUX];
      goto changed_buffer;
    case IDC_BUTTON_GUI:
      pBuff = hwc->stdBuffers[HWC_BUF_DESKTOP0];
      /* recompute endBase and update dialog boxes */
    changed_buffer:
      {
        CHAR title[80];
        if ( pBuff == NULL )
            return TRUE;
        pWnd->buffer = pBuff;
        sprintf(buf,"0x%x",pWnd->buffer->physicalBase);
        SetDlgItemText(hwndDlg,IDC_EDIT1,buf);
        sprintf(buf,"0x%x",pWnd->buffer->stride);
        SetDlgItemText(hwndDlg,IDC_EDIT2,buf);
        sprintf(title, "%s.%d  %s", hws->name, hws->hwc->bn, pBuff->name);
        SetWindowText(hWnd,title);
        SendMessage(hWnd,WM_COMMAND,ID_WINDOW_REFRESH,0);
        return TRUE;
      }
    case IDCANCEL:
      DestroyWindow(hwndDlg);
      pWnd->hWndDisplaySettings = NULL;
      return TRUE;
     }
     break;
  }
  return FALSE;
}

/*---------------------------------------------------------------------------
   process a WM_MENUSELECT message - this figures out what is going on in the
    popup menus and updates the status bar text accordingly
  ---------------------------------------------------------------------------*/
static void doMenuSelect(HWND hWnd, HMENU hMenu, UINT uItem, UINT flags)
{
  char szStatBarText[256];
  UINT nID;

  GDBG_INFO(7,"doMenuSelect(*,0x%x,%d,0x%x)\n",hMenu,uItem,flags);
  if (hMenu==NULL && flags==0xFFFF) {    /* Exiting menu mode */
    HwcWindow *pWnd = (HwcWindow *)GetWindowLong(hWnd,GWL_USERDATA);
    HwcSimulator *hws = pWnd->hws;
    nID = hws->environment.paused ? IDS_PAUSED : IDS_READY;
  }
  else if (flags & MF_SYSMENU) {    /* System menu is up */
    if (flags & MF_POPUP)      /* System menu item is selected */
      nID = IDS_SYSMENU;      /* System menu and no item is selected */
    else
      nID = uItem;      /* uItem contains SC_* code. */
  }
  else {
    if ((flags & MF_POPUP)) {    /* pulling down a popup submenu */
      if ( uItem == 0 )          /* sort of crude - use popup index */
        nID = IDS_FILEMENU;
      else if ( uItem == 1 )
        nID = IDS_VIEWMENU;
      else if ( uItem == 2 )
        nID = IDS_DEBUGMENU;
      else if ( uItem == 3 )
        nID = IDS_WINDOWMENU;
      else if ( uItem == 4 )
        nID = IDS_HELPMENU;
    }
    else
      nID = uItem;
  }

  LoadString((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE), nID, szStatBarText, sizeof(szStatBarText)-1);
  SendMessage(GetDlgItem(hWnd,IDC_STATUSBAR), SB_SETTEXT, 0, (LPARAM)szStatBarText);
}

static void FX_CALL updateStatusBar(HwcWindow *pWnd)
{
  doMenuSelect((HWND) pWnd->hWnd,NULL,0,0xFFFF);
  SendMessage(GetDlgItem((HWND) pWnd->hWnd,IDC_STATUSBAR), WM_PAINT, 0, 0);
}

void
hwcGDIUpdateDynamicText(HwcSimulator *hws);

void 
hwcGDIDoBreak(HwcSimulator *hws)
{
  hws->environment.paused = 1;
  hwcApplyToAllWindows(hws, updateStatusBar);
  if (hwndCsimMainControl) {
    hwcGDIUpdateDynamicText(hws); // NEW
    SendMessage(hwndCsimMainControl,WM_INITDIALOG,0,0);
    UpdateWindow(hwndCsimMainControl);
    KillTimer(hwndCsimMainControl,WM_TIMER_ID); // NEW
  }
  while (hws->environment.paused) {
    WaitMessage();
    hwcGUIReadMessageQueue();
  }
  if (hwndCsimMainControl) { // NEW
    SetTimer(hwndCsimMainControl,WM_TIMER_ID,WM_TIMER_MSECS,NULL);
  }
}

static LRESULT doCommand (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  HwcWindow *pWnd = (HwcWindow *)GetWindowLong(hWnd,GWL_USERDATA);
  HwcSimulator *hws = pWnd->hws;
  HMENU hmenu = GetMenu(hWnd);

  ASSERT(pWnd != NULL);
  switch(LOWORD(wParam)) {
  case ID_FILE_SAVE:
  {
    GDBG_INFO(7,"ID_FILE_SAVE view\n");
    if (pWnd->fileSaveName[0])  /* if already has a name */
      fileSave(pWnd);    /* then use it */
    else        /* else do a save as */
      SendMessage(hWnd, WM_COMMAND, ID_FILE_SAVEAS, 0);
    break;
  }

  case ID_FILE_SAVEAS:
    GDBG_INFO(7,"ID_FILE_SAVEAS view\n");
    doSaveAs( hWnd, hws );
    break;

  case ID_FILE_EXIT:
    GDBG_INFO(7,"ID_FILE_EXIT view\n");
    if (IDYES ==MessageBox(NULL, "Are you sure you want to exit?", "Csim",
                           MB_SETFOREGROUND | MB_TASKMODAL | MB_ICONQUESTION | 
                           MB_YESNO | MB_DEFBUTTON2))
      PostQuitMessage(0);
    break;

  case ID_VIEW_ZOOM_11:
  case ID_VIEW_ZOOM_21:
  case ID_VIEW_ZOOM_31:
  case ID_VIEW_ZOOM_41:
  case ID_VIEW_ZOOM_51:
  case ID_VIEW_ZOOM_61:
  case ID_VIEW_ZOOM_71:
  case ID_VIEW_ZOOM_81:
  case ID_VIEW_ZOOM_91:
  case ID_VIEW_ZOOM_101:
  case ID_VIEW_ZOOM_111:
  case ID_VIEW_ZOOM_121:
  case ID_VIEW_ZOOM_131:
  case ID_VIEW_ZOOM_141:
  case ID_VIEW_ZOOM_151:
  case ID_VIEW_ZOOM_161:
  {
    TCHAR buf[256], *cptr;
    int z = LOWORD(wParam)-ID_VIEW_ZOOM_11+1;

    GetWindowText(hWnd,buf,sizeof(buf));
    cptr = strstr(buf,HWC_VIEW_WINDOW_NAME);
    if (cptr) {
      if (z > 1)
        sprintf(cptr,HWC_VIEW_WINDOW_NAME " (%d:1)",z);
      else cptr[strlen(HWC_VIEW_WINDOW_NAME)] = '\0';
      SetWindowText(hWnd,buf);
    }
    if (pWnd) pWnd->zoom = z;
    GDBG_INFO(7,"ID_VIEW_ZOOM %d title:%s\n",z,buf);
    if ((HWND) pWnd->hWndDisplaySettings)
    SendMessage((HWND) pWnd->hWndDisplaySettings,WM_COMMAND, MAKEWPARAM(IDC_EDIT3,EN_KILLFOCUS),0);
    break;
  }

  case ID_VIEW_FILTER_NONE:  /* MENU commands */
  case ID_VIEW_FILTER_EXPAND:
  case ID_VIEW_FILTER_2X2:
  case ID_VIEW_FILTER_4X1:
    GDBG_INFO(7,"ID_VIEW_FILTER 0x%x\n",hWnd);
    {
      pWnd->videoFilter = LOWORD(wParam) - ID_VIEW_FILTER_NONE;
      if (pWnd->hWndDisplaySettings) {
        CheckRadioButton((HWND) pWnd->hWndDisplaySettings, IDC_RADIO1,IDC_RADIO4,
          IDC_RADIO1+LOWORD(wParam)-ID_VIEW_FILTER_NONE);
      }
      /* now refresh this window since it will be different */
      SendMessage(hWnd,WM_COMMAND,ID_WINDOW_REFRESH,0);
      break;
    }

  case ID_VIEW_HEXADECIMAL:
	  {
		  HWND hStatus = GetDlgItem(hWnd,IDC_STATUSBAR);
    pWnd->hex ^= 1;
    SendMessage(GetDlgItem(hWnd,IDC_STATUSBAR), SB_SETTEXT, 4, 
               (LPARAM)(pWnd->hex ? " hex" : " dec"));
	  }
    break;

  case ID_VIEW_STATUSBAR:
  {
    LONG style;
    RECT rect;

    GDBG_INFO(7,"ID_VIEW_STATUSBAR\n");
    style = GetWindowLong(GetDlgItem(hWnd,IDC_STATUSBAR),GWL_STYLE);
    /* if visible, then make invisible */
    if (pWnd->statusBar) {
      GetWindowRect(GetDlgItem(hWnd,IDC_STATUSBAR),&rect);  /* get this before making invisible */
      SetWindowLong(GetDlgItem(hWnd,IDC_STATUSBAR), GWL_STYLE, style & ~WS_VISIBLE);
      pWnd->statusBar = 0;
    }
    else {  /* if invisible, then make visible */
      SetWindowLong(GetDlgItem(hWnd,IDC_STATUSBAR), GWL_STYLE, style | WS_VISIBLE);
      GetWindowRect(GetDlgItem(hWnd,IDC_STATUSBAR),&rect);  /* get after its visible */
      pWnd->statusBar = 1;
    }
    /* now force a WM_PAINT message in the parent window */
    ScreenToClient(hWnd,(LPPOINT)&rect.left);  /* rect is in screen coords */
    ScreenToClient(hWnd,(LPPOINT)&rect.right);  /* convert to window coords */
    SendMessage(hWnd,WM_USER_RESIZE,0,0);  /* recompute client clipping */
    InvalidateRect(hWnd,&rect,FALSE);    /* invalidate the rect */
    break;
  }

  case ID_DEBUG_GO:
    if (hws->environment.paused) {
      GDBG_INFO(7,"ID_DEBUG_GO 0x%x\n",hWnd);
      hws->environment.paused = 0;
      hwcApplyToAllWindows(hws, updateStatusBar);

      if (hwndCsimMainControl) {
        SendMessage(hwndCsimMainControl,WM_INITDIALOG,0,0);
        UpdateWindow(hwndCsimMainControl);
      }
    }
    break;

  case ID_DEBUG_BREAK:
    if (!hws->environment.paused) {
      GDBG_INFO(7,"ID_DEBUG_BREAK 0x%x\n",hWnd);
      hwcGUIDoBreak(hws);
    }
    break;

  case ID_DEBUG_CSIM:
    hwcInfo.debug = FXTRUE;
    break;

  case ID_VIEW_PROPERTIES:
    if (pWnd->hWndDisplaySettings == NULL) {
      char buf[80];
      pWnd->hWndDisplaySettings = 
      CreateDialog((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE),
          MAKEINTRESOURCE(IDD_HWC_VIEW),
          hWnd,
          csimPropsProc);
/*    hwcGDICreateSpin(pWnd->hWndDisplaySettings,IDC_UPDOWN1, IDC_EDIT1, 0,0x7FFF,pWnd->base); */
/*    hwcGDICreateSpin(pWnd->hWndDisplaySettings,IDC_UPDOWN2, IDC_EDIT2, 0,0x7FFF,pWnd->stride); */
      sprintf(buf,"0x%x",pWnd->buffer->physicalBase);
      SetDlgItemText((HWND) pWnd->hWndDisplaySettings,IDC_EDIT1,buf);
      sprintf(buf,"0x%x",pWnd->buffer->stride);
      SetDlgItemText((HWND) pWnd->hWndDisplaySettings,IDC_EDIT2,buf);
      hwcGDICreateSpin((HWND) pWnd->hWndDisplaySettings,IDC_UPDOWN3, IDC_EDIT3, 1,16,pWnd->zoom);
    }
    else SetActiveWindow((HWND) pWnd->hWndDisplaySettings);
      break;

  case ID_WINDOW_NEWWINDOW:
    GDBG_INFO(7,"ID_WINDOW_NEWWINDOW 0x%x\n",hWnd);
    hwcNewSimViewWindow( hws, hws->hwc->stdBuffers[HWC_BUF_3D_RENDER]);
    break;

  case ID_NEW_MSWIN:
     GDBG_INFO(7,"ID_WINDOW_NEWWINDOW 0x%x\n",hWnd);
     hwcNewSimViewWindow( hws, hws->hwc->stdBuffers[HWC_BUF_MS]);
     break;

  case ID_WINDOW_FIT:
  {
    int bpp;
    RECT rect,rect1;

    GetWindowRect(hWnd,&rect);
    rect.right -= rect.left;    /* compute width and height */
    rect.bottom -= rect.top;
    GetClientRect(hWnd,&rect1);    /* get client rectangle */
    GDBG_INFO(7,"ID_WINDOW_FIT 0x%x  w=%d h=%d\n",hWnd,rect1.right,rect1.bottom);
    switch (pWnd->buffer->format) {
    case HWC_PIXFMT_P_8:  bpp = 1; break;
    case HWC_PIXFMT_ARGB_1555: bpp = 2; break;
    case HWC_PIXFMT_RGB_565:  bpp = 2; break;
    case HWC_PIXFMT_RGB_888:  bpp = 3; break;
    case HWC_PIXFMT_ARGB_8888:  bpp = 4; break;
    }
    rect.right += pWnd->buffer->stride/bpp - rect1.right;
    MoveWindow(hWnd,rect.left,rect.top,rect.right,rect.bottom,1);
    break;
  }

  case ID_WINDOW_REFRESH:
    {
      const char *fmt;
      GDBG_INFO(7,"ID_WINDOW_REFRESH 0x%x\n",hWnd);
      SendMessage(hWnd, WM_USER_RESTAT, 0,0);
      doMenuSelect((HWND) pWnd->hWnd,NULL,0,0xFFFF);
      hWnd = GetDlgItem(hWnd,IDC_STATUSBAR);
      fmt = hwcPixelFormatToString(pWnd->buffer->format);
      SendMessage(hWnd, SB_SETTEXT, 2, (LPARAM)fmt); 
      hwcRefreshWindow(pWnd);
    }
    break;

  case ID_WINDOW_CLOSE:
    GDBG_INFO(7,"ID_WINDOW_CLOSE 0x%x view\n",hWnd);
    SendMessage(hWnd,WM_CLOSE,0,0);
    break;

  case ID_WINDOW_CLOSE_ALL:
    GDBG_INFO(7,"ID_WINDOW_CLOSE ALL 0x%x\n",hWnd);
    hwcDeleteAllWindows(hws);
    break;

  case ID_HELP_CONTENTS:
    GDBG_INFO(7,"ID_HELP_CONTENTS\n");
    break;

  case ID_HELP_ABOUT:
    GDBG_INFO(7,"ID_HELP_ABOUT view\n");
    DialogBox((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE),
              MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd,
              hwcGDIAboutBoxDialogProc);
    break;
  default:
    return(DefWindowProc(hWnd, msg, wParam, lParam));
  }
    return (0);
}

/*---------------------------------------------------------------------------
   the viewer window proc
  ---------------------------------------------------------------------------*/
LRESULT CALLBACK csimViewWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  HwcWindow *pWnd = (HwcWindow *)GetWindowLong(hWnd,GWL_USERDATA);
  HwcSimulator *hws;
  HMENU hmenu = GetMenu(hWnd);

  if ( pWnd )
	  hws = pWnd->hws;

  switch (msg) {
  case WM_CREATE:
  {
    // static int lpParts[] = {225,280,330,450,500};
    static int lpParts[] = {225,300,350,470,520};

    GDBG_INFO(7,"WM_CREATE view: 0x%x\n",hWnd);
    /* create the status bar for the viewer window */
          CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_BORDER,
      "Ready", hWnd, IDC_STATUSBAR);
    SendMessage(GetDlgItem(hWnd,IDC_STATUSBAR), SB_SETPARTS,
    (WPARAM)sizeof(lpParts)/sizeof(*lpParts), (LPARAM)lpParts);
      
    break;
  }

  case WM_INITMENUPOPUP:
  {
    FxI32 i;
    GDBG_INFO(7,"WM_INITMENUPOPUP: 0x%x %d\n",hWnd, wParam);

    ASSERT(pWnd != NULL);
    for (i=1; i<=16; i++)
      CheckMenuItem(hmenu, i+ID_VIEW_ZOOM_11-1, MF_BYCOMMAND | 
                    (pWnd->zoom==i ? MF_CHECKED:MF_UNCHECKED));

    CheckMenuItem(hmenu, ID_VIEW_HEXADECIMAL, MF_BYCOMMAND |
    (pWnd->hex ? MF_CHECKED:MF_UNCHECKED));
    CheckMenuItem(hmenu, ID_VIEW_STATUSBAR, MF_BYCOMMAND |
    (pWnd->statusBar ? MF_CHECKED:MF_UNCHECKED));
    CheckMenuItem(hmenu, ID_VIEW_FILTER_NONE, MF_BYCOMMAND|MF_UNCHECKED);
    CheckMenuItem(hmenu, ID_VIEW_FILTER_EXPAND, MF_BYCOMMAND|MF_UNCHECKED);
    CheckMenuItem(hmenu, ID_VIEW_FILTER_2X2, MF_BYCOMMAND|MF_UNCHECKED);
    CheckMenuItem(hmenu, ID_VIEW_FILTER_4X1, MF_BYCOMMAND|MF_UNCHECKED);
    CheckMenuItem(hmenu, ID_VIEW_FILTER_NONE+pWnd->videoFilter, MF_BYCOMMAND|MF_CHECKED);

    EnableMenuItem(hmenu, ID_DEBUG_GO, MF_BYCOMMAND |
    (hws->environment.paused ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hmenu, ID_DEBUG_BREAK, MF_BYCOMMAND |
    (!hws->environment.paused ? MF_ENABLED : MF_GRAYED));

    break;
  }
  case WM_PAINT:
  {
    RECT rect;
    PAINTSTRUCT ps;
    HDC hDC;

    hDC = BeginPaint(hWnd,&ps);
    if (ps.fErase) {
      GetUpdateRect(hWnd,&rect,FALSE);
      FillRect((HDC) hDC, &rect, (HBRUSH) GetStockObject(GRAY_BRUSH));
    } else {
      GDBG_INFO(31,"WM_PAINT view: 0x%x   %d %d %d %d\n",
      hWnd, ps.rcPaint.left,ps.rcPaint.top, ps.rcPaint.right,ps.rcPaint.bottom);
      
      if (pWnd && pWnd->buffer) {
          hwcGDIRepaintWindow(pWnd, hDC, (HwcRect *)&ps.rcPaint);
      }
    }
    EndPaint(hWnd,&ps);
    break;
  }

  case WM_MBUTTONDOWN:
  case WM_LBUTTONDOWN:
  case WM_RBUTTONDOWN:

  case WM_MBUTTONUP:
  case WM_LBUTTONUP:
  case WM_RBUTTONUP:
    GDBG_INFO(31,"mouse event %x\n",msg);
    break;

  case WM_MOUSEMOVE:
    if (pWnd && pWnd->buffer) { /* GMT: sometimes we get these events after the window is gone */
      char szBuf[256];
      int xOff = GetScrollPos((HWND) pWnd->hWnd, SB_HORZ);
      int yOff = GetScrollPos((HWND) pWnd->hWnd, SB_VERT);

      ASSERT(pWnd != NULL);
      pWnd->mx = xOff+SIGN_EXTEND(LOWORD(lParam),16); /* save current mouse x,y */
      pWnd->my = yOff+SIGN_EXTEND(HIWORD(lParam),16);
      if (pWnd->mx < 0) pWnd->mx = 0;
      if (pWnd->my < 0) pWnd->my = 0;
      // if ((FxU32)pWnd->mx > pWnd->buffer->width) pWnd->mx = pWnd->buffer->width;
      // if ((FxU32)pWnd->my > pWnd->buffer->height) pWnd->my = pWnd->buffer->height;
      /* update the status bar text (x,y) as well as RESTAT the pixel */
      sprintf(szBuf, " (%d,%d)", pWnd->mx, pWnd->my);
      SendMessage(GetDlgItem(hWnd,IDC_STATUSBAR), SB_SETTEXT, 1, (LPARAM)szBuf);
      SendMessage(hWnd, WM_USER_RESTAT, 0,0);  /* update pixel display */
    }
    break;

  case WM_KEYDOWN:
    if (pWnd && pWnd->buffer) {
        int dMouse = 1;
        POINT p,pc;
        RECT rect;
        int cap = GetSystemMetrics(SM_CYCAPTION);
        int xOff = GetScrollPos((HWND) pWnd->hWnd, SB_HORZ);
        int yOff = GetScrollPos((HWND) pWnd->hWnd, SB_VERT);

        GetClientRect ((HWND) pWnd->hWnd, &rect);
        GetCursorPos(&p);
        if (GetKeyState(VK_SHIFT) < 0) dMouse = 10;
        if (GetKeyState(VK_CONTROL) < 0) dMouse = 100000;
        switch(wParam) {
        case VK_LEFT:  p.x-=dMouse; goto clipcursor;
        case VK_RIGHT:  p.x+=dMouse; goto clipcursor; 
        case VK_UP:  p.y-=dMouse; goto clipcursor;
        case VK_DOWN:  p.y+=dMouse; goto clipcursor;
        }
        break;
        clipcursor:
          pc.x = pc.y = 0;
          ClientToScreen(hWnd,&pc);
          if (p.x < pc.x) p.x = pc.x;
          if (p.y < pc.y) p.y = pc.y;
          pc.x = MIN(rect.right-1, (int)pWnd->buffer->width-1-xOff);
          pc.y = MIN(rect.bottom-1-cap, (int)pWnd->buffer->height-1-yOff);
          ClientToScreen(hWnd,&pc);
          if (p.x > pc.x) p.x = pc.x;
          if (p.y > pc.y) p.y = pc.y;
          SetCursorPos(p.x,p.y);
      }
      break;

  case WM_MENUSELECT:
    if (pWnd && pWnd->buffer) {
        doMenuSelect(hWnd, (HMENU)lParam, (UINT)LOWORD(wParam), (UINT)HIWORD(wParam));
    }
    break;

  case WM_MOVE:
    if (pWnd && pWnd->buffer) {
      hwcWriteWindowPlacement( hwcWindowName(pWnd->hws, pWnd->buffer->name),(HWND)  pWnd->hWnd );
    }
    break;
    
  case WM_SIZE:
    GDBG_INFO(7,"WM_SIZE csim view 0x%x\n",hWnd);
    if (GetDlgItem(hWnd,IDC_STATUSBAR))    /* if there's a status bar send it a message */
    SendMessage(GetDlgItem(hWnd,IDC_STATUSBAR),WM_SIZE,wParam,lParam);
    SendMessage(hWnd,WM_USER_RESIZE,0,0);  /* recompute client clipping */
    if (pWnd && pWnd->buffer) {
      hwcGDIUpdateScroll(pWnd, FXFALSE);
      hwcWriteWindowPlacement( hwcWindowName(pWnd->hws, pWnd->buffer->name), (HWND) pWnd->hWnd );
    }
    break;

  case WM_USER_RESIZE:
    // XXX dont support buffer resize since display gets all messed up
    // since its based on fixed stride and x, y which don't change
    break;
    if (pWnd && pWnd->buffer) {
      RECT rect;
      GetClientRect(hWnd,&rect);
      pWnd->buffer->width = rect.right;
      pWnd->buffer->height = rect.bottom;
      if (pWnd->statusBar) pWnd->buffer->height -= GetSystemMetrics(SM_CYCAPTION);
      GDBG_INFO(31,"WM_SIZE csim view 0x%x, sbar=%d  w,h = %d,%d\n",
                hWnd,pWnd->statusBar, pWnd->buffer->width,pWnd->buffer->height);
      // pWnd->buffer->endBase = pWnd->buffer->base + (pWnd->buffer->height+1)*pWnd->buffer->stride;
      }
      break;

  case WM_USER_RESTAT:
    if (pWnd && pWnd->buffer) {
      char szBuf[256];
      HwcPixel pix;
      FxU8 a,r,g,b;
      FxU32 hex=pWnd->hex;
      FxU32 x = pWnd->mx, y = pWnd->my;

      // just in case!
      if (x >= pWnd->buffer->width)
        x = pWnd->buffer->width-1;
      if (y >= pWnd->buffer->height)
        y = pWnd->buffer->height-1;
      /* check the VSD for the window and reformat the text */
      pix = hwcReadPixel(hws, pWnd->buffer, x, y);
      hwcReadColor(pWnd->hws, pWnd->buffer, x, y, &a, &r, &g, &b);
      sprintf(szBuf, hex ? " 0x%08x %02x %02x %02x" :
                           " 0x%08x %3d %3d %3d", pix, r, g, b);
      /* send the text to the status bar */
      SendMessage(GetDlgItem(hWnd,IDC_STATUSBAR), SB_SETTEXT, 3, (LPARAM)szBuf);
      break;
    }

  case WM_COMMAND:
    if (pWnd && pWnd->buffer) {
        return doCommand(hWnd, msg, wParam, lParam);
    }
    break;

  case WM_SYSCOLORCHANGE:      /* CTL3D hooks */
    break;

    /**********************************************************************\
    *  WM_HSCROLL
    *
    * Slide the contents of the window left and right.  Notice that the
    *  scroll bar thumb position is important for painting.
    \**********************************************************************/
  case WM_HSCROLL:
    {
      HWND hWndChild = GetDlgItem(hWnd, IDC_STATUSBAR);
      // WINDOWPLACEMENT wPlace;
      RECT rect;
      int OldPos, NewPos, xAmount = 0, yAmount = 0;

      GetClientRect (hWnd, &rect);
      OldPos = GetScrollPos (hWnd, SB_HORZ);

      switch (LOWORD(wParam)){
      case SB_LINEDOWN:
        SetScrollPos (hWnd, SB_HORZ, OldPos+1, TRUE);
        NewPos = GetScrollPos (hWnd, SB_HORZ);
        break;

      case SB_PAGEDOWN:
        SetScrollPos (hWnd, SB_HORZ, OldPos+(rect.right-1), TRUE);
        NewPos = GetScrollPos (hWnd, SB_HORZ);
        break;

      case SB_LINEUP:
        SetScrollPos (hWnd, SB_HORZ, OldPos-1, TRUE);
        NewPos = GetScrollPos (hWnd, SB_HORZ);
        break;

      case SB_PAGEUP:
        SetScrollPos (hWnd, SB_HORZ, OldPos-(rect.right-1), TRUE);
        NewPos = GetScrollPos (hWnd, SB_HORZ);
        break;

      case SB_THUMBTRACK:
      case SB_THUMBPOSITION:
        NewPos = HIWORD (wParam);
        SetScrollPos (hWnd, SB_HORZ, NewPos, TRUE);
        break;
      }
      hwcGDIUpdateScroll(pWnd, FXFALSE);
    }
    break;

    /**********************************************************************\
    *  WM_VSCROLL
    *
    * Slide the contents of the window up and down.  Notice that the
    *  scroll bar thumb position is important for painting.
    \**********************************************************************/
  case WM_VSCROLL:
    {
      HWND hWndChild = GetDlgItem(hWnd, IDC_STATUSBAR);
      // WINDOWPLACEMENT wPlace;
      RECT rect;
      int OldPos, NewPos, xAmount = 0, yAmount = 0;

      GetClientRect (hWnd, &rect);
      OldPos = GetScrollPos (hWnd, SB_VERT);

      switch (LOWORD(wParam)){
      case SB_LINEDOWN:
        SetScrollPos (hWnd, SB_VERT, OldPos+1, TRUE);
        NewPos = GetScrollPos (hWnd, SB_VERT);
        break;

      case SB_PAGEDOWN:
        SetScrollPos (hWnd, SB_VERT, OldPos+(rect.bottom-1), TRUE);
        NewPos = GetScrollPos (hWnd, SB_VERT);
        break;

      case SB_LINEUP:
        SetScrollPos (hWnd, SB_VERT, OldPos-1, TRUE);
        NewPos = GetScrollPos (hWnd, SB_VERT);
        break;

      case SB_PAGEUP:
        SetScrollPos (hWnd, SB_VERT, OldPos-(rect.bottom-1), TRUE);
        NewPos = GetScrollPos (hWnd, SB_VERT);
        break;

      case SB_THUMBTRACK:
      case SB_THUMBPOSITION:
        NewPos = HIWORD (wParam);
        SetScrollPos (hWnd, SB_VERT, NewPos, TRUE);
        break;

      }

      hwcGDIUpdateScroll(pWnd, FXFALSE);
    }
    break;

  case WM_CLOSE:        /* close the window */
    GDBG_INFO(7,"WM_CLOSE csim view 0x%x\n",hWnd);
    if (pWnd->hWndDisplaySettings)
    DestroyWindow((HWND) pWnd->hWndDisplaySettings);
    DestroyWindow((HWND) pWnd->hWnd);
    hwcDeleteWindow(pWnd);
    break;

  default:
    return(DefWindowProc(hWnd, msg, wParam, lParam));
  }
  return(0);
}

#endif
