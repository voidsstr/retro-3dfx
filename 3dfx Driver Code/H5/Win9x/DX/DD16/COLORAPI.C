/* -*-c++-*- */
/* $Header: colorapi.c, 7, 10/11/00 8:50:48 PM, Brent$ */
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
** $Revision: 7$
** $Date: 10/11/00 8:50:48 PM$
**
** $Log: 
**  7    3dfx      1.5.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  6    3dfx      1.5         12/02/99 Reid Campbell   Restructured code - only
**       effects NT & WIN 2K functionality
**  5    3dfx      1.4         10/22/99 Scott Kephart   
**  4    3dfx      1.3         10/01/99 Reid Campbell   Adds extra colour API call
**       to determine the colour functionality from the driver.
**  3    3dfx      1.2         09/21/99 Xing Cong       Pass 0 instead of 9 in
**       assign_gammaramp() for desktop gamma.
**  2    3dfx      1.1         09/15/99 Xing Cong       Overlay gamma support
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $ 
*/


#define _TEXT(x) x
#include "colorapi.h"
#include <string.h>

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

#include "gramp.h"			// For gamma functions
#include "qmodes.h"

#include <stdlib.h>			// For defintion of _itoa
extern DISPLAYINFO DisplayInfo;
extern DWORD GammaTable[256];

// In order to get this to link we have to add this define and place 
// the include after all the previous includes.
// One of the driver files (which we can't find) appears to already 
// define the structure of a GUID without defining GUID_DEFINED. 
// This leads to a 'GUID redefinition error', nasty! 
#define GUID_DEFINED
#include "edgedefs.h"

    typedef struct _H3_SEARCH_TYPE
    {
      HKEY  hkey;
      char  *pszSubkey;
    } H3_SEARCH_TYPE;


int setupPalette();		//defined in h3.c
int GetDWORDValue( const char *varname, H3_SEARCH_TYPE H3SearchOrder[], int numSearchPaths, DWORD *dwBuffer);

int ColourAPIInit(void)
{
	/*
		Kept this small function in in case there are other things that need done at 
		init time
	*/


	return (ColourAPIResetGamma());

}


int ColourAPIGetGroupProperty(void * lpIndata,void * lpOutdata)
{
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

int ColourAPISetGroupProperty(void * lpIndata,void * lpOutdata)
{
	/******************************
		Escape handling Function
	 ******************************/

	switch (((STB_GROUPPROPERTY * )lpIndata)->ulPropertyId)
	{
	case (STB_SETGAMMA):
		{
			int i;

			// setup the gamma ramp from the table
			
			 for (i=0; i<256; i++)
			 {
				 GammaTable[i] =  ( (STB_GAMMA_TABLE * ) ((STB_GROUPPROPERTY * )lpIndata) ->ulPropertyValue )->GammaTable[i];
				 assign_gammaramp(i, 
					 (int)GammaTable[i] & 0xff,          // red 
					(int)(GammaTable[i]>>8) & 0xff,      // green 
					(int)(GammaTable[i]>>16) & 0xff,     // blue
                    0);                                  //for desktop
			 }
			 // setupPalette(); included folowing call to this function in h3.c
			 return(STB_ESCAPE_HANDLED);
		}
	case (STB_RESETGAMMA):
		{
			return(ColourAPIResetGamma());
		}
	default:
		{
			return(STB_ESCAPE_NOT_SUPPORTED);
		}
	}
}

int ColourAPIGetProperty(void * lpInData,void * lpOutData)
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
           
    		#ifdef WINNT /* Win NT - no overlay or D3D */    
               
           ((STB_PROPERTY *)lpOutData)->ulPropertyValue &= ~STB_COLOUR_CAPS0_OVERLAY
                                                        &  ~STB_COLOUR_CAPS0_D3D;
           #endif                                              

			#if (_WIN32_WINNT >= 0x0500) /* Win 2K - no overlay */    
               
           ((STB_PROPERTY *)lpOutData)->ulPropertyValue &= ~STB_COLOUR_CAPS0_OVERLAY;

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


int ColourAPIResetGamma(void)
{
    #define  	MAX_PATH_LENGTH 			100					// The maximum number of chars for a path to a reg. entry containing a gamma table
	#define	 	PATH_DESKTOP_GAMMA			3					// The number of search paths to check for the desktop gamma table
	#define		PATH_OVERLAY_GAMMA		 	1					// The number of search paths to check for the video overlay gamma table

	typedef union 
	{
		unsigned char c[4];
		DWORD d;
	} g_table;
	
	//  The 2 vars of type H3_SEARCH_TYPE contain the paths to their respective gamma tables in the registry
  	H3_SEARCH_TYPE 		SearchDesktopGamma[PATH_DESKTOP_GAMMA], SearchOverlayGamma[PATH_OVERLAY_GAMMA];
	DWORD 				TmpGammaTable[256], TmpGammaOverlayTable[256];
	/* 	result is whether each of the registry reads succeeds or not
		result[0]!=0 means that reading the desktop gamma succeeded
		result[1]!=0 means that reading the overlay gamma succeeded
	*/
	int 				i, result[2] = {0, 0};	
	char 				szNumber[3];
	/*	These arrays temporarily store the paths to the gamma tables 
	 	PATH_DESKTOP_GAMMA-1 because 2 search paths are the same only one searches in HKEY_LOCAL_MACHINE and the other in
		HKEY_CURRENT_USER
	*/
	char  				desktopKeyStr[PATH_DESKTOP_GAMMA-1][MAX_PATH_LENGTH], overlayKeyStr[PATH_OVERLAY_GAMMA][MAX_PATH_LENGTH];
	char 				szTemp[20];
	DWORD 				dwBuffer;

	// setup search paths for Desktop Gamma

	// setup the first search path 
	strcpy(desktopKeyStr[0], STB_GAMMA_REGPATH);
	strcat(desktopKeyStr[0], DESKTOPGAMMAPATH);
	SearchDesktopGamma[0].hkey = HKEY_LOCAL_MACHINE;
	SearchDesktopGamma[0].pszSubkey = desktopKeyStr[0];

	// setup the 2nd search path   
	strcpy(desktopKeyStr[1], STB_OLD_GAMMA_REGPATH);
	SearchDesktopGamma[1].hkey = HKEY_CURRENT_USER;
	SearchDesktopGamma[1].pszSubkey = desktopKeyStr[1];

	SearchDesktopGamma[2].hkey = HKEY_LOCAL_MACHINE;
	SearchDesktopGamma[2].pszSubkey = desktopKeyStr[1];

	// setup search path for Video Overlay Gamma
	// there is only one search path for Video Overlay
	strcpy(overlayKeyStr[0], STB_GAMMA_REGPATH);
	strcat(overlayKeyStr[0], OVERLAYGAMMAPATH);
	SearchOverlayGamma[0].hkey = HKEY_LOCAL_MACHINE;
	SearchOverlayGamma[0].pszSubkey =  overlayKeyStr[0];

	for ( i=0; i<256; i++ ) 
	{
		strcpy(szTemp, "GAMMA" ); 
		_itoa( i, szNumber, 10 );
	//	sprintf(szNumber,"%03d",i); 
		strcat(szTemp,szNumber); 

		// try to read from the registry the desktop gamma table
	  	if(GetDWORDValue(szTemp, SearchDesktopGamma, PATH_DESKTOP_GAMMA, &dwBuffer))
		{
			TmpGammaTable[i] = dwBuffer;
			result[0] = 1;			
		}
		// try to read from the registry the video overlay gamma table
		if(GetDWORDValue(szTemp, SearchOverlayGamma, PATH_OVERLAY_GAMMA, &dwBuffer))
		{
			TmpGammaOverlayTable[i] = dwBuffer;
			result[1] = 1;
		} 
	}
 
	// setup the gamma ramp from the table for the Video Overlay gamma if we have been successful
	// reading the values from the registry

  	if (result[1])
		for (i=0; i<256; i++)
        {
         	assign_gammaramp(i,
            	(int)TmpGammaOverlayTable[i] & 0xff,          	// red 
            	(int)(TmpGammaOverlayTable[i]>>8) & 0xff,      // green 
            	(int)(TmpGammaOverlayTable[i]>>16) & 0xff,     // blue
            	1);                                  		// 1 = for video overlay
         } 
         
	 // setup the gamma ramp from the table for the Desktop gamma if we have been successful
	 // in reading the values from the registry
     if (result[0])	
	 	for (i=0; i<256; i++)
        {
        	GammaTable[i] =  TmpGammaTable[i];
         	assign_gammaramp(i,
            	(int)GammaTable[i] & 0xff,          	// red 
            	(int)(GammaTable[i]>>8) & 0xff,      	// green 
            	(int)(GammaTable[i]>>16) & 0xff,     	// blue
            	0);                                  	// 0 = for desktop
         }
	 
     setupPalette();
	 
	 // return ESCAPE_NOT_SUPPORTED if either of the registry reads failed
	 if (!result[0] || !result[1])
	 	return (STB_ESCAPE_NOT_SUPPORTED);

	 return(STB_ESCAPE_HANDLED);


}

/***************************************************************************
 *
 * Function : GetDWORDValue( const char *varname, H3_SEARCH_TYPE H3SearchOder, int numSearchPaths, DWORD *dwBuffer  )
 *
 * The function searches the registry for the DWORD passed to it.
 * It searches in the location pointed to by the drivers devnode.
 * If it the function is successful it returns one, failure gives zero.
 * 
 * Updated by jmccartney 3dfx Belfast
 * The search path(s) is/are passed to the program in H3SearchOrder
 * The number of search path(s) is passed in numSearchPaths.
 *
 ***************************************************************************/
int GetDWORDValue( const char *varname, H3_SEARCH_TYPE H3SearchOrder[], int numSearchPaths, DWORD *dwBuffer  )
{
  #define  PRSIZE  50      // GetProfileString return buffer

  char    rstr[ PRSIZE ];
  char    DevNodeKey[MAX_VMM_REG_KEY_LEN];
  char    nstr[]="\0";
  char    *lpnull;
  char    *lprstr;
  DWORD   DevNode;
  int	  i;

  lprstr = rstr;
  lpnull = nstr;

  DevNode = _FF(DevNode);

  // convert devnode to a registry key
  // when successful, this thing returns a string something like
  // "System\CurrentControlSet\Services\Class\DISPLAY\XXXX"
  //
  if (CR_SUCCESS == CM_Get_DevNode_Key(DevNode,
                                        NULL,
                                        (PFARVOID)DevNodeKey,
                                        sizeof(DevNodeKey),
                                        CM_REGISTRY_SOFTWARE))
  { 	
	// moved this stuff to here from original position
	H3_SEARCH_TYPE *pSearchLoc;
    HKEY  hkey;
   	DWORD type;
    DWORD length;
    ULONG DevNodeKeyLength;

    // save original length of DevNodeKey
    DevNodeKeyLength = strlen(DevNodeKey);

	pSearchLoc = &H3SearchOrder[0];
    // loop over possible registry locations the number of search paths is passed in the 3rd var in the call to the function
    for (i = 0; i < numSearchPaths; i++)         
    {
      // if we have a non NULL subkey
      // tack the subkey to the end of the DevNodeKey
      if (NULL != pSearchLoc->pszSubkey)
        strcat(DevNodeKey, pSearchLoc->pszSubkey);

      // attempt to open the key
      if (ERROR_SUCCESS == RegOpenKey(pSearchLoc->hkey,
                                        DevNodeKey,
                                        &hkey))
      {
        // the key exists so attempt to read the value of varname
        length = sizeof(rstr);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey,
                                             (LPSTR)varname,
                                             0,
                                             &type,
                                             (LPBYTE)dwBuffer,
                                             &length))
        {
			  // the value exists so check it's type
			  // if it's a DWORD then return 1
			  // otherwise loop to the next search location

			if (REG_DWORD ==type) // we got a DWORD
				RegCloseKey(hkey);
				return 1;
			}

			RegCloseKey(hkey);
      }

      // restore the original DevNodeKey, in case we tacked on a subkey above
      DevNodeKey[DevNodeKeyLength] = '\0';
	  // move onto to the next search path
	  pSearchLoc++;
    }
  }

  // Return zero if not found 
  return 0;
}



