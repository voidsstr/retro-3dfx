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
#ifdef __GL_CODEGEN
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>

#include "og.h"
#include "context.h"

/* Sometimes, it's useful to be able to write human-readable (and
 * MASM-readable) versions of generated code to a file.  Setting
 * SOURCE_TRAIL to '1' enables this.  Use the og_stream() function to
 * switch on source-code streamihng.
 */
#define SOURCE_TRAIL 0


#define STATIC static

/*
 * This section provides all the exports that 'ag' files need to
 * build parse trees.  We define the Intel register set, instruction
 * set, and a some ways of constructing memory expressions (mem() and mem2()),
 * constants (value()), labels (label() and near_label()).
 */

/* The Intel registers are pre-built nodes, globally visible.
 * They're initialised here with the bit-field that the instructions
 * actually use, hence the rather odd ordering.
 */
struct celem eax[1] = { reg32, { 0 } };
struct celem ecx[1] = { reg32, { 1 } };
struct celem edx[1] = { reg32, { 2 } };
struct celem ebx[1] = { reg32, { 3 } };
struct celem esp[1] = { reg32, { 4 } };
struct celem ebp[1] = { reg32, { 5 } };
struct celem esi[1] = { reg32, { 6 } };
struct celem edi[1] = { reg32, { 7 } };

struct celem al[1] = { reg8, { 0 } };
struct celem cl[1] = { reg8, { 1 } };
struct celem dl[1] = { reg8, { 2 } };
struct celem bl[1] = { reg8, { 3 } };
struct celem ah[1] = { reg8, { 4 } };
struct celem ch[1] = { reg8, { 5 } };
struct celem dh[1] = { reg8, { 6 } };
struct celem bh[1] = { reg8, { 7 } };

struct celem st0[1] = { fpreg, { 0 } };
struct celem st1[1] = { fpreg, { 1 } };
struct celem st2[1] = { fpreg, { 2 } };
struct celem st3[1] = { fpreg, { 3 } };
struct celem st4[1] = { fpreg, { 4 } };
struct celem st5[1] = { fpreg, { 5 } };
struct celem st6[1] = { fpreg, { 6 } };
struct celem st7[1] = { fpreg, { 7 } };

#if MMX
struct celem mm0[1] = { mmxreg, { 0 } };
struct celem mm1[1] = { mmxreg, { 1 } };
struct celem mm2[1] = { mmxreg, { 2 } };
struct celem mm3[1] = { mmxreg, { 3 } };
struct celem mm4[1] = { mmxreg, { 4 } };
struct celem mm5[1] = { mmxreg, { 5 } };
struct celem mm6[1] = { mmxreg, { 6 } };
struct celem mm7[1] = { mmxreg, { 7 } };
#endif /* MMX */

STATIC struct celem*
gimme_celem(__GLcontext *gc, enum what_enum w)
{
    __GLOGState *ogs = &gc->ogState;
    struct celem* r = ogs->next++;

    assert((char*)ogs->next < ((char*)ogs->ready + sizeof(ogs->ready)));
    r->what = w;
    return r;
}

struct celem*
value(__GLcontext *gc, int v)
{
    struct celem* r = gimme_celem(gc, VALUE);

    r->spec.value = v;

    return r;
}

struct celem*
label(__GLcontext *gc)
{
    struct celem* r = gimme_celem(gc, LABEL);

    r->spec.label.prev = NULL;
    r->spec.label.where = NULL;
    r->spec.label.name = NULL;
    r->spec.label.cl = rel_word;
    
    return r;
}

/*
 * By creating a near label, the generator is assuring
 * us that all references to this label are nearer than
 * 128 bytes.  This is useful with forward references
 * that don't yet know the distance to the branch destination.
 * In this sequence, for example:
 *
 *     struct celem *dont_do_it = near_label(gc);
 *
 *     @   jne     dont_do_it
 * 
 *     ....
 * 
 *     @ dont_do_it:
 *
 * The assembler can generate a short (signed byte offset)
 * form of 'jne', even though it doesn't yet know the address of the
 * destination.
 * 
 */

struct celem*
near_label(__GLcontext *gc)
{
    struct celem* r = gimme_celem(gc, LABEL);

    r->spec.label.prev = NULL;
    r->spec.label.where = NULL;
    r->spec.label.name = NULL;
    r->spec.label.cl = rel_byte;

    return r;
}

struct celem*
mem(__GLcontext *gc, 
    struct celem* disp,
    struct celem* reg1,
    struct celem* reg2,
    int scale)
{
    struct celem* r = gimme_celem(gc, MEM);

    r->spec.mem.disp = disp;
    r->spec.mem.scale = scale;
    r->spec.mem.reg1 = reg1;
    r->spec.mem.reg2 = reg2;

    return r;
}

/* mem2() is just a version of mem() that doesn't have the scaled
 * register term.  This saves some code space because most
 * address expresions are just an offset, a register, or both.
 */
struct celem*
mem2(__GLcontext *gc, 
     struct celem* disp,
     struct celem* reg1)
{
    struct celem* r = gimme_celem(gc, MEM);

    r->spec.mem.disp = disp;
    r->spec.mem.reg1 = reg1;
    r->spec.mem.reg2 = NULL;
    r->spec.mem.scale = 1;

    return r;
}

STATIC struct celem*
insn(__GLcontext *gc, 
     enum opcode_enum o,
     struct celem* op1,
     struct celem* op2)
{
    struct celem* r = gimme_celem(gc, INSN);

    r->spec.insn.opcode = o;
    r->spec.insn.op1 = op1;
    r->spec.insn.op2 = op2;

    return r;
}

/*
 * Macro FUN0,1,2 makes a function for building a parse node.
 * The suffix gives the arity of the function (the number of parameters
 * that the instruction takes).
 */

#define FUN2(lower_name, UPPER_NAME) \
struct celem*					\
lower_name(__GLcontext *gc,			\
     struct celem* op1,				\
     struct celem* op2)				\
{						\
    return insn(gc, UPPER_NAME, op1, op2);	\
}

#define FUN1(lower_name, UPPER_NAME) \
struct celem*					\
lower_name(__GLcontext *gc,			\
     struct celem* op1)				\
{						\
    return insn(gc, UPPER_NAME, op1, NULL);	\
}

#define FUN0(lower_name, UPPER_NAME) \
struct celem*					\
lower_name(__GLcontext *gc)			\
{						\
    return insn(gc, UPPER_NAME, NULL, NULL);	\
}

#include "og_insns.h"	/* Run through the instruction set */

#undef FUN0
#undef FUN1
#undef FUN2

/*
 * Simple predicates for categorizing nodes.
 */

STATIC int
is_reg32(struct celem* c)
{
    return (c->what == reg32);
}

STATIC int
is_reg8(struct celem* c)
{
    return (c->what == reg8);
}

STATIC int
is_reg(struct celem* c)
{
  return is_reg8(c) || is_reg32(c);
}

STATIC int
is_mem(struct celem* c)
{
  return (c->what == MEM);
}

STATIC int
is_mmxreg(struct celem* c)
{
  return (c->what == mmxreg);
}

STATIC int
is_imm(struct celem* c)
{
  return (c->what == VALUE);
}

STATIC int
is_byte_imm(struct celem* c)
{
  return is_imm(c) && (-128 <= c->spec.value) && (c->spec.value < 128);
}

STATIC int
is_binop(struct celem* c)
{
    if ((MOVD <= c->spec.insn.opcode) && (c->spec.insn.opcode <= PXOR))
	return 1;

    return ((c->spec.insn.op1 != NULL) && (c->spec.insn.op2 != NULL));
}

STATIC int
is_mmxop(struct celem* c)
{
    return ((MOVD <= c->spec.insn.opcode) && (c->spec.insn.opcode <= PXOR));
}

STATIC int
is_mmx_shiftop(struct celem* c)
{
    return ((PSLLD <= c->spec.insn.opcode) && (c->spec.insn.opcode <= PUNPCKLWD));
}

#if SOURCE_TRAIL

/* Write out a human-readable version of the expression 'op' to
 * the source log file.
 */
static void source_emit_parm(__GLcontext *gc, struct celem* op, int sz)
{
    __GLOGState *ogs = &gc->ogState;
    FILE *source = ogs->source;
    char *gap = "";

    if (op) {
	static char* r32[] = { "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi" };
	static char* r8[] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };

	switch (op->what) {
	case VALUE:
	    fprintf(source, "%d", op->spec.value);
	    break;
	case reg32:
	    fprintf(source, "%s", r32[op->spec.reg32]);
	    break;
	case reg8:
	    fprintf(source, "%s", r8[op->spec.reg8]);
	    break;
	case fpreg:
	    fprintf(source, "st(%d)", op->spec.fpreg);
	    break;
	case mmxreg:
	    fprintf(source, "mm%d", op->spec.mmxreg);
	    break;
	case MEM:
	    switch (sz) {
	    case 1:
		fprintf(source, "byte ptr ");
		break;
	    case 2:
		fprintf(source, "word ptr ");
		break;
	    case 4:
		fprintf(source, "dword ptr ");
		break;
	    case 8:
		fprintf(source, "qword ptr ");
		break;
	    }
	    fprintf(source, "[");
	    if (op->spec.mem.disp) {
		if ((op->spec.mem.reg1 == NULL) && (op->spec.mem.reg2 == NULL))
		    fprintf(source, "null + ");
		source_emit_parm(op->spec.mem.disp, 0);
		gap = " + ";
	    }
	    if (op->spec.mem.reg1) {
		fprintf(source, "%s", gap);
		source_emit_parm(op->spec.mem.reg1, 0);
		gap = " + ";
	    }
	    if (op->spec.mem.reg2) {
		fprintf(source, "%s", gap);
		source_emit_parm(op->spec.mem.reg2, 0);
		fprintf(source, " * %d", op->spec.mem.scale);
	    }
	    fprintf(source, "]");
	    break;
	case LABEL:
	    if (op->spec.label.name)
		fprintf(source, "%s", op->spec.label.name);
	    else
		fprintf(source, "L%04d", op - ready);
	    break;
	}
    }
}

#endif /* SOURCE_TRAIL */

static unsigned char*
emit_modrm_code(unsigned char* cb,
		int regop,
		struct celem* op)
{
    switch (op->what) {
    case reg32:
    case reg8:
	*cb++ = 0xc0 | (regop << 3) | (op->spec.reg32);
	break;
    case MEM:
	if (op->spec.mem.disp && !op->spec.mem.reg1 && !op->spec.mem.reg2) {
	    assert(is_imm(op->spec.mem.disp));
	    *cb++ = 0x00 | (regop << 3) | 5;
	    *((long*)cb)++ = op->spec.mem.disp->spec.value;
	} else if (!op->spec.mem.disp && op->spec.mem.reg1 && !op->spec.mem.reg2) {
	    assert(is_reg32(op->spec.mem.reg1));
	    *cb++ = 0x00 | (regop << 3) | op->spec.mem.reg1->spec.reg32;
	} else if (op->spec.mem.disp && op->spec.mem.reg1 && !op->spec.mem.reg2 && op->spec.mem.reg1 != esp) {
	    int disp;

	    assert(is_imm(op->spec.mem.disp));
	    assert(is_reg32(op->spec.mem.reg1));
	    disp = op->spec.mem.disp->spec.value;

	    if (abs(disp) >= 128) {
	      *cb++ = 0x80 | (regop << 3) | op->spec.mem.reg1->spec.reg32;
	      *((long*)cb)++ = op->spec.mem.disp->spec.value;
	    } else {
	      *cb++ = 0x40 | (regop << 3) | op->spec.mem.reg1->spec.reg32;
	      *cb++ = op->spec.mem.disp->spec.value;
	    }
	} else {
	    static int scale_to_ss[9] = { 0, 0, 1, 1, 2, 2, 2, 2, 3 };
	    int ss;
	    int disp;

	    if (!op->spec.mem.reg1 && op->spec.mem.reg2) {
	      disp = op->spec.mem.disp ? op->spec.mem.disp->spec.value : 0;
	      *cb++ = 0x00 | (regop << 3) | 4;

	      /* Now for SIB */
	      ss = scale_to_ss[op->spec.mem.scale];

	      *cb++ =
		(ss << 6) |
		  (op->spec.mem.reg2->spec.reg32 << 3) |
		    5;
	      *((long*)cb)++ = op->spec.mem.disp->spec.value;

	    } else {
	      int needDisp;

	      disp = op->spec.mem.disp ? op->spec.mem.disp->spec.value : 0;
	      needDisp = (disp != 0) || (op->spec.mem.reg1 == ebp);

	      if (needDisp) {
		if (abs(disp) >= 128)
		  *cb++ = 0x80 | (regop << 3) | 4;
		else
		  *cb++ = 0x40 | (regop << 3) | 4;
	      } else
		*cb++ = 0x00 | (regop << 3) | 4;

	      /* Now for SIB */
	      for (ss = 0; (1 << ss) < op->spec.mem.scale; ss++)
		;

	      *cb++ =
		(ss << 6) |
		  ((op->spec.mem.reg2 ? op->spec.mem.reg2->spec.reg32 : 4) << 3) |
		    (op->spec.mem.reg1->spec.reg32);


	      if (needDisp) {
		if (abs(disp) >= 128)
		  *((long*)cb)++ = disp;
		else
		  *((signed char*)cb)++ = disp;
	      }
	    }
	}
    }
    return cb;
}

static unsigned char*
emit_arith_code(unsigned char* cb,
		int imm, 
		int r_rm,
		int rm_r,
		int second,
		struct celem* op1,
		struct celem* op2)
{
    if (is_imm(op2)) {
	if (is_reg8(op1))
	  *cb++ = imm - 1;
	else
	  *cb++ = imm;
	cb = emit_modrm_code(cb, second, op1);
	if (is_reg8(op1))
	  *cb++ = op2->spec.value;
	else
	  *((long*)cb)++ = op2->spec.value;
    } else if (is_reg(op2)) {
	if (is_reg32(op2))
	    *cb++ = rm_r;
	else
	    *cb++ = rm_r - 1;
	cb = emit_modrm_code(cb, op2->spec.reg32, op1);
    } else if (is_reg(op1)) {
	if (is_reg32(op1))
	    *cb++ = r_rm;
	else
	    *cb++ = r_rm - 1;
	cb = emit_modrm_code(cb, op1->spec.reg32, op2);
    } else
	assert(0);
    return cb;
}

/* Emit the code for a transfer-of-control (toc).
 * code8 is the bit-pattern for the 8-bit (-128 to +127)
 * version of the branch.
 * op1 is the target of the branch.
 *
 * Because 'og' is a single-pass assembler, forward references
 * need special handling.  A forward reference is something
 * like:
 *
 *    @   jmp   x
 *  ...
 *    @ x:
 *
 * This function checks to see if the target is a backwards reference
 * (i.e. the target's 'where' field is nonzero).  If it is, then
 * we can just emit the code as normal.
 * If the target is a forward reference, we append this reference to
 * the target's chain of outstanding references.  When the target
 * is resolved later, 'resolve' walks this list filling in all the
 * branch parameters.
 */
STATIC unsigned char*
emit_toc_code(__GLcontext* gc,
	      unsigned char* cb,
	      int code8,
	      struct celem* op1)
{
    int code32 = code8 + 0x10;

    if (op1->spec.label.where != NULL) {
	if (abs((op1->spec.label.where - cb)) < 127) {
	    *cb++ = code8;
	    *cb = (op1->spec.label.where - cb) - 1;
	    return cb + 1;
	} else {
	    *cb++ = 0x0f;
	    *cb++ = code32;
	    *(signed long*)cb = (op1->spec.label.where - cb) - 4;
	    return cb + 4;
	}
    } else {
	struct celem *new;
    
	/* We can write the head of the instruction now... */
	if (op1->spec.label.cl == rel_byte) {
	    *cb++ = code8;
	} else {
	    *cb++ = 0x0f;
	    *cb++ = code32;
	}
    
	/* Insert current position into chain, for later resolution */

	new = label(gc);
	new->spec.label.prev = op1->spec.label.prev;
	new->spec.label.where = cb;
	new->spec.label.cl = op1->spec.label.cl;
	op1->spec.label.prev = new;

	/* Leave a hole for the branch offset: 'resolve' will fill it in later*/
	return cb + ((op1->spec.label.cl == rel_byte) ? 1 : 4);
    }
}

/* Much like emit_toc_code(), except that the instruction set doesn't have
 * an 8-bit relative form of jmp: it's 32-bit relative only.
 */
unsigned char* emit_jmp_code(__GLcontext *gc,
			     unsigned char* cb,
			     struct celem* target)
{
    if (target->spec.label.where != NULL) {
	if (abs((target->spec.label.where - cb)) <= 127) {
	        *cb++ = 0xeb;
		*cb = (target->spec.label.where - cb) - 1;
		return cb + 1;
	} else {
	        *cb++ = 0xe9;
		*(signed long*)cb = (target->spec.label.where - cb) - 4;
		return cb + 4;
	}
    } else {
	struct celem *new;

	/* Insert current position into chain, for later resolution */
	*cb++ = 0xe9;

	new = label(gc);
	new->spec.label.prev = target->spec.label.prev;
	new->spec.label.where = cb;
	new->spec.label.cl = rel_word;
	target->spec.label.prev = new;

	return cb + 4;
    }
}

/*
 * Resolve a chain for forward references.  See emit_toc_code()
 * for a description of why we have to do this.
 */
STATIC void
resolve_chain(struct celem* ref, unsigned char* cb)
{
  if (ref) {
    assert(ref->what == LABEL);

    if (ref->spec.label.cl == rel_byte) {
      assert(abs((cb - ref->spec.label.where) - 1) <= 127);
      *(ref->spec.label.where) = (cb - ref->spec.label.where) - 1;
    } else {
      *(signed long*)(ref->spec.label.where) = (cb - ref->spec.label.where) - 4;
    }

    resolve_chain(ref->spec.label.prev, cb);
  }
}

void
resolve(__GLcontext *gc, struct celem* ref, unsigned char* cb)
{
#if SOURCE_TRAIL
    if (source)
      fprintf(source, "L%04d:\n", ref - ready);
#endif /* SOURCE_TRAIL */
    resolve_chain(ref->spec.label.prev, cb);
}

STATIC unsigned char*
emit_fp_code(unsigned char* cb,
	     int r1, int r2,
	     int m1, int m2,
	     struct celem* c)
{
    if (c->what == fpreg) {
	*cb++ = r1;
	*cb++ = r2 + c->spec.fpreg;
    } else {
	*cb++ = m1;
	cb = emit_modrm_code(cb, m2, c);
    }

    return cb;
}

#if MMX

/* Emit one of the main group of MMX instructions */
STATIC unsigned char*
emit_mmx_code(unsigned char* cb,
	      unsigned char k,
	      struct celem* r,
	      struct celem* c)
{
    *cb++ = 0x0f;
    *cb++ = k;

    if (c->what == mmxreg) {
	*cb++ = 0xc0 | (r->spec.mmxreg << 3) | (c->spec.mmxreg);
    } else {
	cb = emit_modrm_code(cb, r->spec.mmxreg, c);
    }

    return cb;
}

/* Emit one of the MMX shift instructions.  These have a special
 * format to accomodate immediates for shift distances.
 */
static unsigned char*
emit_mmx_s_code(unsigned char* cb,
		unsigned char k1,
		unsigned char k2,
		struct celem* r,
		struct celem* c)
{
    *cb++ = 0x0f;

    if (is_imm(c)) {
	*cb++ = k2;
	*cb++ = 0xc0 | (k1 & (0x7 << 3)) | r->spec.mmxreg;
	*cb++ = c->spec.value;
    } else if (c->what == mmxreg) {
	*cb++ = k1;
	*cb++ = 0xc0 | (r->spec.mmxreg << 3) | (c->spec.mmxreg);
    } else {
	*cb++ = k1;
	cb = emit_modrm_code(cb, r->spec.mmxreg, c);
    }
    
    return cb;
}
#endif /* MMX */

#if SOURCE_TRAIL
static void source_emit(__GLcontext *gc, struct celem* c)
{
    __GLOGState *ogs = &gc->ogState;
    FILE *source = ogs->source;
    int arity = 0;
    int sz = 0;
    int fprule = 0;

    switch (c->spec.insn.opcode) {
    case MOV:    fprintf(source, " mov "); arity = 2; break;
    case LEA:    fprintf(source, " lea "); arity = 2; break;
    case ADD:    fprintf(source,  " add "); arity = 2; break;
    case SUB:    fprintf(source,  " sub "); arity = 2; break;
    case AND:    fprintf(source,  " and "); arity = 2; break;
    case OR:    fprintf(source,  " or "); arity = 2; break;
    case XOR:    fprintf(source,  " xor "); arity = 2; break;
    case CMP:    fprintf(source,  " cmp "); arity = 2; break;
    case TEST:    fprintf(source,  " test "); arity = 2; break;
    case SHL:	fprintf(source, " shl "); arity = 2; break;
    case SHR:	fprintf(source, " shr "); arity = 2; break;
    case SAR:	fprintf(source, " sar "); arity = 2; break;

    case PUSH:    fprintf(source, " push "); arity = 1; break;
    case POP:    fprintf(source,  " pop "); arity = 1; break;
    case INC:    fprintf(source,  " inc "); arity = 1; sz = 4; break;
    case DEC:    fprintf(source,  " dec "); arity = 1; sz = 4; break;
    case IMUL:   fprintf(source,  " imul "); arity = 2; break;
    case JA:    fprintf(source,  " ja "); arity = 1; break;
    case JAE:    fprintf(source,  " jae "); arity = 1; break;
    case JB:    fprintf(source,  " jb "); arity = 1; break;
    case JBE:    fprintf(source,  " jbe "); arity = 1; break;
    case JGE:    fprintf(source,  " jge "); arity = 1; break;
    case JL:    fprintf(source,  " jl "); arity = 1; break;
    case JLE:    fprintf(source,  " jle "); arity = 1; break;
    case JG:    fprintf(source,  " jge "); arity = 1; break;
    case JE:    fprintf(source,  " je "); arity = 1; break;
    case JNE:    fprintf(source,  " jne "); arity = 1; break;
    case JO:    fprintf(source,  " jo "); arity = 1; break;
    case JNO:    fprintf(source,  " jno "); arity = 1; break;
    case JS:    fprintf(source,  " js "); arity = 1; break;
    case JNS:    fprintf(source,  " jns "); arity = 1; break;
    case CALL:	fprintf(source, " call "); arity = 1; sz = 4; break;
    case JMP:	fprintf(source, " jmp "); arity = 1; break;
    case JMPI:	fprintf(source, " jmp "); arity = 1; break;

    case FLD:	fprintf(source, " fld "); arity = 1; sz = 4; break;
    case FILD:	fprintf(source, " fild "); arity = 1; sz = 4; break;
    case FILDQ:	fprintf(source, " fild "); arity = 1; sz = 8; break;
    case FXCH:	fprintf(source, " fxch "); arity = 1; break;
    case FST:	fprintf(source, " fst "); arity = 1; sz = 4; break;
    case FSTP:	fprintf(source, " fstp "); arity = 1; sz = 4; break;
    case FSTPQ:	fprintf(source, " fstp "); arity = 1; sz = 8; break;
    case FISTP:	fprintf(source, " fistp "); arity = 1; sz = 4; break;
    case FIST:	fprintf(source, " fist "); arity = 1; sz = 4; break;
    case FISTPQ:	fprintf(source, " fistp "); arity = 1; sz = 8; break;
    case FMUL:	fprintf(source, " fmul "); arity = 1; sz = 4; fprule = 1; break;
    case FMULP:	fprintf(source, " fmulp "); arity = 2; sz = 4; fprule = 1; break;
    case FADD:	fprintf(source, " fadd "); arity = 1; sz = 4; fprule = 1; break;
    case FADDQ:	fprintf(source, " fadd "); arity = 1; sz = 8; fprule = 1; break;
    case FADDP:	fprintf(source, " faddp "); arity = 2; sz = 4; fprule = 1; break;
    case FSUBP:	fprintf(source, " fsubp "); arity = 2; sz = 4; fprule = 1; break;
    case FSUBRP:	fprintf(source, " fsubrp "); arity = 2; sz = 4; fprule = 1; break;
    case FSUB:	fprintf(source, " fsub "); arity = 1; sz = 4; fprule = 1; break;
    case FDIVR:	fprintf(source, " fdivr "); arity = 1; sz = 4; fprule = 1; break;
    case FDIV:	fprintf(source, " fdiv "); arity = 1; sz = 4; fprule = 1; break;
    case FLDCW:	fprintf(source, " fldcw "); arity = 1; sz = 2; break;
    case FSTCW:	fprintf(source, " fstcw "); arity = 1; sz = 2; break;
    case FLDQ:	fprintf(source, " fld "); arity = 1; sz = 8; break;
    case RDTSC:	fprintf(source, " rdtsc "); arity = 1; break;

    case SIZE16:	fprintf(source, " db 066h"); arity = 0; break;
    case INT3:	fprintf(source, " int 3"); arity = 0; break;
    case REP:	fprintf(source, " rep "); arity = 0; return;
    case MOVSD:	fprintf(source, " movsd"); arity = 0; break;
    case NOP:	fprintf(source, " nop"); arity = 0; break;
    case FLDPI:	fprintf(source, " fldpi "); arity = 0; break;
    case FABS:	fprintf(source, " fabs "); arity = 0; break;
    case RET:	fprintf(source, " ret "); arity = 0; break;

#if MMX
    case EMMS:	fprintf(source, " emms "); arity = 0; break;

#define FUN2(lo, up) \
    case up:	fprintf(source, " "## #lo ## " "); arity = 2; sz = 4; break;

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
    FUN2(pcmpgtw, PCMPGTW)
    FUN2(pcmpgtd, PCMPGTD)
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
    FUN2(psubw, PSUBW)
    FUN2(psubd, PSUBD)
    FUN2(psubsb, PSUBSB)
    FUN2(psubsw, PSUBSW)
    FUN2(psubusb, PSUBUSB)
    FUN2(psubusw, PSUBUSW)
    FUN2(punpckhbw, PUNPCKHBW)
    FUN2(punpckhdq, PUNPCKHDQ)
    FUN2(punpckhwd, PUNPCKHWD)
    FUN2(punpcklbw, PUNPCKLBW)
    FUN2(punpckldq, PUNPCKLDQ)
    FUN2(punpcklwd, PUNPCKLWD)
    FUN2(pxor, PXOR)

#undef FUN2
#endif /* MMX */

    default:
	assert(0);
    }

    if (fprule && (arity == 1) && c->spec.insn.op1->what == fpreg)
	fprintf(source, "st, ");
    if ((arity == 2) && is_reg8(c->spec.insn.op1) && (sz == 0))
	sz = 1;

    if (1 <= arity)
	source_emit_parm(c->spec.insn.op1, sz);
    if (2 <= arity) {
	fprintf(source, ",");
	source_emit_parm(c->spec.insn.op2, sz);
    }
}
#endif /* SOURCE_TRAIL */

/************************************************************************
			Instruction scheduler
************************************************************************/

static struct celem* block = NULL;

/* Spool code into the block, setting pointers appropriately.
*/
static void spool_code(__GLcontext *gc, struct celem* c)
{
    static struct celem* prev_insn;

    if (block == NULL) {
	block = c;
    } else {
	prev_insn->spec.insn.next = c;
    }
    prev_insn = c;
    c->spec.insn.next = NULL; /* Just in case this is the end */
}

static int can_v(struct celem* insn)
{
    switch (insn->spec.insn.opcode) {
	/* These things really can't run in the V pipe */
    case MOVD:
    case SHL:
    case SHR:
    case SAR:
	/* Pretend these can't go in V, so they'll go in U and pair
	 * with the branch. */
    case CMP:
    case TEST:
	return 0;
    }

    if ((JA <= insn->spec.insn.opcode) && (insn->spec.insn.opcode <= JNS))
	return 1;

    /* Scalar rules */
    if (is_binop(insn)) {

	if (is_mmxreg(insn->spec.insn.op1) && is_mmxreg(insn->spec.insn.op2))
	    return 1;

	if (is_mmxreg(insn->spec.insn.op1) && is_mem(insn->spec.insn.op2))
	    return 0;

	/* shifts can go in either pipe */
	if (is_mmxreg(insn->spec.insn.op1) && is_imm(insn->spec.insn.op2))
	    return 1;

	/* Almost true */
	return 1;
    }

    return 0;
}

/* Use vector.
 * First 8 bits are scalar registers.
 * Next 8 bits are MMX registers.
 * Next bit is flags.
 */

#define USE_FLAGS	0x10000

static unsigned long what_use(struct celem* op)
{
    if (op == NULL)
	return 0;

    switch (op->what) {
    case reg8:
	return (1 << (op->spec.reg8 & 3));
    case reg32:
	return (1 << op->spec.reg32);
    case mmxreg:
	return (8 << op->spec.mmxreg);
    }

    return 0;
}

static unsigned long mem_use(struct celem* op)
{
    if ((op == NULL) || !is_mem(op))
	return 0;
    else {
	return (what_use(op->spec.mem.reg1) | what_use(op->spec.mem.reg2));
    }
}

static unsigned long read_use(struct celem* insn)
{
    unsigned long memuse;

    memuse = mem_use(insn->spec.insn.op1) | mem_use(insn->spec.insn.op2);

    switch (insn->spec.insn.opcode) {
    case MOV:
    case MOVD:
    case MOVQ:
	return (memuse |
		what_use(insn->spec.insn.op2));
    case PUSH:
	return (memuse |
		what_use(insn->spec.insn.op1));
    }
    if (is_binop(insn))
	return (memuse |
		what_use(insn->spec.insn.op1) | 
		what_use(insn->spec.insn.op2));
    else if ((JA <= insn->spec.insn.opcode) && (insn->spec.insn.opcode <= JNS))
	return USE_FLAGS;

    return 0;
}

static unsigned long write_use(struct celem* insn)
{
    switch (insn->spec.insn.opcode) {
    case MOV:
    case MOVD:
    case MOVQ:
    case POP:
	return (what_use(insn->spec.insn.op1));
    case CMP:
    case TEST:
	return USE_FLAGS;
    }
    if (is_binop(insn))
	return (what_use(insn->spec.insn.op1));

    return 0;
}

static int depends_on(struct celem* P, struct celem* Q)
{
    if ((P == NULL) || (Q == NULL))
	return 0;

    /* P depends_on Q if
     *         Q's output is used as P's input.
     */
    if ((write_use(Q) & read_use(P)) != 0)
	return 1;

    return 0;
}

/* Same-cycle depends-on.  As above except that this
 * takes into account the special rule that CMP and TEST
 * pair with the branch.
 */
static int c_depends_on(struct celem* P, struct celem* Q)
{
    if ((P == NULL) || (Q == NULL))
	return 0;

    /* P depends_on Q if
     *         Q's output is used as P's input.
     */
    if (((write_use(Q) & read_use(P)) & ~USE_FLAGS) != 0)
	return 1;

    return 0;
}

/* P can_precede Q if
 *	P doesn't depend on Q
 *	Q doesn't depend on P
 */

static int can_precede(struct celem* P, struct celem* Q)
{
    if ((P == NULL) || (Q == NULL))
	return 1;

    if (depends_on(P, Q))
	return 0;
    if (depends_on(Q, P))
	return 0;

    return 1;
}

static int can_pair(struct celem *u, struct celem *v)
{
    if ((u == NULL) || (v == NULL))
	return 1;

    /* If this is an MMX shift insn, and there is already
     * a shift in the other pipe, this is a bad place.
     */
    if (is_mmx_shiftop(u) &&
	is_mmx_shiftop(v))
	return 0;

    /* Two ops that modify same 32-bit register pair cannot run
     * concurrently.  */
    if ((write_use(u) & write_use(v)) != 0)
	return 0;

    /* If v depends on u, it cannot pair (except for flags). */
    if (((write_use(u) & read_use(v)) & ~USE_FLAGS) != 0)
	return 0;

    return 1;
}

static unsigned char* Optimize_block(__GLcontext *gc, unsigned char* cb)
{
    struct celem* insn;
    struct celem** cooked;
    int i, where, good_place;

#if 0  /* Do we really optimize? */
    for (insn = block; insn != NULL; insn = insn->spec.insn.next)
	cb = emit_code(gc, cb, insn);
#else
    cooked = calloc(1000, sizeof(struct celem*));
    where = 0;

    for (insn = block; insn != NULL; insn = insn->spec.insn.next) {
	if (can_v(insn)) {
	    for (i = ((where + 1) | 1); i >= 0; i --) {
		if (!can_precede(insn, cooked[i + 1]))
		    break;

		/* If the preceding U-pipe insn writes our input,
		 * we can't use this slot.
		 */
		if (can_pair(insn, cooked[i ^ 1]) &&
		    (cooked[i] == NULL) &&
		    (((i & 1) == 0) || !c_depends_on(insn, cooked[i - 1])))
		    good_place = i;
	    }
	    /* 
	     * If this is an MMX operation, we're about to take the
	     * U-slot in an otherwise empty cycle, move over to the
	     * V-slot.
	     */
	    if (is_mmxop(insn) &&
		((good_place & 1) == 0) &&
		(cooked[good_place + 1] == NULL))
		good_place++;
	} else {
	    for (i = ((where + 4) & ~1); i >= 0; i -= 2) {
		if (((!can_precede(insn, cooked[i + 1])) ||
		     (!can_precede(insn, cooked[i + 2]))))
		    break;
		/* MMX store operations 'movq m64,mmX' and 'movd r/m32,mmX'
		 * need their source operand to be ready one cycle ahead.
		 */
		if (((insn->spec.insn.opcode == MOVQ) ||
		     (insn->spec.insn.opcode == MOVD)) &&
		    (is_reg32(insn->spec.insn.op1) || 
		     is_mem(insn->spec.insn.op1)) &&
		    (i >= 2) &&
		    (depends_on(insn, cooked[i - 1]) ||
		     depends_on(insn, cooked[i - 2]))) {
		     continue;
		}
		if (can_pair(insn, cooked[i ^ 1]) &&
		    cooked[i] == NULL)
		    good_place = i;
	    }
	}
	cooked[good_place] = insn;
	good_place++;
	if (where < good_place)
	    where = good_place;
    }

#if SOURCE_TRAIL
    {
	char name[80];
	static int serial = 0;

	sprintf(name, "/tmp/inner%03d", serial++);

	og_stream(name);
	for (i = 0; i < where; i++) {
	    if ((i & 1) == 0)
		fprintf(source, ";(%d)\n", i / 2);
	    if (cooked[i])
		cb = emit_code(gc, cb, cooked[i]);
	    else {
		fprintf(source, " ;\n");
	    }
	}
	og_stream(NULL);
    }
#else
	for (i = 0; i < where; i++) {
	    if (cooked[i])
		cb = emit_code(gc, cb, cooked[i]);
	    else
		/* cb = emit_code(gc, cb, nop()) */;
	}
#endif /* SOURCE_TRAIL */

    free(cooked);
#endif

    block = NULL;

    return cb;
}

unsigned char* emit_code(__GLcontext *gc, unsigned char* cb, struct celem* c)
{
    static int spooling = 0;
#if SOURCE_TRAIL
    unsigned char* cb_was = cb;

    if (source != NULL) {
      fprintf(source, "%p  ", cb);
      source_emit(c);
    }
#endif /* SOURCE_TRAIL */

    assert(c->what == INSN);

    if (spooling) {
	if (c->spec.insn.opcode == OPT_END) {
	    spooling = 0;
	    cb = Optimize_block(gc, cb);
	} else {
	    spool_code(gc, c);
	}
	return cb;
    }

    switch (c->spec.insn.opcode) {
    case OPT_BEGIN:
	spooling = 1;
	break;
    case MOV:
	if (is_reg32(c->spec.insn.op1) && is_imm(c->spec.insn.op2)) {
	  *cb++ = 0xb8 | (c->spec.insn.op1->spec.reg32);
	  *((long*)cb)++ = c->spec.insn.op2->spec.value;
	} else if (is_imm(c->spec.insn.op2)) {
	  *cb++ = 0xc7;
	  cb = emit_modrm_code(cb, 0, c->spec.insn.op1);
	  *((long*)cb)++ = c->spec.insn.op2->spec.value;
	} else if (is_reg32(c->spec.insn.op1)) {
	    *cb++ = 0x8b;
	    cb = emit_modrm_code(cb, c->spec.insn.op1->spec.reg32, c->spec.insn.op2);
	} else if (is_reg32(c->spec.insn.op2)) {
	    *cb++ = 0x89;
	    cb = emit_modrm_code(cb, c->spec.insn.op2->spec.reg32, c->spec.insn.op1);
	} else if (is_reg8(c->spec.insn.op1) && is_imm(c->spec.insn.op2)) {
	    *cb++ = 0xb0 | (c->spec.insn.op1->spec.reg8);
	    *((long*)cb)++ = c->spec.insn.op2->spec.value;
	} else if (is_reg8(c->spec.insn.op1)) {
	    *cb++ = 0x8a;
	    cb = emit_modrm_code(cb, c->spec.insn.op1->spec.reg8, c->spec.insn.op2);
	} else if (is_reg8(c->spec.insn.op2)) {
	    *cb++ = 0x88;
	    cb = emit_modrm_code(cb, c->spec.insn.op2->spec.reg8, c->spec.insn.op1);
	} else
	    assert(0);
	break;
    case MOVZX:
	*cb++ = 0x0f;
	*cb++ = 0xb6;
	cb = emit_modrm_code(cb, c->spec.insn.op1->spec.reg32, c->spec.insn.op2);
	break;
    case MOVSX:
	*cb++ = 0x0f;
	*cb++ = 0xbe;
	cb = emit_modrm_code(cb, c->spec.insn.op1->spec.reg32, c->spec.insn.op2);
	break;
    case MOVZXW:
	*cb++ = 0x0f;
	*cb++ = 0xb7;
	cb = emit_modrm_code(cb, c->spec.insn.op1->spec.reg32, c->spec.insn.op2);
	break;
    case MOVSXW:
	*cb++ = 0x0f;
	*cb++ = 0xbf;
	cb = emit_modrm_code(cb, c->spec.insn.op1->spec.reg32, c->spec.insn.op2);
	break;
    case LEA:
	*cb++ = 0x8d;
	cb = emit_modrm_code(cb, c->spec.insn.op1->spec.reg8, c->spec.insn.op2);
	break;

    case PUSH:
	if (is_imm(c->spec.insn.op1)) {
	  *cb++ = 0x68;
	  *((long*)cb)++ = c->spec.insn.op1->spec.value;
	} else if (is_reg32(c->spec.insn.op1)) {
	  *cb++ = 0x50 + c->spec.insn.op1->spec.reg32;
	} else {
	  *cb++ = 0xff;
	  cb = emit_modrm_code(cb, 6, c->spec.insn.op1);
	}
	break;

    case POP:
	if (is_reg32(c->spec.insn.op1)) {
	  *cb++ = 0x58 + c->spec.insn.op1->spec.reg32;
	} else {
	  *cb++ = 0x8f;
	  cb = emit_modrm_code(cb, 0, c->spec.insn.op1);
	}
	break;

    case INC:
	if (is_reg32(c->spec.insn.op1)) {
	  *cb++ = 0x40 + c->spec.insn.op1->spec.reg32;
	} else {
	  *cb++ = 0xff;
	  cb = emit_modrm_code(cb, 0, c->spec.insn.op1);
	}
	break;

    case DEC:
	if (is_reg32(c->spec.insn.op1)) {
	  *cb++ = 0x48 + c->spec.insn.op1->spec.reg32;
	} else {
	  *cb++ = 0xff;
	  cb = emit_modrm_code(cb, 1, c->spec.insn.op1);
	}
	break;

    case ADD:
	cb = emit_arith_code(cb, 0x81, 0x03, 0x01, 0, c->spec.insn.op1, c->spec.insn.op2);
	break;

    case ADC:
	cb = emit_arith_code(cb, 0x81, 0x13, 0x11, 2, c->spec.insn.op1, c->spec.insn.op2);
	break;

    case IMUL:
	if (is_imm(c->spec.insn.op2)) {
	  *cb++ = 0x69;
	  cb = emit_modrm_code(cb, c->spec.insn.op1->spec.reg32, c->spec.insn.op1);
	  *((long*)cb)++ = c->spec.insn.op2->spec.value;
	} else {
	  *cb++ = 0x0f;
	  *cb++ = 0xaf;
	  cb = emit_modrm_code(cb, c->spec.insn.op1->spec.reg32, c->spec.insn.op2);
	}
	break;

    case IDIV:
        *cb++ = 0xf7;
        cb = emit_modrm_code(cb, 7, c->spec.insn.op1);
	break;
    case SUB:
	cb = emit_arith_code(cb, 0x81, 0x2b, 0x29, 5, c->spec.insn.op1, c->spec.insn.op2);
	break;

    case SBB:
	cb = emit_arith_code(cb, 0x81, 0x1b, 0x19, 3, c->spec.insn.op1, c->spec.insn.op2);
	break;

    case AND:
	cb = emit_arith_code(cb, 0x81, 0x23, 0x21, 4, c->spec.insn.op1, c->spec.insn.op2);
	break;

    case OR:
	cb = emit_arith_code(cb, 0x81, 0x0b, 0x09, 1, c->spec.insn.op1, c->spec.insn.op2);
	break;

    case XOR:
	if (is_reg32(c->spec.insn.op1))
	  cb = emit_arith_code(cb, 0x81, 0x33, 0x31, 6, c->spec.insn.op1, c->spec.insn.op2);
	else
	  cb = emit_arith_code(cb, 0x80, 0x32, 0x30, 6, c->spec.insn.op1, c->spec.insn.op2);
	break;
    case CMP:
	if (is_byte_imm(c->spec.insn.op2)) {
	  cb = emit_arith_code(cb, 0x83, 0, 0, 7, c->spec.insn.op1, c->spec.insn.op2);
	  cb -= 3;
	} else {
	  cb = emit_arith_code(cb, 0x81, 0x3b, 0x39, 7, c->spec.insn.op1, c->spec.insn.op2);
	}
	break;
		    
    case TEST:
	cb = emit_arith_code(cb, 0xf7, -1, 0x85, 0, c->spec.insn.op1, c->spec.insn.op2);
	break;

    case JA:     cb = emit_toc_code(gc, cb, 0x77, c->spec.insn.op1);    break;
    case JB:     cb = emit_toc_code(gc, cb, 0x72, c->spec.insn.op1);    break;
    case JAE:    cb = emit_toc_code(gc, cb, 0x73, c->spec.insn.op1);    break;
    case JBE:    cb = emit_toc_code(gc, cb, 0x76, c->spec.insn.op1);    break;

    case JL:     cb = emit_toc_code(gc, cb, 0x7c, c->spec.insn.op1);    break;
    case JGE:    cb = emit_toc_code(gc, cb, 0x7d, c->spec.insn.op1);    break;
    case JLE:    cb = emit_toc_code(gc, cb, 0x7e, c->spec.insn.op1);    break;
    case JG:     cb = emit_toc_code(gc, cb, 0x7f, c->spec.insn.op1);    break;

    case JE:     cb = emit_toc_code(gc, cb, 0x74, c->spec.insn.op1);    break;
    case JNE:    cb = emit_toc_code(gc, cb, 0x75, c->spec.insn.op1);    break;

    case JO:     cb = emit_toc_code(gc, cb, 0x70, c->spec.insn.op1);    break;
    case JNO:    cb = emit_toc_code(gc, cb, 0x71, c->spec.insn.op1);    break;

    case JS:     cb = emit_toc_code(gc, cb, 0x78, c->spec.insn.op1);    break;
    case JNS:    cb = emit_toc_code(gc, cb, 0x79, c->spec.insn.op1);    break;

    case JMP:
	cb = emit_jmp_code(gc, cb, c->spec.insn.op1);
	break;

    case JMPI:
	*cb++ = 0xff;
	cb = emit_modrm_code(cb, 4, c->spec.insn.op1);
	break;

    case CALL:
	if (c->spec.insn.op1->what == LABEL) {
	  if (c->spec.insn.op1->spec.label.where != NULL) {
	    {
	      *cb++ = 0xe8;
	      *(signed long*)cb = (c->spec.insn.op1->spec.label.where - cb) - 4;
	      return cb + 4;
	    }
	  } else {
	    struct celem *new;

	    /* Insert current position into chain, for later resolution */
	    *cb++ = 0xe8;

	    new = label(gc);
	    new->spec.label.prev = c->spec.insn.op1->spec.label.prev;
	    new->spec.label.where = cb;
	    new->spec.label.cl = rel_word;
	    c->spec.insn.op1->spec.label.prev = new;

	    cb += 4;
	  }
	} else {
	  *cb++ = 0xff;
	  cb = emit_modrm_code(cb, 2, c->spec.insn.op1);
	}
	break;

    case ROL:
	assert(is_imm(c->spec.insn.op2));
	if (c->spec.insn.op2->spec.value > 0) {
	  assert(is_reg32(c->spec.insn.op1));
	  ;
	  *cb++ = 0xc1;
	  *cb++ = 0xc0 | (0 << 3) | c->spec.insn.op1->spec.reg32;
	  *cb++ = c->spec.insn.op2->spec.value;
	}
	break;

    case SHL:
	assert(is_imm(c->spec.insn.op2));
	if (c->spec.insn.op2->spec.value > 0) {
	  assert(is_reg32(c->spec.insn.op1));
	  ;
	  *cb++ = 0xc1;
	  *cb++ = 0xc0 | (4 << 3) | c->spec.insn.op1->spec.reg32;
	  *cb++ = c->spec.insn.op2->spec.value;
	}
	break;

    case SHR:
	assert(is_imm(c->spec.insn.op2));
	if (c->spec.insn.op2->spec.value > 0) {
	  assert(is_reg32(c->spec.insn.op1));
	  ;
	  *cb++ = 0xc1;
	  *cb++ = 0xc0 | (5 << 3) | c->spec.insn.op1->spec.reg32;
	  *cb++ = c->spec.insn.op2->spec.value;
	}
	break;

    case SAR:
	assert(is_imm(c->spec.insn.op2));
	if (c->spec.insn.op2->spec.value > 0) {
	  assert(is_reg32(c->spec.insn.op1));
	  *cb++ = 0xc1;
	  *cb++ = 0xc0 | (7 << 3) | c->spec.insn.op1->spec.reg32;
	  *cb++ = c->spec.insn.op2->spec.value;
	}
	break;

    case RETN:
	*cb++ = 0xc2;
	*((unsigned short*)cb)++ = c->spec.insn.op1->spec.value;
	break;
    case RET:
	*cb++ = 0xc3;
	break;
    case INT3:
	*cb++ = 0xcc;
	break;
    case WAIT:
	*cb++ = 0x9b;
	break;
    case SIZE16:
	*cb++ = 0x66;
	break;
    case REP:
	*cb++ = 0xf3;
	break;
    case MOVSD:
	*cb++ = 0xa5;
	break;
    case NOP:
	*cb++ = 0x90;
	break;

    case FLD:
	cb = emit_fp_code(cb, 0xd9, 0xc0, 0xd9, 0, c->spec.insn.op1);
	break;
    case FILD:
	cb = emit_fp_code(cb, 0, 0, 0xdb, 0, c->spec.insn.op1);
	break;
    case FILDQ:
	cb = emit_fp_code(cb, 0, 0, 0xdf, 5, c->spec.insn.op1);
	break;
    case FLDQ:
	cb = emit_fp_code(cb, 0xd9, 0xc0, 0xdd, 0, c->spec.insn.op1);
	break;
    case FXCH:
	cb = emit_fp_code(cb, 0xd9, 0xc8, 0, 0, c->spec.insn.op1);
	break;
    case FST:
	cb = emit_fp_code(cb, 0xdd, 0xd0, 0xd9, 2, c->spec.insn.op1);
	break;
    case FSTP:
	cb = emit_fp_code(cb, 0xdd, 0xd8, 0xd9, 3, c->spec.insn.op1);
	break;
    case FISTP:
	cb = emit_fp_code(cb, 0, 0, 0xdb, 3, c->spec.insn.op1);
	break;
    case FIST:
	cb = emit_fp_code(cb, 0, 0, 0xdb, 2, c->spec.insn.op1);
	break;
    case FLDCW:
	cb = emit_fp_code(cb, 0, 0, 0xd9, 5, c->spec.insn.op1);
	break;
    case FSTCW:
	cb = emit_fp_code(cb, 0, 0, 0xd9, 7, c->spec.insn.op1);
	break;
    case FISTPQ:
	cb = emit_fp_code(cb, 0, 0, 0xdf, 7, c->spec.insn.op1);
	break;
    case FSTPQ:
	/* If this is an abolsute address, we can make sure that
	 * it's qword aligned at assembly time.
	 */
	if (c->spec.insn.op1->spec.mem.disp)
	    assert((c->spec.insn.op1->spec.mem.disp->spec.value & 7) == 0);
	cb = emit_fp_code(cb, 0xdd, 0xd8, 0xdd, 3, c->spec.insn.op1);
	break;
    case FMUL:
	cb = emit_fp_code(cb, 0xd8, 0xc8, 0xd8, 1, c->spec.insn.op1);
	break;
    case FMULP:
	assert (c->spec.insn.op2 == st0);
	cb = emit_fp_code(cb, 0xde, 0xc8, 0, 0, c->spec.insn.op1);
	break;
    case FADD:
	cb = emit_fp_code(cb, 0xd8, 0xc0, 0xd8, 0, c->spec.insn.op1);
	break;
    case FADDQ:
	if (c->spec.insn.op1->spec.mem.disp)
	    assert((c->spec.insn.op1->spec.mem.disp->spec.value & 7) == 0);
	cb = emit_fp_code(cb, 0xd8, 0xc0, 0xdc, 0, c->spec.insn.op1);
	break;
    case FADDP:
	assert (c->spec.insn.op2 == st0);
	cb = emit_fp_code(cb, 0xde, 0xc0, 0, 0, c->spec.insn.op1);
	break;
    case FSUBP:
	assert (c->spec.insn.op2 == st0);
	cb = emit_fp_code(cb, 0xde, 0xe8, 0, 0, c->spec.insn.op1);
	break;
    case FSUBRP:
	assert (c->spec.insn.op2 == st0);
	cb = emit_fp_code(cb, 0xde, 0xe0, 0, 0, c->spec.insn.op1);
	break;
    case FSUB:
	cb = emit_fp_code(cb, 0xd8, 0xe0, 0xd8, 4, c->spec.insn.op1);
	break;
    case FDIVR:
	cb = emit_fp_code(cb, 0xd8, 0xf8, 0xd8, 7, c->spec.insn.op1);
	break;
    case FDIV:
	cb = emit_fp_code(cb, 0xd8, 0xf0, 0xd8, 6, c->spec.insn.op1);
	break;
    case FLDPI:
	*cb++ = 0xd9;
	*cb++ = 0xeb;
	break;
    case FABS:
	*cb++ = 0xd9;
	*cb++ = 0xe1;
	break;
    case FCHS:
	*cb++ = 0xd9;
	*cb++ = 0xe0;
	break;
    case RDTSC:
	*cb++ = 0x0f;
	*cb++ = 0x31;
	break;

#if MMX
#define FUN2(up, B)  case up: cb = emit_mmx_code(cb, B, c->spec.insn.op1, c->spec.insn.op2);	break;
#define FUN2S(up, B, I)  case up: cb = emit_mmx_s_code(cb, B, I, c->spec.insn.op1, c->spec.insn.op2);	break;

FUN2(PACKSSDW, 0x6b)
FUN2(PACKSSWB, 0x63)
FUN2(PACKUSWB, 0x67)
FUN2(PADDB, 0xfc)
FUN2(PADDD, 0xfe)
FUN2(PADDSB, 0xec)
FUN2(PADDSW, 0xed)
FUN2(PADDUSB, 0xdc)
FUN2(PADDUSW, 0xdd)
FUN2(PADDW, 0xfd)
FUN2(PAND, 0xdb)
FUN2(PANDN, 0xdf)
FUN2(PCMPEQB, 0x74)
FUN2(PCMPEQD, 0x76)
FUN2(PCMPEQW, 0x75)
FUN2(PCMPGTB, 0x64)
FUN2(PCMPGTW, 0x65)
FUN2(PCMPGTD, 0x66)
FUN2(PMADDWD, 0xf5)
FUN2(PMULHW, 0xe5)
FUN2(PMULLW, 0xd5)
FUN2(POR, 0xeb)

FUN2S(PSLLD, 0xf2, 0x72)
FUN2S(PSLLQ, 0xf3, 0x73)
FUN2S(PSLLW, 0xf1, 0x71)
FUN2S(PSRAD, 0xe2, 0x72)
FUN2S(PSRAW, 0xe1, 0x71)
FUN2S(PSRLD, 0xd2, 0x72)
FUN2S(PSRLQ, 0xd3, 0x73)
FUN2S(PSRLW, 0xd1, 0x71)

FUN2(PSUBB, 0xf8)
FUN2(PSUBW, 0xf9)
FUN2(PSUBD, 0xfa)
FUN2(PSUBSB, 0xe8)
FUN2(PSUBSW, 0xe9)
FUN2(PSUBUSB, 0xd8)
FUN2(PSUBUSW, 0xd9)
FUN2(PUNPCKHBW, 0x68)
FUN2(PUNPCKHDQ, 0x6a)
FUN2(PUNPCKHWD, 0x69)
FUN2(PUNPCKLBW, 0x60)
FUN2(PUNPCKLDQ, 0x62)
FUN2(PUNPCKLWD, 0x61)
FUN2(PXOR, 0xef)

    case EMMS:
	*cb++ = 0x0f;
        *cb++ = 0x77;
	break;
    case MOVD:
        *cb++ = 0x0f;
        if (is_mmxreg(c->spec.insn.op1)) {
	  *cb++ = 0x6e;
	  cb = emit_modrm_code(cb, 
			       c->spec.insn.op1->spec.mmxreg,
			       c->spec.insn.op2);
	} else {
	  assert(is_mmxreg(c->spec.insn.op2));
	  *cb++ = 0x7e;
	  cb = emit_modrm_code(cb, 
			       c->spec.insn.op2->spec.mmxreg,
			       c->spec.insn.op1);
	}
	break;
    case MOVQ:
	*cb++ = 0x0f;
	if (is_mmxreg(c->spec.insn.op1)) {
	  *cb++ = 0x6f;
	  if (is_mmxreg(c->spec.insn.op2)) {
	    *cb++ = 0xc0 |
	      (c->spec.insn.op1->spec.mmxreg << 3) |
		(c->spec.insn.op2->spec.mmxreg);
	  } else {
	    cb = emit_modrm_code(cb,
				 c->spec.insn.op1->spec.mmxreg,
				 c->spec.insn.op2);
	  }
	} else {
	  *cb++ = 0x7f;
	  cb = emit_modrm_code(cb,
			       c->spec.insn.op2->spec.mmxreg,
			       c->spec.insn.op1);
	}
        break;

#endif /* MMX */

    default:
	assert(0);
    }
#if SOURCE_TRAIL
    if (source)
      fprintf(source, "; (%d)\n", cb - cb_was);
#endif /* SOURCE_TRAIL */

    return cb;
}

void og_warmup(__GLcontext *gc)
{
    __GLOGState *ogs = &gc->ogState;

    /* Just restart the parse node pointer.
     */
    ogs->next = ogs->ready;
}

#if SOURCE_TRAIL

/* Use og_stream(filename) to start streaming to new file named
 * 'filename'.  og_stream(NULL) to stop streaming.
 */
void og_stream(__GLcontext *gc, char* file)
{
    __GLOGState *ogs = &gc->ogState;

    if (ogs->source)
	fclose(ogs->source);

    if (file)
	ogs->source = fopen(file, "wt");
    else
	ogs->source = NULL;
}
#endif	/* SOURCE_TRAIL */

/* Convenience function for generating floating-point constants.
 * Returns a pointer to a memory location holding the float 'v'.
 * Maintains a constant pool in the gc.
 */
struct celem* 
fpk(__GLcontext *gc, double v)
{
    __GLOGState *ogs = &gc->ogState;
    GLfloat *pool;
    GLuint i;

    pool = ogs->fpconstant_pool;
    for (i = 0; (i < ogs->fpconstant_next) && (pool[i] != v); i++)
	;

    if (i == ogs->fpconstant_next) {
	pool[ogs->fpconstant_next] = (float)v;

	/* Make sure that 'v' fits in a float without loss of accuracy */
	assert((double)pool[ogs->fpconstant_next] == v);

	ogs->fpconstant_next++;

	/* Can't realloc here because that might move previous entries,
	 * to which we have returned pointers.  Make a new pool
	 * that's double the size of the old pool.
	 */
	if (ogs->fpconstant_next == ogs->fpconstant_size) {
	    ogs->fpconstant_size *= 2;
	    ogs->fpconstant_next = 0;
	    ogs->fpconstant_pool = calloc(sizeof(float), ogs->fpconstant_size);
	}
    }

    return mem2(gc, value(gc, (int)&pool[i]), NULL);
}

/* Similar to the above: convenience function for generating 64-bit MMX
 * constants, specified as 4 words in Intel order (least significant word
 * first).
 */
struct celem*
mxk4(__GLcontext *gc,
     GLint v0, GLint v1, GLint v2, GLint v3)
{
    __GLOGState *ogs = &gc->ogState;
    union converter {
	double d;
	short s[4];
    } proto, *pool;
    GLuint i;
    GLboolean hit;

    pool = (union converter *)(ogs->mxconstant_pool);
    proto.s[0] = v0;
    proto.s[1] = v1;
    proto.s[2] = v2;
    proto.s[3] = v3;

    hit = GL_FALSE;

    i = 0;
    while (i < ogs->mxconstant_next) {
	if (memcmp(pool[i].s, proto.s, 8) == 0) {
	    hit = GL_TRUE;
	    break;
	}
	i++;
    }

    if (!hit) {
	pool[ogs->mxconstant_next] = proto;
	ogs->mxconstant_next++;

	if (ogs->mxconstant_next == ogs->mxconstant_size) {
	    ogs->mxconstant_size *= 2;
	    ogs->mxconstant_next = 0;
	    ogs->mxconstant_pool = calloc(sizeof(float), ogs->mxconstant_size);
	}
	
    }

    return mem2(gc, value(gc, (int)&pool[i]), NULL);
}

#endif  /* __GL_CODEGEN */
