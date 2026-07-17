/****************************************************************************/
/*
** Copyright© 1999 STB Systems Inc.  All rights reserved.
** Project			: Edge Tools
** Target Name		: 
** Author #1		: Paul Magee
** Author #2		: 
** Purpose			: Provide Functionality to check status & capabilities of card
** Uses Libraries	: 
** Date Started		: January 99
** Date Completed	: 
** Update History #1: 
**
** $Revision: 16$
** $Date: 10/16/00 3:47:41 PM$
**
** $Log: 
**  16   3dfx      1.9.1.1.1.3 10/16/00 Geoff Bullard   Implement
**       SET_ALTERNATE_DISPLAYS escape thru FunctionAPI.  This is a generic
**       interface to turn on/off TVOUT/DFP/CRT.
**  15   3dfx      1.9.1.1.1.2 10/12/00 Reid Campbell   Added API version for 3dfx
**       Tools to check
**  14   3dfx      1.9.1.1.1.1 10/11/00 Brent           Forced check in to enforce
**       branching.
**  13   3dfx      1.9.1.1.1.0 07/21/00 Dan O'Connel    Pick up changes from Win9x
**       to report DFP-capable to 3dfx Tools instead of DFP-present.
**       Sync. up with other changes made in Win9x driver.
** 
**  12   3dfx      1.9.1.1     05/24/00 Dan O'Connel    Correct a comment.
**  11   3dfx      1.9.1.0     05/19/00 Dan O'Connel    Minor cleanup and minor
**       corrections to TvOut code.  Avoid using QueryMode interface from within
**       DisplayDriver when possible.  For Napalm disallow simultaneous TvOut and
**       CRT output (like Win9x driver).  Remove some unused code.
**  10   3dfx      1.9         02/23/00 Dan O'Connel    Port Reid Campbell's
**       funcapi.c changes to support "3dfx Tools" SLI tweaks from Win9x to
**       Win2K/WinNT4.
**  9    3dfx      1.8         02/17/00 Dan O'Connel    Don't allow simultanious
**       TvOut and CRT monitor output on Napalm.  As suggested by Kyle Pratt.
**  8    3dfx      1.7         01/11/00 Dan O'Connel    Port AllowPALCRT feature
**       from Win9x to WinNT/Win2K.
**  7    3dfx      1.6         12/02/99 Dan O'Connel    Preliminary round of
**       changes to support new mechanism in 3dfx Tools to turn on and off
**       monitor/DFP/TvOut.
**  6    3dfx      1.5         10/18/99 Dan O'Connel    Corrections to initial DFP
**       support checkin.
**  5    3dfx      1.4         10/11/99 Dan O'Connel    Fix build broken by my
**       checkin of DFP support into WinNt4.  Changes shared files to not call DFP
**       routines.  
**  4    3dfx      1.3         10/11/99 Dan O'Connel    Initial port of Digital
**       Flat Panel support from Win9x to WinNT4.
**  3    3dfx      1.2         10/04/99 Dan O'Connel    Change name of tvout.h
**       include file to fxtvout.h to avoid conflict with file of same name in
**       Win2K DDK.
**  2    3dfx      1.1         10/01/99 Dan O'Connel    Port "3dfx Tools" support,
**       TvOut support, and a few misc. bug fixes from WinNt4 to Win2K.  The Win2K
**       DDK has a file named tvout.h so change our includes to us the name
**       fxtvout.h for Win2K and tvout.h for WinNT4.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $History: funcapi.c $
** 
** *****************  Version 1  *****************
** User: Doconnell    Date: 9/03/99    Time: 10:58a
** Created in $/devel/h5/WinNT/Src/Video/Displays/h5
** Add 3dfx Tools interfaces and TVout support.
** 
** *****************  Version 7  *****************
** User: Doconnell    Date: 8/27/99    Time: 5:10p
** Updated in $/Releases/Voodoo3/V3_RT4/3dfx/devel/H3/WINNT/SRC/Video/Displays/Voodoo3
** PRS 8199 Work with Edge Tools to correctly handle concurrent output to
** Monitor and CRT and various OEM specific options (Gateway).  Also port
** some fixes from Win9x to WinNT4 that have to do initializing and
** handling the NVRAM cache.
** 
** *****************  Version 5  *****************
** User: Stb_doconnel Date: 6/04/99    Time: 12:36p
** Updated in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/displays/voodoo3
** PRS 6386 Misc. cancel/apply fixes for 3dfx Tools interface
** 
** *****************  Version 4  *****************
** User: Stb_doconnel Date: 5/19/99    Time: 11:55a
** Updated in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/displays/voodoo3
** PRS 6175 Fix uninit. var. that was causing monitor to be turned off
** randomly.
** 
** *****************  Version 3  *****************
** User: Stb_doconnel Date: 5/18/99    Time: 2:19p
** Updated in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/displays/voodoo3
** Correction to remove compiler warning
** 
** *****************  Version 2  *****************
** User: Stb_doconnel Date: 5/13/99    Time: 12:36p
** Updated in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/displays/voodoo3
** Correct Tv Out Active and Monitor Active status returned to Edge Tools
*/
/****************************************************************************/


#include "precomp.h"
#define _TEXT(x) x
#include "funcapi.h"
#include "edgedefs.h"
#include "edgecaps.h"

#include "qmodes.h"	/* for LPQIN Structure */
#include "tv.h"
#include "fxtvout.h"
#ifdef STB_DFP_ENABLED
#include "dfpapi.h"
#endif
#include "fxioctl.h"
#define GET(hwPtr)                hwPtr
#define ghwIO                     ((SstIORegs *)ppdev->pjBase)
#define	FREF	143184   //Fref*10000 

// WARNING: this file currently supports the old mechanism for turning on TvOut and also supports the new
// mechanism which will be used to simultaniously turn on/off TvOut, DFP, and the analog monitor.  Eventually
// some of the old mechanism may be removed, but in the meantime if it seems to the reader that there are 
// duplicate functions in this code, you may be right.  DanO 12/2/99

int FunctionAPIGetProperty(void * pInData,void * pOutData, PDEV * ppdev)
{
   DWORD   grxFreq, grxClock;
	

	switch (((STB_PROPERTY *)pInData)->ulPropertyId)
		{	
		case (STB_FUNCTION_CAPS):
		{
			/* 
			checks to see what function the card supports
			*/

			QIN Qin;
			QTVSTATUS TVStatus;
			
			/* initialise output structure */

			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
			((STB_PROPERTY *)pOutData)->ulResult=STB_ESCAPE_HANDLED;

			/* ensure VGA capabilities are always enabled */
			/* there is code in qmodes to check for monitor enabled, but not to check if */
			/* is present */

			((STB_PROPERTY *)pOutData)->ulPropertyValue=
				((STB_PROPERTY *)pOutData)->ulPropertyValue | STB_FUNCTIONCAPS0_VGA;


#ifdef STB_TV_ENABLED

			if (ppdev->bTvoCapable)
			{
				((STB_PROPERTY *)pOutData)->ulPropertyValue = 
					((STB_PROPERTY *)pOutData)->ulPropertyValue | STB_FUNCTIONCAPS0_TV;
			}
#if 0
			else 
			{
				((STB_PROPERTY *)pOutData)->ulPropertyValue = 
					((STB_PROPERTY *)pOutData)->ulPropertyValue & ~STB_FUNCTIONCAPS0_TV;
			}
#endif

#endif /* STB_TV_ENABLED */
#ifdef STB_DFP_ENABLED
			/* check for DFP support */
            if (DFPisAdapterDfpCapable(ppdev))
			{
				((STB_PROPERTY *)pOutData)->ulPropertyValue=
				((STB_PROPERTY *)pOutData)->ulPropertyValue | STB_FUNCTIONCAPS0_DFP;
			}
#endif //def STB_DFP_ENABLED


			return (STB_ESCAPE_HANDLED);
		}


    case (STB_FUNCTION_DEFAULT):
        {
            DWORD dwDefault=0x00;


            /* Call the driver passing down no values as it is not required.
               dwDefault is the value returned which contains which display is
               default */  //??? TBD call miniport to get default???
            dwDefault = 0x001;  // hardcode to VGA for now DanO 11/4/99???


            /* intitialise output structure */
            ((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
            ((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
            ((STB_PROPERTY *)pOutData)->ulResult=(DWORD)STB_ESCAPE_HANDLED;

            if (dwDefault == 0x001)	 		// if a VGA output is default
                ((STB_PROPERTY *)pOutData)->ulPropertyValue=STB_FUNCTION_VGA;
            else if (dwDefault == 0x002)	// if a TV output is default
                ((STB_PROPERTY *)pOutData)->ulPropertyValue=STB_FUNCTION_TV;
            else if (dwDefault == 0x004)	// if a DFP output is default
                ((STB_PROPERTY *)pOutData)->ulPropertyValue=STB_FUNCTION_DFP;
            else
            {	/*	If the driver returns an incorrect value which it shouldn't,
                return the escape wasn't handled */
                ((STB_PROPERTY *)pOutData)->ulResult = (DWORD)STB_FEATURE_NOT_SUPPORTED;
                ((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
                return (STB_ESCAPE_NOT_SUPPORTED);
            }

            return (STB_ESCAPE_HANDLED);
            break;

        }

	case (STB_FUNCTION_ACTIVE):
		{
			QGETSET_MONITOR_CTL Monitor;

			/* initialise output structure */

			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
			((STB_PROPERTY *)pOutData)->ulResult=STB_ESCAPE_HANDLED;

			/* check if vga monitor is enabled */

			/* QUERY_ANALOG_MONITOR in qmodes.c casts lpQIN to (QGETSET_MONITOR_CTL *) */
			/* which is a struct of DWORD dwSubfunc, DWORD monitorStatus and */
			/* DWORD monitorControl */

			Monitor.dwSubFunc = QUERY_ANALOG_MONITOR;
			Monitor.monitorControl = 0;
			Monitor.monitorStatus = 0;

			QueryMode(ppdev, (LPQIN)&Monitor, (LPVOID)&Monitor); 

			if (Monitor.monitorStatus == MONITOR_IS_ENABLED)
			{
				((STB_PROPERTY *)pOutData)->ulPropertyValue |= STB_FUNCTIONACTIVE0_VGA;
			}
			else
			{
				((STB_PROPERTY *)pOutData)->ulPropertyValue &= ~STB_FUNCTIONACTIVE0_VGA;
			}

#ifdef STB_TV_ENABLED
			if (ppdev->dwTvoActive)
			{
	  			((STB_PROPERTY *)pOutData)->ulPropertyValue |= STB_FUNCTIONACTIVE0_TV;
			}
			else
			{
				((STB_PROPERTY *)pOutData)->ulPropertyValue &= ~STB_FUNCTIONACTIVE0_TV;
			}

#endif /* STB_TV_ENABLED */

#ifdef STB_DFP_ENABLED
			/* Check for DFP active */
            if (DFPisPanelActive(ppdev))
			{
				((STB_PROPERTY *)pOutData)->ulPropertyValue |= STB_FUNCTIONACTIVE0_DFP;
			}
			else
			{
				((STB_PROPERTY *)pOutData)->ulPropertyValue &= ~STB_FUNCTIONACTIVE0_DFP;
			}
#endif /* STB_DFP_ENABLED */

			return(STB_ESCAPE_HANDLED);
		}

    case (STB_FUNCTION_SIM):
        {
            TVGETSTANDARD TVStandard;

            ((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
            ((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
            ((STB_PROPERTY *)pOutData)->ulPropertyValue=0x00;

            switch (((STB_PROPERTY *)pInData)->ulPropertyValue)
            {
            case STB_FUNCTION_VGA:
                {
                    if (IS_VOODOO3)
                    {
                        switch (ppdev->ulCustomerNumber)
                        {
                        case 7:  //Mystery customer number 7 wants exclusivity
                            ((STB_PROPERTY *)pOutData)->ulPropertyValue=STB_SIM0_VGA;
                            break;

                        default: //Used for retail, and mystery customer number X ("allowPALCRT")
                            TVOutGetStandard(  ppdev, NULL, &TVStandard );
                            // Unless "allowPALCRT" is specified don't allow simultaneous TV/VGA for PAL types.
                            if ((TVStandard.dwStandard != VP_TV_STANDARD_NTSC_M) && !(ppdev->bAllowPALCRT))
                            {
                                ((STB_PROPERTY *)pOutData)->ulPropertyValue = (DWORD)(STB_SIM0_VGA);
                            }
                            else
                            {
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
                        if (!(ppdev->bAllowPALCRT))
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
                    break;
                }

            case STB_FUNCTION_TV:
                {
                    STB_PROPERTY	property, returnproperty;

                    ((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
                    ((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;

                    property.Guid=((STB_PROPERTY *)pInData)->Guid;
                    property.ulPropertyId=STB_FUNCTION_CAPS;

                    /* do escape call to see if there is any point in proceding */

                    FunctionAPIGetProperty((void *)&property,(void *) &returnproperty, ppdev);	

                    if  ((returnproperty.ulResult == STB_ESCAPE_HANDLED) &&
                            (returnproperty.ulPropertyValue & STB_FUNCTIONCAPS0_TV))
                    {
                        if (IS_VOODOO3)
                        {
                            switch (ppdev->ulCustomerNumber)
                            {
                            case 7:  //Mystery customer number 7 wants exclusivity
                                ((STB_PROPERTY *)pOutData)->ulPropertyValue = (DWORD)(STB_SIM0_TV);
                                break;

                            default: //Used for retail, and mystery customer number X ("allowPALCRT")
                                TVOutGetStandard(  ppdev, NULL, &TVStandard );
                                // Unless "allowPALCRT" is specified don't allow simultaneous TV/VGA for PAL types.
                                if ((TVStandard.dwStandard != VP_TV_STANDARD_NTSC_M) && !(ppdev->bAllowPALCRT))
                                {
                                    ((STB_PROPERTY *)pOutData)->ulPropertyValue = (DWORD)(STB_SIM0_VGA);
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
                            if (!(ppdev->bAllowPALCRT))
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
                    break;
                }

            case STB_FUNCTION_DFP:
                {
                    STB_PROPERTY	property, returnproperty;

                    ((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
                    ((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;

                    property.Guid=((STB_PROPERTY *)pInData)->Guid;
                    property.ulPropertyId=STB_FUNCTION_CAPS;

                    /* do escape call to see if there is any point in proceding */

                    FunctionAPIGetProperty((void *)&property,(void *) &returnproperty, ppdev);	

                    if  ((returnproperty.ulResult == STB_ESCAPE_HANDLED) &&
                            (returnproperty.ulPropertyValue & STB_FUNCTIONCAPS0_DFP))
                    {
                        ((STB_PROPERTY *)pOutData)->ulPropertyValue = (DWORD)(STB_SIM0_VGA | !STB_SIM0_TV | STB_SIM0_DFP);
                        ((STB_PROPERTY *)pOutData)->ulResult=(DWORD) STB_ESCAPE_HANDLED;
                    }
                    else
                    {	/* if DFP is not supported return error code */
                        ((STB_PROPERTY *)pOutData)->ulPropertyValue = 0x00;
                        return (STB_FEATURE_NOT_SUPPORTED);
                    }
                    return (STB_ESCAPE_HANDLED);	
                    break;
                }

            default:
                {
                    ((STB_PROPERTY *)pOutData)->ulResult=(DWORD) STB_FEATURE_NOT_SUPPORTED;
                    return(STB_FEATURE_NOT_SUPPORTED);
                    break;
                }
            }
            break;
        }

	case (STB_SIM_VGA):
		{
			TVGETSTANDARD TVStandard;

			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulPropertyValue=STB_SIM0_VGA;
			((STB_PROPERTY *)pOutData)->ulResult=STB_ESCAPE_HANDLED;
#ifdef STB_DFP_ENABLED
			/*
				TODO : Check if simultaneous DFP is allowed 
			*/
			((STB_PROPERTY *)pOutData)->ulPropertyValue |= STB_SIM0_DFP;
#endif //def STB_DFP_ENABLED
#ifdef STB_TV_ENABLED	
            if (IS_VOODOO3)
            {
                switch (ppdev->ulCustomerNumber)
                {
                case 7:  //Mystery customer number 7 wants exclusivity
                    break;

                default: //Used for retail, and mystery customer number X ("allowPALCRT")
                    TVOutGetStandard(  ppdev, NULL, &TVStandard );
                    // if mode is NTSC then allow simultaneous VGA and TV display
                    if ((TVStandard.dwStandard == VP_TV_STANDARD_NTSC_M) || (ppdev->bAllowPALCRT))
                    {
                        ((STB_PROPERTY *)pOutData)->ulPropertyValue |= STB_SIM0_TV;
                    }
                    break;
                }
            }
            else
            {
                // Unless "allowPALCRT" is specified don't allow simultaneous TV/VGA for either PAL or NTSC.
                // Note: "allowPALCRT" allows simultaneous TV/VGA for both PAL and NTSC even though the name
                // is misleading.
                if (ppdev->bAllowPALCRT)
                    ((STB_PROPERTY *)pOutData)->ulPropertyValue |= STB_SIM0_TV;
            }
#endif


			return (STB_ESCAPE_HANDLED);
		}
	case (STB_SIM_TV):
		{
			QTVSTATUS TVStatus;
			TVGETSTANDARD TVStandard;

			/* 
				TVOutStatus uses lpOutput->dwNumSimultaneous = 1; 
			*/

			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulPropertyValue = 0x00;
			
#ifdef STB_TV_ENABLED	
			((STB_PROPERTY *)pOutData)->ulResult=STB_ESCAPE_HANDLED;

			// only set property value if adapter is TVOut capable.
			if (ppdev->bTvoCapable)
			{
                ((STB_PROPERTY *)pOutData)->ulPropertyValue |= STB_SIM0_TV;
                if (IS_VOODOO3)
                {
                    switch (ppdev->ulCustomerNumber)
                    {
                    case 7:  //Mystery customer number 7 wants exclusivity
                        break;

                    default: //Used for retail, and mystery customer number X ("allowPALCRT")
                        TVOutGetStandard( ppdev, NULL, &TVStandard );
                        // if mode is NTSC then allow simultaneous VGA and TV display
                        if ((TVStandard.dwStandard == VP_TV_STANDARD_NTSC_M) || (ppdev->bAllowPALCRT))
                        {
                            ((STB_PROPERTY *)pOutData)->ulPropertyValue |= STB_SIM0_VGA;
                        }
                        break;
                    }
                }
                else
                {
                    // Unless "allowPALCRT" is specified don't allow simultaneous TV/VGA for either PAL or NTSC.
                    // Note: "allowPALCRT" allows simultaneous TV/VGA for both PAL and NTSC even though the name
                    // is misleading.
                    if (ppdev->bAllowPALCRT)
                        ((STB_PROPERTY *)pOutData)->ulPropertyValue |= STB_SIM0_TV;
                }
            }
			else

			
#endif /* STB_TV_ENABLED */
			{
			((STB_PROPERTY *)pOutData)->ulPropertyValue=0x00;
			((STB_PROPERTY *)pOutData)->ulResult=(DWORD)STB_FEATURE_NOT_SUPPORTED;
			return (STB_FEATURE_NOT_SUPPORTED);
			}
			/*
				Note: Simultaneous TV & DFP is not possible on Voodoo3 or Napalm
			*/
			return (STB_ESCAPE_HANDLED);
		}

	case (STB_SIM_DFP):
		{
			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
#ifdef STB_DFP_ENABLED
			((STB_PROPERTY *)pOutData)->ulPropertyValue = STB_SIM0_DFP;
			((STB_PROPERTY *)pOutData)->ulResult=STB_ESCAPE_HANDLED;
			/*
				TODO : Check if simultaneous VGA & DFP, and TV & DFP is allowed 
			*/

#else		/* if dfp is not selected at compile time return error code */
			((STB_PROPERTY *)pOutData)->ulPropertyValue=0x00;
			((STB_PROPERTY *)pOutData)->ulResult=(DWORD)STB_FEATURE_NOT_SUPPORTED;
			return (STB_FEATURE_NOT_SUPPORTED);

#endif /* STB_DFP_ENABLED */
			return (STB_ESCAPE_HANDLED);
		}

	case (STB_FUNCTION_CLOCKRATE): // Return the current clock rate	in MHz
		{		
			/* intitialise output structure */
			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
			((STB_PROPERTY *)pOutData)->ulResult= STB_ESCAPE_HANDLED;

            grxClock = GET(ghwIO->pllCtrl1);
	         
	        grxFreq = PLL2MHz((DWORD)grxClock) / 10000;	
          
          			
			((STB_PROPERTY *)pOutData)->ulPropertyValue = grxFreq ;

		    return (STB_ESCAPE_HANDLED);
		}
       
	case (STB_FUNCTION_CLOCKDEFAULT): // Return the default clock rate in MHz
        {
            TDFX_MISC_INFO miscBuff;
            ULONG ulTemp;

            /* intitialise output structure */
            ((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
            ((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
            ((STB_PROPERTY *)pOutData)->ulResult= STB_ESCAPE_HANDLED;

            if (EngDeviceIoControl(ppdev->hDriver,
                    IOCTL_GET_MISC_VALUES,
                    NULL,
                    0,
                    &miscBuff,
                    sizeof(miscBuff),
                    &ulTemp))
                return (STB_ERROR_SETTING_VALUE);

            ((STB_PROPERTY *)pOutData)->ulPropertyValue = miscBuff.dwDefaultClock;

            return (STB_ESCAPE_HANDLED);
        }

       
	case (STB_FUNCTION_CLOCKMAX): // Return the Max clock rate in MHz

		{		
            TDFX_MISC_INFO miscBuff;
            ULONG ulTemp;

			/* intitialise output structure */
			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulResult= STB_ESCAPE_HANDLED;

            if (EngDeviceIoControl(ppdev->hDriver,
                    IOCTL_GET_MISC_VALUES,
                    NULL,
                    0,
                    &miscBuff,
                    sizeof(miscBuff),
                    &ulTemp))
                return (STB_ERROR_SETTING_VALUE);

            ((STB_PROPERTY *)pOutData)->ulPropertyValue = miscBuff.dwMaxClock;

			return (STB_ESCAPE_HANDLED);
		}

	case (STB_FUNCTION_CLOCKMIN): // Return the Min clock rate in MHz

		{		
            TDFX_MISC_INFO miscBuff;
            ULONG ulTemp;

			/* intitialise output structure */
			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulResult= STB_ESCAPE_HANDLED;

            if (EngDeviceIoControl(ppdev->hDriver,
                    IOCTL_GET_MISC_VALUES,
                    NULL,
                    0,
                    &miscBuff,
                    sizeof(miscBuff),
                    &ulTemp))
                return (STB_ERROR_SETTING_VALUE);

            ((STB_PROPERTY *)pOutData)->ulPropertyValue = miscBuff.dwMinClock;

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
			long dwEnum; 
			dwEnum = 0;  // hardcode to zero for NT4 for now.

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


int FunctionAPISetProperty(void * pInData,void * pOutData, PDEV * ppdev)
{
   ((STB_PROPERTY *)pOutData)->Guid =  ((STB_PROPERTY *)pInData)->Guid;
   ((STB_PROPERTY *)pOutData)->ulPropertyId =  ((STB_PROPERTY *)pInData)->ulPropertyId;
   ((STB_PROPERTY *)pOutData)->ulResult=(DWORD)STB_ESCAPE_HANDLED;

  	switch (((STB_GROUPPROPERTY *)pInData)->ulPropertyId)
	{
      case (STB_FUNCTION_CANCEL):
#ifdef STB_DFP_ENABLED
         DFPCancelSettings(pInData, pOutData, ppdev);
#endif
#ifdef STB_TV_ENABLED
         TVOutCancelSettings(ppdev, pInData, pOutData);
#endif
         ppdev->ulDevicesThatHaveTheirStatesSaved = 0UL;
         return (STB_ESCAPE_HANDLED);

      case (STB_FUNCTION_COMMIT):
         ppdev->ulDevicesThatHaveTheirStatesSaved = 0UL;
         return (STB_ESCAPE_HANDLED);

      case (STB_FUNCTION_ACTIVE):
      {
          DWORD dwReturned = 0;
          DWORD dwSetting = 0x0000;
          DWORD dwMask;
          ULONG ulTemp;

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
          if (EngDeviceIoControl(ppdev->hDriver,
                  IOCTL_SET_ALTERNATE_DISPLAYS,
                  &dwSetting,
                  sizeof(dwSetting),
                  &dwReturned,
                  sizeof(dwReturned),
                  &ulTemp))
          {
              DISPDBG((0, "IOCTL_SET_ALTERNATE_DISPLAYS - failed "));
              return (STB_ERROR_SETTING_VALUE);
          }

          /* initialise output data */
          ((STB_PROPERTY *)pOutData)->Guid =  ((STB_PROPERTY *)pInData)->Guid;

          ((STB_PROPERTY *)pOutData)->ulPropertyId = ((STB_PROPERTY *)pInData)->ulPropertyId;
          /* if we activated/deactivated all 3 correctly */
          if (dwReturned == dwMask)
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

int FunctionAPIGetGroupProperty(void * pInData,void * pOutData, PDEV * ppdev)
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

int FunctionAPISetGroupProperty(void * pInData,void * pOutData, PDEV * ppdev)
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

int FunctionAPIInit(PDEV * ppdev)
{

#ifdef STB_TV_ENABLED
	QTVSTATUS TVStatus;

    TDFX_QUERY_VALUE_INFO queryValueInfo;
    DWORD                 numBytes;


			
	// check tv out status to insure ppdev->dwTvoActive is properly initialized
	// ppdev->dwTvoActive is updated as sideeffect of below TVOutStatus call.

    TVOutStatus(  ppdev, NULL, &TVStatus );
			
	// save TVOut Capable flag to reduce the number of times the this module needs to call TVOutStatus.
	//  Each time TVOutStatus is called the TV screen is garbled.
	if (TV_ENCODER_PRESENT == (TVStatus.dwEncoder & TV_ENCODER_PRESENT))
		ppdev->bTvoCapable = TRUE;
	else 
		ppdev->bTvoCapable = FALSE;

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


#endif /* STB_TV_ENABLED */
//#ifdef STB_DFP_ENABLED
#if 0

    // initialize all display driver code to handle the DFP.
    DFPinitialization(  ppdev );
			
#endif /* STB_TV_ENABLED */

	 // Write out the 3dfx Tools API version

	 SetRegSZ(ppdev, EDGE_API_VERSION_REG, EDGE_API_VERSION);

    return (1);

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

