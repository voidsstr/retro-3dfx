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

/* ATB: skeleton driver
*/

#include <math.h>
#include <stdio.h>
#include <3dfx.h>
#include <glide.h>
#include <atrender.h>
#include <texus.h>

#include "fxatr.h"
#include "rave.h"
#include "rave_system.h"
#include "rave_tc.h"
#include "atb_rave.h"


#define						kVendorID_Mike               42
#define						kEngineID_Mike					2001

static void _atrRAVInitDispatchTable(AtrRaveDriver *ctx);

#define USE_MIKES_ENGINE   0		

#if USE_MIKES_ENGINE
#define TURN_ON_D3D		1
#include "d3d.h"
#include "d3dmain.h"
#include "d3dapp.h"
#else
#define TURN_ON_D3D		0
#endif

#if TURN_ON_D3D		
BOOL _atrD3DBeforeDeviceDestroyed(LPVOID lpContext);
BOOL _atrD3DAfterDeviceCreated(int w, int h, LPDIRECT3DVIEWPORT* lpViewport,
                               LPVOID lpContext);
D3DAppInfo *D3dappi;	
#endif

AtrDriverInfo *gDriverInfo; 
int gNumTMUs=1;   			//really lame, set in UpdateMaterial() if the material has a second texture

short Num_triangles=0;		//debugging junk
int gPassNumber=1;

BOOL validate_triangle(TQAVTexture_TC v[],TQARect rect);
extern AtrMaterial *_atrRenderMaterial;		//current material

TQADevice gDevice;
float Value;							//used for debugging

/*** This is simply the packet of info that is handed off from
 *** routine to routine, and may contain any stuff needed to perform
 *** the duties. ***/

static AtrRaveDriver _atrRAVDriver;

/*** some globals required by IsDrawable to see if the device may be drawn to. ***/

short gPause=0;
short gInitialized=0;
short gMinimized=0;



/*-------------------------------------------------------------------
  Function: _atrRAVBeginScene
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
_atrRAVBeginScene(FxU32 *viewWidth, FxU32 *viewHeight) 
{
	TQARect rect;

	QARenderStart(_atrRAVDriver.ravecontext,NULL,NULL);

	rect=_atrRAVDriver.rect;

	*viewWidth=rect.right-rect.left;
	*viewHeight=rect.bottom-rect.top;

Num_triangles=0;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: _atrRAVEndScene
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
_atrRAVEndScene(void) 
{
	QARenderEnd(_atrRAVDriver.ravecontext,NULL);

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: _atrRAVSplash
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
_atrRAVSplash( void ) {
}

/*-------------------------------------------------------------------
  Function: _atrRAVRenderImg
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Render an image at the specified location
  Arguments:
    i       - the image to display
    screenX - location at which to display image
    screenY 
  Return:
    FXTRUE if the context can currently be drawn to, FXFALSE otherwise
  -------------------------------------------------------------------*/

BOOL 
_atrRAVRenderImg(AtrImg *img, FxU32 screenX, FxU32 screenY ) 
{
	TQAVGouraud bv;
	TQABitmap *bitmap;
	_AtrXImage *ximage;
	TQAError error;
	float delta=0.0f;
	float right;
	long screenW,screenH;

//return;		//this ain't working yet.
	
	/*** if the image hasn't been realized (created) yet, create it, and flip it.
	 *** For rendering images here, the data is already the proper orientation,
	 *** so it shouldn't be flipped in the first place, but since it is we need
	 *** to unflip it. ***/

	if(!img->devPrivate)
	{
		if(!_atrRAVRealizeImg(img))return FXFALSE;

		ximage=(_AtrXImage *)img->devPrivate;
		flip_image(ximage->rave_image);
	}
	else
		ximage=(_AtrXImage *)img->devPrivate;

	/*** this is really lame since it creates and destroys the image each render,
	 *** which happens 20, 30 fps. I need to cache the bitmap handle if possible,
	 *** probably in the devPrivate area. ***/

	screenW=_atrRAVDriver.rect.right-_atrRAVDriver.rect.left;
	screenH=_atrRAVDriver.rect.bottom-_atrRAVDriver.rect.top;

   bv.x=(float)screenX;
   bv.y=(float)screenH-screenY-ximage->rave_image->height;

   bv.z=0.0f;
   bv.a=1.0f;
   bv.r=1.0f;
   bv.g=1.0f;
   bv.b=1.0f;



	right=(float)(screenX+ximage->rave_image->width);

	if(right>_atrRAVDriver.rect.right)
	{
		delta=right-_atrRAVDriver.rect.right;
	
//		bv.x-=delta;
	}

//bv.x=10;
//bv.y=10;

	if(!ximage->bitmap)
		error=QABitmapNew(_atrRAVDriver.engine,0L,ximage->rave_format,ximage->rave_image,&ximage->bitmap);
	else
		error=kQANoErr;

	if(error==kQANoErr)
	{

		QADrawBitmap(_atrRAVDriver.ravecontext,&bv,ximage->bitmap);
		QABitmapDelete(_atrRAVDriver.engine,bitmap);
//		QARenderEnd(_atrRAVDriver.ravecontext,NULL);
	}
}

/*-------------------------------------------------------------------
  Function: _atrRAVGrabImg
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Grab a portion of the display surface
  Arguments:
    dst     - where to put the captured image
    screenX - location at which to start capture
    screenY 
    buf     - capture front or back buffer?
  Return:
    FXTRUE if the context can currently be drawn to, FXFALSE otherwise
  -------------------------------------------------------------------*/

static void 
_atrRAVGrabImg( AtrImg    *dst,
                 FxU32     screenX,
                 FxU32     screenY,
                 AtrBuffer buf) {
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
_atrRAVDrawPoint( const GrVertex *p ) 
{
	TQAVGouraud v;
	long h;

	h=_atrRAVDriver.rect.bottom-_atrRAVDriver.rect.top;
	_atrRAVSetRaveGVertex(&v,p,h);

	QADrawPoint(_atrRAVDriver.ravecontext,&v);	
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
_atrRAVDrawLine( const GrVertex *a, const GrVertex *b ) 
{
	TQAVGouraud v[2];
	long h;

	h=_atrRAVDriver.rect.bottom-_atrRAVDriver.rect.top;

	_atrRAVSetRaveGVertex(&v[0],a,h);
	_atrRAVSetRaveGVertex(&v[1],b,h);

	QADrawLine(_atrRAVDriver.ravecontext,&v[0],&v[1]);	

}

/*-------------------------------------------------------------------
  Function: _atrRAVDrawTriangle
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

static void
_atrRAVDrawTriangle(const GrVertex *a, const GrVertex *b,const GrVertex *c) 
{
	long h;
	unsigned long flags;
	TQAVTexture_TC v[3];

//	if(_atrRAVIsBackface(a,b,c))return;

	h=_atrRAVDriver.rect.bottom-_atrRAVDriver.rect.top;

	_atrRAVSetRaveTVertex(&v[0],a,h);
	_atrRAVSetRaveTVertex(&v[1],b,h);
	_atrRAVSetRaveTVertex(&v[2],c,h);

	if(!validate_triangle(v,_atrRAVDriver.rect))
		return;

//QASetInt(_atrRAVDriver.ravecontext,kQATag_TextureOp,kQATextureOp_Modulate|kQATextureOp_Highlight);

	flags=kQATriFlags_None;

	if(gNumTMUs==2)		//alert the RAVE triangle stuff that the second texture
	{							//stuff is valid
		flags|=kQATriFlags_TC;
	}

	QADrawTriTexture(_atrRAVDriver.ravecontext,
		(TQAVTexture *)&v[0],
		(TQAVTexture *)&v[1],
		(TQAVTexture *)&v[2],kQATriFlags_None);

	Num_triangles++;
}

/*-------------------------------------------------------------------
  Function: _atrRAVDrawGTriangle
  Date: 10/9/96
  Implementor(s): rms
  Library: AT Render
  Description:
    Draws a non-textured, gouraud shaded triangle. I need this for RAVE since
	 RAVE uses different vertex structs and calls when dealing with shaded tris.
  Arguments:
    a  - vertices defining triangle
    b  
    c  
  Return:
    None
  -------------------------------------------------------------------*/

static void
_atrRAVDrawGTriangle(const GrVertex *a, const GrVertex *b,const GrVertex *c) 
{
	long h;
	TQAVGouraud v[3];

	h=_atrRAVDriver.rect.bottom-_atrRAVDriver.rect.top;

	_atrRAVSetRaveGVertex(&v[0],a,h);
	_atrRAVSetRaveGVertex(&v[1],b,h);
	_atrRAVSetRaveGVertex(&v[2],c,h);


	QASetInt(_atrRAVDriver.ravecontext,kQATag_TextureOp,kQATextureOp_Modulate);
	QADrawTriGouraud(_atrRAVDriver.ravecontext,&v[0],&v[1],&v[2],kQATriFlags_None);
	
	Num_triangles++;
}



/*-------------------------------------------------------------------
  Function: _atrRAVClearCanvas
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
    Noe
  ------------------------------------------------------------------*/

static void
_atrRAVClearCanvas( float r, float g, float b, FxU16 za ) 
{
	/*** for the time being, merely let rave handle this, I will
	 *** need to draw 2 g shaded triangles in order to set the Z ***/

	QASetFloat(_atrRAVDriver.ravecontext,kQATag_ColorBG_a,(float)1.0);
	QASetFloat(_atrRAVDriver.ravecontext,kQATag_ColorBG_r,(float)r);
	QASetFloat(_atrRAVDriver.ravecontext,kQATag_ColorBG_b,(float)b);
	QASetFloat(_atrRAVDriver.ravecontext,kQATag_ColorBG_g,(float)g);
}

/*-------------------------------------------------------------------
  Function: _atrRAVSwapBuffer
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
_atrRAVSwapBuffer( FxU32 sync ) 
{

	//for RAVE this is a NOP, since EndScene automatically swaps the buffers
}

/*-------------------------------------------------------------------
  Function: _atrRAVGetPerfStats
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
_atrRAVGetPerfStats(GrSstPerfStats_t *pStats) {
}

/*-------------------------------------------------------------------
  Function: _atrRAVResetPerfStats
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
_atrRAVResetPerfStats(void) {
}

/*-------------------------------------------------------------------
  Function: _atrRAVClipWindow
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
_atrRAVClipWindow( int minx, int miny, int maxx, int maxy ) {
}

/*-------------------------------------------------------------------
  Function: _atrRAVTexEntryInit
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
_atrRAVTexEntryInit( _AtrTexEntry *entry ) 
{
	/*** put any internal init stuff here for my custom
	 *** texture handle ***/
	

}

/* heuristic to convert from a given texture format to
   one the hardware/D3D understands. 

   TBD: this is very crude right now. We appear to be
        missing a lot of texture formats our hardware
        understands.
*/

int 
_atrRAVDetermineFormat(AtrImg *img) 
{
    int fmt;

	/*** a number of the formats are not supported by RAVE. In later versions
	 *** I may be able to do some pixel massaging to make the usable. In the meantime,
	 *** this does its best to map some of the wacky ATB formats into something
	 *** that RAVE can understand, and use for the time being. ***/

    switch (img->format) {
    case GR_TEXFMT_A_8:
        fmt = -1;
        break;
    case GR_TEXFMT_I_8:
        fmt =  -1;
        break;
    case GR_TEXFMT_AI_44:
        fmt =  -1;
        break;
    case GR_TEXFMT_AI_88:
        fmt =  -1;
        break;
    case GR_TEXFMT_RGB_332:
        fmt = GR_TEXFMT_RGB_565;
        break;
    case GR_TEXFMT_YIQ_422:
        fmt = GR_TEXFMT_YIQ_422;
        break;
    case GR_TEXFMT_P_8:
        fmt = -1;
        break;
    case GR_TEXFMT_ARGB_8332:
        fmt = GR_TEXFMT_ARGB_8332;
        break;
    case GR_TEXFMT_AYIQ_8422:
        fmt = GR_TEXFMT_AYIQ_8422;	
        break;
    case GR_TEXFMT_RGB_565:
        fmt = GR_TEXFMT_RGB_565;
        break;
    case GR_TEXFMT_ARGB_1555:
        fmt = GR_TEXFMT_ARGB_1555;
        break;
    case GR_TEXFMT_ARGB_4444:
        fmt = GR_TEXFMT_ARGB_4444;
        break;
    case GR_TEXFMT_AP_88:
        fmt = -2;
        break;
    case GR_TEXFMT_RGB_888:
        fmt = GR_TEXFMT_RGB_565;
        break;
    case GR_TEXFMT_ARGB_8888:
        fmt = GR_TEXFMT_ARGB_8888;
        break;
    default:
        fmt = -1;
    }

    return fmt;
}

/*-------------------------------------------------------------------
  Function: _atrRAVExtractPixel
  Date: 10/30
  Implementor(s): RMS/apple
  Library: AT Render
  Description:
  converts a pixel from the ATB source texture data to 8 bit rgb, and
  returns a pixel in a format recognizable by RAVE
  Arguments:
      e - texture handle to initialize
  Return:
      Nothing
  -------------------------------------------------------------------*/

static long 
_atrRAVExtractPixel(short format,char *src,
	unsigned int *red,unsigned int *green,unsigned int *blue,unsigned int *alpha)
{
	FxU16 tmp16;
	FxU32 tmp32;
	long pixel;

	switch (format ) { 
		case ATR_IMGFMT_RGB_565:
		    tmp16 =  *(FxU16 *)src;
            *blue = (tmp16&0x1f)<<3; tmp16 >>= 5; 
            *green = (tmp16&0x3f)<<2; tmp16 >>= 6; 
            *red = (tmp16&0x1f)<<3; tmp16 >>= 5; 
            *alpha = 0xff;

			/*** the colors are expanded to 8 bits to "normalize" them, so I 
			 *** need to shrink them back down to 5 bits each for the pixel value ***/


				pixel=MAKE_RGB16(((*red)>>3),((*green)>>3),((*blue)>>3));

				pixel|=0x00008000;	//ensure the alpha channel is on 
            break;

	    case ATR_IMGFMT_ARGB_8888:
    			tmp32 = *(FxU32 *)src;
            *blue = (tmp32&0xff); tmp32 >>= 8; 
            *green = (tmp32&0xff); tmp32 >>= 8; 
            *red = (tmp32&0xff); tmp32 >>= 8; 
            *alpha = (tmp32&0xff); tmp32 >>= 8;

				pixel=MAKE_RGB16(((*red)>>3),((*green)>>3),((*blue)>>3)); ;
				if(*alpha)pixel|=0x00008000;

				break;

	    case ATR_IMGFMT_ARGB_4444:

			/*** not supported by RAVE so I at least get the color data to view. 
			 *** Convert to 5 bit colors. ***/

    			tmp16 = *(FxU16 *)src;
            *blue = (tmp16&0xf)<<1; tmp16 >>= 4; 
            *green = (tmp16&0xf)<<1; tmp16 >>= 4; 
            *red = (tmp16&0xf)<<1; tmp16 >>= 4; 
            *alpha = (tmp16&0xf)<<4; tmp16 >>= 4; 
			

				pixel=MAKE_RGB16((*red),(*green),(*blue));
				
				if(*alpha)pixel|=0x00008000;
            break;
	
		case ATR_IMGFMT_ARGB_1555:
			/*** supported by RAVE, no conversion needed ***/

				pixel= *(FxU16 *)src;	

#if 0		//invert the colors (???) YIQ images seem to need this
   			tmp16 = *(FxU16 *)src;

            *blue = (tmp16&0x1f); tmp16 >>= 5; 
            *green = (tmp16&0x1f); tmp16 >>= 5; 
            *red = (tmp16&0x1f); tmp16 >>= 5; 
            *alpha = 0xff;
		
				*red=31.0- *red;
				*green=31.0- *green;
				*blue=31.0- *blue;

				pixel=MAKE_RGB16((*red),(*green),(*blue));
#endif

				pixel|=0x00008000;		//needed to make the targa help file from the
												//cube demo visible.
            break;
		
		case ATR_IMGFMT_ARGB_8332:
			   tmp16 = *(FxU16 *)src;
            *blue = (tmp16&0x2)<<3; tmp16 >>= 2; 
            *green = (tmp16&0x7)<<2; tmp16 >>= 3; 
            *red = (tmp16&0x7)<<2;tmp16 >>= 3;
				*alpha= tmp16;

				pixel=MAKE_RGB16(*red,*green,*blue);
				
				/*** if alpha has any value, set the top bit to 1 ***/

				if(*alpha)pixel|=0x00008000;

				break;


		/*** the negative values are for unsupported image formats,
		 *** so we return solid colors for their pixels. ***/

		case -1:
			pixel=0x66;	//either a palettized for small RGB value, 0x66 will probably
						//generate some color in either format
			break;

		case -2:
			pixel=MAKE_RGB16(16,0,20);
			break;

		case -4:
			pixel=MAKE_RGBA32(31,16,0,20);
			break;

		default:
            atuError(FXTRUE, "PackAndCopy: unknown source texel format %d\n",format);
    }

	return pixel;
}

/*-------------------------------------------------------------------
  Function: _atrRAVRealizeImg
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Convert the image data into a form it can be processed by hardware
  Arguments:
      i        - pointer to image structure
  Return:
      FXTRUE on success, FXFALSE otherwise
  -------------------------------------------------------------------*/

static FxBool 
_atrRAVRealizeImg( AtrImg *img ) 
{
	TQAImage *rimg;		//pointer to start of Rave mipmap data
	TQAImage *tmp_rimg;	//index to each of the Rave mipmap subimages
	AtrImg tmp_aimg;	//index to each of the ATB mipmap subimages.
	int srcBpp;			
	int dstBpp=2;
	char *src_data, *dst_data;	//current index to the pixel data
	long pixel;
	unsigned short row,col;
	unsigned int i;
	unsigned int red,green,blue,alpha;	//not used for now
	_AtrXImage *ximage;
	TQAImagePixelType rave_format;
	long image_size;
	unsigned int nlevels=0;
   Gu3dfInfo info;		//needed for texture conversion
	unsigned long tex_mem_required=0;
	
    /* check if we have already been realized if so just return */

   if ( img->devPrivate != NULL )
        return FXTRUE;


	nlevels=(unsigned int)img->nLevels;

	/*** non-mipmapped textures of nLevel set to 0, so I need to check and
	 *** compensate for that to force at least one time thru the loop ***/

	if(!nlevels)nlevels=1;

	tmp_aimg.data=img->data;
	tmp_aimg.width=img->width;
	tmp_aimg.height=img->height;
	info.data=NULL;							//needed to tell if we should free this later on

	tmp_aimg.format=_atrRAVDetermineFormat(img);

	switch (	tmp_aimg.format ) 
	{
		case ATR_IMGFMT_P_8:
			srcBpp = 1;
			rave_format=kQAPixel_CL8;
			break;

		case ATR_IMGFMT_RGB_565:
			srcBpp=2;
			rave_format=kQAPixel_RGB16;	
			break;

		case ATR_IMGFMT_ARGB_1555:
			srcBpp = 2;
			rave_format=kQAPixel_ARGB16;
			break;

		case ATR_IMGFMT_ARGB_8888:
			srcBpp = 4;
			rave_format=kQAPixel_ARGB16;
			break;

		case ATR_IMGFMT_ARGB_8332:
			srcBpp = 2;
			rave_format=kQAPixel_ARGB16;
			break;

		case ATR_IMGFMT_ARGB_4444:
			srcBpp = 2;
			rave_format=kQAPixel_ARGB16;
			break;

		case ATR_IMGFMT_AYIQ_8422:
			srcBpp = 2;
			rave_format=kQAPixel_ARGB16;

			tex_mem_required=txInit3dfInfo( &info, ATR_IMGFMT_ARGB_1555,&img->width, &img->height,
                                      nlevels, TX_AUTORESIZE_DISABLE );

			info.data = malloc( tex_mem_required ); 

    		txConvert(	&info, img->format,img->width, img->height, img->data, TX_DITHER_ERR,
    				img->table );

	      tmp_aimg.data    = info.data;
         tmp_aimg.format  = ATR_IMGFMT_ARGB_1555;
   		break;

		case ATR_IMGFMT_YIQ_422:
			srcBpp = 2;
			rave_format=kQAPixel_ARGB16;

			tex_mem_required=txInit3dfInfo( &info, ATR_IMGFMT_ARGB_1555,&img->width, &img->height,
                                      nlevels, TX_AUTORESIZE_DISABLE );

			info.data = malloc( tex_mem_required ); 

    		txConvert(	&info, img->format,img->width, img->height, img->data, TX_DITHER_ERR,
    				img->table );

	      tmp_aimg.data    = info.data;
         tmp_aimg.format  = ATR_IMGFMT_ARGB_1555;
			break;
#if 0
		case -1:
			srcBpp = 1;
			rave_format=kQAPixel_CL8;
			break;

		case -2:
			srcBpp = 2;
			rave_format=kQAPixel_RGB16;
			break;

		case -4:
			srcBpp = 4;
			rave_format=kQAPixel_RGB32;
			break;
#endif

		default:
			atuError(FXFALSE, "atrRAVRealizeImg: Unsupported source format %d\n", img->format);
			rave_format=-1;
			break;
    }


	rimg=(TQAImage *)malloc(sizeof(TQAImage)*nlevels);
	tmp_rimg=rimg;	 

	for(i=0;i<nlevels;i++)
	{	

//if(i>0)break;
		image_size=tmp_aimg.width*tmp_aimg.height*dstBpp;
		src_data=tmp_aimg.data;
	
		dst_data=malloc(image_size);

		if(!dst_data)return FXFALSE;		//make sure to deallocate any previous allocations

		/*** construct the header for each of the subimages in the mipmap ***/

		tmp_rimg->width=tmp_aimg.width;

		tmp_rimg->height=tmp_aimg.height;
		tmp_rimg->rowBytes=dstBpp*tmp_aimg.width;
		tmp_rimg->pixmap=dst_data;	 
	
		for(row=0;row<tmp_aimg.height;row++)
		{
			for(col=0;col<tmp_aimg.width;col++)
			{

				pixel=_atrRAVExtractPixel((short)tmp_aimg.format,src_data,&red,&green,&blue,&alpha);
			
//if(row<20 && col <20)pixel=MAKE_RGB16(0,0,31);

				if(dstBpp==4)
					*((unsigned long *)dst_data)=pixel;
				else if(dstBpp==2)
					*((unsigned short *)dst_data)=(unsigned short)pixel;
				else if(dstBpp==1)
					*((unsigned char *)dst_data)=(unsigned char)pixel;

				dst_data+=dstBpp;
				src_data+=srcBpp;
			}
		}

		flip_image(tmp_rimg);		//inverts the image for Rave

		/*** advance the data pointer to the next mipmap image, which is 
		 *** 1/4 the size of the previous one, so the width and height
		 *** are divided by two. ***/

      tmp_aimg.data = ((char *)tmp_aimg.data)+tmp_aimg.width*tmp_aimg.height*srcBpp;
      tmp_aimg.width >>=1 ;
      tmp_aimg.height >>= 1;

		tmp_rimg++;							//skip to the next mipmap struct
	}

	/*** this is needed to preserve the image format, something that TQAImage doesn't
	 *** do. ***/

	ximage=(_AtrXImage *)malloc(sizeof(_AtrXImage));
	ximage->rave_image=rimg;
	ximage->rave_format=rave_format;
	ximage->bitmap=NULL;

	img->devPrivate=ximage;

	if(info.data)free(info.data);

   return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: _atrRAVUnrealizeImg
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

static void 
_atrRAVUnrealizeImg( AtrImg *img ) 
{
	TQAImage *rave_image;
	_AtrXImage *ximage;
	short nlevels;
	short i;

	ximage=(_AtrXImage *)img->devPrivate;
	
	if(!ximage)return;
	
	rave_image=ximage->rave_image;

	nlevels=(short)img->nLevels;

	if(!nlevels)nlevels=0;

	for(i=0;i<nlevels;i++)
	{	
		free(rave_image->pixmap);
		rave_image++;
	}

	free(img->devPrivate);

   img->devPrivate = NULL;
}

/*-------------------------------------------------------------------
  Function: _atrRAVCloneImg
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

FxBool _atrRAVCloneImg( AtrImg *dst, const AtrImg *src ) 
{
	unsigned long tex_mem_required;
	Gu3dfInfo info;	
	int len;


	dst->format=src->format;
	dst->width=src->width;
	dst->height=src->height;
	dst->nLevels=src->nLevels;
	dst->table=src->table;			//dunno if I have to dupe the table, since I don't know what
											//this table is
	dst->devPrivate=src->devPrivate;

	/*** use this utility to simply find out how much memory the data takes, since
	 *** it is all contiguous. ***/

	tex_mem_required=txInit3dfInfo( &info,(GrTextureFormat_t) dst->format,(int *)&dst->width,(int *)&dst->height,
                                      dst->nLevels, TX_AUTORESIZE_DISABLE );

	len=strlen(src->name);
	dst->name=malloc(len+1);
	if(!dst->name)return FXFALSE;
	strncpy(dst->name,src->name,len);

	dst->data=malloc(tex_mem_required);
	if(!dst->data)return FXFALSE;
	memcpy(dst->data,src->data,(size_t)tex_mem_required);

	return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: _atrRAVTexAssociate
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Associate a texture with an image. This info is then used in TramAllocate
	to actually create the texture.
  Arguments:
      handle   - handle to texture
      img      - image to associate with handle
  Return:
  FXTRUE on success, FXFALSE otherwise
  -------------------------------------------------------------------*/

static FxBool 
_atrRAVTexAssociate( AtrTexHandle handle, AtrImg *img ) 
{
	_AtrRaveTexEntry	*tex;

	if(!img->devPrivate)return FXFALSE;

	tex=(_AtrRaveTexEntry *)handle;
	tex->ximage=(_AtrXImage *)img->devPrivate;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: _atrRAVTexSource
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
_atrRAVTexSource( _AtrTexEntry *entry, FxU32 mask ) 
{
	_AtrRaveTexEntry	*tex_entry;
	int tmu;

	tex_entry=(_AtrRaveTexEntry *)entry;

	if(!entry)return;	
	if(!tex_entry->tex)return;

	tmu=entry->atrInfo.tmu;

	if(tmu==ATR_TEXELFX_0)
		QASetPtr(_atrRAVDriver.ravecontext,kQATag_Texture0,tex_entry->tex);
	else if(tmu==ATR_TEXELFX_1)
		QASetPtr(_atrRAVDriver.ravecontext,kQATag_Texture1,tex_entry->tex);
}


/*-------------------------------------------------------------------
  Function: _atrRAVTramAllocate
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
_atrRAVTramAllocate( _AtrTexEntry *entry ) 
{
	static _AtrXImage *image;
	TQATexture *texture;
	_AtrRaveTexEntry *tex;
	static short firsttime=1;
	static short counter=0;
	TQAError error;

	tex=(_AtrRaveTexEntry *)entry;
	
//	if(counter<32)
	{

		image=tex->ximage;
	}

	error=QATextureNew(_atrRAVDriver.engine,kQATexture_NoCompression|kQATexture_Mipmap,image->rave_format,image->rave_image,&texture);

//	QATextureNew(_atrRAVDriver.engine,kQATexture_NoCompression,image->rave_format,image->rave_image,&texture);
 	tex->tex=texture;

	counter++;
}

/*-------------------------------------------------------------------
  Function: _atrRAVTexPunt
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
_atrRAVTexPunt( _AtrTexEntry *entry ) 
{

	/*** I think this is a NOP under RAVE ***/
}

/*-------------------------------------------------------------------
  Function: _atrRAVTexDeleteHandle
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
_atrRAVTexDeleteHandle( _AtrTexEntry *entry) 
{
	_AtrRaveTexEntry *tex;
	
	tex=(_AtrRaveTexEntry *)entry;

 	QATextureDelete(_atrRAVDriver.engine,tex->tex);
	tex->tex=NULL;
}

/*-------------------------------------------------------------------
  Function: _atrRAVUpdateEnv
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
_atrRAVUpdateEnv( AtrEnv *e ) 
{
   if ( e->flags & ATR_FOG_ON_DEPTH ) 
	{
		QASetInt(_atrRAVDriver.ravecontext,kQATag_Fog,kQAFog_On);

		QASetFloat(_atrRAVDriver.ravecontext,kQATag_FogColor_r,e->fogColor.r);
		QASetFloat(_atrRAVDriver.ravecontext,kQATag_FogColor_g,e->fogColor.g);
		QASetFloat(_atrRAVDriver.ravecontext,kQATag_FogColor_b,e->fogColor.b);

		QASetFloat(_atrRAVDriver.ravecontext,kQATag_FogDensity,e->fogDensity);
		QASetFloat(_atrRAVDriver.ravecontext,kQATag_FogNearW,e->fogNearW);
		QASetFloat(_atrRAVDriver.ravecontext,kQATag_FogFarW,e->fogFarW);
	}
	else
		QASetInt(_atrRAVDriver.ravecontext,kQATag_Fog,kQAFog_Off);	
}

/*-------------------------------------------------------------------
  Function: _atrRAVRenderBuffer
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
_atrRAVRenderBuffer( AtrBuffer buf ) {
}

/*-------------------------------------------------------------------
  Function: _atrRAVUpdateMaterial
  Date: 10/6
  Implementor(s): RMS
  Library: AT Render
  Description:
  Set the current material
  Arguments:
      m        - new material
  Return:
      Nothing
  -------------------------------------------------------------------*/

static void 
_atrRAVUpdateMaterial( AtrMaterial *m ) 
{
	/*** The reference routine for this is in atr/rglide.c ***/

//	DWORD blendCaps = d3dapp->ThisDriver.Desc.dpcTriCaps.dwTextureBlendCaps;

   m->sysFlags = 0;
	
	/*** for textured triangles we need a different routine than a simple
	 *** gouraud shaded triangle. So set the desired routine here. ***/

	if(m->typeFlag==ATR_MAT_GSHADE)
		_atrRAVDriver.any.DrawTriangle=_atrRAVDrawGTriangle ;
	else
		_atrRAVDriver.any.DrawTriangle=_atrRAVDrawTriangle ;
	
	/*** call one of the new Rave state vars for Chromakeying. Needed for the
	 *** shameless plug. ***/

	if(m->chromaKeyEnable==FXTRUE)
		QASetInt(_atrRAVDriver.ravecontext,kQATag_ChromaKey,kQAChromaKey_On);
	else
		QASetInt(_atrRAVDriver.ravecontext,kQATag_ChromaKey,kQAChromaKey_Off);		
		
   switch( m->irgbSrc ) 
	{    
        case ATR_IRGBSRC_LIGHTING:
            /* Set up IRGBSRC callback */
            _atrRenderCache->irgbSrcFunc = _atrIRGBSRC_LIGHTING;
            /* Puzzle out appropriate lighting equation */
            m->sysFlags |= ATR_LIGHTFUNC_DIFFUSE;
            if ( FLOAT_BITS( m->specular.r ) +
                 FLOAT_BITS( m->specular.g ) +
                 FLOAT_BITS( m->specular.b ) != 0 )
				{ 
                m->sysFlags |= ATR_LIGHTFUNC_SPECULAR;
					QASetInt(_atrRAVDriver.ravecontext,kQATag_TextureOp,kQATextureOp_Modulate|kQATextureOp_Highlight);
				}

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
        _atrRenderCache->texCoordSrcFunc[0] = _atrRAV_TCSRC0_TC0;
        break;
      case ATR_TCSRC_TC1:
        _atrRenderCache->texCoordSrcFunc[0] = _atrRAV_TCSRC0_TC1;
        break;
      case ATR_TCSRC_TC2:
        _atrRenderCache->texCoordSrcFunc[0] = _atrRAV_TCSRC0_TC2;
        break;
      case ATR_TCSRC_EMAP:
        _atrRenderCache->texCoordSrcFunc[0] = _atrRAV_TCSRC0_EMAP;
        break;
      case ATR_TCSRC_LMAP:
        _atrRenderCache->texCoordSrcFunc[0] = 0;
        break;
      case ATR_TCSRC_PROJECTED:
        _atrRenderCache->texCoordSrcFunc[0] = _atrRAV_TCSRC0_PROJECTED;
        break;
      case ATR_TCSRC_PLANAR:
        _atrRenderCache->texCoordSrcFunc[0] = _atrRAV_TCSRC0_PLANAR;
        break;
      default:
        _atrRenderCache->texCoordSrcFunc[0] = 0;
        break;
    }

    switch( m->tcSrc[1] ) {
      case ATR_TCSRC_TC0:
        _atrRenderCache->texCoordSrcFunc[1] = _atrRAV_TCSRC1_TC0;
        break;
      case ATR_TCSRC_TC1:
        _atrRenderCache->texCoordSrcFunc[1] = _atrRAV_TCSRC1_TC1;
        break;
      case ATR_TCSRC_TC2:
        _atrRenderCache->texCoordSrcFunc[1] = _atrRAV_TCSRC1_TC2;
        break;
      case ATR_TCSRC_EMAP:
        _atrRenderCache->texCoordSrcFunc[1] = _atrRAV_TCSRC1_EMAP;
        break;
      case ATR_TCSRC_LMAP:
        _atrRenderCache->texCoordSrcFunc[1] = 0;
        break;
      case ATR_TCSRC_PROJECTED:
        _atrRenderCache->texCoordSrcFunc[1] = _atrRAV_TCSRC1_PROJECTED;
        break;
      case ATR_TCSRC_PLANAR:
        _atrRenderCache->texCoordSrcFunc[1] = _atrRAV_TCSRC1_PLANAR;
        break;
      default:
        _atrRenderCache->texCoordSrcFunc[1] = 0;
        break;
    }

    /* Set material flags */
    m->sysFlags |= m->texSrc[0] | (m->texSrc[1]<<ATR_TEX1SHIFT);

	switch( m->texSrc[0] ) {
	  case ATR_TEXSRC_DECAL:
		atrTexSource( m->texture[0] );
		break;

	  case ATR_TEXSRC_EMAP:
// 		QASetInt(_atrRAVDriver.ravecontext,kQATag_TextureOp,kQATextureOp_None);
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
		atrTexSource( m->texture[0] );
		break;

	  case ATR_TEXSRC_PROJECTED:
		atrTexSource( m->texture[0] );
		break;

	  case ATR_TEXSRC_LMAP:
	  case ATR_TEXSRC_NONE:
		//zero out any possible texture handle
      default:
        break;
    }        
	
/*** set stuff for the second texture ***/

	/*** this works only for a 2-TMU system ***/

    if ( m->texSrc[1] != ATR_TEXSRC_NONE )
	 {
			gNumTMUs=2;		//really lame.

			switch( m->texSrc[1] ) {
				case ATR_TEXSRC_DECAL:
					atrTexSource( m->texture[1] );
					break;

				case ATR_TEXSRC_EMAP:
#ifdef AT_DEBUGGING
			if ( m->texture[1] == 0 ) 
				 atuError( FXTRUE, "atrPushMaterial(): No texture to source.\n" ); 
				if (_atrTexTmuFromHandle( m->texture[1] ) == ATR_TEXELFX_1)
					  atuError( FXTRUE,
								"atrPushMaterial(): Texture handle mismatch, the ->texture[0] allocated\n"
								"on TEXELFX 1\n" ); 
#endif
					atrTexSource( m->texture[1] );
					break;

				case ATR_TEXSRC_DETAIL:
					atrTexSource( m->texture[1] );
					break;

				case ATR_TEXSRC_PROJECTED:
					atrTexSource( m->texture[1] );
					break;

				case ATR_TEXSRC_LMAP:
				case ATR_TEXSRC_NONE:
					//zero out any possible texture handle
					default:
				break;
			}        
	}
	else
	{
		gNumTMUs=1;
	}

    /* TBD: this should map to different drawing primitives that use
            this color. Flat shaded
     */

    switch( m->crgbSrc ) 
	{
		case ATR_CRGBSRC_STATIC:

			break;
		default:
			break;
    }

    switch ( m->typeFlag & ATR_MAT_LIGHTING_MASK ) 
	{
		case ATR_MAT_LIGHTING_NONE:
			QASetInt(_atrRAVDriver.ravecontext,kQATag_TextureOp,kQATextureOp_None);
			break;

		case ATR_MAT_LIGHTING_MULTIPLY:
			QASetInt(_atrRAVDriver.ravecontext,kQATag_TextureOp,kQATextureOp_Modulate);
			break;

		case ATR_MAT_LIGHTING_ADD:
			QASetInt(_atrRAVDriver.ravecontext,kQATag_TextureOp,kQATextureOp_Modulate); 
			break;

		case ATR_MAT_LIGHTING_BLEND_ON_TEXALPHA:
			QASetInt(_atrRAVDriver.ravecontext,kQATag_TextureOp,kQATextureOp_Modulate);
			break;
    }

	if( m->texMinFilter[0] == ATR_TEXFILTER_BILINEAR)
		QASetInt(_atrRAVDriver.ravecontext,kQATag_TextureFilter,kQATextureFilter_Mid);
	else
		QASetInt(_atrRAVDriver.ravecontext,kQATag_TextureFilter,kQATextureFilter_Fast);

    if ( _atrRAVDriver.any.caps.numTex == 2 ) 
	 {
		/*** handle the second TMU here. ***/

  //      grTexFilterMode( GR_TMU1, m->texMinFilter[1], m->texMagFilter[1] );

    }

    /* specify alpha test and reference value */

    /* face culling, TBD: ATB does not support CCW */

    /* texture filtering TBD: the following modes are not supported by ATB
           D3DFILTER_LINEARMIPNEAREST,
           D3DFILTER_LINEARMIPLINEAR,
     */

   /* TBD: potential problem with wrapping */

}

/*-------------------------------------------------------------------
  Function: _atrRAVShutdown
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
_atrRAVShutdown( void ) 
{
	gInitialized=0;

	QADrawContextDelete(_atrRAVDriver.ravecontext);
}

/*-------------------------------------------------------------------
  Function: _atrRAVInit
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Create all DirectDraw and Direct3D objects necessary to begin rendering.
    and Initialize the RAV ATB driver state including the capabilities 
    structure
  Arguments:
      driver info - descibing specific configaration
  Return:
      Initialized driver
  -------------------------------------------------------------------*/

AtrDriver *
_atrRAVInit( AtrDriverInfo *driverInfo ) 
{
#if USE_MIKES_ENGINE
	TQAError				qaError;
	short i;
#endif
#if TURN_ON_D3D
	long	flags;
#endif

   TQAEngine *engine;
	RECTANGLE wndrect;
	TQARect raverect;
	TQADrawContext *context;

	long	vendorID = 0, engineID = 0;

   BOOL bOnlySystemMemory, bOnlyEmulation;
	AtrDriverCaps *caps = &_atrRAVDriver.any.caps;

	gDriverInfo=driverInfo;

    _atrSnapBias = (float)0.0;
    caps->width  = 640;
    caps->height = 480;
    caps->bpp  = 16;
    caps->pfxRev = 0;
    caps->pfxMem = 0;
    caps->numTex = (FxU32)driverInfo->info; /* default 1 texel unit */
    caps->tfxRev = 0;
    caps->tfxMem = 0;
    caps->fullScreen = FXTRUE;
     caps->sli    = 0;
    _atrRAVDriver.any.texEntrySize = sizeof(_AtrTexEntry);
    _atrRAVInitDispatchTable(&_atrRAVDriver);

#if _WINDOWS
	gDevice.deviceType=kQADeviceWin32DC;
	gDevice.device.d3d.hdc=GetDC(driverInfo->hWnd);
	GetWindowRect(driverInfo->hWnd, &wndrect);
#endif
/*
	raverect.left=wndrect.left;
	raverect.right=wndrect.right;
	raverect.bottom=wndrect.bottom;
	raverect.top=wndrect.top;
*/

	raverect.left=0;
	raverect.right=wndrect.right-wndrect.left;
	raverect.bottom=wndrect.bottom-wndrect.top;
	raverect.top=0;

    bOnlySystemMemory = driverInfo->emulation;
    bOnlyEmulation = driverInfo->emulation;

/*** just for testing now ***/

bOnlyEmulation=0;
bOnlySystemMemory=0;


#if TURN_ON_D3D
    flags = ((bOnlySystemMemory) ? D3DAPP_ONLYSYSTEMMEMORY : 0) | 
            ((bOnlyEmulation) ? (D3DAPP_ONLYD3DEMULATION |
                                 D3DAPP_ONLYDDEMULATION) : 0);

    /*
     * Create all the DirectDraw and D3D objects neccesary to render.  The
     * AfterDeviceCreated callback function is called by D3DApp to create the
     * viewport and the example's execute buffers.
     */


    if (!D3DAppCreateFromHWND(flags, driverInfo->hWnd, _atrD3DAfterDeviceCreated,
                              NULL, _atrD3DBeforeDeviceDestroyed, NULL, &D3dappi)) {
        atuError(FXTRUE, "%s", D3DAppLastErrorString());
    }


   gDevice.deviceType = kQADeviceD3DDevice;
	gDevice.device.d3d.lpView=D3dappi->lpD3DViewport;
	gDevice.device.d3d.lpD3Ddev=D3dappi->lpD3DDevice;
	gDevice.device.d3d.lpD3D=D3dappi->lpD3D;
   gDevice.device.d3d.lpDirectDraw=D3dappi->lpDD;
   gDevice.device.d3d.lpFront=D3dappi->lpFrontBuffer;
   gDevice.device.d3d.lpBack=D3dappi->lpBackBuffer;
   gDevice.device.d3d.lpZbuffer=D3dappi->lpZBuffer;
#endif

#if _WINDOWS
//	test_draw_new_window();
#endif

	/*** should be the default Apple rasterizer ***/

	engine=QADeviceGetFirstEngine(&gDevice);
	
	if(!engine)return NULL;

#if USE_MIKES_ENGINE
	for(i=0;i<10;i++)		//limit this for extra insurance
	{
       	qaError = QAEngineGestalt(engine, kQAGestalt_VendorID, &vendorID);
 			if (qaError != kQANoErr)return NULL;

			qaError = QAEngineGestalt(engine, kQAGestalt_EngineID, &engineID);
			if (qaError != kQANoErr)return NULL;


         if((vendorID ==kVendorID_Mike) && (engineID==kEngineID_Mike))
				break;
         	
			engine = QADeviceGetNextEngine(NULL,&engine );

			if(!engine)return NULL;
	}
#endif

	

	QADrawContextNew(&gDevice,&raverect,NULL,engine,kQAContext_DoubleBuffer,&context);

	_atrRAVDriver.ravecontext=context;
	_atrRAVDriver.rect=raverect;
	_atrRAVDriver.engine=engine;

	QASetInt(_atrRAVDriver.ravecontext,kQATag_TextureFilter,kQATextureFilter_Fast);
	QASetInt(_atrRAVDriver.ravecontext,kQATag_PerspectiveZ,kQAPerspectiveZ_On);

	_atrTexSetScale(ATR_TEXELFX_0, 1.0f, 1.0f);

// 	test_draw();

	gInitialized=1;

	return (AtrDriver *)&_atrRAVDriver;
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
_atrRAVIsDrawable( AtrContext ctx ) 
{
	if(gPause)return FXFALSE;
	if(!gInitialized)return FXFALSE;

   return FXTRUE;
}

/*********************************************************************** 
 *  _atrRAVIsBackface:                                                 *                
 *	Test if polygon is a backface.  Polygon must conform to right       *
 *	hand rule (draw by winding around counter clockwize.  If the        *
 *	surface normal faces the camera it's a front face (0), otherwise    *
 *	it's a back face (1).                                               *
 ***********************************************************************/
BOOL
_atrRAVIsBackface(const GrVertex *a,const GrVertex *b,const GrVertex *c)
{
	float dx1, dy1, dz1;
	float dx2, dy2, dz2;
	float cpx, cpy, cpz;
	float prod;

	dx1 = b->x - a->x;
	dy1 = b->y - a->y;
	dz1 = b->oow - a->oow;
	dx2 = c->x - b->x;
	dy2 = c->y - b->y;
	dz2 = c->oow - b->oow;
	cpx = dy1*dz2 - dz1*dy2;
	cpy = dz1*dx2 - dx1*dz2;
	cpz = dx1*dy2 - dy1*dx2;
	prod = b->x * cpx + b->y * cpy + b->z * cpz;

	if (prod > 0)
		return(1);
	else
		return(0);
}

/*-------------------------------------------------------------------
  Function: _atrRAVResize
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
_atrRAVResize(AtrContext ctx, int w, int h) {
    return FXFALSE;
}

/*-------------------------------------------------------------------
  Function: _atrRAVPause
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
_atrRAVPause(FxBool flag) 
{
		
	if(flag==-1)
		gPause=1;
	else	
		gPause=0;

   return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: _atrRAVIdle
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

static void _atrRAVIdle(void) {
}

/*-------------------------------------------------------------------
  Function: _atrRAVInitDispatchTable
  Date: 10/12/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Initialize the dispatch table for the RAV ATB driver
  Arguments:
    ctx - the rendering context to initialize
  Return:
    Nothing
  -------------------------------------------------------------------*/

static void
_atrRAVInitDispatchTable(AtrRaveDriver *ctx) {
    ctx->any.RealizeImg = _atrRAVRealizeImg ;
    ctx->any.UnrealizeImg = _atrRAVUnrealizeImg ;
    ctx->any.CloneImg = _atrRAVCloneImg ;
    ctx->any.ClearCanvas = _atrRAVClearCanvas ;
    ctx->any.SwapBuffer = _atrRAVSwapBuffer ;
    ctx->any.BeginScene = _atrRAVBeginScene ;
    ctx->any.EndScene = _atrRAVEndScene ;
    ctx->any.RenderImg = _atrRAVRenderImg ;
    ctx->any.GrabImg = _atrRAVGrabImg ;
    ctx->any.TexEntryInit = _atrRAVTexEntryInit ;
    ctx->any.TexDeleteHandle = _atrRAVTexDeleteHandle ;
    ctx->any.TexPunt = _atrRAVTexPunt ;
    ctx->any.TexAssociate = _atrRAVTexAssociate ;
    ctx->any.TexSource = _atrRAVTexSource ;
    ctx->any.TramAllocate = _atrRAVTramAllocate ;
    ctx->any.UpdateEnv = _atrRAVUpdateEnv ;
    ctx->any.UpdateMaterial = _atrRAVUpdateMaterial ;
    ctx->any.Shutdown = _atrRAVShutdown ;
    ctx->any.Idle = _atrRAVIdle ;
    ctx->any.IsDrawable = _atrRAVIsDrawable ;
    ctx->any.Pause = _atrRAVPause ;
    ctx->any.Resize = _atrRAVResize ;
    ctx->any.RenderBuffer = _atrRAVRenderBuffer ;
    ctx->any.DrawLine = _atrRAVDrawLine ;
    ctx->any.DrawPoint = _atrRAVDrawPoint ;
    ctx->any.DrawTriangle = _atrRAVDrawTriangle ;
    ctx->any.Splash = _atrRAVSplash ;
    ctx->any.GetPerfStats = _atrRAVGetPerfStats ;
    ctx->any.ResetPerfStats = _atrRAVResetPerfStats ;
    ctx->any.ClipWindow = _atrRAVClipWindow ;

    ctx->any.TransformVertices = _atrRAVTransformVertices;
    ctx->any.SpecialTransformVertices = _atrRAVSpecialTransformVertices;
    ctx->any.TransformVertices2D = _atrRAVTransformVertices2D;
    ctx->any.RenderTri = atrRAVRenderTri;
    ctx->any.RenderTriSet = atrRAVRenderTriSet;
    ctx->any.SpecialRenderTriSet = atrRAVSpecialRenderTriSet;
    ctx->any.RenderTriWF = atrRAVRenderTriWF;
    ctx->any.RenderTriSetWF = atrRAVRenderTriSetWF;
    ctx->any.TransformOpenTriSet = atrRAVTransformOpenTriSet;
    ctx->any.SpecialTransformOpenTriSet = atrRAVSpecialTransformOpenTriSet;
    ctx->any.RenderOpenTriSet = atrRAVRenderOpenTriSet;
    ctx->any.RenderOpenTriSetWF = atrRAVRenderOpenTriSetWF;
    ctx->any.RenderSegment = atrRAVRenderSegment;
    ctx->any.Render2DTri = atrRAVRender2DTri;

    ctx->any.ClipAndRenderTri = _atrRAVClipAndRenderTri;
    ctx->any.ClipAndRenderSegment = _atrRAVClipAndRenderSegment;
}

/*******************************************************************************
 * _atrRAVSetRaveGVertex :  Merely maps an ATB vertex into a Rave gouraud vertex. *                    				   *
 *******************************************************************************/
void
_atrRAVSetRaveGVertex(TQAVGouraud *v,const GrVertex *a,long h)
{
	v->x = (float)a->x;
	v->y = (float)h-a->y;
//	v->z = (float)a->z;
	v->z = (float)1.0-a->oow;

	v->invW = (float)a->oow;
	v->a = (float)a->a/255.0f;
	v->r = (float)a->r/255.0f;
	v->g = (float)a->g/255.0f;
	v->b = (float)a->b/255.0f;
}

/*******************************************************************************
 * _atrRAVSetRaveTVertex :  Merely maps an ATB vertex into a Rave texture      *
 *		vertex.                                                                  *
 *******************************************************************************/
void
_atrRAVSetRaveTVertex(TQAVTexture_TC *v,const GrVertex *a,long h)
{
	float scale=255.0f;

	v->x=a->x;
	v->y=h-a->y;

 	v->z=(float)(1.0-a->tmuvtx[0].oow);

	/*** this is needed for two pass textures with the RAVE engine, since
	 *** RAVE's z comparison is "draw if new < old". Otherwise if they are
	 *** identical, don't bother. ***/

	if(gPassNumber==2)
	{
		v->z-=.001f;
	}

	v->invW=a->tmuvtx[0].oow;

	v->r=a->r/scale;			
	v->g=a->g/scale;
	v->b=a->b/scale;
	v->a=a->a;

	v->uOverW0=a->tmuvtx[0].sow;
	v->vOverW0=a->tmuvtx[0].tow;

	/*** scales the colors of the specular reflection up or down, from 0 to 1.0. Used with
	 *** kQATextureOp_Highlight ***/

	v->ks_r=v->r;	
	v->ks_g=v->g;
	v->ks_b=v->b;

	/*** scales the value of the texture up or down, from 0 to 1.0. Used with
	 *** kQATextureOp_Modulate. Each "color" effects the value (light/dark)
	 *** of the pixel by 1/3. So the brightest possible value would be
	 *** 1,1,1. Three 0s would yield black. Turning on 1 for any of the 3 
	 *** gives a dim value, that is bright red would show as a dark red. 
	 *** This is used for shading effects. ***/

	v->kd_r=v->r;
	v->kd_g=v->g;
	v->kd_b=v->b;

	/*** the rest of the stuff is for support of multiple textured surfaces. ***/

	v->invW0=v->invW;		
	v->invW1=a->tmuvtx[1].oow;

	v->uOverW1=a->tmuvtx[1].sow;
	v->vOverW1=a->tmuvtx[1].tow;
}

/************************************************************************
 * draw_marker_triangle : draws a triangle over a particular tri on the *
 *		rAVE window, used for marking bad tris for debugging.             *
 ************************************************************************/
void
draw_marker_triangle(TQAVTexture_TC v[])
{
	POINT array[3];
	HBRUSH hbrush;

	array[0].x=(long)v[0].x;
	array[0].y=(long)v[0].y;

	array[1].x=(long)v[1].x;
	array[1].y=(long)v[1].y;

	array[2].x=(long)v[2].x;
	array[2].y=(long)v[2].y;

	hbrush=CreateSolidBrush(RGB(255,0,0));
	SelectObject(gDevice.device.d3d.hdc,hbrush);
	Polygon(gDevice.device.d3d.hdc,array,3);
	DeleteObject(hbrush);
}

/*******************************************************************************
 * flip_image : inverts an image since Rave displays atb stuff upsidedown for  *
 *		some reason.                                                             *                                              				       *
 *******************************************************************************/
BOOL
flip_image(TQAImage *image)
{
	unsigned char *mem_ras,*flip_mem_ras,*flipped_line,*line;
	short cols,rows,bpp;
	short col,row;
	long image_size;
	short i;
	short row_bytes;

	cols=(short)image->width;
	rows=(short)image->height;

	/*** for the final image in the mipmap, the width or height may be 0
	 *** in the case of non-square images. So lets just skip it. ***/

	if(!rows || !cols)return 1;

	bpp=(short)image->rowBytes/cols;
	mem_ras=image->pixmap;
	
	image_size=cols*rows*bpp;
	row_bytes=(short)image->rowBytes;
	flip_mem_ras=(unsigned char *)malloc(image_size);

	if(!flip_mem_ras)return 0;

	flip_mem_ras+=image_size;		//go to the end of the raster
	flip_mem_ras-=cols*bpp;			//move back to the start of the last scanline

	for(row=0;row<rows;row++)
	{
		/*** copy a line ***/

		line=mem_ras;

		flipped_line=flip_mem_ras;

		for(col=0;col<cols;col++)
		{
			for(i=0;i<bpp;i++)		//one pixel at a time
			{
				*flipped_line=*line;

				line++;
				flipped_line++;
			}
		
		}
			
		flip_mem_ras-=row_bytes;
		mem_ras+=row_bytes;
		
	}

	flip_mem_ras+=row_bytes;		//make up for the last decrement which was one too many
	mem_ras-=image_size;				//reset back tot he start and free

	free(mem_ras);						//the original pointer, we no longer need

	image->pixmap=flip_mem_ras;

	return FXTRUE;
}

/*******************************************************************************
 * set_line :                                               				       *
 *******************************************************************************/
void
set_line(TQAVGouraud v[])
{
	v[0].x = (float) 0.0;
	v[0].y = (float)0.0;
	v[0].z = (float).1;
	v[0].invW = (float)1.0;
	v[0].a = (float)1.0;
	v[0].r = (float)1.0;
	v[0].g = (float)0.0;
	v[0].b = (float)0.0;

	v[1].x = (float)300;
	v[1].y = (float)300;
	v[1].z = (float).3;
	v[1].invW = (float)1.0;
	v[1].a = (float)1.0;
	v[1].r = (float)1.0;
	v[1].g = (float)1.0;
	v[1].b = (float)0.0;
}

/*******************************************************************************
 * validate_triangle : returns a 0 if any of the triangles vertices are out of *
 *		bounds. A poor man's clipper.                                            *
 *******************************************************************************/
BOOL
validate_triangle(TQAVTexture_TC v[],TQARect rect)
{
	long width,height;
	short i;
	long x,y;

	width=rect.right-rect.left;
	height=rect.bottom-rect.top;

	for(i=0;i<3;i++)
	{
		x=(long)v[i].x;
		y=(long)v[i].y;

		if(x<=0)return 0;
		if(x>width)return 0;

		if(y<=0)return 0;
		if(y>height)return 0;
	}

	return 1;
}

/*** the following code is for test cases ***/

#if 0
TQAImage *Image;
TQADevice gDevice_temp;
//extern HINSTANCE Hinstance; 
HWND Hwnd;

/*******************************************************************************
 * create_demo_image16 : creats a square image of colorbars with 16BPP format  *
 *		64 pixels on a side.                                                 	 *
 *******************************************************************************/
TQAImage *
create_demo_image16(void)
{
	WORD *data=NULL,*ptr=NULL;
	int width=64,height=64;
	WORD color;
	int i,j;
	TQAImage *image;

	data=malloc(width*height*2);
	ptr=data;

	if(!data)return NULL;

	for(i=0;i<height;i++)
	{
		if(i<16)
      		color=MAKE_RGB16(31,0,0);
		else if (i>=16 && i<32)
      		color=MAKE_RGB16(0,31,0);
		else if (i>=32 && i<64)
      		color=MAKE_RGB16(0,0,31);
		else
      		color=MAKE_RGB16(31,0,31);


   		for(j=0;j<width;j++)
		{
 			*ptr=color;

			ptr++;
		}
	}

	image=malloc(sizeof(TQAImage));

	image->width=width;
	image->height=height;
	image->rowBytes=2*width;
	image->pixmap=data;

	return(image);
}



/*******************************************************************************
 * WndProc : window callback procedure                     							 *
 *******************************************************************************/
LRESULT CALLBACK
WndProc(HWND hWnd,UINT message,WPARAM uParam,LPARAM lParam)
{
	int			wmId, wmEvent;
	PAINTSTRUCT PaintStruct;
	char buffer[50];
   
	switch (message)
	{
		case WM_CREATE:

			break;

		case WM_DESTROY:  // message: window being destroyed
 //        cleanup();
			PostQuitMessage(0);

			break;

		case WM_PAINT:
			BeginPaint(hWnd, &PaintStruct);
			EndPaint(hWnd, &PaintStruct);
			break;

		default:          // Passes it on if unproccessed
			return (DefWindowProc(hWnd, message, uParam, lParam));
    }
    return (0);
}

/*******************************************************************************
 * create_window : create the window and set the "device" type.					 *
 *******************************************************************************/
create_window()
{
#if 0
		BOOL	aResult;
		RECT	aRect;
		int		width, height;
		short i;
	    WNDCLASS  wc;


    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc =  (WNDPROC)WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = Hinstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "mike";

    RegisterClass(&wc);

#if 1
		Hwnd = CreateWindow(
				"mike",
				"kramer",
				WS_OVERLAPPEDWINDOW | WS_VISIBLE,
				250, 250, 400, 400, 	 // x, y, width, height
				NULL,
				NULL,
				Hinstance,
				NULL
		);
#endif

#if 0
    gWindowPtr[1] = CreateWindowEx(
         WS_EX_APPWINDOW,
         szAppName,
         "Mike's Engine",
         WS_OVERLAPPED | WS_CAPTION |
         WS_THICKFRAME | WS_MINIMIZEBOX | WS_VISIBLE,
         800, 0,
         MAX_WIN_WIDTH,MAX_WIN_HEIGHT,
         NULL,                              /* parent window */
         NULL,                              /* menu handle */
         hInstance,                         /* program handle */
         NULL);  

		if (gWindowPtr[1] == NULL)
			return kGSError_OS;
#endif

			gDevice_temp.deviceType = kQADeviceWin32DC;
			gDevice_temp.device.d3d.hdc = GetDC(Hwnd);
#endif

}

/********************************************************************
 * test_draw : tests some drawing to the rave surface.              *
 ********************************************************************/
test_draw_new_window()
{
	TQAVGouraud line[2],bv;
	TQABitmap *bitmap;
	TQAError error;
	TQAEngine *engine;
	TQARect raverect;
	TQADrawContext *context;
	RECTANGLE wndrect;

	create_window();
	engine=QADeviceGetFirstEngine(&gDevice_temp);

	GetClientRect(Hwnd, &wndrect);

	raverect.left=wndrect.left;
	raverect.right=wndrect.right;
	raverect.bottom=wndrect.bottom;
	raverect.top=wndrect.top;

	QADrawContextNew(&gDevice,&raverect,NULL,engine,kQAContext_DoubleBuffer,&context);

	QARenderStart(context,NULL,NULL);

	set_line(line);

	QADrawLine(context,&line[0],&line[1]);



  	Image=create_demo_image16();


   bv.x=0.0f;
   bv.y=0.0f;
   bv.z=0.4f;
   bv.a=1.0f;
   bv.r=1.0f;
   bv.g=1.0f;
   bv.b=1.0f;

   error=QABitmapNew(engine,0L,kQAPixel_ARGB16,Image,&bitmap);
	QADrawBitmap(context,&bv,bitmap);


	QADrawLine(context,&line[1],&line[0]);
	
	QARenderEnd(context,NULL);
	return 1;
}
#endif


#if 1
/********************************************************************
 * test_draw : tests some drawing to the rave surface.              *
 ********************************************************************/
void 
test_draw(void)
{
	TQAVGouraud line[2];


#if 0	
	QASetFloat(_atrRAVDriver.ravecontext,kQATag_ColorBG_a,(float)1.0);
	QASetFloat(_atrRAVDriver.ravecontext,kQATag_ColorBG_r,(float)1.0);
	QASetFloat(_atrRAVDriver.ravecontext,kQATag_ColorBG_b,(float)0.0);
	QASetFloat(_atrRAVDriver.ravecontext,kQATag_ColorBG_g,(float)0.0);
#endif

	QARenderStart(_atrRAVDriver.ravecontext,NULL,NULL);

	set_line(line);
	QADrawLine(_atrRAVDriver.ravecontext,&line[0],&line[1]);
	QARenderEnd(_atrRAVDriver.ravecontext,NULL);
}

#endif
