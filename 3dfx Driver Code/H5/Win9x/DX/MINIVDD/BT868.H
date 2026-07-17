/* -*-c++-*- */
/* $Header: bt868.h, 6, 10/16/00 3:47:05 PM, Geoff Bullard$ */
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
** File name:   bt868.h
**
** Description: BrookTree 868 TV Out support.
**
** $Revision: 6$
** $Date: 10/16/00 3:47:05 PM$
**
** $History: bt868.h $
** 
** *****************  Version 8  *****************
** User: Rbissell     Date: 9/07/99    Time: 2:15p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Deployed the new modularized I2C code.
** 
** *****************  Version 7  *****************
** User: Rbissell     Date: 8/07/99    Time: 3:13p
** Updated in $/devel/h5/Win9x/dx/minivdd
** tvout merge from V3_OEM_100
** 
** *****************  Version 5  *****************
** User: Stuartb      Date: 2/13/99    Time: 6:43p
** Updated in $/devel/h3/Win95/dx/minivdd
** First cut at PAL_M, PAL_N, PAL_NC support in tvout.
** 
** *****************  Version 4  *****************
** User: Michael      Date: 1/04/99    Time: 1:21p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
*/

#ifndef _BT868_H_
#define _BT868_H_

/********************************************************************************
*
* The file Bt868.h implements the BrookTree TV out Functions and has the 
* BrookTree specific TVOut stuff.
*
*
*********************************************************************************/


#ifdef WINNT
#define BT868CONTEXT PHW_DEVICE_EXTENSION
extern int BT868_ProcessRequest(BT868CONTEXT HwDeviceExtension,PVIDEO_REQUEST_PACKET RequestPacket);
extern int BT868_Enable (BT868CONTEXT HwDeviceExtension, int vgaMode);
extern int BT868_GetStandard (BT868CONTEXT HwDeviceExtension, LPTVGETSTANDARD lpOutput);
extern FxBool BT868_SetupTvOut(BT868CONTEXT HwDeviceExtension);
extern int BT868_ProcessVideoParameters (BT868CONTEXT HwDeviceExtension, PVIDEO_REQUEST_PACKET RequestPacket);
extern ULONG BT868_SetActiveState (BT868CONTEXT HwDeviceExtension, FxU32 activeStateMask);
#else
#define BT868CONTEXT PDEVTABLE
int BT868_GetStatus (BT868CONTEXT pDev, int estatus);
int BT868_GetPosition (PTVCURPOS, void *tvOutData);
int BT868_SetStandard(PTVSETSTANDARD pTvSetStandard, BT868CONTEXT pDev);
int BT868_SetPicControl(PTVSETCAP pTvSetPicControl, BT868CONTEXT pDev);
int BT868_SetPosition(PTVSETPOS pTvSetPosition, BT868CONTEXT pDev);
int BT868_SetSize(PTVSETSIZE pTvSetSize, BT868CONTEXT pDev);
int BT868_SetSpecial(PTVSETSPECIAL pTvSetSpecial, DWORD RegBase);
int BT868_Disable(BT868CONTEXT pDev);
int BT868_GetPicControl (PTVCURCAP pTvCurCap, void *tvOutData);
int BT868_GetFilterControl (PTVCURCAP tvPicValue, void *tvOutData);
int BT868_GetSizeControl (PTVCURSIZE tvPicValue, void *tvOutData);
int BT868_Enable (BT868CONTEXT pDev, int vgaMode);
int BT868_GetStandard (BT868CONTEXT, void * );
int BT868_GetSizeCap (BT868CONTEXT, void * );
int BT868_GetPosCap (BT868CONTEXT, void * );
int BT868_GetFilterCap (BT868CONTEXT, void * );
int BT868_GetPicCap (BT868CONTEXT, void * );
int BT868_CopyProtect (BT868CONTEXT, int );
extern FxU32 BT868_SetActiveState (BT868CONTEXT, FxU32);

FxBool BT868_SetupTvOut(BT868CONTEXT pDev);
FxU32 GetTvoutInfoNVRAM(BT868CONTEXT pDev);

FxI32 nvramRead128 (BT868CONTEXT pDev, FxU8 *buffer);
#endif //def WINNT

#endif
