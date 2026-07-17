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
** $Date: 10/11/00 8:09:11 PM$
*/

#include <h3.h>
#include "h3sim.h"

//----------------------------------------------------------------------
// convert various color formats to 8888 RGBA
// note that we replicate msb's into the lsb's until we fill up all 8 bits
//----------------------------------------------------------------------
void	
_sstYab422to8888(NccTable *ncc, unsigned char *c888, unsigned char yab)
{
    unsigned char y,i,q;
    int r,g,b;

    y = yab >> 4;		// separate out YIQ
    i = (yab >> 2) & 0x3;
    q = (yab >> 0) & 0x3;

    r = ncc->yRGB[y] + ncc->iRGB[i][0] + ncc->qRGB[q][0];
    g = ncc->yRGB[y] + ncc->iRGB[i][1] + ncc->qRGB[q][1];
    b = ncc->yRGB[y] + ncc->iRGB[i][2] + ncc->qRGB[q][2];
//gdbg_printf("YIQ: 0x%x  RGB: %d %d %d\n",yab,r,g,b);

    if (r < 0) r = 0;
    else if (r > 0xFF) r = 0xFF;
    if (g < 0) g = 0;
    else if (g > 0xFF) g = 0xFF;
    if (b < 0) b = 0;
    else if (b > 0xFF) b = 0xFF;

//gdbg_printf("YIQ: 0x%x  RGB: 0x%06x\n",yab,(b<<16) | (g<<8) | r);
    c888[0] = r;
    c888[1] = g;
    c888[2] = b;
    c888[3] = 0xFF;
}

void
_sstAi44to8888 (unsigned char c8888[4], unsigned char c44)
{
    c8888[0] =
    c8888[1] =
    c8888[2] = ((c44&0xF)<<4) | (c44&0xF);
    c8888[3] = ((c44&0xF0)>>4) | (c44&0xF0);
}

void
_sstRgba332to8888 (unsigned char c8888[4], unsigned char c332)
{
    static unsigned char a3[] = {0x00,0x24,0x49,0x6d,0x92,0xb6,0xdb,0xff};
    static unsigned char a2[] = {0x00,0x55,0xaa,0xff};

    c8888[0] = a3[c332>>5];
    c8888[1] = a3[(c332>>2)&7];
    c8888[2] = a2[c332&3];
    c8888[3] = 0xFF;
}

void
_sstRgba565to8888 (unsigned char c8888[4], unsigned short c565)
{
    int t;

    t = c565 & 0xF800;
    c8888[0] = (t>>8) | (t>>13);
    t = c565 & 0x07E0;
    c8888[1] = (t>>3) | (t>>9);
    t = c565 & 0x001F;
    c8888[2] = (t<<3) | (t>>2);
    c8888[3] = 0xFF;
}

void
_sstRgba555to8888 (unsigned char c8888[4], unsigned short c555)
{
    int t;

    t = c555 & 0x7C00;
    c8888[0] = (t>>7) | (t>>12);
    t = c555 & 0x03E0;
    c8888[1] = (t>>2) | (t>>7);
    t = c555 & 0x001F;
    c8888[2] = (t<<3) | (t>>2);
    c8888[3] = 0xFF;
}

void
_sstRgba1555to8888 (unsigned char c8888[4], unsigned short c1555)
{
    _sstRgba555to8888(c8888,c1555);
    c8888[3] = c1555 & 0x8000 ? 0xFF : 0x00;
}

void
_sstRgba4444to8888 (unsigned char c8888[4], unsigned short c4444)
{
    int t;

    t = (c4444>>8) & 0xF;
    c8888[0] = t | (t<<4);
    t = (c4444>>4) & 0xF;
    c8888[1] = t | (t<<4);
    t = (c4444>>0) & 0xF;
    c8888[2] = t | (t<<4);
    t = (c4444>>12) & 0xF;
    c8888[3] = t | (t<<4);
}

#ifdef SHARK
_sstRgba1888to8888 (unsigned char c8888[4], unsigned int c1888)
{
    int t;

    t = (c1888>>16) & 0xF;
    c8888[1] = t | (t<<8);
    t = (c1888>>8) & 0xF;
    c8888[2] = t | (t<<8);
    t = (c1888) & 0xF;
    c8888[3] = t | (t<<8);
    t = (c1888>>24) & 0x1;
    c8888[0] = t | (t<<8);
}
#endif

void
_sstPal256to8888(Pal256 *pal, unsigned char c8888[4], unsigned char index)
{
    FxU32 pcol = pal->argb[index & 0xFF];

    GDBG_INFO(174,"pal[%d] = 0x%06x\n",index&0xFF,pcol);
    c8888[0] = (unsigned char) (pcol >> 16);
    c8888[1] = (unsigned char) (pcol >>  8);
    c8888[2] = (unsigned char) (pcol      );
    c8888[3] = 0xFF;
}

void
_sstPal6666to8888(Pal256 *pal, unsigned char c8888[4], unsigned char index)
{
    int t;
    FxU32 pcol = pal->argb[index & 0xFF];

    GDBG_INFO(174,"pal[%d] = 0x%06x\n",index&0xFF,pcol);
    t = (pcol>>10) & 0xFC;
    c8888[0] = t | (t>>6);
    t = (pcol>>4) & 0xFC;
    c8888[1] = t | (t>>6);
    t = (pcol<<2) & 0xFC;
    c8888[2] = t | (t>>6);
    t = (pcol>>16) & 0xFC;
    c8888[3] = t | (t>>6);
}

// convert 8888 ARGB to 565 RGB
void
_sstRgba8888to565 (unsigned short *c565, unsigned long c8888)
{
    *c565 = (unsigned short)
      (((c8888 & 0xF80000)>>8) | ((c8888 & 0xFC00)>>5) | ((c8888 & 0xF8)>>3));
}

// convert 8888 ARGB to 1555 RGB
void
_sstRgba8888to1555 (unsigned short *c1555, unsigned long c8888)
{
  *c1555 = (unsigned short)
    (((c8888 >> 31) << 15) | ((c8888 & 0xF80000)>>9) | ((c8888 & 0xF800)>>6) | ((c8888 & 0xF8)>>3));
    
}

//----------------------------------------------------------------------
// convert from the specified format to our internal format which 
// is ABGR (in a register from bits 31:0)
//----------------------------------------------------------------------
unsigned long
_sstRgbaLanes8888(SstRegs *sst, unsigned long data)
{
    switch (sst->lfbMode & SST_LFB_RGBALANES) {
	case SST_LFB_RGBALANES_ARGB:	// 0
	    return (data & 0xFF00FF00) | ((data>>16)&0xFF) | ((data&0xFF)<<16);
	case SST_LFB_RGBALANES_ABGR:	// 1
	    return data;
	case SST_LFB_RGBALANES_RGBA:	// 2
	    return (data<<24) | ((data&0xFF00)<<8) | ((data>>8)&0xFF00) | (data>>24);
	case SST_LFB_RGBALANES_BGRA:	// 3
	    return (data<<24) | (data>>8);
    }
    return 0;
}

//----------------------------------------------------------------------
// convert from the specified format to our internal format which 
// is RGB (in a register from bits 15:0)
//----------------------------------------------------------------------
unsigned short
_sstRgbaLanes565(SstRegs *sst, unsigned long data)
{
    switch (sst->lfbMode & SST_LFB_RGBALANES) {
	case SST_LFB_RGBALANES_ARGB:	// 0
	case SST_LFB_RGBALANES_RGBA:	// 2
	    return (unsigned short)data;
	case SST_LFB_RGBALANES_ABGR:	// 1
	case SST_LFB_RGBALANES_BGRA:	// 3
	    return (unsigned short)
		((data<<11) | (data & 0x07E0) | ((data>>11)&0x001F));
    }
    return 0;
}

unsigned short
_sstRgbaLanes1555(SstRegs *sst, unsigned long data)
{
    switch (sst->lfbMode & SST_LFB_RGBALANES) {
	case SST_LFB_RGBALANES_ARGB:	// 0
	    return (unsigned short)data;
	case SST_LFB_RGBALANES_ABGR:	// 1
	    return (unsigned short)
		(((data<<10)&0x7C00) | (data & 0x83E0) | ((data>>10)&0x001F));
	case SST_LFB_RGBALANES_RGBA:	// 2
	    return (unsigned short)(((data>>1)&0x7FFF) | (data<<15));
	case SST_LFB_RGBALANES_BGRA:	// 3
	    return (unsigned short) ((data<<15) |
		((data<<9)&0x7C00) | ((data>>1) & 0x03E0) | ((data>>11)&0x001F));
    }
    return 0;
}


//----------------------------------------------------------------------
// utility dithering routines
//----------------------------------------------------------------------
int _sstDit5(int n, int d, int mask)
{
    n = ((n<<1) | ((n>>7)&mask)) - ((n>>4)&mask);	// normalize
    n += d;				// add in dither
    return n>>4;			// return integer
}

int _sstDit6(int n, int d, int mask)
{
    n = ((n<<2) | ((n>>6)&mask)) - ((n>>4)&mask);	// normalize
    n += d;				// add in dither
    return n>>4;			// return integer
}
