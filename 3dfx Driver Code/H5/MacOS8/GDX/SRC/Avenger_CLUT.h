#ifndef __AVENGER_CLUT_H__
#define __AVENGER_CLUT_H__

/*
** Copyright (c) 1996-1999, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   avenger_CLUT.h
**
**
** $Header: Avenger_CLUT.h, 3, 10/11/00 8:33:43 PM, Brent$
**
** $History: avenger_CLUT.h $
** 
** *****************  Version 1  *****************
** User: Kcd          Date: 6/03/99    Time: 6:45p
** Created in $/devel/h3/MacOS8/GDX/src
** MacOS 8 2D Display Driver
** 
**
*/

#define RED_SHIFT       16
#define GREEN_SHIFT     8
#define BLUE_SHIFT      0

GDXErr	AvengerMapDepthModeToCLUTAttributes(DepthMode depthMode, UInt32 *startAddress, UInt32 *entryOffset);
void	AvengerProgramCLUT(void);
void AvengerProgramCLUTColor(FxU32 color);
void	SetDAC555(FxU32 regBase, FxU32 enable);

#endif /* __AVENGER_CLUT_H__ */