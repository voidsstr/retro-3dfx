/******************************************************************************
 **																			 **
 ** 	Module:		RAVE.h													 **
 ** 																		 **
 ** 	Purpose: 	Header file for RAVE API								 **
 ** 																		 **
 ** 	Author:		Mike W. Kelley											 **
 ** 																		 **
 ** 	Copyright (C) 1994-96 Apple Computer, Inc.  All rights reserved.	 **
 **		OpenGLª is a trademark of Silicon Graphics, Inc.					 **
 ** 																		 **
 *****************************************************************************/

#ifndef _RAVE_h
#define _RAVE_h

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Platform macros.
 * This sets kQAPlatform to one of kQAMacOS, kQAWin32, or kQAGeneric.
 * kQAPlatform controls platform-specific compilation switches and types.
 *
 *****************************************************************************/

#if !defined(kQAMacOS)
#define	kQAMacOS		1			/* Target is MacOS					*/
#endif

#if !defined(kQAGeneric)
#define kQAGeneric		2			/* Target is generic platform		*/
#endif

#if !defined(kQAWin32)
#define kQAWin32		3			/* Target is Win32					*/
#endif

#if defined(_WIN32) || defined(_WINDOWS)
	#define kQAPlatform kQAWin32
#elif !defined(kQAPlatform)
	#define kQAPlatform kQAMacOS
#endif

/******************************************************************************
 *
 * Export Control
 *
 *****************************************************************************/

#if defined(_MSC_VER)	/* Microsoft Visual C */
	#define	RAVE_EXPORT		__declspec( dllexport )
#else
	#define RAVE_EXPORT
#endif /* _MSC_VER */

#include <d3d.h>

/******************************************************************************
 *
 * Platform dependent datatypes: TQAImagePixelType, TQADevice, TQAClip, and TQARect.
 *
 *****************************************************************************/

typedef enum TQAImagePixelType
{
	kQAPixel_Alpha1	= 0,		/* 1 bit/pixel alpha */
	kQAPixel_RGB16		= 1,		/* 16 bit/pixel, R=14:10, G=9:5, B=4:0 */
	kQAPixel_ARGB16	= 2,		/* 16 bit/pixel, A=15, R=14:10, G=9:5, B=4:0 */
	kQAPixel_RGB32		= 3,		/* 32 bit/pixel, R=23:16, G=15:8, B=7:0 */
	kQAPixel_ARGB32	= 4,		/* 32 bit/pixel, A=31:24, R=23:16, G=15:8, B=7:0 */
	kQAPixel_CL4		= 5,		/* 4 bit color look up table, always big endian,
										ie high 4 bits effect left pixel */
	kQAPixel_CL8		= 6,		/* 8 bit color look up table */
	kQAPixel_RGB8_332	= 7,		/* 8 bit RGB, 3 for red, 3 green, 2 blue */
	kQAPixel_YIQ8_422	= 8,		/*8 bit YIQ */
	kQAPixel_A8			= 9,		/* 8 bit alpha */ 
	kQAPixel_I8			=10,		/* 8 bit intensity */
	kQAPixel_AI8_44	=11,		/* 8 bit alpha,intensity, 4 bits each */
	kQAPixel_ARGB16_8332 = 12,	/* 16 bit alpha, RGB */
	kQAPixel_AYIQ16_8422 = 13,	/* 16 bit alpha with YIQ */
	kQAPixel_RGB16_565	=14,	/* 16 bit RGB, 5 red, 6 green, 5 blue */
	kQAPixel_ARGB16_1555	=15,	/* 16 bit ARGB, 1 bit alpha 5 each for the colors */
	kQAPixel_ARGB16_4444	=16,	/* 16 bit ARGB, 4 bits each */
	kQAPixel_AI16_88		=17,	/* 16 bit alpha/intensity, 8 bits each */
	kQAPixel_AP16_88		=18,	/* 16 bit alpha/palettized color */
	kQAPixel_ARGB32_8888 = 19	/* full 32 bit color */	
}	TQAImagePixelType;

typedef enum TQAColorTableType
{
	kQAColorTable_CL8_RGB32		= 0,		/* 256 entry, 32 bit/pixel, R=23:16, G=15:8, B=7:0 */
	kQAColorTable_CL4_RGB32		= 1			/* 16 entry, 32 bit/pixel, R=23:16, G=15:8, B=7:0 */
} TQAColorTableType;

typedef enum TQADeviceType			/* Selects target device type */
{
	kQADeviceMemory		= 0,		/* Memory draw context */
	kQADeviceGDevice	= 1,		/* Macintosh GDevice draw context */
	kQADeviceWin32DC	= 2,		/* Win32 DC  */
	kQADeviceDDSurface	= 3,			/* Win32 DirectDraw Surface */
   kQADeviceD3DDevice	=	4			/* Win-95 Direct3D device */
} TQADeviceType;

typedef struct TQADeviceMemory		/* Generic memory pixmap device */
{
	long				rowBytes;	/* Rowbytes */
	TQAImagePixelType	pixelType;	/* Depth, color space, etc. */
	long				width;		/* Width in pixels */
	long				height;		/* Height in pixels */
	void				*baseAddr;	/* Base address of pixmap */
} TQADeviceMemory;

typedef enum TQAClipType			/* Selects target clip type */
{
	kQAClipRgn			= 0,		/* Macintosh clipRgn with serial number */
	kQAClipWin32Rgn		= 1			/* Win32 clip region */
} TQAClipType;

typedef struct TQARect
{
	long				left;
	long				right;
	long				top;
	long				bottom;
} TQARect;

#if (kQAPlatform == kQAMacOS)
	/*
	 * MacOS supports memory and GDevice. TQARect == Rect. TQAClip is a clipRgn.
	 */
	
	#include <Quickdraw.h>
	#include <QDOffscreen.h>
	
	typedef union TQAPlatformDevice
	{
		TQADeviceMemory	memoryDevice;
		GDHandle		gDevice;
	} TQAPlatformDevice;
	
	typedef union TQAPlatformClip
	{
		RgnHandle		clipRgn;
	} TQAPlatformClip;
	
#elif (kQAPlatform == kQAWin32)
	/*
	 * Win32 platform. TQARect is generic. TQAPlatform is DC or Surface. TQAClip is HRGN.
	 */
	#include <windows.h>
 	#include <ddraw.h>

///	typedef union TQAPlatformDevice

	typedef union TQAPlatformDevice
	{
		TQADeviceMemory			memoryDevice;
		HDC							hdc;

 		struct
 		{
 			LPDIRECTDRAW				lpDirectDraw;
  			LPDIRECTDRAWSURFACE		lpDirectDrawSurface;
 		}dd;

      struct
      {
      	HDC							hdc;
      	HWND							hwnd;		// needed so I can get Window size and location

    		LPDIRECT3DDEVICE	lpD3Ddev; 	//draw to this for D3D acceleration instead of a DD device
  			LPDIRECT3DVIEWPORT lpView;
   		LPDIRECT3D	lpD3D;
 			LPDIRECTDRAW	lpDirectDraw;  //needed by texturemapping stuff
  			LPDIRECTDRAWSURFACE		lpFront;
  			LPDIRECTDRAWSURFACE		lpBack;
  			LPDIRECTDRAWSURFACE		lpZbuffer;
      }d3d;


	} TQAPlatformDevice;

	typedef union TQAPlatformClip
	{
		HRGN			clipRgn;
	} TQAPlatformClip;
#elif (kQAPlatform == kQAGeneric)
	/*
	 * Generic platform supports memory device only. TQARect is generic. TQAClip is ???.
	 */
	
	typedef union TQAPlatformDevice
	{
		TQADeviceMemory	memoryDevice;
	} TQAPlatformDevice;
	
	typedef union TQAPlatformClip
	{
		void			*region;			/* ??? */
	} TQAPlatformClip;
#else
	??? Unrecognized kQAPlatform
#endif

typedef struct TQADevice
{
	TQADeviceType		deviceType;
	TQAPlatformDevice	device;
} TQADevice;

typedef struct TQAClip
{
	TQAClipType			clipType;
	TQAPlatformClip		clip;
} TQAClip;

/******************************************************************************
 *
 * Basic data types.
 *
 *****************************************************************************/

typedef struct	TQADrawContext TQADrawContext;	/* Drawing context for an engine */
typedef struct	TQAEngine TQAEngine;			/* Pointer to a drawing engine */
typedef struct	TQATexture TQATexture;			/* Pointer to an allocated texture map */
typedef struct	TQABitmap TQABitmap;			/* Pointer to an allocated bitmap */
typedef struct	TQADrawPrivate TQADrawPrivate;	/* Engine's private draw context pointer */
typedef struct	TQAImage TQAImage;				/* An image for use as texture or bitmap */
typedef struct	TQAColorTable TQAColorTable;

typedef struct TQAEnginex TQAEnginex;

typedef struct TQAIndexedTriangle				/* A single triangle element for QADrawTriMesh */
{
	unsigned long	triangleFlags;				/* Triangle flags, see kQATriFlags_ */
	unsigned long	vertices[3];				/* Indices into a vertex array */
} TQAIndexedTriangle;

struct TQAImage									/* An image for use as texture or bitmap */
{
	long		width;							/* Width of pixmap */
	long		height;							/* Height of pixmap */
	long		rowBytes;						/* Rowbytes of pixmap */
	void		*pixmap;						/* Pixmap */
};

typedef enum TQAError							/* Standard error type */
{
	kQANoErr				= 0,				/* No error */
	kQAError				= 1,				/* Generic error flag */
	kQAOutOfMemory			= 2,				/* Insufficient memory */
	kQANotSupported			= 3,				/* Requested feature is not supported */
	kQAOutOfDate			= 4,				/* A newer drawing engine was registered */
	kQAParamErr				= 5,				/* Error in passed parameters */
#ifdef RAVE_1_1
	kQAGestaltUnknown		= 6,				/* Requested gestalt type isn't available */
	kQADisplayModeUnsupported	= 7				/* Engine cannot render to the display in its current ,
												 * mode, but could if it were in some other mode */
#else
	kQAGestaltUnknown		= 6					/* Requested gestalt type isn't available */
#endif
} TQAError;

/************************************************************************************************
 *
 * Vertex data types.
 *
 ***********************************************************************************************/

/*
 * TQAVGouraud is used for Gouraud shading. Each vertex specifies position, color and Z.
 *
 * Alpha is always treated as indicating transparency. Drawing engines which don't
 * support Z-sorted rendering use the back-to-front transparency blending functions
 * shown below. (ARGBsrc are the source (new) values, ARGBdest are  the destination
 * (previous) pixel values.)
 *
 *		Premultiplied							Interpolated
 *
 *		A = 1 - (1 - Asrc) * (1 - Adest)		A = 1 - (1 - Asrc) * (1 - Adest) 
 *		R = (1 - Asrc) * Rdest + Rsrc			R = (1 - Asrc) * Rdest + Asrc * Rsrc
 *		G = (1 - Asrc) * Gdest + Gsrc			G = (1 - Asrc) * Gdest + Asrc * Gsrc
 *		B = (1 - Asrc) * Bdest + Bsrc			B = (1 - Asrc) * Bdest + Asrc * Bsrc
 *
 * Note that the use of other blending modes to implement antialiasing is performed
 * automatically by the drawing engine when the kQATag_Antialias variable !=
 * kQAAntiAlias_Fast. The driving software should continue to use the alpha fields
 * for transparency even when antialiasing is being used (the drawing engine will
 * resolve the multiple blending requirements as best as it can).
 *
 * Drawing engines which perform front-to-back Z-sorted rendering should replace
 * the blending function shown above with the equivalent front-to-back formula.
 */

typedef struct TQAVGouraud
{
	float				x;		/* X pixel coordinate, 0.0 <= x < width */
	float				y;		/* Y pixel coordinate, 0.0 <= y < height */
	float				z;		/* Z coordinate, 0.0 <= z <= 1.0 */
	float				invW;	/* 1 / w; required only when kQAPerspectiveZ_On is set */
	
	float				r;		/* Red, 0.0 <= r <= 1.0 */
	float				g;		/* Green, 0.0 <= g <= 1.0 */
	float				b;		/* Blue, 0.0 <= b <= 1.0 */
	float				a;		/* Alpha, 0.0 <= a <= 1.0, 1.0 is opaque */
} TQAVGouraud;

/*
 * TQAVTexture is used for texture mapping. The texture mapping operation
 * is controlled by the kQATag_TextureOp variable, which is a mask of
 * kQATextureOp_None/Modulate/Highlight/Decal. Below is pseudo-code for the
 * texture shading operation:
 *
 *		texPix = TextureLookup (uq/q, vq/q);
 *		if (kQATextureOp_Decal)
 *		{
 *			texPix.r = texPix.a * texPix.r + (1 - texPix.a) * r;
 *			texPix.g = texPix.a * texPix.g + (1 - texPix.a) * g;
 *			texPix.b = texPix.a * texPix.b + (1 - texPix.a) * b;
 *			texPix.a = a;
 *		}
 *		else
 *		{
 *			texPix.a = texPix.a * a;
 *		}
 *		if (kQATextureOp_Modulate)
 *		{
 *			texPix.r *= kd_r;		// Clamped to prevent overflow
 *			texPix.g *= kd_g;		// Clamped to prevent overflow
 *			texPix.b *= kd_b;		// Clamped to prevent overflow
 *		}
 *		if (kQATextureOp_Highlight)
 *		{
 *			texPix.r += ks_r;		// Clamped to prevent overflow
 *			texPix.g += ks_g;		// Clamped to prevent overflow
 *			texPix.b += ks_b;		// Clamped to prevent overflow
 *		}
 *
 * After computation of texPix, transparency blending (as shown
 * above for TQAVGouraud) is performed.
 */

typedef struct TQAVTexture
{
	float				x;		/* X pixel coordinate, 0.0 <= x < width */
	float				y;		/* Y pixel coordinate, 0.0 <= y < height */
	float				z;		/* Z coordinate, 0.0 <= z <= 1.0 */
	float				invW;	/* 1 / w (always required) */
	
	/* rgb are used only when kQATextureOp_Decal is set. a is always required */
	
	float				r;		/* Red, 0.0 <= r <= 1.0 */
	float				g;		/* Green, 0.0 <= g <= 1.0 */
	float				b;		/* Blue, 0.0 <= b <= 1.0 */
	float				a;		/* Alpha, 0.0 <= a <= 1.0, 1.0 is opaque */

	/* uOverW and vOverW are required by all modes */
	
	float				uOverW;	/* u / w */
	float				vOverW;	/* v / w */
	
	/* kd_r/g/b are used only when kQATextureOp_Modulate is set */
	
	float				kd_r;	/* Scale factor for texture red, 0.0 <= kd_r */
	float				kd_g;	/* Scale factor for texture green, 0.0 <= kd_g */
	float				kd_b;	/* Scale factor for texture blue, 0.0 <= kd_b */
	
	/* ks_r/g/b are used only when kQATextureOp_Highlight is set */
	
	float				ks_r;	/* Red specular highlight, 0.0 <= ks_r <= 1.0 */
	float				ks_g;	/* Green specular highlight, 0.0 <= ks_g <= 1.0 */
	float				ks_b;	/* Blue specular highlight, 0.0 <= ks_b <= 1.0 */
} TQAVTexture;

/************************************************************************************************
 *
 * Constants used for the state variables.
 *
 ***********************************************************************************************/

/*
 * kQATag_xxx is used to select a state variable when calling QASetFloat(), QASetInt(),
 * QAGetFloat() and QAGetInt(). The kQATag values are split into two separate
 * enumerated types: TQATagInt and TQATagFloat. TQATagInt is used for the QASet/GetInt()
 * functions, and TQATagFloat is used for the QASet/GetFloat() functions. (This is so
 * that a compiler that typechecks enums can flag a float/int tag mismatch during compile.)
 *
 * These variables are required by all drawing engines:
 *		kQATag_ZFunction			(Int)	One of kQAZFunction_xxx
 *		kQATag_ColorBG_a			(Float)	Background color alpha
 *		kQATag_ColorBG_r			(Float)	Background color red
 *		kQATag_ColorBG_g			(Float)	Background color green
 *		kQATag_ColorBG_b			(Float)	Background color blue
 *		kQATag_Width				(Float)	Line and point width (pixels)
 *		kQATag_ZMinOffset			(Float)	Min offset to Z to guarantee visibility (Read only!)
 *		kQATag_ZMinScale			(Float)	Min scale to Z to guarantee visibility (Read only!)
 
 * These variables are used for optional features:
 *		kQATag_Antialias			(Int)	One of kQAAntiAlias_xxx
 *		kQATag_Blend				(Int)	One of kQABlend_xxx
 *		kQATag_PerspectiveZ			(Int)	One of kQAPerspectiveZ_xxx
 *		kQATag_TextureFilter		(Int)	One of kQATextureFilter_xxx
 *		kQATag_TextureOp			(Int)	Mask of kQATextureOp_xxx
 *		kQATag_Texture				(Int)	Pointer to current TQATexture
 *		kQATag_CSGTag				(Int)	One of kQACSGTag_xxx
 *		kQATag_CSGEquation			(Int)	32 bit CSG truth table
 *
 * These variables are used for OpenGLª support:
 *		kQATagGL_DrawBuffer			(Int)	Mask of kQAGL_DrawBuffer_xxx
 *		kQATagGL_TextureWrapU		(Int)	kQAGL_Clamp or kQAGL_Repeat
 *		kQATagGL_TextureWrapV		(Int)	kQAGL_Clamp or kQAGL_Repeat
 *		kQATagGL_TextureMagFilter	(Int)	kQAGL_Nearest or kQAGL_Linear
 *		kQATagGL_TextureMinFilter	(Int)	kQAGL_Nearest, etc.
 *		kQATagGL_ScissorXMin		(Int)	Minimum X value for scissor rectangle
 *		kQATagGL_ScissorYMin		(Int)	Minimum Y value for scissor rectangle
 *		kQATagGL_ScissorXMax		(Int)	Maximum X value for scissor rectangle
 *		kQATagGL_ScissorYMax		(Int)	Maximum Y value for scissor rectangle
 *		kQATagGL_BlendSrc			(Int)	Source blending operation
 *		kQATagGL_BlendDst			(Int)	Destination blending operation
 *		kQATagGL_LinePattern		(Int)	Line rasterization pattern
 *		kQATagGL_AreaPattern0		(Int)	First of 32 area pattern registers
 *		kQATagGL_AreaPattern31		(Int)	Last of 32 area pattern registers
 *		kQATagGL_DepthBG			(Float)	Background Z
 *		kQATagGL_TextureBorder_a	(Float)	Texture border color alpha
 *		kQATagGL_TextureBorder_r	(Float)	Texture border color red
 *		kQATagGL_TextureBorder_g	(Float)	Texture border color green
 *		kQATagGL_TextureBorder_b	(Float)	Texture border color blue
 *
 *		These are for the 3DFX/Mac demo only. Re-engineer for an official RAVE release
 *
 *		kQATag_Fog						(int)	kQAFog_On, kQAFog_Off
 *		kQATag_ChromaKey				(int) kQAChromaKey_On, kQAChromaKey_Off
 *
 *		kQATag_FogColor_r				(float) fog color red, to 1.0
 *		kQATag_FogColor_g				(float) fog color green, to 1.0 
 *		kQATag_FogColor_b				(float) fog color blue, to 1.0
 *		kQATag_FogDensity				(float) fog density (units ??)
 *		kQATag_FogNearW            (float)	 
 *    kQATag_FogFarW             (float)
 *
 * Tags >= kQATag_EngineSpecific_Minimum may be assigned by the vendor for use as
 * engine-specific variables. NOTE: These should be used only in exceptional circumstances,
 * as functions performed by these variables won't be generally accessible. All other tag
 * values are reserved.
 *
 *		kQATag_EngineSpecific_Minimum	Minimum tag value for drawing-engine specific variables
 */

typedef enum TQATagInt
{
	kQATag_ZFunction				= 0,
	kQATag_Antialias				= 8,
	kQATag_Blend					= 9,
	kQATag_PerspectiveZ				= 10,
	kQATag_TextureFilter			= 11,
	kQATag_TextureOp				= 12,
	kQATag_CSGTag					= 14,
	kQATag_CSGEquation				= 15,
	kQATagGL_DrawBuffer				= 100,
	kQATagGL_TextureWrapU			= 101,
	kQATagGL_TextureWrapV			= 102,
	kQATagGL_TextureMagFilter		= 103,
	kQATagGL_TextureMinFilter		= 104,
	kQATagGL_ScissorXMin			= 105,
	kQATagGL_ScissorYMin			= 106,
	kQATagGL_ScissorXMax			= 107,
	kQATagGL_ScissorYMax			= 108,
	kQATagGL_BlendSrc				= 109,
	kQATagGL_BlendDst				= 110,
	kQATagGL_LinePattern			= 111,
	kQATagGL_AreaPattern0			= 117,
		/* ...1-30 */
	kQATagGL_AreaPattern31			= 148,
	kQATag_EngineSpecific_Minimum	= 1000,

	kQATag_Fog						= 200,
	kQATag_ChromaKey				=201
} TQATagInt;

typedef enum TQATagPtr
{
	kQATag_Texture					= 13
} TQATagPtr;

typedef enum TQATagFloat
{
	kQATag_ColorBG_a				= 1,
	kQATag_ColorBG_r				= 2,
	kQATag_ColorBG_g				= 3,
	kQATag_ColorBG_b				= 4,
	kQATag_Width					= 5,
	kQATag_ZMinOffset				= 6,
	kQATag_ZMinScale				= 7,
	kQATagGL_DepthBG				= 112,
	kQATagGL_TextureBorder_a		= 113,
	kQATagGL_TextureBorder_r		= 114,
	kQATagGL_TextureBorder_g		= 115,
	kQATagGL_TextureBorder_b		= 116,

	/*** 3dfx values ***/

	kQATag_FogColor_r					= 202,
	kQATag_FogColor_g					= 203,
	kQATag_FogColor_b					= 204,

	kQATag_FogNearW					= 205,
	kQATag_FogFarW						= 206,
	kQATag_FogDensity					= 207
} TQATagFloat;

/* kQATag_ZFunction */
#define kQAZFunction_None			0	/* Z is neither tested nor written (same as no Z buffer) */
#define kQAZFunction_LT				1	/* Znew < Zbuffer is visible */
#define kQAZFunction_EQ				2	/* Znew == Zbuffer is visible */
#define kQAZFunction_LE				3	/* Znew <= Zbuffer is visible */
#define kQAZFunction_GT				4	/* Znew > Zbuffer is visible */
#define kQAZFunction_NE				5	/* Znew != Zbuffer is visible */
#define kQAZFunction_GE				6	/* Znew >= Zbuffer is visible */
#define kQAZFunction_True			7	/* Znew is always visible */

/* kQATag_Width */
#define kQAMaxWidth					128.0

/* kQATag_Antialias */
#define kQAAntiAlias_Off			0
#define kQAAntiAlias_Fast			1
#define kQAAntiAlias_Mid			2
#define kQAAntiAlias_Best			3

/* kQATag_Blend */
#define kQABlend_PreMultiply		0
#define kQABlend_Interpolate		1
#define kQABlend_OpenGL				2

/* kQATag_PerspectiveZ */
#define kQAPerspectiveZ_Off			0	/* Use Z for hidden surface removal */
#define kQAPerspectiveZ_On			1	/* Use InvW for hidden surface removal */

/* kQATag_ChromaKey */

#define kQAChromaKey_Off			0
#define kQAChromaKey_On				1

/* kQATag_Fog */

#define kQAFog_Off					0
#define kQAFog_On    				1


/* kQATag_TextureFilter */
#define kQATextureFilter_Fast		0
#define kQATextureFilter_Mid		1
#define kQATextureFilter_Best		2

/* kQATag_TextureOp (mask of one or more) */
#define kQATextureOp_None				0			/* Default texture mapping mode */
#define kQATextureOp_Modulate		(1 << 0)		/* Modulate texture color with kd_r/g/b */
#define kQATextureOp_Highlight	(1 << 1)		/* Add highlight value ks_r/g/b */
#define kQATextureOp_Decal			(1 << 2)		/* When texture alpha == 0, use rgb instead */
#define kQATextureOp_Shrink		(1 << 3)		/* This is a non-wrapping texture, so the ??? */
#define kQATextureOp_Composite	(1 << 4)		/* indicates this is a composite texture */

/* kQATag_CSGTag */
#define kQACSGTag_None			0xffffffffUL	/* Do not perform CSG */
#define kQACSGTag_0				0				/* Submitted tris have CSG ID 0 */
#define kQACSGTag_1				1				/* Submitted tris have CSG ID 1 */
#define kQACSGTag_2				2				/* Submitted tris have CSG ID 2 */
#define kQACSGTag_3				3				/* Submitted tris have CSG ID 3 */
#define kQACSGTag_4				4				/* Submitted tris have CSG ID 4 */

/* kQATagGL_TextureWrapU/V */
#define kQAGL_Repeat				0
#define kQAGL_Clamp					1

/* kQATagGL_BlendSrc */
#define kQAGL_SourceBlend_XXX		0

/* kQATagGL_BlendDst */
#define kQAGL_DestBlend_XXX			0

/* kQATagGL_DrawBuffer (mask of one or more) */
#define kQAGL_DrawBuffer_None		0
#define kQAGL_DrawBuffer_FrontLeft	(1<<0)
#define kQAGL_DrawBuffer_FrontRight	(1<<1)
#define kQAGL_DrawBuffer_BackLeft	(1<<2)
#define kQAGL_DrawBuffer_BackRight	(1<<3)
#define kQAGL_DrawBuffer_Front		(kQAGL_DrawBuffer_FrontLeft | kQAGL_DrawBuffer_FrontRight)
#define kQAGL_DrawBuffer_Back		(kQAGL_DrawBuffer_BackLeft | kQAGL_DrawBuffer_BackRight)

/************************************************************************************************
 *
 * Constants used as function parameters.
 *
 ***********************************************************************************************/

/*
 * TQAVertexMode is a parameter to QADrawVGouraud() and QADrawVTexture() that specifies how
 * to interpret and draw the vertex array.
 */

typedef enum TQAVertexMode
{
	kQAVertexMode_Point				= 0,		/* Draw nVertices points */
	kQAVertexMode_Line				= 1,		/* Draw nVertices/2 line segments */
	kQAVertexMode_Polyline			= 2,		/* Draw nVertices-1 connected line segments */
	kQAVertexMode_Tri				= 3,		/* Draw nVertices/3 triangles */
	kQAVertexMode_Strip				= 4,		/* Draw nVertices-2 triangles as a strip */
	kQAVertexMode_Fan				= 5			/* Draw nVertices-2 triangles as a fan from v0 */
} TQAVertexMode;

/*
 * TQAGestaltSelector is a parameter to QAEngineGestalt(). It selects which gestalt
 * parameter will be copied into 'response'.
 */

typedef enum TQAGestaltSelector
{
	kQAGestalt_OptionalFeatures		= 0,		/* Mask of one or more kQAOptional_xxx */
	kQAGestalt_FastFeatures			= 1,		/* Mask of one or more kQAFast_xxx */
	kQAGestalt_VendorID				= 2,		/* Vendor ID */
	kQAGestalt_EngineID				= 3,		/* Engine ID */
	kQAGestalt_Revision				= 4,		/* Revision number of this engine */
	kQAGestalt_ASCIINameLength		= 5,		/* strlen (asciiName) */
	kQAGestalt_ASCIIName			= 6			/* Causes strcpy (response, asciiName) */
} TQAGestaltSelector;

#ifdef RAVE_1_1

/*
 * TQAMethodSelector is a parameter to QASetNoticeMethod to select the notice method
 */

typedef enum TQAMethodSelector
{
	kQAMethod_RenderCompletion		= 0
	
} TQAMethodSelector;

#endif

/*
 * kQATriFlags_xxx are ORed together to generate the 'flags' parameter
 * to QADrawTriGouraud() and QADrawTriTexture().
 */

#define	kQATriFlags_None			0			/* No flags (triangle is front-facing or don't care) */
#define kQATriFlags_Backfacing		(1 << 0)	/* Triangle is back-facing */

/*
 * kQATexture_xxx are ORed together to generate the 'flags' parameter to QATextureNew().
 */

#define	kQATexture_None				0			/* No flags */
#define kQATexture_Lock				(1<<0)		/* Don't swap this texture out */
#define kQATexture_Mipmap			(1<<1)		/* This texture is mipmapped */
#define kQATexture_NoCompression	(1<<2)		/* Do not compress this texture */
#define kQATexture_HighCompression	(1<<3)		/* Compress texture, even if it takes a while */

/*
 * kQABitmap_xxx are ORed together to generate the 'flags' parameter to QABitmapNew().
 */

#define	kQABitmap_None				0			/* No flags */
#define kQABitmap_Lock				(1<<1)		/* Don't swap this bitmap out */
#define kQABitmap_NoCompression		(1<<2)		/* Do not compress this bitmap */
#define kQABitmap_HighCompression	(1<<3)		/* Compress bitmap, even if it takes a while */

/*
 * kQAContext_xxx are ORed together to generate the 'flags' parameter for QADrawContextNew().
 */

#define kQAContext_None				0			/* No flags */
#define kQAContext_NoZBuffer		(1 << 0)	/* No hidden surface removal */
#define kQAContext_DeepZ			(1 << 1)	/* Hidden surface precision >= 24 bits */
#define kQAContext_DoubleBuffer		(1 << 2)	/* Double buffered window */
#define kQAContext_Cache			(1 << 3)	/* This is a cache context */

/*
 * kQAOptional_xxx are ORed together to generate the kQAGestalt_OptionalFeatures response
 * from QAEngineGestalt().
 */

#define kQAOptional_None			0			/* No optional features */
#define kQAOptional_DeepZ			(1 << 0)	/* Hidden surface precision >= 24 bits */
#define kQAOptional_Texture			(1 << 1)	/* Texture mapping */
#define kQAOptional_TextureHQ		(1 << 2)	/* High quality texture (tri-linear mip or better) */
#define kQAOptional_TextureColor	(1 << 3)	/* Full color modulation and highlight of textures */
#define kQAOptional_Blend			(1 << 4)	/* Transparency blending of RGB */
#define kQAOptional_BlendAlpha		(1 << 5)	/* Transparency blending includes alpha channel */
#define kQAOptional_Antialias		(1 << 6)	/* Antialiased rendering */
#define kQAOptional_ZSorted			(1 << 7)	/* Z sorted rendering (for transparency, etc.) */
#define kQAOptional_PerspectiveZ	(1 << 8)	/* Hidden surface removal using InvW instead of Z */
#define kQAOptional_OpenGL			(1 << 9)	/* Extended rasterization features for OpenGLª */
#define kQAOptional_NoClear			(1 << 10)	/* This drawing engine doesn't clear before drawing */
#define kQAOptional_CSG				(1 << 11)	/* kQATag_CSGxxx are implemented */
#define kQAOptional_BoundToDevice	(1 << 12)	/* This engine is tightly bound to GDevice */
#define kQAOptional_CL4				(1 << 13)	/* This engine suports kQAPixel_CL4 */
#define kQAOptional_CL8				(1 << 14)	/* This engine suports kQAPixel_CL8 */

/*
 * kQAFast_xxx are ORed together to generate the kQAGestalt_FastFeatures response
 * from QAEngineGestalt().
 */

#define kQAFast_None				0			/* No accelerated features */
#define kQAFast_Line				(1 << 0)	/* Line drawing */
#define kQAFast_Gouraud				(1 << 1)	/* Gouraud shaded triangles */
#define kQAFast_Texture				(1 << 2)	/* Texture mapped triangles */
#define kQAFast_TextureHQ			(1 << 3)	/* High quality texture (tri-linear mip or better) */
#define kQAFast_Blend				(1 << 4)	/* Transparency blending */
#define kQAFast_Antialiasing		(1 << 5)	/* Antialiased rendering */
#define kQAFast_ZSorted				(1 << 6)	/* Z sorted rendering of non-opaque objects */
#define kQAFast_CL4					(1 << 7)	/* This engine accelerates kQAPixel_CL4 */
#define kQAFast_CL8					(1 << 8)	/* This engine accelerates kQAPixel_CL8 */

/************************************************************************************************
 *
 * Macro definitions for the drawing engine methods included in TQADrawContext. These
 * macros are the recommended means of accessing the engine's draw methods, e.g:
 *
 *		TQADrawContext	*drawContext;
 *		TQAVTexture		vertices[3];
 *
 *		drawContext = QADrawContextNew (rect, gdevice, engine, kQAContext_ZBuffer);
 *		...
 *		QASetInt (drawContext, kQATag_ZFunction, kQAZFunction_LT);
 *		QADrawTriGouraud (drawContext, &vertices[0], &vertices[1], &vertices[2], kQATriFlags_None);
 *
 * Note that QARenderStart(), QARenderEnd(), QAFlush() and QASync() have real function
 * definitions instead of macros. This is because these functions can afford the extra
 * per-call overhead of a function layer (which makes application code a little smaller),
 * and to allow a cleaner implementation of handling NULL parameters to QARenderStart().
 *
 ***********************************************************************************************/

#define QASetFloat(drawContext,tag,newValue) \
		(drawContext)->setFloat (drawContext,tag,newValue)

#define QASetInt(drawContext,tag,newValue) \
		(drawContext)->setInt (drawContext,tag,newValue)

#define QASetPtr(drawContext,tag,newValue) \
		(drawContext)->setPtr (drawContext,tag,newValue)

#define QAGetFloat(drawContext,tag) \
		(drawContext)->getFloat (drawContext,tag)

#define QAGetInt(drawContext,tag) \
		(drawContext)->getInt (drawContext,tag)

#define QAGetPtr(drawContext,tag) \
		(drawContext)->getPtr (drawContext,tag)

#define QADrawPoint(drawContext,v) \
		(drawContext)->drawPoint (drawContext,v)

#define QADrawLine(drawContext,v0,v1) \
		(drawContext)->drawLine (drawContext,v0,v1)

#define QADrawTriGouraud(drawContext,v0,v1,v2,flags) \
		(drawContext)->drawTriGouraud (drawContext,v0,v1,v2,flags)

#define QADrawTriTexture(drawContext,v0,v1,v2,flags) \
		(drawContext)->drawTriTexture (drawContext,v0,v1,v2,flags)

#define QASubmitVerticesGouraud(drawContext,nVertices,vertices) \
		(drawContext)->submitVerticesGouraud(drawContext,nVertices,vertices)
		
#define QASubmitVerticesTexture(drawContext,nVertices,vertices) \
		(drawContext)->submitVerticesTexture(drawContext,nVertices,vertices)
		
#define QADrawTriMeshGouraud(drawContext,nTriangle,triangles) \
		(drawContext)->drawTriMeshGouraud (drawContext,nTriangle,triangles)

#define QADrawTriMeshTexture(drawContext,nTriangle,triangles) \
		(drawContext)->drawTriMeshTexture (drawContext,nTriangle,triangles)

#define QADrawVGouraud(drawContext,nVertices,vertexMode,vertices,flags) \
		(drawContext)->drawVGouraud (drawContext,nVertices,vertexMode,vertices,flags)

#define QADrawVTexture(drawContext,nVertices,vertexMode,vertices,flags) \
		(drawContext)->drawVTexture (drawContext,nVertices,vertexMode,vertices,flags)

#define QADrawBitmap(drawContext,v,bitmap) \
		(drawContext)->drawBitmap (drawContext,v,bitmap)

#define QARenderStart(drawContext,dirtyRect,initialContext) \
		(drawContext)->renderStart (drawContext,dirtyRect,initialContext)

#define QARenderEnd(drawContext,modifiedRect) \
		(drawContext)->renderEnd (drawContext,modifiedRect)

#define QARenderAbort(drawContext) \
		(drawContext)->renderAbort (drawContext)

#define QAFlush(drawContext) \
		(drawContext)->flush (drawContext)

#define QASync(drawContext) \
		(drawContext)->sync (drawContext)

#ifdef RAVE_1_1

#define QASetNoticeMethod(drawContext, method, completionCallBack, refCon) \
		(drawContext)->setNoticeMethod (drawContext, method, completionCallBack, refCon)

#define QAGetNoticeMethod(drawContext, method, completionCallBack, refCon) \
		(drawContext)->getNoticeMethod (drawContext, method, completionCallBack, refCon)

#endif

/************************************************************************************************
 *
 * Typedefs of draw method functions provided by the drawing engine. One function pointer
 * for each of these function types in stored in the TQADrawContext public data structure.
 *
 * These functions should be accessed through the QA<function>(context,...) macros,
 * defined above.
 *
 ***********************************************************************************************/

#ifdef RAVE_1_1

typedef void (*TQANoticeMethod)(
	TQADrawContext			*drawContext,		/* Draw context */
	void					*refCon);

#endif

typedef void (*TQASetFloat) (
	TQADrawContext			*drawContext,		/* Draw context */
	TQATagFloat				tag,				/* Tag of variable to set */
	float					newValue);			/* New value for variable */

typedef void (*TQASetInt) (
	TQADrawContext			*drawContext,		/* Draw context */
	TQATagInt				tag,				/* Tag of variable to set */
	unsigned long			newValue);			/* New value for variable */

typedef void (*TQASetPtr) (
	TQADrawContext			*drawContext,		/* Draw context */
	TQATagPtr				tag,				/* Tag of variable to set */
	const void				*newValue);			/* New value for variable */

typedef float (*TQAGetFloat) (
	const TQADrawContext	*drawContext,		/* Draw context */
	TQATagFloat				tag);				/* Tag of variable to get */

typedef unsigned long (*TQAGetInt) (
	const TQADrawContext	*drawContext,		/* Draw context */
	TQATagInt				tag);				/* Tag of variable to get */

typedef void *(*TQAGetPtr) (
	const TQADrawContext	*drawContext,		/* Draw context */
	TQATagPtr				tag);				/* Tag of variable to get */

typedef void (*TQADrawPoint) (
	const TQADrawContext	*drawContext,		/* Draw context */
	const TQAVGouraud		*v);				/* Vertex */

typedef void (*TQADrawLine) (
	const TQADrawContext	*drawContext,		/* Draw context */
	const TQAVGouraud 		*v0,				/* Vertex 0 */
	const TQAVGouraud 		*v1);				/* Vertex 1 */

typedef void (*TQADrawTriGouraud) (
	const TQADrawContext	*drawContext,		/* Draw context */
	const TQAVGouraud 		*v0,				/* Vertex 0 */
	const TQAVGouraud 		*v1,				/* Vertex 1 */
	const TQAVGouraud 		*v2,				/* Vertex 2 */
	unsigned long			flags);				/* Mask of kQATriFlags_xxx flags */

typedef void (*TQADrawTriTexture) (
	const TQADrawContext	*drawContext,		/* Draw context */
	const TQAVTexture 		*v0,				/* Vertex 0 */
	const TQAVTexture 		*v1,				/* Vertex 1 */
	const TQAVTexture 		*v2,				/* Vertex 2 */
	unsigned long			flags);				/* Mask of kQATriFlags_xxx flags */

typedef void (*TQASubmitVerticesGouraud) (
	const TQADrawContext		*drawContext,	/* Draw context */
	unsigned long	 			nVertices,		/* Number of vertices */
	const TQAVGouraud		 	*vertices);		/* Array of vertices */
	
typedef void (*TQASubmitVerticesTexture) (
	const TQADrawContext		*drawContext,	/* Draw context */
	unsigned long	 			nVertices,		/* Number of vertices */
	const TQAVTexture			*vertices);		/* Array of vertices */
	
typedef void (*TQADrawTriMeshGouraud) (
	const TQADrawContext		*drawContext,	/* Draw context */
	unsigned long	 			nTriangles,		/* Number of triangles */
	const TQAIndexedTriangle	*triangles);	/* Array of triangles */
	
typedef void (*TQADrawTriMeshTexture) (
	const TQADrawContext		*drawContext,	/* Draw context */
	unsigned long	 			nTriangles,		/* Number of triangles */
	const TQAIndexedTriangle 	*triangles);	/* Array of triangles */

typedef void (*TQADrawVGouraud) (
	const TQADrawContext	*drawContext,		/* Draw context */
	unsigned long			nVertices,			/* Number of vertices */
	TQAVertexMode			vertexMode,			/* One of kQAVertexMode_xxx enumerated values */
	const TQAVGouraud 		vertices[],			/* Array of vertices */
	const unsigned long		flags[]);			/* Array of per-triangle flags (or NULL) */

typedef void (*TQADrawVTexture) (
	const TQADrawContext	*drawContext,		/* Draw context */
	unsigned long			nVertices,			/* Number of vertices */
	TQAVertexMode			vertexMode,			/* One of kQAVertexMode_xxx enumerated values */
	const TQAVTexture 		vertices[],			/* Array of vertices */
	const unsigned long		flags[]);			/* Array of per-triangle flags (or NULL) */

typedef void (*TQADrawBitmap) (
	const TQADrawContext	*drawContext,		/* Draw context */
	const TQAVGouraud 		*v,					/* xyz, and (if a 1 bit/pixel bitmap) argb */
	TQABitmap				*bitmap);			/* Previously allocated by QABitmapNew() */

typedef void (*TQARenderStart) (
	const TQADrawContext	*drawContext,		/* Draw context */
	const TQARect			*dirtyRect,			/* Minimum area to clear; NULL means whole buffer */
	const TQADrawContext	*initialContext);	/* Initial background image (or NULL) */

typedef TQAError (*TQARenderEnd) (
	const TQADrawContext	*drawContext,		/* Draw context */
	const TQARect			*modifiedRect);		/* Minimum area to swap; NULL means whole buffer */

typedef TQAError (*TQARenderAbort) (
	const TQADrawContext	*drawContext);		/* Draw context */

typedef TQAError (*TQAFlush) (
	const TQADrawContext	*drawContext);		/* Draw context */

typedef TQAError (*TQASync) (
	const TQADrawContext	*drawContext);		/* Draw context */

#ifdef RAVE_1_1

typedef TQAError (*TQASetNoticeMethod)(
	const TQADrawContext	*drawContext,	/* Draw context */
	TQAMethodSelector		method,
	TQANoticeMethod			completionCallBack,
	void					*refCon);

typedef TQAError (*TQAGetNoticeMethod)(
	const TQADrawContext	*drawContext,	/* Draw context */
	TQAMethodSelector		method,
	TQANoticeMethod			*completionCallBack,
	void					**refCon);

#endif

/************************************************************************************************
 *
 * Public TQADrawContext structure. This contains function pointers for the chosen
 * drawing engine.
 *
 ***********************************************************************************************/

/*
 * TQAVersion sets the TQADrawContext 'version' field. It is set by
 * the manager to indicate the version of the TQADrawContext structure.
 */

typedef enum TQAVersion
{
	kQAVersion_Prerelease		= 0,
	kQAVersion_1_0				= 1,
	kQAVersion_1_1				= 2		/* Added tri mesh functions, color tables */
} TQAVersion;

struct TQADrawContext
{
	TQADrawPrivate			*drawPrivate;		/* Engine's private data for this context */
	const TQAVersion		version;			/* Version number */
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
};

/************************************************************************************************
 *
 * Acceleration manager function prototypes.
 *
 ***********************************************************************************************/

RAVE_EXPORT TQAError QADrawContextNew (
	const TQADevice	*device,				/* Target device */
	const TQARect	*rect,					/* Target rectangle (device coordinates) */
	const TQAClip	*clip,					/* 2D clip region */
	const TQAEngine	*engine,				/* Drawing engine to use */
	unsigned long	flags,					/* Mask of kQAContext_xxx */
	TQADrawContext	**newDrawContext);		/* (Out) Newly created TQADrawContext */

RAVE_EXPORT void QADrawContextDelete (
	TQADrawContext	*drawContext);			/* Context to delete */

RAVE_EXPORT TQAError QAColorTableNew(
	const TQAEngine		*engine,			/* Drawing engine to use */
	TQAColorTableType	tableType,			/* Depth, color space, etc. */
	void				*pixelData,			/* lookup table entries in pixelType format */
	long				transparentIndexFlag,	/* boolean, false means no transparency, true means index 0 is transparent */
	TQAColorTable		**newTable);		/* (Out) Newly created TQAColorTable */

RAVE_EXPORT void QAColorTableDelete(
	const TQAEngine		*engine,			/* Drawing engine to use */
	TQAColorTable		*colorTable);		/* Previously allocated by QAColorTableNew() */

RAVE_EXPORT TQAError QATextureNew (
	const TQAEngine		*engine,			/* Drawing engine to use */
	unsigned long		flags,				/* Mask of kQATexture_xxx flags */
	TQAImagePixelType	pixelType,			/* Depth, color space, etc. */
	const TQAImage		images[],			/* Image(s) for texture */
	TQATexture			**newTexture);		/* (Out) Newly created TQATexture, or NULL on error */ 

RAVE_EXPORT TQAError QATextureDetach (
	const TQAEngine		*engine,			/* Drawing engine to use */
	TQATexture			*texture);			/* Previously allocated by QATextureNew() */

RAVE_EXPORT void QATextureDelete (
	const TQAEngine		*engine,			/* Drawing engine to use */
	TQATexture			*texture);			/* Previously allocated by QATextureNew() */

RAVE_EXPORT TQAError QATextureBindColorTable(
	const TQAEngine		*engine,			/* Drawing engine to use */
	TQATexture			*texture,			/* Previously allocated by QATextureNew() */
	TQAColorTable		*colorTable);		/* Previously allocated by QAColorTableNew() */
	
RAVE_EXPORT TQAError QABitmapNew (
	const TQAEngine		*engine,			/* Drawing engine to use */
	unsigned long		flags,				/* Mask of kQABitmap_xxx flags */
	TQAImagePixelType	pixelType,			/* Depth, color space, etc. */
	const TQAImage		*image,				/* Image */
	TQABitmap			**newBitmap);		/* (Out) Newly created TQABitmap, or NULL on error */ 

RAVE_EXPORT TQAError QABitmapDetach (
	const TQAEngine		*engine,			/* Drawing engine to use */
	TQABitmap			*bitmap);			/* Previously allocated by QABitmapNew() */

RAVE_EXPORT void QABitmapDelete (
	const TQAEngine		*engine,			/* Drawing engine to use */
	TQABitmap			*bitmap);			/* Previously allocated by QABitmapNew() */

RAVE_EXPORT TQAError QABitmapBindColorTable(
	const TQAEngine		*engine,			/* Drawing engine to use */
	TQABitmap			*bitmap,			/* Previously allocated by QABitmapNew() */
	TQAColorTable		*colorTable);		/* Previously allocated by QAColorTableNew() */
	
RAVE_EXPORT TQAEngine *QADeviceGetFirstEngine (
	const TQADevice	*device);				/* Target device */

RAVE_EXPORT TQAEngine *QADeviceGetNextEngine (
	const TQADevice	*device,				/* Target device */
	const TQAEngine	*currentEngine);		/* Engine after 'currentEngine' is returned */

RAVE_EXPORT TQAError QAEngineCheckDevice (
	const TQAEngine	*engine,				/* Engine to check on 'device' */
	const TQADevice	*device);				/* Target device */

RAVE_EXPORT TQAError QAEngineGestalt (
	const TQAEngine		*engine,			/* Engine being queried */
	TQAGestaltSelector	selector,			/* Gestalt parameter being requested */
	void				*response);			/* Buffer that receives response */

RAVE_EXPORT TQAError QAEngineEnable (
	long			vendorID,				/* Vendor ID of engine to enable */
	long			engineID);				/* Engine ID of engine to enable */

RAVE_EXPORT TQAError QAEngineDisable (
	long			vendorID,				/* Vendor ID of engine to disable */
	long			engineID);				/* Engine ID of engine to disable */

#ifdef __cplusplus
}
#endif

#endif /* _RAVE_h */
