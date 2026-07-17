/*******************************************************************************
 * 
 * $Header: mgapitexture2.h, 4, 10/11/00 7:32:41 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:41 PM$
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

#ifndef MGAPITEXTURE2_H_
#define MGAPITEXTURE2_H_

/*----------------------------------------------------------------------------*/

#include "mgapibase.h"
#include "mgapicoord.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/

/*----------------------------------------------------------------------------*\
	Basic Image File Write
\*----------------------------------------------------------------------------*/

extern APIFUNC(int) mgFlipImage ( unsigned char *pixels, int width, int height, int type );
extern APIFUNC(int) mgWriteImage ( char *imageFileName, unsigned char *pixels,
										int type, int width, int height, int compress );
extern APIFUNC(mgbool) mgWriteImageAttributes ( char *imageFileName, mgrec *rec );

/*----------------------------------------------------------------------------*\
	Texture Palette Write
\*----------------------------------------------------------------------------*/

extern APIFUNC(mgbool) mgWriteTexture ( mgrec* db, int index, char* newTextureFileName );
extern APIFUNC(mgbool) mgWriteTexturePalette ( mgrec* db, char *paletteFileName );

/*----------------------------------------------------------------------------*\
	Set Texture Info
\*----------------------------------------------------------------------------*/

extern APIFUNC(void) mgSetTextureName ( mgrec *db, int index, char *textureName );
extern APIFUNC(void) mgSetTexturePosition ( mgrec *db, int index, int x, int y );
extern APIFUNC(void) mgSetTextureTexels ( mgrec *db, int index,
									unsigned char *texels );
extern APIFUNC(void) mgSetTextureAttributes ( mgrec *db, int index, 
											mgrec *attrib_rec );

/*----------------------------------------------------------------------------*\
	Texture Palette Management
\*----------------------------------------------------------------------------*/

extern APIFUNC(mgbool) mgCopyTexturePalette ( mgrec *dstDb, mgrec *srcDb );
extern APIFUNC(int) mgCopyTexture ( mgrec *dstDb, mgrec *srcDb, char *newTextureName, int srcIndex );
extern APIFUNC(void) mgDeleteTexture ( mgrec *db, int index );
extern APIFUNC(mgbool) mgReplaceTexture ( mgrec *db, int index, char *textureName );

/*----------------------------------------------------------------------------*\
	Texture Creation
\*----------------------------------------------------------------------------*/

extern APIFUNC(mgbool) mgNewTexture ( mgrec *db, int index, unsigned char *texels,
							char *textureName, mgrec *attrib_rec, int x, int y );

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
