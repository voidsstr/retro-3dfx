TITLE	afankni.asm
	.686P
	.model FLAT

EXTRN	__asm_data:BYTE
EXTRN	_fifo_make_room_asm:NEAR
EXTRN	_DrawTriFanImmAsm_P5:NEAR
; ----------------
; Fan section, KNI
; Brent Burton, 19 jan 1999
; ----------------

INCLUDE aglobal.inc
IF @Version GE 614
XMMWORD TEXTEQU <DWORD>
xmmword TEXTEQU <DWORD>
MMWORD  TEXTEQU <DWORD>
mmword  TEXTEQU <DWORD>
        .xmm
ELSE
INCLUDE iaxmm.inc
ENDIF

_TEXT	SEGMENT
	.code

; ---- Params: offsets from ebp
_vb_offset = +16
_prims = +12
_count = +8
; ---- Locals: offsets from esp
_next   = 0
_area2  = 4
_area1  = 8
_locals = 12

va	TEXTEQU	<eax>
vb	TEXTEQU <ebx>
vc	TEXTEQU <ecx>
vd	TEXTEQU <edx>
hwptr	TEXTEQU <esi>
tempr	TEXTEQU <edi>		; short-use temporary reg

; ---- Macros for boilerplate code
sendhw macro src:req		; send one dword to hw; advance hw pointer
	mov	dword ptr [hwptr], src
	add	hwptr, 4
endm

sendhw_vert_f macro vp:req	; calc and send vertex data; advance hw ptr
	mov	tempr, dword ptr [__asm_data + _T0_OFFSET]
	fld	dword ptr [vp+tempr]		  ; tu
	fmul	dword ptr [__asm_data + _SCALE_S]; tuadj1
	fld	dword ptr [vp+tempr+4]		  ; tv tuadj1
	fmul	dword ptr [__asm_data + _SCALE_T]; tvadj1   tuadj1
	fadd	dword ptr [__asm_data + _TEXEL_OFFSET_T]; tvadj2 tuadj1
	fxch					  ; tuadj1 tvadj2
	fadd	dword ptr [__asm_data + _TEXEL_OFFSET_S]; tuadj2 tvadj2
	fmul	dword ptr rhw[vp]		  ; tuadj    tvadj2
	fld	dword ptr rhw[vp]		  ; rwh      tuadj    tvadj2
	fmulp	st(2),st			  ; tuadj    tvadj
	fld	dword ptr sz[vp]		  ; sz       tuadj    tvadj
	fmul	dword ptr [__asm_data + _SCALEZ] ; szadj    tuadj    tvadj
	fld	dword ptr sy[vp]		  ; sy  szadj  tuadj  tvadj
	fadd	dword ptr [__asm_data + _PIXEL_CENTER]; syadj  szadj  tuadj  tvadj
	fld	dword ptr sx[vp]
	fadd	dword ptr [__asm_data + _PIXEL_CENTER]; sxadj    syadj    szadj    tuadj    tvadj
	mov	tempr,dword ptr color[vp]
	fstp	dword ptr [hwptr]
	fstp	dword ptr [hwptr+4]
	mov	dword ptr [hwptr+8],tempr
	mov	tempr,dword ptr rhw[vp]
	fstp	dword ptr [hwptr+12]
	mov	dword ptr [hwptr+16],tempr
	fstp	dword ptr [hwptr+20]
	fstp	dword ptr [hwptr+24]
	add	hwptr, 28
endm

sendhw_vert_begin macro		; Set up KNI constant regs prior to any vertices
	lea	edi, __asm_data + _kni_array
	movaps	xmm2, xmmword ptr [edi]
	movaps	xmm3, xmmword ptr [edi + 10h]
	movaps	xmm4, xmmword ptr [edi + 20h]
	movaps	xmm5, xmmword ptr [edi + 30h]
endm

sendhw_vert macro vp:req	; calc vertex info and send to hw; update hwptr
	mov    tempr, dword ptr [__asm_data + _T0_OFFSET]
ifdef DCT_FIX
    	movaps xmm6, XMMWORD ptr [__asm_data + _SCALEWS]     
endif
IF @Version GE 614
	movhps xmm1, [vp + tempr]; load s t
	movlps xmm0, [vp + sx]	; load x y
ELSE
	movhps xmm1, mmword ptr [vp + tempr]; load s t
	movlps xmm0, mmword ptr [vp + sx]	; load x y
ENDIF
	mulps  xmm1, xmm2
	addps  xmm0, xmm3
IF @Version GE 614
	movlps xmm1, [vp + sz]	; load z w
ELSE
	movlps xmm1, mmword ptr [vp + sz]	; load z w
ENDIF
	shufps xmm5, xmm1, 54h
ifdef DCT_FIX
	mulps  xmm5, xmm6
endif
	mov    tempr, color[vp]
	addps  xmm1, xmm4
	mulps  xmm1, xmm5
IF @Version GE 614
	movlps [hwptr], xmm0	; write x y
ELSE
	movlps mmword ptr [hwptr], xmm0	; write x y
ENDIF
	mov    [hwptr + 8], tempr	; write color
IF @Version GE 614
	movlps [hwptr + 12], xmm1; write z w tu tv
	movhps [hwptr + 20], xmm1; write z w tu tv
ELSE
	movlps xmmword ptr [hwptr + 12], xmm1; write z w tu tv
	movhps xmmword ptr [hwptr + 20], xmm1; write z w tu tv
ENDIF
ifdef DCT_FIX
        movaps xmm5, XMMWORD ptr [__asm_data + _SCALEZ1]     
endif
	add hwptr, 28
endm

sendhw_verts macro VA:req,VB:req,VC:req	    ; send 3 vertices' info to hw; update hwptr
	;; vert A
	mov    tempr, dword ptr [__asm_data + _T0_OFFSET]
ifdef DCT_FIX
        movaps xmm6, XMMWORD ptr [__asm_data + _SCALEWS]     
endif
IF @Version GE 614
	movhps xmm1, [VA + tempr]; load s t
	movlps xmm0, [VA + sx]	; load x y
ELSE
	movhps xmm1, mmword ptr [VA + tempr]; load s t
	movlps xmm0, mmword ptr [VA + sx]	; load x y
ENDIF
	mulps  xmm1, xmm2
	addps  xmm0, xmm3
IF @Version GE 614
	movlps xmm1, [VA + sz]	; load z w
ELSE
	movlps xmm1, mmword ptr [VA + sz]	; load z w
ENDIF
	shufps xmm5, xmm1, 54h
ifdef DCT_FIX
        mulps  xmm5, xmm6
endif
	mov    tempr, color[VA]
	addps  xmm1, xmm4
	mulps  xmm1, xmm5
IF @Version GE 614
	movlps [hwptr], xmm0	; write x y
ELSE
	movlps mmword ptr [hwptr], xmm0	; write x y
ENDIF
	mov    dword ptr [hwptr + 8], tempr	; write color
IF @Version GE 614
	movlps [hwptr + 12], xmm1; write z w tu tv
	movhps [hwptr + 20], xmm1; write z w tu tv
ELSE
	movlps xmmword ptr [hwptr + 12], xmm1; write z w tu tv
	movhps xmmword ptr [hwptr + 20], xmm1; write z w tu tv
ENDIF
ifdef DCT_FIX
        movaps xmm5, XMMWORD ptr [__asm_data + _SCALEZ1]     
endif
	;; vert B
	mov tempr, dword ptr [__asm_data + _T0_OFFSET]
IF @Version GE 614
	movhps xmm1, [VB + tempr]; load s t
	movlps xmm0, [VB + sx]	; load x y
ELSE
	movhps xmm1, mmword ptr [VB + tempr]; load s t
	movlps xmm0, mmword ptr [VB + sx]	; load x y
ENDIF
	mulps  xmm1, xmm2
	addps  xmm0, xmm3
IF @Version GE 614
	movlps xmm1, [VB + sz]	; load z w
ELSE
	movlps xmm1, mmword ptr [VB + sz]	; load z w
ENDIF
	shufps xmm5, xmm1, 54h
ifdef DCT_FIX
        mulps  xmm5, xmm6
endif
	mov    tempr, color[VB]
	addps  xmm1, xmm4
	mulps  xmm1, xmm5
IF @Version GE 614
	movlps [hwptr + 28], xmm0	; write x y
ELSE
	movlps mmword ptr [hwptr + 28], xmm0	; write x y
ENDIF
	mov    dword ptr [hwptr + 36], tempr	; write color
IF @Version GE 614
	movlps [hwptr + 40], xmm1; write z w tu tv
	movhps [hwptr + 48], xmm1; write z w tu tv
ELSE
	movlps xmmword ptr [hwptr + 40], xmm1; write z w tu tv
	movhps xmmword ptr [hwptr + 48], xmm1; write z w tu tv
ENDIF
ifdef DCT_FIX
        movaps xmm5, XMMWORD ptr [__asm_data + _SCALEZ1]     
endif
	;; vert C
	mov    tempr, dword ptr [__asm_data + _T0_OFFSET]
IF @Version GE 614
	movhps xmm1, [VC + tempr]; load s t
	movlps xmm0, [VC + sx]	; load x y
ELSE
	movhps xmm1, mmword ptr [VC + tempr]; load s t
	movlps xmm0, mmword ptr [VC + sx]	; load x y
ENDIF
	mulps  xmm1, xmm2
	addps  xmm0, xmm3
IF @Version GE 614
	movlps xmm1, [VC + sz]	; load z w
ELSE
	movlps xmm1, mmword ptr [VC + sz]	; load z w
ENDIF
	shufps xmm5, xmm1, 54h
ifdef DCT_FIX
        mulps  xmm5, xmm6
endif
	mov    tempr, color[VC]
	addps  xmm1, xmm4
	mulps  xmm1, xmm5
IF @Version GE 614
	movlps [hwptr + 56], xmm0	; write x y
ELSE
	movlps mmword ptr [hwptr + 56], xmm0	; write x y
ENDIF
	mov    dword ptr [hwptr + 64], tempr	; write color
IF @Version GE 614
	movlps [hwptr + 68], xmm1; write z w tu tv
	movhps [hwptr + 76], xmm1; write z w tu tv
ELSE
	movlps xmmword ptr [hwptr + 68], xmm1; write z w tu tv
	movhps xmmword ptr [hwptr + 76], xmm1; write z w tu tv
ENDIF
ifdef DCT_FIX
        movaps xmm5, XMMWORD ptr [__asm_data + _SCALEZ1]     
endif
	;; update hwptr
	add    hwptr, 84
endm

align   16
PUBLIC   _DrawTriFanImmAsm_KNI
_DrawTriFanImmAsm_KNI    PROC     NEAR

	;; if (count > 2)
	;;   return DrawTriFanImmAsm_P5(count, prims, vb_offset)
	push ebp
	mov  ebp, esp		; setup stack frame
	cmp  dword ptr _count[ebp], 2
	jg   call_dtfiap5

start:
;	push ebp
;	mov  ebp, esp
	push ebx            ; save registers
	push ecx
	push edx
	push esi
	push edi
	sub  esp, _locals	; ebp for params, esp for locals

	;; If CMDFIFO has no room, adjust it.  Get ptr to HW.
	cmp  dword ptr[__asm_data + _FIFO_ROOM], 32
	jl   adjust_fifo
align 16
fifo_ok:
	mov  hwptr, dword ptr [__asm_data + _FIFO_PTR]

	;; Set up va,vb,vc,vd to point to first 4 vertices
	mov  va, dword ptr _prims[ebp]
	mov  tempr, dword ptr _vb_offset[ebp]
	lea  vb, dword ptr [va + tempr]
	lea  vc, dword ptr [va + tempr*2]
	lea  vd, dword ptr [vb + tempr*2]
	mov  dword ptr _next[esp], vd

	;; if (count == 2) {
	cmp  dword ptr _count[ebp], 2
	jl   draw_1_tri
draw_2_tri:
	mov  tempr, vd		; next += vb_offset;
	add  tempr, _vb_offset[ebp]
	mov _next[esp], tempr

	;; Brent's scalar KNI 2-tri area code:
	movss xmm0, dword ptr [va]	; ax
	movss xmm4, dword ptr [va+4]	; ay
	movss xmm1, dword ptr [vb]	; bx
	movss xmm5, dword ptr [vb+4];     ; by
	subss xmm0, xmm1        ; S1 = ax-bx
	subss xmm4, xmm5        ; S4 = ay-by
	movss xmm2, dword ptr [vc]        ; cx
	movss xmm6, dword ptr [vc+4]      ; cx
	subss xmm1, xmm2        ; S3 = bx-cx
	subss xmm5, xmm6        ; S2 = by-cy
	movss xmm3, dword ptr [vd]        ; dx
	movss xmm7, dword ptr [vd+4]      ; dy
	subss xmm2, xmm3        ; S6 = cx-dx
	mulss xmm4, xmm1        ; T2 = S3*S4
	mulss xmm0, xmm5        ; T1 = S1*S2
	subss xmm6, xmm7        ; S5 = cy-dy
	subss xmm0, xmm4        ; area1 = T1-T2
	mulss xmm6, xmm1        ; T3 = S3*S5
	movmskps tempr, xmm0
;	movss dword ptr _area1[esp], xmm0
	mulss xmm2, xmm5        ; T4 = S2*S6
	subss xmm6, xmm2        ; area2 = T3-T4
;	movss dword ptr _area2[esp], xmm6

	;; Test tri 1 of 2
;	mov tempr, dword ptr _area1[esp]
	shl tempr, 31
	and tempr, 80000000h
	xor tempr, dword ptr[__asm_data + _CULL_MASK]
	jz test_2_of_2_only

	;; send first triangle here
	sendhw FAN_HEADER
	sendhw_vert_begin
	sendhw_verts va,vb,vc

	movmskps tempr, xmm6
;	mov tempr, dword ptr _area2[esp]
	shl tempr, 31
	and tempr, 80000000h	; get sign
	xor tempr, dword ptr[__asm_data + _CULL_MASK]
	jz  finish

	sendhw FAN_HEADER_CONT
	sendhw_vert vd
	jmp finish

align 16
test_2_of_2_only:
	;; Test tri 2 of 2, first was culled
;	mov tempr, dword ptr _area2[esp]
	movmskps tempr, xmm6
	shl tempr, 31
	and tempr, 80000000h	; get sign
	xor tempr, dword ptr[__asm_data + _CULL_MASK]
	jz  finish

	sendhw FAN_HEADER
	sendhw_vert_begin
	sendhw_verts va,vc,vd
	jmp finish

align 16
draw_1_tri:
	;; 1-triangle case:

	;; Scott's code to cull 1 tri:
IF @Version GE 614
	movhps xmm0, [va]
	movlps xmm0, [vb]	; xmm0 = ay ax by bx
	movhps xmm1, [vb]
	movlps xmm1, [vc]	; xmm1 = by bx cy cx
ELSE
	movhps xmm0, mmword ptr [va]
	movlps xmm0, mmword ptr [vb]	; xmm0 = ay ax by bx
	movhps xmm1, mmword ptr [vb]
	movlps xmm1, mmword ptr [vc]	; xmm1 = by bx cy cx
ENDIF

	subps  xmm0, xmm1        ; xmm0 = aby abx bcy bcx
	movaps xmm1, xmm0       ; xmm1 = aby abx bcy bcx
	shufps xmm0, xmm0, 0bh  ; xmm0 = xxx xxx abx aby
	mulps  xmm0, xmm1        ; xmm0 = xxx xxx abx*bcy aby*bcx
	movaps xmm1, xmm0
	shufps xmm1, xmm1, 01h  ; xmm1 = xxx xxx xxxxxxx abx*bcy
	subss  xmm1, xmm0
	movss _area1[esp], xmm1
	
	mov   tempr, _area1[esp]	; now test if cull_mask matches
	and   tempr, 80000000h	; get sign
	xor   tempr, dword ptr [__asm_data + _CULL_MASK]
	jz    finish

	sendhw FAN_HEADER
	sendhw_vert_begin
	sendhw_verts va,vb,vc

align 16
finish:
	;; Save new hwptr values
	mov eax, hwptr
	sub eax, dword ptr [__asm_data + _FIFO_PTR]
	shr eax, 2		; eax = # of dwords (commands) sent
	sub dword ptr [__asm_data + _FIFO_ROOM], eax ; f_r -= count
	mov dword ptr [__asm_data + _FIFO_PTR], hwptr
	
	;; Set return value, clean up stack.
	mov eax, _next[esp]	; return (next);
	add esp, _locals
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop ebp			; esp-> as at start
	ret 0

align 16
call_dtfiap5:			; too many tri's for us -- pass buck.
	push dword ptr _vb_offset[ebp]
	push dword ptr _prims[ebp]
	push dword ptr _count[ebp]
	call _DrawTriFanImmAsm_P5
	add esp, 12
	pop ebp
	ret 0

align 16
adjust_fifo:	
	pushd 32
	call _fifo_make_room_asm
	add esp, 4
	jmp fifo_ok
	
_DrawTriFanImmAsm_KNI    ENDP
_TEXT   ENDS
END
