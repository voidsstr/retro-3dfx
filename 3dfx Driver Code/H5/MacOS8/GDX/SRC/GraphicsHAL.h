/*
	File:		GraphicsHAL.h

	Contains:	This is the declarations of routines that a HAL must implement.

	Written by:	Sean Williams, Kevin Williams

	Copyright:	© 1994-1995 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

		 <2>	 7/17/95	SW		For GraphicsHAL SetSync(), syncBitFieldValid is now input only.
		 <1>	 4/15/95	SW		First Checked In

*/

#ifndef __GRAPHICSHAL__
#define __GRAPHICSHAL__

#include "GraphicsPriv.h"
#include <NameRegistry.h>
#include <Devices.h>
#include <Types.h>
#include <Video.h>
#include <Kernel.h>





/* These are HAL prototypes that ALL HALs implement */

/* initialzation calls */
GDXErr GraphicsHALInitPrivateData(DriverRefNum refNum, const RegEntryID *regEntryID, Boolean *replacingDriver);
GDXErr GraphicsHALOpen(const AddressSpaceID spaceID, Boolean replacingDriver);
GDXErr GraphicsHALClose(const AddressSpaceID spaceID);
GDXErr GraphicsHALTerminate(Boolean superseded);
void GraphicsHALKillPrivateData(void);
Boolean GraphicsHALIsExclusive();

GDXErr GraphicsHALGetVBLInterruptRoutines(Boolean *installVBLInterrupts, Boolean *chainDefault,
		VBLHandler **halVBLHandler, VBLEnabler **halVBLEnabler, VBLDisabler **halVBLDisabler,
		void **vblRefCon);
GDXErr GraphicsHALSetSync(UInt8 syncBitField, UInt8 syncBitFieldValid);
GDXErr GraphicsHALGrayCLUT(const GammaTbl *gamma);
GDXErr GraphicsHALSetCLUT(const ColorSpec *originalCSTable, ColorSpec *correctedCSTable,
		SInt16 startPosition, SInt16 numberOfEntries, Boolean sequential, DepthMode depthMode);
GDXErr GraphicsHALGetCLUT(ColorSpec *csTable, SInt16 startPosition, SInt16 numberOfEntries,
		Boolean sequential, DepthMode depthMode);
GDXErr GraphicsHALGetPages(DisplayModeID displayModeID, DepthMode depthMode, SInt16 *pageCount);
GDXErr GraphicsHALGetBaseAddress(SInt16 page, char **baseAddress);
GDXErr GraphicsHALGetSync(Boolean getHardwareSyncCapability, VDSyncInfoRec *sync);
GDXErr GraphicsHALGetModeTiming(DisplayModeID displayModeID, UInt32 *timingFormat,
		UInt32 *timingFlags);
GDXErr GraphicsHALGetNextResolution(DisplayModeID previousDisplayModeID,
		DisplayModeID *displayModeID, DepthMode *maxDepthMode);
GDXErr GraphicsHALGetVideoParams(DisplayModeID displayModeID, DepthMode depthMode,
		UInt32 *bitsPerPixel, SInt16 *rowBytes);
GDXErr GraphicsHALSetPowerState(VDPowerStateRec *vdPowerState);
GDXErr GraphicsHALGetPowerState(VDPowerStateRec *vdPowerState);
GDXErr GraphicsHALGetMaxDepthMode(DisplayModeID displayModeID, DepthMode *maxDepthMode);
GDXErr GraphicsHALMapDepthModeToBPP(DepthMode depthMode, UInt32 *bitsPerPixel);
GDXErr GraphicsHALModePossible(DisplayModeID displayModeID, DepthMode depthMode, SInt16 page,
		Boolean *modePossible);
GDXErr GraphicsHALGetDefaultDisplayModeID(DisplayCode displayCode, DisplayModeID *displayModeID,
		DepthMode *depthMode);
GDXErr GraphicsHALProgramHardware(DisplayModeID displayModeID, DepthMode depthMode, SInt16 page,
		Boolean *directColor, char **baseAddress);
GDXErr GraphicsHALDrawHardwareCursor(SInt32 x, SInt32 y, Boolean visible);
GDXErr GraphicsHALSetHardwareCursor(const GammaTbl *gamma, Boolean luminanceMapping, void *cursorRef);
GDXErr GraphicsHALGetHardwareCursorDrawState(SInt32  *cursorX, SInt32  *cursorY,
		UInt32  *cursorVisible, UInt32  *cursorSet);
GDXErr GraphicsHALSupportsHardwareCursor(Boolean *supportsHardwareCursor);
GDXErr GraphicsHALDetermineDisplayCode(DisplayCode *displayCode);
GDXErr GraphicsHALGetSenseCodes(RawSenseCode *rawSenseCode, ExtendedSenseCode *extendedSenseCode,
		Boolean *standardInterpretation);
GDXErr GraphicsHALDDCMonitorAvailable(Boolean *ddcAvailable);
GDXErr GraphicsHALGetDDCBlock(UInt32 blockNumber, ResType blockType, UInt32 flags, Byte *data);

OSErr GraphicsHALPrivateControl(void *genericPtr, SInt16 privateControlCode);
OSErr GraphicsHALPrivateStatus(void *genericPtr, SInt16 privateStatusCode);

#endif	/* __GRAPHICSHAL__ */










