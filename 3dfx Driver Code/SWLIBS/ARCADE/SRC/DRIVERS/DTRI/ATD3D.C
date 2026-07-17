/*
** Copyright (c) 1995,1996 3Dfx Interactive, Inc.
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
*/

/* TBD: 
       reconcile d3dapp and d3Context
*/

#include <math.h>
#include <stdio.h>
#include <3dfx.h>
#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <glide.h>
#include <atrender.h>
#include <gump.h>
#include <d3d.h>
#include "d3dappi.h"
#include "ddutil.h"
#include "mipmap.h"
#include "atd3d.h"

void _atrD3DProcessVerticesTemp(_AtrD3DDriver *ctx, DWORD flags, DWORD count);

/* TBD: temp hack 3dfx line & point draw requires z disabled */

static BOOL zEnable=TRUE;

const float _atrD3D_ZSCALE = 1.0f/65535.f;

#ifdef DTRIAPI
extern FxU8 _cacheMemoryChunk[4000];
extern FxU8 *_cacheMemTemp;
#endif

D3DAppInfo *d3dapp;
extern SIZE szBuffers;          /* Current buffer dimensions, not necessarily
                                   the same as the client window */

FxBool _atrD3DRealizeImg( AtrImg *img );
void _atrD3DDrawTriangle(const GrVertex *a, const GrVertex *b, const GrVertex *c);
static BOOL _atrD3DBeforeDeviceDestroyed(LPVOID lpContext);
static BOOL _atrD3DAfterDeviceCreated(int w, int h, LPDIRECT3DVIEWPORT* lpViewport,
                               LPVOID lpContext);
static BOOL _atrD3DRestoreSurfaces();
static void _atrD3DInitDispatchTable(_AtrD3DDriver *ctx);

/* graphics contexts for d3d driver */

static _AtrD3DDriver d3dContexts[MAX_CONTEXTS];
_AtrD3DDriver *d3dContext = &d3dContexts[0]; /* TBD current context # */

#define MAX_VERTICES     1024
#define MAX_INSTRUCTIONS 1024
#define MAX_TEXTURES 100
#define NUM_VERTICES 3
#define NUM_TRIANGLES 1

static D3DTLVERTEX src_v[NUM_VERTICES];

static int t[8][3] = {
        0, 1, 2,
    };

void __cdecl dprintf(char *format, ...) {
#ifdef AT_DEBUGGING
  va_list args;
  char buffer[512];

  va_start(args, format);
  vsprintf(buffer, format, args);
  OutputDebugStr(buffer);
#endif
}

extern float _atrDriverXOffset;
extern float _atrDriverYOffset;


D3DINSTRUCTION      test1;
D3DPROCESSVERTICES  test2;


/*-------------------------------------------------------------------
  Function: _atrD3dInitExecuteBuffer
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Initialize the execute buffer
  Arguments:
    ctx - the rendering context
  Return:
    Nothing
  -------------------------------------------------------------------*/

static void _atrD3DInitExecuteBuffer(_AtrD3DDriver *ctx) {
    EBufferData *ebd = &ctx->ebd;

	if (ctx->lpD3DExBuf->lpVtbl->Lock(ctx->lpD3DExBuf, &ctx->debDesc) != D3D_OK) {
        atuError(FXTRUE, "can't lock execute buffer\n");
    }
    ebd->lpBufStart = ctx->debDesc.lpData;
    ebd->lpVertexPointer = ebd->lpBufStart;
    ebd->lpInstStart = (void *)(((LPD3DVERTEX) (ebd->lpBufStart)) + MAX_VERTICES);
	ebd->lpInstPointer = ebd->lpInstStart;
	ebd->lpInstEnd = ebd->lpInstStart + sizeof(D3DINSTRUCTION) * MAX_INSTRUCTIONS;

    ebd->lastOpcode = NO_OPCODE; /* bOpcode in D3DINSTRUCTION struct */
    ebd->lastCount = NULL;  /* wCount in D3DINSTRUCTION struct */
    ebd->maxVertices = MAX_VERTICES;
    ebd->numVertices = 0;
   _atrD3DProcessVerticesTemp(ctx,
                              D3DPROCESSVERTICES_COPY | D3DPROCESSVERTICES_UPDATEEXTENTS,
                              0);
}

void atD3DHandleError(HRESULT error, char *message) {
    if (LastError == DD_OK) 
        return;

    if (LastError == DDERR_SURFACELOST) {
        d3dappi.lpFrontBuffer->lpVtbl->Restore(d3dappi.lpFrontBuffer);
        d3dappi.lpBackBuffer->lpVtbl->Restore(d3dappi.lpBackBuffer);
        return;
    }

    atuError(FXTRUE, "D3D Error %s: in %s\n", 
             D3DAppErrorToString(LastError), message);
}

/*-------------------------------------------------------------------
  Function: _atrD3DRenderExecuteBuffer
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Render an execute buffer
  Arguments:
    ctx - the rendering context
  Return:
    FXTRUE if the eontexi can currently be drawn to, FXFALSE otherwise
  -------------------------------------------------------------------*/


static BOOL _atrD3DRenderExecuteBuffer(_AtrD3DDriver *ctx) {
#ifndef DTRIAPI 
    EBufferData *ebd = &ctx->ebd;
    char *start;
    HRESULT LastError;

   if ( ebd->lpInstPointer == ebd->lpInstStart ) /*  empty buffer? */
      return TRUE;

	OP_EXIT(ebd->lpInstPointer);

    /* if we have received any vertices complete the the PROCESS VERTICES
       instruction, else start execution following it */

    if ( ebd->numVertices > 0 ) {
        ((D3DPROCESSVERTICES *)(ebd->lpInstStart+4))->wStart=0;
        ((D3DPROCESSVERTICES *)(ebd->lpInstStart+4))->wDest=0;
        ((D3DPROCESSVERTICES *)(ebd->lpInstStart+4))->dwCount=ebd->numVertices;
        start = ebd->lpInstStart;
    } else {
        start = ebd->lpInstStart + sizeof(D3DINSTRUCTION)+
                                          sizeof(D3DPROCESSVERTICES);
    }

    /*
     * Setup the execute data
     */

    ctx->lpD3DExBuf->lpVtbl->Unlock(ctx->lpD3DExBuf);
//    memset(&ctx->d3dExData, 0, sizeof(D3DEXECUTEDATA));
    ctx->d3dExData.dwSize = sizeof(D3DEXECUTEDATA);
    ctx->d3dExData.dwVertexCount = ebd->numVertices;
    ctx->d3dExData.dwInstructionOffset = (ULONG) ((char *)start - (char *)ebd->lpBufStart);
    ctx->d3dExData.dwInstructionLength = (ULONG) ((char *)ebd->lpInstPointer - (char *)start);
    ctx->lpD3DExBuf->lpVtbl->SetExecuteData(ctx->lpD3DExBuf, &ctx->d3dExData);
	LastError = ctx->lpDev->lpVtbl->Execute(ctx->lpDev, ctx->lpD3DExBuf,
                               ctx->lpView, D3DEXECUTE_UNCLIPPED);

    if ( LastError != DD_OK ) {
         atD3DHandleError(LastError, "_atrD3DRenderExecuteBuffer");
    }

	_atrD3DInitExecuteBuffer(ctx);

#endif
	return TRUE;
}

/*-------------------------------------------------------------------
  Function: _atrBeginScene
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Prepare a context for rendering. Check if any surfaces have been
    lost and if so restore them. Do any device specific actions and
    get the current buffer size.
  Arguments:
    viewWidth  - returns the current width of the drawing surface
    viewHeight - returns the current height of the drawing surface
  Return:
    FXTRUE if the context is ready for rendering, FXFALSE otherwise
  -------------------------------------------------------------------*/

static FxBool
_atrD3DBeginScene(FxU32 *viewWidth, FxU32 *viewHeight) {
    /*
     * Restore any lost surfaces
     */
    if (!_atrD3DRestoreSurfaces()) {
        /*
         * Restoring surfaces sometimes fails because the surfaces cannot
         * yet be restored.  If this is the case, the error will show up
         * somewhere else and we should return success here to prevent
         * unnecessary error's being reported.
         */
        return FXTRUE;
    }

    if (d3dapp->lpD3DDevice->lpVtbl->BeginScene(d3dapp->lpD3DDevice) != D3D_OK)
        return FXFALSE;

    *viewWidth = d3dContext->any.caps.width = szBuffers.cx ;
    *viewHeight = d3dContext->any.caps.height = szBuffers.cy ;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: _atrEndScene
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Flush any commands in the execute buffer, perform any device specific
    actions, determine area of surface which needs updating
  Arguments:
    None
  Return:
    FXTRUE if succesful, FXFALSE otherwise
  -------------------------------------------------------------------*/

static FxBool
_atrD3DEndScene(void) {
	D3DRECT rect;

	_atrD3DRenderExecuteBuffer(d3dContext);

    if (d3dapp->lpD3DDevice->lpVtbl->EndScene(d3dapp->lpD3DDevice) != D3D_OK)
        return FXFALSE;

    if (d3dContext->lpD3DExBuf->lpVtbl->GetExecuteData(d3dContext->lpD3DExBuf, 
                                                         &d3dContext->d3dExData) != D3D_OK)
        return FXFALSE;

    rect = d3dContext->d3dExData.dsStatus.drExtent;
    /*
     * Give D3DApp the extents so it can keep track of dirty sections of
     * the back and front buffers
     */

    if (!D3DAppRenderExtents(1, &rect, 
         d3dapp->bResized ? D3DAPP_CLEARALL : 0)) {
        atuError(FXTRUE, "%s", D3DAppLastErrorString());
    }

    /*
     * Reset the resize flag
     */

     d3dapp->bResized = FALSE;

     /* 
      * fake rendering to front buffer for now
      */

     if ( d3dContext->renderBuffer == ATR_BUFFER_FRONTBUFFER ) {
        D3DAppShowBackBuffer(D3DAPP_SHOWALL);
     }

     return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: _atrD3DInstruction
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Add an instruction to the execute buffer, trying to merge it into
    previous one.
  Arguments:
    ctx    - the rendering context
    opcode - what instruction to add
    size   - of instruction
    count  - number of instructions being added
  Return:
    None
  -------------------------------------------------------------------*/

void 
_atrD3DInstruction(_AtrD3DDriver *ctx, BYTE opcode, BYTE size, WORD count) {
    EBufferData *ebd = &ctx->ebd;

    if(ebd->lastOpcode==opcode && *ebd->lastCount!=255) {
      *ebd->lastCount+=count;
      return;
    }

    ((D3DINSTRUCTION *)ebd->lpInstPointer)->bOpcode=opcode;
    ((D3DINSTRUCTION *)ebd->lpInstPointer)->bSize=size;
    ((D3DINSTRUCTION *)ebd->lpInstPointer)->wCount=count;

    ebd->lastOpcode=opcode;
    ebd->lastCount=&(((D3DINSTRUCTION *)ebd->lpInstPointer)->wCount);
    ebd->lpInstPointer+=sizeof(D3DINSTRUCTION);
}

/*-------------------------------------------------------------------
  Function: _atrProcessVertices
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Add an instruction to process a specified number of vertices in
    the execute buffer
  Arguments:
    ctx   - the rendering context
    flags - vertex flags
    count - number of vertices being processed
  Return:
    None
  -------------------------------------------------------------------*/

void 
_atrD3DProcessVerticesTemp(_AtrD3DDriver *ctx, DWORD flags, DWORD count) {
    EBufferData *ebd = &ctx->ebd;

    ebd->lpVertexPointer +=  count*sizeof(D3DTLVERTEX);
  
    _atrD3DInstruction(ctx, D3DOP_PROCESSVERTICES, 
                       sizeof(D3DPROCESSVERTICES), 1);

    ((D3DPROCESSVERTICES *)ebd->lpInstPointer)->dwFlags=flags;
    ((D3DPROCESSVERTICES *)ebd->lpInstPointer)->wStart=ebd->numVertices;
    ((D3DPROCESSVERTICES *)ebd->lpInstPointer)->wDest=ebd->numVertices;
    ((D3DPROCESSVERTICES *)ebd->lpInstPointer)->dwCount=count;
    ((D3DPROCESSVERTICES *)ebd->lpInstPointer)->dwReserved=0;
    ebd->lpInstPointer+=sizeof(D3DPROCESSVERTICES);
    ebd->numVertices += count;
}

void _atrD3DProcessVertices(_AtrD3DDriver *ctx, DWORD flags, DWORD count)
{
    EBufferData *ebd = &ctx->ebd;

    ebd->lpVertexPointer +=  count*sizeof(D3DTLVERTEX);

    ebd->numVertices += count;
}

/*-------------------------------------------------------------------
  Function: atrD3DCheckAlignment
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Use OP_NOP to QWORD align triangle and line instructions.
  Arguments:
    ctx - the rendering context
  Return:
    None
  -------------------------------------------------------------------*/

void 
_atrD3DCheckAlignment(_AtrD3DDriver *ctx) {
    EBufferData *ebd = &ctx->ebd;

    if (QWORD_ALIGNED(ebd->lpInstPointer)) {
        _atrD3DInstruction(ctx, D3DOP_TRIANGLE, sizeof(D3DTRIANGLE), 0);
    }
}

/*-------------------------------------------------------------------
  Function: _atrD3DCheckBufferFull
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    ensure we have enough space in buffer in the worst case we need 
    space for the vertices, and instructions and instruction data
  Arguments:
    ctx - the rendering context
    numVerts - number of vertices to be added
    numInstr - number of instructions to be added
    misc     - space for instruction data
  Return:
    None
  -------------------------------------------------------------------*/


void 
_atrD3DCheckBufferFull(_AtrD3DDriver *ctx, int numVerts, int numInstr, 
                       size_t misc) {
    size_t instBytesNeeded, instBytesLeft;
    EBufferData *ebd = &ctx->ebd;

    instBytesNeeded = numInstr*sizeof(D3DINSTRUCTION) + misc;
    instBytesLeft = ebd->lpInstEnd - ebd->lpInstPointer ;

    if ((( ebd->numVertices + numVerts ) > ebd->maxVertices ) || ( instBytesNeeded > instBytesLeft )) {
       _atrD3DRenderExecuteBuffer(ctx);
    }
}

/*-------------------------------------------------------------------
  Function: _atrD3DRenderStatef
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Add a render state instruction with a D3D value argument
  Arguments:
    ctx  - the rendering context
    type - type of instruction
    v    - value 
  Return:
    None
  -------------------------------------------------------------------*/

#ifdef DTRIAPI
#define MAX_STATE 100
static D3DSTATE state[MAX_STATE];
static nextState = 0;
void _atrD3DFlushState(void) {
    if ( nextState == 0 )
        return;

	LastError = d3dContext->lpDev->lpVtbl->SetState(d3dContext->lpDev,
                                            state, nextState);

    nextState = 0;
}
#endif

#ifdef DTRIAPI
static void 
_atrD3DRenderStatef(_AtrD3DDriver *ctx, D3DRENDERSTATETYPE type, D3DVALUE v) {
    if ( nextState == MAX_STATE )
         _atrD3DFlushState();

    state[nextState].drstRenderStateType=type;
    state[nextState].dvArg[0]=v;
    nextState++;
}
#else
static void 
_atrD3DRenderStatef(_AtrD3DDriver *ctx, D3DRENDERSTATETYPE type, D3DVALUE v) {
    EBufferData *ebd = &ctx->ebd;

    _atrD3DCheckBufferFull(ctx, 0, 1, sizeof(D3DSTATE));

    _atrD3DInstruction(ctx, D3DOP_STATERENDER, sizeof(D3DSTATE), 1);

    ((D3DSTATE *)ebd->lpInstPointer)->drstRenderStateType=type;
    ((D3DSTATE *)ebd->lpInstPointer)->dvArg[0]=v;

    ebd->lpInstPointer+=sizeof(D3DSTATE);
}
#endif

/*-------------------------------------------------------------------
  Function: _atrD3DRenderState
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Add a render state instruction with a DWORD value argument
  Arguments:
    ctx  - the rendering context
    type - type of instruction
    v    - value 
  Return:
    None
  -------------------------------------------------------------------*/

#ifdef DTRIAPI
static void 
_atrD3DRenderState(_AtrD3DDriver *ctx, D3DRENDERSTATETYPE type, DWORD arg) {
    if ( nextState == MAX_STATE )
         _atrD3DFlushState();

    state[nextState].drstRenderStateType=type;
    state[nextState].dwArg[0]=arg;
    nextState++;
}
#else
static void 
_atrD3DRenderState(_AtrD3DDriver *ctx, D3DRENDERSTATETYPE type, DWORD arg) {
    EBufferData *ebd = &ctx->ebd;

    _atrD3DCheckBufferFull(ctx, 0, 1, sizeof(D3DSTATE));

    _atrD3DInstruction(ctx, D3DOP_STATERENDER, sizeof(D3DSTATE), 1);

    ((D3DSTATE *)ebd->lpInstPointer)->drstRenderStateType=type;
    ((D3DSTATE *)ebd->lpInstPointer)->dwArg[0]=arg;

    ebd->lpInstPointer+=sizeof(D3DSTATE);
}
#endif

#ifdef notdef
/*-------------------------------------------------------------------
  Function: _atrD3DLightStatef
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Add a lighting state instruction with a D3D value argument
  Arguments:
    ctx  - the rendering context
    type - type of instruction
    v    - value 
  Return:
    None
  -------------------------------------------------------------------*/

static void 
_atrD3DLightStatef(_AtrD3DDriver *ctx, D3DLIGHTSTATETYPE type, D3DVALUE v) {
    EBufferData *ebd = &ctx->ebd;

    _atrD3DCheckBufferFull(ctx, 0, 1, sizeof(D3DSTATE));

    _atrD3DInstruction(ctx, D3DOP_STATELIGHT, sizeof(D3DSTATE), 1);

    ((D3DSTATE *)ebd->lpInstPointer)->dlstLightStateType=type;
    ((D3DSTATE *)ebd->lpInstPointer)->dvArg[0]=v;

    ebd->lpInstPointer+=sizeof(D3DSTATE);
}

/*-------------------------------------------------------------------
  Function: _atrD3DLightState
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Add a lighting state instruction with a DWORD value argument
  Arguments:
    ctx  - the rendering context
    type - type of instruction
    v    - value 
  Return:
    None
  -------------------------------------------------------------------*/

static void 
_atrD3DLightState(_AtrD3DDriver *ctx, D3DLIGHTSTATETYPE type, DWORD arg) {
    EBufferData *ebd = &ctx->ebd;

    _atrD3DCheckBufferFull(ctx, 0, 1, sizeof(D3DSTATE));

    _atrD3DInstruction(ctx, D3DOP_STATELIGHT, sizeof(D3DSTATE), 1);

    ((D3DSTATE *)ebd->lpInstPointer)->dlstLightStateType=type;
    ((D3DSTATE *)ebd->lpInstPointer)->dwArg[0]=arg;

    ebd->lpInstPointer+=sizeof(D3DSTATE);
}
#endif

/*-------------------------------------------------------------------
  Function: _atrD3DSplash
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Render the splash screen
  Arguments:
    None
  Return:
    None
  -------------------------------------------------------------------*/

static void
_atrD3DSplash( void ) {
}

/*-------------------------------------------------------------------
  Function: _atrD3DRenderImg
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Render an image at the specified location
  Arguments:
    img     - the image to display
    screenX - location at which to display image
    screenY 
  Return:
    None
  TBD: does not support alpha blending
  -------------------------------------------------------------------*/

static FxBool 
_atrD3DRenderImg(  AtrImg *img, FxU32 screenX, FxU32 screenY ) {
    DDSURFACEDESC ddsd;
    HRESULT ddrval;
    LPDIRECTDRAWSURFACE lpDDS, lpImgSurf;
    RECT srcRect, dstRect;
	AtrD3DImg *d3dImage;
    int width, height;
    DDBLTFX ddBltFx;
    DWORD flags = DDBLT_WAIT;


    /* flush any commands already in pipe */

	_atrD3DRenderExecuteBuffer(d3dContext);

	width = ATM_MIN(img->width, _atrCurrentCanvas->xMax-screenX+1);
	height = ATM_MIN(img->height, _atrCurrentCanvas->yMax-screenY+1);

    screenY=d3dContext->any.caps.height - screenY - height;

    srcRect.left = 0;
    srcRect.top = 0;
    srcRect.right = width-1;
    srcRect.bottom = height-1;

    dstRect.left = srcRect.left+screenX;
    dstRect.top = srcRect.top+screenY;
    dstRect.right = srcRect.right+screenX;
    dstRect.bottom = srcRect.bottom+screenY;

    lpDDS = d3dContext->BackBuffer ;

    /* Get the descriptor of the rendering surface */

    if ( img->devPrivate == NULL ) { /* realize surface */
        memset(&ddsd, 0, sizeof(DDSURFACEDESC));
        ddsd.dwSize = sizeof(DDSURFACEDESC);
        ddrval = lpDDS->lpVtbl->GetSurfaceDesc(lpDDS, &ddsd);
        if (ddrval != DD_OK) {
            atuError(FXTRUE, "RenderImg: could not get surface description\n");
            return FXFALSE;
        }
    
        /* Setup the descriptor to create the surface */
    
        ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
        ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;      
        ddsd.dwHeight = img->height;
        ddsd.dwWidth = img->width;
    
        ddrval=IDirectDraw_CreateSurface(d3dapp->lpDD, &ddsd, &lpImgSurf, NULL);
        CHECK(ddrval!=DD_OK, "_atrD3DRenderImg: Could not realize image");

		if ((d3dImage = (AtrD3DImg *)malloc(sizeof(AtrD3DImg))) == NULL ) {
            atuError(FXTRUE, "_atrD3DRealizeImage: could not allocate space for image\n");
        }

        img->devPrivate = d3dImage;
        d3dImage->lpSurface = lpImgSurf;
        d3dImage->lpPalette = NULL; /* TBD: no palettes for NOW */

        if ( !_atrD3DPutImg( lpImgSurf, img)) {
            atuError(FXFALSE, "RenderImg: could not copy image data\n");
            return FXFALSE;
        }
    } else d3dImage = (AtrD3DImg *)img->devPrivate;

    ddBltFx.dwAlphaSrcConst = 0xf;

    /*  TBD: how do you unset the color key */

    if ( _atrCurrentMaterial->chromaKeyEnable ) {
        DDCOLORKEY ddck;
        memset(&ddck, 0, sizeof(ddck));
	    ddck.dwColorSpaceLowValue  = _atrCurrentMaterial->chromaKeyValue;
	    ddck.dwColorSpaceHighValue = ddck.dwColorSpaceLowValue;
        ddrval = d3dImage->lpSurface->lpVtbl->SetColorKey(d3dImage->lpSurface, 
                                                      DDCKEY_SRCBLT, &ddck);

        if (ddrval!=DD_OK ) {
		    dprintf("_atrD3DRenderImg: Could not set color key: %s\n",
			          D3DAppErrorToString(ddrval));
        }

        flags |= DDBLT_KEYSRC ;
    }

#ifndef notdef
    ddrval = lpDDS->lpVtbl->Blt(lpDDS,
                       &dstRect, d3dImage->lpSurface,
                       &srcRect, flags, NULL);
#else
    ddrval = lpDDS->lpVtbl->BltFast(lpDDS,
                       dstRect.left, dstRect.top, d3dImage->lpSurface,
                       &srcRect, /* DDBLTFAST_SRCCOLORKEY | */ DDBLT_WAIT);
#endif

    if (ddrval!=DD_OK ) {
		dprintf("_atrD3DRenderImg: Could not Blt image: %s\n",
			      D3DAppErrorToString(ddrval));
    }

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: _atrD3DGrabImg
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Grab a portion of the display surface
  Arguments:
    i       - where to put the captured image
    screenX - location at which to start capture
    screenY 
    buf     - capture front or back buffer?
  Return:
    FXTRUE if the context can currently be drawn to, FXFALSE otherwise
  -------------------------------------------------------------------*/

static void 
_atrD3DGrabImg(  AtrImg    *i,
                 FxU32     screenX,
                 FxU32     screenY,
                 AtrBuffer buf) {
    FxU16 *dst, *src, *srcLine, *dstLine;
    LPDIRECTDRAWSURFACE lpDDS;
    DDSURFACEDESC ddsd;
    size_t srcStride, dstStride;
    HRESULT ddrval;
	int x, y, width, height;

	width = d3dContext->any.caps.width - screenX ;
	height = d3dContext->any.caps.height - screenY ;

	if (( width < 0 ) || ( height < 0 ))
		return;

    lpDDS = d3dContext->BackBuffer ;

    /* Build a descriptor to lock the surface */

    memset(&ddsd, 0, sizeof(DDSURFACEDESC));
    ddsd.dwSize = sizeof(DDSURFACEDESC);
    ddrval = lpDDS->lpVtbl->Lock(lpDDS, NULL, &ddsd, 0, NULL);
    if (ddrval != DD_OK) {
        atuError(FXTRUE, "_atrD3DGrabImg: could not lock surface\n");
        return;
    }

	/* copy the data */

    srcStride = ddsd.lPitch>>1;
	dstStride = i->width;

    srcLine = ((FxU16 *)ddsd.lpSurface)+screenY*srcStride+screenX;
	dstLine = ((FxU16 *)i->data);

	for ( y = 0; y < height; y++ ) {
		src = srcLine;
		srcLine += srcStride;
		dst = dstLine;
		dstLine += dstStride;
		for ( x = 0; x < width; x++ ) {
			*dst++ = *src++;
		}
	}

    /* release the lock */

    lpDDS->lpVtbl->Unlock(lpDDS, NULL);
}

/*-------------------------------------------------------------------
  Function: _atrDrawPoint
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Draw a point
  Arguments:
    p - point to draw
  Return:
    None
  -------------------------------------------------------------------*/

static void
_atrD3DDrawPoint( const GrVertex *a ) {
#ifndef DTRIAPI
    D3DTLVERTEX *verts;
    EBufferData *ebd = &d3dContext->ebd;
    int vnum;
  
    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ZENABLE, FALSE);

    /* ensure we have enough space in buffer in the worst case we need space 
       for the vertices, and instructions for PROCESSVERTICES, QWORD ALIGN, 
       LINE DATA, EXIT */ 
	
    _atrD3DCheckBufferFull(d3dContext, 1, 4, 
                             sizeof(D3DPROCESSVERTICES) + sizeof(D3DPOINT));

    verts = (D3DTLVERTEX *)(ebd->lpVertexPointer);
    vnum = ebd->numVertices;

    /* TBD: figure out a better z scaling factor. this may be trouble */
    
    verts[0].sx=a->x;
    if (d3dContext->FlipY)
         verts[0].sy=d3dContext->any.caps.height - a->y;
    else verts[0].sy=a->y;
    verts[0].sz=a->ooz;
    verts[0].dvRHW=a->tmuvtx[0].oow;
    verts[0].color=RGBA_MAKE((int)a->r,(int)a->g,(int)a->b, (int)a->a);
    verts[0].specular=RGB_MAKE(0,0,0);

    verts[0].tu=a->tmuvtx[0].sow;
    verts[0].tv=a->tmuvtx[0].tow;
  
    _atrD3DProcessVertices(d3dContext, 
                D3DPROCESSVERTICES_COPY | D3DPROCESSVERTICES_UPDATEEXTENTS, 1);

    /*
     * Make sure that the point data (not OP) will be QWORD aligned
     * TBD: why is this important
     */

    /* ensure its not quad aligned */

    _atrD3DCheckAlignment(d3dContext);

    _atrD3DInstruction(d3dContext, D3DOP_POINT, sizeof(D3DPOINT), 1);

    ((D3DPOINT *)ebd->lpInstPointer)->wCount=1;
    ((D3DPOINT *)ebd->lpInstPointer)->wFirst=vnum;
    ebd->lpInstPointer+=sizeof(D3DPOINT);

    if ( zEnable )
        _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ZENABLE, zEnable);
#endif /* DTRIAPI */
}

/*-------------------------------------------------------------------
  Function: _atrDrawLine
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Draw a line
  Arguments:
    a - vertices defining line
    b
  Return:
    None
  -------------------------------------------------------------------*/

static void
_atrD3DDrawLine( const GrVertex *a, const GrVertex *b ) {
#ifndef DTRIAPI
    D3DTLVERTEX *verts;
    EBufferData *ebd = &d3dContext->ebd;
    int vnum;
  
        _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ZENABLE, FALSE);

    /* ensure we have enough space in buffer in the worst case we need space 
       for the vertices, and instructions for PROCESSVERTICES, QWORD ALIGN, 
       LINE DATA, EXIT */ 
	
    _atrD3DCheckBufferFull(d3dContext, 2, 4, 
                             sizeof(D3DPROCESSVERTICES) + sizeof(D3DLINE));

    verts = (D3DTLVERTEX *)(ebd->lpVertexPointer);
    vnum = ebd->numVertices;

    /* TBD: figure out a better z scaling factor. this may be trouble */
    
    verts[0].sx=a->x;
    if (d3dContext->FlipY)
         verts[0].sy=d3dContext->any.caps.height - a->y;
    else verts[0].sy=a->y;
    verts[0].sz=a->ooz;
    verts[0].dvRHW=a->tmuvtx[0].oow;
    verts[0].color=RGBA_MAKE((int)a->r,(int)a->g,(int)a->b, (int)a->a);
    verts[0].specular=RGB_MAKE(0,0,0);

    verts[0].tu=a->tmuvtx[0].sow;
    verts[0].tv=a->tmuvtx[0].tow;
  
    verts[1].sx=b->x;
    if (d3dContext->FlipY)
         verts[1].sy=d3dContext->any.caps.height - b->y;
    else verts[1].sy=b->y;
    verts[1].sz=b->ooz;
    verts[1].dvRHW=b->tmuvtx[0].oow;
    verts[1].color=RGBA_MAKE((int)b->r,(int)b->g,(int)b->b, (int)a->a);
    verts[1].specular=RGB_MAKE(0,0,0);
    verts[1].tu=b->tmuvtx[0].sow;
    verts[1].tv=b->tmuvtx[0].tow;
  
    _atrD3DProcessVertices(d3dContext, 
                D3DPROCESSVERTICES_COPY | D3DPROCESSVERTICES_UPDATEEXTENTS, 2);

    /*
     * Make sure that the line data (not OP) will be QWORD aligned
     * TBD: why is this important
     */

    /* ensure its not quad aligned */

    _atrD3DCheckAlignment(d3dContext);

    _atrD3DInstruction(d3dContext, D3DOP_LINE, sizeof(D3DLINE), 1);

    ((D3DLINE *)ebd->lpInstPointer)->v1=vnum;
    ((D3DLINE *)ebd->lpInstPointer)->v2=vnum+1;
    ebd->lpInstPointer+=sizeof(D3DLINE);

    if ( zEnable )
        _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ZENABLE, zEnable);
#endif /* DTRIAPI */
}

/*-------------------------------------------------------------------
  Function: _atrD3DDrawTriangle
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Draw a triangle
  Arguments:
    a  - vertices defining triangle
    b  
    c  
  Return:
    None
  -------------------------------------------------------------------*/

#ifdef DTRIAPI
#ifndef AT_PENTIUM_ASSEMBLY
void
_atrD3DDrawTriangle(const GrVertex *a, const GrVertex *b,
                            const GrVertex *c) {
    D3DTLVERTEX verts[3];
    D3DTRIANGLE tri;
  
    _atrD3DFlushState();

    verts[0].sx=a->x;
    verts[0].sy=d3dContext->any.caps.height - a->y;
    verts[0].sz=a->ooz;
    verts[0].dvRHW=a->tmuvtx[0].oow;
    verts[0].color=RGBA_MAKE((int)a->r,(int)a->g,(int)a->b, (int)a->a);
    verts[0].specular=0;
    verts[0].tu=a->tmuvtx[0].sow;
    verts[0].tv=a->tmuvtx[0].tow;
  
    verts[1].sx=b->x;
    verts[1].sy=d3dContext->any.caps.height - b->y;
    verts[1].sz=b->ooz;
    verts[1].dvRHW=b->tmuvtx[0].oow;
    verts[1].color=RGBA_MAKE((int)b->r,(int)b->g,(int)b->b, (int)b->a);
    verts[1].specular=0;
    verts[1].tu=b->tmuvtx[0].sow;
    verts[1].tv=b->tmuvtx[0].tow;
  
    verts[2].sx=c->x;
    verts[2].sy=d3dContext->any.caps.height - c->y;
    verts[2].sz=c->ooz;
    verts[2].dvRHW=c->tmuvtx[0].oow;
    verts[2].color=RGBA_MAKE((int)c->r,(int)c->g,(int)c->b, (int)c->a);
    verts[2].specular=0;
    verts[2].tu=c->tmuvtx[0].sow;
    verts[2].tv=c->tmuvtx[0].tow;

    tri.v1 = 0;
    tri.v2 = 1;
    tri.v3 = 2;
    tri.wFlags= D3DTRIFLAG_EDGEENABLETRIANGLE;

	LastError = d3dContext->lpDev->lpVtbl->DrawTriangle(d3dContext->lpDev, 
                                         d3dContext->lpView, 
                                         verts, 3, &tri, 1, 
                                         D3DTRIFLAG_EDGEENABLETRIANGLE);
}
#endif
#else
void _atrD3DDrawTriangle(const GrVertex *a, const GrVertex *b,
                            const GrVertex *c) {
    D3DTLVERTEX *verts;
    EBufferData *ebd = &d3dContext->ebd;
    int vnum;
  
    /* ensure we have enough space in buffer in the worst case we need space 
       for the vertices, and instructions for PROCESSVERTICES, QWORD ALIGN, 
       TRIANGLE DATA, EXIT */ 

	
    _atrD3DCheckBufferFull(d3dContext, 3, 4, 
                             sizeof(D3DPROCESSVERTICES) + sizeof(D3DTRIANGLE));

    verts = (D3DTLVERTEX *)(ebd->lpVertexPointer);
    vnum = ebd->numVertices;

    /* TBD: figure out a better z scaling factor. this may be trouble */
    
    verts[0].sx=a->x;
    verts[0].sy=d3dContext->any.caps.height - a->y;
    verts[0].sz=a->ooz;
    verts[0].dvRHW=a->tmuvtx[0].oow;
    verts[0].color=RGBA_MAKE((int)a->r,(int)a->g,(int)a->b, (int)a->a);
    verts[0].specular=0;
    verts[0].tu=a->tmuvtx[0].sow;
    verts[0].tv=a->tmuvtx[0].tow;
  
    verts[1].sx=b->x;
    verts[1].sy=d3dContext->any.caps.height - b->y;
    verts[1].sz=b->ooz;
    verts[1].dvRHW=b->tmuvtx[0].oow;
    verts[1].color=RGBA_MAKE((int)b->r,(int)b->g,(int)b->b, (int)b->a);
    verts[1].specular=0;
    verts[1].tu=b->tmuvtx[0].sow;
    verts[1].tv=b->tmuvtx[0].tow;
  
    verts[2].sx=c->x;
    verts[2].sy=d3dContext->any.caps.height - c->y;
    verts[2].sz=c->ooz;
    verts[2].dvRHW=c->tmuvtx[0].oow;
    verts[2].color=RGBA_MAKE((int)c->r,(int)c->g,(int)c->b, (int)c->a);
    verts[2].specular=0;
    verts[2].tu=c->tmuvtx[0].sow;
    verts[2].tv=c->tmuvtx[0].tow;

#ifdef notdef
    for ( i = 0 ; i < 3; i++ ) {
        if (( verts[i].sx < 0.0f ) || ( verts[i].sy < 0.0f ) ) {
	        OutputDebugStr("Negative coordinates\n");
			return;
        }
    }
#endif

    _atrD3DProcessVertices(d3dContext, 
                D3DPROCESSVERTICES_COPY | D3DPROCESSVERTICES_UPDATEEXTENTS, 3);

    /*
     * Make sure that the triangle data (not OP) will be QWORD aligned
     * TBD: why is this important
     */

    /* ensure its not quad aligned */

    _atrD3DCheckAlignment(d3dContext);

    _atrD3DInstruction(d3dContext, D3DOP_TRIANGLE, sizeof(D3DTRIANGLE), 1);

    ((D3DTRIANGLE *)ebd->lpInstPointer)->v1=vnum;
    ((D3DTRIANGLE *)ebd->lpInstPointer)->v2=vnum+1;
    ((D3DTRIANGLE *)ebd->lpInstPointer)->v3=vnum+2;
    ((D3DTRIANGLE *)ebd->lpInstPointer)->wFlags= D3DTRIFLAG_EDGEENABLETRIANGLE;
    ebd->lpInstPointer+=sizeof(D3DTRIANGLE);
}
#endif /* DTRIAPI */

/*-------------------------------------------------------------------
  Function: _atrD3DClearCanvas
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Clear the current canvas to the specified color and z value
  Arguments:
    r  - color to clear to
    g
    b 
    z  - z value to clear to
  Return:
    None
  -------------------------------------------------------------------*/

static void
_atrD3DClearCanvas( float r, float g, float b, FxU16 za ) {
    D3DRECT dummy;

    if ( ( r != d3dContext->bmat.diffuse.r ) ||
         ( g != d3dContext->bmat.diffuse.g ) ||
         ( b != d3dContext->bmat.diffuse.b ) ) {
        memset(&d3dContext->bmat, 0, sizeof(D3DMATERIAL));
        d3dContext->bmat.dwSize = sizeof(D3DMATERIAL);
        d3dContext->bmat.diffuse.r = (D3DVALUE)r;
        d3dContext->bmat.diffuse.g = (D3DVALUE)g;
        d3dContext->bmat.diffuse.b = (D3DVALUE)b;
        d3dContext->lpBmat->lpVtbl->SetMaterial(
                                         d3dContext->lpBmat, &d3dContext->bmat);

        d3dContext->lpBmat->lpVtbl->GetHandle(d3dContext->lpBmat, 
                                       d3dapp->lpD3DDevice, &d3dContext->hBmat);
        d3dContext->lpView->lpVtbl->SetBackground(d3dContext->lpView, 
                                                  d3dContext->hBmat);
    }

    dummy.x1 = _atrCurrentCanvas->xMin;
    dummy.y1 = d3dContext->any.caps.height-_atrCurrentCanvas->yMax-1;
    dummy.x2 = _atrCurrentCanvas->xMax;
    dummy.y2 = d3dContext->any.caps.height-_atrCurrentCanvas->yMin-1;
    d3dappi.lpD3DViewport->lpVtbl->Clear(d3dappi.lpD3DViewport,
                                         1, &dummy, 
                                         D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER);
}

/*-------------------------------------------------------------------
  Function: _atrD3DSwapBuffer
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Swaap front and back buffers
  Arguments:
    sync - sync to screen refresh
  Return:
    None
  -------------------------------------------------------------------*/

static void
_atrD3DSwapBuffer( FxU32 sync ) {
    D3DAppShowBackBuffer(D3DAPP_SHOWALL); /* may want to do a partial update */
}

/*-------------------------------------------------------------------
  Function: _atrD3DGetPerfStats
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Get performance statistics
  Arguments:
    pStats - where to put statistics
  Return:
    None
  -------------------------------------------------------------------*/

static void
_atrD3DGetPerfStats(GrSstPerfStats_t *pStats) {
}

/*-------------------------------------------------------------------
  Function: _atrD3DResetPerfStats
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Reset performance statistics
  Arguments:
    None
  Return:
    None
  -------------------------------------------------------------------*/

static void 
_atrD3DResetPerfStats(void) {
}

static const D3DBLEND DestTranslation[]={
    D3DBLEND_ZERO,/* GR_BLEND_ZERO 0x0 */
    D3DBLEND_SRCALPHA,/* GR_BLEND_SRC_ALPHA 0x1 */
    D3DBLEND_SRCCOLOR,/* GR_BLEND_SRC_COLOR GR_BLEND_DST_COLOR 0x2 */
    D3DBLEND_DESTALPHA,/* GR_BLEND_DST_ALPHA 0x3 */
    D3DBLEND_ONE,/* GR_BLEND_ONE 0x4 */
    D3DBLEND_INVSRCALPHA,/* GR_BLEND_ONE_MINUS_SRC_ALPHA 0x5 */
    D3DBLEND_INVSRCCOLOR,
    /* GR_BLEND_ONE_MINUS_SRC_COLOR GR_BLEND_ONE_MINUS_DST_COLOR 0x6 */
    D3DBLEND_INVDESTALPHA,/* GR_BLEND_ONE_MINUS_DST_ALPHA 0x7 */
    D3DBLEND_ZERO,/* reserved 0x8 */
    D3DBLEND_ZERO,/* reserved 0x9 */
    D3DBLEND_ZERO,/* reserved 0xa */
    D3DBLEND_ZERO,/* reserved 0xb */
    D3DBLEND_ZERO,/* reserved 0xc */
    D3DBLEND_ZERO,/* reserved 0xd */
    D3DBLEND_SRCALPHASAT/*GR_BLEND_ALPHA_SATURATE GR_BLEND_PREFOG_COLOR 0xf*/};

static const D3DBLEND SrcTranslation[]={
    D3DBLEND_ZERO,/* GR_BLEND_ZERO 0x0 */
    D3DBLEND_SRCALPHA,/* GR_BLEND_SRC_ALPHA 0x1 */
    D3DBLEND_DESTCOLOR,/* GR_BLEND_SRC_COLOR GR_BLEND_DST_COLOR 0x2 */
    D3DBLEND_DESTALPHA,/* GR_BLEND_DST_ALPHA 0x3 */
    D3DBLEND_ONE,/* GR_BLEND_ONE 0x4 */
    D3DBLEND_INVSRCALPHA,/* GR_BLEND_ONE_MINUS_SRC_ALPHA 0x5 */
    D3DBLEND_INVDESTCOLOR,
    /* GR_BLEND_ONE_MINUS_SRC_COLOR GR_BLEND_ONE_MINUS_DST_COLOR 0x6 */
    D3DBLEND_INVDESTALPHA,/* GR_BLEND_ONE_MINUS_DST_ALPHA 0x7 */
    D3DBLEND_ZERO,/* reserved 0x8 */
    D3DBLEND_ZERO,/* reserved 0x9 */
    D3DBLEND_ZERO,/* reserved 0xa */
    D3DBLEND_ZERO,/* reserved 0xb */
    D3DBLEND_ZERO,/* reserved 0xc */
    D3DBLEND_ZERO,/* reserved 0xd */
    D3DBLEND_SRCALPHASAT/*GR_BLEND_ALPHA_SATURATE GR_BLEND_PREFOG_COLOR 0xf*/};
  

/*-------------------------------------------------------------------
  Function: _atrD3DClipWindow
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Set current clipping window
  Arguments:
    minx - the new region to clip to
    miny
    maxx
    maxy
  Return:
    None
  -------------------------------------------------------------------*/

static void
_atrD3DClipWindow( int minx, int miny, int maxx, int maxy ) {
  /* TBD: when DirectDraw clipper is working set it up here */

  d3dContext->clip_minx=minx;
  d3dContext->clip_maxx=maxx;
  if(d3dContext->FlipY)
    {
      d3dContext->clip_miny=d3dContext->any.caps.height-maxy;
      d3dContext->clip_maxy=d3dContext->any.caps.height-miny;
    }
  else
    {
      d3dContext->clip_miny=miny;
      d3dContext->clip_maxy=maxy;  
    }
  
}

/*-------------------------------------------------------------------
  Function: atrIsDrawable
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Determine if a context can currently be drawn to
  Arguments:
    ctx - the rendering context
  Return:
    FXTRUE if the context can currently be drawn to, FXFALSE otherwise
  -------------------------------------------------------------------*/

static void
_atrDepthBufferFunction( GrCmpFnc_t function ) {
  /*The D3D values are off by one*/
  function=function-1;

  _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ZFUNC, function);
}

static void
_atrDepthBufferMode( GrDepthBufferMode_t mode ) {
  if(mode==GR_DEPTHBUFFER_DISABLE)
    zEnable=FALSE;
  
  _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ZENABLE, zEnable);
}

static void
_atrDepthMask( FxBool mask ) {
  _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ZWRITEENABLE, mask);
}

/*-------------------------------------------------------------------
  Function: _atrD3DTexEntryInit
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Device specific initialization of a texture handle
  Arguments:
      e - texture handle to initialize
  Return:
      Nothing
  -------------------------------------------------------------------*/

static void 
_atrD3DTexEntryInit( _AtrTexEntry *entry ) {
    _AtrD3DTexEntry *d3dentry = (_AtrD3DTexEntry *)entry;

    d3dentry->texHandle=(unsigned)HANDLE_MISS;
	d3dentry->lpPalette = NULL;
	d3dentry->smSurf = NULL;
	d3dentry->smTexture = NULL;
	d3dentry->vmSurf = NULL;
	d3dentry->vmTexture = NULL;
}

/*-------------------------------------------------------------------
  Function: _atrD3DUnrealizeImg
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Release the device dependent image data
  Arguments:
      i        - pointer to image structure
  Return:
      FXTRUE on success, FXFALSE otherwise
  -------------------------------------------------------------------*/

static FxBool 
_atrD3DUnrealizeImg( AtrImg *i ) {
    AtrD3DImg *d3dImage = (AtrD3DImg *)(i->devPrivate);

    RELEASE(d3dImage->lpSurface );
    RELEASE(d3dImage->lpTexture );
    RELEASE(d3dImage->lpPalette );

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: _atrD3DCloneImg
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Make a new copy of an image
  Arguments:
      dst        - pointer to target image structure
      src        - pointer to source image structure
  Return:
      FXTRUE on success, FXFALSE otherwise
  TBD:
      Implement
  -------------------------------------------------------------------*/

FxBool _atrD3DCloneImg( AtrImg *dst, const AtrImg *src ) {
    atuError(FXTRUE, "_atrD3DCloneImg not implemented yet\n");

    return FXTRUE;
}

static FxBool 
SetTextureData(_AtrD3DTexEntry *d3dentry ) {
    HRESULT rval;

    /* set the palette if there is one */

    if (d3dentry->lpPalette != NULL) {
        LastError = d3dentry->vmSurf->lpVtbl->SetPalette(d3dentry->vmSurf, 
                             d3dentry->lpPalette);
        if (LastError != DD_OK) {
            D3DAppISetErrorString(
                    "Failed to set the destination texture's palette.\n%s",
                    D3DAppErrorToString(LastError));
            return FXFALSE;
        }
    }
    
    /*
     * Load the source texture into the destination.  During this call, a
     * driver could compress or reformat the texture surface and put it in
     * video memory.
     */
    
    rval=IDirect3DTexture_Load(d3dentry->vmTexture, d3dentry->smTexture);
    CHECK(rval!=DD_OK, 
                    "Could not transfer texture from system to video mem");
       
    return TRUE;
}

/*-------------------------------------------------------------------
  Function: _atrD3DTexAssociate
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Load an image from a file
  Arguments:
      handle   - handle to texture
      img      - image to associate with handle
  Return:
  FXTRUE on success, FXFALSE otherwise
  -------------------------------------------------------------------*/

static FxBool 
_atrD3DTexAssociate( AtrTexHandle handle, AtrImg *img ) {
    AtrD3DImg *d3dImage = (AtrD3DImg *)(img->devPrivate);
    _AtrD3DTexEntry *d3dentry = (_AtrD3DTexEntry *)handle;
    DDSURFACEDESC srcDesc, dstDesc;
    HRESULT rval;

    /* is this the first image to be associated with this texture? */

    d3dentry->smSurf = d3dImage->lpSurface;
    d3dentry->smTexture = d3dImage->lpTexture;
    d3dentry->lpPalette = d3dImage->lpPalette;

    if ( d3dentry->vmSurf == NULL ) {
        return FXTRUE;
    } 

    /* we have previously associated a texture with this image */

    memset(&srcDesc, 0, sizeof(DDSURFACEDESC));
    srcDesc.dwSize=sizeof(DDSURFACEDESC);
    rval=IDirectDrawSurface_GetSurfaceDesc(d3dentry->smSurf, &srcDesc);
    CHECK(rval!=DD_OK, "Could not get source surface descriptor");

    memset(&dstDesc, 0, sizeof(DDSURFACEDESC));
    dstDesc.dwSize=sizeof(DDSURFACEDESC);
    rval=IDirectDrawSurface_GetSurfaceDesc(d3dentry->vmSurf, &dstDesc);
    CHECK(rval!=DD_OK, "Could not get destination surface descriptor");

    if (( srcDesc.dwHeight != dstDesc.dwHeight ) ||
        ( srcDesc.dwWidth != dstDesc.dwWidth ) ||
        ( srcDesc.dwMipMapCount != dstDesc.dwMipMapCount ) ||
        ( !SetTextureData(d3dentry ))) {
        RELEASE(d3dentry->vmTexture);
        RELEASE(d3dentry->vmSurf);
        d3dentry->texHandle=(unsigned)HANDLE_MISS;
        d3dentry->any.state = ATR_TEXENT_ALLOCATED;
    }

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: _atrD3DTexSource
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Make texture current
  Arguments:
      entry    - handle to texture
      mask     - which mip maps to download
  Return:
      Nothing
  -------------------------------------------------------------------*/

static void 
_atrD3DTexSource( _AtrTexEntry *entry, FxU32 mask ) {
    _AtrD3DTexEntry *d3dentry = (_AtrD3DTexEntry *)entry;

	if ( d3dentry->vmSurf == NULL ) {
		atuError(FXTRUE, "TexSource: texture not in memory\n");
	}

    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_TEXTUREHANDLE,
		           d3dentry->texHandle);  
}

/*-------------------------------------------------------------------
  Function: _atrD3DTramAllocate
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Allocate texture memory for this object
  Arguments:
      entry    - handle to desired texture
  Return:
      Nothing
  -------------------------------------------------------------------*/

static FxBool 
_atrD3DTramAllocate( _AtrTexEntry *entry ) {
    _AtrD3DTexEntry *d3dentry = (_AtrD3DTexEntry *)entry;
    DDSURFACEDESC ddsd;
    HRESULT rval;

    memset(&ddsd, 0, sizeof(DDSURFACEDESC));
    ddsd.dwSize=sizeof(DDSURFACEDESC);

    rval=IDirectDrawSurface_GetSurfaceDesc(d3dentry->smSurf, &ddsd);
    CHECK_NO_RETURN(rval!=DD_OK, "Could not get surface descriptor");

    /*
     * Create an empty texture surface in video memory to load the source 
     * texture into.
     *
     * The DDSCAPS_ALLOCONLOAD flag allows the DD driver to wait until the
     * load call to allocate the texture in memory because at this point,
     * we may not know how much memory the texture will take up (e.g. it
     * could be compressed to an unknown size in video memory).
     */

    if (ddsd.ddsCaps.dwCaps & DDSCAPS_MIPMAP) {
	    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	    ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | 
	                          DDSCAPS_ALLOCONLOAD | DDSCAPS_MIPMAP | 
                              DDSCAPS_COMPLEX;
    } else {
        ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	    ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_ALLOCONLOAD;
    }

    if ( d3dapp->ThisDriver.bIsHardware )
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;

    while ( d3dentry->vmSurf == NULL ) {
        _AtrD3DTexEntry *tmp ;

        rval=IDirectDraw_CreateSurface(d3dapp->lpDD, &ddsd, 
                                       &d3dentry->vmSurf,NULL);
        if ( d3dentry->vmSurf != NULL )
            break;

        tmp = (_AtrD3DTexEntry *)_inCache[entry->atrInfo.tmu];
        RELEASE(tmp->vmTexture); tmp->vmTexture = NULL;
        RELEASE(tmp->vmSurf); tmp->vmSurf = NULL;
        tmp->texHandle=(unsigned)HANDLE_MISS;
        _atrTexPunt( _inCache[entry->atrInfo.tmu] );
    }
    
    rval=IDirectDrawSurface_QueryInterface(d3dentry->vmSurf,
	                      &IID_IDirect3DTexture, (void *)&d3dentry->vmTexture);

    CHECK(rval!=DD_OK, "Could not Query for video memory texture");

    /* Get the handle for the texture */

    rval=IDirect3DTexture_GetHandle(d3dentry->vmTexture, 
                                    d3dapp->lpD3DDevice, 
                                    &d3dentry->texHandle);

    CHECK(rval!=DD_OK, "Could not get a texture handle");

    return SetTextureData( d3dentry );
}

/*-------------------------------------------------------------------
  Function: _atrD3DTexPunt
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Free texture cache resources used by this texture
  Arguments:
      entry    - handle to desired texture
  Return:
      Nothing
  -------------------------------------------------------------------*/

static void 
_atrD3DTexPunt( _AtrTexEntry *entry ) {
    _AtrD3DTexEntry *d3dentry = (_AtrD3DTexEntry *)entry;

    if ( d3dentry->vmSurf != NULL )
        IDirectDrawSurface_Release(d3dentry->vmSurf);

    if ( d3dentry->vmTexture != NULL )
        IDirect3DTexture_Release(d3dentry->vmTexture);
}

/*-------------------------------------------------------------------
  Function: _atrD3DTexDeleteHandle
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Delete the resources used by this texture handle
  Arguments:
      entry    - handle to desired texture
  Return:
      Nothing
  -------------------------------------------------------------------*/

static void 
_atrD3DTexDeleteHandle( _AtrTexEntry *entry) {
    _atrD3DTexPunt( entry );
}

/*-------------------------------------------------------------------
  Function: _atrD3DUpdateEnv
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Set the current environment
  Arguments:
      e        - new environment
  Return:
      Nothing
  -------------------------------------------------------------------*/

/* mapping from ATB values to supported D3D values */

DWORD _atrD3DFogModes[3] = {
                             D3DFOG_LINEAR,
                             D3DFOG_EXP,
                             D3DFOG_EXP2
                           } ;
static void 
_atrD3DUpdateEnv( AtrEnv *e ) {
    /* set fog */

    if ( e->flags & ATR_FOG_ON_DEPTH ) {
        DWORD r = (DWORD)(255.0f*e->fogColor.r),
              g = (DWORD)(255.0f*e->fogColor.g), 
              b = (DWORD)(255.0f*e->fogColor.b);

		_atrD3DRenderState(d3dContext, D3DRENDERSTATE_FOGENABLE, TRUE);
		_atrD3DRenderState(d3dContext, D3DRENDERSTATE_FOGCOLOR, 
                                       RGB_MAKE(r, g, b));
		_atrD3DRenderState(d3dContext, D3DRENDERSTATE_FOGTABLEMODE, 
                                      _atrD3DFogModes[e->fogFunc]);
		_atrD3DRenderStatef(d3dContext, D3DRENDERSTATE_FOGTABLESTART, 
                                        D3DVAL(e->fogNearW));
		_atrD3DRenderStatef(d3dContext, D3DRENDERSTATE_FOGTABLEEND, 
                                        D3DVAL(e->fogFarW));
		_atrD3DRenderStatef(d3dContext, D3DRENDERSTATE_FOGTABLEDENSITY, 
                                        D3DVAL(e->fogDensity));
    } else {
		_atrD3DRenderState(d3dContext, D3DRENDERSTATE_FOGENABLE, FALSE);
    } 

    /* hidden surface processing, 
       TBD: ATB does not support Z buffer at the api level yet */

    if ( _atrDriver->caps.hasDepth ) {
        switch ( e->flags & ATR_HSR_MASK ) {
        case ATR_HSR_NONE:
		    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ZENABLE, FALSE);
		    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ZWRITEENABLE, FALSE);
            break;
        case ATR_HSR_WBUFFER:
		    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ZENABLE, TRUE);
		    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ZWRITEENABLE, TRUE);
		    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ZFUNC, 
                                           D3DCMP_LESSEQUAL);
            break;
        default:
            break;
        }
    }
}

/*-------------------------------------------------------------------
  Function: _atrD3DInitFilterModes
  Date: 10/15
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Determine mapping from ATB filter attributes to D3D. At some point
    we should examine:

        d3dapp->ThisDriver->Desc->dpcTriCaps.dwTextureFilterCaps

    to determine what this device can really do. For now just map the
    attrubutes.
  Arguments:
      None
  Return:
      Nothing
  -------------------------------------------------------------------*/

static DWORD _atrD3DTexFilter[ATR_NUM_MMODES*ATR_NUM_FILTER_MODES];

#define TEX_FILTER(_f, _mm) _atrD3DTexFilter[ATR_NUM_MMODES*(_f)+(_mm)]

static void
_atrD3DInitFilterModes(void) {
    int filter, mmode; 
 
    for ( filter = 0; filter < ATR_NUM_FILTER_MODES; filter++ ) {
        for ( mmode = 0; mmode < ATR_NUM_MMODES; mmode++ ) {
            switch (filter) {
            case ATR_TEXFILTER_POINT_SAMPLED:
                switch ( mmode ) {
                case ATR_TEXMIPMAP_DISABLE:
                    TEX_FILTER(filter, mmode) = D3DFILTER_NEAREST;
                    break;
                case ATR_TEXMIPMAP_NEAREST:
                case ATR_TEXMIPMAP_NEAREST_DITHER:
                    TEX_FILTER(filter, mmode) = D3DFILTER_MIPNEAREST;
                    break;
                }
                break;
            case ATR_TEXFILTER_BILINEAR:
                switch ( mmode ) {
                case ATR_TEXMIPMAP_DISABLE:
                    TEX_FILTER(filter, mmode) = D3DFILTER_LINEAR;
                    break;
                case ATR_TEXMIPMAP_NEAREST:
                case ATR_TEXMIPMAP_NEAREST_DITHER:
                    TEX_FILTER(filter, mmode) = D3DFILTER_MIPLINEAR;
                    break;
                }
                break;
           }
       }
    }
}

/*-------------------------------------------------------------------
  Function: _atrD3DRenderBuffer
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
	Specify which buffer to render into (fron or back)
  Arguments:
	buf - buffer to be used for rendering
  Return:
	None
  -------------------------------------------------------------------*/

static void 
_atrD3DRenderBuffer( AtrBuffer buf ) {
    d3dContext->renderBuffer = buf;
}

/*-------------------------------------------------------------------
  Function: _atrD3DUpdateMaterial
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Set the current material
  Arguments:
	  m        - new material
  Return:
	  Nothing
  TBD: 3DFX D3D  does not appear to honour mipmaping?
  -------------------------------------------------------------------*/

DWORD alphaFunc[] = {
					  D3DCMP_NEVER,        /* ATR_CMP_NEVER    0x0 */
					  D3DCMP_LESS,         /* ATR_CMP_LESS     0x1 */
					  D3DCMP_EQUAL,        /* ATR_CMP_EQUAL    0x2 */
					  D3DCMP_LESSEQUAL,    /* ATR_CMP_LEQUAL   0x3 */
					  D3DCMP_GREATER,      /* ATR_CMP_GREATER  0x4 */
					  D3DCMP_NOTEQUAL,     /* ATR_CMP_NOTEQUAL 0x5 */
					  D3DCMP_GREATEREQUAL, /* ATR_CMP_GEQUAL   0x6 */
					  D3DCMP_ALWAYS,       /* ATR_CMP_ALWAYS   0x7 */
					};

static void 
_atrD3DUpdateMaterial( AtrMaterial *m ) {
	DWORD blendCaps = d3dapp->ThisDriver.Desc.dpcTriCaps.dwTextureBlendCaps;

    m->sysFlags = 0;

    switch( m->irgbSrc ) {    
        case ATR_IRGBSRC_LIGHTING:
            /* Set up IRGBSRC callback */
            _atrRenderCache->irgbSrcFunc = _atrIRGBSRC_LIGHTING;
            /* Puzzle out appropriate lighting equation */
            m->sysFlags |= ATR_LIGHTFUNC_DIFFUSE;
            if ( FLOAT_BITS( m->specular.r ) +
                 FLOAT_BITS( m->specular.g ) +
                 FLOAT_BITS( m->specular.b ) != 0 ) 
                m->sysFlags |= ATR_LIGHTFUNC_SPECULAR;
            /* Update lighting caches */
            _atrUpdateLights( m );
            break;
        case ATR_IRGBSRC_STATIC:
            /* Set up IRGBSRC callback */
            _atrRenderCache->irgbSrcFunc = _atrIRGBSRC_STATIC;
            break;
        default:
            /* Set up IRGBSRC callback */
            _atrRenderCache->irgbSrcFunc = 0;
            break;
    }

    switch( m->iaSrc ) {
      case ATR_IASRC_STATIC:
        _atrRenderCache->iaSrcFunc = _atrIASRC_STATIC;
          break;
      default:
        _atrRenderCache->iaSrcFunc = 0;
        break;
    }

    switch( m->tcSrc[0] ) {
      case ATR_TCSRC_TC0:
        _atrRenderCache->texCoordSrcFunc[0] = _atrD3D_TCSRC0_TC0;
        break;
      case ATR_TCSRC_TC1:
        _atrRenderCache->texCoordSrcFunc[0] = _atrD3D_TCSRC0_TC1;
        break;
      case ATR_TCSRC_TC2:
        _atrRenderCache->texCoordSrcFunc[0] = _atrD3D_TCSRC0_TC2;
        break;
      case ATR_TCSRC_EMAP:
        _atrRenderCache->texCoordSrcFunc[0] = _atrD3D_TCSRC0_EMAP;
        break;
      case ATR_TCSRC_LMAP:
        _atrRenderCache->texCoordSrcFunc[0] = 0;
        break;
      case ATR_TCSRC_PROJECTED:
        _atrRenderCache->texCoordSrcFunc[0] = _atrD3D_TCSRC0_PROJECTED;
        break;
      case ATR_TCSRC_PLANAR:
        _atrRenderCache->texCoordSrcFunc[0] = _atrD3D_TCSRC0_PLANAR;
        break;
      default:
        _atrRenderCache->texCoordSrcFunc[0] = 0;
        break;
    }
    switch( m->tcSrc[1] ) {
      case ATR_TCSRC_TC0:
        _atrRenderCache->texCoordSrcFunc[1] = _atrD3D_TCSRC1_TC0;
        break;
      case ATR_TCSRC_TC1:
        _atrRenderCache->texCoordSrcFunc[1] = _atrD3D_TCSRC1_TC1;
        break;
      case ATR_TCSRC_TC2:
        _atrRenderCache->texCoordSrcFunc[1] = _atrD3D_TCSRC1_TC2;
        break;
      case ATR_TCSRC_EMAP:
        _atrRenderCache->texCoordSrcFunc[1] = _atrD3D_TCSRC1_EMAP;
        break;
      case ATR_TCSRC_LMAP:
        _atrRenderCache->texCoordSrcFunc[1] = 0;
        break;
      case ATR_TCSRC_PROJECTED:
        _atrRenderCache->texCoordSrcFunc[1] = _atrD3D_TCSRC1_PROJECTED;
        break;
      case ATR_TCSRC_PLANAR:
        _atrRenderCache->texCoordSrcFunc[1] = _atrD3D_TCSRC1_PLANAR;
        break;
      default:
        _atrRenderCache->texCoordSrcFunc[1] = 0;
        break;
    }

    /* Set material flags */
    m->sysFlags |= m->texSrc[0] | (m->texSrc[1]<<ATR_TEX1SHIFT);
	d3dContext->TexturesOn = ( m->texSrc[0] != ATR_TEXSRC_NONE );

	switch( m->texSrc[0] ) {
	  case ATR_TEXSRC_DECAL:
	  case ATR_TEXSRC_EMAP:
#ifdef AT_DEBUGGING
		if ( m->texture[0] == 0 ) 
		    atuError( FXTRUE, "atrPushMaterial(): No texture to source.\n" ); 
			if (_atrTexTmuFromHandle( m->texture[0] ) == ATR_TEXELFX_1)
				  atuError( FXTRUE,
						   "atrPushMaterial(): Texture handle mismatch, the ->texture[0] allocated\n"
						   "on TEXELFX 1\n" ); 
#endif
		atrTexSource( m->texture[0] );
		break;
	  case ATR_TEXSRC_DETAIL:
#ifdef AT_DEBUGGING
		if ( m->texture[0] == 0 ) 
			  atuError( FXTRUE, "atrPushMaterial(): No texture to source.\n" ); 
			  atuError( FXTRUE,
						   "atrPushMaterial(): D3D does mot support detail textures\n");
#endif
		atrTexSource( m->texture[0] );
		break;
	  case ATR_TEXSRC_PROJECTED:
#ifdef AT_DEBUGGING
		if ( m->texture[0] == 0 ) 
		  atuError( FXTRUE, "atrPushMaterial(): No texture to source.\n" ); 
		if (_atrTexTmuFromHandle( m->texture[0] ) == ATR_TEXELFX_1)
		  atuError( FXTRUE, "atrPushMaterial(): Texture handle mismatch, "
                    "the ->texture[0] allocated on TEXELFX 1\n" ); 
#endif
		atrTexSource( m->texture[0] );
		break;
	  case ATR_TEXSRC_LMAP:
	  case ATR_TEXSRC_NONE:
			_atrD3DRenderState(d3dContext, D3DRENDERSTATE_TEXTUREHANDLE, 0);
      default:
        break;
    }        
	
    if ( m->texSrc[1] != ATR_TEXSRC_NONE ) {
          atuError( FXTRUE,
                   "atrPushMaterial(): D3D does mot support multiple "
                   "simultaneous textures\n");
    }

    /* after calling atrTexSource we are assured that the vmSurf exists
       if chromakeying set the value */

    if ( d3dContext->TexturesOn && m->chromaKeyEnable ) {
        _AtrD3DTexEntry *d3dentry = (_AtrD3DTexEntry *)m->texture[0];
		 DDCOLORKEY          ddck;

		if ( d3dentry->vmSurf != NULL ) {
			ddck.dwColorSpaceLowValue  = m->chromaKeyValue;
			ddck.dwColorSpaceHighValue = ddck.dwColorSpaceLowValue;
			d3dentry->vmSurf->lpVtbl->SetColorKey(d3dentry->vmSurf, 
                                                  DDCKEY_SRCBLT, &ddck);
		}
    }

    /* TBD: this should map to different drawing primitives that use
            this color. Flat shaded
     */

    switch( m->crgbSrc ) {
      case ATR_CRGBSRC_STATIC:
        d3dContext->ConstantColorValue = m->constant ;
        break;
      default:
        break;
    }

    switch ( m->typeFlag & ATR_MAT_LIGHTING_MASK ) {
    case ATR_MAT_LIGHTING_NONE:
		if ( blendCaps & D3DPTBLENDCAPS_DECAL ) {
			_atrD3DRenderState(d3dContext, D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_DECAL );
		} else if ( blendCaps & D3DPTBLENDCAPS_COPY ) {
			_atrD3DRenderState(d3dContext, D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_COPY );
		} 
        
        break;
    case ATR_MAT_LIGHTING_MULTIPLY:
        _atrD3DRenderState(d3dContext, D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE );
        break;
    case ATR_MAT_LIGHTING_ADD: 
		if ( blendCaps & D3DPTBLENDCAPS_DECAL ) {
			_atrD3DRenderState(d3dContext, D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_DECAL );
		} else if ( blendCaps & D3DPTBLENDCAPS_COPY ) {
			_atrD3DRenderState(d3dContext, D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_COPY );
		} 
        break;
    case ATR_MAT_LIGHTING_BLEND_ON_TEXALPHA:
        _atrD3DRenderState(d3dContext, D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_DECALALPHA );
        break;
    }

    /* specify alpha test and reference value */

    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ALPHATESTENABLE, ( m->atestFunc != D3DCMP_ALWAYS));
    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ALPHAFUNC, alphaFunc[m->atestFunc] );
    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ALPHAREF, m->aReference );

    /* face culling, TBD: ATB does not support CCW */

    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_CULLMODE, (m->isTwoSided)?D3DCULL_NONE:D3DCULL_CCW);

    /* texture filtering TBD: the following modes are not supported by ATB
           D3DFILTER_LINEARMIPNEAREST,
           D3DFILTER_LINEARMIPLINEAR,
     */

    if ( _atrRenderCache->texCoordSrcFunc[0] == _atrD3D_TCSRC0_PROJECTED ) {
        _atrD3DRenderState(d3dContext, D3DRENDERSTATE_TEXTUREMIN, 
                D3DFILTER_LINEAR);

        _atrD3DRenderState(d3dContext, D3DRENDERSTATE_TEXTUREMAG, 
                D3DFILTER_LINEAR);
    } else {
        _atrD3DRenderState(d3dContext, D3DRENDERSTATE_TEXTUREMIN, 
                TEX_FILTER(m->texMinFilter[0], m->texMMMode[0]));

        _atrD3DRenderState(d3dContext, D3DRENDERSTATE_TEXTUREMAG, 
                TEX_FILTER(m->texMagFilter[0], m->texMMMode[0]));
    }

   /* TBD: potential problem with wrapping */

     _atrD3DRenderState(d3dContext, D3DRENDERSTATE_TEXTUREADDRESS, 
                        m->texSClamp[0] ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);

    _atrD3DRenderState(d3dContext,D3DRENDERSTATE_SRCBLEND,SrcTranslation[m->abuSrcFactor]);
    _atrD3DRenderState(d3dContext,D3DRENDERSTATE_DESTBLEND,DestTranslation[m->abuDstFactor]);
	if ( ( m->abuSrcFactor != GR_BLEND_ONE ) || ( m->abuDstFactor != GR_BLEND_ZERO) )
        _atrD3DRenderState(d3dContext, D3DRENDERSTATE_BLENDENABLE, 
              ( m->abuSrcFactor != GR_BLEND_ONE ) || ( m->abuDstFactor != GR_BLEND_ZERO) );
	else _atrD3DRenderState(d3dContext, D3DRENDERSTATE_BLENDENABLE, FALSE);
}

/*-------------------------------------------------------------------
  Function: _atrD3DShutdown
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Free resources used by this driver
  Arguments:
      None
  Return:
      Nothing
  -------------------------------------------------------------------*/

static void 
_atrD3DShutdown( void ) {
    D3DAppDestroy();
}

/*-------------------------------------------------------------------
  Function: _atrD3DInitDriver
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Initialize the D3D ATB driver
  Arguments:
    ctx - the rendering context to initialize
  Return:
    FXTRUE if the context was successfuly initialized, 
    FXFALSE otherwise
  -------------------------------------------------------------------*/

/*
 * Initialize the graphics context
 */

static FxBool
_atrD3DInitDriver(_AtrD3DDriver *ctx )  {
    HRESULT rval;
    EBufferData *ebd = &ctx->ebd;
    DDSCAPS ddscaps;

    _atrDriverXOffset = 0.1f;
    _atrDriverYOffset = -0.1f;

#ifdef DTRIAPI
    _cacheMemTemp=(FxU8 *)((((FxU32)_cacheMemoryChunk)+32)&0xFFFFFFE0);
#endif

    ctx->any.caps.width = d3dapp->ThisMode.w;
    ctx->any.caps.height = d3dapp->ThisMode.h;
    ctx->any.caps.bpp = d3dapp->ThisMode.bpp;
    ctx->lpDev = d3dapp->lpD3DDevice;
    ctx->lpView = d3dapp->lpD3DViewport;
    ctx->FrontBuffer =  d3dapp->lpFrontBuffer;
    ctx->BackBuffer = d3dapp->lpBackBuffer;
    ctx->ZBuffer = d3dapp->lpZBuffer;

    /* Determine the total amount of video memory */

    ddscaps.dwCaps=DDSCAPS_VIDEOMEMORY;
    rval=d3dapp->lpDD2->lpVtbl->GetAvailableVidMem(d3dapp->lpDD2, &ddscaps,
                           &ctx->TotalVideoMemory,
                           &ctx->FreeVideoMemory);
    CHECK(rval!=DD_OK, "Could not determine amount of video memory");

    /* Determine the total amount of texture memory */

    ddscaps.dwCaps=DDSCAPS_TEXTURE;
    rval=d3dapp->lpDD2->lpVtbl->GetAvailableVidMem(d3dapp->lpDD2,
                           &ddscaps,
                           &ctx->TotalTextureMemory,
                           &ctx->FreeTextureMemory);
    CHECK(rval!=DD_OK, "Could not determine amount of texture memory");

#ifdef DEBUG
    dprintf("_atrD3DInitDriver: total video memory = %d, total texture memory = %d\n", 
            ctx->TotalVideoMemory,ctx->TotalTextureMemory);
#endif

    ctx->MaxBufferSize = d3dapp->ThisDriver.Desc.dwMaxBufferSize;
    ctx->MaxVertexCount = d3dapp->ThisDriver.Desc.dwMaxVertexCount;

#ifdef DEBUG
    dprintf("_atrD3DInitDriver: max buff size = %d, max vertex count = %d\n", 
            d3dapp->ThisDriver.Desc.dwMaxBufferSize,
            d3dapp->ThisDriver.Desc.dwMaxVertexCount);
#endif

    ctx->TexturesOn = 0;     /*Are we texture mapping now*/

    ctx->ConstantColorValue = 0xff000000;
  
    ctx->FlipY = FXTRUE;  /*Should we flip the screen to be consistent with Glide*/

    ctx->clip_minx = 0;
    ctx->clip_miny = 0;
    ctx->clip_maxx = ctx->any.caps.width;
    ctx->clip_maxy =  ctx->any.caps.height;

    ebd->size = sizeof(D3DVERTEX) * MAX_VERTICES;
    ebd->size += sizeof(D3DINSTRUCTION) * MAX_INSTRUCTIONS;
    memset(&ctx->debDesc, 0, sizeof(D3DEXECUTEBUFFERDESC));
    ctx->debDesc.dwSize = sizeof(D3DEXECUTEBUFFERDESC);
    ctx->debDesc.dwFlags = D3DDEB_BUFSIZE;
    ctx->debDesc.dwBufferSize = ebd->size;
    if (d3dapp->lpD3DDevice->lpVtbl->CreateExecuteBuffer(d3dapp->lpD3DDevice, &ctx->debDesc, &ctx->lpD3DExBuf,
                                           NULL) != D3D_OK) {
        atuError(FXTRUE, "could not create execute buffer\n");
    }

	ctx->ddsdTextureFormat = d3dapp->ThisTextureFormat.ddsd;
    _atrD3DInitExecuteBuffer(ctx);

    /* initialize capabilities structure */

    ctx->any.caps.deviceType = ATB_DEVICE_D3D ;
    ctx->any.caps.pfxRev = 0;
    ctx->any.caps.pfxMem = ctx->TotalVideoMemory;
    ctx->any.caps.numTex = 1;
    ctx->any.caps.tfxRev = 0;
    ctx->any.caps.tfxMem = ctx->TotalTextureMemory;
    ctx->any.caps.sli = 0;
    ctx->any.caps.hasDepth = ( ctx->ZBuffer != NULL );
    ctx->any.caps.fullScreen = d3dapp->bFullscreen;
}

/*-------------------------------------------------------------------
  Function: _atrD3DInitTextureFormats
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Check out what interesting texture formats we have
  Arguments:
      ctx - our driver context
  Return:
      None
  -------------------------------------------------------------------*/

static void
_atrD3DInitTextureFormats(_AtrD3DDriver *ctx) {
    int i, alphaBPP, redBPP, greenBPP, blueBPP, indexBPP, texBPP;
    char buff[80];

    /* initialize all texture formats to unknown */

    ctx->texFormatI8 = -1;
    ctx->texFormat565 = -1;
    ctx->texFormat4444 = -1;

	sprintf(buff, "Num Texture Formats = %d\n", d3dapp->NumTextureFormats);
	OutputDebugStr(buff);

    for ( i = 0; i < d3dapp->NumTextureFormats; i++ ) {

		/* determine characteristics of this texture format */

         texBPP = d3dapp->TextureFormat[i].ddsd.ddpfPixelFormat.dwRGBBitCount ;
         alphaBPP = texBPP-d3dapp->TextureFormat[i].ddsd.ddpfPixelFormat.dwAlphaBitDepth ;
         redBPP = d3dapp->TextureFormat[i].RedBPP ;
		 greenBPP = d3dapp->TextureFormat[i].GreenBPP ;
         blueBPP = d3dapp->TextureFormat[i].BlueBPP ;
		 indexBPP = d3dapp->TextureFormat[i].IndexBPP;

		 sprintf(buff, "bpp = %d %d %d %d %d %d\n", texBPP, alphaBPP, redBPP, greenBPP, blueBPP, indexBPP);
         OutputDebugStr(buff);

         if ( d3dapp->TextureFormat[i].bPalettized ) { /* palettized texture */
             if ( d3dapp->TextureFormat[i].IndexBPP == 8 ) {
                ctx->texFormatI8 = i;
				sprintf(buff, "got i8 format, index = %d\n", i);
                OutputDebugStr(buff);
             }
         } else { /* RGB or RGBA texture */

             

             
             /* for now we are only interested in 16 bit formats */

             if ( texBPP != 16 )
                 continue;

             if (( redBPP == 5) && ( greenBPP == 6) && ( blueBPP == 5)) {
                ctx->texFormat565 = i;
				sprintf(buff, "got 565 format, index = %d\n", i);
                OutputDebugStr(buff);
             } else if (( redBPP == 4) && 
                        ( greenBPP == 4) && ( blueBPP == 4)) {
                ctx->texFormat4444 = i;
				sprintf(buff, "got 4444 format, index = %d\n", i);
                OutputDebugStr(buff);
             }
         }
    }
}

/*-------------------------------------------------------------------
  Function: _atrD3DInit
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Create all DirectDraw and Direct3D objects necessary to begin rendering.
  Arguments:
      driver info - descibing specific configaration
  Return:
      Initialized driver
  -------------------------------------------------------------------*/

AtrDriver *
_atrD3DInit( AtrDriverInfo *driverInfo ) {
    BOOL bOnlySystemMemory, bOnlyEmulation;
    DWORD flags;

#ifdef AT_DEBUGGING
        if ( driverInfo == NULL ) 
          atuError( FXTRUE, "atrD3DInit: Null driver info\n");
#endif

    _atrSnapBias = 0.0f;

    UpdateWindow(driverInfo->hWnd);

    /*
     * Process configuration settings
     *     systemmemory  All surfaces should be created in system memory.
     *                   Hardware DD and D3D devices are disabled, but
     *                   debugging during the Win16 lock becomes possible.
     *     emulation     Do not use hardware DD or D3D devices.
     */
    bOnlySystemMemory = driverInfo->emulation;
    bOnlyEmulation = driverInfo->emulation;
    
    /*
     * Set the flags to pass to the D3DApp creation based on command line
     */
    flags = ((bOnlySystemMemory) ? D3DAPP_ONLYSYSTEMMEMORY : 0) | 
            ((bOnlyEmulation) ? (D3DAPP_ONLYD3DEMULATION |
                                 D3DAPP_ONLYDDEMULATION) : 0);
    /*
     * Create all the DirectDraw and D3D objects neccesary to render.  The
     * AfterDeviceCreated callback function is called by D3DApp to create the
     * viewport and the example's execute buffers.
     */
    if (!D3DAppCreateFromHWND(flags, driverInfo->hWnd, _atrD3DAfterDeviceCreated,
                              NULL, _atrD3DBeforeDeviceDestroyed, NULL, &d3dapp)) {
        atuError(FXTRUE, "%s", D3DAppLastErrorString());
    }

    _atrD3DInitDispatchTable(d3dContext);


    /* determine what texture formats we have */

    _atrD3DInitTextureFormats(d3dContext);

    /* determine what filter modes we have */

    _atrD3DInitFilterModes();

	return (AtrDriver *)d3dContext;
}

/*-------------------------------------------------------------------
  Function:  _atrD3DAfterDeviceCreated
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    D3DApp will call this function immediately after the D3D device has been
    created (or re-created).  D3DApp expects the D3D viewport to be created and
    returned.  The sample's execute buffers are also created (or re-created)
    here.
  Arguments:
    w            - viewport width
    w            - viewport height
    lplpViewPort - viewport to be created
    lpContext    - the context
  Return:
    FXTRUE if the context can currently be drawn to, FXFALSE otherwise
  -------------------------------------------------------------------*/

/*
 */
static BOOL
_atrD3DAfterDeviceCreated(int w, int h, LPDIRECT3DVIEWPORT* lplpViewport, LPVOID lpContext) {
    LPDIRECT3DVIEWPORT lpD3DViewport;
    HRESULT rval;
    D3DVIEWPORT viewData;

    /*
     * Create the D3D viewport object
     */
    rval = d3dapp->lpD3D->lpVtbl->CreateViewport(d3dapp->lpD3D, &lpD3DViewport, NULL);
    if (rval != D3D_OK) {
        atuError(FXTRUE, "Create D3D viewport failed.\n%s", 
                 D3DAppErrorToString(rval));
    }
    /*
     * Add the viewport to the D3D device
     */
    rval = d3dapp->lpD3DDevice->lpVtbl->AddViewport(d3dapp->lpD3DDevice, lpD3DViewport);
    if (rval != D3D_OK) {
        atuError(FXTRUE, "Add D3D viewport failed.\n%s", 
                 D3DAppErrorToString(rval));
    }
    /*
     * Setup the viewport for a reasonable viewing area
     */
    memset(&viewData, 0, sizeof(D3DVIEWPORT));
    viewData.dwSize = sizeof(D3DVIEWPORT);
    viewData.dwX = viewData.dwY = 0;
    viewData.dwWidth = w;
    viewData.dwHeight = h;
    viewData.dvScaleX = viewData.dwWidth / (float)2.0;
    viewData.dvScaleY = viewData.dwHeight / (float)2.0;
    viewData.dvMaxX = (float)D3DDivide(D3DVAL(viewData.dwWidth),
                                       D3DVAL(2 * viewData.dvScaleX));
    viewData.dvMaxY = (float)D3DDivide(D3DVAL(viewData.dwHeight),
                                       D3DVAL(2 * viewData.dvScaleY));
    rval = lpD3DViewport->lpVtbl->SetViewport(lpD3DViewport, &viewData);
    if (rval != D3D_OK) {
        atuError(FXTRUE, "SetViewport failed.\n%s", D3DAppErrorToString(rval));
    }

    /*
     * Return the viewport to D3DApp so it can use it
     */

    *lplpViewport = lpD3DViewport;

    _atrD3DInitDriver(d3dContext);

    d3dContext->any.texEntrySize = sizeof(_AtrD3DTexEntry);

    if (d3dapp->lpD3D->lpVtbl->CreateMaterial(d3dapp->lpD3D, &d3dContext->lpBmat, NULL) != D3D_OK) {
        return FALSE;
    }
    memset(&d3dContext->bmat, 0, sizeof(D3DMATERIAL));
    d3dContext->bmat.dwSize = sizeof(D3DMATERIAL);
    d3dContext->bmat.diffuse.r = (D3DVALUE)0.0;
    d3dContext->bmat.diffuse.g = (D3DVALUE)1.0;
    d3dContext->bmat.diffuse.b = (D3DVALUE)0.0;
    d3dContext->bmat.ambient.r = (D3DVALUE)0.0;
    d3dContext->bmat.ambient.g = (D3DVALUE)0.0;
    d3dContext->bmat.ambient.b = (D3DVALUE)0.0;

#ifdef notdef
    d3dContext.bmat.hTexture = TextureHandle[0];
#endif
	
    d3dContext->bmat.dwRampSize = 1;
    d3dContext->lpBmat->lpVtbl->SetMaterial(d3dContext->lpBmat, &d3dContext->bmat);

    d3dContext->lpBmat->lpVtbl->GetHandle(d3dContext->lpBmat, d3dapp->lpD3DDevice, &d3dContext->hBmat);
    d3dContext->lpView->lpVtbl->SetBackground(d3dContext->lpView, d3dContext->hBmat);

    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_CULLMODE, D3DCULL_CW);
  
    d3dContext->FlipY = 1;

	/* set texture scaling */

	_atrTexSetScale(ATR_TEXELFX_0, 1.0f, 1.0f);

    /* set initial hardware state */
  
    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ZENABLE, TRUE);
    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ZWRITEENABLE, TRUE);
    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);
    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_BLENDENABLE, FALSE);
    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_MIPLINEAR);
    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_MIPLINEAR);
    _atrD3DRenderState(d3dContext, D3DRENDERSTATE_TEXTUREADDRESS,D3DTADDRESS_CLAMP);
	_atrD3DRenderState(d3dContext, D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
	_atrD3DRenderState(d3dContext, D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
	_atrD3DRenderState(d3dContext, D3DRENDERSTATE_DITHERENABLE, TRUE);
	_atrD3DRenderState(d3dContext, D3DRENDERSTATE_SPECULARENABLE, TRUE);
	_atrD3DRenderState(d3dContext, D3DRENDERSTATE_ANTIALIAS, FALSE);

    return TRUE;
}

/*-------------------------------------------------------------------
  Function: _atrD3DBeforeDeviceDestroyed
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    D3DApp will call this function before the current D3D device is destroyed
    to give the app the opportunity to destroy objects it has created with the
    DD or D3D objects.
  Arguments:
    ctx - the rendering context
  Return:
    FXTRUE if the context can currently be drawn to, FXFALSE otherwise
  -------------------------------------------------------------------*/

static BOOL
_atrD3DBeforeDeviceDestroyed(LPVOID lpContext) {
    /*
     * Since we created the viewport it is our responsibility to release
     * it.  Use D3DApp's pointer to it since we didn't save one.
     */
    d3dapp->lpD3DViewport->lpVtbl->Release(d3dapp->lpD3DViewport);
    return TRUE;
}

/*-------------------------------------------------------------------
  Function: _atrD3DRestoreSurfaces
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Restores any lost surfaces.  Returns TRUE if all surfaces are not lost and
    FALSE if one or more surfaces is lost and can not be restored at the
    moment.
  Arguments:
    None
  Return:
    FXTRUE if the surfaces are restored, FXFALSE otherwise
  -------------------------------------------------------------------*/

static BOOL
_atrD3DRestoreSurfaces() {

    /*
     * Have D3DApp check all the surfaces it's in charge of
     */
    if (!D3DAppCheckForLostSurfaces()) {
            return FALSE;
    }
    return TRUE;
}


/*-------------------------------------------------------------------
  Function: atrIsDrawable
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Determine if a context can currently be drawn to
  Arguments:
    ctx - the rendering context
  Return:
    FXTRUE if the context can currently be drawn to, FXFALSE otherwise
  -------------------------------------------------------------------*/

static FxBool 
_atrD3DIsDrawable( AtrContext ctx ) {
    return (d3dapp->bRenderingIsOK && !d3dapp->bMinimized
            && !d3dapp->bPaused 
            && (d3dapp->bAppActive || !d3dapp->bFullscreen));
}

/*-------------------------------------------------------------------
  Function: _atrD3DResize
  Date: 10/11/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Resizes all the buffers and re-creates device if necessary.
    A new viewport will definitely be needed, but the
    device and buffers will only be re-created if they have gotten bigger
    or change size by a very large amount.
  Arguments:
    ctx - the rendering context
    w   - new width
    h   - new height
  Return:
    FXTRUE if the resize was succesful, false otherwise
  -------------------------------------------------------------------*/

static FxBool
_atrD3DResize(AtrContext ctx, int w, int h) {
    D3DAppIValidateDirtyRects();
    D3DAppCheckForLostSurfaces();
    /*
     * If w and h are under the minimum, create buffers of the minimum size
     */
    if (w < D3DAPP_WINDOWMINIMUM)
        w = D3DAPP_WINDOWMINIMUM;
    if (h < D3DAPP_WINDOWMINIMUM)
        h = D3DAPP_WINDOWMINIMUM;
    /*
     * Destroy the viewport and all execute buffers
     */
    d3dapp->bRenderingIsOK = FALSE;
    ATTEMPT(D3DAppICallDeviceDestroyCallback());
    /*
     * Only create a new device and buffers if they changed significantly,
     * otherwise just make sure the old buffers aren't lost.
     */
    if ((w > szBuffers.cx || h > szBuffers.cy) ||
        (w < szBuffers.cx / 2 || h < szBuffers.cy / 2)) {
        /*
         * Release the device
         */
        RELEASE(d3dapp->lpD3DDevice);
        /*
         * Release the old buffers
         */
        RELEASE(d3dapp->lpZBuffer);
        RELEASE(lpPalette);
        RELEASE(lpClipper);
        RELEASE(d3dapp->lpBackBuffer);
        RELEASE(d3dapp->lpFrontBuffer);
        /*
         * Create new ones
         */
        ATTEMPT(D3DAppICreateBuffers(d3dapp->hwnd, w, h, D3DAPP_BOGUS, FALSE, 
                                     d3dapp->ThisDriver.bIsHardware));
        ATTEMPT(D3DAppICheckForPalettized());
        ATTEMPT(D3DAppICreateZBuffer(w, h, d3dapp->CurrDriver));
        /*
         * Create the driver
         */
        ATTEMPT(D3DAppICreateDevice(d3dapp->CurrDriver));
    } else {
        D3DAppCheckForLostSurfaces();
    }
    /*
     * Call the device create callback to create the viewport, set the render
     * state and clear the dirty rectangle info
     */
    ATTEMPT(D3DAppICallDeviceCreateCallback(w, h));
    D3DAppIValidateDirtyRects();
    d3dapp->bRenderingIsOK = TRUE;
    return TRUE;
exit_with_error:
    D3DAppICallDeviceDestroyCallback();
    RELEASE(d3dapp->lpD3DDevice);
    RELEASE(d3dapp->lpZBuffer);
    RELEASE(lpPalette);
    RELEASE(lpClipper);
    RELEASE(d3dapp->lpBackBuffer);
    RELEASE(d3dapp->lpFrontBuffer);
    return FALSE;
}

/*-------------------------------------------------------------------
  Function: _atrD3DPause
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Switch between main screen and graphics screen when in pass 
    through mode
  Arguments:
    flag - 1 if application is pausing, 0 if restarting
  Return:
    FXTRUE if switch was successful, FXFALSE otherwise
  -------------------------------------------------------------------*/

static FxBool 
_atrD3DPause(FxBool flag) {
    return D3DAppPause(flag);
}

/*-------------------------------------------------------------------
  Function: _atrD3DIdle
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Wait until the hardware has finsihed rendering
  Arguments:
    None
  Return:
    None
  -------------------------------------------------------------------*/

static void _atrD3DIdle(void) {
}

/*-------------------------------------------------------------------
  Function: _atrDitherMode
  Date: 11/27/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Set dither mode
  Arguments:
    None
  Return:
    None
  -------------------------------------------------------------------*/

static void 
_atrD3DDitherMode(AtrDitherMode mode) {
	_atrD3DRenderState(d3dContext, D3DRENDERSTATE_DITHERENABLE, 
                       ( mode != ATR_DITHER_DISABLE ));
}

/*-------------------------------------------------------------------
  Function: _atrD3DInitDispatchTable
  Date: 10/12/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Initialize the dispatch table for the D3D ATB driver
  Arguments:
    ctx - the rendering context to initialize
  Return:
    Nothing
  -------------------------------------------------------------------*/

static void
_atrD3DInitDispatchTable(_AtrD3DDriver *ctx) {
    ctx->any.RealizeImg = _atrD3DRealizeImg ;
    ctx->any.UnrealizeImg = _atrD3DUnrealizeImg ;
    ctx->any.CloneImg = _atrD3DCloneImg ;
    ctx->any.ClearCanvas = _atrD3DClearCanvas ;
    ctx->any.SwapBuffer = _atrD3DSwapBuffer ;
    ctx->any.BeginScene = _atrD3DBeginScene ;
    ctx->any.EndScene = _atrD3DEndScene ;
    ctx->any.RenderImg = _atrD3DRenderImg ;
    ctx->any.GrabImg = _atrD3DGrabImg ;
    ctx->any.TexEntryInit = _atrD3DTexEntryInit ;
    ctx->any.TexDeleteHandle = _atrD3DTexDeleteHandle ;
    ctx->any.TexPunt = _atrD3DTexPunt ;
    ctx->any.TexAssociate = _atrD3DTexAssociate ;
    ctx->any.TexSource = _atrD3DTexSource ;
    ctx->any.TramAllocate = _atrD3DTramAllocate ;
    ctx->any.UpdateEnv = _atrD3DUpdateEnv ;
    ctx->any.UpdateMaterial = _atrD3DUpdateMaterial ;
    ctx->any.Shutdown = _atrD3DShutdown ;
    ctx->any.Idle = _atrD3DIdle ;
    ctx->any.IsDrawable = _atrD3DIsDrawable ;
    ctx->any.Pause = _atrD3DPause ;
    ctx->any.Resize = _atrD3DResize ;
    ctx->any.RenderBuffer = _atrD3DRenderBuffer ;
    ctx->any.DrawLine = _atrD3DDrawLine ;
    ctx->any.DrawPoint = _atrD3DDrawPoint ;
    ctx->any.DrawTriangle = _atrD3DDrawTriangle ;
    ctx->any.Splash = _atrD3DSplash ;
    ctx->any.GetPerfStats = _atrD3DGetPerfStats ;
    ctx->any.ResetPerfStats = _atrD3DResetPerfStats ;
    ctx->any.ClipWindow = _atrD3DClipWindow ;

    ctx->any.TransformVertices = _atrD3DTransformVertices;
    ctx->any.SpecialTransformVertices = _atrD3DSpecialTransformVertices;
    ctx->any.TransformVertices2D = _atrD3DTransformVertices2D;
    ctx->any.RenderTri = atrD3DRenderTri;
    ctx->any.RenderTriSet = atrD3DRenderTriSet;
    ctx->any.SpecialRenderTriSet = atrD3DSpecialRenderTriSet;
    ctx->any.RenderTriWF = atrD3DRenderTriWF;
    ctx->any.RenderTriSetWF = atrD3DRenderTriSetWF;
    ctx->any.TransformOpenTriSet = atrD3DTransformOpenTriSet;
    ctx->any.SpecialTransformOpenTriSet = atrD3DSpecialTransformOpenTriSet;
    ctx->any.RenderOpenTriSet = atrD3DRenderOpenTriSet;
    ctx->any.RenderOpenTriSetWF = atrD3DRenderOpenTriSetWF;
    ctx->any.RenderSegment = atrD3DRenderSegment;
    ctx->any.Render2DTri = atrD3DRender2DTri;
    ctx->any.ClipAndRenderTri = _atrD3DClipAndRenderTri;
    ctx->any.ClipAndRenderSegment = _atrD3DClipAndRenderSegment;
    ctx->any.DitherMode = _atrD3DDitherMode;
}
