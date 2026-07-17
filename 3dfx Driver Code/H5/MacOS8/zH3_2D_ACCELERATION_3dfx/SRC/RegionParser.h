/**********************************************************************************
	RegionParser.h
	
	Header information for Region Parser. This file presents the interface for using
	the Region parser, which is used by Native Quickdraw Acceleration modules to
	break regions (passed to acceleration procs in NQDDrawVars) into a series of 
	rectangles the acceleration procs can blit onscreen.
	
	In usual use, the acceleration module's Accept proc (either RgnBlit or PatRgnBlit)
	sets RegionParser as the Go proc for the blit. The Accept proc then sets the 
	gBlitProc global to point to their rectangle blitter that knows how to handle 
	the appropriate type of blits.
	
	Chall Fry
	Critical Path Software
	12/10/95
	
*/
// Declarations
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

//#include "Utilities.h"

// Constants
#define kMaxNumInversions 514

// Includes

// Types

	// The Accept proc needs to set up this function pointer to point to a function that
	// will draw the given rectangle onscreen. Functionally, is blit proc can be thought
	// of as a Go proc, except is uses the given rect instead of dstRect. The srcRect
	// can be calculated by determining the offset between the srcRect and the dstRect
	// in drawVars. Note however, that if your hardware uses byte offsets, it's more 
	// efficient to compute the byte offset between the overall srcStart and dstStart once,
	// and then subtract that difference from the computed destination start addr.
typedef void (*RegionBlitProc)(NQDDrawVars* drawVars, Rect * rect);
typedef void (*RegionBlitProcInternal)(void* drawVars, Rect * rect);

typedef struct RgnParserParams
{
	RegionBlitProcInternal	blitFunction;
	void					*blitDataPtr;
	short					parseTopToBottom;
	short					parseLeftToRight;
} RgnParserParams;


// Function Declarations

	// This is the function that the Accept proc should declare as the blitProc in
	// DrawVars if it wants to take this blit.
//void	RegionParser(NQDDrawVars &drawVars);
void	RegionParser(NQDDrawVars *drawVars);


#ifdef __cplusplus
}
#endif