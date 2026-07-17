#if 0 /* Perl cannot handle C-style comments */
/*
** Copyright 1996, 1997, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
*/
#endif
#if (FR_ZSIZE == 2)
#  define ZTYPE word
#  define ZVALUE si
#elif (FR_ZSIZE == 4)
#  define ZTYPE dword
#  define ZVALUE eax
#endif

FR_ZTEST_PROTOTYPE
{
#if FR_ZTEST == NEVER
    __asm mov eax, 0
    __asm ret
#elif (FR_ZTEST == ALWAYS) && !FR_ZWRITE
    __asm mov eax, ecx
    __asm ret
#else
    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask
#if FR_ZTEST != ALWAYS
    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit
#endif

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:
#if FR_ZSIZE == 2
    __asm mov esi, eax
    __asm sar esi, 16
#endif
#if FR_ZTEST == ALWAYS
    __asm mov ZTYPE ptr [edi], ZVALUE                  ; *zp = z
#else
    __asm cmp ZVALUE, ZTYPE ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm ZTESTFAILOPCODE fail0
#if FR_ZWRITE
    __asm mov ZTYPE ptr [edi], ZVALUE
#endif
    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1
#endif
    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

#if FR_ZTEST != ALWAYS
    __asm shr ebp, 1
#endif
    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set
#if FR_ZSIZE == 2
    __asm mov esi, eax
    __asm sar esi, 16
#endif

#if FR_ZTEST == ALWAYS
    __asm mov ZTYPE ptr [edi], ZVALUE                  ; *zp = z
    __asm jmp mask_bit_not_set
#else
    __asm cmp ZVALUE, ZTYPE ptr [edi]
    __asm ZTESTFAILOPCODE fail1
#if FR_ZWRITE
    __asm mov ZTYPE ptr [edi], ZVALUE                  ; *zp = z
#endif
    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:
#endif

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out
#if FR_ZTEST != ALWAYS
   __asm not eax                                       ; return ~mask_out
#endif
    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret
#endif /* FR_ZTEST == NEVER */
}

#  undef ZTYPE
#  undef ZVALUE
