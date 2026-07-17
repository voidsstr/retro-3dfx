/*******************************************************************************
 * 
 * $Header: mgapicolor1.h, 4, 10/11/00 7:32:12 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:12 PM$
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

#ifndef MGAPICOLOR1_H_
#define MGAPICOLOR1_H_

/*----------------------------------------------------------------------------*/

#include "mgapibase.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/

extern APIFUNC(void) mgIndex2RGB ( mgrec *db_rec, unsigned int index, float intensity, short* r, short* g, short* b );
/* takes a palette color index and intensity (0.0 to 1.0) and returns RGB */

extern APIFUNC(void) mgRGB2Index ( mgrec* db, short r, short g, short b,
											unsigned int *pal_index, float *intensity );
/* given rgb, match and find the palette color index and intensity */

extern APIFUNC(mgbool) mgGetColorIndexByName ( mgrec* db_rec, char *name, int *index );
/* search color names in palette, set palette index if name is in table. */

extern APIFUNC(char) *mgGetNextColorName ( mgrec* db_rec, int index, void **ptr );
/* traverse name list of given index.  if p is NULL, return first name.
	if no more names, return NULL.	*/

extern APIFUNC(char) *mgGetCurrentColorName ( mgrec* db_rec, int index );
/* return current name for given o-fmt palette index (w/o intensity) */

extern APIFUNC(mgbool) mgReadDefaultColorPalette ( char *filename );
/* reads color file from disk and loads it into default color palette */

extern APIFUNC(mgbool) mgReadColorPalette ( mgrec* db_rec, char *filename );
/* reads color file from disk and loads it into database's current color palette */


/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
