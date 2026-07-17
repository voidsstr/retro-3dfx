/* -*-c++-*- */
/* $Header: h3irq.c, 8, 10/11/00 8:54:24 PM, Brent$ */
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
** File name:   h3irq.c
**
** Description: Interrupt handler and support services.
**
** $Revision: 8$
** $Date: 10/11/00 8:54:24 PM$
**
** $History: h3irq.c $
** 
** *****************  Version 31  *****************
** User: Xingc        Date: 9/03/99    Time: 10:39a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Use the new VMIPLD_SetCommand() and VMIPLD_ReadComand() functions
** 
** *****************  Version 30  *****************
** User: Lpost        Date: 8/25/99    Time: 2:15p
** Updated in $/devel/h5/Win9x/dx/minivdd
** V3TV code merge into H5
** 
** *****************  Version 29  *****************
** User: Xingc        Date: 8/02/99    Time: 4:41p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Delete code for banshee only
** 
** *****************  Version 28  *****************
** User: Xingc        Date: 7/29/99    Time: 4:52p
** Updated in $/devel/h5/Win9x/dx/minivdd
** change swapbuffer into 3D register group
** 
** *****************  Version 27  *****************
** User: Andrew       Date: 7/16/99    Time: 2:06p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Changed regBase and RegBase from single dword to array to support
** sparse register mapping
** 
** *****************  Version 26  *****************
** User: Cwilcox      Date: 7/08/99    Time: 1:09p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added runtime checking for Napalm versus Voodoo3.
** 
** *****************  Version 24  *****************
** User: Stb_lpost    Date: 5/25/99    Time: 11:53a
** Updated in $/devel/h3/win95/dx/minivdd
** V3TV Video Capture fixes using global events
** 
** *****************  Version 23  *****************
** User: Xingc        Date: 5/05/99    Time: 5:38p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fix HW DVD jittering problem
** 
** *****************  Version 22  *****************
** User: Xingc        Date: 4/28/99    Time: 5:55p
** Updated in $/devel/h3/Win95/dx/minivdd
** For Interleaved mode without BOB, flip overlay address on even field
** only.
** 
** *****************  Version 21  *****************
** User: Stb_lpost    Date: 4/26/99    Time: 7:58a
** Updated in $/devel/h3/win95/dx/minivdd
** V3TV - VBI Capture and Connection fixes
** 
** *****************  Version 20  *****************
** User: Xingc        Date: 4/09/99    Time: 11:08a
** Updated in $/devel/h3/Win95/dx/minivdd
** Flip overlay address for each field in BOB_INTERLEAVED mode
** 
** *****************  Version 19  *****************
** User: Xingc        Date: 4/08/99    Time: 5:25p
** Updated in $/devel/h3/Win95/dx/minivdd
** Support BOB_INTELEAVE by flipping vidoe port addresses
** 
** *****************  Version 18  *****************
** User: Stb_lpost    Date: 3/30/99    Time: 7:53a
** Updated in $/devel/h3/win95/dx/minivdd
** V3TV Video Capture Fixes
** 
** *****************  Version 17  *****************
** User: Stb_lpost    Date: 3/19/99    Time: 12:43p
** Updated in $/devel/h3/win95/dx/minivdd
** Kernel Mode V3TV ifdefd code
** 
** *****************  Version 16  *****************
** User: Xingc        Date: 3/04/99    Time: 6:33p
** Updated in $/devel/h3/Win95/dx/minivdd
** For BOB flip overlay based on even and odd field
** 
** *****************  Version 15  *****************
** User: Agus         Date: 2/01/99    Time: 4:37p
** Updated in $/devel/h3/Win95/dx/minivdd
** Move IS_H4 macro define to h3vdd.h
** 
** *****************  Version 14  *****************
** User: Andrew       Date: 1/21/99    Time: 5:46p
** Updated in $/devel/h3/Win95/dx/minivdd
** Coded EnableInterrupt and DisableInterrupt
** 
** *****************  Version 13  *****************
** User: Andrew       Date: 1/18/99    Time: 9:13p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed a problem with MultiMonitor where we were not sharing IRQ's
** correctly.  This happens due to the fact that the device can be
** disabled in which case we will think that we got a IRQ.  Changed to
** check that lpDriverData is not null before we use it.
** 
** *****************  Version 12  *****************
** User: Agus         Date: 1/15/99    Time: 6:12p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed Avenger's video-in buffer status bits location
** 
** *****************  Version 11  *****************
** User: Agus         Date: 1/15/99    Time: 4:07p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added check for Avenger (H4) when doing manual overlay flipping.
** 
** *****************  Version 10  *****************
** User: Michael      Date: 1/07/99    Time: 1:28p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 9  *****************
** User: Andrew       Date: 12/09/98   Time: 5:30p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added some code to set hIRQ to zero after we force default behavior
** 
** *****************  Version 8  *****************
** User: Agus         Date: 6/30/98    Time: 4:57p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added overlay manual flip to handle overlay cropping properly
** 
** *****************  Version 7  *****************
** User: Stuartb      Date: 6/11/98    Time: 8:34a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added retrace interrupt support, check multiple banshees, share
** interrupt correctly.
** 
** *****************  Version 6  *****************
** User: Stuartb      Date: 6/05/98    Time: 4:43p
** Updated in $/devel/h3/Win95/dx/minivdd
** Adding handling for vertical retrace interrupts.
** 
** *****************  Version 5  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

#include "h3vdd.h"
#include "h3.h"
#define VDDONLY
#include "devtable.h"
#include "h3irq.h"
#undef VDDONLY

#include <ddkmmini.h>

#pragma VxD_LOCKED_DATA_SEG
#pragma VxD_LOCKED_CODE_SEG


extern DWORD GETGBL_dwOvlOffset(DWORD);
extern DWORD GETGBL_KMVTBuff(DWORD);

typedef  BYTE   (* PFNVMIREAD)(DWORD , BYTE);
typedef  void   (* PFNVMIWRITE)(DWORD , BYTE, BYTE) ;

extern PDEVTABLE pVGADevTable;


//h3 intrCtrl bitmasks
#define H3_VMI_INT_ENABLE                   0x00200000
#define H3_VMI_INTERRUPT					0x00800000
#define H3_VSYNC_INT_ENABLE					0x00000004
#define H3_VSYNC_INTERRUPT					0x00000100
#define H3_VMI_HDECIMATION_MASK             0x00100000
#define H3_VMI_VDECIMATION_MASK             0x00200000
#define H3_VMI_DEINTERLACE_WEAVE			0x00000010
#define H3_VMI_SINGLE_BUFFER                0x00000000
#define H3_VMI_DOUBLE_BUFFER                0x00000200
#define H3_VMI_TRIPLE_BUFFER                0x00000400
#define H4_VMI_MODE_VMI                     0x00004000
#define H3_VMI_RESET_DISABLE 				0x10000000
#define H3_VMI_ENABLE						0x00000001
#define VID_IN_FORMAT			0x70     //vidInFormat register offset
#define H5_HP_INT_ENABLE               0x04000000
#define H5_HP_INTERRUPT                0x08000000
#define H5_HP_PIN                      0x00000800 //cfgSliAaMisc

#include "kmvt.h"
extern KMTVDATA  kmtvInfo;   

#include "VMIPLD.h"

#include "sliaa.h"
#include "dfp.h"


/*----------------------------------------------------------------------
Function name:  _VWIN32_PulseWin32Event

Description:    
                
Information:    uses in-line asm.

Return:         VOID
----------------------------------------------------------------------*/
VOID VXDINLINE
_VWIN32_PulseWin32Event( DWORD dwEvent )
{
    __asm mov eax,dwEvent;
    VxDCall(_VWIN32_PulseWin32Event);
}


/*----------------------------------------------------------------------
Function name:  Schedule_VM_Event

Description:    
                
Information:    uses in-line asm.

Return:         VOID
----------------------------------------------------------------------*/
VOID VXDINLINE
Schedule_VM_Event( HVM hVM, DWORD dwCallback, DWORD dwRefval )
{
    __asm mov ebx,hVM;
    __asm mov esi,dwCallback;
    __asm mov edx,dwRefval;
    VMMCall(Schedule_VM_Event);
}

#if 0
HIRQ hIRQ;
VID IRQDesc;
DWORD dwVCount;
BOOL bVMInts;
#endif

BYTE bOldBuf = 0xff;


/*----------------------------------------------------------------------
Function name:  InitializeInterrupts

Description:    Establish handler for interrupts.
                
Information:    

Return:         BOOL    TRUE for success or,
                        FALSE for failure.
----------------------------------------------------------------------*/
BOOL InitializeInterrupts( int iIRQ , PDEVTABLE pDev)
{
    // disable before grab

    pDev->bVMInts = TRUE;
    DisableInterrupts(pDev);

    // connect to interrupt

    pDev->IRQDesc.VID_IRQ_Number = iIRQ;
    pDev->IRQDesc.VID_Options = VPICD_OPT_CAN_SHARE;
    pDev->IRQDesc.VID_Hw_Int_Proc = (ULONG)InterruptHandler;
    if( pDev->hIRQ = VPICD_Virtualize_IRQ( &pDev->IRQDesc ) )
    {
        VPICD_Physically_Unmask( pDev->hIRQ );

        // now enable

        EnableInterrupts(pDev);
        return TRUE;
    }
    return FALSE;
}


/*----------------------------------------------------------------------
Function name:  InterruptsToDOS

Description:    Give up handling interrupt for dos VM.
                
Information:    Uses in-line asm.

Return:         VOID
----------------------------------------------------------------------*/
VOID 
InterruptsToDOS( VOID )
{
    __asm pushfd;
    __asm cli;
    if( pVGADevTable && pVGADevTable->hIRQ && pVGADevTable->bVMInts )
    {
        pVGADevTable->bVMInts = FALSE;
        DisableInterrupts(pVGADevTable);
        VPICD_Force_Default_Behavior( pVGADevTable->hIRQ );
        pVGADevTable->hIRQ = 0x0;
    }
    __asm popfd;
}


/*----------------------------------------------------------------------
Function name:  InterruptsToSVM

Description:    Re-initialize handler.
                
Information:    

Return:         VOID
----------------------------------------------------------------------*/
VOID InterruptsToSVM( VOID )
{
    if( pVGADevTable->IRQDesc.VID_IRQ_Number && !pVGADevTable->bVMInts )
        InitializeInterrupts( pVGADevTable->IRQDesc.VID_IRQ_Number, pVGADevTable );
}


static FxU16 LastServicedBanshee;
DWORD hInIRQ;
static DWORD dwResetCounter = 0;


/*----------------------------------------------------------------------
Function name:  InterruptHandler

Description:    Handle the interrupt.
                
Information:    Uses in-line asm.

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID 
InterruptHandler( VOID )
{
	FxI16 count;
   DWORD dwReg, sst3dRegs;
   PDEVTABLE   pDevTable;     // Ptr to device table array
   DWORD       PllReg;
   DWORD       dwOvlBufAddr;  // Overlay buffer hw address
   DWORD       dwOvlOffset;
   DWORD       dwFieldStatus;
   BYTE        bNewBuf;

    // naked prolog code
    __asm
    {
        push    ebp;
        mov     ebp,esp;
        sub     esp,__LOCAL_SIZE;
        mov     hInIRQ,eax;
//      mov     hVM,ebx;
    }

	for (count = (FxI16)dwNumDevices; --count >= 0;  )
	{
		if (++LastServicedBanshee >= dwNumDevices)
			LastServicedBanshee = 0;
		if (sst3dRegs = DevTable[LastServicedBanshee].RegBase[HWINFO_SST_IOREGS_INDEX])
		{
         PllReg = ((SstIORegs *)sst3dRegs)->pllCtrl0;
		   sst3dRegs = DevTable[LastServicedBanshee].RegBase[HWINFO_SST_3DREGS_INDEX];
			dwReg = ((SstRegs *)sst3dRegs)->intrCtrl;

         // If IO is disabled on this device
         // then the HOST/PCI Bridge will do a Master Abort
         // For a read cycle the HOST/PCI Bridge is required to return all 1's
         // This will check for this occurence 

			// VMI_IRQ_USAGE
			if ((dwReg & (0x300 | H3_VMI_INTERRUPT | H5_HP_INTERRUPT)) && (0xFFFFFFFF != PllReg))
				break;
		}
	}
   if (count < 0)
   {
      // naked epilog code, pass on this interrupt (not for us)
      __asm
      {
         stc;
         mov     esp,ebp;
         pop     ebp;
         ret;
      }
   }

   //Get the pointer to Banshee device generating the interrupt
   pDevTable = &DevTable[LastServicedBanshee];

#if 0 // KMW
    if( dwIPend & softrapen_INT_OCCURRED )   // DMA interrupt
    {
        DWORD dwSoftrap, dwQPos;

        dwSoftrap = mgaReadDWord( DWG1_SOFTRAP );
        if( !(dwSoftrap & 0x80000000) )
        {
            pMMIf->wFlags &= ~MMMIF_DMAINPROGRESS;
            // count given by register
            pMMIf->dwDMACount = dwSoftrap >> 2;
        }
        else
        {
            switch( dwSoftrap )
            {
            case MMMST_STOP:
                pMMIf->wFlags &= ~MMMIF_DMAINPROGRESS;
                // find count of completed DMA secondaries, and update
                dwQPos = mgaReadDWord( HST_PRIMADDRESS ) & ~3;
                dwQPos -= pMMIf->dwDMACQPhys; // now bytes from start of Q
                dwQPos = dwQPos/sizeof(MMDMAENTRY) + 1;
                pMMIf->dwDMACount = (pMMIf->dwDMACount + dwQPos) & 0xFFFF;
                break;

            case MMMST_LOOP:    // restart primary at start of CQ
                // Q finished, so update dma count by number of entries
                pMMIf->dwDMACount = (pMMIf->dwDMACount + MMDMAENTRIES) & 0xFFFF;
                pMMIf->DMAStop.dwVal[0] = MMMST_STOP;
                mgaWriteDWord( HST_PRIMADDRESS, pMMIf->dwDMACQPhys );
                mgaWriteDWord( HST_PRIMEND, pMMIf->dwDMACQPhys + 
                    ((MMDMAENTRIES+1)*sizeof(MMDMAENTRY))/sizeof(DWORD) );
                break;

            default:            // update free
                pMMIf->wFlags &= ~MMMIF_DMAINPROGRESS;
                break;
            }
        }
    }

    if( dwIPend & vsyncpen_INT_OCCURRED )   // vsync interrupt
    {
        BYTE bIndexSave;

        dwVCount++;

        // save CRTC index
        bIndexSave = mgaReadByte( VgaReg|VGA_CRTC_INDEX );

        // update start address if required
        if( pMMIf->wFlags & MMMIF_PLEASEFLIP )
        {
            BYTE bExtIndexSave;

            pMMIf->wFlags &= ~MMMIF_PLEASEFLIP;
            mgaWriteByte( VgaReg|VGA_CRTC_INDEX, 13); 
            mgaWriteByte( VgaReg|VGA_CRTC_DATA, pMMIf->dwFlipToOffset );
            mgaWriteByte( VgaReg|VGA_CRTC_INDEX, 12); 
            mgaWriteByte( VgaReg|VGA_CRTC_DATA, pMMIf->dwFlipToOffset >> 8 );
            bExtIndexSave = mgaReadByte( VgaReg|VGA_CRTCEXT_INDEX ); 
            mgaWriteByte( VgaReg|VGA_CRTCEXT_INDEX, 0); 
            mgaWriteByte( VgaReg|VGA_CRTCEXT_DATA, 
                (mgaReadByte( VgaReg|VGA_CRTCEXT_DATA ) & 0xf0) |
                (pMMIf->dwFlipToOffset >> 16)) ;
            mgaWriteByte( VgaReg|VGA_CRTCEXT_INDEX, bExtIndexSave);
        }
        // post event if needed
        if( pMMIf->wFlags & MMMIF_VSYNCEVENT )
        {
            Schedule_VM_Event( hSysVM, (DWORD)SignalVMEvent, 0 );
        }
        mgaWriteByte( VgaReg|VGA_CRTC_INDEX, 17); 
        mgaWriteByte( VgaReg|VGA_CRTC_DATA, 
            mgaReadByte( VgaReg|VGA_CRTC_DATA ) & ~0x30 );
        mgaWriteByte( VgaReg|VGA_CRTC_DATA, 
            mgaReadByte( VgaReg|VGA_CRTC_DATA ) | 0x10 );
        mgaWriteByte( VgaReg|VGA_CRTC_INDEX, bIndexSave );
    }

    if( dwIPend & pickpen_INT_OCCURRED )   // pick interrupt
    {
        pMMIf->wFlags |= MMMIF_PICKED;
    }

    // reset MGA interrupt sources

    mgaWriteDWord( HST_ICLEAR, dwIPend & ~vsyncpen_MASK );

#endif /* #if 0 KMW */

    // HotPlug
   if(dwReg & H5_HP_INTERRUPT)
   {
      DWORD cfgSliAaMisc;
      CONFIGRET cr;

      dwReg &= ~H5_HP_INTERRUPT;
      ((SstRegs *)sst3dRegs)->intrCtrl = (0x80000000 | dwReg);

      if(pDevTable)
      {
         cr = CM_Call_Enumerator_Function(
            pDevTable->dwDevNode,
            PCI_ENUM_FUNC_GET_DEVICE_INFO,
            CFG_SLI_AA_MISC,
            &cfgSliAaMisc,
            sizeof(DWORD),
            0);
         if(cr == CR_SUCCESS)
         {
            if(cfgSliAaMisc & H5_HP_PIN)
            {
               //Plug In

               //Set the connected bit for the control panel
               *(panelGbl(pDevTable)) = 0x00000001;

               CM_Reenumerate_DevNode(pDevTable->dwDevNode, 0);

#if 0
               //If it was plugged in turn it on.  Special for Kaymann W. 
               panelOn(pDevTable);
#endif
            }
            else
            {
               //UnPlug

               //Deactivate the panel output
               if(isPanelActive(pDevTable))
                  panelOff(pDevTable);

               //Invalidate the current edid info
               pDevTable->dfpDisp.dispWidth = 0;
               pDevTable->dfpDisp.uniqueName[0] = '\0';

               //Set the connected bit for the control panel
               *(panelGbl(pDevTable)) = 0x00000000;

               CM_Reenumerate_DevNode(pDevTable->dwDevNode, 0);
            }
         }
      }
   }
    // V3TV
    // VMI_IRQ_USAGE
	else if(dwReg & H3_VMI_INTERRUPT)   // vmi interrupt
	{
		BYTE		bVideoInterrupt = 0, bVMIStatus;
		BOOL		bReportedPolarity;
		BOOL		bPLDfield;
		BYTE		JustFinishBuf;
		BYTE		thisBuf = 3;
		BYTE		bBufModeSelect;
		DWORD		dwFieldStatus;
        KMVTBUFF  * lpKMVTBuff;

		// clear vmi interrupt - always do this
		dwReg &= ~H3_VMI_INTERRUPT;
		((SstRegs *)sst3dRegs)->intrCtrl = (0x80000000 | dwReg);

#ifdef danofix
//Note: There is a hole during initialization where an interrupt can occur before initialization of 
//      the following pointers is complete.  I fell into this trap when using a proto Voodoo4 4500 PCI.
// Check to make sure that lpDriverData is not bogus
   if (( pDevTable ) && (pDevTable->lpDriverData))
   {
#endif //def danofix
		// set up data and see if enough fields for stable video

        lpKMVTBuff = (KMVTBUFF *)GETGBL_KMVTBuff((DWORD)pDevTable->lpDriverData);

		if (lpKMVTBuff->dwCounter < 30)		// throw out first 30 fields
		{
			bVideoInterrupt = 2;
			lpKMVTBuff->dwCounter++;
			lpKMVTBuff->bSawVBIintr = FALSE;
			dwResetCounter = 0;
		}
		else
			bVideoInterrupt = 1;
        dwFieldStatus = *(DWORD*)(pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX] + H4_VID_IN_STATUS_CURLINE);

		bReportedPolarity = (BOOL)((dwFieldStatus >> 18) & 0x01);
        JustFinishBuf = (BYTE) ((dwFieldStatus >> 16 ) & 0x3);		// not valid when manual flipping is done
		bBufModeSelect = (BYTE)((*(DWORD*)(pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX] + VID_IN_FORMAT)) >> 9) & 0x3;
		if (GETGBL_dwOvlOffset((DWORD)kmtvInfo.pDev->lpDriverData) 
		   && (pVGADevTable->dwIMask & H3_VSYNC_INT_ENABLE) 
		   &&  (lpKMVTBuff->dwStatus & INTERLEAVE_ON ))
		{	// manual flipping
			JustFinishBuf = thisBuf = (BYTE)(lpKMVTBuff->dwStatus & BUFFER_IN_USE_MASK);
			if (JustFinishBuf)
				JustFinishBuf--;
			else
				JustFinishBuf = (BYTE)((lpKMVTBuff->dwStatus & VP_BUFF_MASK)>> VP_BUFF_SHIFT) - 1;
		}
		if (!lpKMVTBuff->bVMIPLDinUse)
		{
			lpKMVTBuff->dwLastBufferFilled = JustFinishBuf;	
			lpKMVTBuff->bLastFieldEven = bReportedPolarity;
		}
		else
		{
			bVMIStatus = VMIPLD_ReadStatus((DWORD)pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX], 
				 VDD_VMIRead );
            lpKMVTBuff->bLastIntrVBI = (bVMIStatus & VMI_WASVBIINTR)? TRUE: FALSE;
            bPLDfield = (bVMIStatus & VMI_WASEVENFIELD)? TRUE: FALSE;

			if (lpKMVTBuff->bLastIntrVBI)
				lpKMVTBuff->bSawVBIintr = TRUE;
			if (lpKMVTBuff->bSawVBIintr && lpKMVTBuff->bLastIntrVBI)
			{
				// handle post vbi interrupt
				bVideoInterrupt = 0;
				lpKMVTBuff->dwLastBufferFilled = JustFinishBuf;			// always valid on VBI
				lpKMVTBuff->bLastFieldEven = !bPLDfield;//bReportedPolarity;
			}
			else if (lpKMVTBuff->bSawVBIintr && !lpKMVTBuff->bLastIntrVBI)
			{
				// handle post video interrupt
				// JustFinishBuf is on last vref - which may not have occured
				/*
				lpKMVTBuff->bLastFieldEven = !lpKMVTBuff->bLastFieldEven;
				// this breaks is the manual flip is going on - see getcurrentautoflip in kmtv.c
				if (!(lpKMVTBuff->dwVidInFormat & H3_VMI_DEINTERLACE_WEAVE)		// changes on every field
					|| lpKMVTBuff->bLastFieldEven)								// both change on even field
				{
					lpKMVTBuff->dwLastBufferFilled++;
					if (lpKMVTBuff->dwVidInFormat & H3_VMI_TRIPLE_BUFFER)
					{
						if (lpKMVTBuff->dwLastBufferFilled > 2)
							lpKMVTBuff->dwLastBufferFilled = 0;
					}
					else if (lpKMVTBuff->dwVidInFormat & H3_VMI_DOUBLE_BUFFER)
					{
						if (lpKMVTBuff->dwLastBufferFilled > 1)
							lpKMVTBuff->dwLastBufferFilled = 0;
					}
					else
						lpKMVTBuff->dwLastBufferFilled = 0;
				}
				*/
			}
		}
		// do actions of interest on video port VREF rising edge

		if ((lpKMVTBuff->bVMIPLDinUse && bVideoInterrupt == 0)	// do it on vbi end
			|| (!lpKMVTBuff->bVMIPLDinUse && bVideoInterrupt == 1))		// do in on vint
		{
			kmtvInfo.dwFieldCount++;
#ifdef DEBUG
			if (kmtvInfo.dwFieldCount && kmtvInfo.dwFieldCount >= 180 && kmtvInfo.dwFieldCount < 200)
			{
				Debug_Printf (" irq field %d, hwpol %d, lastfield %d, pldfield %d hwbuf %d buf %d \n",
					kmtvInfo.dwFieldCount, 
					bReportedPolarity, lpKMVTBuff->bLastFieldEven, bPLDfield,
					JustFinishBuf, lpKMVTBuff->dwLastBufferFilled);
			}
#endif
			if (kmtvInfo.IRQCallback && (kmtvInfo.dwIRQSources & DDIRQ_VPORT0_VSYNC))
			{
			   _asm {
				  push ebp
				  push esi
				  push eax
				  push ebx
				  mov eax, DDIRQ_VPORT0_VSYNC
				  mov ebx, kmtvInfo.Context
				  call [kmtvInfo.IRQCallback]
				  pop ebx
				  pop eax
				  pop esi
				  pop ebp          
			   } 
			}
		}
		if (kmtvInfo.IRQCallback)
		{
#ifdef USE_EVENT_TRANSFER
#ifdef USE_VMM_CALLBACK_IRQ
			if ( kmtvInfo.dwTransferID) 
#else
			if ( kmtvInfo.fTransferFull == 1) 
#endif
#else
			if ( kmtvInfo.dwTransferID) 
#endif
			{
			   _asm {
				  push ebp
				  push esi
				  push eax
				  push ebx
				  mov eax, DDIRQ_BUSMASTER
				  mov ebx, kmtvInfo.Context
				  call [kmtvInfo.IRQCallback]
				  pop ebx
				  pop eax
				  pop esi
				  pop ebp          
			   } 
			}
		}

//#ifdef THEBAD_SILICON_RETURNS
		// see if we need to reset this sucker
		if (bVideoInterrupt == 1 && lpKMVTBuff->bUseReset) 
		{
			BOOL bResetIt = FALSE;
			if (++dwResetCounter >= 30)// && !lpKMVTBuff->bLastFieldEven)		// every 1/2 secs (120 fields)
			{
				bResetIt = TRUE;
				dwResetCounter = 0;
			}

			if (bResetIt)
			{
				DWORD dwVIReg;
				// reset vmi to clear fifo - and kill color problem
				dwVIReg = *(volatile DWORD*)(pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX] + VID_SERIAL_PARALLEL_PORT);
				dwVIReg &= ~(H3_VMI_RESET_DISABLE | H3_VMI_ENABLE);
				*(volatile DWORD*)(pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX] + VID_SERIAL_PARALLEL_PORT) = dwVIReg;
				dwVIReg |= H3_VMI_RESET_DISABLE;
				*(volatile DWORD*)(pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX] + VID_SERIAL_PARALLEL_PORT) = dwVIReg;
		
				if (lpKMVTBuff->bVMIPLDinUse)
				{
					VMIPLD_SetCommand((DWORD)pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX], 
							VMI_VBIINT |							// not passthru - 656 mode
							VMI_VBICROP,						// always need for last buffer.
							VDD_VMIRead,
							VDD_VMIWrite);

					VMIPLD_SetVbiMax((DWORD)pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX], 1,VDD_VMIWrite);		// vbi is actually in video
					VMIPLD_SetVidMax((DWORD)pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX], lpKMVTBuff->dwPLDVidLines,VDD_VMIWrite);
				}
				else
					VMIPLD_SetCommand((DWORD)pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX], 
							VMI_PASSTHRU,
							VDD_VMIRead,
							VDD_VMIWrite);	// passthru, no vbi int, no crop vbi

				dwVIReg |= H3_VMI_ENABLE;
				*(volatile DWORD*)(pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX] + VID_SERIAL_PARALLEL_PORT) = dwVIReg;
			}
		}
#ifdef danofix
}
#endif //def danofix
	}

    //Overlay flipping
    //------------------------------------------------------------------------------
    // Check to make sure that lpDriverData is not bogus also
    else if (( pDevTable ) && (pDevTable->lpDriverData))
    {
       //If overlay offset is not zero do manual flipping
       if ( (dwOvlOffset = GETGBL_dwOvlOffset(pDevTable->lpDriverData)) )
       {
          BOOL fFlipOVL = TRUE;

          dwFieldStatus = *(DWORD*)(pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX] + H4_VID_IN_STATUS_CURLINE);
          bNewBuf = (BYTE) ((dwFieldStatus >> 16 ) & 0x3);
          dwFieldStatus &= 0x40000;

          switch ( bNewBuf )
          {
            case 0:
               dwOvlBufAddr = *(DWORD*)(pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX] + H3_VID_IN_ADDR0);
               break;
            case 1:
               dwOvlBufAddr = *(DWORD*)(pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX] + H3_VID_IN_ADDR1);
               break;
            case 2:
               dwOvlBufAddr = *(DWORD*)(pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX] + H3_VID_IN_ADDR2);
               break;
            default:    
               dwOvlBufAddr = 0;
               break;
          }

          dwOvlOffset &= 0x7FFFFFFF;
              
          if ( dwOvlBufAddr && bNewBuf != bOldBuf )
          {
            KMVTBUFF * lpKMVTBuff;

            lpKMVTBuff = (KMVTBUFF *)GETGBL_KMVTBuff(pDevTable->lpDriverData);
            dwOvlBufAddr += dwOvlOffset;
            
            if(lpKMVTBuff && (lpKMVTBuff->dwStatus & INTERLEAVE_ON ))
            {
               DWORD dwVPAddress;
               DWORD dwTotalBuffs;
               DWORD dwCurrentBuff,dwNextBuff;
               //display interleaved BOB for video port
               //at this time only two vport addresses are used

               dwTotalBuffs =
                  (lpKMVTBuff->dwStatus & VP_BUFF_MASK)>> VP_BUFF_SHIFT;

               dwCurrentBuff = lpKMVTBuff->dwStatus & BUFFER_IN_USE_MASK;

               dwNextBuff = (dwCurrentBuff +1) % dwTotalBuffs;
               //swap to the new address
               dwVPAddress =*( &(lpKMVTBuff->dwVPAddr0) + dwNextBuff);

               if(!dwFieldStatus)
               {
                  //odd field is done, point *dwVPAddress to next line
                  dwVPAddress += lpKMVTBuff->dwVPStride;
                  //if not bob and pure interleaved mode
                  //only flip overlay at even field
                  if(!(lpKMVTBuff->dwStatus & BOB_ON ))
                     fFlipOVL = FALSE;

				  // keep address of last odd field buffer filled
				  kmtvInfo.dwFirstFieldBuffer = dwCurrentBuff;

                  //update current buff to dwNextBuff
                  //V3TV lpKMVTBuff->dwStatus &= ~BUFFER_IN_USE_MASK;
                  //V3TV lpKMVTBuff->dwStatus |= dwNextBuff;

               }
			   else
			   {
				                     //update current buff to dwNextBuff after even
                  lpKMVTBuff->dwStatus &= ~BUFFER_IN_USE_MASK;
                  lpKMVTBuff->dwStatus |= dwNextBuff;
			   }


               if(fFlipOVL)
               {
                  //make sure the offset doesn't go to the next field
                  if( (dwOvlOffset / lpKMVTBuff->dwVPStride) & 1 )
                  {
                     dwOvlBufAddr += lpKMVTBuff->dwVPStride;
                  }
               }

               switch ( bNewBuf )
               {
                    case 0:
                     *(DWORD*)(pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX] + H3_VID_IN_ADDR0) = dwVPAddress;
                    break;
                    case 1:
                     *(DWORD*)(pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX] + H3_VID_IN_ADDR1) = dwVPAddress;
                    break;
                    case 2:
                     *(DWORD*)(pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX] + H3_VID_IN_ADDR2) = dwVPAddress;
                    break;

               }
            }

            if( !(*(DWORD*)(pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX] + H3_VID_PROC_CFG) &
             SST_OVERLAY_DEINTERLACE_EN))
            dwFieldStatus = 0;      //don't set even/odd field if not BOB

            //Direct write to swap overlay buffer
            if(fFlipOVL)
            {
              if(dwFieldStatus)
              ((SstRegs *)(pVGADevTable->RegBase[HWINFO_SST_3DREGS_INDEX]))->leftOverlayBuf
                = dwOvlBufAddr |0x80000000;     //set evne/odd field
              else
              ((SstRegs *)(pVGADevTable->RegBase[HWINFO_SST_3DREGS_INDEX]))->leftOverlayBuf = dwOvlBufAddr;
            ((SstRegs *)(pVGADevTable->RegBase[HWINFO_SST_3DREGS_INDEX]))->swapbufferCMD = 0;
            }

            
            bOldBuf = bNewBuf;
          }   
       }

           
    //------------------------------------------------------------------------------
	// clear vertical interrupts only
	((SstRegs *)sst3dRegs)->intrCtrl = 0x80000000 | (dwReg & ~0x300);

#ifdef REAL_NET
   if (dwReg & H3_VSYNC_INTERRUPT)
      {
      pDevTable->dwVCount++;
      }
#endif

	} //V3TV else

    // ack it and exit
    VPICD_Phys_EOI (hInIRQ);
    __asm
    {
        clc;
        mov     esp,ebp;
        pop     ebp;
        ret;
    }
}


/*----------------------------------------------------------------------
Function name:  SignalVMEvent

Description:    
                
Information:    

Return:         VOID
----------------------------------------------------------------------*/
VOID
SignalVMEvent( VOID )
{
#ifdef KMW
    _VWIN32_PulseWin32Event( pMMIf->dwVSyncEvent );
#endif // #ifdef KMW
}

/*----------------------------------------------------------------------
Function name:  EnableInterrupts

Description: Used to Physical Enable Interrupts on the 3DFX Device    
                
Information:    

Return:         
----------------------------------------------------------------------*/
VOID EnableInterrupts(PDEVTABLE pDevTable)
{
   DWORD dwReg;
   DWORD sst3dRegs = pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX];
   DWORD PllReg;         

   PllReg = ((SstIORegs *)sst3dRegs)->pllCtrl0;

   // If IO is disabled on this device
   // then the HOST/PCI Bridge will do a Master Abort
   // For a read cycle the HOST/PCI Bridge is required to return all 1's
   // This will check for this 
   if (0xFFFFFFFF == PllReg)    
      {
      // This is not good ---
      // Why would we try to do something on a card that is disabled?
#ifdef DEBUG
      _asm {int 03}
#endif
      return;
      }

   sst3dRegs = pDevTable->RegBase[HWINFO_SST_3DREGS_INDEX];
	dwReg = ((SstRegs *)sst3dRegs)->intrCtrl;

   // This is critical code
   // We need to make this a atomic operation since the flags
   // can get modified on a asynchronous basis
   __asm pushfd;
   DISABLE_INTERRUPTS();
   dwReg |= pDevTable->dwIMask;
   
   ((SstRegs *)sst3dRegs)->intrCtrl = dwReg;
   __asm popfd;
}

/*----------------------------------------------------------------------
Function name:  DisableInterrupts

Description: Used to Physical Disable Interrupts on the 3DFX Device    
                
Information:    

Return:         
----------------------------------------------------------------------*/
VOID DisableInterrupts(PDEVTABLE pDevTable)
{
   DWORD dwReg;
   DWORD sst3dRegs = pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX];
   DWORD PllReg;         

   PllReg = ((SstIORegs *)sst3dRegs)->pllCtrl0;

   // If IO is disabled on this device
   // then the HOST/PCI Bridge will do a Master Abort
   // For a read cycle the HOST/PCI Bridge is required to return all 1's
   // This will check for this 
   if (0xFFFFFFFF == PllReg)    
      {
      // This is not good ---
      // Why would we try to do something on a card that is disabled?
#ifdef DEBUG
      _asm {int 03}
#endif
      return;
      }

   sst3dRegs = pDevTable->RegBase[HWINFO_SST_3DREGS_INDEX];
	dwReg = ((SstRegs *)sst3dRegs)->intrCtrl;

   dwReg &= ~(H3_IMASK);
   ((SstRegs *)sst3dRegs)->intrCtrl = dwReg;
}


