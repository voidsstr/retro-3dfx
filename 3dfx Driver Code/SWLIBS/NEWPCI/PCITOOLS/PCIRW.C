/*
** THIS SOFTWARE IS SUBJECT TO COPYRIGHT PROTECTION AND IS OFFERED ONLY
** PURSUANT TO THE 3DFX GLIDE GENERAL PUBLIC LICENSE. THERE IS NO RIGHT
** TO USE THE GLIDE TRADEMARK WITHOUT PRIOR WRITTEN PERMISSION OF 3DFX
** INTERACTIVE, INC. A COPY OF THIS LICENSE MAY BE OBTAINED FROM THE 
** DISTRIBUTOR OR BY CONTACTING 3DFX INTERACTIVE INC(info@3dfx.com). 
** THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER 
** EXPRESSED OR IMPLIED. SEE THE 3DFX GLIDE GENERAL PUBLIC LICENSE FOR A
** FULL TEXT OF THE NON-WARRANTY PROVISIONS.  
** 
** USE, DUPLICATION OR DISCLOSURE BY THE GOVERNMENT IS SUBJECT TO
** RESTRICTIONS AS SET FORTH IN SUBDIVISION (C)(1)(II) OF THE RIGHTS IN
** TECHNICAL DATA AND COMPUTER SOFTWARE CLAUSE AT DFARS 252.227-7013,
** AND/OR IN SIMILAR OR SUCCESSOR CLAUSES IN THE FAR, DOD OR NASA FAR
** SUPPLEMENT. UNPUBLISHED RIGHTS RESERVED UNDER THE COPYRIGHT LAWS OF
** THE UNITED STATES.  
** 
** COPYRIGHT 3DFX INTERACTIVE, INC. 1999, ALL RIGHTS RESERVED
**
**
** $Revision: 3$ 
** $Date: 10/11/00 7:41:05 PM$ 
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <3dfx.h>
#include <fxpci.h>

const PciRegister PCI_INIT_ENABLE  = { 0x40, 4, READ_WRITE };
const PciRegister PCI_BUS_SNOOP0   = { 0x44, 4, WRITE_ONLY };
const PciRegister PCI_BUS_SNOOP1   = { 0x48, 4, WRITE_ONLY };
const PciRegister PCI_CFG_STATUS   = { 0x4C, 4, READ_ONLY };

const PciRegister *pciRegArray[] = {
        &PCI_VENDOR_ID,
        &PCI_DEVICE_ID,
        &PCI_COMMAND,
        &PCI_STATUS,
        &PCI_REVISION_ID,
        &PCI_CLASS_CODE,
        &PCI_CACHE_LINE_SIZE,
        &PCI_LATENCY_TIMER,
        &PCI_HEADER_TYPE,
        &PCI_BIST,
        &PCI_BASE_ADDRESS_0,
        &PCI_BASE_ADDRESS_1,
#ifdef OLD_PCI
        &PCI_BASE_ADDRESS_2,
        &PCI_BASE_ADDRESS_3,
        &PCI_BASE_ADDRESS_4,
        &PCI_BASE_ADDRESS_5,
#else
	&PCI_IO_BASE_ADDRESS,
#endif
        &PCI_INTERRUPT_LINE,
        &PCI_INTERRUPT_PIN,
        &PCI_MIN_GNT,
        &PCI_MAX_LAT,
        &PCI_INIT_ENABLE,
        &PCI_BUS_SNOOP0,
        &PCI_BUS_SNOOP1,
        &PCI_CFG_STATUS
};

//----------------------------------------------------------------------
// stolen from John's pci detect program
//----------------------------------------------------------------------
void cmdScan(void)
{
    FxU32 deviceNumber;
    FxBool firstDeviceDetected = FXFALSE;
    FxU32 deviceID = 0;
    FxU32 vendorID = 0;
    FxU32 baseAddress0 = 0;
    FxU32 command = 0;
    FxU32 classCode = 0;

    if (!pciOpen()) {
        fprintf(stderr, pciGetErrorString());
        exit(100);
    }
    for ( deviceNumber = 0; deviceNumber < MAX_PCI_DEVICES; deviceNumber++ ) {
        if ( pciDeviceExists( deviceNumber ) ) {
            if ( !firstDeviceDetected ) {                                       
                puts( "bus slot vendId  devId   baseAddr0   cmd     description" );
                puts( "--- ---- ------  ------  ----------  ------  -----------" );
                firstDeviceDetected = FXTRUE;
            }
    
            pciGetConfigData( PCI_DEVICE_ID, deviceNumber, &deviceID );
            pciGetConfigData( PCI_VENDOR_ID, deviceNumber, &vendorID );
            pciGetConfigData( PCI_BASE_ADDRESS_0, deviceNumber, &baseAddress0 );
            pciGetConfigData( PCI_COMMAND, deviceNumber, &command );
            pciGetConfigData( PCI_CLASS_CODE, deviceNumber, &classCode );

            printf( " %.02d  %.02d  0x%.04lx  0x%.04lx  0x%.08x  0x%.04x  %.8s:%.25s\n", 
                    deviceNumber>>5, deviceNumber&0x1f, vendorID, deviceID, baseAddress0, command, 
                    pciGetVendorName( (FxU16)vendorID ), pciGetClassName( classCode, deviceID ) );
        }
   }
}

void cmdStatus(FxU32 deviceNumber, int is3DFX)
{
    FxU32 x;

    printf("\n");
    printf("Detailed device information\n");
    printf("---------------------------\n");

    pciGetConfigData( PCI_VENDOR_ID, deviceNumber, &x );
    printf("      Vendor_ID: %04x\n",x);
    pciGetConfigData( PCI_DEVICE_ID, deviceNumber, &x );
    printf("      Device_ID: %04x\n",x);
    pciGetConfigData( PCI_COMMAND, deviceNumber, &x );
    printf("        Command: %04x\n",x);
    pciGetConfigData( PCI_STATUS, deviceNumber, &x );
    printf("         Status: %04x\n",x);
    pciGetConfigData( PCI_REVISION_ID, deviceNumber, &x );
    printf("    Revision_ID: %02x\n",x);
    pciGetConfigData( PCI_CLASS_CODE, deviceNumber, &x );
    printf("     Class_code: %06x\n",x);
    pciGetConfigData( PCI_CACHE_LINE_SIZE, deviceNumber, &x );
    printf("Cache_line_size: %02x\n",x);
    pciGetConfigData( PCI_LATENCY_TIMER, deviceNumber, &x );
    printf("  Latency_timer: %02x\n",x);
    pciGetConfigData( PCI_HEADER_TYPE, deviceNumber, &x );
    printf("    Header_type: %02x\n",x);
    pciGetConfigData( PCI_BIST, deviceNumber, &x );
    printf("           BIST: %02x\n",x);
    pciGetConfigData( PCI_BASE_ADDRESS_0, deviceNumber, &x );
    printf("    memBaseAddr: %08x\n",x);
    pciGetConfigData( PCI_INTERRUPT_LINE, deviceNumber, &x );
    printf("  Interupt_line: %02x\n",x);
    pciGetConfigData( PCI_INTERRUPT_PIN, deviceNumber, &x );
    printf("  Interrupt_pin: %02x\n",x);
    pciGetConfigData( PCI_MIN_GNT, deviceNumber, &x );
    printf("        Min_gnt: %02x\n",x);
    pciGetConfigData( PCI_MAX_LAT, deviceNumber, &x );
    printf("        Max_lat: %02x\n",x);
    if (!is3DFX) return;
    pciGetConfigData( PCI_INIT_ENABLE, deviceNumber, &x );
    printf("     initEnable: %08x\n",x);
    pciGetConfigData( PCI_CFG_STATUS, deviceNumber, &x );
    printf("      cfgStatus: %08x\n",x);
    
}

//----------------------------------------------------------------------
static int usage(void)
{
    fprintf(stderr,"usage: pcirw [-cls] [-bdv #] [[-r] #] [[-w] # #]\n");
    fprintf(stderr,"\t\t-b => baseAddress0 = #\n");
    fprintf(stderr,"\t\t-c => write to config space\n");
    fprintf(stderr,"\t\t-d => deviceID = #\n");
    fprintf(stderr,"\t\t-l => loop\n");
    fprintf(stderr,"\t\t-r => read #(address)\n");
    fprintf(stderr,"\t\t-s => display config space\n");
    fprintf(stderr,"\t\t-v => vendorID = #\n");
    fprintf(stderr,"\t\t-w => write #(address) #(data)\n");
    exit(1);
    return 0;
}

typedef enum {
        CMD_SCAN,
        CMD_BASE,
        CMD_STATUS,
        CMD_READ,
        CMD_WRITE,
} CmdCode;

// GETARG returns either the remainder of the current argument or
//      the next argument if there is one.
#define GETARG token[1] ? ctmp=token+1, token=" ", ctmp : --argc > 0 ? *++argv : (char *)usage()

//----------------------------------------------------------------------
// MAIN Program
//----------------------------------------------------------------------
int main(int argc, char **argv)
{
//    FxU32 vendorID = 0x1002;                  // ATI
//    FxU32 deviceID = 0x4758;                  // Mach64
    int configSpace = 0, loop = 0;
    FxU32 vendorID =  _3DFX_PCI_ID;             // 3Dfx
    FxU32 deviceID = 0xFFFF;                    // any card
    FxI32 sizeOfCard = 0x1000000;               // 16 Mbytes
    FxU32 deviceNumber;
        FxU32 instance = 0;
    FxU32 *sst, addr,data;
    CmdCode cmd = CMD_SCAN;

    //------------------------------------------------------------------
    // parse the command line
    while (--argc > 0 && **++argv == '-') {
        char *token, *ctmp;

        for (token = argv[0] + 1; *token; token++) 
        switch (*token) {
                case '?':
                        usage();
                        break;
                case 'b':       // set the baseAddress of the card to this value
                        sscanf(GETARG,"%i",&addr);
                        cmd = CMD_BASE;
                        break;
                case 'c':
                        configSpace = 1;
                        break;
                case 'd':       // override default deviceID
                        sscanf(GETARG,"%i",&deviceID);
                        break;
                case 'i':       // override default instance
                        sscanf(GETARG,"%i",&instance);
                        break;
                case 'l':
                        loop = 1;
                        break;
                case 'r':
                        cmd = CMD_READ;
                        sscanf(GETARG,"%i",&addr);
                        break;
                case 's':
                        cmd = CMD_STATUS;
                        break;
                case 'v':       // override default deviceID
                        sscanf(GETARG,"%i",&vendorID);
                        break;
                case 'w':
                        cmd = CMD_WRITE;
                        sscanf(GETARG,"%i",&addr);
                        sscanf(GETARG,"%i",&data);
                        break;
                default:
                        fprintf(stderr,"illegal option %c\n", *token);
                        break;
        }
    }

    if (argc > 2) usage();
    if (argc > 0) {
        sscanf(argv[0],"%i",&addr);
        if (argc > 1) {
            cmd = CMD_WRITE;
            sscanf(argv[1],"%i",&data);
        }
        else cmd = CMD_READ;
    }

    // if scanning or status, then do it and exit.
    if (cmd == CMD_SCAN) {
        cmdScan();
        return 0;
    }
    if (cmd == CMD_STATUS) {                    // display PCI status
        if (pciFindCard(vendorID, deviceID, &deviceNumber))
            cmdStatus(deviceNumber, vendorID == _3DFX_PCI_ID);
        else {
            fprintf(stderr, "error: could not find card 0x%04x,0x%04x.\n",
                        vendorID, deviceID);
            exit(2);
        }
        
        return 0;
    }
    if (cmd == CMD_BASE) sizeOfCard = -1;       // hack

    //------------------------------------------------------------------
    // find and map the card into virtual memory
    //sst = pciMapCard(vendorID,deviceID,sizeOfCard, &deviceNumber);
    sst = pciMapCardMulti(vendorID,deviceID,sizeOfCard, &deviceNumber,
                instance, 0);
    if (sst == NULL) {
        fprintf(stderr, "error: could not find card 0x%04x,0x%04x.\n",
                        vendorID, deviceID);
        exit(2);
    }

    //------------------------------------------------------------------
    // execute the requested action
    switch(cmd) {
        case CMD_BASE:                  // change the base address of the card
                        pciSetConfigData( PCI_BASE_ADDRESS_0, deviceNumber, &addr );
                        break;

        case CMD_READ:                          // read a DWORD
                        if (addr & 0x3) {
                            fprintf(stderr,"unaligned accesses NYI, addr=0x%x\n",addr);
                            exit(4);
                        }
                        addr >>= 2;             // make it a dword index
                        data = sst[addr];
                        fprintf(stdout,"read 0x%08x(%d) from 0x%x\n",data,data,addr<<2);
                        while (loop) data = sst[addr];
                        break;

        case CMD_WRITE:                         // write a DWORD
                        if (addr & 0x3) {
                            fprintf(stderr,"unaligned accesses NYI, addr=0x%x\n",addr);
                            exit(4);
                        }
                        if (configSpace) {
                            int i;
                            for (i=0; i<sizeof(pciRegArray)/sizeof(*pciRegArray); i++) {
                                if (pciRegArray[i]->regAddress == addr) {
                                    if (pciRegArray[i]->rwFlag == READ_ONLY) {
                                        fprintf(stderr,"cannot write READ_ONLY address\n");
                                        exit(5);
                                    }
                                    pciSetConfigData( *pciRegArray[i], deviceNumber, &data );
                                    goto found;
                                }
                            }
                            fprintf(stderr,"config address register 0x%x descriptor not found\n",addr);
                            exit(6);

                        found: ;
                        }
                        else {
                            addr >>= 2;         // make it a dword index
                            do {sst[addr] = data;} while (loop);
                        }
                        break;

        default:        usage();
    }

    //----------------------------------------------------------------------
    // cleanup: unmap the card, close down the PCI bus and return
    pciUnmapPhysical( (unsigned long)sst, sizeOfCard );
    if ( !pciClose() ) {
        fprintf(stderr, pciGetErrorString());
        exit(3);
    }
    return 0;
}
