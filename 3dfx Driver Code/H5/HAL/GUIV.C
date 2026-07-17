/*-*-c++-*-*/
#include "vxd.h"
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
** $Revision: 2$
** $Date: 10/11/00 8:31:06 PM$
*/

#ifdef GUI

#include <windows.h>
#include <commctrl.h>

#include <stdio.h>
#include <h3.h>

#ifdef HAL_CSIM
#include "../csim/csim.h"
#endif

#include "gui.h"
#include "guimv.h"
#include <afxres.h>
#include <fximg.h>

#ifdef CVG
#define SSTG_DST_FORMAT SSTG_SRC_FORMAT
#define SSTG_PIXFMT_8BPP 0xdeadbeef
#define SSTG_PIXFMT_32BPP 0xdeadbaaf
#endif

#define SIGN_EXTEND(x,nbits) ((((signed)(x))<<(32-(nbits)))>>(32-(nbits)))

// XXX this goes somewhere else and really should do something!!!
static void fileSave(CsimWindow *cw)
{
    GDBG_INFO(30,"save as %s \n",cw->fileSaveName);
    if (!csimPicSave(cw->cp, cw->fileSaveName,0,cw->width,cw->height,IMG_UNKNOWN))
	MessageBox(NULL,
		"File Save Failed, see console output for more details\n"
		"The probable cause is an invalid file extension.\n"
		"Valid file extensions are .ppm, .sbi, or .tga.\n",
		"Csim",
		MB_SETFOREGROUND | MB_TASKMODAL | MB_OK | MB_ICONSTOP);
}

//---------------------------------------------------------------------------
// common dialog control for doing a Save As
//---------------------------------------------------------------------------
static void doSaveAs(HWND hWnd, CsimPrivate *cp)
{
    static OPENFILENAME ofn;
    char szFileTitle[256];
    char szFilter[] =	"PPM files\0*.ppm\0"
			"Targa files\0*.tga\0"
			"SBI files\0*.sbi\0"
			"All files\0*.*\0";
    CsimWindow *cwFound = guiFindCsimWindow(cp,hWnd);

    ASSERT(cwFound != NULL);
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = szFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = cwFound->fileSaveName;
    ofn.nMaxFile = sizeof(cwFound->fileSaveName);
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = sizeof(szFileTitle);
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn)) {
	strcpy(cwFound->fileSaveName,ofn.lpstrFile);
	fileSave(cwFound);
    }
    else {
	printf("save as CANCELLED: error code = %d 0x%x\n",
		CommDlgExtendedError(), CommDlgExtendedError());
    }
}

//---------------------------------------------------------------------------
// the properties/control dialog window proc
//---------------------------------------------------------------------------
static
BOOL CALLBACK csimPropsProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    char buf[80];
    HWND hWnd = GetParent(hwndDlg);
    CsimPrivate *cp = CSIM_PRIVATE(halInfo.boardInfo[GetWindowLong(hWnd,GWL_USERDATA)].sstCSIM);
    CsimWindow *cwFound = guiFindCsimWindow(cp,hWnd);

    ASSERT(cwFound != NULL);
    switch(uMsg) {
	case WM_INITDIALOG:
	{
	    int temp;
	    CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO4,IDC_RADIO1+cwFound->videoFilter);
	    switch(cwFound->pixFormat) {
		case SSTG_PIXFMT_8BPP:	temp = IDC_RADIO5;	break;
		case SSTG_PIXFMT_16BPP:	temp = IDC_RADIO6;	break;
		case SSTG_PIXFMT_24BPP:	temp = IDC_RADIO7;	break;
		case SSTG_PIXFMT_32BPP:	temp = IDC_RADIO8;	break;
		case SSTG_PIXFMT_15BPP:	temp = IDC_RADIO9;	break;
	    }
	    CheckRadioButton(hwndDlg,IDC_RADIO5,IDC_RADIO9,temp);
	    return TRUE;
	}
#if 0
	case WM_ACTIVATE:
	    GDBG_INFO(30,"csim props dialog: WM_ACTIVATE\n");
	    hwndCsimProps = hwndDlg;
	    return TRUE;
#endif
	case WM_COMMAND:
	    ASSERT( hwndDlg == cwFound->hWndDisplaySettings );
	    GDBG_INFO(30,"csim props dialog: WM_COMMAND: %d %d %d\n",LOWORD(wParam),wParam,lParam);
	    switch(LOWORD(wParam)) {
		case IDC_RADIO1:
		case IDC_RADIO2:
		case IDC_RADIO3:
		case IDC_RADIO4:
			cwFound->videoFilter = LOWORD(wParam)-IDC_RADIO1;
		refresh:
			SendMessage(hWnd,WM_COMMAND,ID_WINDOW_REFRESH,0);
			return TRUE;

		case IDC_RADIO5:
			cwFound->pixFormat = SSTG_PIXFMT_8BPP;
			goto refresh;
		case IDC_RADIO6:
			cwFound->pixFormat = SSTG_PIXFMT_16BPP;
			goto refresh;
		case IDC_RADIO7:
			cwFound->pixFormat = SSTG_PIXFMT_24BPP;
			goto refresh;
		case IDC_RADIO8:
			cwFound->pixFormat = SSTG_PIXFMT_32BPP;
			goto refresh;
		case IDC_RADIO9:
			cwFound->pixFormat = SSTG_PIXFMT_15BPP;
			goto refresh;

		case IDC_EDIT1:
			if (HIWORD(wParam) == EN_KILLFOCUS) {
			    GetDlgItemText(hwndDlg,IDC_EDIT1,buf,sizeof(buf));
			    sscanf(buf,"%i",&cwFound->base);
			    cwFound->base &= ~15;
			    sprintf(buf,"0x%x",cwFound->base);
			    SetDlgItemText(hwndDlg,IDC_EDIT1,buf);
			    goto refresh;
			}
			break;
		case IDC_EDIT2:
			if (HIWORD(wParam) == EN_KILLFOCUS) {
			    GetDlgItemText(hwndDlg,IDC_EDIT2,buf,sizeof(buf));
			    sscanf(buf,"%i",&cwFound->stride);
			    cwFound->stride &= ~15;
			    sprintf(buf,"0x%x",cwFound->stride);
			    SetDlgItemText(hwndDlg,IDC_EDIT2,buf);
			    goto refresh;
			}
			break;
		CASE_UPDOWN(hwndDlg,IDC_EDIT3,IDC_UPDOWN3,cwFound->zoom,
			SendMessage(GetParent(hwndDlg),WM_COMMAND,ID_VIEW_ZOOM_11-1+cwFound->zoom,0));
#ifdef CVG
 		case IDC_BUTTON_FRONT:
			cwFound->stride = 1;
			if (cp->info->sstCSIM->status & SST_DISPLAYED_BUFFER) {
			    cwFound->base = csimPixelAddress(cp->info->sstCSIM,CSIM_BUF_3D_BACK,0,0);
			    cwFound->endBase = csimPixelAddress(cp->info->sstCSIM,CSIM_BUF_3D_FRONT,0,0);
			}
			else {
			    cwFound->base = csimPixelAddress(cp->info->sstCSIM,CSIM_BUF_3D_FRONT,0,0);
			    cwFound->endBase = csimPixelAddress(cp->info->sstCSIM,CSIM_BUF_3D_BACK,0,0);
			}
			goto changed_buffer;
 		case IDC_BUTTON_BACK:
			cwFound->stride = 2;
			if (cp->info->sstCSIM->status & SST_DISPLAYED_BUFFER) {
			    cwFound->base = csimPixelAddress(cp->info->sstCSIM,CSIM_BUF_3D_FRONT,0,0);
			}
			else {
			    cwFound->endBase = csimPixelAddress(cp->info->sstCSIM,CSIM_BUF_3D_AUX1,0,0);
			}
			cwFound->endBase = csimPixelAddress(cp->info->sstCSIM,CSIM_BUF_3D_AUX1,0,0);
			goto changed_buffer;
 		case IDC_BUTTON_AUX:
			cwFound->base = csimPixelAddress(cp->info->sstCSIM,CSIM_BUF_3D_AUX1,0,0);
			cwFound->stride = 3;
			cwFound->endBase = cwFound->base + csimPixelAddress(cp->info->sstCSIM,CSIM_BUF_3D_BACK,0,0);
			goto changed_buffer;
 		case IDC_BUTTON_GUI:
			cwFound->base = cp->info->sstCSIM->bltDstBaseAddr;
			cwFound->stride = cp->info->sstCSIM->bltXYstrides >> 16;
			goto changed_buffer;
#else
 		case IDC_BUTTON_FRONT:
			wParam = cp->bufferAddrHack[0] == (int)halInfo.boardInfo[GetWindowLong(hWnd,GWL_USERDATA)].sstCSIM->colBufferAddr ? 1:0;
			cwFound->base = cp->bufferAddrHack[wParam];		// diags set this!
			cwFound->stride = cp->bufferStrideHack[wParam];	// diags set this!
			goto changed_buffer;
 		case IDC_BUTTON_BACK:
			cwFound->base = halInfo.boardInfo[GetWindowLong(hWnd,GWL_USERDATA)].sstCSIM->colBufferAddr;
			cwFound->stride = halInfo.boardInfo[GetWindowLong(hWnd,GWL_USERDATA)].sstCSIM->colBufferStride;
			goto changed_buffer;
 		case IDC_BUTTON_AUX:
			cwFound->base = halInfo.boardInfo[GetWindowLong(hWnd,GWL_USERDATA)].sstCSIM->auxBufferAddr;
			cwFound->stride = halInfo.boardInfo[GetWindowLong(hWnd,GWL_USERDATA)].sstCSIM->auxBufferStride;
			goto changed_buffer;
 		case IDC_BUTTON_GUI:
			cwFound->base = (cp->gui.dstBaseAddr & SSTG_BASEADDR) >> SSTG_BASEADDR_SHIFT;
			cwFound->stride = (cp->gui.dstFormat & SSTG_DST_LINEAR_STRIDE) >> SSTG_DST_STRIDE_SHIFT;
#endif

			// recompute endBase and update dialog boxes
		changed_buffer:
#ifndef CVG
			cwFound->endBase = cwFound->base + (cwFound->height+1)*cwFound->stride;
#endif
			sprintf(buf,"0x%x",cwFound->base);
			SetDlgItemText(hwndDlg,IDC_EDIT1,buf);
			sprintf(buf,"0x%x",cwFound->stride);
			SetDlgItemText(hwndDlg,IDC_EDIT2,buf);
			SendMessage(hWnd,WM_COMMAND,ID_WINDOW_REFRESH,0);
			return TRUE;
 		case IDCANCEL:
		    DestroyWindow(hwndDlg);
		    cwFound->hWndDisplaySettings = NULL;
		    return TRUE;
	    }
	    break;
    }
    return FALSE;
}

//---------------------------------------------------------------------------
// process a WM_MENUSELECT message - this figures out what is going on in the
//	popup menus and updates the status bar text accordingly
//---------------------------------------------------------------------------
static void doMenuSelect(HWND hWnd, HMENU hMenu, UINT uItem, UINT flags)
{
    char szStatBarText[256];
    UINT nID;

    GDBG_INFO(30,"doMenuSelect(*,0x%x,%d,0x%x)\n",hMenu,uItem,flags);
    if (hMenu==NULL && flags==0xFFFF) {		// Exiting menu mode
	CsimPrivate *cp = CSIM_PRIVATE(halInfo.boardInfo[GetWindowLong(hWnd,GWL_USERDATA)].sstCSIM);
	nID = cp->environment.paused ? IDS_PAUSED : IDS_READY;
    }
    else if (flags & MF_SYSMENU) {		// System menu is up
	if (flags & MF_POPUP)			// System menu item is selected
	    nID = IDS_SYSMENU;			// System menu and no item is selected
	else
	    nID = uItem;			// uItem contains SC_* code.
    }
    else {
        if ((flags & MF_POPUP)) {		// pulling down a popup submenu
	    if ( uItem == 0 )			// sort of crude - use popup index
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

static void resetStatusBppText(CsimWindow *cw)
{
    HWND hWnd = GetDlgItem(cw->hWnd,IDC_STATUSBAR);
    switch (cw->pixFormat) {
	case SSTG_PIXFMT_8BPP:	SendMessage(hWnd, SB_SETTEXT, 2, (LPARAM)" 8 bpp"); break;
	case SSTG_PIXFMT_15BPP:	SendMessage(hWnd, SB_SETTEXT, 2, (LPARAM)" 15 bpp"); break;
	case SSTG_PIXFMT_16BPP:	SendMessage(hWnd, SB_SETTEXT, 2, (LPARAM)" 16 bpp"); break;
	case SSTG_PIXFMT_24BPP:	SendMessage(hWnd, SB_SETTEXT, 2, (LPARAM)" 24 bpp"); break;
	case SSTG_PIXFMT_32BPP:	SendMessage(hWnd, SB_SETTEXT, 2, (LPARAM)" 32 bpp"); break;
    }
}

static void updateStatusBar(CsimWindow *cw)
{
    doMenuSelect(cw->hWnd,NULL,0,0xFFFF);
    SendMessage(GetDlgItem(cw->hWnd,IDC_STATUSBAR), WM_PAINT, 0, 0);
}

FX_EXPORT void FX_CSTYLE guiUpdateStatusBar(void *cw)
{
    resetStatusBppText((CsimWindow *)cw);
    updateStatusBar(cw);
}

void guiDoBreak(CsimPrivate *cp)
{
	cp->environment.paused = 1;
	guiApplyToAllWindows(cp, updateStatusBar);
	if (hwndCsimMainControl) {
	    guiUpdateDynamicText(hwndCsimMainControl,cp);
	    SendMessage(hwndCsimMainControl,WM_INITDIALOG,0,0);
	    UpdateWindow(hwndCsimMainControl);
	    KillTimer(hwndCsimMainControl,WM_TIMER_ID);
	}
	while (cp->environment.paused) {
	    WaitMessage();
	    guiReadMessageQueue();
	}
	if (hwndCsimMainControl) {
	    SetTimer(hwndCsimMainControl,WM_TIMER_ID,WM_TIMER_MSECS,NULL);
	}
}

static LRESULT doCommand (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CsimPrivate *cp = CSIM_PRIVATE(halInfo.boardInfo[GetWindowLong(hWnd,GWL_USERDATA)].sstCSIM);
    CsimWindow *cwFound = guiFindCsimWindow(cp,hWnd);
    HMENU hmenu = GetMenu(hWnd);

    ASSERT(cwFound != NULL);
    switch(LOWORD(wParam)) {
	case ID_FILE_SAVE:
	{
	    GDBG_INFO(30,"ID_FILE_SAVE view\n");
	    if (cwFound->fileSaveName[0])	// if already has a name
		fileSave(cwFound);		// then use it
	    else				// else do a save as
		SendMessage(hWnd, WM_COMMAND, ID_FILE_SAVEAS, 0);
	    break;
	}

	case ID_FILE_SAVEAS:
	    GDBG_INFO(30,"ID_FILE_SAVEAS view\n");
	    doSaveAs( hWnd, cp );
	    break;

	case ID_FILE_EXIT:
	    GDBG_INFO(30,"ID_FILE_EXIT view\n");
	    if (IDYES ==MessageBox(NULL,
			"Are you sure you want to exit?", "Csim",
			MB_SETFOREGROUND | MB_TASKMODAL | MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
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
	    cptr = strstr(buf,CSIM_VIEW_WINDOW_NAME);
	    if (cptr) {
		if (z > 1)
		    sprintf(cptr,CSIM_VIEW_WINDOW_NAME " (%d:1)",z);
		else cptr[strlen(CSIM_VIEW_WINDOW_NAME)] = '\0';
		SetWindowText(hWnd,buf);
	    }
	    if (cwFound) cwFound->zoom = z;
	    GDBG_INFO(30,"ID_VIEW_ZOOM %d title:%s\n",z,buf);
	    if (cwFound->hWndDisplaySettings)
		SendMessage(cwFound->hWndDisplaySettings,WM_COMMAND, MAKEWPARAM(IDC_EDIT3,EN_KILLFOCUS),0);
	    break;
	}

	case ID_VIEW_FILTER_NONE:	// MENU commands
	case ID_VIEW_FILTER_EXPAND:
	case ID_VIEW_FILTER_2X2:
	case ID_VIEW_FILTER_4X1:
	    GDBG_INFO(30,"ID_VIEW_FILTER 0x%x\n",hWnd);
	    {
		cwFound->videoFilter = LOWORD(wParam) - ID_VIEW_FILTER_NONE;
		if (cwFound->hWndDisplaySettings) {
		    CheckRadioButton(cwFound->hWndDisplaySettings,
					IDC_RADIO1,IDC_RADIO4,
					IDC_RADIO1+LOWORD(wParam)-ID_VIEW_FILTER_NONE);
		}
		// now refresh this window since it will be different
		SendMessage(hWnd,WM_COMMAND,ID_WINDOW_REFRESH,0);
		break;
	    }

	case ID_VIEW_HEXADECIMAL:
	    cwFound->hex ^= 1;
	    SendMessage(GetDlgItem(hWnd,IDC_STATUSBAR), SB_SETTEXT, 4, 
				(LPARAM)(cwFound->hex ? " hex" : " dec"));
	    break;

	case ID_VIEW_STATUSBAR:
	{
	    LONG style;
	    RECT rect;

	    GDBG_INFO(30,"ID_VIEW_STATUSBAR\n");
	    style = GetWindowLong(GetDlgItem(hWnd,IDC_STATUSBAR),GWL_STYLE);
	    // if visible, then make invisible
	    if (cwFound->statusBar) {
		GetWindowRect(GetDlgItem(hWnd,IDC_STATUSBAR),&rect);	// get this before making invisible
		SetWindowLong(GetDlgItem(hWnd,IDC_STATUSBAR), GWL_STYLE, style & ~WS_VISIBLE);
		cwFound->statusBar = 0;
	    }
	    else {	// if invisible, then make visible
		SetWindowLong(GetDlgItem(hWnd,IDC_STATUSBAR), GWL_STYLE, style | WS_VISIBLE);
		GetWindowRect(GetDlgItem(hWnd,IDC_STATUSBAR),&rect);	// get after its visible
		cwFound->statusBar = 1;
	    }
	    // now force a WM_PAINT message in the parent window
	    ScreenToClient(hWnd,(LPPOINT)&rect.left);	// rect is in screen coords
	    ScreenToClient(hWnd,(LPPOINT)&rect.right);	// convert to window coords
	    SendMessage(hWnd,WM_USER_RESIZE,0,0);	// recompute client clipping
	    InvalidateRect(hWnd,&rect,FALSE);		// invalidate the rect
	    break;
	}

	case ID_DEBUG_GO:
	    if (cp->environment.paused) {
		GDBG_INFO(30,"ID_DEBUG_GO 0x%x\n",hWnd);
		cp->environment.paused = 0;
		guiApplyToAllWindows(cp, updateStatusBar);

		if (hwndCsimMainControl) {
		    SendMessage(hwndCsimMainControl,WM_INITDIALOG,0,0);
		    UpdateWindow(hwndCsimMainControl);
		}
	    }
	    break;

	case ID_DEBUG_BREAK:
	    if (!cp->environment.paused) {
		GDBG_INFO(30,"ID_DEBUG_BREAK 0x%x\n",hWnd);
		guiDoBreak(cp);
	    }
	    break;

	case ID_VIEW_PROPERTIES:
	    if (cwFound->hWndDisplaySettings == NULL) {
		char buf[80];
		cwFound->hWndDisplaySettings = 
			CreateDialog((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE),
					MAKEINTRESOURCE(IDD_CSIM_VIEW),
					hWnd,
					csimPropsProc);
//		uiCreateSpin(cwFound->hWndDisplaySettings,IDC_UPDOWN1, IDC_EDIT1, 0,0x7FFF,cwFound->base);
//		uiCreateSpin(cwFound->hWndDisplaySettings,IDC_UPDOWN2, IDC_EDIT2, 0,0x7FFF,cwFound->stride);
		sprintf(buf,"0x%x",cwFound->base);
		SetDlgItemText(cwFound->hWndDisplaySettings,IDC_EDIT1,buf);
		sprintf(buf,"0x%x",cwFound->stride);
		SetDlgItemText(cwFound->hWndDisplaySettings,IDC_EDIT2,buf);
		uiCreateSpin(cwFound->hWndDisplaySettings,IDC_UPDOWN3, IDC_EDIT3, 1,16,cwFound->zoom);
	    }
	    else SetActiveWindow(cwFound->hWndDisplaySettings);
	    break;

	case ID_WINDOW_NEWWINDOW:
	    GDBG_INFO(30,"ID_WINDOW_NEWWINDOW 0x%x\n",hWnd);
	    guiNewViewWindow(GetWindowLong(hWnd,GWL_USERDATA),CSIM_VIEW_WINDOW_NAME);
	    break;

	case ID_WINDOW_FIT:
	{
	    int bpp;
	    RECT rect,rect1;

	    GetWindowRect(hWnd,&rect);
	    rect.right -= rect.left;		// compute width and height
	    rect.bottom -= rect.top;
	    GetClientRect(hWnd,&rect1);		// get client rectangle
	    GDBG_INFO(30,"ID_WINDOW_FIT 0x%x  w=%d h=%d\n",hWnd,rect1.right,rect1.bottom);
	    switch (cwFound->pixFormat) {
		case SSTG_PIXFMT_8BPP:	bpp = 1; break;
		case SSTG_PIXFMT_15BPP:	bpp = 2; break;
		case SSTG_PIXFMT_16BPP:	bpp = 2; break;
		case SSTG_PIXFMT_24BPP:	bpp = 3; break;
		case SSTG_PIXFMT_32BPP:	bpp = 4; break;
	    }
	    rect.right += cwFound->stride/bpp - rect1.right;
	    MoveWindow(hWnd,rect.left,rect.top,rect.right,rect.bottom,1);
	    break;
	}

	case ID_WINDOW_REFRESH:
	    GDBG_INFO(30,"ID_WINDOW_REFRESH 0x%x\n",hWnd);
	    SendMessage(hWnd, WM_USER_RESTAT, 0,0);
	    doMenuSelect(cwFound->hWnd,NULL,0,0xFFFF);
	    resetStatusBppText(cwFound);
	    guiRefreshWindow(cwFound);
	    break;

	case ID_WINDOW_CLOSE:
	    GDBG_INFO(30,"ID_WINDOW_CLOSE 0x%x view\n",hWnd);
	    SendMessage(hWnd,WM_CLOSE,0,0);
	    break;

	case ID_WINDOW_CLOSE_ALL:
	    GDBG_INFO(30,"ID_WINDOW_CLOSE ALL 0x%x\n",hWnd);
	    deleteAllCsimWindows(hWnd);
	    break;

	case ID_HELP_CONTENTS:
	    GDBG_INFO(30,"ID_HELP_CONTENTS\n");
	    break;
	case ID_HELP_ABOUT:
	    GDBG_INFO(30,"ID_HELP_ABOUT view\n");
	    DialogBox((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE),
			MAKEINTRESOURCE(IDD_ABOUTBOX),
			hWnd,
			uiAboutBoxDialogProc);
	    break;
	default:
	    return(DefWindowProc(hWnd, msg, wParam, lParam));
    }
    return (0);
}

//---------------------------------------------------------------------------
// the viewer window proc
//---------------------------------------------------------------------------
LRESULT CALLBACK csimViewWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CsimPrivate *cp = CSIM_PRIVATE(halInfo.boardInfo[GetWindowLong(hWnd,GWL_USERDATA)].sstCSIM);
    CsimWindow *cwFound = guiFindCsimWindow(cp,hWnd);
    HMENU hmenu = GetMenu(hWnd);

    switch (msg) {
	case WM_CREATE:
	{
	    static int lpParts[] = {225,280,330,450,500};

	    GDBG_INFO(30,"WM_CREATE view: 0x%x\n",hWnd);
	    // create the status bar for the viewer window
            CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_BORDER,
				"Ready", hWnd, IDC_STATUSBAR);
	    SendMessage(GetDlgItem(hWnd,IDC_STATUSBAR), SB_SETPARTS,
			(WPARAM)sizeof(lpParts)/sizeof(*lpParts), (LPARAM)lpParts);
			
	    break;
	}

	case WM_INITMENUPOPUP:
	{
	    FxI32 i;
	    GDBG_INFO(30,"WM_INITMENUPOPUP: 0x%x %d\n",hWnd, wParam);

	    ASSERT(cwFound != NULL);
	    for (i=1; i<=16; i++)
		CheckMenuItem(hmenu, i+ID_VIEW_ZOOM_11-1, MF_BYCOMMAND | 
				(cwFound->zoom==i ? MF_CHECKED:MF_UNCHECKED));

	    CheckMenuItem(hmenu, ID_VIEW_HEXADECIMAL, MF_BYCOMMAND |
			(cwFound->hex ? MF_CHECKED:MF_UNCHECKED));
	    CheckMenuItem(hmenu, ID_VIEW_STATUSBAR, MF_BYCOMMAND |
			(cwFound->statusBar ? MF_CHECKED:MF_UNCHECKED));
	    CheckMenuItem(hmenu, ID_VIEW_FILTER_NONE, MF_BYCOMMAND|MF_UNCHECKED);
	    CheckMenuItem(hmenu, ID_VIEW_FILTER_EXPAND, MF_BYCOMMAND|MF_UNCHECKED);
	    CheckMenuItem(hmenu, ID_VIEW_FILTER_2X2, MF_BYCOMMAND|MF_UNCHECKED);
	    CheckMenuItem(hmenu, ID_VIEW_FILTER_4X1, MF_BYCOMMAND|MF_UNCHECKED);
	    CheckMenuItem(hmenu, ID_VIEW_FILTER_NONE+cwFound->videoFilter, MF_BYCOMMAND|MF_CHECKED);

	    EnableMenuItem(hmenu, ID_DEBUG_GO, MF_BYCOMMAND |
			(cp->environment.paused ? MF_ENABLED : MF_GRAYED));
	    EnableMenuItem(hmenu, ID_DEBUG_BREAK, MF_BYCOMMAND |
			(!cp->environment.paused ? MF_ENABLED : MF_GRAYED));

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
		FillRect(hDC,&rect,GetStockObject(GRAY_BRUSH));
	    }
	    else {
		GDBG_INFO(31,"WM_PAINT view: 0x%x   %d %d %d %d\n",
			hWnd, ps.rcPaint.left,ps.rcPaint.top, ps.rcPaint.right,ps.rcPaint.bottom);
	    
		if (cwFound) guiRepaint(cwFound,hDC,&ps.rcPaint);
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
	if (cwFound) {		// GMT: sometimes we get these events after the window is gone
	    char szBuf[256];

	    ASSERT(cwFound != NULL);
	    cwFound->mx = SIGN_EXTEND(LOWORD(lParam),16); // save current mouse x,y
	    cwFound->my = SIGN_EXTEND(HIWORD(lParam),16);
	    if (cwFound->mx < 0) cwFound->mx = 0;
	    if (cwFound->my < 0) cwFound->my = 0;
	    if (cwFound->mx > cwFound->width) cwFound->mx = cwFound->width;
	    if (cwFound->my > cwFound->height) cwFound->my = cwFound->height;
	    // update the status bar text (x,y) as well as RESTAT the pixel
	    sprintf(szBuf, " (%d,%d)", cwFound->mx, cwFound->my);
	    SendMessage(GetDlgItem(hWnd,IDC_STATUSBAR), SB_SETTEXT, 1, (LPARAM)szBuf);
	    SendMessage(hWnd, WM_USER_RESTAT, 0,0);	// update pixel display
	    break;
	}

	case WM_KEYDOWN:
	{
	    int dMouse = 1;
	    POINT p,pc;

	    GetCursorPos(&p);
	    if (GetKeyState(VK_SHIFT) < 0) dMouse = 10;
	    if (GetKeyState(VK_CONTROL) < 0) dMouse = 100000;
	    switch(wParam) {
		case VK_LEFT:	p.x-=dMouse; goto clipcursor;
		case VK_RIGHT:	p.x+=dMouse; goto clipcursor; 
		case VK_UP:	p.y-=dMouse; goto clipcursor;
		case VK_DOWN:	p.y+=dMouse; goto clipcursor;
	    }
	    break;
	clipcursor:
	    pc.x = pc.y = 0;
	    ClientToScreen(hWnd,&pc);
	    if (p.x < pc.x) p.x = pc.x;
	    if (p.y < pc.y) p.y = pc.y;
	    pc.x = cwFound->width-1;
	    pc.y = cwFound->height-1;
	    ClientToScreen(hWnd,&pc);
	    if (p.x > pc.x) p.x = pc.x;
	    if (p.y > pc.y) p.y = pc.y;
	    SetCursorPos(p.x,p.y);
	    break;
	}

	case WM_MENUSELECT:
	    doMenuSelect(hWnd, (HMENU)lParam, (UINT)LOWORD(wParam), (UINT)HIWORD(wParam));
	    break;

	case WM_SIZE:
	    GDBG_INFO(30,"WM_SIZE csim view 0x%x\n",hWnd);
	    if (GetDlgItem(hWnd,IDC_STATUSBAR))		// if there's a status bar send it a message
		SendMessage(GetDlgItem(hWnd,IDC_STATUSBAR),WM_SIZE,wParam,lParam);
	    SendMessage(hWnd,WM_USER_RESIZE,0,0);	// recompute client clipping
	    break;

	case WM_USER_RESIZE:
	    if (cwFound) {
		    RECT rect;
		    GetClientRect(hWnd,&rect);
		    cwFound->width = rect.right;
		    cwFound->height = rect.bottom;
		    if (cwFound->statusBar) cwFound->height -= GetSystemMetrics(SM_CYCAPTION);
		    GDBG_INFO(31,"WM_SIZE csim view 0x%x, sbar=%d  w,h = %d,%d\n",
					hWnd,cwFound->statusBar,cwFound->width,cwFound->height);
#ifndef CVG
		// CVG buffers are FIXED size, I allow H3 to enlarge to window
		    cwFound->endBase = cwFound->base + (cwFound->height+1)*cwFound->stride;
#endif
	    }
	    break;

	case WM_USER_RESTAT:
	    ASSERT(cwFound != NULL);
	{
	    char szBuf[256];
	    FxU32 col,r,g,b,a, hex=cwFound->hex;
	    FxU32 x = cwFound->mx, y = cwFound->my;
	    FxU32 base = cwFound->base, stride = cwFound->stride;

	    // check the VSD for the window and reformat the text
	    switch (cwFound->pixFormat) {
		case SSTG_PIXFMT_8BPP:
		        if ( cwFound->tiled )
			  col = csimReadMem8(cp, tiledAddress(base,stride,1,x,y));
			else
			  col = csimReadMem8(cp, base + y*stride + x);
			sprintf(szBuf,
				hex ? " %02x" : " %3d",
				col&0xFF);
			break;
		case SSTG_PIXFMT_15BPP:
		        if ( cwFound->tiled )
			  col = csimReadMem16(cp, tiledAddress(base,stride,2,x,y));
			else
			  col = csimReadMem16(cp, base + y*stride + x*2);
			r = (col>>7)&0xF8;
			g = (col>>2)&0xF8;
			b = (col<<3)&0xF8;
			a = col & 0x8000 ? 0xFF : 0x00;
			switch (cwFound->videoFilter) {
			    case 0:		// none
				break;
			    case 1:		// expand
			    	r |= r >> 5;
                                g |= g >> 5;
			    	b |= b >> 5;
				break;
			    case 2:
				break;
			    case 3:
				break;
			    default:
				GDBG_ERROR("WM_USER_RESTAT","invalid videoFilter = %d\n",
						cwFound->videoFilter);
			}
			sprintf(szBuf,
				hex ? " %02x %02x : %02x %02x %02x %02x" :
				" %3d %3d : %3d %3d %3d %3d",
				(col>>8)&0xFF,col&0xFF,
				a,r,g,b);
			break;
		case SSTG_PIXFMT_16BPP:
		        if ( cwFound->tiled )
			  col = csimReadMem16(cp, tiledAddress(base,stride,2,x,y));
			else
			  col = csimReadMem16(cp, base + y*stride + x*2);
			r = (col>>8)&0xF8;
			g = (col>>3)&0xFC;
			b = (col<<3)&0xF8;
			switch (cwFound->videoFilter) {
			    case 0:		// none
				break;
			    case 1:		// expand
			    	r |= r >> 5;
                                g |= g >> 6;
			    	b |= b >> 5;
				break;
			    case 2:
				break;
			    case 3:
				break;
			    default:
				GDBG_ERROR("WM_USER_RESTAT","invalid videoFilter = %d\n",
						cwFound->videoFilter);
			}
			sprintf(szBuf,
				hex ? " %02x %02x : %02x %02x %02x" :
				" %3d %3d : %3d %3d %3d",
				(col>>8)&0xFF,col&0xFF,
				r,g,b);
			break;
		case SSTG_PIXFMT_24BPP:
		        if ( cwFound->tiled )
			  col = csimReadMem24(cp, cwFound->tiled, tiledAddress(base,stride,3,x,y));
			else
			  col = csimReadMem24(cp, 0, base + y*stride + x*3);
			sprintf(szBuf,
				hex ? " %02x %02x %02x" :
				" %3d %3d %3d",
				(col>>16)&0xFF,(col>>8)&0xFF,col&0xFF);
			break;
		case SSTG_PIXFMT_32BPP:
		        if ( cwFound->tiled )
			  col = csimReadMem32(cp, tiledAddress(base,stride,4,x,y));
			else
			  col = csimReadMem32(cp, base + y*stride + x*4);
			sprintf(szBuf,
				hex ? " %02x %02x %02x %02x" :
				" %3d %3d %3d %3d",
				(col>>24)&0xFF,(col>>16)&0xFF,(col>>8)&0xFF,col&0xFF);
			break;
		default:
			ASSERT(0);
	    }
	    // send the text to the status bar
	    SendMessage(GetDlgItem(hWnd,IDC_STATUSBAR), SB_SETTEXT, 3, (LPARAM)szBuf);
	    break;
	}

	case WM_COMMAND:
	    return doCommand(hWnd, msg, wParam, lParam);
	    break;

        case WM_CLOSE:				// close the window
	    GDBG_INFO(30,"WM_CLOSE csim view 0x%x\n",hWnd);
	    if (cwFound->hWndDisplaySettings)
		DestroyWindow(cwFound->hWndDisplaySettings);
	    deleteCsimWindow(hWnd);
	    DestroyWindow(hWnd);
	    break;

        default:
	    return(DefWindowProc(hWnd, msg, wParam, lParam));
    }
    return(0);
}

#endif
