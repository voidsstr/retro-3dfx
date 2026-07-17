/* -*-c++-*- */
/* $Header: dfpapi.c, 6, 10/11/00 8:45:20 PM, Brent$
/*
** Copyright (c) 1999,2000 3Dfx Interactive, Inc.
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
** $Revision: 6$
** $Date: 10/11/00 8:45:20 PM$
**
** $History: dfpapi.c $
** 
** 
**
** 
*/


#include "precomp.h"
#include "dfpapi.h"
#include "edgedefs.h"
#include "edgecaps.h"

// driver specific includes below 

#include "dfpctrl.h"
#include "funcapi.h"

extern void TVOutDisable( PDEV *ppdev );



void DFPCancelSettings(void * pInData,void * pOutData, PDEV * ppdev)
{
    DFPDEVICEPACKET requestPkt;
    ULONG ulTemp;

    if (ppdev->ulDevicesThatHaveTheirStatesSaved & STB_FUNCTIONACTIVE0_DFP)
    {
        if (ppdev->ulSavedStatesOfSavedDevices & STB_FUNCTIONACTIVE0_DFP)
            requestPkt.dfpPacketFunc = dfpTurnOnFlatPanel;
        else
            requestPkt.dfpPacketFunc = dfpTurnOffFlatPanel;

        EngDeviceIoControl(ppdev->hDriver,
                IOCTL_DFP_TRANSACTION,
                &requestPkt,
                (sizeof(requestPkt.dfpPacketFunc)+sizeof(requestPkt.dfpOptData.dfpControl)),
                NULL,
                0,
                &ulTemp);

#if 0 // don't know how to port this function from Win9x
        // send the monitor change message so modelist gets updated
        CM_Broadcast_Device_Change_Message (DBT_MONITORCHANGE, 0, CM_BROADCAST_SEND);
#endif
    }
}

int DFPGetProperty(void * pInData,void * pOutData, PDEV * ppdev)
{
    DFPSTATUS dfpStat;
    DFPDEVICEPACKET requestPkt;
    ULONG ulTemp;

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

            /* check if dfp is presently turned on */
            ((STB_PROPERTY *)pOutData)->ulPropertyValue = (DFPisPanelActive(ppdev)) ? STB_DFP_ON : 0UL;
            return (STB_ESCAPE_HANDLED);
        }


    case (STB_DFP_DFPCONNECTED):
        {


            /* initialise output structure */
            ((STB_PROPERTY *)pOutData)->ulResult=STB_ESCAPE_HANDLED;
            ((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
            ((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;

            /* check if dfp is presently connected */
            ((STB_PROPERTY *)pOutData)->ulPropertyValue = (DFPisPanelPresent(ppdev)) ? STB_DFP_CONNECTED : 0UL;
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


int DFPSetProperty(void * pInData, void * pOutData, PDEV * ppdev)
{
    DFPSTATUS dfpStat;
    DFPDEVICEPACKET requestPkt;
    ULONG ulTemp;

    switch (((STB_PROPERTY *)pInData)->ulPropertyId)
    {	
    case (STB_DFP_DISPLAYCONTROL):
        {

            /* initialise output structure */
            ((STB_PROPERTY *)pOutData)->ulResult=STB_ESCAPE_HANDLED;
            ((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
            ((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;

            // ...WE'RE ABOUT TO CHANGE THE STATE OF THIS DEVICE, SO FIND OUT
            //    WHAT ITS CURRENT STATE IS, AND SAVE IT FOR POSTERITY.
            requestPkt.dfpPacketFunc = dfpGetFlatPanelStatus;
            EngDeviceIoControl(ppdev->hDriver,
                    IOCTL_DFP_TRANSACTION,
                    &requestPkt,
                    (sizeof(requestPkt.dfpPacketFunc)+sizeof(requestPkt.dfpOptData.dfpControl)),
                    &dfpStat,
                    sizeof(dfpStat),
                    &ulTemp);

            ppdev->ulDevicesThatHaveTheirStatesSaved |= STB_FUNCTIONACTIVE0_DFP;
            ppdev->ulSavedStatesOfSavedDevices |= ((dfpStat.panelActive) ? STB_FUNCTIONACTIVE0_DFP : 0);

            // ...NOW CHANGE THE STATE OF THE DEVICE.
            if (STB_DFP_ON == ((STB_PROPERTY *)pInData)->ulPropertyValue)
            {
///???TBD remove following code that turns off TV when 3dfx Tools supports new mechanism for turning on DFP.
                // ...UH-OH, WE'RE TURNING ON THE DFP.  WE NEED TO TURN OFF THE TV.
                if (ppdev->dwTvoActive)
                {
                    TVOutDisable( ppdev );
                }
                requestPkt.dfpPacketFunc = dfpTurnOnFlatPanel;
            }
            else
                requestPkt.dfpPacketFunc = dfpTurnOffFlatPanel;

            EngDeviceIoControl(ppdev->hDriver,
                    IOCTL_DFP_TRANSACTION,
                    &requestPkt,
                    (sizeof(requestPkt.dfpPacketFunc)+sizeof(requestPkt.dfpOptData.dfpControl)),
                    NULL,
                    0,
                    &ulTemp);

#if 0 // don't know how to port this function from Win9x
            // send the monitor change message so modelist gets updated
            CM_Broadcast_Device_Change_Message (DBT_MONITORCHANGE, 0, CM_BROADCAST_SEND);
#endif
            return (STB_ESCAPE_HANDLED);
        }

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
        {
            ((STB_PROPERTY *)pOutData)->Guid =  ((STB_PROPERTY *)pInData)->Guid;
            ((STB_PROPERTY *)pOutData)->ulPropertyId =  ((STB_PROPERTY *)pInData)->ulPropertyId;
            ((STB_PROPERTY *)pOutData)->ulResult=(DWORD)STB_FEATURE_NOT_SUPPORTED;
            return(STB_ERROR_SETTING_VALUE);
        }
    }
}

int DFPGetGroupProperty(void * pInData,void * pOutData, PDEV * ppdev)
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

int DFPSetGroupProperty(void * pInData,void * pOutData, PDEV * ppdev)
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


BOOL DFPisPanelPresent(PDEV * ppdev)
{
    DFPSTATUS dfpStat;
    DFPDEVICEPACKET requestPkt;
    ULONG ulTemp;

    /* check if dfp is presently connected */

    requestPkt.dfpPacketFunc = dfpGetFlatPanelStatus;
    EngDeviceIoControl(ppdev->hDriver,
            IOCTL_DFP_TRANSACTION,
            &requestPkt,
            (sizeof(requestPkt.dfpPacketFunc)+sizeof(requestPkt.dfpOptData.dfpControl)),
            &dfpStat,
            sizeof(dfpStat),
            &ulTemp);

    return (dfpStat.panelPresent);
}

BOOL DFPisPanelActive(PDEV * ppdev)
{
    DFPSTATUS dfpStat;
    DFPDEVICEPACKET requestPkt;
    ULONG ulTemp;

    /* check if dfp is presently connected */

    requestPkt.dfpPacketFunc = dfpGetFlatPanelStatus;
    EngDeviceIoControl(ppdev->hDriver,
            IOCTL_DFP_TRANSACTION,
            &requestPkt,
            (sizeof(requestPkt.dfpPacketFunc)+sizeof(requestPkt.dfpOptData.dfpControl)),
            &dfpStat,
            sizeof(dfpStat),
            &ulTemp);

    return (dfpStat.panelActive);
}

BOOL DFPisAdapterDfpCapable(PDEV * ppdev)
{
    DFPSTATUS dfpStat;
    DFPDEVICEPACKET requestPkt;
    ULONG ulTemp;

    /* check if this adapter can support a dfp */

    requestPkt.dfpPacketFunc = dfpGetFlatPanelStatus;
    EngDeviceIoControl(ppdev->hDriver,
            IOCTL_DFP_TRANSACTION,
            &requestPkt,
            (sizeof(requestPkt.dfpPacketFunc)+sizeof(requestPkt.dfpOptData.dfpControl)),
            &dfpStat,
            sizeof(dfpStat),
            &ulTemp);

    return (dfpStat.dfpCapable);
}


#if 0
void  DFPInitialization(  ppdev )
{
    // initialize variable in the pDev
    ppdev->bDfpActive = DFPisPanelActive( ppdev );
      
}
#endif





