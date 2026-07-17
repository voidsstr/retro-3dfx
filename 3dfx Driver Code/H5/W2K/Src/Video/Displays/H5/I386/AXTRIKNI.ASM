; 
; AXTRIKNI - Katmai version of DX6 FVF triangle routines
;

TITLE           axtrikni.asm
.686P
.MMX
IF @Version GE 614
.XMM
ELSE
echo Warning! AXTRIKNI.ASM requires MASM 6.14!
ENDIF
.MODEL FLAT

ifdef WINNT
ifdef INCSTBPERF
ifndef MS_VIEW
include ..\..\..\..\build\stbperf.inc
else
include ..\displays\stbperf.inc	
endif		
endif
endif
IF STBKNI

EXTRN	_fifo_make_room_asm:NEAR

; If we have K6_2 or WINNT defined, lets extern and use axtrik's SpecularStream
ifdef WINNT
EXTRN	SpecularStream:DWORD
else  ; Not WINNT
ifdef K6_2
EXTRN	SpecularStream:DWORD
endif ; K6_2
endif ; WINNT
;EXTRN	f_255_reverse_lookup:BYTE	can't figure out how to link to this so I have my own


ifdef PERF_MONITORING
extrn __penter:proc
endif


ifdef DEBUG
GATHER_PERF_INFO equ 1
else
GATHER_PERF_INFO equ 0
endif

; unrolled equates
CHECK_SPLIT_LOWER_XMM_QWORD_WRITE	equ 1
FORCE_SPLIT_LOWER_XMM_QWORD_WRITE	equ 0


NUM_PREFETCH_TRIS		equ 64
MAX_ENTRIES_PER_VERTEX	equ 10	; x,y,color,z,wfbi,rhw,s0,t0,s1,t1
MAX_ENTRIES_PER_TRI		equ MAX_ENTRIES_PER_VERTEX * 3 + 1	; add the command
MAX_ENTRIES_SPEC_BUFF	equ NUM_PREFETCH_TRIS * MAX_ENTRIES_PER_TRI + 20	; 10 entries to switch back & forth
MAX_SPEC_SWITCH_WRITES	equ	32 ;12+1+10	; number of writes for the switch to/from the specular buffer

NUM_PAD_TRIS		equ 2
POST_IDX_SIZE		equ ((NUM_PREFETCH_TRIS+NUM_PAD_TRIS) * 3) * 4 + 8	; =800
LOCAL_VARS			equ 16
STACK_SIZE			equ POST_IDX_SIZE + LOCAL_VARS*4   					; =8xx


; xmm compare immediate values
XMM_EQ		TEXTEQU <0>	; ==
XMM_LT		TEXTEQU <1>	; <
XMM_LE		TEXTEQU <2>	; <=
XMM_Q		TEXTEQU <3>	; ?
XMM_NE		TEXTEQU <4>	; !=
XMM_GE		TEXTEQU <5>	; >=
XMM_GT		TEXTEQU <6>	; >
XMM_NQ		TEXTEQU <7>	; !?

; Stack param section
_params			= STACK_SIZE + (5*4)	; ret addr, ebp, ebx edi, esi
_ret_addr		= _params - 4
_count			= _params + 0
_idx			= _params + 4
_verts			= _params + 8

; Local stack section
_post_idx		= 0
_locals			= POST_IDX_SIZE
_last			= _locals + 0
_triCount		= _locals + 4
_specPtr		= _locals + 8
_localFlags		= _locals + 12
_specCmdFull	= _locals + 16
_specCmdCont	= _locals + 20
_pVertexList	= _locals + 24

.CONST
align 4
__asm_rev_lookup	dd					255.0,	254.0,	253.0,	252.0,	251.0,	250.0
	dd	249.0,	248.0,	247.0,	246.0,	245.0,	244.0,	243.0,	242.0,	241.0,	240.0	
	dd	239.0,	238.0,	237.0,	236.0,	235.0,	234.0,	233.0,	232.0,	231.0,	230.0	
	dd	229.0,	228.0,	227.0,	226.0,	225.0,	224.0,	223.0,	222.0,	221.0,	220.0	
	dd	219.0,	218.0,	217.0,	216.0,	215.0,	214.0,	213.0,	212.0,	211.0,	210.0	
	dd	209.0,	208.0,	207.0,	206.0,	205.0,	204.0,	203.0,	202.0,	201.0,	200.0	
	dd	199.0,	198.0,	197.0,	196.0,	195.0,	194.0,	193.0,	192.0,	191.0,	190.0	
	dd	189.0,	188.0,	187.0,	186.0,	185.0,	184.0,	183.0,	182.0,	181.0,	180.0	
	dd	179.0,	178.0,	177.0,	176.0,	175.0,	174.0,	173.0,	172.0,	171.0,	170.0	
	dd	169.0,	168.0,	167.0,	166.0,	165.0,	164.0,	163.0,	162.0,	161.0,	160.0	
	dd	159.0,	158.0,	157.0,	156.0,	155.0,	154.0,	153.0,	152.0,	151.0,	150.0	
	dd	149.0,	148.0,	147.0,	146.0,	145.0,	144.0,	143.0,	142.0,	141.0,	140.0	
	dd	139.0,	138.0,	137.0,	136.0,	135.0,	134.0,	133.0,	132.0,	131.0,	130.0	
	dd	129.0,	128.0,	127.0,	126.0,	125.0,	124.0,	123.0,	122.0,	121.0,	120.0	
	dd	119.0,	118.0,	117.0,	116.0,	115.0,	114.0,	113.0,	112.0,	111.0,	110.0	
	dd	109.0,	108.0,	107.0,	106.0,	105.0,	104.0,	103.0,	102.0,	101.0,	100.0	
	dd	 99.0,	 98.0,	 97.0,	 96.0,	 95.0,	 94.0,	 93.0,	 92.0,	 91.0,	 90.0	
	dd	 89.0,	 88.0,	 87.0,	 86.0,	 85.0,	 84.0,	 83.0,	 82.0,	 81.0,	 80.0	
	dd	 79.0,	 78.0,	 77.0,	 76.0,	 75.0,	 74.0,	 73.0,	 72.0,	 71.0,	 70.0	
	dd	 69.0,	 68.0,	 67.0,	 66.0,	 65.0,	 64.0,	 63.0,	 62.0,	 61.0,	 60.0	
	dd	 59.0,	 58.0,	 57.0,	 56.0,	 55.0,	 54.0,	 53.0,	 52.0,	 51.0,	 50.0
	dd	 49.0,	 48.0,	 47.0,	 46.0,	 45.0,	 44.0,	 43.0,	 42.0,	 41.0,	 40.0	
	dd	 39.0,	 38.0,	 37.0,	 36.0,	 35.0,	 34.0,	 33.0,	 32.0,	 31.0,	 30.0	
	dd	 29.0,	 28.0,	 27.0,	 26.0,	 25.0,	 24.0,	 23.0,	 22.0,	 21.0,	 20.0	
	dd	 19.0,	 18.0,	 17.0,	 16.0,	 15.0,	 14.0,	 13.0,	 12.0,	 11.0,	 10.0	
	dd	  9.0,	  8.0,	  7.0,	  6.0,	  5.0,	  4.0,	  3.0,	  2.0,	  1.0,	  0.0

align 8
Const_Mult256	dd	(8 SHL 23), (8 SHL 23)

align 16
flt_4_halves	dd	4 dup(0.5)
flt_4_mask_sign	dd	4 dup(07fffffffh)


.DATA

align 16
public  __asm_data
__asm_data	\
	dd	0, 0, 0, 0			;   0 _FIFO_PTR, _FIFO_ROOM, _VERTEX_TYPE, _VERTEX_SIZE
	dd	0, 0				;  16 _CULL_MASK, _T0_OFFSET
	dd	0.0, 0.0, 0.0, 0.0	;  24 _SCALE_S, _SCALE_T, _TEXEL_OFFSET_S, _TEXEL_OFFSET_T
	dd	0.0, 0.0			;  40 _SCALEZ, _PIXEL_CENTER
	dd	0, 0, 0, 0		 	;  48 _CULL_HEADER, _SAVED_RET_ADD, _SAVED_COUNT, _RC
	;; start of the kni_array - not used anymore
	dd	1.0, 1.0, 0.0, 0.0	;  64 _SCALE_TS11			16 bytes  SCALE_T SCALE_S 1.0 1.0
	dd	0.0, 0.0, 0.0, 0.0	;  80 _00PIXEL_CENTER		16 bytes 0.0 0.0 PIXEL_CENTER PIXEL_CENTER 
	dd	0.0, 0.0, 0.0, 0.0	;  96 _TEXEL_OFFSET_TS00	16 bytes _TEXEL_OFFSET_T _TEXEL_OFFSET_S 0.0 0.0
	dd	0.0, 1.0, 0.0, 0.0	; 112 _SCALEZ1				 8 bytes _SCALEZ 1.0
ifdef DCT_FIX
	dd	1.0, 1.0, 1.0, 1.0        ; RAJ 4/30/99 DCT-fix 
	dd	0.0, 0.0, 0.0, 0.0        ; extend kni_array to include
                                  ; scaleW factors and temp space
endif
; ALLEN - added to match definition in D3Global.h
	dd	0, 0, 0, 0			; 128 FVF_SIZE, CMD_START, CMD_CONT, CMD_TRI
	dd	0, 0, 0, 0			; 144 SETUP_FLAG, SPECULAR_OFFSET, T1_OFFSET, RHW_OFFSET
	dd	0.0, 0.0, 0.0, 0.0	; 160 S1_SCALE, T1_SCALE, S1_TEXEL_OFFSET, T1_TEXEL_OFFSET
	dd	0.0, 0, 0.0, 0.0	; 176 SCALE_W, RENDER_STATE, WBUFFER_SCALE_AW, WBUFFER_SCALE_BW
	dd	0, 0				; 192 CMD_ALPHA_BLEND_SPECULAR, CMD_NO_ALPHA_BLEND_SPECULAR
	dd	0, 0				; 200 CMD_CONT_ALPHA_BLEND_SPECULAR, CMD_CONT_NO_ALPHA_BLEND_SPECULAR

align 8
StipCont		dd	0, 0
LastFlags		dd	0
DrawTriFunction	dd	0


align 16
PixelOffset		dd	4 dup (0)
TextureScale	dd	4 dup (0)
TextureOffset	dd	4 dup (0)
ScaleZW			dd	4 dup (0)
TempVa			dd	4 dup (0)
TempVbc			dd	4 dup (0)
flt_wrap_mask	dd	4 dup (0)
flt_wrap_adds	dd	4 dup (0)
flt4_temp1		dd	4 dup (0)

ifndef WINNT
ifndef K6_2
public  SpecularStream
align 16
SpecularStream     dd  NUM_PREFETCH_TRIS*12*3 dup(0)  ; Create an array in global data big enough to
endif ; Not K6_2
endif ; Not WINNT

if GATHER_PERF_INFO
FVF_TYPE	STRUCT	4
	dword	fvfSize
	dword	drawPrimCalls
	dword	triSizeChanged
	dword	numTris
FVF_TYPE	ENDS

TRI_TYPE	STRUCT	4
	dword	fvfSize
	dword	flags
	dword	numDrawPrimCalls
	dword	numTris
TRI_TYPE	ENDS

align 16
PUBLIC   LastFvfSize
LastFvfSize		dd 0
LastTriType		dd 0
TriTypeChanged	dd 0
TriCount		dd 0
align 16
FVF5_Type		FVF_TYPE { 5,0,0,0}
FVF6_Type		FVF_TYPE { 6,0,0,0}
FVF7_Type		FVF_TYPE { 7,0,0,0}
FVF8_Type		FVF_TYPE { 8,0,0,0}
FVF9_Type		FVF_TYPE { 9,0,0,0}
FVF10_Type		FVF_TYPE {10,0,0,0}
FVF11_Type		FVF_TYPE {11,0,0,0}
FVF12_Type		FVF_TYPE {12,0,0,0}
align 16
TriTypes 		TRI_TYPE 100 dup ({0,0,0,0})
debugCount		dd 0
endif ;GATHER_PERF_INFO


_TEXT       SEGMENT
.CODE

INCLUDE aglobal.inc
;include iaxmm.inc

IF @Version GE 614
ADJUST_BITS	MACRO	REG:REQ, START_BIT:REQ, END_BIT:REQ

IF		(START_BIT GT END_BIT)
	shr		REG, (START_BIT - END_BIT)
ELSEIF	(START_BIT LT END_BIT)
	shl		REG, (END_BIT - START_BIT)
ELSE
	; do nothing (they're equal)
ENDIF
ENDM ;ADJUST_BITS
                    

LOAD_LOCAL_FLAGS	MACRO	STACK_OFFSET:REQ
; stack offset is 0 in the prefetch routines, 4 in the main routine
	mov		esi, dword ptr [__asm_data + SETUP_FLAG]
	mov		edi, dword ptr [__asm_data + RENDER_STATE]                           
	mov		eax, 0		; accumulator                         
	mov		edx, dword ptr [__asm_data + _RC]

	; specular
	mov		ecx, edi
	and		ecx, STATE_REQUIRES_SPECULAR
	ADJUST_BITS	ecx, BIT_STATE_REQUIRES_SPECULAR, BIT_SPEC
	or		eax, ecx
	; flat shaded
	xor		ebx, ebx
	test	dword ptr [edx + OFFSET_RC_SHADE_MODE], D3DSHADE_FLAT
	setnz	bl
	shl		ebx, BIT_FLAT
	or		eax, ebx
	; color
	xor		ebx, ebx
	test	esi, (SST_SETUP_RGB OR SST_SETUP_A)
	setnz	bl
	shl		ebx, BIT_COLOR
	or		eax, ebx
	; wbuffer
	mov		ebx, edi
	and		ebx, STATE_REQUIRES_WBUFFER
	ADJUST_BITS	ebx, BIT_STATE_REQUIRES_WBUFFER, BIT_WBUFFER
	or		eax, ebx
	; z
	mov		ecx, esi
	and		ecx, SST_SETUP_Z
	ADJUST_BITS	ecx, BIT_SST_SETUP_Z, BIT_Z
	or		eax, ecx
	; wfbi
	mov		ebx, esi
	and		ebx, SST_SETUP_Wfbi
	ADJUST_BITS	ebx, BIT_SST_SETUP_Wfbi, BIT_WFBI
	or		eax, ebx
	; vertex fog
	mov		ebx, edi
	and		ebx, STATE_REQUIRES_VERTEXFOG
	ADJUST_BITS	ebx, BIT_STATE_REQUIRES_VERTEXFOG, BIT_VFOG
	or		eax, ebx
	; w
	mov		ecx, esi
	and		ecx, SST_SETUP_W0
	ADJUST_BITS	ecx, BIT_SST_SETUP_W0, BIT_W
	or		eax, ecx
	; requires perspective correct
	mov		ebx, edi
	and		ebx, STATE_REQUIRES_PERSPECTIVE
	ADJUST_BITS	ebx, BIT_STATE_REQUIRES_PERSPECTIVE, BIT_PC
	or		eax, ebx
	; alpha blend
	xor		ebx, ebx
	test	dword ptr [edx + OFFSET_RC_ALPHABLEND_ENABLE], -1
	setnz	bl
	shl		ebx, BIT_ALPHA
	or		eax, ebx
	; texture 0
	mov		ebx, esi
	and		ebx, SST_SETUP_ST0
	ADJUST_BITS	ebx, BIT_SST_SETUP_ST0, BIT_TX0
	or		eax, ebx
	; texture 1
	mov		ecx, esi
	and		ecx, SST_SETUP_ST1
	ADJUST_BITS	ecx, BIT_SST_SETUP_ST1, BIT_TX1
	or		eax, ecx
	; wrap tx0-s
	xor		ecx, ecx
	test	dword ptr [edx + OFFSET_RC_WRAPT0], D3DWRAP_U
	setnz	cl
	shl		ecx, BIT_WRAP_T0_S
	or		eax, ecx
	; wrap tx0-t
	xor		ebx, ebx
	test	dword ptr [edx + OFFSET_RC_WRAPT0], D3DWRAP_V
	setnz	bl
	shl		ebx, BIT_WRAP_T0_T
	or		eax, ebx
	; wrap tx1-s
	xor		ecx, ecx
	test	dword ptr [edx + OFFSET_RC_WRAPT1], D3DWRAP_U
	setnz	cl
	shl		ecx, BIT_WRAP_T1_S
	or		eax, ecx
	; wrap tx1-t
	xor		ebx, ebx
	test	dword ptr [edx + OFFSET_RC_WRAPT1], D3DWRAP_V
	setnz	bl
	shl		ebx, BIT_WRAP_T1_T
	or		eax, ebx
	; wrap s or t a valid texture
	xor		ecx, ecx
	test	eax, FLAG_TX0
	jz		NoWrapOnTexture0
	test	eax, (FLAG_WRAP_T0_S OR FLAG_WRAP_T0_T)
	jz		NoWrapOnTexture0
	or		ecx, FLAG_WRAP_TX
	jmp		WrapOnTexture1		; early out
NoWrapOnTexture0:
	test	eax, FLAG_TX1
	jz		NoWrapOnTexture1
	test	eax, (FLAG_WRAP_T1_S OR FLAG_WRAP_T1_T)
	jz		NoWrapOnTexture1
	or		ecx, FLAG_WRAP_TX
NoWrapOnTexture1:
WrapOnTexture1:
	or		eax, ecx
	; Napalm
	xor		ecx, ecx    
	mov		ebx, dword ptr [edx	+ OFFSET_RC_PPDEV]			; *ppdev
	mov		ebx, dword ptr [ebx	+ OFFSET_PDEV_VENDOR_DEVICE_ID] ; ppdev->VendorDeviceID
	cmp		ebx, SST_VENDOR_DEVICE_ID_H5_6					; Napalm or greater?
	setge	cl
	shl		ecx, BIT_NAPALM
	or		eax, ecx
AlphaDone:
	; write flags
	mov		dword ptr _localFlags[esp+STACK_OFFSET], eax
ENDM ;LOAD_LOCAL_FLAGS


COPY_GLOBAL_DATA	MACRO	STACK_OFFSET:REQ
LOCAL	Peload_NoSecondTexture
LOCAL	NoTx0, NoTx0Wrap, NoTx1, NoTx1Wrap

	movss	xmm6, dword ptr [__asm_data + _PIXEL_CENTER]	; --- --- --- _PIXEL_CENTER
	shufps	xmm6, xmm6, 0									; _PIXEL_CENTER _PIXEL_CENTER _PIXEL_CENTER _PIXEL_CENTER
	movaps	oword ptr [PixelOffset], xmm6

; This should be done in the C-code or at the least, at the head of the prefetch routines
;	test	dword ptr [__asm_data + SETUP_FLAG], SST_SETUP_ST1
	test	dword ptr _localFlags[esp+STACK_OFFSET], (FLAG_TX1)
	movlps	xmm6, qword ptr [__asm_data + _SCALE_S]			;  --------  --------  SCALE_T1  SCALE_S1
	movlps	xmm7, qword ptr [__asm_data + _TEXEL_OFFSET_S]	;--------- --------- OFFSET_T1 OFFSET_S1
	je		SHORT Peload_NoSecondTexture
	movhps	xmm6, qword ptr [__asm_data + S1_SCALE]	 		;  SCALE_T2  SCALE_S2  SCALE_T1  SCALE_S1
	movhps	xmm7, qword ptr [__asm_data + S1_TEXEL_OFFSET]		; OFFSET_T2 OFFSET_S2 OFFSET_T1 OFFSET_S1
Peload_NoSecondTexture:
	movaps	oword ptr [TextureScale], xmm6
	movaps	oword ptr [TextureOffset], xmm7

	mov		eax, 3f800000h						; 1.0
	mov		ebx, dword ptr [__asm_data + _SCALEZ]
ifdef DCT_FIX
	mov		ecx, dword ptr [__asm_data + SCALE_W]
else
	mov		ecx, eax
endif
	mov		dword ptr [ScaleZW+0], eax			; 1.0			; multiply * x
	mov		dword ptr [ScaleZW+4], eax			; 1.0			; multiply * y
	mov		dword ptr [ScaleZW+8], ebx
	mov		dword ptr [ScaleZW+12], ecx			; 1.0/SCALE_W


; Disable autostripping on flat-shaded tri's or if wrap is asserted
; Also load the proper wrap add value if needed (1.0)

; Load the autostrip mask insto mm7
	mov		ebx, BIT_22
	mov		ecx, BIT_23
	movd	mm6, ebx
	movd	mm7, ecx
	punpckldq mm7, mm6				; BIT23		BIT22

; Load the default wrap value into mm5
	mov		eax, 3f800000h			; 1.0f
	movd	mm5, eax
	punpckldq mm5, mm5				; 1.0		1.0		; autostrip add for s&t

; Set mm2 to 1,1 (need for a mask)
	pcmpeqd mm2, mm2				; ffffffff	ffffffff
	psrld	mm2, 31					; 00000001	00000001

; Disable autostripping on flat-shaded tri's
	mov		edx, dword ptr _localFlags[esp+STACK_OFFSET]
	test	edx, (FLAG_FLAT)		; hardware can't autostrip flat-shaded tri's
	je		SHORT NotFlat
	pxor	mm7, mm7				; clear autostrip mask (disables autostripping)
NotFlat:

; Disable autostripping if wrap is asserted
	pxor	mm3, mm3				; clear "texture zero autostrip add" for tx1
	test	edx, (FLAG_TX0)			; Texture 0?
	je		NoTx0
	test	edx, ((FLAG_WRAP_T0_S) OR (FLAG_WRAP_T0_T))
	je		NoTx0Wrap
	pxor	mm7, mm7				; clear autostrip mask (disables autostripping)

	; now load flt_wrap_adds with 1.0 or 0.0 depending on if WRAP is asserted
	mov		esi, edx
	mov		edi, edx
	shr		esi, BIT_WRAP_T0_S
	shr		edi, BIT_WRAP_T0_T
	movd	mm3, esi
	movd	mm0, edi
	punpckldq mm3, mm0				; WRAP_T0_T		WRAP_T0_S
	pand	mm3, mm2				; mask off the other bits
	pcmpeqd	mm3, mm2				; WRAP ? -1 : 0  (T/S)
NoTx0Wrap:
NoTx0:

	pxor	mm4, mm4				; clear "texture zero autostrip add" for tx2
	test	edx, (FLAG_TX1)			; Texture 1?
	je		NoTx1
	test	edx, ((FLAG_WRAP_T1_S) OR (FLAG_WRAP_T1_T))
	je		NoTx1Wrap
	pxor	mm7, mm7				; clear autostrip mask (disables autostripping)

	; now load flt_wrap_adds with 1.0 or 0.0 depending on if WRAP is asserted
	mov		esi, edx
	mov		edi, edx
	shr		esi, BIT_WRAP_T1_S
	shr		edi, BIT_WRAP_T1_T
	movd	mm4, esi
	movd	mm0, edi
	punpckldq mm4, mm0				; WRAP_T0_T		WRAP_T0_S
	pand	mm4, mm2				; mask off the other bits
	pcmpeqd	mm4, mm2				; WRAP ? -1 : 0  (T/S)
NoTx1Wrap:
NoTx1:

; Write back the autostrip mask
	movq	qword ptr [StipCont], mm7
	movq	qword ptr [flt_wrap_mask+0], mm3
	movq	qword ptr [flt_wrap_mask+8], mm4
	pand	mm3, mm5				; WRAP ? 1.0 : 0  (T/S)
	pand	mm4, mm5				; WRAP ? 1.0 : 0  (T/S)
	movq	qword ptr [flt_wrap_adds+0], mm3
	movq	qword ptr [flt_wrap_adds+8], mm4

ENDM ;COPY_GLOBAL_DATA

BUILD_SPECULAR_COMMAND	MACRO	STACK_OFFSET:REQ
LOCAL	Peload_No2ndSpecularPass, Peload_SecondPassSetup_HasAlpha
spPtr		TEXTEQU	<esi>		; pointer to specular buffer
; Check if we're doing specular at all.  If we are we need a flag so use
; the specular buffer pointer for the flag.  All writes to it will be 
; done as "mov [spStream + spPtr - 32],xxx".  This won't add to instruction 
; size since the addressing mode is already [reg + disp32].  Just remember
; to adjust the size when copying the specular buffer to the hardware.
; We also build the commands now so we don't have to do it every triangle.
;      if (   (pRc->specular) && (pRc->texture != 0)
;	test	dword ptr [__asm_data + RENDER_STATE], STATE_REQUIRES_SPECULAR
	mov		edx, dword ptr _localFlags[esp+STACK_OFFSET]
	xor		spPtr, spPtr							 ; specular buffer pointer
	test	edx, (FLAG_SPEC)
	je		SHORT Peload_No2ndSpecularPass
	test	edx, ((FLAG_TX0) OR (FLAG_TX1))
	je		SHORT Peload_No2ndSpecularPass
	add		spPtr, 32									; will also be used as a flag

	; build the specular command
	mov		eax, dword ptr [__asm_data + SETUP_FLAG]
	mov		ebx, eax
	and		eax, 000f0000h							; SMode 
	and		ebx, 000000ffh							; Parameter Mask

	test	edx, (FLAG_ALPHA)
	jne		SHORT Peload_SecondPassSetup_HasAlpha

	and		ebx, 0000000fh							; No alpha so mask off W and textures
Peload_SecondPassSetup_HasAlpha:

	shl		eax, 6									; move SMode into bits 25:22
	shl		ebx, 10									; move Paramater Mask into bits 17:10
	or		eax, ebx
	mov		ebx, eax
	; full tri
	or		eax, ((1 SHL 28) OR \					; Packed Color
			      (1 SHL 22) OR \					; fan mode
			      (3 SHL  6) OR \					; 3 verticies
			      (1 SHL  3) OR 3)					; Start new tri strip + packet type
	; partial tri
	or		ebx, ((1 SHL 28) OR \					; Packed Color
			      (0 SHL 22) OR \					; fan mode
			      (1 SHL  6) OR \					; 1 verticies
			      (2 SHL  3) OR 3)					; Continue existing strip/fan + packet type

	mov		dword ptr _specCmdFull[esp+STACK_OFFSET], eax
	mov		dword ptr _specCmdCont[esp+STACK_OFFSET], ebx

Peload_No2ndSpecularPass:
	mov		dword ptr _specPtr[esp+STACK_OFFSET], spPtr

ENDM ;BUILD_SPECULAR_COMMAND

GATHER_PERFORMANCE_DATA	MACRO	VFV_SIZE:REQ
if GATHER_PERF_INFO
;void GatherKniPerfInfo(flags, numTris, fvfSize)
	mov		edx, _count[esp]
	push	dword ptr VFV_SIZE
	push	edx
	push	eax							; flags
	call	GatherKniPerfInfo
	add		esp, 12
endif ;GATHER_PERF_INFO
ENDM





hwPtr		TEXTEQU		<edi>		; pointer to command fifo
vertPtr		TEXTEQU		<ebp>		; pointer to vertex pointer list
STACK_OFFSET = 0					; 0 in the prefetch routines, 4 in the subroutines


;5555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555
_PrefetchTriIdx2_5_Asm_KNI     PROC     NEAR
; 	FVFOT_XYZWD = D3DFVF_XYZRHW | D3DFVF_DIFFUSE
;	size, sx,  sy,  sz, rhw, diff,spec, tu,  tv, tu1, tv1, tu2, tv2
;	  5,   0,   1,   2,   3,   4, 128,   0,   0, 128, 128, 128, 128
ifdef PERF_MONITORING
	call    __penter
endif
MAX_WRITES_PER_TRI = 64	; cmd + 3(X-Y-C-Z-Wfbi-W)

	push	ebp
	push	ebx
	push	edi
	push	esi
	sub		esp, STACK_SIZE

	mov		esi, dword ptr _idx[esp]			; ptr to triangle list index 
	prefetchnta [esi]							; try to get the first cache line prefetched

	LOAD_LOCAL_FLAGS	STACK_OFFSET
	COPY_GLOBAL_DATA	STACK_OFFSET
	BUILD_SPECULAR_COMMAND	STACK_OFFSET

; lookup the special case DrawTriangle function (or use the default)
	mov		eax, dword ptr _localFlags[esp+STACK_OFFSET]
	and		eax, (NOT (FLAG_WRAP_T0_S OR FLAG_WRAP_T0_T OR \	; don't use the individual wrap flags, 
						   FLAG_WRAP_T1_S OR FLAG_WRAP_T1_T OR \	; use FLAG_WRAP_TX instead
						   FLAG_NAPALM))	; we never care if Napalm or not because we always check for it
GATHER_PERFORMANCE_DATA	5
	cmp		eax, dword ptr [LastFlags]
	je		DrawTriangleFunctionKnown
	call	LookupNewKNIDrawTriangleFunction
DrawTriangleFunctionKnown:

	mov		edx, dword ptr _count[esp]			; # of tri's
NextChunk:
	mov		esi, dword ptr _idx[esp]			; ptr to triangle list index 
	mov		eax, edx
	cmp		edx, NUM_PREFETCH_TRIS				; more than max?
	mov		edi, dword ptr _verts[esp]			; address of vertex list
	jl		SHORT @F
	mov		edx, NUM_PREFETCH_TRIS				; set this loop to the max count
@@:	mov		dword ptr _triCount[esp], edx
	sub		eax, edx							; update count to _count - _triCount
	lea		vertPtr, _post_idx[esp+8]			; ptr to destination triangle list (esp+8)
	mov		dword ptr _count[esp], eax			; = total tri's minus tri's done in this loop

	prefetchnta [esi+32]						; prefetch the next cacheline in the index list
	and		vertPtr, 0fffffff8h					; round down to even 8-byte boundary
	pxor	mm7, mm7							; 0			0
	movd	mm6, edi
	punpckldq mm6,mm6							; *verts	*verts

; we want the source index tri list at a 4-byte boundary and the 
; destination vertext list at an 8-byte boundary in the prefetch loop
	test	esi, 2								; check if we start at a word boundary
	je		IndexListAligned

; The index list is unaligned so we set the vertPtr to an odd boundary (4-bytes) 
; and do one vertex, which gets everything aligned again for the prefetch loop
	movzx	eax, word ptr [esi+0]				;   A0
	movd	mm1, dword ptr [esi+2]				; 	0	0	C0	B0
	sub		vertPtr, 4							; force back to an odd boundary
	shl		eax, 2								;	A0*4
	movq	mm3, mm6
	punpcklwd mm1, mm7							;	0	A1	0	C0
	movq	mm4, mm6
	mov		dword ptr _pVertexList[esp], vertPtr
	movd	mm0, eax							;	0		A0*4
	psllq	mm1, 2								;	C0*4	B0*4
	sub		vertPtr, 12
	paddd	mm3, mm0							; *4 + verts
	paddd	mm4, mm1
	psllq	mm0, 2								;	0		A0*16
	psllq	mm1, 2								;	C0*16	B0*16
	paddd	mm3, mm0							; *20 + verts
	paddd	mm4, mm1
	add		esi, 6
	movd	dword ptr [vertPtr+12+0], mm3
	movq	qword ptr [vertPtr+12+4], mm4
	dec		edx
	jg		PrimePrefetchLoop
	jmp		PrefetchDone						; this was a one-triangle index list

IndexListAligned:
	mov		dword ptr _pVertexList[esp], vertPtr
	sub		vertPtr, 24

; Main prefetch loop, each iteration does 6 tri's = 18 verticies
; 18 verticies means 18 indicies which is 36 bytes, a little over 
; one cache line.  Prefetch two cache lines ahead.
PrimePrefetchLoop:
	movd	mm0, dword ptr [esi+0]				; 	0	0	B0	A0
	movd	mm1, dword ptr [esi+4]				; 	0	0	A1	C0
	movd	mm2, dword ptr [esi+8]				; 	0	0	C1	B1
align 16
PrefetchLoop:
	punpcklwd mm0, mm7							;	0	B0	0	A0
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A1	0	C0
	prefetchnta [esi+64]						; prefetch the next cacheline in the index list
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C1	0	B1
	movq	mm5, mm6							; *verts	*verts
	add		vertPtr, 24
	psllq	mm0, 2								;	B0*4	A0*4
	psllq	mm1, 2								;	A1*4	C0*4
	psllq	mm2, 2								;	C1*4	B1*4
	add		esi, 12
	paddd	mm3, mm0							; *4 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	psllq	mm0, 2								;	B0*16	A0*16
	psllq	mm1, 2								;	A1*16	C0*16
	psllq	mm2, 2								;	C1*16	B1*16
	paddd	mm3, mm0							; *20 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	movd	mm0, dword ptr [esi+0]				; 	0	0	B2	A2
	movd	mm1, dword ptr [esi+4]				; 	0	0	A3	C2
	movd	mm2, dword ptr [esi+8]				; 	0	0	C3	B3
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone

	punpcklwd mm0, mm7							;	0	B2	0	A2
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A3	0	C2
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C3	0	B3
	movq	mm5, mm6							; *verts	*verts
	psllq	mm0, 2								;	B2*4	A2*4
	psllq	mm1, 2								;	A3*4	C2*4
	psllq	mm2, 2								;	C3*4	B3*4
	add		vertPtr, 24
	paddd	mm3, mm0							; *4 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	add		esi, 12
	psllq	mm0, 2								;	B2*16	A2*16
	psllq	mm1, 2								;	A3*16	C2*16
	psllq	mm2, 2								;	C3*16	B3*16
	paddd	mm3, mm0							; *20 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	movd	mm0, dword ptr [esi+0]				; 	0	0	B4	A4
	movd	mm1, dword ptr [esi+4]				; 	0	0	A5	C4
	movd	mm2, dword ptr [esi+8]				; 	0	0	C5	B5
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone

	punpcklwd mm0, mm7							;	0	B4	0	A4
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A5	0	C4
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C5	0	B5
	movq	mm5, mm6							; *verts	*verts
	psllq	mm0, 2								;	B4*4	A4*4
	psllq	mm1, 2								;	A5*4	C4*4
	psllq	mm2, 2								;	C5*4	B5*4
	add		vertPtr, 24
	paddd	mm3, mm0							; *4 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	add		esi, 12
	psllq	mm0, 2								;	B4*16	A4*16
	psllq	mm1, 2								;	A5*16	C4*16
	psllq	mm2, 2								;	C5*16	B5*16
	paddd	mm3, mm0							; *20 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	movd	mm0, dword ptr [esi+0]				; 	0	0	B6	A6
	movd	mm1, dword ptr [esi+4]				; 	0	0	A7	C6
	movd	mm2, dword ptr [esi+8]				; 	0	0	C7	B7
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone
	jmp		PrefetchLoop

PrefetchAlmostDone1:
	movd	dword ptr [vertPtr+8], mm4			; write vertex C
	sub		vertPtr, 12
	sub		esi, 6

PrefetchDone:
	mov		eax, dword ptr _triCount[esp]		; number of tri's this loop
	mov		edx, dword ptr [__asm_data + _FIFO_ROOM] ; entries left in the fifo
;	imul	eax, MAX_WRITES_PER_TRI
	shl		eax, 6		;*64
	mov		dword ptr _idx[esp], esi			; set triangle index list pointer to next triangle
	add		eax, MAX_SPEC_SWITCH_WRITES

	; Pad Triangles at end of list
	movd	dword ptr [vertPtr+24+0], mm3
	movd	dword ptr [vertPtr+24+4], mm4
	movd	dword ptr [vertPtr+24+8], mm3
	movd	dword ptr [vertPtr+24+12], mm4
	movd	dword ptr [vertPtr+24+16], mm3
	movd	dword ptr [vertPtr+24+20], mm4

	cmp		edx, eax							; required room - actual room
	jns		SHORT FifoRoomOkay					; room left in the fifo?
	push	eax
	call	_fifo_make_room_asm					; fifo_make_room_asm(FxU32 size)
	add		esp, 4

FifoRoomOkay:
	mov		vertPtr, dword ptr _pVertexList[esp]
	call	dword ptr [DrawTriFunction]
	mov		edx, dword ptr _count[esp]			; # of tri's left
	cmp		edx, 0
	jne		NextChunk

	mov		eax, dword ptr _idx[esp]			; I'm not sure this is the right value to return???
	add		esp,STACK_SIZE
	pop		esi
	pop		edi
	pop		ebx
	pop		ebp
	emms
	ret
_PrefetchTriIdx2_5_Asm_KNI     ENDP
;5555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555

;6666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666
_PrefetchTriIdx2_6T_Asm_KNI     PROC     NEAR
;	FVFOT_XYZWT1 = D3DFVF_XYZRHW | D3DFVF_TEX1
;	size, sx,  sy,  sz, rhw, diff,spec, tu,  tv, tu1, tv1, tu2, tv2
;	  6,   0,   1,   2,   3, 128, 128,   4,   5, 128, 128, 128, 128
ifdef PERF_MONITORING
	call    __penter
endif

MAX_WRITES_PER_TRI = 64	; cmd + 3(X-Y-Z-Wfbi-W-S-T) 

	push	ebp
	push	ebx
	push	edi
	push	esi
	sub		esp, STACK_SIZE

	mov		esi, dword ptr _idx[esp]			; ptr to triangle list index 
	prefetchnta [esi]							; try to get the first cache line prefetched

	LOAD_LOCAL_FLAGS	STACK_OFFSET
	COPY_GLOBAL_DATA	STACK_OFFSET
	BUILD_SPECULAR_COMMAND	STACK_OFFSET

; lookup the special case DrawTriangle function (or use the default)
	mov		eax, dword ptr _localFlags[esp+STACK_OFFSET]
	and		eax, (NOT (FLAG_WRAP_T0_S OR FLAG_WRAP_T0_T OR \	; don't use the individual wrap flags, 
						   FLAG_WRAP_T1_S OR FLAG_WRAP_T1_T OR \	; use FLAG_WRAP_TX instead
						   FLAG_NAPALM))	; we never care if Napalm or not because we always check for it
GATHER_PERFORMANCE_DATA	6
	cmp		eax, dword ptr [LastFlags]
	je		DrawTriangleFunctionKnown
	call	LookupNewKNIDrawTriangleFunction
DrawTriangleFunctionKnown:

	mov		edx, dword ptr _count[esp]			; # of tri's
NextChunk:
	mov		esi, dword ptr _idx[esp]			; ptr to triangle list index 
	mov		eax, edx
	cmp		edx, NUM_PREFETCH_TRIS				; more than max?
	mov		edi, dword ptr _verts[esp]			; address of vertex list
	jl		SHORT @F
	mov		edx, NUM_PREFETCH_TRIS				; set this loop to the max count
@@:	mov		dword ptr _triCount[esp], edx
	sub		eax, edx							; update count to _count - _triCount
	lea		vertPtr, _post_idx[esp+8]			; ptr to destination triangle list (esp+8)
	mov		dword ptr _count[esp], eax			; = total tri's minus tri's done in this loop

	prefetchnta [esi+32]						; prefetch the next cacheline in the index list
	and		vertPtr, 0fffffff8h					; round down to even 8-byte boundary
	pxor	mm7, mm7							; 0			0
	movd	mm6, edi
	punpckldq mm6,mm6							; *verts	*verts

; we want the source index tri list at a 4-byte boundary and the 
; destination vertext list at an 8-byte boundary in the prefetch loop
	test	esi, 2								; check if we start at a word boundary
	je		IndexListAligned

; The index list is unaligned so we set the vertPtr to an odd boundary (4-bytes) 
; and do one vertex, which gets everything aligned again for the prefetch loop
	movzx	eax, word ptr [esi+0]				;   A0
	movd	mm1, dword ptr [esi+2]				; 	0	0	C0	B0
	sub		vertPtr, 4							; force back to an odd boundary
	lea		eax, [eax*2 + eax]					;	A0*3
	shl		eax, 3								;	A0*3*8 = *24
	punpcklwd mm1, mm7							;	0	A1	0	C0
	mov		dword ptr _pVertexList[esp], vertPtr
	movq	mm4, mm1							;	0	A1	0	C0
	paddd	mm4, mm1							;	*2
	paddd	mm4, mm1							;	*3
	psllq	mm4, 3								;	C0*24	B0*24
	movd	mm3, eax							;	0		A0*24
	sub		vertPtr, 12
	paddd	mm3, mm6
	paddd	mm4, mm6
	add		esi, 6
	movd	dword ptr [vertPtr+12+0], mm3
	movq	qword ptr [vertPtr+12+4], mm4
	dec		edx
	jg		PrimePrefetchLoop
	jmp		PrefetchDone						; this was a one-triangle index list

IndexListAligned:
	mov		dword ptr _pVertexList[esp], vertPtr
	sub		vertPtr, 24

; Main prefetch loop, each iteration does 6 tri's = 18 verticies
; 18 verticies means 18 indicies which is 36 bytes, a little over 
; one cache line.  Prefetch two cache lines ahead.
; Also try to prefetch tow of the C verticies
PrimePrefetchLoop:
	movd	mm0, dword ptr [esi+0]				; 	0	0	B0	A0
	movd	mm1, dword ptr [esi+4]				; 	0	0	A1	C0
	movd	mm2, dword ptr [esi+8]				; 	0	0	C1	B1
align 16
PrefetchLoop:
	punpcklwd mm0, mm7							;	0	B0	0	A0
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A1	0	C0
	add		vertPtr, 24
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C1	0	B1
	movq	mm5, mm6							; *verts	*verts
	prefetchnta [esi+64]						; prefetch the next cacheline in the index list
	psllq	mm0, 3								;	B0*8	A0*8
	psllq	mm1, 3								;	A1*8	C0*8
	psllq	mm2, 3								;	C1*8	B1*8
	add		esi, 12
	paddd	mm3, mm0							;	B0*8+v	A0*8+v
	paddd	mm4, mm1							;	A1*8+v	C0*8+v
	paddd	mm5, mm2							;	C1*8+v	B1*8+v
	psllq	mm0, 1								;	B0*16	A0*16
	psllq	mm1, 1								;	A1*16	C0*16
	psllq	mm2, 1								;	C1*16	B1*16
	paddd	mm3, mm0							;	B0*24+v	A0*24+v
	paddd	mm4, mm1							;	A1*24+v	C0*24+v
	paddd	mm5, mm2							;	C1*24+v	B1*24+v
	movd	eax, mm3 							;	*C0
	movd	mm0, dword ptr [esi+0]				; 	0	0	B2	A0
	movd	mm1, dword ptr [esi+4]				; 	0	0	A3	C0
	movd	mm2, dword ptr [esi+8]				; 	0	0	C3	B1
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone

	punpcklwd mm0, mm7							;	0	B2	0	A0
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A3	0	C0
	prefetchnta [eax+32]
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C3	0	B1
	movq	mm5, mm6							; *verts	*verts
	add		vertPtr, 24
	psllq	mm0, 3								;	B2*8	A2*8
	psllq	mm1, 3								;	A3*8	C2*8
	psllq	mm2, 3								;	C3*8	B3*8
	add		esi, 12
	paddd	mm3, mm0							;	B2*8+v	A2*8+v
	paddd	mm4, mm1							;	A3*8+v	C2*8+v
	paddd	mm5, mm2							;	C3*8+v	B3*8+v
	psllq	mm0, 1								;	B2*16	A2*16
	psllq	mm1, 1								;	A3*16	C2*16
	psllq	mm2, 1								;	C3*16	B3*16
	paddd	mm3, mm0							;	B2*24+v	A2*24+v
	paddd	mm4, mm1							;	A3*24+v	C2*24+v
	paddd	mm5, mm2							;	C3*24+v	B3*24+v
	movd 	eax, mm3							;	*C2
	movd	mm0, dword ptr [esi+0]				; 	0	0	B0	A4
	movd	mm1, dword ptr [esi+4]				; 	0	0	A1	C4
	movd	mm2, dword ptr [esi+8]				; 	0	0	C1	B5
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone

	punpcklwd mm0, mm7							;	0	B4	0	A4
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A5	0	C4
	prefetchnta [eax+32]
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C5	0	B5
	movq	mm5, mm6							; *verts	*verts
	add		vertPtr, 24
	psllq	mm0, 3								;	B4*8	A4*8
	psllq	mm1, 3								;	A5*8	C4*8
	psllq	mm2, 3								;	C5*8	B5*8
	add		esi, 12
	paddd	mm3, mm0							;	B4*8+v	A4*8+v
	paddd	mm4, mm1							;	A5*8+v	C4*8+v
	paddd	mm5, mm2							;	C5*8+v	B5*8+v
	psllq	mm0, 1								;	B4*16	A4*16
	psllq	mm1, 1								;	A5*16	C4*16
	psllq	mm2, 1								;	C5*16	B5*16
	paddd	mm3, mm0							;	B4*24+v	A4*24+v
	paddd	mm4, mm1							;	A5*24+v	C4*24+v
	paddd	mm5, mm2							;	C5*24+v	B5*24+v
	movd	mm0, dword ptr [esi+0]				; 	0	0	B6	A6
	movd	mm1, dword ptr [esi+4]				; 	0	0	A7	C6
	movd	mm2, dword ptr [esi+8]				; 	0	0	C7	B7
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone
	jmp		PrefetchLoop

PrefetchAlmostDone1:
	movd	dword ptr [vertPtr+8], mm4			; write vertex C
	sub		vertPtr, 12
	sub		esi, 6

PrefetchDone:
	mov		eax, dword ptr _triCount[esp]		; number of tri's this loop
	mov		edx, dword ptr [__asm_data + _FIFO_ROOM] ; entries left in the fifo
;	imul	eax, MAX_WRITES_PER_TRI
	shl		eax, 6		;*64
	mov		dword ptr _idx[esp], esi			; set triangle index list pointer to next triangle
	add		eax, MAX_SPEC_SWITCH_WRITES

	; Pad Triangles at end of list
	movd	dword ptr [vertPtr+24+0], mm3
	movd	dword ptr [vertPtr+24+4], mm4
	movd	dword ptr [vertPtr+24+8], mm3
	movd	dword ptr [vertPtr+24+12], mm4
	movd	dword ptr [vertPtr+24+16], mm3
	movd	dword ptr [vertPtr+24+20], mm4

	cmp		edx, eax							; required room - actual room
	jns		SHORT FifoRoomOkay					; room left in the fifo?
	push	eax
	call	_fifo_make_room_asm					; fifo_make_room_asm(FxU32 size)
	add		esp, 4

FifoRoomOkay:
	mov		vertPtr, dword ptr _pVertexList[esp]
	call	dword ptr [DrawTriFunction]
	mov		edx, dword ptr _count[esp]			; # of tri's left
	cmp		edx, 0
	jne		NextChunk

	mov		eax, dword ptr _idx[esp]			; I'm not sure this is the right value to return???
	add		esp,STACK_SIZE
	pop		esi
	pop		edi
	pop		ebx
	pop		ebp
	emms
	ret
_PrefetchTriIdx2_6T_Asm_KNI     ENDP
;6666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666

;7777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777
_PrefetchTriIdx2_7_Asm_KNI     PROC     NEAR
; 	FVFOT_XYZWDT1 = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1
;	size, sx,  sy,  sz, rhw, diff,spec, tu,  tv, tu1, tv1, tu2, tv2
;	  7,   0,   1,   2,   3,   4, 128,   5,   6, 128, 128, 128, 128
ifdef PERF_MONITORING
	call    __penter
endif

MAX_WRITES_PER_TRI = 64	; cmd + 3(X-Y-C-Z-Wfbi-W-S-T) 

	push	ebp
	push	ebx
	push	edi
	push	esi
	sub		esp, STACK_SIZE

	mov		esi, dword ptr _idx[esp]			; ptr to triangle list index 
	prefetchnta [esi]							; try to get the first cache line prefetched

	LOAD_LOCAL_FLAGS	STACK_OFFSET
	COPY_GLOBAL_DATA	STACK_OFFSET
	BUILD_SPECULAR_COMMAND	STACK_OFFSET

; lookup the special case DrawTriangle function (or use the default)
	mov		eax, dword ptr _localFlags[esp+STACK_OFFSET]
	and		eax, (NOT (FLAG_WRAP_T0_S OR FLAG_WRAP_T0_T OR \	; don't use the individual wrap flags, 
						   FLAG_WRAP_T1_S OR FLAG_WRAP_T1_T OR \	; use FLAG_WRAP_TX instead
						   FLAG_NAPALM))	; we never care if Napalm or not because we always check for it
GATHER_PERFORMANCE_DATA	7
	cmp		eax, dword ptr [LastFlags]
	je		DrawTriangleFunctionKnown
	call	LookupNewKNIDrawTriangleFunction
DrawTriangleFunctionKnown:

	mov		edx, dword ptr _count[esp]			; # of tri's
NextChunk:
	mov		esi, dword ptr _idx[esp]			; ptr to triangle list index 
	mov		eax, edx
	cmp		edx, NUM_PREFETCH_TRIS				; more than max?
	mov		edi, dword ptr _verts[esp]			; address of vertex list
	jl		SHORT @F
	mov		edx, NUM_PREFETCH_TRIS				; set this loop to the max count
@@:	mov		dword ptr _triCount[esp], edx
	sub		eax, edx							; update count to _count - _triCount
	lea		vertPtr, _post_idx[esp+8]			; ptr to destination triangle list (esp+8)
	mov		dword ptr _count[esp], eax			; = total tri's minus tri's done in this loop

	prefetchnta [esi+32]						; prefetch the next cacheline in the index list
	and		vertPtr, 0fffffff8h					; round down to even 8-byte boundary
	pxor	mm7, mm7							; 0			0
	movd	mm6, edi
	punpckldq mm6,mm6							; *verts	*verts

; we want the source index tri list at a 4-byte boundary and the 
; destination vertext list at an 8-byte boundary in the prefetch loop
	test	esi, 2								; check if we start at a word boundary
	je		IndexListAligned

; The index list is unaligned so we set the vertPtr to an odd boundary (4-bytes) 
; and do one vertex, which gets everything aligned again for the prefetch loop
	movzx	eax, word ptr [esi+0]				;   A0
	movd	mm1, dword ptr [esi+2]				; 	0	0	C0	B0
	sub		vertPtr, 4							; force back to an odd boundary
	shl		eax, 2								;	A0*4
	movq	mm3, mm6
	punpcklwd mm1, mm7							;	0	A1	0	C0
	movq	mm4, mm6
	mov		dword ptr _pVertexList[esp], vertPtr
	movd	mm0, eax							;	0		A0*4
	psllq	mm1, 2								;	C0*4	B0*4
	sub		vertPtr, 12
	psubd	mm3, mm0							; *verts - *4
	psubd	mm4, mm1
	psllq	mm0, 3								;	0		A0*32
	psllq	mm1, 3								;	C0*32	B0*32
	paddd	mm3, mm0							; *verts + *28
	paddd	mm4, mm1
	add		esi, 6
	movd	dword ptr [vertPtr+12+0], mm3
	movq	qword ptr [vertPtr+12+4], mm4
	dec		edx
	jg		PrimePrefetchLoop
	jmp		PrefetchDone						; this was a one-triangle index list

IndexListAligned:
	mov		dword ptr _pVertexList[esp], vertPtr
	sub		vertPtr, 24

; Main prefetch loop, each iteration does 6 tri's = 18 verticies
; 18 verticies means 18 indicies which is 36 bytes, a little over 
; one cache line.  Prefetch two cache lines ahead.
PrimePrefetchLoop:
	movd	mm0, dword ptr [esi+0]				; 	0	0	B0	A0
	movd	mm1, dword ptr [esi+4]				; 	0	0	A1	C0
	movd	mm2, dword ptr [esi+8]				; 	0	0	C1	B1
align 16
PrefetchLoop:
	punpcklwd mm0, mm7							;	0	B0	0	A0
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A1	0	C0
	prefetchnta [esi+64]						; prefetch the next cacheline in the index list
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C1	0	B1
	movq	mm5, mm6							; *verts	*verts
	add		vertPtr, 24
	psllq	mm0, 2								;	B0*4	A0*4
	psllq	mm1, 2								;	A1*4	C0*4
	psllq	mm2, 2								;	C1*4	B1*4
	add		esi, 12
	psubd	mm3, mm0							; *verts - *4
	psubd	mm4, mm1
	psubd	mm5, mm2
	psllq	mm0, 3								;	B0*32	A0*32
	psllq	mm1, 3								;	A1*32	C0*32
	psllq	mm2, 3								;	C1*32	B1*32
	paddd	mm3, mm0							; *verts + *28
	paddd	mm4, mm1
	paddd	mm5, mm2
	movd 	eax, mm3							;	*C0
	movd	mm0, dword ptr [esi+0]				; 	0	0	B2	A2
	movd	mm1, dword ptr [esi+4]				; 	0	0	A3	C2
	movd	mm2, dword ptr [esi+8]				; 	0	0	C3	B3
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone

	punpcklwd mm0, mm7							;	0	B2	0	A2
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A3	0	C2
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C3	0	B3
	movq	mm5, mm6							; *verts	*verts
	prefetchnta [eax+32]
	psllq	mm0, 2								;	B2*4	A2*4
	psllq	mm1, 2								;	A3*4	C2*4
	psllq	mm2, 2								;	C3*4	B3*4
	add		vertPtr, 24
	psubd	mm3, mm0							; *verts - *4
	psubd	mm4, mm1
	psubd	mm5, mm2
	add		esi, 12
	psllq	mm0, 3								;	B2*32	A2*32
	psllq	mm1, 3								;	A3*32	C2*32
	psllq	mm2, 3								;	C3*32	B3*32
	paddd	mm3, mm0							; *verts + *28
	paddd	mm4, mm1
	paddd	mm5, mm2
	movd 	eax, mm3							;	*C2
	movd	mm0, dword ptr [esi+0]				; 	0	0	B4	A4
	movd	mm1, dword ptr [esi+4]				; 	0	0	A5	C4
	movd	mm2, dword ptr [esi+8]				; 	0	0	C5	B5
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone

	punpcklwd mm0, mm7							;	0	B4	0	A4
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A5	0	C4
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C5	0	B5
	movq	mm5, mm6							; *verts	*verts
	prefetchnta [eax+32]
	psllq	mm0, 2								;	B4*4	A4*4
	psllq	mm1, 2								;	A5*4	C4*4
	psllq	mm2, 2								;	C5*4	B5*4
	add		vertPtr, 24
	psubd	mm3, mm0							; *verts - *4
	psubd	mm4, mm1
	psubd	mm5, mm2
	add		esi, 12
	psllq	mm0, 3								;	B4*32	A4*32
	psllq	mm1, 3								;	A5*32	C4*32
	psllq	mm2, 3								;	C5*32	B5*32
	paddd	mm3, mm0							; *verts + *28
	paddd	mm4, mm1
	paddd	mm5, mm2
	movd	mm0, dword ptr [esi+0]				; 	0	0	B6	A6
	movd	mm1, dword ptr [esi+4]				; 	0	0	A7	C6
	movd	mm2, dword ptr [esi+8]				; 	0	0	C7	B7
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone
	jmp		PrefetchLoop

PrefetchAlmostDone1:
	movd	dword ptr [vertPtr+8], mm4			; write vertex C
	sub		vertPtr, 12
	sub		esi, 6

PrefetchDone:
	mov		eax, dword ptr _triCount[esp]		; number of tri's this loop
	mov		edx, dword ptr [__asm_data + _FIFO_ROOM] ; entries left in the fifo
;	imul	eax, MAX_WRITES_PER_TRI
	shl		eax, 6		;*64
	mov		dword ptr _idx[esp], esi			; set triangle index list pointer to next triangle
	add		eax, MAX_SPEC_SWITCH_WRITES

	; Pad Triangles at end of list
	movd	dword ptr [vertPtr+24+0], mm3
	movd	dword ptr [vertPtr+24+4], mm4
	movd	dword ptr [vertPtr+24+8], mm3
	movd	dword ptr [vertPtr+24+12], mm4
	movd	dword ptr [vertPtr+24+16], mm3
	movd	dword ptr [vertPtr+24+20], mm4

	cmp		edx, eax							; required room - actual room
	jns		SHORT FifoRoomOkay					; room left in the fifo?
	push	eax
	call	_fifo_make_room_asm					; fifo_make_room_asm(FxU32 size)
	add		esp, 4

FifoRoomOkay:
	mov		vertPtr, dword ptr _pVertexList[esp]
	call	dword ptr [DrawTriFunction]
	mov		edx, dword ptr _count[esp]			; # of tri's left
	cmp		edx, 0
	jne		NextChunk

	mov		eax, dword ptr _idx[esp]			; I'm not sure this is the right value to return???
	add		esp,STACK_SIZE
	pop		esi
	pop		edi
	pop		ebx
	pop		ebp
	emms
	ret
_PrefetchTriIdx2_7_Asm_KNI     ENDP
;7777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777


;8888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
align   16
PUBLIC   _PrefetchTriIdx2_8_Asm_KNI
_PrefetchTriIdx2_8_Asm_KNI     PROC     NEAR
;	FVFOT_TLVERTEX = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 (D3DFVF_TLVERTEX)
;	size, sx,  sy,  sz, rhw, diff,spec, tu,  tv, tu1, tv1, tu2, tv2
;	  8,   0,   1,   2,   3,   4,   5,   6,   7, 128, 128, 128, 128
;    OR
;	FVFOT_XYZWT2 = D3DFVF_XYZRHW | D3DFVF_TEX2
;	size, sx,  sy,  sz, rhw, diff,spec, tu,  tv, tu1, tv1, tu2, tv2
;	  8,   0,   1,   2,   3, 128, 128,   4,   5,   6,   7, 128, 128
ifdef PERF_MONITORING
	call    __penter
endif

MAX_WRITES_PER_TRI = 64	; 2(cmd + 3(X-Y-C-Z-Wfbi-W-S-T)) 

	push	ebp
	push	ebx
	push	edi
	push	esi
	sub		esp, STACK_SIZE

	mov		esi, dword ptr _idx[esp]			; ptr to triangle list index 
	prefetchnta [esi]							; try to get the first cache line prefetched

	LOAD_LOCAL_FLAGS	STACK_OFFSET
	COPY_GLOBAL_DATA	STACK_OFFSET
	BUILD_SPECULAR_COMMAND	STACK_OFFSET

; lookup the special case DrawTriangle function (or use the default)
	mov		eax, dword ptr _localFlags[esp+STACK_OFFSET]
	and		eax, (NOT (FLAG_WRAP_T0_S OR FLAG_WRAP_T0_T OR \	; don't use the individual wrap flags, 
						   FLAG_WRAP_T1_S OR FLAG_WRAP_T1_T OR \	; use FLAG_WRAP_TX instead
						   FLAG_NAPALM))	; we never care if Napalm or not because we always check for it
GATHER_PERFORMANCE_DATA	8
	cmp		eax, dword ptr [LastFlags]
	je		DrawTriangleFunctionKnown
	call	LookupNewKNIDrawTriangleFunction
DrawTriangleFunctionKnown:

	mov		edx, dword ptr _count[esp]			; # of tri's
NextChunk:
	mov		esi, dword ptr _idx[esp]			; ptr to triangle list index 
	mov		eax, edx
	cmp		edx, NUM_PREFETCH_TRIS				; more than max?
	mov		edi, dword ptr _verts[esp]			; address of vertex list
	jl		SHORT @F
	mov		edx, NUM_PREFETCH_TRIS				; set this loop to the max count
@@:	mov		dword ptr _triCount[esp], edx
	sub		eax, edx							; update count to _count - _triCount
	lea		vertPtr, _post_idx[esp+8]			; ptr to destination triangle list (esp+8)
	mov		dword ptr _count[esp], eax			; = total tri's minus tri's done in this loop

	prefetchnta [esi+32]						; prefetch the next cacheline in the index list
	and		vertPtr, 0fffffff8h					; round down to even 8-byte boundary
	pxor	mm7, mm7							; 0			0
	movd	mm6, edi
	punpckldq mm6,mm6							; *verts	*verts

; we want the source index tri list at a 4-byte boundary and the 
; destination vertext list at an 8-byte boundary in the prefetch loop
	test	esi, 2								; check if we start at a word boundary
	je		IndexListAligned

; The index list is unaligned so we set the vertPtr to an odd boundary (4-bytes) 
; and do one vertex, which gets everything aligned again for the prefetch loop
	movzx	eax, word ptr [esi+0]				;   A0
	movd	mm4, dword ptr [esi+2]				; 	0	0	C0	B0
	sub		vertPtr, 4							; force back to an odd boundary
	shl		eax, 5								;	A0*32
	punpcklwd mm4, mm7							;	0	A1	0	C0
	mov		dword ptr _pVertexList[esp], vertPtr
	movd	mm3, eax							;	0		A0*32
	psllq	mm4, 5								;	C0*32	B0*32
	sub		vertPtr, 12
	paddd	mm3, mm6
	paddd	mm4, mm6
	add		esi, 6
	movd	dword ptr [vertPtr+12+0], mm3
	movq	qword ptr [vertPtr+12+4], mm4
	dec		edx
	jg		PrimePrefetchLoop
	jmp		PrefetchDone						; this was a one-triangle index list

IndexListAligned:
	mov		dword ptr _pVertexList[esp], vertPtr
	sub		vertPtr, 24

; Main prefetch loop, each iteration does 6 tri's = 18 verticies
; 18 verticies means 18 indicies which is 36 bytes, a little over 
; one cache line.  Prefetch two cache lines ahead.
PrimePrefetchLoop:
	movd	mm0, dword ptr [esi+0]				; 	0	0	B0	A0
	movd	mm1, dword ptr [esi+4]				; 	0	0	A1	C0
	movd	mm2, dword ptr [esi+8]				; 	0	0	C1	B1
align 16
PrefetchLoop:
	punpcklwd mm0, mm7							;	0	B0	0	A0
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A1	0	C0
	prefetchnta [esi+64]						; prefetch the next cacheline in the index list
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C1	0	B1
	movq	mm5, mm6							; *verts	*verts
	add		vertPtr, 24
	psllq	mm0, 5								;	B0*32	A0*32
	psllq	mm1, 5								;	A1*32	C0*32
	psllq	mm2, 5								;	C1*32	B1*32
	add		esi, 12
	paddd	mm3, mm0
	paddd	mm4, mm1
	paddd	mm5, mm2
	movd	mm0, dword ptr [esi+0]				; 	0	0	B2	A2
	movd	mm1, dword ptr [esi+4]				; 	0	0	A3	C2
	movd	mm2, dword ptr [esi+8]				; 	0	0	C3	B3
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone

	punpcklwd mm0, mm7							;	0	B2	0	A2
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A3	0	C2
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C3	0	B3
	movq	mm5, mm6							; *verts	*verts
	add		vertPtr, 24
	psllq	mm0, 5								;	B2*32	A2*32
	psllq	mm1, 5								;	A3*32	C2*32
	psllq	mm2, 5								;	C3*32	B3*32
	add		esi, 12
	paddd	mm3, mm0
	paddd	mm4, mm1
	paddd	mm5, mm2
	movd	mm0, dword ptr [esi+0]				; 	0	0	B4	A4
	movd	mm1, dword ptr [esi+4]				; 	0	0	A5	C4
	movd	mm2, dword ptr [esi+8]				; 	0	0	C5	B5
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone

	punpcklwd mm0, mm7							;	0	B4	0	A4
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A5	0	C4
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C5	0	B5
	movq	mm5, mm6							; *verts	*verts
	add		vertPtr, 24
	psllq	mm0, 5								;	B4*32	A4*32
	psllq	mm1, 5								;	A5*32	C4*32
	psllq	mm2, 5								;	C5*32	B5*32
	add		esi, 12
	paddd	mm3, mm0
	paddd	mm4, mm1
	paddd	mm5, mm2
	movd	mm0, dword ptr [esi+0]				; 	0	0	B6	A6
	movd	mm1, dword ptr [esi+4]				; 	0	0	A7	C6
	movd	mm2, dword ptr [esi+8]				; 	0	0	C7	B7
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone
	jmp		PrefetchLoop

PrefetchAlmostDone1:
	movd	dword ptr [vertPtr+8], mm4			; write vertex C
	sub		vertPtr, 12
	sub		esi, 6

PrefetchDone:
	mov		eax, dword ptr _triCount[esp]		; number of tri's this loop
	mov		edx, dword ptr [__asm_data + _FIFO_ROOM] ; entries left in the fifo
;	imul	eax, MAX_WRITES_PER_TRI
	shl		eax, 6		;*64
	mov		dword ptr _idx[esp], esi			; set triangle index list pointer to next triangle
	add		eax, MAX_SPEC_SWITCH_WRITES

	; Pad Triangles at end of list
	movd	dword ptr [vertPtr+24+0], mm3
	movd	dword ptr [vertPtr+24+4], mm4
	movd	dword ptr [vertPtr+24+8], mm3
	movd	dword ptr [vertPtr+24+12], mm4
	movd	dword ptr [vertPtr+24+16], mm3
	movd	dword ptr [vertPtr+24+20], mm4

	cmp		edx, eax							; required room - actual room
	jns		SHORT FifoRoomOkay					; room left in the fifo?
	push	eax
	call	_fifo_make_room_asm					; fifo_make_room_asm(FxU32 size)
	add		esp, 4

FifoRoomOkay:
	mov		vertPtr, dword ptr _pVertexList[esp]
	call	dword ptr [DrawTriFunction]
	mov		edx, dword ptr _count[esp]			; # of tri's left
	cmp		edx, 0
	jne		NextChunk

	mov		eax, dword ptr _idx[esp]			; I'm not sure this is the right value to return???
	add		esp,STACK_SIZE
	pop		esi
	pop		edi
	pop		ebx
	pop		ebp
	emms
	ret
_PrefetchTriIdx2_8_Asm_KNI     ENDP
;8888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888

;9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999
align 16
PUBLIC   _PrefetchTriIdx2_9_Asm_KNI
_PrefetchTriIdx2_9_Asm_KNI     PROC     NEAR
;	FVFOT_XYZWDT2 = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX2
;	size, sx,  sy,  sz, rhw, diff,spec, tu,  tv, tu1, tv1, tu2, tv2
;	  9,   0,   1,   2,   3,   4, 128,   5,   6,   7,   8, 128, 128
ifdef PERF_MONITORING
	call    __penter
endif
MAX_WRITES_PER_TRI = 64	; cmd + 3(X-Y-C-Z-Wfbi-W-S-T-S-T)

	push	ebp
	push	ebx
	push	edi
	push	esi
	sub		esp, STACK_SIZE

	mov		esi, dword ptr _idx[esp]			; ptr to triangle list index 
	prefetchnta [esi]							; try to get the first cache line prefetched

	LOAD_LOCAL_FLAGS	STACK_OFFSET
	COPY_GLOBAL_DATA	STACK_OFFSET
	BUILD_SPECULAR_COMMAND	STACK_OFFSET

; lookup the special case DrawTriangle function (or use the default)
	mov		eax, dword ptr _localFlags[esp+STACK_OFFSET]
	and		eax, (NOT (FLAG_WRAP_T0_S OR FLAG_WRAP_T0_T OR \	; don't use the individual wrap flags, 
						   FLAG_WRAP_T1_S OR FLAG_WRAP_T1_T OR \	; use FLAG_WRAP_TX instead
						   FLAG_NAPALM))	; we never care if Napalm or not because we always check for it
GATHER_PERFORMANCE_DATA	9
	cmp		eax, dword ptr [LastFlags]
	je		DrawTriangleFunctionKnown
	call	LookupNewKNIDrawTriangleFunction
DrawTriangleFunctionKnown:

	mov		edx, dword ptr _count[esp]			; # of tri's
NextChunk:
	mov		esi, dword ptr _idx[esp]			; ptr to triangle list index 
	mov		eax, edx
	cmp		edx, NUM_PREFETCH_TRIS				; more than max?
	mov		edi, dword ptr _verts[esp]			; address of vertex list
	jl		SHORT @F
	mov		edx, NUM_PREFETCH_TRIS				; set this loop to the max count
@@:	mov		dword ptr _triCount[esp], edx
	sub		eax, edx							; update count to _count - _triCount
	lea		vertPtr, _post_idx[esp+8]			; ptr to destination triangle list (esp+8)
	mov		dword ptr _count[esp], eax			; = total tri's minus tri's done in this loop

	prefetchnta [esi+32]						; prefetch the next cacheline in the index list
	and		vertPtr, 0fffffff8h					; round down to even 8-byte boundary
	pxor	mm7, mm7							; 0			0
	movd	mm6, edi
	punpckldq mm6,mm6							; *verts	*verts

; we want the source index tri list at a 4-byte boundary and the 
; destination vertext list at an 8-byte boundary in the prefetch loop
	test	esi, 2								; check if we start at a word boundary
	je		IndexListAligned

; The index list is unaligned so we set the vertPtr to an odd boundary (4-bytes) 
; and do one vertex, which gets everything aligned again for the prefetch loop
	movzx	eax, word ptr [esi+0]				;   A0
	movd	mm1, dword ptr [esi+2]				; 	0	0	C0	B0
	sub		vertPtr, 4							; force back to an odd boundary
	shl		eax, 2								;	A0*4
	movq	mm3, mm6
	punpcklwd mm1, mm7							;	0	A1	0	C0
	movq	mm4, mm6
	mov		dword ptr _pVertexList[esp], vertPtr
	movd	mm0, eax							;	0		A0*4
	psllq	mm1, 2								;	C0*4	B0*4
	sub		vertPtr, 12
	paddd	mm3, mm0							; *4 + verts
	paddd	mm4, mm1
	psllq	mm0, 3								;	0		A0*32
	psllq	mm1, 3								;	C0*32	B0*32
	paddd	mm3, mm0							; *36 + verts
	paddd	mm4, mm1
	add		esi, 6
	movd	dword ptr [vertPtr+12+0], mm3
	movq	qword ptr [vertPtr+12+4], mm4
	dec		edx
	jg		PrimePrefetchLoop
	jmp		PrefetchDone						; this was a one-triangle index list

IndexListAligned:
	mov		dword ptr _pVertexList[esp], vertPtr
	sub		vertPtr, 24

; Main prefetch loop, each iteration does 6 tri's = 18 verticies
; 18 verticies means 18 indicies which is 36 bytes, a little over 
; one cache line.  Prefetch two cache lines ahead.
PrimePrefetchLoop:
	movd	mm0, dword ptr [esi+0]				; 	0	0	B0	A0
	movd	mm1, dword ptr [esi+4]				; 	0	0	A1	C0
	movd	mm2, dword ptr [esi+8]				; 	0	0	C1	B1
align 16
PrefetchLoop:
	punpcklwd mm0, mm7							;	0	B0	0	A0
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A1	0	C0
	prefetchnta [esi+64]						; prefetch the next cacheline in the index list
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C1	0	B1
	movq	mm5, mm6							; *verts	*verts
	add		vertPtr, 24
	psllq	mm0, 2								;	B0*4	A0*4
	psllq	mm1, 2								;	A1*4	C0*4
	psllq	mm2, 2								;	C1*4	B1*4
	add		esi, 12
	paddd	mm3, mm0							; *4 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	psllq	mm0, 3								;	B0*32	A0*32
	psllq	mm1, 3								;	A1*32	C0*32
	psllq	mm2, 3								;	C1*32	B1*32
	paddd	mm3, mm0							; *36 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	movd 	eax, mm3							;	*C0
	movd	mm0, dword ptr [esi+0]				; 	0	0	B2	A2
	movd	mm1, dword ptr [esi+4]				; 	0	0	A3	C2
	movd	mm2, dword ptr [esi+8]				; 	0	0	C3	B3
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone

	punpcklwd mm0, mm7							;	0	B2	0	A2
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A3	0	C2
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C3	0	B3
	movq	mm5, mm6							; *verts	*verts
	prefetchnta [eax+32]
	psllq	mm0, 2								;	B2*4	A2*4
	psllq	mm1, 2								;	A3*4	C2*4
	psllq	mm2, 2								;	C3*4	B3*4
	add		vertPtr, 24
	paddd	mm3, mm0							; *4 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	add		esi, 12
	psllq	mm0, 3								;	B2*32	A2*32
	psllq	mm1, 3								;	A3*32	C2*32
	psllq	mm2, 3								;	C3*32	B3*32
	paddd	mm3, mm0							; *36 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	movd 	eax, mm3							;	*C2
	movd	mm0, dword ptr [esi+0]				; 	0	0	B4	A4
	movd	mm1, dword ptr [esi+4]				; 	0	0	A5	C4
	movd	mm2, dword ptr [esi+8]				; 	0	0	C5	B5
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone

	punpcklwd mm0, mm7							;	0	B4	0	A4
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A5	0	C4
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C5	0	B5
	movq	mm5, mm6							; *verts	*verts
	prefetchnta [eax+32]
	psllq	mm0, 2								;	B4*4	A4*4
	psllq	mm1, 2								;	A5*4	C4*4
	psllq	mm2, 2								;	C5*4	B5*4
	add		vertPtr, 24
	paddd	mm3, mm0							; *4 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	add		esi, 12
	psllq	mm0, 3								;	B4*32	A4*32
	psllq	mm1, 3								;	A5*32	C4*32
	psllq	mm2, 3								;	C5*32	B5*32
	paddd	mm3, mm0							; *36 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	movd	mm0, dword ptr [esi+0]				; 	0	0	B6	A6
	movd	mm1, dword ptr [esi+4]				; 	0	0	A7	C6
	movd	mm2, dword ptr [esi+8]				; 	0	0	C7	B7
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone
	jmp		PrefetchLoop

PrefetchAlmostDone1:
	movd	dword ptr [vertPtr+8], mm4			; write vertex C
	sub		vertPtr, 12
	sub		esi, 6

PrefetchDone:
	mov		eax, dword ptr _triCount[esp]		; number of tri's this loop
	mov		edx, dword ptr [__asm_data + _FIFO_ROOM] ; entries left in the fifo
;	imul	eax, MAX_WRITES_PER_TRI
	shl		eax, 6		;*64
	mov		dword ptr _idx[esp], esi			; set triangle index list pointer to next triangle
	add		eax, MAX_SPEC_SWITCH_WRITES

	; Pad Triangles at end of list
	movd	dword ptr [vertPtr+24+0], mm3
	movd	dword ptr [vertPtr+24+4], mm4
	movd	dword ptr [vertPtr+24+8], mm3
	movd	dword ptr [vertPtr+24+12], mm4
	movd	dword ptr [vertPtr+24+16], mm3
	movd	dword ptr [vertPtr+24+20], mm4

	cmp		edx, eax							; required room - actual room
	jns		SHORT FifoRoomOkay					; room left in the fifo?
	push	eax
	call	_fifo_make_room_asm					; fifo_make_room_asm(FxU32 size)
	add		esp, 4

FifoRoomOkay:
	mov		vertPtr, dword ptr _pVertexList[esp]
	call	dword ptr [DrawTriFunction]
	mov		edx, dword ptr _count[esp]			; # of tri's left
	cmp		edx, 0
	jne		NextChunk

	mov		eax, dword ptr _idx[esp]			; I'm not sure this is the right value to return???
	add		esp,STACK_SIZE
	pop		esi
	pop		edi
	pop		ebx
	pop		ebp
	emms
	ret
_PrefetchTriIdx2_9_Asm_KNI     ENDP
;9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999

;10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10
align   16
PUBLIC   _PrefetchTriIdx2_10_Asm_KNI
_PrefetchTriIdx2_10_Asm_KNI     PROC     NEAR
;	FVFOT_XYZWDST2 = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2
;	size, sx,  sy,  sz, rhw, diff,spec, tu,  tv, tu1, tv1, tu2, tv2
;	 10,  0,   1,   2,   3,   4,   5,   6,   7,   8,   9, 128, 128
ifdef PERF_MONITORING
	call    __penter
endif

MAX_WRITES_PER_TRI = 64	; 2(cmd + 3(X-Y-C-Z-Wfbi-W-S-T-S-T)) 

	push	ebp
	push	ebx
	push	edi
	push	esi
	sub		esp, STACK_SIZE

	mov		esi, dword ptr _idx[esp]			; ptr to triangle list index 
	prefetchnta [esi]							; try to get the first cache line prefetched

	LOAD_LOCAL_FLAGS	STACK_OFFSET
	COPY_GLOBAL_DATA	STACK_OFFSET
	BUILD_SPECULAR_COMMAND	STACK_OFFSET

; lookup the special case DrawTriangle function (or use the default)
	mov		eax, dword ptr _localFlags[esp+STACK_OFFSET]
	and		eax, (NOT (FLAG_WRAP_T0_S OR FLAG_WRAP_T0_T OR \	; don't use the individual wrap flags, 
						   FLAG_WRAP_T1_S OR FLAG_WRAP_T1_T OR \	; use FLAG_WRAP_TX instead
						   FLAG_NAPALM))	; we never care if Napalm or not because we always check for it
GATHER_PERFORMANCE_DATA	10
	cmp		eax, dword ptr [LastFlags]
	je		DrawTriangleFunctionKnown
	call	LookupNewKNIDrawTriangleFunction
DrawTriangleFunctionKnown:

	mov		edx, dword ptr _count[esp]			; # of tri's
NextChunk:
	mov		esi, dword ptr _idx[esp]			; ptr to triangle list index 
	mov		eax, edx
	cmp		edx, NUM_PREFETCH_TRIS				; more than max?
	mov		edi, dword ptr _verts[esp]			; address of vertex list
	jl		SHORT @F
	mov		edx, NUM_PREFETCH_TRIS				; set this loop to the max count
@@:	mov		dword ptr _triCount[esp], edx
	sub		eax, edx							; update count to _count - _triCount
	lea		vertPtr, _post_idx[esp+8]			; ptr to destination triangle list (esp+8)
	mov		dword ptr _count[esp], eax			; = total tri's minus tri's done in this loop

	prefetchnta [esi+32]						; prefetch the next cacheline in the index list
	and		vertPtr, 0fffffff8h					; round down to even 8-byte boundary
	pxor	mm7, mm7							; 0			0
	movd	mm6, edi
	punpckldq mm6,mm6							; *verts	*verts

; we want the source index tri list at a 4-byte boundary and the 
; destination vertext list at an 8-byte boundary in the prefetch loop
	test	esi, 2								; check if we start at a word boundary
	je		IndexListAligned

; The index list is unaligned so we set the vertPtr to an odd boundary (4-bytes) 
; and do one vertex, which gets everything aligned again for the prefetch loop
	movzx	eax, word ptr [esi+0]				;   A0
	movd	mm1, dword ptr [esi+2]				; 	0	0	C0	B0
	sub		vertPtr, 4							; force back to an odd boundary
	shl		eax, 3								;	A0*8
	movq	mm3, mm6
	punpcklwd mm1, mm7							;	0	A1	0	C0
	movq	mm4, mm6
	mov		dword ptr _pVertexList[esp], vertPtr
	movd	mm0, eax							;	0		A0*8
	psllq	mm1, 3								;	C0*8	B0*8
	sub		vertPtr, 12
	paddd	mm3, mm0							; *4 + verts
	paddd	mm4, mm1
	psllq	mm0, 2								;	0		A0*32
	psllq	mm1, 2								;	C0*32	B0*32
	paddd	mm3, mm0							; *40 + verts
	paddd	mm4, mm1
	add		esi, 6
	movd	dword ptr [vertPtr+12+0], mm3
	movq	qword ptr [vertPtr+12+4], mm4
	dec		edx
	jg		PrimePrefetchLoop
	jmp		PrefetchDone						; this was a one-triangle index list

IndexListAligned:
	mov		dword ptr _pVertexList[esp], vertPtr
	sub		vertPtr, 24

; Main prefetch loop, each iteration does 6 tri's = 18 verticies
; 18 verticies means 18 indicies which is 36 bytes, a little over 
; one cache line.  Prefetch two cache lines ahead.
PrimePrefetchLoop:
	movd	mm0, dword ptr [esi+0]				; 	0	0	B0	A0
	movd	mm1, dword ptr [esi+4]				; 	0	0	A1	C0
	movd	mm2, dword ptr [esi+8]				; 	0	0	C1	B1
;align 16
PrefetchLoop:
	punpcklwd mm0, mm7							;	0	B0	0	A0
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A1	0	C0
	prefetchnta [esi+64]						; prefetch the next cacheline in the index list
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C1	0	B1
	movq	mm5, mm6							; *verts	*verts
	add		vertPtr, 24
	psllq	mm0, 3								;	B0*8	A0*8
	psllq	mm1, 3								;	A1*8	C0*8
	psllq	mm2, 3								;	C1*8	B1*8
	add		esi, 12
	paddd	mm3, mm0							; *8 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	psllq	mm0, 2								;	B0*32	A0*32
	psllq	mm1, 2								;	A1*32	C0*32
	psllq	mm2, 2								;	C1*32	B1*32
	paddd	mm3, mm0							; *40 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	movd 	eax, mm3							;	*C0
	movd	mm0, dword ptr [esi+0]				; 	0	0	B2	A2
	movd	mm1, dword ptr [esi+4]				; 	0	0	A3	C2
	movd	mm2, dword ptr [esi+8]				; 	0	0	C3	B3
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone

	punpcklwd mm0, mm7							;	0	B2	0	A2
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A3	0	C2
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C3	0	B3
	movq	mm5, mm6							; *verts	*verts
	prefetchnta [eax+32]
	psllq	mm0, 3								;	B2*8	A2*8
	psllq	mm1, 3								;	A3*8	C2*8
	psllq	mm2, 3								;	C3*8	B3*8
	add		vertPtr, 24
	paddd	mm3, mm0							; *8 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	add		esi, 12
	psllq	mm0, 2								;	B2*32	A2*32
	psllq	mm1, 2								;	A3*32	C2*32
	psllq	mm2, 2								;	C3*32	B3*32
	paddd	mm3, mm0							; *40 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	movd 	eax, mm3							;	*C2
	movd	mm0, dword ptr [esi+0]				; 	0	0	B4	A4
	movd	mm1, dword ptr [esi+4]				; 	0	0	A5	C4
	movd	mm2, dword ptr [esi+8]				; 	0	0	C5	B5
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone

	punpcklwd mm0, mm7							;	0	B4	0	A4
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A5	0	C4
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C5	0	B5
	movq	mm5, mm6							; *verts	*verts
	prefetchnta [eax+32]
	psllq	mm0, 3								;	B4*8	A4*8
	psllq	mm1, 3								;	A5*8	C4*8
	psllq	mm2, 3								;	C5*8	B5*8
	add		vertPtr, 24
	paddd	mm3, mm0							; *8 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	add		esi, 12
	psllq	mm0, 2								;	B4*32	A4*32
	psllq	mm1, 2								;	A5*32	C4*32
	psllq	mm2, 2								;	C5*32	B5*32
	paddd	mm3, mm0							; *40 + verts
	paddd	mm4, mm1
	paddd	mm5, mm2
	movd	mm0, dword ptr [esi+0]				; 	0	0	B6	A6
	movd	mm1, dword ptr [esi+4]				; 	0	0	A7	C6
	movd	mm2, dword ptr [esi+8]				; 	0	0	C7	B7
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone
	jmp		PrefetchLoop

PrefetchAlmostDone1:
	movd	dword ptr [vertPtr+8], mm4			; write vertex C
	sub		vertPtr, 12
	sub		esi, 6

PrefetchDone:
	mov		eax, dword ptr _triCount[esp]		; number of tri's this loop
	mov		edx, dword ptr [__asm_data + _FIFO_ROOM] ; entries left in the fifo
;	imul	eax, MAX_WRITES_PER_TRI
	shl		eax, 6		;*64
	mov		dword ptr _idx[esp], esi			; set triangle index list pointer to next triangle
	add		eax, MAX_SPEC_SWITCH_WRITES

	; Pad Triangles at end of list
	movd	dword ptr [vertPtr+24+0], mm3
	movd	dword ptr [vertPtr+24+4], mm4
	movd	dword ptr [vertPtr+24+8], mm3
	movd	dword ptr [vertPtr+24+12], mm4
	movd	dword ptr [vertPtr+24+16], mm3
	movd	dword ptr [vertPtr+24+20], mm4

	cmp		edx, eax							; required room - actual room
	jns		SHORT FifoRoomOkay					; room left in the fifo?
	push	eax
	call	_fifo_make_room_asm					; fifo_make_room_asm(FxU32 size)
	add		esp, 4

FifoRoomOkay:
	mov		vertPtr, dword ptr _pVertexList[esp]
	call	dword ptr [DrawTriFunction]
	mov		edx, dword ptr _count[esp]			; # of tri's left
	cmp		edx, 0
	jne		NextChunk

	mov		eax, dword ptr _idx[esp]			; I'm not sure this is the right value to return???
	add		esp,STACK_SIZE
	pop		esi
	pop		edi
	pop		ebx
	pop		ebp
	emms
	ret
_PrefetchTriIdx2_10_Asm_KNI     ENDP

if 0
_PrefetchTriIdx2_11_Asm_KNI    PROC     NEAR
;	FVFOT_XYZWDT3 = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEX2
;	size, sx,  sy,  sz, rhw, diff,spec, tu,  tv, tu1, tv1, tu2, tv2
;	  11,  0,   1,   2,   3,   4, 128,   5,   6,   7,   8,   9,  10
	ret
_PrefetchTriIdx2_11_Asm_KNI     ENDP
endif ;0
;10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10

;12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12
align   16
PUBLIC   _PrefetchTriIdx2_12_Asm_KNI
_PrefetchTriIdx2_12_Asm_KNI     PROC     NEAR
;	FVFOT_XYZWDST3 = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 | D3DFVF_TEX2
;	size, sx,  sy,  sz, rhw, diff,spec, tu,  tv, tu1, tv1, tu2, tv2
;	  12,  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11
ifdef PERF_MONITORING
	call    __penter
endif

MAX_WRITES_PER_TRI = 64	; 2(cmd + 3(X-Y-C-Z-Wfbi-W-S-T-S-T)) (never write 3 textures)

	push	ebp
	push	ebx
	push	edi
	push	esi
	sub		esp, STACK_SIZE

	mov		esi, dword ptr _idx[esp]			; ptr to triangle list index 
	prefetchnta [esi]							; try to get the first cache line prefetched

	LOAD_LOCAL_FLAGS	STACK_OFFSET
	COPY_GLOBAL_DATA	STACK_OFFSET
	BUILD_SPECULAR_COMMAND	STACK_OFFSET

; lookup the special case DrawTriangle function (or use the default)
	mov		eax, dword ptr _localFlags[esp+STACK_OFFSET]
	and		eax, (NOT (FLAG_WRAP_T0_S OR FLAG_WRAP_T0_T OR \	; don't use the individual wrap flags, 
						   FLAG_WRAP_T1_S OR FLAG_WRAP_T1_T OR \	; use FLAG_WRAP_TX instead
						   FLAG_NAPALM))	; we never care if Napalm or not because we always check for it
GATHER_PERFORMANCE_DATA	12
	cmp		eax, dword ptr [LastFlags]
	je		DrawTriangleFunctionKnown
	call	LookupNewKNIDrawTriangleFunction
DrawTriangleFunctionKnown:

	mov		edx, dword ptr _count[esp]			; # of tri's
NextChunk:
	mov		esi, dword ptr _idx[esp]			; ptr to triangle list index 
	mov		eax, edx
	cmp		edx, NUM_PREFETCH_TRIS				; more than max?
	mov		edi, dword ptr _verts[esp]			; address of vertex list
	jl		SHORT @F
	mov		edx, NUM_PREFETCH_TRIS				; set this loop to the max count
@@:	mov		dword ptr _triCount[esp], edx
	sub		eax, edx							; update count to _count - _triCount
	lea		vertPtr, _post_idx[esp+8]			; ptr to destination triangle list (esp+8)
	mov		dword ptr _count[esp], eax			; = total tri's minus tri's done in this loop

	prefetchnta [esi+32]						; prefetch the next cacheline in the index list
	and		vertPtr, 0fffffff8h					; round down to even 8-byte boundary
	pxor	mm7, mm7							; 0			0
	movd	mm6, edi
	punpckldq mm6,mm6							; *verts	*verts

; we want the source index tri list at a 4-byte boundary and the 
; destination vertext list at an 8-byte boundary in the prefetch loop
	test	esi, 2								; check if we start at a word boundary
	je		IndexListAligned

; The index list is unaligned so we set the vertPtr to an odd boundary (4-bytes) 
; and do one vertex, which gets everything aligned again for the prefetch loop
	movzx	eax, word ptr [esi+0]				;   A0
	movd	mm1, dword ptr [esi+2]				; 	0	0	C0	B0
	sub		vertPtr, 4							; force back to an odd boundary
	lea		eax, [eax*2 + eax]					;	A0*3
	shl		eax, 4								;	A0*3*16 = *48
	punpcklwd mm1, mm7							;	0	A1	0	C0
	mov		dword ptr _pVertexList[esp], vertPtr
	movq	mm4, mm1							;	0	A1	0	C0
	paddd	mm4, mm1							;	*2
	paddd	mm4, mm1							;	*3
	psllq	mm4, 4								;	C0*48	B0*48
	movd	mm3, eax							;	0		A0*48
	sub		vertPtr, 12
	paddd	mm3, mm6
	paddd	mm4, mm6
	add		esi, 6
	movd	dword ptr [vertPtr+12+0], mm3
	movq	qword ptr [vertPtr+12+4], mm4
	dec		edx
	jg		PrimePrefetchLoop
	jmp		PrefetchDone						; this was a one-triangle index list

IndexListAligned:
	mov		dword ptr _pVertexList[esp], vertPtr
	sub		vertPtr, 24

; Main prefetch loop, each iteration does 6 tri's = 18 verticies
; 18 verticies means 18 indicies which is 36 bytes, a little over 
; one cache line.  Prefetch two cache lines ahead.
; Also try to prefetch tow of the C verticies
PrimePrefetchLoop:
	movd	mm0, dword ptr [esi+0]				; 	0	0	B0	A0
	movd	mm1, dword ptr [esi+4]				; 	0	0	A1	C0
	movd	mm2, dword ptr [esi+8]				; 	0	0	C1	B1
align 16
PrefetchLoop:
	punpcklwd mm0, mm7							;	0	B0	0	A0
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A1	0	C0
	add		vertPtr, 24
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C1	0	B1
	movq	mm5, mm6							; *verts	*verts
	prefetchnta [esi+64]						; prefetch the next cacheline in the index list
	psllq	mm0, 4								;	B0*16	A0*16
	psllq	mm1, 4								;	A1*16	C0*16
	psllq	mm2, 4								;	C1*16	B1*16
	add		esi, 12
	paddd	mm3, mm0							;	B0*16+v	A0*16+v
	paddd	mm4, mm1							;	A1*16+v	C0*16+v
	paddd	mm5, mm2							;	C1*16+v	B1*16+v
	psllq	mm0, 1								;	B0*32	A0*32
	psllq	mm1, 1								;	A1*32	C0*32
	psllq	mm2, 1								;	C1*32	B1*32
	paddd	mm3, mm0							;	B0*48+v	A0*48+v
	paddd	mm4, mm1							;	A1*48+v	C0*48+v
	paddd	mm5, mm2							;	C1*48+v	B1*48+v
	movd	eax, mm3 							;	*C0
	movd	mm0, dword ptr [esi+0]				; 	0	0	B2	A0
	movd	mm1, dword ptr [esi+4]				; 	0	0	A3	C0
	movd	mm2, dword ptr [esi+8]				; 	0	0	C3	B1
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone

	punpcklwd mm0, mm7							;	0	B2	0	A0
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A3	0	C0
	prefetchnta [eax+32]
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C3	0	B1
	movq	mm5, mm6							; *verts	*verts
	add		vertPtr, 24
	psllq	mm0, 4								;	B2*16	A2*16
	psllq	mm1, 4								;	A3*16	C2*16
	psllq	mm2, 4								;	C3*16	B3*16
	add		esi, 12
	paddd	mm3, mm0							;	B2*16+v	A2*16+v
	paddd	mm4, mm1							;	A3*16+v	C2*16+v
	paddd	mm5, mm2							;	C3*16+v	B3*16+v
	psllq	mm0, 1								;	B2*32	A2*32
	psllq	mm1, 1								;	A3*32	C2*32
	psllq	mm2, 1								;	C3*32	B3*32
	paddd	mm3, mm0							;	B2*48+v	A2*48+v
	paddd	mm4, mm1							;	A3*48+v	C2*48+v
	paddd	mm5, mm2							;	C3*48+v	B3*48+v
	movd 	eax, mm3							;	*C2
	movd	mm0, dword ptr [esi+0]				; 	0	0	B0	A4
	movd	mm1, dword ptr [esi+4]				; 	0	0	A1	C4
	movd	mm2, dword ptr [esi+8]				; 	0	0	C1	B5
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone

	punpcklwd mm0, mm7							;	0	B4	0	A4
	movq	mm3, mm6							; *verts	*verts
	punpcklwd mm1, mm7							;	0	A5	0	C4
	prefetchnta [eax+32]
	movq	mm4, mm6							; *verts	*verts
	punpcklwd mm2, mm7							;	0	C5	0	B5
	movq	mm5, mm6							; *verts	*verts
	add		vertPtr, 24
	psllq	mm0, 4								;	B4*16	A4*16
	psllq	mm1, 4								;	A5*16	C4*16
	psllq	mm2, 4								;	C5*16	B5*16
	add		esi, 12
	paddd	mm3, mm0							;	B4*16+v	A4*16+v
	paddd	mm4, mm1							;	A5*16+v	C4*16+v
	paddd	mm5, mm2							;	C5*16+v	B5*16+v
	psllq	mm0, 1								;	B4*32	A4*32
	psllq	mm1, 1								;	A5*32	C4*32
	psllq	mm2, 1								;	C5*32	B5*32
	paddd	mm3, mm0							;	B4*48+v	A4*48+v
	paddd	mm4, mm1							;	A5*48+v	C4*48+v
	paddd	mm5, mm2							;	C5*48+v	B5*48+v
	movd	mm0, dword ptr [esi+0]				; 	0	0	B6	A6
	movd	mm1, dword ptr [esi+4]				; 	0	0	A7	C6
	movd	mm2, dword ptr [esi+8]				; 	0	0	C7	B7
	movq	qword ptr [vertPtr+0], mm3
	dec		edx
	je		PrefetchAlmostDone1
	movq	qword ptr [vertPtr+8], mm4
	movq	qword ptr [vertPtr+16], mm5
	dec		edx
	je		PrefetchDone
	jmp		PrefetchLoop

PrefetchAlmostDone1:
	movd	dword ptr [vertPtr+8], mm4			; write vertex C
	sub		vertPtr, 12
	sub		esi, 6

PrefetchDone:
	mov		eax, dword ptr _triCount[esp]		; number of tri's this loop
	mov		edx, dword ptr [__asm_data + _FIFO_ROOM] ; entries left in the fifo
;	imul	eax, MAX_WRITES_PER_TRI
	shl		eax, 6
	mov		dword ptr _idx[esp], esi			; set triangle index list pointer to next triangle
	add		eax, MAX_SPEC_SWITCH_WRITES

	; Pad Triangles at end of list
	movd	dword ptr [vertPtr+24+0], mm3
	movd	dword ptr [vertPtr+24+4], mm4
	movd	dword ptr [vertPtr+24+8], mm3
	movd	dword ptr [vertPtr+24+12], mm4
	movd	dword ptr [vertPtr+24+16], mm3
	movd	dword ptr [vertPtr+24+20], mm4

	cmp		edx, eax							; required room - actual room
	jns		SHORT FifoRoomOkay					; room left in the fifo?
	push	eax
	call	_fifo_make_room_asm					; fifo_make_room_asm(FxU32 size)
	add		esp, 4

FifoRoomOkay:
	mov		vertPtr, dword ptr _pVertexList[esp]
	call	dword ptr [DrawTriFunction]
	mov		edx, dword ptr _count[esp]			; # of tri's left
	cmp		edx, 0
	jne		NextChunk

	mov		eax, dword ptr _idx[esp]			; I'm not sure this is the right value to return???
	add		esp,STACK_SIZE
	pop		esi
	pop		edi
	pop		ebx
	pop		ebp
	emms
	ret
_PrefetchTriIdx2_12_Asm_KNI     ENDP
;12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12

if 0
_PrefetchTriIdx2_6G_Asm_KNI     PROC     NEAR
;	FVFOT_XYZWDS = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR
;	size, sx,  sy,  sz, rhw, diff,spec, tu,  tv, tu1, tv1, tu2, tv2
;	  6,   0,   1,   2,   3,   4,   5,   0,   0, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128
	ret
;   jmp      _DrawTriIdx2GouradAsm_KNI
_PrefetchTriIdx2_6G_Asm_KNI     ENDP
endif ;0



;***********************************************************************************************************************
;***********************************************************************************************************************
;
;	A whole bunch of macro's
;
;***********************************************************************************************************************
;***********************************************************************************************************************

; Where to write data in the "SETxxx" macro's
CMD_FIFO	EQU		1
SPEC_BUFFER	EQU		2
BOTH_DESTS	EQU		3

; Special case flags ("CHECK" is not special case)
FALSE		EQU		0
TRUE 		EQU		1
CHECK		EQU		2

; For aligning qword writes to the cmd fifo
DONT_CARE	EQU		0
START_ODD 	EQU		1
START_EVEN	EQU		2


;;;;;;;; Macros to store to the cmd fifo and/or the specular buffer
; general purpose register store
SET4GPR		MACRO	GP_REG32:REQ, DEST:REQ
	SETALL	<mov dword ptr>, DEST, 0, GP_REG32, 4
ENDM

; immediate value store
SET4IMM		MACRO	VALUE:REQ, DEST:REQ
	SETALL	<mov dword ptr>, DEST, 0, VALUE, 4
ENDM

; mmx register dword store   
SET4MMX		MACRO	MMX_REG32:REQ, DEST:REQ
	SETALL	<movd dword ptr>, DEST, 0, MMX_REG32, 4
ENDM

; mmx register qword store
SET8MMX		MACRO	MMX_REG64:REQ, DEST:REQ
	SETALL	<movq qword ptr>, DEST, 0, MMX_REG64, 8
ENDM

; xmm register dword store
SET4XMM		MACRO	XMM_REG32:REQ, DEST:REQ
	SETALL	<movss dword ptr>, DEST, 0, XMM_REG32, 4
ENDM

; xmm register qword store - lower half
SET8XMM_L	MACRO	XMM_REG64:REQ, DEST:REQ

; The problem is that misaligned writes really suck on the P3, so it's addressed here.
; This one's pretty nasty ... what we're doing is if the destination address is 
; known we check if it's 8-byte aligned.  If not we write the lower dword, 
; shuffle, and write the upper dword.  (Don't fix the source register after we're 
; finished.)  There's a bunch of cases that must be checked here.  
tmpHWPTR_DWORD_OFFSET textequ %(HWPTR_DWORD_OFFSET)
tmpSBUFF_DWORD_OFFSET textequ %(SBUFF_DWORD_OFFSET)
tmpFORCE_SPLIT textequ %(FORCE_SPLIT_LOWER_XMM_QWORD_WRITE)
tmpCHECK_SPLIT textequ %(CHECK_SPLIT_LOWER_XMM_QWORD_WRITE)
tmpALIGN_HW_WR textequ %(ALIGN_QWORD_HWPTR_WRITES)
tmpALIGN_SP_WR textequ %(ALIGN_QWORD_SBUFF_WRITES)
tmpALIGN_MOD   textequ %((HWPTR_DWORD_OFFSET MOD 8) SHR 2)

tmpSPLIT_HW_WRITE = (FORCE_SPLIT_LOWER_XMM_QWORD_WRITE OR	\
					(ALIGN_QWORD_HWPTR_WRITES AND ((HWPTR_DWORD_OFFSET MOD 8) SHR 2)))
tmpSPLIT_SP_WRITE = (FORCE_SPLIT_LOWER_XMM_QWORD_WRITE OR	\
					(ALIGN_QWORD_SBUFF_WRITES AND ((SBUFF_DWORD_OFFSET MOD 8) SHR 2)))

IF	((FORCE_SPLIT_LOWER_XMM_QWORD_WRITE) OR \
	(tmpSPLIT_HW_WRITE AND tmpSPLIT_SP_WRITE))			; split both writes
	SETALL	<movss dword ptr>, DEST, 0, XMM_REG64, 0		; write lower dword
	shufps	XMM_REG64, XMM_REG64, 0e1h						; swap lower two dwords
	SETALL	<movss dword ptr>, DEST, 4, XMM_REG64, 8		; write the upper dword

ELSEIF ((NOT tmpSPLIT_HW_WRITE) AND tmpSPLIT_SP_WRITE)	; split the sp write only
  IF ((DEST EQ CMD_FIFO) OR (DEST EQ BOTH_DESTS))
	SETALL	<movlps qword ptr>, CMD_FIFO, 0, XMM_REG64, 8	; get the cmd fifo write done first
  ENDIF
  IF ((DEST EQ SPEC_BUFFER) OR (DEST EQ BOTH_DESTS))
	SETALL	<movss dword ptr>, SPEC_BUFFER, 0, XMM_REG64, 0	; write the lower dword
	shufps	XMM_REG64, XMM_REG64, 0e1h						; swap lower two dwords
	SETALL	<movss dword ptr>, SPEC_BUFFER, 4, XMM_REG64, 8	; write the upper dword
  ENDIF

ELSEIF (tmpSPLIT_HW_WRITE AND (NOT tmpSPLIT_SP_WRITE))	; split the hw write only
  IF ((DEST EQ SPEC_BUFFER) OR (DEST EQ BOTH_DESTS))
	SETALL	<movlps qword ptr>, SPEC_BUFFER, 0, XMM_REG64, 8 ; get the spec buffer write done first
  ENDIF
  IF ((DEST EQ CMD_FIFO) OR (DEST EQ BOTH_DESTS))
	SETALL	<movss dword ptr>, CMD_FIFO, 0, XMM_REG64, 0	; write the lower dword
	shufps	XMM_REG64, XMM_REG64, 0e1h						; swap lower two dwords
	SETALL	<movss dword ptr>, CMD_FIFO, 4, XMM_REG64, 8	; write the upper dword
  ENDIF

ELSE	; default case (both are aligned or we're not checking alignment)
	SETALL	<movlps qword ptr>, DEST, 0, XMM_REG64, 8	; write the full qword normally
ENDIF
ENDM

; xmm register qword store - upper half (isn't worth shuffling twice is misaligned)
SET8XMM_H	MACRO	XMM_REG64:REQ, DEST:REQ
	SETALL	<movhps qword ptr>, DEST, 0, XMM_REG64, 8
ENDM

; xmm register oword store
SET16XMM	MACRO	XMM_REG128:REQ, DEST:REQ
	SETALL	<movups qword ptr>, DEST, 0, XMM_REG128, 16
ENDM



SETALL	MACRO		\
	INSTR:REQ,		\ ; the instruction
	DEST:REQ,		\ ; the destination (CMD_FIFO, SPEC_BUFFER, or BOTH_DESTS)
	DEST_OFFSET:REQ,\ ; offset to add to the destination address
	SRC:REQ,		\ ; source register
	INCREMENT:REQ	  ; value to add to the destination register afterwards


IF ((DEST EQ CMD_FIFO) OR (DEST EQ BOTH_DESTS))
	INSTR [hwPtr + DEST_OFFSET], SRC							;; send to hw
ENDIF
IF (((DEST EQ SPEC_BUFFER) OR (DEST EQ BOTH_DESTS)) AND SEND_2ND_SPEC_PASS)
	INSTR  [SpecularStream + spPtr - 32 + DEST_OFFSET], SRC	;; send to spec buffer
ENDIF

IF INCREMENT
  IF ((DEST EQ CMD_FIFO) OR (DEST EQ BOTH_DESTS))
	add		hwPtr, INCREMENT
	HWPTR_DWORD_OFFSET = (HWPTR_DWORD_OFFSET + INCREMENT)
  ENDIF
  IF (((DEST EQ SPEC_BUFFER) OR (DEST EQ BOTH_DESTS)) AND SEND_2ND_SPEC_PASS)
	add		spPtr, INCREMENT
	SBUFF_DWORD_OFFSET = (SBUFF_DWORD_OFFSET + INCREMENT)
  ENDIF
ENDIF

ENDM ; SETALL


;******************************************************************************************************
; Wrap macro
;
;	if (wrap) {
;		if      ((dAC > 0.5) && (dBC > 0.5))
;			if (C < B)	C += 1.0
;			else A += 1.0, B += 1.0
;		else if ((dBC > 0.5) && (dAB > 0.5))
;			if (B < A)	B += 1.0
;			else A += 1.0, C += 1.0
;		else if ((dAB > 0.5) && (dAC > 0.5))
;			if (A < C)	A += 1.0
;			else B += 1.0, C += 1.0
;	}
;
; The macro handles one or two textures simutaiously.  It stores the vertex coordinates because it 
; needs the extra registers.  If no wrap is required it early-outs about halfway through.  The C code
; with x86 FP code and many branches is actually faster for one texture but this is faster for two
; textures.  The other reason for not using x86 code is that the mmx (fp) registers are being used 
; and the overhead of switching back and forth isn't worth it.
;******************************************************************************************************
WRAP	MACRO	NUM_TEXTURES:REQ
LOCAL WrapDone

; registers barrowed by the wrap function
xmmWrap1	TEXTEQU	<xmmT1>		; Temp Reg #1
xmmWrap2	TEXTEQU	<xmmT2>		; Temp Reg #2
xmmWrap3	TEXTEQU	<xmmVa>		; Temp Reg #3
xmmWrap4	TEXTEQU	<xmmVb>		; Temp Reg #4
xmmWrap5	TEXTEQU	<xmmVc>		; Temp Reg #5

IF		(NUM_TEXTURES EQ 1)		; if one texture only load the lower halves of the xmm regs
	MvWrap	textequ	<movlps>
	MvSize	textequ <qword>
ELSEIF	(NUM_TEXTURES EQ 2)		; for two textures we load the entire register
	MvWrap	textequ	<movaps>
	MvSize	textequ <oword>
ENDIF

	; store vertex coordinates
	movlps	qword ptr [TempVa], xmmVa
	movaps	oword ptr [TempVbc], xmmVbc

	movaps	xmmWrap1, xmmTxA
	movaps	xmmWrap2, xmmTxC
	movaps	xmmWrap3, xmmTxB
	MvWrap	xmmWrap4, MvSize ptr [flt_4_mask_sign]
	subps	xmmWrap1, xmmTxB
	subps	xmmWrap2, xmmTxA
	subps	xmmWrap3, xmmTxC
	MvWrap	xmmWrap5, MvSize ptr [flt_4_halves]
	andps	xmmWrap1, xmmWrap4					; dAB
	andps	xmmWrap2, xmmWrap4					; dAC (dCA)
	andps	xmmWrap3, xmmWrap4					; dBC
	cmpps	xmmWrap1, xmmWrap5, XMM_GT			; (dAB > 0.5) ? -1 : 0
	cmpps	xmmWrap2, xmmWrap5, XMM_GT			; (dAC > 0.5) ? -1 : 0
	cmpps	xmmWrap3, xmmWrap5, XMM_GT			; (dBC > 0.5) ? -1 : 0

	MvWrap	xmmWrap5, MvSize ptr [flt_wrap_mask]; Wrap ? -1 : 0
	movaps	xmmWrap4, xmmWrap1					; temporary copy of (dAB > 0.5) ? -1 : 0
	andps	xmmWrap1, xmmWrap2					; ((dAB > 0.5) && (dAC > 0.5)) ? -1 : 0
	andps	xmmWrap2, xmmWrap3					; ((dAC > 0.5) && (dBC > 0.5)) ? -1 : 0
	andps	xmmWrap4, xmmWrap3					; ((dAB > 0.5) && (dBC > 0.5)) ? -1 : 0
	movaps	xmmWrap3, xmmWrap1
	orps	xmmWrap3, xmmWrap2
	orps	xmmWrap3, xmmWrap4
	andps	xmmWrap5, xmmWrap3
	MvWrap	xmmWrap3, MvSize ptr [flt_wrap_adds]; 1.0 1.0 1.0 1.0
	movmskps edx, xmmWrap5						; just store the sign bits
IF		(NUM_TEXTURES EQ 1)
	test	edx, 03h							; check for either wrap (1 texture)
ELSE
	test	edx, 0fh							; check for any wrap (2 textures)
ENDIF
	je		WrapDone							; early out


	MvWrap	MvSize ptr [flt4_temp1], xmmWrap1	; temporary copy of ((dAB > 0.5) && (dAC > 0.5)) ? -1 : 0

	; xmmWrap2 = ((dAC > 0.5) && (dBC > 0.5)) ? -1 : 0
	andps	xmmWrap2, xmmWrap3					; ((dAC > 0.5) && (dBC > 0.5)) ? 1.0 : 0
	movaps	xmmWrap5, xmmTxC
	xorps	xmmWrap3, xmmWrap2					; ((dAC > 0.5) && (dBC > 0.5)) ? 0 : 1.0
	cmpps	xmmWrap5, xmmTxB, XMM_LT			; (C <  B) ? -1 : 0
	movaps	xmmWrap1, xmmWrap5
	andnps	xmmWrap5, xmmWrap2					; ((dAC > 0.5) && (dBC > 0.5) && (C >= B)) ? 1.0 : 0
	andps	xmmWrap1, xmmWrap2					; ((dAC > 0.5) && (dBC > 0.5) && (C <  B)) ? 1.0 : 0
	addps	xmmTxB, xmmWrap5					; B += ((dAC > 0.5) && (dBC > 0.5) && (C >= B)) ? 1.0 : 0
	addps	xmmTxA, xmmWrap5					; A += ((dAC > 0.5) && (dBC > 0.5) && (C >= B)) ? 1.0 : 0
	addps	xmmTxC, xmmWrap1					; C += ((dAC > 0.5) && (dBC > 0.5) && (C <  B)) ? 1.0 : 0

	; xmmWrap4 = ((dAB > 0.5) && (dBC > 0.5))
	andps	xmmWrap4, xmmWrap3					; ((dBC > 0.5) && (dAB > 0.5)) ? 1.0 : 0
	movaps	xmmWrap5, xmmTxB
	xorps	xmmWrap3, xmmWrap4					; ((dBC > 0.5) && (dAB > 0.5)) ? xmmWrapOne-xmmWrapOne : xmmWrapOne
	cmpps	xmmWrap5, xmmTxA, XMM_LT			; (B <  A) ? -1 : 0
	movaps	xmmWrap2, xmmWrap5
	andnps	xmmWrap5, xmmWrap4					; ((dBC > 0.5) && (dAB > 0.5) && (B >= A)) ? 1.0 : 0
	andps	xmmWrap2, xmmWrap4					; ((dBC > 0.5) && (dAB > 0.5) && (B <  A)) ? 1.0 : 0
	addps	xmmTxA, xmmWrap5					; A += ((dBC > 0.5) && (dAB > 0.5) && (B >= A)) ? xmmWrapOne : 0
	addps	xmmTxC, xmmWrap5					; C += ((dBC > 0.5) && (dAB > 0.5) && (B >= A)) ? xmmWrapOne : 0
	addps	xmmTxB, xmmWrap2					; B += ((dBC > 0.5) && (dAB > 0.5) && (B <  A)) ? xmmWrapOne : 0

	movaps	xmmWrap5, xmmTxA
	cmpps	xmmWrap5, xmmTxC, XMM_LT			; (A <  C) ? -1 : 0
	andps	xmmWrap3, oword ptr [flt4_temp1]	; ((dAB > 0.5) && (dAC > 0.5)) ? -1 : 0
	movaps	xmmWrap1, xmmWrap5
	andps	xmmWrap5, xmmWrap3					; ((dAB > 0.5) && (dAC > 0.5) && (A <  C)) ? xmmWrapOne : 0
	andnps	xmmWrap1, xmmWrap3					; ((dAB > 0.5) && (dAC > 0.5) && (A >= C)) ? xmmWrapOne : 0
	addps	xmmTxA, xmmWrap5					; A += ((dAB > 0.5) && (dAC > 0.5) && (A <  C)) ? xmmWrapOne : 0
	addps	xmmTxB, xmmWrap1					; B += ((dAB > 0.5) && (dAC > 0.5) && (A >= C)) ? xmmWrapOne : 0
	addps	xmmTxC, xmmWrap1					; C += ((dAB > 0.5) && (dAC > 0.5) && (A >= C)) ? xmmWrapOne : 0

align 16
WrapDone:
	movlps	xmmVa, qword ptr [TempVa]
	movaps	xmmVbc, oword ptr [TempVbc]
ENDM

;######################################################################################################
;######################################################################################################
;##
;## Triangle Rendering Macros
;##
;######################################################################################################
;######################################################################################################
SEND_FULL_TRIANGLE	MACRO	
LOCAL	OneTexture_WrapDone, TwoTextures, TwoTextures_WrapDone, NoWrap, WrapDone, NotFlatShaded, NextTriangle
LOCAL	VaStart, VaNoColor, VaNoWBuffer, VaNoWBuffer_CheckZ, VaNoWbuffer_CheckWfbi, VaNoWbuffer_Wfbi_NoVFog, VaHasWBuffer, VaHasWbuffer_CheckZ, VaHasWbuffer_CheckWfbi, VaPersCorrect, VaNoPerspective
LOCAL	VbStart, VbNoColor, VbNoWBuffer, VbNoWBuffer_CheckZ, VbNoWbuffer_CheckWfbi, VbNoWbuffer_Wfbi_NoVFog, VbHasWBuffer, VbHasWbuffer_CheckZ, VbHasWbuffer_CheckWfbi, VbPersCorrect, VbNoPerspective
LOCAL	VcStart, VcNoColor, VcNoWBuffer, VcNoWBuffer_CheckZ, VcNoWbuffer_CheckWfbi, VcNoWbuffer_Wfbi_NoVFog, VcHasWBuffer, VcHasWbuffer_CheckZ, VcHasWbuffer_CheckWfbi, VcPersCorrect, VcNoPerspective
LOCAL	VaAlpha_RHW,   VaAlpha_NoRHW,   VaAlpha_NoTexture0,   VaAlpha_NoTexture1
LOCAL	VbAlpha_RHW,   VbAlpha_NoRHW,   VbAlpha_NoTexture0,   VbAlpha_NoTexture1
LOCAL	VcAlpha_RHW,   VcAlpha_NoRHW,   VcAlpha_NoTexture0,   VcAlpha_NoTexture1
LOCAL	VaNoAlpha_RHW, VaNoAlpha_NoRHW, VaNoAlpha_NoTexture0, VaNoAlpha_NoTexture1 
LOCAL	VbNoAlpha_RHW, VbNoAlpha_NoRHW, VbNoAlpha_NoTexture0, VbNoAlpha_NoTexture1 
LOCAL	VcNoAlpha_RHW, VcNoAlpha_NoRHW, VcNoAlpha_NoTexture0, VcNoAlpha_NoTexture1

HWPTR_DWORD_OFFSET = 0
SBUFF_DWORD_OFFSET = 0

;**********************************************************
; Try to get all the reads done before doing writes to HW
;**********************************************************
IFE (SEND_TX0 EQ FALSE)
	mov		edx, dword ptr [__asm_data + _T0_OFFSET]		; Texture 0 coordinate index
	movlps	xmmTxA, qword ptr [VaPtr+edx]					; ----- ----- vA.t0 vA.s0
	movlps	xmmTxB, qword ptr [VbPtr+edx]					; ----- ----- vB.t0 vB.s0
	movlps	xmmTxC, qword ptr [VcPtr+edx]					; vA.y  vA.x  vC.t0 vC.s0
ENDIF

IF (SEND_TX1 EQ CHECK)
	test	dword ptr _localFlags[esp+STACK_OFFSET], FLAG_TX1
	jne		TwoTextures
ENDIF

IF ((SEND_TX1 EQ CHECK) OR (SEND_TX1 EQ FALSE))
  IF ((SEND_WRAP EQ CHECK) OR (SEND_WRAP EQ TRUE))
   IF (SEND_WRAP EQ CHECK)
	test	dword ptr _localFlags[esp+STACK_OFFSET], ((FLAG_WRAP_T0_S) OR (FLAG_WRAP_T0_T))
	je		NoWrap
   ENDIF
	WRAP 1
  ENDIF
  IF (SEND_TX1 EQ CHECK)
	jmp		WrapDone
  ENDIF
ENDIF

IF ((SEND_TX1 EQ CHECK) OR (SEND_TX1 EQ TRUE))
TwoTextures:
	mov		edx, dword ptr [__asm_data + T1_OFFSET]			; Texture 1 coordinate index
	movhps	xmmTxA, qword ptr [VaPtr+edx]					; vA.t1 vA.s1 vA.t0 vA.s0
	movhps	xmmTxB, qword ptr [VbPtr+edx]					; vB.t1 vB.s1 vB.t0 vB.s0
	movhps	xmmTxC, qword ptr [VcPtr+edx]					; vC.t1 vC.s1 vC.t0 vC.s0

  IF ((SEND_WRAP EQ CHECK) OR (SEND_WRAP EQ TRUE))
   IF (SEND_WRAP EQ CHECK)
	; test for any wrap in either texture
	test	dword ptr _localFlags[esp+STACK_OFFSET], ((FLAG_WRAP_T0_S) OR (FLAG_WRAP_T0_T) OR (FLAG_WRAP_T1_S) OR (FLAG_WRAP_T1_T))
	je		NoWrap
   ENDIF
	WRAP 2
  ENDIF
ENDIF

WrapDone:
NoWrap:
	; add pixel center (0.5) to x&y
	; add texture scale
	movaps	xmmVb, oword ptr [PixelOffset]					; PIXEL_CENTER PIXEL_CENTER PIXEL_CENTER PIXEL_CENTER (barrowing Vb for now)
IF ((SEND_TX1 EQ CHECK) OR (SEND_TX1 EQ TRUE))
	movaps	xmmT1, oword ptr [TextureScale]					; TxScale [t1's1't0's0]
ELSEIF ((SEND_TX0 EQ CHECK) OR (SEND_TX0 EQ TRUE))
	movlps	xmmT1, oword ptr [TextureScale]					; TxScale [--'--'t0's0]
ENDIF

	movaps	xmmT2, oword ptr [ScaleZW]
	addps	xmmVa, xmmVb									; vA.y+PIXEL_CENTER vA.x+PIXEL_CENTER
IFE ((SEND_TX0 EQ FALSE) AND (SEND_TX1 EQ FALSE))
	mulps	xmmTxA, xmmT1									; TxA [t1's1't0's0]*scale
ENDIF
	addps	xmmVbc, xmmVb									; vB.y+PIXEL_CENTER vB.x+PIXEL_CENTER vC.y+PIXEL_CENTER vC.x+PIXEL_CENTER
IFE ((SEND_TX0 EQ FALSE) AND (SEND_TX1 EQ FALSE))
	mulps	xmmTxB, xmmT1									; TxB [t1's1't0's0]*scale
ENDIF
IFE ((SEND_TX0 EQ FALSE) AND (SEND_TX1 EQ FALSE))
	mulps	xmmTxC, xmmT1									; TxC [t1's1't0's0]*scale
ENDIF
	movhlps	xmmVb, xmmVbc									; xmmVbc is now xmmVc

	; load z&w
	movhps	xmmVa, qword ptr [VaPtr+8]						; vA.rhw vA.z vA.y+PIXEL_CENTER vA.x+PIXEL_CENTER
	movhps	xmmVc, qword ptr [VcPtr+8]						; vC.rhw vC.z vC.y+PIXEL_CENTER vC.x+PIXEL_CENTER
	movhps	xmmVb, qword ptr [VbPtr+8]						; vB.rhw vB.z vB.y+PIXEL_CENTER vB.x+PIXEL_CENTER

IF ((SEND_TX1 EQ CHECK) OR (SEND_TX1 EQ TRUE))
	movaps	xmmT1, oword ptr [TextureOffset]				; TxOffset [t1's1't0's0]
ELSEIF ((SEND_TX0 EQ CHECK) OR (SEND_TX0 EQ TRUE))
	movlps	xmmT1, qword ptr [TextureOffset]				; TxOffset [--'--'t0's0]
ENDIF

	mulps	xmmVa, xmmT2									;  vA.rhw*SCALE_W vA.z*_SCALEZ vA.y+PIXEL_CENTER vA.x+PIXEL_CENTER
IFE ((SEND_TX0 EQ FALSE) AND (SEND_TX1 EQ FALSE))
	addps	xmmTxA, xmmT1									; (TxA [t1's1't0's0]*scale)+offset
ENDIF

	mov		edx, dword ptr [__asm_data + CMD_START]
	mov		VcPtr, dword ptr _localFlags[esp+STACK_OFFSET]	; VcPtr will be the local flags for a while
IF SEND_2ND_SPEC_PASS
	mov		VbPtr, dword ptr _specCmdFull[esp+STACK_OFFSET]	; VbPtr is hosed now
ENDIF


; This is for some really bad rendering types only -> be very carefull about using it
IF ((START_BOUNDARY EQ START_ODD) AND CHECK_SPLIT_LOWER_XMM_QWORD_WRITE AND ALIGN_QWORD_HWPTR_WRITES)
	SET4IMM	NOP_PACKET, CMD_FIFO							; send a nop to the cmd fifo only 
ENDIF

;**********************************************************
; Send Triangle Start Command
;**********************************************************
	SET4GPR	edx, CMD_FIFO									; send CMD_START
	SET4GPR	VbPtr, SPEC_BUFFER								; send CMD_START for 2nd pass

	mulps	xmmVb, xmmT2									;  vB.rhw*SCALE_W vB.z*_SCALEZ vB.y+PIXEL_CENTER vB.x+PIXEL_CENTER
IFE ((SEND_TX0 EQ FALSE) AND (SEND_TX1 EQ FALSE))
	addps	xmmTxB, xmmT1									; (TxB [t1's1't0's0]*scale)+offset
ENDIF

IF ((SEND_FLAT EQ CHECK) OR (SEND_FLAT EQ TRUE))
  IF (SEND_FLAT EQ CHECK)
	test	VcPtr, (FLAG_FLAT)
	jz		NotFlatShaded
  ENDIF
	movq	mmxCcb, mmxCa
	punpckldq mmxCcb, mmxCcb
  IF SEND_2ND_SPEC_PASS
	movq	mmxScb, mmxSa
	punpckldq mmxScb, mmxScb
  ENDIF
NotFlatShaded:
ENDIF

;******************************************************************
; Vertex A
;******************************************************************
VaStart:
; Send X and Y
	SET8XMM_L	xmmVa, BOTH_DESTS							; SEND vA.x & vA.y

	movhlps	xmmVa, xmmVa									; w  z  w  z

;--- Send color
;	test	dword ptr [__asm_data + SETUP_FLAG], (SST_SETUP_RGB OR SST_SETUP_A)
IF ((SEND_COLOR EQ CHECK) OR (SEND_COLOR EQ TRUE))
  IF (SEND_COLOR EQ CHECK)
	test	VcPtr, (FLAG_COLOR)
	je		SHORT VaNoColor
  ENDIF
	SET4MMX	mmxCa, CMD_FIFO									; SEND Color
	SET4MMX	mmxSa, SPEC_BUFFER								; SEND Specular
VaNoColor:
ENDIF
; mmxCa and mmxSa are now available

;**********************************************************
; Z-FOG, W-FOG, and RHW Implementation
;
;	DCT_FIX:	w = RHW * scaleW
;				if(!STATE_REQUIRES_WBUFFER)
;					if(SST_SETUP_Z)
;						send z*SCALEZ1
;					if(SST_SETUP_Wfbi)
;						if(STATE_REQUIRES_VERTEXFOG)
;							send (float)(255 - spec.alpha)
;						else (HW_FOG???)
;							send RHW (even for DCT fix)
;				else
;					if(SST_SETUP_Z)
;						if(STATE_REQUIRES_VERTEXFOG)	<--- do I really care here? (I must send a z!)
;							send (float)((255 - spec.alpha) * 256)
;					if(SST_SETUP_Wfbi)
;	DCT_FIX:			send w
;	ELSE:				send RHW * scaleAW + scaleBW
;
;				if(SST_SETUP_W0)
;					send w
;
;	Notes:
;		Leave w in xmm reg (used by texturing for perspective correction)
;		Flags are now in VcPtr, restore VcPtr and move the flags to edx in this block
;**********************************************************
	movd	VbPtr, mmxVcb									; restore VbPtr
	mov		edx, VcPtr										; edx is now the local flag

;	test	dword ptr [__asm_data + RENDER_STATE], STATE_REQUIRES_WBUFFER
IF (SEND_WBUFFER EQ CHECK)
	test	VcPtr, (FLAG_WBUFFER)
	jne		VaHasWBuffer
ENDIF

IF ((SEND_WBUFFER EQ CHECK) OR (SEND_WBUFFER EQ FALSE))
VaNoWBuffer:
	movd	mmxCa, edx										; store the flags temporily in the A color mmx reg
VaNoWBuffer_CheckZ:
  IF ((SEND_Z EQ CHECK) OR (SEND_Z EQ TRUE))
   IF (SEND_Z EQ CHECK)
;	test	dword ptr [__asm_data + SETUP_FLAG], SST_SETUP_Z
	test	VcPtr, (FLAG_Z)
	je		SHORT VaNoWbuffer_CheckWfbi
   ENDIF
	SET4XMM	xmmVa, BOTH_DESTS								; SEND z*_SCALEZ
  ENDIF

VaNoWbuffer_CheckWfbi:
	shufps	xmmVa, xmmVa, 055h								;  w  w  w  w
	movd	VcPtr, mmxVac									; restore VcPtr
  IF ((SEND_WFBI EQ CHECK) OR (SEND_WFBI EQ TRUE))
   IF ((SEND_WFBI EQ CHECK) OR (SEND_WBUFFER EQ CHECK))
;	test	dword ptr [__asm_data + SETUP_FLAG], SST_SETUP_Wfbi
	test	edx, (FLAG_WFBI)
	je		VaPersCorrect
   ENDIF
   IF ((SEND_VFOG EQ CHECK) OR (SEND_VFOG EQ TRUE))
   IF (SEND_VFOG EQ CHECK)
;	test	dword ptr [__asm_data + RENDER_STATE], STATE_REQUIRES_VERTEXFOG
	test	edx, (FLAG_VFOG)
	je		SHORT VaNoWbuffer_Wfbi_NoVFog
    ENDIF

	mov		edx, dword ptr [__asm_data + SPECULAR_OFFSET]
	movzx	VaPtr, byte ptr [edx + VaPtr + 3]				; vA.specular.alpha
	movzx	VbPtr, byte ptr [edx + VbPtr + 3]				; vB.specular.alpha
	movzx	edx,   byte ptr [edx + VcPtr + 3]				; vC.specular.alpha
	mov		VaPtr,     dword ptr [__asm_rev_lookup+ VaPtr*4]; LUT = (float)(255 - vA.specular.alpha)
	movd	mmxWfbicb, dword ptr [__asm_rev_lookup+ VbPtr*4]; LUT = (float)(255 - vB.specular.alpha) <-shares 
	movd	mmxT1,     dword ptr [__asm_rev_lookup+   edx*4]; LUT = (float)(255 - vC.specular.alpha)

	SET4GPR	VaPtr, BOTH_DESTS								; SEND (float)(255 - vA.specular.alpha)
	movd	edx, mmxCa										; edx is the local flag again
	punpckldq mmxWfbicb, mmxT1								; vC.wfbi		vB.wfbi
	movd	VbPtr, mmxVcb									; restore VbPtr
   ENDIF ;VFOG
   IF ((SEND_VFOG EQ CHECK) OR (SEND_WFBI EQ CHECK) OR (SEND_Z EQ CHECK) OR (SEND_WBUFFER EQ CHECK))
	jmp		VaPersCorrect
   ENDIF

   IF ((SEND_VFOG EQ CHECK) OR (SEND_VFOG EQ FALSE))
VaNoWbuffer_Wfbi_NoVFog:
ifdef DCT_FIX
	mov		VaPtr, dword ptr [VaPtr+12]						; vA.rhw
	SET4GPR VaPtr, BOTH_DESTS								; SEND rhw
else
	SET4XMM	xmmVa, BOTH_DESTS								; SEND rhw
endif
   ENDIF ;NO VFOG
   IF ((SEND_VFOG EQ CHECK) OR (SEND_WFBI EQ CHECK) OR (SEND_Z EQ CHECK) OR (SEND_WBUFFER EQ CHECK))
	jmp		VaPersCorrect
   ENDIF
  ENDIF ;WFBI
ENDIF ;WBUFFER

IF ((SEND_WBUFFER EQ CHECK) OR (SEND_WBUFFER EQ TRUE))
VaHasWBuffer:
	shufps	xmmVa, xmmVa, 055h								;  w  w  w  w
	movd	mmxCa, edx										; store the flags temporily in the A color mmx reg
VaHasWbuffer_CheckZ:
  IF ((SEND_Z EQ CHECK) OR (SEND_Z EQ TRUE))
   IF (SEND_Z EQ CHECK)
;	test	dword ptr [__asm_data + SETUP_FLAG], SST_SETUP_Z
	test	VcPtr, (FLAG_Z)
	je		SHORT VaHasWbuffer_CheckWfbi
   ENDIF

	mov		edx, dword ptr [__asm_data + SPECULAR_OFFSET]
	movd	VcPtr, mmxVac									; restore VcPtr
	movzx	VaPtr, byte ptr [edx + VaPtr + 3]				; vA.specular.alpha
	movzx	VbPtr, byte ptr [edx + VbPtr + 3]				; vB.specular.alpha
	movzx	edx,   byte ptr [edx + VcPtr + 3]				; vC.specular.alpha
	mov		VaPtr,     dword ptr [__asm_rev_lookup+ VaPtr*4]; LUT = (float)(255 - vA.specular.alpha)
	movd	mmxWfbicb, dword ptr [__asm_rev_lookup+ VbPtr*4]; LUT = (float)(255 - vB.specular.alpha) <-shares 
	movd	mmxT1,     dword ptr [__asm_rev_lookup+   edx*4]; LUT = (float)(255 - vC.specular.alpha)

	; multiply by 256 by adding 8 to the exponent
	add		VaPtr, (8 SHL 23)								; (float)(255 - vA.specular.alpha) * 256.0
	movd	VbPtr, mmxVcb									; restore VbPtr
	punpckldq mmxWfbicb, mmxT1								; vC.wfbi		vB.wfbi
	movd	VcPtr, mmxVac									; restore VcPtr
	paddd	mmxWfbicb, qword ptr [Const_Mult256]			; (8 SHL 23)	(8 SHL 23)
	movd	edx, mmxCa										; edx is the local flag again
	SET4GPR	VaPtr, BOTH_DESTS								; SEND (float)(255 - vA.specular.alpha) * 256.0
  ENDIF ;Z

VaHasWbuffer_CheckWfbi:
	movd	VcPtr, mmxVac									; restore VcPtr
;	test	dword ptr [__asm_data + SETUP_FLAG], SST_SETUP_Wfbi
  IF ((SEND_WFBI EQ CHECK) OR (SEND_WFBI EQ TRUE))
   IF (SEND_WFBI EQ CHECK)
	test	edx, (FLAG_WFBI)
	je		VaPersCorrect
   ENDIF
ifdef DCT_FIX
	SET4XMM	xmmVa, BOTH_DESTS								; SEND w
else
	mulss	xmmVa, dword ptr [__asm_data + WBUFFER_SCALE_AW]; rhw rhw rhw rhw*WBUFFER_SCALE_AW
	addss	xmmVa, dword ptr [__asm_data + WBUFFER_SCALE_BW]; rhw rhw rhw rhw*WBUFFER_SCALE_AW+WBUFFER_SCALE_BW
	SET4XMM	xmmVa, BOTH_DESTS								; rhw rhw rhw SEND rhw*WBUFFER_SCALE_AW+WBUFFER_SCALE_BW
	movhlps	xmmVa, xmmVa									; rhw rhw rhw rhw
endif
  ENDIF ;WFBI
ENDIF ;WBUFFER

; VaPtr is available now
;**********************************************************
; RHW and Multitexture Implementation
;
;	DCT_FIX:	xmm = w
;	ELSE:		xmm = RHW
;	if(STATE_REQUIRES_PERSPECTIVE)
;		s/t = ((u/v * SCALE_S/T) + TEXEL_OFFSET_S/T) * xmm
;	else
;		s/t = ((u/v * SCALE_S/T) + TEXEL_OFFSET_S/T)
;
; Notes:
;	Should already have the correct w or RHW
;	Only send RHW and textures to specular buffer if you have alpha
;**********************************************************
VaPersCorrect:
IFE ((SEND_TX0 EQ FALSE) AND (SEND_TX1 EQ FALSE))
 IF ((SEND_PC EQ CHECK) OR (SEND_PC EQ TRUE))
  IF (SEND_PC EQ CHECK)
;	test	dword ptr [__asm_data + RENDER_STATE], STATE_REQUIRES_PERSPECTIVE
	test	edx, (FLAG_PC)
	je		SHORT VaNoPerspective
  ENDIF
	mulps	xmmTxA, xmmVa									; (TS2*SCALE2+OFFSET2)*w (TS1*SCALE1+OFFSET1)*w
VaNoPerspective:
 ENDIF

 IF (SEND_ALPHA EQ CHECK)
;	mov		edx, dword ptr [__asm_data + _RC]
;	cmp		dword ptr [edx + OFFSET_RC_ALPHABLEND_ENABLE], 0
	test	edx, (FLAG_ALPHA)
	jne		SHORT VaAlpha_RHW
 ENDIF

 IF ((SEND_ALPHA EQ CHECK) OR (SEND_ALPHA EQ FALSE))
;--- no alpha
VaNoAlpha_RHW:
  IF ((SEND_W EQ CHECK) OR (SEND_W EQ TRUE))
   IF (SEND_W EQ CHECK)
;	test	dword ptr [__asm_data + SETUP_FLAG], SST_SETUP_W0
	test	edx, (FLAG_W)
	je		SHORT VaNoAlpha_NoRHW
   ENDIF
	SET4XMM	xmmVa, CMD_FIFO									; SEND w (RHW)
VaNoAlpha_NoRHW:
  ENDIF

  IF ((SEND_TX0 EQ CHECK) OR (SEND_TX0 EQ TRUE))
   IF (SEND_TX0 EQ CHECK)
;	test	dword ptr [__asm_data + SETUP_FLAG], SST_SETUP_ST0
	test	edx, (FLAG_TX0)
	je		SHORT VaNoAlpha_NoTexture0
   ENDIF
	SET8XMM_L	xmmTxA, CMD_FIFO							; SEND first texture
VaNoAlpha_NoTexture0:
  ENDIF

;	test	dword ptr [__asm_data + SETUP_FLAG], SST_SETUP_ST1
  IF ((SEND_TX1 EQ CHECK) OR (SEND_TX1 EQ TRUE))
   IF ((SEND_TX1 EQ CHECK) OR (SEND_ALPHA EQ CHECK))
	test	edx, (FLAG_TX1)
	je		SHORT VbStart
   ENDIF
	SET8XMM_H	xmmTxA, CMD_FIFO							; SEND second texture
VaNoAlpha_NoTexture1:
  ENDIF
  IF (SEND_ALPHA EQ CHECK)
	jmp		SHORT VbStart
  ENDIF
 ENDIF ;NO ALPHA

 IF ((SEND_ALPHA EQ CHECK) OR (SEND_ALPHA EQ TRUE))
;--- has alpha
VaAlpha_RHW:
  IF ((SEND_W EQ CHECK) OR (SEND_W EQ TRUE))
   IF (SEND_W EQ CHECK)
	test	edx, (FLAG_W)
	je		SHORT VaAlpha_NoRHW
   ENDIF
	SET4XMM	xmmVa, BOTH_DESTS								; SEND w (RHW)
VaAlpha_NoRHW:
  ENDIF

  IF ((SEND_TX0 EQ CHECK) OR (SEND_TX0 EQ TRUE))
   IF (SEND_TX0 EQ CHECK)
	test	edx, (FLAG_TX0)
	je		SHORT VaAlpha_NoTexture0
   ENDIF
	SET8XMM_L	xmmTxA, BOTH_DESTS							; SEND first texture
VaAlpha_NoTexture0:
 ENDIF

  IF ((SEND_TX1 EQ CHECK) OR (SEND_TX1 EQ TRUE))
   IF ((SEND_TX1 EQ CHECK) OR (SEND_ALPHA EQ CHECK))
	test	edx, (FLAG_TX1)
	je		SHORT VbStart
   ENDIF
	SET8XMM_H	xmmTxA, BOTH_DESTS							; SEND second texture
VaAlpha_NoTexture1:
  ENDIF
 ENDIF ;ALPHA
ENDIF ;No textures

;******************************************************************
; Vertex B
;******************************************************************
VbStart:
;--- Send X and Y
	SET8XMM_L	xmmVb, BOTH_DESTS							; SEND vB.x & vB.y

	mulps	xmmVc, xmmT2									;  vC.rhw*SCALE_W vC.z*_SCALEZ vC.y+PIXEL_CENTER vC.x+PIXEL_CENTER
IFE ((SEND_TX0 EQ FALSE) AND (SEND_TX1 EQ FALSE))
	addps	xmmTxC, xmmT1									; (TxC [t1's1't0's0]*scale)+offset
ENDIF

;--- Send color
IF ((SEND_COLOR EQ CHECK) OR (SEND_COLOR EQ TRUE))
  IF (SEND_COLOR EQ CHECK)
	test	edx, (FLAG_COLOR)
	je		SHORT VbNoColor
  ENDIF
	SET4MMX	mmxCcb, CMD_FIFO								; SEND Color
	SET4MMX	mmxScb, SPEC_BUFFER								; SEND Specular
	punpckhdq mmxCcb, mmxCcb								; vC.d+s	vC.d+s
	punpckhdq mmxScb, mmxScb								; vC.spec	vC.spec
VbNoColor:
ENDIF


;--- Send Z and fog
	movhlps	xmmVb, xmmVb									; w  z  w  z
IF (SEND_WBUFFER EQ CHECK)
	test	edx, (FLAG_WBUFFER)
	jne		VbHasWBuffer
ENDIF

IF ((SEND_WBUFFER EQ CHECK) OR (SEND_WBUFFER EQ FALSE))
VbNoWBuffer:
VbNoWBuffer_CheckZ:
  IF ((SEND_Z EQ CHECK) OR (SEND_Z EQ TRUE))
   IF (SEND_Z EQ CHECK)
	test	edx, (FLAG_Z)
	je		SHORT VbNoWbuffer_CheckWfbi
   ENDIF
	SET4XMM	xmmVb, BOTH_DESTS								; SEND z*_SCALEZ
  ENDIF

VbNoWbuffer_CheckWfbi:
	shufps	xmmVb, xmmVb, 055h								;  w  w  w  w
  IF ((SEND_WFBI EQ CHECK) OR (SEND_WFBI EQ TRUE))
   IF ((SEND_WFBI EQ CHECK) OR (SEND_Z EQ CHECK) OR (SEND_WBUFFER EQ CHECK))
	test	edx, (FLAG_WFBI)
	je		VbPersCorrect
   ENDIF
   IF ((SEND_VFOG EQ CHECK) OR (SEND_VFOG EQ TRUE))
    IF (SEND_VFOG EQ CHECK)
	test	edx, (FLAG_VFOG)
	je		SHORT VbNoWbuffer_Wfbi_NoVFog
    ENDIF
	SET4MMX	mmxWfbicb, BOTH_DESTS
	punpckhdq mmxWfbicb, mmxWfbicb							; vC.wfbi	vC.wfbi
   ENDIF ;VFOG
   IF ((SEND_VFOG EQ CHECK) OR (SEND_WFBI EQ CHECK) OR (SEND_Z EQ CHECK) OR (SEND_WBUFFER EQ CHECK))
	jmp		VbPersCorrect
   ENDIF

   IF ((SEND_VFOG EQ CHECK) OR (SEND_VFOG EQ FALSE))
VbNoWbuffer_Wfbi_NoVFog:
ifdef DCT_FIX
	mov		VbPtr, dword ptr [VbPtr+12]						; vB.rhw
	SET4GPR VbPtr, BOTH_DESTS								; SEND rhw
else
	SET4XMM	xmmVb, BOTH_DESTS								; SEND rhw
endif
   ENDIF ;NO VFOG
   IF ((SEND_VFOG EQ CHECK) OR (SEND_WFBI EQ CHECK) OR (SEND_Z EQ CHECK) OR (SEND_WBUFFER EQ CHECK))
	jmp		VbPersCorrect
   ENDIF
  ENDIF ;WFBI
ENDIF ;WBUFFER

IF ((SEND_WBUFFER EQ CHECK) OR (SEND_WBUFFER EQ TRUE))
VbHasWBuffer:
	shufps	xmmVb, xmmVb, 055h								;  w  w  w  w
VbHasWbuffer_CheckZ:
  IF ((SEND_Z EQ CHECK) OR (SEND_Z EQ TRUE))
   IF (SEND_Z EQ CHECK)
	test	edx, (FLAG_Z)
	je		SHORT VbHasWbuffer_CheckWfbi
   ENDIF
	SET4MMX	mmxWfbicb, BOTH_DESTS
	punpckhdq mmxWfbicb, mmxWfbicb							; vC.wfbi	vC.wfbi
  ENDIF ;Z

VbHasWbuffer_CheckWfbi:
  IF ((SEND_WFBI EQ CHECK) OR (SEND_WFBI EQ TRUE))
   IF ((SEND_WFBI EQ CHECK) OR (SEND_WBUFFER EQ CHECK))
	test	edx, (FLAG_WFBI)
	je		SHORT VbPersCorrect
   ENDIF
ifdef DCT_FIX
	SET4XMM	xmmVb, BOTH_DESTS								; SEND w
else
	mulss	xmmVb, dword ptr [__asm_data + WBUFFER_SCALE_AW]; rhw rhw rhw rhw*WBUFFER_SCALE_AW
	addss	xmmVb, dword ptr [__asm_data + WBUFFER_SCALE_BW]; rhw rhw rhw rhw*WBUFFER_SCALE_AW+WBUFFER_SCALE_BW
	SET4XMM	xmmVb, BOTH_DESTS								; rhw rhw rhw SEND rhw*WBUFFER_SCALE_AW+WBUFFER_SCALE_BW
	movhlps	xmmVb, xmmVb									; rhw rhw rhw rhw
endif
  ENDIF ;WFBI
ENDIF ;WBUFFER

; VbPtr is free now
;--- Send rhw and textures
VbPersCorrect:
IFE ((SEND_TX0 EQ FALSE) AND (SEND_TX1 EQ FALSE))
 IF ((SEND_PC EQ CHECK) OR (SEND_PC EQ TRUE))
  IF (SEND_PC EQ CHECK)
	test	edx, (FLAG_PC)
	je		SHORT VbNoPerspective
  ENDIF
	mulps	xmmTxB, xmmVb									; (TS2*SCALE2+OFFSET2)*w (TS1*SCALE1+OFFSET1)*w
VbNoPerspective:
 ENDIF

 IF (SEND_ALPHA EQ CHECK)
	test	edx, (FLAG_ALPHA)
	jne		SHORT VbAlpha_RHW
 ENDIF

 IF ((SEND_ALPHA EQ CHECK) OR (SEND_ALPHA EQ FALSE))
;--- no alpha
VbNoAlpha_RHW:
  IF ((SEND_W EQ CHECK) OR (SEND_W EQ TRUE))
   IF (SEND_W EQ CHECK)
	test	edx, (FLAG_W)
	je		SHORT VbNoAlpha_NoRHW
   ENDIF
	SET4XMM	xmmVb, CMD_FIFO									; SEND w (RHW)
VbNoAlpha_NoRHW:
  ENDIF

  IF ((SEND_TX0 EQ CHECK) OR (SEND_TX0 EQ TRUE))
   IF (SEND_TX0 EQ CHECK)
	test	edx, (FLAG_TX0)
	je		SHORT VbNoAlpha_NoTexture0
   ENDIF
	SET8XMM_L	xmmTxB, CMD_FIFO							; SEND first texture
VbNoAlpha_NoTexture0:
  ENDIF

  IF ((SEND_TX1 EQ CHECK) OR (SEND_TX1 EQ TRUE))
   IF ((SEND_TX1 EQ CHECK) OR (SEND_ALPHA EQ CHECK))
	test	edx, (FLAG_TX1)
	je		SHORT VcStart
   ENDIF
	SET8XMM_H	xmmTxB, CMD_FIFO							; SEND second texture
VbNoAlpha_NoTexture1:
  ENDIF
  IF (SEND_ALPHA EQ CHECK)
	jmp		SHORT VcStart
  ENDIF
 ENDIF ;NO ALPHA

 IF ((SEND_ALPHA EQ CHECK) OR (SEND_ALPHA EQ TRUE))
;--- has alpha
VbAlpha_RHW:
  IF ((SEND_W EQ CHECK) OR (SEND_W EQ TRUE))
   IF (SEND_W EQ CHECK)
	test	edx, (FLAG_W)
	je		SHORT VbAlpha_NoRHW
   ENDIF
	SET4XMM	xmmVb, BOTH_DESTS								; SEND w (RHW)
VbAlpha_NoRHW:
  ENDIF

  IF ((SEND_TX0 EQ CHECK) OR (SEND_TX0 EQ TRUE))
   IF (SEND_TX0 EQ CHECK)
	test	edx, (FLAG_TX0)
	je		SHORT VbAlpha_NoTexture0
   ENDIF
	SET8XMM_L	xmmTxB, BOTH_DESTS							; SEND first texture
VbAlpha_NoTexture0:
  ENDIF

  IF ((SEND_TX1 EQ CHECK) OR (SEND_TX1 EQ TRUE))
   IF ((SEND_TX1 EQ CHECK) OR (SEND_ALPHA EQ CHECK))
	test	edx, (FLAG_TX1)
	je		SHORT VcStart
   ENDIF
	SET8XMM_H	xmmTxB, BOTH_DESTS							; SEND second texture
VbAlpha_NoTexture1:
  ENDIF

 ENDIF ;SEND_2ND_SPEC_PASS
ENDIF ;No Textures

;******************************************************************
; Vertex C
;******************************************************************
VcStart:
;--- Send X and Y
	SET8XMM_L	xmmVc, BOTH_DESTS							; SEND vC.x & vC.y

;--- Send color
IF ((SEND_COLOR EQ CHECK) OR (SEND_COLOR EQ TRUE))
  IF (SEND_COLOR EQ CHECK)
	test	edx, (FLAG_COLOR)
	je		SHORT VcNoColor
  ENDIF
	SET4MMX	mmxCcb, CMD_FIFO								; SEND Color
	SET4MMX	mmxScb, SPEC_BUFFER								; SEND Specular
VcNoColor:
ENDIF

	movhlps	xmmVc, xmmVc									; w  z  w  z
;--- Send Z and fog
IF (SEND_WBUFFER EQ CHECK)
	test	edx, (FLAG_WBUFFER)
	jne		VcHasWBuffer
ENDIF

IF ((SEND_WBUFFER EQ CHECK) OR (SEND_WBUFFER EQ FALSE))
VcNoWBuffer:
VcNoWBuffer_CheckZ:
  IF ((SEND_Z EQ CHECK) OR (SEND_Z EQ TRUE))
   IF (SEND_Z EQ CHECK)
	test	edx, (FLAG_Z)
	je		SHORT VcNoWbuffer_CheckWfbi
   ENDIF
	SET4XMM	xmmVc, BOTH_DESTS								; SEND z*_SCALEZ
  ENDIF

VcNoWbuffer_CheckWfbi:
	shufps	xmmVc, xmmVc, 055h							;  w  w  w  w
  IF ((SEND_WFBI EQ CHECK) OR (SEND_WFBI EQ TRUE))
   IF ((SEND_WFBI EQ CHECK) OR (SEND_WBUFFER EQ CHECK))
	test	edx, (FLAG_WFBI)
	je		VcPersCorrect
   ENDIF
   IF ((SEND_VFOG EQ CHECK) OR (SEND_VFOG EQ TRUE))
    IF (SEND_VFOG EQ CHECK)
   	test	edx, (FLAG_VFOG)
	je		SHORT VcNoWbuffer_Wfbi_NoVFog
    ENDIF
	SET4MMX	mmxWfbicb, BOTH_DESTS
   ENDIF ;VFOG
   IF ((SEND_VFOG EQ CHECK) OR (SEND_WFBI EQ CHECK) OR (SEND_Z EQ CHECK) OR (SEND_WBUFFER EQ CHECK))
	jmp		VcPersCorrect
   ENDIF

   IF ((SEND_VFOG EQ CHECK) OR (SEND_VFOG EQ FALSE))
VcNoWbuffer_Wfbi_NoVFog:
ifdef DCT_FIX
	mov		VcPtr, dword ptr [VcPtr+12]						; vC.rhw
	SET4GPR VcPtr, BOTH_DESTS								; SEND rhw
else
	SET4XMM	xmmVc, BOTH_DESTS								; SEND rhw
endif
   ENDIF ;NO VFOG
   IF ((SEND_VFOG EQ CHECK) OR (SEND_WFBI EQ CHECK) OR (SEND_Z EQ CHECK) OR (SEND_WBUFFER EQ CHECK))
	jmp		VcPersCorrect
   ENDIF
  ENDIF ;WFBI
ENDIF ;WBUFFER

IF ((SEND_WBUFFER EQ CHECK) OR (SEND_WBUFFER EQ TRUE))
VcHasWBuffer:
	shufps	xmmVc, xmmVc, 055h							;  w  w  w  w
VcHasWbuffer_CheckZ:
  IF ((SEND_Z EQ CHECK) OR (SEND_Z EQ TRUE))
   IF (SEND_Z EQ CHECK)
	test	edx, (FLAG_Z)
	je		SHORT VcHasWbuffer_CheckWfbi
   ENDIF
	SET4MMX	mmxWfbicb, BOTH_DESTS
  ENDIF ;Z

VcHasWbuffer_CheckWfbi:
  IF ((SEND_WFBI EQ CHECK) OR (SEND_WFBI EQ TRUE))
   IF (SEND_WFBI EQ CHECK)
	test	edx, (FLAG_WFBI)
	je		SHORT VcPersCorrect
   ENDIF
ifdef DCT_FIX
	SET4XMM	xmmVc, BOTH_DESTS								; SEND w
else
	mulss	xmmVc, dword ptr [__asm_data + WBUFFER_SCALE_AW]; rhw rhw rhw rhw*WBUFFER_SCALE_AW
	addss	xmmVc, dword ptr [__asm_data + WBUFFER_SCALE_BW]; rhw rhw rhw rhw*WBUFFER_SCALE_AW+WBUFFER_SCALE_BW
	SET4XMM	xmmVc, BOTH_DESTS								; rhw rhw rhw SEND rhw*WBUFFER_SCALE_AW+WBUFFER_SCALE_BW
	movhlps	xmmVc, xmmVc									; rhw rhw rhw rhw
endif
  ENDIF ;WFBI
ENDIF ;WBUFFER

; VcPtr is free now
VcPersCorrect:
IFE ((SEND_TX0 EQ FALSE) AND (SEND_TX1 EQ FALSE))
 IF ((SEND_PC EQ CHECK) OR (SEND_PC EQ TRUE))
  IF (SEND_PC EQ CHECK)
	test	edx, (FLAG_PC)
	je		SHORT VcNoPerspective
  ENDIF
  	mulps	xmmTxC, xmmVc									; (TS2*SCALE2+OFFSET2)*w (TS1*SCALE1+OFFSET1)*w
VcNoPerspective:
 ENDIF

 IF (SEND_ALPHA EQ CHECK)
	test	edx, (FLAG_ALPHA)
	jne		SHORT VcAlpha_RHW
 ENDIF

 IF ((SEND_ALPHA EQ CHECK) OR (SEND_ALPHA EQ FALSE))
;--- no alpha
VcNoAlpha_RHW:
  IF ((SEND_W EQ CHECK) OR (SEND_W EQ TRUE))
   IF (SEND_W EQ CHECK)
	test	edx, (FLAG_W)
	je		SHORT VcNoAlpha_NoRHW
   ENDIF
	SET4XMM	xmmVc, CMD_FIFO								; SEND w (RHW)
VcNoAlpha_NoRHW:
  ENDIF

  IF ((SEND_TX0 EQ CHECK) OR (SEND_TX0 EQ TRUE))
   IF (SEND_TX0 EQ CHECK)
	test	edx, (FLAG_TX0)
	je		SHORT VcNoAlpha_NoTexture0
   ENDIF
	SET8XMM_L	xmmTxC, CMD_FIFO							; SEND first texture
VcNoAlpha_NoTexture0:
  ENDIF

  IF ((SEND_TX1 EQ CHECK) OR (SEND_TX1 EQ TRUE))
   IF ((SEND_TX1 EQ CHECK) OR (SEND_ALPHA EQ CHECK))
	test	edx, (FLAG_TX1)
	je		SHORT NextTriangle
   ENDIF
	SET8XMM_H	xmmTxC, CMD_FIFO							; SEND second texture
VcNoAlpha_NoTexture1:
  ENDIF
  IF (SEND_ALPHA EQ CHECK)
	jmp		SHORT NextTriangle
  ENDIF
 ENDIF ;NO ALPHA

 IF ((SEND_ALPHA EQ CHECK) OR (SEND_ALPHA EQ TRUE))
;--- has alpha
VcAlpha_RHW:
  IF ((SEND_W EQ CHECK) OR (SEND_W EQ TRUE))
   IF (SEND_W EQ CHECK)
	test	edx, (FLAG_W)
	je		SHORT VcAlpha_NoRHW
   ENDIF
	SET4XMM	xmmVc, BOTH_DESTS								; SEND w (RHW)
VcAlpha_NoRHW:
  ENDIF

  IF ((SEND_TX0 EQ CHECK) OR (SEND_TX0 EQ TRUE))
   IF (SEND_TX0 EQ CHECK)
	test	edx, (FLAG_TX0)
	je		SHORT VcAlpha_NoTexture0
   ENDIF
	SET8XMM_L	xmmTxC, BOTH_DESTS							; SEND first texture
VcAlpha_NoTexture0:
  ENDIF

  IF ((SEND_TX1 EQ CHECK) OR (SEND_TX1 EQ TRUE))
   IF (SEND_TX1 EQ CHECK)
	test	edx, (FLAG_TX1)
	je		SHORT NextTriangle
   ENDIF
	SET8XMM_H	xmmTxC, BOTH_DESTS							; SEND second texture
VcAlpha_NoTexture1:
  ENDIF

 ENDIF ;SEND_2ND_SPEC_PASS
ENDIF ;No Textures


;******************************************************************
; Next Triangle
;******************************************************************
NextTriangle:
	; update last
	mov		dword ptr _last[esp+STACK_OFFSET], BIT_22 ; any full triangle must start out as FAN (HW requirement)

IF (FORCE_SPLIT_LOWER_XMM_QWORD_WRITE OR (CHECK_SPLIT_LOWER_XMM_QWORD_WRITE AND ALIGN_QWORD_HWPTR_WRITES))
  IF (HWPTR_DWORD_OFFSET MOD 8)
	SET4IMM	NOP_PACKET, CMD_FIFO							; send a nop
  ENDIF
ENDIF

IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ TRUE))
	movd 	mmxT2, edx										; _localFlags
ELSE
	pxor	mmxT2, mmxT2									; clear the specular flag
ENDIF

IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ TRUE))
  IF SEND_2ND_SPEC_PASS
	pcmpeqd	mmxT1, mmxT1  									; last tri in spec buffer
  ELSE
	pxor	mmxT1, mmxT1  									; last tri not in spec buffer
  ENDIF
ENDIF

	mov		edx, dword ptr _triCount[esp+4]					; _triCount
ENDM ;SEND_FULL_TRIANGLE



;######################################################################################################
;######################################################################################################
SEND_PARTIAL_TRIANGLE	MACRO
LOCAL NoColor, NoWBuffer, NoWBuffer_CheckZ, NoWbuffer_CheckWfbi, NoWbuffer_Wfbi_NoVFog
LOCAL HasWBuffer, HasWbuffer_CheckZ, HasWbuffer_CheckWfbi, PersCorrect, NoPerspective
LOCAL NoAlpha_RHW, NoAlpha_NoRHW, NoAlpha_NoTexture0, NoAlpha_NoTexture1
LOCAL   Alpha_RHW,   Alpha_NoRHW,   Alpha_NoTexture0,   Alpha_NoTexture1, NextTriangle

HWPTR_DWORD_OFFSET = 0
SBUFF_DWORD_OFFSET = 0

	mov		VaPtr, dword ptr _localFlags[esp+STACK_OFFSET]	; let's use VaPtr for the flags

	; scale z (and w)
ifdef DCT_FIX
	mulps	xmmVa, xmmVb									; --- --- vA.rhw*SCALE_W vA.z*_SCALEZ
else
	mulss	xmmVa, xmmVb									; --- --- ------ vA.z*_SCALEZ
endif

IFE ((SEND_TX0 EQ FALSE) AND (SEND_TX1 EQ FALSE))
	; add texture offsets
	addps	xmmTxC, xmmTxB									; (TxC [t1's1't0's0]*scale)+offset
ENDIF ;No Textures

; this is for some really bad rendering types only -> be very carefull about using it
IF ((START_BOUNDARY EQ START_ODD) AND CHECK_SPLIT_LOWER_XMM_QWORD_WRITE AND ALIGN_QWORD_HWPTR_WRITES)
	SET4IMM	NOP_PACKET, CMD_FIFO							; send a nop to the cmd fifo only 
ENDIF

; Send Triangle Start Command
	SET4MMX	mmxCa, CMD_FIFO									; send CMD_START
	SET4MMX	mmxSa, SPEC_BUFFER								; send CMD_START

; Send X and Y
	SET8XMM_L xmmVc, BOTH_DESTS								; SEND vC.x & vC.y

;--- Send color
IF ((SEND_COLOR EQ CHECK) OR (SEND_COLOR EQ TRUE))
  IF (SEND_COLOR EQ CHECK)
	test	VaPtr, (FLAG_COLOR)
	je		SHORT NoColor
  ENDIF
	SET4MMX	mmxCcb, CMD_FIFO								; SEND Color
	SET4MMX	mmxScb, SPEC_BUFFER								; SEND Specular
NoColor:
ENDIF

; Z-FOG, W-FOG, and RHW Implementation
IF (SEND_WBUFFER EQ CHECK)
	test	VaPtr, (FLAG_WBUFFER)
	jne		HasWBuffer
ENDIF

IF ((SEND_WBUFFER EQ CHECK) OR (SEND_WBUFFER EQ FALSE))
NoWBuffer:
NoWBuffer_CheckZ:
  IF ((SEND_Z EQ CHECK) OR (SEND_Z EQ TRUE))
   IF (SEND_Z EQ CHECK)
	test	VaPtr, (FLAG_Z)
	je		SHORT NoWbuffer_CheckWfbi
   ENDIF
	SET4XMM	xmmVa, BOTH_DESTS								; SEND z*_SCALEZ
  ENDIF

NoWbuffer_CheckWfbi:
	shufps	xmmVa, xmmVa, 055h								;  w  w  w  w
  IF ((SEND_WFBI EQ CHECK) OR (SEND_WFBI EQ TRUE))
   IF ((SEND_WFBI EQ CHECK) OR (SEND_WBUFFER EQ CHECK))
	test	VaPtr, (FLAG_WFBI)
	je		PersCorrect
   ENDIF
   IF ((SEND_VFOG EQ CHECK) OR (SEND_VFOG EQ TRUE))
    IF (SEND_VFOG EQ CHECK)
   	test	VaPtr, (FLAG_VFOG)
	je		SHORT NoWbuffer_Wfbi_NoVFog
    ENDIF
	mov		edx, dword ptr [__asm_data + SPECULAR_OFFSET]
	movzx	edx, byte  ptr [edx + VcPtr + 3]				; vC.specular.alpha
	mov		edx, dword ptr [__asm_rev_lookup + edx*4]		; LUT = (float)(255 - vC.specular.alpha)
	SET4GPR	edx, BOTH_DESTS									; SEND (float)(255 - vC.specular.alpha)
   ENDIF ;VFOG
   IF ((SEND_VFOG EQ CHECK) OR (SEND_WFBI EQ CHECK) OR (SEND_Z EQ CHECK) OR (SEND_WBUFFER EQ CHECK))
	jmp		PersCorrect
   ENDIF

   IF ((SEND_VFOG EQ CHECK) OR (SEND_VFOG EQ FALSE))
NoWbuffer_Wfbi_NoVFog:
ifdef DCT_FIX
	mov		edx, dword ptr [VcPtr+12]						; vC.rhw
	SET4GPR edx, BOTH_DESTS									; SEND rhw
else
	SET4XMM	xmmVa, BOTH_DESTS								; SEND rhw
endif
   ENDIF ;NO VFOG
   IF ((SEND_VFOG EQ CHECK) OR (SEND_WFBI EQ CHECK) OR (SEND_Z EQ CHECK) OR (SEND_WBUFFER EQ CHECK))
	jmp		PersCorrect
   ENDIF
  ENDIF ;WFBI
ENDIF ;WBUFFER

IF ((SEND_WBUFFER EQ CHECK) OR (SEND_WBUFFER EQ TRUE))
HasWBuffer:
	shufps	xmmVa, xmmVa, 055h								;  w  w  w  w
HasWbuffer_CheckZ:
  IF ((SEND_Z EQ CHECK) OR (SEND_Z EQ TRUE))
   IF (SEND_Z EQ CHECK)
	test	VaPtr, (FLAG_Z)
	je		SHORT HasWbuffer_CheckWfbi
   ENDIF
	mov		edx, dword ptr [__asm_data + SPECULAR_OFFSET]
	movzx	edx, byte  ptr [edx + VcPtr + 3]				; vA.specular.alpha
	mov		edx, dword ptr [__asm_rev_lookup + edx*4]		; LUT = (float)(255 - vC.specular.alpha)
	; multiply by 256 by adding 8 to the exponent
	add		edx, (8 SHL 23)									; (float)(255 - vC.specular.alpha) * 256.0
	SET4GPR	edx, BOTH_DESTS									; SEND (float)(255 - vC.specular.alpha) * 256.0
  ENDIF ;Z

HasWbuffer_CheckWfbi:
  IF ((SEND_WFBI EQ CHECK) OR (SEND_WBUFFER EQ TRUE))
   IF (SEND_WFBI EQ CHECK)
	test	VaPtr, (FLAG_WFBI)
	je		SHORT PersCorrect
   ENDIF
ifdef DCT_FIX
	SET4XMM	xmmVa, BOTH_DESTS								; SEND w
else
	mulss	xmmVa, dword ptr [__asm_data + WBUFFER_SCALE_AW]; rhw rhw rhw rhw*WBUFFER_SCALE_AW
	addss	xmmVa, dword ptr [__asm_data + WBUFFER_SCALE_BW]; rhw rhw rhw rhw*WBUFFER_SCALE_AW+WBUFFER_SCALE_BW
	SET4XMM	xmmVa, BOTH_DESTS								; rhw rhw rhw SEND rhw*WBUFFER_SCALE_AW+WBUFFER_SCALE_BW
	movhlps	xmmVa, xmmVa									; rhw rhw rhw rhw
endif
  ENDIF ;WFBI
ENDIF ;WBUFFER

; RHW and Multitexture Implementation
PersCorrect:
IFE ((SEND_TX0 EQ FALSE) AND (SEND_TX1 EQ FALSE))
 IF ((SEND_PC EQ CHECK) OR (SEND_PC EQ TRUE))
  IF (SEND_PC EQ CHECK)
	test	VaPtr, (FLAG_PC)
	je		SHORT NoPerspective
  ENDIF
	mulps	xmmTxC, xmmVa									; (TS2*SCALE2+OFFSET2)*w (TS1*SCALE1+OFFSET1)*w
NoPerspective:
 ENDIF

 IF (SEND_ALPHA EQ CHECK)
	test	VaPtr, (FLAG_ALPHA)
	jne		SHORT Alpha_RHW
 ENDIF

 IF ((SEND_ALPHA EQ CHECK) OR (SEND_ALPHA EQ FALSE))
;--- no alpha
NoAlpha_RHW:
  IF ((SEND_W EQ CHECK) OR (SEND_W EQ TRUE))
   IF (SEND_W EQ CHECK)
	test	VaPtr, (FLAG_W)
	je		SHORT NoAlpha_NoRHW
   ENDIF
	SET4XMM	xmmVa, CMD_FIFO									; SEND w (RHW)
NoAlpha_NoRHW:
  ENDIF

  IF ((SEND_TX0 EQ CHECK) OR (SEND_TX0 EQ TRUE))
   IF (SEND_TX0 EQ CHECK)
	test	VaPtr, (FLAG_TX0)
	je		SHORT NoAlpha_NoTexture0
   ENDIF
	SET8XMM_L	xmmTxC, CMD_FIFO							; SEND first texture
NoAlpha_NoTexture0:
  ENDIF

  IF ((SEND_TX1 EQ CHECK) OR (SEND_TX1 EQ TRUE))
   IF ((SEND_TX1 EQ CHECK) OR (SEND_ALPHA EQ CHECK))
	test	VaPtr, (FLAG_TX1)
	je		SHORT NextTriangle
   ENDIF
	SET8XMM_H	xmmTxC, CMD_FIFO							; SEND second texture
NoAlpha_NoTexture1:
  ENDIF
  IF (SEND_ALPHA EQ CHECK)
	jmp		SHORT NextTriangle
  ENDIF
 ENDIF ;NO ALPHA

 IF ((SEND_ALPHA EQ CHECK) OR (SEND_ALPHA EQ TRUE))
;--- has alpha
Alpha_RHW:
  IF ((SEND_W EQ CHECK) OR (SEND_W EQ TRUE))
   IF (SEND_W EQ CHECK)
	test	VaPtr, (FLAG_W)
	je		SHORT Alpha_NoRHW
   ENDIF
	SET4XMM	xmmVa, BOTH_DESTS								; SEND w (RHW)
Alpha_NoRHW:
  ENDIF

  IF ((SEND_TX0 EQ CHECK) OR (SEND_TX0 EQ TRUE))
   IF (SEND_TX0 EQ CHECK)
	test	VaPtr, (FLAG_TX0)
	je		SHORT Alpha_NoTexture0
   ENDIF
	SET8XMM_L	xmmTxC, BOTH_DESTS							; SEND first texture
Alpha_NoTexture0:
  ENDIF

  IF ((SEND_TX1 EQ CHECK) OR (SEND_TX1 EQ TRUE))
   IF (SEND_TX1 EQ CHECK)
	test	VaPtr, (FLAG_TX1)
	je		SHORT NextTriangle
   ENDIF
	SET8XMM_H	xmmTxC, BOTH_DESTS							; SEND second texture
Alpha_NoTexture1:
  ENDIF
 ENDIF ;SEND_2ND_SPEC_PASS
ENDIF ;No Textures


;******************************************************************
; Next Triangle
;******************************************************************
NextTriangle:
	; update last triangle type
	movd	dword ptr _last[esp+STACK_OFFSET], mmxT2		; FAN | STRIP -> update last triangle type

IF (FORCE_SPLIT_LOWER_XMM_QWORD_WRITE OR (CHECK_SPLIT_LOWER_XMM_QWORD_WRITE AND ALIGN_QWORD_HWPTR_WRITES))
  IF (HWPTR_DWORD_OFFSET MOD 8)
	SET4IMM	NOP_PACKET, CMD_FIFO  							; send a nop
  ENDIF
ENDIF

IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ TRUE))
  IF SEND_2ND_SPEC_PASS
	pcmpeqd	mmxT1, mmxT1  									; last tri in spec buffer
  ELSE
	pxor	mmxT1, mmxT1  									; last tri not in spec buffer
  ENDIF
ENDIF

IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ TRUE))
	movd 	mmxT2, VaPtr									; _localFlags
ELSE
	pxor	mmxT2, mmxT2									; clear the specular flag
ENDIF

	mov		edx, dword ptr _triCount[esp+STACK_OFFSET]		; _triCount
ENDM ;SEND_PARTIAL_TRIANGLE

;***********************************************************************************************************************
;***********************************************************************************************************************
;                             
;                                 Setup
;                                   |
;    -------------------------> Last Tri?                          
;   /                          /         \                         
;  |                          N           Y                        
;  |                         /             \                       
;  |               Backface Cull?      Spec Buffer Empty?
;  +                 /       \            /          \  
;  |\               Y         N          Y            N            
;  | ---------------           \          \            \
;  |                    Autostrippable?    \        Copy to HW           
;  |                   /               \    \           |
;  |                  Y                 N    ----> Done (return)
;  |                 /                   \     
;  |         Specular?                    Specular?           
;  |         /       \                    /       \           
;  |        Y         N                  Y         N          
;  |       /           \                /           \         
;  |     Render       Render          Render       Render	  
;  |    Partial       Partial          Full         Full
;  |    Specular     Non-Spec        Specular     Non-Spec
;  |    Triangle     Triangle        Triangle     Triangle
;   \     /		      /               /		 	   /
;    ----+-----------+---------------+-------------
;***********************************************************************************************************************
;***********************************************************************************************************************
;********************************************************************************
; Core rendering routine
; supports x,y,z,rhw,color,s,t
; requires rhw
; color,s,t are unnecessary but sent to hardware
;
; On entry to this routine:
;		ebp			= post_idx1, pointer to triangle index
;********************************************************************************
DRAW_TRIANGLES	MACRO
LOCAL HwPtrIsAligned, NextTriangle_LastTriNotRendered, NextTriangle_LastTriRendered
LOCAL AutostripCheck, AutostripCheck_NonSpecular, AutostripCheck_Specular
LOCAL FullTriangle, FullTriangle_AddAlphaToSpecular, FullTriangle_NonSpecular, FullTriangle_Specular
LOCAL PartialTriangle, PartialTriangle_AddAlphaToSpecular, PartialTriangle_NonSpecular, PartialTriangle_Specular
LOCAL SendSecondTrianglePass, DoneRender
LOCAL SecondPassSetup_NoFogEnable, SecondPassSetup_NoAlphaBlendEnable, SecondPassSetup_AlphaBlendEnableDone, SecondPassSetup_NoZEnable, SecondPassSetup_IsNapalm, SecondPassSetup_NotNapalm, SecondPassSetup_SendCombineMode
LOCAL SecondPass_HwPtrAlignDone, SecondPass_NextBlockMove, SecondPass_BlockMoveDone
LOCAL SecondPassRestore_IsNapalm, SecondPassRestore_NotNapalm, SecondPassRestore_SendCombineMode, SecondPassRestore_NoFogEnable

VaPtr		TEXTEQU	<eax>		; *vertex A
VbPtr		TEXTEQU	<ebx>		; *vertex B
VcPtr		TEXTEQU	<ecx>		; *vertex C
spPtr		TEXTEQU	<esi>		; pointer to specular buffer
hwPtr		TEXTEQU	<edi>		; pointer to command fifo
vertPtr		TEXTEQU	<ebp>		; pointer to vertex list

mmxVac		TEXTEQU	<mm0>		; mmx VertPtr	vA	vC (for autostripping)
mmxVcb		TEXTEQU	<mm1>		; mmx VertPtr	vC  vB (for autostripping)
mmxCa		TEXTEQU	<mm2>		; mmx Color		--	vA	(shared)
mmxT3		TEXTEQU	<mm2>		; mmx temp 3			(shared)
mmxCcb		TEXTEQU	<mm3>		; mmx Color		vC	vB	(shared)
mmxT4		TEXTEQU	<mm3>		; mmx temp 4			(shared)
mmxSa		TEXTEQU	<mm4>		; mmx Spec		--	vA	(shared)
mmxT5		TEXTEQU	<mm4>		; mmx temp 5			(shared)
mmxScb		TEXTEQU	<mm5>		; mmx Spec		vC	vB	(shared)
mmxT6		TEXTEQU	<mm5>		; mmx temp 6			(shared)
mmxT1		TEXTEQU	<mm6>		; mmx temp 1
mmxT2		TEXTEQU	<mm7>		; mmx temp 2			(shared)
mmxWfbicb	TEXTEQU	<mm7>		; mmx Wfbi		vC	vB	(shared)

xmmVa		TEXTEQU	<xmm0>		; xmm vertex A xyzw
xmmVb		TEXTEQU	<xmm1>		; xmm vertex B xyzw
xmmVbc		TEXTEQU	<xmm2>		; xmm vertex B-C x&y	(shared)
xmmVc		TEXTEQU	<xmm2>		; xmm vertex C xyzw		(shared)
xmmTxA		TEXTEQU	<xmm3>		; xmm vertex A Texture
xmmTxB		TEXTEQU	<xmm4>		; xmm vertex B Texture
xmmTxC		TEXTEQU	<xmm5>		; xmm vertex C Texture
xmmT1		TEXTEQU	<xmm6>		; xmm temp 1
xmmT2		TEXTEQU	<xmm7>		; xmm temp 2

STACK_OFFSET = 4				; 0 in the prefetch routines, 4 in the main routine

; prefetch the 2nd triangle, the first should already prefetched
	mov		VaPtr, dword ptr [vertPtr+12+0] 	; *vA[curr+1]
	mov		VbPtr, dword ptr [vertPtr+12+4] 	; *vB[curr+1]
	mov		VcPtr, dword ptr [vertPtr+12+8] 	; *vC[curr+1]
	prefetchnta [VaPtr]
	prefetchnta [VbPtr]
	prefetchnta [VcPtr]

; In case there's no second texture, clear the upper half of xmmTxA and xmmTxB (so we don't get denormals)
	xorps	xmmTxA, xmmTxA
	movlhps	xmmTxa, xmmTxB

; preload some stuff we'll only load once
	mov		hwPtr, dword ptr[__asm_data + _FIFO_PTR]
	mov		spPtr, dword ptr _specPtr[esp+STACK_OFFSET]
	
IF (SEND_SPEC EQ CHECK)
	movd 	mmxT2, dword ptr _localFlags[esp+STACK_OFFSET]	; need the specular flag
ELSE
	pxor	mmxT2, mmxT2									; clear the specular flag
ENDIF

IF (FORCE_SPLIT_LOWER_XMM_QWORD_WRITE OR (CHECK_SPLIT_LOWER_XMM_QWORD_WRITE AND ALIGN_QWORD_HWPTR_WRITES))
  	test	hwPtr, 4
	je		HwPtrIsAligned
	SET4IMM	0, CMD_FIFO										; send a nop
HwPtrIsAligned:
ENDIF


align 16
;********************************************************************************************************
;********************************************************************************************************
; Main Triangle Rendering Loop
;********************************************************************************************************
;********************************************************************************************************
NextTriangle_LastTriNotRendered:
; prime the autostripping by zeroing the previous tri's vertex pointers
; this works because a valid pointer will never be null
	mov		edx, dword ptr _triCount[esp+STACK_OFFSET]	; _triCount
	pxor	mmxVac, mmxVac								; clear prev tri's vertex pointers so we don't autostrip
IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ TRUE))
	pxor 	mmxT1, mmxT1								; last tri not in spec buffer
ENDIF

align 16
NextTriangle_LastTriRendered:
	dec		edx											; --_triCount
	js		SendSecondTrianglePass

;	movd 	mmxT2, dword ptr _localFlags[esp+STACK_OFFSET] ; updated in the rendering routines

;********************************************************************************
; Backface Cull Check and Autostrip Detection
;
;	if (sign ((vA.x-vB.x)*(vB.y-vC.y) - (vA.y-vB.y)*(vB.x-vC.x)))
;		cull the triangle
;	else if ((prevC == currA) && (prevB == currB)	
;		triangle strip
;	else if ((prevA == currA) && (prevC == currB)
;		triangle fan
;								 
;	if ((prevC == currA) && (prevB == currB)	STRIP	(BIT23)
;	if ((prevA == currA) && (prevC == currB)	FAN		(BIT22)
;
;   Disable autostripping if
;		1) Flat shaded triangles (already done by clearing strip/fan mask)
;		2) Any wrap bit is set (already done by clearing strip/fan mask)
;		3) The triangle needs a second pass and the previous triangle 
;		   was not sent to the second pass buffer (specular buffer)
;		4) The previous triangle was backfaced culled
;
; Notes:
;	Load the current triangle's vertex pointers into the  
;		mmx registers, which at the end of the vertex loop 
;		will be the previous triangle. (used for autostripping)
;	Sneak in prefetching of the second triangle after this one
;********************************************************************************
	mov		VaPtr, dword ptr [vertPtr+0+0] 		; *vA[curr+0]
	mov		VbPtr, dword ptr [vertPtr+0+4] 		; *vB[curr+0]
	mov		VcPtr, dword ptr [vertPtr+0+8] 		; *vC[curr+0]

IF (SEND_SPEC EQ CHECK)
	pslld	mmxT2, 31-BIT_SPEC					; bit 31 = specular flag								(for autostrip)
	psrld	mmxT2, 31							; bit  0 = specular flag, all others = 0				(for autostrip)
	pxor	mmxT3, mmxT3						; 0					0
	pcmpgtd mmxT2, mmxT3						; -					specular ? -1 : 0					(for autostrip)
	punpckldq mmxT2, mmxT2						; specular ? -1 : 0	specular ? -1 : 0					(for autostrip)
	psrld	mmxT2, 8							; specular ? 00ffffff : 0	; alpha mask				(for autostrip)
	; mmxT2 = alpha mask (zero if no specular)
ELSEIF (SEND_SPEC EQ TRUE)
	pcmpeqd	mmxT2, mmxT2						; -1				-1
	psrld	mmxT2, 8							; specular ? 00ffffff : 0	; alpha mask				(for autostrip)
ELSEIF (SEND_SPEC EQ FALSE)
	pxor	mmxT2, mmxT2						; 0					0
ENDIF

	movhps	xmmT1, oword ptr [VaPtr]			; vA.y vA.x ---- ----
	movd	mmxT4, VaPtr						; 0					curr.vA								(for autostrip)

	movlps	xmmT1, oword ptr [VbPtr]			; vA.y vA.x vB.y vB.x
	movd	mmxT5, VbPtr						; 0					curr.vB								(for autostrip)

	movlps	xmmVbc, oword ptr [VcPtr]			; ---- ---- vC.y vC.x
	movd	mmxT6, VcPtr						; 0					curr.vC (temporary copy)

	movq	mmxT3, qword ptr [StipCont]			; FANMASK			STRIPMASK (zero if flatshade)		(for autostrip)
	movhlps	xmmVa, xmmT1						; ---- ---- vA.y vA.x
	punpckldq mmxT4, mmxT4						; curr.vA			curr.vA								(for autostrip)

	mov		VaPtr, dword ptr [vertPtr+(NUM_PAD_TRIS*12)+0] 	; *vA[curr+n]
	movlhps xmmVbc, xmmT1						; vB.y vB.x vC.y vC.x
	punpckldq mmxT5, mmxT5						; curr.vB			curr.vB								(for autostrip)

	mov		VbPtr, dword ptr [vertPtr+(NUM_PAD_TRIS*12)+4] 	; *vB[curr+n]
	subps	xmmT1, xmmVbc						; vA.y-vB.y vA.x-vB.x vB.y-vC.y vB.x-vC.x
	pcmpeqd	mmxVac, mmxT4						; curr.vA==prev.vA ? -1 : 0		curr.vA==prev.vC ? -1 : 0	(for autostrip)

; subps 1
	mov		VcPtr, dword ptr [vertPtr+(NUM_PAD_TRIS*12)+8] 	; *vC[curr+n]
	pcmpeqd	mmxVcb, mmxT5						; curr.vB==prev.vC ? -1 : 0		curr.vB==prev.vB ? -1 : 0	(for autostrip)

; subps 2
	prefetchnta [VaPtr]							; prefetch vA
	movhlps	xmmT2, xmmT1						; ---- ---- vA.y-vB.y vA.x-vB.x
	pand	mmxT3, mmxVac						; curr.vA==prev.vA ? FAN:0	curr.vA==prev.vC ?  STRIP:0	(for autostrip)

	prefetchnta [VbPtr]							; prefetch vB
	shufps	xmmT1, xmmT1, 0e1h					; ---- ---- vB.x-vC.x vB.y-vC.y
	pand	mmxT3, mmxVcb						; curr.vA==prev.vA & curr.vB==prev.vC ? STRIP : 0 (H)	(for autostrip)
												; curr.vA==prev.vC & curr.vB==prev.vB ?  FAN  : 0 (L)	(for autostrip)	

	prefetchnta [VcPtr]							; prefetch vC
	mulps	xmmT2, xmmT1						; ---- ---- (vA.y-vB.y)*(vB.x-vC.x) (vA.x-vB.x)*(vB.y-vC.y)
	movq	mmxVac, mmxT6						; 0					curr.vC								(for autostrip)

;mulps 1
	mov		VbPtr, dword ptr [__asm_data +_CULL_MASK]
	movq	mmxVcb, mmxT5						; curr.vB			curr.vB								(for autostrip)
	punpckldq mmxVac, mmxT4						; curr.vA			curr.vC								(for autostrip)

;mulps 2
	movd	VaPtr, mmxT4						; curr.vA	restore VaPtr
IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ TRUE))
	pxor	mmxT6, mmxT6						; 0					0									(for autostrip)
ENDIF
IF ((SEND_TX0 EQ TRUE) AND (SEND_TX1 EQ TRUE))
	prefetchnta [VcPtr+32]						; prefetch upper half of vertex C if we have 2 textures
ENDIF

;mulps 3
	movss	xmmT1, xmmT2						; ---- ---- ---- (vA.x-vB.x)*(vB.y-vC.y)
	movq	mmxT4, mmxT3						; FAN				STRIP								(for autostrip)
IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ TRUE))
	movd	mmxT5, spPtr						;														(for autostrip)
ENDIF

	shufps	xmmT2, xmmT2, 0e1h					; ---- ---- ---- (vA.y-vB.y)*(vB.x-vC.x)
	punpckhdq mmxT3, mmxT3						; FAN				FAN									(for autostrip)
	pcmpeqd	mmxT5, mmxT6						; -1				!spPtr ? -1 : 0						(for autostrip)

	subss	xmmT1, xmmT2						; ---- ---- ---- (vA.x-vB.x)*(vB.y-vC.y)-(vA.y-vB.y)*(vB.x-vC.x)
	por		mmxT3, mmxT4						; ---				STRIP | FAN							(for autostrip)
	; mmxT3 = mmxStripFan
	mov		dword ptr _triCount[esp+STACK_OFFSET], edx ; _triCount
	punpckldq mmxVcb, mmxVac					; curr.vC			curr.vB								(for autostrip)
	; mmxVac and mmxVcb are restored

; subss 1
	add		vertPtr, 12
IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ TRUE))
	por		mmxT1, mmxT5						; 0			(_lastTriInSpecBuffer || !spPtr) ? -1 : 0	(for autostrip)
	; mmxT1 = (_lastTriInSpecBuffer || !spPtr) ? -1 : 0
ELSE
	pcmpeqd mmxT1, mmxT1						; -1, -1
ENDIF

	movmskps VcPtr, xmmT1						; 00000000b 00000000b 00000000b 0000xxxSb
	shl		VcPtr, 31							; sign((vA.x-vB.x)*(vB.y-vC.y) - (vA.y-vB.y)*(vB.x-vC.x))
	punpckldq mmxT1, mmxT3						; STRIP | FAN		(_lastTriInSpecBuffer || !spPtr) ? -1 : 0
	xor		VbPtr, VcPtr						; sign ^ _CULL_MASK
	jz		NextTriangle_LastTriNotRendered		; cull this triangle

	movd 	VcPtr, mmxT3						; STRIP | FAN
IF (SEND_SPEC EQ CHECK)
AutostripCheck:
	movd	edx, mmxT2							; if mmxT2 = 0, no specular  (leave edx with this if non-spec!)
	cmp		edx, 0
	jne		SHORT AutostripCheck_Specular
ENDIF

IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ FALSE))
AutostripCheck_NonSpecular:
	test	VcPtr, VcPtr						; strip/fan if we can autostrip
	movd	VbPtr, mmxVcb						; curr.vB	restore VbPtr
	movd	VcPtr, mmxVac						; curr.vC	restore VbPtr
	je		FullTriangle
  IFE (SEND_WRAP EQ TRUE) ; if wrap we can't autostrip
	movq	mmxT2, mmxT3						; STRIP | FAN
	pxor	mmxT3, mmxT3						; tells partial no spec
	jmp		PartialTriangle
  ENDIF
ENDIF

IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ TRUE))
AutostripCheck_Specular:
	movd	mmxSa, dword ptr [VaPtr+spec]		; 0					vA.spec
	movd	VbPtr, mmxVcb						; curr.vB	restore VbPtr
	movd	VcPtr, mmxVac						; curr.vC	restore VbPtr

;**********************************************************
; if !STATE_REQUIRES_SPECULAR
;   color = diff
; else
;   color = diff + (spec & 00ffffff) (clamped add)
;   if TEXTURE('s) && (spec a|b|c)
;     2nd pass
;**********************************************************
	movd	mmxT4, dword ptr [VbPtr+spec]		; 0					vB.spec
	pand	mmxSa, mmxT2						; 0					spec ? vA.spec.rgb : 0			<- mmxSa is done
	punpckldq mmxT4, qword ptr [VcPtr+spec]		; vC.spec			vB.spec

	pand	mmxT4, mmxT2						; spec ? vC : 0		spec ? vB.spec.rgb : 0
	movq	mmxT3, mmxSa						; 0					spec ? vA.spec.rgb : 0
	movq	mmxScb, mmxT4						; spec ? vC.spec	spec ? vC.spec.rgb : 0			<- mmxScb is done
  IFE (SEND_WRAP EQ TRUE) ; if wrap we can't autostrip
	por		mmxT3, mmxT4						; spec.vc			spec ? (vA|vB.spec.rgb) : 0
	punpckhdq mmxT4, mmxT4						; -------			spec.vc
	por		mmxT3, mmxT4						; -------			spec ? (vA|vB|vC.spec.rgb) : 0
	pxor	mmxT2, mmxT2						; 0					0
	pcmpeqd	mmxT2, mmxT3						; -------			(!spec || (vA|vB|vC.spec.rgb==0)) : -1 : 0
	por		mmxT2, mmxT1						; ---  (_lastTriInSpecBuffer || !spPtr || !spec || (vA|vB|vC.spec.rgb==0)) : -1 : 0
	punpckhdq mmxT1, mmxT1						; STRIP | FAN		STRIP | FAN
	pand	mmxT2, mmxT1						; (STRIP | FAN) && (_lastTriInSpecBuffer || !spPtr || !spec || (vA|vB|vC.spec.rgb==0)) : -1 : 0
	movd	edx, mmxT2
	test	edx, edx							; edx = strip/fan if we can autostrip
	jne		PartialTriangle
	movd	edx, mmxT3							; spec ? (vA|vB|vC.spec.rgb) : 0  <- mmxT3 = mmxCa
  ENDIF ;wrap
ENDIF


;********************************************************************************
; Send a full triangle (not a strip or fan)
;********************************************************************************
FullTriangle:
	movd	mmxCcb, dword ptr [VbPtr+color]		; 0				vB.diff
	pcmpeqd mmxT2, mmxT2						; -1			-1
	punpckldq mmxCcb, qword ptr [VcPtr+color]	; vC.diff		vB.diff
	pslld	mmxT2, 24							; ff000000		ff000000
	movd	mmxCa, dword ptr [VaPtr+color]		; 0				vA.diff
	movq	mmxT1, mmxT2						; ff000000		ff000000

IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ TRUE))
	cmp		edx,0
	je		FullTriangle_NonSpecular

  IF (((SEND_TX0 EQ FALSE) AND (SEND_TX1 EQ FALSE)) OR ((SEND_TX0 EQ CHECK) AND (SEND_TX1 EQ CHECK)))
   IF ((SEND_SPEC EQ CHECK) OR (SEND_TX0 EQ CHECK) OR (SEND_TX1 EQ CHECK))
	cmp		spPtr, 0							; if 2nd pass don't add specular to diffuse
	jne		FullTriangle_AddAlphaToSpecular
   ENDIF
	; Add the speculars to the diffused colors IF we don't send a second pass
	paddusb	mmxCcb, mmxScb						; vC.diff+spec	vB.diff+spec
	paddusb	mmxCa, mmxSa						; 0				vA.diff+spec

FullTriangle_AddAlphaToSpecular:
  ENDIF ; no textures

	; Now add the alpha from the diffuse to the specular
	pand	mmxT2, mmxCcb						; vC.diff.alpha	vB.diff.alpha
	pand	mmxT1, mmxCa						; 0				vA.diff.alpha
	por		mmxScb, mmxT2						; vC/vB.spec.rgb+diff.alpha
	por		mmxSa, mmxT1						; 0				vA.spec.rgb+diff.alpha

	; Check if this triangle uses specular textures
	cmp		spPtr, 0
	jne		FullTriangle_Specular
ENDIF

;********************************************************************************
;	Separate versions of render loop are done with macros
;	SEND_2ND_SPEC_PASS copies all commands to spec buffer
;********************************************************************************
FullTriangle_NonSpecular:
	SEND_2ND_SPEC_PASS = 0
	SEND_FULL_TRIANGLE
	jmp		NextTriangle_LastTriRendered

IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ TRUE))
align 16
FullTriangle_Specular:
	SEND_2ND_SPEC_PASS = 1
	SEND_FULL_TRIANGLE
	jmp		NextTriangle_LastTriRendered
ENDIF


IFE (SEND_WRAP EQ TRUE) ; if wrap we can't autostrip
;********************************************************************************
; Send partial triangle (strip or fan)
; VaPtr, VbPtr, mmxCa, mmxSa, xmmVa, xmmVb, xmmTxA, and xmTxB are now available
;********************************************************************************
align 16
PartialTriangle:
  IFE (SEND_TX0 EQ FALSE)
	mov		VaPtr, dword ptr [__asm_data + _T0_OFFSET]		; Texture 0 coordinate index
  ENDIF
	pcmpeqd	mmxT1, mmxT1									; -1			-1

  IFE (SEND_TX1 EQ FALSE)
	mov		VbPtr, dword ptr [__asm_data + T1_OFFSET]		; Texture 1 coordinate index
  ENDIF
	psrlq	mmxT1, 63										; 00000000		00000001

	movaps	xmmT1, oword ptr [PixelOffset]					; PIXEL_CENTER PIXEL_CENTER PIXEL_CENTER PIXEL_CENTER (barrowing Vb for now)
	pslld	mmxT1, 22										; 00000000		BIT_22
  IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ TRUE))
	movd	edx, mmxT3										; spec ? (vA|vB|vC.spec.rgb) : 0  <- mmxT3 = mmxCa
  ENDIF
	movd	mmxSa, dword ptr _last[esp+STACK_OFFSET]		; 0				_last

	movlps	xmmVa, qword ptr [VcPtr+8]						; --- --- vA.rhw vA.z
	movd	mmxCcb, dword ptr [VcPtr+color]					; 0				vC.diff

  IFE (SEND_TX0 EQ FALSE)
	movlps	xmmTxC, qword ptr [VcPtr+VaPtr]					; --- --- vC.t0 vC.s0
  ENDIF
	pcmpeqd mmxSa, mmxT2									; ((FAN | STRIP) == _last) ? -1 : 0

	movd	mmxCa, dword ptr [__asm_data + CMD_CONT]

  IFE (SEND_TX1 EQ FALSE)
	movhps	xmmTxC, qword ptr [VcPtr+VbPtr]					; vA.t1 vA.s1 vA.t0 vA.s0
  ENDIF
	pand	mmxT1, mmxSa									; ((FAN | STRIP) == _last) ? BIT_22 : 0 (L)

  IF ((SEND_TX1 EQ CHECK) OR (SEND_TX1 EQ TRUE))
	movaps	xmmTxB, oword ptr [TextureOffset]				; TxOffset [t1's1't0's0]
  ELSEIF ((SEND_TX0 EQ CHECK) OR (SEND_TX0 EQ TRUE))
	movlps	xmmTxB, qword ptr [TextureOffset]				; TxOffset [--'--'t0's0]
  ENDIF

  IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ TRUE))
	movd	mmxSa, dword ptr _specCmdCont[esp+STACK_OFFSET]	; specular command continure
  ENDIF

  IF ((SEND_TX1 EQ CHECK) OR (SEND_TX1 EQ TRUE))
	movaps	xmmTxA, oword ptr [TextureScale]				; TxScale [t1's1't0's0]
  ELSEIF ((SEND_TX0 EQ CHECK) OR (SEND_TX0 EQ TRUE))
	movlps	xmmTxA, qword ptr [TextureScale]				; TxScale [--'--'t0's0]
  ENDIF

  IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ TRUE))
	punpckhdq mmxScb, mmxScb								; vC.diff+spec	vC.diff+spec
  ENDIF

ifdef DCT_FIX
	movlps	xmmVb, qword ptr [ScaleZW+8]					; --- --- ScaleW ScaleZ
else
	movss	xmmVb, dword ptr [ScaleZW+8]					; --- --- ------ ScaleZ
endif
	por		mmxCa, mmxT1									; 1st pass command continue

	; add pixel center (0.5) to x&y
	addps	xmmVc, xmmT1									; --- --- vA.y+PIXEL_CENTER vA.x+PIXEL_CENTER
  IFE ((SEND_TX0 EQ FALSE) AND (SEND_TX1 EQ FALSE))
	; scale textures
	mulps	xmmTxC, xmmTxA									; TxA [t1's1't0's0]*scale
  ENDIF ;No Textures
	por		mmxSa, mmxT1									; 2nd pass command continue

  IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ TRUE))
	cmp		edx, 0											; edx still = vA|vB|vC.spec.rgb
	je		PartialTriangle_NonSpecular

   IF (((SEND_TX0 EQ FALSE) AND (SEND_TX1 EQ FALSE)) OR ((SEND_TX0 EQ CHECK) AND (SEND_TX1 EQ CHECK)))
    IF ((SEND_SPEC EQ CHECK) OR (SEND_TX0 EQ CHECK) OR (SEND_TX1 EQ CHECK))
	cmp		spPtr, 0										; if 2nd pass don't add specular to diffuse
	jne		PartialTriangle_AddAlphaToSpecular
    ENDIF
	; Add the speculars to the diffused colors
	paddusb	mmxCcb, mmxScb									; ------------	vC.diff+spec

PartialTriangle_AddAlphaToSpecular:
   ENDIF ; no textures

	; Now add the alpha from the diffuse to the specular
	pcmpeqd mmxT1, mmxT1									; -1			-1
	pslld	mmxT1, 24										; ff000000		ff000000
	pand	mmxT1, mmxCcb									; 0				vC.diff.alpha
	por		mmxScb, mmxT1									; 0				vC.spec.rgb+diff.alpha

	; Check if this triangle uses specular textures
	cmp		spPtr, 0
	jne		PartialTriangle_Specular
  ENDIF ;SPEC

;********************************************************************************
;	Separate versions (specular and non-specular) of the 
;	partial triangle render loops are done with macros
;********************************************************************************
PartialTriangle_NonSpecular:
	SEND_2ND_SPEC_PASS = 0
	SEND_PARTIAL_TRIANGLE
	jmp		NextTriangle_LastTriRendered

  IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ TRUE))
align 16
PartialTriangle_Specular:
	SEND_2ND_SPEC_PASS = 1
	SEND_PARTIAL_TRIANGLE
	jmp		NextTriangle_LastTriRendered
  ENDIF

ENDIF ;WRAP (negates autostripping)

;********************************************************************************************************
;********************************************************************************************************
; Do the second pass for the specular triangles with textures
;********************************************************************************************************
;********************************************************************************************************
align 16
SendSecondTrianglePass:
; If spPtr is zero we know specular triangles wasn't enabled.
; If spPtr = 32, then specular trianges was enabled but they all 
; had specular's = 0 so we don't have to do a second pass.
IF ((SEND_SPEC EQ CHECK) OR (SEND_SPEC EQ TRUE))
	sub		spPtr, 32
	jbe		DoneRender

	mov		ebx, dword ptr [__asm_data + _RC]			; ebx = *RC, don't hose it up!

comment $
        // Where FOG() = fog function
        // FOG(T1 + T2) = AlphaFog * FogColor + (1 - AlphaFog)[T1 +T2]
        //    Pass 1    = AlphaFog * FogColor + (1 - AlphaFog)T1
        //    Pass 2    =                       (1 - AlphaFog)T2
        if (pRc->fogEnable) {
          SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0xF ) );
          SETPD( cmdFifo, ghw->fogMode, (pRc->sst.fogMode & ~SST_FOGMULT) | SST_FOGADD);
        }
$
	cmp		dword ptr [ebx + OFFSET_RC_FOG_ENABLE], 0
	je		SHORT SecondPassSetup_NoFogEnable
	mov		eax, dword ptr [ebx + OFFSET_RC_FOG_MODE]
	and		eax, (NOT SST_FOGMULT)
	or		eax, SST_FOGADD
	SET4IMM	CMD_FOG_MODE, CMD_FIFO
	SET4GPR	eax, CMD_FIFO
SecondPassSetup_NoFogEnable:


comment $
        // let's just add in the color ignoring the effect on alpha blending for
        // the time being
        // (Src * 1 + Dst * 1)
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, alphaMode, 0xF ) );
$
	SET4IMM	CMD_ALPHA_SPEC_MODE, CMD_FIFO

comment $
        if (!pRc->alphaBlendEnable) {
            SETPD( cmdFifo, ghw->alphaMode, (SST_ENALPHABLEND | (SST_A_ONE << SST_RGBSRCFACT_SHIFT) | (SST_A_ONE << SST_RGBDSTFACT_SHIFT)) );
        } else {
            SETPD( cmdFifo, ghw->alphaMode, ((pRc->sst.alphaMode & ~SST_RGBDSTFACT) | (SST_A_ONE << SST_RGBDSTFACT_SHIFT)) );

        }
$
	cmp		dword ptr [ebx + OFFSET_RC_ALPHABLEND_ENABLE], 0
	jne		SHORT SecondPassSetup_NoAlphaBlendEnable
	SET4IMM	(SST_ENALPHABLEND OR (SST_A_ONE SHL SST_RGBSRCFACT_SHIFT) OR (SST_A_ONE SHL SST_RGBDSTFACT_SHIFT)), CMD_FIFO
	jmp		SHORT SecondPassSetup_AlphaBlendEnableDone
SecondPassSetup_NoAlphaBlendEnable:
	mov		eax, dword ptr [ebx + OFFSET_RC_ALPHA_MODE]
	and		eax, (NOT SST_RGBDSTFACT)
	or		eax, (SST_A_ONE SHL SST_RGBDSTFACT_SHIFT)
	SET4GPR	eax, CMD_FIFO
SecondPassSetup_AlphaBlendEnableDone:

comment $
        if ( pRc->zEnable ) {
          if( pRc->zWriteEnable ) {
            // if z-buffering then this triangle z values equals the values written on pass 1
            fbzMode = (pRc->sst.fbzMode & ~(SST_ZFUNC_LT | SST_ZFUNC_GT)) | SST_ZFUNC_EQ;
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzMode, 0xF ) );
            SETPD( cmdFifo, ghw->fbzMode, fbzMode );
          }
        }
$
	cmp		dword ptr [ebx + OFFSET_RC_Z_ENABLE], 0
	je		SHORT SecondPassSetup_NoZEnable
	cmp		dword ptr [ebx + OFFSET_RC_ZWRITE_ENABLE], 0
	je		SHORT SecondPassSetup_NoZEnable
	mov		eax, [ebx + OFFSET_RC_FBZ_MODE]
	and		eax, (NOT (SST_ZFUNC_LT OR SST_ZFUNC_GT))
	or		eax, SST_ZFUNC_EQ
	SET4IMM	CMD_FBZ_MODE, CMD_FIFO
	SET4GPR	eax, CMD_FIFO
SecondPassSetup_NoZEnable:

comment $
        if (IS_NAPALM) { //NAPALM_CU
        // texture mapping off and texture blending off but keep alpha the same
	#ifdef NEW_CCU
            combineModeFBI = ( (SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK | 
                               SST_CM_DISABLE_CHROMA_SUBSTITUTION | 
                               SST_CM_USE_COMBINE_MODE ) & pRc->sst.combineModeFBI) |
                               (pRc->sst.combineModeFBI & (~(SST_CM_CC_OTHERSELECT |SST_CM_CC_LOCALSELECT|
                                    SST_CM_CC_MSELECT_7 |SST_CM_CC_INVERT_OTHER|
                                    SST_CM_CC_INVERT_LOCAL|SST_CM_CC_OUTSHIFT|SST_CM_CC_INVERT_ADD_LOCAL))); 
            combineModeFBI |= SST_CM_CC_OTHERSELECT_IRGB; //Use iterated color (specular color)
            //fbzcolorpath: pass iterated color and leave alpha the same.
            fbzColorPath = pRc->sst.fbzColorPath &(~SST_CCOMBINE);
            SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, combineModeFBI );
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 0, fbzColorPath, 0x1 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->fbzColorPath, fbzColorPath );
	#else //NEW_CCU
            combineModeFBI = ( SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK | 
                               SST_CM_CCA_OTHERSELECT_TA | 
                               SST_CM_DISABLE_CHROMA_SUBSTITUTION | 
                               SST_CM_USE_COMBINE_MODE ) & pRc->sst.combineModeFBI; 
            SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, combineModeFBI );
	#endif //NEW_CCU
        } else {
            fbzColorPath = (pRc->sst.fbzColorPath & ~(SST_RGBSELECT | SST_CCOMBINE)) | SST_RGBSEL_RGBA;
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzColorPath, 0xF ) );
            SETPD( cmdFifo, ghw->fbzColorPath, fbzColorPath );
        }
$
	test	dword ptr _localFlags[esp+STACK_OFFSET], (FLAG_NAPALM)
	je		SecondPassSetup_NotNapalm

SecondPassSetup_IsNapalm:
ifdef NEW_CCU
	mov		eax, [ebx + OFFSET_RC_COMBINE_MODE_FBI]
	and		eax, (SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK OR SST_CM_DISABLE_CHROMA_SUBSTITUTION OR SST_CM_USE_COMBINE_MODE OR	\
				  (NOT (SST_CM_CC_OTHERSELECT OR SST_CM_CC_LOCALSELECT OR SST_CM_CC_MSELECT_7 OR SST_CM_CC_INVERT_OTHER OR	\
						SST_CM_CC_INVERT_LOCAL OR SST_CM_CC_OUTSHIFT OR SST_CM_CC_INVERT_ADD_LOCAL) )						\
				 )
 if SST_CM_CC_OTHERSELECT_IRGB
	or		eax, SST_CM_CC_OTHERSELECT_IRGB	; = zero
 endif
	SET4IMM	CMD_NAPALM_COMBINE_MODE, CMD_FIFO
	SET4GPR	eax, CMD_FIFO
	mov		eax, (NOT SST_CCOMBINE)
	and		eax, [ebx + OFFSET_RC_FBZ_COLORPATH]
	SET4IMM (CMD_FBZ_COLORPATH OR (1 SHL 11)), CMD_FIFO
	SET4GPR	eax, CMD_FIFO
else
	mov		eax, [ebx + OFFSET_RC_COMBINE_MODE_FBI]
	and		eax, (	SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK OR \
					SST_CM_CCA_OTHERSELECT_TA OR \
					SST_CM_DISABLE_CHROMA_SUBSTITUTION OR \
					SST_CM_USE_COMBINE_MODE )
	SET4IMM	CMD_NAPALM_COMBINE_MODE, CMD_FIFO
	SET4GPR	eax, CMD_FIFO
endif
	jmp		SendSecond_SetupComplete

SecondPassSetup_NotNapalm:
	mov		eax, [ebx + OFFSET_RC_FBZ_COLORPATH]
	and		eax, (NOT (SST_RGBSELECT OR SST_CCOMBINE))
if SST_RGBSEL_RGBA		; =zero
	or		eax, SST_RGBSEL_RGBA
endif
	SET4IMM	CMD_FBZ_COLORPATH, CMD_FIFO
	SET4GPR	eax, CMD_FIFO


SendSecond_SetupComplete:

comment $
        // Send 1st vertex
        SETFPD( cmdFifo, ghw0->sVx, FLTP(pA)[FVFO_SX] + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, FLTP(pA)[FVFO_SY] + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, RGBA_SETALPHA(pA[FVFO_SPECULAR], RGBA_GETALPHA(aColor)) );

        if(setupFlag & SST_SETUP_Z)
          SETFPD( cmdFifo, ghw0->sVz, z1 );
        if(setupFlag & SST_SETUP_Wfbi)
          SETFPBD( cmdFifo, ghw0->sOowfbi, w1 );
        if (pRc->alphaBlendEnable) {
            if(setupFlag & SST_SETUP_W0)
              SETFPD( cmdFifo, ghw0->sOow0, FLTP(pA)[FVFO_RHW] );
            if(setupFlag & SST_SETUP_ST0) {
                SETFPD( cmdFifo, ghw0->sSow0, s1 );
                SETFPD( cmdFifo, ghw0->sTow0, t1 );
            }
            if(setupFlag & SST_SETUP_ST1) {
                SETFPD( cmdFifo, ghw0->sSow1, s1a );
                SETFPD( cmdFifo, ghw0->sTow1, t1a );
            }
        }
        // Send 2nd & 3rd verticies ...
$
; Copy the specular buffer to the hardware

	; align to 4-byte boundary
	test	hwPtr, 4
	je		SecondPass_HwPtrAlignDone
	SET4IMM	0, CMD_FIFO
SecondPass_HwPtrAlignDone:

	shr		spPtr, 2						; # of dwords to write
	mov		ecx, spPtr						; # of  bytes to write

	sub		ecx, 08h						; is there at least 8 dwords to write?
	lea		esi, [SpecularStream]		; esi = spPtr
	jl		SHORT SecondPass_BlockMoveDone

align 16
SecondPass_NextBlockMove:
	movq	mm0, qword ptr [esi+000h]
	movq	mm1, qword ptr [esi+008h]
	add		hwPtr, 020h						; pre-increment destination pointer
	movq	mm2, qword ptr [esi+010h]
	movq	mm3, qword ptr [esi+018h]
	movq	qword ptr [hwPtr-020h], mm0
	movq	qword ptr [hwPtr-018h], mm1
	add		esi, 020h						; post-increment source pointer
	movq	qword ptr [hwPtr-010h], mm2
	sub		ecx, 08h						; we wrote 8 more dwords
	movq	qword ptr [hwPtr-008h], mm3
	jge		SHORT SecondPass_NextBlockMove

SecondPass_BlockMoveDone:
	add		ecx, 08h
	rep movsd


comment $
        // restore everything back to the way it was
        if (IS_NAPALM) { //NAPALM_CU
            SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
            SETPD( cmdFifo, ghw->combineMode, pRc->sst.combineModeFBI );
	#ifdef NEW_CCU
            SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, fbzColorPath,1 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->fbzColorPath, pRc->sst.fbzColorPath );
	#endif
        } else {
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzColorPath, 0xF ) );
            SETPD( cmdFifo, ghw->fbzColorPath, pRc->sst.fbzColorPath );
        }
$
	test	dword ptr _localFlags[esp+STACK_OFFSET], (FLAG_NAPALM)
	je		SecondPassRestore_NotNapalm


SecondPassRestore_IsNapalm:
	mov		eax, [ebx + OFFSET_RC_COMBINE_MODE_FBI]
	SET4IMM	CMD_NAPALM_COMBINE_MODE, CMD_FIFO
	SET4GPR	eax, CMD_FIFO
ifdef NEW_CCU
	SET4IMM (CMD_FBZ_COLORPATH OR (1 SHL 11)), CMD_FIFO
	mov		eax, [ebx + OFFSET_RC_FBZ_COLORPATH]
	SET4GPR	eax, CMD_FIFO
endif
	jmp		SecondPassRestore_Fog

SecondPassRestore_NotNapalm:
	mov		eax, [ebx + OFFSET_RC_FBZ_COLORPATH]
	SET4IMM	CMD_FBZ_COLORPATH, CMD_FIFO
	SET4GPR	eax, CMD_FIFO


comment $
        if (pRc->fogEnable) {
          SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0xF ) );
          SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode );
        }
$
SecondPassRestore_Fog:
	cmp		dword ptr [ebx + OFFSET_RC_FOG_ENABLE], 0
	je		SHORT SecondPassRestore_NoFogEnable
	mov		eax, dword ptr [ebx + OFFSET_RC_FOG_MODE]
	SET4IMM	CMD_FOG_MODE, CMD_FIFO
	SET4GPR	eax, CMD_FIFO
SecondPassRestore_NoFogEnable:

comment $
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, alphaMode, 0xF ) );
        SETPD( cmdFifo, ghw->alphaMode, pRc->sst.alphaMode );
        SETPD( cmdFifo, ghw->fbzMode, pRc->sst.fbzMode );
$
	mov		eax, [ebx + OFFSET_RC_ALPHA_MODE]
	mov		edx, [ebx + OFFSET_RC_FBZ_MODE]
	SET4IMM	CMD_ALPHA_DIFF_MODE, CMD_FIFO
	SET4GPR	eax, CMD_FIFO
	SET4GPR	edx, CMD_FIFO


;********************************************************************************************************
;********************************************************************************************************
; Done - fix up the FIFO pointer and FIFO room values, then outa here
;
;	FIFO_ROOM -= (curr.FIFO_PTR - prev.FIFO_PTR) / 4
;	FIFO_PTR   = curr.FIFO_PTR (hwPtr)
;********************************************************************************************************
;********************************************************************************************************
align 16
ENDIF ;SEND_SPEC
DoneRender:
	mov		ebx, hwPtr									; tmp hwPtr
	sub     hwPtr, dword ptr[__asm_data + _FIFO_PTR]	; current pointer - previous pointer = bytes added

	mov     ecx, dword ptr [__asm_data + _FIFO_ROOM]	; # of entries left in the fifo

	shr     hwPtr, 2 									; dwords added
	mov     dword ptr[__asm_data + _FIFO_PTR], ebx		; update the fifo pointer

	sub		ecx, hwPtr
	mov     dword ptr [__asm_data + _FIFO_ROOM], ecx	; update fifo room

	sfence	; dump write buffers (Intel bug)

	ret

ENDM	;DRAW_TRIANGLES


;#################################################################################################################################
;#################################################################################################################################
align 16
LookupNewKNIDrawTriangleFunction   PROC     NEAR		; expects the local flags to be in eax

;**********************************************************
; Test for special cases:
;**********************************************************
	mov		dword ptr [LastFlags], eax					; update the local flags
	cmp		eax, (  \;FLAG_SPEC		OR \
					\;FLAG_FLAT		OR \
					FLAG_COLOR		OR \
					\;FLAG_WBUFFER	OR \
					FLAG_Z			OR \
					\;FLAG_WFBI		OR \
					\;FLAG_VFOG		OR \
					\;FLAG_W		OR \
					\;FLAG_PC		OR \
					\;FLAG_ALPHA	OR \
					\;FLAG_TX0		OR \
					\;FLAG_TX1		OR \
				  	\;FLAG_WRAP_TX	OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_NoRhw_NoPc_NoAlpha_NoTx_NoWrap	;00014
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_SPEC		OR \
					FLAG_COLOR		OR \
					FLAG_Z			OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_Spec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_NoRhw_NoPc_NoAlpha_NoTx_NoWrap	;00015
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_PC			OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_NoRhw_Pc_NoAlpha_NoTx_NoWrap ;00114
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_ALPHA		OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_NoRhw_NoPc_Alpha_NoTx_NoWrap	;00214
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_COLOR		OR \
					FLAG_W			OR \
					FLAG_PC			OR \
					FLAG_TX0		OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_NoZ_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_NoWrap ;00584
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_W			OR \
					FLAG_PC			OR \
					FLAG_TX0		OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_NoWrap ;00594
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_SPEC		OR \
					FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_W			OR \
					FLAG_PC			OR \
					FLAG_TX0		OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_Spec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_NoWrap ;00595
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_FLAT		OR \
					FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_W			OR \
					FLAG_PC			OR \
					FLAG_TX0		OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_NoSpec_Flat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_NoWrap ;00596
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_WFBI		OR \
					FLAG_VFOG		OR \
					FLAG_W			OR \
					FLAG_PC			OR \
					FLAG_TX0		OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_Wfbi_Vfog_Rhw_Pc_NoAlpha_1Tx_NoWrap ;005f4
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_SPEC		OR \
					FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_WFBI		OR \
					FLAG_VFOG		OR \
					FLAG_W			OR \
					FLAG_PC			OR \
					FLAG_TX0		OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_Spec_NoFlat_Color_NoWbuff_Z_Wfbi_Vfog_Rhw_Pc_NoAlpha_1Tx_NoWrap ;005f5
	jmp		FindDrawTriFunctionDone


@@:	cmp		eax, (  FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_W			OR \
					FLAG_PC			OR \
					FLAG_ALPHA		OR \
					FLAG_TX0		OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_Alpha_1Tx_NoWrap ;00794
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_SPEC		OR \
					FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_W			OR \
					FLAG_PC			OR \
					FLAG_ALPHA		OR \
					FLAG_TX0		OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_Spec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_Alpha_1Tx_NoWrap ;00795
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_FLAT		OR \
					FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_W			OR \
					FLAG_PC			OR \
					FLAG_ALPHA		OR \
					FLAG_TX0		OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_NoSpec_Flat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_Alpha_1Tx_NoWrap ;00796
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_WFBI		OR \
					FLAG_VFOG		OR \
					FLAG_W			OR \
					FLAG_PC			OR \
					FLAG_ALPHA		OR \
					FLAG_TX0		OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_Wfbi_Vfog_Rhw_Pc_Alpha_1Tx_NoWrap ;007f4
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_W			OR \
					FLAG_PC			OR \
					FLAG_TX0		OR \
					FLAG_TX1		OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_2Tx_NoWrap ;00d94
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_WFBI		OR \
					FLAG_VFOG		OR \
					FLAG_W			OR \
					FLAG_PC			OR \
					FLAG_TX0		OR \
					FLAG_TX1		OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_Wfbi_Vfog_Rhw_Pc_NoAlpha_2Tx_NoWrap ;00df4
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_W			OR \
					FLAG_PC			OR \
					FLAG_ALPHA		OR \
					FLAG_TX0		OR \
					FLAG_TX1		OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_Alpha_2Tx_NoWrap ;00f94
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_WFBI		OR \
					FLAG_VFOG		OR \
					FLAG_W			OR \
					FLAG_PC			OR \
					FLAG_ALPHA		OR \
					FLAG_TX0		OR \
					FLAG_TX1		OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_Wfbi_Vfog_Rhw_Pc_Alpha_2Tx_NoWrap ;00ff4
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_W			OR \
					FLAG_PC			OR \
					FLAG_ALPHA		OR \
					FLAG_TX0		OR \
				  	FLAG_WRAP_TX	OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_Alpha_1Tx_Wrap ;10794
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_W			OR \
					FLAG_PC			OR \
					FLAG_TX0		OR \
					FLAG_TX1		OR \
				  	FLAG_WRAP_TX	OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_2Tx_Wrap ;10d94
	jmp		FindDrawTriFunctionDone

@@:	cmp		eax, (  FLAG_COLOR		OR \
					FLAG_Z			OR \
					FLAG_W			OR \
					FLAG_PC			OR \
					FLAG_ALPHA		OR \
					FLAG_TX0		OR \
					FLAG_TX1		OR \
				  	FLAG_WRAP_TX	OR \
					0 )
	jne		@F
	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_Alpha_2Tx_Wrap ;10f94
	jmp		FindDrawTriFunctionDone

	; can't find a special case function, use the default
@@:	mov		dword ptr [DrawTriFunction], DrawTriangleKNI_Default

FindDrawTriFunctionDone:
	ret
LookupNewKNIDrawTriangleFunction   ENDP

;********************************************************************************************************************
; Special Case Draw Triangle Functions
;********************************************************************************************************************
PUBLIC   DrawTriangleKNI_Default	; default procedure that checks everything
DrawTriangleKNI_Default   PROC     NEAR
	SEND_SPEC		= CHECK
	SEND_FLAT		= CHECK
	SEND_COLOR		= CHECK	   
	SEND_WBUFFER	= CHECK
	SEND_Z			= CHECK
	SEND_WFBI		= CHECK
	SEND_VFOG		= CHECK
	SEND_W			= CHECK
	SEND_PC			= CHECK
	SEND_ALPHA		= CHECK
	SEND_TX0		= CHECK
	SEND_TX1		= CHECK
	SEND_WRAP		= CHECK
	SEND_NAPALM		= CHECK
	START_BOUNDARY	= DONT_CARE
	ALIGN_QWORD_HWPTR_WRITES = 0	; don't try to align the default case
	ALIGN_QWORD_SBUFF_WRITES = 0	; don't try to align the default case
		DRAW_TRIANGLES
DrawTriangleKNI_Default		ENDP

PUBLIC   DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_NoRhw_NoPc_NoAlpha_NoTx_NoWrap	;00014
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_NoRhw_NoPc_NoAlpha_NoTx_NoWrap   PROC     NEAR
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_NoRhw_NoPc_Alpha_NoTx_NoWrap::			;00214
	SEND_SPEC		= FALSE
	SEND_FLAT		= FALSE
	SEND_COLOR		= TRUE	   
	SEND_WBUFFER	= FALSE
	SEND_Z			= TRUE
	SEND_WFBI		= FALSE
	SEND_VFOG		= FALSE
	SEND_W			= FALSE
	SEND_PC			= FALSE
	SEND_ALPHA		= FALSE
	SEND_TX0		= FALSE
	SEND_TX1		= FALSE
	SEND_WRAP		= FALSE
	SEND_NAPALM		= CHECK
	START_BOUNDARY	= START_ODD
	ALIGN_QWORD_HWPTR_WRITES = 1	; 13 or 5 writes per tri
	ALIGN_QWORD_SBUFF_WRITES = 0	; no 2nd specular pass (no textures)
		DRAW_TRIANGLES
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_NoRhw_NoPc_NoAlpha_NoTx_NoWrap		ENDP

PUBLIC   DrawTriangleKNI_Spec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_NoRhw_NoPc_NoAlpha_NoTx_NoWrap	;00015
DrawTriangleKNI_Spec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_NoRhw_NoPc_NoAlpha_NoTx_NoWrap   PROC     NEAR
	SEND_SPEC		= TRUE
	SEND_FLAT		= FALSE
	SEND_COLOR		= TRUE	   
	SEND_WBUFFER	= FALSE
	SEND_Z			= TRUE
	SEND_WFBI		= FALSE
	SEND_VFOG		= FALSE
	SEND_W			= FALSE
	SEND_PC			= FALSE
	SEND_ALPHA		= FALSE
	SEND_TX0		= FALSE
	SEND_TX1		= FALSE
	SEND_WRAP		= FALSE
	SEND_NAPALM		= CHECK
	START_BOUNDARY	= START_ODD
	ALIGN_QWORD_HWPTR_WRITES = 1	; 13 or 5 writes per tri
	ALIGN_QWORD_SBUFF_WRITES = 0	; no 2nd specular pass (no textures)
		DRAW_TRIANGLES
DrawTriangleKNI_Spec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_NoRhw_NoPc_NoAlpha_NoTx_NoWrap		ENDP

PUBLIC   DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_NoRhw_Pc_NoAlpha_NoTx_NoWrap	;00114
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_NoRhw_Pc_NoAlpha_NoTx_NoWrap   PROC     NEAR
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_NoRhw_Pc_Alpha_NoTx_NoWrap::			;00314
	SEND_SPEC		= FALSE
	SEND_FLAT		= FALSE
	SEND_COLOR		= TRUE	   
	SEND_WBUFFER	= FALSE
	SEND_Z			= TRUE
	SEND_WFBI		= FALSE
	SEND_VFOG		= FALSE
	SEND_W			= FALSE
	SEND_PC			= TRUE
	SEND_ALPHA		= FALSE
	SEND_TX0		= FALSE
	SEND_TX1		= FALSE
	SEND_WRAP		= FALSE
	SEND_NAPALM		= CHECK
	START_BOUNDARY	= START_ODD
	ALIGN_QWORD_HWPTR_WRITES = 1	; 13 or 5 writes per tri
	ALIGN_QWORD_SBUFF_WRITES = 0	; no 2nd specular pass
		DRAW_TRIANGLES
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_NoRhw_Pc_NoAlpha_NoTx_NoWrap		ENDP

PUBLIC   DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_NoZ_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_NoWrap	;00584
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_NoZ_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_NoWrap   PROC     NEAR
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_NoZ_NoWfbi_NoVfog_Rhw_Pc_Alpha_1Tx_NoWrap::				;00784 - unused
	SEND_SPEC		= FALSE
	SEND_FLAT		= FALSE
	SEND_COLOR		= TRUE	   
	SEND_WBUFFER	= FALSE
	SEND_Z			= FALSE
	SEND_WFBI		= FALSE
	SEND_VFOG		= FALSE
	SEND_W			= TRUE
	SEND_PC			= TRUE
	SEND_ALPHA		= FALSE
	SEND_TX0		= TRUE
	SEND_TX1		= FALSE
	SEND_WRAP		= FALSE
	SEND_NAPALM		= CHECK
	START_BOUNDARY	= START_ODD
	ALIGN_QWORD_HWPTR_WRITES = 1	; 19 or 7 writes per tri
	ALIGN_QWORD_SBUFF_WRITES = 0	; no 2nd specular pass
		DRAW_TRIANGLES
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_NoZ_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_NoWrap		ENDP

PUBLIC   DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_NoWrap	;00594
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_NoWrap   PROC     NEAR
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_Alpha_1Tx_NoWrap::			;00794
	SEND_SPEC		= FALSE
	SEND_FLAT		= FALSE
	SEND_COLOR		= TRUE	   
	SEND_WBUFFER	= FALSE
	SEND_Z			= TRUE
	SEND_WFBI		= FALSE
	SEND_VFOG		= FALSE
	SEND_W			= TRUE
	SEND_PC			= TRUE
	SEND_ALPHA		= FALSE
	SEND_TX0		= TRUE
	SEND_TX1		= FALSE
	SEND_WRAP		= FALSE
	SEND_NAPALM		= CHECK
	START_BOUNDARY	= START_EVEN
	ALIGN_QWORD_HWPTR_WRITES = 1	; 22 or 8 writes per tri
	ALIGN_QWORD_SBUFF_WRITES = 0	; no 2nd specular pass
		DRAW_TRIANGLES
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_NoWrap		ENDP

PUBLIC   DrawTriangleKNI_Spec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_NoWrap	;00595
DrawTriangleKNI_Spec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_NoWrap   PROC     NEAR
	SEND_SPEC		= TRUE
	SEND_FLAT		= FALSE
	SEND_COLOR		= TRUE	   
	SEND_WBUFFER	= FALSE
	SEND_Z			= TRUE
	SEND_WFBI		= FALSE
	SEND_VFOG		= FALSE
	SEND_W			= TRUE
	SEND_PC			= TRUE
	SEND_ALPHA		= FALSE
	SEND_TX0		= TRUE
	SEND_TX1		= FALSE
	SEND_WRAP		= FALSE
	SEND_NAPALM		= CHECK
	START_BOUNDARY	= START_EVEN
	ALIGN_QWORD_HWPTR_WRITES = 1	; 22 or 8 writes per tri (1st pass)
	ALIGN_QWORD_SBUFF_WRITES = 0	; 13 or 5 writes per tri (2nd pass)
		DRAW_TRIANGLES
DrawTriangleKNI_Spec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_NoWrap		ENDP

PUBLIC   DrawTriangleKNI_NoSpec_Flat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_NoWrap	;00596
DrawTriangleKNI_NoSpec_Flat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_NoWrap   PROC     NEAR
DrawTriangleKNI_NoSpec_Flat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_Alpha_1Tx_NoWrap::			;00796
	SEND_SPEC		= FALSE
	SEND_FLAT		= FALSE
	SEND_COLOR		= TRUE	   
	SEND_WBUFFER	= FALSE
	SEND_Z			= TRUE
	SEND_WFBI		= FALSE
	SEND_VFOG		= FALSE
	SEND_W			= TRUE
	SEND_PC			= TRUE
	SEND_ALPHA		= FALSE
	SEND_TX0		= TRUE
	SEND_TX1		= FALSE
	SEND_WRAP		= FALSE
	SEND_NAPALM		= CHECK
	START_BOUNDARY	= START_EVEN
	ALIGN_QWORD_HWPTR_WRITES = 1	; 22 or 8 writes per tri
	ALIGN_QWORD_SBUFF_WRITES = 0	; no 2nd specular pass
		DRAW_TRIANGLES
DrawTriangleKNI_NoSpec_Flat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_NoWrap		ENDP

PUBLIC   DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_Wfbi_Vfog_Rhw_Pc_NoAlpha_1Tx_NoWrap	;005f4
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_Wfbi_Vfog_Rhw_Pc_NoAlpha_1Tx_NoWrap   PROC     NEAR
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_Wfbi_Vfog_Rhw_Pc_Alpha_1Tx_NoWrap::			;007f4
	SEND_SPEC		= FALSE
	SEND_FLAT		= FALSE
	SEND_COLOR		= TRUE	   
	SEND_WBUFFER	= FALSE
	SEND_Z			= TRUE
	SEND_WFBI		= TRUE
	SEND_VFOG		= TRUE
	SEND_W			= TRUE
	SEND_PC			= TRUE
	SEND_ALPHA		= FALSE
	SEND_TX0		= TRUE
	SEND_TX1		= FALSE
	SEND_WRAP		= FALSE
	SEND_NAPALM		= CHECK
	START_BOUNDARY	= START_ODD
	ALIGN_QWORD_HWPTR_WRITES = 1	; 25 or 9 writes per tri
	ALIGN_QWORD_SBUFF_WRITES = 0	; no 2nd specular pass
		DRAW_TRIANGLES
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_Wfbi_Vfog_Rhw_Pc_NoAlpha_1Tx_NoWrap		ENDP

PUBLIC   DrawTriangleKNI_Spec_NoFlat_Color_NoWbuff_Z_Wfbi_Vfog_Rhw_Pc_NoAlpha_1Tx_NoWrap	;005f5
DrawTriangleKNI_Spec_NoFlat_Color_NoWbuff_Z_Wfbi_Vfog_Rhw_Pc_NoAlpha_1Tx_NoWrap   PROC     NEAR
	SEND_SPEC		= TRUE
	SEND_FLAT		= FALSE
	SEND_COLOR		= TRUE	   
	SEND_WBUFFER	= FALSE
	SEND_Z			= TRUE
	SEND_WFBI		= TRUE
	SEND_VFOG		= TRUE
	SEND_W			= TRUE
	SEND_PC			= TRUE
	SEND_ALPHA		= FALSE
	SEND_TX0		= TRUE
	SEND_TX1		= FALSE
	SEND_WRAP		= FALSE
	SEND_NAPALM		= CHECK
	START_BOUNDARY	= START_ODD
	ALIGN_QWORD_HWPTR_WRITES = 1	; 25 or 9 writes per tri
	ALIGN_QWORD_SBUFF_WRITES = 1	; 16 or 6 writes per tri (2nd pass)
		DRAW_TRIANGLES
DrawTriangleKNI_Spec_NoFlat_Color_NoWbuff_Z_Wfbi_Vfog_Rhw_Pc_NoAlpha_1Tx_NoWrap		ENDP

PUBLIC   DrawTriangleKNI_Spec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_Alpha_1Tx_NoWrap ;00795
DrawTriangleKNI_Spec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_Alpha_1Tx_NoWrap   PROC     NEAR
	SEND_SPEC		= TRUE
	SEND_FLAT		= FALSE
	SEND_COLOR		= TRUE	   
	SEND_WBUFFER	= FALSE
	SEND_Z			= TRUE
	SEND_WFBI		= FALSE
	SEND_VFOG		= FALSE
	SEND_W			= TRUE
	SEND_PC			= TRUE
	SEND_ALPHA		= TRUE
	SEND_TX0		= TRUE
	SEND_TX1		= FALSE
	SEND_WRAP		= FALSE
	SEND_NAPALM		= CHECK
	START_BOUNDARY	= START_EVEN
	ALIGN_QWORD_HWPTR_WRITES = 1	; 22 or 8 writes per tri (1st pass)
	ALIGN_QWORD_SBUFF_WRITES = 1	; 22 or 8 writes per tri (2nd pass)
		DRAW_TRIANGLES
DrawTriangleKNI_Spec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_Alpha_1Tx_NoWrap		ENDP

PUBLIC   DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_2Tx_NoWrap	;00d94
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_2Tx_NoWrap   PROC     NEAR
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_Alpha_2Tx_NoWrap::			;00f94
	SEND_SPEC		= FALSE
	SEND_FLAT		= FALSE
	SEND_COLOR		= TRUE	   
	SEND_WBUFFER	= FALSE
	SEND_Z			= TRUE
	SEND_WFBI		= FALSE
	SEND_VFOG		= FALSE
	SEND_W			= TRUE
	SEND_PC			= TRUE
	SEND_ALPHA		= FALSE
	SEND_TX0		= TRUE
	SEND_TX1		= TRUE
	SEND_WRAP		= FALSE
	SEND_NAPALM		= CHECK
	START_BOUNDARY	= START_EVEN
	ALIGN_QWORD_HWPTR_WRITES = 1	; 28 or 10 writes per tri
	ALIGN_QWORD_SBUFF_WRITES = 0	; no 2nd specular pass
		DRAW_TRIANGLES
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_2Tx_NoWrap		ENDP

PUBLIC   DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_Wfbi_Vfog_Rhw_Pc_NoAlpha_2Tx_NoWrap	;00df4
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_Wfbi_Vfog_Rhw_Pc_NoAlpha_2Tx_NoWrap   PROC     NEAR
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_Wfbi_Vfog_Rhw_Pc_Alpha_2Tx_NoWrap::			;00ff4
	SEND_SPEC		= FALSE
	SEND_FLAT		= FALSE
	SEND_COLOR		= TRUE	   
	SEND_WBUFFER	= FALSE
	SEND_Z			= TRUE
	SEND_WFBI		= TRUE
	SEND_VFOG		= TRUE
	SEND_W			= TRUE
	SEND_PC			= TRUE
	SEND_ALPHA		= FALSE
	SEND_TX0		= TRUE
	SEND_TX1		= TRUE
	SEND_WRAP		= FALSE
	SEND_NAPALM		= CHECK
	START_BOUNDARY	= START_ODD
	ALIGN_QWORD_HWPTR_WRITES = 1	; 31 or 11 writes per tri
	ALIGN_QWORD_SBUFF_WRITES = 0	; no 2nd specular pass
		DRAW_TRIANGLES
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_Wfbi_Vfog_Rhw_Pc_NoAlpha_2Tx_NoWrap		ENDP

PUBLIC   DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_Wrap	;10594 - unused
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_Wrap   PROC     NEAR
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_Alpha_1Tx_Wrap::				;10794
	SEND_SPEC		= FALSE
	SEND_FLAT		= FALSE
	SEND_COLOR		= TRUE	   
	SEND_WBUFFER	= FALSE
	SEND_Z			= TRUE
	SEND_WFBI		= FALSE
	SEND_VFOG		= FALSE
	SEND_W			= TRUE
	SEND_PC			= TRUE
	SEND_ALPHA		= FALSE
	SEND_TX0		= TRUE
	SEND_TX1		= FALSE
	SEND_WRAP		= TRUE
	SEND_NAPALM		= CHECK
	START_BOUNDARY	= START_EVEN
	ALIGN_QWORD_HWPTR_WRITES = 1	; 22 writes per tri
	ALIGN_QWORD_SBUFF_WRITES = 0	; no 2nd specular pass
		DRAW_TRIANGLES
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_1Tx_Wrap		ENDP

PUBLIC   DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_2Tx_Wrap	;10d94
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_2Tx_Wrap   PROC     NEAR
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_Alpha_2Tx_Wrap::				;10f94
	SEND_SPEC		= FALSE
	SEND_FLAT		= FALSE
	SEND_COLOR		= TRUE	   
	SEND_WBUFFER	= FALSE
	SEND_Z			= TRUE
	SEND_WFBI		= FALSE
	SEND_VFOG		= FALSE
	SEND_W			= TRUE
	SEND_PC			= TRUE
	SEND_ALPHA		= FALSE
	SEND_TX0		= TRUE
	SEND_TX1		= TRUE
	SEND_WRAP		= TRUE
	SEND_NAPALM		= CHECK
	START_BOUNDARY	= START_EVEN
	ALIGN_QWORD_HWPTR_WRITES = 1	; 28 writes per tri
	ALIGN_QWORD_SBUFF_WRITES = 0	; no 2nd specular pass
		DRAW_TRIANGLES
DrawTriangleKNI_NoSpec_NoFlat_Color_NoWbuff_Z_NoWfbi_NoVfog_Rhw_Pc_NoAlpha_2Tx_Wrap		ENDP



if GATHER_PERF_INFO
;void GatherKniPerfInfo(flags, numTris, fvfSize)
PUBLIC AllenKniBreakpoint
PUBLIC GatherKniPerfInfo
GatherKniPerfInfo PROC NEAR	SYSCALL 	\
	USES	eax ebx ecx edx esi edi	\
	flags:ptr dword, numTris:ptr dword, fvfSize:ptr dword

	lea		edi, [LastFvfSize]						; grab this address with the debugger

; this is for breakpointing whatever you want
	inc		dword ptr [debugCount]
	cmp		dword ptr [debugCount], 0abch
	cmp		edi, edi					; comment this out to enable the first breakpoint
	jne		MissedBreakpoint
	mov		eax, [numTris]
	cmp		eax, 01fh
	jne		MissedBreakpoint
AllenKniBreakpoint::
	nop
MissedBreakpoint:

UpdateFvfTriTypes:
	mov		edx, fvfSize
	cmp		edx, 5
	jl		UpdateFvfTriTypesDone					; can't handle smaller than 6 for now
	cmp		edx, 12
	jg		UpdateFvfTriTypesDone					; can't handle greater than 12 for now
	sub		edx, 5
	shl		edx, 4									; *16
	lea		edx, dword ptr [FVF5_Type + edx]

	; increment #DrawPrimCalls
	push	dword ptr [edx+4]						; DrawPrimCalls
	call	Dec2Hex
	add		esp, 4
	inc		eax
	push	eax
	call	Hex2Dec
	add		esp, 4
	mov		dword ptr [edx+4], eax

	; increment #tris
	push	dword ptr [edx+12]	 					; #tris
	call	Dec2Hex
	add		esp, 4
	add		eax, numTris
	push	eax
	call	Hex2Dec
	add		esp, 4
	mov		dword ptr [edx+12], eax

	mov		eax, fvfSize
	cmp		eax, [LastFvfSize]
	je		UpdateFvfTriTypesDone

	; increment #DrawPrimCalls
	push	dword ptr [edx+8]						; triSizeChanged
	call	Dec2Hex
	add		esp, 4
	inc		eax
	push	eax
	call	Hex2Dec
	add		esp, 4
	mov		dword ptr [edx+8], eax

	mov		eax, fvfSize
	mov		[LastFvfSize], eax
UpdateFvfTriTypesDone:

	mov		eax, flags
	cmp		eax, [LastTriType]
	je		TriTypeUnchanged
	mov		[LastTriType], eax
	inc		dword ptr [TriTypeChanged]
TriTypeUnchanged:

	; increment TriCount
	push	dword ptr [TriCount]
	call	Dec2Hex
	add		esp, 4
	add		eax, [numTris]
	push	eax
	call	Hex2Dec
	add		esp, 4
	mov		dword ptr [TriCount], eax

UpdateRenderingTypes:
	lea		edi, [TriTypes]
	mov		edx, [flags]
	mov		esi, -16
GatherTriInfo_LoopStart:
	add		esi, 16
	cmp		dword ptr [edi+esi+4], edx			  	; flags = this one?
	je		GatherTriInfo_TriTypeFound
	cmp		esi, (100*16)						  	; end of list?
	jge		GatherTriInfo_EndOfList				  	; my list wasn't long enough
	cmp		dword ptr [edi+esi+0], 0			  	; last triangle type in list?
	jne		GatherTriInfo_LoopStart				  	; next tri type

GatherTriInfo_AddNewTriType:
	mov		dword ptr [edi+esi+4], edx
GatherTriInfo_TriTypeFound:
	mov		eax, [fvfSize]
	and		eax, 0fh								; mask off all but lower nibble
	xor		ecx, ecx
GatherTriInfo_NextFvfNibble:
	mov		ebx, dword ptr [edi+esi+0]
	shr		ebx, cl
	and		ebx, 0fh
	cmp		bl, 0
	je		GatherTriInfo_AddNewFvfSize
	cmp		al, bl
	je		GatherTriInfo_IncrementCallsAndTriCount
	add		ecx, 4
	cmp		ecx, 32
	jge		GatherTriInfo_IncrementCallsAndTriCount	; not enough nibbles (shouldn't happen)
	jmp		GatherTriInfo_NextFvfNibble

GatherTriInfo_AddNewFvfSize:
	shl		eax, cl									; shift to the proper nibble
	or		dword ptr [edi+esi+0], eax
	
GatherTriInfo_IncrementCallsAndTriCount:
	; increment numDrawPrimCalls
	push	dword ptr [edi+esi+8]				  	; numDrawPrimCalls
	call	Dec2Hex
	add		esp, 4
	inc		eax
	push	eax
	call	Hex2Dec
	add		esp, 4
	mov		dword ptr [edi+esi+8], eax

	; increment # of tri's we're doing
	push	dword ptr [edi+esi+12]				  	; # of tri's we're doing
	call	Dec2Hex
	add		esp, 4
	add		eax, [numTris]
	push	eax
	call	Hex2Dec
	add		esp, 4
	mov		dword ptr [edi+esi+12], eax

GatherTriInfo_EndOfList:						  	; list wasn't big enough

	ret
GatherKniPerfInfo	ENDP
endif ;GATHER_PERF_INFO

Hex2Dec	PROC	NEAR	SYSCALL 	\
	data:ptr dword
	local	bcd:tbyte

	emms
	mov		eax, [data]
	mov		dword ptr [bcd], eax
	fild	dword ptr [bcd]
	fbstp	tbyte ptr [bcd]
	mov		eax, dword ptr [bcd]
	ret
Hex2Dec	ENDP

Dec2Hex	PROC	NEAR	SYSCALL 	\
	data:ptr dword
	local	bcd:tbyte

	emms
	mov		eax, [data]
	mov		dword ptr [bcd+0], eax
	mov		dword ptr [bcd+4], 0
	mov		 word ptr [bcd+8], 0
	fbld	tbyte ptr [bcd]
	fistp	dword ptr [bcd]
	mov		eax, dword ptr [bcd]
	ret
Dec2Hex	ENDP
ELSE ; Not Masm 6.14 and up
.err
ENDIF ; If Masm 6.14

_TEXT   ENDS

ENDIF ; STBKNI

END

