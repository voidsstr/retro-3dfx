/* $Header: csapi.h, 9, 10/12/00 1:17:21 PM, Don Fowler$ */
/*
** Copyright (c) 1996-1999, 3Dfx Interactive, Inc.
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
** File name:   csapi.h
**
** Description: Prototypes and constants for the CS client API
**
** $Log: 
**  9    Napalm R1.51.4.1.3     10/12/00 Don Fowler      Added type CSDCID for OS
**       independent device contexts
**       Changed all ExtEscape calls to happen from the DC of the desktop because
**       no clients currently known will be multi-monitor compatible.
**  8    Napalm R1.51.4.1.2     10/11/00 Brent           Forced check in to enforce
**       branch.
**  7    Napalm R1.51.4.1.1     10/11/00 Ryan Bissell    Modifications needed for
**       OGL.
**  6    Napalm R1.51.4.1.0     07/13/00 Don Fowler      Changed csExecuteCommands
**       so that the server could be responsible for adding the sentinel buffer
**       write packets to the end of the command buffer. This required the sentinel
**       serial number and the sentinel buffer allocation descriptor to be passed
**       to the api and then on to the server.
**  5    3dfx      1.4         03/29/00 Don Fowler      Test APP: Made render
**       buffers load 640x480x24bpp images test1,2.bmp::Moved csInit and csUnInit
**       into the testdll::Added u32Locale to the csAlloc function calls
** 
**       Test DLL: Added csInit and csUnInit to the DETACH and ATTACH portions fo
**       the DLL::Removed csReleaseResourcesForProcess because the DLL is already
**       process-relative. 
** 
**       Client Library : Moved csReleaseResourcesForProcess to static. 
**  4    3dfx      1.3         03/10/00 Don Fowler      Re moved csRegisterRead and
**       csRegisterWrite and added csDeviceSpecificCoummunication  to supercede
**       these functions.
** 
**       Added register reading and writing information to CSSST2
**  3    3dfx      1.2         03/09/00 Don Fowler      Modified csAlloc to take
**       u32StateSize and u32CommandSize. These changes went into the debug system
**       and into CSALLOCREQ and CSALLOCRES
**  2    3dfx      1.1         03/09/00 Don Fowler      Removed CSRESULT
**  1    3dfx      1.0         03/09/00 Don Fowler      
** $
**
*/

#ifndef __CSAPI_H__
#define __CSAPI_H__

#include "cstypes.h"

#ifdef __cplusplus
extern "C"
{
#endif



/*
**
**
** These, on the other hand, really are what they say they are
*/


CSRESULT FX_CALL csInit           ( PCSINIT psInit );
CSRESULT FX_CALL csUnInit         ( FxVOID );

CSRESULT FX_CALL csGetProtocolRevision( CSDCID idDeviceContext,
                            FxU32 FxFAR*pu32Major, 
                            FxU32 FxFAR*pu32Minor );
CSRESULT FX_CALL csGetGraphicalContext( CSDCID idDeviceContext,
                            PCSGRAPHICALCONTEXT psGraphicalContext );
CSRESULT FX_CALL csFree           ( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSALLOCATIONDESCRIPTOR psAllocationDescriptor );
CSRESULT FX_CALL csSetVideoMode   ( PCSGRAPHICALCONTEXT psGraphicalContext,  
                            PCSVIDEOMODE        psVideoMode,
                            CSWINDOWID          idWindow ); 
CSRESULT FX_CALL csAcquireOverlay ( PCSGRAPHICALCONTEXT psGraphicalContext,
                            PCSOVERLAY          psOverlay );
CSRESULT FX_CALL csExecuteCommands( PCSGRAPHICALCONTEXT psGraphicalContext,  
                            PCSALLOCATIONDESCRIPTOR psStateAllocationDescriptor,
                            PCSALLOCATIONDESCRIPTOR psCommandAllocationDescriptor,
                            PCSALLOCATIONDESCRIPTOR psSentinelAllocationDescriptor,
                            FxU32 u32StateOffset,
                            FxU32 u32CommandOffset,
                            FxU32 u32StateSize,
                            FxU32 u32CommandSize,
                            FxU32 u32SentinelSerial );
CSRESULT FX_CALL csLock           ( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSALLOCATIONDESCRIPTOR psAllocationDescriptor );
CSRESULT FX_CALL csUnlock         ( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSALLOCATIONDESCRIPTOR psAllocationDescriptor );
CSRESULT FX_CALL csReleaseGraphicalContext( PCSGRAPHICALCONTEXT psGraphicalContext );
CSRESULT FX_CALL csAlloc          ( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSALLOCATIONDESCRIPTOR psAllocationDescriptor );
PFxSz    FX_CALL            csDEBUGGetStringForError( CSRESULT idError );
CSRESULT FX_CALL csSwapBufferToDisplay( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSSWAPBUFFERTODISPLAY psSwapBufferToDisplay );
CSRESULT FX_CALL csDeviceSpecificCommunication( PCSGRAPHICALCONTEXT psGraphicalContext,
                                                FxU32 u32MessageID );



#ifdef __cplusplus
}
#endif

#endif //__CSAPI_H__
