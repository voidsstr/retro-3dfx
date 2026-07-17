; 
; atlrend.asm - assembly version of the TL_RENDER_NoClip routines
;

TITLE           atlrend.asm
IF @Version GE 614
ELSE
err requires MASM 6.14 or better!!!
ENDIF

IFDEF TnL_HAL

.686P
.MMX
.K3D
.XMM


.MODEL FLAT


ifdef WINNT
 ifdef INCSTBPERF
  include ..\..\..\..\build\stbperf.inc
 endif
endif

EXTRN	_tnl_fifo_make_room:NEAR


ifdef PERF_MONITORING
extrn __penter:proc
endif


; unrolled equates
SEND_2ND_SPEC_PASS = 1			; send specular for now

MAX_ENTRIES_PER_VERTEX	equ 10	; x,y,color,z,wfbi,rhw,s0,t0,s1,t1
MAX_ENTRIES_PER_TRI		equ MAX_ENTRIES_PER_VERTEX*3 + 1 + 1	; 3 verts, tri command, 1 pad
MAX_BYTES_PER_TRI		equ	MAX_ENTRIES_PER_TRI*4
MAX_SPEC_SWITCH_WRITES	equ	32 ;12+1+10	; number of writes for the switch to/from the specular buffer
MAX_SPEC_SWITCH_BYTES	equ	MAX_SPEC_SWITCH_WRITES*4

SPECULAR_BUFFER_ENTRIES	equ	1024
SPECULAR_SWITCH_ENTRIES equ (1+10+1 + 1+9+1) ; need these for switching back and forth
SPECULAR_BUFFER_SIZE	equ	(SPECULAR_BUFFER_ENTRIES*4)
SPECULAR_SWITCH_SIZE	equ (SPECULAR_SWITCH_ENTRIES * 4) 
MAX_TEXTURES			equ 2

TRI_TYPE_FAN			equ	(1 SHL 0)
TRI_TYPE_STRIP			equ	(1 SHL 1)
STRIP_MODE_BIT			equ	(1 SHL 22)

TEST_BEFORE_LOADING	equ 0


.CONST
align 16
kniSignMask			DWORD	4 dup(07fffffffh)

align 8
MaskFanStrip		DWORD	TRI_TYPE_FAN, TRI_TYPE_STRIP
StripModeBit		DWORD	STRIP_MODE_BIT



.DATA
Public	_asmKniTxScaleHalf	
Public	_asmKniTxScaleOne	
align 16
_asmKniTxScaleHalf	REAL4	4 dup (?)	; wrap ? scale/2 : 0 (*4)
_asmKniTxScaleOne	REAL4	4 dup (?)	; wrap ? scale*1 : 0 (*4)
kni_flt4_temp1		DWORD	4 dup (0)


ifdef DEBUG
; profiling code
Public _TnlStats_TotalTrisRendered				; all tri's
Public _TnlStats_SpecPartailTrisRendered		; specular partial tri's
Public _TnlStats_SpecFullTrisRendered		; specular full tri's
Public _TnlStats_DiffPartailTrisRendered		; diffuse (non-specular) partial tri's
Public _TnlStats_DiffFullTrisRendered		; diffuse (non-specular) full tri's
Public _TnlRndrDbgSpecWrites
_TnlStats_TotalTrisRendered			DWORD 0
_TnlStats_SpecPartailTrisRendered	DWORD 0
_TnlStats_SpecFullTrisRendered	DWORD 0
_TnlStats_DiffPartailTrisRendered	DWORD 0
_TnlStats_DiffFullTrisRendered	DWORD 0
_TnlRndrDbgSpecWrites	DWORD 0
endif



_TEXT       SEGMENT
.CODE
INCLUDE aglobal.inc



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
SET4GPR		MACRO	DEST:REQ, GP_REG32:REQ
	SETALL	<mov dword ptr>, DEST, 0, GP_REG32, 4
ENDM

; immediate value store
SET4IMM		MACRO	DEST:REQ, VALUE:REQ
	SETALL	<mov dword ptr>, DEST, 0, VALUE, 4
ENDM

; mmx register dword store   
SET4MMX		MACRO	DEST:REQ, MMX_REG32:REQ
	SETALL	<movd dword ptr>, DEST, 0, MMX_REG32, 4
ENDM

; mmx register qword store
SET8MMX		MACRO	DEST:REQ, MMX_REG64:REQ
	SETALL	<movq qword ptr>, DEST, 0, MMX_REG64, 8
ENDM

; xmm register dword store
SET4XMM		MACRO	DEST:REQ, XMM_REG32:REQ
	SETALL	<movss dword ptr>, DEST, 0, XMM_REG32, 4
ENDM

; xmm register qword store - lower half
SET8XMM_L	MACRO	DEST:REQ, XMM_REG64:REQ

; The problem is that misaligned writes really suck on the P3, so it's addressed here.
; This one's pretty nasty ... what we're doing is if the destination address is 
; known we check if it's 8-byte aligned.  If not we write the lower dword, 
; shuffle, and write the upper dword.  (Don't fix the source register after we're 
; finished.)  There's a bunch of cases that must be checked here.  
tmpHWPTR_DWORD_OFFSET textequ %(HWPTR_DWORD_OFFSET)
tmpSBUFF_DWORD_OFFSET textequ %(SBUFF_DWORD_OFFSET)

tmpSPLIT_HW_WRITE = (ALIGN_QWORD_HWPTR_WRITES AND NOT ((HWPTR_DWORD_OFFSET MOD 8) SHR 2))
tmpSPLIT_SP_WRITE = (ALIGN_QWORD_SBUFF_WRITES AND NOT ((SBUFF_DWORD_OFFSET MOD 8) SHR 2))

IF	(tmpSPLIT_HW_WRITE AND tmpSPLIT_SP_WRITE)				; split both writes
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
SET8XMM_H	MACRO	DEST:REQ, XMM_REG64:REQ
	SETALL	<movhps qword ptr>, DEST, 0, XMM_REG64, 8
ENDM

; xmm register oword store - we won't use movups because it sucks on Williamette
SET16XMM	MACRO	DEST:REQ, XMM_REG128:REQ

tmpHWPTR_DWORD_OFFSET textequ %(HWPTR_DWORD_OFFSET)
tmpSBUFF_DWORD_OFFSET textequ %(SBUFF_DWORD_OFFSET)

tmpSPLIT_HW_WRITE = (ALIGN_QWORD_HWPTR_WRITES AND NOT ((HWPTR_DWORD_OFFSET MOD 8) SHR 2))
tmpSPLIT_SP_WRITE = (ALIGN_QWORD_SBUFF_WRITES AND NOT ((SBUFF_DWORD_OFFSET MOD 8) SHR 2))

IF	(tmpSPLIT_HW_WRITE AND tmpSPLIT_SP_WRITE)				; split both writes
	SETALL	<movss dword ptr>, DEST, 0, XMM_REG128, 0		; write dword #0
	shufps	XMM_REG128, XMM_REG128, 093h					; shuffle from 3-2-1-0 to 2-1-0-3
	SETALL	<movhps qword ptr>, DEST, 4, XMM_REG128, 0		; write dwords #1 and #2
	SETALL	<movss dword ptr>, DEST, 12, XMM_REG128, 16		; write dword #3

ELSEIF ((NOT tmpSPLIT_HW_WRITE) AND tmpSPLIT_SP_WRITE)	; split the sp write only
  IF ((DEST EQ CMD_FIFO) OR (DEST EQ BOTH_DESTS))
	SETALL	<movlps qword ptr>, CMD_FIFO, 0, XMM_REG128, 0	; get the cmd fifo write done first
	SETALL	<movhps qword ptr>, CMD_FIFO, 8, XMM_REG128, 16
  ENDIF
  IF ((DEST EQ SPEC_BUFFER) OR (DEST EQ BOTH_DESTS))
	SETALL	<movss dword ptr>, SPEC_BUFFER, 0, XMM_REG128, 0; write dword #0
	shufps	XMM_REG128, XMM_REG128, 093h					; shuffle from 3-2-1-0 to 2-1-0-3
	SETALL	<movhps qword ptr>, SPEC_BUFFER, 4, XMM_REG128, 0; write dwords #1 and #2
	SETALL	<movss dword ptr>, SPEC_BUFFER, 12, XMM_REG128, 16; write dword #3
  ENDIF

ELSEIF (tmpSPLIT_HW_WRITE AND (NOT tmpSPLIT_SP_WRITE))	; split the hw write only
  IF ((DEST EQ SPEC_BUFFER) OR (DEST EQ BOTH_DESTS))
	SETALL	<movlps qword ptr>, SPEC_BUFFER, 0, XMM_REG128, 0	; get the spec buffer write done first
	SETALL	<movhps qword ptr>, SPEC_BUFFER, 8, XMM_REG128, 16
  ENDIF
  IF ((DEST EQ CMD_FIFO) OR (DEST EQ BOTH_DESTS))
	SETALL	<movss dword ptr>, CMD_FIFO, 0, XMM_REG128, 0	; write dword #0
	shufps	XMM_REG128, XMM_REG128, 093h					; shuffle from 3-2-1-0 to 2-1-0-3
	SETALL	<movhps qword ptr>, CMD_FIFO, 4, XMM_REG128, 0	; write dwords #1 and #2
	SETALL	<movss dword ptr>, CMD_FIFO, 12, XMM_REG128, 16	; write dword #3
  ENDIF

ELSE	; default case (both are aligned or we're not checking alignment)
	SETALL	<movlps qword ptr>, DEST, 0, XMM_REG128, 0		; write the full qword normally
	SETALL	<movhps qword ptr>, DEST, 8, XMM_REG128, 16
ENDIF

ENDM



SETALL	MACRO		\
	INSTR:REQ,		\ ; the instruction
	DEST:REQ,		\ ; the destination (CMD_FIFO, SPEC_BUFFER, or BOTH_DESTS)
	DEST_OFFSET:REQ,\ ; offset to add to the destination address
	SRC:REQ,		\ ; source register
	INCREMENT:REQ	  ; value to add to the destination register afterwards


IF ((DEST EQ CMD_FIFO) OR (DEST EQ BOTH_DESTS))
;	INSTR [hwPtr + DEST_OFFSET], SRC						;; send to hw
	INSTR [hwPtr + HWPTR_DWORD_OFFSET + DEST_OFFSET], SRC						;; send to hw
ENDIF
IF (((DEST EQ SPEC_BUFFER) OR (DEST EQ BOTH_DESTS)) AND SEND_2ND_SPEC_PASS)
;	INSTR  [spPtr + DEST_OFFSET], SRC						;; send to spec buffer
	INSTR  [spPtr + SBUFF_DWORD_OFFSET + DEST_OFFSET], SRC						;; send to spec buffer
ENDIF

IF INCREMENT
  IF ((DEST EQ CMD_FIFO) OR (DEST EQ BOTH_DESTS))
;	add		hwPtr, INCREMENT
	HWPTR_DWORD_OFFSET = (HWPTR_DWORD_OFFSET + INCREMENT)
  ENDIF
  IF (((DEST EQ SPEC_BUFFER) OR (DEST EQ BOTH_DESTS)) AND SEND_2ND_SPEC_PASS)
;	add		spPtr, INCREMENT
	SBUFF_DWORD_OFFSET = (SBUFF_DWORD_OFFSET + INCREMENT)
  ENDIF
ENDIF

ENDM ; SETALL



;******************************************************************************************************************
;******************************************************************************************************************
;	Dump the specular buffer
;******************************************************************************************************************
;******************************************************************************************************************
; Local stack section
_stk_base		equ (00h)	; esp should point here

; pushad does this
_stk_regEDI		= (_stk_base + 00h)
_stk_regESI		= (_stk_base + 04h)
_stk_regEBP		= (_stk_base + 08h)
_stk_regESP		= (_stk_base + 0ch)
_stk_regEBX		= (_stk_base + 10h)
_stk_regECX		= (_stk_base + 14h)
_stk_regEDX		= (_stk_base + 18h)
_stk_regEAX		= (_stk_base + 1ch)
_stk_RetAddr	= (_stk_base + 20h)

; Stack param section
_stk_pKniRc		= (_stk_RetAddr + 04h)
_stk_pA			= (_stk_RetAddr + 08h)
_stk_pB			= (_stk_RetAddr + 0ch)
_stk_pC			= (_stk_RetAddr + 10h)

STACK_FRAME_SIZE	=	(_stk_RetAddr - _stk_base)
pA		textequ	<ebx>	; ptr to vertex A
pB		textequ	<ecx>	; ptr to vertex B
pC		textequ	<edx>	; ptr to vertex C
KniRc	textequ	<esi>	; ptr to KNI RC struct
spPtr	textequ	<edx>	; ptr to specular buffer
hwPtr	textequ	<edi>	; ptr to command fifo
rFlags	textequ	<ebp>	; RC flag bits

align 16
PUBLIC   _TL_RENDER_DumpSpecBuffer_Asm
_TL_RENDER_DumpSpecBuffer_Asm	PROC	NEAR

	pushad
	mov			KniRc, dword ptr _stk_pKniRc[esp]

;*****************************************************************
;	Check for room in the command fifo 
;	If Intel, fence every 64K writes (cpu bug)
;*****************************************************************
DumpSpec_CmdFifoCheckroom:	; Check that there's enough room in the cmd fifo
	mov			eax, (KNI_RC ptr [KniRc]).CMDFIFOBUMP
	add			eax, (SPECULAR_BUFFER_SIZE+SPECULAR_SWITCH_SIZE)
	cmp			eax, (010000h * 4)
	mov			(KNI_RC ptr [KniRc]).CMDFIFOBUMP, eax
	jl			SHORT DumpSpec_CmdFifoCheckroom_BumpDone
	lock and	(KNI_RC ptr [KniRc]).CMDFIFOBUMP, 0		; store fence
DumpSpec_CmdFifoCheckroom_BumpDone:

	cmp			(KNI_RC ptr [KniRc]).CMDFIFOROOM, (SPECULAR_BUFFER_SIZE+SPECULAR_SWITCH_SIZE)	; add the offset buffer (start & finish)
	jg			SHORT DumpSpec_CmdFifoCheckroom_Done	; enough room left in the fifo?
	push		dword ptr (SPECULAR_BUFFER_ENTRIES+SPECULAR_SWITCH_ENTRIES)
	push		dword ptr (KNI_RC ptr [KniRc]).PRC
	call		_tnl_fifo_make_room						; void tnl_fifo_make_room (RC* pRc, FxU32 size)
	add			esp, 8
	mov			KniRc, dword ptr _stk_pKniRc[esp]
DumpSpec_CmdFifoCheckroom_Done:

	mov			ebx, (KNI_RC ptr [KniRc]).PRC			; ebx = *pRc, don't hose it up!
	mov			rFlags, (KNI_RC ptr [KniRc]).RFBITS		; flags
	mov			hwPtr, (KNI_RC ptr [KniRc]).CMDFIFOPTR

;*****************************************************************
;	Setup the required modes
;*****************************************************************
	test		rFlags, FLAG_FOG_ENABLE
	je			SHORT DumpSpecBuff_Setup_NoFogEnable
	mov			eax, dword ptr [ebx + OFFSET_RC_FOG_MODE]
	and			eax, (NOT SST_FOGMULT)
	or			eax, SST_FOGADD
	mov			dword ptr [hwPtr+0], CMD_FOG_MODE
	mov			dword ptr [hwPtr+4], eax
	add			hwPtr, 8
DumpSpecBuff_Setup_NoFogEnable:


	mov			dword ptr [hwPtr+0], CMD_ALPHA_SPEC_MODE
	cmp			dword ptr [ebx + OFFSET_RC_ALPHABLEND_ENABLE], 0
	jne			SHORT DumpSpecBuff_Setup_NoAlphaBlendEnable
	mov			dword ptr [hwPtr+4], (SST_ENALPHABLEND OR (SST_A_ONE SHL SST_RGBSRCFACT_SHIFT) OR (SST_A_ONE SHL SST_RGBDSTFACT_SHIFT))
	jmp			SHORT DumpSpecBuff_Setup_AlphaBlendEnableDone
DumpSpecBuff_Setup_NoAlphaBlendEnable:
	mov			eax, dword ptr [ebx + OFFSET_RC_ALPHA_MODE]
	and			eax, (NOT SST_RGBDSTFACT)
	or			eax, (SST_A_ONE SHL SST_RGBDSTFACT_SHIFT)
	mov			dword ptr [hwPtr+4], eax
DumpSpecBuff_Setup_AlphaBlendEnableDone:

	mov			eax, rFlags
	and			eax, (FLAG_Z_ENABLE OR FLAG_Z_WRITE_ENABLE)
	add			hwPtr, 8
	cmp			eax, (FLAG_Z_ENABLE OR FLAG_Z_WRITE_ENABLE)
	jne			SHORT DumpSpecBuff_Setup_NoZEnable
	mov			eax, [ebx + OFFSET_RC_FBZ_MODE]
	and			eax, (NOT (SST_ZFUNC_LT OR SST_ZFUNC_GT))
	or			eax, SST_ZFUNC_EQ
	mov			dword ptr [hwPtr+0], CMD_FBZ_MODE
	mov			dword ptr [hwPtr+4], eax
	add			hwPtr, 8
DumpSpecBuff_Setup_NoZEnable:

	test		rFlags, FLAG_NAPALM
	je			SHORT DumpSpecBuff_Setup_NotNapalm

DumpSpecBuff_Setup_IsNapalm:
ifdef NEW_CCU
	mov			eax, [ebx + OFFSET_RC_COMBINE_MODE_FBI]
	and			eax, (SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK OR SST_CM_DISABLE_CHROMA_SUBSTITUTION OR SST_CM_USE_COMBINE_MODE OR	\
					  (NOT (SST_CM_CC_OTHERSELECT OR SST_CM_CC_LOCALSELECT OR SST_CM_CC_MSELECT_7 OR SST_CM_CC_INVERT_OTHER OR	\
							SST_CM_CC_INVERT_LOCAL OR SST_CM_CC_OUTSHIFT OR SST_CM_CC_INVERT_ADD_LOCAL) )						\
					 )
 if SST_CM_CC_OTHERSELECT_IRGB
	or			eax, SST_CM_CC_OTHERSELECT_IRGB	; = zero
 endif
	mov			dword ptr [hwPtr+0], CMD_NAPALM_COMBINE_MODE
	mov			dword ptr [hwPtr+4], eax
	mov			eax, (NOT SST_CCOMBINE)
	and			eax, [ebx + OFFSET_RC_FBZ_COLORPATH]
	mov			dword ptr [hwPtr+8], (CMD_FBZ_COLORPATH OR (1 SHL 11))
	mov			dword ptr [hwPtr+12], eax
	add			hwPtr, 16
else
	mov			eax, [ebx + OFFSET_RC_COMBINE_MODE_FBI]
	and			eax, (	SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK OR SST_CM_CCA_OTHERSELECT_TA OR \
						SST_CM_DISABLE_CHROMA_SUBSTITUTION OR SST_CM_USE_COMBINE_MODE )
	mov			dword ptr [hwPtr+0], CMD_NAPALM_COMBINE_MODE
	mov			dword ptr [hwPtr+4], eax
	add			hwPtr, 8
endif
	jmp			SHORT SendSecond_SetupComplete

DumpSpecBuff_Setup_NotNapalm:
	mov			eax, [ebx + OFFSET_RC_FBZ_COLORPATH]
	and			eax, (NOT (SST_RGBSELECT OR SST_CCOMBINE))
if SST_RGBSEL_RGBA		; =zero
	or			eax, SST_RGBSEL_RGBA
endif
	mov			dword ptr [hwPtr+0], CMD_FBZ_COLORPATH
	mov			dword ptr [hwPtr+4], eax
	add			hwPtr, 8

SendSecond_SetupComplete:


;*****************************************************************
;	Copy the specular buffer to the hardware
;	We have to align everything first...
;*****************************************************************
	mov			edx, KniRc	; save esi
	mov			ecx, (KNI_RC ptr [KniRc]).SPECBUFFPTR
	mov			esi, (KNI_RC ptr [KniRc]).SPECBUFFBASE

	; align to odd 4-byte boundary
	test		hwPtr, 4
	jne			SHORT @F
	mov			dword ptr [hwPtr+0], 0
	add			hwPtr, 4
@@:

	; The specular buffer starts at an odd 8-byte boundary, so set the cmd fifo 
	; an odd boundary (if it's already not).  Then move 1 dword to align source 
	; and destination to even 8-byte boundarys
	mov			eax, dword ptr [esi]
	mov			dword ptr [hwPtr], eax
	add			esi, 4
	add			hwPtr, 4

	sub			ecx, esi
	shr			ecx, 2
ifdef DEBUG
	add			dword ptr [_TnlRndrDbgSpecWrites], ecx
endif

	sub			ecx, 08h						; is there at least 8 dwords to write?
	jl			SHORT DumpSpecBuff_BlockMoveDone

align 16
DumpSpecBuff_NextBlockMove:
	movq		mm0, qword ptr [esi+000h]
	movq		mm1, qword ptr [esi+008h]
	add			hwPtr, 020h						; pre-increment destination pointer
	movq		mm2, qword ptr [esi+010h]
	movq		mm3, qword ptr [esi+018h]
	movq		qword ptr [hwPtr-020h], mm0
	movq		qword ptr [hwPtr-018h], mm1
	add			esi, 020h						; post-increment source pointer
	movq		qword ptr [hwPtr-010h], mm2
	sub			ecx, 08h						; we wrote 8 more dwords
	movq		qword ptr [hwPtr-008h], mm3
	jge			SHORT DumpSpecBuff_NextBlockMove

DumpSpecBuff_BlockMoveDone:
	add			ecx, 08h
	rep movsd

	mov			KniRc, edx	; restore esi

;*****************************************************************
;	Restore the HW
;*****************************************************************
	test		rFlags, FLAG_NAPALM
	je			SHORT DumpSpecBuff_Restore_NotNapalm

DumpSpecBuff_Restore_IsNapalm:
	mov			eax, [ebx + OFFSET_RC_COMBINE_MODE_FBI]
	mov			dword ptr [hwPtr+0], CMD_NAPALM_COMBINE_MODE
	mov			dword ptr [hwPtr+4], eax
ifdef NEW_CCU
	mov			dword ptr [hwPtr+8], (CMD_FBZ_COLORPATH OR (1 SHL 11))
	mov			eax, [ebx + OFFSET_RC_FBZ_COLORPATH]
	mov			dword ptr [hwPtr+12], eax
	add			hwPtr, 16
else
	add			hwPtr, 8
endif
	jmp			SHORT DumpSpecBuff_Restore_Fog

DumpSpecBuff_Restore_NotNapalm:
	mov			eax, [ebx + OFFSET_RC_FBZ_COLORPATH]
	mov			dword ptr [hwPtr+0], CMD_FBZ_COLORPATH
	mov			dword ptr [hwPtr+4], eax
	add			hwPtr, 8

DumpSpecBuff_Restore_Fog:
	test		rFlags, FLAG_FOG_ENABLE
	je			SHORT DumpSpecBuff_Restore_NoFogEnable
	mov			eax, dword ptr [ebx + OFFSET_RC_FOG_MODE]
	mov			dword ptr [hwPtr+0], CMD_FOG_MODE
	mov			dword ptr [hwPtr+4], eax
	add			hwPtr, 8
DumpSpecBuff_Restore_NoFogEnable:

	mov			eax, [ebx + OFFSET_RC_ALPHA_MODE]
	mov			edx, [ebx + OFFSET_RC_FBZ_MODE]
	mov			dword ptr [hwPtr+0], CMD_ALPHA_DIFF_MODE
	mov			dword ptr [hwPtr+4], eax
	mov			dword ptr [hwPtr+8], edx
	add			hwPtr, 12

DumpSpecBuff_Restore:	; leave aligned to an odd 8-byte boundary
	test		hwPtr, 4
	jne			SHORT DumpSpecBuff_Restore_HwPtrAlignDone
	mov			dword ptr [hwPtr+0], 0
	add			hwPtr, 4
DumpSpecBuff_Restore_HwPtrAlignDone:

DumpSpecBuff_RestorePointers:
	mov			eax, (KNI_RC ptr [KniRc]).CMDFIFOPTR
	mov			edx, (KNI_RC ptr [KniRc]).SPECBUFFBASE
	add			eax, (KNI_RC ptr [KniRc]).CMDFIFOROOM
	sub			eax, hwPtr								; CmdFifoRoom -= hwPtr - lastHwPtr
	mov			(KNI_RC ptr [KniRc]).PREVC, 0			; break the autostrip on the next tri
	mov			(KNI_RC ptr [KniRc]).CMDFIFOPTR, hwPtr	; update the cmd fifo ptr
	mov			(KNI_RC ptr [KniRc]).SPECBUFFPTR, edx	; reset the specular pointer to lpSpecBuffBase
	mov			(KNI_RC ptr [KniRc]).CMDFIFOROOM, eax	; update the cmd fifo room

	emms
	popad
	ret

_TL_RENDER_DumpSpecBuffer_Asm	ENDP



;######################################################################################################
;######################################################################################################
;##
;## T&L Triangle Rendering Code
;##
;## void TL_RENDER_NoClip_Asm( RC* pRc, lpTLBN pA, lpTLBN pB, lpTLBN pC )
;##
;######################################################################################################
;######################################################################################################
; Local stack section
_stk_base		equ (00h)	; esp should point here

; pushad does this
_stk_regECX		= (_stk_base + 00h)
_stk_regEBX		= (_stk_base + 04h)
_stk_regEDI		= (_stk_base + 08h)
_stk_regESI		= (_stk_base + 0ch)
_stk_RetAddr	= (_stk_base + 10h)

; Stack param section
_stk_pKniRc		= (_stk_RetAddr + 04h)
_stk_pA			= (_stk_RetAddr + 08h)
_stk_pB			= (_stk_RetAddr + 0ch)
_stk_pC			= (_stk_RetAddr + 10h)

STACK_FRAME_SIZE	=	(_stk_RetAddr - _stk_base)
pA		textequ	<ebx>	; ptr to vertex A
pB		textequ	<ecx>	; ptr to vertex B
pC		textequ	<edx>	; ptr to vertex C
KniRc	textequ	<esi>	; ptr to KNI RC struct
spPtr	textequ	<edx>	; ptr to specular buffer
hwPtr	textequ	<edi>	; ptr to command fifo
rFlags	textequ	<ebp>	; RC flag bits

xmmVa	textequ	<xmm0>
xmmVb	textequ	<xmm1>
xmmVc	textequ	<xmm2>
xmmT1	textequ	<xmm3>
xmmT2	textequ <xmm4>
xmmTxA	textequ	<xmm5>	; texture for vertex A
xmmTxB	textequ	<xmm6>	; texture for vertex B
xmmTxC	textequ	<xmm7>	; texture for vertex C

ALIGN_QWORD_HWPTR_WRITES	equ 1
ALIGN_QWORD_SBUFF_WRITES	equ 1

;******************************************************************************************************************
;******************************************************************************************************************
TL_RENDER_FUNCTION	MACRO
LOCAL CmdFifoSetup, CmdFifoSetup_BumpDone, CmdFifo_RoomOkay, WrapTx, WrapTx_Done
LOCAL AutostripCheck_Spec, AutostripCheck_NonSpec, AutostripCheck_NoSpecFound
LOCAL TwoPassPartialTri, TwoPassPartialTri_Load_TxW, TwoPassPartialTri_Load_Color, TwoPassPartialTri_Load_XYZWfbi, TwoPassPartialTri_SaveLastTriPointers
LOCAL TwoPassPartialTri_SendCommand, TwoPassPartialTri_SendVertexC, TwoPassPartialTri_Cleanup, TwoPassPartialTri_Cleanup_SpecBufferOkay
LOCAL TwoPassFullTri, TwoPassFullTri_Load_TxW, TwoPassFullTri_Load_TxW_Wrap_PcDone, TwoPassFullTri_Load_Color, TwoPassFullTri_Load_XYZWfbi, TwoPassFullTri_SaveLastTriPointers
LOCAL TwoPassFullTri_SendCommand, TwoPassFullTri_SendVertexA, TwoPassFullTri_SendVertexB, TwoPassFullTri_SendVertexC, TwoPassFullTri_Cleanup, TwoPassFullTri_SpecBufferOkay
LOCAL OnePassPartialTri, OnePassPartialTri_Load_TxW, OnePassPartialTri_Load_Color, OnePassPartialTri_Load_XYZWfbi, OnePassPartialTri_SaveLastTriPointers
LOCAL OnePassPartialTri_SendCommand, OnePassPartialTri_SendVertexC, OnePassPartialTri_Cleanup, OnePassPartialTri_Cleanup_SpecBufferOkay
LOCAL OnePassFullTri, OnePassFullTri_Load_TxW, OnePassFullTri_Load_TxW_Wrap_PcDone, OnePassFullTri_Load_Color, OnePassFullTri_Load_XYZWfbi, OnePassFullTri_SaveLastTriPointers
LOCAL OnePassFullTri_SendCommand, OnePassFullTri_SendVertexA, OnePassFullTri_SendVertexB, OnePassFullTri_SendVertexC, OnePassFullTri_Cleanup, OnePassFullTri_SpecBufferOkay


Align 16

ifdef DEBUG
	inc dword ptr [_TnlStats_TotalTrisRendered]
endif

	sub			esp, STACK_FRAME_SIZE
	mov			dword ptr _stk_regESI[esp], esi		; KniRc
	mov			dword ptr _stk_regEDI[esp], edi		; hwPtr
	mov			eax, MAX_BYTES_PER_TRI
	mov			dword ptr _stk_regEBX[esp], ebx		; pA
	mov			dword ptr _stk_regECX[esp], ecx		; pB
	mov			KniRc, dword ptr _stk_pKniRc[esp]
	add			eax, (KNI_RC ptr [KniRc]).CMDFIFOBUMP
	mov			edx, (KNI_RC ptr [KniRc]).CMDFIFOROOM

;******************************************************************************************************************
;	Check for room in the command fifo
;	If Intel, fence every 64K writes (cpu bug)
;******************************************************************************************************************
CmdFifoSetup:
	mov			(KNI_RC ptr [KniRc]).CMDFIFOBUMP, eax
	cmp			eax, (010000h * 4)
	jl			SHORT CmdFifoSetup_BumpDone
	lock and	(KNI_RC ptr [KniRc]).CMDFIFOBUMP, 0		; store fence and clear CMDFIFOBUMP

Align 16
CmdFifoSetup_BumpDone:

	cmp			edx, MAX_BYTES_PER_TRI
	jg			SHORT CmdFifo_RoomOkay					; enough room left in the fifo?
	mov			eax, (KNI_RC ptr [KniRc]).PRC
	push		dword ptr MAX_ENTRIES_PER_TRI
	push		eax
	call		_tnl_fifo_make_room						; void tnl_fifo_make_room (RC* pRc, FxU32 size)
	add			esp, 8
	mov			KniRc, dword ptr _stk_pKniRc[esp]		; gotta reload, tnl_fifo_make_room() stomps everything
Align 16
CmdFifo_RoomOkay:


;******************************************************************************************************************
;	Load the stack variables into local regs
;******************************************************************************************************************
	mov			pA, dword ptr _stk_pA[esp]
	mov			pB, dword ptr _stk_pB[esp]
	mov			pC, dword ptr _stk_pC[esp]


;******************************************************************************************************************
;	WRAP  (ugh!!!)
;
;	First load the textures
;
;	if (wrap)
;		if      ((dAC > 0.5) && (dBC > 0.5))
;			if (C < B)	C += 1.0
;			else A += 1.0, B += 1.0
;		else if ((dBC > 0.5) && (dAB > 0.5))
;			if (B < A)	B += 1.0
;			else A += 1.0, C += 1.0
;		else if ((dAB > 0.5) && (dAC > 0.5))
;			if (A < C)	A += 1.0
;			else B += 1.0, C += 1.0
;
;	if wrap the T&L hands us:	tex = tex * scale + offset
;	else we get:				tex = tex * scale + offset * w
;	What this means is that if wrap is asserted we may have to do the perspective divide (*w)
;******************************************************************************************************************
IF WRAP_TX
WrapTx:

IF (SEND_TX EQ 1)
	movlps		xmmTxA, (TLBN ptr [pA]).TX0
	movlps		xmmTxB, (TLBN ptr [pB]).TX0
	movlps		xmmTxC, (TLBN ptr [pC]).TX0
ELSE
	movaps		xmmTxA, (TLBN ptr [pA]).TEXTURES
	movaps		xmmTxB, (TLBN ptr [pB]).TEXTURES
	movaps		xmmTxC, (TLBN ptr [pC]).TEXTURES
ENDIF

	movaps		xmmT1, xmmTxA
	movaps		xmmT2, xmmTxC
	movaps		xmmVa, xmmTxB
	movaps		xmmVb, oword ptr [kniSignMask]
	subps		xmmT1, xmmTxB
	subps		xmmT2, xmmTxA
	subps		xmmVa, xmmTxC
	movaps		xmmVc, oword ptr [_asmKniTxScaleHalf] ; wrap ? 0.5*scale : 0
	andps		xmmT1, xmmVb					; dAB
	andps		xmmT2, xmmVb					; dAC (dCA)
	andps		xmmVa, xmmVb					; dBC
	cmpnleps 	xmmT1, xmmVc					; (dAB > 0.5) ? -1 : 0
	mov			hwPtr, (KNI_RC ptr [KniRc]).RFBITS; copy flags (hwptr not loaded yet)
	cmpnleps 	xmmT2, xmmVc					; (dAC > 0.5) ? -1 : 0
	cmpnleps 	xmmVa, xmmVc					; (dBC > 0.5) ? -1 : 0

	movaps		xmmVb, xmmT1					; temporary copy of (dAB > 0.5) ? -1 : 0
	andps		xmmT1, xmmT2					; ((dAB > 0.5) && (dAC > 0.5)) ? -1 : 0
	andps		xmmT2, xmmVa					; ((dAC > 0.5) && (dBC > 0.5)) ? -1 : 0
	andps		xmmVb, xmmVa					; ((dAB > 0.5) && (dBC > 0.5)) ? -1 : 0
	movaps		xmmVc, xmmT1
	orps		xmmVc, xmmT2
	movaps		xmmVa, oword ptr [_asmKniTxScaleOne] ; wrap ? 1.0*scale : 0
	orps		xmmVc, xmmVb
	shr			hwPtr, BIT_WRAP_T0_S			; shift wrap bits to bits 3-0
	movmskps	eax, xmmVc						; just store the sign bits (bits 31-4 are zero)
	and			eax, hwPtr
	je			SHORT WrapTx_Done				; early out

	movaps		oword ptr [kni_flt4_temp1], xmmT1 ; temporary copy of ((dAB > 0.5) && (dAC > 0.5)) ? -1 : 0

	andps		xmmT2, xmmVa					; ((dAC > 0.5) && (dBC > 0.5)) ? 1.0 : 0
	movaps		xmmVc, xmmTxC
	xorps		xmmVa, xmmT2					; ((dAC > 0.5) && (dBC > 0.5)) ? 0 : 1.0
	cmpltps		xmmVc, xmmTxB					; (C <  B) ? -1 : 0
	movaps		xmmT1, xmmVc
	andnps		xmmVc, xmmT2					; ((dAC > 0.5) && (dBC > 0.5) && (C >= B)) ? 1.0 : 0
	andps		xmmT1, xmmT2					; ((dAC > 0.5) && (dBC > 0.5) && (C <  B)) ? 1.0 : 0
	addps		xmmTxB, xmmVc					; B += ((dAC > 0.5) && (dBC > 0.5) && (C >= B)) ? 1.0 : 0
	addps		xmmTxA, xmmVc					; A += ((dAC > 0.5) && (dBC > 0.5) && (C >= B)) ? 1.0 : 0
	addps		xmmTxC, xmmT1					; C += ((dAC > 0.5) && (dBC > 0.5) && (C <  B)) ? 1.0 : 0

	andps		xmmVb, xmmVa					; ((dBC > 0.5) && (dAB > 0.5)) ? 1.0 : 0
	movaps		xmmVc, xmmTxB
	xorps		xmmVa, xmmVb					; ((dBC > 0.5) && (dAB > 0.5)) ? xmmWrapOne-xmmWrapOne : xmmWrapOne
	cmpltps		xmmVc, xmmTxA					; (B <  A) ? -1 : 0
	movaps		xmmT2, xmmVc
	andnps		xmmVc, xmmVb					; ((dBC > 0.5) && (dAB > 0.5) && (B >= A)) ? 1.0 : 0
	andps		xmmT2, xmmVb					; ((dBC > 0.5) && (dAB > 0.5) && (B <  A)) ? 1.0 : 0
	addps		xmmTxA, xmmVc					; A += ((dBC > 0.5) && (dAB > 0.5) && (B >= A)) ? xmmWrapOne : 0
	addps		xmmTxC, xmmVc					; C += ((dBC > 0.5) && (dAB > 0.5) && (B >= A)) ? xmmWrapOne : 0
	addps		xmmTxB, xmmT2					; B += ((dBC > 0.5) && (dAB > 0.5) && (B <  A)) ? xmmWrapOne : 0

	movaps		xmmVc, xmmTxA
	cmpltps		xmmVc, xmmTxC					; (A <  C) ? -1 : 0
	andps		xmmVa, oword ptr [kni_flt4_temp1] ; ((dAB > 0.5) && (dAC > 0.5)) ? -1 : 0
	movaps		xmmT1, xmmVc
	andps		xmmVc, xmmVa					; ((dAB > 0.5) && (dAC > 0.5) && (A <  C)) ? xmmWrapOne : 0
	andnps		xmmT1, xmmVa					; ((dAB > 0.5) && (dAC > 0.5) && (A >= C)) ? xmmWrapOne : 0
	addps		xmmTxA, xmmVc					; A += ((dAB > 0.5) && (dAC > 0.5) && (A <  C)) ? xmmWrapOne : 0
	addps		xmmTxB, xmmT1					; B += ((dAB > 0.5) && (dAC > 0.5) && (A >= C)) ? xmmWrapOne : 0
	addps		xmmTxC, xmmT1					; C += ((dAB > 0.5) && (dAC > 0.5) && (A >= C)) ? xmmWrapOne : 0

align 16
WrapTx_Done:
ENDIF ;WRAP_TX

;******************************************************************************************************************
;	Autostrip and Specular Check 
;	Specular 2nd Pass Ck: 2nd pass for specular highlights on textures only if the specular color is not black
;	if SPECULAR && (TX0 | TX1) && ((pA->specular | pB->specular | pC->specular) & 0x00ffffff)
;		spec2ndPassReq
;
;	if (!allowAutostrip || (spec2ndPassReq && !lastSpecRendered))	not-stripable
;	if (not-stripable) lastTriType = FAN
;
;	if ((prevC == currA) && (prevB == currB)    STRIP
;	if ((prevA == currA) && (prevC == currB)    FAN
;	if (currTriType == lastTriType) CmdCont |= STRIP_MODE_BIT
;******************************************************************************************************************
;IF HAS_SPEC_AND_TEXTURES
IF SEND_SPECULAR
IF ENABLE_AUTOSTRIP
AutostripCheck:
AutostripCheck_Spec:
	movd		mm0, (KNI_RC ptr [KniRc]).PREVA				; 0				prevA
	movq		mm1, (KNI_RC ptr [KniRc]).PREVCB 			; prevB			prevC
ENDIF ;ENABLE_AUTOSTRIP
	mov			eax, (TLBN ptr [pA]).SPECULAR				; pA.spec
IF ENABLE_AUTOSTRIP
	movd		mm2, pA										; 0				currA
	movd		mm3, pB										; 0				currB
	punpckldq	mm0, mm1									; prevC			prevA
	movq		mm4, qword ptr [MaskFanStrip]				; TRI_TYPE_STRIP TRI_TYPE_FAN
	punpckldq	mm2, mm2									; currA			currA
ENDIF
	or			eax, (TLBN ptr [pB]).SPECULAR				; pA.spec|pB.spec
IF ENABLE_AUTOSTRIP
	punpckldq	mm3, mm3									; currB			currB
	pcmpeqd		mm0, mm2									; pC=cA?-1:0	pA-cA?-1:0
	pcmpeqd		mm1, mm3									; pB=cB?-1:0	pC-cB?-1:0
	pand		mm0, mm4									; STRIP?-1:0	FAN?-1:0
ENDIF
	or			eax, (TLBN ptr [pC]).SPECULAR				; pA.spec|pB.spec|pC.spec
IF ENABLE_AUTOSTRIP
	pand		mm0, mm1									; STRIP			FAN
	movq		mm1, mm0									; STRIP			FAN
	movd		mm2, (KNI_RC ptr [KniRc]).LASTSPECRENDERED	; lastSpecRendered ? -1 : 0
	punpckhdq	mm1, mm1									; STRIP			STRIP
ENDIF
	and			eax, 00ffffffh								; mask off alpha
IF ENABLE_AUTOSTRIP
	por			mm0, mm1									; STRIP			FAN|STRIP = currTriType
ENDIF ;ENABLE_AUTOSTRIP
	je			AutostripCheck_NoSpecFound

IF ENABLE_AUTOSTRIP
AutostripCheck_SpecFound:
	pand		mm2, mm0									; (autostrip & lastSpecRendered) ? currTriType : 0
	movd		eax, mm2
	test		eax, eax
	je			TwoPassFullTri


;******************************************************************************************************************
;	Two-Pass Partial Triangle
;******************************************************************************************************************
HWPTR_DWORD_OFFSET = 0
SBUFF_DWORD_OFFSET = 0
TwoPassPartialTri:
ifdef DEBUG
	inc			dword ptr [_TnlStats_SpecPartailTrisRendered]
endif
	movd		mm7, (KNI_RC ptr [KniRc]).LASTTRITYPE
	movd		mm2, dword ptr [StripModeBit]	 		; 0				STRIP_MODE_BIT
	movq		mm6, (KNI_RC ptr [KniRc]).CMD_CONT.DIFF_SPEC ; CmdContSpec	CmdContDiff
	pcmpeqd		mm7, mm0								; (currTriType = lastTriType) ? -1 : 0
	pand		mm7, mm2								; (currTriType = lastTriType) ? stripMode : 0

TwoPassPartialTri_Load_TxW:				; Load Rhw and texture(s)
	movss		xmmT2, (TLBN ptr [pC]).W				; 0		0		0		vC.w
IF (SEND_TX EQ 1)
	movlps		xmmTxC, (TLBN ptr [pC]).TX0				; ----------	vC.Tx0.t/s
ELSEIF (SEND_TX EQ 2)
	movaps		xmmTxC, (TLBN ptr [pC]).TEXTURES		; vC.Tx1.t/s	vC.Tx0.t/s
ENDIF

TwoPassPartialTri_Load_Color:			; Load the diffuse (and specular if needed) into mmx registers
	movd		mm2, (TLBN ptr [pC]).DIFFUSE			; 0			vC.diff
	movd		mm5, (TLBN ptr [pC]).SPECULAR			; 0			vC.spec

TwoPassPartialTri_Load_XYZWfbi:			; Load x, y, z, and fog
	movhps		xmmVc, (TLBN ptr [pC]).XY				; vC.y		vC.x	----	----
	movlps		xmmVc, (TLBN ptr [pC]).Z_WFBI			; vC.y		vC.x	vC.wfbi	vC.z

	;Load the Destination Pointers
	punpckldq	mm7, mm7								; stripMode		stripMode
	mov			eax, (KNI_RC ptr [KniRc]).CMDFIFOROOM	; dwCmdFifoRoom
	por			mm6, mm7								; CmdContSpec	CmdContDiff
	mov			hwPtr, (KNI_RC ptr [KniRc]).CMDFIFOPTR
	movd		mm7, (KNI_RC ptr [KniRc]).SPECBUFFPTR	; spPtr = edx = pC

TwoPassPartialTri_SaveLastTriPointers:
	mov			(KNI_RC ptr [KniRc]).PREVC, pC			; pC = edx = spPtr
	movd		spPtr, mm7
	mov			(KNI_RC ptr [KniRc]).PREVB, pB
	movq		mm7, mm6								; CmdContSpec	CmdContDiff
	mov			(KNI_RC ptr [KniRc]).PREVA, pA
	punpckhdq	mm7, mm7								; CmdContSpec	CmdContSpec
	movd		(KNI_RC ptr [KniRc]).LASTTRITYPE, mm0

TwoPassPartialTri_SendCommand:
	SET4MMX		CMD_FIFO, mm6
	SET4MMX		SPEC_BUFFER, mm7

TwoPassPartialTri_SendVertexC:
	SET8XMM_H	BOTH_DESTS, xmmVc			; vC.x, vC.y
	SET4MMX		CMD_FIFO, mm2				; vC.diffuse
	SET4MMX		SPEC_BUFFER, mm5			; vC.specular
IF (SEND_Z AND SEND_WFBI)
	SET8XMM_L	BOTH_DESTS, xmmVc			; vC.z, vC.wfbi
ELSEIF (SEND_Z)
	SET4XMM		BOTH_DESTS, xmmVc			; vC.z
ELSEIF (SEND_WFBI)
	shufps		xmmVc, xmmVc, 0e1h
	SET4XMM		BOTH_DESTS, xmmVc			; vC.wfbi
ENDIF
IF HAS_ALPHA
  IF (SEND_TX)
	SET4XMM		BOTH_DESTS, xmmT2			; vC.w
  ENDIF
  IF (SEND_TX EQ 1)
	SET8XMM_L	BOTH_DESTS, xmmTxC			; vC.Tx0.t, vC.Tx0.s
  ELSEIF (SEND_TX EQ 2)
	SET16XMM	BOTH_DESTS, xmmTxC			; vC.Tx1.t, vC.Tx1.s, vC.Tx0.t, vC.Tx0.s
  ENDIF
ELSE ;HAS_ALPHA
  IF (SEND_TX)
	SET4XMM		CMD_FIFO, xmmT2				; vC.w
  ENDIF
  IF (SEND_TX EQ 1)
	SET8XMM_L	CMD_FIFO, xmmTxC			; vC.Tx0.t, vC.Tx0.s
  ELSEIF (SEND_TX EQ 2)
	SET16XMM	CMD_FIFO, xmmTxC			; vC.Tx1.t, vC.Tx1.s, vC.Tx0.t, vC.Tx0.s
  ENDIF
ENDIF ;HAS_ALPHA


TwoPassPartialTri_Cleanup:
; need to keep the pointers on odd 8-byte alignment
IF ((SBUFF_DWORD_OFFSET MOD 8) SHR 2)
	SET4IMM		SPEC_BUFFER, 0				
ENDIF
IF ((HWPTR_DWORD_OFFSET MOD 8) SHR 2)
	SET4IMM		CMD_FIFO, 0
ENDIF
	sub			eax, HWPTR_DWORD_OFFSET
	add			spPtr, SBUFF_DWORD_OFFSET				; COMMENT OUT TO DISABLE SPECULAR
	mov			(KNI_RC ptr [KniRc]).LASTSPECRENDERED, 0ffffffffh
	add			hwPtr, HWPTR_DWORD_OFFSET
	mov			(KNI_RC ptr [KniRc]).CMDFIFOROOM, eax	; update the cmd fifo room
	mov			(KNI_RC ptr [KniRc]).SPECBUFFPTR, spPtr	; update the spec buffer ptr
	mov			(KNI_RC ptr [KniRc]).CMDFIFOPTR, hwPtr	; update the cmd fifo ptr

	; check if we need to dump the specular buffer yet
	cmp			spPtr, (KNI_RC ptr [KniRc]).SPECBUFFTHRESHOLD
	jl			SHORT TwoPassPartialTri_Cleanup_SpecBufferOkay
	push		KniRc
	call		_TL_RENDER_DumpSpecBuffer_Asm
	pop			eax
TwoPassPartialTri_Cleanup_SpecBufferOkay:

	emms
	mov			ecx, dword ptr _stk_regECX[esp]
	mov			ebx, dword ptr _stk_regEBX[esp]
	mov			esi, dword ptr _stk_regESI[esp]		; KniRc
	mov			edi, dword ptr _stk_regEDI[esp]		; hwPtr
	add			esp, STACK_FRAME_SIZE
	ret
;	Two-Pass Partial Triangle Done


;******************************************************************************************************************
;	Two-Pass Full Triangle
;******************************************************************************************************************
Align 16
ENDIF ;ENABLE_AUTOSTRIP
HWPTR_DWORD_OFFSET = 0
SBUFF_DWORD_OFFSET = 0
TwoPassFullTri:
ifdef DEBUG
	inc			dword ptr [_TnlStats_SpecFullTrisRendered]
endif
	movq		mm6, (KNI_RC ptr [KniRc]).CMD_START.DIFF_SPEC ; CmdStartSpec	CmdStartDiff

TwoPassFullTri_Load_TxW:				; Load Rhw and texture(s)

IF WRAP_TX
	; If wrap we'll probably have to do the perspective correction (since it's per triangle)
	movss		xmmT1, (TLBN ptr [pA]).W			;	0		0		0		vA.w
	movss		xmmVb, (TLBN ptr [pB]).W			;	0		0		0		vB.w
	movss		xmmT2, (TLBN ptr [pC]).W			;	0		0		0		vC.w
	TEST		(KNI_RC ptr [KniRc]).RFBITS, FLAG_PC						
	JE			SHORT TwoPassFullTri_Load_TxW_Wrap_PcDone
	shufps		xmmT1, xmmT1, 0						;	vA.w	vA.w	vA.w	vA.w
	shufps		xmmVb, xmmVb, 0						;	vB.w	vB.w	vB.w	vB.w
	shufps		xmmT2, xmmT2, 0						;	vC.w	vC.w	vC.w	vC.w
	mulps		xmmTxA, xmmT1
	mulps		xmmTxB, xmmVb
	mulps		xmmTxC, xmmT2
TwoPassFullTri_Load_TxW_Wrap_PcDone:
	movlhps		xmmT1, xmmVb						;	vB.w	vB.w	vA.w	vA.w	don't reload w
ELSE ;WRAP_TX
 IF (SEND_TX)
	movss		xmmT1, (TLBN ptr [pA]).W			;	0		0		0		vA.w
	movss		xmmT2, (TLBN ptr [pC]).W			;	0		0		0		vC.w
	movhps		xmmT1, (TLBN ptr [pB]).W_CLIPCODE	;	vB.cc	vB.w	0		vA.w	clipcode is unneeded
  IF (SEND_TX EQ 1)
	movlps		xmmTxA, (TLBN ptr [pA]).TX0
	movlps		xmmTxB, (TLBN ptr [pB]).TX0
	movlps		xmmTxC, (TLBN ptr [pC]).TX0
  ELSEIF (SEND_TX EQ 2)
	movaps		xmmTxA, (TLBN ptr [pA]).TEXTURES
	movaps		xmmTxB, (TLBN ptr [pB]).TEXTURES
	movaps		xmmTxC, (TLBN ptr [pC]).TEXTURES
  ENDIF
 ENDIF
ENDIF ;WRAP_TX

TwoPassFullTri_Load_Color:			; Load the diffuse (and specular if needed) into mmx registers
IF FLAT_SHADED
	movd		mm0, (TLBN ptr [pA]).DIFFUSE   		; 0			vA.diff
	movd		mm3, (TLBN ptr [pA]).SPECULAR  		; 0			vA.spec
	movq		mm1, mm0					   		; 0			vA.diff
	movq		mm2, mm0					   		; 0			vA.diff
	movq		mm4, mm3					   		; 0			vA.spec
	movq		mm5, mm3					   		; 0			vA.spec
ELSE
	movd		mm0, (TLBN ptr [pA]).DIFFUSE		; 0			vA.diff
	movd		mm1, (TLBN ptr [pB]).DIFFUSE		; 0			vB.diff
	movd		mm2, (TLBN ptr [pC]).DIFFUSE		; 0			vC.diff
	movd		mm3, (TLBN ptr [pA]).SPECULAR		; 0			vA.spec
	movd		mm4, (TLBN ptr [pB]).SPECULAR		; 0			vB.spec
	movd		mm5, (TLBN ptr [pC]).SPECULAR		; 0			vC.spec
ENDIF

TwoPassFullTri_Load_XYZWfbi:		; Load x, y, z, and fog
	movhps		xmmVa, (TLBN ptr [pA]).XY			; vA.y		vA.x	----	----
	movlps		xmmVa, (TLBN ptr [pA]).Z_WFBI		; vA.y		vA.x	vA.wfbi	vA.z
	movhps		xmmVb, (TLBN ptr [pB]).XY			; vB.y		vB.x	----	----
	movlps		xmmVb, (TLBN ptr [pB]).Z_WFBI		; vB.y		vB.x	vB.wfbi	vB.z
	movhps		xmmVc, (TLBN ptr [pC]).XY			; vC.y		vC.x	----	----
	movlps		xmmVc, (TLBN ptr [pC]).Z_WFBI		; vC.y		vC.x	vC.wfbi	vC.z

IF ENABLE_AUTOSTRIP
TwoPassFullTri_SaveLastTriPointers:
	;Load the Destination Pointers
	movd		mm7, (KNI_RC ptr [KniRc]).SPECBUFFPTR	; spPtr = edx = pC
	mov			hwPtr, (KNI_RC ptr [KniRc]).CMDFIFOPTR
	mov			eax, (KNI_RC ptr [KniRc]).CMDFIFOROOM

	mov			(KNI_RC ptr [KniRc]).PREVC, pC		; pC is also spPtr
	movd		spPtr, mm7
	mov			(KNI_RC ptr [KniRc]).PREVB, pB
	movq		mm7, mm6			 				; CmdStartSpec	CmdStartDiff
	mov			(KNI_RC ptr [KniRc]).PREVA, pA
	punpckhdq	mm7, mm7			 				; CmdStartSpec	CmdStartSpec
	mov			(KNI_RC ptr [KniRc]).LASTTRITYPE, TRI_TYPE_FAN
ELSE
	mov			hwPtr, (KNI_RC ptr [KniRc]).CMDFIFOPTR
	movq		mm7, mm6			 				; CmdStartSpec	CmdStartDiff
	mov			spPtr, (KNI_RC ptr [KniRc]).SPECBUFFPTR	; spPtr = edx = pC
	punpckhdq	mm7, mm7			 				; CmdStartSpec	CmdStartSpec
	mov			eax, (KNI_RC ptr [KniRc]).CMDFIFOROOM
ENDIF

TwoPassFullTri_SendCommand:
	SET4MMX		CMD_FIFO, mm6				; CmdStartDiff
	SET4MMX		SPEC_BUFFER, mm7			; CmdStartSpec

TwoPassFullTri_SendVertexA:
	SET8XMM_H	BOTH_DESTS, xmmVa		 	; vA.x, vA.y
	SET4MMX		CMD_FIFO, mm0				; vA.diffuse
	SET4MMX		SPEC_BUFFER, mm3			; vA.specular
IF (SEND_Z AND SEND_WFBI)
	SET8XMM_L	BOTH_DESTS, xmmVa			; vA.z, vA.wfbi
ELSEIF (SEND_Z)
	SET4XMM		BOTH_DESTS, xmmVa			; vA.z
ELSEIF (SEND_WFBI)
	shufps		xmmVa, xmmVa, 0e1h
	SET4XMM		BOTH_DESTS, xmmVa			; vA.wfbi
ENDIF
IF HAS_ALPHA
  IF (SEND_TX)
	SET4XMM		BOTH_DESTS, xmmT1			; vA.w
	movhlps		xmmT1, xmmT1				; ----	----	----	vB.w
  ENDIF
  IF (SEND_TX EQ 1)
	SET8XMM_L	BOTH_DESTS, xmmTxA			; vA.Tx0.t, vA.Tx0.s
  ELSEIF (SEND_TX EQ 2)
	SET16XMM	BOTH_DESTS, xmmTxA			; vA.Tx1.t, vA.Tx1.s, vA.Tx0.t, vA.Tx0.s
  ENDIF
ELSE ;HAS_ALPHA
  IF (SEND_TX)
	SET4XMM		CMD_FIFO, xmmT1				; vA.w
	movhlps		xmmT1, xmmT1				; ----	----	----	vB.w
  ENDIF
  IF (SEND_TX EQ 1)
	SET8XMM_L	CMD_FIFO, xmmTxA			; vA.Tx0.t, vA.Tx0.s
  ELSEIF (SEND_TX EQ 2)
	SET16XMM	CMD_FIFO, xmmTxA			; vA.Tx1.t, vA.Tx1.s, vA.Tx0.t, vA.Tx0.s
  ENDIF
ENDIF ;HAS_ALPHA

TwoPassFullTri_SendVertexB:
	movhlps		xmmVa, xmmVb				; vB.x, vB.y	move this because it's almost 
	SET8XMM_L	BOTH_DESTS, xmmVa 			; vB.x, vB.y	always misaligned
	SET4MMX		CMD_FIFO, mm1				; vB.diffuse
	SET4MMX		SPEC_BUFFER, mm4			; vB.specular
IF (SEND_Z AND SEND_WFBI)
	SET8XMM_L	BOTH_DESTS, xmmVb			; vB.z, vB.wfbi
ELSEIF (SEND_Z)
	SET4XMM		BOTH_DESTS, xmmVb			; vB.z
ELSEIF (SEND_WFBI)
	shufps		xmmVb, xmmVb, 0e1h
	SET4XMM		BOTH_DESTS, xmmVb			; vB.wfbi
ENDIF
IF HAS_ALPHA
  IF (SEND_TX)
	SET4XMM		BOTH_DESTS, xmmT1			; vB.w
  ENDIF
  IF (SEND_TX EQ 1)
	SET8XMM_L	BOTH_DESTS, xmmTxB			; vB.Tx0.t, vB.Tx0.s
  ELSEIF (SEND_TX EQ 2)
	SET16XMM	BOTH_DESTS, xmmTxB			; vB.Tx1.t, vB.Tx1.s, vB.Tx0.t, vB.Tx0.s
  ENDIF
ELSE ;HAS_ALPHA
  IF (SEND_TX)
	SET4XMM		CMD_FIFO, xmmT1				; vB.w
  ENDIF
  IF (SEND_TX EQ 1)
	SET8XMM_L	CMD_FIFO, xmmTxB			; vB.Tx0.t, vB.Tx0.s
  ELSEIF (SEND_TX EQ 2)
	SET16XMM	CMD_FIFO, xmmTxB			; vB.Tx1.t, vB.Tx1.s, vB.Tx0.t, vB.Tx0.s
  ENDIF
ENDIF ;HAS_ALPHA

TwoPassFullTri_SendVertexC:	
	SET8XMM_H	BOTH_DESTS, xmmVc			; vC.x, vC.y
	SET4MMX		CMD_FIFO, mm2				; vC.diffuse
	SET4MMX		SPEC_BUFFER, mm5			; vC.specular
IF (SEND_Z AND SEND_WFBI)
	SET8XMM_L	BOTH_DESTS, xmmVc			; vC.z, vC.wfbi
ELSEIF (SEND_Z)
	SET4XMM		BOTH_DESTS, xmmVc			; vC.z
ELSEIF (SEND_WFBI)
	shufps		xmmVc, xmmVc, 0e1h
	SET4XMM		BOTH_DESTS, xmmVc			; vC.wfbi
ENDIF
IF HAS_ALPHA
  IF (SEND_TX)
	SET4XMM		BOTH_DESTS, xmmT2			; vC.w
  ENDIF
  IF (SEND_TX EQ 1)
	SET8XMM_L	BOTH_DESTS, xmmTxC			; vC.Tx0.t, vC.Tx0.s
  ELSEIF (SEND_TX EQ 2)
	SET16XMM	BOTH_DESTS, xmmTxC			; vC.Tx1.t, vC.Tx1.s, vC.Tx0.t, vC.Tx0.s
  ENDIF
ELSE ;HAS_ALPHA
  IF (SEND_TX)
	SET4XMM		CMD_FIFO, xmmT2				; vC.w
  ENDIF
  IF (SEND_TX EQ 1)
	SET8XMM_L	CMD_FIFO, xmmTxC			; vC.Tx0.t, vC.Tx0.s
  ELSEIF (SEND_TX EQ 2)
	SET16XMM	CMD_FIFO, xmmTxC			; vC.Tx1.t, vC.Tx1.s, vC.Tx0.t, vC.Tx0.s
  ENDIF
ENDIF ;HAS_ALPHA

TwoPassFullTri_Cleanup:
; need to keep the pointers on odd 8-byte alignment
IF ((SBUFF_DWORD_OFFSET MOD 8) SHR 2)
	SET4IMM		SPEC_BUFFER, 0				
ENDIF
IF ((HWPTR_DWORD_OFFSET MOD 8) SHR 2)
	SET4IMM		CMD_FIFO, 0
ENDIF
	sub			eax, HWPTR_DWORD_OFFSET
	add			spPtr, SBUFF_DWORD_OFFSET				; COMMENT OUT TO DISABLE SPECULAR
	mov			(KNI_RC ptr [KniRc]).LASTSPECRENDERED, 0ffffffffh
	add			hwPtr, HWPTR_DWORD_OFFSET
	mov			(KNI_RC ptr [KniRc]).CMDFIFOROOM, eax	; update the cmd fifo room
	mov			(KNI_RC ptr [KniRc]).SPECBUFFPTR, spPtr	; update the spec buffer ptr
	mov			(KNI_RC ptr [KniRc]).CMDFIFOPTR, hwPtr	; update the cmd fifo ptr

	; check if we need to dump the specular buffer yet
	cmp			spPtr, (KNI_RC ptr [KniRc]).SPECBUFFTHRESHOLD
	jl			SHORT TwoPassFullTri_SpecBufferOkay
	push		KniRc
	call		_TL_RENDER_DumpSpecBuffer_Asm
	pop			eax
TwoPassFullTri_SpecBufferOkay:

	emms
	mov			ecx, dword ptr _stk_regECX[esp]
	mov			ebx, dword ptr _stk_regEBX[esp]
	mov			esi, dword ptr _stk_regESI[esp]		; KniRc
	mov			edi, dword ptr _stk_regEDI[esp]		; hwPtr
	add			esp, STACK_FRAME_SIZE
	ret
;	Two-Pass Full Triangle Done


;ELSE ;HAS_SPEC_AND_TEXTURES
ELSE ;SEND_SPECULAR
  IF ENABLE_AUTOSTRIP
;******************************************************************************************************************
;	Autostrip check for single-pass triangles
;******************************************************************************************************************
AutostripCheck_NonSpec:
	movd		mm0, (KNI_RC ptr [KniRc]).PREVA				; 0				prevA
	movq		mm1, (KNI_RC ptr [KniRc]).PREVCB 			; prevB			prevC
	movd		mm2, pA										; 0				currA
	movd		mm3, pB										; 0				currB
	punpckldq	mm0, mm1									; prevC			prevA
	punpckldq	mm2, mm2									; currA			currA
	movq		mm4, qword ptr [MaskFanStrip]				; TRI_TYPE_STRIP TRI_TYPE_FAN
	punpckldq	mm3, mm3									; currB			currB
	pcmpeqd		mm0, mm2									; pC=cA?-1:0	pA-cA?-1:0
	pcmpeqd		mm1, mm3									; pB=cB?-1:0	pC-cB?-1:0
	pand		mm0, mm1									; STRIP?-1:0	FAN?-1:0
	pand		mm0, mm4									; STRIP			FAN
	movq		mm1, mm0									; STRIP			FAN
	punpckhdq	mm1, mm1									; STRIP			STRIP
	por			mm0, mm1									; STRIP			FAN|STRIP = currTriType
  ENDIF ;ENABLE_AUTOSTRIP
;ENDIF ;HAS_SPEC_AND_TEXTURES
ENDIF ;SEND_SPECULAR
AutostripCheck_NoSpecFound:
IF ENABLE_AUTOSTRIP
	movd		eax, mm0
	test		eax, eax
	je			OnePassFullTri


;******************************************************************************************************************
;	Single-Pass Partial Triangle
;******************************************************************************************************************
HWPTR_DWORD_OFFSET = 0
SBUFF_DWORD_OFFSET = 0
OnePassPartialTri:
ifdef DEBUG
	inc			dword ptr [_TnlStats_DiffPartailTrisRendered]
endif
	movd		mm1, (KNI_RC ptr [KniRc]).LASTTRITYPE
	movd		mm2, dword ptr [StripModeBit]	 			; 0				STRIP_MODE_BIT
	movd		mm7, (KNI_RC ptr [KniRc]).CMD_CONT.DIFF		; 0				CmdContDiff
	pcmpeqd		mm1, mm0									; (currTriType = lastTriType) ? -1 : 0
	pand		mm1, mm2									; (currTriType = lastTriType) ? stripMode : 0
	por			mm7, mm1									; 0				CmdContDiff

OnePassPartialTri_Load_TxW:				; Load Rhw and texture(s)
IF (SEND_TX)
	movss		xmmT2, (TLBN ptr [pC]).W			; 0		0		0	vC.w
  IF (SEND_TX EQ 1)
	movlps		xmmTxC, (TLBN ptr [pC]).TX0
  ELSEIF (SEND_TX EQ 2)
	movaps		xmmTxC, (TLBN ptr [pC]).TEXTURES
  ENDIF
ENDIF

OnePassPartialTri_Load_Color:			; Load the diffuse into mmx registers
;IF (SEND_SPECULAR AND (NOT HAS_SPEC_AND_TEXTURES))
IF 0
	movd		mm5, (TLBN ptr [pC]).SPECULAR  		; 0			vC.spec.argb
	movd		mm2, (TLBN ptr [pC]).DIFFUSE		; 0			vC.diff.argb
	pcmpeqd		mm6, mm6							; ffffffff	ffffffff
	psrld		mm6, 8								; 00ffffff	00ffffff	(alpha mask)
	pand		mm5, mm6							; 0			vC.spec.rgb
	paddusb		mm2, mm5							; 0			VC.diff.argb + vC.spec.rgb
ELSE
	movd		mm2, (TLBN ptr [pC]).DIFFUSE		; 0			vC.diff.argb
ENDIF

OnePassPartialTri_Load_XYZWfbi:			; Load x, y, z, and fog
	movhps		xmmVc, (TLBN ptr [pC]).XY				; vC.y		vC.x	----	----
	movlps		xmmVc, (TLBN ptr [pC]).Z_WFBI			; vC.y		vC.x	vC.wfbi	vC.z

	;Load the Destination Pointer('s)
	mov			hwPtr, (KNI_RC ptr [KniRc]).CMDFIFOPTR
	mov			eax, (KNI_RC ptr [KniRc]).CMDFIFOROOM

OnePassPartialTri_SaveLastTriPointers:
	mov			(KNI_RC ptr [KniRc]).PREVC, pC
	mov			(KNI_RC ptr [KniRc]).PREVB, pB
	mov			(KNI_RC ptr [KniRc]).PREVA, pA
	movd		(KNI_RC ptr [KniRc]).LASTTRITYPE, mm0

OnePassPartialTri_SendCommand:
	SET4MMX		CMD_FIFO, mm7

OnePassPartialTri_SendVertexC:
	SET8XMM_H	CMD_FIFO, xmmVc			  	; vC.x, vC.y
	SET4MMX		CMD_FIFO, mm2				; vC.diffuse
IF (SEND_Z AND SEND_WFBI)
	SET8XMM_L	CMD_FIFO, xmmVc				; vC.z, vC.wfbi
ELSEIF (SEND_Z)
	SET4XMM		CMD_FIFO, xmmVc				; vC.z
ELSEIF (SEND_WFBI)
	shufps		xmmVc, xmmVc, 0e1h
	SET4XMM		CMD_FIFO, xmmVc				; vC.wfbi
ENDIF
IF (SEND_TX)
	SET4XMM		CMD_FIFO, xmmT2				; vC.w
ENDIF
IF (SEND_TX EQ 1)
	SET8XMM_L	CMD_FIFO, xmmTxC			; vC.Tx0.t, vC.Tx0.s
ELSEIF (SEND_TX EQ 2)
	SET16XMM	CMD_FIFO, xmmTxC			; vC.Tx1.t, vC.Tx1.s, vC.Tx0.t, vC.Tx0.s
ENDIF

OnePassPartialTri_Cleanup:
; need to keep the pointers on odd 8-byte alignment
IF ((HWPTR_DWORD_OFFSET MOD 8) SHR 2)
	SET4IMM		CMD_FIFO, 0
ENDIF
	sub			eax, HWPTR_DWORD_OFFSET
;IF HAS_SPEC_AND_TEXTURES
IF SEND_SPECULAR
	mov			(KNI_RC ptr [KniRc]).LASTSPECRENDERED, 0
ENDIF
	add			hwPtr, HWPTR_DWORD_OFFSET
	mov			(KNI_RC ptr [KniRc]).CMDFIFOROOM, eax	; update the cmd fifo room
	mov			(KNI_RC ptr [KniRc]).CMDFIFOPTR, hwPtr	; update the cmd fifo ptr

	emms
	mov			ecx, dword ptr _stk_regECX[esp]
	mov			ebx, dword ptr _stk_regEBX[esp]
	mov			edi, dword ptr _stk_regEDI[esp]		; hwPtr
	mov			esi, dword ptr _stk_regESI[esp]		; KniRc
	add			esp, STACK_FRAME_SIZE
	ret
ENDIF ;ENABLE_AUTOSTRIP


;******************************************************************************************************************
;	Single-Pass Full Triangle
;******************************************************************************************************************
HWPTR_DWORD_OFFSET = 0
SBUFF_DWORD_OFFSET = 0
Align 16
OnePassFullTri:
ifdef DEBUG
	inc			dword ptr [_TnlStats_DiffFullTrisRendered]
endif
	movd		mm7, (KNI_RC ptr [KniRc]).CMD_START.DIFF	; 0				CmdStartDiff

OnePassFullTri_Load_TxW:				; Load Rhw and texture(s)
IF WRAP_TX
	; If wrap we'll probably have to do the perspective correction (since it's per triangle)
	movss		xmmT1, (TLBN ptr [pA]).W			;	0		0		0		vA.w
	movss		xmmVb, (TLBN ptr [pB]).W			;	0		0		0		vB.w
	movss		xmmT2, (TLBN ptr [pC]).W			;	0		0		0		vC.w
	TEST		(KNI_RC ptr [KniRc]).RFBITS, FLAG_PC
	JE			SHORT OnePassFullTri_Load_TxW_Wrap_PcDone
	shufps		xmmT1, xmmT1, 0						;	vA.w	vA.w	vA.w	vA.w
	shufps		xmmVb, xmmVb, 0						;	vB.w	vB.w	vB.w	vB.w
	shufps		xmmT2, xmmT2, 0						;	vC.w	vC.w	vC.w	vC.w
	mulps		xmmTxA, xmmT1
	mulps		xmmTxB, xmmVb
	mulps		xmmTxC, xmmT2
OnePassFullTri_Load_TxW_Wrap_PcDone:
	movlhps		xmmT1, xmmVb						;	vB.w	vB.w	vA.w	vA.w	don't reload w
ELSE ;WRAP_TX
 IF (SEND_TX)
	movss		xmmT1, (TLBN ptr [pA]).W			;	0		0		0		vA.w
	movss		xmmT2, (TLBN ptr [pC]).W			;	0		0		0		vC.w
	movhps		xmmT1, (TLBN ptr [pB]).W_CLIPCODE	;	vB.cc	vB.w	0		vA.w	clipcode is unneeded
  IF (SEND_TX EQ 1)
	movlps		xmmTxA, (TLBN ptr [pA]).TX0
	movlps		xmmTxB, (TLBN ptr [pB]).TX0
	movlps		xmmTxC, (TLBN ptr [pC]).TX0
  ELSEIF (SEND_TX EQ 2)
	movaps		xmmTxA, (TLBN ptr [pA]).TEXTURES
	movaps		xmmTxB, (TLBN ptr [pB]).TEXTURES
	movaps		xmmTxC, (TLBN ptr [pC]).TEXTURES
  ENDIF
 ENDIF
ENDIF ;WRAP_TX

OnePassFullTri_Load_Color:			; Load the color into mmx registers
IF FLAT_SHADED
;  IF (SEND_SPECULAR AND (NOT HAS_SPEC_AND_TEXTURES))
  IF 0
	movd		mm3, (TLBN ptr [pA]).SPECULAR  		; 0			vA.spec.argb
	movd		mm0, (TLBN ptr [pA]).DIFFUSE		; 0			vA.diff.argb
	pcmpeqd		mm6, mm6							; ffffffff	ffffffff
	psrld		mm6, 8								; 00ffffff	00ffffff	(alpha mask)
	pand		mm3, mm6							; 0			vA.spec.rgb
	paddusb		mm0, mm3							; 0			VA.diff.argb + vA.spec.rgb
  ELSE
	movd		mm0, (TLBN ptr [pA]).DIFFUSE		; 0			vA.diff
  ENDIF
	movq		mm1, mm0							; 0			vA.diff
	movq		mm2, mm0							; 0			vA.diff
ELSE
;  IF (SEND_SPECULAR AND (NOT HAS_SPEC_AND_TEXTURES))
  IF 0
	movd		mm3, (TLBN ptr [pA]).SPECULAR  		; 0			vA.spec.argb
	movd		mm4, (TLBN ptr [pB]).SPECULAR  		; 0			vB.spec.argb
	movd		mm5, (TLBN ptr [pC]).SPECULAR  		; 0			vC.spec.argb
	pcmpeqd		mm6, mm6							; ffffffff	ffffffff
	psrld		mm6, 8								; 00ffffff	00ffffff	(alpha mask)
	movd		mm0, (TLBN ptr [pA]).DIFFUSE		; 0			vA.diff
	movd		mm1, (TLBN ptr [pB]).DIFFUSE		; 0			vB.diff
	movd		mm2, (TLBN ptr [pC]).DIFFUSE		; 0			vC.diff
	pand		mm3, mm6							; 0			vA.spec.rgb
	pand		mm4, mm6							; 0			vB.spec.rgb
	pand		mm5, mm6							; 0			vC.spec.rgb
	paddusb		mm0, mm3							; 0			vA.diff.argb + vA.spec.rgb
	paddusb		mm1, mm4							; 0			vB.diff.argb + vB.spec.rgb
	paddusb		mm2, mm5							; 0			vC.diff.argb + vC.spec.rgb
  ELSE
	movd		mm0, (TLBN ptr [pA]).DIFFUSE		; 0			vA.diff
	movd		mm1, (TLBN ptr [pB]).DIFFUSE		; 0			vB.diff
	movd		mm2, (TLBN ptr [pC]).DIFFUSE		; 0			vC.diff
  ENDIF
ENDIF

	;Load the Destination Pointer('s)
	mov			hwPtr, (KNI_RC ptr [KniRc]).CMDFIFOPTR
	mov			eax, (KNI_RC ptr [KniRc]).CMDFIFOROOM

OnePassFullTri_Load_XYZWfbi:		; Load x, y, z, and fog
	movhps		xmmVa, (TLBN ptr [pA]).XY			; vA.y		vA.x	----	----
	movlps		xmmVa, (TLBN ptr [pA]).Z_WFBI		; vA.y		vA.x	vA.wfbi	vA.z
	movhps		xmmVb, (TLBN ptr [pB]).XY			; vB.y		vB.x	----	----
	movlps		xmmVb, (TLBN ptr [pB]).Z_WFBI		; vB.y		vB.x	vB.wfbi	vB.z
	movhps		xmmVc, (TLBN ptr [pC]).XY			; vC.y		vC.x	----	----
	movlps		xmmVc, (TLBN ptr [pC]).Z_WFBI		; vC.y		vC.x	vC.wfbi	vC.z

OnePassFullTri_SaveLastTriPointers:
IF ENABLE_AUTOSTRIP
	mov			(KNI_RC ptr [KniRc]).PREVC, pC
	mov			(KNI_RC ptr [KniRc]).PREVB, pB
	mov			(KNI_RC ptr [KniRc]).PREVA, pA
	mov			(KNI_RC ptr [KniRc]).LASTTRITYPE, TRI_TYPE_FAN
ENDIF

OnePassFullTri_SendCommand:
	SET4MMX		CMD_FIFO, mm7

OnePassFullTri_SendVertexA:
	SET8XMM_H	CMD_FIFO, xmmVa			   	; vA.x, vA.y
	SET4MMX		CMD_FIFO, mm0			   	; vA.diffuse
IF (SEND_Z AND SEND_WFBI)
	SET8XMM_L	CMD_FIFO, xmmVa				; vA.z, vA.wfbi
ELSEIF (SEND_Z)
	SET4XMM		CMD_FIFO, xmmVa				; vA.z
ELSEIF (SEND_WFBI)
	shufps		xmmVa, xmmVa, 0e1h
	SET4XMM		CMD_FIFO, xmmVa				; vA.wfbi
ENDIF
IF (SEND_TX)
	SET4XMM		CMD_FIFO, xmmT1			   	; vA.w
	movhlps		xmmT1, xmmT1				; vB.cc	vB.w	vB.cc	vB.w
ENDIF
IF (SEND_TX EQ 1)
	SET8XMM_L	CMD_FIFO, xmmTxA			; vA.Tx0.t, vA.Tx0.s
ELSEIF (SEND_TX EQ 2)
	SET16XMM	CMD_FIFO, xmmTxA			; vA.Tx1.t, vA.Tx1.s, vA.Tx0.t, vA.Tx0.s
ENDIF

OnePassFullTri_SendVertexB:
	movhlps		xmmVa, xmmVb				; vB.x, vB.y	move this because it's almost 
	SET8XMM_L	CMD_FIFO, xmmVa 			; vB.x, vB.y	always misaligned
	SET4MMX		CMD_FIFO, mm1				; vB.diffuse
IF (SEND_Z AND SEND_WFBI)
	SET8XMM_L	CMD_FIFO, xmmVb				; vB.z, vB.wfbi
ELSEIF (SEND_Z)
	SET4XMM		CMD_FIFO, xmmVb				; vB.z
ELSEIF (SEND_WFBI)
	shufps		xmmVb, xmmVb, 0e1h
	SET4XMM		CMD_FIFO, xmmVb				; vB.wfbi
ENDIF
IF (SEND_TX)
	SET4XMM		CMD_FIFO, xmmT1				; vB.w
ENDIF
IF (SEND_TX EQ 1)
	SET8XMM_L	CMD_FIFO, xmmTxB			; vB.Tx0.t, vB.Tx0.s
ELSEIF (SEND_TX EQ 2)
	SET16XMM	CMD_FIFO, xmmTxB			; vB.Tx1.t, vB.Tx1.s, vB.Tx0.t, vB.Tx0.s
ENDIF

OnePassFullTri_SendVertexC:	
	SET8XMM_H	CMD_FIFO, xmmVc				; vC.x, vC.y
	SET4MMX		CMD_FIFO, mm2				; vC.diffuse
IF (SEND_Z AND SEND_WFBI)
	SET8XMM_L	CMD_FIFO, xmmVc				; vC.z, vC.wfbi
ELSEIF (SEND_Z)
	SET4XMM		CMD_FIFO, xmmVc				; vC.z
ELSEIF (SEND_WFBI)
	shufps		xmmVc, xmmVc, 0e1h
	SET4XMM		CMD_FIFO, xmmVc				; vC.wfbi
ENDIF
IF (SEND_TX)
	SET4XMM		CMD_FIFO, xmmT2				; vC.w
ENDIF
IF (SEND_TX EQ 1)
	SET8XMM_L	CMD_FIFO, xmmTxC			; vC.Tx0.t, vC.Tx0.s
ELSEIF (SEND_TX EQ 2)
	SET16XMM	CMD_FIFO, xmmTxC			; vC.Tx1.t, vC.Tx1.s, vC.Tx0.t, vC.Tx0.s
ENDIF

OnePassFullTri_Cleanup:
; need to keep the pointers on odd 8-byte alignment
IF ((HWPTR_DWORD_OFFSET MOD 8) SHR 2)
	SET4IMM		CMD_FIFO, 0
ENDIF
	sub			eax, HWPTR_DWORD_OFFSET
;IF HAS_SPEC_AND_TEXTURES
IF SEND_SPECULAR
	mov			(KNI_RC ptr [KniRc]).LASTSPECRENDERED, 0
ENDIF
	add			hwPtr, HWPTR_DWORD_OFFSET
	mov			(KNI_RC ptr [KniRc]).CMDFIFOROOM, eax	; update the cmd fifo room
	mov			(KNI_RC ptr [KniRc]).CMDFIFOPTR, hwPtr	; update the cmd fifo ptr

	emms
	mov			ecx, dword ptr _stk_regECX[esp]
	mov			ebx, dword ptr _stk_regEBX[esp]
	mov			edi, dword ptr _stk_regEDI[esp]		; hwPtr
	mov			esi, dword ptr _stk_regESI[esp]		; KniRc
	add			esp, STACK_FRAME_SIZE
	ret
;	One-Pass Full Triangle Done


ENDM	; TL_RENDER_FUNCTION
;******************************************************************************************************************
;******************************************************************************************************************



Comment ~
##################################################################################################################
##################################################################################################################

	Below are the macro loops that build the render functions and its associated lookup table.   Currently the 
	C function TL_RENDER_Setup() masks out the invalid (weird) rendering requirements and selects the correct 
	asm rendering function.

	The types of triangles not supported by the assembly code are:
		- no color
		- textures without W
		- no textures with W
		- texture #2 without texture #1

	The assembly functions are:
		No textures, any combination of
			- specular
			- flat
			- z
			- fog
			- 16 functions
		1 or 2 textures with no alpha, any combination of
			- wrap
			- specular
			- flat
			- z
			- fog
			- 64 functions
		1 or 2 textures with alpha and specular, any combination of
			- wrap
			- flat
			- z
			- fog
			- 32 functions
		112 unique functions, approx 86K executable
			
	Now for the goofy things:
		- alpha is only important for two pass operations; if it is asserted we must send w and textures
		  on both passes, if it is negated we only send them on the first pass
		- wrap (texture wrap) is only important for textures (duh)
		- W-Buffer is handled upstream of the rendering code, so it's a don't care
		- vertex fog is handled upstream of the rendering code, so it's a don't care
		- perspective correct is only important for wrap (since we have to do the perspective correction), it
		  does not change the number of parameters sent to the hardware so no need for a special case function.

##################################################################################################################
##################################################################################################################
~ ; end comment

ifdef DEBUG
RenderCodeSize_Start::
endif

MAX_FUNCTION_COUNT EQU 256

; This macro loop builds the render functions
FUNCTION_COUNT = 0
UNIQUE_FUNCTIONS = 0
WHILE FUNCTION_COUNT LT MAX_FUNCTION_COUNT
	FUNCTION_COUNT_TXT	TEXTEQU %FUNCTION_COUNT
	UNIQUE_FUNCTION_FLAG = 1
	ENABLE_AUTOSTRIP = 1

	if (FUNCTION_COUNT AND FLAG_TX0)
		if (FUNCTION_COUNT AND FLAG_TX1)
			SEND_TX = 2
			TEXT_TX TEXTEQU		<_Tx2>
		else
			SEND_TX = 1
			TEXT_TX TEXTEQU		<_Tx1>
		endif
	else
		if (FUNCTION_COUNT AND FLAG_TX1)
			SEND_TX = 3		; error
			TEXT_TX TEXTEQU		<_TxErr>
		else
			SEND_TX = 0
			TEXT_TX TEXTEQU		<_NoTx>
		endif
	endif

	if (FUNCTION_COUNT AND FLAG_SPEC)
		SEND_SPECULAR = 1
		TEXT_SPEC	TEXTEQU		<_Spec>
;		if (SEND_TX)
;		  	HAS_SPEC_AND_TEXTURES = 1
;		else
;			HAS_SPEC_AND_TEXTURES = 0
;		endif
	else
		SEND_SPECULAR = 0
		TEXT_SPEC	TEXTEQU		<>
;		HAS_SPEC_AND_TEXTURES = 0
	endif

	if (FUNCTION_COUNT AND FLAG_ALPHA)
;		if (HAS_SPEC_AND_TEXTURES)
		if (SEND_SPECULAR)
			HAS_ALPHA = 1
			TEXT_ALPHA	TEXTEQU		<_Alpha>
		else
			HAS_ALPHA = 0
			TEXT_ALPHA	TEXTEQU		<>
			UNIQUE_FUNCTION_FLAG = 0
		endif
	else
		HAS_ALPHA = 0
		TEXT_ALPHA	TEXTEQU		<>
	endif

	if (FUNCTION_COUNT AND FLAG_WRAP_TX)
		ENABLE_AUTOSTRIP = 0
		if (SEND_TX)
			WRAP_TX = 1
			TEXT_WRAP	TEXTEQU		<_Wrap>
		else
			WRAP_TX = 0
			TEXT_WRAP	TEXTEQU		<>
			UNIQUE_FUNCTION_FLAG = 0
		endif
	else
		WRAP_TX = 0
		TEXT_WRAP	TEXTEQU		<>
	endif

	if (FUNCTION_COUNT AND FLAG_WFBI)
		SEND_WFBI = 1
		TEXT_WFBI	TEXTEQU		<_Wfbi>
	else
		SEND_WFBI = 0
		TEXT_WFBI	TEXTEQU		<>
	endif

	if (FUNCTION_COUNT AND FLAG_FLAT)
		ENABLE_AUTOSTRIP = 0
		FLAT_SHADED = 1
		TEXT_FLAT	TEXTEQU		<_Flat>
	else
		FLAT_SHADED = 0
		TEXT_FLAT	TEXTEQU		<>
	endif

;	SEND_Z = 1		; hard code for now...
;	TEXT_Z	TEXTEQU	<_Z>
	if (FUNCTION_COUNT AND FLAG_Z)
		SEND_Z = 1
		TEXT_Z	TEXTEQU			<_Z>
	else
		SEND_Z = 0
		TEXT_Z	TEXTEQU			<>
	endif

	RENDER_FUNCTION_NAME	TEXTEQU <_TL_RENDER_Asm&TEXT_WRAP&&TEXT_TX&&TEXT_ALPHA&&TEXT_WFBI&&TEXT_Z&&TEXT_FLAT&&TEXT_SPEC&>


	if ((NOT (SEND_TX EQ 3)) AND UNIQUE_FUNCTION_FLAG)
		UNIQUE_FUNCTIONS = UNIQUE_FUNCTIONS + 1
;%echo RENDER_FUNCTION_NAME

		Align 16
		%PUBLIC	RENDER_FUNCTION_NAME
		%RENDER_FUNCTION_NAME PROC NEAR
		TL_RENDER_FUNCTION
		%RENDER_FUNCTION_NAME ENDP
	endif

	FUNCTION_COUNT = FUNCTION_COUNT + 1
ENDM  

ifdef DEBUG
RenderCodeSize_End::

ifdef VERBOSE
TEXT_UNIQUE_FUNCTIONS TEXTEQU %UNIQUE_FUNCTIONS
% echo TEXT_UNIQUE_FUNCTIONS Unique SW T&L Rendering Functions
endif

ifdef VERBOSE
RenderCodeSize = ((offset RenderCodeSize_End) - (offset RenderCodeSize_Start))
RenderCodeSizeTxt TEXTEQU %RenderCodeSize
% echo RenderCodeSizeTxt
endif
endif ;DEBUG

_TEXT   ENDS


; This macro loop builds the lookup table for the render functions
FUNCTION_COUNT = 0
WHILE FUNCTION_COUNT LT MAX_FUNCTION_COUNT
	FUNCTION_COUNT_TXT	TEXTEQU %FUNCTION_COUNT
	UNIQUE_FUNCTION_FLAG = 1


	if (FUNCTION_COUNT AND FLAG_TX0)
		if (FUNCTION_COUNT AND FLAG_TX1)
			SEND_TX = 2
			TEXT_TX TEXTEQU		<_Tx2>
		else
			SEND_TX = 1
			TEXT_TX TEXTEQU		<_Tx1>
		endif
	else
		if (FUNCTION_COUNT AND FLAG_TX1)
			SEND_TX = 3		; error
			TEXT_TX TEXTEQU		<_TxErr>
		else
			SEND_TX = 0
			TEXT_TX TEXTEQU		<_NoTx>
		endif
	endif

	if (FUNCTION_COUNT AND FLAG_SPEC)
		SEND_SPECULAR = 1
		TEXT_SPEC	TEXTEQU		<_Spec>
;		if (SEND_TX)
;		  	HAS_SPEC_AND_TEXTURES = 1
;		else
;			HAS_SPEC_AND_TEXTURES = 0
;		endif
	else
		SEND_SPECULAR = 0
		TEXT_SPEC	TEXTEQU		<>
;		HAS_SPEC_AND_TEXTURES = 0
	endif

	if (FUNCTION_COUNT AND FLAG_ALPHA)
;		if (HAS_SPEC_AND_TEXTURES)
		if (SEND_SPECULAR)
			HAS_ALPHA = 1
			TEXT_ALPHA	TEXTEQU		<_Alpha>
		else
			HAS_ALPHA = 0
			TEXT_ALPHA	TEXTEQU		<>
			UNIQUE_FUNCTION_FLAG = 0
		endif
	else
		HAS_ALPHA = 0
		TEXT_ALPHA	TEXTEQU		<>
	endif

	if (FUNCTION_COUNT AND FLAG_WRAP_TX)
		ENABLE_AUTOSTRIP = 0
		if (SEND_TX)
			WRAP_TX = 1
			TEXT_WRAP	TEXTEQU		<_Wrap>
		else
			WRAP_TX = 0
			TEXT_WRAP	TEXTEQU		<>
			UNIQUE_FUNCTION_FLAG = 0
		endif
	else
		WRAP_TX = 0
		TEXT_WRAP	TEXTEQU		<>
	endif

	if (FUNCTION_COUNT AND FLAG_WFBI)
		SEND_WFBI = 1
		TEXT_WFBI	TEXTEQU		<_Wfbi>
	else
		SEND_WFBI = 0
		TEXT_WFBI	TEXTEQU		<>
	endif

	if (FUNCTION_COUNT AND FLAG_FLAT)
		ENABLE_AUTOSTRIP = 0
		FLAT_SHADED = 1
		TEXT_FLAT	TEXTEQU		<_Flat>
	else
		FLAT_SHADED = 0
		TEXT_FLAT	TEXTEQU		<>
	endif

	if (FUNCTION_COUNT AND FLAG_Z)
		SEND_Z = 1
		TEXT_Z	TEXTEQU			<_Z>
	else
		SEND_Z = 0
		TEXT_Z	TEXTEQU			<>
	endif

	RENDER_FUNCTION_NAME	TEXTEQU <_TL_RENDER_Asm&TEXT_WRAP&&TEXT_TX&&TEXT_ALPHA&&TEXT_WFBI&&TEXT_Z&&TEXT_FLAT&&TEXT_SPEC&>

;%echo RENDER_FUNCTION_NAME

	TEXT_OFFSET TEXTEQU <offset >

	if (NOT (SEND_TX EQ 3))
		TEXT_FUNCT_OFFSET TEXTEQU <(&TEXT_OFFSET&&RENDER_FUNCTION_NAME&)>
;%echo TEXT_FUNCT_OFFSET
	else
		TEXT_FUNCT_OFFSET TEXTEQU <0>
	endif

	%FUNCT_OFFSET = TEXT_FUNCT_OFFSET

	IF (FUNCTION_COUNT EQ 0)
		.CONST
		Align 4
		PUBLIC _TL_RenderFunctionLookupTable
		%_TL_RenderFunctionLookupTable 	dd	TEXT_FUNCT_OFFSET
	ELSE
		%								dd TEXT_FUNCT_OFFSET
	ENDIF


	FUNCTION_COUNT = FUNCTION_COUNT + 1
ENDM  


ENDIF ; TnL_HAL

END

