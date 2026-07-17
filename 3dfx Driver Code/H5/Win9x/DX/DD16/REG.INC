/* $Header:  $ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** File name:   reg.inc
**
** Description: Contains defines for SST1 reg offsets since
**              asm module can't use sst1.h
**
** $Revision: $
** $Date: $
**
** $History: $
** 
*/

EXTRN _sel:WORD
EXTRN _base_ptr:DWORD
EXTRN _tex_ptr:DWORD
EXTRN _fb_ptr:DWORD



;/*----------------------------------------------------------------------*/
;/* Assembler offsets for SST struct					*/
;/*----------------------------------------------------------------------*/

STATUS		EQU        00000000h
RESERVED0	EQU        00000004h
VA_X		EQU        00000008h
VA_Y		EQU        0000000ch
VB_X		EQU        00000010h
VB_Y		EQU        00000014h
VC_X		EQU        00000018h
VC_Y		EQU        0000001ch

R		EQU        00000020h
G		EQU        00000024h
B		EQU        00000028h
Z		EQU        0000002ch
A		EQU        00000030h
S		EQU        00000034h
T		EQU        00000038h
W		EQU        0000003ch

DRDX		EQU        00000040h
DGDX		EQU        00000044h
DBDX		EQU        00000048h
DZDX		EQU        0000004ch
DADX		EQU        00000050h
DSDX		EQU        00000054h
DTDX		EQU        00000058h
DWDX		EQU        0000005ch

DRDY		EQU        00000060h
DGDY		EQU        00000064h
DBDY		EQU        00000068h
DZDY		EQU        0000006ch
DADY		EQU        00000070h
DSDY		EQU        00000074h
DTDY		EQU        00000078h
DWDY		EQU        0000007ch

TRIANGLECMD	EQU        00000080h
RESERVED1	EQU        00000084h
FVA_X		EQU        00000088h
FVA_Y		EQU        0000008ch
FVB_X		EQU        00000090h
FVB_Y		EQU        00000094h
FVC_X		EQU        00000098h
FVC_Y		EQU        0000009ch

F_R		EQU        000000a0h
F_G		EQU        000000a4h
F_B		EQU        000000a8h
F_Z		EQU        000000ach
F_A		EQU        000000b0h
F_S		EQU        000000b4h
F_T		EQU        000000b8h
F_W		EQU        000000bch

FDRDX		EQU        000000c0h
FDGDX		EQU        000000c4h
FDBDX		EQU        000000c8h
FDZDX		EQU        000000cch
FDADX		EQU        000000d0h
FDSDX		EQU        000000d4h
FDTDX		EQU        000000d8h
FDWDX		EQU        000000dch

FDRDY		EQU        000000e0h
FDGDY		EQU        000000e4h
FDBDY		EQU        000000e8h
FDZDY		EQU        000000ech
FDADY		EQU        000000f0h
FDSDY		EQU        000000f4h
FDTDY		EQU        000000f8h
FDWDY		EQU        000000fch

FTRIANGLECMD	EQU        00000100h
FBZCOLORPATH	EQU        00000104h
FOGMODE		EQU        00000108h
ALPHAMODE	EQU        0000010ch
FBZMODE		EQU        00000110h
LFBMODE		EQU        00000114h
CLIPLEFTRIGHT	EQU        00000118h
CLIPBOTTOMTOP	EQU        0000011ch

NOPCMD		EQU        00000120h
FASTFILLCMD	EQU        00000124h
SWAPBUFFERCMD	EQU        00000128h
FOGCOLOR	EQU        0000012ch
ZACOLOR		EQU        00000130h
CHROMAKEY	EQU        00000134h
RESERVED2	EQU        00000138h
RESERVED3	EQU        0000013ch

STIPPLE		EQU        00000140h
C0		EQU        00000144h
C1		EQU        00000148h
FBIPIXELSIN	EQU        0000014ch
FBICHROMAFAIL	EQU        00000150h
FBIZFUNCFAIL	EQU        00000154h
FBIAFUNCFAIL	EQU        00000158h
FBIPIXELSOUT	EQU        0000015ch

FOGTABLE	EQU        00000160h

FBIINIT4	EQU        00000200h
VRETRACE	EQU        00000204h
BACKPORCH	EQU        00000208h
VIDEODIMENSIONS	EQU        0000020ch
FBIINIT0	EQU        00000210h
FBIINIT1	EQU        00000214h
FBIINIT2	EQU        00000218h
FBIINIT3	EQU        0000021ch

HSYNC		EQU        00000220h
VSYNC		EQU        00000224h
CLUTDATA	EQU        00000228h
DACDATA		EQU        0000022ch
VIDEOFILTERRGBTHRESHOLD	EQU        00000230h

TEXTUREMODE	EQU        00000300h
TLOD		EQU        00000304h
TDETAIL		EQU        00000308h
TEXBASEADDR	EQU        0000030ch
TEXBASEADDR1	EQU        00000310h
TEXBASEADDR2	EQU        00000314h
TEXBASEADDR38	EQU        00000318h
TREXINIT0	EQU        0000031ch

TREXINIT1	EQU        00000320h
NCCTABLE0	EQU        00000324h
NCCTABLE1	EQU        00000354h

SIZEOF_SST	EQU        00000384h
