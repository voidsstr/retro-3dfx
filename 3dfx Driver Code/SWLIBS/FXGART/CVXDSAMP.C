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

#define WANTVXDWRAPS

#include <basedef.h>
#include <vmm.h>
#include <debug.h>
#include <vxdwraps.h>
#include <vwin32.h>
#include <winerror.h>
#include <configmg.h>
#include <pci.h>
#include "myvkd.h"
#include "vkdwraps.h"

#define CVXD_VERSION 0x400

#define CVXD_V86_FUNCTION1 1
#define CVXD_V86_FUNCTION2 2
#define CVXD_PM_FUNCTION1  1
#define CVXD_PM_FUNCTION2  2

typedef DIOCPARAMETERS *LPDIOC;

#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG

HVM hSysVM;

DWORD _stdcall CVXD_W32_DeviceIOControl(DWORD, DWORD, DWORD, LPDIOC);
DWORD _stdcall CVXD_CleanUp(void);
DWORD _stdcall CVXD_W32_Proc1(DWORD, DWORD, LPDIOC);
DWORD _stdcall CVXD_W32_Proc2(DWORD, DWORD, LPDIOC);

DWORD _stdcall GartReserve(DWORD, DWORD, LPDIOC);
DWORD _stdcall GartFree(DWORD, DWORD, LPDIOC);
DWORD _stdcall GartCommit(DWORD, DWORD, LPDIOC);
DWORD _stdcall GartUncommit(DWORD, DWORD, LPDIOC);
DWORD _stdcall CM_Locate(DWORD dwDDB, DWORD hDevice, LPDIOC lpDIOCParms);

DWORD ( _stdcall *CVxD_W32_Proc[] )(DWORD, DWORD, LPDIOC) = 
{
        CVXD_W32_Proc1,
        CVXD_W32_Proc2,
	GartReserve,
	GartFree,
	GartCommit,
	GartUncommit,
	CM_Locate
};

#define MAX_CVXD_W32_API (sizeof(CVxD_W32_Proc)/sizeof(DWORD))

/****************************************************************************
                  CVXD_W32_DeviceIOControl
****************************************************************************/
DWORD _stdcall CVXD_W32_DeviceIOControl(DWORD  dwService,
                                        DWORD  dwDDB,
                                        DWORD  hDevice,
                                        LPDIOC lpDIOCParms)
{
    DWORD dwRetVal = 0;

    // DIOC_OPEN is sent when VxD is loaded w/ CreateFile 
    //  (this happens just after SYS_DYNAMIC_INIT)
    if ( dwService == DIOC_OPEN )
    {
        _Debug_Printf_Service("3DfxGart: DIOC Open\n\r");
        // Must return 0 to tell WIN32 that this VxD supports DEVIOCTL
        dwRetVal = 0;
    }
    // DIOC_CLOSEHANDLE is sent when VxD is unloaded w/ CloseHandle
    //  (this happens just before SYS_DYNAMIC_EXIT)
    else if ( dwService == DIOC_CLOSEHANDLE )
    {
        // Dispatch to cleanup proc
        dwRetVal = CVXD_CleanUp();
    }
    else if ( dwService > MAX_CVXD_W32_API )
    {
        // Returning a positive value will cause the WIN32 DeviceIOControl
        // call to return FALSE, the error code can then be retrieved
        // via the WIN32 GetLastError
        dwRetVal = ERROR_NOT_SUPPORTED;
    }
    else
    {
        // CALL requested service
        dwRetVal = (CVxD_W32_Proc[dwService-1])(dwDDB, hDevice, lpDIOCParms);
    }
    return(dwRetVal);
}

DWORD _stdcall CVXD_W32_Proc1(DWORD dwDDB, DWORD hDevice, LPDIOC lpDIOCParms)
{
    PDWORD pdw;

    _Debug_Printf_Service("3DfxGart: CVXD_W32_Proc1\n\r");

    pdw = (PDWORD)lpDIOCParms->lpvOutBuffer;
    hSysVM = Get_Sys_VM_Handle();
    pdw[0] = hSysVM;
    pdw[1] = Get_Execution_Focus();

    return(NO_ERROR);
}

DWORD _stdcall CVXD_W32_Proc2(DWORD dwDDB, DWORD hDevice, LPDIOC lpDIOCParms)
{
    PDWORD pdw;

    _Debug_Printf_Service("3DfxGart: CVXD_W32_Proc2\n\r");

    pdw = (PDWORD)lpDIOCParms->lpvOutBuffer;
    *pdw = hSysVM;
#if 0
    pdw[1] = VKD_Get_Kbd_Owner();
#endif

    return(NO_ERROR);
}


DWORD VXDINLINE
VgartD_Reserve(DWORD devObj,
	       DWORD numPages,
	       DWORD alignMask,
	       DWORD pGARTDev,
	       DWORD flags)
{
    DWORD retval;

    __asm pushad;
    
    __asm push flags;
    __asm push pGARTDev;
    __asm push alignMask;
    __asm push numPages;
    __asm push devObj;
    VMMCall(_GARTReserve);
    __asm mov  retval, eax;
    __asm add  esp, 5*4;

    __asm popad;

    return(retval);
}



DWORD _stdcall
GartReserve(DWORD dwDDB, DWORD hDevice, LPDIOC lpDIOCParms)
{
    PDWORD pdwIn;
    PDWORD pdwOut;
    DWORD gartAddr;

    _Debug_Printf_Service("3DfxGart: GartReserve: ");
    pdwIn = (PDWORD)lpDIOCParms->lpvInBuffer;
    pdwOut = (PDWORD)lpDIOCParms->lpvOutBuffer;

    pdwOut[0] = VgartD_Reserve(pdwIn[1],		// devObj
			       pdwIn[0],		// numPages
			       0,			// 4K aligned address
			       (DWORD)&pdwOut[1],	// out: gart address
			       PG_WRITECOMBINED);	// duh

    _Debug_Printf_Service("NP:0x%lx, LA:0x%08lx, GA:0x%08lx, %s\n\r",
			  pdwIn[0], pdwOut[0], pdwOut[1],
			  ((pdwOut[0] != 0) ? "Succeeded" : "FAILED"));

    return(NO_ERROR);
}



DWORD VXDINLINE
VgartD_Commit(DWORD gartLinAddr,
	      DWORD pageOffset,
	      DWORD numPages,
	      DWORD pGARTDev,
	      DWORD flags)
{
    DWORD retval;

    __asm pushad;
    
    __asm push flags;
    __asm push pGARTDev;
    __asm push numPages;
    __asm push pageOffset;
    __asm push gartLinAddr;
    VMMCall(_GARTCommit);
    __asm mov  retval, eax;
    __asm add  esp, 5*4;

    __asm popad;

    return(retval);
}

DWORD _stdcall
GartCommit(DWORD dwDDB, DWORD hDevice, LPDIOC lpDIOCParms)
{
    PDWORD pdwOut;
    PDWORD pdwIn;
    DWORD gartAddr;

    _Debug_Printf_Service("3DfxGart: GartCommit: ");
    pdwOut = (PDWORD)lpDIOCParms->lpvOutBuffer;
    pdwIn = (PDWORD)lpDIOCParms->lpvInBuffer;

    pdwOut[0] = VgartD_Commit(pdwIn[0],		// cpu lin addr to commit
			      pdwIn[1],		// page offset
			      pdwIn[2],		// # pages to commit
			      (DWORD)&pdwOut[1], // out: gart address
			      0);		 // don't memfill(0) memory

    _Debug_Printf_Service("LA:0x%08lx, PO:0x%lx, NP:0x%lx, GA:0x%08lx, %s\n\r",
		     pdwIn[0], pdwIn[1], pdwIn[2], pdwOut[1],
		     ((pdwOut[0] == 1) ? "Succeeded" : "FAILED"));

    return(NO_ERROR);
}



void VXDINLINE
VgartD_Uncommit(DWORD gartLinAddr,
		DWORD pageOffset,
		DWORD numPages)
{
    __asm pushad;
    
    __asm push numPages;
    __asm push pageOffset;
    __asm push gartLinAddr;
    VMMCall(_GARTUnCommit);
    __asm add  esp, 3*4;

    __asm popad;
}

DWORD _stdcall
GartUncommit(DWORD dwDDB, DWORD hDevice, LPDIOC lpDIOCParms)
{
    PDWORD pdwIn;
    DWORD gartAddr;

    _Debug_Printf_Service("3DfxGart: GartUncommit: ");
    pdwIn = (PDWORD)lpDIOCParms->lpvInBuffer;

    VgartD_Uncommit(pdwIn[0],		// base addr to Uncommit
		    pdwIn[1],		// page offset from base to uncommit
		    pdwIn[2]);		// # pages to uncommit

    _Debug_Printf_Service("LA:0x%08lx, PO:0x%lx, NP:0x%lx\n\r",
		     pdwIn[0], pdwIn[1], pdwIn[2]);

    return(NO_ERROR);
}




void VXDINLINE
VgartD_Free(DWORD gartLinAddr)
{
    __asm pushad;
    
    __asm push gartLinAddr;
    VMMCall(_GARTFree);
    __asm add  esp, 1*4;

    __asm popad;
}

DWORD _stdcall
GartFree(DWORD dwDDB, DWORD hDevice, LPDIOC lpDIOCParms)
{
    PDWORD pdwIn;

    _Debug_Printf_Service("3DfxGart: GartFree: ");

    pdwIn = (PDWORD)lpDIOCParms->lpvInBuffer;

    VgartD_Free(pdwIn[0]);

    _Debug_Printf_Service("LA: 0x%08lx\n\r", pdwIn[0]);

    return(NO_ERROR);
}



//
// traverse the h/w node tree, looking for a devnode that
// has the matches given pciID (== PCI vendorid and deviceid)
//
int
search_CM_for_PCI_device(DWORD pciID,
			 DEVNODE currentDN,
			 DEVNODE *foundDN)
{
    DWORD status, problem;
    DWORD dwDevID;
    DEVNODE childDN, siblingDN;
    DWORD retval;
    
    retval = CM_Call_Enumerator_Function(currentDN,
					 PCI_ENUM_FUNC_GET_DEVICE_INFO,
					 0, &dwDevID, sizeof(DWORD), 0 );
    // look for exit condition
    //
    if ((retval == CR_SUCCESS) && (dwDevID == pciID))
    {
	*foundDN = currentDN;
	return 1;
    }
    
    // not this node, try it's children if it has any
    //
    if (CM_Get_Child(&childDN, currentDN, 0) == CR_SUCCESS)
    {
	if (search_CM_for_PCI_device(pciID, childDN, foundDN))
	    return 1;
    }

    // still no luck, try all the siblings and their subtrees
    //
    if (CM_Get_Sibling(&siblingDN, currentDN, 0) == CR_SUCCESS)
    {
	if (search_CM_for_PCI_device(pciID, siblingDN, foundDN))
	    return 1;
    }

    // didn't find target device in this node's subtree/sibling tree
    //
    return 0;
}


DWORD _stdcall
CM_Locate(DWORD dwDDB, DWORD hDevice, LPDIOC lpDIOCParms)
{
    PDWORD pdwIn, pdwOut;
    DEVNODE devNode, rootDevNode;
    CONFIGRET retval;
    char buf[255] = "";
    DWORD dwDevID = 0xdeadbeef;
    
    _Debug_Printf_Service("3DfxGart: CM_Locate: ");
    pdwIn = (PDWORD)lpDIOCParms->lpvInBuffer;
    pdwOut = (PDWORD)lpDIOCParms->lpvOutBuffer;

    pdwOut[0] = 0;
    
    if (CM_Locate_DevNode(&rootDevNode, buf, 0) != CR_SUCCESS)
    {
	_Debug_Printf_Service("couldn't find root devNode!\n\r");
	return(NO_ERROR);
    }

    if (search_CM_for_PCI_device(pdwIn[0], rootDevNode, &devNode))
    {
	// found it!
	_Debug_Printf_Service("found devNode! pciID:0x%08lx, devNode"
			      ":0x%08lx\n\r", pdwIn[0], devNode);
	if (CM_Get_Device_ID(devNode, buf, sizeof(buf), 0) == CR_SUCCESS)
	    _Debug_Printf_Service("deviceID:\"%s\"\n\r", buf);

	pdwOut[0] = devNode;
    }
    else
    {
	_Debug_Printf_Service("did not find devNode for device ID "
			      "0x%08lx\n\r");
	pdwOut[0] = 0;
    }

    return(NO_ERROR);
}




DWORD _stdcall CVXD_Dynamic_Exit(void)
{
    _Debug_Printf_Service("3DfxGart: Dynamic Exit\n\r");

    return(VXD_SUCCESS);
}

DWORD _stdcall CVXD_CleanUp(void)
{
    _Debug_Printf_Service("3DfxGart: DIOC close\n\r");
    return(VXD_SUCCESS);
}






/****************************************************************************
 *                CVXD_VMAPI
 *
 *    ENTRY: function - the function number (passed in eax)
 *        parm1 - parameter 1 (passed in ebx)
 *        parm2 - parameter 2 (passed in ecx)
 *
 *    EXIT:    NONE
 ***************************************************************************/
int _stdcall CVXD_VMAPI(unsigned int function,
                        unsigned int parm1,
                        unsigned int parm2)
{
    int retcode;

    _Debug_Printf_Service("******* CVXD: V86 API Call\n\r");

    switch (function)
        {
        case CVXD_V86_FUNCTION1:
            retcode = V86Func1(parm1);
        break;

        case CVXD_V86_FUNCTION2:
            retcode = V86Func2(parm1, parm2);
        break;

        default:
            retcode = FALSE;
        break;
        }

    return (retcode);
}

/****************************************************************************
 *                CVXD_PMAPI
 *
 *    ENTRY: function - the function number (passed in eax)
 *        parm1 - parameter 1 (passed in ebx)
 *        parm2 - parameter 2 (passed in ecx)
 *
 *    EXIT:    NONE
 ***************************************************************************/
int _stdcall CVXD_PMAPI(unsigned int function,
                        unsigned int parm1,
                        unsigned int parm2)
{
    int retcode;

    switch (function)
        {
        case CVXD_PM_FUNCTION1:
            retcode = PMFunc1(parm1);
        break;

        case CVXD_PM_FUNCTION2:
            retcode = PMFunc2(parm1, parm2);
        break;

        default:
            retcode = 0;
        break;
        }

    return (retcode);
}


/****************************************************************************
 *                V86Func1
 *
 *    ENTRY: parm1 - sample parameter
 *
 *    EXIT: 1
 ***************************************************************************/
int V86Func1(unsigned int parm1)
{
    _Debug_Printf_Service("******* CVXD: V86 API 2 Call\n\r");
    return (1);
}

/****************************************************************************
 *                V86Func2
 *
 *    ENTRY: parm1 - sample parameter1
 *           parm2 - sample parameter2
 *
 *    EXIT: 2
 ***************************************************************************/
int V86Func2(unsigned int parm1,
         unsigned int parm2)
{
    _Debug_Printf_Service("******* CVXD: V86 API 2 Call\n\r");
    return (2);
}

/****************************************************************************
 *                PMFunc1
 *
 *    ENTRY: parm1 - sample parameter
 *
 *    EXIT: 1
 ***************************************************************************/
int PMFunc1(unsigned int parm1)
{
    _Debug_Printf_Service("******* CVXD: PM API 1 Call\n\r");
    return (1);
}

/****************************************************************************
 *                PMFunc2
 *
 *    ENTRY: parm1 - sample parameter1
 *           parm2 - sample parameter2
 *
 *    EXIT: 2
 ***************************************************************************/
int PMFunc2(unsigned int parm1,
        unsigned int parm2)
{
    _Debug_Printf_Service("******* CVXD: PM API 2 Call\n\r");
    return (2);
}

#pragma VxD_ICODE_SEG
#pragma VxD_IDATA_SEG

DWORD _stdcall CVXD_Dynamic_Init(void)
{
    _Debug_Printf_Service("\n\r3DfxGart: Dynamic Init\n\r");

    return(VXD_SUCCESS);
}



