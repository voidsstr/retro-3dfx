#ifndef __GUIMV_H__
#define __GUIMV_H__

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
** $Date: 10/11/00 8:31:06 PM$
*/

#ifdef GDBG_INFO_ON
  #include <assert.h>
  #define ASSERT(exp) assert(exp)
#else
  #define ASSERT(exp)
#endif

#define	WM_USER_RESIZE	WM_USER
#define	WM_USER_RESTAT	WM_USER+1
#define WM_TIMER_ID	0x23
#define WM_TIMER_MSECS	100

// handy macro for handling spin control events
#define CASE_UPDOWN(hWnd,id1,id2,var,doit)	\
	case id1:			\
	    if (HIWORD(wParam) == EN_KILLFOCUS) {	\
		SendMessage(GetDlgItem(hWnd,id2),UDM_SETPOS,0,(LPARAM)var);	\
		return TRUE;	\
	    }			\
	    if (HIWORD(wParam) == EN_CHANGE) {	\
		var = LOWORD(SendMessage(GetDlgItem(hWnd,id2),UDM_GETPOS,0,0));	\
		doit;		\
		return TRUE;	\
	    }			\
	    break

#if HAL_CSIM
HWND hwndCsimMainControl;

CsimWindow *guiNewWindow(int boardNumber, CHAR *className, const char *name, int w, int h);
CsimWindow *guiFindCsimWindow(CsimPrivate *cp, HWND hWnd);
void guiRefreshWindow(CsimWindow *cw);
void guiRepaint(CsimWindow *, HDC, RECT *);
void guiUpdateDynamicText(HWND hwndCsimMainControl, CsimPrivate *cp);
void guiDoBreak(CsimPrivate *cp);
void deleteCsimWindow(HWND hWnd);
void deleteAllCsimWindows(HWND hWnd);

HWND uiCreateSpin( HWND parent, int spinID, int buddyID, int min, int max, int pos);
LRESULT CALLBACK csimViewWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL FX_EXPORT FX_CSTYLE
uiAboutBoxDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
#endif /* HAL_CSIM */

#endif /* !__GUIMV_H__ */
