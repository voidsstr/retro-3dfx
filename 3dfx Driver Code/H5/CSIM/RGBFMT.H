#ifndef __RGBFMT_H__
#define __RGBFMT_H__

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
** $Date: 10/11/00 8:09:13 PM$
*/

void _sstYab422to8888(NccTable *ncc, unsigned char *c888, unsigned char __yiq);
void _sstAi44to8888 (unsigned char c8888[4], unsigned char c44);
void _sstRgba332to8888 (unsigned char c8888[4], unsigned char c332);
void _sstRgba565to8888 (unsigned char c8888[4], unsigned short c565);
void _sstRgba555to8888 (unsigned char c8888[4], unsigned short c555);
void _sstRgba1555to8888 (unsigned char c8888[4], unsigned short c1555);
void _sstRgba4444to8888 (unsigned char c8888[4], unsigned short c4444);

void _sstPal256to8888(Pal256 *pal, unsigned char *c8888, unsigned char index);
void _sstPal6666to8888(Pal256 *pal, unsigned char *c8888, unsigned char index);

void _sstRgba8888to565 (unsigned short *c565, unsigned long c8888);
void _sstRgba8888to1555 (unsigned short *c565, unsigned long c8888);

unsigned long _sstRgbaLanes8888(SstRegs *sst, unsigned long data);
unsigned short _sstRgbaLanes565(SstRegs *sst, unsigned long data);
unsigned short _sstRgbaLanes1555(SstRegs *sst, unsigned long data);

int _sstDit5(int,int,int mask);
int _sstDit6(int,int,int mask);

#endif	/* !__RGBFMT_H__ */
