;
;  Common eques for .asm module (structure member offsets, consts, etc...)
;

;include         ofs4asm.inc

CacheNOP        equ     nop
Zero            equ     0

movq_mm0_esi    equ     db 0Fh, 6Fh, 46h, 00h
movq_mm1_esi    equ     db 0Fh, 6Fh, 4Eh, 00h
movq_mm2_esi    equ     db 0Fh, 6Fh, 56h, 00h
movq_mm3_esi    equ     db 0Fh, 6Fh, 5Eh, 00h
movq_mm4_esi    equ     db 0Fh, 6Fh, 66h, 00h
movq_mm5_esi    equ     db 0Fh, 6Fh, 6Eh, 00h
movq_mm6_esi    equ     db 0Fh, 6Fh, 76h, 00h
movq_mm7_esi    equ     db 0Fh, 6Fh, 7Eh, 00h

HW2_STRIP_CONTINUE      equ     1000BC53h
HW2_FAN_CONTINUE        equ     1040bc53H

FOG_TABLE_SIZE  equ     64

AA_X            equ     0
AA_Y            equ     1

aaEdgeSenseTop  equ     0
aaEdgeSenseLeft equ     1
aaEdgeSenseBottom equ   2
aaEdgeSenseRight equ    3
