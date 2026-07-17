/* -*-c++-*- */
/* $Header: colorapi.h, 5, 10/11/00 8:57:25 PM, Brent$ */
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
** File name:   colorapi.h
**
** Description: Provide interface for the STB Colour API.
**
** $Revision: 5$
** $Date: 10/11/00 8:57:25 PM$
**
** $Log: 
**  5    3dfx      1.3.3.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  4    3dfx      1.3         10/22/99 Scott Kephart   
**  3    3dfx      1.2         10/06/99 Dan O'Connel    Update ColourAPIGetProperty
**       interface to work for WinNT and Win2K.
**  2    3dfx      1.1         10/01/99 Reid Campbell   Adds extra colour API call
**       to determine the colour functionality from the driver.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $ 
*/

#ifndef EDGEGAM_H
#define EDGEGAM_H


#ifdef WINNT
int ColourAPIInit(PDEV * ppdev);
int ColourAPIResetGamma(PDEV * ppdev);
int ColourAPISetGroupProperty(void * lpIndata,void * lpOutdata, PDEV * ppdev);
int ColourAPIGetGroupProperty(void * lpIndata,void * lpOutdata, PDEV * ppdev);
int ColourAPIGetProperty(void * lpInData,void * lpOutData, PDEV * ppdev);
#else
int ColourAPIInit(void);
int ColourAPIResetGamma(void);
  
int ColourAPISetGroupProperty(void * lpIndata,void * lpOutdata);
int ColourAPIGetGroupProperty(void * lpIndata,void * lpOutdata);
int ColourAPIGetProperty(void * lpInData,void * lpOutData);
#endif //def WINNT


/* Definition of Escape Calls */

#define STB_COLOUR_API_START			1

#define STB_COLOUR_CAPS				STB_COLOUR_API_START
#define STB_RESETGAMMA				STB_COLOUR_API_START + 1
#define STB_SETGAMMA				STB_COLOUR_API_START + 3




/* flags for STB_COLOUR_CAPS */

#define STB_COLOUR_CAPS0_DESKTOP		0x0001
#define STB_COLOUR_CAPS0_OVERLAY		0x0002
#define STB_COLOUR_CAPS0_D3D			0x0004
#define STB_COLOUR_CAPS0_OPENGL			0x0008
#define STB_COLOUR_CAPS0_GLIDE			0x0010

#endif //_H