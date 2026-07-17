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
** $Revision: 3$
** $Date: 10/11/00 7:37:39 PM$
*/

//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>

#include "fxmemmap.h"     // Control Codes for our Vxd



DWORD Physical [2];       // Physical address[0] & size[1]
DWORD Linear [2];         // Linear address[0] & size[1]

#define TESTLDT  0        // If you need to use selectors you could 
                          // use int 31h DMPI calls from a 16-bit app/DLL
                          // instead of our VxD. The DeviceIoControl
                          // interface is callable from a Win32 app/DLL
                          // under Windows 95. Int 31h calls are not.

#if TESTLDT
DWORD LDTSelector [2];    // Created LDTSelector[0]
DWORD LDTFreeResult [2];  // LDTFreeResult[0]
#endif


int main(int argc, char *argv[])
{
    HANDLE hDevice;
    BOOL   waitFlag = 0;
    int    i, j;

    hDevice = CreateFile("\\\\.\\FXMEMMAP.VXD", 0, 0, NULL, 0,
        FILE_FLAG_DELETE_ON_CLOSE, NULL);
    if (hDevice != INVALID_HANDLE_VALUE)
    {
        DWORD version;
        DWORD nret;
        LPDWORD pPhysical = Physical;
        LPDWORD pLinear = Linear;

#if TESTLDT
        LPDWORD pLDTSelector = LDTSelector;
        LPDWORD pLDTFreeResult = LDTFreeResult;
#endif
        Physical[0] = 0xFB000000;    // Whatever works on your machine
        Physical[1] = 0x01000000;
	for (i=1, j=0; i<argc; i++) {
            if (!strcmp(argv[1], "-w")) {
                waitFlag = 1;
            } else {
                sscanf(argv[i],"%lx",&Physical[j++]);
	    }
	}

        if (argc < 3) {
	   printf("usage: vxdtest [-w] [address [size]]\n");
	   printf("       -w       wait for keypress after loading vxd\n");
	   printf("       address  physical address (default 0xFB000000)\n");
	   printf("       size     aperature size   (default 0x01000000)\n");
        }

        DeviceIoControl(hDevice, GETAPPVERSIONDWORD, NULL, 0, &version, 
            sizeof(version), &nret, NULL);
        printf("FXMEMMAP.VXD is version %d.%02d\n", HIBYTE(version),
            LOBYTE(version));
        printf("FXMEMMAP.VXD instance %d\n", (version)>>16);


        printf("Physical address:  %x\n",Physical[0]);
        printf("Physical size:  %x\n",Physical[1]);

        DeviceIoControl(hDevice, GETLINEARADDR, 
            &pPhysical, sizeof(pPhysical), 
            &pLinear, sizeof(pLinear), 
            &nret, NULL);

        printf("Mapped linear address is %x\n", Linear[0]); 

        DeviceIoControl(hDevice, GETAPPVERSIONDWORD, NULL, 0, &version, 
            sizeof(version), &nret, NULL);
        printf("FXMEMMAP.VXD is version %d.%02d\n", HIBYTE(version),
            LOBYTE(version));
        printf("FXMEMMAP.VXD instance %d\n", (version)>>16);


       if (waitFlag) getch();
        // If Linear[0] is non-zero you can access the memory now.

#if TESTLDT
        DeviceIoControl(hDevice, GETLDTSELECTOR, 
            &pLinear, sizeof(pLinear), 
            &pLDTSelector, sizeof(pLDTSelector), 
            &nret, NULL);

        printf("Allocated LDTSelector[0] is %x\n", LDTSelector[0]);

        DeviceIoControl(hDevice, FREELDTSELECTOR, 
            &pLDTSelector, sizeof(pLDTSelector), 
            &pLDTFreeResult, sizeof(pLDTFreeResult), 
            &nret, NULL);

        printf("LDTFreeResult[0] is %x\n", LDTFreeResult[0]);
#endif

        CloseHandle(hDevice);
    }
    return 0;
}

