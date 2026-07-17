/* $Header: fxtvout.h, 4, 10/11/00 8:44:59 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** File name:   tvout.h
**
** Description: Defines the VIDEO_PARAMETER structure and calling
**              parameters change to the ChangeDisplaySettingEx()
**              Win32 API. 
**              The audience for this file are Win32 developers who
**              want to call ChangeDisplaySettingEx with the
**              CDS_VIDEOPARAMETERS flag set, and display driver
**              developers who want to implement the VIDEO_PARAMETERS 
**              escape in their Control() function.
**
** $Revision: 4$ 
** $Date: 10/11/00 8:44:59 PM$
**
** $History: tvout.h $
** 
** *****************  Version 5  *****************
** User: Doconnell    Date: 9/03/99    Time: 10:51a
** Updated in $/devel/h5/WinNT/Src/Video/Displays/h5
** Add WinNT4 specific changes wrapped in #ifdef WINNT.
** 
** *****************  Version 3  *****************
** User: Michael      Date: 12/30/98   Time: 2:29p
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB unified header.
** 
*/
#include <tvout.h>

//
// This file defines the external interfaces for the routines in tvout.c
// This file is named "fxtvout.h" in order to avoid conflict with "tvout.h" which is a Win2K DDK file.

// The definitions for the  VIDEO_PARAMETER structure which are include in this file for some operating
// systems are in the file "tvout.h" for Win2K.

// Display driver Escape value to get/set the parameters.
//
#define VIDEO_PARAMETERS        3077    // escape value used in Control()

extern void TVOutStatus( PDEV *ppdev, LPQIN lpQIN, LPQTVSTATUS lpOutput );
extern void TVOutConStatus( PDEV *ppdev, LPQIN lpQIN, LPTVCONSTATUS lpOutput );
extern void TVOutGetPicCap( PDEV *ppdev, LPQIND lpQIN, LPTVCAPDATA lpOutput );
extern void TVOutGetFilterCap( PDEV *ppdev, LPQIND lpQIN, LPTVCURCAP lpOutput );
extern void TVOutGetPosCap( PDEV *ppdev, LPQIN lpQIN, LPTVPOSCAP lpOutput );
extern void TVOutGetSizeCap( PDEV *ppdev, LPQIN lpQIN, LPTVSIZECAP lpOutput );
extern void TVOutGetSpecialCap( PDEV *ppdev, LPQIND lpQIN, LPVOID lpOutput );
extern void TVOutGetStandard( PDEV *ppdev, LPQIN lpQIN, LPTVGETSTANDARD lpOutput );
extern void TVOutGetPicControl( PDEV *ppdev, LPTVCURCAP lpQIN, LPTVCURCAP lpOutput );
extern void TVOutGetFilterControl( PDEV *ppdev, LPQIND lpQIN, LPTVCURCAP lpOutput );
extern void TVOutGetPosControl( PDEV *ppdev, LPQIN lpQIN, LPTVCURPOS lpOutput );
extern void TVOutGetSizeControl( PDEV *ppdev, LPQIN lpQIN, LPTVCURSIZE lpOutput );
extern void TVOutGetSpecialCtl( PDEV *ppdev, LPQIND lpQIN, LPVOID lpOutput );
extern void TVOutGetConStatus( PDEV *ppdev, LPQIN lpQIN, LPTVCON lpOutput );
extern void TVOutSetStandard( PDEV *ppdev, LPTVSETSTANDARD lpQIN );
extern void TVOutSetPicControl( PDEV *ppdev, LPTVSETCAP lpQIN );
extern void TVOutSetFilterControl( PDEV *ppdev, LPTVSETCAP lpQIN );
extern void TVOutSetPosControl( PDEV *ppdev, LPTVSETPOS lpQIN );
extern void TVOutSetSizeControl( PDEV *ppdev, LPTVSETSIZE lpQIN );
extern void TVOutSetSpecial( PDEV *ppdev, LPTVSETSPECIAL lpQIN );
extern void TVOutSetConStatus( PDEV *ppdev, LPTVSETCONNECTOR lpQIN );
extern void TVOutCommitReg( PDEV *ppdev, LPQIN lpQIN );
extern void TVOutRefreshMem( PDEV *ppdev, LPQIN lpQIN );
extern void TVOutDisable( PDEV *ppdev );
extern void TVOutEnable( PDEV *ppdev, LPQIN lpQIN );
extern int  TVOutVideoParameters (PDEV *ppdev, LPVIDEOPARAMETERS lpVidParamsIn,
                                               LPVIDEOPARAMETERS lpVidParamsOut);
extern void TVOutGetOverride( PDEV *ppdev, LPQIN lpQIN, PTVGETOVERRIDE lpOutput);
extern void TVOutSetOverride( PDEV *ppdev, PTVSETOVERRIDE lpQIN);
extern void TVOutCancelSettings( PDEV *ppdev, void * pInData, void * pOutData);

