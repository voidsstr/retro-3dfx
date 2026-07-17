/*
 *  pcibrd.c
 *
 * This manages the pci boards available to the system
 *
 *  were going to try and put the agp stuff in here 
 */

#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "3dfx.h"
#include "fxpci.h"

#include "mdc.h"

#include "pcibrd.h"
#include "errrpt.h"
#include "h3regs.h"
#include "h3defs.h"
#include "banshee.h"

#ifdef BIT
#undef BIT
#endif
#define BIT(x) (1<<x)     // gives back a value with a 1 in that bit position

// outputs to a config register x the value y
#define CFGOUT32(x,y)     

#define IN32(x)
  
#define GET(s) s
#define SET(d,s) d = s


/// FlushCPUCache() - Flush CPU Cache - necessary for DMA buffers in AGP memory
void FlushCPUCache ();
void FlushCPUCacheX86 ();
#pragma aux FlushCPUCacheX86 = \
	"dw 090fh";

FxU32 *tbuf;


typedef struct hwcRegInfo_s {
  FxBool
    initialized;
  volatile FxU32
    ioMemBase,                  /* mem base for I/O aliases */
    cmdAGPBase,                 /* CMD/AGP register base */
    waxBase,                    /* 2D register base */
    sstBase,                    /* 3D register base */
    lfbBase,                    /* 3D lfb base */
    rawLfbBase;                 /* Raw LFB base (base address 1) */
  volatile FxU16
    ioPortBase,                 /* I/O base address */
    pad;                        /* Keep things aligned */
} hwcRegInfo;


typedef struct board_handler_struct {
  FxU32 vendorID, deviceID;
  int (*init_fn)(LPCARDINFO);
  struct board_handler_struct *next;
} BOARD_HANDLER;

BOARD_HANDLER *brd_initfn_list = NULL;

void mdc_register_pcivendor(FxU32 vendorID, char *name) {
}

void mdc_register_pcidevice(FxU32 deviceID, char *name) {
}

void mdc_register_boardinitfn(FxU32 vendorID, FxU32 deviceID, int (*init_fn)(LPCARDINFO)) {
  BOARD_HANDLER *temp = malloc(sizeof(BOARD_HANDLER));

  if (temp) {
    memset(temp,0,sizeof(BOARD_HANDLER)); /* clear the new struct */
    temp->next = brd_initfn_list;         /* put into the list */
    brd_initfn_list = temp;

    temp->vendorID = vendorID;
    temp->deviceID = deviceID;
    temp->init_fn  = init_fn;
  }
}

void mdc_print_boardinitfns() {
  BOARD_HANDLER *walker = brd_initfn_list;
  int print_count = 0;
  
  while (walker) {
    if (!print_count++) {
      printf("Registered board init functions:");
    }
    printf("0%.04X  0x%.04X    %s\n",walker->vendorID,walker->deviceID, 
	   pciGetVendorName(walker->vendorID));
    walker = walker->next;
  }
}


void DPMIRealModeInt(int intr, PCRS crs)
{
    union REGS regs;
    struct SREGS sregs;
    //
    // DPMI RealMode Reg struct
    //
    static struct rminfo {
      FxU32 EDI;
      FxU32 ESI;
      FxU32 EBP;
      FxU32 reserved_by_system;
      FxU32 EBX;
      FxU32 EDX;
      FxU32 ECX;
      FxU32 EAX;
      FxU16 flags;     
      FxU16 ES,DS,FS,GS,IP,CS,SP,SS;
    } RMI =  {0};


    /* Use DMPI call 300h to issue the DOS interrupt */
    segread(&sregs);
    RMI.ES = sregs.es;
    RMI.DS = sregs.ds;
    RMI.CS = sregs.cs;
    RMI.GS = sregs.gs;
    RMI.FS = sregs.fs;
    RMI.EAX = (*crs).Regs32.Client_EAX;
    RMI.EBX = (*crs).Regs32.Client_EBX;
    RMI.ECX = (*crs).Regs32.Client_ECX;
    RMI.EDX = (*crs).Regs32.Client_EDX;
    RMI.ESI = (*crs).Regs32.Client_ESI;
    RMI.EDI = (*crs).Regs32.Client_EDI;

    regs.w.ax = 0x0300;
    regs.h.bl = intr;
    regs.h.bh = 0;
    regs.w.cx = 0;
    sregs.es = FP_SEG(&RMI);
    regs.x.edi = FP_OFF(&RMI);

    int386x( 0x31, &regs, &regs, &sregs );

    (*crs).Regs32.Client_EAX = RMI.EAX;
    (*crs).Regs32.Client_EBX = RMI.EBX;
    (*crs).Regs32.Client_ECX = RMI.ECX;
    (*crs).Regs32.Client_EDX = RMI.EDX;
    (*crs).Regs32.Client_ESI = RMI.ESI;
    (*crs).Regs32.Client_EDI = RMI.EDI;
}


//
// Device register access array.
//


PHWREG       vd3Addr, romAddr, fbAddr;

static CRS vd3crs;

#define AGP_APERTURE_SIZE	0x2000000	// 32 MB
//#define AGP_APERTURE_SIZE	0x4000000	// 64 MB
//#define AGP_APERTURE_SIZE	0x8000000	// 128 MB

// AGP Bridge present test.
FxU32 AGPBridge()
{
   union  REGS regs;
   struct SREGS sregs;

   CRS    crs;
   FxU32  access;
   FxU32  vendor;
   FxU32  voloco;
   FxU32  PhysFbAddr;
   //FxU32	fbAddr;
   FxU32  PhysAddr;
   FxU32  IntLine;
   FxU32  IntPin;

    //
    // Look for PCI BIOS.
    //
    crs.Regs8.Client_AH = 0xB1;             // PCI_FUNCTION_ID
    crs.Regs8.Client_AL = 0x01;             // PCI_BIOS_PRESENT
    DPMIRealModeInt(0x1A, &crs);
    if (crs.Regs32.Client_EDX != ' ICP')    // FIND_PCI_DEVICE_SIGNATURE
    {
        printf("Cannot find PCI BIOS!!\n");
        return (0);
    }
    access = crs.Regs8.Client_AL & 0x0F;    // Save hardware access method

    //
    // Get the voodoo configuration from PCI space.
    //
    vendor               = 0x121A;          // Vendor ID (3dfx)
    crs.Regs8.Client_AH  = 0xB1;            // PCI_FUNCTION_ID
    crs.Regs8.Client_AL  = 0x02;            // FIND_PCI_DEVICE
    crs.Regs16.Client_CX = 0x09;            // Device ID = Voodoo5
    crs.Regs16.Client_DX = vendor;          // Vendor ID
    crs.Regs16.Client_SI = 0;               // Index
    DPMIRealModeInt(0x1A, &crs);
    if (crs.Regs8.Client_AH)   {
        printf("Cannot find ***Voodoo5***\n");
        return (0);
    }
   
    vd3crs = crs;	// save info for PciCfgreadwrite* routines

    voloco = crs.Regs16.Client_BX | 0x0001;
     //
    // Get voodoo assigned IRQ and interrupt pin.
    //
    crs.Regs8.Client_AH  = 0xB1;            // PCI_FUNCTION_ID
    crs.Regs8.Client_AL  = 0x0A;            // READ_CONFIG_DWORD
    crs.Regs16.Client_DI = 0x3C;            // Register Number
    DPMIRealModeInt(0x1A, &crs);
    IntLine = crs.Regs32.Client_ECX & 0xFF;
    IntPin  = 0x01;      // Int #A
    if (IntLine == 0)   {
        printf("Voodoo5 assigned IRQ0 - assuming no interrupt assigned\n");
    }

    //
    // Get Voodoo5 physical address.
    //
    crs.Regs8.Client_AH  = 0xB1;        // PCI_FUNCTION_ID
    crs.Regs8.Client_AL  = 0x0A;        // READ_CONFIG_DWORD
    crs.Regs16.Client_DI = 0x10;        // Register Number     
    DPMIRealModeInt(0x1A, &crs);
    PhysAddr = crs.Regs32.Client_ECX & 0xFFFFFFF0;
    printf("Voodoo physical address: %08X\n",PhysAddr);

    if (!PhysAddr)    {
        printf("Invalid Voodoo5 address!!\n");
        return (0);
    }
 
    // Get Frame buffer  physical address.
    //
    crs.Regs8.Client_AH  = 0xB1;        // PCI_FUNCTION_ID
    crs.Regs8.Client_AL  = 0x0A;        // READ_CONFIG_DWORD
    crs.Regs16.Client_DI = 0x14;        // Register Number     
    DPMIRealModeInt(0x1A, &crs);
    PhysFbAddr = crs.Regs32.Client_ECX & 0xFFFFFFF0;

    if (!PhysFbAddr)    {
        printf("Invalid Frame Buffer address!!\n");
        return (0);
    }
    printf (" frame buffer phy addr = %x \n", PhysFbAddr);
    //
    // Map Voodoo address
    //
    segread(&sregs);
    regs.w.ax = 0x0800;
    regs.w.bx = PhysAddr >> 16;
    regs.w.cx = PhysAddr & 0xFFFF;
    regs.w.si = 0x0100;
    regs.w.di = 0;
    int386x(0x31, &regs, &regs, &sregs);
    if (regs.x.cflag)  {
        printf("DPMI Physical Mapping Failure!\n");
        return (0);
    }
    else {
        vd3Addr = (PHWREG)regs.w.bx;
	vd3Addr = (PHWREG)((int)vd3Addr << 16);
        vd3Addr = (PHWREG)((int)vd3Addr + regs.w.cx);
    }

    printf("Voodoo address=  %x\n", vd3Addr);


    //
    // Map address of Voodoo FB.
    //
    segread(&sregs);
    regs.w.ax = 0x0800;
    regs.w.bx = PhysFbAddr >> 16;
    regs.w.cx = PhysFbAddr & 0xFFFF;
    regs.w.si = 0x0100;
    regs.w.di = 0;
    int386x(0x31, &regs, &regs, &sregs);
    if (regs.x.cflag)  {
        printf("Dos Protected Mode Interface physical framebuffer mapping failure!\n");
        return (0);
    }
    else {
        fbAddr = (PHWREG)regs.w.bx;
        fbAddr = (PHWREG)((int)fbAddr << 16);
        fbAddr = (PHWREG)((int)fbAddr + regs.w.cx);
    }

    printf("Voodoo frame buffer = %x\n", fbAddr);
    //
    // Get PAC configuration from PCI space.
    //
    // Look for 440LX
    //
    crs.Regs8.Client_AH  = 0xB1;            // PCI_FUNCTION_ID
    crs.Regs8.Client_AL  = 0x02;            // FIND_PCI_DEVICE
    crs.Regs16.Client_CX = 0x7180;          // Device ID
    crs.Regs16.Client_DX = 0x8086;          // Vendor ID
    crs.Regs16.Client_SI = 0;               // Index
    DPMIRealModeInt(0x1A, &crs);
    if (crs.Regs8.Client_AH)    {
	//
	// Look for 440BX
	//
	crs.Regs8.Client_AH  = 0xB1;            // PCI_FUNCTION_ID
	crs.Regs8.Client_AL  = 0x02;            // FIND_PCI_DEVICE
	crs.Regs16.Client_CX = 0x7190;          // Device ID
	crs.Regs16.Client_DX = 0x8086;          // Vendor ID
	crs.Regs16.Client_SI = 0;               // Index
	DPMIRealModeInt(0x1A, &crs);
	if (crs.Regs8.Client_AH)     {
	    // Look for Katmai
	    //
	    crs.Regs8.Client_AH  = 0xB1;            // PCI_FUNCTION_ID
	    crs.Regs8.Client_AL  = 0x02;            // FIND_PCI_DEVICE
	    crs.Regs16.Client_CX = 0x71A0;          // Device ID
	    crs.Regs16.Client_DX = 0x8086;          // Vendor ID
	    crs.Regs16.Client_SI = 0;               // Index
	    DPMIRealModeInt(0x1A, &crs);
	}
	if (crs.Regs8.Client_AH)    {
	        printf("Cannot find 440LX or 440BX or Katmai Bridge!!\n");
	        return (0);
	}
    }
    printf("Found 440LX or 440BX or Katmai Bridge!!\n");
    return(1); // AGP LX or BX found.
}
  
//****************************************************
//
//
//		setup the gart 
//
//
//****************************************************

FxU32 AGPBridgeSetup ()	 {	   // setup Intel LX, BX or Katmai host bridge chip
    CRS  crs;
    int i;
    FxU32 *ApertureTranslationTable, t;
    FxU32 AttSize;
    FxU32 AGPGARTBase;

    // Get Aperture Base Configuration Register, APBASE.
    crs.Regs16.Client_AX  = 0xB10A;
    crs.Regs16.Client_BX  = 0x0000;
    crs.Regs16.Client_DI  = 0x0010;
    crs.Regs32.Client_ECX = 0x0000;
    DPMIRealModeInt(0x1A, &crs);

    AGPGARTBase = crs.Regs32.Client_ECX & 0xFFC00000;
    printf("agpgartbase = %x\n", AGPGARTBase);

    AttSize = (AGP_APERTURE_SIZE>>12) * 4;	// 1 DWORD per 4K page
    t = ((FxU32) malloc (AttSize+0x1000) + 0x1000) & 0xfffff000;
    tbuf = t;
    if (t == NULL) {
	printf("Insufficient memory to set the aperture for AGP test\n");
	return (0);
    }
   
    printf ( "Addr of aperature table = %x, tbuf = %x \n", t, tbuf);
    ApertureTranslationTable = (FxU32 *) t;

    // direct map Virtual to Physical
    for (i = 0; i < (AGP_APERTURE_SIZE>>12); i++)  {
	ApertureTranslationTable[i] = (i<<12) | 1;
       // printf ( "table data = %x\n", ApertureTranslationTable[i]);
    }
    FlushCPUCacheX86 ();
    crs.Regs8.Client_AH  = 0xB1;        // PCI_FUNCTION_ID
    crs.Regs8.Client_AL  = 0x0B;        // WRITE_CONFIG_BYTE
    crs.Regs16.Client_DI = 0x90;        // Register Number     
    crs.Regs8.Client_CL  = 0xE0;        // Enable SERR on bridge
    DPMIRealModeInt(0x1A, &crs);

    crs.Regs8.Client_AH  = 0xB1;        // PCI_FUNCTION_ID
    crs.Regs8.Client_AL  = 0x0B;        // WRITE_CONFIG_BYTE
    crs.Regs16.Client_DI = 0x92;        // Register Number     
    crs.Regs8.Client_CL  = 0xFF;        // Clear status
    DPMIRealModeInt(0x1A, &crs);

    crs.Regs8.Client_AH   = 0xB1;       // PCI_FUNCTION_ID
    crs.Regs8.Client_AL   = 0x0D;       // WRITE_CONFIG_DWORD
    crs.Regs16.Client_DI  = 0xA8;       // Register Number     
    crs.Regs32.Client_ECX = 0x00;	// disable AGP
    DPMIRealModeInt(0x1A, &crs);

    crs.Regs8.Client_AH  = 0xB1;        // PCI_FUNCTION_ID
    crs.Regs8.Client_AL  = 0x0C;        // WRITE_CONFIG_WORD
    crs.Regs16.Client_DI = 0x50;        // Register Number     
    crs.Regs16.Client_CX = 0x0024;      // Disable aperture access
    DPMIRealModeInt(0x1A, &crs);

//    crs.Regs8.Client_AH   = 0xB1;        // PCI_FUNCTION_ID
//    crs.Regs8.Client_AL   = 0x0D;        // WRITE_CONFIG_DWORD
//    crs.Regs16.Client_DI  = 0x10;        // Register Number     
//    crs.Regs32.Client_ECX = AGPGARTBase;
//    DPMIRealModeInt(0x1A, &crs);
 
    crs.Regs8.Client_AH  = 0xB1;        // PCI_FUNCTION_ID
    crs.Regs8.Client_AL  = 0x0D;        // WRITE_CONFIG_DWORD
    crs.Regs16.Client_DI = 0xA8;        // Register Number     
    //// enable agp 2x no matter what 
    crs.Regs32.Client_ECX = 0x00000302; // enable AGP 2X, SB
    DPMIRealModeInt(0x1A, &crs);

    crs.Regs8.Client_AH  = 0xB1;        // PCI_FUNCTION_ID
    crs.Regs8.Client_AL  = 0x0D;        // WRITE_CONFIG_DWORD
    crs.Regs16.Client_DI = 0xB4;        // Register Number     
    crs.Regs32.Client_ECX = 0x3f&((~(AGP_APERTURE_SIZE - 1))>>22);
    DPMIRealModeInt(0x1A, &crs);

    crs.Regs8.Client_AH  = 0xB1;        // PCI_FUNCTION_ID
    crs.Regs8.Client_AL  = 0x0D;        // WRITE_CONFIG_DWORD
    crs.Regs16.Client_DI = 0xB8;        // Register Number     
    crs.Regs32.Client_ECX = (FxU32) ApertureTranslationTable;
    DPMIRealModeInt(0x1A, &crs);

    crs.Regs8.Client_AH  = 0xB1;        // PCI_FUNCTION_ID
    crs.Regs8.Client_AL  = 0x0C;        // WRITE_CONFIG_WORD
    crs.Regs16.Client_DI = 0x50;        // Register Number     
    crs.Regs16.Client_CX = 0x0224;      // Enable aperture access
    DPMIRealModeInt(0x1A, &crs);

    crs.Regs8.Client_AH  = 0xB1;        // PCI_FUNCTION_ID
    crs.Regs8.Client_AL  = 0x0C;        // WRITE_CONFIG_WORD
    crs.Regs16.Client_DI = 0xB0;        // Register Number     
    crs.Regs16.Client_CX = 0x2000;      // Disable GTLB	 graphics translation lookaside buffer
    DPMIRealModeInt(0x1A, &crs);

    crs.Regs8.Client_AH  = 0xB1;        // PCI_FUNCTION_ID
    crs.Regs8.Client_AL  = 0x0C;        // WRITE_CONFIG_WORD
    crs.Regs16.Client_DI = 0xB0;        // Register Number     
    crs.Regs16.Client_CX = 0x2082;      // Enable GTLB
    DPMIRealModeInt(0x1A, &crs);

    return (1);
}



void pcicfgwrite (FxU32 bytes, FxU32 address, FxU32 data) {
    switch (bytes) {
	case 1:
	    vd3crs.Regs8.Client_AL  = 0x0B;            // WRITE_CONFIG_BYTE
	    vd3crs.Regs8.Client_CL  = data;
	    break;
	case 2:
	    vd3crs.Regs8.Client_AL  = 0x0C;            // WRITE_CONFIG_WORD
	    vd3crs.Regs16.Client_CX = data;
	    break;
	case 4:
	    vd3crs.Regs8.Client_AL   = 0x0D;            // WRITE_CONFIG_DWORD
	    vd3crs.Regs32.Client_ECX = data;
	    break;
    }

    vd3crs.Regs8.Client_AH  = 0xB1;               // PCI_FUNCTION_ID
    vd3crs.Regs16.Client_DI = address;            // Register Number
    DPMIRealModeInt(0x1A, &vd3crs);
}


FxU32 pcicfgread (FxU32 bytes, FxU32 address) {
    switch (bytes) {
	case 1:
	    vd3crs.Regs8.Client_AL   = 0x08;            // READ_CONFIG_BYTE
	    break;
	case 2:
	    vd3crs.Regs8.Client_AL   = 0x09;            // READ_CONFIG_WORD
	    break;
	case 4:
	    vd3crs.Regs8.Client_AL   = 0x0A;            // READ_CONFIG_DWORD
	    break;
    }

    vd3crs.Regs8.Client_AH  = 0xB1;            // PCI_FUNCTION_ID
    vd3crs.Regs16.Client_DI = address;         // Register Number
    DPMIRealModeInt(0x1A, &vd3crs);

    switch (bytes) {
	case 1:
	    return (vd3crs.Regs8.Client_CL);
	case 2:
	    return (vd3crs.Regs16.Client_CX);
	case 4:
	    return (vd3crs.Regs32.Client_ECX);
    }
    printf(" Error reading pci register ");
    return (0);

}

void  WritePattern(sstg, z, card)
 SstGRegs *sstg;
 FxU32 z;
 LPCARDINFO card;
 {
 
 FxU32 x,y,rdata,dmask,smask,nBytes,pattern;

 //  for (z1=0; z1<64; z1++) {
 //    SET(sstg->colorPattern[z1], z1 );
 //  }

	nBytes = 64;
        pattern = (z & 0x00000003);
   	sst_idle_really(card);
        switch (pattern) {
			 // walking ones w/ background of zeros
	default:
	case 0:
		for (x=0, y=0; x<nBytes;y++,x++){
 		  rdata = 0x00000000;
		  dmask = (y & 0x00000001F);
		  smask = (rdata | (0x01 << (dmask + 8)) );
		  	 
		  SET(sstg->colorPattern[x], smask );
  		  }
		  //	printf( "pattern 1 1's bg 0's\n" );
		break;
			//walking ones with background of 1's 
	case 1:
		for (x=0, y=0; x<nBytes;y++,x++){
		  rdata = 0x00000000;
		  dmask = (y & 0x00000001F);
  		  smask = (rdata | (0x01 << (dmask + 8)) );
		  
		  /*test*/
	 	  smask = 0xAA55AA55;
		  SET(sstg->colorPattern[x], smask );
  		  }
		  //	printf( "pattern 2 1s bg 1's\n");
		break;
			// walking 0 bg of 1's 
	case 2:
	        for (x=0, y=0; x<nBytes;y++,x++){
		  rdata = 0xffffffff;
		  dmask = (y & 0x00000001F);
  		  smask = (rdata & (~(0x01 << (dmask + 8))) );
		  SET(sstg->colorPattern[x], smask );
  		  }
		  //	printf( "pattern 3  0's bg 1's\n" );
		break;
			
       case 3:	   // walking 0's background of 0's
   		for (x=0, y=0; x<nBytes;y++,x++){
		  rdata = 0xffffffff;
		  dmask = (y & 0x00000001F);
  		  smask = (rdata & (~(0x01 << (dmask + 8))) );
		  SET(sstg->colorPattern[x], smask );
   		  }
		  //	printf( " patterrn 4 0's bg 0's \n" );
		 break;
   	}
}


/**********************************************/
/**********************************************/
FxBool	 CheckPattern(sstg, z, card)
 SstGRegs *sstg;
 FxU32 z;
 LPCARDINFO card;
 {
   FxU32 x,y,rdata,dmask,smask,nBytes,pattern,cpval,counter;
   FxBool retval;

   retval = FXTRUE;    // assume success
   nBytes = 64;
   pattern = (z & 0x00000003);
   switch (pattern){
			 // walking ones w/ background of zeros
	default:
	case 0: //printf("case0\n ");	
	    	for (x=0, y=0; x<nBytes;y++,x++){
	     		rdata = 0x00000000;
			dmask = (y & 0x00000001F);
			smask = (rdata | (0x01 << (dmask + 8)) );
			cpval =	GET(sstg->colorPattern[x]);
			if (cpval != smask){
				printf("AGP error -- data read: %x  should be: %x x=%x case0 z=%x \n", cpval, smask,x,z);
		   		retval = FXFALSE;
		   		return (retval);  	 
			}
  	      }
             //	printf( "pattern 1 1's bg 0's\n" );
	      break;

			//walking ones with background of 1's 
	case 1:	//printf("case1 \n");
	      for (x=0, y=0; x<nBytes;y++,x++){
		 	counter = 0;
		 	rdata = 0x00000000;
		 	dmask = (y & 0x00000001F);
  		 	smask = (rdata | (0x01 << (dmask + 8)) );

		 	/*test*/
	  	 	smask = 0xAA55AA55;
	 //	 	sst_idle_really(card);

	    	while (cpval != smask){
	 		sst_idle_really(card);
		 	cpval = GET(sstg->colorPattern[x]);
			counter++;
			if(counter > 10){
			  //	printf("AGP error -- data read: %x should be: %x x=%x case1 z=%x\n", cpval, smask,x,z);
			  retval = FXFALSE;
		       	  return (retval);  	 
			}
		 }	// endwhile
  	      }		//end for
	      //	printf( "pattern 2 1s bg 1's\n");
	      break;

			// walking 0 bg of 1's 
	case 2: //printf("case2 \n");
	      for (x=0, y=0; x<nBytes;y++,x++){
		 rdata = 0xffffffff;
		 dmask = (y & 0x00000001F);
  		 smask = (rdata & (~(0x01 << (dmask + 8))) );
		 cpval = GET(sstg->colorPattern[x]);
		 if (cpval != smask){
		 	printf("AGP error -- data read: %x should be: %x x=%x case2 z=%x\n", cpval, smask,x,z);
		 	retval = FXFALSE;
		 	return (retval);  	 
		 }
  	       }
	       //	printf( "pattern 3  0's bg 1's\n" );
	       break;
			
		     // walking 0's background of 0's
	case 3: //printf("case3 \n");	
  	      for (x=0, y=0; x<nBytes;y++,x++){
		 rdata = 0xffffffff;
		 dmask = (y & 0x00000001F);
  		 smask = (rdata & (~(0x01 << (dmask + 8))) );
		 cpval = GET(sstg->colorPattern[x]);
		 if (cpval != smask){
		 	printf("AGP error -- data read: %x should be: %x x=%x case3 z=%x\n", cpval, smask,x,z);
		 	retval = FXFALSE;
		 	return (retval);  	 
		 }
   	       }
	       //	printf( " patterrn 4 0's bg 0's \n" );
	       break;
	}
	return(retval);
}

/**********************************************/

/**********************************************/
#define SST_CMDAGP_OFFSET  0x080000
#define SST_2D_OFFSET	   0x100000

FxBool bansheeAgpTest(LPCARDINFO card, FxU32 testcode) {

  BOARD_HANDLER *walker = brd_initfn_list;

//FxU32 agpread(LPCARDINFO card) {
  SstCRegs *sstc = (SstCRegs *)((FxU32)card->NatMem0.MappedAddr + SST_CMDAGP_OFFSET);
  SstGRegs *sstg = (SstGRegs *)((FxU32)card->NatMem0.MappedAddr +  SST_2D_OFFSET);
 /*****************/
  FxU32 devNum;
  FxU32 deviceNumber= 0;
  FxU32 agpcommand = 0;
  FxU32 agpcommanddata = 0;
  FxU32 vendorID = 0;
  FxU32 agpstatusreg = 0;
  FxU32 agpcapabilites = 0;
  FxU32 val = 0;
  FxU32 val2= 0;
  FxU32 *x;
  FxU32 *ptr;
  FxU32 mmio, z;
  SstIORegs *ioregs;
  FxU32 pcistatusreg = 0;
  FxU32 pcicommandreg = 0;
  static  FxU32 baseaddresslow = 0;
  FxU32 patternfail = 0;
  FxU32 patternfailcount = 0;

/* registers we want to restore */
  FxU32 savevidproccfg;
  FxU32 saveagpcommanddata;
  FxU32 savemiscInit1;
  FxU32 status;   	 
  SstRegs *sstregs;
  if (testcode == 2)  {
    printf("Testing AGP 2x\n");
  
    mmio = card->NatMem0.MappedAddr;
    ioregs = (SstIORegs *)mmio;
  
    mmio = card->NatMem0.MappedAddr + SST_3D_OFFSET;
    sstregs = (SstRegs *)mmio;
    savevidproccfg =   GET(ioregs->vidProcCfg );
 
    SET(ioregs->vidProcCfg, 0x00);	//turn off vga
    // 	val = ioregs->agpInit;
    // 	printf( " agp init 0 = %x\n", val);
    //	val = GET(ioregs->agpInit);
    // 	printf( " agp init 0 = %x\n", val);
  
    while (walker) {
      if (pciFindCardMulti(walker->vendorID,walker->deviceID, &devNum,0) == FXTRUE) {
      ASSERT(card = (LPCARDINFO)malloc(sizeof(CARDINFO)),"couldn't allocate cardinfo");
      memset(card,0,sizeof(CARDINFO));
      
      card->vendorID = walker->vendorID;
      card->deviceID = walker->deviceID;
      card->pciDevNum = devNum;
      
      //printf("Default Board Selected bus:%d slot:%d\n", (devNum>>5), (devNum & 0x1F));
      if (walker->init_fn(card) == FXTRUE) {
	status = AGPBridge (); 	// go find the intel 440 lx  chipset this routine
	 		        // says it will find 440 bx but dont think so 
				// or uses the same init values so go with it
	if (status == 0) {	// Did error occur?
 	  // restore registers before exiting
	  SET(ioregs->vidProcCfg, savevidproccfg);  // restore vidProcCfg register
	  return(FXFALSE);
	}											    
	status = AGPBridgeSetup ();		// set up the intel 440 lx/440 bx gart 
        //printf("status of AGPBridgeSetup = %x, tbuf = %x\n",status, tbuf);
        if (status == 0)   {			// error occurred, so error exit
  	  // restore registers before exiting
	  SET(ioregs->vidProcCfg, savevidproccfg);	//restore vidProcCfg register
	  free(tbuf);
	  return(FXFALSE);
	}

	savemiscInit1 = ioregs->miscInit1;	
	ioregs->miscInit1 |= BIT(26);	   /* this should turn on 66 MHZ agp */

  	ioregs->miscInit1 |= BIT(27);	   /* this should turn on agp */
        val = ioregs->miscInit1;	   /* save the miscinit1 reg ----> to val */

	ioregs->miscInit1 |= BIT(19);	   /* this should reset command stream */

	ioregs->miscInit1 = val;	   /* now put back old value */

	// printf("addr of miscinit1 = %x\n", &ioregs->miscInit1); 
	// printf("new miscInit1 = 0x%X\n", ioregs->miscInit1);

        pciGetConfigData( PCI_AGP_CMD, deviceNumber, &saveagpcommanddata);

	// set up agp command reg data 
	agpcommanddata = (BIT(1) | BIT(8) | BIT(9) | BIT(24) | BIT(25));  //  2x sb = 302 rq_depth 3

	for ( deviceNumber = 0; deviceNumber < MAX_PCI_DEVICES; deviceNumber++ ) {
	    if (pciDeviceExists(deviceNumber)) {
	   	pciGetConfigData( PCI_VENDOR_ID, deviceNumber, &vendorID );
		if (vendorID == 0x121a) {
		  //    printf("devnum = %x\n",deviceNumber);
		  if(pciSetConfigData(PCI_AGP_CMD, deviceNumber , &agpcommanddata) == FXFALSE){
		  printf("pci set configdata error\n");
		}
		pciGetConfigData( PCI_AGP_CMD, deviceNumber, &agpcommand);
		pciGetConfigData( PCI_AGP_CAP_ID, deviceNumber, &agpcapabilites);
		pciGetConfigData( PCI_AGP_STATUS, deviceNumber, &agpstatusreg);
	   	pciGetConfigData( PCI_STATUS, deviceNumber, &pcistatusreg);
		pciGetConfigData( PCI_COMMAND, deviceNumber, &pcicommandreg);

	        //printf("vd3 pci 15c agp command data reg = %x\n",agpcommand);
	        //printf("vd3 pci 154 agp capabilities reg = %x\n",agpcapabilites);
	        //printf("vd3 pci 158 agp status reg = %x\n",agpstatusreg );
	        //printf("vd3 pci 104 pci status reg = %x\n",pcistatusreg );
	        //printf("vd3 pci 104 pci command reg = %x\n",pcicommandreg );
		}
	     }
	}

   	x = malloc(100000);
	if (x == NULL) {
	  printf("Insufficient memory available to run AGP test\n");
  	  // restore registers before exiting
	  SET(ioregs->vidProcCfg, savevidproccfg);	//restore vidProcCfg register
	  pciSetConfigData(PCI_AGP_CMD, deviceNumber, &saveagpcommanddata);
	  ioregs->miscInit1 |= BIT(19);	   	       // this should reset command stream
	  ioregs->miscInit1 = savemiscInit1;
          free(tbuf);
	  free(x);
	  return(FXFALSE);
	}
  	val = x;
	val |= 0xF0000000;			// or in gart base address

   	ptr = (FxU32 *)val;
   	*ptr = 0x3f8840;			// put command to write color regs
 	baseaddresslow = val;		        // save off address for later

   	SET(sstc->cmdFifo0.baseAddrL, (val >> 12)); // the base address is assumed to be on 1 meg boundry

	//printf ("sstc->command fifo baseaddr is %x\n", &sstc->cmdFifo0.baseAddrL);
	//printf ("sstc->commandfifobaseaddr = %x\n", sstc->cmdFifo0.baseAddrL); 

	SET(sstc->cmdFifo0.readPtrL, val);
	val = GET(sstc->cmdFifo0.baseSize);
	//printf("sstc cmd fifo 0 .basesize = %x \n", val);
	val = GET(sstc->cmdFifo0.readPtrL);
	//printf("sstc cmd fifo0.readpointerlow = %x \n", val);
  	val = GET(sstc->cmdFifo0.baseAddrL);
	//printf("sstc cmd fifo 0 .baseaddresslow = %x \n", val);
   	val = GET(sstc->cmdFifo0.unusedB);
	//printf("sstc cmd fifo command/status0  = %x \n", val);
	val = GET(sstc->cmdFifo0.depth);
	//printf("sstc cmd fifo depth  = %x \n", val);
   	val = GET(sstc->cmdFifoThresh);
	//printf(" cmd fifo threshold = %x\n", val);
	val = sstregs->intrCtrl;
	//printf("3d interupt control reg = %x\n", val);
   	val = (BIT(0)| BIT(8) | BIT(9) | BIT(10));   // 8k list (0) enable(8) it and
						     // tell it list is in agp mem (9) 
						     //	and disable hole counter(10)
	SET(sstc->cmdFifo0.baseSize, val);	     // this will enable list

   /* Write test pattern to clear out the command fifo before starting the test */
   //  z=1;
   //  	WritePattern(sstg, z, card);
   //  	val = 64;
   //  	SET(sstc->cmdFifo0.bump, val);
   //   z=1;
   //	WritePattern(sstg, z, card);
   // 	val = 64;
   //  	SET(sstc->cmdFifo0.bump, val);

   /******** write read loop *******/
 	patternfailcount = 0;
   	for (z=0; z<10000; z++) {
	   val = baseaddresslow;
	   SET(sstc->cmdFifo0.readPtrL, val);
	   //printf("..z=%x",z);
 	   WritePattern(sstg, 1, card);
	   val = 64;
	   SET(sstc->cmdFifo0.bump, val);			 
	   patternfail= 0;
    	   if (CheckPattern(sstg,1, card) == FXFALSE) {
    		patternfail++;
		patternfailcount++;
		ioregs->miscInit1 |= BIT(19);	   /* this should reset command stream */
		ioregs->miscInit1 = savemiscInit1;
    		if (patternfail > 1) {
            	  printf("AGP 2x test failed\n");
  		  // restore registers before exiting
		  SET(ioregs->vidProcCfg, savevidproccfg);	//restore vidProcCfg register
		  pciSetConfigData( PCI_AGP_CMD, deviceNumber, &saveagpcommanddata);
  			 	    ioregs->miscInit1 |= BIT(19);	   /* this should reset command stream */
			 	    ioregs->miscInit1 = savemiscInit1;
			 	    pciSetConfigData( PCI_AGP_CMD, deviceNumber, &saveagpcommanddata);
		  free(tbuf);
		  free(x);
		  return(FXFALSE);
		}
	  }
   	 //	val = 64;
	 //	SET(sstc->cmdFifo0.bump, val);			 
	}
        // printf("patternfailcount= %x\n",patternfailcount);
        // printf ("sstc->commandfifobaseaddr = %x\n", sstc->cmdFifo0.baseAddrL); 
        // printf("made it to after for loop\n");

        val = GET(sstc->cmdFifo0.baseSize);
        // printf("sstc cmd fifo 0 .basesize = %x \n", val);
	val = GET(sstc->cmdFifo0.readPtrL);
 	// printf("sstc cmd fifo0.readpointerlow = %x \n", val);
  	val = GET(sstc->cmdFifo0.baseAddrL);
	// printf("sstc cmd fifo 0 .baseaddresslow = %x \n", val);
   	val = GET(sstc->cmdFifo0.unusedB);
	// printf("sstc cmd fifo command/status0  = %x \n", val);
 	val = GET(sstc->cmdFifo0.depth);
	// printf("sstc cmd fifo depth  = %x \n", val);
	val = GET(sstc->cmdFifoThresh);
	// printf(" cmd fifo threshold = %x\n", val);
	val = GET(sstc->cmdFifo0.bump);
	// printf(" cmd fifo 0 bump = %x\n", val);
	val = GET(sstc->agpReqSize);
	// printf("agpreqsize = %x\n", val);
	val = GET(sstc->hostAddrLow);
	// printf("agphostaddresslow= %x\n", val);
	val = GET(sstc->hostAddrHigh);
	// printf("agphostaddrhigh = %x\n", val);
   	val = GET(sstc->graphicsAddr);
	// printf("agpGraphicsaddr= %x\n", val);
	val = GET(sstc->graphicsStride);
	// printf("agpstride= %x\n", val);

	// restore registers before exiting
	SET(ioregs->vidProcCfg, savevidproccfg);	//restore vidProcCfg register
	pciSetConfigData( PCI_AGP_CMD, deviceNumber, &saveagpcommanddata);

   	ioregs->miscInit1 |= BIT(19);	   /* this should reset command stream */
  	ioregs->miscInit1 = savemiscInit1;

	free(tbuf);
	free(x);
        printf("AGP 2x test passed\n");

	val = GET(sstc->cmdFifo0.depth);
	if (val > 0){
	   val2 = 64;
	   SET(sstc->cmdFifo0.bump, val2);			 
	   val = GET(sstc->cmdFifo0.depth);
	   // printf("bump\n");
	}
	// printf("cmdfifodepth = %x\n", val);
  	return (FXTRUE);
     }
    } 
    walker = walker->next;
    }
  }

  else  {    // Test AGP 4X 
    printf("Testing AGP 4x\n");
    printf("AGP 4x test passed\n");
  }

  return(FXTRUE);
}

//*******************************
//*
//*
//*******************************/
sst_idle_really(LPCARDINFO card)
{
  FxU32 mmio,idle,busy;
  SstIORegs *ioregs;
  SstRegs *sstregs;

  mmio = card->NatMem0.MappedAddr;
  ioregs = (SstIORegs *)mmio;

  mmio = card->NatMem0.MappedAddr + SST_3D_OFFSET;
  sstregs = (SstRegs *)mmio;

  idle = 1;
  busy = 0;
	
  sstregs->nopCMD	= 0;  // write to nopcmd reg per spec
  while (idle) {
    idle = (ioregs->status & 0x00E20);	   // check for device busy bit(9) and cmd fifo busy (11)  pcififobusy (5) 
    busy ++;
    if (busy > 1000) {
      // printf("chip busy ... status reg = %x\n", idle);
      return(FXFALSE);
    }
  }
  //printf("idle=%x\n",idle);
  return(FXTRUE);
}

/************************************
 * MDC INIT BOARD (default)
 ***********************************/
LPCARDINFO mdc_init_default_board() {
  BOARD_HANDLER *walker = brd_initfn_list;
  LPCARDINFO card = NULL;
 
  FxU32 devNum;
  
  while (walker) {
    if (pciFindCardMulti(walker->vendorID,walker->deviceID, &devNum,0) == FXTRUE) {

      ASSERT(card = (LPCARDINFO)malloc(sizeof(CARDINFO)),"couldn't allocate cardinfo");
      memset(card,0,sizeof(CARDINFO));
      
      card->vendorID = walker->vendorID;
      card->deviceID = walker->deviceID;
      card->pciDevNum = devNum;
      
      printf("Default Board Selected bus:%d slot:%d\n", (devNum>>5), (devNum & 0x1F));
      if (walker->init_fn(card) == FXTRUE) {
	return (card);
      }
      else {
	printf("Board init failed!\n");
	return (NULL);
      }
    } 
    walker = walker->next;
  }
  if (!card) {
    printf("No default board available!\n");
  }
  return (card);
}
