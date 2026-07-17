/* $header: /devel/h3/win95/dx/dd16/tvout.c 21    3/18/99 10:34a stuartb $ */
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
** File name:   tvout.c
**
** Description: TV out functions.
**
** $Revision: 12$
** $Date: 10/24/00 7:24:29 AM$
**
** $History: tvout.c $
** 
** *****************  Version 1  *****************
** User: Doconnell    Date: 9/03/99    Time: 10:57a
** Created in $/devel/h5/WinNT/Src/Video/Displays/h5
** Add 3dfx Tools interfaces and TVout support.
** 
** *****************  Version 8  *****************
** User: Doconnell    Date: 8/27/99    Time: 5:11p
** Updated in $/Releases/Voodoo3/V3_RT4/3dfx/devel/H3/WINNT/SRC/Video/Displays/Voodoo3
** PRS 8199 Work with Edge Tools to correctly handle concurrent output to
** Monitor and CRT and various OEM specific options (Gateway).  Also port
** some fixes from Win9x to WinNT4 that have to do initializing and
** handling the NVRAM cache.
** 
** *****************  Version 7  *****************
** User: Doconnell    Date: 8/13/99    Time: 4:36p
** Updated in $/Releases/Voodoo3/V3_RT31/3dfx/devel/H3/WINNT/SRC/Video/Displays/Voodoo3
** PRS 7926 - Port Multiple TVOut corrections from Win9x to WinNT.
** 
** *****************  Version 5  *****************
** User: Stb_doconnel Date: 6/04/99    Time: 12:38p
** Updated in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/displays/voodoo3
** PRS 6386 Misc. cancel/apply fixes for 3dfx Tools interface
** 
** *****************  Version 4  *****************
** User: Stb_doconnel Date: 5/24/99    Time: 3:42p
** Updated in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/displays/voodoo3
** PRS 6227 Add connector override capability.
** 
** *****************  Version 3  *****************
** User: Stb_doconnel Date: 5/18/99    Time: 2:22p
** Updated in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/displays/voodoo3
** Correction to remove compiler warnings
** 
** *****************  Version 2  *****************
** User: Stb_doconnel Date: 5/13/99    Time: 12:37p
** Updated in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/displays/voodoo3
** Correct Tv Out Active and Monitor Active status returned to Edge Tools
** 
** *****************  Version 1  *****************
** User: Stb_doconnel Date: 5/12/99    Time: 9:58a
** Created in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/displays/voodoo3
** 
** *****************  Version 21  *****************
** User: Stuartb      Date: 3/18/99    Time: 10:34a
** Updated in $/devel/h3/Win95/dx/dd16
** Fix bug, causing LCD to go blank when booted to LCD.  Reported by QA,
** no PRS.
** 
** *****************  Version 20  *****************
** User: Stuartb      Date: 3/02/99    Time: 4:20p
** Updated in $/devel/h3/Win95/dx/dd16
** Removed obsolete TV_STANDARD_XXX defines.
** 
** *****************  Version 19  *****************
** User: Stuartb      Date: 2/25/99    Time: 9:58a
** Updated in $/devel/h3/Win95/dx/dd16
** In all cases of tvstd setting, remember to update BIOS' knowlege of
** tvstd.
** 
** *****************  Version 18  *****************
** User: Stuartb      Date: 2/18/99    Time: 11:41a
** Updated in $/devel/h3/Win95/dx/dd16
** Return tv dwAvailableStandards that includes new PAL modes.
** Handle importing of tv boot standard from BIOS.
** 
** *****************  Version 17  *****************
** User: Stuartb      Date: 2/13/99    Time: 6:48p
** Updated in $/devel/h3/Win95/dx/dd16
** First cut at PAL_M, PAL_N, PAL_NC tvout support.
** 
** 
** *****************  Version 16  *****************
** User: Stuartb      Date: 2/08/99    Time: 9:00a
** Updated in $/devel/h3/Win95/dx/dd16
** Changes to support simultaneous VMI-TV/LCD for h4.
** 
** *****************  Version 15  *****************
** User: Stuartb      Date: 1/25/99    Time: 11:08a
** Updated in $/devel/h3/Win95/dx/dd16
** Fixed some oops'.  Removed evil broadcastMonitorChange calls().
** 
** *****************  Version 14  *****************
** User: Cwilcox      Date: 1/25/99    Time: 11:40a
** Updated in $/devel/h3/Win95/dx/dd16
** Minor modifications to remove compiler warnings.
** 
** *****************  Version 13  *****************
** User: Stuartb      Date: 1/14/99    Time: 3:46p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed flat panel & tvout enable/disable philosophy.  No longer
** requires booting to that device to enable.
** 
** *****************  Version 12  *****************
** User: Stuartb      Date: 1/06/99    Time: 5:09p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed misleading define H3_VDD_GET_BIOS_SCRATCH_REG to something
** meaningful.
** 
** *****************  Version 11  *****************
** User: Bob          Date: 1/05/99    Time: 5:14p
** Updated in $/devel/h3/Win95/dx/dd16
** Altered nesting of header files so that tv.h is not inside qmodes.h.
** 
** This is a sharing issue with NT.
** 
** *****************  Version 10  *****************
** User: Michael      Date: 12/30/98   Time: 2:29p
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB unified header.
**
*/

/***************************************************************************
* I N C L U D E S
****************************************************************************/

#include "precomp.h"
#include "tv.h"
#include "qmodes.h"
#include "fxtvout.h"
#include "funcapi.h"
#include "edgedefs.h"
#ifdef STB_DFP_ENABLED
#include "dfpapi.h"
#endif



void TVOutCancelSettings( PDEV *ppdev, void * pInData, void * pOutData)
{
    ULONG ulTemp;
    TVPACKET requestPkt;

    if (ppdev->ulDevicesThatHaveTheirStatesSaved & STB_FUNCTIONACTIVE0_TV)
    {
        if (ppdev->ulSavedStatesOfSavedDevices & STB_FUNCTIONACTIVE0_TV)
        {
            ppdev->dwTvoActive = 0;   // set to inactive
            requestPkt.tvPacketFunc = tvDisable;
        }
        else
        {
            requestPkt.tvPacketFunc = tvEnable;
            ppdev->dwTvoActive = 1;   // on
        }

        EngDeviceIoControl(ppdev->hDriver,
                IOCTL_TV_TRANSACTION,
                &requestPkt,
                sizeof(requestPkt.tvPacketFunc),
                NULL,
                0,
                &ulTemp);
    }
}


DWORD GetTvStatus( PDEV *ppdev )
{
   ULONG ulTemp;
   DWORD TvStatus;
   TVPACKET requestPkt;
   
   //
   // Collect the TV status from the miniport
   //
   requestPkt.tvPacketFunc = tvGetStatus;
   if (!EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_TV_TRANSACTION,
                           &requestPkt,
                           sizeof(requestPkt.tvPacketFunc),
                           &TvStatus,
                           sizeof(TvStatus),
                           &ulTemp))
      {
      if (0x80 == (TvStatus & 0x80))
        ppdev->dwTvoActive = 1;   // on
      else
        ppdev->dwTvoActive = 0;   // off
      return(TvStatus);
      }
   else
      return(0xffffffff);

}

/*----------------------------------------------------------------------
Function name:  TVOutStatus

Description:    Get encoder type, set capabilities flags.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutStatus( PDEV *ppdev, LPQIN lpQIN, LPQTVSTATUS lpOutput )
{
   ULONG ulTemp;
   DWORD TvStatus;
   TVPACKET requestPkt;
   
   lpOutput->dwEncoder = 0;                              //Encoder is not present or busy

   //
   // Collect the TV status from the miniport
   //
   requestPkt.tvPacketFunc = tvGetStatus;
   if (!EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_TV_TRANSACTION,
                           &requestPkt,
                           sizeof(requestPkt.tvPacketFunc),
                           &TvStatus,
                           sizeof(TvStatus),
                           &ulTemp))
   {
      DISPDBG((1, "GET_TVSTATUS okay"));

      if (0x80 == (TvStatus & 0x80))
        ppdev->dwTvoActive = 1;   // on
      else
        ppdev->dwTvoActive = 0;   // off

      if ( !(TvStatus & 0x00000001) )
      {
          switch ( (TvStatus & 0x00e00000) >> 21 )
          {
             case 0:
                strcpy(lpOutput->szName, "BT868");           //Name of encoder
                lpOutput->dwEncoder = TV_ENCODER_PRESENT;    //Encoder is on board and functioning
                break;
             case 1:
                strcpy(lpOutput->szName, "BT869");           //Name of encoder
                lpOutput->dwEncoder = TV_ENCODER_PRESENT;    //Encoder is on board and functioning
                lpOutput->dwSpecial = TV_MACROVISION;        //Special feature
                break;
             default:
                strcpy(lpOutput->szName, "None");            //Name of encoder
                break;
          }
      }

      lpOutput->dwNumConnectors = 2; 
      lpOutput->TVCon[0].dwType = TV_TYPE_SVIDEO;
      lpOutput->TVCon[0].dwStatus = (TvStatus & 0x80) ? TV_CONNECTOR_ENABLED : 0
                                | (TvStatus & 0x6000) ? TV_PRESENT : 0;
      lpOutput->TVCon[1].dwType = TV_TYPE_COMPOSITE;
      lpOutput->TVCon[1].dwStatus = (TvStatus & 0x80) ? TV_CONNECTOR_ENABLED : 0
                                | (TvStatus & 0x8000) ? TV_PRESENT : 0;
   }

   lpOutput->dwStandard =  VP_TV_STANDARD_NTSC_M |     //Standards supported
                           VP_TV_STANDARD_PAL_B  |
                           VP_TV_STANDARD_PAL_D  |
                           VP_TV_STANDARD_PAL_H  |
                           VP_TV_STANDARD_PAL_I  |
                           VP_TV_STANDARD_PAL_M  |
                           VP_TV_STANDARD_PAL_N  |
                           VP_TV_STANDARD_PAL_G  |
                           VP_TV_STANDARD_PAL_NC |
                          0;
                          

   lpOutput->dwPicControl = TV_BRIGHTNESS |           //Picture control caps
                            //TV_CONTRAST |
                            //TV_GAMMA |
                            //TV_HUE |
                            TV_SATURATION |
                            //TV_SHARPNESS |
                            0;
                            
   
   lpOutput->dwFilterControl = TV_FLICKER |           //Filter control caps
                               //TV_CHROMA |
                               //TV_LUMA |
                               0;
   
   lpOutput->dwPosControl = TV_HORIZONTAL |           //Position control 
                            TV_VERTICAL |
                            0; 
   
   lpOutput->dwSizeControl = //TV_UNDERSCAN |         //Size control
                             TV_OVERSCAN |
                             //TV_ADJUST_UNDERSCAN |
                             //TV_ADJUST_OVERSCAN |
                             0;
   
   lpOutput->dwSpecial = //TV_CLOSED_CAPTION |        //Special feature
                         0;

   lpOutput->dwNumSimultaneous = 1;                   //Only one output at a time

}


/*----------------------------------------------------------------------
Function name:  TVOutConStatus

Description:    Get status of the TV connector.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutConStatus( PDEV *ppdev, LPQIN lpQIN, LPTVCONSTATUS lpOutput )
{
   ULONG ulTemp;
   DWORD TvStatus;
   int iConnector;
   TVPACKET requestPkt;
   
   //
   // Collect the TV status from the miniport
   //
   requestPkt.tvPacketFunc = tvGetStatus;
   if (!EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_TV_TRANSACTION,
                           &requestPkt,
                           sizeof(requestPkt.tvPacketFunc),
                           &TvStatus,
                           sizeof(TvStatus),
                           &ulTemp))
   {
      DISPDBG((1, "GET_TVSTATUS okay"));
   }

   iConnector = 0;               
   
   if ( TvStatus & 0x2000 )                        //Composite connector
   {                                               //Connection type and status
      lpOutput->TVCon[iConnector].dwType = TV_TYPE_SVIDEO;
      lpOutput->TVCon[iConnector].dwStatus = TV_CONNECTOR_ENABLED | TV_PRESENT;
      iConnector++;                                   
   }
   if ( TvStatus & 0x4000 )                        //S-Video connector
   {                                               //Connection type and status
      lpOutput->TVCon[iConnector].dwType = TV_TYPE_SVIDEO;
      lpOutput->TVCon[iConnector].dwStatus = TV_CONNECTOR_ENABLED | TV_PRESENT;
      iConnector++;                    
   }
   if ( TvStatus & 0x8000 )                        //Composite connector?
   {                                               //Connection type and status
      lpOutput->TVCon[iConnector].dwType = TV_TYPE_COMPOSITE;  
      lpOutput->TVCon[iConnector].dwStatus = TV_CONNECTOR_ENABLED | TV_PRESENT;
      iConnector++;                    
   }
   lpOutput->dwNumConnectors = iConnector; 
}


/*----------------------------------------------------------------------
Function name:  TVOutGetPicCap

Description:    Get TV picture capabilities.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetPicCap( PDEV *ppdev, LPQIND lpQIN, LPTVCAPDATA lpOutput )
{
  TVPACKET requestPkt;
  ULONG ulTemp;

//    if ( ppdev->dwVmiTv != H3VMI_VIDEO_IN )
    {
        ppdev->dwVmiTv = H3VMI_TV_OUT;
        lpOutput->dwCap = lpQIN->dwCap;
        //
        // Collect the Pic Cap from the miniport
        //
        requestPkt.tvPacketFunc = tvGetPicCaps;
        if (!EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_TV_TRANSACTION,
                                &requestPkt,
                                sizeof(requestPkt.tvPacketFunc),
                                lpOutput,
                                sizeof(TVCAPDATA),
                                &ulTemp))
        {
            DISPDBG((1, "GET_PIC_CAPS okay"));
        }
    }
}


#ifdef notused  //3dfx Tools uses QUERYGETPICCAP for flicker filter
/*----------------------------------------------------------------------
Function name:  TVOutGetFilterCap

Description:    Get TV filter capabilities.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetFilterCap( PDEV *ppdev, LPQIND lpQIN, LPTVCURCAP lpOutput )
{
    ULONG ulTemp;
    TVPACKET requestPkt;
//  if ( ppdev->dwVmiTv != H3VMI_VIDEO_IN )
    {
        ppdev->dwVmiTv = H3VMI_TV_OUT;
        lpOutput->dwCap = lpQIN->dwCap;
        //
        // Collect the Filter Cap from the miniport
        //
        requestPkt.tvPacketFunc = tvGetFilterCaps;
        if (!EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_TV_TRANSACTION,
                                &requestPkt,
                                sizeof(requestPkt.tvPacketFunc),
                                lpOutput,
                                sizeof(TVCURCAP),
                                &ulTemp))
        {
            DISPDBG((1, "GET_FILTER_CAPS okay"));
        }
    }
}
#endif //def notused

/*----------------------------------------------------------------------
Function name:  TVOutGetPosCap

Description:    Get TV Position capabilities.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetPosCap( PDEV *ppdev, LPQIN lpQIN, LPTVPOSCAP lpOutput )
{
  ULONG ulTemp;
  TVPACKET requestPkt;
//  if ( ppdev->dwVmiTv != H3VMI_VIDEO_IN )
    {
        ppdev->dwVmiTv = H3VMI_TV_OUT;
        //
        // Collect the Position Cap from the miniport
        //
        requestPkt.tvPacketFunc = tvGetPositionCaps;
        if (!EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_TV_TRANSACTION,
                                &requestPkt,
                                sizeof(requestPkt.tvPacketFunc),
                                lpOutput,
                                sizeof(TVPOSCAP),
                                &ulTemp))
        {
            DISPDBG((1, "GET_POS_CAPS okay"));
        }
    }
}


/*----------------------------------------------------------------------
Function name:  TVOutGetSizeCap

Description:    Get TV Size capabilities.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetSizeCap( PDEV *ppdev, LPQIN lpQIN, LPTVSIZECAP lpOutput )
{
  ULONG ulTemp;
  TVPACKET requestPkt;
//  if ( ppdev->dwVmiTv != H3VMI_VIDEO_IN )
    {
        ppdev->dwVmiTv = H3VMI_TV_OUT;
        //
        // Collect the Size Caps from the miniport
        //
        requestPkt.tvPacketFunc = tvGetSizeCaps;
        if (!EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_TV_TRANSACTION,
                                &requestPkt,
                                sizeof(requestPkt.tvPacketFunc),
                                lpOutput,
                                sizeof(TVSIZECAP),
                                &ulTemp))
        {
            DISPDBG((1, "GET_SIZE_CAPS okay"));
        }
    }
}


/*----------------------------------------------------------------------
Function name:  TVOutGetSpecialCap

Description:    Get TV special capabilities.

Information:    Function is currently empty.
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetSpecialCap( PDEV *ppdev, LPQIND lpQIN, LPVOID lpOutput )
{
}


/*----------------------------------------------------------------------
Function name:  TVOutGetStandard

Description:    Get TV standard capabilities.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetStandard( PDEV *ppdev, LPQIN lpQIN, LPTVGETSTANDARD lpOutput )
{
  ULONG ulTemp;
  TVPACKET requestPkt;
//  if ( ppdev->dwVmiTv != H3VMI_VIDEO_IN )
    {
        ppdev->dwVmiTv = H3VMI_TV_OUT;
        requestPkt.tvPacketFunc = tvGetStandard;
        if (!EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_TV_TRANSACTION,
                                &requestPkt,
                                sizeof(requestPkt.tvPacketFunc),
                                lpOutput,
                                sizeof(TVGETSTANDARD),
                                &ulTemp))
        {
            DISPDBG((1, "GET_STANDARD okay"));
        }
    }
}


/*----------------------------------------------------------------------
Function name:  TVOutGetOverride

Description:    Get Connector Override value.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetOverride( PDEV *ppdev, LPQIN lpQIN, PTVGETOVERRIDE lpOutput )
{
    ULONG ulTemp;
    TVPACKET requestPkt;

//  if ( ppdev->dwVmiTv != H3VMI_VIDEO_IN )
    {
        requestPkt.tvPacketFunc = tvGetConnOverride;
        if (!EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_TV_TRANSACTION,
                                &requestPkt,
                                sizeof(requestPkt.tvPacketFunc),
                                lpOutput,
                                sizeof(TVGETOVERRIDE),
                                &ulTemp))
        {
            DISPDBG((1, "GET_OVERRIDE okay"));
        }
    }
}


/*----------------------------------------------------------------------
Function name:  TVOutGetPicControl

Description:    get TV picture control.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetPicControl( PDEV *ppdev, LPTVCURCAP lpInput, LPTVCURCAP lpOutput )
{
  ULONG ulTemp;
  TVPACKET requestPkt;
//  if ( ppdev->dwVmiTv != H3VMI_VIDEO_IN )
    {
        ppdev->dwVmiTv = H3VMI_TV_OUT;
        requestPkt.tvPacketFunc = tvGetPicCtrl;
        memcpy( &requestPkt.tvOptData.tvCurCap, lpInput, sizeof(requestPkt.tvOptData.tvCurCap) );
        if (!EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_TV_TRANSACTION,
                                &requestPkt,
                                (sizeof(requestPkt.tvPacketFunc)+sizeof(requestPkt.tvOptData.tvCurCap)),
                                lpOutput,
                                sizeof(TVCURCAP),
                                &ulTemp))
        {
            DISPDBG((1, "GET_TVPIC_CTRL okay"));
        }
    }
}


#ifdef notused  //3dfx Tools uses QUERYGETPICCONTROL for flicker filter
/*----------------------------------------------------------------------
Function name:  TVOutGetFilterControl

Description:    Get TV filter control.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetFilterControl( PDEV *ppdev, LPQIND lpQIN, LPTVCURCAP lpOutput )
{
  ULONG ulTemp;
  TVPACKET requestPkt;
//  if ( ppdev->dwVmiTv != H3VMI_VIDEO_IN )
    {
        ppdev->dwVmiTv = H3VMI_TV_OUT;
        lpOutput->dwCap = lpQIN->dwCap;
        requestPkt.tvPacketFunc = tvGetFilterCtrl;
        if (!EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_TV_TRANSACTION,
                                &requestPkt,
                                sizeof(requestPkt.tvPacketFunc),
                                lpOutput,
                                sizeof(TVCURCAP),
                                &ulTemp))
        {
            DISPDBG((1, "GET_FILTER_CTRL okay"));
        }
    }
}
#endif //def notused

/*----------------------------------------------------------------------
Function name:  TVOutGetPosControl

Description:    Get TV position contorl.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetPosControl( PDEV *ppdev, LPQIN lpQIN, LPTVCURPOS lpOutput )
{
  ULONG ulTemp;
  TVPACKET requestPkt;
//  if ( ppdev->dwVmiTv != H3VMI_VIDEO_IN )
    {
        ppdev->dwVmiTv = H3VMI_TV_OUT;
        requestPkt.tvPacketFunc = tvGetPositionCtrl;
        if (!EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_TV_TRANSACTION,
                                &requestPkt,
                                sizeof(requestPkt.tvPacketFunc),
                                lpOutput,
                                sizeof(TVCURPOS),
                                &ulTemp))
        {
            DISPDBG((1, "GET_TVPOSITION_CTRL okay"));
        }
    }
}


/*----------------------------------------------------------------------
Function name:  TVOutGetSizeControl

Description:    Get TV size contorl.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetSizeControl( PDEV *ppdev, LPQIN lpQIN, LPTVCURSIZE lpOutput )
{
  ULONG ulTemp;
  TVPACKET requestPkt;
//  if ( ppdev->dwVmiTv != H3VMI_VIDEO_IN )
    {
        ppdev->dwVmiTv = H3VMI_TV_OUT;
        requestPkt.tvPacketFunc = tvGetSizeCtrl;
        if (!EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_TV_TRANSACTION,
                                &requestPkt,
                                sizeof(requestPkt.tvPacketFunc),
                                lpOutput,
                                sizeof(TVCURSIZE),
                                &ulTemp))
        {
            DISPDBG((1, "GET_TVSIZE_CTRL okay"));
        }
    }
}


/*----------------------------------------------------------------------
Function name:  TVOutGetSpecialCtl

Description:    Get TV special control.

Information:    Function is currently empty.
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetSpecialCtl( PDEV *ppdev, LPQIND lpQIN, LPVOID lpOutput )
{
}


/*----------------------------------------------------------------------
Function name:  TVOutGetConStatus

Description:    Get TV connector status.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetConStatus( PDEV *ppdev, LPQIN lpQIN, LPTVCON lpOutput )
{
   ULONG ulTemp;
   DWORD TvStatus;
   TVPACKET requestPkt;
   
   //
   // Collect the TV status from the miniport
   //
   requestPkt.tvPacketFunc = tvGetStatus;
   if (!EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_TV_TRANSACTION,
                           &requestPkt,
                           sizeof(requestPkt.tvPacketFunc),
                           &TvStatus,
                           sizeof(TvStatus),
                           &ulTemp))
   {
      DISPDBG((1, "GET_TVSTATUS okay"));
   }

   lpOutput->dwStatus = 0;
   
   //Get current connector being used from registry setting
   //??? this switch statement looks wrong to me DanO 9/16/99
   switch ( lpOutput->dwType = TV_TYPE_COMPOSITE )
   {
      case TV_TYPE_COMPOSITE:
         if ( TvStatus & 0x8000 )                     //Composite connector?
            lpOutput->dwStatus = TV_PRESENT;
         break;
      case TV_TYPE_SVIDEO:
         if ( TvStatus & 0x6000 )                     //S-Video connector
            lpOutput->dwStatus = TV_PRESENT;
         break;
      default:
         break;
   }
}

/*----------------------------------------------------------------------
Function name:  TVOutSetPicControl

Description:    Set TV picture contorl.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutSetPicControl( PDEV *ppdev, LPTVSETCAP lpInput )
{
  ULONG ulTemp;
  TVPACKET requestPkt;
//   if ( ppdev->dwVmiTv != H3VMI_VIDEO_IN )
   {
        requestPkt.tvPacketFunc = tvSetPicCtrl;
        memcpy( &requestPkt.tvOptData.tvCapability, lpInput, sizeof(requestPkt.tvOptData.tvCapability) );
        if (!EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_TV_TRANSACTION,
                                &requestPkt,
                                (sizeof(requestPkt.tvPacketFunc)+sizeof(requestPkt.tvOptData.tvCapability)),
                                NULL,
                                0,
                                &ulTemp))
        {
            DISPDBG((1, "SET_TVCONTROL okay"));
        }
   }
}


#ifdef notused  //3dfx Tools uses QUERYSETPICCONTROL for flicker filter
/*----------------------------------------------------------------------
Function name:  TVOutSetFilterControl

Description:    Set TV filter control.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutSetFilterControl( PDEV *ppdev, LPTVSETCAP lpInput )
{
  ULONG ulTemp;
  TVPACKET requestPkt;
//   if ( ppdev->dwVmiTv != H3VMI_VIDEO_IN )
   {
        requestPkt.tvPacketFunc = tvSetFilterCtrl;
        memcpy( &requestPkt.tvOptData.tvCapability, lpInput, sizeof(requestPkt.tvOptData.tvCapability));
        if (!EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_TV_TRANSACTION,
                                &requestPkt,
                                (sizeof(requestPkt.tvPacketFunc)+sizeof(requestPkt.tvOptData.tvCapability)),
                                NULL,
                                0,
                                &ulTemp))
        {
            DISPDBG((1, "SET_TVCONTROL okay"));
        }
   }
}
#endif //def notused

/*----------------------------------------------------------------------
Function name:  TVOutSetPosControl

Description:    Set TV position contorl.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutSetPosControl( PDEV *ppdev, LPTVSETPOS lpInput )
{
  ULONG ulTemp;
  TVPACKET requestPkt;
//   if ( ppdev->dwVmiTv != H3VMI_VIDEO_IN )
   {
        requestPkt.tvPacketFunc = tvSetPositionCtrl;
        memcpy( &requestPkt.tvOptData.tvPosition, lpInput, sizeof(requestPkt.tvOptData.tvPosition));
        if (!EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_TV_TRANSACTION,
                                &requestPkt,
                                (sizeof(requestPkt.tvPacketFunc)+sizeof(requestPkt.tvOptData.tvPosition)),
                                NULL,
                                0,
                                &ulTemp))
        {
            DISPDBG((1, "SET_TVPOSITION okay"));
        }
   }
}


/*----------------------------------------------------------------------
Function name:  TVOutSetSizeControl

Description:    Set TV size control.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutSetSizeControl( PDEV *ppdev, LPTVSETSIZE lpInput )
{
  ULONG ulTemp;
  TVPACKET requestPkt;
//   if ( ppdev->dwVmiTv != H3VMI_VIDEO_IN )
   {
        requestPkt.tvPacketFunc = tvSetSizeCtrl;
        memcpy( &requestPkt.tvOptData.tvSize, lpInput, sizeof(requestPkt.tvOptData.tvSize));
        if (!EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_TV_TRANSACTION,
                                &requestPkt,
                                (sizeof(requestPkt.tvPacketFunc)+sizeof(requestPkt.tvOptData.tvSize)),
                                NULL,
                                0,
                                &ulTemp))
        {
            DISPDBG((1, "SET_TVSIZE okay"));
        }
   }
}


/*----------------------------------------------------------------------
Function name:  TVOutSetSpecial

Description:    Set TV special

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutSetSpecial( PDEV *ppdev, LPTVSETSPECIAL lpInput )
{
    ULONG ulTemp;
    DWORD dwCap;
    TVPACKET requestPkt;
   
//    if ( ppdev->dwVmiTv != H3VMI_VIDEO_IN )
    {
        ppdev->dwVmiTv = H3VMI_TV_OUT;
        dwCap = lpInput->dwCap;
        requestPkt.tvPacketFunc = tvSetSpecial;
        memcpy( &requestPkt.tvOptData.tvSpecial, lpInput, sizeof(requestPkt.tvOptData.tvSpecial));
        if (!EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_TV_TRANSACTION,
                                &requestPkt,
                                (sizeof(requestPkt.tvPacketFunc)+sizeof(requestPkt.tvOptData.tvSpecial)),
                                NULL,
                                0,
                                &ulTemp))
        {
            DISPDBG((1, "SET_SPECIAL okay"));
        }
    }
}


/*----------------------------------------------------------------------
Function name:  TVOutSetConStatus

Description:    Set TV connector status.

Information:    Function is currently empty.
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutSetConStatus( PDEV *ppdev, LPTVSETCONNECTOR lpInput )
{
}


const char *TvOutRegKey = \
    "System\\CurrentControlSet\\Services\\Class\\Display\\";


/*----------------------------------------------------------------------
Function name:  TVOutCommitReg

Description:    Write TV values out to the registry.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutCommitReg( PDEV *ppdev, LPQIN lpInput )
{
    DWORD loc_status;
    TVPACKET requestPkt;
   
    requestPkt.tvPacketFunc = tvCommitRegistry;
    EngDeviceIoControl(ppdev->hDriver,
                       IOCTL_TV_TRANSACTION,
                       &requestPkt,
                       sizeof(requestPkt.tvPacketFunc),
                       NULL,
                       0,
                       &loc_status);
}


/*----------------------------------------------------------------------
Function name:  TVOutRefreshMem

Description:    Restore TV values from what is saved in the registry.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutRefreshMem( PDEV *ppdev, LPQIN lpInput )
{
    DWORD loc_status;
    TVPACKET requestPkt;

    requestPkt.tvPacketFunc = tvRefreshRegistry;
    EngDeviceIoControl(ppdev->hDriver,
                       IOCTL_TV_TRANSACTION,
                       &requestPkt,
                       sizeof(requestPkt.tvPacketFunc),
                       NULL,
                       0,
                       &loc_status);

}


/*----------------------------------------------------------------------
Function name:  TVOutDisable

Description:    Disable TV output.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutDisable( PDEV *ppdev)
{
    ULONG ulTemp;
    TVPACKET requestPkt;

    ppdev->dwTvoActive = 0;   // inactive
    requestPkt.tvPacketFunc = tvDisable;
    EngDeviceIoControl(ppdev->hDriver,
                       IOCTL_TV_TRANSACTION,
                       &requestPkt,
                       sizeof(requestPkt.tvPacketFunc),
                       NULL,
                       0,
                       &ulTemp);

    //check if state has not been previously saved
    if ((ppdev->ulDevicesThatHaveTheirStatesSaved & STB_FUNCTIONACTIVE0_TV) != STB_FUNCTIONACTIVE0_TV)
    {
        // mark that the state is changed
        ppdev->ulDevicesThatHaveTheirStatesSaved |= STB_FUNCTIONACTIVE0_TV;
        // save the (presumed) previous state
        ppdev->ulSavedStatesOfSavedDevices &= ~STB_FUNCTIONACTIVE0_TV;
    }

#if 0 // don't know how to port this function from Win9x
	// send the monitor change message so modelist gets updated
	CM_Broadcast_Device_Change_Message (DBT_MONITORCHANGE, 0,CM_BROADCAST_SEND);
#endif
}

/*----------------------------------------------------------------------
Function name:  vpStdToBiosStd

Description:    given VP_TV_STANDARD_THING (from win98 ddk) return BIOS_XX value

Information:    PAL GDHIB are all considered 'generic' PAL.

Return:         unsigned char
----------------------------------------------------------------------*/

static FxU8 vpStdToBiosStd (DWORD vpStd)
{
    if (vpStd & VP_TV_STANDARD_PAL_NC)
        return (BIOS_PAL_Nc);
    else if (vpStd & VP_TV_STANDARD_PAL_M)
        return (BIOS_PAL_M);
    else if (vpStd & VP_TV_STANDARD_PAL_N)
        return (BIOS_PAL_N);
    else if (vpStd & VP_TV_STANDARD_NTSC_M)
        return (BIOS_NTSC);
    else
        return (BIOS_PAL);
}

/*----------------------------------------------------------------------
Function name:  TVOutEnable

Description:    Enable tvout at last known standard and set BIOS flags

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutEnable ( PDEV *ppdev, LPQIN lpInput )
{
    ULONG ulTemp;
    TVPACKET requestPkt;

//#ifdef STB_DFP_ENABLED
#if 0
    // IF THE LCD PANEL IS ON, TURN IT OFF
    if (ppdev->bDfpActive)
    {
        STB_PROPERTY In, Out;

        In.ulPropertyId = STB_DFP_DISPLAYCONTROL;
        In.ulPropertyValue = STB_DFP_OFF;
        DFPSetProperty((void*)&In, (void*)&Out, ppdev);
    }
#endif //def STB_DFP_ENABLED

    requestPkt.tvPacketFunc = tvEnable;
    EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_TV_TRANSACTION,
                           &requestPkt,
                           sizeof(requestPkt.tvPacketFunc),
                           NULL,
                           0,
                           &ulTemp);
    ppdev->dwTvoActive = 1;   // on

//??? should save current state before changing it???
    //check if state has not been previously saved
    if ((ppdev->ulDevicesThatHaveTheirStatesSaved & STB_FUNCTIONACTIVE0_TV) != STB_FUNCTIONACTIVE0_TV)
    {
        // mark that the state is changed
        ppdev->ulDevicesThatHaveTheirStatesSaved |= STB_FUNCTIONACTIVE0_TV;
        // save the (presumed) previous state
        ppdev->ulSavedStatesOfSavedDevices |= STB_FUNCTIONACTIVE0_TV;
    }

#if 0 // don't know how to port this function from Win9x
	// send the monitor change message so modelist gets updated
	CM_Broadcast_Device_Change_Message (DBT_MONITORCHANGE, 0,CM_BROADCAST_SEND);
#endif
}

/*----------------------------------------------------------------------
Function name:  TVOutSetStandard

Description:    Set TV standard.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutSetStandard( PDEV *ppdev, LPTVSETSTANDARD lpInput )
{
    ULONG ulTemp;
    TVPACKET requestPkt;

    ppdev->dwVmiTv = H3VMI_TV_OUT;
    requestPkt.tvPacketFunc = tvSetStandard;
    memcpy( &requestPkt.tvOptData.tvStandard, lpInput, sizeof(requestPkt.tvOptData.tvStandard));
    EngDeviceIoControl(ppdev->hDriver,
                       IOCTL_TV_TRANSACTION,
                       &requestPkt,
                       (sizeof(requestPkt.tvPacketFunc)+sizeof(requestPkt.tvOptData.tvStandard)),
                       NULL,
                       0,
                       &ulTemp);

}


/*----------------------------------------------------------------------
Function name:  TVOutSetOverride

Description:    Set Connector Override.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutSetOverride( PDEV *ppdev, PTVSETOVERRIDE lpInput )
{
    ULONG ulTemp;
    TVPACKET requestPkt;

    requestPkt.tvPacketFunc = tvSetConnOverride;
    memcpy( &requestPkt.tvOptData.tvConnOverride, lpInput, sizeof(requestPkt.tvOptData.tvConnOverride));
    EngDeviceIoControl(ppdev->hDriver,
                       IOCTL_TV_TRANSACTION,
                       &requestPkt,
                       (sizeof(requestPkt.tvPacketFunc)+sizeof(requestPkt.tvOptData.tvConnOverride)),
                       NULL,
                       0,
                       &ulTemp);

}


/*----------------------------------------------------------------------
Function name:  TVOutVideoParameters

Description:    Process the TvOout HandleVideoParameters request from the caller.

Information:    See Win2K DDK IOCTL_VIDEO_HANDLE_VIDEOPARAMETERS for details.
                Also see "3dfx Tools API Specification Document".
  
Return:         int     1 for success, -1 for failure
----------------------------------------------------------------------*/
int TVOutVideoParameters (PDEV *ppdev, LPVIDEOPARAMETERS lpVidParamsIn, LPVIDEOPARAMETERS lpVidParamsOut)
{
    ULONG ulTemp;

	// Check the GUID of the tv out part:
	if (memcmp (&vpguid, &(lpVidParamsIn)->Guid, sizeof(vpguid)))
 		return (-1);

    if (!EngDeviceIoControl(ppdev->hDriver,
            IOCTL_VIDEO_HANDLE_VIDEOPARAMETERS,
            lpVidParamsIn,
            sizeof(VIDEOPARAMETERS),
            lpVidParamsOut,
            sizeof(VIDEOPARAMETERS),
            &ulTemp))
    {
        DISPDBG((1, "VideoParameters operation sucessful"));
        return(1);
    }
    else
        return(-1);
}


