/* -*-c++-*- */
/* $Header: TILED.H, 2, 10/11/00 8:53:45 PM, Brent$ */
/*
** Copyright (c) 1998-1999, 3Dfx Interactive, Inc.
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
** File name:   tiled.h
**
** Description: support for linear/tiled promotion/demotion.
**
** $Revision: 2$
** $Date: 10/11/00 8:53:45 PM$
**
** $History: TILED.H $
** 
** *****************  Version 3  *****************
** User: Michael      Date: 1/15/99    Time: 9:25a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** 2     4/18/98 11:02a Michael
** More work to get gdi to work with tiled mode.
** 
** 1     4/16/98 8:15p Michael
** Support macros to handle drawing when in tiled mode.
*/

#define TILE_BIT  0x80000000L

#define _DD(Field) ((lpDriverData)->Field)

#define IS_TILED_AND_EXCLUSIVE \
  ((_DD(ddExclusiveMode) == TRUE) && \
   (_DD(ddPrimaryInTile) == TRUE)) ? 1: 0

#define IS_DEV_BITMAP(lp) \
  ((VRAM|OFFSCREEN) == ((lp)->deFlags & (VRAM|OFFSCREEN))) ? 1: 0

#define IS_DESKTOP(ptr) \
  ((ptr) == (_DD(gdiDesktopStart))) ? 1: 0

#define IS_ONSCREEN(lp) \
  (VRAM == ((lp)->deFlags & (VRAM|OFFSCREEN))) ? 1: 0

#define MAKE_DST_TILED(dstAddr, dstFormat) \
   { \
   dstAddr |= TILE_BIT; \
   dstFormat &= 0xFFFFC000L; \
   dstFormat |= (((lpDriverData->ddTileStride) << SSTG_DST_STRIDE_SHIFT) \
      & SSTG_DST_LINEAR_STRIDE); \
   }

#define MAKE_SRC_TILED(srcAddr, srcFormat) \
   { \
   srcAddr |= TILE_BIT; \
	srcFormat &= ~(SSTG_SRC_PACK); \
   srcFormat &= 0xFFFFC000L; \
   srcFormat |= (((lpDriverData->ddTileStride) << SSTG_SRC_STRIDE_SHIFT) \
      & SSTG_SRC_LINEAR_STRIDE); \
   }
