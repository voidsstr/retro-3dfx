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
** $Date: 10/11/00 8:08:56 PM$
*/

#ifndef __H3SIM_H__
#define __H3SIM_H__

#include "csim.h"

#define ABS(x) ((x)<0 ? -(x) : (x))
#define SIGN_EXTEND(x,nbits) ((((signed)(x))<<(32-(nbits)))>>(32-(nbits)))

#define LOWORD(x) ((FxU16)((x)&0xFFFF))
#define HIWORD(x) ((FxU16)((x)>>16))

// default CSIM configuration
#define CSIM_DEFAULT_DEVICE_ID SST_DEVICE_ID_AP_OEM

#define CSIM_DEFAULT_FBI_REV 1
#define CSIM_DEFAULT_FBI_MEM 4
#define CSIM_DEFAULT_TMU_NUM 2
#define CSIM_DEFAULT_TMU_REV 4

// some internal precisions
#define SST_E_SIZE	32
#define SST_E_INTBITS	28
#define SST_UV_SIZE	32
#define SST_UV_INTBITS	28
#define SST_UV_FRACBITS	8
#define SST_WRECIP_FRACBITS 16
#define SST_ST64_SIZE	48
#define SST_ST64_INTBITS 16
#define SST_ST64_FRACBITS 32
#define SST_Z64_SIZE	48
#define SST_Z64_INTBITS_32BPP	24
#define SST_Z64_FRACBITS_32BPP	20
#define SST_Z64_INTBITS_16BPP	16
#define SST_Z64_FRACBITS_16BPP	28
#define SST_W64_SIZE	48
#define SST_W64_INTBITS	16
#define SST_W64_FRACBITS 32

// GMT: these need to go
extern FxI32 sstMipMapOffset[4][16];
extern FxI32 sstMipMapOffset_Tsplit[4][16];

typedef int (* AFUNC)(int,int,int,int);
extern AFUNC _srcFactRGB[], _srcFactA[];
extern AFUNC _dstFactRGB[], _dstFactA[];
extern AFUNC _srcFactA32[], _dstFactA32[];

extern int sstLOG(unsigned long w);
extern int sstLod(SstRegs *);

extern FxI64 _convertFloat2Fix64(FxU32 data, int bits);
extern void sstCopyVertex(SstRegs *sst);
extern int sstTriangleSetup (SstRegs *sst);
extern void sstGo(SstRegs *, int cmdCode);
extern void sstgGo(SstRegs *, SstGRegs *);
extern void sstgLaunch(SstRegs *, SstGRegs *, FxU32 data);
extern void sstgPrintModes(SstGRegs *, char *msg);
extern void sstgPrintRegs(SstGRegs *, char *msg);
extern  int sstChromaTest(FxU32 r, FxU32 g, FxU32 b, FxU32 key, FxU32 range);
extern  int sstGuardbandClip(SstRegs *sst, int x, int y);
extern  int sstRectClip(SstRegs *sst, int x, int y);
extern void sstRecipInit(SstRegs *);
extern void sstTextureUV(SstRegs *);
extern  int sstWfloat64(SstRegs *, FxI64 w);
extern void sstPrintModes(SstRegs *, char *msg, int cmdCode);
extern void sstPrintRegs(SstRegs *, char *msg);
extern void sstPrintSpanRegs(SstRegs *, char *msg, int level);
extern void sstPrintStats(SstRegs *);

extern void sstCompositeRGB(SstRegs *, int isFbi,
			unsigned char local[4], unsigned char other[4],
			unsigned char iterators[4],
			unsigned char c_mselect_7[3],
			unsigned char localTexture[4],
			unsigned char otherTexture[4],
			unsigned long mode, unsigned long reverse);
extern void sstCompositeA(SstRegs *, int isFbi,
			unsigned char local[4],unsigned char other[4],
			unsigned char iterators[4],
                        unsigned char localTextureAlpha,
			unsigned char otherTextureAlpha,
			unsigned long mode, unsigned long reverse);
extern void sstTrexColor(SstRegs *, unsigned char out[4], unsigned char in[4]);
extern void sstFbiPixel(SstRegs *, int x, int y, FxBool useLfbData);
extern void sstFbiLfb(SstRegs *, int addr, unsigned long data, int words);
extern NccTable *sstValidateNcc(SstRegs *);

typedef enum {tmu0TexturePort, tmu1TexturePort, largeTexturePort} TexturePort;
extern void sstTrexWriteMem(SstRegs *, long addr, FxU32 data, int nbytes, TexturePort texturePort);
extern void sstTrexBackdoor(SstRegs *, long addr, FxU32 data, int nbytes, TexturePort texturePort);

// CSIM routines to aid diagnostic checking
void trexAddToCheckFifo(unsigned long rgba, int x, int y, int tcount, int pcount);
void trexFifoCheckAllCsim(void);
void trexFifoCheckAllHsim(void);

#endif	/* !__H3SIM_H__ */

