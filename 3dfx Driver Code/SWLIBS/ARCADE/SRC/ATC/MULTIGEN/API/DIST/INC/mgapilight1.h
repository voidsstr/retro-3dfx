/*******************************************************************************
 * 
 * $Header: mgapilight1.h, 4, 10/11/00 7:32:25 PM, Brent$
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

#ifndef MGAPILIGHT1_H_
#define MGAPILIGHT1_H_

/*----------------------------------------------------------------------------*/

#include "mgapibase.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/


extern APIFUNC(mgrec) *mgGetLightSource ( mgrec *db, int index );
extern APIFUNC(int) mgIndexOfLightSource ( mgrec* db, char* name );
extern APIFUNC(char) *mgNameOfLightSource ( mgrec* db, int index );

extern APIFUNC(int) mgGetLightSourceCount ( mgrec *db );
extern APIFUNC(mgrec) *mgGetFirstLightSource ( mgrec *db, int *index );
/* 
 * Find and return the first light source entry in the light source table; mgNULL if not found.
 * Also return index (if passed in) of the light source in the table, -1 if not found.
 */

extern APIFUNC(mgrec) *mgGetNextLightSource ( mgrec *ltsrec, int *index );
/* 
 * Find and return the next light source entry after the one in rec; mgNULL if not found.
 * Also return index (if passed in) of the next light source entry in the table, -1 if not found;
 */

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
