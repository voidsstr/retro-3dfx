/* -*-c++-*- */
/* $Header: colorapi.c, 4, 10/11/00 8:45:02 PM, Brent$ */
/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** File name:   colorapi.c
**
** Description: Propvides functions for colour control.
**
** $Revision: 4$
** $Date: 10/11/00 8:45:02 PM$
**
** $Log: 
**  4    3dfx      1.2.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  3    Napalm    1.2         01/28/00 Dan O'Connel    Correct ifdefs to allow
**       3dfx Tools to attempt to support D3d on the color page as intended.
**  2    Napalm    1.1         12/02/99 Reid Campbell   Modified code to return
**       correct capabilities for NT and Win 2K
**  1    Napalm    1.0         10/06/99 Dan O'Connel    
** $ */


#include "precomp.h"

#include "colorapi.h"
#include "edgedefs.h"


int ColourAPIInit(PDEV * ppdev)
{
// *** this function is not used yet for NT or Win2K ***
	/*
		Kept this small function in in case there are other things that need done at 
		init time
	*/


	return (ColourAPIResetGamma(ppdev));

}


int ColourAPIGetGroupProperty(void * lpIndata,void * lpOutdata, PDEV * ppdev)
{
// *** this function is not used yet for NT or Win2K ***
	/******************************
		Escape handling Function
	 ******************************/
/*
	switch (((STB_GROUPPROPERTY * )lpIndata)->ulPropertyId)
	{
	default:
		{
		}
	}
*/

	return(STB_ESCAPE_NOT_SUPPORTED);
}

int ColourAPISetGroupProperty(void * lpIndata,void * lpOutdata, PDEV * ppdev)
{
// *** this function is not used yet for NT or Win2K ***
    return(STB_ESCAPE_NOT_SUPPORTED);
}

int ColourAPIGetProperty(void * lpInData,void * lpOutData, PDEV * ppdev)
{
	
   int returnValue= STB_ESCAPE_HANDLED;
	switch (((STB_PROPERTY *)lpInData)->ulPropertyId)
	{
		case (STB_COLOUR_CAPS):
		{
			
           /* Setup for all properties (Win98) */
           
			((STB_PROPERTY *)lpOutData)->Guid = ((STB_PROPERTY *)lpInData)->Guid;
			((STB_PROPERTY *)lpOutData)->ulPropertyId = ((STB_PROPERTY *)lpInData)->ulPropertyId;
			((STB_PROPERTY *)lpOutData)->ulResult = 0;


			((STB_PROPERTY *)lpOutData)->ulPropertyValue = 0x0000 | STB_COLOUR_CAPS0_DESKTOP
                                                                 | STB_COLOUR_CAPS0_OVERLAY
                                                                 | STB_COLOUR_CAPS0_D3D
                                                                 | STB_COLOUR_CAPS0_OPENGL
                                                                 | STB_COLOUR_CAPS0_GLIDE;
           	
               
           #ifdef H3 /* Banshee - no overlay */    
               
           ((STB_PROPERTY *)lpOutData)->ulPropertyValue &= ~STB_COLOUR_CAPS0_OVERLAY;
                                                      
           #endif                                              
           
            #ifdef WINNT
               
            #if (_WIN32_WINNT >= 0x0500) /* Win 2K - no overlay */    
               
            ((STB_PROPERTY *)lpOutData)->ulPropertyValue &= ~STB_COLOUR_CAPS0_OVERLAY;

            #else  /* Win NT4 - no overlay or D3D */    
       
            ((STB_PROPERTY *)lpOutData)->ulPropertyValue &= ~STB_COLOUR_CAPS0_OVERLAY
                                                        &  ~STB_COLOUR_CAPS0_D3D;
            #endif                                              
            #endif                                              


			break;
		}
		default:
		{
			break; 
		}
	}
   
	return returnValue;

}


int ColourAPIResetGamma(PDEV * ppdev)
{
// *** this function is not used yet for NT or Win2K ***
    return(STB_ESCAPE_NOT_SUPPORTED);

}
