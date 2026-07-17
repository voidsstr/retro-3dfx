/*
 * pcibrd.h
 *
 * pci board definition 
 *
 * This provides a flexible definition of what a board looks like to the
 * diag software.
 *
 */

#ifndef MDC_PCIBRD_H
#define MDC_PCIBRD_H

#include "3dfx.h"


// Data structures and data types
typedef struct tagREGION {
	FxU32	PhysAddr;	/* physical address base */
	FxU32   PhysSize;       /* size in bytes */
	FxU32	MappedAddr;	/* zero if unmapped */
	FxU32	MappedSize;	/* zero if unmapped */
} REGION;

typedef	REGION  *LPREGION;

typedef struct tagCARDINFO {
	int		vendorID;		// PCI Vendor ID
	int		deviceID;		// Device Index on PCI Bus
	FxU32           pciDevNum;              // PCI Device Number (bus/slot)
	int             OEM_version;            // OEM defined version information
	int		PCICntrl;	// Command Register
	int		PCIStatus;  // Status Register 
	FxU32		PCIBase0;	// Physical Base Address @10h 
	FxU32		PCIBase1;	// Physical Base Address @14h 
	FxU32		PCIBase2;	// Physical Base Address @18h 
	//up to 3 more PCI base addresses are possible
	REGION	NatMem0;		// Device Memory Region 0
	REGION	NatMem1;		// OEM defined Device Memory Region	1  
	REGION	NatMem2;		// OEM defined Device Memory Region	2  
	REGION	NatMem3;		// OEM defined Device Memory Region	3  
	REGION	NatMem4;		// Expansion ROM Region
	int		NumMemRegions;  // Number of mem regions defined for this card. 
	int		RegionMask;
	REGION	MemMapRegs0;  // Memory Mapped Regs Region 0 
	REGION	MemMapRegs1;  // Memory Mapped Regs Region 1 
	REGION	MemMapRegs2;  // Memory Mapped Regs Region 2 
	REGION	MemMapRegs3;  // Memory Mapped Regs Region 3 
	FxBool		InVGAMode;
	FxU16		Mode;				// Current Mode # 
} CARDINFO;
typedef	CARDINFO *LPCARDINFO;

void mdc_register_pcivendor(FxU32 vendorID, char *name);
void mdc_register_pcidevice(FxU32 deviceID, char *name);
void mdc_register_boardinitfn(FxU32 vendorId, FxU32 deviceID, int (*init_fn)(LPCARDINFO));
void mdc_print_boardinitfns(void);

LPCARDINFO mdc_init_default_board();

#endif
