#if defined(__GL_PC_RAST) && !defined(__GL_CODEGEN)
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
#include "context.h"
#include "global.h"
#include "xform.h"
#include "string.h"
#include "fr_modes.h"
#include "fr_tri.h"

#undef __GL_USE_INTEL_ASM

#ifdef __GL_USE_INTEL_ASM
#pragma warning(disable : 4035)
#define SAVE_EBP dword ptr [esp-4]
#define SAVE_EBX dword ptr [esp-8]
#define SAVE_ECX dword ptr [esp-12]
#define SAVE_ESI dword ptr [esp-16]
#define SAVE_EDI dword ptr [esp-20]
#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 0
#define FR_ZTEST_OP 0
#define FR_ZTEST_OPCODE 0
#define FR_ZWRITE 0
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_0(GLbitfield mask, __GLtri *tr) 
{

}

#else

__declspec(naked) void __fastcall __glDepthTest_0(GLbitfield mask, __GLtri *tr) 
{

    __asm mov eax, 0
    __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 1
#define FR_ZTEST_OP <
#define FR_ZTEST_OPCODE jae
#define FR_ZWRITE 0
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_1(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz <  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_1(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jae   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm jae   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 2
#define FR_ZTEST_OP ==
#define FR_ZTEST_OPCODE jne
#define FR_ZWRITE 0
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_2(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz ==  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_2(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jne   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm jne   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 3
#define FR_ZTEST_OP <=
#define FR_ZTEST_OPCODE ja
#define FR_ZWRITE 0
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_3(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz <=  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_3(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm ja   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm ja   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 4
#define FR_ZTEST_OP >
#define FR_ZTEST_OPCODE jbe
#define FR_ZWRITE 0
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_4(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz >  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_4(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jbe   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm jbe   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 5
#define FR_ZTEST_OP !=
#define FR_ZTEST_OPCODE je
#define FR_ZWRITE 0
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_5(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz !=  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_5(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm je   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm je   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 6
#define FR_ZTEST_OP >=
#define FR_ZTEST_OPCODE jb
#define FR_ZWRITE 0
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_6(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz >=  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_6(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jb   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm jb   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 7
#define FR_ZTEST_OP 1
#define FR_ZTEST_OPCODE 1
#define FR_ZWRITE 0
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_7(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    /* We should never get here if PickTriangleProcs is smart */
    (*gc->procs.afterDepthTest)(mask, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_7(GLbitfield mask, __GLtri *tr) 
{

    __asm mov eax, ecx
    __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 0
#define FR_ZTEST_OP 0
#define FR_ZTEST_OPCODE 0
#define FR_ZWRITE 0
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_8(GLbitfield mask, __GLtri *tr) 
{

}

#else

__declspec(naked) void __fastcall __glDepthTest_8(GLbitfield mask, __GLtri *tr) 
{

    __asm mov eax, 0
    __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 1
#define FR_ZTEST_OP <
#define FR_ZTEST_OPCODE jae
#define FR_ZWRITE 0
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_9(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz <  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_9(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jae   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm jae   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 2
#define FR_ZTEST_OP ==
#define FR_ZTEST_OPCODE jne
#define FR_ZWRITE 0
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_A(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz ==  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_A(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jne   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm jne   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 3
#define FR_ZTEST_OP <=
#define FR_ZTEST_OPCODE ja
#define FR_ZWRITE 0
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_B(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz <=  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_B(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm ja   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm ja   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 4
#define FR_ZTEST_OP >
#define FR_ZTEST_OPCODE jbe
#define FR_ZWRITE 0
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_C(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz >  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_C(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jbe   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm jbe   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 5
#define FR_ZTEST_OP !=
#define FR_ZTEST_OPCODE je
#define FR_ZWRITE 0
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_D(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz !=  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_D(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm je   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm je   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 6
#define FR_ZTEST_OP >=
#define FR_ZTEST_OPCODE jb
#define FR_ZWRITE 0
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_E(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz >=  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_E(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jb   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm jb   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 7
#define FR_ZTEST_OP 1
#define FR_ZTEST_OPCODE 1
#define FR_ZWRITE 0
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_F(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    /* We should never get here if PickTriangleProcs is smart */
    (*gc->procs.afterDepthTest)(mask, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_F(GLbitfield mask, __GLtri *tr) 
{

    __asm mov eax, ecx
    __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 0
#define FR_ZTEST_OP 0
#define FR_ZTEST_OPCODE 0
#define FR_ZWRITE 1
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_10(GLbitfield mask, __GLtri *tr) 
{

}

#else

__declspec(naked) void __fastcall __glDepthTest_10(GLbitfield mask, __GLtri *tr) 
{

    __asm mov eax, 0
    __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 1
#define FR_ZTEST_OP <
#define FR_ZTEST_OPCODE jae
#define FR_ZWRITE 1
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_11(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz <  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_11(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jae   fail0

    __asm mov word  ptr [edi], si 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm jae   fail1

    __asm mov word  ptr [edi], si                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 2
#define FR_ZTEST_OP ==
#define FR_ZTEST_OPCODE jne
#define FR_ZWRITE 1
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_12(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz ==  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_12(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jne   fail0

    __asm mov word  ptr [edi], si 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm jne   fail1

    __asm mov word  ptr [edi], si                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 3
#define FR_ZTEST_OP <=
#define FR_ZTEST_OPCODE ja
#define FR_ZWRITE 1
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_13(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz <=  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_13(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm ja   fail0

    __asm mov word  ptr [edi], si 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm ja   fail1

    __asm mov word  ptr [edi], si                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 4
#define FR_ZTEST_OP >
#define FR_ZTEST_OPCODE jbe
#define FR_ZWRITE 1
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_14(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz >  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_14(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jbe   fail0

    __asm mov word  ptr [edi], si 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm jbe   fail1

    __asm mov word  ptr [edi], si                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 5
#define FR_ZTEST_OP !=
#define FR_ZTEST_OPCODE je
#define FR_ZWRITE 1
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_15(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz !=  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_15(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm je   fail0

    __asm mov word  ptr [edi], si 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm je   fail1

    __asm mov word  ptr [edi], si                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 6
#define FR_ZTEST_OP >=
#define FR_ZTEST_OPCODE jb
#define FR_ZWRITE 1
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_16(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz >=  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_16(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jb   fail0

    __asm mov word  ptr [edi], si 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm jb   fail1

    __asm mov word  ptr [edi], si                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 7
#define FR_ZTEST_OP 1
#define FR_ZTEST_OPCODE 1
#define FR_ZWRITE 1
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_17(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    GLbitfield mask_out = mask;

    while (1) {
	while (((int)mask) < 0) {

	    *zp = TruncFixed(z, 16);

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

    }

    if (mask_out)
	(*gc->procs.afterDepthTest)(mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_17(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm mov word  ptr [edi], si                   ; *zp = z

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm mov word  ptr [edi], si                   ; *zp = z
    __asm jmp mask_bit_not_set

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 0
#define FR_ZTEST_OP 0
#define FR_ZTEST_OPCODE 0
#define FR_ZWRITE 1
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_18(GLbitfield mask, __GLtri *tr) 
{

}

#else

__declspec(naked) void __fastcall __glDepthTest_18(GLbitfield mask, __GLtri *tr) 
{

    __asm mov eax, 0
    __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 1
#define FR_ZTEST_OP <
#define FR_ZTEST_OPCODE jae
#define FR_ZWRITE 1
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_19(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz <  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_19(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jae   fail0

    __asm mov dword  ptr [edi], eax 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm jae   fail1

    __asm mov dword  ptr [edi], eax                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 2
#define FR_ZTEST_OP ==
#define FR_ZTEST_OPCODE jne
#define FR_ZWRITE 1
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_1A(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz ==  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_1A(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jne   fail0

    __asm mov dword  ptr [edi], eax 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm jne   fail1

    __asm mov dword  ptr [edi], eax                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 3
#define FR_ZTEST_OP <=
#define FR_ZTEST_OPCODE ja
#define FR_ZWRITE 1
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_1B(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz <=  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_1B(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm ja   fail0

    __asm mov dword  ptr [edi], eax 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm ja   fail1

    __asm mov dword  ptr [edi], eax                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 4
#define FR_ZTEST_OP >
#define FR_ZTEST_OPCODE jbe
#define FR_ZWRITE 1
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_1C(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz >  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_1C(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jbe   fail0

    __asm mov dword  ptr [edi], eax 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm jbe   fail1

    __asm mov dword  ptr [edi], eax                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 5
#define FR_ZTEST_OP !=
#define FR_ZTEST_OPCODE je
#define FR_ZWRITE 1
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_1D(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz !=  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_1D(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm je   fail0

    __asm mov dword  ptr [edi], eax 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm je   fail1

    __asm mov dword  ptr [edi], eax                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 6
#define FR_ZTEST_OP >=
#define FR_ZTEST_OPCODE jb
#define FR_ZWRITE 1
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_1E(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz >=  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_1E(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jb   fail0

    __asm mov dword  ptr [edi], eax 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm jb   fail1

    __asm mov dword  ptr [edi], eax                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 7
#define FR_ZTEST_OP 1
#define FR_ZTEST_OPCODE 1
#define FR_ZWRITE 1
#define FR_ZSTENCIL 0
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_1F(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    GLbitfield mask_out = mask;

    while (1) {
	while (((int)mask) < 0) {

	    *zp = z;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

    }

    if (mask_out)
	(*gc->procs.afterDepthTest)(mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_1F(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov dword  ptr [edi], eax                   ; *zp = z

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov dword  ptr [edi], eax                   ; *zp = z
    __asm jmp mask_bit_not_set

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 0
#define FR_ZTEST_OP 0
#define FR_ZTEST_OPCODE 0
#define FR_ZWRITE 0
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_20(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */
    (*gc->procs.afterDepthTest)(0, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_20(GLbitfield mask, __GLtri *tr) 
{

    __asm mov eax, 0
    __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 1
#define FR_ZTEST_OP <
#define FR_ZTEST_OPCODE jae
#define FR_ZWRITE 0
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_21(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz <  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_21(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jae   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm jae   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 2
#define FR_ZTEST_OP ==
#define FR_ZTEST_OPCODE jne
#define FR_ZWRITE 0
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_22(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz ==  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_22(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jne   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm jne   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 3
#define FR_ZTEST_OP <=
#define FR_ZTEST_OPCODE ja
#define FR_ZWRITE 0
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_23(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz <=  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_23(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm ja   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm ja   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 4
#define FR_ZTEST_OP >
#define FR_ZTEST_OPCODE jbe
#define FR_ZWRITE 0
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_24(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz >  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_24(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jbe   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm jbe   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 5
#define FR_ZTEST_OP !=
#define FR_ZTEST_OPCODE je
#define FR_ZWRITE 0
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_25(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz !=  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_25(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm je   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm je   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 6
#define FR_ZTEST_OP >=
#define FR_ZTEST_OPCODE jb
#define FR_ZWRITE 0
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_26(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz >=  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_26(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jb   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm jb   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 7
#define FR_ZTEST_OP 1
#define FR_ZTEST_OPCODE 1
#define FR_ZWRITE 0
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_27(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    /* We should never get here if PickTriangleProcs is smart */
    (*gc->procs.afterDepthTest)(mask, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_27(GLbitfield mask, __GLtri *tr) 
{

    __asm mov eax, ecx
    __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 0
#define FR_ZTEST_OP 0
#define FR_ZTEST_OPCODE 0
#define FR_ZWRITE 0
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_28(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */
    (*gc->procs.afterDepthTest)(0, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_28(GLbitfield mask, __GLtri *tr) 
{

    __asm mov eax, 0
    __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 1
#define FR_ZTEST_OP <
#define FR_ZTEST_OPCODE jae
#define FR_ZWRITE 0
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_29(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz <  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_29(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jae   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm jae   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 2
#define FR_ZTEST_OP ==
#define FR_ZTEST_OPCODE jne
#define FR_ZWRITE 0
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_2A(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz ==  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_2A(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jne   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm jne   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 3
#define FR_ZTEST_OP <=
#define FR_ZTEST_OPCODE ja
#define FR_ZWRITE 0
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_2B(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz <=  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_2B(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm ja   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm ja   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 4
#define FR_ZTEST_OP >
#define FR_ZTEST_OPCODE jbe
#define FR_ZWRITE 0
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_2C(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz >  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_2C(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jbe   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm jbe   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 5
#define FR_ZTEST_OP !=
#define FR_ZTEST_OPCODE je
#define FR_ZWRITE 0
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_2D(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz !=  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_2D(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm je   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm je   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 6
#define FR_ZTEST_OP >=
#define FR_ZTEST_OPCODE jb
#define FR_ZWRITE 0
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_2E(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz >=  *zp) {

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_2E(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jb   fail0

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm jb   fail1

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 7
#define FR_ZTEST_OP 1
#define FR_ZTEST_OPCODE 1
#define FR_ZWRITE 0
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_2F(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    /* We should never get here if PickTriangleProcs is smart */
    (*gc->procs.afterDepthTest)(mask, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_2F(GLbitfield mask, __GLtri *tr) 
{

    __asm mov eax, ecx
    __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 0
#define FR_ZTEST_OP 0
#define FR_ZTEST_OPCODE 0
#define FR_ZWRITE 1
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_30(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */
    (*gc->procs.afterDepthTest)(0, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_30(GLbitfield mask, __GLtri *tr) 
{

    __asm mov eax, 0
    __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 1
#define FR_ZTEST_OP <
#define FR_ZTEST_OPCODE jae
#define FR_ZWRITE 1
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_31(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz <  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_31(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jae   fail0

    __asm mov word  ptr [edi], si 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm jae   fail1

    __asm mov word  ptr [edi], si                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 2
#define FR_ZTEST_OP ==
#define FR_ZTEST_OPCODE jne
#define FR_ZWRITE 1
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_32(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz ==  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_32(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jne   fail0

    __asm mov word  ptr [edi], si 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm jne   fail1

    __asm mov word  ptr [edi], si                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 3
#define FR_ZTEST_OP <=
#define FR_ZTEST_OPCODE ja
#define FR_ZWRITE 1
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_33(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz <=  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_33(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm ja   fail0

    __asm mov word  ptr [edi], si 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm ja   fail1

    __asm mov word  ptr [edi], si                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 4
#define FR_ZTEST_OP >
#define FR_ZTEST_OPCODE jbe
#define FR_ZWRITE 1
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_34(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz >  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_34(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jbe   fail0

    __asm mov word  ptr [edi], si 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm jbe   fail1

    __asm mov word  ptr [edi], si                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 5
#define FR_ZTEST_OP !=
#define FR_ZTEST_OPCODE je
#define FR_ZWRITE 1
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_35(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz !=  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_35(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm je   fail0

    __asm mov word  ptr [edi], si 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm je   fail1

    __asm mov word  ptr [edi], si                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 6
#define FR_ZTEST_OP >=
#define FR_ZTEST_OPCODE jb
#define FR_ZWRITE 1
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_36(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    __GLzValue16  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = TruncFixed(z, 16);

	    if (fz >=  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_36(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jb   fail0

    __asm mov word  ptr [edi], si 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm cmp si , word  ptr [edi]
    __asm jb   fail1

    __asm mov word  ptr [edi], si                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 2
#define FR_ZTYPE __GLzValue16
#define FR_ZTEST 7
#define FR_ZTEST_OP 1
#define FR_ZTEST_OPCODE 1
#define FR_ZWRITE 1
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_37(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue16  *zp = (__GLzValue16  *) tr->zp;

    GLbitfield mask_out = mask;

    while (1) {
	while (((int)mask) < 0) {

	    *zp = TruncFixed(z, 16);

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_37(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov esi, eax
    __asm sar esi, 16

    __asm mov word  ptr [edi], si                   ; *zp = z

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov esi, eax
    __asm sar esi, 16

    __asm mov word  ptr [edi], si                   ; *zp = z
    __asm jmp mask_bit_not_set

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 0
#define FR_ZTEST_OP 0
#define FR_ZTEST_OPCODE 0
#define FR_ZWRITE 1
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_38(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */
    (*gc->procs.afterDepthTest)(0, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_38(GLbitfield mask, __GLtri *tr) 
{

    __asm mov eax, 0
    __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 1
#define FR_ZTEST_OP <
#define FR_ZTEST_OPCODE jae
#define FR_ZWRITE 1
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_39(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz <  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_39(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jae   fail0

    __asm mov dword  ptr [edi], eax 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm jae   fail1

    __asm mov dword  ptr [edi], eax                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 2
#define FR_ZTEST_OP ==
#define FR_ZTEST_OPCODE jne
#define FR_ZWRITE 1
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_3A(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz ==  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_3A(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jne   fail0

    __asm mov dword  ptr [edi], eax 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm jne   fail1

    __asm mov dword  ptr [edi], eax                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 3
#define FR_ZTEST_OP <=
#define FR_ZTEST_OPCODE ja
#define FR_ZWRITE 1
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_3B(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz <=  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_3B(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm ja   fail0

    __asm mov dword  ptr [edi], eax 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm ja   fail1

    __asm mov dword  ptr [edi], eax                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 4
#define FR_ZTEST_OP >
#define FR_ZTEST_OPCODE jbe
#define FR_ZWRITE 1
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_3C(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz >  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_3C(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jbe   fail0

    __asm mov dword  ptr [edi], eax 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm jbe   fail1

    __asm mov dword  ptr [edi], eax                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 5
#define FR_ZTEST_OP !=
#define FR_ZTEST_OPCODE je
#define FR_ZWRITE 1
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_3D(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz !=  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_3D(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm je   fail0

    __asm mov dword  ptr [edi], eax 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm je   fail1

    __asm mov dword  ptr [edi], eax                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 6
#define FR_ZTEST_OP >=
#define FR_ZTEST_OPCODE jb
#define FR_ZWRITE 1
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_3E(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    __GLzValue  fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;

    while (1) {
	while (((int)mask) < 0) {

	    fz = z;

	    if (fz >=  *zp) {

		*zp = fz;

	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

	bit >>= 1;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(~mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_3E(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm mov esi, 0                                   ; ESI = fz
    __asm not ebx                                      ; EBX = mask_out = ~mask
    __asm mov ebp, HIBIT                               ; EBP = bit

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm cmp eax , dword  ptr [edi]                  ; if ( z ZTESTOP *zp )
    __asm jb   fail0

    __asm mov dword  ptr [edi], eax 

    __asm jmp after_fail0
fail0:
    __asm or  ebx, ebp                                 ; mask_out |= bit
after_fail0:
    __asm shr ebp, 1                                   ; bit >>= 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm shr ebp, 1

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm cmp eax , dword  ptr [edi]
    __asm jb   fail1

    __asm mov dword  ptr [edi], eax                   ; *zp = z

    __asm jmp after_fail1
fail1:
    __asm or ebx, ebp                                  ; mask_out |= bit
after_fail1:

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

   __asm not eax                                       ; return ~mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif

/*
#define FR_ZSIZE 4
#define FR_ZTYPE __GLzValue
#define FR_ZTEST 7
#define FR_ZTEST_OP 1
#define FR_ZTEST_OPCODE 1
#define FR_ZWRITE 1
#define FR_ZSTENCIL 1
*/
#ifndef __GL_USE_INTEL_ASM

void __fastcall __glDepthTest_3F(GLbitfield mask, __GLtri *tr) 
{

    __GLcontext *gc = tr->gc;
    int z = tr->z;
    __GLzValue  *zp = (__GLzValue  *) tr->zp;

    GLbitfield mask_out = mask;

    while (1) {
	while (((int)mask) < 0) {

	    *zp = z;

	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;

    }

    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */

    (*gc->procs.afterDepthTest)(mask_out, tr);

}

#else

__declspec(naked) void __fastcall __glDepthTest_3F(GLbitfield mask, __GLtri *tr) 
{

    __asm mov SAVE_EBP, ebp
    __asm mov SAVE_EBX, ebx
    __asm mov SAVE_ECX, ecx
    __asm mov SAVE_ESI, esi
    __asm mov SAVE_EDI, edi

    __asm mov eax, dword ptr [edx+__FR_TRI_Z_OFF]      ; EAX = z
    __asm mov edi, dword ptr [edx+__FR_TRI_ZP_OFF]     ; EDI = zp
    __asm mov ebx, ecx                                 ; EBX = mask_out = mask

    __asm cmp ecx, 0                                   ; while ( mask < 0 )
    __asm jns mask_stippled
mask_solid_loop:

    __asm mov dword  ptr [edi], eax                   ; *zp = z

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]  ; zp++
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]   ; z += dzdx
    __asm shl ecx, 1                                   ; mask <<= 1
    __asm js mask_solid_loop

mask_stippled:

    __asm shl ecx, 1
    __asm jz  done                                     ; while ( mask )

mask_stippled_loop:

    __asm add edi, dword ptr [edx+__FR_TRI_DZPDX_OFF]
    __asm add eax, dword ptr [edx+__FR_TRI_DZDX_OFF]

    __asm cmp ecx, 0                                   ; if ( mask & HIBIT )
    __asm jge  mask_bit_not_set

    __asm mov dword  ptr [edi], eax                   ; *zp = z
    __asm jmp mask_bit_not_set

mask_bit_not_set:
    __asm shl ecx, 1
    __asm jnz mask_stippled_loop

done:
   __asm mov eax, ebx                                  ; return mask_out

    __asm mov ebp, SAVE_EBP
    __asm mov ebx, SAVE_EBX
    __asm mov ecx, SAVE_ECX
    __asm mov esi, SAVE_ESI
    __asm mov edi, SAVE_EDI

   __asm ret

}

#endif
/*
 *
 */
void (__fastcall *__fr_ztest_table[64])(GLbitfield mask, __GLtri *tr) =
{
    __glDepthTest_0,
    __glDepthTest_1,
    __glDepthTest_2,
    __glDepthTest_3,
    __glDepthTest_4,
    __glDepthTest_5,
    __glDepthTest_6,
    __glDepthTest_7,
    __glDepthTest_8,
    __glDepthTest_9,
    __glDepthTest_A,
    __glDepthTest_B,
    __glDepthTest_C,
    __glDepthTest_D,
    __glDepthTest_E,
    __glDepthTest_F,
    __glDepthTest_10,
    __glDepthTest_11,
    __glDepthTest_12,
    __glDepthTest_13,
    __glDepthTest_14,
    __glDepthTest_15,
    __glDepthTest_16,
    __glDepthTest_17,
    __glDepthTest_18,
    __glDepthTest_19,
    __glDepthTest_1A,
    __glDepthTest_1B,
    __glDepthTest_1C,
    __glDepthTest_1D,
    __glDepthTest_1E,
    __glDepthTest_1F,
    __glDepthTest_20,
    __glDepthTest_21,
    __glDepthTest_22,
    __glDepthTest_23,
    __glDepthTest_24,
    __glDepthTest_25,
    __glDepthTest_26,
    __glDepthTest_27,
    __glDepthTest_28,
    __glDepthTest_29,
    __glDepthTest_2A,
    __glDepthTest_2B,
    __glDepthTest_2C,
    __glDepthTest_2D,
    __glDepthTest_2E,
    __glDepthTest_2F,
    __glDepthTest_30,
    __glDepthTest_31,
    __glDepthTest_32,
    __glDepthTest_33,
    __glDepthTest_34,
    __glDepthTest_35,
    __glDepthTest_36,
    __glDepthTest_37,
    __glDepthTest_38,
    __glDepthTest_39,
    __glDepthTest_3A,
    __glDepthTest_3B,
    __glDepthTest_3C,
    __glDepthTest_3D,
    __glDepthTest_3E,
    __glDepthTest_3F,
};

#endif /* __GL_PC_RAST && !__GL_CODEGEN */
