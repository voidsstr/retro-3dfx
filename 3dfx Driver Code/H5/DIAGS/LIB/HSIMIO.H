#ifndef __HSIMIO_H__
#define __HSIMIO_H__

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
** $Date: 10/11/00 8:11:54 PM$
*/


//////////////////////////////////////////////////////////////
//
//                   CSIM IO functions
//
//////////////////////////////////////////////////////////////
FxU32 CSIM_PIXEL_RD(FxI32 buffer, int x, int y);
void CSIM_PIXEL_WR(FxI32 buffer, int x, int y, FxU32 data);

FxU32 CSIM_MEM_RD(int addr, int nbytes);
void CSIM_MEM_WR(int addr, FxU32 data, int nbytes);

FxU32 csimPixelRead(FxI32 sampleIndex, FxI32 buffer, FxI32 x, FxI32 y);
FxU32 csimPixelReadCompositeBuffer(FxI32 buffer, int x, int y);
void  csimPixelWrite(FxI32 sampleIndex, FxI32 buffer, FxI32 x, FxI32 y, FxU32 data);

FxU32 csimMemoryRead(FxI32 chipIndex, FxI32 addr, FxI32 nBytes);
void  csimMemoryWrite(FxI32 chipIndex, FxI32 addr, FxU32 data, FxI32 nBytes);



//////////////////////////////////////////////////////////////
//
//                   HSIM IO functions
//
//////////////////////////////////////////////////////////////
FxU32 HSIM_PIXEL_RD(FxI32 buffer, int x, int y);
void HSIM_PIXEL_WR(FxI32 buffer, int x, int y, FxU32 data);

FxU32 HSIM_MEM_RD(int addr, int nbytes);
void HSIM_MEM_WR(int addr, FxU32 data, int nbytes);

FxU32 hsimPixelRead(FxI32 sampleIndex, FxI32 buffer, FxI32 x, FxI32 y);
FxU32 hsimPixelReadCompositeBuffer(FxI32 buffer, int x, int y);
void  hsimPixelWrite(FxI32 sampleIndex, FxI32 buffer, FxI32 x, FxI32 y, FxU32 data);

FxU32 hsimMemoryRead(FxI32 chipIndex, FxI32 addr, FxI32 nBytes);
void  hsimMemoryWrite(FxI32 chipIndex, FxI32 addr, FxU32 data, FxI32 nBytes);


//////////////////////////////////////////////////////////////
//
//                   HW IO functions
//
//////////////////////////////////////////////////////////////
void HW_PIXEL_FAST_BEGIN(FxU32 chipIndex, FxI32 buffer);
void HW_PIXEL_FAST_END(FxU32 chipIndex);

FxU32 HW_PIXEL_RD(FxI32 buffer, int x, int y);
void HW_PIXEL_WR(FxI32 buffer, int x, int y, FxU32 data);

FxU32 HW_MEM_RD(FxU32 chipIndex, int addr, int nbytes);
void HW_MEM_WR(FxU32 chipIndex, int addr, FxU32 data, int nbytes);

void HW_MEM_RD2(int addr, FxU8 *data, int nbytes);
void HW_MEM_WR2(int addr, FxU8 *data, int nbytes);

FxU32 HW_MEM_CMP2(FxU32 chipIndex, int addr, FxU8 *data, int nbytes);

#endif /* !__HSIMIO_H__ */
