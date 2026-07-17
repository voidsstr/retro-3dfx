;; Copyright (c) 1995, 3Dfx Interactive, Inc.
;; All Rights Reserved.
;;
;; This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
;; the contents of this file may not be disclosed to third parties, copied or
;; duplicated in any form, in whole or in part, without the prior written
;; permission of 3Dfx Interactive, Inc.
;;
;; RESTRICTED RIGHTS LEGEND:
;; Use, duplication or disclosure by the Government is subject to restrictions
;; as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
;; and Computer Software clause at DFARS 252.227-7013, and/or in similar or
;; successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
;; rights reserved under the Copyright Laws of the United States.
;;
;; $Header: /devel/h3/glide/src/xdraw2.asm 11    3/19/99 1:24p Peter $
;; $Revision: 11 $
;; $Log: /devel/h3/glide/src/xdraw2.asm $
;; 
;; 11    3/19/99 1:24p Peter
;; change for calling convention change from asm fifo wrapping code
;; 
;; 10    10/09/98 6:57p Peter
;; dynamic 3DNow!(tm) mods
;; 
;; 9     9/21/98 5:35p Dow
;; 3DNow! mods
;; 
;; 8     5/18/98 3:21p Peter
;; dynamic culling changes
; 
; 6     1/15/98 1:12p Peter
; dispatch w/o packing
; 
; 5     11/06/97 3:47p Peter
; dispatch code w/ simulator
; 
; 4     11/04/97 5:04p Peter
; cataclysm part deux
; 
; 3     11/01/97 10:01a Peter
; tri dispatch stuff
; 
; 2     10/30/97 6:53p Peter
; first real cut at tri asm
; 
; 1     10/30/97 4:29p Peter
; asm tri code
; 
; 2     7/07/97 2:14p Jdt
; assembly now on par with C code.
; 
; 1     7/07/97 8:37a Jdt
; B4 Chip field fix.
;;

TITLE   xdraw2.asm
OPTION OLDSTRUCTS

.586P
.MMX
.K3D
    
EXTRN   __GlideRoot:    DWORD
EXTRN   __grCommandTransportMakeRoom@12: NEAR

IFDEF HAL_CSIM
EXTRN   _halStore32@8:  NEAR
ENDIF

_DATA   SEGMENT
    One         DD  03f800000r
    Area        DD  0

IF GLIDE_PACKED_RGB    
    bias0   DD  0
    bias1   DD  0
ENDIF
_DATA   ENDS

; Ugly, but seems to workaround the problem with locally defined
; data segment globals not getting relocated properly when using
; djgpp.

zArea   TEXTEQU <One+04h>

;;; Definitions of cvg regs and glide root structures.
INCLUDE fxgasm.h

; Arguments (STKOFF = 16 from 4 pushes)
STKOFF  = 16
_va$    =  4 + STKOFF
_vb$    =  8 + STKOFF
_vc$    = 12 + STKOFF

PROC_TYPE MACRO procType:=<Default>
    IFDEF GL_AMD3D
        EXITM <__trisetup_3DNow_&procType&@12>
    ELSE
        EXITM <__trisetup_Default_&procType&@12>
    ENDIF
    ENDM        
    
;--------------------------------------------------------------------------
_TEXT       SEGMENT PAGE PUBLIC USE32 'CODE'
            ASSUME DS: FLAT, SS: FLAT

            ALIGN  32

            PUBLIC  PROC_TYPE(cull)
PROC_TYPE(cull)  PROC    NEAR

GLIDE_CULLING       textequ <1>
GLIDE_PACK_RGB      textequ <0>
GLIDE_PACK_ALPHA    textequ <0>
GLIDE_GENERIC_SETUP textequ <0>
INCLUDE xdraw2.inc
GLIDE_GENERIC_SETUP textequ <0>    
GLIDE_PACK_ALPHA    textequ <0>
GLIDE_PACK_RGB      textequ <0>    
GLIDE_CULLING       textequ <0>

PROC_TYPE(cull) ENDP

            ALIGN  32

IF GLIDE_PACKED_RGB
            PUBLIC  PROC_TYPE(cull_rgb)
PROC_TYPE(cull_rgb)  PROC    NEAR

GLIDE_CULLING       textequ <1>
GLIDE_PACK_RGB      textequ <1>
GLIDE_PACK_ALPHA    textequ <0>
GLIDE_GENERIC_SETUP textequ <0>
INCLUDE xdraw2.inc
GLIDE_GENERIC_SETUP textequ <0>    
GLIDE_PACK_ALPHA    textequ <0>
GLIDE_PACK_RGB      textequ <0>    
GLIDE_CULLING       textequ <0>

PROC_TPYE(cull_rgb) ENDP

            ALIGN  32

            PUBLIC  PROC_TPYE(cull_argb)
PROC_TPYE(cull_argb)  PROC    NEAR

GLIDE_CULLING       textequ <1>
GLIDE_PACK_RGB      textequ <1>
GLIDE_PACK_ALPHA    textequ <1>
GLIDE_GENERIC_SETUP textequ <0>
INCLUDE xdraw2.inc
GLIDE_GENERIC_SETUP textequ <0>    
GLIDE_PACK_ALPHA    textequ <0>
GLIDE_PACK_RGB      textequ <0>    
GLIDE_CULLING       textequ <0>

PROC_TPYE(cull_argb) ENDP
ENDIF ; GLIDE_PACKED_RGB

            ALIGN   32

            PUBLIC  PROC_TYPE()
PROC_TYPE()  PROC    NEAR

GLIDE_CULLING       textequ <0>
GLIDE_PACK_RGB      textequ <0>
GLIDE_PACK_ALPHA    textequ <0>
GLIDE_GENERIC_SETUP textequ <0>
INCLUDE xdraw2.inc
GLIDE_GENERIC_SETUP textequ <0>    
GLIDE_PACK_ALPHA    textequ <0>
GLIDE_PACK_RGB      textequ <0>    
GLIDE_CULLING       textequ <0>

PROC_TYPE() ENDP

            ALIGN  32

IF GLIDE_PACKED_RGB    
            PUBLIC  PROC_TYPE(rgb)
PROC_TPYE(rgb)  PROC    NEAR

GLIDE_CULLING       textequ <0>
GLIDE_PACK_RGB      textequ <1>
GLIDE_PACK_ALPHA    textequ <0>
GLIDE_GENERIC_SETUP textequ <0>
INCLUDE xdraw2.inc
GLIDE_GENERIC_SETUP textequ <0>    
GLIDE_PACK_ALPHA    textequ <0>
GLIDE_PACK_RGB      textequ <0>    
GLIDE_CULLING       textequ <0>

PROC_TPYE(rgb) ENDP

            ALIGN  32

            PUBLIC  PROC_TPYE(argb)
PROC_TPYE(argb)  PROC    NEAR

GLIDE_CULLING       textequ <0>
GLIDE_PACK_RGB      textequ <1>
GLIDE_PACK_ALPHA    textequ <1>
GLIDE_GENERIC_SETUP textequ <0>
INCLUDE xdraw2.inc
GLIDE_GENERIC_SETUP textequ <0>    
GLIDE_PACK_ALPHA    textequ <0>
GLIDE_PACK_RGB      textequ <0>    
GLIDE_CULLING       textequ <0>

PROC_TPYE(argb) ENDP
ENDIF ; GLIDE_PACKED_RGB

    
_TEXT ENDS

END

