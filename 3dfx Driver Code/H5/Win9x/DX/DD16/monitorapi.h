/****************************************************************************/
/*
** Copyright© 1999 STB Systems Inc.  All rights reserved.
** Project			: Edge Tools
** Target Name		: 
** Author #1		: Paul Magee
** Author #2		: 
** Purpose			: Defines calls and structures for monitor api
** Uses Libraries	: 
** Date Started		: February 99
** Date Completed	: 
** Update History #1: 
**
** File Name              : monitorapi.h
** Source Safe Location   : \edgetools\common\; \3dfx\voodoo3\dd16
** Checked in by          : pmagee
*/
/****************************************************************************/
#ifndef STB_MONITOR__H
#define STB_MONITOR__H

//Monitor Control capabilities
#define STB_MONCTRLCAPS0_POSITION		0x00000001
#define STB_MONCTRLCAPS0_SIZE			0x00000002
#define STB_MONCTRLCAPS0_DDC12B			0x00000004

//Monitor DDC12B capabilities
#define STB_MONDDC12BCAPS0_POSITION		0x00000001
#define STB_MONDDC12BCAPS0_SIZE			0x00000002
#define STB_MONDDC12BCAPS0_BRIGHTNESS	0x00000004
#define STB_MONDDC12BCAPS0_CONTRAST		0x00000008


/* ****************************************************** */
/*
** Control Properties, used when calling into the API to
** get information
**
*/
/* ****************************************************** */

#define STB_MONITOR_START			1
#define STB_MONITOR_CONTROL_CAPS	STB_MONITOR_START

#define STB_MONCTRL_HPOSITION		STB_MONITOR_START + 1	/* read &write */
#define STB_MONCTRL_HPOSITIONMAX	STB_MONITOR_START + 2	/* read only */
#define STB_MONCTRL_HPOSITIONMIN	STB_MONITOR_START + 3	/* read only */
#define STB_MONCTRL_HPOSITIONINC	STB_MONITOR_START + 4
#define STB_MONCTRL_HPOSITIONDEC	STB_MONITOR_START + 5

#define STB_MONCTRL_VPOSITION		STB_MONITOR_START + 6	/* read & write */
#define STB_MONCTRL_VPOSITIONMAX	STB_MONITOR_START + 7	/* read only */
#define STB_MONCTRL_VPOSITIONMIN	STB_MONITOR_START + 8	/* read only */
#define STB_MONCTRL_VPOSITIONINC	STB_MONITOR_START + 9
#define STB_MONCTRL_VPOSITIONDEC	STB_MONITOR_START + 10

#define STB_MONITOR_DDC12B_CAPS		STB_MONITOR_START + 11

#define STB_MONCTRL_WIDTH			STB_MONITOR_START + 12
#define STB_MONCTRL_WIDTHMAX		STB_MONITOR_START + 13
#define STB_MONCTRL_WIDTHMIN		STB_MONITOR_START + 14
#define STB_MONCTRL_WIDTHINC		STB_MONITOR_START + 15
#define STB_MONCTRL_WIDTHDEC		STB_MONITOR_START + 16

#define STB_MONCTRL_HEIGHT			STB_MONITOR_START + 17
#define STB_MONCTRL_HEIGHTMAX		STB_MONITOR_START + 18
#define STB_MONCTRL_HEIGHTMIN		STB_MONITOR_START + 19
#define STB_MONCTRL_HEIGHTINC		STB_MONITOR_START + 20
#define STB_MONCTRL_HEIGHTDEC		STB_MONITOR_START + 21

#define STB_MONCTRL_BRIGHTNESS		STB_MONITOR_START + 22
#define STB_MONCTRL_BRIGHTNESSMAX	STB_MONITOR_START + 23
#define STB_MONCTRL_BRIGHTNESSMIN	STB_MONITOR_START + 24
#define STB_MONCTRL_BRIGHTNESSINC	STB_MONITOR_START + 25
#define STB_MONCTRL_BRIGHTNESSDEC	STB_MONITOR_START + 26

#define STB_MONCTRL_CONTRAST		STB_MONITOR_START + 27
#define STB_MONCTRL_CONTRASTMAX		STB_MONITOR_START + 28
#define STB_MONCTRL_CONTRASTMIN		STB_MONITOR_START + 29
#define STB_MONCTRL_CONTRASTINC		STB_MONITOR_START + 30
#define STB_MONCTRL_CONTRASTDEC		STB_MONITOR_START + 31

#define STB_MONCTRL_SAVESETTINGS	STB_MONITOR_START + 32
#define STB_MONCTRL_RESTORESETTINGS	STB_MONITOR_START + 33
#define STB_MONCTRL_DEFAULTSETTINGS	STB_MONITOR_START + 34

/* ********************* */
/*
** Function Prototypes
*/
/* ********************* */

int MonitorAPISetProperty(void * pInData,void * pOutData);
int MonitorAPIGetProperty(void * pInData,void * pOutData);

#endif /* STB_MONITOR__H */




