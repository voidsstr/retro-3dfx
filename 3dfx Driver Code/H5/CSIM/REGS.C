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
** $Revision: 3$
** $Date: 10/11/00 8:09:09 PM$
*/

#include <stdlib.h>

#include <h3.h>
#include "h3sim.h"

//----------------------------------------------------------------------
// An array of SST register info
// NOTE: whenever a register is added or modified, this
//	 **** MUST BE UPDATED ****
//----------------------------------------------------------------------

// XXX GMT: we need to reduce these to the their correct size
static RegInfo regsIoInfo[] = {
 	"STATUS",	0x00000000,0,0,0,0,1,0,
 	"pciInit0",	0x07FFFB7C,0,0,0,0,1,0,
	"sipMonitor",   	0x7FFFFFFF,0,0,0,0,1,0,
	"lfbMemoryConfig",0xE1FFFFFF,12345,0,0,0,1,0,
 	"miscInit0",	0xFFFDFFFF,0,0,0,0,1,0,
 	"miscInit1",	0xFF0FFFF9,0,0,0,0,1,0,
 	"dramInit0",	0x3FFFFFFF,0,0,0,0,1,0,
 	"dramInit1",	0x7FFFFFFF,0,0,0,0,1,0,
 	"agpInit",	0x000007FE,0,0,0,0,1,0,
	"tmuGbeInit",	0x00007FFF,0,0,0,0,1,0,
 	"vgaInit0",	0x01FFFFC7,0,0,0,0,1,0,
 	"vgaInit1",	0x1FFFFFFF,0,0,0,0,1,0,
  	"dramCommand",	0xFFFFFF0F,0,0,0,0,1,0,
  	"dramData",	0xFFFFFFFF,0,0,0,0,1,0,
	"reservedZ_0",	0x00000000,0,0,0,0,0,0,
	"reservedZ_1",	0x00000000,0,0,0,0,0,0,
 	"pllCtrl0",	0x0001FFFF,0,0,0,0,1,0,
 	"pllCtrl1",	0x0001FFFF,0,0,0,0,1,0,
 	"pllCtrl2",	0x0001FFFF,0,0,0,0,1,0,
 	"dacMode",	0x0000001F,0,0,0,0,1,0,
 	"dacAddr",	0x000001FF,0,0,0,0,1,0,
 	"dacData",	0x00FFFFFF,100,0,0,0,1,0,
	"vidMaxRGBDelta",	0x003F3F3F,0,0,0,0,1,0,
 	"vidProcCfg",	0xFFFFFFFF,0,0,0,0,1,0,
 	"hwCurPatAddr",	0x03FFFFFF,0,0,0,0,1,0,
 	"hwCurLoc",	0x07FF07FF,0,0,0,0,1,0,
 	"hwCurC0",	0x00FFFFFF,0,0,0,0,1,0,
 	"hwCurC1",	0x00FFFFFF,0,0,0,0,1,0,
 	"vidInFormat",	0x003FCFFF,0,0,0,0,1,0,
 	"vidInStatus",	0x00000000,0,0,0,0,1,0,
 	"vidSerialParallelPort",	0xFFFFFFFF,0,0,0,0,1,0,
 	"vidInXDecimDeltas",	0x0FFF0FFF,0,0,0,0,1,0,
 	"vidInDecimInitErrs",	0x1FFF1FFF,0,0,0,0,1,0,
 	"vidInYDecimDeltas",	0x0FFF0FFF,0,0,0,0,1,0,
 	"vidPixelBufThold",	0x0003FFFF,0,0,0,0,1,0,
 	"vidChromaMin",	0xFFFFFFFF,0,0,0,0,1,0,
 	"vidChromaMax",	0xFFFFFFFF,0,0,0,0,1,0,
 	"vidCurrentLine",	0x00000000,0,0,0,0,1,0,
 	"vidScreenSize",	0x00FFFFFF,0,0,0,0,1,0,
 	"vidOverlayStartCoords",	0x0FFFFFFF,0,0,0,0,1,0, // ek
 	"vidOverlayEndScreenCoord",	0x00FFFFFF,0,0,0,0,1,0,
 	"vidOverlayDudx",	0x000FFFFF,0,0,0,0,1,0,
 	"vidOverlayDudxOffsetSrcWidth",	0xFFFFFFFF,0,0,0,0,1,0,
 	"vidOverlayDvdy",	0x000FFFFF,0,0,0,0,1,0,
 	"vgaRegister_0",	0xFFFFFFFF,0,0,0,0,1,0,
 	"vgaRegister_1",	0xFFFFFFFF,0,0,0,0,1,0,
 	"vgaRegister_2",	0xFFFFFFFF,0,0,0,0,1,0,
 	"vgaRegister_3",	0xFFFFFFFF,0,0,0,0,1,0,
 	"vgaRegister_4",	0xFFFFFFFF,0,0,0,0,1,0,
 	"vgaRegister_5",	0xFFFFFFFF,0,0,0,0,1,0,
 	"vgaRegister_6",	0xFFFFFFFF,0,0,0,0,1,0,
 	"vgaRegister_7",	0xFFFFFFFF,0,0,0,0,1,0,
 	"vgaRegister_8",	0xFFFFFFFF,0,0,0,0,1,0,
 	"vgaRegister_9",	0xFFFFFFFF,0,0,0,0,1,0,
 	"vgaRegister_10",	0xFFFFFFFF,0,0,0,0,1,0,
 	"vgaRegister_11",	0xFFFFFFFF,0,0,0,0,1,0,
 	"vidOverlayDvdyOffset",	0x0007FFFF,0,0,0,0,1,0,
 	"vidDesktopStartAddr",	0x03FFFFFF,0,0,0,0,1,0,
 	"vidDesktopOverlayStride",	0x7FFF7FFF,0,0,0,0,1,0,
 	"vidInAddr0",	0x03FFFFFF,0,0,0,0,1,0,
 	"vidInAddr1",	0x03FFFFFF,0,0,0,0,1,0,
 	"vidInAddr2",	0x03FFFFFF,0,0,0,0,1,0,
 	"vidInStride",	0x00007FFF,0,0,0,0,1,0,
 	"vidCurrOverlayStartAddr",	0x00000000,0,0,0,0,1,0,
};

#define MAXIOREGADDR sizeof(regsIoInfo)/sizeof(RegInfo)

static RegInfo regsCmdInfo[] = {
 	"agpReqSize",	0x000FFFFF,0,0,0,0,1,0,
 	"hostAddrLow",	0xFFFFFFFF,0,0,0,0,1,0,
 	"hostAddrHigh",	0xFFFFFFFF,0,0,0,0,1,0,
 	"graphicsAddr",	0x03FFFFFF,0,0,0,0,1,0,
 	"graphicsStride",0x00007FFF,0,0,0,0,1,0,
 	"moveCMD",	0x00000038,0,0,1,0,0,0,
 	"reservedL_0",	0x00000000,0,0,0,0,0,0,
 	"reservedL_1",	0x00000000,0,0,0,0,0,0,

 	"cmd0.BaseAddrL",0x00FFFFFF,0,0,0,0,1,0,
 	"cmd0.BaseSize",0x000007FF,0,0,0,0,1,0,
 	"cmd0.Bump",	0x0000FFFF,200,0,0,0,0,0,
 	"cmd0.ReadPtrL",0xFFFFFFFF,0,0,0,0,1,0,
 	"cmd0.ReadPtrH",0x0000000F,0,0,0,0,1,0,
 	"cmd0.AMin",	0x01FFFFFF,0,0,0,0,1,0,
 	"cmd0.unusedA",	0x00000000,0,0,0,0,0,0,
 	"cmd0.AMax",	0x01FFFFFF,0,0,0,0,1,0,
 	"cmd0.unusedB",	0x00000000,0,0,0,0,0,0,
 	"cmd0.Depth",	0x000FFFFF,0,0,0,0,1,0,
 	"cmd0.HoleCount",0x0000FFFF,0,0,0,0,1,0,
 	"cmd0.reserved",0x00000000,0,0,0,0,0,0,

 	"cmd1.BaseAddrL",0x00FFFFFF,0,0,0,0,1,0,
 	"cmd1.BaseSize",0x000007FF,0,0,0,0,1,0,
 	"cmd1.Bump",	0x0000FFFF,200,0,0,0,0,0,
 	"cmd1.ReadPtrL",0xFFFFFFFF,0,0,0,0,1,0,
 	"cmd1.ReadPtrH",0x0000000F,0,0,0,0,1,0,
 	"cmd1.AMin",	0x01FFFFFF,0,0,0,0,1,0,
 	"cmd1.unusedA",	0x00000000,0,0,0,0,0,0,
 	"cmd1.AMax",	0x01FFFFFF,0,0,0,0,1,0,
 	"cmd1.unusedB",	0x00000000,0,0,0,0,0,0,
 	"cmd1.Depth",	0x000FFFFF,0,0,0,0,1,0,
 	"cmd1.HoleCount",0x0000FFFF,0,0,0,0,1,0,
 	"cmd1.reserved",0x00000000,0,0,0,0,0,0,

 	"cmdFifoThresh",0x0000001F,0,0,0,0,1,0,
 	"reservedO_0",	0x00000000,0,0,0,0,0,0,
 	"reservedO_1",	0x00000000,0,0,0,0,0,0,
 	"reservedO_2",	0x00000000,0,0,0,0,0,0,
 	"reservedO_3",	0x00000000,0,0,0,0,0,0,
 	"reservedO_4",	0x00000000,0,0,0,0,0,0,
 	"reservedO_5",	0x00000000,0,0,0,0,0,0,
 	"reservedO_6",	0x00000000,0,0,0,0,0,0,
 	"reservedP_0",	0x00000000,0,0,0,0,0,0,
 	"reservedP_1",	0x00000000,0,0,0,0,0,0,
 	"reservedP_2",	0x00000000,0,0,0,0,0,0,
 	"reservedP_3",	0x00000000,0,0,0,0,0,0,
 	"reservedP_4",	0x00000000,0,0,0,0,0,0,
 	"reservedP_5",	0x00000000,0,0,0,0,0,0,
 	"reservedP_6",	0x00000000,0,0,0,0,0,0,
 	"reservedP_7",	0x00000000,0,0,0,0,0,0,
 	"reservedQ_0",	0x00000000,0,0,0,0,0,0,
 	"reservedQ_1",	0x00000000,0,0,0,0,0,0,
 	"reservedQ_2",	0x00000000,0,0,0,0,0,0,
 	"reservedQ_3",	0x00000000,0,0,0,0,0,0,
 	"reservedQ_4",	0x00000000,0,0,0,0,0,0,
 	"reservedQ_5",	0x00000000,0,0,0,0,0,0,
 	"reservedQ_6",	0x00000000,0,0,0,0,0,0,
 	"reservedQ_7",	0x00000000,0,0,0,0,0,0,
 	"reservedR_0",	0x00000000,0,0,0,0,0,0,
 	"reservedR_1",	0x00000000,0,0,0,0,0,0,
 	"reservedR_2",	0x00000000,0,0,0,0,0,0,
 	"reservedR_3",	0x00000000,0,0,0,0,0,0,
 	"reservedR_4",	0x00000000,0,0,0,0,0,0,
 	"reservedR_5",	0x00000000,0,0,0,0,0,0,
 	"reservedR_6",	0x00000000,0,0,0,0,0,0,
 	"reservedR_7",	0x00000000,0,0,0,0,0,0,
 	"yuvBaseAddr",	0x03FFFFFF,0,0,0,0,1,0,
	"yuvStride",    0x80003FFF,0,0,0,0,1,0,
 	"reservedS_0",	0x00000000,0,0,0,0,0,0,
 	"reservedS_1",	0x00000000,0,0,0,0,0,0,
 	"reservedS_2",	0x00000000,0,0,0,0,0,0,
 	"reservedS_3",	0x00000000,0,0,0,0,0,0,
 	"reservedS_4",	0x00000000,0,0,0,0,0,0,
 	"reservedS_5",	0x00000000,0,0,0,0,0,0,
 	"crc1",		0xFFFFFFFF,0,0,0,0,1,0,
 	"reservedT_0",	0x00000000,0,0,0,0,0,0,
 	"reservedT_1",	0x00000000,0,0,0,0,0,0,
 	"reservedT_2",	0x00000000,0,0,0,0,0,0,
 	"crc2",		0x00000000,0,0,0,0,1,0, // (Read-Only)
};

#define MAXCMDREGADDR sizeof(regsCmdInfo)/sizeof(RegInfo)

static RegInfo regs2dInfo[] = {
 	"STATUS",	0x00000000,0,0,0,0,1,0,
 	"INTRCTRL",	0x00000FFF,0,0,0,0,1,0,
 	"CLIP0MIN",	0x0FFF0FFF,0,0,0,0,1,0,
 	"CLIP0MAX",	0x0FFF0FFF,0,0,0,0,1,0,
 	"DSTBASEADDR",	0x83FFFFFF,0,0,0,0,1,0,
 	"DSTFORMAT",	0x00073FFF,0,0,0,0,1,0,
 	"SRCCOLORKEYMIN",0x00FFFFFF,0,0,0,0,1,0,
 	"SRCCOLORKEYMAX",0x00FFFFFF,0,0,0,0,1,0,
 	"DSTCOLORKEYMIN",0x00FFFFFF,0,0,0,0,1,0,
 	"DSTCOLORKEYMAX",0x00FFFFFF,0,0,0,0,1,0,
 	"BRESERROR0",	0x8000FFFF,0,0,0,0,1,0,
 	"BRESERROR1",	0x8000FFFF,0,0,0,0,1,0,
 	"ROP",		0x00FFFFFF,0,0,0,0,1,0,
 	"SRCBASEADDR",	0x83FFFFFF,0,0,0,0,1,0,
 	"COMMANDEX",	0xF000000F,0,0,0,0,1,0,
 	"LINESTIPPLE",	0xFFFFFFFF,0,0,0,0,1,0,
 	"LINESTYLE",	0x1FFF1FFF,0,0,0,0,1,0,
 	"PATTERN0ALIAS",0xFFFFFFFF,0,0,0,0,1,1,
 	"PATTERN1ALIAS",0xFFFFFFFF,0,0,0,0,1,2,
 	"CLIP1MIN",	0x0FFF0FFF,0,0,0,0,1,0,
 	"CLIP1MAX",	0x0FFF0FFF,0,0,0,0,1,0,
 	"SRCFORMAT",	0x00FF3FFF,0,0,0,0,1,0,
 	"SRCSIZE",	0x1FFF1FFF,0,0,0,0,1,0,
 	"SRCXY",	0x1FFF1FFF,0,0,0,0,1,0,
 	"COLORBACK",	0xFFFFFFFF,0,0,0,0,1,0,
 	"COLORFORE",	0xFFFFFFFF,0,0,0,0,1,0,
 	"DSTSIZE",	0x1FFF1FFF,0,0,0,0,1,0,
 	"DSTXY",	0x1FFF1FFF,0,0,0,0,1,0,
 	"COMMAND",	0xFFFFFF0F,0,0,1,0,1,0,
	"reserved_0",   0x00000000,0,0,0,0,0,0,
	"reserved_1",   0x00000000,0,0,0,0,0,0,
	"reserved_2",   0x00000000,0,0,0,0,0,0,
};

#define MAX2DREGADDR sizeof(regs2dInfo)/sizeof(RegInfo)

static RegInfo regs3dInfo[] = {
 	"STATUS",	0x00000000,0,0,0,0,1,0,	// (Read-Only)
 	"INTRCTRL",	0x00000FFF,0,0,0,0,1,0,
	"vA.x",		0x0001FFFF,-1,0,0,0,0,0,
	"vA.y",		0x0001FFFF,-1,0,0,0,0,0,
	"vB.x",		0x0001FFFF,-1,0,0,0,0,0,
	"vB.y",		0x0001FFFF,-1,0,0,0,0,0,
	"vC.x",		0x0001FFFF,-1,0,0,0,0,0,
	"vC.y",		0x0001FFFF,-1,0,0,0,0,0,

	"R",		0x00FFFFFF,0,8,0,0,0,0,
	"G",		0x00FFFFFF,0,8,0,0,0,7,
	"B",		0x00FFFFFF,0,8,0,0,0,14,
	"Z",		0xFFFFFFFF,0,0,0,-16,-9,-2,
	"A",		0x00FFFFFF,0,8,0,0,0,5,
	"S",		0xFFFFFFFF,0,18,0,-14,0,12,
	"T",		0xFFFFFFFF,0,18,0,-14,-3,-4,
	"W",		0xFFFFFFFF,0,30,0,-2,-6,3,

	"DRDX",		0x00FFFFFF,0,8,0,0,0,10,
	"DGDX",		0x00FFFFFF,0,8,0,0,0,-6,
	"DBDX",		0x00FFFFFF,0,8,0,0,0,1,
	"DZDX",		0xFFFFFFFF,0,0,0,-16,-10,8,
	"DADX",		0x00FFFFFF,0,8,0,0,0,-8,
	"DSDX",		0xFFFFFFFF,0,18,0,-14,-1,-1,
	"DTDX",		0xFFFFFFFF,0,18,0,-14,-4,6,
	"DWDX",		0xFFFFFFFF,0,30,0,-2,-7,-10,

	"DRDY",		0x00FFFFFF,0,8,0,0,0,-3,
	"DGDY",		0x00FFFFFF,0,8,0,0,0,4,
	"DBDY",		0x00FFFFFF,0,8,0,0,0,-12,
	"DZDY",		0xFFFFFFFF,0,0,0,-16,-11,-5,
	"DADY",		0x00FFFFFF,0,8,0,0,0,2,
	"DSDY",		0xFFFFFFFF,0,18,0,-14,-2,-14,
	"DTDY",		0xFFFFFFFF,0,18,0,-14,-5,-7,
	"DWDY",		0xFFFFFFFF,0,30,0,-2,-8,0,

 	"triangleCMD",	0x80000000,-1,0,SST_TRIANGLECMD,0,0,0,
	"reservedA",	0x00000000,0,0,0,0,0,0,
	"FvA.x",	0x0001FFFF,-1,0,0,4,0,0,
	"FvA.y",	0x0001FFFF,-1,0,0,4,0,0,
	"FvB.x",	0x0001FFFF,-1,0,0,4,0,0,
	"FvB.y",	0x0001FFFF,-1,0,0,4,0,0,
	"FvC.x",	0x0001FFFF,-1,0,0,4,0,0,
	"FvC.y",	0x0001FFFF,-1,0,0,4,0,0,

	"FR",		0x00FFFFFF,0,0,0,12,0,0,
	"FG",		0x00FFFFFF,0,0,0,12,0,7,
	"FB",		0x00FFFFFF,0,0,0,12,0,14,
	"FZ",		0xFFFFFFFF,0,0,0,28,0,-2,
	"FA",		0x00FFFFFF,0,0,0,12,0,5,
	"FS",		0xFFFFFFFF,0,0,0,32,0,12,
	"FT",		0xFFFFFFFF,0,0,0,32,0,-4,
	"FW",		0xFFFFFFFF,0,0,0,32,0,3,

	"FDRDX",	0x00FFFFFF,0,0,0,12,0,10,
	"FDGDX",	0x00FFFFFF,0,0,0,12,0,-6,
	"FDBDX",	0x00FFFFFF,0,0,0,12,0,1,
	"FDZDX",	0xFFFFFFFF,0,0,0,28,0,8,
	"FDADX",	0x00FFFFFF,0,0,0,12,0,-8,
	"FDSDX",	0xFFFFFFFF,0,0,0,32,0,-1,
	"FDTDX",	0xFFFFFFFF,0,0,0,32,0,6,
	"FDWDX",	0xFFFFFFFF,0,0,0,32,0,-10,

	"FDRDY",	0x00FFFFFF,0,0,0,12,0,-3,
	"FDGDY",	0x00FFFFFF,0,0,0,12,0,4,
	"FDBDY",	0x00FFFFFF,0,0,0,12,0,-12,
	"FDZDY",	0xFFFFFFFF,0,0,0,28,0,-5,
	"FDADY",	0x00FFFFFF,0,0,0,12,0,2,
	"FDSDY",	0xFFFFFFFF,0,0,0,32,0,-14,
	"FDTDY",	0xFFFFFFFF,0,0,0,32,0,-7,
	"FDWDY",	0xFFFFFFFF,0,0,0,32,0,0,

 	"FtriangleCMD",	0x80000000,-1,0,SST_TRIANGLECMD,8,0,0,
	"FBZCOLORPATH",	0xFFFFFFFF,-1,0,0,0,1,0,
	"FOGMODE",	0x000FFFFF,0,0,0,0,1,0,
	"ALPHAMODE",	0xFFFFFFFF,0,0,0,0,1,0,
	"FBZMODE",	0x003F3FFF,0,0,0,0,1,0,
	"LFBMODE",	0x0001FFCF,0,0,0,0,1,0,
	"CLIPLEFTRIGHT",0x0FFF0FFF,0,0,0,0,1,0,
	"CLIPBOTTOMTOP",0x0FFF0FFF,0,0,0,0,1,0,

 	"nopCMD",	0x00000003,666187,0,SST_NOPCMD,0,0,0,
 	"fastfillCMD",	0x00000001,0,0,SST_FASTFILLCMD,0,0,0,
 	"swapbufCMD",	0x000003FF,0,0,SST_SWAPBUFCMD,0,0,0,
	"FOGCOLOR",	0xFFFFFFFF,0,0,0,0,0,0,
	"ZACOLOR",	0xFFFFFFFF,0,0,0,0,0,0,
	"CHROMAKEY",	0xFFFFFFFF,0,0,0,0,0,0,
	"CHROMARANGE",  0xFFFFFFFF,0,0,0,0,0,0,
	"userIntrCMD",	0xFFFFFFFF,0,0,SST_USERINTRCMD,0,0,0, // PS FIX MASK

	"STIPPLE",	0xFFFFFFFF,0,0,0,0,1,0,
	"C0",		0xFFFFFFFF,0,0,0,0,1,0,
	"C1",		0xFFFFFFFF,0,0,0,0,1,0,
	"pixelsIN",	0x00000000,0,0,0,0,1,0,	// (Read-Only)
	"chromaFAIL",	0x00000000,0,0,0,0,1,0,	// (Read-Only)
	"zfuncFAIL",	0x00000000,0,0,0,0,1,0,	// (Read-Only)
	"afuncFAIL",	0x00000000,0,0,0,0,1,0,	// (Read-Only)
	"pixelsOUT",	0x00000000,0,0,0,0,1,0,	// (Read-Only)

	"FOGTABLE[0]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[1]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[2]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[3]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[4]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[5]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[6]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[7]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[8]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[9]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[10]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[11]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[12]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[13]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[14]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[15]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[16]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[17]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[18]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[19]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[20]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[21]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[22]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[23]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[24]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[25]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[26]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[27]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[28]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[29]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[30]",	0xFFFFFFFF,0,0,0,0,0,0,
	"FOGTABLE[31]",	0xFFFFFFFF,0,0,0,0,0,0,

	"RENDERMODE",	0xFFFFFFFF,187,0,0,0,1,0,
	"STENCILMODE",	0x0FFFFFFF,0,0,0,0,1,0,
	"STENCILOP",	0x00000FFF,0,0,0,0,1,0,
	"colBufferAddr",	0x83FFFFFF,667,0,0,0,1,0,
	"colBufferStride",	0x0000BFFF,0,0,0,0,1,0,
	"auxBufferAddr",	0x83FFFFFF,668,0,0,0,1,0,
	"auxBufferStride",	0x0000BFFF,0,0,0,0,1,0,
	"stencilFAIL",	0x00000000,0,0,0,0,1,0,	// (Read-Only)

	"clipLeftRight1",	0x8FFF0FFF,0,0,0,0,1,0,
	"clipTopBottom1",	0x8FFF0FFF,0,0,0,0,1,0,
	"COMBINEMODE",	0xFFFFFFFF,187666,0,0,0,0,0,
	"sliCtrl",	0x07FFFFFF,0,0,0,0,0,0,
	"aaCtrl",	0x1FFFFFFF,666,0,0,0,0,0,
	"chipMask",	0xFFFFFFFF,0,0,0,0,0,0,
        "leftDesktopBuf", 0x00000000,0,0,0,0,0,0,
	"reservedD_0",	0x00000000,0,0,0,0,0,0,
	"reservedD_1",	0x00000000,0,0,0,0,0,0,

	"reservedE_0",	0x00000000,0,0,0,0,0,0,
	"reservedE_1",	0x00000000,0,0,0,0,0,0,
	"reservedE_2",	0x00000000,0,0,0,0,0,0,
	"reservedE_3",	0x00000000,0,0,0,0,0,0,
	"reservedE_4",	0x00000000,0,0,0,0,0,0,
	"reservedE_5",	0x00000000,0,0,0,0,0,0,
	"reservedE_6",	0x00000000,0,0,0,0,0,0,

	"reservedF_0",	0x00000000,0,0,0,0,0,0,
	"reservedF_1",	0x00000000,0,0,0,0,0,0,
	"reservedF_2",	0x00000000,0,0,0,0,0,0,
	"swapBufferPend",	0x00000000,0,0,0,0,0,0,
	"leftOverlayBuf",	0x83FFFFFF,0,0,0,0,0,0, // (Write-Only)
	"rightOverlayBuf",	0x03FFFFFF,0,0,0,0,0,0, // (Write-Only)
	"fbiSwapHistory", 	0x00000000,0,0,0,0,1,0,	// (Read-Only)
	"fbiTrianglesOut",	0x00000000,0,0,0,0,1,0,	// (Read-Only)

	"sSETUPMODE",	0x000F00FF,0,0,0,0,0,0,
	"sVX",		0xFFFFFFFF,0,0,0,0,0,0,
	"sVY",		0xFFFFFFFF,0,0,0,0,0,0,
	"sARGB",	0xFFFFFFFF,123,0,0,0,0,0,
	"sRED",		0xFFFFFFFF,0,0,0,0,0,0,
	"sGREEN",	0xFFFFFFFF,0,0,0,0,0,0,
	"sBLUE",	0xFFFFFFFF,0,0,0,0,0,0,
	"sALPHA",	0xFFFFFFFF,0,0,0,0,0,0,

	"sVZ",		0xFFFFFFFF,0,0,0,0,0,0,
	"sOOWFBI",	0xFFFFFFFF,0,0,0,0,0,0,
	"sOOW0",	0xFFFFFFFF,0,0,0,0,0,0,
	"sSOW0",	0xFFFFFFFF,0,0,0,0,0,0,
	"sTOW0",	0xFFFFFFFF,0,0,0,0,0,0,
	"sOOW1",	0xFFFFFFFF,0,0,0,0,0,0,
	"sSOW1",	0xFFFFFFFF,0,0,0,0,0,0,
	"sTOW1",	0xFFFFFFFF,0,0,0,0,0,0,

	"sDrawTriCMD",	0x00000000,0,0,SST_SDRAWTRICMD,0,0,0,
	"sBeginTriCMD",	0x00000000,0,0,SST_SBEGINTRICMD,0,0,0,
	"reservedG_0",	0x00000000,0,0,0,0,0,0,
	"reservedG_1",	0x00000000,0,0,0,0,0,0,
	"reservedG_2",	0x00000000,0,0,0,0,0,0,
	"reservedG_3",	0x00000000,0,0,0,0,0,0,
	"reservedG_4",	0x00000000,0,0,0,0,0,0,
	"reservedG_5",	0x00000000,0,0,0,0,0,0,

	"reservedH_0",	0x00000000,0,0,0,0,0,0,
	"reservedH_1",	0x00000000,0,0,0,0,0,0,
	"reservedH_2",	0x00000000,0,0,0,0,0,0,
	"reservedH_3",	0x00000000,0,0,0,0,0,0,
	"reservedH_4",	0x00000000,0,0,0,0,0,0,
	"reservedH_5",	0x00000000,0,0,0,0,0,0,
	"reservedH_6",	0x00000000,0,0,0,0,0,0,
	"reservedH_7",	0x00000000,0,0,0,0,0,0,

	"reservedI_0",	0x00000000,0,0,0,0,0,0,
	"reservedI_1",	0x00000000,0,0,0,0,0,0,
	"reservedI_2",	0x00000000,0,0,0,0,0,0,
	"reservedI_3",	0x00000000,0,0,0,0,0,0,
	"reservedI_4",	0x00000000,0,0,0,0,0,0,
	"reservedI_5",	0x00000000,0,0,0,0,0,0,
	"reservedI_6",	0x00000000,0,0,0,0,0,0,
	"reservedI_7",	0x00000000,0,0,0,0,0,0,

	"TEXTUREMODE",	0xFFFFFFFF,0,0,0,0,0,0,		// shrink these down later
	"TLOD",		0x7FFFFFFF,0,0,0,0,0,0,
	"TDETAIL",	0x003FFFFF,0,0,0,0,0,0,
	"TEXBASEADDR",	0xFFFFFFF3,700,0,0,0,0,0,
	"TEXBASEADDR1",	0x03FFFFFF,700,0,0,0,0,0,
	"TEXBASEADDR2",	0x03FFFFFF,700,0,0,0,0,0,
	"TEXBASEADDR38",0x03FFFFFF,700,0,0,0,0,0,
	"TREXINIT0",	0xFFFFFFFF,0,0,0,0,0,0,

	"TREXINIT1",	0xFFFFFFFF,0,0,0,0,0,0,
	"NCCTABLE0[0]",	0xFFFFFFFF, 9,0,0,0,0,0,
	"NCCTABLE0[1]",	0xFFFFFFFF, 9,0,0,0,0,0,
	"NCCTABLE0[2]",	0xFFFFFFFF, 9,0,0,0,0,0,
	"NCCTABLE0[3]",	0xFFFFFFFF, 9,0,0,0,0,0,
	"NCCTABLE0[4]",	0x07FFFFFF,10,0,0,0,0,0,
	"NCCTABLE0[5]",	0x07FFFFFF,10,0,0,0,0,0,
	"NCCTABLE0[6]",	0x07FFFFFF,10,0,0,0,0,0,

	"NCCTABLE0[7]",	0x07FFFFFF,10,0,0,0,0,0,
	"NCCTABLE0[8]",	0x07FFFFFF,10,0,0,0,0,0,
	"NCCTABLE0[9]",	0x07FFFFFF,10,0,0,0,0,0,
	"NCCTABLE0[10]",0x07FFFFFF,10,0,0,0,0,0,
	"NCCTABLE0[11]",0x07FFFFFF,10,0,0,0,0,0,
	"NCCTABLE1[0]",	0xFFFFFFFF,11,0,0,0,0,0,
	"NCCTABLE1[1]",	0xFFFFFFFF,11,0,0,0,0,0,
	"NCCTABLE1[2]",	0xFFFFFFFF,11,0,0,0,0,0,

	"NCCTABLE1[3]",	0xFFFFFFFF,11,0,0,0,0,0,
	"NCCTABLE1[4]",	0x07FFFFFF,11,0,0,0,0,0,
	"NCCTABLE1[5]",	0x07FFFFFF,11,0,0,0,0,0,
	"NCCTABLE1[6]",	0x07FFFFFF,11,0,0,0,0,0,
	"NCCTABLE1[7]",	0x07FFFFFF,11,0,0,0,0,0,
	"NCCTABLE1[8]",	0x07FFFFFF,11,0,0,0,0,0,
	"NCCTABLE1[9]",	0x07FFFFFF,11,0,0,0,0,0,
	"NCCTABLE1[10]",0x07FFFFFF,11,0,0,0,0,0,

	"NCCTABLE1[11]",0x07FFFFFF,11,0,0,0,0,0,
	
};

#define MAX3DREGADDR sizeof(regs3dInfo)/sizeof(RegInfo)

//----------------------------------------------------------------------
// initializes any configuration dependent register info
//----------------------------------------------------------------------
void csimRegisterInfoInit( SstRegs *sst )
{

    if (MAX3DREGADDR*4 != sizeof(SstRegs)) {
	GDBG_ERROR("csimInit","REGINFO size != SST structure size\n");
	exit(2);
    }
}

//----------------------------------------------------------------------
// returns a pointer to RegInfo struct given a byte offset into registers
//----------------------------------------------------------------------
RegInfo *csimRegisterIoInfo( FxU32 addr )
{
    FxI32 reg = addr >> 2;			// turn into index

    if (reg < MAXIOREGADDR)			// sanity check
	return regsIoInfo + reg;
    else
	return NULL;
}

RegInfo *csimRegisterCmdInfo( FxU32 addr )
{
    FxI32 reg = addr >> 2;			// turn into index

    if (reg < MAXCMDREGADDR)			// sanity check
	return regsCmdInfo + reg;
    else
	return NULL;
}

RegInfo *csimRegister2dInfo( FxU32 addr )
{
    FxI32 reg = addr >> 2;			// turn into index

    if (reg < MAX2DREGADDR)			// sanity check
	return regs2dInfo + reg;
    else
	return NULL;
}

RegInfo *csimRegister3dInfo( FxU32 addr )
{
    FxI32 reg = addr >> 2;			// turn into index

    if (reg < MAX3DREGADDR)			// sanity check
	return regs3dInfo + reg;
    else
	return NULL;
}
