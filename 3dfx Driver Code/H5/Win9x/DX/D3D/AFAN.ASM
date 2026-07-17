TITLE    amesh.asm
         .686P
         .model FLAT

EXTRN    __asm_data:BYTE
EXTRN    _fifo_make_room_asm:NEAR

; *****************************
; Fan section
; *****************************

INCLUDE aglobal.inc

_TEXT       SEGMENT
            .code

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

ifdef PERF_MONITORING
extrn __penter:proc
endif

align   16
PUBLIC   _DrawTriFanImmAsm_P5
_DrawTriFanImmAsm_P5    PROC     NEAR

ifdef PERF_MONITORING
   call    __penter
endif

_DrawTriFanImmAsm_P5_begin:
   push     esp
   push     ebp
   push     ebx
   push     edi
   push     esi
   sub      esp,STACK_SIZE


;  vb1         =ebx
;  vc1         =ecx
;  count1      =eax
;  room1       =edx
;  chunk_size1 =esi

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

   mov      count1,dword ptr _count[esp]
   mov      chunk_size1,CHUNK_SIZE

   sub      chunk_size1,count1
   mov      vb1,dword ptr _vb_offset[esp]

   jl       fan_too_large

fan_split_reentry:

; size1     =replaces count1
   mov      count2,count1
   lea      size1,dword ptr[size1+size1*2]
   mov      room1,dword ptr [__asm_data + _FIFO_ROOM]

   shl      size1,3h

   sub      room1,size1
   mov      vc1,dword ptr [__asm_data + _VERTEX_SIZE]

   js       fifo_check_fan

fifo_fail_reentry:
   mov      va1,dword ptr _verts[esp]
   add      vb1,va1

   mov      s0,dword ptr[__asm_data + _T0_OFFSET]

   add      vc1,vb1
   dec      count2

   mov      hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   js       done_render


start_fan:

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
   mov      dword ptr _count[esp],count2
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

   mov      dword ptr [hwptr1],FAN_HEADER
   mov      color1,dword ptr color[va1]

   fstp     dword ptr [hwptr1+4]
   mov      rhw1,dword ptr rhw[va1]
   fstp     dword ptr [hwptr1+8]
   mov      dword ptr [hwptr1+12],color1
   fstp     dword ptr [hwptr1+16]
   mov      color1,dword ptr color[vb1]
ifdef DCT_FIX
   fld		dword ptr rhw[va1]
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
   mov      vb1,vc1
   fstp     dword ptr [hwptr1+4 - 28]
   mov      vc1,dword ptr[__asm_data + _VERTEX_SIZE]
   mov      dword ptr [hwptr1+8 - 28],color1
   fstp     dword ptr [hwptr1+12 - 28]
   mov      count2,dword ptr _count[esp]
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+16 - 28]
else
   mov      dword ptr [hwptr1+16 - 28],rhw1
endif
   add      vc1,vb1
   fstp     dword ptr [hwptr1+20 - 28]
   dec      count2
   fstp     dword ptr [hwptr1+24 - 28]

   js       done_render

fan_continue:
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
   mov      dword ptr _count[esp],count2
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
   fld      dword ptr rhw[vc1]
   mov      area1,dword ptr _area[esp]
   fmul     st(1),st
   and      area1,80000000h
   ;  rhw      tuadj    tvadj2
   fmulp    st(2),st
   ;  tuadj    tvadj
   xor      area1,dword ptr [__asm_data +_CULL_MASK]
   fld      dword ptr sz[vc1]
   ;  sz       tuadj    tvadj
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    tuadj    tvadj
   jz       culled_tri

   fld      dword ptr sy[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    tuadj    tvadj
   fld      dword ptr sx[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    tuadj    tvadj

   mov      dword ptr [hwptr1],FAN_HEADER_CONT
   mov      color1,dword ptr color[vc1]

   fstp     dword ptr [hwptr1+4]
   mov      rhw1,dword ptr rhw[vc1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif
   fstp     dword ptr [hwptr1+8]
   mov      vb1,vc1
   mov      dword ptr [hwptr1+12],color1
   mov      count2,dword ptr _count[esp]
   fstp     dword ptr [hwptr1+16]
   mov      vc1,dword ptr[__asm_data + _VERTEX_SIZE]
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+20]
else
   mov      dword ptr [hwptr1+20],rhw1
endif
   add      vc1,vb1
   fstp     dword ptr [hwptr1+24]
   fstp     dword ptr [hwptr1+28]

   add      hwptr1,32
   dec      count2
   jns      fan_continue

done_render:
   mov      hwptr_old,dword ptr[__asm_data + _FIFO_PTR]
   mov      dword ptr[__asm_data + _FIFO_PTR],hwptr1

   sub      hwptr1,hwptr_old
   mov      room2,dword ptr [__asm_data + _FIFO_ROOM]

   shr      hwptr1,2

   sub      room2,hwptr1

   mov      dword ptr [__asm_data + _FIFO_ROOM],room2

   mov      eax,vc1
   add      esp,STACK_SIZE
   pop      esi
   pop      edi
   pop      ebx
   pop      ebp
   pop      esp

   ret

align   16
culled_tri:
   fstp     st(0)
   mov      count2,dword ptr _count[esp]
   mov      vb1,vc1
   mov      vc1,dword ptr[__asm_data + _VERTEX_SIZE]
   fstp     st(0)
   add      vc1,vb1
   dec      count2
   fstp     st(0)
   jns      start_fan
   nop
   jmp      done_render

align 16
fan_too_large:
   mov      ret_addr1,dword ptr _ret_addr[esp]
   mov      dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub      count1,CHUNK_SIZE
   mov      dword ptr[__asm_data + _SAVED_COUNT],count1
   mov      dword ptr _ret_addr[esp],fan_split_continue
   mov      count1,CHUNK_SIZE
   jmp      fan_split_reentry

align 16
fan_split_continue:
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

   jmp      _DrawTriFanImmAsm_P5_begin

align   16
fifo_check_fan:
   push     vc1
   push     count2
   push     size1
   call     _fifo_make_room_asm
   pop      size1
   pop      count2
   pop      vc1
   jmp      fifo_fail_reentry

_DrawTriFanImmAsm_P5    ENDP

_TEXT   ENDS

END
