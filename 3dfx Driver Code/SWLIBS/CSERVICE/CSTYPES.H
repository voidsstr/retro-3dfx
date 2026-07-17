/* $Header: cstypes.h, 27, 10/24/00 9:27:51 AM, Don Fowler$ */
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
** File name:   cstypes.h
**
** Description: Types for the entire CS system
**
** $Log: 
**  27   Napalm R1.51.14.1.11   10/24/00 Don Fowler      Added CSTIME structure to
**       the CSGRAPHICALCONTEXT structure. This structure will be used to store a
**       time stamp for the creation of the process.
**  26   Napalm R1.51.14.1.10   10/12/00 Don Fowler      Added buffer type
**       CS_BUFFER_TEXTUREHEAP which is a memory type that is not a child of a
**       graphical context
**  25   Napalm R1.51.14.1.9    10/12/00 Don Fowler      Added type CSDCID for OS
**       independent device specifiers
**  24   Napalm R1.51.14.1.8    10/11/00 Brent           Forced check in to enforce
**       branch.
**  23   Napalm R1.51.14.1.7    10/11/00 Ryan Bissell    Modifications needed for
**       OGL.
**  22   Napalm R1.51.14.1.6    10/09/00 Don Fowler      Fixed up a couple of typos
**       in the CSSST1 structure elements
**  21   Napalm R1.51.14.1.5    09/28/00 Don Fowler      Added CSSST1 device
**       specific data structure so that CS can be tested in the W2k banshee driver
**       and possibly be used with the W2K Napalm driver.
**  20   Napalm R1.51.14.1.4    09/27/00 Don Fowler      Removed ddraw.h in cases
**       where the file is being included in the W2K driver build.
** 
**       Added an array of reserved DWORDS to be used by the server for its own
**       purpose
**  19   Napalm R1.51.14.1.3    08/28/00 Ryan Bissell    Added u32Staggering
**  18   Napalm R1.51.14.1.2    08/24/00 Ryan Bissell    Added a "u32NumSamples"
**       field to CSALLOCATIONDESCRIPTOR.
**  17   Napalm R1.51.14.1.1    07/20/00 Andrew  Bell    Rolling back all swlibs
**       files to 2nd latest version due to an incorrect checkin.  This should
**       bring things back to normal.
**  16   Napalm R1.51.14.1.0    07/19/00 Dinesh Raja Savari Amirtharaj CSIM for the
**       RTL release_3_2
**  15   3dfx      1.14        06/26/00 Ryan Bissell    Incremental updates
** 
**  14   3dfx      1.13        06/15/00 Ryan Bissell    Incremental updates.
**  13   3dfx      1.12        03/29/00 Don Fowler      Removed CS_MEMORY_AGP since
**       we now have CS_LOCALE_AGP
**       Added CSALLOC_APIERROR_INVALIDASPECT
**  12   3dfx      1.11        03/29/00 Don Fowler      Test APP: Made render
**       buffers load 640x480x24bpp images test1,2.bmp::Moved csInit and csUnInit
**       into the testdll::Added u32Locale to the csAlloc function calls
** 
**       Test DLL: Added csInit and csUnInit to the DETACH and ATTACH portions fo
**       the DLL::Removed csReleaseResourcesForProcess because the DLL is already
**       process-relative. 
** 
**       Client Library : Moved csReleaseResourcesForProcess to static. 
**  11   3dfx      1.10        03/23/00 Ryan Bissell    Renamed u32SurfacePitch to
**       u32LinearStride
**  10   3dfx      1.9         03/19/00 Don Fowler      Added a server base error
**       code to set a marker for other server error codes
**  9    3dfx      1.8         03/19/00 Don Fowler      Added the
**       CSALLOCATIONDESCRIPTOR.u32Locale so that the client can specify
**       CS_LOCALE_FRAMEBUFFER  or CS_LOCALE_AGP separate from tiled or linear
**       memory types.
** 
**       Changed all of the error codes in the API from _ERROR_ to _APIERROR_ so
**       that server codes can have a different base 
** 
**  8    3dfx      1.7         03/16/00 Don Fowler      Removed u32DestColorFormat
**       from CSSWAPBUFFERTODISPLAY
**  7    3dfx      1.6         03/16/00 Don Fowler      Added u32PhysicalStride
**       field to CSALLOCATIONDESCRIPTOR
**  6    3dfx      1.5         03/11/00 Don Fowler      Added device specific id's
**       to allow a client to map and unmap memory bases.
**  5    3dfx      1.4         03/10/00 Don Fowler      Removed read and write
**       regsiter types to be superceded by a more device-specific read and
**       write...
**  4    3dfx      1.3         03/09/00 Don Fowler      Added CSRESULT 
**  3    3dfx      1.2         03/09/00 Don Fowler      Changed the chip type
**       defines from enumerations to defines in case some compilers have problems
**       with that. Added CSEXECUTEBUFFER_ERROR_SERVERBUSY to be returned if the
**       server is ever busy in another thread.
**  2    3dfx      1.1         03/09/00 Don Fowler      Added error types 
**  1    3dfx      1.0         03/09/00 Don Fowler      
** $
**
*/

#ifndef CSTYPES_H
#define CSTYPES_H

#include "3dfx.h"

#if (defined(WIN32) || (defined(WIN95) && defined(IS_16))) && !defined(CSNT)
#include <ddraw.h>
#else 
typedef unsigned long DWORD;
typedef void * HINSTANCE;
typedef unsigned int UINT;
#endif

typedef FxU32 CSRESULT; 

/* If this code is compiling 16 bit then far needs to be defined for structure alignment. This
   will actually become more intelligent on future architectures */
#if (defined(WIN95) && defined(IS_16))
#define FxFAR FAR
#else 
#define FxFAR
#endif 

/* This needs to be in 3dfx.h but it's only used in this library for now */
typedef void FxVOID,FxFAR*PFxVOID;
typedef char FxSz,FxFAR*PFxSz; 
typedef void FxNULL;


/* These types will change on different OSes due to 32/64 bit issues,
   etc.  FxU32 is simply a placeholder until I get around to the
   ifdefs --CHD

   These are Win32 specific at the moment and will need to be changed 
   for other OS's --DWF
*/
#if (defined(WIN32) || (defined(WIN95) && defined(IS_16)))
typedef DWORD CSWINDOWID,FxFAR*PCSWINDOWID;
typedef DWORD CSPROCESSID,FxFAR*PCSPROCESSID;
typedef DWORD CSTHREADID,FxFAR*PCSTHREADID;
typedef DWORD CSHDC,FxFAR*PCSHDC;
typedef DWORD CSDCID,FxFAR*PCSDCID;
#else
/* Other operating systems */
#endif /* (defined(WIN32) || (defined(WIN95) && defined(IS_16))) */

/* Structure for initializing the system */
typedef struct csinit_s
{
#if (defined(WIN32) || (defined(WIN95) && defined(IS_16)))
    HINSTANCE hDLLInstance;     /* Instance of the DLL that is initializing the system */ 
#endif /* WIN32 */

} CSINIT,FxFAR*PCSINIT;

/* Node for a singly-linked list */
typedef struct cssllistnode_s
{
    struct cssllistnode_s FxFAR* psNext;

} CSSLLISTNODE,FxFAR*PCSSLLISTNODE;

/* Rectangle structure for CS, The elements are in pixel-coordinates */
typedef struct csrect_s
{
    FxU32 u32Left;     
    FxU32 u32Top;     
    FxU32 u32Right;     
    FxU32 u32Bottom; 

} CSRECT,FxFAR*PCSRECT;

/* This structure will be used to store the clip list to be passed to the server */
#define CS_CLIPREGIONS_MAX  0x200
typedef struct cscliplist_s
{
   FxU32 u32Flags;                              /* Undefined */   
   FxU32 u32TotalRegions;                       /* Total number of regions */
   CSRECT  sClipRegion[ CS_CLIPREGIONS_MAX ];   /* Static array to store the regions */          

} CSCLIPLIST,FxFAR*PCSCLIPLIST;

/* 
/* This section is for hardware specific information 
/***********************************************************************************
*/

/* Device specific ID's for communicating with the server */
#define CSSST2DEVICESPECIFIC_BASE    0x100
#define CSSST2DEVICESPECIFIC_REGISTERREAD           ( CSSST2DEVICESPECIFIC_BASE + 0x00 )            
#define CSSST2DEVICESPECIFIC_REGISTERWRITE          ( CSSST2DEVICESPECIFIC_BASE + 0x01 )            
#define CSSST2DEVICESPECIFIC_MAPMEMORYBASES         ( CSSST2DEVICESPECIFIC_BASE + 0x02 )            
#define CSSST2DEVICESPECIFIC_UNMAPMEMORYBASES       ( CSSST2DEVICESPECIFIC_BASE + 0x03 ) 

/* Error codes from CSSST2DEVICESPECIFIC_REGISTERWRITE  */
#define CSSST2DEVICESPECIFIC_REGISTERWRITE_APIERROR_BASE  0x900
#define CSSST2DEVICESPECIFIC_REGISTERWRITE_APIERROR_BADREGISTER  ( CSSST2DEVICESPECIFIC_REGISTERWRITE_APIERROR_BASE + 0x00 )
#define CSSST2DEVICESPECIFIC_REGISTERWRITE_APIERROR_BADTYPE      ( CSSST2DEVICESPECIFIC_REGISTERWRITE_APIERROR_BASE + 0x01 )
#define CSSST2DEVICESPECIFIC_REGISTERWRITE_APIERROR_BADIOSIZE    ( CSSST2DEVICESPECIFIC_REGISTERWRITE_APIERROR_BASE + 0x02 )

/* Error codes from CSSST2DEVICESPECIFIC_REGISTERREAD */
#define CSSST2DEVICESPECIFIC_REGISTERREAD_APIERROR_BASE  0x700
#define CSSST2DEVICESPECIFIC_REGISTERREAD_APIERROR_BADREGISTER ( CSSST2DEVICESPECIFIC_REGISTERREAD_APIERROR_BASE + 0x00 )
#define CSSST2DEVICESPECIFIC_REGISTERREAD_APIERROR_BADTYPE     ( CSSST2DEVICESPECIFIC_REGISTERREAD_APIERROR_BASE + 0x01 )
#define CSSST2DEVICESPECIFIC_REGISTERREAD_APIERROR_BADIOSIZE   ( CSSST2DEVICESPECIFIC_REGISTERREAD_APIERROR_BASE + 0x02 )

/* This structure will be a list of unions( one for each chip type ) that will be used
   to communicate information from the server to the client and vise-versa about an 
   allocation
*/
/* Chip specific data for the sst2 chipset */
#define CSSST2_REGSPACE_BASE  0x01
#define CSSST2_REGSPACE_IO    ( CSSST2_REGSPACE_BASE + 0x00 )
#define CSSST2_REGSPACE_2D    ( CSSST2_REGSPACE_BASE + 0x01 )
#define CSSST2_REGSPACE_3D    ( CSSST2_REGSPACE_BASE + 0x02 )
#define CSSST2_REGSPACE_CMD   ( CSSST2_REGSPACE_BASE + 0x03 )
#define CSSST2_REGSPACE_PCI   ( CSSST2_REGSPACE_BASE + 0x04 )
#define CSSST2_RESERVED_ENTRIES 0x20

typedef struct cssst2_s
{
    FxU32   u32Flags;           /* Flags for this chip type */
    FxU32   u32SLIAvailable;    /* TRUE if SLI is available */
    FxU32   u32NumSLIChips;     /* Number of SLI chips available */
    FxU32   u32UseSLI;          /* Set this to TRUE to use SLI on this chip( SetVideoMode ) */
    FxU32   u32DeviceRev;       /* The revision of the device.  This used by the client 
                                   to determine if there are bugs present in the hardware 
                                   which need to be worked around. */
    FxU32   u32LFBRam;          /* The amount of RAM available on the device.  
                                   This value is per-framebuffer, not per-board. */
    FxU32   u32AGPRam;          /* The amount of AGP memory available on the device */
    FxU32   u32Antialiased;     /* Set this parameter to TRUE to enable anti-aliasing( SetVideoMode ) */
    FxU32   u32CAMEntry;        /* CAM specific data */
    FxU32   u32TileMode;        /* Tile mode being used */
    FxU32   u32RegisterSpace;   /* Register space for a read/write request */ 
                                /* CSSST2_REGSPACE_PCI	
                                   CSSST2_REGSPACE_IO	
                                   CSSST2_REGSPACE_2D	
                                   CSSST2_REGSPACE_3D	
                                   CSSST2_REGSPACE_CMD */	
    FxU32   u32RegisterOffset;  /* Offset of a register within a register space */
    FxU32   u32IOSize;          /* IO size of a read/write request */
    FxU32   u32RegisterValue;   /* Value to write to/returned from a register on a write/read request */
    PFxVOID pvIOBase;           /* IO base memory space for SST2 */
    PFxVOID pvMemBase0;         /* Memory Base 0 space for SST2 */
    PFxVOID pvMemBase1;         /* Memory Base 1 space for SST2 */
    FxU32   u32Reserved[ CSSST2_RESERVED_ENTRIES ];

} CSSST2,FxFAR*PCSSST2;

/* Device specific ID's for communicating with the server */
#define CSSST1DEVICESPECIFIC_BASE    0x100
#define CSSST1DEVICESPECIFIC_REGISTERREAD           ( CSSST1DEVICESPECIFIC_BASE + 0x00 )            
#define CSSST1DEVICESPECIFIC_REGISTERWRITE          ( CSSST1DEVICESPECIFIC_BASE + 0x01 )            
#define CSSST1DEVICESPECIFIC_MAPMEMORYBASES         ( CSSST1DEVICESPECIFIC_BASE + 0x02 )            
#define CSSST1DEVICESPECIFIC_UNMAPMEMORYBASES       ( CSSST1DEVICESPECIFIC_BASE + 0x03 ) 

/* Error codes from CSSST1DEVICESPECIFIC_REGISTERWRITE  */
#define CSSST1DEVICESPECIFIC_REGISTERWRITE_APIERROR_BASE  0x900
#define CSSST1DEVICESPECIFIC_REGISTERWRITE_APIERROR_BADREGISTER  ( CSSST1DEVICESPECIFIC_REGISTERWRITE_APIERROR_BASE + 0x00 )
#define CSSST1DEVICESPECIFIC_REGISTERWRITE_APIERROR_BADTYPE      ( CSSST1DEVICESPECIFIC_REGISTERWRITE_APIERROR_BASE + 0x01 )
#define CSSST1DEVICESPECIFIC_REGISTERWRITE_APIERROR_BADIOSIZE    ( CSSST1DEVICESPECIFIC_REGISTERWRITE_APIERROR_BASE + 0x02 )

/* Error codes from CSSST1DEVICESPECIFIC_REGISTERREAD */
#define CSSST1DEVICESPECIFIC_REGISTERREAD_APIERROR_BASE  0x700
#define CSSST1DEVICESPECIFIC_REGISTERREAD_APIERROR_BADREGISTER ( CSSST1DEVICESPECIFIC_REGISTERREAD_APIERROR_BASE + 0x00 )
#define CSSST1DEVICESPECIFIC_REGISTERREAD_APIERROR_BADTYPE     ( CSSST1DEVICESPECIFIC_REGISTERREAD_APIERROR_BASE + 0x01 )
#define CSSST1DEVICESPECIFIC_REGISTERREAD_APIERROR_BADIOSIZE   ( CSSST1DEVICESPECIFIC_REGISTERREAD_APIERROR_BASE + 0x02 )

/* Chip specific data for the sst2 chipset */
#define CSSST1_REGSPACE_BASE  0x01
#define CSSST1_REGSPACE_IO    ( CSSST1_REGSPACE_BASE + 0x00 )
#define CSSST1_REGSPACE_2D    ( CSSST1_REGSPACE_BASE + 0x01 )
#define CSSST1_REGSPACE_3D    ( CSSST1_REGSPACE_BASE + 0x02 )
#define CSSST1_REGSPACE_CMD   ( CSSST1_REGSPACE_BASE + 0x03 )
#define CSSST1_REGSPACE_PCI   ( CSSST1_REGSPACE_BASE + 0x04 )
#define CSSST1_RESERVED_ENTRIES 0x20

typedef struct cssst1_s
{
    FxU32   u32Flags;           /* Flags for this chip type */
    FxU32   u32SLIAvailable;    /* TRUE if SLI is available */
    FxU32   u32NumSLIChips;     /* Number of SLI chips available */
    FxU32   u32UseSLI;          /* Set this to TRUE to use SLI on this chip( SetVideoMode ) */
    FxU32   u32DeviceRev;       /* The revision of the device.  This used by the client 
                                   to determine if there are bugs present in the hardware 
                                   which need to be worked around. */
    FxU32   u32LFBRam;          /* The amount of RAM available on the device.  
                                   This value is per-framebuffer, not per-board. */
    FxU32   u32AGPRam;          /* The amount of AGP memory available on the device */
    FxU32   u32Antialiased;     /* Set this parameter to TRUE to enable anti-aliasing( SetVideoMode ) */
    FxU32   u32RegisterSpace;   /* Register space for a read/write request */ 
                                /* CSSST1_REGSPACE_PCI	
                                   CSSST1_REGSPACE_IO	
                                   CSSST1_REGSPACE_2D	
                                   CSSST1_REGSPACE_3D	
                                   CSSST1_REGSPACE_CMD */	
    FxU32   u32RegisterOffset;  /* Offset of a register within a register space */
    FxU32   u32IOSize;          /* IO size of a read/write request */
    FxU32   u32RegisterValue;   /* Value to write to/returned from a register on a write/read request */
    PFxVOID pvIOBase;           /* IO base memory space for SST2 */
    PFxVOID pvMemBase0;         /* Memory Base 0 space for SST2 */
    PFxVOID pvMemBase1;         /* Memory Base 1 space for SST2 */
    FxU32   u32Reserved[ CSSST1_RESERVED_ENTRIES ];

} CSSST1,FxFAR*PCSSST1;


/* Chip types for the chip specific data field */
#define CSCHIPSPECIFICDATA_CHIPTYPE_SST1  0x01  
#define CSCHIPSPECIFICDATA_CHIPTYPE_SST2  0x02  


typedef struct cschipspecificdata_s
{
    FxU32   u32Flags;           /* Undefined */
    FxU32   u32ChipType;        /* Chip type for this structure.
                                   CSCHIPSPECIFICDATA_CHIPTYPE_SST2
                                */
    union
    {

        CSSST1 sSST1;           /* sst1 chip specific data */
        CSSST2 sSST2;           /* sst2 chip specific data */
        
    } unionChipType;

} CSCHIPSPECIFICDATA,FxFAR*PCSCHIPSPECIFICDATA;

/*
/* End Hardare Specific Information 
/************************************************************************************
*/

/* Structure used to store a time stamp */
typedef struct cstime_s
{
    FxU32 u32Year;
    FxU32 u32Month;
    FxU32 u32DayOfWeek;
    FxU32 u32Day;
    FxU32 u32Hour;
    FxU32 u32Minute;
    FxU32 u32Second;
    FxU32 u32Milliseconds;

} CSTIME,*PCSTIME;

/* This structure will be used to store information about the current video mode, as
   well as to set the video mode
*/
typedef struct csvideomode_s
{
    FxU32  u32Flags;            /* flags for this structure */
    FxU32  u32Width;            /* The width of the requested video mode in pixels */
    FxU32  u32Height;           /* The height of the request video mode in pixels */
    FxU32  u32ColorFormat;      /* The color format for video 
                                   CS_COLORFORMAT_8BPP
                                   CS_COLORFORMAT_DVD
                                   CS_COLORFORMAT_RGB565
                                   CS_COLORFORMAT_RGB32
                                   CS_COLORFORMAT_UYVY
                                   CS_COLORFORMAT_YUYV
                                   CS_COLORFORMAT_RGB1555
                                   CS_COLORFORMAT_YUVA  */
    FxU32  u32RefreshRate;      /* The refresh rate of the requested mode */
    CSCHIPSPECIFICDATA sChipSpecificData;   /* chip specific data for this video mode */
    
} CSVIDEOMODE,FxFAR*PCSVIDEOMODE;


/* Structure to store the information about an overlay */
#define CSOVERLAY_FLAGS_INITIALIZED         0x00000001
typedef struct csoverlay_s
{
    FxU32 u32Flags;         /* General flags for the structure */
    FxU32 u32ColorFormat;   /* The format of the overlay surface. See csSetVideoMode */       
    FxU32 u32SrcWidth;      /* The width of the source data */
    FxU32 u32SrcHeight;     /* The height of the source data */
    FxU32 u32DestX;         /* The X coordinate of the top left corner of where the 
                               overlay surface will be rendered on the desktop 
                               (for fullscreen clients) or relative to the position of 
                               their primary rendering surface (for window'd clients.) */
    FxU32 u32DestY;         /* The Y coordinate of the top left corner of where the 
                               overlay surface will be rendered on the desktop 
                               (for fullscreen clients) or relative to the position of 
                               their primary rendering surface (for window'd clients.) */
    FxU32 u32DestWidth;     /* The width of the overlay surface */
    FxU32 u32DestHeight;    /* The height of the overlay surface */
    CSCHIPSPECIFICDATA sChipSpecificData;   /* chip specific data for this overlay */

} CSOVERLAY,FxFAR*PCSOVERLAY;

/* Structure to store the device configuration information */
typedef struct csdeviceconfig_s
{
    FxU32  u32Flags;            /* General flags for the structure */
    FxU32  u32DeviceID;         /* The PCI device ID for the device on which the Central  
                                   Services client will run.  The Vendor ID is assumed to 
                                   be 0x121A-the one assigned to 3dfx. */

} CSDEVICECONFIG,FxFAR*PCSDEVICECONFIG;

/* Graphical context structure. This structure will be used to store client-side information
   about the graphical context.
*/


#define CS_STATE_SLIPOSSIBLE      0x00000001
#define CS_STATE_SLINOTPOSSIBLE   0x00000002
#define CS_STATE_EXCLUSIVE        0x00000003
#define CS_STATE_NOTEXCLUSIVE     0x00000004
#define CS_STATE_OVERLAYAVAILABLE 0x00000005
#define CS_STATE_OVERLAYLOST      0x00000006
typedef void (*PCSCALLBACK)( struct csgraphicalcontext_s* psGraphicalContext, FxU32 u32Message, FxVOID* param );


#define CSGRAPHICALCONTEXT_FLAGS_INITIALIZED    0x00000001
typedef struct csgraphicalcontext_s
{
    CSSLLISTNODE sNextNodeGraphicalContext; /* Singly-linked list node, Must be first element */
    CSSLLISTNODE sRootNodeAllocationDescriptor; /* Singly-link list root node for allocation descriptors */
    FxU32       u32Flags;        /* CSGRAPHICALCONTEXT_FLAGS_INITIALIZED */
    FxU32       u32ContextID;    /* Context ID */   
    CSWINDOWID  idWindow;        /* Window ID */
    CSPROCESSID idProcess;       /* Current process ID */
    CSTIME      sProcessTimeStamp; /* Time stamp that csInit was called for this process..
                                      used jointly with the process ID to ensure uniqueness */
    CSTHREADID  idThread;        /* Current thread ID */
    CSVIDEOMODE sPreviousVideoMode; /* Information to restore video mode with */
    CSVIDEOMODE sCurrentVideoMode;  /* Information for the current video mode */
    CSOVERLAY   sOverlay;        /* Overlay structure if one is acquired */
    CSDEVICECONFIG sDeviceConfig; /* Device configuration structure */
    PCSCALLBACK    pStateCallback;

#if (defined(WIN32) || (defined(WIN95) && defined(IS_16))) && !defined(CSNT)
    CSHDC hDC;                /* WIN32 device context for ExtEscape calls */
    LPDIRECTDRAWCLIPPER psDirectDrawClipper; /* Pointer to the direct draw clipper 
                                                for this device context */
#else
    CSHDC hDC;                /* WIN32 device context for ExtEscape calls */
    void * psDirectDrawClipper; /* Pointer to the direct draw clipper 
                                 for this device context */

#endif /* WIN32 */

    CSCHIPSPECIFICDATA  sChipSpecificData; /* Data that is chip specific */

} CSGRAPHICALCONTEXT,FxFAR*PCSGRAPHICALCONTEXT;




/* Allocation desctriptor structure. This structure will be used to store information about
   an allocation. 
*/  
#define CSALLOCATIONDESCRIPTOR_FLAGS_INITIALIZED    0x00000001 
#define CSALLOCATIONDESCRIPTOR_FLAGS_LOCKED         0x00000002 
typedef struct csallocationdescriptor_s
{
    CSSLLISTNODE sNextNodeAllocationDescriptor; /* Singly-linked list node, Must be first element */
    FxU32   u32Flags;            /* flags for the structure */
    FxU32   u32BufferID;         /* used by the server to keep track of the allocation */       
    FxU32   u32BufferType;       /* CS_BUFFER_FIFO (a special case of PERSISTENT)
                                    CS_BUFFER_PERSISTENT
                                    CS_BUFFER_RENDER
                                    CS_BUFFER_TEXTURE
                                    CS_BUFFER_TEXTUREHEAP
                                    CS_BUFFER_ZBUFFER */
    FxU32   u32MemType;          /* CS_MEMORY_LINEAR
                                    CS_MEMORY_TILED */
    FxU32   u32Locale;           /* CS_LOCALE_AGP   
                                    CS_LOCALE_FRAMEBUFFER */
    FxU32   u32Size;             /* The size of the buffer in bytes.  
                                    The actual amount of memory reserved may be larger 
                                    (in order to account for memory aligment issues), 
                                    but this will be reflected only in the display driver. */
    FxU32   u32Width;            /* The width of the surface if the memory type requested 
                                    is tiled. */
    FxU32   u32Height;           /* The height of the surface if the memory type is tiled. */
    FxU32   u32Depth;            /* The depth of the surface if the memory type is tiled */
    FxU32   u32PhysicalOffset;   /* The physical offset of the allocated buffer. */
    PFxVOID pvLinearAddress;     /* Linear address of the memory, only valid after a csLock */
    FxU32   u32LockCount;        /* Number of times descriptor has been locked */
    FxU32   u32LockFlags;        /* Flags for the lock 
                                    CSLOCK_FLAG_WRITEABLE     	Lock for writing
                                    CSLOCK_FLAG_READABLE	Lock for reading
                                    CSLOCK_FLAG_3DLFB	Indicates the client wishes to 
                                    use the 3D LFB aperture. */
    FxU32   u32LinearStride;     /* Pitch of the surface as it appears when locked */
    FxU32   u32LFBDepth;         /* Apparent depth of surface as seen throught the LFB aperture */
    FxU32   u32PhysicalStride;   /* Physical stride of the surface, as it exists in memory */
    FxU32   u32ColorFormat;
    FxU32   u32NumSamples;       /* Indicates AA surface.  0 is an alias for 1 (both mean no AA) */
    FxU32   u32Staggering;       /* 0 means not staggered; non-zero means staggered. */
    CSCHIPSPECIFICDATA  sChipSpecificData; /* Data that is chip specific */

} CSALLOCATIONDESCRIPTOR,FxFAR*PCSALLOCATIONDESCRIPTOR;


/* Structure to store the information for a csSwapBufferToDisplay call */
typedef struct csswapbuffertodisplay_s
{
    FxU32 u32Flags;                             /* General flags for the structure */
    PCSALLOCATIONDESCRIPTOR psSrcBufferAllocationDescriptor; 
    CSRECT sDestClipRegion;     /* Master destination clip region. Optional parameter */
  //FxU32 u32DestX;             /* Destination X on the screen */
  //FxU32 u32DestY;             /* Destination Y on the screen */
  //CSRECT sSrcClipRegion;      /* Source clip region. Optional parameter */
  //FxU32 u32InterpolationType; /*  */
    CSWINDOWID idDestWindow;    /* Window ID of the destination window. */
    CSCLIPLIST sClipList;       /* Clip list to be sent to server */ 

} CSSWAPBUFFERTODISPLAY,FxFAR*PCSSWAPBUFFERTODISPLAY;


/* colorspace format constants */
#define CS_COLORFORMAT_BASE     0x01
#define CS_COLORFORMAT_8BPP     ( CS_COLORFORMAT_BASE + 0x00 ) /*  Surface is 8 bpp paletized data. */
#define CS_COLORFORMAT_DVD      ( CS_COLORFORMAT_BASE + 0x01 ) /*  Surface is DVD sub-picture, 8bpp */
#define CS_COLORFORMAT_RGB565   ( CS_COLORFORMAT_BASE + 0x02 ) /*  Surface is RGB565, 16bpp */
#define CS_COLORFORMAT_RGB32    ( CS_COLORFORMAT_BASE + 0x03 ) /*  Surface is ARGB, 32 bpp */
#define CS_COLORFORMAT_UYVY     ( CS_COLORFORMAT_BASE + 0x04 ) /*  Surface is YUV422 in UYVY,  16bpp */
#define CS_COLORFORMAT_YUYV     ( CS_COLORFORMAT_BASE + 0x05 ) /*  Surface is YUV422 in YUYV, 16bpp */
#define CS_COLORFORMAT_RGB1555  ( CS_COLORFORMAT_BASE + 0x06 ) /*  Surface is ARGB1555, 16bpp */
#define CS_COLORFORMAT_YUVA     ( CS_COLORFORMAT_BASE + 0x07 ) /*  Surface is YUVA, 32 bpp */


/* memory types */
#define CS_MEMORY_BASE      0x01
#define CS_MEMORY_LINEAR    ( CS_MEMORY_BASE + 0x00 )
#define CS_MEMORY_TILED     ( CS_MEMORY_BASE + 0x01 )

/* locale types */
#define CS_LOCALE_BASE      0x01
#define CS_LOCALE_FRAMEBUFFER   ( CS_LOCALE_BASE + 0x00 )
#define CS_LOCALE_AGP           ( CS_LOCALE_BASE + 0x01 )


/* buffer types */
#define CS_BUFFER_BASE          0x01
#define CS_BUFFER_FIFO          ( CS_BUFFER_BASE + 0x00 )
#define CS_BUFFER_PERSISTENT    ( CS_BUFFER_BASE + 0x01 )
#define CS_BUFFER_RENDER        ( CS_BUFFER_BASE + 0x02 )
#define CS_BUFFER_TEXTURE       ( CS_BUFFER_BASE + 0x03 )
#define CS_BUFFER_TEXTUREHEAP   ( CS_BUFFER_BASE + 0x04 )
#define CS_BUFFER_ZBUFFER       ( CS_BUFFER_BASE + 0x05 )


/* interpolation types */
#define CSBLIT_BASE         0x01
#define CSBLIT_INTERP_POINTSAMPLED   ( CSBLIT_BASE + 0x00 )
#define CSBLIT_INTERP_POINTBILINEAR  ( CSBLIT_BASE + 0x01 )



/* Protocol verioning */
#define CS_PROTOCOL_MAJOR   0
#define CS_PROTOCOL_MINOR   0

#define CSLOCK_FLAG_READABLE  0x00000001
#define CSLOCK_FLAG_WRITEABLE 0x00000002
#define CSLOCK_FLAG_3DLFB     0x00000004

/* Global error messages */
#define CS_SUCCESS                                  0
#define CS_APIERROR_BASE                               0x01
#define CS_APIERROR_INVALIDCONTEXT                     ( CS_APIERROR_BASE + 0x00 )
#define CS_APIERROR_INVALIDBUFFERID                    ( CS_APIERROR_BASE + 0x01 )
#define CS_APIERROR_OUTOFMEMORY                        ( CS_APIERROR_BASE + 0x02 )
#define CS_APIERROR_INVALIDPARAM                       ( CS_APIERROR_BASE + 0x03 )
#define CS_APIERROR_IMPOSSIBILITY                      ( CS_APIERROR_BASE + 0x04 )
#define CS_APIERROR_FULLSCREENACTIVE                   ( CS_APIERROR_BASE + 0x05 )
#define CS_APIERROR_SURFACELOST                        ( CS_APIERROR_BASE + 0x06 )
#define CS_APIERROR_NOTSUPPORTED                       ( CS_APIERROR_BASE + 0x07 )
#define CS_APIERROR_SYSTEMNOTINITIALIZED               ( CS_APIERROR_BASE + 0x08 )
#define CS_APIERROR_UNKNOWN                            ( CS_APIERROR_BASE + 0x09 )
#define CS_APIERROR_GRAPHICALCONTEXTNOTINITIALIZED     ( CS_APIERROR_BASE + 0x0c )
#define CS_APIERROR_GRAPHICALCONTEXTINITIALIZED        ( CS_APIERROR_BASE + 0x0d )
#define CS_APIERROR_INVALIDSTRUCTUREPARAM              ( CS_APIERROR_BASE + 0x0e )
#define CS_APIERROR_ALLOCATIONDESCRIPTORINITIALIZED    ( CS_APIERROR_BASE + 0x0f )
#define CS_APIERROR_ALLOCATIONDESCRIPTORNOTINITIALIZED ( CS_APIERROR_BASE + 0x10 )
#define CS_APIERROR_OVERLAYINITIALIZED                 ( CS_APIERROR_BASE + 0x11 )
#define CS_APIERROR_OVERLAYNOTINITIALIZED              ( CS_APIERROR_BASE + 0x12 )
#define CS_APIERROR_INVALIDCHIPTYPE                    ( CS_APIERROR_BASE + 0x13 )
#if (defined(WIN32) || (defined(WIN95) && defined(IS_16)))
#define CS_APIERROR_DIRECTDRAWFAILED                   ( CS_APIERROR_BASE + 0x20 )
#define CS_APIERROR_SYSTEMFAILEDEXTESCAPE              ( CS_APIERROR_BASE + 0x21 )
#define CS_APIERROR_SERVERFAILEDEXTESCAPE              ( CS_APIERROR_BASE + 0x22 )
#endif /* WIN32 */

/* Error codes from csAlloc */
#define CSALLOC_APIERROR_BASE  0x100
#define CSALLOC_APIERROR_INVALIDMEMORYTYPE         ( CSALLOC_APIERROR_BASE + 0x00 )
#define CSALLOC_APIERROR_INVALIDCOMB               ( CSALLOC_APIERROR_BASE + 0x01 )
#define CSALLOC_APIERROR_INVALIDSIZE               ( CSALLOC_APIERROR_BASE + 0x02 )
#define CSALLOC_APIERROR_INVALIDBUFFERTYPE         ( CSALLOC_APIERROR_BASE + 0x03 )
#define CSALLOC_APIERROR_INVALIDLOCALE             ( CSALLOC_APIERROR_BASE + 0x04 )
#define CSALLOC_APIERROR_INVALIDASPECT             ( CSALLOC_APIERROR_BASE + 0x05 )

/* Error codes from csSLIInit */
#define CSSLIINIT_APIERROR_BASE  0x200
#define CSSLIINIT_APIERROR_NOOVERLAY               ( CSSLIINIT_APIERROR_BASE + 0x00 )

/* Error codes from csSetVideoMode */
#define CSSETVIDEOMODE_APIERROR_BASE  0x300
#define CSSETVIDEOMODE_STATUS_BADREFRESH        ( CSSETVIDEOMODE_APIERROR_BASE + 0x00 )
#define CSSETVIDEOMODE_APIERROR_NOTFULLSCREEN      ( CSSETVIDEOMODE_APIERROR_BASE + 0x01 )
#define CSSETVIDEOMODE_APIERROR_UNSUPPORTEDRES     ( CSSETVIDEOMODE_APIERROR_BASE + 0x02 )
#define CSSETVIDEOMODE_APIERROR_AAUNSUPPORTED      ( CSSETVIDEOMODE_APIERROR_BASE + 0x03 )
#define CSSETVIDEOMODE_APIERROR_OVERLAYBUSY        ( CSSETVIDEOMODE_APIERROR_BASE + 0x04 )

/* Error codes from csExecuteBuffer */
#define CSEXECUTEBUF_APIERROR_BASE  0x400
#define CSEXECUTEBUF_APIERROR_SERVERBUSY           ( CSEXECUTEBUF_APIERROR_BASE + 0x00 )
#define CSEXECUTEBUF_APIERROR_STATELOST            ( CSEXECUTEBUF_APIERROR_BASE + 0x01 )

/* Error codes from csSwapBufferToDisplay */
#define CSSWAPBUFFERTODISPLAY_APIERROR_BASE  0x500
#define CSSWAPBUFFERTODISPLAY_APIERROR_UNSUPPORTEDCONV            ( CSSWAPBUFFERTODISPLAY_APIERROR_BASE + 0x00 )
#define CSSWAPBUFFERTODISPLAY_APIERROR_UNSUPPORTEDSCALE           ( CSSWAPBUFFERTODISPLAY_APIERROR_BASE + 0x01 )
#define CSSWAPBUFFERTODISPLAY_APIERROR_CANTGETCLIPLISTSIZE        ( CSSWAPBUFFERTODISPLAY_APIERROR_BASE + 0x02 )
#define CSSWAPBUFFERTODISPLAY_APIERROR_CANTGETCLIPLIST            ( CSSWAPBUFFERTODISPLAY_APIERROR_BASE + 0x03 )
#define CSSWAPBUFFERTODISPLAY_APIERROR_CANTASSOCIATEWINDOWTOCLIP  ( CSSWAPBUFFERTODISPLAY_APIERROR_BASE + 0x04 )

/* Error codes from csLock */
#define CSLOCK_APIERROR_BASE  0x600
#define CSLOCK_APIERROR_UNSUPPORTEDLOCK ( CSLOCK_APIERROR_BASE + 0x00 )
#define CSLOCK_APIERROR_NOLOCKAVAILABLE ( CSLOCK_APIERROR_BASE + 0x01 )


/* Error codes from csUnLock */
#define CSUNLOCK_APIERROR_BASE  0x800
#define CSUNLOCK_APIERROR_NOTLOCKED ( CSUNLOCK_APIERROR_BASE + 0x00 )

/* Error codes from csInit */
#define CSINIT_APIERROR_BASE  0xa00
#define CSINIT_APIERROR_ALREADYINITIALIZED   ( CSINIT_APIERROR_BASE + 0x00 )

/* Error codes from csUnInit */
#define CSUNINIT_APIERROR_BASE  0xb00

/* Error codes from csCleanupResourcesForProcess */
#define CSCLEANUPRESOURCESFORPROCESS_APIERROR_BASE  0xb00

/* Error codes from csFree  */
#define CSFREE_APIERROR_BASE  0xc00
#define CSFREE_APIERROR_STILLLOCKED            ( CSFREE_APIERROR_BASE + 0x00 )

/* Error codes from csAcquireOverlay */
#define CSACQUIREOVERLAY_APIERROR_BASE  0xd00
#define CSACQUIREOVERLAY_APIERROR_INITIALIZED  ( CSACQUIREOVERLAY_APIERROR_BASE + 0x00 )

/* Error codes from csGetGraphicalContext */
#define CSGETGRAPHICALCONTEXT_APIERROR_BASE  0xe00
#if (defined(WIN32) || (defined(WIN95) && defined(IS_16)))
#define CSGETGRAPHICALCONTEXT_APIERROR_CANTCREATECLIPPER           ( CSGETGRAPHICALCONTEXT_APIERROR_BASE + 0x00 )
#endif /* WIN32 */


/* Error codes from the server */
#define CS_SERVERERROR_BASE     0x8000

#endif /* CSTYPES_H */