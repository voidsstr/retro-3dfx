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
IFDEF DTRIAPI 
EXTERN C _cacheMemTemp:DWORD

EXTERN _atrD3DFlushState:NEAR
ENDIF

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
   align 4
   .586

   PUBLIC  _atrD3DTransformVertices
   _atrD3DTransformVertices PROC NEAR

   push  ebp
   mov   ebp,esp
   push  ebx
   push  ecx
   push  edx
   push  edi
   push  esi
   sub   esp,8h             ;leave room for allIn and allOut

   ;    For not inline    allIn = ebp-1C
   ;                      allOut= ebp-18

   xor   ecx,ecx           ; clear flag reg
   mov   dword ptr[ebp-1Ch],0h   ; set AllIn
   mov   dword ptr[ebp-18h],0FFh   ; set AllOut

   mov   edx,dword ptr[ebp+0Ch]  ;src
   mov   esi,dword ptr[ebp+8h]  ;dest
   mov   edi,dword ptr[_atrRenderCache]

begin_loop:   
   cmp   edx,dword ptr[ebp+10h]
   jge   done_transform

   fld   dword ptr[edx]       ;src->x
   fmul  dword ptr[edi]       ;data[0]  a
   fld   dword ptr[edx+4h]    ;src->y
   fmul  dword ptr[edi+10h]   ;data[4]  b
   fld   dword ptr[edx+8h]    ;src->z
   fmul  dword ptr[edi+20h]   ;data[8]  c

   fxch  st(1)                ; b         c        a
   faddp st(2),st             ; c         a+b
   fld   dword ptr[edx]       ; ap1       c        a+b
   fmul  dword ptr[edi+4h]    ; a1        c        a+b
   fxch  st(1)                ; c         a1       a+b
   faddp st(2),st             ; a1        a+b+c
   fld   dword ptr[edx+4h]    ; bp1       a1       a+b+c
   fmul  dword ptr[edi+14h]   ; b1        a1       a+b+c
   fxch  st(2)                ; a+b+c     a1       b1
   fadd  dword ptr[edi+30h]   ; a+b+c+d   a1       b1
   fld   dword ptr[edx+8h]    ; cp1       a+b+c+d  a1       b1
   fmul  dword ptr[edi+24h]   ; c1        a+b+c+d  a1       b1
   fxch  st(1)                ; a+b+c+d   c1       a1       b1
   fstp  dword ptr[esi]       ; finally pop off dest->x
                              ; c         a        b
   fxch  st(1)                ; a         c        b
   faddp st(2),st             ; c         a+b
   fld   dword ptr[edx]       ; ap1       c        a+b
   fmul  dword ptr[edi+8h]    ; a1        c        a+b
   fxch  st(1)                ; c         a1       a+b
   faddp st(2),st             ; a1        a+b+c
   fld   dword ptr[edx+4h]    ; bp1       a1       a+b+c
   fmul  dword ptr[edi+18h]   ; b1        a1       a+b+c
   fxch  st(2)                ; a+b+c     a1       b1
   fadd  dword ptr[edi+34h]   ; a+b+c+d   a1       b1
   fld   dword ptr[edx+8h]    ; cp1       a+b+c+d  a1       b1
   fmul  dword ptr[edi+28h]   ; c1        a+b+c+d  a1       b1
   fxch  st(1)                ; a+b+c+d   c1       a1       b1
   fstp  dword ptr[esi+4h]    ; finally pop off dest->y
                              ; c1        a1       b1
   fxch  st(1)                ; a         c        b
   faddp st(2),st             ; c         a+b
   fadd  dword ptr[edi+38h]   ; c+d       a+b
                              ; got two cycles to stall
   xor   ecx,ecx              ; clear flag reg
   mov   ebx,dword ptr[esi]   ; mov into ebx dest->x

   faddp st(1),st             ; a+b+c
                              ; got another two cycles to stall
   and   ebx,7fffffffh        ; remove sign bit
                              ; one more cycle needed
   fst   dword ptr[esi+20h]   ; copy to dest->oow
   fsub  dword ptr[edi+50h]   ; sub dest->oow - atrRenderCache->nearClip
   fmul  dword ptr[one_over_far_minus_near]
   mov   eax,dword ptr[esi+20h]
   cmp   eax,dword ptr[edi+50h] ;cmp oow with near clip
   fstp  dword ptr[esi+18h]   ; pop off dest->ooz stall 1-2 cycles

   jle   w_le_0
   
   ; kick off divide
   fld1
   fdiv  dword ptr[esi+20h]
   
   cmp   eax,ebx
   jge   chk_y
   add   ecx,2h
   mov   ebx,dword ptr[esi]
   shr   ebx,1fh
   sub   ecx,ebx

chk_y:
   shl   ecx,2h
   mov   ebx,dword ptr[esi+4h]
   and   ebx,7fffffffh
   cmp   eax,ebx
   jge   chk_far
   add   ecx,2h
   mov   ebx,dword ptr[esi+4h]
   shr   ebx,1fh
   sub   ecx,ebx

chk_far:
   shl   ecx,1h
   cmp   eax,dword ptr[edi+54h]    ; compare with far_clip
   jle   chk_done
   add   ecx,1h

chk_done:
   and   dword ptr[ebp-18h],ecx
   or    dword ptr[ebp-1ch],ecx
   mov   dword ptr[esi+8h],ecx    ; set dest->flags
   test  ecx,ecx
   jnz   go_next_vert

   ; do some cache touching cause I got some cycles to spare
   mov   eax,dword ptr[edx+40h]
   mov   eax,dword ptr[edi+50h]

   fstp  dword ptr[esi+20h]       ; pop off result of 1/z
   fld   dword ptr[esi]          ; load up dest->x
   fmul  dword ptr[esi+20h]       ; multiply x by oow
   fld   dword ptr[esi+4h]        ; dest->y        dest->x*oow
   fmul  dword ptr[esi+20h]       ; dest->y*oow    dest->x*oow
   fxch                          ; dest->x*oow    dest->y*oow
   fmul  dword ptr[edi+40h]      ; x*oow*sx       y*oow
   fxch                          ; y*oow          x*oow*sx
   fmul  dword ptr[edi+48h]      ; y*oow*sy       x*oow*sx
   fxch                          ; x*oow*sx       y*oow*sy
   fadd  dword ptr[edi+44h]      ; x*oow*sx+ox    y*oow*sy
   fxch
   fadd  dword ptr[edi+4Ch]
   fxch
   fstp  dword ptr[esi]
   fstp  dword ptr[esi+4h]

   add   edx,40h
   add   esi,40h
   jmp   begin_loop

w_le_0:
   mov   ecx,20h
   mov   dword ptr[esi+8h],ecx
   or    dword ptr[ebp-1Ch],ecx
   add   esi,40h
   add   edx,40h
   jmp   begin_loop

go_next_vert:
   add   edx,40h
   add   esi,40h
   ; do some cache touching cause I got some cycles to spare
   mov   eax,dword ptr[edx+40h]
   mov   eax,dword ptr[edi+50h]
   
   fstp  st(0)
   jmp   begin_loop

done_transform:
   mov   eax,dword ptr[ebp-18h]
   shl   eax,8h
   or    eax,dword ptr[ebp-1Ch]
   add   esp,8h
   pop   esi
   pop   edi
   pop   edx
   pop   ecx
   pop   ebx
   pop   ebp
   ret
   _atrD3DTransformVertices ENDP


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

   align 4
   .586

IFDEF DTRIAPI
   PUBLIC RenderNode
   RenderNode PROC NEAR

   push  ebp
   mov   ebp,esp
   push  ebx
   push  ecx
   push  edi
   push  esi
   sub   esp,14h

   mov   ecx,dword ptr[d3dContext]
   fild  dword ptr[ecx+28h]               ; st(0) =d3dContext->any.caps.height
   fstp  dword ptr[ebp-14h]               ; save height at data[0]

   call  _atrD3DFlushState

   mov   esi,dword ptr[_atrRenderCache]
   add   esi,80h                          ; esi=_atrRenderCache->destVertex
   mov   edi,dword ptr[_cacheMemTemp]     ; verts for D3D
   mov   edx,dword ptr[esi]               ; touch destVertex
   mov   eax,dword ptr[edi]
   mov   eax,dword ptr[esi+20h]           ; touch 2nd part of destVertex
   mov   ecx,dword ptr[ebp+08h]
   mov   ecx,dword ptr[ecx]               ; ecx=numVerts

beginProcessVerts:
   test  ecx,ecx
   jz    endProcessVerts

   mov   eax,dword ptr[esi+40h]           ; touch next set of data
   mov   ebx,dword ptr[edi+20h]           ; ditto
   mov   edx,dword ptr[esi+60h]           ; ditto

   mov   eax,dword ptr[esi]               ; mov into eax a->x
   mov   edx,dword ptr[esi+18h]           ; mov into edx a->ooz

   mov   dword ptr[edi],eax               ; verts->sx=a->x
   mov   dword ptr[edi+8h],edx            ; verts->sz=a->ooz

   mov   eax,dword ptr[esi+20h]           ; eax=a->oow
   mov   dword ptr[edi+0ch],eax           ; verts->dvRHW=a->oow

   fld   dword ptr[ebp-14h]               ; load height from data[0]
   fsub  dword ptr[esi+4h]
   ; 3 clocks to go on subtract
   fld   dword ptr[esi+14h]               ; load up a->b
   fld   dword ptr[esi+10h]               ; load up a->g
   fld   dword ptr[esi+0ch]               ; load up a->r
   fld   dword ptr[esi+1ch]               ; load up a->a
   fxch  st(4)
   fstp  dword ptr[edi+4h]                ; store verts->sy=height-a->y
   fistp dword ptr[ebp-18h]               ; store r into data[1]
   fistp dword ptr[ebp-1ch]               ; store g into data[2]
   fistp dword ptr[ebp-20h]               ; store b into data[3]
   fistp dword ptr[ebp-24h]               ; store a into data[4]
   
   mov   eax,dword ptr[ebp-18h]           ; store r
   
   shl   eax,10h
   mov   edx,dword ptr[ebp-1ch]           ; store g

   shl   edx,8h
   mov   ebx,dword ptr[ebp-20h]           ; store b

   add   eax,edx
   mov   edx,dword ptr[ebp-24h]           ; store a

   add   eax,ebx
   mov   dword ptr[edi+14h],0h            ; verts->specular=0                           

   shl   edx,18h
   mov   ebx,dword ptr[esi+24h]           ; ebx=a->s0

   add   eax,edx
   mov   dword ptr[edi+18h],ebx           ; verts->tu=a->s0

   mov   dword ptr[edi+10h],eax           ; verts->color=argb
   mov   edx,dword ptr[esi+28h]           ; edx=a->t0

   add   esi,40h
   mov   dword ptr[edi+1ch],edx

   dec   ecx
   add   edi,20h
   jmp   beginProcessVerts

endProcessVerts:
   mov   ecx,dword ptr[ebp+8h]
   mov   esi,dword ptr[ecx+8h]            ; esi=n->connectivity
                                          ; note edi now equals tris[0] party of _cacheMemTemp
   push  ebp                              ; need the extra reg
   mov   dword ptr[ebp-18h],edi           ; save address of tris into data[1]
   xor   ebp,ebp

beginCountTris:
   mov   eax,dword ptr[esi]
   test  eax,eax
   jz    endCountTris
   
   mov   ecx,eax

   shr   eax,10h
   mov   edx,ecx

   and   eax,0ffh
   mov   ebx,ecx

   shr   edx,8h
   mov   word ptr[edi],ax

   and   edx,0ffh
   mov   word ptr[edi+6h],0700h

   and   ebx,0ffh
   mov   word ptr[edi+2h],dx

   add   esi,4h
   mov   word ptr[edi+4h],bx

   add   edi,8h
   inc   ebp      ; ? inc count as v pipeline, I think so not sure.
   
   jmp   beginCountTris

endCountTris:
   mov   ecx,ebp
   pop   ebp
   mov   edx,dword ptr[d3dContext]
   mov   eax,0700h
   push  eax                              ; push D3DTRIFLAG_EDGEENABLETRIANGLE
   push  ecx                              ; push numTri
   mov   eax,dword ptr[ebp-18h]           ; get tri address from data[1]
   push  eax                              ; push address of tris
   mov   ecx,dword ptr[ebp+08h]
   mov   ecx,dword ptr[ecx]               ; eax=numVerts
   push  ecx                              ; push numVerts
   mov   eax,dword ptr[_cacheMemTemp]
   push  eax                              ; push address of verts
   mov   ecx,dword ptr[edx+104h]
   push  ecx                              ; push d3dContext->lpView
   mov   eax,dword ptr[edx+100h]
   push  eax                              ; push d3dContext->lpDev

   mov   edx,dword ptr[eax]
   call  dword ptr[edx+5ch]
   add   esp,14h
   pop   esi
   pop   edi
   pop   ecx
   pop   ebx
   pop   ebp
   ret
   RenderNode ENDP

   PUBLIC _atrD3DDrawTriangle
   _atrD3DDrawTriangle PROC NEAR
   
   push  ebp
   mov   ebp,esp
   push  ebx
   push  ecx
   push  edi
   push  esi
   sub   esp,18h

   call  _atrD3DFlushState
   mov   edi,dword ptr[_cacheMemTemp]
   mov   esi,ebp
   add   esi,8h
   mov   dword ptr[ebp-14h],esi           ; save address of vert a
   
   mov   ecx,dword ptr[d3dContext]
   fild  dword ptr[ecx+28h]               ; st(0) =d3dContext->any.caps.height
   fstp  dword ptr[ebp-18h]               ; save height at data[1]

   mov   ecx,3

beginProcTriVerts:
   test  ecx,ecx
   jz    endProcTriVerts

   mov   esi,dword ptr[esi]               ; esi=vert
   mov   eax,dword ptr[esi]               ; mov into eax a->x
   mov   edx,dword ptr[esi+18h]           ; mov into edx a->ooz

   mov   dword ptr[edi],eax               ; verts->sx=a->x
   mov   dword ptr[edi+8h],edx            ; verts->sz=a->ooz

   mov   eax,dword ptr[esi+20h]           ; eax=a->oow
   mov   dword ptr[edi+0ch],eax           ; verts->dvRHW=a->oow

   fld   dword ptr[ebp-18h]               ; load height from data[1]
   fsub  dword ptr[esi+4h]
   ; 3 clocks to go on subtract
   fld   dword ptr[esi+14h]               ; load up a->b
   fld   dword ptr[esi+10h]               ; load up a->g
   fld   dword ptr[esi+0ch]               ; load up a->r
   fld   dword ptr[esi+1ch]               ; load up a->a
   fxch  st(4)
   fstp  dword ptr[edi+4h]                ; store verts->sy=height-a->y
   fistp dword ptr[ebp-1ch]               ; store r into data[2]
   fistp dword ptr[ebp-20h]               ; store g into data[3]
   fistp dword ptr[ebp-24h]               ; store b into data[4]
   fistp dword ptr[ebp-28h]               ; store a into data[5]
   
   mov   eax,dword ptr[ebp-1ch]           ; store r
   
   shl   eax,10h
   mov   edx,dword ptr[ebp-20h]           ; store g

   shl   edx,8h
   mov   ebx,dword ptr[ebp-24h]           ; store b

   add   eax,edx
   mov   edx,dword ptr[ebp-28h]           ; store a

   add   eax,ebx
   mov   dword ptr[edi+14h],0h            ; verts->specular=0                           

   shl   edx,18h
   mov   ebx,dword ptr[esi+24h]           ; ebx=a->s0

   add   eax,edx
   mov   dword ptr[edi+18h],ebx           ; verts->tu=a->s0

   mov   dword ptr[edi+10h],eax           ; verts->color=argb
   mov   edx,dword ptr[esi+28h]           ; edx=a->t0

   mov   esi,dword ptr[ebp-14h]           ; get address of argument from data[0]
   mov   dword ptr[edi+1ch],edx

   add   edi,20h
   dec   ecx

   add   esi,4h
   mov   dword ptr[ebp-14h],esi
   jmp   beginProcTriVerts

endProcTriVerts:
   mov   dword ptr[edi],10000h
   mov   dword ptr[edi+4],7000002h
   
   mov   edx,dword ptr[d3dContext]
   mov   eax,0700h
   push  eax                              ; push D3DTRIFLAG_EDGEENABLETRIANGLE
   mov   ecx,1h
   push  ecx                              ; push numTri
   push  edi                              ; push address of tris
   mov   ecx,3h
   push  ecx                              ; push numVerts
   mov   eax,dword ptr[_cacheMemTemp]
   push  eax                              ; push address of verts
   mov   ecx,dword ptr[edx+104h]
   push  ecx                              ; push d3dContext->lpView
   mov   eax,dword ptr[edx+100h]
   push  eax                              ; push d3dContext->lpDev

   mov   edx,dword ptr[eax]
   call  dword ptr[edx+5ch]

   add   esp,18h
   pop   esi
   pop   edi
   pop   ecx
   pop   ebx
   pop   ebp
   ret
   _atrD3DDrawTriangle ENDP

ELSE

   PUBLIC RenderNode
   RenderNode PROC NEAR
   push  ebp
   mov   ebp,esp
   push  ebx
   push  ecx
   push  edi
   push  esi
   sub   esp,1ch
   
   xor   eax,eax                 ; clear eax for num_tris
   mov   ebx,dword ptr[ebp+8h]   ; put n into ebx
   mov   ecx,dword ptr[ebx+8h]   ; put ecx = n->connectivity
   
begin_get_num_tris:
   cmp   dword ptr[ecx],0h
   je    end_add_tri
   inc   eax
   add   ecx,4h                  ;
   jmp   begin_get_num_tris

end_add_tri:
   mov   dword ptr[ebp-14h],eax  ; save num_tris into temp data 0
   shl   eax,3h
   add   eax,10h
   push  eax                     ; num_tris * sizeof(D3DTRIANGEL)
   push  4h
   mov   ecx,dword ptr[ebx]      ; ecx=n->numVertices
   push  ecx
   mov   ecx,dword ptr[d3dContext]  ; ecx=d3dContext
   push  ecx
   call  _atrD3DCheckBufferFull
   add   esp,10h
   
   mov   ecx,dword ptr[d3dContext]
   fild  dword ptr[ecx+28h]      ; st(0) =d3dContext->any.caps.height
   fstp  dword ptr[ebp-1Ch]      ; save height at data[2]
   add   ecx,0ba4h               ; ecx now equals ebd
   mov   dword ptr[ebp-18h],ecx  ; data[1] = ebd
   mov   eax,dword ptr[ecx+24h]  ; eax=ebd->numVertices
   mov   dword ptr[ebp-20h],eax  ; data[3] = ebd->numVertices
   mov   edi,dword ptr[ecx+4h]   ; edi=ebd->lpVertexPointer
   mov   ebx,dword ptr[ebp+8h]   ; put n into ebx
   mov   esi,dword ptr[ebx]      ; esi=n->numVertices
   xor   edx,edx                 ; clear counter
   mov   ebx,dword ptr[_atrRenderCache]
   add   ebx,80h                 ; ebx=_atrRenderCache->destVertex
   
begin_vert_setup:
   cmp   edx,esi
   je    end_vert_setup

   mov   eax,dword ptr[ebx]
   mov   dword ptr[edi],eax      ; verts->sx=a->x
   fld   dword ptr[ebp-1Ch]      ; load height from data[2]
   fsub  dword ptr[ebx+4h]       ; height-a->y
   fstp  dword ptr[edi+4]        ; verts->sy=height-a->y
   mov   eax,dword ptr[ebx+18h]  ; eax=a->ooz
   mov   dword ptr[edi+8h],eax   ; verts->sz=a->ooz
   mov   eax,dword ptr[ebx+20h]  ; eax=a->oow
   mov   dword ptr[edi+0ch],eax  ; verts->dvRHW=a->oow

   push  edx
   push  ebx

   fld   dword ptr[ebx+14h]         ; load a->b
   fld   dword ptr[ebx+10h]         ; load a->g
   fld   dword ptr[ebx+0ch]         ; load a->r
   fistp dword ptr[ebp-24h]         ; store in data[4]
   fistp dword ptr[ebp-28h]         ; store in data[5]
   fistp dword ptr[ebp-2ch]         ; store in data[6]

   mov   eax,dword ptr[ebp-24h]
   mov   edx,dword ptr[ebp-28h]

   shl   eax,10h
   mov   ebx,dword ptr[ebp-2ch]

   shl   edx,8h
   add   eax,ebx
   
   add   edx,0ff000000h
   add   eax,edx

   pop   ebx
   pop   edx

   mov   dword ptr[edi+10h],eax  ; verts->color=FFRGB

   mov   dword ptr[edi+14h],0h   ; verts->specular=0
   mov   eax,dword ptr[ebx+24h]  
   mov   dword ptr[edi+18h],eax  ; verts->tu=a->s0
   mov   eax,dword ptr[ebx+28h]  
   mov   dword ptr[edi+1ch],eax  ; verts->tv=a->t0
   add   edi,20h                 ; verts++
   add   ebx,40h                 ; a++
   inc   edx                     ; counter++
   jmp   begin_vert_setup
   
end_vert_setup:
   push  esi                     ;push n->numVertices
   push  0ah                     ;push 0xA
   mov   ebx,dword ptr[d3dContext]
   push  ebx                     ;push d3dContext
   call  _atrD3DProcessVertices
   add   esp,0ch

   mov   ebx,dword ptr[d3dContext]
   push  ebx                     ;push d3dContext
   call  _atrD3DCheckAlignment
   add   esp,4h

   mov   eax,dword ptr[ebp-14h]  ;move eax=num_tris data[0]
   push  eax
   push  8h                      ;sizeof D3DTRIANGLE
   push  3h                      ;sizoef D3DOP_TRIANGLE
   mov   ebx,dword ptr[d3dContext]
   push  ebx                     ;push d3dContext
   call  _atrD3DInstruction
   add   esp,10h

   mov   eax,dword ptr[ebp-18h]  ;eax=ebd
   mov   ebx,dword ptr[eax+0ch]  ;ebx=ebd->lpInstPointer,tri_pt
   mov   ecx,dword ptr[ebp+8h]   ;ecx=n
   mov   ecx,dword ptr[ecx+8h]   ;ecx=n->connectivity
   mov   edx,dword ptr[ebp-20h]  ;edx=data[3] ,vnum

begin_loop_3:
   cmp   dword ptr[ecx],0h       ;cmp *connectivity
   je    end_loop3
   
   mov   eax,dword ptr[ecx]      ;eax= *connectivity
   shr   eax,10h
   and   eax,0ffh
   add   eax,edx                 ;eax=vnum+((*connectivity)>>16)&0xff
   mov   word ptr[ebx],ax
   
   mov   eax,dword ptr[ecx]
   shr   eax,8h
   and   eax,0ffh
   add   eax,edx                 ;eax=vnum+((*connectivity)>>8)&0xff
   mov   word ptr[ebx+02h],ax

   mov   eax,dword ptr[ecx]
   and   eax,0ffh
   add   eax,edx                 ;eax=vnum+(*connectivity)&0xff
   mov   word ptr[ebx+04h],ax

   mov   word ptr[ebx+06h],700h

   add   ebx,8h
   add   ecx,4h
   jmp   begin_loop_3

end_loop3:
   mov   eax,dword ptr[ebp-18h]  ;eax=ebd
   mov   dword ptr[eax+0ch],ebx  ;ebx=ebd->lpInstPointer

   ;stat section
   mov   eax,dword ptr[ebp-14h]  ;eax=num_tris
   and   eax,0ffffh
   push  eax
   call  _atrStatsIncTris
   add   esp,4h

   add   esp,1ch
   pop   esi
   pop   edi
   pop   ecx
   pop   ebx
   pop   ebp
   ret
   RenderNode ENDP
ENDIF

_TEXT ENDS
END
