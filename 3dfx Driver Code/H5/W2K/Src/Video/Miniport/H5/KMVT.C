typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long ULONG;
typedef int     BOOL;
#define INITGUID

#define FULL_DRIVER_COMPILE
#ifdef FULL_DRIVER_COMPILE
#include "ntddk.h"
// some definitions to make video.h compile without miniport.h
typedef PVOID	PEMULATOR_ACCESS_ENTRY;
typedef PVOID	PBANKED_SECTION_ROUTINE;	// needed for 3.51
#ifdef	_X86_
#undef ALLOC_PRAGMA
#endif
#if DBG
#undef PAGED_CODE
#endif
#endif

//do not use on Pentium 2 system - only works on p3
//#define Fence   {__asm _emit 0x0F __asm _emit 0xAE __asm _emit 0xF8}

volatile ULONG   p6fenceLoc;
#define P6_FENCE 	{ __asm xchg eax, p6fenceLoc }

#include "dderror.h"
#include "devioctl.h"

#ifndef FULL_DRIVER_COMPILE
#include "miniport.h"
#endif
#include "ntddvdeo.h"
#include "video.h"
#include "dxmini.h"
#include "h3.h"
#include "ds_i2c.h"
#define NTSTATUS DWORD

#include "i2cgpio.h"
#include "kmvt.h"
#include "vmipld.h"
#include "dfp.h"
//???#include "sliaa.h"
typedef I2CINTERFACE *PI2CINTERFACE;
#define I2C_DEVINFOPTR(pcontext)  ((PI2C_DEVINFO)&(((PHW_DEVICE_EXTENSION)(pcontext))->i2cInfo))

GUID GUID_MDxApi = { 0x8a79bef0, 0xb915, 0x11d0, 0x91, 0x44, 0x08, 0x00, 0x36, 0xd2, 0xef, 0x02};

static DWORD __stdcall DDGetPolarity(PVOID,PDDGETPOLARITYININFO,PDDGETPOLARITYOUTINFO);
static DWORD __stdcall DDGetIrqInfo(PVOID HWExtension, PVOID Input, PDDGETIRQINFO lpGetIrqInfo);
static DWORD __stdcall DDEnableIrq(PVOID HWExtension, PDDENABLEIRQINFO lpEnableIrq, PVOID Output);
static DWORD __stdcall DDSkipNextField(PVOID HWExtension, PDDSKIPNEXTFIELDINFO lpSkipInfo,PVOID pOutput);
static DWORD __stdcall DDBobNextField(PVOID HWExtension, PDDBOBNEXTFIELDINFO lpBobInfo,PVOID pOutput);
static DWORD __stdcall DDSetState(PVOID HWExtension, PDDSETSTATEININFO lpStateIn,PDDSETSTATEOUTINFO lpStateOut);
static DWORD __stdcall DDLock(PVOID HWExtension, PDDLOCKININFO lpLockInInfo,PDDLOCKOUTINFO lpLockOutInfo);
//static DWORD __stdcall DDFlipOverlay(PVOID HWExtension,PDDFLIPOVERLAYINFO lpFlipInfo,PVOID pOutput);
static DWORD __stdcall DDFlipVideoPort(PVOID HWExtension, PDDFLIPVIDEOPORTINFO lpFlipVideoPort,PVOID pOutput);
static DWORD __stdcall DDGetCurrentAutoflip(PVOID HWExtension, PDDGETCURRENTAUTOFLIPININFO lpAutoFlipIn,PDDGETCURRENTAUTOFLIPOUTINFO lpAutoFlipOut);
static DWORD __stdcall DDGetPreviousAutoflip(PVOID HWExtension, PDDGETPREVIOUSAUTOFLIPININFO lpAutoFlipIn,PDDGETPREVIOUSAUTOFLIPOUTINFO lpAutoFlipOut);
static DWORD __stdcall DDTransfer(PVOID HWExtension, PDDTRANSFERININFO lpTransferIn,PDDTRANSFEROUTINFO lpTransferOut);
static DWORD __stdcall DDGetTransferStatus(PVOID HWExtension, PVOID pInput,PDDGETTRANSFEROUTINFO lpTransferStatus);

NTSTATUS STDMETHODCALLTYPE I2cOpen (PDEVICE_OBJECT pDev, ULONG fOpenClose, PI2CControl pI2cControl);
NTSTATUS STDMETHODCALLTYPE I2cAccess (PDEVICE_OBJECT pDev, PI2CControl pControl);

DWORD TransferVMEvent( DWORD refData, PHW_DEVICE_EXTENSION HwDeviceExtension );
BYTE AddTransfer(DDTRANSFERININFO *lpTransferIn, DWORD *dwCurrentIndex, DWORD polarity, PHW_DEVICE_EXTENSION HwDeviceExtension);

//prototype and define for VRSetRegistry to fix PAL registry bug
VP_STATUS VRSetRegistry (PHW_DEVICE_EXTENSION HwDeviceExtension); 
#define V3TV_VPX3225D               0x7230  //copied from power.c

KMVTDATA kmvtInfo;
#define MAKE_FOURCC( ch0, ch1, ch2, ch3 )                       \
        ( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |    \
        ( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#define FOURCC_RAW8	    MAKE_FOURCC('R','A','W','8')
#define H3_VMI_DEINTERLACE_WEAVE			0x00000010
#define DEBUG_PRINT

/*************************************************************************
*  Get interface from MiniPort.
*
*  The KMVT interface in miniport is get from here.
*  Note: we must at least support one KMVT function to enable VPE
*        right now DDGetPolarity is enabled.
*
*************************************************************************/
VP_STATUS QueryDriverInterface(
	PHW_DEVICE_EXTENSION hwDeviceExtension,
    PQUERY_INTERFACE QueryInterface)

{
    PDXAPI_INTERFACE DxApiInterface;
    VP_STATUS status;
 
    status = DXERR_UNSUPPORTED;
 
    if (IsEqualGUID(QueryInterface->InterfaceType, &GUID_MDxApi))
    {
        DxApiInterface = (PDXAPI_INTERFACE) QueryInterface->Interface;
 
        //So far we only support DDGetPolarity to let VPE work
 
        DxApiInterface->Size    = sizeof(DXAPI_INTERFACE);
        DxApiInterface->Version = DXAPI_HALVERSION;
        DxApiInterface->Context = hwDeviceExtension;
 
        DxApiInterface->DxGetIrqInfo          = DDGetIrqInfo;
        DxApiInterface->DxEnableIrq           = DDEnableIrq;
        DxApiInterface->DxSkipNextField       = DDSkipNextField;
        DxApiInterface->DxBobNextField        = DDBobNextField;
        DxApiInterface->DxSetState            = DDSetState;
		DxApiInterface->DxLock				  = DDLock;
        //DxApiInterface->DxFlipOverlay         = DDFlipOverlay;
        DxApiInterface->DxFlipVideoPort       = DDFlipVideoPort;
		DxApiInterface->DxTransfer			  = DDTransfer;
		DxApiInterface->DxGetTransferStatus   = DDGetTransferStatus;
        DxApiInterface->DxGetPolarity         = DDGetPolarity;
        DxApiInterface->DxGetCurrentAutoflip  = DDGetCurrentAutoflip;
        DxApiInterface->DxGetPreviousAutoflip = DDGetPreviousAutoflip;
        status = DX_OK;
    } 

	else if (IsEqualGUID(QueryInterface->InterfaceType, &GUID_I2C_INTERFACE))
	{
        PI2CINTERFACE pXInterface = (PI2CINTERFACE)(QueryInterface->Interface);
 
		//maybe add size to queryinterface data structure???
		pXInterface->_vddInterface.Size = sizeof(I2CINTERFACE);
		pXInterface->_vddInterface.Version = 0;
		pXInterface->i2cOpen = I2cOpen;
		pXInterface->i2cAccess = I2cAccess;
		pXInterface->_vddInterface.Context = (I2CCONTEXT)hwDeviceExtension;
		status = DX_OK;
	}


 
    return status;
}


#if ENABLE_IRQ
#define H3_VSYNC_INT_ENABLE					0x00000004
#define H3_VMI_RESET_DISABLE 				0x10000000
#define H3_VMI_ENABLE						0x00000001
#define H3_VMI_INTERRUPT					0x00800000
#define H3_VSYNC_INTERRUPT					0x00000100
#define H3_VMI_TRIPLE_BUFFER				0x00000400
#define H3_VMI_DOUBLE_BUFFER				0x00000200
//static DWORD dwResetCounter = 0;

/**************************************************************************
* H3VidInterrupt:
*
*  Chip Interrupt handler.
*  For video port overlay display, this function use overlay address to
*  to crop video data and display BOB mode.
*
*
**************************************************************************/
BOOLEAN H3VidInterrupt(
	PHW_DEVICE_EXTENSION HwDeviceExtension
    )
{
   PH3_MEMBASE0 sstIORegs = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
#if REDUCED_MEMORY_MAPPINGS
   PH3_3D_REGISTERS sst3DRegs = (PH3_3D_REGISTERS) ((UCHAR *) HwDeviceExtension->MappedAddress[SST_3D_REGS_INDEX]);
#else
   PH3_3D_REGISTERS sst3DRegs = (PH3_3D_REGISTERS) ((UCHAR *) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX] + SST_3D_OFFSET);
#endif
   DWORD       PllReg,dwReg;

   PllReg = sstIORegs->pllCtrl0;
   dwReg = sst3DRegs->intrCtrl;

     // If IO is disabled on this device
     // then the HOST/PCI Bridge will do a Master Abort
     // For a read cycle the HOST/PCI Bridge is required to return all 1's
      // This will check for this occurence 


   if (!(dwReg & (0x300 | H3_VMI_INTERRUPT | H5_HP_INTERRUPT)) || (0xFFFFFFFF == PllReg))   
	   return FALSE;

   // check for HotPlug interrupt
   if(dwReg & H5_HP_INTERRUPT)
   {

       // clear hotplug interrupt
       dwReg &= ~H5_HP_INTERRUPT;
       sst3DRegs->intrCtrl = (0x80000000 | dwReg);

       // send DPC 
       VideoPortQueueDpc(HwDeviceExtension,
                                ProcessHotPlugEvent,
                                NULL);
   }
   else
   {
    // VMI_IRQ_USAGE
	if(dwReg & H3_VMI_INTERRUPT)   // vmi interrupt
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
		sst3DRegs->intrCtrl = (0x80000000 | dwReg);

		// set up data and see if enough fields for stable video
        lpKMVTBuff = HwDeviceExtension->KMVTBuff;
		if (lpKMVTBuff->dwCounter < 30)		// throw out first 30 fields
		{
			bVideoInterrupt = 2;
			lpKMVTBuff->dwCounter++;
			lpKMVTBuff->bSawVBIintr = FALSE;
			lpKMVTBuff->dwResetCounter = 0;
		}
		else
			bVideoInterrupt = 1;
        dwFieldStatus = sstIORegs->vidCurrentLine;

		bReportedPolarity = (BOOL)((dwFieldStatus >> 18) & 0x01);
        JustFinishBuf = (BYTE) ((dwFieldStatus >> 16 ) & 0x3);		// not valid when manual flipping is done
		bBufModeSelect = (BYTE)(sstIORegs->vidInFormat >> 9) & 0x3;

		if (lpKMVTBuff->dwOVLOffset 
		   && (lpKMVTBuff->dwIMask & H3_VSYNC_INT_ENABLE) 
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
			lpKMVTBuff->bLastFieldEven = (BOOLEAN)bReportedPolarity;
		}
		else
		{
			bVMIStatus = VMIPLD_ReadStatus((DWORD)sstIORegs, VDD_VMIRead );
            lpKMVTBuff->bLastIntrVBI = (bVMIStatus & VMI_WASVBIINTR)? TRUE: FALSE;
            bPLDfield = (bVMIStatus & VMI_WASEVENFIELD)? TRUE: FALSE;

			if (lpKMVTBuff->bLastIntrVBI)
				lpKMVTBuff->bSawVBIintr = TRUE;
			if (lpKMVTBuff->bSawVBIintr && lpKMVTBuff->bLastIntrVBI)
			{
				// handle post vbi interrupt
				if (bVideoInterrupt != 2)
					bVideoInterrupt = 0;
				//handle case of vidcurrentline returning invalid buffer 3 (
				if (JustFinishBuf==3)
				{
					lpKMVTBuff->dwLastBufferFilled++;
					if (lpKMVTBuff->dwLastBufferFilled==3)
						lpKMVTBuff->dwLastBufferFilled=0;
				} else 
					lpKMVTBuff->dwLastBufferFilled = JustFinishBuf;			// always valid on VBI
				lpKMVTBuff->bLastFieldEven = (BOOLEAN)bPLDfield;//bReportedPolarity;
			}
			else if (lpKMVTBuff->bSawVBIintr && !lpKMVTBuff->bLastIntrVBI)	
			{

				// handle post video interrupt
				// JustFinishBuf is on last vref - which may not have occured
				/*				
				bPLDfield = 
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
				bVideoInterrupt = 1;
				*/
				
			}
		}
		// do actions of interest on video port VREF rising edge
		if ((lpKMVTBuff->bVMIPLDinUse && bVideoInterrupt == 0)	// do it on vbi end
			|| (!lpKMVTBuff->bVMIPLDinUse && bVideoInterrupt == 1))		// do in on vint
		{
			lpKMVTBuff->dwFieldCount++;
#ifdef DEBUG_PRINT
			if (lpKMVTBuff->dwFieldCount && lpKMVTBuff->dwFieldCount >= 180 && lpKMVTBuff->dwFieldCount < 300)
			{
				VideoDebugPrint ((0," irq field %d, hwpol %d, lastfield %d, pldfield %d hwbuf %d buf %d \n",
					lpKMVTBuff->dwFieldCount, 
					bReportedPolarity, lpKMVTBuff->bLastFieldEven, bPLDfield,
					JustFinishBuf, lpKMVTBuff->dwLastBufferFilled));
			}
#endif
			if (HwDeviceExtension->pfnIrqCallback && 
			//	!lpKMVTBuff->bLastFieldEven &&
				(HwDeviceExtension->dwIRQSources & DDIRQ_VPORT0_VSYNC))
			{
				((PDX_IRQDATA)HwDeviceExtension->lpIRQData)->dwIrqFlags = DDIRQ_VPORT0_VSYNC;
				((PDX_IRQCALLBACK)(HwDeviceExtension->pfnIrqCallback))(((PDX_IRQDATA)HwDeviceExtension->lpIRQData));
			}
		}
		if (HwDeviceExtension->pfnIrqCallback)
		{
			if ( lpKMVTBuff->fTransferFull==1) 
			{
				((PDX_IRQDATA)HwDeviceExtension->lpIRQData)->dwIrqFlags = DDIRQ_BUSMASTER;
   	VideoDebugPrint((0, "<B"));
				((PDX_IRQCALLBACK)(HwDeviceExtension->pfnIrqCallback))(((PDX_IRQDATA)HwDeviceExtension->lpIRQData));
   	VideoDebugPrint((0, ">"));
			}
		}

//#ifdef THEBAD_SILICON_RETURNS
		// see if we need to reset this sucker
		if ((IS_VOODOO3) && (bVideoInterrupt == 1) && lpKMVTBuff->bUseReset) 
		{
			BOOL bResetIt = FALSE;
			if (++(lpKMVTBuff->dwResetCounter) >= 30)// && !lpKMVTBuff->bLastFieldEven)		// every 1/2 secs (120 fields)
			{
				bResetIt = TRUE;
				lpKMVTBuff->dwResetCounter = 0;
			}

			if (bResetIt)
			{
				DWORD dwVIReg;
				DWORD dwStatus;
				// reset vmi to clear fifo - and kill color problem
				dwVIReg = sstIORegs->vidSerialParallelPort;
  				dwVIReg &= ~(H3_VMI_RESET_DISABLE | H3_VMI_ENABLE);

				//Loop until pci fifo is not busy - without this we could
				//corrupt cmd fifo by doing the reset
				//Fence;
				P6_FENCE;
#if 0
				dwStatus = sstIORegs->status;
				if ((dwStatus & 0x3F) != 0x1F) 
				{
					lpKMVTBuff->dwResetCounter = 30;
					return TRUE;
				}
#endif

				while ((sstIORegs->status & 0x3F) != 0x1F);
				//punt if cmdfifo is busy and do on next interrupt
				if (sstIORegs->status & 0x1C00)
				{
					lpKMVTBuff->dwResetCounter = 30;
					return TRUE;
				}
  				sstIORegs->vidSerialParallelPort = dwVIReg;
  				dwVIReg |= H3_VMI_RESET_DISABLE;
  				sstIORegs->vidSerialParallelPort = dwVIReg;
				if (lpKMVTBuff->bVMIPLDinUse)
				{
					VMIPLD_SetCommand((DWORD)sstIORegs, 
							VMI_VBIINT |							// not passthru - 656 mode
							VMI_VBICROP,						// always need for last buffer.
							VDD_VMIRead,
							VDD_VMIWrite);

					VMIPLD_SetVbiMax((DWORD)sstIORegs, 1,VDD_VMIWrite);		// vbi is actually in video
					VMIPLD_SetVidMax((DWORD)sstIORegs, lpKMVTBuff->dwPLDVidLines,VDD_VMIWrite);
				}
				else
					VMIPLD_SetCommand((DWORD)sstIORegs, 
							VMI_PASSTHRU,
							VDD_VMIRead,
							VDD_VMIWrite);	// passthru, no vbi int, no crop vbi

				dwVIReg |= H3_VMI_ENABLE;
				sstIORegs->vidSerialParallelPort = dwVIReg;
			}
		}
//#endif
	}

    else  {

   //	VideoDebugPrint((0, "I"));

    //Overlay flipping
    //------------------------------------------------------------------------------
    // Check to make sure  KMVTBuff is here
    if(HwDeviceExtension->KMVTBuff )
    {
        DWORD       dwOvlOffset, dwOvlBufAddr;
        DWORD       dwFieldStatus;
        DWORD       dwNewBuf,dwOldBuff;
        BOOL fFlipOVL = TRUE;
        KMVTBUFF * lpKMVTBuff;
        
         dwFieldStatus = sstIORegs->vidCurrentLine;
         dwNewBuf = ((dwFieldStatus >> 16 ) & 0x3);

         dwFieldStatus &= 0x40000;
         lpKMVTBuff =HwDeviceExtension->KMVTBuff;


         switch ( dwNewBuf )
         {
            case 0:
               dwOvlBufAddr = sstIORegs->vidInAddr0;
               break;
            case 1:
               dwOvlBufAddr = sstIORegs->vidInAddr1;
               break;
            case 2:
               dwOvlBufAddr = sstIORegs->vidInAddr2;
               break;
            default:    
               dwOvlBufAddr = 0;
               break;
          }

          dwOvlOffset = lpKMVTBuff->dwOVLOffset;

          dwOvlOffset &= 0x7FFFFFFF;
              

          if ( dwOvlBufAddr && (dwNewBuf != lpKMVTBuff->dwOldBuf))
          {
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
				  lpKMVTBuff->bFirstFieldBuffer = (BYTE) dwCurrentBuff;
			   } else   {
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

               switch ( dwNewBuf )
               {
                    case 0:
                        sstIORegs->vidInAddr0  = dwVPAddress;
                    break;
                    case 1:
                        sstIORegs->vidInAddr1  = dwVPAddress;
                    break;
                    case 2:
                        sstIORegs->vidInAddr2 = dwVPAddress;
                    break;
               }
            }

            if( !sstIORegs->vidProcCfg &  SST_OVERLAY_DEINTERLACE_EN)
                 dwFieldStatus = 0;      //don't set even/odd field if not BOB

            //Direct write to swap overlay buffer
            if(fFlipOVL)
            {
              if(dwFieldStatus)
                sst3DRegs->leftOverlayBuf
                = dwOvlBufAddr |0x80000000;     //set evne/odd field
              else
                sst3DRegs->leftOverlayBuf = dwOvlBufAddr;
              sst3DRegs->swapbufferCMD = 0;
            }

            
            lpKMVTBuff->dwOldBuf = dwNewBuf;
          }   
    }           
    //------------------------------------------------------------------------------
	// clear vertical interrupts only
	sst3DRegs->intrCtrl = 0x80000000 | (dwReg & ~0x300);
	}

    }
    return TRUE;
 
}

#endif

#if  KMVT || ENABLE_IRQ
/*****************************************************************
*
*  H3AllocatKMBuff:
*
*  Allocate and Free KMVTBUFF memory, which is shared with display
*  driver.
*
*  if the first parameter is zero free the memory
*  otherwise allocate the memory
*
*******************************************************************/
VP_STATUS H3AllocatKMBuff(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket)
{
#ifdef MS_VIEW
	PPHYSICAL_ADDRESS  LogicalAddress = { 0 };
#endif

     if ((RequestPacket->OutputBufferLength <  4)
		||  (RequestPacket->InputBufferLength < 4))
	 {
			return ERROR_INSUFFICIENT_BUFFER;
     }

     if( *(DWORD *)(RequestPacket->InputBuffer))
     {
        //allocate memory
        if(! HwDeviceExtension->KMVTBuff )
        {
         //	VideoDebugPrint((0, "KMVT Alloc_KMVT_MEMORY\n"));

#ifdef MS_VIEW
          HwDeviceExtension->KMVTBuff = VideoPortGetCommonBuffer ( HwDeviceExtension, 
          							 							   sizeof( KMVTBUFF), 
                                                                   0,
                                                                   LogicalAddress,
                                                                   NULL,
                                                                   TRUE );
#else
		HwDeviceExtension->KMVTBuff = ExAllocatePool( NonPagedPool, sizeof( KMVTBUFF) );                                                                   
#endif
        
          if (HwDeviceExtension->KMVTBuff == NULL)
          {
    			return ERROR_INSUFFICIENT_BUFFER;
            }
        }

        memset(HwDeviceExtension->KMVTBuff, 0, sizeof(KMVTBUFF));

        HwDeviceExtension->KMVTBuff->dwOldBuf = 0x1234;

  //	VideoDebugPrint((0, "KMVT Alloc_KMVT_MEMORY get it=%lx\n",HwDeviceExtension->KMVTBuff ));

     	RequestPacket->StatusBlock->Information = sizeof( ULONG );
        *((DWORD *)(RequestPacket->OutputBuffer))  = 
                          (DWORD)HwDeviceExtension->KMVTBuff;

     }
     else
     {
        if( HwDeviceExtension->KMVTBuff )
        {
     //   	VideoDebugPrint((0, "KMVT Free_KMVT_MEMORY\n"));
            
            
#ifdef MS_VIEW            
            VideoPortFreeCommonBuffer ( HwDeviceExtension,
            							sizeof( KMVTBUFF),
                                        HwDeviceExtension->KMVTBuff,
                                        *LogicalAddress,
                                        TRUE );
#else
			ExFreePool(HwDeviceExtension->KMVTBuff );
#endif
                                                    
                                        
            HwDeviceExtension->KMVTBuff = NULL;
        }

     	RequestPacket->StatusBlock->Information = sizeof( ULONG );
     }
     return NO_ERROR;

}
#endif


/**********************************************************************
*   DDGetIRQInfo
*
*   DESCRIPTION: If the Miniport is already managing the IRQ, this
*          function returns that information; otherwise, it returns the
*          IRQ number assigned to the device so DDraw can manage the IRQ.
*
*          The returning the IRQ number, it is important that it get the
*          value assigned by the Config Manager rather than simply get
*          the value from the hardware (since it can be remapped by PCI).
*
*   ENTRY:
*		  Input		HwExtension (device extension so we don't have to use globals!
*         Output	PDDGETIRQINFO
*						DWORD dwSize;
*	                    DWORD dwFlags;
*		                DWORD dwIRQNum;
*
*	Return DX_OK for success
*
**********************************************************************/
static DWORD __stdcall DDGetIrqInfo(PVOID HWExtension, PVOID Input, PDDGETIRQINFO lpGetIrqInfo)
{
   //return handled since we have our own irq handler in h3irq and
   //will manage the IRQ ourselves
   VideoDebugPrint((0, "DDGetIrqInfo\n"));
   lpGetIrqInfo->dwFlags = IRQINFO_HANDLED;
   return DX_OK;
}

/**********************************************************************
*   DDEnableIRQInfo
*
*   DESCRIPTION: Notifies the Mini VDD which IRQs should be enabled.  If
*          a previously enabled IRQ is not specified in this call,
*          it should be disabled.
*	
*   ENTRY:
*		  Input		HwExtension (device extension so we don't have to use globals!         
*					PDDENABLEIRQINFO
*						DWORD dwSize
*						DWORD dwIRQSources
*						DWORD dwLine
*						PDX_IRQCALLBACK  lpIRQCallback - callback routine pointer
*						PDX_IRQDATA		pointer to data to pass in callback routine above
*
*	Return DX_OK for success
*
**********************************************************************/
//h3 intrCtrl bitmasks
#define H3_VMI_INT_ENABLE						 0x00200000

static DWORD __stdcall DDEnableIrq(PVOID HWExtension, PDDENABLEIRQINFO lpEnableIrq, PVOID Output)
{
   PHW_DEVICE_EXTENSION HwDeviceExtension = (PHW_DEVICE_EXTENSION) HWExtension;
   KMVTBUFF * lpKMVTBuff = HwDeviceExtension->KMVTBuff;
#if REDUCED_MEMORY_MAPPINGS
   PH3_3D_REGISTERS sstRegs = (PH3_3D_REGISTERS) ((UCHAR *) HwDeviceExtension->MappedAddress[SST_3D_REGS_INDEX]);
#else
   PH3_3D_REGISTERS sstRegs = (PH3_3D_REGISTERS) ((UCHAR *) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX] + SST_3D_OFFSET);
#endif  
   DWORD 	       dwReg;
   int i;
#ifdef DEBUG_PRINT
   VideoDebugPrint((0,"KMVT - Enable IRQ sources %04.4x (vsync 0x01 vport 0x04)\n", lpEnableIrq->dwIRQSources));
#endif

   if (lpEnableIrq->dwIRQSources != HwDeviceExtension->dwIRQSources) {
		dwReg = sstRegs->intrCtrl & 0x7FFFFFFF;
		if (lpEnableIrq->dwIRQSources & DDIRQ_VPORT0_VSYNC)
		{
		// VMI_IRQ_ENABLE;
		VideoDebugPrint((0, "DDEnableIRQ - VPORT0_VSYNC Callback=%x lpIRQData=%x lpIrqData->dwIrqFlags=%x line=%d\n",
					lpEnableIrq->IRQCallback,lpEnableIrq->lpIRQData,lpEnableIrq->lpIRQData->dwIrqFlags,lpEnableIrq->dwLine));
			lpKMVTBuff->dwIMask |= H3_VMI_INT_ENABLE;
			dwReg |= H3_VMI_INT_ENABLE;
		   
			if (!lpKMVTBuff->bInitialized) {
			   //lpKMVTBuff->Num = 0;
			   lpKMVTBuff->dwCurrentTransfer = 0;
			   lpKMVTBuff->dwQueuedTransfers = 0;
			   lpKMVTBuff->dwBusMasterTransfers = 0;	       
			   lpKMVTBuff->dwFieldCount = 0;		   
			   lpKMVTBuff->bReportForVideo = FALSE;		// default to vbi		   
			   memset (kmvtInfo.Transfer, sizeof (TRANSFER_DATA) * MAX_TRANSFERS, 0);		   
			   lpKMVTBuff->bInitialized = TRUE;
		   }
		}
		else
		{
		//  VMI_IRQ_DISABLE;
			lpKMVTBuff->dwIMask &= ~H3_VMI_INT_ENABLE;
			dwReg &= ~H3_VMI_INT_ENABLE;
			lpKMVTBuff->bInitialized = FALSE;
#if 0		
#ifndef USE_VMM_CALLBACK
          //Just by chance there are any transfers still pending, be sure to cancel them.
		   for (i=0; i<MAX_TRANSFERS; i++)
			{
				if (kmvtInfo.Transfer[i].dwFlags & TRANSFER_QUEUED) 
				{
					if (kmvtInfo.Transfer[i].EventHandle)
					Cancel_Global_Event(kmvtInfo.Transfer[i].EventHandle);
				}
  			}
#endif	
#endif
			//lpKMVTBuff->dwActive = 1;//eav ??
  			lpKMVTBuff->dwCurrentTransfer = 0;
  			lpKMVTBuff->dwQueuedTransfers = 0;
			lpKMVTBuff->dwBusMasterTransfers = 0;
			lpKMVTBuff->dwFieldCount = 0;
			lpKMVTBuff->bReportForVideo = FALSE;		// default to vbi
			memset (kmvtInfo.Transfer, sizeof (TRANSFER_DATA) * MAX_TRANSFERS, 0);
		}

		HwDeviceExtension->pfnIrqCallback  = (lpEnableIrq->dwIRQSources) ? (PVOID)lpEnableIrq->IRQCallback : 0;
		HwDeviceExtension->dwIRQSources = lpEnableIrq->dwIRQSources;
		HwDeviceExtension->lpIRQData = lpEnableIrq->lpIRQData;
		lpEnableIrq->dwLine = 21;
		sstRegs->intrCtrl = dwReg;
   }
   return DX_OK;
}




/**********************************************************************
*   DDSkipNextField
*
*   DESCRIPTION: Called when they want to skip the next field, usually
*       to undo a 3:2 pulldown but also for decreasing the frame rate.
*       The driver should not lose the VBI lines if dwVBIHeight contains
*       a valid value.
*
*   ENTRY:
*		  Input		HwExtension (device extension so we don't have to use globals!
*				    PDDSKIPINFO
*						DWORD dwSize
*						LPDDVIDEOPORTDATA video port
*
*	Return DX_OK for success
*
**********************************************************************/
static DWORD __stdcall DDSkipNextField(PVOID HWExtension, PDDSKIPNEXTFIELDINFO lpSkipInfo,PVOID pOutput)
{
   PHW_DEVICE_EXTENSION HwDeviceExtension = (PHW_DEVICE_EXTENSION) HWExtension;
   KMVTBUFF * lpKMVTBuff = HwDeviceExtension->KMVTBuff;

#ifdef DEBUG_PRINT
   VideoDebugPrint((0, "DDSkipNexField\n"));
#endif
#if 0
   if (lpSkipInfo->dwSkipFlags & DDSKIP_SKIPNEXT)
       lpKMVTBuff->bSkipNextField = TRUE; 
   else if (lpSkipInfo->dwSkipFlags & DDSKIP_ENABLENEXT)
       lpKMVTBuff->bSkipNextField = FALSE;
   else
       return 1;
#endif
   return DX_OK;
}

/**********************************************************************
*   DDBobNextField
*
*   DDBobNextInterleavedEvenOverlayField
*
*   DESCRIPTION: Called when "bob" is used and a VPORT VSYNC occurs that does
*       not cause a flip to occur (e.g. bobbing while interleaved).  When
*       bobbing, the overlay must adjust itself on every VSYNC, so this
*       function notifies it of the VSYNCs that it doesn't already know
*       about (e.g. VSYNCs that trigger a flip to occur).
*
*   ENTRY:
*		  Input		HwExtension (device extension so we don't have to use globals!
*					PDDBOBINFO
*						DWORD dwSize
*						LPDDSURFACE lpSurface
*
*	Return DX_OK for success
*
**********************************************************************/
static DWORD __stdcall DDBobNextField(PVOID HWExtension, PDDBOBNEXTFIELDINFO lpBobInfo,PVOID pOutput)
{

#ifdef DEBUG_PRINT
   //VideoDebugPrint((0, "DDBOBNextField\n"));
#endif
   return DX_OK;
}

/**********************************************************************
*   DDSetState
*
*   DESCRIPTION: Called when the client wants to switch from bob to weave.
*       The overlay flags indicate which state to use. Only called for interleaved
*   surfaces.
*
*       NOTE: When this is called, the specified surface may not be
*       displaying the overlay (due to a flip).  Instead of failing
*       the call, change the bob/weave state for the overlay that would
*       be used if the overlay was flipped again to the specified surface.
*
*   ENTRY:
*		  Input		HwExtension (device extension so we don't have to use globals!
*					LPDDSTATEININFO
*						DWORD dwSize
*						LPDDSURFACEDATA overlay surface
*         OutputI  LPDDSTATEOUTINFO
*					    DWORD dwSize
*						DWORD dwSoftwareAutoflip
*						DWORD dwSurfaceIndex        ; Return Current hardware autoflip
*
*   DURING:
*           EDI OverlaySuface
*           EBX Overlay Flags
*           ECX Overlay Pitch
*           EDX Overlay Src Height
* 
*	Return DX_OK for success
*
**********************************************************************/
static DWORD __stdcall DDSetState(PVOID HWExtension, PDDSETSTATEININFO lpStateIn,PDDSETSTATEOUTINFO lpStateOut)
{

#ifdef DEBUG_PRINT
   VideoDebugPrint((0, "DDSetState\n"));
#endif
   return DX_OK;
}

/**********************************************************************
*   DDLock
*
*   DESCRIPTION: Called when the client wants to lock the surface to
*       access the frame buffer. The driver doens't have to do anything,
*       but it can if it needs to.
*
*   ENTRY:
*		  Input		HwExtension (device extension so we don't have to use globals!
*					PDDLOCKININFO
*						DWORD dwSize
*						LPDDSURFACEDATA surface
*         Output	PDDLOCKOUTINFO
*						DWORD dwSize
*						DWORD Pointer to a pointer to the surface
*
*	Return DX_OK for success
*
**********************************************************************/
static DWORD __stdcall DDLock(PVOID HWExtension, PDDLOCKININFO lpLockInInfo,PDDLOCKOUTINFO lpLockOutInfo)
{
   PHW_DEVICE_EXTENSION HwDeviceExtension = (PHW_DEVICE_EXTENSION) HWExtension;
   LPDDSURFACEDATA lpSurf;
   KMVTBUFF * lpKMVTBuff = HwDeviceExtension->KMVTBuff;

#ifdef DEBUG_PRINT
   VideoDebugPrint((0, "DDLock\n"));
#endif
   // modify to reflect decimated size
   if (lpKMVTBuff)
   {
		lpSurf = (LPDDSURFACEDATA)lpLockInInfo->lpSurfaceData;
		lpSurf->dwWidth = lpKMVTBuff->dwDataWidth;
		lpSurf->dwHeight = lpKMVTBuff->dwDataHeight;	
		lpLockOutInfo->dwSurfacePtr = lpSurf->dwSurfaceOffset;
   }
   return DX_OK;
}

#if 0
/**********************************************************************
*   DESCRIPTION: Flips the overlay to the target surface.
*                For BOB and autofliping, it is only called when Vport 
*                is not involved, accroding to Scott McDonald.               
*   ENTRY:
*		  Input		HwExtension (device extension so we don't have to use globals!
*					PDDFLIPOVERLAYINFO
*						DWORD 		dwSize
*						LPDDSURFACEDATA	lpCurrentSurface
*						LPDDSURFACEDATA	lpTargetSurface
*						DWORD 		dwFlags
*
*	Return DX_OK for success
*
**********************************************************************/
static DWORD __stdcall DDFlipOverlay(PVOID HWExtension, PDDFLIPOVERLAYINFO lpFlipInfo,PVOID pOutput)
{
   return 0;
}    
#endif
/**********************************************************************
*   DDFlipVideoPort
*
*   DESCRIPTION: Flips the video port to the target surface.
*
*   ENTRY:
*		  Input		HwExtension (device extension so we don't have to use globals!
*					PDDFLIPVIDEOPORTINFO
*						DWORD dwSize
*						LPDDVIDEOPORTDATA video port info
*						LPDDSURFACEDATA current surface
*						LPDDSURFACEDATA target surface
*						DWORD dwFlipVPFlags
*
*	Return DX_OK for success
*
*
**********************************************************************/
static DWORD __stdcall DDFlipVideoPort(PVOID HWExtension, PDDFLIPVIDEOPORTINFO lpFlipVideoPort,PVOID pOutput)
{
#ifdef DEBUG_PRINT
   VideoDebugPrint((0, "DDFlipVideoPort\n"));
#endif
   return DXERR_UNSUPPORTED ;
}

/**********************************************************************
*   DDGetPolarity
*
*   DESCRIPTION: Returns the polarity of the current field being written
*       to the specified video port.
*
*   ENTRY:
*		  Input		HwExtension (device extension so we don't have to use globals!
*					PDDPOLARITYININFO
*						DWORD dwSize
*						LPDDVIDEOPORTDATA
*         Output	PDDPOLARITYOUTINFO
*						DWORD dwSize
*						DWORD bPolority (even field = TRUE, odd field = FALSE)
*
*	Return DX_OK for success
*
**********************************************************************/
static DWORD __stdcall DDGetPolarity(PVOID HWExtension, PDDGETPOLARITYININFO lpPolarityIn,PDDGETPOLARITYOUTINFO lpPolarityOut)
{

	PHW_DEVICE_EXTENSION HwDeviceExtension = (PHW_DEVICE_EXTENSION) HWExtension;
	PH3_MEMBASE0 sstIORegs = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
    KMVTBUFF * lpKMVTBuff = HwDeviceExtension->KMVTBuff;
	if (lpKMVTBuff && lpKMVTBuff->bVMIPLDinUse)
	{
		lpPolarityOut->bPolarity = lpKMVTBuff->bLastFieldEven;
	}
	else
	{
	   if (((sstIORegs->vidCurrentLine) >> 18) & 0x01)
		lpPolarityOut->bPolarity = TRUE;
	   else
		lpPolarityOut->bPolarity = FALSE;
	}
#ifdef DEBUG_PRINT
   VideoDebugPrint((0,"KMVT - DDGetPolarity %d\n", lpPolarityOut->bPolarity));
#endif
   return DX_OK;
}


/**********************************************************************
*   DDGetCurrentAutoflip
*
*   DESCRIPTION: Returns the current surface receiving data from the
*       video port while autoflipping is taking palce.  Only called when
*   hardware autoflipping.
*
*   ENTRY:
*		  Input		HwExtension (device extension so we don't have to use globals!
*					PDDGETAUTOFLIPINFO
*						DWORD               dwSize
*         Output    PDDGETAUTOFLIPINFO
*					  DWORD               dwSize
*					  DWORD               dwSurfaceIndex
*					  DWORD				  dwVBISurfaceIndex
*
*	Return DX_OK for success
*
**********************************************************************/
static DWORD __stdcall DDGetCurrentAutoflip(PVOID HWExtension, PDDGETCURRENTAUTOFLIPININFO lpAutoFlipIn,PDDGETCURRENTAUTOFLIPOUTINFO lpAutoFlipOut)
{
 	PHW_DEVICE_EXTENSION HwDeviceExtension = (PHW_DEVICE_EXTENSION) HWExtension;
    PH3_MEMBASE0 sstIORegs = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
    BYTE bNewBuf, bBufModeSelect, bPolarity;
    KMVTBUFF * lpKMVTBuff= HwDeviceExtension->KMVTBuff;

    //VideoDebugPrint((0, "DDGetCurrentAutoflip\n"));

	//Get previous buffer number
	bNewBuf = (BYTE)((sstIORegs->vidCurrentLine) >> 16) & 0x3;

	if (lpKMVTBuff->bReportForVideo)
	{
	   if (lpKMVTBuff->dwOVLOffset 
		   && (lpKMVTBuff->dwIMask & H3_VSYNC_INT_ENABLE) 
		   &&  (lpKMVTBuff->dwStatus & INTERLEAVE_ON ))
	   {	// manual flipping
			bNewBuf = (BYTE)(lpKMVTBuff->dwStatus & BUFFER_IN_USE_MASK);
	   }
	   else
	   {
		   //Query buffer mode - single, double or triple
		   bBufModeSelect = (BYTE)((sstIORegs->vidInFormat) >> 9) & 0x3;
			if (lpKMVTBuff && lpKMVTBuff->bVMIPLDinUse)		// since this is on vbi - use hw
			{
				bPolarity = (BYTE)lpKMVTBuff->bLastFieldEven;
			}
			else
			{
			   bPolarity = (BYTE)((sstIORegs->vidCurrentLine) >> 18) & 0x01;
			}
			if (!(lpKMVTBuff->dwVidInFormat & H3_VMI_DEINTERLACE_WEAVE)		// changes on every field
				|| bPolarity)								// both change on even field
			{
				bNewBuf++;
				if (bNewBuf > bBufModeSelect)
					bNewBuf = 0;
			}
	   }
	} else
		bNewBuf = 0;

	if (bNewBuf == 3)
		bNewBuf = 0;
    lpAutoFlipOut->dwSurfaceIndex = bNewBuf;
	lpAutoFlipOut->dwVBISurfaceIndex = bNewBuf; //lpKMVTBuff->dwLastBufferFilled;

#ifdef DEBUG_PRINTx
	if (lpKMVTBuff->dwFieldCount >= 180 && lpKMVTBuff->dwFieldCount < 190)
	{
	   VideoDebugPrint((0,"KMVT - DDGetCurrentAutoflip - %d\n", bNewBuf));
	}
#endif

   return DX_OK;
}


/**********************************************************************
*   DDGetPreviousAutoflip
*
*   DESCRIPTION: Returns the surface that received the data from the
*       previous field of video port while autoflipping is taking palce. Only
*   called for hardware autoflipping.
*
*   ENTRY:
*		  Input		HwExtension (device extension so we don't have to use globals!
*					PDDGETAUTOFLIPINFO
*						DWORD               dwSize
*         Output    PDDGETAUTOFLIPINFO
*					  DWORD               dwSize
*					  DWORD               dwSurfaceIndex
*					  DWORD				  dwVBISurfaceIndex
*
*	Return DX_OK for success
*
**********************************************************************/
static DWORD __stdcall DDGetPreviousAutoflip(PVOID HWExtension, PDDGETPREVIOUSAUTOFLIPININFO lpAutoFlipIn,PDDGETPREVIOUSAUTOFLIPOUTINFO lpAutoFlipOut)
{

   PHW_DEVICE_EXTENSION HwDeviceExtension = (PHW_DEVICE_EXTENSION) HWExtension;
   PH3_MEMBASE0 sstIORegs = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
   BYTE bNewBuf;
   KMVTBUFF * lpKMVTBuff= HwDeviceExtension->KMVTBuff;

   VideoDebugPrint((0, "DDPreviousAutoFlip\n"));
   if (lpKMVTBuff->bReportForVideo)
   {
		if (0 && lpKMVTBuff && lpKMVTBuff->bVMIPLDinUse)			// since this is on vbi - use hw
			lpAutoFlipOut->dwSurfaceIndex = lpKMVTBuff->dwLastBufferFilled;
		else
		{
		   bNewBuf = (BYTE)((sstIORegs->vidCurrentLine) >> 16) & 0x3;
		}
   }
   else
	   bNewBuf = 0;

   if (bNewBuf == 3)
		bNewBuf = 0;
   lpAutoFlipOut->dwSurfaceIndex = bNewBuf;
   lpAutoFlipOut->dwVBISurfaceIndex = bNewBuf;
#ifdef DEBUG_PRINT
   VideoDebugPrint((0,"KMVT - DDGetPreviousAutoflip - %d\n", bNewBuf));
#endif
   return DX_OK;
}

/**********************************************************************
*   DDTransfer
*
*   DESCRIPTION: tells the driver to bus master data from a surface to the buffer
*                specified in the memory descriptor list (MDL).  The MDL is defined
*                in the WDM documentation.   Called at HW interrupt time
*				 after a Vsync callback is made in interrupt handler
*
*   ENTRY:
*		  Input		HwExtension (device extension so we don't have to use globals!
*					PDDTRANSFERININFO
*						LPDDSURFACEDATA lpSurfaceData - information about surface (vbi or video)
*						DWORD	dwStartLine
*						DWORD   dwEndLine
*						DWORD   dwTransferID (used later in gettransferstatus)
*						PMDL    lpDestMDL - ptr to struct containt dest buffers
*         Output    PDDTRANSFEROUTINFO
*						DWORD dwBufferPolarity;								
*
*	Return DX_OK for success
*
**********************************************************************/
#define H3_VMI_DEINTERLACE_WEAVE  0x00000010

static DWORD __stdcall DDTransfer(PVOID HWExtension, PDDTRANSFERININFO lpTransferIn,PDDTRANSFEROUTINFO lpTransferOut)
{
   PHW_DEVICE_EXTENSION HwDeviceExtension = (PHW_DEVICE_EXTENSION) HWExtension;
   PH3_MEMBASE0 sstIORegs = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
   DDSURFACEDATA        *lpSurfaceData;
   KMVTBUFF			   *lpKMVTBuff=HwDeviceExtension->KMVTBuff;
   DWORD				dwTransferIndex=0;
   BOOL					bIsEven;
  
   //VideoDebugPrint((0, "DDTransfer\n"));
   lpSurfaceData = (DDSURFACEDATA *)lpTransferIn->lpSurfaceData;
 
   if (!lpKMVTBuff)
     return DXERR_GENERIC;

   if (lpKMVTBuff->bVMIPLDinUse)
   {
		//lpTransferOut->dwBufferPolarity = !lpKMVTBuff->bLastFieldEven;
	     // looks like polarity is now true for even fields
		lpTransferOut->dwBufferPolarity = lpKMVTBuff->bLastFieldEven;
		bIsEven = lpKMVTBuff->bLastFieldEven;	
   } else	{
	   if (((sstIORegs->vidCurrentLine) >> 18) & 0x01)
			lpTransferOut->dwBufferPolarity = FALSE;
	   else
			lpTransferOut->dwBufferPolarity = TRUE;
	   bIsEven = !lpTransferOut->dwBufferPolarity;	
   }

   if ((lpTransferIn->lpDestMDL==0) || (lpTransferIn->lpDestMDL->ByteCount==0)){   
	   VideoDebugPrint((0, "DDTransfer error - null mdls\n"));
     //return DXERR_GENERIC;	 
	   return DX_OK;
   }

   //do synchronous transfers
   lpKMVTBuff->fTransferFull = AddTransfer (lpTransferIn, &dwTransferIndex, bIsEven,HwDeviceExtension);

   if (!lpKMVTBuff->fTransferFull && (dwTransferIndex < MAX_TRANSFERS)) 
   {
	 return (TransferVMEvent(dwTransferIndex, HwDeviceExtension));
	 //return DX_OK;
   } 

#ifdef DEBUG_PRINT
   VideoDebugPrint ((0," ******* Transfer FULL ******** \n"));
#endif
   return DXERR_GENERIC;
}

/**********************************************************************
*   DDGetTransferStatus
*
*   DESCRIPTION: Returns the transfer id in the DDGETTRANSFERSTATUSOUTINFO 
*                structure.  This function is used to determined which 
*                hardware bus master has completed.
*
*   ENTRY:
*		  Input		HwExtension (device extension so we don't have to use globals!
*		  Output	PDDGETTRANSFERSTATUSOUTINFO
*					    DWORD_PTR dwTransferID;
*
*	Return DX_OK for success
*
**********************************************************************/
static DWORD __stdcall DDGetTransferStatus(PVOID HWExtension, PVOID pInput,PDDGETTRANSFEROUTINFO lpTransferStatus)
{
   PHW_DEVICE_EXTENSION HwDeviceExtension = (PHW_DEVICE_EXTENSION) HWExtension;
   KMVTBUFF * lpKMVTBuff= HwDeviceExtension->KMVTBuff;
   DWORD i=0;
   BOOL  fFound=FALSE;

   //VideoDebugPrint((0, "DDGetTransferStatus\n"));
 //VideoDebugPrint((0,"KMVT - DDGetTransferStatus - QueuedTransfers=%d Busmasterready = %d\n",lpKMVTBuff->dwQueuedTransfers, lpKMVTBuff->dwBusMasterTransfers));
   for (i=lpKMVTBuff->dwCurrentTransfer;i<MAX_TRANSFERS && !fFound; i++) {
	if (kmvtInfo.Transfer[i].dwFlags == TRANSFER_COMPLETE) {
		lpTransferStatus->dwTransferID = kmvtInfo.Transfer[lpKMVTBuff->dwCurrentTransfer].dwTransferID;  
		kmvtInfo.Transfer[i].dwFlags = TRANSFER_READY;
		lpKMVTBuff->dwBusMasterTransfers--;
		lpKMVTBuff->dwCurrentTransfer++;
		if (lpKMVTBuff->dwCurrentTransfer == MAX_TRANSFERS)
			lpKMVTBuff->dwCurrentTransfer = 0;
	}
	fFound = TRUE;
   }
   for (i=0; i<lpKMVTBuff->dwCurrentTransfer && !fFound; i++) {
	if (kmvtInfo.Transfer[i].dwFlags == TRANSFER_COMPLETE) {
		lpTransferStatus->dwTransferID = kmvtInfo.Transfer[lpKMVTBuff->dwCurrentTransfer].dwTransferID; 
		kmvtInfo.Transfer[i].dwFlags = TRANSFER_READY;
		lpKMVTBuff->dwBusMasterTransfers--;
		lpKMVTBuff->dwCurrentTransfer++;
		if (lpKMVTBuff->dwCurrentTransfer == MAX_TRANSFERS)
			lpKMVTBuff->dwCurrentTransfer = 0;
	}
	fFound = TRUE;
   }
   if (fFound) {
	  return DX_OK;
   }
   
   if ((lpKMVTBuff->dwBusMasterTransfers > 0) && 
	    kmvtInfo.Transfer[lpKMVTBuff->dwCurrentTransfer].dwFlags == TRANSFER_COMPLETE) {
		lpTransferStatus->dwTransferID = kmvtInfo.Transfer[lpKMVTBuff->dwCurrentTransfer].dwTransferID;
		kmvtInfo.Transfer[lpKMVTBuff->dwCurrentTransfer].dwFlags = TRANSFER_READY;
		lpKMVTBuff->dwBusMasterTransfers--;
		lpKMVTBuff->dwCurrentTransfer++;
		if (lpKMVTBuff->dwCurrentTransfer == MAX_TRANSFERS)
			lpKMVTBuff->dwCurrentTransfer = 0;
   }
   return DX_OK;
}



BYTE AddTransfer(DDTRANSFERININFO *lpTransferIn, DWORD *dwCurrentIndex, DWORD polarity, PHW_DEVICE_EXTENSION HwDeviceExtension)
{
   BYTE  fSlotsFull = 1;
   BOOL  bFirstCheck = TRUE;
   DWORD i;
   KMVTBUFF * lpKMVTBuff= HwDeviceExtension->KMVTBuff;

   *dwCurrentIndex = 0;
   for (i = lpKMVTBuff->dwCurrentTransfer, bFirstCheck = TRUE; 
		i<MAX_TRANSFERS; i++)
   {
	if (kmvtInfo.Transfer[i].dwFlags == TRANSFER_READY)
	{
  	    lpKMVTBuff->dwQueuedTransfers ++;
		//VideoDebugPrint ((0," AddTransfer1 = %d Queued=%d\n", i,lpKMVTBuff->dwQueuedTransfers));
	    *dwCurrentIndex = i;
	    kmvtInfo.Transfer[i].lpSurfaceData = (DDSURFACEDATA *)lpTransferIn->lpSurfaceData;
	    kmvtInfo.Transfer[i].dwTransferID = lpTransferIn->dwTransferID;
	    kmvtInfo.Transfer[i].pMDL = lpTransferIn->lpDestMDL;
	    kmvtInfo.Transfer[i].dwTransferFlags = lpTransferIn->dwTransferFlags;
	    kmvtInfo.Transfer[i].dwStartLine = lpTransferIn->dwStartLine;
	    kmvtInfo.Transfer[i].dwEndLine = lpTransferIn->dwEndLine;
	    kmvtInfo.Transfer[i].dwFlags = TRANSFER_QUEUED;
	    kmvtInfo.Transfer[i].bPolarity = polarity;
        fSlotsFull = 0;
	    break;
     }
   }
   for (i=0; (i<lpKMVTBuff->dwCurrentTransfer) && fSlotsFull; i++)
   {
   	if (kmvtInfo.Transfer[i].dwFlags == TRANSFER_READY)
	{
	 	lpKMVTBuff->dwQueuedTransfers ++;
		//VideoDebugPrint ((0," AddTransfer2 = %d Queued=%d\n", i,lpKMVTBuff->dwQueuedTransfers));
		*dwCurrentIndex = i;	  
		kmvtInfo.Transfer[i].lpSurfaceData = (DDSURFACEDATA *)lpTransferIn->lpSurfaceData;	   
		kmvtInfo.Transfer[i].dwTransferID = lpTransferIn->dwTransferID;	   
		kmvtInfo.Transfer[i].pMDL = lpTransferIn->lpDestMDL;	   
		kmvtInfo.Transfer[i].dwTransferFlags = lpTransferIn->dwTransferFlags;
	    kmvtInfo.Transfer[i].dwStartLine = lpTransferIn->dwStartLine;
	    kmvtInfo.Transfer[i].dwEndLine = lpTransferIn->dwEndLine;
	    kmvtInfo.Transfer[i].dwFlags = TRANSFER_QUEUED;
	    kmvtInfo.Transfer[i].bPolarity = polarity;
        fSlotsFull = 0;
	    break;
     }
   }
   return fSlotsFull;
}

/*----------------------------------------------------------------------
Function name:  TransferVMEvent

Description:    
                
Information:    

Return:         VOID
----------------------------------------------------------------------*/
DWORD TransferVMEvent( DWORD refData, PHW_DEVICE_EXTENSION HwDeviceExtension )
{

   DWORD			RefData;
   DDSURFACEDATA   *lpSurfaceData;
   PMDL             pMDL;
   BYTE            *lpBufferStart, *lpBufferDest;
   DWORD 			dwNumLines;
   KMVTBUFF        *lpKMVTBuff;
   BYTE				bUseAllLines = 0;
   DWORD            dwMaxTransferCnt;
   DWORD			lineWidth, dwTransferFlags;
   DWORD			linecnt;
   DWORD			lineOffset;
   BYTE				*lpSrcLine;
   BOOL				bInterleaved = FALSE;
   DWORD			polarity;
   PH3_MEMBASE0 sstIORegs = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
   RefData = refData;

   //Get global information specifically number of VBI lines
   lpKMVTBuff = HwDeviceExtension->KMVTBuff;
   kmvtInfo.Transfer[RefData].dwFlags = TRANSFER_IN_PROGRESS;	// do this first to avoid deadlock
   if (RefData >= MAX_TRANSFERS || !lpKMVTBuff->bInitialized)
	   return DXERR_GENERIC;

   lpKMVTBuff->dwQueuedTransfers --;
   polarity = kmvtInfo.Transfer[RefData].bPolarity;

   //Video Capture
   bUseAllLines = 1;

   //Get global information specifically number of VBI lines
   lpSurfaceData = kmvtInfo.Transfer[RefData].lpSurfaceData; 
   if (!lpSurfaceData || !lpKMVTBuff->bInitialized)
	   return DXERR_GENERIC;

   pMDL = kmvtInfo.Transfer[RefData].pMDL;
   dwNumLines =    kmvtInfo.Transfer[RefData].dwEndLine - kmvtInfo.Transfer[RefData].dwStartLine + 1;
   dwTransferFlags = kmvtInfo.Transfer[RefData].dwTransferFlags;
   lineWidth = lpSurfaceData->dwWidth * (lpSurfaceData->dwFormatBitCount / 8);	

   if (!lpKMVTBuff || !lpKMVTBuff->bInitialized)
	   return DXERR_GENERIC;

   bInterleaved = ((lpKMVTBuff->dwVidInFormat & H3_VMI_DEINTERLACE_WEAVE)
			|| (lpKMVTBuff->dwStatus & INTERLEAVE_ON));

   ///////////////////////////////////
   //VBI Capture 
   ///////////////////////////////////
   if (lpSurfaceData->dwFormatFourCC == FOURCC_RAW8) 
   {
      BYTE bBufNum;		
	  //always use last buffer since we sometimes can't count on vidcurrentline to represent
	  //correct buffer
	  bBufNum= (BYTE)lpKMVTBuff->dwLastBufferFilled;
	  if (bBufNum == 3)
		  bBufNum=0;		
#if 0
	  if (0 && lpKMVTBuff->bVMIPLDinUse)			// since this is on vbi - use hw		
		  bBufNum = (BYTE)lpKMVTBuff->dwLastBufferFilled;		
	  else		
		  bBufNum = (BYTE)(sstIORegs->vidCurrentLine >> 16) & 0x3;

	  //VideoDebugPrint((0, "\tVBI Buf=%d, Lastbuf=%d\n",bBufNum,lpKMVTBuff->dwLastBufferFilled));
	  if (bBufNum==3) {
		  bBufNum=(BYTE)lpKMVTBuff->dwLastBufferFilled;
	    //  VideoDebugPrint((0, "\tMiniPort VBI Buf=%d, polarity = %d vidcurrentline=%x\n",bBufNum,polarity, sstIORegs->vidCurrentLine));
	  }
#endif
	  lpBufferStart = (BYTE *)lpKMVTBuff->fpVidMem[bBufNum];
	  //VideoDebugPrint((0, "\tMiniPort VBI Buf=%d, Ptr=%x\n",bBufNum,lpBufferStart));

	  if (!lpBufferStart || !lpKMVTBuff->bInitialized)			
		  return DXERR_GENERIC;

	  if (bInterleaved)	// see if half or full
	  {
			if (dwNumLines < (lpKMVTBuff->dwVBILines-2))			// not a match 
				bUseAllLines = 0;
	  }
	  lineOffset = lpKMVTBuff->dwVideoSurfacePitch;

	  if (!bUseAllLines) {
	    lineOffset <<= 1;		// double if every other line
	  }

      if (bInterleaved && polarity)		// must skip first line
		lpBufferStart += lpKMVTBuff->dwVideoSurfacePitch;
 

	  lpSrcLine = lpBufferStart;
	
	  if (dwTransferFlags & DDTRANSFER_INVERT)	
	  {	
		  lineOffset = 0 - (lineOffset);
		  lpSrcLine = lpBufferStart + ((dwNumLines - 1) * lpKMVTBuff->dwVideoSurfacePitch);	
	  }
	
	  //Micronas chip only generates 1140 valid bytes per line so only copy that much.	
	  lineWidth = 1140;	
	  for (linecnt = 0; linecnt < dwNumLines && pMDL;)	
	  {
		BYTE *lpDestLine;
		DWORD bytes;
		DWORD bytesPerLine = lineWidth;

		//Now get the pointer to the memory destination
		lpBufferDest = (BYTE *)(pMDL->MappedSystemVa);
		if (!lpBufferDest || !lpKMVTBuff->bInitialized)			
			return DXERR_GENERIC;
		lpDestLine = lpBufferDest;
		dwMaxTransferCnt = pMDL->ByteCount; 

		// must copy line at a time to not affect graphic performance (kephart suggestion)
		for (bytes = 0;  lpKMVTBuff->bInitialized &&
			linecnt < dwNumLines && bytes < dwMaxTransferCnt;)		// should this be <= for numlines
		{
			memcpy (lpDestLine, lpSrcLine, bytesPerLine);	// use width in case pitch diff - tiled space
			linecnt++;
  			lpDestLine += lineWidth;
			lpSrcLine += lineOffset;
  			bytes += lineWidth;		// must reflect dest
		}
		pMDL = pMDL->Next;	
	  }  
   }
   else 
   {
     ///////////////////////////////////
     // Video Capture
     ///////////////////////////////////
	    DWORD dwVidLines = lpKMVTBuff->dwVideoLines;
		BYTE bBufNum;
		//lpBufferStart = (BYTE *)(lpSurfaceData->fpLockPtr);
		lpBufferStart = (BYTE *)(lpSurfaceData->dwSurfaceOffset);

		if (!lpBufferStart || !lpKMVTBuff->bInitialized)	   
			return DXERR_GENERIC;

		// if the fake interleave is active use this code for buffer start
		if (lpKMVTBuff->dwOVLOffset
		   && (lpKMVTBuff->dwIMask & H3_VSYNC_INT_ENABLE) 
		   &&  (lpKMVTBuff->dwStatus & INTERLEAVE_ON ))
		{	// manual flipping
			bBufNum = lpKMVTBuff->bFirstFieldBuffer;
			if (bBufNum==3)
				bBufNum=0;
			lpBufferStart = (BYTE *)lpKMVTBuff->fpVidMem[bBufNum];
			if (!lpBufferStart || !lpKMVTBuff->bInitialized)	   
				return DXERR_GENERIC;
			polarity = FALSE;		// just capture first field - ok for v3
		}

 	    lpKMVTBuff->bReportForVideo = TRUE;		// doing video

		//if (lpKMVTBuff->dwActive == 1)		// is eav
		if (lpKMVTBuff->bUsingEAV)
			dwVidLines++;

//		VDDtoWDMTellCapture(lpKMVTBuff->dwFieldCount, bIsEven, FALSE); 

		if (!lpKMVTBuff->bVBIcropped)
			lpBufferStart += (lpKMVTBuff->dwVBILines*lpSurfaceData->lPitch);

		lineOffset = lpSurfaceData->lPitch;						// pitch on surface
		if (dwNumLines > dwVidLines)		// should be double
			bUseAllLines = 1;
		else 
			bUseAllLines = 0;
		if (bInterleaved && !bUseAllLines)	// see if half or full
		{
			if (polarity)		// must skip first line
				lpBufferStart += lpKMVTBuff->dwVideoSurfacePitch;
		}
		if (lpKMVTBuff->bDoublePitch && !bUseAllLines)
				lineOffset <<= 1;		// double if every other line

		lpSrcLine = lpBufferStart;
		if (dwTransferFlags & DDTRANSFER_INVERT)
		{
			lineOffset = 0 - (lineOffset);
			lpSrcLine = lpBufferStart + ((dwNumLines - 1) * lpSurfaceData->lPitch);
		}

		//in the case of vddtransfer we need to copy the data as specified 
		//in the linked list of MDL
		for (linecnt = 0; linecnt < dwNumLines && pMDL;)
		{
			BYTE *lpDestLine;
			DWORD bytes;
			DWORD bytesPerLine = lineWidth;
			if (bytesPerLine > lineOffset)
				bytesPerLine = lineOffset;

			//Now get the pointer to the memory destination
			lpBufferDest = (BYTE *)(pMDL->MappedSystemVa);
			if (!lpBufferDest || !lpKMVTBuff->bInitialized)	   
				return DXERR_GENERIC;
			lpDestLine = lpBufferDest;
			dwMaxTransferCnt = pMDL->ByteCount; 

			// must copy line at a time to not affect graphic performance (kephart suggestion)
			for (bytes = 0; lpKMVTBuff->bInitialized &&
				linecnt < dwNumLines && bytes < dwMaxTransferCnt;)		// should this be <= for numlines
			{
				if (!linecnt)	// get rid of cc data
				{
					DWORD ndx;
					WORD *pWord = (WORD *)lpDestLine;
					for (ndx = 0; ndx < bytesPerLine; ndx += 2)
						*pWord++ = 0x1080;
				}
				else
					memcpy (lpDestLine, lpSrcLine, bytesPerLine);	// use width in case pitch diff - tiled space
				linecnt++;
  				lpDestLine += lineWidth;
				lpSrcLine += lineOffset;
  				bytes += lineWidth;
			}
			pMDL = pMDL->Next;
		}  
   }

   kmvtInfo.Transfer[RefData].dwFlags = TRANSFER_COMPLETE;
   lpKMVTBuff->dwBusMasterTransfers ++;

   //VideoDebugPrint ((0," Transfer BusMaster Complete=%d Outstanding=%d\n", RefData,lpKMVTBuff->dwBusMasterTransfers)); 

   //Now notify Windows that transfer is complete
   if (HwDeviceExtension->pfnIrqCallback)
   {
	  ((PDX_IRQDATA)HwDeviceExtension->lpIRQData)->dwIrqFlags = DDIRQ_BUSMASTER;
	  ((PDX_IRQCALLBACK)(HwDeviceExtension->pfnIrqCallback))(((PDX_IRQDATA)HwDeviceExtension->lpIRQData));
   }
	  
   //VideoDebugPrint((0,"End DDTransfer 2\n"));
   return DX_OK;
	
}

/*****************************************************************
*
*  I2cOpen
*
*  I2c Open routine to lock access to i2c device
*		Open will lock device
*       Close will release it.
*
*******************************************************************/
static DWORD __stdcall I2cOpen (PDEVICE_OBJECT pDev, ULONG fOpenClose, PI2CControl pControl)
{
	//hack since I can't figure out how to gt the hwdeviceextension from the deviceobject
	I2CCONTEXT pContext = (I2CCONTEXT)pControl->Status;
	if (!pControl || !pContext)
		return I2C_STATUS_ERROR;
	
    if(fOpenClose)    // True means open the channel   
        // Put a unique identifier in the Cookie field
 	    pControl->dwCookie = (DWORD)i2c_getaccess((I2CCONTEXT)pContext, I2C_MULTIMEDIA,0);
    else    // Closing the channel
		i2c_endaccess((I2CCONTEXT)pContext, (I2CKEY)pControl->dwCookie);
	return I2C_STATUS_NOERROR;
}

/*****************************************************************
*
*  I2cAccess
*
*  I2c Access routine to perform read and writes to i2c device
*	  First i2c open must be called before set of i2c access calls
*	  and after last i2caccess then i2c close must be called
*
*******************************************************************/
#define V3TV_FLAGS_ACTIVE 0x1000
static DWORD __stdcall I2cAccess (PDEVICE_OBJECT pDev, PI2CControl pControl)
{
	//hack since I can't figure out how to gt the hwdeviceextension from the deviceobject
	I2CCONTEXT pContext = (I2CCONTEXT)pControl->Status;

	if (!pControl || !pContext)
		return I2C_STATUS_ERROR;

    switch(pControl->Command)    {        
		case I2C_COMMAND_READ:
		    if (pControl->Flags & I2C_FLAGS_START)	//check for start case
				i2c_start((I2CCONTEXT)pContext, (I2CKEY)pControl->dwCookie);

            // Read a byte from the I2C bus and put it in pControl->Data   
			// remember to send ack as 1 (even though it is really a 0
			// error is when a 0 is returned - no ack
            if (!(i2c_readbyte((I2CCONTEXT)pContext, (I2CKEY)pControl->dwCookie, (FxU8*) &pControl->Data, (pControl->Flags&I2C_FLAGS_ACK))))
            {			
				//stop and return error               
				i2c_stop((I2CCONTEXT)pContext, (I2CKEY)pControl->dwCookie); // acknolge failed --> generate stop condition
				//pControl->Status = I2C_STATUS_ERROR;             
				return I2C_STATUS_ERROR; 
            }
			if (pControl->Flags & I2C_FLAGS_STOP)	//check for stop case
               i2c_stop((I2CCONTEXT)pContext, (I2CKEY)pControl->dwCookie);

			//pControl->Status = I2C_STATUS_NOERROR;	//return success
		    return I2C_STATUS_NOERROR;

		break; 

		case I2C_COMMAND_WRITE:
		   if (pControl->Flags & I2C_FLAGS_START)	//check for start case
				i2c_start((I2CCONTEXT)pContext, (I2CKEY)pControl->dwCookie);

           // Send the byte in pControl->Data to the I2C bus
		   //error is when a 0 is returned (no ack) or data failed to set when checked
           if (!(i2c_sendbyte((I2CCONTEXT)pContext, (I2CKEY)pControl->dwCookie, pControl->Data)))
           {
			   //stop and return error
               i2c_stop((I2CCONTEXT)pContext, (I2CKEY)pControl->dwCookie);
			   //pControl->Status = I2C_STATUS_NOERROR;
               return I2C_STATUS_ERROR; 
           }
		   if (pControl->Flags & I2C_FLAGS_STOP)	//check for stop case
               i2c_stop((I2CCONTEXT)pContext, (I2CKEY)pControl->dwCookie);

			//pControl->Status = I2C_STATUS_NOERROR;	//return success
		   return I2C_STATUS_NOERROR;
		break; 

		//this is a new one so that we can pass standalone starts and stops
		case I2C_COMMAND_NULL:
		   if (pControl->Flags & I2C_FLAGS_STOP)
               i2c_stop((I2CCONTEXT)pContext, (I2CKEY)pControl->dwCookie);

		   if (pControl->Flags & I2C_FLAGS_START)
				i2c_start((I2CCONTEXT)pContext, (I2CKEY)pControl->dwCookie);		   

		   //check for private flags the tell us where WDM has open streams
		   //this way we know whether to do private scaling through Micronas chip
		   if (pControl->Flags & V3TV_FLAGS_ACTIVE)
		   {
			   KMVTBUFF *lpKMVTBuff = ((PHW_DEVICE_EXTENSION)pContext)->KMVTBuff;
			   if (lpKMVTBuff)
			   {
					if (pControl->Data) {
						lpKMVTBuff->fWDMActive = TRUE;
						lpKMVTBuff->bUsingEAV = (pControl->Data==1) ? TRUE : FALSE;
					} else {
						lpKMVTBuff->fWDMActive = FALSE;
					}
			   }
		   }
		break;

		case I2C_COMMAND_STATUS:
		break;
	};
	//pControl->Status = I2C_STATUS_NOERROR;
	return I2C_STATUS_NOERROR;
}

#define VPX2_VBEG1              0x120
#define VPX2_VLINEI1            0x121
#define VPX2_VLINEO1            0x122
#define VPX2_HBEG1              0x123
#define VPX2_HLEN1              0x124
#define VPX2_NPIX1              0x125
#define VPX2_CMDWD              0x140 
#define VPX2_TDECFRAMES         0x157

#define V3TV_CHIPID			    0x86
#define VPX_RET_OK              0x0000
#define VPX_RET_POLLING_TIMEOUT 0x0001
#define VPX_TIME_OUT_COUNT      10000
#define MASTER_WRITE (0x00)
#define MASTER_READ (0x01)

WORD I2cWrite(BYTE addr, WORD data,I2CKEY dwCookie, I2CCONTEXT pContext);
WORD I2cRead(BYTE addr, BYTE *data, BYTE datalength, I2CKEY dwCookie, I2CCONTEXT pContext);
WORD VPXReadFP(WORD wFPRegister, WORD *lpwFPData, I2CKEY dwCookie, I2CCONTEXT pContext);
WORD VPXStatusFP(I2CKEY dwCookie, I2CCONTEXT pContext);
WORD VPXWriteFP(WORD wFPRegister, WORD wFPData, I2CKEY dwCookie, I2CCONTEXT pContext);

VP_STATUS WDMV3TVScale(PHW_DEVICE_EXTENSION HwDeviceExtension,	PVIDEO_REQUEST_PACKET RequestPacket)
{
	I2CKEY dwCookie;
    KMVTBUFF *lpKMVTBuff = HwDeviceExtension->KMVTBuff;
	DWORD dwWidth,dwHeight,dwLeft,dwTop;
	DWORD *inBuff =  (DWORD *)RequestPacket->InputBuffer;
	DWORD dwSize = inBuff[0];
	DWORD dwStart = inBuff[1];
	DWORD left,extra;
	WORD  nNumLines = (WORD)lpKMVTBuff->dwVideoLines;
	WORD  nNumPixels,wTemp, vpxReturn;   
	long  lWait;

	dwWidth = (dwSize & 0x0000FFFFL);	
	dwHeight = ((dwSize>>16) & 0x00007FFFL);	
	dwLeft = (dwStart & 0x0000FFFFL);	
	dwTop = ((dwStart>>16) & 0x00007FFFL);
	
	if (dwLeft & 1)
		dwLeft--;
	if( dwWidth & 1 ) 	
		dwWidth++;
	if (dwHeight & 1 )
		dwHeight++;

	left = (8 + dwLeft) * dwWidth / lpKMVTBuff->decoderWidth;
	extra = 8 * dwWidth/ lpKMVTBuff->decoderWidth;
	nNumPixels = (WORD)(extra + left + dwWidth);

    //VideoDebugPrint((0, "\tWDM Scale decwidth=%d numlines=%d, height=%d width=%d dwleft=%d dwbottom=%d left=%d numpixels=%d\n",lpKMVTBuff->decoderWidth, nNumLines, dwHeight,dwWidth,dwLeft,dwTop,left,nNumPixels));
 	dwCookie = i2c_getaccess((I2CCONTEXT)HwDeviceExtension, I2C_MULTIMEDIA,0);

	//Write the temporal decimation value
	VPXWriteFP(VPX2_TDECFRAMES, 0, dwCookie, (I2CCONTEXT)HwDeviceExtension);
    vpxReturn = VPXWriteFP(VPX2_VBEG1,   (WORD)(22 + dwTop & 0x1FF), dwCookie, (I2CCONTEXT)HwDeviceExtension);
    vpxReturn = VPXWriteFP(VPX2_VLINEI1, nNumLines, dwCookie, (I2CCONTEXT)HwDeviceExtension);
    vpxReturn = VPXWriteFP(VPX2_VLINEO1, (WORD)dwHeight, dwCookie, (I2CCONTEXT)HwDeviceExtension);
    vpxReturn = VPXWriteFP(VPX2_HBEG1,   (WORD)left, dwCookie, (I2CCONTEXT)HwDeviceExtension);
    vpxReturn = VPXWriteFP(VPX2_HLEN1,   (WORD)dwWidth, dwCookie, (I2CCONTEXT)HwDeviceExtension);
    vpxReturn = VPXWriteFP(VPX2_NPIX1,   nNumPixels, dwCookie, (I2CCONTEXT)HwDeviceExtension);

#if 1
	//latch registers
	vpxReturn = VPXReadFP(VPX2_CMDWD, &wTemp, dwCookie, (I2CCONTEXT)HwDeviceExtension);	
	wTemp |= 0x20;     
	 vpxReturn = VPXWriteFP(VPX2_CMDWD, wTemp, dwCookie, (I2CCONTEXT)HwDeviceExtension); 
	 for (lWait=VPX_TIME_OUT_COUNT; lWait--;)        
	 {        	
		 if (vpxReturn = VPXReadFP(VPX2_CMDWD, &wTemp, dwCookie, (I2CCONTEXT)HwDeviceExtension) != 0) 
			 lWait = 1;
		 if (!(wTemp & 0x20) | vpxReturn) 
				break;         
	 }        
	//if (!lWait)		 //error never set
	// vpxReturn |= VPX_RET_POLLING_TIMEOUT;
#endif
		
	i2c_endaccess((I2CCONTEXT)HwDeviceExtension, (I2CKEY)dwCookie);
#ifdef WINNT
    return NO_ERROR;
#else
	return 0;
#endif   
}
 
#define VPX2_FPSTA              0x35
#define VPX2_FPRD               0x36
#define VPX2_FPWR               0x37
#define VPX2_FPDAT              0x38
WORD VPXWriteFP(WORD wFPRegister, WORD wFPData, I2CKEY dwCookie, I2CCONTEXT pContext)
{
    WORD vpxReturn = VPX_RET_OK;
    vpxReturn |= VPXStatusFP(dwCookie, pContext);
    if(vpxReturn == VPX_RET_OK)
    	vpxReturn |= I2cWrite(VPX2_FPWR, wFPRegister, dwCookie, pContext);
    vpxReturn |= VPXStatusFP(dwCookie, pContext);
    if(vpxReturn == VPX_RET_OK)
        vpxReturn |= I2cWrite(VPX2_FPDAT, wFPData, dwCookie, pContext);
    return(vpxReturn);
}

WORD VPXReadFP(WORD wFPRegister, WORD *lpwFPData, I2CKEY dwCookie, I2CCONTEXT pContext)
{
    WORD vpxReturn = VPX_RET_OK;  
         
    vpxReturn |= VPXStatusFP(dwCookie, pContext);
    if(vpxReturn == VPX_RET_OK)
    	vpxReturn |= I2cWrite(VPX2_FPRD, wFPRegister, dwCookie, pContext);
   	vpxReturn |= VPXStatusFP(dwCookie, pContext);
    if(vpxReturn == VPX_RET_OK)
        vpxReturn |= I2cRead(VPX2_FPDAT, (BYTE *)lpwFPData, 2, dwCookie, pContext);

	return(vpxReturn);
}

WORD VPXStatusFP(I2CKEY dwCookie, I2CCONTEXT pContext)
{
    BYTE bTemp;
    WORD wI;
    WORD vpxReturn = VPX_RET_OK;
        
    for (wI=0; wI < VPX_TIME_OUT_COUNT; wI++)
   	{
    	vpxReturn |= I2cRead(VPX2_FPSTA, &bTemp, 1, dwCookie, pContext);
        if (!(bTemp & 0x04)) break;
    }
    if (wI == VPX_TIME_OUT_COUNT)
    	vpxReturn = VPX_RET_POLLING_TIMEOUT;
            	    
    return(vpxReturn);
}

WORD I2cRead(BYTE addr, BYTE *data, BYTE datalength, I2CKEY dwCookie, I2CCONTEXT pContext)
{
	int rc=0;  //should be 0 for error
	BYTE temp;

	i2c_start(pContext, dwCookie);

    rc = i2c_sendbyte(pContext, dwCookie, V3TV_CHIPID|MASTER_WRITE);
	if (rc)
	  rc = i2c_sendbyte(pContext, dwCookie, addr);

	i2c_start(pContext,  dwCookie);
	if (rc)
      rc = i2c_sendbyte(pContext, dwCookie, V3TV_CHIPID|MASTER_READ);

	if (rc)
		rc = i2c_readbyte(pContext, dwCookie, (FxU8*)&temp, (datalength>1)?1:0);

	*data = temp;

	if (rc && (datalength>1)) {
		*data = temp<<8;
		rc = i2c_readbyte(pContext, dwCookie, (FxU8*)&temp, 0);
		*data |= temp;
	}
		
	i2c_stop(pContext,  dwCookie);
	return (!rc) ? I2C_STATUS_ERROR : 0;
}

WORD I2cWrite(BYTE addr, WORD data, I2CKEY dwCookie, I2CCONTEXT pContext)
{
	int rc=0;  //should be 0 for error
	i2c_start(pContext,  dwCookie);

    rc = i2c_sendbyte(pContext, dwCookie, V3TV_CHIPID|MASTER_WRITE);
	
	if (rc)
		rc = i2c_sendbyte(pContext, dwCookie, addr);

	if (rc)
		rc = i2c_sendbyte(pContext, dwCookie, (BYTE)(data>>8));

	if (rc)
		rc = i2c_sendbyte(pContext, dwCookie, (BYTE)(data &0xFF));
		
	i2c_stop(pContext, dwCookie);
	return (!rc) ? I2C_STATUS_ERROR : 0;
}

#define VPX_ID1                 0x01
#define VPX_ID2                 0x02
WORD VPXDetectDevice(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    BYTE bTemp;
    WORD wTemp;
	I2CKEY dwCookie;
  	dwCookie = i2c_getaccess((I2CCONTEXT)HwDeviceExtension, I2C_MULTIMEDIA,0);
    I2cRead(VPX_ID2, &bTemp, 1, dwCookie, (I2CCONTEXT)HwDeviceExtension);
    wTemp = bTemp;
    I2cRead(VPX_ID1, &bTemp, 1, dwCookie, (I2CCONTEXT)HwDeviceExtension);
    wTemp = (wTemp << 8) | bTemp;
	i2c_endaccess((I2CCONTEXT)HwDeviceExtension, (I2CKEY)dwCookie);

	// check for V3TV and then set reg for PAL reg problem.
	if (wTemp == V3TV_VPX3225D){
		VRSetRegistry (HwDeviceExtension);
	}

    return(wTemp);
}

#ifdef MS_VIEW
#pragma warning ( push )
#pragma warning ( disable : 4296 )
#endif

VP_STATUS VRSetRegistry (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
  USHORT SID = HwDeviceExtension->PCISubSystemID;
  ULONG VID = (ULONG)HwDeviceExtension->PCISubVendorID;

  const WCHAR tdfxDevNodeKey[] = L"\\Registry\\Machine\\SOFTWARE\\3dfx Interactive\\VisualReality";
  const WCHAR ValueName[] = L"V3TV_ID";
  //const char ValueName[] = "V3TV_ID";

  VP_STATUS           status;
  HANDLE              hkey;
  OBJECT_ATTRIBUTES   keyAttr;
  UNICODE_STRING      ucKeyName;
  //DWORD			  SID = ppdev->usSubSystemID;
 
  ANSI_STRING		  ansiValueName;
  UNICODE_STRING	  ucValueName;
  //UNICODE_STRING	  ucValueData;
  //ANSI_STRING       ansiValueData;
  
  ULONG				  LSID = 0;
 
  VideoDebugPrint((2, "SetRegistryValue - %s\n", SID));

  // open the key
  RtlInitUnicodeString(&ucKeyName, (PWSTR)tdfxDevNodeKey);
  InitializeObjectAttributes(&keyAttr,
                             &ucKeyName,
                             OBJ_CASE_INSENSITIVE,
                             NULL,
                             NULL);
  //open the existing key or create one if not.
  status = ZwCreateKey(&hkey, KEY_ALL_ACCESS, &keyAttr, 0, NULL , REG_OPTION_NON_VOLATILE, NULL);
  
    // convert value name to a unicode string
    //RtlInitAnsiString(&ansiValueName, (PCHAR)ValueName);
    //RtlAnsiStringToUnicodeString(&ucValueName, &ansiValueName, TRUE);
	RtlInitUnicodeString(&ucValueName, (PWSTR)ValueName);

    // append VID (121a) to SID, 121a is 3dfx vendor id number.
	LSID = SID;
	LSID = (LSID << 16) | VID;

	// write data to registry
    status = ZwSetValueKey(hkey,
						   &ucValueName,
                           0,
                           REG_DWORD,
                           &LSID,
                           sizeof(LSID));

	//Important! Release only when they are from conversions. 
	//If they are initialized unicode strings, don't do Free.
	//RtlFreeUnicodeString(&ucValueName);
	//RtlFreeUnicodeString(&ucKeyName);

    ZwClose(hkey);
	if (!NT_SUCCESS(status)) {
		  status = ERROR_DEV_NOT_EXIST;
		  return status;
	}else {
		status = 1;
		return status;
	}
 }
#ifdef MS_VIEW
#pragma warning ( pop )
#endif