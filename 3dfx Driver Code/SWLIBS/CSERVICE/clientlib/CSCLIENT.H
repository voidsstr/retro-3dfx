/*
** Copyright (c) 1996-2000, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   csclient.h
**
** Description: Main include file for CSCLIENT.LIB
**
** $Log: 
**  20   Napalm R1.51.15.1.3    10/24/00 Don Fowler      Added CSTIME structure to
**       debug system and CSGRAPHICALCONTEXT structure. De-coupled
**       CSGRAPHICALCONTEXT from CS_BUFFER_TEXTURE_HEAP type in all functions that
**       deal with this memory type. 
**  19   Napalm R1.51.15.1.2    10/11/00 Brent           Forced check in to enforce
**       branch.
**  18   Napalm R1.51.15.1.1    07/20/00 Andrew  Bell    Rolling back all swlibs
**       files to 2nd latest version due to an incorrect checkin.  This should
**       bring things back to normal.
**  17   Napalm R1.51.15.1.0    07/19/00 Dinesh Raja Savari Amirtharaj CSIM for the
**       RTL release_3_2
**  16   3dfx      1.15        03/09/00 Don Fowler      Modified include file
**       structure to separate out a server-specific file and a client-specific
**       file. 
**  15   3dfx      1.14        03/08/00 Don Fowler      Added an FxFAR define for
**       16 bit code. Fixed a minor type check in GetClipListFrom DirectDraw
**  14   3dfx      1.13        03/06/00 Don Fowler      Removed the inclusion of
**       csvalues.h.. 
**  13   3dfx      1.12        03/05/00 Don Fowler      Removed the notion of a
**       device context from the code. Since a graphical context is locked to a
**       window the same as a device context it was a redundant notion. 
**  12   3dfx      1.11        03/03/00 Don Fowler      Incremental development
**  11   3dfx      1.10        03/03/00 Don Fowler      Incremental development
**  10   3dfx      1.9         03/02/00 Don Fowler      Incremental development
**  9    3dfx      1.8         03/01/00 Don Fowler      Incremental development
**  8    3dfx      1.7         02/29/00 Don Fowler      Incremental development
**  7    3dfx      1.6         02/28/00 Don Fowler      Incremental development
**  6    3dfx      1.5         02/28/00 Don Fowler      Incremental development
**  5    3dfx      1.4         02/27/00 Don Fowler      Incremental development
** 
**  4    3dfx      1.3         02/27/00 Don Fowler      Incremental development
**  3    3dfx      1.2         02/27/00 Don Fowler      Incremental development
**  2    3dfx      1.1         02/27/00 Don Fowler      Incremental development
**  1    3dfx      1.0         02/27/00 Don Fowler      
** $
**
*/
#ifndef _CSCLIENT_H
#define _CSLCIENT_H

#include "3dfx.h"



/* operating system specific includes */
#ifdef _WIN32 
#include <windows.h>
#include <winbase.h>
#include <stdio.h>
#include "ddraw.h"
#endif

#include "csapi.h"
#include "debug.h"

/* Private functions to this library */
PCSSLLISTNODE FX_CALL csSLLISTGetNext( PCSSLLISTNODE psRootNode );
FxVOID FX_CALL csSLLISTRemove( PCSSLLISTNODE psRootNode, PCSSLLISTNODE psNodeToRemove );
FxVOID FX_CALL csSLLISTInsert( PCSSLLISTNODE psRootNode, PCSSLLISTNODE psNodeToInsert );
#if (defined(WIN32) || (defined(WIN95) && defined(IS_16)))
LRESULT CALLBACK csSnoopWindowProc( int nCode, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK csSnoopKeyboardProc( int nCode, WPARAM wParam, LPARAM lParam );
#endif /* WIN32 */

/* Structure to store information global to the CSCLIENT.LIB */
#define CSCLIENTLIBDATA_FLAGS_INITIALIZED  0x00000001
typedef struct csclientlibdata_s
{
    CSSLLISTNODE sRootNodeGraphicalContext; /* Singly-linked list root node.. points to the list of 
                                   graphical contexts. */
    FxU32 u32Flags;             /* Flags for CSCLIENT.LIB */
                                /* CSCLIENTLIBDATA_FLAGS_INITIALIZED - Client library is initialized */    
    PCSINIT psInit;             /* Pointer to the initialization structure */
#if (defined(WIN32) || (defined(WIN95) && defined(IS_16)))
    CSTIME sProcessTimeStamp;   /* Time stamp taken from when csInit was called for the current
                                   process */
    HDC hDesktopDC;             /* DC of the desktop in WIN32 */  
    LPDIRECTDRAW psDirectDraw;  /* Direct Draw main object pointer */
    HHOOK hWindowSnoopHook;     /* Hook handle from setting the WindowProc snooping hook */
    HHOOK hKeyboardSnoopHook;   /* Hook handle from setting the KeyboardProc snooping hook */
    OSVERSIONINFO sOSVersionInfo; /* Version information of the operating system */
    FxU32 u32SnoopHooksEnabled; /* Flag to protect our snoop functions from crashing */ 
#endif /* WIN32 */

} CSCLIENTLIBDATA,*PCSCLIENTLIBDATA;

#include "data.h"

#endif /* _CSCLIENT_H */
