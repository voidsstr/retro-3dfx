
.586p
.model FLAT

include k6common.inc

.K3D

public  _detect3DX
public  __3DNowAllowed

IFNDEF WINNT  ; K6-2 optimized fogGenerateLinear not implemented under WinNT
public  _fogGenerateLinear_K62O
public  _fogGenerateLinear

extrn   _indexToW : DWORD
extrn   _fogGenerateLinear_Orig : proc
public  _fogGenerateLinear
public  _fogGenerateLinear_K62O

extrn   _indexToW : DWORD
ENDIF

_DATA   segment

align 16

__3DNowAllowed          dd      0

IFNDEF WINNT
; Function ptr table:
_fogGenerateLinear      dd      _fogGenerateLinear_Orig

_65536f                 REAL4   65536.0R
                        REAL4   0.0R

_255f                   REAL4   255.0R

oneminusone             REAL4   1.0R
                        REAL4   -1.0R

                dd      0 ; for safe
ENDIF

_DATA   ends

_TEXT   segment

; BOOL detect3DX();
align 16
_detect3DX      proc
                push    ebx
                pushfd
                pop     eax
                mov     ebx, eax
                xor     eax, 00200000h
                push    eax
                popfd
                pushfd
                pop     eax
                cmp     eax, ebx
                jz      d3dx_ret_false          ; no cpuid
                mov     eax, 80000000h
                cpuid
                cmp     eax, 80000001h
                jb      d3dx_ret_false          ; no extended function
                mov     eax, 80000001h
                cpuid
                test    edx, 80000000h
                jz      d3dx_ret_false          ; no 3DNow!
                mov     eax, 1
        d3dx_no_3dx:
                pop     ebx
                ret
        d3dx_ret_false:
                mov     eax, 0
                pop     ebx
                ret
_detect3DX      endp

IFNDEF WINNT    ; shouston - K6-2 optimized fogGenerateLinear not implemented under WinNT

; **************************************************************************
;void fogGenerateLinear( unsigned char fogtable[FOG_TABLE_SIZE], float nearZ, float farZ )
_fogGenerateLinear_K62O proc
; Parameters:
        p_farZ          equ     [esp+12]
        p_nearZ         equ     [esp+ 8]
        p_fogtable      equ     [esp+ 4]
        ; eip is here - [esp]
        ; local vars further
                femms
;{
;   int i;
;   float world_w;
;   float f;
;
                mov     ecx, FOG_TABLE_SIZE - 1
                ; indexes be gone through backward and be muled by 4
                mov     edx, p_fogTable
                movd    mm7, _65536f
                movd    mm6, oneminusone    ; mm6: - | 1.0
                pxor    mm5, mm5            ; mm5: 0 | 0
                movd    mm4, _255f
                movd    mm1, dword ptr p_nearZ
                lea     eax, [ecx*4]
                jmp     fgl_do_cycle
align 16
        fgl_do_cycle:
;   for ( i = 0; i < FOG_TABLE_SIZE; i++ )
;   {
;      world_w = INDEXTOW( i ); //guFogTableIndexToW( i );
                movd    mm2, dword ptr p_farZ
                movd    mm0, [_indexToW+eax]          ; mm0: 0 | world_w

                pfsub   mm2, mm1            ; mm2: - | (farZ - nearZ)
                pfrcp   mm3, mm2

                pfmin   mm0, mm7
                pfrcpit1 mm2, mm3
;      if ( world_w > 65535.0F )
;         world_w = 65535.0F;
;
;      f = ( world_w - nearZ ) / ( farZ - nearZ );
                pfsub   mm0, mm1            ; mm0: - | (world_w - nearZ)
                pfrcpit2 mm2, mm3           ; mm2: - | 1/(farZ - nearZ)

                pfmul   mm0, mm2  ; mm0: - | (world_w - nearZ) / (farZ - nearZ)
                pfmax   mm0, mm5
;      if ( f > 1.0F )
;         f = 1.0F;
;      else if ( f < 0.0F )
;         f = 0.0F;
                pfmin   mm0, mm6
                pfmul   mm0, mm4

                pf2id   mm0, mm0
                movd    eax, mm0
;      f *= 255.0F;
;
;      fogtable[i] = ( unsigned char ) float2int(f);
                mov     byte ptr [edx+ecx], al
                dec     ecx

                lea     eax, [ecx*4]
                jns     fgl_do_cycle
;   }
;}
                femms
                ret
_fogGenerateLinear_K62O endp

ENDIF ; WINNT

align   16

_TEXT   ends

        end
