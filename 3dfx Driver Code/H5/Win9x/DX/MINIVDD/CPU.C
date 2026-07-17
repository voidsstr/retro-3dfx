/* $Header: cpu.c, 3, 10/16/00 12:18:29 PM, Allen Hansen$ */
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
** File name:   cpu.c
**
** Description: cpu features detection and cpu specific functions
**
*/

#include "h3vdd.h"
#include "h3.h"
#include "cpu.h"



// DoCpuid() will store the cpu registers in this structure	(only used locally)
typedef struct _CPUID_REGS {
	DWORD	dwRegEAX;
	DWORD	dwRegEBX;
	DWORD	dwRegECX;
	DWORD	dwRegEDX;
} CPUID_REGS, *LPCPUID_REGS;


// Local function definitions
int HasCpuid();
void DoCpuid(DWORD mode, LPCPUID_REGS id_regs);
int fxstrncmp( char *string1, const char *string2, int count );
char *fxstrncpy( char *strDest, const char *strSource, int count );


#pragma VxD_LOCKED_DATA_SEG
/****************** Vendor ID strings *********************************/
const char	cpVendorId_Intel[]		= "GenuineIntel";
const char	cpVendorId_Amd[]	  	= "AuthenticAMD";
const char	cpVendorId_Cyrix[]		= "CyrixInstead";
const char	cpVendorId_Centaur[]  	= "CentaurHauls";
const char	cpVendorId_Rise[]	  	= "RiseRiseRise";
const char	cpVendorId_Transmetta[]	= "GenuineTMx86";


#pragma VxD_ICODE_SEG
#pragma VxD_LOCKED_CODE_SEG

/*****************************************************************************************************
******************************************************************************************************
*
*	This function reads and decodes most cpuid registers.  It stores the results in the 
*	CpuFeatures structure.
*
*	THIS MUST BE RAN AT RING ZERO!!!
*
******************************************************************************************************
*****************************************************************************************************/
DWORD GetCpuFeatures(LPCPU_FEATURES CpuFeatures)
{                
	DWORD			i;
	DWORD			maxlevels;	// max cpuid levels
	CPUID_REGS		id_regs;	// temporary cpuid registers
	unsigned char	cd[16];		// cache descriptor bits
	DWORD			reg_cr0, reg_cr4;


/*******************************************************************************************
*	Set everything to defaults first
*******************************************************************************************/
	CpuFeatures->dwCpuFlags				= 0;
	CpuFeatures->dwCpuType				= CPU_TYPE_UNKNOWN_CPU;
	CpuFeatures->dwFeatureFlags1		= 0;
	CpuFeatures->dwFeatureFlags2		= 0;
	CpuFeatures->dwProcessorSignature	= CPU_SIG_486;	// default to 486
	CpuFeatures->dwId1Ebx				= 0;
	CpuFeatures->dwSizeL1DCache			= 0;
	CpuFeatures->dwSizeL1ICache			= 0;
	CpuFeatures->dwSizeL2Cache			= 0;
	*CpuFeatures->cpVendorIdString		= 0;	// null termintate the string
	*CpuFeatures->cpCpuNameString		= 0;	// null termintate the string


/*******************************************************************************************
*	Read all the CPUID levels, figure out what we can that's not cpu specific.
*******************************************************************************************/
	if(HasCpuid())
	{
		CpuFeatures->dwCpuFlags |= CPU_FEATURE_CPUID;

		DoCpuid(0, &id_regs);
		maxlevels = id_regs.dwRegEAX;	// max number of cpuid levels (not including extended cpuid)
		*(DWORD*)(&CpuFeatures->cpVendorIdString[ 0]) = id_regs.dwRegEBX;
		*(DWORD*)(&CpuFeatures->cpVendorIdString[ 4]) = id_regs.dwRegEDX;
		*(DWORD*)(&CpuFeatures->cpVendorIdString[ 8]) = id_regs.dwRegECX;
		*(DWORD*)(&CpuFeatures->cpVendorIdString[12]) = 0;	// null termintate the string

		if(maxlevels >= 1)
		{
			DoCpuid(1, &id_regs);
			CpuFeatures->dwProcessorSignature	= id_regs.dwRegEAX;
			CpuFeatures->dwId1Ebx				= id_regs.dwRegEBX;
			CpuFeatures->dwFeatureFlags1		= id_regs.dwRegEDX;
		}

		// Read in the cache discriptors, used to differentiate P2, Celeron, Celeron-A, and Xeon
		// AMD doesn't use level 2, they use the extended CPUID
		// Didn't bother saving the TLB descriptors because they're really not very important.
		if(maxlevels >= 2) 
		{
			DoCpuid(2, &id_regs);
			*(DWORD*)(&cd[ 0])	= id_regs.dwRegEAX & 0xffffff00;	// al = the number of times we need
			*(DWORD*)(&cd[ 4])	= id_regs.dwRegEBX;					// to execute this level ... only
			*(DWORD*)(&cd[ 8])	= id_regs.dwRegECX;					// bothered doing it once.  Intel's 
			*(DWORD*)(&cd[12])	= id_regs.dwRegEDX;					// docs are unclear on this anyway.
			for(i=0; i < 16; ++i) 
			{
				switch (cd[i])
				{
					case CPU_CD_NULL:													break;	// ignore empty descriptor
					case CPU_CD_ITLB4K_4x32:											break;	// don't care about TLB
					case CPU_CD_ITLB4K_4x2:												break;	// don't care about TLB
					case CPU_CD_DTLB4K_4x64:											break;	// don't care about TLB
					case CPU_CD_DTLB4K_4x8:												break;	// don't care about TLB
					case CPU_CD_IL1_8K_4x32:	CpuFeatures->dwSizeL1ICache	= 1<<13;	break;
					case CPU_CD_IL1_16K_4x32:	CpuFeatures->dwSizeL1ICache	= 1<<15;	break;
					case CPU_CD_DL1_8K_2x32:	CpuFeatures->dwSizeL1DCache	= 1<<13;	break;
					case CPU_CD_DL1_16K_4x32:	CpuFeatures->dwSizeL1DCache	= 1<<15;	break;
					case CPU_CD_L2_0K:			CpuFeatures->dwSizeL2Cache	= 0;		break;
					case CPU_CD_L2_128K_4x32:	CpuFeatures->dwSizeL2Cache	= 1<<17;	break;
					case CPU_CD_L2_256K_4x32:	CpuFeatures->dwSizeL2Cache	= 1<<18;	break;
					case CPU_CD_L2_512K_4x32:	CpuFeatures->dwSizeL2Cache	= 1<<19;	break;
					case CPU_CD_L2_1M_4x32:		CpuFeatures->dwSizeL2Cache	= 1<<20;	break;
					case CPU_CD_L2_2M_4x32:		CpuFeatures->dwSizeL2Cache	= 1<<21;	break;
					case CPU_CD_L2_4M_4x32:		CpuFeatures->dwSizeL2Cache	= 1<<22;	break;
					case CPU_CD_L2_8M_4x32:		CpuFeatures->dwSizeL2Cache	= 1<<23;	break;
					case CPU_CD_TLB4K_4x32:												break;	// don't care about TLB
					case CPU_CD_L1_16K_4x32:	CpuFeatures->dwSizeL1ICache	= 
												CpuFeatures->dwSizeL1DCache	= 1<<13;	break;	// unified L1 cache
					case CPU_CD_L2_256K_8x32:	CpuFeatures->dwSizeL2Cache	= 1<<18;	break;
					case CPU_CD_L2_512K_8x32:	CpuFeatures->dwSizeL2Cache	= 1<<19;	break;
					case CPU_CD_L2_1M_8x32:		CpuFeatures->dwSizeL2Cache	= 1<<20;	break;
					case CPU_CD_L2_2M_8x32:		CpuFeatures->dwSizeL2Cache	= 1<<21;	break;
					default:															break;	// ignore unknown cd
				}
			}
		}
		// CPUID level 3 is the serial number: we're NOT reading that!


		// Extended cpuid stuff
		// NOTE:  Intel says they're adding this starting with Williamette (P4) but if you reference
		// their current docs they aren't compatible with AMD's extended cpuid.  The pre-production
		// P4's don't have extended cpuid so who knows what's going to happen.  Bleah!!!
		DoCpuid(0x80000000, &id_regs);
		maxlevels = id_regs.dwRegEAX; // max number of extended cpuid levels
		if( maxlevels >= 0x80000001 )
		{
			// get the extended feature flags
			DoCpuid(0x80000001, &id_regs);
			CpuFeatures->dwFeatureFlags2 = id_regs.dwRegEDX;

			// read the processor name string
			if(maxlevels >= 0x80000004)
			{
				DoCpuid(0x80000002, &id_regs);
				*(DWORD*)(&CpuFeatures->cpCpuNameString[ 0]) = id_regs.dwRegEAX;
				*(DWORD*)(&CpuFeatures->cpCpuNameString[ 4]) = id_regs.dwRegEBX;
				*(DWORD*)(&CpuFeatures->cpCpuNameString[ 8]) = id_regs.dwRegECX;
				*(DWORD*)(&CpuFeatures->cpCpuNameString[12]) = id_regs.dwRegEDX;
				DoCpuid(0x80000003, &id_regs);
				*(DWORD*)(&CpuFeatures->cpCpuNameString[16]) = id_regs.dwRegEAX;
				*(DWORD*)(&CpuFeatures->cpCpuNameString[20]) = id_regs.dwRegEBX;
				*(DWORD*)(&CpuFeatures->cpCpuNameString[24]) = id_regs.dwRegECX;
				*(DWORD*)(&CpuFeatures->cpCpuNameString[28]) = id_regs.dwRegEDX;
				DoCpuid(0x80000004, &id_regs);
				*(DWORD*)(&CpuFeatures->cpCpuNameString[32]) = id_regs.dwRegEAX;
				*(DWORD*)(&CpuFeatures->cpCpuNameString[36]) = id_regs.dwRegEBX;
				*(DWORD*)(&CpuFeatures->cpCpuNameString[40]) = id_regs.dwRegECX;
				*(DWORD*)(&CpuFeatures->cpCpuNameString[44]) = id_regs.dwRegEDX;
				CpuFeatures->cpCpuNameString[48] = 0;	// null termintate the string
			}

			// Get the TLB, L1, and L2 cache config data
			// This may already be done with CPUID level 2, so some data could get written twice.
			// We don't check for the data being different between the two because you wouldn't 
			// know which one is right anyway.  This method will probably change in a couple of 
			// years because this only supports a maximum L1 size of 255K.
			if(maxlevels >= 0x80000005)
			{
				DoCpuid(0x80000005, &id_regs);
				CpuFeatures->dwSizeL1DCache = (id_regs.dwRegECX >> 24) << 10;
				CpuFeatures->dwSizeL1ICache = (id_regs.dwRegEDX >> 24) << 10;
			}
			if(maxlevels >= 0x80000006)
			{
				DoCpuid(0x80000006, &id_regs);
				CpuFeatures->dwSizeL2Cache = (id_regs.dwRegECX >> 16) << 10;
			}
		}


	} // if(HasCpuid())
	else 
	{	// Check for a Cyrix with CPUID disabled ... do a divide instruction,
		// the flags will change on non-Cyrix parts
		_asm {
			push	DWORD PTR 0			// clear our flags if possible
			popfd
			pushfd
			pop		ecx					// ecx = cleared flags
			mov		eax, 5
			mov		edx, 2
			div		dl
			pushfd
			pop		eax					// eax = modified flags
			xor		eax, ecx			// if they changed, eax is non-zero
			mov		DWORD PTR i, eax	// use "i" for a flag
		}

		if (i == 0)
		{	
			CpuFeatures->dwProcessorSignature = CPU_SIG_M1;		// just default to the 6x86/6x86L
			fxstrncpy(CpuFeatures->cpVendorIdString, cpVendorId_Cyrix, 12);
			CpuFeatures->dwFeatureFlags1 |= CPU_FF1_FPU;
		}
	} // if(HasCpuid())


/*******************************************************************************************
*	Read cr0 and cr4.
*******************************************************************************************/
	_asm {
		// First read CR0.  We can do this from ring 3 using the smsw instruction
		smsw	ax
		and		eax, 0x0000ffff		// upper word is undefined, so mask if off
		mov		DWORD PTR reg_cr0, eax

		// CR4 is a lot harder to get to.  You can't read it from ring 3.  On top of that MS 
		// forgot to support CR4 in the MSVC compiler so we have to use these ugly emit's
		;mov eax, cr4
		__asm _emit 0x0f __asm _emit 0x20 __asm _emit 0xe0	// "mov eax,cr4"
		mov		DWORD PTR reg_cr4, eax
	}



/*******************************************************************************************
*	Figure out which features this cpu has.
*******************************************************************************************/
	// It's probably a longshot that this could be a problem, but if we're at 
	// ring 3 we need to check the TSD bit in CR4 before executing RDTSC.  I've
	// never seen this bit set but lets check it anyway.
	if((CpuFeatures->dwFeatureFlags1 & CPU_FF1_TSC) && !(reg_cr4 & CPU_CR4_TSD))
		CpuFeatures->dwCpuFlags |= CPU_FEATURE_TSC;

	// Conditional moves - only weirdness here was the K6's.  They had 2 flags in the 
	// extended cpuid (icmov and fcmov).  Neither one was ever set though, and the 
	// "split" cmov bits got redefined as a single cmov starting with the Athlon. 
	// Bottom line: ignore the old split cmov bits and just use the normal feature flag "cmov"
	if (CpuFeatures->dwFeatureFlags1 & CPU_FF1_CMOV)
		CpuFeatures->dwCpuFlags |= CPU_FEATURE_CMOV;

	// P6-style Memory Region Control Registers - AMD-K6 and Cyrix get done later
	if ((CpuFeatures->dwFeatureFlags1 & CPU_FF1_MSR) && (CpuFeatures->dwFeatureFlags1 & CPU_FF1_MTRR))
		CpuFeatures->dwCpuFlags |= CPU_FEATURE_P6_MTRR;


	/*******************************************************************************
	*	Figure out if this cpu supports SIMD instructions and if it's okay to use
	*	them.  There's some goofy stuff we need to check for fp, mmx, and sse.
	*******************************************************************************/
	// If FPU is emulated we can't do fpu, mmx, 3dnow, or sse because
	// any of those instructions causes an undefined opcode exception.
	if(! (reg_cr0 & CPU_CR0_EM))
	{
		if (CpuFeatures->dwFeatureFlags1 & CPU_FF1_FPU)		CpuFeatures->dwCpuFlags |= CPU_FEATURE_FPU;
		if (CpuFeatures->dwFeatureFlags1 & CPU_FF1_MMX)		CpuFeatures->dwCpuFlags |= CPU_FEATURE_MMX;
		if (CpuFeatures->dwFeatureFlags2 & CPU_FF2_AMMX)	CpuFeatures->dwCpuFlags |= CPU_FEATURE_AMMX;
		if (CpuFeatures->dwFeatureFlags2 & CPU_FF2_3DNOW)	CpuFeatures->dwCpuFlags |= CPU_FEATURE_3DNOW;
		if (CpuFeatures->dwFeatureFlags2 & CPU_FF2_3DNOWX)	CpuFeatures->dwCpuFlags |= CPU_FEATURE_3DNOWX;

		// Now we'll check for SSE.  The cpu must support the extended fp saves/restores.  
		// We must also check for OS support first by checking osfxsr (bit 9 in cr4).  
		// Remember we have to be at ring 0 to read cr4 so be carefull here.  This is not 
		// something we can blow off because Win95 doesn't support SSE!
		// Note: This doesn't check the osxmmexcpt bit (in CR4); you'll also need to do
		// this if you unmask SSE exceptions.
		if((CpuFeatures->dwFeatureFlags1 & CPU_FF1_FXSR) && (reg_cr4 & CPU_CR4_OSFXSR))
		{
			if (CpuFeatures->dwFeatureFlags1 & CPU_FF1_SSE)		CpuFeatures->dwCpuFlags |= CPU_FEATURE_SSE;
			if (CpuFeatures->dwFeatureFlags1 & CPU_FF1_SSE2)	CpuFeatures->dwCpuFlags |= CPU_FEATURE_SSE2;
		}
	}



/*******************************************************************************************
*	Enumerate the different processors and handle any processor specific stuff 
*******************************************************************************************/
	// Check for Intel
	if( 0 == fxstrncmp ((char *) &CpuFeatures->cpVendorIdString, cpVendorId_Intel, 12)) 
	{
		CpuFeatures->dwCpuFlags |= CPU_VENDOR_INTEL;

		if( (CpuFeatures->dwProcessorSignature & CPU_SIG_ALL_MASK) < 0x500 ) // all 486's
		{
			CpuFeatures->dwCpuType = CPU_TYPE_INTEL_486;						// Intel 486
		} 
		else 
		{
			switch (CpuFeatures->dwProcessorSignature & CPU_SIG_ALL_MASK) 
			{
				case CPU_SIG_P5A:
				case CPU_SIG_P5:
				case CPU_SIG_P5OD:
				case CPU_SIG_P5OD486:
					CpuFeatures->dwCpuType		= CPU_TYPE_INTEL_P5;			// Intel P5 (Original Pentium)
					CpuFeatures->dwSizeL1ICache	= 1<<13;
					CpuFeatures->dwSizeL1DCache	= 1<<13;
					CpuFeatures->dwSizeL2Cache	= 0;
					break;
				case CPU_SIG_P54C:
				case CPU_SIG_P54COD:
					CpuFeatures->dwCpuType		= CPU_TYPE_INTEL_P54C;			// Intel P54C (Socket 7)
					CpuFeatures->dwSizeL1ICache	= 1<<13;
					CpuFeatures->dwSizeL1DCache	= 1<<13;
					CpuFeatures->dwSizeL2Cache	= 0;
					break;
				case CPU_SIG_P55C:
				case CPU_SIG_P55COD:
				case CPU_SIG_P5M:
					CpuFeatures->dwCpuType		= CPU_TYPE_INTEL_P55C;			// Intel P55C (added MMX)
					CpuFeatures->dwSizeL1ICache	= 1<<15;
					CpuFeatures->dwSizeL1DCache	= 1<<15;
					CpuFeatures->dwSizeL2Cache	= 0;
					break;
				case CPU_SIG_P6A:
				case CPU_SIG_P6:
				case CPU_SIG_P6OD:
					CpuFeatures->dwCpuType		= CPU_TYPE_INTEL_P6;			// Intel P6 (Pentium Pro)
					break;
				case CPU_SIG_P2:
				case CPU_SIG_P2XC:
				case CPU_SIG_CELA:
					switch (CpuFeatures->dwSizeL2Cache) 
					{
						case 0: // No L2 cache
							CpuFeatures->dwCpuType = CPU_TYPE_INTEL_CELERON;	// Intel Celeron (Mendencino)
							break;
						case 1<<17:	// 128K
							CpuFeatures->dwCpuType = CPU_TYPE_INTEL_CELERON_A;	// Intel Celeron-A
							break;
						case 1<<18:	// 256K
						case 1<<19:	// 512K
							if(CpuFeatures->dwFeatureFlags1 & CPU_FF1_FXSR)
								CpuFeatures->dwCpuType = CPU_TYPE_INTEL_P2_DECH;// Intel PII Dechutes (250nm)
							else
								CpuFeatures->dwCpuType = CPU_TYPE_INTEL_P2;		// Intel PII (350nm)
							break;
						case 1<<20:	// 1M
						case 1<<21:	// 2M
						case 1<<22:	// 4M
						case 1<<23:	// 8M
							CpuFeatures->dwCpuType = CPU_TYPE_INTEL_P2_XEON;	// Intel PII Xeon
							break;
						default:
							CpuFeatures->dwCpuType = CPU_TYPE_INTEL_P2;			// some other Intel PII
							break;
					}
					break;
				case CPU_SIG_P3:
					if (CpuFeatures->dwSizeL2Cache >= (1<<20))	// 1M
						CpuFeatures->dwCpuType = CPU_TYPE_INTEL_P3_XEON;		// Intel PIII-Xeon (250nm)
					else					
						CpuFeatures->dwCpuType = CPU_TYPE_INTEL_P3;				// Intel PIII (250nm)
					break;
				case CPU_SIG_P3CU:
				case CPU_SIG_P3CUX:
					if (CpuFeatures->dwSizeL2Cache >= (1<<20))	// 1M
						CpuFeatures->dwCpuType = CPU_TYPE_INTEL_P3C_XEON;		// Intel PIII-Coppermine-Xeon (180nm)
					else
						CpuFeatures->dwCpuType = CPU_TYPE_INTEL_P3C;			// Intel PIII-Coppermine (180nm)
					break;
				case CPU_SIG_P4_70:
				case CPU_SIG_P4_71:
				case CPU_SIG_P4_f0:	//pre-production
				case CPU_SIG_P4_f1:	//pre-production
					CpuFeatures->dwCpuType = CPU_TYPE_INTEL_P4;					// Intel Willamette
					break;
				default:
					CpuFeatures->dwCpuType = CPU_TYPE_INTEL_UNKNOWN;			// Intel Unknown
					break;
			}
		}

		// Intel P6's have a write-buffer bug where stale data gets stuck in
		// them and we have to drain them periodically.  So we set the 
		// CPU_INTELP6 bit on Intel P6's.
		if ((CpuFeatures->dwProcessorSignature & CPU_SIG_FAMILY_MASK) == (6 << CPU_SIG_FAMILY_POSITION))
			CpuFeatures->dwCpuFlags |= CPU_INTELP6;

	} // done with Intel
	else // Check for AMD
	if(0 == fxstrncmp ((char *) &CpuFeatures->cpVendorIdString, cpVendorId_Amd, 12)) 
	{
		CpuFeatures->dwCpuFlags |= CPU_VENDOR_AMD;

		if( (CpuFeatures->dwProcessorSignature & CPU_SIG_ALL_MASK) < 0x500 )	// all 486's
		{
			if ((CpuFeatures->dwProcessorSignature & CPU_SIG_ALL_MASK) < CPU_SIG_KX586)
				CpuFeatures->dwCpuType = CPU_TYPE_AMD_486;					// AMD 486
			else
				CpuFeatures->dwCpuType = CPU_TYPE_AMD_5X86;					// AMD Am5x86
		} 
		else 
		{
			switch (CpuFeatures->dwProcessorSignature & CPU_SIG_ALL_MASK) 
			{
				case CPU_SIG_K5M0:
				case CPU_SIG_K5M1:
				case CPU_SIG_K5M2:
				case CPU_SIG_K5M3:
					CpuFeatures->dwCpuType		= CPU_TYPE_AMD_K5;			// AMD K5
					break;
				case CPU_SIG_K6M6:
				case CPU_SIG_K6M7:
					CpuFeatures->dwCpuType		= CPU_TYPE_AMD_K6;			// AMD K6
					break;
				case CPU_SIG_K62:
					CpuFeatures->dwCpuType		= CPU_TYPE_AMD_K6_2;		// AMD K6-2 (added 3dnow)
					break;
				case CPU_SIG_K63:
					CpuFeatures->dwCpuType		= CPU_TYPE_AMD_K6_3;		// AMD K6-3 (added backside L2)
					break;
				case CPU_SIG_K7:
				case CPU_SIG_K7A:
					CpuFeatures->dwCpuType		= CPU_TYPE_AMD_K7;			// AMD Athlon
					break;
				case CPU_SIG_K7D:
					CpuFeatures->dwCpuType		= CPU_TYPE_AMD_K7D;			// AMD Duron
					// AMD Duron Rev 0 bug fix: the L2 cache descriptor field means *64K instead of *1K
					if (CpuFeatures->dwProcessorSignature & CPU_SIG_REVISION_MASK == 0)
						CpuFeatures->dwSizeL2Cache *= 64;		
					break;
				case CPU_SIG_K7T:
					CpuFeatures->dwCpuType		= CPU_TYPE_AMD_K7T;			// AMD Thunderbird
					break;
				default:
					if (CpuFeatures->dwProcessorSignature & CPU_SIG_FAMILY_MASK < 6)
						// For forward compatibility (AMD may release other K6's)
						if (CpuFeatures->dwSizeL2Cache == 0)
							CpuFeatures->dwCpuType = CPU_TYPE_AMD_K6_2;		// AMD K6-2 (added 3dnow)
						else
							CpuFeatures->dwCpuType = CPU_TYPE_AMD_K6_3;		// AMD K6-3 (added backside L2)
					else if (CpuFeatures->dwProcessorSignature & CPU_SIG_FAMILY_MASK < 7)
						CpuFeatures->dwCpuType = CPU_TYPE_AMD_K7;			// AMD Athlons (or maybe K8???)
					else 
						CpuFeatures->dwCpuType = CPU_TYPE_AMD_UNKNOWN;		// AMD Unknown
					break;
			}

			// K6 MTRR check - the Athlon supports P6 style MTRR's (and the feature flags tell us this). 
			// But before that some K6's had their own proprietary MTRR's starting with model 5, rev 8
			if ( (CpuFeatures->dwProcessorSignature >= CPU_SIG_K62) && 
			     ((CpuFeatures->dwProcessorSignature & CPU_SIG_REVISION_MASK) >= 8) &&
			     (CpuFeatures->dwProcessorSignature < CPU_SIG_K7) )
				CpuFeatures->dwCpuFlags |= CPU_FEATURE_K6_MTRR;
		}
	} // done with AMD
	else // Check for Cyrix
	if(0 == fxstrncmp ((char *) &CpuFeatures->cpVendorIdString, cpVendorId_Cyrix, 12)) 
	{
		CpuFeatures->dwCpuFlags |= CPU_VENDOR_CYRIX;

		if( (CpuFeatures->dwProcessorSignature & CPU_SIG_ALL_MASK) < 0x440 )	// 486's
		{
			CpuFeatures->dwCpuType = CPU_TYPE_CYRIX_486;					// Cyrix 486
		}
		else if( (CpuFeatures->dwProcessorSignature & CPU_SIG_ALL_MASK) < 0x500 )
		{
			CpuFeatures->dwCpuType = CPU_TYPE_CYRIX_GX;						// Cyrix MediaGX
		}
		else
		{
			switch (CpuFeatures->dwProcessorSignature & CPU_SIG_ALL_MASK) 
			{
				case CPU_SIG_GXM:
					CpuFeatures->dwCpuType		= CPU_TYPE_CYRIX_GXM;		// Cyrix MediaGXm (added MMX)
					break;
				case CPU_SIG_M9:
					CpuFeatures->dwCpuType		= CPU_TYPE_CYRIX_5X86;		// Cyrix 5x86 (M9 core)
					break;
				case CPU_SIG_M1:
					CpuFeatures->dwCpuType		= CPU_TYPE_CYRIX_M1;		// Cyrix 6x86/L	(M1 core)
					CpuFeatures->dwSizeL1ICache	= 1<<14;
					CpuFeatures->dwSizeL1DCache	= 1<<14;
					CpuFeatures->dwSizeL2Cache	= 0;
					CpuFeatures->dwCpuFlags |= CPU_FEATURE_CYRIX_RCR;
					break;
				case CPU_SIG_M2:
					CpuFeatures->dwCpuType		= CPU_TYPE_CYRIX_M2;		// Cyrix 6x86MX / MII
					CpuFeatures->dwSizeL1ICache	= 1<<16;
					CpuFeatures->dwSizeL1DCache	= 1<<16;
					CpuFeatures->dwSizeL2Cache	= 0;
					CpuFeatures->dwCpuFlags |= CPU_FEATURE_CYRIX_RCR;
					// M2 rev 1 bug: MOVD r32,mmx is intermittant so just disable MMX
					if (CpuFeatures->dwProcessorSignature & CPU_SIG_REVISION_MASK == 1)
						CpuFeatures->dwCpuFlags &= ~CPU_FEATURE_MMX;
					break;
				case CPU_SIG_MX:
					CpuFeatures->dwCpuType		= CPU_TYPE_CYRIX_MX;		// Cyrix MX (integrated 3D, never released)
					CpuFeatures->dwCpuFlags |= CPU_FEATURE_CYRIX_RCR;
					break;
				case CPU_SIG_M3:
					CpuFeatures->dwCpuType		= CPU_TYPE_CYRIX_M3;		// Cyrix III
					CpuFeatures->dwCpuFlags |= CPU_FEATURE_CYRIX_RCR;
					break;
				default:
					CpuFeatures->dwCpuType		= CPU_TYPE_CYRIX_UNKNOWN;	// Cyrix Unknown
					break;
			}
		}
	} // done with Cyrix
	else // Check for Centaur (IDT)
	if(0 == fxstrncmp ((char *) &CpuFeatures->cpVendorIdString, cpVendorId_Centaur, 12)) 
	{
		CpuFeatures->dwCpuFlags |= CPU_VENDOR_CENTAUR;

		switch (CpuFeatures->dwProcessorSignature & CPU_SIG_ALL_MASK) 
		{
			case CPU_SIG_C6:
				CpuFeatures->dwCpuType		= CPU_TYPE_CENTAUR_C6;			// Centaur C6 (Winchip)
				break;
			case CPU_SIG_C62:
				CpuFeatures->dwCpuType		= CPU_TYPE_CENTAUR_C6_2;		// Centaur C6-2 (Winchip2)
				break;
			case CPU_SIG_C2:
				CpuFeatures->dwCpuType		= CPU_TYPE_CENTAUR_C2;			// Centaur C2
				break;
			case CPU_SIG_C3:
				CpuFeatures->dwCpuType		= CPU_TYPE_CENTAUR_C3;			// Centaur C3
				break;
			default:
				CpuFeatures->dwCpuType		= CPU_TYPE_CENTAUR_UNKNOWN;		// Centaur Unknown
				break;
		}
	} // done with Centaur
	else // Check for Rise
	if(0 == fxstrncmp ((char *) &CpuFeatures->cpVendorIdString, cpVendorId_Rise, 12)) 
	{
		// I don't know anything about these guys
		CpuFeatures->dwCpuFlags |= CPU_VENDOR_RISE;
		CpuFeatures->dwCpuType = CPU_TYPE_RISE_UNKNOWN;		// Rise Unknown
	} // done with Rise
	else // Check for Transmetta
	if(0 == fxstrncmp ((char *) &CpuFeatures->cpVendorIdString, cpVendorId_Transmetta, 12))
	{
		// I don't know anything about these guys either
		CpuFeatures->dwCpuFlags |= CPU_VENDOR_TRANSMETTA;
		CpuFeatures->dwCpuType = CPU_TYPE_TRANSMETTA_UNKNOWN;	// Transmetta Unknown
	} // done with Transmetta
	// end of processor specific 


	/* This bit is for backwards compatibility with our old cpuid code */
	if ((CpuFeatures->dwCpuFlags & CPU_INTELP6) && (CpuFeatures->dwCpuFlags & CPU_FEATURE_P6_MTRR) && (CpuFeatures->dwCpuFlags & CPU_FEATURE_SSE))
		CpuFeatures->dwCpuFlags |= P6_INTELCPU_WITH_KNI;
//    Here's the old code:
//    if (isP6 == (P6_INTELCPU_WITH_MTRRS | P6_INTELCPU_WITH_KNI))
//	   pHwInfo->cpuType |= P6_INTELCPU_WITH_KNI;


/*******************************************************************************************
*	Check if any debug registers are being used.  May be used to enable a different 
*	codepath to deture some of the lesser amature hackers using Softice.  This isn't
*	a very good check, if this check was done in self-generated code it would be better.
*	That's still not foolproof but it will slow some of them down.
*	I threw a bunch of BS instructions in here to try to confuse the amatures.
*******************************************************************************************/
	_asm {
		mov		ecx, dr3
		bswap	ecx					// BS
		mov		eax, dr1
		rol		eax, cl				// BS
		mov		edx, ecx
		mov		ecx, dr0
		rol		ecx, 12				// BS
		or		edx, eax			// edx=dr1|dr3
		mov		eax, dr2
		xor		ebx, ebx			// BS
		or		ecx, edx			// ecx=dr0|dr1|dr3
		mov		edx, eax			// eax=edx=dr2
		bswap	edx
		or		ax, dx				// ax=dr2
		mov		ebx, dr6
		setnz	dl					// dl.0 = dr2
		shl		edx, 25				// edx.25 = dr2
		mov		eax, dr7
		and		eax, 0xffff23ff		// mask unused bits
		and		bx, 0xe00f			// mask unused bits 0-15
		or		edx, ecx			// edx=dr0|dr1|dr2/dr3
		shr		ecx, 27				// BS
		shl		ebx, 16				// masks the rest of the dr6 (upper half)
		cmp		eax, 0xfff37fe3		// BS
		setge	cl					// BS
		cmp		eax, ebx			// dr6|dr7
		setnz	al					// al= dr6 != dr7 (a bit MUST be set)
		shl		cl, 3				// BS
		dec		ebx					// dr6-1
		shl		eax, 28				// ax.28 = dr6 != dr7 (a bit MUST be set)
		or		eax, edx			// eax=dr0|dr1|dr2/dr3| dr6 OR dr7 not zero
		rcr		edx, cl				// BS
		or		bh, bl
		dec		ebx					// dr6-2
		sets	cl					// BS but cl = 0 or 1
		add		ebx, 2
		bswap	ebx
		sub		edx, edx			// BS (edx=0)
		or		bh, bl
		shl		ebx, 16				// ebx.0-15 = 0,  ebx.15-31 = dr6
		xchg	eax, edx			// edx=dr0|dr1|dr2/dr3| dr6 OR dr7 not zero
		shr		ebx, cl				// shift right 0 or 1
		or		edx, ebx			// edx=all dr's OR'd together
		setnc	bl					// BS
		mov		cx, dx
		bswap	edx
		or		ch, dl
		rcr		edx, 8
		or		cl, dl				// cx=all dr's OR'd together
		test	dh, 4				// BS
		adc		ax, bx				// BS
		xor		edx, edx			// edx=0
		mov		ebx, CPU_DEBUG_RUNNING
		or		ch, cl				// cl=all dr's OR'd together
		setnz	al					// BS
		setz	dl					// debug ? 0 : 1
		shl		eax, 31				// BS
		dec		edx					// debug ? -1 : 0
		and		ebx, edx			// debug ? CPU_DEBUG_RUNNING : 0
		xor		cx, dx				// BS
		mov		i, ebx				// write the flag to i
		mov		cl, ch				// BS
		cmp		ch, 0				// BS
		jz		BS_JumpTarget		// BS
		cmp		maxlevels, 7		// BS
		jl		BS_JumpTarget		// BS
		dec		maxlevels			// BS
	BS_JumpTarget:
	}
	CpuFeatures->dwCpuFlags |= i;

	return CpuFeatures->dwCpuFlags;
}	// GetCpuFeatures()



/*******************************************************************************************
*	Local functions
*******************************************************************************************/

// Checks for CPUID support.  CPU should be at least a 386, I dunno what 
// happens on earlier CPU's but they won't be running protected mode (Windoz)
// anyway so I guess this is okay.
int HasCpuid()
{
	int supported = 0;

	_asm {
		push	ebx				; save ebx
		pushfd					; save flags
		pushfd
		pop		eax
		mov		ebx,eax			; save old flags
		xor		eax,(1 SHL 18)	; flip alignment bit (AC)
		push	eax
		popfd
		pushfd
		pop	eax
		cmp		ebx,eax			; did AC bit change?
		je		Done			; if AC didn't change, it's a 386 or less 

		mov		eax, ebx		; get origional flags again
		xor		eax,(1 SHL 21)	; flip id bit
		push	eax
		popfd
		pushfd
		pop	eax
		cmp		ebx,eax			; did ID bit change?
		je		Done			; if no change, CPUID is not supported
		mov		supported,1		; CPUID is supported

	Done:
		popfd					; restore origional flags
		pop		ebx
	}

	return supported;
}	// HasCpuid()



// Does a cpuid mode "mode" and loads the CPUID_REGS structure
// Assumes CPUID is available (CHECK FOR CPUID FIRST!!!)
void DoCpuid(DWORD mode, LPCPUID_REGS id_regs)
{
	_asm {
		// SAVE/RESTORE all the registers because 
		// some compilers forget to do this
		push	eax
		push	ebx
		push	ecx
		push	edx
		push	esi

		mov		eax, mode
		xor		ebx, ebx
		xor		ecx, ecx
		xor		edx, edx
		cpuid

		mov		esi, dword ptr [id_regs]
		mov		dword ptr [esi]CPUID_REGS.dwRegEAX, eax
		mov		dword ptr [esi]CPUID_REGS.dwRegEBX, ebx
		mov		dword ptr [esi]CPUID_REGS.dwRegECX, ecx
		mov		dword ptr [esi]CPUID_REGS.dwRegEDX, edx

		pop		esi
		pop		edx
		pop		ecx
		pop		ebx
		pop		eax
	}
}	// DoCpuid()




//	Just a copy of strncmp
//	Return Value Description 
//	< 0 string1 substring less than string2 substring 
//	0 string1 substring identical to string2 substring 
//	> 0 string1 substring greater than string2 substring 
int fxstrncmp( char *string1, const char *string2, int count )
{
	char *s1 = (char *) string1;
	char *s2 = (char *) string2;
	int i = count;

	while (i--)
	{
		if (*s1 < *s2)	
			return -1;
		else if (*s1 > *s2)
			return  1;
		++s1; 
		++s2;
	};

	return 0;
}

//	Just a copy of strncmp
//	Return Value = strDest
char *fxstrncpy( char *strDest, const char *strSource, int count )
{
	char *src = (char *) strSource;
	char *dst = (char *) strDest;
	int i = count;

	while (i-- && *src && *dst)
	{
		*src++ = *dst++;
	};
	*dst = 0;

	return strDest;
}

