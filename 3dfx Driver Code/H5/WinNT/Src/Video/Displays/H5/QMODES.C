/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** $Revision: 7$
** $Date: 10/11/00 8:56:52 PM$
** $Log: 
**  7    3dfx      1.4.1.0.1.0 10/11/00 Brent           Forced check in to enforce
**       branching.
**  6    3dfx      1.4.1.0     05/22/00 Dan O'Connel    Major clean up of DFP
**       support code. Restructures DFP code to simplify interfaces.  Also sync
**       source with Win2K driver.
**  5    3dfx      1.4         10/18/99 Dan O'Connel    Corrections to initial DFP
**       support checkin.
**  4    3dfx      1.3         10/11/99 Dan O'Connel    Initial port of Digital
**       Flat Panel support from Win9x to WinNT4.
**  3    3dfx      1.2         10/04/99 Dan O'Connel    Change name of tvout.h
**       include file to fxtvout.h to avoid conflict with file of same name in
**       Win2K DDK.
**  2    3dfx      1.1         09/23/99 Dan O'Connel    Restructure and cleanup
**       tvout code, and base code on top of Ryan Bissell's Modularized Reference
**       Implementation of I2C.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 8     1/05/99 7:28p Bob
** Fixed a problem in the BIOS version reporting.
** 
** 7     1/05/99 5:57p Bob
** BIOS version reporting to Control Panel applet.
** 
** Shared qmodes.h with Win9x and the rest of the universe.
** 
** 6     12/07/98 7:57p Bob
** PRS 2495
** Gamma for Glide and Desktop
** Desktop Gamma updates in control panel when rollers are moved.
** Glide Gamma appears when quake2 or other glide apps are invoked.
** 
** 5     12/01/98 3:10p Bob
** PRS 2495
** Gamma support for Glide
** 
** Added hwcext_gamma_download.
** 
** 4     10/26/98 3:17p Bob
** Carried forward PRS 2951 fix from GLOP tree.
** 
** 3     8/26/98 3:42a Bob
** Gamma table support.
** 
** PRS 2086, 2305
** 
** 2     8/21/98 1:00p Russ
** added QUERYNUMMODES and QUERYMODES processing
**
** 1     8/20/98 9:05p Russ
**
*/

/***************************************************************************
* I N C L U D E S
****************************************************************************/

#include "precomp.h"
#include "qmodes.h"
#include "tv.h"
#include "fxtvout.h"
#include "dfpctrl.h"
#include "dfpapi.h"

extern BOOL HWSetPalette ( PDEV *, int, int, PVIDEO_CLUTDATA, GAMMA_STATE );

/*********************************************************************************
**
**  int QueryMode(LPQIN lpQIN, LPVOID lpOutput)
**   This Control Subfunction is used to handle all of the queries defined in
**   qmodes.h
**
**********************************************************************************/

int
QueryMode ( PDEV *ppdev, LPQIN lpQIN, LPVOID lpOutput )
{
  LPQVERSION              lpQVersion;
  LPQNUMMODE              lpQNumMode;
  LPQMODE                 lpQMode;
  LPQGETGAMMA             lpQGetGamma;
  LPQSETGAMMA             lpQSetGamma;
  LPQGETBIOSVERSION		  lpQGetBiosVersion;
  int                     i;
  int                     nReturn = TDFXERR;
  ULONG                   ulTemp;
  VIDEO_NUM_MODES         modes;
  ULONG                   cModes;
  PVIDEO_MODE_INFORMATION pVideoBuffer;
  PVIDEO_MODE_INFORMATION pVideoTemp;
  ULONG                   cbModeSize;
  TDFX_SET_VALUE_INFO	  setValueInfo;
  TDFX_QUERY_VALUE_INFO	  queryValueInfo;
  PUCHAR				  pulTmp;
  UCHAR					  ucTmp[MAX_BIOS_VERSION_STRING+1];
  ULONG					  monitorCmd;
  ULONG					  monitorStatus;

  switch (lpQIN->dwSubFunc)
  {
    case QUERYVERSION:
      lpQVersion = (LPQVERSION)lpOutput;
      lpQVersion->dwMajor = QUERYMODE_MAJOR;
      lpQVersion->dwMinor = QUERYMODE_MINOR;
      nReturn = TDFXACK;
      break;

    case QUERYNUMMODES:
      lpQNumMode = (LPQNUMMODE)lpOutput;

      // Get the number of modes supported by the mini-port
      if (! EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES,
                               NULL,
                               0,
                               &modes,
                               sizeof(VIDEO_NUM_MODES),
                               &ulTemp))
      {
        lpQNumMode->dwNum = modes.NumModes;
        nReturn = TDFXACK;
      }
      break;

    case QUERYMODES:
      lpQMode = (LPQMODE)lpOutput;
      cModes = getAvailableModes(ppdev->hDriver, &pVideoBuffer, &cbModeSize);
      if (0 != cModes)
      {
        pVideoTemp = pVideoBuffer;
        while (cModes--)
        {
          if (pVideoTemp->Length != 0)
          {
            lpQMode->dwX     = pVideoTemp->VisScreenWidth;
            lpQMode->dwY     = pVideoTemp->VisScreenHeight;
            lpQMode->dwBpp   = pVideoTemp->BitsPerPlane *
                               pVideoTemp->NumberOfPlanes;
            lpQMode->dwRef   = pVideoTemp->Frequency;
            lpQMode->dwValid = QUERY_MODE_VALID;
          }
          else
          {
            // mark as an invalid mode
            memset(lpQMode, 0, sizeof(QMODE));
            //lpQMode->dwValid = 0L;
          }

          lpQMode++;
          pVideoTemp = (PVIDEO_MODE_INFORMATION)(((PUCHAR)pVideoTemp) + cbModeSize);
        }
        ENGFREEMEM(pVideoBuffer);
        nReturn = TDFXACK;
      }

      break;

    case QUERYGETGAMMA:
      lpQGetGamma = (LPQGETGAMMA)lpOutput;
      lpQGetGamma->dwRed   = ppdev->dwRedGamma;
      lpQGetGamma->dwGreen = ppdev->dwGreenGamma;
      lpQGetGamma->dwBlue  = ppdev->dwBlueGamma;

	  //
	  // Collect the gamma table from the miniport
	  //
	  if (!EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_REG_RETRIEVE_GAMMA_LUT,
                               NULL,
                               0,
                               ppdev->GammaTable,
                               256 * sizeof(ULONG),
                               &ulTemp))
	  {
	     DISPDBG((0, "IOCTL_VIDEO_REG_RETRIEVE_GAMMA_LUT okay"));
	  }

      memcpy(lpQGetGamma->GammaTable, ppdev->GammaTable, (256 * sizeof(ULONG)));

      nReturn = TDFXACK;
      break;

    case QUERYSETGAMMA:
      lpQSetGamma = (LPQSETGAMMA)lpQIN;
      ppdev->dwRedGamma   = lpQSetGamma->dwRed;
      ppdev->dwGreenGamma = lpQSetGamma->dwGreen;
      ppdev->dwBlueGamma  = lpQSetGamma->dwBlue;

      for (i=0; i<256; i++)
        ppdev->GammaTable[i] = lpQSetGamma->GammaTable[i] & 0x00ffffff;

	  // 
	  // Save the gamma table in the registry
	  //
	  if (!EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_REG_SAVE_GAMMA_LUT,
                               ppdev->GammaTable,
                               256 * sizeof(ULONG),
                               NULL,
                               0,
                               &ulTemp))
	  {
	     DISPDBG((0, "IOCTL_VIDEO_REG_SAVE_GAMMA_LUT okay"));
	  }

      HWSetPalette(ppdev, 0, 256, (PVIDEO_CLUTDATA)ppdev->pPal, GAMMA_DESKTOP);

      nReturn = TDFXACK;
      break;

	case QUERYGETGLIDEGAMMA:
		lpQGetGamma = (LPQGETGAMMA)lpOutput;
		lpQGetGamma->dwRed   = ppdev->dwRedGamma;
		lpQGetGamma->dwGreen = ppdev->dwGreenGamma;
		lpQGetGamma->dwBlue  = ppdev->dwBlueGamma;

		//
		// Collect the gamma table from the miniport
		//
		// initialize queryValueInfo for binary data
		queryValueInfo.Type = REG_BINARY;
		queryValueInfo.DataLength = TDFX_MAX_DATA_LENGTH;

		//
		// Collect the gamma table from the miniport
		//
		if (!EngDeviceIoControl(ppdev->hDriver,
            IOCTL_3DFX_QUERY_REGISTRY_VALUE,
			(PVOID)"GlideGammaTable",
			strlen("GlideGammaTable") + 1,
			&queryValueInfo,
            256 * sizeof(ULONG),
            &ulTemp))
		{
			DISPDBG((0, "IOCTL_3DFX_QUERY_REGISTRY_VALUE okay"));
		}

		for (i=0; i<256; i++)
			lpQGetGamma->GammaTable[i] = ppdev->GlideGammaTable[i];

		nReturn = TDFXACK;
		break;

	case QUERYSETGLIDEGAMMA:
		lpQSetGamma = (LPQSETGAMMA)lpQIN;
		ppdev->dwRedGamma   = lpQSetGamma->dwRed;
		ppdev->dwGreenGamma = lpQSetGamma->dwGreen;
		ppdev->dwBlueGamma  = lpQSetGamma->dwBlue;

		for (i=0; i<256; i++)
			ppdev->GlideGammaTable[i] = lpQSetGamma->GammaTable[i] & 0x00ffffff;

		// initialize setValueInfo for binary data
		setValueInfo.DataLength = (256 * sizeof(ULONG));
		strcpy(setValueInfo.ValueName, "GlideGammaTable");
		setValueInfo.ValueNameLength = strlen("GlideGammaTable") + 1;
		setValueInfo.Type = REG_BINARY;
		memcpy(setValueInfo.Data, ppdev->GlideGammaTable, (256 * sizeof(ULONG)));

		// 
		// Save the gamma table in the registry
		//
		if (!EngDeviceIoControl(ppdev->hDriver,
            IOCTL_3DFX_SET_REGISTRY_VALUE,
            &setValueInfo,
			sizeof(setValueInfo),
			NULL,
			0,
            &ulTemp))
		{
			DISPDBG((0, "IOCTL_3DFX_SET_REGISTRY_VALUE okay"));
		}

		// Don't do the update here, because this is just when the
		// Control Panel issues the new settings for the GlideGammaTable
		//
		//HWSetPalette(ppdev, 0, 256, (PVIDEO_CLUTDATA)ppdev->pPal, GAMMA_GLIDE);

		nReturn = TDFXACK;
		break;

	case QUERYGETBIOSVERSION:
		DISPDBG((1, "QUERYGETBIOSVERSION"));

		lpQGetBiosVersion = (LPQGETBIOSVERSION) lpOutput;

		//
		// Collect the BIOS version from the miniport
		//
		if (!EngDeviceIoControl(ppdev->hDriver,
            IOCTL_3DFX_GET_BIOS_VERSION,
			NULL,
			0,
            &ucTmp,
			MAX_BIOS_VERSION_STRING,
            &ulTemp))
		{
			DISPDBG((1, "IOCTL_3DFX_GET_BIOS_VERSION okay"));
			DISPDBG((1, "Version String = \"%s\"", ucTmp));
			DISPDBG((1, "ulTemp         = %d", ulTemp));

			if (ucTmp[0] != '\0')
				memcpy((PUCHAR)lpQGetBiosVersion->bBIOSVersion, ucTmp, MAX_BIOS_VERSION_STRING);
			else
				lpQGetBiosVersion->bBIOSVersion[0] = '\0';
		}
		nReturn = TDFXACK;
		break;

      //TV-Out functions----------------------------------------------------------//
      
	case QUERYTVAVAIL:
                TVOutStatus( ppdev, lpQIN, (LPQTVSTATUS)lpOutput );
                nReturn = TDFXACK;
                break;
	case QUERYTVSENSE:
                TVOutConStatus( ppdev, lpQIN, (LPTVCONSTATUS)lpOutput );
                nReturn = TDFXACK;
                break;
	case QUERYGETPICCAP:
                TVOutGetPicCap( ppdev, (LPQIND)lpQIN, (LPTVCAPDATA)lpOutput );
                nReturn = TDFXACK;
                break;
#ifdef notused  //3dfx Tools uses QUERYGETPICCAP for flicker filter
	case QUERYGETFILTERCAP:
                TVOutGetFilterCap( ppdev, (LPQIND)lpQIN, (LPTVCURCAP)lpOutput );
                nReturn = TDFXACK;
                break;
#endif
	case QUERYGETPOSCAP:
                TVOutGetPosCap( ppdev, lpQIN, (LPTVPOSCAP)lpOutput );
                nReturn = TDFXACK;
                break;
	case QUERYGETSIZECAP:
                TVOutGetSizeCap( ppdev, lpQIN, (LPTVSIZECAP)lpOutput );
                nReturn = TDFXACK;
                break;
	case QUERYGETSPECIALCAP:
                TVOutGetSpecialCap( ppdev, (LPQIND)lpQIN, (LPVOID)lpOutput );
                nReturn = TDFXACK;
                break;
	case QUERYGETSTANDARD:
                TVOutGetStandard( ppdev, lpQIN, (LPTVGETSTANDARD)lpOutput );
                nReturn = TDFXACK;
                break;
	case QUERYGETOVERRIDE:
                TVOutGetOverride( ppdev, lpQIN, (PTVGETOVERRIDE)lpOutput );
                nReturn = TDFXACK;
                break;
	case QUERYGETPICCONTROL:
                TVOutGetPicControl( ppdev, (LPTVCURCAP)lpQIN, (LPTVCURCAP)lpOutput );
                nReturn = TDFXACK;
                break;
#ifdef notused  //3dfx Tools uses QUERYGETPICCONTROL for flicker filter
	case QUERYGETFILTERCONTROL:
                TVOutGetFilterControl( ppdev, (LPQIND)lpQIN, (LPTVCURCAP)lpOutput );
                nReturn = TDFXACK;
                break;
#endif
	case QUERYGETPOSCONTROL:
                TVOutGetPosControl( ppdev, lpQIN, (LPTVCURPOS)lpOutput );
                nReturn = TDFXACK;
                break;
	case QUERYGETSIZECONTROL:
                TVOutGetSizeControl( ppdev, lpQIN, (LPTVCURSIZE)lpOutput );
                nReturn = TDFXACK;
                break;
	case QUERYGETSPECIAL:
                TVOutGetSpecialCtl( ppdev, (LPQIND)lpQIN, (LPVOID)lpOutput );
                nReturn = TDFXACK;
                break;
	case QUERYGETCONSTATUS:
                TVOutGetConStatus( ppdev, lpQIN, (LPTVCON)lpOutput );
                nReturn = TDFXACK;
                break;
	case QUERYSETSTANDARD:
// Warning :  Ignore setting the standard to zero, to be compatible with Win9x kludge.
		if (((LPTVSETSTANDARD)lpQIN)->dwStandard != 0)
                    TVOutSetStandard( ppdev, (LPTVSETSTANDARD)lpQIN );
                nReturn = TDFXACK;
                break;
	case QUERYSETOVERRIDE:
                TVOutSetOverride( ppdev, (PTVSETOVERRIDE)lpQIN );
                nReturn = TDFXACK;
                break;
	case QUERYSETPICCONTROL:
                TVOutSetPicControl( ppdev, (LPTVSETCAP)lpQIN );
                nReturn = TDFXACK;
                break;
#ifdef notused  //3dfx Tools uses QUERYSETPICCONTROL for flicker filter
	case QUERYSETFILTERCONTROL:
                TVOutSetFilterControl( ppdev, (LPTVSETCAP)lpQIN );
                nReturn = TDFXACK;
                break;
#endif
	case QUERYSETPOSCONTROL:
                TVOutSetPosControl( ppdev, (LPTVSETPOS)lpQIN );
                nReturn = TDFXACK;
                break;
	case QUERYSETSIZECONTROL:
                TVOutSetSizeControl( ppdev, (LPTVSETSIZE)lpQIN );
                nReturn = TDFXACK;
                break;
	case QUERYSSETSPECIAL:
                TVOutSetSpecial( ppdev, (LPTVSETSPECIAL)lpQIN );
                nReturn = TDFXACK;
                break;
	case QUERYSETCONSTATUS:
                TVOutSetConStatus( ppdev, (LPTVSETCONNECTOR)lpQIN );
                nReturn = TDFXACK;
                break;
	case QUERYCOMMITREG:
                TVOutCommitReg( ppdev, lpQIN );
                nReturn = TDFXACK;
                break;
	case QUERYREFRESH:
                TVOutRefreshMem( ppdev, lpQIN );
                nReturn = TDFXACK;
                break;
	case QUERYDISABLETV:
                TVOutDisable( ppdev );
                nReturn = TDFXACK;
                break;
	case QUERYENABLETV:
                TVOutEnable( ppdev, lpQIN );
                nReturn = TDFXACK;
         break;

      //End TV-Out section----------------------------------------------------------//

        case QUERY_LCDCTRL:
        {
#if DBG
            _asm int 3;  // this interface is obsolete and should not be used.
#endif
            nReturn = TDFXACK;
            break;
        }

	 case QUERY_ANALOG_MONITOR:
  	        monitorCmd = 0;  // just check status
		if (((QGETSET_MONITOR_CTL *)lpQIN)->monitorControl & ENABLE_MONITOR)
  	            monitorCmd = 1;   // enable the monitor
		else if (((QGETSET_MONITOR_CTL *)lpQIN)->monitorControl & DISABLE_MONITOR)
  	            monitorCmd = 2;   // disable the monitor

                memcpy( lpOutput, lpQIN, sizeof(QGETSET_MONITOR_CTL) );

                EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_QUERY_MONITOR,
                           &monitorCmd,
	                   sizeof(ULONG),
                           lpOutput,
	                   sizeof(QGETSET_MONITOR_CTL),
                           &ulTemp);

                nReturn = TDFXACK;
		break;


    default:
		break;
  }

  return nReturn;
}

