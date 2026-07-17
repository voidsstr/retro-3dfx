/*******************************************************************************
 * 
 * $Header: mgapimaterial1.h, 4, 10/11/00 7:32:27 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:27 PM$
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

#ifndef MGAPIMATERIAL1_H_
#define MGAPIMATERIAL1_H_

/*----------------------------------------------------------------------------*/

#include "mgapibase.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/

extern APIFUNC(mgrec) *mgGetMaterial ( mgrec* db, int index );
/* 
 * Fill rec with data for material by index.
 */
 
extern APIFUNC(int) mgIndexOfMaterial ( mgrec* db, char* name );
/*
 * Return the index of named material
 */
 
extern APIFUNC(char) *mgNameOfMaterial ( mgrec* db, int index );
/*
 * Return pointer to name string of material (by index)
 */
 
extern APIFUNC(int) mgGetMaterialCount ( mgrec *db );
/*
 * Return the number of materials in the db material table
 */

extern APIFUNC(mgrec) *mgGetFirstMaterial ( mgrec* db, int *index );
/* 
 * Find and return the first material entry in the material table; mgNULL if not found.
 * Also return index (if passed in) of the material in the table, -1 if not found.
 */

extern APIFUNC(mgrec) *mgGetNextMaterial ( mgrec* matrec, int *index );
/* 
 * Find and return the next material entry after the one in rec; mgNULL if not found.
 * Also return index (if passed in) of the next material entry in the table, -1 if not found.
 */

extern APIFUNC(mgbool) mgGetMaterialElem ( mgrec* rec, 
									float *ambred, float *ambgreen, float *ambblue,
									float *difred, float *difgreen, float *difblue, 
									float *spered, float *spegreen, float *speblue,
									float *emired, float *emigreen, float *emiblue,
									float* shine, float* alpha );
/*
 * Get components of material used by polygon in rec  (convenience function)
 */

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
