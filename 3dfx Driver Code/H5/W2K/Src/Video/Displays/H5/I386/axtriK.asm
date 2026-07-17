;*************************************************************
; AXTRIK.ASM
;
; 3DNow! Triangle Rendering Routines, Voodoo3 and Napalm
; Support.
;
; 12/28/99 - MDM - Code Complete
;
;*************************************************************
TITLE          axtriK.asm
.686P
.K3D

ifdef MS_VIEW
.MODEL FLAT
endif

ifdef PERF_MONITORING
extrn __penter:proc
endif

NUM_PREFETCH_TRIS EQU  64
IGX_SPECULAR_FIX EQU 1
SPECIAL_CASES EQU 1
;DOWRAP EQU 1
FALSE		EQU		0
TRUE 		EQU		1
CHECK		EQU		2

; *****************************
; Data section
; *****************************
ifndef MS_VIEW
_DATA SEGMENT PAGE PUBLIC 'DATA'
else
.DATA
endif

public  SpecularStream
ifndef MS_VIEW
align 32
else
align 16
endif
SpecularStream     dd  NUM_PREFETCH_TRIS*12*3 dup(0)  ; Create an array in global data big enough to

ifndef MS_VIEW
align 32
else
align 16
endif
Post_Idx           dd  (((NUM_PREFETCH_TRIS+1)*3)+1) dup (0)

ifndef MS_VIEW
align 32
else
align 16
endif
_AST0              dd 2 dup (0)
_BST0              dd 2 dup (0)
_CST0              dd 2 dup (0)
_AST1              dd 2 dup (0)
_BST1              dd 2 dup (0)
_CST1              dd 2 dup (0)

ifndef MS_VIEW
align 32
else
align 16
endif
_ATEMP             dd 2 dup (0)
_BTEMP			   dd 2 dup (0)
_CTEMP			   dd 2 dup (0)

ifndef MS_VIEW
align 32
else
align 16
endif
_aColor            dd 0
_bColor            dd 0
_cColor            dd 0

ifndef MS_VIEW
align 32
else
align 16
endif
_X_Offset          dd 0
_Y_Offset          dd 0
_Z_Offset          dd 0
_RHW_Offset        dd 0
_Color_Offset      dd 0
_Spec_Offset       dd 0
__T0_Offset        dd 0
_T1_Offset         dd 0


_DATA ENDS

; *****************************
; Extern section
; *****************************
EXTRN    __asm_data:BYTE
EXTRN    _fifo_make_room_asm:NEAR

ifndef MS_VIEW
_TEXT   SEGMENT PAGE PUBLIC USE32 'CODE'
else
_TEXT	SEGMENT
.CODE
endif
        ASSUME  DS:FLAT, SS:FLAT, ES:FLAT

ifdef WINNT
ifdef INCSTBPERF
ifndef MS_VIEW	
include ..\..\..\..\build\stbperf.inc
else
include ..\displays\stbperf.inc	
endif		
endif
endif

INCLUDE aglobal.inc

STACK_SIZE     = 76
; Local stack section
_locals        = 0
_area          = _locals
_hwptr         = _locals + 4
_strip_cont    = _locals + 8
_fan_cont      = _locals + 12
_last          = _locals + 16
_temp1         = _locals + 20
_temp2         = _locals + 24
_temp3         = _locals + 28
_temp4         = _locals + 32
_temp5         = _locals + 36
_specstream    = _locals + 40
_lastspec      = _locals + 44
_specflag      = _locals + 48
_specdwords    = _locals + 52
_localflags    = _locals + 56
_RenderPath    = _locals + 60
_PacketSize    = _locals + 64
_SpecularTriCmd = _locals + 68
_SpecularContCmd = _locals + 72
; Param section
_params        = STACK_SIZE + 24
_ret_addr      = _params - 4
_count         = _params + 0
_idx           = _params + 4
_verts         = _params + 8
; specific
last0    = -12
last1    = -8
last2    = -4

PRIM_TEST=0C00000h

;*************************************************************
; ADJUST_BITS
; 
; Supporting macro to LOAD_LOCAL_FLAGS
; 
;*************************************************************
ADJUST_BITS	MACRO	REG:REQ, START_BIT:REQ, END_BIT:REQ

IF		(START_BIT GT END_BIT)
	shr		REG, (START_BIT - END_BIT)
ELSEIF	(START_BIT LT END_BIT)
	shl		REG, (END_BIT - START_BIT)
ELSE
	; do nothing (they're equal)
ENDIF
ENDM ;ADJUST_BITS

;*************************************************************
; LOAD_LOCAL_FLAGS
; 
; Mirror of KNI code except Specular is determined also by
; specular and if texture != 0
; 
;*************************************************************
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
	mov		ebx, dword ptr [edx	+ OFFSET_RC_PPDEV] 			; *ppdev
	mov		ebx, dword ptr [ebx	+ OFFSET_PDEV_VENDOR_DEVICE_ID] ; ppdev->VendorDeviceID
	cmp		ebx, SST_VENDOR_DEVICE_ID_H5_6					; Napalm or greater?
	setge	cl
	shl		ecx, BIT_NAPALM
	or		eax, ecx
if 1
	; Bad FVF check
	xor     ecx, ecx
	mov     ebx, dword ptr [__asm_data + RHW_OFFSET]
	cmp     ebx, rhw
	setne   cl
	mov     ebx, dword ptr [__asm_data + Z_OFFSET]
	cmp     ebx, sz
	setne   cl
	mov     ebx, dword ptr [__asm_data + COLOR_OFFSET]
	cmp     ebx, color
	setne   cl
	shl     ecx, BIT_BADFVF
	or      eax, ecx
endif
AlphaDone:
	; write flags
	mov		dword ptr _localFlags[esp+STACK_OFFSET], eax
ENDM ;LOAD_LOCAL_FLAGS

;*************************************************************
; BUILD_SPECULAR_COMMANDS
; 
; Chooses the Specular commands once based off alpha blending
; 
;*************************************************************
BUILD_SPECULAR_COMMANDS macro
LOCAL NoSpecular, NoSpecularAlpha
   mov     edx, dword ptr _localflags[esp]
   test    edx, FLAG_SPEC
   je      SHORT NoSpecular

   test    edx, FLAG_ALPHA
   je      SHORT NoSpecularAlpha

; Alpha blending path is broken!
ifdef IGX_SPECULAR_FIX
   mov     edx, dword ptr [__asm_data + CMD_ALPHA_BLEND_SPECULAR]
   mov     dword ptr _SpecularTriCmd[esp], edx
   mov     edx, dword ptr [__asm_data + CMD_CONT_ALPHA_BLEND_SPECULAR]
   mov     dword ptr _SpecularContCmd[esp], edx
endif

   jmp     SHORT NoSpecular

NoSpecularAlpha:

   mov     edx, dword ptr [__asm_data + CMD_NO_ALPHA_BLEND_SPECULAR]
   mov     dword ptr _SpecularTriCmd[esp], edx
   mov     edx, dword ptr [__asm_data + CMD_CONT_NO_ALPHA_BLEND_SPECULAR]
   mov     dword ptr _SpecularContCmd[esp], edx

NoSpecular:
endM

;*************************************************************
; ADJUST_DATA_OFFSETS
; 
; Downloads the offsets of parts of the FVF, and accounts for
; their absence.. I.e. 3DMark 2000 Diffuse Color not being there
; 
;*************************************************************
ADJUST_DATA_OFFSETS MACRO
   ; Force X and Y

   mov     edx, 0
   mov     dword ptr [_X_Offset], edx

   mov     edx, 4
   mov     dword ptr [_Y_Offset], edx

   mov     edx, dword ptr [__asm_data + Z_OFFSET]
   shr     edx, 2
   cmp     edx, dword ptr [__asm_data + FVF_SIZE]
   jg      NoZInFVF

   shl     edx, 2
   mov     dword ptr [_Z_Offset], edx

   jmp     PastZFVF

NoZInFVF:

   mov     edx, 8
   mov     dword ptr [_Z_Offset], edx

PastZFVF:

   mov     edx, dword ptr [__asm_data + RHW_OFFSET]
   shr     edx, 2
   cmp     edx, dword ptr [__asm_data + FVF_SIZE]
   jg      NoRHWInFVF

   shl     edx, 2
   mov     dword ptr [_RHW_Offset], edx

   jmp     PastRHWFVF

NoRHWInFVF:

   mov     edx, 12
   mov     dword ptr [_RHW_Offset], edx

PastRHWFVF:

   mov     edx, dword ptr [__asm_data + COLOR_OFFSET]
   shr     edx, 2
   cmp     edx, dword ptr [__asm_data + FVF_SIZE]
   jg      NoColorInFVF

   shl     edx, 2
   mov     dword ptr [_Color_Offset], edx

   jmp     PastColorFVF

NoColorInFVF:

   mov     edx, 128
   mov     dword ptr [_Color_Offset], edx

PastColorFVF:

   mov     edx, dword ptr [__asm_data + SPECULAR_OFFSET]
   shr     edx, 2
   cmp     edx, dword ptr [__asm_data + FVF_SIZE]
   jg      NoSpecularInFVF

   shl     edx, 2
   mov     dword ptr [_Spec_Offset], edx

   jmp     PastSpecularFVF

NoSpecularInFVF:

   mov     edx, 20
   mov     dword ptr [_Spec_Offset], edx

PastSpecularFVF:

   mov     edx, dword ptr [__asm_data + _T0_OFFSET]
   shr     edx, 2
   cmp     edx, dword ptr [__asm_data + FVF_SIZE]
   jg      NoT0InFVF

   shl     edx, 2
   mov     dword ptr [__T0_Offset], edx

   jmp     PastT0FVF

NoT0InFVF:

   mov     edx, 24
   mov     dword ptr [__T0_Offset], edx

PastT0FVF:

   mov     edx, dword ptr [__asm_data + T1_OFFSET]
   shr     edx, 2
   cmp     edx, dword ptr [__asm_data + FVF_SIZE]
   jg      NoT1InFVF

   shl     edx, 2
   mov     dword ptr [_T1_Offset], edx

   jmp     PastT1FVF

NoT1InFVF:

   mov     edx, 32
   mov     dword ptr [_T1_Offset], edx

PastT1FVF:

ENDM

;*************************************************************
; Specular_Test_And_Build_Tri_Packet
; 
; Specular Triangle packet building macro
; 
;
;*************************************************************
Specular_Test_And_Build_Tri_Packet macro
;******
;Specular Test Begin
;******
; // Specular on Textures
; // specular highlights on texture where the specular color is not black
; if (   (pRc->specular) && (pRc->texture != 0)
;  && (CI_MASKALPHA(pA[FVFO_SPECULAR] | pB[FVFO_SPECULAR] | pC[FVFO_SPECULAR]) != 0))
   mov    edi, 0
   mov    dword ptr _specflag[esp+4], edi

   mov    edx, dword ptr _localflags[esp+4]    
   test   edx, FLAG_SPEC
   je     NoSpecularEnable

; if (pRc->specular)
   mov    edx, dword ptr [__asm_data + _RC]
   mov	  edi, DWORD PTR [edx+OFFSET_RC_SPECULAR_FLAG]
   test   edi, edi
   je     NoSpecularEnable

; if (pRc->texture != 0) 
   mov	  edi, DWORD PTR [edx+OFFSET_RC_TEXTURE_PTR]
   test	  edi, edi
   je     NoSpecularEnable

; && (CI_MASKALPHA(pA[FVFO_SPECULAR] | pB[FVFO_SPECULAR] | pC[FVFO_SPECULAR]) != 0))
   mov    edx, dword ptr [__asm_data + SPECULAR_OFFSET]
   mov    edi, dword ptr [edx + va1]
   or     edi, dword ptr [edx + vb1]
   or     edi, dword ptr [edx + vc1]
   and    edi, 0ffffffh
   test   edi, edi
   je     NoSpecularEnable

   mov    edi, 1
   mov    dword ptr _specflag[esp+4], edi

NoSpecularEnable:
; Save off new specular flag
   mov    edi, dword ptr _specflag[esp+4]
   mov    dword ptr _lastspec[esp+4], edi

SpecularReEntry:
   ;*********************************
   ; Save off data for Specular pass
   mov     dword ptr _temp1[esp+4], esi
   mov     dword ptr _temp2[esp+4], eax
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      NoCMDTRISpecular

   mov     eax, dword ptr _specdwords[esp+4]
   mov     esi, dword ptr _SpecularTriCmd[esp+4]
   mov     dword ptr [SpecularStream + eax], esi
   add     eax, 4
   mov     dword ptr _specdwords[esp+4], eax

NoCMDTRISpecular:
   mov     esi, dword ptr _temp1[esp+4]
   mov     eax, dword ptr _temp2[esp+4]
endM

;*************************************************************
; Specular_Test_And_Build_Autostrip_Packet
; 
; Specular Triangle Autostripped packet building macro
; 
;
;*************************************************************
Specular_Test_And_Build_Autostrip_Packet macro
;******
;Specular Test Begin
;******
; // Specular on Textures
; // specular highlights on texture where the specular color is not black
; if (   (pRc->specular) && (pRc->texture != 0)
;  && (CI_MASKALPHA(pA[FVFO_SPECULAR] | pB[FVFO_SPECULAR] | pC[FVFO_SPECULAR]) != 0))
   mov    edi, 0
   mov    dword ptr _specflag[esp+4], edi

   mov    edx, dword ptr _localflags[esp+4]
   test   edx, FLAG_SPEC
   je     ASNoSpecularEnable

; if (pRc->specular)
   mov    edx, dword ptr [__asm_data + _RC]
   mov	  edi, DWORD PTR [edx+OFFSET_RC_SPECULAR_FLAG]
   test   edi, edi
   je     ASNoSpecularEnable

; if (pRc->texture != 0) 
   mov	  edi, DWORD PTR [edx+OFFSET_RC_TEXTURE_PTR]
   test	  edi, edi
   je     ASNoSpecularEnable

; && (CI_MASKALPHA(pA[FVFO_SPECULAR] | pB[FVFO_SPECULAR] | pC[FVFO_SPECULAR]) != 0))
   mov    edx, dword ptr [__asm_data + SPECULAR_OFFSET]
   mov    edi, dword ptr [edx + va1]
   or     edi, dword ptr [edx + vb1]
   or     edi, dword ptr [edx + vc1]
   and    edi, 0ffffffh
   test   edi, edi
   je     ASNoSpecularEnable

   mov    edi, 1
   mov    dword ptr _specflag[esp+4], edi

ASNoSpecularEnable:
   mov    edi, dword ptr _specflag[esp+4]
   mov    edx, dword ptr _lastspec[esp+4]
   cmp    edx, edi
   je     NoSpecularReentry

   mov    dword ptr _lastspec[esp+4], edi
   mov    edi, dword ptr _temp5[esp+4]
   mov    edx, dword ptr _temp4[esp+4]

   mov    hwptr1, dword ptr _hwptr[esp+4]
   jmp    SpecularReEntry 

NoSpecularReentry:

  ;*********************************
  ; Save off data for Specular pass
   mov     dword ptr _temp1[esp+4], esi
   mov     dword ptr _temp2[esp+4], eax
   mov     dword ptr _temp3[esp+4], ebx
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      ASNoCMDTRISpecular

   mov     ebx, dword ptr _SpecularContCmd[esp+4]

   mov     eax, _last[esp+4]

   mov     edx,esi                         ;tmp1 = test1
   xor     esi,eax
 
   xor     esi,BIT_22

   and     esi,BIT_22
 
   or      ebx, esi                        ;header1 |= test1	 

   mov     eax, dword ptr _specdwords[esp+4]

   mov     dword ptr [SpecularStream + eax], ebx
   add     eax, 4
   mov     dword ptr _specdwords[esp+4], eax

ASNoCMDTRISpecular:
   mov     esi, dword ptr _temp1[esp+4]
   mov     eax, dword ptr _temp2[esp+4]
   mov     ebx, dword ptr _temp3[esp+4]
;***************************
; End Specular Check
;***************************
endM

;*************************************************************
; Autostrip_Tri_To_Strip
; 
; Triangle Autostripping for triangles that share common 
; vertices
;
;*************************************************************
Autostrip_Tri_To_Strip macro
LOCAL NoFan
;******
; Post Render
;******																					
   add     ebp,12                          ;post_idx1 += 12 -- advance to next triangle
   mov     eax, [ebp]                      ;va1 = *post_idx1
   mov     ebx, [ebp+4]                    ;vb1 = *(post_idx+1)
   mov     ecx, [ebp+8]                    ;vc1 = *(post_idx+2)

   mov     edi, [ebp+last1]                ;get prev.vertex1
   mov     edx, [ebp+last2]                ;get prev.vertex2

   mov     _hwptr[esp+4], esi                ;save hwptr1

;********* Autostrippping check Begin ************;
;       look for duplicate vertices
;       we want to make a strip!
;       compare prev.vc1 to curr.va1
;       compare prev.vb1 to curr.vb1
;       if they both match, curr triangle is part of a strip!
   sub     edx, eax                        ;tmp1 -= va1
   sub     edi, ebx                        ;tmp2 -= vb1     
        

   xor     esi, esi                        ;test1 = 0
   or      edi, edx                        ;tmp2 |= tmp1

;       If prev.vc1 == curr.va1 && prev.vb1 == curr.vb1, we found a strip
   jnz     @f
   mov     esi, dword ptr _strip_cont[esp+4]  ;esi = strip header
   jmp     NoFan
@@:
   mov     edx,dword ptr [ebp + last0]     ;tmp1 = prev.vertex0
   mov     edi,dword ptr [ebp + last2]     ;tmp2 = prev.vertex2

;       compare prev.va1 to curr.va1
;       compare prev.vc1 to curr.vb1
;       if they both match, this is part of a fan!
   sub     edx, eax                        ;tmp1 -= va1
   sub     edi, ebx                        ;tmp2 -= vb1

   or      edi, edx                        ;tmp2 |= tmp1

   jnz     @f
   mov     esi, dword ptr _fan_cont[esp+4]   ;esi = fan header

@@:
NoFan:
;********* Autostrippping check END ************;
   mov     edx, dword ptr _verts[esp+4]      ;verts2 = _verts[esp]

   mov     edi,dword ptr _count[esp+4]       ;count1 = _count[esp]

   and     esi, PRIM_TEST                  ;Check to see if we are a valid autostrip

   jz      do_cycle_k6m                    ;If not, lets go back to the single path
endM

;*************************************************************
; Cull 3DNow
; 
; Triangle Culling code for triangles with zero area or
; negative dot products
;
;*************************************************************
Cull_3DNow macro
   movq    mm0, qword ptr sx[va1]     ; mm0: pA->sy | pA->sx

   movq    mm1, qword ptr sx[vb1]     ; mm1: pB->sy | pB->sx

   movq    mm3, qword ptr sx[vc1]     ; mm2: pC->sy | pC->sx
   pfsub   mm0, mm1                   ; mm0: pA->sy - pB->sy | pA->sx - pB->sx

   pfsub   mm1, mm3                   ; mm1: pB->sy - pC->sy | pB->sx - pC->sx

   punpckldq mm3, mm1                 ; mm2: pB->sx - pC->sx | (ni)

ifdef DCT_FIX
   pfmul   mm5, qword ptr [__asm_data + _SCALEWS+8 ]  ; multiply w scale factor with correction.
endif

   punpckhdq mm1, mm3                 ; mm1: pB->sx - pC->sx |  pB->sy - pC->sy
   pcmpeqd mm3, mm3

   pfmul   mm0, mm1                   ; mm0: (pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)
   psllq   mm3, 63

   pxor    mm0, mm3                   ; mm0: -(pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)

   pfacc   mm0, mm0     
   nop

   movd    area1, mm0

   and     area1,80000000h

   xor     area1,dword ptr [__asm_data +_CULL_MASK]

   jz      culled_tri_k6m
endM

;*************************************************************
; Specular Setup Hardware
; 
; Sets up the hardware state after pushing
; specular triangles
;
;*************************************************************
SpecularSetupHardware macro
LOCAL NotNapalm, PastNapalmCheck
;***********************************************
; Execute Specular Stream
;
; // Where FOG() = fog function
; // FOG(T1 + T2) = AlphaFog * FogColor + (1 - AlphaFog)[T1 +T2]
; //    Pass 1    = AlphaFog * FogColor + (1 - AlphaFog)T1
; //    Pass 2    =                       (1 - AlphaFog)T2
; if (pRc->fogEnable)
; {
;   SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0xF ) );
;   SETPD( cmdFifo, ghw->fogMode, (pRc->sst.fogMode & ~SST_FOGMULT) | SST_FOGADD);
   mov     edx, dword ptr [__asm_data + _RC]
   mov     edi, dword ptr [edx+OFFSET_RC_FOG_ENABLE]
   test    edi, edi           ; if (pRc->fogEnable)
   je      FogNotEnabled

;   SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0xF ) );
   mov     dword ptr [hwptr1], CMD_FOG_MODE
   add     hwptr1, 4
   
;   SETPD( cmdFifo, ghw->fogMode, (pRc->sst.fogMode & ~SST_FOGMULT) | SST_FOGADD); 
   mov     edi, dword ptr [edx+OFFSET_RC_FOG_MODE]
   and     edi, (NOT SST_FOGMULT)
   or	   edi, SST_FOGADD
   mov     dword ptr [hwptr1], edi
   add     hwptr1, 4 

FogNotEnabled:

;   SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, alphaMode, 0xF ) );
   mov	   DWORD PTR [hwptr1] , CMD_ALPHA_SPEC_MODE ; 00010219H
   add     hwptr1, 4
   
ifdef IGX_SPECULAR_FIX
; if (!pRc->alphaBlendEnable)
; {
;   SETPD( cmdFifo, ghw->alphaMode, (SST_ENALPHABLEND | (SST_A_ONE << SST_RGBSRCFACT_SHIFT) | (SST_A_ONE << SST_RGBDSTFACT_SHIFT)) );
; }
; else
; {
;   SETPD( cmdFifo, ghw->alphaMode, ((pRc->sst.alphaMode & ~SST_RGBDSTFACT) | (SST_A_ONE << SST_RGBDSTFACT_SHIFT)) );
; }
   mov    edi, dword ptr [edx+OFFSET_RC_ALPHABLEND_ENABLE]
   test   edi, edi			 ;if (!pRc->alphaBlendEnable)
   jne    AlphaBlendEnabled
endif

;   SETPD( cmdFifo, ghw->alphaMode, (SST_ENALPHABLEND | (SST_A_ONE << SST_RGBSRCFACT_SHIFT) | (SST_A_ONE << SST_RGBDSTFACT_SHIFT)) );
   mov    dword ptr [hwptr1], (SST_ENALPHABLEND OR (SST_A_ONE SHL SST_RGBSRCFACT_SHIFT) OR (SST_A_ONE SHL SST_RGBDSTFACT_SHIFT))
   add    hwptr1, 4
   jmp    PastAlphaBlendCheck

ifdef IGX_SPECULAR_FIX
AlphaBlendEnabled:

;   SETPD( cmdFifo, ghw->alphaMode, ((pRc->sst.alphaMode & ~SST_RGBDSTFACT) | (SST_A_ONE << SST_RGBDSTFACT_SHIFT)) );
   mov    edi, dword ptr [edx+OFFSET_RC_ALPHA_MODE]
   and    edi, (NOT SST_RGBDSTFACT)
   or     edi, (SST_A_ONE SHL SST_RGBDSTFACT_SHIFT)
   mov    dword ptr [hwptr1], edi
   add    hwptr1, 4
endif

PastAlphaBlendCheck:

; }
; 
; if ( pRc->zEnable )
; {
;   if( pRc->zWriteEnable )
;   {
;     // if z-buffering then this triangle z values equals the values written on pass 1
;     fbzMode = (pRc->sst.fbzMode & ~(SST_ZFUNC_LT | SST_ZFUNC_GT)) | SST_ZFUNC_EQ;
; 
;     SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzMode, 0xF ) );
;     SETPD( cmdFifo, ghw->fbzMode, fbzMode );
;   }
; }
   mov    edi, dword ptr [edx+OFFSET_RC_Z_ENABLE]
   test   edi, edi	   ;if ( pRc->zEnable )
   je     ZNotEnabled

   mov    edi, dword ptr [edx+OFFSET_RC_ZWRITE_ENABLE]
   test   edi, edi	   ;if( pRc->zWriteEnable )
   je     ZNotEnabled

;     SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzMode, 0xF ) );
   mov    dword ptr [hwptr1], CMD_FBZ_MODE
   add    hwptr1, 4

;     SETPD( cmdFifo, ghw->fbzMode, fbzMode );
   mov    edi, dword ptr [edx+OFFSET_RC_FBZ_MODE]
   and    edi, (NOT (SST_ZFUNC_LT OR SST_ZFUNC_GT))
   or     edi, SST_ZFUNC_EQ
   mov    dword ptr [hwptr1], edi
   add    hwptr1, 4

ZNotEnabled:

ifdef IGX_SPECULAR_FIX
; // texture mapping off and texture blending off but keep alpha the same
; if ( Napalm )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_NAPALM
   je      NotNapalm

; 2/14/00 New Color Combine fixes for single stage rendering for
; tri-linear mip-mapping
ifdef NEW_CCU
;   SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
   mov	   DWORD PTR [hwptr1], CMD_NAPALM_COMBINE_MODE                           ; 00010c11H
   add     hwptr1, 4

; combineModeFBI = ( (SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK | 
;                     SST_CM_DISABLE_CHROMA_SUBSTITUTION | 
;                     SST_CM_USE_COMBINE_MODE ) & pRc->sst.combineModeFBI) |
;                    (pRc->sst.combineModeFBI & (~(SST_CM_CC_OTHERSELECT |SST_CM_CC_LOCALSELECT|
;                     SST_CM_CC_MSELECT_7 |SST_CM_CC_INVERT_OTHER|
;                     SST_CM_CC_INVERT_LOCAL|SST_CM_CC_OUTSHIFT|SST_CM_CC_INVERT_ADD_LOCAL))); 
;
;   SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, combineModeFBI );
   mov     edx, dword ptr [__asm_data + _RC]
   mov	   edi, DWORD PTR [edx+OFFSET_RC_COMBINE_MODE_FBI]                       ; pRc->sst.combineModeFBI

   mov     edx, edi
   and     edx, ( SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK OR \
                  SST_CM_DISABLE_CHROMA_SUBSTITUTION OR \
				  SST_CM_USE_COMBINE_MODE )
   and     edi, ( NOT ( SST_CM_CC_OTHERSELECT OR \
                        SST_CM_CC_LOCALSELECT OR \
						SST_CM_CC_MSELECT_7 OR \
						SST_CM_CC_INVERT_OTHER OR \
						SST_CM_CC_INVERT_LOCAL OR \
						SST_CM_CC_OUTSHIFT OR \
						SST_CM_CC_INVERT_ADD_LOCAL ) )
   or      edi, edx

;   combineModeFBI |= SST_CM_CC_OTHERSELECT_IRGB; //Use iterated color (specular color)
   or      edi, ( SST_CM_CC_OTHERSELECT_IRGB )
   mov     DWORD PTR [hwptr1], edi
   add     hwptr1, 4

; //fbzcolorpath: pass iterated color and leave alpha the same.
;   SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 0, fbzColorPath, 0x1 ) );
   mov     DWORD PTR [hwptr1], ( CMD_FBZ_COLORPATH OR ( 1 SHL 11 ) )
   add     hwptr1, 4

;   fbzColorPath = pRc->sst.fbzColorPath &(~SST_CCOMBINE);
;   SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->fbzColorPath, fbzColorPath );
   mov     edx, dword ptr [__asm_data + _RC]
   mov	   edi, DWORD PTR [edx+OFFSET_RC_FBZ_COLORPATH]                       ; pRc->sst.combineModeFBI
   and     edi, ( NOT ( SST_CCOMBINE ) )
   mov     DWORD PTR [hwptr1], edi
   add     hwptr1, 4
else ; Not NEW_CCU
; combineModeFBI = ( SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK | 
;                    SST_CM_CCA_OTHERSELECT_TA | 
;                    SST_CM_DISABLE_CHROMA_SUBSTITUTION | 
;                    SST_CM_USE_COMBINE_MODE ) & pRc->sst.combineModeFBI; 
; SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
   mov	   DWORD PTR [hwptr1], CMD_NAPALM_COMBINE_MODE                           ; 00010c11H
   mov     edx, dword ptr [__asm_data + _RC]
   mov	   edi, DWORD PTR [edx+OFFSET_RC_COMBINE_MODE_FBI]                       ; pRc->sst.combineModeFBI
   and	   edi, (	SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK OR \
					SST_CM_CCA_OTHERSELECT_TA OR \
					SST_CM_DISABLE_CHROMA_SUBSTITUTION OR \
					SST_CM_USE_COMBINE_MODE )

; SETPD( cmdFifo, ghw->combineMode, combineModeFBI );
   mov     DWORD PTR [hwptr1 + 4], edi
   add     hwptr1, 8
endif ; NEW_CCU

   jmp     PastNapalmCheck

NotNapalm:
; fbzColorPath = (pRc->sst.fbzColorPath & ~(SST_RGBSELECT | SST_CCOMBINE)) | SST_RGBSEL_RGBA;
; 
; SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzColorPath, 0xF ) );
; SETPD( cmdFifo, ghw->fbzColorPath, fbzColorPath );

; SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzColorPath, 0xF ) );
   mov     dword ptr [hwptr1], CMD_FBZ_COLORPATH
   add     hwptr1, 4
		 
; SETPD( cmdFifo, ghw->fbzColorPath, fbzColorPath );
   mov     edx, dword ptr [__asm_data + _RC]
   mov     edi, dword ptr [edx + OFFSET_RC_FBZ_COLORPATH]
   and     edi, (NOT (SST_RGBSELECT OR SST_CCOMBINE))
   or      edi, (SST_RGBSEL_RGBA)
   mov     dword ptr [hwptr1], edi
   add     hwptr1, 4

;    mov  dword ptr _specdwords[esp], edi ; Save off fbzColorPath
PastNapalmCheck:

endif
endM

;*************************************************************
; Specular Restore Hardware
; 
; Restores the hardware state after pushing
; specular triangles
;
;*************************************************************
SpecularRestoreHardware macro
LOCAL NotNapalm, PastNapalmCheck
; Restore the hardware
; if ( Napalm )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_NAPALM
   je      NotNapalm

;   SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
   mov     dword ptr [hwptr1], CMD_NAPALM_COMBINE_MODE
   add     hwptr1, 4

;   SETPD( cmdFifo, ghw->combineMode, pRc->sst.combineModeFBI );
   mov     edx, dword ptr [__asm_data + _RC]
   mov     edi, dword ptr [edx + OFFSET_RC_COMBINE_MODE_FBI]
   mov     dword ptr [hwptr1], edi
   add     hwptr1, 4

; 2/14/00 New Color Combine fixes for single stage rendering for
; tri-linear mip-mapping
ifdef NEW_CCU
;   SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, fbzColorPath,1 ) );
   mov     dword ptr [hwptr1], ( CMD_FBZ_COLORPATH OR ( 1 SHL 11 ) )
   add     hwptr1, 4

;   SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->fbzColorPath, pRc->sst.fbzColorPath );
   mov     edi, dword ptr [edx + OFfSET_RC_FBZ_COLORPATH]
   mov     dword ptr [hwptr1], edi
   add     hwptr1, 4
endif

   jmp     PastNapalmCheck

NotNapalm:
; SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzColorPath, 0xF ) );
   mov     dword ptr [hwptr1], CMD_FBZ_COLORPATH     
   add     hwptr1, 4

; SETPD( cmdFifo, ghw->fbzColorPath, pRc->sst.fbzColorPath );
   mov     edx, dword ptr [__asm_data + _RC]
   mov     edi, dword ptr [edx+OFFSET_RC_FBZ_COLORPATH]
   mov     dword ptr [hwptr1], edi      
   add     hwptr1, 4

PastNapalmCheck:

; if (pRc->fogEnable)
   mov      edx, dword ptr [__asm_data + _RC]
   mov      edi, dword ptr [edx+OFFSET_RC_FOG_ENABLE]
   test     edi, edi	  ;if (pRc->fogEnable)
   je       SSNoFogEnabled

;   SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0xF ) );
   mov   dword ptr [hwptr1], CMD_FOG_MODE
   add   hwptr1, 4

;   SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode );
   mov   edi, dword ptr [edx + OFFSET_RC_FOG_MODE]
   mov   dword ptr [hwptr1], edi
   add   hwptr1, 4

SSNoFogEnabled:

; SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, alphaMode, 0xF ) );
   mov   dword ptr [hwptr1], CMD_ALPHA_DIFF_MODE
   add   hwptr1, 4

; SETPD( cmdFifo, ghw->alphaMode, pRc->sst.alphaMode );
   mov  edi, dword ptr [edx + OFFSET_RC_ALPHA_MODE]
   mov  dword ptr [hwptr1], edi
   add  hwptr1, 4

; SETPD( cmdFifo, ghw->fbzMode, pRc->sst.fbzMode );
   mov   edi, dword ptr [edx + OFFSET_RC_FBZ_MODE]
   mov   dword ptr [hwptr1], edi
   add   hwptr1, 4
endM

;**********************************************************
; Wrap implementation
;
;	if (wrap) {
;		if      ((dAC > 0.5) && (dBC > 0.5))
;			if (C < B)	C += 1.0
;			else		A += 1.0, B += 1.0
;		else if ((dBC > 0.5) && (dAB > 0.5))
;			if (B < A)	B += 1.0
;			else		A += 1.0, C += 1.0
;		else if ((dAB > 0.5) && (dAC > 0.5))
;			if (A < C)	A += 1.0
;			else		B += 1.0, C += 1.0
;	}
;**********************************************************
Wrap MACRO TEX_OFFSET:REQ, AST:REQ, BST:REQ, CST:REQ, WRAP_S_FLAG:REQ, WRAP_T_FLAG:REQ
LOCAL No_El_Er_Sign_S, No_El_Er_S, No_Er_Em_Sign_S, No_Er_Em_S, No_Em_El_Sign_S, No_Em_El_S
LOCAL No_El_Er_Sign_T, No_El_Er_T, No_Er_Em_Sign_T, No_Er_Em_T, No_Em_El_Sign_T, No_Em_El_T
LOCAL NoWrap_S, PastWrap_S, NoWrap_T, PastWrap_T
  pxor      mm1, mm1
  movq      qword ptr [_ATEMP], mm1
  movq      qword ptr [_BTEMP], mm1
  movq      qword ptr [_CTEMP], mm1

  mov       dword ptr _temp3[esp+4], 0bf800000h
  mov       dword ptr _temp4[esp+4], 3f000000h
  mov       dword ptr _temp5[esp+4], 3f800000h

  movq      mm0, qword ptr [va1 + TEX_OFFSET]
  movq      mm1, qword ptr [vb1 + TEX_OFFSET]
  movq      mm2, qword ptr [vc1 + TEX_OFFSET]

  mov       dword ptr _temp1[esp+4], eax

  ; Generate a FFFFFFFF | FFFFFFFF
  pcmpeqd   mm4, mm4
  ; Generate a 7FFFFFFF | 7FFFFFFF
  psrld     mm4, 1

  movq      mm3, mm0

  pfsub     mm3, mm1 ;(mm3_mm1) }                  /* mm3: uv1 - uv2 (em) */
  pfsub     mm1, mm2 ;  __asm {  pfsub(mm1_mm2) }  /* mm1: uv2 - uv3 (er) */
  pfsub     mm2, mm0 ;  __asm {  pfsub(mm2_mm0) }  /* mm2: uv3 - uv1 (el) */

  movq      qword ptr [AST], mm3
  movq      qword ptr [BST], mm1
  movq      qword ptr [CST], mm2

  pand      mm3, mm4 ; ABS of UV & 7FFFFFFF | 7FFFFFFF
  pand      mm1, mm4 ; ABS of UV & 7FFFFFFF | 7FFFFFFF
  pand      mm2, mm4 ; ABS of UV & 7FFFFFFF | 7FFFFFFF

  movd      mm7, dword ptr _temp4[esp+4];const_5}                                    \
  punpckldq mm7, mm7 ;}  /* mm7: 0.5 | 0.5 */                  \

  pfcmpgt   mm3, mm7 ; em > 0.5 | em > 0.5
  pfcmpgt   mm1, mm7 ; er > 0.5 | er > 0.5 
  pfcmpgt   mm2, mm7 ; el > 0.5 | el > 0.5

  movq      mm4, mm3 ; em > 0.5 | em > 0.5

  ; er && em
  ; el && em
  ; el && er
  pand      mm4, mm1 ; er && em  OR  dBC > 0.5 && dAB > 0.5
  pand      mm3, mm2 ; el && em	 OR  dAC > 0.5 && dAB > 0.5
  pand      mm2, mm1 ; el && er	 OR  dAC > 0.5 && dBC > 0.5

  movd      mm1, dword ptr _temp5[esp+4] ; Load 1.0

  mov       eax, dword ptr _localflags [esp+4]
  test      eax, WRAP_S_FLAG
  je        NoWrap_S

  movd      eax, mm2
  test      eax, eax
  je        No_El_Er_S

  mov       eax, dword ptr [BST]
  test      eax, 80000000h
  je        No_El_Er_Sign_S

  ; Insert 1.0 into ATemp and BTemp
  movd      dword ptr [_ATEMP], mm1
  movd      dword ptr [_BTEMP], mm1

  jmp       PastWrap_S
No_El_Er_Sign_S:

  ; Insert 1.0 into CTemp
  movd      dword ptr [_CTEMP], mm1

  jmp       PastWrap_S
No_El_Er_S:

  movd      eax, mm4
  test      eax, eax
  je        No_Er_Em_S

  mov       eax, dword ptr [AST]
  test      eax, 80000000h
  je        No_Er_Em_Sign_S

  ; Insert 1.0 into ATemp and CTemp
  movd      dword ptr [_ATEMP], mm1
  movd      dword ptr [_CTEMP], mm1

  jmp       PastWrap_S
No_Er_Em_Sign_S:

  ; Insert 1.0 into BTemp
  movd      dword ptr [_BTEMP], mm1

  jmp       PastWrap_S
No_Er_Em_S:

  movd      eax, mm3
  test      eax, eax
  je        No_Em_El_S

  mov       eax, dword ptr [CST]
  test      eax, 80000000h
  je        No_Em_El_Sign_S

  ; Insert 1.0 into BTemp and CTemp
  movd      dword ptr [_BTEMP], mm1
  movd      dword ptr [_CTEMP], mm1

  jmp       PastWrap_S

No_Em_El_Sign_S:

  ; Insert 1.0 into ATemp
  movd      dword ptr [_ATEMP], mm1

  jmp       PastWrap_S
No_Em_El_S:

PastWrap_S:  

NoWrap_S:

  mov       eax, dword ptr _localflags [esp+4]
  test      eax, WRAP_T_FLAG
  je        NoWrap_T

  punpckhdq mm2, mm2
  movd      eax, mm2
  test      eax, eax
  je        No_El_Er_T

  mov       eax, dword ptr [BST+4]
  test      eax, 80000000h
  je        No_El_Er_Sign_T

  ; Insert 1.0 into ATemp and BTemp
  movd      dword ptr [_ATEMP+4], mm1
  movd      dword ptr [_BTEMP+4], mm1

  jmp       PastWrap_T
No_El_Er_Sign_T:

  ; Insert 1.0 into CTemp
  movd      dword ptr [_CTEMP+4], mm1

  jmp       PastWrap_T
No_El_Er_T:
  
  punpckhdq mm4, mm4
  movd      eax, mm4
  test      eax, eax
  je        No_Er_Em_T

  mov       eax, dword ptr [AST+4]
  test      eax, 80000000h
  je        No_Er_Em_Sign_T

  ; Insert 1.0 into ATemp and CTemp
  movd      dword ptr [_ATEMP+4], mm1
  movd      dword ptr [_CTEMP+4], mm1

  jmp       PastWrap_T
No_Er_Em_Sign_T:

  ; Insert 1.0 into BTemp
  movd      dword ptr [_BTEMP+4], mm1

  jmp       PastWrap_T
No_Er_Em_T:

  punpckhdq mm3, mm3
  movd      eax, mm3
  test      eax, eax
  je        No_Em_El_T

  mov       eax, dword ptr [CST+4]
  test      eax, 80000000h
  je        No_Em_El_Sign_T

  ; Insert 1.0 into BTemp and CTemp
  movd      dword ptr [_BTEMP+4], mm1
  movd      dword ptr [_CTEMP+4], mm1

  jmp       PastWrap_T

No_Em_El_Sign_T:

  ; Insert 1.0 into ATemp
  movd      dword ptr [_ATEMP+4], mm1

  jmp       PastWrap_T
No_Em_El_T:

PastWrap_T:  

NoWrap_T:

  mov       eax, dword ptr _temp1[esp+4]

  movq      mm0, qword ptr [va1 + TEX_OFFSET]
  movq      mm1, qword ptr [vb1 + TEX_OFFSET]
  movq      mm2, qword ptr [vc1 + TEX_OFFSET]
  
  movq      mm3, qword ptr [_ATEMP]
  movq      mm4, qword ptr [_BTEMP]
  movq      mm5, qword ptr [_CTEMP]
  
  pfadd     mm0, mm3
  pfadd     mm1, mm4
  pfadd     mm2, mm5
  
  movq      qword ptr [AST], mm0
  movq      qword ptr [BST], mm1
  movq      qword ptr [CST], mm2
endM

;*************************************************************
; SELECT_RENDER_PATH
; 
; Based on Setup Flags, determines the rendering path
;
;*************************************************************
SELECT_RENDER_PATH macro
   mov     dword ptr _temp1[esp], edx
   mov     dword ptr _temp2[esp], edi

ifndef SPECIAL_CASES
   jmp     Run_Not_COLOR_Z
endif

   mov     edx, dword ptr _localflags[esp]
   test    edx, FLAG_SPEC
   je      No_XX_Specular

;   jmp     No_XX_Specular

   mov     edx, dword ptr _localflags[esp]
   and     edx, ((FLAG_TX1 OR FLAG_TX0 OR FLAG_ALPHA OR FLAG_PC OR FLAG_W OR \
                  FLAG_VFOG OR FLAG_WFBI OR FLAG_Z OR FLAG_WBUFFER OR \
                  FLAG_COLOR OR FLAG_SPEC OR FLAG_WRAP_TX OR FLAG_BADFVF OR \
                  FLAG_FLAT))
   cmp     edx, ((FLAG_SPEC OR FLAG_COLOR OR FLAG_Z OR FLAG_W OR \
                  FLAG_TX0 OR FLAG_PC))

   jne     Run_Not_SPEC_COLOR_Z_W_PC_TX0

   mov     edx, 24
   mov     dword ptr _PacketSize[esp], edx

   ; 3D Winbench 2000 Test 1 and 2 all non-guardband clipped polygons
   ; 3DMark 2000 Test 2 ( Adventure ) barrells and fountain draw
   ; with this
   mov     edx, _Draw_K62O_SPEC_COLOR_Z_W_PC_TX0
   mov     dword ptr _RenderPath[esp], edx
   jmp     Past_No_XX_Specular

Run_Not_SPEC_COLOR_Z_W_PC_TX0:
   mov     edx, dword ptr _localflags[esp]
   and     edx, ((FLAG_TX1 OR FLAG_TX0 OR FLAG_ALPHA OR FLAG_PC OR FLAG_W OR \
                  FLAG_VFOG OR FLAG_WFBI OR FLAG_Z OR FLAG_WBUFFER OR \
                  FLAG_COLOR OR FLAG_SPEC OR FLAG_WRAP_TX OR FLAG_BADFVF OR \
                  FLAG_FLAT))
   cmp     edx, ((FLAG_SPEC OR FLAG_COLOR OR FLAG_Z OR \
                  FLAG_WFBI OR FLAG_W OR FLAG_TX0 OR FLAG_VFOG OR \
                  FLAG_TX1 OR FLAG_PC OR FLAG_BADFVF))

   jne     Run_Not_SPEC_COLOR_Z_WFBI_VFOG_W_PC_TX0_TX1_BADFVF

   mov     edx, 36
   mov     dword ptr _PacketSize[esp], edx

   ; 3DMark 2000 Test One ( Helicopter ) Mountains are drawn with
   ; this
   mov     edx, _Draw_K62O_SPEC_COLOR_Z_WFBI_VFOG_W_PC_TX0_TX1_BADFVF
   mov     dword ptr _RenderPath[esp], edx
   
   jmp     Past_No_XX_Specular
   
Run_Not_SPEC_COLOR_Z_WFBI_VFOG_W_PC_TX0_TX1_BADFVF:

   mov     edx, dword ptr _localflags[esp]
   and     edx, ((FLAG_TX1 OR FLAG_TX0 OR FLAG_ALPHA OR FLAG_PC OR FLAG_W OR \
                  FLAG_VFOG OR FLAG_WFBI OR FLAG_Z OR FLAG_WBUFFER OR \
                  FLAG_COLOR OR FLAG_SPEC OR FLAG_WRAP_TX OR FLAG_BADFVF OR \
                  FLAG_FLAT))
   cmp     edx, ((FLAG_SPEC OR FLAG_COLOR OR FLAG_Z OR \
                  FLAG_W OR FLAG_TX0 OR FLAG_ALPHA OR FLAG_PC))

   jne     Run_Not_SPEC_COLOR_Z_W_PC_ALPHA_TX0

   mov     edx, 28
   mov     dword ptr _PacketSize[esp], edx

   ; Undefined
   mov     edx, _Draw_K62O_SPEC_COLOR_Z_W_PC_ALPHA_TX0
   mov     dword ptr _RenderPath[esp], edx
   
   jmp     Past_No_XX_Specular
   
Run_Not_SPEC_COLOR_Z_W_PC_ALPHA_TX0:

   mov     edx, dword ptr _localflags[esp]
   and     edx, ((FLAG_TX1 OR FLAG_TX0 OR FLAG_ALPHA OR FLAG_PC OR FLAG_W OR \
                  FLAG_VFOG OR FLAG_WFBI OR FLAG_Z OR FLAG_WBUFFER OR \
                  FLAG_COLOR OR FLAG_SPEC OR FLAG_WRAP_TX OR FLAG_BADFVF OR \
                  FLAG_FLAT))
   cmp     edx, ((FLAG_SPEC OR FLAG_COLOR OR FLAG_Z))

   jne     Run_Not_SPEC_COLOR_Z

   mov     edx, 16
   mov     dword ptr _PacketSize[esp], edx

   ; Undefined
   mov     edx, _Draw_K62O_SPEC_COLOR_Z
   mov     dword ptr _RenderPath[esp], edx
   
   jmp     Past_No_XX_Specular
   
Run_Not_SPEC_COLOR_Z:

   mov     edx, 62
   mov     dword ptr _PacketSize[esp], edx
   
   ; Fallback
   mov     edx, _DrawTriIdx2MainAsm_K62O
   mov     dword ptr _RenderPath[esp], edx

   jmp     Past_No_XX_Specular

No_XX_Specular:

   mov     edx, dword ptr _localflags[esp]
   and     edx, ((FLAG_TX1 OR FLAG_TX0 OR FLAG_ALPHA OR FLAG_PC OR FLAG_W OR \
                  FLAG_VFOG OR FLAG_WFBI OR FLAG_Z OR FLAG_WBUFFER OR \
                  FLAG_COLOR OR FLAG_SPEC OR FLAG_WRAP_TX OR FLAG_BADFVF OR \
                  FLAG_FLAT))
   cmp     edx, ((FLAG_COLOR OR FLAG_Z OR FLAG_W OR FLAG_PC OR FLAG_TX0))
   jne     Run_Not_COLOR_Z_W_PC_TX0

   mov     edx, 24
   mov     dword ptr _PacketSize[esp], edx

   ; 3D Winbench 2000 Tests 3 - 9
   ; Test 3 RustValley - Helicopter
   ; Test 4 Canyon - Space Ship
   ; Test 5 Chamber - Walls and stairs
   ; Test 6 Stations - Domes on stations and second pass triangles
   ; Test 7 Islands ( Fog ) - None
   ; Test 8 Racetrack - All non-guardband clipped triangles
   ; Test 9 Chapel - All non-guardband clipped triangles except
   ;                 the animated leaf pattern
   mov     edx, _Draw_K62O_COLOR_Z_W_PC_TX0
   mov     dword ptr _RenderPath[esp], edx

   jmp     Past_No_XX_Specular

Run_Not_COLOR_Z_W_PC_TX0:

   mov     edx, dword ptr _localflags[esp]
   and     edx, ((FLAG_TX1 OR FLAG_TX0 OR FLAG_ALPHA OR FLAG_PC OR FLAG_W OR \
                  FLAG_VFOG OR FLAG_WFBI OR FLAG_Z OR FLAG_WBUFFER OR \
                  FLAG_COLOR OR FLAG_SPEC OR FLAG_WRAP_TX OR FLAG_BADFVF OR \
                  FLAG_FLAT))
   cmp     edx, ((FLAG_COLOR OR FLAG_Z OR FLAG_W OR FLAG_TX0 OR FLAG_TX1 OR FLAG_PC))
   jne     Run_Not_COLOR_Z_W_PC_TX0_TX1

   mov     edx, 30
   mov     dword ptr _PacketSize[esp], edx

   ; 3D Winbench 2000 Tests 3 - 9
   ; Test 3 RustValley - Ground
   ; Test 4 Canyon - Canyon Walls
   ; Test 5 Chamber - Reflective Floor
   ; Test 6 Stations - Body of the stations ( Currently Fallback Mode 12/20 )
   ; Test 7 Islands ( Fog ) - All triangles not guardband clipped
   ; Test 8 Racetrack - None
   ; Test 9 Chapel - Animated leave pattern behind ball
   mov     edx, _Draw_K62O_COLOR_Z_W_PC_TX0_TX1
   mov     dword ptr _RenderPath[esp], edx

   jmp     Past_No_XX_Specular
   
Run_Not_COLOR_Z_W_PC_TX0_TX1:

   mov     edx, dword ptr _localflags[esp]
   and     edx, ((FLAG_TX1 OR FLAG_TX0 OR FLAG_ALPHA OR FLAG_PC OR FLAG_W OR \
                  FLAG_VFOG OR FLAG_WFBI OR FLAG_Z OR FLAG_WBUFFER OR \
                  FLAG_COLOR OR FLAG_SPEC OR FLAG_WRAP_TX OR FLAG_BADFVF OR \
                  FLAG_FLAT))
   cmp     edx, ((FLAG_COLOR OR FLAG_Z OR FLAG_W OR FLAG_TX0 OR FLAG_TX1 OR FLAG_PC OR \
                  FLAG_BADFVF))
   jne     Run_Not_COLOR_Z_W_PC_TX0_TX1_BADFVF

   mov     edx, 30
   mov     dword ptr _PacketSize[esp], edx

   ; 3DMark 2000 Test 2, Buildings
   mov     edx, _Draw_K62O_COLOR_Z_W_PC_TX0_TX1_BADFVF
   mov     dword ptr _RenderPath[esp], edx

   jmp     Past_No_XX_Specular
   
Run_Not_COLOR_Z_W_PC_TX0_TX1_BADFVF:


   mov     edx, dword ptr _localflags[esp]
   and     edx, ((FLAG_TX1 OR FLAG_TX0 OR FLAG_ALPHA OR FLAG_PC OR FLAG_W OR \
                  FLAG_VFOG OR FLAG_WFBI OR FLAG_Z OR FLAG_WBUFFER OR \
                  FLAG_COLOR OR FLAG_SPEC OR FLAG_WRAP_TX OR FLAG_BADFVF OR \
                  FLAG_FLAT))
   cmp     edx, ((FLAG_COLOR OR FLAG_Z OR FLAG_WFBI OR FLAG_W OR FLAG_VFOG OR \
                 FLAG_TX0 OR FLAG_PC))
   jne     Run_Not_COLOR_Z_WFBI_VFOG_W_PC_TX0

   mov     edx, 24
   mov     dword ptr _PacketSize[esp], edx

   ; 3DMark 2000 Test 1 ( Helicopter ) tanks drawn
   ; with this
   mov     edx, _Draw_K62O_COLOR_Z_WFBI_VFOG_W_PC_TX0
   mov     dword ptr _RenderPath[esp], edx

   jmp     Past_No_XX_Specular

Run_Not_COLOR_Z_WFBI_VFOG_W_PC_TX0:

   mov     edx, dword ptr _localflags[esp]
   and     edx, ((FLAG_TX1 OR FLAG_TX0 OR FLAG_ALPHA OR FLAG_PC OR FLAG_W OR \
                  FLAG_VFOG OR FLAG_WFBI OR FLAG_Z OR FLAG_WBUFFER OR \
                  FLAG_COLOR OR FLAG_SPEC OR FLAG_WRAP_TX OR FLAG_BADFVF OR \
                  FLAG_FLAT))
   cmp     edx, ((FLAG_COLOR OR FLAG_Z OR FLAG_WFBI OR FLAG_W OR FLAG_VFOG OR \
                 FLAG_ALPHA OR FLAG_TX0 OR FLAG_PC))
   jne     Run_Not_COLOR_Z_WFBI_VFOG_W_PC_ALPHA_TX0

   mov     edx, 24
   mov     dword ptr _PacketSize[esp], edx

   ; 3DMark 2000 Test 1 ( Helicopter ) trees drawn
   ; with this
   mov     edx, _Draw_K62O_COLOR_Z_WFBI_VFOG_W_PC_ALPHA_TX0
   mov     dword ptr _RenderPath[esp], edx

   jmp     Past_No_XX_Specular

Run_Not_COLOR_Z_WFBI_VFOG_W_PC_ALPHA_TX0:

   mov     edx, dword ptr _localflags[esp]
   and     edx, ((FLAG_TX1 OR FLAG_TX0 OR FLAG_ALPHA OR FLAG_PC OR FLAG_W OR \
                  FLAG_VFOG OR FLAG_WFBI OR FLAG_Z OR FLAG_WBUFFER OR \
                  FLAG_COLOR OR FLAG_SPEC OR FLAG_WRAP_TX OR FLAG_BADFVF OR \
                  FLAG_FLAT))
   cmp     edx, ((FLAG_COLOR OR FLAG_Z OR FLAG_W OR FLAG_PC OR \ 
                  FLAG_TX0 OR FLAG_TX1 OR FLAG_WRAP_TX))
   jne     Run_Not_COLOR_Z_W_PC_TX0_TX1_WRAP

   mov     edx, 36
   mov     dword ptr _PacketSize[esp], edx

   ; Undefined
   mov     edx, _Draw_K62O_COLOR_Z_W_PC_TX0_TX1_WRAP
   mov     dword ptr _RenderPath[esp], edx

   jmp     Past_No_XX_Specular

Run_Not_COLOR_Z_W_PC_TX0_TX1_WRAP:

   mov     edx, dword ptr _localflags[esp]
   and     edx, ((FLAG_TX1 OR FLAG_TX0 OR FLAG_ALPHA OR FLAG_PC OR FLAG_W OR \
                  FLAG_VFOG OR FLAG_WFBI OR FLAG_Z OR FLAG_WBUFFER OR \
                  FLAG_COLOR OR FLAG_SPEC OR FLAG_WRAP_TX OR FLAG_BADFVF OR \
                  FLAG_FLAT))
   cmp     edx, ((FLAG_COLOR OR FLAG_Z OR FLAG_W OR FLAG_PC OR \ 
                  FLAG_TX0 OR FLAG_WRAP_TX))
   jne     Run_Not_COLOR_Z_W_PC_TX0_WRAP

   mov     edx, 28
   mov     dword ptr _PacketSize[esp], edx

   ; Undefined
   mov     edx, _Draw_K62O_COLOR_Z_W_PC_TX0_WRAP
   mov     dword ptr _RenderPath[esp], edx
;   mov     dword ptr [DrawTriFunction], edx

   jmp     Past_No_XX_Specular

Run_Not_COLOR_Z_W_PC_TX0_WRAP:

   mov     edx, dword ptr _localflags[esp]
   and     edx, ((FLAG_TX1 OR FLAG_TX0 OR FLAG_ALPHA OR FLAG_PC OR FLAG_W OR \
                  FLAG_VFOG OR FLAG_WFBI OR FLAG_Z OR FLAG_WBUFFER OR \
                  FLAG_COLOR OR FLAG_SPEC OR FLAG_WRAP_TX OR FLAG_BADFVF OR \
                  FLAG_FLAT))
   cmp     edx, ((FLAG_COLOR OR FLAG_Z OR FLAG_PC OR FLAG_TX0))
   jne     Run_Not_COLOR_Z_PC_TX0

   mov     edx, 24
   mov     dword ptr _PacketSize[esp], edx

   ; Undefined
   mov     edx, _Draw_K62O_COLOR_Z_PC_TX0
   mov     dword ptr _RenderPath[esp], edx

   jmp     Past_No_XX_Specular

Run_Not_COLOR_Z_PC_TX0:

   mov     edx, dword ptr _localflags[esp]
   and     edx, ((FLAG_TX1 OR FLAG_TX0 OR FLAG_ALPHA OR FLAG_PC OR FLAG_W OR \
                  FLAG_VFOG OR FLAG_WFBI OR FLAG_Z OR FLAG_WBUFFER OR \
                  FLAG_COLOR OR FLAG_SPEC OR FLAG_WRAP_TX OR FLAG_BADFVF OR \
                  FLAG_FLAT))
   cmp     edx, ((FLAG_COLOR OR FLAG_Z OR FLAG_PC))
   jne     Run_Not_COLOR_Z_PC

   mov     edx, 16
   mov     dword ptr _PacketSize[esp], edx

   ; Undefined
   mov     edx, _Draw_K62O_COLOR_Z_PC
   mov     dword ptr _RenderPath[esp], edx
;   mov     dword ptr [DrawTriFunction], edx

   jmp     Past_No_XX_Specular

Run_Not_COLOR_Z_PC:

   mov     edx, dword ptr _localflags[esp]
   and     edx, ((FLAG_TX1 OR FLAG_TX0 OR FLAG_ALPHA OR FLAG_PC OR FLAG_W OR \
                  FLAG_VFOG OR FLAG_WFBI OR FLAG_Z OR FLAG_WBUFFER OR \
                  FLAG_COLOR OR FLAG_SPEC OR FLAG_WRAP_TX OR FLAG_BADFVF OR \
                  FLAG_FLAT))
   cmp     edx, ((FLAG_COLOR OR FLAG_Z))
   jne     Run_Not_COLOR_Z

   mov     edx, 16
   mov     dword ptr _PacketSize[esp], edx

   ; Undefined
   mov     edx, _Draw_K62O_COLOR_Z
   mov     dword ptr _RenderPath[esp], edx

   jmp     Past_No_XX_Specular

Run_Not_COLOR_Z:

   mov     edx, 62
   mov     dword ptr _PacketSize[esp], edx
   
   ; Fallback
   mov     edx, _DrawTriIdx2MainAsm_K62O
   mov     dword ptr _RenderPath[esp], edx

Past_No_XX_Specular:

   mov     edi, dword ptr _temp2[esp]
   mov     edx, dword ptr _temp1[esp]
endM

ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _PrefetchTriIdx2_XX_Asm_K6
_PrefetchTriIdx2_XX_Asm_K6     PROC     NEAR
chunk_size1 TEXTEQU     <eax>
va1        TEXTEQU     <eax>
vb1        TEXTEQU     <ebx>
vc1        TEXTEQU     <ecx>
size1      TEXTEQU     <ecx>
room1      TEXTEQU     <edx>
verts2     TEXTEQU     <edx>
ret_addr1  TEXTEQU     <edx>
count1     TEXTEQU     <edi>
idx1       TEXTEQU     <esi>
hwptr1     TEXTEQU     <esi>
post_idx1  TEXTEQU     <ebp>
STACK_OFFSET = 0					; 0 in the prefetch routines, 4 in the main routine

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_XX_Asm_K6_begin:
   push    esp
   push    ebp
   push    ebx
   push    edi
   push    esi
   sub     esp,STACK_SIZE

   femms

   LOAD_LOCAL_FLAGS	STACK_OFFSET

   SELECT_RENDER_PATH

   BUILD_SPECULAR_COMMANDS

   ADJUST_DATA_OFFSETS
   
Pre_Entry_XX_Asm_K6:

   mov     count1,dword ptr _count[esp]
   mov     chunk_size1,NUM_PREFETCH_TRIS
   sub     chunk_size1,count1
   jl      Tri_Too_Large_XX_Asm_K6

   mov     count1, 0
   mov     dword ptr [__asm_data + _SAVED_COUNT], count1
   mov     count1,dword ptr _count[esp]

   jmp     Past_Tri_Too_Large_XX_Asm_K6

Tri_Too_Large_XX_Asm_K6:
   sub     count1, NUM_PREFETCH_TRIS
   mov     dword ptr [__asm_data + _SAVED_COUNT], count1
   mov     count1, NUM_PREFETCH_TRIS
   mov     dword ptr _count[esp], count1

Past_Tri_Too_Large_XX_Asm_K6:

TriIdx2_XX_reentry:
   mov     idx1,dword ptr _idx[esp]
   lea     post_idx1,Post_Idx ;_post_idx[esp-12]
   sub     post_idx1, 12

   mov     verts2,dword ptr _verts[esp]

fetch_again:
   movzx   va1,word ptr [idx1]

   add     post_idx1,12
   movzx   vb1,word ptr [idx1+2]

   movzx   vc1,word ptr [idx1+4]

   mov     dword ptr _temp1[esp], edi
   mov     edi, dword ptr [__asm_data + FVF_SIZE]
   imul    va1, edi
   imul    vb1, edi
   imul    vc1, edi
   mov     edi, dword ptr _temp1[esp]

  
   lea     va1, dword ptr [verts2 + va1*4]
   lea     vb1, dword ptr [verts2 + vb1*4]

   mov     dword ptr [post_idx1], va1

   mov     dword ptr [post_idx1+4], vb1

   lea     vc1, dword ptr [verts2 + vc1*4]

   mov     dword ptr [post_idx1+8], vc1

   add     idx1, 6

   dec     count1

   jnz     fetch_again

   mov     dword ptr _idx[esp],idx1
   mov     count1,dword ptr _count[esp]
   lea     post_idx1, Post_Idx;dword ptr _post_idx[esp]

   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov     size1,count1

   lea     size1,dword ptr[size1+size1*2]
   mov     va1,dword ptr [post_idx1]

   imul    size1, dword ptr _PacketSize[esp]

   test    dword ptr _localflags[esp], FLAG_SPEC
   je      No_Specular_Packet_Setup

; 2/14/00 New Color Combine fixes for single stage rendering for
; tri-linear mip-mapping
; Make room for the new packet headers
ifdef NEW_CCU   
   add     size1, 36
else
   add     size1, 20
endif

No_Specular_Packet_Setup:

   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]

   inc     size1   ; reserve + 1 for pad dword

   sub     room1,size1
   mov     vb1,dword ptr [post_idx1+4]
   jns     NoNeedToCheckFifo

   push    va1
   push    vb1
   push    count1
   push    size1
   call    _fifo_make_room_asm
   pop     size1
   pop     count1
   pop     vb1
   pop     va1

NoNeedToCheckFifo:
   mov     verts2,dword ptr _verts[esp]
   mov     hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   mov     vc1,dword ptr[post_idx1+8]
   mov     dword ptr _hwptr[esp],hwptr1
   
   ; Fallback
   call    dword ptr _RenderPath[esp]

   mov     edx, dword ptr _temp1[esp]

   mov     count1, dword ptr [__asm_data + _SAVED_COUNT]
   cmp     count1, 0
   jne     TriIdx2_XX_continue

ifndef MS_VIEW
align 32
else
align 16
endif
exit_early:
   mov     eax,dword ptr _idx[esp]
   
   femms

   add     esp,STACK_SIZE
   pop     esi
   pop     edi
   pop     ebx
   pop     ebp
   pop     esp

   ret

ifndef MS_VIEW
align 32
else
align 16
endif
TriIdx2_XX_continue:
   mov     edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov     dword ptr _count[esp],edx; -_params],edx
   jmp     Pre_Entry_XX_Asm_K6 ;_PrefetchTriIdx2_XX_Asm_K6_begin

 
_PrefetchTriIdx2_XX_Asm_K6     ENDP

ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _PrefetchTriIdx2_8_Asm_K6
_PrefetchTriIdx2_8_Asm_K6     PROC     NEAR
chunk_size1 TEXTEQU     <eax>
va1        TEXTEQU     <eax>
vb1        TEXTEQU     <ebx>
vc1        TEXTEQU     <ecx>
size1      TEXTEQU     <ecx>
room1      TEXTEQU     <edx>
verts2     TEXTEQU     <edx>
ret_addr1  TEXTEQU     <edx>
count1     TEXTEQU     <edi>
idx1       TEXTEQU     <esi>
hwptr1     TEXTEQU     <esi>
post_idx1  TEXTEQU     <ebp>
STACK_OFFSET = 0					; 0 in the prefetch routines, 4 in the main routine

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_XX_Asm_K6_begin:
   push    esp
   push    ebp
   push    ebx
   push    edi
   push    esi
   sub     esp,STACK_SIZE

   femms

   LOAD_LOCAL_FLAGS	STACK_OFFSET

   SELECT_RENDER_PATH

   BUILD_SPECULAR_COMMANDS

   ADJUST_DATA_OFFSETS
   
Pre_Entry_XX_Asm_K6:

   mov     count1,dword ptr _count[esp]
   mov     chunk_size1,NUM_PREFETCH_TRIS
   sub     chunk_size1,count1
   jl      Tri_Too_Large_XX_Asm_K6

   mov     count1, 0
   mov     dword ptr [__asm_data + _SAVED_COUNT], count1
   mov     count1,dword ptr _count[esp]

   jmp     Past_Tri_Too_Large_XX_Asm_K6

Tri_Too_Large_XX_Asm_K6:
   sub     count1, NUM_PREFETCH_TRIS
   mov     dword ptr [__asm_data + _SAVED_COUNT], count1
   mov     count1, NUM_PREFETCH_TRIS
   mov     dword ptr _count[esp], count1

Past_Tri_Too_Large_XX_Asm_K6:

TriIdx2_XX_reentry:
   mov     idx1,dword ptr _idx[esp]
   lea     post_idx1, Post_Idx;_post_idx[esp-12]
   sub     post_idx1, 12

   mov     verts2,dword ptr _verts[esp]

fetch_again:
   movzx   va1,word ptr [idx1]

   add     post_idx1,12
   movzx   vb1,word ptr [idx1+2]

   movzx   vc1,word ptr [idx1+4]

   shl     va1, 3
   shl     vb1, 3
   shl     vc1, 3
  
   lea     va1, dword ptr [verts2 + va1*4]
   lea     vb1, dword ptr [verts2 + vb1*4]

   mov     dword ptr [post_idx1], va1

   mov     dword ptr [post_idx1+4], vb1

   lea     vc1, dword ptr [verts2 + vc1*4]

   mov     dword ptr [post_idx1+8], vc1

   add     idx1, 6

   dec     count1

   jnz     fetch_again

   mov     dword ptr _idx[esp],idx1
   mov     count1,dword ptr _count[esp]
   lea     post_idx1, Post_Idx ;dword ptr _post_idx[esp]

   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov     size1,count1

   lea     size1,dword ptr[size1+size1*2]
   mov     va1,dword ptr [post_idx1]

   imul    size1, dword ptr _PacketSize[esp]

   test    dword ptr _localflags[esp], FLAG_SPEC
   je      No_Specular_Packet_Setup
   
; 2/14/00 New Color Combine fixes for single stage rendering for
; tri-linear mip-mapping
; Make room for the new packet headers
ifdef NEW_CCU   
   add     size1, 36
else
   add     size1, 20
endif

No_Specular_Packet_Setup:

   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]

   inc     size1   ; reserve + 1 for pad dword

   sub     room1,size1
   mov     vb1,dword ptr [post_idx1+4]
   jns     NoNeedToCheckFifo

   push    va1
   push    vb1
   push    count1
   push    size1
   call    _fifo_make_room_asm
   pop     size1
   pop     count1
   pop     vb1
   pop     va1

NoNeedToCheckFifo:
   mov     verts2,dword ptr _verts[esp]
   mov     hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   mov     vc1,dword ptr[post_idx1+8]
   mov     dword ptr _hwptr[esp],hwptr1
   
   ; Fallback
   call    dword ptr _RenderPath[esp]

   mov     edx, dword ptr _temp1[esp]

   mov     count1, dword ptr [__asm_data + _SAVED_COUNT]
   cmp     count1, 0
   jne     TriIdx2_XX_continue

ifndef MS_VIEW
align 32
else
align 16
endif
exit_early:
   mov     eax,dword ptr _idx[esp]
   
   femms

   add     esp,STACK_SIZE
   pop     esi
   pop     edi
   pop     ebx
   pop     ebp
   pop     esp

   ret

ifndef MS_VIEW
align 32
else
align 16
endif
TriIdx2_XX_continue:
   mov     edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov     dword ptr _count[esp],edx; -_params],edx
   jmp     Pre_Entry_XX_Asm_K6 ;_PrefetchTriIdx2_XX_Asm_K6_begin
 
_PrefetchTriIdx2_8_Asm_K6     ENDP

ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _PrefetchTriIdx2_10_Asm_K6
_PrefetchTriIdx2_10_Asm_K6     PROC     NEAR
chunk_size1 TEXTEQU     <eax>
va1        TEXTEQU     <eax>
vb1        TEXTEQU     <ebx>
vc1        TEXTEQU     <ecx>
size1      TEXTEQU     <ecx>
room1      TEXTEQU     <edx>
verts2     TEXTEQU     <edx>
ret_addr1  TEXTEQU     <edx>
count1     TEXTEQU     <edi>
idx1       TEXTEQU     <esi>
hwptr1     TEXTEQU     <esi>
post_idx1  TEXTEQU     <ebp>
STACK_OFFSET = 0					; 0 in the prefetch routines, 4 in the main routine
ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_XX_Asm_K6_begin:
   push    esp
   push    ebp
   push    ebx
   push    edi
   push    esi
   sub     esp,STACK_SIZE

   femms

   LOAD_LOCAL_FLAGS	STACK_OFFSET

   SELECT_RENDER_PATH

   BUILD_SPECULAR_COMMANDS

   ADJUST_DATA_OFFSETS
      
Pre_Entry_XX_Asm_K6:

   mov     count1,dword ptr _count[esp]
   mov     chunk_size1,NUM_PREFETCH_TRIS
   sub     chunk_size1,count1
   jl      Tri_Too_Large_XX_Asm_K6

   mov     count1, 0
   mov     dword ptr [__asm_data + _SAVED_COUNT], count1
   mov     count1,dword ptr _count[esp]
   
   jmp     Past_Tri_Too_Large_XX_Asm_K6
   
Tri_Too_Large_XX_Asm_K6:
   sub     count1, NUM_PREFETCH_TRIS
   mov     dword ptr [__asm_data + _SAVED_COUNT], count1
   mov     count1, NUM_PREFETCH_TRIS
   mov     dword ptr _count[esp], count1

Past_Tri_Too_Large_XX_Asm_K6:

TriIdx2_XX_reentry:
   mov     idx1,dword ptr _idx[esp]
   lea     post_idx1, Post_Idx;_post_idx[esp-12]
   sub     post_idx1, 12

   mov     verts2,dword ptr _verts[esp]

fetch_again:
   movzx   va1,word ptr [idx1]
   add     post_idx1,12
   movzx   vb1,word ptr [idx1+2]
   movzx   vc1,word ptr [idx1+4]

   shl     va1,1

   add      idx1,6

   shl     vb1,1

   lea     va1,dword ptr [va1*4 + va1]
   shl     vc1,1

   lea     vb1,dword ptr [vb1*4 + vb1]

   lea     va1, dword ptr [verts2 + va1*4]
   mov     dword ptr [post_idx1],va1

   lea     vc1,dword ptr [vc1*4 + vc1]
   lea     vb1, dword ptr [verts2 + vb1*4]
   mov     dword ptr [post_idx1+4],vb1

   dec     count1
   lea     vc1, dword ptr [verts2 + vc1*4]
   mov     dword ptr [post_idx1+8],vc1

   jnz     fetch_again

   mov     dword ptr _idx[esp],idx1
   mov     count1,dword ptr _count[esp]
   lea     post_idx1, Post_Idx ;dword ptr _post_idx[esp]

   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov     size1,count1

   lea     size1,dword ptr[size1+size1*2]
   mov     va1,dword ptr [post_idx1]

   imul    size1, dword ptr _PacketSize[esp]

   test    dword ptr _localflags[esp], FLAG_SPEC
   je      No_Specular_Packet_Setup
   
; 2/14/00 New Color Combine fixes for single stage rendering for
; tri-linear mip-mapping
; Make room for the new packet headers
ifdef NEW_CCU   
   add     size1, 36
else
   add     size1, 20
endif

No_Specular_Packet_Setup:

   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]

   inc     size1   ; reserve + 1 for pad dword

   sub     room1,size1
   mov     vb1,dword ptr [post_idx1+4]
   jns     NoNeedToCheckFifo

   push    va1
   push    vb1
   push    count1
   push    size1
   call    _fifo_make_room_asm
   pop     size1
   pop     count1
   pop     vb1
   pop     va1

NoNeedToCheckFifo:
   mov     verts2,dword ptr _verts[esp]
   mov     hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   mov     vc1,dword ptr[post_idx1+8]
   mov     dword ptr _hwptr[esp],hwptr1
   
   ; Fallback
   call    dword ptr _RenderPath[esp]

   mov     edx, dword ptr _temp1[esp]

   mov     count1, dword ptr [__asm_data + _SAVED_COUNT]
   cmp     count1, 0
   jne     TriIdx2_XX_continue

ifndef MS_VIEW
align 32
else
align 16
endif
exit_early:
   mov     eax,dword ptr _idx[esp]
   
   femms

   add     esp,STACK_SIZE
   pop     esi
   pop     edi
   pop     ebx
   pop     ebp
   pop     esp

   ret

ifndef MS_VIEW
align 32
else
align 16
endif
TriIdx2_XX_continue:
   mov     edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov     dword ptr _count[esp],edx; -_params],edx
   jmp     Pre_Entry_XX_Asm_K6 ;_PrefetchTriIdx2_XX_Asm_K6_begin
 
_PrefetchTriIdx2_10_Asm_K6     ENDP

ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _PrefetchTriIdx2_12_Asm_K6
_PrefetchTriIdx2_12_Asm_K6     PROC     NEAR
chunk_size1 TEXTEQU     <eax>
va1        TEXTEQU     <eax>
vb1        TEXTEQU     <ebx>
vc1        TEXTEQU     <ecx>
size1      TEXTEQU     <ecx>
room1      TEXTEQU     <edx>
verts2     TEXTEQU     <edx>
ret_addr1  TEXTEQU     <edx>
count1     TEXTEQU     <edi>
idx1       TEXTEQU     <esi>
hwptr1     TEXTEQU     <esi>
post_idx1  TEXTEQU     <ebp>
STACK_OFFSET = 0					; 0 in the prefetch routines, 4 in the main routine

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_XX_Asm_K6_begin:
   push    esp
   push    ebp
   push    ebx
   push    edi
   push    esi
   sub     esp,STACK_SIZE

   femms

   LOAD_LOCAL_FLAGS	STACK_OFFSET

   SELECT_RENDER_PATH

   BUILD_SPECULAR_COMMANDS

   ADJUST_DATA_OFFSETS
   
Pre_Entry_XX_Asm_K6:

   mov     count1,dword ptr _count[esp]
   mov     chunk_size1,NUM_PREFETCH_TRIS
   sub     chunk_size1,count1
   jl      Tri_Too_Large_XX_Asm_K6

   mov     count1, 0
   mov     dword ptr [__asm_data + _SAVED_COUNT], count1
   mov     count1,dword ptr _count[esp]

   jmp     Past_Tri_Too_Large_XX_Asm_K6

Tri_Too_Large_XX_Asm_K6:
   sub     count1, NUM_PREFETCH_TRIS
   mov     dword ptr [__asm_data + _SAVED_COUNT], count1
   mov     count1, NUM_PREFETCH_TRIS
   mov     dword ptr _count[esp], count1

Past_Tri_Too_Large_XX_Asm_K6:

TriIdx2_XX_reentry:
   mov     idx1,dword ptr _idx[esp]
   lea     post_idx1, Post_Idx;_post_idx[esp-12]
   sub     post_idx1, 12

   mov     verts2,dword ptr _verts[esp]

fetch_again:
   movzx   va1,word ptr [idx1]
   add     post_idx1,12
   movzx   vb1,word ptr [idx1+2]
   movzx   vc1,word ptr [idx1+4]

   shl     va1,2

   add      idx1,6

   shl     vb1,2

   lea     va1,dword ptr [va1*2 + va1]
   shl     vc1,2

   lea     vb1,dword ptr [vb1*2 + vb1]

   lea     va1, dword ptr [verts2 + va1*4]
   mov     dword ptr [post_idx1],va1
   
   lea     vc1,dword ptr [vc1*2 + vc1]
   lea     vb1, dword ptr [verts2 + vb1*4]
   mov     dword ptr [post_idx1+4],vb1

   dec     count1
   lea     vc1, dword ptr [verts2 + vc1*4]
   mov     dword ptr [post_idx1+8],vc1

   jnz     fetch_again

   mov     dword ptr _idx[esp],idx1
   mov     count1,dword ptr _count[esp]
   lea     post_idx1, Post_Idx ;dword ptr _post_idx[esp]

   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov     size1,count1

   lea     size1,dword ptr[size1+size1*2]
   mov     va1,dword ptr [post_idx1]

   imul    size1, dword ptr _PacketSize[esp]

   test    dword ptr _localflags[esp], FLAG_SPEC
   je      No_Specular_Packet_Setup
   
; 2/14/00 New Color Combine fixes for single stage rendering for
; tri-linear mip-mapping
; Make room for the new packet headers
ifdef NEW_CCU   
   add     size1, 36
else
   add     size1, 20
endif

No_Specular_Packet_Setup:

   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]

   inc     size1   ; reserve + 1 for pad dword

   sub     room1,size1
   mov     vb1,dword ptr [post_idx1+4]
   jns     NoNeedToCheckFifo

   push    va1
   push    vb1
   push    count1
   push    size1
   call    _fifo_make_room_asm
   pop     size1
   pop     count1
   pop     vb1
   pop     va1

NoNeedToCheckFifo:
   mov     verts2,dword ptr _verts[esp]
   mov     hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   mov     vc1,dword ptr[post_idx1+8]
   mov     dword ptr _hwptr[esp],hwptr1
   
   ; Fallback
   call    dword ptr _RenderPath[esp]

   mov     edx, dword ptr _temp1[esp]

   mov     count1, dword ptr [__asm_data + _SAVED_COUNT]
   cmp     count1, 0
   jne     TriIdx2_XX_continue

ifndef MS_VIEW
align 32
else
align 16
endif
exit_early:
   mov     eax,dword ptr _idx[esp]
   
   femms

   add     esp,STACK_SIZE
   pop     esi
   pop     edi
   pop     ebx
   pop     ebp
   pop     esp

   ret

ifndef MS_VIEW
align 32
else
align 16
endif
TriIdx2_XX_continue:
   mov     edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov     dword ptr _count[esp],edx; -_params],edx
   jmp     Pre_Entry_XX_Asm_K6 ;_PrefetchTriIdx2_XX_Asm_K6_begin
 
_PrefetchTriIdx2_12_Asm_K6     ENDP

ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _DrawTriIdx2MainAsm_K62O
_DrawTriIdx2MainAsm_K62O   PROC     NEAR
va1        TEXTEQU     <eax>
color1     TEXTEQU     <eax>
vb1        TEXTEQU     <ebx>
vc1        TEXTEQU     <ecx>
verts2     TEXTEQU     <edx>
s0         TEXTEQU     <edx>
room2      TEXTEQU     <edx>
count1     TEXTEQU     <edi>
area1      TEXTEQU     <edi>
rhw1       TEXTEQU     <edi>
hwptr1     TEXTEQU     <esi>
post_idx1  TEXTEQU     <ebp>
hwptr_old  TEXTEQU     <ebp>
;ret
;   add     esp, 4

;********************************************************
; Initialize the Specular Stream 
; 
; Specular Stream
; [Number of bytes in stream] 
; [ Data ]
; 
; ~
;
; _specdwords = Number of bytes in stream
; 
; First DWORD In Specular stream is incremented by accessing it
; and rewriting the incremented number of triangles in stream
;

   mov     dword ptr _temp1[esp+4], edx
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_SPEC
   je      NoIntializeSpecularStream

   mov     edx, 0
   mov     dword ptr _specdwords[esp+4], edx

NoIntializeSpecularStream:
   mov     edx, dword ptr _temp1[esp+4]
;********************************************************


   mov     dword ptr _strip_cont[esp+4],BIT_23
   mov     dword ptr _fan_cont[esp+4],BIT_22

   test    hwptr1, 4

   movq    mm5, qword ptr [__asm_data + _SCALE_S]

   movd    mm6, dword ptr [__asm_data + _PIXEL_CENTER]

   movq    mm7, qword ptr [__asm_data + _TEXEL_OFFSET_S]

ifdef DCT_FIX
   pfmul   mm7, qword ptr [__asm_data + _SCALEWS+8 ]       ; Pre multiply S& T scale factors with correction.
endif

   punpckldq mm6, mm6                                      ; Pixel Center | Pixel Center

   jz      @F

   add     hwptr1, 4
   mov     dword ptr [hwptr1-4], 0                         ; Quad Word Align the Fifo
@@:
   mov     dword ptr _hwptr[esp+4], hwptr1

ifndef MS_VIEW
align 32
else
align 16
endif
do_cycle_k6m:

   mov     hwptr1, dword ptr _hwptr[esp+4]

   dec     count1

   js      done_render_k6m                                 ; If we hit sign, we are done going through the list

   mov     dword ptr _count[esp+4], count1

;********************************************************
; Culling Check
;********************************************************
   Cull_3DNow
;********************************************************
; Culling Check
;********************************************************

;********************************************************
; Specular Check
;********************************************************
; Disable specular and let the tri packet checking mechanism enable it
   mov     dword ptr _specflag[esp+4], 0

   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_SPEC
   je      NoBuildTriPacketSpecular

   Specular_Test_And_Build_Tri_Packet

NoBuildTriPacketSpecular:
;********************************************************
; Specular Check
;********************************************************

;********************************************************
; Wrap Implementation
;********************************************************
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_TX0
   je      NoWrapT0

   mov     edx, dword ptr [__T0_Offset]
   Wrap    edx, _AST0, _BST0, _CST0, (1 SHL BIT_WRAP_T0_S), (1 SHL BIT_WRAP_T0_T)
NoWrapT0:

   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_TX1
   je      NoWrapT1

   mov     edx, dword ptr [_T1_Offset]
   Wrap    edx, _AST1, _BST1, _CST1, (1 SHL BIT_WRAP_T1_S), (1 SHL BIT_WRAP_T1_T)
NoWrapT1:

   movq    mm5, qword ptr [__asm_data + _SCALE_S]
   movd    mm6, dword ptr [__asm_data + _PIXEL_CENTER]

   movq    mm7, qword ptr [__asm_data + _TEXEL_OFFSET_S]

ifdef DCT_FIX
   pfmul   mm7, qword ptr [__asm_data + _SCALEWS+8 ]       ; Pre multiply S& T scale factors with correction.
endif

   punpckldq mm6, mm6
;********************************************************
; Wrap Implementation
;********************************************************

;********************************************************
; Send down Triangle Header Packet
;********************************************************
;  movd    mm0, dword ptr [__asm_data + CMD_TRI]
   movd    mm0, dword ptr [__asm_data + CMD_START]
   movd    dword ptr [hwptr1], mm0					       ; Write Header
   add     hwptr1, 4

;********************************************************
; Color Precalc!
;
; if ( FLAG_COLOR )
;   if ( Specular Enabled && No Texture )
;     CLAMP888(aColor, pA[FVFO_COLOR], pA[FVFO_SPECULAR]);
;     if ( Flat Shading Enabled )
;	    bColor = cColor = aColor
;	  else 
;	    CLAMP888(bColor, pB[FVFO_COLOR], pB[FVFO_SPECULAR]);
;		CLAMP888(cColor, pC[FVFO_COLOR], pC[FVFO_SPECULAR]);
;   else
;      aColor = va1 color
;	  if ( Flat SHading Enabled )
;	    bColor = cColor = aColor
;	  else
;	    bColor = vb1 color
;		cColor = vc1 color
;
; CLAMP888(xColor, pX[FVFO_COLOR], pX[FVFO_SPECULAR]);
;
; If the diffuse color index == 128, then xColor = FFFFFFFFh
; 
; Add the Specular to the Diffuse Color without
; Overflow into the higher packed bytes
;********************************************************
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_COLOR
   je      PrecalcNoColor

   test    edx, FLAG_SPEC
   je      PrecalcNoSpecularEnabled

; if ( pRc->specular )
   mov     edx, dword ptr [__asm_data + _RC]
   mov     edi, dword ptr [edx + OFFSET_RC_SPECULAR_FLAG]
   test    edi, edi
   je      PrecalcNoSpecularEnabled
   
; if ( pRc->texture == 0 )
   mov     edi, dword ptr [edx + OFFSET_RC_TEXTURE_PTR]
   test    edi, edi
   jne     PrecalcNoSPecularEnabled

   mov     edi, 0fefefeh

   movd      mm4, edi
   punpckldq mm4, mm4

   mov        edx, dword ptr [_Color_Offset]
   cmp        edx, 128
   je         PrecalcNoColorInFVF
    
   movd       mm1, dword ptr [va1 + edx]
   movd       mm0, dword ptr [vb1 + edx]
   punpckldq  mm1, mm0
   
   jmp        PrecalcPastNoColorInFVF

PrecalcNoColorInFVF:
   pcmpeqd    mm1, mm1
   pcmpeqd    mm0, mm0

PrecalcPastNoColorInFVF:

   mov     edi, dword ptr [_Spec_Offset]

   movd    mm0, dword ptr [edi + va1]
   movd    mm2, dword ptr [edi + vb1]
   movd    mm3, dword ptr [edi + vc1]
   
   punpckldq  mm0, mm2
   pand       mm0, mm4
   paddusb    mm1, mm0

   movd    mm0, dword ptr [vc1 + edx]

   movd    dword ptr [_aColor], mm1

   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_FLAT
   je      PrecalcSpecularNoFlatShading

   movd    dword ptr [_bColor], mm1
   movd    dword ptr [_cColor], mm1
   
   jmp     PrecalcNoColor

PrecalcSpecularNoFlatShading:
   pand    mm3, mm4

   paddusb mm0, mm3

   punpckhdq mm1, mm1
   movd    dword ptr [_bColor], mm1
   movd    dword ptr [_cColor], mm0
   
   jmp     PrecalcNoColor

PrecalcNoSpecularEnabled:
   mov     edx, dword ptr [_Color_Offset]
   cmp     edx, 128
   je      PrecalcNoSpecNoColorInFVF

   movd    mm0, dword ptr [va1 + edx]

   jmp     PrecalcNoSpecPastNoColorInFVF

PrecalcNoSpecNoColorInFVF:
   pcmpeqd mm0, mm0

PrecalcNoSpecPastNoColorInFVF:

   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_FLAT
   je      PrecalcNoSpecularNoFlatShading

   movd    dword ptr [_aColor], mm0
   movd    dword ptr [_bColor], mm0
   movd    dword ptr [_cColor], mm0

   jmp     PrecalcNoColor

PrecalcNoSpecularNoFlatShading:
   mov     edx, dword ptr [_Color_Offset]
   cmp     edx, 128
   je      PrecalcNoSpecNoFlatNoColorInFVF

   movd    mm1, dword ptr [vb1 + edx]
   movd    mm2, dword ptr [vc1 + edx]

   jmp     PrecalcNoSpecNoFlatPastNoColorInFVF

PrecalcNoSpecNoFlatNoColorInFVF:

   pcmpeqd mm1, mm1
   pcmpeqd mm2, mm2

PrecalcNoSpecNoFlatPastNoColorInFVF:

   movd    dword ptr [_aColor], mm0
   movd    dword ptr [_bColor], mm1
   movd    dword ptr [_cColor], mm2
   
PrecalcNoColor:
;********************************************************
; Color Precalc!
;********************************************************

;********************************************************
; Vert1
;
; mm0 - X | Y, shared with Wfbi later
; mm1 - RGBA Color, shared with ST1 later
; mm2 - Z
; mm3 - RHW or W0
; mm4 - ST0
; mm5 - Scale T | Scale S
; mm6 - Pixel Center | Pixel Center
; mm7 - Texel Offset | Texel Offset
;
;********************************************************
   mov     edx, dword ptr [_X_Offset]
   movq    mm0, qword ptr [va1 + edx]

   movd    mm1, dword ptr [_aColor]

   pfadd   mm0, mm6                                        ; Y + PIXEL_OFFSET | 
                                                           ; X + PIXEL_OFFSET

   mov     edx, dword ptr [_Z_Offset]
   movd    mm2, dword ptr [va1 + edx]

   movq    qword ptr [hwptr1], mm0                         ; Write X + Y
   add     hwptr1, 8

;********************************************************
; Save off data for specular
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      ANoXYSpecular

   mov     edi, dword ptr _specdwords[esp+4]
   movq    qword ptr [SpecularStream + edi], mm0

   mov     edi, dword ptr [_Spec_Offset]
   mov     edx, dword ptr [edi + va1]

ifdef IGX_SPECULAR_FIX
   movd    edi, mm1
   xor     edx, edi
   and     edx, 16777215
   xor     edx, edi
   movd    mm0, edx
endif

   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi + 8], mm0
   add     edi, 12
   mov     dword ptr _specdwords[esp+4], edi

ANoXYSpecular:
;********************************************************

   mov     edx, dword ptr [_RHW_Offset]
   movd    mm3, dword ptr [va1 + edx]

   movq    mm4, qword ptr [_AST0] ; Wrap Code
;   movq    mm4, qword ptr [vc1 + s0]

   punpckldq mm3, mm3                                      ; pB->rhw | pB->rhw

ifdef DCT_FIX
   pfmul   mm3, qword ptr [__asm_data + _SCALEWS+8 ]       ; multiply w scale factor with correction.
endif

   pfmul   mm4, mm5                                        ; T * SCALE_T | S * SCALE_S

;********************************************************
; COLOR
;********************************************************
;  if ( setupFlag & ( SST_SETUP_RGB | SST_SETUP_A ) )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_COLOR
   je      SHORT ANoColor
           
   movd    dword ptr [hwptr1], mm1	                       ; Write Color
   add     hwptr1, 4

ANoColor:
;********************************************************
; COLOR
;********************************************************

;********************************************************
; Z Setup
; 
; if( pRc->state & STATE_REQUIRES_WBUFFER )
;   if (pRc->state & STATE_REQUIRES_VERTEXFOG)
;     zX = (float)((255 - RGBA_GETALPHA(pX[FVFO_SPECULAR])) << 8);
;   else
;     zX = ZSCALE ( FLTP(pX)[FVFO_SZ] );
; else
;   Send down Z the way we got it
;
;********************************************************
;  if ( setupFlag & SST_SETUP_Z )
   test    edx, FLAG_Z
   je      SHORT ANoZ

;  if( pRc->state & STATE_REQUIRES_WBUFFER )
   test    edx, FLAG_WBUFFER
   je      SHORT ANoZWBuffer

;  if (pRc->state & STATE_REQUIRES_VERTEXFOG)
   test    edx, FLAG_VFOG
   je      SHORT ASendZ

;********************************************************
; Z-FOG Implementation
;
; (255 - RGBA_GETALPHA(pX[FVFO_SPECULAR])) << 8;
;********************************************************
   xor     edx, edx
   mov     edi, dword ptr [_Spec_Offset]
   mov     dl, byte ptr [edi + va1 + 3]
   not     dl
   shl     edx, 8
   movd    mm2, edx
   pi2fd   mm2, mm2
   jmp     SHORT ASendZ
;********************************************************
; Z-FOG Implementation
;********************************************************

ANoZWBuffer:

   pfmul   mm2, qword ptr [__asm_data + _SCALEZ]           ; 0 | Z * SCALEZ

ASendZ:

   movd    dword ptr [hwptr1], mm2                         ; Write Z
   add     hwptr1, 4

;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      ANoZSpecular

   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi], mm2
   add     edi, 4
   mov     dword ptr _specdwords[esp+4], edi

ANoZSpecular:
;********************************************************

ANoZ:
;********************************************************
; Z Setup
;********************************************************

;********************************************************
; Wfbi Setup
;
; if( pRc->state & STATE_REQUIRES_WBUFFER )
;   wX = WSCALE( FLTP(pX)[FVFO_RHW] );
; else
;   if (pRc->state & STATE_REQUIRES_VERTEXFOG)
;     wX = (float)(255 - RGBA_GETALPHA(pX[FVFO_SPECULAR]));
;	else if (pRc->state & STATE_REQUIRES_HWFOG)
;     wX = RHW
;
;********************************************************
;  if ( setupFlag & SST_SETUP_Wfbi )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_WFBI
   je      SHORT ANoWfbi

;  if( pRc->state & STATE_REQUIRES_WBUFFER )
   test    edx, FLAG_WBUFFER
   je      SHORT ANoWWBuffer

   movq    mm0, mm3
   pfmul   mm0, qword ptr [__asm_data + WBUFFER_SCALE_AW]  ; RHW * WBUFFER_SCALE_AW 
   pfadd   mm0, qword ptr [__asm_data + WBUFFER_SCALE_BW]  ; (RHW * WBUFFER_SCALE_AW) + WBUFFER_SCALE_BW
   jmp     SHORT ASendW
;#define WSCALE(a) ((a * pRc->aW) + pRc->bW)
 
ANoWWBuffer:
;  if (pRc->state & STATE_REQUIRES_VERTEXFOG)
   test    edx, FLAG_VFOG
   je      SHORT ANoWVertexFog

;********************************************************
; W-FOG Implementation
;
; 255 - RGBA_GETALPHA(pX[FVFO_SPECULAR]);
;********************************************************
   xor     edx, edx
   mov     edi, dword ptr [_Spec_Offset]
   mov     dl, byte ptr [edi + va1 + 3]
   not     dl
   movd    mm0, edx
   pi2fd   mm0, mm0
   jmp     SHORT ASendW
;   (rgb) >> 24
;********************************************************
; W-FOG Implementation
;********************************************************

ANoWVertexFog:
;  if (pRc->state & STATE_REQUIRES_HWFOG)
   mov     edx, dword ptr [__asm_data + RENDER_STATE]
   test    dh, 32
   je      SHORT ASendW

   movq    mm0, mm3                                        ; Load up RHW

ASendW:

   movd    dword ptr [hwptr1], mm0                         ; Write Wfbi
   add     hwptr1, 4

;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      ANoWfbiSpecular

   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi], mm0
   add     edi, 4
   mov     dword ptr _specdwords[esp+4], edi

ANoWfbiSpecular:  
;********************************************************

ANoWfbi:
;********************************************************
; Wfbi Setup
;********************************************************

;********************************************************
; W0 Setup
;********************************************************
;  if ( setupFlag & SST_SETUP_W0 )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_W
   je      SHORT ANoW0

   movd    dword ptr [hwptr1], mm3                         ; Write W
   add     hwptr1, 4

ifdef IGX_SPECULAR_FIX
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      ANoW0Specular

   mov     edx, dword ptr [__asm_data + _RC]
   mov     edx, dword ptr [edx+OFFSET_RC_ALPHABLEND_ENABLE]
   test    edx, edx
   je      ANoW0Specular

   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi], mm3
   add     edi, 4
   mov     dword ptr _specdwords[esp+4], edi

ANoW0Specular:
;********************************************************
endif

ANoW0:
;********************************************************
; W0 Setup
;********************************************************

;********************************************************
; ST0 Setup
;********************************************************
;  if ( setupFlag & SST_SETUP_ST0 )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_TX0
   je      SHORT ANoST0

   pfadd   mm4, mm7                                        ; (T * SCALE_T) + T_OFFSET |
                                                           ; (S * SCALE_S) + S_OFFSET

;  if ( pRc->state & STATE_REQUIRES_PERSPECTIVE )
   test    edx, FLAG_PC
   je      SHORT ANoPerspTextureST0

   pfmul   mm4, mm3                                        ; ((T * SCALE_T) + T_OFFSET) * RHW |
                                                           ; ((S * SCALE_S) + S_OFFSET) * RHW

ANoPerspTextureST0:

   movq    qword ptr [hwptr1], mm4                         ; Write TU1 TV1
   add     hwptr1, 8

ifdef IGX_SPECULAR_FIX
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      ANoST0Specular

   mov     edx, dword ptr [__asm_data + _RC]
   mov     edx, dword ptr [edx+OFFSET_RC_ALPHABLEND_ENABLE]
   test    edx, edx
   je      ANoST0Specular

   mov     edi, dword ptr _specdwords[esp+4]
   movq    qword ptr [SpecularStream + edi], mm4
   add     edi, 8
   mov     dword ptr _specdwords[esp+4], edi

ANoST0Specular:
;********************************************************
endif

ANoST0:
;********************************************************
; ST0 Setup 
;********************************************************

;********************************************************
;ST1 Setup
;********************************************************
;  if ( setupFlag & SST_SETUP_ST1 )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_TX1
   je      SHORT ANoST1

   movq    mm1, qword ptr [_AST1]
;   mov     edx, dword ptr [__asm_data + T1_OFFSET]         
;   movq    mm1, qword ptr [va1 + edx]					   ; T | S

   pfmul   mm1, qword ptr [__asm_data + S1_SCALE]          ; T * SCALE_T | S * SCALE_S
   pfadd   mm1, qword ptr [__asm_data + S1_TEXEL_OFFSET]   ; ((T * SCALE_T) + T_OFFSET) |
                                                           ; ((S * SCALE_S) + S_OFFSET)

;  if ( pRc->state & STATE_REQUIRES_PERSPECTIVE )
   test    edx, FLAG_PC
   je      SHORT ANoPerspTextureST1

   pfmul   mm1, mm3                                        ; ((T * SCALE_T) + T_OFFSET) * RHW | 
                                                           ; ((S * SCALE_S) + S_OFFSET) * RHW

ANoPerspTextureST1:

   movq    qword ptr [hwptr1], mm1                         ; Write TU2 TV2
   add     hwptr1, 8

ifdef IGX_SPECULAR_FIX
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      ANoST1Specular

   mov     edx, dword ptr [__asm_data + _RC]
   mov     edx, dword ptr [edx+OFFSET_RC_ALPHABLEND_ENABLE]
   test    edx, edx
   je      ANoST1Specular

   mov     edi, dword ptr _specdwords[esp+4]
   movq    qword ptr [SpecularStream + edi], mm1
   add     edi, 8
   mov     dword ptr _specdwords[esp+4], edi

ANoST1Specular:
;********************************************************
endif

ANoST1:
;********************************************************
;ST1 Setup
;********************************************************

;********************************************************
; Vert2
;
; mm0 - X | Y, shared with Wfbi later
; mm1 - RGBA Color, shared with ST1 later
; mm2 - Z
; mm3 - RHW or W0
; mm4 - ST0
; mm5 - Scale T | Scale S
; mm6 - Pixel Center | Pixel Center
; mm7 - Texel Offset | Texel Offset
;
;********************************************************
   mov     edx, dword ptr [_X_Offset]
   movq    mm0, qword ptr [vb1 + edx]

   movd    mm1, dword ptr [_bColor]

   pfadd   mm0, mm6                                        ; Y + PIXEL_OFFSET | 
                                                           ; X + PIXEL_OFFSET

   mov     edx, dword ptr [_Z_Offset]
   movd    mm2, dword ptr [vb1 + edx]

   movq    qword ptr [hwptr1], mm0                         ; Write X + Y
   add     hwptr1, 8

;********************************************************
; Save off data for specular
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      BNoXYSpecular

   mov     edi, dword ptr _specdwords[esp+4]
   movq    qword ptr [SpecularStream + edi], mm0

   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_FLAT
   je      BNoFlatShading

   mov     edi, dword ptr [_Spec_Offset]
   mov     edx, dword ptr [edi + va1]

   jmp     short BPastNoFlatShading

BNoFlatShading:

   mov     edi, dword ptr [_Spec_Offset]
   mov     edx, dword ptr [edi + vb1]

BPastNoFlatShading:

ifdef IGX_SPECULAR_FIX
   movd    edi, mm1
   xor     edx, edi
   and     edx, 16777215
   xor     edx, edi
   movd    mm0, edx
endif

   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi + 8], mm0
   add     edi, 12
   mov     dword ptr _specdwords[esp+4], edi

BNoXYSpecular:
;********************************************************

   mov     edx, dword ptr [_RHW_Offset]
   movd    mm3, dword ptr [vb1 + edx]

   movq    mm4, qword ptr [_BST0] ; Wrap Code
;   movq    mm4, qword ptr [vb1 + s0]

   punpckldq mm3, mm3                                      ; pB->rhw | pB->rhw

ifdef DCT_FIX
   pfmul   mm3, qword ptr [__asm_data + _SCALEWS+8 ]       ; multiply w scale factor with correction.
endif

   pfmul   mm4, mm5                                        ; T * SCALE_T | S * SCALE_S

;********************************************************
; COLOR
;********************************************************
;  if ( setupFlag & ( SST_SETUP_RGB | SST_SETUP_A ) )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_COLOR
   je      SHORT BNoColor

   movd    dword ptr [hwptr1], mm1	                       ; Write Color
   add     hwptr1, 4

BNoColor:
;********************************************************
; COLOR
;********************************************************

;********************************************************
; Z Setup
; 
; if( pRc->state & STATE_REQUIRES_WBUFFER )
;   if (pRc->state & STATE_REQUIRES_VERTEXFOG)
;     zX = (float)((255 - RGBA_GETALPHA(pX[FVFO_SPECULAR])) << 8);
;   else
;     zX = ZSCALE ( FLTP(pX)[FVFO_SZ] );
; else
;   Send down Z the way we got it
;
;********************************************************
;  if ( setupFlag & SST_SETUP_Z )
   test    edx, FLAG_Z
   je      SHORT BNoZ

;  if( pRc->state & STATE_REQUIRES_WBUFFER )
   test    edx, FLAG_WBUFFER
   je      SHORT BNoZWBuffer

;  if (pRc->state & STATE_REQUIRES_VERTEXFOG)
   test    edx, FLAG_VFOG
   je      SHORT BSendZ

;********************************************************
; Z-FOG Implementation
;
; (255 - RGBA_GETALPHA(pX[FVFO_SPECULAR])) << 8;
;********************************************************
   xor     edx, edx
   mov     edi, dword ptr [_Spec_Offset]
   mov     dl, byte ptr [edi + vb1 + 3]
   not     dl
   shl     edx, 8
   movd    mm2, edx
   pi2fd   mm2, mm2
   jmp     SHORT BSendZ
;********************************************************
; Z-FOG Implementation
;********************************************************

BNoZWBuffer:

   pfmul   mm2, qword ptr [__asm_data + _SCALEZ]           ; 0 | Z * SCALEZ

BSendZ:

   movd    dword ptr [hwptr1], mm2                         ; Write Z
   add     hwptr1, 4

;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      BNoZSpecular

   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi], mm2
   add     edi, 4
   mov     dword ptr _specdwords[esp+4], edi

BNoZSpecular:
;********************************************************

BNoZ:
;********************************************************
; Z Setup
;********************************************************

;********************************************************
; Wfbi Setup
;
; if( pRc->state & STATE_REQUIRES_WBUFFER )
;   wX = WSCALE( FLTP(pX)[FVFO_RHW] );
; else
;   if (pRc->state & STATE_REQUIRES_VERTEXFOG)
;     wX = (float)(255 - RGBA_GETALPHA(pX[FVFO_SPECULAR]));
;	else if (pRc->state & STATE_REQUIRES_HWFOG)
;     wX = RHW
;
;********************************************************
;  if ( setupFlag & SST_SETUP_Wfbi )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_WFBI
   je      SHORT BNoWfbi

;  if( pRc->state & STATE_REQUIRES_WBUFFER )
   test    edx, FLAG_WBUFFER
   je      SHORT BNoWWBuffer

   movq    mm0, mm3
   pfmul   mm0, qword ptr [__asm_data + WBUFFER_SCALE_AW]  ; RHW * WBUFFER_SCALE_AW 
   pfadd   mm0, qword ptr [__asm_data + WBUFFER_SCALE_BW]  ; (RHW * WBUFFER_SCALE_AW) + WBUFFER_SCALE_BW
   jmp     SHORT BSendW
;#define WSCALE(a) ((a * pRc->aW) + pRc->bW)
 
BNoWWBuffer:
;  if (pRc->state & STATE_REQUIRES_VERTEXFOG)
   test    edx, FLAG_VFOG
   je      SHORT BNoWVertexFog

;********************************************************
; W-FOG Implementation
;
; 255 - RGBA_GETALPHA(pX[FVFO_SPECULAR]);
;********************************************************
   xor     edx, edx
   mov     edi, dword ptr [_Spec_Offset]
   mov     dl, byte ptr [edi + vb1 + 3]
   not     dl
   movd    mm0, edx
   pi2fd   mm0, mm0
   jmp     SHORT BSendW
;   (rgb) >> 24
;********************************************************
; W-FOG Implementation
;********************************************************

BNoWVertexFog:
;  if (pRc->state & STATE_REQUIRES_HWFOG)
   mov     edx, dword ptr [__asm_data + RENDER_STATE]
   test    dh, 32
   je      SHORT BSendW

   movq    mm0, mm3                                        ; Load up RHW

BSendW:

   movd    dword ptr [hwptr1], mm0                         ; Write Wfbi
   add     hwptr1, 4

;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      BNoWfbiSpecular

   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi], mm0
   add     edi, 4
   mov     dword ptr _specdwords[esp+4], edi

BNoWfbiSpecular:  
;********************************************************

BNoWfbi:
;********************************************************
; Wfbi Setup
;********************************************************

;********************************************************
; W0 Setup
;********************************************************
;  if ( setupFlag & SST_SETUP_W0 )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_W
   je      SHORT BNoW0

   movd    dword ptr [hwptr1], mm3                         ; Write W
   add     hwptr1, 4

ifdef IGX_SPECULAR_FIX
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      BNoW0Specular

   mov     edx, dword ptr [__asm_data + _RC]
   mov     edx, dword ptr [edx+OFFSET_RC_ALPHABLEND_ENABLE]
   test    edx, edx
   je      BNoW0Specular

   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi], mm3
   add     edi, 4
   mov     dword ptr _specdwords[esp+4], edi

BNoW0Specular:
;********************************************************
endif

BNoW0:
;********************************************************
; W0 Setup
;********************************************************

;********************************************************
; ST0 Setup
;********************************************************
;  if ( setupFlag & SST_SETUP_ST0 )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_TX0
   je      SHORT BNoST0

   pfadd   mm4, mm7                                        ; (T * SCALE_T) + T_OFFSET |
                                                           ; (S * SCALE_S) + S_OFFSET

;  if ( pRc->state & STATE_REQUIRES_PERSPECTIVE )
   test    edx, FLAG_PC
   je      SHORT BNoPerspTextureST0

   pfmul   mm4, mm3                                        ; ((T * SCALE_T) + T_OFFSET) * RHW |
                                                           ; ((S * SCALE_S) + S_OFFSET) * RHW

BNoPerspTextureST0:

   movq    qword ptr [hwptr1], mm4                         ; Write TU1 TV1
   add     hwptr1, 8

ifdef IGX_SPECULAR_FIX
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      BNoST0Specular

   mov     edx, dword ptr [__asm_data + _RC]
   mov     edx, dword ptr [edx+OFFSET_RC_ALPHABLEND_ENABLE]
   test    edx, edx
   je      BNoST0Specular

   mov     edi, dword ptr _specdwords[esp+4]
   movq    qword ptr [SpecularStream + edi], mm4
   add     edi, 8
   mov     dword ptr _specdwords[esp+4], edi

BNoST0Specular:
;********************************************************
endif

BNoST0:
;********************************************************
; ST0 Setup 
;********************************************************

;********************************************************
;ST1 Setup
;********************************************************
;  if ( setupFlag & SST_SETUP_ST1 )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_TX1
   je      SHORT BNoST1

   movq    mm1, qword ptr [_BST1]
;   mov     edx, dword ptr [__asm_data + T1_OFFSET]         
;   movq    mm1, qword ptr [vc1 + edx]					   ; T | S

   pfmul   mm1, qword ptr [__asm_data + S1_SCALE]          ; T * SCALE_T | S * SCALE_S
   pfadd   mm1, qword ptr [__asm_data + S1_TEXEL_OFFSET]   ; ((T * SCALE_T) + T_OFFSET) |
                                                           ; ((S * SCALE_S) + S_OFFSET)

;  if ( pRc->state & STATE_REQUIRES_PERSPECTIVE )
   test    edx, FLAG_PC
   je      SHORT BNoPerspTextureST1

   pfmul   mm1, mm3                                        ; ((T * SCALE_T) + T_OFFSET) * RHW | 
                                                           ; ((S * SCALE_S) + S_OFFSET) * RHW

BNoPerspTextureST1:

   movq    qword ptr [hwptr1], mm1                         ; Write TU2 TV2
   add     hwptr1, 8

ifdef IGX_SPECULAR_FIX
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      BNoST1Specular

   mov     edx, dword ptr [__asm_data + _RC]
   mov     edx, dword ptr [edx+OFFSET_RC_ALPHABLEND_ENABLE]
   test    edx, edx
   je      BNoST1Specular

   mov     edi, dword ptr _specdwords[esp+4]
   movq    qword ptr [SpecularStream + edi], mm1
   add     edi, 8
   mov     dword ptr _specdwords[esp+4], edi

BNoST1Specular:
;********************************************************
endif

BNoST1:
;********************************************************
;ST1 Setup
;********************************************************

;********************************************************
; Vert3
;
; mm0 - X | Y, shared with Wfbi later
; mm1 - RGBA Color, shared with ST1 later
; mm2 - Z
; mm3 - RHW or W0
; mm4 - ST0
; mm5 - Scale T | Scale S
; mm6 - Pixel Center | Pixel Center
; mm7 - Texel Offset | Texel Offset
;
;********************************************************
   mov     edx, dword ptr [_X_Offset]
   movq    mm0, qword ptr [vc1 + edx]

   movd    mm1, dword ptr [_cColor]

   pfadd   mm0, mm6                                        ; Y + PIXEL_OFFSET | 
                                                           ; X + PIXEL_OFFSET

   mov     edx, dword ptr [_Z_Offset]
   movd    mm2, dword ptr [vc1 + edx]

   movq    qword ptr [hwptr1], mm0                         ; Write X + Y
   add     hwptr1, 8

;********************************************************
; Save off data for specular
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      CNoXYSpecular

   mov     edi, dword ptr _specdwords[esp+4]
   movq    qword ptr [SpecularStream + edi], mm0

   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_FLAT
   je      CNoFlatShading

   mov     edi, dword ptr [_Spec_Offset]
   mov     edx, dword ptr [edi + va1]

   jmp     short CPastNoFlatShading

CNoFlatShading:

   mov     edi, dword ptr [_Spec_Offset]
   mov     edx, dword ptr [edi + vc1]

CPastNoFlatShading:

ifdef IGX_SPECULAR_FIX
   movd    edi, mm1
   xor     edx, edi
   and     edx, 16777215
   xor     edx, edi
   movd    mm0, edx
endif

   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi + 8], mm0
   add     edi, 12
   mov     dword ptr _specdwords[esp+4], edi

CNoXYSpecular:
;********************************************************

   mov     edx, dword ptr [_RHW_Offset]
   movd    mm3, dword ptr [vc1 + edx]

   movq    mm4, qword ptr [_CST0] ; Wrap Code
;  movq    mm4, qword ptr [vc1 + s0]

   punpckldq mm3, mm3                                      ; pB->rhw | pB->rhw

ifdef DCT_FIX
   pfmul   mm3, qword ptr [__asm_data + _SCALEWS+8 ]       ; multiply w scale factor with correction.
endif

   pfmul   mm4, mm5                                        ; T * SCALE_T | S * SCALE_S

;********************************************************
; COLOR
;********************************************************
;  if ( setupFlag & ( SST_SETUP_RGB | SST_SETUP_A ) )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_COLOR
   je      SHORT CNoColor

   movd    dword ptr [hwptr1], mm1	                       ; Write Color
   add     hwptr1, 4

CNoColor:
;********************************************************
; COLOR
;********************************************************

;********************************************************
; Z Setup
; 
; if( pRc->state & STATE_REQUIRES_WBUFFER )
;   if (pRc->state & STATE_REQUIRES_VERTEXFOG)
;     zX = (float)((255 - RGBA_GETALPHA(pX[FVFO_SPECULAR])) << 8);
;   else
;     zX = ZSCALE ( FLTP(pX)[FVFO_SZ] );
; else
;   Send down Z the way we got it
;
;********************************************************
;  if ( setupFlag & SST_SETUP_Z )
   test    edx, FLAG_Z
   je      SHORT CNoZ

;  if( pRc->state & STATE_REQUIRES_WBUFFER )
   test    edx, FLAG_WBUFFER
   je      SHORT CNoZWBuffer

;  if (pRc->state & STATE_REQUIRES_VERTEXFOG)
   test    edx, FLAG_VFOG
   je      SHORT CSendZ

;********************************************************
; Z-FOG Implementation
;
; (255 - RGBA_GETALPHA(pX[FVFO_SPECULAR])) << 8;
;********************************************************
   xor     edx, edx
   mov     edi, dword ptr [_Spec_Offset]
   mov     dl, byte ptr [edi + vc1 + 3]
   not     dl
   shl     edx, 8
   movd    mm2, edx
   pi2fd   mm2, mm2
   jmp     SHORT CSendZ
;********************************************************
; Z-FOG Implementation
;********************************************************

CNoZWBuffer:

   pfmul   mm2, qword ptr [__asm_data + _SCALEZ]           ; 0 | Z * SCALEZ

CSendZ:

   movd    dword ptr [hwptr1], mm2                         ; Write Z
   add     hwptr1, 4

;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      CNoZSpecular

   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi], mm2
   add     edi, 4
   mov     dword ptr _specdwords[esp+4], edi

CNoZSpecular:
;********************************************************

CNoZ:
;********************************************************
; Z Setup
;********************************************************

;********************************************************
; Wfbi Setup
;
; if( pRc->state & STATE_REQUIRES_WBUFFER )
;   wX = WSCALE( FLTP(pX)[FVFO_RHW] );
; else
;   if (pRc->state & STATE_REQUIRES_VERTEXFOG)
;     wX = (float)(255 - RGBA_GETALPHA(pX[FVFO_SPECULAR]));
;	else if (pRc->state & STATE_REQUIRES_HWFOG)
;     wX = RHW
;
;********************************************************
;  if ( setupFlag & SST_SETUP_Wfbi )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_WFBI
   je      SHORT CNoWfbi

;  if( pRc->state & STATE_REQUIRES_WBUFFER )
   test    edx, FLAG_WBUFFER
   je      SHORT CNoWWBuffer

   movq    mm0, mm3
   pfmul   mm0, qword ptr [__asm_data + WBUFFER_SCALE_AW]  ; RHW * WBUFFER_SCALE_AW 
   pfadd   mm0, qword ptr [__asm_data + WBUFFER_SCALE_BW]  ; (RHW * WBUFFER_SCALE_AW) + WBUFFER_SCALE_BW
   jmp     SHORT CSendW
;#define WSCALE(a) ((a * pRc->aW) + pRc->bW)
 
CNoWWBuffer:
;  if (pRc->state & STATE_REQUIRES_VERTEXFOG)
   test    edx, FLAG_VFOG
   je      SHORT CNoWVertexFog

;********************************************************
; W-FOG Implementation
;
; 255 - RGBA_GETALPHA(pX[FVFO_SPECULAR]);
;********************************************************
   xor     edx, edx
   mov     edi, dword ptr [_Spec_Offset]
   mov     dl, byte ptr [edi + vc1 + 3]
   not     dl
   movd    mm0, edx
   pi2fd   mm0, mm0
   jmp     SHORT CSendW
;   (rgb) >> 24
;********************************************************
; W-FOG Implementation
;********************************************************

CNoWVertexFog:
;  if (pRc->state & STATE_REQUIRES_HWFOG)
   mov     edx, dword ptr [__asm_data + RENDER_STATE]
   test    dh, 32
   je      SHORT CSendW

   movq    mm0, mm3                                        ; Load up RHW

CSendW:

   movd    dword ptr [hwptr1], mm0                         ; Write Wfbi
   add     hwptr1, 4

;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      CNoWfbiSpecular

   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi], mm0
   add     edi, 4
   mov     dword ptr _specdwords[esp+4], edi

CNoWfbiSpecular:  
;********************************************************

CNoWfbi:
;********************************************************
; Wfbi Setup
;********************************************************

;********************************************************
; W0 Setup
;********************************************************
;  if ( setupFlag & SST_SETUP_W0 )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_W
   je      SHORT CNoW0

   movd    dword ptr [hwptr1], mm3                         ; Write W
   add     hwptr1, 4

ifdef IGX_SPECULAR_FIX
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      CNoW0Specular

   mov     edx, dword ptr [__asm_data + _RC]
   mov     edx, dword ptr [edx+OFFSET_RC_ALPHABLEND_ENABLE]
   test    edx, edx
   je      CNoW0Specular

   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi], mm3
   add     edi, 4
   mov     dword ptr _specdwords[esp+4], edi

CNoW0Specular:
;********************************************************
endif

CNoW0:
;********************************************************
; W0 Setup
;********************************************************

;********************************************************
; ST0 Setup
;********************************************************
;  if ( setupFlag & SST_SETUP_ST0 )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_TX0
   je      SHORT CNoST0

   pfadd   mm4, mm7                                        ; (T * SCALE_T) + T_OFFSET |
                                                           ; (S * SCALE_S) + S_OFFSET

;  if ( pRc->state & STATE_REQUIRES_PERSPECTIVE )
   test    edx, FLAG_PC
   je      SHORT CNoPerspTextureST0

   pfmul   mm4, mm3                                        ; ((T * SCALE_T) + T_OFFSET) * RHW |
                                                           ; ((S * SCALE_S) + S_OFFSET) * RHW

CNoPerspTextureST0:

   movq    qword ptr [hwptr1], mm4                         ; Write TU1 TV1
   add     hwptr1, 8

ifdef IGX_SPECULAR_FIX
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      CNoST0Specular

   mov     edx, dword ptr [__asm_data + _RC]
   mov     edx, dword ptr [edx+OFFSET_RC_ALPHABLEND_ENABLE]
   test    edx, edx
   je      CNoST0Specular

   mov     edi, dword ptr _specdwords[esp+4]
   movq    qword ptr [SpecularStream + edi], mm4
   add     edi, 8
   mov     dword ptr _specdwords[esp+4], edi

CNoST0Specular:
;********************************************************
endif

CNoST0:
;********************************************************
; ST0 Setup 
;********************************************************

;********************************************************
;ST1 Setup
;********************************************************
;  if ( setupFlag & SST_SETUP_ST1 )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_TX1
   je      SHORT CNoST1

   movq    mm1, qword ptr [_CST1]
;   mov     edx, dword ptr [__asm_data + T1_OFFSET]         
;   movq    mm1, qword ptr [vc1 + edx]					   ; T | S

   pfmul   mm1, qword ptr [__asm_data + S1_SCALE]          ; T * SCALE_T | S * SCALE_S
   pfadd   mm1, qword ptr [__asm_data + S1_TEXEL_OFFSET]   ; ((T * SCALE_T) + T_OFFSET) |
                                                           ; ((S * SCALE_S) + S_OFFSET)

;  if ( pRc->state & STATE_REQUIRES_PERSPECTIVE )
   test    edx, FLAG_PC
   je      SHORT CNoPerspTextureST1

   pfmul   mm1, mm3                                        ; ((T * SCALE_T) + T_OFFSET) * RHW | 
                                                           ; ((S * SCALE_S) + S_OFFSET) * RHW

CNoPerspTextureST1:

   movq    qword ptr [hwptr1], mm1                         ; Write TU2 TV2
   add     hwptr1, 8

ifdef IGX_SPECULAR_FIX
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      CNoST1Specular

   mov     edx, dword ptr [__asm_data + _RC]
   mov     edx, dword ptr [edx+OFFSET_RC_ALPHABLEND_ENABLE]
   test    edx, edx
   je      CNoST1Specular

   mov     edi, dword ptr _specdwords[esp+4]
   movq    qword ptr [SpecularStream + edi], mm1
   add     edi, 8
   mov     dword ptr _specdwords[esp+4], edi

CNoST1Specular:
;********************************************************
endif

CNoST1:
;********************************************************
;ST1 Setup
;********************************************************
   Autostrip_Tri_To_Strip
      
   ;   jmp     do_cycle_k6m

;********************************************************
; CHECK FOR WRAP
;********************************************************
   mov     edx, dword ptr [__asm_data + _RC]
   mov     edi, dword ptr [edx + 1076]
   or      edi, dword ptr [edx + 1080]
   mov     edx, dword ptr _verts[esp+4]
   test    edi, edi
   mov     edi,dword ptr _count[esp+4]                       ;count1 = _count[esp]
   jne     do_cycle_k6m
;********************************************************
; CHECK FOR WRAP
;********************************************************

;********************************************************
; CHECK FOR FLAT SHADING
;********************************************************
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_FLAT
   jne     do_cycle_k6m
;********************************************************
; CHECK FOR FLAT SHADING
;********************************************************

;  jmp      do_cycle_k6m
   mov     dword ptr _last[esp+4],BIT_22

prim_continue:
;  We found a strip or a fan
   dec     edi                                             ;count1--

   mov     dword ptr _temp1[esp+4], esi
   mov     esi, _hwptr[esp+4]                                ;save hwptr1
   js      done_render_k6m

   mov     esi, dword ptr _temp1[esp+4]

   mov     dword ptr _count[esp+4],edi

;********************************************************
; Culling Check
;********************************************************
   Cull_3DNow
;********************************************************
; Culling Check
;********************************************************

;********************************************************
; Specular Check
;********************************************************
   mov     dword ptr _specflag[esp+4], 0

   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_SPEC
   je      NoBuildAutostripPacketSpecular

   Specular_Test_And_Build_Autostrip_Packet

NoBuildAutostripPacketSpecular:
;********************************************************
; Specular Check
;********************************************************

   mov     eax, _last[esp+4]
   mov     ebx, dword ptr [__asm_data + CMD_CONT]

   mov     edx,esi                                         ;tmp1 = test1
   xor     esi,eax

   mov     dword ptr _last[esp+4],edx
   xor     esi,BIT_22

   and     esi,BIT_22
 
   or      ebx, esi                                        ;header1 |= test1	 

   mov     esi, _hwptr[esp+4]                                ;hwptr1 = _hwptr[esp]

;********************************************************
; Send down Triangle Contiunation Header Packet
;********************************************************
   mov     [esi], ebx                                      ;*hwptr = header1
   add     esi, 4

;********************************************************
; Color Precalc!
;
; if ( FLAG_COLOR )
;   if ( Specular Enabled && No Texture )
;      CLAMP888(cColor, pC[FVFO_COLOR], pC[FVFO_SPECULAR]);
;   else
;      cColor = vc1 color
;  
; CLAMP888(xColor, pX[FVFO_COLOR], pX[FVFO_SPECULAR]);
;
; If the diffuse color index == 128, then xColor = FFFFFFFFh
; 
; Add the Specular to the Diffuse Color without
; Overflow into the higher packed bytes
;********************************************************
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_COLOR
   je      SHORT DPrecalcNoColor

   test    edx, FLAG_SPEC
   je      DPrecalcNoSpecularEnabled

; if ( pRc->specular )
   mov     edx, dword ptr [__asm_data + _RC]
   mov     edi, dword ptr [edx + OFFSET_RC_SPECULAR_FLAG]
   test    edi, edi
   je      DPrecalcNoSpecularEnabled
   
; if ( pRc->texture == 0 )
   mov     edi, dword ptr [edx + OFFSET_RC_TEXTURE_PTR]
   test    edi, edi
   jne     DPrecalcNoSPecularEnabled

   mov     edi, 0fefefeh

   mov     edx, dword ptr [_Color_Offset]   
   cmp     edx, 128
   je      DPrecalcNoColorInFVF

   movd    mm1, dword ptr [vc1 + edx]

   jmp     DPrecalcPastNoColorInFVF

DPrecalcNoColorInFVF:
 
   pcmpeqd mm1, mm1

DPrecalcPastNoColorInFVF:

   movd    mm4, edi

   mov     edx, dword ptr [_Spec_Offset]
   movd    mm0, dword ptr [edx + vc1]
   pand    mm0, mm4
   paddusb mm1, mm0

   movd    dword ptr [_cColor], mm1

   jmp     DPrecalcNoColor

DPrecalcNoSpecularEnabled:
   
   mov     edx, dword ptr [_Color_Offset]
   cmp     edx, 128
   je      DPrecalcNoSpecNoColorInFVF

   movd    mm0, dword ptr [vc1 + edx]

   jmp     DPrecalcNoSpecPastNoColorInFVF

DPrecalcNoSpecNoColorInFVF:

   pcmpeqd mm0, mm0

DPrecalcNoSpecPastNoColorInFVF:

   movd    dword ptr [_cColor], mm0

DPrecalcNoColor:
;********************************************************
; Color Precalc!
;********************************************************

;********************************************************
; Vert4
;
; mm0 - X | Y, shared with Wfbi later
; mm1 - RGBA Color, shared with ST1 later
; mm2 - Z
; mm3 - RHW or W0
; mm4 - ST0
; mm5 - Scale T | Scale S
; mm6 - Pixel Center | Pixel Center
; mm7 - Texel Offset | Texel Offset
;
;********************************************************
   mov     edx, dword ptr [_X_Offset]
   movq    mm0, qword ptr [vc1 + edx]

   movd    mm1, dword ptr [_cColor]

   pfadd   mm0, mm6                                        ; Y + PIXEL_OFFSET | 
                                                           ; X + PIXEL_OFFSET
   mov     edx, dword ptr [_Z_Offset]
   movd    mm2, dword ptr [vc1 + edx]

   movq    qword ptr [hwptr1], mm0                         ; Write X + Y
   add     hwptr1, 8

;********************************************************
; Save off data for specular
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      DNoXYSpecular

   mov     edi, dword ptr _specdwords[esp+4]
   movq    qword ptr [SpecularStream + edi], mm0

   mov     edi, dword ptr [_Spec_Offset]
   mov     edx, dword ptr [edi + vc1]

ifdef IGX_SPECULAR_FIX
   movd    edi, mm1
   xor     edx, edi
   and     edx, 16777215
   xor     edx, edi
   movd    mm0, edx
endif

   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi + 8], mm0
   add     edi, 12
   mov     dword ptr _specdwords[esp+4], edi

DNoXYSpecular:
;********************************************************

   mov     edx, dword ptr [_RHW_Offset]
   movd    mm3, dword ptr [vc1 + edx]

;  movq    mm4, qword ptr [_CST0] ; Wrap Code
   mov     edx, dword ptr [__T0_Offset]

   movq    mm4, qword ptr [vc1 + edx]

   punpckldq mm3, mm3                                      ; pB->rhw | pB->rhw

ifdef DCT_FIX
   pfmul   mm3, qword ptr [__asm_data + _SCALEWS+8 ]       ; multiply w scale factor with correction.
endif

   pfmul   mm4, mm5                                        ; T * SCALE_T | S * SCALE_S

;********************************************************
; COLOR
;********************************************************
;  if ( setupFlag & ( SST_SETUP_RGB | SST_SETUP_A ) )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_COLOR
   je      SHORT DNoColor

   movd    dword ptr [hwptr1], mm1	                       ; Write Color
   add     hwptr1, 4

DNoColor:
;********************************************************
; COLOR
;********************************************************

;********************************************************
; Z Setup
; 
; if( pRc->state & STATE_REQUIRES_WBUFFER )
;   if (pRc->state & STATE_REQUIRES_VERTEXFOG)
;     zX = (float)((255 - RGBA_GETALPHA(pX[FVFO_SPECULAR])) << 8);
;   else
;     zX = ZSCALE ( FLTP(pX)[FVFO_SZ] );
; else
;   Send down Z the way we got it
;
;********************************************************
;  if ( setupFlag & SST_SETUP_Z )
   test    edx, FLAG_Z
   je      SHORT DNoZ

;  if( pRc->state & STATE_REQUIRES_WBUFFER )
   test    edx, FLAG_WBUFFER
   je      SHORT DNoZWBuffer

;  if (pRc->state & STATE_REQUIRES_VERTEXFOG)
   test    edx, FLAG_VFOG
   je      SHORT DSendZ

;********************************************************
; Z-FOG Implementation
;
; (255 - RGBA_GETALPHA(pX[FVFO_SPECULAR])) << 8;
;********************************************************
   xor     edx, edx
   mov     edi, dword ptr [_Spec_Offset]
   mov     dl, byte ptr [edi + vc1 + 3]
   not     dl
   shl     edx, 8
   movd    mm2, edx
   pi2fd   mm2, mm2
   jmp     SHORT DSendZ
;********************************************************
; Z-FOG Implementation
;********************************************************

DNoZWBuffer:

   pfmul   mm2, qword ptr [__asm_data + _SCALEZ]           ; 0 | Z * SCALEZ

DSendZ:

   movd    dword ptr [hwptr1], mm2                         ; Write Z
   add     hwptr1, 4

;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      DNoZSpecular

   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi], mm2
   add     edi, 4
   mov     dword ptr _specdwords[esp+4], edi

DNoZSpecular:
;********************************************************

DNoZ:
;********************************************************
; Z Setup
;********************************************************

;********************************************************
; Wfbi Setup
;
; if( pRc->state & STATE_REQUIRES_WBUFFER )
;   wX = WSCALE( FLTP(pX)[FVFO_RHW] );
; else
;   if (pRc->state & STATE_REQUIRES_VERTEXFOG)
;     wX = (float)(255 - RGBA_GETALPHA(pX[FVFO_SPECULAR]));
;	else if (pRc->state & STATE_REQUIRES_HWFOG)
;     wX = RHW
;
;********************************************************
;  if ( setupFlag & SST_SETUP_Wfbi )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_WFBI
   je      SHORT DNoWfbi

;  if( pRc->state & STATE_REQUIRES_WBUFFER )
   test    edx, FLAG_WBUFFER
   je      SHORT DNoWWBuffer

   movq    mm0, mm3
   pfmul   mm0, qword ptr [__asm_data + WBUFFER_SCALE_AW]  ; RHW * WBUFFER_SCALE_AW 
   pfadd   mm0, qword ptr [__asm_data + WBUFFER_SCALE_BW]  ; (RHW * WBUFFER_SCALE_AW) + WBUFFER_SCALE_BW
   jmp     SHORT DSendW
;#define WSCALE(a) ((a * pRc->aW) + pRc->bW)
 
DNoWWBuffer:
;  if (pRc->state & STATE_REQUIRES_VERTEXFOG)
   test    edx, FLAG_VFOG
   je      SHORT DNoWVertexFog

;********************************************************
; W-FOG Implementation
;
; 255 - RGBA_GETALPHA(pX[FVFO_SPECULAR]);
;********************************************************
   xor     edx, edx
   mov     edi, dword ptr [_Spec_Offset]
   mov     dl, byte ptr [edi + vc1 + 3]
   not     dl
   movd    mm0, edx
   pi2fd   mm0, mm0
   jmp     SHORT DSendW
;   (rgb) >> 24
;********************************************************
; W-FOG Implementation
;********************************************************

DNoWVertexFog:
;  if (pRc->state & STATE_REQUIRES_HWFOG)
   mov     edx, dword ptr [__asm_data + RENDER_STATE]
   test    dh, 32
   je      SHORT DSendW

   movq    mm0, mm3                                        ; Load up RHW

DSendW:

   movd    dword ptr [hwptr1], mm0                         ; Write Wfbi
   add     hwptr1, 4

;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      DNoWfbiSpecular

   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi], mm0
   add     edi, 4
   mov     dword ptr _specdwords[esp+4], edi

DNoWfbiSpecular:  
;********************************************************

DNoWfbi:
;********************************************************
; Wfbi Setup
;********************************************************

;********************************************************
; W0 Setup
;********************************************************
;  if ( setupFlag & SST_SETUP_W0 )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_W
   je      SHORT DNoW0

   movd    dword ptr [hwptr1], mm3                         ; Write W
   add     hwptr1, 4

ifdef IGX_SPECULAR_FIX
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      DNoW0Specular

   mov     edx, dword ptr [__asm_data + _RC]
   mov     edx, dword ptr [edx+OFFSET_RC_ALPHABLEND_ENABLE]
   test    edx, edx
   je      DNoW0Specular

   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi], mm3
   add     edi, 4
   mov     dword ptr _specdwords[esp+4], edi

DNoW0Specular:
;********************************************************
endif

DNoW0:
;********************************************************
; W0 Setup
;********************************************************

;********************************************************
; ST0 Setup
;********************************************************
;  if ( setupFlag & SST_SETUP_ST0 )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_TX0
   je      SHORT DNoST0

   pfadd   mm4, mm7                                        ; (T * SCALE_T) + T_OFFSET |
                                                           ; (S * SCALE_S) + S_OFFSET

;  if ( pRc->state & STATE_REQUIRES_PERSPECTIVE )
   test    edx, FLAG_PC
   je      SHORT DNoPerspTextureST0

   pfmul   mm4, mm3                                        ; ((T * SCALE_T) + T_OFFSET) * RHW |
                                                           ; ((S * SCALE_S) + S_OFFSET) * RHW

DNoPerspTextureST0:

   movq    qword ptr [hwptr1], mm4                         ; Write TU1 TV1
   add     hwptr1, 8

ifdef IGX_SPECULAR_FIX
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      DNoST0Specular

   mov     edx, dword ptr [__asm_data + _RC]
   mov     edx, dword ptr [edx+OFFSET_RC_ALPHABLEND_ENABLE]
   test    edx, edx
   je      DNoST0Specular

   mov     edi, dword ptr _specdwords[esp+4]
   movq    qword ptr [SpecularStream + edi], mm4
   add     edi, 8						
   mov     dword ptr _specdwords[esp+4], edi

DNoST0Specular:
;********************************************************
endif

DNoST0:
;********************************************************
; ST0 Setup 
;********************************************************

;********************************************************
;ST1 Setup
;********************************************************
;  if ( setupFlag & SST_SETUP_ST1 )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_TX1
   je      SHORT DNoST1

;   movq    mm1, qword ptr [_CST1]
   mov     edx, dword ptr [_T1_Offset]
   movq    mm1, qword ptr [vc1 + edx]					   ; T | S

   pfmul   mm1, qword ptr [__asm_data + S1_SCALE]          ; T * SCALE_T | S * SCALE_S
   pfadd   mm1, qword ptr [__asm_data + S1_TEXEL_OFFSET]   ; ((T * SCALE_T) + T_OFFSET) |
                                                           ; ((S * SCALE_S) + S_OFFSET)

;  if ( pRc->state & STATE_REQUIRES_PERSPECTIVE )
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_PC
   je      SHORT DNoPerspTextureST1

   pfmul   mm1, mm3                                        ; ((T * SCALE_T) + T_OFFSET) * RHW | 
                                                           ; ((S * SCALE_S) + S_OFFSET) * RHW

DNoPerspTextureST1:

   movq    qword ptr [hwptr1], mm1                         ; Write TU2 TV2
   add     hwptr1, 8

ifdef IGX_SPECULAR_FIX
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      DNoST1Specular

   mov     edx, dword ptr [__asm_data + _RC]
   mov     edx, dword ptr [edx+OFFSET_RC_ALPHABLEND_ENABLE]
   test    edx, edx
   je      DNoST1Specular

   mov     edi, dword ptr _specdwords[esp+4]
   movq    qword ptr [SpecularStream + edi], mm1
   add     edi, 8
   mov     dword ptr _specdwords[esp+4], edi

DNoST1Specular:
;********************************************************
endif

DNoST1:
;********************************************************
;ST1 Setup
;********************************************************

   Autostrip_Tri_To_Strip

;  jmp     do_cycle_k6m

   jmp     prim_continue

ifndef MS_VIEW
align 32
else
align 16
endif
culled_tri_k6m:
   add     post_idx1,12
   mov     verts2,dword ptr _verts[esp+4]
   mov     va1,dword ptr [post_idx1]
   mov     vb1,dword ptr [post_idx1+4]

   mov     vc1,dword ptr [post_idx1+8]

   mov     count1,dword ptr _count[esp+4]

   jmp     do_cycle_k6m
   nop

ifndef MS_VIEW
align 32
else
align 16
endif
done_render_k6m:
;********************************************************
; Specular Stream Execution
;********************************************************
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_SPEC
   je      NoSpecularBufferRendering

;********************************************************
; Setup Hardware for Specular Pass
;********************************************************
   SpecularSetupHardware
;********************************************************
; Setup Hardware for Specular Pass
;********************************************************

   mov     ebx, dword ptr _specdwords[esp+4]
   shr     ebx, 2							; # of dwords to write
   cmp     ebx, 0
   je      SpecularStreamEmpty

   mov     edi, hwptr1

   mov     ecx, ebx

   sub     ecx, 08h							; is there at least 8 dwords to write?
   lea     esi, [SpecularStream]
   jl      SHORT SecondPass_BlockMoveDone

align 16
SecondPass_NextBlockMove:
	movq	mm0, qword ptr [esi+000h]
	movq	mm1, qword ptr [esi+008h]
	add		edi, 020h						; pre-increment destination pointer
	movq	mm2, qword ptr [esi+010h]
	movq	mm3, qword ptr [esi+018h]
	movq	qword ptr [edi-020h], mm0
	movq	qword ptr [edi-018h], mm1
	add		esi, 020h						; post-increment source pointer
	movq	qword ptr [edi-010h], mm2
	sub		ecx, 08h						; we wrote 8 more dwords
	movq	qword ptr [edi-008h], mm3
	jge		SHORT SecondPass_NextBlockMove

SecondPass_BlockMoveDone:
	add		ecx, 08h
	rep     movsd

    mov     hwptr1, edi

SpecularStreamEmpty:
;********************************************************
; Restore hardware to previous state
;********************************************************
   SpecularRestoreHardware
;********************************************************
; Restore hardware to previous state
;********************************************************

NoSpecularBufferRendering:
;********************************************************
; Specular Stream Execution
;********************************************************

   mov     hwptr_old,dword ptr[__asm_data + _FIFO_PTR]
   mov     dword ptr[__asm_data + _FIFO_PTR],hwptr1

   sub     hwptr1,hwptr_old
   mov     room2,dword ptr [__asm_data + _FIFO_ROOM]

   shr     hwptr1,2
   sub     room2,hwptr1

   mov     dword ptr [__asm_data + _FIFO_ROOM],room2

;  sub     esp, 4

   ret
_DrawTriIdx2MainAsm_K62O   ENDP

if 0
;********************************************************
; Vertex Macro, requires:
;
; vx1 - va1, vb1, or vc1
; _xColor - _aColor, _bColor, or _cColor
; _XST0 - _AST0, _BST0, or _CST0
; _XST1 - _AST1, _BST1, or _CST1
; AUTOSTRIP - If we are in the autostrip section, remove
;             the WRAP and FLAT shaded references
;
;********************************************************
Vertex MACRO vx1:REQ, _xColor:REQ, _XST0:REQ, _XST1:REQ, AUTOSTRIP:REQ
LOCAL NoXYSpecular, NoZSpecular, NoWfbiSpecular, NoW0Specular
LOCAL NoST0Specular, NoST1Specular
;********************************************************
; Vert X
;
; mm0 - X | Y, shared with Wfbi later
; mm1 - RGBA Color, shared with ST1 later
; mm2 - Z
; mm3 - RHW or W0
; mm4 - ST0
; mm5 - Scale T | Scale S
; mm6 - Pixel Center | Pixel Center
; mm7 - Texel Offset | Texel Offset
;
;********************************************************
   mov     edx, dword ptr [_X_Offset]
   movq    mm0, qword ptr [vx1 + edx]

   movd    mm1, dword ptr [_xColor]

   pfadd   mm0, mm6                                        ; Y + PIXEL_OFFSET | 
                                                           ; X + PIXEL_OFFSET

IF (SEND_Z EQ TRUE)
   mov     edx, dword ptr [_Z_Offset]
   movd    mm2, dword ptr [vx1 + edx]
ENDIF

   movq    qword ptr [hwptr1], mm0                         ; Write X + Y
   add     hwptr1, 8

IF (SEND_SPEC EQ TRUE)
;********************************************************
; Save off data for specular
;********************************************************
   mov     edx, dword ptr _specflag[esp]
   test    edx, 1
   je      NoXYSpecular

   mov     edi, dword ptr _specdwords[esp]
   movq    qword ptr [SpecularStream + edi], mm0

   mov     edi, dword ptr [__asm_data + SPECULAR_OFFSET]
   mov     edx, dword ptr [edi + vx1]

ifdef IGX_SPECULAR_FIX
   movd    edi, mm1
   xor     edx, edi
   and     edx, 16777215
   xor     edx, edi
   movd    mm0, edx
endif

   mov     edi, dword ptr _specdwords[esp]
   movd    dword ptr [SpecularStream + edi + 8], mm0
   add     edi, 12
   mov     dword ptr _specdwords[esp], edi

NoXYSpecular:
;********************************************************
ENDIF ; Specular

IF ((SEND_W EQ TRUE) OR ((SEND_WFBI EQ TRUE) AND ((SEND_WBUFFER EQ TRUE) OR (SEND_HWFOG EQ TRUE))))
   mov     edx, dword ptr [_RHW_Offset]
   movd    mm3, dword ptr [vx1 + edx]
ENDIF ; W, Wfbi & WBufer | HWFog

IF (SEND_TX0 EQ TRUE)
 IF ((SEND_WRAP EQ TRUE) AND (AUTOSTRIP EQ FALSE))
   movq    mm4, qword ptr [_XST0] ; Wrap Code
 ELSE ; Not Wrap
   mov     edx, dword ptr [__T0_Offset]
   movq    mm4, qword ptr [vx1 + edx]
 ENDIF ; Wrap
ENDIF ; TX0

IF ((SEND_W EQ TRUE) OR ((SEND_WFBI EQ TRUE) AND ((SEND_WBUFFER EQ TRUE) OR (SEND_HWFOG EQ TRUE))))
   punpckldq mm3, mm3                                      ; pB->rhw | pB->rhw
ENDIF ; W, Wfbi & WBufer | HWFog

ifdef DCT_FIX
   pfmul   mm3, qword ptr [__asm_data + _SCALEWS+8 ]       ; multiply w scale factor with correction.
endif

IF (SEND_TX0 EQ TRUE)
   pfmul   mm4, mm5                                        ; T * SCALE_T | S * SCALE_S
ENDIF ; TX0

;********************************************************
; COLOR
;********************************************************
;  if ( setupFlag & ( SST_SETUP_RGB | SST_SETUP_A ) )
IF (SEND_COLOR EQ TRUE)
   movd    dword ptr [hwptr1], mm1	                       ; Write Color
   add     hwptr1, 4
ENDIF ; Color
;********************************************************
; COLOR
;********************************************************

;********************************************************
; Z Setup
; 
; if( pRc->state & STATE_REQUIRES_WBUFFER )
;   if (pRc->state & STATE_REQUIRES_VERTEXFOG)
;     zX = (float)((255 - RGBA_GETALPHA(pX[FVFO_SPECULAR])) << 8);
;   else
;     zX = ZSCALE ( FLTP(pX)[FVFO_SZ] );
; else
;   Send down Z the way we got it
;
;********************************************************
IF (SEND_Z EQ TRUE)
 IF (SEND_WBUFFER EQ TRUE)
  IF (SEND_VFOG EQ TRUE)
;********************************************************
; Z-FOG Implementation
;
; (255 - RGBA_GETALPHA(pX[FVFO_SPECULAR])) << 8;
;********************************************************
   xor     edx, edx
   mov     edi, dword ptr [_Spec_Offset]
   mov     dl, byte ptr [edi + vx1 + 3]
   not     dl
   shl     edx, 8
   movd    mm2, edx
   pi2fd   mm2, mm2
;********************************************************
; Z-FOG Implementation
;********************************************************
  ENDIF
 ELSE ; Not WBuffer
 
   pfmul   mm2, qword ptr [__asm_data + _SCALEZ]           ; 0 | Z * SCALEZ
 ENDIF ; WBuffer

   movd    dword ptr [hwptr1], mm2                         ; Write Z
   add     hwptr1, 4

 IF (SEND_SPEC EQ TRUE)
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp]
   test    edx, 1
   je      NoZSpecular

   mov     edi, dword ptr _specdwords[esp]
   movd    dword ptr [SpecularStream + edi], mm2
   add     edi, 4
   mov     dword ptr _specdwords[esp], edi

NoZSpecular:
;********************************************************
 ENDIF ; Specular
ENDIF ; Z
;********************************************************
; Z Setup
;********************************************************

;********************************************************
; Wfbi Setup
;
; if( pRc->state & STATE_REQUIRES_WBUFFER )
;   wX = WSCALE( FLTP(pX)[FVFO_RHW] );
; else
;   if (pRc->state & STATE_REQUIRES_VERTEXFOG)
;     wX = (float)(255 - RGBA_GETALPHA(pX[FVFO_SPECULAR]));
;	else if (pRc->state & STATE_REQUIRES_HWFOG)
;     wX = RHW
;
;********************************************************
IF (SEND_WFBI EQ TRUE)
 IF (SEND_WBUFFER EQ TRUE)
   movq    mm0, mm3
   pfmul   mm0, qword ptr [__asm_data + WBUFFER_SCALE_AW]  ; RHW * WBUFFER_SCALE_AW 
   pfadd   mm0, qword ptr [__asm_data + WBUFFER_SCALE_BW]  ; (RHW * WBUFFER_SCALE_AW) + WBUFFER_SCALE_BW
;#define WSCALE(a) ((a * pRc->aW) + pRc->bW)
 ELSE ; Not WBuffer 
  IF (SEND_VFOG EQ TRUE)
;********************************************************
; W-FOG Implementation
;
; 255 - RGBA_GETALPHA(pX[FVFO_SPECULAR]);
;********************************************************
   xor     edx, edx
   mov     edi, dword ptr [_Spec_Offset]
   mov     dl, byte ptr [edi + vx1 + 3]
   not     dl
   movd    mm0, edx
   pi2fd   mm0, mm0
;   (rgb) >> 24
;********************************************************
; W-FOG Implementation
;********************************************************
  ELSE ; No Vertex Fog
;  if (pRc->state & STATE_REQUIRES_HWFOG)
   mov     edx, dword ptr [__asm_data + RENDER_STATE]
   test    dh, 32
   je      SHORT ASendW

   movq    mm0, mm3                                        ; Load up RHW
  ENDIF ; Vertex Fog
 ENDIF ; Wbuffer

   movd    dword ptr [hwptr1], mm0                         ; Write Wfbi
   add     hwptr1, 4

 IF (SEND_SPEC EQ TRUE)
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp]
   test    edx, 1
   je      NoWfbiSpecular

   mov     edi, dword ptr _specdwords[esp]
   movd    dword ptr [SpecularStream + edi], mm0
   add     edi, 4
   mov     dword ptr _specdwords[esp], edi

NoWfbiSpecular:  
;********************************************************
 ENDIF ; Specular

ENDIF ; Wfbi
;********************************************************
; Wfbi Setup
;********************************************************

;********************************************************
; W0 Setup
;********************************************************
IF (SEND_W EQ TRUE)
   movd    dword ptr [hwptr1], mm3                         ; Write W
   add     hwptr1, 4

ifdef IGX_SPECULAR_FIX
 IF ((SEND_SPEC EQ TRUE) AND (SEND_ALPHA EQ TRUE))
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp]
   test    edx, 1
   je      NoW0Specular

   mov     edi, dword ptr _specdwords[esp]
   movd    dword ptr [SpecularStream + edi], mm3
   add     edi, 4
   mov     dword ptr _specdwords[esp], edi

NoW0Specular:
;********************************************************
 ENDIF ; Specular And Alpha Blending
endif

ENDIF ; W
;********************************************************
; W0 Setup
;********************************************************

;********************************************************
; ST0 Setup
;********************************************************
IF (SEND_TX0 EQ TRUE)
   pfadd   mm4, mm7                                        ; (T * SCALE_T) + T_OFFSET |
                                                           ; (S * SCALE_S) + S_OFFSET
 IF (SEND_PC EQ TRUE)
   pfmul   mm4, mm3                                        ; ((T * SCALE_T) + T_OFFSET) * RHW |
                                                           ; ((S * SCALE_S) + S_OFFSET) * RHW
 ENDIF ; Perspective Correct

   movq    qword ptr [hwptr1], mm4                         ; Write TU1 TV1
   add     hwptr1, 8

ifdef IGX_SPECULAR_FIX
 IF ((SEND_SPEC EQ TRUE) AND (SEND_ALPHA EQ TRUE))
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp]
   test    edx, 1
   je      NoST0Specular
  
   mov     edi, dword ptr _specdwords[esp]
   movq    qword ptr [SpecularStream + edi], mm4
   add     edi, 8
   mov     dword ptr _specdwords[esp], edi

NoST0Specular:
;********************************************************
 ENDIF ; Specular And Alpha Blending
endif

ENDIF ; TX0
;********************************************************
; ST0 Setup 
;********************************************************

;********************************************************
;ST1 Setup
;********************************************************
IF (SEND_TX1 EQ TRUE)
 IF ((SEND_WRAP EQ TRUE) AND (AUTOSTRIP EQ FALSE))
   movq    mm1, qword ptr [_XST1]
 ELSE ; Not Wrap
   mov     edx, dword ptr [_T1_Offset]
   movq    mm1, qword ptr [vx1 + edx]					   ; T | S
 ENDIF ; Wrap
 
   pfmul   mm1, qword ptr [__asm_data + S1_SCALE]          ; T * SCALE_T | S * SCALE_S
   pfadd   mm1, qword ptr [__asm_data + S1_TEXEL_OFFSET]   ; ((T * SCALE_T) + T_OFFSET) |
                                                           ; ((S * SCALE_S) + S_OFFSET)
 IF (SEND_PC EQ TRUE)
   pfmul   mm1, mm3                                        ; ((T * SCALE_T) + T_OFFSET) * RHW | 
                                                           ; ((S * SCALE_S) + S_OFFSET) * RHW
 ENDIF ; Perpsecitve Correct

   movq    qword ptr [hwptr1], mm1                         ; Write TU2 TV2
   add     hwptr1, 8

ifdef IGX_SPECULAR_FIX
 IF ((SEND_SPEC EQ TRUE) AND (SEND_ALPHA EQ TRUE))
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edx, dword ptr _specflag[esp]
   test    edx, 1
   je      NoST1Specular

   mov     edi, dword ptr _specdwords[esp]
   movq    qword ptr [SpecularStream + edi], mm1
   add     edi, 8
   mov     dword ptr _specdwords[esp], edi

NoST1Specular:
;********************************************************
 ENDIF ; Specular And Alpha Blending
endif

ENDIF ; TX1
;********************************************************
;ST1 Setup
;********************************************************
endM
endif

;********************************************************
; VertexSpecular Macro, requires:
;
; vx1 - va1, vb1, or vc1
; _xColor - _aColor, _bColor, or _cColor
; _XST0 - _AST0, _BST0, or _CST0
; _XST1 - _AST1, _BST1, or _CST1
; AUTOSTRIP - If we are in the autostrip section, remove
;             the WRAP and FLAT shaded references
;
;********************************************************
VertexSpecular MACRO vx1:REQ, _xColor:REQ, _XST0:REQ, _XST1:REQ, AUTOSTRIP:REQ
LOCAL NoXYSpecular, NoZSpecular, NoWfbiSpecular, NoW0Specular
LOCAL NoST0Specular, NoST1Specular
;********************************************************
; Vert X
;
; mm0 - X | Y, shared with Wfbi later
; mm1 - RGBA Color, shared with ST1 later
; mm2 - Z
; mm3 - RHW or W0
; mm4 - ST0
; mm5 - Scale T | Scale S
; mm6 - Pixel Center | Pixel Center
; mm7 - Texel Offset | Texel Offset
;
;********************************************************
IF (BAD_FVF EQ TRUE)
   mov     edx, dword ptr [_X_Offset]
   movq    mm0, qword ptr [vx1 + edx]
ELSE
   movq    mm0, qword ptr sx[vx1]
ENDIF

   movd    mm1, dword ptr [_xColor]

   pfadd   mm0, mm6                                        ; Y + PIXEL_OFFSET | 
                                                           ; X + PIXEL_OFFSET

IF (SEND_Z EQ TRUE)
 IF (BAD_FVF EQ TRUE)
   mov     edx, dword ptr [_Z_Offset]
   movd    mm2, dword ptr [vx1 + edx]
 ELSE
   movd    mm2, dword ptr sz[vx1]
 ENDIF
ENDIF

   movq    qword ptr [hwptr1], mm0                         ; Write X + Y
   add     hwptr1, 8

IF (SEND_SPEC EQ TRUE)
;********************************************************
; Save off data for specular
;********************************************************
   mov     edi, dword ptr _specdwords[esp+4]
   movq    qword ptr [SpecularStream + edi], mm0
   add     edi, 8

 ; If we are not sending Z, lets send this seperately, if
 ; we are, lets pack Color and Z 
 IF (SEND_Z EQ FALSE)
   IF (SEND_FLAT EQ TRUE)
     mov     edi, dword ptr [_Spec_Offset]
     mov     edx, dword ptr [edi + va1]
   ELSE
     mov     edi, dword ptr [_Spec_Offset]
     mov     edx, dword ptr [edi + vx1]
   ENDIF

ifdef IGX_SPECULAR_FIX
   movd    edi, mm1
   xor     edx, edi
   and     edx, 16777215
   xor     edx, edi
   movd    mm0, edx
endif

   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi], mm0
   add     edi, 4
 ENDIF ; Sending Z

   mov     dword ptr _specdwords[esp+4], edi
;********************************************************
ENDIF ; Specular

IF ((SEND_W EQ TRUE) OR ((SEND_WFBI EQ TRUE) AND ((SEND_WBUFFER EQ TRUE) OR (SEND_HWFOG EQ TRUE))))
 IF (BAD_FVF EQ TRUE)
   mov     edx, dword ptr [_RHW_Offset]
   movd    mm3, dword ptr [vx1 + edx]
 ELSE
   movd    mm3, dword ptr rhw[vx1]
 ENDIF
ENDIF ; W, Wfbi & WBufer | HWFog

IF (SEND_TX0 EQ TRUE)
 IF ((SEND_WRAP EQ TRUE) AND (AUTOSTRIP EQ FALSE))
   movq    mm4, qword ptr [_XST0] ; Wrap Code
 ELSE ; Not Wrap
   mov     edx, dword ptr [__T0_Offset]
   movq    mm4, qword ptr [vx1 + edx]
 ENDIF ; Wrap
ENDIF ; TX0

IF ((SEND_W EQ TRUE) OR ((SEND_WFBI EQ TRUE) AND ((SEND_WBUFFER EQ TRUE) OR (SEND_HWFOG EQ TRUE))))
   punpckldq mm3, mm3                                      ; pB->rhw | pB->rhw
ENDIF ; W, Wfbi & WBufer | HWFog

ifdef DCT_FIX
   pfmul   mm3, qword ptr [__asm_data + _SCALEWS+8 ]       ; multiply w scale factor with correction.
endif

IF (SEND_TX0 EQ TRUE)
   pfmul   mm4, mm5                                        ; T * SCALE_T | S * SCALE_S
ENDIF ; TX0

;********************************************************
; COLOR
;********************************************************
;  if ( setupFlag & ( SST_SETUP_RGB | SST_SETUP_A ) )
IF (SEND_COLOR EQ TRUE)
   movd    dword ptr [hwptr1], mm1	                       ; Write Color
   add     hwptr1, 4
ENDIF ; Color
;********************************************************
; COLOR
;********************************************************

;********************************************************
; Z Setup
; 
; if( pRc->state & STATE_REQUIRES_WBUFFER )
;   if (pRc->state & STATE_REQUIRES_VERTEXFOG)
;     zX = (float)((255 - RGBA_GETALPHA(pX[FVFO_SPECULAR])) << 8);
;   else
;     zX = ZSCALE ( FLTP(pX)[FVFO_SZ] );
; else
;   Send down Z the way we got it
;
;********************************************************
IF (SEND_Z EQ TRUE)
 IF (SEND_WBUFFER EQ TRUE)
  IF (SEND_VFOG EQ TRUE)
;********************************************************
; Z-FOG Implementation
;
; (255 - RGBA_GETALPHA(pX[FVFO_SPECULAR])) << 8;
;********************************************************
   xor     edx, edx
   mov     edi, dword ptr [_Spec_Offset]
   mov     dl, byte ptr [edi + vx1 + 3]
   not     dl
   shl     edx, 8
   movd    mm2, edx
   pi2fd   mm2, mm2
;********************************************************
; Z-FOG Implementation
;********************************************************
  ENDIF
 ELSE ; Not WBuffer
 
   pfmul   mm2, qword ptr [__asm_data + _SCALEZ]           ; 0 | Z * SCALEZ
 ENDIF ; WBuffer

   movd    dword ptr [hwptr1], mm2                         ; Write Z
   add     hwptr1, 4

 IF (SEND_SPEC EQ TRUE)
;********************************************************
; Save off data for Specular pass
;********************************************************
;   mov     dword ptr _temp2[esp], esi
  
  ; If we are sending color and Z, Pack the writes
  IF (SEND_COLOR EQ TRUE)
   IF (SEND_FLAT EQ TRUE)
     mov     edi, dword ptr [_Spec_Offset]
     mov     edx, dword ptr [edi + va1]
   ELSE
     mov     edi, dword ptr [_Spec_Offset]
     mov     edx, dword ptr [edi + vx1]
   ENDIF

ifdef IGX_SPECULAR_FIX
   movd    edi, mm1
   xor     edx, edi
   and     edx, 16777215
   xor     edx, edi
   movd    mm0, edx
endif

   movd    mm0, edx
   punpckldq  mm0, mm2

   mov     edi, dword ptr _specdwords[esp+4]
   movq    qword ptr [SpecularStream + edi], mm0
   add     edi, 8
   mov     dword ptr _specdwords[esp+4], edi
  ELSE ; No Color only Z
   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi], mm2
   add     edi, 4
   mov     dword ptr _specdwords[esp+4], edi
  ENDIF ; If color
;********************************************************
 ENDIF ; Specular
ENDIF ; Z
;********************************************************
; Z Setup
;********************************************************

;********************************************************
; Wfbi Setup
;
; if( pRc->state & STATE_REQUIRES_WBUFFER )
;   wX = WSCALE( FLTP(pX)[FVFO_RHW] );
; else
;   if (pRc->state & STATE_REQUIRES_VERTEXFOG)
;     wX = (float)(255 - RGBA_GETALPHA(pX[FVFO_SPECULAR]));
;	else if (pRc->state & STATE_REQUIRES_HWFOG)
;     wX = RHW
;
;********************************************************
IF (SEND_WFBI EQ TRUE)
 IF (SEND_WBUFFER EQ TRUE)
   movq    mm0, mm3
   pfmul   mm0, qword ptr [__asm_data + WBUFFER_SCALE_AW]  ; RHW * WBUFFER_SCALE_AW 
   pfadd   mm0, qword ptr [__asm_data + WBUFFER_SCALE_BW]  ; (RHW * WBUFFER_SCALE_AW) + WBUFFER_SCALE_BW
;#define WSCALE(a) ((a * pRc->aW) + pRc->bW)
 ELSE ; Not WBuffer 
  IF (SEND_VFOG EQ TRUE)
;********************************************************
; W-FOG Implementation
;
; 255 - RGBA_GETALPHA(pX[FVFO_SPECULAR]);
;********************************************************
   xor     edx, edx
   mov     edi, dword ptr [_Spec_Offset]
   mov     dl, byte ptr [edi + vx1 + 3]
   not     dl
   movd    mm0, edx
   pi2fd   mm0, mm0
;   (rgb) >> 24
;********************************************************
; W-FOG Implementation
;********************************************************
  ELSE ; No Vertex Fog
;  if (pRc->state & STATE_REQUIRES_HWFOG)
   mov     edx, dword ptr [__asm_data + RENDER_STATE]
   test    dh, 32
   je      SHORT ASendW

   movq    mm0, mm3                                        ; Load up RHW
  ENDIF ; Vertex Fog
 ENDIF ; Wbuffer

   movd    dword ptr [hwptr1], mm0                         ; Write Wfbi
   add     hwptr1, 4

 IF (SEND_SPEC EQ TRUE)
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi], mm0
   add     edi, 4
   mov     dword ptr _specdwords[esp+4], edi
;********************************************************
 ENDIF ; Specular

ENDIF ; Wfbi
;********************************************************
; Wfbi Setup
;********************************************************

;********************************************************
; W0 Setup
;********************************************************
IF (SEND_W EQ TRUE)
   movd    dword ptr [hwptr1], mm3                         ; Write W
   add     hwptr1, 4

ifdef IGX_SPECULAR_FIX
 IF ((SEND_SPEC EQ TRUE) AND (SEND_ALPHA EQ TRUE))
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edi, dword ptr _specdwords[esp+4]
   movd    dword ptr [SpecularStream + edi], mm3
   add     edi, 4
   mov     dword ptr _specdwords[esp+4], edi
;********************************************************
 ENDIF ; Specular And Alpha Blending
endif

ENDIF ; W
;********************************************************
; W0 Setup
;********************************************************

;********************************************************
; ST0 Setup
;********************************************************
IF (SEND_TX0 EQ TRUE)
   pfadd   mm4, mm7                                        ; (T * SCALE_T) + T_OFFSET |
                                                           ; (S * SCALE_S) + S_OFFSET
 IF (SEND_PC EQ TRUE)
   pfmul   mm4, mm3                                        ; ((T * SCALE_T) + T_OFFSET) * RHW |
                                                           ; ((S * SCALE_S) + S_OFFSET) * RHW
 ENDIF ; Perspective Correct

   movq    qword ptr [hwptr1], mm4                         ; Write TU1 TV1
   add     hwptr1, 8

ifdef IGX_SPECULAR_FIX
 IF ((SEND_SPEC EQ TRUE) AND (SEND_ALPHA EQ TRUE))
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edi, dword ptr _specdwords[esp+4]
   movq    qword ptr [SpecularStream + edi], mm4
   add     edi, 8
   mov     dword ptr _specdwords[esp+4], edi
;********************************************************
 ENDIF ; Specular And Alpha Blending
endif

ENDIF ; TX0
;********************************************************
; ST0 Setup 
;********************************************************

;********************************************************
;ST1 Setup
;********************************************************
IF (SEND_TX1 EQ TRUE)
 IF ((SEND_WRAP EQ TRUE) AND (AUTOSTRIP EQ FALSE))
   movq    mm1, qword ptr [_XST1]
 ELSE ; Not Wrap
   mov     edx, dword ptr [_T1_Offset]
   movq    mm1, qword ptr [vx1 + edx]					   ; T | S
 ENDIF ; Wrap
 
   pfmul   mm1, qword ptr [__asm_data + S1_SCALE]          ; T * SCALE_T | S * SCALE_S
   pfadd   mm1, qword ptr [__asm_data + S1_TEXEL_OFFSET]   ; ((T * SCALE_T) + T_OFFSET) |
                                                           ; ((S * SCALE_S) + S_OFFSET)
 IF (SEND_PC EQ TRUE)
   pfmul   mm1, mm3                                        ; ((T * SCALE_T) + T_OFFSET) * RHW | 
                                                           ; ((S * SCALE_S) + S_OFFSET) * RHW
 ENDIF ; Perpsecitve Correct

   movq    qword ptr [hwptr1], mm1                         ; Write TU2 TV2
   add     hwptr1, 8

ifdef IGX_SPECULAR_FIX
 IF ((SEND_SPEC EQ TRUE) AND (SEND_ALPHA EQ TRUE))
;********************************************************
; Save off data for Specular pass
;********************************************************
   mov     edi, dword ptr _specdwords[esp+4]
   movq    qword ptr [SpecularStream + edi], mm1
   add     edi, 8
   mov     dword ptr _specdwords[esp+4], edi
;********************************************************
 ENDIF ; Specular And Alpha Blending
endif

ENDIF ; TX1
;********************************************************
;ST1 Setup
;********************************************************
endM

;********************************************************
; Vertex Macro, requires:
;
; vx1 - va1, vb1, or vc1
; _xColor - _aColor, _bColor, or _cColor
; _XST0 - _AST0, _BST0, or _CST0
; _XST1 - _AST1, _BST1, or _CST1
; AUTOSTRIP - If we are in the autostrip section, remove
;             the WRAP and FLAT shaded references
;
;********************************************************
VertexNoSpecular MACRO vx1:REQ, _xColor:REQ, _XST0:REQ, _XST1:REQ, AUTOSTRIP:REQ
LOCAL NoXYSpecular, NoZSpecular, NoWfbiSpecular, NoW0Specular
LOCAL NoST0Specular, NoST1Specular
;********************************************************
; Vert X
;
; mm0 - X | Y, shared with Wfbi later
; mm1 - RGBA Color, shared with ST1 later
; mm2 - Z
; mm3 - RHW or W0
; mm4 - ST0
; mm5 - Scale T | Scale S
; mm6 - Pixel Center | Pixel Center
; mm7 - Texel Offset | Texel Offset
;
;********************************************************
IF (BAD_FVF EQ TRUE)
   mov     edx, dword ptr [_X_Offset]
   movq    mm0, qword ptr [vx1 + edx]
ELSE
   movq    mm0, qword ptr sx[vx1]
ENDIF

   movd    mm1, dword ptr [_xColor]

   pfadd   mm0, mm6                                        ; Y + PIXEL_OFFSET | 
                                                           ; X + PIXEL_OFFSET

IF (SEND_Z EQ TRUE)
 IF (BAD_FVF EQ TRUE)
   mov     edx, dword ptr [_Z_Offset]
   movd    mm2, dword ptr [vx1 + edx];sz[vx1]
 ELSE
   movd    mm2, dword ptr sz[vx1]
 ENDIF
ENDIF

   movq    qword ptr [hwptr1], mm0                         ; Write X + Y
   add     hwptr1, 8

IF ((SEND_W EQ TRUE) OR ((SEND_WFBI EQ TRUE) AND ((SEND_WBUFFER EQ TRUE) OR (SEND_HWFOG EQ TRUE))))
 IF (BAD_FVF EQ TRUE)
   mov     edx, dword ptr [_RHW_Offset]
   movd    mm3, dword ptr [vx1 + edx]
 ELSE
   movd    mm3, dword ptr rhw[vx1]
 ENDIF
ENDIF ; W, Wfbi & WBufer | HWFog

IF (SEND_TX0 EQ TRUE)
 IF ((SEND_WRAP EQ TRUE) AND (AUTOSTRIP EQ FALSE))
   movq    mm4, qword ptr [_XST0] ; Wrap Code
 ELSE ; Not Wrap
   mov     edx, dword ptr [__T0_Offset]
   movq    mm4, qword ptr [vx1 + edx]
 ENDIF ; Wrap
ENDIF ; TX0

IF ((SEND_W EQ TRUE) OR ((SEND_WFBI EQ TRUE) AND ((SEND_WBUFFER EQ TRUE) OR (SEND_HWFOG EQ TRUE))))
   punpckldq mm3, mm3                                      ; pB->rhw | pB->rhw
ENDIF ; W, Wfbi & WBufer | HWFog

ifdef DCT_FIX
   pfmul   mm3, qword ptr [__asm_data + _SCALEWS+8 ]       ; multiply w scale factor with correction.
endif

IF (SEND_TX0 EQ TRUE)
   pfmul   mm4, mm5                                        ; T * SCALE_T | S * SCALE_S
ENDIF ; TX0

;********************************************************
; COLOR
;********************************************************
;  if ( setupFlag & ( SST_SETUP_RGB | SST_SETUP_A ) )
IF (SEND_COLOR EQ TRUE)
   movd    dword ptr [hwptr1], mm1	                       ; Write Color
   add     hwptr1, 4
ENDIF ; Color
;********************************************************
; COLOR
;********************************************************

;********************************************************
; Z Setup
; 
; if( pRc->state & STATE_REQUIRES_WBUFFER )
;   if (pRc->state & STATE_REQUIRES_VERTEXFOG)
;     zX = (float)((255 - RGBA_GETALPHA(pX[FVFO_SPECULAR])) << 8);
;   else
;     zX = ZSCALE ( FLTP(pX)[FVFO_SZ] );
; else
;   Send down Z the way we got it
;
;********************************************************
IF (SEND_Z EQ TRUE)
 IF (SEND_WBUFFER EQ TRUE)
  IF (SEND_VFOG EQ TRUE)
;********************************************************
; Z-FOG Implementation
;
; (255 - RGBA_GETALPHA(pX[FVFO_SPECULAR])) << 8;
;********************************************************
   xor     edx, edx
   mov     edi, dword ptr [_Spec_Offset]
   mov     dl, byte ptr [edi + vx1 + 3]
   not     dl
   shl     edx, 8
   movd    mm2, edx
   pi2fd   mm2, mm2
;********************************************************
; Z-FOG Implementation
;********************************************************
  ENDIF
 ELSE ; Not WBuffer
 
   pfmul   mm2, qword ptr [__asm_data + _SCALEZ]           ; 0 | Z * SCALEZ
 ENDIF ; WBuffer

   movd    dword ptr [hwptr1], mm2                         ; Write Z
   add     hwptr1, 4

ENDIF ; Z
;********************************************************
; Z Setup
;********************************************************

;********************************************************
; Wfbi Setup
;
; if( pRc->state & STATE_REQUIRES_WBUFFER )
;   wX = WSCALE( FLTP(pX)[FVFO_RHW] );
; else
;   if (pRc->state & STATE_REQUIRES_VERTEXFOG)
;     wX = (float)(255 - RGBA_GETALPHA(pX[FVFO_SPECULAR]));
;	else if (pRc->state & STATE_REQUIRES_HWFOG)
;     wX = RHW
;
;********************************************************
IF (SEND_WFBI EQ TRUE)
 IF (SEND_WBUFFER EQ TRUE)
   movq    mm0, mm3
   pfmul   mm0, qword ptr [__asm_data + WBUFFER_SCALE_AW]  ; RHW * WBUFFER_SCALE_AW 
   pfadd   mm0, qword ptr [__asm_data + WBUFFER_SCALE_BW]  ; (RHW * WBUFFER_SCALE_AW) + WBUFFER_SCALE_BW
;#define WSCALE(a) ((a * pRc->aW) + pRc->bW)
 ELSE ; Not WBuffer 
  IF (SEND_VFOG EQ TRUE)
;********************************************************
; W-FOG Implementation
;
; 255 - RGBA_GETALPHA(pX[FVFO_SPECULAR]);
;********************************************************
   xor     edx, edx
   mov     edi, dword ptr [_Spec_Offset]
   mov     dl, byte ptr [edi + vx1 + 3]
   not     dl
   movd    mm0, edx
   pi2fd   mm0, mm0
;   (rgb) >> 24
;********************************************************
; W-FOG Implementation
;********************************************************
  ELSE ; No Vertex Fog
;  if (pRc->state & STATE_REQUIRES_HWFOG)
   mov     edx, dword ptr [__asm_data + RENDER_STATE]
   test    dh, 32
   je      SHORT ASendW

   movq    mm0, mm3                                        ; Load up RHW
  ENDIF ; Vertex Fog
 ENDIF ; Wbuffer

   movd    dword ptr [hwptr1], mm0                         ; Write Wfbi
   add     hwptr1, 4

ENDIF ; Wfbi
;********************************************************
; Wfbi Setup
;********************************************************

;********************************************************
; W0 Setup
;********************************************************
IF (SEND_W EQ TRUE)
   movd    dword ptr [hwptr1], mm3                         ; Write W
   add     hwptr1, 4

ENDIF ; W
;********************************************************
; W0 Setup
;********************************************************

;********************************************************
; ST0 Setup
;********************************************************
IF (SEND_TX0 EQ TRUE)
   pfadd   mm4, mm7                                        ; (T * SCALE_T) + T_OFFSET |
                                                           ; (S * SCALE_S) + S_OFFSET
 IF (SEND_PC EQ TRUE)
   pfmul   mm4, mm3                                        ; ((T * SCALE_T) + T_OFFSET) * RHW |
                                                           ; ((S * SCALE_S) + S_OFFSET) * RHW
 ENDIF ; Perspective Correct

   movq    qword ptr [hwptr1], mm4                         ; Write TU1 TV1
   add     hwptr1, 8

ENDIF ; TX0
;********************************************************
; ST0 Setup 
;********************************************************

;********************************************************
;ST1 Setup
;********************************************************
IF (SEND_TX1 EQ TRUE)
 IF ((SEND_WRAP EQ TRUE) AND (AUTOSTRIP EQ FALSE))
   movq    mm1, qword ptr [_XST1]
 ELSE ; Not Wrap
   mov     edx, dword ptr [_T1_Offset]
   movq    mm1, qword ptr [vx1 + edx]					   ; T | S
 ENDIF ; Wrap
 
   pfmul   mm1, qword ptr [__asm_data + S1_SCALE]          ; T * SCALE_T | S * SCALE_S
   pfadd   mm1, qword ptr [__asm_data + S1_TEXEL_OFFSET]   ; ((T * SCALE_T) + T_OFFSET) |
                                                           ; ((S * SCALE_S) + S_OFFSET)
 IF (SEND_PC EQ TRUE)
   pfmul   mm1, mm3                                        ; ((T * SCALE_T) + T_OFFSET) * RHW | 
                                                           ; ((S * SCALE_S) + S_OFFSET) * RHW
 ENDIF ; Perpsecitve Correct

   movq    qword ptr [hwptr1], mm1                         ; Write TU2 TV2
   add     hwptr1, 8

ENDIF ; TX1
;********************************************************
;ST1 Setup
;********************************************************
endM


;********************************************************
; Core Triangle Rendering Macro
;
; Direct copy of DrawTriIdx2MainAsm_K62O, minus the 
; flag checks.  Added ifdefs for quick Special case creation
;
;********************************************************
CoreTriangleRendering MACRO
va1        TEXTEQU     <eax>
color1     TEXTEQU     <eax>
vb1        TEXTEQU     <ebx>
vc1        TEXTEQU     <ecx>
verts2     TEXTEQU     <edx>
s0         TEXTEQU     <edx>
room2      TEXTEQU     <edx>
count1     TEXTEQU     <edi>
area1      TEXTEQU     <edi>
rhw1       TEXTEQU     <edi>
hwptr1     TEXTEQU     <esi>
post_idx1  TEXTEQU     <ebp>
hwptr_old  TEXTEQU     <ebp>

;   add     esp, 4

IF (SEND_SPEC EQ TRUE)
;********************************************************
; Initialize the Specular Stream 
; 
; Specular Stream
; [Number of bytes in stream] 
; [ Data ]
; 
; ~
;
; _specdwords = Number of bytes in stream
; 
; First DWORD In Specular stream is incremented by accessing it
; and rewriting the incremented number of triangles in stream
;
   mov     dword ptr _temp1[esp+4], edx
   mov     edx, 0
   mov     dword ptr _specdwords[esp+4], edx
   mov     edx, dword ptr _temp1[esp+4]
;********************************************************
ENDIF

   mov     dword ptr _strip_cont[esp+4],BIT_23
   mov     dword ptr _fan_cont[esp+4],BIT_22

   test    hwptr1, 4

   movq    mm5, qword ptr [__asm_data + _SCALE_S]

   movd    mm6, dword ptr [__asm_data + _PIXEL_CENTER]

   movq    mm7, qword ptr [__asm_data + _TEXEL_OFFSET_S]

ifdef DCT_FIX
   pfmul   mm7, qword ptr [__asm_data + _SCALEWS+8 ]       ; Pre multiply S& T scale factors with correction.
endif

   punpckldq mm6, mm6                                      ; Pixel Center | Pixel Center

   jz      @F

   add     hwptr1, 4
   mov     dword ptr [hwptr1-4], 0                         ; Quad Word Align the Fifo
@@:
   mov     dword ptr _hwptr[esp+4], hwptr1

ifndef MS_VIEW
align 32
else
align 16
endif
do_cycle_k6m:

   mov     hwptr1, dword ptr _hwptr[esp+4]

   dec     count1

   js      done_render_k6m                                 ; If we hit sign, we are done going through the list

   mov     dword ptr _count[esp+4], count1

;********************************************************
; Culling Check
;********************************************************
   Cull_3DNow
;********************************************************
; Culling Check
;********************************************************

;********************************************************
; Specular Check
;********************************************************
IF (SEND_ALPHA EQ TRUE)
   mov     edx, dword ptr [__asm_data + CMD_ALPHA_BLEND_SPECULAR]
   mov     dword ptr _SpecularTriCmd[esp+4], edx
   mov     edx, dword ptr [__asm_data + CMD_CONT_ALPHA_BLEND_SPECULAR]
   mov     dword ptr _SpecularContCmd[esp+4], edx
ELSE ; No Alpha blending
   mov     edx, dword ptr [__asm_data + CMD_NO_ALPHA_BLEND_SPECULAR]
   mov     dword ptr _SpecularTriCmd[esp+4], edx
   mov     edx, dword ptr [__asm_data + CMD_CONT_NO_ALPHA_BLEND_SPECULAR]
   mov     dword ptr _SpecularContCmd[esp+4], edx
ENDIF ; Alpha Blending

IF (SEND_SPEC EQ TRUE)
   Specular_Test_And_Build_Tri_Packet
ENDIF
;********************************************************
; Specular Check
;********************************************************

;********************************************************
; Wrap Implementation
;********************************************************
IF (SEND_WRAP EQ TRUE)
 IF (SEND_TX0 EQ TRUE)
   mov     edx, dword ptr [__T0_Offset]
   Wrap    edx, _AST0, _BST0, _CST0, (1 SHL BIT_WRAP_T0_S), (1 SHL BIT_WRAP_T0_T)
 ENDIF

 IF (SEND_TX1 EQ TRUE)
   mov     edx, dword ptr [_T1_Offset]
   Wrap    edx, _AST1, _BST1, _CST1, (1 SHL BIT_WRAP_T1_S), (1 SHL BIT_WRAP_T1_T)
 ENDIF

   movq    mm5, qword ptr [__asm_data + _SCALE_S]
   movd    mm6, dword ptr [__asm_data + _PIXEL_CENTER]

   movq    mm7, qword ptr [__asm_data + _TEXEL_OFFSET_S]

ifdef DCT_FIX
   pfmul   mm7, qword ptr [__asm_data + _SCALEWS+8 ]       ; Pre multiply S& T scale factors with correction.
endif

   punpckldq mm6, mm6
ENDIF
;********************************************************
; Wrap Implementation
;********************************************************

;********************************************************
; Send down Triangle Header Packet
;********************************************************
;  movd    mm0, dword ptr [__asm_data + CMD_TRI]
   movd    mm0, dword ptr [__asm_data + CMD_START]
   movd    dword ptr [hwptr1], mm0					       ; Write Header
   add     hwptr1, 4

;********************************************************
; Color Precalc!
;
; if ( FLAG_COLOR )
;   if ( Specular Enabled && No Texture )
;     CLAMP888(aColor, pA[FVFO_COLOR], pA[FVFO_SPECULAR]);
;     if ( Flat Shading Enabled )
;	    bColor = cColor = aColor
;	  else 
;	    CLAMP888(bColor, pB[FVFO_COLOR], pB[FVFO_SPECULAR]);
;		CLAMP888(cColor, pC[FVFO_COLOR], pC[FVFO_SPECULAR]);
;   else
;      aColor = va1 color
;	  if ( Flat SHading Enabled )
;	    bColor = cColor = aColor
;	  else
;	    bColor = vb1 color
;		cColor = vc1 color
;
; CLAMP888(xColor, pX[FVFO_COLOR], pX[FVFO_SPECULAR]);
;
; If the diffuse color index == 128, then xColor = FFFFFFFFh
; 
; Add the Specular to the Diffuse Color without
; Overflow into the higher packed bytes
;********************************************************
IF (SEND_COLOR EQ TRUE)
 IF ((SEND_SPEC EQ TRUE) AND (SEND_TX0 EQ FALSE) AND (SEND_TX1 EQ FALSE))
   mov     edi, 0fefefeh

   movd      mm4, edi
   punpckldq mm4, mm4

  IF (BAD_FVF EQ TRUE)
   mov        edx, dword ptr [_Color_Offset]
   cmp        edx, 128
   je         PrecalcNoColorInFVF

   movd       mm1, dword ptr [va1 + edx]
   movd       mm0, dword ptr [vb1 + edx]
   
   jmp        short PrecalcPastNoColorInFVF

PrecalcNoColorInFVF:

   pcmpeqd    mm0, mm0
   pcmpeqd    mm1, mm1

PrecalcPastNoColorInFVF:
  ELSE
   movd       mm1, dword ptr color[va1]
   movd       mm0, dword ptr color[vb1]
  ENDIF

   punpckldq  mm1, mm0

   mov     edi, dword ptr [_Spec_Offset]

   movd    mm0, dword ptr [edi + va1]
   movd    mm2, dword ptr [edi + vb1]
   movd    mm3, dword ptr [edi + vc1]
   
   punpckldq  mm0, mm2
   pand       mm0, mm4
   paddusb    mm1, mm0

  IF (BAD_FVF EQ TRUE)
   cmp     edx, 128
   je      CPrecalcNoColorInFVF

   movd    mm0, dword ptr [vc1 + edx]

   jmp     short CPrecalcPastNoColorInFVF

CPrecalcNoColorInFVF:
   
   pcmpeqd mm0, mm0

CPrecalcPastNoColorInFVF:
  ELSE
   movd    mm0, dword ptr color[vc1]
  ENDIF

   movd    dword ptr [_aColor], mm1

  IF (SEND_FLAT EQ TRUE)
   movd    dword ptr [_bColor], mm1
   movd    dword ptr [_cColor], mm1
  ELSE ; No Flat Shading
   pand    mm3, mm4

   paddusb mm0, mm3

   punpckhdq mm1, mm1
   movd    dword ptr [_bColor], mm1
   movd    dword ptr [_cColor], mm0
  ENDIF ; Flat Shading
 ELSE ; No Specular Enabled
  IF (BAD_FVF EQ TRUE)
   mov     edx, dword ptr [_Color_Offset]
   cmp     edx, 128
   je      PrecalcNoSpecNoColorInFVF

   movd    mm0, dword ptr [va1 + edx]

   jmp     short PrecalcNoSpecPastNoColorInFVF

PrecalcNoSpecNoColorInFVF:

   pcmpeqd mm0, mm0

PrecalcNoSpecPastNoColorInFVF:

  ELSE
   movd    mm0, dword ptr color[va1]
  ENDIF

  IF (SEND_FLAT EQ TRUE)
   movd    dword ptr [_aColor], mm0
   movd    dword ptr [_bColor], mm0
   movd    dword ptr [_cColor], mm0
  ELSE ; No Flat Shading
   IF (BAD_FVF EQ TRUE)
    cmp     edx, 128
	je      PrecalcNoSpecNoFlatNoColorInFVF

    movd    mm1, dword ptr [vb1 + edx]
    movd    mm2, dword ptr [vc1 + edx]

    jmp     short PrecalcNoSpecNoFlatPastNoColorInFVF

PrecalcNoSpecNoFlatNoColorInFVF:

    pcmpeqd mm1, mm1
	pcmpeqd mm2, mm2

PrecalcNoSpecNoFlatPastNoColorInFVF:

   ELSE
    movd    mm1, dword ptr color[vb1]
    movd    mm2, dword ptr color[vc1]
   ENDIF   

   movd    dword ptr [_aColor], mm0
   movd    dword ptr [_bColor], mm1
   movd    dword ptr [_cColor], mm2
  ENDIF ; Flat Shading
 ENDIF ; Specular
ENDIF ; Send_Color
PrecalcNoColor:
;********************************************************
; Color Precalc!
;********************************************************
IF (SEND_SPEC EQ TRUE)
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      NoSpecular

   VertexSpecular va1, _aColor, _AST0, _AST1, FALSE
   VertexSpecular vb1, _bColor, _BST0, _BST1, FALSE
   VertexSpecular vc1, _cColor, _CST0, _CST1, FALSE
 
   jmp     PastSpecularCheck 

NoSpecular:  
   VertexNoSpecular va1, _aColor, _AST0, _AST1, FALSE
   VertexNoSpecular vb1, _bColor, _BST0, _BST1, FALSE
   VertexNoSpecular vc1, _cColor, _CST0, _CST1, FALSE

PastSpecularCheck:
ELSE ; Not sending specular no matter what
   VertexNoSpecular va1, _aColor, _AST0, _AST1, FALSE
   VertexNoSpecular vb1, _bColor, _BST0, _BST1, FALSE
   VertexNoSpecular vc1, _cColor, _CST0, _CST1, FALSE
ENDIF ; Specular
   
   Autostrip_Tri_To_Strip
      
;   jmp     do_cycle_k6m

;********************************************************
; CHECK FOR WRAP
;********************************************************
IF (SEND_WRAP EQ TRUE)
   mov     edx, dword ptr [__asm_data + _RC]
   mov     edi, dword ptr [edx + OFFSET_RC_WRAPT0]
   or      edi, dword ptr [edx + OFFSET_RC_WRAPT1]
   mov     edx, dword ptr _verts[esp+4]
   test    edi, edi
   mov     edi,dword ptr _count[esp+4]                       ;count1 = _count[esp]
   jne     do_cycle_k6m
ENDIF ; Wrap
;********************************************************
; CHECK FOR WRAP
;********************************************************

;********************************************************
; CHECK FOR FLAT SHADING
;********************************************************
IF (SEND_FLAT EQ TRUE)
   mov     edx, dword ptr _localflags[esp+4]
   test    edx, FLAG_FLAT
   jne     do_cycle_k6m
ENDIF ; Flat
;********************************************************
; CHECK FOR FLAT SHADING
;********************************************************

;  jmp      do_cycle_k6m
   mov     dword ptr _last[esp+4],BIT_22

prim_continue:
;  We found a strip or a fan
   dec     edi                                             ;count1--

   mov     dword ptr _temp1[esp+4], esi
   mov     esi, _hwptr[esp+4]                                ;save hwptr1
   js      done_render_k6m

   mov     esi, dword ptr _temp1[esp+4]

   mov     dword ptr _count[esp+4],edi

;********************************************************
; Culling Check
;********************************************************
   Cull_3DNow
;********************************************************
; Culling Check
;********************************************************

;********************************************************
; Specular Check
;********************************************************
IF (SEND_SPEC EQ TRUE)
   Specular_Test_And_Build_Autostrip_Packet
ENDIF ; Specular
;********************************************************
; Specular Check
;********************************************************

   mov     eax, _last[esp+4]
   mov     ebx, dword ptr [__asm_data + CMD_CONT]

   mov     edx,esi                                         ;tmp1 = test1
   xor     esi,eax

   mov     dword ptr _last[esp+4],edx
   xor     esi,BIT_22

   and     esi,BIT_22
 
   or      ebx, esi                                        ;header1 |= test1	 

   mov     esi, _hwptr[esp+4]                                ;hwptr1 = _hwptr[esp]

;********************************************************
; Send down Triangle Contiunation Header Packet
;********************************************************
   mov     [esi], ebx                                      ;*hwptr = header1
   add     esi, 4

;********************************************************
; Color Precalc!
;
; if ( FLAG_COLOR )
;   if ( Specular Enabled && No Texture )
;     CLAMP888(cColor, pC[FVFO_COLOR], pC[FVFO_SPECULAR]);
;   else
;      cColor = vc1 color
;  
; CLAMP888(xColor, pX[FVFO_COLOR], pX[FVFO_SPECULAR]);
;
; If the diffuse color index == 128, then xColor = FFFFFFFFh
; 
; Add the Specular to the Diffuse Color without
; Overflow into the higher packed bytes
;********************************************************
IF (SEND_COLOR EQ TRUE)
 IF ((SEND_SPEC EQ TRUE) AND (SEND_TX0 EQ FALSE) AND (SEND_TX1 EQ FALSE))

   mov     edi, 0fefefeh

  IF (BAD_FVF EQ TRUE)
   mov     edx, dword ptr [_Color_Offset]   
   cmp     edx, 128
   je      DPrecalcNoColorInFVF

   movd    mm1, dword ptr [vc1 + edx]

   jmp     short DPrecalcPastNoColorInFVF

DPrecalcNoColorInFVF:
   
   pcmpeqd mm1, mm1

DPrecalcPastNoColorInFVF:

  ELSE
   movd    mm1, dword ptr color[vc1]
  ENDIF

   movd    mm4, edi

   mov     edx, dword ptr [_Spec_Offset]
   movd    mm0, dword ptr [edx + vc1]
   pand    mm0, mm4
   paddusb mm1, mm0

   movd    dword ptr [_cColor], mm1
 ELSE ; Not Specular   
  IF (BAD_FVF EQ TRUE)
   mov     edx, dword ptr [_Color_Offset]
   cmp     edx, 128
   je      DPrecalcNoSpecNoColorInFVF

   movd    mm0, dword ptr [vc1 + edx]

   jmp     short DPrecalcNoSpecPastNoColorInFVF
DPrecalcNoSpecNoColorInFVF:

   pcmpeqd mm0, mm0

DPrecalcNoSpecPastNoColorInFVF:

  ELSE
   movd    mm0, dword ptr color[vc1]
  ENDIF

   movd    dword ptr [_cColor], mm0
 ENDIF ; Specular
ENDIF ; Color
;********************************************************
; Color Precalc!
;********************************************************
IF (SEND_SPEC EQ TRUE)
   mov     edx, dword ptr _specflag[esp+4]
   test    edx, 1
   je      AS_NoSpecular

   VertexSpecular vc1, _cColor, _CST0, _CST1, TRUE
 
   jmp     AS_PastSpecularCheck 

AS_NoSpecular:  
   VertexNoSpecular vc1, _cColor, _CST0, _CST1, TRUE

AS_PastSpecularCheck:
ELSE ; Not sending specular no matter what
   VertexNoSpecular vc1, _cColor, _CST0, _CST1, TRUE
ENDIF ; Specular

;   Vertex vc1, _cColor, _CST0, _CST1, TRUE
  
   Autostrip_Tri_To_Strip

;  jmp     do_cycle_k6m

   jmp     prim_continue

ifndef MS_VIEW
align 32
else
align 16
endif
culled_tri_k6m:
   add     post_idx1,12
   mov     verts2,dword ptr _verts[esp+4]
   mov     va1,dword ptr [post_idx1]
   mov     vb1,dword ptr [post_idx1+4]

   mov     vc1,dword ptr [post_idx1+8]

   mov     count1,dword ptr _count[esp+4]

   jmp     do_cycle_k6m
   nop

ifndef MS_VIEW
align 32
else
align 16
endif
done_render_k6m:
;********************************************************
; Specular Stream Execution
;********************************************************
IF (SEND_SPEC EQ TRUE)

;********************************************************
; Setup Hardware for Specular Pass
;********************************************************
   SpecularSetupHardware
;********************************************************
; Setup Hardware for Specular Pass
;********************************************************

   mov     ebx, dword ptr _specdwords[esp+4]
   shr     ebx, 2							; # of dwords to write
   cmp     ebx, 0
   je      SpecularStreamEmpty

   mov     edi, hwptr1

   mov     ecx, ebx

   sub     ecx, 08h							; is there at least 8 dwords to write?
   lea     esi, [SpecularStream]
   jl      SHORT SecondPass_BlockMoveDone

align 16
SecondPass_NextBlockMove:
	movq	mm0, qword ptr [esi+000h]
	movq	mm1, qword ptr [esi+008h]
	add		edi, 020h						; pre-increment destination pointer
	movq	mm2, qword ptr [esi+010h]
	movq	mm3, qword ptr [esi+018h]
	movq	qword ptr [edi-020h], mm0
	movq	qword ptr [edi-018h], mm1
	add		esi, 020h						; post-increment source pointer
	movq	qword ptr [edi-010h], mm2
	sub		ecx, 08h						; we wrote 8 more dwords
	movq	qword ptr [edi-008h], mm3
	jge		SHORT SecondPass_NextBlockMove

SecondPass_BlockMoveDone:
	add		ecx, 08h
	rep     movsd

    mov     hwptr1, edi

SpecularStreamEmpty:
;********************************************************
; Restore hardware to previous state
;********************************************************
   SpecularRestoreHardware
;********************************************************
; Restore hardware to previous state
;********************************************************
ENDIF ; Specular
;********************************************************
; Specular Stream Execution
;********************************************************

   mov     hwptr_old,dword ptr[__asm_data + _FIFO_PTR]
   mov     dword ptr[__asm_data + _FIFO_PTR],hwptr1

   sub     hwptr1,hwptr_old
   mov     room2,dword ptr [__asm_data + _FIFO_ROOM]

   shr     hwptr1,2
   sub     room2,hwptr1

   mov     dword ptr [__asm_data + _FIFO_ROOM],room2

;   sub     esp, 4

   ret
endM

; _Draw_K62O_SPEC_COLOR_Z_W_PC_TX0
;
; 3D Winbench 2000 Test 1 and 2 all non-guardband clipped polygons
; 3DMark 2000 Test 2 ( Adventure ) barrells and fountain draw
; with this
;
; FLAG_SPEC, FLAG_COLOR, FLAG_Z, FLAG_W, FLAG_TX0, FLAG_PC
; Specular, Diffuse Color, Z, W0, Texture Stage 1, Perspective Correct
;
ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _Draw_K62O_SPEC_COLOR_Z_W_PC_TX0
_Draw_K62O_SPEC_COLOR_Z_W_PC_TX0   PROC     NEAR
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
	SEND_HWFOG      = FALSE
	BAD_FVF         = FALSE
    CoreTriangleRendering
_Draw_K62O_SPEC_COLOR_Z_W_PC_TX0   ENDP

; _Draw_K62O_SPEC_COLOR_Z_WFBI_VFOG_W_PC_TX0_TX1_BADFVF
; 
; 3DMark 2000 Test One ( Helicopter ) Mountains are drawn with
; this
;
ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _Draw_K62O_SPEC_COLOR_Z_WFBI_VFOG_W_PC_TX0_TX1_BADFVF
_Draw_K62O_SPEC_COLOR_Z_WFBI_VFOG_W_PC_TX0_TX1_BADFVF   PROC     NEAR
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
	SEND_TX1		= TRUE
	SEND_WRAP		= FALSE
	SEND_NAPALM		= CHECK
	SEND_HWFOG      = FALSE
	BAD_FVF         = TRUE
    CoreTriangleRendering
_Draw_K62O_SPEC_COLOR_Z_WFBI_VFOG_W_PC_TX0_TX1_BADFVF   ENDP

; _Draw_K62O_COLOR_Z_W_PC_TX0
; 
; 3D Winbench 2000 Tests 3 - 9
; Test 3 RustValley - Helicopter
; Test 4 Canyon - Space Ship
; Test 5 Chamber - Walls and stairs
; Test 6 Stations - Domes on stations and second pass triangles
; Test 7 Islands ( Fog ) - None
; Test 8 Racetrack - All non-guardband clipped triangles
; Test 9 Chapel - All non-guardband clipped triangles except
;                 the animated leaf pattern
;
ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _Draw_K62O_COLOR_Z_W_PC_TX0
_Draw_K62O_COLOR_Z_W_PC_TX0   PROC     NEAR
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
	SEND_HWFOG      = FALSE
	BAD_FVF         = FALSE
    CoreTriangleRendering
_Draw_K62O_COLOR_Z_W_PC_TX0   ENDP

; _Draw_K62O_COLOR_Z_WFBI_VFOG_W_PC_TX0
; 
; 3DMark 2000 Test 1 ( Helicopter ) tanks drawn
; with this
;
ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _Draw_K62O_COLOR_Z_WFBI_VFOG_W_PC_TX0
_Draw_K62O_COLOR_Z_WFBI_VFOG_W_PC_TX0   PROC     NEAR
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
	SEND_HWFOG      = FALSE
	BAD_FVF         = FALSE
    CoreTriangleRendering
_Draw_K62O_COLOR_Z_WFBI_VFOG_W_PC_TX0   ENDP

; _Draw_K62O_COLOR_Z_WFBI_VFOG_W_PC_ALPHA_TX0
; 
; 3DMark 2000 Test 1 ( Helicopter ) trees drawn
; with this
;
ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _Draw_K62O_COLOR_Z_WFBI_VFOG_W_PC_ALPHA_TX0
_Draw_K62O_COLOR_Z_WFBI_VFOG_W_PC_ALPHA_TX0   PROC     NEAR
	SEND_SPEC		= FALSE
	SEND_FLAT		= FALSE
	SEND_COLOR		= TRUE	   
	SEND_WBUFFER	= FALSE
	SEND_Z			= TRUE
	SEND_WFBI		= TRUE
	SEND_VFOG		= TRUE
	SEND_W			= TRUE
	SEND_PC			= TRUE
	SEND_ALPHA		= TRUE
	SEND_TX0		= TRUE
	SEND_TX1		= FALSE
	SEND_WRAP		= FALSE
	SEND_NAPALM		= CHECK
	SEND_HWFOG      = FALSE
	BAD_FVF         = FALSE
    CoreTriangleRendering
_Draw_K62O_COLOR_Z_WFBI_VFOG_W_PC_ALPHA_TX0   ENDP

; _Draw_K62O_COLOR_Z_W_PC_TX0_TX1
; 
; 3D Winbench 2000 Tests 3 - 9
; Test 3 RustValley - Ground
; Test 4 Canyon - Canyon Walls
; Test 5 Chamber - Reflective Floor
; Test 6 Stations - Body of the stations ( Currently Fallback Mode 12/20 )
; Test 7 Islands ( Fog ) - All triangles not guardband clipped
; Test 8 Racetrack - None
; Test 9 Chapel - Animated leave pattern behind ball
; 3DMark 2000 Test 2, Buildings
;
ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _Draw_K62O_COLOR_Z_W_PC_TX0_TX1
_Draw_K62O_COLOR_Z_W_PC_TX0_TX1   PROC     NEAR
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
	SEND_HWFOG      = FALSE
	BAD_FVF         = FALSE
    CoreTriangleRendering
_Draw_K62O_COLOR_Z_W_PC_TX0_TX1   ENDP

; _Draw_K62O_COLOR_Z_W_PC_TX0_TX1_BADFVF
; 
; 3DMark 2000 Test 2, Buildings
;
ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _Draw_K62O_COLOR_Z_W_PC_TX0_TX1_BADFVF
_Draw_K62O_COLOR_Z_W_PC_TX0_TX1_BADFVF   PROC     NEAR
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
	SEND_HWFOG      = FALSE
	BAD_FVF         = TRUE
    CoreTriangleRendering
_Draw_K62O_COLOR_Z_W_PC_TX0_TX1_BADFVF   ENDP

; _Draw_K62O_COLOR_Z
; 
; Undefined, have to talk to Allen about why they exist
;
ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _Draw_K62O_COLOR_Z
_Draw_K62O_COLOR_Z   PROC     NEAR
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
	SEND_HWFOG      = FALSE
	BAD_FVF         = FALSE
    CoreTriangleRendering
_Draw_K62O_COLOR_Z   ENDP


; _Draw_K62O_SPEC_COLOR_Z
; 
; Undefined
;
ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _Draw_K62O_SPEC_COLOR_Z
_Draw_K62O_SPEC_COLOR_Z   PROC     NEAR
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
	SEND_HWFOG      = FALSE
	BAD_FVF         = FALSE
    CoreTriangleRendering
_Draw_K62O_SPEC_COLOR_Z   ENDP

; _Draw_K62O_COLOR_Z_PC
; 
; Undefined
;
ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _Draw_K62O_COLOR_Z_PC
_Draw_K62O_COLOR_Z_PC   PROC     NEAR
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
	SEND_HWFOG      = FALSE
	BAD_FVF         = FALSE
    CoreTriangleRendering
_Draw_K62O_COLOR_Z_PC   ENDP

; _Draw_K62O_COLOR_Z_PC_TX0
; 
; Undefined
;
ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _Draw_K62O_COLOR_Z_PC_TX0
_Draw_K62O_COLOR_Z_PC_TX0   PROC     NEAR
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
	SEND_TX0		= TRUE
	SEND_TX1		= FALSE
	SEND_WRAP		= FALSE
	SEND_NAPALM		= CHECK
	SEND_HWFOG      = FALSE
	BAD_FVF         = FALSE
    CoreTriangleRendering
_Draw_K62O_COLOR_Z_PC_TX0   ENDP

; _Draw_K62O_SPEC_COLOR_Z_W_PC_ALPHA_TX0
; 
; Undefined
;
ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _Draw_K62O_SPEC_COLOR_Z_W_PC_ALPHA_TX0
_Draw_K62O_SPEC_COLOR_Z_W_PC_ALPHA_TX0   PROC     NEAR
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
	SEND_HWFOG      = FALSE
	BAD_FVF         = FALSE
    CoreTriangleRendering
_Draw_K62O_SPEC_COLOR_Z_W_PC_ALPHA_TX0   ENDP

; _Draw_K62O_COLOR_Z_W_PC_TX0_WRAP
; 
; Undefined - Stations ?
;
ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _Draw_K62O_COLOR_Z_W_PC_TX0_WRAP
_Draw_K62O_COLOR_Z_W_PC_TX0_WRAP   PROC     NEAR
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
	SEND_HWFOG      = FALSE
	BAD_FVF         = FALSE
    CoreTriangleRendering
_Draw_K62O_COLOR_Z_W_PC_TX0_WRAP   ENDP

; _Draw_K62O_COLOR_Z_W_PC_TX0_TX1_WRAP
; 
; Undefined - Stations ?
;
ifndef MS_VIEW
align 32
else
align 16
endif
PUBLIC   _Draw_K62O_COLOR_Z_W_PC_TX0_TX1_WRAP
_Draw_K62O_COLOR_Z_W_PC_TX0_TX1_WRAP   PROC     NEAR
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
	SEND_HWFOG      = FALSE
	BAD_FVF         = FALSE
    CoreTriangleRendering
_Draw_K62O_COLOR_Z_W_PC_TX0_TX1_WRAP   ENDP

_TEXT   ENDS

END
