/*
	File:		GraphicsPriv.h

	Contains:	This file is private to GDX items.  It contains declarations for items that
				are strictly internal to the GDX model.

	Written by:	Sean Williams, Kevin Williams

	Copyright:	© 1994-1995 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

		 <1>	 4/15/95	SW		First Checked In

*/

#ifndef __GRAPHICSPRIV__
#define __GRAPHICSPRIV__

#include <Types.h>
#include <Video.h>
#include <3Dfx.h>
#include <DCon.h>

/* When writing to hardware in the C world, register address need to be declared  */
/* as volatile.  Why?  Often a register is written to, and then read immediately. */
/* C, optimizing away programmers intentions, will not do the read.               */
typedef volatile  UInt8		HWRegister8Bit;
typedef volatile  UInt16	HWRegister16Bit;
typedef volatile  UInt32	HWRegister32Bit;



/*
** GDXErr
**	These error codes are private to the GDX model, and are not seen by any of the
**	Graphics driver clients.  They are used for debugging purposes, so the driver
**	writer will have more resolution than is provided by 'controlErr' or 'statusErr'.
*/
typedef SInt32 GDXErr;
enum
{
	kGDXErrUnknownError = -4096,				/* Sloppy programming if ever seen */

	kGDXErrNoError = 0,
	kGDXErrInvalidParameters = -1,				/* Invalid parameters were provided */

	kGDXErrDriverAlreadyOpen = -2,				/* Driver is already open and got another open command */
	kGDXErrDriverAlreadyClosed = -3,			/* Driver is closed and got another close command */
	kGDXErrRequestedModeNotPossible = -4,		/* Invalid depth mode and/or display page requested */
	kGDXErrDisplayModeIDUnsupported = -5,		/* Frame buffer cannot support requested DisplayModeID */
	kGDXErrMonitorUnsupported = -6,				/* The connected monitor type is not supported */
	kGDXErrDepthModeUnsupported = -7,			/* Frame buffer cannot support requested DepthMode */
	kGDXErrInvalidForIndexedDevice = -8,		/* An invalid operation was attempted for a indexed device */
	kGDXErrInvalidForDirectDevice = -9,			/* An invalid operation was attempted for a direct device */
	kGDXErrUnableToMapDepthModeToBPP = -10,		/* Cannot relative pixel depth to absolute pixel depth */
	kGDXErrInvalidColorSpecTable = -11,			/* Client supplied invalid ColorSpec table */
	kGDXErrUnableToAllocateColorSpecTable = -12,/* Unable to allocate memory for a ColorSpec table */
	kGDXErrUnableToAllocateGammaTable = -13,	/* Unable to allocate memory for a gamma table */
	kGDXErrUnableToAllocateCoreData = -14,		/* Unable to allocate memory for a Core data */
	kGDXErrUnableToAllocateHALData = -15,		/* Unable to allocate memory for a HAL data */
	kGDXErrUnableToAllocateData = -16,			/* Unable to allocate misc data */
	kGDXErrInvalidGammaTable = -17,				/* Client supplied an invalid gamma table. */
	kGDXErrInvalidResolutionForMonitor = -18,	/* Monitor doesn't support requested resolution */
	kGDXErrNoConnectedMonitor = -19,			/* No monitor connected */
	kGDXErrOSSPropertyNameLengthTooLong = -20,	/* PropertyName exceeded the maximum length */
	kGDXErrOSSPropertyStorageInvalid = -21,		/* OSS PropertyStorage invalid */
	kGDXErrOSSUnableToSavePropertyStorage= -22,	/* OSS PropertyStorage could not be saved */
	kGDXErrOSSNoProperyNameAndValue = -23,		/* Unable to find the propertyName and propertyValue */
	kGDXErrOSSUnableToGetPropertyValue = -24,	/* Property exists but unable to Get the propertyValue */
	kGDXErrOSSPropertySizeExceedsBuffer = -25,	/* PropertySize exceeds the buffer size */
	kGDXErrOSSUnexpectedPropertySize = -26,		/* PropertySize didn't match.  propetyValue is bad */
	kGDXErrOSSUnableToSetPropertyValue = -27,	/* Unable to set a propertyValue for a property we know exists */
	kGDXErrOSSUnableToCreateProperty = -28,		/* Unable to create a property */
	kGDXErrOSSUknownMappedDisplayModeID = -29,	/* Mapped displayModeID (grpf) is unknown */
	kGDXErrOSSUknownMappedDepthMode = -30,		/* Mapped depthMode (grpf) is unknown */
	kGDXErrOSSUknownMappedDisplayCode = -31,	/* Mapped displayCode (grpf) is unknown */
	kGDXErrOSSNoISTProperty = -32,				/* No kISTPropertyName for the RegEntryID */
	kGDXErrOSSNoDefaultVBLRoutines = -33,		/* GetInterruptFunctions (Interrupts.h) failed */
	kGDXErrOSSUnableToInstallVBLRoutines = -34,	/* InstallInterruptFunctions (Interrupts.h) failed */
	kGDXErrOSSUnableToInstallVSLService = -35,	/* VSLNewInterruptService (VideoServices.h) failed */
	kGDXErrOSSInterruptSourceStillActive = -36,	/* Can't dispose of VSL services since interrupts ON */
	kGDXErrOSSUnableToDisposeVSLService = -37,	/* VSLDisposeInterruptService (VideoServices.h) failed */
	kGDXErrUnableToSizeVRAM = -38,				/* Failed VRAM sizing */
	kGDXErrNoHardwareCursorSet = -39,			/* No hardware cursor has been set */
	kGDXErrCannotRenderCursorImage = -40,		/* Specific cursor image cannot be rendered by hardware */
	kGDXErrUnsupportedFunctionality = -41,		/* Functionally (such as hw cursor, power saving) not supported */
	kGDXErrDriverCannotRun = -42,				/* Driver can't (or shouldn't) be run. */

	kGDXErrPad = 0x7fffffff						/* Pad to avoid warning of dangling ',' */
};



/*
** GammaTableID
**	This abstract data type is used to reference gamma tables.
**	(The typedef is declared in the API, so just the enumerated constants appear here)
*/
enum
{
	kGammaTableIDStandard = 2000,				/* Mac Standard Gamma */
	kGammaTableIDPageWhite = 2001,				/* Page-White Gamma */
	kGammaTableIDGray = 2002,					/* Mac Gray Gamma */
	kGammaTableIDRubik = 2004,					/* Mac RGB Gamma */
	kGammaTableIDNTSCPAL = 2005,				/* NTSC/PAL Gamma */
	kGammaTableIDCSCTFT = 2006					/* Active Matrix Color LCD Gamma */
};



/*
** DisplayModeID
**	This abstract data type is used to reference display resolutions.
**	The examples of monitors listed do NOT constitute the full list for a given DisplayModeID.
**	Instead, they are just intended only to be a represetative sample.
**
**	DisplayModeIDs shouldn't change arbitrarily, even though the are completely private to the driver.
**	This is because the DisplayModeID is stored in NVRAM, and it could result in confusion if
**	a ROM based driver interprets the NVRAM items one way, and a subsequent disk based driver
**	interprets them differently.
*/
#if 0
enum
{
	kDisplay512x384At60HzNTSC = 1,			/* NTSC Safe Title */
	kDisplay512x384At60Hz = 2,				/* Apple's 12" RGB */
	kDisplay640x480At50HzPAL = 3,			/* PAL Safe Title */
	kDisplay640x480At60HzNTSC = 4,			/* NTSC Full Frame */
	kDisplay640x480At60HzVGA = 5,			/* Typical VGA monitor */
	kDisplay640x480At67Hz = 6,				/* Apple's 13" & 14" RGB */
	kDisplay640x870At75Hz = 7, 				/* Apple's Full Page Display (Portrait) */
	kDisplay768x576At50HzPAL = 8,			/* PAL Full Frame */
	kDisplay800x600At56HzVGA = 9,			/* SVGA	(VESA Standard) */
	kDisplay800x600At60HzVGA = 10,			/* SVGA	(VESA Standard) */
	kDisplay800x600At72HzVGA = 11,			/* SVGA	(VESA Standard) */
	kDisplay800x600At75HzVGA = 12,			/* SVGA at a higher refresh rate */
	kDisplay832x624At75Hz = 13,				/* Apple's 16" RGB */
	kDisplay1024x768At60HzVGA = 14,			/* VESA 1K-60Hz */
	kDisplay1024x768At72HzVGA = 15,			/* 72Hz (So it records well on TV) */
	kDisplay1024x768At75HzVGA = 16,			/* VESA 1K-75Hz....higher refresh rate */
	kDisplay1024x768At75Hz = 17,			/* Apple's 19" RGB */
	kDisplay1152x870At75Hz = 18,			/* Apple's 21" RGB */
	kDisplay1280x960At75Hz = 19,			/* Square Pixel version of 1280x1024 */
	kDisplay1280x1024At75Hz = 20,			/* ??? */
	kDisplay256x192At60HzNTSCZoom = 21,		/* NTSC Safe Title timing, but 1/4 pixels */
	kDisplay320x240At50HzPALZoom = 22,		/* PAL Safe Title timing, but 1/4 pixels */
	kDisplay320x240At60HzNTSCZoom = 23,		/* NTSC Full Frame timing, but 1/4 pixels */
	kDisplay384x288At50HzPALZoom = 24,		/* PAL Full Frame timing, but 1/4 pixels */

	kOneGreaterThanLastDisplayModeID,							/* Let this value be auto-incremented */
	kMaxDisplayModeIDs = kOneGreaterThanLastDisplayModeID - 1	/* so this value is always correct. */
};
#else
/* Avenger Display Modes, frome modetabl.h */
enum
{
	kDisplay640x480At60Hz = 1,
	kDisplay640x480At67Hz = 2,
	kDisplay640x480At72Hz = 3,
	kDisplay640x480At75Hz = 4,
	kDisplay640x480At85Hz = 5,
	kDisplay640x480At100Hz = 6,
	kDisplay640x480At120Hz = 7,
  kDisplay640x480At140Hz = 8,
  kDisplay640x480At160Hz = 9,
  
	kDisplay800x600At56Hz = 10,
	kDisplay800x600At60Hz = 11,
	kDisplay800x600At72Hz = 12,
	kDisplay800x600At75Hz = 13,
	kDisplay800x600At85Hz = 14,
	kDisplay800x600At100Hz = 15,
	kDisplay800x600At120Hz = 16,
  kDisplay800x600At140Hz = 17,
  kDisplay800x600At160Hz = 18,

  kDisplay800x500At60Hz = 19,
  kDisplay800x512At60Hz = 20,

  kDisplay832x624At75Hz = 21,
  kDisplay864x480At60Hz = 22,

	kDisplay1024x768At60Hz = 23,
	kDisplay1024x768At70Hz = 24,
	kDisplay1024x768At72Hz = 25,
	kDisplay1024x768At74_9Hz = 26,
	kDisplay1024x768At75Hz = 27,
	kDisplay1024x768At85Hz = 28,
	kDisplay1024x768At100Hz = 29,
	kDisplay1024x768At120Hz = 30,

	kDisplay1072x600At60Hz = 31,

	kDisplay1152x864At60Hz = 32,
	kDisplay1152x864At75Hz = 33,
	kDisplay1152x864At85Hz = 34,
	kDisplay1152x864At100Hz = 35,

	kDisplay1152x870At75Hz = 36,			/* Apple's 21" RGB */

	kDisplay1280x960At60Hz = 37,
	kDisplay1280x960At75Hz = 38,
	kDisplay1280x960At85Hz = 39,

	kDisplay1280x1024At60Hz = 40,
	kDisplay1280x1024At75Hz = 41,
	kDisplay1280x1024At85Hz = 42,
	kDisplay1280x1024At100Hz = 43,

	kDisplay1376x768At60Hz = 44,

  kDisplay1600x1024At60Hz = 45,
  kDisplay1600x1024At76Hz = 46,
  kDisplay1600x1024At85Hz = 47,

	kDisplay1600x1200At60Hz = 48,
	kDisplay1600x1200At65Hz = 49,
	kDisplay1600x1200At70Hz = 50,
	kDisplay1600x1200At75Hz = 51,
	kDisplay1600x1200At80Hz = 52,
	kDisplay1600x1200At85Hz = 53,
  kDisplay1600x1200At100Hz = 54,

	kDisplay1792x1344At60Hz = 55,
	kDisplay1792x1344At75Hz = 56,

	kDisplay1856x1392At60Hz = 57,
  kDisplay1856x1392At75Hz = 58,
          
  kDisplay1920x1080At60Hz = 59,
  kDisplay1920x1080At72Hz = 60,
  kDisplay1920x1080At75Hz = 61,
  kDisplay1920x1080At85Hz = 62,
  
  kDisplay1920x1200At60Hz = 63,
	kDisplay1920x1200At76Hz = 64,
	kDisplay1920x1200At85Hz = 65,
	
	kDisplay1920x1440At60Hz = 66,
	kDisplay1920x1440At75Hz = 67,
	
	kDisplay2048x1536At60Hz = 68,
	kDisplay2048x1536At75Hz = 69,
	kDisplay2048x1536At85Hz = 70,
	
	kOneGreaterThanLastDisplayModeID,							/* Let this value be auto-incremented */
	kMaxDisplayModeIDs = kOneGreaterThanLastDisplayModeID - 1	/* so this value is always correct. */
};
#endif


enum
{
	kDVI640x480 = 1,
};

/*
** DisplayCode
**	This abstract data type is used to represent what type of display is physically connected to
**	the frame buffer controller.  It is important to note that NO ATTEMPT should be made to directly
**	map a DisplayCode from raw or exteneded sense codes.
**	All monitors are considered to be RGB, unless explicitly marked otherwise.
**	The examples of displays listed in the comments represent only a some instances of that type, not
**	necessarily the full set.
**
**	WARNING! since the the DisplayCode is saved into NVRAM (allocated 1 byte of space), NO DisplayCode
**	should be greater than 255.
**
**	Again, just as with DisplayModeIDs, these values should not be arbitrarily changed, thus avoid
**	confusion between ROM based and disk based drivers.
*/
typedef SInt32 DisplayCode;
enum
{
	kDisplayCodeNoDisplay 			= 0,			/* No display attached */
	kDisplayCodeUnknown				= 1,			/* A display is present, but it's type is unknown */
	kDisplayCode12Inch				= 2,			/* 12" RGB (Rubik) */
	kDisplayCodeStandard			= 3,			/* 13"/14" RGB or 12" Monochrome */
	kDisplayCodePortrait 			= 4,			/* 15" Portait RGB (Manufactured by Radius) */
	kDisplayCodePortraitMono 		= 5,			/* 15" Portrait Monochrome */
	kDisplayCode16Inch				= 6,			/* 16" RGB (GoldFish) */
	kDisplayCode19Inch				= 7,			/* 19" RGB (Third Party) */
	kDisplayCode21Inch				= 8,			/* 21" RGB (Vesuvio, Radius 21" RGB) */
	kDisplayCode21InchMono			= 9,			/* 21" Monochrome (Kong, Radius 21" Mono) */
	kDisplayCodeVGA 				= 10,			/* VGA  */
	kDisplayCodeNTSC		 		= 11,			/* NTSC */
	kDisplayCodePAL 				= 12,			/* PAL  */
	kDisplayCodeMultiScanBand1		= 13,			/* MultiScan Band-1 (12" thru 16" resolutions) */
	kDisplayCodeMultiScanBand2		= 14,			/* MultiScan Band-3 (13" thru 19" resolutions) */
	kDisplayCodeMultiScanBand3		= 15,			/* MultiScan Band-3 (13" thru 21" resolutions) */
	kDisplayCodeDVI                 = 16			/* DVI */
};

#define REFRESH_50HZ	0x00320000
#define REFRESH_56HZ	0x00000000
#define REFRESH_NTSC	0x003BF080
#define REFRESH_60HZ	0x003C0000
#define REFRESH_65HZ	0x00000000
#define REFRESH_67HZ	0x0042AA80
#define REFRESH_70HZ	0x00000000
#define REFRESH_72HZ	0x00480000
#define REFRESH_75HZ	0x004B0000
#define REFRESH_80HZ	0x00000000
#define REFRESH_85HZ	0x00000000
#define REFRESH_100HZ	0x00000000
#define REFRESH_120HZ	0x00000000


/*
** SenseLine
**	This abstract data type is used to reference the various monitor sense lines used by frame buffer
**	controllers which can implement Apple's Sense Line Protocol.
*/
typedef UInt32 SenseLine;
enum
{
	kSenseLineA = 0,			/* Corresponds to Monitor Sense Line 2 (pin 10 on a DB-15 connector) */
	kSenseLineB = 1,			/* Corresponds to Monitor Sense Line 1 (pin 7 on a DB-15 connector) */
	kSenseLineC = 2				/* Corresponds to Monitor Sense Line 0 (pin 4 on a DB-15 connector) */
};



/*
** OSSPropertyStorage
**	This abstract data type specifies how the driver can save information.  The information can be
**	NonVolatile (kOSSPropertyAvailableAtBoot, kOSSPropertyAvailableAtDisk) or Volatile.
**	The OSS insulates the rest of the driver from whatever storage mechanism is used by the OS
*/
typedef UInt32 OSSPropertyStorage;
enum
{
	kOSSPropertyAvailableAtBoot,
	kOSSPropertyAvailableAtDisk,
	kOSSPropertyVolatile
};



/*
** GraphicsPreferred
**	This structure describes the PropertyValue that is needed for the
**	GetPreferredConfiguration and SetPreferredConfiguration calls.  The DisplayCode is saved in
**	addition to the DisplayModeID and the DepthMode since the core looks at the DisplayCode from the
**	previous boot.  Since only 8 bytes of NVRAM are available to store the data, the OSS will map
**	the GraphicsPreferred structure into a GraphicsNonVolatile structure (defined in GraphicsOSS.c)
*/
typedef struct GraphicsPreferred GraphicsPreferred;
struct GraphicsPreferred
{
    DisplayModeID displayModeID;
    DepthMode depthMode;
	DisplayCode displayCode;				/* Save the DisplayCode in case user switches monitors */
};


/*
/* GraphicsHAL VBL handler prototypes
/*	Function definitions for the HALs to handle real vbl's
*/
typedef void VBLHandler(void* vblRefCon);
typedef void VBLEnabler(void *vblRefcon);
typedef Boolean VBLDisabler(void *vblRefcon);


/*
** DisplayModeIDData
**	All the DisplayModeIDs need to be examined by the Core/OSS for GetNextResolution,
**	GetVideoParamenters and when the OSS compresses the 'gprf' data to be in 8 bytes.
**	This table combines all the information into one table
*/
typedef struct DisplayModeIDData DisplayModeIDData;
struct DisplayModeIDData
{
  DisplayModeID displayModeID;			/* The DisplayModeID in question */
	Fixed refreshRate;						/* GetNextResolution/GetVideoParams data */
	UInt16 horizontalPixels; 				/* GetNextResolution/GetVideoParams data */
	UInt16 verticalLines;					/* GetNextResolution/GetVideoParams data */
	FxU32 timingFlags;
	FxU32 timingData;
};

extern DisplayModeIDData displayModeIDMap[];

#endif	/* __GRAPHICSPRIV__ */
