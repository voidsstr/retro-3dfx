/* $Header: tvout.c, 8, 10/11/00 8:51:44 PM, Brent$ */
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
** $Revision: 8$
** $Date: 10/11/00 8:51:44 PM$
**
** $History: tvout.c $
** 
** *****************  Version 25  *****************
** User: Rbissell     Date: 8/07/99    Time: 3:11p
** Updated in $/devel/h5/Win9x/dx/dd16
** tvout merge from V3_OEM_100
** 
** *****************  Version 24  *****************
** User: Michael      Date: 6/02/99    Time: 2:42p
** Updated in $/devel/h5/Win9x/dx/dd16
** Add -WX to the makefile and clean up all warnings in the DD16
** subdirectory.
** 
** *****************  Version 22  *****************
** User: Stb_rbissell Date: 5/14/99    Time: 9:47p
** Updated in $/devel/h3/win95/dx/dd16
** tvout and dfp changes
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

#include <string.h>
#include "header.h"

#define Not_VxD
#include "minivdd.h"

#define Not_VxD
#include <vmm.h>

#define MIDL_PASS     // suppress 32-bit only #pragma pack(push)

#pragma warning (disable: 4047 4704)
#include <configmg.h>
#pragma warning (default: 4047 4704)

#include "modelist.h"
#include "tv.h"
#include "qmodes.h"
#include "fxtvout.h"
#include "funcapi.h"
#include "edgedefs.h"
#include "dfpapi.h"

extern char *_itoa( int value, char *string, int radix );
extern int atoi( const char *string );

extern DWORD dwDeviceHandle;
extern DISPLAYINFO DisplayInfo;
extern WORD FirstEnable;



void TVOutCancelSettings(void * pInData,void * pOutData)
{
   QIN Qin;
   TVSETSTANDARD QStd;

   if (_FF(ulDevicesThatHaveTheirStatesSaved) & STB_FUNCTIONACTIVE0_TV)
   {
      if (_FF(ulSavedStatesOfSavedDevices) & STB_FUNCTIONACTIVE0_TV)
      {
         Qin.dwSubFunc = QUERYDISABLETV;
      }
      else
      {
         // TURNING ON THE TV IS A TWO-STEP PROCESS.
         
         // FIRST, CALL Q-SETSTANDARD TO ENABLE THE TV (as opposed to "Q-ENABLETV")
         QStd.dwStandard = 0;
         QStd.dwSubFunc = QUERYSETSTANDARD;
         QueryMode((LPQIN)&QStd, (LPQIN)&QStd);

         // NOW REMIND MYSELF THAT I JUST TURNED ON THE TV
         // (THIS DOESN'T ACTUALLY TURN ON THE TV.)
         Qin.dwSubFunc = QUERYENABLETV;
      }


      QueryMode((LPQIN)&Qin, (LPQIN)&QStd);
   }
}


/*----------------------------------------------------------------------
Function name:  TVOutStatus

Description:    Get encoder type, set capabilities flags.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutStatus( LPQIN lpQIN, LPQTVSTATUS lpOutput )
{
   DWORD TvStatus;
   
   TvStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_GET_TVSTATUS, 0, lpQIN);

   lpOutput->dwEncoder = 0;                              //Encoder is not present or busy
   
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

	lpOutput->dwNumConnectors = 2; 
	lpOutput->TVCon[0].dwType = TV_TYPE_SVIDEO;
	lpOutput->TVCon[0].dwStatus = (TvStatus & 0x80) ? TV_CONNECTOR_ENABLED : 0
		                        | (TvStatus & 0x6000) ? TV_PRESENT : 0;
	lpOutput->TVCon[1].dwType = TV_TYPE_COMPOSITE;
	lpOutput->TVCon[1].dwStatus = (TvStatus & 0x80) ? TV_CONNECTOR_ENABLED : 0
		                        | (TvStatus & 0x8000) ? TV_PRESENT : 0;
}


/*----------------------------------------------------------------------
Function name:  TVOutConStatus

Description:    Get status of the TV connector.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutConStatus( LPQIN lpQIN, LPTVCONSTATUS lpOutput )
{
   DWORD TvStatus;
   int iConnector;
   
   TvStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_GET_TVSTATUS, 0, lpQIN);
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
void TVOutGetPicCap( LPQIND lpQIN, LPTVCAPDATA lpOutput )
{
//	if ( _FF(dwVmiTv) != H3VMI_VIDEO_IN )
	{
		_FF(dwVmiTv) = H3VMI_TV_OUT;
		lpOutput->dwCap = lpQIN->dwCap;
		VDDCall (VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
						H3VDD_GET_PIC_CAPS, 0, lpOutput);
	}
}


/*----------------------------------------------------------------------
Function name:  TVOutGetFilterCap

Description:    Get TV filter capabilities.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetFilterCap( LPQIND lpQIN, LPTVCAPDATA lpOutput )
{
//	if ( _FF(dwVmiTv) != H3VMI_VIDEO_IN )
	{
		_FF(dwVmiTv) = H3VMI_TV_OUT;
		lpOutput->dwCap = lpQIN->dwCap;
		VDDCall (VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
						H3VDD_GET_FILTER_CAPS, 0, lpOutput);
	}
}


/*----------------------------------------------------------------------
Function name:  TVOutGetPosCap

Description:    Get TV Position capabilities.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetPosCap( LPQIN lpQIN, LPTVPOSCAP lpOutput )
{
//	if ( _FF(dwVmiTv) != H3VMI_VIDEO_IN )
	{
		_FF(dwVmiTv) = H3VMI_TV_OUT;
		VDDCall (VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
						H3VDD_GET_POS_CAPS, 0, lpOutput);
	}
}


/*----------------------------------------------------------------------
Function name:  TVOutGetSizeCap

Description:    Get TV Size capabilities.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetSizeCap( LPQIN lpQIN, LPTVSIZECAP lpOutput )
{
//	if ( _FF(dwVmiTv) != H3VMI_VIDEO_IN )
	{
		_FF(dwVmiTv) = H3VMI_TV_OUT;
		VDDCall (VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
						H3VDD_GET_SIZE_CAPS, 0, lpOutput);
	}
}


/*----------------------------------------------------------------------
Function name:  TVOutGetSpecialCap

Description:    Get TV special capabilities.

Information:    Function is currently empty.
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetSpecialCap( LPQIND lpQIN, LPVOID lpOutput )
{
}


/*----------------------------------------------------------------------
Function name:  TVOutGetStandard

Description:    Get TV standard capabilities.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetStandard( LPQIN lpQIN, LPTVGETSTANDARD lpOutput )
{
   TVSETSTANDARD HackOutput;

#ifdef STU_ORIGINAL
//	if ( _FF(dwVmiTv) != H3VMI_VIDEO_IN )
	{
		_FF(dwVmiTv) = H3VMI_TV_OUT;
		VDDCall (VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
						H3VDD_GET_STANDARD, 0, lpOutput);
	}
#else

   // MAJOR HACK: You really don't want to know this, but...
   // if "myEDX" is 0, then it's a traditional "H3VDD_GET_STANDARD" call.
   // if "myEDX" is 1, it's POTENTIALLY an overloaded call to get the current standard override.
   //
   // Stuart's code used 0, the new stuff uses 1.

   HackOutput.dwSubFunc = lpQIN->dwSubFunc;

   switch (lpQIN->dwSubFunc)
   {
      case QUERYGETSTANDARD:
		   _FF(dwVmiTv) = H3VMI_TV_OUT;
		   VDDCall (VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_GET_STANDARD, 1, &HackOutput);
         _FF(dwTvoStd) = HackOutput.dwStandard;
         break;

      default:
      case QUERYGETOVERRIDE:
         VDDCall (VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_GET_STANDARD, 1, &HackOutput);
         break;
   }

   *((LPTVSETSTANDARD)lpOutput) = HackOutput;

#endif //STU_ORIGINAL
}


/*----------------------------------------------------------------------
Function name:  TVOutGetPicControl

Description:    get TV picture control.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetPicControl( LPQIND lpQIN, LPTVCURCAP lpOutput )
{
//	if ( _FF(dwVmiTv) != H3VMI_VIDEO_IN )
	{
		_FF(dwVmiTv) = H3VMI_TV_OUT;
		lpOutput->dwCap = lpQIN->dwCap;
		VDDCall (VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
						H3VDD_GET_TVPIC_CTRL, 0, lpOutput);
	}
}


/*----------------------------------------------------------------------
Function name:  TVOutGetFilterControl

Description:    Get TV filter control.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetFilterControl( LPQIND lpQIN, LPTVCURCAP lpOutput )
{
//	if ( _FF(dwVmiTv) != H3VMI_VIDEO_IN )
	{
		_FF(dwVmiTv) = H3VMI_TV_OUT;
		lpOutput->dwCap = lpQIN->dwCap;
		VDDCall (VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
						H3VDD_GET_FILTER_CTRL, 0, lpOutput);
	}
}


/*----------------------------------------------------------------------
Function name:  TVOutGetPosControl

Description:    Get TV position contorl.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetPosControl( LPQIN lpQIN, LPTVCURPOS lpOutput )
{
//	if ( _FF(dwVmiTv) != H3VMI_VIDEO_IN )
	{
		_FF(dwVmiTv) = H3VMI_TV_OUT;
		VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
							H3VDD_GET_TVPOSITION_CTRL, 0, lpOutput);
	}
}


/*----------------------------------------------------------------------
Function name:  TVOutGetSizeControl

Description:    Get TV size contorl.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetSizeControl( LPQIN lpQIN, LPTVCURSIZE lpOutput )
{
//	if ( _FF(dwVmiTv) != H3VMI_VIDEO_IN )
	{
		_FF(dwVmiTv) = H3VMI_TV_OUT;
		VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
							H3VDD_GET_TVSIZE_CTRL, 0, lpOutput);
	}
}


/*----------------------------------------------------------------------
Function name:  TVOutGetSpecialCtl

Description:    Get TV size contorl.

Information:    Function is currently empty.
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetSpecialCtl( LPQIND lpQIN, LPVOID lpOutput )
{
}


/*----------------------------------------------------------------------
Function name:  TVOutGetConStatus

Description:    Get TV connector status.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutGetConStatus( LPQIN lpQIN, LPTVCON lpOutput )
{
   DWORD TvStatus;
   
   TvStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_GET_TVSTATUS, 0, lpQIN);

   lpOutput->dwStatus = 0;
   
   //Get current connector being used from registry setting
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
void TVOutSetPicControl( LPTVSETCAP lpQIN )
{
//   if ( _FF(dwVmiTv) != H3VMI_VIDEO_IN )
   {
      VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_SET_TVCONTROL, 0, lpQIN);
   }
}


/*----------------------------------------------------------------------
Function name:  TVOutSetFilterControl

Description:    Set TV filter control.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutSetFilterControl( LPTVSETCAP lpQIN )
{
//   if ( _FF(dwVmiTv) != H3VMI_VIDEO_IN )
   {
      VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_SET_TVCONTROL, 0, lpQIN);
   }
}


/*----------------------------------------------------------------------
Function name:  TVOutSetPosControl

Description:    Set TV position contorl.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutSetPosControl( LPTVSETPOS lpQIN )
{
//   if ( _FF(dwVmiTv) != H3VMI_VIDEO_IN )
   {
      VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_SET_TVPOSITION, 0, lpQIN);
   }
}


/*----------------------------------------------------------------------
Function name:  TVOutSetSizeControl

Description:    Set TV size control.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutSetSizeControl( LPTVSETSIZE lpQIN )
{
//   if ( _FF(dwVmiTv) != H3VMI_VIDEO_IN )
   {
      VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_SET_TVSIZE, 0, lpQIN);
   }
}


/*----------------------------------------------------------------------
Function name:  TVOutSetSpecial

Description:    Set TV special

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutSetSpecial( LPTVSETSPECIAL lpQIN )
{
   DWORD dwCap;
   
//   if ( _FF(dwVmiTv) != H3VMI_VIDEO_IN )
   {
      _FF(dwVmiTv) = H3VMI_TV_OUT;
      dwCap = lpQIN->dwCap;
      VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_SET_SPECIAL, 0, lpQIN);
   }
}


/*----------------------------------------------------------------------
Function name:  TVOutSetConStatus

Description:    Set TV connector status.

Information:    Function is currently empty.
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutSetConStatus( LPTVSETCONNECTOR lpQIN )
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
void TVOutCommitReg( LPQIN lpQIN )
{
	char valueName[24], firstVname[24], value[16]; 
	DWORD v;

	VDDCall (VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
			  H3VDD_GET_REGISTRY_INFO, 0, firstVname);
	do
	{
		v = VDDCall (VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
				     H3VDD_GET_REGISTRY_INFO, 0, (void _far *)valueName);
		_itoa ((int)v, value, 10);
		CM_Write_Registry_Value (DisplayInfo.diDevNodeHandle, "TV",
			valueName, REG_SZ, value, strlen(value), CM_REGISTRY_SOFTWARE);
	}
	while (strcmp (firstVname, valueName));
}


/*----------------------------------------------------------------------
Function name:  TVOutRefreshMem

Description:    Restore TV values from what is saved in the registry.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutRefreshMem( LPQIN lpQIN )
{
	char valueName[24], firstVname[24], value[16]; 
	DWORD v;

	VDDCall (VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
			  H3VDD_GET_REGISTRY_INFO, 0, firstVname);
	do
	{
		// get the name of the next parameter
		VDDCall (VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
				     H3VDD_GET_REGISTRY_INFO, 0, (void _far *)valueName);
		v = sizeof(value);
		// read the registry's recollection of this parameter
		value[0] = 0;
		CM_Read_Registry_Value (DisplayInfo.diDevNodeHandle, "TV",
				valueName, REG_SZ, (LPBYTE)value, &v, CM_REGISTRY_SOFTWARE);
		if (!value[0])    // something horribly wrong
			break;
		v = atoi (value);
		// set the parameter
		VDDCall (VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
				     H3VDD_SET_REGISTRY_INFO, v, (void _far *)valueName);
	}
	while (strcmp (firstVname, valueName));
}

/*----------------------------------------------------------------------
Function name:  tvoutCallMinivdd

Description:    Call into the display driver's minivdd.

Information:    
  
Return:         LONG    value returned from VDD call.
----------------------------------------------------------------------*/
long tvoutCallMinivdd (int parameter, void *io)
{
	return (VDDCall (VDD_REGISTER_DISPLAY_DRIVER_INFO,
				dwDeviceHandle, parameter, 0, (void _far *)io));
}


/*----------------------------------------------------------------------
Function name:  TVOutDisable

Description:    Disable TV output.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutDisable()
{
   DWORD value;
   char valueName[24];

   _FF(dwTvoActive) = 0;   // inactive
   tvoutCallMinivdd (H3VDD_DISABLE_TV, NULL);

   if (!(_FF(ulDevicesThatHaveTheirStatesSaved) & STB_FUNCTIONACTIVE0_TV))
   {
       _FF(ulDevicesThatHaveTheirStatesSaved) |= STB_FUNCTIONACTIVE0_TV;
       if (_FF(dwTvoActive))
           _FF(ulSavedStatesOfSavedDevices) |= STB_FUNCTIONACTIVE0_TV;
       else
           _FF(ulSavedStatesOfSavedDevices) &= ~STB_FUNCTIONACTIVE0_TV;
   }

   // commit our decisions back to the registry
   value = 0;  // we are not presently active
   strcpy(valueName, "enabled");
   CM_Write_Registry_Value(DisplayInfo.diDevNodeHandle, "TV",
                           valueName, REG_DWORD, &value,
                           sizeof(DWORD), CM_REGISTRY_SOFTWARE);

   // send the monitor change message so modelist gets updated
   CM_Broadcast_Device_Change_Message (DBT_MONITORCHANGE, 0, CM_BROADCAST_SEND);
}


/*----------------------------------------------------------------------
Function name:  TVOutEnable

Description:    Enable tvout at last known standard and set BIOS flags

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutEnable ( LPQIN lpQIN )
{
   DWORD value;
   char valueName[24];
	TVSETSTANDARD tvSetStd; // no, that is not a typo.

   // IF THE LCD PANEL IS ON, TURN IT OFF
   if (DFPisPanelActive())
   {
      STB_PROPERTY In, Out;

      In.ulPropertyId = STB_DFP_DISPLAYCONTROL;
      In.ulPropertyValue = STB_DFP_OFF;
      DFPSetProperty((void*)&In, (void*)&Out);
   }

   _FF(ulDevicesThatHaveTheirStatesSaved) |= STB_FUNCTIONACTIVE0_TV;
   _FF(ulSavedStatesOfSavedDevices) |= STB_FUNCTIONACTIVE0_TV;

   // "h3vdd_get_standard" has been overloaded to return of of two
   // things, dependent on the "dwSubFunc" field of the tvSetStd struct.
   // if "tvSetStd->dwSubfunc == QUERYGETSTANDARD", then it works as it
   // always has.  But if it is ZERO, then we return the BIOS connection
   // override setting.
   //
   // Ugly? Yes!

   tvSetStd.dwSubFunc = QUERYGETSTANDARD;

	// get the last known standard
	tvoutCallMinivdd (H3VDD_GET_STANDARD, &tvSetStd);
	_FF(dwTvoActive) = 1;   // on

   // commit our decisions back to the registry
   value = 1;  // we are presently active
   strcpy(valueName, "enabled");
   CM_Write_Registry_Value(DisplayInfo.diDevNodeHandle, "TV",
                           valueName, REG_DWORD, &value,
                           sizeof(DWORD), CM_REGISTRY_SOFTWARE);

   // send the monitor change message so modelist gets updated
   CM_Broadcast_Device_Change_Message (DBT_MONITORCHANGE, 0, CM_BROADCAST_SEND);
}


void TvoutAllowCRTWithTV(FxU32 State)
{
   VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_ALLOW_CRT_WITH_TV, 0, &State);
}

void TvoutAllowPALandCRT(FxU32 State)
{
   VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_ALLOW_PAL_AND_CRT, 0, &State);
}

/*----------------------------------------------------------------------
Function name:  TVOutSetStandard

Description:    Set TV standard.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void TVOutSetStandard( LPTVSETSTANDARD lpQIN )
{

      switch (lpQIN->dwSubFunc)
      {
         case QUERYSETSTANDARD:
            if (lpQIN->dwStandard)
            {
               // just changing the standard
               _FF(dwTvoStd) = lpQIN->dwStandard;

            }
            else
            {
               // just turning on the TV.
               _FF(dwVmiTv) = H3VMI_TV_OUT;
            }

            VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_SET_TVSTANDARD, 0, lpQIN);
            // send the monitor change message so modelist gets updated
	         CM_Broadcast_Device_Change_Message (DBT_MONITORCHANGE, 0, CM_BROADCAST_SEND);
            break;

         case QUERYSETOVERRIDE:
         default:
            VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_SET_TVSTANDARD, 0, lpQIN);
      }
}


/*
Windows 98 Display Driver Interface.  This is over and above Andrew's 
implementation as currently described in inc\tv.h.  I think I will assume
just the Brooktree chip for caps we advertise.
*/

/*----------------------------------------------------------------------
Function name:  TVOutVideoParameters

Description:    Process the TV out request from the caller.

Information:    
  
Return:         int     1 for success, -1 for failure
----------------------------------------------------------------------*/
TVOutVideoParameters (LPVIDEOPARAMETERS lpVidParams)
{
	VIDEOPARAMETERS vidParmTmp;
	union
	{
		TVSETSTANDARD tvSetStd;
		TVSETCAP tvSetCap;
		TVSETPOS tvSetPos;
		TVSETSIZE tvSetSize;
		TVGETSTANDARD tvGetStd;
		TVCURCAP tvCurCap;
		TVCURPOS tvCurPos;
		TVCURSIZE tvCurSize;
		QIN qin;
		TVSETSPECIAL tvSetSpecial;
		DWORD mkSpc[40];
	}  ioBuf;
//	long dbgParms[8];
	static DWORD dwCpKey, tvStatus;
	DWORD dwPal	= VP_TV_STANDARD_PAL_B | VP_TV_STANDARD_PAL_D |  \
				  VP_TV_STANDARD_PAL_H | VP_TV_STANDARD_PAL_I;

	// check the GUID
	if (memcmp (&vpguid, &lpVidParams->Guid, sizeof(vpguid)))
		return (-1);    // wrong GUID

	if (!tvStatus)
		tvStatus = tvoutCallMinivdd (H3VDD_GET_TVSTATUS, &ioBuf.qin);
	if (tvStatus == 0xffffffff || tvStatus == 0xdeadbeef)
		return (-1);    // no tvout adapter, apparently

//	dbgParms[0] = lpVidParams->dwCommand;
//	dbgParms[1] = lpVidParams->dwCPCommand;
//	dbgParms[2] = lpVidParams->dwCPKey;
//	dbgParms[3] = lpVidParams->dwFlags;
//	mysprintf ((void _far *)ioBuf.tvSetSpecial.wData,
//			   (void _far *)"%ld %ld %lx %lx", (void _far *)dbgParms);
//	ioBuf.tvSetSpecial.dwCap = TV_SETSPECIAL_WR_LCDREG;
//	tvoutCallMinivdd (H3VDD_SET_SPECIAL, &ioBuf.tvSetSpecial);

	if (lpVidParams->dwCommand == VP_COMMAND_GET)
	{
		 vidParmTmp = *lpVidParams;
		 memset (lpVidParams, 0, sizeof(lpVidParams));
		 lpVidParams->Guid = vidParmTmp.Guid;
		 lpVidParams->dwCommand = vidParmTmp.dwCommand;

		 if (((tvStatus >> 21) & 7) == 1)    // is a BT869
		 {
			 lpVidParams->dwCPType = \
				 VP_CP_TYPE_MACROVISION | VP_CP_TYPE_APS_TRIGGER;
			 lpVidParams->dwCPStandard = dwPal | VP_TV_STANDARD_NTSC_M;
		 }

		 lpVidParams->dwFlags = VP_FLAGS_TV_MODE |
		                        VP_FLAGS_TV_STANDARD |
								VP_FLAGS_FLICKER |
								VP_FLAGS_OVERSCAN |
								VP_FLAGS_MAX_UNSCALED |
								VP_FLAGS_POSITION |
								VP_FLAGS_BRIGHTNESS |
								VP_FLAGS_CONTRAST |    // actually chroma
								VP_FLAGS_COPYPROTECT;

		 lpVidParams->dwMode = VP_MODE_WIN_GRAPHICS;
		 lpVidParams->dwMaxUnscaledX = 800;
		 lpVidParams->dwMaxUnscaledY = 600;

       ioBuf.tvSetStd.dwSubFunc = QUERYGETSTANDARD;
		 tvoutCallMinivdd (H3VDD_GET_STANDARD, &ioBuf.tvSetStd);
		 lpVidParams->dwTVStandard = ioBuf.tvSetStd.dwStandard;

		 lpVidParams->dwAvailableModes = VP_MODE_WIN_GRAPHICS;
		 lpVidParams->dwAvailableTVStandard = dwPal | VP_TV_STANDARD_NTSC_M |
		    VP_TV_STANDARD_PAL_NC | VP_TV_STANDARD_PAL_N | VP_TV_STANDARD_PAL_M;

		 ioBuf.tvCurCap.dwCap = TV_FLICKER;
		 tvoutCallMinivdd (H3VDD_GET_TVPIC_CTRL, &ioBuf.tvCurCap);
		 lpVidParams->dwFlickerFilter = ioBuf.tvCurCap.dwStep * 250;

		 tvoutCallMinivdd (H3VDD_GET_TVPOSITION_CTRL, &ioBuf.tvCurPos);
		 lpVidParams->dwPositionX = ioBuf.tvCurPos.dwCurLeft;
		 lpVidParams->dwPositionY = ioBuf.tvCurPos.dwCurTop;

		 ioBuf.tvCurCap.dwCap = TV_BRIGHTNESS;
		 tvoutCallMinivdd (H3VDD_GET_TVPIC_CTRL, &ioBuf.tvCurCap);
		 if ((lpVidParams->dwBrightness = ioBuf.tvCurCap.dwStep * 14) == 98)
			 lpVidParams->dwBrightness = 100;

		 ioBuf.tvCurCap.dwCap = TV_SATURATION;
		 tvoutCallMinivdd (H3VDD_GET_TVPIC_CTRL, &ioBuf.tvCurCap);
		 if ((lpVidParams->dwContrast = ioBuf.tvCurCap.dwStep * 14) == 98)
			 lpVidParams->dwContrast = 100;

		 tvoutCallMinivdd (H3VDD_GET_TVSIZE_CTRL, &ioBuf.tvCurSize);
		 lpVidParams->dwOverScanX = ioBuf.tvCurSize.dwCurHorOutput * 250;
		 lpVidParams->dwOverScanY = ioBuf.tvCurSize.dwCurVerOutput * 250;

		 return (1);
	}
	else if (lpVidParams->dwCommand == VP_COMMAND_SET)
	{
		if (lpVidParams->dwFlags & VP_FLAGS_FLICKER)
		{
			ioBuf.tvSetCap.dwCap = TV_FLICKER;
			ioBuf.tvSetCap.dwStep = lpVidParams->dwFlickerFilter / 250;
			tvoutCallMinivdd (H3VDD_SET_TVCONTROL, &ioBuf.tvSetCap);
		}
		else if (lpVidParams->dwFlags & VP_FLAGS_TV_STANDARD)
		{
         // RYAN@990623: I think this is wrong.  I think the "VP_TV_STANDARD_WIN_VGA"
         // standard is what is supposed to be used to turn off the TV... not zero.
         // Need to research this.
			if (!(ioBuf.tvSetStd.dwStandard = lpVidParams->dwTVStandard))
			{
				tvoutCallMinivdd (H3VDD_DISABLE_TV, &ioBuf);
				return (1);
			}
			ioBuf.tvSetStd.dwSubFunc = QUERYSETSTANDARD;
         TVOutSetStandard(&ioBuf.tvSetStd);
		}
		else if (lpVidParams->dwFlags & VP_FLAGS_POSITION)
		{
			ioBuf.tvSetPos.dwLeft = lpVidParams->dwPositionX;
			ioBuf.tvSetPos.dwTop = lpVidParams->dwPositionY;
			tvoutCallMinivdd (H3VDD_SET_TVPOSITION, &ioBuf.tvSetPos);
		}
		else if (lpVidParams->dwFlags & VP_FLAGS_OVERSCAN)
		{
			ioBuf.tvSetSize.dwOverScan =  \
				(lpVidParams->dwOverScanX + lpVidParams->dwOverScanY) / 500;
			if (ioBuf.tvSetSize.dwOverScan > (2000 / 400))
				return (-1);
			tvoutCallMinivdd (H3VDD_SET_TVSIZE, &ioBuf.tvSetSize);
		}
		else if (lpVidParams->dwFlags & VP_FLAGS_BRIGHTNESS)
		{
			ioBuf.tvSetCap.dwCap = TV_BRIGHTNESS;
			ioBuf.tvSetCap.dwStep = lpVidParams->dwBrightness / 14;
			tvoutCallMinivdd (H3VDD_SET_TVCONTROL, &ioBuf.tvSetCap);
		}
		else if (lpVidParams->dwFlags & VP_FLAGS_CONTRAST)
		{
			ioBuf.tvSetCap.dwCap = TV_SATURATION;
			ioBuf.tvSetCap.dwStep = lpVidParams->dwContrast / 14;
			tvoutCallMinivdd (H3VDD_SET_TVCONTROL, &ioBuf.tvSetCap);
		}
		else if (lpVidParams->dwFlags & VP_FLAGS_COPYPROTECT)
		{
			switch (lpVidParams->dwCPCommand)
			{
				case VP_CP_CMD_ACTIVATE:
					dwCpKey = lpVidParams->dwCPKey;  // remember this key
					ioBuf.mkSpc[0] = lpVidParams->bCP_APSTriggerBits & 3;
					if (1L != tvoutCallMinivdd (H3VDD_MACROVISION_ON, &ioBuf.mkSpc[0]))
						return (-1);
					break;
				case VP_CP_CMD_DEACTIVATE:
					// don't disable copy protect unless key matches
					if (dwCpKey == lpVidParams->dwCPKey)
						tvoutCallMinivdd (H3VDD_MACROVISION_OFF, 0);
					break;
				case VP_CP_CMD_CHANGE:
					// don't change copy protect unless key matches
					if (dwCpKey == lpVidParams->dwCPKey)
					{
        				ioBuf.mkSpc[0] = lpVidParams->bCP_APSTriggerBits & 3;
						if (1L != tvoutCallMinivdd (H3VDD_MACROVISION_ON, &ioBuf.mkSpc[0]))
							return (-1);
					}
			}
		}
		return (1);
	}
	else
		return (-1);
}


/*----------------------------------------------------------------------
Function name:  tvoutGetBootStatus

Description:    Get the boot status of the TV.

Information:    
  
Return:         FxU32       Masked bits of the boot status.
----------------------------------------------------------------------*/
FxU32 tvoutGetBootStatus()
{
	QIN qin;
	FxI32 bootStatus;

	bootStatus = tvoutCallMinivdd (H3VDD_GET_TVINIT_STATUS, 0);
	if (bootStatus == -1)   // we have not attempted to initialize yet
	{
		// this will do the init just fine
		tvoutCallMinivdd (H3VDD_GET_TVSTATUS, &qin);
		bootStatus = tvoutCallMinivdd (H3VDD_GET_TVINIT_STATUS, 0);
	}

	// If this is FirstEnable time we have now recorded the BIOS mode in the
	// vxd due to the H3VDD_GET_TVINIT_STATUS call.  (see h3vdd.c)

	if (!(bootStatus & BIOS_TVOUT_ACTIVE))
	{
		if (FirstEnable && (bootStatus & BIOS_TVSTD_MASK))
		{
			// clear the standard from the scratch register
			bootStatus = 0;
		}
	}
	return (bootStatus & BIOS_TVSTD_MASK);
}


/*----------------------------------------------------------------------
Function name:  tvoutSetStdInternal

Description:    Fill out elements from an internal TV structure
                and set as a standard TV.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void tvoutSetStdInternal()
{
	TVSETSTANDARD tvoutStd;
	FxU32 bootStat = tvoutGetBootStatus();

	if (bootStat & 0x1b)
	{
		switch (bootStat)
		{
			case BIOS_PAL:
				tvoutStd.dwStandard = VP_TV_STANDARD_PAL_G;
				break;
			case BIOS_PAL_N:
				tvoutStd.dwStandard = VP_TV_STANDARD_PAL_N;
				break;
			case BIOS_PAL_M:
				tvoutStd.dwStandard = VP_TV_STANDARD_PAL_M;
				break;
			case BIOS_PAL_Nc:
				tvoutStd.dwStandard = VP_TV_STANDARD_PAL_NC;
				break;
			default:
				tvoutStd.dwStandard = VP_TV_STANDARD_NTSC_M;
		}
		tvoutStd.dwSubFunc = QUERYSETSTANDARD;
		tvoutCallMinivdd (H3VDD_SET_TVSTANDARD, &tvoutStd);
	}
}


/*----------------------------------------------------------------------
Function name:  tvoutDisableInternal

Description:    Disable the TV.

Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void tvoutDisableInternal()
{
	QIN Qin;

	if (tvoutGetBootStatus() & 0x1b)
		tvoutCallMinivdd (H3VDD_DISABLE_TV, &Qin);
}

