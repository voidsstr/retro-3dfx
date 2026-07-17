/*
 * pcibrd.h
 *
 * pci board definition 
 *
 * This provides a flexible definition of what a board looks like to the
 * diag software.
 *
 */


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


 typedef struct tagCRS_32
{
   FxU32  Client_EDI ;
   FxU32  Client_ESI ;
   FxU32  Client_EBP ;
   FxU32  dwReserved_1 ;          // ESP at pushall
   FxU32  Client_EBX ;
   FxU32  Client_EDX ;
   FxU32  Client_ECX ;
   FxU32  Client_EAX ;
   FxU32  Client_Error ;          // DWORD error code
   FxU32  Client_EIP ;
   FxU16  Client_CS ;
   FxU16  wReserved_2 ;           // (padding)
   FxU32  Client_EFlags ;
   FxU32  Client_ESP ;
   FxU16  Client_SS ;
   FxU16  wReserved_3 ;           // (padding)
   FxU16  Client_ES ;
   FxU16  WReserved_4 ;           // (padding)
   FxU16  Client_DS ;
   FxU16  wReserved_5 ;           // (padding)
   FxU16  Client_FS ;
   FxU16  wReserved_6 ;           // (padding)
   FxU16  Client_GS ;
   FxU16  wReserved_7 ;           // (padding)

   FxU32  Client_Alt_EIP ;
   FxU16  Client_Alt_CS ;
   FxU16  wReserved_8 ;           // (padding)
   FxU32  Client_Alt_EFlags ;
   FxU32  Client_Alt_ESP ;
   FxU16  Client_Alt_SS ;
   FxU16  wReserved_9 ;           // (padding)
   FxU16  Client_Alt_ES ;
   FxU16  WReserved_10 ;          // (padding)
   FxU16  Client_Alt_DS ;
   FxU16  wReserved_11 ;          // (padding)
   FxU16  Client_Alt_FS ;
   FxU16  wReserved_12 ;          // (padding)
   FxU16  Client_Alt_GS ;
   FxU16  wReserved_13 ;          // (padding)
} CRS_32, *PCRS_32 ;

typedef struct tagCRS_16
{
   FxU16  Client_DI ;
   FxU16  wReserved_1 ;           // (padding)
   FxU16  Client_SI ;
   FxU16  wReserved_2 ;           // (padding)
   FxU16  Client_BP ;
   FxU16  wReserved_3 ;           // (padding)
   FxU32  dwReserved_4 ;          // ESP at pushall
   FxU16  Client_BX ;
   FxU16  wReserved_5 ;           // (padding)
   FxU16  Client_DX ;
   FxU16  wReserved_6 ;           // (padding)
   FxU16  Client_CX ;
   FxU16  wReserved_7 ;           // (padding)
   FxU16  Client_AX ;
   FxU16  wReserved_8 ;           // (padding)
   FxU32  dwReserved_9 ;          // DWORD error code
   FxU16  Client_IP ;
   FxU16  wReserved_10 ;          // (padding)
   FxU16  Client_CS ;
   FxU16  wReserved_11 ;          // (padding)
   FxU16  Client_Flags ;          // flags (loword)
   FxU16  wReserved_12 ;          // (padding)
   FxU16  Client_SP ;
   FxU16  wReserved_13 ;          // (padding)
   FxU16  Client_SS ;
   FxU16  wReserved_14 ;          // (padding)
   FxU16  Client_ES ;
   FxU16  wReserved_15 ;          // (padding)
   FxU16  Client_DS ;
   FxU16  wReserved_16 ;          // (padding)
   FxU16  Client_FS ;
   FxU16  wReserved_17 ;          // (padding)
   FxU16  Client_GS ;
   FxU16  wReserved_18 ;          // (padding)

   FxU16  Client_Alt_IP ;
   FxU16  wReserved_19 ;          // (padding)
   FxU16  Client_Alt_CS ;
   FxU16  wReserved_20 ;          // (padding)
   FxU16  Client_Alt_Flags ;
   FxU16  wReserved_21 ;          // (padding)
   FxU16  Client_Alt_SP ;
   FxU16  wReserved_22 ;          // (padding)
   FxU16  Client_Alt_SS ;
   FxU16  wReserved_23 ;          // (padding)
   FxU16  Client_Alt_ES ;
   FxU16  wReserved_24 ;          // (padding)
   FxU16  Client_Alt_DS ;
   FxU16  wReserved_25 ;          // (padding)
   FxU16  Client_Alt_FS ;
   FxU16  wReserved_26 ;          // (padding)
   FxU16  Client_Alt_GS ;
   FxU16  wReserved_27 ;          // (padding)
} CRS_16, *PCRS_16 ;

typedef struct tagCRS_8
{
   FxU16  Client_DI ;
   FxU16  wReserved_1 ;           // (padding)
   FxU16  Client_SI ;
   FxU16  wReserved_2 ;           // (padding)
   FxU16  Client_BP ;
   FxU16  wReserved_3 ;           // (padding)
   FxU32  dwReserved_4 ;          // ESP at pushall
   FxU8   Client_BL ;
   FxU8   Client_BH ;
   FxU16  wReserved_5 ;           // (padding)
   FxU8   Client_DL ;
   FxU8   Client_DH ;
   FxU16  wReserved_6 ;           // (padding)
   FxU8   Client_CL ;
   FxU8   Client_CH ;
   FxU16  wReserved_7 ;           // (padding)
   FxU8   Client_AL ;
   FxU8   Client_AH ;
   FxU16  wReserved_8 ;           // (padding)
   FxU32  dwReserved_9 ;          // DWORD error code
   FxU16  Client_IP ;
   FxU16  wReserved_10 ;          // (padding)
   FxU16  Client_CS ;
   FxU16  wReserved_11 ;          // (padding)
   FxU16  Client_Flags ;          // flags (loFxU16)
   FxU16  wReserved_12 ;          // (padding)
   FxU16  Client_SP ;
   FxU16  wReserved_13 ;          // (padding)
   FxU16  Client_SS ;
   FxU16  wReserved_14 ;          // (padding)
   FxU16  Client_ES ;
   FxU16  wReserved_15 ;          // (padding)
   FxU16  Client_DS ;
   FxU16  wReserved_16 ;          // (padding)
   FxU16  Client_FS ;
   FxU16  wReserved_17 ;          // (padding)
   FxU16  Client_GS ;
   FxU16  wReserved_18 ;          // (padding)

   FxU16  Client_Alt_IP ;
   FxU16  wReserved_19 ;          // (padding)
   FxU16  Client_Alt_CS ;
   FxU16  wReserved_20 ;          // (padding)
   FxU16  Client_Alt_Flags ;
   FxU16  wReserved_21 ;          // (padding)
   FxU16  Client_Alt_SP ;
   FxU16  wReserved_22 ;          // (padding)
   FxU16  Client_Alt_SS ;
   FxU16  wReserved_23 ;          // (padding)
   FxU16  Client_Alt_ES ;
   FxU16  wReserved_24 ;          // (padding)
   FxU16  Client_Alt_DS ;
   FxU16  wReserved_25 ;          // (padding)
   FxU16  Client_Alt_FS ;
   FxU16  wReserved_26 ;          // (padding)
   FxU16  Client_Alt_GS ;
   FxU16  wReserved_27 ;          // (padding)
} CRS_8, *PCRS_8 ;



typedef union tagCRS
{
    CRS_32 Regs32;
    CRS_16 Regs16;
    CRS_8  Regs8;
} CRS, *PCRS ;

typedef union _def_HwReg
{
    volatile FxU8  Reg008[1];
    volatile FxU16 Reg016[1];
    volatile FxU32 Reg032[1];
} HWREG, * PHWREG;
//

void DPMIRealModeInt(int intr, PCRS crs);
