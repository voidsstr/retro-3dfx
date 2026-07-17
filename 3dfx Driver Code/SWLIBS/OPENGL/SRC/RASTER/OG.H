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
#ifndef __OG_H
#define __OG_H

#define MMX	1	/* Enable MMX assembler support */

#include <stdio.h>
#include "types.h"

#if defined(_WINDOWS) && !defined(_INC_STDDEF)
/* define it only if not already defined */
#define offsetof(M, S)  ((int)&(((S*)NULL)->M))
#endif /* _WINDOWS && !_INC_STDDEF */
#define ssizeof(M, S)	(sizeof(((S*)NULL)->M))

struct celem {
  enum what_enum {
    VALUE,	/* constant value */
    reg8,
    reg16,
    reg32,
    fpreg,
    mmxreg,
    MEM,
    INSN,
    LABEL
  } what;
  union {
    int scalar; /* used for initialisation */
    int value;
    int reg8, reg16, reg32, fpreg, mmxreg;
    struct {
      struct celem* disp;
      int scale;
      struct celem* reg1;
      struct celem* reg2;
    } mem;
    struct insn_struct {
      enum opcode_enum {
	MOV,
	MOVSX,
	MOVZX,
	MOVSXW,
	MOVZXW,
	LEA,
	PUSH,
	POP,
	INC,
	DEC,

	JA,
	JAE,
	JB,
	JBE,

	JGE,
	JL,
	JLE,
	JG,
	JE,
	JNE,

	JO,
	JNO,

	JS,
	JNS,

	CALL,
	JMP,
	JMPI,
	RET,
	RETN,
	CMP,
	ADD,
	ADC,
	IMUL,
	IDIV,
	SUB,
	SBB,
	AND,
	OR,
	XOR,
	ROL,
	SHL,
	SHR,
	SAR,
	TEST,
	INT3,
	WAIT,
	SIZE16,
	REP,
	MOVSD,
	NOP,

	FLD,
	FILD,
	FILDQ,
	FXCH,
	FST,
	FSTP,
	FSTPQ,
	FISTP,
	FIST,
	FISTPQ,
	FMUL,
	FMULP,
	FADD,
	FADDP,
	FSUB,
	FDIVR,
	FDIV,
	FLDPI,
	FABS,
	FLDCW,
	FSTCW,

	FSUBP,
	FSUBRP,
	FLDQ,
	FADDQ,
	FCHS,

	RDTSC,

	EMMS,

	MOVD,
	MOVQ,

	PACKSSDW,
	PACKSSWB,
	PACKUSWB,
	PSLLD,
	PSLLQ,
	PSLLW,
	PSRAD,
	PSRAQ,
	PSRAW,
	PSRLD,
	PSRLQ,
	PSRLW,
	PUNPCKHBW,
	PUNPCKHDQ,
	PUNPCKHWD,
	PUNPCKLBW,
	PUNPCKLDQ,
	PUNPCKLWD,

	PADDB,
	PADDD,
	PADDSB,
	PADDSW,
	PADDUSB,
	PADDUSW,
	PADDW,
	PAND,
	PANDN,
	PCMPEQB,
	PCMPEQD,
	PCMPEQW,
	PCMPGTB,
	PCMPGTW,
	PCMPGTD,
	PMADDWD,
	PMULHW,
	PMULLW,
	PSUBB,
	PSUBW,
	PSUBD,
	PSUBSB,
	PSUBSW,
	PSUBUSB,
	PSUBUSW,
	POR,
	PXOR,

	OPT_BEGIN,
	OPT_END

      }	opcode;
      struct celem* op1;
      struct celem* op2;
      struct celem* next;	/* For code blocks */
    } insn;
    struct {
	enum { rel_byte, rel_word } cl;
	struct celem *prev;
	unsigned char *where;
	char *name;
    } label;
  } spec;
};

extern struct celem eax[1];
extern struct celem ebx[1];
extern struct celem ecx[1];
extern struct celem edx[1];
extern struct celem esi[1];
extern struct celem edi[1];
extern struct celem ebp[1];
extern struct celem esp[1];

extern struct celem al[1];
extern struct celem ah[1];
extern struct celem bl[1];
extern struct celem bh[1];
extern struct celem cl[1];
extern struct celem ch[1];
extern struct celem dl[1];
extern struct celem dh[1];

extern struct celem st0[1];
extern struct celem st1[1];
extern struct celem st2[1];
extern struct celem st3[1];
extern struct celem st4[1];
extern struct celem st5[1];
extern struct celem st6[1];
extern struct celem st7[1];

extern struct celem mm0[1];
extern struct celem mm1[1];
extern struct celem mm2[1];
extern struct celem mm3[1];
extern struct celem mm4[1];
extern struct celem mm5[1];
extern struct celem mm6[1];
extern struct celem mm7[1];

struct celem* value(__GLcontext *gc, int v);
struct celem* label(__GLcontext *gc);
struct celem* near_label(__GLcontext *gc);
struct celem* mem(__GLcontext *gc,
		  struct celem* disp,
		  struct celem* reg1,
		  struct celem* reg2,
		  int scale);
struct celem* mem2(__GLcontext *gc,
		  struct celem* disp,
		  struct celem* reg1);


/*
 * Make declarations for all the instructions 'og' supports...
 * Macro FUN0,1,2 makes a declaration for a function for building a parse node.
 */

#define FUN2(lower_name, UPPER_NAME) \
struct celem*					\
lower_name(__GLcontext *gc,			\
     struct celem* op1,				\
     struct celem* op2);

#define FUN1(lower_name, UPPER_NAME) \
struct celem*					\
lower_name(__GLcontext *gc,			\
     struct celem* op1);

#define FUN0(lower_name, UPPER_NAME) \
struct celem*					\
lower_name(__GLcontext *gc);

#include "og_insns.h"	/* Run through the instruction set */

#undef FUN0
#undef FUN1
#undef FUN2

typedef struct {
    struct celem ready[8000];	/* An array of nodes from which we build the parse tree */
				/* (We catch overflow with an assert in og.c */
    struct celem* next;		/* Next available node */
    FILE *source;		/* File to dump assembly source to, if enabled */

    float *fpconstant_pool;	/* Floating-point constants */
    GLuint fpconstant_next;	/* Next free slot in pool */
    GLuint fpconstant_size;	/* How big is the pool? */

    double *mxconstant_pool;	/* 64-bit MMX constants */
    GLuint mxconstant_next;	/* Next free slot in pool */
    GLuint mxconstant_size;	/* How big is the pool? */

    GLuint spsave;		/* Used by triangle renderer to preserve stack state */
    GLuint rsave;
    GLuint spsave2; 		/* Similarly used by BuildDrawVertexes */
} __GLOGState;

void resolve(__GLcontext *gc, struct celem* ref, unsigned char* cb);
unsigned char* emit_code(__GLcontext *gc, unsigned char* cb, struct celem* c);
void og_warmup(__GLcontext *gc);
void og_stream(__GLcontext *gc, char* file);

struct celem* fpk(__GLcontext *gc, double v);
struct celem* mxk4(__GLcontext *gc, GLint v0, GLint v1, GLint v2, GLint v3);

#endif
