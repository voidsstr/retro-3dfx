/*******************************************************************************
 * 
 * $Header: mgapistruc1.h, 4, 10/11/00 7:32:38 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:38 PM$
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

#ifndef MGAPISTRUC1_H_
#define MGAPISTRUC1_H_

/*----------------------------------------------------------------------------*/

#include "mgapibase.h"
#include "mgapixform.h"
#include "mgapimatrix.h"

typedef enum mgwalk_type {
	udlr,				/* depth first, then left-to-right */
	udrl,				/* depth first, then right-to-left */
	lrud,				/* left-to-right, depth last */
	rlud				/* right-to-left, depth last */
} mgwalk_type;

typedef enum mgwalk_flag {
	MGWALKNEXT = 1,			/* traverse next bead */
	MGWALKON = 2,			/* traverse only "on" (currently displayed LOD) beads */
	MGWALKMASTER = 4,		/* traverse master bead */
	MGWALKNORDONLY = 8,		/* don't traverse read-only beads */
	MGWALKVERTEX = 16,		/* traverse vertex beads */
	MGWALKMASTERALL = 32,	/* traverse all instances */
	MGWALKATTR = 64		/* traverse attribute beads */
} mgwalk_flag;


typedef mgbool ( *mgcompare_func ) ( void*, void* );
typedef mgbool	( *mgwalk_func ) ( mgrec* db_rec, mgrec* par_rec, mgrec* rec, void *info );

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
extern APIFUNC(char) *mgRec2Filename (mgrec *rec);
extern APIFUNC(void) mgSetCurrentDb (mgrec *db);
extern APIFUNC(mgrec) *mgGetCurrentDb (void);
extern APIFUNC(mgrec) *mgRec2Db ( mgrec* rec );	/* should db be saved? */
extern APIFUNC(int) mgCountChild ( mgrec* rec );
extern APIFUNC(mgrec) *mgGetChildNth ( mgrec* parent, int nth );
extern APIFUNC(mgrec) *mgGetRecByName ( mgrec* rec0, char* name );
extern APIFUNC(mgrec) *mgGetNext ( mgrec* rec0 );
extern APIFUNC(mgrec) *mgGetPrevious ( mgrec* rec0 );
extern APIFUNC(mgrec) *mgGetParent ( mgrec* rec0 );
extern APIFUNC(mgrec) *mgGetChild ( mgrec* rec0 );
extern APIFUNC(mgrec) *mgGetNestedParent ( mgrec* rec0 );
extern APIFUNC(mgrec) *mgGetNestedChild ( mgrec* rec0 );
extern APIFUNC(mgrec) *mgGetReference ( mgrec* rec0 );
extern APIFUNC(mgrec) *mgGetFirstInstance ( mgrec* rec0 );
extern APIFUNC(mgrec) *mgGetNextInstance ( mgrec* rec0 );
extern APIFUNC(mgbool) mgWalk ( mgrec* rec0, mgwalk_func preaction, mgwalk_func postaction,
									void *info, int flags );
extern APIFUNC(mgbool) mgHasXform ( mgrec* rec );
extern APIFUNC(mgrec) *mgGetXform ( mgrec* rec0 );
extern APIFUNC(mgxfllcode) mgGetXformType ( mgrec* rec );
extern APIFUNC(mgbool) mgMoreDetail ( mgrec *db );
extern APIFUNC(mgbool) mgLessDetail ( mgrec *db );
extern APIFUNC(mgbool) mgMostDetail ( mgrec *db );
extern APIFUNC(mgbool) mgLeastDetail ( mgrec *db );
extern APIFUNC(mgbool) mgIsReference ( mgrec *rec );
extern APIFUNC(mgbool) mgIsInstance ( mgrec *rec );
extern APIFUNC(mgbool) mgIsFirstInstance ( mgrec* rec0 );
extern APIFUNC(mgbool) mgIsFlagOn ( mgrec* rec );

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
