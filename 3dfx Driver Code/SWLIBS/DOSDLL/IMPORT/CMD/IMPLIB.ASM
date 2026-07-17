;**
;** IMPLIB.ASM:  This is a template for an import library.
;**
;** Copyright (c) 1996,1997 Dan Teven.
;**

.386p
.MODEL FLAT

.CODE

EXTRN	__loadme:NEAR			; magic symbol names
PUBLIC	__dllname, __dllfirst, __dlltab, __dllcount

;** Declare the DLL name here, followed by any names of functions 
;** that the DLL exports (if you will be looking the functions up by
;** name).  The names should match those of the entry-point labels
;** defined below, including underscores, to prevent accidental
;** calling convention mismatches -- note that dllentry0 and
;** dllentry1 are prototyped as __cdecl functions.

__dllname	DB	'sample.ovl',0

entry0		DB	'_dllentry0',0
entry1		DB	'_dllentry1',0

;** List the DLL entry point stubs here.  Each should consist of a public
;** name and a five-byte call to __loadme, nothing else.  (Implementation
;** note: to extend this code to support multiple import libraries, the
;** mechanism could be changed to pass the names of the DLL and the
;** symbol to __loadme; this module wouldn't have to export any public
;** symbols except the entry point stubs.  However, doing it this way
;** is much smaller when there are many entry points, since each one
;** is only five bytes.)

		ALIGN	4
__dllfirst:
		PUBLIC	_dllentry0
_dllentry0:	call	__loadme
		PUBLIC	_dllentry1
_dllentry1:	call	__loadme

;** List the pointers to the names for each of the exported functions
;** in the table above.  List them in the same order.
;**
;** After the DLL has been loaded, a call to any public name in the
;** table above will be routed to the corresponding function in the
;** DLL.

		ALIGN	4
__dlltab	LABEL	DWORD
		DD	entry0		; index 0: name for _dllentry0
		DD	entry1		; index 1: name for _dllentry1
__dlltabend	LABEL	DWORD
__dllcount	EQU	((__dlltabend - __dlltab) / 4)

END
