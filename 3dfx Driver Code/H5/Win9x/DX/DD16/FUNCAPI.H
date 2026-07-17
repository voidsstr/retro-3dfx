/* -*-c++-*- */
/* $Header: funcapi.h, 5, 10/11/00 8:51:10 PM, Brent$ */
/*
** Copyright (c) 2000, 3Dfx Interactive, Inc.
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
** File name:   funcapi.h
**
** Description: Defines calls and structures for function api.
**
** $Revision: 5$
** $Date: 10/11/00 8:51:10 PM$
**
** $Log: 
**  5    3dfx      1.3.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  4    3dfx      1.3         02/23/00 Dan O'Connel    Port Reid Campbell's
**       funcapi.c changes to support SLI from Win9x to Win2K/WinNT4.
**  3    3dfx      1.2         02/08/00 Reid Campbell   New interface added for
**       over clocking
** 
** 
**  2    3dfx      1.1         12/02/99 Dan O'Connel    Preliminary round of
**       changes to support new mechanism in 3dfx Tools to turn on and off
**       monitor/DFP/TvOut.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $ 
*/
#include "3dfx.h"

#ifndef STB_FUNCAPI__H
#define STB_FUNCAPI__H

#define STB_FUNCTION_CAPS_START			0

#define STB_FUNCTION_CAPS				STB_FUNCTION_CAPS_START
#define STB_FUNCTION_ACTIVE				STB_FUNCTION_CAPS_START +1
#define STB_SIM_VGA						STB_FUNCTION_CAPS_START +2
#define STB_SIM_TV						STB_FUNCTION_CAPS_START +3
#define STB_SIM_DFP						STB_FUNCTION_CAPS_START +4
#define STB_DISPLAY_ENUM				STB_FUNCTION_CAPS_START +5
#define STB_FUNCTION_CANCEL             STB_FUNCTION_CAPS_START +6
#define STB_FUNCTION_COMMIT             STB_FUNCTION_CAPS_START +7
#define STB_FUNCTION_SIM				STB_FUNCTION_CAPS_START +8
#define STB_FUNCTION_DEFAULT			STB_FUNCTION_CAPS_START +9
#define STB_FUNCTION_CLOCKRATE			STB_FUNCTION_CAPS_START +10
#define STB_FUNCTION_CLOCKDEFAULT  		STB_FUNCTION_CAPS_START +11
#define STB_FUNCTION_CLOCKMAX			STB_FUNCTION_CAPS_START +12
#define STB_FUNCTION_CLOCKMIN			STB_FUNCTION_CAPS_START +13


/* return values for STB_FUNCTION_CAPS */
/* set to binary bit values for easy comparison */
/* do we want the following areas in separate containers */
/* or in a single one? */

#define STB_FUNCTIONCAPS0_VGA			0x01
#define STB_FUNCTIONCAPS0_TV			0x02
#define STB_FUNCTIONCAPS0_DFP			0x04

/* return values for STB_FUNCTION_ACTIVE */

#define STB_FUNCTIONACTIVE0_VGA			0x01
#define STB_FUNCTIONACTIVE0_TV			0x02
#define STB_FUNCTIONACTIVE0_DFP			0x04

/* defines for STB_SIM */
#define STB_FUNCTION_VGA				0x01
#define STB_FUNCTION_TV					0x02
#define STB_FUNCTION_DFP				0x04

/* return values for STB_SIM_VGA, STB_SIM_TV & STB_SIM_DFP */
/* functions that can work simultaneously with VGA output */

#define STB_SIM0_VGA					0x01 /* always set when called for VGA */
#define STB_SIM0_TV						0x02/* always set when called for TV */
#define STB_SIM0_DFP					0x04 /* always set when called for DFP */

#ifdef WINNT
int FunctionAPIGetProperty(void * pInData,void * pOutData, PDEV * ppdev);
int FunctionAPISetProperty(void * pInData,void * pOutData, PDEV * ppdev);
int FunctionAPIGetGroupProperty(void * pInData,void * pOutData, PDEV * ppdev);
int FunctionAPISetGroupProperty(void * pInData,void * pOutData, PDEV * ppdev);
int FunctionAPIInit(PDEV * ppdev);
#else
int FunctionAPIGetProperty(void * pInData,void * pOutData);
int FunctionAPISetProperty(void * pInData,void * pOutData);
int FunctionAPIGetGroupProperty(void * pInData,void * pOutData);
int FunctionAPISetGroupProperty(void * pInData,void * pOutData);
int FunctionAPIInit();
#endif
FxU32	PLL2MHz(FxU32 clock );


#endif /* _FUNCAPI__H */




