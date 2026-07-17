/********************************************************************************
	 DynamicPatches.h
	 	
	 Function declarations and structures to support dynamic patching.
	 
	 This file, as well as DynamicPatches.cp are intended to be included in HRM
	 clients wishing to make system patches.
	 
	 To patch new functions, search for ADD_PATCH and follow the comments.

	 Chall Fry
	 Critical Path Software
*/
#pragma once

// Includes
#include <MixedMode.h>
#include <Quickdraw.h>
#include <QDOffscreen.h>
#include <Traps.h>
#include <Patches.h>

// ADD_PATCH: May have to add an #include here, to get types included
#include <Fonts.h>

#ifdef __cplusplus
extern "C" {
#endif

class PatchStdBits
{
public:
	enum {
	// StdBits procinfo
		kProcInfo = kPascalStackBased | RESULT_SIZE(kNoByteCode) |
			STACK_ROUTINE_PARAMETER(1, kFourByteCode) |
			STACK_ROUTINE_PARAMETER(2, kFourByteCode) |
			STACK_ROUTINE_PARAMETER(3, kFourByteCode) |
			STACK_ROUTINE_PARAMETER(4, kTwoByteCode)  |
			STACK_ROUTINE_PARAMETER(5, kFourByteCode),

		kTrapNum = _StdBits
	};
	
	// The type of the trap call
	typedef void (*PatchFnType)(BitMap *srcBits, Rect *srcRect, Rect *dstRect,
			short mode, RgnHandle maskRgn);

	// The function we're written to handle the trap call
	static void PatchFn(BitMap *srcBits, Rect *srcRect, Rect *dstRect,
				short mode, RgnHandle maskRgn);
};

class PatchStdText
{
public:
	enum {
	// StdText procinfo
		kProcInfo = kPascalStackBased | RESULT_SIZE(kNoByteCode) |
			STACK_ROUTINE_PARAMETER(1, kTwoByteCode)  |
			STACK_ROUTINE_PARAMETER(2, kFourByteCode) |
			STACK_ROUTINE_PARAMETER(3, kFourByteCode) |
			STACK_ROUTINE_PARAMETER(4, kFourByteCode),

		kTrapNum = _StdText
	};	
	
	// The type of the trap call
	typedef void (*PatchFnType)(short byteCount, UInt8 *textBuf, Point numer, Point denom);

	// The function we're written to handle the trap call
	static void PatchFn(short byteCount, UInt8 *textBuf, Point numer, Point denom);
};

class PatchStdTextMeas
{
public:
	enum {
		// StdTextMeas procinfo
		kProcInfo = kPascalStackBased | RESULT_SIZE(kTwoByteCode) |
			STACK_ROUTINE_PARAMETER(1, kTwoByteCode)  |
			STACK_ROUTINE_PARAMETER(2, kFourByteCode) |
			STACK_ROUTINE_PARAMETER(3, kFourByteCode) |
			STACK_ROUTINE_PARAMETER(4, kFourByteCode) |
				STACK_ROUTINE_PARAMETER(5, kFourByteCode),

		kTrapNum = _StdTxMeas
	};	
	
	// The type of the trap call
	typedef short (*PatchFnType)(short byteCount, UInt8 *textBuf, Point *numer, 
					Point *denom, FontInfo *info);

	// The function we're written to handle the trap call
	static short PatchFn(short byteCount, UInt8 *textBuf, Point *numer, 
					Point *denom, FontInfo *info);
};

class PatchDrawPicture
{
public:
	enum {
		// StdTextMeas procinfo
		kProcInfo = kPascalStackBased | RESULT_SIZE(kNoByteCode) |
				STACK_ROUTINE_PARAMETER(1, kFourByteCode)  |
				STACK_ROUTINE_PARAMETER(2, kFourByteCode),

		kTrapNum = _DrawPicture
	};	
	
	// The type of the trap call
	typedef void (*PatchFnType)(PicHandle picture, Rect *destRect);

	// The function we're written to handle the trap call
	static void PatchFn(PicHandle picture, Rect *destRect);
};

class PatchNewGWorld
{
public:
	enum {
		// NewGWorld procinfo
		kProcInfo = kPascalStackBased | RESULT_SIZE(kTwoByteCode) |
				STACK_ROUTINE_PARAMETER(1, kFourByteCode)  |
				STACK_ROUTINE_PARAMETER(2, kTwoByteCode) |
				STACK_ROUTINE_PARAMETER(3, kFourByteCode) |
				STACK_ROUTINE_PARAMETER(4, kFourByteCode) |
				STACK_ROUTINE_PARAMETER(5, kFourByteCode) |
				STACK_ROUTINE_PARAMETER(6, kFourByteCode),

		kTrapNum = _QDExtensions,
		
		
		kSelectorProcInfo = kD0DispatchedPascalStackBased | RESULT_SIZE(kTwoByteCode) |
				DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(kFourByteCode) |
				DISPATCHED_STACK_ROUTINE_PARAMETER(1, kFourByteCode)  |
				DISPATCHED_STACK_ROUTINE_PARAMETER(2, kTwoByteCode) |
				DISPATCHED_STACK_ROUTINE_PARAMETER(3, kFourByteCode) |
				DISPATCHED_STACK_ROUTINE_PARAMETER(4, kFourByteCode) |
				DISPATCHED_STACK_ROUTINE_PARAMETER(5, kFourByteCode) |
				DISPATCHED_STACK_ROUTINE_PARAMETER(6, kFourByteCode),

		kTrapSelector = 0x00160000
	};	
	
	// The type of the trap call
	typedef short (*PatchFnType)(GWorldPtr *				offscreenGWorld,
								short 					PixelDepth,
								const Rect *			boundsRect,
								CTabHandle 				cTable,
								GDHandle 				aGDevice,
								GWorldFlags 			flags);

	// The function we're written to handle the trap call
	static short PatchFn(GWorldPtr *				offscreenGWorld,
						short 					PixelDepth,
						const Rect *			boundsRect,
						CTabHandle 				cTable,
						GDHandle 				aGDevice,
						GWorldFlags 			flags);
};

class PatchMenuSelect
{
public:
	enum {
		// MenuSelect procinfo
		kProcInfo = kPascalStackBased | RESULT_SIZE(kFourByteCode) |
				STACK_ROUTINE_PARAMETER(1, kFourByteCode),

		kTrapNum = _MenuSelect
	};	
	
	// The type of the trap call
	typedef long (*PatchFnType)(Point startPt);

	// The function we're written to handle the trap call
	static long PatchFn(Point startPt);
};

// ADD_PATCH:: Add a new Patch____ class, like those above



// Constants

const UInt32	kPatchGroupVersion	= 2;


// Types

class PatchGroup
{
public:
	void Init();

	UInt32				version;
	UInt32				structSize;
	UInt32				isToolbox;
	PatchGroup			*nextPatchGroup;
	
	// ADD_PATCH: Add a field to this struct AT THE BOTTOM of the list of PatchFnTypes. 
	// Bump the struct's version #, and reduce the size of the reseved block by 1.
	PatchStdBits::PatchFnType		stdBitsPatch;
	PatchStdText::PatchFnType		stdTextPatch;
	PatchStdTextMeas::PatchFnType	stdTextMeasPatch;
	PatchDrawPicture::PatchFnType	drawPicturePatch;
	PatchNewGWorld::PatchFnType		newGWorldPatch;
	PatchMenuSelect::PatchFnType	menuSelectPatch;
	
	
	UInt32		reserved[16];
};

// Function Declarations
	
typedef void (*hrmInitPatchManagerPtr)();
typedef void (*hrmRegisterPatchesPtr)(PatchGroup *patches);
typedef void (*hrmDeregisterPatchesPtr)(PatchGroup *patches);

void InitDynamicPatches(PatchGroup *patches);
void RemoveDynamicPatches();


// HRM functions for installing Patches
void hrm_InitPatchManager();
void hrm_RegisterPatches(PatchGroup *patches);
void hrm_DeregisterPatches(PatchGroup *patches);

// Globals
extern PatchGroup gPatchGroup;

#ifdef __cplusplus
}
#endif


