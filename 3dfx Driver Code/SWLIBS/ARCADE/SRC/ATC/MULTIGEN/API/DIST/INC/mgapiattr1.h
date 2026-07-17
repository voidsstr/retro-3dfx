/*******************************************************************************
 * 
 * $Header: mgapiattr1.h, 4, 10/11/00 7:32:09 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:09 PM$
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

#ifndef MGAPIATTR1_H_
#define MGAPIATTR1_H_

/*============================================================================*/

#include "mgapibase.h"
#include "mgapidd1.h"
#include "mgapimem1.h"
#include "mgapimatrix.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/

extern APIFUNC(mgbool) mgGetAttBuf ( mgrec* rec0, mgcode fcode, void* buf );
/* Will fill "buf" with the data of "fcode" in "rec0". */

extern APIFUNC(mgbool) mgGetAttRec ( mgrec* rec_rec, mgcode fcode, mgrec* rec_field_out );
/* Store data record containing "fcode" record data from "rec_rec" */

extern APIFUNC(char) *mgGetName ( mgrec* rec );
/* return pointer to node record's id string */

extern APIFUNC(char) *mgGetComment ( mgrec* rec );
/* return pointer to node record's comment string */

extern APIFUNC (mgbool) mgGetPolyRGBA (mgrec *poly, unsigned char *red, 
										unsigned char *green, unsigned char *blue, unsigned char *alpha);
/* Get current RGBA color values of a polygon  
NOTE:  Polygon colors set in index mode will not be automatically converted to RGB mode */


extern APIFUNC(int) mgGetAttList ( mgrec* rec0, ... );
/* Get data from rec0.  Expect the argument list to be in pairs of
   mgcode and pointer to storage space, ending with NULL terminator */


extern APIFUNC(mgbool) mgHasAtt ( mgrec *parent_rec, mgcode code );
/* return true if this attribute (pointer) record exists in its parent record */


extern APIFUNC(mgbool) mgGetIcoord ( mgrec *rec, mgcode icoordCode, double *x,
									 double *y, double *z);
/* Get values out of Icoord rec.  recname is is the name of the
	desired fltIcoord rec  */

extern APIFUNC(mgbool) mgGetNormColor ( mgrec *rec, mgcode normcolorCode, 
										  float *r, float *g, float *b);
/* Get values out of NormColor rec.  recname is is the name of the
	desired fltNormColor rec  */

extern APIFUNC(mgbool) mgGetIPoint ( mgrec *rec, mgcode ipointCode, 
										  int *x, int *y);
/* Get values out of IPoint rec.  recname is is the name of the
	desired fltIPoint rec  */

extern APIFUNC(mgbool) mgGetColorRGBA ( mgrec *rec, mgcode colorRgbaCode, 
										  float *r, float *g, float *b, float *a);
/* Get values out of ColorRGBA rec.  recname is is the name of the
	desired fltColorRGBA rec  */

extern APIFUNC(mgbool) mgGetPackedColor ( mgrec *rec, mgcode packedcolorCode, 
										  unsigned char *r, unsigned char *g, 
										  unsigned char *b, unsigned char *a);
/* Get values out of PackedColor rec .  recname is is the name of the
	desired fltPackedColor rec  */

extern APIFUNC(mgbool) mgGetFCoord ( mgrec *rec, mgcode fcoordCode, float *x,
									 float *y, float *z);
/* Get values out of FCoord rec.  recname is is the name of the
	desired fltFCoord rec  */

extern APIFUNC(mgbool) mgGetVector ( mgrec *rec, mgcode vectorCode, 
										  float *i, float *j, float *k);
/* Get values out of Vector rec.  recname is is the name of the
	desired fltVector rec  */

extern APIFUNC(mgbool) mgGetMatrix ( mgrec* rec, mgcode matrixCode, mgMatrix *mtx );
/* Get the matrix from either a node record or a transform record.
   Memory is not allocated for the matrix by the function */

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
