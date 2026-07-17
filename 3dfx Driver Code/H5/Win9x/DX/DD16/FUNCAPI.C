/* -*-c++-*- */
/* $Header: funcapi.c, 24, 10/16/00 3:47:03 PM, Geoff Bullard$ */
/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** File name:   funcapi.c
**
** Description: Provide Functionality to check status & capabilities of card.
**
** $Revision: 24$
** $Date: 10/16/00 3:47:03 PM$
**
** $Log: 
**  24   3dfx      1.15.1.7    10/16/00 Geoff Bullard   Implement
**       SET_ALTERNATE_DISPLAYS escape thru FunctionAPI.  This is a generic
**       interface to turn on/off TVOUT/DFP/CRT.
**  23   3dfx      1.15.1.6    10/16/00 Matt McClure    Added changes for glide and
**       4 chip boards.
**  22   3dfx      1.15.1.5    10/12/00 Reid Campbell   Add API version for 3dfx
**       Tools to check
**  21   3dfx      1.15.1.4    10/11/00 Brent           Forced check in to enforce
**       branching.
**  20   3dfx      1.15.1.3    10/03/00 Matt McClure    Added code to support
**       detection of the VIA Chipset for 3dfx Tools
**  19   3dfx      1.15.1.2    09/07/00 Reid Campbell   Updated to switch off
**       tweaks base on PCI and CPU type
**  18   3dfx      1.15.1.1    08/07/00 Reid Campbell   Added driver ability to
**       select different Video refresh optimization Tweaks based on number of
**       chips.
**  17   3dfx      1.15.1.0    07/07/00 Dan O'Connel     Report "DFP CAPS" to 3dfx
**       Tools when adapter is DFP capable instead of when DFP is present.
**  16   3dfx      1.15        03/21/00 Reid Campbell   Added default name for
**       Voodoo3 and min clock freq now 100 MHz
**  15   3dfx      1.14        03/16/00 Reid Campbell   Now writes the product name
**       to the registry
**  14   3dfx      1.13        03/16/00 Reid Campbell   Added check for Voodoo3 to
**       switch off all SLI/AA tweaks
**  13   3dfx      1.12        02/23/00 Dan O'Connel    Minor clean up of DFP code
**       to prepare for future work.
**  12   3dfx      1.11        02/17/00 Dan O'Connel    Don't allow simultanious
**       TvOut and CRT monitor output on Napalm.  As suggested by Kyle Pratt.
**  11   3dfx      1.10        02/14/00 Reid Campbell   Added config for Glide and
**       SLI/AA
**  10   3dfx      1.9         02/08/00 Reid Campbell   New interface added for
**       over clocking.
**  9    3dfx      1.8         01/25/00 Dan O'Connel    Change mechanism used to
**       detect if board is TvOut capable from kludgy Voodoo3 specific mechanism to
**       a more generic mechanism that works for Napalm also.  This mechanism was
**       borrowed from the WinNT/Win2k driver.
**  8    3dfx      1.7         01/11/00 Reid Campbell   Will now only write disable
**       key to registry for AA/SLI tweaks if the tweak already exists.
**  7    3dfx      1.6         12/01/99 Reid Campbell   Disables different Tweaks
**       based on the number of chips on the card
**  6    3dfx      1.5         10/22/99 Scott Kephart   
**  5    3dfx      1.4         09/29/99 Steve Rogers    Porting Suresh Nadella's
**       fix from OEM tree for PRS 8829: This function call has a bug on Packard
**       Bell's new motherboard upon returning from Power standby mode. It doesnt
**       look like we need to verify the Sub Vendor Id for V3 since we are already
**       verifying the Vendor ID and only 3dfx makes V3 boards now.
**  4    3dfx      1.3         09/28/99 Reid Campbell   Header changed to standard
**       3dfx format.
**  3    3dfx      1.2         09/28/99 Reid Campbell   Removed redundant code
**       which wrote to the registry if card didn't support TV or LCD 
**  2    3dfx      1.1         09/24/99 Dale  Kenaston  Modified
**       FunctionAPIGetProperty and FunctionAPIInit to look at _FF(dwDFPState)
**       instead of board desc table when looking for DFP support. Renamed
**       dwLcdActive to dwDFPActive and use bit 1.
** 
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $ 
*/


#define _TEXT(x) x
#include "funcapi.h"
#include "edgedefs.h"
#include "edgecaps.h"

// driver specific includes below 
#include "header.h"

#define Not_VxD
#include <vmm.h>

#define MIDL_PASS     // suppress 32-bit only #pragma pack(push)
#pragma warning (disable: 4047 4704)
#include <configmg.h>
#pragma warning (default: 4047 4704)

#include "h3g.h"
#include "modelist.h"
#include "qmodes.h"	/* for LPQIN Structure */
#include "tv.h"
#include "dfpapi.h"

#define Not_VxD
#include "minivdd.h"

#include <string.h>
#include <pci.h>

#define	FREF	143184   //Fref*10000 

#define PCI_SSVID	0x2C	//Subsystem Vendor ID
#define PCI_VID		0x00	//Vendor ID
#define STB_SSVENDOR_ID					(0x000010b4L)
#define TDFX_SSVENDOR_ID				(0x0000121aL)
#define BUS_MASK	0x0001
#define	TVO_MASK	0x0002
#define LCD_MASK	0x0004
#define MEM_MASK	0x0008

extern DISPLAYINFO DisplayInfo;
extern DWORD dwDevNode;
extern DWORD dwDeviceHandle;
extern long atol( const char *string );

extern void TVOutGetStandard( LPQIN lpQIN, LPTVGETSTANDARD lpOutput );
extern void TVOutStatus( LPQIN lpQIN, LPQTVSTATUS lpOutput );


int FunctionAPIGetProperty(void * pInData,void * pOutData)
{
   TVSETSTANDARD Output;
   
   DWORD   grxFreq, grxClock;

	switch (((STB_PROPERTY *)pInData)->ulPropertyId)
		{	
		case (STB_FUNCTION_CAPS):
		{		
			/* initialise output structure */
			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
			((STB_PROPERTY *)pOutData)->ulResult=1;

			/* ensure VGA capabilities are always enabled */
			
			((STB_PROPERTY *)pOutData)->ulPropertyValue=
				((STB_PROPERTY *)pOutData)->ulPropertyValue | STB_FUNCTIONCAPS0_VGA;

			/* check tv out status */
            if (_FF(dwTvoCapable))
            {
                ((STB_PROPERTY *)pOutData)->ulPropertyValue = 
                        ((STB_PROPERTY *)pOutData)->ulPropertyValue | STB_FUNCTIONCAPS0_TV;
            }
            else 
            {
                ((STB_PROPERTY *)pOutData)->ulPropertyValue = 
                        ((STB_PROPERTY *)pOutData)->ulPropertyValue & ~STB_FUNCTIONCAPS0_TV;
            }


			/* check for DFP support */
			
			if (DFPisAdapterDfpCapable())
			{
				((STB_PROPERTY *)pOutData)->ulPropertyValue=
				((STB_PROPERTY *)pOutData)->ulPropertyValue | STB_FUNCTIONCAPS0_DFP;
			}
			else
			{
				((STB_PROPERTY *)pOutData)->ulPropertyValue=
				(((STB_PROPERTY *)pOutData)->ulPropertyValue) & ~STB_FUNCTIONCAPS0_DFP;
			}


			return (STB_ESCAPE_HANDLED);
		}


	case (STB_FUNCTION_ACTIVE):
		{
			QTVSTATUS TVStatus;
			
			QGETSET_MONITOR_CTL Monitor;

			/* initialise output structure */

			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
			((STB_PROPERTY *)pOutData)->ulResult=1;

			/* check if vga monitor is enabled */

			/* QUERY_ANALOG_MONITOR in qmodes.c casts lpQIN to (QGETSET_MONITOR_CTL *) */
			/* which is a struct of DWORD dwSubfunc, DWORD monitorStatus and */
			/* DWORD monitorControl */

			Monitor.dwSubFunc = QUERY_ANALOG_MONITOR;
			Monitor.monitorStatus = 0;
            Monitor.monitorControl = 0;

			QueryMode((LPQIN)&Monitor, (LPVOID)&TVStatus); 
			/* second parameter is ignored for this call */

			if (Monitor.monitorStatus == MONITOR_IS_ENABLED)
			{
				((STB_PROPERTY *)pOutData)->ulPropertyValue |= STB_FUNCTIONACTIVE0_VGA;
			}

			/* check tv out status */

			if (_FF(dwTvoActive))
			{
	  			((STB_PROPERTY *)pOutData)->ulPropertyValue |= STB_FUNCTIONACTIVE0_TV;
			}
		
			/* Check for DFP enabled */

			if (DFPisPanelActive())
      		{
				((STB_PROPERTY *)pOutData)->ulPropertyValue |= STB_FUNCTIONACTIVE0_DFP;
			}

			return(STB_ESCAPE_HANDLED);
		}
	case (STB_SIM_VGA):
        {

            ((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
            ((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
            ((STB_PROPERTY *)pOutData)->ulPropertyValue=0x00;

            /*
            values are currently hard coded until I find a method of deriving them from
            the card or the driver
            */
            if (IS_VOODOO3)
            {
                switch (_FF(customerNumber))
                {
                case 7:  //Mystery customer number 7 wants exclusivity
                    ((STB_PROPERTY *)pOutData)->ulPropertyValue=STB_SIM0_VGA;
                    break;

                default: //Used for retail, and mystery customer number X ("allowPALCRT")
                    Output.dwSubFunc = QUERYGETSTANDARD;
                    TVOutGetStandard( (LPQIN) &Output, (LPTVGETSTANDARD) &Output );
                    // Unless "allowPALCRT" is specified don't allow simultaneous TV/VGA for PAL types.
                    if ((_FF(dwTvoStd) != VP_TV_STANDARD_NTSC_M) && !(_FF(allowPALCRT)))
                    {
                        ((STB_PROPERTY *)pOutData)->ulPropertyValue = STB_SIM0_VGA;
                    }
                    else
                    {
                        // allow simultanious vga/dfp and vga/tv
                        ((STB_PROPERTY *)pOutData)->ulPropertyValue=STB_SIM0_TV | STB_SIM0_DFP | STB_SIM0_VGA;
                    }
                    break;

                }

            }
            else
            {
                // Unless "allowPALCRT" is specified don't allow simultaneous TV/VGA for either PAL or NTSC.
                // Note: "allowPALCRT" allows simultaneous TV/VGA for both PAL and NTSC even though the name
                // is misleading.
                if (!(_FF(allowPALCRT)))
                {
                    // allow simultanious vga/dfp
                    ((STB_PROPERTY *)pOutData)->ulPropertyValue = STB_SIM0_DFP | STB_SIM0_VGA;
                }
                else
                {
                    // allow simultanious vga/dfp and vga/tv
                    ((STB_PROPERTY *)pOutData)->ulPropertyValue=STB_SIM0_TV | STB_SIM0_DFP | STB_SIM0_VGA;
                }
            }
            ((STB_PROPERTY *)pOutData)->ulResult=(DWORD) STB_ESCAPE_HANDLED;
            return (STB_ESCAPE_HANDLED);
        }

    case (STB_SIM_TV):
        {
            STB_PROPERTY	property, returnproperty;

            ((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
            ((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;

            property.Guid=((STB_PROPERTY *)pInData)->Guid;
            property.ulPropertyId=STB_FUNCTION_CAPS;

            /* do escape call to see if there is any point in proceding */

            FunctionAPIGetProperty((void *)&property,(void *) &returnproperty);	

            if  ((returnproperty.ulResult == STB_ESCAPE_HANDLED) &&
                    (returnproperty.ulPropertyValue & STB_FUNCTIONCAPS0_TV))
            {
                if (IS_VOODOO3)
                {
                    switch (_FF(customerNumber))
                    {
                    case 7:  //Mystery customer number 7 wants exclusivity
                        ((STB_PROPERTY *)pOutData)->ulPropertyValue = (DWORD)(STB_SIM0_TV);
                        break;

                    default: //Used for retail, and mystery customer number X ("allowPALCRT")
                        Output.dwSubFunc = QUERYGETSTANDARD;
                        TVOutGetStandard( (LPQIN) &Output, (LPTVGETSTANDARD) &Output );
                        // Unless "allowPALCRT" is specified don't allow simultaneous TV/VGA for PAL types.
                        if ((_FF(dwTvoStd) != VP_TV_STANDARD_NTSC_M) && !(_FF(allowPALCRT)))
                        {
                            ((STB_PROPERTY *)pOutData)->ulPropertyValue = (DWORD)(STB_SIM0_TV);
                        }
                        else
                        {
                            ((STB_PROPERTY *)pOutData)->ulPropertyValue = (DWORD)(STB_SIM0_VGA | STB_SIM0_TV | !STB_SIM0_DFP) ;
                        }
                        break;
                    }
                }
                else
                {
                    // Unless "allowPALCRT" is specified don't allow simultaneous TV/VGA for either PAL or NTSC.
                    // Note: "allowPALCRT" allows simultaneous TV/VGA for both PAL and NTSC even though the name
                    // is misleading.
                    if (!(_FF(allowPALCRT)))
                    {
                        ((STB_PROPERTY *)pOutData)->ulPropertyValue = (DWORD)(STB_SIM0_TV);
                    }
                    else
                    {
                        ((STB_PROPERTY *)pOutData)->ulPropertyValue = (DWORD)(STB_SIM0_VGA | STB_SIM0_TV | !STB_SIM0_DFP) ;
                    }
                }

                ((STB_PROPERTY *)pOutData)->ulResult=(DWORD) STB_ESCAPE_HANDLED;
            }
            else
            {	/* if tv is not supported return error code */
                ((STB_PROPERTY *)pOutData)->ulPropertyValue = 0x00;
                return (STB_FEATURE_NOT_SUPPORTED);
            }

            return (STB_ESCAPE_HANDLED);
        }

    case (STB_SIM_DFP):
		{
			STB_PROPERTY	property, returnproperty;
			
			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			
			property.Guid=((STB_PROPERTY *)pInData)->Guid;
			property.ulPropertyId=STB_FUNCTION_CAPS;

			/* do escape call to see if there is any point in proceding */

			FunctionAPIGetProperty((void *)&property,(void *) &returnproperty);	
			
			if  ((returnproperty.ulResult == STB_ESCAPE_HANDLED) &&
				(returnproperty.ulPropertyValue & STB_FUNCTIONCAPS0_DFP))
			{
               ((STB_PROPERTY *)pOutData)->ulPropertyValue = (DWORD)(STB_SIM0_VGA | !STB_SIM0_TV | STB_SIM0_DFP);
				((STB_PROPERTY *)pOutData)->ulResult=(DWORD) STB_ESCAPE_HANDLED;
			}
			else
			{	/* if tv is not supported return error code */
				((STB_PROPERTY *)pOutData)->ulPropertyValue = 0x00;
				return (STB_FEATURE_NOT_SUPPORTED);
			}
			return (STB_ESCAPE_HANDLED);
           
	case (STB_FUNCTION_CLOCKRATE): // Return the current clock rate	in MHz
		{		
			/* intitialise output structure */
			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
			((STB_PROPERTY *)pOutData)->ulResult=1;

            grxClock = GET(lph3IORegs->pllCtrl1);
	         
	        grxFreq = PLL2MHz((DWORD)grxClock) / 10000;	
          
          			
			((STB_PROPERTY *)pOutData)->ulPropertyValue = grxFreq ;

		    return (STB_ESCAPE_HANDLED);
		}
       
	case (STB_FUNCTION_CLOCKDEFAULT): // Return the default clock rate in MHz

		{		
			/* intitialise output structure */
			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulResult=1;

			((STB_PROPERTY *)pOutData)->ulPropertyValue = PLL2MHz(_FF(dwDefaultClock)) / 10000;
           
			return (STB_ESCAPE_HANDLED);
		}
       
       
	case (STB_FUNCTION_CLOCKMAX): // Return the Max clock rate in MHz

		{		
			/* intitialise output structure */
			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulResult=1;

			((STB_PROPERTY *)pOutData)->ulPropertyValue = 220; // This should have been got from the MiniVDD .h files, but
           												   // these files define an array for the clock speeds, so the 
                                                              // .h files can only be called once in a module 


			return (STB_ESCAPE_HANDLED);
		}

	case (STB_FUNCTION_CLOCKMIN): // Return the Min clock rate in MHz

		{		
			/* intitialise output structure */
			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulResult=1;

			((STB_PROPERTY *)pOutData)->ulPropertyValue = 100;  // This should have been got from the MiniVDD .h files, but
           												   // these files define an array for the clock speeds, so the 
                                                              // .h files can only be called once in a module 


			return (STB_ESCAPE_HANDLED);
		}
       
       
	case (STB_DISPLAY_ENUM):
// *********************************************************
// Property name : STB_DISPLAY_ENUM
// Gets the registry location of the driver from the DevNode
// The location is of the form :
// HKLM\System\CurrentControlSet\Class\Display\xxxx
// We then get the value represented by XXXX, convert it to a DWORD
// and return this in ulPropertyValue
// *********************************************************
		{
			char szBuffer[255];
			char szDisplayEnum[255];
			int iSize;
			int i,j;
			int iResult;
			long dwEnum; 
			DWORD	dwVendorID, dwDeviceID;
			/* DWORD dwSubVendorID;  Not needed.  See SNadella's comment below*/

			/* 
				First of all do the checks to see if it is an STB or 3DFX card
			*/

			CM_Call_Enumerator_Function( _FF(DevNode),
				     PCI_ENUM_FUNC_GET_DEVICE_INFO,
				     PCI_VID, &dwVendorID, sizeof(DWORD), 0 );

			dwDeviceID = dwVendorID>>16;

			if(dwDeviceID < 3)
			{
				((STB_GROUPPROPERTY *)pOutData)->ulResult=(DWORD)STB_NON_3DFX_CARD;
				return (STB_NON_3DFX_CARD);
			}

			dwVendorID &= 0x0000ffff; 

			if (! ((dwVendorID == STB_SSVENDOR_ID) || (dwVendorID == TDFX_SSVENDOR_ID)) )
			{
				((STB_GROUPPROPERTY *)pOutData)->ulResult=(DWORD)STB_NON_3DFX_CARD;
				return (STB_NON_3DFX_CARD);
			}

			/*
				Get SubVendor ID to ensure that it is a 3Dfx or STB board
			*/
            /*
                    Get SubVendor ID to ensure that it is a 3Dfx or STB board
            */
            /*  SNadella: PRS 8829 This function call has a bug
                          on Packard Bell's new motherboard upon
                          returning from Power standby mode. It doesnt
                          look like we need to verify the Sub Vendor Id for
                          V3 since we are already verifying the Vendor ID and
                          only 3dfx makes V3 boards now
            */
            /*
			CM_Call_Enumerator_Function( _FF(DevNode),
				     PCI_ENUM_FUNC_GET_DEVICE_INFO,
				     PCI_SSVID, &dwSubVendorID, sizeof(DWORD), 0 );


			dwSubVendorID &= 0x0000ffff; 
			if (!(	( dwSubVendorID == STB_SSVENDOR_ID ) || 
					( dwSubVendorID == TDFX_SSVENDOR_ID ) ||
					( dwSubVendorID == 0xFFFF )	// stuck in due to there being too many feckin' reference boards!!
					))

			{
				// this is not one of our cards
				((STB_PROPERTY *)pOutData)->ulResult = (DWORD)STB_NON_3DFX_CARD; 
				return (STB_NON_3DFX_CARD);
			}
			*/


			iResult = CM_Get_DevNode_Key(	DisplayInfo.diDevNodeHandle, 
											NULL, 
											&szBuffer, 
											sizeof(szBuffer), 
											CM_REGISTRY_SOFTWARE);

		// Test the value of iResult and the length of szBuffer for
		// the possible chance of an error occuring
	
		// Get the last 4 characters of the string (plus the null terminator) 
		// - these represent the number that we want 

			iSize=lstrlen(szBuffer);
			j=0; 
			for(i=(iSize-4);i<(iSize+1);i++)
			{
				szDisplayEnum[j]=szBuffer[i];
				j++;
			}
			dwEnum = atol(szDisplayEnum);
			DPF(DBGLVL_ALL,"Enum = %d ", dwEnum);

			((STB_PROPERTY *)pOutData)->Guid =  ((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulResult = STB_ESCAPE_HANDLED; 
			((STB_PROPERTY *)pOutData)->ulPropertyId = STB_DISPLAY_ENUM;
			((STB_PROPERTY *)pOutData)->ulPropertyValue = (DWORD)dwEnum;
			return (STB_ESCAPE_HANDLED);
			}
		default:
			{
				((STB_PROPERTY *)pOutData)->Guid =  ((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId =  ((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulResult=(DWORD)STB_FEATURE_NOT_SUPPORTED;
				return(STB_ERROR_SETTING_VALUE);
			}
		}
    }
}

void TVOutCancelSettings(void * pInData,void * pOutData);

int FunctionAPISetProperty(void * pInData,void * pOutData)
{
   ((STB_PROPERTY *)pOutData)->Guid =  ((STB_PROPERTY *)pInData)->Guid;
   ((STB_PROPERTY *)pOutData)->ulPropertyId =  ((STB_PROPERTY *)pInData)->ulPropertyId;
   ((STB_PROPERTY *)pOutData)->ulResult=(DWORD)STB_ESCAPE_HANDLED;

  	switch (((STB_GROUPPROPERTY *)pInData)->ulPropertyId)
	{
      case (STB_FUNCTION_CANCEL):
         DFPCancelSettings(pInData, pOutData);
         TVOutCancelSettings(pInData, pOutData);
         _FF(ulDevicesThatHaveTheirStatesSaved) = 0UL;
         return (STB_ESCAPE_HANDLED);

      case (STB_FUNCTION_COMMIT):
         _FF(ulDevicesThatHaveTheirStatesSaved) = 0UL;
         return (STB_ESCAPE_HANDLED);

      case (STB_FUNCTION_ACTIVE):
      {
          DWORD dwReturned = 0;
          DWORD dwSetting = 0x0000;
          DWORD dwMask;

          // build up an all 3 correct DWORD 
          dwMask = STB_FUNCTION_VGA | STB_FUNCTION_TV | STB_FUNCTION_DFP;

          dwSetting |= (((STB_PROPERTY *)pInData)->ulPropertyValue) & dwMask;

          /*	Call the miniport driver passing down a DWORD that contains the flags
          for which display outputs are to be activated and which are
          to be disabled.

          Return code returned in dwReturned
                
              0x001	= sucessfully changed VGA output
              0x002   = sucessfully changed TV output
              0x004   = sucessfully changed DFP output  
          */

		  VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
              H3VDD_SET_ALT_DISPLAYS, 0, &dwSetting);

          /* initialise output data */
          ((STB_PROPERTY *)pOutData)->Guid =  ((STB_PROPERTY *)pInData)->Guid;

          ((STB_PROPERTY *)pOutData)->ulPropertyId = ((STB_PROPERTY *)pInData)->ulPropertyId;
          /* if we activated/deactivated all 3 correctly */
          if (dwSetting == dwMask)
          {
              ((STB_PROPERTY *)pOutData)->ulPropertyValue = ((STB_PROPERTY *)pInData)->ulPropertyValue;
              ((STB_PROPERTY *)pOutData)->ulResult = (DWORD)STB_ESCAPE_HANDLED;
              //return (STB_ESCAPE_HANDLED);
          }
          /* if we couldn't activate/deactivate all 3 correctly */
          else
          {
              ((STB_PROPERTY *)pOutData)->ulPropertyValue =  (dwReturned & STB_FUNCTION_VGA);
              ((STB_PROPERTY *)pOutData)->ulPropertyValue |= (dwReturned & STB_FUNCTION_TV);
              ((STB_PROPERTY *)pOutData)->ulPropertyValue |= (dwReturned & STB_FUNCTION_DFP);
              ((STB_PROPERTY *)pOutData)->ulResult=(DWORD)STB_CHANGE_MODE;
              //return (STB_ESCAPE_HANDLED);
          }
          return (STB_ESCAPE_HANDLED);
      }

   	default:
	/*
		*************************
		Function API is Read Only
		************************* 
	*/
	((STB_PROPERTY *)pOutData)->Guid =  ((STB_PROPERTY *)pInData)->Guid;
	((STB_PROPERTY *)pOutData)->ulPropertyId =  ((STB_PROPERTY *)pInData)->ulPropertyId;
	((STB_PROPERTY *)pOutData)->ulResult=(DWORD)STB_FEATURE_NOT_SUPPORTED;
	return (STB_ESCAPE_NOT_SUPPORTED);
}
}

int FunctionAPIGetGroupProperty(void * pInData,void * pOutData)
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

int FunctionAPISetGroupProperty(void * pInData,void * pOutData)
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

int FunctionAPIInit()
{

	QTVSTATUS TVStatus;

#if 0
//??? not ported from WinNT
    TDFX_QUERY_VALUE_INFO queryValueInfo;
    DWORD                 numBytes;
#endif

   #define REG_D3D_SINGLE  		 "D3D\\SingleChipAASLI"
   #define REG_D3D_DUAL    		 "D3D\\DualChipAASLI"
   #define REG_D3D_QUAD     		 "D3D\\QuadChipAASLI"
   #define REG_D3D_GEOMETRY_ASSIST  "D3D\\Geometry Assist"    
   
   #define REG_GLIDE_SINGLE   		 "Glide\\SingleChipAASLI"
   #define REG_GLIDE_DUAL     		 "Glide\\DualChipAASLI"
   #define REG_GLIDE_QUAD     		 "Glide\\QuadChipAASLI"

   #define REG_GLIDE_HSR             "Glide\\Hidden Surface Removal"
   
   #define REG_DEFAULT_REFRESH_OPT	 "Default\\VideoRefresh"
   #define REG_DEFAULT_2REFRESH_OPT "Default\\VideoRefresh2"
   
   #define REG_DEFAULT_COMMAND_FIFO "Default\\AGP Command FIFO"
   #define REG_DEFAULT_VIA_CHIPSET  "Default\\VIA Chipset"
   
   #define REG_DISABLED "Disabled"
   #define REG_CONTROL  "Control"
   #define REG_ENABLE	 "0"
   #define REG_DISABLE	 "2"
   
   #define CPUTYPE _FF(cpuType)

   
   char tempStr[256];  
   DWORD length = sizeof(tempStr); 
   
   PFARVOID singleStatus; 
   PFARVOID dualStatus;
   PFARVOID quadStatus;
   
   PFARVOID refreshStatus;
   PFARVOID refresh2Status;
   
   
   DWORD    chips;

   DWORD	DevNode = _FF(DevNode);
   
   QIN				Qin;			// used to access functions in qmodes.h
	QGETOEMBOARDNAME OemBoardName;
   

   /* Write out the product name */

   if (IS_VOODOO3)
   {	/* Just write out the generic name for Voodoo3 cards */
   	lstrcpy((LPSTR)&tempStr, (LPSTR)"Voodoo3 ");
   }
   else
   {	/* Get the name from the BIOS */
    	Qin.dwSubFunc = QUERYGETOEMBOARDNAME;
		QueryMode((LPQIN)&Qin, (LPVOID)&OemBoardName);
   
		lstrcpy((LPSTR)&tempStr, (LPSTR)&OemBoardName);
   }	
   
   if (_FF(AGPCaps & IS_AGP_CARD)) 
	   	lstrcat((LPSTR)&tempStr,(LPCSTR)"AGP");
	else 
		lstrcat((LPSTR)&tempStr,(LPCSTR)"PCI");

   
   CM_Write_Registry_Value(DevNode, "", (PFARCHAR) "DriverDesc", 
						   	REG_SZ, (PFARCHAR) &tempStr, sizeof(tempStr), 
						   	CM_REGISTRY_SOFTWARE);
	
   /* Write out the 3dxfTools API version */
   
   CM_Write_Registry_Value(DevNode, "Default", (PFARCHAR) EDGE_API_VERSION_REG, 
						   	REG_SZ, (PFARCHAR) EDGE_API_VERSION, sizeof(EDGE_API_VERSION), 
						   	CM_REGISTRY_SOFTWARE);
                              
                   
#if defined(TnL_HAL)
                   
  
    /* Turn off Tweaks which will not work on a PII */
    
/* These defines can be removed and the IF statment updated once cpu.h has beeen update to work with 16 bit code */     
    
#define CPU_FEATURE_MMX_16	   	(1l << 15)  
#define CPU_FEATURE_AMMX_16  	(1l << 16)
#define CPU_FEATURE_3DNOWX_16	(1l << 18)
#define CPU_FEATURE_SSE_16	   	(1l << 19) 
    
    
   if( ((CPUTYPE & CPU_FEATURE_MMX_16) && (CPUTYPE & CPU_FEATURE_SSE_16)) || 									// p3 or better
       ((CPUTYPE & CPU_FEATURE_MMX_16) && (CPUTYPE & CPU_FEATURE_AMMX_16) && (CPUTYPE & CPU_FEATURE_3DNOWX_16)) )	// athlons or better
   {
       if (CM_Read_Registry_Value(DevNode, REG_D3D_GEOMETRY_ASSIST, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_D3D_GEOMETRY_ASSIST, (PFARCHAR) REG_DISABLED, 
									REG_SZ, REG_ENABLE, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
       }        
   }
   else
   {
       if (CM_Read_Registry_Value(DevNode, REG_D3D_GEOMETRY_ASSIST, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_D3D_GEOMETRY_ASSIST, (PFARCHAR) REG_DISABLED, 
									REG_SZ, REG_DISABLE, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
       }        
   }
   
#endif

    /* Turn off AGP specific Tweaks */
                   
  if (_FF(AGPCaps & IS_AGP_CARD))
  {
  		/* AGP card	*/ 
  
       /* Make sure the AGP Command FIFO option is available */
        
        if (CM_Read_Registry_Value(DevNode, REG_DEFAULT_COMMAND_FIFO, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_DEFAULT_COMMAND_FIFO, (PFARCHAR) REG_DISABLED, 
									REG_SZ, REG_ENABLE, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
       }        
  	}
   else
   {
       /* PCI Card */
       /* Turn off AGP Command Fifo Options */
        
        if (CM_Read_Registry_Value(DevNode, REG_DEFAULT_COMMAND_FIFO, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_DEFAULT_COMMAND_FIFO, (PFARCHAR) REG_DISABLED, 
									REG_SZ, REG_DISABLE, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
       }        
   }
  
   /* VIA Core Logic Chipset Determination */
  if (_FF(dwVIACoreLogic))
  {
  		/* VIA Chipset	*/ 
       /* Make sure the VIA Chipset option is available */
        
        if (CM_Read_Registry_Value(DevNode, REG_DEFAULT_VIA_CHIPSET, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_DEFAULT_VIA_CHIPSET, (PFARCHAR) REG_DISABLED, 
									REG_SZ, REG_ENABLE, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
       }        
  	}
   else
   {
   	   /* Not a VIA Chipset*/
       /* Turn off the VIA Chipset option */
        
        if (CM_Read_Registry_Value(DevNode, REG_DEFAULT_VIA_CHIPSET, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_DEFAULT_VIA_CHIPSET, (PFARCHAR) REG_DISABLED, 
									REG_SZ, REG_DISABLE, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
       }        
   }

   /* Turn on different tweaks based on the number of graphic chips */
   
   /* If we are running on a Voodoo3, then switch all the AA/SLI tweaks
      off by making the chip number selection fall through to default 
      condition */
   
   if (IS_VOODOO3)
   {
   	chips = 0;
   }
   else
   {
   	chips = _FF(dwNumUnits);
   }
   
   switch (chips)
	{
   	case 1:
   	{
       	singleStatus   = REG_ENABLE; 
   		dualStatus     = REG_DISABLE;
   		quadStatus     = REG_DISABLE;
           refreshStatus  = REG_ENABLE;
           refresh2Status = REG_DISABLE;
           
   		break;
   	}
       
       case 2:
       {
       	singleStatus   = REG_DISABLE; 
   		dualStatus     = REG_ENABLE;
   		quadStatus     = REG_DISABLE;
           refreshStatus  = REG_DISABLE;
           refresh2Status = REG_ENABLE;
   		break;
       }
       	
       case 4:
       {
       	singleStatus   = REG_DISABLE; 
   		dualStatus     = REG_DISABLE;
   		quadStatus     = REG_ENABLE;
           refreshStatus  = REG_ENABLE;
           refresh2Status = REG_DISABLE;
   		break;

       }
       
       default:
       {
       	singleStatus   = REG_DISABLE; 
   		dualStatus     = REG_DISABLE;
   		quadStatus     = REG_DISABLE;
           refreshStatus  = REG_DISABLE;
           refresh2Status = REG_DISABLE;
           
      
   		break;
       }
   }
   
   
   /* Write out if the registry entries exist. All tweaks will have a Control key - so 
      check for it */
      
   /* D3D First */   
   
   if (CM_Read_Registry_Value(DevNode, REG_D3D_SINGLE, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_D3D_SINGLE, (PFARCHAR) REG_DISABLED, 
									REG_SZ, singleStatus, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
   	}
         
   if (CM_Read_Registry_Value(DevNode, REG_D3D_DUAL, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_D3D_DUAL, (PFARCHAR) REG_DISABLED, 
									REG_SZ, dualStatus, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
   	}
       
   if (CM_Read_Registry_Value(DevNode, REG_D3D_QUAD, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_D3D_QUAD, (PFARCHAR) REG_DISABLED, 
									REG_SZ, quadStatus, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
   	}
       
   /* Then the Gilde */
   
   if (CM_Read_Registry_Value(DevNode, REG_GLIDE_SINGLE, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_GLIDE_SINGLE, (PFARCHAR) REG_DISABLED, 
									REG_SZ, singleStatus, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
   	}
         
   if (CM_Read_Registry_Value(DevNode, REG_GLIDE_DUAL, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_GLIDE_DUAL, (PFARCHAR) REG_DISABLED, 
									REG_SZ, dualStatus, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
   	}
       
   if (CM_Read_Registry_Value(DevNode, REG_GLIDE_QUAD, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_GLIDE_QUAD, (PFARCHAR) REG_DISABLED, 
									REG_SZ, quadStatus, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
   	}

    /* Hidden Surface Removal */
    if (_FF(dwNumUnits) == 4) {
	  /* Only enable Hidden Surface Removal for 4 chip boards */
      if (CM_Read_Registry_Value(DevNode, REG_GLIDE_HSR, REG_CONTROL, REG_SZ,
          (LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	  {
        CM_Write_Registry_Value(DevNode, REG_GLIDE_HSR, (PFARCHAR) REG_DISABLED, 
	   							REG_SZ, REG_ENABLE, sizeof(REG_DISABLE), 
    							CM_REGISTRY_SOFTWARE);
      }        
  	} else {
      if (CM_Read_Registry_Value(DevNode, REG_GLIDE_HSR, REG_CONTROL, REG_SZ,
          (LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
      {
   	    CM_Write_Registry_Value(DevNode, REG_GLIDE_HSR, (PFARCHAR) REG_DISABLED, 
								REG_SZ, REG_DISABLE, sizeof(REG_DISABLE), 
								CM_REGISTRY_SOFTWARE);
      }
    }
       
	// Now the Default area
   
   if (CM_Read_Registry_Value(DevNode, REG_DEFAULT_REFRESH_OPT, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_DEFAULT_REFRESH_OPT, (PFARCHAR) REG_DISABLED, 
									REG_SZ, refreshStatus, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
   	}
       
   if (CM_Read_Registry_Value(DevNode, REG_DEFAULT_2REFRESH_OPT, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_DEFAULT_2REFRESH_OPT, (PFARCHAR) REG_DISABLED, 
									REG_SZ, refresh2Status, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
   	}
       
         

			
    // check tv out status to insure ppdev->dwTvoActive is properly initialized
    // ppdev->dwTvoActive is updated as sideeffect of above TVOutGetStatus call.
    TVOutStatus( NULL /*not used*/, &TVStatus );

    // save TVOut Capable flag to reduce the number of times the this module needs to call TVOutStatus.
    //  Each time TVOutStatus is called the TV screen is garbled.
    if (TV_ENCODER_PRESENT == (TVStatus.dwEncoder & TV_ENCODER_PRESENT))
        _FF(dwTvoCapable) = TRUE;
    else 
        _FF(dwTvoCapable) = FALSE;

#if 0
// not ported from WinNT DanO 1/25/99
    // Check if allowPALCRT registry key for a string exists in registry.  If so allow simultanious PAL and CRT display.
    ppdev->bAllowPALCRT = FALSE;
    // call ioctl to have miniport read data from registry
    queryValueInfo.DataLength = TDFX_MAX_DATA_LENGTH;
    if (! EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_3DFX_QUERY_REGISTRY_VALUE,
                           "allowPALCRT",
                           strlen("allowPALCRT") + 1,
                           &queryValueInfo,
                           sizeof(queryValueInfo),
                           &numBytes))
    {
      ppdev->bAllowPALCRT = TRUE;
    }
#endif

   return(0);
   
}


// Function to convert PLL control reg to Frequency.

FxU32	PLL2MHz(FxU32 clock )
{

	FxU32	MHz, Fref, N, M, P, tmp, Fout, Exp;

	MHz=0; Fref=0; N=0 ; M=0 ;P=0; tmp=0; Fout=0; Exp=1;

	tmp		=	clock;
	P		=	tmp & 0x3;
	tmp		=	clock;
	M		=	(tmp>>2) & 0x3f;
	tmp		=	clock;
	N		=	(tmp>>8) & 0xff;
	
	Exp		=	Exp<<P;
	
	Fout	=	( FREF * (N + 2) ) / ((M + 2) * (Exp));

	return	Fout;

}

 
 

 
 


