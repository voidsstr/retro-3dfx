//: tnslBitmap.c
//: alt.drivers inc.
//: Glenn Nissen
//: Vadim Kochubievski
//: Rohit Gundecha


#include "r3Core.h"
extern TRvInfo	gRvEngInfo;

/////////////////////
//                 //
//  tnslBitmapNew  //
//                 //
/////////////////////

TQAError 
tnslBitmapNew(unsigned long     flags,
              TQAImagePixelType pixelType,
              const TQAImage*   image,
              TQABitmap**       newBitmap)
{
	TQAError err = kQANoErr;
	
	DebugStr("--> tnslBitmapNew - ");
	DebugNum(image->width);
	DebugNum(image->height);
	
	*newBitmap = (TQABitmap*) AllocPtr(sizeof (TQABitmap));
	if(*newBitmap == NULL) {
		DebugStr("*** could not alloc memory for structure");
		err = kQAError;
		goto bail;
	}
	memset(*newBitmap, 0, sizeof (TQABitmap));

	(**newBitmap).width     = image->width;
	(**newBitmap).height    = image->height;
	(**newBitmap).stride    = image->rowBytes;
	(**newBitmap).bitmapPtr = (void*) image->pixmap;
	(**newBitmap).pixelType = pixelType;
	(**newBitmap).copied    = 0;
	
bail:
	DebugStr("\n");
	return err;
#pragma unused(flags)
}


////////////////////////
//                    //
//  tnslBitmapDetach  //
//                    //
////////////////////////

TQAError 
tnslBitmapDetach(TQABitmap* bitmap)
{
	TQAError err = kQANoErr;
	
	void* bitmapPtr;
	
	DebugStr("--> tnslBitmapDetach - ");
	
	if(bitmap->copied) goto bail;

	bitmapPtr = (void*) AllocPtr(bitmap->stride * bitmap->height);
	if(bitmapPtr == NULL) {
		DebugStr("*** could not alloc memory for bitmap");
		err = kQAError;
		goto bail;
	}
	
	memcpy(bitmapPtr, bitmap->bitmapPtr, bitmap->stride * bitmap->height);
	
	bitmap->bitmapPtr = bitmapPtr;
	bitmap->copied    = 1;

bail:
	DebugStr("\n");
	return err;
}


////////////////////////
//                    //
//  tnslBitmapDelete  //
//                    //
////////////////////////

void 
tnslBitmapDelete(TQABitmap* bitmap)
{
	DebugStr("--> tnslBitmapDelete - ");
	
	if(bitmap->copied) FreePtr((void*) bitmap->bitmapPtr);
	FreePtr((void*) bitmap);
	DebugStr("\n");
}


