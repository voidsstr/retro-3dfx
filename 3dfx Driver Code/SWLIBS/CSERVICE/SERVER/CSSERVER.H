/* $Header: csserver.h, 10, 10/11/00 8:08:42 PM, Brent$ */
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
** File name:   csproto.h
**
** Description: Structure definitions for the CS server protocol
**
** $Log: 
**  10   Napalm R1.51.4.1.4     10/11/00 Brent           Forced check in to enforce
**       branch.
**  9    Napalm R1.51.4.1.3     10/09/00 Don Fowler      Added debug structures to
**       server
**  8    Napalm R1.51.4.1.2     10/09/00 Don Fowler      Added support for
**       returning debug strings to the client
**  7    Napalm R1.51.4.1.1     07/21/00 Ryan Bissell    Addition of sentinel
**       buffer parameters to csExecuteCommands
**  6    Napalm R1.51.4.1.0     07/13/00 Ryan Bissell    Addition of sentinel
**       buffer parameters to csExecuteCommands
**  5    3dfx      1.4         06/15/00 Ryan Bissell    Incremental updates.
**  4    3dfx      1.3         03/11/00 Don Fowler      Added device specific id's
**       to allow a client to map and unmap memory bases.
**  3    3dfx      1.2         03/10/00 Don Fowler      Removed csREgisterREad and
**       csREgisterWrite to and added csDeviceSpecificCommunication to supercede
**       these functions .
**  2    3dfx      1.1         03/09/00 Don Fowler      Added the fields
**       u32StateSize and u32CommandSize to the CSALLOCREQ and CSALLOCRES
**       structures.
**  1    3dfx      1.0         03/09/00 Don Fowler      
** $
**
*/

#ifndef CSSERVER_H
#define CSSERVER_H

#include "cstypes.h"

#if (defined(WIN32) || (defined(WIN95) && defined(IS_16)))

/*
**
**
**  ExtEscape Number for Win9x platforms:
*/
#define CS_EXTESCAPE    0xFDC5


/*
**
**
** Definitions for CSGETPROTOCOLREVISION
*/
typedef struct csgetprotocolrevisionreq_s 
{
  FxU32 u32Major;                      /* major revision */
  FxU32 u32Minor;                      /* minor revision */

} CSGETPROTOCOLREVISIONREQ,FxFAR*PCSGETPROTOCOLREVISIONREQ;


typedef struct csgetprotocolrevisionres_s 
{
  FxU32 u32Major;                      /* major revision */
  FxU32 u32Minor;                      /* minor revision */

} CSGETPROTOCOLREVISIONRES,FxFAR*PCSGETPROTOCOLREVISIONRES;


/*
**
**
** Definitions for CSGETGRAPHICALCONTEXT
*/
typedef struct csgetgraphicalcontextreq_s 
{
    CSGRAPHICALCONTEXT sGraphicalContext;

} CSGETGRAPHICALCONTEXTREQ,FxFAR*PCSGETGRAPHICALCONTEXTREQ;

typedef struct csgetgraphicalcontextres_s 
{
    CSGRAPHICALCONTEXT sGraphicalContext;

} CSGETGRAPHICALCONTEXTRES,FxFAR*PCSGETGRAPHICALCONTEXTRES;

/*
**
**
** Definitions for CSRELEASEGRAPHICALCONTEXT
*/
typedef struct csreleasegraphicalcontextreq_s 
{
    CSGRAPHICALCONTEXT sGraphicalContext;

} CSRELEASEGRAPHICALCONTEXTREQ,FxFAR*PCSRELEASEGRAPHICALCONTEXTREQ;

typedef struct csreleasegraphicalcontextres_s 
{
    CSGRAPHICALCONTEXT sGraphicalContext;

} CSRELEASEGRAPHICALCONTEXTRES,FxFAR*PCSRELEASEGRAPHICALCONTEXTRES;

/*
**
**
**  Memory Allocation
*/
typedef struct csallocreq_s 
{
  CSALLOCATIONDESCRIPTOR sAllocationDescriptor; /* allocation descriptor structure */
  
} CSALLOCREQ,FxFAR*PCSALLOCREQ;


typedef struct csallocres_s 
{
  CSALLOCATIONDESCRIPTOR sAllocationDescriptor;
  
} CSALLOCRES,FxFAR*PCSALLOCRES;



/*
**
**
**  Free
*/
typedef struct csfreereq_s 
{
  CSALLOCATIONDESCRIPTOR sAllocationDescriptor; /* allocation descriptor structure */

} CSFREEREQ,FxFAR*PCSFREEREQ;


typedef struct csfreeres_s 
{
  CSALLOCATIONDESCRIPTOR sAllocationDescriptor; /* allocation descriptor structure */

} CSFREERES,FxFAR*PCSFREERES;

/*
**
**
** Set Video Mode
*/
typedef struct cssetvideomodereq_s 
{
  CSVIDEOMODE sVideoMode;
   
} CSSETVIDEOMODEREQ,FxFAR*PCSSETVIDEOMODEREQ;

typedef struct cssetvideomoderes_s 
{
  CSVIDEOMODE sVideoMode;

} CSSETVIDEOMODERES,FxFAR*PCSSETVIDEOMODERES;

/*
**
**
** Acquire Overlay
*/
typedef struct csacquireoverlayreq_s
{
  CSOVERLAY sOverlay;

} CSACQUIREOVERLAYREQ,FxFAR*PCSACQUIREOVERLAYREQ;

typedef struct csacquireoverlayres_s
{
  CSOVERLAY sOverlay;

} CSACQUIREOVERLAYRES,FxFAR*PCSACQUIREOVERLAYRES;

/*
**
**
** csSwapBufferToDisplay
*/
typedef struct csswapbuffertodisplayreq_s 
{
    CSSWAPBUFFERTODISPLAY sSwapBufferToDisplay;
    CSALLOCATIONDESCRIPTOR sSrcAllocationDescriptor;

} CSSWAPBUFFERTODISPLAYREQ,FxFAR*PCSSWAPBUFFERTODISPLAYREQ;

typedef struct csswapbuffertodisplayres_s 
{
    CSSWAPBUFFERTODISPLAY sSwapBufferToDisplay;
    CSALLOCATIONDESCRIPTOR sSrcAllocationDescriptor;

} CSSWAPBUFFERTODISPLAYRES,FxFAR*PCSSWAPBUFFERTODISPLAYRES;

/*
**
**
** Execute Command Buffer
*/
typedef struct csexecutecommandsreq_s
{
  CSALLOCATIONDESCRIPTOR sStateAllocationDescriptor;
  CSALLOCATIONDESCRIPTOR sCommandAllocationDescriptor;
  CSALLOCATIONDESCRIPTOR sSentinelAllocationDescriptor;
  FxU32                  u32StateOffset;
  FxU32                  u32CommandOffset;
  FxU32                  u32StateSize;
  FxU32                  u32CommandSize;
  FxU32                  u32SentinelSerial;

} CSEXECUTECOMMANDSREQ,FxFAR*PCSEXECUTECOMMANDSREQ;  

typedef struct csexecutecommandsres_s
{
  CSALLOCATIONDESCRIPTOR sStateAllocationDescriptor;
  CSALLOCATIONDESCRIPTOR sCommandAllocationDescriptor;
  CSALLOCATIONDESCRIPTOR sSentinelAllocationDescriptor;
  FxU32                  u32StateOffset;
  FxU32                  u32CommandOffset;
  FxU32                  u32StateSize;
  FxU32                  u32CommandSize;
  FxU32                  u32SentinelSerial;

} CSEXECUTECOMMANDSRES,FxFAR*PCSEXECUTECOMMANDSRES;   

/*
**
**
**  Lock buffer for LFB access
*/
typedef struct cslockreq_s 
{
  CSALLOCATIONDESCRIPTOR sAllocationDescriptor;

} CSLOCKREQ,FxFAR*PCSLOCKREQ;

typedef struct cslockres_s 
{
  CSALLOCATIONDESCRIPTOR sAllocationDescriptor;

} CSLOCKRES,FxFAR*PCSLOCKRES;

/*
**
**
**  Unlock buffer after finished w/ LFB
*/
typedef struct csunlockreq_s
{
  CSALLOCATIONDESCRIPTOR sAllocationDescriptor;

} CSUNLOCKREQ,FxFAR*PCSUNLOCKREQ;


typedef struct csunlockres_s
{
  CSALLOCATIONDESCRIPTOR sAllocationDescriptor;

} CSUNLOCKRES,FxFAR*PCSUNLOCKRES;


// Flag to indicate this is a server error code 
#define CSDEBUG_ERROR_TYPE_SERVER 0x80000000

/*
**
**
**  Unlock buffer after finished w/ LFB
*/
typedef struct csgetservererrorstringreq_s
{
  CSRESULT idError;

} CSGETSERVERERRORSTRINGREQ,FxFAR*PCSGETSERVERERRORSTRINGREQ;


#define SERVER_ERROR_STRING_SIZE   0x100

typedef struct csgetservererrorstringres_s
{
  FxU8 szError[ SERVER_ERROR_STRING_SIZE ];

} CSGETSERVERERRORSTRINGRES,FxFAR*PCSGETSERVERERRORSTRINGRES;

/*
**
**
** Central Services Request
*/
typedef struct csserverreq_s 
{
    FxU32       u32Flags;        /* Undefined */
    FxU32       u32ReqID;        /* ID of request */
    CSGRAPHICALCONTEXT sGraphicalContext;  /* Graphical context of client */

    union
    {
        CSGETPROTOCOLREVISIONREQ        sGetProtocolRevisionReq;
        CSGETGRAPHICALCONTEXTREQ        sGetGraphicalContextReq;
        CSRELEASEGRAPHICALCONTEXTREQ    sReleaseGraphicalContextReq;
        CSALLOCREQ                      sAllocReq;
        CSFREEREQ                       sFreeReq;
        CSSETVIDEOMODEREQ               sSetVideoModeReq;
        CSACQUIREOVERLAYREQ             sAcquireOverlayReq;
        CSSWAPBUFFERTODISPLAYREQ        sSwapBufferToDisplayReq;
        CSEXECUTECOMMANDSREQ            sExecuteCommandsReq;
        CSLOCKREQ                       sLockReq;
        CSUNLOCKREQ                     sUnLockReq;
        CSGETSERVERERRORSTRINGREQ       sGetServerErrorStringReq;      

    } unionReqData;

} CSSERVERREQ,FxFAR*PCSSERVERREQ;


/*
**
**
** Central Services Result
*/
typedef struct csserverres_s
{
    FxU32    u32Flags;                      /* Undefined */
    FxU32    u32ReqID;                      /* ID of the request for bookkeeping */
    FxU32    u32Status;                     /* Status of request */
    CSGRAPHICALCONTEXT sGraphicalContext;   /* graphical context of client */

    union
    {
        CSGETPROTOCOLREVISIONRES        sGetProtocolRevisionRes;
        CSGETGRAPHICALCONTEXTRES        sGetGraphicalContextRes;
        CSRELEASEGRAPHICALCONTEXTRES    sReleaseGraphicalContextRes;
        CSALLOCRES                      sAllocRes;
        CSFREERES                       sFreeRes;
        CSACQUIREOVERLAYRES             sAcquireOverlayRes;
        CSSWAPBUFFERTODISPLAYRES        sSwapBufferToDisplayRes;
        CSEXECUTECOMMANDSRES            sExecuteCommandsRes;
        CSLOCKRES                       sLockRes;
        CSUNLOCKRES                     sUnLockRes;
        CSGETSERVERERRORSTRINGRES       sGetServerErrorStringRes;    

    } unionResData;

} CSSERVERRES,FxFAR*PCSSERVERRES;

/* These values need to be enumerated, but were defined because of possible cross-compiler problems */
#define CSREQ_NOTAREQUEST                       0  /*used to report error codes that are not specific to just one request ID */
#define CSREQ_GETPROTOCOLREVISION               1
#define CSREQ_GETGRAPHICALCONTEXT               2
#define CSREQ_RELEASEGRAPHICALCONTEXT           3
#define CSREQ_ALLOC                             4
#define CSREQ_FREE                              5
#define CSREQ_SETVIDEOMODE                      6
#define CSREQ_ACQUIREOVERLAY                    7
#define CSREQ_SWAPBUFFERTODISPLAY               8
#define CSREQ_EXECUTECOMMANDS                   9
#define CSREQ_LOCK                              10
#define CSREQ_UNLOCK                            11
#define CSREQ_GETSERVERERRORSTRING              12


#endif /* (defined(WIN32) || (defined(WIN95) && defined(IS_16))) */

#endif                          /* CSSERVER_H not defined */
