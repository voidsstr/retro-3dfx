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
** $Revision: 2$
** $Date: 10/11/00 8:08:36 PM$
*/

#include <assert.h>
#include <h3.h>
#include "h3sim.h"
#include "trexfunc.h"

//----------------------------------------------------------------------
// composite RGB using local and other colors, leave results in local
// composite 2 colors together, two routines: RGB and A
//	out: the "local" color, also the output color
//	 in: the "other" color
//----------------------------------------------------------------------
void sstCompositeRGB(SstRegs *sst, int isFbi,
		     unsigned char local[4], unsigned char other[4],
		     unsigned char iterators[4], unsigned char mselect_7[3],
		     unsigned char localTexture[4], 
		     unsigned char otherTexture[4],
		     unsigned long mode, unsigned long reverse)
{
    int i, r,g,b, beta;
    int rLocalTemp, gLocalTemp, bLocalTemp;
    int rOtherTemp, gOtherTemp, bOtherTemp;
    int dr, dg, db;

    if (mode & SST_TC_ZERO_OTHER) 
      {		// zero out the "other" color	
	rOtherTemp = 0;
	gOtherTemp = 0;
	bOtherTemp = 0;
      }
    else 
      {
	rOtherTemp = other[0];
	gOtherTemp = other[1];
	bOtherTemp = other[2];
      }
    if (mode & SST_TC_SUB_CLOCAL) {	
	rLocalTemp = local[0];
	gLocalTemp = local[1];
	bLocalTemp = local[2];
    }
    else {
	rLocalTemp = 0;
	gLocalTemp = 0;
	bLocalTemp = 0;
    }
    
    switch(sst->combineMode & SST_CM_TC_INVERT_OTHER)
      {
      case SST_CM_TC_INVERT_OTHER_X:
	//Don't do anything
	break;
      case SST_CM_TC_INVERT_OTHER_ZERO_MINUS_X:
	rOtherTemp = -rOtherTemp;
	gOtherTemp = -gOtherTemp;
	bOtherTemp = -bOtherTemp;
	break;
      case SST_CM_TC_INVERT_OTHER_ONE_MINUS_X:
	rOtherTemp = 0xFF-rOtherTemp;
	gOtherTemp = 0xFF-gOtherTemp;
	bOtherTemp = 0xFF-bOtherTemp;	    
	break;
      case SST_CM_TC_INVERT_OTHER_X_MINUS_HALF:
	rOtherTemp = rOtherTemp-0x80;
	gOtherTemp = gOtherTemp-0x80;
	bOtherTemp = bOtherTemp-0x80;	    
	break;
      }
    
    switch(sst->combineMode & SST_CM_TC_INVERT_LOCAL)
      {
      case SST_CM_TC_INVERT_LOCAL_X:
	//Don't do anything
	break;
      case SST_CM_TC_INVERT_LOCAL_ZERO_MINUS_X:
	rLocalTemp = -rLocalTemp;
	gLocalTemp = -gLocalTemp;
	bLocalTemp = -bLocalTemp;
	break;
      case SST_CM_TC_INVERT_LOCAL_ONE_MINUS_X:
	rLocalTemp = 0xFF-rLocalTemp;
	gLocalTemp = 0xFF-gLocalTemp;
	bLocalTemp = 0xFF-bLocalTemp;	    
	break;
      case SST_CM_TC_INVERT_LOCAL_X_MINUS_HALF:
	rLocalTemp = rLocalTemp-0x80;
	gLocalTemp = gLocalTemp-0x80;
	bLocalTemp = bLocalTemp-0x80;	    
	break;
      }

    r = rOtherTemp + rLocalTemp;
    g = gOtherTemp + gLocalTemp;
    b = bOtherTemp + bLocalTemp;
    
    i = 1;		// default increment
    switch (mode & SST_TC_MSELECT) {
	case SST_TC_MONE:
	case SST_TC_MLODFRAC+SST_TC_MCLOCAL:	// 6
	    beta = 0;
	    goto mbeta;
	case SST_TC_MCLOCAL:
	    r *= (local[0] ^ reverse)+1;
	    g *= (local[1] ^ reverse)+1;
	    b *= (local[2] ^ reverse)+1;
	    break;
	case SST_TC_MAOTHER:
	  if(isFbi)
	    {
	      beta = other[3];
	    }
	  else
	    {
	      beta = otherTexture[3];
	    }
	    goto mbeta;
	case SST_TC_MALOCAL:
	  if(isFbi)
	    {
	      beta = local[3];
	    }
	  else
	    {
	      beta = localTexture[3];
	    }
	  goto mbeta;	      
	case SST_TC_MLOD:
	    if (isFbi) {		        // SST_CC_MATREX
		beta = CSIM_PRIVATE(sst)->fbiData.trexIn[3];

	    if(!(sst->fbzColorPath & SST_ENTEXTUREMAP))
	      GDBG_ERROR("sstCompositeRGB", "Damn! FBI can't use texture data with texturing disabled %s(%d)\n",
			 __FILE__, __LINE__);

		goto mbeta;
	    }
	    beta = TMU_PRIVATE(sst)->detail_lod;// .8 format
	    i = (sst->tDetail & SST_DETAIL_BIAS)>>SST_DETAIL_BIAS_SHIFT;
	    i = SIGN_EXTEND(i,6);
	    beta = (i<<8) + ~beta;		// add one's complement
	    if (beta < 0) beta = 0;		// detect underflow
	    i = (sst->tDetail & SST_DETAIL_SCALE)>>SST_DETAIL_SCALE_SHIFT;
	    beta <<= i;
	    beta >>= 8;				// 8.0 format
	    i = (sst->tDetail & SST_DETAIL_MAX)>>SST_DETAIL_MAX_SHIFT;
	    if (beta > i || TMU_PRIVATE(sst)->st.hw_w_is_neg) beta = i;	// clamp (also overflow)
	    i = 1;
		//GDBG_INFO(176,"MLOD = %d + %d\n",beta,i);
	    goto mbeta;
	case SST_TC_MLODFRAC:
	    if (isFbi) {			// SST_CC_MRGBTMU
		int rbeta, gbeta, bbeta;
		rbeta = CSIM_PRIVATE(sst)->fbiData.trexIn[0];
		gbeta = CSIM_PRIVATE(sst)->fbiData.trexIn[1];
		bbeta = CSIM_PRIVATE(sst)->fbiData.trexIn[2];
		if (reverse) {
	    	    rbeta ^= reverse;
	    	    gbeta ^= reverse;
	    	    bbeta ^= reverse;
		}

		if(!(sst->fbzColorPath & SST_ENTEXTUREMAP))
		  GDBG_ERROR("sstCompositeRGB", "Damn! FBI can't use texture data with texturing disabled %s(%d)\n",
			     __FILE__, __LINE__);

		//GDBG_INFO(176,"MRGBTMU = %d %d %d\n",rbeta,gbeta,bbeta);
		r *= rbeta+1;
		g *= gbeta+1;
		b *= bbeta+1;
		break;
	    }
	    beta = TMU_PRIVATE(sst)->lod & 0xFF;
	    if (!reverse) i = 0;		// DON'T INCREMENT		
		//GDBG_INFO(176,"MLODFRAC = %d+%d rev=%x grn oth=%d loc=%d\n",beta,i,reverse,other[1],local[1]);
	    goto mbeta;

        case SST_TC_MCMSELECT7:
	  r *= (mselect_7[0] ^ reverse) + 1;
	  g *= (mselect_7[1] ^ reverse) + 1;
	  b *= (mselect_7[2] ^ reverse) + 1;
	  break;  //Don't do beta multiplication. We're done already.

    mbeta:
	    beta ^= reverse;
	    beta += i;
	    r *= beta;
	    g *= beta;
	    b *= beta;
	    break;
	default:
	   GDBG_ERROR("sstTrexColor","invalid TC combine mode 0x%x\n",
		(mode & SST_TCOMBINE)>>SST_TCOMBINE_SHIFT);
    }

    r >>= 8;				// return to 8.0 format
    g >>= 8;
    b >>= 8;


    switch((((mode & SST_TC_ADD_ALOCAL)!=0) << 0) |
	   (((mode & SST_TC_ADD_CLOCAL)!=0) << 1))
      {
      case 0:  //Add 0 to result of multiplication
	dr = 0;
	dg = 0;
	db = 0;
	break;
      case 1:  
	if(isFbi)
	  {  //Add Local alpha      
	    dr = local[3];
	    dg = local[3];
	    db = local[3];
	  }
	else
	  {  //TMU, add local texture alpha
	    dr = localTexture[3];
	    dg = localTexture[3];
	    db = localTexture[3];	    
	  }	
	break;
      case 2:  //Add local RGB
	dr = local[0];
	dg = local[1];
	db = local[2];
	break;
      case 3:  //Add texture RGB
	if(isFbi)  //Add texture RGB
	  {
	    CsimPrivate *cp = CSIM_PRIVATE(sst);

	    if(!(sst->fbzColorPath & SST_ENTEXTUREMAP))
	      GDBG_ERROR("sstCompositeRGB", "Damn! FBI can't use texture data with texturing disabled %s(%d)\n",
			 __FILE__, __LINE__);

	    dr = cp->fbiData.trexIn[0];
	    dg = cp->fbiData.trexIn[1];
	    db = cp->fbiData.trexIn[2];	    
	  }
	else       //(TMU case) Add Iterated RGB
	  {
	    dr = iterators[0];
	    dg = iterators[1];
	    db = iterators[2];
	  }
	break;
      }

    if(sst->combineMode & SST_CM_TC_INVERT_ADD_LOCAL)
      { 
	dr=(~dr) & 0xFF;
	dg=(~dg) & 0xFF;
	db=(~db) & 0xFF;
      }
    
    //Bottom Adder in CCU
    r+=dr;
    g+=dg;
    b+=db;
	      

    //Modulate 1x, 2x, 4x    

    switch(sst->combineMode & SST_CM_TC_OUTSHIFT)
      {
      case SST_CM_TC_OUTSHIFT_1X:
	// Do nothing
	break;
      case SST_CM_TC_OUTSHIFT_2X:
	r=r<<1;
	g=g<<1;
	b=b<<1;
	break;
      case SST_CM_TC_OUTSHIFT_4X:
	r=r<<2;
	g=g<<2;
	b=b<<2;
	break;
      default:
	assert("Illegal case" && 0);
      }

    if (r < 0) r = 0;
    if (g < 0) g = 0;
    if (b < 0) b = 0;
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    if (mode & SST_TC_INVERT_OUTPUT) {
	r ^= 0xFF;
	g ^= 0xFF;
	b ^= 0xFF;
    }
    
    local[0] = r;			// leave results in local
    local[1] = g;
    local[2] = b;
}

//----------------------------------------------------------------------
// composite Alpha using local and other colors, leave results in local
//----------------------------------------------------------------------
void sstCompositeA(SstRegs *sst, int isFbi,
		   unsigned char local[4], unsigned char other[4],
		   unsigned char iterators[4], 
		   unsigned char localTextureAlpha,
		   unsigned char otherTextureAlpha,
		   unsigned long mode, unsigned long reverse)
{
    int i,a, beta;
    int aLocalTemp;
    int aOtherTemp;
    int da;

    aOtherTemp = mode & SST_TCA_ZERO_OTHER ? 0 : other[3];
    aLocalTemp = mode & SST_TCA_SUB_CLOCAL ? local[3] : 0;

    switch(sst->combineMode & SST_CM_TCA_INVERT_OTHER)
      {
      case SST_CM_TCA_INVERT_OTHER_X:
	//Don't do anything
	break;
      case SST_CM_TCA_INVERT_OTHER_ZERO_MINUS_X:
	aOtherTemp = -aOtherTemp;
	break;
      case SST_CM_TCA_INVERT_OTHER_ONE_MINUS_X:
	aOtherTemp = 0xFF-aOtherTemp;
	break;
      case SST_CM_TCA_INVERT_OTHER_X_MINUS_HALF:
	aOtherTemp = aOtherTemp-0x80;
	break;
      }
    
    switch(sst->combineMode & SST_CM_TCA_INVERT_LOCAL)
      {
      case SST_CM_TCA_INVERT_LOCAL_X:
	//Don't do anything
	break;
      case SST_CM_TCA_INVERT_LOCAL_ZERO_MINUS_X:
	aLocalTemp = -aLocalTemp;
	break;
      case SST_CM_TCA_INVERT_LOCAL_ONE_MINUS_X:
	aLocalTemp = 0xFF-aLocalTemp;
	break;
      case SST_CM_TCA_INVERT_LOCAL_X_MINUS_HALF:
	aLocalTemp = aLocalTemp-0x80;
	break;
      }
    
    a = aOtherTemp + aLocalTemp;

    i = 1;
    switch (mode & SST_TCA_MSELECT) {
	case SST_TCA_MONE:
	    beta = 0;
	    goto mbeta;
	case SST_TCA_MCLOCAL:
	    a *= (local[3] ^ reverse)+1;
	    break;
	case SST_TCA_MAOTHER:
	  if(isFbi)
	    {
	      beta = other[3];
	    }
	  else
	    {
	      beta = otherTextureAlpha;
	    }
	    goto mbeta;
	case SST_TCA_MALOCAL:
	  if(isFbi)
	    {
	      beta = local[3];
	    }
	  else
	    {
	      beta = localTextureAlpha;
	    }
	    goto mbeta;
	case SST_TCA_MLOD:
	    if (isFbi) {		// SST_CCA_MATREX
		beta = CSIM_PRIVATE(sst)->fbiData.trexIn[3];

		if(!(sst->fbzColorPath & SST_ENTEXTUREMAP))
		  GDBG_ERROR("sstCompositeRGB", "Damn! FBI can't use texture data with texturing disabled %s(%d)\n",
			     __FILE__, __LINE__);

		goto mbeta;
	    }
	    beta = TMU_PRIVATE(sst)->detail_lod;// .8 format
	    i = (sst->tDetail & SST_DETAIL_BIAS)>>SST_DETAIL_BIAS_SHIFT;
	    i = SIGN_EXTEND(i,6);
	    beta = (i<<8) + ~beta;		// add one's complement
	    if (beta < 0) beta = 0;		// detect underflow
	    i = (sst->tDetail & SST_DETAIL_SCALE)>>SST_DETAIL_SCALE_SHIFT;
	    beta <<= i;
	    beta >>= 8;				// 8.0 format
	    i = (sst->tDetail & SST_DETAIL_MAX)>>SST_DETAIL_MAX_SHIFT;
	    if (beta > i || TMU_PRIVATE(sst)->st.hw_w_is_neg) beta = i;	// clamp (also overflow)
	    i = 1;
            //GDBG_INFO(176,"MLOD = %d + %d\n",beta,i);
	    goto mbeta;
	case SST_TCA_MLODFRAC:
	  if (isFbi) //This is SST_CCA_MAITER
	    {		
	      beta = iterators[3];
	    }
	  else
	    {
	      beta = TMU_PRIVATE(sst)->lod & 0xFF;
	      if (!reverse) i = 0;		// DON'T INCREMENT		
	      //GDBG_INFO(176,"MLODF = %d + %d\n",beta,i);
	    }
	  goto mbeta;
	  break;
        case SST_TCA_MAITER: //Iterator Alpha
	  if (isFbi) //This is SST_CCA_MAC1 which is Color 1 Alpha
	    {
	      beta = (unsigned char)((sst->c1 >> 24) & 0xFF);
	    }
	  else
	    {
	      beta = iterators[3];
	    }

	  goto mbeta;
	  break;
        case SST_TCA_MCR:  //Chroma Range Alpha
	  if (isFbi) //The FBI just passes 0 in the case
	    {
	      beta = 0;
	    }
	  else
	    {
	      beta = (unsigned char)((sst->chromaRange >> 24) & 0xFF);
	    }

	  goto mbeta;
	  break;

    mbeta:
	    beta ^= reverse;
	    beta += i;
//GDBG_INFO(176,"%02x = a(%02x) * beta(%02x)\n",(a * beta)>>8, a,beta);
	    a *= beta;
	    break;
	default:
	   GDBG_ERROR("sstCompositeA","invalid TCA combine mode 0x%x\n",
		(mode & SST_TACOMBINE)>>SST_TACOMBINE_SHIFT);
    }

    a >>= 8;
    switch((((mode & SST_TCA_ADD_ALOCAL)!=0) << 0) |
	   (((mode & SST_TCA_ADD_CLOCAL)!=0) << 1))
      {
      case 0:  //Add 0 to result of multiplication
	da = 0;
	break;
      case 1:
	if(isFbi)
	  {   //Add Local alpha    
	    da = local[3];
	  }
	else
	  {  //TMU case, add local texture alpha
	    da = localTextureAlpha;
	  }	
	break;
      case 2:  //Add Local alpha
	da = local[3];
	break;
      case 3:  
	if(isFbi)  //Add texture Alpha
	  {
	    CsimPrivate *cp = CSIM_PRIVATE(sst);

	    if(!(sst->fbzColorPath & SST_ENTEXTUREMAP))
	      GDBG_ERROR("sstCompositeRGB", "Damn! FBI can't use texture data with texturing disabled %s(%d)\n",
			 __FILE__, __LINE__);

	    da = cp->fbiData.trexIn[3];
	  }
	else       //(TMU case) Add Iterated Alpha
	  {	
	    da = iterators[3];
	  }
	break;
      }

    if(sst->combineMode & SST_CM_TCA_INVERT_ADD_LOCAL)
      da=(~da) & 0xFF;
    
    //Bottom Adder in Alpha combine unit
    a+=da;
	      
    //Modulate 1x, 2x, 4x    
    switch(sst->combineMode & SST_CM_TCA_OUTSHIFT)
      {
      case SST_CM_TCA_OUTSHIFT_1X:
	// Do nothing
	break;
      case SST_CM_TCA_OUTSHIFT_2X:
	a=a<<1;
	break;
      case SST_CM_TCA_OUTSHIFT_4X:
	a=a<<2;
	break;
      default:
	assert("Illegal case" && 0);
      }

    if (a < 0) a = 0;
    if (a > 255) a = 255;
    if (mode & SST_TCA_INVERT_OUTPUT)
      a ^= 0xFF;

    local[3] = a;
}
