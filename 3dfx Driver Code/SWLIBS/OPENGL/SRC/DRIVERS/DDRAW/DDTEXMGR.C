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
**
*/
#define INITGUID
#include <ddraw.h>

#include "context.h"
#include "imports.h"
#include "global.h"
#include "g_imfncs.h"
#include "types.h"
#include "namesint.h"
#include "pixel.h"
#include "image.h"
#include "glmath.h"
#include "texfmt.h"
#include "texmgr.h"
#include "texture.h"
#include "gldevice.h"

#include "ddtexmgr.h"
#include "ddtexfmt.h"


//#define __DEBUG_PRINT 
#include "dbg.h"

extern HINSTANCE hDirectDraw;
extern struct IDirectDraw *lpDirectDraw;


/* XXX remove this later */
extern LPDIRECTDRAW __wglAllocateDDrawObject(GLvoid);
extern void __wglFreeDDrawObject(LPDIRECTDRAW);


/*
 * DDraw base machine-independent texture manager implementation 
 *
 * This texture manager is a generic implementation. It can be used one of
 * three ways:
 *
 *  1. Without modification, in cases where it suffices
 *  2. Use the manager as-is, but substitute a new (or enhanced) texture object
 *  3. Rewrite the whole thing to take advantage of an underlying texture
 *     management system.
 *
 */


/*
**  the texture manager object
*/
__GLDDrawTextureManager __glDDrawTextureManager =
{
    {
	0,
	__glDDrawReleaseTextureManager,
	__glDDrawInvalidateTextureManager,
    },
    NULL,

    NULL, 0, 0, NULL,
    0, 0, NULL, 0, 0,
};


/* -------------------------------------------------------------- */

static GLint 
ReleaseActiveTexture(__GLDDrawTextureManager *ddmgr, GLint objIdx, 
		     __GLDDrawTexture *tex)
{
    __GLDDrawTexture **activeListLim, **prevListP;
    GLint found = 0;

    if (!IDirectDrawSurface_IsLost(tex->lpVRAMSurface)) {
         IDirectDrawSurface_Release(tex->lpVRAMSurface);
     
        /* Now, remove it from the active list */

        if (objIdx == ddmgr->objListHead) {
              ddmgr->objListHead = ddmgr->activeList[objIdx]->next;
              if (objIdx == ddmgr->objListTail) {
                  ddmgr->objListHead = -1;
                  ddmgr->objListTail = -1;
              }
        }
        else {
              for (prevListP = ddmgr->activeList, 
                   activeListLim = prevListP + ddmgr->maxActiveObj; 
                   prevListP < activeListLim; prevListP++) {
                  if (*prevListP) {
                      if ((*prevListP)->next == objIdx) {
                          found = 1;
                          break;
                      }
                  }
              }
              if (found) {
                  if (ddmgr->objListTail == objIdx) {
                      ddmgr->objListTail = prevListP-ddmgr->activeList;
                  }
                  (*prevListP)->next = ddmgr->activeList[objIdx]->next;
              }
        }
        ddmgr->activeList[objIdx] = NULL;
    }
    objIdx = tex->next;
    return objIdx;
}


static GLint 
AddActiveTexture(__GLcontext *gc, __GLDDrawTextureManager *ddmgr, 
		 __GLDDrawTexture *tex)
{
    GLint objIdx;
    __GLDDrawTexture *prevP;

    if (ddmgr->numActiveObj >= ddmgr->maxActiveObj) {
        __GLDDrawTexture **oldlist;

        oldlist = ddmgr->activeList;
        ddmgr->activeList = (__GLDDrawTexture **)
                       (*gc->imports.calloc)(gc, ddmgr->maxActiveObj+20,
    			  sizeof(__GLDDrawTexture*));
        memset(ddmgr->activeList, 0, sizeof(__GLDDrawTexture*)*ddmgr->maxActiveObj+20);
        if (ddmgr->numActiveObj) {
                memcpy(ddmgr->activeList, oldlist, 
                     sizeof(__GLDDrawTexture*)*ddmgr->maxActiveObj);
        }
        ddmgr->maxActiveObj += 20;
    }

    for (objIdx = 0; ddmgr->activeList[objIdx]; objIdx++);
    ddmgr->numActiveObj++;
   
    ddmgr->activeList[objIdx] = tex;

    if (ddmgr->objListHead == -1) {
        tex->next          = -1;
        ddmgr->objListHead = objIdx;
        ddmgr->objListTail = objIdx;
    }
    else {
        prevP = ddmgr->activeList[ddmgr->objListTail];
        tex->next          = -1;
        prevP->next        = objIdx;
        ddmgr->objListTail = objIdx;
    }
    return objIdx;
}



/*******************************************************************/
/*                   Texture manager interface                     */
/*******************************************************************/

 
void __glDDrawInitTextureManager(__GLcontext *gc)
{
    __GLDDrawTextureManager *ddmgr = &__glDDrawTextureManager;

    ddmgr->texmgr.refcount++;
    if (ddmgr->texmgr.refcount == 1) {
	ddmgr->lpDDraw = __wglAllocateDDrawObject();
	assert(NULL != ddmgr->lpDDraw);

        ddmgr->activeList    = NULL;
        ddmgr->numActiveObj  = 0;
        ddmgr->maxActiveObj  = 0;
        ddmgr->objListHead   = -1;
        ddmgr->objListTail   = -1;
        ddmgr->lookupDDrawTextureFormat = __glLookupDDrawTextureFormat;
    }

    gc->texture.textureManager = &ddmgr->texmgr;
    gc->texture.createTexture = __glDDrawCreateTexture;
}

void __glDDrawReleaseTextureManager(__GLcontext *gc)
{
    __GLDDrawTextureManager *ddmgr =
	(__GLDDrawTextureManager *) gc->texture.textureManager;

    if (1 == ddmgr->texmgr.refcount) {
	__wglFreeDDrawObject(ddmgr->lpDDraw);
    }
    __glReleaseTextureManager(gc);
}


/* Notify the texture manager that texture priorities have changed */
void __glDDrawInvalidateTextureManager( __GLtextureManager *texmgr )
{
}


/* Register texture with manager */
static void TexMgrCreateTexture(__GLcontext *gc, __GLDDrawTexture *ddtex)
{
    __GLDDrawTextureManager *mgr =
	(__GLDDrawTextureManager *)gc->texture.textureManager;
    GLint objIdx;

    if (mgr->numObj >= mgr->maxObj) {
	__GLDDrawTexture **oldlist;

	oldlist = mgr->objList;
	mgr->objList = (__GLDDrawTexture **)
	    (*gc->imports.calloc)(gc, mgr->maxObj+20,
				  sizeof(__GLDDrawTexture*));
        memset(mgr->objList, 0, sizeof(__GLDDrawTexture*)*mgr->maxObj+20);
	if (mgr->numObj) {
	    memcpy(mgr->objList, oldlist, sizeof(__GLDDrawTexture*)*mgr->maxObj);
        }
	mgr->maxObj += 20;
    }

    for (objIdx = 0; mgr->objList[objIdx]; objIdx++);
    mgr->numObj++;
    mgr->objList[objIdx] = ddtex;
}


/* Delete texture from manager */
static void TexMgrDeleteTexture(__GLcontext *gc, __GLDDrawTexture *ddtex)
{
    __GLDDrawTextureManager *ddmgr =
	(__GLDDrawTextureManager *)gc->texture.textureManager;
    __GLDDrawTexture **objListP, **activeListP, **activeListLim, **prevP;
    IDirectDrawSurface *lpSurface;
    GLint objIdx;
    GLint found;

    for (found = 0, activeListP   = ddmgr->activeList, 
         activeListLim = activeListP+ddmgr->maxActiveObj; 
         activeListP < activeListLim;
         activeListP++) {
         if (*activeListP == ddtex) {
             found = 1;
             break;
         }
    }

    if (found) {
        objIdx = activeListP - ddmgr->activeList;
        if (objIdx == ddmgr->objListHead) {
             ddmgr->objListHead = (*activeListP)->next;
             if (objIdx == ddmgr->objListTail) {
                  ddmgr->objListHead = -1;
                  ddmgr->objListTail = -1;
             }
        }
        else {
            for (prevP = ddmgr->activeList, activeListLim = prevP + ddmgr->maxActiveObj; 
                 prevP < activeListLim; prevP++) {
                if ((*prevP)->next == objIdx) {
                    found = 1;
                    break;
                }
            }
            if (found) {
                if (ddmgr->objListTail == objIdx) {
                    ddmgr->objListTail = prevP-ddmgr->activeList;
                }
                (*prevP)->next = (*activeListP)->next;
            }
        }
    }

    /* Search for the texture in the manager's list */
    for (objListP = ddmgr->objList; *objListP != ddtex; objListP++);
    assert(*objListP == ddtex);

    /* If we didn't find it, something went terribly wrong! */

    lpSurface = (*objListP)->lpVRAMSurface;
    if (lpSurface) {
        IDirectDrawSurface_Release(lpSurface);
    }
    (*objListP)->lpVRAMSurface = NULL;

    lpSurface = (*objListP)->lpSystemSurface;
    if (lpSurface) IDirectDrawSurface_Release(lpSurface);
    (*objListP)->lpSystemSurface = NULL;

    *objListP = NULL;
}


/* Make a texture resident */
static void TexMgrMakeResident(__GLcontext *gc, __GLDDrawTexture *tex,
			       GLclampf priority)
{
    __GLDDrawTextureManager *ddmgr =
	(__GLDDrawTextureManager *)gc->texture.textureManager;
    __GLDDrawTexture *objP = (__GLDDrawTexture *)tex;
    GLint pri = (GLint)(priority*__GL_NUM_PRIORITIES);
    GLint loaded = 0;
    GLint totalVidMem, availVidMem, needVidMem;
    struct IDirectDraw2 *lpDDraw2;
    __GLDDrawMipMapLevel    *levelP;
    IDirectDrawSurface *lpMipSurf, *lpVramSurf = NULL;
    GLint i, ddrval;
    DDSURFACEDESC ddsd;
    DDSCAPS	  ddsCaps;
    GLint maxDim;
    GLint objIdx, dispair;
    __GLDDrawTexture *texP;
    __GLDDrawTextureFormat *ddtf;
    
    
/* ErrorF("TexMgrMakeResident\r\n"); */

    /* This question is: can we fit the current texture into texture 
     * storage without releasing another one? To find out, we allocate 
     * the surface here. 
     */

#ifndef __GL_SOFTWARE_ONLY

    if (!objP->lpVRAMSurface) {
        IDirectDraw_QueryInterface(ddmgr->lpDDraw, &IID_IDirectDraw2, (LPVOID*)&lpDDraw2);
	memset(&ddsd, 0, sizeof(DDSURFACEDESC));
	ddsd.dwSize  = sizeof(DDSURFACEDESC);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
        ddtf = (*ddmgr->lookupDDrawTextureFormat)(tex->texture.format->internalFormat);
        ddsd.ddpfPixelFormat = ddtf->ddFormat;

	ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY |
                              DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;

#ifndef __GL_RECT_TEXTURES
        maxDim = (objP->texture.level[0]->width > objP->texture.level[0]->height)?
                  objP->texture.level[0]->width :
                  objP->texture.level[0]->height;
	ddsd.dwWidth       = maxDim;
	ddsd.dwHeight      = maxDim;
#else
	ddsd.dwWidth       = objP->texture.level[0]->width;
	ddsd.dwHeight      = objP->texture.level[0]->height;
#endif
	ddrval =
	    IDirectDraw_CreateSurface(ddmgr->lpDDraw, &ddsd,
				      &lpVramSurf, NULL);
	if (ddrval == DD_OK) {

            /* We can put this object on the active list */

            objIdx = AddActiveTexture(gc, ddmgr, objP);
            loaded = 1;

	    /* We created the complex surface for the texture. Now we 
	     * associate the mip level surfaces with each mip level.
	     *
	     * The base (level 0 ) mipmap is the master surface for the texture. 
	     */


            objP->lpVRAMSurface = lpVramSurf;
	    ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
	    for (i = 0, 
	         lpMipSurf = objP->lpVRAMSurface; 
	         i < objP->texture.numLevels; i++) {
	         levelP = (__GLDDrawMipMapLevel *)objP->texture.level[i], 
	         levelP->lpMipSurface = lpMipSurf;

	         ddrval = IDirectDrawSurface_Lock(lpMipSurf, NULL, &ddsd, 
		        DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	         levelP->lpMipBuffer = ddsd.lpSurface;
	         if (levelP->slurpImage) {
                      (*levelP->slurpImage)(objP, levelP);
                 }
	         IDirectDrawSurface_Unlock(lpMipSurf, NULL);
	         if (lpMipSurf) {
		     ddrval = IDirectDrawSurface_GetAttachedSurface(lpMipSurf, 
					 &ddsCaps, &lpMipSurf);
		     if (ddrval != DD_OK) {
		         lpMipSurf = NULL;
		     }
	         }
	    }
	    objP->texture.residence = __GL_TEXTURE_RESIDENCE_HIPERF;
            objP->lpTexSurface = objP->lpVRAMSurface;
        }
        else if (ddrval == DDERR_OUTOFVIDEOMEMORY) {
          
            /* If the texture is higher priority than the textures which have video
             * ram, take the vram, and boot the texture out.  If we are where we should
             * be, then just alloc a system memory texture, if possible
             */
    
             if (__glDevice->textureFlags & __GL_ALLOCATE_HW_TEXTURE) {
                 loaded = 0;
             }
             else {
                 loaded = 0;
             }
        }
    }
    else if (IDirectDrawSurface_IsLost(objP->lpVRAMSurface)){
        loaded = 1;
        objP->texture.residence = __GL_TEXTURE_RESIDENCE_HIPERF;
    }
    else {
        return;
    }

    if (!loaded) {
          DDSCAPS ddscaps;
         
          /* We need to toss someone out. First, find out how much we need */
          
          IDirectDraw_QueryInterface(ddmgr->lpDDraw, &IID_IDirectDraw2, (LPVOID*)&lpDDraw2);
          ddscaps.dwCaps = DDSCAPS_VIDEOMEMORY;
          IDirectDraw2_GetAvailableVidMem(lpDDraw2, &ddscaps, &totalVidMem, &availVidMem);
#ifndef __GL_RECT_TEXTURES
          needVidMem = (tex->texture.format->bitsPerTexel >> 3)*maxDim*maxDim;
#else
          needVidMem = (tex->texture.format->bitsPerTexel >> 3)*
                           objP->texture.level[0]->width*objP->texture.level[0]->height;
#endif

          /* Then release until we have the requesite memory. We know we have to
             free at least the difference between the available vram and the 
             required vram, so do that first
           */

          for (objIdx = ddmgr->objListHead; 
               objIdx != -1;
               ) {

              texP = ddmgr->activeList[objIdx];
              if (texP->lpVRAMSurface) {
                  objIdx = ReleaseActiveTexture(ddmgr, objIdx, texP);
                  texP->lpVRAMSurface = NULL;
                  ddscaps.dwCaps = DDSCAPS_VIDEOMEMORY;
                  IDirectDraw2_GetAvailableVidMem(lpDDraw2, &ddscaps, &totalVidMem, &availVidMem);
                  if (availVidMem >= needVidMem) {
                      break;
                  }
              }
              else {
                  objIdx = texP->next;
              }
          }

          if (availVidMem < needVidMem) {
               ErrorF("TexMgrMakeResident -- Out Of Memory1\r\n");
               return;
          }

          ddtf = (*ddmgr->lookupDDrawTextureFormat)(tex->texture.format->internalFormat);
          for (dispair = 0; !loaded && !dispair; ) {
              memset(&ddsd, 0, sizeof(DDSURFACEDESC));
              ddsd.dwSize  = sizeof(DDSURFACEDESC);
	      ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
              ddsd.ddpfPixelFormat = ddtf->ddFormat;
              ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY |
                                     DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;

#ifndef __GL_RECT_TEXTURES
              maxDim = (objP->texture.level[0]->width > objP->texture.level[0]->height)?
                  objP->texture.level[0]->width :
                  objP->texture.level[0]->height;
              ddsd.dwWidth       = maxDim;
              ddsd.dwHeight      = maxDim;
#else
              ddsd.dwWidth       = objP->texture.level[0]->width;
              ddsd.dwHeight      = objP->texture.level[0]->height;
#endif

	      ddrval = IDirectDraw_CreateSurface(ddmgr->lpDDraw, &ddsd,
				      &lpVramSurf, NULL);
         
	      if (ddrval == DD_OK) {
                  objIdx = AddActiveTexture(gc, ddmgr, objP);
                  loaded = 1;
                  objP->lpVRAMSurface = lpVramSurf;
                  ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
                  for (i = 0, 
                      lpMipSurf = objP->lpVRAMSurface; 
                      i < objP->texture.numLevels && lpMipSurf; i++) {
                      levelP = (__GLDDrawMipMapLevel *)objP->texture.level[i], 
                      levelP->lpMipSurface = lpMipSurf;
    
                      ddrval = IDirectDrawSurface_Lock(lpMipSurf, NULL, &ddsd, 
            	                      DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
                      levelP->lpMipBuffer = ddsd.lpSurface;
                      if (levelP->slurpImage) {
                          (*levelP->slurpImage)(objP, levelP);
                      }
                      IDirectDrawSurface_Unlock(lpMipSurf, NULL);
                      if (lpMipSurf) {
            	           ddrval = IDirectDrawSurface_GetAttachedSurface(lpMipSurf, 
            				 &ddsCaps, &lpMipSurf);
            	           if (ddrval != DD_OK) {
            	                 lpMipSurf = NULL;
            	           }
		      }
	          }
              }
              else if (ddrval == DDERR_OUTOFVIDEOMEMORY) {
                  dispair = 1;
                  while (objIdx != -1 ) {

                      /* Not enough space? Probably fragmentation. Reap another one */

                      dispair = 0;
                      texP = ddmgr->activeList[objIdx];
                      if (texP->lpVRAMSurface) {
                           objIdx = ReleaseActiveTexture(ddmgr, objIdx, texP);
                           texP->lpVRAMSurface = NULL;
                           ddscaps.dwCaps = DDSCAPS_VIDEOMEMORY;
                           IDirectDraw2_GetAvailableVidMem(lpDDraw2, 
                                        &ddscaps, &totalVidMem, &availVidMem);
                           if (availVidMem >= needVidMem) {
                               break;
                           }
                      }
                      else {
                          objIdx = texP->next;
                      }
                  }
              }
              else {
                  dispair = 1;
ErrorF("TexMgrMakeResident -- something is wrong\r\n");
              }
          }
    }

    if (/* still */!loaded) {
ErrorF("TexMgrMakeResident -- Out Of Memory2\r\n");
        return;
    }
#endif
// ErrorF("TexMgrMakeResident -- end\r\n");
}


/*******************************************************************/
/*                   Texture object management                     */
/*******************************************************************/


__GLtextureBuffer *
__glDDrawTextureCreateLevel(__GLcontext *gc, __GLtexture *tex,
			    GLint lod, GLint components,
			    GLsizei w, GLsizei h, GLsizei d,
			    GLint border, GLint dim)
{ 
    __GLtextureBuffer *rtn;
    __GLDDrawTextureFormat *ddtf;
    __GLDDrawMipMapLevel   *ddlp;
    __GLDDrawTexture       *ddtex = (__GLDDrawTexture *)tex;
    GLint prevw, prevh;
    GLint objIdx;
    __GLDDrawTextureManager *ddmgr =
	(__GLDDrawTextureManager *)gc->texture.textureManager;

//ErrorF("glDDrawTextureCreateLevel\r\n");

    ddlp = (__GLDDrawMipMapLevel *)tex->level[lod];
    prevw = ddlp->level.width;
    prevh = ddlp->level.height;
    rtn = __glTextureCreateLevel(gc, tex, lod, components, w, h, d, border, dim);

    ddtf = (*ddmgr->lookupDDrawTextureFormat)(tex->format->internalFormat);
    ddlp->slurpImage    = ddtf->slurpImage;
    ddlp->slurpSubImage = ddtf->slurpSubImage;
    ddlp->bytesPerTexel = ddtf->ddFormat.dwRGBBitCount >> 3;

    if (prevw && prevh) {
        if ( (prevw < ddlp->level.width) || (prevw < ddlp->level.height)) {
            if (ddtex->lpVRAMSurface) {
                for (objIdx = 0; ddmgr->objList[objIdx] != ddtex; objIdx++);
                objIdx = ReleaseActiveTexture(ddmgr, objIdx, ddtex);

                ddtex->lpVRAMSurface = NULL;
                ddtex->texture.residence = __GL_TEXTURE_RESIDENCE_LOADED;
            }
        }
    }

//ErrorF("glDDrawTextureCreateLevel -- end\r\n");
    return rtn;
}


__GLtexture *
__glDDrawCreateTexture(__GLcontext *gc, GLint name, GLint targetIndex)
{
    __GLtexture *tex;
    GLint level, maxMipMapLevel;
    __GLDDrawMipMapLevel *ddlp;

    tex = (__GLtexture *)
	(*gc->imports.calloc)(gc, 1, sizeof(__GLDDrawTexture));
    assert(NULL != tex);

    tex->gc		  = gc;
    tex->refcount	  = 1;
    tex->targetIndex	  = targetIndex;
    tex->residence	  = GL_FALSE;
    tex->free	          = __glDDrawFreeTexture;
    tex->makeResident     = __glDDrawTextureMakeResident;
    tex->copyTexImage     = __glCopyTexImage;
    tex->readTexImage     = __glReadTexImage;
    tex->name             = name;
    tex->texobjs.name     = name;
    tex->texobjs.priority = 1.0;
    tex->CT.format	  = GL_RGBA;

    /*
    ** Can't copy the params currently in the gc state.texture params,
    ** because they might not be at init conditions.
    */
    tex->params.sWrapMode = GL_REPEAT;
    tex->params.tWrapMode = GL_REPEAT;
    tex->params.minFilter = GL_NEAREST_MIPMAP_LINEAR;
    tex->params.magFilter = GL_LINEAR;

    switch (targetIndex) {
    case __GL_TEXTURE_INDEX_1D:
	tex->dim = 1;
	tex->createLevel  = __glDDrawTextureCreateLevel;
	tex->CT.target = GL_TEXTURE_1D;
	break;
    case __GL_TEXTURE_INDEX_2D:
	tex->dim = 2;
	tex->createLevel  = __glDDrawTextureCreateLevel;
	tex->CT.target = GL_TEXTURE_2D;
	break;
    case __GL_PROXY_TEXTURE_INDEX_1D:
	tex->dim = 1;
	tex->createLevel  = __glTextureCreateProxyLevel;
	break;
    case __GL_PROXY_TEXTURE_INDEX_2D:
	tex->dim = 2;
	tex->createLevel  = __glTextureCreateProxyLevel;
	break;
    default:
	break;
    }
    tex->releaseLevel = __glDDrawTextureDeleteLevel;

    maxMipMapLevel = gc->constants.maxMipMapLevel;
    tex->level = (__GLmipMapLevel**)
	(*gc->imports.calloc)(gc, (size_t) maxMipMapLevel,
			      sizeof(__GLmipMapLevel*));

    tex->level[0] = (__GLmipMapLevel*)
	(*gc->imports.calloc)(gc, (size_t) maxMipMapLevel,
			      sizeof(__GLDDrawMipMapLevel));
    /* Init each texture level */
    for (level = 0, ddlp = (__GLDDrawMipMapLevel *)tex->level[0];
	 level < maxMipMapLevel; level++, ddlp++) {
	tex->level[level] = (__GLmipMapLevel *)ddlp;
	tex->level[level]->requestedFormat = 1;
    }

    TexMgrCreateTexture(gc, (__GLDDrawTexture *) tex);

    return (__GLtexture *)tex;
}


void __glDDrawFreeTexture(__GLcontext *gc, __GLtexture *tex)
{
    TexMgrDeleteTexture(gc, (__GLDDrawTexture *) tex);
    __glFreeTexture(gc, tex);
}



GLenum
__glDDrawTextureMakeResident(__GLcontext *gc, __GLtexture *tex,
			     GLclampf priority)
{
    __GLDDrawTextureManager *ddmgr =
	(__GLDDrawTextureManager *) gc->texture.textureManager;
    LPDIRECTDRAW lpDirectDraw = ddmgr->lpDDraw;
    __GLDDrawTexture *ddtex = (__GLDDrawTexture *)tex;
    GLint texloc =
	(__glDevice->textureFlags & (__GL_ALLOCATE_HW_TEXTURE |
				     __GL_ALLOCATE_FASTHW_TEXTURE)) ?
	DDSCAPS_VIDEOMEMORY :
	DDSCAPS_SYSTEMMEMORY;

    assert(NULL != tex);

    /* Notify the texture manager. Per notification can result in 
     * the TM clearing stuff per priority. In this impl, ddraw is 
     * managing all of the residence (wont work for AGP, will it?)
     * So we ignore the result.
     */

    TexMgrMakeResident(gc, (__GLDDrawTexture *)ddtex, priority);

    /* let's just say that we loaded the texture */
    return __GL_TEXTURE_RESIDENCE_LOWPERF;
}


void __glDDrawTextureDeleteLevel(__GLcontext *gc, __GLtexture *tex, GLint lod)
{
    __GLmipMapLevel *lp = tex->level[lod];

    __glTextureDeleteLevel(gc, tex,lod);
}


