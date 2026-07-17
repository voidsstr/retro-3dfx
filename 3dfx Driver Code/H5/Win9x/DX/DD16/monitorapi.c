/* -*-c++-*- */
/* $Header: monitorapi.c, 4, 10/11/00 8:51:47 PM, Brent$ */
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
** File name:   monitorapi.c
**
** Description: Provide Functionality to check status & capabilities of card.
**
** $Revision: 4$
** $Date: 10/11/00 8:51:47 PM$
**
** $Log: 
**  4    3dfx      1.2.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  3    Napalm    1.2         10/22/99 Scott Kephart   
**  2    Napalm    1.1         09/30/99 Reid Campbell   Extra comments added and
**       3dfx type header added.
**  1    Napalm    1.0         09/22/99 Reid Campbell   
** $ 
*/


#define _TEXT(x) x
#include "monitorapi.h"
#include "edgedefs.h"


// driver specific includes below 
#include "header.h"

// added jmccartney to allow calls down to the VDD and the associated data structures
#define Not_VxD
#include "minivdd.h"

#define Not_VxD
#include <vmm.h>

// used when getting the device context of the card to call the driver
extern DWORD			dwDeviceHandle;

/*	This function calls the driver to move the position of the image
	on the monitor.  
	movHoriz is the increment to move the image Horizontally
	*NB* a negative movHoriz values moves the image to the right
	and a positive movHoriz value moves the image to the left
	For movVert a positive value moves the image up and a negative
	values moves the image down 
*/
int callVdd(int movHoriz, int movVert)
{
	DWORD					dwStatus;

	long					scrUpdate[2];

	scrUpdate[0] = (long)movHoriz;
	scrUpdate[1] = (long)movVert;

	dwDeviceHandle = _FF(DevNode);	//Get the device context of our card

	// call to the driver to move the image on the monitor
	dwStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO,
					   dwDeviceHandle,
					   H3VDD_MOVE_SCREEN,
					   (DWORD)((long _far*)scrUpdate),             
					   0);
	return (int)dwStatus;
	
}

int MonitorAPIGetProperty(void * pInData,void * pOutData)
{
	int	returnValue = 1;
	switch (((STB_PROPERTY *)pInData)->ulPropertyId)
		{
		case (STB_MONITOR_CONTROL_CAPS):	// Read Only
			{
				/*  Pre:	TRUE
					
					Post:	pOutData = STB_MONCTRLCAPS0_POSITION (TRUE if driver has ability to control 
							the position of the image on the monitor)
							pOutData = STB_MONCTRLCAPS0_SIZE (TRUE if the driver has the ability to control
							the size of te image on the monitor)
							pOutData = STB_MONCTRLCAPS0_DDC12B (TRUE ifthe driver has the ability to control
							the monitor via DDC1/2B)
				*/	
				
				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				// Resizing the image does not work correctly at the moment
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000 | STB_MONCTRLCAPS0_POSITION;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}	
		case (STB_MONITOR_DDC12B_CAPS):	// Read Only
			{
				/*	Pre:	TRUE

					Post:	pOutData = STB_MONDDC12BCAPS0_POSITION (TRUE if the driver has the ability to control
							the position of the image on the monitor via DDC1/2B)
							pOutData = STB_MONDDC12BCAPS0_SIZE (TRUE if the driver has the ability to control
							the size of the image on the monitor via DDC1/2B)
							pOutData = STB_MONDDC12BCAPS0_BRIGHTNESS (TRUE if the driver supports brightness control
							for the monitor via DDC1/2B)
							pOutData = STB_MONDDC12BCAPS0_CONTRAST (TRUE if the driver supports contrast control for
							the monitor via DDC1/2B)
				*/
								
				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_HPOSITION):	// Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_POSITION = TRUE 
							STB_MONDC12BCAPS0_POSITION = TRUE

					Post:	pOutData = STB_MONCTRL_HPOSITION (current setting for the horizontal position of the 
							image on the screen)
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_HPOSITIONMAX): // Read Only
			{
				/*	Pre:	STB_MONCTRLCAPS0_POSITION = TRUE
							STB_MONDDC12BCAPS0_POSITION = TRUE

					Post:	pOutData = STB_MONCTRL_HPOSITIONMAX (maximum horizontal position of the image on the 
							screen)
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_HPOSITIONMIN): // Read Only
			{
				/*	Pre:	STB_MONCTRLCAPS0_POSITION = TRUE
							STB_MONDDC12BCAPS0_POSITION = TRUE

					Post:	pOutData = STB_MONCTRL_HPOSITIONMIN (minimum horizontal position of the image on the 
							screen)
				*/
								
				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_HPOSITIONINC): // Read/Write
			{
				/*	Pre:	TRUE

					Post:	If STB_MONCTRL_HPOSITION + STB_MONCTRL_HPOSITIONINC > STB_MONCTRL_HPOSITIONMAX
							Then STB_MONCTRL_HPOSITION = STB_MONCTRL_HPOSITIONMAX
							Else STB_MONCTRL_HPOSITION = STB_MONCTRL_HPOSITION + STB_MONCTRL_HPOSITIONINC
				*/
				

				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_HPOSITIONDEC): // Read/Write
			{
				/*	Pre:	TRUE

					Post:	If STB_MONCTRL_HPOSITION - STB_MONCTRL_HPOSITIONDEC < STB_MONCTRL_HPOSITIONMIN
							Then STB_MONCTRL_HPOSITION = STB_MONCTRL_HPOSITIONMIN
							Else STB_MONCTRL_HPOSITION = STB_MONCTRL_HPOSITION - STB_MONCTRL_HPOSITIONDEC
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_VPOSITION): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_POSITION = TRUE 
							STB_MONDC12BCAPS0_POSITION = TRUE

					Post:	pOutData = STB_MONCTRL_VPOSITION (current setting for the vertical position of the 
							image on the screen)
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}	
		case (STB_MONCTRL_VPOSITIONMAX): // Read Only
			{
				/*	Pre:	STB_MONCTRLCAPS0_POSITION = TRUE
							STB_MONDDC12BCAPS0_POSITION = TRUE

					Post:	pOutData = STB_MONCTRL_VPOSITIONMAX (maximum vertical position of the image on the 
							screen)
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}	
		case (STB_MONCTRL_VPOSITIONMIN): // Read Only
			{
				/*	Pre:	STB_MONCTRLCAPS0_POSITION = TRUE
							STB_MONDDC12BCAPS0_POSITION = TRUE

					Post:	pOutData = STB_MONCTRL_VPOSITIONMIN (minimum vertical position of the image on the 
							screen)
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_VPOSITIONINC): // Read/Write
			{
				/*	Pre:	TRUE

					Post:	If STB_MONCTRL_VPOSITION + STB_MONCTRL_VPOSITIONINC > STB_MONCTRL_VPOSITIONMAX
							Then STB_MONCTRL_VPOSITION = STB_MONCTRL_VPOSITIONMAX
							Else STB_MONCTRL_VPOSITION = STB_MONCTRL_VPOSITION + STB_MONCTRL_VPOSITIONINC
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_VPOSITIONDEC): // Read/Write
			{
				/*	Pre:	TRUE

					Post:	If STB_MONCTRL_VPOSITION - STB_MONCTRL_VPOSITIONDEC < STB_MONCTRL_VPOSITIONMIN
							Then STB_MONCTRL_VPOSITION = STB_MONCTRL_VPOSITIONMIN
							Else STB_MONCTRL_VPOSITION = STB_MONCTRL_VPOSITION - STB_MONCTRL_VPOSITIONDEC
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_WIDTH): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_SIZE = TRUE
							STB_MONDDC12BCAPS0_SIZE = TRUE

					Post:	pOutData = STB_MONCTRL_WIDTH (the current setting for the width of the image on the
							screen)
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_WIDTHMAX): // Read Only
			{
				/*	Pre:	STB_MONCTRLCAPS0_SIZE = TRUE
							STB_MONDDC12BCAPS0_SIZE = TRUE

					Post:	pOutData = STB_MONCTRL_WIDTHMAX (maximum value that can be applied to STB_MONCTRL_WIDTH)
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_WIDTHMIN): // Read Only
			{
				/*	Pre:	STB_MONCTRLCAPS0_SIZE = TRUE
							STB_MONDDC12BCAPS0_SIZE = TRUE

					Post:	pOutData = STB_MONCTRL_WIDTHMIN (minimum value that can be applied to STB_MONCTRL_WIDTH)
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_WIDTHINC): // Read/Write
			{
				/*	Pre:	TRUE

					Post:	If STB_MONCTRL_WIDTH + STB_MONCTRL_WIDTHINC > STB_MONCTRL_WIDTHMAX
							Then STB_MONCTRL_WIDTH = STB_MONCTRL_WIDTHMAX
							Else STB_MONCTRL_WIDTH = STB_MONCTRL_WIDTH + STB_MONCTRL_WIDTHINC
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_WIDTHDEC): // Read/Write
			{
				/*	Pre:	TRUE

					Post:	If STB_MONCTRL_WIDTH - STB_MONCTRL_WIDTHDEC < STB_MONCTRL_WIDTHMIN
							Then STB_MONCTRL_WIDTH = STB_MONCTRL_WIDTHMIN
							Else STB_MONCTRL_WIDTH = STB_MONCTRL_WIDTH - STB_MONCTRL_WIDTHDEC
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_HEIGHT): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_SIZE = TRUE
							STB_MONDDC12BCAPS0_SIZE = TRUE

					Post:	pOutData = STB_MONCTRL_HEIGHT (the current setting for the height of the image on the
							screen)
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_HEIGHTMAX): // Read Only
			{
				/*	Pre:	STB_MONCTRLCAPS0_SIZE = TRUE
							STB_MONDDC12BCAPS0_SIZE = TRUE

					Post:	pOutData = STB_MONCTRL_HEIGHTMAX (maximum value that can be applied to STB_MONCTRL_HEIGHT)
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_HEIGHTMIN): // Read Only
			{
				/*	Pre:	STB_MONCTRLCAPS0_SIZE = TRUE
							STB_MONDDC12BCAPS0_SIZE = TRUE

					Post:	pOutData = STB_MONCTRL_HEIGHTMIN (minimum value that can be applied to STB_MONCTRL_HEIGHT)
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_HEIGHTINC): // Read/Write
			{	
				/*	Pre:	TRUE

					Post:	If STB_MONCTRL_HEIGHT + STB_MONCTRL_HEIGHTINC > STB_MONCTRL_HEIGHTMAX
							Then STB_MONCTRL_HEIGHT = STB_MONCTRL_HEIGHTMAX
							Else STB_MONCTRL_HEIGHT = STB_MONCTRL_HEIGHT + STB_MONCTRL_HEIGHTINC
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_HEIGHTDEC): // Read/Write
			{
				/*	Pre:	TRUE

					Post:	If STB_MONCTRL_HEIGHT - STB_MONCTRL_HEIGHTDEC < STB_MONCTRL_HEIGHTMIN
							Then STB_MONCTRL_HEIGHT = STB_MONCTRL_HEIGHTMIN
							Else STB_MONCTRL_HEIGHT = STB_MONCTRL_HEIGHT - STB_MONCTRL_HEIGHTDEC
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_BRIGHTNESS): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_BRIGHTNESS = TRUE

					Post:	pOutData = STB_MONCTRL_BRIGHTNESS (the current setting for brightness)
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_BRIGHTNESSMAX): // Read Only
			{
				/*	Pre:	STB_MONCTRLCAPS0_BRIGHTNESS = TRUE

					Post:	pOutData = STB_MONCTRL_BRIGHTNESSMAX (maximum value that can be applied
							to STB_MONCTRL_BRIGHTNESS)
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_BRIGHTNESSMIN): // Read Only
			{
				/*	Pre:	STB_MONCTRLCAPS0_BRIGHTNESS = TRUE

					Post:	pOutData = STB_MONCTRL_BRIGHTNESSMIN (minimum value that can be applied
							to STB_MONCTRL_BRIGHTNESS)
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}			
		case (STB_MONCTRL_BRIGHTNESSINC): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_BRIGHTNESS = TRUE

					Post:	If STB_MONCTRL_BRIGHTNESS + STB_MONCTRL_BRIGHTNESSINC > STB_MONCTRL_BRIGHTNESSMAX
							Then STB_MONCTRL_BRIGHTNESS = STB_MONCTRL_BRIGHTNESSMAX
							Else STB_MONCTRL_BRIGHTNESS = STB_MONCTRL_BRIGHTNESS + STB_MONCTRL_BRIGHTNESSINC
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_BRIGHTNESSDEC): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_BRIGHTNESS = TRUE

					Post:	If STB_MONCTRL_BRIGHTNESS - STB_MONCTRL_BRIGHTNESSDEC < STB_MONCTRL_BRIGHTNESSMIN
							Then STB_MONCTRL_BRIGHTNESS = STB_MONCTRL_BRIGHTNESSMIN
							Else STB_MONCTRL_BRIGHTNESS = STB_MONCTRL_BRIGHTNESS - STB_MONCTRL_BRIGHTNESSDEC
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_CONTRAST): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_CONTRAST = TRUE

					Post:	pOutData = STB_MONCTRL_CONTRAST (the current setting for contrast)
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_CONTRASTMAX): // Read Only
			{
				/*	Pre:	STB_MONCTRLCAPS0_CONTRAST = TRUE

					Post:	pOutData = STB_MONCTRL_CONTRASTMAX (maximum value that can be applied
							to STB_MONCTRL_CONTRAST)
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_CONTRASTMIN): // Read Only
			{	
				/*	Pre:	STB_MONCTRLCAPS0_CONTRAST = TRUE

					Post:	pOutData = STB_MONCTRL_CONTRASTMIN (minimum value that can be applied
							to STB_MONCTRL_CONTRAST)
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_CONTRASTINC): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_CONMTRAST = TRUE

					Post:	If STB_MONCTRL_CONTRAST + STB_MONCTRL_CONTRASTINC > STB_MONCTRL_CONTRASTMAX
							Then STB_MONCTRL_CONTRAST = STB_MONCTRL_CONTRASTMAX
							Else STB_MONCTRL_CONTRAST = STB_MONCTRL_CONTRAST + STB_MONCTRL_CONTRASTINC
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_CONTRASTDEC): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_CONTRAST = TRUE

					Post:	If STB_MONCTRL_CONTRAST - STB_MONCTRL_CONTRASTDEC < STB_MONCTRL_CONTRASTMIN
							Then STB_MONCTRL_CONTRAST = STB_MONCTRL_CONTRASTMIN
							Else STB_MONCTRL_CONTRAST = STB_MONCTRL_CONTRAST - STB_MONCTRL_CONTRASTDEC
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}	
		}

		return 1;  // just return 1 for the moment

}



int MonitorAPISetProperty(void * pInData,void * pOutData)
{

	int returnValue = STB_ESCAPE_HANDLED;

	switch (((STB_PROPERTY *)pInData)->ulPropertyId)
		{
		
		case (STB_MONCTRL_HPOSITION):	// Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_POSITION = TRUE 
							STB_MONDC12BCAPS0_POSITION = TRUE

					Post:	STB_MONCTRL_HPOSITION = pInData					
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_HPOSITIONINC): // Read/Write
			{
				/*	Pre:	TRUE

					Post:	If STB_MONCTRL_HPOSITION + STB_MONCTRL_HPOSITIONINC > STB_MONCTRL_HPOSITIONMAX
							Then STB_MONCTRL_HPOSITION = STB_MONCTRL_HPOSITIONMAX
							Else STB_MONCTRL_HPOSITION = STB_MONCTRL_HPOSITION + STB_MONCTRL_HPOSITIONINC
				*/

				//  Initialise output structure
				// Stuff jmccartney added
				// This case moves the image displayed on the monitor right by calling the driver
				
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;

				// call the driver to move the position of the image
				returnValue = callVdd(-1, 0);

				break;
			}
		case (STB_MONCTRL_HPOSITIONDEC): // Read/Write
			{
				/*	Pre:	TRUE

					Post:	If STB_MONCTRL_HPOSITION - STB_MONCTRL_HPOSITIONDEC < STB_MONCTRL_HPOSITIONMIN
							Then STB_MONCTRL_HPOSITION = STB_MONCTRL_HPOSITIONMIN
							Else STB_MONCTRL_HPOSITION = STB_MONCTRL_HPOSITION - STB_MONCTRL_HPOSITIONDEC
				*/

				//  Initialise output structure
				// This case moves the image displayed on the monitor left by calling the driver
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;

				// call the driver to move the position of the image
				returnValue = callVdd(1, 0);
				break;
			}
		case (STB_MONCTRL_VPOSITION): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_POSITION = TRUE 
							STB_MONDC12BCAPS0_POSITION = TRUE

					Post:	STB_MONCTRL_VPOSITION = pInData						
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}	
		case (STB_MONCTRL_VPOSITIONINC): // Read/Write
			{
				/*	Pre:	TRUE

					Post:	If STB_MONCTRL_VPOSITION + STB_MONCTRL_VPOSITIONINC > STB_MONCTRL_VPOSITIONMAX
							Then STB_MONCTRL_VPOSITION = STB_MONCTRL_VPOSITIONMAX
							Else STB_MONCTRL_VPOSITION = STB_MONCTRL_VPOSITION + STB_MONCTRL_VPOSITIONINC
				*/

				//  Initialise output structure
				// This case moves the image displayed on the monitor up by calling the driver
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;

				// call the driver to move the position of the image
				returnValue = callVdd(0, 1);
				break;
			}
		case (STB_MONCTRL_VPOSITIONDEC): // Read/Write
			{
				/*	Pre:	TRUE

					Post:	If STB_MONCTRL_VPOSITION - STB_MONCTRL_VPOSITIONDEC < STB_MONCTRL_VPOSITIONMIN
							Then STB_MONCTRL_VPOSITION = STB_MONCTRL_VPOSITIONMIN
							Else STB_MONCTRL_VPOSITION = STB_MONCTRL_VPOSITION - STB_MONCTRL_VPOSITIONDEC
				*/

				//  Initialise output structure
				/* 	This case moves the image displayed on the monitor down by calling the driver
					This is only used when the user has moved the image up and wants to move down
				 	The image cannot move down below its standard position as this causes problems
				 	with the mouse hotspot and cursor position
				*/
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;

				// call the driver to move the position of the image
				returnValue = callVdd(0, -1);
				break;
			}
		case (STB_MONCTRL_WIDTH): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_SIZE = TRUE
							STB_MONDDC12BCAPS0_SIZE = TRUE

					Post:	STB_MONCTRL_WIDTH = pInData							
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_WIDTHINC): // Read/Write
			{
				/*	Pre:	TRUE

					Post:	If STB_MONCTRL_WIDTH + STB_MONCTRL_WIDTHINC > STB_MONCTRL_WIDTHMAX
							Then STB_MONCTRL_WIDTH = STB_MONCTRL_WIDTHMAX
							Else STB_MONCTRL_WIDTH = STB_MONCTRL_WIDTH + STB_MONCTRL_WIDTHINC
				*/

				DWORD			dwStatus;
				long			scrUpdate[2] = {-16, 0};

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;

				dwDeviceHandle = _FF(DevNode);
				
				dwStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO,
								   dwDeviceHandle,
								   H3VDD_RESIZE_SCREEN,
								   (DWORD)((long _far*)scrUpdate),             
								   0);
				break;
			}
		case (STB_MONCTRL_WIDTHDEC): // Read/Write
			{
				/*	Pre:	TRUE

					Post:	If STB_MONCTRL_WIDTH - STB_MONCTRL_WIDTHDEC < STB_MONCTRL_WIDTHMIN
							Then STB_MONCTRL_WIDTH = STB_MONCTRL_WIDTHMIN
							Else STB_MONCTRL_WIDTH = STB_MONCTRL_WIDTH - STB_MONCTRL_WIDTHDEC
				*/
				DWORD			dwStatus;
				long			scrUpdate[2] = {16, 0};

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;

				dwDeviceHandle = _FF(DevNode);
				
				dwStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO,
								   dwDeviceHandle,
								   H3VDD_RESIZE_SCREEN,
								   (DWORD)((long _far*)scrUpdate),             
								   0);
			}
		case (STB_MONCTRL_HEIGHT): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_SIZE = TRUE
							STB_MONDDC12BCAPS0_SIZE = TRUE

					Post:	STB_MONCTRL_HEIGHT = pInData							
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_HEIGHTINC): // Read/Write
			{	
				/*	Pre:	TRUE

					Post:	If STB_MONCTRL_HEIGHT + STB_MONCTRL_HEIGHTINC > STB_MONCTRL_HEIGHTMAX
							Then STB_MONCTRL_HEIGHT = STB_MONCTRL_HEIGHTMAX
							Else STB_MONCTRL_HEIGHT = STB_MONCTRL_HEIGHT + STB_MONCTRL_HEIGHTINC
				*/

				DWORD			dwStatus;
				long			scrUpdate[2] = {0, -16};

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;

				dwDeviceHandle = _FF(DevNode);
				
				dwStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO,
								   dwDeviceHandle,
								   H3VDD_RESIZE_SCREEN,
								   (DWORD)((long _far*)scrUpdate),             
								   0);
				break;
			}
		case (STB_MONCTRL_HEIGHTDEC): // Read/Write
			{
				/*	Pre:	TRUE

					Post:	If STB_MONCTRL_HEIGHT - STB_MONCTRL_HEIGHTDEC < STB_MONCTRL_HEIGHTMIN
							Then STB_MONCTRL_HEIGHT = STB_MONCTRL_HEIGHTMIN
							Else STB_MONCTRL_HEIGHT = STB_MONCTRL_HEIGHT - STB_MONCTRL_HEIGHTDEC
				*/

				DWORD			dwStatus;
				long			scrUpdate[2] = {0, 16};

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;

				dwDeviceHandle = _FF(DevNode);
				
				dwStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO,
								   dwDeviceHandle,
								   H3VDD_RESIZE_SCREEN,
								   (DWORD)((long _far*)scrUpdate),             
								   0);
				break;
			}
		case (STB_MONCTRL_BRIGHTNESS): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_BRIGHTNESS = TRUE

					Post:	STB_MONCTRL_BRIGHTNESS = pInData
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_BRIGHTNESSINC): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_BRIGHTNESS = TRUE

					Post:	If STB_MONCTRL_BRIGHTNESS + STB_MONCTRL_BRIGHTNESSINC > STB_MONCTRL_BRIGHTNESSMAX
							Then STB_MONCTRL_BRIGHTNESS = STB_MONCTRL_BRIGHTNESSMAX
							Else STB_MONCTRL_BRIGHTNESS = STB_MONCTRL_BRIGHTNESS + STB_MONCTRL_BRIGHTNESSINC
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_BRIGHTNESSDEC): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_BRIGHTNESS = TRUE

					Post:	If STB_MONCTRL_BRIGHTNESS - STB_MONCTRL_BRIGHTNESSDEC < STB_MONCTRL_BRIGHTNESSMIN
							Then STB_MONCTRL_BRIGHTNESS = STB_MONCTRL_BRIGHTNESSMIN
							Else STB_MONCTRL_BRIGHTNESS = STB_MONCTRL_BRIGHTNESS - STB_MONCTRL_BRIGHTNESSDEC
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_CONTRAST): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_CONTRAST = TRUE

					Post:	STB_MONCTRL_CONTRAST = pInData
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_CONTRASTINC): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_CONMTRAST = TRUE

					Post:	If STB_MONCTRL_CONTRAST + STB_MONCTRL_CONTRASTINC > STB_MONCTRL_CONTRASTMAX
							Then STB_MONCTRL_CONTRAST = STB_MONCTRL_CONTRASTMAX
							Else STB_MONCTRL_CONTRAST = STB_MONCTRL_CONTRAST + STB_MONCTRL_CONTRASTINC
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_CONTRASTDEC): // Read/Write
			{
				/*	Pre:	STB_MONCTRLCAPS0_CONTRAST = TRUE

					Post:	If STB_MONCTRL_CONTRAST - STB_MONCTRL_CONTRASTDEC < STB_MONCTRL_CONTRASTMIN
							Then STB_MONCTRL_CONTRAST = STB_MONCTRL_CONTRASTMIN
							Else STB_MONCTRL_CONTRAST = STB_MONCTRL_CONTRAST - STB_MONCTRL_CONTRASTDEC
				*/

				//  Initialise output structure
				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;
				break;
			}
		case (STB_MONCTRL_SAVESETTINGS):  // Read/Write
			{
				/*  Pre:	

					Post:	The new settings for the position of the image on the monitor are saved to
							to the registry.
					
					This is called when the user clicks apply signally that he wants to save the new
					position to the registry.
				*/

				DWORD			dwStatus;
				long			scrUpdate[2] = {0, 0};

				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;

				dwDeviceHandle = _FF(DevNode);	//Get the device context of our card

				// call the driver to save the settings to the registry
				dwStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO,
								   dwDeviceHandle,
								   H3VDD_SET_MON_POS_REGISTRY,
								   (DWORD)((long _far*)scrUpdate),             
								   0);
				break;
			}

		case (STB_MONCTRL_RESTORESETTINGS):  // Read/Write
			{
				/*	Pre:	?

					Post:	The image has been returned to its previous position based on the
							values stored in the registry	
					
					This case restores the image on the monitor to its previous position
					and would be used if the user decided he wanted to return to his
					previous settings and click Reset to Previous
				*/

				DWORD			dwStatus;
				long			scrUpdate[2] = {0, 0};

				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;

				dwDeviceHandle = _FF(DevNode);

				// call the driver to restore the position of the image to its previous state	
				dwStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO,
								   dwDeviceHandle,
								   H3VDD_RESTORE_POS,
								   (DWORD)((long _far*)scrUpdate),             
								   0);

				break;
			}

		case (STB_MONCTRL_DEFAULTSETTINGS):
			{
				/*	Pre:

					Post: The image has been restore to its default position on the monitor	
					
					This case restores the image on the monitor to its factory default
					position.  It calls the driver to restore the factory default position
					if the user click Reset to Factory Defaults
				*/

				DWORD			dwStatus;
				long			scrUpdate[2] = {1, 0};

				((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
				((STB_PROPERTY *)pOutData)->ulResult=0;

				dwDeviceHandle = _FF(DevNode);

				// call the driver to restore the factory default position for the image				
				dwStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO,
								   dwDeviceHandle,
								   H3VDD_RESTORE_POS,
								   (DWORD)((long _far*)scrUpdate),             
								   0);
				break;
				
			}

		}

		return (returnValue = 1); // return 1 for the time being
}

