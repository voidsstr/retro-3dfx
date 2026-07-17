/* $Header: cpu.h, 3, 10/11/00 8:53:29 PM, Brent$ */
/*
** Copyright (c) 1997-1999, 3Dfx Interactive, Inc.
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
** File name:   cpu.h
**
** Description: cpu features structure and supporting #defines
**
*/
#ifndef __CPU_H__
#define __CPU_H__

/****************** Structure Definitions ********************************/
typedef struct _CPU_FEATURES {
	// The #defines for these are CPU_VENDOR_xxx and CPU_FEATURE_xxx
	DWORD	dwCpuFlags;

	// Enumerated CPU type, #defined below as CPU_TYPE_xxx
	DWORD	dwCpuType;

	// These fields are filled with cpuid level 1
	// The #defines for the fields are CPU_SIG_xxx
	DWORD	dwProcessorSignature;		// overdrive, family, model, revision

	// The feature flags are like the caps bits in DirectX.  The normal features 
	// has always been around since the Pentium.  AMD added the extended stuff so 
	// they could add things without Intel comming in later and changing the bit 
	// definitions.  Starting with Williamette, Intel also adopted the extended cpuid.  
	// However, their documentation has the extended bitfields are different from AMD's.  
	// We shall see if this is the case when we get production parts.
	// The #defines for the bits are CPU_FF1_xxx and CPU_FF2_xxx, respectively
	DWORD	dwFeatureFlags1;		// normal feature flags
	DWORD	dwFeatureFlags2;		// extended feature flags

	// Intel added this to level in ebx starting with Coppermine (I think)
	// The brand index gives them a few more fields to destinguish their cpu's.  
	// I haven't bothered implementing a switch statement based on this yet.
	// The APIC ID may become important with SMP.  
	// The #defines for the fields are CPU_BRND_ID_xxx
	DWORD	dwId1Ebx;

	// These fields come from level 2 or the extended levels, depending on the vendor.
	DWORD	dwSizeL1DCache;
	DWORD	dwSizeL1ICache;
	DWORD	dwSizeL2Cache;

	// The vendor ID (manufacture's name) comes from cpuid level 0
	char	cpVendorIdString[16];	// only need 12 chars, but use 16 bytes

	// The name string is part of the extended cpuid.  It should be null terminated. 
	// If it is not supported, the string should be null.  
	char	cpCpuNameString[64];

} CPU_FEATURES, *LPCPU_FEATURES;



/****************** Function Definitions *********************************/
extern DWORD	GetCpuFeatures(LPCPU_FEATURES CpuFeatures);


/******************************* #Defines ********************************/

// dwCpuFlags bit definitions - 3dfx specific:
// this is in the CPU_FEATURES structure and also the value 
// returned by GetCpuFeatures()
// CPU Vendor
#define CPU_VENDOR_INTEL		(1 <<  0)	// Intel
#define CPU_VENDOR_AMD			(1 <<  1)	// AMD
#define CPU_VENDOR_CYRIX		(1 <<  2)	// Cyrix
#define CPU_VENDOR_CENTAUR		(1 <<  3)	// Centaur/IDT
#define CPU_VENDOR_RISE			(1 <<  4)	// Rise
#define CPU_VENDOR_TRANSMETTA	(1 <<  5)	// Transmetta
#define CPU_VENDOR_RES6			(1 <<  6)	// reserved
#define CPU_VENDOR_RES7			(1 <<  7)	// reserved
// Feature Bits (3dfx specific)
#define CPU_FEATURE_CPUID		(1 <<  8)	// cpuid supported
#define CPU_FEATURE_FPU			(1 <<  9)	// fpu (and no fp emulation)
#define CPU_FEATURE_TSC			(1 << 10)	// tsc and tsd bit not set
#define CPU_FEATURE_P6_MTRR		(1 << 11)	// P6 compatible memory type region registers
#define CPU_FEATURE_K6_MTRR		(1 << 12)	// K6 memory type region registers
#define CPU_FEATURE_CYRIX_RCR	(1 << 13)	// Cyrix address region control registers
#define CPU_FEATURE_CMOV		(1 << 14)	// conditional move instructions
#define CPU_FEATURE_MMX			(1 << 15)	// mmx (and no fp emulation)
#define CPU_FEATURE_AMMX		(1 << 16)	// AMD mmx extentions (the KNI mmx instructions)
#define CPU_FEATURE_3DNOW		(1 << 17)	// 3DNow!
#define CPU_FEATURE_3DNOWX		(1 << 18)	// 3DNow! extentions
#define CPU_FEATURE_SSE			(1 << 19)	// KNI/SSE (and OS support)
#define CPU_FEATURE_SSE2		(1 << 20)	// WNI/SSE2
#define CPU_FEATURE_RES21		(1 << 21)	// reserved
#define CPU_FEATURE_RES22		(1 << 22)	// reserved
#define CPU_FEATURE_RES23		(1 << 23)	// reserved
// Other (processor specific bugs, etc)
#define CPU_INTELP6				(1 << 24)	// check for write buffer bug
#define CPU_DEBUG_RUNNING		(1 << 25)	// a debugger is being used
#define CPU_FEATURE_RES26		(1 << 26)	// reserved
#define CPU_FEATURE_RES27		(1 << 27)	// reserved
#define CPU_FEATURE_RES28		(1 << 28)	// reserved
#define CPU_FEATURE_RES29		(1 << 29)	// reserved
#define CPU_FEATURE_RES30		(1 << 30)	// reserved
#define P6_INTELCPU_WITH_KNI	(1 << 31)	// for compatibility with our old cpuid code


/****************** CPU types ************************************/
#define	CPU_TYPE_UNKNOWN_CPU		0x000	// default
#define	CPU_TYPE_INTEL_UNKNOWN		0x100	// unidentified Intel
#define	CPU_TYPE_INTEL_486			0x101	// 486
#define CPU_TYPE_INTEL_P5			0x102	// origional Pentium (P60 & P90)
#define CPU_TYPE_INTEL_P54C			0x103	// socket 7 Pentium
#define CPU_TYPE_INTEL_P55C			0x104	// MMX Pentium
#define CPU_TYPE_INTEL_P6			0x105	// Pentium Pro
#define CPU_TYPE_INTEL_P2			0x106	// PII
#define CPU_TYPE_INTEL_P2_DECH		0x107	// Dechutes (had FXSAVE/FXRESTORE)
#define CPU_TYPE_INTEL_CELERON		0x108	// First Celeron w/o L2
#define CPU_TYPE_INTEL_CELERON_A	0x109	// Celeron with backside L2
#define CPU_TYPE_INTEL_P2_XEON		0x10a	// 1M or larger L2 cache
#define CPU_TYPE_INTEL_P3			0x10b	// PIII w/SSE
#define CPU_TYPE_INTEL_P3_XEON		0x10c	// 1M or larger L2 cache
#define CPU_TYPE_INTEL_P3C			0x10d	// PIII with backside L2 (Coppermine)
#define CPU_TYPE_INTEL_P3C_XEON		0x10e	// 1M or larger L2 cache
#define CPU_TYPE_INTEL_P4			0x10f	// Willamette
#define CPU_TYPE_AMD_UNKNOWN		0x200	// unidentified AMD
#define CPU_TYPE_AMD_486			0x201	// 486
#define CPU_TYPE_AMD_5X86			0x202	// Am5x86
#define CPU_TYPE_AMD_K5				0x203	// K5
#define CPU_TYPE_AMD_K6				0x204	// K6 w/MMX
#define CPU_TYPE_AMD_K6_2			0x205	// K6 w/3DNow
#define CPU_TYPE_AMD_K6_3			0x206	// K6 with backside L2
#define CPU_TYPE_AMD_K7				0x207	// K7 Athlon
#define CPU_TYPE_AMD_K7D			0x208	// K7 Duron
#define CPU_TYPE_AMD_K7T			0x209	// K7 Thunderbird
#define CPU_TYPE_CYRIX_UNKNOWN		0x300	// unidentified Cyrix
#define CPU_TYPE_CYRIX_486			0x301	// 486
#define CPU_TYPE_CYRIX_5X86			0x302	// 5x86
#define CPU_TYPE_CYRIX_GX			0x303	// MediaGX
#define CPU_TYPE_CYRIX_GXM			0x304	// MediaGX w/MMX
#define CPU_TYPE_CYRIX_M1			0x305	// 6x86
#define CPU_TYPE_CYRIX_M2			0x306	// 6x86MX/MII
#define CPU_TYPE_CYRIX_MX			0x307	// Cayenne core
#define CPU_TYPE_CYRIX_M3			0x308	// MIII (Gobi)
#define CPU_TYPE_CENTAUR_UNKNOWN	0x400	// unidentified Centaur
#define CPU_TYPE_CENTAUR_C6			0x401	// Winchip
#define CPU_TYPE_CENTAUR_C6_2		0x402	// Winchip 2 w/3DNow
#define CPU_TYPE_CENTAUR_C2			0x403	// C2
#define CPU_TYPE_CENTAUR_C3			0x404	// C3
#define CPU_TYPE_RISE_UNKNOWN		0x500	// unidentified Rise
#define CPU_TYPE_TRANSMETTA_UNKNOWN	0x600	// unidentified Transmetta


/****************** CPU Feature Flags ****************************/
// Standard feature flag bits, comes from edx after cpuid-1 
#define CPU_FF1_FPU			(1 <<  0)	// fpu on chip
#define CPU_FF1_VME			(1 <<  1)	// virtual mode extension
#define CPU_FF1_DE			(1 <<  2)	// debugging extension
#define CPU_FF1_PSE			(1 <<  3)	// page size extension
#define CPU_FF1_TSC			(1 <<  4)	// time stamp counter
#define CPU_FF1_MSR			(1 <<  5)	// model specific registers
#define CPU_FF1_PAE			(1 <<  6)	// page address extension (2M/4M pages)
#define CPU_FF1_MCE			(1 <<  7)	// machine check exception
#define CPU_FF1_CX8			(1 <<  8)	// CMPXCHG8 instruction
#define CPU_FF1_APIC		(1 <<  9)	// on-chip APIC hardware
#define CPU_FF1_BIT10		(1 << 10)	// reserved
#define CPU_FF1_SEP			(1 << 11)	// fast system call
#define CPU_FF1_MTRR		(1 << 12)	// memory type region registers
#define CPU_FF1_PGE			(1 << 13)	// page global enable
#define CPU_FF1_MCA			(1 << 14)	// machine check architecture
#define CPU_FF1_CMOV		(1 << 15)	// conditional move instructions
#define CPU_FF1_PAT			(1 << 16)	// page attribute table
#define CPU_FF1_PSE36		(1 << 17)	// 36-bit page size extension
#define CPU_FF1_PSN			(1 << 18)	// processor serial number
#define CPU_FF1_CLFSH		(1 << 19)	// cache line flush instruction
#define CPU_FF1_BIT20		(1 << 20)	// reserved
#define CPU_FF1_DTES		(1 << 21)	// debug trace and EMON store
#define CPU_FF1_ACPI		(1 << 22)	// processor duty cycle control
#define CPU_FF1_MMX			(1 << 23)	// mmx technology
#define CPU_FF1_FXSR		(1 << 24)	// extended fp save and restore
#define CPU_FF1_SSE			(1 << 25)	// streaming simd extentions
#define CPU_FF1_SSE2		(1 << 26)	// streaming simd extentions 2
#define CPU_FF1_SLFSNP		(1 << 27)	// selfsnoop
#define CPU_FF1_BIT28		(1 << 28)	// reserved (set in Williamette)
#define CPU_FF1_BIT29		(1 << 29)	// reserved (set in Williamette)
#define CPU_FF1_BIT30		(1 << 30)	// reserved
#define CPU_FF1_BIT31		(1 << 31)	// reserved

// Extended feature flag bits, comes from edx after cpuid-80000001 
#define CPU_FF2_FPU			(1 <<  0)	// fpu on chip
#define CPU_FF2_VME			(1 <<  1)	// virtual mode extension
#define CPU_FF2_DE			(1 <<  2)	// debugging extension
#define CPU_FF2_PSE			(1 <<  3)	// page size extension
#define CPU_FF2_TSC			(1 <<  4)	// time stamp counter
#define CPU_FF2_MSR			(1 <<  5)	// model specific registers
#define CPU_FF2_PAE			(1 <<  6)	// page address extension (2M/4M pages)
#define CPU_FF2_MCE			(1 <<  7)	// machine check exception
#define CPU_FF2_CX8			(1 <<  8)	// CMPXCHG8 instruction
#define CPU_FF2_APIC		(1 <<  9)	// on-chip APIC hardware
#define CPU_FF2_BIT10		(1 << 10)	// reserved
#define CPU_FF2_SYSCALL		(1 << 11)	// AMD SYSCALL and SYSRET instructions
#define CPU_FF2_MTRR		(1 << 12)	// memory type region registers
#define CPU_FF2_PGE			(1 << 13)	// page global enable
#define CPU_FF2_MCA			(1 << 14)	// machine check architecture
#define CPU_FF2_CMOV		(1 << 15)	// conditional move instructions
#define CPU_FF2_PAT			(1 << 16)	// page attribute table
#define CPU_FF2_PSE36		(1 << 17)	// 36-bit page size extension
#define CPU_FF2_L2ECC		(1 << 18)	// ECC on the L2 cache
#define CPU_FF2_BIT19		(1 << 19)	// reserved
#define CPU_FF2_BIT20		(1 << 20)	// reserved
#define CPU_FF2_BIT21		(1 << 21)	// reserved
#define CPU_FF2_AMMX		(1 << 22)	// AMD mmx extentions (the KNI mmx instructions)
#define CPU_FF2_MMX			(1 << 23)	// mmx technology
#define CPU_FF2_FXSR		(1 << 24)	// extended fp save and restore
#define CPU_FF2_BIT25		(1 << 25)	// reserved
#define CPU_FF2_BIT26		(1 << 26)	// reserved
#define CPU_FF2_CLL			(1 << 27)	// cache-line locking (Cyrix only)
#define CPU_FF2_BIT28		(1 << 28)	// reserved
#define CPU_FF2_LM		    (1 << 29)	// long mode (64-bit mode)
#define CPU_FF2_3DNOWX		(1 << 30)	// extended 3DNow!
#define CPU_FF2_3DNOW		(1 << 31)	// 3DNow!

/****************** CPU Signatures ************************************/
/* These are the od/family/model/codes returned by CPUID-1 in eax     */
/* Revision is masked												  */
//--- Generic
#define	CPU_SIG_486			0x0400	// 80486
//--- Intel
#define	CPU_SIG_P4DX		0x0400	// i80486DX-25/33
#define	CPU_SIG_P4DX50		0x0410	// i80486DX-50
#define	CPU_SIG_P4SX		0x0420	// i80486SX
#define	CPU_SIG_P4DX2		0x0430	// i80486DX2 2x core clock
#define	CPU_SIG_P4SL		0x0440	// i80486SL
#define	CPU_SIG_P4SX2		0x0450	// i80486SX2
#define	CPU_SIG_P4DX2WB		0x0470	// i80486DX2WB WB cache
#define	CPU_SIG_P4DX4		0x0480	// i80486DX4
#define	CPU_SIG_P4DX4WB		0x0480	// i80486DX4WB
#define	CPU_SIG_P5A			0x0500	// P5 A-step
#define	CPU_SIG_P5			0x0510	// P5 - Socket 4
#define	CPU_SIG_P54C		0x0520	// P54C - Socket 5/7
#define	CPU_SIG_P5OD		0x1510	// P5 Overdrive
#define	CPU_SIG_P54COD		0x1520	// P54C Overdrive
#define	CPU_SIG_P5OD486		0x1530	// P5 Overdrive - 486 socket
#define	CPU_SIG_P5M			0x0570	// P54C Mobile
#define	CPU_SIG_P55C		0x0540	// P55C - first mmx
#define	CPU_SIG_P55COD		0x1540	// P55C Overdrive
#define	CPU_SIG_P55CM		0x0580	// P55C Mobile (250nm)
#define	CPU_SIG_P6A			0x0600	// PPro A-step
#define	CPU_SIG_P6			0x0610	// PPro
#define	CPU_SIG_P6OD		0x1630	// PPro Overdrive
#define	CPU_SIG_P2			0x0630	// P2
#define	CPU_SIG_P2XC		0x0650	// Xeon & Celeron
#define	CPU_SIG_CELA		0x0660	// Celeron-A/Mobile PII
#define	CPU_SIG_P3			0x0670	// P3
#define	CPU_SIG_P3CU		0x0680	// P3 Coppermine
#define	CPU_SIG_P3CUX		0x06A0	// P3 Coppermine Xeon
#define	CPU_SIG_P4_70		0x0700	// P4 Willamette
#define	CPU_SIG_P4_71		0x0710	// P4 Willamette
#define	CPU_SIG_P4_f0		0x0f00	// P4 Willamette pre-production
#define	CPU_SIG_P4_f1		0x0f10	// P4 Willamette pre-production
#define CPU_BRND_ID_NONE	0x00	// Brand ID not supported
#define CPU_BRND_ID_CEL		0x01	// Celeron
#define CPU_BRND_ID_P3		0x02	// PIII
#define CPU_BRND_ID_P3X		0x03	// PIII Xeon
//--- Amd
#define CPU_SIG_K4DX2		0x0430	// 80486DX2
#define CPU_SIG_K4DX2WB		0x0470	// 80486DX2
#define CPU_SIG_K4DX4		0x0480	// 80486DX2
#define CPU_SIG_K4DX4WB		0x0490	// 80486DX2
#define CPU_SIG_KX586		0x04E0	// 5x86
#define CPU_SIG_KX586WB		0x04F0	// 5x86WB
#define CPU_SIG_K5M0		0x0500	// K5 - Model 0
#define CPU_SIG_K5M1		0x0510	// K5 - Model 1
#define CPU_SIG_K5M2		0x0520	// K5 - Model 2
#define CPU_SIG_K5M3		0x0530	// K5 - Model 3
#define CPU_SIG_K6M6		0x0560	// K6 - Model 6
#define CPU_SIG_K6M7		0x0570	// K6 - Model 7
#define CPU_SIG_K62			0x0580	// K6-2 with 3DNow
#define CPU_SIG_K63			0x0590	// K6-3
#define CPU_SIG_K63_180		0x05D0	// K6-2/3 (180nm)
#define CPU_SIG_K7			0x0610	// Athlon (250nm)
#define CPU_SIG_K7A			0x0620	// Athlon (180nm)
#define CPU_SIG_K7D			0x0630	// Duron
#define CPU_SIG_K7T			0x0640	// Thunderbird
//--- Centaur
#define CPU_SIG_C6			0x0540	// Winchip
#define CPU_SIG_C62			0x0560	// Winchip 2 w/3DNow
#define CPU_SIG_C2			0x0580	// C2
#define CPU_SIG_C3			0x0590	// C3
//--- Cyrix
#define CPU_SIG_GX			0x0440	// MediaGX
#define CPU_SIG_GXM			0x0540	// MediaGX with MMX
#define CPU_SIG_M9			0x0500	// 5x86?? (I can't remember if CPUID was ever in these)
#define CPU_SIG_M1			0x0520	// 6x86
#define CPU_SIG_M2			0x0600	// 6x86MX and M2
#define CPU_SIG_MX			0x0620	// Cayenne bring-up core
#define CPU_SIG_M3			0x0650	// M3 (Gobi)
//--- Rise
//--- Transmetta


/******************************************************************************/
/* The rest of these #defines should only used locally by by GetCpuFeatures() */
// Cpu Signature, comes from eax after cpuid-1 
#define CPU_SIG_OVERDRIVE_POSITION	(12)
#define CPU_SIG_FAMILY_POSITION		( 8)
#define CPU_SIG_MODEL_POSITION		( 4)
#define CPU_SIG_REVISION_POSITION	( 0)
#define CPU_SIG_OVERDRIVE_MASK		(0x03 << CPU_SIG_OVERDRIVE_POSITION)	// overdrive bits
#define CPU_SIG_FAMILY_MASK			(0x0f << CPU_SIG_FAMILY_POSITION)		// family
#define CPU_SIG_MODEL_MASK			(0x0f << CPU_SIG_MODEL_POSITION)		// model
#define CPU_SIG_REVISION_MASK		(0x0f << CPU_SIG_REVISION_POSITION)		// revision
#define CPU_SIG_ALL_MASK			(CPU_SIG_OVERDRIVE_MASK | CPU_SIG_FAMILY_MASK | CPU_SIG_MODEL_MASK | CPU_SIG_REVISION_MASK)

// Brand Index, CFLCNT, APIC_ID come from ebx after cpuid-1, this started with the Coppermine (I think)
#define CPU_ID1EBX_APICID_POSITION	(24)
#define CPU_ID1EBX_CFLCNT_POSITION	( 8)
#define CPU_ID1EBX_BRNDIDX_POSITION	( 0)
#define CPU_ID1EBX_APICID_MASK		(0xff << CPU_ID1EBX_APICID_POSITION)	// APIC ID
#define CPU_ID1EBX_CFLCNT_MASK		(0xff << CPU_ID1EBX_CFLCNT_POSITION)	// CLFLUSH chunk count
#define CPU_ID1EBX_BRNDIDX_MASK		(0xff << CPU_ID1EBX_BRNDIDX_POSITION)	// Brand Index

// CR0 bit definitions
#define CPU_CR0_PE			(1 <<  0)	// protected mode enable
#define CPU_CR0_MP			(1 <<  1)	// monitor coprocessor 
#define CPU_CR0_EM			(1 <<  2)	// emulation (emulate fpu instructions)
#define CPU_CR0_TS			(1 <<  3)	// task switched (fpu state has not been saved yet)
#define CPU_CR0_ET			(1 <<  4)	// extension type (fpu supported), reserved on P5
#define CPU_CR0_NE			(1 <<  5)	// numeric error (internal mechanism handles fp errors)
#define CPU_CR0_WP			(1 << 16)	// write protect
#define CPU_CR0_AM			(1 << 18)	// alignment mask
#define CPU_CR0_NW			(1 << 29)	// not write-through (means "not write-back" on a 486)
#define CPU_CR0_CD			(1 << 30)	// cache disable
#define CPU_CR0_PG			(1 << 31)	// paging
// CR4 bit definitions
#define CPU_CR4_VME			(1 <<  0)	// virtual-8086 mode extentions
#define CPU_CR4_PVI			(1 <<  1)	// protected virtual interrupts
#define CPU_CR4_TSD			(1 <<  2)	// time stamp disable
#define CPU_CR4_DE			(1 <<  3)	// debugging extensions
#define CPU_CR4_PSE			(1 <<  4)	// page size extensions (4M pages)
#define CPU_CR4_PAE			(1 <<  5)	// physical address extensions (36-bit addresses)
#define CPU_CR4_MCE			(1 <<  6)	// machine-check enable
#define CPU_CR4_PGE			(1 <<  7)	// page global enable
#define CPU_CR4_PCE			(1 <<  8)	// performance-monitor counter enable
#define CPU_CR4_OSFXSR		(1 <<  9)	// operating system fpu extended save/restore support
#define CPU_CR4_OSXMMEXCPT	(1 << 10)	// operating system unmasked xmm exception support


/****************** Cache Descriptors ************************************/
/* These are the values returned with CPUID-2.  They describe the        */
/* different cache sizes.  		                                         */
#define CPU_CD_NULL			0x00	// empty cache descriptor
#define CPU_CD_ITLB4K_4x32	0x01
#define CPU_CD_ITLB4K_4x2	0x02
#define CPU_CD_DTLB4K_4x64	0x03
#define CPU_CD_DTLB4K_4x8	0x04
#define CPU_CD_IL1_8K_4x32	0x06
#define CPU_CD_IL1_16K_4x32	0x08
#define CPU_CD_DL1_8K_2x32	0x0a
#define CPU_CD_DL1_16K_4x32	0x0c
#define	CPU_CD_L2_0K		0x40
#define	CPU_CD_L2_128K_4x32	0x41
#define	CPU_CD_L2_256K_4x32	0x42
#define	CPU_CD_L2_512K_4x32	0x43
#define	CPU_CD_L2_1M_4x32	0x44
#define	CPU_CD_L2_2M_4x32	0x45
#define	CPU_CD_L2_4M_4x32	0x46	// not defined yet
#define	CPU_CD_L2_8M_4x32	0x47	// not defined yet
#define CPU_CD_TLB4K_4x32	0x70
#define CPU_CD_L1_16K_4x32	0x80	// unified L1 cache
#define	CPU_CD_L2_256K_8x32	0x82
#define	CPU_CD_L2_512K_8x32	0x83
#define	CPU_CD_L2_1M_8x32	0x84
#define	CPU_CD_L2_2M_8x32	0x85



/************************** Misc ****************************************/
/* Early versions of the VC++ compiler may not recognize these.         */
//#define cpuid __asm _emit 0x0f __asm _emit 0xa2 
//#define rdtsc __asm _emit 0x0f __asm _emit 0x31 


#endif //__CPU_H__
