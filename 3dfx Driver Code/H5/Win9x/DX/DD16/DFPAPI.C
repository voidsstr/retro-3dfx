/* -*-c++-*- */
/* $Header: dfpapi.c, 7, 10/11/00 8:50:57 PM, Brent$
/*
** Copyright (c) 2000 3Dfx Interactive, Inc.
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
** File name:   dfpapi.c
**
** Description: Support functions for 3dfx Tools support of a digital flat panel.
**
** $Revision: 7$
** $Date: 10/11/00 8:50:57 PM$
**
** $History: dfpapi.c $
** 
** 
**
** 
*/


#include "dfpapi.h"
#include "edgedefs.h"

// driver specific includes below 
#include "header.h"
#define Not_VxD
#include "minivdd.h"

#define Not_VxD
#include <vmm.h>

#define MIDL_PASS     // suppress 32-bit only #pragma pack(push)
#pragma warning (disable: 4047 4704)
#include <configmg.h>
#pragma warning (default: 4047 4704)

#include "funcapi.h"
#include "dfpctrl.h"
#include <string.h>

extern void TVOutDisable( );
extern DWORD dwDeviceHandle;


void DFPCancelSettings(void * pInData,void * pOutData)
{
    DFPDEVICEPACKET requestPkt;

    if (_FF(ulDevicesThatHaveTheirStatesSaved) & STB_FUNCTIONACTIVE0_DFP)
    {
        if ((_FF(ulSavedStatesOfSavedDevices) & STB_FUNCTIONACTIVE0_DFP))
            requestPkt.dfpPacketFunc = dfpTurnOnFlatPanel;
        else
            requestPkt.dfpPacketFunc = dfpTurnOffFlatPanel;

		VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
						H3VDD_DFP_REQUEST, 1, &requestPkt);

		// send the monitor change message so modelist gets updated
		CM_Broadcast_Device_Change_Message (DBT_MONITORCHANGE, 0,
											 CM_BROADCAST_SEND);
    }
}

int DFPGetProperty(void * pInData,void * pOutData)
{
	switch (((STB_PROPERTY *)pInData)->ulPropertyId)
	{	
	case (STB_DFP_CAPS):
		{
		/* initialise output structure */
        ((STB_PROPERTY *)pOutData)->ulResult=STB_ESCAPE_HANDLED;
		((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
		((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
		
		((STB_PROPERTY *)pOutData)->ulPropertyValue = 0UL; //STB_DFPCAPS0_AUTOSCALE;

		return (STB_ESCAPE_HANDLED);
		}


   	case (STB_DFP_DISPLAYCONTROL):
		{
		/* initialise output structure */
        ((STB_PROPERTY *)pOutData)->ulResult=STB_ESCAPE_HANDLED;
		((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
		((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;

        ((STB_PROPERTY *)pOutData)->ulPropertyValue = (DFPisPanelActive()) ? STB_DFP_ON : 0UL;
		return (STB_ESCAPE_HANDLED);
		}


   	case (STB_DFP_DFPCONNECTED):
		{
        /* initialise output structure */
        ((STB_PROPERTY *)pOutData)->ulResult=STB_ESCAPE_HANDLED;
		((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
		((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;

        ((STB_PROPERTY *)pOutData)->ulPropertyValue = (DFPisPanelPresent()) ? STB_DFP_CONNECTED : 0UL;
		return (STB_ESCAPE_HANDLED);
		}


	  case (STB_DFP_AUTOSCALE):
      case (STB_DFP_BRIGHTNESS):
      case (STB_DFP_BRIGHTNESSMAX):
      case (STB_DFP_BRIGHTNESSMIN):
      case (STB_DFP_BRIGHTNESSINC):
      case (STB_DFP_BRIGHTNESSDEC):
      case (STB_DFP_CONTRAST):
      case (STB_DFP_CONTRASTMAX):
      case (STB_DFP_CONTRASTMIN):
      case (STB_DFP_CONTRASTINC):
      case (STB_DFP_CONTRASTDEC):
	  default:
			{
				((STB_PROPERTY *)pOutData)->Guid =  ((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId =  ((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulResult=(DWORD)STB_FEATURE_NOT_SUPPORTED;
				return(STB_ERROR_SETTING_VALUE);
			}
	}
}


int DFPSetProperty(void * pInData,void * pOutData)
{
    DFPDEVICEPACKET requestPkt;

    switch (((STB_PROPERTY *)pInData)->ulPropertyId)
    {	
    case (STB_DFP_DISPLAYCONTROL):

        /* initialise output structure */
        ((STB_PROPERTY *)pOutData)->ulResult=STB_ESCAPE_HANDLED;
        ((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
        ((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;

        // ...WE'RE ABOUT TO CHANGE THE STATE OF THIS DEVICE, SO FIND OUT
        //    WHAT ITS CURRENT STATE IS, AND SAVE IT FOR POSTERITY.

        _FF(ulDevicesThatHaveTheirStatesSaved) |= STB_FUNCTIONACTIVE0_DFP;
        _FF(ulSavedStatesOfSavedDevices) |= ((DFPisPanelActive()) ? STB_FUNCTIONACTIVE0_DFP : 0);

        // ...NOW CHANGE THE STATE OF THE DEVICE.
        if (STB_DFP_ON == ((STB_PROPERTY *)pInData)->ulPropertyValue)
        {
            // ...UH-OH, WE'RE TURNING ON THE DFP.  WE NEED TO TURN OFF THE TV.
            if (_FF(dwTvoActive))
            {
                TVOutDisable( );
            }
            requestPkt.dfpPacketFunc = dfpTurnOnFlatPanel;

        }
        else
        {
            requestPkt.dfpPacketFunc = dfpTurnOffFlatPanel;
        }

        VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
                H3VDD_DFP_REQUEST, 1, &requestPkt);

		// send the monitor change message so modelist gets updated
		CM_Broadcast_Device_Change_Message (DBT_MONITORCHANGE, 0, CM_BROADCAST_SEND);

        return (STB_ESCAPE_HANDLED);

    case (STB_DFP_CAPS):
    case (STB_DFP_DFPCONNECTED):
    case (STB_DFP_AUTOSCALE):
    case (STB_DFP_BRIGHTNESS):
    case (STB_DFP_BRIGHTNESSMAX):
    case (STB_DFP_BRIGHTNESSMIN):
    case (STB_DFP_BRIGHTNESSINC):
    case (STB_DFP_BRIGHTNESSDEC):
    case (STB_DFP_CONTRAST):
    case (STB_DFP_CONTRASTMAX):
    case (STB_DFP_CONTRASTMIN):
    case (STB_DFP_CONTRASTINC):
    case (STB_DFP_CONTRASTDEC):
    default:
        ((STB_PROPERTY *)pOutData)->Guid =  ((STB_PROPERTY *)pInData)->Guid;
        ((STB_PROPERTY *)pOutData)->ulPropertyId =  ((STB_PROPERTY *)pInData)->ulPropertyId;
        ((STB_PROPERTY *)pOutData)->ulResult=(DWORD)STB_FEATURE_NOT_SUPPORTED;
        return(STB_ERROR_SETTING_VALUE);
    }
}

int DFPGetGroupProperty(void * pInData, void * pOutData)
{
	switch (((STB_GROUPPROPERTY *)pInData)->ulPropertyId)
	{
	   default:
	   {
		   ((STB_GROUPPROPERTY *)pOutData)->Guid = ((STB_GROUPPROPERTY *)pInData)->Guid;
		   ((STB_GROUPPROPERTY *)pOutData)->ulPropertyId =  ((STB_GROUPPROPERTY *)pInData)->ulPropertyId;
		   ((STB_GROUPPROPERTY *)pOutData)->ulPropertySize = ((STB_GROUPPROPERTY *)pInData)->ulPropertySize;
		   ((STB_GROUPPROPERTY *)pOutData)->ulResult=(DWORD)STB_FEATURE_NOT_SUPPORTED;
		   return (STB_ESCAPE_NOT_SUPPORTED);
	   }
	}
}

int DFPSetGroupProperty(void * pInData, void * pOutData)
{
	switch (((STB_GROUPPROPERTY *)pInData)->ulPropertyId)
	{
   	default:
		{
			((STB_GROUPPROPERTY *)pOutData)->Guid = ((STB_GROUPPROPERTY *)pInData)->Guid;
			((STB_GROUPPROPERTY *)pOutData)->ulPropertyId =  ((STB_GROUPPROPERTY *)pInData)->ulPropertyId;
			((STB_GROUPPROPERTY *)pOutData)->ulPropertySize = ((STB_GROUPPROPERTY *)pInData)->ulPropertySize;
			((STB_GROUPPROPERTY *)pOutData)->ulResult=(DWORD)STB_FEATURE_NOT_SUPPORTED;
			return (STB_ESCAPE_NOT_SUPPORTED);
		}
	}
}

void DFPgetUniqueName(char * namePtr )
{
    DFPDEVICEPACKET requestPkt;

    /* check if dfp is presently connected */

    requestPkt.dfpPacketFunc = dfpGetPanelUniqueName;
    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
                H3VDD_DFP_REQUEST, 1, &requestPkt);

    _fmemcpy(namePtr, requestPkt.dfpOptData.uniqueName, UNIQUENAMELEN);
}

BOOL DFPisPanelPresent()
{
    DFPDEVICEPACKET requestPkt;

    /* check if dfp is presently connected */

    requestPkt.dfpPacketFunc = dfpGetFlatPanelStatus;
    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
                H3VDD_DFP_REQUEST, 1, &requestPkt);

    return (requestPkt.dfpOptData.dfpStatusResponse.panelPresent);
}

BOOL DFPisPanelActive()
{
    DFPDEVICEPACKET requestPkt;

    /* check if dfp is presently active */

    requestPkt.dfpPacketFunc = dfpGetFlatPanelStatus;
    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
                H3VDD_DFP_REQUEST, 1, &requestPkt);

    return (requestPkt.dfpOptData.dfpStatusResponse.panelActive);
}

BOOL DFPisPanelScaling()
{
    DFPDEVICEPACKET requestPkt;

    /* check if dfp is presently active */

    requestPkt.dfpPacketFunc = dfpGetFlatPanelStatus;
    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
                H3VDD_DFP_REQUEST, 1, &requestPkt);

    return (requestPkt.dfpOptData.dfpStatusResponse.panelScaling);
}

BOOL DFPisAdapterDfpCapable()
{
    DFPDEVICEPACKET requestPkt;

    /* check if dfp is presently active */

    requestPkt.dfpPacketFunc = dfpGetFlatPanelStatus;
    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
                H3VDD_DFP_REQUEST, 1, &requestPkt);

    return (requestPkt.dfpOptData.dfpStatusResponse.dfpCapable);
}
//EOF

