
TITLE           amesh2K.asm

.686P
.K3D

EXTRN    __asm_data:BYTE
EXTRN    _fifo_make_room_asm:NEAR

ifdef PERF_MONITORING
extrn __penter:proc
endif

; *****************************
; Mesh section
; *****************************

_DATA SEGMENT PAGE PUBLIC 'DATA'

oneminusone     label   qword
                REAL4   1.0R
                REAL4   -1.0R

fan_header_data label   dword
                dd      FAN_HEADER
fan_header_cont__data label   dword
                dd      FAN_HEADER_CONT

fan_gourad_header_data  dd      FAN_GOURAD_HEADER
                dd      0

_DATA ENDS

_TEXT   SEGMENT PAGE PUBLIC USE32 'CODE'
        ASSUME  DS:FLAT, SS:FLAT, ES:FLAT

INCLUDE afdbg.inc
INCLUDE aglobal.inc

CHUNK_SIZE     = 64
STACK_SIZE     = 20

; Local stack section
_locals        = 0

_area          = _locals
_hwptr         = _locals + 4
_idx           = _locals + 8
_last          = _locals + 12
_fog           = _locals + 16

; Param section
_params        = STACK_SIZE + 24
_ret_addr      = _params - 4
_count         = _params + 0
_idx           = _params + 4
_verts         = _params + 8


chunk_size1 TEXTEQU  <eax>
va1         TEXTEQU  <eax>
cull2       TEXTEQU  <eax>
tmp1        TEXTEQU  <eax>
ret_addr1   TEXTEQU  <eax>

vb1         TEXTEQU  <ebx>
size1       TEXTEQU  <ebx>
area2       TEXTEQU  <ebx>

vc1         TEXTEQU  <ecx>
hwptr_old   TEXTEQU  <ecx>
count2      TEXTEQU  <ecx>

count1      TEXTEQU  <edi>
rhw1        TEXTEQU  <edi>
last_flags  TEXTEQU  <edi>
verts2      TEXTEQU  <edi>
cull1       TEXTEQU  <edi>
flags2      TEXTEQU  <edi>

hwptr1      TEXTEQU  <ebp>
verts1      TEXTEQU  <ebp>

idx1        TEXTEQU  <esi>

room1       TEXTEQU  <edx>
area1       TEXTEQU  <edx>
color1      TEXTEQU  <edx>
flags1      TEXTEQU  <edx>
last2       TEXTEQU  <edx>


align 32
PUBLIC   _DrawTriEdge2Asm_K62O
_DrawTriEdge2Asm_K62O   PROC     NEAR

ifdef PERF_MONITORING
   call    __penter
endif

_DrawTriEdge2Asm_K62O_begin:
    ; 2, 1
    femms
    push     esp
    ; 5, 1
    mov      chunk_size1,CHUNK_SIZE
    push     ebp
    ; 1, 1
    push     ebx
    push     edi
    ; 1, 3
    push     esi
    sub      esp,STACK_SIZE
    ; 4, 2
    mov      count1,dword ptr _count[esp]
    sub      chunk_size1,count1
    ; 6, 2 (+29)
    mov      room1,dword ptr [__asm_data + _FIFO_ROOM]
    mov      size1,count1
    ; 1, 6 [2 - 4]
    nop
    jl       TriMeshTooLarge_k6m

TriMeshReentry_k6m:
    ; 3, 4
    lea      size1,dword ptr[size1+size1*2]
    mov      idx1,dword ptr _idx[esp]
    ; 3, 2
    shl      size1,3h
    mov      va1,dword ptr [idx1]
    ; 1, 2
    inc      size1
    sub      room1,size1
    ; 3, 6 (+24)
    mov      vc1,dword ptr [idx1+4]
    js       fifo_check_mesh_k6m

fifo_check_mesh_reentry_k6m:
    ; 6, 1
    mov      hwptr1,dword ptr[__asm_data + _FIFO_PTR]
    dec      count1
    ; 2 [1 - 1], 5
    mov      vb1,va1
    and      va1,0ffffh
    ; 4, 6
    mov      dword ptr _count[esp],count1
    and      vb1,0ffff0000h
    ; 6, 2 (+24)
    and      vc1,0ffffh
    jmp      ind_tri_begin_k6m

align 32
ind_tri_begin_k6m:
    movd    mm6, dword ptr [__asm_data + _PIXEL_CENTER]
    test    hwptr1, 4

    movq    mm2, qword ptr [__asm_data + _TEXEL_OFFSET_S]
    punpckldq mm6, mm6

    movq    mm4, qword ptr [__asm_data + _SCALE_S]

ifdef DCT_FIX
    pfmul   mm4, qword ptr [__asm_data + _SCALEWS+8 ]  ; Pre multiply S& T scale factors with correction.
endif

    jz      @F

    mov     dword ptr [hwptr1], 0
    add     hwptr1, 4
@@:
    shl     va1,5h
    mov     verts2,dword ptr _verts[esp]

    shr     vb1,11
    lea     va1,dword ptr[verts2+va1]

    and     vc1,0ffffh
    lea     vb1,dword ptr[verts2+vb1]

    shl     vc1,5h
    movq    mm0, qword ptr sx[va1]     ; mm0: pA->sy | pA->sx

    mov     area1,80000000h
    lea     vc1,dword ptr[verts2+vc1]

    pcmpeqd mm5, mm5
    movq    mm1, qword ptr sx[vb1]   ; mm1: pB->sy | pB->sx

    psllq   mm5, 63
    movq    mm3, qword ptr sx[vc1]     ; mm2: pC->sy | pC->sx

    pfsub   mm0, mm1    ; mm0: pA->sy - pB->sy | pA->sx - pB->sx
    mov     cull1,dword ptr [__asm_data +_CULL_MASK]

    pfsub   mm1, mm3    ; mm1: pB->sy - pC->sy | pB->sx - pC->sx
    movq    mm7, qword ptr tu[va1] ; mm3: pA->tv | pA->tu

    punpckldq mm3, mm1  ; mm2: pB->sx - pC->sx | (ni)
    pfmul   mm7, mm4  ; mm3: pA->tv*l_lscaleT | pA->tu*l_lscaleS

    punpckhdq mm1, mm3  ; mm1: pB->sx - pC->sx |  pB->sy - pC->sy
    movd    mm3, dword ptr sz[va1] ; mm3: 0 | pA->sz

    pfadd   mm7, mm2
    pfmul   mm0, mm1    ; mm0: (pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)

    movq    mm1, qword ptr sx[va1]  ; mm1 : pA->sy | pA->sx
    pxor    mm0, mm5  ; mm0: -(pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)

    movd    mm5, dword ptr [__asm_data + _SCALEZ]
    pfacc   mm0, mm0

    pfmul   mm3, mm5 ; mm3: 0 | ZSCALE( pA->sz )
    pfadd   mm1, mm6   ; mm1: pA->sy+PIXEL_OFFSET | pA->sx+PIXEL_OFFSET

    movd    dword ptr _area[esp], mm0
    movd    mm0, dword ptr fan_header_data

    and      area1,dword ptr _area[esp]
    punpckldq mm0, mm1

    xor      area1,cull1
    punpckhdq mm1, mm1

    movd    mm5, dword ptr rhw[va1] ; mm5: - | pA->rhw
    jz       culled_tri_k6m

    punpckldq mm1, qword ptr color[va1]
    movq    qword ptr [hwptr1], mm0

    punpckldq mm5, mm5

ifdef DCT_FIX
    pfmul   mm5, qword ptr [__asm_data + _SCALEWS+8 ]  ; multiply w scale factor with correction.
endif

    movq    mm0, qword ptr sx[vb1]

    movq    qword ptr [hwptr1+8*1], mm1
    pfadd   mm0, mm6  ; mm0: pB->sy + PIXEL_OFFSET | pB->sx + PIXEL_OFFSET

    punpckldq mm3, mm5
    movd    mm1, dword ptr color[vb1]

    movq    qword ptr [hwptr1+8*2], mm3
    pfmul   mm7, mm5

    movd    mm3, dword ptr sz[vb1]
    movq    qword ptr [hwptr1+8*3], mm7

    movd    mm7, dword ptr [__asm_data + _SCALEZ]
    movq    qword ptr [hwptr1+8*4], mm0

    ; sending down vertB
    pfmul   mm3, mm7 ; mm3: 0 | ZSCALE( pB->sz )
    movd    mm5, dword ptr rhw[vb1]

    punpckldq mm1, mm3 ; mm1: ZSCALE( pB->sz ) | bColor
    movq    mm7, qword ptr tu[vb1] ; mm7: pB->tv | pB->tu

    punpckldq mm5, mm5 ; mm5: pB->rhw | pB->rhw

ifdef DCT_FIX
    pfmul   mm5, qword ptr [__asm_data + _SCALEWS+8 ]  ; multiply w scale factor with correction.
endif

    pfmul   mm7, mm4 ; mm7: pB->tv*l_lscaleT | pB->tu*l_lscaleS

    movq    qword ptr [hwptr1+8*5], mm1
    pfadd   mm7, mm2 ; mm7: pB->tv*l_lscaleT + TEXEL_SOFFSET | pB->tu*l_lscaleS + TEXEL_SOFFSET

    movq    mm0, qword ptr sx[vc1] ; mm0: pC->sy | pC->sx
    pfmul   mm7, mm5 ; mm7: (pB->tv*l_lscaleT + TEXEL_SOFFSET)*pB->rhw | (pB->tu*l_lscaleS + TEXEL_SOFFSET)*pB->rhw

    pfadd   mm0, mm6 ; mm0: pC->sy + PIXEL_OFFSET | pC->sx + PIXEL_OFFSET
    punpckldq mm5, mm7 ; mm5: (pB->tu*l_lscaleS + TEXEL_SOFFSET)*pB->rhw | pB->rhw

    movd    mm1, dword ptr sz[vc1]
    movq    qword ptr [hwptr1+8*6], mm5

    movd    mm5, dword ptr [__asm_data + _SCALEZ]
    punpckhdq mm7, mm7

    pfmul   mm1, mm5 ; mm1: (ni) | ZSCALE( pC->sz )
    movd    mm3, dword ptr rhw[vc1]

    punpckldq mm7, mm0 ; mm7: pC->sx + PIXEL_OFFSET | (pB->tv*l_lscaleT + TEXEL_SOFFSET)*pB->rhw
    punpckldq mm3, mm3 ; mm3: pC->rhw | pC->rhw

ifdef DCT_FIX
    pfmul   mm3, qword ptr [__asm_data + _SCALEWS+8 ]  ; multiply w scale factor with correction.
endif


    movq    qword ptr [hwptr1+8*7], mm7
    punpckhdq mm0, mm0 ; mm0: (ni) | pC->sy + PIXEL_OFFSET
    ; sending down vertC
    movq    mm5, qword ptr tu[vc1]
    punpckldq mm1, mm3 ; mm1: pC->rhw | ZSCALE( pC->sz )

    pfmul   mm5, mm4 ; mm5: pC->tv * lscaleT | pC->tu * lscaleS
    punpckldq mm0, qword ptr color[vc1]

    pfadd   mm5, mm2 ; mm5: (pC->tv*lscaleT)+TEXEL_TOFFSET | (pC->tu*lscaleS)+TEXEL_SOFFSET
    movq    qword ptr [hwptr1+8*8], mm0

    pfmul   mm5, mm3
    mov      count2,dword ptr _count[esp]

    add      hwptr1, 8*10+8
    mov      tmp1,idx1

    movq    qword ptr [hwptr1-8*2], mm1  ; [hwptr1+8*9]
    add      idx1,8

    dec      count2
    movq    qword ptr [hwptr1-8*1], mm5 ; [hwptr1+8*10]

    jns      @F
    mov      idx1,tmp1
@@:
    mov      va1,dword ptr [idx1]
    js       done_render1_k6m

    mov      dword ptr _count[esp],count2
    mov      flags1,dword ptr [idx1+4]

    mov      vb1,va1
    mov      vc1,flags1

    and      flags1,1f0000h
    and      va1,0ffffh

    and      vb1,0ffff0000h
    and      vc1,0ffffh

    sub      flags1,01E0000h
    js       ind_tri_begin_k6m

    mov      dword ptr _last[esp],flags1
    jmp      prim_continue_k6m

align 32
prim_continue_k6m:
    shl     va1,5h
    mov     verts2,dword ptr _verts[esp]

    shr     vb1,11
    lea     va1,dword ptr[verts2+va1]

    and     vc1,0ffffh
    lea     vb1,dword ptr[verts2+vb1]

    shl     vc1,5h
    movq    mm0, qword ptr sx[va1]     ; mm0: pA->sy | pA->sx

    shl     flags1,6
    lea     vc1,dword ptr[verts2+vc1]

    pcmpeqd mm5, mm5
    movq    mm1, qword ptr sx[vb1]   ; mm1: pB->sy | pB->sx

    psllq   mm5, 63
    movq    mm3, qword ptr sx[vc1]     ; mm2: pC->sy | pC->sx

    mov     area2,80000000h
    pfsub   mm0, mm1    ; mm0: pA->sy - pB->sy | pA->sx - pB->sx

    mov     cull2,dword ptr [__asm_data +_CULL_MASK]
    pfsub   mm1, mm3    ; mm1: pB->sy - pC->sy | pB->sx - pC->sx

    or      flags1,STRIP_CONTINUE
    punpckldq mm3, mm1  ; mm2: pB->sx - pC->sx | (ni)

    movd    mm7, dword ptr [__asm_data + _SCALEZ]
    punpckhdq mm1, mm3  ; mm1: pB->sx - pC->sx |  pB->sy - pC->sy

    pfmul   mm0, mm1    ; mm0: (pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)
    movd    mm3, dword ptr sz[vc1] ; mm3: 0 | pC->sz

    pxor    mm0, mm5 ; mm0: -(pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)
    pfmul   mm3, mm7 ; mm3: 0 | ZSCALE( pC->sz )

    pfacc   mm0, mm0
    movq    mm1, qword ptr sx[vc1]  ; mm1 : pC->sy | pC->sx

    movd    dword ptr _area[esp], mm0
    pfadd   mm1, mm6   ; mm1: pC->sy+PIXEL_OFFSET | pC->sx+PIXEL_OFFSET

    movd    mm0, flags1
    and     area2,dword ptr _area[esp]

    punpckldq mm0, mm1
    xor     area2,cull2

    movd    mm5, dword ptr rhw[vc1] ; mm5: - | pC->rhw
    jz      culled_tri_k6m

    punpckhdq mm1, mm1
    movq    qword ptr [hwptr1], mm0

    punpckldq mm5, mm5
    punpckldq mm1, qword ptr color[vc1]

    punpckldq mm3, mm5
    movq    qword ptr [hwptr1+8*1], mm1

    movq    mm7, qword ptr tu[vc1] ; mm3: pC->tv | pC->tu
    add     idx1,8

    pfmul   mm7, mm4  ; mm3: pC->tv*l_lscaleT | pC->tu*l_lscaleS
    movq    qword ptr [hwptr1+8*2], mm3

    pfadd   mm7, mm2
    mov     count2,dword ptr _count[esp]

    add     hwptr1,8*3+8
    pfmul   mm7, mm5

    dec     count2
    movq    qword ptr [hwptr1-8], mm7 ; [hwptr1+8*3]

    js      done_render_k6m
    mov     va1,dword ptr[idx1]

    mov     dword ptr _count[esp],count2
    mov     flags1,dword ptr[idx1+4]

    mov     vb1,va1
    mov     vc1,flags1

    and     flags1,1f0000h
    and     va1,0ffffh

    and     vb1,0ffff0000h
    and     vc1,0ffffh

    sub     flags1,01E0000h
    js      ind_tri_begin_k6m

    mov     flags2,flags1
    xor     flags1,dword ptr _last[esp]

    mov     dword ptr _last[esp],flags2
    xor     flags1,BIT_16

    jmp      prim_continue_k6m

align 32
culled_tri_k6m:
    ; 4, 3
    mov      count1,dword ptr _count[esp]
    add      idx1,8
    ; 1, 4
    dec      count1
    mov      dword ptr _count[esp],count1
    ; 2, 2
    js       done_render_k6m
    mov      va1,dword ptr [idx1]
    ; 3, 2
    mov      vc1,dword ptr [idx1+4]
    mov      vb1,va1
    ; 4, 5 (+30)
    mov      verts2,dword ptr _verts[esp]
    and      va1,0ffffh
    ; 6 [2 - 4], 6
    and      vb1,0ffff0000h
    and      vc1,0ffffh
    ; 5 (+15)
    jmp      ind_tri_begin_k6m

align 32
done_render1_k6m:
    ; 3, 2 (+5)
    add      idx1,8
    jmp      done_render_k6m

align 32
done_render_k6m:
    ; 6, 6 (+28)
    mov      hwptr_old,dword ptr[__asm_data + _FIFO_PTR]
    mov      dword ptr[__asm_data + _FIFO_PTR],hwptr1
    ; 2, 6 [2 - 4]
    sub      hwptr1,hwptr_old
    mov      room1,dword ptr [__asm_data + _FIFO_ROOM]
    ; 3, 4
    shr      hwptr1,2
    mov      dword ptr _idx[esp],idx1
    ; 2, 6
    sub      room1,hwptr1
    mov      dword ptr [__asm_data + _FIFO_ROOM],room1
    ; 2, 3
    femms
    add      esp,STACK_SIZE
    ; 1, 1
    pop      esi
    pop      edi
    ; 1, 1
    pop      ebx
    pop      ebp
    ; 1, 1 (+30)
    pop      esp
    ret

align 32
fifo_check_mesh_k6m:
;regs that still need to be valid on jmp
;va1,vc1,idx1,count1
    ; 1, 1
    push     va1
    push     vc1
    ; 1, 5
    push     size1
    call     _fifo_make_room_asm
    ; 1, 1
    pop      size1
    pop      vc1
    ; 1, 5 (+16)
    pop      va1
    jmp      fifo_check_mesh_reentry_k6m

align 32
TriMeshTooLarge_k6m:
;saving count1,size1,room1
    ; [-] 4, 5
    mov      ret_addr1,dword ptr _ret_addr[esp]
    mov      dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
    ; 3, 6
    sub      count1,CHUNK_SIZE
    mov      dword ptr[__asm_data + _SAVED_COUNT],count1
    ; 8, 5 (+31)
    mov      dword ptr _ret_addr[esp],TriMeshContinue_k6m
    mov      count1,CHUNK_SIZE
    ; 5 [1 - 4]
    jmp      TriMeshReentry_k6m

align 32
TriMeshContinue_k6m:
    ; [-] 6, 5
    mov      edx,dword ptr[__asm_data + _SAVED_COUNT]
    mov      eax,dword ptr[__asm_data + _SAVED_RET_ADDR]
    ;  3, 1
    mov      dword ptr _count[esp - _params],edx
    push     eax
    ; 5 (+20)
    jmp      _DrawTriEdge2Asm_K62O_begin

_DrawTriEdge2Asm_K62O   ENDP

; ***************************************************************************
align 32
PUBLIC   _DrawTriEdge2GAsm_K62O
_DrawTriEdge2GAsm_K62O   PROC     NEAR

ifdef PERF_MONITORING
   call    __penter
endif

_DrawTriEdge2GAsm_K62O_begin:
    femms
    push     esp

    mov      chunk_size1,CHUNK_SIZE
    push     ebp

    push     ebx
    push     edi

    push     esi
    sub      esp,STACK_SIZE

    mov      count1,dword ptr _count[esp]
    sub      chunk_size1,count1

    mov      room1,dword ptr [__asm_data + _FIFO_ROOM]
    mov      size1,count1

    nop
    jl       TriMeshTooLarge_k6g

TriMeshReentry_k6g:

    lea      size1,dword ptr[size1+size1*2]
    mov      idx1,dword ptr _idx[esp]

    shl      size1,3h
    mov      va1,dword ptr [idx1]

    inc      size1
    sub      room1,size1

    mov      vc1,dword ptr [idx1+4]
    js       fifo_check_mesh_k6g

fifo_check_mesh_reentry_k6g:
    mov      hwptr1,dword ptr[__asm_data + _FIFO_PTR]
    dec      count1

    mov      vb1,va1
    and      va1,0ffffh

    mov      dword ptr _count[esp],count1
    and      vb1,0ffff0000h

    and      vc1,0ffffh
    jmp      ind_tri_begin_k6g

align 32
ind_tri_begin_k6g:
    movd    mm6, dword ptr [__asm_data + _PIXEL_CENTER]
    test    hwptr1, 4

    punpckldq mm6, mm6
    jz      @F

    mov     dword ptr [hwptr1], 0
    add     hwptr1, 4
@@:
    shl     va1,5h
    mov     verts2,dword ptr _verts[esp]

    shr     vb1,11
    lea     va1,dword ptr[verts2+va1]

    and     vc1,0ffffh
    lea     vb1,dword ptr[verts2+vb1]

    shl     vc1,5h
    movq    mm0, qword ptr sx[va1]     ; mm0: pA->sy | pA->sx

    mov     area1,80000000h
    lea     vc1,dword ptr[verts2+vc1]

    pcmpeqd mm7, mm7
    movq    mm1, qword ptr sx[vb1]   ; mm1: pB->sy | pB->sx

    psllq   mm7, 63
    movq    mm3, qword ptr sx[vc1]     ; mm2: pC->sy | pC->sx

    pfsub   mm0, mm1    ; mm0: pA->sy - pB->sy | pA->sx - pB->sx
    movq    mm5, qword ptr sx[vb1]

    pfsub   mm1, mm3    ; mm1: pB->sy - pC->sy | pB->sx - pC->sx
    mov     cull1,dword ptr [__asm_data +_CULL_MASK]

    pfadd   mm5, mm6  ; mm5: pB->sy + PIXEL_OFFSET | pB->sx + PIXEL_OFFSET
    punpckldq mm3, mm1  ; mm2: pB->sx - pC->sx | (ni)

    punpckhdq mm1, mm3  ; mm1: pB->sx - pC->sx |  pB->sy - pC->sy
    movd    mm3, dword ptr sz[va1] ; mm3: 0 | pA->sz

    pfmul   mm0, mm1    ; mm0: (pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)
    movd    mm1, dword ptr [__asm_data + _SCALEZ]

    pxor    mm0, mm7 ; mm0: -(pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)
    pfmul   mm3, mm1 ; mm3: 0 | ZSCALE( pA->sz )

    pfacc   mm0, mm0
    punpckldq mm3, qword ptr rhw[va1]

    movd    area1, mm0
    movq    mm1, qword ptr sx[va1]  ; mm1 : pA->sy | pA->sx

    and     area1,dword ptr _area[esp]
    pfadd    mm1, mm6   ; mm1: pA->sy+PIXEL_OFFSET | pA->sx+PIXEL_OFFSET

    movd     mm0, dword ptr fan_gourad_header_data
    xor      area1,cull1

    movd     mm7, dword ptr sz[vb1]
    jz       culled_tri_k6g

    punpckldq mm0, mm1
    pfmul    mm7, qword ptr [__asm_data + _SCALEZ] ; mm3: 0 | ZSCALE( pB->sz )

    punpckhdq mm1, mm1
    movq     qword ptr [hwptr1], mm0  ; <-- fan_gourad_header, pA->sx+PIXEL_OFFSET

    punpckldq mm1, qword ptr color[va1]
    mov      tmp1,idx1

    movd     mm0, dword ptr color[vb1]
    movq     qword ptr [hwptr1+8*1], mm1 ; <-- pA->sy+PIXEL_OFFSET, aColor

    punpckldq mm0, mm7
    movd     mm1, dword ptr rhw[vc1]

    movq     qword ptr [hwptr1+8*2], mm3 ; <-- ZSCALE( pA->sz ), pA-rhw
    add      idx1,8

    movq     mm3, qword ptr sx[vc1] ; mm3: pC->sy | pC->sx
    movq     qword ptr [hwptr1+8*3], mm5 ; <-- pB->sx + PIXEL_OFFSET, pB->sy + PIXEL_OFFSET

    pfadd    mm3, mm6 ; mm3: pC->sy + PIXEL_OFFSET | pC->sx + PIXEL_OFFSET
    movd     mm5, dword ptr sz[vc1] ; mm5: 0 | pC->sz

    movq     qword ptr [hwptr1+8*4], mm0 ; <-- bColor, ZSCALE( pB->sz )
    pfmul    mm5, qword ptr [__asm_data + _SCALEZ] ; mm5: 0 | ZSCALE( pC->sz )

    punpckldq mm1, mm3
    punpckldq mm5, qword ptr rhw[vc1]

    punpckhdq mm3, mm3
    movq     qword ptr [hwptr1+8*5], mm1 ; <-- pB->rhw, pC->sx + PIXEL_OFFSET

    punpckldq mm3, qword ptr color[vc1]
    add      hwptr1,8*7+8

    mov      count2,dword ptr _count[esp]
    movq     qword ptr [hwptr1-8*2], mm3 ; <-- pC->sy + PIXEL_OFFSET, cColor

    dec      count2
    movq     qword ptr [hwptr1-8*1], mm5 ; <-- ZSCALE( pC->sz ), pC->rhw

    jns      @F
    mov      idx1,tmp1
@@:
    mov      va1,dword ptr [idx1]
    js       done_render1_k6g

    mov      dword ptr _count[esp],count2
    mov      flags1,dword ptr [idx1+4]

    mov      vb1,va1
    mov      vc1,flags1

    and      flags1,1f0000h
    and      va1,0ffffh

    and      vb1,0ffff0000h
    and      vc1,0ffffh

    sub      flags1,01E0000h
    js       ind_tri_begin_k6g

    mov      dword ptr _last[esp],flags1
    jmp      prim_continue_k6g

align 32
prim_continue_k6g:
; ***
    shl     va1,5h
    mov     verts2,dword ptr _verts[esp]

    shr     vb1,11
    lea     va1,dword ptr[verts2+va1]

    and     vc1,0ffffh
    lea     vb1,dword ptr[verts2+vb1]

    shl     vc1,5h
    movq    mm0, qword ptr sx[va1]     ; mm0: pA->sy | pA->sx

    shl     flags1,6
    lea     vc1,dword ptr[verts2+vc1]

    or      flags1,STRIP_GOURAD_CONT
    movq    mm1, qword ptr sx[vb1]   ; mm1: pB->sy | pB->sx

    pcmpeqd mm5, mm5
    mov     area2,80000000h
    
    movq    mm3, qword ptr sx[vc1]     ; mm2: pC->sy | pC->sx
    pfsub   mm0, mm1    ; mm0: pA->sy - pB->sy | pA->sx - pB->sx

    psllq   mm5, 63
    pfsub   mm1, mm3    ; mm1: pB->sy - pC->sy | pB->sx - pC->sx

    mov     cull2,dword ptr [__asm_data +_CULL_MASK]
    punpckldq mm3, mm1  ; mm2: pB->sx - pC->sx | (ni)

    movd    mm7, dword ptr [__asm_data + _SCALEZ]
    punpckhdq mm1, mm3  ; mm1: pB->sx - pC->sx |  pB->sy - pC->sy

    movd    mm3, dword ptr sz[vc1] ; mm3: 0 | pC->sz
    pfmul   mm0, mm1    ; mm0: (pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)

    movq    mm1, qword ptr sx[vc1]  ; mm1 : pC->sy | pC->sx
    pxor    mm0, mm5 ; mm0: -(pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)

    pfadd    mm1, mm6   ; mm1: pC->sy+PIXEL_OFFSET | pC->sx+PIXEL_OFFSET
    pfacc   mm0, mm0

    pfmul   mm3, mm7 ; mm3: 0 | ZSCALE( pC->sz )
    movd    dword ptr _area[esp], mm0

    movd    mm0, flags1
    and     area2,dword ptr _area[esp]

    punpckldq mm0, mm1
    xor     area2,cull2

    punpckhdq mm1, mm1
    jz      culled_tri_k6g

    movq    qword ptr [hwptr1], mm0  ; <-- fan_gourad_header, pC->sx+PIXEL_OFFSET
    punpckldq mm1, qword ptr color[vc1]

    add     idx1,8
    punpckldq mm3, qword ptr rhw[vc1]

    movq    qword ptr [hwptr1+8*1], mm1 ; <-- pC->sy+PIXEL_OFFSET, aColor
    mov      count2,dword ptr _count[esp]

    add      hwptr1,8*2+8
    dec      count2

    movq     qword ptr [hwptr1-8], mm3 ; <-- ZSCALE( pC->sz ), pC-rhw
    js       done_render_k6g

    mov      va1,dword ptr[idx1]
    mov      dword ptr _count[esp],count2

    mov      flags1,dword ptr[idx1+4]
    mov      vb1,va1

    mov      vc1,flags1
    and      flags1,1f0000h

    and      va1,0ffffh
    and      vb1,0ffff0000h

    and      vc1,0ffffh
    sub      flags1,01E0000h

    js       ind_tri_begin_k6g
    mov      flags2,flags1

    xor      flags1,dword ptr _last[esp]
    mov      dword ptr _last[esp],flags2

    xor      flags1,BIT_16
    jmp      prim_continue_k6g

align 32
culled_tri_k6g:
    ; [-] 4, 3
    mov      count1,dword ptr _count[esp]
    add      idx1,8
    ; 1, 4
    dec      count1
    mov      dword ptr _count[esp],count1
    ; 2, 2
    js       done_render_k6g
    mov      va1,dword ptr [idx1]
    ; 3, 2
    mov      vc1,dword ptr [idx1+4]
    mov      vb1,va1
    ; 4, 5 (+30)
    mov      verts2,dword ptr _verts[esp]
    and      va1,0ffffh
    ; 6 [2 - 4], 6
    and      vb1,0ffff0000h
    and      vc1,0ffffh
    ; 5 (+15)
    jmp      ind_tri_begin_k6g

align 32
done_render1_k6g:
    ; 3, 1 (+4)
    add      idx1,8
    CacheNop

done_render_k6g:
    ; 6, 6
    mov      hwptr_old,dword ptr[__asm_data + _FIFO_PTR]
    mov      dword ptr[__asm_data + _FIFO_PTR],hwptr1
    ; 2, 6
    sub      hwptr1,hwptr_old
    mov      room1,dword ptr [__asm_data + _FIFO_ROOM]
    ; 3, 4 (+31)
    shr      hwptr1,2
    mov      dword ptr _idx[esp],idx1
    ; 2 [1 - 1], 2
    sub      room1,hwptr1
    femms
    ; 6, 3
    mov      dword ptr [__asm_data + _FIFO_ROOM],room1
    add      esp,STACK_SIZE
    ; 1, 1
    pop      esi
    pop      edi
    ; 1, 1
    pop      ebx
    pop      ebp
    ; 1, 1 (+18)
    pop      esp
    ret


align 32
fifo_check_mesh_k6g:
;regs that still need to be valid on jmp
;va1,vc1,idx1,count1
    ; [-] 1, 1
    push     va1
    push     vc1
    ; 1, 5
    push     size1
    call     _fifo_make_room_asm
    ; 1, 1
    pop      size1
    pop      vc1
    ; 1, 5 (+16)
    pop      va1
    jmp      fifo_check_mesh_reentry_k6g

align 32
TriMeshTooLarge_k6g:
;saving count1,size1,room1
    ; (+16) 4, 5 (+25)
    mov      ret_addr1,dword ptr _ret_addr[esp]
    mov      dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
    ; 3, 6 [4 - 2]
    sub      count1,CHUNK_SIZE
    mov      dword ptr[__asm_data + _SAVED_COUNT],count1
    ; 8, 5
    mov      dword ptr _ret_addr[esp],TriMeshContinue_k6g
    mov      count1,CHUNK_SIZE
    ; 5 (+20)
    jmp      TriMeshReentry_k6g

align 32
TriMeshContinue_k6g:
    ; [-] 6, 5
    mov      edx,dword ptr[__asm_data + _SAVED_COUNT]
    mov      eax,dword ptr[__asm_data + _SAVED_RET_ADDR]
    ; 3, 1
    mov      dword ptr _count[esp - _params],edx
    push     eax
    ; 5 (+20)
    jmp      _DrawTriEdge2GAsm_K62O_begin

_DrawTriEdge2GAsm_K62O   ENDP

_TEXT   ENDS

END
