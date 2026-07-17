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
** $Date: 10/11/00 8:31:05 PM$
*/

#ifdef GUI
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include <stdio.h>
#include <h3.h>
#include <fximg.h>

#ifdef HAL_CSIM
#include "../csim/csim.h"
#endif

#include "gui.h"
#include "guimv.h"
#include <afxres.h>

static CHAR szMainClassName[] = "CsimClassMain";
static CHAR szViewClassName[] = "CsimClassView";

static FxU32 hWndCount;         // count of CSIM open windows
static HANDLE hInstance;        // module handle
static HACCEL hAccel;
HWND hwndCsimMainControl;

typedef struct {
  BITMAPINFOHEADER bmiHeader;
  RGBQUAD bmiColors[256];
} BITMAPINFO256;

BITMAPINFO256 bmi;

void *pvbits;
HBITMAP hBM;
HDC bmDC;

//---------------------------------------------------------------------------
// draw a pixel in each of the open windows
//---------------------------------------------------------------------------
void guiDrawPixel(CsimPrivate *cp, int x, int y, int addr, FxU32 r, FxU32 g, FxU32 b)
{
    CsimWindow *cw;
    FxU8 *dest;

    // quick exit
    if (cp->environment.flushCount < 0) return;

    if ((x >= bmi.bmiHeader.biWidth) || (y >= -bmi.bmiHeader.biHeight))
        return;

    // convert and copy the framebuffer data to the bitmap
    dest = pvbits;
    dest += 3 * (y * bmi.bmiHeader.biWidth + x);
    dest[0] = (FxU8)b;
    dest[1] = (FxU8)g;
    dest[2] = (FxU8)r;

    // amazingly BitBlt is as fast as FillREct!
    for (cw = cp->windows; cw != NULL; cw = cw->next) {
        // if addr falls within this window
        if (addr >= cw->base && addr < cw->endBase && x<cw->width && y<cw->height)
#ifndef CVG
        // and if base and stride match 3D color buffer or 2D destination buffer
#if 0
            //KMW: always do the BitBlt on the initial pixel output
        if ((cw->base == (int)cp->info->sstCSIM->colBufferAddr && cw->stride == (int)cp->info->sstCSIM->colBufferStride) ||
            ((cw->base == (int)((cp->gui.dstBaseAddr & SSTG_BASEADDR) >> SSTG_BASEADDR_SHIFT)) &&
            ( cw->stride == (int)((cp->gui.dstFormat & SSTG_DST_STRIDE) >> SSTG_DST_STRIDE_SHIFT))))
#endif /* #if 0 */
#endif
        {
            BitBlt(cw->hDC,x,y,1,1,bmDC,x,y,SRCCOPY);
            // this is WAY too slow....
            // we need this because otherwise BitBlt delays and the AUX
            // and color buffers share the same DibSection!!
//          GdiFlush();
        }
    }
}

#if 0
void guiDrawPixel(CsimPrivate *cp, int x, int y)
{
    CsimWindow *cw;

    for (cw = cp->windows; cw; cw = cw->next) {
        if (addr >= cw->base && addr < cw->endBase)
            BitBlt(cw->hDC,x,y,1,1,bmDC,x,y,SRCCOPY);
    }
}
#endif
// a real HACK - I need the application to call this so that I can get a chance
// to translate accelerator keys
FX_EXPORT int FX_CSTYLE h3HalTranslateAccelerator(HWND hWnd, MSG *msg)
{
    if (TranslateAccelerator(msg->hwnd, hAccel, msg)) return TRUE;
    if (IsDialogMessage(hwndCsimMainControl,msg)) return TRUE;
    // the parent turns out to be the csim property dialog box
    if (IsDialogMessage(GetParent(msg->hwnd),msg)) return TRUE;
    return FALSE;
}

//---------------------------------------------------------------------------
// update dynamic text boxes, this is fairly slow so we don't want to do it
// all the time
//---------------------------------------------------------------------------
void guiUpdateDynamicText(HWND hwndCsimMainControl, CsimPrivate *cp)
{
    char buf[80];

    if (cp->environment.valid) return;
    cp->environment.valid = FXTRUE;
    sprintf(buf,"%d",cp->environment.curBufferSwapCount+1);
    SetDlgItemText(hwndCsimMainControl,IDC_EDIT5,buf);
    sprintf(buf,"%d",cp->environment.curTriangleCount);
    SetDlgItemText(hwndCsimMainControl,IDC_EDIT6,buf);
    sprintf(buf,"%d",cp->environment.curTrianglePerFrameCount);
    SetDlgItemText(hwndCsimMainControl,IDC_EDIT7,buf);
    sprintf(buf,"%d",cp->info->sstCSIM->stats.fbiPixelsIn);
    SetDlgItemText(hwndCsimMainControl,IDC_EDIT9,buf);
#if 0
    sprintf(buf,"%d",cp->stats.texDownloads);
    SetDlgItemText(hwndCsimMainControl,IDC_EDIT10,buf);
    sprintf(buf,"%d",cp->stats.texBytes);
    SetDlgItemText(hwndCsimMainControl,IDC_EDIT11,buf);
    sprintf(buf,"%d",cp->stats.palDownloads+cp->stats.nccDownloads);
    SetDlgItemText(hwndCsimMainControl,IDC_EDIT12,buf);
#endif
}

//---------------------------------------------------------------------------
// generic proc for about box dialogs, this one is local to this DLL
// NOTE: we count on FX_CSTYLE to be the same as CALLBACK (__stdcall)
//---------------------------------------------------------------------------
BOOL FX_EXPORT FX_CSTYLE
uiAboutBoxDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
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

// HACK: global combo text list for the GDBG_LEVEL control
static char comboText[64][80];
static int comboTextN;

//---------------------------------------------------------------------------
// the properties/control dialog window proc
//---------------------------------------------------------------------------
static
BOOL CALLBACK CsimMainControl( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    int temp=0;
    HWND hWnd = GetParent(hwndDlg);
    HWND gdbgCombo = GetDlgItem(hwndDlg, IDC_COMBO_GDBG);
    CsimPrivate *cp = CSIM_PRIVATE(halInfo.boardInfo[GetWindowLong(hWnd,GWL_USERDATA)].sstCSIM);

    switch(uMsg) {
        case WM_INITDIALOG:
                CheckDlgButton(hwndDlg, IDC_CHECK1, cp->environment.flushOnCommands);
                CheckDlgButton(hwndDlg, IDC_CHECK2, cp->environment.paused);
                CheckDlgButton(hwndDlg, IDC_CHECK4, cp->environment.saveAfterSwap);
                CheckDlgButton(hwndDlg, IDC_CHECK5, cp->environment.resetPixelsIn);
                CheckDlgButton(hwndDlg, IDC_CHECK6, cp->environment.recipFlag == 0);
                CheckDlgButton(hwndDlg, IDC_CHECK7, cp->environment.skipRendering);
                CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO2,
                        cp->environment.pauseAfterNextSwap ? IDC_RADIO1 : IDC_RADIO2);
                CheckRadioButton(hwndDlg,IDC_RADIO3,IDC_RADIO4,
                        cp->environment.pauseAfterNextTriangle ? IDC_RADIO3 : IDC_RADIO4);
                // add all the items into the combo box
                for (temp=0; temp<comboTextN; temp++) {
                    SendMessage(gdbgCombo, CB_ADDSTRING, 0, (LONG)comboText[temp]); 
                }
                SendMessage(gdbgCombo, CB_SETCURSEL, 0, 0);

            return TRUE;

        case WM_TIMER:
            GDBG_INFO(30,"csim main control dialog: WM_TIMER: 0x%x 0x%x %d\n",HIWORD(wParam),LOWORD(wParam),lParam);
            ASSERT(hwndDlg == hwndCsimMainControl);
            guiReadMessageQueue();
            guiUpdateDynamicText(hwndDlg,cp);
            return TRUE;

        case WM_COMMAND:
            GDBG_INFO(30,"csim main control dialog: WM_COMMAND: 0x%x 0x%x %d\n",HIWORD(wParam),LOWORD(wParam),lParam);
            ASSERT(hwndDlg == hwndCsimMainControl);
            switch(LOWORD(wParam)) {
                case IDC_CHECK1:
                        cp->environment.flushOnCommands = !cp->environment.flushOnCommands;
                        return TRUE;

                case IDC_CHECK2:
                        if (cp->windows)
                            SendMessage(cp->windows->hWnd, WM_COMMAND,
                                cp->environment.paused ? ID_DEBUG_GO : ID_DEBUG_BREAK,
                                0);
                        return TRUE;

                case IDC_CHECK4:
                        cp->environment.saveAfterSwap = !cp->environment.saveAfterSwap;
                        return TRUE;
                case IDC_CHECK5:
                        cp->environment.resetPixelsIn = !cp->environment.resetPixelsIn;
                        return TRUE;
                case IDC_CHECK6:
                        cp->environment.recipFlag = cp->environment.recipFlag ? 0 : -1;
                        return TRUE;
                case IDC_CHECK7:
                        cp->environment.skipRendering = !cp->environment.skipRendering;
                        return TRUE;

                case IDC_RADIO1:
                        cp->environment.pauseAfterNextSwap = 1;
                        return TRUE;
                case IDC_RADIO2:
                        cp->environment.pauseAfterNextSwap = 0;
                        return TRUE;
                case IDC_RADIO3:
                        cp->environment.pauseAfterNextTriangle = 1;
                        return TRUE;
                case IDC_RADIO4:
                        cp->environment.pauseAfterNextTriangle = 0;
                        return TRUE;

                CASE_UPDOWN(hwndDlg,IDC_EDIT1,IDC_UPDOWN1,cp->environment.flushCount,0);
                CASE_UPDOWN(hwndDlg,IDC_EDIT2,IDC_UPDOWN2,halInfo.pollLimit,0);
                CASE_UPDOWN(hwndDlg,IDC_EDIT3,IDC_UPDOWN3,temp,0);

                case IDC_EDIT4:
                        if (HIWORD(wParam) == EN_KILLFOCUS) {
                            char buf[80];
                            GetDlgItemText(hwndDlg,IDC_EDIT4,buf,sizeof(buf));
                            sscanf(buf,"%i",&cp->environment.pauseAfterSwapNum);
                            sprintf(buf,"%d",cp->environment.pauseAfterSwapNum);
                            SetDlgItemText(hwndDlg,IDC_EDIT4,buf);
                            return TRUE;
                        }
                        break;
                case IDC_EDIT8:
                        if (HIWORD(wParam) == EN_KILLFOCUS) {
                            char buf[80];
                            GetDlgItemText(hwndDlg,IDC_EDIT8,buf,sizeof(buf));
                            sscanf(buf,"%i",&cp->environment.pauseAfterTriangleNum);
                            sprintf(buf,"%d",cp->environment.pauseAfterTriangleNum);
                            SetDlgItemText(hwndDlg,IDC_EDIT8,buf);
                            return TRUE;
                        }
                        break;

                case IDC_BUTTON_GDBG:
                {
                        char *cp = comboText[0];

                        // accept the current text entry, first get the text
                        SendMessage(gdbgCombo, WM_GETTEXT, sizeof(comboText[0]), (LPARAM)cp);
                        while (isspace(*cp)) cp++;      // skip over white space
                        if (*cp == '\0') return TRUE;   // if no tokens return

                        // if not already in the combo box then add it
                        if (CB_ERR == SendMessage(gdbgCombo, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)cp)) {
                            GDBG_INFO(30,"combo box: add '%s'\n", comboText[0]);
                            SendMessage(gdbgCombo, CB_INSERTSTRING, (WPARAM)0, (LONG)cp); 
                        }
                        // now 
                        GDBG_INFO(30,"combo box: '%s'\n", cp);
                        gdbg_parse(cp);
                        return TRUE;
                }

                case IDCANCEL:
                    // kill any timers associated with this
                    KillTimer(hwndCsimMainControl,WM_TIMER_ID);
                    // save all the combo box text entries in our global array
                    temp = SendMessage(gdbgCombo, CB_GETCOUNT, 0, 0);
                    if (temp > 64) temp = 64;
                    for (comboTextN=0; comboTextN<temp; comboTextN++)
                        SendMessage(gdbgCombo, CB_GETLBTEXT, comboTextN, (LPARAM)comboText[comboTextN]);
                    DestroyWindow(hwndDlg);
                    hwndCsimMainControl = NULL;
                    return TRUE;
            }
            break;
    }
    return FALSE;
}

//---------------------------------------------------------------------------
// create a spin control next to a buddy window
// NOTE: we do not use UDS_ALIGNRIGHT as that makes the spinner overap the
//      3D controls a little, so we place it on the right side ourselves
//---------------------------------------------------------------------------
HWND
uiCreateSpin(
        HWND parent,            // the parent window
        int spinID,             // the ID for this control
        int buddyID,            // the ID for the buddy control
        int min,                // the minimum pos value
        int max,                // the maximum pos value
        int pos                 // the initial pos value
        )
{
    RECT rect;
    HWND buddyHWND, spin;

    buddyHWND = GetDlgItem(parent,buddyID);             // get buddy window
    GetClientRect(buddyHWND,&rect);                     // and its coords
    MapWindowPoints(buddyHWND,parent,(LPPOINT)&rect,2); // convert to parent coords
    spin = CreateUpDownControl(WS_CHILD | WS_BORDER | WS_VISIBLE |
                        UDS_ARROWKEYS | UDS_SETBUDDYINT,
                        rect.right+2,rect.top-2,12,rect.bottom-rect.top+4,
                        parent, spinID, GetModuleHandle(NULL),
                        buddyHWND,max,min,pos);
    if (spin == NULL)
        GDBG_ERROR("CreateSpin", "CreateUpDownControl failed\n");
    return spin;
}

//---------------------------------------------------------------------------
// the main window proc, it processes menu commands
//---------------------------------------------------------------------------
LRESULT CALLBACK csimMainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
            // load up the accelerator table for CSIM
            hAccel = LoadAccelerators((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE),
                                        MAKEINTRESOURCE(IDR_ACCELERATOR1));
            break;
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case ID_FILE_EXIT:
                    GDBG_INFO(30,"ID_FILE_EXIT main\n");
                    if (IDYES ==MessageBox(NULL,
                                "Are you sure you want to exit?", "Csim",
        MB_SETFOREGROUND | MB_TASKMODAL | MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
                        PostQuitMessage(0);
                    break;

                case ID_WINDOW_NEWWINDOW:
                    GDBG_INFO(30,"ID_WINDOW_NEWWINDOW 0x%x\n",hWnd);
                    guiNewViewWindow(GetWindowLong(hWnd,GWL_USERDATA),CSIM_VIEW_WINDOW_NAME);
                    break;

                case ID_VIEW_PROPERTIES:
                case ID_WINDOW_PROPERTIES:
                    GDBG_INFO(30,"ID_WINDOW_PROPERTIES 0x%x\n",hWnd);
                    if (hwndCsimMainControl == NULL) {
                        char buf[80];
                        CsimPrivate *cp = CSIM_PRIVATE(halInfo.boardInfo[GetWindowLong(hWnd,GWL_USERDATA)].sstCSIM);
                        hwndCsimMainControl = CreateDialog((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE),
                                                MAKEINTRESOURCE(IDD_CSIM_MAIN),
                                                hWnd,
                                                CsimMainControl);
                        // initialize the spin controls
                        uiCreateSpin(hwndCsimMainControl,IDC_UPDOWN1, IDC_EDIT1, -1,9999,cp->environment.flushCount);
                        uiCreateSpin(hwndCsimMainControl,IDC_UPDOWN2, IDC_EDIT2, 1,10000,halInfo.pollLimit);
                        uiCreateSpin(hwndCsimMainControl,IDC_UPDOWN3, IDC_EDIT3, 1,16,1);
                        sprintf(buf,"%d",cp->environment.pauseAfterSwapNum);
                        SetDlgItemText(hwndCsimMainControl,IDC_EDIT4,buf);
                        sprintf(buf,"%d",cp->environment.pauseAfterTriangleNum);
                        SetDlgItemText(hwndCsimMainControl,IDC_EDIT8,buf);
                        if (!SetTimer(hwndCsimMainControl,WM_TIMER_ID,WM_TIMER_MSECS,NULL))
                            GDBG_ERROR("SetTimer","failed\n");
                        cp->environment.valid = 0;
                    }
                    else SetActiveWindow(hwndCsimMainControl);
                    break;

                case ID_WINDOW_CLOSE:
                    GDBG_INFO(30,"ID_WINDOW_CLOSE 0x%x main\n",hWnd);
                    SendMessage(hWnd,WM_CLOSE,0,0);
                    break;

                case ID_WINDOW_CLOSE_ALL:
                    GDBG_INFO(30,"ID_WINDOW_CLOSE_ALL 0x%x\n",hWnd);
                    deleteAllCsimWindows(hWnd);
                    break;

                case ID_HELP_CONTENTS:
                    GDBG_INFO(30,"ID_HELP_CONTENTS\n");
                    break;
                case ID_HELP_ABOUT:
                    GDBG_INFO(30,"ID_HELP_ABOUT main\n");
                    DialogBox((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE),
                                MAKEINTRESOURCE(IDD_ABOUTBOX),
                                hWnd,
                                uiAboutBoxDialogProc);
                    break;
                default:
                    return(DefWindowProc(hWnd, msg, wParam, lParam));
            }
            break;

        case WM_CLOSE:                          // close the window
            GDBG_INFO(30,"WM_CLOSE csim Main 0x%x\n",hWnd);
            deleteCsimWindow(hWnd);
            DestroyWindow(hWnd);
            break;

        default:
            return(DefWindowProc(hWnd, msg, wParam, lParam));
    }
    return(0);
}

// find a Csim window that contains hWnd and move it to front of list
CsimWindow *guiFindCsimWindow(CsimPrivate *cp, HWND hWnd)
{
    CsimWindow *found = cp->windows;
    CsimWindow *wlast = NULL;

    if (cp->mainWindow) {               // first check the main window
        if (cp->mainWindow->hWnd == hWnd)
            return cp->mainWindow;
    }
    while (found) {                     // search thru the entire list
        if (found->hWnd == hWnd) {      // if found it
            if (wlast) {                // if not the first entry
                wlast->next = found->next;      // then unlink it
                found->next = cp->windows;      // and make it the head
                cp->windows = found;
            }
            return found;
        }
        wlast = found;
        found = found->next;
    }
    return found;
}

//---------------------------------------------------------------------------
// Close a CSIM window
//---------------------------------------------------------------------------
void deleteCsimWindow(HWND hWnd)
{
    CsimWindow *found;
    CsimPrivate *cp = CSIM_PRIVATE(halInfo.boardInfo[GetWindowLong(hWnd,GWL_USERDATA)].sstCSIM);

    found = guiFindCsimWindow(cp,hWnd); // this brings it to the front of the list
    if (found) {
        if (cp->mainWindow == found)    // if it's the main window
            cp->mainWindow = NULL;
        else if (cp->windows == found)
            cp->windows = found->next;
        else
            GDBG_ERROR("deleteCsimWindow", "window not in front of list\n");

        free(found);
        hWndCount--;
        GDBG_INFO(102,"deleteCsimWindow(0x%x), %d windows open\n", hWnd, hWndCount);
    }
    else GDBG_ERROR("deleteCsimWindow", "window not found\n");
}

void deleteAllCsimWindows(HWND hWnd)
{
    CsimWindow *found = NULL;
    CsimPrivate *cp = CSIM_PRIVATE(halInfo.boardInfo[GetWindowLong(hWnd,GWL_USERDATA)].sstCSIM);

    while (cp->windows)
        SendMessage(cp->windows->hWnd,WM_CLOSE,0,0);
//    SendMessage(cp->mainWindow->hWnd,WM_CLOSE,0,0);
}

void guiChange(CsimPrivate *cp, int which)
{
    char buf[80];

    cp->environment.valid = 0;
    switch (which) {
        case 0:                 // cp->environment.curBufferSwapCount changed
                if (cp->environment.saveAfterSwap) {
                    sprintf(buf,"cmov%03d",cp->environment.saveAfterSwap);
                    cp->environment.saveAfterSwap++;
                    // XXX have to get width,height somehow
#ifdef CVG
                    // on CVG we have already swapped, so output FRONT
                    csimPicSave(cp,buf, CSIM_BUF_3D_FRONT, 640,480,IMG_P6);
#else
                    // on H3, we haven't yet changed colBufferAddr
                    csimPicSave(cp,buf, CSIM_BUF_3D_COLOR, 640,480,IMG_P6);
#endif
                }
                // if not displaying pixels then show the swap
                if (cp->environment.flushCount < 0 && !cp->environment.skipRendering)
                    guiRefresh(cp);
                if (cp->environment.pauseAfterNextSwap ||
                    (cp->environment.curBufferSwapCount+1 == cp->environment.pauseAfterSwapNum))
                    guiDoBreak(cp);
                cp->environment.curTrianglePerFrameCount = 0;
                if (cp->environment.resetPixelsIn)
                    cp->info->sstCSIM->stats.fbiPixelsIn = 0;
                break;
        case 1:
                cp->environment.curTrianglePerFrameCount++;
                cp->environment.curTriangleCount++;
                if (cp->environment.pauseAfterNextTriangle ||
                    (cp->environment.curTrianglePerFrameCount == cp->environment.pauseAfterTriangleNum))
                    guiDoBreak(cp);
                break;
        case 2: // texture downloads, pixels drawn
                break;
        default:
                GDBG_ERROR("guiChange","invalid parameter %d\n",which);
                break;
    }
}

//---------------------------------------------------------------------------
// redisplay the contents of all the windows
//---------------------------------------------------------------------------
void guiRefresh(CsimPrivate *cp)
{
    guiApplyToAllWindows(cp,guiRefreshWindow);
}

void guiRefreshWindow(CsimWindow *cw)
{
    RECT rect;
    GDBG_INFO(30,"guiRefreshWindow(0x%x)\n",cw);
    rect.left = rect.top = 0;
    rect.right = cw->width;
    rect.bottom = cw->height;
    guiRepaint(cw,cw->hDC,&rect);
}

//---------------------------------------------------------------------------
// apply a function to all view windows
//---------------------------------------------------------------------------
void guiApplyToAllWindows(CsimPrivate *cp, void (*func)(CsimWindow *))
{
    CsimWindow *cw;

    for (cw = cp->windows; cw; cw = cw->next)
        func(cw);
}

//---------------------------------------------------------------------------
// Open up a generic window
//---------------------------------------------------------------------------
CsimWindow *guiNewWindow(int boardNumber, CHAR *className, const char *name, int w, int h)
{
    CHAR title[80];
    int xb,yb, cap;
    HWND hWnd;
    CsimWindow *cw;

    if (!halInfo.video) return NULL;
    xb = GetSystemMetrics(SM_CXFRAME);
    yb = GetSystemMetrics(SM_CYFRAME);
    cap = GetSystemMetrics(SM_CYCAPTION);

#ifdef CVG
    sprintf(title, "CVG:Csim.%d  %s",boardNumber,name);
#else
    sprintf(title, "H3:Csim.%d  %s",boardNumber,name);
#endif
    hWnd = CreateWindow(className,
                        title,
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        w + xb + xb,
                        h + yb + yb + cap + cap,
                        NULL,
                        NULL,
                        hInstance,
                        NULL);

    SetWindowLong(hWnd,GWL_USERDATA,boardNumber);
    hWndCount++;
    
    cw = (CsimWindow *)malloc(sizeof (CsimWindow));
    cw->next = NULL;
    cw->hDC = GetDC(hWnd);
    cw->hWnd = hWnd;
    cw->hWndDisplaySettings = NULL;
    cw->pixFormat = SSTG_PIXFMT_15BPP;
    cw->base = 0;
    cw->stride = 0;
    cw->tiled = 0;
    cw->zoom = 1;
    cw->videoFilter = 0;
    cw->hex = 0;
    cw->statusBar = 1;
    cw->fileSaveName[0] = '\0';
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    InvalidateRgn(hWnd,NULL,FALSE);             // force a WM_PAINT message
    GDBG_INFO(102,"guiNewWindow(%d,%s), %d windows open\n", boardNumber, name, hWndCount);
    return cw;
}

//---------------------------------------------------------------------------
// Open up a CSIM view window
//---------------------------------------------------------------------------
FX_EXPORT void FX_CSTYLE 
guiNewViewWindow(FxU32 boardNumber, const char *name)
{
    CsimPrivate *cp;
    CsimWindow *cw;

    if (!halInfo.video) return;
    GDBG_INFO(102,"guiNewViewWindow(%d,%s)\n", boardNumber, name);
    cp = CSIM_PRIVATE(halInfo.boardInfo[boardNumber].sstCSIM);
    cw = guiNewWindow(boardNumber, szViewClassName, name,
                        halInfo.boardInfo[boardNumber].fbiVideoWidth,
                        halInfo.boardInfo[boardNumber].fbiVideoHeight +
                                GetSystemMetrics(SM_CYCAPTION));
    cw->cp = cp;                                // link window back to private
    cw->next = cp->windows;                     // link into window list
    cp->windows = cw;
    // default it to the 3D color buffer
    cw->base = halInfo.boardInfo[boardNumber].sstCSIM->colBufferAddr;
    cw->stride = halInfo.boardInfo[boardNumber].sstCSIM->colBufferStride;

    //Make a 1x1 window as a default. A 0x0 window will generate a negative
    //tiled address in the tiledAddress() call below
    cw->width = 1;
    cw->height= 1;
    if ( (cw->stride & SST_BUFFER_MEMORY_TYPE) == SST_BUFFER_MEMORY_TILED ) {
      cw->tiled = 1;
      cw->stride &= SST_BUFFER_TILE_STRIDE;
      cw->endBase = tiledAddress(cw->base,
				 cw->stride,
				 2,
				 cw->width-1,
				 cw->height-1)+2;
    } else {
      cw->tiled = 0;
      cw->stride &= SST_BUFFER_LINEAR_STRIDE;
      cw->endBase = cw->base + (cw->height+1)*cw->stride;
    }

    // toggle the hex display and reformat the status bar text
    SendMessage(cw->hWnd,WM_COMMAND,ID_VIEW_HEXADECIMAL,0);
    SendMessage(cw->hWnd,WM_USER_RESIZE,0,0);   // recompute client clipping
    SendMessage(cw->hWnd,WM_COMMAND,ID_WINDOW_REFRESH,0);
}

//---------------------------------------------------------------------------
// read and process all messages, this gets called every so many pixels
// exit upon a WM_QUIT message because we assume we are running a console app
//---------------------------------------------------------------------------
void guiReadMessageQueue(void)
{
    MSG msg;

    while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
        if (!h3HalTranslateAccelerator(NULL, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (msg.message == WM_QUIT) {
#if !defined(FX_DLL_ENABLE)
            fxHalShutdownAll();
#endif
            exit(0);
        }
    }
}

//---------------------------------------------------------------------------
// initialize the display DIBs
//---------------------------------------------------------------------------
static void InitDibs(HWND hWnd)
{
    bmi.bmiHeader.biSize                = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth               = 1280;
    bmi.bmiHeader.biHeight              = -1024;        // top-down bitmap
    bmi.bmiHeader.biPlanes              = 1;
    bmi.bmiHeader.biBitCount            = 24;
    bmi.bmiHeader.biCompression         = BI_RGB;
    bmi.bmiHeader.biSizeImage           = 0;
    bmi.bmiHeader.biXPelsPerMeter       = 0;
    bmi.bmiHeader.biYPelsPerMeter       = 0;
    bmi.bmiHeader.biClrUsed             = 3;
    bmi.bmiHeader.biClrImportant        = 0;
    ((DWORD *)bmi.bmiColors)[0] = 0xff0000;
    ((DWORD *)bmi.bmiColors)[1] = 0x00ff00;
    ((DWORD *)bmi.bmiColors)[2] = 0x0000ff;
    hBM = CreateDIBSection(GetDC(hWnd),
                        (BITMAPINFO *) &bmi,
                        DIB_RGB_COLORS,
                        &pvbits, NULL, 0);
    if (!hBM) {
        GDBG_ERROR("InitDibs", "CreateDIBSection failed %d\n",GetLastError());
    }
    // NOTE: we use the main control window's DC (ok, a hack)
    bmDC = CreateCompatibleDC(GetDC(hWnd));
    SelectObject(bmDC,hBM);
}

// XXX the BITMAP pointed to by pvbits should be treated as a cache
// if we are repainting a window with the same VSD formt data, then we
// should take advantage of that fact and not recopy/reconvert data
void guiRepaint(CsimWindow *cw, HDC hDC, RECT *prect)
{
    int x,y, sliY;
    int save199 = GDBG_GET_DEBUGLEVEL(199);
    FxU8 *dest;

    SstRegs *sst;
    CsimPrivate *cp;

    ASSERT(cw != NULL);
    if (prect->bottom > cw->height)             // don't overwrite status bar
        prect->bottom = cw->height;

    GDBG_SET_DEBUGLEVEL(199,0);
    // convert and copy the framebuffer data to the bitmap
    for (y=prect->top; y<prect->bottom; y++) {
        FxU32 src;
        dest = pvbits;
        dest += 3 * (y * bmi.bmiHeader.biWidth + prect->left);

	sst = csimFindPixelOwner(y, &sliY);
	cp = CSIM_PRIVATE(sst);

	src = cw->base + sliY*cw->stride;

	/*
	GDBG_INFO(0, "y=%d  owner=%s  sliY=%d cp->memory=0x%x\n", y, cp->environment.name, sliY,
		  cp->memory);
	*/

        switch (cw->pixFormat) {
            case SSTG_PIXFMT_8BPP:
                src += prect->left*1;
                for (x=prect->left; x<prect->right; x++) {
                    FxU8 col;
                    if ( cw->tiled )
                      src = tiledAddress(cw->base,cw->stride,1,x,sliY);
                    col =  cp->memory[src];
// XXX for now just grey scale it....
                    dest[0] = col;
                    dest[1] = col;
                    dest[2] = col;
                    dest += 3;
                    src += 1;
                }
                break;
            case SSTG_PIXFMT_15BPP:
            case SSTG_PIXFMT_16BPP:
                src += prect->left*2;
                for (x=prect->left; x<prect->right; x++) {
                    FxU32 col,r,g,b;
                    if ( cw->tiled )
                      src = tiledAddress(cw->base,cw->stride,2,x,sliY);
                    col =  *(FxU16 *)&cp->memory[src];
		    if (cw->pixFormat==SSTG_PIXFMT_16BPP) {
                        r = (col>>8)&0xF8;
                        g = (col>>3)&0xFC;
                        b = (col<<3)&0xF8;
		    }
		    else {
                        r = (col>>7)&0xF8;
                        g = (col>>2)&0xF8;
                        b = (col<<3)&0xF8;
		    }
                    switch (cw->videoFilter) {
                        case 0:                 // none
                                break;
                        case 1:                 // expand
                                r |= r >> 5;
                                g |= g >> (cw->pixFormat==SSTG_PIXFMT_15BPP?5:6);
                                b |= b >> 5;
                                break;
                        case 2:
                                break;
                        case 3:
                                break;
                        default:
                                GDBG_ERROR("guiRepaint","invalid videoFilter = %d\n",
                                                cw->videoFilter);
                    }

                    dest[0] = (FxU8)b;
                    dest[1] = (FxU8)g;
                    dest[2] = (FxU8)r;
                    dest += 3;
                    src += 2;
                }
                break;
            case SSTG_PIXFMT_24BPP:
                src += prect->left*3;
                for (x=prect->left; x<prect->right; x++) {
                    if ( cw->tiled ) {
                      FxU32 src2 = tiledAddress(cw->base,cw->stride,3,x,sliY);
                      dest[0] = cp->memory[src2++];
                      if ( (src2 & SST_TILE_WIDTH_MASK) == 0 ) // check for tile straddle
                        src2 += SST_TILE_SIZE - SST_TILE_WIDTH;
                      dest[1] = cp->memory[src2++];
                      if ( (src2 & SST_TILE_WIDTH_MASK) == 0 ) // check for tile straddle
                        src2 += SST_TILE_SIZE - SST_TILE_WIDTH;
                      dest[2] = cp->memory[src2++];
                    } else {
                      dest[0] = cp->memory[src];
                      dest[1] = cp->memory[src+1];
                      dest[2] = cp->memory[src+2];
                    }
                    dest += 3;
                    src += 3;
                }
                break;
            case SSTG_PIXFMT_32BPP:
                src += prect->left*4;
                for (x=prect->left; x<prect->right; x++) {
                    if ( cw->tiled )
                      src = tiledAddress(cw->base,cw->stride,4,x,sliY);
                    dest[0] = cp->memory[src];
                    dest[1] = cp->memory[src+1];
                    dest[2] = cp->memory[src+2];
                    dest += 3;
                    src += 4;
                }
                break;
        }
    }
    GDBG_SET_DEBUGLEVEL(199,save199);

    x = BitBlt(hDC,prect->left,                 // now display it
                prect->top,
                prect->right - prect->left,
                prect->bottom - prect->top,
                bmDC,
                prect->left,
                prect->top,
                SRCCOPY);
    GdiFlush();
    if (x == 0)
        GDBG_ERROR("guiRepaint", "BitBlt failed  = %d\n",GetLastError());
    guiReadMessageQueue();
}

// callback to give to GDEBUG to keep things alive
void guiKeepAlive(int count)
{
    halInfo.pollCount -= count;
    if (halInfo.pollCount < 0) {
        halInfo.pollCount = halInfo.pollLimit;
        guiReadMessageQueue();
    }
}

//---------------------------------------------------------------------------
// one time init routine
//---------------------------------------------------------------------------
void guiInit( void )
{
    WNDCLASS wc;

#if !defined(FX_DLL_ENABLE)
    GDBG_INIT();
#endif
    hInstance        = GetModuleHandle("h4hal");
    GDBG_INFO(101,"guiInit(): hInstance=0x%x\n", hInstance);

    wc.style         = CS_NOCLOSE | CS_DBLCLKS | CS_OWNDC; // | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = csimMainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(SstRegs *);
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_CSIM));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_APPMENU);
    wc.lpszClassName = szMainClassName;
    if (!RegisterClass(&wc)) {
        GDBG_ERROR("RegisterClass (%s)", "error = %d\n", wc.lpszClassName, GetLastError());
        return;
    }
    // now register the views
    wc.style         = CS_DBLCLKS | CS_OWNDC; // | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = csimViewWndProc;
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_APPMENU1);
    wc.lpszClassName = szViewClassName;
    if (!RegisterClass(&wc)) {
        GDBG_ERROR("RegisterClass (%s)", "error = %d\n", wc.lpszClassName, GetLastError());
        return;
    }

    InitCommonControls();

    gdbg_set_keepalive((GDBGKeepAliveProc) guiKeepAlive);
}

//---------------------------------------------------------------------------
// open a CSIM gui for a board
//---------------------------------------------------------------------------
FxBool guiOpen( FxU32 boardNumber )
{
    CsimPrivate *cp = CSIM_PRIVATE(halInfo.boardInfo[boardNumber].sstCSIM);

#if !defined(FX_DLL_ENABLE)
    GDBG_INIT();
#endif
    if (!halInfo.video) return FXFALSE;
    GDBG_INFO(101,"guiOpen(%d)\n", boardNumber);
    if (hInstance == NULL)
        guiInit();
    if (cp->mainWindow == NULL)
        cp->mainWindow = guiNewWindow(boardNumber,szMainClassName,"Main Control",320,240);
    InitDibs(cp->mainWindow->hWnd);
    return FXTRUE;
}

//---------------------------------------------------------------------------
// shut down the CSIM gui for a board
//---------------------------------------------------------------------------
static void freeWindows(CsimWindow *w)
{
    CsimWindow *wnext = w;

    while (w = wnext) {
        hWndCount--;
        DestroyWindow(w->hWnd);                 // destroy the window
        wnext = w->next;
        free(w);                                // free the memory
    }
}

void guiShutdown( SstRegs *sst )
{
    CsimPrivate *cp = CSIM_PRIVATE(sst);

    GDBG_INFO(101,"guiShutdown(0x%x)\n", sst);
    DeleteDC(bmDC);
    DeleteObject(hBM);
    freeWindows(cp->windows);
    freeWindows(cp->mainWindow);
    cp->windows = NULL;
    cp->mainWindow = NULL;

    // this counts on the fact that boards are opened first and then shutdown later
    if (hWndCount == 0) {
        GDBG_INFO(101,"guiShutdown: destroyed last window\n");

        // class, so it goes away with the application
        // if (!UnregisterClass( szClassName, hInstance ))
        //      GDBG_ERROR("UnregisterClass", "error = %d\n", GetLastError());
    }
}

#else

//---------------------------------------------------------------------------
// stubs for when we do not compile with -DGUI=xxx
//---------------------------------------------------------------------------

#include <stdio.h>
#include <h3.h>
#include "../csim/csim.h"

void guiDrawPixel(CsimPrivate *cp, int x, int y, int addr, FxU32 r, FxU32 g, FxU32 b) {}

void guiKeepAlive(int count) {}

void guiRefresh(CsimPrivate *cp) {}

void guiChange(CsimPrivate *cp, int which) {}

void guiReadMessageQueue(void) {}

FX_ENTRY void FX_CALL guiUpdateStatusBar(void *cw) {}

FX_EXPORT void FX_CSTYLE 
guiNewViewWindow(FxU32 boardNumber, const char *name) {}

FxBool guiOpen( FxU32 boardNumber )
{
    return FXFALSE;
}

void guiShutdown( SstRegs *sst ) {}

#endif
