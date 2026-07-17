/********************************************************************************
	 DynamicPatches.cp
	 	
	 This file is part of the HRM Patch Manager. This file is compiled into the HRM,
	 *and* into the HRM client. Rules of operations are as follows:
	 
	 The HRM actually installs the OS patches (i.e., the HRM calls NSetTrapAddress).
	 The HRM can never go away after installing the OS Patch.
	 At installation time (before any clients attach themselves to the Patch Manager),
	 	the HRM simply passes all trap calls it's patching through to the OS. This 
	 	is accomplished through the Voodoo_XXXPatch functions below.
	 When clients register with the HRM (using the function InitDynamicPatches), 
	 	the HRM will begin calling the client when OS traps are called. 
	 The client can then call RemoveDynamicPatches, and they will no longer be called.
	 
	 
	 
	 
	 What not to do:
	 
	 Unloading the HRM is bad.
	 Unloading a client while registered is bad.
	 
	 
*/

// Include files

#include "DynamicPatches.h"
#include "HDWR_Res_Mgr.h"

// Globals
PatchGroup	gPatchGroup;


/********************************************************************************
	PatchGroup::PatchGroup
		
	
*/
void PatchGroup::Init()
{
	// Clear out the patch group
	for (SInt32 index = 0; index < sizeof(PatchGroup); ++index)
			((char *) this)[index] = 0;

	version = kPatchGroupVersion;
	structSize = sizeof(PatchGroup);
	isToolbox = false;
}

/********************************************************************************
	InitDynamicPatches
		PatchGroup *patches
		
	This function should only be called by clients of the HRM, not the HRM itself.
	The caller should set to 0 any PatchGroup patch fields they don't want to patch.
*/
void InitDynamicPatches(PatchGroup *patches)
{	
	gPatchGroup = *patches;
	
	// Register the patch group
	hrm_RegisterPatches(&gPatchGroup);
}

/********************************************************************************
	RemoveDynamicPatches
		
	
*/
void RemoveDynamicPatches()
{
	hrm_DeregisterPatches(&gPatchGroup);
}

/********************************************************************************
	PatchStdBits::PatchFn
		BitMap *srcBits
		Rect *srcRect
		Rect *dstRect
		short mode
		RgnHandle maskRgn
*/
void PatchStdBits::PatchFn(BitMap *srcBits, Rect *srcRect, Rect *dstRect,
		short mode, RgnHandle maskRgn)
{
	PatchStdBits::PatchFnType	nextPatch;
	PatchGroup					*listIter = &gPatchGroup;
	
	do {
		listIter = listIter->nextPatchGroup;
		nextPatch = listIter->stdBitsPatch;
	} while (!nextPatch);
	
	if (listIter->isToolbox)
		CallUniversalProc((RoutineDescriptor *) nextPatch, PatchStdBits::kProcInfo,
				srcBits, srcRect, dstRect, mode, maskRgn);
	else
		nextPatch(srcBits, srcRect, dstRect, mode, maskRgn);
}

/********************************************************************************
	PatchStdText::PatchFn
		short byteCount
		UInt8 *textBuf
		Point numer
		Point denom
*/
void  PatchStdText::PatchFn(short byteCount, UInt8 *textBuf, Point numer, Point denom)
{
	PatchStdText::PatchFnType	nextPatch;
	PatchGroup					*listIter = &gPatchGroup;
	
	do {
		listIter = listIter->nextPatchGroup;
		nextPatch = listIter->stdTextPatch;
	} while (!nextPatch);
	
	if (listIter->isToolbox)
		CallUniversalProc((RoutineDescriptor *) nextPatch, PatchStdText::kProcInfo,
				byteCount, textBuf, numer, denom);
	else
		nextPatch(byteCount, textBuf, numer, denom);
}

/********************************************************************************
	PatchStdTextMeas::PatchFn
		short byteCount
		UInt8 *textBuf
		Point *numer
		Point *denom
		FontInfo *info	
*/
short  PatchStdTextMeas::PatchFn(short byteCount, UInt8 *textBuf, Point *numer, 
			Point *denom, FontInfo *info)
{
	PatchStdTextMeas::PatchFnType	nextPatch;
	PatchGroup						*listIter = &gPatchGroup;
	
	do {
		listIter = listIter->nextPatchGroup;
		nextPatch = listIter->stdTextMeasPatch;
	} while (!nextPatch);
	
	if (listIter->isToolbox)
		return CallUniversalProc((RoutineDescriptor *) nextPatch, PatchStdBits::kProcInfo,
				byteCount, textBuf, numer, denom, info);
	else
		return nextPatch(byteCount, textBuf, numer, denom, info);
}

/********************************************************************************
	PatchDrawPicture::PatchFn
		PicHandle picture
		Rect *destRect
*/
void  PatchDrawPicture::PatchFn(PicHandle picture, Rect *destRect)
{
	PatchDrawPicture::PatchFnType	nextPatch;
	PatchGroup						*listIter = &gPatchGroup;
	
	do {
		listIter = listIter->nextPatchGroup;
		nextPatch = listIter->drawPicturePatch;
	} while (!nextPatch);
	
	if (listIter->isToolbox)
		CallUniversalProc((RoutineDescriptor *) nextPatch, PatchDrawPicture::kProcInfo,
				picture, destRect);
	else
		nextPatch(picture, destRect);
}


/********************************************************************************
	PatchNewGWorld::PatchFn
		GWorldPtr *				offscreenGWorld,
		short 					PixelDepth,
		const Rect *			boundsRect,
		CTabHandle 				cTable,
		GDHandle 				aGDevice,
		GWorldFlags 			flags
*/
short  PatchNewGWorld::PatchFn(GWorldPtr *				offscreenGWorld,
								short 					PixelDepth,
								const Rect *			boundsRect,
								CTabHandle 				cTable,
								GDHandle 				aGDevice,
								GWorldFlags 			flags)
{
	PatchNewGWorld::PatchFnType	nextPatch;
	PatchGroup						*listIter = &gPatchGroup;
	
	do {
		listIter = listIter->nextPatchGroup;
		nextPatch = listIter->newGWorldPatch;
	} while (!nextPatch);
	
	if(listIter->isToolbox)
	{
		return CallUniversalProc(
			(RoutineDescriptor *) nextPatch, 
			PatchNewGWorld::kSelectorProcInfo,
			0x00160000,	// Dispatched trap selector
			offscreenGWorld, 
			PixelDepth, 
			boundsRect, 
			cTable, 
			aGDevice, 
			flags);
	}
	else
	{
		return nextPatch(
			offscreenGWorld, 
			PixelDepth, 
			boundsRect, 
			cTable, 
			aGDevice, 
			flags);
	}
}

/********************************************************************************
	PatchMenuSelect::PatchFn
		Point startPt
*/
long  PatchMenuSelect::PatchFn(Point startPt)
{
	PatchMenuSelect::PatchFnType	nextPatch;
	PatchGroup						*listIter = &gPatchGroup;
	long result;
		
	do {
		listIter = listIter->nextPatchGroup;
		nextPatch = listIter->menuSelectPatch;
	} while (!nextPatch);
	
	if (listIter->isToolbox)
		result = CallUniversalProc((RoutineDescriptor *) nextPatch, PatchMenuSelect::kProcInfo,
				startPt);
	else
		result = nextPatch(startPt);
	
	return result;
}

	// ADD_PATCH: Add a function here
