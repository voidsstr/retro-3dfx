#include "vxd.h"
/*
** Copyrightc) 1997, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 8:09:18 PM$
*/

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

#include <h3.h>
#include "h3sim.h"

static char *cmd_str[] ={"NOP", "BLT",  "SBLT", "HBLT",
			 "HSBL","RECT", "LINE", "PLIN",
			 "POLG","****", "****", "****",
			 "****","****", "****", "****"};
static char *lo_str[] ={"ZERO", "NOR",  "ANDI", "NSRC",
			"ANDR", "NDST", "XOR",  "NAND",
			"AND",  "XNOR", "DST",  "ORI",
			"SRC",  "ORR",  "OR",   "ONE"};
static char *pixfmt_str[] = {
	"1BPP"," 8BPP","15BPP","16BPP","24BPP","32BPP","****", "****",
	"422YUV","422UVY","411YUV", "****", "****", "****", "****", "****"};

static char *pack_str[] = {"Pk00", "Pk8", "Pk16", "Pk32"};

static char *ropname(FxU32 rop, char *buf)
{
    if (SSTG_ISBINARYROP(rop))	/* if a binary rop	*/
	return lo_str[rop & 0xF];
    sprintf(buf,"%02x",rop);
    return buf;
}

static char blanks[] = "";

//----------------------------------------------------------------------
// print SSTG mode information
//----------------------------------------------------------------------
void sstgPrintModes(SstGRegs *sstg, char *msg)
{
    char buf[4][8];
    FxU32 clipMin, clipMax, stride;
    FxU32 cmd = sstg->command;
    FxU32 cmdX = sstg->commandEx;
    FxU32 srcFormat = sstg->srcFormat;
    FxU32 dstFormat = sstg->dstFormat;

    if (sstg->command & SSTG_CLIPSELECT) {	// first select clip regs
	clipMin = sstg->clip1min;
	clipMax = sstg->clip1max;
    }
    else {
	clipMin = sstg->clip0min;
	clipMax = sstg->clip0max;
    }
    gdbg_info(125,"%s =============== Clip[%s]: [%d,%d] to (%d,%d) ===============\n",
		msg,
		cmd & SSTG_CLIPSELECT ? "1" : "0",
		LOWORD(clipMin),HIWORD(clipMin),LOWORD(clipMax),HIWORD(clipMax));
    gdbg_printf("\t command = %s%s %s %s %2s %s %5s %sX %sY  Poff:%d,%d  Rop[0]:%s\n",
		cmd_str[(cmd & SSTG_COMMAND) >> SSTG_COMMAND_SHIFT],
		cmd & SSTG_REVERSIBLE ? "*" : blanks,
		cmd & SSTG_UPDATE_DSTX ? "x+=w" : blanks,
		cmd & SSTG_UPDATE_DSTY ? "y+=h" : blanks,
		cmd & SSTG_EN_LINESTIPPLE ? "LS" : blanks,
		cmd & SSTG_TRANSPARENT ? "TRANS" : "OPAQU",
		cmd & SSTG_MONO_PATTERN ? "MONOP" : blanks,
		cmd & SSTG_XDIR ? "-" : "+",
		cmd & SSTG_YDIR ? "-" : "+",
#if COLORTRANSLUT
		cmd & SSTG_EN_CLUT88 ? "ENCLUT" : blanks,
#endif
		(cmd & SSTG_X_PATOFFSET)>>SSTG_X_PATOFFSET_SHIFT,
		(cmd & SSTG_Y_PATOFFSET)>>SSTG_Y_PATOFFSET_SHIFT,
		ropname(cmd>>SSTG_ROP0_SHIFT,buf[0]));
    gdbg_printf("\t commdEx = %6s %6s %s %s\n",
		cmdX & SSTG_EN_SRC_COLORKEY_EX ? "SRCKEY" : blanks,
		cmdX & SSTG_EN_DST_COLORKEY_EX ? "DSTKEY" : blanks,
		cmdX & SSTG_WAIT_FOR_VSYNC_EX ? "waitVSYNC" : blanks,
		cmdX & SSTG_PAT_FORCE_ROW0 ? "patForceRow0" : blanks);
    gdbg_printf("\trop[3:0] = %s %s %s %s\tlineStipple: 0x%08x\n",
		ropname((sstg->rop>>16)&0xFF,buf[3]),
		ropname((sstg->rop>>8)&0xFF,buf[2]),
		ropname(sstg->rop&0xFF,buf[1]),
		ropname(cmd>>SSTG_ROP0_SHIFT,buf[0]),
		sstg->lineStipple);
    if (cmd & SSTG_EN_LINESTIPPLE)
	gdbg_printf("\tlineStyle= %d(rep) %d(siz) %d.%d(pos)\n",
		(sstg->lineStyle & SSTG_LSREPEAT) >> SSTG_LSREPEAT_SHIFT,
		(sstg->lineStyle & SSTG_LSSIZE) >> SSTG_LSSIZE_SHIFT,
		(sstg->lineStyle & SSTG_LSPOS_INT) >> SSTG_LSPOS_INT_SHIFT,
		(sstg->lineStyle & SSTG_LSPOS_FRAC) >> SSTG_LSPOS_FRAC_SHIFT);
    if ((sstg->bresError0 | sstg->bresError1) & 0x80000000);
	gdbg_printf("\tbresErr0 = %08x    bresErr1  = %08x\n",
			sstg->bresError0, sstg->bresError1);
    
    stride = (sstg->srcBaseAddr&SSTG_IS_TILED) ?
      (srcFormat&SSTG_SRC_TILE_STRIDE)>>SSTG_SRC_STRIDE_SHIFT :
      (srcFormat&SSTG_SRC_LINEAR_STRIDE)>>SSTG_SRC_STRIDE_SHIFT;
    gdbg_printf("\tcolorBak = %08x    srcFormat = %s 0x%x(%d) %s %s %s %s\n",
		sstg->colorBack,
		pixfmt_str[(srcFormat&SSTG_SRC_FORMAT)>>SSTG_SRC_FORMAT_SHIFT],
		stride,
		stride,
		sstg->srcBaseAddr&SSTG_IS_TILED ? "TILE" : "LIN",
		pack_str[(srcFormat & SSTG_SRC_PACK)>>SSTG_SRC_PACK_SHIFT],
		srcFormat & SSTG_HOST_BYTE_SWIZZLE ? "BSWP" : blanks,
		srcFormat & SSTG_HOST_WORD_SWIZZLE ? "WSWP" : blanks);

    stride = (sstg->dstBaseAddr&SSTG_IS_TILED) ?
      (dstFormat&SSTG_DST_TILE_STRIDE)>>SSTG_DST_STRIDE_SHIFT :
      (dstFormat&SSTG_DST_LINEAR_STRIDE)>>SSTG_DST_STRIDE_SHIFT;
    gdbg_printf("\tcolorFor = %08x    dstFormat = %s 0x%x(%d) %s\n",
		sstg->colorFore,
		pixfmt_str[(dstFormat&SSTG_DST_FORMAT)>>SSTG_DST_FORMAT_SHIFT],
		stride,
		stride,
		sstg->dstBaseAddr&SSTG_IS_TILED ? "TILE" : "LIN");
    gdbg_printf("\t srcAddr = %08x    srcColorKey = %08x %08x  %sEN\n",
		sstg->srcBaseAddr,
		sstg->srcColorkeyMin,
		sstg->srcColorkeyMax,
		cmdX & SSTG_EN_SRC_COLORKEY_EX ? " " : "!");
    gdbg_printf("\t dstAddr = %08x    dstColorKey = %08x %08x  %sEN\n",
		sstg->dstBaseAddr,
		sstg->dstColorkeyMin,
		sstg->dstColorkeyMax,
		cmdX & SSTG_EN_DST_COLORKEY_EX ? " " : "!");
}

void sstgPrintRegs(SstGRegs *sstg, char *msg)
{
    FxU32 mask = SST_MASK(SSTG_XY_SIZE);

    gdbg_info(126,"%s     src = %d,%d %d,%d     dst = %d,%d, %d,%d\n",
		msg,
		SIGN_EXTEND(sstg->srcXY,SSTG_XY_SIZE), SIGN_EXTEND((sstg->srcXY>>16),SSTG_XY_SIZE),
		sstg->srcSize & mask, (sstg->srcSize>>16) & mask,
		SIGN_EXTEND(sstg->dstXY,SSTG_XY_SIZE), SIGN_EXTEND((sstg->dstXY>>16),SSTG_XY_SIZE),
		sstg->dstSize & mask, (sstg->dstSize>>16) & mask);
}
