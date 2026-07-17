;**
;** LOADME.ASM: Support code for implicit DLL loading under DOS/4GW.
;**
;** Copyright (c) 1996,1997 Dan Teven.
;**

.386p
.MODEL FLAT

.DATA

dllhandle	DD	0FFFFh	; 0FFFFh = not loaded
initfailmsg	DB	'Fatal error: unable to initialize DLL loader.',0,'$'
loadfailmsg	DB	'Fatal error: unable to load DLL.',0,'$'
findfailmsg	DB	'Fatal error: unable to find entry point in DLL.',0,'$'

EXTRN	__dllname:BYTE
EXTRN	__dllfirst:BYTE
EXTRN	__dlltab:DWORD


.CODE

EXTRN	_init_dll_loader:NEAR
EXTRN	_load_module:NEAR
EXTRN	_get_exported_address:NEAR
EXTRN	_fatal_load_error:NEAR

;**
;** __loadme:
;**
;** This function is responsible for simulating implicit DLL loading
;** under DOS/4GW.  The import library stubs all point here.  The
;** first time one is called, the DLL loader is initalized (if it
;** hasn't already been explicitly initialized), the DLL corresponding
;** to the import library is loaded, the address of the target function
;** is looked up, and control is transferred to the target function.
;** Future calls to that function will go there more directly.
;**
;** Implementation note: it's possible for __loadme to look up the
;** addresses (and patch the entry points) for all of the exported
;** functions listed in the import library.  There's no compelling
;** reason to do all that work up front.
;**
	ALIGN	4
	PUBLIC	__loadme
__loadme:
	pop	eax		; EAX = return address
	sub	eax, 5		; EAX = address of CALL
	pushad			; protect other registers
	mov	edi, eax	; save address of CALL in EDI
	cmp	dllhandle, 0FFFFh
	jne	loaded_ok	; if (the DLL has not been loaded) {
	call	_init_dll_loader;   patch DOS/4GW, init loader if necessary
	test	eax, eax	;   (check for init failure)
	mov	edx, OFFSET initfailmsg;
	jz	whoops		;
	push	OFFSET __dllname;
	call	_load_module	;   load the DLL
	add	esp, 4		;
	mov	dllhandle, eax	;   save module handle
	cmp	ax, 0FFFFh	;   (check for load failure)
	mov	edx, OFFSET loadfailmsg
	je	whoops		;
loaded_ok:			;   }
	sub	esp, 8		; allocate space on stack for far pointer
	mov	eax, esp	;
	push	eax		; push address of far pointer
	mov	eax, edi	;
	mov	cl, 5		; convert CALL address to table index
	sub	eax, OFFSET __dllfirst
	div	cl		;
	sub	ah, ah		; clear remainder of division
	mov	eax, __dlltab[4*eax]
	push	eax		; push function name
	push	dllhandle	; push module handle
	call	_get_exported_address
	add	esp, 12		;
	pop	eax		; recover far pointer, release stack space
	pop	edx		;
	mov	cx, cs		;
	cmp	dx, cx		; if not flat address, can't do it!
	mov	edx, OFFSET findfailmsg
	jne	whoops

	;**
	;** When we get here, the DLL has been loaded and we have
	;** retrieved the entry point address in EAX.  EDI points to
	;** the CALL opcode, which we patch with a near jump.  We
	;** also patch the destination.  Future calls to that function
	;** will go there without having to come through __loadme --
	;** just the one extra jump will be taken.
	;**

	sub	eax, edi	; E9 = jump near relative
	mov	BYTE PTR [edi], 0E9h
	sub	eax, 5		; adjust for size of instruction
	mov	[edi+1], eax
	popad			; restore registers, reload EAX
	jmp	eax		; jump to destination

	;**
	;** On a load failure, we can't return to the caller, because
	;** we don't know how many bytes of parameters we need to clean
	;** up when we return.  More flexible error handling would
	;** require a byte (parameter list length) after each import
	;** stub.  Also, it would be nice to jump to an optional
	;** caller-supplied function instead of fatal_load_error.
	;**
whoops:
	int	3		; DLL load failed!
	push	edx
	call	_fatal_load_error
	hlt			; should never get here

END

