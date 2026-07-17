/* -*-c++-*- */
/* $Header: dfp.h, 18, 10/11/00 8:58:39 PM, Brent$ */
/*
** Copyright (c) 1995-2000, 3Dfx Interactive, Inc.
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
** File name:   dfp.h
**
** Description: Structures and functions declarations for xlcd.
**
** $Revision: 18$
** $Date: 10/11/00 8:58:39 PM$
**
** $History: dfp.h $
** 
** *****************  Version 1  *****************
** User: Dalev        Date: 8/20/99    Time: 4:26p
** Created in $/devel/h5/Win9x/dx/minivdd
** Original;  Prototypes for dfp.c.
** 
**
** 
*/
#ifndef __DFP_H__
#define __DFP_H__



#ifndef WINNT
#include "devtable.h"
#endif
#include "dfpctrl.h"

#include "dfpdisp.h"

void panelOff(void * pContext);
void panelOn(void * pContext);
FxU8 isPanelPresent(void * pContext);
FxU8 isPanelActive(void * pContext);
FxU8 isPanelScaling(void * pContext);
extern void DFP_Initialize(void * pContext);
#ifdef WINNT
extern void DfpSetPowerOn(void * pContext);
extern void DfpSetPowerOff(void * pContext);
extern void ProcessHotPlugEvent(void * pContext, void * pParams);
extern int DFP_ProcessRequest(void * pContext, PVIDEO_REQUEST_PACKET RequestPacket);
extern FxU32 DFP_SetActiveState (void * pContext, FxU32 activeStateMask);
#else
extern int DFP_ProcessRequest(void * pContext, DFPDEVICEPACKET * RequestPacket);
int panelFixupVGA(void * pContext, FxU32);
PDWORD panelGbl(void * pContext);
int panelEDID(void * pContext, FxU8 *);
#endif



#endif
