; 
; ATRIKNI - Katmai version of DX6 FVF triangle routines
;
;



TITLE           atrikni.asm
                        .686P
                        .model FLAT

;ÿEXTRN    __asm_data:BYTE
EXTRN    _fifo_make_room_asm:NEAR

ifdef PERF_MONITORING
extrn __penter:proc
endif

    .data
    align   16
public  __asm_data
__asm_data dd 0, 0, 0, 0, 0, 0
       dd 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
       dd 0, 0, 0, 0
       dd 1.0, 1.0, 0.0, 0.0
       dd 0.0, 0.0, 0.0, 0.0
       dd 0.0, 0.0, 0.0, 0.0
       dd 0.0, 1.0, 0.0, 0.0
ifdef DCT_FIX
       dd 1.0, 1.0, 1.0, 1.0        ; RAJ 4/30/99 DCT-fix 
       dd 0.0, 0.0, 0.0, 0.0        ; extend kni_array to include
                                    ; scaleW factors and temp space
endif

_TEXT       SEGMENT
            .code

INCLUDE aglobal.inc
IF @Version GE 614
XMMWORD 	TEXTEQU	<DWORD>
xmmword	TEXTEQU	<DWORD>
ELSE
include iaxmm.inc
ENDIF

.686P
.MMX
IF @Version GE 614
.XMM
ENDIF

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



align   16
PUBLIC   _PrefetchTriIdx2_5_Asm_KNI
_PrefetchTriIdx2_5_Asm_KNI     PROC     NEAR

chunk_size1 TEXTEQU     <eax>
va1         TEXTEQU     <eax>
vb1         TEXTEQU     <ebx>
vc1         TEXTEQU     <ecx>
size1       TEXTEQU     <ecx>
room1       TEXTEQU     <edx>
verts2      TEXTEQU     <edx>
ret_addr1   TEXTEQU     <edx>
count1      TEXTEQU     <edi>
idx1        TEXTEQU     <esi>
hwptr1      TEXTEQU     <esi>
post_idx1   TEXTEQU     <ebp>

ifdef PERF_MONITORING
   call    __penter
endif

   push     esp
   push     ebp
   push     ebx
   push     edi
   push     esi
   sub      esp,STACK_SIZE

   mov      count1,dword ptr _count[esp]
   mov      chunk_size1,NUM_PREFETCH_TRIS

   sub      chunk_size1,count1
   jl       TriIdx2_5_too_large

TriIdx2_5_reentry:
   mov      idx1,dword ptr _idx[esp]
   lea      post_idx1,_post_idx[esp-12]

fetch_again:
   movzx    va1,word ptr [idx1]
   add      post_idx1,12
   movzx    vb1,word ptr [idx1+2]
   movzx    vc1,word ptr [idx1+4]

   lea      va1,dword ptr [va1*4 + va1]
   add      idx1,6

   lea      vb1,dword ptr [vb1*4 + vb1]
   mov      dword ptr [post_idx1],va1

   lea      vc1,dword ptr [vc1*4 + vc1]
   mov      dword ptr [post_idx1+4],vb1

   dec      count1
   mov      dword ptr [post_idx1+8],vc1

   jnz      fetch_again

   mov      dword ptr _idx[esp],idx1
   mov      count1,dword ptr _count[esp]
   lea      post_idx1,dword ptr _post_idx[esp]

   mov      room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov      size1,count1

   lea      size1,dword ptr[size1+size1*2]
   mov      va1,dword ptr [post_idx1]
   shl      size1,3h

   sub      room1,size1
   mov      vb1,dword ptr [post_idx1+4]
   js       fifo_check_TriIdx2_5

fifo_reentry_TriIdx2_5:
   mov      verts2,dword ptr _verts[esp]
   mov      hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea      va1,dword ptr[verts2+va1*4]
   mov      vc1,dword ptr[post_idx1+8]
   lea      vb1,dword ptr[verts2+vb1*4]
   jmp      _DrawTriIdx2GouradAsm_KNI

exit_early:
   add      esp,STACK_SIZE
   pop      esi
   pop      edi
   pop      ebx
   pop      ebp
   pop      esp

   ret

align   16
fifo_check_TriIdx2_5:
   push     va1
   push     vb1
   push     count1
   push     size1
   call     _fifo_make_room_asm
   pop      size1
   pop      count1
   pop      vb1
   pop      va1
   jmp      fifo_reentry_TriIdx2_5

align 16
TriIdx2_5_too_large:
   mov      ret_addr1,dword ptr _ret_addr[esp]
   mov      dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub      count1,NUM_PREFETCH_TRIS
   mov      dword ptr[__asm_data + _SAVED_COUNT],count1
   mov      dword ptr _ret_addr[esp],TriIdx2_5_continue
   mov      count1,NUM_PREFETCH_TRIS
   mov      dword ptr _count[esp],count1
   jmp      TriIdx2_5_reentry

align 16
TriIdx2_5_continue:
   mov      edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov      ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov      dword ptr _count[esp -_params],edx
   push     ecx
   jmp      _PrefetchTriIdx2_5_Asm_KNI

_PrefetchTriIdx2_5_Asm_KNI     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_6T_Asm_KNI
_PrefetchTriIdx2_6T_Asm_KNI     PROC     NEAR

chunk_size1 TEXTEQU     <eax>
va1         TEXTEQU     <eax>
vb1         TEXTEQU     <ebx>
tmp1        TEXTEQU     <ecx>
size1       TEXTEQU     <ecx>
tmp2        TEXTEQU     <edx>
room1       TEXTEQU     <edx>
verts2      TEXTEQU     <edx>
ret_addr1   TEXTEQU     <edx>
count1      TEXTEQU     <edi>
idx1        TEXTEQU     <esi>
hwptr1      TEXTEQU     <esi>
post_idx1   TEXTEQU     <ebp>


   push     esp
   push     ebp
   push     ebx
   push     edi
   push     esi
   sub      esp,STACK_SIZE

;  preload katmai registers with constants
   movaps   xmm2, XMMWORD ptr [__asm_data + _SCALE_TS11]
   movaps   xmm3, XMMWORD ptr [__asm_data + _00PIXEL_CENTER]
   movaps   xmm4, XMMWORD ptr [__asm_data + _TEXEL_OFFSET_TS00]
   movaps   xmm5, XMMWORD ptr [__asm_data + _SCALEZ1]     

   mov      count1,dword ptr _count[esp]
   mov      dword ptr _strip_cont[esp],BIT_23
   mov      chunk_size1,NUM_PREFETCH_TRIS

   sub      chunk_size1,count1
   mov      dword ptr _fan_cont[esp],BIT_22
   jl       TriIdx2_6_too_large

TriIdx2_6_reentry:
   mov      idx1,dword ptr _idx[esp]
   lea      post_idx1,_post_idx[esp-12]

fetch_again:
   movzx    va1,word ptr [idx1]
   add      post_idx1,12
   movzx    vb1,word ptr [idx1+2]
   movzx    vc1,word ptr [idx1+4]

   shl      va1,1
   add      idx1,6
   shl      vb1,1

   lea      va1,dword ptr [va1*2 + va1]
   shl      vc1,1

   lea      vb1,dword ptr [vb1*2 + vb1]
   mov      dword ptr [post_idx1],va1

   lea      vc1,dword ptr [vc1*2 + vc1]
   mov      dword ptr [post_idx1+4],vb1

   dec      count1
   mov      dword ptr [post_idx1+8],vc1

   jnz      fetch_again

   mov      dword ptr [post_idx1+12],0ffffffffh
   mov      dword ptr [post_idx1+16],0ffffffffh
   mov      dword ptr [post_idx1+20],0ffffffffh

   mov      dword ptr _idx[esp],idx1
   mov      count1,dword ptr _count[esp]
   lea      post_idx1,dword ptr _post_idx[esp]

   mov      room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov      size1,count1

   lea      size1,dword ptr[size1+size1*2]
   mov      va1,dword ptr [post_idx1]
   shl      size1,3h

   sub      room1,size1
   mov      vb1,dword ptr [post_idx1+4]
   js       fifo_check_TriIdx2_6

fifo_reentry_TriIdx2_6:
   mov      verts2,dword ptr _verts[esp]
   mov      hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea      va1,dword ptr[verts2+va1*4]
   mov      vc1,dword ptr[post_idx1+8]
   lea      vb1,dword ptr[verts2+vb1*4]
   mov      dword ptr _hwptr[esp],hwptr1
   jmp      _DrawTriIdx2MainAsm_KNI

exit_early:
   add      esp,STACK_SIZE
   pop      esi
   pop      edi
   pop      ebx
   pop      ebp
   pop      esp

   ret

align   16
fifo_check_TriIdx2_6:
   push     va1
   push     vb1
   push     count1
   push     size1
   call     _fifo_make_room_asm
   pop      size1
   pop      count1
   pop      vb1
   pop      va1
   jmp      fifo_reentry_TriIdx2_6

align 16
TriIdx2_6_too_large:
   mov      ret_addr1,dword ptr _ret_addr[esp]
   mov      dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub      count1,NUM_PREFETCH_TRIS
   mov      dword ptr[__asm_data + _SAVED_COUNT],count1
   mov      dword ptr _ret_addr[esp],TriIdx2_6_continue
   mov      count1,NUM_PREFETCH_TRIS
   mov      dword ptr _count[esp],count1
   jmp      TriIdx2_6_reentry

align 16
TriIdx2_6_continue:
   mov      edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov      ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov      dword ptr _count[esp -_params],edx
   push     ecx
   jmp      _PrefetchTriIdx2_6T_Asm_KNI

_PrefetchTriIdx2_6T_Asm_KNI     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_7_Asm_KNI
_PrefetchTriIdx2_7_Asm_KNI     PROC     NEAR

chunk_size1 TEXTEQU     <eax>
va1         TEXTEQU     <eax>
vb1         TEXTEQU     <ebx>
tmp1        TEXTEQU     <ecx>
size1       TEXTEQU     <ecx>
tmp2        TEXTEQU     <edx>
room1       TEXTEQU     <edx>
verts2      TEXTEQU     <edx>
ret_addr1   TEXTEQU     <edx>
count1      TEXTEQU     <edi>
idx1        TEXTEQU     <esi>
hwptr1      TEXTEQU     <esi>
post_idx1   TEXTEQU     <ebp>

   push     esp
   push     ebp
   push     ebx
   push     edi
   push     esi
   sub      esp,STACK_SIZE

;  preload katmai registers with constants
   movaps   xmm2, XMMWORD ptr [__asm_data + _SCALE_TS11]
   movaps   xmm3, XMMWORD ptr [__asm_data + _00PIXEL_CENTER]
   movaps   xmm4, XMMWORD ptr [__asm_data + _TEXEL_OFFSET_TS00]
   movaps   xmm5, XMMWORD ptr [__asm_data + _SCALEZ1]     

   mov      count1,dword ptr _count[esp]
   mov      dword ptr _strip_cont[esp],BIT_23
   mov      chunk_size1,NUM_PREFETCH_TRIS

   sub      chunk_size1,count1
   mov      dword ptr _fan_cont[esp],BIT_22
   jl       TriIdx2_7_too_large

TriIdx2_7_reentry:
   mov      idx1,dword ptr _idx[esp]
   lea      post_idx1,_post_idx[esp-12]

fetch_again:
   movzx    va1,word ptr [idx1]
   add      post_idx1,12
   movzx    vb1,word ptr [idx1+2]

   mov      tmp1,va1
   mov      tmp2,vb1

   shl      va1,3
   add      idx1,6
   shl      vb1,3

   sub      va1,tmp1
   movzx    tmp1,word ptr [idx1-2]
   sub      vb1,tmp2

   mov      tmp2,tmp1

   mov      dword ptr [post_idx1],va1

   shl      tmp1,3
   mov      dword ptr [post_idx1+4],vb1

   sub      tmp1,tmp2
   dec      count1

   mov      dword ptr [post_idx1+8],tmp1
   jnz      fetch_again

   mov      dword ptr [post_idx1+12],0ffffffffh
   mov      dword ptr [post_idx1+16],0ffffffffh
   mov      dword ptr [post_idx1+20],0ffffffffh

   mov      dword ptr _idx[esp],idx1
   mov      count1,dword ptr _count[esp]
   lea      post_idx1,dword ptr _post_idx[esp]

   mov      room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov      size1,count1

   lea      size1,dword ptr[size1+size1*2]
   mov      va1,dword ptr [post_idx1]
   shl      size1,3h

   sub      room1,size1
   mov      vb1,dword ptr [post_idx1+4]
   js       fifo_check_TriIdx2_7

fifo_reentry_TriIdx2_7:
   mov      verts2,dword ptr _verts[esp]
   mov      hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea      va1,dword ptr[verts2+va1*4]
   mov      vc1,dword ptr[post_idx1+8]
   lea      vb1,dword ptr[verts2+vb1*4]
   mov      dword ptr _hwptr[esp],hwptr1
   jmp      _DrawTriIdx2MainAsm_KNI

exit_early:
   add      esp,STACK_SIZE
   pop      esi
   pop      edi
   pop      ebx
   pop      ebp
   pop      esp

   ret

align   16
fifo_check_TriIdx2_7:
   push     va1
   push     vb1
   push     count1
   push     size1
   call     _fifo_make_room_asm
   pop      size1
   pop      count1
   pop      vb1
   pop      va1
   jmp      fifo_reentry_TriIdx2_7

align 16
TriIdx2_7_too_large:
   mov      ret_addr1,dword ptr _ret_addr[esp]
   mov      dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub      count1,NUM_PREFETCH_TRIS
   mov      dword ptr[__asm_data + _SAVED_COUNT],count1
   mov      dword ptr _ret_addr[esp],TriIdx2_7_continue
   mov      count1,NUM_PREFETCH_TRIS
   mov      dword ptr _count[esp],count1
   jmp      TriIdx2_7_reentry

align 16
TriIdx2_7_continue:
   mov      edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov      ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov      dword ptr _count[esp -_params],edx
   push     ecx
   jmp      _PrefetchTriIdx2_7_Asm_KNI

_PrefetchTriIdx2_7_Asm_KNI     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_8_Asm_KNI
_PrefetchTriIdx2_8_Asm_KNI     PROC     NEAR

chunk_size1 TEXTEQU     <eax>
va1         TEXTEQU     <eax>
vb1         TEXTEQU     <ebx>
tmp1        TEXTEQU     <ecx>
size1       TEXTEQU     <ecx>
tmp2        TEXTEQU     <edx>
room1       TEXTEQU     <edx>
verts2      TEXTEQU     <edx>
ret_addr1   TEXTEQU     <edx>
count1      TEXTEQU     <edi>
idx1        TEXTEQU     <esi>
hwptr1      TEXTEQU     <esi>
post_idx1   TEXTEQU     <ebp>


   push     esp
   push     ebp
   push     ebx
   push     edi
   push     esi
   sub      esp,STACK_SIZE

;  preload katmai registers with constants
   movaps   xmm2, XMMWORD ptr [__asm_data + _SCALE_TS11]
   movaps   xmm3, XMMWORD ptr [__asm_data + _00PIXEL_CENTER]
   movaps   xmm4, XMMWORD ptr [__asm_data + _TEXEL_OFFSET_TS00]
   movaps   xmm5, XMMWORD ptr [__asm_data + _SCALEZ1]     

   mov      count1,dword ptr _count[esp]
   mov      dword ptr _strip_cont[esp],BIT_23
   mov      chunk_size1,NUM_PREFETCH_TRIS

   sub      chunk_size1,count1
   mov      dword ptr _fan_cont[esp],BIT_22
   jl       TriIdx2_8_too_large

TriIdx2_8_reentry:
   mov      idx1,dword ptr _idx[esp]
   lea      post_idx1,_post_idx[esp-12]

fetch_again:
   movzx    va1,word ptr [idx1]
   add      post_idx1,12
   movzx    vb1,word ptr [idx1+2]

   shl      va1,3
   movzx    vc1,word ptr [idx1+4]
   add      idx1,6
   shl      vb1,3
   mov      dword ptr [post_idx1],va1
   shl      vc1,3
   mov      dword ptr [post_idx1+4],vb1

   dec      count1

   mov      dword ptr [post_idx1+8],vc1
   jnz      fetch_again

   mov      dword ptr [post_idx1+12],0ffffffffh
   mov      dword ptr [post_idx1+16],0ffffffffh
   mov      dword ptr [post_idx1+20],0ffffffffh

   mov      dword ptr _idx[esp],idx1
   mov      count1,dword ptr _count[esp]
   lea      post_idx1,dword ptr _post_idx[esp]

   mov      room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov      size1,count1

   lea      size1,dword ptr[size1+size1*2]
   mov      va1,dword ptr [post_idx1]
   shl      size1,3h

   sub      room1,size1
   mov      vb1,dword ptr [post_idx1+4]
   js       fifo_check_TriIdx2_8

fifo_reentry_TriIdx2_8:
   mov      verts2,dword ptr _verts[esp]
   mov      hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea      va1,dword ptr[verts2+va1*4]
   mov      vc1,dword ptr[post_idx1+8]
   lea      vb1,dword ptr[verts2+vb1*4]
   mov      dword ptr _hwptr[esp],hwptr1
   jmp      _DrawTriIdx2MainAsm_KNI

exit_early:
   add      esp,STACK_SIZE
   pop      esi
   pop      edi
   pop      ebx
   pop      ebp
   pop      esp

   ret

align   16
fifo_check_TriIdx2_8:
   push     va1
   push     vb1
   push     count1
   push     size1
   call     _fifo_make_room_asm
   pop      size1
   pop      count1
   pop      vb1
   pop      va1
   jmp      fifo_reentry_TriIdx2_8

align 16
TriIdx2_8_too_large:
   mov      ret_addr1,dword ptr _ret_addr[esp]
   mov      dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub      count1,NUM_PREFETCH_TRIS
   mov      dword ptr[__asm_data + _SAVED_COUNT],count1
   mov      dword ptr _ret_addr[esp],TriIdx2_8_continue
   mov      count1,NUM_PREFETCH_TRIS
   mov      dword ptr _count[esp],count1
   jmp      TriIdx2_8_reentry

align 16
TriIdx2_8_continue:
   mov      edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov      ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov      dword ptr _count[esp -_params],edx
   push     ecx
   jmp      _PrefetchTriIdx2_8_Asm_KNI

_PrefetchTriIdx2_8_Asm_KNI     ENDP

align 16
PUBLIC   _PrefetchTriIdx2_9_Asm_KNI
_PrefetchTriIdx2_9_Asm_KNI     PROC     NEAR

chunk_size1 TEXTEQU     <eax>
va1         TEXTEQU     <eax>
vb1         TEXTEQU     <ebx>
tmp1        TEXTEQU     <ecx>
size1       TEXTEQU     <ecx>
tmp2        TEXTEQU     <edx>
room1       TEXTEQU     <edx>
verts2      TEXTEQU     <edx>
ret_addr1   TEXTEQU     <edx>
count1      TEXTEQU     <edi>
idx1        TEXTEQU     <esi>
hwptr1      TEXTEQU     <esi>
post_idx1   TEXTEQU     <ebp>

   push     esp
   push     ebp
   push     ebx
   push     edi
   push     esi
   sub      esp,STACK_SIZE

;  preload katmai registers with constants
   movaps   xmm2, XMMWORD ptr [__asm_data + _SCALE_TS11]
   movaps   xmm3, XMMWORD ptr [__asm_data + _00PIXEL_CENTER]
   movaps   xmm4, XMMWORD ptr [__asm_data + _TEXEL_OFFSET_TS00]
   movaps   xmm5, XMMWORD ptr [__asm_data + _SCALEZ1]     

   mov      count1,dword ptr _count[esp]
   mov      dword ptr _strip_cont[esp],BIT_23
   mov      chunk_size1,NUM_PREFETCH_TRIS

   sub      chunk_size1,count1
   mov      dword ptr _fan_cont[esp],BIT_22
   jl       TriIdx2_9_too_large

TriIdx2_9_reentry:
   mov      idx1,dword ptr _idx[esp]
   lea      post_idx1,_post_idx[esp-12]

fetch_again:
   movzx    va1,word ptr [idx1]
   add      post_idx1,12
   movzx    vb1,word ptr [idx1+2]

   mov      tmp1,va1
   mov      tmp2,vb1

   shl      va1,3
   add      idx1,6
   shl      vb1,3

   add      va1,tmp1
   movzx    tmp1,word ptr [idx1-2]
   add      vb1,tmp2

   mov      tmp2,tmp1

   mov      dword ptr [post_idx1],va1

   shl      tmp1,3
   mov      dword ptr [post_idx1+4],vb1

   add      tmp1,tmp2
   dec      count1

   mov      dword ptr [post_idx1+8],tmp1
   jnz      fetch_again

   mov      dword ptr [post_idx1+12],0ffffffffh
   mov      dword ptr [post_idx1+16],0ffffffffh
   mov      dword ptr [post_idx1+20],0ffffffffh

   mov      dword ptr _idx[esp],idx1
   mov      count1,dword ptr _count[esp]
   lea      post_idx1,dword ptr _post_idx[esp]

   mov      room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov      size1,count1

   lea      size1,dword ptr[size1+size1*2]
   mov      va1,dword ptr [post_idx1]
   shl      size1,3h

   sub      room1,size1
   mov      vb1,dword ptr [post_idx1+4]
   js       fifo_check_TriIdx2_9

fifo_reentry_TriIdx2_9:
   mov      verts2,dword ptr _verts[esp]
   mov      hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea      va1,dword ptr[verts2+va1*4]
   mov      vc1,dword ptr[post_idx1+8]
   lea      vb1,dword ptr[verts2+vb1*4]
   mov      dword ptr _hwptr[esp],hwptr1
   jmp      _DrawTriIdx2MainAsm_KNI

exit_early:
   add      esp,STACK_SIZE
   pop      esi
   pop      edi
   pop      ebx
   pop      ebp
   pop      esp

   ret

align   16
fifo_check_TriIdx2_9:
   push     va1
   push     vb1
   push     count1
   push     size1
   call     _fifo_make_room_asm
   pop      size1
   pop      count1
   pop      vb1
   pop      va1
   jmp      fifo_reentry_TriIdx2_9

align 16
TriIdx2_9_too_large:
   mov      ret_addr1,dword ptr _ret_addr[esp]
   mov      dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub      count1,NUM_PREFETCH_TRIS
   mov      dword ptr[__asm_data + _SAVED_COUNT],count1
   mov      dword ptr _ret_addr[esp],TriIdx2_9_continue
   mov      count1,NUM_PREFETCH_TRIS
   mov      dword ptr _count[esp],count1
   jmp      TriIdx2_9_reentry

align 16
TriIdx2_9_continue:
   mov      edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov      ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov      dword ptr _count[esp -_params],edx
   push     ecx
   jmp      _PrefetchTriIdx2_9_Asm_KNI

_PrefetchTriIdx2_9_Asm_KNI     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_10_Asm_KNI
_PrefetchTriIdx2_10_Asm_KNI     PROC     NEAR

chunk_size1 TEXTEQU     <eax>
va1         TEXTEQU     <eax>
vb1         TEXTEQU     <ebx>
tmp1        TEXTEQU     <ecx>
size1       TEXTEQU     <ecx>
tmp2        TEXTEQU     <edx>
room1       TEXTEQU     <edx>
verts2      TEXTEQU     <edx>
ret_addr1   TEXTEQU     <edx>
count1      TEXTEQU     <edi>
idx1        TEXTEQU     <esi>
hwptr1      TEXTEQU     <esi>
post_idx1   TEXTEQU     <ebp>


   push     esp
   push     ebp
   push     ebx
   push     edi
   push     esi
   sub      esp,STACK_SIZE

;  preload katmai registers with constants
   movaps   xmm2, XMMWORD ptr [__asm_data + _SCALE_TS11]
   movaps   xmm3, XMMWORD ptr [__asm_data + _00PIXEL_CENTER]
   movaps   xmm4, XMMWORD ptr [__asm_data + _TEXEL_OFFSET_TS00]
   movaps   xmm5, XMMWORD ptr [__asm_data + _SCALEZ1]     

   mov      count1,dword ptr _count[esp]
   mov      dword ptr _strip_cont[esp],BIT_23
   mov      chunk_size1,NUM_PREFETCH_TRIS

   sub      chunk_size1,count1
   mov      dword ptr _fan_cont[esp],BIT_22
   jl       TriIdx2_10_too_large

TriIdx2_10_reentry:
   mov      idx1,dword ptr _idx[esp]
   lea      post_idx1,_post_idx[esp-12]

fetch_again:
   movzx    va1,word ptr [idx1]
   add      post_idx1,12
   movzx    vb1,word ptr [idx1+2]
   movzx    vc1,word ptr [idx1+4]

   shl      va1,1
   add      idx1,6
   shl      vb1,1

   lea      va1,dword ptr [va1*4 + va1]
   shl      vc1,1

   lea      vb1,dword ptr [vb1*4 + vb1]
   mov      dword ptr [post_idx1],va1

   lea      vc1,dword ptr [vc1*4 + vc1]
   mov      dword ptr [post_idx1+4],vb1

   dec      count1
   mov      dword ptr [post_idx1+8],vc1

   jnz      fetch_again

   mov      dword ptr [post_idx1+12],0ffffffffh
   mov      dword ptr [post_idx1+16],0ffffffffh
   mov      dword ptr [post_idx1+20],0ffffffffh

   mov      dword ptr _idx[esp],idx1
   mov      count1,dword ptr _count[esp]
   lea      post_idx1,dword ptr _post_idx[esp]

   mov      room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov      size1,count1

   lea      size1,dword ptr[size1+size1*2]
   mov      va1,dword ptr [post_idx1]
   shl      size1,3h

   sub      room1,size1
   mov      vb1,dword ptr [post_idx1+4]
   js       fifo_check_TriIdx2_10

fifo_reentry_TriIdx2_10:
   mov      verts2,dword ptr _verts[esp]
   mov      hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea      va1,dword ptr[verts2+va1*4]
   mov      vc1,dword ptr[post_idx1+8]
   lea      vb1,dword ptr[verts2+vb1*4]
   mov      dword ptr _hwptr[esp],hwptr1
   jmp      _DrawTriIdx2MainAsm_KNI

exit_early:
   add      esp,STACK_SIZE
   pop      esi
   pop      edi
   pop      ebx
   pop      ebp
   pop      esp

   ret

align   16
fifo_check_TriIdx2_10:
   push     va1
   push     vb1
   push     count1
   push     size1
   call     _fifo_make_room_asm
   pop      size1
   pop      count1
   pop      vb1
   pop      va1
   jmp      fifo_reentry_TriIdx2_10

align 16
TriIdx2_10_too_large:
   mov      ret_addr1,dword ptr _ret_addr[esp]
   mov      dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub      count1,NUM_PREFETCH_TRIS
   mov      dword ptr[__asm_data + _SAVED_COUNT],count1
   mov      dword ptr _ret_addr[esp],TriIdx2_10_continue
   mov      count1,NUM_PREFETCH_TRIS
   mov      dword ptr _count[esp],count1
   jmp      TriIdx2_10_reentry

align 16
TriIdx2_10_continue:
   mov      edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov      ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov      dword ptr _count[esp -_params],edx
   push     ecx
   jmp      _PrefetchTriIdx2_10_Asm_KNI

_PrefetchTriIdx2_10_Asm_KNI     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_11_Asm_KNI
_PrefetchTriIdx2_11_Asm_KNI    PROC     NEAR

chunk_size1 TEXTEQU     <eax>
va1         TEXTEQU     <eax>
vb1         TEXTEQU     <ebx>
tmp1        TEXTEQU     <ecx>
size1       TEXTEQU     <ecx>
tmp2        TEXTEQU     <edx>
room1       TEXTEQU     <edx>
verts2      TEXTEQU     <edx>
ret_addr1   TEXTEQU     <edx>
count1      TEXTEQU     <edi>
idx1        TEXTEQU     <esi>
hwptr1      TEXTEQU     <esi>
post_idx1   TEXTEQU     <ebp>

   push     esp
   push     ebp
   push     ebx
   push     edi
   push     esi
   sub      esp,STACK_SIZE

;  preload katmai registers with constants
   movaps   xmm2, XMMWORD ptr [__asm_data + _SCALE_TS11]
   movaps   xmm3, XMMWORD ptr [__asm_data + _00PIXEL_CENTER]
   movaps   xmm4, XMMWORD ptr [__asm_data + _TEXEL_OFFSET_TS00]
   movaps   xmm5, XMMWORD ptr [__asm_data + _SCALEZ1]     

   mov      count1,dword ptr _count[esp]
   mov      dword ptr _strip_cont[esp],BIT_23
   mov      chunk_size1,NUM_PREFETCH_TRIS

   sub      chunk_size1,count1
   mov      dword ptr _fan_cont[esp],BIT_22
   jl       TriIdx2_11_too_large

TriIdx2_11_reentry:
   mov      idx1,dword ptr _idx[esp]
   lea      post_idx1,_post_idx[esp-12]

fetch_again:
   movzx    va1,word ptr [idx1]
   add      post_idx1,12
   movzx    vb1,word ptr [idx1+2]

   mov      tmp1,va1
   shl      va1,1

   mov      tmp2,vb1
   shl      vb1,1

   lea      va1,dword ptr [va1*4 + va1]
   add      idx1,6
   
   lea      vb1,dword ptr [vb1*4 + vb1]
   add      va1,tmp1
   movzx    vc1,word ptr [idx1-2]

   add      vb1,tmp2
   mov      tmp2,vc1
   shl      vc1,1

   mov      dword ptr [post_idx1],va1
   lea      vc1,dword ptr [vc1*4 + vc1]

   mov      dword ptr [post_idx1+4],vb1
   add      vc1,tmp2

   dec      count1
   mov      dword ptr [post_idx1+8],vc1

   jnz      fetch_again

   mov      dword ptr [post_idx1+12],0ffffffffh
   mov      dword ptr [post_idx1+16],0ffffffffh
   mov      dword ptr [post_idx1+20],0ffffffffh

   mov      dword ptr _idx[esp],idx1
   mov      count1,dword ptr _count[esp]
   lea      post_idx1,dword ptr _post_idx[esp]

   mov      room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov      size1,count1

   lea      size1,dword ptr[size1+size1*2]
   mov      va1,dword ptr [post_idx1]
   shl      size1,3h

   sub      room1,size1
   mov      vb1,dword ptr [post_idx1+4]
   js       fifo_check_TriIdx2_11

fifo_reentry_TriIdx2_11:
   mov      verts2,dword ptr _verts[esp]
   mov      hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea      va1,dword ptr[verts2+va1*4]
   mov      vc1,dword ptr[post_idx1+8]
   lea      vb1,dword ptr[verts2+vb1*4]
   mov      dword ptr _hwptr[esp],hwptr1
   jmp      _DrawTriIdx2MainAsm_KNI

exit_early:
   add      esp,STACK_SIZE
   pop      esi
   pop      edi
   pop      ebx
   pop      ebp
   pop      esp

   ret

align   16
fifo_check_TriIdx2_11:
   push     va1
   push     vb1
   push     count1
   push     size1
   call     _fifo_make_room_asm
   pop      size1
   pop      count1
   pop      vb1
   pop      va1
   jmp      fifo_reentry_TriIdx2_11

align 16
TriIdx2_11_too_large:
   mov      ret_addr1,dword ptr _ret_addr[esp]
   mov      dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub      count1,NUM_PREFETCH_TRIS
   mov      dword ptr[__asm_data + _SAVED_COUNT],count1
   mov      dword ptr _ret_addr[esp],TriIdx2_11_continue
   mov      count1,NUM_PREFETCH_TRIS
   mov      dword ptr _count[esp],count1
   jmp      TriIdx2_11_reentry

align 16
TriIdx2_11_continue:
   mov      edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov      ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov      dword ptr _count[esp -_params],edx
   push     ecx
   jmp      _PrefetchTriIdx2_11_Asm_KNI

_PrefetchTriIdx2_11_Asm_KNI     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_12_Asm_KNI
_PrefetchTriIdx2_12_Asm_KNI     PROC     NEAR

chunk_size1 TEXTEQU     <eax>
va1         TEXTEQU     <eax>
vb1         TEXTEQU     <ebx>
tmp1        TEXTEQU     <ecx>
size1       TEXTEQU     <ecx>
tmp2        TEXTEQU     <edx>
room1       TEXTEQU     <edx>
verts2      TEXTEQU     <edx>
ret_addr1   TEXTEQU     <edx>
count1      TEXTEQU     <edi>
idx1        TEXTEQU     <esi>
hwptr1      TEXTEQU     <esi>
post_idx1   TEXTEQU     <ebp>


   push     esp
   push     ebp
   push     ebx
   push     edi
   push     esi
   sub      esp,STACK_SIZE

;  preload katmai registers with constants
   movaps   xmm2, XMMWORD ptr [__asm_data + _SCALE_TS11]
   movaps   xmm3, XMMWORD ptr [__asm_data + _00PIXEL_CENTER]
   movaps   xmm4, XMMWORD ptr [__asm_data + _TEXEL_OFFSET_TS00]
   movaps   xmm5, XMMWORD ptr [__asm_data + _SCALEZ1]     

   mov      count1,dword ptr _count[esp]
   mov      dword ptr _strip_cont[esp],BIT_23
   mov      chunk_size1,NUM_PREFETCH_TRIS

   sub      chunk_size1,count1
   mov      dword ptr _fan_cont[esp],BIT_22
   jl       TriIdx2_12_too_large

TriIdx2_12_reentry:
   mov      idx1,dword ptr _idx[esp]
   lea      post_idx1,_post_idx[esp-12]

fetch_again:
   movzx    va1,word ptr [idx1]
   add      post_idx1,12
   movzx    vb1,word ptr [idx1+2]
   movzx    vc1,word ptr [idx1+4]

   shl      va1,2
   add      idx1,6
   shl      vb1,2

   lea      va1,dword ptr [va1*2 + va1]
   shl      vc1,2

   lea      vb1,dword ptr [vb1*2 + vb1]
   mov      dword ptr [post_idx1],va1

   lea      vc1,dword ptr [vc1*2 + vc1]
   mov      dword ptr [post_idx1+4],vb1

   dec      count1
   mov      dword ptr [post_idx1+8],vc1

   jnz      fetch_again

   mov      dword ptr [post_idx1+12],0ffffffffh
   mov      dword ptr [post_idx1+16],0ffffffffh
   mov      dword ptr [post_idx1+20],0ffffffffh

   mov      dword ptr _idx[esp],idx1
   mov      count1,dword ptr _count[esp]
   lea      post_idx1,dword ptr _post_idx[esp]

   mov      room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov      size1,count1

   lea      size1,dword ptr[size1+size1*2]
   mov      va1,dword ptr [post_idx1]
   shl      size1,3h

   sub      room1,size1
   mov      vb1,dword ptr [post_idx1+4]
   js       fifo_check_TriIdx2_12

fifo_reentry_TriIdx2_12:
   mov      verts2,dword ptr _verts[esp]
   mov      hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea      va1,dword ptr[verts2+va1*4]
   mov      vc1,dword ptr[post_idx1+8]
   lea      vb1,dword ptr[verts2+vb1*4]
   mov      dword ptr _hwptr[esp],hwptr1
   jmp      _DrawTriIdx2MainAsm_KNI

exit_early:
   add      esp,STACK_SIZE
   pop      esi
   pop      edi
   pop      ebx
   pop      ebp
   pop      esp

   ret

align   16
fifo_check_TriIdx2_12:
   push     va1
   push     vb1
   push     count1
   push     size1
   call     _fifo_make_room_asm
   pop      size1
   pop      count1
   pop      vb1
   pop      va1
   jmp      fifo_reentry_TriIdx2_12

align 16
TriIdx2_12_too_large:
   mov      ret_addr1,dword ptr _ret_addr[esp]
   mov      dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub      count1,NUM_PREFETCH_TRIS
   mov      dword ptr[__asm_data + _SAVED_COUNT],count1
   mov      dword ptr _ret_addr[esp],TriIdx2_12_continue
   mov      count1,NUM_PREFETCH_TRIS
   mov      dword ptr _count[esp],count1
   jmp      TriIdx2_12_reentry

align 16
TriIdx2_12_continue:
   mov      edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov      ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov      dword ptr _count[esp -_params],edx
   push     ecx
   jmp      _PrefetchTriIdx2_12_Asm_KNI

_PrefetchTriIdx2_12_Asm_KNI     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_6G_Asm_KNI
_PrefetchTriIdx2_6G_Asm_KNI     PROC     NEAR

chunk_size1 TEXTEQU     <eax>
va1         TEXTEQU     <eax>
vb1         TEXTEQU     <ebx>
tmp1        TEXTEQU     <ecx>
size1       TEXTEQU     <ecx>
tmp2        TEXTEQU     <edx>
room1       TEXTEQU     <edx>
verts2      TEXTEQU     <edx>
ret_addr1   TEXTEQU     <edx>
count1      TEXTEQU     <edi>
idx1        TEXTEQU     <esi>
hwptr1      TEXTEQU     <esi>
post_idx1   TEXTEQU     <ebp>


   push     esp
   push     ebp
   push     ebx
   push     edi
   push     esi
   sub      esp,STACK_SIZE

   mov      count1,dword ptr _count[esp]
   mov      dword ptr _strip_cont[esp],BIT_23
   mov      chunk_size1,NUM_PREFETCH_TRIS

   sub      chunk_size1,count1
   mov      dword ptr _fan_cont[esp],BIT_22
   jl       TriIdx2_6_too_large

TriIdx2_6_reentry:
   mov      idx1,dword ptr _idx[esp]
   lea      post_idx1,_post_idx[esp-12]

fetch_again:
   movzx    va1,word ptr [idx1]
   add      post_idx1,12
   movzx    vb1,word ptr [idx1+2]
   movzx    vc1,word ptr [idx1+4]

   shl      va1,1
   add      idx1,6
   shl      vb1,1

   lea      va1,dword ptr [va1*2 + va1]
   shl      vc1,1

   lea      vb1,dword ptr [vb1*2 + vb1]
   mov      dword ptr [post_idx1],va1

   lea      vc1,dword ptr [vc1*2 + vc1]
   mov      dword ptr [post_idx1+4],vb1

   dec      count1
   mov      dword ptr [post_idx1+8],vc1

   jnz      fetch_again

   mov      dword ptr [post_idx1+12],0ffffffffh
   mov      dword ptr [post_idx1+16],0ffffffffh
   mov      dword ptr [post_idx1+20],0ffffffffh

   mov      dword ptr _idx[esp],idx1
   mov      count1,dword ptr _count[esp]
   lea      post_idx1,dword ptr _post_idx[esp]

   mov      room1,dword ptr [__asm_data + _FIFO_ROOM]
   mov      size1,count1

   lea      size1,dword ptr[size1+size1*2]
   mov      va1,dword ptr [post_idx1]
   shl      size1,3h

   sub      room1,size1
   mov      vb1,dword ptr [post_idx1+4]
   js       fifo_check_TriIdx2_6

fifo_reentry_TriIdx2_6:
   mov      verts2,dword ptr _verts[esp]
   mov      hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea      va1,dword ptr[verts2+va1*4]
   mov      vc1,dword ptr[post_idx1+8]
   lea      vb1,dword ptr[verts2+vb1*4]
   mov      dword ptr _hwptr[esp],hwptr1
   jmp      _DrawTriIdx2GouradAsm_KNI

exit_early:
   add      esp,STACK_SIZE
   pop      esi
   pop      edi
   pop      ebx
   pop      ebp
   pop      esp

   ret

align   16
fifo_check_TriIdx2_6:
   push     va1
   push     vb1
   push     count1
   push     size1
   call     _fifo_make_room_asm
   pop      size1
   pop      count1
   pop      vb1
   pop      va1
   jmp      fifo_reentry_TriIdx2_6

align 16
TriIdx2_6_too_large:
   mov      ret_addr1,dword ptr _ret_addr[esp]
   mov      dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub      count1,NUM_PREFETCH_TRIS
   mov      dword ptr[__asm_data + _SAVED_COUNT],count1
   mov      dword ptr _ret_addr[esp],TriIdx2_6_continue
   mov      count1,NUM_PREFETCH_TRIS
   mov      dword ptr _count[esp],count1
   jmp      TriIdx2_6_reentry

align 16
TriIdx2_6_continue:
   mov      edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov      ecx,dword ptr[__asm_data + _SAVED_RET_ADDR]
   mov      dword ptr _count[esp -_params],edx
   push     ecx
   jmp      _PrefetchTriIdx2_6G_Asm_KNI

_PrefetchTriIdx2_6G_Asm_KNI     ENDP



; Core rendering routine
; supports x,y,z,rhw,color,s,t
; requires rhw
; color,s,t are unnecessary but sent to hardware

;       On entry to this routine:
;       eax = pointer to vertex A (va1)
;       ebx = pointer to vertex B (vb1)
;       ecx = index to vertex C
;       edi = count1 , count of triangles to process

;       xmm2 = __asm_data + _SCALE_TS11
;       xmm3 = __asm_data + _00PIXEL_CENTER
;       xmm4 = __asm_data + _TEXEL_OFFSET_TS00
;       xmm5 = __asm_data + _SCALEZ1

;IFDEF AGP_CMDFIFO
if 1
align 16
PUBLIC   _DrawTriIdx2MainAsm_KNI
_DrawTriIdx2MainAsm_KNI   PROC     NEAR

va1         TEXTEQU  <eax>
color1      TEXTEQU  <eax>

vb1         TEXTEQU  <ebx>
cull1       TEXTEQU  <ebx>
header1     TEXTEQU  <ebx>

vc1         TEXTEQU  <ecx>
tmp3        TEXTEQU  <ecx>

verts2      TEXTEQU  <edx>
s0          TEXTEQU  <edx>
color2      TEXTEQU  <edx>
room2       TEXTEQU  <edx>
tmp1        TEXTEQU  <edx>

count1      TEXTEQU  <edi>
area1       TEXTEQU  <edi>
rhw1        TEXTEQU  <edi>
tmp2        TEXTEQU  <edi>

hwptr1      TEXTEQU  <esi>
test1       TEXTEQU  <esi>

post_idx1   TEXTEQU  <ebp>
hwptr_old   TEXTEQU  <ebp>

        dec     edi                             ; count1--
        lea     ecx, dword ptr [edx + ecx * 4]  ;vc1 = &verts2[vc1]
        mov     edx, dword ptr [__asm_data + _T0_OFFSET]

        js      done_render


;       First off, perform backface culling

        movhps  xmm6, [eax]
        movlps  xmm6, [ebx]
        movhps  xmm7, [ebx]
        movlps  xmm7, [ecx]
    mov esi, dword ptr [__asm_data +_CULL_MASK]
;               xmm6 = va.y va.x vb.y vb.x
;               xmm7 = vb.y vb.x vc.y vc.x

        subps   xmm6, xmm7
;               xmm6 = va.y - vb.y va.x - vb.x vb.y - vc.y vb.x - vc.x 
;               xmm6 = aby abx bcy bcx
        movaps  xmm7, xmm6
        shufps  xmm6, xmm6, 0bh
;               xmm6 = xxx xxx abx  aby
;               xmm7 = xxx xxx bcy  bcx
        mulps   xmm6, xmm7
;               xmm6 = xxx xxx abx*bcy aby*bcx
        mov     dword ptr _count[esp],edi
        movaps  xmm7, xmm6
        shufps  xmm7, xmm7, 01h
;               xmm6 = xxx xxx abx*bcy aby*bcx
;               xmm7 = xxx xxx xxxxxx  abx*bcy
        subss   xmm7, xmm6
;           xmm7 = abx*bcy-aby*bcx

        
;       Save area
;        movss   _area[esp],xmm7
        movmskps edi, xmm7


;       Process vertex A
ifdef DCT_FIX
        movaps  xmm6, XMMWORD ptr [__asm_data + _SCALEWS]     ;RAJ 4/30/99 DCT-fix
endif
        movhps  xmm1, [eax+edx]             ;get s & t
;               xmm1 = t,s, xxxx, xxxx
        shl edi, 31
;        mov     edi,dword ptr _area[esp]
        movlps  xmm0, [eax+sx]              ;get x & y
;               xmm0 = 0, 0, y, x
        mulps   xmm1, xmm2
;               xmm1 = t*_SCALE_T,s*_SCALE_S, xxxx, xxxx
        and     edi,80000000h
        addps   xmm0, xmm3
        xor     edi, esi
;               xmm0 = 0, 0, y+_PIXEL_CENTER, x+_PIXEL_CENTER
        jz      culled_tri
        movlps  xmm1, [eax+sz]              ;get z & w
;               xmm1 = t*_SCALE_T, s*_SCALE_S, w, z
        mov     esi, dword ptr _hwptr[esp]
        shufps  xmm5,xmm1,54h
ifdef DCT_FIX
        mulps   xmm5, xmm6
endif

        mov     dword ptr [esi],FAN_HEADER
        mov     edi, [eax+color]
;               xmm5 = w, w, 1.0, _SCALEZ
        addps   xmm1, xmm4
;               xmm1 = t*_SCALE_T+_TEXEL_OFFSET_T, s*_SCALE_S+_TEXEL_OFFSET_S, w, z
        movlps  [esi+4], xmm0

        mov eax, [ebp+12]
        mov edx, _verts[esp]
        lea eax, [edx+eax*4]
        prefetchnta [eax]
        mov     edx, dword ptr [__asm_data + _T0_OFFSET]

        mulps   xmm1, xmm5
;       xmm1 = ;xmm1 = (t*_SCALE_T+_TEXEL_OFFSET_T)*w, (s*_SCALE_S+_TEXEL_OFFSET_S)*w, w, z*_SCALEZ
        mov     [esi+12], edi
        movlps  [esi+16], xmm1
        movhps  [esi+24], xmm1
ifdef DCT_FIX
        movaps  xmm5, XMMWORD ptr [__asm_data + _SCALEZ1]     
endif
        add     esi, 32

;       Process vertex B
        movhps  xmm1, [ebx+edx]             ;get s & t
        movlps  xmm0, [ebx+sx]              ;get x & y
        mulps   xmm1, xmm2
        addps   xmm0, xmm3
        movlps  xmm1, [ebx+sz]              ;get z & w
        shufps  xmm5,xmm1,54h
ifdef DCT_FIX
        mulps   xmm5, xmm6
endif
        mov edi, [ebx+color]
        addps   xmm1, xmm4
        movlps  [esi], xmm0
        mulps   xmm1, xmm5

        mov ebx, [ebp+16]
        mov edx, _verts[esp]
        lea ebx, [edx+ebx*4]
        prefetchnta [ebx]
        mov     edx, dword ptr [__asm_data + _T0_OFFSET]

        mov [esi+8], edi
        movlps  [esi+12], xmm1
        movhps  [esi+20], xmm1
ifdef DCT_FIX
        movaps  xmm5, XMMWORD ptr [__asm_data + _SCALEZ1]     
endif
        add esi, 28


;       Process vertex C
        movhps  xmm1, [ecx+edx]             ;get s & t
        add     ebp,12                          ;post_idx1 += 12 -- advance to next triangle
        movlps  xmm0, [ecx+sx]              ;get x & y
        mulps   xmm1, xmm2
        mov     eax, [ebp]                      ;va1 = *post_idx1
        addps   xmm0, xmm3
        mov     ebx, [ebp+4]                    ;vb1 = *(post_idx+1)
        movlps  xmm1, [ecx+sz]              ;get z & w
        mov edi, [ecx+color]
        shufps  xmm5,xmm1,54h
ifdef DCT_FIX
        mulps   xmm5, xmm6
endif

        mov ecx, [ebp+8]
        mov edx, _verts[esp]
        lea ecx, [edx+ecx*4]
        prefetchnta [ecx]
        mov     edx, dword ptr [__asm_data + _T0_OFFSET]

        mov     ecx, [ebp+8]                    ;vc1 = *(post_idx+2)
        addps   xmm1, xmm4
        movlps  [esi], xmm0
        mulps   xmm1, xmm5
        mov     edx, [ebp+last2]                ;get prev.vertex2
        mov [esi+8], edi
        movlps  [esi+12], xmm1
        movhps  [esi+20], xmm1
ifdef DCT_FIX
        movaps  xmm5, XMMWORD ptr [__asm_data + _SCALEZ1]     
endif
        add     esi, 28
        mov     edi, [ebp+last1]                ;get prev.vertex1

        mov     _hwptr[esp], esi                ;save hwptr1


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
        cmovz   esi,dword ptr _strip_cont[esp]  ;esi = strip header

        mov     edi,dword ptr [ebp + last2]     ;tmp2 = prev.vertex2

;       compare prev.va1 to curr.va1
;       compare prev.vc1 to curr.vb1
;       if they both match, this is part of a fan!
        sub     edx, eax                        ;tmp1 -= va1
        sub     edi, ebx                        ;tmp2 -= vb1

        or      edi, edx                        ;tmp2 |= tmp1

        mov     edx, dword ptr _verts[esp]      ;verts2 = _verts[esp]
        cmovz   esi, dword ptr _fan_cont[esp]   ;esi = fan header

        lea     eax, dword ptr [edx + eax*4]    ;va1 = &verts[va1]
        and     esi, PRIM_TEST

        lea     ebx, dword ptr [edx + ebx*4]    ;vb1 = &verts[vb1]

        mov     edi,dword ptr _count[esp]       ;count1 = _count[esp]

        jz      _DrawTriIdx2MainAsm_KNI         ;not strip or fan, go back to top of routine & loop

        mov     dword ptr _last[esp],BIT_22

prim_continue:
;       We found a strip or a fan
        dec     edi                             ;count1--

        lea     ecx,dword ptr [edx + ecx*4]     ;vc1 = &verts[ecx]
        mov     dword ptr _count[esp],edi

;       First off, perform backface culling

        movhps  xmm6, [eax]
        movhps  xmm7, [ebx]
        movlps  xmm6, [ebx]
        movlps  xmm7, [ecx]
;               xmm6 = va.y va.x vb.y vb.x
;               xmm7 = vb.y vb.x vc.y vc.x

        subps   xmm6, xmm7
;               xmm6 = va.y - vb.y va.x - vb.x vb.y - vc.y vb.x - vc.x 
;               xmm6 = aby abx bcy bcx
        movaps  xmm7, xmm6
        shufps  xmm6, xmm6, 0bh
;               xmm6 = xxx xxx abx  aby
;               xmm7 = xxx xxx bcy  bcx
        mulps   xmm6, xmm7
;               xmm6 = xxx xxx abx*bcy aby*bcx

        mov edi, [ebp+12]
        mov edx, _verts[esp]
        lea edi, [edx+edi*4]
        prefetchnta [edi]

        movaps  xmm7, xmm6
        shufps  xmm7, xmm7, 01h
;               xmm6 = xxx xxx abx*bcy aby*bcx
;               xmm7 = xxx xxx xxxxxx  abx*bcy
        subss   xmm7, xmm6
;           xmm7 = abx*bcy-aby*bcx

        
;       Save area
;        movss   _area[esp],xmm7
        movmskps edi, xmm7
;        mov     edi,dword ptr _area[esp]
        shl edi, 31
        and     edi,80000000h
        xor     edi, dword ptr [__asm_data +_CULL_MASK]
        jz      culled_tri


        mov eax, _last[esp]
        mov     ebx,STRIP_CONTINUE

        mov     edx,esi                         ;tmp1 = test1
        xor     esi,eax

        mov     dword ptr _last[esp],edx
        xor     esi,BIT_22

        mov eax, [ebp+20]
        mov edx, _verts[esp]
        lea eax, [edx+eax*4]
        prefetchnta [eax]
        mov     edx, dword ptr [__asm_data + _T0_OFFSET]

        and     esi,BIT_22


ifdef DCT_FIX
        movaps  xmm6, XMMWORD ptr [__asm_data + _SCALEWS]     
endif
        movhps  xmm1, [ecx+edx]             ;get s & t
        or      ebx, esi                        ;header1 |= test1
        add     ebp,12                          ;post_idx1 += 12 -- advance to next triangle
        movlps  xmm0, [ecx+sx]              ;get x & y
        mov     esi, _hwptr[esp]                ;hwptr1 = _hwptr[esp]
        mulps   xmm1, xmm2
        mov     eax, [ebp]                      ;va1 = *post_idx1
        addps   xmm0, xmm3
        movlps  xmm1, [ecx+sz]              ;get z & w
        mov edi, [ecx+color]
        shufps  xmm5,xmm1,54h
ifdef DCT_FIX
        mulps   xmm5, xmm6
endif

        mov ecx, [ebp]
        mov edx, _verts[esp]
        lea ecx, [edx+ecx*4]
        prefetchnta [ecx]


        mov     ecx, [ebp+8]                    ;vc1 = *(post_idx+2)
        addps   xmm1, xmm4
        mov     [esi], ebx                      ;*hwptr = header1
        movlps  [esi+4], xmm0
        mulps   xmm1, xmm5
        mov     ebx, [ebp+4]                    ;vb1 = *(post_idx+1)
        mov     edx, [ebp+last2]                ;get prev.vertex2
        mov [esi+12], edi
        movlps  [esi+16], xmm1
        movhps  [esi+24], xmm1
ifdef DCT_FIX
        movaps  xmm5, XMMWORD ptr [__asm_data + _SCALEZ1]     
endif
        mov     edi, [ebp+last1]                ;get prev.vertex1

        add     esi, 32                         ;hwptr1 += 32
        sub     edx, eax                        ;tmp1 -= va1

        mov     dword ptr _hwptr[esp],esi
        sub     edi, ebx                        ;tmp2 -= vb1

        xor     esi, esi                        ;test1 = 0
        or      edi, edx                        ;tmp2 |= tmp1

        mov     edx, dword ptr [ebp + last0]
        cmovz   esi, dword ptr _strip_cont[esp]

        mov     edi, dword ptr [ebp + last2]

        sub     edx, eax
        sub     edi, ebx

        or      edi, edx
        mov     edx, dword ptr _verts[esp]
        cmovz   esi, dword ptr _fan_cont[esp]

        lea     eax, dword ptr [edx + eax*4]
        mov     edi, dword ptr _count[esp]

        and     esi, PRIM_TEST
        lea     ebx, dword ptr [edx + ebx*4]

        jz      _DrawTriIdx2MainAsm_KNI

        jmp     prim_continue
culled_tri:
        add     ebp, 12
        mov     edx, dword ptr _verts[esp]
        mov     eax, dword ptr [ebp]
        mov     ebx, dword ptr [ebp+4]
        mov     ecx, dword ptr [ebp+8]

        lea     eax, dword ptr [edx+eax*4]
        mov     count1, dword ptr _count[esp]

        lea     ebx, dword ptr [edx+ebx*4]

        jmp     _DrawTriIdx2MainAsm_KNI

done_render:
        mov     esi, dword ptr _hwptr[esp]
        mov     ebp, dword ptr[__asm_data + _FIFO_PTR]
        mov     dword ptr[__asm_data + _FIFO_PTR], esi

        sub     esi, ebp
        mov     edx, dword ptr [__asm_data + _FIFO_ROOM]

        shr     esi, 2
        sub     edx, esi

        mov     dword ptr [__asm_data + _FIFO_ROOM], edx

        mov     eax,dword ptr _idx[esp]

        add     esp,STACK_SIZE
        pop     esi
        pop     edi
        pop     ebx
        pop     ebp
        pop     esp

        ret

_DrawTriIdx2MainAsm_KNI   ENDP

ENDIF

align 16
PUBLIC   _DrawTriIdx2GouradAsm_KNI
_DrawTriIdx2GouradAsm_KNI   PROC     NEAR

va1         TEXTEQU     <eax>
vb1         TEXTEQU     <ebx>
vc1         TEXTEQU     <ecx>
verts2      TEXTEQU     <edx>
color1      TEXTEQU     <edx>
room2       TEXTEQU     <edx>
count1      TEXTEQU     <edi>
area1       TEXTEQU     <edi>
rhw1        TEXTEQU     <edi>
hwptr1      TEXTEQU     <esi>
post_idx1   TEXTEQU     <ebp>
hwptr_old   TEXTEQU     <ebp>

   dec      count1
   lea      vc1,dword ptr [verts2 + vc1*4]

   js       done_render

   fld      dword ptr sx[va1]
   fsub     dword ptr sx[vb1]
   ;  abx
   fld      dword ptr sx[vb1]
   fsub     dword ptr sx[vc1]
   ;  bcx      abx
   fld      dword ptr sy[va1]
   fsub     dword ptr sy[vb1]
   ;  aby      bcx      abx
   fld      dword ptr sy[vb1]
   fsub     dword ptr sy[vc1]
   ;  bcy      aby      bcx      abx
   fxch     st(1)
   ;  aby      bcy      bcx      abx
   fmulp    st(2),st
   ;  bcy      ar2      abx
   mov      dword ptr _count[esp],count1
   fmulp    st(2),st
   ;  ar2      ar1
   fld      dword ptr sz[va1]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    ar2      ar1
   fxch     st(2)
   ;  ar1      ar2      szadj
   fsubrp   st(1),st
   ;  area     szadj
   mov      color1,dword ptr color[va1]
   mov      area1,80000000h
   fstp     dword ptr _area[esp]
   ;  szadj
   fld      dword ptr sy[va1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj
   and      area1,dword ptr _area[esp]

   fld      dword ptr sx[va1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj

   xor      area1,dword ptr [__asm_data +_CULL_MASK]

   jz       culled_tri

   mov      rhw1,dword ptr rhw[va1]

   mov      dword ptr [hwptr1],FAN_GOURAD_HEADER
   fstp     dword ptr [hwptr1+4]
   fstp     dword ptr [hwptr1+8]
   mov      dword ptr [hwptr1+12],color1
   fstp     dword ptr [hwptr1+16]
   mov      color1,dword ptr color[vb1]
   mov      dword ptr [hwptr1+20],rhw1

   fld      dword ptr sz[vb1]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj
   fld      dword ptr sy[vb1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj
   fld      dword ptr sx[vb1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj

   fstp     dword ptr [hwptr1+24]
   mov      rhw1,dword ptr rhw[vb1]
   fstp     dword ptr [hwptr1+28]
   add      post_idx1,12
   mov      dword ptr [hwptr1+32],color1
   fstp     dword ptr [hwptr1+36]
   mov      color1,dword ptr color[vc1]
   mov      dword ptr [hwptr1+40],rhw1

   fld      dword ptr sz[vc1]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj
   fld      dword ptr sy[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj
   fld      dword ptr sx[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj

   fstp     dword ptr [hwptr1+44]
   mov      rhw1,dword ptr rhw[vc1]
   fstp     dword ptr [hwptr1+48]
   mov      va1,dword ptr[post_idx1]
   mov      dword ptr [hwptr1+52],color1
   mov      verts2,dword ptr _verts[esp]
   fstp     dword ptr [hwptr1+56]
   mov      vb1,dword ptr[post_idx1+4]
   mov      dword ptr [hwptr1+60],rhw1
   mov      vc1,dword ptr[post_idx1+8]

   lea      va1,dword ptr [verts2+va1*4]
   mov      count1,dword ptr _count[esp]
   lea      vb1,dword ptr [verts2+vb1*4]
   add      hwptr1,64

   jmp      _DrawTriIdx2GouradAsm_KNI

culled_tri:
   add      post_idx1,12
   mov      verts2,dword ptr _verts[esp]
   fstp     st(0)
   mov      va1,dword ptr [post_idx1]
   fstp     st(0)
   mov      vb1,dword ptr [post_idx1+4]
   fstp     st(0)
   mov      vc1,dword ptr [post_idx1+8]

   lea      va1,dword ptr [verts2+va1*4]
   mov      count1,dword ptr _count[esp]

   lea      vb1,dword ptr [verts2+vb1*4]

   jmp      _DrawTriIdx2GouradAsm_KNI

done_render:
   mov      hwptr_old,dword ptr[__asm_data + _FIFO_PTR]
   mov      dword ptr[__asm_data + _FIFO_PTR],hwptr1

   sub      hwptr1,hwptr_old
   mov      room2,dword ptr [__asm_data + _FIFO_ROOM]

   shr      hwptr1,2
   sub      room2,hwptr1

   mov      dword ptr [__asm_data + _FIFO_ROOM],room2

   mov      eax,dword ptr _idx[esp]

   add      esp,STACK_SIZE
   pop      esi
   pop      edi
   pop      ebx
   pop      ebp
   pop      esp

   ret

_DrawTriIdx2GouradAsm_KNI   ENDP


_TEXT   ENDS


END

