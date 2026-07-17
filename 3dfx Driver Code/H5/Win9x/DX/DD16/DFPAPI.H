/****************************************************************************/
/*
** Copyright© 1999 STB Systems Inc.  All rights reserved.
** Project			: Edge Tools
** Target Name		: 
** Author #1		: Ryan Bissell
** Author #2		: 
** Purpose			: Provide interface the the STB DFP API
** Uses Libraries	: 
** Date Started		: April 99
** Date Completed	: 
** Update History #1: 
**
** File Name              : dfpapi.h
** Source Safe Location   : \3dfx\voodoo3\dd16\; \edgetools\common\driver\
** Checked in by          : pmagee
*/
/****************************************************************************/

#ifndef DFPAPI_H
#define DFPAPI_H
#include "3dfx.h"



////////////////////#include "capabilities.h"


/* Digital Flat Panel properties */


#define STB_DFP_START					0

#define STB_DFP_CAPS					STB_DFP_START	
#define STB_DFP_DISPLAYCONTROL			STB_DFP_START + 1
#define STB_DFP_DFPCONNECTED			STB_DFP_START + 2
#define STB_DFP_AUTOSCALE				STB_DFP_START + 3
#define STB_DFP_BRIGHTNESS				STB_DFP_START + 4
#define STB_DFP_BRIGHTNESSMAX			STB_DFP_START + 5
#define STB_DFP_BRIGHTNESSMIN			STB_DFP_START + 6
#define STB_DFP_BRIGHTNESSINC			STB_DFP_START + 7
#define STB_DFP_BRIGHTNESSDEC			STB_DFP_START + 8
#define STB_DFP_CONTRAST				STB_DFP_START + 9
#define STB_DFP_CONTRASTMAX				STB_DFP_START + 10
#define STB_DFP_CONTRASTMIN				STB_DFP_START + 11
#define STB_DFP_CONTRASTINC				STB_DFP_START + 12
#define STB_DFP_CONTRASTDEC				STB_DFP_START + 13

#define STB_DFP_ON 1
#define STB_DFP_OFF 0
#define STB_DFP_CONNECTED 1

/* function return values for the DFP API */


/* return values for STB_DFP_CAPS */

#define STB_DFPCAPS0_AUTOSCALE			0x01
#define STB_DFPCAPS0_BRIGHTNESS			0x02
#define STB_DFPCAPS0_CONTRAST          0x04


/* function declaration headers */

#ifdef WINNT
void DFPCancelSettings(void * pInData,void * pOutData, PDEV * ppdev);

int DFPGetProperty(void * pInData,void * pOutData, PDEV * ppdev);
int DFPSetProperty(void * pInData,void * pOutData, PDEV * ppdev);
int DFPGetGroupProperty(void * pInData,void * pOutData, PDEV * ppdev);
int DFPSetGroupProperty(void * pInData,void * pOutData, PDEV * ppdev);
FxBool DFPisPanelPresent(PDEV * ppdev);
FxBool DFPisPanelActive(PDEV * ppdev);
FxBool DFPisAdapterDfpCapable(PDEV * ppdev);
#else
void DFPCancelSettings(void * pInData,void * pOutData);

int DFPGetProperty(void * pInData,void * pOutData);
int DFPSetProperty(void * pInData,void * pOutData);
int DFPGetGroupProperty(void * pInData,void * pOutData);
int DFPSetGroupProperty(void * pInData,void * pOutData);
FxBool DFPisPanelPresent();
FxBool DFPisPanelActive();
FxBool DFPisPanelScaling();
FxBool DFPisAdapterDfpCapable();
void DFPgetUniqueName(char * namePtr );
#endif

#endif //_H

