/*
** Copyright (c) 1999 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
*/

#include <string.h>
#include <3dfx.h>
#include <glidesys.h>

#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <glide.h>
#include "g3ext.h"

#include "fxglide.h"
#include "fxcmd.h"

#include "mac_sfc.h"
#include "h3defs.h"
#include "h3gdefs.h"
#include "minihwc.h"
#include "hwcio.h"
#include <DCon.h>
#include <hrm_lists.h>
#include <hrm_mem.h>
#include <hrm_fifo.h>
#include <hrm_mode.h>

extern hrmSetExclusiveModePtr _hrmSetExclusiveMode;
extern hrmSetVideoModePtr _hrmSetVideoMode;
extern hrmReleaseExclusiveModePtr _hrmReleaseExclusiveMode;    
extern hrmAllocateBlockPtr _hrmAllocateBlock;
extern hrmFreeBlockPtr _hrmFreeBlock;
extern hrmAllocWinContextPtr _hrmAllocWinContext;
extern hrmFreeWinContextPtr _hrmFreeWinContext;
extern hrmExecuteWinFifoPtr _hrmExecuteWinFifo;
extern hrmGetDeviceConfigPtr _hrmGetDeviceConfig;

#if 0
typedef struct Node_s Node_t;

/* My old favorite Amiga style doubly-linked list stuff */

struct Node_s
{
	Node_t *succ;
	Node_t *prev;
};

typedef struct List_s List_t;

/* I know someone is going to look at these next two definitions (List_s & NewList)
 * and scratch their head, so here's the deal:
 *
 * The list header saves space by embedding & overlapping the head and tail sentinel
 * nodes.  Normally we'd have something like this:
 *
 *     		Head		Node		Node		Tail
 * succ		----------->----------->-----------> 0
 * prev      0  <-----------<-----------<-----------
 *
 * The List_s structure just overlaps the 0's from the head & tail nodes.
 *
 * That's it!  The only thing complicated from other systems is that nodes with 
 * 0 succ or 0 prev are not valid nodes.
 */
 
struct List_s
{
	Node_t *head;
	Node_t *tail;
	Node_t *tailPred;
};
#endif

void NewList(List_t *list)
{
	list->head = (Node_t *)&list->tail;
	list->tail = 0;
	list->tailPred = (Node_t *)list;
}

void AddHead(List_t *list, Node_t *node)
{
	Node_t *head = list->head;
	
	node->succ = head;
	node->prev = head->prev;
	head->prev = node;
	list->head = node;
}

void AddTail(List_t *list, Node_t *node)
{
	Node_t *tail = list->tailPred;
	
	node->succ = tail->succ;
	node->prev = tail;
	list->tailPred = node;
	tail->succ = node;
}

Node_t *Remove(Node_t *node)
{
	node->succ->prev = node->prev;
	node->prev->succ = node->succ;
	return node;
}

Node_t *RemHead(List_t *list)
{
	if(list->head->succ)
		return Remove(list->head);
	return 0;
}

Node_t *RemTail(List_t *list)
{
	if(list->tailPred->prev)
		return Remove(list->head);
	return 0;
}


/* Okay, enough of that. */

struct GDXSurface_s
{
	Node_t				node;
	GrSurfaceDesc_t	    desc;
	/* Whatever else I need here... */	
	FxU32				memBase;
	FxU32				memSize;
	mmBlock_t          *memBlock;
};



OSErr SetUpPixMap(
    short        depth,       /* Desired number of bits/pixel in off-screen*/
    Rect         *bounds,     /* Bounding rectangle of off-screen */
    CTabHandle   colors,      /* Color table to assign to off-screen */
    short        bytesPerRow, /* Number of bytes per row in the PixMap */
    PixMapHandle aPixMap,	  /* Handle to the PixMap being initialized */
    Ptr			 offBaseAddr); /* Pointer to the off-screen pixel image */    
void DisposeOffScreen(
    CGrafPtr doomedPort,    /* Pointer to the CGrafPort to be disposed of */
    GDHandle doomedGDevice); /* Handle to the GDevice to be disposed of */
OSErr CreateGDevice(
    PixMapHandle basePixMap,  /* Handle to the PixMap to base GDevice on */
    GDHandle     *retGDevice); /* Returns a handle to the new GDevice */
OSErr CreateOffScreen(
    Rect       *bounds,     /* Bounding rectangle of off-screen */
    short      depth,       /* Desired number of bits per pixel in off-screen*/
    CTabHandle colors,      /* Color table to assign to off-screen */
    Ptr			pixMapBaseAddr,	/* Memory to use for PixMap */
    short		bytesPerRow,
    CGrafPtr   *retPort,    /* Returns a pointer to the new CGrafPort */
    GDHandle   *retGDevice); /* Returns a handle to the new GDevice */
static OSErr  InstallAcceleration();
static OSErr  UninstallAcceleration();

/* Our list of allocated surfaces, so we can clean up any that
   are left over when Glide exits. */

static List_t surfaceList = { 0, 0, 0 };

/* This doesn't do much for now */
void gdxSurfaceInit(void)
{	
	NewList(&surfaceList);
}

static void _gdxSurfaceNotify(struct mmBlock_s *block, FxU32 code)
{
  GDXSurface_t *sfc = block->userData;
  
  if(sfc->desc.notifyCallback) {
    switch(code) {
      case HRM_MEM_NOTIFY_DISPOSE:
        sfc->desc.notifyCallback(sfc, sfc->desc.userData, GR_SURFACE_NOTIFY_LOST);
        break;
    }
  }

  /* Now do local processing. */
  switch(code) {
    case HRM_MEM_NOTIFY_DISPOSE:
      Remove(&sfc->node);
      DisposePtr((Ptr)sfc);
      break;
  }
}

/* Allocate a drawing/fifo/etc. surface of a given size */
void *gdxSurfaceAlloc(hwcBoardInfo *bInfo, hrmBoard_t *theBoard, GrSurfaceDesc_t *desc)
{
	GDXSurface_t *sfc;
	FxU32 hwWidth;
	OSErr err;
	Rect boundsRect;

	/* Make sure HRM extensions we need are around */
	if(!_hrmAllocateBlock)
	  return 0;
	
	sfc = (GDXSurface_t *)NewPtr(sizeof(*sfc));
		  
	if(!sfc)
		return 0;
		
	/* Copy over requested info */
	sfc->desc = *desc;
	
	/* Calc minumum surface size requirements, in case this is a drawing surface */
	hwWidth = (desc->width + 15) & ~15;
	sfc->desc.pitch = hwWidth * sfc->desc.bytesPerPixel;		
	sfc->memSize = sfc->desc.height * sfc->desc.pitch;
	
	sfc->memBlock = _hrmAllocateBlock(theBoard, sfc->memSize, 0, 0);
	if(!sfc->memBlock) {
	  DisposePtr((Ptr)sfc);
	  return 0;
	}
	
	/* Set up HRM notification proc */
	sfc->memBlock->notifyProc = _gdxSurfaceNotify;
	sfc->memBlock->userData = sfc;
	
	sfc->memBase = sfc->memBlock->start;	
	sfc->memSize = sfc->memBlock->size;
	
	/* Now do magic byte swizzling stuff for 16-bit and 32-bit surfaces. */
#if 1	
	if(sfc->desc.bytesPerPixel == 2) {
	  sfc->memBase += bInfo->pciInfo.swizzleOffset[3];
	} else if(sfc->desc.bytesPerPixel == 4) {
	  sfc->memBase += bInfo->pciInfo.swizzleOffset[1];
	}
#endif	
	sfc->desc.surface = sfc->memBase;		
	/* sfc->desc.owner = theBoard; */
	
	boundsRect.left = 0;
	boundsRect.right = desc->width;
	boundsRect.top = 0;
	boundsRect.bottom = desc->height;
	
	/* Don't create MacOS rendering info for 8-bit surfaces, since
	   they are really only used for FIFO stuff. */
	if(sfc->desc.bytesPerPixel >= 2) {
		/* Create an offscreen rendering device for this surface */
		err = CreateOffScreen(&boundsRect,sfc->desc.bytesPerPixel << 3,0,
							  (Ptr)sfc->memBase,
							  sfc->desc.pitch,
							  &sfc->desc.systemPortId,
							  &sfc->desc.systemDeviceId);
    }			
    
	/* Make sure list is initialized */
	if(!surfaceList.head)
		gdxSurfaceInit();
								
	/* Add to our list of surfaces */
	AddHead(&surfaceList,&sfc->node);
	
	return sfc;
}

void gdxSurfaceFree(void *sfc_ptr)
{
	GDXSurface_t *sfc = sfc_ptr;
	
	/* Remove from list of surfaces */
	Remove(&sfc->node);
		
	/* Free video memory */
	_hrmFreeBlock(sfc->memBlock);
	
	if(sfc->desc.systemPortId && sfc->desc.systemDeviceId) {
      /* Free offscreen crap */
	  DisposeOffScreen(sfc->desc.systemPortId,sfc->desc.systemDeviceId);
	}
	
	/* Free the surface struct */
	DisposePtr((char *)sfc);
}

void gdxSurfaceGetDesc(void *sfc, GrSurfaceDesc_t *desc)
{
	*desc = ((GDXSurface_t *)sfc)->desc;
}

/* Clean up any dangling surfaces */
void gdxSurfaceShutdown(void)
{
	GDXSurface_t *sfc;
    
	/*
	SJL 10-15-00
	Quick fix for LW crashing bug.
	bug was due to the fact that our GL has a shared data section.
	When linked to Glide it causes glide to have a shared data section.
	thus the surfaceList is shared between apps.
	When one app quits it disposes of the surfaces from the other open apps.
	thus causing a crash.
	for now just disable this clean up routine by returning.
	*/
	return;	

    /* This is just in case we got called before init... */
	if(!surfaceList.head)
	  return;
	  
	while(sfc = (GDXSurface_t *)RemHead(&surfaceList))
	{
		gdxSurfaceFree(sfc);
	}
	/* UninstallAcceleration(); */
}

/* Lame MacOS crud */

#define kMaxRowBytes 0x3FFE /* Maximum number of bytes in a row of pixels */

OSErr CreateOffScreen(
    Rect       *bounds,     /* Bounding rectangle of off-screen */
    short      depth,       /* Desired number of bits per pixel in off-screen*/
    CTabHandle colors,      /* Color table to assign to off-screen */
    Ptr			pixMapBaseAddr,	/* Memory to use for PixMap */
    short		bytesPerRow,
    CGrafPtr   *retPort,    /* Returns a pointer to the new CGrafPort */
    GDHandle   *retGDevice) /* Returns a handle to the new GDevice */
{
    CGrafPtr     newPort;     /* Pointer to the new off-screen CGrafPort */
    PixMapHandle newPixMap;   /* Handle to the new off-screen PixMap */
    GDHandle     newDevice;   /* Handle to the new off-screen GDevice */
    long         qdVersion;   /* Version of QuickDraw currently in use */
    GrafPtr      savedPort;   /* Pointer to GrafPort used for save/restore */
    SignedByte   savedState;  /* Saved state of color table handle */
    OSErr        error;       /* Returns error code */

    /* Initialize a few things before we begin */
    newPort = nil;
    newPixMap = nil;
    newDevice = nil;
    error = noErr;

    /* Save the color table's current state and make sure it isn't purgeable*/
    if (colors != nil)
    {
        savedState = HGetState( (Handle)colors );
        HNoPurge( (Handle)colors );
    }

    /* Get the current QuickDraw version */
    (void)Gestalt( gestaltQuickdrawVersion, &qdVersion );

    /* Make sure depth is indexed or depth is direct and 32-Bit QD installed*/
    if (depth == 1 || depth == 2 || depth == 4 || depth == 8 ||
            ((depth == 16 || depth == 32) && qdVersion >=gestalt32BitQD))
    {
        /* Maximum number of bytes per row is 16,382; make sure within range*/
        if (bytesPerRow <= kMaxRowBytes)
        {
            /* Make sure a color table is provided if the depth is indexed */
            if (depth <= 8)
            	if (colors == nil)
                  /* Indexed depth and clut is NIL; is parameter error */
                  error = paramErr;
        }
        else
            /* # of bytes per row is more than 16,382; is parameter error */
            error = paramErr;
    }
    else
        /* Pixel depth isn't valid; is parameter error */
        error = paramErr;

    /* If sanity checks succeed, then allocate a new CGrafPort */
    if (error == noErr)
    {
        newPort = (CGrafPtr)NewPtr( sizeof (CGrafPort) );
        if (newPort != nil)
        {
            /* Save the current port */
            GetPort( &savedPort );

            /* Initialize the new CGrafPort and make it the current port */
            OpenCPort( newPort );

            /* Set portRect, visRgn, and clipRgn to the given bounds rect */
            newPort->portRect = *bounds;
            RectRgn( newPort->visRgn, bounds );
            ClipRect( bounds );

            /* Initialize the new PixMap for off-screen drawing */
            error = SetUpPixMap( depth, bounds, colors, bytesPerRow,
                    newPort->portPixMap, pixMapBaseAddr );
            if (error == noErr)
            {
                /* Grab the initialized PixMap handle */
                newPixMap = newPort->portPixMap;

                /* Allocate and initialize a new GDevice */
                error = CreateGDevice( newPixMap, &newDevice );
            }

            /* Restore the saved port */
            SetPort( savedPort );
        }
        else
            error = MemError();
    }

    /* Restore the given state of the color table */
    if (colors != nil)
        HSetState( (Handle)colors, savedState );

    /* One Last Look Around The House Before We Go... */
    if (error != noErr)
    {
        /* Some error occurred; dispose of everything we allocated */
        if (newPixMap != nil)
        {
            DisposeCTable( (**newPixMap).pmTable );
            /* DisposPtr( (**newPixMap).baseAddr ); */
        }
        if (newDevice != nil)
        {
            DisposeHandle( (Handle)(**newDevice).gdITable );
            DisposeHandle( (Handle)newDevice );
        }
        if (newPort != nil)
        {
            CloseCPort( newPort );
            DisposePtr( (Ptr)newPort );
        }
    }
    else
    {
        /* Everything's OK; return refs to off-screen CGrafPort and GDevice*/
        *retPort = newPort;
        *retGDevice = newDevice;
    }
    return error;
}

#define kDefaultRes 0x00480000 /* Default resolution is 72 DPI; Fixed type */

OSErr SetUpPixMap(
    short        depth,       /* Desired number of bits/pixel in off-screen*/
    Rect         *bounds,     /* Bounding rectangle of off-screen */
    CTabHandle   colors,      /* Color table to assign to off-screen */
    short        bytesPerRow, /* Number of bytes per row in the PixMap */
    PixMapHandle aPixMap,	  /* Handle to the PixMap being initialized */
    Ptr			 offBaseAddr) /* Pointer to the off-screen pixel image */    
{
    CTabHandle newColors;   /* Color table used for the off-screen PixMap */
    OSErr      error;       /* Returns error code */

    error = noErr;
    newColors = nil;

    /* Clone the clut if indexed color; allocate a dummy clut if direct color*/
    newColors = (CTabHandle)NewHandle( sizeof (ColorTable) -
            sizeof (CSpecArray) );
    error = MemError();
    
    if (error == noErr)
    {
        /* Initialize fields common to indexed and direct PixMaps */
        (**aPixMap).baseAddr = offBaseAddr;  /* Point to image */
        (**aPixMap).rowBytes = bytesPerRow | /* MSB set for PixMap */
                0x8000;
        (**aPixMap).bounds = *bounds;        /* Use given bounds */
        (**aPixMap).pmVersion = 0;           /* No special stuff */
        (**aPixMap).packType = 0;            /* Default PICT pack */
        (**aPixMap).packSize = 0;            /* Always zero in mem */
        (**aPixMap).hRes = kDefaultRes;      /* 72 DPI default res */
        (**aPixMap).vRes = kDefaultRes;      /* 72 DPI default res */
        (**aPixMap).pixelSize = depth;       /* Set # bits/pixel */
        (**aPixMap).planeBytes = 0;          /* Not used */
        (**aPixMap).pmReserved = 0;          /* Not used */

        /* PixMap is direct */
        (**aPixMap).pixelType = RGBDirect; /* Indicates direct */
        (**aPixMap).cmpCount = 3;          /* Have 3 components */
        if (depth == 16)
            (**aPixMap).cmpSize = 5;       /* 5 bits/component */
        else
            (**aPixMap).cmpSize = 8;       /* 8 bits/component */
        (**newColors).ctSeed = 3 * (**aPixMap).cmpSize;
        (**newColors).ctFlags = 0;
        (**newColors).ctSize = 0;
        (**aPixMap).pmTable = newColors;
    }
    else
        newColors = nil;

    /* If no errors occurred, return a handle to the new off-screen PixMap */
    if (error != noErr)
    {
        if (newColors != nil)
            DisposeCTable( newColors );
    }

    /* Return the error code */
    return error;
}

#define kITabRes 4 /* Inverse-table resolution */

OSErr CreateGDevice(
    PixMapHandle basePixMap,  /* Handle to the PixMap to base GDevice on */
    GDHandle     *retGDevice) /* Returns a handle to the new GDevice */
{
    GDHandle   newDevice;  /* Handle to the new GDevice */
    ITabHandle embryoITab; /* Handle to the embryonic inverse table */
    Rect       deviceRect; /* Rectangle of GDevice */
    OSErr      error;      /* Error code */

    /* Initialize a few things before we begin */
    error = noErr;
    newDevice = nil;
    embryoITab = nil;

    /* Allocate memory for the new GDevice */
    newDevice = (GDHandle)NewHandle( sizeof (GDevice) );
    if (newDevice != nil)
    {
        /* Allocate the embryonic inverse table */
        embryoITab = (ITabHandle)NewHandleClear( 2 );
        if (embryoITab != nil)
        {
            /* Set rectangle of device to PixMap bounds */
            deviceRect = (**basePixMap).bounds;

            /* Initialize the new GDevice fields */
            (**newDevice).gdRefNum = 0;            /* Only used for screens*/
            (**newDevice).gdID = 0;                /* Won't normally use */
            if ((**basePixMap).pixelSize <= 8)
                (**newDevice).gdType = clutType;   /* Depth<=8; clut device*/
            else
                (**newDevice).gdType = directType; /* Depth>8; direct device*/
            (**newDevice).gdITable = embryoITab;   /* 2-byte handle for now*/
            (**newDevice).gdResPref = kITabRes;    /* Normal inv table res */
            (**newDevice).gdSearchProc = nil;      /* No color-search proc */
            (**newDevice).gdCompProc = nil;        /* No complement proc */
            (**newDevice).gdFlags = 0;             /* Will set these later */
            (**newDevice).gdPMap = basePixMap;     /* Reference our PixMap */
            (**newDevice).gdRefCon = 0;            /* Won't normally use */
            (**newDevice).gdNextGD = nil;          /* Not in GDevice list */
            (**newDevice).gdRect = deviceRect;     /* Use PixMap dimensions*/
            (**newDevice).gdMode = -1;             /* For nonscreens */
            (**newDevice).gdCCBytes = 0;           /* Only used for screens*/
            (**newDevice).gdCCDepth = 0;           /* Only used for screens*/
            (**newDevice).gdCCXData = 0;           /* Only used for screens*/
            (**newDevice).gdCCXMask = 0;           /* Only used for screens*/
            (**newDevice).gdReserved = 0;          /* Currently unused */

            /* Set color-device bit if PixMap isn't black & white */
            if ((**basePixMap).pixelSize > 1)
                SetDeviceAttribute( newDevice, gdDevType, true );

            /* Set bit to indicate that the GDevice has no video driver */
            SetDeviceAttribute( newDevice, noDriver, true );

            /* Initialize the inverse table */
            if ((**basePixMap).pixelSize <= 8)
            {
                MakeITable( (**basePixMap).pmTable, (**newDevice).gdITable,
                        (**newDevice).gdResPref );
                error = QDError();
            }
        }
        else
            error = MemError();
    }
    else
        error = MemError();

    /* Handle any errors along the way */
    if (error != noErr)
    {
        if (embryoITab != nil)
            DisposeHandle( (Handle)embryoITab );
        if (newDevice != nil)
            DisposeHandle( (Handle)newDevice );
    }
    else
        *retGDevice = newDevice;

    /* Return a handle to the new GDevice */
    return error;
}

void DisposeOffScreen(
    CGrafPtr doomedPort,    /* Pointer to the CGrafPort to be disposed of */
    GDHandle doomedGDevice) /* Handle to the GDevice to be disposed of */
{
    CGrafPtr currPort;    /* Pointer to the current port */
    GDHandle currGDevice; /* Handle to the current GDevice */

    /* Check to see whether the doomed CGrafPort is the current port */
    GetPort( (GrafPtr *)&currPort );
    if (currPort == doomedPort)
    {
        /* It is; set current port to Window Manager CGrafPort */
        GetCWMgrPort( &currPort );
        SetPort( (GrafPtr)currPort );
    }

    /* Check to see whether the doomed GDevice is the current GDevice */
    currGDevice = GetGDevice();
    if (currGDevice == doomedGDevice)
        /* It is; set current GDevice to the main screen's GDevice */
        SetGDevice( GetMainDevice() );

    /* Throw everything away */
    (**doomedGDevice).gdPMap = nil;
    DisposeGDevice( doomedGDevice );
    /* Don't do this since it's not owned by the system */
    /* DisposPtr( (**doomedPort->portPixMap).baseAddr ); */
    if ((**doomedPort->portPixMap).pmTable != nil)
        DisposeCTable( (**doomedPort->portPixMap).pmTable );
    CloseCPort( doomedPort );
    DisposePtr( (Ptr)doomedPort );
}

/* More nasty MacOS Hacks */

/*
	File:		AcceleratedBitBlit.c

	Contains:	Routines to accelerate bit blits using
				simulated graphics acceleration hardware.

	Written by:	Erik Staats

	Copyright:	© 1994 by Apple Computer, Inc., all rights reserved.

*/

#if 0

#include <NQDAcceleration.h>

/* internal procedure prototypes */

hwcBoardInfo *acceleratorBoardInfo;

static void  AcceleratedBitBlit (NQDDrawVars  *drawVars);
static void  AcceleratedScaleBlit (NQDDrawVars  *drawVars);

Int32  GetAcceleratedBitBlitProc(NQDDrawVars  *drawVars)
{
#if 0		
  if ((drawVars->mode < 8) &&
      (drawVars->srcPixMap.pixelSize >= 8))
  {
    drawVars->blitProc = AcceleratedBitBlit;
    return (true);
  }
#endif
  return (false);
}

Int32  GetAcceleratedScaleBlitProc(NQDDrawVars  *drawVars)
{
	hwcBoardInfo *bInfo = acceleratorBoardInfo;
	
	/* Must be simple copy mode */
	if(drawVars->mode == 0 && drawVars->trimResult == 0) 
	{
		/* Check src & dst locations */
		FxU32 srcAddr = (FxU32)drawVars->srcPixMap.baseAddr;
		FxU32 dstAddr = (FxU32)drawVars->dstPixMap.baseAddr;
		
		if(srcAddr >= bInfo->regInfo.rawLfbBase &&
	       srcAddr < (bInfo->regInfo.rawLfbBase + 16777216) &&
	       dstAddr >= bInfo->regInfo.rawLfbBase &&
	       dstAddr < (bInfo->regInfo.rawLfbBase + 16777216))
	    {
	    	/* Make sure it's a 16 -> 32 bit blit */
	    	if(drawVars->srcPixMap.pixelSize == 16 &&
	    	   drawVars->dstPixMap.pixelSize == 32)
	    	{
	    		drawVars->blitProc = AcceleratedScaleBlit;
	    		return true;
	    	}
		}
	}
  return (false);
}


static void  AcceleratedBitBlit(NQDDrawVars  *drawVars)
{
  unsigned short  height, width;
  unsigned short  srcRowBytes, dstRowBytes;

  height = drawVars->dstRect.bottom - drawVars->dstRect.top;
  width = drawVars->dstRect.right - drawVars->dstRect.left;
  //GASetHeight (height);
  //GASetWidth (width);

  if (drawVars->srcPixMap.rowBytes > 0)
  {
    srcRowBytes = drawVars->srcPixMap.rowBytes;
    dstRowBytes = drawVars->dstPixMap.rowBytes;
    //GASetSrcRowBytes (srcRowBytes);
    //GASetDstRowBytes (dstRowBytes);
  }
  else
  {
    srcRowBytes = -(drawVars->srcPixMap.rowBytes);
    dstRowBytes = -(drawVars->dstPixMap.rowBytes);
    //GASetSrcRowBytes (srcRowBytes);
    //GASetDstRowBytes (dstRowBytes);
  }

  if ((drawVars->srcPixMap.rowBytes > 0) && (drawVars->hBump > 0))
  { /* process in forward direction */
#if 0  
    GASetSrcPtr ((unsigned long) drawVars->srcPixMap.baseAddr +
                 (drawVars->srcRect.left - drawVars->srcPixMap.bounds.left)*
                   (drawVars->srcPixMap.pixelSize >> 3) +
                 (drawVars->srcRect.top - drawVars->srcPixMap.bounds.top)*
                   (srcRowBytes));
    GASetDstPtr ((unsigned long) drawVars->dstPixMap.baseAddr +
                 (drawVars->dstRect.left - drawVars->dstPixMap.bounds.left)*
                   (drawVars->dstPixMap.pixelSize >> 3) +
                 (drawVars->dstRect.top - drawVars->dstPixMap.bounds.top)*
                   (dstRowBytes));
    GASetForwardProcessingDirection ();
#endif    
  }
  else
  {
#if 0  
    GASetSrcPtr ((unsigned long) drawVars->srcPixMap.baseAddr +
                 (drawVars->srcRect.left - drawVars->srcPixMap.bounds.left)*
                   (drawVars->srcPixMap.pixelSize >> 3) +
                 (drawVars->srcRect.top - drawVars->srcPixMap.bounds.top)*
                   (srcRowBytes) +
                 (width - 1)*(drawVars->srcPixMap.pixelSize >> 3) +
                 (height - 1)*(srcRowBytes));
    GASetDstPtr ((unsigned long) drawVars->dstPixMap.baseAddr +
                 (drawVars->dstRect.left - drawVars->dstPixMap.bounds.left)*
                   (drawVars->dstPixMap.pixelSize >> 3) +
                 (drawVars->dstRect.top - drawVars->dstPixMap.bounds.top)*
                   (dstRowBytes) +
                 (width - 1)*(drawVars->dstPixMap.pixelSize >> 3) +
                 (height - 1)*(dstRowBytes));
    GASetBackwardProcessingDirection ();
#endif    
  }

  //GASetPixelDepth (drawVars->dstPixMap.pixelSize);
  //GAClearControlReg ();
  //GASetTransferMode (drawVars->mode);

/* set up colorizing if necessary */
  if (drawVars->colorizeFlag)
  {
    //GADoColorize ();
    //GASetForegroundColor (drawVars->foreColor);
    //GASetBackgroundColor (drawVars->backColor);
  }
  else
  {
    //GADontColorize ();
  }

/* do acceleration */
  //GAGo ();
}

static void  AcceleratedScaleBlit(NQDDrawVars  *drawVars)
{
  unsigned long  height, width;
  unsigned long  srcRowBytes, dstRowBytes;
  hwcBoardInfo *bInfo = acceleratorBoardInfo;

  CGrafPtr cport = (CGrafPtr)drawVars->port;

  height = drawVars->dstRect.bottom - drawVars->dstRect.top;
  width =  drawVars->dstRect.right - drawVars->dstRect.left;

  HWC_WAX_STORE(bInfo->regInfo, dstSize, width | (height << 16));
  
  if (drawVars->srcPixMap.rowBytes > 0)
  {
    srcRowBytes = drawVars->srcPixMap.rowBytes;
    dstRowBytes = drawVars->dstPixMap.rowBytes;
  }
  else
  {
    srcRowBytes = -(drawVars->srcPixMap.rowBytes);
    dstRowBytes = -(drawVars->dstPixMap.rowBytes);
  }
  
  HWC_WAX_STORE(bInfo->regInfo,srcFormat, srcRowBytes | SSTG_PIXFMT_16BPP);
  HWC_WAX_STORE(bInfo->regInfo,dstFormat, dstRowBytes | SSTG_PIXFMT_32BPP);
  
  HWC_WAX_STORE(bInfo->regInfo, srcBaseAddr, (FxU32) drawVars->srcRow - bInfo->regInfo.rawLfbBase);
  HWC_WAX_STORE(bInfo->regInfo, dstBaseAddr, (FxU32) drawVars->dstRow - bInfo->regInfo.rawLfbBase);  
  
  HWC_WAX_STORE(bInfo->regInfo, srcXY, 0);
  HWC_WAX_STORE(bInfo->regInfo, dstXY, 0);
  HWC_WAX_STORE(bInfo->regInfo, clip0min, 0);  
  HWC_WAX_STORE(bInfo->regInfo, clip0max, width | (height << 16));
  HWC_WAX_STORE(bInfo->regInfo, commandEx, 0);
  HWC_WAX_STORE(bInfo->regInfo, commandEx, SSTG_WAIT_FOR_VSYNC_EX);
  HWC_WAX_STORE(bInfo->regInfo, command, SSTG_BLT | SSTG_GO | (SSTG_ROP_SRC << SSTG_ROP0_SHIFT));
  
  /* wait for command to complete */
  {
  	FxU32 status;
  	
  	while(FXTRUE)
  	{
  		status = HWC_WAX_LOAD(bInfo->regInfo, status, status);
  		if(!(status & SST_GUI_BUSY))
  			break;
  	}
  }  	
}

/* accelerating procedures */
Int32  GetAcceleratedBitBlitProc (NQDDrawVars  *drawVars);
Int32  GetAcceleratedPatBlitProc (NQDDrawVars  *drawVars);

/* some internal prototypes */
OSErr  InitializeGraphicsAcceleration ();
OSErr  GraphicsAcceleration (OSType  selector, Ptr  params);
OSErr  TerminateGraphicsAcceleration ();
static OSErr  InstallAcceleration ();
static OSErr  UninstallAcceleration ();

/* some globals */
Boolean  accelerationInstalled = false;

static OSErr  InstallAcceleration()
{
  NQDGetBlitProcParamBlock  paramBlock;

  if (!accelerationInstalled)
  { /* install acceleration hooks */
    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedBitBlitProc;
    paramBlock.finishProc = nil;
    paramBlock.index = kBitBlitIndex;
    NQDMisc (kAddBlitProcPtr, (Int32 *) &paramBlock);

    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedScaleBlitProc;
    paramBlock.finishProc = nil;
    paramBlock.index = kScaleBlitIndex;
    NQDMisc (kAddBlitProcPtr, (Int32 *) &paramBlock);

    accelerationInstalled = true;
  }

  return (noErr);
}

static OSErr  UninstallAcceleration()
{
  NQDGetBlitProcParamBlock  paramBlock;

  if (accelerationInstalled)
  { /* remove acceleration hooks */
    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedBitBlitProc;
    paramBlock.finishProc = nil;
    paramBlock.index = kBitBlitIndex;
    NQDMisc (kRemoveBlitProcPtr, (Int32 *) &paramBlock);

    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedScaleBlitProc;
    paramBlock.finishProc = nil;
    paramBlock.index = kScaleBlitIndex;
    NQDMisc (kRemoveBlitProcPtr, (Int32 *) &paramBlock);

    accelerationInstalled = false;
  }

  return (noErr);
}
#endif
