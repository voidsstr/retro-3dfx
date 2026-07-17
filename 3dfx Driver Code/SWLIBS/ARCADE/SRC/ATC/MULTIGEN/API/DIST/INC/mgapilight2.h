/*******************************************************************************
 * 
 * $Header: mgapilight2.h, 4, 10/11/00 7:32:25 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:25 PM$
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

#ifndef MGAPILIGHT2_H_
#define MGAPILIGHT2_H_

/*----------------------------------------------------------------------------*/

#include "mgapibase.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/

extern APIFUNC(mgrec) *mgNewLightSource ( mgrec *db, char *name, int *index );
/* Allocate and return a new light source entry; return mgNULL if failed.
	Also return the new index in the light source table; return -1 if failed.
	"name" and "index" are optional */


extern APIFUNC(mgbool) mgWriteLightSourceFile ( mgrec* db, char* fname );

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
