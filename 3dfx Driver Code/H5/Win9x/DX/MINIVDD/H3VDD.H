/* -*-c++-*- */
/* $Header: h3vdd.h, 5, 10/11/00 8:54:32 PM, Brent$ */
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
** File name:   h3vdd.h
**
** Description: VMM/VDD macros, externs, etc..
**
** $Revision: 5$
** $Date: 10/11/00 8:54:32 PM$
**
** $History: h3vdd.h $
** 
** *****************  Version 25  *****************
** User: Andrew       Date: 7/09/99    Time: 4:23p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added a Define for APCI_CTRL
** 
** *****************  Version 24  *****************
** User: Cwilcox      Date: 7/08/99    Time: 1:10p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added runtime checking for Napalm versus Voodoo3.
** 
** *****************  Version 23  *****************
** User: Andrew       Date: 6/25/99    Time: 9:55a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Modified GetNumUnits Prototype
** 
** *****************  Version 22  *****************
** User: Cwilcox      Date: 6/22/99    Time: 5:13p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Moved device ID defines to h3g.h.
** 
** *****************  Version 21  *****************
** User: Andrew       Date: 6/15/99    Time: 4:59p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added prototypes for PCI functions
** 
** *****************  Version 20  *****************
** User: Andrew       Date: 6/04/99    Time: 4:14p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added code to support UnitNumbers
** 
** *****************  Version 18  *****************
** User: Cwilcox      Date: 4/23/99    Time: 4:16p
** Updated in $/devel/h3/Win95/dx/minivdd
** Completed Napalm 32mb/64Mb changes.
** 
** *****************  Version 17  *****************
** User: Stuartb      Date: 3/18/99    Time: 11:57a
** Updated in $/devel/h3/Win95/dx/minivdd
** Fix outpw in DEBUG section to out a word rather than a byte.
** 
** *****************  Version 16  *****************
** User: Stuartb      Date: 2/23/99    Time: 2:34p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added MACRO to distinguish 3DFX reference boards from those of STB
** manufacture.
** 
** *****************  Version 15  *****************
** User: Agus         Date: 2/01/99    Time: 4:38p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added IS_H4 macro define
** 
** *****************  Version 14  *****************
** User: Andrew       Date: 1/21/99    Time: 5:51p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added some new constants
** 
** *****************  Version 13  *****************
** User: Michael      Date: 1/08/99    Time: 11:46a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 12  *****************
** User: Andrew       Date: 1/05/99    Time: 10:44a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added new define
** 
** *****************  Version 11  *****************
** User: Andrew       Date: 12/09/98   Time: 5:28p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added H4 OEM define
** 
** *****************  Version 10  *****************
** User: Andrew       Date: 11/24/98   Time: 8:36a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added some defines to help nagivate through the PCI Caps
** 
** *****************  Version 9  *****************
** User: Andrew       Date: 11/16/98   Time: 8:32p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added new defines for PCI space
** 
** *****************  Version 8  *****************
** User: Andrew       Date: 10/05/98   Time: 4:01p
** Updated in $/devel/h3/Win95/dx/minivdd
** Removed a prototype
** 
** *****************  Version 7  *****************
** User: Andrew       Date: 8/22/98    Time: 12:04p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added VesaPostProcessing to turn on IO trapping
** 
** *****************  Version 6  *****************
** User: Andrew       Date: 6/07/98    Time: 8:46a
** Updated in $/devel/h3/Win95/dx/minivdd
** Removed a prototype
** 
** *****************  Version 5  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

#define SST_PCI_DEVICE_ID (0x00)
#define SST_PCI_COMMAND_ID (0x04)
#define SST_PCI_REVISION_ID (0x08)
#define SST_PCI_HEADER_TYPE_ID (0x0E)
#define SST_PCI_MMIO_ID (0x10)
#define SST_PCI_FB_ID (0x14)
#define SST_PCI_IOBASE_ID (0x18)
#define SST_PCI_SSID (0x2C)
#define SST_BIOS_ROM (0x30)
#define SST_1ST_CAP (0x34)
#define SST_AGP_CAP (0x02)
#define SST_AGP_STATUS_ID (0x58)
#define SST_AGP_CMD_ID (0x5c)
#define SST_PCI_ACPI_CTL (0x64)
#define H3_IMASK (BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0))

// this macro reads 8 bits of the subVendorID from the VGA BIOS shadow.  
// It is at ROM_END-6, ROM_SIZE in 512 byte blocks is at ROM_START+2.
// The BIOS is shadowed at 0xc0000.

#define IS_3DFX_REF_BOARD (*(FxU8 *)((*(FxU8 *)0xc0002 * 512) - 6 + 0xc0000) < 0x30)
                    
#define H3_NUM_MEM_BASES	2

#define WANTVXDWRAPS
#include <basedef.h>
#include <vmm.h>
#include <debug.h>
#include <vwin32.h>
#include <vpicd.h>
#include <vxdwraps.h>
#include <configmg.h>
// #undef WANTVXDWRAPS
#include <pci.h>
#define MINIVDD
#include "minivdd.h"
#include "winerror.h"

//#include "h3irq.h"

#ifndef DEBUG
#pragma warning( disable : 4002 )
#endif

#define VNAME "H3VDD: "

// Structure for Hot Key system

typedef struct hotkey_data_node {
   DWORD HotKeyData ;
   DWORD dwHotKeyHandle ;
   DWORD dwHotKeyScan ; 
   DWORD dwHotKeyModifier ;  
} HOTKEY_DATA_NODE ;

// Locked data - available at interrupt time

#pragma VxD_LOCKED_DATA_SEG

// card configuration

extern BOOL     bIsVGA;                         // true if system VGA

// Windows data

extern HVM hSysVM;

// Miscellaneous

extern WORD     wVESATotalMemory;
extern int      iMillisecLoop;                  // loop variable for a millsec

// pageable data

#pragma VxD_PAGEABLE_DATA_SEG

// comms with VDD and Config manager

extern CMCONFIG        ConfigData;              // resources of board
extern DISPLAYINFO     DispInfo;                // data from main VDD

// config space

extern DWORD    dwRegBase;                      // Physical addr of registers
extern DWORD    dwFBBase;                       // Physical address of frame buffer
extern DWORD    dwDMABase;                      // Physical address of pseudo DMA

// Global functions

// VDD callbacks
VOID RegisterDisplayDriver( VOID );   
VOID PreHiResToVGA( VOID );                  
VOID PreHiResToVGA( VOID );                  
VOID PreVGAToHiRes( VOID );                  
VOID DisplayDriverDisabling( VOID ); 
VOID VESAPostSupport( VOID );       
DWORD MiniVDD_Virtual3DEH( VOID );
VOID VESASupport( VOID );
VOID PreHiResToVGA( VOID );
VOID PreVGAToHires( VOID );
VOID PostVGAToHiRes( VOID );
VOID GetTotalVRAMSize( VOID );
BOOL TurnVGAOff( VOID );
BOOL TurnVGAOn( VOID );
VOID VirtualizeCRTCout( VOID );
VOID VirtualizeSequencerOut( VOID );
VOID VirtualizeDacOut( VOID );
VOID Virtual_3c2h( VOID );
VOID myIOhandler( VOID );
VOID PreCRTCModeChange(VOID);
VOID PostCRTCModeChange(VOID);
VOID GetVDDbank(VOID);
//#ifdef SLI_AA			// #ifdef SLI_AA, commented out so that driver would build for non AA hardware (ie. Voodoo 3) - jhunter@3dfx.com
DWORD GetNumUnits(DWORD dwDevNode);
DWORD PCI_Read_Config(DWORD dwBus, DWORD dwDevFunc, DWORD dwOffset);
DWORD PCI_Write_Config(DWORD dwBus, DWORD dwDevFunc, DWORD dwOffset, DWORD dwValue);
DWORD PCIGetBusDevFunc(DWORD dwDevNode, DWORD * pdwBus, DWORD * pdwDevFunc);
//#endif

// Internal stuff
VOID VESASupport( VOID );

// VxD stuff

#pragma VxD_LOCKED_DATA_SEG
#pragma VxD_LOCKED_CODE_SEG

PVOID VXDINLINE
Map_Flat( BYTE dbSelReg, BYTE dbOffReg )
{
    PVOID pV;

    __asm mov ah,dbSelReg;
    __asm mov al,dbOffReg;
    VMMCall(Map_Flat);
    __asm mov pV,eax;
    return(pV);
}

#ifdef DEBUG

VOID VXDINLINE
outp( WORD wPort, BYTE dbValue )
{
    __asm mov dx,wPort;
    __asm mov al,dbValue;
    __asm out dx,al;
}

BYTE VXDINLINE
inp( WORD wPort )
{
    BYTE dbR;

    __asm mov dx,wPort;
    __asm in  al,dx;
    __asm mov dbR,al;
    return( dbR );
}

VOID VXDINLINE
outpw( WORD wPort, WORD wValue )
{
    __asm mov dx,wPort;
    __asm mov ax,wValue;
    __asm out dx,ax;
}

WORD VXDINLINE
inpw( WORD wPort )
{
    WORD wR;

    __asm mov dx,wPort;
    __asm in  ax,dx;
    __asm mov wR,ax;
    return( wR );
}

#endif // #ifdef DEBUG


#define WaitNMicroseconds(  n )                   \
{                                                 \
    volatile int t = (n * iMillisecLoop) / 1000;  \
                                                  \
    while( t-- ) /* void */;                      \
}

#pragma VxD_PAGEABLE_CODE_SEG
#pragma VxD_PAGEABLE_DATA_SEG

/*----------------------------------------------------------------------
Function name:  VDD_Get_DISPLAYINFO

Description:    
                
Information:    Why is this in the .h file?

Return:         DWORD    return from VxDCall 
----------------------------------------------------------------------*/
DWORD VXDINLINE
VDD_Get_DISPLAYINFO(DISPLAYINFO *pDI, DWORD dwSize)
{
    DWORD dwR;

    __asm pushad;
    __asm mov eax,pDI;
    __asm mov ecx,dwSize;
    VxDCall(VDD_Get_DISPLAYINFO);
    __asm mov dwR,eax;
    __asm popad;
    return(dwR);
}

