/******************************************************************************
 **																			 **
 ** 	Module:		RAVE_system.h											 **
 ** 																		 **
 ** 	Purpose: 	Header file for RAVE 3D drawing engine SPI				 **
 ** 																		 **
 ** 	Author:		Mike W. Kelley											 **
 ** 																		 **
 ** 	Copyright (C) 1994-95 Apple Computer, Inc.  All rights reserved.	 **
 **		OpenGLª is a trademark of Silicon Graphics, Inc.					 **
 ** 																		 **
 *****************************************************************************/

#ifndef _Drive3D_system_h
#define _Drive3D_system_h

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************************
 *
 * Typedefs of texture/bitmap method functions provided by the drawing engine.
 *
 ***********************************************************************************************/

typedef TQAError (*TQAColorTableNew)(
	TQAColorTableType	pixelType,			/* Depth, color space, etc. */
	void				*pixelData,			/* lookup table entries in pixelType format */
	long				transparentIndex,	/* boolean, false means no transparency, true means index 0 is transparent */
	TQAColorTable		**newTable);		/* (Out) Newly created TQAColorTable */

typedef void (*TQAColorTableDelete)(
	TQAColorTable		*colorTable);		/* Previously allocated by QAColorTableNew() */

typedef TQAError (*TQATextureNew) (
	unsigned long		flags,				/* Mask of kQATexture_xxx flags */
	TQAImagePixelType	pixelType,			/* Depth, color space, etc. */
	const TQAImage		images[],			/* Image(s) for texture */
	TQATexture			**newTexture);		/* (Out) Newly created TQATexture, or NULL on error */ 

typedef TQAError (*TQATextureDetach) (
	TQATexture			*texture);			/* Previously allocated by QATextureNew() */

typedef void (*TQATextureDelete) (
	TQATexture			*texture);			/* Previously allocated by QATextureNew() */

typedef TQAError (*TQATextureBindColorTable)(
	TQATexture			*texture,			/* Previously allocated by QATextureNew() */
	TQAColorTable		*colorTable);		/* Previously allocated by QAColorTableNew() */
	
typedef TQAError (*TQABitmapNew) (
	unsigned long		flags,				/* Mask of kQABitmap_xxx flags */
	TQAImagePixelType	pixelType,			/* Depth, color space, etc. */
	const TQAImage		*image,				/* Image */
	TQABitmap			**newBitmap);		/* (Out) Newly created TQABitmap, or NULL on error */ 

typedef TQAError (*TQABitmapDetach) (
	TQABitmap			*bitmap);			/* Previously allocated by QABitmapNew() */

typedef void (*TQABitmapDelete) (
	TQABitmap			*bitmap);			/* Previously allocated by QABitmapNew() */

typedef TQAError (*TQABitmapBindColorTable)(
	TQABitmap			*bitmap,			/* Previously allocated by QABitmapNew() */
	TQAColorTable		*colorTable);		/* Previously allocated by QAColorTableNew() */
	
/************************************************************************************************
 *
 * Typedefs of private (system-only) functions provided by the drawing engine.
 *
 * The TQADrawPrivateNew function returns a TQADrawPrivate *, which points to the
 * engine-specific private data created for the context. (TQADrawPrivate is a dummy
 * type which is then cast to the correct engine-specific datatype by the engine code.)
 *
 * The TQADrawPrivateDelete function deletes the engine-specific private data.
 *
 * TQAStorePrivateNew and TQAStorePrivateDelete provide the same function as QADrawPrivateNew
 * and TQADrawPrivateDelete, but for the texture and bitmap storage context.
 *
 * TQADrawMethodGet and TQAStoreMethodGet are called by the RAVE manager to retrieve
 * the method pointers for a drawing engine.
 *
 * The TQAEngineCheckDevice function returns TRUE if the engine can render to the
 * indicated GDevice.
 *
 ***********************************************************************************************/

typedef TQAError (*TQADrawPrivateNew) (
	TQADrawContext		*newDrawContext,	/* Draw context to initialize */
	const TQADevice		*device,			/* Target device */
	const TQARect		*rect,				/* Target rectangle (device coordinates) */
	const TQAClip		*clip,				/* 2D clip region (or NULL) */
	unsigned long		flags);				/* Mask of kQAContext_xxx */

typedef void (*TQADrawPrivateDelete) (
	TQADrawPrivate		*drawPrivate);		/* Private context data to delete */

typedef TQAError (*TQAEngineCheckDevice) (
	const TQADevice		*device);			/* Target device */
	
typedef TQAError (*TQAEngineGestalt) (
	TQAGestaltSelector	selector,			/* Gestalt parameter being requested */
	void				*response);			/* Buffer that receives response */

/************************************************************************************************
 *
 * The TQAEngineMethod union is used to represent a single engine method (it's a
 * parameter to QAEngineGetMethod). TQAEngineMethodTag identifies which method is being
 * requested.
 *
 ***********************************************************************************************/

typedef union TQAEngineMethod
{
	TQADrawPrivateNew		drawPrivateNew;		/* Method: Create a private draw context */
	TQADrawPrivateDelete	drawPrivateDelete;	/* Method: Delete a private draw context */
	TQAEngineCheckDevice	engineCheckDevice;	/* Method: Check a device for drawing */
	TQAEngineGestalt		engineGestalt;		/* Method: Gestalt */
	TQATextureNew			textureNew;			/* Method: Create a texture (load is non-blocking) */
	TQATextureDetach		textureDetach;		/* Method: Complete load of a texture (blocking) */
	TQATextureDelete		textureDelete;		/* Method: Delete a texture */
	TQABitmapNew			bitmapNew;			/* Method: Create a bitmap (load is non-blocking)  */
	TQABitmapDetach			bitmapDetach;		/* Method: Complete load of a bitmap (blocking) */
	TQABitmapDelete			bitmapDelete;		/* Method: Delete a bitmap */
	TQAColorTableNew		colorTableNew;		/* Method: Create a new color table */
	TQAColorTableDelete		colorTableDelete;	/* Method: Create a new color table */
	TQATextureBindColorTable	textureBindColorTable;	/* Method: Bind a CLUT to a texture */
	TQABitmapBindColorTable		bitmapBindColorTable;	/* Method: Bind a CLUT to a bitmap */
} TQAEngineMethod;

typedef enum TQAEngineMethodTag
{
	kQADrawPrivateNew		= 0,
	kQADrawPrivateDelete	= 1,
	kQAEngineCheckDevice	= 2,
	kQAEngineGestalt		= 3,
	kQATextureNew			= 4,
	kQATextureDetach		= 5,
	kQATextureDelete		= 6,
	kQABitmapNew			= 7,
	kQABitmapDetach			= 8,
	kQABitmapDelete			= 9,
	kQAColorTableNew		= 10,
	kQAColorTableDelete		= 11,
	kQATextureBindColorTable	= 12,
	kQABitmapBindColorTable		= 13
} TQAEngineMethodTag;

/************************************************************************************************
 *
 * QARegisterEngine() registers a new engine. This is called at boot time by the drawing engine
 * initialization code to register itself with the system. This call takes only one parameter,
 * the engine's function that allows the manager to request the other methods.
 *
 ***********************************************************************************************/

typedef TQAError (*TQAEngineGetMethod) (
	TQAEngineMethodTag		methodTag,				/* Method being requested */
	TQAEngineMethod			*method);				/* (Out) Method */

RAVE_EXPORT TQAError QARegisterEngine (
	TQAEngineGetMethod		engineGetMethod);		/* Engine's getMethod method */

/************************************************************************************************
 *
 * The TQADrawMethod union is used to represent a single draw context method (it's a
 * parameter to QARegisterDrawMethod). TQADrawMethodTag identifies which method is being
 * passed.
 *
 ***********************************************************************************************/

typedef union TQADrawMethod
{
	TQASetFloat				setFloat;			/* Method: Set a float state variable */
	TQASetInt				setInt;				/* Method: Set an unsigned long state variable */
	TQASetPtr				setPtr;				/* Method: Set an unsigned long state variable */
	TQAGetFloat				getFloat;			/* Method: Get a float state variable */
	TQAGetInt				getInt;				/* Method: Get an unsigned long state variable */
	TQAGetPtr				getPtr;				/* Method: Get an pointer state variable */
	TQADrawPoint			drawPoint;			/* Method: Draw a point */
	TQADrawLine				drawLine;			/* Method: Draw a line */
	TQADrawTriGouraud		drawTriGouraud;		/* Method: Draw a Gouraud shaded triangle */
	TQADrawTriTexture		drawTriTexture;		/* Method: Draw a texture mapped triangle */
	TQADrawVGouraud			drawVGouraud;		/* Method: Draw Gouraud vertices */
	TQADrawVTexture			drawVTexture;		/* Method: Draw texture vertices */
	TQADrawBitmap			drawBitmap;			/* Method: Draw a bitmap */
	TQARenderStart			renderStart;		/* Method: Initialize for rendering */
	TQARenderEnd			renderEnd;			/* Method: Complete rendering and display */
	TQARenderAbort			renderAbort;		/* Method: Abort any outstanding rendering (blocking) */
	TQAFlush				flush;				/* Method: Start render of any queued commands (non-blocking) */
	TQASync					sync;				/* Method: Wait for completion of all rendering (blocking) */
	TQASubmitVerticesGouraud	submitVerticesGouraud;	/* Method: Submit Gouraud vertices for trimesh */
	TQASubmitVerticesTexture	submitVerticesTexture;	/* Method: Submit Texture vertices for trimesh */
	TQADrawTriMeshGouraud		drawTriMeshGouraud;		/* Method: Draw a Gouraud triangle mesh */
	TQADrawTriMeshTexture		drawTriMeshTexture;		/* Method: Draw a Texture triangle mesh */

#if RAVE_1_1
	TQASetNoticeMethod			setNoticeMethod;		/* Method: Set a notice method */
	TQAGetNoticeMethod			getNoticeMethod;		/* Method: Get a notice method */
#endif
} TQADrawMethod;

typedef enum TQADrawMethodTag
{
	kQASetFloat				= 0,
	kQASetInt				= 1,
	kQASetPtr				= 2,
	kQAGetFloat				= 3,
	kQAGetInt				= 4,
	kQAGetPtr				= 5,
	kQADrawPoint			= 6,
	kQADrawLine				= 7,
	kQADrawTriGouraud		= 8,
	kQADrawTriTexture		= 9,
	kQADrawVGouraud			= 10,
	kQADrawVTexture			= 11,
	kQADrawBitmap			= 12,
	kQARenderStart			= 13,
	kQARenderEnd			= 14,
	kQARenderAbort			= 15,
	kQAFlush				= 16,
	kQASync					= 17,
	kQASubmitVerticesGouraud	= 18,
	kQASubmitVerticesTexture	= 19,
	kQADrawTriMeshGouraud		= 20,
	kQADrawTriMeshTexture		= 21,
	kQASetNoticeMethod			= 22,
	kQAGetNoticeMethod			= 23
} TQADrawMethodTag;

/************************************************************************************************
 *
 * System call to register a new method for an engine. This is called during the engine's
 * draw private new functions (to set the initial value of the draw methods), and possibly
 * at other times when the engine needs to change a draw method.
 *
 ***********************************************************************************************/

RAVE_EXPORT TQAError QARegisterDrawMethod (
	TQADrawContext			*drawContext,			/* Draw context in which to set method */
	TQADrawMethodTag		methodTag,				/* Method to set */
	TQADrawMethod			method);				/* Method */

/************************************************************************************************
 *
 * RAVE Prefs file.
 *
 ***********************************************************************************************/

		 								  /*  1		.     1            d            1   */
#define kQAPrefsFileVersion_Current			((1 << 24) + (1 << 16) + (0x20 << 8) + (1))

#define kQAPrefsFileVersion_Minimum			kQAPrefsFileVersion_Current

/************************************************************************************************
 *
 * Debugging functions.
 *
 ***********************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* _Drive3D_system_h */
