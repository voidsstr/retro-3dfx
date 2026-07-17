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
** $Date: 10/11/00 8:41:25 PM$
*/

#include <windows.h>
#include <commctrl.h>
#include <3dfx.h>
#include <gdebug.h>
#include <ui.h>

//---------------------------------------------------------------------------
// create a spin control next to a buddy window
// NOTE: we do not use UDS_ALIGNRIGHT as that makes the spinner overap the
//	3D controls a little, so we place it on the right side ourselves
//---------------------------------------------------------------------------
HWND FX_EXPORT FX_CSTYLE
uiCreateSpin(
	HWND parent,		// the parent window
	int spinID,		// the ID for this control
	int buddyID,		// the ID for the buddy control
	int min,		// the minimum pos value
	int max,		// the maximum pos value
	int pos			// the initial pos value
	)
{
    RECT rect;
    HWND buddyHWND, spin;

    buddyHWND = GetDlgItem(parent,buddyID);		// get buddy window
    GetClientRect(buddyHWND,&rect);			// and its coords
    MapWindowPoints(buddyHWND,parent,(LPPOINT)&rect,2);	// convert to parent coords
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
// generic proc for about box dialogs
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
