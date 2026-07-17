/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
**
** $Revision: 9$
** $Date: 10/11/00 7:39:49 PM$
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fxhwc.h>
#include <init.h>
#include <fximg.h>

FX_EXPORT const char * FX_CSTYLE
hwcPixelFormatToString(HwcPixelFormat format)
{
    const char *fmt;

    switch (format) {
    case HWC_PIXFMT_1BPP:
        fmt = "1bpp"; 
        break;
    case HWC_PIXFMT_P_8:  
        fmt = "p8"; 
        break;
    case HWC_PIXFMT_P_8_6666:
        fmt = "p8-6666"; 
        break;
    case HWC_PIXFMT_A_8:
        fmt = "a8"; 
        break;
    case HWC_PIXFMT_I_8:
        fmt = "i8"; 
        break;
    case HWC_PIXFMT_AI_44:
        fmt = "ai44"; 
        break;
    case HWC_PIXFMT_RGB_332:
        fmt = "rgb332"; 
        break;
    case HWC_PIXFMT_YUYV: 
        fmt = "yuyv"; 
        break;
    case HWC_PIXFMT_UYVY: 
        fmt = "uyvy"; 
        break;
    case HWC_PIXFMT_YUV_411:
        fmt = "yuv411"; 
        break;
    case HWC_PIXFMT_YIQ_422:
        fmt = "yiq422"; 
        break;
    case HWC_PIXFMT_AP_88:
        fmt = "ap88"; 
        break;
    case HWC_PIXFMT_AI_88:
        fmt = "ai88"; 
        break;
    case HWC_PIXFMT_AYIQ_8422:
        fmt = "ayiq8422"; 
        break;
    case HWC_PIXFMT_RGB_565:  
        fmt = "rgb565"; 
        break;
    case HWC_PIXFMT_ARGB_1555:  
        fmt = "argb1555"; 
        break;
    case HWC_PIXFMT_ARGB_8332:
        fmt = "argb8332"; 
        break;
    case HWC_PIXFMT_ARGB_4444:
        fmt = "argb4444"; 
        break;
    case HWC_PIXFMT_TXCMP_4:
        fmt = "txcmp"; 
        break;
    case HWC_PIXFMT_RGB_888:  
        fmt = "rgb888"; 
        break;
    case HWC_PIXFMT_ARGB_8888:  
        fmt = "argb8888";  
        break;
    case HWC_PIXFMT_AA_16: 
        fmt = "aa-16"; 
        break;
    case HWC_PIXFMT_AA_32: 
        fmt = "aa-32"; 
        break;
    case HWC_PIXFMT_AP_44:
        fmt = "ap44"; 
        break;
    case HWC_PIXFMT_AYUV_32:
        fmt = "ayuv32"; 
        break;
    case HWC_PIXFMT_AA_1555: 
        fmt = "aa-1555"; 
        break;
    case HWC_PIXFMT_VIP_ANC:
        fmt = "vip-anc";
        break;
    default: 
        fmt = "unknown"; 
        break;
    }

    return fmt;
}

FX_EXPORT FxBool FX_CSTYLE
_hwcInitMemRegion(HwcMemRegion *pRegion, FxI32 baseAddress, FxI32 sizeInBytes)
{
    GDBG_INFO(4,"_hwcInitMemReg(0x%lx, 0x%lx, %d, %d)\n", pRegion, baseAddress, sizeInBytes);
    pRegion->next = NULL;
    pRegion->children = NULL;
    pRegion->baseAddress = baseAddress;
    pRegion->sizeInBytes = sizeInBytes;
    pRegion->allocMethod = HWC_ALLOCATE_LINEAR; 
    return FXTRUE;
}

/*----------------------------------------------------------------------
  allocate memory (local, agp, system), XXX add more randomized memory allocation
 ----------------------------------------------------------------------*/

FX_EXPORT FxBool FX_CSTYLE
_hwcAllocMemRegRandom( HwcMemRegion *pBlock, HwcMemRegion *pRegion, FxU32 sizeInBytes, FxU32 align)
{
    HwcMemRegion *pNext, *pPrev;
    FxI32 bit = BIT(align&HWC_ALIGN_BITS);
    FxU32 mask = SST_MASK(align&HWC_ALIGN_BITS);
    FxI32 baseAddress;
    FxU32 allocMethod = pRegion->allocMethod; 

    GDBG_INFO(4,"_hwcAllocMemReg(0x%lx, 0x%lx, %d, 0x%lx)\n", pBlock, pRegion, sizeInBytes, align);

    if ( allocMethod == HWC_ALLOCATE_RANDOM ) 
        allocMethod = hwcInt32Random(1) ? HWC_ALLOCATE_LINEAR : HWC_ALLOCATE_RLINEAR; 

    switch ( allocMethod ) {
    default:
        allocMethod = HWC_ALLOCATE_LINEAR; 
    case HWC_ALLOCATE_LINEAR:
        baseAddress = pRegion->baseAddress;
        break;
    case HWC_ALLOCATE_RLINEAR:
        baseAddress = pRegion->baseAddress+pRegion->sizeInBytes-sizeInBytes;
        bit = -bit;
        break;
    }

    while (1) {
        if ( align ) {
            if ( allocMethod == HWC_ALLOCATE_LINEAR ) {
                baseAddress = ( baseAddress + mask )&(~mask);
            } else {
                baseAddress = baseAddress & (~mask);
            }
        
            if (( align & HWC_ALIGN_ODD ) && !( baseAddress & bit )) 
                baseAddress += bit;
            
            if (( align & HWC_ALIGN_EVEN ) && ( baseAddress & bit )) 
                baseAddress += bit;
        }

        // check if this the proposed block is within the available memory region
        if (( baseAddress < (FxI32)pRegion->baseAddress ) ||
            (( baseAddress + sizeInBytes ) > ((FxI32)pRegion->baseAddress+pRegion->sizeInBytes))) {
            GDBG_ERROR("_hwcAllocMemReg", "size requested (%d) > total available (%d)\n", 
                        sizeInBytes, pRegion->sizeInBytes);
            return FXFALSE; // no place to put buffer
        }

        // check if any allocated region intersects proposed region
        for ( pNext = pRegion->children; pNext; pNext = pNext->next ) {
            if ((((FxI32)(pNext->baseAddress+pNext->sizeInBytes)) <= baseAddress ) || 
                (( baseAddress + (FxI32)sizeInBytes ) <= (FxI32)(pNext->baseAddress) ))
                continue;
            else break;
        }

        if ( pNext ) { // region occupied, try another
            if ( allocMethod == HWC_ALLOCATE_LINEAR ) {
                baseAddress = pNext->baseAddress+pNext->sizeInBytes;
            } else {
                baseAddress = pNext->baseAddress-sizeInBytes;
            }
        } else { // region available
            pPrev = NULL;
            for ( pNext = pRegion->children; pNext; pPrev = pNext, pNext = pNext->next ) {
                 if ( pNext->baseAddress >= ( baseAddress + sizeInBytes ))
                     break;
            }

            // initialize region entry

	        pBlock->baseAddress = baseAddress;
	        pBlock->sizeInBytes = sizeInBytes;
	        pBlock->children = NULL;

            // add to active list
        
            if ( pPrev ) {
                pBlock->next = pPrev->next;
                pPrev->next = pBlock;
            } else {
                pBlock->next = pRegion->children;
                pRegion->children = pBlock;
            }

            // perform internal consistency check

            pPrev = NULL;
            for ( pNext = pRegion->children; pNext; pPrev = pNext, pNext = pNext->next ) {
                if ((( pNext->next ) && (( pNext->baseAddress+pNext->sizeInBytes ) > pNext->next->baseAddress )) ||
                     (( pNext->baseAddress+pNext->sizeInBytes ) > ( pRegion->baseAddress+pRegion->sizeInBytes))) { 
                    GDBG_ERROR("_hwcAllocMemReg", "Internal consistency check failed\n");
                    return FXFALSE;
                }
            }

            return FXTRUE;
        }
    }
}

FX_EXPORT FxBool FX_CSTYLE
_hwcAllocMemReg( HwcMemRegion *pBlock, HwcMemRegion *pRegion, FxU32 sizeInBytes, FxU32 align, FxU32 hints)
{
#define FN_NAME "_hwcAllocMemReg"
#define FIRST_TRY 1
#define SECOND_TRY 1
    HwcMemRegion *pNext = NULL, *pPrev = NULL;
    FxU32 bit = BIT(align&HWC_ALIGN_BITS);
    FxU32 mask = SST_MASK(align&HWC_ALIGN_BITS);
    FxI32 baseAddress;
    FxU32 clientPlaced = ( hints & HWC_BUFFER_BASEADDR ) ? 1 : 0;

    GDBG_INFO(4,"%s(0x%lx, 0x%lx, %d, 0x%lx)\n", FN_NAME, pBlock, pRegion, sizeInBytes, align);

    if ( !clientPlaced && pRegion->allocMethod != HWC_ALLOCATE_LINEAR )
        return _hwcAllocMemRegRandom( pBlock, pRegion, sizeInBytes, align);

retry_alloc:
    if ( sizeInBytes > pRegion->sizeInBytes ) {
        GDBG_ERROR(FN_NAME, "size requested (%d) > total available (%d)\n", 
                   sizeInBytes, pRegion->sizeInBytes);
    }

    baseAddress = ( clientPlaced == FIRST_TRY ) ? pBlock->baseAddress : pRegion->baseAddress;

    if ( align ) {
        baseAddress = ( baseAddress + mask )&(~mask);

        if (( align & HWC_ALIGN_ODD ) && !( baseAddress & bit )) 
            baseAddress += bit;
    
        if (( align & HWC_ALIGN_EVEN ) && ( baseAddress & bit )) 
            baseAddress += bit;
    }

    if (( clientPlaced == FIRST_TRY ) && ( baseAddress != (FxI32)pBlock->baseAddress )) {
        GDBG_INFO(0, "%s pre assigned base address (0x%lx) not aligned should be (0x%lx)\n",
                  FN_NAME, pBlock->baseAddress, baseAddress);
    }

    // see if there is a space within the area covered by active list
    for ( pNext = pRegion->children; pNext; pPrev = pNext, pNext = pNext->next ) {
        if ( pNext->baseAddress > ( baseAddress + sizeInBytes ))
           goto foundSlot;
        
        if ( baseAddress <= (FxI32)( pNext->baseAddress + pNext->sizeInBytes )) {
            baseAddress = pNext->baseAddress+pNext->sizeInBytes;

            if ( align ) {
                baseAddress = ( baseAddress + mask )&(~mask);

                if (( align & HWC_ALIGN_ODD ) && !( baseAddress & bit )) 
                    baseAddress += bit;
    
                if (( align & HWC_ALIGN_EVEN ) && ( baseAddress & bit )) 
                    baseAddress += bit;
            }
        }
    }

    // see if there is space after the currently allocated regions
    if ( (pRegion->baseAddress+pRegion->sizeInBytes) < ( baseAddress + sizeInBytes )) {
        if ( clientPlaced == FIRST_TRY ) {
            GDBG_INFO(0, "%s pre assigned base address (0x%lx) does not fit retrying\n",
                      FN_NAME, pBlock->baseAddress );
            clientPlaced = SECOND_TRY;
            goto retry_alloc;
        } else {
            GDBG_ERROR(FN_NAME, "size requested (%d) > total available (%d)\n", 
                       sizeInBytes, pRegion->sizeInBytes);
            return FXFALSE; // no place to put buffer
        }
    }

foundSlot:

    // initialize region entry

    if ( clientPlaced && ( baseAddress != (FxI32)pBlock->baseAddress )) {
        GDBG_INFO(0, "%s pre assigned base address (0x%lx) moved to 0x%lx \n",
                  FN_NAME, pBlock->baseAddress, baseAddress );
    }
	pBlock->baseAddress = baseAddress;
	pBlock->sizeInBytes = sizeInBytes;
	pBlock->children = NULL;

    // add to active list

    if ( pPrev ) {
        pBlock->next = pPrev->next;
        pPrev->next = pBlock;
    } else {
        pBlock->next = pRegion->children;
        pRegion->children = pBlock;
    }

    GDBG_INFO(4,"%s allocated 0x%lx in 0x%lx\n", FN_NAME, pBlock->baseAddress, pRegion);

    return FXTRUE;
#undef FN_NAME
#undef FIRST_TRY
#undef SECOND_TRY
}


FX_EXPORT void FX_CSTYLE
_hwcFreeMemReg( HwcMemRegion *pBlock, HwcMemRegion *pRegion)
{
    HwcMemRegion *pNext = NULL, *pPrev = NULL;

    GDBG_INFO(4,"_hwcFreeMemReg(0x%lx, 0x%lx)\n", pBlock, pRegion);

    for ( pNext = pRegion->children; pNext; pPrev = pNext, pNext = pNext->next ) {
        if ( pNext == pBlock ) {
            if ( pPrev != NULL ) 
                pPrev->next = pNext->next;
            else pRegion->children = pNext->next;
            return;
        }
    }

    GDBG_ERROR("_hwcFreeMemReg", "region 0x%lx not found in 0x%lx\n", pBlock->baseAddress, pRegion);
}

#if !defined(KERNEL)
/*----------------------------------------------------------------------
   Save buffer to file
   XXX: this assumes 565 format
  ----------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcBufferSave(HwcSimulator *hws, HwcBuffer *pBuff, const char *filename, FxU32 type)
{
  char filenameEx[1000];
  FxU32 x,y,*buf;
  ImgInfo info;

  GDBG_INFO(4,"hwcBufferSave(0x%lx, %s, %d)\n", pBuff, filename, type);

  if ( hws == NULL )
      hws = pBuff->hwc->hws;

  if ( hws == NULL ) {
     GDBG_ERROR("hwcBufferSave", "NULL simulator\n");
     return FXFALSE;
  }

  if ( pBuff == NULL ) {
     GDBG_ERROR("hwcBufferSave", "NULL buffer\n");
     return FXFALSE;
  }

  info.any.width = pBuff->width;
  info.any.height = pBuff->height;
  info.any.sizeInBytes = pBuff->width * pBuff->height * 4;
  info.any.data = (ImgData *)hwcMemAlloc(info.any.sizeInBytes);
  strcpy(filenameEx,filename);
  /* IMG_UNKNOWN means the filename dictates the format */
  if (type == IMG_UNKNOWN) {
    int len = strlen(filenameEx);
    if (strcmp(&filenameEx[len-4],".ppm")==0)
      type = IMG_P6;
    else if (strcmp(&filenameEx[len-4],".sbi")==0)
      type = IMG_SBI;
    else if (strcmp(&filenameEx[len-4],".tga")==0)
      type = IMG_TGA32;
    if (type != IMG_UNKNOWN)
      filenameEx[len-4] = '\0';
  }

  strcat(filenameEx, hws->name);

  switch (type) {
  case IMG_P6:
    strcat(filenameEx, ".ppm");
    break;
  case IMG_SBI:
    info.sbiInfo.redBits = 5;
    info.sbiInfo.greenBits = 6;
    info.sbiInfo.blueBits = 5;
    info.sbiInfo.yOrigin = 1;
    strcat(filenameEx, ".sbi");
    break;

  case IMG_TGA32:
    info.tgaInfo.yOrigin = 0;
    strcat(filenameEx, ".tga");
    break;
  default:
    GDBG_ERROR("csimPicSave", "invalid image type %d\n",type);
    return FXFALSE;
  }

#if !defined(__unix__)
  strlwr(filenameEx);
#endif

  /* note we first convert the framebuffer which is in 16-bit RGB format
     to standard 32-bit ARGB format (in memory) for the image library
   */

  buf = (FxU32 *)info.any.data;
  for (y = 0; y < pBuff->height; y++) {
    for (x = 0; x < pBuff->width; x++) {
        FxU8 a, r, g, b;
        hwcReadColor(hws, pBuff, x,y, &a, &r, &g, &b);
#ifdef __unix__
        /* since Suns are big endian, store as BGRA, so when read */
        *buf++ = (b << 24) | (g << 16) | (r << 8) | a;
#else
        *buf++ = (a << 24) | (r << 16) | (g << 8) | b;
#endif
    }
  }
  if (!imgWriteFile(filenameEx,&info, (ImgType) type, info.any.data))
    GDBG_ERROR( "imgWriteFile", "file '%s' failed: %s\n", 
                filename, imgGetErrorString() );
  hwcMemFree(info.any.data);
  return FXTRUE;
}
#else
static FxBool
hwcBufferSave(HwcSimulator *hws, HwcBuffer *pBuff, const char *filename, FxU32 type)
{
  return FXTRUE;
}
#endif /* !KERNEL */

/*------------------------------------------------------------------------------
@func hwcAllocMemRegion
@date 8/15/98
@arg HwcContext * - hwc - hardware context
@arg FxU32 - memType - type of memory to allocate (HWC_BUFFER_MEM_LOCAL, 
                       HWC_BUFFER_MEM_AGP, HWC_BUFFER_MEM_SYSTEM)
@arg FxU32 - sizeInBytes - amount of memory needed
@arg FxU32 - align - specifies number of bits to align by, 0 = no alignment 
                     and whether we want odd or even alignment
@arg HwcMemRegion * - pBlock - memory descriptor
@return FxBool - FXTRUE on success, FXFALSE on error
@imp mlwp
@sect hwc
@html
Allocate a memory region of the specified size, alignment and type.
@end
------------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcAllocMemRegion(HwcContext *hwc, FxU32 memType, FxU32 sizeInBytes, FxU32 align, HwcMemRegion *pBlock, FxU32 hints)
{
  GDBG_INFO(4,"hwcAllocMemRegion(0x%lx, %d, %d, %d, 0x%lx)\n", 
            hwc, memType, sizeInBytes, align, pBlock);

  if ( hwc == NULL ) {
     GDBG_ERROR("hwcAllocMemRegion", "NULL context\n");
     return FXFALSE;
  }

  if ( pBlock == NULL ) {
     GDBG_ERROR("hwcAllocMemRegion", "NULL block\n");
     return FXFALSE;
  }

  pBlock->memType = memType;
  pBlock->sizeInBytes = sizeInBytes;
  return (* hwc->AllocMemRegion)(hwc, memType, sizeInBytes, align, pBlock, hints);
}

/*------------------------------------------------------------------------------
@func hwcFreeMemRegion
@date 8/15/98
@arg HwcContext * - hwc - hardware context
@arg HwcMemRegion * - pBlock - memory descriptor
@return FxBool - FXTRUE on success, FXFALSE on error
@imp mlwp
@sect hwc
@html
Free a previously allocated memory region 
@end
------------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcFreeMemRegion(HwcContext *hwc, HwcMemRegion *pBlock)
{
  GDBG_INFO(4,"hwcFreeMemRegion(0x%lx, 0x%lx)\n", hwc, pBlock);

  if ( hwc == NULL ) {
     GDBG_ERROR("hwcFreeMemRegion", "NULL context\n");
     return FXFALSE;
  }

  if ( pBlock == NULL ) {
     GDBG_ERROR("hwcFreeMemRegion", "NULL block\n");
     return FXFALSE;
  }

  return (* hwc->FreeMemRegion)(hwc, pBlock);
}

/*---------------------------------------------------------------------------
   Initialize a buffer
  ---------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcInitBuffer(HwcBuffer *buff, HwcContext *hwc, FxU32 width, FxU32 height, 
              HwcBufferType type, HwcPixelFormat format)
{
  GDBG_INFO(4,"hwcInitBuffer(0x%lx)\n", hwc);

  if ( buff == NULL ) {
     GDBG_ERROR("hwcInitBuffer", "NULL buffer\n");
     return FXFALSE;
  }

  memset( buff, 0, sizeof(HwcBuffer));
  buff->name = "no name";
  buff->hwc = hwc;
  buff->width = width;
  buff->height = height;
  buff->type = type;
  buff->format = format;
  buff->stride = 0;
  buff->agp = FXFALSE;
  buff->hints = HWC_BUFFER_SHOW_UPDATES;
  buff->physicalBase = 0;
  buff->physicalEnd = 0;
  buff->next = hwc->buffers;
  hwc->buffers = buff;

  return FXTRUE;
}

/*---------------------------------------------------------------------------
   Allocate a buffer
  ---------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcAllocBuffer(HwcBuffer *buff, HwcContext *hwc, FxU32 width, FxU32 height, 
              HwcBufferType type, HwcPixelFormat format, FxU32 hints)
{
  FxU32 physicalBase = buff->physicalBase;
  GDBG_INFO(4,"hwcAllocBuffer(0x%lx)\n", hwc);

  if ( buff == NULL ) {
     GDBG_ERROR("hwcAllocBuffer", "NULL buffer\n");
     return FXFALSE;
  }

  hwcInitBuffer(buff, hwc, width, height, type, format);
  if ( hints & HWC_BUFFER_BASEADDR )
      buff->physicalBase = physicalBase;
  buff->hints |= hints;
  if (!(* hwc->AllocBuffer)(buff)) {
    GDBG_ERROR("hwcAllocBuffer", "not enough memory\n");
    return FXFALSE;
  }

  return FXTRUE;
}

FX_EXPORT FxBool FX_CSTYLE
hwcAllocNamedBuffer(HwcBuffer *buff, HwcContext *hwc, int width, int height, HwcBufferType type, 
         HwcPixelFormat format, FxU32 hints, char *name)
{
  if ( buff == NULL ) {
     GDBG_ERROR("hwcAllocNamedBuffer", "NULL buffer\n");
     return FXFALSE;
  }

  if (!(hwcAllocBuffer(buff, hwc, width, height, type, format, hints))){
    return FXFALSE;
  }

  buff->name = name;

  if ( hints & HWC_BUFFER_VISIBLE )
      hwcShowBuffer(buff, FXTRUE);

  return FXTRUE;
}

/*---------------------------------------------------------------------------
   Make buffer visible
  ---------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcShowBuffer(HwcBuffer *buff, FxBool visible)
{
  HwcContext *hwc;
  HwcSimulator *hws;
  HwcWindow *pWnd;

  if ( buff == NULL ) {
     GDBG_ERROR("hwcShowBuffer", "NULL buffer\n");
     return FXFALSE;
  }

  hwc = buff->hwc;

  if ( visible ) {
    return hwcNewViewWindow(buff) != NULL;
  } else {
    /* check if any windows are displaying this buffer */

    for ( hws = hwc->hws; hws; hws = hws->next ) {
      for ( pWnd = hws->windows; pWnd; pWnd = pWnd->next ) {
        if ( pWnd->buffer == buff ) {
          hwcDeleteWindow(pWnd);
        }
      }
    }
  }

  return FXTRUE;
}


/*---------------------------------------------------------------------------
   Update all windows showing this buffer
  ---------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcUpdateBuffer(HwcBuffer *buff)
{
  HwcContext *hwc = buff->hwc;
  HwcSimulator *hws;
  HwcWindow *pWnd;

  for ( hws = hwc->hws; hws; hws = hws->next ) {
    for ( pWnd = hws->windows; pWnd; pWnd = pWnd->next ) {
      if ( pWnd->buffer == buff ) {
        hwcWindowSetBuffer(pWnd, buff);
      }
    }
  }

  return FXTRUE;
}

/*---------------------------------------------------------------------------
   Reallocate a buffer
  ---------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcReallocBuffer(HwcBuffer *buff, HwcContext *hwc, FxU32 width, FxU32 height, 
              HwcBufferType type, HwcPixelFormat format, FxU32 hints)
{
  GDBG_INFO(4,"hwcReallocBuffer(0x%lx)\n", hwc);

  if ( buff == NULL ) {
     GDBG_ERROR("hwcReallocBuffer", "NULL buffer\n");
     return FXFALSE;
  }

  (* buff->hwc->FreeBuffer)(buff);
  buff->width = width;
  buff->height = height;
  buff->type = type;
  buff->format = format;
  buff->stride = 0;
  buff->agp = FXFALSE;
  buff->hints = hints|HWC_BUFFER_SHOW_UPDATES;
  buff->physicalBase = 0;
  buff->physicalEnd = 0;
  if (!(* hwc->AllocBuffer)(buff)) {
    hwcFreeBuffer(buff);
    return FXFALSE;
  }

  hwcUpdateBuffer(buff);

  return FXTRUE;
}

/*---------------------------------------------------------------------------
   Initialize a buffer. 
       Initialize a buffer. If ctl is HWC_BUFINIT_RANDOM then the buffer is 
       initialized to a random value. If ctl is HWC_BUFINIT_DITHER then pixel
       is an RGB888 value and the buffer is initialized to a dithered version
       of that color. Otherwise it is initialized to the specified pixel value
       which should be in the native pixel format for that buffer.
  ---------------------------------------------------------------------------*/
FX_EXPORT void FX_CSTYLE
hwcBufferInit(HwcBuffer *pBuff, FxI32 xmin, FxI32 ymin, FxI32 w, FxI32 h, FxU32 pix, HwcBufferInit ctl)
{
  HwcContext *hwc;
  HwcSimulator *hws;

  GDBG_INFO(4,"hwcBufferInit(0x%lx, %d, %d, %d, %d, 0x%lx, 0x%lx)\n", pBuff, xmin, ymin, w, h, pix, ctl);

  if ( pBuff == NULL ) {
     GDBG_ERROR("hwcBufferInit", "NULL buffer\n");
     return;
  }

  hwc = pBuff->hwc;

  if ( pBuff->hints & HWC_BUFFER_WIREFRAME ) {
    hwcFillRectangle(pBuff, xmin, ymin, w, h);
    return;
  }

  for ( hws = hwc->hws; hws; hws = hws->next ) {
    if ( !hws->environment.skipRendering ) {
      (* hws->BufferInit)(hws, (HwcBufferSpec*)pBuff, xmin, ymin, w, h, pix, ctl);
    }
  }
}

/*---------------------------------------------------------------------------
   Set a buffer
  ---------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcSetBuffer(HwcBuffer *buff, HwcBufferType type)
{
  HwcContext *hwc;
  HwcSimulator *hws;
  HwcWindow *pWnd;

  GDBG_INFO(4,"hwcSetBuffer(0x%lx, %d)\n", buff, type);

  if ( buff == NULL ) {
     return FXFALSE;
  }

  hwc = buff->hwc;

  /* update the device state */

  if ( hwc->fbiMemoryFifoEn == 0 ) {
    if (!(* hwc->SetBuffer)(buff, type)) {
      return FXFALSE;
    }
  }

  /* check if any windows are displaying this buffer */

  for ( hws = hwc->hws; hws; hws = hws->next ) {
    for ( pWnd = hws->windows; pWnd; pWnd = pWnd->next ) {
      if ( pWnd->stdBuffer && ( pWnd->which == type )) {
        (* pWnd->SetBuffer)(pWnd, buff);
      }
    }
  }

  hwc->stdBuffers[type] = buff;
  return FXTRUE;
}

/*---------------------------------------------------------------------------
   Free a buffer
  ---------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcFreeBuffer(HwcBuffer *buff)
{
  int i;
  HwcContext *hwc;
  HwcBuffer *pBuff, *prevBuff = NULL;
  HwcWindow *pWnd;
  HwcSimulator *hws;

  GDBG_INFO(4,"hwcFreeBuffer(0x%lx)\n", buff);

  if ( buff == NULL ) {
     GDBG_ERROR("hwcFreeBuffer", "NULL buffer\n");
     return FXFALSE;
  }

  hwc = buff->hwc;

  /* check buffer is not in use */

  for ( i = 0; i < HWC_BUF_COUNT; i++ ) {
    if ( hwc->stdBuffers[i] == buff )
      hwc->stdBuffers[i] = NULL;
  }

  /* unlink buffer from contexts buffer list */

  for ( pBuff = hwc->buffers; pBuff; pBuff = pBuff->next ) {
    if ( pBuff == buff ) {
      if ( prevBuff )
        prevBuff->next = pBuff->next;
      else if ( pBuff == hwc->buffers ) 
        hwc->buffers = pBuff->next; 
      break;
    }
    prevBuff = pBuff;
  }

  /* check if any windows are displaying this buffer */

  for ( hws = hwc->hws; hws; hws = hws->next ) {
    for ( pWnd = hws->windows; pWnd; pWnd = pWnd->next ) {
      if ( pWnd->buffer == buff ) {
        (* pWnd->SetBuffer)(pWnd, NULL);
      }
    }
  }

  (* buff->hwc->FreeBuffer)(buff);

  return FXTRUE;
}

FX_EXPORT void FX_CSTYLE
hwcPrintBuffer(HwcBuffer *buff) {
#define FN_NAME "hwcPrintBuffer"
   char *allocMethod = "unknown";
   char *allocLocation = "unknown";

   if (buff->hints & HWC_BUFFER_TILED) {
       allocMethod = "tiled";
   } else if ( buff->hints & HWC_BUFFER_PURE_LINEAR ) {
       allocMethod = "pure linear";
   } else if ( buff->hints & HWC_BUFFER_BLOCK_LINEAR) {
       allocMethod = "block linear";
   } else {
       allocMethod = "linear";
   }

  switch (SST_GET_FIELD(buff->hints, HWC_BUFFER_MEM) ) {
  case HWC_BUFFER_MEM_LOCAL:
    allocLocation = "local";
    break;
  case HWC_BUFFER_MEM_AGP:
    allocLocation = "agp";
    break;
  case HWC_BUFFER_MEM_SYSTEM:
    allocLocation = "system";
    break;
  };

  GDBG_INFO(0, "%s (%s %s), size = %d(0x%lx), addr = 0x%lx, end = 0x%lx\n",
            buff->name ? buff->name : "(no name)",  allocLocation, allocMethod,
            buff->sizeInBytes, buff->sizeInBytes, buff->physicalBase, buff->physicalEnd);

  if ( buff->hints & HWC_BUFFER_PURE_LINEAR )
      return;

  GDBG_INFO(0, 
            "\twidth = %d, height = %d, format = %s, bpp = %d, stagger = %d\n",
            buff->width, buff->height,
            hwcPixelFormatToString(buff->format), buff->bpp,
            (buff->hints & HWC_BUFFER_STAGGERED) != 0);

  GDBG_INFO(0, 
            "\tdepth = %d, stride = %d, bswiz: %d, wswiz %d, pack: %d\n",
            (buff->hints & HWC_BUFFER_AUX) != 0, buff->stride,
            (buff->hints & HWC_BUFFER_HOST_BYTE_SWIZZLE) != 0, 
            (buff->hints & HWC_BUFFER_HOST_WORD_SWIZZLE) != 0,
            buff->srcPack);
#undef FN_NAME
}

// update video display based on current hardware state
FX_EXPORT void FX_CSTYLE
hwcVideoUpdate(HwcContext *hwc)
{
  HwcSimulator *hws;
  HwcWindow *pWnd;
  
  for ( hws = hwc->hws; hws; hws = hws->next ) {
    (* hws->VideoUpdate)(hws);
    for ( pWnd = hws->windows; pWnd; pWnd = pWnd->next ) {
      if (( hws->vecInfo.type & HWC_TEST_VIDEO0) &&
          ( pWnd->buffer == hwc->stdBuffers[HWC_BUF_VIDEO0] )) {
           hwcWindowSetBuffer(pWnd, hwc->stdBuffers[HWC_BUF_VIDEO0]);
           hwcRefreshWindow(pWnd);
      }
      if (( hws->vecInfo.type & HWC_TEST_VIDEO1) &&
          ( pWnd->buffer == hwc->stdBuffers[HWC_BUF_VIDEO1] )) {
           hwcWindowSetBuffer(pWnd, hwc->stdBuffers[HWC_BUF_VIDEO1]);
           hwcRefreshWindow(pWnd);
      }
    }
  }
}

// Returns the number of bits that a hwc pixel format specifies

FX_EXPORT FxU32 FX_CSTYLE
hwcFormatToBPP( HwcPixelFormat pixelFormat) 
{
  switch (pixelFormat) {
    case HWC_PIXFMT_1BPP:
      return 1;
    case HWC_PIXFMT_P_8:
    case HWC_PIXFMT_P_8_6666:
    case HWC_PIXFMT_A_8:
    case HWC_PIXFMT_I_8:
    case HWC_PIXFMT_AI_44:
    case HWC_PIXFMT_RGB_332:
      return 8;
    case HWC_PIXFMT_YUYV:
    case HWC_PIXFMT_UYVY:
      return 16;
    case HWC_PIXFMT_YUV_411:
    case HWC_PIXFMT_YIQ_422:
      return 8;
    case HWC_PIXFMT_AP_88:
    case HWC_PIXFMT_AI_88:
    case HWC_PIXFMT_AYIQ_8422:
    case HWC_PIXFMT_RGB_565:
    case HWC_PIXFMT_ARGB_1555:
    case HWC_PIXFMT_ARGB_8332:
    case HWC_PIXFMT_ARGB_4444:
    case HWC_PIXFMT_FBCMP_16:
      return 16;
    case HWC_PIXFMT_FBCMP_32:
      return 32;
    case HWC_PIXFMT_TXCMP_4:
      // this is odd.. 32 bits?
      return 32;
    case HWC_PIXFMT_RGB_888:
      return 24;
    case HWC_PIXFMT_ARGB_8888:
      return 32;
    case HWC_PIXFMT_AA_16:
      return 16;
    case HWC_PIXFMT_AA_32:
      return 32;
    case HWC_PIXFMT_AP_44:
        return 8;
    case HWC_PIXFMT_AYUV_32:
      return 32;
    case HWC_PIXFMT_AA_1555:
      return 16;
    case HWC_PIXFMT_VIP_ANC:
      return 8;
    default:
        GDBG_ERROR("hwcFormatToBPP", "Unknown pixel format %d\n", pixelFormat);
      return (0xffffffff);
  }
}