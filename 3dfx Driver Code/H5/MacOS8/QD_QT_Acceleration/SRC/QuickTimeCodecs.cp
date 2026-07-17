/*
	QuickTime Codecs.cp
	
	QuickTime acceleration codecs for Voodoo3/4/5 hardware
	
	Monroe Williams
	Critical Path Software
	1999.12.
*/

#include <Memory.h>
#include <Resources.h>
#include <Quickdraw.h>	
#include <QDOffscreen.h>
#include <OSUtils.h>
#include <Errors.h>
#include <FixMath.h>
#include <CodeFragments.h>

#include "ImageCodec.h"

// All this is imported from the QuickDraw acceleration.
#include <NQDAcceleration.h>
#include "h3defs.h"
#include "h3gdefs.h"
#include "minihwc.h"
#include "hwcio.h"
#include <GraphicsPrivHwc.h>
#include "Utilities.h"
#include "FifoClasses.h"
#include "RegionParser.h"
#include "hrm_mem.h"
#include "hrm_overlay_surface.h"
#include "hrm_arbitration.h"

#include "QuickTimeCodecs.h"
#include "BitBlit.h"

#pragma mark ¥ options

// This enables the code that uses the 3d section as a frontend scaler.
#ifdef VOODOO4
	#define VOODOO4_3D_BLITS
#endif

// This enables using tiled memory as the source for 3d blits.
// NOTE:  This doesn't yet function correctly.
#ifdef VOODOO4_3D_BLITS
//	#define VOODOO4_3D_TILED
#endif

// Turn this on to disable the use of the overlay plane.  Useful for testing the other blitters.
#define DISABLE_OVERLAY 1

/* Turn this on to enable the 'mpyc' codec.
	NOTE:  This won't give good results unless we can turn on ASYNC_DECODE below.
*/
#define REGISTER_MPYC 1

/* Turn this on to enable async execution. 
*/
#define ASYNC_DECODE 1


#ifndef _CreateRoutineDescriptor_
#define _CreateRoutineDescriptor_
#if GENERATINGPOWERPC
	#define ExternRoutineDescriptor(info, proc)									\
	 extern RoutineDescriptor g##proc##RD
	
	#define CreateRoutineDescriptor(info, proc)									\
	 RoutineDescriptor g##proc##RD = BUILD_ROUTINE_DESCRIPTOR(info, proc)
	
	#define GetRoutineAddress(proc)	(&g##proc##RD)
#else
	#define ExternRoutineDescriptor(info, proc)
	#define CreateRoutineDescriptor(info, proc)

	#define GetRoutineAddress(proc)	(proc)
#endif
#endif


#pragma	options align=mac68k

#ifndef ASYNC_DECODE
	#define ASYNC_DECODE 1
#endif

/* Version information */

#define	EXAMPLE_CODEC_REV			2
#define	codecInterfaceVersion		2				/* high word returned in component GetVersion */



/*
	Our data structure declarations
*/
#pragma mark ¥ types



typedef void (*notifyProcPtr)(struct mmBlock_s *block, unsigned long code);

struct Globals;
struct BoardGlobals;

/*
	The DecompressRecord is used to store the information needed
	to decompress a frame asynchronously.
*/
#pragma mark
struct DecompressRecord 
{
	Ptr srcData;			// pointer to compressed data
	UInt32 srcDataSize;		// source data size from ImageDescription

	Rect srcRect;
	Rect dstRect;
	Point dstRectOffset;
	UInt32 dstPixelSize;
	
	RgnHandle maskRgn;

	Boolean shieldCursor;	// if we need to shield the cursor
	ICMCompletionProcRecord completionProc;	// completion proc record to call when done
	struct Globals *storage;	// pointer to our globals
	hrmArbiterQElem arbitrator; // used when arbitrating for the hardware
};

enum blitType
{
	blitType2d,
	blitTypeOverlay,
	blitType3d
};

/* This is the structure we use to store our global data for each instance */
#pragma mark
struct Globals
{		
	Globals					*next;
	ComponentInstance		baseCodec;
	BoardGlobals			*boardGlobals;
	h3Info 					*dstBoard;	
	OSType					srcFormat;	
	UInt32					imageDepth;
	UInt32					overlayMode;
	
	bool					needsScratch;
	bool					needsTexture;
	blitType				whichBlitter;
	
	UInt32					originalRowBytes;
	UInt32					scratchRowBytes;
	UInt32					textureRowBytes;

							// Texture "Level of Detail" number for 3d blits.
	UInt32					lod;
	
	/*
		This is cheating.  NewImageBufferMemory is supposed to allocate memory on the card,
		but due to swizzling problems, that doesn't work.  Unfortunately, the only way that
		QuickTime will give us YUV data for several of the most common compressors is through
		'yuvs'/'yuvu', which _only_ works with ImageBufferMemory.  We can still accelerate things
		greatly by allocating ImageBufferMemory in host memory and doing a proper blit with
		swizzling and yuv->rgb conversion.
	*/
	UInt32					*ImageBufferMemory;

	ICMMemoryDisposedUPP 	qtMemoryGoneProc;
	void 					*qtRefcon;	
	
	// Inforation needed for queueing blits
	UInt32 srcWidth;		
	UInt32 srcHeight;
	UInt32 srcPixelShift;	
	UInt32 texPixelShift;	

	Ptr dstBaseAddr;		// base address of destination PixMap
	long dstrowBytes;		// rowBytes parameter of dest PixMap
	UInt32 dstPixelSize;
	UInt32 dstPixelShift;
	Rect dstPixMapBounds;
	
	UInt32 wantedPixelSize;
	
	ImageSequence		sequenceID;
	ComponentResult		lastPreDecompressResult;
	
	ComponentInstance	target;
} ;

/* This is the structure we use to store our global data for each board */
#pragma mark
struct BoardGlobals
{		
	Globals					*instances;
	UInt32					instanceCount;
	h3Info 					*board;	
	mmBlock_t				*scratchMemory;

	mmBlock_t				*overlayMemory;
	bool					overlayRunning;			// True if the overlay is currently turned on
	bool					overlayDirty;			// True if the overlay settings have changed
	UInt32					overlayColor;
	Globals					*currentOverlay;
	UInt32					currentOverlayMetric;
	Globals					*bestOverlay;
	UInt32					bestOverlayMetric;

	mmBlock_t				*mpycScratch;
	mmBlock_t				*textureScratch;
} ;

// This is the structure of an 'mpyc' frame.
#pragma mark
struct mpycFrame
{
	//These are publicly defined fields.
	UInt32 headerSize;
	UInt16 luminanceOffset, cbOffset, crOffset;
	UInt16 luminanceRowBytes, chromaRowBytes;

	//Following, here, is the actual Y, cR and cB data.
};

#pragma mark -
#pragma mark ¥ generated prototypes

#define IMAGECODEC_BASENAME() CD
#define IMAGECODEC_GLOBALS() Globals *storage

#define CALLCOMPONENT_BASENAME IMAGECODEC_BASENAME
#define CALLCOMPONENT_GLOBALS IMAGECODEC_GLOBALS

#include "ImageCodec.k.h"
#include "Components.k.h"


#pragma mark -
#pragma mark ¥ globals

bool gGlobalsInited = false;

Component	gRawCodec = nil;
Component	gyuv2Codec = nil;
Component	g2vuyCodec = nil;
Component	gyuvuCodec = nil;
Component	gmpycCodec = nil;

CodecInfo gCodecInfo =
{
	"3Dfx Hardware Transfer",
	1,
	0x00010000,
	'3Dfx',
	
	// decompressFlags
	//	codecInfoDoes1					|					/* codec can work with 1-bit pixels */
	//	codecInfoDoes2					|					/* codec can work with 2-bit pixels */
	//	codecInfoDoes4					|					/* codec can work with 4-bit pixels */
		codecInfoDoes8					|					/* codec can work with 8-bit pixels */
		codecInfoDoes16					|					/* codec can work with 16-bit pixels */
		codecInfoDoes32					|					/* codec can work with 32-bit pixels */
	//	codecInfoDoesDither				|					/* codec can do ditherMode */
		codecInfoDoesStretch			|					/* codec can stretch to arbitrary sizes */
		codecInfoDoesShrink				|					/* codec can shrink to arbitrary sizes */
		codecInfoDoesMask				|					/* codec can mask to clipping regions */
		codecInfoDoesDouble				|					/* codec can stretch to double size exactly */
		codecInfoDoesQuad				|					/* codec can stretch to quadruple size exactly */
	//	codecInfoDoesHalf				|					/* codec can shrink to half size */
	//	codecInfoDoesQuarter			|					/* codec can shrink to quarter size */
	//	codecInfoDoesRotate				|					/* codec can rotate on decompress */
	//	codecInfoDoesHorizFlip			|					/* codec can flip horizontally on decompress */
	//	codecInfoDoesVertFlip			|					/* codec can flip vertically on decompress */
	//	codecInfoHasEffectParameterList	|					/* codec implements get effects parameter list call, once was codecInfoDoesSkew */
	//	codecInfoDoesBlend				|					/* codec can blend on decompress */
	//	codecInfoDoesWarp				|					/* codec can warp arbitrarily on decompress */
	//	codecInfoDoesRecompress			|					/* codec can recompress image without accumulating errors */
	//	codecInfoDoesSpool				|					/* codec can spool image data */
	0,
	
	// compressFlags
	0,
	
	// formatFlags (may be unnecessary)
		codecInfoDepth1				|					/* compressed data at 1 bpp depth available */
		codecInfoDepth2				|					/* compressed data at 2 bpp depth available */
		codecInfoDepth4				|					/* compressed data at 4 bpp depth available */
		codecInfoDepth8				|					/* compressed data at 8 bpp depth available */
		codecInfoDepth16			|					/* compressed data at 16 bpp depth available */
		codecInfoDepth32			|					/* compressed data at 32 bpp depth available */
		codecInfoDepth24			|					/* compressed data at 24 bpp depth available */
		codecInfoDepth33			|					/* compressed data at 1 bpp monochrome depth  available */
		codecInfoDepth34			|					/* compressed data at 2 bpp grayscale depth available */
		codecInfoDepth36			|					/* compressed data at 4 bpp grayscale depth available */
		codecInfoDepth40			|					/* compressed data at 8 bpp grayscale depth available */
	0,
	
	0,		// compressionAccuracy
	0,		// decompressionAccuracy
	0,		// compressionSpeed
	1,		// decompressionSpeed
			// NOTE: '0' would indicate unknown, but we want the minimum value so QT will always pick our codec.
	0,		// compressionLevel
	0,		// padding
	1,		// minimumHeight
	1,		// minimumWidth
	0,		// decompressPipelineLatency
	0,		// compressPipelineLatency,
	0		// privateData
};


BoardGlobals gBoards[16];

inline BoardGlobals *FindBoard(UInt32 destPtr)
{
	return (&gBoards[destPtr >> 28]);
}

inline BoardGlobals *FindBoard(h3Info *board)
{
	return (&gBoards[((UInt32)board->fifo->fifoStart) >> 28]);
}

#pragma mark -
#pragma mark ¥Êprototypes

/* Function prototypes to keep the compiler smiling. */

pascal void
DecompressStrip(char *data,char *baseAddr,short rowBytes,short w);

ComponentResult
InitGlobals(ComponentInstance self);

ComponentResult DoBlit(DecompressRecord *drp);

bool TryAllocate(Globals *storage, UInt32 wantedSize);
bool TryAllocateMpyc(Globals *storage, UInt32 wantedSize);
bool TryAllocateTexture(Globals *storage, UInt32 wantedSize);
bool TryAllocateFake(Globals *storage, UInt32 wantedSize);
void DisposeFake(Globals *storage);

pascal void DecompressCallBack(QTCallBack cb,long refcon);
void arbiterCallback(UInt32 userData);

void LinkInstance(Globals *storage);
void UnlinkInstance(Globals *storage);

void StartOverlay(DecompressRecord *drp);
void StopOverlay(Globals *storage);

#if ASYNC_DECODE
	
CreateRoutineDescriptor(uppQTCallBackProcInfo, DecompressCallBack);
#endif

// Dispose callbacks
void QuickTimeOverlayNotifyProc(struct mmBlock_s *block, UInt32 code);
void QuickTimeScratchNotifyProc(struct mmBlock_s *block, UInt32 code);
void QuickTimeMpycScratchNotifyProc(struct mmBlock_s *block, UInt32 code);
void QuickTimeTextureScratchNotifyProc(struct mmBlock_s *block, UInt32 code);

#pragma mark ¥ÊUtility functions

/******************************************************************
	ceilLog2(x)
	
	This function returns the smallest n such that (1 << n) >= x.
	ceilLog2(0) will give undefined results.
	
	For example:
	
	ceilLog2(4) == 2
	ceilLog2(255) == 8
	ceilLog2(256) == 8
	ceilLog2(257) == 9
	
******************************************************************/
inline UInt32 ceilLog2(UInt32 x)
{
	UInt32 result = 31 - __cntlzw(x);
	
	if(x > (1 << result))
		result++;
	
	return(result);	
}

inline bool IsOverlayRunning(Globals *storage)
{
	return((storage->dstBoard->regsIO->vidProcCfg & SST_OVERLAY_EN) != 0);
}

#pragma mark -

/************************************************************************************ 
 *	This is the main dispatcher for our codec. All calls from the codec manager
 *	will come through here, with a unique selector and corresponding parameter block.
 *
 *	This routine must be first in the code segment of the codec component.
 */

pascal ComponentResult CDComponentDispatch(ComponentParameters *params, Globals *storage);

struct RoutineDescriptor CDComponentDispatchRD =
  		BUILD_ROUTINE_DESCRIPTOR((kPascalStackBased | RESULT_SIZE (kFourByteCode) |
                            STACK_ROUTINE_PARAMETER (1, kFourByteCode) |
                            STACK_ROUTINE_PARAMETER (2, kFourByteCode)),CDComponentDispatch);

static ProcPtr CDFindRoutineProcPtr(short selector, ProcInfoType *procInfo);

pascal ComponentResult CDComponentDispatch(ComponentParameters *params, Globals *storage)
{
	ProcPtr theProc;
	ProcInfoType theProcInfo;
	ComponentResult result = codecUnimpErr;

	switch(params->what)
	{
//		case kImageCodecDroppingFrameSelect:
//			DebugStr("\pCall to ImageCodecDroppingFrame");
//		break;
//		case kImageCodecScheduleFrameSelect:
//			DebugStr("\pCall to ImageCodecScheduleFrame");
//		break;
//		case kImageCodecCancelTriggerSelect:
//			DebugStr("\pCall to ImageCodecCancelTrigger");
//		break;
		
		default:
 	theProc = CDFindRoutineProcPtr(params->what, &theProcInfo);
	if (theProc) 
			{
		result = CallComponentFunctionWithStorageProcInfo((Handle)storage, params, theProc, theProcInfo);
			}
			else
			{
				result = DelegateComponentCall(params, storage->baseCodec);
			}
		break;
	}
	return result;
}

static ProcPtr CDFindRoutineProcPtr(short selector, ProcInfoType *procInfo)
{
	ProcPtr aProc;
	ProcInfoType pi;

#define ComponentCall(a)	case kComponent##a##Select:  aProc = (ProcPtr)CD##a; pi = uppCallComponent##a##ProcInfo; break;
#define CodecCall(a)		case kImageCodec##a##Select: aProc = (ProcPtr)CD##a; pi = uppImageCodec##a##ProcInfo; break;
#define ComponentError(a)

#define DecompressCall(a)	CodecCall(a)
#define DecompressError(a)
	
	switch (selector) 
	{
	
//		ComponentError	(Unregister)
//		ComponentError	(Register)
		ComponentCall	(Version)
		ComponentCall	(CanDo)
		ComponentCall	(Open)
		ComponentCall	(Close)


		CodecCall		(GetCodecInfo)
//		CodecCall		(Busy)
		
		DecompressCall	(Initialize)
		DecompressCall	(Preflight)
		DecompressCall	(BeginBand)
		DecompressCall	(DrawBand)
		DecompressCall	(EndBand)

		DecompressCall	(QueueStarting)
		DecompressCall	(QueueStopping)
		
		CodecCall	(NewMemory)
		CodecCall	(DisposeMemory)
		CodecCall	(NewImageBufferMemory)
		
//		DecompressCall	(Flush)
		
		default:
			aProc = nil;
			pi = 0;
		}

	*procInfo = pi;
	return aProc;
}

/************************************************************************************ 
 * 	Return true if we can handle the selector, otherwise false.
 */

pascal ComponentResult CDCanDo(Globals *storage, short selector) 
{	
#pragma unused(storage)
	ProcInfoType ignoreResult;
	if(CDFindRoutineProcPtr(selector,&ignoreResult) != 0)
	{
		return(1);
	}
	
	return(CallComponentCanDo(storage->baseCodec, selector) != 0);
}

/************************************************************************************ 
 *	This gets called when the component instance is opened. We allocate our storage at this
 *	point. If we have shared globals, we check if they exist, and put a pointer to them 
 *	in our instance globals so that other calls can get to them.
 */

pascal ComponentResult CDOpen(Globals *storage, ComponentInstance self)
{
	ComponentResult result = noErr;
	
	if(!gGlobalsInited)
	{
		// Since incorporating the QT accel into the QD fragment, this is no longer necessary.
		// Set up our connection to the hrm
//		if ((result = InitializeAccelerationHardware ()) != noErr)
//			return (result);
		
		/* 	Check and initialize our shared globals */
		if ((result = InitGlobals(self)) != noErr)
			return (result);

		gGlobalsInited = true;
	}
	/* 
		First we allocate our local storage. This should store any
		kind of data used by the component instance. It should be allocated
		in the current heap.
	*/	 
		 
	if ( (storage = (Globals *)NewPtrClear(sizeof(Globals))) == nil )  {
		return(MemError());
	}
	
	SetComponentInstanceStorage(self,(Handle)storage);
		
	// Connect to the base image decompressor codec
	if((result = OpenADefaultComponent('imdc', 'base', &(storage->baseCodec))) != noErr)
		return(result);

	if((result = ComponentSetTarget(storage->baseCodec, self)) != noErr)
		return(result);

		storage->target = self;

	// Attempt to forcibly preempt the ATI codec.
	if(gRawCodec != nil)
		SetDefaultComponent(gRawCodec, defaultComponentAnyFlagsAnyManufacturer);
	if(gyuv2Codec != nil)
		SetDefaultComponent(gyuv2Codec, defaultComponentAnyFlagsAnyManufacturer);
	if(g2vuyCodec != nil)
		SetDefaultComponent(g2vuyCodec, defaultComponentAnyFlagsAnyManufacturer);
	if(gyuvuCodec != nil)
		SetDefaultComponent(gyuvuCodec, defaultComponentAnyFlagsAnyManufacturer);
	if(gmpycCodec != nil)
		SetDefaultComponent(gmpycCodec, defaultComponentAnyFlagsAnyManufacturer);
	
	return result;
}


pascal ComponentResult CDInitialize(Globals */*storage*/, ImageSubCodecDecompressCapabilities * cap)
{
	cap->decompressRecordSize = sizeof(DecompressRecord);
	cap->canAsync = true;
	
	return(noErr);
}


/************************************************************************************ 
 *	This gets called when the component instance is opened. We allocate our shared storage at this
 *	point. 
 
 *	If we have shared globals, we check if they exist, otherwise we allocate
 *  them and set the ComponentRefCon so that other instances can use them.
 *
 *	The shared globals hold our CodecInfo struct, which we read from our resource file,
 *  and some tables that we use for speed. If we cant get the tables we can work without
 *  them. All items in the shared globals are made purgeable when the last of our 
 *	instances is closed. If our component was loaded in the application heap ( because
 *	there was no room in the system heap) then we keep our shared storage in the app heap.
 *
 *  We keep a pointer to the shared globals in our instance globals so that other calls can get to them.
 */


ComponentResult
InitGlobals(ComponentInstance self)
{
	OSErr			result = noErr;
		
	return(result);
}

/************************************************************************************ 
 *	This gets called when the component instance is closed. We need to get rid of any 
 *	instance storage here. 
 */

pascal ComponentResult CDClose(Globals *storage,ComponentInstance self)
{
	if (storage) 
	{
		if(storage->dstBoard != nil)
		{
			if(storage->dstBoard->board->lastContextID == (FxU32)storage)
			{
				// Set the current context ID to an invalid ID
				storage->dstBoard->board->lastContextID = -1;
			}
		}

		// Maintain the global links between boards and instances.  This also cleans
		// up allocated memory on the card if this was the last instance.
		UnlinkInstance(storage);

		if ( CountComponentInstances((Component)self) == 1) 
		{
			// Do last-instance cleanup
		}
		
		if(storage->baseCodec != nil)
			CloseComponent(storage->baseCodec);
			
		DisposeFake(storage);

		DisposePtr((Ptr)storage);
	}
	return(noErr);
}



/************************************************************************************ 
 *	Return the version of this component ( defines interface ) and revision level
 *	of the code.
 */

pascal ComponentResult CDVersion(Globals */*storage*/)
{
	return ((codecInterfaceVersion<<16) | EXAMPLE_CODEC_REV);		/* interface version in hi word, code rev in lo word  */
}




inline void ClipRects(Globals *storage, Rect &srcRect, Rect &dstRect)
{
	UInt32	screenWidth = storage->dstPixMapBounds.right - storage->dstPixMapBounds.left;
	UInt32	screenHeight = storage->dstPixMapBounds.bottom - storage->dstPixMapBounds.top;

	if((dstRect.left < 0) || (dstRect.top < 0) || (dstRect.right > screenWidth) || (dstRect.bottom > screenHeight) )
	{
		// The movie is being clipped by an edge of the screen.
		// The blitter doesn't like dealing with negative coordinates, so we fake it.

		// NOTE:  If we wanted to be really accurate here, we could set the bresenham error
		// for the left side of the new dest rect.  We currently don't bother.

		Rect tempDst = dstRect;

		if(tempDst.left < 0)
			tempDst.left = 0;
		if(tempDst.top < 0)
			tempDst.top = 0;
		if(tempDst.right > screenWidth)
			tempDst.right = screenWidth;
		if(tempDst.bottom > screenHeight)
			tempDst.bottom = screenHeight;
		
		Rect tempSrc = tempDst;
		
		MapRect(&tempSrc, &dstRect, &srcRect);
		
		srcRect = tempSrc;
		dstRect = tempDst;
	}
}

/************************************************************************************ 
 *	CDPreDecompress gets called before an image is decompressed. We return information about
 *	how we can decompress the image to the codec manager, so that it can fit the destination data
 *	to our requirements. 
 */

pascal ComponentResult CDPreflight(Globals *storage, CodecDecompressParams *p)
{
	bool overlayPossible = false;
	bool blitterPossible = false;
	bool texturePossible = false;
	bool willUseOverlay = false;
	register CodecCapabilities	*capabilities = p->capabilities;	
	h3Info *h3InfoDst = FindH3Info(p->dstPixMap.baseAddr);
	
	if(p->conditionFlags == 0)
	{
		// Nothing has changed.  Return the same thing as the last call.
		return(storage->lastPreDecompressResult);
	}
	
	// Make sure the destination is one of our boards
	if(!h3InfoDst || h3InfoDst->fifo->exclusiveMode)
		return(storage->lastPreDecompressResult = codecConditionErr);
	
	switch(p->imageDescription[0]->cType)
	{
		case kRawCodecType:
			if(gPreferences->mQT[h3InfoDst->prefsIdx]->disableFlags & kDisableQTCodec_raw)
				return(storage->lastPreDecompressResult = codecConditionErr);
		break;
		case '2vuy':
		case kComponentVideoCodecType:
			if(gPreferences->mQT[h3InfoDst->prefsIdx]->disableFlags & kDisableQTCodec_yuv2)
				return(storage->lastPreDecompressResult = codecConditionErr);
		break;
		case kComponentVideoUnsigned:
			if(gPreferences->mQT[h3InfoDst->prefsIdx]->disableFlags & kDisableQTCodec_yuvs)
				return(storage->lastPreDecompressResult = codecConditionErr);
		break;
		case 'mpyc':
			if(gPreferences->mQT[h3InfoDst->prefsIdx]->disableFlags & kDisableQTCodec_mpyc)
				return(storage->lastPreDecompressResult = codecConditionErr);
		break;
		
		default:
			return(storage->lastPreDecompressResult = codecConditionErr);
		break;
	}
	// stash the destination depth.  We'll need it later.
	h3InfoDst->depth = p->dstPixMap.pixelSize;
	
	storage->dstBoard = h3InfoDst;
	
	// Maintain the global links between boards and instances
	LinkInstance(storage);
	
	if(storage->dstBoard->board->lastContextID == (FxU32)storage)
	{
		// Set the current context ID to an invalid ID
		storage->dstBoard->board->lastContextID = -1;
	}

	storage->srcFormat = p->imageDescription[0]->cType;
	
	if(p->matrixType > scaleTranslateMatrixType)
	{
		// No complex matrices
		return(storage->lastPreDecompressResult = codecConditionErr);
	}	
			
	UInt32 pixelSize = 0;
	UInt32 realDepth = 0;
	UInt32 overlayMetric = 0;
	
	capabilities->extendWidth = 0;
	capabilities->extendHeight = 0;
//	capabilities->time = 0;
	
	p->wantedDestinationPixelTypes = nil;
	
	storage->needsScratch = false;
	storage->needsTexture = false;
	switch(storage->srcFormat)
	{
		case kRawCodecType:
			
			/*	Decide which depth compressed data we can deal with. */
			storage->imageDepth = p->imageDescription[0]->depth;
			switch ( storage->imageDepth )  
			{
#ifdef VOODOO4
				case 16:
				case 32:
					// If we're not doing depth conversion, the blitter will work.
					if(storage->imageDepth == p->dstPixMap.pixelSize)
					{
						blitterPossible = true;
					}

#ifdef VOODOO4_3D_BLITS
					// The 3d blitter can handle some types of depth conversion.
					switch(p->dstPixMap.pixelSize)
					{
						case 16:
							texturePossible = true;
							storage->texPixelShift = 1;
						break;
						
						case 32:
							texturePossible = true;
							storage->texPixelShift = 2;
						break;
					}
#endif

				// The voodoo4 overlay plane can do 1555 and 888 sources.
					overlayPossible = true;
					if(storage->imageDepth == 16)
					storage->overlayMode = SST_OVERLAY_PIXEL_RGB1555U;
					else
					storage->overlayMode = SST_OVERLAY_PIXEL_RGB32U;


					pixelSize = p->dstPixMap.pixelSize / 8;

					if((storage->imageDepth / 8) > pixelSize)
						pixelSize = storage->imageDepth / 8;

					realDepth = storage->imageDepth;
				break;
#else
				case 16:
				case 32:
#endif
				case 8:
					if(storage->imageDepth != p->dstPixMap.pixelSize)
			{
				// For now, no depth conversion.
						return(storage->lastPreDecompressResult = codecConditionErr);
					}

					blitterPossible = true;
					
					pixelSize = p->dstPixMap.pixelSize / 8;
					realDepth = storage->imageDepth;
				break;
				
				default:
					return(storage->lastPreDecompressResult = codecConditionErr);
				break;
			}
		break;
		
		case kComponentVideoCodecType:
		case kComponentVideoUnsigned:
		case '2vuy':
		case 'mpyc':
			pixelSize = 2;
			storage->imageDepth = 32;	// All of these types will end up being expanded to 32-bit before being handed to the hardware.
			realDepth = p->dstPixMap.pixelSize;
			storage->overlayMode = SST_OVERLAY_PIXEL_YUYV422;

			// In true-color modes, we can always deal with yuv data.
			overlayPossible = true;
			
			// Except that the yuv->rgb expander doesn't do 1555 mode, just 565.  Grrr.
			if(p->dstPixMap.pixelSize == 32)
			{
				blitterPossible = true;
				
#ifdef VOODOO4_3D_BLITS
				texturePossible = true;
				
				/*
					For voodoo4, if we are using the 3d section for blits, we need to convert
					yuv to rgb while putting the data on board, since YUV 422 isn't a supported
					texture format.  Fortunately, we can do this during the host blit, but we
					need a larger buffer on the board.
				*/
				storage->texPixelShift = 2;
#endif
			}
#ifdef VOODOO4_3D_BLITS
			else if(p->dstPixMap.pixelSize == 16)
			{
				texturePossible = true;
				storage->texPixelShift = 2;
			}
#endif
			
//			capabilities->extendWidth = p->imageDescription[0]->width & 1;
		break;
		
		default:
			// Um, what?
			return(storage->lastPreDecompressResult = codecConditionErr);
		break;
	}
	
	switch(p->dstPixMap.pixelSize)
	{
		case 8:
			storage->boardGlobals->overlayColor = 0x000000FE;
		break;
		case 16:
			storage->boardGlobals->overlayColor = 0x00000020;
		break;
		case 32:
			storage->boardGlobals->overlayColor = 0x00000100;
		break;
	}

	// The metric we use for determining who gets to use the overlay plane is the size of
	// the destination (height * width), not taking clipping into account.
	overlayMetric = p->dstRect.right - p->dstRect.left;
	overlayMetric *= p->dstRect.bottom - p->dstRect.top;
	
	// If someone else (i.e. DVI scaling) is using the overlay plane, leave it alone.
	if(!storage->boardGlobals->overlayRunning && IsOverlayRunning(storage))
	{
		overlayPossible = false;
	}

	// If use of the overlay plane is disabled, don't use it.
	if(gPreferences->mQT[h3InfoDst->prefsIdx]->disableFlags & kDisableQTOverlay)
		overlayPossible = false;
		
	// Same for the frontend scaler.
	if(gPreferences->mQT[h3InfoDst->prefsIdx]->disableFlags & kDisableQTFrontend)
		blitterPossible = false;
		
#ifdef DISABLE_OVERLAY
	overlayPossible = false;
#endif
			
	storage->srcWidth = p->imageDescription[0]->width;		
	storage->srcHeight = p->imageDescription[0]->height;
	storage->srcPixelShift = pixelSize >> 1;	

	storage->dstBaseAddr = p->dstPixMap.baseAddr;
	storage->dstrowBytes = p->dstPixMap.rowBytes;
	storage->dstPixelSize = p->dstPixMap.pixelSize;
	storage->dstPixelShift = storage->dstPixelSize >> 4;
	storage->dstPixMapBounds = p->dstPixMap.bounds;
	
	// The rowbytes of the incoming data can be a bit tricky to figure out.
	storage->originalRowBytes = p->imageDescription[0]->dataSize / storage->srcHeight;

	// The on-board rowBytes should always be a multiple of 4.
	storage->scratchRowBytes  = ((storage->srcWidth << storage->srcPixelShift) + 3) & ~3;

	// The overlay plane can't shrink.
	if(	((p->dstRect.bottom - p->dstRect.top) < (p->srcRect.bottom - p->srcRect.top)) ||
		((p->dstRect.right - p->dstRect.left) < (p->srcRect.right - p->srcRect.left)))
	{
		overlayPossible = false;
	}
			
	// If we're not stretching, there's no need to use the overlay plane or 3d blits.
	if(	((p->dstRect.bottom - p->dstRect.top) == (p->srcRect.bottom - p->srcRect.top)) &&
		((p->dstRect.right - p->dstRect.left) == (p->srcRect.right - p->srcRect.left)))
	{
		if(blitterPossible)
		{
			texturePossible = false;
			overlayPossible = false;
		}
	}
	
	// If it's possible to use the overlay plane for this stream, see if we should.
	if(overlayPossible && (overlayMetric != 0))
	{
		if((storage->boardGlobals->bestOverlay == nil) || (overlayMetric > storage->boardGlobals->bestOverlayMetric))
		{
			// This stream is the best choice for using the overlay plane.
			storage->boardGlobals->bestOverlay = storage;
			storage->boardGlobals->bestOverlayMetric = overlayMetric;
		}
		
		if(storage->boardGlobals->bestOverlay == storage)
		{
			if(storage->boardGlobals->currentOverlay == nil)
			{
				// The overlay is available.  Use it.
				storage->boardGlobals->currentOverlay = storage;
				storage->boardGlobals->currentOverlayMetric = overlayMetric;
			}
		}
		else
		{
			if(storage->boardGlobals->currentOverlay == storage)
			{
				// This stream is no longer the best choice for the overlay plane.  Release it.
				StopOverlay(storage);
				storage->boardGlobals->currentOverlay = nil;
				storage->boardGlobals->currentOverlayMetric = 0;
			}
		}
	}
	else
	{
		if(storage->boardGlobals->currentOverlay == storage)
		{
			// This stream is no longer the best choice for the overlay plane.  Release it.
			StopOverlay(storage);
			storage->boardGlobals->currentOverlay = nil;
			storage->boardGlobals->currentOverlayMetric = 0;
		}
	}
	
	if(storage->boardGlobals->currentOverlay == storage)
	{
		willUseOverlay = true;
	}
	
	if(!willUseOverlay && !blitterPossible && !texturePossible)
	{
		// The overlay is not available, and the blitter won't work in this case.
		return(storage->lastPreDecompressResult = codecConditionErr);
	}
	
	if(!willUseOverlay)
	{
		if(texturePossible)
		{
			storage->needsTexture = true;
			storage->whichBlitter = blitType3d;
		}
		else
		{
			storage->whichBlitter = blitType2d;
		}
	}
	else
	{
		storage->whichBlitter = blitTypeOverlay;
	}
	
	// Allocate space on the board for the transferred data
	UInt32 wantedSize = storage->scratchRowBytes * storage->srcHeight;
	
	if(storage->srcFormat == 'mpyc')
	{
		storage->needsScratch = true;

		// MBW -- XXX -- I don't remember the reason for this double allocation.
#if 0
		if(!TryAllocateMpyc(storage, p->imageDescription[0]->dataSize))
			return(storage->lastPreDecompressResult = codecConditionErr);
#endif
	}

	if(!TryAllocate(storage, wantedSize))
		return(storage->lastPreDecompressResult = codecConditionErr);

	if(storage->srcFormat == 'mpyc')
	{
		UInt32 chromaSize = (storage->srcWidth / 2) * (storage->srcHeight / 2);
		
		if(!TryAllocateMpyc(storage, chromaSize))
			return(storage->lastPreDecompressResult = codecConditionErr);
	}

#ifdef VOODOO4_3D_BLITS
	if(storage->needsTexture)
	{
		UInt32 textureSize;
	
	#ifdef VOODOO4_3D_TILED
		// Figure out the smallest width and height of tiles that will hold the source.
		{
			UInt32 width = storage->srcWidth << storage->texPixelShift;
			width += 127;
			width /= 128;
			
			UInt32 height = storage->srcHeight;
			height += 31;
			height /= 32;
			
			textureSize = width * height;
			textureSize += 1;			// one extra tile for starting alignment
			textureSize *= 4096;	// number of bytes per tile
			
			// This actually becomes the tile stride.
			storage->textureRowBytes = width;
			
			storage->lod = 0;
		}
	#else // VOODOO4_3D_TILED
		// The on-board rowBytes should always be a power of 2.
		{
			UInt32 temp = MAX(ceilLog2(storage->srcWidth), ceilLog2(storage->srcHeight));
			
			if(temp < 8)
			{
				// To simplify matters later, we always use textures at least
				// 256 pixels wide.
				
				temp = 8;
			}
			
			storage->lod = 11 - temp;
			
			storage->textureRowBytes = 1 << (temp + storage->texPixelShift);
			
			textureSize = storage->textureRowBytes * storage->srcHeight;
		}
	#endif // VOODOO4_3D_TILED

		if(!TryAllocateTexture(storage, textureSize))
			return(storage->lastPreDecompressResult = codecConditionErr);
	}
#endif

	/*	We want the data at its original depth. */
	storage->wantedPixelSize = capabilities->wantedPixelSize = realDepth;	
	
	// We'd like to do the whole source at once, please...
	capabilities->bandMin = p->imageDescription[0]->height;
	capabilities->bandInc = p->imageDescription[0]->height;
	
	capabilities->flags = 
			codecCanScale				|
			codecCanMask				|
		//	codecCanMatte				|
		//	codecCanTransform			|
		//	codecCanTransferMode		|
		//	codecCanCopyPrev			|
		//	codecCanSpool				|
			codecCanClipVertical		|
			codecCanClipRectangular		|
		//	codecCanRemapColor			|
		//	codecCanFastDither			|
			codecCanSrcExtract			|
		//	codecCanCopyPrevComp		|
#if ASYNC_DECODE
			codecCanAsync				|
		//	codecCanMakeMask			|
		//	codecCanShift				|
			codecCanAsyncWhen			|
#endif
		//	codecCanShieldCursor		|	
		//	codecCanManagePrevBuffer	|
			// Volatile buffer isn't really necessary when not using the overlay, but it
			// keeps things a little bit more sane WRT selecting which movie uses the overlay.
		//	codecHasVolatileBuffer		|	/* codec requires redraw after window movement */
		//	codecWantsRegionMask		|
		//	codecImageBufferIsOnScreen	|	/* old def of codec using overlay surface, = ( codecIsDirectToScreenOnly | codecUsesOverlaySurface | codecImageBufferIsOverlaySurface | codecSrcMustBeImageBuffer ) */
		//	codecWantsDestinationPixels	|
		//	codecWantsSpecialScaling	|
		//	codecHandlesInputs			|
		//	codecCanDoIndirectSurface	|	/* codec can handle indirect surface (GDI) */
		//	codecIsSequenceSensitive	|
		//	codecRequiresOffscreen		|
		//	codecRequiresMaskBits		|
		//	codecCanRemapResolution		|
			codecIsDirectToScreenOnly	|	/* codec can only decompress data to the screen */
		//	codecCanLockSurface			|	/* codec can lock destination surface, icm doesn't lock for you */
			0;

	capabilities->flags2 = 
		//	codecUsesOverlaySurface				|	/* codec uses overlay surface */
		//	codecImageBufferIsOverlaySurface	|	/* codec image buffer is overlay surface, the bits in the buffer are on the screen */
		//	codecSrcMustBeImageBuffer			|	/* codec can only source data from an image buffer */
			0;

	storage->sequenceID = p->sequenceID;
	
	if(willUseOverlay)
	{
		// If this stream will be done with the overlay plane, tell Quicktime to flood with our key color.
		p->screenFloodMethod = kScreenFloodMethodKeyColor;
		p->screenFloodValue = storage->boardGlobals->overlayColor;	
		
		capabilities->flags |= codecHasVolatileBuffer;
		capabilities->flags2 |= codecUsesOverlaySurface;

		storage->boardGlobals->overlayDirty = true;
		if(storage->boardGlobals->overlayRunning)
		{
			// If the overlay is running at this point, it belonged to 
			// this stream before and it still does.
			DecompressRecord drp;
			
			// Adjust the overlay plane settings.
			drp.dstRect = p->dstRect;	
			drp.dstRectOffset.h = -p->dstPixMap.bounds.left;
			drp.dstRectOffset.v = -p->dstPixMap.bounds.top;
			drp.storage = storage;
			drp.dstPixelSize = p->dstPixMap.pixelSize;

			StartOverlay(&drp);
		}
	}
	else
	{
		// This stream will not be done with the overlay plane.  
		p->screenFloodMethod = kScreenFloodMethodNone;
		capabilities->flags |= codecWantsRegionMask;
	}
	
	// Tell QuickTime what it wants to hear about individual codecs.
	switch(storage->srcFormat)
	{
		case kComponentVideoUnsigned:
			capabilities->flags |= codecImageBufferIsOnScreen;
			capabilities->flags2 |= codecUsesOverlaySurface | codecImageBufferIsOverlaySurface | codecSrcMustBeImageBuffer;
		break;
		
		case '2vuy':
		case 'mpyc':
//			capabilities->wantedPixelSize = p->dstPixMap.pixelSize;	
		break;
	}

	return(storage->lastPreDecompressResult = noErr);
}



void arbiterCallback(UInt32 userData)
{
	DecompressRecord *drp = (DecompressRecord *)userData;
	Globals *storage = (Globals *)drp->storage;

	if (drp->srcData) 
	{
		if (drp->shieldCursor) 
		{
			ICMShieldSequenceCursor(storage->sequenceID);
			drp->shieldCursor = false;
		}
		
		{
			ComponentResult err;
			
			drp->storage->dstBoard->holdArbitration = true;
			err = DoBlit(drp);
			drp->storage->dstBoard->holdArbitration = false;

			// MBW -- XXX -- The base codec does this for us now.
			//ICMDecompressComplete(storage->sequenceID, err, codecCompletionSource | codecCompletionDest, &drp->completionProc);

			drp->srcData = nil;
		}
	}
}

pascal ComponentResult CDBeginBand(Globals *storage, CodecDecompressParams * p, ImageSubCodecDecompressRecord * sub, long /*flags*/)
{
	OSErr				result = noErr;
	DecompressRecord *drp = (DecompressRecord *)(sub->userDecompressRecord);
	
//	p->capabilities->wantedPixelSize = storage->wantedPixelSize;

		drp->srcData = p->data;
		drp->srcDataSize = p->bufferSize;
		drp->srcRect = p->srcRect;
		drp->dstRect = p->dstRect;	
		drp->dstRectOffset.h = -p->dstPixMap.bounds.left;
		drp->dstRectOffset.v = -p->dstPixMap.bounds.top;
		drp->dstPixelSize = p->dstPixMap.pixelSize;
		drp->maskRgn = p->maskRegion;
		drp->completionProc = p->completionProcRecord;
		drp->shieldCursor = (p->conditionFlags & codecConditionDoCursor) != 0;
		drp->storage = storage;
		drp->arbitrator.delayedTask = arbiterCallback;
		drp->arbitrator.userData = (UInt32)drp;
		
	storage->sequenceID = p->sequenceID;

	return(result);
}


pascal ComponentResult CDDrawBand(Globals *storage, ImageSubCodecDecompressRecord * sub)
{
	OSErr				result = noErr;
	DecompressRecord *drp = (DecompressRecord *)(sub->userDecompressRecord);
			
	hrmArbiterQueue(storage->dstBoard->board, &(drp->arbitrator));
		
	return result;
}

pascal ComponentResult CDEndBand(Globals */*storage*/, ImageSubCodecDecompressRecord * /*drp*/, OSErr  result, long  /*flags*/)
{
	result = noErr;
	return result;
}

pascal ComponentResult CDQueueStarting(Globals */*storage*/)
{
	OSErr result = noErr;
	return result;
}

pascal ComponentResult CDQueueStopping(Globals */*storage*/)
{
	OSErr result = noErr;
	return result;
}

struct blitParsedStruct
{
	Fifo2DRegs *blitter;
	DecompressRecord *drp;
	UInt32 srcBase;
	UInt32 dstBase;
	Rect srcRect;
	Rect dstRect;
};

void DoBlitParsed(blitParsedStruct *s, Rect &rect);

void DoBlitParsed(blitParsedStruct *s, Rect &rect)
{
	Rect localRect = rect;
	OffsetRect(&localRect, s->drp->dstRectOffset.h, s->drp->dstRectOffset.v);
	
	if(localRect.top < 0)
		localRect.top = 0;
	
	if(localRect.left < 0)
		localRect.left = 0;
	
	s->blitter->clip1_cmd_go(
			localRect.left | (localRect.top << 16),
			localRect.right | (localRect.bottom << 16),
			SSTG_STRETCH_BLT | SSTG_GO | SSTG_CLIPSELECT | kROPSource);
}

#ifdef VOODOO4_3D_BLITS

struct blit3DParsedStruct
{
	Fifo3DRegs *blitter;
	DecompressRecord *drp;
	UInt32 srcBase;
	UInt32 dstBase;
	Rect srcRect;
	Rect dstRect;
};

void DoBlit3DParsed(blit3DParsedStruct *s, Rect &rect);

void DoBlit3DParsed(blit3DParsedStruct *s, Rect &rect)
{
	Globals				*storage = s->drp->storage;
	Rect localRect = rect;
	OffsetRect(&localRect, s->drp->dstRectOffset.h, s->drp->dstRectOffset.v);
	
	if(localRect.top < 0)
		localRect.top = 0;
	
	if(localRect.left < 0)
		localRect.left = 0;
	
	Rect iterRect = localRect;
	
	/*	The 3d section of the chip rasterizes triangles in vertical bands.  This is
		non-optimal in terms of memory column select and causes extremely noticable
		tearing artifacts during movie playback.  Breaking large areas into relatively 
		narrow vertical bands ("bandHeight" pixels high) makes the visually annoying 
		tearing artifacts go away.
	*/
	const int bandHeight = 8;
	
	if(iterRect.bottom - iterRect.top > bandHeight)
		iterRect.bottom = iterRect.top + bandHeight;
	
	do
	{
		s->blitter->drawRect(storage->lod, s->srcRect, s->dstRect, iterRect);
		
		iterRect.top = iterRect.bottom;
		iterRect.bottom = iterRect.top + bandHeight;
		
		if(iterRect.bottom > localRect.bottom)
			iterRect.bottom = localRect.bottom;
	} 
	while(iterRect.top < localRect.bottom);
}
#endif

ComponentResult DoBlit(DecompressRecord *drp)
{
	ComponentResult result = noErr;
	Globals				*storage = drp->storage;
	
	mmBlock_t *&memoryPtr = ((storage->whichBlitter == blitTypeOverlay)?
								(storage->boardGlobals->overlayMemory):
								(storage->boardGlobals->scratchMemory));

	bool mpycIsMonochrome = false;
	
	UInt32				dstDepth = storage->dstPixelSize;
	UInt32				srcPixelShift = storage->srcPixelShift;
	UInt32				dstPixelShift = storage->dstPixelShift;
	Rect 				srcRect = drp->srcRect;
	Rect 				dstRect = drp->dstRect;
	
	UInt32				xferPixelFormat = 0;		// src format for the host blit
	UInt32				xferDstPixelFormat = 0;		// dst format for the host blit
	UInt32				xferSwizzle = 0;			// swizzle mode for the host blit
	UInt32				srcPixelFormat = 0;			// src format for the stretch
	UInt32				dstPixelFormat = 0;			// dst format for the stretch

	UInt32				srcSize, dstSize, srcBase, dstBase;
	
	Fifo2DRegs		blitter(storage->dstBoard);
	
	// Check for necessary memory having been purged.
	if(memoryPtr == nil)
	{
		// Our memory has been purged since PreDecompress was called.  This is a critical failure.
		// We have to bail.  This frame will not be drawn.
		return(-1);
	}
	
	if(storage->needsScratch)
	{
		if(storage->boardGlobals->scratchMemory == nil)
			return(-1);
	}

	if(storage->srcFormat == 'mpyc')
	{
		if(storage->boardGlobals->mpycScratch == nil)
			return(-1);
	}
	
	if(storage->needsTexture)
	{
		if(storage->boardGlobals->textureScratch == nil)
			return(-1);
	}
	
	OffsetRect(&dstRect, drp->dstRectOffset.h, drp->dstRectOffset.v);
	

    if(storage->whichBlitter != blitType3d)
    {
        ClipRects(storage, srcRect, dstRect);
    }

	
	dstPixelFormat = blitter.setDepth(dstDepth);
	
	switch(storage->srcFormat)
	{
		case kRawCodecType:
			if(storage->imageDepth > 16)
			{
				xferDstPixelFormat = xferPixelFormat = srcPixelFormat = SSTG_PIXFMT_32BPP;
#ifdef VOODOO4
				xferSwizzle = 0;
#endif
			}
			else if(storage->imageDepth == 16)
			{
				xferDstPixelFormat = xferPixelFormat = srcPixelFormat = SSTG_PIXFMT_16BPP;
#ifdef VOODOO4
				xferSwizzle = SSTG_HOST_WORD_SWIZZLE;
#endif
			}
			else
			{
				xferDstPixelFormat = xferPixelFormat = srcPixelFormat = SSTG_PIXFMT_8BPP;
#ifdef VOODOO4
				xferSwizzle = SSTG_HOST_BYTE_SWIZZLE;
#endif
			}
		break;
		case kComponentVideoUnsigned:
		case kComponentVideoCodecType:
		case 'mpyc':

#ifdef VOODOO4_3D_BLITS
			if(storage->whichBlitter == blitType3d)
			{
				srcPixelFormat = SSTG_PIXFMT_32BPP;
				xferPixelFormat = SSTG_PIXFMT_422YUV;
				xferDstPixelFormat = SSTG_PIXFMT_32BPP;
			}
			else
#endif
			{
				srcPixelFormat = SSTG_PIXFMT_422YUV;
				xferDstPixelFormat = xferPixelFormat = SSTG_PIXFMT_16BPP;
			}
			
#ifdef VOODOO4
			xferSwizzle = SSTG_HOST_BYTE_SWIZZLE;

#else
			if(dstDepth > 16)
			{
				xferSwizzle = SSTG_HOST_BYTE_SWIZZLE;
			}
			else if(dstDepth == 16)
			{
				xferSwizzle = SSTG_HOST_BYTE_SWIZZLE | SSTG_HOST_WORD_SWIZZLE;
			}
			else
			{
				xferSwizzle = 0;
			}
#endif
		break;

		case '2vuy':

#ifdef VOODOO4_3D_BLITS
			if(storage->whichBlitter == blitType3d)
			{
				srcPixelFormat = SSTG_PIXFMT_32BPP;
				xferPixelFormat = SSTG_PIXFMT_422YUV;
				xferDstPixelFormat = SSTG_PIXFMT_32BPP;
			}
			else
#endif
			{
				srcPixelFormat = SSTG_PIXFMT_422YUV;
				xferDstPixelFormat = xferPixelFormat = SSTG_PIXFMT_16BPP;
			}
			
#ifdef VOODOO4
			xferSwizzle = SSTG_HOST_WORD_SWIZZLE;

#else
			if(dstDepth > 16)
			{
				xferSwizzle = 0;
			}
			else if(dstDepth == 16)
			{
				xferSwizzle = SSTG_HOST_WORD_SWIZZLE;
			}
			else
			{
				xferSwizzle = SSTG_HOST_BYTE_SWIZZLE;
			}
#endif
		break;
	}
	
	srcSize = ((srcRect.bottom - srcRect.top) << 16) | (srcRect.right - srcRect.left);
	srcBase = memoryPtr->start & kBaseAddrOffsetMask;
	dstSize = ((dstRect.bottom - dstRect.top) << 16) | (dstRect.right - dstRect.left);
	dstBase = (FxU32) storage->dstBaseAddr & kBaseAddrOffsetMask;

	UInt32 textureBase = nil;	
	if(storage->boardGlobals->textureScratch != nil)
		textureBase = storage->boardGlobals->textureScratch->start & kBaseAddrOffsetMask;

#ifdef VOODOO4_3D_TILED
	// Munge the texture base address so it looks tiled.
	textureBase += 4095;
	textureBase &= ~4095;
	textureBase |= 0x80000000;		// Set the "tiled" bit for the 2d blitter.
#endif

	// Transfer the data to the board.
	{
		FifoBlitData1	blitdata(storage->dstBoard);
		UInt32			*src;
		UInt32			origSrcSize = ((storage->srcHeight) << 16) | (storage->srcWidth);
		switch(storage->srcFormat)
		{
			case 'mpyc':
			{
				/**************************************************************************************
					This code probably deserves an explanation.
					
					The input to the 'mpyc' codec is in planar YUV 420 format.  The YUV input of the 
					blitter (or the overlay plane) needs packed YUV 422.  The following series of blits 
					performs this conversion.
					
					The input can be thought of as three 8-bit surfaces per frame, as follows:
					
					Y0 Y1 Y2 Y3			U0 U1		V0 V1
					Y4 Y5 Y6 Y7			U2 U3		V2 V3
					Y8 Y9 Ya Yb
					Yc Yd Ye Yf
					
					Note that the U and V (chrominance) channels are half the resolution of the
					Y (luminance) channel.
					
					We start by treating the Y channel as 8-bit data and using a host blit 
					with a 2x horizontal stretch to put it on the board.  This gives us:

					Y0 Y0 Y1 Y1 Y2 Y2 Y3 Y3
					Y4 Y4 Y5 Y5 Y6 Y6 Y7 Y7
					Y8 Y8 Y9 Y9 Ya Ya Yb Yb
					Yc Yc Yd Yd Ye Y3 Yf Yf
					
					Next, we move the U channel into scratch space elsewhere on the board 
					without stretching.  We then blit the U channel onto the Y channel (again treating
					everything as 8-bit data) with a 2x vertical stretch and a 4x horizontal stretch. 
					This blit uses the pattern registers and the ROP to create a bytelane mask which 
					only writes the U data into the second byte of each long word.  
					
					This inserts the U channel into the proper places in the Y data:
									
					Y0 U0 Y1 Y1 Y2 U1 Y3 Y3
					Y4 U0 Y5 Y5 Y6 U1 Y7 Y7
					Y8 U2 Y9 Y9 Ya U3 Yb Yb
					Yc U2 Yd Yd Ye U3 Yf Yf
					
					The Y channel gets essentially the same treatment as the U channel, but it
					gets inserted into the fourth byte of each long word.  
					
					This gives us the desired result:
					
					Y0 U0 Y1 V0 Y2 U1 Y3 V1
					Y4 U0 Y5 V0 Y6 U1 Y7 V1
					Y8 U2 Y9 V2 Ya U3 Yb V3
					Yc U2 Yd V2 Ye U3 Yf V3
					
					Some mpyc streams are monochrome, and therefore have no U and V channels at all.
					In this case, the first step is the same, but a transparent pattern fill is used to
					insert a neutral chrominance value in place of the U and V channels:
					
					Y0 00 Y1 00 Y2 00 Y3 00
					Y4 00 Y5 00 Y6 00 Y7 00
					Y8 00 Y9 00 Ya 00 Yb 00
					Yc 00 Yd 00 Ye 00 Yf 00
				**************************************************************************************/
			
				// Since this is a multiple-stage blit, we must use the scratch space instead of going
				// directly to the overlay plane.  (If we don't do this, the intermediate stages are 
				// briefly visible when using the overlay plane.)
				UInt32 scratchBase = storage->boardGlobals->scratchMemory->start & kBaseAddrOffsetMask;

				mpycFrame *header = (mpycFrame*)drp->srcData;
				UInt32 scratchCrCb = storage->boardGlobals->mpycScratch->start & kBaseAddrOffsetMask;
				UInt32 chromaSize = ((storage->srcHeight / 2) << 16) | (storage->srcWidth / 2);
				UInt32 tempSize = ((storage->srcHeight) << 16) | (storage->srcWidth * 2);
				
//				storage->scratchRowBytes = storage->srcWidth << 1;
				
				// Check for a monochrome stream
				if(header->chromaRowBytes == 0)
					mpycIsMonochrome = true;
				
				// Put the luminance data on board, stretched 2x horizontally.
				src = (UInt32*)((drp->srcData) + header->luminanceOffset);

				blitter.reg(reg_dstBaseAddr, scratchBase);
				blitter.reg(reg_dstFormat, (storage->scratchRowBytes) | SSTG_PIXFMT_8BPP);
				blitter.reg(reg_dstSize, tempSize);
				blitter.reg(reg_dstXY, 0);

				blitter.reg(reg_srcFormat, (storage->srcWidth) | SSTG_PIXFMT_8BPP | xferSwizzle);
				blitter.reg(reg_srcXY, 0);
				blitter.reg(reg_srcSize, origSrcSize);
				
				blitter.reg(reg_command, SSTG_HOST_STRETCH_BLT | SSTG_GO | kROPSource);
				blitter.go();

				pushDataToFifo<variantBitBlt::xfer>(
						storage->dstBoard, 
						(variantBitBlt::xfer::srcPtr) src, 
						header->luminanceRowBytes,
						storage->srcWidth >> 2,
						storage->srcHeight,
						0);
				
				if(!mpycIsMonochrome)
				{
					// Put the cr data on board
					src = (UInt32*)((drp->srcData) + header->cbOffset);

					blitter.reg(reg_dstBaseAddr, scratchCrCb);
					blitter.reg(reg_dstFormat, (storage->srcWidth / 2) | SSTG_PIXFMT_8BPP);
					blitter.reg(reg_dstSize, chromaSize);
					blitter.reg(reg_dstXY, 0);

					blitter.reg(reg_srcFormat, (storage->srcWidth / 2) | SSTG_PIXFMT_8BPP | xferSwizzle);
					blitter.reg(reg_srcXY, 0);
					blitter.reg(reg_srcSize, chromaSize);
					
					blitter.reg(reg_command, SSTG_HOST_BLT | SSTG_GO | kROPSource);
					blitter.go();

					pushDataToFifo<variantBitBlt::xfer>(
							storage->dstBoard, 
							(variantBitBlt::xfer::srcPtr) src, 
							header->chromaRowBytes,
							(storage->srcWidth / 2) >> 2,
							storage->srcHeight / 2,
							0);

					// insert the cr data, using a pattern as a bytelane mask
					blitter.reg(reg_dstBaseAddr, scratchBase);
					blitter.reg(reg_dstFormat, (storage->scratchRowBytes) | SSTG_PIXFMT_8BPP);
					blitter.reg(reg_dstSize, tempSize);
					blitter.reg(reg_dstXY, 0);

					blitter.reg(reg_srcBaseAddr, scratchCrCb);
					blitter.reg(reg_srcFormat, (storage->srcWidth / 2) | SSTG_PIXFMT_8BPP);
					blitter.reg(reg_srcSize, chromaSize);
					blitter.reg(reg_srcXY, 0);

					blitter.reg(reg_pattern0alias, 0x44444444);
					blitter.reg(reg_pattern1alias, 0x44444444);
					blitter.reg(reg_colorFore, 0xFFFFFFFF);
					blitter.reg(reg_colorBack, 0x00000000);


					blitter.reg(reg_command, SSTG_STRETCH_BLT | SSTG_GO | 
							SSTG_MONO_PATTERN |
							(0x000000CA << SSTG_ROP0_SHIFT)
					);
					
					blitter.go();

					// Put the cb data on board
					src = (UInt32*)((drp->srcData) + header->crOffset);

					blitter.reg(reg_dstBaseAddr, scratchCrCb);
					blitter.reg(reg_dstFormat, (storage->srcWidth / 2) | SSTG_PIXFMT_8BPP);
					blitter.reg(reg_dstSize, chromaSize);
					blitter.reg(reg_dstXY, 0);

					blitter.reg(reg_srcFormat, (storage->srcWidth / 2) | SSTG_PIXFMT_8BPP | xferSwizzle);
					blitter.reg(reg_srcXY, 0);
					blitter.reg(reg_srcSize, chromaSize);
					
					blitter.reg(reg_command, SSTG_HOST_BLT | SSTG_GO | kROPSource);
					blitter.go();

					pushDataToFifo<variantBitBlt::xfer>(
							storage->dstBoard, 
							(variantBitBlt::xfer::srcPtr) src, 
							header->chromaRowBytes,
							(storage->srcWidth / 2) >> 2,
							storage->srcHeight / 2,
							0);

					// insert the cb data, using a pattern as a bytelane mask
					blitter.reg(reg_dstBaseAddr, scratchBase);
					blitter.reg(reg_dstFormat, (storage->scratchRowBytes) | SSTG_PIXFMT_8BPP);
					blitter.reg(reg_dstSize, tempSize);
					blitter.reg(reg_dstXY, 0);

					blitter.reg(reg_srcBaseAddr, scratchCrCb);
					blitter.reg(reg_srcFormat, (storage->srcWidth / 2) | SSTG_PIXFMT_8BPP);
					blitter.reg(reg_srcSize, chromaSize);
					blitter.reg(reg_srcXY, 0);

					blitter.reg(reg_pattern0alias, 0x11111111);
					blitter.reg(reg_pattern1alias, 0x11111111);
					blitter.reg(reg_colorFore, 0xFFFFFFFF);
					blitter.reg(reg_colorBack, 0x00000000);

					blitter.reg(reg_command, SSTG_STRETCH_BLT | SSTG_GO | 
							SSTG_MONO_PATTERN |
							(0x000000CA << SSTG_ROP0_SHIFT)
					);
					
					blitter.go();

				}
				else
				{
					// Clear the UV bytes
					// Note that neutral chrominance is not 0 but 0x80, since this is unsigned YUV.
					blitter.reg(reg_pattern0alias, 0x55555555);
					blitter.reg(reg_pattern1alias, 0x55555555);
					blitter.reg(reg_colorFore, 0x80808080);

					blitter.reg(reg_command, SSTG_RECTFILL | SSTG_GO | 
							SSTG_MONO_PATTERN | SSTG_TRANSPARENT |
							kROPPattern
					);

					blitter.go();
					
				}
				
				if(storage->whichBlitter == blitTypeOverlay)
				{
					// This stream is using the overlay plane.  Blit from the scratch memory to the overlay.

					blitter.reg(reg_dstBaseAddr, srcBase);
					blitter.reg(reg_dstFormat, (storage->scratchRowBytes) | xferDstPixelFormat);
					blitter.reg(reg_dstSize, origSrcSize);
					blitter.reg(reg_dstXY, 0);

					blitter.reg(reg_srcBaseAddr, scratchBase);
					blitter.reg(reg_srcFormat, (storage->scratchRowBytes) | xferPixelFormat);
					blitter.reg(reg_srcXY, 0);
					blitter.reg(reg_srcSize, origSrcSize);
					
					blitter.reg(reg_command, SSTG_BLT | SSTG_GO | kROPSource);
					blitter.go();
				}
#ifdef VOODOO4_3D_BLITS
				else if(storage->whichBlitter == blitType3d)
				{
					// This data will be used as a source texture.  Color expand to 32 bit.
					blitter.reg(reg_dstBaseAddr, textureBase);
					blitter.reg(reg_dstFormat, (storage->textureRowBytes) | SSTG_PIXFMT_32BPP);
					blitter.reg(reg_dstSize, origSrcSize);
					blitter.reg(reg_dstXY, 0);

					blitter.reg(reg_srcBaseAddr, scratchBase);
					blitter.reg(reg_srcFormat, (storage->scratchRowBytes) | SSTG_PIXFMT_422YUV);
					blitter.reg(reg_srcXY, 0);
					blitter.reg(reg_srcSize, origSrcSize);
					
					blitter.reg(reg_command, SSTG_BLT | SSTG_GO | kROPSource);
					
					blitter.go();
				}
#endif
			}
			break;

			default:
			{
				src = (UInt32*)drp->srcData;

#ifdef VOODOO4_3D_BLITS
				if(storage->whichBlitter == blitType3d)
				{
					blitter.reg(reg_dstBaseAddr, textureBase);
					blitter.reg(reg_dstFormat, (storage->textureRowBytes) | xferDstPixelFormat);
				}
				else
#endif
				{
					blitter.reg(reg_dstBaseAddr, srcBase);
					blitter.reg(reg_dstFormat, (storage->scratchRowBytes) | xferDstPixelFormat);
				}
				blitter.reg(reg_dstSize, origSrcSize);
				blitter.reg(reg_dstXY, 0);

				blitter.reg(reg_srcFormat, (storage->originalRowBytes) | xferPixelFormat | xferSwizzle);
				blitter.reg(reg_srcXY, 0);
				blitter.reg(reg_srcSize, origSrcSize);
				
				blitter.reg(reg_command, SSTG_HOST_BLT | SSTG_GO | kROPSource);
				blitter.go();

				pushDataToFifo<variantBitBlt::xfer>(
						storage->dstBoard, 
						(variantBitBlt::xfer::srcPtr) src, 
						storage->originalRowBytes,
						storage->scratchRowBytes >> 2,
						storage->srcHeight,
						0);
			}
		
			break;

		}
	}

	switch(storage->whichBlitter)
	{
#ifdef VOODOO4_3D_BLITS
		case blitType3d:
		{
			Fifo3DRegs blitter3(storage->dstBoard);
			
			// If the context ID has changed, do the necessary setup.
			if(storage->dstBoard->board->lastContextID != (FxU32)storage)
			{
				storage->dstBoard->board->lastContextID = (FxU32)storage;
				
				// This may or may not be necessary.
				//blitter3.idleFifo();
				
				// Set up the destination surface
				blitter3.setupDest(
						((storage->dstPixelSize == 32)?(SST_RM_32BPP):(SST_RM_15BPP)) |
						SST_RM_YORIGIN_SELECT | SST_RM_RGB_WMASK,
						dstBase, 
						storage->dstrowBytes);
				
				// Set up the pixel pipeline
				blitter3.setupPipeline(
						// reg3_FBI(reg3_combineMode)
						SST_CM_CC_OTHERSELECT_TRGB |		// c_other select "other texture"
						SST_CM_CC_LOCALSELECT_ZERO |		// c_local (output not used)
						SST_CM_CC_MSELECT_7_C1_RGB |		// cc_mselect_7 (output not used)
						SST_CM_CCA_OTHERSELECT_C1_A |		// a_other (output not used)
						SST_CM_CCA_LOCALSELECT_ZERO |		// a_local (output not used)
						SST_CM_USE_COMBINE_MODE,			// use these selections, not fbzMode contents
						
						// reg3_TREX(reg3_combineMode)
						SST_CM_TC_OTHERSELECT_LOCAL_TRGB |	// tc_other select "local texture"
						SST_CM_TC_LOCALSELECT_CK_RGB |		// tc_local (output not used)
						SST_CM_TC_MSELECT_7_ZERO |			// tc_mselect_7 (output not used)
						SST_CM_TCA_OTHERSELECT_CR_A |		// ta_other (output not used)
						SST_CM_TCA_LOCALSELECT_CK_A |		// ta_local (output not used)
						0,
						
						// reg3_fbzColorPath
						SST_CC_MONE |						// cc_mselect select constant 1
						SST_CCA_MONE |						// cca_mselect select constant 1
						SST_ENTEXTUREMAP |
						(2 << SST_TRIANGLE_ITERATOR_COLUMN_BAND_CONTROL_SHIFT),
						
						// reg3_fbzMode
						SST_ENRECTCLIP |
						SST_RGBWRMASK |
						SST_ENDITHER
				);

	#ifdef VOODOO4_3D_TILED
				// Set up the tiled source texture
				blitter3.setupTextureTiled(textureBase & kBaseAddrOffsetMask, 
						storage->textureRowBytes,
						storage->lod,
						((storage->imageDepth == 32)?SST_ARGB8888:SST_ARGB1555) |
						SST_TMINFILTER |
						SST_TMAGFILTER |
						SST_TCLAMPS |
						SST_TCLAMPT |
						SST_TC_MONE |			// tc_mselect select constant 1
						SST_TCA_MONE |			// tca_mselect select constant 1
						0);
	#else
				// Set up the source texture
				blitter3.setupTexture(textureBase, storage->lod,
						((storage->imageDepth == 32)?SST_ARGB8888:SST_ARGB1555) |
						SST_TMINFILTER |
						SST_TMAGFILTER |
						SST_TCLAMPS |
						SST_TCLAMPT |
						SST_TC_MONE |			// tc_mselect select constant 1
						SST_TCA_MONE |			// tca_mselect select constant 1
						0);
	#endif
			}
			
			{
				RgnParserParams params;
				blit3DParsedStruct s;
				
				s.blitter = &blitter3;
				s.drp = drp;
				s.srcBase = textureBase;
				s.dstBase = dstBase;
				s.srcRect = srcRect;
				s.dstRect = dstRect;
				
				params.blitFunction = (RegionBlitProcInternal)DoBlit3DParsed;
				params.blitDataPtr = (void*)&s;
				params.parseTopToBottom = true;
				params.parseLeftToRight = true;
				
				RegionParserDirect(params, drp->maskRgn, nil, nil, drp->dstRect);
			}
		}
		break;
		
#endif
		case blitType2d:
		{			
			// Use the 2d stretch blitter (ugly) for this blit
			blitter.reg(reg_dstBaseAddr, dstBase);
			blitter.reg(reg_dstFormat, storage->dstrowBytes | dstPixelFormat);
			blitter.reg(reg_dstSize, dstSize);
			blitter.reg(reg_dstXY, (dstRect.left & 0x0FFF) | ((dstRect.top & 0x0FFF) << 16));
			
			blitter.reg(reg_srcBaseAddr, srcBase);
			blitter.reg(reg_srcFormat, (storage->scratchRowBytes) | srcPixelFormat);
			blitter.reg(reg_srcXY, (srcRect.left & 0x0FFF) | ((srcRect.top & 0x0FFF) << 16));
			blitter.reg(reg_srcSize, srcSize);		
			blitter.go();
			blitter.alignFifo();
			
			{
				RgnParserParams params;
				blitParsedStruct s;
				
				s.blitter = &blitter;
				s.drp = drp;
				
				params.blitFunction = (RegionBlitProcInternal)DoBlitParsed;
				params.blitDataPtr = (void*)&s;
				params.parseTopToBottom = true;
				params.parseLeftToRight = true;
				
				RegionParserDirect(params, drp->maskRgn, nil, nil, drp->dstRect);
			}
		}
	}
	
	// Allow the blitter to finish
	while(!H3BlitFinish((long)(storage->dstBoard)))
		;

	if(storage->whichBlitter == blitTypeOverlay)
	{
		// Do this _after_ waiting for the blit to finish, so the user doesn't see garbage.
		
		// This stream is using the overlay plane.  Make sure the hardware is set up for it.
		// Note that we could have done this in PreDecompress, but the offscreen memory contained
		// garbage at that point.  We may still need to move this there so that the user doesn't
		// get to see the key color flood the window.
		StartOverlay(drp);
	}
	
	return(result);
}

/************************************************************************************ 
 *	CDGetCodecInfo allows us to return information about ourselves to the codec manager.
 *	
 *	There will be a tool for determining appropriate values for the accuracy, speed
 *	and level information. For now we estimate with scientific wild guessing.
 *
 */

pascal ComponentResult CDGetCodecInfo(Globals */*storage*/,CodecInfo *info)
{
	if ( info == nil ) 
		return(paramErr);

	BlockMoveData((Ptr)&gCodecInfo,(Ptr)info,sizeof(CodecInfo));

	return(noErr);
}


pascal ComponentResult
CDNewMemory(
		Globals *				/*storage*/,
		Ptr *					/*data*/,
		Size 					/*dataSize*/,
		long 					/*dataUse*/,
		ICMMemoryDisposedUPP 	/*memoryGoneProc*/,
		void *					/*refCon*/)
{
//	storage->qtMemoryGoneProc = memoryGoneProc;
//	storage->qtRefcon = refCon;	
	return(codecConditionErr);
}

pascal ComponentResult
CDDisposeMemory(
		Globals *storage,
		Ptr 					/*data*/)
{
	// Don't call the dispose proc in this case
	storage->qtMemoryGoneProc = nil;

	DisposeFake(storage);
	
	return(noErr);
}

pascal ComponentResult
CDNewImageBufferMemory(
		Globals *storage,
		CodecDecompressParams * params,
		long 					/*flags*/,
		ICMMemoryDisposedUPP 	memoryGoneProc,
		void *					refCon)
{

	if(ImageCodecPreDecompress(storage->target, params) == noErr)
	{
		if(TryAllocateFake(storage, storage->scratchRowBytes * params->imageDescription[0]->height))
		{
			storage->qtMemoryGoneProc = memoryGoneProc;
			storage->qtRefcon = refCon;	

			params->dstPixMap.baseAddr = (Ptr)(storage->ImageBufferMemory);
			params->dstPixMap.rowBytes = storage->scratchRowBytes;
			return(noErr);
		}
	}
	
	return(codecConditionErr);
}		

bool TryAllocateLow(Globals *storage, UInt32 wantedSize, mmBlock_t *&memoryPtr, notifyProcPtr notifyProc, char *name);


bool TryAllocateLow(Globals *storage, UInt32 wantedSize, mmBlock_t *&memoryPtr, notifyProcPtr notifyProc, char *name)
{
	UInt32 oldSize = 0;

	if(memoryPtr != nil)
	{
		oldSize = memoryPtr->size;
		
		if(memoryPtr->size < wantedSize)
		{
			// We have memory allocated, but it's too small.  Try again.
			hrmFreeBlock(memoryPtr);
			memoryPtr = nil;
		}
	}

	if(memoryPtr == nil)
	{
		// We don't have any memory allocated.  Try the allocation.
		memoryPtr = hrmAllocateBlock(storage->dstBoard->board, wantedSize, HRM_MEMF_LINEAR, HRM_MEM_PRI_IMPORTANT);
		if(memoryPtr != nil)
		{
			strcpy(memoryPtr->comment, name);
			memoryPtr->userData = (void*)(storage->boardGlobals);
			memoryPtr->notifyProc = notifyProc;
		}
	}

	if(memoryPtr == nil)
	{
		// Whoops.  The allocation failed.  If there was memory allocated before, try to reallocate
		// the same amount again, and tell this instance it doesn't get any.
		if(oldSize != 0)
		{
			memoryPtr = hrmAllocateBlock(storage->dstBoard->board, oldSize, HRM_MEMF_LINEAR, HRM_MEM_PRI_IMPORTANT);
			if(memoryPtr == nil)
			{
				// This is bad.  Some other instance will probably be hosed now.  
				DebugStr("\ppanic: allocation and fallback both failed.");
			}
			else
			{
				strcpy(memoryPtr->comment, name);
				strcat(memoryPtr->comment, " fallback");
				memoryPtr->userData = (void*)(storage->boardGlobals);
				memoryPtr->notifyProc = notifyProc;
			}

		}
		
		return(false);
	}	
	
	return(true);
}

bool TryAllocate(Globals *storage, UInt32 wantedSize)
{
	bool isOverlay = (storage->boardGlobals->currentOverlay == storage);
	mmBlock_t *&memoryPtr = (isOverlay?(storage->boardGlobals->overlayMemory):(storage->boardGlobals->scratchMemory));
	mmBlock_t *&otherPtr = (isOverlay?(storage->boardGlobals->scratchMemory):(storage->boardGlobals->overlayMemory));
	
	// Optimizations that are only possible in the one-instance case.
	if((storage->boardGlobals->instanceCount == 1) && (!storage->needsScratch))
	{
		if((memoryPtr == nil) && (otherPtr != nil))
		{
			// I'm the only instance open, and I'm just switching between overlay and blitter.
			memoryPtr = otherPtr;
			otherPtr = nil;
			strcpy(memoryPtr->comment, (char *) (isOverlay ? "QT Overlay exchange" : "QT Scratch exchange"));
			memoryPtr->userData = (void*)(storage->boardGlobals);
			memoryPtr->notifyProc = isOverlay?QuickTimeOverlayNotifyProc:QuickTimeScratchNotifyProc;
		}
		else if(otherPtr != nil)
		{
			// There's only one instance open, might as well dispose the other memory.
			hrmFreeBlock(otherPtr);
			otherPtr = nil;
		}
	}
	
	// Make sure we have the primary block needed by this codec
	if(!TryAllocateLow(
			storage, 
			wantedSize, 
			memoryPtr, 
			isOverlay?QuickTimeOverlayNotifyProc:QuickTimeScratchNotifyProc, 
			(char *) (isOverlay?"QT Overlay":"QT Scratch")))
	{
		return(false);
	}
	
	// If this stream is using the overlay but needs the scratch too, make sure the scratch is allocated.
	if(isOverlay && storage->needsScratch)
	{
		if(!TryAllocateLow(
				storage, 
				wantedSize, 
				storage->boardGlobals->scratchMemory, 
				QuickTimeScratchNotifyProc, 
				"QT Scratch"))
		{
			return(false);
		}
	}
	
	return(true);
}

bool TryAllocateMpyc(Globals *storage, UInt32 wantedSize)
{
	UInt32 oldSize = 0;
	mmBlock_t *&memoryPtr = storage->boardGlobals->mpycScratch;
	
	if(memoryPtr != nil)
	{
		oldSize = memoryPtr->size;
		
		if(memoryPtr->size < wantedSize)
		{
			// We have memory allocated, but it's too small.  Try again.
			hrmFreeBlock(memoryPtr);
			memoryPtr = nil;
		}
	}
	
	if(memoryPtr == nil)
	{
		// We don't have any memory allocated.  Try the allocation.
		memoryPtr = hrmAllocateBlock(storage->dstBoard->board, wantedSize, HRM_MEMF_LINEAR, HRM_MEM_PRI_IMPORTANT);
		if(memoryPtr != nil)
		{
			strcpy(memoryPtr->comment, "QT mpyc Scratch");
			memoryPtr->userData = (void*)(storage->boardGlobals);
			memoryPtr->notifyProc = QuickTimeMpycScratchNotifyProc;
		}
	}

	if(memoryPtr == nil)
	{
		// Whoops.  The allocation failed.  If there was memory allocated before, try to reallocate
		// the same amount again, and tell this instance it doesn't get any.
		if(oldSize != 0)
		{
			memoryPtr = hrmAllocateBlock(storage->dstBoard->board, oldSize, HRM_MEMF_LINEAR, HRM_MEM_PRI_IMPORTANT);
			if(memoryPtr == nil)
			{
				// This is bad.  Some other instance will probably be hosed now.  
				DebugStr("\ppanic: mpyc allocation and fallback both failed.");
			}
			else
			{
				strcpy(memoryPtr->comment, "QT mpyc fallback");
				memoryPtr->userData = (void*)(storage->boardGlobals);
				memoryPtr->notifyProc = QuickTimeMpycScratchNotifyProc;
			}

		}
		
		return(false);
	}
	
	return(memoryPtr != nil);
}

bool TryAllocateTexture(Globals *storage, UInt32 wantedSize)
{
	UInt32 oldSize = 0;
	mmBlock_t *&memoryPtr = storage->boardGlobals->textureScratch;
	
	if(memoryPtr != nil)
	{
		oldSize = memoryPtr->size;
		
		if(memoryPtr->size < wantedSize)
		{
			// We have memory allocated, but it's too small.  Try again.
			hrmFreeBlock(memoryPtr);
			memoryPtr = nil;
		}
	}
	
	if(memoryPtr == nil)
	{
		// We don't have any memory allocated.  Try the allocation.
		memoryPtr = hrmAllocateBlock(storage->dstBoard->board, wantedSize, HRM_MEMF_LINEAR, HRM_MEM_PRI_IMPORTANT);
		if(memoryPtr != nil)
		{
			strcpy(memoryPtr->comment, "QT texture Scratch");
			memoryPtr->userData = (void*)(storage->boardGlobals);
			memoryPtr->notifyProc = QuickTimeTextureScratchNotifyProc;
		}
	}

	if(memoryPtr == nil)
	{
		// Whoops.  The allocation failed.  If there was memory allocated before, try to reallocate
		// the same amount again, and tell this instance it doesn't get any.
		if(oldSize != 0)
		{
			memoryPtr = hrmAllocateBlock(storage->dstBoard->board, oldSize, HRM_MEMF_LINEAR, HRM_MEM_PRI_IMPORTANT);
			if(memoryPtr == nil)
			{
				// This is bad.  Some other instance will probably be hosed now.  
				DebugStr("\ppanic: texture allocation and fallback both failed.");
			}
			else
			{
				strcpy(memoryPtr->comment, "QT texture fallback");
				memoryPtr->userData = (void*)(storage->boardGlobals);
				memoryPtr->notifyProc = QuickTimeTextureScratchNotifyProc;
			}

		}
		
		return(false);
	}
	
	return(memoryPtr != nil);
}

bool TryAllocateFake(Globals *storage, UInt32 wantedSize)
{
	storage->qtMemoryGoneProc = nil;
	
	DisposeFake(storage);
	
	if(storage->ImageBufferMemory == nil)
	{
		// We don't have any memory allocated.  Try the allocation.
		storage->ImageBufferMemory = (UInt32*)NewPtrSys(wantedSize);
	}
	
	return(storage->ImageBufferMemory != nil);
}

void DisposeFake(Globals *storage)
{
	if(storage->qtMemoryGoneProc != nil)
	{
		CallICMMemoryDisposedProc(
				storage->qtMemoryGoneProc,
				storage->ImageBufferMemory, 
				storage->qtRefcon);

		storage->qtMemoryGoneProc = nil;
	}	

	if(storage->ImageBufferMemory != nil)
	{
		DisposePtr((Ptr)storage->ImageBufferMemory);
		storage->ImageBufferMemory = nil;
	}
	
}


void LinkInstance(Globals *storage)
{
	BoardGlobals *board = FindBoard(storage->dstBoard);
	
	if((storage->boardGlobals != nil) && (storage->boardGlobals != board))
	{
		// Switch this instance to a different board
		if(storage->dstBoard->board->lastContextID == (FxU32)storage)
		{
			// Set the current context ID to an invalid ID
			storage->dstBoard->board->lastContextID = -1;
		}

		UnlinkInstance(storage);
	}
	
	if(storage->boardGlobals == nil)
	{
		if(board->instances == nil)
		{
			// This is the first instance on this board.  
			// ??? is there anything we need to do here?
		}
		
		// Add this instance to the list in the board info
		storage->next = board->instances;
		board->instances = storage;
		board->instanceCount++;
		
		board->board = storage->dstBoard;
		
		// Add a link from this instance back to the board globals
		storage->boardGlobals = board;
	}
	else
	{
		if(storage->boardGlobals != board)
		{
			// Something's horribly wrong.
			DebugStr("\ppanic: storage->boardGlobals != board");
		}
	}
	
}

void UnlinkInstance(Globals *storage)
{
	BoardGlobals *board = storage->boardGlobals;
	
	// It's possible that this instance may not have been added to any board's list yet.
	if(board != nil)
	{
		// If this instance is the one using the overlay, release it.
		if(storage == board->currentOverlay)
		{
			// Stop using the overlay plane first.
			StopOverlay(storage);

			board->currentOverlay = nil;
			board->currentOverlayMetric = 0;
			if(board->overlayMemory != nil)
			{
				hrmFreeBlock(board->overlayMemory);
				board->overlayMemory = nil;
			}
		}
		
		if(storage == board->bestOverlay)
		{
			board->bestOverlay = nil;
			board->bestOverlayMetric = 0;
		}
		
		// Remove this instance from the list.
		if(board->instances == storage)
		{
			// This instance is at the head of the list
			board->instances = storage->next;
		}
		else
		{
			// This instance is elsewhere in the list.
			Globals *temp;
			
			for(temp = board->instances; temp != nil; temp = temp->next)
			{
				if(temp->next == storage)
				{
					temp->next = temp->next->next;
				}
			}
		}
		
		board->instanceCount--;

		// If this is the last instance on this board, dispose the scratch buffers on the card.
		if(board->instances == nil)
		{
			if(board->overlayMemory != nil)
			{
				hrmFreeBlock(board->overlayMemory);
				board->overlayMemory = nil;
			}
			if(board->scratchMemory != nil)
			{
				hrmFreeBlock(board->scratchMemory);
				board->scratchMemory = nil;
			}
			if(board->mpycScratch != nil)
			{
				hrmFreeBlock(board->mpycScratch);
				board->mpycScratch = nil;
			}
			if(board->textureScratch != nil)
			{
				hrmFreeBlock(board->textureScratch);
				board->textureScratch = nil;
			}
		}
		
		// Clean up dangling pointers in this instance, just in case.
		storage->boardGlobals = nil;
		storage->next = nil;
	}
}

void setSecondClutGamma(h3Info *dstBoard);

void setSecondClutGamma(h3Info *board)
{
	int i;
	
	// MBW -- XXX -- FIX THIS to set up the unused half of the clut to the same
	// values it would have if the screen were 32-bit.
	for(i=0; i<256; i++)
	{
		board->regsIO->dacAddr = i;
		board->regsIO->dacData = i;
	}
}


void StartOverlay(DecompressRecord *drp)
{
	if(!drp->storage->boardGlobals->overlayRunning || drp->storage->boardGlobals->overlayDirty)
	{
		bool clutBypass = false;
		UInt32 clutSelect = 0;
		
		// MBW -- XXX -- Should we clear the overlay buffer here?
		
		// XXX -- Should we wait for the fifo to drain so that we don't show garbage data by accident?
		
		// XXX -- Gamma correct overlay plane on 8 and 16 bit screens.
		if(drp->dstPixelSize != 32)
		{
			// Set up the unused CLUT to a proper gamma ramp for a 32-bit screen.
//			setSecondClutGamma(drp->storage->dstBoard);

			// Select the unused CLUT
//			clutSelect = 1;		// XXX -- This isn't right.

			// XXX -- Remove this once the second CLUT is set up right.
			clutBypass = true;
		}
		else
		{
			// XXX -- Someone complained about the gamma correction on the overlay.
			clutBypass = true;
		}
		
		// Start the overlay plane
		hrmSetVideoOverlay(
				drp->storage->dstBoard->board,
				true,
				false,
				drp->storage->boardGlobals->overlayMemory->start,
				drp->storage->boardGlobals->overlayMemory->start,
				drp->storage->scratchRowBytes,
				drp->storage->srcWidth,
				drp->storage->srcHeight,
				drp->dstRect.right - drp->dstRect.left,
				drp->dstRect.bottom - drp->dstRect.top,
				drp->dstRect.top + drp->dstRectOffset.v,
				drp->dstRect.left + drp->dstRectOffset.h,
				SST_OVERLAY_FILTER_BILINEAR,
				false,
				drp->storage->overlayMode,
				clutBypass,	
				clutSelect,
				drp->storage->boardGlobals->overlayColor);
		
		drp->storage->boardGlobals->overlayRunning = true;
		drp->storage->boardGlobals->overlayDirty = false;
	}
}

void StopOverlay(Globals *storage)
{
	if(storage->boardGlobals->overlayRunning)
	{
		hrmSetVideoOverlay(
				storage->dstBoard->board,
				false, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
		
		storage->boardGlobals->overlayRunning = false;
	}
}


void QuickTimeOverlayNotifyProc(struct mmBlock_s *block, UInt32 code)
{
	BoardGlobals *board = (BoardGlobals*) block->userData;
	
	if (code == HRM_MEM_NOTIFY_DISPOSE)
	{
		// Make sure the overlay plane is turned off
		if(board->currentOverlay != nil)
			StopOverlay(board->currentOverlay);
			
		board->overlayMemory = nil;
	}
}

void QuickTimeScratchNotifyProc(struct mmBlock_s *block, UInt32 code)
{
	BoardGlobals *board = (BoardGlobals*) block->userData;
	
	if (code == HRM_MEM_NOTIFY_DISPOSE)
	{
		board->scratchMemory = nil;
	}
}

void QuickTimeMpycScratchNotifyProc(struct mmBlock_s *block, UInt32 code)
{
	BoardGlobals *board = (BoardGlobals*) block->userData;
	
	if (code == HRM_MEM_NOTIFY_DISPOSE)
	{
		board->mpycScratch = nil;
	}
}

void QuickTimeTextureScratchNotifyProc(struct mmBlock_s *block, UInt32 code)
{
	BoardGlobals *board = (BoardGlobals*) block->userData;
	
	if (code == HRM_MEM_NOTIFY_DISPOSE)
	{
		board->textureScratch = nil;
	}
}

#pragma mark -
#pragma mark ¥ Installation/removal


void InstallQuickTimeAccel(void)
{
	ComponentDescription td;
	Handle	dname;
#ifdef VOODOO4
	Str255	rawName = "\pVoodoo4 Raw codec";
	Str255	yuv2Name = "\pVoodoo4 Component Video codec";
	Str255	vuyName = "\pVoodoo4 2vuy codec";
	Str255	yuvuName = "\pVoodoo4 YUV unsigned codec";
	Str255	mpycName = "\pVoodoo4 MPEG helper codec";
	OSType	manufacturer = '3dfx';
#else
	Str255	rawName = "\pVoodoo3 Raw codec";
	Str255	yuv2Name = "\pVoodoo3 Component Video codec";
	Str255	vuyName = "\pVoodoo3 2vuy codec";
	Str255	yuvuName = "\pVoodoo3 YUV unsigned codec";
	Str255	mpycName = "\pVoodoo3 MPEG helper codec";
	OSType	manufacturer = '3Dfx';
#endif
	
	// Make sure we bound to QuickTimeLib
	if((UInt32)RegisterComponent == kUnresolvedCFragSymbolAddress)
		return;
	
	// Register the 'raw ' codec
	dname = NewHandleSys(256);
	td.componentType = 'imdc';
	td.componentSubType = kRawCodecType;
	td.componentManufacturer = manufacturer;
	td.componentFlags = 
	//	codecInfoDoes1					|					/* codec can work with 1-bit pixels */
	//	codecInfoDoes2					|					/* codec can work with 2-bit pixels */
	//	codecInfoDoes4					|					/* codec can work with 4-bit pixels */
		codecInfoDoes8					|					/* codec can work with 8-bit pixels */
		codecInfoDoes16					|					/* codec can work with 16-bit pixels */
		codecInfoDoes32					|					/* codec can work with 32-bit pixels */
	//	codecInfoDoesDither				|					/* codec can do ditherMode */
		codecInfoDoesStretch			|					/* codec can stretch to arbitrary sizes */
		codecInfoDoesShrink				|					/* codec can shrink to arbitrary sizes */
		codecInfoDoesMask				|					/* codec can mask to clipping regions */
	//	codecInfoDoesTemporal			|					/* codec can handle temporal redundancy */
		codecInfoDoesDouble				|					/* codec can stretch to double size exactly */
		codecInfoDoesQuad				|					/* codec can stretch to quadruple size exactly */
	//	codecInfoDoesHalf				|					/* codec can shrink to half size */
	//	codecInfoDoesQuarter			|					/* codec can shrink to quarter size */
	//	codecInfoDoesRotate				|					/* codec can rotate on decompress */
	//	codecInfoDoesHorizFlip			|					/* codec can flip horizontally on decompress */
	//	codecInfoDoesVertFlip			|					/* codec can flip vertically on decompress */
	//	codecInfoHasEffectParameterList	|					/* codec implements get effects parameter list call, once was codecInfoDoesSkew */
	//	codecInfoDoesBlend				|					/* codec can blend on decompress */
	//	codecInfoDoesWarp				|					/* codec can warp arbitrarily on decompress */
	//	codecInfoDoesRecompress			|					/* codec can recompress image without accumulating errors */
	//	codecInfoDoesSpool				|					/* codec can spool image data */
	//	codecInfoDoesRateConstrain		|					/* codec can data rate constrain */
		0;
	
	td.componentFlagsMask = 0;

	BlockMoveData(rawName,*dname,rawName[0] + 1);

	if ((gRawCodec = RegisterComponent(	
						&td,
						&CDComponentDispatchRD, 
						registerComponentGlobal |
						0,
						dname,
						nil, 
						nil)) == 0 ) 
	{
		Debugger();
		ExitToShell();
	}

	SetDefaultComponent(gRawCodec, defaultComponentAnyFlagsAnyManufacturer);

	// MBW -- XXX -- The yuv2 codec is signed YUV, which our hardware doesn't handle.
	//		We'll have to implement a special transfer loop if we want to do this.
#if 0
	// Register the 'yuv2' codec
	dname = NewHandleSys(256);
	td.componentType = 'imdc';
	td.componentSubType = kComponentVideoCodecType;
	td.componentManufacturer = manufacturer;
	td.componentFlags = 
	//	codecInfoDoes1					|					/* codec can work with 1-bit pixels */
	//	codecInfoDoes2					|					/* codec can work with 2-bit pixels */
	//	codecInfoDoes4					|					/* codec can work with 4-bit pixels */
		codecInfoDoes8					|					/* codec can work with 8-bit pixels */
		codecInfoDoes16					|					/* codec can work with 16-bit pixels */
		codecInfoDoes32					|					/* codec can work with 32-bit pixels */
	//	codecInfoDoesDither				|					/* codec can do ditherMode */
		codecInfoDoesStretch			|					/* codec can stretch to arbitrary sizes */
		codecInfoDoesShrink				|					/* codec can shrink to arbitrary sizes */
		codecInfoDoesMask				|					/* codec can mask to clipping regions */
	//	codecInfoDoesTemporal			|					/* codec can handle temporal redundancy */
		codecInfoDoesDouble				|					/* codec can stretch to double size exactly */
		codecInfoDoesQuad				|					/* codec can stretch to quadruple size exactly */
	//	codecInfoDoesHalf				|					/* codec can shrink to half size */
	//	codecInfoDoesQuarter			|					/* codec can shrink to quarter size */
	//	codecInfoDoesRotate				|					/* codec can rotate on decompress */
	//	codecInfoDoesHorizFlip			|					/* codec can flip horizontally on decompress */
	//	codecInfoDoesVertFlip			|					/* codec can flip vertically on decompress */
	//	codecInfoHasEffectParameterList	|					/* codec implements get effects parameter list call, once was codecInfoDoesSkew */
	//	codecInfoDoesBlend				|					/* codec can blend on decompress */
	//	codecInfoDoesWarp				|					/* codec can warp arbitrarily on decompress */
	//	codecInfoDoesRecompress			|					/* codec can recompress image without accumulating errors */
	//	codecInfoDoesSpool				|					/* codec can spool image data */
	//	codecInfoDoesRateConstrain		|					/* codec can data rate constrain */
		0;
	td.componentFlagsMask = 0;

	BlockMoveData(yuv2Name,*dname,yuv2Name[0] + 1);

	if ((gyuv2Codec = RegisterComponent(	
						&td,
						&CDComponentDispatchRD, 
						registerComponentGlobal |
						0,
						dname,
						nil, 
						nil)) == 0 ) 
	{
		Debugger();
		ExitToShell();
	}

	SetDefaultComponent(gyuv2Codec, defaultComponentAnyFlagsAnyManufacturer);
#endif

	// Register the '2vuy' codec
	dname = NewHandleSys(256);
	td.componentType = 'imdc';
	td.componentSubType = '2vuy';
	td.componentManufacturer = manufacturer;
	td.componentFlags = 
	//	codecInfoDoes1					|					/* codec can work with 1-bit pixels */
	//	codecInfoDoes2					|					/* codec can work with 2-bit pixels */
	//	codecInfoDoes4					|					/* codec can work with 4-bit pixels */
		codecInfoDoes8					|					/* codec can work with 8-bit pixels */
		codecInfoDoes16					|					/* codec can work with 16-bit pixels */
		codecInfoDoes32					|					/* codec can work with 32-bit pixels */
	//	codecInfoDoesDither				|					/* codec can do ditherMode */
		codecInfoDoesStretch			|					/* codec can stretch to arbitrary sizes */
		codecInfoDoesShrink				|					/* codec can shrink to arbitrary sizes */
		codecInfoDoesMask				|					/* codec can mask to clipping regions */
	//	codecInfoDoesTemporal			|					/* codec can handle temporal redundancy */
		codecInfoDoesDouble				|					/* codec can stretch to double size exactly */
		codecInfoDoesQuad				|					/* codec can stretch to quadruple size exactly */
	//	codecInfoDoesHalf				|					/* codec can shrink to half size */
	//	codecInfoDoesQuarter			|					/* codec can shrink to quarter size */
	//	codecInfoDoesRotate				|					/* codec can rotate on decompress */
	//	codecInfoDoesHorizFlip			|					/* codec can flip horizontally on decompress */
	//	codecInfoDoesVertFlip			|					/* codec can flip vertically on decompress */
	//	codecInfoHasEffectParameterList	|					/* codec implements get effects parameter list call, once was codecInfoDoesSkew */
	//	codecInfoDoesBlend				|					/* codec can blend on decompress */
	//	codecInfoDoesWarp				|					/* codec can warp arbitrarily on decompress */
	//	codecInfoDoesRecompress			|					/* codec can recompress image without accumulating errors */
	//	codecInfoDoesSpool				|					/* codec can spool image data */
	//	codecInfoDoesRateConstrain		|					/* codec can data rate constrain */
		0;
	td.componentFlagsMask = 0;

	BlockMoveData(vuyName,*dname,vuyName[0] + 1);

	if ((g2vuyCodec = RegisterComponent(	
						&td,
						&CDComponentDispatchRD, 
						registerComponentGlobal |
						0,
						dname,
						nil, 
						nil)) == 0 ) 
	{
		Debugger();
		ExitToShell();
	}

	SetDefaultComponent(g2vuyCodec, defaultComponentAnyFlagsAnyManufacturer);

	// Register the 'yuvu' codec
	dname = NewHandleSys(256);
	td.componentType = 'imdc';
	td.componentSubType = kComponentVideoUnsigned;
	td.componentManufacturer = manufacturer;
	td.componentFlags = 
	//	codecInfoDoes1					|					/* codec can work with 1-bit pixels */
	//	codecInfoDoes2					|					/* codec can work with 2-bit pixels */
	//	codecInfoDoes4					|					/* codec can work with 4-bit pixels */
		codecInfoDoes8					|					/* codec can work with 8-bit pixels */
		codecInfoDoes16					|					/* codec can work with 16-bit pixels */
		codecInfoDoes32					|					/* codec can work with 32-bit pixels */
	//	codecInfoDoesDither				|					/* codec can do ditherMode */
		codecInfoDoesStretch			|					/* codec can stretch to arbitrary sizes */
		codecInfoDoesShrink				|					/* codec can shrink to arbitrary sizes */
		codecInfoDoesMask				|					/* codec can mask to clipping regions */
	//	codecInfoDoesTemporal			|					/* codec can handle temporal redundancy */
		codecInfoDoesDouble				|					/* codec can stretch to double size exactly */
		codecInfoDoesQuad				|					/* codec can stretch to quadruple size exactly */
	//	codecInfoDoesHalf				|					/* codec can shrink to half size */
	//	codecInfoDoesQuarter			|					/* codec can shrink to quarter size */
	//	codecInfoDoesRotate				|					/* codec can rotate on decompress */
	//	codecInfoDoesHorizFlip			|					/* codec can flip horizontally on decompress */
	//	codecInfoDoesVertFlip			|					/* codec can flip vertically on decompress */
	//	codecInfoHasEffectParameterList	|					/* codec implements get effects parameter list call, once was codecInfoDoesSkew */
	//	codecInfoDoesBlend				|					/* codec can blend on decompress */
	//	codecInfoDoesWarp				|					/* codec can warp arbitrarily on decompress */
	//	codecInfoDoesRecompress			|					/* codec can recompress image without accumulating errors */
	//	codecInfoDoesSpool				|					/* codec can spool image data */
	//	codecInfoDoesRateConstrain		|					/* codec can data rate constrain */
		0;
	td.componentFlagsMask = 0;

	BlockMoveData(yuvuName,*dname,yuvuName[0] + 1);

	if ((gyuvuCodec = RegisterComponent(	
						&td,
						&CDComponentDispatchRD, 
						registerComponentGlobal |
						0,
						dname,
						nil, 
						nil)) == 0 ) 
	{
		Debugger();
		ExitToShell();
	}

	SetDefaultComponent(gyuvuCodec, defaultComponentAnyFlagsAnyManufacturer);

#if REGISTER_MPYC
	// Register the 'mpyc' codec
	dname = NewHandleSys(256);
	td.componentType = 'imdc';
	td.componentSubType = 'mpyc';
	td.componentManufacturer = manufacturer;
	td.componentFlags = 
	//	codecInfoDoes1					|					/* codec can work with 1-bit pixels */
	//	codecInfoDoes2					|					/* codec can work with 2-bit pixels */
	//	codecInfoDoes4					|					/* codec can work with 4-bit pixels */
		codecInfoDoes8					|					/* codec can work with 8-bit pixels */
		codecInfoDoes16					|					/* codec can work with 16-bit pixels */
		codecInfoDoes32					|					/* codec can work with 32-bit pixels */
	//	codecInfoDoesDither				|					/* codec can do ditherMode */
		codecInfoDoesStretch			|					/* codec can stretch to arbitrary sizes */
		codecInfoDoesShrink				|					/* codec can shrink to arbitrary sizes */
		codecInfoDoesMask				|					/* codec can mask to clipping regions */
	//	codecInfoDoesTemporal			|					/* codec can handle temporal redundancy */
		codecInfoDoesDouble				|					/* codec can stretch to double size exactly */
		codecInfoDoesQuad				|					/* codec can stretch to quadruple size exactly */
	//	codecInfoDoesHalf				|					/* codec can shrink to half size */
	//	codecInfoDoesQuarter			|					/* codec can shrink to quarter size */
	//	codecInfoDoesRotate				|					/* codec can rotate on decompress */
	//	codecInfoDoesHorizFlip			|					/* codec can flip horizontally on decompress */
	//	codecInfoDoesVertFlip			|					/* codec can flip vertically on decompress */
	//	codecInfoHasEffectParameterList	|					/* codec implements get effects parameter list call, once was codecInfoDoesSkew */
	//	codecInfoDoesBlend				|					/* codec can blend on decompress */
	//	codecInfoDoesWarp				|					/* codec can warp arbitrarily on decompress */
	//	codecInfoDoesRecompress			|					/* codec can recompress image without accumulating errors */
	//	codecInfoDoesSpool				|					/* codec can spool image data */
	//	codecInfoDoesRateConstrain		|					/* codec can data rate constrain */
		0;
	td.componentFlagsMask = 0;

	BlockMoveData(mpycName,*dname,mpycName[0] + 1);

	if ((gmpycCodec = RegisterComponent(	
						&td,
						&CDComponentDispatchRD, 
						registerComponentGlobal |
						0,
						dname,
						nil, 
						nil)) == 0 ) 
	{
		Debugger();
		ExitToShell();
	}

	SetDefaultComponent(gmpycCodec, defaultComponentAnyFlagsAnyManufacturer);
#endif
}

void RemoveQuickTimeAccel(void)
{
	if(gRawCodec != nil)
	{
		UnregisterComponent(gRawCodec);
		gRawCodec = nil;
	}

	if(gyuv2Codec != nil)
	{
		UnregisterComponent(gyuv2Codec);
		gyuv2Codec = nil;
	}

	if(g2vuyCodec != nil)
	{
		UnregisterComponent(g2vuyCodec);
		g2vuyCodec = nil;
	}

	if(gyuvuCodec != nil)
	{
		UnregisterComponent(gyuvuCodec);
		gyuvuCodec = nil;
	}

	if(gmpycCodec != nil)
	{
		UnregisterComponent(gmpycCodec);
		gmpycCodec = nil;
	}

}


#pragma mark -
#pragma mark ¥ scratch space


//#ifdef VOODOO4_3D_BLITS
#if 0
		if(dstDepth == 32)
		{
			// Use the 3d section of the chip for this blit
			Fifo3DRegs blit3(storage->dstBoard);
			
			// Figure out the texture size the board wants to know.
			UInt32 hLog = ceilLog2(srcRect.right);			// actual h size of source
			UInt32 vLog = ceilLog2(srcRect.bottom);			// actual v size of source
			UInt32 tLod = 0;								// contents of tLOD register
			UInt32 texLod = 0;								// lod number for the chip
			UInt32 texAspect = 0;
			
			// Always use large textures and multiple texBaseAddr registers.  This makes several things easier.
			tLod |= 0x41000000;

			if(hLog > vLog)
			{
				// Texture is wider than it is high
				tLod |= 0x00100000;
				texAspect = hLog - vLog;
				texLod = hLog;
			}
			else
			{
				// Texture is square or higher than it is wide.
				texAspect = vLog - hLog;
				texLod = vLog;
			}
			
			// Convert texLod to the form the chip wants
			texLod = 11 - texLod;
			
			// Set min and max LOD to the LOD of this texture to the one we're really using.
			// Take the 4.2 fixed-point format into account.
			tLod |= texLod << 2;
			tLod |= texLod << 8;
			
			// Set all the texture base address registers to the source.
			blit3.reg(reg3_texBaseAddr, (srcBase & 0x01FFFFF0) | ((srcBase >> 24) & 0x00000002));
			blit3.reg(reg3_texBaseAddr_1, srcBase & 0x03FFFFF0);
			blit3.reg(reg3_texBaseAddr_2, srcBase & 0x03FFFFF0);
			blit3.reg(reg3_texBaseAddr_3_8, srcBase & 0x03FFFFF0);			
			
			// Set up the destination
			blit3.reg(reg3_colBufferAddr, dstBase & kBaseAddrOffsetMask);
			blit3.reg(reg3_colBufferStride, storage->dstrowBytes & 0x00003FFF);
			
			// Set up texture mapping, no colorization.
			blit3.reg(reg3_renderMode, 0x000E0006);
			blit3.reg(reg3_combineMode, 0x8000001F);
			blit3.reg(reg3_textureMode, 0x00000FC6);
			blit3.reg(reg3_fogMode, 0);
			blit3.reg(reg3_alphaMode, 0);
			
			// We'll be drawing a triangle strip
			blit3.reg(reg3_sSetupMode, 0x00000020);		
			
			// First vertex (top left)
			blit3.reg(reg3_sS_W0, (float)srcRect.left);
			blit3.reg(reg3_sT_W0, (float)srcRect.top);
			blit3.reg(reg3_sVx, (float)dstRect.left);
			blit3.reg(reg3_sVy, (float)dstRect.top);
			blit3.go(reg3_sBeginTriCMD, 0);

			// Second vertex (top right)
			blit3.reg(reg3_sS_W0, (float)srcRect.right);
			blit3.reg(reg3_sT_W0, (float)srcRect.top);
			blit3.reg(reg3_sVx, (float)dstRect.right);
			blit3.reg(reg3_sVy, (float)dstRect.top);
			blit3.go(reg3_sDrawTriCMD, 0);
			
			// Third vertex (bottom left)
			blit3.reg(reg3_sS_W0, (float)srcRect.left);
			blit3.reg(reg3_sT_W0, (float)srcRect.bottom);
			blit3.reg(reg3_sVx, (float)dstRect.left);
			blit3.reg(reg3_sVy, (float)dstRect.bottom);
			blit3.go(reg3_sDrawTriCMD, 0);

			// Fourth vertex (bottom right)
			blit3.reg(reg3_sS_W0, (float)srcRect.right);
			blit3.reg(reg3_sT_W0, (float)srcRect.bottom);
			blit3.reg(reg3_sVx, (float)dstRect.right);
			blit3.reg(reg3_sVy, (float)dstRect.bottom);
			blit3.go(reg3_sDrawTriCMD, 0);
		}
		else
#endif
