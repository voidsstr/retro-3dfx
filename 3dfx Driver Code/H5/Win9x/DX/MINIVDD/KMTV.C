/* -*-c++-*- */
/* $Header: KMTV.C, 9, 10/11/00 8:53:43 PM, Brent$ */
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
** File name:   kmtv.c
**
** Description: Some Video shrink functions
**
** $Revision: 9$
** $Date: 10/11/00 8:53:43 PM$
**
** $History: KMTV.C $
** 
** *****************  Version 31  *****************
** User: Xingc        Date: 9/03/99    Time: 10:42a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Only set ddFLipOverlay call back function if PLD is not presented.
** This fixs a VPE PGF for regular V3 board.
** 
** *****************  Version 30  *****************
** User: Lpost        Date: 8/25/99    Time: 2:16p
** Updated in $/devel/h5/Win9x/dx/minivdd
** V3TV code merge into H5
** 
** *****************  Version 29  *****************
** User: Xingc        Date: 7/22/99    Time: 2:07p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Fix compiler error with the new setenv.bat
** 
** *****************  Version 28  *****************
** User: Xingc        Date: 7/22/99    Time: 11:59a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Use 3D for overlay shrink
** 
** *****************  Version 27  *****************
** User: Andrew       Date: 7/16/99    Time: 2:06p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Changed regBase and RegBase from single dword to array to support
** sparse register mapping
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
typedef int (FAR WINAPI *FARPROC)();


#pragma pack( 1 )
    #include "gdidefs.h"
    #include "dibeng.h"
#pragma pack()

#include "string.h"
#include "stdlib.h"
//Need define for vm event services
#define WIN40SERVICES 
#define WIN403SERVICES 

#include "3dfx.h"

#include "h3vdd.h"

#include "h3.h"

#include "h3g.h"

#define IS_32 1
#define MM 1

#include "shared.h"
#include "devtable.h"
#undef  THUNK32

#define NT9XDEVICEDATA GLOBALDATA
#define  ghwAC (SstCRegs*)(_FF(regBase[HWINFO_SST_CMDFIFOREGS_INDEX]))

#include "cmddefs.h"
#include "fifomgr.h"

#include <ddkmmini.h>
#include <stddef.h>
#include "kmvt.h"
#include "v3tvmap.h"

#pragma intrinsic (memcmp, memcpy,memset)


//from D3GLOBAL.H
#define TREX0   0
#define TREX1   1
#define TEXFMT_RGB_565            10
#define TEXFMT_ARGB_8888          15
#define CMD_START 1
#define TMU2CHIP(tmu)       (0x2 << (tmu))

#pragma VxD_LOCKED_DATA_SEG
#pragma VxD_LOCKED_CODE_SEG

#define BUSY_BIT        0x0004  // bit number to test for BUSY
#define CBUSY_BIT       0x0001  // bit number to test cursor busy

#define DDOVER_BOB	        	0x00200000
#define DDOVER_INTERLEAVED  	0x00800000
#define DDOVER_AUTOFLIP         0x00100000
#define DDFLIP_EVEN             0x00000002
#define DDFLIP_ODD              0x00000004

//H4 vidInStatus bitmasks
#define H4_VMI_BUFFER_MASK                  0x00030000
#define H4_VMI_FIELD_MASK                   0x00040000
#define H4_VID_IN_STATUS_CURLINE 0x94

//Fix for building w/95 or 98 DDK with DX 7/8 (New structure member name)
#ifdef DDIRQ_MISCELLANOUS
#define vddSetSkipPattern vddReserved1
#endif

#ifndef DEBUG
#define  KMVT_CMDFIFO_PROLOG( hwPtr )  CMDFIFO_PROLOG( hwPtr )
#endif

#pragma VxD_LOCKED_DATA_SEG
#pragma VxD_LOCKED_CODE_SEG

DWORD dwLockCounter = 0;

DWORD ddFlipOverlay(void);
DWORD ddGetPolarity(void);
DWORD DDGetIRQInfo(void);
DWORD DDIsOurIRQ(void);
DWORD DDEnableIRQ(void);
DWORD DDSkipNextField(void);
DWORD DDBobNextField(void);
DWORD DDSetState(void);
DWORD DDLock(void);
DWORD DDFlipVideoPort(void);
DWORD DDGetPolarity(void);
DWORD DDSetSkipPattern(void);
DWORD DDGetCurrentAutoflip(void);
DWORD DDGetPreviousAutoflip(void);
DWORD DDTransfer(void);
DWORD DDGetTransferStatus(void);

extern DWORD GETGBL_dwOvlOffset(DWORD);
BYTE AddTransfer(DDTRANSFERININFO      *lpTransferIn, DWORD *dwCurrentIndex, DWORD polarity);
//#define USE_VMM_CALLBACK_IRQ		// must have USE_VMM_CALLBACK also defined
HEVENT VXDINLINE
Call_When_VM_Ints_Enabled(void (__cdecl *pfnEvent)(), ULONG ulRefData)
{
    _asm mov edx, [ulRefData]
    _asm mov esi, [pfnEvent]
    VMMCall(Call_When_VM_Ints_Enabled)
    return 0;
}

//#define USE_VMM_CALLBACK
HEVENT VXDINLINE
Call_When_VM_Returns(void (__cdecl *pfnEvent)(), ULONG ulRefData)
{
	_asm mov eax, 1
    _asm mov edx, [ulRefData]
    _asm mov esi, [pfnEvent]
    VMMCall(Call_When_VM_Returns)
    return 0;
}

#ifdef USE_EVENT_TRANSFER
VOID TransferVMEvent( VOID );
#else
VOID TransferVMEvent( DWORD );
#endif
DWORD QueryForWDMVXD();
DWORD VDDtoWDMTellCapture(DWORD frame, DWORD polarity, BOOL vbiBuffer);

KMTVDATA kmtvInfo;

#define MAKE_FOURCC( ch0, ch1, ch2, ch3 )                       \
        ( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |    \
        ( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )

#define FOURCC_RAW8	    MAKE_FOURCC('R','A','W','8')
extern DWORD GETGBL_KMVTBuff(DWORD);
extern DWORD GETGBL_Pitch(DWORD);
#define H3_VMI_DEINTERLACE_WEAVE			0x00000010


/****************************************************
*  Allocate a page of shared system memory
*  for KMTVBUFF
*  Deallocate it when  lpvInBuffer[0] == 0
**************************************************/

DWORD AllocalSysBuff( DIOCPARAMETERS * lpParams)
{
   if( ( lpParams->cbInBuffer >= 8))
   {
       GLOBALDATA * ppdev =(GLOBALDATA *)(*((DWORD *)(lpParams->lpvInBuffer) + 2));

       if( *(DWORD *)(lpParams->lpvInBuffer))
       {
           if(!_FF(KMVTBuff))
           {
             //allocate one page
             _FF(KMVTBuff) =(DWORD) _PageAllocate( 1,  PG_SYS, (ULONG) NULL,
                0x0, 0x0, 0xFFFFFFFF, (ULONG)NULL, PAGEFIXED | PAGEZEROINIT);
           }
       }
       else
       {
           if(_FF(KMVTBuff))
           {
             _PageFree((PVOID)_FF(KMVTBuff), 0);
             _FF(KMVTBuff) = 0;
           }
       }
       return 0;
    }
    else
        return 1;
}

WORD SetBusy( WORD * lpBusy, WORD wFlag)
{
      _asm{
        mov   ax, wFlag
        mov   ebx, lpBusy
        bts   WORD PTR [ebx],ax
        jnc   NotBusy
      }
      return 1;
NotBusy:
      return 0;
}// SetBusy


void ClearBusy(WORD *lpBusy, WORD wFlag)
{
   _asm{
       mov  ax, wFlag
       mov  ebx, lpBusy
       btr  WORD PTR [ebx], ax
    }
}


DWORD __stdcall GetKernelInfo(DIOCPARAMETERS * lpParams)
{
   LPDDMINIVDDTABLE lpVddTable;

   if( lpParams->cbOutBuffer < sizeof( DDMINIVDDTABLE ))
       return 1;
   lpParams->lpcbBytesReturned = sizeof(DDMINIVDDTABLE);

   lpVddTable = (LPDDMINIVDDTABLE)lpParams->lpvOutBuffer;
   
   lpVddTable->dwMiniVDDContext = 0x1;     //any number now
   lpVddTable->vddFlipOverlay = ddFlipOverlay;


   memset (&kmtvInfo, 0, sizeof (kmtvInfo));   
   kmtvInfo.pDev = FindActiveBanshee();
   kmtvInfo.Context =  lpVddTable->dwMiniVDDContext;

   if(kmtvInfo.pDev->lpDriverData->PLDRevisionID)
   {
    lpVddTable->vddGetIRQInfo = DDGetIRQInfo;
    lpVddTable->vddIsOurIRQ = DDIsOurIRQ;
    lpVddTable->vddEnableIRQ = DDEnableIRQ;   
    lpVddTable->vddSkipNextField = DDSkipNextField;
    lpVddTable->vddBobNextField = DDBobNextField;
    lpVddTable->vddSetState = DDSetState;
    lpVddTable->vddLock = DDLock;
    lpVddTable->vddGetPolarity = DDGetPolarity;
    lpVddTable->vddFlipVideoPort = DDFlipVideoPort;
    lpVddTable->vddSetSkipPattern = DDSetSkipPattern;
    lpVddTable->vddGetCurrentAutoflip = DDGetCurrentAutoflip;
    lpVddTable->vddGetPreviousAutoflip = DDGetPreviousAutoflip;
    lpVddTable->vddTransfer = DDTransfer;
    lpVddTable->vddGetTransferStatus = DDGetTransferStatus;
   }

   return 0;   

}

/**********************************************************************
*   DESCRIPTION: Flips the overlay to the target surface.
*                For BOB and autofliping, it is only called when Vport 
*                is not involved, accroding to Scott McDonald.               
*   ENTRY:
*	   ESI	LPDDFLIPOVERLAYINFO
*		    DWORD 		dwSize
*		    LPDDSURFACEDATA	lpCurrentSurface
*		    LPDDSURFACEDATA	lpTargetSurface
*		    DWORD 		dwFlags
*	   EDI  NULL
*
*   EXIT:
*          EAX	0 = success, 1 = error
*
**********************************************************************/
DWORD ddFlipOverlay(void)
{
  DDFLIPOVERLAYINFO *lpFlipInfo;
  LPDDSURFACEDATA   lpTarget;
  DWORD dwSurfAddr;
  GLOBALDATA * ppdev;
  WORD       * pFlags;
  OVLBLTPARAMS * lpBltParams;
  BOOL       fHWBusy = TRUE;
  DWORD        dwDstBaseAddr;
  DWORD        dwTxtrBaseAddr;
  SstIORegs   *ghwIO;
  SstGRegs   *ghw2D;
  SstRegs   *ghw3D;

   _asm mov lpFlipInfo, esi
   
   lpTarget= (LPDDSURFACEDATA)lpFlipInfo->lpTargetSurface;

   ppdev = (GLOBALDATA *)lpTarget->dwDriverReserved1;

   lpBltParams =
       (OVLBLTPARAMS *)( (DWORD )ppdev + (DWORD)lpTarget->dwDriverReserved2);

   pFlags = lpBltParams->pDebAddr;
   if( !pFlags)
       pFlags = (WORD *)_FF(lpDeFlags);

   //The check deVersion and deType in DIBENGINE
   //if it is not a correct one then we cannot use deFlags too
   if( *(WORD *)((DWORD)pFlags + offsetof(DIBENGINE, deVersion) 
                        - offsetof(DIBENGINE,deFlags)) != 0x400)
           return 0;

   if( *(WORD *)((DWORD)pFlags + offsetof(DIBENGINE, deType) 
                        - offsetof(DIBENGINE,deFlags)) != 0x5250)
           return 0;

   if(lpBltParams->wShrinkFlags & OVL_SHRINK)
   {
       if(lpTarget->dwOverlayDestHeight)
         return 0;      //not sync
        
   }
   else
   {
       if(!lpTarget->dwOverlayDestHeight)
         return 0;      //not sync
   }

   dwSurfAddr = lpTarget->dwSurfaceOffset;


   ghw2D = (SstGRegs*)(_FF(regBase[HWINFO_SST_2DREGS_INDEX]));
   ghw3D = (SstRegs*)(_FF(regBase[HWINFO_SST_3DREGS_INDEX]));
   ghwIO = (SstIORegs*)(_FF(regBase[HWINFO_SST_IOREGS_INDEX]));


   if(!lpTarget->dwOverlayDestHeight)
   {
      //in this case we are shrinking overlay
      dwTxtrBaseAddr = _FF(dwTxtrSurfAddr1);
      if(_FF(dwCurrentSKSurf) & 0x1)
      {
       dwDstBaseAddr = _FF(dwShrinkSurfAddr2);
      }
      else
      {
       dwDstBaseAddr = _FF(dwShrinkSurfAddr1);
      }
      if(_FF(dwCurrentSKSurf) & 0x10000)  //there are two shrink surfaces
         (WORD)_FF(dwCurrentSKSurf) += 1;
   }
   else
   {
       if(lpTarget->dwOverlayFlags & DDOVER_INTERLEAVED)
       {
          if(lpFlipInfo->dwFlags & DDFLIP_ODD )
          {
            dwSurfAddr += lpTarget->lPitch; //set in ring0 for pitch
            dwSurfAddr &= ~SSTG_IS_TILED;
          }
          else if(lpFlipInfo->dwFlags & DDFLIP_EVEN )
            dwSurfAddr |= 0x80000000;
       }
       else
       {
          if(lpFlipInfo->dwFlags & DDFLIP_EVEN )
            dwSurfAddr |= 0x80000000;
          else
            dwSurfAddr &= ~SSTG_IS_TILED;
       }

       dwDstBaseAddr = dwSurfAddr;
   }

   if( dwLockCounter++ == 0) 
   {
      //check busy bit
      if( !SetBusy(pFlags, BUSY_BIT))
      {

        if(!SetBusy(&_FF(cursorBusy), CBUSY_BIT))
        {
            if(!(GET(ghwIO->status) & SST_BUSY))
            {
               CMDFIFO_PROLOG(hwPtr);
               _FF(gdiFlags) |= SDATA_GDIFLAGS_2D_DIRTY;
               _FF(gdiFlags) &= ~SDATA_GDIFLAGS_KMTV_FLAG;

               if(!lpTarget->dwOverlayDestHeight)
               {
                 if(!_FF(dd3DSurfaceCount) && IS_NAPALM(_FF(VendorDeviceID))&&dwTxtrBaseAddr)
                 {

                     CMDFIFO_CHECKROOM(hwPtr,
                        13 + PH3_SIZE + 20 +  PH1_SIZE + 2 + PH4_SIZE +3);
//                        13 + PH3_SIZE + 20 + 7 * PH1_SIZE + 12 + 3 * ( PH4_SIZE +3));

				     SETPH(hwPtr, CMDFIFO_BUILD_PK2(
                        dstBaseAddrBit
                        | dstFormatBit
                        | ropBit
                        | srcBaseAddrBit
                        | clip1minBit
                        | clip1maxBit
                        | srcFormatBit
                        | srcSizeBit
                        | srcXYBit
                        | dstSizeBit
                        | dstXYBit
                        | commandBit ) );
                    SETPD(hwPtr, ghw2D->dstBaseAddr, dwTxtrBaseAddr);
                    SETPD(hwPtr, ghw2D->dstFormat, lpBltParams->txtrbltDstFormat); 
                    SETPD(hwPtr, ghw2D->rop,
                    (SSTG_ROP_SRC << 16 )| (SSTG_ROP_SRC << 8 ) | SSTG_ROP_SRC );
                    SETPD(hwPtr, ghw2D->srcBaseAddr, dwSurfAddr);
                    SETPD(hwPtr, ghw2D->clip1min,  lpBltParams->clip1min);     
                    SETPD(hwPtr, ghw2D->clip1max,  lpBltParams->txtrclip1max);     
                    SETPD(hwPtr, ghw2D->srcFormat, lpBltParams->bltSrcFormat); 
                    SETPD(hwPtr, ghw2D->srcSize,   lpBltParams->bltSrcSize);     
                    SETPD(hwPtr, ghw2D->srcXY,     lpBltParams->bltSrcXY);          
                    SETPD(hwPtr, ghw2D->dstSize,   lpBltParams->txtrbltDstSize);     
                    SETPD(hwPtr, ghw2D->dstXY,     lpBltParams->bltDstXY);          
                    SETPD(hwPtr, ghw2D->command,
                        (SSTG_ROP_SRC << SSTG_ROP0_SHIFT )|
                        SSTG_BLT | SSTG_GO | SSTG_CLIPSELECT );

				    dwTxtrBaseAddr += lpBltParams->srcAddrOff;
 				    dwTxtrBaseAddr = (dwTxtrBaseAddr & 0x1FFFFFF) + ((dwTxtrBaseAddr & 0x2000000) >> 24);
                    //do texture shrinking
//                    while(GET(ghwIO->status)&(SST_BUSY | SST_TMU_BUSY))
//                        ;

//                    SETPH( hwPtr, CMDFIFO_BUILD_PK1( 2, 1, clipLeftRight, 0xf ) );
//                    SETPD( hwPtr, ghw3D->clipLeftRight, lpBltParams->dstWidth);
//                    SETPD( hwPtr, ghw3D->clipBottomTop, lpBltParams->dstHeight);

                    SETPH( hwPtr, CMDFIFO_BUILD_PK1( 2, 1, colBufferAddr, 0x0 ) );
                    SETPD( hwPtr, ghw3D->colBufferAddr,   dwDstBaseAddr);
                    SETPD( hwPtr, ghw3D->colBufferStride, lpBltParams->dstPitch);

				    //set sextureMode for TMU0
                    //loads texture and enbales bi-linear filter


/*  SETPH( hwPtr, CMDFIFO_BUILD_PK4(R0|R1|R3, textureMode, TMU2CHIP(TREX1)) );
  SETPD( hwPtr, SST_TREX(ghw0,TREX1)->textureMode, SST_TCLAMPS|
                                                     SST_TCLAMPT|
                         (TEXFMT_ARGB_8888 << SST_TFORMAT_SHIFT));
  SETPD( hwPtr, SST_TREX(ghw0,TREX1)->tLOD, lpBltParams->tLOD );
  SETPD( hwPtr, SST_TREX(ghw0,TREX1)->texBaseAddr, dwTxtrBaseAddr);
*/
					SETPH( hwPtr, CMDFIFO_BUILD_PK4(R0|R1|R3, textureMode, TMU2CHIP(TREX0)));
					SETPD( hwPtr, SST_TREX(ghw0,TREX0)->textureMode,
                                                     SST_TCLAMPS|
                                                     SST_TCLAMPT|
                         (TEXFMT_ARGB_8888 << SST_TFORMAT_SHIFT)|
                        SST_TC_REPLACE | SST_TCA_REPLACE |
                        SST_TMINFILTER | SST_TMAGFILTER);
                    SETPD( hwPtr, SST_TREX(ghw3D,TREX0)->tLOD, lpBltParams->tLOD );
                    SETPD( hwPtr, SST_TREX(ghw3D,TREX0)->texBaseAddr, dwTxtrBaseAddr);
/*
                    // decal texture, filtered, no fog, no alpha blending, no z buffering
                    SETPH( hwPtr, CMDFIFO_BUILD_PK1( 4, 1, fbzColorPath, 0xf ) );
                    SETPD( hwPtr, ghw3D->fbzColorPath, SST_ENTEXTUREMAP | SST_PARMADJUST |
                            SST_RGBAZ_CLAMP );
                    SETPD( hwPtr, ghw3D->fogMode, 0 );
                    SETPD( hwPtr, ghw3D->alphaMode, 0 );
                    SETPD( hwPtr, ghw3D->fbzMode, SST_RGBWRMASK );
				 	 //need a NOP
					SETPH( hwPtr, CMDFIFO_BUILD_PK1(1, 0, nopCMD, 0xF));
  					SETPD( hwPtr, ghw->nopCMD,0);
					 //combineMode for LFB
                    SETPH( hwPtr, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 1 ) );
                    SETPD( hwPtr, SST_CHIP(ghw, CHIP_FBI)->combineMode, SST_CM_CC_OTHERSELECT_TRGB |
                                    SST_CM_USE_COMBINE_MODE );
                    //combineMode for TMU0
                    SETPH( hwPtr, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 2 ) );
                    SETPD( hwPtr, SST_CHIP(ghw , CHIP_TMU0)->combineMode, SST_CM_TC_LOCALSELECT_LOCAL_TRGB |
                                    SST_CM_USE_COMBINE_MODE );
                    //combineMode for TMU1
                   // SETPH( hwPtr, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 4 ) );
                   // SETPD( hwPtr, SST_CHIP(ghw, CHIP_TMU1)->combineMode, SST_CM_TC_LOCALSELECT_LOCAL_TRGB |
                   //                 SST_CM_USE_COMBINE_MODE );
 
                    SETPH( hwPtr, CMDFIFO_BUILD_PK4( R0|R1|R2, renderMode, 0 ) );
                    SETPD( hwPtr, ghw3D->renderMode,SST_RM_32BPP       |
								   SST_RM_YORIGIN_SELECT|	
                                   SST_RM_RED_WMASK   |
                                   SST_RM_GREEN_WMASK |
                                   SST_RM_BLUE_WMASK );
                    SETPD( hwPtr, ghw3D->stencilMode, SST_STENCIL_MODE_DISABLE);
                    SETPD( hwPtr, ghw3D->stencilOp, 0); // D3DSTENCILOP_KEEP
*/
               
                    SETPH( hwPtr, CMDFIFO_BUILD_PK3( CMD_START, 4, (SST_SETUP_ST0 | SST_SETUP_W0 |SST_SETUP_FAN), 1) );
                    SETFPD( hwPtr, ghw3D->sVx, 0.0f );
                    SETFPD( hwPtr, ghw3D->sVy, 0.0f );
                    SETFPD( hwPtr, ghw3D->sOow0, 1.f );
                    SETFPD( hwPtr, ghw3D->sSow0, 0.f );
                    SETFPD( hwPtr, ghw3D->sTow0, 0.f );
				   
				    SETFPD( hwPtr, ghw3D->sVx,  0.0f );
                    SETPD( hwPtr, ghw3D->sVy,  lpBltParams->dstHeight);
                    SETFPD( hwPtr, ghw3D->sOow0, 1.f );
                    SETFPD( hwPtr, ghw3D->sSow0, 0.0f );
                    SETPD( hwPtr, ghw3D->sTow0, lpBltParams->maxt);

                 	SETPD( hwPtr, ghw3D->sVx,   lpBltParams->dstWidth);
                    SETPD( hwPtr, ghw3D->sVy,   lpBltParams->dstHeight);
                    SETFPD( hwPtr, ghw3D->sOow0, 1.f );
                    SETPD( hwPtr, ghw3D->sSow0, lpBltParams->maxs);
                    SETPD( hwPtr, ghw3D->sTow0, lpBltParams->maxt);

				    SETPD( hwPtr, ghw3D->sVx,   lpBltParams->dstWidth);
                    SETFPD( hwPtr, ghw3D->sVy,   0.0f);
                    SETFPD( hwPtr, ghw3D->sOow0, 1.f );
                    SETPD( hwPtr, ghw3D->sSow0, lpBltParams->maxs);
                    SETFPD( hwPtr, ghw3D->sTow0, 0.0f);
				 
                   // BUMP(13 + PH3_SIZE + 20 + 7 * PH1_SIZE + 12 + 4 * ( PH4_SIZE +3));
                    BUMP(13 + PH3_SIZE + 20 +  PH1_SIZE + 2 +  PH4_SIZE +3);
                 }
                 else
                 {
                    CMDFIFO_CHECKROOM(hwPtr,13);
                    SETPH(hwPtr, CMDFIFO_BUILD_PK2(
                        dstBaseAddrBit
                        | dstFormatBit
                        | ropBit
                        | srcBaseAddrBit
                        | clip1minBit
                        | clip1maxBit
                        | srcFormatBit
                        | srcSizeBit
                        | srcXYBit
                        | dstSizeBit
                        | dstXYBit
                        | commandBit ) );
                    SETPD(hwPtr, ghw2D->dstBaseAddr, dwDstBaseAddr);
                    SETPD(hwPtr, ghw2D->dstFormat, lpBltParams->bltDstFormat); 
                    SETPD(hwPtr, ghw2D->rop,
                    (SSTG_ROP_SRC << 16 )| (SSTG_ROP_SRC << 8 ) | SSTG_ROP_SRC );
                    SETPD(hwPtr, ghw2D->srcBaseAddr, dwSurfAddr);
                    SETPD(hwPtr, ghw2D->clip1min,  lpBltParams->clip1min);     
                    SETPD(hwPtr, ghw2D->clip1max,  lpBltParams->clip1max);     
                    SETPD(hwPtr, ghw2D->srcFormat, lpBltParams->bltSrcFormat); 
                    SETPD(hwPtr, ghw2D->srcSize,   lpBltParams->bltSrcSize);     
                    SETPD(hwPtr, ghw2D->srcXY,     lpBltParams->bltSrcXY);          
                    SETPD(hwPtr, ghw2D->dstSize,   lpBltParams->bltDstSize);     
                    SETPD(hwPtr, ghw2D->dstXY,     lpBltParams->bltDstXY);          
                    SETPD(hwPtr, ghw2D->command,
                        (SSTG_ROP_SRC << SSTG_ROP0_SHIFT )|
                        SSTG_STRETCH_BLT | SSTG_GO | SSTG_CLIPSELECT );
                     BUMP(13);
                 }    

                 if(lpFlipInfo->dwFlags & DDFLIP_EVEN )
                    dwDstBaseAddr |= 0x80000000;
                 else
                    dwDstBaseAddr &= ~SSTG_IS_TILED;
               }

               CMDFIFO_CHECKROOM(hwPtr, 4);
               SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, leftOverlayBuf, 0xF ));
               SETPD(hwPtr, ghw3D->leftOverlayBuf, dwDstBaseAddr);
               SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, swapbufferCMD, 0xF) );
               SETPD(hwPtr, ghw3D->swapbufferCMD, 1);
               BUMP(4);
   
               CMDFIFO_EPILOG(hwPtr);
               fHWBusy = FALSE;
            }
            ClearBusy(&_FF(cursorBusy), CBUSY_BIT);
        }

          ClearBusy(pFlags, BUSY_BIT);
      }

/*      if (fHWBusy & !(_FF(gdiFlags) & SDATA_GDIFLAGS_KMTV_FLAG))
      {
         DWORD wrPtr;
         KMVTBUFF * bltBuff;
         KMVT_CMDFIFO_PROLOG(hwPtr);
         bltBuff = (KMVTBUFF *)_FF(KMVTBuff);
         hwPtr = bltBuff->data;
         bltBuff->dwSize = 4;
         wrPtr = 0;
         hwIndex = wrPtr;

         if(!lpTarget->dwOverlayDestHeight)
         {

             srcFormat = lpBltParams->bltSrcFormat & (~0x3FFF);
             dstFormat = lpBltParams->bltDstFormat & (~0x3FFF);

             dstFormat |= (DWORD)lpTarget->dwDriverReserved3;
             srcFormat |=lpTarget->lPitch;


             bltBuff->dwSize = 17;
             SETPH(hwPtr, CMDFIFO_BUILD_PK2(
                  dstBaseAddrBit
                  | dstFormatBit
                  | ropBit
                  | srcBaseAddrBit
                  | clip1minBit
                  | clip1maxBit
                  | srcFormatBit
                  | srcSizeBit
                  | srcXYBit
                  | dstSizeBit
                  | dstXYBit
                  | commandBit ) );
   
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            
            SETPD(hwPtr, ghw2D->dstBaseAddr, dwDstBaseAddr);
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->dstFormat, dstFormat); 
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->rop,
                (SSTG_ROP_SRC << 16 )| (SSTG_ROP_SRC << 8 ) | SSTG_ROP_SRC );
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->srcBaseAddr, dwSurfAddr);
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->clip1min,  lpBltParams->clip1min);     
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->clip1max,  lpBltParams->clip1max);     
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->srcFormat, srcFormat); 
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->srcSize,   lpBltParams->bltSrcSize);     
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->srcXY,     lpBltParams->bltSrcXY);          
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->dstSize,   lpBltParams->bltDstSize);     
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->dstXY,     lpBltParams->bltDstXY);          
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->command,
                (SSTG_ROP_SRC << SSTG_ROP0_SHIFT )|
                SSTG_STRETCH_BLT | SSTG_GO | SSTG_CLIPSELECT );
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;

            if(lpFlipInfo->dwFlags & DDFLIP_EVEN )
                   dwDstBaseAddr |= 0x80000000;
             else
                   dwDstBaseAddr &= ~SSTG_IS_TILED;

        }
        SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, leftOverlayBuf, 0xF ));
        wrPtr = (wrPtr + 1) & 0xFF;
        hwIndex = wrPtr;
        SETPD(hwPtr, ghw3D->leftOverlayBuf, dwDstBaseAddr);
        wrPtr = (wrPtr + 1) & 0xFF;
        hwIndex = wrPtr;
        SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, swapbufferCMD, 0xF) );
        wrPtr = (wrPtr + 1) & 0xFF;
        hwIndex = wrPtr;
        SETPD(hwPtr, ghw3D->swapbufferCMD, 1);

        _FF(gdiFlags) |= SDATA_GDIFLAGS_KMTV_FLAG;
      }    */
   } 
   dwLockCounter--;
   return 0;
}    

/**********************************************************************
*   DDGetIRQInfo
*
*   DESCRIPTION: If the Mini VDD is already managing the IRQ, this
*          function returns that information; otherwise, it returns the
*          IRQ number assigned to the device so DDraw can manage the IRQ.
*
*          The returning the IRQ number, it is important that it get the
*          value assigned by the Config Manager rather than simply get
*          the value from the hardware (since it can be remapped by PCI).
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  NULL
*          EDI  LPDDGETIRQINFO
*                   DWORD dwSize;
*                   DWORD dwFlags;
*                   DWORD dwIRQNum;
*
*   EXIT:
*          EAX  0 = success, 1 = error
*
*
**********************************************************************/
DWORD DDGetIRQInfo(void)
{
   DDGETIRQINFO	*lpGetIrqInfo;
   _asm mov lpGetIrqInfo, edi
// _asm int 1;
   //Debug_Printf("KMVT - GetIRQInfo\n");

   //return handled since we have our own irq handler in h3irq and
   //will manage the IRQ ourselves
   lpGetIrqInfo->dwSize = sizeof (DDGETIRQINFO);
   lpGetIrqInfo->dwFlags = IRQINFO_HANDLED;
   return 0;
}

/**********************************************************************
*   DDEnableIRQInfo
*
*   DESCRIPTION: Notifies the Mini VDD which IRQs should be enabled.  If
*          a previously enabled IRQ is not specified in this call,
*          it should be disabled.
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDENABLEIRQINFO
*                   DWORD dwSize
*                   DWORD dwIRQSources
*                   DWORD dwLine
*                   DWORD IRQCallback
*                   DWORD dwContext
*          EDI  NULL
*
*   EXIT:
*          EAX  0 = success, 1 = error
*
*
**********************************************************************/
//h3 intrCtrl bitmasks
#define H3_VMI_INT_ENABLE                   0x00200000
#define H3_VMI_INTERRUPT					     0x00800000
#define H3_VSYNC_INT_ENABLE				     0x00000004
#define H3_VSYNC_INTERRUPT					     0x00000100

DWORD DDEnableIRQ(void)
{

   DDENABLEIRQINFO    *lpEnableIRQ;
   DWORD 	       dwReg;
   int i;

   _asm mov lpEnableIRQ, esi
#ifdef DEBUG
   Debug_Printf("KMVT - Enable IRQ sources %04.4x (vsync 0x01 vport 0x04)\n", lpEnableIRQ->dwIRQSources);
#endif
//_asm int 1;
   if (lpEnableIRQ->dwIRQSources != kmtvInfo.dwIRQSources) {
	dwReg = (((SstRegs *)(kmtvInfo.pDev->RegBase[HWINFO_SST_3DREGS_INDEX]))->intrCtrl &
                0x7FFFFFFF);
	if (lpEnableIRQ->dwIRQSources & DDIRQ_VPORT0_VSYNC)
	{
	   // VMI_IRQ_ENABLE;
	   kmtvInfo.pDev->dwIMask |= H3_VMI_INT_ENABLE;
	   dwReg |= H3_VMI_INT_ENABLE;

	   if (!kmtvInfo.bInitialized) {
	       KMVTBUFF * lpKMVTBuff;
	       lpKMVTBuff = (KMVTBUFF *)GETGBL_KMVTBuff((DWORD)kmtvInfo.pDev->lpDriverData);
	       kmtvInfo.dwVBILines = lpKMVTBuff->dwVBILines;
		   kmtvInfo.Num = 0;
		   kmtvInfo.dwCurrentTransfer = 0;
		   kmtvInfo.dwQueuedTransfers = 0;
		   kmtvInfo.dwBusMasterTransfers = 0;
	       kmtvInfo.dwTransferID = 0;
	       kmtvInfo.dwVBITransferID = 0;
		   kmtvInfo.dwFieldCount = 0;
		   kmtvInfo.bReportForVideo = FALSE;		// default to vbi
		   memset (kmtvInfo.Transfer, sizeof (TRANSFER_DATA) * MAX_TRANSFERS, 0);
		   kmtvInfo.bInitialized = 1;
	   }
	}
	else
	{
	   //  VMI_IRQ_DISABLE;
	   kmtvInfo.pDev->dwIMask &= ~H3_VMI_INT_ENABLE;
	   dwReg &= ~H3_VMI_INT_ENABLE;
           kmtvInfo.bInitialized = 0;
		
#ifndef USE_VMM_CALLBACK
          //Just by chance there are any transfers still pending, be sure to cancel them.
	   for (i=0; i<MAX_TRANSFERS; i++)
	   {
		if (kmtvInfo.Transfer[i].dwFlags & TRANSFER_QUEUED) 
		{
			if (kmtvInfo.Transfer[i].EventHandle)
		     Cancel_Global_Event(kmtvInfo.Transfer[i].EventHandle);
		}
  	   }
#endif	
  	   kmtvInfo.dwCurrentTransfer = 0;
  	   kmtvInfo.dwQueuedTransfers = 0;
	   kmtvInfo.dwBusMasterTransfers = 0;
	   kmtvInfo.dwTransferID = 0;
	   kmtvInfo.dwVBITransferID = 0;
	   kmtvInfo.dwFieldCount = 0;
	   kmtvInfo.bReportForVideo = FALSE;		// default to vbi
	   memset (kmtvInfo.Transfer, sizeof (TRANSFER_DATA) * MAX_TRANSFERS, 0);
	}
#if 0
	if (lpEnableIRQ->dwIRQSources & DDIRQ_DISPLAY_VSYNC)
	{
	   //VSYNC_IRQ_ENABLE;
	   kmtvInfo.pDev->dwIMask |= H3_VSYNC_INT_ENABLE;
	   dwReg |= H3_VSYNC_INT_ENABLE;
	}
	else
	{
	   //VSYNC_IRQ_DISABLE;
	   kmtvInfo.pDev->dwIMask &= ~H3_VSYNC_INT_ENABLE;
	   dwReg &= ~H3_VSYNC_INT_ENABLE;
	}
#endif
//kmtvInfo.Context = lpEnableIRQ->dwContext;
	kmtvInfo.IRQCallback = (lpEnableIRQ->dwIRQSources) ? lpEnableIRQ->IRQCallback : 0;
	kmtvInfo.dwIRQSources = lpEnableIRQ->dwIRQSources;
	((SstRegs *)((kmtvInfo.pDev)->RegBase[HWINFO_SST_3DREGS_INDEX]))->intrCtrl = dwReg;

     }
	return 0;
}


/**********************************************************************
*   DDIsOurIRQ
*
*  DESCRIPTION: Called when the VDD's IRQ handler is triggered.  This
*          determines if the IRQ was caused by our VGA and if so, it
*          clears the IRQ and returns which event(s) generated the IRQ.
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  NULL
*
*   EXIT:
*          EDI  IRQ source flags
*          EAX  0 = success, 1 = error
*
*   NOTE that if ANYTHING weird is going on, like a new version of the
*   VDD resulting in traps in other people's code, it's most likely
*   because edi returned by this function isn't accurately reflecting
*   the interrupt that was generated.  It's crucial that EDI be accurate,
*   so that the system components acknowledge the interrupt correctly.
*
*
**********************************************************************/
DWORD DDIsOurIRQ(void)
{
  
   //This shouldn't occur since we are handling the interupts as stated
   //in ddgetirqinfo when it returned handled 
   int IrqCtrl = ((SstRegs *)(kmtvInfo.pDev->RegBase[HWINFO_SST_3DREGS_INDEX]))->intrCtrl;
   if (IrqCtrl & H3_VSYNC_INTERRUPT) 
   {
      _asm mov edi, DDIRQ_DISPLAY_VSYNC
   }
   if (IrqCtrl & H3_VMI_INTERRUPT) 
   {
      _asm mov edi, DDIRQ_VPORT0_VSYNC
   }

   return 0;
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
*          EAX  dwMiniVDDContext
*          ESI  LPDDSKIPINFO
*                   DWORD dwSize
*                   LPDDVIDEOPORTDATA video port
*          EDI  NULL
*
*   EXIT:
*          EAX  0 = success, 1 = error
*
**********************************************************************/
DWORD DDSkipNextField(void)
{

   DDSKIPINFO    *lpSkipInfo;
   _asm mov lpSkipInfo, esi
#ifdef DEBUG
   Debug_Printf("KMVT - DDSkipNextField\n");
#endif
//_asm int 1;

   if (lpSkipInfo->dwSkipFlags & DDSKIP_SKIPNEXT)
       kmtvInfo.bSkipNextField = TRUE; 
   else if (lpSkipInfo->dwSkipFlags & DDSKIP_ENABLENEXT)
       kmtvInfo.bSkipNextField = FALSE;
   else
       return 1;

   return 0;
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
*          EAX  dwMiniVDDContext
*          ESI  LPDDBOBINFO
*                   DWORD dwSize
*                   LPDDSURFACE lpSurface
*          EDI  NULL
*
*   EXIT:
*          EAX  0 = success, 1 = error
*
*
**********************************************************************/
DWORD DDBobNextField(void)
{

   DDBOBINFO	*lpBobInfo;
   _asm mov lpBobInfo, esi

#ifdef DEBUG
   Debug_Printf("KMVT - DDBobNextField\n");
#endif
//_asm int 1;
   return 0;
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
*          EAX  dwMiniVDDContext
*          ESI  LPDDSTATEININFO
*                   DWORD dwSize
*                   LPDDSURFACEDATA overlay surface
*          EDI  LPDDSTATEOUTINFO
*                   DWORD dwSize
*                   DWORD dwSoftwareAutoflip
*                   DWORD dwSurfaceIndex        ; Return Current hardware autoflip
*
*   DURING:
*           EDI OverlaySuface
*           EBX Overlay Flags
*           ECX Overlay Pitch
*           EDX Overlay Src Height
* 
*   EXIT:
*          EAX  0 = success, 1 = error
*
**********************************************************************/
DWORD DDSetState(void)
{

  DDSTATEOUTINFO	*lpStateOut;
  DDSTATEININFO		*lpStateIn;
   _asm mov lpStateIn, esi
   _asm mov lpStateOut, edi

#ifdef DEBUG
   Debug_Printf("KMVT - DDSetState\n");
#endif
//_asm int 1;
   return 0;
}

/**********************************************************************
*   DDLock
*
*   DESCRIPTION: Called when the client wants to lock the surface to
*       access the frame buffer. The driver doens't have to do anything,
*       but it can if it needs to.
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDLOCKININFO
*                   DWORD dwSize
*                   LPDDSURFACEDATA surface
*          EDI  LPDDLOCKOUTINFO
*                   DWORD dwSize
*                   DWORD Pointer to a pointer to the surface
*
*   EXIT:
*          EAX  0 = success, 1 = error
*
**********************************************************************/
DWORD DDLock(void)
{
   DDLOCKININFO		*lpLockInInfo;
   DDLOCKOUTINFO	*lpLockOutInfo;
   LPDDSURFACEDATA lpSurf;
    KMVTBUFF * lpKMVTBuff;
   _asm mov lpLockInInfo, esi
   _asm mov lpLockOutInfo, edi

#ifdef DEBUG
   Debug_Printf("KMVT - DDLock\n");
#endif
   // modify to reflect decimated size
    lpKMVTBuff = (KMVTBUFF *)GETGBL_KMVTBuff((DWORD)kmtvInfo.pDev->lpDriverData);
	if (lpKMVTBuff)
	{
		lpSurf = (LPDDSURFACEDATA)lpLockInInfo->lpSurfaceData;
		lpSurf->dwWidth = lpKMVTBuff->dwDataWidth;
		lpSurf->dwHeight = lpKMVTBuff->dwDataHeight;
	}

   return 0;
}

/**********************************************************************
*   DDFlipVideoPort
*
*   DESCRIPTION: Flips the video port to the target surface.
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDFLIPVIDEOPORTINFO
*                   DWORD dwSize
*                   LPDDVIDEOPORTDATA video port info
*                   LPDDSURFACEDATA current surface
*                   LPDDSURFACEDATA target surface
*                   DWORD dwFlipVPFlags
*          EDI  NULL
*
*   EXIT:
*          EAX  0 = success, 1 = error
*
*
**********************************************************************/
DWORD DDFlipVideoPort(void)
{

   DDFLIPVIDEOPORTINFO	*lpFlipVideoPort;
   _asm mov lpFlipVideoPort, esi
#ifdef DEBUG
   Debug_Printf("KMVT - DDFlipVideoPort\n");
#endif
//_asm int 1;
   return 0;
}

/**********************************************************************
*   DDGetPolarity
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
DWORD DDGetPolarity(void)
{

   DDPOLARITYININFO   *lpPolarityIn;
   DDPOLARITYOUTINFO  *lpPolarityOut;
    KMVTBUFF * lpKMVTBuff;
   _asm mov lpPolarityIn, esi
   _asm mov lpPolarityOut, edi

    lpKMVTBuff = (KMVTBUFF *)GETGBL_KMVTBuff((DWORD)kmtvInfo.pDev->lpDriverData);
	if (lpKMVTBuff && lpKMVTBuff->bVMIPLDinUse)
	{
		lpPolarityOut->bPolarity = lpKMVTBuff->bLastFieldEven;
	}
	else
	{
	   if (((((SstIORegs *)(kmtvInfo.pDev->RegBase[HWINFO_SST_IOREGS_INDEX]))->vidCurrentLine) >> 18) & 0x01)
		lpPolarityOut->bPolarity = TRUE;
	   else
		lpPolarityOut->bPolarity = FALSE;
	}
#ifdef DEBUG
   Debug_Printf("KMVT - DDGetPolarity %d\n", lpPolarityOut->bPolarity);
#endif
   return 0;
}

/**********************************************************************
*   DDSkipPattern
*
*  DESCRIPTION: Sets the skip pattern that is to start on the next video field.
*                If device returns an error, DDraw perofms the appropriate skipping
*                using IRQ logic and vddSkipNextField function
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDSETSKIPINFO
*          EDI  0
*
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
DWORD DDSetSkipPattern(void)
{

   DDSKIPINFO	*lpSkipInfo;
   _asm mov lpSkipInfo, esi
#ifdef DEBUG
   Debug_Printf("KMVT - DDSetSkipPattern\n");
#endif
//_asm int 1;
   return 0;
}

/**********************************************************************
*   DDGetCurrentAutoflip
*
*   DESCRIPTION: Returns the current surface receiving data from the
*       video port while autoflipping is taking palce.  Only called when
*   hardware autoflipping.
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDGETAUTOFLIPINFO
*                   DWORD               dwSize
*          EDI  LPDDGETAUTOFLIPINFO
*                   DWORD               dwSize
*                   DWORD               dwSurfaceIndex
*
*   EXIT:
*          EAX  0 = success, 1 = error
*
**********************************************************************/
DWORD DDGetCurrentAutoflip(void)
{

   DDGETAUTOFLIPININFO	*lpAutoFlipIn;
   DDGETAUTOFLIPOUTINFO *lpAutoFlipOut;
   BYTE bNewBuf, bBufModeSelect, bPolarity;
    KMVTBUFF * lpKMVTBuff;

   _asm mov lpAutoFlipIn, esi
   _asm mov lpAutoFlipOut, edi
//   Debug_Printf("KMVT - DDGetCurrentAutoflip\n");
	//Get previous buffer number
   bNewBuf = (BYTE)((((SstIORegs *)(kmtvInfo.pDev->RegBase[HWINFO_SST_IOREGS_INDEX]))->vidCurrentLine) >> 16) & 0x3;

   if (kmtvInfo.bReportForVideo)
   {

	   lpKMVTBuff = (KMVTBUFF *)GETGBL_KMVTBuff((DWORD)kmtvInfo.pDev->lpDriverData);
	   if (GETGBL_dwOvlOffset((DWORD)kmtvInfo.pDev->lpDriverData) 
		   && (kmtvInfo.pDev->dwIMask & H3_VSYNC_INT_ENABLE) 
		   &&  (lpKMVTBuff->dwStatus & INTERLEAVE_ON ))
	   {	// manual flipping
			bNewBuf = (BYTE)(lpKMVTBuff->dwStatus & BUFFER_IN_USE_MASK);
	   }
	   else
	   {
		   //Query buffer mode - single, double or triple
		    bBufModeSelect = (BYTE)((((SstIORegs *)(kmtvInfo.pDev->RegBase[HWINFO_SST_IOREGS_INDEX]))->vidInFormat) >> 9) & 0x3;
			if (lpKMVTBuff && lpKMVTBuff->bVMIPLDinUse)		// since this is on vbi - use hw
			{
				bPolarity = (BYTE)lpKMVTBuff->bLastFieldEven;
			}
			else
			{
			   bPolarity = (BYTE)((((SstIORegs *)(kmtvInfo.pDev->RegBase[HWINFO_SST_IOREGS_INDEX]))->vidCurrentLine) >> 18) & 0x01;
			}
			if (!(lpKMVTBuff->dwVidInFormat & H3_VMI_DEINTERLACE_WEAVE)		// changes on every field
				|| bPolarity)								// both change on even field
			{
				bNewBuf++;
				if (bNewBuf > bBufModeSelect)
					bNewBuf = 0;
			}
	   }
   }
   else
		bNewBuf = 0;

    lpAutoFlipOut->dwSurfaceIndex = bNewBuf;
#ifdef DEBUG
	if (kmtvInfo.dwFieldCount >= 180 && kmtvInfo.dwFieldCount < 190)
	{
	   Debug_Printf("KMVT - DDGetCurrentAutoflip - %d\n", bNewBuf);
	}
#endif

   return 0;
}


/**********************************************************************
*   DDGetPreviousAutoflip
*
*   DESCRIPTION: Returns the surface that received the data from the
*       previous field of video port while autoflipping is taking palce. Only
*   called for hardware autoflipping.
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDGETAUTOFLIPINFO
*                   DWORD               dwSize
*          EDI  LPDDGETAUTOFLIPINFO
*                   DWORD               dwSize
*                   DWORD               dwSurfaceIndex
*
*   EXIT:
*          EAX  0 = success, 1 = error
*
**********************************************************************/
DWORD DDGetPreviousAutoflip(void)
{
   DDGETAUTOFLIPININFO	*lpAutoFlipIn;
   DDGETAUTOFLIPOUTINFO *lpAutoFlipOut;
   BYTE bNewBuf;
    KMVTBUFF * lpKMVTBuff;

   _asm mov lpAutoFlipIn, esi
   _asm mov lpAutoFlipOut, edi

   _asm mov lpAutoFlipIn, esi
   _asm mov lpAutoFlipOut, edi
//   Debug_Printf("KMVT - DDGetCurrentAutoflip\n");
   if (kmtvInfo.bReportForVideo)
   {
		lpKMVTBuff = (KMVTBUFF *)GETGBL_KMVTBuff((DWORD)kmtvInfo.pDev->lpDriverData);
		if (0 && lpKMVTBuff && lpKMVTBuff->bVMIPLDinUse)			// since this is on vbi - use hw
			lpAutoFlipOut->dwSurfaceIndex = lpKMVTBuff->dwLastBufferFilled;
		else
		{
		   bNewBuf = (BYTE)((((SstIORegs *)(kmtvInfo.pDev->RegBase[HWINFO_SST_IOREGS_INDEX]))->vidCurrentLine) >> 16) & 0x3;
		}
   }
   else
	   bNewBuf = 0;
   lpAutoFlipOut->dwSurfaceIndex = bNewBuf;
#ifdef DEBUG
   Debug_Printf("KMVT - DDGetPreviousAutoflip - %d\n", bNewBuf);
#endif


   return 0;
}

/**********************************************************************
*   DDTransfer
*
*   DESCRIPTION: tells the driver to bus master data from a surface to the buffer
*                specified in the memory descriptor list (MDL).  The MDL is defined
*                in the WDM documentation.   Called at HW interrupt time
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDTRANSFERININFO
*
*          EDI  LPDDTRANSFEROUTINFO
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
#define H3_VMI_DEINTERLACE_WEAVE  0x00000010

DWORD DDTransfer(void)
{
   DDTRANSFERININFO      *lpTransferIn;
   DDTRANSFEROUTINFO     *lpTransferOut;
   DDSURFACEDATA         *lpSurfaceData;
   GLOBALDATA 		 *ppdev;
   KMVTBUFF            * bltBuff = 0;
   DWORD		  dwTransferIndex=0;
   BOOL				bIsEven;


   _asm mov lpTransferIn, esi
   _asm mov lpTransferOut, edi
  
   lpSurfaceData = (DDSURFACEDATA *)lpTransferIn->lpSurfaceData;
   lpTransferOut->dwSize = sizeof (DDTRANSFEROUTINFO ); 
   //Get global information specifically number of VBI lines
   if (lpSurfaceData) {
	ppdev = (GLOBALDATA *)lpSurfaceData->dwDriverReserved1;
	bltBuff = (KMVTBUFF *)_FF(KMVTBuff);
   }
   if (!bltBuff)
     return 0;
	if (bltBuff->bVMIPLDinUse)
	{
		lpTransferOut->dwBufferPolarity = !bltBuff->bLastFieldEven;
		bIsEven = bltBuff->bLastFieldEven;
	}
	else
	{
	   if (((((SstIORegs *)(kmtvInfo.pDev->RegBase[HWINFO_SST_IOREGS_INDEX]))->vidCurrentLine) >> 18) & 0x01)
			lpTransferOut->dwBufferPolarity = FALSE;
	   else
			lpTransferOut->dwBufferPolarity = TRUE;
	   bIsEven = !lpTransferOut->dwBufferPolarity;
	}


   if ((lpTransferIn->lpDestMDL==0) || (lpTransferIn->lpDestMDL->ByteCount==0)){
     Debug_Printf("KMVT - MDL is NULL - return\n");    
     return 0;
   }

   //do asynchronous transfers
   kmtvInfo.fTransferFull = AddTransfer (lpTransferIn, &dwTransferIndex, bIsEven);

   if (!kmtvInfo.fTransferFull && (dwTransferIndex < MAX_TRANSFERS)) 
   {
#ifdef USE_EVENT_TRANSFER
#ifdef USE_VMM_CALLBACK
	 kmtvInfo.pKMVTbuff = bltBuff;
#ifdef USE_VMM_CALLBACK_IRQ
     kmtvInfo.Transfer[dwTransferIndex].EventHandle = Call_When_VM_Ints_Enabled( TransferVMEvent, (ULONG)dwTransferIndex);
#else
     kmtvInfo.Transfer[dwTransferIndex].EventHandle = Call_When_VM_Returns( TransferVMEvent, (ULONG)dwTransferIndex);
#endif
#else
	 kmtvInfo.Transfer[dwTransferIndex].EventHandle = Schedule_Global_Event( TransferVMEvent, (ULONG)dwTransferIndex);
#endif
#else
	 TransferVMEvent(dwTransferIndex);
#endif

     return 0;
   } 

#ifdef DEBUG
    Debug_Printf (" ******* Transfer FULL ******** \n");
#endif
	return 0;
 
}

/**********************************************************************
*   DDGetTransferStatus
*
*   DESCRIPTION: Returns the transfer id in the DDGETTRANSFERSTATUSOUTINFO 
*                structure.  This function is used to determined which 
*                hardware bus master has completed.
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDGETTRANSFERSTATUSOUTINFO
*
*          EDI  0
*
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
DWORD DDGetTransferStatus(void)
{
   DDGETTRANSFERSTATUSOUTINFO	*lpTransferStatus;
   DWORD i=0;
   BOOL  fFound=FALSE;

   _asm mov lpTransferStatus, edi
// Debug_Printf("KMVT - DDGetTransferStatus - QueuedTransfers=%d Busmasterready = %d\n",kmtvInfo.dwQueuedTransfers, kmtvInfo.dwBusMasterTransfers);
   for (i=kmtvInfo.dwCurrentTransfer;i<MAX_TRANSFERS && !fFound; i++) {
	if (kmtvInfo.Transfer[i].dwFlags == TRANSFER_COMPLETE) {
		lpTransferStatus->dwTransferID = kmtvInfo.Transfer[kmtvInfo.dwCurrentTransfer].dwTransferID;
#ifdef USE_VMM_CALLBACK_IRQ
		if (kmtvInfo.dwTransferID)
			kmtvInfo.dwTransferID--;  
#else
#ifndef USE_EVENT_TRANSFER
		if (kmtvInfo.dwTransferID)
			kmtvInfo.dwTransferID--;  
#endif
#endif
//       Debug_Printf("KMVT - DDGetTransferStatus - Index=%d \n",kmtvInfo.dwCurrentTransfer);
		kmtvInfo.Transfer[i].dwFlags = TRANSFER_READY;
		kmtvInfo.dwBusMasterTransfers--;
		kmtvInfo.dwCurrentTransfer++;
		if (kmtvInfo.dwCurrentTransfer == MAX_TRANSFERS)
			kmtvInfo.dwCurrentTransfer = 0;
	}
	fFound = TRUE;
   }
   for (i=0; i<kmtvInfo.dwCurrentTransfer && !fFound; i++) {
	if (kmtvInfo.Transfer[i].dwFlags == TRANSFER_COMPLETE) {
		lpTransferStatus->dwTransferID = kmtvInfo.Transfer[kmtvInfo.dwCurrentTransfer].dwTransferID;
#ifdef USE_VMM_CALLBACK_IRQ
		if (kmtvInfo.dwTransferID)
			kmtvInfo.dwTransferID--;  
#else
#ifndef USE_EVENT_TRANSFER
		if (kmtvInfo.dwTransferID)
			kmtvInfo.dwTransferID--;  
#endif
#endif
//             Debug_Printf("KMVT - DDGetTransferStatus - Index=%d \n",kmtvInfo.dwCurrentTransfer);
		kmtvInfo.Transfer[i].dwFlags = TRANSFER_READY;
		kmtvInfo.dwBusMasterTransfers--;
		kmtvInfo.dwCurrentTransfer++;
		if (kmtvInfo.dwCurrentTransfer == MAX_TRANSFERS)
			kmtvInfo.dwCurrentTransfer = 0;
	}
	fFound = TRUE;
   }
   if (fFound) {
	  lpTransferStatus->dwSize = sizeof (DDGETTRANSFERSTATUSOUTINFO);
	  return 0;
   }

   
   if ((kmtvInfo.dwBusMasterTransfers > 0) && kmtvInfo.Transfer[kmtvInfo.dwCurrentTransfer].dwFlags == TRANSFER_COMPLETE) {
		lpTransferStatus->dwTransferID = kmtvInfo.Transfer[kmtvInfo.dwCurrentTransfer].dwTransferID;
#ifdef USE_VMM_CALLBACK_IRQ
		if (kmtvInfo.dwTransferID)
			kmtvInfo.dwTransferID--;  
#else
#ifndef USE_EVENT_TRANSFER
		if (kmtvInfo.dwTransferID)
			kmtvInfo.dwTransferID--;  
#endif
#endif
//        Debug_Printf("KMVT - DDGetTransferStatus - Index=%d \n",kmtvInfo.dwCurrentTransfer);

		kmtvInfo.Transfer[kmtvInfo.dwCurrentTransfer].dwFlags = TRANSFER_READY;
		kmtvInfo.dwBusMasterTransfers--;
		kmtvInfo.dwCurrentTransfer++;
		if (kmtvInfo.dwCurrentTransfer == MAX_TRANSFERS)
			kmtvInfo.dwCurrentTransfer = 0;
#ifdef USE_EVENT_TRANSFER
   } else if (kmtvInfo.dwTransferID) {
		lpTransferStatus->dwTransferID = kmtvInfo.dwTransferID;
   } else if (kmtvInfo.dwVBITransferID) {
			lpTransferStatus->dwTransferID = kmtvInfo.dwVBITransferID;
#endif
   }
   lpTransferStatus->dwSize = sizeof (DDGETTRANSFERSTATUSOUTINFO);
   return 0;
}



BYTE AddTransfer(DDTRANSFERININFO      *lpTransferIn, DWORD *dwCurrentIndex, DWORD polarity)
{
   BYTE  fSlotsFull = 1;
   BOOL  bFirstCheck = TRUE;
   DWORD i;
   *dwCurrentIndex = 0;
//   for (i = kmtvInfo.dwCurrentTransfer, bFirstCheck = TRUE; 
//		fSlotsFull && (bFirstCheck || i != kmtvInfo.dwCurrentTransfer)
//			i = ((i < MAX_TRANSFERS - 1) ? i + 1 : 0))

   for (i = kmtvInfo.dwCurrentTransfer, bFirstCheck = TRUE; 
		i<MAX_TRANSFERS; i++)
   {
	if (kmtvInfo.Transfer[i].dwFlags == TRANSFER_READY)
	{
  	   kmtvInfo.dwQueuedTransfers ++;
//  Debug_Printf (" AddTransfer1 = %d Queued=%d\n", i,kmtvInfo.dwQueuedTransfers);

	   *dwCurrentIndex = i;
	   kmtvInfo.Transfer[i].lpSurfaceData = (DDSURFACEDATA *)lpTransferIn->lpSurfaceData;
	   kmtvInfo.Transfer[i].dwTransferID = lpTransferIn->dwTransferID;
	   kmtvInfo.Transfer[i].pMDL = lpTransferIn->lpDestMDL;
	   kmtvInfo.Transfer[i].dwTransferFlags = lpTransferIn->dwTransferFlags;
	   kmtvInfo.Transfer[i].dwStartLine = lpTransferIn->dwStartLine;
	   kmtvInfo.Transfer[i].dwEndLine = lpTransferIn->dwEndLine;
	   kmtvInfo.Transfer[i].dwFlags = TRANSFER_QUEUED;
	   kmtvInfo.Transfer[i].bPolarity = polarity;
       fSlotsFull = 0;
	   break;
     }
   }
   for (i=0; (i<kmtvInfo.dwCurrentTransfer) && fSlotsFull; i++)
   {
   	if (kmtvInfo.Transfer[i].dwFlags == TRANSFER_READY)
	{
	 	kmtvInfo.dwQueuedTransfers ++;
// Debug_Printf (" AddTransfer2 = %d Queued=%d\n", i,kmtvInfo.dwQueuedTransfers);

	   *dwCurrentIndex = i;
	   kmtvInfo.Transfer[i].lpSurfaceData = (DDSURFACEDATA *)lpTransferIn->lpSurfaceData;
	   kmtvInfo.Transfer[i].dwTransferID = lpTransferIn->dwTransferID;
	   kmtvInfo.Transfer[i].pMDL = lpTransferIn->lpDestMDL;
	   kmtvInfo.Transfer[i].dwTransferFlags = lpTransferIn->dwTransferFlags;
	   kmtvInfo.Transfer[i].dwStartLine = lpTransferIn->dwStartLine;
	   kmtvInfo.Transfer[i].dwEndLine = lpTransferIn->dwEndLine;
	   kmtvInfo.Transfer[i].dwFlags = TRANSFER_QUEUED;
	   kmtvInfo.Transfer[i].bPolarity = polarity;
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
// a cancel is occurring and causing a BSOD on a null pointer
#ifdef USE_EVENT_TRANSFER
VOID TransferVMEvent( VOID )
#else
VOID TransferVMEvent( DWORD refData )
#endif
{

   DWORD	RefData;
   DDSURFACEDATA         *lpSurfaceData;
   PMDL                   pMDL;
   BYTE                  *lpBufferStart, *lpBufferDest;
   DWORD 		  dwNumLines;
   GLOBALDATA 		 *ppdev;
   KMVTBUFF            * bltBuff;
   BYTE			  bUseAllLines = 0;
   DWORD                  dwMaxTransferCnt;
   DWORD		  lineWidth, dwTransferFlags;
   DWORD			linecnt;
   DWORD				lineOffset;
	BYTE		*lpSrcLine;
	BOOL		bInterleaved = FALSE;
	DWORD		polarity;


#ifdef USE_EVENT_TRANSFER
   _asm mov RefData, edx
#else
	   RefData = refData;
#endif

   kmtvInfo.Transfer[RefData].dwFlags = TRANSFER_IN_PROGRESS;	// do this first to avoid deadlock
   if (RefData >= MAX_TRANSFERS || !kmtvInfo.bInitialized)
	return;

   kmtvInfo.dwQueuedTransfers --;
   polarity = kmtvInfo.Transfer[RefData].bPolarity;

   
   //Video Capture
   bUseAllLines = 1;

   //Get global information specifically number of VBI lines
   lpSurfaceData = kmtvInfo.Transfer[RefData].lpSurfaceData; 
   if (!lpSurfaceData || !kmtvInfo.bInitialized)
	   return;
   pMDL = kmtvInfo.Transfer[RefData].pMDL;
   dwNumLines =    kmtvInfo.Transfer[RefData].dwEndLine - kmtvInfo.Transfer[RefData].dwStartLine + 1;
   lineWidth = lpSurfaceData->dwWidth * (lpSurfaceData->dwFormatBitCount / 8);
   dwTransferFlags = kmtvInfo.Transfer[RefData].dwTransferFlags;

   //Get global information specifically number of VBI lines
   ppdev = (GLOBALDATA *)lpSurfaceData->dwDriverReserved1;
   if (!ppdev || !kmtvInfo.bInitialized)
	   return;
#ifdef USE_VMM_CALLBACK
	bltBuff = kmtvInfo.pKMVTbuff;
#else
   bltBuff = (KMVTBUFF *)_FF(KMVTBuff);
#endif
   if (!bltBuff || !kmtvInfo.bInitialized)
	   return;
   lineOffset = lpSurfaceData->lPitch;						// pitch on surface

	bInterleaved = ((bltBuff->dwVidInFormat & H3_VMI_DEINTERLACE_WEAVE)
			|| (bltBuff->dwStatus & INTERLEAVE_ON));
   ///////////////////////////////////
   //VBI Capture 
   ///////////////////////////////////
   if (lpSurfaceData->dwFormatFourCC == FOURCC_RAW8) 
   {
      BYTE bBufNum;

		if (0 && bltBuff->bVMIPLDinUse)			// since this is on vbi - use hw
			bBufNum = (BYTE)bltBuff->dwLastBufferFilled;
		else
		   bBufNum = (BYTE)((*(DWORD*)(kmtvInfo.pDev->RegBase[HWINFO_SST_IOREGS_INDEX] + H4_VID_IN_STATUS_CURLINE)) >> 16) & 0x3;

	  lpBufferStart = (BYTE *)bltBuff->fpVidMem[bBufNum];
	  if (!lpBufferStart || !kmtvInfo.bInitialized)
		  return;

	  if (bInterleaved)	// see if half or full
	  {
			if (dwNumLines < (bltBuff->dwVBILines-2))			// not a match 
				bUseAllLines = 0;
	  }
	  lineOffset = bltBuff->dwVideoSurfacePitch;

	  if (!bUseAllLines) {
	    lineOffset <<= 1;		// double if every other line
	  }

      if ((bInterleaved) && polarity)		// must skip first line
		lpBufferStart += bltBuff->dwVideoSurfacePitch;
 
	lpSrcLine = lpBufferStart;
	if (dwTransferFlags & DDTRANSFER_INVERT)
	{
		lineOffset = 0 - (lineOffset);
		lpSrcLine = lpBufferStart + ((dwNumLines - 1) * bltBuff->dwVideoSurfacePitch);
	}
	//in the case of vddtransfer we need to copy the data as specified 
	//in the linked list of MDL

	//Micronas chip only generates 1140 valid bytes per line so only 
	//copy that much.
	lineWidth = 1140;
	for (linecnt = 0; linecnt < dwNumLines && pMDL;)
	{
		BYTE *lpDestLine;
		DWORD bytes;
		DWORD bytesPerLine = lineWidth;

		//Now get the pointer to the memory destination
		lpBufferDest = (BYTE *)(pMDL->lpMappedSystemVa);
		if (!lpBufferDest || !kmtvInfo.bInitialized)
			return;
		lpDestLine = lpBufferDest;
		dwMaxTransferCnt = pMDL->ByteCount; 

		// must copy line at a time to not affect graphic performance (kephart suggestion)
		for (bytes = 0;  kmtvInfo.bInitialized &&
			linecnt < dwNumLines && bytes < dwMaxTransferCnt;)		// should this be <= for numlines
		{

			memcpy (lpDestLine, lpSrcLine, bytesPerLine);	// use width in case pitch diff - tiled space
			linecnt++;
  			lpDestLine += lineWidth;
			lpSrcLine += lineOffset;
  			bytes += lineWidth;		// must reflect dest
		}
		pMDL = pMDL->MdlNext;
	}  
   }
   else 
   {
     ///////////////////////////////////
     // Video Capture
     ///////////////////////////////////
	    DWORD dwVidLines = bltBuff->dwVideoLines;
		BYTE bBufNum;
		lpBufferStart = (BYTE *)(lpSurfaceData->fpLockPtr);
		if (!lpBufferStart || !kmtvInfo.bInitialized)
			return;
		// if the fake interleave is active use this code for buffer start
		if (GETGBL_dwOvlOffset((DWORD)kmtvInfo.pDev->lpDriverData) 
		   && (kmtvInfo.pDev->dwIMask & H3_VSYNC_INT_ENABLE) 
		   &&  (bltBuff->dwStatus & INTERLEAVE_ON ))
		{	// manual flipping
			bBufNum = (BYTE) kmtvInfo.dwFirstFieldBuffer;
			lpBufferStart = (BYTE *)bltBuff->fpVidMem[bBufNum];
			if (!lpBufferStart || !kmtvInfo.bInitialized)
				return;
			polarity = FALSE;		// just capture first field - ok for v3
		}

 	    kmtvInfo.bReportForVideo = TRUE;		// doing video

		if (kmtvInfo.dwActive == 1)		// is eav
			dwVidLines++;

//		VDDtoWDMTellCapture(kmtvInfo.dwFieldCount, bIsEven, FALSE); 

		if (!bltBuff->bVBIcropped)
			lpBufferStart += (bltBuff->dwVBILines*lpSurfaceData->lPitch);

		lineOffset = lpSurfaceData->lPitch;						// pitch on surface
		if (dwNumLines > dwVidLines)		// should be double
			bUseAllLines = 1;
		else 
			bUseAllLines = 0;
		if (bInterleaved && !bUseAllLines)	// see if half or full
		{
			if (polarity)		// must skip first line
				lpBufferStart += bltBuff->dwVideoSurfacePitch;
		}
		if (bltBuff->bDoublePitch && !bUseAllLines)
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
			lpBufferDest = (BYTE *)(pMDL->lpMappedSystemVa);
			if (!lpBufferDest || !kmtvInfo.bInitialized)
				return;
			lpDestLine = lpBufferDest;
			dwMaxTransferCnt = pMDL->ByteCount; 

			// must copy line at a time to not affect graphic performance (kephart suggestion)
			for (bytes = 0; kmtvInfo.bInitialized &&
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
			pMDL = pMDL->MdlNext;
		}  
   }

   kmtvInfo.Transfer[RefData].dwFlags = TRANSFER_COMPLETE;
   kmtvInfo.dwBusMasterTransfers ++;

//Debug_Printf (" Transfer BusMaster Complete=%d Outstanding=%d\n", RefData,kmtvInfo.dwBusMasterTransfers); 

#ifdef USE_EVENT_TRANSFER
#ifdef USE_VMM_CALLBACK_IRQ
	kmtvInfo.dwTransferID++;
#else
   //Now notify Windows that transfer is complete
   if (kmtvInfo.IRQCallback)
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
#endif
#else
	kmtvInfo.dwTransferID++;
#endif
   return;
	
}



//THis will query to see if a WDM driver VXD is installed
//so that we don't GP Fault when trying to talk to it.
DWORD QueryActiveWDMVXD(void)
{
    DWORD fWDMVXDInstalled= 0;
	struct VxD_Desc_Block *pWDMDDB=0;
	CHAR Name = 0;
    pWDMDDB = Get_DDB(V3TVMAP_DEVICE_ID, &Name );
	if (pWDMDDB != 0)
		fWDMVXDInstalled = 1;
	return fWDMVXDInstalled;
}

DWORD __stdcall VDDtoWDMQueryActive(DIOCPARAMETERS * lpParams)
{   
   DWORD *outBuff = (DWORD *)lpParams->lpvOutBuffer;
   lpParams->lpcbBytesReturned = 0;
   kmtvInfo.dwActive = QueryActiveWDMVXD();
   //Don't bother querying WDM if it isn't installed
   if (kmtvInfo.dwActive == 1) {
	   //Now that we know WDM is installed, now
	   //check if any streams are open for the case
	   //of a DVD opening videoport versus WDM
		_asm {
			push eax
			mov eax, 0
  			VxDCall (V3TVMAP_QueryActive);
			mov kmtvInfo.dwActive, eax
	 	    pop eax
		}
   } 
// else 
//	   Debug_Printf (" ****No WDM VXD Installed***\n");
   
   outBuff[0] = kmtvInfo.dwActive;
   return 0;
}

DWORD VDDtoWDMTellCapture(DWORD frame, DWORD polarity, BOOL vbiBuffer)
{
	DWORD   dwResult;
	DWORD	dwPolarity_BufType = (polarity | (vbiBuffer << 16));

	if (!kmtvInfo.dwActive)
	   return 0;
	_asm {
	   mov eax, frame
	   push ecx
	   mov ecx, dwPolarity_BufType
	   VxDCall (V3TVMAP_NotifyCapture);
	   pop ecx
	   mov dwResult, eax
	}
	return dwResult;
}


DWORD __stdcall VDDtoWDMScale(DIOCPARAMETERS * lpParams)
{   
   DWORD *inBuff =  (DWORD *)lpParams->lpvInBuffer;
   DWORD *outBuff = (DWORD *)lpParams->lpvOutBuffer;
   DWORD lpSize = inBuff[0];
   DWORD lpStart = inBuff[1];
   DWORD   dwResult;
   lpParams->lpcbBytesReturned = 0;
   outBuff[0] = 0;
   if (!kmtvInfo.dwActive)
	   return 0;
   _asm {
	   mov eax, lpSize
	   push ecx
	   mov ecx, lpStart
  	   VxDCall (V3TVMAP_NotifyMap);
	   pop ecx
	   mov dwResult, eax
   }
   outBuff[0] = dwResult;
   return 0;
}


#pragma optimize( "", on )

#undef IS_32
#undef MM
#define THUNK32

  
