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
** $Date: 10/11/00 8:41:28 PM$
*/

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
 
#include <h3.h>

#include "sstimage.h"

static int once = 0;

unsigned long sstMipMapSize[4][16] = {
    {	// 1:1 aspect ratio
	0x10000,	// 0 : 256x256
	0x04000,	// 1 : 128x128
	0x01000,	// 2 :  64x64
	0x00400,	// 3 :  32x32
	0x00100,	// 4 :  16x16
	0x00040,	// 5 :   8x8
	0x00010,	// 6 :   4x4
	0x00004,	// 7 :   2x2
#ifdef CVG
	0x00004,	// 8 :   1x1
#else
	0x00001,	// 8 :   1x1
#endif
    },
    {	// 2:1 aspect ratio
	0x08000,	// 0 : 256x128
	0x02000,	// 1 : 128x64
	0x00800,	// 2 :  64x32
	0x00200,	// 3 :  32x16
	0x00080,	// 4 :  16x8
	0x00020,	// 5 :   8x4
	0x00008,	// 6 :   4x2
#ifdef CVG
	0x00004,	// 7 :   2x1
	0x00004,	// 8 :   1x1
#else
	0x00002,	// 7 :   2x1
	0x00001,	// 8 :   1x1
#endif
    },
    {	// 4:1 aspect ratio
	0x04000,	// 0 : 256x64
	0x01000,	// 1 : 128x32
	0x00400,	// 2 :  64x16
	0x00100,	// 3 :  32x8
	0x00040,	// 4 :  16x4
	0x00010,	// 5 :   8x2
#ifdef CVG
	0x00008,	// 6 :   4x1
	0x00004,	// 7 :   2x1
	0x00004,	// 8 :   1x1
#else
	0x00004,	// 6 :   4x1
	0x00002,	// 7 :   2x1
	0x00001,	// 8 :   1x1
#endif
    },
    {	// 8:1 aspect ratio
	0x02000,	// 0 : 256x32
	0x00800,	// 1 : 128x16
	0x00200,	// 2 :  64x8
	0x00080,	// 3 :  32x4
	0x00020,	// 4 :  16x2
#ifdef CVG
	0x00010,	// 5 :   8x1
	0x00008,	// 6 :   4x1
	0x00004,	// 7 :   2x1
	0x00004,	// 8 :   1x1
#else
	0x00008,	// 5 :   8x1
	0x00004,	// 6 :   4x1
	0x00002,	// 7 :   2x1
	0x00001,	// 8 :   1x1
#endif
    },
};

// the offset from mipmap level 0 of each mipmap level in texels
unsigned long sstMipMapOffset[4][16];
unsigned long sstMipMapOffset_Tsplit[4][16];

// this is called from sstGinit()
void sstMipMapInit(void)
{
    int ar,lod;
    once = 1;

    for (ar=0; ar<4; ar++) {		// for each aspect ratio
	sstMipMapOffset[ar][0] = 0;	// start off with offset=0
	for (lod=1; lod<=8; lod++) {	// for each lod, add in previous size
	    sstMipMapOffset[ar][lod] = sstMipMapOffset[ar][lod-1] +
					sstMipMapSize[ar][lod-1];
	}
	sstMipMapOffset_Tsplit[ar][0] = 0;	// start off with offset=0
	sstMipMapOffset_Tsplit[ar][1] = 0;	// start off with offset=0
	for (lod=2; lod<=8; lod++) {	// for each lod, add in previous size
	    sstMipMapOffset_Tsplit[ar][lod] = sstMipMapOffset_Tsplit[ar][lod-2] +
					sstMipMapSize[ar][lod-2];
	}
    }
}

void
sstSetNccTable(SstRegs *sst, int n, NccTable *ncc)
{
    unsigned char *nc = ncc->yRGB;
    int i,t,*ni;
    volatile unsigned long *nTab;

    nTab = n ? sst->nccTable1 : sst->nccTable0;
    for (i=0; i<4; i++) {	// first the Y table
	t = (nc[3]<<24) | (nc[2]<<16) | (nc[1]<<8) | nc[0];
	SET(nTab[i], t);
	nc += 4;
    }
    ni = (int *)nc;
    for (i=4; i<12; i++) {	// then pack I,Q tables: R|G|B
	t = ((ni[0]&0x1FF)<<18) | ((ni[1]&0x1FF)<<9) | (ni[2]&0x1FF);
	SET(nTab[i], t);
	ni += 3;
    }
}

void
sstSetPal256Table(SstRegs *sst, Pal256 *pal)
{
    int 	i;
    volatile unsigned long *nccTabI0 = &sst->nccTable0[4];

    for (i=0; i<256; i++) {
    	unsigned long	c;

		c = (pal->argb[i] & 0x00ffffff) | ((i << 23) & 0x7f000000); 
		c |= 0x80000000;
    	SET(nccTabI0[i&7], c);
    }
}

// use a 4x4 dither, should probably have the option for 2x2 dither
static int dithmat[4][4] = {0,8,2,10, 12,4,14,6, 3,11,1,9, 15,7,13,5};
static int dithmat2[4][4] = {10,6,10,6, 2,14,2,14, 10,6,10,6, 2,14,2,14};

unsigned char *
sstDither332(unsigned char *out, unsigned int *data, int xsize, int ysize)
{
    int d,n,x,y;
    unsigned int t32;
    unsigned char t8,*d8;

    if (out==NULL)
	out = (unsigned char *)malloc(xsize*ysize*sizeof(*d8));
    if (getenv("SST_TEX_DITHER2"))
	memcpy(dithmat,dithmat2,sizeof(dithmat2));
    d8 = out;
    for (y=0; y<ysize; y++) {
	for (x=0; x<xsize; x++) {		// for each texel
	    t32 = *data++;			// get the 32 bit texel
	    d = dithmat[y&3][x&3];

	    n = (t32>>16) & 0xFF;		// get RED channel
	    n = (int)(0x70/255.0F * n + 0.5F);	// scale
	    n += d;				// add in dither
	    t8 = (n>>4)<<5;

	    n = (t32>>8) & 0xFF;		// get GREEN channel
	    n = (int)(0x70/255.0F * n + 0.5F);	// scale
	    n += d;				// add in dither
	    t8 |= (n>>4)<<2;

	    n = t32 & 0xFF;			// get BLUE channel
	    n = (int)(0x30/255.0F * n + 0.5F);	// scale
	    n += d;				// add in dither
	    t8 |= (n>>4)<<0;

	    *d8++ = t8;				// store the 8 bit texel
	}
    }
    return out;
}

unsigned short *
sstDither565(unsigned short *out, unsigned int *data, int xsize, int ysize)
{
    int d,n,x,y;
    unsigned int t32;
    unsigned short t16,*d16;

    if (out==NULL)
	out = (unsigned short *)malloc(xsize*ysize*sizeof(*d16));
    if (getenv("SST_TEX_DITHER2"))
	memcpy(dithmat,dithmat2,sizeof(dithmat2));
    d16 = out;
    for (y=0; y<ysize; y++) {
	for (x=0; x<xsize; x++) {		// for each texel
	    t32 = *data++;			// get the 32 bit texel
	    d = dithmat[y&3][x&3];

	    n = (t32>>16) & 0xFF;		// get RED channel
	    n = (int)(0x1F0/255.0F * n + 0.5F);	// scale
	    n += d;				// add in dither
	    t16 = (n>>4)<<11;

	    n = (t32>>8) & 0xFF;		// get GREEN channel
	    n = (int)(0x3F0/255.0F * n + 0.5F);	// scale
	    n += d;				// add in dither
	    t16 |= (n>>4)<<5;

	    n = t32 & 0xFF;			// get BLUE channel
	    n = (int)(0x1F0/255.0F * n + 0.5F);	// scale
	    n += d;				// add in dither
	    t16 |= (n>>4)<<0;

	    *d16++ = t16;			// store the 16 bit texel
	}
    }
    return out;
}

unsigned short *
sstTruncate565(unsigned short *out, unsigned int *data, int xsize, int ysize)
{
    int x,y;
    unsigned short *d16;
    unsigned int t32;

    if (out==NULL)
	out = (unsigned short *)malloc(xsize*ysize*sizeof(*d16));
    d16 = out;
    for (y=0; y<ysize; y++) {
	for (x=0; x<xsize; x++) {		// for each texel
	    t32 = *data++;			// get the 32 bit texel
	    *d16++ =	((t32>>(19-11))&0xF100)|// red
			((t32>>(10-5))&0x7E0) |	// green
			((t32>>3)&0x1F);	// blue
	}
    }
    return out;
}

unsigned char *
sstAI44(unsigned char *out, unsigned int *data, int xsize, int ysize)
{
    int x,y;
    unsigned char *d8;
    unsigned int t32;
    float I;

    if (out==NULL)
	out = (unsigned char *)malloc(xsize*ysize*sizeof(*d8));
    d8 = out;
    for (y=0; y<ysize; y++) {
	for (x=0; x<xsize; x++) {		// for each texel
	    t32 = *data++;			// get the 32 bit texel
	    I =	((t32>>16)&0xFF) * .30F +	// red
		((t32>>8)&0xFF) * .59F +	// green
		(t32&0xFF) * .11F;		// blue
	    I /= 16.0F;				// convert 8. to 4.
	    *d8++ = (((t32>>24)+0x8) & 0xF0) | (int)(I + 0.5F);
	}
    }
    return out;
}

unsigned short *
sstAI88(unsigned short *out, unsigned int *data, int xsize, int ysize)
{
    int x,y;
    unsigned short *d16;
    unsigned int t32;
    float I;

    if (out==NULL)
	out = (unsigned short *)malloc(xsize*ysize*sizeof(*d16));
    d16 = out;
    for (y=0; y<ysize; y++) {
	for (x=0; x<xsize; x++) {		// for each texel
	    t32 = *data++;			// get the 32 bit texel
	    I =	((t32>>16)&0xFF) * .30F +	// red
		((t32>>8)&0xFF) * .59F +	// green
		(t32&0xFF) * .11F;		// blue
	    *d16++ = ((t32>>16) & 0xFF00) | (int)(I + 0.5F);
	}
    }
    return out;
}

// download a retangular texture to sst; HACK: negative address implies TSPLIT
// and return the next available texture addr (in this TREX)
long sstDownLoadTexture(SstRegs *sst, 		// base address of entire SST
			int trex,		// which TREX chip to load [0,3]
			long addr,		// base address of this mipmap
			unsigned long texmode,	// textureMode (8 bit sequential)
			int ar,			// aspect ratio
			int slog,		// log2 of S size
			int tlog,		// log2 of T size
			int bpt,		// bytes per texel
			unsigned long *data)	// the texture data
{
    long s,t, lodmax, retval;
    long vincr;					// how to increment vaddr after each texture write
    long *vaddr;

    if (!once)
	GDBG_ERROR("sstDownLoadTexture", "sstMipMapInit was not called yet\n");
    // NOTE: ar is not necessarily slog-tlog, eg. its possible to have a texture
    // that is 1x2 with an ar=3, figure that out!
    if (slog >= tlog) {				// s is wider (or square)
	lodmax = 8-slog;
    }
    else {					// t is wider
	lodmax = 8-tlog;
    }
    if (ar > 3 || ar < 0)
	GDBG_ERROR("sstDownLoadTexture","invalid aspect ratio: %d by %d\n",
			1<<slog,1<<tlog);
    // compute mipmap offset (negative addr implies tsplit)
    if (addr < 0) {
	addr = ~addr;
	t = sstMipMapOffset_Tsplit[ar][lodmax];
    }
    else
	t = sstMipMapOffset[ar][lodmax];

    retval = addr+sstMipMapSize[ar][lodmax]*bpt;// compute next avail tex memory
#ifdef CVG
    if (lodmax==8)				// special case 1x1
	retval = (retval+7) & ~7;		// round up to next 8 bytes
#else
    if (trex > 0)
	GDBG_ERROR("sstDownLoadTexture","TMU %d is invalid, only 0 is valid\n",trex);
    if (lodmax==8)				// special case 1x1
	retval = (retval+15) & ~15;		// round up to next 16 bytes
#endif
    GDBG_INFO(110,"sstDownLoadTexture(trex=%d,addr=0x%x,log=%d,%d,%s,0x%x) ==> 0x%x\n",
			trex,addr,slog,tlog, bpt==1?"8b":"16b",data,retval);

    // set the chip's texture base address to the beginning of where mipmap 0 is
    addr -= t * bpt;
#ifdef CVG
    SET(SST_TREX(sst,trex)->texBaseAddr,addr>>3);
#else
    SET(SST_TREX(sst,trex)->texBaseAddr,addr);
#endif

#ifdef CVG
    addr = SST_TEX_ADDRESS(sst);		// get texture address space
    addr += (trex<<21) | (lodmax<<17);		// add in trex # and LOD
    slog = 1<<slog;				// get max S,T values
    tlog = 1<<tlog;

    // detect and handle special cases here
    // 1 by N 16-bit  or 2 by N 8-bit : send 2 bytes each word
    if ((slog==1 && bpt==2) || (slog==2 && bpt==1)) {
	for (t=0; t<tlog; t++) {
	    long *vaddr = (long *)(addr + (t<<9));	// add in T coord
	    if (t & 1) {
		SET(vaddr[0],data[0]>>16);	// send upper word
		data++;
	    }
	    else
		SET(vaddr[0],data[0]);		// send word, upper texel ignored
	}
	return retval;
    }
    if (slog==1 && bpt==1) {			// 1 by N 8-bit
	for (t=0; t<tlog; t++) {
	    long *vaddr = (long *)(addr + (t<<9));	// add in T coord
	    SET(vaddr[0],data[0]>>((t&3)<<3));
	    if ((t & 3) == 3) data++;
	}
	return retval;
    }

    bpt = bpt==1 ? 4:2;			// bpt is now texels per 32-bit word
    vincr = 1;				// how to increment vaddr.

    if (bpt == 4) {
    	if (texmode & SST_SEQ_8_DOWNLD) {
		GDBG_INFO(111, "\tfast 8-bit sequential download\n");
	} else {
    		// Downloading 8bits/texel the old way, 
		vincr = 2;
		GDBG_INFO(111, "\tslow 8-bit download\n");
	}
    } else {
    	// 16bpt, same as it used to be.
    	GDBG_INFO(111, "\t16-bit download\n", bpt);
    }

    for (t=0; t<tlog; t++) {
	vaddr = (long *)(addr + (t<<9));	// add in T coord
    
	for (s=0; s<slog; s+=bpt) {		// loop on S coord
	    SET(vaddr[0],*data);		// send the texture word
	    vaddr += vincr;			// depends on 8/16bpt etc. 
	    data++;				// advance to next word
	}
    }
#else
    FXUNUSED(s);
    FXUNUSED(vincr);
{ static FxU32 hack;	// stores the last texture word written
    addr = sstMipMapOffset[ar][lodmax]*bpt;
    vaddr = (long *)(addr + SST_TEX_ADDRESS(sst));// add in texture address space
    t = 3 & (long)vaddr;
    if (t) {
	GDBG_INFO(0,"WARNING: hacking unaligned write of %d bytes by GMT\n",t);
	if (t != 2)
	    GDBG_ERROR("sstDownLoadTexture","unexpected case of %d bytes by GMT\n",t);
	if (lodmax == 8) {
	    vaddr = (long *)((long)vaddr-t);	// dword align the write
	    t <<= 3;
	    bpt = *data << t;			// move data over
	    bpt |= (hack<<t)>>t;		// merge in with last word
	    SET(vaddr[0],bpt);			// send the texture word
	}
	else {
	    GDBG_ERROR("sstDownLoadTexture","unaligned lod < 8 NYI by GMT\n");
	}
    }
    else
    for (t=sstMipMapSize[ar][lodmax]*bpt; t>0; t-=4) {
	hack = *data;
	SET(vaddr[0],*data);			// send the texture word
	vaddr++;
	data++;
    }
}
#endif
    return retval;				// return the next available loc
}
