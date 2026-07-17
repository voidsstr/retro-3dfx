/******************************************************************************
	File:		
	Copyright:	© 1995-99 Critical Path Software Inc.  All Rights Reserved.
	Author:		Andrew Mellinger
	Purpose:	
	History:	
 ******************************************************************************/
#pragma once

	/* Headers */
	/* Constants */
	/* Typedefs */
/******************************************************************************/
typedef void (*SlabBlitProc)(NQDDrawVars* drawVars, Rect &rect);

	/* Globals */
	/* Classes */
	/* Prototypes */
/******************************************************************************/
void PatBlitSolid(NQDDrawVars *drawVars, Rect &dstRect);
void PatBlitSmallPattern(NQDDrawVars *drawVars, Rect &dstRect);
void PatBlit1BitPattern(NQDDrawVars *drawVars, Rect &dstRect);

SlabBlitProc  SelectSlabPatBlitProc(NQDDrawVars  *drawVars);

	/* Macros */
/******************************************************************************/
/*----------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
