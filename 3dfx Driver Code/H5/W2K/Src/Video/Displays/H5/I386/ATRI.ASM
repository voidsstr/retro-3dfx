TITLE           atri.asm
                        .686P
                        .model FLAT

EXTRN    __asm_data:BYTE
EXTRN    _fifo_make_room_asm:NEAR

ifdef PERF_MONITORING
extrn __penter:proc
endif

_TEXT       SEGMENT
            .code

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



align   16
PUBLIC   _PrefetchTriIdx2_5_Asm_P5
_PrefetchTriIdx2_5_Asm_P5     PROC     NEAR

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

_PrefetchTriIdx2_5_Asm_P5_begin:
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
   jmp      _DrawTriIdx2GouradAsm_P5

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
   jmp      _PrefetchTriIdx2_5_Asm_P5_begin

_PrefetchTriIdx2_5_Asm_P5     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_6T_Asm_P6
_PrefetchTriIdx2_6T_Asm_P6     PROC     NEAR

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

ifdef PERF_MONITORING
   call    __penter
endif

_PrefetchTriIdx2_6T_Asm_P6_begin:
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
   jmp      _DrawTriIdx2MainAsm_P6

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
   jmp      _PrefetchTriIdx2_6T_Asm_P6_begin

_PrefetchTriIdx2_6T_Asm_P6     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_7_Asm_P6
_PrefetchTriIdx2_7_Asm_P6     PROC     NEAR

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

ifdef PERF_MONITORING
   call    __penter
endif

_PrefetchTriIdx2_7_Asm_P6_begin:
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
   jmp      _DrawTriIdx2MainAsm_P6

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
   jmp      _PrefetchTriIdx2_7_Asm_P6_begin

_PrefetchTriIdx2_7_Asm_P6     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_8_Asm_P6
_PrefetchTriIdx2_8_Asm_P6     PROC     NEAR

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

ifdef PERF_MONITORING
   call    __penter
endif

_PrefetchTriIdx2_8_Asm_P6_begin:
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
   jmp      _DrawTriIdx2MainAsm_P6

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
   jmp      _PrefetchTriIdx2_8_Asm_P6_begin

_PrefetchTriIdx2_8_Asm_P6     ENDP

align 16
PUBLIC   _PrefetchTriIdx2_9_Asm_P6
_PrefetchTriIdx2_9_Asm_P6     PROC     NEAR

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

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_9_Asm_P6_begin:
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
   jmp      _DrawTriIdx2MainAsm_P6

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
   jmp      _PrefetchTriIdx2_9_Asm_P6_begin

_PrefetchTriIdx2_9_Asm_P6     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_10_Asm_P6
_PrefetchTriIdx2_10_Asm_P6     PROC     NEAR

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

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_10_Asm_P6_begin:
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
   jmp      _DrawTriIdx2MainAsm_P6

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
   jmp      _PrefetchTriIdx2_10_Asm_P6_begin

_PrefetchTriIdx2_10_Asm_P6     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_11_Asm_P6
_PrefetchTriIdx2_11_Asm_P6    PROC     NEAR

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

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_11_Asm_P6_begin:
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
   jmp      _DrawTriIdx2MainAsm_P6

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
   jmp      _PrefetchTriIdx2_11_Asm_P6_begin

_PrefetchTriIdx2_11_Asm_P6     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_12_Asm_P6
_PrefetchTriIdx2_12_Asm_P6     PROC     NEAR

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

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_12_Asm_P6_begin:
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
   jmp      _DrawTriIdx2MainAsm_P6

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
   jmp      _PrefetchTriIdx2_12_Asm_P6_begin

_PrefetchTriIdx2_12_Asm_P6     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_6G_Asm_P5
_PrefetchTriIdx2_6G_Asm_P5     PROC     NEAR

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

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_6G_Asm_P5_begin:
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
   jmp      _DrawTriIdx2GouradAsm_P5

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
   jmp      _PrefetchTriIdx2_6G_Asm_P5_begin

_PrefetchTriIdx2_6G_Asm_P5     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_6T_Asm_P5
_PrefetchTriIdx2_6T_Asm_P5     PROC     NEAR

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

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_6T_Asm_P5_begin:
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
   jmp      _DrawTriIdx2MainAsm_P5

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
   jmp      _PrefetchTriIdx2_6T_Asm_P5_begin

_PrefetchTriIdx2_6T_Asm_P5     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_7_Asm_P5
_PrefetchTriIdx2_7_Asm_P5     PROC     NEAR

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

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_7_Asm_P5_begin:
   push     esp
   push     ebp
   push     ebx
   push     edi
   push     esi
   sub      esp,STACK_SIZE

   mov      count1,dword ptr _count[esp]
   mov      chunk_size1,NUM_PREFETCH_TRIS

   sub      chunk_size1,count1
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
   jmp      _DrawTriIdx2MainAsm_P5

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
   jmp      _PrefetchTriIdx2_7_Asm_P5_begin

_PrefetchTriIdx2_7_Asm_P5     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_8_Asm_P5
_PrefetchTriIdx2_8_Asm_P5     PROC     NEAR

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

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_8_Asm_P5_begin:
   push     esp
   push     ebp
   push     ebx
   push     edi
   push     esi
   sub      esp,STACK_SIZE

   mov      count1,dword ptr _count[esp]
   mov      chunk_size1,NUM_PREFETCH_TRIS

   sub      chunk_size1,count1
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
   jmp      _DrawTriIdx2MainAsm_P5

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
   jmp      _PrefetchTriIdx2_8_Asm_P5_begin

_PrefetchTriIdx2_8_Asm_P5     ENDP

align 16
PUBLIC   _PrefetchTriIdx2_9_Asm_P5
_PrefetchTriIdx2_9_Asm_P5     PROC     NEAR

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

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_9_Asm_P5_begin:
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
   jmp      _DrawTriIdx2MainAsm_P5

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
   jmp      _PrefetchTriIdx2_9_Asm_P5_begin

_PrefetchTriIdx2_9_Asm_P5     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_10_Asm_P5
_PrefetchTriIdx2_10_Asm_P5     PROC     NEAR

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

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_10_Asm_P5_begin:
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
   jmp      _DrawTriIdx2MainAsm_P5

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
   jmp      _PrefetchTriIdx2_10_Asm_P5_begin

_PrefetchTriIdx2_10_Asm_P5     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_11_Asm_P5
_PrefetchTriIdx2_11_Asm_P5     PROC     NEAR

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

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_11_Asm_P5_begin:
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
   jmp      _DrawTriIdx2MainAsm_P5

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
   jmp      _PrefetchTriIdx2_11_Asm_P5_begin

_PrefetchTriIdx2_11_Asm_P5     ENDP

align   16
PUBLIC   _PrefetchTriIdx2_12_Asm_P5
_PrefetchTriIdx2_12_Asm_P5     PROC     NEAR

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

ifdef PERF_MONITORING
   call    __penter
endif
_PrefetchTriIdx2_12_Asm_P5_begin:
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
   jmp      _DrawTriIdx2MainAsm_P5

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
   jmp      _PrefetchTriIdx2_12_Asm_P5_begin

_PrefetchTriIdx2_12_Asm_P5     ENDP


; Core rendering routine
; supports x,y,z,rhw,color,s,t
; requires rhw
; color,s,t are unnecessary but sent to hardware
align 16
PUBLIC   _DrawTriIdx2MainAsm_P5
_DrawTriIdx2MainAsm_P5   PROC     NEAR

va1         TEXTEQU     <eax>
color1      TEXTEQU     <eax>
vb1         TEXTEQU     <ebx>
vc1         TEXTEQU     <ecx>
verts2      TEXTEQU     <edx>
s0          TEXTEQU     <edx>
room2       TEXTEQU     <edx>
count1      TEXTEQU     <edi>
area1       TEXTEQU     <edi>
rhw1        TEXTEQU     <edi>
hwptr1      TEXTEQU     <esi>
post_idx1   TEXTEQU     <ebp>
hwptr_old   TEXTEQU     <ebp>

; interface vars
; eax =  va1
; ebx =  vb1
; ecx =  dword offset of vc1
; edx =  verts2
; edi =  count1
; esi =  hwptr1
; ebp =  post_idx1

   dec      count1
   lea      vc1,dword ptr [verts2 + vc1*4]
   mov      s0,dword ptr[__asm_data + _T0_OFFSET]

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
   fld      dword ptr [s0 + va1]
   fmul     dword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tuadj1   ar2      ar1
   fld      dword ptr [s0 + 4 + va1]
   fmul     dword ptr [__asm_data + _SCALE_T]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tvadj1   tuadj1   ar2      ar1
   fxch     st(2)
   ;  ar2      tuadj1   tvadj1   ar1
   fsubp    st(3),st
   ;  tuadj1   tvadj1   area
   fadd     dword ptr [__asm_data + _TEXEL_OFFSET_S]
   ;  tuadj2   tvadj1   area
   fxch     st(1)
   ;  tvadj1   tuadj2   area
   fadd     dword ptr [__asm_data + _TEXEL_OFFSET_T]
   ;  tvadj2   tuadj2   area
   fxch     st(2)
   ;  area     tuadj2   tvadj2
   fstp     dword ptr _area[esp]
   ;  tuadj2   tvadj2
   fmul     dword ptr rhw[va1]
   ;  tuadj    tvadj2
   mov      area1,dword ptr _area[esp]

   fld      dword ptr rhw[va1]
   ;  rwh      tuadj    tvadj2
   and      area1,80000000h
   fmulp    st(2),st
   ;  tuadj    tvadj

   xor      area1,dword ptr [__asm_data +_CULL_MASK]
   fld      dword ptr sz[va1]
   ;  sz       tuadj    tvadj

   jz       culled_tri

   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    tuadj    tvadj
   fld      dword ptr sy[va1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    tuadj    tvadj
   fld      dword ptr sx[va1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    tuadj    tvadj

   mov      rhw1,dword ptr rhw[va1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif
   mov      dword ptr [hwptr1],FAN_HEADER
   mov      color1,dword ptr color[va1]

   fstp     dword ptr [hwptr1+4]
   fstp     dword ptr [hwptr1+8]
   mov      dword ptr [hwptr1+12],color1
   fstp     dword ptr [hwptr1+16]
   mov      color1,dword ptr color[vb1]
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+20]
else
   mov      dword ptr [hwptr1+20],rhw1
endif
   fstp     dword ptr [hwptr1+24]
   mov      rhw1,dword ptr rhw[vb1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif
   fstp     dword ptr [hwptr1+28]

   ; sending down vertB
   fld      dword ptr [s0 + 4 + vb1]
   fmul     dword ptr [__asm_data + _SCALE_T]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tvadj1
   fld      dword ptr [s0 + vb1]
   fmul     dword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tuadj1   tvadj1
   fld      dword ptr sz[vb1]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    tuadj1   tvadj1
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_T]
   faddp    st(3),st
   ;  szadj    tuadj1   tvadj2
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_S]
   faddp    st(2),st
   ;  szadj    tuadj2   tvadj2
   fld      dword ptr sy[vb1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    tuadj2   tvadj2
   fld      dword ptr sx[vb1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    tuadj2   tvadj2
   fld      dword ptr rhw[vb1]
   ;  rhw      sxadj    syadj    szadj    tuadj2   tvadj2
   fmul     st(5),st
   add      hwptr1,32
   fmulp    st(4),st
   ;  sxadj    syadj    szadj    tuadj    tvadj

   fstp     dword ptr [hwptr1]
   fstp     dword ptr [hwptr1+4]
   mov      dword ptr [hwptr1+8],color1
   fstp     dword ptr [hwptr1+12]
   mov      color1,dword ptr color[vc1]
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+16]
else
   mov      dword ptr [hwptr1+16],rhw1
endif
   fstp     dword ptr [hwptr1+20]
   mov      rhw1,dword ptr rhw[vc1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif
   fstp     dword ptr [hwptr1+24]

   ; sending down vertC
   fld      dword ptr [s0 + 4 + vc1]
   fmul     dword ptr [__asm_data + _SCALE_T]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tvadj1
   fld      dword ptr [s0 + vc1]
   fmul     dword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tuadj1   tvadj1
   fld      dword ptr sz[vc1]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    tuadj1   tvadj1
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_T]
   faddp    st(3),st
   ;  szadj    tuadj1   tvadj2
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_S]
   faddp    st(2),st
   ;  szadj    tuadj2   tvadj2
   fld      dword ptr sy[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    tuadj2   tvadj2
   fld      dword ptr sx[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    tuadj2   tvadj2
   fld      dword ptr rhw[vc1]
   ;  rhw      sxadj    syadj    szadj    tuadj2   tvadj2
   fmul     st(5),st
   add      hwptr1,56
   fmulp    st(4),st
   ;  sxadj    syadj    szadj    tuadj    tvadj

   fstp     dword ptr [hwptr1 - 28]
   add      post_idx1,12
   fstp     dword ptr [hwptr1+4 - 28]
   mov      verts2,dword ptr _verts[esp]
   mov      dword ptr [hwptr1+8 - 28],color1
   mov      va1,dword ptr[post_idx1]
   fstp     dword ptr [hwptr1+12 - 28]
   mov      vb1,dword ptr[post_idx1+4]
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+16 - 28]
else
   mov      dword ptr [hwptr1+16 - 28],rhw1
endif
   mov      count1,dword ptr _count[esp]
   fstp     dword ptr [hwptr1+20 - 28]
   lea      va1,dword ptr [verts2 + va1*4]
   fstp     dword ptr [hwptr1+24 - 28]
   lea      vb1,dword ptr [verts2 + vb1*4]
   mov      vc1,dword ptr [post_idx1+8]

   jmp      _DrawTriIdx2MainAsm_P5

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

   jmp      _DrawTriIdx2MainAsm_P5

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

_DrawTriIdx2MainAsm_P5   ENDP

; Core rendering routine
; supports x,y,z,rhw,color,s,t
; requires rhw
; color,s,t are unnecessary but sent to hardware
;IFDEF AGP_CMDFIFO
if 0	;STB-SK This is slower in Winbench 99 -- culling in software is a good thing!
align 16
PUBLIC   _DrawTriIdx2MainAsm_P6
_DrawTriIdx2MainAsm_P6   PROC     NEAR

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

   dec      count1
   lea      vc1,dword ptr [verts2 + vc1*4]
   mov      s0,dword ptr[__asm_data + _T0_OFFSET]

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
   fld      dword ptr [s0 + va1]
   fmul     dword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tuadj1   ar2      ar1
   fld      dword ptr [s0 + 4 + va1]
   fmul     dword ptr [__asm_data + _SCALE_T]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tvadj1   tuadj1   ar2      ar1
   fxch     st(2)
   ;  ar2      tuadj1   tvadj1   ar1
   fsubp    st(3),st
   ;  tuadj1   tvadj1   area
   fadd     dword ptr [__asm_data + _TEXEL_OFFSET_S]
   ;  tuadj2   tvadj1   area
   fxch     st(1)
   ;  tvadj1   tuadj2   area
   fadd     dword ptr [__asm_data + _TEXEL_OFFSET_T]
   ;  tvadj2   tuadj2   area
   fxch     st(2)
   ;  area     tuadj2   tvadj2
   fstp     dword ptr _area[esp]
   ;  tuadj2   tvadj2
   fmul     dword ptr rhw[va1]
   ;  tuadj    tvadj2
   mov      area1,dword ptr _area[esp]

   fld      dword ptr rhw[va1]
   ;  rwh      tuadj    tvadj2
   and      area1,80000000h
   mov      hwptr1,dword ptr _hwptr[esp]
   fmulp    st(2),st
   ;  tuadj    tvadj

   xor      area1,dword ptr [__asm_data +_CULL_MASK]
   fld      dword ptr sz[va1]
   ;  sz       tuadj    tvadj

   jz       culled_tri

   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    tuadj    tvadj
   fld      dword ptr sy[va1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    tuadj    tvadj
   fld      dword ptr sx[va1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    tuadj    tvadj

   mov      rhw1,dword ptr rhw[va1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif
   mov      dword ptr [hwptr1],FAN_HEADER
   mov      color1,dword ptr color[va1]

   fstp     dword ptr [hwptr1+4]
   fstp     dword ptr [hwptr1+8]
   mov      dword ptr [hwptr1+12],color1
   fstp     dword ptr [hwptr1+16]
   mov      color1,dword ptr color[vb1]
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+20]
else
   mov      dword ptr [hwptr1+20],rhw1
endif
   fstp     dword ptr [hwptr1+24]
   mov      rhw1,dword ptr rhw[vb1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif
   fstp     dword ptr [hwptr1+28]

   ; sending down vertB
   fld      dword ptr [s0 + 4 + vb1]
   fmul     dword ptr [__asm_data + _SCALE_T]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tvadj1
   fld      dword ptr [s0 + vb1]
   fmul     dword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tuadj1   tvadj1
   fld      dword ptr sz[vb1]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    tuadj1   tvadj1
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_T]
   faddp    st(3),st
   ;  szadj    tuadj1   tvadj2
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_S]
   faddp    st(2),st
   ;  szadj    tuadj2   tvadj2
   fld      dword ptr sy[vb1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    tuadj2   tvadj2
   fld      dword ptr sx[vb1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    tuadj2   tvadj2
   fld      dword ptr rhw[vb1]
   ;  rhw      sxadj    syadj    szadj    tuadj2   tvadj2
   fmul     st(5),st
   add      hwptr1,32
   fmulp    st(4),st
   ;  sxadj    syadj    szadj    tuadj    tvadj

   fstp     dword ptr [hwptr1]
   fstp     dword ptr [hwptr1+4]
   mov      dword ptr [hwptr1+8],color1
   fstp     dword ptr [hwptr1+12]
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+16]
else
   mov      dword ptr [hwptr1+16],rhw1
endif
   fstp     dword ptr [hwptr1+20]
   mov      rhw1,dword ptr rhw[vc1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif
   fstp     dword ptr [hwptr1+24]

   ; sending down vertC
   fld      dword ptr [s0 + 4 + vc1]
   fmul     dword ptr [__asm_data + _SCALE_T]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tvadj1
   fld      dword ptr [s0 + vc1]
   fmul     dword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tuadj1   tvadj1
   fld      dword ptr sz[vc1]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    tuadj1   tvadj1
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_T]
   faddp    st(3),st
   ;  szadj    tuadj1   tvadj2
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_S]
   faddp    st(2),st
   ;  szadj    tuadj2   tvadj2
   fld      dword ptr sy[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    tuadj2   tvadj2
   fld      dword ptr sx[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    tuadj2   tvadj2
   fld      dword ptr rhw[vc1]
   ;  rhw      sxadj    syadj    szadj    tuadj2   tvadj2
   fmul     st(5),st
   add      hwptr1,56
   fmulp    st(4),st
   ;  sxadj    syadj    szadj    tuadj    tvadj

   mov      color2,dword ptr color[vc1]
   add      post_idx1,12
   fstp     dword ptr [hwptr1 - 28]
   mov      va1,dword ptr[post_idx1]
   fstp     dword ptr [hwptr1+4 - 28]
   mov      vb1,dword ptr[post_idx1+4]
   mov      dword ptr [hwptr1+8 - 28],color2
   mov      vc1,dword ptr [post_idx1+8]
   fstp     dword ptr [hwptr1+12 - 28]
   mov      tmp1,dword ptr [post_idx1 + last2]
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+16 - 28]
else
   mov      dword ptr [hwptr1+16 - 28],rhw1
endif
   mov      tmp2,dword ptr [post_idx1 + last1]
   fstp     dword ptr [hwptr1+20 - 28]
   fstp     dword ptr [hwptr1+24 - 28]

   mov      dword ptr _hwptr[esp],hwptr1
   sub      tmp1,va1
   sub      tmp2,vb1

   mov      test1,0
   or       tmp2,tmp1

   mov      tmp1,dword ptr [post_idx1 + last0]
   cmovz    test1,dword ptr _strip_cont[esp]

   mov      tmp2,dword ptr [post_idx1 + last2]

   sub      tmp1,va1
   sub      tmp2,vb1

   mov      vc1,dword ptr [post_idx1+8]
   or       tmp2,tmp1
   mov      verts2,dword ptr _verts[esp]
   cmovz    test1,dword ptr _fan_cont[esp]

   lea      va1,dword ptr [verts2 + va1*4]
   and      test1,PRIM_TEST

   lea      vb1,dword ptr [verts2 + vb1*4]
   mov      count1,dword ptr _count[esp]

   jz       _DrawTriIdx2MainAsm_P6

   mov      dword ptr _last[esp],BIT_22

prim_continue:
   dec      count1

   lea      vc1,dword ptr [verts2 + vc1*4]
   mov      dword ptr _count[esp],count1

   mov      tmp1,test1
   xor      test1,dword ptr _last[esp]

   mov      dword ptr _last[esp],tmp1
   xor      test1,BIT_22

   mov      s0,dword ptr[__asm_data + _T0_OFFSET]
   and      test1,BIT_22

   mov      header1,dword ptr[__asm_data + _CULL_HEADER]

   fld      dword ptr [s0 + 4 + vc1]
   fmul     dword ptr [__asm_data + _SCALE_T]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tvadj1
   fld      dword ptr [s0 + vc1]
   fmul     dword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tuadj1   tvadj1
   fld      dword ptr sz[vc1]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    tuadj1   tvadj1
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_T]
   faddp    st(3),st
   ;  szadj    tuadj1   tvadj2
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_S]
   faddp    st(2),st
   ;  szadj    tuadj2   tvadj2
   fld      dword ptr sy[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    tuadj2   tvadj2
   fld      dword ptr sx[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    tuadj2   tvadj2
   fld      dword ptr rhw[vc1]
   ;  rhw      sxadj    syadj    szadj    tuadj2   tvadj2
   fmul     st(5),st
   or       header1,test1
   fmulp    st(4),st

   mov      hwptr1,dword ptr _hwptr[esp]
   add      post_idx1,12
   mov      rhw1,dword ptr rhw[vc1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif

   mov      dword ptr [hwptr1],header1
   mov      color2,dword ptr color[vc1]
   fstp     dword ptr [hwptr1+4]
   mov      va1,dword ptr[post_idx1]
   fstp     dword ptr [hwptr1+8]
   mov      vb1,dword ptr[post_idx1+4]
   mov      dword ptr [hwptr1+12],color2
   fstp     dword ptr [hwptr1+16]
   mov      tmp1,dword ptr [post_idx1 + last2]
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+20]
else
   mov      dword ptr [hwptr1+20],rhw1
endif
   fstp     dword ptr [hwptr1+24]
   mov      tmp2,dword ptr [post_idx1 + last1]
   fstp     dword ptr [hwptr1+28]

   add      hwptr1,32
   sub      tmp1,va1

   mov      dword ptr _hwptr[esp],hwptr1
   sub      tmp2,vb1

   mov      test1,0
   or       tmp2,tmp1

   mov      tmp1,dword ptr [post_idx1 + last0]
   cmovz    test1,dword ptr _strip_cont[esp]

   mov      tmp2,dword ptr [post_idx1 + last2]

   sub      tmp1,va1
   sub      tmp2,vb1

   or       tmp2,tmp1
   mov      verts2,dword ptr _verts[esp]
   cmovz    test1,dword ptr _fan_cont[esp]

   lea      va1,dword ptr [verts2 + va1*4]
   mov      count1,dword ptr _count[esp]

   and      test1,PRIM_TEST
   lea      vb1,dword ptr [verts2 + vb1*4]
   mov      vc1,dword ptr [post_idx1+8]

   jz       _DrawTriIdx2MainAsm_P6

   jmp      prim_continue

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

   jmp      _DrawTriIdx2MainAsm_P6

done_render:
   mov      hwptr1,dword ptr _hwptr[esp]
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

_DrawTriIdx2MainAsm_P6   ENDP

ELSE

align 16
PUBLIC   _DrawTriIdx2MainAsm_P6
_DrawTriIdx2MainAsm_P6   PROC     NEAR

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

   dec      count1
   lea      vc1,dword ptr [verts2 + vc1*4]
   mov      s0,dword ptr[__asm_data + _T0_OFFSET]

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
   fld      dword ptr [s0 + va1]
   fmul     dword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tuadj1   ar2      ar1
   fld      dword ptr [s0 + 4 + va1]
   fmul     dword ptr [__asm_data + _SCALE_T]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tvadj1   tuadj1   ar2      ar1
   fxch     st(2)
   ;  ar2      tuadj1   tvadj1   ar1
   fsubp    st(3),st
   ;  tuadj1   tvadj1   area
   fadd     dword ptr [__asm_data + _TEXEL_OFFSET_S]
   ;  tuadj2   tvadj1   area
   fxch     st(1)
   ;  tvadj1   tuadj2   area
   fadd     dword ptr [__asm_data + _TEXEL_OFFSET_T]
   ;  tvadj2   tuadj2   area
   fxch     st(2)
   ;  area     tuadj2   tvadj2
   fstp     dword ptr _area[esp]
   ;  tuadj2   tvadj2
   fmul     dword ptr rhw[va1]
   ;  tuadj    tvadj2
   mov      area1,dword ptr _area[esp]

   fld      dword ptr rhw[va1]
   ;  rwh      tuadj    tvadj2
   and      area1,80000000h
   mov      hwptr1,dword ptr _hwptr[esp]
   fmulp    st(2),st
   ;  tuadj    tvadj

   xor      area1,dword ptr [__asm_data +_CULL_MASK]
   fld      dword ptr sz[va1]
   ;  sz       tuadj    tvadj

   jz       culled_tri

   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    tuadj    tvadj
   fld      dword ptr sy[va1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    tuadj    tvadj
   fld      dword ptr sx[va1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    tuadj    tvadj

   mov      rhw1,dword ptr rhw[va1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif
   mov      dword ptr [hwptr1],FAN_HEADER
   mov      color1,dword ptr color[va1]
   fstp     dword ptr [hwptr1+4]
   fstp     dword ptr [hwptr1+8]
   mov      dword ptr [hwptr1+12],color1
   fstp     dword ptr [hwptr1+16]
   mov      color1,dword ptr color[vb1]
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+20]
else
   mov      dword ptr [hwptr1+20],rhw1
endif
   fstp     dword ptr [hwptr1+24]
   mov      rhw1,dword ptr rhw[vb1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif
   fstp     dword ptr [hwptr1+28]

   ; sending down vertB
   fld      dword ptr [s0 + 4 + vb1]
   fmul     dword ptr [__asm_data + _SCALE_T]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tvadj1
   fld      dword ptr [s0 + vb1]
   fmul     dword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tuadj1   tvadj1
   fld      dword ptr sz[vb1]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    tuadj1   tvadj1
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_T]
   faddp    st(3),st
   ;  szadj    tuadj1   tvadj2
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_S]
   faddp    st(2),st
   ;  szadj    tuadj2   tvadj2
   fld      dword ptr sy[vb1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    tuadj2   tvadj2
   fld      dword ptr sx[vb1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    tuadj2   tvadj2
   fld      dword ptr rhw[vb1]
   ;  rhw      sxadj    syadj    szadj    tuadj2   tvadj2
   fmul     st(5),st
   add      hwptr1,32
   fmulp    st(4),st
   ;  sxadj    syadj    szadj    tuadj    tvadj

   fstp     dword ptr [hwptr1]
   fstp     dword ptr [hwptr1+4]
   mov      dword ptr [hwptr1+8],color1
   fstp     dword ptr [hwptr1+12]
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+16]
else
   mov      dword ptr [hwptr1+16],rhw1
endif
   fstp     dword ptr [hwptr1+20]
   mov      rhw1,dword ptr rhw[vc1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif
   fstp     dword ptr [hwptr1+24]

   ; sending down vertC
   fld      dword ptr [s0 + 4 + vc1]
   fmul     dword ptr [__asm_data + _SCALE_T]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tvadj1
   fld      dword ptr [s0 + vc1]
   fmul     dword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tuadj1   tvadj1
   fld      dword ptr sz[vc1]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    tuadj1   tvadj1
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_T]
   faddp    st(3),st
   ;  szadj    tuadj1   tvadj2
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_S]
   faddp    st(2),st
   ;  szadj    tuadj2   tvadj2
   fld      dword ptr sy[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    tuadj2   tvadj2
   fld      dword ptr sx[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    tuadj2   tvadj2
   fld      dword ptr rhw[vc1]
   ;  rhw      sxadj    syadj    szadj    tuadj2   tvadj2
   fmul     st(5),st
   add      hwptr1,56
   fmulp    st(4),st
   ;  sxadj    syadj    szadj    tuadj    tvadj

   mov      color2,dword ptr color[vc1]
   add      post_idx1,12
   fstp     dword ptr [hwptr1 - 28]
   mov      va1,dword ptr[post_idx1]
   fstp     dword ptr [hwptr1+4 - 28]
   mov      vb1,dword ptr[post_idx1+4]
   mov      dword ptr [hwptr1+8 - 28],color2
   mov      vc1,dword ptr [post_idx1+8]
   fstp     dword ptr [hwptr1+12 - 28]
   mov      tmp1,dword ptr [post_idx1 + last2]
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+16 - 28]
else
   mov      dword ptr [hwptr1+16 - 28],rhw1
endif
   mov      tmp2,dword ptr [post_idx1 + last1]
   fstp     dword ptr [hwptr1+20 - 28]
   fstp     dword ptr [hwptr1+24 - 28]

   mov      dword ptr _hwptr[esp],hwptr1
   sub      tmp1,va1
   sub      tmp2,vb1

   mov      test1,0
   or       tmp2,tmp1

   mov      tmp1,dword ptr [post_idx1 + last0]
   cmovz    test1,dword ptr _strip_cont[esp]

   mov      tmp2,dword ptr [post_idx1 + last2]

   sub      tmp1,va1
   sub      tmp2,vb1

   mov      vc1,dword ptr [post_idx1+8]
   or       tmp2,tmp1
   mov      verts2,dword ptr _verts[esp]
   cmovz    test1,dword ptr _fan_cont[esp]

   lea      va1,dword ptr [verts2 + va1*4]
   and      test1,PRIM_TEST

   lea      vb1,dword ptr [verts2 + vb1*4]
   mov      count1,dword ptr _count[esp]

   jz       _DrawTriIdx2MainAsm_P6

   mov      dword ptr _last[esp],BIT_22

prim_continue:
   dec      count1

   lea      vc1,dword ptr [verts2 + vc1*4]
   mov      dword ptr _count[esp],count1

   mov      tmp1,test1
   xor      test1,dword ptr _last[esp]

   mov      dword ptr _last[esp],tmp1
   xor      test1,BIT_22

   mov      s0,dword ptr[__asm_data + _T0_OFFSET]

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
   mov      cull1,dword ptr [__asm_data +_CULL_MASK]
   mov      area1,80000000h
   fmulp    st(2),st
   ;  ar2      ar1
   fld      dword ptr [s0 + vc1]
   fmul     dword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tuadj1   ar2      ar1
   fld      dword ptr [s0 + 4 + vc1]
   fmul     dword ptr [__asm_data + _SCALE_T]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tvadj1   tuadj1   ar2      ar1
   fxch     st(2)
   ;  ar2      tuadj1   tvadj1   ar1
   fsubp    st(3),st
   ;  tuadj1   tvadj1   area
   fadd     dword ptr [__asm_data + _TEXEL_OFFSET_S]
   ;  tuadj2   tvadj1   area
   fxch     st(1)
   ;  tvadj1   tuadj2   area
   fadd     dword ptr [__asm_data + _TEXEL_OFFSET_T]
   ;  tvadj2   tuadj2   area
   fxch     st(2)
   ;  area     tuadj2   tvadj2
   fstp     dword ptr _area[esp]
   ;  tuadj2   tvadj2
   fmul     dword ptr rhw[vc1]
   ;  tuadj    tvadj2
   and      area1,dword ptr _area[esp]
   and      test1,BIT_22
   fld      dword ptr rhw[vc1]
   ;  rwh      tuadj    tvadj2
   fmulp    st(2),st
   ;  tuadj    tvadj

   xor      area1,cull1
   mov      header1,STRIP_CONTINUE
   fld      dword ptr sz[vc1]
   ;  sz       tuadj    tvadj

   jz       culled_tri

   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    tuadj    tvadj
   fld      dword ptr sy[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    tuadj    tvadj
   fld      dword ptr sx[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    tuadj    tvadj


   or       header1,test1
   mov      hwptr1,dword ptr _hwptr[esp]

   mov      color2,dword ptr color[vc1]
   mov      rhw1,dword ptr rhw[vc1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif

   mov      dword ptr [hwptr1],header1
   add      post_idx1,12
   fstp     dword ptr [hwptr1+4]
   mov      va1,dword ptr[post_idx1]
   fstp     dword ptr [hwptr1+8]
   mov      vb1,dword ptr[post_idx1+4]
   mov      dword ptr [hwptr1+12],color2
   fstp     dword ptr [hwptr1+16]
   mov      tmp1,dword ptr [post_idx1 + last2]
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+20]
else
   mov      dword ptr [hwptr1+20],rhw1
endif
   fstp     dword ptr [hwptr1+24]
   mov      tmp2,dword ptr [post_idx1 + last1]
   fstp     dword ptr [hwptr1+28]

   add      hwptr1,32
   sub      tmp1,va1

   mov      dword ptr _hwptr[esp],hwptr1
   sub      tmp2,vb1

   mov      test1,0
   or       tmp2,tmp1

   mov      tmp1,dword ptr [post_idx1 + last0]
   cmovz    test1,dword ptr _strip_cont[esp]

   mov      tmp2,dword ptr [post_idx1 + last2]

   sub      tmp1,va1
   sub      tmp2,vb1

   or       tmp2,tmp1
   mov      verts2,dword ptr _verts[esp]
   cmovz    test1,dword ptr _fan_cont[esp]

   lea      va1,dword ptr [verts2 + va1*4]
   mov      count1,dword ptr _count[esp]

   and      test1,PRIM_TEST
   lea      vb1,dword ptr [verts2 + vb1*4]
   mov      vc1,dword ptr [post_idx1+8]

   jz       _DrawTriIdx2MainAsm_P6

   jmp      prim_continue

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

   jmp      _DrawTriIdx2MainAsm_P6

done_render:
   mov      hwptr1,dword ptr _hwptr[esp]
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

_DrawTriIdx2MainAsm_P6   ENDP
ENDIF

align 16
PUBLIC   _DrawTriIdx2GouradAsm_P5
_DrawTriIdx2GouradAsm_P5   PROC     NEAR

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

   jmp      _DrawTriIdx2GouradAsm_P5

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

   jmp      _DrawTriIdx2GouradAsm_P5

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

_DrawTriIdx2GouradAsm_P5   ENDP


_TEXT   ENDS


END

