;**
;** DLLLOADS: Glue between DLLLOAD.LIB and Watcom stack-based libraries.
;**
;** Copyright (c) 1997 Dan Teven.
;**

.386p
.MODEL FLAT

.DATA

PUBLIC	__psp
EXTRN	_psp:WORD

__psp	DW	28h			; not valid until first int386 call!

.CODE

PUBLIC	malloc_, free_, _heapmin_
PUBLIC	int386_, int386x_, exit_
PUBLIC	_access_, _searchenv_, strrchr_, getenv_

EXTRN	malloc:NEAR, free:NEAR, _heapmin:NEAR
EXTRN	int386:NEAR, int386x:NEAR, exit:NEAR
EXTRN	_access:NEAR, _searchenv:NEAR, strrchr:NEAR, getenv:NEAR

arg1	EQU	eax
arg2	EQU	edx
arg3	EQU	ebx
arg4	EQU	ecx

malloc_:	push	ecx		; allowed to alter EAX
		push	edx
		push	arg1
		call	malloc		; may alter EAX, ECX, EDX
		pop	edx
		pop	edx
		pop	ecx
		ret

free_:		push	ecx		; allowed to alter EAX
		push	edx
		push	arg1
		call	free		; may alter EAX, ECX, EDX
		pop	edx
		pop	edx
		pop	ecx
		ret

_heapmin_:	push	eax		; allowed to alter no registers
		push	ecx
		push	edx
		call	_heapmin	; may alter EAX, ECX, EDX
		pop	edx
		pop	ecx
		pop	eax
		ret

int386_:	push	ecx		; allowed to alter EAX, EBX, EDX
		push	arg3
		push	arg2
		push	arg1
		mov	ax, _psp	; copy real _psp value
		mov	__psp, ax
		call	int386		; may alter EAX, ECX, EDX
		lea	esp, [esp+12]
		pop	ecx
		ret

int386x_:	push	arg4		; allowed to alter EAX, EBX, ECX, EDX
		push	arg3
		push	arg2
		push	arg1
		call	int386x		; may alter EAX, ECX, EDX
		lea	esp, [esp+16]
		ret

exit_:		push	arg1
		call	exit		; never returns
		int	3
		hlt

_access_:	push	ecx		; allowed to alter EAX, EDX
		push	arg2
		push	arg1
		call	_access		; may alter EAX, ECX, EDX
		pop	ecx
		pop	ecx
		pop	ecx
		ret

_searchenv_:	push	ecx		; allowed to alter EAX, EBX, EDX
		push	arg3
		push	arg2
		push	arg1
		call	_searchenv	; may alter EAX, ECX, EDX
		lea	esp, [esp+12]
		pop	ecx
		ret

strrchr_:	push	ecx		; allowed to alter EAX, EDX
		push	arg2
		push	arg1
		call	strrchr		; may alter EAX, ECX, EDX
		pop	ecx
		pop	ecx
		pop	ecx
		ret

getenv_:	push	ecx		; allowed to alter EAX
		push	edx
		push	arg1
		call	getenv		; may alter EAX, ECX, EDX
		pop	edx
		pop	edx
		pop	ecx
		ret

END
