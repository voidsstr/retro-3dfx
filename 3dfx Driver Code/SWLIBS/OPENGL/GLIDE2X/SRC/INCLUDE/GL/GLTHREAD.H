#ifndef __gl_thread_h_
#define __gl_thread_h_

/*
** Copyright 1996,1997 Silicon Graphics, Inc.
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
** $Revision: 4$
** $Date: 10/11/00 7:52:37 PM$
*/

/*
** Hacks to get fast access to thread local storage.
**
** See: "Window's 95 System Programming Secrets" for more info about
** the thread data structures.
**
** TIB -- thread information block
** TEB -- thread environment block
** TLS -- thread local storage array
** FS segment register is set to point to the TIB
*/

#define W95_TEB_PTR			0x18
#define W95_TEB_TLS_OFFSET		0x88
#define W95_TLS_INDEX_TO_OFFSET(i)	((i)*sizeof(DWORD)+W95_TEB_TLS_OFFSET)

#define WNT_TEB_PTR			0x18
#define WNT_TEB_TLS_OFFSET		0xE10
#define WNT_TLS_INDEX_TO_OFFSET(i)	((i)*sizeof(DWORD)+WNT_TEB_TLS_OFFSET)

extern unsigned long __wglTLSIndex;
extern unsigned long __wglTLSOffset;
extern int __wglOSWin95;

#if (W95_TEB_PTR == WNT_TEB_PTR)
#define __GL_GET_THREAD_VALUE() \
    __asm { \
	__asm mov eax, DWORD PTR fs:[WNT_TEB_PTR] \
	__asm add eax, DWORD PTR __wglTLSOffset \
	__asm mov eax, DWORD PTR [eax] \
    }

#define __GL_SET_THREAD_VALUE(val) \
    __asm { \
	__asm mov eax, DWORD PTR fs:[WNT_TEB_PTR] \
	__asm add eax, DWORD PTR __wglTLSOffset \
	__asm mov ecx, DWORD PTR (val) \
	__asm mov DWORD PTR [eax], ecx \
    }
#else /* (W95_TEB_PTR == WNT_TEB_PTR) */
#define __GL_GET_THREAD_VALUE() \
    __asm { \
	__asm mov eax, __wglOSWin95 \
	__asm test eax, eax \
	__asm je $+17 \
	__asm mov eax, DWORD PTR fs:[W95_TEB_PTR] \
	__asm jmp $+11 \
	__asm mov eax, DWORD PTR fs:[WNT_TEB_PTR] \
	__asm add eax, DWORD PTR __wglTLSOffset \
	__asm mov eax, DWORD PTR [eax] \
    }

#define __GL_SET_THREAD_VALUE(val) \
    __asm { \
	__asm mov eax, __wglOSWin95 \
	__asm test eax,eax \
	__asm je $+17 \
	__asm mov eax, DWORD PTR fs:[W95_TEB_PTR] \
	__asm jmp $+11 \
	__asm mov eax, DWORD PTR fs:[WNT_TEB_PTR] \
	__asm mov ebx, DWORD PTR __wglTLSOffset \
	__asm add eax, DWORD PTR (val) \
	__asm mov DWORD PTR [eax], ecx \
    }
#endif /* (W95_TEB_PTR == WNT_TEB_PTR) */

#pragma warning (4:4035)        /* No return value */
__inline void *
__glGetThreadValue(void)
{
    __GL_GET_THREAD_VALUE();
}
#pragma warning (3:4035)

__inline void
__glSetThreadValue(void *val)
{
    __GL_SET_THREAD_VALUE(val);
}

#endif /* __gl_thread_h_ */
