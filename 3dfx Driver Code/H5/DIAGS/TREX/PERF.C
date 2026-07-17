/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 8:19:13 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

#define NUM_TRIS_PER_STRIP 10
#define NUM_STRIPS 20

enum TSUMODES { TEST_XY, TEST_XY_RGB, TEST_XY_RGBW,
		TEST_XY_STW, TEST_XY_RGBSTW,
		TEST_XY_RGBSTWZ, TEST_XY_RGBASTWZ };
		
#define M_PI 3.14159265358979323846
#define DEG_TO_RAD(a) ((a) * M_PI / 180.0)

int angle = 0;
float ratio = 1.0F;
float ratio1 = 1.0F;
int packedARGB;
FxU32 smode;

void sendV(SstRegs *sst, Vertex *v)
{
    float fx = v->fx, fy = v->fy;

    SETF(sst->sVx,fx);
    SETF(sst->sVy,fy);
    gdbg_info(3,"Vertex: %g %g  (unsnapped) \n",fx,fy);
    if ((smode & (SST_SETUP_RGB|SST_SETUP_A)) && packedARGB) {
	FxU32 argb;
	argb  = ((int)v->fa)<<24;
	argb |= ((int)v->fr)<<16;
	argb |= ((int)v->fg)<<8;
	argb |= ((int)v->fb)<<0;
	SET(sst->sARGB,argb);
    }
    else {
	if (smode & SST_SETUP_RGB) {
	    SETF(sst->sRed,v->fr);
	    SETF(sst->sGreen,v->fg);
	    SETF(sst->sBlue,v->fb);
	}
	if (smode & SST_SETUP_A) {
	    SETF(sst->sAlpha,v->fa);
	}
    }

    if (smode & SST_SETUP_Z) {
	SETF(sst->sVz,v->fz);
    }
    if (smode & SST_SETUP_Wfbi) {
	SETF(sst->sOowfbi,v->fw);
    }
    if (smode & SST_SETUP_ST0) {
	SETF(sst->sSow0,v->fs);
	SETF(sst->sTow0,v->ft);
    }
    if (smode & SST_SETUP_W0) {
	SETF(sst->sOow0,v->fw);
    }
    if (smode & SST_SETUP_ST1) {
	SETF(sst->sSow1,v->fs1);
	SETF(sst->sTow1,v->ft1);
    }
    if (smode & SST_SETUP_W1) {
	SETF(sst->sOow1,v->fw1);
    }
    
}

Xusage(void)
{
	gdbg_printf( "perf option description:\n"  );
	gdbg_printf( "-b -> draw independent tris\n" );
	gdbg_printf( "-f -> disable mipmapping, force 1:1 ratio\n" );
	gdbg_printf( "-m -> change LOD bias from 0.0 to 0.5\n" );
	gdbg_printf( "-M -> enable LOD dithering\n" );
	gdbg_printf( "-u -> number of triangles in the strip\n" );
	gdbg_printf( "-x""a #"" -> angle in degress of strip\n" );
	gdbg_printf( "-x""r #"" -> ratio of texels to pixels (TMU0)\n" );
	gdbg_printf( "-x""R #"" -> ratio of texels to pixels (TMU1)\n" );
	gdbg_printf( "-O  -> setup parameters listed below\n" );
	gdbg_printf( " %d -> XY triangles\n", TEST_XY );
	gdbg_printf( " %d -> float RGB\n", TEST_XY_RGB );
	gdbg_printf( " %d -> float RGB+W\n", TEST_XY_RGBW );
	gdbg_printf( " %d -> float STW\n", TEST_XY_STW );
	gdbg_printf( " %d -> float RGBSTW\n", TEST_XY_RGBSTW );
	gdbg_printf( " %d -> float RGBSTWZ\n", TEST_XY_RGBSTWZ );
	gdbg_printf( " %d -> float RGBASTWZ\n", TEST_XY_RGBASTWZ );
	exit( 0 );
	return 0;
}

/* myParseOpts
 *
 * look for "-x" options and interpret them for this test
 *
 */

#define XGETARG() opts[1] ? done = 1, ++opts : \
			  (--argc > 0) ? done = 1, *++argv : \
					 (char *)Xusage()

void
XParseOpts(int argc, char **argv)
{
    char *opts = 0;
    FxBool done;
    
    while ((--argc > 0) && (**++argv))
    {
	if (argv[0][0] != '-')
	    continue;
	if (argv[0][1] != 'x')
	    continue;

	/* now parse all extended parameters */
	done = 0;
	opts = &argv[0][2];
	if (*opts == '\0')
	    Xusage();
	
	while (!done && *opts)
	{
	    switch (*opts)
	    {
	      case 'a':
		  sscanf(XGETARG(), "%i", &angle);
		  break;
	      case 'r':
		  sscanf(XGETARG(), "%f", &ratio);
		  break;
	      case 'R':
		  sscanf(XGETARG(), "%f", &ratio1);
		  break;
	      default:
		  Xusage();
	    }
	    
	    opts += 1;
	}
    }
}

void main (int argc, char **argv)
{
    int n,startTime,endTime,strips=20;
    FxU32 fbzCP, csrc, auxbits;
    SstRegs *sst;
    Triangle *t;
    int bilinear;
    int bilinear1;
    int mipmapping;

    //This diag behaves randomly switches between texturing, single-texturing, 
    //and multi-texturing in an unfriendly way. Consequently, 2 ppc mode
    //should not be run.
    inhibitTwoPixelsPerClock();   

    // Parse the Command Line and Initialize the Simulator
    sst = SST_BEGIN( argc, argv );
    XParseOpts( argc, argv );
    FXUNUSED(startTime);
    FXUNUSED(endTime);

    if(diago.bigAssTextures)
      t = buildTriangle(2048, 2048);
    else
      t = buildTriangle(256, 256);

    // Handle Options
    if ( diago.printOpts )
	Xusage();
    if (angle < 0 || angle > 90) {
	GDBG_ERROR("perf","invalid angle of %d, must be in [0,90] range\n",angle);
	DIAG_FAIL();
    }

    packedARGB = 0;
    switch (diago.option) {
    case TEST_XY:
      smode = 0;
      break;
    case TEST_XY_RGB:
      smode = SST_SETUP_RGB;
      break;
    case TEST_XY_RGBW:
      smode = SST_SETUP_RGB | SST_SETUP_Wfbi;
      break;
    case TEST_XY_STW:
      smode = SST_SETUP_ST0 | SST_SETUP_Wfbi;
      if ( diago.trex ) smode |= SST_SETUP_ST1;
      break;
    case TEST_XY_RGBSTW:
      smode = SST_SETUP_RGB | SST_SETUP_ST0 | SST_SETUP_Wfbi;
      if ( diago.trex ) smode |= SST_SETUP_ST1;
      break;
    case TEST_XY_RGBSTWZ:
      smode = SST_SETUP_RGB | SST_SETUP_ST0 | SST_SETUP_Wfbi | SST_SETUP_Z;
      if ( diago.trex ) smode |= SST_SETUP_ST1;
      break;
    case TEST_XY_RGBASTWZ:
    default:
      smode = SST_SETUP_RGB | SST_SETUP_ST0 | SST_SETUP_Wfbi |
      	SST_SETUP_A | SST_SETUP_Z;
      if ( diago.trex ) smode |= SST_SETUP_ST1;
      break;
    }
    
    // initialize memory contents
    csimVideo(CSIM_PRIVATE(diago.sstCSIM),FXFALSE);	// disable video
    DIAG_FORCE_RECT(diago.curdrawbuffer,0,0,diago.xmaxscreen,diago.ymaxscreen,0x80);
    DIAG_FORCE_RECT(CSIM_BUF_3D_AUX1,0,0,diago.xmaxscreen,diago.ymaxscreen,0xDEAD);
    csimVideo(CSIM_PRIVATE(diago.sstCSIM),FXTRUE);	// enable video
    
    // set up textures
    mipmapping = !diago.flip;
    bilinear = ratio <= 2.0 || mipmapping;
    bilinear1 = ratio1 <= 2.0 || mipmapping;
    if (mipmapping)
      GDBG_INFO(2,"TMU 0: mipmapping enabled, %s\n",
		bilinear ? "bilinear" : "point-sampled");
    else
      GDBG_INFO(2,"TMU 0: no mipmapping, %.4f texels to pixels, %s\n",
		ratio, bilinear ? "bilinear" : "point-sampled");
    if ( diago.trex ) {
      if (mipmapping)
	GDBG_INFO(2,"TMU 1: mipmapping enabled, %s\n",
		  bilinear1 ? "bilinear" : "point-sampled");
      else
	GDBG_INFO(2,"TMU 1: no mipmapping, %.4f texels to pixels, %s\n",
		  ratio1, bilinear1 ? "bilinear" : "point-sampled");
    }

    // initialize all TMUs to render from a 1x1 texture
    for ( n=0; n<MAX_NUM_TMUS; n++ ) {
      GDBG_INFO(0,"perf: setting defaults for TMU %d\n",n);
      SET(SST_TREX(sst,n)->textureMode,SST_RGB332 | SST_TC_PASS | SST_TCA_PASS);
      SET(SST_TREX(sst,n)->tLOD, SST_TLOD_MINMAX_INT(8,8));
      SET(SST_TREX(sst,n)->texBaseAddr, diago.minTrashMem); 
    }

    while (DIAG_STARTPASS()) {			// for each pass
	int i,istop,itris;
	int size = diago.tsize;
	float rotsin = (float)sin(DEG_TO_RAD(angle));	// sin and cos of rotation
	float rotcos = (float)cos(DEG_TO_RAD(angle));

	if (size < 0) size = -size;
	istop = diago.dstFormat;
	if (istop <= 0) istop = NUM_TRIS_PER_STRIP;

	if (diago.tex8) // use the 8-bit 332 RGB texture format, size = 256x256
	  t->tex->tMode = SST_RGB332;
	else            // use the 16-bit 565 RGB texture format, size = 256x256
	  t->tex->tMode = SST_RGB565;
	
	if (bilinear) t->tex->tMode |= SST_TMINFILTER | SST_TMAGFILTER;
	if (mipmapping)
	  t->tex->tMode |= SST_TPERSP_ST;
	if (diago.loddither)			// enable LOD dithering
	  t->tex->tMode |= SST_TLODDITHER;

	if ( diago.trex ) {
	  FxU32 startAddr = diago.texMemStart;
	  FxU32 endAddr = diago.maxTrashMem - 1;
	  int tMode = t->tex->tMode;
	  // TMU1
	  t->tex->tMode = tMode;
	  t->tex->tMode |= SST_TC_REPLACE | SST_TCA_REPLACE;
	  texRandomTextureMapNoOverlap(sst,1,mipmapping,8,8,t->tex,&startAddr,&endAddr);
	  // TMU0
	  t->tex->tMode = tMode;
	  t->tex->tMode |= SST_TC_MULT | SST_TCA_MULT;
	  texRandomTextureMapNoOverlap(sst,0,mipmapping,8,8,t->tex,&startAddr,&endAddr);
	} else {
	  t->tex->tMode |= SST_TC_REPLACE | SST_TCA_REPLACE;
	  texRandomTextureMap(sst,diago.trex,mipmapping,8,8,t->tex);
	}

	// set LODbias to 1/2
	if (diago.lodbias) {
	  t->tex->tLOD &= ~SST_LODBIAS;
	  t->tex->tLOD |= (1<<(SST_LOD_FRACBITS-1)) << SST_LODBIAS_SHIFT;
	  if ( diago.trex ) {
	    SET(SST_TREX(sst,0)->tLOD,t->tex->tLOD);
	    SET(SST_TREX(sst,1)->tLOD,t->tex->tLOD);
	  } else {
	    SET(SST_TREX(sst,diago.trex)->tLOD,t->tex->tLOD);
	  }
	}
	
	randomFloatRgbaTriangle(t);
	t->vA.fz = fexpRandom(8,SST_Z_INTBITS);
	t->vB.fz = fexpRandom(8,SST_Z_INTBITS);
	t->vC.fz = fexpRandom(8,SST_Z_INTBITS);

	// Initialize for Simple Drawing Based on C0
	fbzCP = diago.adjust ? SST_PARMADJUST : 0;
	if (!(smode & SST_SETUP_RGB))
	    fbzCP |= SST_LOCALSELECT;

	// need to decide which to check A,Z,W
	if (smode & SST_SETUP_Wfbi)
	    auxbits = SST_WBUFFER | SST_ZAWRMASK | SST_ENDEPTHBUFFER | 
			SST_ZFUNC_LT | SST_ZFUNC_GT;
	else
	    auxbits = 0;

	if (diago.xmaxscreen - size*(istop+1)/2 < 10) {
	    GDBG_ERROR("main","triangle strip is too large, extends off X screen\n");
	    DIAG_FAIL();
	}
	if (diago.ymaxscreen - size*rotcos-size*(istop+1)/2*rotsin < 10) {
	    GDBG_ERROR("main","triangle strip is too large, extends off Y screen\n");
	    DIAG_FAIL();
	}
	if (smode & SST_SETUP_ST0) {		// if texturing , add it in
	    fbzCP |= SST_ENTEXTUREMAP | SST_RGBSEL_TMUOUT| SST_CC_ADD;
	}
	else fbzCP |= SST_CC_REPLACE;
	if (smode & SST_SETUP_A) {
	    fbzCP |= SST_CCA_REPLACE;
	    SET(sst->alphaMode, SST_ENALPHABLEND |
			(SST_A_SRCALPHA<<SST_RGBSRCFACT_SHIFT) |
			(SST_AOM_SRCALPHA<<SST_RGBDSTFACT_SHIFT));
	}

	SET(sst->fbzColorPath, fbzCP);
	SET(sst->fbzMode, SST_RGBWRMASK | auxbits | drawbufferRandom());
	csrc = colRandom16();			// and random colors
	SET(sst->c0,csrc);

	sst_idle_really(sst);
	startTime = DIAG_TIME();
	gdbg_info(1,"start of %d strips at time = %d ns\n",strips,startTime);

      for (n=0; n<strips; n++) {		// do # strips
	double a = aRandom();			// get a random angle
	float sina = (float)sin(a), cosa = (float)cos(a);

	gdbg_info(1,"rRandom %d %d\n",(int)((size+2)*rotsin*XY_ONE),
			(diago.xmaxscreen-(int)(size*(istop+1)/2*rotcos))*XY_ONE);
	t->vA.x = rRandom((int)((size+2)*rotsin*XY_ONE),
			(diago.xmaxscreen-(int)(size*(istop+1)/2*rotcos))*XY_ONE);
	t->vA.y = iRandom((diago.ymaxscreen-(int)(size*rotcos)-(int)(size*(istop+1)/2*rotsin))*XY_ONE);
	t->vA.fx = t->vA.x/(float)XY_ONE;
	t->vA.fy = t->vA.y/(float)XY_ONE;
	t->vA.fz = fexpRandom(8,SST_Z_INTBITS);
	t->vA.fw = 1.0F/fexpRandom(1,10);
	if (mipmapping) {
	    t->vA.fs = fexpRandom(0,9)*t->vA.fw;
	    t->vA.ft = fexpRandom(0,9)*t->vA.fw;
	    t->vA.fs1 = fexpRandom(0,9)*t->vA.fw;
	    t->vA.ft1 = fexpRandom(0,9)*t->vA.fw;
	}
	else {
	    t->vA.fs = t->vA.fx * ratio;
	    t->vA.ft = t->vA.fy * ratio;
	    t->vA.fs1 = t->vA.fx * ratio1;
	    t->vA.ft1 = t->vA.fy * ratio1;
	}


	t->vB.fx = t->vA.fx - size*rotsin;
	t->vB.fy = t->vA.fy + size*rotcos;
	t->vB.fz = fexpRandom(8,SST_Z_INTBITS);
	t->vB.fw = t->vA.fw + (1.1F*t->vA.fw);
	if (mipmapping) {
	    t->vB.fs = t->vA.fs + (cosa*size*t->vA.fw);
	    t->vB.ft = t->vA.ft + (sina*size*t->vA.fw);
	    t->vB.fs1 = t->vA.fs1 + (cosa*size*t->vA.fw);
	    t->vB.ft1 = t->vA.ft1 + (sina*size*t->vA.fw);
	}
	else {
	    t->vB.fs = t->vB.fx * ratio;
	    t->vB.ft = t->vB.fy * ratio;
	    t->vB.fs1 = t->vB.fx * ratio1;
	    t->vB.ft1 = t->vB.fy * ratio1;
	}

	t->vC = t->vB;

	SET(sst->sSetupMode, smode);
	sendV(sst,&t->vA);
	SET(sst->sBeginTriCMD,0);

	sendV(sst,&t->vB);
	SET(sst->sDrawTriCMD,0);

	itris = diago.bilinear ? (istop+2)/3 : istop;
	gdbg_info(2,"generating %d random tris in a %s\n",
			itris, smode & SST_SETUP_FAN ? "fan" : "strip");
	// NOTE: i==0 for the first triangle since we already output vA and vB
	for (i=0; i<istop; i++) {
	    if (i & 1) {
		t->vC.fx = t->vC.fx - size*rotsin;
		t->vC.fy = t->vC.fy + size*rotcos;
	    }
	    else {
		t->vC.fx = t->vC.fx+size*(rotcos+rotsin);
		t->vC.fy = t->vC.fy+size*(rotsin-rotcos);
	    }
	    t->vC.fr = fexpRandom(-1,SST_RGBA_INTBITS);
	    t->vC.fg = fexpRandom(-3,SST_RGBA_INTBITS);
	    t->vC.fb = fexpRandom(-2,SST_RGBA_INTBITS);
	    t->vC.fa = fexpRandom(-4,SST_RGBA_INTBITS);
	    t->vC.fz = fexpRandom(8,SST_Z_INTBITS);
	    if (smode & SST_SETUP_Wfbi) {
		t->vC.fw = t->vC.fw + 0.2F*t->vC.fw;
	    }
	    else
		t->vC.fw = fexpRandom(-1,8);
	    if (smode & SST_SETUP_ST0) {
		if (mipmapping) {
		    if (i & 1) {
			t->vC.fs = t->vC.fs + (-sina-cosa)*size*t->vC.fw;
			t->vC.ft = t->vC.ft + (cosa-sina)*size*t->vC.fw;
			t->vC.fs1 = t->vC.fs1 + (-sina-cosa)*size*t->vC.fw;
			t->vC.ft1 = t->vC.ft1 + (cosa-sina)*size*t->vC.fw;
		    }
		    else {
			t->vC.fs = t->vC.fs + (cosa*size*t->vC.fw);
			t->vC.ft = t->vC.ft + (sina*size*t->vC.fw);
			t->vC.fs1 = t->vC.fs1 + (cosa*size*t->vC.fw);
			t->vC.ft1 = t->vC.ft1 + (sina*size*t->vC.fw);
		    }
		}
		else {
		    t->vC.fs = t->vC.fx * ratio;
		    t->vC.ft = t->vC.fy * ratio;
		    t->vC.fs1 = t->vC.fx * ratio1;
		    t->vC.ft1 = t->vC.fy * ratio1;
		}
	    }

	    sendV(sst,&t->vC);
	    if (diago.bilinear && !((i+2)%3)) {
		SET(sst->sBeginTriCMD,0);
	    }
	    else
		SET(sst->sDrawTriCMD,0);
	}
      }
	sst_idle_really(sst);
	endTime = DIAG_TIME();
	gdbg_info(1,"end of %d strips at time = %d ns  angle=%d LODbias=%s\n",
			strips,endTime,angle,diago.lodbias?"0.5":"0.0");
	gdbg_printf("%d strips of %d triangles in %d ns, %.2fM triangles/sec\n",
		strips,itris,endTime-startTime,itris*strips*1e3/(endTime-startTime));
    }
    DIAG_PASS(0);
}
