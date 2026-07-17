/*******************************************************************************
 * 
 * $Header: mgapiattr2.h, 4, 10/11/00 7:32:10 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:10 PM$
 *
 ******************************************************************************/

/*============================================================================*\

   PROPRIETARY RIGHTS NOTICE: All rights reserved.  This software contains 
   proprietary information and trade secrets of MultiGen Inc. of San Jose, 
   California, and embodies substantial creative efforts as well as 
   confidential information, ideas, and expressions.  No part or all of this 
   software may be reproduced in any form, or by any means of electronic, 
   mechanical, or otherwise, without the written permission of MultiGen Inc.

   COPYRIGHT NOTICE: Copyright (C) 1986-1996 MultiGen Inc., San Jose, California.

\*============================================================================*/

/*----------------------------------------------------------------------------*/

#ifndef MGAPIATTR2_H_
#define MGAPIATTR2_H_

/*----------------------------------------------------------------------------*/

#include "mgapibase.h"
#include "mgapimatrix.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/

extern APIFUNC(mgbool) mgSetAttBuf ( mgrec* rec0, mgcode fcode, void* buf );
/* copy the content of a field (may be more than one) from the passed-in data buffer */

extern APIFUNC(int) mgSetAttList ( mgrec* rec0, ... );
/* Set data in rec0.  Expect the argument list to be in pairs of
	mgcode and a new value, ending with NULL terminator */

extern APIFUNC(mgbool) mgSetName ( mgrec* rec, char* name );
/* Assign a name to a node record */

extern APIFUNC(mgbool) mgSetComment ( mgrec* rec, char* comment );
/* Set the comment field of a node record.  A comment of NULL is ignored.  
	The	previous comment string (if any) will be deallocated first. */

extern APIFUNC(mgbool) mgDeleteComment ( mgrec* rec );
/* Deallocate comment string and set comment field pointer to NULL */

extern APIFUNC (mgbool) mgSetPolyRGBA (mgrec *poly, unsigned char red, 
										unsigned char green, unsigned char blue, unsigned char alpha);
/* Set the RGBA color of a polygon (RGBMode flag must be set) */



extern APIFUNC(mgbool) mgSetIcoord ( mgrec *rec, mgcode icoordCode, 
												double x, double y, double z);
/* Set values in Icoord rec.  recname is is the name of the
	desired fltIcoord rec  */

extern APIFUNC(mgbool) mgSetNormColor ( mgrec *rec, mgcode normcolorCode, 
										  float r, float g, float b);
/* Set values in NormColor rec.  recname is is the name of the
	desired fltNormColor rec  */

extern APIFUNC(mgbool) mgSetIPoint ( mgrec *rec, mgcode ipointCode, 
										  int x, int y);
/* Set values in IPoint rec.  recname is is the name of the
	desired fltIPoint rec  */

extern APIFUNC(mgbool) mgSetColorRGBA ( mgrec *rec, mgcode colorRgbaCode, 
										  float r, float g, float b, float a);
/* Set values in ColorRGBA rec.  recname is is the name of the
	desired fltColorRGBA rec  */

extern APIFUNC(mgbool) mgSetPackedColor ( mgrec *rec, mgcode packedcolorCode, 
										  unsigned char r, unsigned char g, 
										  unsigned char b, unsigned char a);
/* Set values in PackedColor rec .  recname is is the name of the
	desired fltPackedColor rec  */

extern APIFUNC(mgbool) mgSetFCoord ( mgrec *rec, mgcode fcoordCode, 
												float x, float y, float z);
/* Set values in FCoord rec.  recname is is the name of the
	desired fltFCoord rec  */

extern APIFUNC(mgbool) mgSetVector ( mgrec *rec, mgcode vectorCode, 
										  float i, float j, float k);
/* Set values in Vector rec.  recname is is the name of the
	desired fltVector rec  */

extern APIFUNC(mgbool) mgSetMatrix ( mgrec* rec, mgcode matrixCode, mgMatrix mtx );
/* Set the matrix of general transform record. */

/*============================================================================*/



#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
