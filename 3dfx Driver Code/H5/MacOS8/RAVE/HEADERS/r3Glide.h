/*________________________________________________________________________________________
** 
** Copyright (c) 1999, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
**________________________________________________________________________________________
**
**
** r3Glide.h
** Purpose: provide Glide 3 extensions to RAVE driver.
**
*/

#ifndef _RAVE_GLIDE_H_
#define _RAVE_GLIDE_H_


#include <3dfx.h>
#include <glide.h>
#include <g3ext.h>
#include <gsfc.h>
#include <glidesys.h>
#include <RAVE.h>
#include <RaveSystem.h>

// prototypes
UInt32 		ValidateGlideEnvironment(void);
UInt32   	InitializeWindowedGlideRenderingSurface( TQADrawContext * inContext, long inWidth, long inHeight, long inRenderDepth);
UInt32		SetupGlideSurfaceBlit(TQADrawPrivate * inContext, const TQARect*   rect);
UInt32		InitializeGlide(void);
void		RvSurfaceNotify(GrSurface_t sfc, void *userData, unsigned long code);

/*_____ Glide internal definitions _____*/

	/* Glide Extensions Functions Pointer Definitions */
	
typedef GrContext_t (*grSurfaceCreateContextFuncPtr)( GrSurfaceContextType_t type);
typedef void (*grSurfaceReleaseContextFuncPtr)( GrContext_t ctx);
typedef void (*grSurfaceSetRenderingSurfaceFuncPtr)( GrSurface_t sfc);
typedef FxBool (*grSurfaceCalcTextureWHDFuncPtr)( GrTexInfo *info, FxU32 *w, FxU32 *h, FxU32 *d);
typedef void (*grSurfaceSetAuxSurfaceFuncPtr)( GrSurface_t sfc);
typedef void (*grSurfaceSetTextureSurfaceFuncPtr)( GrChipID_t tmu, GrSurface_t sfc);
typedef FxU32 (*grDeviceQueryFuncPtr)(GrDeviceInfo_t devList[], FxU32 listCount);
typedef GrContext_t (*grSstWinOpenFuncPtr)( FxU32 hWnd, GrScreenResolution_t resolution, GrScreenRefresh_t refresh, GrColorFormat_t format, GrOriginLocation_t origin, GrPixelFormat_t pixelformat, int nColBuffers, int nAuxBuffers);
typedef void (*grColorMaskFuncPtr)( FxBool r, FxBool g, FxBool b, FxBool a );
typedef void (*grStencilFuncPtr)(GrCmpFnc_t fnc, GrStencil_t ref, GrStencil_t mask);
typedef void (*grStencilMaskFuncPtr)(GrStencil_t write_mask);
typedef void (*grStencilOpFuncPtr)( GrStencilOp_t stencil_fail, GrStencilOp_t depth_fail, GrStencilOp_t depth_pass);
typedef void (*grBufferClearFuncPtr)( GrColor_t color, GrAlpha_t alpha, FxU32 depth, GrStencil_t stencil);
typedef void (*grLfbConstantStencilFuncPtr)( GrStencil_t mode);
typedef void (*grTBufferWriteMaskFuncPtr)( FxU32 mask );
typedef void (*grColorCombineFuncPtr)( GrCCUColor_t a, GrCombineMode_t a_mode, GrCCUColor_t b, GrCombineMode_t b_mode, GrCCUColor_t c, FxBool c_invert, GrCCUColor_t d, FxBool d_invert, FxU32 shift, FxBool invert);
typedef void (*grAlphaCombineFuncPtr)( GrACUColor_t a, GrCombineMode_t a_mode, GrACUColor_t b, GrCombineMode_t b_mode, GrACUColor_t c, FxBool c_invert, GrACUColor_t d, FxBool d_invert, FxU32 shift, FxBool invert);
typedef void (*grTexColorCombineFuncPtr)( GrChipID_t tmu, GrTCCUColor_t a, GrCombineMode_t a_mode, GrTCCUColor_t b, GrCombineMode_t b_mode, GrTCCUColor_t c, FxBool c_invert, GrTCCUColor_t d, FxBool d_invert, FxU32 shift, FxBool invert);
typedef void (*grTexAlphaCombineFuncPtr)( GrChipID_t tmu, GrTACUColor_t a, GrCombineMode_t a_mode, GrTACUColor_t b, GrCombineMode_t b_mode, GrTACUColor_t c, FxBool c_invert, GrTACUColor_t d, FxBool d_invert, FxU32 shift, FxBool invert);

typedef GrSurface_t (*grSurfaceCreateFuncPtr)( GrSurfaceDesc_t *desc);
typedef void (*grSurfaceReleaseFuncPtr)( GrSurface_t sfc);
typedef void (*grSurfaceGetDescFuncPtr)( GrSurface_t sfc, GrSurfaceDesc_t * desc);
							
typedef struct cmdTransportInfo* (*grCommandTransportInfoPtr)(void);
typedef void (*grCommandTransportMakeRoomPtr)(const FxI32 blockSize, const char* fName, const int fLine);



/*_____ Glide internal variables _____*/

	/* Glide Extensions */
	
extern grSurfaceCreateContextFuncPtr grSurfaceCreateContextExt;
extern grSurfaceReleaseContextFuncPtr grSurfaceReleaseContextExt;
extern grSurfaceSetRenderingSurfaceFuncPtr grSurfaceSetRenderingSurfaceExt;
extern grSurfaceCalcTextureWHDFuncPtr grSurfaceCalcTextureWHDExt;
extern grSurfaceSetAuxSurfaceFuncPtr grSurfaceSetAuxSurfaceExt;
extern grSurfaceSetTextureSurfaceFuncPtr grSurfaceSetTextureSurfaceExt;
extern grDeviceQueryFuncPtr grDeviceQueryExt;

extern grSstWinOpenFuncPtr grSstWinOpenExt;
extern grColorMaskFuncPtr grColorMaskExt;
extern grStencilFuncPtr grStencilFuncExt;
extern grStencilMaskFuncPtr grStencilMaskExt;
extern grStencilOpFuncPtr grStencilOpExt;
extern grBufferClearFuncPtr grBufferClearExt;
extern grLfbConstantStencilFuncPtr grLfbConstantStencilExt;
extern grTBufferWriteMaskFuncPtr grTBufferWriteMaskExt;
extern grColorCombineFuncPtr grColorCombineExt;
extern grAlphaCombineFuncPtr grAlphaCombineExt;
extern grTexColorCombineFuncPtr grTexColorCombineExt;
extern grTexAlphaCombineFuncPtr grTexAlphaCombineExt;

extern grCommandTransportInfoPtr grCommandTransportInfoExt;
extern grCommandTransportMakeRoomPtr grCommandTransportMakeRoomExt;

	/* MacOS specific Glide Extensions */
	
extern grSurfaceCreateFuncPtr grSurfaceCreateExt;
extern grSurfaceReleaseFuncPtr grSurfaceReleaseExt;
extern grSurfaceGetDescFuncPtr grSurfaceGetDescExt;

	/* global variables */
#define MAX_GLIDE_DEVICES 16
extern GrDeviceInfo_t rvDeviceList[MAX_GLIDE_DEVICES];
extern UInt32 tnslDeviceListCount;

extern Boolean tnsl_GlideInitialized;
extern Boolean tnsl_surface_ext;

extern GLboolean 	gls_2k_texture_size;
extern Boolean		tnsl_32bit_texture;
#define NAPALM_AND_BEYOND	tnsl_32bit_texture

TQAError InitializeFullscreenGlide( TQADrawContext* );

/* temp RES needed for Rave?
extern GLboolean gls_pix_ext;
extern GLboolean gls_combine_ext;
extern GLboolean gls_32bit_texture;
extern GLboolean gls_uma_texture;
extern GLboolean gls_CMDTransport;
*/

/*_____ Glide Operation Internal Functions _____*/
/*
extern GLenum   InitializeGlide( void );
extern long     ValidateGlideEnvironment( void );
extern GLenum   InitializeWindowedGlideRenderingSurface( GLDContext inContext, GLint inWidth, GLint inHeight, GLint inRowbytes);
extern GLenum   InitializeFullscreenGlide( GLDContext inContext, const GLIFullScreen * inDrawable, const CGrafPtr inDrawableWindow);

extern void     RvSurfaceNotify(GrSurface_t sfc, void *userData, unsigned long code);
*/

#endif /* _RAVE_GLIDE_H_ */
