TITLE  xatr.asm

.586P
.model FLAT, C

; Coddle Watcom compiling with stack based convention
IFDEF __SW_3S
_atrRenderCache TEXTEQU <atrRenderCache>
_atrLightHead   TEXTEQU <atrLightHead>
ENDIF

;; External Data Declarations
EXTERN C _atrRenderCache:DWORD
EXTERN C _atrLightHead:DWORD

EXTERN grDrawTriangle@12:NEAR


; some useful floating load and store macros <ala gmt>
flds    TEXTEQU <fld  DWORD PTR>
fmuls   TEXTEQU <fmul DWORD PTR>
fadds   TEXTEQU <fadd DWORD PTR>
fsubs   TEXTEQU <fsub DWORD PTR>
fstsp   TEXTEQU <fstp DWORD PTR>
fsts    TEXTEQU <fst  DWORD PTR>
fdivs   TEXTEQU <fdiv DWORD PTR>

DWP     TEXTEQU <DWORD PTR>

; constants

CC_BEHIND   TEXTEQU <00000001h>
CC_BELOW    TEXTEQU <00000002h>
CC_ABOVE    TEXTEQU <00000004h>
CC_LEFT     TEXTEQU <00000008h>
CC_RIGHT    TEXTEQU <00000010h>
CC_BEFORE   TEXTEQU <00000020h>

; Data Storage
_DATA   SEGMENT
_ONE    DD  03f800000r      ;1.0f - 'cause its faster to load it from
                            ;       memory than to generate it...    
_BIAS   DD  049000000r      ;(float)(1<<19)
_FACTOR DD  ?               ; Storage for lighting factor
_DATA   ENDS

_TEXT           SEGMENT
;;
;;  void __cdecl fpu_single_precision( void )
;;     set the fpu control word to strict single precision
;;

        align 4
    PUBLIC  fpu_single_precision           ; this routine sets the precision to single
    fpu_single_precision PROC NEAR         ; which effects all adds, mults, and divs

        .586
        push    eax                                             ; make room
        fnclex                                                  ; clear pending exceptions      
        fstcw   WORD PTR [esp]
        mov             eax, DWORD PTR [esp]
        and             eax, 0000fcffh                  ; clear bits 9:8
        mov             DWORD PTR [esp], eax
        fldcw   WORD PTR [esp]
        pop             eax
        ret     0
    fpu_single_precision ENDP


   PUBLIC  _asm_TransformVertices
   _asm_TransformVertices PROC NEAR
   push  ebp
   mov   ebp,esp
   sub   esp,4h
   push  ebx
   push  ecx
   push  edx
   push  edi
   push  esi
   
   mov   esi,dword ptr[ebp+0ch]  ;esi=src
   mov   edi,dword ptr[ebp+8h]   ;edi=dest
   mov   edx,_atrRenderCache
   mov   dword ptr[ebp-14h],0
   
next_vertex_andy:
   cmp   esi,dword ptr[ebp+10h]  ;cmp src<end
   jz    done_transform

   fld   dword ptr[esi]
   fmul  dword ptr[edx]
   fld   dword ptr[esi]
   fmul  dword ptr[edx+4h]
   fld   dword ptr[esi]
   fmul  dword ptr[edx+8h]

   fld   dword ptr[esi+4h]
   fmul  dword ptr[edx+10h]
   fld   dword ptr[esi+4h]
   fmul  dword ptr[edx+14h]
   fld   dword ptr[esi+4h]
   fmul  dword ptr[edx+18h]
   
   fxch  st(2)
   faddp st(5),st
   faddp st(3),st
   faddp st(1),st

   fld   dword ptr[esi+8h]
   fmul  dword ptr[edx+20h]
   fld   dword ptr[esi+8h]
   fmul  dword ptr[edx+24h]
   fld   dword ptr[esi+8h]
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

   fstp  dword ptr[edi]          ;store dest->x
   fxch
   fstp  dword ptr[edi+4h]       ;store dest->y
   fstp  dword ptr[edi+20h]      ;store dest->oow

   xor   ecx,ecx                 ;clear clip code
   mov   eax,dword ptr[edx+50h]  ;move near clip into ebx

   mov   ebx,dword ptr[edi+20h]  ;ebx = z
   cmp   ebx,eax
   jle   w_le_0_andy

   fld   _ONE
   fdiv  dword ptr[edi+20h]
   
   ; check x
      ; murali fetches next vert here probably couldn't hurt
   mov   eax,dword ptr[esi+40h]
   mov   eax,dword ptr[edi]
   and   eax,7fffffffh

   cmp   ebx,eax
   jl    x_fail

   ; check y
   mov   eax,dword ptr[edi+4h]
   and   eax,7fffffffh

   cmp   ebx,eax
   jl    y_fail

   ; check far
   mov   eax,dword ptr[edx+54h]
   cmp   ebx,eax
   jg    chk_far_fail

   mov   dword ptr[edi+8h],0h    ; set flags=0 since not clipped
   ; convert to screen space wasn't clipped
   fstp  dword ptr[edi+20h]
   fld   dword ptr[edi]
   fmul  dword ptr[edi+20h]
   fld   dword ptr[edi+4h]
   fmul  dword ptr[edi+20h]
   fxch
   fmul  dword ptr[edx+40h]
   fxch
   fmul  dword ptr[edx+48h]
   fxch
   fadd  dword ptr[edx+44h]
   fxch
   fadd  dword ptr[edx+4ch]
   fxch
   fsub  _BIAS
   fxch  
   fsub  _BIAS
   fxch
   fstp  dword ptr[edi]
   fstp  dword ptr[edi+4h]

   add   edi,40h
   add   esi,40h
   jmp   next_vertex_andy

w_le_0_andy:
   mov   dword ptr[edi+8h],20h      ; set clip before flag
   add   edi,40h
   add   esi,40h
   add   dword ptr[ebp-14h],1h
   jmp   next_vertex_andy

x_fail:
   add   ecx,2h
   mov   eax,dword ptr[edi]

   shr   eax,1fh
   add   dword ptr[ebp-14h],100h
   test  eax,1h
   jz    x_signed
   sub   dword ptr[ebp-14h],100h
   add   dword ptr[ebp-14h],10000h

x_signed:
   sub   ecx,eax

   shl   ecx,2h                     ; begin y check
   mov   eax,dword ptr[edi+4h]

   and   eax,7fffffffh
   cmp   ebx,eax
   jge   chk_far_fail

y_fail:
   add   ecx,2h
   mov   eax,dword ptr[edi+4h]

   shr   eax,1fh
   sub   ecx,eax

chk_far_fail:
   shl   ecx,1h
   mov   eax,dword ptr[edx+54h]  ; move far clip into eax
   
   cmp   ebx,eax
   jle   chk_fail_done
   add   dword ptr[ebp-14h],1000000h
   add   ecx,1h

chk_fail_done:
   mov   dword ptr[edi+8h],ecx
   add   edi,40h
   add   esi,40h
   or    dword ptr[ebp-14h],80000000h
   fstp  st(0)
   jmp   next_vertex_andy

done_transform:
   pop   esi
   pop   edi
   pop   edx
   pop   ecx
   pop   ebx
   add   esp,4h
   mov   eax,dword ptr[ebp-14h]
   pop   ebp
   ret
   _asm_TransformVertices ENDP

        align 4
        .586

   PUBLIC  AndyRenderNode
   AndyRenderNode PROC NEAR
   push  ebx
   mov   ebx,dword ptr[esp+8h]
   push  ecx
   push  edx
   push  edi

   mov   edi,_atrRenderCache
   add   edi,80h

   mov   eax,dword ptr[ebx]

next_tri:
   test  eax,eax
   jz    done_con

   xor   eax,eax
   xor   ecx,ecx
   xor   edx,edx
   mov   al,byte ptr[ebx]
   mov   cl,byte ptr[ebx+1]
   mov   dl,byte ptr[ebx+2]
   shl   eax,6h
   shl   ecx,6h
   shl   edx,6h
   add   eax,edi
   add   ecx,edi
   add   edx,edi
   push  eax
   push  ecx
   push  edx
   call  grDrawTriangle@12
;   add   esp,0ch
   add   ebx,4h
   mov   eax,dword ptr[ebx]
   jmp   next_tri

done_con:
   pop   edi
   pop   edx
   pop   ecx
   pop   ebx
   ret

   AndyRenderNode ENDP

;;
;; void a_atrIRGBSRC_LIGHTING_DIFFUSE( _AtrDestVertex *dest, 
;;                                     AtrVertex *src )
;;
;;
        align 4
        .586

    PUBLIC  a_atrIRGBSRC_LIGHTING_DIFFUSE
    a_atrIRGBSRC_LIGHTING_DIFFUSE PROC NEAR

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Structure offset definitions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; RenderCache Element Offsets
m00  = 0
m01  = 4
m02  = 8
m03  = 12
m10  = 16
m11  = 20
m12  = 24
m13  = 28
m20  = 32
m21  = 36
m22  = 40
m23  = 44
m30  = 48
m31  = 52
m32  = 56
m33  = 60
xs   = 64
xo   = 68
ys   = 72
yo   = 76
nc   = 80
fc   = 84
mbr  = 88
mbg  = 92
mbb  = 96
tff  = 100

;; DestVertex Element Offsets
d_x    = 0
d_y    = 4
d_flag = 8
pad    = 12
d_oow  = 16
d_r    = 20
d_g    = 24
d_b    = 28

;; SrcVertex Element Offsets
s_x    = 0
s_y    = 4
s_z    = 8
s_i    = 12
s_j    = 16
s_k    = 20
s_next = 64

;; LightNode Element Offsets
l_vf0  = 0
l_vf1  = 4
l_vf2  = 8
l_lgt  = 12
l_tag  = 92
l_lp0  = 96
l_lp1  = 100
l_lp2  = 104
l_lcr  = 108
l_lcg  = 112
l_lcb  = 116
l_next = 120

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;     STARTUP
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; SrcVertex Element Offsets
    push ebx                  ; 1 - save ebx 
    push esi                  ; 2 - save esi

;; The required preserved registers appears to be misdocumented in
;; the MASM manual.  This convention does seem to be consistent
;; between code generated by msvc and watcom, though.

;; Stack Offsets
;;
;;  src     - ESP + 16
;;  dest    - ESP + 12
;;  retaddr - ESP + 8
;;  ebx     - ESP + 4
;;  esi     - ESP
_src  = 16
_dest = 12

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   Light
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Register Assignments
dest  TEXTEQU   <eax>
cur_l TEXTEQU   <ebx>
src   TEXTEQU   <ecx>
tmp   TEXTEQU   <ecx>
rc    TEXTEQU   <edx>

    mov cur_l, _atrLightHead        ;
    mov src, _src[esp]              ;
    mov dest, _dest[esp]            ;

    flds mbr[rc]                    ;  r 
    flds mbg[rc]                    ;  r  g
    flds mbb[rc]                    ;  r  g  b

    test cur_l, cur_l               ;
    jz   done                       ;

    flds    l_vf0[cur_l]            ;  r  g  b    vf0
    fmuls   s_i[src]                ;  r  g  b   ivf0
    flds    l_vf1[cur_l]            ;  r  g  b   ivf0  vf1
    fmuls   s_j[src]                ;  r  g  b   ivf0  ivf1
    flds    l_vf2[cur_l]            ;  r  g  b   ivf0  ivf1   vf2
    fmuls   s_k[src]                ;  r  g  b   ivf0  ivf1  ivf2
    faddp   st(2),st                ;  r  g  b  ivf02  ivf1
    faddp   st(1),st                ;  r  g  b ivf012
    fstp    _FACTOR                 ;  r  g  b

    mov     esi, _FACTOR            ;
    test    esi, esi
    js      next
    
    flds    l_lcr[cur_l]            ;  r  g  b  lcr
    fmuls   _FACTOR                 ;  r  g  b flcr

    flds    l_lcg[cur_l]            ;  r  g  b flcr  lcg
    fmuls   _FACTOR                 ;  r  g  b flcr flcg

    flds    l_lcb[cur_l]            ;  r  g  b flcr flcg  lcb
    fmuls   _FACTOR                 ;  r  g  b flcr flcg flcb

    fxch    st(2)                   ;  r  g  b flcb flcg flcr
    faddp   st(5),st                ; ar  g  b flcb flcg
    faddp   st(3),st                ; ar ag  b flcb
    faddp   st(1),st                ; ar ag ab

next:        
    mov     cur_l, l_next[cur_l]    ; l = l->next
    test    cur_l, cur_l
    jnz     again


done:
    fstsp d_b[dest]
    fstsp d_g[dest]
    fstsp d_r[dest]
    pop esi
    pop ebx                         ; restore state
    ret             

again:
    flds    l_vf0[cur_l]            ;  r  g  b    vf0
    fmuls   s_i[src]                ;  r  g  b   ivf0
    flds    l_vf1[cur_l]            ;  r  g  b   ivf0  vf1
    fmuls   s_j[src]                ;  r  g  b   ivf0  ivf1
    flds    l_vf2[cur_l]            ;  r  g  b   ivf0  ivf1   vf2
    fmuls   s_k[src]                ;  r  g  b   ivf0  ivf1  ivf2
    faddp   st(2),st                ;  r  g  b  ivf02  ivf1
    faddp   st(1),st                ;  r  g  b ivf012
    fstp    _FACTOR                 ;  r  g  b

    mov     esi, _FACTOR            ;
    test    esi, esi
    js      next
    
    flds    l_lcr[cur_l]            ;  r  g  b  lcr
    fmuls   _FACTOR                 ;  r  g  b flcr

    flds    l_lcg[cur_l]            ;  r  g  b flcr  lcg
    fmuls   _FACTOR                 ;  r  g  b flcr flcg

    flds    l_lcb[cur_l]            ;  r  g  b flcr flcg  lcb
    fmuls   _FACTOR                 ;  r  g  b flcr flcg flcb

    fxch    st(2)                   ;  r  g  b flcb flcg flcr
    faddp   st(5),st                ; ar  g  b flcb flcg
    faddp   st(3),st                ; ar ag  b flcb
    faddp   st(1),st                ; ar ag ab

    mov     cur_l, l_next[cur_l]    ; l = l->next
    test    cur_l, cur_l
    jnz     again

clamp:
                                    ;  ar  ag  ab
    fxch    st(2)                   ;  ab  ag  ar
    fmuls   tff[rc]                 ;  ab  ag ars
    fxch    st(1)                   ;  ab ars  ag
    fmuls   tff[rc]                 ;  ab ars ags
    fxch    st(2)                   ; ags ars  ab
    fmuls   tff[rc]                 ; ags ars abs
    fxch    st(1)                   ; ags abs ars
r_clamp:
    fstsp   d_r[dest]               ; ags abs
    mov     tmp, d_r[dest]
    cmp     tmp, 437F0000h          ; tmp ? 255.0f
    jl      g_clamp
    mov     DWP d_r[dest], 437F0000h
g_clamp:
    fxch    st(1)                   ; abs ags
    fstsp   d_g[dest]               ; abs
    mov     tmp, d_g[dest]
    cmp     tmp, 437F0000h          ; tmp ? 255.0f
    jl      b_clamp
    mov     DWP d_g[dest], 437F0000h
b_clamp:
    fstsp   d_b[dest]               ; abs
    mov     tmp, d_b[dest]
    cmp     tmp, 437F0000h          ; tmp ? 255.0f
    jl      b_clamp
    mov     DWP d_g[dest], 437F0000h
    pop esi
    pop ebx                         ; restore state
    ret             

    a_atrIRGBSRC_LIGHTING_DIFFUSE ENDP


_TEXT ENDS
END
