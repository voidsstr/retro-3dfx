/* -*-c++-*- */
/* $Header: vmipld.h, 2, 10/11/00 8:47:01 PM, Brent$ */
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
** File name:   vmipld.c
**
** Description: Interface between an external PLD and the VMI port.
**
** $Revision: 2$
** $Date: 10/11/00 8:47:01 PM$
**
** $History: vmipld.h $
** 
** *****************  Version 3  *****************
** User: Xingc        Date: 9/07/99    Time: 11:54a
** Updated in $/devel/h5/W2K/Src/Video/Displays/h5
** Add W2K support
** 
** *****************  Version 2  *****************
** User: Xingc        Date: 9/03/99    Time: 10:34a
** Updated in $/devel/h5/Win9x/dx/inc
** move all the functions into \minivdd\vmipld.c.  Defines some constant
** for register and functions
** 
** 
** *****************  Version 1  *****************
** User: Lpost        Date: 8/25/99    Time: 2:17p
** Created in $/devel/h5/Win9x/dx/inc
** V3TV code merge into h5
** 
** 
**
*/

#ifndef __VMIPLD_H__
#define __VMIPLD_H__

//////////////////////////////////////////////////////////////////////////
//Function pointers to either external PLD reads and write or internal chip reads and writes
//This is detected early on and both use the same input and output parameters.

//V3TV functions for Napalm versus external pld read/writes
typedef  BYTE  (* PFNVMIREAD)(DWORD , BYTE);
typedef  void  (* PFNVMIWRITE)(DWORD , BYTE, BYTE) ;

BYTE __stdcall VMIPLD_Read(DWORD  sstIOregs, BYTE addr);
VOID __stdcall VMIPLD_Write(DWORD  sstIOregs, BYTE addr, BYTE data);

void PLD656_Init(DWORD sstIOregs);
void PLD656_Write(DWORD sstIOregs, BYTE addr, BYTE data);
void VMIWriteData(DWORD sstIOregs, BYTE addr, BYTE data);
BYTE PLD656_Read(DWORD sstIOregs, BYTE addr);
BYTE VMIReadData(DWORD sstIOregs, BYTE addr);

BYTE VMIPLD_Available(DWORD		sstIOregs, PFNVMIREAD VMIPLD_Read);  // returns version
BYTE VMIPLD_ReadStatus(DWORD	sstIOregs, PFNVMIREAD VMIPLD_Read);
BYTE VMIPLD_ReadCommand(DWORD	sstIOregs, PFNVMIREAD VMIPLD_Read);
void VMIPLD_SetCommand(DWORD	sstIOregs, BYTE bCommand, PFNVMIREAD VMIPLD_Read, PFNVMIWRITE VMIPLD_Write);
void VMIPLD_SetVbiMax(DWORD		sstIOregs, DWORD dwLines, PFNVMIWRITE VMIPLD_Write);
void VMIPLD_SetVidMax(DWORD		sstIOregs, DWORD dwLines, PFNVMIWRITE VMIPLD_Write);
void VMIPLD_ReadVidMax(DWORD	sstIOregs, DWORD *dwLines, PFNVMIREAD VMIPLD_Read);
void VMIPLD_ReadVbiMax(DWORD	sstIOregs, DWORD *dwLines, PFNVMIREAD VMIPLD_Read);

//////////////////////////////////////////////////////////////////////////
#define VMI_HOST_ENABLE					0x00001
#define VMI_HOST_CS_N					0x00002
#define VMI_HOST_RD_N					0x00004
#define VMI_HOST_WR_N					0x00008
#define VMI_HOST_RDY					0x00010
#define VMI_HOST_DATA_OUT_DISABLE		0x00020
#define VMI_HOST_DATA_MASK				0x03FC0
#define VMI_HOST_ADDR_MASK				0x3C000
#define VMI_HOST_RESET_N				0x10000000
#define VMI_HOST_MASK					0x3FFFF
#define VMI_HOST_DATA_SHIFT				6
#define VMI_HOST_ADDR_SHIFT				14
#define VMI_HOST_IDLE					0x0002F		// data tristate, host enable, cs, rd, wr, rdy high
//////////////////////////////////////////////////////////////////////////
#define VID_SERIAL_PARALLEL_PORT		0x78

//////////////////////////////////////////////////////////////////////////
#define VMIPLD_MODE_PASSTHRU	0
#define VMIPLD_MODE_656			1
#define VMIPLD_MODE_USE_GUID	2
#define VMIPLD_MODE_MAX_VAL		2
//////////////////////////////////////////////////////////////////////////
//Field for register 0
#define VMI_PASSTHRU    1
#define VMI_VBIINT      2
#define VMI_VBICROP     4
#define VMI_WASVBIINTR     0x20
#define VMI_WASEVENFIELD   0x10

// The following defines are for Napalm & above.
#define VIP2VMI_ACCESS					0x40000000
#define VIP2VMI_ENABLE					0x80000000
#define VIP2VMI_RESET					0x00080000
#define VIP2VMI_CMD_BITS            0xC0000007
#define VIP2VMI_VBIMAX_BITS			0x000001F0
#define VIP2VMI_VIDMAXLO_BITS       0x0001FE00
#define VIP2VMI_VIDMAXHI_BITS       0x00060000

#define VBIMAX_SHIFT                4
#define VIDMAXLO_SHIFT              9
#define VIDMAXHI_SHIFT              17
#define INTR_FIELD_SHIFT            26
#define VMI_HOST_REV_SHIFT          2


//////////////////////////////////////////////////////////////////////////
//IOCotrol function codes
#define  VMI_INIT       0x0001
#define  VMI_SETVIDMAX  0x0002
#define  VMI_SETVBIMAX  0x0003
#define  VMI_GETVIDMAX  0x0004
#define  VMI_GETVBIMAX  0x0005
#define  VMI_SETCOMMAND 0x0006
#define  VMI_GETCOMMAND 0x0007
#define  VMI_GETSTATUS  0x0008
#define  VMI_AVAILABLE  0x0009

//////////////////////////////////////////////////////////////////////////
#ifdef WINNT
#define VDD_VMI_FUNCTION IOCTL_VMI_FUNCTION 
#define VDD_VMIWrite (PFNVMIWRITE) HwDeviceExtension->Kernel_VMIWrite
#define VDD_VMIRead  (PFNVMIREAD) HwDeviceExtension->Kernel_VMIRead
#endif

#define DD_PLD656_Init(ppdev)  IOControl(ppdev,VDD_VMI_FUNCTION,VMI_INIT,0)
#define DD_VMI_SetVidMax(ppdev,dwLines) IOControl(ppdev, VDD_VMI_FUNCTION,VMI_SETVIDMAX, dwLines)
#define DD_VMI_SetVbiMax(ppdev,dwLines) IOControl(ppdev, VDD_VMI_FUNCTION,VMI_SETVBIMAX, dwLines)
#define DD_VMI_ReadVidMax(ppdev) IOControl(ppdev, VDD_VMI_FUNCTION,VMI_GETVIDMAX,0)
#define DD_VMI_ReadVbiMax(ppdev) IOControl(ppdev, VDD_VMI_FUNCTION,VMI_GETVBIMAX,0)
#define DD_VMI_SetCommand(ppdev,dwCommand) IOControl(ppdev, VDD_VMI_FUNCTION,VMI_SETCOMMAND,dwCommand)
#define DD_VMI_ReadCommand(ppdev) IOControl(ppdev, VDD_VMI_FUNCTION,VMI_GETCOMMAND,0)
#define DD_VMI_ReadStatus(ppdev) IOControl(ppdev, VDD_VMI_FUNCTION,VMI_GETSTATUS,0,)
#define DD_VMI_Available(ppdev)  IOControl(ppdev, VDD_VMI_FUNCTION,VMI_AVAILABLE,0)  

#define  TOTAL_VMI_FUNCTIONS 9
#ifndef WINNT
extern PFNVMIWRITE VDD_VMIWrite;
extern PFNVMIREAD VDD_VMIRead;
#endif

//////////////////////////////////////////////////////////////////////////
#endif //VMIPLD inc
