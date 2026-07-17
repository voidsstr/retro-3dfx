/****************************************************************************
*                                                                           *
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY     *
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE       *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR     *
* PURPOSE.                                                                  *
*                                                                           *
* Copyright 1993-95  Microsoft Corporation.  All Rights Reserved.           *
*                                                                           *
****************************************************************************/

/****************************************************************************
*
* PROGRAM: CON_SAMP.C
*
* PURPOSE: Simple console application for calling CVXDSAMP (C VxD Sample) VxD
*
* FUNCTIONS:
*  main() - Console application calls VMM and VKD through CVXDSAMP which
*           supports DeviceIoControl. CVXDSAMP will return values to this
*           application through this same DeviceIoControl interface.
*
* SPECIAL INSTRUCTIONS:
*
****************************************************************************/

#include <stdio.h>
#include <windows.h>
#include <vmm.h>
#include <vxdldr.h>

#define CVXD_APIFUNC_1		1
#define CVXD_APIFUNC_2		2
#define CVXD_GARTRESERVE	3
#define CVXD_GARTFREE		4
#define CVXD_GARTCOMMIT		5
#define CVXD_GARTUNCOMMIT	6

#define CVXD_CM_LOCATE		7

#define SST_VENDOR_DEVICE_ID_H3        0x0003121a

int
main(int argc, char **argv)
{
    HANDLE      hCVxD = 0;
    DWORD       cbBytesReturned;
    DWORD       dwErrorCode;
    DWORD       RetInfo[2];
    DWORD       parmInfo[3];
    DWORD gartLinAddr, gartPhysAddr, gartCommitPhys;
    DWORD devNode;

    // Dynamically load and prepare to call CVXDSAMP
    // The CREATE_NEW flag is not necessary
    hCVxD = CreateFile("\\\\.\\3DFXGART.VXD", 0,0,0,
                        CREATE_NEW, FILE_FLAG_DELETE_ON_CLOSE, 0);

    if ( hCVxD == INVALID_HANDLE_VALUE )
    {
        dwErrorCode = GetLastError();
        if ( dwErrorCode == ERROR_NOT_SUPPORTED )
        {
            printf("Unable to open VxD, \n device does not support DeviceIOCTL\n");
        }
        else
        {
            printf("Unable to open VxD, Error code: %lx\n", dwErrorCode);
        }
    }
    else
    {
	//char *devIdString = "\\HKEY_LOCAL_MACHINE\\Enum\\PCI\\VEN_121A&DEV_0003";
	//char *devIdString = "BUS_00&DEV_0D&FUNC_00";
	
	
	// locate the banshee devNode
	parmInfo[0] = SST_VENDOR_DEVICE_ID_H3;
	
        if ( DeviceIoControl(hCVxD, CVXD_CM_LOCATE,
			     (LPVOID)parmInfo, sizeof(parmInfo),
			     (LPVOID)RetInfo, sizeof(RetInfo),
			     &cbBytesReturned, NULL) )
	{
	    devNode = RetInfo[0];
	    if (devNode != 0)
	    {
		printf("located pciID 0x%08lx, devNode = 0x%08lx\n",
		       SST_VENDOR_DEVICE_ID_H3, RetInfo[0]);
	    }
	    else
	    {
		printf("did not locate pciID 0x%08lx\n",
		       SST_VENDOR_DEVICE_ID_H3);
	    }
	}
	else
	{
	    printf("did not locate pciID 0x%08lx\n", SST_VENDOR_DEVICE_ID_H3);
	}
	
	if (argc <= 1)
	    return 0;
	
        // Make Gart Reserve call here
	parmInfo[0] = 1024;	// # of pages to reserve
	parmInfo[1] = devNode;

        if ( DeviceIoControl(hCVxD, CVXD_GARTRESERVE,
                (LPVOID)parmInfo, sizeof(parmInfo),
                (LPVOID)RetInfo, sizeof(RetInfo),
                &cbBytesReturned, NULL) )
        {
	    gartLinAddr = RetInfo[0];
	    gartPhysAddr = RetInfo[1];
            printf("return value from GartReserve: 0x%08lx\n", gartLinAddr);
            printf("AGP memory GART physical address: 0x%08lx\n",gartPhysAddr);
        }
        else
        {
            printf("GARTReserve call: Something went wrong!\n");
        }

        // Make Gart Commit call here
	parmInfo[0] = gartLinAddr;	// base address of range to commit
	parmInfo[1] = 0;		// page offset from base to commit
	parmInfo[2] = 1024;		// # of pages to commit at offset

        if ( DeviceIoControl(hCVxD, CVXD_GARTCOMMIT,
                (LPVOID)parmInfo, sizeof(parmInfo),
                (LPVOID)RetInfo, sizeof(RetInfo),
                &cbBytesReturned, NULL) )
        {
	    gartCommitPhys = RetInfo[1];
            printf("committed CPU addr ");
	    printf("0x%08lx to GART address 0x%08lx (retval:%d)\n",
		   gartLinAddr, gartCommitPhys, RetInfo[0]);
        }
        else
        {
            printf("GARTCommit call: Something went wrong!\n");
        }

        // Make Gart Uncommit call here
	parmInfo[0] = gartLinAddr;	// base address of range to uncommit
	parmInfo[1] = 0;		// page offset from base to uncommit
	parmInfo[2] = 1024;		// # of pages to uncommit at offset

        if ( DeviceIoControl(hCVxD, CVXD_GARTUNCOMMIT,
                (LPVOID)parmInfo, sizeof(parmInfo),
                (LPVOID)RetInfo, sizeof(RetInfo),
                &cbBytesReturned, NULL) )
        {
            printf("uncommitted GART allocation at CPU addr ");
	    printf("0x%08lx\n", gartLinAddr);
        }
        else
        {
            printf("GARTUncommit call: Something went wrong!\n");
        }


        // Make Gart Free call here
	parmInfo[0] = gartLinAddr;

        if ( DeviceIoControl(hCVxD, CVXD_GARTFREE,
                (LPVOID)parmInfo, sizeof(parmInfo),
                (LPVOID)RetInfo, sizeof(RetInfo),
                &cbBytesReturned, NULL) )
        {
            printf("freed GART allocation from CPU addr 0x%lx\n", gartLinAddr);
        }
        else
        {
            printf("GARTFree call: Something went wrong!\n");
        }

        // Dynamically UNLOAD the C Virtual Device sample.
        CloseHandle(hCVxD);
    }
    return(0);
}
