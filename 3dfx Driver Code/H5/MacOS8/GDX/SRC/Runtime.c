//	runtime.c	-	Runtime Support routines for Metrowerks C++ for PowerPC
//
//	Copyright © 1995-1997 Metrowerks, Inc.  All Rights Reserved.
//

#pragma tvectors off
#pragma internal on
#define PPC_RT_ROUND_NEAREST 1
//
//	Prototypes
//

#ifdef __cplusplus
extern "C" {
#endif

unsigned long __cvt_fp2unsigned(double d);
unsigned long __uitrunc(double d);
void __ptr_glue(void);
void _ptrgl12(void);
void _ptrgl(void);
asm void _ptrglpas(void);
void __save_fpr_14(void);
void __save_fpr_15(void);
void __save_fpr_16(void);
void __save_fpr_17(void);
void __save_fpr_18(void);
void __save_fpr_19(void);
void __save_fpr_20(void);
void __save_fpr_21(void);
void __save_fpr_22(void);
void __save_fpr_23(void);
void __save_fpr_24(void);
void __save_fpr_25(void);
void __save_fpr_26(void);
void __save_fpr_27(void);
void __save_fpr_28(void);
void __save_fpr_29(void);
void __save_fpr_30(void);
void __save_fpr_31(void);
void _savef14(void);
void _savef15(void);
void _savef16(void);
void _savef17(void);
void _savef18(void);
void _savef19(void);
void _savef20(void);
void _savef21(void);
void _savef22(void);
void _savef23(void);
void _savef24(void);
void _savef25(void);
void _savef26(void);
void _savef27(void);
void _savef28(void);
void _savef29(void);
void _savef30(void);
void _savef31(void);
#if __ALTIVEC__
void __savev20(void);
void __savev21(void);
void __savev22(void);
void __savev23(void);
void __savev24(void);
void __savev25(void);
void __savev26(void);
void __savev27(void);
void __savev28(void);
void __savev29(void);
void __savev30(void);
void __savev31(void);
#endif
void __restore_fpr_14(void);
void __restore_fpr_15(void);
void __restore_fpr_16(void);
void __restore_fpr_17(void);
void __restore_fpr_18(void);
void __restore_fpr_19(void);
void __restore_fpr_20(void);
void __restore_fpr_21(void);
void __restore_fpr_22(void);
void __restore_fpr_23(void);
void __restore_fpr_24(void);
void __restore_fpr_25(void);
void __restore_fpr_26(void);
void __restore_fpr_27(void);
void __restore_fpr_28(void);
void __restore_fpr_29(void);
void __restore_fpr_30(void);
void __restore_fpr_31(void);
void _restf14(void);
void _restf15(void);
void _restf16(void);
void _restf17(void);
void _restf18(void);
void _restf19(void);
void _restf20(void);
void _restf21(void);
void _restf22(void);
void _restf23(void);
void _restf24(void);
void _restf25(void);
void _restf26(void);
void _restf27(void);
void _restf28(void);
void _restf29(void);
void _restf30(void);
void _restf31(void);
#if __ALTIVEC__
void __restv20(void);
void __restv21(void);
void __restv22(void);
void __restv23(void);
void __restv24(void);
void __restv25(void);
void __restv26(void);
void __restv27(void);
void __restv28(void);
void __restv29(void);
void __restv30(void);
void __restv31(void);
#endif
asm void __div2u(void);
asm void __div2i(void);
asm void __mod2u(void);
asm void __mod2i(void);
asm void __shl2i(void);
asm void __shr2u(void);
asm void __shr2i(void);
asm void __cvt_sll_dbl(void);
asm void __cvt_ull_dbl(void);
asm void __cvt_sll_flt(void);
asm void __cvt_ull_flt(void);
asm void __cvt_dbl_usll(void);


#ifdef __cplusplus
}
#endif


//
//	Private Data
//

static const unsigned long __constants[] = {
		0x00000000, 0x00000000,		// 0.0
		0x41F00000, 0x00000000,		// 2**32
		0x41E00000, 0x00000000,		// 2**31
};


//	__cvt_fp2unsigned	-	convert floating-point to 32-bit unsigned integer
//
//	Convert the floating-point value in F1 to a 32-bit unsigned integer
//	and return the result in R3.
//
//	This routine is copied from the Motorola PowerPC 602 Users Manual, pg. F-1.
//
//	We provide the synonym ".__uitrunc" for compatibility with PPCC.
//

asm unsigned long __cvt_fp2unsigned(register double d)
{
		entry	__uitrunc
		lwz		r4,__constants(RTOC)
		li		r3,0				// r3 = 0
		lfd		fp0,0(r4)			// fp0 = 0.0
		lfd		fp3,8(r4)			// fp3 = 2**32
		lfd		fp4,16(r4)			// fp4 = 2**31
		fcmpu	cr0,fp1,fp0			// cr0 = compare(fp1,0.0)
		fcmpu	cr6,fp1,fp3			// cr6 = compare(fp1,2**32)
		bltlr	cr0					// return 0x00000000 if input was < 0.0
		addi	r3,r3,-1
		bgelr	cr6					// return 0xFFFFFFFF if input was >= 2**32
		fcmpu	cr7,fp1,fp4			// cr7 = compare(fp1,2**31)
		fmr		fp2,fp1
		blt		cr7,@1				// use fp1 if < 2**31
		fsub	fp2,fp1,fp4			// subtract 2**31
@1		fctiwz	fp2,fp2				// convert float to 8-byte integer
		stfd	fp2,-8(SP)			// store 8-byte integer
		lwz		r3,-4(SP)			// load 4-byte integer
		bltlr	cr7
		addis	r3,r3,-32768		// add 2**31 if input was >= 2**31
		blr
}


//	__ptr_glue		-	glue for function calls through pointers
//
//	Call the function whose 2-word TVector address is in R12.
//

asm void __ptr_glue(void)
{
		smclass	GL
		lwz		r0,0(r12)
		stw		RTOC,20(SP)
		mtctr	r0
		lwz		RTOC,4(r12)
		bctr
}
		
		
//	_ptrgl12		-	PPCC-compatible version of __ptr_glue
//
//	Call the function whose 2-word TVector address is in R12.
//

asm void _ptrgl12(void)
{
		smclass	GL
		lwz		r0,0(r12)
		stw		RTOC,20(SP)
		mtctr	r0
		lwz		RTOC,4(r12)
		bctr
}


//	_ptrgl			-	PPCC-compatible version of __ptr_glue
//
//	Call the function whose 3-word TVector address is in R11. R12 must be
//	set to point to the TVector on exit. The 3rd word of the TVector is ignored.
//	(This is different from the AIX convention)
//

asm void _ptrgl(void)
{
		smclass	GL
		lwz		r0,0(r11)
		stw		RTOC,20(SP)
		mtctr	r0
		lwz		RTOC,4(r11)
		mr		r12,r11
		bctr
}


//	_ptrglpas		-	AIX-compatible version of __ptr_glue
//
//	Call the function whose 3-word TVector address is in R11.
//

asm void _ptrglpas(void)
{
		smclass	GL
		lwz		r0,0(r11)
		stw		RTOC,20(SP)
		mtctr	r0
		lwz		RTOC,4(r11)
		lwz		r11,8(r11)
		bctr
}


//
//	__save_fpr_XX	-	save FPR's XX through 31
//

static asm void __save_fpr(void)
{
	entry	__save_fpr_14
	entry	_savef14
		stfd	fp14,-144(SP)
	entry	__save_fpr_15
	entry	_savef15
		stfd	fp15,-136(SP)
	entry	__save_fpr_16
	entry	_savef16
		stfd	fp16,-128(SP)
	entry	__save_fpr_17
	entry	_savef17
		stfd	fp17,-120(SP)
	entry	__save_fpr_18
	entry	_savef18
		stfd	fp18,-112(SP)
	entry	__save_fpr_19
	entry	_savef19
		stfd	fp19,-104(SP)
	entry	__save_fpr_20
	entry	_savef20
		stfd	fp20,-96(SP)
	entry	__save_fpr_21
	entry	_savef21
		stfd	fp21,-88(SP)
	entry	__save_fpr_22
	entry	_savef22
		stfd	fp22,-80(SP)
	entry	__save_fpr_23
	entry	_savef23
		stfd	fp23,-72(SP)
	entry	__save_fpr_24
	entry	_savef24
		stfd	fp24,-64(SP)
	entry	__save_fpr_25
	entry	_savef25
		stfd	fp25,-56(SP)
	entry	__save_fpr_26
	entry	_savef26
		stfd	fp26,-48(SP)
	entry	__save_fpr_27
	entry	_savef27
		stfd	fp27,-40(SP)
	entry	__save_fpr_28
	entry	_savef28
		stfd	fp28,-32(SP)
	entry	__save_fpr_29
	entry	_savef29
		stfd	fp29,-24(SP)
	entry	__save_fpr_30
	entry	_savef30
		stfd	fp30,-16(SP)
	entry	__save_fpr_31
	entry	_savef31
		stfd	fp31,-8(SP)
		blr
}

#if __ALTIVEC__
static asm void _savevr(void)
{
			nofralloc
			machine altivec
			
	entry __savev20
	 		addi 	r12,r0,-192
		  	stvx 	v20,r12,r0
	entry __savev21 	
			addi 	r12,r0,-176
			stvx 	v21,r12,r0
	entry __savev22 	
			addi 	r12,r0,-160
			stvx 	v22,r12,r0
	entry __savev23	
			addi 	r12,r0,-144
			stvx 	v23,r12,r0
	entry __savev24 	
			addi 	r12,r0,-128
			stvx 	v24,r12,r0
	entry __savev25 	
			addi 	r12,r0,-112
			stvx 	v25,r12,r0
	entry __savev26 	
			addi 	r12,r0,-96
			stvx 	v26,r12,r0
	entry __savev27 	
			addi 	r12,r0,-80
			stvx 	v27,r12,r0
	entry __savev28 	
			addi 	r12,r0,-64
			stvx 	v28,r12,r0
	entry __savev29 	
			addi 	r12,r0,-48
			stvx 	v29,r12,r0
	entry __savev30 	
			addi 	r12,r0,-32
			stvx 	v30,r12,r0
	entry __savev31 	
			addi 	r12,r0,-16
		 	stvx 	v31,r12,r0 
			blr 
}
#endif		

//
//	__restore_fpr_XX	-	restore FPR's XX through 31
//

static asm void __restore_fpr(void)
{
	nofralloc
		
	entry	__restore_fpr_14
	entry	_restf14
		lfd		fp14,-144(SP)
	entry	__restore_fpr_15
	entry	_restf15
		lfd		fp15,-136(SP)
	entry	__restore_fpr_16
	entry	_restf16
		lfd		fp16,-128(SP)
	entry	__restore_fpr_17
	entry	_restf17
		lfd		fp17,-120(SP)
	entry	__restore_fpr_18
	entry	_restf18
		lfd		fp18,-112(SP)
	entry	__restore_fpr_19
	entry	_restf19
		lfd		fp19,-104(SP)
	entry	__restore_fpr_20
	entry	_restf20
		lfd		fp20,-96(SP)
	entry	__restore_fpr_21
	entry	_restf21
		lfd		fp21,-88(SP)
	entry	__restore_fpr_22
	entry	_restf22
		lfd		fp22,-80(SP)
	entry	__restore_fpr_23
	entry	_restf23
		lfd		fp23,-72(SP)
	entry	__restore_fpr_24
	entry	_restf24
		lfd		fp24,-64(SP)
	entry	__restore_fpr_25
	entry	_restf25
		lfd		fp25,-56(SP)
	entry	__restore_fpr_26
	entry	_restf26
		lfd		fp26,-48(SP)
	entry	__restore_fpr_27
	entry	_restf27
		lfd		fp27,-40(SP)
	entry	__restore_fpr_28
	entry	_restf28
		lfd		fp28,-32(SP)
	entry	__restore_fpr_29
	entry	_restf29
		lfd		fp29,-24(SP)
	entry	__restore_fpr_30
	entry	_restf30
		lfd		fp30,-16(SP)
	entry	__restore_fpr_31
	entry	_restf31
		lfd		fp31,-8(SP)
		blr
}

#if __ALTIVEC__
static asm void _restorevr(void)
{
			nofralloc
			machine altivec
	entry __restv20	
				addi 	r12,r0,-192
				lvx 	v20,r12,r0
	entry __restv21 	
				addi 	r12,r0,-176
				lvx 	v21,r12,r0
	entry __restv22 	
				addi 	r12,r0,-160
				lvx 	v22,r12,r0
	entry __restv23 	
				addi 	r12,r0,-144
				lvx 	v23,r12,r0
	entry __restv24 	
				addi 	r12,r0,-128
				lvx 	v24,r12,r0
	entry __restv25 	
				addi 	r12,r0,-112
				lvx 	v25,r12,r0
	entry __restv26 	
				addi 	r12,r0,-96
				lvx 	v26,r12,r0
	entry __restv27 	
				addi 	r12,r0,-80
				lvx 	v27,r12,r0
	entry __restv28 	
				addi 	r12,r0,-64
				lvx 	v28,r12,r0
	entry __restv29 	
				addi 	r12,r0,-48
				lvx 	v29,r12,r0
	entry __restv30 	
				addi 	r12,r0,-32
				lvx 	v30,r12,r0
	entry __restv31 	
				addi 	r12,r0,-16
				lvx 	v31,r12,r0
				blr
}
#endif


//	__div2u	-	64-bit unsigned integer divide on 32-bit PowerPC	
//
//	This routine copied and modified slightly from figure 3-40 of
//  "The PowerPC Compiler Writer's Guide."
//
//	(R3:R4) = (R3:R4) / (R5:R6)		(64b) = (64b / 64b) 
//	quo			dvd		dvs 
// 
//	Code comment notation: 
//	msw = most-significant (high-order) word, i.e. bits 0..31 
//	lsw = least-significant (low-order) word, i.e. bits 32..63 
//	LZ = Leading Zeroes 
//	SD = Significant Digits 
// 
//	R3:R4 = dvd (input dividend); quo (output quotient) 
//	R5:R6 = dvs (input divisor)
// 
//	R7:R8 = tmp 
//

asm void __div2u(void)
{
       // count the number of leading 0s in the dividend 
       cmpwi	cr0,r3,0		// dvd.msw == 0? 
       cntlzw	r0,r3			// R0 = dvd.msw.LZ 
       cntlzw	r9,r4			// R9 = dvd.lsw.LZ 
       bne		cr0,lab1		// if(dvd.msw == 0) dvd.LZ = dvd.msw.LZ 
       addi		r0,r9,32		// dvd.LZ = dvd.lsw.LZ + 32 
 lab1: 
       // count the number of leading 0s in the divisor 
       cmpwi	cr0,r5,0		// dvd.msw == 0? 
       cntlzw	r9,r5			// R9 = dvs.msw.LZ 
       cntlzw	r10,r6			// R10 = dvs.lsw.LZ 
       bne		cr0,lab2		// if(dvs.msw == 0) dvs.LZ = dvs.msw.LZ 
       addi		r9,r10,32		// dvs.LZ = dvs.lsw.LZ + 32 
 lab2: 
       // determine shift amounts to minimize the number of iterations 
       cmpw		cr0,r0,r9		// compare dvd.LZ to dvs.LZ 
       subfic	r10,r0,64		// R10 = dvd.SD 
       bgt		cr0,lab9		// if(dvs > dvd) quotient = 0 
       addi		r9,r9,1			// ++dvs.LZ (or --dvs.SD) 
       subfic	r9,r9,64		// R9 = dvs.SD 
       add		r0,r0,r9		// (dvd.LZ + dvs.SD) = left shift of dvd for 
                         		// initial dvd 
       subf		r9,r9,r10		// (dvd.SD - dvs.SD) = right shift of dvd for 
                         		// initial tmp 
       mtctr	r9				// number of iterations = dvd.SD - dvs.SD
        
       // R7:R8 = R3:R4 >> R9 
       cmpwi	cr0,r9,32		// compare R9 to 32 
       addi		r7,r9,-32 
       blt		cr0,lab3		// if(R9 < 32) jump to lab3 
       srw		r8,r3,r7		// tmp.lsw = dvd.msw >> (R9 - 32) 
       li		r7,0			// tmp.msw = 0 
       b		lab4 
 lab3: 
       srw		r8,r4,r9		// R8 = dvd.lsw >> R9 
       subfic	r7,r9,32 
       slw		r7,r3,r7		// R7 = dvd.msw << 32 - R9 
       or		r8,r8,r7		// tmp.lsw = R8 | R7 
       srw		r7,r3,r9		// tmp.msw = dvd.msw >> R9 
 lab4: 
       // R3:R4 = R3:R4 << R0 
       cmpwi	cr0,r0,32		// compare R0 to 32 
       addic	r9,r0,-32 
       blt		cr0,lab5		// if(R0 < 32) jump to lab5 
       slw		r3,r4,r9		// dvd.msw = dvd.lsw << R9 
       li		r4,0			// dvd.lsw = 0 
       b		lab6 
 lab5: 
       slw		r3,r3,r0		// R3 = dvd.msw << R0 
       subfic	r9,r0,32 
       srw		r9,r4,r9		// R9 = dvd.lsw >> 32 - R0 
       or		r3,r3,r9		// dvd.msw = R3 | R9 
       slw		r4,r4,r0		// dvd.lsw = dvd.lsw << R0 
 lab6: 
       // restoring division shift and subtract loop 
       li		r10,-1			// R10 = -1 
       addic	r7,r7,0			// clear carry bit before loop starts 
 lab7: 
       // tmp:dvd is considered one large register 
       // each portion is shifted left 1 bit by adding it to itself 
       // adde sums the carry from the previous and creates a new carry 
       adde		r4,r4,r4		// shift dvd.lsw left 1 bit 
       adde		r3,r3,r3		// shift dvd.msw to left 1 bit 
       adde		r8,r8,r8		// shift tmp.lsw to left 1 bit 
       adde		r7,r7,r7		// shift tmp.msw to left 1 bit 
       subfc	r0,r6,r8		// tmp.lsw - dvs.lsw 
       subfe.	r9,r5,r7		// tmp.msw - dvs.msw 
       blt		cr0,lab8		// if(result < 0) clear carry bit 
       mr		r8,r0			// move lsw 
       mr		r7,r9			// move msw 
       addic	r0,r10,1		// set carry bit 
 lab8: 
       bdnz		lab7
       
       // write quotient 
       adde		r4,r4,r4		// quo.lsw (lsb = CA) 
       adde		r3,r3,r3		// quo.msw (lsb from lsw) 
       blr						// return 
 lab9: 
       // Quotient is 0 (dvs > dvd) 
       li		r4,0			// dvd.lsw = 0 
       li		r3,0			// dvd.msw = 0 
       blr						// return 
}


//	__div2i	-	64-bit signed integer divide on 32-bit PowerPC	
//
//	This routine copied and modified slightly from figure 3-40 of
//  "The PowerPC Compiler Writer's Guide."
//
//	(R3:R4) = (R3:R4) / (R5:R6)		(64b) = (64b / 64b) 
//	quo			dvd		dvs 
// 
//	Code comment notation: 
//	msw = most-significant (high-order) word, i.e. bits 0..31 
//	lsw = least-significant (low-order) word, i.e. bits 32..63 
//	LZ = Leading Zeroes 
//	SD = Significant Digits 
// 
//	R3:R4 = dvd (input dividend); quo (output quotient) 
//	R5:R6 = dvs (input divisor)
// 
//	R7:R8 = tmp 
//

asm void __div2i(void)
{
       // first we need the absolute values of both operands. we also
       // remember the signs so that we can adjust the result later.
       rlwinm.  r9,r3,0,0,0
       beq      cr0,positive1
       subfic   r4,r4,0
       subfze   r3,r3
positive1:
       stw      r9,24(r1)
       rlwinm.  r9,r5,0,0,0
       beq      cr0,positive2
       subfic   r6,r6,0
       subfze   r5,r5
positive2:
       stw      r9,28(r1)
       
       
       // count the number of leading 0s in the dividend 
       cmpwi	cr0,r3,0		// dvd.msw == 0? 
       cntlzw	r0,r3			// R0 = dvd.msw.LZ 
       cntlzw	r9,r4			// R9 = dvd.lsw.LZ 
       bne		cr0,lab1		// if(dvd.msw == 0) dvd.LZ = dvd.msw.LZ 
       addi		r0,r9,32		// dvd.LZ = dvd.lsw.LZ + 32 
 lab1: 
       // count the number of leading 0s in the divisor 
       cmpwi	cr0,r5,0		// dvd.msw == 0? 
       cntlzw	r9,r5			// R9 = dvs.msw.LZ 
       cntlzw	r10,r6			// R10 = dvs.lsw.LZ 
       bne		cr0,lab2		// if(dvs.msw == 0) dvs.LZ = dvs.msw.LZ 
       addi		r9,r10,32		// dvs.LZ = dvs.lsw.LZ + 32 
 lab2: 
       // determine shift amounts to minimize the number of iterations 
       cmpw		cr0,r0,r9		// compare dvd.LZ to dvs.LZ 
       subfic	r10,r0,64		// R10 = dvd.SD 
       bgt		cr0,lab9		// if(dvs > dvd) quotient = 0 
       addi		r9,r9,1			// ++dvs.LZ (or --dvs.SD) 
       subfic	r9,r9,64		// R9 = dvs.SD 
       add		r0,r0,r9		// (dvd.LZ + dvs.SD) = left shift of dvd for 
                         		// initial dvd 
       subf		r9,r9,r10		// (dvd.SD - dvs.SD) = right shift of dvd for 
                         		// initial tmp 
       mtctr	r9				// number of iterations = dvd.SD - dvs.SD
        
       // R7:R8 = R3:R4 >> R9 
       cmpwi	cr0,r9,32		// compare R9 to 32 
       addi		r7,r9,-32 
       blt		cr0,lab3		// if(R9 < 32) jump to lab3 
       srw		r8,r3,r7		// tmp.lsw = dvd.msw >> (R9 - 32) 
       li		r7,0			// tmp.msw = 0 
       b		lab4 
 lab3: 
       srw		r8,r4,r9		// R8 = dvd.lsw >> R9 
       subfic	r7,r9,32 
       slw		r7,r3,r7		// R7 = dvd.msw << 32 - R9 
       or		r8,r8,r7		// tmp.lsw = R8 | R7 
       srw		r7,r3,r9		// tmp.msw = dvd.msw >> R9 
 lab4: 
       // R3:R4 = R3:R4 << R0 
       cmpwi	cr0,r0,32		// compare R0 to 32 
       addic	r9,r0,-32 
       blt		cr0,lab5		// if(R0 < 32) jump to lab5 
       slw		r3,r4,r9		// dvd.msw = dvd.lsw << R9 
       li		r4,0			// dvd.lsw = 0 
       b		lab6 
 lab5: 
       slw		r3,r3,r0		// R3 = dvd.msw << R0 
       subfic	r9,r0,32 
       srw		r9,r4,r9		// R9 = dvd.lsw >> 32 - R0 
       or		r3,r3,r9		// dvd.msw = R3 | R9 
       slw		r4,r4,r0		// dvd.lsw = dvd.lsw << R0 
 lab6: 
       // restoring division shift and subtract loop 
       li		r10,-1			// R10 = -1 
       addic	r7,r7,0			// clear carry bit before loop starts 
 lab7: 
       // tmp:dvd is considered one large register 
       // each portion is shifted left 1 bit by adding it to itself 
       // adde sums the carry from the previous and creates a new carry 
       adde		r4,r4,r4		// shift dvd.lsw left 1 bit 
       adde		r3,r3,r3		// shift dvd.msw to left 1 bit 
       adde		r8,r8,r8		// shift tmp.lsw to left 1 bit 
       adde		r7,r7,r7		// shift tmp.msw to left 1 bit 
       subfc	r0,r6,r8		// tmp.lsw - dvs.lsw 
       subfe.	r9,r5,r7		// tmp.msw - dvs.msw 
       blt		cr0,lab8		// if(result < 0) clear carry bit 
       mr		r8,r0			// move lsw 
       mr		r7,r9			// move msw 
       addic	r0,r10,1		// set carry bit 
 lab8: 
       bdnz		lab7
       
       // write quotient 
       adde		r4,r4,r4		// quo.lsw (lsb = CA) 
       adde		r3,r3,r3		// quo.msw (lsb from lsw) 
       
       // now adjust result based on sign of original DVD and DVS
       lwz		r9,24(r1)
       lwz		r10,28(r1)
       xor.		r7,r9,r10
       beq		cr0,no_adjust	// sign of DVD and DVS same, no adjust
       cmpwi	cr0,r9,0		
       // sign of DVD and DVS opposite, we negate result
       subfic   r4,r4,0
       subfze   r3,r3       
 no_adjust:
       blr						// return 
 
 lab9: 
       // Quotient is 0 (dvs > dvd) 
       li		r4,0			// dvd.lsw = 0 
       li		r3,0			// dvd.msw = 0 
       blr						// return 
}


//	__mod2u	-	64-bit unsigned integer mod on 32-bit PowerPC	
//
//	This routine copied and modified slightly from figure 3-40 of
//  "The PowerPC Compiler Writer's Guide."
//
//	(R3:R4) = (R3:R4) / (R5:R6)		(64b) = (64b / 64b) 
//	rem			dvd		dvs 
// 
// 
//	Code comment notation: 
//	msw = most-significant (high-order) word, i.e. bits 0..31 
//	lsw = least-significant (low-order) word, i.e. bits 32..63 
//	LZ = Leading Zeroes 
//	SD = Significant Digits 
// 
//	R3:R4 = dvd (input dividend); rem (output remainder) 
//	R5:R6 = dvs (input divisor)
// 
//	R7:R8 = tmp 
//

asm void __mod2u(void)
{
       // count the number of leading 0s in the dividend 
       cmpwi	cr0,r3,0		// dvd.msw == 0? 
       cntlzw	r0,r3			// R0 = dvd.msw.LZ 
       cntlzw	r9,r4			// R9 = dvd.lsw.LZ 
       bne		cr0,lab1		// if(dvd.msw == 0) dvd.LZ = dvd.msw.LZ 
       addi		r0,r9,32		// dvd.LZ = dvd.lsw.LZ + 32 
 lab1: 
       // count the number of leading 0s in the divisor 
       cmpwi	cr0,r5,0		// dvd.msw == 0? 
       cntlzw	r9,r5			// R9 = dvs.msw.LZ 
       cntlzw	r10,r6			// R10 = dvs.lsw.LZ 
       bne		cr0,lab2		// if(dvs.msw == 0) dvs.LZ = dvs.msw.LZ 
       addi		r9,r10,32		// dvs.LZ = dvs.lsw.LZ + 32 
 lab2: 
       // determine shift amounts to minimize the number of iterations 
       cmpw		cr0,r0,r9		// compare dvd.LZ to dvs.LZ 
       subfic	r10,r0,64		// R10 = dvd.SD 
       bgt		cr0,lab9		// if(dvs > dvd) quotient = 0 
       addi		r9,r9,1			// ++dvs.LZ (or --dvs.SD) 
       subfic	r9,r9,64		// R9 = dvs.SD 
       add		r0,r0,r9		// (dvd.LZ + dvs.SD) = left shift of dvd for 
                         		// initial dvd 
       subf		r9,r9,r10		// (dvd.SD - dvs.SD) = right shift of dvd for 
                         		// initial tmp 
       mtctr	r9				// number of iterations = dvd.SD - dvs.SD
        
       // R7:R8 = R3:R4 >> R9 
       cmpwi	cr0,r9,32		// compare R9 to 32 
       addi		r7,r9,-32 
       blt		cr0,lab3		// if(R9 < 32) jump to lab3 
       srw		r8,r3,r7		// tmp.lsw = dvd.msw >> (R9 - 32) 
       li		r7,0			// tmp.msw = 0 
       b		lab4 
 lab3: 
       srw		r8,r4,r9		// R8 = dvd.lsw >> R9 
       subfic	r7,r9,32 
       slw		r7,r3,r7		// R7 = dvd.msw << 32 - R9 
       or		r8,r8,r7		// tmp.lsw = R8 | R7 
       srw		r7,r3,r9		// tmp.msw = dvd.msw >> R9 
 lab4: 
       // R3:R4 = R3:R4 << R0 
       cmpwi	cr0,r0,32		// compare R0 to 32 
       addic	r9,r0,-32 
       blt		cr0,lab5		// if(R0 < 32) jump to lab5 
       slw		r3,r4,r9		// dvd.msw = dvd.lsw << R9 
       li		r4,0			// dvd.lsw = 0 
       b		lab6 
 lab5: 
       slw		r3,r3,r0		// R3 = dvd.msw << R0 
       subfic	r9,r0,32 
       srw		r9,r4,r9		// R9 = dvd.lsw >> 32 - R0 
       or		r3,r3,r9		// dvd.msw = R3 | R9 
       slw		r4,r4,r0		// dvd.lsw = dvd.lsw << R0 
 lab6: 
       // restoring division shift and subtract loop 
       li		r10,-1			// R10 = -1 
       addic	r7,r7,0			// clear carry bit before loop starts 
 lab7: 
       // tmp:dvd is considered one large register 
       // each portion is shifted left 1 bit by adding it to itself 
       // adde sums the carry from the previous and creates a new carry 
       adde		r4,r4,r4		// shift dvd.lsw left 1 bit 
       adde		r3,r3,r3		// shift dvd.msw to left 1 bit 
       adde		r8,r8,r8		// shift tmp.lsw to left 1 bit 
       adde		r7,r7,r7		// shift tmp.msw to left 1 bit 
       subfc	r0,r6,r8		// tmp.lsw - dvs.lsw 
       subfe.	r9,r5,r7		// tmp.msw - dvs.msw 
       blt		cr0,lab8		// if(result < 0) clear carry bit 
       mr		r8,r0			// move lsw 
       mr		r7,r9			// move msw 
       addic	r0,r10,1		// set carry bit 
 lab8: 
       bdnz		lab7
       
       // write remainder 
       mr		r4,r8			// rem.lsw 
       mr		r3,r7			// rem.msw 
       blr						// return 
 lab9: 
       // remainder already in r3:r4 (quotient is 0 (dvs > dvd)) 
       blr						// return 
}


//	__mod2i	-	64-bit signed integer mod on 32-bit PowerPC
//
//	This routine copied and modified slightly from figure 3-40 of
//  "The PowerPC Compiler Writer's Guide."
//
//	(R3:R4) = (R3:R4) / (R5:R6)		(64b) = (64b / 64b)
//	rem			dvd		dvs
//
//
//	Code comment notation:
//	msw = most-significant (high-order) word, i.e. bits 0..31
//	lsw = least-significant (low-order) word, i.e. bits 32..63
//	LZ = Leading Zeroes
//	SD = Significant Digits
//
//	R3:R4 = dvd (input dividend); rem (output remainder)
//	R5:R6 = dvs (input divisor)
//
//	R7:R8 = tmp
//

asm void __mod2i(void)
{
       // remember sign of dvd in condition register 7
       cmpwi	cr7,r3,0
       bge	cr7,positive1
       subfic   r4,r4,0
       subfze   r3,r3
positive1:
       cmpwi	cr0,r5,0
       bge      cr0,positive2
       subfic   r6,r6,0
       subfze   r5,r5
positive2:


       // count the number of leading 0s in the dividend
       cmpwi	cr0,r3,0		// dvd.msw == 0?
       cntlzw	r0,r3			// R0 = dvd.msw.LZ
       cntlzw	r9,r4			// R9 = dvd.lsw.LZ
       bne	cr0,lab1			// if(dvd.msw == 0) dvd.LZ = dvd.msw.LZ
       addi	r0,r9,32			// dvd.LZ = dvd.lsw.LZ + 32
 lab1:
       // count the number of leading 0s in the divisor
       cmpwi	cr0,r5,0		// dvd.msw == 0?
       cntlzw	r9,r5			// R9 = dvs.msw.LZ
       cntlzw	r10,r6			// R10 = dvs.lsw.LZ
       bne	cr0,lab2			// if(dvs.msw == 0) dvs.LZ = dvs.msw.LZ
       addi	r9,r10,32			// dvs.LZ = dvs.lsw.LZ + 32
 lab2:
       // determine shift amounts to minimize the number of iterations
       cmpw	cr0,r0,r9			// compare dvd.LZ to dvs.LZ
       subfic	r10,r0,64		// R10 = dvd.SD
       bgt	cr0,lab9			// if(dvs > dvd) quotient = 0
       addi	r9,r9,1				// ++dvs.LZ (or --dvs.SD)
       subfic	r9,r9,64		// R9 = dvs.SD
       add	r0,r0,r9			// (dvd.LZ + dvs.SD) = left shift of dvd for
                         		// initial dvd
       subf	r9,r9,r10			// (dvd.SD - dvs.SD) = right shift of dvd for
                         		// initial tmp
       mtctr	r9				// number of iterations = dvd.SD - dvs.SD

       // R7:R8 = R3:R4 >> R9
       cmpwi	cr0,r9,32		// compare R9 to 32
       addi	r7,r9,-32
       blt	cr0,lab3			// if(R9 < 32) jump to lab3
       srw	r8,r3,r7			// tmp.lsw = dvd.msw >> (R9 - 32)
       li	r7,0				// tmp.msw = 0
       b	lab4
 lab3:
       srw	r8,r4,r9			// R8 = dvd.lsw >> R9
       subfic	r7,r9,32
       slw	r7,r3,r7			// R7 = dvd.msw << 32 - R9
       or	r8,r8,r7			// tmp.lsw = R8 | R7
       srw	r7,r3,r9			// tmp.msw = dvd.msw >> R9
 lab4:
       // R3:R4 = R3:R4 << R0
       cmpwi	cr0,r0,32		// compare R0 to 32
       addic	r9,r0,-32
       blt	cr0,lab5			// if(R0 < 32) jump to lab5
       slw	r3,r4,r9			// dvd.msw = dvd.lsw << R9
       li	r4,0				// dvd.lsw = 0
       b	lab6
 lab5:
       slw	r3,r3,r0			// R3 = dvd.msw << R0
       subfic	r9,r0,32
       srw	r9,r4,r9			// R9 = dvd.lsw >> 32 - R0
       or	r3,r3,r9			// dvd.msw = R3 | R9
       slw	r4,r4,r0			// dvd.lsw = dvd.lsw << R0
 lab6:
       // restoring division shift and subtract loop
       li	r10,-1				// R10 = -1
       addic	r7,r7,0			// clear carry bit before loop starts
 lab7:
       // tmp:dvd is considered one large register
       // each portion is shifted left 1 bit by adding it to itself
       // adde sums the carry from the previous and creates a new carry
       adde	r4,r4,r4			// shift dvd.lsw left 1 bit
       adde	r3,r3,r3			// shift dvd.msw to left 1 bit
       adde	r8,r8,r8			// shift tmp.lsw to left 1 bit
       adde	r7,r7,r7			// shift tmp.msw to left 1 bit
       subfc	r0,r6,r8		// tmp.lsw - dvs.lsw
       subfe.	r9,r5,r7		// tmp.msw - dvs.msw
       blt	cr0,lab8			// if(result < 0) clear carry bit
       mr	r8,r0				// move lsw
       mr	r7,r9				// move msw
       addic	r0,r10,1		// set carry bit
 lab8:
       bdnz	lab7

       // write remainder
       mr	r4,r8				// rem.lsw
       mr	r3,r7				// rem.msw

 lab9:
       // remainder already in r3:r4 (quotient is 0 (dvs > dvd))
       // now adjust result based on sign of original DVD
       // we negate result if DVD was negative.
       bge	cr7,no_adjust
       subfic   r4,r4,0
       subfze   r3,r3
 no_adjust:
       blr						// return
}


// __shl2i 	- 64-bit shift left for 32-bit PowerPC
//
//	Input:
//		[unsigned/signed] long long value in R3:R4
//		shift count in R5
//		
//
//	Output:
//		shifted value in R3:R4
//		
//
asm void __shl2i(void)
{
	subfic	r8,r5,32
	subic	r9,r5,32
	slw		r3,r3,r5
	srw		r10,r4,r8
	or		r3,r3,r10
	slw		r10,r4,r9
	or		r3,r3,r10			// high word
	slw		r4,r4,r5			// low word
	blr
}


// __shr2u 	- 64-bit logical shift right for 32-bit PowerPC
//
//	Input:
//		unsigned long long value in R3:R4
//		shift count in R5
//		
//
//	Output:
//		shifted value in R3:R4
//		
//
asm void __shr2u(void)
{
	subfic	r8,r5,32
	subic	r9,r5,32
	srw		r4,r4,r5
	slw		r10,r3,r8
	or		r4,r4,r10
	srw		r10,r3,r9
	or		r4,r4,r10			// low word
	srw		r3,r3,r5			// high word
	blr
}


// __shr2i 	- 64-bit arithmetic shift right for 32-bit PowerPC
//
//	Input:
//		signed long long value in R3:R4
//		shift count in R5
//		
//
//	Output:
//		shifted value in R3:R4
//		
//
asm void __shr2i(void)
{
	subfic	r8,r5,32
	subic.	r9,r5,32
	srw		r4,r4,r5
	slw		r10,r3,r8
	or		r4,r4,r10
	sraw	r10,r3,r9
	ble		around
	or		r4,r4,r10			// low word
around:
	sraw	r3,r3,r5			// high word
	blr	
}


// __cvt_sll_dbl 	- convert signed long long to double
//
//	Input:
//		signed long long value in R3:R4
//
//	Output:
//		value converted to double in F1
//		
//	Algorithm courtesy of Lee Killough (killough@convex.com)
//	Coded in PowerPC assembly by Jason Eckhardt.
//
asm void __cvt_sll_dbl(void)
{
	// r3 = nh (input), r4 = nl (input)
	// r5 = sign, r6 = exp, r7-r10 = scratch (all local temps)

  	// remember sign bit and negate r3:r4 if it is negative
  	rlwinm.	r5,r3,0,0,0
	beq		cr0,positive
	subfic	r4,r4,0
	subfze	r3,r3
positive:
	// exit early if r3:r4 is 0:0
	or.		r7,r3,r4
	li		r6,0				// exp = 0
	beq		cr0,zero
	// count the leading zeroes in r3:r4
	cntlzw	r7,r3
	cntlzw	r8,r4
	rlwinm	r9,r7,26,0,4		// (i.e. slwi r9,r7,26)
	srawi	r9,r9,31
	and		r9,r9,r8
	add		r7,r7,r9			// r7 contains number of leading zeroes
	// shift r3:r4 left by r7 bits
	subfic	r8,r7,32
	subic	r9,r7,32
	slw		r3,r3,r7
	srw		r10,r4,r8
	or		r3,r3,r10
	slw		r10,r4,r9
	or		r3,r3,r10			// high word
	slw		r4,r4,r7			// low word
	// subtract the number of leading 0's from exponent
	sub		r6,r6,r7
#if PPC_RT_ROUND_NEAREST
	rlwinm	r7,r4,0,21,31
	cmpi	cr0,r7,0x400
	addi	r6,r6,1086			// adjust exponent (scheduled to here)
	blt		noround				// if ((nl & 0x7ff) < 0x400) don't round
	bgt		round				// if ((nl & 0x7ff) > 0x400) round
	rlwinm.	r7,r4,0,20,20		// if ((nl & 0x7ff) == 0x400) && (nl & 0x800) round
								// last case is if exactly half way round to even
	beq		noround
round:
	addic	r4,r4,0x0800
	addze	r3,r3
	addze	r6,r6				// adjust exponent (if carry add one to exponent)
noround:
#else
	// don't round on convertions, just truncate BC 26-Mar-99
	addi	r6,r6,1086			// adjust exponent
#endif
	// shift r3:r4 right by 11 bits, but need only lower word
	rlwinm	r4,r4,21,0,31		// (i.e. srwi	r4,r4,11)
	rlwimi	r4,r3,21,0,10		// (i.e. insrwi	r4,r3,11,0)
	// construct high word of double now
	rlwinm	r3,r3,21,12,31		// (nh >> 11) & 0xfffff
	rlwinm	r6,r6,20,0,11		// (exp << 20)
	or		r3,r6,r3
	or		r3,r5,r3			// finally, set the sign bit
zero:
	// r3:r4 now makes up a floating point double
	// we'll transfer to a fpr through memory using input param area
	// as temporary storage (8 words always allocated at sp+24).
	stw		r3,24(r1)
	stw		r4,28(r1)
	lfd		f1,24(r1)
	blr 
}


// __cvt_ull_dbl 	- convert unsigned long long to double
//
//	Input:
//		unsigned long long value in R3:R4
//
//	Output:
//		value converted to double in F1
//		
//	Algorithm courtesy of Lee Killough (killough@convex.com)
//	Coded in PowerPC assembly by Jason Eckhardt.
//
asm void __cvt_ull_dbl(void)
{
	// r3 = nh (input), r4 = nl (input)
	// r5 = scratch, r6 = exp, r7-r10 = scratch (all local temps)

	// exit early if r3:r4 is 0:0
	or.		r7,r3,r4
	li		r6,0				// exp = 0
	beq		cr0,zero
	// count the leading zeroes in r3:r4
	cntlzw	r7,r3
	cntlzw	r8,r4
	rlwinm	r9,r7,26,0,4		// (i.e. slwi r9,r7,26)
	srawi	r9,r9,31
	and		r9,r9,r8
	add		r7,r7,r9			// r7 contains number of leading zeroes
	// shift r3:r4 left by r7 bits
	subfic	r8,r7,32
	subic	r9,r7,32
	slw		r3,r3,r7
	srw		r10,r4,r8
	or		r3,r3,r10
	slw		r10,r4,r9
	or		r3,r3,r10			// high word
	slw		r4,r4,r7			// low word
	// subtract the number of leading 0's from exponent
	sub		r6,r6,r7
#if PPC_RT_ROUND_NEAREST
	rlwinm	r7,r4,0,21,31
	cmpi	cr0,r7,0x400
	addi	r6,r6,1086			// adjust exponent (scheduled to here)
	blt		noround				// if ((nl & 0x7ff) < 0x400) don't round
	bgt		round				// if ((nl & 0x7ff) > 0x400) round
	rlwinm.	r7,r4,0,20,20		// if ((nl & 0x7ff) == 0x400) && (nl & 0x800) round
								// last case is if exactly half way round to even
	beq		noround
round:
	addic	r4,r4,0x0800
	addze	r3,r3
	addze	r6,r6				// adjust exponent (if carry add one to exponent)
noround:
#else
	addi	r6,r6,1086			// adjust exponent
#endif
	// shift r3:r4 right by 11 bits, but need only lower word
	rlwinm	r4,r4,21,0,31		// (i.e. srwi	r4,r4,11)
	rlwimi	r4,r3,21,0,10		// (i.e. insrwi	r4,r3,11,0)
	// construct high word of double now (sign bit always zero for ull)
	rlwinm	r3,r3,21,12,31		// (nh >> 11) & 0xfffff
	rlwinm	r6,r6,20,0,11		// (exp << 20)
	or		r3,r6,r3
zero:
	// r3:r4 now makes up a floating point double
	// we'll transfer to a fpr through memory using input param area
	// as temporary storage (8 words always allocated at sp+24).
	stw		r3,24(r1)
	stw		r4,28(r1)
	lfd		f1,24(r1)
	blr 
}


// __cvt_sll_flt 	- convert signed long long to flt
//
//	Input:
//		signed long long value in R3:R4
//
//	Output:
//		value converted to float in F1
//		
//	Algorithm courtesy of Lee Killough (killough@convex.com)
//	Coded in PowerPC assembly by Jason Eckhardt.
//
asm void __cvt_sll_flt(void)
{
	// r3 = nh (input), r4 = nl (input)
	// r5 = sign, r6 = exp, r7-r10 = scratch (all local temps)

  	// remember sign bit and negate r3:r4 if it is negative
  	rlwinm.	r5,r3,0,0,0
	beq		cr0,positive
	subfic	r4,r4,0
	subfze	r3,r3
positive:
	// exit early if r3:r4 is 0:0
	or.		r7,r3,r4
	li		r6,0				// exp = 0
	beq		cr0,zero
	// count the leading zeroes in r3:r4
	cntlzw	r7,r3
	cntlzw	r8,r4
	rlwinm	r9,r7,26,0,4		// (i.e. slwi r9,r7,26)
	srawi	r9,r9,31
	and		r9,r9,r8
	add		r7,r7,r9			// r7 contains number of leading zeroes
	// shift r3:r4 left by r7 bits
	subfic	r8,r7,32
	subic	r9,r7,32
	slw		r3,r3,r7
	srw		r10,r4,r8
	or		r3,r3,r10
	slw		r10,r4,r9
	or		r3,r3,r10			// high word
	slw		r4,r4,r7			// low word
	// subtract the number of leading 0's from exponent
	sub		r6,r6,r7
#if PPC_RT_ROUND_NEAREST
	rlwinm	r7,r4,0,21,31
	cmpi	cr0,r7,0x400
	addi	r6,r6,1086			// adjust exponent (scheduled to here)
	blt		noround				// if ((nl & 0x7ff) < 0x400) don't round
	bgt		round				// if ((nl & 0x7ff) > 0x400) round
	rlwinm.	r7,r4,0,20,20		// if ((nl & 0x7ff) == 0x400) && (nl & 0x800) round
								// last case is if exactly half way round to even
	beq		noround
round:
	addic	r4,r4,0x0800
	addze	r3,r3
	addze	r6,r6				// adjust exponent (if carry add one to exponent)
noround:
#else
	addi	r6,r6,1086			// adjust exponent
#endif
	// shift r3:r4 right by 11 bits, but need only lower word
	rlwinm	r4,r4,21,0,31		// (i.e. srwi	r4,r4,11)
	rlwimi	r4,r3,21,0,10		// (i.e. insrwi	r4,r3,11,0)
	// construct high word of double now
	rlwinm	r3,r3,21,12,31		// (nh >> 11) & 0xfffff
	rlwinm	r6,r6,20,0,11		// (exp << 20)
	or		r3,r6,r3
	or		r3,r5,r3			// finally, set the sign bit
zero:
	// r3:r4 now makes up a floating point double
	// we'll transfer to a fpr through memory using input param area
	// as temporary storage (8 words always allocated at sp+24).
	stw		r3,24(r1)
	stw		r4,28(r1)
	lfd		f1,24(r1)
	frsp	f1,f1
	blr 
}


// __cvt_ull_flt 	- convert unsigned long long to float
//
//	Input:
//		unsigned long long value in R3:R4
//
//	Output:
//		value converted to float in F1
//		
//	Algorithm courtesy of Lee Killough (killough@convex.com)
//	Coded in PowerPC assembly by Jason Eckhardt.
//
asm void __cvt_ull_flt(void)
{
	// r3 = nh (input), r4 = nl (input)
	// r5 = scratch, r6 = exp, r7-r10 = scratch (all local temps)

	// exit early if r3:r4 is 0:0
	or.		r7,r3,r4
	li		r6,0				// exp = 0
	beq		cr0,zero
	// count the leading zeroes in r3:r4
	cntlzw	r7,r3
	cntlzw	r8,r4
	rlwinm	r9,r7,26,0,4		// (i.e. slwi r9,r7,26)
	srawi	r9,r9,31
	and		r9,r9,r8
	add		r7,r7,r9			// r7 contains number of leading zeroes
	// shift r3:r4 left by r7 bits
	subfic	r8,r7,32
	subic	r9,r7,32
	slw		r3,r3,r7
	srw		r10,r4,r8
	or		r3,r3,r10
	slw		r10,r4,r9
	or		r3,r3,r10			// high word
	slw		r4,r4,r7			// low word
	// subtract the number of leading 0's from exponent
	sub		r6,r6,r7
#if PPC_RT_ROUND_NEAREST
	rlwinm	r7,r4,0,21,31
	cmpi	cr0,r7,0x400
	addi	r6,r6,1086			// adjust exponent (scheduled to here)
	blt		noround				// if ((nl & 0x7ff) < 0x400) don't round
	bgt		round				// if ((nl & 0x7ff) > 0x400) round
	rlwinm.	r7,r4,0,20,20		// if ((nl & 0x7ff) == 0x400) && (nl & 0x800) round
								// last case is if exactly half way round to even
	beq		noround
round:
	addic	r4,r4,0x0800
	addze	r3,r3
	addze	r6,r6				// adjust exponent (if carry add one to exponent)
noround:
#else
	addi	r6,r6,1086			// adjust exponent
#endif
	// shift r3:r4 right by 11 bits, but need only lower word
	rlwinm	r4,r4,21,0,31		// (i.e. srwi	r4,r4,11)
	rlwimi	r4,r3,21,0,10		// (i.e. insrwi	r4,r3,11,0)
	// construct high word of double now (sign bit always zero for ull)
	rlwinm	r3,r3,21,12,31		// (nh >> 11) & 0xfffff
	rlwinm	r6,r6,20,0,11		// (exp << 20)
	or		r3,r6,r3
zero:
	// r3:r4 now makes up a floating point double
	// we'll transfer to a fpr through memory using input param area
	// as temporary storage (8 words always allocated at sp+24).
	stw		r3,24(r1)
	stw		r4,28(r1)
	lfd		f1,24(r1)
	frsp	f1,f1
	blr 
}


// __cvt_dbl_usll 	- convert double to [unsigned/signed] long long
//
//	Input:
//		double value to convert in F1
//
//	Output:
//		[unsigned/signed] long long value in R3:R4
//
//	This works unchanged for float to long long as well since the 
//	incoming parameter is always a double.
//
//	Algorithm courtesy of Lee Killough (killough@convex.com)
//	Coded in PowerPC assembly by Jason Eckhardt.
//
asm void __cvt_dbl_usll(void)
{
	// r3 = high word, r4 = low word
	// r5 = exp, r6 = sign , r7-r10 = scratch (local temps)
	
	// we first transfer the double from an fpr to a general register pair
	// through memory. we'll use the input param area as temporary storage
	// (8 words always allocated at sp+24).
	stfd	f1,24(r1)
	lwz		r3,24(r1)
	lwz		r4,28(r1)
	// extract the exponent from the floating point value
	rlwinm   r5,r3,12,21,31
	// round all fractions to zero and return early
	cmpli	cr0,0,r5,1023
	bge		cr0,not_fraction	
	li		r3,0
	li		r4,0
	blr							// return to caller
not_fraction:
	// remember the sign
	mr		r6,r3
	// mantissa has implied 1 bit [nh = (nh & 0xfffff) | 0x100000] 
	rlwinm	r3,r3,0,12,31
	oris	r3,r3,0x0010
	// when exp-1075 < 0 we need to shift right, else shift left
	addi	r5,r5,-1075
	cmpwi	cr0,r5,0
	bge		cr0,left
	// shift r3:r4 right by -exp bits
	neg		r5,r5
	subfic	r8,r5,32
	subic	r9,r5,32
	srw		r4,r4,r5
	slw		r10,r3,r8
	or		r4,r4,r10
	srw		r10,r3,r9
	or		r4,r4,r10			// low word
	srw		r3,r3,r5			// high word
	b		around
left:
	// shift r3:r4 left by 'exp' bits
	// if exp >= 10, then the result is undefined. force this to the maximum
	// long long value or maximum negative long long
	cmpwi	cr0,r5,10
	ble+	no_overflow
	rlwinm.	r6,r6,0,0,0
	beq		cr0,max_positive
	// max negative value is 0x80000000
	lis		r3,0x8000
	li		r4,0
	blr
max_positive:
	// max positive is 0x7fffffff
	lis		r3,0x7fff
	ori		r3,r3,0xffff
	li		r4,-1
	blr
no_overflow:
	subfic	r8,r5,32
	subic	r9,r5,32
	slw		r3,r3,r5
	srw		r10,r4,r8
	or		r3,r3,r10
	slw		r10,r4,r9
	or		r3,r3,r10			// high word
	slw		r4,r4,r5			// low word
around:
	// if the sign bit was set, negate the long long
	rlwinm.	r6,r6,0,0,0
	beq		cr0,positive
	subfic	r4,r4,0
	subfze	r3,r3
positive:
	blr
}



    
