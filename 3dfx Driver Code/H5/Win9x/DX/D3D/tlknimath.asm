TITLE   tlknimath.asm
.686
.XMM
ASSUME cs:FLAT,ds:FLAT,es:FLAT,fs:FLAT,gs:FLAT
.MODEL FLAT


;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
COMMENT~

This code implements the power function using Intel SSE instructions.  It 
calculates 4 power at once in about 60 clocks, all with the same exponent.
The hope is that the exponent won't change very often.  

The only externally linkable function is UpdateKniPowerFunctionPtr, which 
returns a pointer to another function type KniPowerFunctionType.
UpdateKniPowerFunctionPtr() must be called before calling the power function.  
It takes the exponent as a variable and figures out which power function needs 
to be executed and which interpolation values (if any) will be needed. 

The way this works internally is that there are 101 separate functions that 
calculate power from x^0 to x^100, all using multiplies.  If the exponent is 
a positive integer <= 100, one of these functions is executed directly.  Otherwise 
we interpolate between two values.  Furthermore, if the exponent is under 25 
we'll interpolate between 1/4 integers and if the exponent is under 6.25 we 
interpolate between 1/16th integers.  If the exponent is negative the reciprocal 
of the result is returned.  


Here's the pseudo code to call these functions:

////// Global Defines
typedef void KniPowerFunctionType();
extern "C" KniPowerFunctionType* UpdateKniPowerFunctionPtr(float exponent);
KniPowerFunctionType* KniPowerFunction;
void KniExp();


	//////////// In UpdateRenderingContext()
	// update the pointer whenever it changes
	if (newExponent)
		KniPowerFunction = UpdateKniPowerFunctionPtr(exponent);


	//////////// In the lighting code :
	// load base in xmmBase (xmm0), result is returned in xmmBase
	// xmmTmp1 and xmmTmp2 are modified (xmm1 & xmm2)
	float *base, *result;
	_asm movups  xmm0, oword ptr [base + offset]	; load base values
	KniPowerFunction();
	_asm movups  oword ptr [result + offset], xmm0	; get results


~ ;END COMMENT
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

;******************************************************************************
; MACROS
;******************************************************************************
xmmBase		TEXTEQU		<xmm0>		; base and result register
xmmTmp1		TEXTEQU		<xmm1>		; Temporary registers which
xmmTmp2		TEXTEQU		<xmm2>		; will be overwritten
xmmTmp3		TEXTEQU		<xmm3>
xmmTmp4		TEXTEQU		<xmm4>
xmmTmp5		TEXTEQU		<xmm5>
xmmTmp6		TEXTEQU		<xmm6>
xmmTmp7		TEXTEQU		<xmm7>

; xmm compare immediate values
XMM_EQ		TEXTEQU <0>	; ==
XMM_LT		TEXTEQU <1>	; <
XMM_LE		TEXTEQU <2>	; <=
XMM_Q		TEXTEQU <3>	; ?
XMM_NE		TEXTEQU <4>	; !=
XMM_GE		TEXTEQU <5>	; >=
XMM_GT		TEXTEQU <6>	; >
XMM_NQ		TEXTEQU <7>	; !?

USE_SQRT_INTP_HACK	EQU 1			; "Kentucky windage"

;******************************************************************************
; CONSTANTS - segment and definitions
;******************************************************************************
.CONST
Align 4
flt4		dd		4.0

Align 16
ones		dd      4 DUP(1.0)
halves		dd		4 DUP(0.5)
MAXLOG		dd		4 DUP(87.0)			;; over exp(87) blows up (NaN)
MagMask		dd		4 DUP(07fffffffh)
C1PC2		dd		4 DUP(0bf317218h)	;;-0.693147182
LOG2EF		dd		4 DUP(03fb8aa3bh)	;; 1.442695022
RCP_1FACT	equ		ones				;; 1.000000000 = 1/1!
RCP_2FACT	equ		halves				;; 0.500000000 = 1/2!
RCP_3FACT	dd		4 DUP(03e2aaaabh)	;; 0.166666667 = 1/3!
RCP_4FACT	dd		4 DUP(03d2aaaabh)	;; 0.041666667 = 1/4!
RCP_5FACT	dd		4 DUP(03c088889h)	;; 0.008333333 = 1/5!
RCP_6FACT	dd		4 DUP(03ab60b61h)	;; 0.001398888 = 1/6!
RCP_7FACT	dd		4 DUP(039500d01h)	;; 0.000198413 = 1/7!

Align 16
Sign_PNPN	dd		00000000h, 80000000h, 00000000h, 80000000h
Sign_NPNP	dd		80000000h, 00000000h, 80000000h, 00000000h


;******************************************************************************
; DATA - segment and definitions
;******************************************************************************
.DATA
Align 16
	fIntpFactorLow					dd	4 DUP(?)		; lower interpolation factor
	fIntpFactorHi					dd	4 DUP(?)		; upper interpolation factor
	lpKniPowerFunctionIntpLow		dd	?				; interpolation lower power function
	lpKniPowerFunctionNegative		dd	?				; interpolation power function called by negative powers

	dwMxcsrRoundChop				dd	0000ff80h		; SSE CSR register, set rounding to chop, FTZ=1, exceptions masked
	dwMxcsrTmp						dd	?				; SSE CSR register, for restoring org value

	lpOSPowFunction					dd	?				; this is just for testing the power function

Align 16
	tmp128							dd	4 DUP(?)		; temporary aligned structure

;******************************************************************************
; CODE 
;******************************************************************************
_TEXT   SEGMENT 
.CODE

Align 4
LookupTable_KniPowFunction:
	dd	KniPowFunction0,	KniPowFunction1,	KniPowFunction2,	KniPowFunction3,	KniPowFunction4
	dd	KniPowFunction5,	KniPowFunction6,	KniPowFunction7,	KniPowFunction8,	KniPowFunction9
	dd	KniPowFunction10,	KniPowFunction11,	KniPowFunction12,	KniPowFunction13,	KniPowFunction14
	dd	KniPowFunction15,	KniPowFunction16,	KniPowFunction17,	KniPowFunction18,	KniPowFunction19
	dd	KniPowFunction20,	KniPowFunction21,	KniPowFunction22,	KniPowFunction23,	KniPowFunction24
	dd	KniPowFunction25,	KniPowFunction26,	KniPowFunction27,	KniPowFunction28,	KniPowFunction29
	dd	KniPowFunction30,	KniPowFunction31,	KniPowFunction32,	KniPowFunction33,	KniPowFunction34
	dd	KniPowFunction35,	KniPowFunction36,	KniPowFunction37,	KniPowFunction38,	KniPowFunction39
	dd	KniPowFunction40,	KniPowFunction41,	KniPowFunction42,	KniPowFunction43,	KniPowFunction44
	dd	KniPowFunction45,	KniPowFunction46,	KniPowFunction47,	KniPowFunction48,	KniPowFunction49
	dd	KniPowFunction50,	KniPowFunction51,	KniPowFunction52,	KniPowFunction53,	KniPowFunction54
	dd	KniPowFunction55,	KniPowFunction56,	KniPowFunction57,	KniPowFunction58,	KniPowFunction59
	dd	KniPowFunction60,	KniPowFunction61,	KniPowFunction62,	KniPowFunction63,	KniPowFunction64
	dd	KniPowFunction65,	KniPowFunction66,	KniPowFunction67,	KniPowFunction68,	KniPowFunction69
	dd	KniPowFunction70,	KniPowFunction71,	KniPowFunction72,	KniPowFunction73,	KniPowFunction74
	dd	KniPowFunction75,	KniPowFunction76,	KniPowFunction77,	KniPowFunction78,	KniPowFunction79
	dd	KniPowFunction80,	KniPowFunction81,	KniPowFunction82,	KniPowFunction83,	KniPowFunction84
	dd	KniPowFunction85,	KniPowFunction86,	KniPowFunction87,	KniPowFunction88,	KniPowFunction89
	dd	KniPowFunction90,	KniPowFunction91,	KniPowFunction92,	KniPowFunction93,	KniPowFunction94
	dd	KniPowFunction95,	KniPowFunction96,	KniPowFunction97,	KniPowFunction98,	KniPowFunction99
	dd	KniPowFunction100



;--------------- START OF CODE FOR TESTING THIS FUNCTION ---------------
ifdef TEST_KNI_POW_FUNCTION

;******************************************************************************
; Since I don't want to load/unload the kni registers in the C code we'll
; through an extra call overhead and I'll do this in assembly
;
; Routine:  void Asm_KniPowFunction(float *);
;
;******************************************************************************
PUBLIC _Asm_KniPowFunction
_Asm_KniPowFunction	PROC
	mov		eax, dword ptr [esp+4]
	movups	xmmBase, oword ptr [eax]
	call	dword ptr [lpOSPowFunction]
	mov		eax, dword ptr [esp+4]
	movups	oword ptr [eax], xmmBase
	ret
_Asm_KniPowFunction	ENDP

;******************************************************************************
; Routine:  void Asm_UpdateKniPowFunctionPtr(void* lpFnctPtr);
;
;******************************************************************************
PUBLIC _Asm_UpdateKniPowFunctionPtr
_Asm_UpdateKniPowFunctionPtr	PROC
	mov		eax, dword ptr [esp+4]
	mov		dword ptr [lpOSPowFunction], eax
	ret
_Asm_UpdateKniPowFunctionPtr	ENDP

endif ;TEST_KNI_POW_FUNCTION
;--------------- END OF CODE FOR TESTING THIS FUNCTION ---------------



;*******************************************************************************************
; Routine:  void* UpdateKniPowerFunctionPtr(float power)
;
; Comment:  Called when power changes, updates pointers in the interpolate 
;			function (if needed) and returns a pointer to the correct 
;			KniPowFunction
;*******************************************************************************************
INTERPOLATE_OVER_25 = 0

ALIGN   16
PUBLIC _UpdateKniPowerFunctionPtr
_UpdateKniPowerFunctionPtr	PROC
numLocalVars	= 0
retAddr			= numLocalVars*4 + 1*4 + 0		; 1*4 for the push's
stackParams		= numLocalVars*4 + 1*4 + 4
power			= stackParams + 4*0
locals			= 0

	push		edx

	movss		xmmTmp1, dword ptr [esp+power]
	cvttss2si	edx, xmmTmp1					; float to int (truncate)
	cvtsi2ss	xmmTmp2, edx					; back to float

	; force to a positive value
	test		edx, 1 SHL 31
	je			@F
	neg			edx
@@:

	; anything over 100 we clamp to 100
	cmp			edx, 100
	jg			MaxFunction

Boundary1_1:					; look for integers
	comiss		xmmTmp1, xmmTmp2
	jne			Boundary1_1_NonInteger
	mov			eax, dword ptr [LookupTable_KniPowFunction + edx*4]
	jmp			CheckForNegative

Boundary1_1_NonInteger:			; not an even integer
	cmp			edx, 25
	jl			Boundary1_4

Boundary1_1_Interpolate:		; interpolate between integers
if INTERPOLATE_OVER_25
	mov			eax, offset KniPowFunctionInterpolateLarge
	jmp			GenerateInterpolationFactors
else	; hack!!! (but it's good enough)
	mov			eax, dword ptr [LookupTable_KniPowFunction + edx*4]
	jmp			CheckForNegative
endif

Boundary1_4:					; do this in int/4 increments
	mulss		xmmTmp1, dword ptr [flt4]
	cvttss2si	edx, xmmTmp1					; float to int (truncate)
	cvtsi2ss	xmmTmp2, edx					; back to float

	; force to a positive value
	test		edx, 1 SHL 31
	je			@F
	neg			edx
@@:

	; check if power is an integer
	comiss		xmmTmp1, xmmTmp2
	jne			Boundary1_4_NonInteger
	mov			edx, dword ptr [LookupTable_KniPowFunction + edx*4]
	mov			dword ptr [lpKniPowerFunctionIntpLow], edx
	mov			eax, offset KniPowFunctionPowerMedium
	jmp			CheckForNegative

Boundary1_4_NonInteger:			; not an even integer/4
	cmp			edx, 25
	jl			Boundary1_16

Boundary1_4_Interpolate:		; interpolate between integers/4
	mov			eax, offset KniPowFunctionInterpolateMedium
	jmp			GenerateInterpolationFactors


Boundary1_16:					; do this in int/16 increments
	mulss		xmmTmp1, dword ptr [flt4]
	cvttss2si	edx, xmmTmp1					; float to int (truncate)
	cvtsi2ss	xmmTmp2, edx					; back to float

	; force to a positive value
	test		edx, 1 SHL 31
	je			@F
	neg			edx
@@:

	; check if power is an integer
	comiss		xmmTmp1, xmmTmp2
	jne			Boundary1_16_NonInteger
	mov			edx, dword ptr [LookupTable_KniPowFunction + edx*4]
	mov			dword ptr [lpKniPowerFunctionIntpLow], edx
	mov			eax, offset KniPowFunctionPowerSmall
	jmp			CheckForNegative

Boundary1_16_NonInteger:			; not an even integer/16
	; check if the number is < 1/16th, is so we must call a special function
	cmp			edx, 0
	je			Under_1_16
	mov			eax, offset KniPowFunctionInterpolateSmall
	jmp			GenerateInterpolationFactors

Under_1_16:
	; generate interpolation factors    hi = power - (int)power,  low = 1.0 - hi
	subss		xmmTmp1, xmmTmp2
IF USE_SQRT_INTP_HACK
	rsqrtss		xmmTmp1, xmmTmp1
	rcpss		xmmTmp1, xmmTmp1
ENDIF
	movss		xmmTmp2, dword ptr [ones]
	subss		xmmTmp2, xmmTmp1
	shufps		xmmTmp1, xmmTmp1, 0				; replicate across
	shufps		xmmTmp2, xmmTmp2, 0				; all 4 values
	movaps		oword ptr [fIntpFactorHi], xmmTmp1
	movaps		oword ptr [fIntpFactorLow], xmmTmp2
	mov			eax, offset KniPowFunctionInterpolate_1_16
	jmp			Done


MaxFunction:	; clamp to 100
	mov			eax, offset KniPowFunction100
	jmp			CheckForNegative


GenerateInterpolationFactors:
	; generate interpolation factors    hi = power - (int)power,  low = 1.0 - hi
	subss		xmmTmp1, xmmTmp2
IF USE_SQRT_INTP_HACK
	rsqrtss		xmmTmp1, xmmTmp1
	rcpss		xmmTmp1, xmmTmp1
ENDIF
	movss		xmmTmp2, dword ptr [ones]
	subss		xmmTmp2, xmmTmp1
	shufps		xmmTmp1, xmmTmp1, 0				; replicate across
	shufps		xmmTmp2, xmmTmp2, 0				; all 4 values
	mov			edx, dword ptr [LookupTable_KniPowFunction + edx*4]
	mov			dword ptr [lpKniPowerFunctionIntpLow], edx
	movaps		oword ptr [fIntpFactorHi], xmmTmp1
	movaps		oword ptr [fIntpFactorLow], xmmTmp2
	jmp			CheckForNegative


CheckForNegative:
	test		dword ptr [esp+power], 1 SHL 31
	je			Done

	mov			dword ptr [lpKniPowerFunctionNegative], eax
	mov			eax, offset KniPowFunctionNegative

Done:
	pop		edx
	ret

_UpdateKniPowerFunctionPtr	ENDP

;===========================================================================================
;===========================================================================================
; PRIVATE FUNCTIONS
;===========================================================================================
;===========================================================================================

;*******************************************************************************************
; Routine:  void KniPowFunctionPowerMedium ()
;
; Comment:  Handles cases of small exponents on even integers of 1/4th.  For instance 1 1/4
;			we do x^5 and then do x^-4 by doing do 2 square-roots
;*******************************************************************************************
ALIGN   16
KniPowFunctionPowerMedium	PROC	PRIVATE
	mov		eax, dword ptr [lpKniPowerFunctionIntpLow]
	call	eax
	rsqrtps		xmmBase, xmmBase				; 1/x^-2
	rsqrtps		xmmBase, xmmBase				; 1/x^-4
	ret
KniPowFunctionPowerMedium	ENDP

;*******************************************************************************************
; Routine:  void KniPowFunctionPowerSmall ()
;
; Comment:  Handles cases of small exponents on even integers of 1/16th.  For instance 1 1/16
;			we do x^17 and then do x^-16 by doing do 4 square-roots
;*******************************************************************************************
ALIGN   16
KniPowFunctionPowerSmall	PROC	PRIVATE
	mov		eax, dword ptr [lpKniPowerFunctionIntpLow]
	call	eax
	rsqrtps		xmmBase, xmmBase				; 1/x^-2
	rsqrtps		xmmBase, xmmBase				; 1/x^-4
	rsqrtps		xmmBase, xmmBase				; 1/x^-8
	rsqrtps		xmmBase, xmmBase				; 1/x^-16
	ret
KniPowFunctionPowerSmall	ENDP

;*******************************************************************************************
; Routine:  void KniPowFunctionInterpolateLarge ()
;
; Comment:  Interpolates power between (int) power and (int) power+1 when power is positive
;			Power is already known, base is in xmmBase, and is returned in xmmBase
;			x1 = pow(x,(int)y)		f2 = y - (int)y
;			x2 = x1 * x2			f1 = 1.0 - f1
;			x = (x1 * f1) + (x2 * f2)
;			This only works for power >= +1.0
;*******************************************************************************************
if INTERPOLATE_OVER_25
ALIGN   16
KniPowFunctionInterpolateLarge	PROC	PRIVATE
	mov		eax, dword ptr [lpKniPowerFunctionIntpLow]
	movaps	xmmTmp2, oword ptr [fIntpFactorHi]
	mulps	xmmTmp2, xmmBase
	call	eax
	mulps	xmmTmp2, xmmBase
	mulps	xmmBase, oword ptr [fIntpFactorLow]
	addps	xmmBase, xmmTmp2
	ret
KniPowFunctionInterpolateLarge	ENDP
endif

;*******************************************************************************************
; Routine:  void KniPowFunctionInterpolateMedium ()
;
; Comment:  Similar to KniPowFunctionInterpolateLarge() except the values are interpolated
;			on 1/4th integers.  
;*******************************************************************************************
ALIGN   16
KniPowFunctionInterpolateMedium	PROC	PRIVATE
	mov		eax, dword ptr [lpKniPowerFunctionIntpLow]
	movaps	xmmTmp2, oword ptr [fIntpFactorHi]
	mulps	xmmTmp2, xmmBase
	call	eax
	mulps	xmmTmp2, xmmBase
	mulps	xmmBase, oword ptr [fIntpFactorLow]
	addps	xmmBase, xmmTmp2
	rsqrtps		xmmBase, xmmBase				; 1/x^-2
	rsqrtps		xmmBase, xmmBase				; 1/x^-4
	ret
KniPowFunctionInterpolateMedium	ENDP

;*******************************************************************************************
; Routine:  void KniPowFunctionInterpolateSmall ()
;
; Comment:  Similar to KniPowFunctionInterpolateLarge() except the values are interpolated
;			on 1/16th integers.  So for example the number 1.001 we interpolate between 
;			1 1/2 and 1 7/16.  What we really do in this case we interpolate between x^24 and 
;			x^25, and then take x^-16 by doing do 4 square-roots.
;*******************************************************************************************
ALIGN   16
KniPowFunctionInterpolateSmall	PROC	PRIVATE
	mov		eax, dword ptr [lpKniPowerFunctionIntpLow]
	movaps	xmmTmp2, oword ptr [fIntpFactorHi]
	mulps	xmmTmp2, xmmBase
	call	eax
	mulps	xmmTmp2, xmmBase
	mulps	xmmBase, oword ptr [fIntpFactorLow]
	addps	xmmBase, xmmTmp2
	rsqrtps		xmmBase, xmmBase				; 1/x^-2
	rsqrtps		xmmBase, xmmBase				; 1/x^-4
	rsqrtps		xmmBase, xmmBase				; 1/x^-8
	rsqrtps		xmmBase, xmmBase				; 1/x^-16
	ret
KniPowFunctionInterpolateSmall	ENDP

;*******************************************************************************************
; Routine:  void KniPowFunctionInterpolate_1_16 ()
;
; Comment:  Special function to interpolate between 0 and 1/16.  Required because x^0 = 1 and
;			x^1 = x.  The problem is the other interpolations call x^y for the lower value and
;			then use this value *x for the upper value.  
;*******************************************************************************************
ALIGN   16
KniPowFunctionInterpolate_1_16	PROC	PRIVATE
	; build mask for any zero
	xorps	xmmTmp1, xmmTmp1					;	0	0	0	0
	cmpps	xmmTmp1, xmmBase, XMM_NE			;  (x != 0) ? -1 : 0  (all 4)

	mulps	xmmBase, oword ptr [fIntpFactorHi]	; x^1 = x so x^1 * hi = x * hi
	addps	xmmBase, oword ptr [fIntpFactorLow]	; x^0 = 1 so x^0 * low = low
	rsqrtps		xmmBase, xmmBase				; 1/x^-2
	rsqrtps		xmmBase, xmmBase				; 1/x^-4
	rsqrtps		xmmBase, xmmBase				; 1/x^-8
	rsqrtps		xmmBase, xmmBase				; 1/x^-16

	; mask any zero base values back to zero
	andps	xmmBase, xmmTmp1
	ret
KniPowFunctionInterpolate_1_16	ENDP



;*******************************************************************************************
; Routine:  void KniPowFunctionNegative ()
;
; Comment:  Handles cases of x^-y 
;			Since x^-y = 1/x^+y we just do a reciprocal when finished
;*******************************************************************************************
ALIGN   16
KniPowFunctionNegative	PROC	PRIVATE
	mov		eax, dword ptr [lpKniPowerFunctionNegative]
	call	eax
	rcpps	xmmBase, xmmBase
	ret
KniPowFunctionNegative	ENDP





;*******************************************************************************************
;*******************************************************************************************
;*******************************************************************************************
; Routines:	void KniPowFunctionN ()
;
; Comments:	Takes register xmmBase to the "N" power, and returns the value in xmmBase
;			Use "ALIGN 8" rather than 16 because these are all 3-byte instructions
;			so the decoder will, worst case, decode two full instructions on the 
;			first clock, which is plenty for SSE instructions.  16-byte alignment
;			is unnecessary.
;*******************************************************************************************
ALIGN   8
KniPowFunction0	PROC	PRIVATE
	movaps		xmmBase, oword ptr [ones]		; anything to the zero power is one
	ret
KniPowFunction0	ENDP

ALIGN   8
KniPowFunction1	PROC	PRIVATE
	ret											; x^1 = x
KniPowFunction1	ENDP

ALIGN   8
KniPowFunction2	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	ret
KniPowFunction2	ENDP

ALIGN   8
KniPowFunction3	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmTmp1				; x^3
	ret
KniPowFunction3	ENDP

ALIGN   8
KniPowFunction4	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	ret
KniPowFunction4	ENDP

ALIGN   8
KniPowFunction5	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmTmp1				; x^5
	ret
KniPowFunction5	ENDP

ALIGN   8
KniPowFunction6	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmTmp1				; x^3
	mulps		xmmBase, xmmBase				; x^6
	ret
KniPowFunction6	ENDP

ALIGN   8
KniPowFunction7	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmTmp1				; x^7
	ret
KniPowFunction7	ENDP

ALIGN   8
KniPowFunction8	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	ret
KniPowFunction8	ENDP

ALIGN   8
KniPowFunction9	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmTmp1				; x^9
	ret
KniPowFunction9	ENDP

ALIGN   8
KniPowFunction10	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmTmp1				; x^10
	ret
KniPowFunction10	ENDP

ALIGN   8
KniPowFunction11	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmTmp1				; x^11
	ret
KniPowFunction11	ENDP

ALIGN   8
KniPowFunction12	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	movaps		xmmTmp1, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmTmp1				; x^12
	ret
KniPowFunction12	ENDP

ALIGN   8
KniPowFunction13	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^5
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmTmp1				; x^13
	ret
KniPowFunction13	ENDP

ALIGN   8
KniPowFunction14	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^6
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmTmp1				; x^14
	ret
KniPowFunction14	ENDP

ALIGN   8
KniPowFunction15	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^7
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmTmp1				; x^15
	ret
KniPowFunction15	ENDP

ALIGN   8
KniPowFunction16	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	ret
KniPowFunction16	ENDP

ALIGN   8
KniPowFunction17	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmTmp1				; x^17
	ret
KniPowFunction17	ENDP

ALIGN   8
KniPowFunction18	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmTmp1				; x^18
	ret
KniPowFunction18	ENDP

ALIGN   8
KniPowFunction19	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmTmp1				; x^19
	ret
KniPowFunction19	ENDP

ALIGN   8
KniPowFunction20	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	movaps		xmmTmp1, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmTmp1				; x^20
	ret
KniPowFunction20	ENDP

ALIGN   8
KniPowFunction21	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^5
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmTmp1				;  x^21
	ret
KniPowFunction21	ENDP

ALIGN   8
KniPowFunction22	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^6
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmTmp1				; x^22
	ret
KniPowFunction22	ENDP

ALIGN   8
KniPowFunction23	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^7
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmTmp1				; x^23
	ret
KniPowFunction23	ENDP

ALIGN   8
KniPowFunction24	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	movaps		xmmTmp1, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmTmp1				; x^24
	ret
KniPowFunction24	ENDP

ALIGN   8
KniPowFunction25	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^9
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmTmp1				; x^25
	ret
KniPowFunction25	ENDP

ALIGN   8
KniPowFunction26	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^10
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmTmp1				; x^26
	ret
KniPowFunction26	ENDP

ALIGN   8
KniPowFunction27	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^11
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmTmp1				; x^27
	ret
KniPowFunction27	ENDP

ALIGN   8
KniPowFunction28	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	movaps		xmmTmp1, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^12
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmTmp1				; x^28
	ret
KniPowFunction28	ENDP

ALIGN   8
KniPowFunction29	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^5
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^13
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmTmp1				; x^29
	ret
KniPowFunction29	ENDP

ALIGN   8
KniPowFunction30	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^6
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^14
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmTmp1				; x^30
	ret
KniPowFunction30	ENDP

ALIGN   8
KniPowFunction31	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^7
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^15
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmTmp1				; x^31
	ret
KniPowFunction31	ENDP

ALIGN   8
KniPowFunction32	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	ret
KniPowFunction32	ENDP

ALIGN   8
KniPowFunction33	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^33
	ret
KniPowFunction33	ENDP

ALIGN   8
KniPowFunction34	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^34
	ret
KniPowFunction34	ENDP

ALIGN   8
KniPowFunction35	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^35
	ret
KniPowFunction35	ENDP

ALIGN   8
KniPowFunction36	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	movaps		xmmTmp1, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^36
	ret
KniPowFunction36	ENDP

ALIGN   8
KniPowFunction37	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^5
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^37
	ret
KniPowFunction37	ENDP

ALIGN   8
KniPowFunction38	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^6
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^38
	ret
KniPowFunction38	ENDP

ALIGN   8
KniPowFunction39	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^7
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^39
	ret
KniPowFunction39	ENDP

ALIGN   8
KniPowFunction40	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	movaps		xmmTmp1, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^40
	ret
KniPowFunction40	ENDP

ALIGN   8
KniPowFunction41	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^9
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^41
	ret
KniPowFunction41	ENDP

ALIGN   8
KniPowFunction42	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^10
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^42
	ret
KniPowFunction42	ENDP

ALIGN   8
KniPowFunction43	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^11
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^43
	ret
KniPowFunction43	ENDP

ALIGN   8
KniPowFunction44	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	movaps		xmmTmp1, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^12
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^44
	ret
KniPowFunction44	ENDP

ALIGN   8
KniPowFunction45	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^5
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^13
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^45
	ret
KniPowFunction45	ENDP

ALIGN   8
KniPowFunction46	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^6
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^14
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^46
	ret
KniPowFunction46	ENDP

ALIGN   8
KniPowFunction47	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^7
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^15
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^47
	ret
KniPowFunction47	ENDP

ALIGN   8
KniPowFunction48	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	movaps		xmmTmp1, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^48
	ret
KniPowFunction48	ENDP

ALIGN   8
KniPowFunction49	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^17
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^49
	ret
KniPowFunction49	ENDP

ALIGN   8
KniPowFunction50	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^18
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^50
	ret
KniPowFunction50	ENDP

ALIGN   8
KniPowFunction51	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^19
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^51
	ret
KniPowFunction51	ENDP

ALIGN   8
KniPowFunction52	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	movaps		xmmTmp1, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^20
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^52
	ret
KniPowFunction52	ENDP

ALIGN   8
KniPowFunction53	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^5
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^21
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^53
	ret
KniPowFunction53	ENDP

ALIGN   8
KniPowFunction54	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^6
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^22
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^54
	ret
KniPowFunction54	ENDP

ALIGN   8
KniPowFunction55	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^7
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^23
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^55
	ret
KniPowFunction55	ENDP

ALIGN   8
KniPowFunction56	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	movaps		xmmTmp1, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^24
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^56
	ret
KniPowFunction56	ENDP

ALIGN   8
KniPowFunction57	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^9
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^25
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^57
	ret
KniPowFunction57	ENDP

ALIGN   8
KniPowFunction58	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^10
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^26
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^58
	ret
KniPowFunction58	ENDP

ALIGN   8
KniPowFunction59	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^11
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^27
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^59
	ret
KniPowFunction59	ENDP

ALIGN   8
KniPowFunction60	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	movaps		xmmTmp1, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^12
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^28
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^60
	ret
KniPowFunction60	ENDP

ALIGN   8
KniPowFunction61	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^5
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^13
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^29
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^61
	ret
KniPowFunction61	ENDP

ALIGN   8
KniPowFunction62	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^6
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^14
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^30
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^62
	ret
KniPowFunction62	ENDP

ALIGN   8
KniPowFunction63	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^7
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^15
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^31
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmTmp1				; x^63
	ret
KniPowFunction63	ENDP

ALIGN   8
KniPowFunction64	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	ret
KniPowFunction64	ENDP

ALIGN   8
KniPowFunction65	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^65
	ret
KniPowFunction65	ENDP

ALIGN   8
KniPowFunction66	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^66
	ret
KniPowFunction66	ENDP

ALIGN   8
KniPowFunction67	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^67
	ret
KniPowFunction67	ENDP

ALIGN   8
KniPowFunction68	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	movaps		xmmTmp1, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^68
	ret
KniPowFunction68	ENDP

ALIGN   8
KniPowFunction69	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^5
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^69
	ret
KniPowFunction69	ENDP

ALIGN   8
KniPowFunction70	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^6
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^70
	ret
KniPowFunction70	ENDP

ALIGN   8
KniPowFunction71	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^7
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^71
	ret
KniPowFunction71	ENDP

ALIGN   8
KniPowFunction72	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	movaps		xmmTmp1, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^72
	ret
KniPowFunction72	ENDP

ALIGN   8
KniPowFunction73	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^9
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^73
	ret
KniPowFunction73	ENDP

ALIGN   8
KniPowFunction74	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^10
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^74
	ret
KniPowFunction74	ENDP

ALIGN   8
KniPowFunction75	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^11
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^75
	ret
KniPowFunction75	ENDP

ALIGN   8
KniPowFunction76	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	movaps		xmmTmp1, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^12
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^76
	ret
KniPowFunction76	ENDP

ALIGN   8
KniPowFunction77	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^5
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^13
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^77
	ret
KniPowFunction77	ENDP

ALIGN   8
KniPowFunction78	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^6
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^14
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^78
	ret
KniPowFunction78	ENDP

ALIGN   8
KniPowFunction79	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^7
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^15
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^79
	ret
KniPowFunction79	ENDP

ALIGN   8
KniPowFunction80	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	movaps		xmmTmp1, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^80
	ret
KniPowFunction80	ENDP

ALIGN   8
KniPowFunction81	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^17
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^81
	ret
KniPowFunction81	ENDP

ALIGN   8
KniPowFunction82	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^18
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^82
	ret
KniPowFunction82	ENDP

ALIGN   8
KniPowFunction83	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^19
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^83
	ret
KniPowFunction83	ENDP

ALIGN   8
KniPowFunction84	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	movaps		xmmTmp1, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^20
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^84
	ret
KniPowFunction84	ENDP

ALIGN   8
KniPowFunction85	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^5
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^21
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^85
	ret
KniPowFunction85	ENDP

ALIGN   8
KniPowFunction86	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^6
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^22
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^86
	ret
KniPowFunction86	ENDP

ALIGN   8
KniPowFunction87	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^7
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^23
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^87
	ret
KniPowFunction87	ENDP

ALIGN   8
KniPowFunction88	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	movaps		xmmTmp1, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^24
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^88
	ret
KniPowFunction88	ENDP

ALIGN   8
KniPowFunction89	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^9
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^25
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^89
	ret
KniPowFunction89	ENDP

ALIGN   8
KniPowFunction90	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^10
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^26
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^90
	ret
KniPowFunction90	ENDP

ALIGN   8
KniPowFunction91	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^11
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^27
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^91
	ret
KniPowFunction91	ENDP

ALIGN   8
KniPowFunction92	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	movaps		xmmTmp1, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^12
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^28
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^92
	ret
KniPowFunction92	ENDP

ALIGN   8
KniPowFunction93	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^5
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^13
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^29
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^93
	ret
KniPowFunction93	ENDP

ALIGN   8
KniPowFunction94	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^6
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^14
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^30
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^94
	ret
KniPowFunction94	ENDP

ALIGN   8
KniPowFunction95	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmTmp1, xmmBase				; x^7
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmTmp1, xmmBase				; x^15
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmTmp1, xmmBase				; x^31
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^95
	ret
KniPowFunction95	ENDP

ALIGN   8
KniPowFunction96	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	movaps		xmmTmp1, xmmBase				; x^32
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^96
	ret
KniPowFunction96	ENDP


ALIGN   8
KniPowFunction97	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmTmp1, xmmBase				; x^33
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^97
	ret
KniPowFunction97	ENDP


ALIGN   8
KniPowFunction98	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	movaps		xmmTmp1, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmTmp1, xmmBase				; x^34
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^98
	ret
KniPowFunction98	ENDP

ALIGN   8
KniPowFunction99	PROC	PRIVATE
	movaps		xmmTmp1, xmmBase
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmTmp1, xmmBase				; x^3
	mulps		xmmBase, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmTmp1, xmmBase				; x^35
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^99
	ret
KniPowFunction99	ENDP


ALIGN   8
KniPowFunction100	PROC	PRIVATE
	mulps		xmmBase, xmmBase				; x^2
	mulps		xmmBase, xmmBase				; x^4
	movaps		xmmTmp1, xmmBase				; x^4
	mulps		xmmBase, xmmBase				; x^8
	mulps		xmmBase, xmmBase				; x^16
	mulps		xmmBase, xmmBase				; x^32
	mulps		xmmTmp1, xmmBase				; x^36
	mulps		xmmBase, xmmBase				; x^64
	mulps		xmmBase, xmmTmp1				; x^100
	ret
KniPowFunction100	ENDP


;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
COMMENT~

This code implements the exponent function using Intel SSE instructions.  
It calculates 4 exponents at once in about 80 clocks.  Exponents over +/-88.0f
are clamped to +/-88.0.  (The result is infinite at about 88.7).

The value sent to the function is expected to be in xmm0.  Likewise, the 
result is returned in xmm0.  This is faster than using the stack to pass 
parameters.

The other xmm registers as well as the mmx registers will be overwritten.

As with the exponent functions, this will run a hell of a lot faster if FTZ 
in MXCSR is asserted.


~ ;END COMMENT
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

;--------------- START OF CODE FOR TESTING THIS FUNCTION ---------------
ifdef TEST_KNI_EXP_FUNCTION
;******************************************************************************
; Since I don't want to load/unload the kni registers in the C code we'll
; through an extra call overhead and I'll do this in assembly
;
; Routine:  void _Asm_KniExpFunction(float *);
;******************************************************************************
PUBLIC _Asm_KniExpFunction
_Asm_KniExpFunction	PROC
	mov		eax, dword ptr [esp+4]
	movups	xmmBase, oword ptr [eax]
	call	_KniExp
	mov		eax, dword ptr [esp+4]
	movups	oword ptr [eax], xmmBase
	ret
_Asm_KniExpFunction	ENDP
endif ;TEST_KNI_EXP_FUNCTION
;--------------- END OF CODE FOR TESTING THIS FUNCTION ---------------

;*******************************************************************************************
; Routine:  void KniExp()
;
; Comments: Calculates exp(x) on the 4 values in xmm0 and returns them in xmm0.
;			All xmm and mmx registers are overwritten.  All integer registers
;			are unmodified.  Implemented with #4 (see below) so accuracy is 
;			about 10 bits and speed is about 55 clocks.
;
;           VERY IMPORTANT: Rounding mode must be set to "ROUND NEAREST"
;
;			Unlike the KniPower implementation/hack, which is really only good
;			for DirectX lighting, this is a pretty generic implementation.
;
; Psedo:
;	m128 KniExp(m128 e)
;	{
;		if (|e| > MAX)	x = MAX;
;		n = (int)( LOG2EF * |e| );  // rounding must be nearest
;		z = (float) n
;		x -= z * (C1+C2);
;
;		// use one of the 5 formulas in order of best->least accuracy and slowest->fastest
;/*1*/	x = 1/1! + x + x*x * (1/2! + x * (1/3! + x * (1/4! + x * (1/5! + x * (1/6! + x * 1/7!)))));
;/*2*/	x = 1/1! + x + x*x * (1/2! + x * (1/3! + x * (1/4! + x * 1/5!)));	// 75 clocks, good to 18 bits
;/*3*/	x = 1/1! + x + x*x * (1/2! + x * (1/3! + x * 1/4!));	// 65 clocks, good to 13 bits
;/*4*/	x = 1/1! + x + x*x * (1/2! + x * 1/3!);	// 55 clocks, good to 10 bits
;/*5*/	x = 1/1! + x + x*x * 1/2!;	// 45 clocks, good to 6 bits
;
;		n = 1.0 + n<<23;		// 1.0 + exponent
;		x *= n;					// add new exponent value
;		if(e < 0)	x = 1.0/x;	// do reciprocal if e was negative
;		return x;
;	}
;*******************************************************************************************
ALIGN   16
PUBLIC _KniExp
_KniExp	PROC

	xorps		xmmTmp1, xmmTmp1				; 0
	cmpps		xmmTmp1, xmmBase, XMM_LT		; x>0 ? -1 : 0
	andps		xmmBase, oword ptr [MagMask]	; (x & 7fffffff) = |x|
	minps		xmmBase, oword ptr [MAXLOG]		; clamp to max to keep this from blowing up (88.0)
	movaps		xmmTmp2, oword ptr [LOG2EF]		; 1.44
	mulps		xmmTmp2, xmmBase				; x * log2ef

	;; get nearest integer (rounding must be nearest!)
	movhlps		xmmTmp3, xmmTmp2
	cvtps2pi	mm0, xmmTmp2
	cvtps2pi	mm1, xmmTmp3
	cvtpi2ps	xmmTmp2, mm0
	cvtpi2ps	xmmTmp3, mm1
	movlhps		xmmTmp2, xmmTmp3				; z = int(x * log23f)

	mulps		xmmTmp2, oword ptr [C1PC2]		; z * -(C1+C2)  (-0.693)
	movaps		xmmTmp3, oword ptr [RCP_3FACT]	; 1/3! = 0.1666
	movq		mm2, qword ptr [ones]			; load the biased exponent (3f800000)
	psllq		mm0, 23							; shift x into the exponent position
	psllq		mm1, 23
	addps		xmmTmp2, xmmBase				; x = x - z*(C1+C2)
	paddd		mm0, mm2
	paddd		mm1, mm2

	;; do the taylor series (mix the mmx work in)
	movaps		xmmBase, oword ptr [RCP_1FACT]	; 1/1! = 1.0
	mulps		xmmTmp3, xmmTmp2				; x*1/3!
	addps		xmmBase, xmmTmp2				; 1/1! + x
	mulps		xmmTmp2, xmmTmp2				; x*x
	addps		xmmTmp3, oword ptr [RCP_2FACT]	; 1/2! + x*1/3! (1/2! = 0.5)
	movq		qword ptr [tmp128+0], mm0		; write biased exponents to memory (yuk!)
	movq		qword ptr [tmp128+8], mm1
	mulps		xmmTmp3, xmmTmp2				; x*x*(1/2! + x*1/3!)
	movaps		xmmTmp4, oword ptr [tmp128]		; exponent to add (from memory = yuk!)
	addps		xmmBase, xmmTmp3				; z = 1/1! + x + x*x*(1/2! + x*1/3!)
	xorps		xmmTmp5, xmmTmp5				; 0

	;; add the exponent back in
	mulps		xmmBase, xmmTmp4				; e = z + exp
	cmpps		xmmTmp5, xmmTmp1, XMM_EQ		; x>0 ? 0 : -1
	rcpps		xmmTmp2, xmmBase				; 1/e
	andps		xmmBase, xmmTmp1				; x>0 ? e : 0
	andps		xmmTmp5, xmmTmp2				; x>0 ? 0 : 1/e
	orps		xmmBase, xmmTmp5				; x>0 ? e : 1/e

	emms
	ret

_KniExp	ENDP



;*******************************************************************************************
; Routine:  void _Inverse4x4_SOA_ASM(D3DMATRIX *src, D3DMATRIX *dst)
;
; Comments: This function uses Cramer's Rule to calculate the matrix inverse.
;			See nt\private\windows\opengl\serever\soft\so_math.c
;			All xmm registers are overwritten.  Integer registers are not 
;			modified (except eax).
;
;	a11 = _11*_22 - _12*_21		b11 = _13*_24 - _14*_23
;	a12 = _11*_32 - _12*_31		b12 = _13*_34 - _14*_33
;	a13 = _11*_42 - _12*_41		b13 = _13*_44 - _14*_43
;	a22 = _21*_32 - _22*_31		b22 = _23*_34 - _24*_33
;	a23 = _21*_42 - _22*_41		b23 = _23*_44 - _24*_43
;	a33 = _31*_42 - _32*_41		b33 = _33*_44 - _34*_43
;
;	c01 = 12*23-13*22		d01 = 31*42-32*41
;	c02 = 11*22-12*21		d02 = 32*43-33*42
;	c03 = 14*21-11*24		d03 = 33*44-34*43
;	c04 = 13*24-14*23		d04 = 34*41-31*44
;	c05 = 11*22-14*23		d05 = 33*41-31*43
;	c06 = 14*21-13*22		d06 = 32*44-34*42
;	c07 = 13*24-12*21		d07 = 31*43-33*41
;	c08 = 12*23-11*24		d08 = 34*42-32*44
;	c09 = 13*21-11*23
;	c10 = 12*24-14*22
;	c11 = 11*23-13*21
;	c12 = 14*22-12*24
;
;*******************************************************************************************
ALIGN   16
PUBLIC _Inverse4x4_KNI_Asm
_Inverse4x4_KNI_Asm PROC
	mov		eax, dword ptr [esp+4]	; src

	movups    xmm0, [eax+32]	;	_34			_33			_32			_31
	movups    xmm3, [eax+48]	;	_44			_43			_42			_41
	movups    xmm2, [eax+16]	;	_24			_23			_22			_21

	shufps    xmm0, xmm0, 39h	;	_31			_34			_33			_32
	movhlps   xmm1, xmm3		;	---			---			_44			_43
	movlhps   xmm1, xmm3		;	_42			_41			_44			_43
	mulps     xmm1, xmm0		;	31*42		34*41		33*44		32*43
	movaps    xmm4, xmm1		;	31*42		34*41		33*44		32*43
	movaps    xmm5, xmm0		;	_31			_34			_33			_32
	mulps     xmm0, xmm3		;	31*44		34*43		33*42		32*41
	shufps    xmm3, xmm3, 93h	;	_43			_42			_41			_44
	mulps     xmm5, xmm3		;	31*43		34*42		33*41		32*44

	shufps    xmm0, xmm0, 39h	;	32*41		31*44		34*43		33*42
	subps     xmm4, xmm0		;	31*42-32*41	34*41-31*44	33*44-34*43	32*43-33*42 = d01 d02 d03 d04

	movaps    xmm7, xmm2		;	_24			_23			_22			_21
	mulps     xmm2, xmm4        ;	24*d01		23*d02		22*d03		21*d04
	shufps    xmm2, xmm2, 39h	;	21*d04		24*d01		23*d02		22*d03

	movhlps   xmm6, xmm5		;	---			---			31*43		34*42
	movlhps   xmm6, xmm5		;	33*41		32*44		31*43		34*42
	subps     xmm6, xmm5		;	33*41-31*43	32*44-34*42	31*43-33*41	34*42-32*44 = d05 d06 d07 d08

	movaps    xmm1, xmm4		;	d01			d02			d03			d04
	shufps    xmm4, xmm4, 39h	;	d04			d01			d02			d03

	movhlps   xmm5, xmm7		;	---			---			_24			_23
	movlhps   xmm5, xmm7		;	_22			_21			_24			_23
	movaps    xmm0, xmm5		;	_22			_21			_24			_23
	shufps    xmm7, xmm7, 93h	;	_23			_22			_21			_24

	mulps     xmm5, xmm6		;	22*d05		21*d06		24*d07		23*d08
	movaps    xmm3, xmm7		;	_23			_22			_21			_24
	addps     xmm2, xmm5		;	21*d04+22*d05	24*d01+21*d06	23*d02+24*d07	22*d03+23*d08

	mulps     xmm7, xmm1		;	23*d01		22*d02		21*d03		24*d04
	addps     xmm2, xmm7		;	23*d01+21*d04+22*d05	24*d01+22*d02+21*d06	23*d02+21*d03+24*d07	22*d03+24*d04+23*d08

	movups    xmm5, [eax+00]	;	_14			_13			_12			_11
	mulps     xmm5, xmm2		;	14*(23*d01+21*d04+22*d05)	13*(24*d01+22*d02+21*d06)	12*(23*d02+21*d03+24*d07)	11*(22*d03+24*d04+23*d08)
	xorps     xmm2, [Sign_PNPN]	;	+(23*d01+21*d04+22*d05)		-(24*d01+22*d02+21*d06)		+(23*d02+21*d03+24*d07)		-(22*d03+24*d04+23*d08)

	movaps    xmm7, xmm5		;	14*(23*d01+21*d04+22*d05)	13*(24*d01+22*d02+21*d06)	12*(23*d02+21*d03+24*d07)	11*(22*d03+24*d04+23*d08)
	movhlps   xmm5, xmm5		;	14*(23*d01+21*d04+22*d05)	13*(24*d01+22*d02+21*d06)	14*(23*d01+21*d04+22*d05)	13*(24*d01+22*d02+21*d06)
	addps     xmm7, xmm5		;	-- -- 12*(23*d02+21*d03+24*d07)+14*(23*d01+21*d04+22*d05)	11*(22*d03+24*d04+23*d08)+13*(24*d01+22*d02+21*d06)

	movhps    xmm5, [eax+00]	;	_12		_11		---		---
	movlps    xmm5, [eax+08]	;	_12		_11		_14		_13
	mulps     xmm6, xmm5		;	12*d05	11*d06	13*d08	13*d08

	shufps    xmm5, xmm5, 93h	;	_11		_14		_13		_12
	mulps     xmm4, xmm5		;	11*d04	14*d01	13*d02	12*d03
	shufps    xmm5, xmm5, 4eh	;	_13		_12		_11		_14

	addps     xmm4, xmm6		;	11*d04+12*d05	14*d01+11*d06	13*d02+13*d08	
	mulps     xmm5, xmm1		;	13*d01	12*d02	11*d03	14*d04

	movss     xmm6, xmm7		;	11*(22*d03+24*d04+23*d08)+13*(24*d01+22*d02+21*d06)
	movups    xmm1, [eax+00]	;	_14		_13		_12		_11

	shufps    xmm7, xmm7, 1		;	-- -- 11*(22*d03+24*d04+23*d08)+13*(24*d01+22*d02+21*d06)	12*(23*d02+21*d03+24*d07)+14*(23*d01+21*d04+22*d05)

	subss     xmm6, xmm7		;	11*(22*d03+24*d04+23*d08)-12*(23*d02+21*d03+24*d07)+13*(24*d01+22*d02+21*d06)-14*(23*d01+21*d04+22*d05)

	addps     xmm4, xmm5		;	13*d01+11*d04+12*d05	14*d01+12*d02+11*d06	11*d03+13*d02+13*d08	12*d03+14*d04+13*d08
	movups    xmm7, [eax+16]	;	_24		_23		_22		_21

	shufps    xmm1, xmm1, 39h	;	_11		_14		_13		_12
	movups    xmm5, [eax+48]	;	_44		_43		_42		_41
	mulps     xmm0, xmm1		;	11*22	14*21	13*24	12*23
	xorps     xmm4, [Sign_NPNP]	;	-(13*d01+11*d04+12*d05)	+(14*d01+12*d02+11*d06)	-(11*d03+13*d02+13*d08)	+(12*d03+14*d04+13*d08)
	mulps     xmm7, xmm1		;	11*24	14*23	13*22	12*21
	shufps    xmm5, xmm5, 39h	;	_41		_44		_43		_42

	mulps     xmm3, xmm1		;	11*23	14*22	13*21	12*24
	movss    [tmp128], xmm6		;	11*(22*d03+24*d04+23*d08)-12*(23*d02+21*d03+24*d07)+13*(24*d01+22*d02+21*d06)-14*(23*d01+21*d04+22*d05)
	movaps    xmm1, xmm0		;	11*22	14*21	13*24	12*23
	shufps    xmm0, xmm0, 39h	;	12*23	11*22	14*21	13*24

	movhlps   xmm6, xmm7		;	---		---		11*24	14*23
	movlhps   xmm6, xmm7		;	13*22	12*21	11*24	14*23
	shufps    xmm7, xmm7, 39h	;	14*23	13*22	12*21	11*24

	subps     xmm0, xmm6		;	12*23-13*22	11*22-12*21	14*21-11*24	13*24-14*23 = c01 c02 c03 c04
	subps     xmm1, xmm7		;	11*22-14*23	14*21-13*22	13*24-12*21	12*23-11*24 = c05 c06 c07 c08
	movaps    xmm6, xmm5		;	_41		_44		_43		_42
	mulps     xmm5, xmm0		;	41*c01	44*c02	43*c03	42*c04
	shufps    xmm6, xmm6, 39h	;	_42		_41		_44		_43

	movhlps   xmm7, xmm3		;	---		---		11*23	14*22
	movlhps   xmm7, xmm3		;	13*21	12*24	11*23	14*22
	subps     xmm7, xmm3		;	13*21-11*23	12*24-14*22	11*23-13*21	14*22-12*24 = c09 c10 c11 c12

	movaps    xmm3, xmm6		;	_42		_41		_44		_43
	mulps     xmm6, xmm7		;	42*c09	41*c10	44*c11	43*c12
	shufps    xmm3, xmm3, 39h	;	_41		_44		_43		_42
	mulps     xmm3, xmm1		;	41*c05	44*c06	43*c07	42*c08
	addps     xmm5, xmm6		;	41*c01+42*c09	44*c02+41*c10	43*c03+44*c11	42*c04+43*c12
	rcpss	  xmm6, [tmp128]	;	1/(11*(22*d03+24*d04+23*d08)-12*(23*d02+21*d03+24*d07)+13*(24*d01+22*d02+21*d06)-14*(23*d01+21*d04+22*d05)) = determinant

	addps     xmm5, xmm3		;	41*c01+41*c05+42*c09	44*c02+44*c06+41*c10	43*c03+43*c07+44*c11	42*c04+42*c08+43*c12

	shufps    xmm6, xmm6, 0		;	broadcast the determinant (D) = d d d d
	xorps     xmm5, [Sign_PNPN]	;	+(41*c01+41*c05+42*c09)		-(44*c02+44*c06+41*c10)		+(43*c03+43*c07+44*c11)		-(42*c04+42*c08+43*c12)
	movups    xmm3, [eax+32]	;	_34		_33		_32		_31

	mulps     xmm5, xmm6		;	+(41*c01+41*c05+42*c09)/D	-(44*c02+44*c06+41*c10)/D	+(43*c03+43*c07+44*c11)/D	-(42*c04+42*c08+43*c12)/D
	shufps    xmm3, xmm3, 39h	;	_31		_34		_33		_32
	mulps     xmm2, xmm6		;	+(23*d01+21*d04+22*d05)/D	-(24*d01+22*d02+21*d06)/D	+(23*d02+21*d03+24*d07)/D	-(22*d03+24*d04+23*d08)/D
	mulps     xmm0, xmm3		;	31*c01	34*c02	32*c03	31*c04

	shufps    xmm3, xmm3, 39h	;	_32		_31		_34		_33
	mulps     xmm7, xmm3		;	32*c09	31*c10	34*c11	33*c12
	shufps    xmm3, xmm3, 39h	;	_33		_32		_31		_34

	addps     xmm0, xmm7		;	31*c01+32*c09	34*c02+31*c10	32*c03+34*c11	31*c04+33*c12
	mulps     xmm3, xmm1		;	33*c05	32*c06	31*c07	34*c08
	addps     xmm0, xmm3		;	31*c01+33*c05+32*c09	34*c02+32*c06+31*c10	32*c03+31*c07+34*c11	31*c04+34*c08+33*c12

	xorps     xmm0, [Sign_NPNP]	;	-(31*c01+33*c05+32*c09)		+(34*c02+32*c06+31*c10)		-(32*c03+31*c07+34*c11)		+(31*c04+34*c08+33*c12)

	mulps     xmm4, xmm6		;	-(13*d01+11*d04+12*d05)/D	+(14*d01+12*d02+11*d06)/D	-(11*d03+13*d02+13*d08)/D	+(12*d03+14*d04+13*d08)/D
	mov       eax, dword ptr [esp+8]	; dst
	mulps     xmm0, xmm6		;	-(31*c01+33*c05+32*c09)/D	+(34*c02+32*c06+31*c10)/D	-(32*c03+31*c07+34*c11)/D	+(31*c04+34*c08+33*c12)/D

	movaps    xmm7, xmm2		;	+(23*d01+21*d04+22*d05)/D	-(24*d01+22*d02+21*d06)/D	+(23*d02+21*d03+24*d07)/D	-(22*d03+24*d04+23*d08)/D
	unpcklps  xmm2, xmm4		;	-(11*d03+13*d02+13*d08)/D	+(23*d02+21*d03+24*d07)/D	+(12*d03+14*d04+13*d08)/D	-(22*d03+24*d04+23*d08)/D
	unpckhps  xmm7, xmm4		;	-(13*d01+11*d04+12*d05)/D	+(23*d01+21*d04+22*d05)/D	+(14*d01+12*d02+11*d06)/D	-(24*d01+22*d02+21*d06)/D

	movaps    xmm1, xmm5		;	+(41*c01+41*c05+42*c09)/D	-(44*c02+44*c06+41*c10)/D	+(43*c03+43*c07+44*c11)/D	-(42*c04+42*c08+43*c12)/D
	unpcklps  xmm5, xmm0		;	-(32*c03+31*c07+34*c11)/D	+(43*c03+43*c07+44*c11)/D	+(31*c04+34*c08+33*c12)/D	-(42*c04+42*c08+43*c12)/D
	unpckhps  xmm1, xmm0		;	-(31*c01+33*c05+32*c09)/D	+(41*c01+41*c05+42*c09)/D	+(34*c02+32*c06+31*c10)/D	-(44*c02+44*c06+41*c10)/D

	movlps    [eax+00], xmm2	;	+(12*d03+14*d04+13*d08)/D	-(22*d03+24*d04+23*d08)/D
	movlps    [eax+08], xmm5	;	+(43*c03+43*c07+44*c11)/D	-(42*c04+42*c08+43*c12)/D
	movhps    [eax+16], xmm2	;	-(11*d03+13*d02+13*d08)/D	+(23*d02+21*d03+24*d07)/D
	movhps    [eax+24], xmm5	;	-(32*c03+31*c07+34*c11)/D	+(43*c03+43*c07+44*c11)/D
	movlps    [eax+32], xmm7	;	+(14*d01+12*d02+11*d06)/D	-(24*d01+22*d02+21*d06)/D
	movlps    [eax+40], xmm1	;	+(34*c02+32*c06+31*c10)/D	-(44*c02+44*c06+41*c10)/D
	movhps    [eax+48], xmm7	;	-(13*d01+11*d04+12*d05)/D	+(23*d01+21*d04+22*d05)/D
	movhps    [eax+56], xmm1	;	-(31*c01+33*c05+32*c09)/D	+(41*c01+41*c05+42*c09)/D

	ret
_Inverse4x4_KNI_Asm ENDP



;*******************************************************************************************
; Routine:  void _MatrixProduct_KNI_Asm(D3DMATRIX *a, D3DMATRIX *b, D3DMATRIX* dst)
;
;	dst->_11 = a->_11*b->_11 + a->_12*b->_21 + a->_13*b->_31 + a->_14*b->_41;   \
;	dst->_12 = a->_11*b->_12 + a->_12*b->_22 + a->_13*b->_32 + a->_14*b->_42;   \
;	dst->_13 = a->_11*b->_13 + a->_12*b->_23 + a->_13*b->_33 + a->_14*b->_43;   \
;	dst->_14 = a->_11*b->_14 + a->_12*b->_24 + a->_13*b->_34 + a->_14*b->_44;   \
;
;	dst->_21 = a->_21*b->_11 + a->_22*b->_21 + a->_23*b->_31 + a->_24*b->_41;   \
;	dst->_22 = a->_21*b->_12 + a->_22*b->_22 + a->_23*b->_32 + a->_24*b->_42;   \
;	dst->_23 = a->_21*b->_13 + a->_22*b->_23 + a->_23*b->_33 + a->_24*b->_43;   \
;	dst->_24 = a->_21*b->_14 + a->_22*b->_24 + a->_23*b->_34 + a->_24*b->_44;   \
;
;	dst->_31 = a->_31*b->_11 + a->_32*b->_21 + a->_33*b->_31 + a->_34*b->_41;   \
;	dst->_32 = a->_31*b->_12 + a->_32*b->_22 + a->_33*b->_32 + a->_34*b->_42;   \
;	dst->_33 = a->_31*b->_13 + a->_32*b->_23 + a->_33*b->_33 + a->_34*b->_43;   \
;	dst->_34 = a->_31*b->_14 + a->_32*b->_24 + a->_33*b->_34 + a->_34*b->_44;   \
;
;	dst->_41 = a->_41*b->_11 + a->_42*b->_21 + a->_43*b->_31 + a->_44*b->_41;   \
;	dst->_42 = a->_41*b->_12 + a->_42*b->_22 + a->_43*b->_32 + a->_44*b->_42;   \
;	dst->_43 = a->_41*b->_13 + a->_42*b->_23 + a->_43*b->_33 + a->_44*b->_43;   \
;	dst->_44 = a->_41*b->_14 + a->_42*b->_24 + a->_43*b->_34 + a->_44*b->_44;
;
;*******************************************************************************************
ALIGN   16
_MatrixProduct_KNI_Asm PROC
	mov		edx, dword ptr [esp+8]	; b
	mov		eax, dword ptr [esp+4]	; a
	movups	xmm4, [edx+00]		;	b14		b13		b12		b11
	movups	xmm5, [edx+16]		;	b24		b23		b22		b21
	movups	xmm6, [edx+32]		;	b34		b33		b32		b31
	movups	xmm7, [edx+48]		;	b44		b43		b42		b41
	mov		edx, dword ptr [esp+12]	; dst

	movss	xmm0, [eax+00]		;	0		0		0		a11
	movss	xmm1, [eax+04]		;	0		0		0		a12
	movss	xmm2, [eax+08]		;	0		0		0		a13
	shufps	xmm0, xmm0, 0		;	a11		a11		a11		a11
	movss	xmm3, [eax+12]		;	0		0		0		a14
	shufps	xmm1, xmm1, 0		;	a12		a12		a12		a12
	prefetchnta [eax+32]
	mulps	xmm0, xmm4			;	a11*b14	a11*b13	a11*b12	a11*b11
	shufps	xmm2, xmm2, 0		;	a13		a13		a13		a13
	shufps	xmm3, xmm3, 0		;	a14		b14		a14		a14
	mulps	xmm1, xmm5			;	a12*b24	a12*b23	a12*b22	a12*b21
	prefetchnta [eax+48]
	mulps	xmm2, xmm6			;	a13*b34	a13*b33	a13*b32	a13*b31
	mulps	xmm3, xmm7			;	a14*b44	a14*b43	a14*b42	a14*b41
	addps	xmm1, xmm0			;	a11*b14+a12*b24	a11*b13+a12*b23	a11*b12+a12*b22	a11*b11+a12*b21
	addps	xmm3, xmm2			;	a13*b34+a14*b44	a13*b33+a14*b43	a13*b32+a14*b42	a13*b31+a14*b41
	addps	xmm3, xmm1			;	a11*b14 + a12*b24 + a13*b34 + a14*b44
								;	a11*b13 + a12*b23 + a13*b33 + a14*b43
								;	a11*b12 + a12*b22 + a13*b32 + a14*b42
	movups	[edx+00], xmm3		;	a11*b11 + a12*b21 + a13*b31 + a14*b41

	movss	xmm0, [eax+16]		;	0		0		0		a21
	movss	xmm1, [eax+20]		;	0		0		0		a22
	movss	xmm2, [eax+24]		;	0		0		0		a23
	shufps	xmm0, xmm0, 0		;	a21		a21		a21		a21
	movss	xmm3, [eax+28]		;	0		0		0		a24
	shufps	xmm1, xmm1, 0		;	a22		a22		a22		a22
	mulps	xmm0, xmm4			;	a21*b14	a21*b13	a21*b12	a21*b11
	shufps	xmm2, xmm2, 0		;	a23		a23		a23		a23
	shufps	xmm3, xmm3, 0		;	a24		b14		a24		a24
	mulps	xmm1, xmm5			;	a22*b24	a22*b23	a22*b22	a22*b21
	mulps	xmm2, xmm6			;	a23*b34	a23*b33	a23*b32	a23*b31
	mulps	xmm3, xmm7			;	a24*b44	a24*b43	a24*b42	a24*b41
	addps	xmm1, xmm0			;	a21*b14+a22*b24	a21*b13+a22*b23	a21*b12+a22*b22	a21*b11+a22*b21
	addps	xmm3, xmm2			;	a23*b34+a24*b44	a23*b33+a24*b43	a23*b32+a24*b42	a23*b31+a24*b41
	addps	xmm3, xmm1			;	a21*b14 + a22*b24 + a23*b34 + a24*b44
								;	a21*b13 + a22*b23 + a23*b33 + a24*b43
								;	a21*b12 + a22*b22 + a23*b32 + a24*b42
	movups	[edx+16], xmm3		;	a21*b11 + a22*b21 + a23*b31 + a24*b41

	movss	xmm0, [eax+32]		;	0		0		0		a31
	movss	xmm1, [eax+36]		;	0		0		0		a32
	movss	xmm2, [eax+40]		;	0		0		0		a33
	shufps	xmm0, xmm0, 0		;	a31		a31		a31		a31
	movss	xmm3, [eax+44]		;	0		0		0		a34
	shufps	xmm1, xmm1, 0		;	a32		a32		a32		a32
	mulps	xmm0, xmm4			;	a31*b14	a31*b13	a31*b12	a31*b11
	shufps	xmm2, xmm2, 0		;	a33		a33		a33		a33
	shufps	xmm3, xmm3, 0		;	a34		b14		a34		a34
	mulps	xmm1, xmm5			;	a32*b24	a32*b23	a32*b22	a32*b21
	mulps	xmm2, xmm6			;	a33*b34	a33*b33	a33*b32	a33*b31
	mulps	xmm3, xmm7			;	a34*b44	a34*b43	a34*b42	a34*b41
	addps	xmm1, xmm0			;	a31*b14+a32*b24	a31*b13+a32*b23	a31*b12+a32*b22	a31*b11+a32*b21
	addps	xmm3, xmm2			;	a33*b34+a34*b44	a33*b33+a34*b43	a33*b32+a34*b42	a33*b31+a34*b41
	addps	xmm3, xmm1			;	a31*b14 + a32*b24 + a33*b34 + a34*b44
								;	a31*b13 + a32*b23 + a33*b33 + a34*b43
								;	a31*b12 + a32*b22 + a33*b32 + a34*b42
	movups	[edx+32], xmm3		;	a31*b11 + a32*b21 + a33*b31 + a34*b41

	movss	xmm0, [eax+48]		;	0		0		0		a41
	movss	xmm1, [eax+52]		;	0		0		0		a42
	movss	xmm2, [eax+56]		;	0		0		0		a43
	shufps	xmm0, xmm0, 0		;	a41		a41		a41		a41
	movss	xmm3, [eax+60]		;	0		0		0		a44
	shufps	xmm1, xmm1, 0		;	a42		a42		a42		a42
	shufps	xmm2, xmm2, 0		;	a43		a43		a43		a43
	shufps	xmm3, xmm3, 0		;	a44		b14		a44		a44
	mulps	xmm0, xmm4			;	a41*b14	a41*b13	a41*b12	a41*b11
	mulps	xmm1, xmm5			;	a42*b24	a42*b23	a42*b22	a42*b21
	mulps	xmm2, xmm6			;	a43*b34	a43*b33	a43*b32	a43*b31
	mulps	xmm3, xmm7			;	a44*b44	a44*b43	a44*b42	a44*b41
	addps	xmm1, xmm0			;	a41*b14+a42*b24	a41*b13+a42*b23	a41*b12+a42*b22	a41*b11+a42*b21
	addps	xmm3, xmm2			;	a43*b34+a44*b44	a43*b33+a44*b43	a43*b32+a44*b42	a43*b31+a44*b41
	addps	xmm3, xmm1			;	a41*b14 + a42*b24 + a43*b34 + a44*b44
								;	a41*b13 + a42*b23 + a43*b33 + a44*b43
								;	a41*b12 + a42*b22 + a43*b32 + a44*b42
	movups	[edx+48], xmm3		;	a41*b11 + a42*b21 + a43*b31 + a44*b41

	ret
_MatrixProduct_KNI_Asm	ENDP

_TEXT   ENDS


END
