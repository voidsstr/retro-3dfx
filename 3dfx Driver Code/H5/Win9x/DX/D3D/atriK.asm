
TITLE          atriK.asm
.686P
.K3D

ifdef PERF_MONITORING
extrn __penter:proc
endif

; *****************************
; Data section
; *****************************
_DATA SEGMENT PAGE PUBLIC 'DATA'

fan_header_data                 dd      FAN_HEADER
;fan_header_cont_data            dd      FAN_HEADER_CONT
fan_gourad_header_data          dd      FAN_GOURAD_HEADER
                                dd      0

oneminusone     label   qword
                REAL4   1.0R
                REAL4   -1.0R

_DATA ENDS


; *****************************
; Extern section
; *****************************
EXTRN    __asm_data:BYTE
EXTRN    _fifo_make_room_asm:NEAR

_TEXT   SEGMENT PAGE PUBLIC USE32 'CODE'
        ASSUME  DS:FLAT, SS:FLAT, ES:FLAT

INCLUDE aglobal.inc

NUM_PREFETCH_TRIS = 64
POST_IDX_SIZE  = (((NUM_PREFETCH_TRIS+1) * 3) + 1)*4
STACK_SIZE     = POST_IDX_SIZE + 20

; Local stack section
_post_idx      = 0
_locals        = POST_IDX_SIZE

_area          = _locals
_hwptr         = _locals + 4
_strip_cont    = _locals + 8
_fan_cont      = _locals + 12
_last          = _locals + 16

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


align 32
PUBLIC   _PrefetchTriIdx2_5_Asm_K6
_PrefetchTriIdx2_5_Asm_K6     PROC     NEAR

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

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_5_Asm_K6_begin:
   push    esp
   push    ebp
   push    ebx
   push    edi
   push    esi
   sub     esp,STACK_SIZE

   mov     count1,dword ptr _count[esp]
   mov     chunk_size1,NUM_PREFETCH_TRIS

   sub     chunk_size1,count1
   jl      TriIdx2_5_too_large

TriIdx2_5_reentry:
   mov     idx1,dword ptr _idx[esp]
   lea     post_idx1,_post_idx[esp-12]

fetch_again:
   movzx   va1,word ptr [idx1]
   add     post_idx1,12
   movzx   vb1,word ptr [idx1+2]
   movzx   vc1,word ptr [idx1+4]

   lea     va1,dword ptr [va1*4 + va1]
   add     idx1,6

   lea     vb1,dword ptr [vb1*4 + vb1]
   mov     dword ptr [post_idx1],va1

   lea     vc1,dword ptr [vc1*4 + vc1]
   mov     dword ptr [post_idx1+4],vb1

   dec     count1
   mov     dword ptr [post_idx1+8],vc1

   jnz     fetch_again

; interface vars
;va1        TEXTEQU     <eax>
;vb1        TEXTEQU     <ebx>
;vc1        TEXTEQU     <ecx>
;count1     TEXTEQU     <edi>
;hwptr1     TEXTEQU     <esi>
;post_idx1  TEXTEQU     <ebp>

   mov     dword ptr _idx[esp],idx1
   mov     count1,dword ptr _count[esp]
   lea     post_idx1,dword ptr _post_idx[esp]

   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov     size1,count1

   lea     size1,dword ptr[size1+size1*2]
   mov     va1,dword ptr [post_idx1]
   shl     size1,3h

   inc     size1   ; reserve + 1 for pad dword

   sub     room1,size1
   mov     vb1,dword ptr [post_idx1+4]
   js      fifo_check_TriIdx2_5

fifo_reentry_TriIdx2_5:
   mov     verts2,dword ptr _verts[esp]
   mov     hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea     va1,dword ptr[verts2+va1*4]
   mov     vc1,dword ptr[post_idx1+8]
   lea     vb1,dword ptr[verts2+vb1*4]
   jmp     _DrawTriIdx2GouradAsm_K62O

align 32
exit_early:
   add     esp,STACK_SIZE
   pop     esi
   pop     edi
   pop     ebx
   pop     ebp
   pop     esp

   ret

align 32
fifo_check_TriIdx2_5:
   push    va1
   push    vb1
   push    count1
   push    size1
   call    _fifo_make_room_asm
   pop     size1
   pop     count1
   pop     vb1
   pop     va1
   jmp     fifo_reentry_TriIdx2_5

align 32
TriIdx2_5_too_large:
   mov     ret_addr1,dword ptr _ret_addr[esp]
   mov     dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub     count1,NUM_PREFETCH_TRIS
   mov     dword ptr[__asm_data + _SAVED_COUNT],count1
   mov     dword ptr _ret_addr[esp],TriIdx2_5_continue
   mov     count1,NUM_PREFETCH_TRIS
   mov     dword ptr _count[esp],count1
   jmp     TriIdx2_5_reentry

align 32
TriIdx2_5_continue:
   mov     edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov     ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov     dword ptr _count[esp -_params],edx
   push    ecx
   jmp     _PrefetchTriIdx2_5_Asm_K6_begin

_PrefetchTriIdx2_5_Asm_K6     ENDP

align 32
PUBLIC   _PrefetchTriIdx2_7_Asm_K6
_PrefetchTriIdx2_7_Asm_K6     PROC     NEAR

chunk_size1 TEXTEQU     <eax>
va1        TEXTEQU     <eax>
vb1        TEXTEQU     <ebx>
tmp1       TEXTEQU     <ecx>
size1      TEXTEQU     <ecx>
tmp2       TEXTEQU     <edx>
room1      TEXTEQU     <edx>
verts2     TEXTEQU     <edx>
ret_addr1  TEXTEQU     <edx>
count1     TEXTEQU     <edi>
idx1       TEXTEQU     <esi>
hwptr1     TEXTEQU     <esi>
post_idx1  TEXTEQU     <ebp>

ifdef PERF_MONITORING
   call    __penter
endif

_PrefetchTriIdx2_7_Asm_K6_begin:
   push    esp
   push    ebp
   push    ebx
   push    edi
   push    esi
   sub     esp,STACK_SIZE

   mov     count1,dword ptr _count[esp]
   mov     chunk_size1,NUM_PREFETCH_TRIS

   sub     chunk_size1,count1
   jl      TriIdx2_7_too_large

TriIdx2_7_reentry:
   mov     idx1,dword ptr _idx[esp]
   lea     post_idx1,_post_idx[esp-12]

fetch_again:
   movzx   va1,word ptr [idx1]
   add     post_idx1,12
   movzx   vb1,word ptr [idx1+2]

   mov     tmp1,va1
   mov     tmp2,vb1

   shl     va1,3
   add     idx1,6
   shl     vb1,3

   sub     va1,tmp1
   movzx   tmp1,word ptr [idx1-2]
   sub     vb1,tmp2

   mov     tmp2,tmp1

   mov     dword ptr [post_idx1],va1

   shl     tmp1,3
   mov     dword ptr [post_idx1+4],vb1

   sub     tmp1,tmp2
   dec     count1

   mov     dword ptr [post_idx1+8],tmp1
   jnz     fetch_again

; interface vars
;va1         TEXTEQU     <eax>
;vb1         TEXTEQU     <ebx>
;vc1         TEXTEQU     <ecx>
;count1      TEXTEQU     <edi>
;hwptr1      TEXTEQU     <esi>
;post_idx1   TEXTEQU     <ebp>

   mov     dword ptr _idx[esp],idx1
   mov     count1,dword ptr _count[esp]
   lea     post_idx1,dword ptr _post_idx[esp]

   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov     size1,count1

   lea     size1,dword ptr[size1+size1*2]
   mov     va1,dword ptr [post_idx1]
   shl     size1,3h

   inc     size1   ; reserve + 1 for pad dword

   sub     room1,size1
   mov     vb1,dword ptr [post_idx1+4]
   js      fifo_check_TriIdx2_7

fifo_reentry_TriIdx2_7:
   mov     verts2,dword ptr _verts[esp]
   mov     hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea     va1,dword ptr[verts2+va1*4]
   mov     vc1,dword ptr[post_idx1+8]
   lea     vb1,dword ptr[verts2+vb1*4]

   jmp     _DrawTriIdx2MainAsm_K62O

align 32
exit_early:
   add     esp,STACK_SIZE
   pop     esi
   pop     edi
   pop     ebx
   pop     ebp
   pop     esp

   ret

align 32
fifo_check_TriIdx2_7:
   push    va1
   push    vb1
   push    count1
   push    size1
   call    _fifo_make_room_asm
   pop     size1
   pop     count1
   pop     vb1
   pop     va1
   jmp     fifo_reentry_TriIdx2_7

align 32
TriIdx2_7_too_large:
   mov     ret_addr1,dword ptr _ret_addr[esp]
   mov     dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub     count1,NUM_PREFETCH_TRIS
   mov     dword ptr[__asm_data + _SAVED_COUNT],count1
   mov     dword ptr _ret_addr[esp],TriIdx2_7_continue
   mov     count1,NUM_PREFETCH_TRIS
   mov     dword ptr _count[esp],count1
   jmp     TriIdx2_7_reentry

align 32
TriIdx2_7_continue:
   mov     edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov     ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov     dword ptr _count[esp -_params],edx
   push    ecx
   jmp     _PrefetchTriIdx2_7_Asm_K6_begin

_PrefetchTriIdx2_7_Asm_K6     ENDP

align 32
PUBLIC   _PrefetchTriIdx2_8_Asm_K6
_PrefetchTriIdx2_8_Asm_K6     PROC     NEAR

chunk_size1 TEXTEQU     <eax>
va1        TEXTEQU     <eax>
vb1        TEXTEQU     <ebx>
tmp1       TEXTEQU     <ecx>
size1      TEXTEQU     <ecx>
tmp2       TEXTEQU     <edx>
room1      TEXTEQU     <edx>
verts2     TEXTEQU     <edx>
ret_addr1  TEXTEQU     <edx>
count1     TEXTEQU     <edi>
idx1       TEXTEQU     <esi>
hwptr1     TEXTEQU     <esi>
post_idx1  TEXTEQU     <ebp>

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_8_Asm_K6_begin:
   push    esp
   push    ebp
   push    ebx
   push    edi
   push    esi
   sub     esp,STACK_SIZE

   mov     count1,dword ptr _count[esp]
   mov     chunk_size1,NUM_PREFETCH_TRIS

   sub     chunk_size1,count1
   jl      TriIdx2_8_too_large

TriIdx2_8_reentry:
   mov     idx1,dword ptr _idx[esp]
   lea     post_idx1,_post_idx[esp-12]

fetch_again:
   movzx   va1,word ptr [idx1]
   add     post_idx1,12
   movzx   vb1,word ptr [idx1+2]

   shl     va1,3
   movzx   vc1,word ptr [idx1+4]
   add     idx1,6
   shl     vb1,3
   mov     dword ptr [post_idx1],va1
   shl     vc1,3
   mov     dword ptr [post_idx1+4],vb1

   dec     count1

   mov     dword ptr [post_idx1+8],vc1
   jnz     fetch_again

; interface vars
;va1         TEXTEQU     <eax>
;vb1         TEXTEQU     <ebx>
;vc1         TEXTEQU     <ecx>
;count1      TEXTEQU     <edi>
;hwptr1      TEXTEQU     <esi>
;post_idx1   TEXTEQU     <ebp>

   mov     dword ptr _idx[esp],idx1
   mov     count1,dword ptr _count[esp]
   lea     post_idx1,dword ptr _post_idx[esp]

   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov     size1,count1

   lea     size1,dword ptr[size1+size1*2]
   mov     va1,dword ptr [post_idx1]
   shl     size1,3h

   sub     room1,size1
   mov     vb1,dword ptr [post_idx1+4]
   js      fifo_check_TriIdx2_8

fifo_reentry_TriIdx2_8:
   mov     verts2,dword ptr _verts[esp]
   mov     hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea     va1,dword ptr[verts2+va1*4]
   mov     vc1,dword ptr[post_idx1+8]
   lea     vb1,dword ptr[verts2+vb1*4]
   jmp     _DrawTriIdx2MainAsm_K62O

exit_early:
   add     esp,STACK_SIZE
   pop     esi
   pop     edi
   pop     ebx
   pop     ebp
   pop     esp

   ret

align 32
fifo_check_TriIdx2_8:
   push    va1
   push    vb1
   push    count1
   push    size1
   call    _fifo_make_room_asm
   pop     size1
   pop     count1
   pop     vb1
   pop     va1
   jmp     fifo_reentry_TriIdx2_8

align 32
TriIdx2_8_too_large:
   mov     ret_addr1,dword ptr _ret_addr[esp]
   mov     dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub     count1,NUM_PREFETCH_TRIS
   mov     dword ptr[__asm_data + _SAVED_COUNT],count1
   mov     dword ptr _ret_addr[esp],TriIdx2_8_continue
   mov     count1,NUM_PREFETCH_TRIS
   mov     dword ptr _count[esp],count1
   jmp     TriIdx2_8_reentry

align 32
TriIdx2_8_continue:
   mov     edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov     ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov     dword ptr _count[esp -_params],edx
   push    ecx
   jmp     _PrefetchTriIdx2_8_Asm_K6_begin

_PrefetchTriIdx2_8_Asm_K6     ENDP

align 32
PUBLIC   _PrefetchTriIdx2_6T_Asm_K6
_PrefetchTriIdx2_6T_Asm_K6     PROC     NEAR

chunk_size1 TEXTEQU     <eax>
va1        TEXTEQU     <eax>
vb1        TEXTEQU     <ebx>
tmp1       TEXTEQU     <ecx>
size1      TEXTEQU     <ecx>
tmp2       TEXTEQU     <edx>
room1      TEXTEQU     <edx>
verts2     TEXTEQU     <edx>
ret_addr1  TEXTEQU     <edx>
count1     TEXTEQU     <edi>
idx1       TEXTEQU     <esi>
hwptr1     TEXTEQU     <esi>
post_idx1  TEXTEQU     <ebp>

ifdef PERF_MONITORING
   call    __penter
endif

_PrefetchTriIdx2_6T_Asm_K6_begin:
   push    esp
   push    ebp
   push    ebx
   push    edi
   push    esi
   sub     esp,STACK_SIZE

   mov     count1,dword ptr _count[esp]
   mov     dword ptr _strip_cont[esp],BIT_23
   mov     chunk_size1,NUM_PREFETCH_TRIS

   sub     chunk_size1,count1
   mov     dword ptr _fan_cont[esp],BIT_22
   jl      TriIdx2_6_too_large

TriIdx2_6_reentry:
   mov     idx1,dword ptr _idx[esp]
   lea     post_idx1,_post_idx[esp-12]

fetch_again:
   movzx   va1,word ptr [idx1]
   add     post_idx1,12
   movzx   vb1,word ptr [idx1+2]
   movzx   vc1,word ptr [idx1+4]

   shl     va1,1
   add     idx1,6
   shl     vb1,1

   lea     va1,dword ptr [va1*2 + va1]
   shl     vc1,1

   lea     vb1,dword ptr [vb1*2 + vb1]
   mov     dword ptr [post_idx1],va1

   lea     vc1,dword ptr [vc1*2 + vc1]
   mov     dword ptr [post_idx1+4],vb1

   dec     count1
   mov     dword ptr [post_idx1+8],vc1

   jnz     fetch_again

   mov     dword ptr [post_idx1+12],0ffffffffh
   mov     dword ptr [post_idx1+16],0ffffffffh
   mov     dword ptr [post_idx1+20],0ffffffffh

   mov     dword ptr _idx[esp],idx1
   mov     count1,dword ptr _count[esp]
   lea     post_idx1,dword ptr _post_idx[esp]

   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov     size1,count1

   lea     size1,dword ptr[size1+size1*2]
   mov     va1,dword ptr [post_idx1]
   shl     size1,3h

   sub     room1,size1
   mov     vb1,dword ptr [post_idx1+4]
   js      fifo_check_TriIdx2_6

fifo_reentry_TriIdx2_6:
   mov     verts2,dword ptr _verts[esp]
   mov     hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea     va1,dword ptr[verts2+va1*4]
   mov     vc1,dword ptr[post_idx1+8]
   lea     vb1,dword ptr[verts2+vb1*4]
   mov     dword ptr _hwptr[esp],hwptr1
   jmp     _DrawTriIdx2MainAsm_K62O

exit_early:
   add     esp,STACK_SIZE
   pop     esi
   pop     edi
   pop     ebx
   pop     ebp
   pop     esp

   ret

align 32
fifo_check_TriIdx2_6:
   push    va1
   push    vb1
   push    count1
   push    size1
   call    _fifo_make_room_asm
   pop     size1
   pop     count1
   pop     vb1
   pop     va1
   jmp     fifo_reentry_TriIdx2_6

align 32
TriIdx2_6_too_large:
   mov     ret_addr1,dword ptr _ret_addr[esp]
   mov     dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub     count1,NUM_PREFETCH_TRIS
   mov     dword ptr[__asm_data + _SAVED_COUNT],count1
   mov     dword ptr _ret_addr[esp],TriIdx2_6_continue
   mov     count1,NUM_PREFETCH_TRIS
   mov     dword ptr _count[esp],count1
   jmp     TriIdx2_6_reentry

align 32
TriIdx2_6_continue:
   mov     edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov     ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov     dword ptr _count[esp -_params],edx
   push    ecx
   jmp     _PrefetchTriIdx2_6T_Asm_K6_begin

_PrefetchTriIdx2_6T_Asm_K6     ENDP

align 32
PUBLIC   _PrefetchTriIdx2_6G_Asm_K6
_PrefetchTriIdx2_6G_Asm_K6     PROC     NEAR

chunk_size1 TEXTEQU     <eax>
va1        TEXTEQU     <eax>
vb1        TEXTEQU     <ebx>
tmp1       TEXTEQU     <ecx>
size1      TEXTEQU     <ecx>
tmp2       TEXTEQU     <edx>
room1      TEXTEQU     <edx>
verts2     TEXTEQU     <edx>
ret_addr1  TEXTEQU     <edx>
count1     TEXTEQU     <edi>
idx1       TEXTEQU     <esi>
hwptr1     TEXTEQU     <esi>
post_idx1  TEXTEQU     <ebp>

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_6G_Asm_K6_begin:
   push    esp
   push    ebp
   push    ebx
   push    edi
   push    esi
   sub     esp,STACK_SIZE

   mov     count1,dword ptr _count[esp]
   mov     dword ptr _strip_cont[esp],BIT_23
   mov     chunk_size1,NUM_PREFETCH_TRIS

   sub     chunk_size1,count1
   mov     dword ptr _fan_cont[esp],BIT_22
   jl      TriIdx2_6_too_large

TriIdx2_6_reentry:
   mov     idx1,dword ptr _idx[esp]
   lea     post_idx1,_post_idx[esp-12]

fetch_again:
   movzx   va1,word ptr [idx1]
   add     post_idx1,12
   movzx   vb1,word ptr [idx1+2]
   movzx   vc1,word ptr [idx1+4]

   shl     va1,1
   add     idx1,6
   shl     vb1,1

   lea     va1,dword ptr [va1*2 + va1]
   shl     vc1,1

   lea     vb1,dword ptr [vb1*2 + vb1]
   mov     dword ptr [post_idx1],va1

   lea     vc1,dword ptr [vc1*2 + vc1]
   mov     dword ptr [post_idx1+4],vb1

   dec     count1
   mov     dword ptr [post_idx1+8],vc1

   jnz     fetch_again

   mov     dword ptr [post_idx1+12],0ffffffffh
   mov     dword ptr [post_idx1+16],0ffffffffh
   mov     dword ptr [post_idx1+20],0ffffffffh

   mov     dword ptr _idx[esp],idx1
   mov     count1,dword ptr _count[esp]
   lea     post_idx1,dword ptr _post_idx[esp]

   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov     size1,count1

   lea     size1,dword ptr[size1+size1*2]
   mov     va1,dword ptr [post_idx1]
   shl     size1,3h

   sub     room1,size1
   mov     vb1,dword ptr [post_idx1+4]
   js      fifo_check_TriIdx2_6

fifo_reentry_TriIdx2_6:
   mov     verts2,dword ptr _verts[esp]
   mov     hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea     va1,dword ptr[verts2+va1*4]
   mov     vc1,dword ptr[post_idx1+8]
   lea     vb1,dword ptr[verts2+vb1*4]
   mov     dword ptr _hwptr[esp],hwptr1
   jmp     _DrawTriIdx2GouradAsm_K62O

exit_early:
   add     esp,STACK_SIZE
   pop     esi
   pop     edi
   pop     ebx
   pop     ebp
   pop     esp

   ret

align 32
fifo_check_TriIdx2_6:
   push    va1
   push    vb1
   push    count1
   push    size1
   call    _fifo_make_room_asm
   pop     size1
   pop     count1
   pop     vb1
   pop     va1
   jmp     fifo_reentry_TriIdx2_6

align 32
TriIdx2_6_too_large:
   mov     ret_addr1,dword ptr _ret_addr[esp]
   mov     dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub     count1,NUM_PREFETCH_TRIS
   mov     dword ptr[__asm_data + _SAVED_COUNT],count1
   mov     dword ptr _ret_addr[esp],TriIdx2_6_continue
   mov     count1,NUM_PREFETCH_TRIS
   mov     dword ptr _count[esp],count1
   jmp     TriIdx2_6_reentry

align 32
TriIdx2_6_continue:
   mov     edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov     ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov     dword ptr _count[esp -_params],edx
   push    ecx
   jmp     _PrefetchTriIdx2_6G_Asm_K6_begin

_PrefetchTriIdx2_6G_Asm_K6     ENDP

align 32
PUBLIC   _PrefetchTriIdx2_9_Asm_K6
_PrefetchTriIdx2_9_Asm_K6     PROC     NEAR

chunk_size1 TEXTEQU     <eax>
va1        TEXTEQU     <eax>
vb1        TEXTEQU     <ebx>
tmp1       TEXTEQU     <ecx>
size1      TEXTEQU     <ecx>
tmp2       TEXTEQU     <edx>
room1      TEXTEQU     <edx>
verts2     TEXTEQU     <edx>
ret_addr1  TEXTEQU     <edx>
count1     TEXTEQU     <edi>
idx1       TEXTEQU     <esi>
hwptr1     TEXTEQU     <esi>
post_idx1  TEXTEQU     <ebp>

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_9_Asm_K6_begin:
   push    esp
   push    ebp
   push    ebx
   push    edi
   push    esi
   sub     esp,STACK_SIZE

   mov     count1,dword ptr _count[esp]
   mov     dword ptr _strip_cont[esp],BIT_23
   mov     chunk_size1,NUM_PREFETCH_TRIS

   sub     chunk_size1,count1
   mov     dword ptr _fan_cont[esp],BIT_22
   jl      TriIdx2_9_too_large

TriIdx2_9_reentry:
   mov     idx1,dword ptr _idx[esp]
   lea     post_idx1,_post_idx[esp-12]

fetch_again:
   movzx   va1,word ptr [idx1]
   add     post_idx1,12
   movzx   vb1,word ptr [idx1+2]

   mov     tmp1,va1
   mov     tmp2,vb1

   shl     va1,3
   add     idx1,6
   shl     vb1,3

   add     va1,tmp1
   movzx   tmp1,word ptr [idx1-2]
   add     vb1,tmp2

   mov     tmp2,tmp1

   mov     dword ptr [post_idx1],va1

   shl     tmp1,3
   mov     dword ptr [post_idx1+4],vb1

   add     tmp1,tmp2
   dec     count1

   mov     dword ptr [post_idx1+8],tmp1
   jnz     fetch_again

   mov     dword ptr [post_idx1+12],0ffffffffh
   mov     dword ptr [post_idx1+16],0ffffffffh
   mov     dword ptr [post_idx1+20],0ffffffffh

   mov     dword ptr _idx[esp],idx1
   mov     count1,dword ptr _count[esp]
   lea     post_idx1,dword ptr _post_idx[esp]

   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov     size1,count1

   lea     size1,dword ptr[size1+size1*2]
   mov     va1,dword ptr [post_idx1]
   shl     size1,3h

   sub     room1,size1
   mov     vb1,dword ptr [post_idx1+4]
   js      fifo_check_TriIdx2_9

fifo_reentry_TriIdx2_9:
   mov     verts2,dword ptr _verts[esp]
   mov     hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea     va1,dword ptr[verts2+va1*4]
   mov     vc1,dword ptr[post_idx1+8]
   lea     vb1,dword ptr[verts2+vb1*4]
   mov     dword ptr _hwptr[esp],hwptr1
   jmp     _DrawTriIdx2MainAsm_K62O

exit_early:
   add     esp,STACK_SIZE
   pop     esi
   pop     edi
   pop     ebx
   pop     ebp
   pop     esp

   ret

align 32
fifo_check_TriIdx2_9:
   push    va1
   push    vb1
   push    count1
   push    size1
   call    _fifo_make_room_asm
   pop     size1
   pop     count1
   pop     vb1
   pop     va1
   jmp     fifo_reentry_TriIdx2_9

align 32
TriIdx2_9_too_large:
   mov     ret_addr1,dword ptr _ret_addr[esp]
   mov     dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub     count1,NUM_PREFETCH_TRIS
   mov     dword ptr[__asm_data + _SAVED_COUNT],count1
   mov     dword ptr _ret_addr[esp],TriIdx2_9_continue
   mov     count1,NUM_PREFETCH_TRIS
   mov     dword ptr _count[esp],count1
   jmp     TriIdx2_9_reentry

align 32
TriIdx2_9_continue:
   mov     edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov     ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov     dword ptr _count[esp -_params],edx
   push    ecx
   jmp     _PrefetchTriIdx2_9_Asm_K6_begin

_PrefetchTriIdx2_9_Asm_K6     ENDP

align 32
PUBLIC   _PrefetchTriIdx2_10_Asm_K6
_PrefetchTriIdx2_10_Asm_K6     PROC     NEAR

chunk_size1 TEXTEQU     <eax>
va1        TEXTEQU     <eax>
vb1        TEXTEQU     <ebx>
tmp1       TEXTEQU     <ecx>
size1      TEXTEQU     <ecx>
tmp2       TEXTEQU     <edx>
room1      TEXTEQU     <edx>
verts2     TEXTEQU     <edx>
ret_addr1  TEXTEQU     <edx>
count1     TEXTEQU     <edi>
idx1       TEXTEQU     <esi>
hwptr1     TEXTEQU     <esi>
post_idx1  TEXTEQU     <ebp>

ifdef PERF_MONITORING
   call    __penter
endif

_PrefetchTriIdx2_10_Asm_K6_begin:
   push    esp
   push    ebp
   push    ebx
   push    edi
   push    esi
   sub     esp,STACK_SIZE

   mov     count1,dword ptr _count[esp]
   mov     dword ptr _strip_cont[esp],BIT_23
   mov     chunk_size1,NUM_PREFETCH_TRIS

   sub     chunk_size1,count1
   mov     dword ptr _fan_cont[esp],BIT_22
   jl      TriIdx2_10_too_large

TriIdx2_10_reentry:
   mov     idx1,dword ptr _idx[esp]
   lea     post_idx1,_post_idx[esp-12]

fetch_again:
   movzx   va1,word ptr [idx1]
   add     post_idx1,12
   movzx   vb1,word ptr [idx1+2]
   movzx   vc1,word ptr [idx1+4]

   shl     va1,1
   add     idx1,6
   shl     vb1,1

   lea     va1,dword ptr [va1*4 + va1]
   shl     vc1,1

   lea     vb1,dword ptr [vb1*4 + vb1]
   mov     dword ptr [post_idx1],va1

   lea     vc1,dword ptr [vc1*4 + vc1]
   mov     dword ptr [post_idx1+4],vb1

   dec     count1
   mov     dword ptr [post_idx1+8],vc1

   jnz     fetch_again

   mov     dword ptr [post_idx1+12],0ffffffffh
   mov     dword ptr [post_idx1+16],0ffffffffh
   mov     dword ptr [post_idx1+20],0ffffffffh

   mov     dword ptr _idx[esp],idx1
   mov     count1,dword ptr _count[esp]
   lea     post_idx1,dword ptr _post_idx[esp]

   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov     size1,count1

   lea     size1,dword ptr[size1+size1*2]
   mov     va1,dword ptr [post_idx1]
   shl     size1,3h

   sub     room1,size1
   mov     vb1,dword ptr [post_idx1+4]
   js      fifo_check_TriIdx2_10

fifo_reentry_TriIdx2_10:
   mov     verts2,dword ptr _verts[esp]
   mov     hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea     va1,dword ptr[verts2+va1*4]
   mov     vc1,dword ptr[post_idx1+8]
   lea     vb1,dword ptr[verts2+vb1*4]
   mov     dword ptr _hwptr[esp],hwptr1
   jmp     _DrawTriIdx2MainAsm_K62O

exit_early:
   add     esp,STACK_SIZE
   pop     esi
   pop     edi
   pop     ebx
   pop     ebp
   pop     esp

   ret

align 32
fifo_check_TriIdx2_10:
   push    va1
   push    vb1
   push    count1
   push    size1
   call    _fifo_make_room_asm
   pop     size1
   pop     count1
   pop     vb1
   pop     va1
   jmp     fifo_reentry_TriIdx2_10

align 32
TriIdx2_10_too_large:
   mov     ret_addr1,dword ptr _ret_addr[esp]
   mov     dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub     count1,NUM_PREFETCH_TRIS
   mov     dword ptr[__asm_data + _SAVED_COUNT],count1
   mov     dword ptr _ret_addr[esp],TriIdx2_10_continue
   mov     count1,NUM_PREFETCH_TRIS
   mov     dword ptr _count[esp],count1
   jmp     TriIdx2_10_reentry

align 32
TriIdx2_10_continue:
   mov     edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov     ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov     dword ptr _count[esp -_params],edx
   push    ecx
   jmp     _PrefetchTriIdx2_10_Asm_K6_begin

_PrefetchTriIdx2_10_Asm_K6     ENDP

align 32
PUBLIC   _PrefetchTriIdx2_11_Asm_K6
_PrefetchTriIdx2_11_Asm_K6     PROC     NEAR

chunk_size1 TEXTEQU     <eax>
va1        TEXTEQU     <eax>
vb1        TEXTEQU     <ebx>
tmp1       TEXTEQU     <ecx>
size1      TEXTEQU     <ecx>
tmp2       TEXTEQU     <edx>
room1      TEXTEQU     <edx>
verts2     TEXTEQU     <edx>
ret_addr1  TEXTEQU     <edx>
count1     TEXTEQU     <edi>
idx1       TEXTEQU     <esi>
hwptr1     TEXTEQU     <esi>
post_idx1  TEXTEQU     <ebp>

ifdef PERF_MONITORING
   call    __penter
endif

_PrefetchTriIdx2_11_Asm_K6_begin:
   push    esp
   push    ebp
   push    ebx
   push    edi
   push    esi
   sub     esp,STACK_SIZE

   mov     count1,dword ptr _count[esp]
   mov     dword ptr _strip_cont[esp],BIT_23
   mov     chunk_size1,NUM_PREFETCH_TRIS

   sub     chunk_size1,count1
   mov     dword ptr _fan_cont[esp],BIT_22
   jl      TriIdx2_11_too_large

TriIdx2_11_reentry:
   mov     idx1,dword ptr _idx[esp]
   lea     post_idx1,_post_idx[esp-12]

fetch_again:
   movzx   va1,word ptr [idx1]
   add     post_idx1,12
   movzx   vb1,word ptr [idx1+2]

   mov     tmp1,va1
   shl     va1,1

   mov     tmp2,vb1
   shl     vb1,1

   lea     va1,dword ptr [va1*4 + va1]
   add     idx1,6

   lea     vb1,dword ptr [vb1*4 + vb1]
   add     va1,tmp1
   movzx   vc1,word ptr [idx1-2]

   add     vb1,tmp2
   mov     tmp2,vc1
   shl     vc1,1

   mov     dword ptr [post_idx1],va1
   lea     vc1,dword ptr [vc1*4 + vc1]

   mov     dword ptr [post_idx1+4],vb1
   add     vc1,tmp2

   dec     count1
   mov     dword ptr [post_idx1+8],vc1

   jnz     fetch_again

   mov     dword ptr [post_idx1+12],0ffffffffh
   mov     dword ptr [post_idx1+16],0ffffffffh
   mov     dword ptr [post_idx1+20],0ffffffffh

   mov     dword ptr _idx[esp],idx1
   mov     count1,dword ptr _count[esp]
   lea     post_idx1,dword ptr _post_idx[esp]

   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov     size1,count1

   lea     size1,dword ptr[size1+size1*2]
   mov     va1,dword ptr [post_idx1]
   shl     size1,3h

   sub     room1,size1
   mov     vb1,dword ptr [post_idx1+4]
   js      fifo_check_TriIdx2_11

fifo_reentry_TriIdx2_11:
   mov     verts2,dword ptr _verts[esp]
   mov     hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea     va1,dword ptr[verts2+va1*4]
   mov     vc1,dword ptr[post_idx1+8]
   lea     vb1,dword ptr[verts2+vb1*4]
   mov     dword ptr _hwptr[esp],hwptr1
   jmp     _DrawTriIdx2MainAsm_K62O

exit_early:
   add     esp,STACK_SIZE
   pop     esi
   pop     edi
   pop     ebx
   pop     ebp
   pop     esp

   ret

align 32
fifo_check_TriIdx2_11:
   push    va1
   push    vb1
   push    count1
   push    size1
   call    _fifo_make_room_asm
   pop     size1
   pop     count1
   pop     vb1
   pop     va1
   jmp     fifo_reentry_TriIdx2_11

align 32
TriIdx2_11_too_large:
   mov     ret_addr1,dword ptr _ret_addr[esp]
   mov     dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub     count1,NUM_PREFETCH_TRIS
   mov     dword ptr[__asm_data + _SAVED_COUNT],count1
   mov     dword ptr _ret_addr[esp],TriIdx2_11_continue
   mov     count1,NUM_PREFETCH_TRIS
   mov     dword ptr _count[esp],count1
   jmp     TriIdx2_11_reentry

align 32
TriIdx2_11_continue:
   mov     edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov     ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov     dword ptr _count[esp -_params],edx
   push    ecx
   jmp     _PrefetchTriIdx2_11_Asm_K6_begin

_PrefetchTriIdx2_11_Asm_K6     ENDP

align 32
PUBLIC   _PrefetchTriIdx2_12_Asm_K6
_PrefetchTriIdx2_12_Asm_K6     PROC     NEAR

chunk_size1 TEXTEQU     <eax>
va1        TEXTEQU     <eax>
vb1        TEXTEQU     <ebx>
tmp1       TEXTEQU     <ecx>
size1      TEXTEQU     <ecx>
tmp2       TEXTEQU     <edx>
room1      TEXTEQU     <edx>
verts2     TEXTEQU     <edx>
ret_addr1  TEXTEQU     <edx>
count1     TEXTEQU     <edi>
idx1       TEXTEQU     <esi>
hwptr1     TEXTEQU     <esi>
post_idx1  TEXTEQU     <ebp>

ifdef PERF_MONITORING
   call    __penter
endif

_PrefetchTriIdx2_12_Asm_K6_begin:
   push    esp
   push    ebp
   push    ebx
   push    edi
   push    esi
   sub     esp,STACK_SIZE

   mov     count1,dword ptr _count[esp]
   mov     dword ptr _strip_cont[esp],BIT_23
   mov     chunk_size1,NUM_PREFETCH_TRIS

   sub     chunk_size1,count1
   mov     dword ptr _fan_cont[esp],BIT_22
   jl      TriIdx2_12_too_large

TriIdx2_12_reentry:
   mov     idx1,dword ptr _idx[esp]
   lea     post_idx1,_post_idx[esp-12]

fetch_again:
   movzx   va1,word ptr [idx1]
   add     post_idx1,12
   movzx   vb1,word ptr [idx1+2]
   movzx   vc1,word ptr [idx1+4]

   shl     va1,2
   add     idx1,6
   shl     vb1,2

   lea     va1,dword ptr [va1*2 + va1]
   shl     vc1,2

   lea     vb1,dword ptr [vb1*2 + vb1]
   mov     dword ptr [post_idx1],va1

   lea     vc1,dword ptr [vc1*2 + vc1]
   mov     dword ptr [post_idx1+4],vb1

   dec     count1
   mov     dword ptr [post_idx1+8],vc1

   jnz     fetch_again

   mov     dword ptr [post_idx1+12],0ffffffffh
   mov     dword ptr [post_idx1+16],0ffffffffh
   mov     dword ptr [post_idx1+20],0ffffffffh

   mov     dword ptr _idx[esp],idx1
   mov     count1,dword ptr _count[esp]
   lea     post_idx1,dword ptr _post_idx[esp]

   mov     room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov     size1,count1

   lea     size1,dword ptr[size1+size1*2]
   mov     va1,dword ptr [post_idx1]
   shl     size1,3h

   sub     room1,size1
   mov     vb1,dword ptr [post_idx1+4]
   js      fifo_check_TriIdx2_12

fifo_reentry_TriIdx2_12:
   mov     verts2,dword ptr _verts[esp]
   mov     hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea     va1,dword ptr[verts2+va1*4]
   mov     vc1,dword ptr[post_idx1+8]
   lea     vb1,dword ptr[verts2+vb1*4]
   mov     dword ptr _hwptr[esp],hwptr1
   jmp     _DrawTriIdx2MainAsm_K62O

exit_early:
   add     esp,STACK_SIZE
   pop     esi
   pop     edi
   pop     ebx
   pop     ebp
   pop     esp

   ret

align 32
fifo_check_TriIdx2_12:
   push    va1
   push    vb1
   push    count1
   push    size1
   call    _fifo_make_room_asm
   pop     size1
   pop     count1
   pop     vb1
   pop     va1
   jmp     fifo_reentry_TriIdx2_12

align 32
TriIdx2_12_too_large:
   mov     ret_addr1,dword ptr _ret_addr[esp]
   mov     dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub     count1,NUM_PREFETCH_TRIS
   mov     dword ptr[__asm_data + _SAVED_COUNT],count1
   mov     dword ptr _ret_addr[esp],TriIdx2_12_continue
   mov     count1,NUM_PREFETCH_TRIS
   mov     dword ptr _count[esp],count1
   jmp     TriIdx2_12_reentry

align 32
TriIdx2_12_continue:
   mov     edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov     ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov     dword ptr _count[esp -_params],edx
   push    ecx
   jmp     _PrefetchTriIdx2_12_Asm_K6_begin

_PrefetchTriIdx2_12_Asm_K6     ENDP

; Core rendering routine (MMX+3DNOW version)
; supports x,y,z,rhw,color,s,t
; requires rhw
; color,s,t are unnecessary but sent to hardware

align 32
PUBLIC   _DrawTriIdx2GouradAsm_K62O
_DrawTriIdx2GouradAsm_K62O   PROC     NEAR

va1        TEXTEQU     <eax>
vb1        TEXTEQU     <ebx>
vc1        TEXTEQU     <ecx>
verts2     TEXTEQU     <edx>
color1     TEXTEQU     <edx>
room2      TEXTEQU     <edx>
count1     TEXTEQU     <edi>
area1      TEXTEQU     <edi>
rhw1       TEXTEQU     <edi>
hwptr1     TEXTEQU     <esi>
post_idx1  TEXTEQU     <ebp>
hwptr_old  TEXTEQU     <ebp>

   femms
   test    hwptr1, 4

   movd    mm6, dword ptr [__asm_data + _PIXEL_CENTER]
   jz      @F

   add     hwptr1, 4
   mov     dword ptr [hwptr1-4], 0
@@:
   punpckldq mm6, mm6
   jmp     do_cycle_k6g

align 32
do_cycle_k6g:
   dec     count1
   lea     vc1,dword ptr [verts2 + vc1*4]

   js      done_render_k6g
   movq    mm0, qword ptr sx[va1]     ; mm0: pA->sy | pA->sx

   pcmpeqd mm7, mm7                 ; mm7: all 1's
   movq    mm1, qword ptr sx[vb1]   ; mm1: pB->sy | pB->sx

   psllq   mm7, 63    ; mm7: -0 | 0
   movq    mm3, qword ptr sx[vc1]     ; mm2: pC->sy | pC->sx

   pfsub   mm0, mm1    ; mm0: pA->sy - pB->sy | pA->sx - pB->sx
   movq    mm5, qword ptr sx[vb1]

   pfsub   mm1, mm3    ; mm1: pB->sy - pC->sy | pB->sx - pC->sx
   mov     dword ptr _count[esp],count1

   punpckldq mm3, mm1  ; mm2: pB->sx - pC->sx | (ni)
   punpckhdq mm1, mm3  ; mm1: pB->sx - pC->sx |  pB->sy - pC->sy

   pfadd   mm5, mm6  ; mm5: pB->sy + PIXEL_OFFSET | pB->sx + PIXEL_OFFSET
   pfmul   mm0, mm1    ; mm0: (pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)

   movd    mm3, dword ptr sz[va1] ; mm3: 0 | pA->sz
   pxor    mm0, mm7 ; mm0: -(pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)

   movd    mm7, dword ptr [__asm_data + _SCALEZ]
   pfacc   mm0, mm0

   movq    mm1, qword ptr sx[va1]  ; mm1 : pA->sy | pA->sx
   pfmul   mm3, mm7 ; mm3: 0 | ZSCALE( pA->sz )

   pfadd   mm1, mm6   ; mm1: pA->sy+PIXEL_OFFSET | pA->sx+PIXEL_OFFSET
   movd    area1, mm0

   punpckldq mm3, qword ptr rhw[va1]
   and     area1, 80000000h

   movd    mm0, dword ptr fan_gourad_header_data
   punpckldq mm0, mm1

   punpckhdq mm1, mm1
   xor     area1,dword ptr [__asm_data +_CULL_MASK]

   punpckldq mm1, qword ptr color[va1]
   jz      culled_tri_k6g

   movd    mm7, dword ptr sz[vb1]
   add     post_idx1,12

   ;movq     qword ptr [hwptr1], mm0  ; <-- fan_gourad_header, pA->sx+PIXEL_OFFSET
   movq_esi_mm0
   mov     verts2,dword ptr _verts[esp]

   movd    mm0, dword ptr [__asm_data + _SCALEZ]
   movq    qword ptr [hwptr1+8*1], mm1 ; <-- pA->sy+PIXEL_OFFSET, aColor

   pfmul   mm7, mm0 ; mm3: 0 | ZSCALE( pB->sz )
   movq    qword ptr [hwptr1+8*2], mm3 ; <-- ZSCALE( pA->sz ), pA-rhw

   movd    mm0, dword ptr color[vb1]
   movq    qword ptr [hwptr1+8*3], mm5 ; <-- pB->sx + PIXEL_OFFSET, pB->sy + PIXEL_OFFSET

   movd    mm1, dword ptr rhw[vc1]
   punpckldq mm0, mm7

   movq    mm3, qword ptr sx[vc1] ; mm3: pC->sy | pC->sx
   movq    qword ptr [hwptr1+8*4], mm0 ; <-- bColor, ZSCALE( pB->sz )

   movd    mm5, dword ptr sz[vc1] ; mm5: 0 | pC->sz
   pfadd   mm3, mm6 ; mm3: pC->sy + PIXEL_OFFSET | pC->sx + PIXEL_OFFSET

   pfmul   mm5, qword ptr [__asm_data + _SCALEZ] ; mm5: 0 | ZSCALE( pC->sz )
   punpckldq mm1, mm3

   mov     va1, dword ptr[post_idx1]
   punpckhdq mm3, mm3

   punpckldq mm5, qword ptr rhw[vc1]
   movq    qword ptr [hwptr1+8*5], mm1 ; <-- pB->rhw, pC->sx + PIXEL_OFFSET

   punpckldq mm3, qword ptr color[vc1]
   mov     vb1, dword ptr[post_idx1+4]

   mov     count1, dword ptr _count[esp]
   movq    qword ptr [hwptr1+8*6], mm3 ; <-- pC->sy + PIXEL_OFFSET, cColor

   lea     va1, dword ptr [verts2+va1*4]
   movq    qword ptr [hwptr1+8*7], mm5 ; <-- ZSCALE( pC->sz ), pC->rhw

   mov     vc1, dword ptr[post_idx1+8]
   add     hwptr1,8*7+8

   lea     vb1, dword ptr [verts2+vb1*4]
   jmp     _DrawTriIdx2GouradAsm_K62O

align 32
culled_tri_k6g:
   add     post_idx1,12
   mov     verts2,dword ptr _verts[esp]

   mov     va1,dword ptr [post_idx1]
   mov     vb1,dword ptr [post_idx1+4]

   mov     vc1,dword ptr [post_idx1+8]
   lea     va1,dword ptr [verts2+va1*4]

   mov     count1,dword ptr _count[esp]
   lea     vb1,dword ptr [verts2+vb1*4]

   jmp     do_cycle_k6g

align 32
done_render_k6g:
   mov     hwptr_old,dword ptr[__asm_data + _FIFO_PTR]
   mov     dword ptr[__asm_data + _FIFO_PTR],hwptr1

   sub     hwptr1,hwptr_old
   mov     room2,dword ptr [__asm_data + _FIFO_ROOM]

   shr     hwptr1,2
   sub     room2,hwptr1

   mov     dword ptr [__asm_data + _FIFO_ROOM],room2
   femms

   mov     eax,dword ptr _idx[esp]
   add     esp,STACK_SIZE

   pop     esi
   pop     edi

   pop     ebx
   pop     ebp

   pop     esp
   ret

_DrawTriIdx2GouradAsm_K62O   ENDP

align 32

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

   femms
   test    hwptr1, 4

   movd    mm6, dword ptr [__asm_data + _PIXEL_CENTER]

   movq    mm2, qword ptr [__asm_data + _TEXEL_OFFSET_S]

ifdef DCT_FIX
   pfmul   mm2, qword ptr [__asm_data + _SCALEWS+8 ]  ; Pre multiply S& T scale factors with correction.
endif

   punpckldq mm6, mm6
   jz      @F

   add     hwptr1, 4
   mov     dword ptr [hwptr1-4], 0
@@:

   movq    mm4, qword ptr [__asm_data + _SCALE_S]

align 32
do_cycle_k6m:

   lea     vc1,dword ptr [verts2 + vc1*4]
   dec     count1

   mov     s0,dword ptr[__asm_data + _T0_OFFSET]
   js      done_render_k6m

   movq    mm0, qword ptr sx[va1]     ; mm0: pA->sy | pA->sx

   movq    mm1, qword ptr sx[vb1]   ; mm1: pB->sy | pB->sx

   movq    mm3, qword ptr sx[vc1]     ; mm2: pC->sy | pC->sx
   pfsub   mm0, mm1    ; mm0: pA->sy - pB->sy | pA->sx - pB->sx

   movq    mm7, qword ptr [s0 + va1] ; mm3: pA->tv | pA->tu
   pfsub   mm1, mm3    ; mm1: pB->sy - pC->sy | pB->sx - pC->sx

   movd    mm5, dword ptr rhw[va1] ; mm5: - | pA->rhw
   punpckldq mm3, mm1  ; mm2: pB->sx - pC->sx | (ni)
   punpckldq mm5, mm5

ifdef DCT_FIX
   pfmul   mm5, qword ptr [__asm_data + _SCALEWS+8 ]  ; multiply w scale factor with correction.
endif

   mov      dword ptr _count[esp], count1

   punpckhdq mm1, mm3  ; mm1: pB->sx - pC->sx |  pB->sy - pC->sy
   pcmpeqd mm3, mm3

   pfmul   mm0, mm1    ; mm0: (pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)
   psllq   mm3, 63

   movq    mm1, qword ptr sx[va1]  ; mm1 : pA->sy | pA->sx
   pxor    mm0, mm3 ; mm0: -(pB->sx - pC->sx)*(pA->sy - pB->sy) | (pA->sx - pB->sx)*(pB->sy - pC->sy)

   pfadd   mm1, mm6   ; mm1: pA->sy+PIXEL_OFFSET | pA->sx+PIXEL_OFFSET
   movd    mm3, dword ptr sz[va1] ; mm3: 0 | pA->sz

   pfacc   mm0, mm0
   nop

   movd    area1, mm0
   movd    mm0, dword ptr fan_header_data

   and     area1,80000000h
   punpckldq mm0, mm1

   xor     area1,dword ptr [__asm_data +_CULL_MASK]
   punpckhdq mm1, mm1

   punpckldq mm1, qword ptr color[va1]
   jz      culled_tri_k6m

   ;movq    qword ptr [hwptr1], mm0
   movq_esi_mm0
   pfmul   mm7, mm4  ; mm3: pA->tv*l_lscaleT | pA->tu*l_lscaleS

   movd    mm0, dword ptr [__asm_data + _SCALEZ]
   movq    qword ptr [hwptr1+8*1], mm1

   pfmul   mm3, mm0 ; mm3: 0 | ZSCALE( pA->sz )
   movq    mm0, qword ptr sx[vb1]

   punpckldq mm3, mm5
   pfadd   mm7, mm2

   movd    mm1, dword ptr color[vb1]
   movq    qword ptr [hwptr1+8*2], mm3

   pfmul   mm7, mm5
   pfadd   mm0, mm6  ; mm0: pB->sy + PIXEL_OFFSET | pB->sx + PIXEL_OFFSET

   movq    qword ptr [hwptr1+8*3], mm7
   movd    mm7, dword ptr [__asm_data + _SCALEZ]

   movd    mm3, dword ptr sz[vb1]
   movq    qword ptr [hwptr1+8*4], mm0

   movd    mm5, dword ptr rhw[vb1]
   pfmul   mm3, mm7 ; mm3: 0 | ZSCALE( pB->sz )

   movq    mm7, qword ptr [s0 + vb1] ; mm7: pB->tv | pB->tu
   punpckldq mm5, mm5 ; mm5: pB->rhw | pB->rhw

ifdef DCT_FIX
   pfmul   mm5, qword ptr [__asm_data + _SCALEWS+8 ]  ; multiply w scale factor with correction.
endif

   punpckldq mm1, mm3 ; mm1: ZSCALE( pB->sz ) | bColor
   pfmul   mm7, mm4 ; mm7: pB->tv*l_lscaleT | pB->tu*l_lscaleS

   movq    qword ptr [hwptr1+8*5], mm1
   pfadd   mm7, mm2 ; mm7: pB->tv*l_lscaleT + TEXEL_SOFFSET | pB->tu*l_lscaleS + TEXEL_SOFFSET

   movq    mm0, qword ptr sx[vc1] ; mm0: pC->sy | pC->sx
   pfmul   mm7, mm5 ; mm7: (pB->tv*l_lscaleT + TEXEL_SOFFSET)*pB->rhw | (pB->tu*l_lscaleS + TEXEL_SOFFSET)*pB->rhw

   movd    mm3, dword ptr [__asm_data + _SCALEZ]
   pfadd   mm0, mm6 ; mm0: pC->sy + PIXEL_OFFSET | pC->sx + PIXEL_OFFSET

   movd    mm1, dword ptr sz[vc1]
   punpckldq mm5, mm7 ; mm5: (pB->tu*l_lscaleS + TEXEL_SOFFSET)*pB->rhw | pB->rhw

   pfmul   mm1, mm3 ; mm1: (ni) | ZSCALE( pC->sz )
   movq    qword ptr [hwptr1+8*6], mm5

   punpckhdq mm7, mm7
   movd    mm3, dword ptr rhw[vc1]

   punpckldq mm7, mm0 ; mm7: pC->sx + PIXEL_OFFSET | (pB->tv*l_lscaleT + TEXEL_SOFFSET)*pB->rhw
   punpckldq mm3, mm3 ; mm3: pC->rhw | pC->rhw

ifdef DCT_FIX
   pfmul   mm3, qword ptr [__asm_data + _SCALEWS+8 ]  ; multiply w scale factor with correction.
endif

   movq    qword ptr [hwptr1+8*7], mm7
   punpckhdq mm0, mm0 ; mm0: (ni) | pC->sy + PIXEL_OFFSET

   movq    mm5, qword ptr [s0 + vc1]
   punpckldq mm1, mm3 ; mm1: pC->rhw | ZSCALE( pC->sz )

   punpckldq mm0, qword ptr color[vc1]
   add     post_idx1,12

   movq    qword ptr [hwptr1+8*8], mm0
   mov     verts2,dword ptr _verts[esp]

   movq    qword ptr [hwptr1+8*9], mm1
   mov     va1,dword ptr[post_idx1]

   pfmul   mm5, mm4 ; mm5: pC->tv * lscaleT | pC->tu * lscaleS
   mov     vb1,dword ptr[post_idx1+4]

   pfadd   mm5, mm2 ; mm5: (pC->tv*lscaleT)+TEXEL_TOFFSET | (pC->tu*lscaleS)+TEXEL_SOFFSET
   mov     count1,dword ptr _count[esp]

   pfmul   mm5, mm3



IFDEF  STBPERF_K6_ASM_AUTOSTRIP
;miles porting from oem tree 8/3/99
;   int 3	

   movq    qword ptr [hwptr1+8*10], mm5

   add     hwptr1, 8*10+8
   mov     dword ptr temphwptr, hwptr1
   mov     vc1,dword ptr [post_idx1+8]

   mov     edx, [post_idx1+last2]                ;get prev.vertex2
   mov     edi, [post_idx1+last1]                ;get prev.vertex1

;       look for duplicate vertices
;       we want to make a strip!
;       compare prev.vc1 to curr.va1
;       compare prev.vb1 to curr.vb1
;       if they both match, curr triangle is part of a strip!
   sub     edx, eax                        ;tmp1 -= va1
   sub     edi, ebx                        ;tmp2 -= vb1     
        

   xor     esi, esi                        ;test1 = 0
   or      edi, edx                        ;tmp2 |= tmp1

   mov     edx,dword ptr [ebp + last0]     ;tmp1 = prev.vertex0
;       If prev.vc1 == curr.va1 && prev.vb1 == curr.vb1, we found a strip
   jnz     @f
   mov     esi,dword ptr _strip_cont[esp]  ;esi = strip header
@@:
   mov     edi,dword ptr [ebp + last2]     ;tmp2 = prev.vertex2

;       compare prev.va1 to curr.va1
;       compare prev.vc1 to curr.vb1
;       if they both match, this is part of a fan!
   sub     edx, eax                        ;tmp1 -= va1
   sub     edi, ebx                        ;tmp2 -= vb1

   or      edi, edx                        ;tmp2 |= tmp1

   mov     edx, dword ptr _verts[esp]      ;verts2 = _verts[esp]
   jnz     @f
   mov   esi, dword ptr _fan_cont[esp]   ;esi = fan header
@@:
   lea     eax, dword ptr [edx + eax*4]    ;va1 = &verts[va1]
   and     esi, PRIM_TEST

   lea     ebx, dword ptr [edx + ebx*4]    ;vb1 = &verts[vb1]

   mov     edi,dword ptr _count[esp]       ;count1 = _count[esp]

   jnz     @f
   mov     hwptr1, dword ptr temphwptr     ; hwptr1 was messed up, restore it here
   jmp     do_cycle_k6m        ;not strip or fan, go back to top of routine & loop

@@:
   mov     dword ptr _last[esp],BIT_22

align 32
prim_continue:
;       We found a strip or a fan
        dec     edi                             ;count1--
   js      done_render_k6m

        lea     ecx,dword ptr [edx + ecx*4]     ;vc1 = &verts[ecx]
        mov     dword ptr _count[esp],edi

;       First off, perform backface culling

; miles testing
;   int 3


   movq    mm0, qword ptr sx[va1]     ; mm0: pA->sy | pA->sx

   movq    mm1, qword ptr sx[vb1]   ; mm1: pB->sy | pB->sx
   pcmpeqd mm5, mm5

   movq    mm3, qword ptr sx[vc1]     ; mm2: pC->sy | pC->sx
   pfsub   mm0, mm1    ; mm0: pA->sy - pB->sy | pA->sx - pB->sx

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

   xor     area1,dword ptr [__asm_data +_CULL_MASK]
   jz      culled_tri_k6m

   mov	   eax, _last[esp]
   mov     ebx,STRIP_CONTINUE

   mov     edx,esi                         ;tmp1 = test1
   xor     esi,eax

   mov     dword ptr _last[esp],edx
   xor     esi,BIT_22

   mov     edx, dword ptr [__asm_data + _T0_OFFSET]

   and     esi,BIT_22

   or      ebx, esi                        ;header1 |= test1

   mov     hwptr1, dword ptr temphwptr     ; hwptr1 was messed up, restore it here
   movd    mm7,  ebx
   punpckldq mm7, mm1

   movd    mm5, dword ptr rhw[vc1] ; mm5: - | pC->rhw
   punpckhdq mm1, mm1

;   movq_esi_mm7 ;movq    qword ptr [hwptr1], mm7
   movq    qword ptr [hwptr1], mm7
   punpckldq mm1, qword ptr color[vc1]

   punpckldq mm5, mm5
   movq    qword ptr [hwptr1+8*1], mm1

   punpckldq mm3, mm5
   mov     s0,dword ptr[__asm_data + _T0_OFFSET]
   movq    mm7, qword ptr [s0 + vc1] ; mm3: pC->tv | pC->tu

   movq    qword ptr [hwptr1+8*2], mm3
   pfmul   mm7, mm4  ; mm3: pC->tv*l_lscaleT | pC->tu*l_lscaleS

   pfadd   mm7, mm2

   add     hwptr1, 8*3+8
   mov     dword ptr temphwptr, hwptr1
   pfmul   mm7, mm5

   movq    qword ptr [hwptr1-8], mm7  ; [hwptr1+8*3]


Clip_Continue2:
    add     ebp,12                          ;post_idx1 += 12 -- advance to next triangle
    mov     eax, [ebp]                      ;va1 = *post_idx1
    mov     ecx, [ebp+8]                    ;vc1 = *(post_idx+2)
    mov     ebx, [ebp+4]                    ;vb1 = *(post_idx+1)
    mov     edx, [ebp+last2]                ;get prev.vertex2
    mov     edi, [ebp+last1]                ;get prev.vertex1

    sub     edx, eax                        ;tmp1 -= va1
    sub     edi, ebx                        ;tmp2 -= vb1

    xor     esi, esi                        ;test1 = 0
    or      edi, edx                        ;tmp2 |= tmp1

    mov     edx, dword ptr [ebp + last0]
    jnz     @f
    mov   esi, dword ptr _strip_cont[esp]
@@:
    mov     edi, dword ptr [ebp + last2]

    sub     edx, eax
    sub     edi, ebx

    or      edi, edx
    mov     edx, dword ptr _verts[esp]
    jnz     @f
    mov   esi, dword ptr _fan_cont[esp]
@@:

    lea     eax, dword ptr [edx + eax*4]
    mov     edi, dword ptr _count[esp]

    and     esi, PRIM_TEST
    lea     ebx, dword ptr [edx + ebx*4]

    jnz     prim_continue

    mov     hwptr1, dword ptr temphwptr     ; hwptr1 was messed up, restore it here
    jmp      do_cycle_k6m


align 32
culled_tri_k6m:
   add     post_idx1,12
   mov     verts2,dword ptr _verts[esp]

   mov     va1,dword ptr [post_idx1]
   mov     vb1,dword ptr [post_idx1+4]

   mov     vc1,dword ptr [post_idx1+8]
   lea     va1,dword ptr [verts2+va1*4]

   mov     count1,dword ptr _count[esp]
   lea     vb1,dword ptr [verts2+vb1*4]

   mov     hwptr1, dword ptr temphwptr
   jmp     do_cycle_k6m
   nop

align 32
done_render_k6m:
   mov     hwptr_old,dword ptr[__asm_data + _FIFO_PTR]
   mov     hwptr1, dword ptr temphwptr
   mov     dword ptr[__asm_data + _FIFO_PTR],hwptr1

   sub     hwptr1,hwptr_old
   mov     room2,dword ptr [__asm_data + _FIFO_ROOM]

   shr     hwptr1,2
   sub     room2,hwptr1

   mov     dword ptr [__asm_data + _FIFO_ROOM],room2
   femms

   mov     eax,dword ptr _idx[esp]
   add     esp,STACK_SIZE

   pop     esi
   pop     edi

   pop     ebx
   pop     ebp

   pop     esp
   ret


ELSE
; code without auto-strip
   lea     va1,dword ptr [verts2 + va1*4]

   movq    qword ptr [hwptr1+8*10], mm5
   lea     vb1,dword ptr [verts2 + vb1*4]

   add     hwptr1, 8*10+8
   mov     vc1,dword ptr [post_idx1+8]

   jmp     do_cycle_k6m

align 32
culled_tri_k6m:
   add     post_idx1,12
   mov     verts2,dword ptr _verts[esp]

   mov     va1,dword ptr [post_idx1]
   mov     vb1,dword ptr [post_idx1+4]

   mov     vc1,dword ptr [post_idx1+8]
   lea     va1,dword ptr [verts2+va1*4]

   mov     count1,dword ptr _count[esp]
   lea     vb1,dword ptr [verts2+vb1*4]

   jmp     do_cycle_k6m
   nop

align 32
done_render_k6m:
   mov     hwptr_old,dword ptr[__asm_data + _FIFO_PTR]
   mov     dword ptr[__asm_data + _FIFO_PTR],hwptr1

   sub     hwptr1,hwptr_old
   mov     room2,dword ptr [__asm_data + _FIFO_ROOM]

   shr     hwptr1,2
   sub     room2,hwptr1

   mov     dword ptr [__asm_data + _FIFO_ROOM],room2
   femms

   mov     eax,dword ptr _idx[esp]
   add     esp,STACK_SIZE

   pop     esi
   pop     edi

   pop     ebx
   pop     ebp

   pop     esp
   ret

ENDIF




_DrawTriIdx2MainAsm_K62O   ENDP


_TEXT   ENDS


END
