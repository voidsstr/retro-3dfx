;/* $Header: /devel/h3/Win95/dx/dd16/convert.asm 2     12/28/98 11:23a Michael $ */
;/*
;** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
;** File name:   convert.asm
;**
;** Description: Functions to convert Floats/Longs to Integers.
;**              Primarily (only?) used for working on monitor refresh
;**              rates extracted from the registry.
;**
;** $Revision: 2 $
;** $Date: 12/28/98 11:23a $
;**
;** $History: convert.asm $
;; 
;; *****************  Version 2  *****************
;; User: Michael      Date: 12/28/98   Time: 11:23a
;; Updated in $/devel/h3/Win95/dx/dd16
;; Added the 3Dfx/STB unified file/funciton header.  Add VSS keywords to
;; files where this was missing.
;** 
;**
;*/

        .386
        _TEXT	SEGMENT	DWORD PUBLIC USE16 'CODE'
        _TEXT	ENDS

tab	equ	09h

;----------------------------------------------------------------------------
; C O D E
;----------------------------------------------------------------------------
_TEXT	SEGMENT
	assume	cs:_TEXT

;*----------------------------------------------------------------------
;Function name:  fatoi
;
;Description:    Converts an ASCII value to an Integer.
;
;Information:
;
;Return:         AX     Integer value
;----------------------------------------------------------------------*
	align	4
fatoi	PROC FAR C PUBLIC USES ds si, string:FAR PTR BYTE

	xor	bx,bx		; initialize result
	xor	cx,cx		; initialize sign flag
	lds	si,string	; initialize ptr

	; scan off leading white space
fatoi1:
	lodsb
	cmp	al,' '
	je	fatoi1
	cmp	al,tab
	je	fatoi1

	cmp	al,'+'		; if plus sign, go get next char
	je	fatoi2
	cmp	al,'-'		; if not negative, go use char
	jne	fatoi3
	dec	cx		; set sign flag for negative

fatoi2:
	lodsb
fatoi3:
	; see if char is a number
	cmp	al,'0'
	jb	fatoi4
	cmp	al,'9'
	ja	fatoi4

	and	ax,0Fh

	; mult previous answer by 10
	mov	dx,bx
	add	bx,bx		; * 2
	add	bx,bx		; * 4
	add	bx,dx		; * 5
	add	bx,bx		; * 10

	; add current value
	add	bx,ax
	jmp	fatoi2

fatoi4:
	mov	ax,bx
	cmp	cx,0
	je	fatoi5
	neg	ax

fatoi5:
	ret

fatoi	ENDP


;*----------------------------------------------------------------------
;Function name:  fatol
;
;Description:    Converts an ASCII value to an Long.
;
;Information:
;
;Return:         DX:AX  Long value
;----------------------------------------------------------------------*
	align	4
fatol	PROC FAR C PUBLIC USES ds si di, string:FAR PTR BYTE

	
	xor	bx,bx		; initialize result in dx:bx
	xor	dx,dx
	xor	cx,cx		; initialize sign flag
	lds	si,string	; initialize ptr

	; scan off leading white space
fatol1:
	lodsb
	cmp	al,' '
	je	fatol1
	cmp	al,tab
	je	fatol1

	cmp	al,'+'		; if plus sign, go get next char
	je	fatol2
	cmp	al,'-'		; if not negative, go use char
	jne	fatol3
	dec	cx		; set sign flag for negative

fatol2:
	lodsb
fatol3:
	; see if char is a number
	cmp	al,'0'
	jb	fatol4
	cmp	al,'9'
	ja	fatol4

	and	ax,0Fh
	push	ax		; save current value

	mov	ax,bx		; save previous answer in di:ax
	mov	di,dx

	; mult previous answer by 10
	add	bx,bx		; * 2
	adc	dx,dx
	add	bx,bx		; * 4
	adc	dx,dx
	add	bx,ax		; * 5
	adc	dx,di
	add	bx,bx		; * 10
	adc	dx,dx

	; add current value
	pop	ax
	add	bx,ax
	adc	dx,0

	jmp	fatol2

fatol4:
	mov	ax,bx
	cmp	cx,0
	je	fatol5

	not	ax
	not	dx
	add	ax,1
	adc	dx,0

fatol5:
	ret

fatol	ENDP


;*----------------------------------------------------------------------
;Function name:  fatoi10x
;
;Description:    Converts an ASCII float to an Integer.
;
;Information:
;
;Return:         AX  Integer value
;----------------------------------------------------------------------*
	align	4
fatoi10x	PROC FAR C PUBLIC USES ds si, string:FAR PTR BYTE
		LOCAL	DecPtFound:WORD

	mov	DecPtFound,0

	xor	bx,bx		; initialize result
	xor	cx,cx		; initialize sign flag
	lds	si,string	; initialize ptr

	; scan off leading white space
fatoi10x1:
	lodsb
	cmp	al,' '
	je	fatoi10x1
	cmp	al,tab
	je	fatoi10x1

	cmp	al,'+'		; if plus sign, go get next char
	je	fatoi10x2
	cmp	al,'-'		; if not negative, go use char
	jne	fatoi10x3
	dec	cx		; set sign flag for negative

fatoi10x2:
	lodsb
fatoi10x3:
	cmp	DecPtFound,0
	jne	fatoi10x4

	; see if it's the decimal pt
	cmp	al,'.'
	jne	fatoi10x4

	inc	DecPtFound
	lodsb

	; see if char is a number
	cmp	al,'0'
	jb	@f
	cmp	al,'9'
	jbe	fatoi10x4

@@:
	mov	al,'0'

fatoi10x4:
	; see if char is a number
	cmp	al,'0'
	jb	fatoi10x5
	cmp	al,'9'
	ja	fatoi10x5

	and	ax,0Fh

	; mult previous answer by 10
	mov	dx,bx
	add	bx,bx		; * 2
	add	bx,bx		; * 4
	add	bx,dx		; * 5
	add	bx,bx		; * 10

	; add current value
	add	bx,ax

	cmp	DecPtFound,1
	je	fatoi10x5

	jmp	fatoi10x2

fatoi10x5:
	cmp	DecPtFound,1
	je	fatoi10x6

	inc	DecPtFound
	mov	al,'0'
	jmp	fatoi10x4

fatoi10x6:
	mov	ax,bx
	cmp	cx,0
	je	fatoi10x7
	neg	ax

fatoi10x7:
	ret

fatoi10x	ENDP


;*----------------------------------------------------------------------
;Function name:  fatol10x
;
;Description:    Converts an ASCII float to an Long.
;
;Information:
;
;Return:         DX:AX  Long value
;----------------------------------------------------------------------*
	align	4
fatol10x	PROC FAR C PUBLIC USES ds si di, string:FAR PTR BYTE
		LOCAL	DecPtFound:WORD

	mov	DecPtFound,0

	xor	bx,bx		; initialize result
	xor	dx,dx
	xor	cx,cx		; initialize sign flag
	lds	si,string	; initialize ptr

	; scan off leading white space
fatol10x1:
	lodsb
	cmp	al,' '
	je	fatol10x1
	cmp	al,tab
	je	fatol10x1

	cmp	al,'+'		; if plus sign, go get next char
	je	fatol10x2
	cmp	al,'-'		; if not negative, go use char
	jne	fatol10x3
	dec	cx		; set sign flag for negative

fatol10x2:
	lodsb
fatol10x3:
	cmp	DecPtFound,0
	jne	fatol10x4

	; see if it's the decimal pt
	cmp	al,'.'
	jne	fatol10x4

	inc	DecPtFound
	lodsb

	; see if char is a number
	cmp	al,'0'
	jb	@f
	cmp	al,'9'
	jbe	fatol10x4

@@:
	mov	al,'0'

fatol10x4:
	; see if char is a number
	cmp	al,'0'
	jb	fatol10x5
	cmp	al,'9'
	ja	fatol10x5

	and	ax,0Fh
	push	ax		; save current value

	mov	ax,bx		; save previous answer in di:ax
	mov	di,dx

	; mult previous answer by 10
	add	bx,bx		; * 2
	adc	dx,dx
	add	bx,bx		; * 4
	adc	dx,dx
	add	bx,ax		; * 5
	adc	dx,di
	add	bx,bx		; * 10
	adc	dx,dx

	; add current value
	pop	ax
	add	bx,ax
	adc	dx,0

	cmp	DecPtFound,1
	je	fatol10x5

	jmp	fatol10x2

fatol10x5:
	cmp	DecPtFound,1
	je	fatol10x6

	inc	DecPtFound
	mov	al,'0'
	jmp	fatol10x4

fatol10x6:
	mov	ax,bx
	cmp	cx,0
	je	fatol10x7

	not	ax
	not	dx
	add	ax,1
	adc	dx,0

fatol10x7:
	ret

fatol10x	ENDP


;*----------------------------------------------------------------------
;Function name:  fhtol
;
;Description:    Converts an ASCII Hex to an Integer.
;
;Information:
;
;Return:         AX  Integer value
;----------------------------------------------------------------------*
	align	4
fhtol	PROC FAR C PUBLIC USES ds si di, string:FAR PTR BYTE

	
	xor	bx,bx		; initialize result in dx:bx
	xor	dx,dx
	lds	si,string	; initialize ptr

	; scan off leading white space
fhtol1:
	lodsb
	cmp	al,' '
	je	fhtol1
	cmp	al,tab
	je	fhtol1

	jmp	fhtol3

fhtol2:
	lodsb
fhtol3:

	; see if char is a hex number
	cmp	al,'0'
	jb	fhtol5
	cmp	al,'9'
	jbe	fhtol4
	or	al,'a'-'A'
	cmp	al,'f'
	ja	fhtol5
	cmp	al,'a'
	jb	fhtol5
	add	al,9

fhtol4:
	; mult previous answer by 16
	shl	bx,1
	rcl	dx,1
	shl	bx,1
	rcl	dx,1
	shl	bx,1
	rcl	dx,1
	shl	bx,1
	rcl	dx,1

	; add current value
	and	ax,0Fh
	or	bx,ax

	jmp	fhtol2

fhtol5:
	mov	ax,bx

	ret

fhtol	ENDP

_TEXT	ENDS
END
