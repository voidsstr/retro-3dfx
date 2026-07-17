/**********************************************************************************
	RegionParser.c
	
	Routines for decomposing regions into series of blittable rectangles.
	
	Chall Fry
	Critical Path Software
	10/8/95
*/

// Includes
#include "NQDAcceleration.h"
#include "RegionParser.h"

#include "minihwc.h"
#include "H3Acceleration.h"

// Constants

// Macros
#define Min(a, b)	((a < b) ? a : b)
#define Max(a, b)	((a > b) ? a : b)
#define Min3(a, b, c)	(Min(a, Min(b, c)))
#define Max3(a, b, c)	(Max(a, Max(b, c)))

// Types
class NScanLine
{
public:
	// Constructor
	NScanLine(NQDRegion ** const rgn, const Rect &theMinRect, bool topToBottom) : minRect(theMinRect),
			numPoints(0), points(pts1), rgnDone(false)
			{ InitializeScanLine(rgn, topToBottom); };

	Boolean		RgnParseDone() { return rgnDone; };


		// This is the current state of the region being processed, intersected with the minRect.
	Rect		currentRect;		// Intersected w MinRect
	
		// This is the minRect from the drawVars; used to clip all the rects we generate
		// in currentRect
	const Rect	minRect;

		// These values are not intersected with the minRect, and indicate our current
		// position within the region
	SInt16		top;					// The y-value for the top of the scanline's rect
	SInt16		bottom;					// The y-value for the bottom of the scanline's rect
	SInt16		left;					// The x-value for the bottom of the scanline's rect
	SInt16		right;					// The x-value for the bottom of the scanline's rect

		// This points to either pts1 or pts2; it flip-flops
	SInt16		*points;					// Current Array of inversion points
	SInt32		numPoints;					// The number of inversion points in the points array
	SInt32		ptIndex;						// Used to step through points array
	
		// Why do this function pointer madness instead of virtual functions? Because,
		// I need to have NScanLines be local variables.
	Boolean 	(NScanLine::*NextIntersectingScanLineFn)(const Rect &intersectingRect);
	inline Boolean NextIntersectingScanLine(const Rect &intersectingRect)
					 { return (this->*NextIntersectingScanLineFn)(intersectingRect); };

protected:
	void		InitializeScanLine(const NQDRegion * const * const region, bool topToBottom);
	Boolean		IntersectingScanLineForward(const Rect &intersectingRect);
	Boolean		IntersectingScanLineBackward(const Rect &intersectingRect);

private:
	const NQDRegion	**rgn;					// The region being parsed
	short		*rgnDataPtr;				// Used to step through the region data structure

	UInt32		rgnDone;					// TRUE if done processing region

	SInt16		pts1[kMaxNumInversions];	// Array of inversion points
	SInt16		pts2[kMaxNumInversions];	// Array of inversion points
};
typedef NScanLine *NScanLinePtr;


// Globals

// Macros

// Function Declarations
inline void 	RPFastSetRect(Rect &rect, SInt16 top, SInt16 left, SInt16 bottom, SInt16 right);
inline Boolean 	FastSectRect(const Rect &source, const Rect &source2, Rect &dest);

void		ParseOneRgnProc(const RgnParserParams &parserParams, NQDRegion ** const rgn,
					const Rect &minRect);
void		ParseOneRgnProcR2L(const RgnParserParams &parserParams, NQDRegion ** const rgn,
					const Rect &minRect);
void		ParseTwoRgnProc(const RgnParserParams &parserParams, NQDRegion ** const rgn1, 
					NQDRegion ** const rgn2, const Rect &minRect);
void		ParseTwoRgnProcR2L(const RgnParserParams &parserParams, NQDRegion ** const rgn1, 
					NQDRegion ** const rgn2, const Rect &minRect);
void 		ParseThreeRgnProc(const RgnParserParams &parserParams, NQDRegion ** const rgn1, 
					NQDRegion ** const rgn2, NQDRegion ** const rgn3, const Rect &minRect);
void 		ParseThreeRgnProcR2L(const RgnParserParams &parserParams, NQDRegion ** const rgn1, 
					NQDRegion ** const rgn2, NQDRegion ** const rgn3, const Rect &minRect);
void 		ParseThreeRgnProcR2L(const RgnParserParams &parserParams, NQDRegion ** const rgn1, 
					NQDRegion ** const rgn2, NQDRegion ** const rgn3, const Rect &minRect);

// Function Definitions

#pragma mark ¥ Utilities
/**********************************************************************************
	RPFastSetRect
	
	Sets the coordinates of a rectangle to the given values.
*/
inline void RPFastSetRect(Rect &rect, SInt16 top, SInt16 left, SInt16 bottom, SInt16 right)
{
	rect.top = top;
	rect.left = left;
	rect.bottom = bottom;
	rect.right = right;
}

/**********************************************************************************
	RPFastSectRect
		Rect *source
		Rect *dest
		
	Calculates the intersection of two rectangles. Sets dest to the intersection.
	Returns TRUE if the intersection is non-null. Otherwise, dest is undefined on exit.
*/
inline Boolean RPFastSectRect(const Rect &source, const Rect &source2, Rect &dest)
{
	SInt32 destTop = source.top;
	SInt32 destLeft = source.left;
	SInt32 destBottom = source.bottom;
	SInt32 destRight = source.right;
	
	if (source2.top > destTop)
		destTop = source2.top;
	if (source2.bottom < destBottom)
		destBottom = source2.bottom;
	if (source2.left > destLeft)
		destLeft = source2.left;
	if (source2.right < destRight)
		destRight = source2.right;
		
	// Why cast to UInt16? Because, it makes CW not add extraneous extsh instructions
	dest.top = (UInt16) destTop;
	dest.left = (UInt16) destLeft;
	dest.bottom = (UInt16) destBottom;
	dest.right = (UInt16) destRight;
	
	return (destTop < destBottom) && (destLeft < destRight);
}

#pragma mark -
#pragma mark ¥ NScanLine

/**********************************************************************************
	InitializeScanLine
	
	Prepares a ScanLine structure for use by NextScanLine. 
*/
void NScanLine::InitializeScanLine(const NQDRegion * const * const region, bool topToBottom)
{
	// Set up the region pointer stuff
	rgn = (const NQDRegion **) region;
	
	if (topToBottom)
	{
		rgnDataPtr = (short *) &rgn[0][0].rgnData;

		// Set up the rectangle stuff
		currentRect.top = currentRect.bottom = top = bottom = *rgnDataPtr;
		
		NextIntersectingScanLineFn = &IntersectingScanLineForward;
	} else
	{
		rgnDataPtr = ((short *) *rgn) + (rgn[0][0].rgnSize - 8 >> 1);

		// Iterate backwards through the region until we find a region terminator
		// End on the line number indicator
		while (*rgnDataPtr != kRgnTerminator)
			rgnDataPtr -= 2;
		++rgnDataPtr;
	
		// Set up the rectangle stuff
		currentRect.top = currentRect.bottom = top = bottom = *rgnDataPtr;

		NextIntersectingScanLineFn = &IntersectingScanLineBackward;
	}
}

/**********************************************************************************
	IntersectingScanLineForward(rgnState)
		ScanLine	rgnState			The region to modify

	Parses the next line of the region, and modifies the array of inversion points to
	reflect the changes.
	
	On entry, the rgnDataPtr points to the line number marker on the scanline to be
	processed. On exit, rgnDataPtr points to the next line number marker.
	
	Returns TRUE if the scanline now intersects the intersectingRect vertically, OR
	if the region is finished or has passed below the minRect. The idea here is to
	return FALSE in the case where  the controlling loop should continue moving regions
	forward in an attempt to have the regions intersect vertically, and TRUE when the loop
	should exit and do something else, either look at horizontal intersections, or bail
	because the region is done.
*/
Boolean	NScanLine::IntersectingScanLineForward(const Rect &intersectingRect)
{
	SInt16		*newPoints;
	UInt32		numNewPts;
	SInt16		oldScanLinePoint;
	UInt32		oldPtIndex;
	SInt16		nextPoint;
	SInt16 		*startDataPtr = (SInt16 *) rgnDataPtr;	

	// For each line, until we reach the top of the intersecting rect *and* the minRect
	do {
		// If we've gone below the bottom of the minRect, the region's done
		if ((top = bottom) >= minRect.bottom)
		{
			// Return true so that we'll pop out of the loop
			rgnDone = true;
			return true;
		}
		
		// Prepare for a new line
		newPoints = (points == pts1) ? pts2 : pts1;		// Flip-flop between pts1 and pts2
		oldPtIndex = numNewPts = 0;
		nextPoint = *(++rgnDataPtr);
		oldScanLinePoint = points[0];
		
		// Merge the points, up to the next line end marker
		while ((oldPtIndex < numPoints) && (nextPoint != kRgnTerminator) && 
				numNewPts < kMaxNumInversions)
		{
			// If the points are equal, we remove the inversion point from the array
			if (nextPoint == oldScanLinePoint)
			{
				++oldPtIndex;
				nextPoint = *(++rgnDataPtr);			
				oldScanLinePoint = points[oldPtIndex];
			} else if (nextPoint < oldScanLinePoint)
			{
				// Merge: Add nextPoint from the rgn to the new array
				newPoints[numNewPts++] = nextPoint;
				nextPoint = *(++rgnDataPtr);
			} else
			{
				// Merge: Copy the old array point into the new array
				newPoints[numNewPts++] = oldScanLinePoint;
				++oldPtIndex;
				oldScanLinePoint = points[oldPtIndex];
			}
		}
		
		// Finish out whichever set of points isn't fully merged yet
		while (oldPtIndex < numPoints && numNewPts < kMaxNumInversions)
			newPoints[numNewPts++] = points[oldPtIndex++];
		while (nextPoint != kRgnTerminator && numNewPts < kMaxNumInversions)
		{
			newPoints[numNewPts++] = nextPoint;
			nextPoint = *(++rgnDataPtr);
		}
				
		// Finish the swap of the two inversion point arrays
		numPoints = numNewPts;
		points = newPoints;

		// Set the bottom of the current rectangle; bail if we're at the end of the rgn
		if ((bottom = *(++rgnDataPtr)) == kRgnTerminator || numNewPts >= kMaxNumInversions)
		{
			rgnDone = true;
			return true; 
		}

	} while (bottom <= intersectingRect.top || bottom <= minRect.top);
	
	// Write new values out to the class's data
	currentRect.top = Max(top, minRect.top);
	currentRect.bottom = Min(bottom, minRect.bottom);

	// Check to see if we've gone past the intersecting rect
	if (top >= intersectingRect.bottom)
		return false;
	else
		return true;
}

/**********************************************************************************
	IntersectingScanLineBackward(rgnState)
		ScanLine	rgnState			The region to modify

	Parses the previous line of the region, and modifies the array of inversion points to
	reflect the changes.
	
	On entry, the rgnDataPtr points to the line number marker on the scanline to be
	processed. On exit, rgnDataPtr points to the next line number marker.
	
	Returns TRUE if the scanline now intersects the intersectingRect vertically, OR
	if the region is finished or has passed below the minRect. The idea here is to
	return FALSE in the case where  the controlling loop should continue moving regions
	forward in an attempt to have the regions intersect vertically, and TRUE when the loop
	should exit and do something else, either look at horizontal intersections, or bail
	because the region is done.
*/
Boolean	NScanLine::IntersectingScanLineBackward(const Rect &intersectingRect)
{
	SInt16		*newPoints;
	UInt32		numNewPts;
	SInt16		oldScanLinePoint;
	UInt32		oldPtIndex;
	SInt16		nextPoint;
	SInt16 		*startDataPtr;

	// For each line, until we reach the top of the intersecting rect *and* the minRect
	do {

		// If we've gone above the top of the minRect, the region's done
		if ((bottom = top) <= minRect.top)
		{
			// Return true so that we'll pop out of the loop
			rgnDone = true;
			return true;
		}
		
		// Prepare for a new line
		startDataPtr = (SInt16 *) rgnDataPtr;	
		newPoints = (points == pts1) ? pts2 : pts1;		// Flip-flop between pts1 and pts2
		oldPtIndex = numNewPts = 0;
		nextPoint = *(++rgnDataPtr);
		oldScanLinePoint = points[0];
		
		// Merge the points, up to the next line end marker
		while ((oldPtIndex < numPoints) && (nextPoint != kRgnTerminator) && 
				numNewPts < kMaxNumInversions)
		{
			// If the points are equal, we remove the inversion point from the array
			if (nextPoint == oldScanLinePoint)
			{
				++oldPtIndex;
				nextPoint = *(++rgnDataPtr);			
				oldScanLinePoint = points[oldPtIndex];
			} else if (nextPoint < oldScanLinePoint)
			{
				// Merge: Add nextPoint from the rgn to the new array
				newPoints[numNewPts++] = nextPoint;
				nextPoint = *(++rgnDataPtr);
			} else
			{
				// Merge: Copy the old array point into the new array
				newPoints[numNewPts++] = oldScanLinePoint;
				++oldPtIndex;
				oldScanLinePoint = points[oldPtIndex];
			}
		}
		
		// Finish out whichever set of points isn't fully merged yet
		while (oldPtIndex < numPoints && numNewPts < kMaxNumInversions)
			newPoints[numNewPts++] = points[oldPtIndex++];
		while (nextPoint != kRgnTerminator && numNewPts < kMaxNumInversions)
		{
			newPoints[numNewPts++] = nextPoint;
			nextPoint = *(++rgnDataPtr);
		}
		
		// Finish the swap of the two inversion point arrays
		numPoints = numNewPts;
		points = newPoints;
		
		// Bail if that was the top line of the region
		if(startDataPtr == &rgn[0][0].rgnData[0] || numNewPts >= kMaxNumInversions)
		{
			rgnDone = true;
			return true;
		}

		// Search backward through the region for the previous line marker
		rgnDataPtr = startDataPtr - 3;
		while (*rgnDataPtr != kRgnTerminator && rgnDataPtr > &rgn[0][0].rgnData[0])
			rgnDataPtr -= 2;
		++rgnDataPtr;

		// Set the top of the current rectangle
		top = *rgnDataPtr;

	} while (top >= intersectingRect.bottom || top >= minRect.bottom);
	
	// Write new values out to the class's data
	currentRect.top = Max(top, minRect.top);
	currentRect.bottom = Min(bottom, minRect.bottom);

	// Check to see if we've gone past the intersecting rect
	if (bottom <= intersectingRect.top)
		return false;
	else
		return true;
}

#pragma mark -
#pragma mark ¥ Multi-region intersection procs
/**********************************************************************************
	ParseOneRgnProc
	
	Decomposes a region into rectangles, and calls the blit proc to draw the pattern
	in those rects.
*/
void ParseOneRgnProc(const RgnParserParams &parserParams, NQDRegion ** const rgn, const Rect &minRect)
{
	NScanLine		scanLine(rgn, minRect, parserParams.parseTopToBottom);
	SInt32			scIndex;
	Rect			blitRect;
		
	while (scanLine.NextIntersectingScanLine(minRect) && !scanLine.RgnParseDone())
	{
		for (scIndex = 0; scIndex < scanLine.numPoints; scIndex += 2)
		{
			scanLine.currentRect.left = scanLine.points[scIndex];
			scanLine.currentRect.right = scanLine.points[scIndex + 1];
			
			if (RPFastSectRect(scanLine.currentRect, minRect, blitRect))
				parserParams.blitFunction(parserParams.blitDataPtr, &blitRect);
		}
	}
} 

/**********************************************************************************
	ParseOneRgnProcR2L
	
	Decomposes a region into rectangles, and calls the blit proc to draw the pattern
	in those rects.
*/
void ParseOneRgnProcR2L(const RgnParserParams &parserParams, NQDRegion ** const rgn, const Rect &minRect)
{
	NScanLine		scanLine(rgn, minRect, parserParams.parseTopToBottom);
	SInt32			scIndex;
	Rect			blitRect;
		
	while (scanLine.NextIntersectingScanLine(minRect) && !scanLine.RgnParseDone())
	{
		for (scIndex = scanLine.numPoints - 2; scIndex >= 0; scIndex -= 2)
		{
			scanLine.currentRect.left = scanLine.points[scIndex];
			scanLine.currentRect.right = scanLine.points[scIndex + 1];
			
			if (RPFastSectRect(scanLine.currentRect, minRect, blitRect))
				parserParams.blitFunction(parserParams.blitDataPtr, &blitRect);
		}
	}
} 

/**********************************************************************************
	ParseTwoRgnProc
	
	Intersects two regions, and draws a pattern in the intersected area. This function
	decomposes the regions into rectangles, and calls the blit proc with those rects. 
*/
void ParseTwoRgnProc(const RgnParserParams &parserParams, NQDRegion ** const rgn1, 
			NQDRegion ** const rgn2, const Rect &minRect)
{
	NScanLine		scanLine1(rgn1, minRect, parserParams.parseTopToBottom);
	NScanLine		scanLine2(rgn2, minRect, parserParams.parseTopToBottom);
	Rect			blitRect;
	SInt32			sc1Index, sc2Index;
	
	// Get the first rectangle of one of the scanlines, exit if it doesn't happen
		// to intersect the minRect
	if (!scanLine1.NextIntersectingScanLine(minRect))
		return;

	// Move one or the other rgn forward until they intersect vertically
	while (!scanLine2.NextIntersectingScanLine(scanLine1.currentRect) &&
			!scanLine1.NextIntersectingScanLine(scanLine2.currentRect)) ;

	// Loop until one of the regions is finished
	while (!scanLine1.RgnParseDone() && !scanLine2.RgnParseDone())
	{
		// The regions now intersect vertically. Work on the horizontal stuff.
		blitRect.top = Max(scanLine1.currentRect.top, scanLine2.currentRect.top);
		blitRect.bottom = Min(scanLine1.currentRect.bottom, scanLine2.currentRect.bottom);
		
		sc1Index = 0;
		sc2Index = 0;
		if (scanLine1.numPoints && scanLine2.numPoints)
		{
			while (1)
			{
				if (scanLine1.points[sc1Index + 1] <= scanLine2.points[sc2Index])
				{
					sc1Index += 2;
					if (sc1Index >= scanLine1.numPoints)
						break;
				} else if (scanLine2.points[sc2Index + 1] <= scanLine1.points[sc1Index])
				{
					sc2Index += 2;
					if (sc2Index >= scanLine2.numPoints)
						break;
				} else
				{
					// The points now intersect horizontally. Blit the intersection
					blitRect.left = Max3(scanLine1.points[sc1Index], scanLine2.points[sc2Index],
							minRect.left);
					blitRect.right = Min3(scanLine1.points[sc1Index + 1], scanLine2.points[sc2Index + 1],
							minRect.right);
					if (blitRect.left < blitRect.right)
						parserParams.blitFunction(parserParams.blitDataPtr, &blitRect);
					
					// Find one of the scanlines to move forward
					if (scanLine1.points[sc1Index + 1] < scanLine2.points[sc2Index + 1])
					{
						sc1Index += 2;
						if (sc1Index >= scanLine1.numPoints)
							break;
					} else
					{
						sc2Index += 2;
						if (sc2Index >= scanLine2.numPoints)
							break;
					}
				}
			}
		}
		
		// Move one or the other rgn forward until they intersect vertically
			// (or they're done)
		if (parserParams.parseTopToBottom)
		{
			if (scanLine1.bottom < scanLine2.bottom)
				while (!scanLine1.NextIntersectingScanLine(scanLine2.currentRect) &&
						!scanLine2.NextIntersectingScanLine(scanLine1.currentRect)) ;
			else
				while (!scanLine2.NextIntersectingScanLine(scanLine1.currentRect) &&
						!scanLine1.NextIntersectingScanLine(scanLine2.currentRect)) ;
		} else
		{
			if (scanLine1.top > scanLine2.top)
				while (!scanLine1.NextIntersectingScanLine(scanLine2.currentRect) &&
						!scanLine2.NextIntersectingScanLine(scanLine1.currentRect)) ;
			else
				while (!scanLine2.NextIntersectingScanLine(scanLine1.currentRect) &&
						!scanLine1.NextIntersectingScanLine(scanLine2.currentRect)) ;
		}
	}
}

/**********************************************************************************
	ParseTwoRgnProcR2L
	
	Intersects two regions, and draws a pattern in the intersected area. This function
	decomposes the regions into rectangles, and calls the blit proc with those rects. 
*/
void ParseTwoRgnProcR2L(const RgnParserParams &parserParams, NQDRegion ** const rgn1, 
			NQDRegion ** const rgn2, const Rect &minRect)
{
	NScanLine		scanLine1(rgn1, minRect, parserParams.parseTopToBottom);
	NScanLine		scanLine2(rgn2, minRect, parserParams.parseTopToBottom);
	Rect			blitRect;
	SInt32			sc1Index, sc2Index;
	
	// Loop until one of the regions is finished
	while (1)
	{
		// Move one or the other rgn forward until they intersect vertically
			// (or they're done)
		if (parserParams.parseTopToBottom)
		{
			if (scanLine1.bottom < scanLine2.bottom)
				while (!scanLine1.NextIntersectingScanLine(scanLine2.currentRect) &&
						!scanLine2.NextIntersectingScanLine(scanLine1.currentRect)) ;
			else
				while (!scanLine2.NextIntersectingScanLine(scanLine1.currentRect) &&
						!scanLine1.NextIntersectingScanLine(scanLine2.currentRect)) ;
		} else
		{
			if (scanLine1.top > scanLine2.top)
				while (!scanLine1.NextIntersectingScanLine(scanLine2.currentRect) &&
						!scanLine2.NextIntersectingScanLine(scanLine1.currentRect)) ;
			else
				while (!scanLine2.NextIntersectingScanLine(scanLine1.currentRect) &&
						!scanLine1.NextIntersectingScanLine(scanLine2.currentRect)) ;
		}
		
		// Break out of the loop if either region is finished
		if (scanLine1.RgnParseDone() || scanLine2.RgnParseDone())
			break;

		if (scanLine1.numPoints && scanLine2.numPoints)
		{
			// The regions now intersect vertically. Work on the horizontal stuff.
			blitRect.top = Max(scanLine1.currentRect.top, scanLine2.currentRect.top);
			blitRect.bottom = Min(scanLine1.currentRect.bottom, scanLine2.currentRect.bottom);

			sc1Index = scanLine1.numPoints - 2;
			sc2Index = scanLine2.numPoints - 2;

			while (1)
			{
				if (scanLine1.points[sc1Index + 1] <= scanLine2.points[sc2Index])
				{
					// If scanline1's rect is to the left of scanline2's... (No intersection)
					sc2Index -= 2;
					if (sc2Index < 0)
						break;
				} else if (scanLine2.points[sc2Index + 1] <= scanLine1.points[sc1Index])
				{
					// If scanline2's rect is to the left of scanline1's... (No intersection)
					sc1Index -= 2;
					if (sc1Index < 0)
						break;
				} else
				{
					// The points now intersect horizontally. Blit the intersection
					blitRect.left = Max3(scanLine1.points[sc1Index], scanLine2.points[sc2Index],
							minRect.left);
					blitRect.right = Min3(scanLine1.points[sc1Index + 1], scanLine2.points[sc2Index + 1],
							minRect.right);
					if (blitRect.left < blitRect.right)
						parserParams.blitFunction(parserParams.blitDataPtr, &blitRect);
					
					// Find one of the scanlines to move forward
					if (scanLine1.points[sc1Index] > scanLine2.points[sc2Index])
					{
						sc1Index -= 2;
						if (sc1Index < 0)
							break;
					} else
					{
						sc2Index -= 2;
						if (sc2Index < 0)
							break;
					}
				}
			}
		}
	}
}

/**********************************************************************************
	ParseThreeRgnProc
	
	Intersects three regions, and draws the intersected area. This function
	decomposes the regions into rectangles, and calls the blit proc with those rects. 
*/
void ParseThreeRgnProc(const RgnParserParams &parserParams, NQDRegion ** const rgn1, 
			NQDRegion ** const rgn2, NQDRegion ** const rgn3, const Rect &minRect)
{
	NScanLine		scanLine1(rgn1, minRect, parserParams.parseTopToBottom);
	NScanLine		scanLine2(rgn2, minRect, parserParams.parseTopToBottom);
	NScanLine		scanLine3(rgn3, minRect, parserParams.parseTopToBottom);
	NScanLinePtr	scanLinePtr;
	Rect			intersectionRect;
	Rect			blitRect;
	bool			parseDone = false;
	
	while (1)
	{
		// Move one or the other rgn forward until they intersect vertically (or they're done)
		while (1)
		{
			scanLinePtr = &scanLine1;
			if (parserParams.parseTopToBottom)
			{
				// Find the region that's farthest behind (whose bottom edge has the smallest value)
				if (scanLine2.bottom < scanLine1.bottom)
					scanLinePtr = &scanLine2;
				if (scanLine3.bottom < scanLinePtr->bottom)
					scanLinePtr = &scanLine3;
			} else
			{
				// Find the region that's farthest behind (whose top edge has the largest value)
				if (scanLine2.top > scanLine1.top)
					scanLinePtr = &scanLine2;
				if (scanLine3.top > scanLinePtr->top)
					scanLinePtr = &scanLine3;
			}		
	
			// Determine the intersection of the two other regions' currentRects. Note that
			// they needn't intersect at all.
			if (scanLinePtr == &scanLine1)
				RPFastSectRect(scanLine2.currentRect, scanLine3.currentRect, intersectionRect);
			else if (scanLinePtr == &scanLine2)
				RPFastSectRect(scanLine1.currentRect, scanLine3.currentRect, intersectionRect);
			else
				RPFastSectRect(scanLine1.currentRect, scanLine2.currentRect, intersectionRect);

			// Step the chosen scanline forward, and see if it intersects the intersectionRect.
			// Also, see if the intersectionRect is itself valid.
			if (scanLinePtr->NextIntersectingScanLine(intersectionRect) && 
					intersectionRect.top < intersectionRect.bottom)
			{
				// Break out of the loop if any region is finished
				parseDone = scanLinePtr->RgnParseDone();

				// All 3 regions should now intersect vertically.
				break;
			}
		}
			
		// Break out of the loop if any region is finished
		if (parseDone)
			break;
			
		// If all 3 scan lines have some inversion points, begin horizontal checks.
		if (scanLine1.numPoints && scanLine2.numPoints && scanLine3.numPoints)
		{
			// Determine the top and bottom of our blitRect for the current scanline state
			blitRect.top = Max3(scanLine1.currentRect.top, scanLine2.currentRect.top, 
					scanLine3.currentRect.top);		
			blitRect.bottom = Min3(scanLine1.currentRect.bottom, scanLine2.currentRect.bottom, 
					scanLine3.currentRect.bottom);
					
			scanLine1.ptIndex = scanLine2.ptIndex = scanLine3.ptIndex = 0;

			while (1)
			{
				// Check to see if all three rects intersect
				blitRect.left = Max3(scanLine1.points[scanLine1.ptIndex],
						scanLine2.points[scanLine2.ptIndex], scanLine3.points[scanLine3.ptIndex]);
				blitRect.right = Min3(scanLine1.points[scanLine1.ptIndex + 1], 
						scanLine2.points[scanLine2.ptIndex + 1], scanLine3.points[scanLine3.ptIndex + 1]);
						
				// If the intersection intersects the minRect, blit the intersection
				if (RPFastSectRect(blitRect, minRect, blitRect))
					parserParams.blitFunction(parserParams.blitDataPtr, &blitRect);
					
				// Choose a scanLine to move forward horizontally
				// Base it on the right edge that's furthest to the left
				scanLinePtr = &scanLine1;
				if (scanLine2.points[scanLine2.ptIndex + 1] < scanLine1.points[scanLine1.ptIndex + 1])
					scanLinePtr = &scanLine2;
				if (scanLine3.points[scanLine3.ptIndex + 1] < scanLinePtr->points[scanLinePtr->ptIndex + 1])
					scanLinePtr = &scanLine3;
					
				scanLinePtr->ptIndex += 2;
				if (scanLinePtr->ptIndex >= scanLinePtr->numPoints)
					break;
			}
		}
	}
}

/**********************************************************************************
	ParseThreeRgnProcR2L
	
	Intersects three regions, and draws the intersected area. This function
	decomposes the regions into rectangles, and calls the blit proc with those rects. 
*/
void ParseThreeRgnProcR2L(const RgnParserParams &parserParams, NQDRegion ** const rgn1, 
			NQDRegion ** const rgn2, NQDRegion ** const rgn3, const Rect &minRect)
{
	NScanLine		scanLine1(rgn1, minRect, parserParams.parseTopToBottom);
	NScanLine		scanLine2(rgn2, minRect, parserParams.parseTopToBottom);
	NScanLine		scanLine3(rgn3, minRect, parserParams.parseTopToBottom);
	NScanLinePtr	scanLinePtr;
	Rect			intersectionRect;
	Rect			blitRect;
	bool			parseDone = false;
	
	while (1)
	{
		// Move one or the other rgn forward until they intersect vertically (or they're done)
		while (1)
		{
			scanLinePtr = &scanLine1;
			if (parserParams.parseTopToBottom)
			{
				// Find the region that's farthest behind (whose bottom edge has the smallest value)
				if (scanLine2.bottom < scanLine1.bottom)
					scanLinePtr = &scanLine2;
				if (scanLine3.bottom < scanLinePtr->bottom)
					scanLinePtr = &scanLine3;
			} else
			{
				// Find the region that's farthest behind (whose top edge has the largest value)
				if (scanLine2.top > scanLine1.top)
					scanLinePtr = &scanLine2;
				if (scanLine3.top > scanLinePtr->top)
					scanLinePtr = &scanLine3;
			}		
	
			// Move that region forward until it intersects the intersection of the two other
			// regions' currentRects
			if (scanLinePtr == &scanLine1)
				RPFastSectRect(scanLine2.currentRect, scanLine3.currentRect, intersectionRect);
			else if (scanLinePtr == &scanLine2)
				RPFastSectRect(scanLine1.currentRect, scanLine3.currentRect, intersectionRect);
			else
				RPFastSectRect(scanLine1.currentRect, scanLine2.currentRect, intersectionRect);

			if (scanLinePtr->NextIntersectingScanLine(intersectionRect) && 
					intersectionRect.top < intersectionRect.bottom)
			{
				// Break out of the loop if any region is finished
				parseDone = scanLinePtr->RgnParseDone();

				// All 3 regions should now intersect vertically.
				break;
			}
		}
			
		// Break out of the loop if any region is finished
		if (parseDone)
			break;
			
		// If all 3 scan lines have some inversion points, begin horizontal checks.
		if (scanLine1.numPoints && scanLine2.numPoints && scanLine3.numPoints)
		{
			// Determine the top and bottom of our blitRect for the current scanline state
			blitRect.top = Max3(scanLine1.currentRect.top, scanLine2.currentRect.top, 
					scanLine3.currentRect.top);		
			blitRect.bottom = Min3(scanLine1.currentRect.bottom, scanLine2.currentRect.bottom, 
					scanLine3.currentRect.bottom);
					
			scanLine1.ptIndex = scanLine1.numPoints - 2;
			scanLine2.ptIndex = scanLine2.numPoints - 2;
			scanLine3.ptIndex = scanLine3.numPoints - 2;

			while (1)
			{
				// Check to see if all three rects intersect
				blitRect.left = Max3(scanLine1.points[scanLine1.ptIndex],
						scanLine2.points[scanLine2.ptIndex], scanLine3.points[scanLine3.ptIndex]);
				blitRect.right = Min3(scanLine1.points[scanLine1.ptIndex + 1], 
						scanLine2.points[scanLine2.ptIndex + 1], scanLine3.points[scanLine3.ptIndex + 1]);
						
				// If the intersection intersects the minRect, blit the intersection
				if (RPFastSectRect(blitRect, minRect, blitRect))
					parserParams.blitFunction(parserParams.blitDataPtr, &blitRect);
					
				// Choose a scanLine to move forward horizontally
				// Base it on the left edge that's furthest to the right
				scanLinePtr = &scanLine1;
				if (scanLine2.points[scanLine2.ptIndex] > scanLine1.points[scanLine1.ptIndex])
					scanLinePtr = &scanLine2;
				if (scanLine3.points[scanLine3.ptIndex] > scanLinePtr->points[scanLinePtr->ptIndex])
					scanLinePtr = &scanLine3;
					
				scanLinePtr->ptIndex -= 2;
				if (scanLinePtr->ptIndex < 0)
					break;
			}
		}
	}
}

#pragma mark -
#pragma mark ¥ Entry Points

// rcf Temporary
#include "h3defs.h"
#include "h3gdefs.h"
#include "minihwc.h"
#include "hwcio.h"
#include "GraphicsPrivHwc.h"
//#include "hdwr_res_mgr.h"
//#include "hrm_fifo.h"

//#include "Utilities.h"

/**********************************************************************************
	RegionParser
		const NQDDrawVars &drawVars
	
	Entry point for the region-parsing code. This entry point is designed to be 
	used as a NQD BlitProc, and placed in the blitProc field of the NQD structure.
	
	This is the only function in this file that should access the drawVars structure elements
	directly.
*/
void RegionParser(NQDDrawVars *dv)
{
	UInt32			numRgns = 0;
	NQDRegion		**rgns[3];
	RgnParserParams	parserParams;
	const NQDDrawVars		&drawVars = *dv;
	
	parserParams.blitFunction = (RegionBlitProcInternal) BlitMoveRect;

	// rcf Temp hack until we get the one-board-per-instance stuff in
	dv->refCon = (long) FindH3Info(drawVars.dstPixMap.baseAddr);

	// Are there regions to be parsed?
	if (drawVars.trimResult > 0)
	{
		// Fill in the rest of the parserParams
		parserParams.blitDataPtr = (void *) &drawVars;
		parserParams.parseTopToBottom = (drawVars.dstPixMap.rowBytes > 0);
		
		// Most of the time we can parse left-to-right.
		parserParams.parseLeftToRight = true;
		
		// The only case where we need to parse right-to-left is a scroll which moves pixels from left to right
		if(drawVars.srcPixMap.baseAddr == drawVars.dstPixMap.baseAddr)
		{
			if(drawVars.srcRect.left < drawVars.dstRect.left)
				parserParams.parseLeftToRight = false;
		}
	
		// Determine how many regions are complex
		if (drawVars.rgnA && drawVars.rgnA[0][0].rgnSize > 10)
			rgns[numRgns++] = (NQDRegion **) drawVars.rgnA;
		if (drawVars.rgnB && drawVars.rgnB[0][0].rgnSize > 10)
			rgns[numRgns++] = (NQDRegion **) drawVars.rgnB;
		if (drawVars.rgnC && drawVars.rgnC[0][0].rgnSize > 10)
			rgns[numRgns++] = (NQDRegion **) drawVars.rgnC;
		
		parserParams.blitDataPtr = (void *) &drawVars;
		
		// Call the approriate region function
		if (parserParams.parseLeftToRight)
		{
			if (numRgns == 1)
				ParseOneRgnProc(parserParams, rgns[0], drawVars.minRect);
			else if (numRgns == 2)
				ParseTwoRgnProc(parserParams, rgns[0], rgns[1], drawVars.minRect);
			else
				ParseThreeRgnProc(parserParams, rgns[0], rgns[1], rgns[2], drawVars.minRect);
		} else
		{
			if (numRgns == 1)
				ParseOneRgnProcR2L(parserParams, rgns[0], drawVars.minRect);
			else if (numRgns == 2)
				ParseTwoRgnProcR2L(parserParams, rgns[0], rgns[1], drawVars.minRect);
			else
				ParseThreeRgnProcR2L(parserParams, rgns[0], rgns[1], rgns[2], drawVars.minRect);
		}
	} else
	{
		// No region parsing. Call straight through through to the rect blitter.
		parserParams.blitFunction((void *) dv, &dv->minRect);
	}
}

