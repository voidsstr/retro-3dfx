/******************************************************************************
*
*   Module:     iNapalm.H       Napalm Device Interface Header
*
*   Revision:   1.00
*
*   Date:       August 1, 1998
*
*   Author:     Randy Spurlock
*
*******************************************************************************
*
*   Module Description:
*
*       This module contains all the data and function definitions
*   for the Napalm Simulated Device.
*
*******************************************************************************
*
*   Changes:
*
*    DATE     REVISION  DESCRIPTION                             AUTHOR
*  --------   --------  -------------------------------------------------------
*  08/01/98     1.00    Original                                Randy Spurlock
*
/******************************************************************************
*   Check for Basic Type Definitions
******************************************************************************/
#ifndef __INLP_H
#define __INLP_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef BYTE
typedef unsigned char   BYTE;           /* Define a BYTE value               */
typedef BYTE           *LPBYTE;         /* Pointer to a BYTE value           */
#endif

#ifndef WORD
typedef unsigned short  WORD;           /* Define a WORD value               */
typedef WORD           *LPWORD;         /* Pointer to a WORD value           */
#endif

#ifndef DWORD
typedef unsigned long   DWORD;          /* Define a DWORD value              */
typedef DWORD          *LPDWORD;        /* Pointer to a DWORD value          */
#endif

#ifndef VOID
typedef void            VOID;           /* Define a VOID value               */
typedef VOID           *LPVOID;         /* Pointer to a VOID value           */
#endif

#define NAPALM_VXD_ID   0x3002                  /* Napalm device VxD ID      */
#define NAPALM_NAME     "\\\\.\\Napalm.VxD"     /* Napalm device VxD name    */

/******************************************************************************
*   Define Napalm Simulated Device Type Definitions
******************************************************************************/

typedef DWORD           NLPRET;         /* Napalm API return code type       */

typedef DWORD           NLPHANDLE;      /* Napalm device handle value        */
typedef NLPHANDLE       NLPINTERFACE;   /* Napalm interface handle           */

typedef DWORD          *PNLPHANDLE;     /* Pointer to Napalm handle          */
typedef NLPINTERFACE   *PNLPINTERFACE;  /* Pointer to interface handle       */

/******************************************************************************
*   Define Napalm Simulated Device Structure Type Definitions
******************************************************************************/

/* This interface structure returns only a master chip / single config       */

typedef struct tagNLPMEMINFO
{
    DWORD dwBase0;
    DWORD dwBase1;
    DWORD dwLen0;
    DWORD dwLen1;
    DWORD dwHostV3Base0;
    DWORD dwHostV3Base1;
} NLPMEMINFO;
typedef NLPMEMINFO     *PNLPMEMINFO;

/* This interface structure returns multichip configuration                  */

typedef struct tagNLPMEMINFOEX
{
    DWORD dwBase0[4];
    DWORD dwBase1[4];
    DWORD dwLen0[4];
    DWORD dwLen1[4];
    DWORD dwHostV3Base0;
    DWORD dwHostV3Base1;
    DWORD dwNumChips;
} NLPMEMINFOEX;
typedef NLPMEMINFOEX     *PNLPMEMINFOEX;

/* This interface structure is used for reading from and writing to
   the PCI config space of the simulated device                             */

typedef struct tagNPLPCICONFIG
{
    DWORD dwBus;         /* Ignored for now                                 */
    DWORD dwDevFunc;     /* Is chip number: 0, 1, 2, 3                      */
    DWORD dwOffset;      /* Byte offset into the register space             */
    DWORD dwValue;       /* Value to write / returning read value           */
} NPLPCICONFIG;
typedef NPLPCICONFIG     *PNPLPCICONFIG;

/******************************************************************************
*   Define Napalm Simulated Device Return Code Values
******************************************************************************/

enum tagNLPRETURN {
    NLPRET_OK,                          /* Napalm API successful code        */
    NLPRET_INTERFACE_ERROR,             /* Napalm interface error code       */
    NLPRET_INVALID_API,                 /* Invalid API error code            */
    NLPRET_NULL_POINTER,                /* Null pointer error code           */
    NLPRET_INVALID_ARGUMENT             /* Invalid argument error code       */
} NLPRETURN;

/******************************************************************************
*   Define Napalm Simulated Device Function Definitions
******************************************************************************/

/* Define Napalm Simulated Device Interface API Functions */

NLPRET      Get_Napalm_Interface(PNLPINTERFACE pInterface, DWORD dwReference);
NLPRET      Release_Napalm_Interface(NLPINTERFACE hInterface);

/* Define Napalm Simulated Device Information API Functions */

NLPRET      Get_Napalm_Version(NLPINTERFACE hInterface, LPDWORD pVersion);
NLPRET      Get_Napalm_Memory_Info(NLPINTERFACE hInterface, PNLPMEMINFO pInfo);
NLPRET      Get_Napalm_Memory_InfoEx(NLPINTERFACE hInterface, PNLPMEMINFOEX pInfo);
NLPRET      Napalm_PCI_Read_Config(NLPINTERFACE hInterface, PNPLPCICONFIG pPCI);
NLPRET      Napalm_PCI_Write_Config(NLPINTERFACE hInterface, PNPLPCICONFIG pPCI);

#ifdef  __cplusplus
}
#endif

#endif

