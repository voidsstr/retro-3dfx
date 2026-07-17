/*******************************************************************************
 * 
 * $Header: mgapistruc2.h, 4, 10/11/00 7:32:38 PM, Brent$
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

#ifndef MGAPISTRUC2_H_
#define MGAPISTRUC2_H_

/*----------------------------------------------------------------------------*/

#include "mgapidecl.h"
#include "mgapibase.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/

extern APIFUNC(mgrec) *mgNewRec ( mgcode rcode );
extern APIFUNC(mgrec) *mgDuplicate ( mgrec* rec );
extern APIFUNC(mgbool) mgDelete ( mgrec* rec );
extern APIFUNC(mgbool) mgDetach ( mgrec* rec );
extern APIFUNC(mgbool) mgAttach ( mgrec* parent, mgrec* child );
	/* always put the child to the front/head of the parent's child list */
extern APIFUNC(mgbool) mgAppend ( mgrec* parent, mgrec* child );
	/* always put the child to the end/tail of the parent's child list */
extern APIFUNC(mgbool) mgInsert ( mgrec* previous, mgrec* child );
extern APIFUNC(mgbool) mgReference ( mgrec* rec, mgrec* ref_rec );
	/* make referencing to a reference/master bead; hence rec becomes an instance bead */
extern APIFUNC(mgbool) mgDeReference ( mgrec* rec );
	/* make rec an orphan bead again */

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
