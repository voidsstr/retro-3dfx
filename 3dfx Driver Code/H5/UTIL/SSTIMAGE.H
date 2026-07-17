#ifndef __SSTIMAGE_H__
#define __SSTIMAGE_H__

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
** $Date: 10/11/00 8:41:29 PM$
*/

#ifndef __CSIM_H__

// translation tables for decompressing NCC format
typedef struct {
    unsigned char yRGB[16];
    int iRGB[4][3];
    int qRGB[4][3];
} NccTable;

// 256 color palette tables
typedef struct {
    unsigned long argb[256];
} Pal256;

#endif

extern unsigned long sstMipMapSize[4][16]; 
extern unsigned long sstMipMapOffset[4][16];
extern unsigned long sstMipMapOffset_Tsplit[4][16];
void sstMipMapInit(void);

void sstSetNccTable(SstRegs *sst, int n, NccTable *ncc);
void sstSetPal256Table(SstRegs *sst, Pal256 *pal);

unsigned char *sstDither332(unsigned char *out, unsigned int *__in,
				int __xsize, int __ysize);
unsigned short *sstDither565(unsigned short *out, unsigned int *__in,
				int __xsize, int __ysize);
unsigned short *sstTruncate565(unsigned short *out, unsigned int *__in,
				int __xsize, int __ysize);
unsigned char *sstAI44(unsigned char *out, unsigned int *__in,
				int __xsize, int __ysize);
unsigned short *sstAI88(unsigned short *out, unsigned int *__in,
				int __xsize, int __ysize);

long sstDownLoadTexture(SstRegs *, int trex, long addr, unsigned long texmode,
			int ar,	int slod, int tlod, int bpt, unsigned long *data);

#endif	/* !__SSTIMAGE_H__ */
