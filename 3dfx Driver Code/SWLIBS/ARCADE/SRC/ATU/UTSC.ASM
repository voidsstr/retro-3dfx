;; 
;; Utility Assembly functions for the Arcade Toolbox
;;
TITLE  utsc.asm
.586P
;include listing.inc
if @Version gt 510
.model FLAT, C
else
_TEXT	SEGMENT PARA USE32 PUBLIC 'CODE'
_TEXT	ENDS
_DATA	SEGMENT DWORD USE32 PUBLIC 'DATA'
_DATA	ENDS
CONST	SEGMENT DWORD USE32 PUBLIC 'CONST'
CONST	ENDS
_BSS	SEGMENT DWORD USE32 PUBLIC 'BSS'
_BSS	ENDS
_TLS	SEGMENT DWORD USE32 PUBLIC 'TLS'
_TLS	ENDS
FLAT	GROUP _DATA, CONST, _BSS
	ASSUME	CS: FLAT, DS: FLAT, SS: FLAT
endif

; some useful floating load and store macros <ala gmt>
flds	TEXTEQU	<fld  DWORD PTR>
fsubs	TEXTEQU <fsub DWORD PTR>
fmuls	TEXTEQU	<fmul DWORD PTR>
fstsp	TEXTEQU	<fstp DWORD PTR>

_DATA	SEGMENT
One 	DD	03f800000r
Dummy   DD  ?
_DATA	ENDS

_TEXT		SEGMENT

;;  void atuTscRead( FxU32 *low, FxU32 *high ) 
;;  Reads the pentium time stamp counter and returns
;;  the low 32bits 

	align 4
    PUBLIC  atuTscRead
    atuTscRead PROC NEAR   

;; Stack After Register Save
;; +20 - high
;; +16 - low
;; +12 - return address
;; +8  - edx
;; +4  - eax
;; esp - ebx

	.586
    push    edx             ; Save State
    push    eax             ; Save State
    push    ebx             ; Save State

    rdtsc                   ; Read the time stamp counter ~12 clocks

    mov     ebx, [esp+20]   ; Get Pointer to High Order Word
    mov     [ebx],edx       ; Save High Order Word
    mov     ebx, [esp+16]   ; Get Pointer to Low Order Word
    mov     [ebx],eax       ; Save Low Order Word

    pop     ebx             ; Restore ebx
    pop     eax             ; Restore eax
    pop     edx             ; Restore edx

	ret     0
    atuTscRead ENDP

_TEXT ENDS
END
