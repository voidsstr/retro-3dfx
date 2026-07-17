TITLE   k3dmath.asm
.686
.K3D
ASSUME cs:FLAT,ds:FLAT,es:FLAT,fs:FLAT,gs:FLAT
.MODEL FLAT


;******************************************************************************
;
; Copyright (c) 1999 Advanced Micro Devices, Inc.
;
; LIMITATION OF LIABILITY:  THE MATERIALS ARE PROVIDED *AS IS* WITHOUT ANY
; EXPRESS OR IMPLIED WARRANTY OF ANY KIND INCLUDING WARRANTIES OF MERCHANTABILITY,
; NONINFRINGEMENT OF THIRD-PARTY INTELLECTUAL PROPERTY, OR FITNESS FOR ANY
; PARTICULAR PURPOSE.  IN NO EVENT SHALL AMD OR ITS SUPPLIERS BE LIABLE FOR ANY
; DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF PROFITS,
; BUSINESS INTERRUPTION, LOSS OF INFORMATION) ARISING OUT OF THE USE OF OR
; INABILITY TO USE THE MATERIALS, EVEN IF AMD HAS BEEN ADVISED OF THE POSSIBILITY
; OF SUCH DAMAGES.  BECAUSE SOME JURISDICTIONS PROHIBIT THE EXCLUSION OR LIMITATION
; OF LIABILITY FOR CONSEQUENTIAL OR INCIDENTAL DAMAGES, THE ABOVE LIMITATION MAY
; NOT APPLY TO YOU.
;
; AMD does not assume any responsibility for any errors which may appear in the
; Materials nor any responsibility to support or update the Materials.  AMD retains
; the right to make changes to its test specifications at any time, without notice.
;
; NO SUPPORT OBLIGATION: AMD is not obligated to furnish, support, or make any
; further information, software, technical information, know-how, or show-how
; available to you.
;
; So that all may benefit from your experience, please report  any  problems
; or  suggestions about this software to 3dsdk.support@amd.com
;
; AMD Developer Technologies, M/S 585
; Advanced Micro Devices, Inc.
; 5900 E. Ben White Blvd.
; Austin, TX 78741
; 3dsdk.support@amd.com
;
; AMD3D 3D library code: Math primitives
;
;******************************************************************************


.CONST
;******************************************************************************
; Scalar (single float) data
;******************************************************************************

Align 8
	ones		dd      2 DUP(1.0)
	halves		dd		2 DUP(0.5)
	MAXLOG		dd		2 DUP(87.0)			;; over exp(87) blows up (NaN)
	MagMask		dd		2 DUP(07fffffffh)
	C1PC2		dd		2 DUP(0bf317218h)	;;-0.693147182
	LOG2EF		dd		2 DUP(03fb8aa3bh)	;; 1.442695022
	RCP_1FACT	equ		ones				;; 1.000000000 = 1/1!
	RCP_2FACT	dd		2 DUP(03f000000h)	;; 0.500000000 = 1/2!
	RCP_3FACT	dd		2 DUP(03e2aaaabh)	;; 0.166666667 = 1/3!
	;RCP_4FACT	dd		2 DUP(03d2aaaabh)	;; 0.041666667 = 1/4!
	;RCP_5FACT	dd		2 DUP(03c088889h)	;; 0.008333333 = 1/5!
	;RCP_6FACT	dd		2 DUP(03ab60b61h)	;; 0.001398888 = 1/6!
	;RCP_7FACT	dd		2 DUP(039500d01h)	;; 0.000198413 = 1/7!

	mabs        dd      07FFFFFFFh              ; mask for absolute value (~sgn)
	mant        dd      0007FFFFFh              ; mask for mantissa
	expo        dd      07F800000h              ; mask for exponent
	two         dd      040000000h              ; 2.0
	rt2         dd      03FB504F3h
	edec        dd      000800000h
	bias        dd      00000007Fh
	c2          dd      03E18EFE2h
	c1          dd      03E4CAF6Fh
	c0          dd      03EAAAABDh
	tl2e        dd      04038AA3Bh
	maxn        dd      0FF7FFFFFh
	q1          dd      043BC00B5h
	p1          dd      041E77545h
	q0          dd      045E451C5h
	p0          dd      0451E424Bh
	mine        dd      0C2FC0000h
	maxe        dd      043000000h
	fltmax      dd      07F7FFFFFh              ; FLT_MAX
	flt4		dd		4.0


;******************************************************************************
; DATA - segment and definitions
;******************************************************************************
.DATA
Align 16
	fIntpFactorLow					dd	2 DUP(?)		; lower interpolation factor
	fIntpFactorHi					dd	2 DUP(?)		; upper interpolation factor
	lpK3dPowerFunctionIntpLow		dd	?				; interpolation lower power function
	lpK3dPowerFunctionNegative		dd	?				; interpolation power function called by negative powers
;	lpOSPowFunction					dd	?				; this is just for testing the power function

Align 8
	tmpMM2		dq	?
	tmpMM3		dq	?
	tmpMM4		dq	?
	tmpMM5		dq	?
	tmpMM6		dq	?
	tmpMM7		dq	?


;******************************************************************************
;******************************************************************************
;******************************************************************************
; CODE 
;******************************************************************************
_TEXT   SEGMENT 
.CODE

; leave this as far a possible from where this is used, so we don't get data into the instruction cache
LookupTable_K3dPowFunction:
	dd	K3dPowFunction0,	K3dPowFunction1,	K3dPowFunction2,	K3dPowFunction3,	K3dPowFunction4
	dd	K3dPowFunction5,	K3dPowFunction6,	K3dPowFunction7,	K3dPowFunction8,	K3dPowFunction9
	dd	K3dPowFunction10,	K3dPowFunction11,	K3dPowFunction12,	K3dPowFunction13,	K3dPowFunction14
	dd	K3dPowFunction15,	K3dPowFunction16,	K3dPowFunction17,	K3dPowFunction18,	K3dPowFunction19
	dd	K3dPowFunction20,	K3dPowFunction21,	K3dPowFunction22,	K3dPowFunction23,	K3dPowFunction24
	dd	K3dPowFunction25,	K3dPowFunction26,	K3dPowFunction27,	K3dPowFunction28,	K3dPowFunction29
	dd	K3dPowFunction30,	K3dPowFunction31,	K3dPowFunction32,	K3dPowFunction33,	K3dPowFunction34
	dd	K3dPowFunction35,	K3dPowFunction36,	K3dPowFunction37,	K3dPowFunction38,	K3dPowFunction39
	dd	K3dPowFunction40,	K3dPowFunction41,	K3dPowFunction42,	K3dPowFunction43,	K3dPowFunction44
	dd	K3dPowFunction45,	K3dPowFunction46,	K3dPowFunction47,	K3dPowFunction48,	K3dPowFunction49
	dd	K3dPowFunction50,	K3dPowFunction51,	K3dPowFunction52,	K3dPowFunction53,	K3dPowFunction54
	dd	K3dPowFunction55,	K3dPowFunction56,	K3dPowFunction57,	K3dPowFunction58,	K3dPowFunction59
	dd	K3dPowFunction60,	K3dPowFunction61,	K3dPowFunction62,	K3dPowFunction63,	K3dPowFunction64
	dd	K3dPowFunction65,	K3dPowFunction66,	K3dPowFunction67,	K3dPowFunction68,	K3dPowFunction69
	dd	K3dPowFunction70,	K3dPowFunction71,	K3dPowFunction72,	K3dPowFunction73,	K3dPowFunction74
	dd	K3dPowFunction75,	K3dPowFunction76,	K3dPowFunction77,	K3dPowFunction78,	K3dPowFunction79
	dd	K3dPowFunction80,	K3dPowFunction81,	K3dPowFunction82,	K3dPowFunction83,	K3dPowFunction84
	dd	K3dPowFunction85,	K3dPowFunction86,	K3dPowFunction87,	K3dPowFunction88,	K3dPowFunction89
	dd	K3dPowFunction90,	K3dPowFunction91,	K3dPowFunction92,	K3dPowFunction93,	K3dPowFunction94
	dd	K3dPowFunction95,	K3dPowFunction96,	K3dPowFunction97,	K3dPowFunction98,	K3dPowFunction99
	dd	K3dPowFunction100


mmBase		TEXTEQU		<mm0>		; base and result register
mmTmp1		TEXTEQU		<mm1>		; Temporary registers which
mmTmp2		TEXTEQU		<mm2>		; may be overwritten
mmTmp3		TEXTEQU		<mm3>		; may be overwritten
mmTmp4		TEXTEQU		<mm4>		; may be overwritten


;*******************************************************************************************
; Routine:  void K3dExp()
;
; Comments: Calculates exp(x) on the 2 values in mm0 and returns them in mm0.
;			All mmx registers 1-4 are overwritten.  All integer registers
;			are unmodified.  Implemented with #4 (see below) so accuracy is 
;			about 10 bits. Speed is about 30 clocks for one exponent or 35
;			clocks for two.
;
;			Unlike the K3dPower implementation/hack, which is really only good
;			for DirectX lighting, this is a pretty generic implementation.
;
; Psedo:
;	float K3dExp(float e)
;	{
;		if (|e| > MAX)	x = MAX;
;		n = (int)( LOG2EF * |e| );  // rounding must be nearest
;		z = (float) n
;		x -= z * (C1+C2);
;
;		// use one of the 5 formulas in order of best->least accuracy and slowest->fastest
;/*1*/	x = 1/1! + x + x*x * (1/2! + x * (1/3! + x * (1/4! + x * (1/5! + x * (1/6! + x * 1/7!)))));
;/*2*/	x = 1/1! + x + x*x * (1/2! + x * (1/3! + x * (1/4! + x * 1/5!)));	// good to 18 bits
;/*3*/	x = 1/1! + x + x*x * (1/2! + x * (1/3! + x * 1/4!));	// good to 13 bits
;/*4*/	x = 1/1! + x + x*x * (1/2! + x * 1/3!);	// good to 10 bits
;/*5*/	x = 1/1! + x + x*x * 1/2!;	// good to 6 bits
;
;		n = 1.0 + n<<23;		// 1.0 + exponent
;		x *= n;					// add new exponent value
;		if(e < 0)	x = 1.0/x;	// do reciprocal if e was negative
;		return x;
;	}
;*******************************************************************************************
DO_TWO_EXPONENTS equ 0
ALIGN   16
PUBLIC _K3dExp
_K3dExp	PROC
	pxor		mmTmp1, mmTmp1					; 0
	pfcmpgt		mmTmp1, mmBase					; x >= 0 ? 0 : -1

	pand		mmBase, qword ptr [MagMask]		; (x & 7fffffff) = |x|
	pfmin		mmBase, qword ptr [MAXLOG]		; clamp to max to keep this from blowing up (88.0)
	movq		mmTmp2, qword ptr [LOG2EF]		; 1.44
	pfmul		mmTmp2, mmBase					; x * log2ef

	;; get nearest integer (rounding must be nearest!)
	pf2id		mmTmp3, mmTmp2
	pi2fd		mmTmp2, mmTmp3					; z = int(x * log23f)

	pfmul		mmTmp2, qword ptr [C1PC2]		; z * -(C1+C2)  (-0.693)
	psllq		mmTmp3, 23							; shift x into the exponent position
	movq		mmTmp4, qword ptr [RCP_3FACT]	; 1/3! = 0.1666
	pfadd		mmTmp2, mmBase					; x = x - z*(C1+C2)

	;; do the taylor series
	movq		mmBase, qword ptr [RCP_1FACT]	; 1/1! = 1.0
	paddd		mmTmp3, qword ptr [ones]		; load the biased exponent (3f800000)
	pfmul		mmTmp4, mmTmp2					; x*1/3!
	pfadd		mmBase, mmTmp2					; 1/1! + x
	pfmul		mmTmp2, mmTmp2					; x*x
	pfadd		mmTmp4, qword ptr [RCP_2FACT]	; 1/2! + x*1/3! (1/2! = 0.5)
	pfmul		mmTmp4, mmTmp2					; x*x*(1/2! + x*1/3!)
	pfadd		mmBase, mmTmp4					; z = 1/1! + x + x*x*(1/2! + x*1/3!)
	
	;; add the exponent back in
	pfmul		mmBase, mmTmp3					; e = z + exp
	pxor		mmTmp4, mmTmp4					; 0
IF DO_TWO_EXPONENTS
	movq		mmTmp3, mmBase
	punpckhdq	mmTmp3, mmTmp3
ENDIF
	pfrcp		mmTmp2, mmBase					; 1/e (low half)
	pfcmpeq		mmTmp4, mmTmp1					; x>=0 ? 0 : -1
IF DO_TWO_EXPONENTS
	pfrcp		mmTmp3, mmTmp3
	punpckldq	mmTmp2, mmTmp3
ENDIF
	pand		mmBase, mmTmp4					; x>0 ? e : 0
	pand		mmTmp1, mmTmp2					; x>0 ? 0 : 1/e
	por			mmBase, mmTmp1					; x>0 ? e : 1/e

	ret
_K3dExp ENDP



;******************************************************************************
; Routine:  k3dpow_amd - compute pow(mm0.lo,mm1.lo)
; Input:    mm0.lo - base
;           mm1.lo - exponent
; Result:   stored in mm0.lo
; Uses:     mm0-mm7
; Comment:
;   Compute pow(x,y) using MMX and 3DNow! instructions. Scalar version.
;
;   This routine is an almost perfect replacement for the C library
;   function pow(). The main differences are:
;
;   o If one of the inputs has an exponent of 0xFF, the result of
;     this routine is undefined. Inputs with an exponent of 0 are
;     treated as true zeros.
;   o Flushes underflowed results to 0 and clamps overflowed results
;     to the maximum single precision normal.
;   o No error and different result for certain undefined cases,
;     (e.g. negative x raised to non-integer power y; In particular
;     0 ^ y = 0, for all y).
;
;   This routine uses inline versions of y*log(abs(x)) and 2^x
;   routines which may also be used seperately. It computes pow(x,y)
;   as x^y = 2^y*log2(x)). This is fast, but numerically this is not
;   the optimal method.
;
;   This routine has a worst case accuracy of about 16 bits, but
;   for many arguments the accuracy is significantly better, often
;   19 bits or more.
;
;******************************************************************************
ALIGN   16
PUBLIC  _k3dpow_amd
_k3dpow_amd   PROC
		; save all register except 0 and 1
		movq		[tmpMM2], mm2
		movq		[tmpMM3], mm3
		movq		[tmpMM4], mm4
		movq		[tmpMM5], mm5
		movq		[tmpMM6], mm6
		movq		[tmpMM7], mm7

        movq        mm6, mm1    ; y
        pf2id       mm6, mm6    ; int(y)
        pslld       mm6, 31     ; (int(y)&1)<<31
        pand        mm6, mm0    ; bit<31> = (x<0) && (y&1)

;       Compute y*log2(abs(x)) using MMX and 3DNow! instructions. Scalar version.
;
;       If one of the inputs has an exponent of 0xFF, the result of this
;       routine is undefined. Inputs with an exponent of 0 are treated
;       as true zeros. The following cases occur:
;
;       expo(x) == 0 && expo(y) == 0  --> y*log2(abs(x)) = - max_normal
;       expo(x) == 0 && expo(y) != 0  --> y*log2(abs(x)) = - max_normal
;       expo(x) != 0 && expo(y) == 0  --> y*log2(abs(x)) = 0
;
;       Results whose absolute value is less than min_normal are flushed
;       to zero. Results whose absolute value exceeds max_normal are
;       clamped to +/- max_normal.
;
;       The input x = 2^k * m, thus log2(x) = k + log2mm). log2mm) =
;       log2(e)*logmm). Here, m is chosen such than m is < sqrt(2) and
;       k is ajusted accordingly. Then, logmm) = 2*artanhmm-1)/mm+1). A
;       polynomial minimax approximation is used to compute artanh(z),
;       where z = mm-1)/mm+1)
;
;       Testing shows that this function has an error of less than 3.75
;       single precision ulps.
;
;       input      mm0.lo   argument x
;                  mm1.lo   argument y
;       output     mm0.lo   result y*log2(x)
;       destroys   mm0, mm2, mm3, mm4, mm5, mm7

        movd        mm5, [mant] ; mask for mantissa                  
        movq        mm4, mm0    ; save x                            
        movd        mm2, [expo] ; mask for exponent      
        pand        mm0, mm5    ; extract mantissa of x => m 
        movd        mm3, [ones] ; 1.0                            
        pand        mm4, mm2    ; extract biased exponent of x => e 
        por         mm0, mm3    ; floatmm)                            
        movd        mm3, [rt2]  ; sqrt(2)                             
        psrld       mm4, 23     ; biased exponent e               
        movq        mm2, mm0    ; save m                    
        pxor        mm5, mm5    ; create 0                     
        pcmpgtd     mm0, mm3    ; m > sqrt(2) ? 0xFFFFFFFF : 0        
        pcmpeqd     mm5, mm4    ; sel = (e == 0) ? 0xFFFFFFFFL : 0
        movd        mm3, [edec] ; 0x0080000                         
        psubd       mm4, mm0    ; increment e if m > sqrt(2)          
        pand        mm0, mm3    ; m > sqrt(2) ? 0x00800000 : 0
        movd        mm3, [bias] ; 127                             
        psubd       mm2, mm0    ; if m > sqrt(2),  m = m/2      
        psubd       mm4, mm3    ; true exponent = i                  
        movd        mm3, [ones] ; 1.0                                
        movq        mm0, mm2    ; save m                             
        pfadd       mm2, mm3    ; m + 1                               
        pfsub       mm0, mm3    ; m - 1                          
        movd        mm7, [c2]   ; c2             
        pfrcp       mm3, mm2    ; approx 1/mm+1)                    
        pfrcpit1    mm2, mm3    ; refine 1/mm+1) 
        pi2fd       mm4, mm4    ; float(i)
        pfrcpit2    mm2, mm3    ; 1/mm+1)        
        pfmul       mm0, mm2    ; z=mm-1)/mm+1)
        movq        mm2, mm0    ; save z                            
        pfmul       mm0, mm0    ; z^2                             
        movq        mm3, mm0    ; save z^2                             
        pfmul       mm0, mm7    ; c2 * z^2                          
        movd        mm7, [tl2e] ; 2*log2(e)       
        pfmul       mm2, mm7    ; z * 2 * log2(e)
        movd        mm7, [c1]   ; c1        
        pfadd       mm0, mm7    ; c2 * z^2 + c1        
        movd        mm7, [c0]   ; c0               
        pfmul       mm0, mm3    ; (c2 * z^2 + c1) * z^2
        pfmul       mm3, mm2    ; z^3 * 2 * log2(e)       
        pfadd       mm0, mm7    ; px = (c2 * z^2 + c1) * z^2 + c0
        movd        mm7, [maxn] ; maxn (largest normal)             
        pfmul       mm3, mm0    ; px*z^3*2*log2(e)                 
        movq        mm0, mm5    ; sel                               
        pfadd       mm2, mm3    ; px*z^3*2*log2(e)+z*2*log2(e)         
        pand        mm5, mm7    ; select largest negative normal if e = 0
        pfadd       mm2, mm4    ; log2(x)=px*z^3*2*log2(e)+z*2*log2(e)+i 
        pandn       mm0, mm2    ; select regular result if e != 0
        pfmul       mm0, mm1    ; r = y * log2(x)                     
        por         mm0, mm5    ; mux in either normal or special result 

;       Compute 2^r using MMX and 3DNow! instructions. Scalar version.
;
;       If the input has an exponent of 0xFF, the result of this routine
;       is undefined. Inputs with an exponent of 0 are treated as true
;       zeroes and return a function value of 1. If the input is < -126
;       the result is flushed to zero, as 2^-126 is the smallest normal
;       SP number. If the input is >= 128, the result is clamped to
;       max_norm.
;
;       Testing shows that this function has an error of less than 4 SP
;       ulps, meaning that the accuracy is 22 bits or better.
;
;       2^r = 2^(i+x), where i = trunc(x), and -1 <= x <= 1. Then, 2^r =
;       2^i * 2^x = 2^i * ((2^x-1)+1). 2^x-1 is approximated by a Pade
;       type rational minimax approximation.
;
;       input      mm0.lo   argument r
;       output     mm0.lo   result 2^r
;       destroys   mm0, mm1, mm2, mm3, mm4, mm5, mm7

        pf2id     mm4, mm0   ; i = trunc(r)   
        movq      mm5, mm0   ; r                
        pi2fd     mm1, mm4   ; float(trunc(r))   
        movd      mm3, [q1]  ; q1             
        pfsub     mm0, mm1   ; x = frac(r)    
        movq      mm1, mm0   ; save x           
        pfmul     mm0, mm0   ; compute x^2       
        movd      mm7, [p1]  ; p1             
        movq      mm2, mm0   ; save x^2       
        pfadd     mm0, mm3   ; x^2 + q1       
        movq      mm3, mm2   ; save x^2        
        pfmul     mm2, mm7   ; p1 * x^2        
        movd      mm7, [p0]  ; p0             
        pfmul     mm0, mm3   ; (x^2 + q1) * x^2
        movd      mm3, [q0]  ; q0              
        pfadd     mm2, mm7   ; p1 * x^2 + p0   
        movd      mm7, [two] ; 2.0             
        pfadd     mm0, mm3   ; qx = (x^2 + q1) * x^2 + q0                    
        pfmul     mm2, mm1   ; px = (p1 * x^2 + p0) * x           
        pfsub     mm0, mm2   ; qx - px         
        pfmul     mm2, mm7   ; 2*px              
        pfrcp     mm1, mm0   ; approx 1/(qx-px)
        movd      mm7, [ones]; 1.0                
        pfrcpit1  mm0, mm1   ; refine 1/(qx-px)
        pslld     mm4, 23    ; i = i << 23 (shift into exponent)
        pfrcpit2  mm0, mm1   ; 1/(qx-px)                            
        movd      mm3, [mine]; mine                                 
        pfmul     mm2, mm0   ; 2xm1 = 2*px / (qx - px)
        movd      mm1, [maxe]; maxe                              
        pfadd     mm2, mm7   ; 2^x                                
        movq      mm0, mm5   ; r
        pfcmpge   mm5, mm3   ; mask = (r >= -126) ? 0xFFFFFFFFL : 0
        pfcmpge   mm0, mm1   ; sel = (r >= 128) ? 0xFFFFFFFFL : 0
        movd      mm1, [fltmax] ; FLT_MAX
        paddd     mm4, mm2   ; 2^x * 2^i = 2^(x+i)
        pand      mm1, mm0   ; if (r >= 128) select max norm
        pand      mm5, mm4   ; flush normal result to 0 if r < -126
        pandn     mm0, mm5   ; if (r < 128) select normal result
        por       mm0, mm1   ; mux together result                 
     
;       Here we basically have pow(x,y). But in case y is an odd integer
;       and x is negative, the sign bit is wrong. Correct it here using
;       a mask we constructed when we entered pow() based on the values
;       of x and y.
                                
        pxor      mm0, mm6   ; invert sign if ((x<0) && (y&1)) ==> pow(x,y)

		; restore all register except 0 and 1
		movq		mm2, [tmpMM2]
		movq		mm3, [tmpMM3]
		movq		mm4, [tmpMM4]
		movq		mm5, [tmpMM5]
		movq		mm6, [tmpMM6]
		movq		mm7, [tmpMM7]

        ret
_k3dpow_amd  ENDP






;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
COMMENT~

This code implements the power function using 3DNow instructions.  It 
calculates 2 powers at once in about 60 clocks, both with the same exponent.
The hope is that the exponent won't change very often.  

The only externally linkable function is UpdateK3dPowerFunctionPtr, which 
returns a pointer to another function type K3dPowerFunctionType.
UpdateK3dPowerFunctionPtr() must be called before calling the power function.  
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
typedef void K3dPowerFunctionType();
extern "C" K3dPowerFunctionType* UpdateK3dPowerFunctionPtr(float exponent);
K3dPowerFunctionType* K3dPowerFunction;


	//////////// In UpdateRenderingContext()
	// update the pointer whenever it changes
	if (newExponent)
		K3dPowerFunction = UpdateK3dPowerFunctionPtr(exponent);


	//////////// In the lighting code :
	// load base in mmBase (mm0), result is returned in mmBase
	// mmTmp1 and mmTmp2 are modified (mm1 & mm2)
	float *base, *result;
	_asm movd  mm0, dword ptr [base + offset]	; load base values
	K3dPowerFunction();
	_asm movd  oword ptr [result + offset], mm0	; get results
	_asm femms


	//////////// In UpdateRenderingContext()
	// update the pointer whenever it changes
	if (newExponent)
		K3dPowerFunction = UpdateK3dPowerFunctionPtr(exponent, *K3dPowerStuct);


	//////////// In the lighting code :
	// load base in mmBase (mm0), result is returned in mmBase
	// mmTmp1 and mmTmp2 are modified (mm1 & mm2)


~ ;END COMMENT
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

USE_SQRT_INTP_HACK	EQU 1			; "Kentucky windage"

;--------------- START OF CODE FOR TESTING THIS FUNCTION ---------------
ifdef TEST_KNI_POW_FUNCTION

;******************************************************************************
; Since I don't want to load/unload the K3d registers in the C code we'll
; through an extra call overhead and I'll do this in assembly
;
; Routine:  float Asm_K3dPowFunction(float);
;
;******************************************************************************
PUBLIC _Asm_K3dPowFunction
_Asm_K3dPowFunction	PROC
	movd	mmBase, dword ptr [esp+4]
	sub		esp, 4
	call	dword ptr [lpOSPowFunction]
	movd	[esp], mmBase
	femms
	fld		dword ptr [esp]
	pop		eax
	ret
_Asm_K3dPowFunction	ENDP

;******************************************************************************
; Routine:  void Asm_UpdateK3dPowFunctionPtr(void* lpFnctPtr);
;
;******************************************************************************
PUBLIC _Asm_UpdateK3dPowFunctionPtr
_Asm_UpdateK3dPowFunctionPtr	PROC
	mov		eax, dword ptr [esp+4]
	mov		dword ptr [lpOSPowFunction], eax
	ret
_Asm_UpdateK3dPowFunctionPtr	ENDP

endif ;TEST_KNI_POW_FUNCTION
;--------------- END OF CODE FOR TESTING THIS FUNCTION ---------------



;*******************************************************************************************
; Routine:  void* UpdateK3dPowerFunctionPtr(float power)
;
; Comment:  Called when power changes, updates pointers in the interpolate 
;			function (if needed) and returns a pointer to the correct 
;			K3dPowFunction
;*******************************************************************************************
INTERPOLATE_OVER_25 = 0

ALIGN   16
PUBLIC _UpdateK3dPowerFunctionPtr
_UpdateK3dPowerFunctionPtr	PROC
retAddr			= 4 + 0		; 1*4 for the push's
power			= 4 + 4

	push		edx
	movd		mm0, dword ptr [esp+power]
	movd		mm1, [mabs]
	movd		mm4, [flt4]

	pand		mm1, mm0						; |power|
	movq		mm0, mm1
	pf2id		mm1, mm1						; int |power|
	movd		edx, mm1
	
	; anything over 100 we clamp to 100
	cmp			edx, 100
	jg			MaxFunction

Boundary1_1:					; look for integers
	pi2fd		mm1, mm1						; int |power|
	movq		mm2, mm0
	pcmpeqd		mm2, mm1
	movd		eax, mm2
	cmp			eax, 0
	je			Boundary1_1_NonInteger
	mov			eax, dword ptr [LookupTable_K3dPowFunction + edx*4]
	jmp			CheckForNegative

Boundary1_1_NonInteger:			; not an even integer
	cmp			edx, 25
	jl			Boundary1_4

Boundary1_1_Interpolate:		; interpolate between integers
if INTERPOLATE_OVER_25
	mov			eax, offset K3dPowFunctionInterpolateLarge
	jmp			GenerateInterpolationFactors
else
	mov			eax, dword ptr [LookupTable_K3dPowFunction + edx*4]
	jmp			CheckForNegative
endif

Boundary1_4:					; do this in int/4 increments
	pfmul		mm0, mm4
	pf2id		mm1, mm0						; int |power*4|
	movd		edx, mm1

	; check if power is an integer
	pi2fd		mm1, mm1						; int |power|
	movq		mm2, mm0
	pcmpeqd		mm2, mm1
	movd		eax, mm2
	cmp			eax, 0
	je			Boundary1_4_NonInteger

	mov			edx, dword ptr [LookupTable_K3dPowFunction + edx*4]
	mov			dword ptr [lpK3dPowerFunctionIntpLow], edx
	mov			eax, offset K3dPowFunctionPowerMedium
	jmp			CheckForNegative

Boundary1_4_NonInteger:			; not an even integer/4
	cmp			edx, 25			; over 1/(1/4) (6.25)
	jl			Boundary1_16

Boundary1_4_Interpolate:		; interpolate between integers/4
	mov			eax, offset K3dPowFunctionInterpolateMedium
	jmp			GenerateInterpolationFactors


Boundary1_16:					; do this in int/16 increments
	pfmul		mm0, mm4
	pf2id		mm1, mm0						; int |power*4|
	movd		edx, mm1

	; check if power is an integer
	pi2fd		mm1, mm1						; int |power|
	movq		mm2, mm0
	pcmpeqd		mm2, mm1
	movd		eax, mm2
	cmp			eax, 0
	je			Boundary1_16_NonInteger

	mov			edx, dword ptr [LookupTable_K3dPowFunction + edx*4]
	mov			dword ptr [lpK3dPowerFunctionIntpLow], edx
	mov			eax, offset K3dPowFunctionPowerSmall
	jmp			CheckForNegative

Boundary1_16_NonInteger:			; not an even integer/16
	; check if the number is < 1/16th, is so we must call a special function
	cmp			edx, 1
	jb			Under_1_16
	mov			eax, offset K3dPowFunctionInterpolateSmall
	jmp			GenerateInterpolationFactors

Under_1_16:
	; generate interpolation factors    hi = power - (int)power,  low = 1.0 - hi
	pfsub		mm0, mm1
IF USE_SQRT_INTP_HACK
	pfrsqrt		mm0, mm0
	pfrcp		mm0, mm0
ENDIF
	movd		mm1, dword ptr [ones]
	pfsub		mm1, mm0
	punpckldq	mm0, mm0				; broadcast
	punpckldq	mm1, mm1				; broadcast
	movq		qword ptr [fIntpFactorHi], mm0
	movq		qword ptr [fIntpFactorLow], mm1
	mov			eax, offset K3dPowFunctionInterpolate_1_16
	jmp			Done


MaxFunction:	; clamp to 100
	mov			eax, offset K3dPowFunction100
	jmp			CheckForNegative


GenerateInterpolationFactors:
	; generate interpolation factors    hi = power - (int)power,  low = 1.0 - hi
	pfsub		mm0, mm1
IF USE_SQRT_INTP_HACK
	pfrsqrt		mm0, mm0
	pfrcp		mm0, mm0
ENDIF
	movd		mm1, dword ptr [ones]
	pfsub		mm1, mm0
	punpckldq	mm0, mm0				; broadcast
	punpckldq	mm1, mm1				; broadcast
	mov			edx, dword ptr [LookupTable_K3dPowFunction + edx*4]
	mov			dword ptr [lpK3dPowerFunctionIntpLow], edx
	movq		qword ptr [fIntpFactorHi], mm0
	movq		qword ptr [fIntpFactorLow], mm1
	jmp			CheckForNegative


CheckForNegative:
	test		dword ptr [esp+power], 1 SHL 31
	je			Done

	mov			dword ptr [lpK3dPowerFunctionNegative], eax
	mov			eax, offset K3dPowFunctionNegative

Done:
	pop		edx
	femms
	ret

_UpdateK3dPowerFunctionPtr	ENDP


;===========================================================================================
;===========================================================================================
; PRIVATE FUNCTIONS
;===========================================================================================
;===========================================================================================

;*******************************************************************************************
; Routine:  void K3dPowFunctionPowerMedium ()
;
; Comment:  Handles cases of small exponents on even integers of 1/4th.  For instance 1 1/4
;			we do x^5 and then do x^-4 by doing do 2 square-roots
;*******************************************************************************************
ALIGN   16
K3dPowFunctionPowerMedium	PROC	PRIVATE
	mov			eax, dword ptr [lpK3dPowerFunctionIntpLow]
	call		eax
	pfrsqrt		mmBase, mmBase				; 1/x^-2
	pfrsqrt		mmBase, mmBase				; 1/x^-4
	ret
K3dPowFunctionPowerMedium	ENDP

;*******************************************************************************************
; Routine:  void K3dPowFunctionPowerSmall ()
;
; Comment:  Handles cases of small exponents on even integers of 1/16th.  For instance 1 1/16
;			we do x^17 and then do x^-16 by doing do 4 square-roots
;*******************************************************************************************
ALIGN   16
K3dPowFunctionPowerSmall	PROC	PRIVATE
	mov			eax, dword ptr [lpK3dPowerFunctionIntpLow]
	call		eax
	pfrsqrt		mmBase, mmBase				; 1/x^-2
	pfrsqrt		mmBase, mmBase				; 1/x^-4
	pfrsqrt		mmBase, mmBase				; 1/x^-8
	pfrsqrt		mmBase, mmBase				; 1/x^-16
	ret
K3dPowFunctionPowerSmall	ENDP

;*******************************************************************************************
; Routine:  void K3dPowFunctionInterpolateLarge ()
;
; Comment:  Interpolates power between (int) power and (int) power+1 when power is positive
;			Power is already known, base is in mmBase, and is returned in mmBase
;			x1 = pow(x,(int)y)		f2 = y - (int)y
;			x2 = x1 * x2			f1 = 1.0 - f1
;			x = (x1 * f1) + (x2 * f2)
;			This only works for power >= +1.0
;*******************************************************************************************
ALIGN   16
K3dPowFunctionInterpolateLarge	PROC	PRIVATE
	mov			eax, dword ptr [lpK3dPowerFunctionIntpLow]
	movq		mmTmp2, qword ptr [fIntpFactorHi]
	pfmul		mmTmp2, mmBase
	call		eax
	pfmul		mmTmp2, mmBase
	pfmul		mmBase, qword ptr [fIntpFactorLow]
	pfadd		mmBase, mmTmp2
	ret
K3dPowFunctionInterpolateLarge	ENDP

;*******************************************************************************************
; Routine:  void K3dPowFunctionInterpolateMedium ()
;
; Comment:  Similar to K3dPowFunctionInterpolateLarge() except the values are interpolated
;			on 1/4th integers.  
;*******************************************************************************************
ALIGN   16
K3dPowFunctionInterpolateMedium	PROC	PRIVATE
	mov			eax, dword ptr [lpK3dPowerFunctionIntpLow]
	movq		mmTmp2, qword ptr [fIntpFactorHi]
	pfmul		mmTmp2, mmBase
	call		eax
	pfmul		mmTmp2, mmBase
	pfmul		mmBase, qword ptr [fIntpFactorLow]
	pfadd		mmBase, mmTmp2
	pfrsqrt		mmBase, mmBase				; 1/x^-2
	pfrsqrt		mmBase, mmBase				; 1/x^-4
	ret
K3dPowFunctionInterpolateMedium	ENDP

;*******************************************************************************************
; Routine:  void K3dPowFunctionInterpolateSmall ()
;
; Comment:  Similar to K3dPowFunctionInterpolateLarge() except the values are interpolated
;			on 1/16th integers.  So for example the number 1.001 we interpolate between 
;			1 1/2 and 1 7/16.  What we really do in this case we interpolate between x^24 and 
;			x^25, and then take x^-16 by doing do 4 square-roots.
;*******************************************************************************************
ALIGN   16
K3dPowFunctionInterpolateSmall	PROC	PRIVATE
	mov			eax, dword ptr [lpK3dPowerFunctionIntpLow]
	movq		mmTmp2, qword ptr [fIntpFactorHi]
	pfmul		mmTmp2, mmBase
	call		eax
	pfmul		mmTmp2, mmBase
	pfmul		mmBase, qword ptr [fIntpFactorLow]
	pfadd		mmBase, mmTmp2
	pfrsqrt		mmBase, mmBase				; 1/x^-2
	pfrsqrt		mmBase, mmBase				; 1/x^-4
	pfrsqrt		mmBase, mmBase				; 1/x^-8
	pfrsqrt		mmBase, mmBase				; 1/x^-16
	ret
K3dPowFunctionInterpolateSmall	ENDP

;*******************************************************************************************
; Routine:  void K3dPowFunctionInterpolate_1_16 ()
;
; Comment:  Special function to interpolate between 0 and 1/16.  Required because x^0 = 1 and
;			x^1 = x.  The problem is the other interpolations call x^y for the lower value and
;			then use this value *x for the upper value.  
;*******************************************************************************************
K3dPowFunctionInterpolate_1_16	PROC	PRIVATE
	; build mask for any zero
	pxor		mmTmp1, mmTmp1					;	0	0	0	0
	pcmpeqd		mmTmp1, mmBase					;  (x != 0) ? 0 : -1

	pfmul		mmBase, qword ptr [fIntpFactorHi]	; x^1 = x so x^1 * hi = x * hi
	pfadd		mmBase, qword ptr [fIntpFactorLow]	; x^0 = 1 so x^0 * low = low
	pfrsqrt		mmBase, mmBase				; 1/x^-2
	pcmpeqd		mmTmp2, mmTmp2					; fffffffff ffffffff
	pfrsqrt		mmBase, mmBase				; 1/x^-4
	pfrsqrt		mmBase, mmBase				; 1/x^-8
	pxor		mmTmp1, mmTmp2					;  (x != 0) ? -1 : 0  
	pfrsqrt		mmBase, mmBase				; 1/x^-16

	; mask any zero base values back to zero
	pand		mmBase, mmTmp1
	ret
K3dPowFunctionInterpolate_1_16	ENDP



;*******************************************************************************************
; Routine:  void K3dPowFunctionNegative ()
;
; Comment:  Handles cases of x^-y 
;			Since x^-y = 1/x^+y we just do a reciprocal when finished
;*******************************************************************************************
ALIGN   16
K3dPowFunctionNegative	PROC	PRIVATE
	mov			eax, dword ptr [lpK3dPowerFunctionNegative]
	call		eax
	pfrcp		mmBase, mmBase
	ret
K3dPowFunctionNegative	ENDP





;*******************************************************************************************
;*******************************************************************************************
;*******************************************************************************************
; Routines:	void K3dPowFunctionN ()
;
; Comments:	Takes register mmBase to the "N" power, and returns the value in mmBase
;			Use "ALIGN 8" rather than 16 because these are all 3-byte instructions
;			so the decoder will, worst case, decode two full instructions on the 
;			first clock, which is plenty for SSE instructions.  16-byte alignment
;			is unnecessary.
;*******************************************************************************************
ALIGN   8
K3dPowFunction0	PROC	PRIVATE
	movd		mmBase, dword ptr [ones]	; anything to the zero power is one
	ret
K3dPowFunction0	ENDP

ALIGN   8
K3dPowFunction1	PROC	PRIVATE
	nop										; k7 doesn't like branches to nop's
	ret										; x^1 = x
K3dPowFunction1	ENDP

ALIGN   8
K3dPowFunction2	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	ret
K3dPowFunction2	ENDP

ALIGN   8
K3dPowFunction3	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmTmp1				; x^3
	ret
K3dPowFunction3	ENDP

ALIGN   8
K3dPowFunction4	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	ret
K3dPowFunction4	ENDP

ALIGN   8
K3dPowFunction5	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmTmp1				; x^5
	ret
K3dPowFunction5	ENDP

ALIGN   8
K3dPowFunction6	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmTmp1				; x^3
	pfmul		mmBase, mmBase				; x^6
	ret
K3dPowFunction6	ENDP

ALIGN   8
K3dPowFunction7	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmTmp1				; x^7
	ret
K3dPowFunction7	ENDP

ALIGN   8
K3dPowFunction8	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	ret
K3dPowFunction8	ENDP

ALIGN   8
K3dPowFunction9	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmTmp1				; x^9
	ret
K3dPowFunction9	ENDP

ALIGN   8
K3dPowFunction10	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmTmp1				; x^10
	ret
K3dPowFunction10	ENDP

ALIGN   8
K3dPowFunction11	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmTmp1				; x^11
	ret
K3dPowFunction11	ENDP

ALIGN   8
K3dPowFunction12	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	movq		mmTmp1, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmTmp1				; x^12
	ret
K3dPowFunction12	ENDP

ALIGN   8
K3dPowFunction13	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^5
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmTmp1				; x^13
	ret
K3dPowFunction13	ENDP

ALIGN   8
K3dPowFunction14	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^6
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmTmp1				; x^14
	ret
K3dPowFunction14	ENDP

ALIGN   8
K3dPowFunction15	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^7
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmTmp1				; x^15
	ret
K3dPowFunction15	ENDP

ALIGN   8
K3dPowFunction16	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	ret
K3dPowFunction16	ENDP

ALIGN   8
K3dPowFunction17	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmTmp1				; x^17
	ret
K3dPowFunction17	ENDP

ALIGN   8
K3dPowFunction18	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmTmp1				; x^18
	ret
K3dPowFunction18	ENDP

ALIGN   8
K3dPowFunction19	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmTmp1				; x^19
	ret
K3dPowFunction19	ENDP

ALIGN   8
K3dPowFunction20	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	movq		mmTmp1, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmTmp1				; x^20
	ret
K3dPowFunction20	ENDP

ALIGN   8
K3dPowFunction21	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^5
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmTmp1				;  x^21
	ret
K3dPowFunction21	ENDP

ALIGN   8
K3dPowFunction22	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^6
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmTmp1				; x^22
	ret
K3dPowFunction22	ENDP

ALIGN   8
K3dPowFunction23	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^7
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmTmp1				; x^23
	ret
K3dPowFunction23	ENDP

ALIGN   8
K3dPowFunction24	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	movq		mmTmp1, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmTmp1				; x^24
	ret
K3dPowFunction24	ENDP

ALIGN   8
K3dPowFunction25	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^9
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmTmp1				; x^25
	ret
K3dPowFunction25	ENDP

ALIGN   8
K3dPowFunction26	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^10
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmTmp1				; x^26
	ret
K3dPowFunction26	ENDP

ALIGN   8
K3dPowFunction27	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^11
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmTmp1				; x^27
	ret
K3dPowFunction27	ENDP

ALIGN   8
K3dPowFunction28	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	movq		mmTmp1, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^12
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmTmp1				; x^28
	ret
K3dPowFunction28	ENDP

ALIGN   8
K3dPowFunction29	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^5
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^13
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmTmp1				; x^29
	ret
K3dPowFunction29	ENDP

ALIGN   8
K3dPowFunction30	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^6
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^14
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmTmp1				; x^30
	ret
K3dPowFunction30	ENDP

ALIGN   8
K3dPowFunction31	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^7
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^15
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmTmp1				; x^31
	ret
K3dPowFunction31	ENDP

ALIGN   8
K3dPowFunction32	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	ret
K3dPowFunction32	ENDP

ALIGN   8
K3dPowFunction33	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^33
	ret
K3dPowFunction33	ENDP

ALIGN   8
K3dPowFunction34	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^34
	ret
K3dPowFunction34	ENDP

ALIGN   8
K3dPowFunction35	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^35
	ret
K3dPowFunction35	ENDP

ALIGN   8
K3dPowFunction36	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	movq		mmTmp1, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^36
	ret
K3dPowFunction36	ENDP

ALIGN   8
K3dPowFunction37	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^5
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^37
	ret
K3dPowFunction37	ENDP

ALIGN   8
K3dPowFunction38	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^6
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^38
	ret
K3dPowFunction38	ENDP

ALIGN   8
K3dPowFunction39	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^7
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^39
	ret
K3dPowFunction39	ENDP

ALIGN   8
K3dPowFunction40	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	movq		mmTmp1, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^40
	ret
K3dPowFunction40	ENDP

ALIGN   8
K3dPowFunction41	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^9
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^41
	ret
K3dPowFunction41	ENDP

ALIGN   8
K3dPowFunction42	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^10
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^42
	ret
K3dPowFunction42	ENDP

ALIGN   8
K3dPowFunction43	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^11
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^43
	ret
K3dPowFunction43	ENDP

ALIGN   8
K3dPowFunction44	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	movq		mmTmp1, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^12
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^44
	ret
K3dPowFunction44	ENDP

ALIGN   8
K3dPowFunction45	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^5
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^13
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^45
	ret
K3dPowFunction45	ENDP

ALIGN   8
K3dPowFunction46	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^6
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^14
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^46
	ret
K3dPowFunction46	ENDP

ALIGN   8
K3dPowFunction47	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^7
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^15
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^47
	ret
K3dPowFunction47	ENDP

ALIGN   8
K3dPowFunction48	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	movq		mmTmp1, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^48
	ret
K3dPowFunction48	ENDP

ALIGN   8
K3dPowFunction49	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^17
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^49
	ret
K3dPowFunction49	ENDP

ALIGN   8
K3dPowFunction50	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^18
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^50
	ret
K3dPowFunction50	ENDP

ALIGN   8
K3dPowFunction51	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^19
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^51
	ret
K3dPowFunction51	ENDP

ALIGN   8
K3dPowFunction52	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	movq		mmTmp1, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^20
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^52
	ret
K3dPowFunction52	ENDP

ALIGN   8
K3dPowFunction53	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^5
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^21
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^53
	ret
K3dPowFunction53	ENDP

ALIGN   8
K3dPowFunction54	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^6
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^22
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^54
	ret
K3dPowFunction54	ENDP

ALIGN   8
K3dPowFunction55	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^7
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^23
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^55
	ret
K3dPowFunction55	ENDP

ALIGN   8
K3dPowFunction56	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	movq		mmTmp1, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^24
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^56
	ret
K3dPowFunction56	ENDP

ALIGN   8
K3dPowFunction57	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^9
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^25
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^57
	ret
K3dPowFunction57	ENDP

ALIGN   8
K3dPowFunction58	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^10
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^26
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^58
	ret
K3dPowFunction58	ENDP

ALIGN   8
K3dPowFunction59	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^11
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^27
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^59
	ret
K3dPowFunction59	ENDP

ALIGN   8
K3dPowFunction60	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	movq		mmTmp1, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^12
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^28
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^60
	ret
K3dPowFunction60	ENDP

ALIGN   8
K3dPowFunction61	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^5
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^13
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^29
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^61
	ret
K3dPowFunction61	ENDP

ALIGN   8
K3dPowFunction62	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^6
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^14
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^30
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^62
	ret
K3dPowFunction62	ENDP

ALIGN   8
K3dPowFunction63	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^7
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^15
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^31
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmTmp1				; x^63
	ret
K3dPowFunction63	ENDP

ALIGN   8
K3dPowFunction64	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	ret
K3dPowFunction64	ENDP

ALIGN   8
K3dPowFunction65	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^65
	ret
K3dPowFunction65	ENDP

ALIGN   8
K3dPowFunction66	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^66
	ret
K3dPowFunction66	ENDP

ALIGN   8
K3dPowFunction67	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^67
	ret
K3dPowFunction67	ENDP

ALIGN   8
K3dPowFunction68	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	movq		mmTmp1, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^68
	ret
K3dPowFunction68	ENDP

ALIGN   8
K3dPowFunction69	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^5
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^69
	ret
K3dPowFunction69	ENDP

ALIGN   8
K3dPowFunction70	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^6
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^70
	ret
K3dPowFunction70	ENDP

ALIGN   8
K3dPowFunction71	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^7
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^71
	ret
K3dPowFunction71	ENDP

ALIGN   8
K3dPowFunction72	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	movq		mmTmp1, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^72
	ret
K3dPowFunction72	ENDP

ALIGN   8
K3dPowFunction73	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^9
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^73
	ret
K3dPowFunction73	ENDP

ALIGN   8
K3dPowFunction74	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^10
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^74
	ret
K3dPowFunction74	ENDP

ALIGN   8
K3dPowFunction75	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^11
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^75
	ret
K3dPowFunction75	ENDP

ALIGN   8
K3dPowFunction76	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	movq		mmTmp1, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^12
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^76
	ret
K3dPowFunction76	ENDP

ALIGN   8
K3dPowFunction77	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^5
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^13
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^77
	ret
K3dPowFunction77	ENDP

ALIGN   8
K3dPowFunction78	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^6
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^14
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^78
	ret
K3dPowFunction78	ENDP

ALIGN   8
K3dPowFunction79	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^7
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^15
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^79
	ret
K3dPowFunction79	ENDP

ALIGN   8
K3dPowFunction80	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	movq		mmTmp1, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^80
	ret
K3dPowFunction80	ENDP

ALIGN   8
K3dPowFunction81	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^17
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^81
	ret
K3dPowFunction81	ENDP

ALIGN   8
K3dPowFunction82	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^18
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^82
	ret
K3dPowFunction82	ENDP

ALIGN   8
K3dPowFunction83	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^19
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^83
	ret
K3dPowFunction83	ENDP

ALIGN   8
K3dPowFunction84	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	movq		mmTmp1, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^20
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^84
	ret
K3dPowFunction84	ENDP

ALIGN   8
K3dPowFunction85	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^5
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^21
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^85
	ret
K3dPowFunction85	ENDP

ALIGN   8
K3dPowFunction86	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^6
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^22
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^86
	ret
K3dPowFunction86	ENDP

ALIGN   8
K3dPowFunction87	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^7
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^23
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^87
	ret
K3dPowFunction87	ENDP

ALIGN   8
K3dPowFunction88	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	movq		mmTmp1, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^24
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^88
	ret
K3dPowFunction88	ENDP

ALIGN   8
K3dPowFunction89	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^9
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^25
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^89
	ret
K3dPowFunction89	ENDP

ALIGN   8
K3dPowFunction90	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^10
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^26
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^90
	ret
K3dPowFunction90	ENDP

ALIGN   8
K3dPowFunction91	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^11
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^27
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^91
	ret
K3dPowFunction91	ENDP

ALIGN   8
K3dPowFunction92	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	movq		mmTmp1, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^12
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^28
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^92
	ret
K3dPowFunction92	ENDP

ALIGN   8
K3dPowFunction93	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^5
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^13
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^29
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^93
	ret
K3dPowFunction93	ENDP

ALIGN   8
K3dPowFunction94	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^6
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^14
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^30
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^94
	ret
K3dPowFunction94	ENDP

ALIGN   8
K3dPowFunction95	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmTmp1, mmBase				; x^7
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmTmp1, mmBase				; x^15
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmTmp1, mmBase				; x^31
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^95
	ret
K3dPowFunction95	ENDP

ALIGN   8
K3dPowFunction96	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	movq		mmTmp1, mmBase				; x^32
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^96
	ret
K3dPowFunction96	ENDP


ALIGN   8
K3dPowFunction97	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmTmp1, mmBase				; x^33
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^97
	ret
K3dPowFunction97	ENDP


ALIGN   8
K3dPowFunction98	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	movq		mmTmp1, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmTmp1, mmBase				; x^34
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^98
	ret
K3dPowFunction98	ENDP

ALIGN   8
K3dPowFunction99	PROC	PRIVATE
	movq		mmTmp1, mmBase
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmTmp1, mmBase				; x^3
	pfmul		mmBase, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmTmp1, mmBase				; x^35
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^99
	ret
K3dPowFunction99	ENDP


ALIGN   8
K3dPowFunction100	PROC	PRIVATE
	pfmul		mmBase, mmBase				; x^2
	pfmul		mmBase, mmBase				; x^4
	movq		mmTmp1, mmBase				; x^4
	pfmul		mmBase, mmBase				; x^8
	pfmul		mmBase, mmBase				; x^16
	pfmul		mmBase, mmBase				; x^32
	pfmul		mmTmp1, mmBase				; x^36
	pfmul		mmBase, mmBase				; x^64
	pfmul		mmBase, mmTmp1				; x^100
	ret
K3dPowFunction100	ENDP




;*******************************************************************************************
; Routine:  void _MatrixProduct_K3d_Asm( D3DMATRIX *a, D3DMATRIX *b, D3DMATRIX* dst)
;
;  This is about 86 clocks on a K6 vs ~450 for the C version, I didn't measure the Athlon
;
;	dst->_11 = a->_11*b->_11 + a->_12*b->_21 + a->_13*b->_31 + a->_14*b->_41;   
;	dst->_12 = a->_11*b->_12 + a->_12*b->_22 + a->_13*b->_32 + a->_14*b->_42;   
;	dst->_13 = a->_11*b->_13 + a->_12*b->_23 + a->_13*b->_33 + a->_14*b->_43;   
;	dst->_14 = a->_11*b->_14 + a->_12*b->_24 + a->_13*b->_34 + a->_14*b->_44;   
;
;	dst->_21 = a->_21*b->_11 + a->_22*b->_21 + a->_23*b->_31 + a->_24*b->_41;   
;	dst->_22 = a->_21*b->_12 + a->_22*b->_22 + a->_23*b->_32 + a->_24*b->_42;   
;	dst->_23 = a->_21*b->_13 + a->_22*b->_23 + a->_23*b->_33 + a->_24*b->_43;   
;	dst->_24 = a->_21*b->_14 + a->_22*b->_24 + a->_23*b->_34 + a->_24*b->_44;   
;
;	dst->_31 = a->_31*b->_11 + a->_32*b->_21 + a->_33*b->_31 + a->_34*b->_41;   
;	dst->_32 = a->_31*b->_12 + a->_32*b->_22 + a->_33*b->_32 + a->_34*b->_42;   
;	dst->_33 = a->_31*b->_13 + a->_32*b->_23 + a->_33*b->_33 + a->_34*b->_43;   
;	dst->_34 = a->_31*b->_14 + a->_32*b->_24 + a->_33*b->_34 + a->_34*b->_44;   
;
;	dst->_41 = a->_41*b->_11 + a->_42*b->_21 + a->_43*b->_31 + a->_44*b->_41;   
;	dst->_42 = a->_41*b->_12 + a->_42*b->_22 + a->_43*b->_32 + a->_44*b->_42;   
;	dst->_43 = a->_41*b->_13 + a->_42*b->_23 + a->_43*b->_33 + a->_44*b->_43;   
;	dst->_44 = a->_41*b->_14 + a->_42*b->_24 + a->_43*b->_34 + a->_44*b->_44;
;
;*******************************************************************************************
ALIGN   16
_MatrixProduct_K3d_Asm PROC
	mov			ecx, dword ptr [esp+ 4]	; a
	mov			edx, dword ptr [esp+ 8]	; b
	mov			eax, dword ptr [esp+12]	; dst

	movq		mm0,[ecx+ 0]	; a_21	| a_11
	movq		mm1,[ecx+ 8]	; a_41	| a_31
	movq		mm4,[edx+ 0]	; b_21	| b_11
	punpckhdq   mm2,mm0			; a_21	| XXX
	movq		mm5,[edx+16]	; b_22	| b_12
	punpckhdq   mm3,mm1			; a_41	| XXX
	movq		mm6,[edx+32]	; b_23	| b_13
	punpckldq   mm0,mm0			; a_11	| a_11
	punpckldq   mm1,mm1			; a_31	| a_31
	pfmul       mm4,mm0			; a_11 * b_21 | a_11 * b_11
	punpckhdq   mm2,mm2			; a_21	| a_21
	pfmul       mm0,[edx+ 8]	; a_11 * b_41 | a_11 * b_31
	movq		mm7,[edx+48]	; b_24	| b_14
	pfmul       mm5,mm2			; a_21 * b_22 | a_21 * b_12
	punpckhdq   mm3,mm3			; a_41	| a_41
	pfmul       mm2,[edx+24]	; a_21 * b_42 | a_21 * b_32
	pfmul       mm6,mm1			; a_31 * b_23 | a_31 * b_13 
	pfadd       mm5,mm4			; a_21 * b_22 + a_11 * b_21 | a_21 * b_12 + a_11 * b_11
	pfmul       mm1,[edx+40]	; a_31 * b_43 | a_31 * b_33
	pfadd       mm2,mm0			; a_21 * b_42 + a_11 * b_41 | a_21 * b_32 + a_11 * b_31
	pfmul       mm7,mm3			; a_41 * b_24 | a_41 * b_14 
	pfadd       mm6,mm5			; a_21 * b_22 + a_11 * b_21 + a_31 * b_23 | a_21 * b_12 + a_11 * b_11 + a_31 * b_13
	pfmul       mm3,[edx+56]	; a_41 * b_44 | a_41 * b_34
	pfadd       mm2,mm1			; a_21 * b_42 + a_11 * b_41 + a_31 * b_43 | a_21 * b_32 + a_11 * b_31 + a_31 * b_33 
	pfadd       mm7,mm6			; a_41 * b_24 + a_21 * b_22 + a_11 * b_21 + a_31 * b_23 |  a_41 * b_14 + a_21 * b_12 + a_11 * b_11 + a_31 * b_13
	movq		mm0,[ecx+16]	; a_22	| a_12
	pfadd       mm3,mm2			; a_41 * b_44 + a_21 * b_42 + a_11 * b_41 + a_31 * b_43 | a_41 * b_34 + a_21 * b_32 + a_11 * b_31 + a_31 * b_33 
	movq		mm1,[ecx+24]	; a_42	| a_32
	movq		[eax+ 0],mm7	; r_21	| r_11 
	movq		mm4,[edx+ 0]	; b_21	| b_11
	movq		[eax+ 8],mm3	; r_41	| r_31

	punpckhdq   mm2,mm0			; a_22	| XXX
	movq		mm5,[edx+16]	; b_22	| b_12
	punpckhdq   mm3,mm1			; a_42	| XXX
	movq		mm6,[edx+32]	; b_23	| b_13
	punpckldq   mm0,mm0			; a_12	| a_12
	punpckldq   mm1,mm1			; a_32	| a_32
	pfmul       mm4,mm0			; a_12 * b_21 | a_12 * b_11
	punpckhdq   mm2,mm2			; a_22	| a_22
	pfmul       mm0,[edx+ 8]	; a_12 * b_41 | a_12 * b_31
	movq		mm7,[edx+48]	; b_24	| b_14
	pfmul       mm5,mm2			; a_22 * b_22 | a_22 * b12
	punpckhdq   mm3,mm3			; a_42	| a_42
	pfmul       mm2,[edx+24]	; a_22 * b_42 | a_22 * b_32
	pfmul       mm6,mm1			; a_32 * b_23 | a_32 * b_13
	pfadd       mm5,mm4			; a_12 * b_21 + a_22 * b_22 | a_12 * b_11 + a_22 * b12
	pfmul       mm1,[edx+40]	; a_32 * b_43 | a_32 * b_33
	pfadd       mm2,mm0			; a_12 * b_41 + a_22 * b_42 | a_12 * b_11 + a_22 * b_32
	pfmul       mm7,mm3			; a_42 * b_24 | a_42 * b_14
	pfadd       mm6,mm5			; a_32 * b_23 + a_12 * b_21 + a_22 * b_22 | a_32 * b_13 + a_12 * b_11 + a_22 * b12
	pfmul       mm3,[edx+56]	; a_42 * b_44 | a_42 * b_34
	pfadd       mm2,mm1			; a_32 * b_43 + a_12 * b_41 + a_22 * b_42 | a_32 * b_33 + a_12 * b_11 + a_22 * b_32
	pfadd       mm7,mm6			; a_42 * b_24 + a_32 * b_23 + a_12 * b_21 + a_22 * b_22 | a_42 * b_14 + a_32 * b_13 + a_12 * b_11 + a_22 * b12
	movq		mm0,[ecx+32]	; a_23 | a_13
	pfadd       mm3,mm2			; a_42 * b_44 + a_32 * b_43 + a_12 * b_41 + a_22 * b_42 | a_42 * b_34 + a_32 * b_33 + a_12 * b_11 + a_22 * b_32
	movq		mm1,[ecx+40]	; a_43 | a_33
	movq		[eax+16],mm7	; r_22 | r_12
	movq		mm4,[edx]		; b_21	| b_11
	movq		[eax+24],mm3	; r_42 | r_32

	punpckhdq   mm2,mm0			; a_23 | XXX
	movq		mm5,[edx+16]	; b_22 | b_12
	punpckhdq   mm3,mm1			; a_43 | XXX
	movq		mm6,[edx+32]	; b_23 | b_13
	punpckldq   mm0,mm0			; a_13 | a_13
	punpckldq   mm1,mm1			; a_33 | a_33
	pfmul       mm4,mm0			; a_13 * b_21 | a_13 * b_11
	punpckhdq   mm2,mm2			; a_23 | a_23
	pfmul       mm0,[edx+ 8]	; a_13 * b_41 | a_13 * b_31
	movq		mm7,[edx+48]	; b_24 | b_14
	pfmul       mm5,mm2			; a_23 * b_22 | a_23 * b_12
	punpckhdq   mm3,mm3			; a_43 | a_43
	pfmul       mm2,[edx+24]	; a_23 * b_42 | a_23 * b_32
	pfmul       mm6,mm1			; a_33 * b_23 | a_33 * b_13
	pfadd       mm5,mm4			; a_23 * b_22 + a_13 * b_21 | a_23 * b_12 + a_13 * b_11
	pfmul       mm1,[edx+40]	; a_33 * b_43 | a_33 * b_33 
	pfadd       mm2,mm0			; a_13 * b_41 + a_23 * b_42 | a_13 * b_31 + a_23 * b_32
	pfmul       mm7,mm3			; a_43 * b_24 | a_43 * b_14
	pfadd       mm6,mm5			; a_33 * b_23 + a_23 * b_22 + a_13 * b_21 | a_33 * b_13 + a_23 * b_12 + a_13 * b_11
	pfmul       mm3,[edx+56]	; a_43 * b_44 | a_43 * b_34
	pfadd       mm2,mm1			; a_33 * b_43 * a_13 * b_41 + a_23 * b_42 | a_33 * b_33 + a_13 * b_31 + a_23 * b_32
	pfadd       mm7,mm6			; a_43 * b_24 + a_33 * b_23 + a_23 * b_22 + a_13 * b_21 | a_43 * b_14 + a_33 * b_13 + a_23 * b_12 + a_13 * b_11
	movq		mm0,[ecx+48]	; a_24 | a_14
	pfadd       mm3,mm2			; a_43 * b_44 + a_33 * b_43 * a_13 * b_41 + a_23 * b_42 | a_43 * b_34 + a_33 * b_33 + a_13 * b_31 + a_23 * b_32
	movq		mm1,[ecx+56]	; a_44 | a_34
	movq		[eax+32],mm7	; r_23 | r_13
	movq		mm4,[edx]		; b_21 | b_11
	movq		[eax+40],mm3	; r_43 | r_33

	punpckhdq   mm2,mm0			; a_24 | XXX
	movq		mm5,[edx+16]	; b_22 | b_12
	punpckhdq   mm3,mm1			; a_44 | XXX
	movq		mm6,[edx+32]	; b_23 | b_13
	punpckldq   mm0,mm0			; a_14 | a_14
	punpckldq   mm1,mm1			; a_34 | a_34
	pfmul       mm4,mm0			; a_14 * b_21 | a_14 * b_11
	punpckhdq   mm2,mm2			; a_24 | a_24
	pfmul       mm0,[edx+ 8]	; a_14 * b_41 | a_14 * b_31
	movq		mm7,[edx+48]	; b_24 | b_14
	pfmul       mm5,mm2			; a_24 * b_22 | a_24 * b_12
	punpckhdq   mm3,mm3			; a_44 | a_44
	pfmul       mm2,[edx+24]	; a_24 * b_ 42 | a_24 * b_32
	pfmul       mm6,mm1			; a_34 * b_23 | a_34 * b_13
	pfadd       mm5,mm4			; a_14 * b_21 + a_24 * b_22 | a_14 * b_11 + a_24 * b_12
	pfmul       mm1,[edx+40]	; a_34 * b_43 | a_34 * b_33
	pfadd       mm2,mm0			; a_14 * b_41 + a_24 * b_ 42 | a_14 * b_31 + a_24 * b_32
	pfmul       mm7,mm3			; a_44 * b_24 | a_44 * b_14
	pfadd       mm6,mm5			; a_34 * b_23 + a_14 * b_21 + a_24 * b_22 | a_34 * b_13 + a_14 * b_11 + a_24 * b_12
	pfmul       mm3,[edx+56]	; a_44 * b_44 | a_44 * b_34
	pfadd       mm2,mm1			; a_34 * b_43 + a_14 * b_41 + a_24 * b_ 42 | a_34 * b_33 + a_14 * b_31 + a_24 * b_32
	pfadd       mm7,mm6			; a_44 * b_24 + a_14 * b_23 + a_24 * b_ 42 | a_44 * b_14 + a_14 * b_31 + a_24 * b_32
	pfadd       mm3,mm2			; a_44 * b_44 + a_34 * b_43 + a_14 * b_41 + a_24 * b_42 | a_44 * b_34 + a_34 * b_33 + a_14 * b_31 + a_24 * b_32
	movq		[eax+48],mm7	; r_24 | r_14
	movq		[eax+56],mm3	; r_44 | r_34

	femms
	ret
_MatrixProduct_K3d_Asm	ENDP


_TEXT   ENDS


END
