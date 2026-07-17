/*
	File:		GraphicsCorePriv.h

	Contains:	This has declarations that are private to the graphics 'core.'  Neither the HALs nor
				the OSS need any of this.

	Written by:	Sean Williams, Kevin Williams

	Copyright:	© 1994-1995 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

		 <1>	 4/15/95	SW		First Checked In

*/

#ifndef __GRAPHICSCOREPRIV__
#define __GRAPHICSCOREPRIV__

#include "GraphicsPriv.h"

#include <Types.h>
#include <Devices.h>
#include <NameRegistry.h>
#include <Video.h>


/*
** GraphicsCoreData
**	This structure contains the 'globals' needed to maintain the necessary state
**	information regarding the graphics core.
*/
typedef struct GraphicsCoreData GraphicsCoreData;
struct GraphicsCoreData
{
	RegEntryID regEntryID;			/* RegEntryID describing Graphics HW */
	AddressSpaceID spaceID;			/* grabbed in Initialize */
	DriverRefNum driverRefNum;  /* Reference number of driver */
 	DepthMode depthMode;			  /* Relative bit depth */
	UInt32 bitsPerPixel;			  /* Absolute bit depth */
	DisplayModeID displayModeID;	/* Current display mode selector */
	DisplayCode displayCode;		/* DisplayCode for the connected display */
	
	/* All the information that is needed for Core/OSS for all DisplayModeIDs */
	DisplayModeIDData masterTable[kMaxDisplayModeIDs];

	SInt16 currentPage;				/* Current graphics page (0 based) */
	void *baseAddress;				/* Base address of current page of frame buffer */
	GammaTbl *gammaTable;			/* Current gamma table */
	unsigned long maxGammaTableSize;/* Biggest gamma table allocated..reuse existing gamma if can fit */
	Boolean luminanceMapping;		/* True if using luminance mapping */
	Boolean	monoOnly;				/* True if attached display only support Monochrome */
	Boolean directColor;			/* True for direct color, false for	indexed color */
	Boolean interruptsEnabled;		/* True when VBL interrupts are enabled */
	Boolean driverOpen;				/* True the driver is opened */
	Boolean replacingDriver;		/* True if got a kReplaceCommand instead of kInitializeCommand */
};


/* Prototype for access function to core data */
#if 0
GraphicsCoreData *GraphicsCoreGetCoreData(void);
#else
#define GraphicsCoreGetCoreData() (&gCoreData)
#endif

extern GraphicsCoreData gCoreData;				/*  Persistant globals for the Core's private data. */


#endif	/* __GRAPHICSCOREPRIV__ */
