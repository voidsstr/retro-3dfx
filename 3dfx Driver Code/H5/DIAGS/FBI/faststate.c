#include <assert.h>

#include "udiag.h"
#include "sstdiag.h"

void randomCombine(SstRegs *sst);

int main(int argc, char **argv)
{
  SstRegs *sst;
  static Triangle t;
  FxI32 trianglePasses;
  FxI32 trianglesToDraw;

  sst = SST_BEGIN(argc, argv);
  SET(sst->fbzMode, SST_RGBWRMASK);
  SET(sst->fbzColorPath, SST_RGBSEL_RGBA);


  if(diago.tsize < 0)
    diago.tsize = 10;

  while (DIAG_STARTPASS())
    {
      trianglePasses = 100;

      while(trianglePasses-- > 0)
	{
	  trianglesToDraw = iRandom(16) + 1;

	  while(trianglesToDraw-- > 0)
	    {
	      if(iRandom(1))
		randomCombine(sst);

	      randomTriangle(&t, iRandom(diago.tsize), 1);
	      randomRgbaTriangle(&t);
	      areaTriangle(&t);
	      setupTriangle(&t, 1, 0, 0);
	      sortTriangle(&t);
	      printTriangle(2, &t, 1, 0, 0);
	      printTriangleSlopes(3, &t, 1, 0, 0);
	      GDBG_INFO(3, "    Triangle Area = %d\n", t.area);
	      drawTriangle(sst, &t, 1, 0, 0);	      
	    }

	  SET(sst->c0, iRandom(0xFFFFFFFF));
	  SET(sst->c1, iRandom(0xFFFFFFFF));	  
	  SET(sst->nopCMD, 0);
	}
    }
  DIAG_PASS(0);

  return(0);
}

void randomCombine(SstRegs *sst)
{
  FxU32 fbzColorPath=0;
  FxU32 combineMode=0;

  switch(iRandom(1))
    {
    case 0:
      fbzColorPath |= SST_RGBSEL_RGBA;
      break;
    case 1:
      fbzColorPath |= SST_RGBSEL_C1;
      break;
    default:
      assert(0);	
    }
  
  switch(iRandom(1))
    {
    case 0:
      fbzColorPath |= SST_ASEL_RGBA;
      break;
    case 1:
      fbzColorPath |= SST_ASEL_C1;
      break;
    default:
      assert(0);	
    }

  switch(iRandom(5))
    {
    case 0:
      fbzColorPath |= SST_CC_MONE;
      break;
    case 1:
      fbzColorPath |= SST_CC_MCLOCAL;
      break;
    case 2:
      fbzColorPath |= SST_CC_MAOTHER;
      break;
    case 3:
      fbzColorPath |= SST_CC_MALOCAL;      
      break;
    case 4:
      fbzColorPath |= SST_CC_MONE6;
      break;
    case 5:
      fbzColorPath |= SST_CC_MCMSELECT7;
      break;
    default:
      assert(0);	
    }

  switch(iRandom(2))
    {
    case 0:
      fbzColorPath |= SST_CC_ADD_CLOCAL;
      break;
    case 1:
      fbzColorPath |= SST_CC_ADD_ALOCAL;
      break;
    case 2:
      fbzColorPath |= 0;
      break;
    default:
      assert(0);	
    }

  fbzColorPath |= iRandom(0xFFFFFFFF) & (SST_LOCALSELECT | SST_ALOCALSELECT |
					 SST_CC_ZERO_OTHER | SST_CC_SUB_CLOCAL |
					 SST_CC_REVERSE_BLEND | SST_CCA_INVERT_OUTPUT |
					 SST_PARMADJUST | SST_RGBAZ_CLAMP);
  SET(sst->fbzColorPath, fbzColorPath);



  switch(iRandom(4))
    {
    case 0:
      combineMode |= SST_CM_CC_OTHERSELECT_IRGB;
      break;
    case 1:
      combineMode |= SST_CM_CC_OTHERSELECT_C1_RGB;
      break;
    case 2:
      combineMode |= SST_CM_CC_OTHERSELECT_IA;
      break;
    case 3:
      combineMode |= SST_CM_CC_OTHERSELECT_C1_A;
      break;
    case 4:
      combineMode |= SST_CM_CC_OTHERSELECT_ZERO;
      break;
    default:
      assert(0);	
    }

  switch(iRandom(4))
    {
    case 0:
      combineMode |= SST_CM_CC_LOCALSELECT_IRGB;
      break;
    case 1:
      combineMode |= SST_CM_CC_LOCALSELECT_C0_RGB;
      break;
    case 2:
      combineMode |= SST_CM_CC_LOCALSELECT_IA;
      break;
    case 3:
      combineMode |= SST_CM_CC_LOCALSELECT_C0_A;
      break;
    case 4:
      combineMode |= SST_CM_CC_LOCALSELECT_ZERO;
      break;
    default:
      assert(0);	
    }

  switch(iRandom(3))
    {
    case 0:
      combineMode |= SST_CM_CC_MSELECT_7_IRGB;
      break;
    case 1:
      combineMode |= SST_CM_CC_MSELECT_7_C1_RGB;
      break;
    case 2:
      combineMode |= SST_CM_CC_MSELECT_7_IA;
      break;
    case 3:
      combineMode |= SST_CM_CC_MSELECT_7_C1_A;
      break;
    default:
      assert(0);	
    }

  switch(iRandom(1))
    {
    case 0:
      combineMode |= SST_CM_CCA_OTHERSELECT_IA;
      break;
    case 1:
      combineMode |= SST_CM_CCA_OTHERSELECT_C1_A;
      break;
    default:
      assert(0);	
    }

  switch(iRandom(4))
    {
    case 0:
      combineMode |= SST_CM_CCA_LOCALSELECT_IA;
      break;
    case 1:
      combineMode |= SST_CM_CCA_LOCALSELECT_C0_A;
      break;
    case 2:
      combineMode |= SST_CM_CCA_LOCALSELECT_IZ;
      break;
    case 3:
      combineMode |= SST_CM_CCA_LOCALSELECT_IW;
      break;
    case 4:
      combineMode |= SST_CM_CCA_LOCALSELECT_ZERO;
      break;
    default:
      assert(0);	
    }

  combineMode |= iRandom(2) & SST_CM_CC_OUTSHIFT;
  combineMode |= iRandom(2) & SST_CM_CCA_OUTSHIFT;

  combineMode |= iRandom(0xFFFFFFFF) & (SST_CM_CC_INVERT_OTHER | SST_CM_CC_INVERT_LOCAL |
					SST_CM_CC_INVERT_ADD_LOCAL | 
					SST_CM_CCA_INVERT_OTHER | SST_CM_CCA_INVERT_LOCAL |
					SST_CM_CCA_INVERT_ADD_LOCAL |
					SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK | SST_CM_USE_COMBINE_MODE);
					
  SET(sst->combineMode, combineMode);					
}
