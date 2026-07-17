/******************************************************************************
	File:		SRM_Prefs.h
	Copyright:	© 1995-2000 Critical Path Software Inc.  All Rights Reserved.
	Author:		Andrew Mellinger
	Purpose:	This is the public header for the Prefernce file API.
	History:	
 ******************************************************************************/
#pragma once

	/* Headers */
/******************************************************************************/
#include <MacTypes.h>

	/* Constants */
#ifdef __cplusplus
extern "C"
{
#endif

#define	kMaxNumberOfBoards		16

/******************************************************************************/
enum
{
	// Preference flags
	kPrefsFlag2D			= 0x00000001,
	kPrefsFlag3D			= 0x00000002,
	kPrefsFlagQT			= 0x00000004,

	// Version number
	kHRMPrefsVersionCurrent = 0x00000002,

	// Change this part of the version number if non-backwards-compatible changes are made.
	kHRMPrefsVersionMajorMask = 0xFFFF0000,

	// Flags to disable parts of the 2D acceleration

	// 2D Proc flags (set to 1 to disable each blitProc)
	kDisableBitBlit			= 0x00000001,	// BitBlit, RgnBlit, ScaleBlit
	kDisablePatBlit			= 0x00000002,	// PatBlit, PatRgnBlit
	kDisableLineBlit		= 0x00000004,
	kDisableSlabBlit		= 0x00000008,

	// patch flags (set to 1 to disable each patch)
	kDisableTextPatches		= 0x00000100,
	kDisableStdBitsPatch	= 0x00000200,
	kDisableDrawPictPatch	= 0x00000400,

	// high-level flags 
	kBitAccurateOnly		= 0x00010000,
	/* Set kBitAccurateOnly to disable all operations which don't have bit-accurate 
		results.  This would currently include:
		- 16->32 bit depth conversion in hardware
		- dither copy
		- maybe stretch blits?
	*/
	
	kDisableVRAMGWorlds		= 0x00020000,
	kDisableAGPGWorlds		= 0x00040000,
	
	// Flags to disable parts of the QT acceleration
	
	// Disable individual codecs
	kDisableQTCodec_raw		= 0x00000001,
	kDisableQTCodec_yuvs	= 0x00000002,
	kDisableQTCodec_yuv2	= 0x00000004,
	kDisableQTCodec_mpyc	= 0x00000008,
	
	// Don't use overlay plane
	kDisableQTOverlay		= 0x00000100,
	// Don't use stretch blits
	kDisableQTFrontend		= 0x00000200
};

	/* Typedefs */
/******************************************************************************/
typedef struct
{
	UInt32				version;
	UInt32				size;
} HRMPrefsHeader;

typedef struct
{
	HRMPrefsHeader		header;
	UInt32				antialiasing;
	UInt32				antialiasingKey;
	UInt32				screenShotKey;
	float				gammaValue;
	UInt32				VSyncEnable;	
	UInt32				swappending;	
	UInt32				force32bit;	
} HRMPrefs3DOpenGL;

typedef struct
{
	HRMPrefsHeader		header;
	UInt32				antialiasing;
	UInt32				antialiasingKey;
	UInt32				screenShotKey;
	float				gammaValue;
	UInt32				VSyncEnable;	
	UInt32				swappending;	
	UInt32				force32bit;	
} HRMPrefs3DGlide;

typedef struct
{
	HRMPrefsHeader		header;
	UInt32				antialiasing;
	UInt32				antialiasingKey;
	UInt32				screenShotKey;
	float				gammaValue;
	UInt32				VSyncEnable;	
	UInt32				swappending;	
	UInt32				force32bit;	
} HRMPrefs3DRAVE;

typedef struct
{
	HRMPrefsHeader		header;
	UInt32				disableFlags;	
} HRMPrefs2D;

typedef struct
{
	HRMPrefsHeader		header;
	HRMPrefs3DOpenGL	ogl;
	HRMPrefs3DGlide		glide;
	HRMPrefs3DRAVE		rave;
} HRMPrefs3D;

typedef struct
{
	HRMPrefsHeader		header;
	UInt32				disableFlags;	
} HRMPrefsQT;

typedef struct
{
	HRMPrefsHeader		header;
	HRMPrefs2D *		m2D[kMaxNumberOfBoards];
	HRMPrefs3D *		m3D[kMaxNumberOfBoards];
	HRMPrefsQT *		mQT[kMaxNumberOfBoards];
} HRMPrefsTable;

typedef void(*PrefsChangedFn)(void);

	/* Globals */
	/* Classes */
	/* Prototypes */
/******************************************************************************/
	// Usage Notes:
	// When a component initializes, it should request the current preferences
	// from the preference manager.

HRMPrefsTable	*hrm_GetPrefsTable();
OSStatus		hrm_RegisterPrefsChanged(PrefsChangedFn fn, UInt32 prefsFlags);
OSStatus		hrm_UnregisterPrefsChanged(PrefsChangedFn fn);

	// For the UI to set the prefs.
OSStatus	hrm_ChangePrefs(HRMPrefsTable *newPrefsTable, UInt32 flags);

	// Called by the HRM to set up the prefs manager.
OSStatus hrm_InitPrefs();

#ifdef __cplusplus
}
#endif

	/* Macros */
/******************************************************************************/
/*----------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
