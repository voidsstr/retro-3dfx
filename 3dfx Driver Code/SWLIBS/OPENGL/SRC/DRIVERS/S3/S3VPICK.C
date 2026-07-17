/*
** Copyright 1991-1997, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
*/
#include <stdio.h>
#include "s3vcontext.h"
#include "ddtexmgr.h"



void __glS3VPickTextureProcs(__GLcontext *gc)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLS3Vcontext *hwcx = (__GLS3Vcontext *) gc;
    GLuint hwTexFunc = 0, hwBlendFunc = 0;

    __glGenericPickTextureProcs(gc); 

    /* now pick the hardware register values */
    if (modeFlags & __GL_SHADE_TEXTURE) {
	__GLtexture *current = gc->texture.currentTexture;
	__GLDDrawMipMapLevel *lp;

	if (gc->texture.currentTexture) {
	    lp = (__GLDDrawMipMapLevel *) current->level[0];

	    hwcx->hwTexStride = lp->level.width * lp->bytesPerTexel;

	    hwTexFunc |= (lp->level.widthLog2 << 8);

	    /* Apparently, only wrap mode REPEAT is supported */
	    /* Borders are also not supported */
	    hwTexFunc |= S3D_TEXTURE_WRAP_ENABLE;

	    switch(current->params.minFilter) {
	      case GL_NEAREST:
		  hwTexFunc |= S3D_TEXTURE_FILTER_NEAREST;
		  break;
	      case GL_LINEAR:
		  hwTexFunc |= S3D_TEXTURE_FILTER_LINEAR;
		  break;
	      case GL_NEAREST_MIPMAP_NEAREST:
		  hwTexFunc |= S3D_TEXTURE_FILTER_MIP_NEAREST;
		  break ;
	      case GL_LINEAR_MIPMAP_NEAREST:
		  hwTexFunc |= S3D_TEXTURE_FILTER_MIP_LINEAR;
		  break ;
	      case GL_NEAREST_MIPMAP_LINEAR:
		  hwTexFunc |= S3D_TEXTURE_FILTER_LINEAR_MIP_NEAREST;
		  break ;
	      case GL_LINEAR_MIPMAP_LINEAR:
		  hwTexFunc |= S3D_TEXTURE_FILTER_LINEAR_MIP_LINEAR;
		  break ;
	      default:
		  break;
            }

            switch(gc->state.texture.env[0].mode) {
	      case GL_MODULATE:
		  switch(lp->level.internalFormat) {
		    case __GL_FORMAT_ALPHA8:
			OutputDebugStringA("HWTexEnvState: GL_MODULATE + GL_ALPHA unsupported\n");
			break;
		    case __GL_FORMAT_COLOR_INDEX8:
		    case __GL_FORMAT_COLOR_INDEX16:
		    case __GL_FORMAT_INTENSITY8:
		    case __GL_FORMAT_RGB8:
		    case __GL_FORMAT_RGB332:
		    case __GL_FORMAT_XRGB1555:
		    case __GL_FORMAT_RGB565:
			hwTexFunc |= S3D_TEXTURE_BLEND_MODULATE;
			hwTexFunc |= (modeFlags & __GL_SHADE_TEXTURE_PERSP)?
			    S3D_PERSPECTIVE_LIT_TEXTURED_TRIANGLE:
			    S3D_LIT_TEXTURED_TRIANGLE;
			break;
		    case __GL_FORMAT_LUMINANCE8:
		    case __GL_FORMAT_LUMINANCE_ALPHA8:
		    case __GL_FORMAT_RGBA8:
		    case __GL_FORMAT_RGBA4:
		    case __GL_FORMAT_ARGB4:
		    case __GL_FORMAT_RGBA5551:
		    case __GL_FORMAT_ARGB1555:
			hwTexFunc |= S3D_TEXTURE_BLEND_MODULATE;
			hwTexFunc |= (modeFlags & __GL_SHADE_TEXTURE_PERSP)?
			    S3D_PERSPECTIVE_LIT_TEXTURED_TRIANGLE:
			    S3D_LIT_TEXTURED_TRIANGLE;
			hwBlendFunc = S3D_ALPHA_BLEND_TEXTURE_ALPHA;
			break;
		    default:
			OutputDebugStringA("HWTexEnvState: Unrecognizable internal format\n");
			break;
		  }
		  break;
    
	      case GL_DECAL:
		  // No need to generate colors, only texture
		  switch(lp->level.internalFormat) {
		    case __GL_FORMAT_ALPHA8:
		    case __GL_FORMAT_LUMINANCE8:
		    case __GL_FORMAT_LUMINANCE_ALPHA8:
		    case __GL_FORMAT_INTENSITY8:
			break;
		    case __GL_FORMAT_RGBA8:
		    case __GL_FORMAT_RGBA4:
		    case __GL_FORMAT_ARGB4:
		    case __GL_FORMAT_RGBA5551:
		    case __GL_FORMAT_ARGB1555:
			hwTexFunc |= S3D_TEXTURE_BLEND_DECAL;
			hwTexFunc |= (modeFlags & __GL_SHADE_TEXTURE_PERSP)?
			    S3D_PERSPECTIVE_UNLIT_TEXTURED_TRIANGLE:
			    S3D_UNLIT_TEXTURED_TRIANGLE;
			hwcx->hwBlendFunc = S3D_ALPHA_BLEND_TEXTURE_ALPHA;
			break;
		    case __GL_FORMAT_RGB8:
		    case __GL_FORMAT_RGB332:
		    case __GL_FORMAT_XRGB1555:
		    case __GL_FORMAT_RGB565:
			hwTexFunc |= S3D_TEXTURE_BLEND_DECAL;
			hwTexFunc |= (modeFlags & __GL_SHADE_TEXTURE_PERSP)?
			    S3D_PERSPECTIVE_UNLIT_TEXTURED_TRIANGLE:
			    S3D_UNLIT_TEXTURED_TRIANGLE;
			break;
		    default:
			OutputDebugStringA("HWTexEnvState: Unrecognizable nternal format\n");
			break;
		  }
		  break;
		  
	      case GL_BLEND:
		  OutputDebugStringA("HWTexEnvState: GL_BLEND unsupported");
		  break; // Blending not possible on S3Virge
    
	      case GL_REPLACE:
		  // No need to generate colors, only texture
		  // On S3V seems we need at least flat colors
		  // so only disable smooth color generation!
		  switch(lp->level.internalFormat) {
		    case __GL_FORMAT_ALPHA8:
			OutputDebugStringA("HWTexEnvState: GL_REPLACE+GL_ALPHA unsupported\n");
			break;
		    case __GL_FORMAT_INTENSITY8:
		    case __GL_FORMAT_RGB8:
		    case __GL_FORMAT_RGB332:
		    case __GL_FORMAT_XRGB1555:
		    case __GL_FORMAT_RGB565:
			hwTexFunc |= S3D_TEXTURE_BLEND_DECAL;
			hwTexFunc |= (modeFlags & __GL_SHADE_TEXTURE_PERSP)?
			    S3D_PERSPECTIVE_UNLIT_TEXTURED_TRIANGLE:
			    S3D_UNLIT_TEXTURED_TRIANGLE;
			break;
		    case __GL_FORMAT_COLOR_INDEX8:
		    case __GL_FORMAT_COLOR_INDEX16:
		    case __GL_FORMAT_RGBA8:
		    case __GL_FORMAT_RGBA4:
		    case __GL_FORMAT_ARGB4:
		    case __GL_FORMAT_RGBA5551:
		    case __GL_FORMAT_ARGB1555:
		    case __GL_FORMAT_LUMINANCE8:
		    case __GL_FORMAT_LUMINANCE_ALPHA8:
			hwTexFunc |= S3D_TEXTURE_BLEND_DECAL;
			hwTexFunc |= (modeFlags & __GL_SHADE_TEXTURE_PERSP)?
			    S3D_PERSPECTIVE_UNLIT_TEXTURED_TRIANGLE:
			    S3D_UNLIT_TEXTURED_TRIANGLE;
			hwBlendFunc = S3D_ALPHA_BLEND_TEXTURE_ALPHA;
			break;
		    default:
			{ 
			    char dbgbuf[256];
			    sprintf(dbgbuf, 
				    "Unknown texture internal format %x\n", 
				    lp->level.internalFormat);
			    OutputDebugStringA(dbgbuf);
			}
			break;
		  }
		  break;

	      default:
		  OutputDebugStringA("HWTexEnvState: Unknown texturing function\n");
		  break;
            }
	}
    }

    hwcx->hwTexFunc   = hwTexFunc;
    hwcx->hwBlendFunc = hwBlendFunc;
}

void __glS3VPickLineProcs(__GLcontext *gc)
{
    __GLS3Vcontext *hwcx = (__GLS3Vcontext *)gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    
    hwcx->swRenderLines = 1;
    __glGenericPickLineProcs(gc);

    if ((gc->buffers.lock.hwBufferMask & gc->buffers.lock.renderBufferMask) !=
	gc->buffers.lock.renderBufferMask) {
	return;
    }

    if ((gc->renderMode == GL_RENDER) &&
        !(gc->state.enables.general & __GL_LINE_SMOOTH_ENABLE) &&
        !(modeFlags & (__GL_SHADE_LINE_STIPPLE |
                     __GL_SHADE_STENCIL_TEST |
                     __GL_SHADE_STIPPLE      |
                     __GL_SHADE_TEXTURE      |
                     __GL_SHADE_SLOW_FOG     |
                     __GL_SHADE_INDEX_TEST   |
	             __GL_SHADE_LOGICOP      |
	             __GL_SHADE_ALPHA_TEST))) {

        if (modeFlags & __GL_SHADE_CHEAP_FOG) {
	    __glGenericPickLineProcs(gc);
	    return;
        } else {
            if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                if (modeFlags & __GL_SHADE_SMOOTH) {
    	            gc->procs.renderLine = __glS3VRenderSmoothDepthLine;
                    hwcx->swRenderLines = 0;
                }
                else {
    	            gc->procs.renderLine = __glS3VRenderFlatDepthLine;
                    hwcx->swRenderLines = 0;
                }
            }
            else {
                if (modeFlags & __GL_SHADE_SMOOTH) {
    	            gc->procs.renderLine = __glS3VRenderSmoothLine;
                    hwcx->swRenderLines = 0;
                }
                else {
    	            gc->procs.renderLine = __glS3VRenderFlatLine;
                    hwcx->swRenderLines = 0;
                }
            }
        }
    }
}


/*
** pick the depth function pointers
*/
int __glS3VPickDepthProcs(__GLcontext *gc)
{
    GLint depthIndex;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLS3Vcontext *hwcx = (__GLS3Vcontext *)gc;
    __GLdepthBuffer *dfb = &gc->depthBuffer;
    GLint byteWidth = dfb->buf.byteWidth;
    GLuint hwZFunc;

    depthIndex = __glGenericPickDepthProcs(gc);

    hwZFunc = 0;

    if (modeFlags & __GL_SHADE_DEPTH_TEST) {
	if (gc->state.depth.writeEnable) {
	    hwZFunc |= S3D_Z_BUFFER_NORMAL | S3D_Z_BUFFER_UPDATE;
	}

	switch (gc->state.depth.testFunc) {
	  default:
	  case GL_NEVER:
	      hwZFunc |= S3D_Z_COMP_NEVER ;
	      break;
	  case GL_LESS:
	      hwZFunc |= S3D_Z_COMP_S_LT_B;
	      break;
	  case GL_EQUAL:
	      hwZFunc |= S3D_Z_COMP_S_EQ_B;
	      break;
	  case GL_LEQUAL:
	      hwZFunc |= S3D_Z_COMP_S_LE_B;
	      break;
	  case GL_GREATER:
	      hwZFunc |= S3D_Z_COMP_S_GT_B;
	      break;
	  case GL_NOTEQUAL:
	      hwZFunc |= S3D_Z_COMP_S_NE_B;
	      break;
	  case GL_GEQUAL:
	      hwZFunc |= S3D_Z_COMP_S_GE_B;
	      break;
	  case GL_ALWAYS:
	      hwZFunc |= S3D_Z_COMP_ALWAYS;
	      break;
	} 
    } else {
	hwZFunc |= S3D_Z_COMP_ALWAYS;
    }

    hwcx->hwZFunc = hwZFunc;

    return depthIndex;
}

void __glS3VValidate(__GLcontext *gc)
{
    GLuint fogEnable;
    GLuint destinationColorFormat;
    GLuint hardwareClippingEnable;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLS3Vcontext *hwcx = (__GLS3Vcontext *)gc;

    (*gc->procs.pickAllProcs)(gc);

    /* set up hardware flags */
    if (modeFlags & __GL_SHADE_BLEND) {
	hwcx->hwBlendFunc = S3D_ALPHA_BLEND_SOURCE_ALPHA;
    }

    fogEnable = (modeFlags & __GL_SHADE_SLOW_FOG);
    destinationColorFormat = 1;
    hardwareClippingEnable = 0;

    hwcx->hwCmdMask =
	S3D_COMMAND_3D |
	hwcx->hwTexFunc |
	hwcx->hwZFunc |
	hwcx->hwBlendFunc |
	(fogEnable << 17) |
	(destinationColorFormat << 2) |
	(hardwareClippingEnable << 1);
}


void __glS3VPickAllProcs(__GLcontext *gc)
{
    __glGenericPickAllProcs(gc);
}


void __glS3VPickTriangleProcs(__GLcontext *gc)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLS3Vcontext *hwcx = (__GLS3Vcontext *)gc;

    if ((gc->buffers.lock.hwBufferMask & gc->buffers.lock.renderBufferMask) !=
	gc->buffers.lock.renderBufferMask) {
	__glGenericPickTriangleProcs(gc);
	return;
    }

    /*
    ** Setup cullFace so that a single test will do the cull check.
    */
    if (modeFlags & __GL_SHADE_CULL_FACE) {
	switch (gc->state.polygon.cull) {
	  case GL_FRONT:
	    gc->polygon.cullFace = __GL_CULL_FLAG_FRONT;
	    break;
	  case GL_BACK:
	    gc->polygon.cullFace = __GL_CULL_FLAG_BACK;
	    break;
	  case GL_FRONT_AND_BACK:
	    gc->procs.renderTriangle = __glDontRenderTriangle;
	    gc->procs.fillTriangle = 0;		/* Done to find bugs */
	    return;
	}
    } else {
	gc->polygon.cullFace = __GL_CULL_FLAG_DONT;
    }

    /* Build lookup table for face direction */
    switch (gc->state.polygon.frontFaceDirection) {
      case GL_CW:
	if (gc->constants.yInverted) {
	    gc->polygon.face[__GL_CW] = __GL_BACKFACE;
	    gc->polygon.face[__GL_CCW] = __GL_FRONTFACE;
	} else {
	    gc->polygon.face[__GL_CW] = __GL_FRONTFACE;
	    gc->polygon.face[__GL_CCW] = __GL_BACKFACE;
	}
	break;
      case GL_CCW:
	if (gc->constants.yInverted) {
	    gc->polygon.face[__GL_CW] = __GL_FRONTFACE;
	    gc->polygon.face[__GL_CCW] = __GL_BACKFACE;
	} else {
	    gc->polygon.face[__GL_CW] = __GL_BACKFACE;
	    gc->polygon.face[__GL_CCW] = __GL_FRONTFACE;
	}
	break;
    }

    /* Make polygon mode indexable and zero based */
    gc->polygon.mode[__GL_FRONTFACE] =
	(GLubyte) (gc->state.polygon.frontMode & 0xf);
    gc->polygon.mode[__GL_BACKFACE] =
	(GLubyte) (gc->state.polygon.backMode & 0xf);
    
    /* Check for modes not supported by chip */
    if ((gc->renderMode != GL_RENDER) ||
	(modeFlags & (__GL_SHADE_STIPPLE |
		      __GL_SHADE_ALPHA_TEST |
		      __GL_SHADE_INDEX_TEST |
		      __GL_SHADE_STENCIL_TEST |
		      __GL_SHADE_SLOW_FOG |
		      __GL_SHADE_POLYGON_OFFSET_FILL |
		      __GL_SHADE_LOGICOP ))) {

        hwcx->swRenderTri = 1;
	__glGenericPickTriangleProcs(gc);
	return;
    }

    if (gc->buffers.doubleStore == GL_TRUE) {
        hwcx->swRenderTri = 1;
	__glGenericPickTriangleProcs(gc);
	return;
    }

    if (modeFlags & __GL_SHADE_TEXTURE) {
        if (modeFlags & __GL_SHADE_BLEND) {
            hwcx->swRenderTri = 1;
	    __glGenericPickTriangleProcs(gc);
	    return;
        }
        if (modeFlags & __GL_SHADE_TEXTURE_PERSP) {
	    __GLtexture *current = gc->texture.currentTexture;
	    __GLDDrawMipMapLevel *lp = (__GLDDrawMipMapLevel *)current->level[0];

            if (lp->level.width > 128) {
                hwcx->swRenderTri = 0;
                gc->procs.fillTriangle = __glS3VTexPerspTessellatedTriangle;
            }
            else {
                if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                    if (modeFlags & __GL_SHADE_SMOOTH) {
                        hwcx->swRenderTri = 0;
                        gc->procs.fillTriangle = 
			    __glS3VTexPerspectiveSmoothDepthTriangle;
                    }  
                    else {
                        hwcx->swRenderTri = 0;
                        gc->procs.fillTriangle = 
			    __glS3VTexPerspectiveFlatDepthTriangle;
                    }
                }
                else {
                    if (modeFlags & __GL_SHADE_SMOOTH) {
                        hwcx->swRenderTri = 0;
                        gc->procs.fillTriangle = 
			    __glS3VTexPerspectiveSmoothTriangle;
                    }  
                    else {
                        hwcx->swRenderTri = 0;
                        gc->procs.fillTriangle = 
			    __glS3VTexPerspectiveFlatTriangle;
                    }
                }
            }
        }
        else {
            if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                if (modeFlags & __GL_SHADE_SMOOTH) {
                    hwcx->swRenderTri = 0;
                    gc->procs.fillTriangle = __glS3VTexSmoothDepthTriangle;
                }  
                else {
                    hwcx->swRenderTri = 0;
                    gc->procs.fillTriangle = __glS3VTexFlatDepthTriangle;
                }
            }
            else {
                if (modeFlags & __GL_SHADE_SMOOTH) {
                    hwcx->swRenderTri = 0;
                    gc->procs.fillTriangle = __glS3VTexSmoothDepthTriangle;
                }  
                else {
                    hwcx->swRenderTri = 0;
                    gc->procs.fillTriangle = __glS3VTexFlatTriangle;
                }
            }
        }

    } else {

        if (modeFlags & __GL_SHADE_DEPTH_TEST) {
	    if (modeFlags & __GL_SHADE_SMOOTH) {
                hwcx->swRenderTri = 0;
                gc->procs.fillTriangle = __glS3VRenderSmoothDepthTriangle;
            }  
            else {
                hwcx->swRenderTri = 0;
                gc->procs.fillTriangle = __glS3VRenderFlatDepthTriangle;
            }
        }
        else {
	    if (modeFlags & __GL_SHADE_SMOOTH) {
                hwcx->swRenderTri = 0;
                gc->procs.fillTriangle = __glS3VRenderSmoothTriangle;
            }  
            else {
                hwcx->swRenderTri = 0;
                gc->procs.fillTriangle = __glS3VRenderFlatTriangle;
            }
        }
    }

    if ((gc->state.polygon.frontMode == gc->state.polygon.backMode) &&
	    (gc->state.polygon.frontMode == GL_FILL)) {
	if (modeFlags & __GL_SHADE_SMOOTH_LIGHT) {
	    gc->procs.renderTriangle = __glRenderSmoothTriangle;
	} else {
	    gc->procs.renderTriangle = __glRenderFlatTriangle;
	}
    } else {
	gc->procs.renderTriangle = __glRenderTriangle;
    }

    if (modeFlags & __GL_SHADE_OWNERSHIP_TEST) {
	gc->procs.renderTriangle2 = gc->procs.renderTriangle;
	gc->procs.renderTriangle = __glClipAndRenderTriangle;
    }
}
