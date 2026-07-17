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
    
    /* ATB: Glide driver */
    
    #include <math.h>
    #include <string.h>
    #include <stdio.h>
    #include <3dfx.h>
#ifdef HWC_CSIM
    #include <fxhwc.h>
#else
    #define SET16(d,s) (d = s)
#endif


    #ifdef GLIDE3
    #include <glide.h>
    #define PARAM_XY_OFFSET 0
    // between 8 and 12 there is the original unused
    // ooz, which is used a FxU32 Flags in ATB
    #define PARAM_RGB_OFFSET 12
    #define PARAM_Z_OFFSET 24
    #define PARAM_A_OFFSET 28
    #define PARAM_W_OFFSET 32
    //#define PARAM_PARGB_OFFSET 
    #define PARAM_ST0_OFFSET 36
    #define PARAM_W0_OFFSET 44
    #define PARAM_ST1_OFFSET 48
    #define PARAM_W1_OFFSET 56

    #define GR_STWHINT_W_DIFF_FBI   (1<<0)
    #define GR_STWHINT_W_DIFF_TMU0  (1<<1)
    #define GR_STWHINT_ST_DIFF_TMU0 (1<<2)
    #define GR_STWHINT_W_DIFF_TMU1  (1<<3)
    #define GR_STWHINT_ST_DIFF_TMU1 (1<<4)
    #define GR_STWHINT_W_DIFF_TMU2  (1<<5)
    #define GR_STWHINT_ST_DIFF_TMU2 (1<<6)

    #else
    #include <glide.h>
    #endif
    
    #include <texus.h>
    #include <atrender.h>
    #include "fxatr.h"
    #include "rglide.h"
    
    #if macintosh
    #define fpu_single_precision()
    #else
    extern void __cdecl fpu_single_precision( void );
    #endif
    
    static FxU32 _atTexMaxAddress[2];
    #ifdef GLIDE3
    GrProc grChromaRangeMode = NULL;
    GrProc grChromaRange = NULL;
    #endif
    
    /* 3dfx texture entry */
    
    typedef struct _ATRFXTE {
        _AtrTexEntry        any; /* must be first entry */
        GrTexInfo      grInfo;
        FxU32          startAddress ;
        float          s_scale ;
        float          t_scale ;
    } _AtrFXTexEntry ;
    
    typedef struct {
        void  *ncc[2];
        FxU32 nextNcc;
        void  *pal;
        FxU32 startAddress;
    } _AtrTexelFxState;
    
    AtrDriver _atrGlideDriver;
    
    static FxBool gDualMonitors = FXFALSE;
    
#ifndef GLIDE3
    static GrHwConfiguration _atrHwConfig;
#endif
    
    static _AtrTexelFxState tfxState[2];
    
    FxU32 _defaultImgData[2] = { 0x0000FFFF, 0xFFFF0000 };
    
    static AtrImg _defaultImg = 
    {
        ATR_IMGFMT_RGB_565,  /* format */
        2,                   /* width */
        2,                   /* height */
        1,                   /* nLevels */
        0,                   /* name */
        _defaultImgData      /* data */
        /* table - nil */
    };
    
    static void 
    glideErrorCallback( const char *msg, FxBool fatal ) {
      atuError(fatal, msg);
    }
    
    static void _atrGlideInitDispatchTable(AtrDriver *ctx);
     
    /*-------------------------------------------------------------------
      Function: _atrGlideBeginScene
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
    _atrGlideBeginScene(FxU32 *viewWidth, FxU32* viewHeight) {
        *viewWidth = _atrGlideDriver.caps.width  ;
        *viewHeight = _atrGlideDriver.caps.height ;
        fpu_single_precision();
        return FXTRUE;
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideEndScene
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
    _atrGlideEndScene(void) {
         return FXTRUE;
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideRenderImg
      Date: 10/9/96
      Implementor(s): mlwp
      Library: AT Render
      Description:
        Render an image at the specified location
      Arguments:
        i    - the image to display
        winX - location at which to display image
        winY 
      Return:
        FXTRUE if the context can currently be drawn to, FXFALSE otherwise
        TBD: figure out why alpha blending is not working
      -------------------------------------------------------------------*/
    #ifdef GLIDE3 
//    #define DEBUG_GLIDE3 1
    #endif
    
    static void 
    _atrGlideRenderImg( AtrImg *i, FxU32 screenX, FxU32 screenY ) {
        int format;
        GrLfbInfo_t info;
        unsigned y,x;

    #ifdef DEBUG_GLIDE3
        FxU32 PI_before, PI_after, PO_before, PO_after; /* Pixel In & Out stats */
        FxU32 delta_In, delta_Out;
    
        // Debug glide3. try to force the display of the image.
        // force the Z test to pass
        grDepthBufferFunction( GR_CMP_ALWAYS );
    #endif 
    
    
        if ( _atrGlideDriver.caps.height > ( i->height + screenY)) 
                screenY = _atrGlideDriver.caps.height - i->height - screenY;
        else screenY = 0;
    
        info.size = sizeof( info );
        format = GR_LFBWRITEMODE_565;
    
		grLfbConstantAlpha((GrAlpha_t)(_atrCurrentMaterial->constant>>24));
        if ( !grLfbLock( GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER,
                         format, GR_ORIGIN_UPPER_LEFT, FXTRUE,
                         &info ) ) {
            if ( !grLfbLock( GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER,
                             format, GR_ORIGIN_UPPER_LEFT, FXFALSE,
                             &info ) ) {
                atuError(FXTRUE, "Could not set lfb lock");
            }
        }
    
    #ifdef DEBUG_GLIDE3
        grGet( GR_STATS_PIXELS_IN, 4, &PI_before);
        grGet( GR_STATS_PIXELS_OUT, 4, &PO_before);
    #endif 
    
    
        switch( i->format ) {
        case ATR_IMGFMT_RGB_565:
            {
                FxU16 *src, *dst;
                for( y = 0; y < i->height; y++ ) {
                    FxU32 sy = y + screenY;
                    dst = (FxU16*)( ((char*)info.lfbPtr) + 
                                             (sy*info.strideInBytes) +
                                             (screenX<<1) );
                    src = ((unsigned short*)(i->data)) + (y*i->width);
                    if ( _atrCurrentMaterial->chromaKeyEnable ) {
                        for( x = 0; x < i->width; x++ ) {
                            if ( *src != 0 )
                                SET16(*dst, *src);
                            dst++; src++;
                        }
                    } else {
                        for( x = 0; x < i->width; x++ ) {
                            SET16(*dst++, *src++);
                        }
                    }
                }
                
            }
            break;
        case ATR_IMGFMT_ARGB_1555:
            {
                FxU16 *src, *dst;
                FxU16 r, g, b, a;
                FxU16 pix;
    
                for( y = 0; y < i->height; y++ ) {
                    FxU32 sy = y + screenY;
                    dst = (FxU16*)( ((char*)info.lfbPtr) + 
                                             (sy*info.strideInBytes) +
                                             (screenX<<1) );
                    src = ((unsigned short*)(i->data)) + (y*i->width);
                    if ( _atrCurrentMaterial->chromaKeyEnable ) {
                        for( x = 0; x < i->width; x++ ) {
                            pix = *src;
                            b = pix & 0x1f; pix >>= 5;
                            g = pix & 0x1f; pix >>= 5;
                            r = pix & 0x1f; pix >>= 5;
                            if ( *src != 0 )
                                SET16(*dst, (FxU16)(( r << 11 ) | ( g << 6 ) | b));
                            dst++; src++;
                        }
                    } else {
                        for( x = 0; x < i->width; x++ ) {
                            pix = *src;
                            b = pix & 0x1f; pix >>= 5;
                            g = pix & 0x1f; pix >>= 5;
                            r = pix & 0x1f; pix >>= 5;
                            SET16(*dst, (FxU16)(( r << 11 ) | ( g << 6 ) | b));
                            dst++; src++;
                        }
                    }
                }
                
            }
            break;
        case ATR_IMGFMT_ARGB_8888:
            {
                FxU16 *dst;
                FxU16 r, g, b, a;
                FxU32 *src, pix;
    
                for( y = 0; y < i->height; y++ ) {
                    FxU32 sy = y + screenY;
                    dst = (FxU16*)( ((char*)info.lfbPtr) + 
                                            (sy*info.strideInBytes) +
                                            (screenX<<1) );
                    src = ((unsigned long*)(i->data)) + (y*i->width);
                    for( x = 0; x < i->width; x++ ) {
                        pix = *src;
                        b = (FxU16)(( pix >> 3 ) & 0x1f); pix >>= 8;
                        g = (FxU16)(( pix >> 2 ) & 0x3f); pix >>= 8;
                        r = (FxU16)(( pix >> 3 ) & 0x1f); pix >>= 8;
                        a = (FxU16)(( pix >> 3 ) & 0x1f); pix >>= 8;
                        SET16(*dst, (FxU16)(( r << 11 ) | ( g << 5 ) | b));
                        *dst++; src++;
                    }
                }
                
            }
            break;
        default:
            atuError(FXTRUE, 
                     "RenderImage: Unsupported image format 0x%lx", i->format);
        }
    
        grLfbUnlock( GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER );
    
    #ifdef DEBUG_GLIDE3
        grGet( GR_STATS_PIXELS_IN, 4, &PI_after);
        grGet( GR_STATS_PIXELS_OUT, 4, &PO_after);
        delta_In = PI_after - PI_before;
        delta_Out = PO_after- PO_before;
        // Debug glide3. try to force the display of the image.
        // force the Z test to pass
        grDepthBufferFunction( GR_CMP_LEQUAL );
    #endif 
    
        return;
    }
    
    static void 
    _atrGlideRenderImgBuffer( AtrImg *i, FxU32 screenX, FxU32 screenY, FxU32 buffer ) {
        int format;
        GrLfbInfo_t info;
        unsigned y,x;

    #ifdef DEBUG_GLIDE3
        FxU32 PI_before, PI_after, PO_before, PO_after; /* Pixel In & Out stats */
        FxU32 delta_In, delta_Out;
    
        // Debug glide3. try to force the display of the image.
        // force the Z test to pass
        grDepthBufferFunction( GR_CMP_ALWAYS );
    #endif 
    
    
        if ( _atrGlideDriver.caps.height > ( i->height + screenY)) 
                screenY = _atrGlideDriver.caps.height - i->height - screenY;
        else screenY = 0;
    
        info.size = sizeof( info );
        format = GR_LFBWRITEMODE_565;
    
    // Denis debug GLIDE3???    
		grLfbConstantAlpha((GrAlpha_t)(_atrCurrentMaterial->constant>>24));
        //grLfbConstantAlpha((GrAlpha_t)(_atrCurrentMaterial->constant>>18));
        if ( !grLfbLock( GR_LFB_WRITE_ONLY, buffer,
                         format, GR_ORIGIN_UPPER_LEFT, FXTRUE,
                         &info ) ) {
            if ( !grLfbLock( GR_LFB_WRITE_ONLY, buffer,
                             format, GR_ORIGIN_UPPER_LEFT, FXFALSE,
                             &info ) ) {
                atuError(FXTRUE, "Could not set lfb lock");
            }
        }
    
    #ifdef DEBUG_GLIDE3
        grGet( GR_STATS_PIXELS_IN, 4, &PI_before);
        grGet( GR_STATS_PIXELS_OUT, 4, &PO_before);
    #endif 
    
    
        switch( i->format ) {
        case ATR_IMGFMT_RGB_565:
            {
                FxU16 *src, *dst;
                for( y = 0; y < i->height; y++ ) {
                    FxU32 sy = y + screenY;
                    dst = (FxU16*)( ((char*)info.lfbPtr) + 
                                             (sy*info.strideInBytes) +
                                             (screenX<<1) );
                    src = ((unsigned short*)(i->data)) + (y*i->width);
                    if ( _atrCurrentMaterial->chromaKeyEnable ) {
                        for( x = 0; x < i->width; x++ ) {
                            if ( *src != 0 )
                                SET16(*dst, *src);
                            dst++; src++;
                        }
                    } else {
                        for( x = 0; x < i->width; x++ ) {
                            SET16(*dst, *src);
                            dst++; src++;
                        }
                    }
                }
                
            }
            break;
        case ATR_IMGFMT_ARGB_1555:
            {
                FxU16 *src, *dst;
                FxU16 r, g, b, a;
                FxU16 pix;
    
                for( y = 0; y < i->height; y++ ) {
                    FxU32 sy = y + screenY;
                    dst = (FxU16*)( ((char*)info.lfbPtr) + 
                                             (sy*info.strideInBytes) +
                                             (screenX<<1) );
                    src = ((unsigned short*)(i->data)) + (y*i->width);
                    if ( _atrCurrentMaterial->chromaKeyEnable ) {
                        for( x = 0; x < i->width; x++ ) {
                            pix = *src;
                            b = pix & 0x1f; pix >>= 5;
                            g = pix & 0x1f; pix >>= 5;
                            r = pix & 0x1f; pix >>= 5;
                            if ( *src != 0 )
                                SET16(*dst, (FxU16)(( r << 11 ) | ( g << 6 ) | b));
                            dst++; src++;
                        }
                    } else {
                        for( x = 0; x < i->width; x++ ) {
                            pix = *src;
                            b = pix & 0x1f; pix >>= 5;
                            g = pix & 0x1f; pix >>= 5;
                            r = pix & 0x1f; pix >>= 5;
                            SET16(*dst, (FxU16)(( r << 11 ) | ( g << 6 ) | b));
                            dst++; src++;
                        }
                    }
                }
                
            }
            break;
        case ATR_IMGFMT_ARGB_8888:
            {
                FxU16 *dst;
                FxU16 r, g, b, a;
                FxU32 *src, pix;
    
                for( y = 0; y < i->height; y++ ) {
                    FxU32 sy = y + screenY;
                    dst = (FxU16*)( ((char*)info.lfbPtr) + 
                                            (sy*info.strideInBytes) +
                                            (screenX<<1) );
                    src = ((unsigned long*)(i->data)) + (y*i->width);
                    for( x = 0; x < i->width; x++ ) {
                        pix = *src;
                        b = (FxU16)(( pix >> 3 ) & 0x1f); pix >>= 8;
                        g = (FxU16)(( pix >> 2 ) & 0x3f); pix >>= 8;
                        r = (FxU16)(( pix >> 3 ) & 0x1f); pix >>= 8;
                        a = (FxU16)(( pix >> 3 ) & 0x1f); pix >>= 8;
                        SET16(*dst, (FxU16)(( r << 11 ) | ( g << 5 ) | b));
                        *dst++; src++;
                    }
                }
                
            }
            break;
        default:
            atuError(FXTRUE, 
                     "RenderImage: Unsupported image format 0x%lx", i->format);
        }
    
        grLfbUnlock( GR_LFB_WRITE_ONLY, buffer );
    
    #ifdef DEBUG_GLIDE3
        grGet( GR_STATS_PIXELS_IN, 4, &PI_after);
        grGet( GR_STATS_PIXELS_OUT, 4, &PO_after);
        delta_In = PI_after - PI_before;
        delta_Out = PO_after- PO_before;
        // Debug glide3. try to force the display of the image.
        // force the Z test to pass
        grDepthBufferFunction( GR_CMP_LEQUAL );
    #endif 
    
        return;
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideGrabImg
      Date: 10/9/96
      Implementor(s): mlwp
      Library: AT Render
      Description:
        Grab a portion of the display surface
      Arguments:
        dst  - where to put the captured image
        winX - location at which to start capture
        winY 
        buf  - capture front or back buffer?
      Return:
        FXTRUE if the context can currently be drawn to, FXFALSE otherwise
      -------------------------------------------------------------------*/
    
    static void 
    _atrGlideGrabImg( AtrImg    *dst,
                     FxU32     screenX,
                     FxU32     screenY,
                     AtrBuffer buf) {
        grLfbReadRegion(buf, screenX, screenY, 
                        dst->width, dst->height,
                        dst->width*2, dst->data);
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideClearCanvas
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
    _atrGlideClearCanvas( float r, float g, float b, FxU16 za ) {
        GrColor_t color;
        FxI32 wrange[2];

    #ifdef GLIDE3
        grGet(GR_WDEPTH_MIN_MAX, 8, wrange);
    #endif //GLIDE3
        color =  ((GrColor_t)( r * 255.0 )) << 16;
        color |= ((GrColor_t)( g * 255.0 )) <<  8;
        color |= ((GrColor_t)( b * 255.0 )) <<  0;
    #ifdef GLIDE3
        grBufferClear( color, 0, wrange[1] );
    #else
        grBufferClear( color, 0, za );
    #endif //GLIDE3
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideSwapBuffer
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
    
    #define SST_SWAPBUFPENDING_SHIFT 28
    #define SST_SWAPBUFPENDING       (0x7<<SST_SWAPBUFPENDING_SHIFT)
    
    static void
    _atrGlideSwapBuffer( FxU32 sync ) {
        FxU32 counter = 0;
        /* spin while there is more than 1 swap_buffer command in the fifo */
    #ifdef GLIDE3
            FxU32 pending_buffers = 0;
            if ( !grGet( GR_PENDING_BUFFERSWAPS , 4, &pending_buffers ) )
                    atuError( FXTRUE, "Failed to get PENDING_BUFFERSWAPS.\n" );
        while( pending_buffers > 3 )
    #else
        while( ( ( grSstStatus() & SST_SWAPBUFPENDING ) >> 
                 SST_SWAPBUFPENDING_SHIFT ) > 3 )
    #endif
         {
            counter++;
            if ( counter > 16000000l ) {
                atuError( FXTRUE, "Infinite stall for swap buf pending.\n" );
            }
    #ifdef GLIDE3
                    if ( !grGet( GR_PENDING_BUFFERSWAPS , 4, &pending_buffers ) )
                            atuError( FXTRUE, "Failed to get PENDING_BUFFERSWAPS.\n" );
    #endif
        }
        grBufferSwap( sync );
    }
    
    /*-------------------------------------------------------------------
      Function: _atrTexAssociate
      Date: 7/1
      Implementor(s): jdt
      Library: AT Render
      Description:
      Associate an image with a texture handle given a bunch of information
      internal to the ATR texture manager.
      Arguments:
      entry  - texture list entry
      mmMask - mipmap level mask ( even, odd, both )
      largeLod - glide constant for largest lod level
      smallLod - glide constant for smallest lod level
      img      - pointer to image data structure
      Return:
      FXTRUE on success, FXFALSE otherwise
      -------------------------------------------------------------------*/
    static FxBool _atrTexAssociate(_AtrTexEntry    *entry,
                          FxU32           mmMask,
                          GrAspectRatio_t aspect,
                          GrLOD_t         largeLod,
                          GrLOD_t         smallLod,
                          AtrImg          *img) {
        FxU32 size;
        FxU32 tmu;
        GrTexInfo  grInfo;
        _AtrFXTexEntry *fxentry = (_AtrFXTexEntry *)entry;
        
        /*-----------------------------------------------------
          Discern TMU
          -----------------------------------------------------*/
        tmu  = entry->atrInfo.tmu;
    
        /*-----------------------------------------------------
          Fill GrTexInfo
          -----------------------------------------------------*/

#ifdef GLIDE3
        grInfo.smallLodLog2 = smallLod;
        grInfo.largeLodLog2 = largeLod;
        grInfo.aspectRatioLog2 = aspect;
#else
        grInfo.smallLod = smallLod;
        grInfo.largeLod = largeLod;
        grInfo.aspectRatio = aspect;
#endif /* GLIDE3 */
        grInfo.format = img->format;
        grInfo.data = img->data;
        
        /*-----------------------------------------------------
          Fill AtrTexInfo
          -----------------------------------------------------*/
        size = grTexTextureMemRequired( mmMask, &grInfo );
        
        if ( entry->state == ATR_TEXENT_CACHED ) {
            if ( size > entry->atrInfo.sizeInBytes )
              _atrTexPunt( entry );
            else 
              entry->dirty = FXTRUE;
        }
    
#ifdef GLIDE3
        switch( aspect ) {
          case GR_ASPECT_LOG2_8x1:
            fxentry->s_scale = 255.0f;
            fxentry->t_scale =  32.0f;
            break;
          case GR_ASPECT_LOG2_4x1:
            fxentry->s_scale = 255.0f;
            fxentry->t_scale =  64.0f;
            break;
          case GR_ASPECT_LOG2_2x1:
            fxentry->s_scale = 255.0f;
            fxentry->t_scale = 128.0f;
            break;
          case GR_ASPECT_LOG2_1x1:
            fxentry->s_scale = 255.0f;
            fxentry->t_scale = 255.0f;
            break;
          case GR_ASPECT_LOG2_1x2:
            fxentry->s_scale = 128.0f;
            fxentry->t_scale = 255.0f;
            break;
          case GR_ASPECT_LOG2_1x4:
            fxentry->s_scale =  64.0f;
            fxentry->t_scale = 255.0f;
            break;
          case GR_ASPECT_LOG2_1x8:
            fxentry->s_scale =  32.0f;
            fxentry->t_scale = 255.0f;
            break;
        }
#else
        switch( aspect ) {
          case GR_ASPECT_8x1:
            fxentry->s_scale = 255.0f;
            fxentry->t_scale =  32.0f;
            break;
          case GR_ASPECT_4x1:
            fxentry->s_scale = 255.0f;
            fxentry->t_scale =  64.0f;
            break;
          case GR_ASPECT_2x1:
            fxentry->s_scale = 255.0f;
            fxentry->t_scale = 128.0f;
            break;
          case GR_ASPECT_1x1:
            fxentry->s_scale = 255.0f;
            fxentry->t_scale = 255.0f;
            break;
          case GR_ASPECT_1x2:
            fxentry->s_scale = 128.0f;
            fxentry->t_scale = 255.0f;
            break;
          case GR_ASPECT_1x4:
            fxentry->s_scale =  64.0f;
            fxentry->t_scale = 255.0f;
            break;
          case GR_ASPECT_1x8:
            fxentry->s_scale =  32.0f;
            fxentry->t_scale = 255.0f;
            break;
        }
#endif /* GLIDE3 */    
        /*-----------------------------------------------------
          Copy data over to entry
          -----------------------------------------------------*/
        entry->atrInfo.sizeInBytes = size;
        fxentry->grInfo  = grInfo;
    
        return FXTRUE;
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideTexEntryInit
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
    _atrGlideTexEntryInit( _AtrTexEntry *e ) {
        _AtrFXTexEntry *entry = (_AtrFXTexEntry *)e;
    
        entry->startAddress = ATR_NULL_TEXADDR;
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideRealizeImg
      Date: 10/6
      Implementor(s): mlwp
      Library: AT Render
      Description:
      Load an image from a file
      Arguments:
          i        - pointer to image structure
          filename - name of image file
      Return:
          FXTRUE on success, FXFALSE otherwise
      TBD:
         Add conversion from other formats
      -------------------------------------------------------------------*/
    
    /* This calculates the maximum number of mipmaps a texture can have
       given its dimensions
       TBD: duplicates function in D3D driver move to common area
    */
    
    static int 
    CalculateMipMapLevels(int width, int height) {
      int LOD=0;
    
      while(width>0 && height>0) {
          width >>= 1; height >>= 1;
          LOD++;
      }
    
      return(LOD);
    }
    
    FxBool
    _atrGlideRemapTexture(AtrImg *img) {
        Gu3dfInfo info;
        FxU32 target_width, target_height, target_format, target_nlevels;
        size_t tex_mem_required;
        FxU32 flags = TX_DITHER_ERR ; /* TX_FIXED_PAL_QUANT_TABLE; */
    
        switch (img->format) {
        case ATR_IMGFMT_RGB_888:
            target_format = ATR_IMGFMT_RGB_565;
            break;
        case ATR_IMGFMT_ARGB_8888:
            target_format = ATR_IMGFMT_ARGB_4444;
            break;
        default:
            target_format = img->format;
            break;
        }
    
        target_width = img->width;
        target_height = img->height;
     
        /* make sure texture is within range, NB texus assumes textures must be <= 256 */
    
        while( target_width > 256 )
            target_width >>= 1;
        while( target_height > 256 )
            target_height >>= 1;
    
        target_nlevels=CalculateMipMapLevels(target_width, target_height);
    
        if (( target_width == img->width ) &&
             ( target_height == img->height ) &&
             ( target_format == img->format ) &&
             ( img->nLevels != 0 ))
            return FXTRUE;
    
        tex_mem_required = txInit3dfInfo( &info, target_format,
                                          &target_width, &target_height,
                                          target_nlevels, TX_AUTORESIZE_GROW );
        /*
         * Make sure txInit3dfInfo didn't fail.
         */
    
        if ( tex_mem_required == 0 ) {
            atuError( FXTRUE, "_atrGlideRemapTexture: Problem with txInit3dfInfo" );
        }
        
        /*
         * Allocate system memory for the texture.
         * TBD: texus does not use ATB's memory allocator so we better not use
         *      it here either. 
         */
    
        if ( ( info.data = malloc( tex_mem_required )) == NULL ) {
            atuError( FXTRUE,
                     "Out of memory allocating system memory textures." );
        }
        
        /*
         * Convert to a texture that can be downloaded by Glide.
         */
    
        txConvert(  &info, img->format, img->width, img->height,
                    img->data, flags, img->table );
    
        free(img->data);
    
        img->data    = info.data;
        img->format  = info.header.format;
        img->width   = info.header.width;
        img->height  = info.header.height;
        img->nLevels = target_nlevels;
    
        if (!(flags & TX_FIXED_PAL_QUANT_TABLE )) {
            if( ( img->format == ATR_IMGFMT_YIQ_422 ) ||
                ( img->format == ATR_IMGFMT_AYIQ_8422 ) ) {
              memcpy(img->table, info.table.palette.data, sizeof(AtrNCCTable));
            } else if( ( img->format == ATR_IMGFMT_P_8 ) || 
                       ( img->format == ATR_IMGFMT_AP_88 )) {
              memcpy(img->table, info.table.palette.data, sizeof(AtrPalette));
            }
        }
    
        return FXTRUE;
    }
    
    static FxBool 
    _atrGlideRealizeImg( AtrImg *i ) {
    
        /* TBD: use _atrGlideRemapTexture to handle cases where textures are too 
                large or we don't have mip map's and we want them.
         */
    
        if ( i->devPrivate == NULL ) {
            switch( i->format ) {
              case ATR_IMGFMT_YIQ_422:
              case ATR_IMGFMT_AYIQ_8422:
              case ATR_IMGFMT_P_8:
              case ATR_IMGFMT_AP_88:
                if ( !i->table ) 
                  atuError( FXTRUE, "atrGlideImageRealize: img requires "
                            "table and contains null table data.\n" );
                break;
              case ATR_IMGFMT_RGB_888:
              case ATR_IMGFMT_ARGB_8888:
                _atrGlideRemapTexture(i);
                break;
            }
            i->devPrivate = (void *)1;
        }
    
        return FXTRUE;
    }
    
    static void 
    _atrGlideUnrealizeImg( AtrImg *i ) {
        i->devPrivate = NULL;
    }
    
    static FxBool 
    _atrGlideCloneImg( AtrImg *dst, const AtrImg *src ) {
    
        FXUNUSED(src);
    
        dst->devPrivate = NULL;
        return FXTRUE;
    }
    
    /*-------------------------------------------------------------------
      Function: _atrTexLenToLOD
      Date: 7/1
      Implementor(s): jdt
      Library: AT Render
      Description:
      Return an GR_LOD for a given length, -1 if error.
      Arguments:
      len - length
      Return:
      lod constant
      -------------------------------------------------------------------*/
    static  GrLOD_t _atrTexLenToLOD( FxU32 len ) {
        GrLOD_t lod;  
    
#ifdef GLIDE3
        switch( len ) {
            case 256:
              lod = GR_LOD_LOG2_256;
              break;
            case 128:
              lod = GR_LOD_LOG2_128;
              break;
            case  64:
               lod = GR_LOD_LOG2_64;
              break;
            case  32:
              lod = GR_LOD_LOG2_32;
              break;
            case  16:
              lod = GR_LOD_LOG2_16;
              break;
            case   8:
              lod = GR_LOD_LOG2_8;
              break;
            case   4:
              lod = GR_LOD_LOG2_4;
              break;
            case   2:
              lod = GR_LOD_LOG2_2;
              break;
            case   1:
              lod = GR_LOD_LOG2_1;
              break;
            default:
              lod = -1;
              break;
        }
#else
        switch( len ) {
            case 256:
              lod = GR_LOD_256;
              break;
            case 128:
              lod = GR_LOD_128;
              break;
            case  64:
              lod = GR_LOD_64;
              break;
            case  32:
              lod = GR_LOD_32;
              break;
            case  16:
              lod = GR_LOD_16;
              break;
            case   8:
              lod = GR_LOD_8;
              break;
            case   4:
              lod = GR_LOD_4;
              break;
            case   2:
              lod = GR_LOD_2;
              break;
            case   1:
              lod = GR_LOD_1;
              break;
            default:
              lod = -1;
              break;
        }
#endif /* GLIDE3 */    
        return lod;
      }
    
    /*-------------------------------------------------------------------
      Function: _atrTexAspect
      Date: 7/1
      Implementor(s): jdt
      Library: AT Render
      Description:
      Return the aspect ratio given a width and height
      Arguments:
      width, height
      Return:
      glide aspect ratio constant
      -------------------------------------------------------------------*/
    static GrAspectRatio_t _atrTexAspect( FxU32 width, FxU32 height ) {
        GrAspectRatio_t aspect;
    
        width *= 8;
        width /= height;
#ifdef GLIDE3
        switch( width ) {
            case 64:
              aspect = GR_ASPECT_LOG2_8x1;
              break;
            case 32:
              aspect = GR_ASPECT_LOG2_4x1;
              break;
            case 16:
              aspect = GR_ASPECT_LOG2_2x1;
              break;
            case  8:
              aspect = GR_ASPECT_LOG2_1x1;
              break;
            case  4:
              aspect = GR_ASPECT_LOG2_1x2;
              break;
            case  2:
              aspect = GR_ASPECT_LOG2_1x4;
              break;
            case  1:
              aspect = GR_ASPECT_LOG2_1x8;
              break;
        }
#else
        switch( width ) {
            case 64:
              aspect = GR_ASPECT_8x1;
              break;
            case 32:
              aspect = GR_ASPECT_4x1;
              break;
            case 16:
              aspect = GR_ASPECT_2x1;
              break;
            case  8:
              aspect = GR_ASPECT_1x1;
              break;
            case  4:
              aspect = GR_ASPECT_1x2;
              break;
            case  2:
              aspect = GR_ASPECT_1x4;
              break;
            case  1:
              aspect = GR_ASPECT_1x8;
              break;
        }
#endif /* GLIDE3 */
        return aspect;
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideTexAssociate
      Date: 10/6
      Implementor(s): mlwp
      Library: AT Render
      Description:
        Associate a texture with an image
      Arguments:
          handle   - handle to texture
          img      - image to associate with handle
      Return:
          Nothing
      -------------------------------------------------------------------*/
    
    static FxBool 
    _atrGlideTexAssociate( AtrTexHandle handle, AtrImg *img ) {
        _AtrTexEntry *thisEntry;
        GrLOD_t smallLod, largeLod;
        GrAspectRatio_t aspect;
        FxU32 longSide, shortSide;
    
        /*-------------------------------------------------------
          Discern Texture Attributes
          -------------------------------------------------------*/
        if ( img->width > img->height ) {
            longSide  = img->width;
            shortSide = img->height;
        } else {
            longSide  = img->height;
            shortSide = img->width;
        }
    
        largeLod = _atrTexLenToLOD( longSide );
    #ifdef GLIDE3
        smallLod = largeLod - ( img->nLevels - 1 );
    #else
        smallLod = largeLod + img->nLevels - 1;
    #endif
    
        /*--------------------------------------------------------------
          Validate Image Data and attributes
          --------------------------------------------------------------*/
    #ifdef AT_DEBUGGING
        if ( !img->data )
          atuError( FXTRUE, "atrTexAssociate: img contains null data.\n" );
    
        if ( _atrTexLenToLOD( shortSide ) == -1 )
          atuError( FXTRUE, "atrTexAssociate: invalid source "
                   " image dimension.\n" );
        if ( largeLod == -1 ) 
          atuError( FXTRUE, "atrTexAssociate: invalid source "
                   " image dimension.\n" );
    #endif
    
        aspect   = _atrTexAspect( img->width, img->height );
    
        /*-------------------------------------------------------
          
          -------------------------------------------------------*/
        thisEntry = (_AtrTexEntry*)handle;
        if ( thisEntry->other ) {
            _atrTexAssociate(thisEntry,
                             GR_MIPMAPLEVELMASK_EVEN, 
                             aspect,
                             largeLod,
                             smallLod,
                             img);
            _atrTexAssociate((_AtrTexEntry*)thisEntry->other,
                             GR_MIPMAPLEVELMASK_ODD,
                             aspect,
                             largeLod,
                             smallLod,
                             img);
        } else {
            _atrTexAssociate(thisEntry,
                             GR_MIPMAPLEVELMASK_BOTH,
                             aspect,
                             largeLod,
                             smallLod,
                             img);
        }
    
        return FXTRUE;
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideTexSource
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
    _atrGlideTexSource( _AtrTexEntry *entry, FxU32 mask ) {
        _AtrFXTexEntry *fxentry = (_AtrFXTexEntry *)entry;
        FxU32 tmu = entry->atrInfo.tmu;
    
        /*-----------------------------------------------------------
          fix texture scaling
          -----------------------------------------------------------*/
          _atrTexSetScale( tmu, fxentry->s_scale, fxentry->t_scale );
    
        /*-----------------------------------------------------------
          Download Texture/Tables If Necessary
          -----------------------------------------------------------*/
        if ( entry->dirty ) {
    #ifdef AT_STATISTICS
            _atrStatsTexMemoryDownload( tmu, entry->atrInfo.sizeInBytes );
    #endif  
            grTexDownloadMipMap( tmu, 
                                 fxentry->startAddress,
                                 mask,
                                 &fxentry->grInfo );
            tfxState[tmu].startAddress = fxentry->startAddress;
            entry->dirty = FXFALSE;
        }
          
        switch( entry->atrInfo.img->format ) {
          case ATR_IMGFMT_P_8:
          case ATR_IMGFMT_AP_88:
            if ( entry->atrInfo.img->table != tfxState[tmu].pal ) {
    #ifdef AT_STATISTICS
                _atrCurrentStats.paletteDownloads[tmu]++;
    #endif  
    #ifdef GLIDE3
                grTexDownloadTable(
    #else 
                grTexDownloadTable(tmu, 
    #endif
                                   GR_TEXTABLE_PALETTE, 
                                   entry->atrInfo.img->table );
                tfxState[tmu].pal = entry->atrInfo.img->table;
            }
            break;
          case ATR_IMGFMT_YIQ_422:
          case ATR_IMGFMT_AYIQ_8422:
            if ( entry->atrInfo.img->table != tfxState[tmu].ncc[0] &&
                 entry->atrInfo.img->table != tfxState[tmu].ncc[1] ) {
                GrTexTable_t table;
                if ( tfxState[tmu].nextNcc ) 
                  table = GR_TEXTABLE_NCC0;
                else
                  table = GR_TEXTABLE_NCC1;
    #ifdef AT_STATISTICS
                _atrCurrentStats.nccTableDownloads[tmu]++;
    #endif  
    #ifdef GLIDE3
                grTexDownloadTable(
    #else 
                grTexDownloadTable(tmu, 
    #endif
                                   table,
                                   entry->atrInfo.img->table );
    #ifdef GLIDE3
                grTexNCCTable( table );
    #else
                grTexNCCTable( tmu, table );
    #endif
                tfxState[tmu].nextNcc ^= 1;
                if ( table == GR_TEXTABLE_NCC0 )
                    tfxState[tmu].ncc[0] = entry->atrInfo.img->table;
                else if ( table == GR_TEXTABLE_NCC1 )
                    tfxState[tmu].ncc[1] = entry->atrInfo.img->table;
            }
            break;
        }
    
        /*-----------------------------------------------------------
          Source Texture
          -----------------------------------------------------------*/
        grTexSource( tmu, fxentry->startAddress, mask, &fxentry->grInfo );
    }
    
    /*-------------------------------------------------------------------
      Function: _atrNextFreeAddress
      Date: 7/1
      Implementor(s): jdt
      Library: AT Render
      Description:
      Come up with the next available address, 
      freeing entries if necessary
      Arguments:
      Return:
      -------------------------------------------------------------------*/
    
    static FxU32 _atrNextFreeAddress( AtrTexelFx tmu, FxU32 size ) {
        
        FxU32 freeSpace;
        FxU32 segBase, segExtent;
        _AtrFXTexEntry *first, *last;
    
        /*-----------------------------------------------------------
          Determine segBase and segExtent the lower and upper bounds
          on allocated texram.
          -----------------------------------------------------------*/
        if ( _inCache[tmu] ) {
            first = (_AtrFXTexEntry *)_inCache[tmu];
            segBase = first->startAddress;
            last = first;
            while( last->any.next ) last = (_AtrFXTexEntry *)(last->any.next);
            segExtent = last->startAddress + last->any.atrInfo.sizeInBytes;
            /*-----------------------------------------------------------
              Calculate Free Space
              -----------------------------------------------------------*/
            if ( segBase > segExtent ) {
                freeSpace = segBase - segExtent;
            } else if ( segBase < segExtent ) {
                freeSpace = _atTexMaxAddress[tmu] - segExtent;
                if ( freeSpace < size ) { /* handle wraparound */
                    segExtent = 0;
                    freeSpace = segBase - segExtent;
                }
            } else { /* segBase == segExtent */
                freeSpace = 0;
            }
        } else { /* texram empty */
            segBase   = 0;
            segExtent = 0;
            freeSpace = _atTexMaxAddress[tmu];
        }
    
        /*-----------------------------------------------------------
          Free Up Space As Necessary
          -----------------------------------------------------------*/
        while ( freeSpace < size ) {
            _atrTexPunt( _inCache[tmu] );
            segBase = ((_AtrFXTexEntry *)_inCache[tmu])->startAddress;
            if ( segBase > segExtent ) {
                freeSpace = segBase - segExtent;
            } else if ( segBase < segExtent ) {
                freeSpace = _atTexMaxAddress[tmu] - segExtent;
                if ( freeSpace < size ) { /* handle wraparound */
                    segExtent = 0;
                    freeSpace = segBase - segExtent;
                }
            } else { /* segBase == segExtent */
                freeSpace = 0;
            }
        }
    
        /* quick hack for 2MB boundary */
    
        if ((segExtent < 0x200000) && (segExtent + size > 0x200000)) {
            segExtent = 0x200000;
            if ( segBase > segExtent ) {
                 while ((((_AtrFXTexEntry *)_inCache[tmu])->startAddress) <
                         ( segExtent + size ))
                    _atrTexPunt( _inCache[tmu] );
            }
        }
    
        return segExtent;
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideTramAllocate
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
    
    static void 
    _atrGlideTramAllocate( _AtrTexEntry *entry ) {
        FxU32 tmu = entry->atrInfo.tmu;
    
        _AtrFXTexEntry *fxentry = (_AtrFXTexEntry*)entry;
        fxentry->startAddress = 
              _atrNextFreeAddress( tmu, entry->atrInfo.sizeInBytes );
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideTexPunt
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
    _atrGlideTexPunt( _AtrTexEntry *e ) {
        _AtrFXTexEntry *entry = (_AtrFXTexEntry *)e;
    
        entry->startAddress = ATR_NULL_TEXADDR;
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideTexDeleteHandle
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
    _atrGlideTexDeleteHandle( _AtrTexEntry *e) {
        _AtrFXTexEntry *entry = (_AtrFXTexEntry *)e;
    
        entry->startAddress = ATR_NULL_TEXADDR;
        entry->grInfo.data = 0;
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideUpdateEnv
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
    static void 
    _atrGlideUpdateEnv( AtrEnv *e ) {
        FxU32 fcR, fcG, fcB;
    
        if ( e->flags & ATR_FOG_TABLE_UPDATE ) {
            switch( e->fogFunc ) {
            case ATR_FOGFUNC_LINEAR:
                guFogGenerateLinear( (GrFog_t*)&e->fogTable,
                                     e->fogNearW, e->fogFarW );
                break;
            case ATR_FOGFUNC_EXP:
                guFogGenerateExp( (GrFog_t*)&e->fogTable, e->fogDensity );
                break;
            case ATR_FOGFUNC_EXP2:
                guFogGenerateExp2( (GrFog_t*)&e->fogTable, e->fogDensity );
                break;
            default:
                break;
            }
        }
    
        switch( e->flags & ATR_HSR_MASK ) {
            case ATR_HSR_NONE:
                grDepthBufferMode( GR_DEPTHBUFFER_DISABLE );
                break;
            case ATR_HSR_WBUFFER:
                grDepthBufferMode( GR_DEPTHBUFFER_WBUFFER );
                grDepthBufferFunction( GR_CMP_LEQUAL );
                grDepthMask( FXTRUE );
                break;
            default:
                break;
        }
    
        switch( e->flags & ATR_FOG_MASK ) {
            case ATR_FOG_ON_DEPTH:
#ifdef GLIDE3
                grFogMode( GR_FOG_WITH_TABLE_ON_Q );
#else
                grFogMode( GR_FOG_WITH_TABLE );
#endif
                grFogTable( (GrFog_t*)&e->fogTable );
                fcR = (((FxU32)(e->fogColor.r * 255.0f))&0xff)<<16;
                fcG = (((FxU32)(e->fogColor.g * 255.0f))&0xff)<<8;
                fcB = (((FxU32)(e->fogColor.b * 255.0f))&0xff)<<0;
                grFogColorValue( fcR | fcG | fcB );
                break;
            case ATR_FOG_ON_DEPTH_MP:
#ifdef GLIDE3
                grFogMode( GR_FOG_ADD2 | GR_FOG_WITH_TABLE_ON_Q );
#else
                grFogMode( GR_FOG_ADD2 | GR_FOG_WITH_TABLE );
#endif
                grFogTable( (GrFog_t*)&e->fogTable );
                fcR = (((FxU32)(e->fogColor.r * 255.0f))&0xff)<<16;
                fcG = (((FxU32)(e->fogColor.g * 255.0f))&0xff)<<8;
                fcB = (((FxU32)(e->fogColor.b * 255.0f))&0xff)<<0;
                grFogColorValue( fcR | fcG | fcB );
                break;
            default:
                grFogMode( GR_FOG_DISABLE );
                break;
        }
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideUpdateMaterial
      Date: 10/6
      Implementor(s): mlwp
      Library: AT Render
      Description:
      Set the current material
      Arguments:
          m        - new material
      Return:
          Nothing
      -------------------------------------------------------------------*/
    static FxU32 hardware_ac=0xFFFFFFFF;
    static FxU32 hardware_cc=0xFFFFFFFF;
    static FxU32 hardware_ab=0xFFFFFFFF;
    static FxU32 hardware_tc0=0xFFFFFFFF;
    static FxU32 hardware_hints=0xFFFFFFFF;
    static FxU32 hardware_tex_filter=0xFFFFFFFF;
    static FxU32 hardware_clamp_mode=0xFFFFFFFF;
    static FxU32 hardware_mipmap_mode=0xFFFFFFFF;
    static FxU32 hardware_alpha_test=0xFFFFFFFF;
    static FxU32 hardware_alpha_ref_value=0xFFFFFFFF;
    static FxU32 hardware_depth=0xFFFFFFFF;
    static FxU32 hardware_fog=0xFFFFFFFF;
    static FxU32 hardware_tex_source0=0xFFFFFFFF;
    static FxU32 hardware_texture_combine_src=0xFFFFFFFF;
    
    void 
    _atrGlideUpdateMaterial( AtrMaterial *m ) {
        FxU32  hints = 0;
        FxU32  temp;
    
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
    
        if (hardware_texture_combine_src!=m->tcSrc[0])
        {
           switch( m->tcSrc[0] ) {
             case ATR_TCSRC_TC0:
               _atrRenderCache->texCoordSrcFunc[0] = _atrTCSRC0_TC0;
               break;
             case ATR_TCSRC_TC1:
               _atrRenderCache->texCoordSrcFunc[0] = _atrTCSRC0_TC1;
               break;
             case ATR_TCSRC_TC2:
               _atrRenderCache->texCoordSrcFunc[0] = _atrTCSRC0_TC2;
               break;
             case ATR_TCSRC_EMAP:
               _atrRenderCache->texCoordSrcFunc[0] = _atrTCSRC0_EMAP;
               break;
             case ATR_TCSRC_LMAP:
               _atrRenderCache->texCoordSrcFunc[0] = 0;
               break;
             case ATR_TCSRC_PROJECTED:
               _atrRenderCache->texCoordSrcFunc[0] = _atrTCSRC0_PROJECTED;
               break;
             case ATR_TCSRC_PLANAR:
               _atrRenderCache->texCoordSrcFunc[0] = _atrTCSRC0_PLANAR;
               break;
			   // Denis. Fixing detail textures
             case ATR_TCSRC_TC0_SRC1_SCALE_TC0:
               _atrRenderCache->texCoordSrcFunc[0] = _atrTCSRC0_TC0_SRC1_SCALE_TC0;
               break;
             case ATR_TCSRC_TC0_SCALE:
               _atrRenderCache->texCoordSrcFunc[0] = _atrTCSRC0_TC0_SCALE;
               break;
             default:
               _atrRenderCache->texCoordSrcFunc[0] = 0;
               break;
           }
           hardware_texture_combine_src=m->tcSrc[0];
        }
        switch( m->tcSrc[1] ) {
			case ATR_TCSRC_TC0:
				_atrRenderCache->texCoordSrcFunc[1] = _atrTCSRC1_TC0;
				break;
			case ATR_TCSRC_TC1:
				_atrRenderCache->texCoordSrcFunc[1] = _atrTCSRC1_TC1;
				break;
			case ATR_TCSRC_TC2:
				_atrRenderCache->texCoordSrcFunc[1] = _atrTCSRC1_TC2;
				break;
			case ATR_TCSRC_EMAP:
				_atrRenderCache->texCoordSrcFunc[1] = _atrTCSRC1_EMAP;
				break;
			case ATR_TCSRC_LMAP:
				_atrRenderCache->texCoordSrcFunc[1] = 0;
				break;
			case ATR_TCSRC_PROJECTED:
				_atrRenderCache->texCoordSrcFunc[1] = _atrTCSRC1_PROJECTED;
				break;
			case ATR_TCSRC_PLANAR:
				_atrRenderCache->texCoordSrcFunc[1] = _atrTCSRC1_PLANAR;
				break;
			// Denis. Fixing detail textures
			case ATR_TCSRC_TC0_SRC1_SCALE_TC0:
				_atrRenderCache->texCoordSrcFunc[1] = _atrTCSRC0_TC0_SRC1_SCALE_TC0;
				break;
			case ATR_TCSRC_TC0_SCALE:
				_atrRenderCache->texCoordSrcFunc[1] = _atrTCSRC1_TC1_SCALE;
				break;
			default:
				_atrRenderCache->texCoordSrcFunc[1] = 0;
				break;
        }
    
        /* Set material flags */
        m->sysFlags |= m->texSrc[0] | (m->texSrc[1]<<ATR_TEX1SHIFT);
    
        switch( m->texSrc[0] ) {
          case ATR_TEXSRC_DECAL:
          case ATR_TEXSRC_EMAP:
    #ifdef AT_DEBUGGING
            if ( m->texture[0] == 0 ) 
              atuError( FXTRUE, 
                       "atrPushMaterial(): No texture to source.\n" ); 
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
              atuError( FXTRUE, 
                       "atrPushMaterial(): No texture to source.\n" ); 
            if ( _atrGlideDriver.caps.numTex != 2 ) 
              atuError( FXTRUE,
                       "atrPushMaterial(): Tried to push a multiple "
                       "texelFx material on a single texelFx system.\n" ); 
            if (_atrTexTmuFromHandle( m->texture[0] ) == ATR_TEXELFX_1)
              atuError( FXTRUE,
                       "atrPushMaterial(): Texture handle mismatch, the ->texture[0] allocated\n"
                       "on TEXELFX 1\n" ); 
    #endif
            atrTexSource( m->texture[0] );
			
            grTexDetailControl( GR_TMU0, 
								// Denis. Fixing texture detail
                               (FxU8)m->texDetail[0].bias,
                               (FxU8)m->texDetail[0].scale,
                               m->texDetail[0].max );
                               
							   //(FxU8)m->texDetail[1].bias,
                               //(FxU8)m->texDetail[1].scale,
                               //m->texDetail[1].max );
							   
			
            break;
          case ATR_TEXSRC_PROJECTED:
    #ifdef AT_DEBUGGING
            if ( m->texture[0] == 0 ) 
              atuError( FXTRUE, 
                       "atrPushMaterial(): No texture to source.\n" ); 
            if (_atrTexTmuFromHandle( m->texture[0] ) == ATR_TEXELFX_1)
              atuError( FXTRUE,
                       "atrPushMaterial(): Texture handle mismatch, the ->texture[0] allocated\n"
                       "on TEXELFX 1\n" ); 
    #endif
            hints |= GR_STWHINT_W_DIFF_TMU0;
            atrTexSource( m->texture[0] );
            break;
          case ATR_TEXSRC_LMAP:
          case ATR_TEXSRC_NONE:
          default:
            break;
        }
    
        switch( m->texSrc[1] ) {
          case ATR_TEXSRC_DECAL:
          case ATR_TEXSRC_EMAP:
    #ifdef AT_DEBUGGING
            if ( m->texture[1] == 0 ) 
              atuError( FXTRUE, 
                       "atrPushMaterial(): No texture to source.\n" ); 
            if ( _atrGlideDriver.caps.numTex != 2 ) 
              atuError( FXTRUE,
                       "atrPushMaterial(): Tried to push a multiple "
                       "texelFx material on a single texelFx system.\n" ); 
            if (_atrTexTmuFromHandle( m->texture[1] ) == ATR_TEXELFX_0)
              atuError( FXTRUE,
                       "atrPushMaterial(): Texture handle mismatch, the ->texture[1] allocated\n"
                       "on TEXELFX 0\n" ); 
    #endif
            atrTexSource( m->texture[1] );
			hints |= GR_STWHINT_ST_DIFF_TMU0 | GR_STWHINT_ST_DIFF_TMU1;
            break;
          case ATR_TEXSRC_DETAIL:
    #ifdef AT_DEBUGGING
            if ( m->texture[1] == 0 ) 
              atuError( FXTRUE, 
                       "atrUpdateMaterial(): No texture to source.\n" ); 
            if ( _atrGlideDriver.caps.numTex != 2 ) 
              atuError( FXTRUE,
                       "atrUpdateMaterial(): Tried to push a multiple "
                       "texelFx material on a single texelFx system.\n" ); 
    #endif
            atrTexSource( m->texture[1] );
            grTexDetailControl( GR_TMU1, 
                               (FxU8)m->texDetail[1].bias,
                               (FxU8)m->texDetail[1].scale,
                               m->texDetail[1].max );
            // DENIS . debugging detail textures 
            hints |= GR_STWHINT_ST_DIFF_TMU0 | GR_STWHINT_ST_DIFF_TMU1;
            break;
          case ATR_TEXSRC_PROJECTED:
            hints |= GR_STWHINT_ST_DIFF_TMU0 | GR_STWHINT_ST_DIFF_TMU1 | 
                     GR_STWHINT_W_DIFF_TMU1 | GR_STWHINT_W_DIFF_TMU0;
            atrTexSource( m->texture[1] );
            break;
          case ATR_TEXSRC_LMAP:
            hints |= GR_STWHINT_ST_DIFF_TMU1;
            break;
          case ATR_TEXSRC_NONE:
          default:
            break;
        }
    
    #ifdef GLIDE3
    //#ifdef GLIDE3_buggy
        /*
        ** grHint has become obsolete in Glide3. The explicit
        ** use of grVertexLayout( GR_PARAM_ST1, ...) indicates
        ** that ST0 and ST1 are different. Use the flag GR_INHERIT
        ** otherwise.
            **
            ** WELL, THAT'S NOT TRUE! it looks like a bug, but if
            ** I don't use the grHints under glide3 on a 2 tmu system
            ** flipper will fail (the 2nd light texture won't slide).
        */
         if (hints!=hardware_hints) {
            if ( hints & GR_STWHINT_W_DIFF_TMU0 )
                 grVertexLayout( GR_PARAM_Q0, PARAM_W0_OFFSET, GR_PARAM_ENABLE );
            else grVertexLayout( GR_PARAM_Q0, PARAM_W_OFFSET,  GR_PARAM_ENABLE );

            if ( hints & GR_STWHINT_ST_DIFF_TMU1 ) {
                 grVertexLayout( GR_PARAM_ST1, PARAM_ST1_OFFSET, GR_PARAM_ENABLE );
                 if ( hints & GR_STWHINT_W_DIFF_TMU1 )
                      grVertexLayout( GR_PARAM_Q1, PARAM_W1_OFFSET, GR_PARAM_ENABLE );
                 else grVertexLayout( GR_PARAM_Q1, PARAM_W_OFFSET, GR_PARAM_ENABLE );
            } else {
                grVertexLayout( GR_PARAM_ST1, PARAM_ST1_OFFSET, GR_PARAM_DISABLE );
                grVertexLayout( GR_PARAM_Q1, PARAM_W1_OFFSET, GR_PARAM_DISABLE );
            }
            hardware_hints = hints;
        }
    #else
        if (hints!=hardware_hints)
        {
           grHints( GR_HINT_STWHINT, hints );
           hardware_hints=hints;
        }
    #endif
    
        switch( m->crgbSrc ) {
          case ATR_CRGBSRC_STATIC:
            grConstantColorValue( m->constant );
            break;
          default:
            break;
        }
    
   #ifdef GLIDE3
        if (m->chromaKeyEnable) {
          if (grChromaRangeMode)
            grChromaRangeMode( GR_CHROMAKEY_ENABLE );
          if (grChromaRange)
            grChromaRange( m->chromaKeyValue, m->chromaKeyValue, GR_CHROMARANGE_RGB_ALL_EXT);
        } else {
          if (grChromaRangeMode)
            grChromaRangeMode( GR_CHROMAKEY_DISABLE );
        }
   #else
        if (m->chromaKeyEnable) {
            grChromakeyMode( GR_CHROMAKEY_ENABLE );
            grChromakeyValue( m->chromaKeyValue);
        } else grChromakeyMode( GR_CHROMAKEY_DISABLE );
   #endif

        grCullMode( (m->isTwoSided)?GR_CULL_DISABLE:GR_CULL_POSITIVE );
    
        if (hardware_alpha_test!=m->atestFunc)
        {
           grAlphaTestFunction( m->atestFunc );
           hardware_alpha_test=m->atestFunc;
        }
        if (hardware_alpha_ref_value!=m->aReference)
        {
           grAlphaTestReferenceValue( m->aReference );
           hardware_alpha_ref_value=m->aReference;
        }
    
        temp=(m->texMinFilter[0]<<8)+(m->texMagFilter[0]);
        if(hardware_tex_filter!=temp)
        {
           grTexFilterMode( GR_TMU0, m->texMinFilter[0], m->texMagFilter[0] );
           hardware_tex_filter=temp;
        }
    
        temp=(m->texSClamp[0]<<8)+(m->texTClamp[0]);
        if(hardware_clamp_mode!=temp)
        {
           grTexClampMode( GR_TMU0, m->texSClamp[0], m->texTClamp[0] );
           hardware_clamp_mode=temp;
        }
    
        temp=(m->texMMMode[0]<<8)+m->texLODBlend[0];
        if(hardware_mipmap_mode!=temp)
        {
           grTexMipMapMode( GR_TMU0, 
                            m->texMMMode[0],
                            m->texLODBlend[0] );
           hardware_mipmap_mode=temp;
        }
                              
        if ( _atrGlideDriver.caps.numTex == 2 ) {
            grTexFilterMode( GR_TMU1, m->texMinFilter[1], m->texMagFilter[1] );
            grTexClampMode( GR_TMU1, m->texSClamp[1], m->texTClamp[1] );
            grTexMipMapMode( GR_TMU1, 
                             m->texMMMode[1],
                             m->texLODBlend[1] );
            grTexCombine( GR_TMU1, m->tcuCFunction[1], m->tcuCFactor[1],
                          m->tcuAFunction[1], m->tcuAFactor[1],
                          m->tcuCInvert[1], m->tcuAInvert[1] );
        }
    
        if (m->texture_combine[0]!=hardware_tc0)
        {
           grTexCombine( GR_TMU0, m->tcuCFunction[0], m->tcuCFactor[0],
                         m->tcuAFunction[0], m->tcuAFactor[0],
                         m->tcuCInvert[0], m->tcuAInvert[0] );
           hardware_tc0=m->texture_combine[0];
        }
    
        if (m->alpha_combine!=hardware_ac)
        {
           grAlphaCombine( m->acuFunction, m->acuFactor, m->acuLocal,
                           m->acuOther, m->acuInvert );
           hardware_ac=m->alpha_combine;
        }
    
        if (m->color_combine!=hardware_cc)
        {
           grColorCombine( m->ccuFunction, m->ccuFactor, m->ccuLocal, 
                           m->ccuOther, m->ccuInvert );
           hardware_cc=m->color_combine;
        }
    
        if (m->alpha_blend!=hardware_ab)
        {
             grAlphaBlendFunction( m->abuSrcFactor, m->abuDstFactor, 
                                   GR_BLEND_ONE, GR_BLEND_ZERO );
             hardware_ab=m->alpha_blend;
        }
    
        if ( _atrCurrentEnv->flags & ATR_FOG_ON_DEPTH_MP ) {
            if ( ( ( m->typeFlag & ATR_MAT_FB_MASK ) == ATR_MAT_FB_MULT_MP_FOG ) )
            {
               if(hardware_fog!=1)
               {
#ifdef GLIDE3
                  grFogMode( GR_FOG_MULT2 | GR_FOG_WITH_TABLE_ON_Q );
#else
                  grFogMode( GR_FOG_MULT2 | GR_FOG_WITH_TABLE );
#endif
                  hardware_fog=1;
               }
            }
            else
            {
               if (hardware_fog!=2)
               {
#ifdef GLIDE3
                  grFogMode( GR_FOG_ADD2 | GR_FOG_WITH_TABLE_ON_Q );
#else
                  grFogMode( GR_FOG_ADD2 | GR_FOG_WITH_TABLE );
#endif
                  hardware_fog=2;
               }
            }
        }
        if ( _atrGlideDriver.caps.hasDepth )
        {
           if (hardware_depth!=m->depthMask)
           {
              grDepthMask( m->depthMask );
              hardware_depth=m->depthMask;
           }
           
        }
        return;
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideInit
      Date: 10/6
      Implementor(s): mlwp
      Library: AT Render
      Description:
        Initialize the Glide driver
      Arguments:
          driver info - descibing specific configaration
      Return:
          Initialized driver
      -------------------------------------------------------------------*/
    
    AtrDriver *
    _atrGlideInit( AtrDriverInfo *driverInfo ) {
        FxU32 width = driverInfo->width;
        FxU32 height = driverInfo->height;
        FxU32 tmu;
        AtrDriverCaps *caps = &_atrGlideDriver.caps;
        GrScreenResolution_t resolution;
        FxBool open = FXFALSE;
    #ifdef GLIDE3
        FxU32 num_boards, num_fb;
        FxU32 fb_revision[3], fb_memory[3],
                  tmu_revision[3], tmu_memory[3];
        FxU32 get_viewport[4];
    #endif
    
        fpu_single_precision();
    
        if ( getenv("SST_DUALHEAD") != NULL ) {
            gDualMonitors = FXTRUE;
        }
    
        /* !!JDT Hack.  Need to be able to pass down a default resolution
           !!JDT and refresh */
        if ( getenv("ARCADE_RES_OVERRIDE") ) {
            width  = 320;
            height = 200;
        }
    
        _atrSnapBias = ATR_SNAP_BIAS;
        _atrDriverXOffset = 0.1f;
        _atrDriverYOffset = 0.1f;
    
        switch( width ) {
            case 320:
                switch( height ) {
                    case 200:
                        _atrGlideDriver.caps.hasDepth = FXTRUE;
                        resolution = GR_RESOLUTION_320x200;
                        break;
                    case 240:
                        _atrGlideDriver.caps.hasDepth = FXTRUE;
                        resolution = GR_RESOLUTION_320x240;
                        break;
                    default:
                        atuError( FXFALSE, 
                                  "atrGlideInit(): Unknown resolution %dx%d.\n"
                                  "Defaulting to 640z480\n", width, height );
                        resolution = GR_RESOLUTION_640x480;
                        break;
                }
                break;
            case 400:
                switch( height ) {
                    case 256:
                        _atrGlideDriver.caps.hasDepth = FXTRUE;
                        resolution = GR_RESOLUTION_400x256;
                        break;
                    default:
                        atuError( FXFALSE, 
                                  "atrGlideInit(): Unknown resolution %dx%d.\n"
                                  "Defaulting to 640z480\n", width, height );
                        resolution = GR_RESOLUTION_640x480;
                        break;
                }
                break;
            case 512:
                switch( height ) {
                    case 384:
                        _atrGlideDriver.caps.hasDepth = FXTRUE;
                        resolution = GR_RESOLUTION_512x384;
                        break;
                    default:
                        atuError( FXFALSE, 
                                  "atrGlideInit(): Unknown resolution %dx%d.\n"
                                  "Defaulting to 640z480\n", width, height );
                        resolution = GR_RESOLUTION_640x480;
                        break;
                }
                break;
            case 640:
                switch( height ) {
                    case 350:
                        resolution = GR_RESOLUTION_640x350;
                        _atrGlideDriver.caps.hasDepth = FXTRUE;
                        break;
                    case 400:
                        resolution = GR_RESOLUTION_640x400;
                        _atrGlideDriver.caps.hasDepth = FXTRUE;
                        break;
                    case 480:
                        resolution = GR_RESOLUTION_640x480;
                        _atrGlideDriver.caps.hasDepth = FXTRUE;
                        break;
                    default:
                        atuError( FXFALSE, 
                                  "atrGlideInit(): Unknown resolution %dx%d.\n"
                                  "Defaulting to 640z480\n", width, height );
                        resolution = GR_RESOLUTION_640x480;
                        break;
                }
                break;
            case 800:
                switch( height ) {
                    case 600:
                        resolution = GR_RESOLUTION_800x600;
                        break;
                    default:
                        atuError( FXFALSE, 
                                  "atrGlideInit(): Unknown resolution %dx%d.\n"
                                  "Defaulting to 640z480\n", width, height );
                        resolution = GR_RESOLUTION_640x480;
                        break;
                }
                break;
            case 1024:
                switch( height ) {
                    case 768:
                        resolution = GR_RESOLUTION_1024x768;
                        break;
                    default:
                        atuError( FXFALSE, 
                                  "atrGlideInit(): Unknown resolution %dx%d.\n"
                                  "Defaulting to 640z480\n", width, height );
                        resolution = GR_RESOLUTION_640x480;
                        break;
                }
                break;
            case 1280:
                switch( height ) {
                    case 1024:
                        resolution = GR_RESOLUTION_1280x1024;
                        break;
                    default:
                        atuError( FXFALSE, 
                                  "atrGlideInit(): Unknown resolution %dx%d.\n"
                                  "Defaulting to 640z480\n", width, height );
                        resolution = GR_RESOLUTION_640x480;
                        break;
                }
                break;
            case 1600:
                switch( height ) {
                    case 1200:
                        resolution = GR_RESOLUTION_1600x1200;
                        break;
                    default:
                        atuError( FXFALSE, 
                                  "atrGlideInit(): Unknown resolution %dx%d.\n"
                                  "Defaulting to 640z480\n", width, height );
                        resolution = GR_RESOLUTION_640x480;
                        break;
                }
                break;
            default:
                atuError( FXFALSE, 
                          "atrGlideInit(): Unknown resolution %dx%d.\n"
                          "Defaulting to 640z480\n", width, height );
                resolution = GR_RESOLUTION_640x480;
                break;
        }
    
        grGlideInit();
    
        grErrorSetCallback(glideErrorCallback);
    
    #ifdef GLIDE3
        /* Detect Hardware, and fill in the fields in the legacy structure
        ** atrHwConfig
        */
        /* First, lets make sure there is at least one board */
            if ( !grGet( GR_NUM_BOARDS, 4, &num_boards ) )
                atuError( FXTRUE, "Failed to query for the number of Voodoo boards.\n" );
        if ( num_boards == 0 )
                atuError( FXTRUE, "Found no Voodoo board compatible with the current glide dll.\n" );
    
    #else
        if ( !grSstQueryHardware( &_atrHwConfig ) ) {
            atuError( FXTRUE, "Couldn't detect the SST-1\n." );
            return NULL;
        }
    #endif
    
        grSstSelect( 0 );
    
        if (!driverInfo->fullScreen) {
            if ( grSstWinOpen( 
                          (FxU32)driverInfo->hWnd,
                          GR_RESOLUTION_NONE,
                          GR_REFRESH_60Hz, 
                          GR_COLORFORMAT_ARGB,
                          GR_ORIGIN_LOWER_LEFT,
                          2, 1 ) ) {
                open = FXTRUE;
            } else {
                _atrGlideDriver.caps.fullScreen = FXTRUE;
            }
        }
    
        if (!open) {
            if ( grSstWinOpen( 
                              0,
                              resolution,
                              GR_REFRESH_60Hz, 
                              GR_COLORFORMAT_ARGB,
                              GR_ORIGIN_LOWER_LEFT,
                              2, 1 ) ) {
                            _atrGlideDriver.caps.fullScreen = FXTRUE;
                    } else {
                atuError( FXTRUE, "Couldn't initialize graphics engine.\n" );
                return NULL;
            }
        };
    
    #ifdef GLIDE3
        /* Should I call grViewport?... or is there a default value?? */
            grGet( GR_VIEWPORT, sizeof( get_viewport ), get_viewport );
        caps->width =  ((FxI32) get_viewport[2] ) - ((FxI32) get_viewport[0]);
        caps->height =  ((FxI32) get_viewport[3] ) - ((FxI32) get_viewport[1]);
            grGet( GR_BITS_DEPTH, 4, &(caps->bpp) );
    
        caps->width = 640;
        caps->height = 480;
        caps->bpp = 16;
    #else
        caps->width  = grSstScreenWidth();
        caps->height = grSstScreenHeight();
        caps->bpp  = 16;
    #endif
    
        grClipWindow( 0, 0, _atrGlideDriver.caps.width - 1, 
                            _atrGlideDriver.caps.height - 1);
    
        grCullMode( GR_CULL_POSITIVE );
    
    #ifdef GLIDE3
        /* Note. This code does not deal with UMA.
        ** it's ok for Voodoo, Rush & Voodoo2 
        ** but it will have to be updated for Banshee
        */
            grGet( GR_REVISION_FB, sizeof( fb_revision ), fb_revision ); /* pixelFx revision # */
        caps->pfxRev = fb_revision[0];
            grGet( GR_MEMORY_FB, sizeof( fb_memory ), fb_memory );  /* ram per pixelFx */
        caps->pfxMem = fb_memory[0];
            grGet( GR_NUM_TMU, 4, &(caps->numTex) );    /* # of TMU per pixelFx */
    
            // DEBUG force numTex to 1
            //caps->numTex = 1;
    
    
            grGet( GR_NUM_FB, 4, &num_fb );                      /* # of pixelFx. if > 1 then this is an SLI */
        if ( num_fb > 1 ) caps->sli = FXTRUE;
        else caps->sli = FXFALSE;
            grGet( GR_REVISION_TMU, sizeof( tmu_revision ), tmu_revision );
            grGet( GR_MEMORY_TMU, sizeof( tmu_memory ), tmu_memory );
        for ( tmu = 0; tmu < ATR_NUM_TFX; tmu++ ) {
            caps->tfxConfig[tmu].tfxRev = tmu_revision[tmu];
            caps->tfxConfig[tmu].tfxMem = tmu_memory[tmu];
        }
    #else
        switch ( _atrHwConfig.SSTs[0].type ) {
        case GR_SSTTYPE_VOODOO:
        case GR_SSTTYPE_Voodoo2:
            caps->pfxRev = _atrHwConfig.SSTs[0].sstBoard.VoodooConfig.fbiRev;
            caps->pfxMem = _atrHwConfig.SSTs[0].sstBoard.VoodooConfig.fbRam;
            caps->numTex = _atrHwConfig.SSTs[0].sstBoard.VoodooConfig.nTexelfx;
            for ( tmu = 0; tmu < ATR_NUM_TFX; tmu++ ) {
                caps->tfxConfig[tmu].tfxRev = 
                    _atrHwConfig.SSTs[0].sstBoard.VoodooConfig.tmuConfig[tmu].tmuRev;
                caps->tfxConfig[tmu].tfxMem = 
                    _atrHwConfig.SSTs[0].sstBoard.VoodooConfig.tmuConfig[tmu].tmuRam;
            }
            caps->sli    = _atrHwConfig.SSTs[0].sstBoard.VoodooConfig.sliDetect;
            break;
        case GR_SSTTYPE_SST96:
            caps->pfxRev = 0;
            caps->pfxMem = _atrHwConfig.SSTs[0].sstBoard.SST96Config.fbRam;
            caps->numTex = 1;
            caps->tfxConfig[0].tfxRev = 
                    _atrHwConfig.SSTs[0].sstBoard.SST96Config.tmuConfig.tmuRev;
            caps->tfxConfig[0].tfxMem = 
                    _atrHwConfig.SSTs[0].sstBoard.SST96Config.tmuConfig.tmuRam;
            caps->sli    = 0;
            break;
        default:
            atuError(FXTRUE, "Unknown device type %d:", _atrHwConfig.SSTs[0].type);
            break;
        }
    #endif /* GLIDE3 */
    
        _atrGlideInitDispatchTable(&_atrGlideDriver);
        _atrGlideDriver.texEntrySize = sizeof(_AtrFXTexEntry);
    
        _atTexMaxAddress[0] = grTexMaxAddress( 0 );
        if ( caps->numTex == 2 )
            _atTexMaxAddress[1] = grTexMaxAddress( 1 );
    
    #ifdef GLIDE3
        /* Extra Glide3 inits */
        grCoordinateSpace( GR_WINDOW_COORDS );
        grDisable( GR_AA_ORDERED );
    
            grVertexLayout( GR_PARAM_XY, PARAM_XY_OFFSET , GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_Z, PARAM_Z_OFFSET , GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_Q, PARAM_W_OFFSET , GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_A, PARAM_A_OFFSET , GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_RGB, PARAM_RGB_OFFSET , GR_PARAM_ENABLE );
            //grVertexLayout( GR_PARAM_PARGB, PARAM_PARGB_OFFSET , GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_ST0, PARAM_ST0_OFFSET , GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_Q0, PARAM_W_OFFSET , GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_ST1, PARAM_ST1_OFFSET , GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_Q1, PARAM_W_OFFSET , GR_PARAM_ENABLE );
    
    
    #endif /* GLIDE3 */
    
        return &_atrGlideDriver;
    }
    
    /*-------------------------------------------------------------------
      Function: _atrIsDrawable
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
    _atrGlideIsDrawable( AtrContext ctx ) {
    
        FXUNUSED(ctx);
    
        return FXTRUE;
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideResize
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
        FXTRUE if the resize was succesful, FXFALSE otherwise
      -------------------------------------------------------------------*/
    
    static FxBool
    _atrGlideResize(AtrContext ctx, int w, int h) {
    
        FXUNUSED(ctx);
    
    #ifndef GLIDE3
        if (!_atrGlideDriver.caps.fullScreen) {
            _atrGlideDriver.caps.width  = w;
            _atrGlideDriver.caps.height = h;
            return grSstControl(GR_CONTROL_RESIZE);
        } else 
    #endif
              return FXFALSE;
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideMove
      Date: 10/11/96
      Implementor(s): mlwp
      Library: AT Render
      Description:
        Move all the buffers and re-creates device if necessary.
      Arguments:
        ctx - the rendering context
        x   - new x coordinate
        y   - new y coordinate
      Return:
        FXTRUE if the resize was succesful, FXFALSE otherwise
      -------------------------------------------------------------------*/
    
    static FxBool
    _atrGlideMove(AtrContext ctx, int x, int y) {
    
        FXUNUSED(ctx);
    
    #ifndef GLIDE3
        if (!_atrGlideDriver.caps.fullScreen) {
            _atrGlideDriver.caps.x = x;
            _atrGlideDriver.caps.y = y;
            return grSstControl(GR_CONTROL_MOVE);
        } else 
    #endif
              return FXFALSE;
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlidePause
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
    _atrGlidePause(FxBool flag) {
    #ifndef GLIDE3
        if (!gDualMonitors) 
            return grSstControl( flag ? GR_CONTROL_DEACTIVATE : 
                                        GR_CONTROL_ACTIVATE);
    #endif
        return FXTRUE;
    }
    
    int aglob;
    
    static void dummy(void) {
        aglob += 1;
    }
    
    static void
    _atrGlideShutdown( void ) {
        grGlideShutdown();
    }
    
    static void _atrGlideIdle(void) {
    #ifdef GLIDE3
        grFinish();
    #else
        grSstIdle();
    #endif
    }
    
    static void
    _atrGlideRenderBuffer( AtrBuffer buf ) {
        grRenderBuffer(buf);
    }
    
    /*
     * Snap a vertex coordinate the nearest 1/16th.
     */
    #define SNAP_COORD(v)   (v += ATR_SNAP_BIAS ) 
    
    static void
    _atrGlideDrawLine( const GrVertex *a, const GrVertex *b ) {
        GrVertex da, db;
        
            da = *a; SNAP_COORD(da.x); SNAP_COORD(da.y); 
            db = *b; SNAP_COORD(db.x); SNAP_COORD(db.y); 
            
        grDrawLine(&da, &db);
    }
    
    static void
    _atrGlideDrawPoint( const GrVertex *a ) {
        GrVertex da;
    
            da = *a; SNAP_COORD(da.x); SNAP_COORD(da.y); 
            
        grDrawPoint(&da);
    }
    
    static FxBool CheckVertex( const GrVertex *c) {
        int x ;
    
        x = (int)(c->x*16);
    
        if ( x != c->x*16 ) {
            printf("bad vertex \n");
            return FXTRUE;
        }
    
        x = (int)(c->y*16);
    
        if ( x != c->y*16 ) {
            printf("bad vertex \n");
            return FXTRUE;
        }
    
        return FXFALSE;
    }
    
    #ifdef GLIDE3
    static void
    _atrGlideDrawTriangle(const void *a, const void *b, const void *c) {
        grDrawTriangle(a, b, c);
    }
    #else
    static void
    _atrGlideDrawTriangle(const GrVertex *a, const GrVertex *b, const GrVertex *c) {
        grDrawTriangle(a, b, c);
    }
    #endif /* GLIDE3 */
    
    static void
    _atrGlideSplash( void ) {
    #ifdef GLIDE3
        /* no Splash in Glide3? */
    #else
        grSplash(0.f, 0.f, 0.f, 0.f, 0);
    #endif
    }
    
    static void
    _atrGlideGetPerfStats(GrSstPerfStats_t *pStats) {
    #ifdef GLIDE3
            grGet( GR_STATS_PIXELS_IN, 4, &(pStats->pixelsIn) );
            grGet( GR_STATS_PIXELS_OUT, 4, &(pStats->pixelsOut) );
            grGet( GR_STATS_PIXELS_CHROMA_FAIL, 4, &(pStats->chromaFail) );
            grGet( GR_STATS_PIXELS_DEPTHFUNC_FAIL, 4, &(pStats->zFuncFail) );
            grGet( GR_STATS_PIXELS_AFUNC_FAIL, 4, &(pStats->aFuncFail) );
    #else
        grSstPerfStats(pStats);
    #endif /* GLIDE3 */
    }
    
    static void
    _atrGlideResetPerfStats(void) {
    #ifdef GLIDE3
        // not defined?????
            grReset( GR_STATS_PIXELS );
    #else
        grSstResetPerfStats();
    #endif
    }
    
    static void
    _atrGlideClipWindow( int minx, int miny, int maxx, int maxy ) {
        grClipWindow(minx, miny, maxx, maxy);
    }
    
    static void
    _atrGlideDitherMode(AtrDitherMode mode) {
        grDitherMode(mode);
    }
    
    /*-------------------------------------------------------------------
      Function: _atrGlideInitDispatchTable
      Date: 10/12/96
      Implementor(s): mlwp
      Library: AT Render
      Description:
        Initialize the dispatch table for the Glide ATB driver
      Arguments:
        ctx - the rendering context to initialize
      Return:
        Nothing
      -------------------------------------------------------------------*/
    
    static void
    _atrGlideInitDispatchTable(AtrDriver *ctx) {
        ctx->RealizeImg = _atrGlideRealizeImg ;
        ctx->UnrealizeImg = _atrGlideUnrealizeImg ;
        ctx->CloneImg = _atrGlideCloneImg ;
        ctx->ClearCanvas = _atrGlideClearCanvas ;
        ctx->SwapBuffer = _atrGlideSwapBuffer ;
        ctx->BeginScene = _atrGlideBeginScene ;
        ctx->EndScene = _atrGlideEndScene ;
        ctx->RenderImg = _atrGlideRenderImg ;
        ctx->RenderImgBuffer = _atrGlideRenderImgBuffer ;
        ctx->GrabImg = _atrGlideGrabImg ;
        ctx->TexEntryInit = _atrGlideTexEntryInit ;
        ctx->TexDeleteHandle = _atrGlideTexDeleteHandle ;
        ctx->TexPunt = _atrGlideTexPunt ;
        ctx->TexAssociate = _atrGlideTexAssociate ;
        ctx->TexSource = _atrGlideTexSource ;
        ctx->TramAllocate = _atrGlideTramAllocate ;
        ctx->UpdateEnv = _atrGlideUpdateEnv ;
        ctx->UpdateMaterial = _atrGlideUpdateMaterial ;
        ctx->IsDrawable = _atrGlideIsDrawable ;
        ctx->Pause = _atrGlidePause ;
        ctx->Resize = _atrGlideResize ;
        ctx->Move   = _atrGlideMove ;
    #ifdef notdef
        (void *)ctx->Shutdown = &grGlideShutdown ;
        (void *)ctx->Idle = &grGlideIdle ;
        (void *)ctx->RenderBuffer = &grGlideRenderBuffer ;
        (void *)ctx->DrawLine = &grGlideDrawLine ;
        (void *)ctx->DrawPoint = &grGlideDrawPoint ;
        (void *)ctx->DrawTriangle = &grGlideDrawTriangle ;
        (void *)ctx->Splash = &grGlideSplash ;
        (void *)ctx->GetPerfStats = &grGlidePerfStats ;
        (void *)ctx->ResetPerfStats = &grGlideResetPerfStats ;
        (void *)ctx->ClipWindow = &grGlideClipWindow ;
    #else
        ctx->Shutdown = _atrGlideShutdown ;
        ctx->Idle = _atrGlideIdle ;
        ctx->RenderBuffer = _atrGlideRenderBuffer ;
        ctx->DrawLine = _atrGlideDrawLine ;
        ctx->DrawPoint = _atrGlideDrawPoint ;
        ctx->DrawTriangle = _atrGlideDrawTriangle ;
        ctx->Splash = _atrGlideSplash ;
        ctx->GetPerfStats = _atrGlideGetPerfStats ;
        ctx->ResetPerfStats = _atrGlideResetPerfStats ;
        ctx->ClipWindow = _atrGlideClipWindow ;
    #endif
    
    #ifdef AT_PENTIUM_ASSEMBLY
        ctx->TransformVertices = (AtrXfFunc*)_asm_TransformVertices;
    #else
        ctx->TransformVertices = CTransformVertices;
    #endif
        ctx->SpecialTransformVertices = _atrGlideSpecialTransformVertices;
        ctx->TransformVertices2D = _atrGlideTransformVertices2D;
        ctx->RenderTri = _atrGlideRenderTri;
        ctx->RenderTriSet = _atrGlideRenderTriSet;
        ctx->SpecialRenderTriSet = _atrGlideSpecialRenderTriSet;
        ctx->RenderTriWF = _atrGlideRenderTriWF;
        ctx->RenderTriSetWF = _atrGlideRenderTriSetWF;
        ctx->TransformOpenTriSet = _atrGlideTransformOpenTriSet;
        ctx->SpecialTransformOpenTriSet = _atrGlideSpecialTransformOpenTriSet;
        ctx->RenderOpenTriSet = _atrGlideRenderOpenTriSet;
        ctx->RenderOpenTriSetWF = _atrGlideRenderOpenTriSetWF;
        ctx->RenderSegment = _atrGlideRenderSegment;
        ctx->Render2DTri = _atrGlideRender2DTri;
        ctx->ClipAndRenderTri = _atrGlideClipAndRenderTri;
        ctx->ClipAndRenderSegment = _atrGlideClipAndRenderSegment;
        ctx->DitherMode = _atrGlideDitherMode;
    #ifdef GLIDE3
        {
            const char *extension = grGetString(GR_EXTENSION);
            const char *extstr = strstr(extension, "CHROMARANGE");
            if (!strncmp(extstr, "CHROMARANGE", 11)) {
                grChromaRangeMode = grGetProcAddress( "grChromaRangeModeExt" );
                grChromaRange = grGetProcAddress("grChromaRangeExt");
            }
        }
    #endif
    }
    
