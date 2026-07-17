TITLE    afank.asm
         .686P

EXTRN    __asm_data:BYTE
EXTRN    _fifo_make_room_asm:NEAR

ifdef PERF_MONITORING
extrn __penter:proc
endif

INCLUDE aglobal.inc

.K3D

_DATA SEGMENT PAGE PUBLIC 'DATA'

oneminusone     label   qword
                REAL4   1.0R
                REAL4   -1.0R

fan_header_data label   dword
                dd      FAN_HEADER
fan_header_cont__data label   dword
                dd      FAN_HEADER_CONT
                dd      0

_DATA ENDS



; *****************************
; Fan section
; *****************************

_TEXT   SEGMENT PAGE PUBLIC USE32 'CODE'
        ASSUME  DS:FLAT, SS:FLAT, ES:FLAT

CHUNK_SIZE     = 64

STACK_SIZE     = 8

_locals        = 0

; Local stack section
_area          = _locals
_extra         = _locals + 4

; Param section
_params        = STACK_SIZE + 24
_ret_addr      = _params - 4
_count         = _params + 0
_verts         = _params + 4
_vb_offset     = _params + 8

; ***************************************************************************
; K6-2 version (MMX+3DNOW)
;
align 32
PUBLIC   _DrawTriFanImmAsm_K62O
_DrawTriFanImmAsm_K62O   PROC     NEAR

room1       TEXTEQU     <eax>
va1         TEXTEQU     <eax>
vb1         TEXTEQU     <ebx>
vc1         TEXTEQU     <ecx>
count2      TEXTEQU     <edx>
color1      TEXTEQU     <edx>
area1       TEXTEQU     <edx>
tmp2        TEXTEQU     <edx>
count1      TEXTEQU     <edi>
size1       TEXTEQU     <edi>
rhw1        TEXTEQU     <edi>
room2       TEXTEQU     <edi>
chunk_size1 TEXTEQU     <esi>
ret_addr1   TEXTEQU     <esi>
s0          TEXTEQU     <esi>
hwptr_old   TEXTEQU     <esi>
hwptr1      TEXTEQU     <ebp>

ifdef PERF_MONITORING
   call    __penter
endif

_DrawTriFanImmAsm_K62O_begin:
   push    esp
   push    ebp

   push    ebx
   push    edi

   push    esi
   sub     esp,STACK_SIZE

;  vb1         =ebx
;  vc1         =ecx
;  count1      =eax
;  room1       =edx
;  chunk_size1 =esi

   mov     count1,dword ptr _count[esp]
   mov     chunk_size1,CHUNK_SIZE

   sub     chunk_size1,count1
   mov     vb1,dword ptr _vb_offset[esp]

   femms
   jl      fan_too_large_k6

fan_split_reentry_k6:

; size1     =replaces count1
   mov     count2,count1
   lea     size1,dword ptr[size1+size1*2]

   shl     size1,3h
   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]

   inc     size1
   sub     room1,size1

   mov     vc1,dword ptr [__asm_data + _VERTEX_SIZE]
   js      fifo_check_fan_k6

fifo_fail_reentry_k6:
   mov     va1,dword ptr _verts[esp]
   add     vb1,va1

   mov     s0,dword ptr[__asm_data + _T0_OFFSET]
   add     vc1,vb1

   mov      hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   dec     count2

   movd    mm6, dword ptr [__asm_data + _PIXEL_CENTER]
   js      done_render_k6

   movq    mm2, qword ptr [__asm_data + _TEXEL_OFFSET_S]
   test    hwptr1, 4

   movq    mm4, qword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   pfmul   mm4, qword ptr [__asm_data + _SCALEWS+8 ]  ; Pre multiply S& T scale factors with correction.
endif

   jz      start_fan_k6

   add     hwptr1, 4
   mov     dword ptr [hwptr1-4], 0  ; align to 8 bytes (NOP)

   jmp     start_fan_k6

align 32
start_fan_k6:
   movq    mm1, qword ptr sx[vb1]   ; mm1: pB->sy | pB->sx
   punpckldq mm6, mm6

   movq    mm0, qword ptr sx[va1]     ; mm0: pA->sy | pA->sx
   mov     dword ptr _count[esp],count2

   movq    mm3, qword ptr sx[vc1]     ; mm2: pC->sy | pC->sx
   pfsub   mm0, mm1    ; mm0: pA->sy - pB->sy | pA->sx - pB->sx

   pfsub   mm1, mm3    ; mm1: pB->sy - pC->sy | pB->sx - pC->sx
   pcmpeqd mm7, mm7

   punpckldq mm3, mm1  ; mm2: pB->sx - pC->sx | (ni)
   psllq   mm7, 63

   punpckhdq mm1, mm3  ; mm1: pB->sx - pC->sx |  pB->sy - pC->sy
   movd    mm5, dword ptr [__asm_data + _SCALEZ]

   pfmul   mm0, mm1    ; mm0: (pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)
   movq    mm1, qword ptr sx[va1]  ; mm1 : pA->sy | pA->sx

   pxor    mm0, mm7 ; mm0: -(pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)
   movd    mm7, dword ptr fan_header_data

   pfadd   mm1, mm6   ; mm1: pA->sy+PIXEL_OFFSET | pA->sx+PIXEL_OFFSET
   movd    mm3, dword ptr sz[va1] ; mm3: 0 | pA->sz

   pfacc   mm0, mm0
   pfmul   mm3, mm5 ; mm3: 0 | ZSCALE( pA->sz )

   punpckldq mm7, mm1
   movd    area1, mm0

   punpckhdq mm1, mm1
   movd    mm5, dword ptr rhw[va1] ; mm5: - | pA->rhw

   and     area1,80000000h
   punpckldq mm5, mm5

ifdef DCT_FIX
   pfmul   mm5, qword ptr [__asm_data + _SCALEWS+8 ]  ; multiply w scale factor with correction.
endif


   xor     area1,dword ptr [__asm_data +_CULL_MASK]
   punpckldq mm3, mm5

   punpckldq mm1, qword ptr color[va1]
   jz      culled_tri_k6

   movq_esi_mm7 ;movq    qword ptr [hwptr1], mm7
   movq    mm7, qword ptr [s0 + va1] ; mm3: pA->tv | pA->tu

   movq    qword ptr [hwptr1+8*1], mm1
   movq    mm0, qword ptr sx[vb1]

   pfmul   mm7, mm4  ; mm3: pA->tv*l_lscaleT | pA->tu*l_lscaleS
   movd    mm1, dword ptr color[vb1]

   pfadd   mm7, mm2
   movq    qword ptr [hwptr1+8*2], mm3

   movd    mm3, dword ptr sz[vb1]
   pfmul   mm7, mm5

   movd    mm5, dword ptr [__asm_data + _SCALEZ]
   pfadd   mm0, mm6  ; mm0: pB->sy + PIXEL_OFFSET | pB->sx + PIXEL_OFFSET

   pfmul   mm3, mm5 ; mm3: 0 | ZSCALE( pB->sz )
   movq    qword ptr [hwptr1+8*3], mm7

   ; sending down vertB
   movd    mm5, dword ptr rhw[vb1]
   punpckldq mm1, mm3 ; mm1: ZSCALE( pB->sz ) | bColor

   movq    qword ptr [hwptr1+8*4], mm0
   punpckldq mm5, mm5 ; mm5: pB->rhw | pB->rhw

ifdef DCT_FIX
   pfmul   mm5, qword ptr [__asm_data + _SCALEWS+8 ]  ; multiply w scale factor with correction.
endif

   movq    mm7, qword ptr [s0 + vb1] ; mm7: pB->tv | pB->tu
   movq    qword ptr [hwptr1+8*5], mm1

   movd    mm3, dword ptr [__asm_data + _SCALEZ]
   pfmul   mm7, mm4 ; mm7: pB->tv*l_lscaleT | pB->tu*l_lscaleS

   movq    mm0, qword ptr sx[vc1] ; mm0: pC->sy | pC->sx
   pfadd   mm7, mm2 ; mm7: pB->tv*l_lscaleT + TEXEL_SOFFSET | pB->tu*l_lscaleS + TEXEL_SOFFSET

   movd    mm1, dword ptr sz[vc1]
   pfadd   mm0, mm6 ; mm0: pC->sy + PIXEL_OFFSET | pC->sx + PIXEL_OFFSET

   pfmul   mm1, mm3 ; mm1: (ni) | ZSCALE( pC->sz )
   movd    mm3, dword ptr rhw[vc1]

   pfmul   mm7, mm5 ; mm7: (pB->tv*l_lscaleT + TEXEL_SOFFSET)*pB->rhw | (pB->tu*l_lscaleS + TEXEL_SOFFSET)*pB->rhw
   punpckldq mm3, mm3 ; mm3: pC->rhw | pC->rhw

ifdef DCT_FIX
   pfmul   mm3, qword ptr [__asm_data + _SCALEWS+8 ]  ; multiply w scale factor with correction.
endif

   punpckldq mm5, mm7 ; mm5: (pB->tu*l_lscaleS + TEXEL_SOFFSET)*pB->rhw | pB->rhw
   punpckldq mm1, mm3 ; mm1: pC->rhw | ZSCALE( pC->sz )

   movq    qword ptr [hwptr1+8*6], mm5
   punpckhdq mm7, mm7

   movq    mm5, qword ptr [s0 + vc1]
   punpckldq mm7, mm0 ; mm7: pC->sx + PIXEL_OFFSET | (pB->tv*l_lscaleT + TEXEL_SOFFSET)*pB->rhw

   pfmul   mm5, mm4 ; mm5: pC->tv * lscaleT | pC->tu * lscaleS
   movq    qword ptr [hwptr1+8*7], mm7

   punpckhdq mm0, mm0 ; mm0: (ni) | pC->sy + PIXEL_OFFSET
   pfadd   mm5, mm2 ; mm5: (pC->tv*lscaleT)+TEXEL_TOFFSET | (pC->tu*lscaleS)+TEXEL_SOFFSET

   ; sending down vertC
   punpckldq mm0, qword ptr color[vc1]
   pfmul   mm5, mm3

   movq    qword ptr [hwptr1+8*8], mm0
   mov     vb1,vc1

   mov     vc1,dword ptr[__asm_data + _VERTEX_SIZE]
   movq    qword ptr [hwptr1+8*9], mm1

   mov     count2,dword ptr _count[esp]
   movq    qword ptr [hwptr1+8*10], mm5

   add      vc1,vb1
   add      hwptr1, 8*10+8

   dec      count2
   js       done_render_k6

   jmp      fan_continue_k6

align 32
fan_continue_k6:
   movq    mm0, qword ptr sx[va1]     ; mm0: pA->sy | pA->sx
   mov      dword ptr _count[esp],count2

   movq    mm1, qword ptr sx[vb1]   ; mm1: pB->sy | pB->sx
   pcmpeqd mm5, mm5

   movq    mm3, qword ptr sx[vc1]     ; mm2: pC->sy | pC->sx
   pfsub   mm0, mm1    ; mm0: pA->sy - pB->sy | pA->sx - pB->sx

   movd    mm7, dword ptr fan_header_cont__data
   pfsub   mm1, mm3    ; mm1: pB->sy - pC->sy | pB->sx - pC->sx

   psllq   mm5, 63
   punpckldq mm3, mm1  ; mm2: pB->sx - pC->sx | (ni)

   punpckhdq mm1, mm3  ; mm1: pB->sx - pC->sx |  pB->sy - pC->sy
   movd    mm3, dword ptr sz[vc1] ; mm3: 0 | pC->sz

   pfmul   mm0, mm1    ; mm0: (pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)
   movq    mm1, qword ptr sx[vc1]  ; mm1 : pC->sy | pC->sx

   pxor    mm0, mm5 ; mm0: -(pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)
   movd    mm5, dword ptr [__asm_data + _SCALEZ]

   pfacc   mm0, mm0
   pfmul   mm3, mm5 ; mm3: 0 | ZSCALE( pC->sz )

   movd    area1, mm0
   pfadd   mm1, mm6   ; mm1: pC->sy+PIXEL_OFFSET | pC->sx+PIXEL_OFFSET

   and     area1,80000000h
   punpckldq mm7, mm1

   xor     area1,dword ptr [__asm_data +_CULL_MASK]
   jz      culled_tri_k6

   movd    mm5, dword ptr rhw[vc1] ; mm5: - | pC->rhw
   punpckhdq mm1, mm1

   movq_esi_mm7 ;movq    qword ptr [hwptr1], mm7
   punpckldq mm1, qword ptr color[vc1]

   punpckldq mm5, mm5
   movq    qword ptr [hwptr1+8*1], mm1

   punpckldq mm3, mm5
   movq    mm7, qword ptr [s0 + vc1] ; mm3: pC->tv | pC->tu

   movq    qword ptr [hwptr1+8*2], mm3
   pfmul   mm7, mm4  ; mm3: pC->tv*l_lscaleT | pC->tu*l_lscaleS

   mov     vb1,vc1
   mov     count2,dword ptr _count[esp]

   pfadd   mm7, mm2
   mov     vc1,dword ptr[__asm_data + _VERTEX_SIZE]

   add     hwptr1, 8*3+8
   pfmul   mm7, mm5

   add     vc1,vb1
   dec     count2

   movq    qword ptr [hwptr1-8], mm7  ; [hwptr1+8*3]
   jns     fan_continue_k6

done_render_k6:
   mov      hwptr_old,dword ptr[__asm_data + _FIFO_PTR]
   mov      dword ptr[__asm_data + _FIFO_PTR],hwptr1

   sub      hwptr1,hwptr_old
   mov      room2,dword ptr [__asm_data + _FIFO_ROOM]

   shr      hwptr1,2
   mov      eax,vc1

   sub      room2,hwptr1
   femms

   mov      dword ptr [__asm_data + _FIFO_ROOM],room2
   add      esp,STACK_SIZE

   pop      esi
   pop      edi

   pop      ebx
   pop      ebp

   pop      esp
   ret

align 32
culled_tri_k6:

   mov      count2,dword ptr _count[esp]
   mov      vb1,vc1
   mov      vc1,dword ptr[__asm_data + _VERTEX_SIZE]
   add      vc1,vb1
   dec      count2
   jns      start_fan_k6

   jmp      done_render_k6

align 32
fan_too_large_k6:
   mov      ret_addr1,dword ptr _ret_addr[esp]
   mov      dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub      count1,CHUNK_SIZE
   mov      dword ptr[__asm_data + _SAVED_COUNT],count1
   mov      dword ptr _ret_addr[esp],fan_split_continue_k6
   mov      count1,CHUNK_SIZE
   jmp      fan_split_reentry_k6

align 32
fan_split_continue_k6:
   mov      eax,dword ptr[__asm_data + _VERTEX_SIZE]
   sub      ecx,eax
   mov      eax,dword ptr _verts[esp - _params]

   add      esp,12
   sub      ecx,eax

   mov      edx,dword ptr[__asm_data + _SAVED_COUNT]
   push     ecx
   push     eax

   mov      eax,dword ptr[__asm_data + _SAVED_RET_ADDR]
   push     edx
   push     eax

   jmp      _DrawTriFanImmAsm_K62O_begin

align 32
fifo_check_fan_k6:
   push     vc1
   push     count2
   push     size1
   call     _fifo_make_room_asm
   pop      size1
   pop      count2
   pop      vc1
   jmp      fifo_fail_reentry_k6

_DrawTriFanImmAsm_K62O  ENDP

_TEXT   ENDS

END
