;**
;** LDRCALL.ASM: DOS/4GW DLL API and 16-bit support code.
;**
;** Copyright (c) 1996,1997 Dan Teven.
;**

.386p
.MODEL FLAT

.DATA

AliasStackSize		EQU	8192

LoadModule		EQU	0
FreeModule		EQU	1
GetModuleTable		EQU	2
GetModuleName		EQU	3
GetModuleHandle		EQU	4
GetExportedAddress	EQU	5
RelocatePointer		EQU	6
UnrelocatePointer	EQU	7
NumLoaderExports	EQU	8

NullModule		EQU	0FFFFh

PackageEntry	STRUC
	apiname		DD	?
	api		DD	?
PackageEntry	ENDS

EXTRN	___loader_exports:WORD		; PackageEntry table
EXTRN	___data16:WORD			; 16-bit data scratch selector
EXTRN	___data16_2:WORD		; Second 16-bit data scratch selector
EXTRN	___stack16:WORD			; 16-bit stack scratch selector
EXTRN	___alt_calls:BYTE		; Use changed API for DOS/4GW 2.01

;** The compiler generates an EXTRN record for the symbol _fltused_ when
;** floating point is used in the object module.  Here we make sure the
;** linker pulls in the floating-point initialization code when it links
;** the DLL client, because the DLL (which may use floating point) will
;** not be able to safely reinitialize the coprocessor.

EXTRN	_fltused_:ABS

DLLCODEGROUP	GROUP	DLLCODE16

DLLCODE16	SEGMENT	PUBLIC BYTE USE16 'CODE'
	ASSUME	CS:DLLCODE16,DS:FLAT,SS:FLAT

	PUBLIC	___flat_ss, ___flat_cs
___flat_ss	DW	?
SS_VALUE	EQU	WORD PTR cs:[0]	; necessary to work around bug in
___flat_cs	DW	?
CS_VALUE	EQU	WORD PTR cs:[2]	; DOS/4GW loader (mixed 16-bit code)

;**
;** code16_thunk:
;**
;** 16-bit glue between DOS/4GW's internal 16-bit loader API and
;** my exported DLL API.  This function is called on the flat model
;** stack, but any arguments have already been pushed on it as if
;** it were a 16-bit stack (and any pointer arguments have been
;** converted to 16:16 aliases).  The stack itself will be converted
;** to a 16:16 alias before the 16-bit function is called.
;**
;** Calling convention:
;**	EBP		stack frame with far return to 32-bit caller
;**	EDI		target function (16:16 pointer)
;**	ESI		value to return on error
;**	BX		scratch selector for stack alias
;**
;** Returns:
;**	EAX		return value from target function
;**			other registers except EBP modified
;**			ESP (post-RETF) will be 8 bytes > EBP
;**

code16_thunk:
	test	edi, edi		; Make sure function was exported
	jz	unsupported_function	;
	mov	ax, 7			; DPMI Set Segment Base
	lea	ecx, [esp-AliasStackSize]
	mov	dx, cx			; DX:CX = stack base =
	shr	ecx, 16			;   8K below current ESP
	int	31h			; Set base of BX to ESP-AliasStackSize
	jc	stk_alias_failed

	cli
	mov	ss, bx			; load 16-bit alias stack
	mov	sp, AliasStackSize
	ASSUME	ss:NOTHING
	sti

	;**
	;** Note: this code assumes that none of the thunked-to
	;** functions modify EBP (even its high word).  In order
	;** to call a function that modifies EBP, we'd have to find
	;** some other safe place to store the stack frame we will
	;** use when we return.
	;**

	lea	ax, thunk16_return
	and	ax, 0FFFh		; work around loader bug
	push	cs			; push pseudo-return address
	push	ax
	push	edi			; push 16:16 target address
	retf				;   and call it
thunk16_return:
	movzx	eax, ax			; convert AX return to EAX

thunk_return:
	cli
	mov	ss, SS_VALUE		; reload 32-bit stack
	mov	esp, ebp		; pop parameters, address return frame
	ASSUME	ss:FLAT
	sti
	db	66h			; do a full 32-bit return and
	retf				; return to 32-bit code

unsupported_function:			; not in this version of DOS/4GW?
stk_alias_failed:			; out of descriptors?
	int	3
	mov	eax, esi
	jmp	SHORT	thunk_return

DLLCODE16	ENDS


.CODE

EXTRN	_init_dll_loader:NEAR
EXTRN	_locate_dll:NEAR

;**
;** int load_module (char *module_name):
;**
;** Loads a named DLL.  Returns a module handle, or 0FFFFh if unsuccessful.
;**
	ALIGN	4
	PUBLIC _load_module
_load_module:
	push	ebp			; preserve registers for C code
	push	edi
	push	esi
	push	ebx
	push	es
	mov	ebp, esp		; 1st argument is at [ebp+24]

	call	_init_dll_loader	; initialize loader if necessary
	test	eax, eax
	jz	common_init_failed

	push	DWORD PTR [ebp+24]
	call	_locate_dll		; get full path to DLL
	mov	[ebp+24], eax
	test	eax, eax
	pop	eax
	jz	SHORT locate_failed

	mov	eax, 7			; DPMI Set Segment Base
	mov	bx, ___data16		; use __data16 scratch descriptor
	mov	cx, WORD PTR [ebp+26]	; module_name+2
	mov	dx, WORD PTR [ebp+24]	; module_name
	int	31h
	jc	common_alias_failed

	push	cs			; push return address
	push	OFFSET common_thunk_ret
	mov	ebp, esp		;   and remember its stack address!

	shl	ebx, 16			; push __data16:0 (alias of module name)
	push	ebx			; (no need to pop parameters later)

	lea	eax, ___loader_exports[LoadModule*8].api
	mov	edi, [eax]		; pass destination in EDI
	mov	bx, ___stack16		; pass __stack16 scratch descriptor in BX
	mov	esi, NullModule		; pass value to return on error
	jmp	FAR PTR code16_thunk	; execute thunk
locate_failed:
	mov	ax, 0FFFFh		; return -1 handle on error
	jmp	common_thunk_ret

;**
;** int free_module (unsigned short handle):
;**
;** Unloads the DLL whose handle is passed.  Returns nothing useful.
;**
	ALIGN	4
	PUBLIC _free_module
_free_module:
	push	ebp			; preserve registers for C code
	push	edi
	push	esi
	push	ebx
	push	es
	mov	ebp, esp		; 1st argument is at [ebp+24]

	call	_init_dll_loader	; initialize loader if necessary
	test	eax, eax
	jz	common_init_failed

	mov	edx, [ebp+24]		; DX = handle

	push	cs			; push return address
	push	OFFSET common_thunk_ret
	mov	ebp, esp		;   and remember its stack address!

	push	edx			; push handle (no aliasing needed)

	lea	eax, ___loader_exports[FreeModule*8].api
	mov	edi, [eax]		; pass destination in EDI
	mov	bx, ___stack16		; pass __stack16 scratch descriptor in BX
	sub	esi, esi		; return zero on failure
	jmp	FAR PTR code16_thunk	; execute thunk

;**
;** int get_module_table (unsigned short **address):
;**
;** Obtains a pointer to an internal table of the currently valid 
;** module handles.  Returns the number of entries in the table.
;**
	ALIGN	4
	PUBLIC _get_module_table
_get_module_table:
	push	ebp			; preserve registers for C code
	push	edi
	push	esi
	push	ebx
	push	es
	mov	ebp, esp		; 1st argument is at [ebp+24]

	call	_init_dll_loader	; initialize loader if necessary
	test	eax, eax
	jz	common_init_failed

	mov	eax, 7			; DPMI Set Segment Base
	mov	bx, ___data16		; use __data16 scratch descriptor
	mov	cx, WORD PTR [ebp+26]	; address+2
	mov	dx, WORD PTR [ebp+24]	; address
	int	31h
	jc	common_alias_failed

	push	cs			; push return address
	push	OFFSET getmodtab_thunk_ret
	mov	ebp, esp		;   and remember its stack address!

	shl	ebx, 16			; push __data16:0 (address alias)
	push	ebx			; (no need to pop parameters later)

	lea	eax, ___loader_exports[GetModuleTable*8].api
	mov	edi, [eax]		; pass destination in EDI
	mov	bx, ___stack16		; pass __stack16 scratch descriptor in BX
	sub	esi, esi		; pass value to return on error
	jmp	FAR PTR code16_thunk	; execute thunk

	;** We return here because the function returns a 16:16 far pointer,
	;** and we need to convert it to a flat model near pointer.

getmodtab_thunk_ret:
	mov	esi, eax		; save return value (table size)
	mov	ebp, esp
	mov	edi, [ebp+24]		; address the returned 16:16 pointer
	mov	eax, 6			; DPMI Get Segment Base
	mov	bx, [edi+2]		; get the returned segment
	int	31h			; get its base address
	jc	common_alias_failed
	shl	ecx, 16
	mov	cx, dx
	sub	edx, edx
	mov	dx, [edi]		; add in the returned 16-bit offset
	add	ecx, edx
	mov	[edi], ecx		; and return the new flat pointer
	mov	eax, esi		; restore return value
	jmp	SHORT common_thunk_ret

;**
;** int get_module_name (unsigned short handle, char *buffer, unsigned short buf_len):
;**
;** Obtains the name of the specified module.  Returns 1 if successful,
;** 0 otherwise.
;**
	ALIGN	4
	PUBLIC _get_module_name
_get_module_name:
	push	ebp			; preserve registers for C code
	push	edi
	push	esi
	push	ebx
	push	es
	mov	ebp, esp		; 1st argument is at [ebp+24]

	call	_init_dll_loader	; initialize loader if necessary
	test	eax, eax
	jz	common_init_failed

	mov	eax, 7			; DPMI Set Segment Base
	mov	bx, ___data16		; use __data16 scratch descriptor
	mov	cx, WORD PTR [ebp+30]	; buffer+2
	mov	dx, WORD PTR [ebp+28]	; buffer
	int	31h
	jc	common_alias_failed

	mov	edx, [ebp+24]		; DX = handle
	mov	ecx, [ebp+32]		; CX = buffer length

	push	cs			; push return address
	push	OFFSET common_thunk_ret
	mov	ebp, esp		;   and remember its stack address!

	push	ecx			; push buffer length
	shl	ebx, 16			; push buffer pointer (__data16:0)
	push	ebx
	push	edx			; push handle

	lea	eax, ___loader_exports[GetModuleName*8].api
	mov	edi, [eax]		; pass destination in EDI
	mov	bx, ___stack16		; pass __stack16 scratch descriptor in BX
	sub	esi, esi		; pass value to return on error
	jmp	FAR PTR code16_thunk	; execute thunk

;**
;** int get_module_handle (char *module_name, unsigned short *handle):
;**
;** Obtains the handle of the specified module.  If the module is not
;** loaded, reports its handle as 0FFFFh.  Returns nothing useful.
;**
	ALIGN	4
	PUBLIC _get_module_handle
_get_module_handle:
	push	ebp			; preserve registers for C code
	push	edi
	push	esi
	push	ebx
	push	es
	mov	ebp, esp		; 1st argument is at [ebp+24]

	call	_init_dll_loader	; initialize loader if necessary
	test	eax, eax
	jz	common_init_failed

	mov	eax, 7			; DPMI Set Segment Base
	mov	bx, ___data16_2		; use __data16_2 scratch descriptor
	mov	cx, WORD PTR [ebp+30]	; handle+2
	mov	dx, WORD PTR [ebp+28]	; handle
	int	31h
	jc	common_alias_failed

	mov	eax, 7			; DPMI Set Segment Base
	mov	bx, ___data16		; use __data16 scratch descriptor
	mov	cx, WORD PTR [ebp+26]	; module_name+2
	mov	dx, WORD PTR [ebp+24]	; module_name
	int	31h
	jc	common_alias_failed

	push	cs			; push return address
	push	OFFSET common_thunk_ret
	mov	ebp, esp		;   and remember its stack address!

	mov	ax, ___data16_2		; push __data16_2:0 (alias pointer
	shl	eax, 16			;   to handle)
	push	eax			;
	mov	ax, ___data16		; push __data16:0 (alias pointer
	shl	eax, 16			;   to module_name)
	push	eax			;

	lea	eax, ___loader_exports[GetModuleHandle*8].api
	mov	edi, [eax]		; pass destination in EDI
	mov	bx, ___stack16		; pass __stack16 scratch descriptor in BX
	sub	esi, esi		; return zero on failure
	jmp	FAR PTR code16_thunk	; execute thunk

;**
;** int get_exported_address (unsigned short handle, char *name, void __far **address):
;**
;** Looks up a name in a specified module.  Returns nothing useful.
;**
	ALIGN	4
	PUBLIC _get_exported_address
_get_exported_address:
	push	ebp			; preserve registers for C code
	push	edi
	push	esi
	push	ebx
	push	es
	mov	ebp, esp		; 1st argument is at [ebp+24]

	call	_init_dll_loader	; initialize loader if necessary
	test	eax, eax
	jz	common_init_failed

	mov	eax, 7			; DPMI Set Segment Base
	mov	bx, ___data16_2		; use __data16_2 scratch descriptor
	mov	cx, WORD PTR [ebp+34]	; address+2
	mov	dx, WORD PTR [ebp+32]	; address
	int	31h
	jc	common_alias_failed

	mov	eax, 7			; DPMI Set Segment Base
	mov	bx, ___data16		; use __data16 scratch descriptor
	mov	cx, WORD PTR [ebp+30]	; name+2
	mov	dx, WORD PTR [ebp+28]	; name
	int	31h
	jc	common_alias_failed

	mov	edx, [ebp+24]		; DX = handle

	push	cs			; push return address
	push	OFFSET common_thunk_ret
	mov	ebp, esp		;   and remember its stack address!

	cmp	___alt_calls, 0		; check for DOS/4GW 2.01
	jnz	alt_get_exported_api	;

	sub	eax, eax		; push high word of paddress alias
	mov	ax, ___data16_2		;
	push	eax			;
	mov	ax, ___data16		; push combined low word of paddr alias
	push	eax			; (zero) and high word of name alias
	mov	ax, dx			; push combined low word of name alias
	push	eax			; (zero) and handle
	jmp	SHORT do_get
alt_get_exported_api:
	sub	eax, eax		; push __data16_2:0 (alias pointer
	mov	ax, ___data16_2		;   to paddress)
	shl	eax, 16			;
	push	eax			;
	mov	ax, ___data16		; push __data16:0 (alias pointer
	shl	eax, 16			;   to name)
	push	eax			;
	sub	eax, eax		; push combined new flag (zero)
	mov	ax, dx			;   and handle
	push	eax
do_get:
	lea	eax, ___loader_exports[GetExportedAddress*8].api
	mov	edi, [eax]		; pass destination in EDI
	mov	bx, ___stack16		; pass __stack16 scratch descriptor in BX
	sub	esi, esi		; return zero on failure
	jmp	FAR PTR code16_thunk	; execute thunk

;**
;** int relocate_pointer (void __far **address, unsigned short handle):
;**
;** Converts a pointer in section:offset form (as might appear in a
;** link map) to its in-memory selector:offset form.  Returns 0 if
;** successful, 1 if unsuccessful.
;**
	ALIGN	4
	PUBLIC _relocate_pointer
_relocate_pointer:
	push	ebp			; preserve registers for C code
	push	edi
	push	esi
	push	ebx
	push	es
	mov	ebp, esp		; 1st argument is at [ebp+24]

	call	_init_dll_loader	; initialize loader if necessary
	test	eax, eax
	jz	common_init_failed

	mov	eax, 7			; DPMI Set Segment Base
	mov	bx, ___data16		; use __data16 scratch descriptor
	mov	cx, WORD PTR [ebp+26]	; address+2
	mov	dx, WORD PTR [ebp+24]	; address
	int	31h
	jc	common_alias_failed

	mov	edx, [ebp+28]		; DX = handle

	push	cs			; push return address
	push	OFFSET common_thunk_ret
	mov	ebp, esp		;   and remember its stack address!

	push	edx			; push handle
	mov	ax, ___data16		; push __data16:0 (alias pointer
	shl	eax, 16			;   to address)
	push	eax			;

	lea	eax, ___loader_exports[RelocatePointer*8].api
	mov	edi, [eax]		; pass destination in EDI
	mov	bx, ___stack16		; pass __stack16 scratch descriptor in BX
	mov	esi, 1			; return one on failure
	jmp	FAR PTR code16_thunk	; execute thunk

;**
;** int unrelocate_pointer (void __far **address, unsigned short handle):
;**
;** Converts a pointer in selector:offset form to its unrelocated
;** (link map) section:offset equivalent.  Returns 0 if successful,
;** 1 if unsuccessful.
;**
	ALIGN	4
	PUBLIC _unrelocate_pointer
_unrelocate_pointer:
	push	ebp			; preserve registers for C code
	push	edi
	push	esi
	push	ebx
	push	es
	mov	ebp, esp		; 1st argument is at [ebp+24]

	call	_init_dll_loader	; initialize loader if necessary
	test	eax, eax
	jz	common_init_failed

	mov	eax, 7			; DPMI Set Segment Base
	mov	bx, ___data16		; use __data16 scratch descriptor
	mov	cx, WORD PTR [ebp+26]	; address+2
	mov	dx, WORD PTR [ebp+24]	; address
	int	31h
	jc	common_alias_failed

	mov	edx, [ebp+28]		; DX = handle

	push	cs			; push return address
	push	OFFSET common_thunk_ret
	mov	ebp, esp		;   and remember its stack address!

	push	edx			; push handle
	mov	ax, ___data16		; push __data16:0 (alias pointer
	shl	eax, 16			;   to address)
	push	eax			;

	lea	eax, ___loader_exports[UnrelocatePointer*8].api
	mov	edi, [eax]		; pass destination in EDI
	mov	bx, ___stack16		; pass __stack16 scratch descriptor in BX
	mov	esi, 1			; return one on failure
	jmp	FAR PTR code16_thunk	; execute thunk

common_init_failed:
common_alias_failed:
	int	3
	mov	eax, 8BADF00Dh		; return error code for debugging
common_thunk_ret:			; we will come back here with ESP right
	pop	es
	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret

END
