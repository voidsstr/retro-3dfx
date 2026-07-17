/*******************************************************************************
 * 
 * $Header: mgapitxtrmap1.h, 4, 10/11/00 7:32:42 PM, Brent$
 * $Revision: 4$
 * $Author: Brent$
 * $Date: 10/11/00 7:32:42 PM$
 *
 ******************************************************************************/

/*============================================================================*\

   PROPRIETARY RIGHTS NOTICE: All rights reserved.  This software contains 
   proprietary information and trade secrets of MultiGen Inc. of San Jose, 
   California, and embodies substantial creative efforts as welll as 
   confidential information, ideas, and expressions.  No part or all of this 
   software may be reproduced in any form, or by any means of electronic, 
   mechanical, or otherwise, without the written permission of MultiGen Inc.

   COPYRIGHT NOTICE: Copyright (C) 1986-1996 MultiGen Inc., San Jose, California.

\*============================================================================*/

/*============================================================================*\
	History
\*============================================================================*/

/* $R_HEAD: $ */

/*----------------------------------------------------------------------------*/

#ifndef LAPITXTRMAP1_H_
#define LAPITXTRMAP1_H_

/*----------------------------------------------------------------------------*/

#include "mgapibase.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/


/*============================================================================*/


/*----------------------------------------------------------------------------*\
	Get Texture Mapping Info
\*----------------------------------------------------------------------------*/

extern APIFUNC(char) *mgGetTextureMappingName ( mgrec* db, int index );
extern APIFUNC(int) mgGetTextureMappingType ( mgrec* db, int index );
extern APIFUNC(mgbool) mgGetTextureMappingMatrix ( mgrec* db, int index,
											double textureMappingMatrix[4][4] );

/*----------------------------------------------------------------------------*\
	Texture Mapping Palette Query
\*----------------------------------------------------------------------------*/

extern APIFUNC(mgbool) mgIsTextureMappingInPalette ( mgrec* db, int index );
extern APIFUNC(mgbool) mgGetFirstTextureMapping ( mgrec* db, int *index,
													char *textureMappingName );
extern APIFUNC(mgbool) mgGetNextTextureMapping ( mgrec* db, int *index,
													char *textureMappingName );

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
