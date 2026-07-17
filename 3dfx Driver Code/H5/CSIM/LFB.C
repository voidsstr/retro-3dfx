#include "vxd.h"

/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 4$
** $Date: 10/11/00 8:08:57 PM$
*/

#include <stdlib.h>

#include <h3.h>
#include "h3sim.h"
#include "rgbfmt.h"

FxU32
csimFbiLfbRead( SstRegs *sst, FxU32 addr )
{
    FxU32 temp;
    int buf=0, x,y;
    FxU32 lfbMode = sst->lfbMode;
    FxU32 data;

    addr = SST_FAKE_ADDRESS_GET_OFFSET(addr) - SST_LFB_OFFSET;
    addr >>= 1;				// convert to short address
    x = (addr & SST_LFB_ADDR_X) >> SST_LFB_ADDR_X_SHIFT;
    y = (addr & SST_LFB_ADDR_Y) >> SST_LFB_ADDR_Y_SHIFT;

    if (lfbMode & SST_LFB_YORIGIN)
      {
	// check where to get yorigin subtraction value from, put result into y
	if (sst->renderMode & SST_RM_YORIGIN_SELECT) { //use renderMode
	  y = (sst->renderMode & SST_RM_YORIGIN_TOP) >> SST_RM_YORIGIN_TOP_SHIFT;
	}
      }
    else {	// use miscInit0
      SstIORegs *sstio = &(CSIM_PRIVATE(sst))->io; 
      y = ((sstio->miscInit0 & SST_YORIGIN_TOP) >> SST_YORIGIN_TOP_SHIFT) - y;
    }
    
    GDBG_INFO(131,"\t-LFB read 2 addr= %06x  x,y=%d,%d\n",addr,x,y);

    // decode read buffer select and read the appropriate buffer
    switch(lfbMode & SST_LFB_READBUFSELECT) {
	case SST_LFB_READCOLORBUFFER:
	  buf = CSIM_BUF_3D_COLOR;
	  temp = csimReadPixel(sst,buf,x,y);
	  data = _sstRgbaLanes565(sst, temp);
	  temp = csimReadPixel(sst,buf,x+1,y);
	  data |= _sstRgbaLanes565(sst, temp) << 16;
	  break;
	case SST_LFB_READDEPTHABUFFER:
	    data = csimReadPixel(sst,CSIM_BUF_3D_AUX1,x,y);		// get first pixel
	    data |= csimReadPixel(sst,CSIM_BUF_3D_AUX1,x+1,y)<<16;	// get second pixel
	    break;
    }

    GDBG_INFO(132,"\t\tdata before swap = %08x\n",data);

    if (lfbMode & SST_LFB_READ_BYTESWAP) {	// byte swap
	data = (data<<24) | ((data&0xFF00)<<8) | ((data>>8)&0xFF00) | (data>>24);
    }
    if (lfbMode & SST_LFB_READ_SWAP16) {	// swap the 2 16-bit words
	data = (data << 16) | (data >> 16);
    }
    GDBG_INFO(132,"\t\tdata  after swap = %08x\n",data);

    return data;
}

FxU32 
csimFbiLfbSize(SstRegs *sst)
{
  FxU32 size;
  switch( sst->lfbMode & SST_LFB_FORMAT ) {
  case SST_LFB_565:
  case SST_LFB_555:
  case SST_LFB_1555:
  case SST_LFB_ZZ:   
    size = 2;
    break;
  case SST_LFB_888:
  case SST_LFB_8888:
  case SST_LFB_Z565:
  case SST_LFB_Z555:
  case SST_LFB_Z1555:
  case SST_LFB_Z32:
    size = 4;
    break;
  default:
    GDBG_ERROR("csimFbiLfbSize","Invalid lfb format %x\n",sst->lfbMode & SST_LFB_FORMAT);
  }
  return(size);
}

static char *lfbfmt_str[] = {"RGB-565","RGB-555","ARGB-1555","****",
			   "RGB-888","ARGB-8888","****","****",
			   "Z-32","****","****","****",
			   "Z+565","Z+555","Z+1555", "Z+Z"};

//----------------------------------------------------------------------
// draw a pixel in Linear Frame Buffer (LFB) mode
// this routine passes the 2 colors to sstFbiPixel in 
// fbiData.lfbRGBdata, fbiData.lfbAdata, and fbiData.lfbZdata
//----------------------------------------------------------------------
void csimFbiLfbWrite( SstRegs *sst, FxU32 addr, FxU32 data, int words )
{
    int x,y;
    FxU16 adata1,adata2;
    FxU32 zdata1, zdata2;		// left justified .32
    FxU32 fmt, iaddr, data1,data2;
    FxU32 saveCP = sst->fbzColorPath;	// save the old fbzColorPath
    FxU32 saveCM = sst->combineMode;	// save the old combineMode
    CsimPrivate *cp = CSIM_PRIVATE(sst);

    iaddr = SST_FAKE_ADDRESS_GET_OFFSET(addr) - SST_LFB_OFFSET;

    //Do some error checking
    if(sst->fbzColorPath & SST_ENTEXTUREMAP)
      {
	if(!(sst->combineMode & SST_CM_USE_COMBINE_MODE))
	  {//Not using combine mode
	    if(((sst->fbzColorPath & SST_RGBSELECT) == SST_RGBSEL_TREXOUT) ||
	       ((sst->fbzColorPath & SST_ASELECT) == SST_ASEL_TREXOUT))
	      GDBG_ERROR("sstFbiLfb",
			 "illegal use of LFB with texture mapping %s(%d)\n\t\t(fbzColorPath=0x%x) (combineMode=0x%x)\n",
			 __FILE__, __LINE__, sst->fbzColorPath, sst->combineMode);	    
	  }
	else 
	  {//Using combine mode
	    if(((sst->combineMode & SST_CM_CC_OTHERSELECT) == SST_CM_CC_OTHERSELECT_TRGB) ||
	       ((sst->combineMode & SST_CM_CC_OTHERSELECT) == SST_CM_CC_OTHERSELECT_TA)   ||
	       ((sst->combineMode & SST_CM_CC_LOCALSELECT) == SST_CM_CC_LOCALSELECT_TRGB) ||
	       ((sst->combineMode & SST_CM_CC_LOCALSELECT) == SST_CM_CC_LOCALSELECT_TA)   ||
	       ((sst->combineMode & SST_CM_CCA_OTHERSELECT) == SST_CM_CCA_OTHERSELECT_TA) ||
	       ((sst->combineMode & SST_CM_CCA_LOCALSELECT) == SST_CM_CCA_LOCALSELECT_TA))
	      GDBG_ERROR("sstFbiLfb",
			 "illegal use of LFB with texture mapping %s(%d)\n\t\t(fbzColorPath=0x%x) (combineMode=0x%x)\n",
			 __FILE__, __LINE__, sst->fbzColorPath, sst->combineMode);	    	    
	  }

	if(((sst->fbzColorPath & SST_CC_MSELECT) == SST_CC_MATREX)   ||	    
	   ((sst->fbzColorPath & SST_CC_MSELECT) == SST_CC_MRGBTMU)  ||
	   ((sst->fbzColorPath & SST_CCA_MSELECT) == SST_CCA_MATREX) ||
	   ( sst->fbzColorPath & SST_LOCALSELECT_OVERRIDE_WITH_ATEX))
	  GDBG_ERROR("sstFbiLfb",
		     "illegal use of LFB with texture mapping %s(%d)\n\t\t(fbzColorPath=0x%x) (combineMode=0x%x)\n",
		     __FILE__, __LINE__, sst->fbzColorPath, sst->combineMode);	    
      }

	
    sst->fbzColorPath &= ~(SST_RGBSELECT|SST_ASELECT);	// clear out SELECTs
    sst->combineMode &= ~(SST_CM_CC_OTHERSELECT|SST_CM_CCA_OTHERSELECT);
    sst->fbzColorPath |= SST_RGBSEL_LFB|SST_ASEL_LFB;	// force it to LFB
    sst->combineMode |= SST_CM_CC_OTHERSELECT_LFB_RGB|SST_CM_CCA_OTHERSELECT_LFB_A;

    sst->status |= SST_BUSY;			// set BUSY status
    GDBG_INFO(131,"\t-LFB write %d      = %04x  %04x  mode=0x%x  fmt=%s\n",
			words,data>>16,data&0xFFFF,
			sst->lfbMode,
			lfbfmt_str[(sst->lfbMode&SST_LFB_FORMAT)>>SST_LFB_FORMAT_SHIFT]);
    if (iaddr < 0 || iaddr >= 0x1000000)
	GDBG_ERROR("sstFbiLfb","invalid address offset=0x%x\n",addr);
    if (iaddr & (words==1?1:3))
	GDBG_ERROR("sstFbiLfb","unaligned address=0x%x  data=%d(0x%08x)\n",
			addr,data,data);
    if (words < 2 ) {
	switch( sst->lfbMode & SST_LFB_FORMAT) {
	    case SST_LFB_Z565:
	    case SST_LFB_Z555:
	    case SST_LFB_Z1555:
	    case SST_LFB_Z32:
		GDBG_ERROR("sstFbiLfb","invalid LFB mode 0x%x with 16-bit access\n",
				sst->lfbMode);
		break;
	}
    }

    if (sst->lfbMode & SST_LFB_WRITE_BYTESWAP) {	// byte swap first
	data = (data<<24) | ((data&0xFF00)<<8) | ((data>>8)&0xFF00) | (data>>24);
	if (words==1) {
		iaddr ^= 2;				// change word address
		data >>= 16;
	}
    }
    fmt = sst->lfbMode & SST_LFB_FORMAT;
    if ((sst->lfbMode & SST_LFB_WRITE_SWAP16) &&	// swap the 2 16-bit words
	fmt != SST_LFB_888 && fmt != SST_LFB_8888
	&& fmt != SST_LFB_Z32)
    {
 	if (words==1)
 	  iaddr ^= 2		;			// change word address
 	else
 	  data = (data << 16) | (data >> 16);
    }
    GDBG_INFO(132,"\t\tpost swap = %04x  %04x\n",data>>16,data&0xFFFF);

    iaddr >>= 1;					// convert to 16-bit pixel address
    // default alpha and z comes from zaColor
    adata1 = adata2 = (unsigned short)((sst->zaColor>>24) & 0xFF);

    //This works in 16bpp or 32bpp because zaColor is stored 24bits
    //wide in either case (in 16bpp, zaColor is << 8).
    if((sst->renderMode & SST_RM_3D_MODE) == SST_RM_32BPP)      
      zdata1 = zdata2 = (unsigned int)((sst->zaColor & 0x00FFFFFF)<<8);
    else
      zdata1 = zdata2 = (unsigned int)((sst->zaColor & 0x0000FFFF)<<16);


    // copy fbzMode register, but clear write mask bits and depthbuffer enable
    // these will be set to whatever components need to be written
    // NOTE: SST_ENDEPTHBUFFER is set to fool logic at end of sstFbiPixel()
    cp->fbiData.lfbFBZmode = sst->fbzMode & ~( SST_ENALPHABUFFER |
			SST_ENDEPTHBUFFER | SST_RGBWRMASK | SST_ZAWRMASK);

    //If the pixel pipeline is enabled, then we strictly obey 
    //the renderMode write masks. Otherwise, we need to set
    //the write masks depending on what channels a particular
    //lfb write format contains.
    if(sst->lfbMode & SST_LFB_ENPIXPIPE) //Don't mess with write masks here
      cp->fbiData.lfbRenderMode = sst->renderMode;
    else //Set the write masks depending on format
      cp->fbiData.lfbRenderMode = sst->renderMode 
	& ~(SST_RM_RGB_WMASK | SST_RM_ALPHA_WMASK);	    
    
    switch (fmt) {
#ifdef ENDB
#define ENDIAN_SWAP(x) x = (x<<24) | ((x&0xFF00)<<8) | ((x>>8)&0xFF00) | (x>>24)
#else
#define ENDIAN_SWAP(x)
#endif
	case SST_LFB_565:		// format 0
		if (words > 1) {
		    _sstRgba565to8888(( unsigned char * )&data2,
					_sstRgbaLanes565(sst,data>>16));
		    ENDIAN_SWAP(data2);
		}	
		_sstRgba565to8888((unsigned char * )&data1,
					_sstRgbaLanes565(sst,data));
		ENDIAN_SWAP(data1);
		cp->fbiData.lfbFBZmode |= SST_RGBWRMASK;
		
		if(!(sst->lfbMode & SST_LFB_ENPIXPIPE))
		  cp->fbiData.lfbRenderMode |= SST_RM_RGB_WMASK;
		break;	
	case SST_LFB_555:		// format 1
		if (words > 1) {
		    _sstRgba555to8888(( unsigned char * )&data2,
					_sstRgbaLanes1555(sst,data>>16));
		    ENDIAN_SWAP(data2);
		}
		_sstRgba555to8888((unsigned char *)&data1,
					_sstRgbaLanes1555(sst,data));
		ENDIAN_SWAP(data1);
		cp->fbiData.lfbFBZmode |= SST_RGBWRMASK;

		if(!(sst->lfbMode & SST_LFB_ENPIXPIPE))
		  cp->fbiData.lfbRenderMode |= SST_RM_RGB_WMASK;
		break;	
	case SST_LFB_1555:		// format 2
		if (words > 1) {
		    _sstRgba1555to8888((unsigned char * )&data2,
					_sstRgbaLanes1555(sst,data>>16));
		    ENDIAN_SWAP(data2);
		    adata2 = (unsigned short)(data2 >> 24);	// get alpha
		}
		_sstRgba1555to8888((unsigned char *)&data1,
					_sstRgbaLanes1555(sst,data));
		ENDIAN_SWAP(data1);
		adata1 = (unsigned short)(data1 >> 24);
 		if ( sst->lfbMode & SST_LFB_ENPIXPIPE ) {
 		    cp->fbiData.lfbFBZmode |= SST_RGBWRMASK | SST_ZAWRMASK | 
 		                                ( sst->fbzMode & SST_ENALPHABUFFER );
 		} else {
 		    if ( sst->fbzMode & SST_ENALPHABUFFER ) {
				cp->fbiData.lfbFBZmode |= SST_RGBWRMASK | SST_ZAWRMASK | SST_ENALPHABUFFER;
 		    } else /* Alpha buffer disabled */ {
				cp->fbiData.lfbFBZmode |= SST_RGBWRMASK;
 		    }
 		}

		if(!(sst->lfbMode & SST_LFB_ENPIXPIPE))
		  cp->fbiData.lfbRenderMode|= SST_RM_RGB_WMASK | SST_RM_ALPHA_WMASK;
		break;	
	case SST_LFB_888:		// format 4
		data1 = _sstRgbaLanes8888(sst,data);
		cp->fbiData.lfbFBZmode |= SST_RGBWRMASK;

		if(!(sst->lfbMode & SST_LFB_ENPIXPIPE))
		  cp->fbiData.lfbRenderMode |= SST_RM_RGB_WMASK;
		words = 1;
		iaddr >>= 1;		// convert to 32-bit pixel address
		break;
	case SST_LFB_8888:		// format 5
		data1 = _sstRgbaLanes8888(sst,data);
		adata1 = (unsigned short)(data1>>24);	// get alpha
 		if ( sst->lfbMode & SST_LFB_ENPIXPIPE ) {
 		    cp->fbiData.lfbFBZmode |= SST_RGBWRMASK | SST_ZAWRMASK | 
 		    				 		    ( sst->fbzMode & SST_ENALPHABUFFER );
 		} else {
 		    if ( sst->fbzMode & SST_ENALPHABUFFER ) {
				cp->fbiData.lfbFBZmode |= SST_RGBWRMASK | SST_ZAWRMASK | SST_ENALPHABUFFER;
 		    } else /* Alpha buffer disabled */ {
				cp->fbiData.lfbFBZmode |= SST_RGBWRMASK;
 		    }
 		}
		
		if(!(sst->lfbMode & SST_LFB_ENPIXPIPE))
		  cp->fbiData.lfbRenderMode|= SST_RM_RGB_WMASK | SST_RM_ALPHA_WMASK;
		words = 1;
		iaddr >>= 1;		// convert to 32-bit pixel address
		break;
        case SST_LFB_Z32:
		data2 = sst->c1;	// color comes from C1 register
		data2 = (data2&0xFF00FF00) | ((data2>>16)&0xFF) | ((data2&0xFF)<<16);
		data1 = data2;		// have to convert from ARGB to ABGR
		zdata1 = data;
		words = 1;
		iaddr >>= 1;		// convert to 32-bit pixel address
		if ( sst->fbzMode & SST_ENALPHABUFFER ) {
			cp->fbiData.lfbFBZmode |= 0;
		} else {
			cp->fbiData.lfbFBZmode |= SST_ENDEPTHBUFFER | SST_ZAWRMASK;
		}
		break;
	case SST_LFB_Z565:		// format 12
		_sstRgba565to8888((unsigned char * )&data1,
					_sstRgbaLanes565(sst,data));
		ENDIAN_SWAP(data1);

		if((sst->renderMode & SST_RM_3D_MODE) == SST_RM_32BPP)
		  zdata1 = (data & 0xFFFF0000) | ((data>>16) & 0xFFFF);
		else
		  zdata1 = data & 0xFFFF0000;
//GDBG_INFO(132,"data,zdata1 = %x %x\n",data,zdata1);
		
		if ( sst->fbzMode & SST_ENALPHABUFFER ) {
			cp->fbiData.lfbFBZmode |= SST_RGBWRMASK;
		} else {
			cp->fbiData.lfbFBZmode |= SST_ENDEPTHBUFFER | SST_RGBWRMASK | SST_ZAWRMASK;
		}

		if(!(sst->lfbMode & SST_LFB_ENPIXPIPE))
		  cp->fbiData.lfbRenderMode |= SST_RM_RGB_WMASK;

		iaddr >>= 1;		// convert to 32-bit pixel address
		words = 1;
		break;
	case SST_LFB_Z555:		// format 13
		_sstRgba555to8888((unsigned char * )&data1,
					_sstRgbaLanes1555(sst,data));
		ENDIAN_SWAP(data1);

		if((sst->renderMode & SST_RM_3D_MODE) == SST_RM_32BPP)
		  zdata1 = (data & 0xFFFF0000) | ((data>>16) & 0xFFFF);
		else
		  zdata1 = data & 0xFFFF0000;
//GDBG_INFO(132,"data,zdata1 = %x %x\n",data,zdata1);

		if ( sst->fbzMode & SST_ENALPHABUFFER ) {
			cp->fbiData.lfbFBZmode |= SST_RGBWRMASK;
		} else {
			cp->fbiData.lfbFBZmode |= SST_ENDEPTHBUFFER | SST_RGBWRMASK | SST_ZAWRMASK;
		}

		if(!(sst->lfbMode & SST_LFB_ENPIXPIPE))
		  cp->fbiData.lfbRenderMode |= SST_RM_RGB_WMASK;

		iaddr >>= 1;		// convert to 32-bit pixel address
		words = 1;
		break;
	case SST_LFB_Z1555:		// format 14
		_sstRgba1555to8888((unsigned char * )&data1,
					_sstRgbaLanes1555(sst,data));
		ENDIAN_SWAP(data1);
		adata1 = (unsigned short)(data1>>24);

		if((sst->renderMode & SST_RM_3D_MODE) == SST_RM_32BPP)
		  zdata1 = (data & 0xFFFF0000) | ((data>>16) & 0xFFFF);
		else
		  zdata1 = data & 0xFFFF0000;

		// write both RGB planes and ZA planes
		cp->fbiData.lfbFBZmode |= SST_RGBWRMASK | SST_ZAWRMASK;
		// if alpha buffer enabled, write from alpha, else write from z
		if (sst->fbzMode & SST_ENALPHABUFFER)
		    cp->fbiData.lfbFBZmode |= SST_ENALPHABUFFER;
		else
		    cp->fbiData.lfbFBZmode |= SST_ENDEPTHBUFFER;

		if(!(sst->lfbMode & SST_LFB_ENPIXPIPE))
		  cp->fbiData.lfbRenderMode|= SST_RM_RGB_WMASK | SST_RM_ALPHA_WMASK;                  
		iaddr >>= 1;             // convert to 32-bit pixel address
		words = 1;
		break;
	case SST_LFB_ZZ:		// format 15
		data2 = sst->c1;	// color comes from C1 register
		data2 = (data2&0xFF00FF00) | ((data2>>16)&0xFF) | ((data2&0xFF)<<16);
		data1 = data2;		// have to convert from ARGB to ABGR

		if((sst->renderMode & SST_RM_3D_MODE) == SST_RM_32BPP)
		  zdata1 = ((data << 16) & 0xFFFF0000) | (data & 0xFFFF);
		else
		  zdata1 = (data << 16) & 0xFFFF0000;

		if((sst->renderMode & SST_RM_3D_MODE) == SST_RM_32BPP)
		  zdata2 = (data & 0xFFFF0000) | ((data>>16) & 0xFFFF);
		else
		  zdata2 = data & 0xFFFF0000;

		cp->fbiData.lfbFBZmode |= SST_ENDEPTHBUFFER | SST_ZAWRMASK;
 		if ( (sst->fbzMode & SST_ENALPHABUFFER) && !( sst->lfbMode & SST_LFB_ENPIXPIPE ) )
 		{
		    sst->stats.fbiPixelsOut += words;
		    sst->stats.fbiPixelsOut &= 0xFFFFFF;
 		    goto done;
 		}		
		break;
	default:
		GDBG_ERROR("sstFbiLfb","invalid LFB mode %d\n",
			(sst->lfbMode & SST_LFB_FORMAT)>>SST_LFB_FORMAT_SHIFT);
		break;
    }
    // words is now set to the number of pixels to write
    cp->fbiData.lfbRGBdata = data1;		// send the 1st pixel down
    cp->fbiData.lfbAdata = adata1;
    cp->fbiData.lfbZdata = zdata1;

    x = (iaddr & SST_LFB_ADDR_X) >> SST_LFB_ADDR_X_SHIFT;
    y = (iaddr & SST_LFB_ADDR_Y) >> SST_LFB_ADDR_Y_SHIFT;

    GDBG_INFO(132,"\t-LFB x,y = %d,%d\n",x,y);
    if (sst->lfbMode & SST_LFB_ENPIXPIPE)	// if pixel pipe is enabled then
	if (!sstRectClip(sst,x,y)) goto skip;	// first perform rectClip function

    if(sst->renderMode & SST_RM_DITHER_ROTATION)
      GDBG_ERROR("sstFastFill", "Can't use dither rotation with 3d lfb writes (%d)!\n", __LINE__);    

    currentRenderingChip = sst;
    sstFbiPixel(sst,x,y,FXTRUE);

    if( sst->aaCtrl & SST_AA_CONTROL_AA_ENABLE )
      {
	cp->environment.aaPrimaryBuffers = FXFALSE;
	sst->colBufferAddr = COL_BUFFER_ADDR_SECONDARY( sst );
	sst->auxBufferAddr = AUX_BUFFER_ADDR_SECONDARY( sst );
	sstFbiPixel(sst,x,y,FXTRUE);   // process the pixel
	sst->colBufferAddr = COL_BUFFER_ADDR_PRIMARY( sst );
	sst->auxBufferAddr = AUX_BUFFER_ADDR_PRIMARY( sst );		
	cp->environment.aaPrimaryBuffers = FXTRUE;
      }

skip:
    if (words < 2) goto done;			// only 1 pixel, all done

    cp->fbiData.lfbRGBdata = data2;		// send the 2nd pixel down
    cp->fbiData.lfbAdata = adata2;
    cp->fbiData.lfbZdata = zdata2;
    if (sst->lfbMode & SST_LFB_ENPIXPIPE)	// if pixel pipe is enabled then
	if (!sstRectClip(sst,x+1,y)) goto done;	// first perform rectClip function

    if(sst->renderMode & SST_RM_DITHER_ROTATION)
      GDBG_ERROR("sstFastFill", "Can't use dither rotation with 3d lfb writes (%d)!\n", __LINE__);    

    sstFbiPixel(sst,x+1,y,FXTRUE);

    if( sst->aaCtrl & SST_AA_CONTROL_AA_ENABLE )
      {
	cp->environment.aaPrimaryBuffers = FXFALSE;
	sst->colBufferAddr = COL_BUFFER_ADDR_SECONDARY( sst );
	sst->auxBufferAddr = AUX_BUFFER_ADDR_SECONDARY( sst );
	sstFbiPixel(sst,x+1,y,FXTRUE);   // process the pixel
	sst->colBufferAddr = COL_BUFFER_ADDR_PRIMARY( sst );
	sst->auxBufferAddr = AUX_BUFFER_ADDR_PRIMARY( sst );		
	cp->environment.aaPrimaryBuffers = FXTRUE;
      }

done:
    sst->fbzColorPath = saveCP;		// restore the mode
    sst->combineMode = saveCM;
}
