
	.xlist
DOS5 = 1
	include cmacros.inc
	incControl = 1
	include gdidefs.inc
	include macros.inc
	.list


;;; --------------------------------------------------------------------
;;; C O D E
;;; --------------------------------------------------------------------
sBegin Code
assumes cs,Code
	.386

;;; *****:	IHV specific

;;; OPENGL_CMD subescapes
OPENGL_MAP_REG		equ	100
OPENGL_UNMAP_REG	equ	101
OPENGL_GETLOCK		equ	102

;;; --------------------------------------------------------------------
;;; DRIVER NAME STRING
__GLDrvName	db	"S3Virge"
		db	0
__GLDrvNameLen	dw	$-__GLDrvName
	
;;; Version: Must be 2.
__GLDrvVersion	dw	2
	
;;; ICD specific Driver Version.
__GLDrvDriverVersion	dw	1
;;; --------------------------------------------------------------------

externdef	__GLDrvName:	byte
externdef	__GLDrvNameLen:	word
externdef	__GLDrvVersion:	word
externdef	__GLDrvDriverVersion:	word

externFP	DIB_Control

cProc	__glDoOpenglCmd,<FAR,PUBLIC,PASCAL,NODATA>,<esi,edi>
	parmD	lpDevice
	parmW	function
	parmD	lpInData
	parmD	lpOutData
cBegin
	assumes	gs,nothing
	assumes	es,nothing
	assumes	fs,nothing
	assumes	gs,nothing

	lds	di, lpInData	; Get pointer to the data
	mov	bx, wptr [di]	; Get subescape

	cmp	bx, OPENGL_MAP_REG
	je	C_DoMapReg

	cmp	bx, OPENGL_UNMAP_REG
	je	C_DoUnmapReg

	cmp	bx, OPENGL_GETLOCK
	je	C_DoGetLock

	push	lpDevice
	push	function
	push	lpInData
	push	lpOutData
	call	DIB_Control	; nothing match.  Let the DIB Engine handle it.
	
C_exit:
cEnd


;;; Map the registers in
PLABEL	C_DoMapReg

	lds	di, lpInData
	add	di, 16		; 1st arg
	mov	cx, word ptr [di]; physAddrLow
	add	di, 2
	mov	bx, word ptr [di]; physAddrHigh
	add	di, 2		; 2nd arg
	mov	ax, word ptr [di]; lenLow
	add	di, 2
	mov	si, word ptr [di]; lenHigh
	mov	di, ax
	;; int31h, ax=0800h converts phys to linear.
	;; check: http://www.delorie.com/djgpp/doc/dpmi/api/310800.html
	mov	ax, 0800h
	int	31h
	nop
	nop

	jc	C_failed1
	lds	di, lpOutData
	mov	word ptr [di], cx; low
	add	di, 2
	mov	word ptr [di], bx; high
	mov	ax, 1		; >0 means ok
	jmp	C_ok1

C_failed1:
__glDrvMapReg_failed:
	lds	di, lpOutData
	mov	dword ptr [di], 0; 0 address

C_ok1:
	mov	ax, 1
	
	jmp	C_exit

;;; Unmap the registers
PLABEL	C_DoUnmapReg

	lds	di, lpOutData

	mov	eax, 1		; >0 means ok
	mov	dword ptr [di], eax

	jmp	C_ok1

;;; Get the lock information
PLABEL	C_DoGetLock

	lds	di, lpOutData

	mov	eax, 1234h
	mov	dword ptr [di], eax

	add	di, 4
	mov	eax, 5678h
	mov	dword ptr [di], eax

	jmp	C_ok1
	
sEnd
end
