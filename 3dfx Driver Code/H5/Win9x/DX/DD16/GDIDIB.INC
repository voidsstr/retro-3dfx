;/* $Header:  $ */
;/*
;** Copyright (c) 1995-1998, 3Dfx Interactive, Inc.
;** All Rights Reserved.
;**
;** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
;** the contents of this file may not be disclosed to third parties, copied or
;** duplicated in any form, in whole or in part, without the prior written
;** permission of 3Dfx Interactive, Inc.
;**
;** RESTRICTED RIGHTS LEGEND:
;** Use, duplication or disclosure by the Government is subject to restrictions
;** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
;** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
;** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
;** rights reserved under the Copyright Laws of the United States.
;**
;**
;** $Revision: $
;** $Date: $
;**
;** $History: $
;; 
;; 
;**
;*/

;----------------------------------------------------------------------------
;
; gdidib.inc
;
; local macro for elements in the gdidefs.inc and dibeng.inc
;
;----------------------------------------------------------------------------

        .xlist
        include dibeng.inc
        .list

; STB Begin change
; STB-SR 12/16/98 Added for STB Performance optimizations
ifdef INCSTBPERF
INCLUDE ..\..\BUILD\STBPERF.INC
endif
; STB END change

;
; Things I grabbed from minivdd or other places and didn't know where to put
;


; flags for BitBlt
BS_SOLID    equ 0
BS_HOLLOW   equ 1
BS_HATCHED  equ 2
BS_PATTERN  equ 3

; flags for extended textout

ETO_GRAYED  equ 0001h
ETO_OPAQUE  equ 0002h
ETO_CLIPPED equ 0004h
ETO_GLYPH_INDEX equ 0010h
ETO_RTL     equ 0080h
ETO_BYTE_PACKED equ 0100h
ETO_BIT_PACKED  equ 0200h
ETO_LEVEL_MODE  equ 1000h


;------------------------------------------------------------------------------
;--------------------------  4.0 font structures  -----------------------------
;------------------------------------------------------------------------------
; This font format has a simple font header. It contains a version field 
; followed by a glyph format field, the total number of glyphs in the 
; entire font, and the dword offset to the array of glyph offsets.
;------------------------------------------------------------------------------

;------------------------------------------------------------------------------
; The NF_LARGE flag indicates that the glyph offset table is
;  array of DWORDs instead of WORD offsets AND that the glyphs
;  have LARGExxxGLYPH headers instead of SMALLxxxGLYPH headers.
;------------------------------------------------------------------------------
NF_BYTE_PACKED  equ 0001h       ; Byte aligned row format
NF_BIT_PACKED   equ 0002h       ; Bit packed row format
NF_FROM_BMP equ 0040h       ; Derived from bitmap font
NF_LARGE    equ 0080h       ; Large offsets and glyph headers

NewFontSeg  struct
 nfVersion  dw  0       ; Font version (0x0400)
 nfFormat   dw  0       ; Font format flags
 nfNumGlyphs    dw  0       ; Number of glyphs in font
 nfGlyphOffset  dd  0       ; Offset to glyph offset table
 nfAWTable  dd  0       ; Offset to advance width table

 nfHeight   dw  0       ; Height of font in pixels
 nfAscent   dw  0       ; Distance from baseline to top
 nfUniqueID dd  0       ; Unique identifier for this PFont
NewFontSeg  ends


;----------------------  Byte-packed glyph structures  ------------------------
;
; This structure is the header for a glyph stored in byte-packed 
; row-oriented format. The glyph bitmap is stored immediately after 
; the ROWGLYPH structure.
;------------------------------------------------------------------------------
SMALLROWGLYPH   struc
 srgOrgX    db  0       ; Vector to glyph origin in bitmap
 srgOrgY    db  0       ;  from upper left corner
 srgWidth   db  0       ; Width and height of glyph bmp in
 srgHeight  db  0       ;  pixels
SMALLROWGLYPH   ends

LARGEROWGLYPH   struc
 lrgOrgX    dw  0       ; Vector to glyph origin in bitmap
 lrgOrgY    dw  0       ;  from upper left corner
 lrgWidth   dw  0       ; Width and height of glyph bmp in
 lrgHeight  dw  0       ;  pixels
LARGEROWGLYPH   ends

;----------------------  Bit-packed glyph structures  -------------------------
;
; This structure is the header for a glyph stored in bit-packed row-oriented 
; format. The glyph bitmap is stored immediately after the BITGLYPH structure.
;
; Bit-packed format means that the glyph rows are packed together end-to-end. 
; There is no padding out to byte boundaries at the end of a row. This
; format results in the highest performance on some accelerated cards and 
; the smallest memory footprint.
;------------------------------------------------------------------------------
SMALLBITGLYPH   struc
 sbgOrgX    db  0       ; Vector to glyph origin in bitmap
 sbgOrgY    db  0       ;  from upper left corner
 sbgWidth   db  0       ; Width and height of glyph bmp in
 sbgHeight  db  0       ;  pixels
 sbgPixels  dw  0       ; Total # of pixels in the glyph
SMALLBITGLYPH   ends

LARGEBITGLYPH   struc
 lbgOrgX    dw  0       ; Vector to glyph origin in bitmap
 lbgOrgY    dw  0       ;  from upper left corner
 lbgWidth   dw  0       ; Width and height of glyph bmp in
 lbgHeight  dw  0       ;  pixels
 lbgPixels  dd  0       ; Total # of pixels in the glyph
LARGEBITGLYPH   ends

