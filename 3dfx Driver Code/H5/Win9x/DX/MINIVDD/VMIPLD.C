/* -*-c++-*- */
/* $Header: vmipld.c, 5, 10/11/00 8:55:23 PM, Brent$ */
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
** $Revision: 5$
** $Date: 10/11/00 8:55:23 PM$
**
** $History: vmipld.c $
**
** *****************  Version 6  *****************
** User: Xingc        Date: 9/07/99    Time: 12:20p
** Updated in $/devel/h5/W2K/Src/Video/Miniport/h5
** W2K  VIM/PLD functions
**
** *****************  Version 5  *****************
** User: Xingc        Date: 9/03/99    Time: 1:10p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Change VMIPLD functions
**
** *****************  Version 4  *****************
** User: Lpost        Date: 8/25/99    Time: 2:17p
** Updated in $/devel/h5/Win9x/dx/minivdd
** V3TV code merge into H5
**
** *****************  Version 3  *****************
** User: Dalev        Date: 7/28/99    Time: 12:53p
** Updated in $/devel/h5/Win9x/dx/minivdd
** 'AccessExternalPLD()' now determines correct DevNode for register
** reads/writes.
**
** *****************  Version 2  *****************
** User: Andrew       Date: 7/16/99    Time: 2:06p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Changed regBase and RegBase from single dword to array to support
** sparse register mapping
**
** *****************  Version 1  *****************
** User: Dalev        Date: 7/01/99    Time: 6:50p
** Created in $/devel/h5/Win9x/dx/minivdd
** Original;  Interface between external PLD & VMI port or, on Napalm, the
** internal 'VIP2VMICtrl' register & VMI port.
**
**
*/


#define FAR
#define WINAPI __stdcall
typedef          char  CHAR;
typedef   signed char  SCHAR;
typedef unsigned char  UCHAR;

typedef          short SHORT;
typedef unsigned short USHORT;

typedef          long  LONG;
typedef unsigned long  ULONG;

typedef unsigned char  BYTE,  FAR* LPBYTE;
typedef unsigned short WORD,  FAR* LPWORD;
typedef unsigned long  DWORD, FAR* LPDWORD;
typedef          void  VOID,  FAR* LPVOID;
typedef          int   BOOL;
#ifndef WINNT
typedef int (FAR WINAPI *FARPROC)();


#pragma pack( 1 )
    #include "gdidefs.h"
    #include "dibeng.h"
#pragma pack()

#ifdef V3TV
#include "string.h"
#include "stdlib.h"
#endif

#include "3dfx.h"

#include "h3vdd.h"

#include "h3.h"

#include "h3g.h"

#define IS_32 1
#define MM 1

#include "shared.h"
#include "devtable.h"         // Leave this here; it externs 'pVGADevTable'.
#undef  THUNK32               // which holds the PDevice of the 1st display
                              // device detected.  That's a good thing.
//#define NT9XDEVICEDATA GLOBALDATA
//#define  ghwAC (SstCRegs*)(_FF(regBase) + SST_CMDAGP_OFFSET)

#include <ddkmmini.h>
#include <stddef.h>
#include "vmipld.h"

#pragma VxD_LOCKED_DATA_SEG
#pragma VxD_LOCKED_CODE_SEG


#define VMI_HOST_MASK					0x3FFFF
#define VMI_HOST_IDLE					0x0002F		// data tristate, host enable, cs, rd, wr, rdy high
#define VMI_HOST_ADDR_SHIFT         14
#define VMI_HOST_CS_N					0x00002
#define VMI_HOST_RD_N					0x00004
#define VMI_HOST_WR_N					0x00008

#define VMI_HOST_DATA_SHIFT         6
#define VMI_HOST_ADDR_SHIFT         14
#define VMI_HOST_REV_SHIFT          2


#pragma VxD_LOCKED_DATA_SEG

PFNVMIWRITE VDD_VMIWrite;
PFNVMIREAD VDD_VMIRead;

#pragma VxD_LOCKED_CODE_SEG
#else
#include "dderror.h"
#include "devioctl.h"

#include "miniport.h"
#include "ntddvdeo.h"
#include "video.h"
#include "dxmini.h"
#include "h3.h"
#include "vmipld.h"
#endif

/**********************************************************************
*   QueryForVMIPld
*
*   DESCRIPTION: Returns the polarity of the current field being written
*       to the specified video port.
*
*   ENTRY:
*          EAX  dwMiniVDDContext;
*		   ESI  LPDDPOLARITYININFO
*                   DWORD dwSize
*                   LPDDVIDEOPORTDATA
*          EDI  LPDDPOLARITYOUTINFO
*                   DWORD dwSize
*                   DWORD bPolority (even field = TRUE, odd field = FALSE)
*
*   EXIT:
*          EAX  0 = success, 1 = error
*          ECX  0 = odd,     1 = even
*
*
*   MODIFIES:
*          EAX, EBX, ECX
*
**********************************************************************/
#ifndef WINNT
DWORD QueryForVMIPld( PDEVTABLE pDev )
#else
DWORD QueryForVMIPld(  PHW_DEVICE_EXTENSION HwDeviceExtension)
#endif

{
#ifndef WINNT
    SstIORegs *sstIOregs = (void *)pDev->RegBase[HWINFO_SST_IOREGS_INDEX];
#else
   PH3_MEMBASE0 sstIOregs = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
#endif
    DWORD dwRet;
#ifndef WINNT
    if(IS_NAPALM(pDev->dwVendorDeviceID))
#else
    if (IS_NAPALM)
#endif
    {
      VMIWriteData((DWORD)sstIOregs,04, 0xff);
      VDD_VMIWrite = VMIWriteData;
      VDD_VMIRead  = VMIReadData;
    }
    else
    {
   	  PLD656_Init( (DWORD)sstIOregs );
      VDD_VMIWrite = PLD656_Write;
      VDD_VMIRead = PLD656_Read;
    }
     dwRet =   (DWORD)VMIPLD_Available( (DWORD)sstIOregs,VDD_VMIRead);

    return dwRet;

}


#ifndef WINNT
DWORD __stdcall VDDVMIFunctions(DIOCPARAMETERS *lpParams)
#else
VP_STATUS VDDVMIFunctions(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
        PVIDEO_REQUEST_PACKET lpParams)
#endif

{
#ifndef WINNT
   GLOBALDATA * ppdev =(GLOBALDATA *)(*((DWORD *)(lpParams->lpvInBuffer) + 2));
   SstIORegs *sstIORegs  = (SstIORegs*)(_FF(regBase[HWINFO_SST_IOREGS_INDEX]));
   DWORD dwFunctions =*(DWORD *)(lpParams->lpvInBuffer);
   DWORD dwInput = *((DWORD *)(lpParams->lpvInBuffer) + 1);
   DWORD *lpOutput = (DWORD *)(lpParams->lpvOutBuffer);
   lpParams->lpcbBytesReturned = 4;
#else
   PH3_MEMBASE0 sstIORegs = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
   DWORD dwFunctions =*(DWORD *)(lpParams->InputBuffer);
   DWORD dwInput = *((DWORD *)(lpParams->InputBuffer) + 1);
   DWORD *lpOutput = (DWORD *)(lpParams->OutputBuffer);

   lpParams->StatusBlock->Information = sizeof( ULONG );
#endif

   switch(dwFunctions)
   {
      case VMI_INIT:
	   if(VDD_VMIRead == PLD656_Read)
        	PLD656_Init((DWORD)sstIORegs);
		else
			VMIWriteData((DWORD)sstIORegs,4, 0xff);
		
        break;

      case  VMI_SETVIDMAX:
        VMIPLD_SetVidMax((DWORD)sstIORegs, dwInput, VDD_VMIWrite);
        break;

      case  VMI_SETVBIMAX:
        VMIPLD_SetVbiMax((DWORD)sstIORegs, dwInput, VDD_VMIWrite);
        break;

      case  VMI_GETVIDMAX:
        VMIPLD_ReadVidMax((DWORD)sstIORegs,lpOutput, VDD_VMIRead);
        break;

      case  VMI_GETVBIMAX:
        VMIPLD_ReadVbiMax((DWORD)sstIORegs,lpOutput, VDD_VMIRead);
        break;

      case VMI_SETCOMMAND:
        VMIPLD_SetCommand((DWORD)sstIORegs, (BYTE)dwInput,
                        VDD_VMIRead, VDD_VMIWrite);
        break;

      case VMI_GETCOMMAND:
        *lpOutput  =
          (DWORD)VMIPLD_ReadCommand((DWORD)sstIORegs, VDD_VMIRead);
        break;

      case VMI_GETSTATUS:
         *lpOutput =
           (DWORD)VMIPLD_ReadCommand((DWORD)sstIORegs, VDD_VMIRead);
        break;

      case VMI_AVAILABLE:
        *lpOutput =
           (DWORD)VMIPLD_Available((DWORD)sstIORegs, VDD_VMIRead);
        break;

      default:
        return -1;
   }
#ifndef WINNT
   return 0;
#else
   return NO_ERROR;
#endif
}

void PLD656_Init(DWORD sstIOregs)
{
#ifndef WINNT
	volatile DWORD *VidSerialParallelPort = &(DWORD)(((SstIORegs *)sstIOregs)->vidSerialParallelPort);
#else
	volatile DWORD *VidSerialParallelPort = &(DWORD)(((PH3_MEMBASE0)sstIOregs)->vidSerialParallelPort);
#endif
	DWORD dwReg;

	dwReg = *VidSerialParallelPort;
	dwReg &= ~VMI_HOST_MASK;
	//  reset & idle
	dwReg |= VMI_HOST_IDLE;
	dwReg &= ~VMI_HOST_RESET_N;
	*VidSerialParallelPort = dwReg;
	// clear reset
	dwReg |= VMI_HOST_RESET_N;
	*VidSerialParallelPort = dwReg;
}

/**********************************************************************
*   PLD656_Write
*
*   DESCRIPTION: Write to external PLD chip - V3TV
**********************************************************************/
void  PLD656_Write(DWORD sstIOregs,
					  BYTE addr,
					  BYTE data)
{
#ifndef WINNT
	volatile DWORD *VidSerialParallelPort = &(DWORD)(((SstIORegs *)sstIOregs)->vidSerialParallelPort);
#else
   volatile	DWORD *VidSerialParallelPort = &(DWORD)(((PH3_MEMBASE0)sstIOregs)->vidSerialParallelPort);
#endif
	DWORD dwReg;			// note writes should use the command fifo

	dwReg = *VidSerialParallelPort;
	dwReg &= ~VMI_HOST_MASK;
	//  idle
	dwReg |= VMI_HOST_IDLE;
	*VidSerialParallelPort = dwReg;

	// add data and addr
	dwReg |= (addr << VMI_HOST_ADDR_SHIFT) | (data << VMI_HOST_DATA_SHIFT);
	*VidSerialParallelPort = dwReg;
	// set cs active
	dwReg &= ~VMI_HOST_CS_N;
	*VidSerialParallelPort = dwReg;
	// set data active
	dwReg &= ~VMI_HOST_DATA_OUT_DISABLE;
	*VidSerialParallelPort = dwReg;
	// set wr active
	dwReg &= ~VMI_HOST_WR_N;
	*VidSerialParallelPort = dwReg;
	// should not require time here (5ns is spec)
	// set wr inactive
	dwReg |= VMI_HOST_WR_N;
	*VidSerialParallelPort = dwReg;
	// set data inactive
	dwReg |= VMI_HOST_DATA_OUT_DISABLE;
	*VidSerialParallelPort = dwReg;
	// set cs inactive
	dwReg |= VMI_HOST_CS_N;
	*VidSerialParallelPort = dwReg;
	// turn off host interface
	dwReg &= ~VMI_HOST_ENABLE;
	*VidSerialParallelPort = dwReg;
}

/**********************************************************************
*   VMIWriteData
*
*   DESCRIPTION: Writes the VMI data to the Napalm chipset
*   addr = 0   set command
*   addr = 1   set vbi lines
*   addr = 2, 3 set video lines
*   addr = 4   init to 656 port type
**********************************************************************/
VOID VMIWriteData(DWORD sstIOregs,
				   BYTE addr,
				   BYTE data)
{
   DWORD dwReg;                // note writes should use the command fifo

   // Read 'miscInit0' and enable access to 'vip2vmiCtrl' reg.
#ifndef WINNT
   dwReg = ((SstIORegs *)sstIOregs)->miscInit0;
   dwReg |= VIP2VMI_ACCESS;
   ((SstIORegs *)sstIOregs)->miscInit0 = dwReg;

   // 'miscInit1' now becomes the 'vip2vmiCtrl' register.  Read it.
   dwReg = ((SstIORegs *)sstIOregs)->miscInit1;
#else
   dwReg = ((PH3_MEMBASE0)sstIOregs)->miscInit0;
   dwReg |= VIP2VMI_ACCESS;

   ((PH3_MEMBASE0)sstIOregs)->miscInit0 = dwReg;

   // 'miscInit1' now becomes the 'vip2vmiCtrl' register.  Read it.
   dwReg = ((PH3_MEMBASE0)sstIOregs)->miscInit1;
#endif
   switch( addr )
   {
   case 0:                             // Access bits 2-0 in 'vip2vmictrl' reg.
      dwReg &= ~VIP2VMI_CMD_BITS;      // Clear bits 2-0.
      dwReg |= (data & 0x07);        // Set bits 2-0 w/ 'dwData' values.
      if( (data & 0x01) == 0)        // Enable VIP2VMI Translation
      {
         dwReg |= VIP2VMI_ENABLE;
         dwReg &= ~VIP2VMI_RESET;
      }
      else                             // Enable PassThru.
      {
         dwReg &= ~VIP2VMI_ENABLE;
         dwReg |= VIP2VMI_RESET;
      }

   break;
   case 1:
      dwReg &= ~VIP2VMI_VBIMAX_BITS;
      dwReg |= ((data & 0x1F) << VBIMAX_SHIFT);
      if( (dwReg & 0x01) == 0)        // Enable VIP2VMI Translation
      {
         dwReg |= VIP2VMI_ENABLE;
         dwReg &= ~VIP2VMI_RESET;
      }
      else                             // Enable PassThru.
      {
         dwReg &= ~VIP2VMI_ENABLE;
         dwReg |= VIP2VMI_RESET;
      }

   break;
   case 2:
      dwReg &= ~VIP2VMI_VIDMAXLO_BITS;
      dwReg |= (data << VIDMAXLO_SHIFT);
      if( (dwReg & 0x01) == 0)        // Enable VIP2VMI Translation
      {
         dwReg |= VIP2VMI_ENABLE;
         dwReg &= ~VIP2VMI_RESET;
      }
      else                             // Enable PassThru.
      {
         dwReg &= ~VIP2VMI_ENABLE;
         dwReg |= VIP2VMI_RESET;
      }

   break;
   case 3:
      dwReg &= ~VIP2VMI_VIDMAXHI_BITS;
      dwReg |= ( (data & 0x03) << VIDMAXHI_SHIFT);
      if( (dwReg & 0x01) == 0)        // Enable VIP2VMI Translation
      {
         dwReg |= VIP2VMI_ENABLE;
         dwReg &= ~VIP2VMI_RESET;
      }
      else                             // Enable PassThru.
      {
         dwReg &= ~VIP2VMI_ENABLE;
         dwReg |= VIP2VMI_RESET;
      }
   break;
   case 4:	  //set 656 port
      dwReg = VIP2VMI_VBIMAX_BITS | VIP2VMI_VIDMAXLO_BITS | 
      			VIP2VMI_VIDMAXHI_BITS | VIP2VMI_ENABLE;
   break;
   }

#ifndef WINNT
   ((SstIORegs *)sstIOregs)->miscInit1 = dwReg;

   // Re-enable 'miscInit1'
   dwReg = ((SstIORegs *)sstIOregs)->miscInit0;
   dwReg &= ~VIP2VMI_ENABLE;      // Enable access to 'miscInit1' reg.
   ((SstIORegs *)sstIOregs)->miscInit0 = dwReg;

#else
   ((PH3_MEMBASE0)sstIOregs)->miscInit1 = dwReg;

   // Re-enable 'miscInit1'
   dwReg = ((PH3_MEMBASE0)sstIOregs)->miscInit0;
   dwReg &= ~VIP2VMI_ENABLE;      // Enable access to 'miscInit1' reg.
   ((PH3_MEMBASE0)sstIOregs)->miscInit0 = dwReg;

#endif

}


/**********************************************************************
*   PLD656_ReadData
*
*   DESCRIPTION: Read PLD data from external pld chip - V3TV
*   addr = 0   read command and status bits
*   addr = 1   read vbi lines
*   addr = 2, 3 read video lines
**********************************************************************/
BYTE PLD656_Read(DWORD sstIOregs,
				 BYTE addr)
{
#ifndef WINNT
	volatile DWORD *VidSerialParallelPort = &(DWORD)(((SstIORegs *)sstIOregs)->vidSerialParallelPort);
#else
	volatile DWORD *VidSerialParallelPort = &(DWORD)(((PH3_MEMBASE0)sstIOregs)->vidSerialParallelPort);
#endif
	DWORD dwReg;			// note writes should use the command fifo
	DWORD dwData;
	BYTE  data;
	dwReg = *VidSerialParallelPort;
	dwReg &= ~VMI_HOST_MASK;
	//  dle
	dwReg |= VMI_HOST_IDLE;
	*VidSerialParallelPort = dwReg;

	// add data and addr
	dwReg |= (addr << VMI_HOST_ADDR_SHIFT);
	*VidSerialParallelPort = dwReg;
	// set cs active
	dwReg &= ~VMI_HOST_CS_N;
	*VidSerialParallelPort = dwReg;
	// set rd active
	dwReg &= ~VMI_HOST_RD_N;
	*VidSerialParallelPort = dwReg;
	// should not require time here (5ns is spec)

	// read data
	dwData = *((volatile DWORD *)VidSerialParallelPort);
	data = (BYTE)((dwData >> VMI_HOST_DATA_SHIFT) & 0xFF);
	// set rd inactive
	dwReg |= VMI_HOST_RD_N;
	*VidSerialParallelPort = dwReg;
	// set cs inactive
	dwReg |= VMI_HOST_CS_N;
	*VidSerialParallelPort = dwReg;
	// turn off host interface
	dwReg &= ~VMI_HOST_ENABLE;
	*VidSerialParallelPort = dwReg;
	return data;
}

/**********************************************************************
*   VMIReadData
*
*   DESCRIPTION: Read Napalm data as PLD information is within chip
*   addr = 0   read command and status bits
*   addr = 1   read vbi lines
*   addr = 2, 3 read video lines
**********************************************************************/
BYTE  VMIReadData(DWORD sstIOregs,
				 BYTE addr)
{
   DWORD dwReg, dwData;                // note writes should use the command fifo
   BYTE  data;

   // Read 'miscInit0' and enable access to 'vip2vmiCtrl' reg.
#ifndef WINNT
   dwReg = ((SstIORegs *)sstIOregs)->miscInit0;
   dwReg |= VIP2VMI_ACCESS;
   ((SstIORegs *)sstIOregs)->miscInit0 = dwReg;

   // 'miscInit1' now becomes the 'vip2vmiCtrl' register.  Read it.
   dwReg = ((SstIORegs *)sstIOregs)->miscInit1;

#else
   dwReg = ((PH3_MEMBASE0)sstIOregs)->miscInit0;
   dwReg |= VIP2VMI_ACCESS;
   ((PH3_MEMBASE0)sstIOregs)->miscInit0 = dwReg;

   // 'miscInit1' now becomes the 'vip2vmiCtrl' register.  Read it.
   dwReg = ((PH3_MEMBASE0)sstIOregs)->miscInit1;

#endif

   switch( addr )
   {
   case 0:
      dwData = (dwReg & VIP2VMI_CMD_BITS);
      dwData |= (dwReg >> INTR_FIELD_SHIFT);
      // Copy to Output buffer of DeviceIOControl() call.
	  data = (BYTE) (dwData & 0xFF);
   break;
   case 1:
      dwData = (dwReg & VIP2VMI_VBIMAX_BITS);
      dwData >>= VBIMAX_SHIFT;
      // Copy to Output buffer of DeviceIOControl() call.
      data = (BYTE)(dwData & 0xFF);
   break;
   case 2:
      dwData = (dwReg & VIP2VMI_VIDMAXLO_BITS);
      dwData >>= VIDMAXLO_SHIFT;
      // Copy to Output buffer of DeviceIOControl() call.
      data = (BYTE) (dwData & 0xFF);
   break;
   case 3:
      dwData = (dwReg & VIP2VMI_VIDMAXHI_BITS);
      dwData >>= VIDMAXHI_SHIFT;
      // Copy to Output buffer of DeviceIOControl() call.
      data = (BYTE)(dwData & 0xFF);
  break;
   }

#ifndef WINNT
   // Re-enable 'miscInit1'
   dwReg = ((SstIORegs *)sstIOregs)->miscInit0;
   dwReg &= ~VIP2VMI_ENABLE;      // Enable access to 'miscInit1' reg.
   ((SstIORegs *)sstIOregs)->miscInit0 = dwReg;
#else

   // Re-enable 'miscInit1'
   dwReg = ((PH3_MEMBASE0)sstIOregs)->miscInit0;
   dwReg &= ~VIP2VMI_ENABLE;      // Enable access to 'miscInit1' reg.
   ((PH3_MEMBASE0)sstIOregs)->miscInit0 = dwReg;

#endif
   return data;
}


/**********************************************************************
*   VMIPLD_SetVidMax
*
*   DESCRIPTION:
**********************************************************************/
void VMIPLD_SetVidMax(DWORD sstIOregs,
					  DWORD dwLines,
					  PFNVMIWRITE VMIPLD_Write)
{
	VMIPLD_Write(sstIOregs, 2, (BYTE)(dwLines & 0xFF));
	VMIPLD_Write(sstIOregs, 3, (BYTE)((dwLines >> 8) & 0x03));
}

/**********************************************************************
*   VMIPLD_SetVbiMax
*
*   DESCRIPTION:
**********************************************************************/
void VMIPLD_SetVbiMax(DWORD  sstIOregs,
					  DWORD dwLines,
					  PFNVMIWRITE VMIPLD_Write)
{
	VMIPLD_Write(sstIOregs, 1, (BYTE)(dwLines & 0xFF));
}



/**********************************************************************
*   VMIPLD_ReadVidMax
*
*   DESCRIPTION:
**********************************************************************/
void VMIPLD_ReadVidMax(DWORD  sstIOregs,
					   DWORD *dwLines,
					   PFNVMIREAD VMIPLD_Read)
{
	BYTE data1, data2;
	data1 = VMIPLD_Read(sstIOregs, 2);
	data2 = VMIPLD_Read(sstIOregs, 3);
	*dwLines = data1 | ((data2 & 0x03) << 8);
}


/**********************************************************************
*   VMIPLD_ReadVbiMax
*
*   DESCRIPTION:
**********************************************************************/
void VMIPLD_ReadVbiMax(DWORD  sstIOregs,
					   DWORD *dwLines,
					   PFNVMIREAD VMIPLD_Read)
{
	BYTE data;
	data = VMIPLD_Read(sstIOregs, 1);
	*dwLines = (data & 0x01F);
}


/**********************************************************************
*   VMIPLD_SetCommand
*
*   DESCRIPTION:
**********************************************************************/
void VMIPLD_SetCommand(DWORD sstIOregs,
                       BYTE bCommand,
					   PFNVMIREAD VMIPLD_Read,
					   PFNVMIWRITE VMIPLD_Write)
{
	BYTE reg;
	reg = VMIPLD_Read(sstIOregs, 0);
	reg &= 0xF8;
 	reg |= bCommand;
	VMIPLD_Write(sstIOregs, 0, reg);
}


/**********************************************************************
*   VMIPLD_ReadCommand
*
*   DESCRIPTION:
**********************************************************************/
BYTE VMIPLD_ReadCommand(DWORD sstIOregs,
						PFNVMIREAD VMIPLD_Read)
{
	return VMIPLD_Read(sstIOregs, 0);
}

/**********************************************************************
*   VMIPLD_ReadStatus
*
*   DESCRIPTION:
**********************************************************************/
BYTE VMIPLD_ReadStatus(DWORD sstIOregs,
					   PFNVMIREAD VMIPLD_Read)
{
	return VMIPLD_Read(sstIOregs, 0);
}

/**********************************************************************
*   VMIPLD_Available
*
*   DESCRIPTION:
**********************************************************************/
BYTE VMIPLD_Available(DWORD sstIOregs,
					  PFNVMIREAD VMIPLD_Read)
{
	BYTE data;

	if(VMIPLD_Read == PLD656_Read)
	{
		data = VMIPLD_Read(sstIOregs, 1);

		if ((data & 0xE0) != 0x80)
			return 0;
		data = VMIPLD_Read(sstIOregs, 3);
		data = ((data & 0xFC) >> 2);
		if (data >= 1 && data < 0x3F)	// not all 1's
			return data;
	 }
	 else
	 	return 2;		//for NAPALM

	return 0;
}

#ifndef WINNT
#undef IS_32
#undef MM
#define THUNK32
#endif
