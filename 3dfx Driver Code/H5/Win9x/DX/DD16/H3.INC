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
** File name:   h3.inc
**
** Description: Support EQUATES for h3.
**              Does not appear to be used!
**
** $Revision: 9 $
** $Date: 12/28/98 11:23a $
**
** $History: cursor.h $
** 
** *****************  Version 9  *****************
** User: Michael      Date: 12/28/98   Time: 11:23a
** Updated in $/devel/h3/Win95/dx/dd16
** Added the 3Dfx/STB unified file/funciton header.  Add VSS keywords to
** files where this was missing.
**
*/

MAX_GLYPHS 		equ	256
MAX_GLYPH_HEIGHT	equ	32
MAX_GLYPH_WIDTH		equ	MAX_GLYPH_HEIGHT
MAX_CHAR_HEIGHT		equ     <32>

DEF_STACK MACRO ProcName
ProcName PROC FAR PASCAL <LOADDSGS> USES ESI EDI,
	lpDestDev:FAR PTR,
	xDest:WORD,			;x,y Origin
	yDest:WORD,
	lpClipRect:FAR PTR,		;Clipping rectangle
	lpString:FAR PTR,		;The string itself
	cChars:WORD,			;chars in string
	lpFont:FAR PTR, 		;Font to use
	lpDrawMode:FAR PTR,
	lpTransform:FAR PTR,
	lpDX:FAR PTR,			;Widths for the characters
	lpOpaqueRect:FAR PTR,		;Opaquing rectangle
	wOptions:WORD			;ExtTextOut options


  IFDIFI <ProcName>,<EExtTextOut>
	nop
	org	offset ProcName
  ENDIF
ENDM

END_STACK MACRO ProcName
	local	proc_end
proc_end label near
	ProcName ENDP
	org	offset proc_end
ENDM
;;

; EQUATES For ExtTextOut

	lpDestDev       EQU      [bp+2a]
	xDest           EQU      [bp+28]
	yDest           EQU      [bp+26]
	lpClipRect      EQU      [bp+22]
	lpString        EQU      [bp+1E]
	cChars          EQU      [bp+1C]
	lpFont          EQU      [bp+18]
	lpDrawMode      EQU      [bp+14]
	lpTransform     EQU      [bp+10]
	lpDX            EQU      [bp+C]
	lpOpaqueRect    EQU      [bp+A]
	wOptions        EQU      [bp+6]

; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_DATA   SEGMENT PAGE PUBLIC 'DATA'

_DATA   ENDS

