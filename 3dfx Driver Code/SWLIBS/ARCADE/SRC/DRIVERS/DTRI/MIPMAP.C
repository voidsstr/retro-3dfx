#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <3dfx.h>
#include <ddraw.h>
#include <d3d.h>
#include "atd3d.h"
#include "d3dappi.h"
#include "mipmap.h"
#include "fximg.h"
#include "texusint.h"

extern D3DAppInfo *d3dapp;
extern _AtrD3DDriver *d3dContext; 

/* Load and create a mipmapped texture surface
   TBD: map texture formats 
        use mip maps created by texus
        load 3df files directly
        paletized textures
 */

/*This calculates the maximum number of mipmaps a texture can have
  given its dimensions*/
int CalculateMipMapLevels(int width, int height) {
  int LOD=0;
  
  while(width>0 && height>0) {
      width/=2; height/=2;
      LOD++;
  }

  return(LOD);
}

/* heuristic to convert from a given texture format to
   one the hardware/D3D understands. 

   TBD: this is very crude right now. We appear to be
        missing a lot of texture formats our hardware
        understands.
*/

int 
DetermineFormat(_AtrD3DDriver *ctx, AtrImg *img, 
                LPDDSURFACEDESC *lplpFormat, int *srcBpp) {
    int fmt;
    int tidx;

    switch (img->format) {
    case GR_TEXFMT_A_8:
        fmt = -1;
        break;
    case GR_TEXFMT_I_8:
        fmt = GR_TEXFMT_RGB_565;
        break;
    case GR_TEXFMT_AI_44:
        fmt = GR_TEXFMT_ARGB_4444;
        break;
    case GR_TEXFMT_AI_88:
        fmt = GR_TEXFMT_ARGB_4444;
        break;
    case GR_TEXFMT_RGB_332:
        fmt = GR_TEXFMT_RGB_565;
        break;
    case GR_TEXFMT_YIQ_422:
        fmt = GR_TEXFMT_RGB_565;
        break;
    case GR_TEXFMT_P_8:
        fmt = GR_TEXFMT_P_8;
        break;
    case GR_TEXFMT_ARGB_8332:
        fmt = GR_TEXFMT_ARGB_4444;
        break;
    case GR_TEXFMT_AYIQ_8422:
        fmt = GR_TEXFMT_ARGB_4444;
        break;
    case GR_TEXFMT_RGB_565:
        fmt = GR_TEXFMT_RGB_565;
        break;
    case GR_TEXFMT_ARGB_1555:
        fmt = GR_TEXFMT_ARGB_4444;
        break;
    case GR_TEXFMT_ARGB_4444:
        fmt = GR_TEXFMT_ARGB_4444;
        break;
    case GR_TEXFMT_AP_88:
        fmt = GR_TEXFMT_P_8;
        break;
    case GR_TEXFMT_RGB_888:
        fmt = GR_TEXFMT_RGB_565;
        break;
    case GR_TEXFMT_ARGB_8888:
        fmt = GR_TEXFMT_ARGB_4444;
        break;
    default:
        fmt = -1;
    }

    /* TBD: we are assuming that 565 is always supported */

    if (( fmt == GR_TEXFMT_P_8 ) && ( ctx->texFormatI8 == -1 )) {
        fmt = GR_TEXFMT_RGB_565;
    }

    if (( fmt == GR_TEXFMT_ARGB_4444 ) && ( ctx->texFormat4444 == -1 )) {
        fmt = GR_TEXFMT_RGB_565;
    }

    if ( fmt == -1 ) {
        atuError(FXTRUE, "Unknown texture format %d\n", img->format);
    }

    switch ( fmt ) {
    case GR_TEXFMT_P_8:
        tidx = ctx->texFormatI8;
        *srcBpp = 1;
        break;
    case GR_TEXFMT_RGB_565:
        tidx = ctx->texFormat565;
        *srcBpp = 2;
        break;
    case GR_TEXFMT_ARGB_4444:
        tidx = ctx->texFormat4444;
        *srcBpp = 2;
        break;
    }

    *lplpFormat = &d3dapp->TextureFormat[tidx].ddsd;
    return fmt;
}

/*-------------------------------------------------------------------
  Function: _atrD3DPutImg
  Date: 10/16
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Copy an image to a D3D surface
  Arguments:
      lpDDS    - pointer to surface
      i        - pointer to image structure
  Return:
      FXTRUE on success, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool _atrD3DPutImg( LPDIRECTDRAWSURFACE lpDDS, const AtrImg *img) {
    unsigned char *dst, *src, *srcLine, *dstLine;
    size_t srcStride, dstStride;
    int srcBpp, dstBpp;
    DDSURFACEDESC ddsd;
    HRESULT ddrval;
    unsigned int r_up_shift, g_up_shift, b_up_shift, a_up_shift;
    unsigned int r_truncate_shift, g_truncate_shift, b_truncate_shift, 
                a_truncate_shift;
    unsigned int packed_pixel;
    unsigned int red, green, blue, alpha;
    FxU16 tmp16;
    FxU32 tmp32;
    FxU32 row, col;

    /* check for valid source format and determine pixel size */

    switch ( img->format ) {
    case ATR_IMGFMT_P_8:
        srcBpp = 1;
        break;
    case ATR_IMGFMT_RGB_565:
    case ATR_IMGFMT_ARGB_4444:
	case ATR_IMGFMT_ARGB_1555:
        srcBpp = 2;
        break;
    case ATR_IMGFMT_ARGB_8888:
        srcBpp = 4;
        break;
    default:
        atuError(FXFALSE, "D3DPutImg: Unsupported source format %d\n", img->format);
        return FXTRUE;
    }

    /* ATB images are tightly packed so */

    srcStride = srcBpp*img->width;
    srcLine = img->data;

    /* Build a descriptor to lock the surface */

    memset(&ddsd, 0, sizeof(DDSURFACEDESC));
    ddsd.dwSize = sizeof(DDSURFACEDESC);
    ddrval = lpDDS->lpVtbl->Lock(lpDDS, NULL, &ddsd, 0, NULL);
    if (ddrval != DD_OK) {
        atuError(FXTRUE, "_atrD3DPutImg: could not lock surface\n");
        return FXFALSE;
    }

    dstBpp = ddsd.ddpfPixelFormat.dwRGBBitCount/8;
    dstStride = ddsd.lPitch;

    /* first deal with 8 bit paletted textures */

    dstLine = (unsigned char *)ddsd.lpSurface;

    if ( img->format  == ATR_IMGFMT_P_8 ) {
        memcpy( dstLine, srcLine, img->width*img->height);
    
        lpDDS->lpVtbl->Unlock(lpDDS, NULL);

        return FXTRUE;
    }

    /* Get packing shift information */

    DeterminePackingShift( ddsd.ddpfPixelFormat.dwRBitMask,
                           &r_truncate_shift, &r_up_shift);
    DeterminePackingShift( ddsd.ddpfPixelFormat.dwGBitMask,
                           &g_truncate_shift, &g_up_shift);
    DeterminePackingShift( ddsd.ddpfPixelFormat.dwBBitMask,
                           &b_truncate_shift, &b_up_shift);
    DeterminePackingShift( ddsd.ddpfPixelFormat.dwRGBAlphaBitMask,
                           &a_truncate_shift, &a_up_shift);
  
    /* Slow code is good, or something */

    for ( row = 0; row < img->height; row++ ) {
        dst = dstLine;
        dstLine += dstStride;
        src = srcLine;
        srcLine += srcStride;
        for ( col = 0; col < img->width; col++ ) {
       
            /* extract the pixel */
    
            switch ( img->format ) { 
            case ATR_IMGFMT_RGB_565:
    		    tmp16 = *(FxU16 *)src;
                blue = (tmp16&0x1f)<<3; tmp16 >>= 5; 
                green = (tmp16&0x3f)<<2; tmp16 >>= 6; 
                red = (tmp16&0x1f)<<3; tmp16 >>= 6; 
                alpha = 0xff;
                break;
            case ATR_IMGFMT_ARGB_4444:
    		    tmp16 = *(FxU16 *)src;
                blue = (tmp16&0xf)<<4; tmp16 >>= 4; 
                green = (tmp16&0xf)<<4; tmp16 >>= 4; 
                red = (tmp16&0xf)<<4; tmp16 >>= 4; 
                alpha = (tmp16&0xf)<<4; tmp16 >>= 4; 
                break;
			case ATR_IMGFMT_ARGB_1555:
				tmp16 = *(FxU16 *)src;
                blue = (tmp16&0x1f)<<3; tmp16 >>= 5; 
                green = (tmp16&0x1f)<<3; tmp16 >>= 5; 
                red = (tmp16&0x1f)<<3; tmp16 >>= 5; 
                alpha = (tmp16&0xf)?0xff:0x0; 
                break;
            case ATR_IMGFMT_ARGB_8888:
    		    tmp32 = *(FxU32 *)src;
                blue = (tmp32&0xff); tmp32 >>= 8; 
                green = (tmp32&0xff); tmp32 >>= 8; 
                red = (tmp32&0xff); tmp32 >>= 8; 
                alpha = (tmp32&0xff); tmp32 >>= 8; 
                break;
            default:
                atuError(FXTRUE, "PackAndCopy: unknown source texel format %d\n", img->format);
            }
    
            /* Pack the pixel */
    
            packed_pixel= ((blue >> b_truncate_shift) << b_up_shift) ;
    	    packed_pixel|= ((green >> g_truncate_shift) << g_up_shift) ;
    	    packed_pixel|= ((red >> r_truncate_shift) << r_up_shift) ;
    
    		/* TBD: compiler error!!, shifting by 32 is a NOOP */
    
    		if ( a_truncate_shift < 32 )
    			packed_pixel|= ((alpha >> a_truncate_shift) << a_up_shift);
          
            switch(dstBpp) {
    	    case 4:
    	        *((unsigned int *)dst)=packed_pixel;
    	        break;
    	    case 2:
    	        *((unsigned short *)dst)=
    	        (unsigned short)packed_pixel;
    	        break;
    	    case 1:
    	        *dst = (unsigned char)packed_pixel;
    	        break;
    	    default:
    	        dprintf("Damn't! We don't support %d byte textures: %s;%d\n",
    		            dstBpp, __FILE__, __LINE__);
    	        return FXFALSE;	  
            }
            dst += dstBpp;
            src += srcBpp;
        }
    }

    /* release the lock */

    lpDDS->lpVtbl->Unlock(lpDDS, NULL);

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: _atrD3DRealizeImg
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Convert the image data into a form it can be processed by D3D
  Arguments:
      i        - pointer to image structure
  Return:
      FXTRUE on success, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool _atrD3DRealizeImg( AtrImg *img ) {
    LPDDSURFACEDESC lpFormat;
    LPDIRECTDRAWSURFACE lpDDS;
    LPDIRECTDRAWSURFACE lpSurface = NULL;
    LPDIRECT3DTEXTURE   lpTexture = NULL;
    LPDIRECTDRAWPALETTE lpPalette = NULL;
    DDSURFACEDESC ddsd;
    HRESULT ddrval;
    DDSCAPS ddscaps;
    int LOD, MAX_LOD;
    Gu3dfInfo info;
	FxU32 target_width, target_height, target_format;
    size_t tex_mem_required;
    int      i;
    AtrD3DImg *d3dImage;
    int srcBpp;
    AtrImg tmpImg;
    FxBool allocatedTmp = FXFALSE;

    /* check if we have already been realized if so just return */

    if ( img->devPrivate != NULL )
        return FXTRUE;

    /* determine what the final width, height and format the
       image should be that can be supported by D3D
     */

	target_width = img->width;
    target_height = img->height;
	target_format = DetermineFormat(d3dContext, img, &lpFormat, &srcBpp);

	MAX_LOD=CalculateMipMapLevels(target_width, target_height);

    tex_mem_required = txInit3dfInfo( &info, target_format,
                                      &target_width, &target_height,
                                      MAX_LOD, TX_AUTORESIZE_GROW );
    /*
     * Make sure txInit3dfInfo didn't fail.
     */
    
    if ( tex_mem_required == 0 ) {
        atuError( FXTRUE, "Problem with txInit3dfInfo\n" );
    }

    if ( ( target_width == img->width ) && 
         ( target_height == img->height ) &&
         ( target_format == img->format )) {
        tmpImg = *img;
        /* honor the input images # of LOD's */
        MAX_LOD = ATM_MIN(MAX_LOD, img->nLevels);
    } else {
    
        /*
         * Allocate system memory for the texture.
         */
    
        if ( ( info.data = malloc( tex_mem_required )) == NULL ) {
            atuError( FXTRUE, 
                     "Out of memory allocating system memory textures.\n" );
        }
    
        allocatedTmp = FXTRUE;

    	/*
         * Convert to a texture that can be downloaded to D3D.
         */
    
    	txConvert(	&info, img->format,
    				img->width, img->height, img->data, TX_DITHER_ERR,
    				img->table );

        tmpImg.data    = info.data;
        tmpImg.format  = info.header.format;
        tmpImg.width   = info.header.width;
        tmpImg.height  = info.header.height;
        tmpImg.nLevels = MAX_LOD;
    }
    
    /* Setup the descriptor to create the surface */

    memcpy(&ddsd, lpFormat, sizeof(DDSURFACEDESC));
    ddsd.dwSize = sizeof(DDSURFACEDESC);
    if(MAX_LOD!=1) {
        ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT
	                  | DDSD_MIPMAPCOUNT;
        ddsd.dwMipMapCount=MAX_LOD;
        ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_COMPLEX | 
	                          DDSCAPS_MIPMAP | DDSCAPS_SYSTEMMEMORY;
    } else {
        ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
        ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;      
    }
    ddsd.dwHeight = target_height;
    ddsd.dwWidth = target_width;

    ddrval=IDirectDraw_CreateSurface(d3dapp->lpDD, &ddsd, &lpDDS, NULL);
    CHECK(ddrval!=DD_OK, "Could not create mipmap surface");
    lpSurface=lpDDS;

    /* fill the MipMaps by looping through each Level of Detail */    

    ddscaps.dwCaps=DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
        
    for(LOD=0;LOD<MAX_LOD;LOD++) {
        if (!_atrD3DPutImg( lpDDS, &tmpImg)) {
            atuError(FXTRUE, "D3DRealizeImage: could not copy image\n");
        }

        tmpImg.data = ((char *)tmpImg.data)+tmpImg.width*tmpImg.height*srcBpp;
        tmpImg.width >>=1 ;
        tmpImg.height >>= 1;

        /* Get the next mipmap level */
      
        ddrval=lpDDS->lpVtbl->GetAttachedSurface(lpDDS, &ddscaps, &lpDDS);
	
        if (ddrval!=DD_OK && lpDDS!=NULL) {
	        lpDDS->lpVtbl->Release(lpDDS);
	        return  FXFALSE;
	    }
    }

    if ( allocatedTmp )
	    free(info.data);

    /* process palette data if any, D3D does not support NCC textures (yet!) */

    if (( img->format == ATR_IMGFMT_P_8 ) || ( img->format == ATR_IMGFMT_AP_88 )) {

        memset(ppe, 0, sizeof(PALETTEENTRY) * 256);
        for (i = 0; i < 256; i++) {
            ppe[i].peRed = (unsigned char)RGB_GETRED(img->table->palette[i]);
            ppe[i].peGreen = (unsigned char)RGB_GETGREEN(img->table->palette[i]);
            ppe[i].peBlue = (unsigned char)RGB_GETBLUE(img->table->palette[i]);


			ppe[i].peRed = 0xff;
            ppe[i].peGreen = 0;
            ppe[i].peBlue = 0;
        }
    
        /*
         * Create the palette with the DDPCAPS_ALLOW256 flag because we want to
         * have access to all entries.
         */
    
        ddrval = d3dapp->lpDD->lpVtbl->CreatePalette(d3dapp->lpDD,
                                             DDPCAPS_INITIALIZE | 
                                             DDPCAPS_8BIT | 
                                             DDPCAPS_ALLOW256,
                                             ppe, &lpPalette, NULL);
        if (ddrval != DD_OK) {
            atuError(FXFALSE, "Create palette failed while loading surface (loadtex).\n%s",
                               D3DAppErrorToString(ddrval));
            goto exit_with_error;
        }
    
        /*
         * Finally, bind the palette to the surface
         */
    
        ddrval = lpSurface->lpVtbl->SetPalette(lpSurface, lpPalette);
        if (ddrval != DD_OK) {
            atuError( FXFALSE, "SetPalette failed while loading surface (loadtex) .\n%s",
                                  D3DAppErrorToString(ddrval));
            goto exit_with_error;
        }
    }

    ddrval=IDirectDrawSurface_QueryInterface(lpSurface,
                             &IID_IDirect3DTexture, (void*)&lpTexture);

    if (ddrval!=DD_OK) {
        atuError( FXFALSE, "Could not Query for System Memory texture");
        goto exit_with_error;
    }

    if ((d3dImage = (AtrD3DImg *)malloc(sizeof(AtrD3DImg))) == NULL ) {
        atuError(FXFALSE, 
                 "_atrD3DRealizeImage: could not allocate space for image\n");
        goto exit_with_error;
    }

    img->devPrivate = d3dImage;
    d3dImage->lpSurface = lpSurface;
    d3dImage->lpTexture = lpTexture;
    d3dImage->lpPalette = lpPalette;

    return FXTRUE;

exit_with_error:
    RELEASE(lpSurface);
    RELEASE(lpPalette);
    RELEASE(lpTexture);
    return FXFALSE;
}

/*
  This packs and copies data from a source image to a destination image.
  dstBpp means bytes per pixel, not bpp
  size is the size of the data source in bytes
  The masks specify where the bits for each channel should go
  */

void PackAndCopy(unsigned char *dest, unsigned char *src, int size, int dstBpp,
		 int src_fmt, int r_mask, int g_mask, int b_mask, int a_mask) {
    unsigned int dest_index;
    unsigned int r_up_shift, g_up_shift, b_up_shift, a_up_shift;
    unsigned int r_truncate_shift, g_truncate_shift, b_truncate_shift, 
                a_truncate_shift;
    unsigned int packed_pixel;
    unsigned int red, green, blue, alpha;
    FxU8 *ptr, *end;
    FxU16 tmp16;
    FxU32 tmp32;

    /* first deal with 8 bit paletted textures */

    if ( src_fmt == GR_TEXFMT_P_8 ) {
        memcpy( dest,  src, size);
        return;
    }

    /* Get packing shift information */

    DeterminePackingShift(r_mask, &r_truncate_shift, &r_up_shift);
    DeterminePackingShift(g_mask, &g_truncate_shift, &g_up_shift);
    DeterminePackingShift(b_mask, &b_truncate_shift, &b_up_shift);
    DeterminePackingShift(a_mask, &a_truncate_shift, &a_up_shift);
  
    /* Slow code is good, or something */

    for ( ptr = src, end = src+size, dest_index=0; 
        ptr < end; 
        dest_index+=dstBpp) {
       
        /* extract the pixel */

        switch ( src_fmt ) { 
        case ATR_IMGFMT_RGB_565:
		    tmp16 = *(FxU16 *)ptr;
            ptr += 2;
            blue = (tmp16&0x1f)<<3; tmp16 >>= 5; 
            green = (tmp16&0x3f)<<2; tmp16 >>= 6; 
            red = (tmp16&0x1f)<<3; tmp16 >>= 6; 
            alpha = 0xff;
            break;
        case ATR_IMGFMT_ARGB_4444:
		    tmp16 = *(FxU16 *)ptr;
            ptr += 2;
            blue = (tmp16&0xf)<<4; tmp16 >>= 4; 
            green = (tmp16&0xf)<<4; tmp16 >>= 4; 
            red = (tmp16&0xf)<<4; tmp16 >>= 4; 
            alpha = 0xff;
            break;
        case ATR_IMGFMT_ARGB_8888:
		    tmp32 = *(FxU32 *)ptr;
            ptr += 4;
            blue = (tmp32&0xff); tmp32 >>= 8; 
            green = (tmp32&0xff); tmp32 >>= 8; 
            red = (tmp32&0xff); tmp32 >>= 8; 
            alpha = (tmp32&0xff); tmp32 >>= 8; 
            break;
        default:
            atuError(FXTRUE, "PackAndCopy: unknown source texel format %d\n", src_fmt);
        }

        /* Pack the pixel */

        packed_pixel= ((blue >> b_truncate_shift) << b_up_shift) ;
	    packed_pixel|= ((green >> g_truncate_shift) << g_up_shift) ;
	    packed_pixel|= ((red >> r_truncate_shift) << r_up_shift) ;

		/* TBD: compiler error!!, shifting by 32 is a NOOP */

		if ( a_truncate_shift < 32 )
			packed_pixel|= ((alpha >> a_truncate_shift) << a_up_shift);
      
        switch(dstBpp) {
	    case 4:
	        *((unsigned int *)(&dest[dest_index]))=packed_pixel;
	        break;
	    case 2:
	        *((unsigned short *)(&dest[dest_index]))=
	        (unsigned short)packed_pixel;
	        break;
	    case 1:
	        dest[dest_index]=(unsigned char)packed_pixel;
	        break;
	    default:
	        dprintf("Damn't! We don't support %d byte textures: %s;%d\n",
		            dstBpp, __FILE__, __LINE__);
	        return;	  
        }
    }
}

/*
  This determines how the bits from the unpacked data must be shifted
  around to fit into the packed format
  */

void DeterminePackingShift(unsigned int mask, unsigned int *truncate_shift,
			   unsigned int *up_shift) {
    unsigned int size;
  
    /* Determine how much the channel needs to be shifted up */

    if (mask==0) {
        *up_shift=0;
        *truncate_shift=32;
    } else {
        *up_shift=0;
        while((mask&1)==0) {
	        (*up_shift)++;
	        mask>>=1;
	    }
      
        /* Determine how many bits the channel is */

        size=0;
        while(mask&1) {
	        size++;
	        mask>>=1;
	    }
  
        /* Calculate how many bits need to be chopped off */

        *truncate_shift=8-size;
    }
}
  
/* Given a D3D channel bit mask, determine the size of the channel in bits */

int DetermineDepth(int a) {
    int count=0;
    while(!(a&1) && a)
        a=a>>1;
  
    while(a&1) {
        a=a>>1;
        count++;
    }

    return(count);
}
