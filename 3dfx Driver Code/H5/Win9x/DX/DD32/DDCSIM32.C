/* $Header: ddcsim32.c, 2, 10/11/00 8:51:57 PM, Brent$ */
/*
** Copyright (c) 1998-1999, 3Dfx Interactive, Inc.
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
** File Name:    DDCSIM32.C
**
** Description:  
**	
** $Revision: 2$
** $Date: 10/11/00 8:51:57 PM$
**
** $History: ddcsim32.c $
** 
** *****************  Version 8  *****************
** User: Michael      Date: 12/30/98   Time: 4:47p
** Updated in $/devel/h3/Win95/dx/dd32
** Implement the 3Dfx/STB unified header.
**
 * 
 * 7     1/16/98 6:23p Miriam
 * texture download & general D3D support.
*/

#include "windows.h"
#include "ddrawi.h"
#include "hw.h"
#include "header.h"
#include "ddglobal.h"
#include "wingdi.h"

/*----------------------------------------------------------------------
Function name: h3Write32

Description:   

Return:        NONE
----------------------------------------------------------------------*/

void __stdcall h3Write32(volatile void * hwptr, unsigned long data) 
{
  HDC       hdc;
  DWORD     pInData[2];

  hdc = GetDC(0);
  pInData[0]= (DWORD) hwptr;
  pInData[1]= (DWORD) data;
    
  ExtEscape(
    hdc,                      //  handle to device context
    EXT_DDHAL_CSIM_WRITE,     //  escape function
    8,                        //  number of bytes in input structure
    (const char*) pInData,    //  pointer to input structure
    0,                        //  number of bytes in output structure
    NULL);                    //  pointer to output structrue
    
  ReleaseDC(NULL,             // handle of window
            hdc);             // handle of device context
}// h3Write32

/*----------------------------------------------------------------------
Function name: h3Write32f

Description:   

Return:        NONE
----------------------------------------------------------------------*/

void __stdcall h3Write32f(volatile void * hwptr, float data) 
{
  HDC       hdc;
  DWORD     pInData[2];

  hdc = GetDC(0);
  pInData[0]= (DWORD) hwptr;
  pInData[1]= (DWORD) * (unsigned long *) &data;
    
  ExtEscape(
    hdc,                      //  handle to device context
    EXT_DDHAL_CSIM_WRITE,     //  escape function
    8,                        //  number of bytes in input structure
    (const char*) pInData,    //  pointer to input structure
    0,                        //  number of bytes in output structure
    NULL);                    //  pointer to output structrue
    
  ReleaseDC(NULL,             // handle of window
            hdc);             // handle of device context
}// h3Write32f

/*----------------------------------------------------------------------
Function name: hRead32

Description:   

Return:        DWORD outData
----------------------------------------------------------------------*/

DWORD __stdcall h3Read32(volatile void * hwptr)
{
  DWORD outData, inData;

  HDC   hdc;
  
  inData = (DWORD) hwptr;
 
  hdc = GetDC(0);
  ExtEscape(
    hdc,                        //  handle to device context
    EXT_DDHAL_CSIM_READ,        //  escape function
    4,                          //  number of bytes in input structure
    (const char*) &inData,      //  pointer to input structure
    4,                          //  number of bytes in output structure
    (char*) &outData);          //  pointer to output structrue
      
  ReleaseDC(NULL, // handle of window
            hdc); // handle of device context
  return outData;
}// hRead32
