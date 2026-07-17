#include "vxd.h"

/*
** Copyright (c) 1995, 1996, 1997 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 2$
** $Date: 10/11/00 8:31:13 PM$
*/

#ifdef KERNEL

#ifdef KERNEL_NT

#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE	1
#endif
#ifndef NULL
#define NULL    '\0'                    // Null pointer
#endif

typedef unsigned long DWORD;            // dw

//
// Kernel mode memory operations
//

#define FL_ZERO_MEMORY      0x00000001

void *
__stdcall
EngAllocMem(
    unsigned long Flags,
    unsigned long MemSize,
    unsigned long Tag
    );

void
__stdcall
EngFreeMem(
    void * Mem
    );

#endif	// KERNEL_NT

#include <h3.h>


void __cdecl exit(int code)
{
}

int __cdecl getenv(char *str)
{
    return 0;
}

void __cdecl putenv(char *str)
{
}

int __cdecl strcmp(char *a, char *b)
{
	return 0;
}


void * __cdecl malloc(int nbytes)
{
#ifndef KERNEL_NT
    void *ptr = _HeapAllocate(nbytes, 0);
#else
    void *ptr = EngAllocMem( 0, nbytes, '3hD');
#endif

    return ptr;
}

void __cdecl free(void *ptr)
{
#ifndef KERNEL_NT
    _HeapFree(ptr, 0);
#else
    EngFreeMem( ptr );
#endif
}

int __cdecl atoi(char *buf)
{
    return 0;
}

int __cdecl sprintf(char *buf, const char *fmt, ...)
{
    return 0;
}

void *  __cdecl memset(void *ptr, int value, int size)
{
    char *cptr = (char *)ptr;
    char cvalue = (char)value;

    while (size--)
    {
	*cptr++ = cvalue;
    }

    return ptr;
}



_declspec(naked) void __cdecl  _allshr()
{
    __asm
    {
	cmp         cl,40h;
	jae         label1;
	cmp         cl,20h;
	jae         label2;
	shrd        eax,edx,cl;
	sar         edx,cl;
	ret;

label2:
	mov         eax,edx;
	sar         edx,1Fh;
	and         cl,1Fh;
	sar         eax,cl;
	ret;

label1:
	sar         edx,1Fh;
	mov         eax,edx;
	ret;

    }
}


_declspec(naked) void __cdecl _allshl()
{
    __asm
    {
	cmp         cl,40h;
	jae         label1;
	cmp         cl,20h;
	jae         label2;
        shld        edx,eax,cl;
	shl         eax,cl;
	ret;

      label2:

	mov         edx,eax;
	xor         eax,eax;
	and         cl,1Fh;
	shl         edx,cl;
	ret;

      label1:

	xor         eax,eax;
	xor         edx,edx;
	ret;
    }
}

#ifndef KERNEL_NT
_declspec(naked) void __cdecl _allmul()
{
    __asm
    {
	mov         eax,dword ptr [esp+8];
	mov         ecx,dword ptr [esp+10h];
	or          ecx,eax;
	mov         ecx,dword ptr [esp+0Ch];
	jne         label1;
	mov         eax,dword ptr [esp+4];
	_emit 0xf7;
	_emit 0xe1;	;mul         eax,ecx;
	ret         10h;

      label1:

	push        ebx;
	_emit 0xf7;
	_emit 0xe1;	;mul         eax,ecx;
	mov         ebx,eax;
	mov         eax,dword ptr [esp+8];
	_emit 0xf7;
	_emit 0x64;
	_emit 0x24;
	_emit 0x14;	mul         eax,dword ptr [esp+14h];
	add         ebx,eax;
	mov         eax,dword ptr [esp+8];
	_emit 0xf7;
	_emit 0xe1;	;mul         eax,ecx;
	add         edx,ebx;
	pop         ebx;
	ret         10h;
    }
}
#endif

#undef SET
#define SET(reg, val) \
	halStore32(BIT(24) + 0x10000000 + \
		   (FxU32)(&pH3->reg), val)

#ifndef KERNEL_NT
DWORD _stdcall H3Hal_Dyn_Init( HVM hWindowsVM )
{
    SstGRegs *pH3 = (SstGRegs *)NULL;

    GDBG_INIT();

    return(VXD_SUCCESS);
}
#else
__declspec( dllexport ) DWORD _stdcall H3Hal_Dyn_Init( void )
{
    SstGRegs *pH3 = (SstGRegs *)NULL;

    GDBG_INIT();

    return(TRUE);
}
#endif


DWORD _stdcall H3Hal_Shutdown()
{
    extern void csimShutdown( SstRegs *sst );

    csimShutdown(halInfo.boardInfo[0].sstCSIM);

#ifdef KERNEL_NT
    return(TRUE);
#else
    return(VXD_SUCCESS);
#endif
}

#ifndef KERNEL_NT
#define VxD_Prolog \
    struct Client_Reg_Struc *crRegs; \
    _asm mov eax,ebp \
    _asm mov ebp,esp  \
    _asm sub esp, __LOCAL_SIZE \
    _asm mov [crRegs],eax;

#ifdef ORIG
#define VxD_Epilog \
    _asm mov eax,[crRegs] \
    _asm mov esp, ebp \
    _asm mov ebp,eax \
    _asm ret;
#else
#define VxD_Epilog \
    _asm mov esp, ebp \
    _asm ret;
#endif
#endif  // KERNEL_NT

#ifndef KERNEL_NT

#define THUNK32
extern DWORD func_table[];

_declspec(naked) void H3Hal_API()
{
    FxU32 function;
    FxU32 address;
    FxU32 data;
    extern void setLevel(int level, int value);
    static h3hal_gdbg = 0;
    
    VxD_Prolog;

    function = crRegs->Client_ECX;
    address = crRegs->Client_EDI;
    data = crRegs->Client_EAX;

    switch (function)
    {
      case 1:
	  halInfo.hw = 0;
	  csimInitDriver(data,	/* nbytes of board memory */
			 (volatile FxU32 *)address,	/* linear address
							 * of sim host board
							 * vram
							 */
			 (volatile FxU32 *) 0x10000000L);	/* hack */
	  break;

      case 2:	/* halStore32 */

	  halStore32((volatile void *)(address), data);
	  break;


      case 3:
	  data = halLoad32((volatile void *)(address));
	  crRegs->Client_EAX = data;
	  crRegs->Client_EDX = ((data & 0xFFFF0000) >> 16);
	  break;

#ifdef THUNK32
      case 4:
	  crRegs->Client_EAX = (DWORD) func_table;
	  break;
#endif

    }

    // allow interactive setting of individual gdbg_debuglevels
    if (h3hal_gdbg != 0)
    {
	// upper 16 bits is level, lower 16 bits is value
	setLevel(h3hal_gdbg >> 16, h3hal_gdbg & 0xFFFF);
	h3hal_gdbg = 0;
    }

    VxD_Epilog;
}

#endif /* #ifndef KERNEL_NT */
#endif /* #ifdef KERNEL */



