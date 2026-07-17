/*******************************************************************************
 * 
 * $Header: mgapiutil4.h, 4, 10/11/00 7:32:45 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:45 PM$
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

#ifndef MGAPIUTIL4_H_
#define MGAPIUTIL4_H_

/*----------------------------------------------------------------------------*/

#include "mgapibase.h"
#include "mgapimatrix.h"
#include "mgapill.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/

extern APIFUNC(owll) *mgGetSplitList ( mgrec* rec );
extern APIFUNC(mgbool) mgSplitBigPoly ( mgrec* face_rec, int maxverts );
extern APIFUNC(owll) *mgTriangulate ( mgrec* rec );
extern APIFUNC(mgbool) mgReverse ( mgrec *rec );
extern APIFUNC(mgbool) mgIsCoplanar ( mgrec *rec );
extern APIFUNC(int) mgParseFname ( char *str, char **pathname, char **fname, char **ext );
extern APIFUNC(void) mgUpdateMatrix ( mgrec* rec );
	/* multiply a matrix to a record, which may already have a matrix */

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
