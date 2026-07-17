TITLE           amesh2p5.asm
                        .686P
                        .model FLAT

EXTRN    __asm_data:BYTE
EXTRN    _fifo_make_room_asm:NEAR

ifdef PERF_MONITORING
extrn __penter:proc
endif

; *****************************
; Mesh section
; *****************************

_TEXT       SEGMENT
            .code

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

align   16
PUBLIC   _DrawTriEdge2Asm_P5
_DrawTriEdge2Asm_P5   PROC     NEAR
;esp->0  local6
;     4  local5
;     8  local4
;     12 local3
;     16 local2
;     20 - 80        _post_idx
;     84 esi
;     28 edi
;     32 ebx
;     36 ebp
;     40 esp
;     44 ret_addr
;     48 pRc         _pRc
;     52 count       _count
;     56 idx         _idx
;     60 vertices    _vertices

; *****************************
; Mesh Prefetch Section
; *****************************
chunk_size1 TEXTEQU  <eax>
va1         TEXTEQU  <eax>
cull2       TEXTEQU  <eax>
tmp1        TEXTEQU  <eax>
ret_addr1   TEXTEQU  <eax>

vb1         TEXTEQU  <ebx>
size1       TEXTEQU  <ebx>
area2       TEXTEQU  <ebx>
spec2       TEXTEQU  <ebx>

vc1         TEXTEQU  <ecx>
hwptr_old   TEXTEQU  <ecx>
count2      TEXTEQU  <ecx>

count1      TEXTEQU  <edi>
rhw1        TEXTEQU  <edi>
last_flags  TEXTEQU  <edi>
verts2      TEXTEQU  <edi>
cull1       TEXTEQU  <edi>
flags2      TEXTEQU  <edi>
fog         TEXTEQU  <edi>

hwptr1      TEXTEQU  <ebp>

idx1        TEXTEQU  <esi>

room1       TEXTEQU  <edx>
area1       TEXTEQU  <edx>
color1      TEXTEQU  <edx>
flags1      TEXTEQU  <edx>
last2       TEXTEQU  <edx>
specular    TEXTEQU  <edx>

ifdef PERF_MONITORING
   call    __penter
endif

_DrawTriEdge2Asm_P5_begin:
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
   jl       TriMeshTooLarge

TriMeshReentry:
   lea      size1,dword ptr[size1+size1*2]
   mov      idx1,dword ptr _idx[esp]

   shl      size1,3h
   mov      va1,dword ptr [idx1]

   sub      room1,size1
   mov      vc1,dword ptr [idx1+4]

   js       fifo_check_mesh

fifo_check_mesh_reentry:
   mov      hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   dec      count1

   mov      vb1,va1
   and      va1,0ffffh

   mov      dword ptr _count[esp],count1
   and      vb1,0ffff0000h
   and      vc1,0ffffh

ind_tri_begin:
   shl      va1,5h
   mov      verts2,dword ptr _verts[esp]

   shr      vb1,11
   and      vc1,0ffffh

   lea      va1,dword ptr[verts2+va1]
   shl      vc1,5h

   lea      vb1,dword ptr[verts2+vb1]
   lea      vc1,dword ptr[verts2+vc1]

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

   mov      area1,80000000h
   mov      cull1,dword ptr [__asm_data +_CULL_MASK]

   fmulp    st(2),st
   ;  ar2      ar1
   fld      dword ptr tu[va1]
   fmul     dword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tuadj1   ar2      ar1
   fld      dword ptr tv[va1]
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

   and      area1,dword ptr _area[esp]

   fld      dword ptr rhw[va1]
   ;  rwh      tuadj    tvadj2
   fmulp    st(2),st
   ;  tuadj    tvadj

   xor      area1,cull1

   fld      dword ptr sz[va1]
   ;  sz       tuadj    tvadj
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    tuadj    tvadj

   jz       culled_tri

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
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+20]	
else
   mov      dword ptr [hwptr1+20],rhw1
endif
   fstp     dword ptr [hwptr1+24]
   fstp     dword ptr [hwptr1+28]

   mov      color1,dword ptr color[vb1]
   mov      rhw1,dword ptr rhw[vb1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif

   ; sending down vertB
   fld      dword ptr tv[vb1]
   fmul     dword ptr [__asm_data + _SCALE_T]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tvadj1
   fld      dword ptr tu[vb1]
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
   fstp     dword ptr [hwptr1+24]

   mov      color1,dword ptr color[vc1]
   mov      rhw1,dword ptr rhw[vc1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif

   ; sending down vertC
   fld      dword ptr tv[vc1]
   fmul     dword ptr [__asm_data + _SCALE_T]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tvadj1
   fld      dword ptr tu[vc1]
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

   mov      count2,dword ptr _count[esp]

   mov      tmp1,idx1
   add      idx1,8

   fstp     dword ptr [hwptr1 - 28]
   fstp     dword ptr [hwptr1+4 - 28]
   mov      dword ptr [hwptr1+8 - 28],color1
   fstp     dword ptr [hwptr1+12 - 28]
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+16 - 28]	
else
   mov      dword ptr [hwptr1+16 - 28],rhw1
endif
   dec      count2
   fstp     dword ptr [hwptr1+20 - 28]
   fstp     dword ptr [hwptr1+24 - 28]

   js       done_render
   mov      va1,dword ptr [idx1]

   mov      dword ptr _count[esp],count2
   mov      flags1,dword ptr [idx1+4]

   mov      vb1,va1
   mov      vc1,flags1

   and      flags1,1f0000h
   and      va1,0ffffh
   and      vb1,0ffff0000h

   and      vc1,0ffffh
   sub      flags1,01E0000h

   js       ind_tri_begin

   mov      dword ptr _last[esp],flags1

prim_continue:
   shl      va1,5h
   mov      verts2,dword ptr _verts[esp]

   shr      vb1,11
   and      vc1,0ffffh

   lea      va1,dword ptr[verts2+va1]
   shl      vc1,5h

   lea      vb1,dword ptr[verts2+vb1]
   lea      vc1,dword ptr[verts2+vc1]

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

   mov      cull2,dword ptr [__asm_data +_CULL_MASK]
   mov      area2,80000000h

   fmulp    st(2),st
   ;  ar2      ar1
   fld      dword ptr tu[vc1]
   fmul     dword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tuadj1   ar2      ar1
   fld      dword ptr tv[vc1]
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

   and      area2,dword ptr _area[esp]

   fld      dword ptr rhw[vc1]
   ;  rwh      tuadj    tvadj2
   fmulp    st(2),st
   ;  tuadj    tvadj

   shl      flags1,6
   xor      area2,cull2

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

   or       flags1,STRIP_CONTINUE
   mov      rhw1,dword ptr rhw[vc1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif

   mov      dword ptr [hwptr1],flags1
   add      hwptr1,32
   mov      color1,dword ptr color[vc1]
   add      idx1,8

   fstp     dword ptr [hwptr1 - 28]
   fstp     dword ptr [hwptr1 - 24]
   mov      dword ptr [hwptr1 - 20],color1
   mov      count2,dword ptr _count[esp]
   fstp     dword ptr [hwptr1 - 16]
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+16 - 28]	
else
   mov      dword ptr [hwptr1 - 12],rhw1
endif
   fstp     dword ptr [hwptr1 - 8]
   dec      count2
   fstp     dword ptr [hwptr1 - 4]

   js       done_render

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

   js       ind_tri_begin

   mov      flags2,flags1
   xor      flags1,dword ptr _last[esp]

   mov      dword ptr _last[esp],flags2
   xor      flags1,BIT_16

   jmp      prim_continue


align   16
culled_tri:
   mov      count1,dword ptr _count[esp]
   fstp     st(0)
   add      idx1,8
   dec      count1
   fstp     st(0)
   fstp     st(0)
   mov      dword ptr _count[esp],count1
   js       done_render

   mov      va1,dword ptr [idx1]
   mov      vc1,dword ptr [idx1+4]

   mov      vb1,va1
   mov      verts2,dword ptr _verts[esp]

   and      va1,0ffffh
   and      vb1,0ffff0000h
   and      vc1,0ffffh

   jmp      ind_tri_begin

done_render:
   mov      hwptr_old,dword ptr[__asm_data + _FIFO_PTR]
   mov      dword ptr[__asm_data + _FIFO_PTR],hwptr1

   sub      hwptr1,hwptr_old
   mov      room1,dword ptr [__asm_data + _FIFO_ROOM]

   shr      hwptr1,2
   mov      dword ptr _idx[esp],idx1

   sub      room1,hwptr1

   mov      dword ptr [__asm_data + _FIFO_ROOM],room1

   add      esp,STACK_SIZE
   pop      esi
   pop      edi
   pop      ebx
   pop      ebp
   pop      esp

   ret


align 16
fifo_check_mesh:
;regs that still need to be valid on jmp
;va1,vc1,idx1,count1

   push     va1
   push     vc1

   push     size1
   call     _fifo_make_room_asm
   pop      size1

   pop      vc1
   pop      va1

   jmp      fifo_check_mesh_reentry

align   16
TriMeshTooLarge:
;saving count1,size1,room1
   mov      ret_addr1,dword ptr _ret_addr[esp]
   mov      dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub      count1,CHUNK_SIZE
   mov      dword ptr[__asm_data + _SAVED_COUNT],count1
   mov      dword ptr _ret_addr[esp],TriMeshContinue
   mov      count1,CHUNK_SIZE
   jmp      TriMeshReentry

align   16
TriMeshContinue:
   mov      edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov      eax,dword ptr[__asm_data + _SAVED_RET_ADDR]

   mov      dword ptr _count[esp - _params],edx

   push     eax

   jmp      _DrawTriEdge2Asm_P5_begin

_DrawTriEdge2Asm_P5   ENDP

align   16
PUBLIC   _DrawTriEdge2GAsm_P5
_DrawTriEdge2GAsm_P5   PROC     NEAR

; *****************************
; Mesh Prefetch Section
; *****************************

ifdef PERF_MONITORING
   call    __penter
endif

_DrawTriEdge2GAsm_P5_begin:
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
   jl       TriMeshTooLarge

TriMeshReentry:
   lea      size1,dword ptr[size1+size1*2]
   mov      idx1,dword ptr _idx[esp]

   shl      size1,3h
   mov      va1,dword ptr [idx1]

   sub      room1,size1
   mov      vc1,dword ptr [idx1+4]

   js       fifo_check_mesh

fifo_check_mesh_reentry:
   mov      hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   dec      count1

   mov      vb1,va1
   and      va1,0ffffh



   mov      dword ptr _count[esp],count1

   and      vb1,0ffff0000h

   and      vc1,0ffffh

ind_tri_begin:
   shl      va1,5h
   mov      verts2,dword ptr _verts[esp]

   shr      vb1,11
   and      vc1,0ffffh

   lea      va1,dword ptr[verts2+va1]
   shl      vc1,5h

   lea      vb1,dword ptr[verts2+vb1]
   lea      vc1,dword ptr[verts2+vc1]

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
   mov      area1,80000000h
   fmulp    st(2),st
   ;  ar2      ar1
   fld      dword ptr sz[va1]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    ar2      ar1
   fxch     st(2)
   ;  ar1      ar2      szadj
   fsubrp   st(1),st
   ;  area     szadj

   mov      cull1,dword ptr [__asm_data +_CULL_MASK]

   fstp     dword ptr _area[esp]

   fld      dword ptr sy[va1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]

   and      area1,dword ptr _area[esp]

   fld      dword ptr sx[va1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]

   xor      area1,cull1

   jz       culled_tri

   mov      color1,dword ptr color[va1]
   mov      rhw1,dword ptr rhw[va1]

   mov      dword ptr [hwptr1],FAN_GOURAD_HEADER

   fstp     dword ptr [hwptr1+4]
   fstp     dword ptr [hwptr1+8]
   mov      dword ptr [hwptr1+12],color1
   fstp     dword ptr [hwptr1+16]
   mov      dword ptr [hwptr1+20],rhw1

   mov      color1,dword ptr color[vb1]
   mov      rhw1,dword ptr rhw[vb1]

   ; sending down vertB
   fld      dword ptr sz[vb1]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj
   fld      dword ptr sy[vb1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj
   fld      dword ptr sx[vb1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj
   add      hwptr1,24

   fstp     dword ptr [hwptr1]
   fstp     dword ptr [hwptr1+4]
   mov      dword ptr [hwptr1+8],color1
   fstp     dword ptr [hwptr1+12]
   mov      dword ptr [hwptr1+16],rhw1

   mov      color1,dword ptr color[vc1]
   mov      rhw1,dword ptr rhw[vc1]

   ; sending down vertC
   fld      dword ptr sz[vc1]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj
   fld      dword ptr sy[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj
   fld      dword ptr sx[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj

   add      hwptr1,40
   mov      count2,dword ptr _count[esp]

   mov      tmp1,idx1
   add      idx1,8

   fstp     dword ptr [hwptr1 - 20]
   fstp     dword ptr [hwptr1 - 16]
   mov      dword ptr [hwptr1 - 12],color1
   dec      count2
   fstp     dword ptr [hwptr1 - 8]
   mov      dword ptr [hwptr1 - 4],rhw1

   js       done_render

   mov      va1,dword ptr [idx1]

   mov      dword ptr _count[esp],count2
   mov      flags1,dword ptr [idx1+4]

   mov      vb1,va1
   mov      vc1,flags1

   and      flags1,1f0000h
   and      va1,0ffffh
   and      vb1,0ffff0000h

   and      vc1,0ffffh
   sub      flags1,01E0000h

   js       ind_tri_begin

   mov      dword ptr _last[esp],flags1

prim_continue:
   shl      va1,5h
   mov      verts2,dword ptr _verts[esp]

   shr      vb1,11
   and      vc1,0ffffh

   lea      va1,dword ptr[verts2+va1]
   shl      vc1,5h

   lea      vb1,dword ptr[verts2+vb1]
   lea      vc1,dword ptr[verts2+vc1]

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

   shl      flags1,6
   mov      area2,80000000h

   fmulp    st(2),st
   ;  ar2      ar1
   fld      dword ptr sz[vc1]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    ar2      ar1
   fxch     st(2)
   ;  ar1      ar2      szadj
   fsubrp   st(1),st
   ;  area     szadj

   mov      cull2,dword ptr [__asm_data +_CULL_MASK]
   or       flags1,STRIP_GOURAD_CONT

   fstp     dword ptr _area[esp]

   fld      dword ptr sy[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]

   and      area2,dword ptr _area[esp]

   fld      dword ptr sx[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]

   xor      area2,cull2

   jz       culled_tri

   mov      dword ptr [hwptr1],flags1

   mov      rhw1,dword ptr rhw[vc1]
   add      hwptr1,24
   mov      color1,dword ptr color[vc1]
   add      idx1,8

   fstp     dword ptr [hwptr1 - 20]
   mov      count2,dword ptr _count[esp]
   fstp     dword ptr [hwptr1 - 16]
   mov      dword ptr [hwptr1 - 12],color1
   fstp     dword ptr [hwptr1 - 8]
   dec      count2
   mov      dword ptr [hwptr1 - 4],rhw1

   js       done_render

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

   js       ind_tri_begin

   mov      flags2,flags1
   xor      flags1,dword ptr _last[esp]

   mov      dword ptr _last[esp],flags2
   xor      flags1,BIT_16

   jmp      prim_continue


align   16
culled_tri:
   mov      count1,dword ptr _count[esp]
   fstp     st(0)
   add      idx1,8
   dec      count1
   fstp     st(0)
   fstp     st(0)
   mov      dword ptr _count[esp],count1
   js       done_render

   mov      va1,dword ptr [idx1]
   mov      vc1,dword ptr [idx1+4]

   mov      vb1,va1
   mov      verts2,dword ptr _verts[esp]

   and      va1,0ffffh
   and      vb1,0ffff0000h
   and      vc1,0ffffh

   jmp      ind_tri_begin

done_render:
   mov      hwptr_old,dword ptr[__asm_data + _FIFO_PTR]
   mov      dword ptr[__asm_data + _FIFO_PTR],hwptr1

   sub      hwptr1,hwptr_old
   mov      room1,dword ptr [__asm_data + _FIFO_ROOM]

   shr      hwptr1,2
   mov      dword ptr _idx[esp],idx1

   sub      room1,hwptr1
   mov      dword ptr [__asm_data + _FIFO_ROOM],room1

   add      esp,STACK_SIZE
   pop      esi
   pop      edi
   pop      ebx
   pop      ebp
   pop      esp

   ret


align 16
fifo_check_mesh:
;regs that still need to be valid on jmp
;va1,vc1,idx1,count1

   push     va1
   push     vc1

   push     size1
   call     _fifo_make_room_asm
   pop      size1

   pop      vc1
   pop      va1

   jmp      fifo_check_mesh_reentry

align   16
TriMeshTooLarge:
;saving count1,size1,room1
   mov      ret_addr1,dword ptr _ret_addr[esp]
   mov      dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub      count1,CHUNK_SIZE
   mov      dword ptr[__asm_data + _SAVED_COUNT],count1
   mov      dword ptr _ret_addr[esp],TriMeshContinue
   mov      count1,CHUNK_SIZE
   jmp      TriMeshReentry

align   16
TriMeshContinue:
   mov      edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov      eax,dword ptr[__asm_data + _SAVED_RET_ADDR]

   mov      dword ptr _count[esp - _params],edx

   push     eax

   jmp      _DrawTriEdge2GAsm_P5_begin

_DrawTriEdge2GAsm_P5   ENDP

align   16
PUBLIC   _DrawTriEdge2FAsm_P5
_DrawTriEdge2FAsm_P5   PROC     NEAR

; *****************************
; Mesh Prefetch Section
; *****************************
ifdef PERF_MONITORING
   call    __penter
endif

_DrawTriEdge2FAsm_P5_begin:
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
   jl       TriMeshTooLarge

TriMeshReentry:
   lea      size1,dword ptr[size1+size1*2]
   mov      idx1,dword ptr _idx[esp]

   mov      tmp1,size1
   shl      size1,3h

   add      size1,tmp1
   mov      va1,dword ptr [idx1]

   sub      room1,size1
   mov      vc1,dword ptr [idx1+4]

   js       fifo_check_mesh

fifo_check_mesh_reentry:
   mov      hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   dec      count1

   mov      vb1,va1
   and      va1,0ffffh

   mov      dword ptr _count[esp],count1
   and      vb1,0ffff0000h
   and      vc1,0ffffh

ind_tri_begin:
   shl      va1,5h
   mov      verts2,dword ptr _verts[esp]

   shr      vb1,11
   and      vc1,0ffffh

   lea      va1,dword ptr[verts2+va1]
   shl      vc1,5h

   lea      vb1,dword ptr[verts2+vb1]
   lea      vc1,dword ptr[verts2+vc1]

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

   mov      area1,80000000h
   mov      cull1,dword ptr [__asm_data +_CULL_MASK]

   fmulp    st(2),st
   ;  ar2      ar1
   fld      dword ptr tu[va1]
   fmul     dword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tuadj1   ar2      ar1
   fld      dword ptr tv[va1]
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

   and      area1,dword ptr _area[esp]

   fld      dword ptr rhw[va1]
   ;  rwh      tuadj    tvadj2
   fmulp    st(2),st
   ;  tuadj    tvadj

   xor      area1,cull1
   mov      specular,dword ptr spec[va1]

   jz       culled_tri

   mov      fog,0ffh
   shr      specular,24
   sub      fog,specular

   mov      color1,dword ptr color[va1]

   mov      dword ptr _fog[esp],fog

   mov      rhw1,dword ptr rhw[va1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif

   fild     dword ptr _fog[esp]

   fld      dword ptr sz[va1]
   ;  sz       fog      tuadj    tvadj
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    fog      tuadj    tvadj
   fld      dword ptr sy[va1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    fog      tuadj    tvadj
   fld      dword ptr sx[va1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    fog      tuadj    tvadj

   mov      dword ptr [hwptr1],STRIP_FOGW_HEADER

   fstp     dword ptr [hwptr1+4]
   fstp     dword ptr [hwptr1+8]
   mov      dword ptr [hwptr1+12],color1
   mov      specular,dword ptr spec[vb1]
   fstp     dword ptr [hwptr1+16]
   fstp     dword ptr [hwptr1+20]
   shr      specular,24
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+24]	
else
   mov      dword ptr [hwptr1+24],rhw1
endif
   mov      fog,0ffh
   fstp     dword ptr [hwptr1+28]
   sub      fog,specular
   fstp     dword ptr [hwptr1+32]

   mov      color1,dword ptr color[vb1]
   mov      dword ptr _fog[esp],fog

   mov      rhw1,dword ptr rhw[vb1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif

   ; sending down vertB
   fld      dword ptr tv[vb1]
   fmul     dword ptr [__asm_data + _SCALE_T]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tvadj1
   fld      dword ptr tu[vb1]
   fmul     dword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tuadj1   tvadj1
   fild     dword ptr _fog[esp]
   ;  fog      tuadj1   tvadj1
   fld      dword ptr sz[vb1]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    fog      tuadj1   tvadj1
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_T]
   faddp    st(4),st
   ;  szadj    fog      tuadj1   tvadj2
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_S]
   faddp    st(3),st
   ;  szadj    fog      tuadj2   tvadj2
   fld      dword ptr sy[vb1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    fog      tuadj2   tvadj2
   fld      dword ptr sx[vb1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    fog      tuadj2   tvadj2
   fld      dword ptr rhw[vb1]
   ;  rhw      sxadj    syadj    szadj    fog      tuadj2   tvadj2
   fmul     st(6),st
   add      hwptr1,36
   fmulp    st(5),st
   ;  sxadj    syadj    szadj    fog      tuadj    tvadj

   fstp     dword ptr [hwptr1]
   fstp     dword ptr [hwptr1+4]
   mov      dword ptr [hwptr1+8],color1
   mov      specular,dword ptr spec[vc1]
   fstp     dword ptr [hwptr1+12]
   fstp     dword ptr [hwptr1+16]
   shr      specular,24
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+20]	
else
   mov      dword ptr [hwptr1+20],rhw1
endif
   mov      fog,0ffh
   fstp     dword ptr [hwptr1+24]
   sub      fog,specular
   fstp     dword ptr [hwptr1+28]

   mov      color1,dword ptr color[vc1]
   mov      dword ptr _fog[esp],fog

   mov      rhw1,dword ptr rhw[vc1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif

   ; sending down vertC
   fld      dword ptr tv[vc1]
   fmul     dword ptr [__asm_data + _SCALE_T]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tvadj1
   fld      dword ptr tu[vc1]
   fmul     dword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tuadj1   tvadj1
   fild     dword ptr _fog[esp]
   ;  fog      tuadj1   tvadj1
   fld      dword ptr sz[vc1]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    fog      tuadj1   tvadj1
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_T]
   faddp    st(4),st
   ;  szadj    fog      tuadj1   tvadj2
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_S]
   faddp    st(3),st
   ;  szadj    fog      tuadj2   tvadj2
   fld      dword ptr sy[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    fog      tuadj2   tvadj2
   fld      dword ptr sx[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    fog      tuadj2   tvadj2
   fld      dword ptr rhw[vc1]
   ;  rhw      sxadj    syadj    szadj    fog      tuadj2   tvadj2
   fmul     st(6),st
   add      hwptr1,64
   fmulp    st(5),st
   ;  sxadj    syadj    szadj    fog      tuadj    tvadj

   mov      count2,dword ptr _count[esp]

   mov      tmp1,idx1
   add      idx1,8

   fstp     dword ptr [hwptr1 - 32]
   fstp     dword ptr [hwptr1+4 - 32]
   mov      dword ptr [hwptr1+8 - 32],color1
   fstp     dword ptr [hwptr1+12 - 32]
   fstp     dword ptr [hwptr1+16 - 32]
   dec      count2
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+20 - 32]	
else
   mov      dword ptr [hwptr1+20 - 32],rhw1
endif
   fstp     dword ptr [hwptr1+24 - 32]
   fstp     dword ptr [hwptr1+28 - 32]

   js       done_render
   mov      va1,dword ptr [idx1]

   mov      dword ptr _count[esp],count2
   mov      flags1,dword ptr [idx1+4]

   mov      vb1,va1
   mov      vc1,flags1

   and      flags1,1f0000h
   and      va1,0ffffh
   and      vb1,0ffff0000h

   and      vc1,0ffffh
   sub      flags1,01E0000h

   js       ind_tri_begin

   mov      dword ptr _last[esp],flags1

prim_continue:
   shl      va1,5h
   mov      verts2,dword ptr _verts[esp]

   shr      vb1,11
   and      vc1,0ffffh

   lea      va1,dword ptr[verts2+va1]
   shl      vc1,5h

   lea      vb1,dword ptr[verts2+vb1]
   lea      vc1,dword ptr[verts2+vc1]

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

   mov      cull2,dword ptr [__asm_data +_CULL_MASK]
   mov      area2,80000000h

   fmulp    st(2),st
   ;  ar2      ar1
   fld      dword ptr tu[vc1]
   fmul     dword ptr [__asm_data + _SCALE_S]
ifdef DCT_FIX
   fmul     dword ptr [__asm_data + _SCALEWS+4]
endif
   ;  tuadj1   ar2      ar1
   fld      dword ptr tv[vc1]
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

   and      area2,dword ptr _area[esp]
   shl      flags1,6

   fld      dword ptr rhw[vc1]
   ;  rwh      tuadj    tvadj2
   fmulp    st(2),st
   ;  tuadj    tvadj

   xor      area2,cull2
   mov      spec2,dword ptr spec[vc1]

   jz       culled_tri

   mov      fog,0ffh
   shr      spec2,24

   sub      fog,spec2
   or       flags1,STRIP_FOGW_CONTINUE

   mov      dword ptr _fog[esp],fog
   add      hwptr1,36


   fild     dword ptr _fog[esp]
   ;  fog      tuadj    tvadj
   fld      dword ptr sz[vc1]
   ;  sz       fog      tuadj    tvadj
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    fog      tuadj    tvadj
   fld      dword ptr sy[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    fog      tuadj    tvadj
   fld      dword ptr sx[vc1]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    fog      tuadj    tvadj

   mov      dword ptr [hwptr1 - 36],flags1
   mov      rhw1,dword ptr rhw[vc1]
ifdef DCT_FIX
   mov		dword ptr [__asm_data + _TEMPSPACE], rhw1
endif

   add      idx1,8
   mov      color1,dword ptr color[vc1]

   fstp     dword ptr [hwptr1 - 32]
   fstp     dword ptr [hwptr1 - 28]
   mov      dword ptr [hwptr1 - 24],color1
   mov      count2,dword ptr _count[esp]
   fstp     dword ptr [hwptr1 - 20]
   fstp     dword ptr [hwptr1 - 16]
ifdef DCT_FIX
   fld		dword ptr [__asm_data + _TEMPSPACE]
   fmul     dword ptr [__asm_data + _SCALEWS+4]
   fstp		dword ptr [hwptr1+12]	
else
   mov      dword ptr [hwptr1 - 12],rhw1
endif
   fstp     dword ptr [hwptr1 - 8]
   dec      count2
   fstp     dword ptr [hwptr1 - 4]

   js       done_render

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

   js       ind_tri_begin

   mov      flags2,flags1
   xor      flags1,dword ptr _last[esp]

   mov      dword ptr _last[esp],flags2
   xor      flags1,BIT_16

   jmp      prim_continue


align   16
culled_tri:
   mov      count1,dword ptr _count[esp]
   fstp     st(0)
   add      idx1,8
   dec      count1
   fstp     st(0)
   mov      dword ptr _count[esp],count1
   js       done_render

   mov      va1,dword ptr [idx1]
   mov      vc1,dword ptr [idx1+4]

   mov      vb1,va1
   mov      verts2,dword ptr _verts[esp]

   and      va1,0ffffh
   and      vb1,0ffff0000h
   and      vc1,0ffffh

   jmp      ind_tri_begin

done_render:
   mov      hwptr_old,dword ptr[__asm_data + _FIFO_PTR]
   mov      dword ptr[__asm_data + _FIFO_PTR],hwptr1

   sub      hwptr1,hwptr_old
   mov      room1,dword ptr [__asm_data + _FIFO_ROOM]

   shr      hwptr1,2
   mov      dword ptr _idx[esp],idx1

   sub      room1,hwptr1

   mov      dword ptr [__asm_data + _FIFO_ROOM],room1

   add      esp,STACK_SIZE
   pop      esi
   pop      edi
   pop      ebx
   pop      ebp
   pop      esp

   ret


align 16
fifo_check_mesh:
;regs that still need to be valid on jmp
;va1,vc1,idx1,count1

   push     va1
   push     vc1

   push     size1
   call     _fifo_make_room_asm
   pop      size1

   pop      vc1
   pop      va1

   jmp      fifo_check_mesh_reentry

align   16
TriMeshTooLarge:
;saving count1,size1,room1
   mov      ret_addr1,dword ptr _ret_addr[esp]
   mov      dword ptr[__asm_data + _SAVED_RET_ADDR],ret_addr1
   sub      count1,CHUNK_SIZE
   mov      dword ptr[__asm_data + _SAVED_COUNT],count1
   mov      dword ptr _ret_addr[esp],TriMeshContinue
   mov      count1,CHUNK_SIZE
   jmp      TriMeshReentry

align   16
TriMeshContinue:
   mov      edx,dword ptr[__asm_data + _SAVED_COUNT]
   mov      eax,dword ptr[__asm_data + _SAVED_RET_ADDR]

   mov      dword ptr _count[esp - _params],edx

   push     eax

   jmp      _DrawTriEdge2FAsm_P5_begin

_DrawTriEdge2FAsm_P5   ENDP


_TEXT   ENDS

END
