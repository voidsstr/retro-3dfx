/*
  * banshee specific functions
 */

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "3dfx.h"
#include "fxpci.h"

#include "h3regs.h"
#include "h3defs.h"
#include "h3cinit.h"
#include "fxvid.h"
#include "fxhal.h"

#include "pcibrd.h"
#include "memtst.h"

#include <glide.h>

#include "banshee.h"
#include "crc.h"
#include "vgatst\ediag_ex.h"
#include "errrpt.h"
#include "misc.h"
#include "vgasim.h"
#include "vgacore.h"

//void (*cb_fn)(int frame) = 0;


// #include <env.h>

#define MIN(x,y) (x < y ? x : y)


#define PCI_3DFX_VENDORID     0x121A
#define PCI_BANSHEE_DEVICEID  0x0003

#define PCI_V3_2000_DEVICEID  0x0004
#define PCI_V3_3000_DEVICEID  0x0005
#define PCI_NAPALM_DEVICEID   0x0009

//#define BANSHEE_MMIO_SIZE     0x8000000
//#define BANSHEE_LFBMEM_SIZE   0x8000000

#define BANSHEE_MMIO_SIZE     0x2000000	// Use this for Napalm
#define BANSHEE_LFBMEM_SIZE   0x2000000

//#define BANSHEE_MMIO_SIZE     0x8000000  //0x4000000	//0x8	// Use this for 32Mb
//#define BANSHEE_LFBMEM_SIZE   0x8000000

#define BANSHEE_ROM_SIZE      (64 * (1024))     /* 64k */

SstIORegs *slaveioregs;
SstRegs *slavemm_regs;
SstCRegs *slavesstc;

int	slavevendorID;	// PCI Vendor ID
int	slavedeviceID;	// Device Index on PCI Bus
FxU32   slavepciDevNum;      // PCI Device Number (bus/slot)
int     slaveOEM_version;  // OEM defined version information
int	slavePCICntrl;	 // Command Register
int	slavePCIStatus;	 // Status Register 
FxU32	slavePCIBase0;	 // Physical Base Address @10h 
FxU32	slavePCIBase1;	 // Physical Base Address @14h 
FxU32	slavePCIBase2;	 // Physical Base Address @18h 
//up to 3 more PCI base addresses are possible
REGION	slaveNatMem0;	 // Device Memory Region 0
REGION	slaveNatMem1;	 // OEM defined Device Memory Region	1  
REGION	slaveNatMem2;	 // OEM defined Device Memory Region	2  
REGION	slaveNatMem3;	 // OEM defined Device Memory Region	3  
REGION	slaveNatMem4;	 // Expansion ROM Region
int	slaveNumMemRegions;  // Number of mem regions defined for this card. 
int	slaveRegionMask;
REGION	slaveMemMapRegs0;	// Memory Mapped Regs Region 0 
REGION	slaveMemMapRegs1;	// Memory Mapped Regs Region 1 
REGION	slaveMemMapRegs2;	// Memory Mapped Regs Region 2 
REGION	slaveMemMapRegs3;	// Memory Mapped Regs Region 3 
FxBool	slaveInVGAMode;
FxU16	slaveMode;		// Current Mode # 

void pci_write_dword(int pcireg, int device, FxU32 value);
FxU32 pci_read_dword(int pcireg, int device);


int init_banshee(LPCARDINFO card);

void init_banshee_module() {
  mdc_register_pcivendor(PCI_3DFX_VENDORID,"3dfx Interactive, Inc.");

  mdc_register_boardinitfn(PCI_3DFX_VENDORID,PCI_BANSHEE_DEVICEID, init_banshee);
  mdc_register_boardinitfn(PCI_3DFX_VENDORID,PCI_V3_2000_DEVICEID, init_banshee);
  mdc_register_boardinitfn(PCI_3DFX_VENDORID,PCI_V3_3000_DEVICEID, init_banshee);
  mdc_register_boardinitfn(PCI_3DFX_VENDORID,PCI_NAPALM_DEVICEID, init_banshee);

}

/*
 * this is the banshee init function, it should map the card number requested
 */

int init_banshee(LPCARDINFO card) {
  FxU32 base_addr;
  
  pciGetConfigData( PCI_BASE_ADDRESS_0, card->pciDevNum, &base_addr); 
  card->PCIBase0 = base_addr & ~0xF; /* MMIO */
  pciGetConfigData( PCI_BASE_ADDRESS_1, card->pciDevNum, &base_addr); 
  card->PCIBase1 = base_addr & ~0xF; /* FBMEM */
  pciGetConfigData( PCI_IO_BASE_ADDRESS, card->pciDevNum, &base_addr); 
  card->PCIBase2 = base_addr & ~0x1; /* IO SPACE */

  card->NatMem0.PhysAddr = card->NatMem0.MappedAddr = card->PCIBase0;
  card->NatMem1.MappedSize = card->NatMem0.PhysSize = BANSHEE_MMIO_SIZE;

  if (pciMapPhysicalToLinear(&card->NatMem0.MappedAddr, card->NatMem0.PhysAddr, 
			     &card->NatMem0.PhysSize) != FXTRUE) {
    card->NatMem0.MappedAddr = NULL;
    return (FXFALSE);
  }

  card->NatMem1.PhysAddr = card->NatMem1.MappedAddr = card->PCIBase1;
  card->NatMem1.MappedSize = card->NatMem1.PhysSize = BANSHEE_LFBMEM_SIZE;

  if (pciMapPhysicalToLinear(&card->NatMem1.MappedAddr, card->NatMem1.PhysAddr, 
			     &card->NatMem1.PhysSize) != FXTRUE) {
    card->NatMem1.MappedAddr = NULL;
    return (FXFALSE);
  }
  
  /*  card->OEM_version = */

  return FXTRUE;
}

int memfind(char *mem, int len,char *find_str) {
  int pos = 0;
  int find_len = strlen(find_str);

  len -= find_len;

  while (pos < len) {
    if (!strncmp(mem + pos, find_str, find_len)) {
      return pos;
    }
    pos++;
  }

  return -1; // didn't find it!
}

/*
   bansheeChecksumRom:

   This routine is used to return the BIOS version number. The entire
   BIOS ROM is searched for the string "Version" and the value after
   this is returned.
   
   This routine also checks the current BIOS checksum with the value
   passed in mb_bios_chk.  If mb_bios_chk = 0, then this check is not
   done. The parameter mb_bios_chk gets set based on the /cksm xxxx
   command line switch where xxxx is the expected checksum value in hex.
*/

FxBool bansheeChecksumRom(LPCARDINFO card, int mb_bios_cksm, char *version_str, int check_cksm, int *calc_mb_cksm) {
  int   version_loc,chksum,nSize,i;
  unsigned long biosseg;
  unsigned char *biosptr;
 
    biosseg = 0xC0000;

    /* setup pointer to ROM data */
    biosptr = (char far *) biosseg;
    nSize = *(biosptr + 2);		// Get the size of ROM
    nSize *= 512;			// Convert size to bytes

    if ((*(biosptr) != 0x55) || (*(biosptr + 1) != 0xAA)) {
      /* signature didn't match! */
      printf("Didn't find ROM signature 0x55, 0xAA at address C000:0000\n");
      return (FXFALSE);
    }

    /* get the Version number out! */
    version_str[0] = 0; // null the string to begin with!
    if ((version_loc = memfind(biosptr,nSize,"Version ")) > 0) {
      int copyright_loc = memfind(biosptr,nSize,"Copy");
      if (copyright_loc > 0) {
        strncpy(version_str,biosptr+version_loc,MIN(copyright_loc - version_loc,28)); // max size = 28
        version_str[MIN(copyright_loc - version_loc,28)] = 0;
      }
    }

    if (check_cksm == 1) {			/* Verify checksum? */
       /* checksum the rom */
       chksum = 0;
       for (i = 0; i < nSize; i++) {
         chksum += *(biosptr + i);
       }
       *calc_mb_cksm = chksum;			/* Return calculated checksum */
       if (chksum == mb_bios_cksm)   {
         printf("\nROM checksums matched (passed).\n", chksum);
       }
       else   {
         printf("\nERROR: ROM checksums did not match.");
         printf("\n       Checksum expected: 0x%04Xh  Checksum computed:0x%04Xh\n",mb_bios_cksm, chksum);
         return (FXFALSE);
       }
    }
    else if (check_cksm == 2) {		/* Calculate checksum? */
       /* checksum the rom */
       chksum = 0;
       for (i = 0; i < nSize; i++) 
         chksum += *(biosptr + i);
       printf("\nCalculated ROM checksum = 0x%04Xh.\n", chksum);
       *calc_mb_cksm = chksum;			/* Return checksum */
    }
    return FXTRUE;
}

/*
    bansheeCheckSerialRom:

    Verify that all the bits of the Auxiliary EEPROM which is
    used to store TV out data can be written to and read from.
    The contents of address 0 are saved and restored upon
    entry and exit.  Data patterns 0xAA and 0x55 are written
    to and read from address 0.

    I2C Address of Auxiliary EEPROM = 0xA0.

    This code is designed for the ATMEL 24C01A not the 24C01

    This test is only valid on boards which support TV out.

*/
FxBool bansheeCheckSerialRom(LPCARDINFO card) {
  SstIORegs *ioregs;
  unsigned char testdata,olddata;
  FxU32 mmio;
  unsigned char pIn[3];
  int rc,retry,error_exit;

  mmio = card->NatMem0.MappedAddr;
  ioregs = (SstIORegs *)mmio;
  VD3I2CInit((FxU32) ioregs);	// Initialize the I2C bus
  
// Save current setting of address 0 of the 24C01A
  retry = 5;
  error_exit = 0;
  while (retry > 0) {   

    pIn[0] = 0x00;
    rc = VD3I2cRead(0xA0,1,pIn,1,&olddata);
    if (rc) {
	retry--;			// decrement retry
	error_exit = 1;			// error occurred
    }
    else	{
       retry = 0;			// set retry to zero
       error_exit = 0;			// no error occurred
    }
  }
  if (error_exit)  {
     printf("\nError occurred reading Serial ROM data from AM24C01A.\n");
     return(FXFALSE);
  }

// Set contents of address 0 of the 24C01A = 0xAA
  retry = 5;
  error_exit = 0;
  while (retry > 0) {   

    pIn[0] = 0x00;
    pIn[1] = 0xAA;

    rc = VD3I2cWrite(0xA0,2,pIn,0,NULL);
    if (rc) {
	retry--;			// decrement retry
	error_exit = 1;			// error occurred
    }
    else	{
       retry = 0;			// set retry to zero
       error_exit = 0;			// no error occurred
    }
  }
  if (error_exit)  {
     printf("\nError occurred writing 0xAA to Serial ROM.\n");
     return(FXFALSE);
  }

// Read back what we just wrote
  retry = 5;
  error_exit = 0;
  while (retry > 0) {   
    pIn[0] = 0x00;
    rc = VD3I2cRead(0xA0,1,pIn,1,&testdata);
    if (rc) {
	retry--;			// decrement retry
	error_exit = 1;			// error occurred
    }
    else	{
       retry = 0;			// set retry to zero
       error_exit = 0;			// no error occurred
    }
  }
  if ((error_exit) || (testdata != 0xAA)) {		// If data read back doesn't match, error exit
     printf("\nError occurred reading Serial ROM data from AM24C01A.\nExpected: 0xAA   Read: 0x%0.2X\n",testdata);
     return(FXFALSE);
  }

// Set contents of address 0 of the 24C01A = 0x55
  retry = 5;
  error_exit = 0;
  while (retry > 0) {   

    pIn[0] = 0x00;
    pIn[1] = 0x55;

    rc = VD3I2cWrite(0xA0,2,pIn,0,NULL);
    if (rc) {
	retry--;			// decrement retry
	error_exit = 1;			// error occurred
    }
    else	{
       retry = 0;			// set retry to zero
       error_exit = 0;			// no error occurred
    }
  }
  if (error_exit)  {
     printf("\nError occurred writing 0x55 to Serial ROM.\n");
     return(FXFALSE);
  }

// Read back what we just wrote
  retry = 5;
  error_exit = 0;
  while (retry > 0) {   

    pIn[0] = 0x00;
    rc = VD3I2cRead(0xA0,1,pIn,1,&testdata);
    if (rc) {
	retry--;			// decrement retry
	error_exit = 1;			// error occurred
    }
    else	{
       retry = 0;			// set retry to zero
       error_exit = 0;			// no error occurred
    }
  }
  if ((error_exit) || (testdata != 0x55)) {		// If data read back doesn't match, error exit
     printf("\nError occurred reading Serial ROM data from AM24C01A.\nExpected: 0x55   Read: 0x%0.2X\n",testdata);
     return(FXFALSE);
  }

// Restore contents of address 0 of the 24C01A
  retry = 5;
  error_exit = 0;
  while (retry > 0) {   

    pIn[0] = 0x00;
    pIn[1] = olddata;

    rc = VD3I2cWrite(0xA0,2,pIn,0,NULL);
    if (rc) {
	retry--;			// decrement retry
	error_exit = 1;			// error occurred
    }
    else	{
       retry = 0;			// set retry to zero
       error_exit = 0;			// no error occurred
    }
  }
  if (error_exit)  {
     printf("\nError occurred restoring original contents of Serial ROM.\n");
     return(FXFALSE);
  }
  else  {
     printf("Serial ROM test passed.\n");
     return(FXTRUE);
  }

}	// End of bansheeCheckSerialRom


/*
   bansheeCheckVPD:

   This routine compares the assembly part number passed to it from the command line
   with value stored in the PCI Vital Product Data Area (VPD).  The VPD data is always
   located at address C000:00A0 in the Video BIOS.  First, location C000:00A3 is checked
   to make sure that the string "3Dfx VPD" is there. If not, then the BIOS does not
   support VPD.  Part numbers are then compared to make sure that they match.  If so,
   then the ECO level and serial number are returned.

   The suppress_msg variable is set so that this routine can be run with no messages
   output to the log file.  This was added so that the serial number and part number
   can be displayed on the top line of the output file to make it easier to parse.
   
   Below is a brief description of the contents:
     Offset        Item
     00A0          VPD Start
     00B1 to 00B7  Part Number   ( 7 Characters - Static)
     00BB to 00BD  ECO Level     ( 3 Characters - Static)
     00C1 to 00DE  Serial Number (30 Characters - Varies, LSB at 00DF)
     00E9          VPD End Tag
     00EA          VPD Checksum  ( 1 Byte) 

   More detailed description:

     00A0 82	Tag		           00B8 45 E	        
     00A1 08 Length		           00B9 43 C
     00A2 00 (word)			   00BA 03 Length(byte)
     00A3 33 3				   00BB 30 0
     00A4 44 D				     ....
     00A5 66 f				   00BD 30 0
     00A6 78 x				   00BE 53 S
     00A7 20 ' '			   00BF 4E N
     00A8 56 V				   00C0 1E Length(Byte)
     00A9 50 P				   00C1 20 ' '
     00AA 44 D				     ....
     00AB 90 VPD LR Tag		   00DE 20 ' '
     00AC 3D Length			   00DF 52 R
     00AD 00 (word)			   00E0 4C L
     00AE 50 P				   00E1 07 Length(byte)
     00AF 4E N				   00E2 20 ' '
     00B0 07 Length(byte)		     ....
     00B1 30 0				   00E8 20 ' '
      ....				   00E9 79 End Tag
     00B7 30 0				   00EA ?? Checksum

*/
FxBool bansheeCheckVPD(LPCARDINFO card, char *assembly_number, int assbly_size, char *serial_number, char *eco_level, int suppress_msg) {
  SstIORegs *ioregs = (SstIORegs *)(card->NatMem0.MappedAddr);
  FxU32 base_addr,old_base_addr;
  FxU32 phys_addr;
  FxU32 mapped_addr;
  char *ptr;
  FxBool result;
  int	size, assbly_pos;

  pciGetConfigData( PCI_ROM_BASE_ADDRESS, card->pciDevNum, &base_addr);
  old_base_addr = base_addr;

  /* we are going to steal address space from the MMIO registers */
  base_addr = card->PCIBase0 + 0x800000;
  phys_addr = base_addr & ~0x7FF;
  mapped_addr = card->NatMem0.MappedAddr + 0x800000; 
  ioregs->miscInit1 |= BIT(25);

  /* enable ROM address decoding */
  base_addr &= ~0x1;
  pciSetConfigData( PCI_ROM_BASE_ADDRESS, card->pciDevNum, &base_addr);
  base_addr |= 1;
  pciSetConfigData( PCI_ROM_BASE_ADDRESS, card->pciDevNum, &base_addr);

  /* check the VPD data */
  ptr = (char *)mapped_addr;
  assbly_pos = 0xA0;			/* VPD structure starts at location C000:00A0 */

  /* check that the BIOS supports VPD */
  if (strnicmp((ptr + assbly_pos+3), "3Dfx VPD", 8)) {
    if (suppress_msg) {
      printf("BIOS does not support PCI VPD data structure!\n");
      assembly_number = "Unknown";
    }
    return (FXFALSE);			    // strings did not match!
  }

  /* check that the part numbers match */
  if (!strncmp((ptr + assbly_pos+17), assembly_number, assbly_size))  {
    if (suppress_msg) {
      printf("Assembly part numbers matched (passed).\n");
    }
    result = FXTRUE;    // strings matched!
  }
  else	{
    if (suppress_msg) {
      printf("ERROR: Assembly part numbers do not match. Verify correct BIOS installed.\n");
      printf("Assembly part number should be %s\n",assembly_number);
      strncpy(assembly_number,(ptr+assbly_pos+17),assbly_size);
      printf("Assembly part number found is %s\n",assembly_number);
    }
    result = FXFALSE;   // strings didn't match!
  }

  /* At this point, return the eco level */
  eco_level[0] = 0;			// null the string to begin with!
  size	= *(ptr+assbly_pos+26);
  strncpy(eco_level,(ptr+assbly_pos+27), size);

  /* return the serial number */
  serial_number[0] = 0; 		// null the string to begin with!
  size = *(ptr+assbly_pos+32);
  strncpy(serial_number,(ptr+assbly_pos+33), size);

  return (result);
}

extern  int frame;

void bansheeFbiTest(LPCARDINFO card,void (*cbfn)(int frame)) {
  FxU32 fbiMemory;
  FxU32 memSize =  TO_MB(bansheeFbiSize(card));
  
  if (!memSize) {
    printf("Failed to detect memory size for Voodoo5 #%d\n",card->pciDevNum);
    return;
  }
  fbiMemory = card->NatMem1.MappedAddr;
  printf("Voodoo5FbiTest linear: 0x%.08X (mapped)\n",fbiMemory);
  MemoryPatternTests(fbiMemory, memSize, cbfn);
}

FxBool bansheeSliFbiTest(LPCARDINFO card) {
  FxU32 fbiMemory;
  FxU32 PCIDecode,strapInfo;
  FxU32 baseaddr,baseaddr0,baseaddr1,baseaddr2;
  FxU32 vgaInit0,cfgInitEnable;
  FxU32 memSize;
  FxU32 err_status=0;
  CRS   crs;
  int   masterpciDevNum;
  FxU32 mmio, slavemmio;
  SstIORegs *ioregs;
  SstRegs *mm_regs;
  FxBool retval = FXTRUE;

// Find the slave device on PCI bus
    crs.Regs8.Client_AH  = 0xB1;            // PCI_FUNCTION_ID
    crs.Regs8.Client_AL  = 0x02;            // FIND_PCI_DEVICE
    crs.Regs16.Client_CX = 0x09;            // Device ID = Voodoo5
    crs.Regs16.Client_DX = 0x121A;          // Vendor ID
    crs.Regs16.Client_SI = 0;               // Index
    DPMIRealModeInt(0x1A, &crs);
    if (crs.Regs8.Client_AH)   {
        printf("Cannot find ***Voodoo5***\n");
    }

    masterpciDevNum = crs.Regs16.Client_BX;         // Set function number = 0
    slavepciDevNum = crs.Regs16.Client_BX | 0x0001; // Set function number = 1

    // Set slave cfgInitEnable
    cfgInitEnable = pci_read_dword(0x40, masterpciDevNum);
    //Set bit 10 for enable updates to membase0,etc.
    pci_write_dword(0x40, slavepciDevNum, (cfgInitEnable | 0x0400));  // Update MMIO
    pci_write_dword(0x40, masterpciDevNum, (cfgInitEnable | 0x0400));  // Update MMIO
    cfgInitEnable = pci_read_dword(0x40, slavepciDevNum);

/********************************************************************/
/*    Set Master IO Decode and MemBase0 Decode to 0x100 and 32 MB   */
/********************************************************************/
    PCIDecode = pci_read_dword(0x48, masterpciDevNum);
    PCIDecode = PCIDecode & ~(SST_PCI_MEMBASE0_DECODE | SST_PCI_IOBASE0_DECODE);
    PCIDecode = PCIDecode | (SST_PCI_MEMBASE0_DECODE_32MB << SST_PCI_MEMBASE0_DECODE_SHIFT);
//    PCIDecode = PCIDecode | (SST_PCI_MEMBASE0_DECODE_64MB << SST_PCI_MEMBASE0_DECODE_SHIFT);
//    pci_write_dword(0x48, masterpciDevNum, PCIDecode);

    // Set FB Size to 2 * Physical Memory to test
//    PCIDecode = pci_read_dword(0x48, masterpciDevNum);
    PCIDecode = PCIDecode & ~(SST_PCI_MEMBASE1_DECODE);
//    PCIDecode = PCIDecode | (SST_PCI_MEMBASE1_DECODE_32MB << SST_PCI_MEMBASE1_DECODE_SHIFT);
    PCIDecode = PCIDecode | (SST_PCI_MEMBASE1_DECODE_64MB << SST_PCI_MEMBASE1_DECODE_SHIFT);
    pci_write_dword(0x48, masterpciDevNum, PCIDecode);

/*******************************************************************/
/*    Set Slave IO Decode and MemBase0 Decode to 0x100 and 32 MB   */
/*******************************************************************/
    PCIDecode = pci_read_dword(0x48, slavepciDevNum);
    PCIDecode = PCIDecode & ~(SST_PCI_MEMBASE0_DECODE | SST_PCI_IOBASE0_DECODE);
    PCIDecode = PCIDecode | (SST_PCI_MEMBASE0_DECODE_32MB << SST_PCI_MEMBASE0_DECODE_SHIFT);
//    PCIDecode = PCIDecode | (SST_PCI_MEMBASE0_DECODE_64MB << SST_PCI_MEMBASE0_DECODE_SHIFT);
//    pci_write_dword(0x48, slavepciDevNum, PCIDecode);

    // Set FB Size to 2 * Physical Memory to test Tiled Memory
//    PCIDecode = pci_read_dword(0x48, slavepciDevNum);
    PCIDecode = PCIDecode & ~(SST_PCI_MEMBASE1_DECODE);
//    PCIDecode = PCIDecode | (SST_PCI_MEMBASE1_DECODE_32MB << SST_PCI_MEMBASE1_DECODE_SHIFT);
    PCIDecode = PCIDecode | (SST_PCI_MEMBASE1_DECODE_64MB << SST_PCI_MEMBASE1_DECODE_SHIFT);
    pci_write_dword(0x48, slavepciDevNum, PCIDecode);

/*************************************************************************/
// At this point, give some of the resources to the slave.  Currently,
// the two-way PCI boards ask for 256 MB Membase0, Membase1 and 512 bytes
// IO space.  We will place the devices 128MB, 128MB, 256 bytes apart.
// The final strapping options are still in doubt, so this needs to be
// modified in the future to take this into account!
/*************************************************************************/

    // Set slave PCI Base 0 address
    baseaddr = pci_read_dword(0x10, masterpciDevNum);
printf("Master PCIBase 0 = %X\n",baseaddr);
//    baseaddr +=0x14000000L;
//    baseaddr +=0x10000000L;
    baseaddr += 0x8000000L;    		// Place slave 128MB apart
//test was       baseaddr +=0x18000000L;
    pci_write_dword(0x10, slavepciDevNum, baseaddr);  // Update MMIO
    slavePCIBase0 = pci_read_dword(0x10, slavepciDevNum);
    slavePCIBase0 &= ~0xF;
printf("slavePCIBase 0 = %X\n",slavePCIBase0);

    // Set slave PCI Base 1 address
    baseaddr = pci_read_dword(0x14, masterpciDevNum);
printf("Master PCIBase 1 = %X\n",baseaddr);
    baseaddr += 0x8000000L;    		// Place slave 128MB apart

// tony crap comeon man 
    //baseaddr += 0x10000000L;    		// Place slave 256MB apart
    pci_write_dword(0x14, slavepciDevNum, baseaddr);  // Update FBMEM
    slavePCIBase1 = pci_read_dword(0x14, slavepciDevNum);  
    slavePCIBase1 &= ~0xF;
printf("slavePCIBase 1 = %X\n",slavePCIBase1);

    // Set slave PCI Base 2 address
    baseaddr = pci_read_dword(0x18, masterpciDevNum);
printf("Master PCIBase 2 = %X\n",baseaddr);

// tony crap comeon man 

    baseaddr += 0x200L;    		// Place slave 512 bytes apart
//    baseaddr += 0x100L;    		// Place slave 256 bytes apart
    baseaddr &= ~0x1;
    pci_write_dword(0x18, slavepciDevNum, baseaddr);  // Update IO Space
    slavePCIBase2 = pci_read_dword(0x18, slavepciDevNum);
    slavePCIBase2 &= ~0x1;
printf("slavePCIBase 2 = %X\n",slavePCIBase2);

//    cfgInitEnable &= ~(0x0400);  //Reset bit 10 to disable updates to membase0,etc.
//    pci_write_dword(0x40, slavepciDevNum, cfgInitEnable);  // Update MMIO

  slaveNatMem0.PhysAddr = slaveNatMem0.MappedAddr = slavePCIBase0;
  slaveNatMem1.MappedSize = slaveNatMem0.PhysSize = BANSHEE_MMIO_SIZE;

  if (pciMapPhysicalToLinear(&slaveNatMem0.MappedAddr, slaveNatMem0.PhysAddr, 
			     &slaveNatMem0.PhysSize) != FXTRUE) {
    slaveNatMem0.MappedAddr = NULL;
    printf("error converting pciMapPhystoLin for slave!\n");
  }

  slaveNatMem1.PhysAddr = slaveNatMem1.MappedAddr = slavePCIBase1;
  slaveNatMem1.MappedSize = slaveNatMem1.PhysSize = BANSHEE_LFBMEM_SIZE;

  if (pciMapPhysicalToLinear(&slaveNatMem1.MappedAddr, slaveNatMem1.PhysAddr, 
			     &slaveNatMem1.PhysSize) != FXTRUE) {
    slaveNatMem1.MappedAddr = NULL;
    printf("error converting pciMapPhystoLin for slave (2nd)!\n");
  }

// Enable Memory and I/O addressing on slave device
  pci_write_dword(0x04, slavepciDevNum, 0x00000003);  // Update command register

// Point to Master registers
  mmio = card->NatMem0.MappedAddr;	// point to master IO registers
  ioregs = (SstIORegs *)mmio;
  mmio = card->NatMem0.MappedAddr + SST_3D_OFFSET;
  mm_regs = (SstRegs *)mmio;

// Point to Slave registers
  slavemmio = slaveNatMem0.MappedAddr;
  slaveioregs = (SstIORegs *)slavemmio;

  slavemmio = slaveNatMem0.MappedAddr + SST_3D_OFFSET;
  slavemm_regs = (SstRegs *)slavemmio;

  slavemmio = slaveNatMem0.MappedAddr + SST_CMDAGP_OFFSET;
  slavesstc = (SstCRegs *)slavemmio;

// Initialize slave registers based on master settings
  slaveioregs->dramInit0 = ioregs->dramInit0;
  slaveioregs->dramInit1 = ioregs->dramInit1;
printf("Master dramInit0 = %X\n",ioregs->dramInit0);
printf("Slave dramInit0 = %X\n",slaveioregs->dramInit0);
printf("Master dramInit1 = %X\n",ioregs->dramInit1);
printf("Slave dramInit1 = %X\n\n",slaveioregs->dramInit1);

  memSize = TO_MB(GetMemSize(slaveioregs->dramInit0, slaveioregs->dramInit1));	/* Returns memory size in MB */
  if (!memSize) {
    printf("Slave dramInit0 = %X, dramInit1 = %X \n",slaveioregs->dramInit0, slaveioregs->dramInit1);
    printf("Failed to detect memory size for Voodoo5 slave device \n");
    // Disable Memory and I/O addressing on slave device before exiting
    pci_write_dword(0x04, slavepciDevNum, 0x00000000);  // Update command register
    return FXFALSE;
  }
  else
    printf("Slave Memory Size = %dMB\n",memSize/(1024*1024));

  slaveioregs->pciInit0 = ioregs->pciInit0;
  slaveioregs->lfbMemoryConfig = ioregs->lfbMemoryConfig;
  slaveioregs->miscInit0 = ioregs->miscInit0;
  slaveioregs->miscInit1 = ioregs->miscInit1;
  slaveioregs->agpInit = ioregs->agpInit;
  slaveioregs->pllCtrl1 = ioregs->pllCtrl1;
  slaveioregs->pllCtrl2 = ioregs->pllCtrl2;
  slaveioregs->tmuGbeInit = ioregs->tmuGbeInit;
  vgaInit0 = ioregs->vgaInit0;
  vgaInit0 |= 0x01;			// Disable VGA decoding
  slaveioregs->vgaInit0 = vgaInit0;
  slaveioregs->vgaInit1 = ioregs->vgaInit1;
  slaveioregs->dramData = 0x37;
  slaveioregs->dramCommand = 0x10d;

//ioregs->reservedZ[0] = 0x0;
//strapInfo = ioregs->reservedZ[0];
//printf("master strapInfo[0] = %X\n",strapInfo);
//ioregs->reservedZ[0] = 0x1;
//strapInfo = ioregs->reservedZ[0];
//printf("master strapInfo[1] = %X\n",strapInfo);

//slaveioregs->reservedZ[0] = 0x0;
//strapInfo = slaveioregs->reservedZ[0];
//printf("slave strapInfo[0] = %X\n",strapInfo);
//slaveioregs->reservedZ[0] = 0x1;
//strapInfo = slaveioregs->reservedZ[0];
//printf("slave strapInfo[1] = %X\n",strapInfo);

  slavemm_regs->lfbMode = mm_regs->lfbMode & (~SST_LFB_ENPIXPIPE);

  slaveioregs->lfbMemoryConfig = 0x1FFF; //0x03FFF; /* turn off tiled memory */

/* Map slave into master    */
    baseaddr0 = pci_read_dword(0x10, masterpciDevNum);
    baseaddr1 = pci_read_dword(0x14, masterpciDevNum);
    baseaddr2 = pci_read_dword(0x18, masterpciDevNum);
    baseaddr2 &= ~0x1;

    // Disable Memory and I/O addressing on master device
    pci_write_dword(0x04, masterpciDevNum, 0x00000000);  // Update command register

    pci_write_dword(0x10, slavepciDevNum, baseaddr0);  // Update MMIO
    pci_write_dword(0x14, slavepciDevNum, baseaddr1);  // Update FBMEM
    pci_write_dword(0x18, slavepciDevNum, baseaddr2);  // Update IO Space

    // Enable Memory and I/O addressing on slave device
    pci_write_dword(0x04, slavepciDevNum, 0x00000003);  // Update command register

  fbiMemory = baseaddr1 & ~(0xF);
  printf("slave Voodoo5FbiTest linear: 0x%.08X (mapped)\n",fbiMemory);

/* End of map slave into master    */
  err_status = MemoryPatternTests(fbiMemory, memSize, 0);
  if (err_status)
     retval = FXFALSE;
  else
     retval = FXTRUE;

/* Map slave into master    */

    pci_write_dword(0x10, masterpciDevNum, baseaddr0);  // Update MMIO
    pci_write_dword(0x14, masterpciDevNum, baseaddr1);  // Update FBMEM
    pci_write_dword(0x18, masterpciDevNum, baseaddr2);  // Update IO Space

    // Enable Memory and I/O addressing on slave device
    pci_write_dword(0x04, masterpciDevNum, 0x00000003);  // Update command register

/* End of map slave into master    */
// Now Disable Memory and I/O addressing on slave device
  pci_write_dword(0x04, slavepciDevNum, 0x00000000);  // Update command register
  cfgInitEnable &= ~(0x0400);  //Reset bit 10 to disable updates to membase0,etc.
  pci_write_dword(0x40, slavepciDevNum, cfgInitEnable);  // Update MMIO

  return (retval);
}


FxU32 MemoryPatternTests(FxU32 fbiMemory,FxU32 memSize, void (*cbfn)(int frame))
{
  FxU32 err_count=0;
  FxU32 err_status=0;

  err_count = constFill((FxU32 *)fbiMemory, memSize, 0x0);
  if (cbfn) {
	frame=1;
	cbfn(frame);
  }
  else if (err_count)
     err_status |= 1;

  err_count = alternateFill((FxU32 *)fbiMemory, memSize, 0, 0xFFFFFFFF);
  if (cbfn) {
	frame=2;
	cbfn(frame);
  }
  else if (err_count)
     err_status |= 1;

  err_count = alternateFill((FxU32 *)fbiMemory, memSize, 0xFFFFFFFF,0);
  if (cbfn) {
	frame=3;
	cbfn(frame);
  }
  else if (err_count)
     err_status |= 1;

  err_count = constFill((FxU32 *)fbiMemory, memSize, 0xAA55AA55);
  if (cbfn) {
	frame=4;
	cbfn(frame);
  }
  else if (err_count)
     err_status |= 1;

  err_count = constFill((FxU32 *)fbiMemory, memSize, 0x55AA55AA);
  if (cbfn) {
	frame=5;
	cbfn(frame);
  }
  else if (err_count)
     err_status |= 1;

  err_count = alternateFill((FxU32 *)fbiMemory, memSize, 0xAA55AA55, 0x55aa55aa);
  if (cbfn) {
	frame=6;
	cbfn(frame);
  }
  else if (err_count)
     err_status |= 1;

  err_count = alternateFill((FxU32 *)fbiMemory, memSize, 0x55aa55aa, 0xaa55aa55);
  if (cbfn) {
	frame=7;
	cbfn(frame);
  }
  else if (err_count)
     err_status |= 1;

  err_count = alternateFill((FxU32 *)fbiMemory, memSize, 0, 0xFFFFFFFF);
  if (cbfn) {
	frame=2;
	cbfn(frame);
  }
  else if (err_count)
     err_status |= 1;

  err_count = alternateFill((FxU32 *)fbiMemory, memSize, 0xFFFFFFFF,0);
  if (cbfn) {
	frame=3;
	cbfn(frame);
  }
  else if (err_count)
     err_status |= 1;

  err_count = alternateFill((FxU32 *)fbiMemory, memSize, 0, 0xFFFFFFFF);
  if (cbfn) {
	frame=2;
	cbfn(frame);
  }
  else if (err_count)
     err_status |= 1;

  err_count = alternateFill((FxU32 *)fbiMemory, memSize, 0xFFFFFFFF,0);
  if (cbfn) {
	frame=3;
	cbfn(frame);
  }
  else if (err_count)
     err_status |= 1;

return (err_status);
  /*  walkingFill((FxU32 *)fbiMemory, memSize, 0x55AA55AA); */ /* this is VERY slow */

}

void b2d_setlocation(struct location_struct *l, int x, int y) {
  l->x = x; 
  l->y = y;
}

struct blit2d_params_struct current_2dblit;

void b2d_setdstsize(int w, int h) {
  b2d_setlocation(&(current_2dblit.dst_size),w,h);
}

void b2d_setsrcsize(int w, int h) {
  b2d_setlocation(&(current_2dblit.src_size),w,h);
}
void b2d_setdest(int x, int y) {
  b2d_setlocation(&(current_2dblit.dst),x,y); 
}
void b2d_setsrc(int x, int y) {
  b2d_setlocation(&(current_2dblit.src),x,y);
}

void b2d_setcolor(FxU32 color) {
  current_2dblit.colorFore = color;
}

void bansheeWaitForVsync(LPCARDINFO card,int state) {
  SstRegs *sstregs;
  FxU32 mmio;
  int a=1, b=2;

  mmio = card->NatMem0.MappedAddr + SST_3D_OFFSET;
  sstregs = (SstRegs *)mmio;

  if (state) {
     while ((sstregs->status & SST_VRETRACE)) {
       a = b; b = a;
     }
  } else {
     while (!(sstregs->status & SST_VRETRACE)) {
       a = b; b = a;
     }
  }

}

FxU32 bansheeCrc(LPCARDINFO card) {
 SstCRegs *sstc = (SstCRegs *)((FxU32)card->NatMem0.MappedAddr + SST_CMDAGP_OFFSET);
 SstIORegs *ioregs = (SstIORegs *)(card->NatMem0.MappedAddr);
 FxU32 lastcrc=0, crc=1;
 int timeout = 200;
 int match_count = 0;
 int x = 0;
 FxU32 dacMode;

 /* make sure we are only seeing the screen */

 dacMode = ioregs->dacMode;
 dacMode = (dacMode & ~(1<<5));
 ioregs->dacMode = dacMode;

/*
  printf("MappedAddr = %X, cmdagpAddr = %X, crc2 addr = %x\n",
	card->NatMem0.MappedAddr,
	sstc,&(sstc->crc2));
 */
 
 while (timeout--) {
   crc = GET(sstc->crc2);

   for (x=0;x<10000;x++) {
      // do nothing!!
   }

   if (crc == lastcrc) {
      match_count++;
   } else {
      match_count--;
   }
   lastcrc = crc;

   if (match_count > 10) {
      // printf("B0 CRC reg = %X\n",crc);
      return (crc);
   }
 }

 return 0;

}

FxU32 bansheeMemCrc(LPCARDINFO card) {
 SstCRegs *sstc = (SstCRegs *)((FxU32)card->NatMem0.MappedAddr + SST_CMDAGP_OFFSET);
 SstIORegs *ioregs = (SstIORegs *)(card->NatMem0.MappedAddr);
 FxU32 lastcrc=0, crc=1;
 int timeout = 200;
 int match_count = 0;
 int mismatch_count = 0;
 FxU32 dacMode;

 /* make sure we are only seeing the screen */

 dacMode = ioregs->dacMode;
 dacMode = (dacMode & ~(1<<5));
 ioregs->dacMode = dacMode;

/*
  printf("MappedAddr = %X, cmdagpAddr = %X, crc2 addr = %x\n",
	card->NatMem0.MappedAddr,
	sstc,&(sstc->crc2));
 */
 while (timeout) {
   crc = GET(sstc->crc2);

   if (crc == lastcrc) {
      match_count++;
   } else {
      mismatch_count++;
   }
   lastcrc = crc;

   if (match_count > 10000) {
     //  printf("B0 CRC reg = %X\n",crc);
      return (crc);
   }
   else {
     if (mismatch_count > 1) {
 	printf("mmc=%d mc=%d\n",mismatch_count,match_count);
	return(0);
     }
   }	
 }
 return 0;
}

FxU32 bansheeSliMemCrc(LPCARDINFO card) {
 FxU32 lastcrc=0, crc=1;
 int timeout = 200;
 int match_count = 0;
 int mismatch_count = 0;
 FxU32 dacMode;

 /* make sure we are only seeing the screen */
 dacMode = slaveioregs->dacMode;
 dacMode = (dacMode & ~(1<<5));
 slaveioregs->dacMode = dacMode;

 while (timeout) {
   crc = GET(slavesstc->crc2);

   if (crc == lastcrc) {
      match_count++;
   } else {
      mismatch_count++;
   }
   lastcrc = crc;

   if (match_count > 10000) {
     //  printf("B0 CRC reg = %X\n",crc);
      return (crc);
   }
   else {
     if (mismatch_count > 1) {
 	printf("mmc=%d mc=%d\n",mismatch_count,match_count);
	return(0);
     }
   }	
 }
 return 0;
}


FxU32 bansheeHWCursor(LPCARDINFO card,int en,int xLoc, int yLoc) {
  SstIORegs *ioregs = (SstIORegs *)(card->NatMem0.MappedAddr);
  FxU32 vidProcCfg;


  // set pattern address, we're just setting it to 0
  ioregs->hwCurPatAddr  =  0;

  // set cursor location on screen
  ioregs->hwCurLoc = (yLoc << 16) | xLoc;

  // set colors C0 and C1 for cursor
  ioregs->hwCurC0 = 0xA5A5A5;
  ioregs->hwCurC1 = 0x5a5a5a;

  // get the vidProcCfg register
  vidProcCfg = ioregs->vidProcCfg;

  // enable or disable cursor
  if (en) {
    vidProcCfg = (vidProcCfg & ~(1<<27)) | (en<<27);
  } else {
    vidProcCfg = (vidProcCfg & ~(1<<27));
  }

  // write out changes to vidProcCfg
  ioregs->vidProcCfg = vidProcCfg;

  return FXTRUE;
}

/* bansheeDesktopStartCheck:

   This test was added to check whether the vidDesktopStartAddr
   register is functioning properly.  If not, then this causes
   the Windows desktop icons to be improperly offset so that
   they can not be selected by clicking the mouse.
*/

FxU32 bansheeDesktopStartCheck(LPCARDINFO card, FxU32 StartAddr) {
  SstIORegs *ioregs = (SstIORegs *)(card->NatMem0.MappedAddr);
  FxU32 fbiMemory;
  FxU32 *source;
  FxU32 *end;
  FxU32 *destination;

  if (StartAddr == 0)  {			// Restore desktop start address back to zero 
     ioregs->vidDesktopStartAddr = 0;
  }
  else  {
    printf("\nChecking desktop starting address at 0x%X\n",StartAddr);

    fbiMemory = card->NatMem1.MappedAddr;	// point to frame buffer memory

    source = fbiMemory;			// point source to frame buffer
    end = source+1024;				// set ending memory address
    while (source < end) {			// write a 0x55 pattern to memory
      *source = 0x55;
      source++;
    }

    source = fbiMemory;			// reset pointer to beginning of memory
    destination = fbiMemory+StartAddr;		// copy this same value to memory located
    while (source < end) {			//   at our new starting address   
      *destination = *source;
      source++;
      destination++;
    }

    ioregs->vidDesktopStartAddr = StartAddr;	// change desktop starting address to new setting
  }
  return FXTRUE;				// return to verify checksum
}


FxU32 ban_bilinearOvl(LPCARDINFO card) {
  FxU32 mmio;
  SstIORegs *ioregs;
  FxU32 thold = 32;
  SstRegs *sstregs;
  // int src_y = 70, src_x = 60;
  int src_y = 70, src_x = 140;

  mmio = card->NatMem0.MappedAddr;
  ioregs = (SstIORegs *)mmio;
  mmio = card->NatMem0.MappedAddr + SST_3D_OFFSET;
  sstregs = (SstRegs *)mmio;

  ioregs->vidProcCfg = SST_VIDEO_PROCESSOR_EN;
  ioregs->vidOverlayDudxOffsetSrcWidth = ((640 << 1) << 19);

  thold &= 0x3f;
  ioregs->vidPixelBufThold = (thold | (thold << 6) | (thold << 12));
  
  { 
   FxU32 doStride, stride = 640 * 2;
   FxU32 vidProcCfg = ioregs->vidProcCfg;
   
   sstregs->leftOverlayBuf = 0 + (640 * 2 * src_y) + src_x;
   sstregs->rightOverlayBuf = 0 + (640 * 2 * src_y) + src_x;

   vidProcCfg &= ~(SST_DESKTOP_EN | SST_DESKTOP_TILED_EN |
                   SST_DESKTOP_PIXEL_FORMAT | SST_DESKTOP_CLUT_BYPASS |
                   SST_DESKTOP_CLUT_SELECT | SST_OVERLAY_FILTER_MODE |
                   SST_OVERLAY_TILED_EN | SST_VIDEO_2X_MODE_EN | SST_CURSOR_EN);
   vidProcCfg |= SST_OVERLAY_EN;
   vidProcCfg |= SST_DESKTOP_EN;
   vidProcCfg |= SST_OVERLAY_PIXEL_RGB565U;
   vidProcCfg |= SST_DESKTOP_PIXEL_RGB565;
   vidProcCfg |= SST_DESKTOP_CLUT_BYPASS;
   vidProcCfg |= SST_OVERLAY_CLUT_BYPASS;
   vidProcCfg |= SST_OVERLAY_HORIZ_SCALE_EN;
   vidProcCfg |= SST_OVERLAY_VERT_SCALE_EN;
   vidProcCfg |= SST_OVERLAY_FILTER_BILINEAR;

   ioregs->vidProcCfg = vidProcCfg;
   ioregs->vidDesktopStartAddr = (0 & SST_VIDEO_START_ADDR) << 
                                    SST_VIDEO_START_ADDR_SHIFT;
   // change only the desktop portion of the vidDesktopOverlayStride register
   doStride = ioregs->vidDesktopOverlayStride;
   doStride &= ~(SST_DESKTOP_LINEAR_STRIDE | SST_DESKTOP_TILE_STRIDE);
   stride <<= SST_DESKTOP_STRIDE_SHIFT;
   stride &= SST_DESKTOP_LINEAR_STRIDE;
   doStride |= stride;
   ioregs->vidDesktopOverlayStride = doStride;
  }


#if 1
  ioregs->vidOverlayStartCoords = (30) | (100 << 12); /* x | ( y << 12) */
  ioregs->vidOverlayEndScreenCoord = (230  << SST_OVERLAY_Y_SHIFT) | 
				     (130 & SST_OVERLAY_X);
#else
  ioregs->vidOverlayStartCoords = 0;
  ioregs->vidOverlayEndScreenCoord = (479 << SST_OVERLAY_Y_SHIFT) |
                                     (639 & SST_OVERLAY_X);
#endif

#if 0
  ioregs->vidProcCfg = SST_VIDEO_PROCESSOR_EN | SST_OVERLAY_FILTER_POINT |
      // SST_OVERLAY_HORIZ_SCALE_EN | SST_OVERLAY_VERT_SCALE_EN | 
      SST_OVERLAY_PIXEL_RGB565U | SST_OVERLAY_CLUT_BYPASS;
#endif

  ioregs->vidOverlayDudx = (0x100 << 19) | 0x80000;
  ioregs->vidOverlayDudxOffsetSrcWidth = (50 * 2) << 19 | 0;
  ioregs->vidOverlayDvdy = 0x80000;
  ioregs->vidOverlayDvdyOffset = 0;

  ioregs->vidDesktopStartAddr = 0;
  ioregs->vidDesktopOverlayStride = (640 * 2) | (640 * 2) << 16;

  sstregs->swapbufferCMD = 0;

  return FXTRUE;
}


// ****************************************
// banshee 2d Blit
// ****************************************

FxU32 banshee2dBlit(LPCARDINFO card, enum blit2d_enum type) {
  // GrLfbInfo_t info;
  // FxU32 size = 0;
  FxU32 mmio, x,y,z;
  SstGRegs *regs;
  SstIORegs *ioregs;
  FxU32 cmd_value;
  struct blit2d_params_struct *i = &current_2dblit;
  int cordx[11]  = 
	 {160,80,120,40,80,200,320,520,440,440,400};
  int cordy[11] = 
	  {20,80,120,120,160,220,160,160,120,60,20};

  mmio = card->NatMem0.MappedAddr;
  ioregs = (SstIORegs *)mmio;
  mmio = card->NatMem0.MappedAddr + SST_2D_OFFSET;

  // printf("Voodoo5 mmio 2d linear: 0x%.08X (mapped)\n",mmio);

  regs = (SstGRegs *)mmio;

 // SetMode(0x3);
 // return(FXTRUE);
  
  // setup a decent video mode

#if 0
  if (h3InitSetVideoMode((FxU32) ioregs,640,480,60,0) == FXFALSE) { // 640x480x60hz
    printf("error setting video mode");
    return FXFALSE;
  }
  h3InitVideoDesktopSurface((FxU32)ioregs,1,0,SST_DESKTOP_PIXEL_RGB565,
                            1, 0, 0, 640*2);
#endif

 //  	SetMode(0x111);   // set up mode
 	
//for (z=0;z<10;z++){	  
for (z=0;z<100;z++){	  

// ******* polyline test **********//

         cmd_value = 7; // polyline
	 i->dst.x = 100;
	 i->dst.y = 100;
	 i->src.x = 0;
	 i->src.y = 0;
	 i->colorFore = 2000;

 	for (y=0; y < 20;y++) {
	    i->src.x += 30;
	    i->src.y += 10;
  	    for (x=0; x < 50; x++) {
	       i->dst.y += 5;
	       i->dst.x += 10;
	       setupandwritecommand(card, i, cmd_value);
	    }  /* end for */
	} // end for
  
// ******* rectangle fill test *********//

          sst_idle(card);	
          cmd_value = 5; // rect fill
	  i->dst.x = 0;
	  i->dst.y = 0;
	  i->dst_size.x = 100;
	  i->dst_size.y = 100;
	  i->colorFore = 4660;
	
	for(y=0;y<20;y++){
		i->dst.x +=100;
		 
  		for (x=0; x < 20; x++){
	
			i->dst.x += 10;
			i->dst.y += 10;
			i->colorFore -= 10;

	  		setupandwritecommand(card,i, cmd_value);
		}//endfor
	}/* end for */
  

   	  i->dst.x = 0;
	  i->dst.y = 110;
	  i->colorFore = 2016;

//  for (x=0; x < 50; x++){
  for (x=0; x < 100; x++){
	
	i->dst.x += 10;
	i->dst.y += 10;
	i->colorFore -= 32;

	  setupandwritecommand(card,i, cmd_value);

	}/* end for */

	  i->dst.x = 0;
	  i->dst.y = 220;
	  i->colorFore = 63488;

//  for (x=0; x < 50; x++){
  for (x=0; x < 100; x++){
	
	i->dst.x += 10;
	i->dst.y += 220;
	i->colorFore -= 2048;
		    setupandwritecommand(card,i, cmd_value);

	}/* end for */

		i->dst.x = 0;
	  	i->dst.y = 0;
	  	i->dst_size.x = 100;
	  	i->dst_size.y = 100;
	  	i->colorFore = 1234;

   	    setupandwritecommand(card,i, cmd_value);
  //******* line mode test ******* //
   	 sst_idle(card);	
     cmd_value = 6; // line
     i->dst.x = 200;
	  i->dst.y = 200;
	  i->src.x = 100;
	  i->src.y = 100;
	  i->colorFore = 1245;

//  	for (y=0; y<50; y++){
  	for (y=0; y<100; y++){
	  	i->dst.x += 10;

//	  	for (x=0; x < 50; x++){
	  	for (x=0; x < 100; x++){
			i->dst.y += 10;

	 		setupandwritecommand(card,i, cmd_value);

		}/* end for */

	}//end for
//  ****** screen to screen blit ********//
	  	sst_idle(card);	
	 	i->dst.x = 300;
		i->dst.y = 300;
		i->dst_size.x = 100;
		i->dst_size.y = 100;
		i->src_size.x = 100;
		i->src_size.y = 100;
		i->src.x = 0;
	  	i->src.y = 0;
	
		cmd_value = 1;

//for (y=0; y < 100; y++){  Original value!!!
for (y=0; y < 100; y++){
		i->src.x += 10;
	  	i->src.y += 10;

	for (x=0; x<40; x++){
	    setupandwritecommand(card,i, cmd_value);
	    i->src.x += 20;
	    i->dst.y += 15;
	} //end for

}//end for
// ****** screen to screen stretch  *******//
		sst_idle(card);	
 		i->dst.x = 350;
		i->dst.y = 100;
		i->dst_size.x = 200;
		i->dst_size.y = 200;
		i->src_size.x = 100;
		i->src_size.y = 100;
		i->src.x = 400;
	  	i->src.y = 300;
	
		cmd_value = 2;

		//for (y=0; y < 10; y++){
		//for (y=0; y < 50; y++){  /* Still needed 3 memtests */
		for (y=0; y < 100; y++){
		  i->src.x += 20;
	  	  i->src.y -= 10;
			
		  for (x=0; x<40; x++){
		    setupandwritecommand(card,i, cmd_value);
		    i->src.x += 20;
		    i->dst.y += 15;
		  }
		}
   
	// ******* polygon fill *************//
	// the setupandwrite command has been called prev to this routine
	// so clipping and some other important regs have been setup.
	// we have to manipulate some reg's directly in this routine because
	// the setup&wr command is not (yet) sufficently complicated to know how to 
	// manipulate the regs
	 
		sst_idle(card);	 
		cmd_value = 8; // polygon fill
     
	 
  //	  	i->src.x = 4;
  //	  	i->src.y = 1;
	  	regs->colorFore = 1234;
   	  	sst_idle(card);
		regs->command = ( 0xcc000100 | cmd_value | BIT(8));
		
		for (x=0; x < 12; x++){
		  writetolaunch(card, cordx[x], cordy[x]);
		}
   
 }//end for
   //********************************************************

		   
	printf("end 2dblit\n");

  //  SetMode(0x03);   // restore prev vid mode

					      		  
 	
  	return FXTRUE; //return good status


}

//
//
//


writetolaunch(LPCARDINFO card,int xvertex, int yvertex){
  SstGRegs *regs;
  FxU32 mmio;
  
  
   mmio = card->NatMem0.MappedAddr + SST_2D_OFFSET;
   regs = (SstGRegs *)mmio;


   regs->launch[0] =  (xvertex | (yvertex << 16));


}


  setupandwritecommand(LPCARDINFO card, struct blit2d_params_struct *i,FxU32 cmd_value){

   SstGRegs *regs;
   FxU32 mmio;
  
  
   mmio = card->NatMem0.MappedAddr + SST_2D_OFFSET;

  
   regs = (SstGRegs *)mmio;


// probably should check status reg for fifo room

//  setup clipping
  regs->clip0min = 0;
  regs->clip0max = 0x01e00280; // x and y max values

// destination addr
  regs->dstBaseAddr = 0x00000;

// srcbase address
  regs->srcBaseAddr = 0x00000;

// destination format to 16bpp, 640x2 (bytes) stride
  regs->dstFormat = 0x00030000 + 640*2;

// src format to 16bpp, 640x2 (bytes) stride
  regs->srcFormat = 0x00030000 + 640*2;

// set command Extra to zero
  regs->commandEx = 0;

  regs->colorFore = (FxU16) i->colorFore;
//  printf("color = 0x%X\n",i->colorFore);
  
  regs->dstXY = i->dst.x | (i->dst.y << 16);
  regs->dstSize = i->dst_size.x | (i->dst_size.y << 16);


	if(cmd_value == 6 || cmd_value == 7 || cmd_value == 1 || cmd_value == 2){
	/* load src and src size */
	regs->srcXY = (i->src.x | (i->src.y << 16));
	regs->srcSize= i->src_size.x | (i->src_size.y << 16);
	}
    
   	sst_idle(card);

/*endtest*/

// issue the draw command!
  regs->command = ( 0xcc000100 | cmd_value);

//

 }

//**********************
//	sst idle
/*******/ //test idle
sst_idle(LPCARDINFO card)
{
FxU32 mmio,idle,busy;
SstIORegs *ioregs;
SstRegs *sstregs;

	mmio = card->NatMem0.MappedAddr + SST_3D_OFFSET;
	sstregs = (SstRegs *)mmio;
	
	mmio = card->NatMem0.MappedAddr;
	ioregs = (SstIORegs *)mmio;
  	idle = 1;
	busy = 0;

   	sstregs->nopCMD	= 0;  // write to nopcmd reg per spec

	while (idle != 0 ){
  	
  		   	idle = (ioregs->status & 0x00E20);	   // check for device busy bit(9) 2dbusy(10) and cmd fifo busy (11)  pcififobusy (5) 
		   //	printf("status reg = %x\n", idle);
			busy ++;
			if (busy > 1000000){
				printf("voodoo chip busy ... status reg = %x\n", idle);
				return(FXFALSE);
			}

	}
 
   return(FXTRUE);
}



// ************************// 

FxU32 bansheeFbiSize(LPCARDINFO card) {
  FxU32 mmio;
  FxU32 size = 0;
  SstIORegs *regs;
  SstRegs *mm_regs;
        
// printf("Voodoo5 mmio linear: 0x%.08X (mapped)\n",card->NatMem0.MappedAddr);

  mmio = card->NatMem0.MappedAddr;
  regs = (SstIORegs *)mmio;

  mmio = card->NatMem0.MappedAddr + SST_3D_OFFSET;
  mm_regs = (SstRegs *)mmio;

  size = GetMemSize(regs->dramInit0, regs->dramInit1);	/* Returns memory size in MB */

  mm_regs->lfbMode = mm_regs->lfbMode & (~SST_LFB_ENPIXPIPE);

  regs->lfbMemoryConfig = 0x01FFF; //0x03FFF; /* turn off tiled memory */

  return (size);
}

/*----------------------------------------------------------------------
Function name:  GetMemSize
Description:    Return the size of memory in MBs.
             
Information:
Return:         FxU32   The size of memory in MBs.
----------------------------------------------------------------------*/
FxU32 GetMemSize(FxU32 dramInit0, FxU32 dramInit1)  {  // init io-register base
   FxU32
        partSize,               // size of SGRAM chips in Mbits
        memSize,                // total size of memory in MBytes
        nChips,                 // # of chips of SDRAM/SGRAM
        dramInit0_strap;
        
  // determine memory size from strapping pins (dramInit0 and dramInit1)
    dramInit0_strap = dramInit0;
    dramInit0_strap &= SST_SGRAM_TYPE | SST_SGRAM_NUM_CHIPSETS;

	

    nChips = ((dramInit0_strap & SST_SGRAM_NUM_CHIPSETS) == 0) ? 4 : 8;
  
    if ( (dramInit0_strap & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_8MBIT )  {
      partSize = 8;
    } else if ( (dramInit0_strap & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_16MBIT) {
      partSize = 16;
    } else if ( (dramInit0_strap & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_32MBIT) {
      partSize = 32;
    } else if ( (dramInit0_strap & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_64MBIT) {
      partSize = 64;
    } else if ( (dramInit0_strap & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_128MBIT) {
      partSize = 128;
    } else {
        printf("GetMemSize: Invalid sdram/sgram type = 0x%x\n",
	 //   (dramInit0_strap & SST_SGRAM_TYPE) << SST_SGRAM_TYPE_SHIFT );
          (dramInit0_strap & SST_SGRAM_TYPE));
        return 0;
    }

   memSize = (nChips * partSize) / 8;       // in MBytes
   return (memSize);
}


FxU32 bansheeMemClock(LPCARDINFO card) {
   SstIORegs *ioregs = (SstIORegs *)(card->NatMem0.MappedAddr);
   double clkfreq;
   long dwPllCtrl1;
   int   m,n,p;

   // set pattern address, we're just setting it to 0
   dwPllCtrl1 = ioregs->pllCtrl1;
   n = ((dwPllCtrl1 & 0xFF00)>>8);
   m = ((dwPllCtrl1 & 0xFC)>>2);
   p = (dwPllCtrl1 & 0x3);

   clkfreq = (14.31818 * (double)(n + 2.0))/((double)(m + 2.0) * pow(2.0,(double)(p)));

   printf("pllctrl1 register = 0x%lX\n",dwPllCtrl1);

  return (clkfreq);
}

const PciRegister PCI_CFGSCRATCH = { 80, 4, READ_WRITE };
#define NUMTEST 5

FxBool bansheePciTest(LPCARDINFO card, FxU32 testcode, FxU32 SSTRegister, FxU32 mask)
{
  FxBool retval;
  FxU32 scratchData, scratchReadback, savereg, base_addr;
  FxU32 dataTest[NUMTEST] = { 0x0, 0xFFFFFFFF, 0xAA55AA55, 0x55AA55AA, 0x12345678 };
  int loop;
  FxU32 err_count = 0;

  if (testcode == 0)  {
    /* first, check config space, using cfgScratch */
    printf("Testing PCI scratch register\n");

    for (loop=0 ; loop < NUMTEST ; loop++) {
      scratchData = dataTest[loop];
      pciSetConfigData( PCI_CFGSCRATCH , card->pciDevNum, &scratchData);
      pciGetConfigData( PCI_CFGSCRATCH , card->pciDevNum, &scratchReadback); 
      if (scratchData != scratchReadback) {
        printf("PCI scratch test failure ex: 0x%.08X  rec: 0x%.08X\n",scratchData, 
  	       scratchReadback);
        err_count++;
      }
    }
  }
  else {
    printf("Testing PCI Base Address 0, offset %X\n",SSTRegister);
    base_addr = card->PCIBase0;
    base_addr = base_addr + SSTRegister;
    
    for (loop=0 ; loop < NUMTEST ; loop++) {
      scratchData = dataTest[loop];
      savereg = base_addr;
      base_addr = scratchData;
      scratchReadback = base_addr;
      base_addr = savereg;

      scratchReadback = scratchReadback & mask;
      scratchData = scratchData & mask;
      if (scratchData != scratchReadback) {
        printf("Error reading PCI Base Address 0, offset %X  ex: 0x%.08X  rec: 0x%.08X\n", SSTRegister, scratchData, 
  	       scratchReadback);
        err_count++;
      }
    }
  }
  if (!err_count) {
//    printf("PCI test passed\n");
    retval =  FXTRUE;
  }
  else {
    retval = FXFALSE;
  }
  return (retval);
}

/*
 * these are the framebuffer check routines, basically, there are two checksum routines, one for
 * reading the computed checksum out of a B0 banshee, and one for doing a framebuffer compare.
 *
 * we should abstract these into a validate API per-card
 */

FxU32 bansheeFbChecksum(LPCARDINFO *card) {
  /* how do we check the visible framebuffer? */

  return 0;
}


/* this should read the Banshee B0 video checksum register, wait for vsync, and do it again,
   just to make sure it's stable. */

FxU32 bansheeVideoChecksum(LPCARDINFO *card) {
  return 0;
}

void pci_write_dword(int pcireg, int device, FxU32 value) {
    CRS    crs;

    crs.Regs32.Client_ECX = value;
    crs.Regs8.Client_AH  = 0xB1;          // PCI_FUNCTION_ID
    crs.Regs8.Client_AL  = 0x0D;          // WRITE_CONFIG_DWORD
    crs.Regs16.Client_DI = pcireg;	  // Register Number
    crs.Regs16.Client_BX = device;        // Set device for master or slave
    DPMIRealModeInt(0x1A, &crs);
}

FxU32 pci_read_dword(int pcireg, int device) {
    CRS    crs;

    crs.Regs8.Client_AH  = 0xB1;          // PCI_FUNCTION_ID
    crs.Regs8.Client_AL  = 0x0A;          // READ_CONFIG_DWORD
    crs.Regs16.Client_DI = pcireg;	  // Register Number
    crs.Regs16.Client_BX = device;     // Set function number = master or slave
    DPMIRealModeInt(0x1A, &crs);
    return (crs.Regs32.Client_ECX);	  // Return register contents
}

///////////////////////////////////////////////////////////////////////////
// RAW BANSHEE INIT!
//
//
//
#ifdef BIT
#undef BIT
#endif
#define BIT(x) (1<<x)     // gives back a value with a 1 in that bit position

// outputs to a config register x the value y
#define CFGOUT32(x,y)     

// output to a banshee register
#define OUT32(x,y)
#define IN32(x)

#if 0

FxBool bansheeRawInit(CARDINFO *card) {
  // regbase
  
  // enable IO access
  CFGOUT32(CFG_PCICOMMAND, BIT(0));

  // you have to add a read wait-stat before you can read any regs under AGP
  OUT32(B_PCIINIT0, 0x1800940); 
/*test */
//  OUT32(B_PCIINIT0, 0x1800140); 


// pciInit0 = IN32(B_PCIINIT0);                    // don't know what this is for or why it is commented
// OUT32(B_PCIINIT0, pciInit0 | BIT(8) | BIT(9)    // out

  // other init routines
  h3InitPlls(regbase,50,50);  // grx_clk = 50mhz, mem_clk = 50mhz
  h3InitSgram(regbase);
  h3InitVga(regbase);
//  h3InitTvOut(regbase,"ChrontelSlave");
//  h3InitVideoTiming();  // don't know why this is commented out either
  h3InitVideo(regbase);
//  h3InitAgp(); // nor this
  
  // enable memory mapped access
  CFGOUT32(B_PCICOMMAND, BIT(0) | BIT(1) );

  // bansheeRawInit() done 
}

#endif

