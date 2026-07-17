#ifndef __UI_H__
#define __UI_H__

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
** $Date: 10/11/00 8:41:31 PM$
*/

#if defined(BUILD_UI)
#define FX_DLL_DEFINITION
#include <fxdll.h>
#endif

FX_ENTRY void	FX_CALL uiCursor( HWND hWnd );
FX_ENTRY void	FX_CALL uiMain( const char *szAppName,
			FxU32 w, FxU32 h,
			WORD IconID, WORD MenuID,
			WNDPROC MainWndProc,
			BOOL (*CheckMessage)(HWND,MSG *),
			void (*Reshape)(FxU32 w, FxU32 h),
			void (*doFrame)(HWND hwnd));
FX_ENTRY void	FX_CALL uiSwapBuffers( HWND hWnd, HWND hwndStatusBar );

FX_ENTRY int	FX_CALL uiTimingTicks( int ticks );
FX_ENTRY void	FX_CALL uiTimingEnable( int flag );
FX_ENTRY int	FX_CALL uiTimingIsEnabled( void );
FX_ENTRY void	FX_CALL uiTimingLoop( void );

FX_ENTRY BOOL   FX_CALL uiAboutBoxDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
FX_ENTRY HWND	FX_CALL uiCreateSpin( HWND parent, int spinID, int buddyID,
				    int min, int max, int pos);

#endif /* !__UI_H__ */
