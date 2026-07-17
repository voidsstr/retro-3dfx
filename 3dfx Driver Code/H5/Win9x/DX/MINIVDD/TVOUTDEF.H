/* -*-c++-*- */
/* $Header: tvoutdef.h, 4, 10/11/00 8:55:20 PM, Brent$ */
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
** File name:   tvoutdef.h
**
** Description: TV Out universal definitions file.
**
** $Revision: 4$
** $Date: 10/11/00 8:55:20 PM$
**
** $History: tvoutdef.h $
** 
** *****************  Version 11  *****************
** User: Rbissell     Date: 8/07/99    Time: 3:13p
** Updated in $/devel/h5/Win9x/dx/minivdd
** tvout merge from V3_OEM_100
** 
** *****************  Version 9  *****************
** User: Stuartb      Date: 3/18/99    Time: 11:45a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added (*tvout_FixupVGA)(PDEVTABLE, int) to tvout function table.
** 
** *****************  Version 8  *****************
** User: Stuartb      Date: 3/12/99    Time: 8:45a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added cvbsOut and tvBoot parms to TVOUT_CURSETUP.
** 
** *****************  Version 2  *****************
** User: Stuartb      Date: 2/23/99    Time: 2:42p
** Updated in $/Releases/Voodoo3/MT2/3Dfx/devel/H3/Win95/DX/minivdd
** Added prototype for tvstdToNVRAM().
** 
** *****************  Version 7  *****************
** User: Stuartb      Date: 2/23/99    Time: 2:33p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added prototype for tvstdToNVRAM().
** 
** *****************  Version 6  *****************
** User: Stuartb      Date: 2/13/99    Time: 6:41p
** Updated in $/devel/h3/Win95/dx/minivdd
** First cut at PAL_M, PAL_N, PAL_NC support in tvout.
** 
** 
** *****************  Version 5  *****************
** User: Michael      Date: 1/15/99    Time: 9:43a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
*/

#ifndef _TVOUTDEF_H
#define _TVOUTDEF_H

/********************************************************************************
*
*  TV Out universal definitions file  (all encoders)
*
*
*********************************************************************************/

#define BT868_ADDR           (0x88)
#define BT868_ALT_ADDR       (0x8A)
#define CH7003_ADDR          (0xEC)
#define CH7003_ALT_ADDR      (0xEA)

//vidInFormat register defines and masks:
#define H3_VMI_MODE_MASK                    0x0000c000
#define H3_VMI_MODE_TV                      0x00008000
#define H3_VMI_MODE_VMI                     0x00004000

// NOTE: This struct appears in DevTable as tvOutData.  Make sure that DevTable
// member is big enough!  Currently 32 bits.

typedef struct
{
// don't save mode in registry anymore because it is made obsolete by changes between 640x480 and 800x600.
// Instead derive mode from tvStd on the fly.  DanO 5/18/99 in WinNt, 12/28/99 in Win9x
    unsigned obsolete_mode:2;
	unsigned bright:3;
	unsigned filter:3;
	unsigned saturation:3;
	unsigned hpos:7;
	unsigned vpos:7;
	unsigned size:3;
	unsigned is_master:1;
	unsigned lastValueIndex:3;
	unsigned copyProtectOn:2;
	unsigned encoderInitAttempt:1;
	unsigned tvStd:5;    // this mirrors the BIOS scratch register
	unsigned cvbsOut:1;
	unsigned tvBoot:1;
}  TVOUT_CURSETUP;


// device specific calls for tvout encoders

typedef struct
{
	int (*tvout_GetStatus)(PDEVTABLE, int);
	int (*tvout_GetPosition)(PTVCURPOS, void *tvOutData);
	int (*tvout_SetStandard)(PTVSETSTANDARD pTvSetStandard, PDEVTABLE pDev);
	int (*tvout_SetPicControl)(PTVSETCAP pTvSetPicControl, PDEVTABLE pDev);
	int (*tvout_SetPosition)(PTVSETPOS pTvSetPosition, PDEVTABLE pDev);
	int (*tvout_SetSize)(PTVSETSIZE pTvSetSize, PDEVTABLE pDev);
	int (*tvout_SetSpecial)(PTVSETSPECIAL pTvSetSpecial, DWORD RegBase);
	int (*tvout_Disable)(PDEVTABLE pDev);
	int (*tvout_GetPicControl)(PTVCURCAP pTvCurCap, void *tvOutData);
	int (*tvout_GetFilterControl)(PTVCURCAP tvPicValue, void *tvOutData);
	int (*tvout_GetSizeControl)(PTVCURSIZE tvPicValue, void *tvOutData);
	int (*tvout_Enable)(PDEVTABLE pDev, int vgaMode);
	int (*tvout_GetStandard)(PDEVTABLE, void * );
	int (*tvout_GetSizeCap)(PDEVTABLE, void * );
	int (*tvout_GetPosCap)(PDEVTABLE, void * );
	int (*tvout_GetFilterCap)(PDEVTABLE, void * );
	int (*tvout_GetPicCap)(PDEVTABLE, void * );
	int (*tvout_CopyProtect)(PDEVTABLE, int);
	int (*tvout_FixupVGA)(PDEVTABLE, int);
}  TVOUT_DEV_CALLS;

int tvOutSetNextValue (char *valueName, int value, TVOUT_CURSETUP *tvOutData);
int tvOutGetNextValue (char *valueName, TVOUT_CURSETUP *tvOutData);

#endif
