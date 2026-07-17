TITLE  d3d.asm

.586P
.model FLAT, C

; Coddle Watcom compiling with stack based convention
IFDEF __SW_3S
_atrRenderCache TEXTEQU <atrRenderCache>
ENDIF

;; External Data Declarations
EXTERN C _atrRenderCache:DWORD
EXTERN C _atrD3D_ZSCALE:DWORD
EXTERN C d3dContext:DWORD
EXTERN C one_over_far_minus_near:DWORD
EXTERN C _atr_src_size:DWORD
EXTERN C _atr_src_xyz_offset:DWORD
EXTERN C _atr_dst_size:DWORD
EXTERN C _atr_dst_x_offset:DWORD
EXTERN C _atr_dst_y_offset:DWORD
EXTERN C _atr_dst_z_offset:DWORD
EXTERN C _atr_dst_oow_offset:DWORD
EXTERN C _atr_dst_flags_offset:DWORD
EXTERN C _ONE:DWORD
EXTERN C _atrTempVertSpace:DWORD


EXTERN _atrD3DCheckBufferFull:NEAR
EXTERN _atrD3DProcessVertices:NEAR
EXTERN _atrD3DCheckAlignment:NEAR
EXTERN _atrD3DInstruction:NEAR
EXTERN _atrStatsIncTris:NEAR

CC_BEFORE   TEXTEQU <00000020h>

_TEXT		SEGMENT

;;
;; void a_atrTransformVertices( _AtrDestVertex *dest, 
;;                              AtrVertex *src, 
;;                              AtrVertex *end )
;; 
;; Transform a vertex into clip space, do clip testing, if
;; unclipped, then project into screen space, else store
;; clip code for vertex in flags field.
;; 

atr_src_x_offset TEXTEQU   <0>
atr_src_y_offset TEXTEQU   <4>
atr_src_z_offset TEXTEQU   <8>

tran_temp2 TEXTEQU         <-36>
tran_temp TEXTEQU          <-32>
tran_xflags TEXTEQU        <-28>
tran_yflags TEXTEQU        <-24>
tran_clip_flags TEXTEQU    <-20>
tran_ret_value TEXTEQU     <-16>
tran_dest TEXTEQU          <8>
tran_src TEXTEQU           <12>
tran_end TEXTEQU           <16>

   align 4
   .586

   PUBLIC atr_d3d_transform_verts_asm
   atr_d3d_transform_verts_asm PROC NEAR

   push  ebp
   mov   ebp,esp
   push  edi
   push  esi
   push  ebx
   sub   esp,18h
   
   ; esi is source vertex
   ; edi is dest vertex 
   ; edx is _atrRenderCache 
   ; ecx is source xyz offset 

   mov   esi,dword ptr[ebp+tran_src]
   mov   edi,dword ptr[ebp+tran_dest]
   mov   edx,_atrRenderCache
   mov   dword ptr[ebp+tran_ret_value],0ffffffffh
   mov   dword ptr[ebp+tran_xflags],0ffffffffh
   mov   dword ptr[ebp+tran_yflags],0ffffffffh
   mov   dword ptr[ebp+tran_temp2],0h

next_vertex_andy:
   cmp   esi,dword ptr[ebp+tran_end]
   jz    done_transform

   mov   ecx,dword ptr[_atr_src_xyz_offset]     ;ecx = source xyz offset

   fld   dword ptr[esi+ecx+atr_src_x_offset]    ;load source x
   fmul  dword ptr[edx]
   fld   dword ptr[esi+ecx+atr_src_x_offset]    ;load source x
   fmul  dword ptr[edx+4h]
   fld   dword ptr[esi+ecx+atr_src_x_offset]    ;load source x
   fmul  dword ptr[edx+8h]

   fld   dword ptr[esi+ecx+atr_src_y_offset]    ;load source y
   fmul  dword ptr[edx+10h]
   fld   dword ptr[esi+ecx+atr_src_y_offset]    ;load source y
   fmul  dword ptr[edx+14h]
   fld   dword ptr[esi+ecx+atr_src_y_offset]    ;load source y
   fmul  dword ptr[edx+18h]
   
   fxch  st(2)
   faddp st(5),st
   faddp st(3),st
   faddp st(1),st

   fld   dword ptr[esi+ecx+atr_src_z_offset]    ;load source z
   fmul  dword ptr[edx+20h]
   fld   dword ptr[esi+ecx+atr_src_z_offset]    ;load source z
   fmul  dword ptr[edx+24h]
   fld   dword ptr[esi+ecx+atr_src_z_offset]    ;load source z
   fmul  dword ptr[edx+28h]

   fxch  st(2)
   faddp st(5),st
   faddp st(3),st
   faddp st(1),st

   fxch  st(2)
   fadd  dword ptr[edx+30h]
   fxch  st(1)
   fadd  dword ptr[edx+34h]
   fxch  st(2)
   fadd  dword ptr[edx+38h]
   fxch

   mov   eax,dword ptr[_atr_dst_x_offset]
   mov   ebx,dword ptr[_atr_dst_y_offset]

   fstp  dword ptr[edi+eax]                     ;store dest->x
   fxch
   fstp  dword ptr[edi+ebx]                     ;store dest->y

   mov   ecx,dword ptr[_atr_dst_oow_offset]
   mov   ebx,dword ptr[_atr_dst_z_offset]

   fst   dword ptr[edi+ecx]                     ;store z into dest->oow 
   fsub  dword ptr[edx+50h]
   fmul  dword ptr[one_over_far_minus_near]
   fstp  dword ptr[edi+ebx]                     ;store scaled dest->z

   mov   dword ptr[ebp+tran_clip_flags],0       ;set clip flags to 0
   mov   eax,dword ptr[edx+50h]                 ;move near clip into eax

   mov   ebx,dword ptr[edi+ecx]                 ;ebx = z
   cmp   ebx,eax
   jle   w_le_0_andy

   fld   _ONE
   fdiv  dword ptr[edi+ecx]
   
   ; check x
   mov   ecx,dword ptr[_atr_dst_x_offset]
   mov   eax,dword ptr[edi+ecx]
   and   eax,7fffffffh

   cmp   ebx,eax
   jl    x_fail

   ; check y
   mov   ecx,dword ptr[_atr_dst_y_offset]
   mov   eax,dword ptr[edi+ecx]
   and   eax,7fffffffh

   cmp   ebx,eax
   jl    y_fail

   ; check far
   mov   eax,dword ptr[edx+54h]
   cmp   ebx,eax
   jg    far_fail

   mov   eax,dword ptr[_atr_dst_flags_offset]
   mov   ebx,dword ptr[_atr_dst_oow_offset]
   
   mov   dword ptr[edi+eax],0h      ; set flags=0 since not clipped

   mov   eax,dword ptr[_atr_dst_x_offset]
   mov   ecx,dword ptr[_atr_dst_y_offset]

   ; convert to screen space wasn't clipped
   fstp  dword ptr[edi+ebx]         ; store off 1/z
   fld   dword ptr[edi+eax]         ; load dest->x
   fmul  dword ptr[edi+ebx]
   fld   dword ptr[edi+ecx]         ; load dest->y
   fmul  dword ptr[edi+ebx]
   fxch
   fmul  dword ptr[edx+40h]
   fxch
   fmul  dword ptr[edx+48h]
   fxch
   fadd  dword ptr[edx+44h]
   fxch
   fadd  dword ptr[edx+4ch]
   fxch
   fstp  dword ptr[edi+eax]
   fstp  dword ptr[edi+ecx]

   mov   dword ptr[ebp+tran_ret_value],0
   add   edi,dword ptr[_atr_dst_size]
   add   esi,dword ptr[_atr_src_size]
   jmp   next_vertex_andy

w_le_0_andy:
   and   dword ptr[ebp+tran_ret_value],1h
   mov   eax,dword ptr[_atr_dst_flags_offset]
   mov   dword ptr[edi+eax],20h                 ; set clip before flag
   add   edi,dword ptr[_atr_dst_size]
   add   esi,dword ptr[_atr_src_size]
   jmp   next_vertex_andy

x_fail:
   ;ecx should already be setup with x offset
   mov   dword ptr[ebp+tran_temp],2h
   mov   eax,dword ptr[edi+ecx]
   shr   eax,1fh

   sub   dword ptr[ebp+tran_temp],eax
   mov   eax,dword ptr[ebp+tran_temp]
   or    dword ptr[ebp+tran_clip_flags],eax
   and   dword ptr[ebp+tran_xflags],eax
   
   shl   dword ptr[ebp+tran_clip_flags],2h

   ; begin y check
   mov   ecx,dword ptr[_atr_dst_y_offset]
   mov   eax,dword ptr[edi+ecx]

   and   eax,7fffffffh
   cmp   ebx,eax
   jge   chk_far_fail

y_fail:
   ;ecx should already be setup with y offset
   mov   dword ptr[ebp+tran_temp],2h
   mov   eax,dword ptr[edi+ecx]
   shr   eax,1fh

   sub   dword ptr[ebp+tran_temp],eax
   mov   eax,dword ptr[ebp+tran_temp]
   or    dword ptr[ebp+tran_clip_flags],eax
   and   dword ptr[ebp+tran_yflags],eax

chk_far_fail:
   shl   dword ptr[ebp+tran_clip_flags],1h
   mov   eax,dword ptr[edx+54h]              ; move far clip into eax
   
   cmp   ebx,eax
   jle   chk_fail_done

   ; no far plane mesh rejection yet.
far_fail:
   or    dword ptr[ebp+tran_clip_flags],1h

chk_fail_done:
   mov   eax,dword ptr[_atr_dst_flags_offset]
   mov   ecx,dword ptr[ebp+tran_clip_flags]
   mov   dword ptr[edi+eax],ecx
   add   edi,dword ptr[_atr_dst_size]
   add   esi,dword ptr[_atr_src_size]
   mov   dword ptr[ebp+tran_temp2],1h
   fstp  st(0)
   jmp   next_vertex_andy

done_transform:
   rol   dword ptr[ebp+tran_xflags],1h
   rol   dword ptr[ebp+tran_yflags],3h
   mov   ebx,dword ptr[ebp+tran_xflags]
   and   ebx,dword ptr[ebp+tran_yflags]
   and   ebx,dword ptr[ebp+tran_ret_value]
   test  ebx,0ffffffffh
   jnz   mesh_rejected
   mov   eax,dword ptr[ebp+tran_temp2]
   
   add   esp,18h
   pop   ebx
   pop   esi
   pop   edi
   pop   ebp
   ret

mesh_rejected:
   mov   eax,80000000h
   add   esp,18h
   pop   ebx
   pop   esi
   pop   edi
   pop   ebp
   ret
   atr_d3d_transform_verts_asm ENDP

   align 4
   .586

node_ptr TEXTEQU              <8>
node_num_vert_offset TEXTEQU  <0>
node_temp1 TEXTEQU            <-16>
node_temp2 TEXTEQU            <-20>
node_temp3 TEXTEQU            <-24>
node_temp4 TEXTEQU            <-28>
node_count TEXTEQU            <-32>

node_src_x_offset TEXTEQU     <0>
node_src_y_offset TEXTEQU     <4>
node_src_z_offset TEXTEQU     <24>
node_src_oow_offset TEXTEQU   <32>
node_src_tx_offset TEXTEQU    <36>
node_src_ty_offset TEXTEQU    <40>
node_src_r_offset TEXTEQU     <12>
node_src_g_offset TEXTEQU     <16>
node_src_b_offset TEXTEQU     <20>

node_dest_vert_offset TEXTEQU <80h>
node_dest_vert_size TEXTEQU   <40h>

node_dest_x_offset TEXTEQU    <0>
node_dest_y_offset TEXTEQU    <4>
node_dest_z_offset TEXTEQU    <8>
node_dest_oow_offset TEXTEQU  <12>
node_dest_argb_offset TEXTEQU <16>
node_dest_spec_offset TEXTEQU <20>
node_dest_tx_offset TEXTEQU   <24>
node_dest_ty_offset TEXTEQU   <28>

   PUBLIC  render_node_asm
   render_node_asm PROC NEAR

   push  ebp
   mov   ebp,esp
   push  edi
   push  esi
   push  ebx
   sub   esp,14h
   
   mov   esi,dword ptr[ebp+node_ptr]
   mov   edi,dword ptr[_atrRenderCache]
   add   edi,80h
   lea   ecx,dword ptr[_atrTempVertSpace]
   mov   dword ptr[ebp+node_count],0
   mov   eax,dword ptr[d3dContext]
   fild  dword ptr[eax+38h]

node_vert_copy:
   mov   eax,dword ptr[ebp+node_count]
   cmp   dword ptr[esi+node_num_vert_offset],eax
   jbe   node_vert_copy_done

   mov   eax,dword ptr[edi+node_src_x_offset]
   fst   st(1)
   fsub  dword ptr[edi+node_src_y_offset]

   mov   dword ptr[ecx+node_dest_x_offset],eax
   mov   dword ptr[ecx+node_dest_spec_offset],0

   mov   eax,dword ptr[edi+node_src_tx_offset]
   mov   ebx,dword ptr[edi+node_src_ty_offset]

   mov   dword ptr[ecx+node_dest_tx_offset],eax
   mov   dword ptr[ecx+node_dest_ty_offset],ebx
   
   mov   eax,dword ptr[edi+node_src_z_offset]
   mov   ebx,dword ptr[edi+node_src_oow_offset]

   fstp  dword ptr[ecx+node_dest_y_offset]

   mov   dword ptr[ecx+node_dest_z_offset],eax
   mov   dword ptr[ecx+node_dest_oow_offset],ebx

   fld   dword ptr[edi+node_src_r_offset]
   fld   dword ptr[edi+node_src_g_offset]
   fld   dword ptr[edi+node_src_b_offset]

   fistp dword ptr[ebp+node_temp1]
   fistp dword ptr[ebp+node_temp2]
   fistp dword ptr[ebp+node_temp3]

   shl   dword ptr[ebp+node_temp1],10h
   shl   dword ptr[ebp+node_temp2],8h

   mov   eax,0ff000000h
   or    eax,dword ptr[ebp+node_temp3]

   or    eax,dword ptr[ebp+node_temp1]
   or    eax,dword ptr[ebp+node_temp2]

   mov   dword ptr[ecx+node_dest_argb_offset],eax

   add   ecx,20h
   add   edi,40h
   inc   dword ptr[ebp+node_count]
   jmp   node_vert_copy

node_vert_copy_done:
   
   fstp  st(0)
   mov   dword ptr[ebp+node_temp1],ecx    ;save off location of index list
   mov   edi,ecx

   xor   eax,eax
   xor   ebx,ebx
   xor   ecx,ecx
   xor   edx,edx
   
   mov   esi,dword ptr[esi+8h]

node_index_list_setup:
   test  dword ptr[esi],0ffffffffh
   jz    node_done_index_list

   mov   al,byte ptr[esi]
   mov   bl,byte ptr[esi+1]
   mov   cl,byte ptr[esi+2]
   mov   word ptr[edi+4],ax
   mov   word ptr[edi+2],bx
   mov   word ptr[edi],cx
   inc   edx
   add   esi,4h
   add   edi,6h
   jmp   node_index_list_setup

node_done_index_list:

   ;edx contains num tris

   push  0Ch
   lea   edx,dword ptr[edx+edx*2]
   push  edx
   mov   eax,dword ptr[ebp+node_temp1]
   push  eax

   
   mov   eax,dword ptr[ebp+node_ptr]         ;push num verts
   mov   eax,dword ptr[eax]
   push  eax
   lea   eax,dword ptr[_atrTempVertSpace]
   push  eax                                 ;push verts
   push  3h
   push  4h
   
   mov   eax,dword ptr[d3dContext]
   mov   eax,dword ptr[eax+11Ch]
   push  eax

   mov   eax,dword ptr[eax]
   call  dword ptr[eax+78h]

   add   esp,14h
   pop   ebx
   pop   esi
   pop   edi
   pop   ebp
   ret
   render_node_asm ENDP

   align 4
   .586

   PUBLIC  RGBA_MAKE_ASM
   RGBA_MAKE_ASM PROC NEAR
   mov   eax,dword ptr[esp+4h]
   push  ebx
   
   sub   esp,10h
   fld   dword ptr[eax+1ch]
   fld   dword ptr[eax+14h]
   fld   dword ptr[eax+10h]
   fld   dword ptr[eax+0ch]
   fistp dword ptr[esp]
   fistp dword ptr[esp+4h]
   fistp dword ptr[esp+8h]
   fistp dword ptr[esp+0ch]

   mov   ebx,dword ptr[esp]
   mov   eax,dword ptr[esp+4h]

   shl   ebx,10h
   mov   edx,dword ptr[esp+8h]

   shl   eax,8h

   add   eax,ebx
   mov   ebx,dword ptr[esp+0ch]

   add   eax,edx                 ;eax=00rgb

   shl   ebx,18h

   add   eax,ebx

   add   esp,10h
   pop   ebx
   ret
   RGBA_MAKE_ASM ENDP

   align 4
   .586

   PUBLIC RGB_MAKE_ASM
   RGB_MAKE_ASM PROC NEAR
   mov   eax,dword ptr[esp+4h]
   push  ebx
   sub   esp,0ch
   fld   dword ptr[eax+14h]
   fld   dword ptr[eax+10h]
   fld   dword ptr[eax+0ch]
   fistp dword ptr[esp]
   fistp dword ptr[esp+4h]
   fistp dword ptr[esp+8h]

   mov   eax,dword ptr[esp]
   mov   edx,dword ptr[esp+4h]

   shl   eax,10h
   mov   ebx,dword ptr[esp+8h]

   shl   edx,8h
   add   eax,ebx

   add   eax,edx

   add   esp,0ch
   pop   ebx
   ret
   RGB_MAKE_ASM ENDP

_TEXT ENDS
END
