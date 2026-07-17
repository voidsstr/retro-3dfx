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

/* All the instructions that 'og' supports are here.
 * Ordered by ascending arity, then alphabetically.
 */

/* We include this file in several places (in og.h and og.c) where we
 * want to run through all supported instructions.  If you add extra
 * instructions, just add them here, and add a case for them in
 * 'emit_code'.
 */

FUN0(fabsolute,	FABS)
FUN0(fchs,	FCHS)
FUN0(fldpi,	FLDPI)
FUN0(int3,	INT3)
FUN0(wait,	WAIT)
FUN0(movsd,	MOVSD)
FUN0(nop,	NOP)
FUN0(opt_begin,	OPT_BEGIN)
FUN0(opt_end,	OPT_END)
FUN0(rdtsc,	RDTSC)
FUN0(rep,	REP)
FUN0(ret,	RET)
FUN0(size16,	SIZE16)
FUN1(call,	CALL)
FUN1(fadd,	FADD)
FUN1(faddq,	FADDQ)
FUN1(fdiv,	FDIV)
FUN1(fdivr,	FDIVR)
FUN1(fild,	FILD)
FUN1(fildq,	FILDQ)
FUN1(fist,	FIST)
FUN1(fistp,	FISTP)
FUN1(fistpq,	FISTPQ)
FUN1(fld,	FLD)
FUN1(fldcw,	FLDCW)
FUN1(fldq,	FLDQ)
FUN1(fmul,	FMUL)
FUN1(fst,	FST)
FUN1(fstcw,	FSTCW)
FUN1(fstp,	FSTP)
FUN1(fstpq,	FSTPQ)
FUN1(fsub,	FSUB)
FUN1(fxch,	FXCH)
FUN1(ja,	JA)
FUN1(jae,	JAE)
FUN1(jb,	JB)
FUN1(jbe,	JBE)
FUN1(je,	JE)
FUN1(jg,	JG)
FUN1(jge,	JGE)
FUN1(jl,	JL)
FUN1(jle,	JLE)
FUN1(jmp,	JMP)
FUN1(jmpi,	JMPI)
FUN1(jne,	JNE)
FUN1(jno,	JNO)
FUN1(jns,	JNS)
FUN1(jo,	JO)
FUN1(js,	JS)
FUN1(pop,	POP)
FUN1(push,	PUSH)
FUN1(retn,	RETN)
FUN2(adc,	ADC)
FUN2(add,	ADD)
FUN2(and,	AND)
FUN2(cmp,	CMP)
FUN1(dec,	DEC)
FUN2(faddp,	FADDP)
FUN2(fmulp,	FMULP)
FUN2(fsubp,	FSUBP)
FUN2(fsubrp,	FSUBRP)
FUN2(idiv,	IDIV)
FUN2(imul,	IMUL)
FUN1(inc,	INC)
FUN2(lea,	LEA)
FUN2(mov,	MOV)
FUN2(movsx,	MOVSX)
FUN2(movzx,	MOVZX)
FUN2(movsxw,	MOVSXW)
FUN2(movzxw,	MOVZXW)
FUN2(or,	OR)
FUN2(rol,	ROL)
FUN2(sar,	SAR)
FUN2(sbb,	SBB)
FUN2(shl,	SHL)
FUN2(shr,	SHR)
FUN2(sub,	SUB)
FUN2(test,	TEST)
FUN2(xor,	XOR)

#if MMX
FUN0(emms,	EMMS)
FUN2(movd, MOVD)
FUN2(movq, MOVQ)
FUN2(packssdw, PACKSSDW)
FUN2(packsswb, PACKSSWB)
FUN2(packuswb, PACKUSWB)
FUN2(paddb, PADDB)
FUN2(paddd, PADDD)
FUN2(paddsb, PADDSB)
FUN2(paddsw, PADDSW)
FUN2(paddusb, PADDUSB)
FUN2(paddusw, PADDUSW)
FUN2(paddw, PADDW)
FUN2(pand, PAND)
FUN2(pandn, PANDN)
FUN2(pcmpeqb, PCMPEQB)
FUN2(pcmpeqd, PCMPEQD)
FUN2(pcmpeqw, PCMPEQW)
FUN2(pcmpgtb, PCMPGTB)
FUN2(pcmpgtd, PCMPGTD)
FUN2(pcmpgtw, PCMPGTW)
FUN2(pmaddwd, PMADDWD)
FUN2(pmulhw, PMULHW)
FUN2(pmullw, PMULLW)
FUN2(por, POR)
FUN2(pslld, PSLLD)
FUN2(psllq, PSLLQ)
FUN2(psllw, PSLLW)
FUN2(psrad, PSRAD)
FUN2(psraq, PSRAQ)
FUN2(psraw, PSRAW)
FUN2(psrld, PSRLD)
FUN2(psrlq, PSRLQ)
FUN2(psrlw, PSRLW)
FUN2(psubb, PSUBB)
FUN2(psubd, PSUBD)
FUN2(psubsb, PSUBSB)
FUN2(psubsw, PSUBSW)
FUN2(psubusb, PSUBUSB)
FUN2(psubusw, PSUBUSW)
FUN2(psubw, PSUBW)
FUN2(punpckhbw, PUNPCKHBW)
FUN2(punpckhdq, PUNPCKHDQ)
FUN2(punpckhwd, PUNPCKHWD)
FUN2(punpcklbw, PUNPCKLBW)
FUN2(punpckldq, PUNPCKLDQ)
FUN2(punpcklwd, PUNPCKLWD)
FUN2(pxor, PXOR)
#endif
