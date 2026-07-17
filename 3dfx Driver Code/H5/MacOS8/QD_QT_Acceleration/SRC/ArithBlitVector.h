/********************************************************************************
	ArithBlitVector.h
		
	Function Declarations for arithmetic blit functions.
*/

#pragma once

// Function Declarations

RegionBlitProc PickArithBlitVectorVariantLL(UInt32 srcDepth, UInt32 dstDepth, NQDDrawVars *drawVars);
RegionBlitProc PickArithBlitVariantLL(NQDDrawVars *drawVars, UInt32 srcDepth, UInt32 dstDepth);
