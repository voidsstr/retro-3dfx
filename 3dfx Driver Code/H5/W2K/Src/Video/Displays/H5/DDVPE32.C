/* $Header: ddvpe32.c, 10, 10/11/00 8:44:06 PM, Brent$ */
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
** File Name:	DDVPE32.C
**
** Description: Direct Draw Video Port Extentions and
**              supporting functions.
**
** $Revision: 10$
** $Date: 10/11/00 8:44:06 PM$
**
** $History: ddvpe32.c $
** 
** *****************  Version 55  *****************
** User: Xingc        Date: 9/07/99    Time: 11:58a
** Updated in $/devel/h5/W2K/Src/Video/Displays/h5
** VMI and PLD support for W2K
** 
** *****************  Version 54  *****************
** User: Xingc        Date: 9/03/99    Time: 11:19a
** Updated in $/devel/h5/W2K/Src/Video/Displays/h5
** Dont' inlcude vmipld.h for W2k now
** 
** *****************  Version 53  *****************
** User: Xingc        Date: 9/03/99    Time: 10:57a
** Updated in $/devel/h5/Win9x/dx/dd32
** Use new VMIPLD functions
** 
** *****************  Version 52  *****************
** User: Russ         Date: 8/25/99    Time: 3:16p
** Updated in $/devel/h5/W2K/Src/Video/Displays/h5
** define VBIHeight for w2k
** whack a couple calls to UpdateIMask for w2k
** fix WINT typo
**
** *****************  Version 51  *****************
** User: Lpost        Date: 8/25/99    Time: 2:07p
** Updated in $/devel/h5/Win9x/dx/dd32
** V3TV code merge into H5
**
** *****************  Version 50  *****************
** User: Xingc        Date: 8/10/99    Time: 5:34p
** Updated in $/devel/h5/Win9x/dx/dd32
** ccir656 settings and fifo flush
**
** *****************  Version 49  *****************
** User: Xingc        Date: 7/28/99    Time: 2:57p
** Updated in $/devel/h5/W2K/Src/Video/Displays/h5
** Set right number of port connection for W2K
**
** *****************  Version 48  *****************
** User: Xingc        Date: 7/26/99    Time: 1:44p
** Updated in $/devel/h5/Win9x/dx/dd32
** Clear #ifdef H4 and #ifdef H5
**
** *****************  Version 47  *****************
** User: Xingc        Date: 7/26/99    Time: 12:49p
** Updated in $/devel/h5/Win9x/dx/dd32
** Add support  CCIR656 port type
**
** *****************  Version 46  *****************
** User: Xingc        Date: 7/19/99    Time: 1:41p
** Updated in $/devel/h5/W2K/Src/Video/Displays/h5
** Enables VPE and KMVT support for W2K
**
** *****************  Version 45  *****************
** User: Cwilcox      Date: 7/15/99    Time: 4:23p
** Updated in $/devel/h5/Win9x/dx/dd32
** Add runtime checks for Napalm.
**
** *****************  Version 44  *****************
** User: Xingc        Date: 7/14/99    Time: 3:06p
** Updated in $/devel/h5/Win9x/dx/dd32
** Include the change fro W2K driver
**
** *****************  Version 43  *****************
** User: Xingc        Date: 7/14/99    Time: 2:51p
** Updated in $/devel/h5/Win9x/dx/dd32
** Add H3Bandwidth and gsVportFlip define
**
** *****************  Version 42  *****************
** User: Xingc        Date: 7/13/99    Time: 6:10p
** Updated in $/devel/h5/W2K/Src/Video/Displays/h5
** Move VidInData, VidSrcData, VidDstData rOvlDst from GLOBALDATA into
** DDGLOBAL for NT driver
**
** *****************  Version 41  *****************
** User: Xingc        Date: 6/30/99    Time: 4:55p
** Updated in $/devel/h5/Win9x/dx/dd32
** Create and use not more than two overlay shrink surfaces
**
** *****************  Version 40  *****************
** User: Edwin        Date: 6/29/99    Time: 3:56p
** Updated in $/devel/h5/Win9x/dx/dd32
** Remove obsolete Banshee ifdefs.
**
** *****************  Version 39  *****************
** User: Xingc        Date: 6/24/99    Time: 1:37p
** Updated in $/devel/h5/Win9x/dx/dd32
** Fix DCt200 test problem for FlipVideoPort
**
** *****************  Version 38  *****************
** User: Edwin        Date: 6/10/99    Time: 1:10p
** Updated in $/devel/h5/Win9x/dx/dd32
** Remove function prototypes, it is now declared in fnproto.h.
**
** *****************  Version 36  *****************
** User: Edwin        Date: 6/01/99    Time: 1:49p
** Updated in $/devel/h3/Win95/dx/dd32
** Remove ifdef MM, multi-monitor support is always enabled.
**
** *****************  Version 35  *****************
** User: Lpost        Date: 5/19/99    Time: 6:46p
** Updated in $/devel/h3/Win95/dx/dd32
** V3TV fixes for capture and scaling
**
** *****************  Version 34  *****************
** User: Stb_lpost    Date: 5/17/99    Time: 1:42p
** Updated in $/devel/h3/win95/dx/dd32
** V3TV Video Capture fixes for E3 Demo
**
** *****************  Version 33  *****************
** User: Xingc        Date: 5/10/99    Time: 6:48p
** Updated in $/devel/h3/Win95/dx/dd32
** Fix MS DVD Player problems
** 1. Missing bottom lines when zoom up
** 2. Color formate miss match when scale down after pause
** 3. Scale factor error after zoom up from zero
**
** *****************  Version 32  *****************
** User: Stb_lpost    Date: 5/03/99    Time: 2:55p
** Updated in $/devel/h3/win95/dx/dd32
** V3TV VBI fixes for VBI capture
**
** *****************  Version 31  *****************
** User: Stb_lpost    Date: 4/26/99    Time: 7:58a
** Updated in $/devel/h3/win95/dx/dd32
** V3TV - VBI Capture and Connection fixes
**
** *****************  Version 30  *****************
** User: Stb_lpost    Date: 4/13/99    Time: 3:10p
** Updated in $/devel/h3/win95/dx/dd32
** VBI filter connect fixes for Voodoo3 TV -ifdefed only with V3TV
**
** *****************  Version 29  *****************
** User: Xingc        Date: 4/09/99    Time: 3:31p
** Updated in $/devel/h3/Win95/dx/dd32
** For interleaved bob mode set double buffering
**
** *****************  Version 28  *****************
** User: Stb_bseitsin Date: 4/09/99    Time: 12:38p
** Updated in $/devel/h3/win95/dx/dd32
** Added Napalm registers. Added ifdef H5.
**
** *****************  Version 27  *****************
** User: Xingc        Date: 4/08/99    Time: 5:34p
** Updated in $/devel/h3/Win95/dx/dd32
** ADD BOB_INTERLEAVE support and fix video port scale factors miss
** calculation problem.
**
** *****************  Version 26  *****************
** User: Xingc        Date: 3/26/99    Time: 10:26a
** Updated in $/devel/h3/Win95/dx/dd32
** For Interleved BOB mode, switch Interleaved video into non-interleaved
** video, so that the video can be adjusted for both fields.
**
** *****************  Version 25  *****************
** User: Agus         Date: 3/25/99    Time: 3:35p
** Updated in $/devel/h3/Win95/dx/dd32
** Fix PRS#5117: Fixed HW DVD black screen when on 1280x1024 or 1600x1200.
**
** *****************  Version 24  *****************
** User: Xingc        Date: 3/22/99    Time: 5:49p
** Updated in $/devel/h3/Win95/dx/dd32
** Call minivdd IOControl to create DEBENGIN linear Address
**
** *****************  Version 23  *****************
** User: Xingc        Date: 3/08/99    Time: 3:37p
** Updated in $/devel/h3/Win95/dx/dd32
** Pass overlay pitch into KMVT
**
** *****************  Version 22  *****************
** User: Xingc        Date: 3/04/99    Time: 6:31p
** Updated in $/devel/h3/Win95/dx/dd32
** Fix shrink factor problem when destination is smaller than the source
**
** *****************  Version 21  *****************
** User: Xingc        Date: 3/02/99    Time: 1:46p
** Updated in $/devel/h3/Win95/dx/dd32
** Use dwVidSrcWidth and dwVidSrcHeight in VidInData to calculate video
** port scale factor.
**
** *****************  Version 20  *****************
** User: Xingc        Date: 2/16/99    Time: 5:18p
** Updated in $/devel/h3/Win95/dx/dd32
** Support shrink overlay in SyncSurfaceData()
**
** *****************  Version 19  *****************
** User: Agus         Date: 2/03/99    Time: 5:24p
** Updated in $/devel/h3/Win95/dx/dd32
** Fixed VMI/TV-XLCD bits and mask for V3
**
** *****************  Version 18  *****************
** User: Stb_srogers  Date: 1/29/99    Time: 7:54a
** Updated in $/devel/h3/win95/dx/dd32
**
**
** *****************  Version 17  *****************
** User: Agus         Date: 1/15/99    Time: 4:10p
** Updated in $/devel/h3/Win95/dx/dd32
** Added #ifdef for Avenger [H4]
**
** *****************  Version 16  *****************
** User: Michael      Date: 12/31/98   Time: 7:34a
** Updated in $/devel/h3/Win95/dx/dd32
** Implement the 3Dfx/STB unified header.
**
*/

#include "precomp.h"
#include "ddkernel.h" // for guid
#include "fxglobal.h"

#include "ddvpe32.h"
//#ifndef WINNT
#include "tv.h"
//#endif
#define DEBUG_VPEENTRY 0

#define PC98_DNSCL_FILT	0		//PC-98 downscale bilinear filter effect

#define H3BandWidth _DD(H3BandWidth)
#define gsVportFlip _DD(gsVportFlip)

void H3_VMI_Disable(NT9XDEVICEDATA * ppdev);
void H3_VMI_Enable_Func(NT9XDEVICEDATA * ppdev);
void H3_VMI_Reset(NT9XDEVICEDATA * ppdev);
void H3_VMI_SetDecimation(NT9XDEVICEDATA * ppdev,RECT *, RECT *);
void H3_InitBandwidth(NT9XDEVICEDATA * ppdev);       //Initialize bandwidth data
void H3_InitializeAll(NT9XDEVICEDATA * ppdev);       //Initialize all other
//BOOL H3_Calculate_Bandwidth(NT9XDEVICEDATA * ppdev, RECT *, RECT * );

#define VBIHeight   _DD(dwVBIHeight)

DWORD __stdcall vpGetVideoPortFlipstatus32(LPDDHAL_GETVPORTFLIPSTATUSDATA pgvpfsd);

#include "vmipld.h"

#define FXTRACE 1
/*----------------------------------------------------------------------
Function name: IOControl

Description:  Call into minivdd
----------------------------------------------------------------------*/
DWORD  IOControl( NT9XDEVICEDATA * ppdev, DWORD dwCommand, DWORD dwParm1, DWORD dwParm2 )
{
  DWORD inBuff[3];
  DWORD outBuff[1];
  DWORD dwRetBytes;
#ifndef WINNT
  HANDLE hDevice;
#endif
      outBuff[0] = 0;
      inBuff[0] = dwParm1;
      inBuff[1] = dwParm2;

#ifdef WINNT
    if (EngDeviceIoControl(ppdev->hDriver,
                               dwCommand,
                               (LPVOID)&inBuff,
                               8,
                               (LPVOID)&outBuff,
                               4,
                               &dwRetBytes))
     {
            outBuff[0] = 0;
      }

#else
     hDevice = CreateFile("\\\\.\\H4VDD", 0, 0, NULL, 0, 0, NULL);

     if (INVALID_HANDLE_VALUE == hDevice)
     {
       return 0;
     }

     inBuff[2] = (DWORD)ppdev;

     if(!DeviceIoControl( hDevice, dwCommand,
             &inBuff, 12, &outBuff, 4, &dwRetBytes, NULL))
     {
            outBuff[0] = 0;
     }

     CloseHandle(hDevice);

#endif
      return outBuff[0];
}

//#ifndef WINNT
/*----------------------------------------------------------------------
Function name:  UpdateIMask

Description:    Call miniVDD to update IMASK.

Return:         int
----------------------------------------------------------------------*/

int UpdateIMask(NT9XDEVICEDATA * ppdev, DWORD dwIMask)
{
#ifdef WINNT
	KMVTBUFF * lpKMBuff = (KMVTBUFF *)_FF( KMVTBuff);
	if (lpKMBuff)
		lpKMBuff->dwIMask = dwIMask;
#else
    IOControl( ppdev, UPDATE_IMASK, _FF(DevNode), dwIMask);
#endif
    return 0;

} // UpdateIMask

//#endif
/*----------------------------------------------------------------------
Function name: vpCanCreateVideoPort32

Description:   DDRAW video port callback CanCreateVideoPort

               Verify if a video port specified by pccvpd can be
			   created.
			
			   If vccvpd does not conform to size constraints,
			   port id's, connection types or connection flags then
			   set pccvpd->ddRVal to DDERR_UNSUPPORTED.

               Otherwise, save video port create info and return
               DDHAL_DRIVER_HANDLED

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall vpCanCreateVideoPort32(LPDDHAL_CANCREATEVPORTDATA pccvpd)
{
	RECT rVidIn;
	DDVIDEOPORTDESC vpDesc;
   DD_ENTRY_SETUP(pccvpd->lpDD->lpGbl);

#ifdef FXTRACE
#ifndef WINNT 	
    Msg(ppdev, DEBUG_VPEENTRY, "CanCreate VideoPort32" );
#else
    DISPDBG(( DEBUG_VPEENTRY, "CanCreate VideoPort32" ));
#endif

	//Dump_VIDOPORTDESC( DEBUG_DDGORY, pccvpd->lpDDVideoPortDesc );
#endif
	vpDesc = *pccvpd->lpDDVideoPortDesc;

	rVidIn.top    = rVidIn.left = 0;
	rVidIn.right  = vpDesc.dwFieldWidth;
	rVidIn.bottom = vpDesc.dwFieldHeight;

	if ( (rVidIn.right > H3_MAX_VID_IN_X) || (rVidIn.bottom > H3_MAX_VID_IN_Y) ||	//Video-in size g.t. max allowed
	 	 //(vpDesc.dwMicrosecondsPerField > H3_MAX_TIME_PER_FIELD)					//?
		 (vpDesc.dwVideoPortID != H3_PORT_ID) ||									//Mismatch port ID
		 (vpDesc.VideoPortType.dwPortWidth > H3_MAX_VID_IN_X) ||					//Port specified g.t. max allowed
		 (!memcmp(&vpDesc.VideoPortType.guidTypeID, &DDVPTYPE_BROOKTREE, sizeof(GUID))  ||	//Unsupported connect types
         (
           (!_DD(bVMIPLDpresent) &&
  		  !memcmp(&vpDesc.VideoPortType.guidTypeID, &DDVPTYPE_CCIR656, sizeof(GUID))))   ||
		  !memcmp(&vpDesc.VideoPortType.guidTypeID, &DDVPTYPE_PHILIPS, sizeof(GUID)))   ||
		 ((vpDesc.VideoPortType.dwFlags & DDVPCONNECT_DOUBLECLOCK) ||				//Unsupported misc
		 /* The following flag is not required, we actually support this so why should we reject any Video Ports which
		    are created which has this set? - JHunter 
		  (vpDesc.VideoPortType.dwFlags & DDVPCONNECT_HALFLINE) ||					*/
		  (vpDesc.VideoPortType.dwFlags & DDVPCONNECT_SHAREEVEN) ||
		  (vpDesc.VideoPortType.dwFlags & DDVPCONNECT_SHAREODD)) )
	{
		pccvpd->ddRVal = DDERR_UNSUPPORTED;
	}	
	else
	{
		//Save the VideoPort created info in Video In datastruct
		VidInData.rVidIn = rVidIn;                                      //Frame size of video in stream
		VidInData.gConnectTypeID = vpDesc.VideoPortType.guidTypeID;     //Connection info
		VidInData.dwConnectFlags = vpDesc.VideoPortType.dwFlags;        //Connect type
		pccvpd->ddRVal = DD_OK;
	}
	return DDHAL_DRIVER_HANDLED;
}// vpCanCreateVideoPort32


/*----------------------------------------------------------------------
Function name: vpCreateVideoPort32

Description:   DDRAW video port callback CreateVideoPort

               Create a video port specified by pcvpd.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall vpCreateVideoPort32(LPDDHAL_CREATEVPORTDATA pcvpd)
{
	DDVIDEOPORTDESC vpDesc;
    KMVTBUFF * lpKMBuff;
    DD_ENTRY_SETUP(pcvpd->lpDD->lpGbl);

#ifdef FXTRACE
#ifndef WINNT
        Msg(ppdev, DEBUG_VPEENTRY, "Create VideoPort32" );
#else
        DISPDBG(( DEBUG_VPEENTRY, "Create VideoPort32" ));
#endif

  	//Dump_VIDOPORTDESC( DEBUG_DDGORY, pcvpd->lpDDVideoPortDesc );
#endif
	
	vpDesc = *pcvpd->lpDDVideoPortDesc;

//#ifndef WINNT
	_DD(dwAdjustForEav) = 1;	// normal 656
	if (vpDesc.VideoPortType.dwFlags & 0x80000000)		// vfw flag for us
	{
		_DD(dwAdjustForEav) = 0;	// change on sav
		vpDesc.VideoPortType.dwFlags &= ~0x80000000;
	}
//#endif
	
	//Compare and correct any mismatched Video-in variables specified
	if ( (VidInData.rVidIn.right != (int)vpDesc.dwFieldWidth) && (vpDesc.dwFieldWidth <= H3_MAX_VID_IN_X) )
		VidInData.rVidIn.right = vpDesc.dwFieldWidth;

	if ( (VidInData.rVidIn.bottom != (int)vpDesc.dwFieldHeight) && (vpDesc.dwFieldHeight <= H3_MAX_VID_IN_Y) )
		VidInData.rVidIn.bottom = vpDesc.dwFieldHeight;

	if ( memcmp(&VidInData.gConnectTypeID, &vpDesc.VideoPortType.guidTypeID, sizeof(GUID)) &&
		  (memcmp(&vpDesc.VideoPortType.guidTypeID, &DDVPTYPE_BROOKTREE, sizeof(GUID)) ||
          (
           (!_DD(bVMIPLDpresent)&&
  		   memcmp(&vpDesc.VideoPortType.guidTypeID, &DDVPTYPE_CCIR656, sizeof(GUID)))) ||
		   memcmp(&vpDesc.VideoPortType.guidTypeID, &DDVPTYPE_PHILIPS, sizeof(GUID))) )
		VidInData.gConnectTypeID = vpDesc.VideoPortType.guidTypeID;
		
	if ( (VidInData.dwConnectFlags != vpDesc.VideoPortType.dwFlags) &&
		 !((vpDesc.VideoPortType.dwFlags & DDVPCONNECT_DOUBLECLOCK) ||	
		  (vpDesc.VideoPortType.dwFlags & DDVPCONNECT_HALFLINE) ||
		  (vpDesc.VideoPortType.dwFlags & DDVPCONNECT_SHAREEVEN) ||
		  (vpDesc.VideoPortType.dwFlags & DDVPCONNECT_SHAREODD)) )
		VidInData.dwConnectFlags = vpDesc.VideoPortType.dwFlags;

#ifdef WINNT
   VidInData.WeaveDeinterlacing = 0;
#else
   VidInData.WeaveDeinterlacing = TRUE;
#endif

   //Initialization of all VPE internal data structure and state flags should be done here
   //
   H3_InitBandwidth(ppdev);     //Initialize bandwidth info data-structure
   H3_InitializeAll(ppdev);     //Initialize all other

   //I2C_INIT(ppdev);                    //Initialize I2C
	//WRITEI2CREG(ppdev, 0xec, 0x0e, 0x04); //Power down 7003
   //I2C_INIT(ppdev);                    //Need to do it twice
	//WRITEI2CREG(ppdev, 0xec, 0x0e, 0x04); //to really kill 7003!
    if(!_FF(KMVTBuff))
    {
#ifndef WINNT
      IOControl(ppdev, VIDEO_SYS_BUF,1, 0 );
#else
      _FF(KMVTBuff) = (KMVTBUFF *)IOControl(ppdev,IOCTL_ALLOC_KMVT_MEMORY, 1, 0 );
#endif
    }
    lpKMBuff = (KMVTBUFF *)_FF( KMVTBuff);

    if(lpKMBuff == NULL)
    {
		pcvpd->ddRVal = DDERR_UNSUPPORTED;

      	return DDHAL_DRIVER_HANDLED;
    }

    lpKMBuff->dwStatus = 0;
    VidInData.dwVPEFlags = 0;

	if (_DD(bVMIPLDpresent))
	{
		if (!memcmp(&vpDesc.VideoPortType.guidTypeID, &DDVPTYPE_CCIR656, sizeof(GUID)))
		{
			DD_VMI_SetCommand(ppdev,VMI_VBICROP);  // not passthru, no vbi int, crop vbi
			_DD(bVMIPLDinUse) = TRUE;
		}
		else
		{
    		DD_VMI_SetCommand( ppdev, VMI_PASSTHRU); // passthru, no vbi int, no crop vbi
			_DD(bVMIPLDinUse) = FALSE;
    		DD_VMI_SetVbiMax(ppdev, 0x1f);
			DD_VMI_SetVidMax(ppdev, 0x3ff);
		}
	}
	lpKMBuff->bVMIPLDinUse = (BYTE)_DD(bVMIPLDinUse);

#ifdef WINNT
	_DD(bVideoPortActive) = TRUE;
#else
	//query if the WDM driver is active and use old scaling if so
	_DD(bVideoPortActive) = TRUE;
	if (_DD(bIgnoreWDM))
		_DD(fWDMVXDActive) = 0;
	else
	{
		DWORD dwVal = IOControl(ppdev, VDD_V3TV_WDMQUERYACTIVE, 0, 0);
		if (dwVal)
		{
			_DD(fWDMVXDActive) = 1;
			if (dwVal == 1)				// 1 is eav, 2 is sav
				_DD(dwAdjustForEav) = 1;	// 0 if sav used, 1 for eav - wdm dependent
			else
				_DD(dwAdjustForEav) = 0;
		}
	}
#ifdef FXTRACE
	Msg(ppdev, DEBUG_VPEENTRY, "Adjust for EAV = %d", _DD(dwAdjustForEav) );
#endif
#endif

	pcvpd->ddRVal = DD_OK;

  	return DDHAL_DRIVER_HANDLED;
}// vpCreateVideoPort32


/*----------------------------------------------------------------------
Function name: vpFlipVideoPort32

Description:   DDRAW video port callback FlipVideoPort

Return:        DWORD DDRAW result

               DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/

DWORD __stdcall vpFlipVideoPort32(LPDDHAL_FLIPVPORTDATA pfpvd)
{
   DWORD dwTargAddr;
   __int64 lFr;
   DDHAL_GETVPORTFLIPSTATUSDATA sStatusInput;
   DWORD dwVidSrcStride;
   KMVTBUFF * lpKMBuff;
   DD_ENTRY_SETUP(pfpvd->lpDD->lpGbl);

#ifdef FXTRACE
#ifndef WINNT
	Msg(ppdev, DEBUG_VPEENTRY, "Flip VideoPort32" );
#else
	DISPDBG(( DEBUG_VPEENTRY, "Flip VideoPort32" ));
#endif

#endif

   sStatusInput.lpDD = pfpvd->lpDD;

   vpGetVideoPortFlipstatus32 (&sStatusInput);

   if (sStatusInput.ddRVal == DD_OK)
   {
      dwTargAddr = GET_HW_ADDR(pfpvd->lpSurfTarg);

      lpKMBuff = (KMVTBUFF *)_FF( KMVTBuff);

      if(IS_TILED(dwTargAddr))
         dwVidSrcStride = _DS(ddTileStride);                //Tile buffer stride
      else
         dwVidSrcStride = pfpvd->lpSurfTarg->lpGbl->lPitch; //Linear buffer stride

      dwTargAddr += pfpvd->lpVideoPort->ddvpInfo.dwOriginY * dwVidSrcStride +
                    pfpvd->lpVideoPort->ddvpInfo.dwOriginX * VidInData.dwBpp;   //Target surface board address
      lpKMBuff->dwVPAddr0 = dwTargAddr;

      SETDW(ghwIO->vidInAddr0, dwTargAddr); //Set video-in address 0 to point to surface at X,Y

      // We need to remember when we performed
      // the flip (a circus act!) so that when someone later asks us if
      // it has occurred, we can tell them definitively.
#ifndef WINNT
      QueryPerformanceCounter ((LARGE_INTEGER *) &gsVportFlip.liFlipTime);
      gsVportFlip.bFlipFlag = TRUE;
      QueryPerformanceFrequency( (LARGE_INTEGER *) & lFr);
#else
      EngQueryPerformanceCounter ( &gsVportFlip.liFlipTime);
      gsVportFlip.bFlipFlag = TRUE;
      EngQueryPerformanceFrequency( & lFr);
#endif

      //The flip duration is the length of the field (MAX).
      gsVportFlip.dwFlipDuration =
              pfpvd->lpVideoPort->ddvpDesc.dwMicrosecondsPerField *
              (DWORD)lFr / 1000ul; //As counter

      pfpvd->ddRVal = DD_OK;
   }
   else
   {
      pfpvd->ddRVal = DDERR_WASSTILLDRAWING;
   }
   //return DDHAL_DRIVER_NOTHANDLED;
   return DDHAL_DRIVER_HANDLED;
}// vpFlipVideoPort32


/*----------------------------------------------------------------------
Function name: vpgetVideoPortBandwidth32

Description:   DDRAW video port callback GetVideoPortBandwidth

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
			   DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/

DWORD __stdcall vpGetVideoPortBandwidth32(LPDDHAL_GETVPORTBANDWIDTHDATA pgvpbd)
{
   RECT rVidSrc, rMaxVidSrcAllowed;
   DWORD dwMaxOverlayWidth, dwMaxOverlayHeight;
   DWORD dwDecimPctg;
   DD_ENTRY_SETUP(pgvpbd->lpDD->lpGbl);

#ifdef FXTRACE
#ifndef WINNT
	Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort Bandwidth32" );
#else
	DISPDBG(( DEBUG_VPEENTRY, "GetVideoPort Bandwidth32" ));
#endif

#endif

	if (pgvpbd->dwFlags & DDVPB_TYPE )
	{
		switch (pgvpbd->lpddpfFormat->dwFlags)
		{
			case DDPF_FOURCC:
               switch (pgvpbd->lpddpfFormat->dwFourCC)
               {
                   case FOURCC_YUY2:
                   case FOURCC_UYVY:
     				pgvpbd->lpBandwidth->dwCaps = DDVPBCAPS_SOURCE;  //Bandwidth is Overlay Source size dependent
       				break;
				   default:
         	            pgvpbd->ddRVal = DDERR_UNSUPPORTED;
          	            return DDHAL_DRIVER_NOTHANDLED;
               }
               break;
			default:
	            pgvpbd->ddRVal = DD_OK;
  	            return DDHAL_DRIVER_NOTHANDLED;
		}
	}
	else if (pgvpbd->dwFlags & DDVPB_VIDEOPORT )
	{
       pgvpbd->ddRVal = DDERR_UNSUPPORTED;     //Fail the call
       return DDHAL_DRIVER_NOTHANDLED;
	}
	else if (pgvpbd->dwFlags & DDVPB_OVERLAY )
	{
       //Overlay rectangle size
       rVidSrc.left   = rVidSrc.top = 0;
       rVidSrc.right  = pgvpbd->dwWidth;
       rVidSrc.bottom = pgvpbd->dwHeight;

      if( H3BandWidth.MClk== 0)
       {
        //W2K DCT300 uses different ppdev to call this function
         //make sure H3BandWidth is initialized
          H3_InitBandwidth(ppdev);
       }
       //Overlay pixel format
       switch (pgvpbd->lpddpfFormat->dwFlags)
       {
           case DDPF_FOURCC:
               switch (pgvpbd->lpddpfFormat->dwFourCC)
               {
                   case FOURCC_YUY2:
                   case FOURCC_UYVY:
                       H3BandWidth.OverlayBPP = 2; //Overlay bytes per pixel
                       break;
                   case FOURCC_RAW8:
                       H3BandWidth.OverlayBPP = 1; //Overlay bytes per pixel
                       break;
                   default:
                       pgvpbd->ddRVal = DDERR_UNSUPPORTED;
                       return DDHAL_DRIVER_NOTHANDLED;
               }
               break;
           default:
               pgvpbd->ddRVal = DD_OK;
               return DDHAL_DRIVER_NOTHANDLED;
       }

       //Return the max size of overlay source for max video bandwidth allowed.
       H3_Calculate_Bandwidth(ppdev, &rVidSrc, &rMaxVidSrcAllowed ,FALSE );
       dwMaxOverlayWidth  = rMaxVidSrcAllowed.right - rMaxVidSrcAllowed.left;
       dwMaxOverlayHeight = rMaxVidSrcAllowed.bottom - rMaxVidSrcAllowed.top;

		pgvpbd->lpBandwidth->dwCaps = DDVPBCAPS_SOURCE;  //Bandwidth is Overlay Source size dependent

       //For now we'll just use the worst bandwidth requirements (colorkey with interpolation)
       //for the overlay maximum size
       dwDecimPctg = (dwMaxOverlayWidth * dwMaxOverlayHeight * 1000)/(pgvpbd->dwWidth * pgvpbd->dwHeight);
		pgvpbd->lpBandwidth->dwOverlay = dwDecimPctg;
		pgvpbd->lpBandwidth->dwColorkey = dwDecimPctg;
		pgvpbd->lpBandwidth->dwYInterpolate = dwDecimPctg;
		pgvpbd->lpBandwidth->dwYInterpAndColorkey = dwDecimPctg;
	}
	pgvpbd->ddRVal = DD_OK;

  	return DDHAL_DRIVER_HANDLED;
}// vpgetVideoPortBandwidth32


/*----------------------------------------------------------------------
Function name: vpGetVideoPortInputFormats32

Description:   DDRAW video port callback GetVideoPortInputFormats

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall vpGetVideoPortInputFormats32(LPDDHAL_GETVPORTINPUTFORMATDATA pgvpifd)
{
	DDPIXELFORMAT vpPixFmt[H3_VIDEOIN_FORMATS];
    DD_ENTRY_SETUP(pgvpifd->lpDD->lpGbl);

#ifdef FXTRACE
    #ifndef WINNT
	Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort InputFormats32" );
#else
	DISPDBG(( DEBUG_VPEENTRY, "GetVideoPort InputFormats32" ));
#endif

#endif

	if ( pgvpifd->lpddpfFormat )
	{ 	
		memset(vpPixFmt, 0, sizeof(vpPixFmt));
		if ( pgvpifd->dwFlags & DDVPFORMAT_VIDEO )
		{
			vpPixFmt[0].dwSize   = sizeof(DDPIXELFORMAT);
			vpPixFmt[0].dwFlags  = DDPF_FOURCC;			//H3 support YUY2
			vpPixFmt[0].dwFourCC = FOURCC_YUY2;

			vpPixFmt[1].dwSize   = sizeof(DDPIXELFORMAT);
			vpPixFmt[1].dwFlags  = DDPF_FOURCC;			//H3 support UYVY
			vpPixFmt[1].dwFourCC = FOURCC_UYVY;

			memcpy(pgvpifd->lpddpfFormat, vpPixFmt, sizeof(vpPixFmt));
		}
		if ( pgvpifd->dwFlags & DDVPFORMAT_VBI ) 		
		{
#ifdef FXTRACE
			Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort InputFormats32: VBI");
#endif
			//VBI is supported with RAW8
			vpPixFmt[0].dwSize   = sizeof(DDPIXELFORMAT);
			vpPixFmt[0].dwFlags  = DDPF_FOURCC;			//H3 support YUY2
			vpPixFmt[0].dwFourCC = FOURCC_RAW8;
			vpPixFmt[0].dwYUVBitCount = 8;
			memcpy(&pgvpifd->lpddpfFormat[pgvpifd->dwNumFormats-1], &vpPixFmt[0], sizeof(DDPIXELFORMAT));
		}
	}
	else
	{	
		pgvpifd->dwNumFormats = 0; 	
		if ( pgvpifd->dwFlags & DDVPFORMAT_VIDEO )
		{
			pgvpifd->dwNumFormats += H3_VIDEOIN_FORMATS;//H3 support YUY2 & UYVY
		}
		if ( pgvpifd->dwFlags & DDVPFORMAT_VBI )
		{
#ifdef FXTRACE
			Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort InputFormats32: VBI");
#endif
			//VBI is supported with RAW8
			pgvpifd->dwNumFormats += 1; 			
		}
	}
	pgvpifd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
}// vpGetVideoPortInputFormats32


/*----------------------------------------------------------------------
Function name: vpGetVideoPortOutputFormats32

Description:   DDRAW video port callback GetVideoPortOutputFormats

               Fills in the specified array with all of the formats that
               can be written to the frame buffer based on the specified
               input format and puts that number in dwNumFormats of the
               DDHAL_GETVPORTOUTPUTFORMATDATA structure. If
               lpddpfOutputFormats is NULL, it only fills in dwNumFormats
               with the number of formats that can be written to the frame
               buffer. This callback is required

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall vpGetVideoPortOutputFormats32(LPDDHAL_GETVPORTOUTPUTFORMATDATA pgvpofd)
{
	DDPIXELFORMAT vpPixFmt;
    DD_ENTRY_SETUP(pgvpofd->lpDD->lpGbl);
		
#ifdef FXTRACE
    #ifndef WINNT
	Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort OutputFormats32" );
#else
	DISPDBG(( DEBUG_VPEENTRY, "GetVideoPort OutputFormats32" ));
#endif

#endif

	if ( pgvpofd->lpddpfOutputFormats )
	{
		if ( pgvpofd->dwFlags & DDVPFORMAT_VIDEO )
		{
			switch ( pgvpofd->lpddpfInputFormat->dwFlags )
			{
				case DDPF_FOURCC:
					switch (pgvpofd->lpddpfInputFormat->dwFourCC)
					{
                       case FOURCC_YUY2:
   						memset(&vpPixFmt, 0, sizeof(vpPixFmt));
   						vpPixFmt.dwSize = sizeof(DDPIXELFORMAT);
   						vpPixFmt.dwFlags= DDPF_FOURCC; 		//H3 support YUY2
   						vpPixFmt.dwFourCC = FOURCC_YUY2;
   						memcpy(pgvpofd->lpddpfOutputFormats, &vpPixFmt, sizeof(vpPixFmt));
                           VidInData.dwBpp = 2;                //Save video-in byte per pixel
                           break;
                       case FOURCC_UYVY:
   						memset(&vpPixFmt, 0, sizeof(vpPixFmt));
   						vpPixFmt.dwSize = sizeof(DDPIXELFORMAT);
   						vpPixFmt.dwFlags= DDPF_FOURCC; 		//H3 support UYVY
   						vpPixFmt.dwFourCC = FOURCC_UYVY;
   						memcpy(pgvpofd->lpddpfOutputFormats, &vpPixFmt, sizeof(vpPixFmt));
                           VidInData.dwBpp = 2;                //Save video-in byte per pixel

                           break;
					}
					break;
               default:
					pgvpofd->dwNumFormats = 0;
					break;
			}
		}											
		if ( pgvpofd->dwFlags & DDVPFORMAT_VBI ) 				//H3 does not support VBI
		{
#ifdef FXTRACE
            #ifndef WINNT
			Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort OutputFormats32: VBI" );
#else
			DISPDBG(( DEBUG_VPEENTRY, "GetVideoPort OutputFormats32: VBI" ));
#endif

#endif

			if (pgvpofd->lpddpfInputFormat->dwFlags & DDPF_FOURCC )
			{
				if (pgvpofd->lpddpfInputFormat->dwFourCC == FOURCC_RAW8)
				{
					memset(&vpPixFmt, 0, sizeof(vpPixFmt));
					vpPixFmt.dwSize = sizeof(DDPIXELFORMAT);
					vpPixFmt.dwFlags= DDPF_FOURCC; 		//H3 support UYVY
					vpPixFmt.dwFourCC = FOURCC_RAW8;
					vpPixFmt.dwYUVBitCount = 8;
  					memcpy (pgvpofd->lpddpfOutputFormats, &vpPixFmt, sizeof (DDPIXELFORMAT) );
				} 	
			}
		}
	}
	else
	{
       	pgvpofd->dwNumFormats = 0;
		if ( pgvpofd->dwFlags & DDVPFORMAT_VIDEO )
		{
			switch ( pgvpofd->lpddpfInputFormat->dwFlags )
			{
				case DDPF_FOURCC:
					switch (pgvpofd->lpddpfInputFormat->dwFourCC)
                   {
                       case FOURCC_YUY2:
						    pgvpofd->dwNumFormats = 1;
                           break;
                       case FOURCC_UYVY:
						    pgvpofd->dwNumFormats = 1;
                           break;
                       default:
						    pgvpofd->dwNumFormats = 0;
                           break;

                   }
					break;

               default:
				    pgvpofd->dwNumFormats = 0;
                   break;
			}
		}
		if ( pgvpofd->dwFlags & DDVPFORMAT_VBI )
		{
#ifdef FXTRACE
            #ifndef WINNT
			Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort OutputFormats32: VBI" );
#else
			DISPDBG(( DEBUG_VPEENTRY, "GetVideoPort OutputFormats32: VBI" ));
#endif

#endif
			if (pgvpofd->lpddpfInputFormat->dwFlags & DDPF_FOURCC ) {
				if (pgvpofd->lpddpfInputFormat->dwFourCC == FOURCC_RAW8) {
			   	    	pgvpofd->dwNumFormats = 1;
				} 		
			}
		}
	}
	pgvpofd->ddRVal = DD_OK;
  	return DDHAL_DRIVER_HANDLED;
}// vpGetVideoPortOutputFormats32


/*----------------------------------------------------------------------
Function name: vpGetVideoPortField32

Description:   DDRAW video port callback getVideoPortField

               Determines if the current field of an interlaced signal
               is even or odd.

Information:   This callback is only required if the DDVPCAPS_READBACKFIELD
               flag is set in the DDVIDEOPORTCAPS structure

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall vpGetVideoPortField32(LPDDHAL_GETVPORTFIELDDATA pgvpfd)
{
   DD_ENTRY_SETUP(pgvpfd->lpDD->lpGbl);

#ifdef FXTRACE
#ifndef WINNT
	Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort Field32" );
#else
	DISPDBG(( DEBUG_VPEENTRY, "GetVideoPort Field32" ));
#endif

#endif

   if ( GET(ghwIO->vidCurrentLine) & H4_VMI_FIELD_MASK )
       pgvpfd->bField = FALSE;     //The last field written is even, the current one is odd
   else
       pgvpfd->bField = TRUE;      //The last field written is odd, the current one is even

	pgvpfd->ddRVal = DD_OK;

  	return DDHAL_DRIVER_HANDLED;
}// vpGetVideoPortField32

/*----------------------------------------------------------------------
Function name: vpGetVideoPortLine32

Description:   DDRAW video port callback GetVideoPortLine

Information:   This function is only required if the driver sets the
               DDVPCAPS_READBACKLINE flag in the DDVIDEOPORTCAPS structure.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/

DWORD __stdcall vpGetVideoPortLine32(LPDDHAL_GETVPORTLINEDATA pgvpld)
{
   DD_ENTRY_SETUP(pgvpld->lpDD->lpGbl);

#ifdef FXTRACE
#ifndef WINNT
	Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort Line32" );
#else
	DISPDBG(( DEBUG_VPEENTRY, "GetVideoPort Line32" ));
#endif

#endif

   //H3 does not support reading of the current video line being written

	//pgvpld->ddRVal = DD_OK;

  	return DDHAL_DRIVER_NOTHANDLED;
}// vpGetVideoPortLine32


/*----------------------------------------------------------------------
Function name: vpGetVideoPortConnectInfo32

Description:   DDRAW video port callback GetVideoPortConnectInfo

               Fills in the specified array with all of the connection
               combinations supported by the specified video port and puts
               that number in the dwNumEntries member of the
               DDHAL_GETVPORTCONNECTDATA structure. If lpConnect is NULL,
               it only fills in dwNumEntries with the number of
               DDVIDEOPORTCONNECT entries supported

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
			   DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/
DWORD __stdcall vpGetVideoPortConnectInfo32(LPDDHAL_GETVPORTCONNECTDATA pgvpcd)
{
	DDVIDEOPORTCONNECT vpConnect[VSYNC_TYPES];
	int i;
	int base = 1;
    DD_ENTRY_SETUP(pgvpcd->lpDD->lpGbl);

#ifdef FXTRACE
    #ifndef WINNT
	Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort ConnectInfo32" );
#else
	DISPDBG(( DEBUG_VPEENTRY, "GetVideoPort ConnectInfo32" ));
#endif

#endif

	if (_DD(bVMIPLDpresent))
	{
		base = 0;
	}

	if ( pgvpcd->dwPortId == H3_PORT_ID )
	{
		if ( pgvpcd->lpConnect )
		{
			memset(vpConnect, 0, sizeof(vpConnect));

			for ( i = base; i < VSYNC_TYPES; i++ )
			{
				vpConnect[i - base].dwSize = sizeof(DDVIDEOPORTCONNECT);
				vpConnect[i - base].dwPortWidth = H3_VMI_DATA_BITS;
				vpConnect[i - base].dwFlags = DDVPCONNECT_DISCARDSVREFDATA|	//No VREF data for H3
									   DDVPCONNECT_HALFLINE		   |    //Capture half-line
									   DDVPCONNECT_INVERTPOLARITY  |	//Capable of swapping odd & even fields
									   DDVPCONNECT_VACT			   |
									   0;

				switch ( i )
				{	
					case VSYNC_CCIR656:
						vpConnect[i - base].guidTypeID = DDVPTYPE_CCIR656;
						vpConnect[i - base].dwFlags = DDVPCONNECT_DISCARDSVREFDATA|	//No VREF data for H3
									   DDVPCONNECT_HALFLINE		   |    //Capture half-line
									   DDVPCONNECT_INVERTPOLARITY  |	//Capable of swapping odd & even fields
									   0;
						break;
					case VSYNC_AHHREF_AHVREF:
						vpConnect[i - base].guidTypeID = DDVPTYPE_E_HREFH_VREFH;
						break;
					case VSYNC_AHHREF_ALVREF:
						vpConnect[i - base].guidTypeID = DDVPTYPE_E_HREFH_VREFL;
						break;
					case VSYNC_ALHREF_AHVREF:
						vpConnect[i - base].guidTypeID = DDVPTYPE_E_HREFL_VREFH;
						break;
					case VSYNC_ALHREF_ALVREF:
						vpConnect[i - base].guidTypeID = DDVPTYPE_E_HREFL_VREFL;
						break;
				}
			}
			memcpy(pgvpcd->lpConnect, vpConnect, sizeof(DDVIDEOPORTCONNECT) * (VSYNC_TYPES - base) );
		}
		else
		{
			pgvpcd->dwNumEntries =  VSYNC_TYPES -base;
		}
		pgvpcd->ddRVal = DD_OK;
  		return DDHAL_DRIVER_HANDLED;
	}
	return DDHAL_DRIVER_NOTHANDLED;
}// vpGetVideoPortConnectInfo32

/*----------------------------------------------------------------------
Function name: vpDestroyVideoPort32

Description:   DDRAW video port callback DestroyVideoPort

               Notifies the HAL when the video port is destroyed.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall vpDestroyVideoPort32(LPDDHAL_DESTROYVPORTDATA pdvpd)
{
   DD_ENTRY_SETUP(pdvpd->lpDD->lpGbl);

#ifdef FXTRACE
#ifndef WINNT
	Msg(ppdev, DEBUG_VPEENTRY, "Destroy VideoPort32" );
#else
	DISPDBG(( DEBUG_VPEENTRY, "Destroy VideoPort32" ));
#endif

#endif
	//Disable Video Port
	H3_VMI_Disable(ppdev);

   //May need to force turning off the video processor registers by calling UpdateOverlay?

   //Clear and reset all VPE internal data structure and state flags should be done here
   //
    if(_FF(KMVTBuff))
    {
#ifndef WINNT
      //Free KMVTBUFF
      IOControl(ppdev, VIDEO_SYS_BUF,0 , 0 );
#else
      IOControl( ppdev, IOCTL_ALLOC_KMVT_MEMORY, 0, 0 );
      _FF(KMVTBuff) = 0;
#endif
    }
    VidInData.dwVPEFlags = 0;

    VBIHeight = 0;
	_DD(fWDMVXDActive) = FALSE;
	_DD(bVideoPortActive) = FALSE;

	pdvpd->ddRVal = DD_OK;

  	return DDHAL_DRIVER_HANDLED;
}// vpDestroyVideoPort32


/*----------------------------------------------------------------------
Function name: vpGetVideoPortFlipstatus32

Description:   DDRAW video port callback GetVideoPortFlipStatus

               Indicates whether a VSYNC has occurred since the flip was
               performed on the specified surface

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall vpGetVideoPortFlipstatus32(LPDDHAL_GETVPORTFLIPSTATUSDATA pgvpfsd)
{
   DD_ENTRY_SETUP(pgvpfsd->lpDD->lpGbl);

#ifdef FXTRACE
#ifndef WINNT
	Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort Flipstatus32" );
#else
	DISPDBG(( DEBUG_VPEENTRY, "GetVideoPort Flipstatus32" ));
#endif

#endif

   //Indicates whether VSYNC has occurred since flipping to the specified surface
   if(gsVportFlip.bFlipFlag )
   {
      __int64 ttime;      //Current time

#ifdef WINNT
      EngQueryPerformanceCounter (&ttime);
#else
      QueryPerformanceCounter ((LARGE_INTEGER *) &ttime);
#endif
      if ((ttime - gsVportFlip.liFlipTime) <= gsVportFlip.dwFlipDuration)
      {
           pgvpfsd->ddRVal = DDERR_WASSTILLDRAWING;
      }
      else
      {
        gsVportFlip.bFlipFlag = FALSE;
        pgvpfsd->ddRVal = DD_OK;
      }
   }
   else
       pgvpfsd->ddRVal = DD_OK;

  	return DDHAL_DRIVER_HANDLED;
}// vpGetVideoPortFlipstatus32


/*----------------------------------------------------------------------
Function name: vpUpdateVideoPort32

Description:   DDRAW video port callback UpdateVideoPort

               Starts, stops, and changes the video port.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
			   DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/
DWORD __stdcall vpUpdateVideoPort32(LPDDHAL_UPDATEVPORTDATA puvpd)
{
   DWORD dwReg, dwVPFlags, dwVPOffset;
   DWORD dwVidBufAddr0, dwVidBufAddr1, dwVidBufAddr2;
   RECT rVidSrc;
   KMVTBUFF * lpKMBuff;
   DD_ENTRY_SETUP(puvpd->lpDD->lpGbl);

#ifdef FXTRACE
#ifndef WINNT
	Msg(ppdev, DEBUG_VPEENTRY, "Update VideoPort32" );
#else
	DISPDBG(( DEBUG_VPEENTRY, "Update VideoPort32" ));
#endif

#endif

	//Starts VideoPort or update VideoPort registers
	if ( puvpd->dwFlags == DDRAWI_VPORTSTART || puvpd->dwFlags == DDRAWI_VPORTUPDATE )
	{
#ifdef FXTRACE
       if ( puvpd->dwFlags == DDRAWI_VPORTSTART )
        #ifndef WINNT
		    Msg(ppdev, DEBUG_VPEENTRY, "Start VideoPort" );
#else
		DISPDBG((DEBUG_VPEENTRY, "Start VideoPort" ));
#endif

       else
        #ifndef WINNT
		    Msg(ppdev, DEBUG_VPEENTRY, "Update VideoPort" );
#else
		DISPDBG((DEBUG_VPEENTRY, "Update VideoPort" ));
#endif

#endif


		//Handle VBI and return - code below will mess up VBI surfaces
	   if (puvpd->lplpDDVBISurface && !puvpd->lplpDDSurface) {
            puvpd->ddRVal = DD_OK;
	        return DDHAL_DRIVER_HANDLED;
  	   }


       VidInData.dwVPEFlags = VPORT_START;

		//Program Video Input Port registers:
		//Video in width: 	puvpd->lpVideoPort->ddvpDesc.dwFieldWidth
		//Video in height: 	puvpd->lpVideoPort->ddvpDesc.dwFieldHeight
		//Videoport connection characteristics:	HSync/VSync type, DblClk, Interlaced, VACT
		//	puvpd->lpVideoPort->ddvpDesc.VideoPortType.guidTypeID, dwFlags
		//
		//Video buffers:
		//  Autoflip buffer number:     puvpd->dwNumAutoflip
		//  Buffer addresses:			puvpd->lplpDDSurface->lpLcl->lpGbl->fpVidMem,
		//  Buffer height/width/stride: puvpd->lplpDDSurface->lpLcl->lpGbl->wHeight, wWidth, lPitch
		//  Buffer pixel format:		puvpd->lplpDDSurface->lpLcl->lpGbl.ddpfSurface
		//	X & Y offsets:  			puvpd->lpVideoInfo->dwOriginX, dwOriginY
		//	Autoflip:					puvpd->lpVideoInfo->dwVPFlags & DDVP_AUTOFLIP
		//	Crop:						puvpd->lpVideoInfo->dwVPFlags & DDVP_CROP
		//	Weave/Interleaved mode:		puvpd->lpVideoInfo->dwVPFlags & DDVP_INTERLEAVE
		//	Prescale:					puvpd->lpVideoInfo->dwVPFlags & DDVP_PRESCALE
		//  Field mode:					puvpd->lpVideoInfo->dwVPFlags & DDVP_SKIPODDFIELDS, DDVP_SKIPEVENFIELDS
		//  Genlock CRTC to Video:		puvpd->lpVideoInfo->dwVPFlags & DDVP_SYNCMASTER
		//  Prescale width:				puvpd->lpVideoInfo->dwPrescaleWidth
		//  Prescale height:			puvpd->lpVideoInfo->dwPrescaleHeight
		//  Input format: YUV/RGB		puvpd->lpVideoInfo->lpddfInputFormat->dwFlags & DDPF_RGB, DDPF_YUV
		//
		
		//Read video port flags
		dwVPFlags = puvpd->lpVideoInfo->dwVPFlags;

		//Setup Video In Format register:
		dwReg = GET(ghwIO->vidInFormat);
		dwReg &= ~(	H3_VMI_DATA_FORMAT_MASK	|	//YUY2 or UYVY
					H3_VMI_DEINTERLACE_MASK 	|	//No weave interlace mode
					H3_VMI_VSYNC_POLAR_MASK		|	//Vsync active high
					H3_VMI_HSYNC_POLAR_MASK		|	//Hsync active high
					H3_VMI_VACT_POLAR_MASK		|	//VAct high?
					H3_VMI_BUFFER_MODE_MASK		|	//Triple or double buffer mode
					H3_VMI_TILE_SPACE_MASK		|	//Linear space
					H4_VMI_MODE_MASK			   |	//VMI mode
					//H3_VMI_TVOUT_GENLOCK_MASK	|	//TV Out genlock?
					//H3_VMI_VGA_TIMING_MASK	|	//TV Out VGA timing?
					H3_VMI_HDECIMATION_MASK		|	//Enable horizontal decimation
					H3_VMI_VDECIMATION_MASK		|	//Enable vertical decimation
					0 );

		dwReg |=
             H4_VMI_MODE_VMI			   |		  //Set the port mode to Video Module Interface mode
 				 H3_VMI_HDECIMATION_ENABLE	|       //Enable horz decimation
				 H3_VMI_VDECIMATION_ENABLE	|       //Enable vert decimation
				 0;

//#ifndef WINNT   //V3TV      
#ifdef WINNT   //V3TV      
		lpKMBuff = (KMVTBUFF *)_FF( KMVTBuff);
		if (!lpKMBuff)
	        return DDHAL_DRIVER_HANDLED;
		if (lpKMBuff->fWDMActive)
		{
			_DD(fWDMVXDActive) = 1;
			_DD(bUseWDMScaling) = 1;
		}
#endif
		if (_DD(fWDMVXDActive) && _DD(bUseWDMScaling)) {
			dwReg &=  	
				 ~(H3_VMI_HDECIMATION_ENABLE);       //disable horz decimation
		}
//#endif
		//VBI data may not have input format set
		if ( !puvpd->lpVideoInfo->lpddpfInputFormat) {
	       	       puvpd->ddRVal = DD_OK;
        	       return DDHAL_DRIVER_NOTHANDLED;
		}

		//Video In pixel format
		switch ( puvpd->lpVideoInfo->lpddpfInputFormat->dwFlags )
		{
	    case DDPF_FOURCC:
               switch (puvpd->lpVideoInfo->lpddpfInputFormat->dwFourCC)
               {
                   case FOURCC_UYVY:
                        dwReg |= H3_VMI_DATA_UYVY; //UYVY
                        VidInData.dwPixFmt = FOURCC_UYVY;
                        VidInData.dwBpp = H3BandWidth.OverlayBPP = 2;
                        break;
         			case FOURCC_YUY2:
                   default:
                        dwReg |= H3_VMI_DATA_YUYV;	//YUYV
                        VidInData.dwPixFmt = FOURCC_YUY2;
                        VidInData.dwBpp = H3BandWidth.OverlayBPP = 2;
                        break;
               }
               break;
           default:
               puvpd->ddRVal = DD_OK;
               return DDHAL_DRIVER_NOTHANDLED;
		}

      //Check if should WEAVE deinterlacing
      if ( dwVPFlags & DDVP_INTERLEAVE )
      {
          VidInData.WeaveDeinterlacing |= INTERLEAVED_VIDEO;
          if(!(VidInData.WeaveDeinterlacing & BOB_INTERLEAVED))
           //if Not BOB_INTERLEAVED mode -- set in ddovl32.c
              dwReg |= H3_VMI_DEINTERLACE_WEAVE;
      }
      else
      {
          VidInData.WeaveDeinterlacing &= ~INTERLEAVED_VIDEO;
      }

      //Vsync, Hsync polarity
      if (_DD(bVMIPLDpresent) && !memcmp(&VidInData.gConnectTypeID, &DDVPTYPE_CCIR656, sizeof(GUID)) )
          dwReg |= H3_VMI_VSYNC_ACTIVE_LOW;
	  else if ( !memcmp(&VidInData.gConnectTypeID, &DDVPTYPE_E_HREFH_VREFL, sizeof(GUID)) )
          dwReg |= H3_VMI_VSYNC_ACTIVE_LOW;
      else if ( !memcmp(&VidInData.gConnectTypeID, &DDVPTYPE_E_HREFL_VREFH, sizeof(GUID)) )
          dwReg |= H3_VMI_HSYNC_ACTIVE_LOW;
      else if ( !memcmp(&VidInData.gConnectTypeID, &DDVPTYPE_E_HREFL_VREFL, sizeof(GUID)) )
          dwReg |= H3_VMI_VSYNC_ACTIVE_LOW|H3_VMI_HSYNC_ACTIVE_LOW;

      //VACT signal: no info found regarding which VACT polarity to be used, default to active high for now
      //if ( VidInData.dwConnectFlags & DDVPCONNECT_VACT )
      //    dwReg |= H3_VMI_VACT_ACTIVE_LOW;

      lpKMBuff = (KMVTBUFF *)_FF( KMVTBuff);

//#ifndef WINNT   //V3TV
      //We use the rcrop since this is the information provided from the WDM driver specifying the
      //the VBI at the top that shouldn't be visible for video.
	  VBIHeight = puvpd->lpVideoInfo->dwVBIHeight;

	  if (puvpd->lpVideoInfo->dwVPFlags & DDVP_CROP)		// do cropping
	  {
		  lpKMBuff->bVBIcropped = TRUE;
		  VBIHeight = puvpd->lpVideoInfo->rCrop.top;
	  }
	  else
	  {
		  VBIHeight = puvpd->lpVideoInfo->dwVBIHeight;
		  lpKMBuff->bVBIcropped = (puvpd->lpVideoInfo->dwVBIHeight == 0);
	  }
	  lpKMBuff->dwVideoLines = puvpd->lpVideoInfo->rCrop.bottom - VBIHeight;
      lpKMBuff->dwCounter = 0;			// used to throw out  fields in h3irq
	
	  lpKMBuff->bUseReset = TRUE;
	  lpKMBuff->bUsingEAV = (BYTE)_DD(dwAdjustForEav);

	  if (lpKMBuff->bVMIPLDinUse)
	  {
		    VBIHeight -= _DD(dwAdjustForEav);		// first line lost on eav - do after calc of videolines
			DD_VMI_SetCommand(ppdev, VMI_VBIINT| VMI_VBICROP);	// not passthru - 656 mode
                                                       // always need for last buffer.
			DD_VMI_SetVbiMax(ppdev, 1);		// vbi is actually in video
			if (lpKMBuff->bVBIcropped)
				lpKMBuff->dwPLDVidLines = lpKMBuff->dwVideoLines - _DD(dwAdjustForEav);
			else
				lpKMBuff->dwPLDVidLines = lpKMBuff->dwVideoLines + VBIHeight;
			DD_VMI_SetVidMax(ppdev, lpKMBuff->dwPLDVidLines);
	  }

	  lpKMBuff->dwVBILinesOrig = VBIHeight;
	  lpKMBuff->dwVideoLinesOrig = lpKMBuff->dwVideoLines;
#ifdef WINNT
	  lpKMBuff->decoderWidth = puvpd->lpVideoPort->ddvpDesc.dwFieldWidth;
#endif

      if (dwReg & H3_VMI_DEINTERLACE_WEAVE)
	  {
	    VBIHeight <<= 1;
		lpKMBuff->dwVideoLines <<= 1;
	  }

	  lpKMBuff->dwVBILines = VBIHeight;
      if (puvpd->lplpDDSurface)
	  {
		lpKMBuff->dwVideoSurfacePitch = (*puvpd->lplpDDSurface)->lpLcl->lpGbl->lPitch;  // need this in case tiled
		lpKMBuff->fpVidMem[0] = (*puvpd->lplpDDSurface)->lpLcl->lpGbl->fpVidMem;
		lpKMBuff->fpVidMem[1] = (puvpd->dwNumAutoflip < 2) ? lpKMBuff->fpVidMem[0] : (*(puvpd->lplpDDSurface+1))->lpLcl->lpGbl->fpVidMem;
		lpKMBuff->fpVidMem[2] = (puvpd->dwNumAutoflip < 3) ? lpKMBuff->fpVidMem[0] : (*(puvpd->lplpDDSurface+2))->lpLcl->lpGbl->fpVidMem;
#ifdef WINNT  //update with correct linear address offset
		lpKMBuff->fpVidMem[0] += (ULONG)ppdev->pjLfbBase;
		lpKMBuff->fpVidMem[1] += (ULONG)ppdev->pjLfbBase;
		lpKMBuff->fpVidMem[2] += (ULONG)ppdev->pjLfbBase;
		DISPDBG((DEBUG_VPEENTRY, "\tVP fp0=%x fp1=%x fp2=%x",lpKMBuff->fpVidMem[0],lpKMBuff->fpVidMem[1],lpKMBuff->fpVidMem[2] ));		
#endif
      }

//#endif

      if ( dwVPFlags & DDVP_AUTOFLIP )
      {
         //Initialize VideoIn buffer address and stride registers
         switch (puvpd->dwNumAutoflip)
         {
             case 0:
                 puvpd->ddRVal = DD_OK;      //No surface for video buffer
                 return DDHAL_DRIVER_HANDLED;

             case 1:
                 //There is only one VideoOverlaySurface:  this can not work without tearing
                 dwVidBufAddr0 = GET_HW_ADDR((*puvpd->lplpDDSurface)->lpLcl);//->lpGbl->dwReserved1;      //Get the surface board addr
                 dwVidBufAddr1 = dwVidBufAddr2 = dwVidBufAddr0;

                 dwReg |= H3_VMI_SINGLE_BUFFER;
                 VidSrcData.wNumbufs = 1;
                 break;

             case 2:
                 //There are two VideoOverlaySurfaces
                 dwVidBufAddr0 = GET_HW_ADDR((*puvpd->lplpDDSurface)->lpLcl);//->lpGbl->dwReserved1;      //Get the surface board addr
                 dwVidBufAddr1 = GET_HW_ADDR((*(puvpd->lplpDDSurface+1))->lpLcl);//->lpGbl->dwReserved1;  //Get the next surface board addr
                 dwVidBufAddr2 = dwVidBufAddr1;

                 dwReg |= H3_VMI_DOUBLE_BUFFER;
                 VidSrcData.wNumbufs = 2;
                 break;

             case 3:
             default:
                 //There are three or more VideoOverlaySurfaces
                 dwVidBufAddr0 = GET_HW_ADDR((*puvpd->lplpDDSurface)->lpLcl);//->lpGbl->dwReserved1;      //Get the surface board addr
                 dwVidBufAddr1 = GET_HW_ADDR((*(puvpd->lplpDDSurface+1))->lpLcl);//->lpGbl->dwReserved1;  //Get the next surface board addr
                 dwVidBufAddr2 = GET_HW_ADDR((*(puvpd->lplpDDSurface+2))->lpLcl);//->lpGbl->dwReserved1;  //Get the next surface board addr

                 dwReg |= H3_VMI_TRIPLE_BUFFER;
                 VidSrcData.wNumbufs = 3;
                 break;
         }
      }
      else
      {
         //Videoport is in manual flip mode:
         dwVidBufAddr0 = GET_HW_ADDR((*puvpd->lplpDDSurface)->lpLcl);//->lpGbl->dwReserved1;  //Get the surface board addr
         dwVidBufAddr1 = dwVidBufAddr2 = dwVidBufAddr0;
         dwReg |= H3_VMI_SINGLE_BUFFER;
         VidSrcData.wNumbufs = 1;
      }

      H3_VMI_Disable(ppdev);                           //Disable video port

      //Set the video-in buffer stride in number of tiles or bytes
      if(IS_TILED(dwVidBufAddr0))
      {
         dwReg |= H3_VMI_TILE_SPACE_MASK;                //Tile space
         VidSrcData.dwVidSrcStride = _DS(ddTileStride);  //Tile buffer stride
      }
      else
      {
          VidSrcData.dwVidSrcStride = (*puvpd->lplpDDSurface)->lpLcl->lpGbl->lPitch;  //Linear buffer stride
      }

      dwVPOffset = puvpd->lpVideoInfo->dwOriginY * VidSrcData.dwVidSrcStride +
                   puvpd->lpVideoInfo->dwOriginX * VidInData.dwBpp;

      dwVidBufAddr0 += dwVPOffset;
      dwVidBufAddr1 += dwVPOffset;
      dwVidBufAddr2 += dwVPOffset;

      SETDW(ghwIO->vidInAddr0, dwVidBufAddr0);    //Set video-in address 0 to point to surface
      SETDW(ghwIO->vidInAddr1, dwVidBufAddr1);    //Set video-in address 1 to point to surface
      SETDW(ghwIO->vidInAddr2, dwVidBufAddr2);    //Set video-in address 2 to point to surface

      //Set the video-in buffer stride in number of tiles or bytes
      if(IS_TILED(dwVidBufAddr0))
      {
		  dwReg |= H3_VMI_TILE_SPACE_MASK;                //Tile space
          VidSrcData.dwVidSrcStride = _DS(ddTileStride);  //Tile buffer stride
      }
      else
      {
          VidSrcData.dwVidSrcStride = (*puvpd->lplpDDSurface)->lpLcl->lpGbl->lPitch;  //Linear buffer stride
      }
     lpKMBuff->dwStatus &= ~(VP_BUFF_MASK ); //|BUFFER_IN_USE_MASK) ;
     lpKMBuff->dwStatus  |= VidSrcData.wNumbufs << VP_BUFF_SHIFT;
     lpKMBuff->dwVPAddr0 = dwVidBufAddr0;
     lpKMBuff->dwVPAddr1 = dwVidBufAddr1;
     lpKMBuff->dwVPAddr2 = dwVidBufAddr2;
     lpKMBuff->dwVPStride = VidSrcData.dwVidSrcStride;
#ifndef WINNT
     {
      BYTE bScReg2;
       outp ((FxU16)(_FF(ioBase) + 0xd4), 0x1e);
	   bScReg2 =inp ((FxU16)(_FF(ioBase) + 0xd5));
       if( (bScReg2 & BIOS_TVOUT_ACTIVE) &&
           (bScReg2 & BIOS_PAL))
          VidInData.WeaveDeinterlacing &= ~CAN_SIMULATE;
       else
           VidInData.WeaveDeinterlacing |= CAN_SIMULATE;
      }
#else
	 //the Subvendor id tells us where we are ntsc (0x60) or pal anything else
	 //only ntsc can handle simulation of interalacing
	 if (ppdev->usSubSystemID == 0x60)
		VidInData.WeaveDeinterlacing |= CAN_SIMULATE;  
	 else
        VidInData.WeaveDeinterlacing &= ~CAN_SIMULATE;
#endif

     if((lpKMBuff->dwStatus & INTERLEAVE_ON) &&
         ( VidInData.WeaveDeinterlacing & INTERLEAVED_VIDEO ))
     {
          dwReg &= ~(H3_VMI_BUFFER_MODE_MASK |H3_VMI_DEINTERLACE_MASK);
          dwReg |= H3_VMI_DOUBLE_BUFFER;  //double buffering only
          SETDW(ghwIO->vidInStride, VidSrcData.dwVidSrcStride << 1);
		   //V3TV
		  VidSrcData.wNumbufs = 2;
		  lpKMBuff->dwStatus &= ~(VP_BUFF_MASK );
		  lpKMBuff->dwStatus  |= VidSrcData.wNumbufs << VP_BUFF_SHIFT;
     }
     else
          SETDW(ghwIO->vidInStride, VidSrcData.dwVidSrcStride);

	  //V3TV
	  lpKMBuff->dwVidInFormat = dwReg;

      if(IS_NAPALM)
       dwReg |=H5_VMI_VFREF_FLUSH | H5_VMI_VACT_FLUSH;

      SETDW(ghwIO->vidInFormat, dwReg);   //Set VideoIn format register


      //Setup VideoSerialParallelPort
      dwReg = GET(ghwIO->vidSerialParallelPort);
      dwReg &= ~( H3_VMI_CS_N_MASK			|	//VMI device select?
                  H3_VMI_OUTPUT_ENABLE_MASK	|	//VMI output disable
                  H3_VMI_RESET_MASK			|	//VMI reset
                  H3_VMI_GPIO1_MASK			|  //VMI device select on shared pins
                  0 );
      dwReg |= 	H3_VMI_OUTPUT_DISABLE	|	//VMI data input
   			   0;
      SETDW(ghwIO->vidSerialParallelPort, dwReg); //Set Video Serial Parallel Port register

      if (puvpd->lpVideoInfo->dwVPFlags & DDVP_PRESCALE)
      {
          VidInData.dwVidSrcWidth  =
        	        puvpd->lpVideoInfo->dwPrescaleWidth;
          VidInData.dwVidSrcHeight =
        	        puvpd->lpVideoInfo->dwPrescaleHeight;

//#ifdef WINNT  //V3TV
         if(puvpd->lpVideoInfo->dwVPFlags & DDVP_CROP)
         {
              VidInData.dwVidSrcHeight +=
        	        puvpd->lpVideoInfo->rCrop.top;
         }
//#endif
      }
      else
      {
          VidInData.dwVidSrcWidth  =
                puvpd->lpVideoPort->ddvpDesc.dwFieldWidth;
          VidInData.dwVidSrcHeight =
                puvpd->lpVideoPort->ddvpDesc.dwFieldHeight;
//#ifndef WINNT //V3TV
		 if(_DD(bVideoPortActive))
		 {
			VidInData.dwVidSrcHeight -= lpKMBuff->dwVBILinesOrig;
			if (lpKMBuff->bVMIPLDinUse)
				VidInData.dwVidSrcHeight -= _DD(dwAdjustForEav);
		 }
//#endif
      }

      //If UpdateOverlay has never been called, initialize video source rect to overlay surface dimension
      //and set video destination equal to video source
      if (!_DD(fUpdateOverlay))
      {
          VidSrcData.rVidSrc.left = 0;
          VidSrcData.rVidSrc.top  = 0;
          VidSrcData.rVidSrc.right  = (*puvpd->lplpDDSurface)->lpLcl->lpGbl->wWidth;      //Buffer width
          VidSrcData.rVidSrc.bottom = (*puvpd->lplpDDSurface)->lpLcl->lpGbl->wHeight;     //Buffer height
          VidDstData.rVidDst = VidSrcData.rVidSrcRgn = VidSrcData.rVidSrc;
      }

      //Calculate decimation of the Video In fields into the video buffers,
      //taking into account the bandwidth requirements and the final destination
      //video window size
//#ifndef WINNT	// let overlay do it for wdm cases - call for all other cases
	  if (!_DD(fWDMVXDActive))
//#endif
      H3_VMI_SetDecimation(ppdev, &VidDstData.rVidDst, &rVidSrc);

      H3_VMI_Reset(ppdev);                             //All set?

      if (lpKMBuff->bVMIPLDinUse)
	  {
	      DD_VMI_SetCommand(ppdev, VMI_VBIINT| VMI_VBICROP);	// not passthru - 656 mode
                                                        // always need for last buffer.
	      DD_VMI_SetVbiMax(ppdev, 1);		// vbi is actually in video
	      DD_VMI_SetVidMax(ppdev, lpKMBuff->dwPLDVidLines);
	  }
	  else  if(_DD(bVMIPLDpresent))
		   DD_VMI_SetCommand(ppdev, VMI_PASSTHRU);// passthru, no vbi int, no crop vbi

      H3_VMI_Enable_Func(ppdev);                       //Go!

      //May need to force turning on or updating the video processor registers by calling UpdateOverlay?
	}
	//Stop VideoPort
	else if ( puvpd->dwFlags == DDRAWI_VPORTSTOP )
	{
#ifdef FXTRACE
        #ifndef WINNT
		Msg(ppdev, DEBUG_VPEENTRY, "Stop VideoPort" );
#else
		DISPDBG(( DEBUG_VPEENTRY, "Stop VideoPort" ));
#endif

#endif
        VidInData.dwVPEFlags = VPORT_STOP;

#ifndef WINNT //V3TV
		VSYNC_IRQ_DISABLE;								// shut off flip overlay
#endif
		H3_VMI_Disable(ppdev);                           //Disable video port
	}

	puvpd->ddRVal = DD_OK;

  	return DDHAL_DRIVER_HANDLED;
}// vpUpdateVideoPort32


/*----------------------------------------------------------------------
Function name: vpWaitForVideoPortSync32

Description:   DDRAW video port callback WaitForVideoPortSync

               Returns at the beginning or end of either the video VSYNC

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall vpWaitForVideoPortSync32(LPDDHAL_WAITFORVPORTSYNCDATA pwfvpsd)
{
	DWORD inpFld;
    __int64 ttimeout;
    __int64 ttime;
    __int64 lFr;
   DD_ENTRY_SETUP(pwfvpsd->lpDD->lpGbl);

#ifdef FXTRACE
//	Msg(ppdev, DEBUG_VPEENTRY, "WaitFor VideoPortSync32" );
#endif

   if( pwfvpsd->dwFlags & ( DDVPWAIT_BEGIN | DDVPWAIT_END ) )
   {
      pwfvpsd->ddRVal = DDERR_VIDEONOTACTIVE;   // assume failure
   }
   else
   {
      pwfvpsd->ddRVal = DDERR_UNSUPPORTED;   //doesn't support other flags
      return (DDHAL_DRIVER_HANDLED);
   }

#ifdef WINNT
      EngQueryPerformanceFrequency( & lFr);
#else
      QueryPerformanceFrequency( (LARGE_INTEGER *) & lFr);
#endif

	// first, do some checks to see if we are in the right mode
	if ((GET(ghwIO->vidInFormat) & H3_VMI_MODE_MASK) != H3_VMI_MODE_VMI)
		return (DDHAL_DRIVER_HANDLED);

	if (!(GET(ghwIO->vidSerialParallelPort) & H3_VMI_ENABLE))
		return (DDHAL_DRIVER_HANDLED);

#ifndef WINNT
   if( pwfvpsd->dwTimeOut > 30 )
      ttimeout =   30 * lFr / 1000;     //only wait for 20ms, which is 50 fps for PAL
   else
      ttimeout = pwfvpsd->dwTimeOut * lFr / 1000;
#else
   if( pwfvpsd->dwTimeOut > 20 )
      ttimeout =   30 * lFr / 1000;     //only wait for 20ms, which is 50 fps for PAL
   else
      ttimeout = pwfvpsd->dwTimeOut * lFr / 100;
#endif


#ifdef WINNT
   EngQueryPerformanceCounter ( &ttime);
#else
   QueryPerformanceCounter ((LARGE_INTEGER *) &ttime);
#endif
   ttimeout += ttime;

   inpFld = GET(ghwIO->vidCurrentLine);

   while (1)
   {
      // Edge detect odd/even field or buffer changes, this occurs at video input vertical sync
      if ( (inpFld ^ GET(ghwIO->vidCurrentLine)) & (H4_VMI_FIELD_MASK|H4_VMI_BUFFER_MASK) )
      {
      	pwfvpsd->ddRVal = DD_OK;
         break;
      }

#ifdef WINNT
   EngQueryPerformanceCounter (&ttime);
#else
   QueryPerformanceCounter ((LARGE_INTEGER *) &ttime);
#endif

      if( ttime> ttimeout)
       {
        break;
      }
   }

    return (DDHAL_DRIVER_HANDLED);
}// vpWaitForVideoPortSync32

/*----------------------------------------------------------------------
Function name: vpGetVideoSignalStatus32

Description:   DDRAW video port callback GetVideoSignalStatus

               Indicates whether the video port is receiving a good
               signal or not.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall vpGetVideoSignalStatus32(LPDDHAL_GETVPORTSIGNALDATA pgvpsd)
{
   DWORD inpFld;
   __int64 ttime,ttimeout,lFr;      //Current time
   DD_ENTRY_SETUP(pgvpsd->lpDD->lpGbl);

#ifdef FXTRACE
#ifndef WINNT
	Msg(ppdev, DEBUG_VPEENTRY, "GetVideoSignalStatus32" );
#else
	DISPDBG(( DEBUG_VPEENTRY, "GetVideoSignalStatus32" ));
#endif

#endif
   pgvpsd->dwStatus = DDVPSQ_NOSIGNAL;      //default as no signal

   //Uses the change of field ID to test video signal?

	//First, do some checks to see if we are in the right mode
	if (((GET(ghwIO->vidInFormat) & H3_VMI_MODE_MASK) == H3_VMI_MODE_VMI)
	  && (GET(ghwIO->vidSerialParallelPort) & H3_VMI_ENABLE))

    {
#ifdef WINNT
       EngQueryPerformanceFrequency(  & lFr);
       EngQueryPerformanceCounter ( &ttimeout);
       ttimeout += 30 * lFr / 1000;     //only wait for 20ms, which is 50 fps for PAL
#else
       QueryPerformanceFrequency( (LARGE_INTEGER *) & lFr);
       QueryPerformanceCounter ((LARGE_INTEGER *) &ttimeout);
       ttimeout += 30 * lFr / 1000;     //only wait for 20ms, which is 50 fps for PAL
#endif
       inpFld = GET(ghwIO->vidCurrentLine);

       do
       {
         if ( (inpFld ^ GET(ghwIO->vidCurrentLine)) & (H4_VMI_FIELD_MASK|H4_VMI_BUFFER_MASK) )
         {
            pgvpsd->dwStatus = DDVPSQ_SIGNALOK;
            break;
         }
#ifdef WINNT
         EngQueryPerformanceCounter ( &ttime);
#else
         QueryPerformanceCounter ((LARGE_INTEGER *) &ttime);
#endif

       }while ( ttime < ttimeout);
    }

	pgvpsd->ddRVal = DD_OK;

  	return DDHAL_DRIVER_HANDLED;
}// vpGetVideoSignalStatus32


/*----------------------------------------------------------------------
Function name: vpColorControl32

Description:   DDRAW video port callback ColorControl

               Gets or sets the video port color controls.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall vpColorControl32(LPDDHAL_VPORTCOLORDATA pvpcd)
{
   DD_ENTRY_SETUP(pvpcd->lpDD->lpGbl);

#ifdef FXTRACE
#ifndef WINNT
	Msg(ppdev, DEBUG_VPEENTRY, "ColorControl32" );
#else
	DISPDBG(( DEBUG_VPEENTRY, "ColorControl32" ));
#endif

#endif
   //H3 does not provide color control on the data being written to frame buffer

	pvpcd->ddRVal = DD_OK;

  	return DDHAL_DRIVER_HANDLED;
}// vpColorControl32


/*----------------------------------------------------------------------
Function name: ovlColorControl32

Description:   DDRAW color control callback ColorControl

               Controls the luminance and brightness controls of an
               overlay or a primary surface.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall ovlColorControl32(LPDDHAL_COLORCONTROLDATA pccd)
{
	//DDCOLORCONTROL ovlColorCtrl;

   DD_ENTRY_SETUP(pccd->lpDD);

#ifdef FXTRACE
#ifndef WINNT
	Msg(ppdev, DEBUG_VPEENTRY, "Overlay ColorControl32" );
#else
	DISPDBG(( DEBUG_VPEENTRY, "Overlay ColorControl32" ));
#endif

#endif

	if ( pccd->dwFlags & DDRAWI_GETCOLOR )	//Get current color control
	{
	}
	if ( pccd->dwFlags & DDRAWI_SETCOLOR )	//Set current color control
	{
	}	
	pccd->ddRVal = DD_OK;

  	return DDHAL_DRIVER_HANDLED;
}// ovlColorControl32



/*----------------------------------------------------------------------
Function name: kSyncSurfaceData32

Description:   DDRAW Kernel callback SyncSurfaceData

               Allows the HAL to set and massage surface data before it
               is passed to the mini-VDD

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall kSyncSurfaceData32(LPDDHAL_SYNCSURFACEDATA pssd)
{
   FXSURFACEDATA *SurfaceData;
   DD_ENTRY_SETUP(pssd->lpDD->lpGbl);

#ifdef FXTRACE
#ifndef WINNT
	Msg(ppdev, DEBUG_VPEENTRY, "SyncSurfaceData32" );
#else
	DISPDBG(( DEBUG_VPEENTRY, "SyncSurfaceData32" ));
#endif

#endif
#ifndef WINNT
   pssd->dwSize = sizeof( DDHAL_SYNCSURFACEDATA);
#endif
   //pass the global data point first
   pssd->dwDriverReserved1 = (ULONG_PTR)ppdev;
#ifdef KMVP
  //pass the offset too. GLOBALDATA is different at minivdd level
   pssd->dwDriverReserved2 = offsetof( GLOBALDATA, ovlBltParams);
#endif

   SurfaceData = (FXSURFACEDATA*) pssd->lpDDSurface->lpGbl->dwReserved1;

#ifndef WINNT
   if(!_FF(ovlBltParams.pDebAddr))
   {
     DWORD dwRet;
     if( (dwRet =IOControl(ppdev, GET_DIB_ADDR, (DWORD)ppdev->lpDeFlags, 0 ))
                > 0x20)
            _FF(ovlBltParams.pDebAddr) = (WORD *)dwRet;
   }

   if(_FF(ovlBltParams.wShrinkFlags) & OVL_SHRINK)
   {
      pssd->dwSurfaceOffset =
          GET_HW_ADDR(pssd->lpDDSurface);

      pssd->dwOverlayDestHeight = 0;

   }
   else
   {
      pssd->dwSurfaceOffset =
        ( GET_HW_ADDR(pssd->lpDDSurface) +
        _FF(lastOverlayAddress) -
        _FF(ddVisibleOverlaySurf) );

      pssd->dwOverlayDestHeight = 0xFF;     //none zero
   }

   if( IS_TILED( GET_HW_ADDR(pssd->lpDDSurface)) )
      pssd->dwSurfaceOffset &=~SSTG_IS_TILED;
#else //winnt
    pssd->dwSurfaceOffset = pssd->lpDDSurface->lpGbl->fpVidMem + (ULONG)ppdev->pjLfbBase;
#endif
   pssd->ddRVal = DD_OK;

   return DDHAL_DRIVER_HANDLED;

}// kSyncSurfaceData32


/*----------------------------------------------------------------------
Function name: kSyncVideoportData32

Description:   DDRAW kernel callback SyncVideoPortData

               Allows the HAL to set and massage video port data before
               it is passed to the mini-VDD

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall kSyncVideoportData32(LPDDHAL_SYNCVIDEOPORTDATA psvpd)
{
   DD_ENTRY_SETUP(psvpd->lpDD->lpGbl);

#ifdef FXTRACE
#ifndef WINNT
	Msg(ppdev, DEBUG_VPEENTRY, "SyncVideoportData32" );
#else
	DISPDBG(( DEBUG_VPEENTRY, "SyncVideoportData32" ));
#endif

#endif

	psvpd->ddRVal = DD_OK;

  	return DDHAL_DRIVER_HANDLED;
}//  kSyncVideoportData32


//======================================================================
//Miscellaneous internal functions
//

/*----------------------------------------------------------------------
Function name: H3_VMI_Disable

Description:

Return:        NONE
----------------------------------------------------------------------*/

void H3_VMI_Disable(NT9XDEVICEDATA * ppdev)
{
	DWORD dwReg;
#ifdef WINNT
	KMVTBUFF * lpKMBuff = (KMVTBUFF *)_FF( KMVTBuff);
#endif

   //May need to wait for Video VSync?
	dwReg = GET(ghwIO->vidSerialParallelPort) & ~(H3_VMI_ENABLE|H3_VMI_RESET_MASK);
    SETDW(ghwIO->vidSerialParallelPort, dwReg);

#ifndef WINNT
   //V3TV
   UpdateIMask(ppdev, GET(ghw0->intrCtrl) & ~H3_VMI_INT_ENABLE);
   SETDW(ghw0->intrCtrl, (GET(ghw0->intrCtrl) & ~H3_VMI_INT_ENABLE));
#else
   if (lpKMBuff)
		lpKMBuff->dwIMask = GET(ghw0->intrCtrl) & ~H3_VMI_INT_ENABLE;
#endif

}// H3_VMI_Disable

/*----------------------------------------------------------------------
Function name: H3_VMI_Enable_Func

Description:

Return:        NONE
----------------------------------------------------------------------*/

void H3_VMI_Enable_Func(NT9XDEVICEDATA * ppdev)
{
	DWORD dwReg;
#ifdef WINNT
	KMVTBUFF * lpKMBuff = (KMVTBUFF *)_FF( KMVTBuff);
#endif

	dwReg = GET(ghwIO->vidSerialParallelPort) | H3_VMI_ENABLE;
    SETDW(ghwIO->vidSerialParallelPort, dwReg);

#ifndef WINNT
    //V3TV needed for 95 to enable interrupt so vmi reset can be done
    UpdateIMask(ppdev, GET(ghw0->intrCtrl) | H3_VMI_INT_ENABLE);
    SETDW(ghw0->intrCtrl, (GET(ghw0->intrCtrl) | H3_VMI_INT_ENABLE));
#else
	lpKMBuff->dwIMask = GET(ghw0->intrCtrl) | H3_VMI_INT_ENABLE;

#endif

}// H3_VMI_Enable_Func

/*----------------------------------------------------------------------
Function name: H3_VMI_Reset

Description:

Return:        NONE
----------------------------------------------------------------------*/

void H3_VMI_Reset(NT9XDEVICEDATA * ppdev)
{
	DWORD dwReg;

	dwReg = GET(ghwIO->vidSerialParallelPort) & ~H3_VMI_RESET_MASK;
    SETDW(ghwIO->vidSerialParallelPort, dwReg);
	dwReg |= H3_VMI_RESET_DISABLE;
    SETDW(ghwIO->vidSerialParallelPort, dwReg);
}// H3_VMI_Reset

/*----------------------------------------------------------------------
Function name: H3_VMI_SetDecimation

Description:

Return:        NONE
----------------------------------------------------------------------*/
void H3_VMI_SetDecimation(NT9XDEVICEDATA * ppdev, RECT *vdrptr, RECT *vdsrcptr)
{
  RECT  rVidSrc, rMaxVidSrcAllowed;
  DWORD dwReg;
  DWORD dwVidInWidth,  dwVidSrcWidth, dwVidDstWidth, dwVidSrcRgnWidth, dwVidMaxSrcWidth;
  DWORD dwVidInHeight, dwVidSrcHeight, dwVidDstHeight, dwVidSrcRgnHeight, dwVidMaxSrcHeight;
//#ifndef WINNT  //V3TV
   DWORD dwWDMSrcRight = VidSrcData.rVidSrcRgn.right;
   DWORD dwWDMSrcLeft = VidSrcData.rVidSrcRgn.left;
   DWORD dwWDMvidInWidth, dwVidRealInHt;
   KMVTBUFF * lpKMBuff = (KMVTBUFF *)_FF( KMVTBuff);
//#endif


  rVidSrc = VidSrcData.rVidSrc;   //Overlay surface allocated
  *vdsrcptr = VidSrcData.rVidSrcRgn;
  //Return the max size of overlay source for max available video bandwidth.
  H3_Calculate_Bandwidth(ppdev, &rVidSrc, &rMaxVidSrcAllowed ,FALSE);
  VidSrcData.rMaxVidSrc = rMaxVidSrcAllowed;                                    //Save max allowed overlay size

  VidDstData.rVidDst = *vdrptr;                                                 //Save video destination rect

  dwVidInWidth  = VidInData.rVidIn.right - VidInData.rVidIn.left;               //VideoIn width
  dwVidDstWidth = VidDstData.rVidDst.right - VidDstData.rVidDst.left;           //VideoDst width
  dwVidMaxSrcWidth = VidSrcData.rMaxVidSrc.right - VidSrcData.rMaxVidSrc.left;  //Max video overlay width

  //If max src width is less than VidIn width, scale-down the video source region's left & right coords
  if ( dwVidMaxSrcWidth < dwVidInWidth )
  {
      vdsrcptr->left  = (VidSrcData.rVidSrcRgn.left * dwVidMaxSrcWidth) / dwVidInWidth;
      vdsrcptr->right = (VidSrcData.rVidSrcRgn.right * dwVidMaxSrcWidth) / dwVidInWidth;
  }

  dwVidSrcRgnWidth = VidSrcData.rVidSrcRgn.right - VidSrcData.rVidSrcRgn.left;  //VideoSrc region width

  //If destination is smaller than source region we will decimate else we keep max video src width as overlay width
  if ( dwVidDstWidth < dwVidSrcRgnWidth )
  {
#if 0
#if PC98_DNSCL_FILT //--Do downscale filter effect by making the src region 75% smaller than destination
      dwVidSrcWidth = (((dwVidDstWidth*3) >> 2) * VidInData.dwVidSrcWidth)/dwVidSrcRgnWidth;
#else
      dwVidSrcWidth = (dwVidDstWidth * VidInData.dwVidSrcWidth)/dwVidSrcRgnWidth;
#endif
      vdsrcptr->left  = (VidSrcData.rVidSrcRgn.left  * dwVidSrcWidth)/VidInData.dwVidSrcWidth;
      vdsrcptr->right = (VidSrcData.rVidSrcRgn.right * dwVidSrcWidth)/VidInData.dwVidSrcWidth;
#else
#if PC98_DNSCL_FILT //--Do downscale filter effect by making the src region 75% smaller than destination
      dwVidSrcWidth = (((dwVidDstWidth*3) >> 2) * dwVidMaxSrcWidth)/dwVidSrcRgnWidth;
#else
      dwVidSrcWidth = (dwVidDstWidth * dwVidMaxSrcWidth)/dwVidSrcRgnWidth;
#endif
      vdsrcptr->left  = (VidSrcData.rVidSrcRgn.left  * dwVidSrcWidth)/dwVidMaxSrcWidth;
      vdsrcptr->right = (VidSrcData.rVidSrcRgn.right * dwVidSrcWidth)/dwVidMaxSrcWidth;
#endif
  }
  else
  {
#if  1
    dwVidSrcWidth = dwVidMaxSrcWidth;
#else
    dwVidSrcWidth = VidInData.dwVidSrcWidth;
#endif
  }

//#ifndef WINNT  //V3TV
  if ((rVidSrc.left == 0 ||
#ifndef WINNT
    (ULONG)rVidSrc.right == _FF(hres))
#else
    (ULONG)rVidSrc.right == (ULONG)ppdev->cyScreen)
#endif
	  && dwVidSrcRgnWidth < (DWORD)(rVidSrc.right - rVidSrc.left) - 12)
  {
	  _DD(bScaleWDMinUse) = FALSE;
	  dwWDMSrcLeft  = 0;
	  dwWDMSrcRight = dwVidInWidth;
	  dwWDMvidInWidth = dwVidInWidth;
  }  else  {
	  _DD(bScaleWDMinUse) = TRUE;
	  dwWDMvidInWidth = dwVidSrcRgnWidth;
	  if ( dwVidDstWidth < dwVidSrcRgnWidth )
	  {
		  dwWDMSrcLeft  = (VidSrcData.rVidSrcRgn.left  * dwVidSrcWidth)/dwVidMaxSrcWidth;
		  dwWDMSrcRight = (VidSrcData.rVidSrcRgn.right * dwVidSrcWidth)/dwVidMaxSrcWidth;
		  dwWDMvidInWidth = dwWDMSrcRight - dwWDMSrcLeft;
	  }

  }
//#endif


  dwVidInHeight  = VidInData.rVidIn.bottom - VidInData.rVidIn.top;              //VideoIn height
  dwVidDstHeight = VidDstData.rVidDst.bottom - VidDstData.rVidDst.top;          //VideoDst height
  dwVidMaxSrcHeight = VidSrcData.rMaxVidSrc.bottom - VidSrcData.rMaxVidSrc.top; //Max video overlay height

//#ifndef WINNT //V3TV
  if (_DD(bVideoPortActive))
  {
		dwVidInHeight -= VBIHeight;
		if (lpKMBuff->bVMIPLDinUse)
			dwVidInHeight -= _DD(dwAdjustForEav);
	   dwVidRealInHt = dwVidInHeight;
  }
//#endif

  //For weave deinterlacing, double video in field height
  if ( (VidInData.WeaveDeinterlacing & ~CAN_SIMULATE)== INTERLEAVED_VIDEO )
  {
      dwVidInHeight  <<= 1;                                                     //Video-In Height
      dwVidMaxSrcHeight <<=1;

  }

  //If max src height is less than VidIn width, scale-down the video source region's top & bottom coords
  if ( dwVidMaxSrcHeight < dwVidInHeight )
  {
      vdsrcptr->top = (VidSrcData.rVidSrcRgn.top * dwVidMaxSrcHeight) / dwVidInHeight;
      vdsrcptr->bottom = (VidSrcData.rVidSrcRgn.bottom * dwVidMaxSrcHeight) / dwVidInHeight;
  }

  dwVidSrcRgnHeight = VidSrcData.rVidSrcRgn.bottom - VidSrcData.rVidSrcRgn.top; //VideoSrc region height
//#ifndef WINNT //V3TV
  if (dwVidSrcRgnHeight > VidInData.dwVidSrcHeight)
	  dwVidSrcRgnHeight = VidInData.dwVidSrcHeight;
//#endif

  //If destination is smaller than source we will decimate else we keep max video src height as overlay height
  if ( dwVidDstHeight < dwVidSrcRgnHeight )
  {
#if 0
#if PC98_DNSCL_FILT //--Do downscale filter effect by making the src region 75% smaller than destination
      dwVidSrcHeight = (((dwVidDstHeight*3) >> 2) * dwVidMaxSrcHeight)/dwVidSrcRgnHeight;
#else
      dwVidSrcHeight = (dwVidDstHeight * dwVidMaxSrcHeight)/dwVidSrcRgnHeight;
#endif
      vdsrcptr->top = (VidSrcData.rVidSrcRgn.top  * dwVidSrcHeight)/dwVidMaxSrcHeight;
      vdsrcptr->bottom = (VidSrcData.rVidSrcRgn.bottom * dwVidSrcHeight)/dwVidMaxSrcHeight;
#else
#if PC98_DNSCL_FILT //--Do downscale filter effect by making the src region 75% smaller than destination
      dwVidSrcHeight = (((dwVidDstHeight*3) >> 2) * VidInData.dwVidSrcHeight)/dwVidSrcRgnHeight;
#else
      dwVidSrcHeight = (dwVidDstHeight * VidInData.dwVidSrcHeight)/dwVidSrcRgnHeight;
#endif
      vdsrcptr->top = (VidSrcData.rVidSrcRgn.top  * dwVidSrcHeight)/VidInData.dwVidSrcHeight;
      vdsrcptr->bottom = (VidSrcData.rVidSrcRgn.bottom * dwVidSrcHeight)/VidInData.dwVidSrcHeight;
#endif
  }
  else
  {
#if 0
      dwVidSrcHeight = dwVidMaxSrcHeight;
#else
      dwVidSrcHeight = VidInData.dwVidSrcHeight;
#endif
  }

//#ifndef WINNT //V3TV
  {		// this is needed for kmvt ddlock for still capture
	  if (lpKMBuff)
	  {
		  lpKMBuff->dwDataWidth = dwVidSrcWidth;
		  lpKMBuff->dwDataHeight = dwVidSrcHeight;
	  }
	  if (_DD(fWDMVXDActive) && _DD(bUseWDMScaling) && _DD(bVideoPortActive))
	  {
		   DWORD  dwScaleSize;
		   DWORD  dwScaleStart;

		   dwScaleStart = ((0) << 16) + dwWDMSrcLeft;
		   dwScaleSize = (dwVidRealInHt << 16) + (dwWDMSrcRight);

		   if (dwScaleSize)
		   {	
				if (( (VidInData.WeaveDeinterlacing & ~CAN_SIMULATE) == INTERLEAVED_VIDEO)
					|| (lpKMBuff->dwVidInFormat & H3_VMI_DEINTERLACE_WEAVE)
					|| (lpKMBuff->dwStatus & INTERLEAVE_ON))
				{
					lpKMBuff->bDoublePitch = TRUE;
					dwScaleSize |= 0x80000000;		// let wdm know
				}
				else
					lpKMBuff->bDoublePitch = FALSE;		

				if (_DD(dwSavedSize) != dwScaleSize || _DD(dwSavedStart) != dwScaleStart)
				{
#ifdef WINNT
					IOControl(ppdev, IOCTL_V3TV_SCALE_FUNCTION, dwScaleSize, dwScaleStart);
#else
					IOControl(ppdev, VDD_V3TV_WDMSCALE, dwScaleSize, dwScaleStart);
#endif
				}
				_DD(dwSavedSize) = dwScaleSize;
				_DD(dwSavedStart) = dwScaleStart;
	   			dwReg = GET(ghwIO->vidInFormat);
				if (_DD(bScaleWDMinUse))
					dwReg &= ~(H3_VMI_HDECIMATION_MASK);
				else
					dwReg |= (H3_VMI_HDECIMATION_MASK);
				dwReg |= (H3_VMI_VDECIMATION_MASK);
       			SETDW(ghwIO->vidInFormat, dwReg);	

				if (lpKMBuff)
				{
					lpKMBuff->dwDataWidth = dwWDMvidInWidth;
				}
				if (_DD(bScaleWDMinUse))
				{
					vdsrcptr->left = 0;
					vdsrcptr->right = dwWDMvidInWidth;
					dwVidSrcWidth = dwWDMvidInWidth;
				}

		   }
		}
		if (_DD(bVideoPortActive))
		{
			if (!lpKMBuff->bVBIcropped)		// if !cropped - vbi
			{
				vdsrcptr->top += VBIHeight;
				vdsrcptr->bottom += VBIHeight;
				VidSrcData.rVidSrcRgn.top += VBIHeight;
				VidSrcData.rVidSrcRgn.bottom +=VBIHeight;
			}
			vdsrcptr->top += 1;
			vdsrcptr->left += 2;
			if (_DD(bScaleWDMinUse) && dwVidDstWidth > dwVidSrcWidth - 14)	// zooming
			{
				vdsrcptr->right -= 14;
			}
			else
				vdsrcptr->right -= 4;
			vdsrcptr->bottom -= 2;
			_DD(rWDMSrc) = *vdsrcptr;
			Msg(ppdev, 0, "rWDMsrc (%d, %d) (%d, %d)", vdsrcptr->left, vdsrcptr->top,
				vdsrcptr->right, vdsrcptr->bottom);
		}
  }

//#endif //ifdef v3tv


  //For weave deinterlacing, double video in field height
  if ( (VidInData.WeaveDeinterlacing & ~CAN_SIMULATE) == INTERLEAVED_VIDEO)
      dwVidSrcHeight  <<= 1;

  dwReg = (dwVidInWidth - dwVidSrcWidth) & 0x0fff;    //Put width difference between video in and video src
//#ifndef WINNT //V3TV
  {
	  DWORD val = dwVidSrcWidth * 105 / 100;		// adjust value
	  dwReg = (dwReg << 16) | (val & 0x0fff);   //in high word and put video src width in low word
  }
//#else
//  dwReg = (dwReg << 16) | (dwVidSrcWidth & 0x0fff);   //in high word and put video src width in low word
//#endif
  SETDW(ghwIO->vidInXDecimDeltas, dwReg);
  VidSrcData.dwVidCurWidth = dwVidSrcWidth;           //Save the width of video data field being written into buffer

  dwReg = (dwVidInHeight - dwVidSrcHeight) & 0x0fff; 	//Put height difference between video in and video src
//#ifndef WINNT //V3TV		// do better calc for small size
  {
	  DWORD val = dwVidSrcHeight * 12 / 10;		// adjust value
	  dwReg = (dwReg << 16) | (val & 0x0fff);	//in high word and put video src width in low word
  }
//#else
//  dwReg = (dwReg << 16) | (dwVidSrcHeight & 0x0fff);	//in high word and put video src width in low word
//#endif
  SETDW(ghwIO->vidInYDecimDeltas, dwReg);
  VidSrcData.dwVidCurHeight = dwVidSrcHeight;         //Save the height of video data field being written into buffer

  //Initial Bresenham error term: ev = 2*Dest Height - Source Height
  // eh = 2*Dest Width  - Source Width
  dwReg = (2 * dwVidSrcHeight - dwVidInHeight) & 0x1fff;
  dwReg = (dwReg << 16) | ((2 * dwVidSrcWidth - dwVidInWidth) & 0x1fff);
  SETDW(ghwIO->vidInDecimInitErrs, dwReg);
  VidInData.dwXScale = (dwVidSrcWidth << 16)  / dwVidInWidth;
  VidInData.dwYScale = (dwVidSrcHeight << 16) / dwVidInHeight;

}// H3_VMI_SetDecimation

/*----------------------------------------------------------------------
Function name: H3_InitBandWidth

Description:

Return:        NONE
----------------------------------------------------------------------*/

void H3_InitBandwidth(NT9XDEVICEDATA * ppdev)
{

   H3BandWidth.MClk        = (WORD)_FF(memclk);               //Memory clock from PDEV global data
   H3BandWidth.DClk        = 80;                        //Dotclk 80Mhz - get from global data
   H3BandWidth.VidProcClk  = 80;                        //Video Processor max clk 80 Mhz???
   H3BandWidth.FSBltPerSec = 8;                         //Full screen blt per second - h3.ini
   H3BandWidth.MBusWidth   = H3_MBUS_WIDTH_BYTES;       //Memory bus interface width 16 bytes
   H3BandWidth.MAvgCycle   = 12;                        //Memory average cycle 1.2 - about 83%
   H3BandWidth.PgBreakPlty = 9 * 16;                    //Page break penalty = 9 cycles * 16 bytes
   H3BandWidth.DesktopBPP  = (WORD)GETPRIMARYBYTEDEPTH;       //Desktop bytes per pixel - get from global data
   H3BandWidth.DesktopPPW  = (WORD)(H3BandWidth.MBusWidth)/H3BandWidth.DesktopBPP; //Desktop Pixel per Word
   H3BandWidth.OverlayBPP  = 2;                         //Overlay bytes per pixel
   H3BandWidth.OverlayPPW  = (WORD)(H3BandWidth.MBusWidth)/H3BandWidth.OverlayBPP;
   H3BandWidth.BurstSize   = 16;                        //Burst size 16 bytes

   //Calculate H3 total bandwidth in MB
   H3BandWidth.MBTotal = H3BandWidth.MClk * H3BandWidth.MBusWidth * 10 / H3BandWidth.MAvgCycle;

}// H3_InitBandWidth

/*----------------------------------------------------------------------
Function name: H3_InitializeAll

Description:

Return:        NONE
----------------------------------------------------------------------*/

void H3_InitializeAll(NT9XDEVICEDATA * ppdev)
{
   _DD(fUpdateOverlay) = FALSE; //Set this to FALSE until UpdateOverlay sets it to TRUE

   //Initialize video source, video destination rectangles to video in rectangle
   VidSrcData.rVidSrc.left =   VidSrcData.rVidSrcRgn.left = VidDstData.rVidDst.left = VidInData.rVidIn.left;
   VidSrcData.rVidSrc.top  =   VidSrcData.rVidSrcRgn.top  = VidDstData.rVidDst.top  = VidInData.rVidIn.top;
   VidSrcData.rVidSrc.right=   VidSrcData.rVidSrcRgn.right= VidDstData.rVidDst.right= VidInData.rVidIn.right;
   VidSrcData.rVidSrc.bottom = VidSrcData.rVidSrcRgn.bottom = VidDstData.rVidDst.bottom = VidInData.rVidIn.bottom;

   //Number of buffers
   VidSrcData.wNumbufs = 0;
}// H3_InitializeAll

/*----------------------------------------------------------------------
Function name: H3_Calculate_BandWidth

Description:

Return:        NONE
----------------------------------------------------------------------*/

//Caculate the maximum bandwidth Video can use given the current display resolution and pixel depth.
//3D and 2D performance must be considered.
BOOL H3_Calculate_Bandwidth(NT9XDEVICEDATA * ppdev, RECT *rpVidSrc, RECT *rpMaxVidSrcAllowed ,
    BOOL fDisplay)
{
   DWORD MBDesktopDraws, MBFree, MBVmiWrites;
   DWORD OvlWidth, OvlHeight;
#ifdef WINNT
   DWORD MaxOvlSurface;
   int i, xred, yred;
#endif

#ifndef WINNT
   DWORD XRes = _FF(hres);       		//XRes get from PDEV global data
   DWORD YRes = _FF(vres);       		//YRes get from PDEV global data
#else
   DWORD XRes = ppdev->cxScreen;  		//XRes get from PDEV global data
   DWORD YRes = ppdev->cyScreen;  		//YRes get from PDEV global data
#endif


	H3BandWidth.DesktopBPP = (WORD)GETPRIMARYBYTEDEPTH; //Pixel depth get from PDEV global data
	H3BandWidth.DesktopPPW = H3BandWidth.MBusWidth/H3BandWidth.DesktopBPP;
    H3BandWidth.DClk       = (WORD)((DWORD)_FF(refresh) * XRes * YRes / (DWORD)1000000);
   //Desktop refresh bandwidth requirements
   H3BandWidth.MBDesktop = H3BandWidth.DesktopBPP * H3BandWidth.DClk +
                           (H3BandWidth.PgBreakPlty * H3BandWidth.DClk) /
                           (H3BandWidth.BurstSize * H3BandWidth.DesktopPPW);

   //Desktop drawing bandwidth requirements
   MBDesktopDraws = ((XRes>>5) * (YRes>>5) * H3BandWidth.DesktopBPP * H3BandWidth.FSBltPerSec) >> 10;

   //Desktop total bandwidth requirements
   H3BandWidth.MBDesktop += MBDesktopDraws;

   //Free memory bandwidth left for video
   MBFree = H3BandWidth.MBTotal - H3BandWidth.MBDesktop;

   //Overlay refresh bandwidth requirements assuming overlay source X is equal to overlay dest X and bilinear filter
   H3BandWidth.OverlayPPW  = H3BandWidth.MBusWidth/H3BandWidth.OverlayBPP;
   H3BandWidth.MBOverlay =  H3BandWidth.OverlayBPP * H3BandWidth.DClk +
                                 (H3BandWidth.PgBreakPlty * H3BandWidth.DClk) /
                                 (H3BandWidth.BurstSize * H3BandWidth.OverlayPPW);


   //VMI memory write bandwidth in MB for 64 frames persecond assuming overlay size of VidSrc
   OvlWidth  = rpVidSrc->right - rpVidSrc->left;
   OvlHeight = rpVidSrc->bottom - rpVidSrc->top;
   MBVmiWrites = ((OvlWidth >> 5) * (OvlHeight >> 5) * H3BandWidth.OverlayBPP) >> 4;

   //Overlay total bandwidth requirements
   if(!fDisplay)
   {   
      H3BandWidth.MBOverlay = MBVmiWrites + 3 * H3BandWidth.MBOverlay;
   }
   else
   {
      H3BandWidth.MBOverlay = (MBVmiWrites + H3BandWidth.MBOverlay) * 12 /10;

      MBFree = H3BandWidth.MBDesktop + H3BandWidth.MBOverlay; //borrow it
      if(GET(ghwIO->vidProcCfg) & SST_VIDEO_2X_MODE_EN )
      {
        //2x Mode
        MBFree = 10 * MBFree / 13;
      }

      if( H3BandWidth.MBTotal > MBFree)
        return TRUE;
      else
        return FALSE;
   }


   //Belows breaks 1600x1200 16bpp as 32bpp for v3tv
#ifdef WINNT   //V3TV
   if ( MBFree < H3BandWidth.MBOverlay )
   {
      //Maximum overlay surface allowed
      MaxOvlSurface = OvlWidth * OvlHeight * MBFree / H3BandWidth.MBOverlay;

      //Reduction factor in x and y direction
      xred = yred = 1;

      if ( OvlWidth > OvlHeight )
          xred += OvlWidth/OvlHeight;
      else if ( OvlWidth < OvlHeight )
          yred += OvlHeight/OvlWidth;

      for ( ;; )
      {
          for ( i = xred; i > 0; i-- )
          {
               if ( (OvlWidth * OvlHeight) <= MaxOvlSurface )
                   break;   //Sufficient bandwidth for this OvlWidth and OvlHeight

               if ( OvlWidth > 1 )
                   OvlWidth--;
               else
                   break;
          }
          if ( (OvlWidth * OvlHeight) <= MaxOvlSurface )
               break;   //Sufficient bandwidth for this OvlWidth and OvlHeight

          if ( OvlHeight > (DWORD)yred )
               OvlHeight -= yred;
          else
               break;
      }
   }
#endif

   //Return the max size of overlay source in MaxVidSrcAllowed rect structure
   //for max video bandwidth allowed.
   *rpMaxVidSrcAllowed = *rpVidSrc;
   rpMaxVidSrcAllowed->right  = rpMaxVidSrcAllowed->left + OvlWidth;
   rpMaxVidSrcAllowed->bottom = rpMaxVidSrcAllowed->top  + OvlHeight;
   return TRUE;
}// H3_Calculate_BandWidth
