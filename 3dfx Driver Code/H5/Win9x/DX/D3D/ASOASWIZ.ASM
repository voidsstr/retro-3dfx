TITLE	swizzle.asm
.686P
.MMX
.K3D
.XMM

_TEXT	SEGMENT PARA USE32 PUBLIC 'CODE'
_TEXT	ENDS
_DATA	SEGMENT DWORD USE32 PUBLIC 'DATA'
_DATA	ENDS
CONST	SEGMENT DWORD USE32 PUBLIC 'CONST'
CONST	ENDS
_BSS	SEGMENT DWORD USE32 PUBLIC 'BSS'
_BSS	ENDS
$$SYMBOLS	SEGMENT BYTE USE32 'DEBSYM'
$$SYMBOLS	ENDS
$$TYPES	SEGMENT BYTE USE32 'DEBTYP'
$$TYPES	ENDS
_TLS	SEGMENT DWORD USE32 PUBLIC 'TLS'
_TLS	ENDS
_TEXT	SEGMENT PARA USE32 PUBLIC 'CODE'
_TEXT	ENDS
ASSUME	CS: FLAT, DS: FLAT, SS: FLAT

INCLUDE aglobal.inc

_TEXT	SEGMENT

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                               SWIZZLE-SINGLE CODE DESCRIPTION
;;
;; We need to swizzle 4 FVF's to a single SOA_FVF.  We swizzle the position, blend factor('s), normal, and colors.  The textures are
;; swizzled but the u/v  coordinates are kept together.  
;; 
;; In soaswoz.c there's a function, SwizzleSingleD3DtoSOA_Setup(), which figures out what type of FVF we have.  There's a huge 
;; combination of FVF's so to special case all of them would be impratical.  However, we can combine many of these.  
;; SwizzleSingleD3DtoSOA_Setup() figures out how many parameters are required for position, blend factor('s), normal, and colors 
;; (everything but textures).  This value will always be between 3 and 13 so 11 separate special-case functions will swizzle any 
;; FVF. Then there can be from 0 to 8 textures so that this requires 9 more functions.  
;; 
;; The way this is implemented is SwizzleSingleD3DtoSOA_Setup() loads two pointers to the appropriate functions into the RC, 
;; KniSwizParmsFunct and KniCpyTxtrFunct.  When a single-swizzle is required an indirect call is made to the function pointed to 
;; by KniSwizParmsFunct.  This function swizzles everything but the textures and then does an indirect jump to the function pointed 
;; to by KniCpyTxtrFunct.  Then this function swizzles the textures and returns.  This will not hose up the return stack in the cpu. 
;; SwizzleSingleD3DtoSOA_Setup() requires the 2 function lookup tables in the CONST section of this file.
;; 
;; Below are a bunch of macros that swizzle from 1 to 8 parameters or copy/swizzle 1 to 4 textures.  The swizzle functions are used 
;; by 11 functions that swizzle from 3 to 13 parameters from 4 FVF's to 1 SOA_FVF.  All macros update the source and destination 
;; pointers, except where noted.
;; 
;; Prefetching is done where it makes sense, i.e. I didn't prefetch with less than 3 parameters.  The macros prefetch the same  
;; parameters that they are swizzling from the 4 FVF's after the current one.  This seems to work well as long as the indicies are 
;; fairly linear.  If they start jumping backwards to FVF's that we haven't seen yet, cache-misses will occur.  Also, the vertex 
;; buffers that I tested fit in the L2 cache, i.e. without prefetching there were many L1 cache misses but few L2 cache misses.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


SWIZZLE1	MACRO
	movss		xmm6, dword ptr [esi+000h]		   	;   0   0   0   a0
	lea			esi, [esi+edx]
	movhps		xmm6, qword ptr [esi+000h]	   		;   --  b0  0   a0
	add			edi, 1*4*4
	movss		xmm7, dword ptr [esi+edx+000h]   	;   0   0   0   c0
	movhps		xmm7, qword ptr [esi+edx*2+000h]   	;   --  d0  0   c0
	sub			esi, edx							; correct esi
	shufps		xmm6, xmm7, 088h					;	d0  c0  b0  a0
	add			esi, 1*4*1
	movups		oword ptr [edi-0010h], xmm6			;	d0  c0  b0  a0
ENDM

SWIZZLE2	MACRO
	movlps		xmm0, qword ptr [esi+000h]			;	--	--	a1	a0
	lea			esi, [esi+edx]
	movhps		xmm0, qword ptr [esi+000h]			;	b1  b0  a1	a0
	add			edi, 2*4*4
	movlps		xmm1, qword ptr [esi+edx+000h]		;	--	--	c1	c0
	shufps		xmm0, xmm0, 0d8h					;	b1  a1  b0  a0
	movhps		xmm1, qword ptr [esi+edx*2+000h]		;	d1  d0  c1	c0
	shufps		xmm1, xmm1, 0d8h					;	d1  c1  d0  c0
	movlps		qword ptr [edi-0020h], xmm0			;	b0  a0
	sub			esi, edx							; correct esi
	movhps		qword ptr [edi-0010h], xmm0			;	b1  a1
	add			esi, 2*4*1
	movlps		qword ptr [edi-0018h], xmm1			;	d0  c0
	movhps		qword ptr [edi-0008h], xmm1			;	d1  c1
ENDM

SWIZZLE3	MACRO
	movlps		xmm0, qword ptr [esi+000h]			;	--	--	a1	a0
	movss		xmm6, dword ptr [esi+008h]			;   0   0   0   a2
	lea			esi, [esi+edx]
	movhps		xmm0, qword ptr [esi+000h]			;	b1  b0  a1	a0
	add			edi, 3*4*4
	movhps		xmm6, qword ptr [esi+008h]			;   --  b2  0   a2
	movlps		xmm1, qword ptr [esi+edx+000h]		;	--	--	c1	c0
	movss		xmm7, dword ptr [esi+edx+008h]		;   0   0   0   c2
	shufps		xmm0, xmm0, 0d8h					;	b1  a1  b0  a0
	movhps		xmm1, qword ptr [esi+edx*2+000h]	;	d1  d0  c1	c0
	movhps		xmm7, qword ptr [esi+edx*2+008h]	;   --  d2  0   c2
	shufps		xmm1, xmm1, 0d8h					;	d1  c1  d0  c0
	movlps		qword ptr [edi-0030h], xmm0			;	b0  a0
	sub			esi, edx							; correct esi
	movhps		qword ptr [edi-0020h], xmm0			;	b1  a1
	shufps		xmm6, xmm7, 088h					;	d2  c2  b2  a2
	movlps		qword ptr [edi-0028h], xmm1			;	d0  c0
	add			esi, 3*4*1
	movhps		qword ptr [edi-0018h], xmm1			;	d1  c1
	movups		oword ptr [edi-0010h], xmm6			;	d2  c2  b2  a2
ENDM

SWIZZLE4	MACRO
	movlps		xmm0, qword ptr [esi+000h]			;	--	--	a1	a0
	lea			ebx, [esi+edx*4]					; ebx = addr of next next 4 FVF
	movlps		xmm2, qword ptr [esi+008h]			;	--	--	a3	a2
	lea			esi, [esi+edx]
	movhps		xmm0, qword ptr [esi+000h]			;	b1  b0  a1	a0
	add			edi, 4*4*4
	movhps		xmm2, qword ptr [esi+008h]			;	b3  b2  a3	a2
	movlps		xmm1, qword ptr [esi+edx+000h]		;	--	--	c1	c0
	movlps		xmm3, qword ptr [esi+edx+008h]		;	--	--	c3	c2
	shufps		xmm0, xmm0, 0d8h					;	b1  a1  b0  a0
	movhps		xmm1, qword ptr [esi+edx*2+000h]	;	d1  d0  c1	c0
	shufps		xmm2, xmm2, 0d8h					;	b3  a3  b2  a2
	movhps		xmm3, qword ptr [esi+edx*2+008h]	;	d3  d2  c3	c2
	prefetchnta [ebx]
	movlps		qword ptr [edi-0040h], xmm0			;	b0  a0
	add			ebx, edx
	movhps		qword ptr [edi-0030h], xmm0			;	b1  a1
	shufps		xmm1, xmm1, 0d8h					;	d1  c1  d0  c0
	movlps		qword ptr [edi-0020h], xmm2			;	b2  a2
	prefetchnta [ebx]
	movhps		qword ptr [edi-0010h], xmm2			;	b3  a3
	shufps		xmm3, xmm3, 0d8h					;	d3  c3  d2  c2
	movlps		qword ptr [edi-0038h], xmm1			;	d0  c0
	sub			esi, edx							; correct esi
	movhps		qword ptr [edi-0028h], xmm1			;	d1  c1
	prefetchnta [ebx+edx]
	movlps		qword ptr [edi-0018h], xmm3			;	d2  c2
	add			esi, 4*4*1
	movhps		qword ptr [edi-0008h], xmm3			;	d3  c3
	prefetchnta [ebx+edx*2]
ENDM

SWIZZLE5	MACRO
	movlps		xmm0, qword ptr [esi+000h]			;	--	--	a1	a0
	lea			ebx, [esi+edx*4]					; ebx = addr of next next 4 FVF
	movlps		xmm2, qword ptr [esi+008h]			;	--	--	a3	a2
	movss		xmm6, dword ptr [esi+010h]		   	;   0   0   0   a4
	lea			esi, [esi+edx]
	movhps		xmm0, qword ptr [esi+000h]			;	b1  b0  a1	a0
	add			edi, 5*4*4
	movhps		xmm2, qword ptr [esi+008h]			;	b3  b2  a3	a2
	movhps		xmm6, qword ptr [esi+010h]	   		;   --  b4  0   a4
	movlps		xmm1, qword ptr [esi+edx+000h]		;	--	--	c1	c0
	shufps		xmm0, xmm0, 0d8h					;	b1  a1  b0  a0
	movlps		xmm3, qword ptr [esi+edx+008h]		;	--	--	c3	c2
	movss		xmm7, dword ptr [esi+edx+010h]   	;   0   0   0   c4
	shufps		xmm2, xmm2, 0d8h					;	b3  a3  b2  a2
	movhps		xmm1, qword ptr [esi+edx*2+000h]	;	d1  d0  c1	c0
	movhps		xmm3, qword ptr [esi+edx*2+008h]	;	d3  d2  c3	c2
	movhps		xmm7, qword ptr [esi+edx*2+010h]	;   --  d4  0   c4
	prefetchnta [ebx]
	movlps		qword ptr [edi-0050h], xmm0			;	b0  a0
	shufps		xmm1, xmm1, 0d8h					;	d1  c1  d0  c0
	movhps		qword ptr [edi-0040h], xmm0			;	b1  a1
	add			ebx, edx
	movlps		qword ptr [edi-0030h], xmm2			;	b2  a2
	shufps		xmm3, xmm3, 0d8h					;	d3  c3  d2  c2
	movhps		qword ptr [edi-0020h], xmm2			;	b3  a3
	prefetchnta [ebx]
	movlps		qword ptr [edi-0048h], xmm1			;	d0  c0
	shufps		xmm6, xmm7, 088h					;	d4  c4  b4  a4
	movhps		qword ptr [edi-0038h], xmm1			;	d1  c1
	sub			esi, edx							; correct esi
	movlps		qword ptr [edi-0028h], xmm3			;	d2  c2
	prefetchnta [ebx+edx]
	movhps		qword ptr [edi-0018h], xmm3			;	d3  c3
	add			esi, 5*4*1
	movups		oword ptr [edi-0010h], xmm6			;	d4  c4  b4  a4
	prefetchnta [ebx+edx*2]
ENDM

SWIZZLE6	MACRO
	movlps		xmm0, qword ptr [esi+000h]			;	--	--	a1	a0
	movlps		xmm2, qword ptr [esi+008h]			;	--	--	a3	a2
	lea			ebx, [esi+edx*4]					; ebx = addr of next next 4 FVF
	movlps		xmm4, qword ptr [esi+010h]			;	--	--	a5	a4
	lea			esi, [esi+edx]
	movhps		xmm0, qword ptr [esi+000h]			;	b1  b0  a1	a0
	movhps		xmm2, qword ptr [esi+008h]			;	b3  b2  a3	a2
	add			edi, 6*4*4
	movhps		xmm4, qword ptr [esi+010h]			;	b5  b4  a5	a4
	movlps		xmm1, qword ptr [esi+edx+000h]		;	--	--	c1	c0
	shufps		xmm0, xmm0, 0d8h					;	b1  a1  b0  a0
	movlps		xmm3, qword ptr [esi+edx+008h]		;	--	--	c3	c2
	movlps		xmm5, qword ptr [esi+edx+010h]		;	--	--	c5	c4
	shufps		xmm2, xmm2, 0d8h					;	b3  a3  b2  a2
	movhps		xmm1, qword ptr [esi+edx*2+000h]	;	d1  d0  c1	c0
	movhps		xmm3, qword ptr [esi+edx*2+008h]	;	d3  d2  c3	c2
	shufps		xmm4, xmm4, 0d8h					;	b5  b4  a5	a4
	movhps		xmm5, qword ptr [esi+edx*2+010h]	;	d5  d4  c5	c4
	movlps		qword ptr [edi-0060h], xmm0			;	b0  a0
	shufps		xmm1, xmm1, 0d8h					;	d1  c1  d0  c0
	movhps		qword ptr [edi-0050h], xmm0			;	b1  a1
	prefetchnta [ebx]
	movlps		qword ptr [edi-0040h], xmm2			;	b2  a2
	shufps		xmm3, xmm3, 0d8h					;	d3  c3  d2  c2
	movhps		qword ptr [edi-0030h], xmm2			;	b3  a3
	add			ebx, edx
	movlps		qword ptr [edi-0020h], xmm4			;	b4  a4
	shufps		xmm5, xmm5, 0d8h					;	d6  c6  d5  c5
	movhps		qword ptr [edi-0010h], xmm4			;	b5  a5
	prefetchnta [ebx]
	movlps		qword ptr [edi-0058h], xmm1			;	d0  c0
	sub			esi, edx							; correct esi
	movhps		qword ptr [edi-0048h], xmm1			;	d1  c1
	prefetchnta [ebx+edx]
	movlps		qword ptr [edi-0038h], xmm3			;	d2  c2
	add			esi, 6*4*1
	movhps		qword ptr [edi-0028h], xmm3			;	d3  c3
	prefetchnta [ebx+edx*2]
	movlps		qword ptr [edi-0018h], xmm5			;	d4  c4
	movhps		qword ptr [edi-0008h], xmm5			;	d5  c5
ENDM

SWIZZLE7	MACRO
	movlps		xmm0, qword ptr [esi+000h]			;	--	--	a1	a0
	movlps		xmm2, qword ptr [esi+008h]			;	--	--	a3	a2
	lea			ebx, [esi+edx*4]					; ebx = addr of next next 4 FVF
	movlps		xmm4, qword ptr [esi+010h]			;	--	--	a5	a4
	movss		xmm6, dword ptr [esi+018h]		   	;   0   0   0   a6
	lea			esi, [esi+edx]
	movhps		xmm0, qword ptr [esi+000h]			;	b1  b0  a1	a0
	movhps		xmm2, qword ptr [esi+008h]			;	b3  b2  a3	a2
	add			edi, 7*4*4
	movhps		xmm4, qword ptr [esi+010h]			;	b5  b4  a5	a4
	movhps		xmm6, qword ptr [esi+018h]	   		;   --  b6  0   a6
	shufps		xmm0, xmm0, 0d8h					;	b1  a1  b0  a0
	movlps		xmm1, qword ptr [esi+edx+000h]		;	--	--	c1	c0
	movlps		xmm3, qword ptr [esi+edx+008h]		;	--	--	c3	c2
	shufps		xmm2, xmm2, 0d8h					;	b3  a3  b2  a2
	movlps		xmm5, qword ptr [esi+edx+010h]		;	--	--	c5	c4
	movss		xmm7, dword ptr [esi+edx+018h]   	;   0   0   0   c6
	shufps		xmm4, xmm4, 0d8h					;	b5  a5  b4	a4
	movhps		xmm1, qword ptr [esi+edx*2+000h]	;	d1  d0  c1	c0
	movhps		xmm3, qword ptr [esi+edx*2+008h]	;	d3  d2  c3	c2
	movhps		xmm5, qword ptr [esi+edx*2+010h]	;	d5  d4  c5	c4
	movhps		xmm7, qword ptr [esi+edx*2+018h]	  	;   --  d6  0   c6
	movlps		qword ptr [edi-0070h], xmm0			;	b0  a0
	prefetchnta [ebx]
	movhps		qword ptr [edi-0060h], xmm0			;	b1  a1
	shufps		xmm1, xmm1, 0d8h					;	d1  c1  d0  c0
	movlps		qword ptr [edi-0050h], xmm2			;	b2  a2
	add			ebx, edx
	movhps		qword ptr [edi-0040h], xmm2			;	b3  a3
	prefetchnta [ebx]
	movlps		qword ptr [edi-0030h], xmm4			;	b4  a4
	shufps		xmm3, xmm3, 0d8h					;	d3  c3  d2  c2
	movhps		qword ptr [edi-0020h], xmm4			;	b5  a5
	shufps		xmm5, xmm5, 0d8h					;	d5  c5  d4  c4
	movlps		qword ptr [edi-0068h], xmm1			;	d0  c0
	prefetchnta [ebx+edx]
	movhps		qword ptr [edi-0058h], xmm1			;	d1  c1
	shufps		xmm6, xmm7, 088h					;	d6  c6  b6  a6
	movlps		qword ptr [edi-0048h], xmm3			;	d2  c2
	sub			esi, edx							; correct esi
	movhps		qword ptr [edi-0038h], xmm3			;	d3  c3
	prefetchnta [ebx+edx*2]
	movlps		qword ptr [edi-0028h], xmm5			;	d4  c4
	add			esi, 7*4*1
	movhps		qword ptr [edi-0018h], xmm5			;	d5  c5
	movups		oword ptr [edi-0010h], xmm6			;	d6  c6  b6  a6
ENDM

SWIZZLE8	MACRO
	movlps		xmm0, qword ptr [esi+000h]			;	--	--	a1	a0
	movlps		xmm2, qword ptr [esi+008h]			;	--	--	a3	a2
	lea			ebx, [esi+edx*4]					; ebx = addr of next next 4 FVF
	movlps		xmm4, qword ptr [esi+010h]			;	--	--	a5	a4
	movlps		xmm6, qword ptr [esi+018h]			;	--	--	a7	a6
	lea			esi, [esi+edx]
	movhps		xmm0, qword ptr [esi+000h]			;	b1  b0  a1	a0
	movhps		xmm2, qword ptr [esi+008h]			;	b3  b2  a3	a2
	add			edi, 8*4*4
	movhps		xmm4, qword ptr [esi+010h]			;	b5  b4  a5	a4
	movhps		xmm6, qword ptr [esi+018h]			;	b7  b6  a7	a6
	shufps		xmm0, xmm0, 0d8h					;	b1  a1  b0  a0
	movlps		xmm1, qword ptr [esi+edx+000h]		;	--	--	c1	c0
	movlps		xmm3, qword ptr [esi+edx+008h]		;	--	--	c3	c2
	shufps		xmm2, xmm2, 0d8h					;	b3  a3  b2  a2
	movlps		xmm5, qword ptr [esi+edx+010h]		;	--	--	c5	c4
	movlps		xmm7, qword ptr [esi+edx+018h]		;	--	--	c7	c6
	shufps		xmm4, xmm4, 0d8h					;	b5  a5  b4	a4
	movhps		xmm1, qword ptr [esi+edx*2+000h]	;	d1  d0  c1	c0
	movhps		xmm3, qword ptr [esi+edx*2+008h]	;	d3  d2  c3	c2
	shufps		xmm6, xmm6, 0d8h					;	b7  a7  b6	a6
	movhps		xmm5, qword ptr [esi+edx*2+010h]	;	d5  d4  c5	c4
	movhps		xmm7, qword ptr [esi+edx*2+018h]	;	d7  d6  c7	c6
	movlps		qword ptr [edi-0080h], xmm0			;	b0  a0
	prefetchnta [ebx]
	movhps		qword ptr [edi-0070h], xmm0			;	b1  a1
	shufps		xmm1, xmm1, 0d8h					;	d1  c1  d0  c0
	movlps		qword ptr [edi-0060h], xmm2			;	b2  a2
	add			ebx, edx
	movhps		qword ptr [edi-0050h], xmm2			;	b3  a3
	shufps		xmm3, xmm3, 0d8h					;	d3  c3  d2  c2
	movlps		qword ptr [edi-0040h], xmm4			;	b4  a4
	prefetchnta [ebx]
	movhps		qword ptr [edi-0030h], xmm4			;	b5  a5
	shufps		xmm5, xmm5, 0d8h					;	d5  c5  d4  c4
	movlps		qword ptr [edi-0020h], xmm6			;	b6  a6
	sub			esi, edx							; correct esi
	movhps		qword ptr [edi-0010h], xmm6			;	b7  a7
	shufps		xmm7, xmm7, 0d8h					;	d7  c7  d6  c6
	movlps		qword ptr [edi-0078h], xmm1			;	d0  c0
	prefetchnta [ebx+edx]
	movhps		qword ptr [edi-0068h], xmm1			;	d1  c1
	add			esi, 8*4*1
	movlps		qword ptr [edi-0058h], xmm3			;	d2  c2
	movhps		qword ptr [edi-0048h], xmm3			;	d3  c3
	prefetchnta [ebx+edx*2]
	movlps		qword ptr [edi-0038h], xmm5			;	d4  c4
	movhps		qword ptr [edi-0028h], xmm5			;	d5  c5
	movlps		qword ptr [edi-0018h], xmm7			;	d6  c6
	movhps		qword ptr [edi-0008h], xmm7			;	d7  c7
ENDM


COPYTXTR1	MACRO		; copy 1 doesn't adjust esi/edi and does no prefetch
	movlps		xmm0, qword ptr [esi+000h]			;	s00	t00 --- ---
	movhps		xmm0, qword ptr [esi+edx+000h]		;	s00	t00 s10	t10
	lea			esi, [esi+edx*2]					; set esi to the 3rd FVF
	movlps		xmm4, qword ptr [esi+000h]			;	s20	t20 --- ---
	movhps		xmm4, qword ptr [esi+edx+000h]		;	s20	t20 s30	t30
	movups		oword ptr [edi], xmm0				;	s00	t00 s10	t10
	movups		oword ptr [edi+0010h], xmm4			;	s20	t20 s30	t30
ENDM

COPYTXTR2	MACRO		; copy 2 doesn't adjust esi/edi
	movlps		xmm0, qword ptr [esi+000h]			;	s00	t00 --- ---
	lea			ebx, [esi+edx*4]					; ebx = addr of next next 4 FVF's textures
	movlps		xmm1, qword ptr [esi+008h]			;	s01	t01 --- ---
	movhps		xmm0, qword ptr [esi+edx+000h]		;	s00	t00 s10	t10
	movhps		xmm1, qword ptr [esi+edx+008h]		;	s01	t01 s11	t11
	lea			esi, [esi+edx*2]					; set esi to the 3rd FVF
	movlps		xmm4, qword ptr [esi+000h]			;	s20	t20 --- ---
	movlps		xmm5, qword ptr [esi+008h]			;	s21	t21 --- ---
	movhps		xmm4, qword ptr [esi+edx+000h]		;	s20	t20 s30	t30
	movhps		xmm5, qword ptr [esi+edx+008h]		;	s21	t21 s31	t31
	prefetchnta [ebx]
	movups		oword ptr [edi+0000h], xmm0			;	s00	t00 s10	t10
	add			ebx, edx
	movups		oword ptr [edi+0020h], xmm1			;	s01	t01 s11	t11
	prefetchnta [ebx]
	movups		oword ptr [edi+0010h], xmm4			;	s20	t20 s30	t30
	prefetchnta [ebx+edx]
	movups		oword ptr [edi+0030h], xmm5			;	s21	t21 s31	t31
	prefetchnta [ebx+edx*2]
ENDM

COPYTXTR3	MACRO		; copy 3 adjusts esi/edi (can be cascaded)
	movlps		xmm0, qword ptr [esi+000h]			;	s00	t00 --- ---
	lea			ebx, [esi+edx*4]					; ebx = addr of next next 4 FVF's textures
	movlps		xmm1, qword ptr [esi+008h]			;	s01	t01 --- ---
	movlps		xmm2, qword ptr [esi+010h]			;	s02	t02 --- ---
	lea			esi, [esi+edx]						; set esi to the 2nd FVF
	movhps		xmm0, qword ptr [esi+000h]			;	s00	t00 s10	t10
	movhps		xmm1, qword ptr [esi+008h]			;	s01	t01 s11	t11
	add			edi, 3*20h
	movhps		xmm2, qword ptr [esi+010h]			;	s02	t02 s12	t12
	movlps		xmm4, qword ptr [esi+edx+000h]		;	s20	t20 --- ---
	movlps		xmm5, qword ptr [esi+edx+008h]		;	s21	t21 --- ---
	movlps		xmm6, qword ptr [esi+edx+010h]		;	s22	t22 --- ---
	movhps		xmm4, qword ptr [esi+edx*2+000h]	;	s20	t20 s30	t30
	movhps		xmm5, qword ptr [esi+edx*2+008h]	;	s21	t21 s31	t31
	movhps		xmm6, qword ptr [esi+edx*2+010h]	;	s22	t22 s32	t32
	prefetchnta [ebx]
	movups		oword ptr [edi-0060h], xmm0			;	s00	t00 s10	t10
	add			ebx, edx
	movups		oword ptr [edi-0040h], xmm1			;	s01	t01 s11	t11
	prefetchnta [ebx]
	movups		oword ptr [edi-0020h], xmm2			;	s02	t02 s12	t12
	sub			esi, edx							; correct esi
	movups		oword ptr [edi-0050h], xmm4			;	s20	t20 s30	t30
	prefetchnta [ebx+edx]
	movups		oword ptr [edi-0030h], xmm5			;	s21	t21 s31	t31
	add			esi, 3*8h
	movups		oword ptr [edi-0010h], xmm6			;	s22	t22 s32	t32
	prefetchnta [ebx+edx*2]
ENDM

COPYTXTR4	MACRO		; copy 4 adjusts esi/edi (can be cascaded)
	movlps		xmm0, qword ptr [esi+000h]			;	s00	t00 --- ---
	add			edi, 4*20h
	movlps		xmm1, qword ptr [esi+008h]			;	s01	t01 --- ---
	lea			ebx, [esi+edx*4]					; ebx = addr of next next 4 FVF's textures
	movlps		xmm2, qword ptr [esi+010h]			;	s02	t02 --- ---
	movlps		xmm3, qword ptr [esi+018h]			;	s03	t03 --- ---
	lea			esi, [esi+edx]						; set esi to the 2nd FVF
	movhps		xmm0, qword ptr [esi+000h]			;	s00	t00 s10	t10
	movhps		xmm1, qword ptr [esi+008h]			;	s01	t01 s11	t11
	movhps		xmm2, qword ptr [esi+010h]			;	s02	t02 s12	t12
	movhps		xmm3, qword ptr [esi+018h]			;	s03	t03 s13	t13
	movlps		xmm4, qword ptr [esi+edx+000h]		;	s20	t20 --- ---
	movlps		xmm5, qword ptr [esi+edx+008h]		;	s21	t21 --- ---
	movlps		xmm6, qword ptr [esi+edx+010h]			;	s22	t22 --- ---
	movlps		xmm7, qword ptr [esi+edx+018h]		;	s23	t23 --- ---
	movhps		xmm4, qword ptr [esi+edx*2+000h]	;	s20	t20 s30	t30
	movhps		xmm5, qword ptr [esi+edx*2+008h]	;	s21	t21 s31	t31
	movhps		xmm6, qword ptr [esi+edx*2+010h]	;	s22	t22 s32	t32
	movhps		xmm7, qword ptr [esi+edx*2+018h]	;	s23	t23 s33	t33
	movups		oword ptr [edi-0080h], xmm0			;	s00	t00 s10	t10
	prefetchnta [ebx]
	movups		oword ptr [edi-0060h], xmm1			;	s01	t01 s11	t11
	add			ebx, edx
	movups		oword ptr [edi-0040h], xmm2			;	s02	t02 s12	t12
	prefetchnta [ebx]
	movups		oword ptr [edi-0020h], xmm3			;	s03	t03 s13	t13
	sub			esi, edx							; correct esi
	movups		oword ptr [edi-0070h], xmm4			;	s20	t20 s30	t30
	prefetchnta [ebx+edx]
	movups		oword ptr [edi-0050h], xmm5			;	s21	t21 s31	t31
	add			esi, 4*8h
	movups		oword ptr [edi-0030h], xmm6			;	s22	t22 s32	t32
	prefetchnta [ebx+edx*2]
	movups		oword ptr [edi-0010h], xmm7			;	s23	t23 s33	t33
ENDM



;===================================================================================================
;	void SwizzleSingleD3DtoSOA_Swiz n Params ( DWORD *lpSrc, DWORD* lpDst, DWORD dwStride, 
;		KniSwizSingle_CopyTxtrs_FunctionType* lpTextureCopyFunction )
;	lpSrc = source
;	lpDst = destination
;	dwStride = stride between INPUT FVF's, or sizeof(FVF)
;	KniSwizSingle_CopyTxtrs_FunctionType* = pointer to the texture copy function
;
;	Description:	
;	These functions swizzle and copy n Params.  It then jumps to the texture copy function which
;	which copies the textures, and returns.  Therefore, all the pointers have to be correct and 
;	the register save/restore mechanism must be the same between the two functions.
;===================================================================================================
_stkBase		= 0
_savedEDI		= (_stkBase +  0)		; lpDst
_savedESI		= (_stkBase +  4)		; lpSrc
_savedEBX		= (_stkBase +  8)		; prefetch pointer
_retAddr		= (_stkBase +  12)
_lpSrc			= (_retAddr +  4)
_lpDst			= (_retAddr +  8)
_dwStride		= (_retAddr + 12)
_txCpFunct		= (_retAddr + 16)
SwizzleSingleD3DtoSOA_Swiz0Params	PROC NEAR PUBLIC
	ret			; bad FVF type, don't copy anything, just return
SwizzleSingleD3DtoSOA_Swiz0Params	ENDP

Align 16
SwizzleSingleD3DtoSOA_Swiz3Params	PROC NEAR PUBLIC
	sub			esp, (_retAddr - _stkBase)
	mov			dword ptr _savedEBX[esp], ebx	; push ebx
	mov			dword ptr _savedESI[esp], esi	; push esi
	mov			dword ptr _savedEDI[esp], edi	; push edi

	mov			esi, dword ptr _lpSrc[esp]
	mov			edi, dword ptr _lpDst[esp]
	mov			edx, dword ptr _dwStride[esp]
	mov			eax, dword ptr _txCpFunct[esp]

	SWIZZLE3
	jmp			eax
SwizzleSingleD3DtoSOA_Swiz3Params	ENDP

Align 16
SwizzleSingleD3DtoSOA_Swiz4Params	PROC NEAR PUBLIC
	sub			esp, (_retAddr - _stkBase)
	mov			dword ptr _savedEBX[esp], ebx	; push ebx
	mov			dword ptr _savedESI[esp], esi	; push esi
	mov			dword ptr _savedEDI[esp], edi	; push edi

	mov			esi, dword ptr _lpSrc[esp]
	mov			edi, dword ptr _lpDst[esp]
	mov			edx, dword ptr _dwStride[esp]
	mov			eax, dword ptr _txCpFunct[esp]

	SWIZZLE4
	jmp			eax
SwizzleSingleD3DtoSOA_Swiz4Params	ENDP

Align 16
SwizzleSingleD3DtoSOA_Swiz5Params	PROC NEAR PUBLIC
	sub			esp, (_retAddr - _stkBase)
	mov			dword ptr _savedEBX[esp], ebx	; push ebx
	mov			dword ptr _savedESI[esp], esi	; push esi
	mov			dword ptr _savedEDI[esp], edi	; push edi

	mov			esi, dword ptr _lpSrc[esp]
	mov			edi, dword ptr _lpDst[esp]
	mov			edx, dword ptr _dwStride[esp]
	mov			eax, dword ptr _txCpFunct[esp]

	SWIZZLE5
	jmp			eax
SwizzleSingleD3DtoSOA_Swiz5Params	ENDP

Align 16
SwizzleSingleD3DtoSOA_Swiz6Params	PROC NEAR PUBLIC
	sub			esp, (_retAddr - _stkBase)
	mov			dword ptr _savedEBX[esp], ebx	; push ebx
	mov			dword ptr _savedESI[esp], esi	; push esi
	mov			dword ptr _savedEDI[esp], edi	; push edi

	mov			esi, dword ptr _lpSrc[esp]
	mov			edi, dword ptr _lpDst[esp]
	mov			edx, dword ptr _dwStride[esp]
	mov			eax, dword ptr _txCpFunct[esp]

	SWIZZLE6
	jmp			eax
SwizzleSingleD3DtoSOA_Swiz6Params	ENDP

Align 16
SwizzleSingleD3DtoSOA_Swiz7Params	PROC NEAR PUBLIC
	sub			esp, (_retAddr - _stkBase)
	mov			dword ptr _savedEBX[esp], ebx	; push ebx
	mov			dword ptr _savedESI[esp], esi	; push esi
	mov			dword ptr _savedEDI[esp], edi	; push edi

	mov			esi, dword ptr _lpSrc[esp]
	mov			edi, dword ptr _lpDst[esp]
	mov			edx, dword ptr _dwStride[esp]
	mov			eax, dword ptr _txCpFunct[esp]

	SWIZZLE7
	jmp			eax
SwizzleSingleD3DtoSOA_Swiz7Params	ENDP

Align 16
SwizzleSingleD3DtoSOA_Swiz8Params	PROC NEAR PUBLIC
	sub			esp, (_retAddr - _stkBase)
	mov			dword ptr _savedEBX[esp], ebx	; push ebx
	mov			dword ptr _savedESI[esp], esi	; push esi
	mov			dword ptr _savedEDI[esp], edi	; push edi

	mov			esi, dword ptr _lpSrc[esp]
	mov			edi, dword ptr _lpDst[esp]
	mov			edx, dword ptr _dwStride[esp]
	mov			eax, dword ptr _txCpFunct[esp]

	SWIZZLE8
	jmp			eax
SwizzleSingleD3DtoSOA_Swiz8Params	ENDP

Align 16
SwizzleSingleD3DtoSOA_Swiz9Params	PROC NEAR PUBLIC
	sub			esp, (_retAddr - _stkBase)
	mov			dword ptr _savedEBX[esp], ebx	; push ebx
	mov			dword ptr _savedESI[esp], esi	; push esi
	mov			dword ptr _savedEDI[esp], edi	; push edi

	mov			esi, dword ptr _lpSrc[esp]
	mov			edi, dword ptr _lpDst[esp]
	mov			edx, dword ptr _dwStride[esp]
	mov			eax, dword ptr _txCpFunct[esp]

	SWIZZLE4
	SWIZZLE5
	jmp			eax
SwizzleSingleD3DtoSOA_Swiz9Params	ENDP

Align 16
SwizzleSingleD3DtoSOA_Swiz10Params	PROC NEAR PUBLIC
	sub			esp, (_retAddr - _stkBase)
	mov			dword ptr _savedEBX[esp], ebx	; push ebx
	mov			dword ptr _savedESI[esp], esi	; push esi
	mov			dword ptr _savedEDI[esp], edi	; push edi

	mov			esi, dword ptr _lpSrc[esp]
	mov			edi, dword ptr _lpDst[esp]
	mov			edx, dword ptr _dwStride[esp]
	mov			eax, dword ptr _txCpFunct[esp]

	SWIZZLE6
	SWIZZLE4
	jmp			eax
SwizzleSingleD3DtoSOA_Swiz10Params	ENDP

Align 16
SwizzleSingleD3DtoSOA_Swiz11Params	PROC NEAR PUBLIC
	sub			esp, (_retAddr - _stkBase)
	mov			dword ptr _savedEBX[esp], ebx	; push ebx
	mov			dword ptr _savedESI[esp], esi	; push esi
	mov			dword ptr _savedEDI[esp], edi	; push edi

	mov			esi, dword ptr _lpSrc[esp]
	mov			edi, dword ptr _lpDst[esp]
	mov			edx, dword ptr _dwStride[esp]
	mov			eax, dword ptr _txCpFunct[esp]

	SWIZZLE8
	SWIZZLE3
	jmp			eax
SwizzleSingleD3DtoSOA_Swiz11Params	ENDP

Align 16
SwizzleSingleD3DtoSOA_Swiz12Params	PROC NEAR PUBLIC
	sub			esp, (_retAddr - _stkBase)
	mov			dword ptr _savedEBX[esp], ebx	; push ebx
	mov			dword ptr _savedESI[esp], esi	; push esi
	mov			dword ptr _savedEDI[esp], edi	; push edi

	mov			esi, dword ptr _lpSrc[esp]
	mov			edi, dword ptr _lpDst[esp]
	mov			edx, dword ptr _dwStride[esp]
	mov			eax, dword ptr _txCpFunct[esp]

	SWIZZLE8
	SWIZZLE4
	jmp			eax
SwizzleSingleD3DtoSOA_Swiz12Params	ENDP

Align 16
SwizzleSingleD3DtoSOA_Swiz13Params	PROC NEAR PUBLIC
	sub			esp, (_retAddr - _stkBase)
	mov			dword ptr _savedEBX[esp], ebx	; push ebx
	mov			dword ptr _savedESI[esp], esi	; push esi
	mov			dword ptr _savedEDI[esp], edi	; push edi

	mov			esi, dword ptr _lpSrc[esp]
	mov			edi, dword ptr _lpDst[esp]
	mov			edx, dword ptr _dwStride[esp]
	mov			eax, dword ptr _txCpFunct[esp]

	SWIZZLE8
	SWIZZLE5
	jmp			eax
SwizzleSingleD3DtoSOA_Swiz13Params	ENDP


Align 16
SwizzleSingleD3DtoSOA_Copy0Txtrs	PROC NEAR PUBLIC
	mov			edi, dword ptr _savedEDI[esp]	; pop edi
	mov			esi, dword ptr _savedESI[esp]	; pop esi
	mov			ebx, dword ptr _savedEBX[esp]	; pop ebx
	add			esp, (_retAddr - _stkBase)
	ret
SwizzleSingleD3DtoSOA_Copy0Txtrs	ENDP

Align 16
SwizzleSingleD3DtoSOA_Copy1Txtrs	PROC NEAR PUBLIC
	COPYTXTR1
	mov			edi, dword ptr _savedEDI[esp]	; pop edi
	mov			esi, dword ptr _savedESI[esp]	; pop esi
	mov			ebx, dword ptr _savedEBX[esp]	; pop ebx
	add			esp, (_retAddr - _stkBase)
	ret
SwizzleSingleD3DtoSOA_Copy1Txtrs	ENDP

Align 16
SwizzleSingleD3DtoSOA_Copy2Txtrs	PROC NEAR PUBLIC
	COPYTXTR2
	mov			edi, dword ptr _savedEDI[esp]	; pop edi
	mov			esi, dword ptr _savedESI[esp]	; pop esi
	mov			ebx, dword ptr _savedEBX[esp]	; pop ebx
	add			esp, (_retAddr - _stkBase)
	ret
SwizzleSingleD3DtoSOA_Copy2Txtrs	ENDP

Align 16
SwizzleSingleD3DtoSOA_Copy3Txtrs	PROC NEAR PUBLIC
	COPYTXTR3
	mov			edi, dword ptr _savedEDI[esp]	; pop edi
	mov			esi, dword ptr _savedESI[esp]	; pop esi
	mov			ebx, dword ptr _savedEBX[esp]	; pop ebx
	add			esp, (_retAddr - _stkBase)
	ret
SwizzleSingleD3DtoSOA_Copy3Txtrs	ENDP

Align 16
SwizzleSingleD3DtoSOA_Copy4Txtrs	PROC NEAR PUBLIC
	COPYTXTR4
	mov			edi, dword ptr _savedEDI[esp]	; pop edi
	mov			esi, dword ptr _savedESI[esp]	; pop esi
	mov			ebx, dword ptr _savedEBX[esp]	; pop ebx
	add			esp, (_retAddr - _stkBase)
	ret
SwizzleSingleD3DtoSOA_Copy4Txtrs	ENDP

Align 16
SwizzleSingleD3DtoSOA_Copy5Txtrs	PROC NEAR PUBLIC
	COPYTXTR3
	COPYTXTR2
	mov			edi, dword ptr _savedEDI[esp]	; pop edi
	mov			esi, dword ptr _savedESI[esp]	; pop esi
	mov			ebx, dword ptr _savedEBX[esp]	; pop ebx
	add			esp, (_retAddr - _stkBase)
	ret
SwizzleSingleD3DtoSOA_Copy5Txtrs	ENDP

Align 16
SwizzleSingleD3DtoSOA_Copy6Txtrs	PROC NEAR PUBLIC
	COPYTXTR4
	COPYTXTR2
	mov			edi, dword ptr _savedEDI[esp]	; pop edi
	mov			esi, dword ptr _savedESI[esp]	; pop esi
	mov			ebx, dword ptr _savedEBX[esp]	; pop ebx
	add			esp, (_retAddr - _stkBase)
	ret
SwizzleSingleD3DtoSOA_Copy6Txtrs	ENDP

Align 16
SwizzleSingleD3DtoSOA_Copy7Txtrs	PROC NEAR PUBLIC
	COPYTXTR4
	COPYTXTR3
	mov			edi, dword ptr _savedEDI[esp]	; pop edi
	mov			esi, dword ptr _savedESI[esp]	; pop esi
	mov			ebx, dword ptr _savedEBX[esp]	; pop ebx
	add			esp, (_retAddr - _stkBase)
	ret
SwizzleSingleD3DtoSOA_Copy7Txtrs	ENDP

Align 16
SwizzleSingleD3DtoSOA_Copy8Txtrs	PROC NEAR PUBLIC
	COPYTXTR4
	COPYTXTR4
	mov			edi, dword ptr _savedEDI[esp]	; pop edi
	mov			esi, dword ptr _savedESI[esp]	; pop esi
	mov			ebx, dword ptr _savedEBX[esp]	; pop ebx
	add			esp, (_retAddr - _stkBase)
	ret
SwizzleSingleD3DtoSOA_Copy8Txtrs	ENDP




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                               SWIZZLE VERTEX BUFFER CODE DESCRIPTION
;;
;; We need to swizzle an entire vertex buffer from AOS to SOA4 format.  This is just like the SwizzleSingle code except it does the 
;; entire buffer.  It doesn't use a C function to set up all the pointers, it does this locally.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;===================================================================================================
;	SwizzleD3DtoSOA_Asm ( pSrc, pDst, dwSrcStride, dwDstStride, dwNumSOAVecs, dwFVFEntries, dwNumTxtures )
;		pSrc = ptr to source vertex list (AOS format)
;		pDst = ptr to destination vertex list (SOA format)
;		dwSrcStride = source vertex stride (for each vert)
;		dwDstStride = destination vertex stride (for a group of 4 verts)
;		dwNumSOAVecs = groups of 4 verts that we'll copy
;		dwFVFEntries = number of params to swizzle (doesn't include textures)
;		dwNumTxtures = number of textures to copy
;
;	Description:	
;	These functions swizzle and copy "dwNumSOAVecs" verticies
;===================================================================================================
_stkBase		= 0
_savedEDI		= (_stkBase +  0)		; lpDst
_savedESI		= (_stkBase +  4)		; lpSrc
_savedEBX		= (_stkBase +  8)		; prefetch pointer
_savedECX		= (_stkBase +  12)		; count
_retAddr		= (_stkBase +  16)
_pSrc			= (_retAddr +  4)
_pDst			= (_retAddr +  8)
_dwSrcStride	= (_retAddr +  12)
_dwDstStride	= (_retAddr +  16)
_dwNumSOAVecs	= (_retAddr +  20)
_dwFVFEntries	= (_retAddr +  24)
_dwNumTxtures	= (_retAddr +  28)
Align 16
_SwizzleD3DtoSOA_Asm	PROC NEAR PUBLIC
	sub			esp, (_retAddr - _stkBase)
	mov			dword ptr _savedECX[esp], ecx	; push ecx
	mov			dword ptr _savedEBX[esp], ebx	; push ebx
	mov			dword ptr _savedESI[esp], esi	; push esi
	mov			dword ptr _savedEDI[esp], edi	; push edi

	mov			ebx, dword ptr _dwFVFEntries[esp]
	mov			eax, dword ptr _dwNumTxtures[esp]
	movd		mm0, dword ptr _pSrc[esp]
	movd		mm1, dword ptr _pDst[esp]
	mov			ebx, dword ptr [SwizzleLoop_SwizParmsFunctions+ebx*4]
	mov			eax, dword ptr [SwizzleLoop_CopyTxtrsFunctions+eax*4]
	movd		mm2, dword ptr _dwSrcStride[esp]	; src stride
	movd		mm3, dword ptr _dwDstStride[esp]	; dst stride
	movd		esi, mm0
	movd		edi, mm1
	mov			ecx, dword ptr _dwNumSOAVecs[esp]	; count
	mov			edx, dword ptr _dwSrcStride[esp]	; src stride
	pslld		mm2, 2								; src stride * 4
	movd		mm7, ebx

	jmp			ebx
_SwizzleD3DtoSOA_Asm	ENDP


SwizzleLoopD3DtoSOA_Swiz0Params	PROC NEAR PRIVATE
	paddd		mm0, mm2
	paddd		mm1, mm3
	jmp			eax
SwizzleLoopD3DtoSOA_Swiz0Params	ENDP

Align 16
SwizzleLoopD3DtoSOA_Swiz3Params	PROC NEAR PRIVATE
	SWIZZLE3
	paddd		mm0, mm2
	paddd		mm1, mm3
	jmp			eax
SwizzleLoopD3DtoSOA_Swiz3Params	ENDP

Align 16
SwizzleLoopD3DtoSOA_Swiz4Params	PROC NEAR PRIVATE
	SWIZZLE4
	paddd		mm0, mm2
	paddd		mm1, mm3
	jmp			eax
SwizzleLoopD3DtoSOA_Swiz4Params	ENDP

Align 16
SwizzleLoopD3DtoSOA_Swiz5Params	PROC NEAR PRIVATE
	SWIZZLE5
	paddd		mm0, mm2
	paddd		mm1, mm3
	jmp			eax
SwizzleLoopD3DtoSOA_Swiz5Params	ENDP

Align 16
SwizzleLoopD3DtoSOA_Swiz6Params	PROC NEAR PRIVATE
	SWIZZLE6
	paddd		mm0, mm2
	paddd		mm1, mm3
	jmp			eax
SwizzleLoopD3DtoSOA_Swiz6Params	ENDP

Align 16
SwizzleLoopD3DtoSOA_Swiz7Params	PROC NEAR PRIVATE
	SWIZZLE7
	paddd		mm0, mm2
	paddd		mm1, mm3
	jmp			eax
SwizzleLoopD3DtoSOA_Swiz7Params	ENDP

Align 16
SwizzleLoopD3DtoSOA_Swiz8Params	PROC NEAR PRIVATE
	SWIZZLE8
	paddd		mm0, mm2
	paddd		mm1, mm3
	jmp			eax
SwizzleLoopD3DtoSOA_Swiz8Params	ENDP

Align 16
SwizzleLoopD3DtoSOA_Swiz9Params	PROC NEAR PRIVATE
	SWIZZLE4
	SWIZZLE5
	paddd		mm0, mm2
	paddd		mm1, mm3
	jmp			eax
SwizzleLoopD3DtoSOA_Swiz9Params	ENDP

Align 16
SwizzleLoopD3DtoSOA_Swiz10Params	PROC NEAR PRIVATE
	SWIZZLE6
	SWIZZLE4
	paddd		mm0, mm2
	paddd		mm1, mm3
	jmp			eax
SwizzleLoopD3DtoSOA_Swiz10Params	ENDP

Align 16
SwizzleLoopD3DtoSOA_Swiz11Params	PROC NEAR PRIVATE
	SWIZZLE8
	SWIZZLE3
	paddd		mm0, mm2
	paddd		mm1, mm3
	jmp			eax
SwizzleLoopD3DtoSOA_Swiz11Params	ENDP

Align 16
SwizzleLoopD3DtoSOA_Swiz12Params	PROC NEAR PRIVATE
	SWIZZLE8
	SWIZZLE4
	paddd		mm0, mm2
	paddd		mm1, mm3
	jmp			eax
SwizzleLoopD3DtoSOA_Swiz12Params	ENDP

Align 16
SwizzleLoopD3DtoSOA_Swiz13Params	PROC NEAR PRIVATE
	SWIZZLE8
	SWIZZLE5
	paddd		mm0, mm2
	paddd		mm1, mm3
	jmp			eax
SwizzleLoopD3DtoSOA_Swiz13Params	ENDP


Align 16
SwizzleLoopD3DtoSOA_Copy0Txtrs	PROC NEAR PRIVATE
	dec			ecx
	jz			SwizzleD3DtoSOA_Asm_End
	movd		ebx, mm7
	movd		esi, mm0
	movd		edi, mm1
	jmp			ebx
SwizzleLoopD3DtoSOA_Copy0Txtrs	ENDP

Align 16
SwizzleLoopD3DtoSOA_Copy1Txtrs	PROC NEAR PRIVATE
	COPYTXTR1
	dec			ecx
	jz			SwizzleD3DtoSOA_Asm_End
	movd		ebx, mm7
	movd		esi, mm0
	movd		edi, mm1
	jmp			ebx
SwizzleLoopD3DtoSOA_Copy1Txtrs	ENDP

Align 16
SwizzleLoopD3DtoSOA_Copy2Txtrs	PROC NEAR PRIVATE
	COPYTXTR2
	dec			ecx
	jz			SwizzleD3DtoSOA_Asm_End
	movd		ebx, mm7
	movd		esi, mm0
	movd		edi, mm1
	jmp			ebx
SwizzleLoopD3DtoSOA_Copy2Txtrs	ENDP

Align 16
SwizzleLoopD3DtoSOA_Copy3Txtrs	PROC NEAR PRIVATE
	COPYTXTR3
	dec			ecx
	jz			SwizzleD3DtoSOA_Asm_End
	movd		ebx, mm7
	movd		esi, mm0
	movd		edi, mm1
	jmp			ebx
SwizzleLoopD3DtoSOA_Copy3Txtrs	ENDP

Align 16
SwizzleLoopD3DtoSOA_Copy4Txtrs	PROC NEAR PRIVATE
	COPYTXTR4
	dec			ecx
	jz			SwizzleD3DtoSOA_Asm_End
	movd		ebx, mm7
	movd		esi, mm0
	movd		edi, mm1
	jmp			ebx
SwizzleLoopD3DtoSOA_Copy4Txtrs	ENDP

Align 16
SwizzleLoopD3DtoSOA_Copy5Txtrs	PROC NEAR PRIVATE
	COPYTXTR3
	COPYTXTR2
	dec			ecx
	jz			SwizzleD3DtoSOA_Asm_End
	movd		ebx, mm7
	movd		esi, mm0
	movd		edi, mm1
	jmp			ebx
SwizzleLoopD3DtoSOA_Copy5Txtrs	ENDP

Align 16
SwizzleLoopD3DtoSOA_Copy6Txtrs	PROC NEAR PRIVATE
	COPYTXTR4
	COPYTXTR2
	dec			ecx
	jz			SwizzleD3DtoSOA_Asm_End
	movd		ebx, mm7
	movd		esi, mm0
	movd		edi, mm1
	jmp			ebx
SwizzleLoopD3DtoSOA_Copy6Txtrs	ENDP

Align 16
SwizzleLoopD3DtoSOA_Copy7Txtrs	PROC NEAR PRIVATE
	COPYTXTR4
	COPYTXTR3
	dec			ecx
	jz			SwizzleD3DtoSOA_Asm_End
	movd		ebx, mm7
	movd		esi, mm0
	movd		edi, mm1
	jmp			ebx
SwizzleLoopD3DtoSOA_Copy7Txtrs	ENDP

Align 16
SwizzleLoopD3DtoSOA_Copy8Txtrs	PROC NEAR PRIVATE
	COPYTXTR4
	COPYTXTR4
	dec			ecx
	jz			SwizzleD3DtoSOA_Asm_End
	movd		ebx, mm7
	movd		esi, mm0
	movd		edi, mm1
	jmp			ebx
SwizzleLoopD3DtoSOA_Copy8Txtrs	ENDP


Align 16
SwizzleD3DtoSOA_Asm_End	PROC NEAR PRIVATE
	mov			edi, dword ptr _savedEDI[esp]	; pop edi
	mov			esi, dword ptr _savedESI[esp]	; pop esi
	mov			ebx, dword ptr _savedEBX[esp]	; pop ebx
	mov			ecx, dword ptr _savedECX[esp]	; pop ecx
	emms
	add			esp, (_retAddr - _stkBase)
	ret
SwizzleD3DtoSOA_Asm_End	ENDP



_TEXT	ENDS


CONST	SEGMENT DWORD USE32 PUBLIC 'CONST'
ALIGN 4
PUBLIC	_SwizzleSngl_SwizParmsFunctions
_SwizzleSngl_SwizParmsFunctions:
	dd	SwizzleSingleD3DtoSOA_Swiz0Params		; 0 params = bad fvf type
	dd	SwizzleSingleD3DtoSOA_Swiz0Params		; 1 params = bad fvf type
	dd	SwizzleSingleD3DtoSOA_Swiz0Params		; 2 params = bad fvf type
	dd	SwizzleSingleD3DtoSOA_Swiz3Params
	dd	SwizzleSingleD3DtoSOA_Swiz4Params
	dd	SwizzleSingleD3DtoSOA_Swiz5Params
	dd	SwizzleSingleD3DtoSOA_Swiz6Params
	dd	SwizzleSingleD3DtoSOA_Swiz7Params
	dd	SwizzleSingleD3DtoSOA_Swiz8Params
	dd	SwizzleSingleD3DtoSOA_Swiz9Params
	dd	SwizzleSingleD3DtoSOA_Swiz10Params
	dd	SwizzleSingleD3DtoSOA_Swiz11Params
	dd	SwizzleSingleD3DtoSOA_Swiz12Params
	dd	SwizzleSingleD3DtoSOA_Swiz13Params
	dd	SwizzleSingleD3DtoSOA_Swiz0Params		; 14 params = bad fvf type
	dd	SwizzleSingleD3DtoSOA_Swiz0Params		; 15 params = bad fvf type

PUBLIC _SwizzleSngl_CopyTxtrsFunctions
_SwizzleSngl_CopyTxtrsFunctions:
	dd	SwizzleSingleD3DtoSOA_Copy0Txtrs
	dd	SwizzleSingleD3DtoSOA_Copy1Txtrs
	dd	SwizzleSingleD3DtoSOA_Copy2Txtrs
	dd	SwizzleSingleD3DtoSOA_Copy3Txtrs
	dd	SwizzleSingleD3DtoSOA_Copy4Txtrs
	dd	SwizzleSingleD3DtoSOA_Copy5Txtrs
	dd	SwizzleSingleD3DtoSOA_Copy6Txtrs
	dd	SwizzleSingleD3DtoSOA_Copy7Txtrs
	dd	SwizzleSingleD3DtoSOA_Copy8Txtrs
	dd	SwizzleSingleD3DtoSOA_Copy0Txtrs		; 9 textures = bad fvf type
	dd	SwizzleSingleD3DtoSOA_Copy0Txtrs		; 10 textures = bad fvf type
	dd	SwizzleSingleD3DtoSOA_Copy0Txtrs		; 11 textures = bad fvf type
	dd	SwizzleSingleD3DtoSOA_Copy0Txtrs		; 12 textures = bad fvf type
	dd	SwizzleSingleD3DtoSOA_Copy0Txtrs		; 13 textures = bad fvf type
	dd	SwizzleSingleD3DtoSOA_Copy0Txtrs		; 14 textures = bad fvf type
	dd	SwizzleSingleD3DtoSOA_Copy0Txtrs		; 15 textures = bad fvf type

SwizzleLoop_SwizParmsFunctions:
	dd	SwizzleLoopD3DtoSOA_Swiz0Params			; 0 params = bad fvf type
	dd	SwizzleLoopD3DtoSOA_Swiz0Params			; 1 params = bad fvf type
	dd	SwizzleLoopD3DtoSOA_Swiz0Params			; 2 params = bad fvf type
	dd	SwizzleLoopD3DtoSOA_Swiz3Params
	dd	SwizzleLoopD3DtoSOA_Swiz4Params
	dd	SwizzleLoopD3DtoSOA_Swiz5Params
	dd	SwizzleLoopD3DtoSOA_Swiz6Params
	dd	SwizzleLoopD3DtoSOA_Swiz7Params
	dd	SwizzleLoopD3DtoSOA_Swiz8Params
	dd	SwizzleLoopD3DtoSOA_Swiz9Params
	dd	SwizzleLoopD3DtoSOA_Swiz10Params
	dd	SwizzleLoopD3DtoSOA_Swiz11Params
	dd	SwizzleLoopD3DtoSOA_Swiz12Params
	dd	SwizzleLoopD3DtoSOA_Swiz13Params
	dd	SwizzleLoopD3DtoSOA_Swiz0Params			; 14 params = bad fvf type
	dd	SwizzleLoopD3DtoSOA_Swiz0Params			; 15 params = bad fvf type

SwizzleLoop_CopyTxtrsFunctions:
	dd	SwizzleLoopD3DtoSOA_Copy0Txtrs
	dd	SwizzleLoopD3DtoSOA_Copy1Txtrs
	dd	SwizzleLoopD3DtoSOA_Copy2Txtrs
	dd	SwizzleLoopD3DtoSOA_Copy3Txtrs
	dd	SwizzleLoopD3DtoSOA_Copy4Txtrs
	dd	SwizzleLoopD3DtoSOA_Copy5Txtrs
	dd	SwizzleLoopD3DtoSOA_Copy6Txtrs
	dd	SwizzleLoopD3DtoSOA_Copy7Txtrs
	dd	SwizzleLoopD3DtoSOA_Copy8Txtrs
	dd	SwizzleLoopD3DtoSOA_Copy0Txtrs			; 9 textures = bad fvf type
	dd	SwizzleLoopD3DtoSOA_Copy0Txtrs			; 10 textures = bad fvf type
	dd	SwizzleLoopD3DtoSOA_Copy0Txtrs			; 11 textures = bad fvf type
	dd	SwizzleLoopD3DtoSOA_Copy0Txtrs			; 12 textures = bad fvf type
	dd	SwizzleLoopD3DtoSOA_Copy0Txtrs			; 13 textures = bad fvf type
	dd	SwizzleLoopD3DtoSOA_Copy0Txtrs			; 14 textures = bad fvf type
	dd	SwizzleLoopD3DtoSOA_Copy0Txtrs			; 15 textures = bad fvf type

CONST	ENDS



END
