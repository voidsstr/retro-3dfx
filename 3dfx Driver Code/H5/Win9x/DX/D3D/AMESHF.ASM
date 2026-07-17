TITLE    ameshf.asm
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

NUM_PREFETCH_TRIS = 32
POST_IDX_SIZE  = ((NUM_PREFETCH_TRIS * 3) + 1)*4
STACK_SIZE     = POST_IDX_SIZE + 12

; Local stack section
_post_idx      = 0
_locals        = POST_IDX_SIZE

_area          = _locals
_sub_count     = _locals + 4
_fog           = _locals + 8

; Param section
_params        = STACK_SIZE + 24
_count         = _params + 0
_idx           = _params + 4
_vertices      = _params + 8

align   16
PUBLIC   _DrawTriEdgeFAsm
_DrawTriEdgeFAsm  PROC     NEAR
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

va1         TEXTEQU     <eax>
tmp1        TEXTEQU     <eax>
room1       TEXTEQU     <eax>
vb1         TEXTEQU     <ebx>
vc1         TEXTEQU     <ecx>
flags1      TEXTEQU     <edx>
verts1      TEXTEQU     <edx>
count1      TEXTEQU     <edi>
idx1        TEXTEQU     <esi>
post_idx1   TEXTEQU     <ebp>

ifdef PERF_MONITORING
   call    __penter
endif

_DrawTriEdgeFAsm_begin:
   push     esp
   push     ebp
   push     ebx
   push     edi
   push     esi
   sub      esp,STACK_SIZE

prefetch_start:
   ; move these lines up into stack setup part later
   mov      tmp1,dword ptr _count[esp]
   lea      post_idx1,dword ptr _post_idx[esp - 12]
   mov      count1,NUM_PREFETCH_TRIS

   mov      idx1,dword ptr _idx[esp]
   sub      tmp1,NUM_PREFETCH_TRIS
   ;cmovs    count1,dword ptr _count[esp]
   jns      @F
   mov      count1,dword ptr _count[esp]
@@:
   mov      dword ptr _count[esp],tmp1
   mov      dword ptr _sub_count[esp],count1

fetch_early:
   mov      va1,dword ptr[idx1]
   add      post_idx1,12

   mov      flags1,dword ptr[idx1+4]
   mov      vb1,va1

   mov      vc1,flags1

   shl      flags1,6
   and      va1,0ffffh

   and      vb1,0ffff0000h
   and      flags1,07C00000h

   shr      vb1,11
   and      vc1,0ffffh

   shl      va1,5h
   sub      flags1,7400000h
   ;cmovs    flags1,va1
   jns      @F
   mov      flags1,va1
@@:

   shl      vc1,5h
   or       va1,flags1
;verts1     =replaces flags1 here
   mov      verts1,dword ptr _vertices[esp]

   lea      vb1,[verts1+vb1]
   mov      dword ptr[post_idx1],va1

   lea      vc1,[verts1+vc1]
   mov      dword ptr[post_idx1+4],vb1
   add      idx1,8

   dec      count1
   mov      dword ptr[post_idx1+8],vc1
   jnz      fetch_early

   mov      dword ptr[post_idx1+12],0h
   add      post_idx1,12

   sub      post_idx1,esp     ;post_idx starts at top of stack

   mov      ebx,post_idx1
   shl      post_idx1,1h
   shr      ebx,2

   mov      room1,dword ptr [__asm_data + _FIFO_ROOM]
   add      post_idx1,ebx

   mov      dword ptr _idx[esp],idx1
   sub      room1,post_idx1
   js       fifo_check_mesh

; *****************************
; Start Primitive Section
; *****************************

va2         TEXTEQU     <eax>
header1     TEXTEQU     <eax>
color2      TEXTEQU     <eax>
specular2   TEXTEQU     <eax>

vb2         TEXTEQU     <ebx>
area2       TEXTEQU     <ebx>
rhw2        TEXTEQU     <ebx>
fog2        TEXTEQU     <ebx>

vc2         TEXTEQU     <ecx>
verts2      TEXTEQU     <ecx>

count2      TEXTEQU     <edx>
area1       TEXTEQU     <edx>
rhw1        TEXTEQU     <edx>
flags3      TEXTEQU     <edx>
fog         TEXTEQU     <edx>

color1      TEXTEQU     <edi>
flags2      TEXTEQU     <edi>
last_flags  TEXTEQU     <edi>
specular    TEXTEQU     <edi>

hwptr1      TEXTEQU     <esi>
post_idx1   TEXTEQU     <ebp>

mesh_begin:
   mov      hwptr1,dword ptr[__asm_data + _FIFO_PTR]
   lea      post_idx1,dword ptr _post_idx[esp]
   mov      verts2,dword ptr _vertices[esp]

   mov      count2,dword ptr _sub_count[esp]
   mov      va2,dword ptr[post_idx1]
   mov      vb2,dword ptr[post_idx1+4]
   and      va2,1fffffh

;needed here
;verts2
;va2 offset off verts
;vb2 ptr
;count2

;vc2     = replaces verts2
ind_tri:
   lea      va2,dword ptr[verts2+va2]
   dec      count2
   mov      vc2,dword ptr[post_idx1+8]
   js       done_render

   fld      dword ptr sx[va2]
   fsub     dword ptr sx[vb2]
   ;  abx
   fld      dword ptr sx[vb2]
   fsub     dword ptr sx[vc2]
   ;  bcx      abx
   fld      dword ptr sy[va2]
   fsub     dword ptr sy[vb2]
   ;  aby      bcx      abx
   fld      dword ptr sy[vb2]
   fsub     dword ptr sy[vc2]
   ;  bcy      aby      bcx      abx
   fxch     st(1)
   ;  aby      bcy      bcx      abx
   fmulp    st(2),st
   ;  bcy      ar2      abx
   mov      dword ptr _sub_count[esp],count2
   fmulp    st(2),st
   ;  ar2      ar1
   fld      dword ptr tu[va2]
   fmul     dword ptr [__asm_data + _SCALE_S]
   ;  tuadj1   ar2      ar1
   fld      dword ptr tv[va2]
   fmul     dword ptr [__asm_data + _SCALE_T]
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
   fmul     dword ptr rhw[va2]
   ;  tuadj    tvadj2

;area1    = replaces count2

   mov      area1,dword ptr _area[esp]
   fld      dword ptr rhw[va2]
   ;  rwh      tuadj    tvadj2
   and      area1,80000000h
   fmulp    st(2),st
   ;  tuadj    tvadj

   xor      area1,dword ptr [__asm_data +_CULL_MASK]
   mov      specular,dword ptr spec[va2]
   mov      fog,0ffh

   jz       culled_tri

   shr      specular,24

   sub      fog,specular
   mov      color1,dword ptr color[va2]

   mov      dword ptr _fog[esp],fog
   mov      rhw1,dword ptr rhw[va2]

   fild     dword ptr _fog[esp]

   fld      dword ptr sz[va2]
   ;  sz       fog      tuadj    tvadj
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    fog      tuadj    tvadj
   fld      dword ptr sy[va2]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    fog      tuadj    tvadj
   fld      dword ptr sx[va2]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    fog      tuadj    tvadj

;rhw1       = replaces area1
;color1     = replaces nothing
;   mov      color1,dword ptr color[va2]
;   mov      rhw1,dword ptr rhw[va2]

   WRITE_FIFO_3         hwptr1,0,STRIP_FOGW_HEADER
   ; sending down vertA
   WRITE_FIFO_FSTP_3    hwptr1,4
   WRITE_FIFO_FSTP_3    hwptr1,8
   WRITE_FIFO_3         hwptr1,12,color1
   mov      specular,dword ptr spec[vb2]
   WRITE_FIFO_FSTP_3    hwptr1,16
   mov      va2,dword ptr[post_idx1+12]
   WRITE_FIFO_FSTP_3    hwptr1,20
   shr      specular,24
   WRITE_FIFO_3         hwptr1,24,rhw1
   mov      fog,0FFh
   WRITE_FIFO_FSTP_3    hwptr1,28
   sub      fog,specular
   WRITE_FIFO_FSTP_3    hwptr1,32
   mov      color1,dword ptr color[vb2]

   mov      dword ptr _fog[esp],fog
   mov      rhw1,dword ptr rhw[vb2]

   ; sending down vertB
   fld      dword ptr tv[vb2]
   fmul     dword ptr [__asm_data + _SCALE_T]
   ;  tvadj1
   fld      dword ptr tu[vb2]
   fmul     dword ptr [__asm_data + _SCALE_S]
   ;  tuadj1   tvadj1
   fild     dword ptr _fog[esp]
   ;  fog      tuadj1   tvadj1
   fld      dword ptr sz[vb2]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    fog      tuadj1   tvadj1
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_T]
   faddp    st(4),st
   ;  szadj    fog      tuadj1   tvadj2
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_S]
   faddp    st(3),st
   ;  szadj    fog      tuadj2   tvadj2
   fld      dword ptr sy[vb2]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    fog      tuadj2   tvadj2
   fld      dword ptr sx[vb2]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    fog      tuadj2   tvadj2
   fld      dword ptr rhw[vb2]
   ;  rhw      sxadj    syadj    szadj    fog      tuadj2   tvadj2
   fmul     st(6),st
   add      hwptr1,36
   fmulp    st(5),st
   ;  sxadj    syadj    szadj    fog      tuadj    tvadj


   fstp     dword ptr [hwptr1]
   and      va2,1fffffh
   fstp     dword ptr [hwptr1+4]
   mov      dword ptr [hwptr1+8],color1
   mov      specular,dword ptr spec[vc2]
   fstp     dword ptr [hwptr1+12]
   fstp     dword ptr [hwptr1+16]
   shr      specular,24
   mov      dword ptr [hwptr1+20],rhw1
   mov      fog,0FFh
   fstp     dword ptr [hwptr1+24]
   sub      fog,specular
   fstp     dword ptr [hwptr1+28]
   mov      color1,dword ptr color[vc2]

   mov      dword ptr _fog[esp],fog
   mov      rhw1,dword ptr rhw[vc2]

   ; sending down vertC
   fld      dword ptr tv[vc2]
   fmul     dword ptr [__asm_data + _SCALE_T]
   ;  tvadj1
   fld      dword ptr tu[vc2]
   fmul     dword ptr [__asm_data + _SCALE_S]
   ;  tuadj1   tvadj1
   fild     dword ptr _fog[esp]
   ;  fog      tuadj1   tvadj1
   fld      dword ptr sz[vc2]
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    fog      tuadj1   tvadj1
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_T]
   faddp    st(4),st
   ;  szadj    fog      tuadj1   tvadj2
   fld      dword ptr [__asm_data + _TEXEL_OFFSET_S]
   faddp    st(3),st
   ;  szadj    fog      tuadj2   tvadj2
   fld      dword ptr sy[vc2]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    fog      tuadj2   tvadj2
   fld      dword ptr sx[vc2]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    fog      tuadj2   tvadj2
   fld      dword ptr rhw[vc2]
   ;  rhw      sxadj    syadj    szadj    fog      tuadj2   tvadj2
   fmul     st(6),st
   add      hwptr1,32
   fmulp    st(5),st
   ;  sxadj    syadj    szadj    fog      tuadj    tvadj

;flags2     = replaces color1
;verts2     = replaces vc2

   fstp     dword ptr [hwptr1]
   add      post_idx1,12
   fstp     dword ptr [hwptr1+4]
   mov      verts2,dword ptr _vertices[esp]
   mov      dword ptr [hwptr1+8],color1
   fstp     dword ptr [hwptr1+12]
   fstp     dword ptr [hwptr1+16]
   mov      flags2,dword ptr[post_idx1]
   mov      dword ptr [hwptr1+20],rhw1
   fstp     dword ptr [hwptr1+24]
   mov      vb2,dword ptr[post_idx1+4]
   fstp     dword ptr [hwptr1+28]

;count2     = replaces rhw1
   add      hwptr1,32
   test     flags2,0C00000h
   mov      count2,dword ptr _sub_count[esp]
   jz       ind_tri

;last_flags = replaces flags2
   mov      last_flags,BIT_22

;needed
;va2 offset off of verts
;vb2
;verts2
;post_idx1
;hwptr1
mesh_continue:
   lea      va2,dword ptr[verts2+va2]
   dec      count2
   mov      vc2,dword ptr[post_idx1+8]

   fld      dword ptr sx[va2]
   fsub     dword ptr sx[vb2]
   ;  abx
   fld      dword ptr sx[vb2]
   fsub     dword ptr sx[vc2]
   ;  bcx      abx
   fld      dword ptr sy[va2]
   fsub     dword ptr sy[vb2]
   ;  aby      bcx      abx
   fld      dword ptr sy[vb2]
   fsub     dword ptr sy[vc2]
   ;  bcy      aby      bcx      abx
   fxch     st(1)
   ;  aby      bcy      bcx      abx
   fmulp    st(2),st
   ;  bcy      ar2      abx
   mov      dword ptr _sub_count[esp],count2
   mov      flags3,dword ptr[post_idx1]
   fmulp    st(2),st
   ;  ar2      ar1
   fld      dword ptr tu[vc2]
   fmul     dword ptr [__asm_data + _SCALE_S]
   ;  tuadj1   ar2      ar1
   fld      dword ptr tv[vc2]
   fmul     dword ptr [__asm_data + _SCALE_T]
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
   and      flags3,BIT_22
   fld      dword ptr rhw[vc2]
   ;  rhw      tuadj    tvadj2
;area2      = replaces vb1
   mov      area2,dword ptr _area[esp]
   xor      last_flags,flags3
   fmul     st(1),st
   ;  rhw      tuadj    tvadj2

;header1    = replaces va1
   and      area2,80000000h
   mov      header1,STRIP_FOGW_CONTINUE
   fmulp    st(2),st
   ;  tuadj    tvadj
   xor      area2,dword ptr [__asm_data +_CULL_MASK]

   jz       culled_tri

   or       header1,last_flags
   mov      dword ptr [hwptr1],header1

   mov      specular2,dword ptr spec[vc2]
   mov      fog2,0ffh

   shr      specular2,24
   mov      last_flags,flags3

   sub      fog2,specular2
   mov      color2,dword ptr color[vc2]

   mov      dword ptr _fog[esp],fog2

   fild     dword ptr _fog[esp]
   ;  fog      tuadj    tvadj
   fld      dword ptr sz[vc2]
   ;  sz       fog      tuadj    tvadj
   fmul     dword ptr [__asm_data + _SCALEZ]
   ;  szadj    fog      tuadj    tvadj
   fld      dword ptr sy[vc2]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  syadj    szadj    fog      tuadj    tvadj
   fld      dword ptr sx[vc2]
   fadd     dword ptr [__asm_data + _PIXEL_CENTER]
   ;  sxadj    syadj    szadj    fog      tuadj    tvadj

;color2     =replaces header1
;rhw2       =replaces area2

   fstp     dword ptr [hwptr1+4]
   mov      rhw2,dword ptr rhw[vc2]
   fstp     dword ptr [hwptr1+8]
   mov      flags3,dword ptr[post_idx1+12]
   mov      dword ptr [hwptr1+12],color2
   mov      verts2,dword ptr _vertices[esp]
   fstp     dword ptr [hwptr1+16]
   mov      va2,flags3
   fstp     dword ptr [hwptr1+20]
   xor      last_flags,BIT_22
   add      post_idx1,12
   mov      dword ptr [hwptr1+24],rhw2
   fstp     dword ptr [hwptr1+28]
   and      va2,1fffffh
   fstp     dword ptr [hwptr1+32]

;count2     = replaces flags3
   add      hwptr1,36
   test     flags3,0C00000h
   mov      vb2,[post_idx1 +4]
   mov      count2,dword ptr _sub_count[esp]
   jz       ind_tri

   jmp      mesh_continue


culled_tri:
   add      post_idx1,12
   mov      va2,dword ptr [post_idx1]
   fstp     st(0)
   mov      count2,dword ptr _sub_count[esp]
   fstp     st(0)
   mov      verts2,dword ptr _vertices[esp]
   mov      vb2,[post_idx1 +4]
   and      va2,1fffffh

   jmp      ind_tri

align   16
fifo_check_mesh:
   push     post_idx1
   call     _fifo_make_room_asm
   pop      post_idx1
   jmp      mesh_begin

align   16
done_render:
;hwptr1      TEXTEQU     <esi>
hwptr_old   TEXTEQU     <eax>
room2       TEXTEQU     <ebx>
tmp2        TEXTEQU     <ecx>

   mov      hwptr_old,dword ptr[__asm_data + _FIFO_PTR]
   mov      dword ptr[__asm_data + _FIFO_PTR],hwptr1

   sub      hwptr1,hwptr_old
   mov      room2,dword ptr [__asm_data + _FIFO_ROOM]

   shr      hwptr1,2
   mov      tmp2,dword ptr _count[esp]

   sub      room2,hwptr1

   mov      dword ptr [__asm_data + _FIFO_ROOM],room2

   cmp      tmp2,0
   jg       prefetch_start

   add      esp,STACK_SIZE
   pop      esi
   pop      edi
   pop      ebx
   pop      ebp
   pop      esp

   ret

_DrawTriEdgeFAsm  ENDP

_TEXT   ENDS


END
