/*******************************************************************************
 * 
 * $Header: mgapicolor2.h, 4, 10/11/00 7:32:14 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:14 PM$
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

#ifndef MGAPICOLOR2_H_
#define MGAPICOLOR2_H_

/*----------------------------------------------------------------------------*/

#include "mgapibase.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/


extern APIFUNC(mgbool) mgNewColorName ( mgrec* db_rec, int index, char *name );
/* add new name to index's name list; assuming that name is not in any index's */
/* name list already. */

extern APIFUNC(mgbool) mgDeleteColorName ( mgrec* db_rec, int index, char *name );
/* remove name from index's name list; index = -1 means to search entire palette */

extern APIFUNC(mgbool) mgSetCurrentColorName ( mgrec* db_rec, int index, char *name );
/* set current name from given index and name; index = -1 means to search from name symbol table */

extern APIFUNC(mgbool) mgWriteDefaultColorPalette ( char *filename );
/* writes default color palette to disk */

extern APIFUNC(mgbool) mgWriteColorPalette ( mgrec* db_rec, char *filename );
/* writes database's current color palette to disk */

extern APIFUNC(mgbool) mgSetColorIndex ( mgrec *db, int index, short r, short g, short b );
/* assigns rgb values to color index in the color palette for db */

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
