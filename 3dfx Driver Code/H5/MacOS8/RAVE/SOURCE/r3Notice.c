//: r3Notice.c
//: alt.drivers inc.
//: Glenn Nissen


#include "r3Core.h"
extern TRvInfo gRvEngInfo;

/*
enum TQAMethodSelector {
	kQAMethod_RenderCompletion	= 0,			// Called when rendering has completed and buffers swapped 
	kQAMethod_DisplayModeChanged = 1,			// Called when a display mode has changed 
	kQAMethod_ReloadTextures	= 2,			// Called when texture memory has been invalidated 
	kQAMethod_ImageBufferInitialize = 3,		// Called when a buffer needs to be initialized 
	kQAMethod_ImageBuffer2DComposite = 4,		// Called when rendering is finished and its safe to composite 
	kQAMethod_NumSelectors		= 5
};
typedef enum TQAMethodSelector TQAMethodSelector;
*/

///////////////////////////
//                       //
//  RvSetNoticeMethod  //
//                       //
///////////////////////////

TQAError 
RvSetNoticeMethod(const TQADrawContext* drawContext,
                    TQAMethodSelector     method,
                    TQANoticeMethod       completionCallBack,
                    void*                 refCon)
{
	TQADrawPrivate* dp = drawContext->drawPrivate;
	TQAError err = kQANoErr;

	DebugStr("--> RvSetNoticeMethod - ");
	
	if( method < 0 || method >= kQAMethod_NumSelectors )
		return( kQAError );
	
	switch(method) {
	case kQAMethod_RenderCompletion:
		DebugStr("RenderCompletion");
		break;
	case kQAMethod_DisplayModeChanged:
		DebugStr("DisplayModeChanged");
		break;
	case kQAMethod_ReloadTextures:
		DebugStr("ReloadTextures");
		break;
	case kQAMethod_ImageBufferInitialize:
		DebugStr("ImageBufferInitialize");
		break;
	case kQAMethod_ImageBuffer2DComposite:
		DebugStr("ImageBuffer2DComposite");
		break;
	default:
		DebugStr("*** unknown method");
		err = kQANotSupported;
		goto bail;
	}

	dp->completionCallBack[method] = completionCallBack;
	dp->callBackRefCon[method] = refCon;
	
bail:
	DebugStr("\n");
	return err;
}


///////////////////////////
//                       //
//  RvGetNoticeMethod  //
//                       //
///////////////////////////

TQAError 
RvGetNoticeMethod(const TQADrawContext* drawContext,
                    TQAMethodSelector     method,
                    TQANoticeMethod*      completionCallBack,
                    void**                refCon)
{
	TQADrawPrivate* dp = drawContext->drawPrivate;
	TQAError err = kQANoErr;

	DebugStr("--> RvGetNoticeMethod - ");
	
	switch(method) {
	case kQAMethod_RenderCompletion:
		DebugStr("RenderCompletion");
		break;
	case kQAMethod_DisplayModeChanged:
		DebugStr("DisplayModeChanged");
		break;
	case kQAMethod_ReloadTextures:
		DebugStr("ReloadTextures");
		break;
	case kQAMethod_ImageBufferInitialize:
		DebugStr("ImageBufferInitialize");
		break;
	case kQAMethod_ImageBuffer2DComposite:
		DebugStr("ImageBuffer2DComposite");
		break;
	default:
		DebugStr("*** unknown method");
		err = kQANotSupported;
		goto bail;
	}

	*completionCallBack = dp->completionCallBack[method];
	*refCon             = dp->callBackRefCon[method];
	
bail:
	DebugStr("\n");
	return err;
}


#pragma mark -
static void DeregisterCallbacks( void )
{

}
