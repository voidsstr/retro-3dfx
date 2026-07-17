/*******************************************************************************
 * 
 * $Header: mgapimath4.h, 4, 10/11/00 7:32:30 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:30 PM$
 *
 ******************************************************************************/

/*============================================================================*\

   PROPRIETARY RIGHTS NOTICE: All rights reserved.  This software contains 
   proprietary information and trade secrets of MultiGen Inc. of San Jose, 
   California, and embodies substantial creative efforts as well as 
   confidential information, ideas, and expressions.  No part or all of this 
   software may be reproduced in any form, or by any means of electronic, 
   mechanical, or otherwise, without the written permission of MultiGen Inc.

   COPYRIGHT NOTICE: Copyright(C) 1986-1996 MultiGen Inc., San Jose, California.

\*============================================================================*/

/*----------------------------------------------------------------------------*/

#ifndef MGAPIMATH4_H_
#define MGAPIMATH4_H_

/*----------------------------------------------------------------------------*/

#include "mgapibase.h"
#include "mgapicoord.h"
#include "mgapimatrix.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/

extern APIFUNC(void) mgMatrixFormRotate ( mgMatrix *m, double theta, double a, double b, double c );
/* form a rotation matrix for rotation around a vector given direction cosines */

extern APIFUNC(icoord) mgTransformCoord ( mgMatrix m, icoordpt p );
/* transform a coordinate by applying a transformation matrix */

extern APIFUNC(icoord) mgAddCoord (icoordpt c1, icoordpt c2);
/* add two coordinates */

extern APIFUNC(dvector) mgVectorFromLine ( dlinept line );
/* forms a vector from a line */

extern APIFUNC(void) mgUnitizeVector (dvector *v);
/* changes a vector to a unit vector */

extern APIFUNC(double) mgDistance (icoord *fp1, icoord *fp2);
/* get the distance between two coordinates */

extern APIFUNC(dvector) mgMakeVector (icoord *fp1, icoord *fp2);
/* make a vector from two coordinates */

extern APIFUNC(dvector) mgCrossProdVector (dvector *a, dvector *b);
/* get the cross product of two vectors */

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
