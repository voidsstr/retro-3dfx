/*******************************************************************************
 * 
 * $Header: mgapitexture1.h, 4, 10/11/00 7:32:40 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:40 PM$
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

#ifndef MGAPITEXTURE1_H_
#define MGAPITEXTURE1_H_

/*----------------------------------------------------------------------------*/

#include "mgapibase.h"
#include "mgapicoord.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/

typedef struct {
	int	numberofchannels;
	int	texelsize;
	int	memorysize;
} MGTextureStatistics;

typedef struct {
	int						numtextures;
	MGTextureStatistics	*statistics;
} MGTexturePaletteStatistics;

/*============================================================================*/

/*----------------------------------------------------------------------------*\
	Basic Image File Read
\*----------------------------------------------------------------------------*/

extern APIFUNC(int) mgReadImage ( char *imageFileName, unsigned char **pixels,
							  int *type, int *width, int *height );
extern APIFUNC(int) mgReadImageHeader ( char *imageFileName, int *type, int *width,
										int *height );
extern APIFUNC(mgrec) *mgReadImageAttributes ( char* imageFileName );

/*----------------------------------------------------------------------------*\
	Texture Palette Loading
\*----------------------------------------------------------------------------*/

extern APIFUNC(mgbool) mgReadTexture ( mgrec* db, char *textureFileName,
								int index, int x, int y );
extern APIFUNC(mgbool) mgReadTextureAndAlpha ( mgrec* db, char *textureFileName,
								char *alphaFileName, char *mergeTextureName,
								int index, int x, int y );
extern APIFUNC(int) mgInsertTexture ( mgrec* db, char *textureFileName );
extern APIFUNC(int) mgInsertTextureAndAlpha ( mgrec* db, char *textureFileName,
								char *alphaFileName, char *mergeTextureName );
extern APIFUNC(int) mgReadTexturePalette ( mgrec* db, char *paletteFileName );

/*----------------------------------------------------------------------------*\
	Get Texture Info
\*----------------------------------------------------------------------------*/

extern APIFUNC(int) mgGetTextureIndex ( mgrec* db, char* textureName );
extern APIFUNC(char) *mgGetTextureName ( mgrec* db, int index );
extern APIFUNC(mgbool) mgGetTexturePosition ( mgrec* db, int index, int *x, int *y );
extern APIFUNC(unsigned char) *mgGetTextureTexels ( mgrec* db, int index );
extern APIFUNC(mgrec) *mgGetTextureAttributes ( mgrec* db, int index );

/*----------------------------------------------------------------------------*\
	Texture Palette Query
\*----------------------------------------------------------------------------*/

extern APIFUNC(mgbool) mgIsTextureInPalette ( mgrec* db, char *textureName );
extern APIFUNC(mgbool) mgGetFirstTexture ( mgrec* db, int *index, char *textureName );
extern APIFUNC(mgbool) mgGetNextTexture ( mgrec* db, int *index, char *textureName );
extern APIFUNC(mgbool) mgIsTextureDefault ( mgrec* db, int index );

/*----------------------------------------------------------------------------*\
	Texture Palette Statistics
\*----------------------------------------------------------------------------*/

extern APIFUNC(int) mgGetTextureCount ( mgrec* db );
extern APIFUNC(int) mgGetTextureTotalSize ( mgrec* db );
extern APIFUNC(int) mgGetTextureSize ( mgrec* db, int index );

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
