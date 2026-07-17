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
** $Date: 10/11/00 8:19:20 PM$
*/

int texRandomZAbuffer(SstRegs *sst, unsigned long texMode);
int texFormatHasAlpha(unsigned long texMode);
int texFormatHasPalette(unsigned long texMode);
int texRandomFormat(int t16, int t32, int ncc);
void texRandomNccTable(SstRegs *sst, Texture *tx);
void texRandomTextureMap(SstRegs *sst, int trex, int mipmaps,
			 int w, int h, Texture *tx);
void texRandomTextureMapEx(SstRegs *sst, int trex, int mipmaps,
			 int w, int h, Texture *tx, int addrHint);
void texRandomTextureMapNoOverlap(SstRegs *sst, int trex, int mipmaps,
				  int w, int h, Texture *tx, 
				  FxU32 *start, FxU32 *end);
void texTo8888(unsigned char *rgba, Triangle *t, int x,int y);

void printStwTriangle(int level, Triangle *t);
void printStwTriangleSlopes(int level, Triangle *t);
void randomStTriangle(Triangle *t);
void randomSt1418Triangle(Triangle *t);
void randomW230Triangle(Triangle *t);
void drawStwTriangle(SstRegs *sst, Triangle *t);
void drawStwTriangle2(SstRegs *sst, Triangle *t, Triangle *t2);
int setupStwTriangle(Triangle *t);
int insideStTriangle(Triangle *t, int x, int y, unsigned long col);
