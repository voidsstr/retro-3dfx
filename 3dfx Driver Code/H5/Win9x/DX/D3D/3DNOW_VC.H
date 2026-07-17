/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** File name: 3dnow_vc.h
**
** Description: contains macros to implement 3DNow opcodes in 'C' programs
**
** $Revision: 2$
** $Date: 10/11/00 8:47:41 PM$
**
** $Log: 
**  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 3     1/25/99 4:40p Peterm
** Added Log line to header
**
 ***************************************************************************
 *
 * Copyright (c) 1997 ADVANCED MICRO DEVICES, INC.  All Rights Reserved.
 *      
 * This software is unpublished and contains the trade secrets and 
 * confidential proprietary information of AMD. Unless otherwise
 * provided in the software Agreement associated herewith, it is 
 * licensed in confidence "AS IS" and is not to be reproduced in
 * whole or part by any means except for backup. Use, duplication,
 * or disclosure by the Government is subject to the restrictions
 * in paragraph(b)(3)(B) of the Rights in Technical Data and 
 * Computer Software clause in DFAR 52.227-7013(a)(Oct. 1988).
 * Software owned by Advanced Micro Devices Inc., One AMD Place
 * P.O. Box 3453, Sunnyvale, CA 94088-3453
 ***************************************************************************
 *
 *
 * 3DNow_VC.H - Macros for VSTUDIO C/C++ version 5.0
 *
 * MACRO FORMAT
 * ============
 * This file contains inline assembly macros that
 * generate 3DNow! instructions in binary format.
 * Therefore, C or C++ programmer can use 3DNow! instructions
 * without any penalty in their C or C++ source code.
 * 
 * The macro's name and format conventions for when using the
 * _asm directive are as follows:
 *
 *     1. For instructions with two operands, operand names
 *        are combined into a single argument to the macro.
 *        First part of the argument is a destination, 
 *        second part of the macro is source operand.
 *              ex) _asm PFCMPEQ (mm3_mm4)
 *                                        |    |
 *                               dst  src
 *
 *      2. The destination operand can be mm0 to mm7 only. 
 *         The source operand can be any one of the registers
 *         mm0 to mm7 or eax, ecx, edx, ebx, esi, or edi 
 *         that contains effective address.
 *              ex) _asm PFRCP    (MM7_MM6)
 *              ex) _asm PFRCPIT2 (mm0_mm4)
 *              ex) _asm PFMUL    (mm3_edi)
 *      
 *      3. PREFETCH and PREFETCHW take one src operand, 
 *         PREF_EAX to PREF_EDI and PREFW_EAX to PREFW_EDI
 *         respectively that contains an effective address.
 *              ex) _asm PREFETCH (pref_edi)
 *
 *
 * NOTE: Not all instruction macros, nor all possible 
 *       combinations of operands have been explicitely
 *       tested. If any errors are found, please report
 *       them. 
 *                          
 * _asm EXAMPLE
 * ============
 * Following program doesn't do anything but it shows you
 * how to use inline assembly 3DNow! instructions in C.
 * Note that this will only work in flat memory model which
 * segment registers cs, ds, ss and es point to the same 
 * linear address space total less than 4GB.
 * 
 * #include <stdio.h>                    
 * #include "3DNow_VC.h"                   
 * 
 * void DoMul(float x, float y, float *z)
 * {
 *      _asm {                            
 *              movd mm1, x
 *              movd mm2, y                  
 *              pfmul (mm1_mm2)               
 *              mov  eax, z
 *              movd [eax], mm1
 *              femms
 *      }                                 
 * }
 *
 * void DoMulMem(float x, float y, float *z)
 * {
 *      // 
 *      // Demonstration of using the memory instead of
 *      // multimedia register
 *      //
 *      _asm {        
 *              movd mm3, x
 *              lea esi, y   // load effective address of y
 *              pfmul (mm3_esi)
 *              mov eax, z
 *              movd [eax], mm3                  
 *              femms
 *      }
 * }
 *                                       
 * void main ()                          
 * {                                     
 *      float x = (float)1.25;            
 *      float y = (float)1.25;            
 *      float z, zz;                      
 *
 *      DoMul(x, y, &z);
 *
 *      printf ("value of z = %f\n", z);  
 *        
 *      DoMul(x, y, &zz);
 *
 *      printf ("value of zz = %f\n", zz);
 *  }          
 *
 *               
 ***************************************************************************/
#ifndef _3DNow_VC_H_
#define _3DNow_VC_H_
/* Operand defines for instructions two operands */
#define mm0_mm0 __asm __emit 0xc0
#define mm0_mm1 __asm __emit 0xc1
#define mm0_mm2 __asm __emit 0xc2
#define mm0_mm3 __asm __emit 0xc3
#define mm0_mm4 __asm __emit 0xc4
#define mm0_mm5 __asm __emit 0xc5
#define mm0_mm6 __asm __emit 0xc6
#define mm0_mm7 __asm __emit 0xc7
#define mm0_eax __asm __emit 0x00
#define mm0_ecx __asm __emit 0x01
#define mm0_edx __asm __emit 0x02
#define mm0_ebx __asm __emit 0x03
#define mm0_esi __asm __emit 0x06
#define mm0_edi __asm __emit 0x07
#define mm1_mm0 __asm __emit 0xc8
#define mm1_mm1 __asm __emit 0xc9
#define mm1_mm2 __asm __emit 0xca
#define mm1_mm3 __asm __emit 0xcb
#define mm1_mm4 __asm __emit 0xcc
#define mm1_mm5 __asm __emit 0xcd
#define mm1_mm6 __asm __emit 0xce
#define mm1_mm7 __asm __emit 0xcf
#define mm1_eax __asm __emit 0x08
#define mm1_ecx __asm __emit 0x09
#define mm1_edx __asm __emit 0x0a
#define mm1_ebx __asm __emit 0x0b
#define mm1_esi __asm __emit 0x0e
#define mm1_edi __asm __emit 0x0f
#define mm2_mm0 __asm __emit 0xd0
#define mm2_mm1 __asm __emit 0xd1
#define mm2_mm2 __asm __emit 0xd2
#define mm2_mm3 __asm __emit 0xd3
#define mm2_mm4 __asm __emit 0xd4
#define mm2_mm5 __asm __emit 0xd5
#define mm2_mm6 __asm __emit 0xd6
#define mm2_mm7 __asm __emit 0xd7
#define mm2_eax __asm __emit 0x10
#define mm2_ecx __asm __emit 0x11
#define mm2_edx __asm __emit 0x12
#define mm2_ebx __asm __emit 0x13
#define mm2_esi __asm __emit 0x16
#define mm2_edi __asm __emit 0x17
#define mm3_mm0 __asm __emit 0xd8
#define mm3_mm1 __asm __emit 0xd9
#define mm3_mm2 __asm __emit 0xda
#define mm3_mm3 __asm __emit 0xdb
#define mm3_mm4 __asm __emit 0xdc
#define mm3_mm5 __asm __emit 0xdd
#define mm3_mm6 __asm __emit 0xde
#define mm3_mm7 __asm __emit 0xdf
#define mm3_eax __asm __emit 0x18
#define mm3_ecx __asm __emit 0x19
#define mm3_edx __asm __emit 0x1a
#define mm3_ebx __asm __emit 0x1b
#define mm3_esi __asm __emit 0x1e
#define mm3_edi __asm __emit 0x1f
#define mm4_mm0 __asm __emit 0xe0
#define mm4_mm1 __asm __emit 0xe1
#define mm4_mm2 __asm __emit 0xe2
#define mm4_mm3 __asm __emit 0xe3
#define mm4_mm4 __asm __emit 0xe4
#define mm4_mm5 __asm __emit 0xe5
#define mm4_mm6 __asm __emit 0xe6
#define mm4_mm7 __asm __emit 0xe7
#define mm4_eax __asm __emit 0x20
#define mm4_ecx __asm __emit 0x21
#define mm4_edx __asm __emit 0x22
#define mm4_ebx __asm __emit 0x23
#define mm4_esi __asm __emit 0x26
#define mm4_edi __asm __emit 0x27
#define mm5_mm0 __asm __emit 0xe8
#define mm5_mm1 __asm __emit 0xe9
#define mm5_mm2 __asm __emit 0xea
#define mm5_mm3 __asm __emit 0xeb
#define mm5_mm4 __asm __emit 0xec
#define mm5_mm5 __asm __emit 0xed
#define mm5_mm6 __asm __emit 0xee
#define mm5_mm7 __asm __emit 0xef
#define mm5_eax __asm __emit 0x28
#define mm5_ecx __asm __emit 0x29
#define mm5_edx __asm __emit 0x2a
#define mm5_ebx __asm __emit 0x2b
#define mm5_esi __asm __emit 0x2e
#define mm5_edi __asm __emit 0x2f
#define mm6_mm0 __asm __emit 0xf0
#define mm6_mm1 __asm __emit 0xf1
#define mm6_mm2 __asm __emit 0xf2
#define mm6_mm3 __asm __emit 0xf3
#define mm6_mm4 __asm __emit 0xf4
#define mm6_mm5 __asm __emit 0xf5
#define mm6_mm6 __asm __emit 0xf6
#define mm6_mm7 __asm __emit 0xf7
#define mm6_eax __asm __emit 0x30
#define mm6_ecx __asm __emit 0x31
#define mm6_edx __asm __emit 0x32
#define mm6_ebx __asm __emit 0x33
#define mm6_esi __asm __emit 0x36
#define mm6_edi __asm __emit 0x37
#define mm7_mm0 __asm __emit 0xf8
#define mm7_mm1 __asm __emit 0xf9
#define mm7_mm2 __asm __emit 0xfa
#define mm7_mm3 __asm __emit 0xfb
#define mm7_mm4 __asm __emit 0xfc
#define mm7_mm5 __asm __emit 0xfd
#define mm7_mm6 __asm __emit 0xfe
#define mm7_mm7 __asm __emit 0xff
#define mm7_eax __asm __emit 0x38
#define mm7_ecx __asm __emit 0x39
#define mm7_edx __asm __emit 0x3a
#define mm7_ebx __asm __emit 0x3b
#define mm7_esi __asm __emit 0x3e
#define mm7_edi __asm __emit 0x3f

#define MM0_MM0 __asm __emit 0xc0
#define MM0_MM1 __asm __emit 0xc1
#define MM0_MM2 __asm __emit 0xc2
#define MM0_MM3 __asm __emit 0xc3
#define MM0_MM4 __asm __emit 0xc4
#define MM0_MM5 __asm __emit 0xc5
#define MM0_MM6 __asm __emit 0xc6
#define MM0_MM7 __asm __emit 0xc7
#define MM0_EAX __asm __emit 0x00
#define MM0_ECX __asm __emit 0x01
#define MM0_EDX __asm __emit 0x02
#define MM0_EBX __asm __emit 0x03
#define MM0_ESI __asm __emit 0x06
#define MM0_EDI __asm __emit 0x07
#define MM1_MM0 __asm __emit 0xc8
#define MM1_MM1 __asm __emit 0xc9
#define MM1_MM2 __asm __emit 0xca
#define MM1_MM3 __asm __emit 0xcb
#define MM1_MM4 __asm __emit 0xcc
#define MM1_MM5 __asm __emit 0xcd
#define MM1_MM6 __asm __emit 0xce
#define MM1_MM7 __asm __emit 0xcf
#define MM1_EAX __asm __emit 0x08
#define MM1_ECX __asm __emit 0x09
#define MM1_EDX __asm __emit 0x0a
#define MM1_EBX __asm __emit 0x0b
#define MM1_ESI __asm __emit 0x0e
#define MM1_EDI __asm __emit 0x0f
#define MM2_MM0 __asm __emit 0xd0
#define MM2_MM1 __asm __emit 0xd1
#define MM2_MM2 __asm __emit 0xd2
#define MM2_MM3 __asm __emit 0xd3
#define MM2_MM4 __asm __emit 0xd4
#define MM2_MM5 __asm __emit 0xd5
#define MM2_MM6 __asm __emit 0xd6
#define MM2_MM7 __asm __emit 0xd7
#define MM2_EAX __asm __emit 0x10
#define MM2_ECX __asm __emit 0x11
#define MM2_EDX __asm __emit 0x12
#define MM2_EBX __asm __emit 0x13
#define MM2_ESI __asm __emit 0x16
#define MM2_EDI __asm __emit 0x17
#define MM3_MM0 __asm __emit 0xd8
#define MM3_MM1 __asm __emit 0xd9
#define MM3_MM2 __asm __emit 0xda
#define MM3_MM3 __asm __emit 0xdb
#define MM3_MM4 __asm __emit 0xdc
#define MM3_MM5 __asm __emit 0xdd
#define MM3_MM6 __asm __emit 0xde
#define MM3_MM7 __asm __emit 0xdf
#define MM3_EAX __asm __emit 0x18
#define MM3_ECX __asm __emit 0x19
#define MM3_EDX __asm __emit 0x1a
#define MM3_EBX __asm __emit 0x1b
#define MM3_ESI __asm __emit 0x1e
#define MM3_EDI __asm __emit 0x1f
#define MM4_MM0 __asm __emit 0xe0
#define MM4_MM1 __asm __emit 0xe1
#define MM4_MM2 __asm __emit 0xe2
#define MM4_MM3 __asm __emit 0xe3
#define MM4_MM4 __asm __emit 0xe4
#define MM4_MM5 __asm __emit 0xe5
#define MM4_MM6 __asm __emit 0xe6
#define MM4_MM7 __asm __emit 0xe7
#define MM4_EAX __asm __emit 0x20
#define MM4_ECX __asm __emit 0x21
#define MM4_EDX __asm __emit 0x22
#define MM4_EBX __asm __emit 0x23
#define MM4_ESI __asm __emit 0x26
#define MM4_EDI __asm __emit 0x27
#define MM5_MM0 __asm __emit 0xe8
#define MM5_MM1 __asm __emit 0xe9
#define MM5_MM2 __asm __emit 0xea
#define MM5_MM3 __asm __emit 0xeb
#define MM5_MM4 __asm __emit 0xec
#define MM5_MM5 __asm __emit 0xed
#define MM5_MM6 __asm __emit 0xee
#define MM5_MM7 __asm __emit 0xef
#define MM5_EAX __asm __emit 0x28
#define MM5_ECX __asm __emit 0x29
#define MM5_EDX __asm __emit 0x2a
#define MM5_EBX __asm __emit 0x2b
#define MM5_ESI __asm __emit 0x2e
#define MM5_EDI __asm __emit 0x2f
#define MM6_MM0 __asm __emit 0xf0
#define MM6_MM1 __asm __emit 0xf1
#define MM6_MM2 __asm __emit 0xf2
#define MM6_MM3 __asm __emit 0xf3
#define MM6_MM4 __asm __emit 0xf4
#define MM6_MM5 __asm __emit 0xf5
#define MM6_MM6 __asm __emit 0xf6
#define MM6_MM7 __asm __emit 0xf7
#define MM6_EAX __asm __emit 0x30
#define MM6_ECX __asm __emit 0x31
#define MM6_EDX __asm __emit 0x32
#define MM6_EBX __asm __emit 0x33
#define MM6_ESI __asm __emit 0x36
#define MM6_EDI __asm __emit 0x37
#define MM7_MM0 __asm __emit 0xf8
#define MM7_MM1 __asm __emit 0xf9
#define MM7_MM2 __asm __emit 0xfa
#define MM7_MM3 __asm __emit 0xfb
#define MM7_MM4 __asm __emit 0xfc
#define MM7_MM5 __asm __emit 0xfd
#define MM7_MM6 __asm __emit 0xfe
#define MM7_MM7 __asm __emit 0xff
#define MM7_EAX __asm __emit 0x38
#define MM7_ECX __asm __emit 0x39
#define MM7_EDX __asm __emit 0x3a
#define MM7_EBX __asm __emit 0x3b
#define MM7_ESI __asm __emit 0x3e
#define MM7_EDI __asm __emit 0x3f

/* Operand defines for prefetch and prefetchw */
#define pref_eax  __asm __emit 0x00
#define pref_ecx  __asm __emit 0x01
#define pref_edx  __asm __emit 0x02
#define pref_ebx  __asm __emit 0x03
#define pref_esi  __asm __emit 0x06
#define pref_edi  __asm __emit 0x07
#define prefw_eax __asm __emit 0x08
#define prefw_ecx __asm __emit 0x09
#define prefw_edx __asm __emit 0x0A
#define prefw_ebx __asm __emit 0x0B
#define prefw_esi __asm __emit 0x0E
#define prefw_edi __asm __emit 0x0F

#define PREF_EAX  __asm __emit 0x00
#define PREF_ECX  __asm __emit 0x01
#define PREF_EDX  __asm __emit 0x02
#define PREF_EBX  __asm __emit 0x03
#define PREF_ESI  __asm __emit 0x06
#define PREF_EDI  __asm __emit 0x07
#define PREFW_EAX __asm __emit 0x08
#define PREFW_ECX __asm __emit 0x09
#define PREFW_EDX __asm __emit 0x0A
#define PREFW_EBX __asm __emit 0x0B
#define PREFW_ESI __asm __emit 0x0E
#define PREFW_EDI __asm __emit 0x0F

/* Defines for 3DNow! instructions */

#define pf2id(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x1d
#define pf2iw(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x1c
#define pfacc(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xae
#define pfadd(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x9e
#define pfcmpeq(dst_src)        __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xb0
#define pfcmpge(dst_src)        __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x90
#define pfcmpgt(dst_src)        __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xa0
#define pfmax(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xa4
#define pfmin(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x94
#define pfmul(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xb4
#define pfrcp(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x96
#define pfrcpit1(dst_src)       __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xa6
#define pfrcpit2(dst_src)       __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xb6
#define pfrsqrt(dst_src)        __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x97
#define pfrsqit1(dst_src)       __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xa7
#define pfsub(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x9a
#define pfsubr(dst_src)         __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xaa
#define pi2fd(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x0d
#define pi2fw(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x0c
#define femms					__asm __emit 0x0f __asm __emit 0x0e 
#define pavgusb(dst_src)        __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xbf
#define pmulhrw(dst_src)        __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xb7
#define prefetch(pref_src)      __asm __emit 0x0f __asm __emit 0x0d pref_src
#define prefetchw(prefw_src)    __asm __emit 0x0f __asm __emit 0x0d prefw_src
#define pswapw(dst_src)         __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xbb


#define PF2ID(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x1d
#define PF2IW(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x1c
#define PFACC(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xae
#define PFADD(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x9e
#define PFCMPEQ(dst_src)        __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xb0
#define PFCMPGE(dst_src)        __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x90
#define PFCMPGT(dst_src)        __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xa0
#define PFMAX(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xa4
#define PFMIN(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x94
#define PFMUL(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xb4
#define PFRCP(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x96
#define PFRCPIT1(dst_src)       __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xa6
#define PFRCPIT2(dst_src)       __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xb6
#define PFRSQRT(dst_src)        __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x97
#define PFRSQIT1(dst_src)       __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xa7
#define PFSUB(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x9a
#define PFSUBR(dst_src)         __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xaa
#define PI2FD(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x0d
#define PI2FW(dst_src)          __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0x0c
#define FEMMS					__asm __emit 0x0f __asm __emit 0x0e 
#define PAVGUSB(dst_src)        __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xbf
#define PMULHRW(dst_src)        __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xb7
#define PREFETCH(pref_src)      __asm __emit 0x0f __asm __emit 0x0d pref_src
#define PREFETCHW(prefw_src)    __asm __emit 0x0f __asm __emit 0x0d prefw_src
#define PSWAPW(dst_src)         __asm __emit 0x0f __asm __emit 0x0f dst_src __asm __emit 0xbb

#endif

