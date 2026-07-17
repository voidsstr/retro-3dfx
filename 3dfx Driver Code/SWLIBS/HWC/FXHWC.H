#ifndef __FXHWC_H__
#define __FXHWC_H__

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
** $Revision: 18$
** $Date: 10/11/00 7:39:56 PM$
*/
#if defined(KERNEL) && !defined(KERNEL_NT)
#include "vxd.h"
#undef PCI_DEVICE_ID
#endif
#if defined(BUILD_HWC)
#define FX_DLL_DEFINITION
#endif
#include <stdlib.h>
#include <3dfx.h>
#include <fx64.h>
#include <fxdll.h>
#include <gdebug.h>

/* Make C Decls Palatable to C++ Compilers  */
#ifdef __cplusplus
extern "C" {
#endif

// split an I64 into two I32 for tracing output
#define FX_SPLIT64(_d, _s) { _d[0] = FX_LO64(_s); _d[1] = FX_HI64(_s); }

// macros for accessing fields

#define SST_SET_FIELD(_r, _f) (((_r) << _f##_SHIFT) & _f)
#define SST_GET_FIELD(_r, _f) (((_r) & _f) >> _f##_SHIFT)

// this crazy macro fixes the binary point within a floating point
// number so that it has 'fracbits' to the right of the binary point
// it also adds a '1' bit to the MSB so that slightly negative numbers
// end up looking like twos complement numbers (they carry out from the '1')
#ifndef FLOAT_FIX
#define FLOAT_FIX(f,fracbits) ((f)+(float)(3<<(22-(fracbits))))
#endif

#ifndef SST_FLOAT
#define SST_FLOAT(f,scale,shift) (unsigned long)((f)*((scale)*(float)(1<<shift)))
#endif

// this crazy macro tests the sign bit of a float by loading it into
// an integer register and then testing the sign bit of the integer
#define FLOAT_ISNEG(f) ((*(int *)(&(f))) < 0)

// these crazy macros returns the sign of a number (1 if >= 0; -1 if < 0)
#define ISIGN(x) (((x) | 0x40000000L) >> 30)
#define FSIGN(f) ISIGN(*(long *)&f)

#ifndef BIT
#define BIT(n)  (1UL<<(n))
#endif
#define SST_MASK(n) ((n) ? (0xFFFFFFFFL >> (32-(n))) : (0x0L))

#define SST_MASK64(n) FX_MASK64(n)

#ifndef ABS
#define ABS(x) ((x)<0 ? -(x) : (x))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(__x, __y) (((__x) < (__y)) ? (__x) : (__y))
#endif

#ifndef SIGN_EXTEND
#define SIGN_EXTEND(x,nbits) ((((signed)(x))<<(32-(nbits)))>>(32-(nbits)))
#endif

#ifndef DIRECTX
#ifndef LOWORD
#define LOWORD(x) ((FxU16)((x)&0xFFFF))
#endif

#ifndef HIWORD
#define HIWORD(x) ((FxU16)((x)>>16))
#endif
#endif

#define HWC_SWAP16(_p) {             \
    FxU32 *p = (FxU32 *)(_p);          \
    *p = ( *p >> 16 ) | ( *p << 16); \
}

#define HWC_SWAP8(_p) {             \
    FxU32 *p = (FxU32 *)(_p);          \
    *p = ((*p & 0xff000000 ) >> 24 ) | ((*p & 0xff0000 ) >> 8 )|((*p & 0xff00 ) << 8 )|((*p & 0xff ) << 24 ); \
}

/* Allow SourceSafe to track Revision values */
#define HWC_H_REV "$Revision: 18$"

/* macro to convert megabytes to bytes */

#define HWC_MBYTE(n)	(((FxU32)(n))<<20)
#define DEAD		0xDEAD

/* macro to create a 32 bit word with the n'th bit set */

#ifndef BIT
#define BIT(n)  (1UL<<(n))
#endif

#define CEIL(x,y)      ( ((x)+(y)-1) / (y) )	// divide x/y, rounding up

/* More slots than you will ever see in a consumer system. Must be
  less than 15 for software emulation */

#define HWC_MAX_BOARDS       8
#define HWC_NUM_BASE_ADDR    9 /* Max # base addresses */
#define HWC_VIEW_WINDOW_NAME "Viewer"

/*-------------------------------------------------------------------
  HWC Memory allocation macros
  -------------------------------------------------------------------*/

/*-------------------------------------------------------------------
  Structure: _HwcMemEntry
  Date: 3/15/96
  Implementor(s): jdt
  Library: hwc ( memory management )
  Description:
    Linked structure that keeps track of allocated memory.
  Members:
    size    - size in bytes of memory allocated
    type    - type of memory object
    use_count - number of references to this object
    pointer - pointer returned to application
    actual_pointer - actual heap pointer returned by the
                     os, it may be different than pointer
                     for aligned mallocs
    where   - pointer to string with source file
    line    - line number in source file where allocated
    next    - points to next entry in list ( NULL at end )
    gap     - buffer area before and after allocated space to
              check for data corruption
  -------------------------------------------------------------------*/

#define HWC_MEM_GAP  ((FxU8)0xFD)
#define HWC_MEM_FREE ((FxU8)0xDD)

#define HWC_GAP_SIZE    4
typedef struct HWC_AME {
    FxU32  size;
    FxU16  type;
    FxU16  use_count;
    void   *pointer;
    void   *actual_pointer;
    char   *where;
    FxU32  line;
    struct HWC_AME *next;
    FxU8   gap[HWC_GAP_SIZE];
} _HwcMemEntry;

/*-------------------------------------------------------------------
  Structure: HwcMemStats
  Date: 3/15/96
  Implementor(s): jdt
  Library: hwc ( memory managment )
  Description:
    Vehicle for returning memory allocation statistics to the user.
  Members:
    peakMemAllocated - peak amount of memory allocated over life of
        application, in bytes.
    currentMemAllocated - amount of memory currently allocated using
        hwc memory allocators
  -------------------------------------------------------------------*/
typedef struct {
    FxU32 peakMemAllocated;
    FxU32 currentMemAllocated;
} HwcMemStats;

/*-------------------------------------------------------------------
  Function: hwcMemEntry
  Date: 4/24/96
  Implementor(s): mlwp
  Library: hwc ( memory managment )
  Description:
    Return the _HwcMemEntry associated with a memory object
    allocated by the hwc memory manager. This is an internal
    function and in general should not be accessed directly.
    Use the higher level functions to access this data structure.
  Arguments:
    ptr - pointer to memory object
  Return:
    pointer to _HwcMemEntry associated with this object
  -------------------------------------------------------------------*/
#define hwcMemEntry( ptr ) (((_HwcMemEntry *)(ptr))-1)

/*-------------------------------------------------------------------
  Function: hwcMemGetSize
  Date: 4/24/96
  Implementor(s): mlwp
  Library: hwc ( memory managment )
  Description:
    Return the size in bytes of this memory object
    allocated by the hwc memory manager.
  Arguments:
    ptr - pointer to memory object
  Return:
    size in bytes of this object
  -------------------------------------------------------------------*/
#define hwcMemGetSize( ptr ) ( hwcMemEntry( ptr )->size )

/*-------------------------------------------------------------------
  Function: hwcMemType
  Date: 4/24/96
  Implementor(s): mlwp
  Library: hwc ( memory managment )
  Description:
    Set the type of this memory object allocated by
    the hwc memory manager.
  Arguments:
    ptr - pointer to memory object
    type - type of the memory object
  Return:
    type of object
  -------------------------------------------------------------------*/
#define hwcMemType( ptr, t ) ( hwcMemEntry( ptr )->type = t )

/*-------------------------------------------------------------------
  Function: hwcMemGetType
  Date: 4/24/96
  Implementor(s): mlwp
  Library: hwc ( memory managment )
  Description:
    Return the type of this memory object allocated by
    the hwc memory manager.
  Arguments:
    ptr - pointer to memory object
  Return:
    type of this object
  -------------------------------------------------------------------*/
#define hwcMemGetType( ptr ) ( hwcMemEntry( ptr )->type )

/*-------------------------------------------------------------------
  Function: hwcMemRef
  Date: 4/24/96
  Implementor(s): mlwp
  Library: hwc ( memory managment )
  Description:
    Increment the reference count of this memory object allocated by
    the hwc memory manager.
  Arguments:
    ptr - pointer to memory object
  Return:
    The number of references to this object
  -------------------------------------------------------------------*/
#define hwcMemRef( ptr ) ( ++(hwcMemEntry( ptr )->use_count) )

/*-------------------------------------------------------------------
  Function: hwcMemGetRef
  Date: 4/24/96
  Implementor(s): mlwp
  Library:hwc ( memory managment )
  Description:
    Return the number of references to this memory object allocated by
    the hwc memory manager.
  Arguments:
    ptr - pointer to memory object
  Return:
    Number of references to this object
  -------------------------------------------------------------------*/
#define hwcMemGetRef( ptr ) ( hwcMemEntry( ptr )->use_count )

/*-------------------------------------------------------------------
  Function: hwcMemDeref
  Date: 4/24/96
  Implementor(s): mlwp
  Library: hwc ( memory managment )
  Description:
    Decrement the reference count of this memory object allocated by
    the hwc memory manager. Returns the remaining number of references
    to the object.
  Arguments:
    ptr - pointer to memory object
  Return:
    The number of outstanding references to this object
  -------------------------------------------------------------------*/
#define hwcMemDeref( ptr ) ( --(hwcMemEntry( ptr )->use_count) )

/*-------------------------------------------------------------------
  Function: hwcMemDerefFree
  Date: 4/24/96
  Implementor(s): mlwp
  Library: hwc ( memory managment )
  Description:
    Decrement the reference count of this memory object allocated by
    the hwc memory manager. If the reference count falls to zero the
    object is deleted.
  Arguments:
    ptr - pointer to memory object
  Return:
    The number of outstanding references to this object
  -------------------------------------------------------------------*/
#define hwcMemDerefFree( mem ) _hwcMemDerefFree( (mem), __FILE__, __LINE__ )

/*-------------------------------------------------------------------
  Function: hwcMemCalloc
  Date: 3/16/96
  Implementor(s): jdt
  Library: hwc ( memory managment )
  Description:
    Just like CAPI calloc, except is does error checking and
    keeps track of allocated memory for statistical/debugging
    purposes
  Arguments:
    size - size in bytes of an element
    num - number of elements to allocate
  Return:
    void pointer to new heap memory
  -------------------------------------------------------------------*/
#define hwcMemCalloc( num, size ) _hwcMemRealloc( NULL, (num)*(size), __FILE__, __LINE__ )

/*-------------------------------------------------------------------
  Function: hwcMemAlignedCalloc
  Date: 3/18/96
  Implementor(s): jdt
  Library: hwc ( memory managment )
  Description:
    Allocateds an aligned chunk of memory so that the base of the
    memory falls on an even alignment boundary. NOTE!!! This memory
    MUST BE FREED THROUGH THE GDMEMFREE API OR THE HEAP WILL BE
    FRAGGED.
  Arguments:
    alignment - n-byte aligment
    size - size in bytes of an element
    num - number of elements to allocate
  Return:
    void pointer to new heap memory
  -------------------------------------------------------------------*/
#define hwcMemAlignedCalloc( num, size, alignment ) \
        _hwcMemAlignedCalloc( (num), (size), \
        (alignment), __FILE__, __LINE__ )


/*-------------------------------------------------------------------
  Function: hwcMemAlloc
  Date: 3/16/96
  Implementor(s): jdt
  Library: hwc ( memory managment )
  Description:
    Just like CAPI malloc, except it does error checking and
    keeps track of allocated memory for statistical/debugging
    purposes
  Arguments:
    size - size in bytes of to be allocated
  Return:
    void pointer to new heap memory
  -------------------------------------------------------------------*/
#define hwcMemAlloc( size ) _hwcMemRealloc( NULL, (size), __FILE__, __LINE__ )

/*-------------------------------------------------------------------
  Function: hwcMemRealloc
  Date: 11/9/96
  Implementor(s): mlwp
  Library: hwc ( memory managment )
  Description:
    Just like CAPI realloc, except it does error checking and
    keeps track of allocated memory for statistical/debugging
    purposes. If the input pointer is NULL then this acts just
    like hwcMemAlloc.
  Arguments:
    ptr  - pointer to allocated memory
    size - size in bytes of to be allocated
  Return:
    void pointer to new heap memory
  -------------------------------------------------------------------*/
#define hwcMemRealloc( ptr, size ) _hwcMemRealloc((ptr),(size), __FILE__, __LINE__ )

/*-------------------------------------------------------------------
  Function: hwcMemFree
  Date: 3/16/96
  Implementor(s): jdt
  Library: hwc ( memory managment )
  Description:
    Just like CAPI free, except it does error checking and
    keeps track of allocated memory for statistical/debugging
    purposes
  Arguments:
    mem - pointer to heap memory to be freed
  Return:
    none
  -------------------------------------------------------------------*/
#define hwcMemFree( mem ) _hwcMemFree( (mem), __FILE__, __LINE__ )

/*-------------------------------------------------------------------
  Function: hwcMemCheck
  Date: 11/9/96
  Implementor(s): mlwp
  Library: hwc ( memory managment )
  Description:
    Validate the allocated memory
  Arguments:
    None
  Return:
    FXTRUE if allocated memory ok, FXFALSE otherwise
  -------------------------------------------------------------------*/
#define hwcMemCheck() _hwcMemCheck( __FILE__, __LINE__ )

/*-------------------------------------------------------------------
  Function: hwcStrDup
  Date: 1/15/97
  Implementor(s): mlwp
  Library: hwc ( memory managment )
  Description:
    Just like CAPI strdup, except it does error checking and
    keeps track of allocated memory for statistical/debugging
    purposes.
  Arguments:
    s  - string to be duplicated
  Return:
    duplicated string
  -------------------------------------------------------------------*/
#define hwcStrDup( s ) _hwcStrDup((s), __FILE__, __LINE__ )

/*-------------------------------------------------------------------
  Function: hwcMemDebug
  Date: 11/9/96
  Implementor(s): mlwp
  Library: hwc ( memory managment )
  Description:
    Enable/disable memory debugging
  Arguments:
    flags - memory debugging state
  Return:
    void
  -------------------------------------------------------------------*/

#define HWC_MEM_CHECK   1
#define HWC_MEM_NO_FREE 2

void hwcMemDebug(FxU32 flags) ;

FX_ENTRY void * FX_CALL _hwcMemRealloc(void *ptr, size_t size, char *where, FxU32 line );
FX_ENTRY char * FX_CALL _hwcStrDup(const char *s, char *where, FxU32 line );
FX_ENTRY FxBool FX_CALL _hwcMemCheck(char *where, FxU32 line );
FX_ENTRY void * FX_CALL _hwcMemAlignedCalloc( size_t num, size_t size, FxU32 alignment, char *where, FxU32 line );
FX_ENTRY void FX_CALL _hwcMemFree( void *ptr, char *where, FxU32 line);
FX_ENTRY int  FX_CALL _hwcMemDerefFree( void *mem, char *where, FxU32 line );
FX_ENTRY void FX_CALL hwcMemStats( HwcMemStats *s );
FX_ENTRY void FX_CALL hwcMemInit( FxU32 flags );
FX_ENTRY void FX_CALL hwcMemShutdown( void );

#define HWC_NEW(t) ((t *)hwcMemAlloc(sizeof(t)))
#define HWC_NEW_VEC(t, n) ((t *)hwcMemAlloc(n*sizeof(t)))
#define HWC_FREE(ptr) free(ptr)

/*-------------------------------------------------------------------
  Enumeration defining known 3DFX device types
  -------------------------------------------------------------------*/

typedef FxI32 HwcDeviceClass;
#define HWC_VOODOO  0
#define HWC_VG96    1
#define HWC_VOODOO2 2
#define HWC_H3      3
#define HWC_SST2    4
#define HWC_NEPTUNE 5  /* XXX: wtf is neptune */

/*-------------------------------------------------------------------
   Just to unconfuse myself:
  
        CHIP            FBI-REV TMU-REV DEV-ID
        SST1-0.6u       1       0       1
        SST1-0.5u       2       1       1
        SST-96          2       (1)     2
        H3              3       4       3
  	    H4		        ?	    ?	4?
  	    SST2				5
  -------------------------------------------------------------------*/

/* 3Dfx Voodoo Graphics */
#define HWC_VID_3DFX                0x121A
#define HWC_DID_DEFAULT             0xFFFF

#define HWC_DID_SST1                0x0001
#define HWC_DID_VOODOO2             0x0001
#define HWC_DID_SST96               0x0002
#define HWC_DID_BANSHEE             0x0003
#define HWC_DID_AVENGER             0x0004 /* Avenger */
#define HWC_DID_AVENGER_SS          0x0005 /* Avenger speed sorted */
#define HWC_DID_AVENGER_PLUS        0x0006
#define HWC_DID_AVENGER_PLUS_SS     0x0007
#define HWC_DID_AVENGER_PLUS_SLI    0x0008
#define HWC_DID_AVENGER_PLUS_SLI_SS 0x0009 /* Avenger+ SLI speed sorted */
/* 0xA-0xF Reserved for future Voodoo-based products */
#define HWC_DID_SST2                0x0010 /* SST2 */
#define HWC_DID_SST2_SS             0x0011 /* SST2 speed sorted */
/* 0x12 SST2 TBD */
/* 0x13 SST2 TBD */
#define HWC_DID_R2                  0x0014 /* R2 */
#define HWC_DID_R2_SS               0x0015 /* R2 speed sorted */
/* 0x16 R2 TBD */
/* 0x17 R2 TBD */
/* 0x18-0x1F Reserved for future SST2-based products */


/* Alliance AT3D */
#define HWC_VID_AT3D                0x1142
#define HWC_DID_AT3D                0x643d

/* Trident T96851 */
#define HWC_VID_T968                0x1023
#define HWC_DID_T968                0x9660

/* MRTI M510 */
#define HWC_VID_MRTI                0x1293
#define HWC_DID_MRTI                0x0510

/* Macronix MX86251 Fixme!!! these are bogus !!! XXX ??? */
#define HWC_VID_MCRX                0x10d9
#define HWC_DID_MCRX                0x8626

/*
    16 Busses ( of possible 256, I am making the
                assumption that busses are numbered
                in increasing order and that no
                PC will have more than 16 busses )
    32 Slots Per Bus

    Device Number = Bus Number * 32 + Slot Number
*/

#define HWC_MAX_PCI_DEVICES 512

#define CONFIG_ADDRESS_PORT 0xCF8
#define CONFIG_DATA_PORT    0xCFC
#define CONFIG_ADDRESS_ENABLE_BIT 0x80000000UL

#define CONFIG_MAPPING_ENABLE_BYTE 0x80
#define CONFIG_MAPPING_DISABLE_BYTE 0x00

#define CONFIG_SPACE_ENABLE_PORT 0xCF8
#define FORWARD_REGISTER_PORT    0xCFA
#define CONFIG_MAPPING_OFFSET    0xC000

#define HWC_PCI_INTERRUPT  0x1A
#define HWC_PCI_FUNC_CODE  0xB1
#define HWC_PCI_PHYS_ADDR  0x000FFE6E

#define BYTE0(a) ((a) & 0xff)
#define BYTE1(a) (((a) >> 8) & 0xff)
#define VXDREFCOUNT(a) (((a) >> 16) & 0xff)

/*-------------------------------------------------------------------
  Enumeration defining PC memory types
  -------------------------------------------------------------------*/
typedef FxU32 HwcMemType;
#define HWC_MEM_TYPE_UNCACHEABLE           0
#define HWC_MEM_TYPE_WRITE_COMBINING       1
#define HWC_MEM_TYPE_WRITE_THROUGH         4
#define HWC_MEM_TYPE_WRITE_PROTECTED       5
#define HWC_MEM_TYPE_WRITEBACK             6

/*-------------------------------------------------------------------
  Enumeration defining supported register access modes
  -------------------------------------------------------------------*/
typedef FxU32 HwcIOFlag;
#define HWC_READ_ONLY  0
#define HWC_WRITE_ONLY 1
#define HWC_READ_WRITE 2

/*-------------------------------------------------------------------
  Enumeration defining memory areas for backdoor accesses
  -------------------------------------------------------------------*/
#define HWC_MEM_CSIM 0x1
#define HWC_MEM_HSIM 0x2
#define HWC_MEM_HW   0x4
#define HWC_MEM_AGP  0x8
#define HWC_MEM_ALL  0xf

/*-------------------------------------------------------------------
  Enumeration defining controls for bit acurate csim
  -------------------------------------------------------------------*/

#define HWC_BA_CALC     BIT(0)  // enable bit accurate computations
#define HWC_BA_NO_CHECK BIT(1)  // disable checking of bit accurate results
/*-------------------------------------------------------------------
  Enumeration defining existance of PCI device. A PCI device can be 
  either none existant, implemented as a hardware PCI device, or
  emulated through a C and/or HW simulator
  -------------------------------------------------------------------*/
typedef FxU32 HwcPCIState;
#define HWC_NONE              (0<<0)
#define HWC_HAS_CSIM          (1<<0)
#define HWC_HAS_HW            (1<<1)
#define HWC_HAS_HSIM          (1<<2)
#define HWC_HAS_PSIM          (1<<3)
#define HWC_HAS_CSIM_SERVER   (1<<4)

#define HWC_SIM_CSIM        "CSim"
#define HWC_SIM_HW          "Hw  "
#define HWC_SIM_HSIM        "HSim"
#define HWC_SIM_PSIM        "PSim"
#define HWC_SIM_CSIM_SERVER "CSrv"

/*-------------------------------------------------------------------
  Structure describing a PCI configuration register
  -------------------------------------------------------------------*/

typedef struct _HwcPciRegister {
    FxU32              regAddress;   /* offset from start of description */
    FxU32              sizeInBytes;  /* number of bytes in register */
    HwcIOFlag          rwFlag;       /* read only, write only, read/write */
} HwcPciRegister;

/*-------------------------------------------------------------------
  Structure describing a PCI device
  -------------------------------------------------------------------*/

typedef struct {
  HwcPCIState state;          /* Not in use, hw, c sim, h sim */
  FxU16 vendorID;             /* PCI vendor ID */
  FxU16 deviceID;             /* PCI device ID */
#ifdef HWC_CSIM
    FxBool sliDisabled;       /* sli disabled */
    FxI32 sliLog2ChipCount;   /* log2 of number of chips in system */
    FxI32 sliLog2Interleave;  /* Log2 of number of contiguous scanlines being used by surface */
    FxI32 traceChip;          /* which chip to generate streams for */
    FxU32 memSizeInBytes;     /* local memory size (in Bytes) */
    FxU32 agpSizeInBytes;     /* AGP memory size (in Bytes) */
  struct _HwcContext *hwc;
#endif
} HwcDevice;

typedef FxBool HwcHWEnumCallback( const HwcDevice *dev );

/* enumerate standard PCI configuration registers */

#define PCI_VENDOR_ID           0
#define PCI_DEVICE_ID           1
#define PCI_COMMAND             2
#define PCI_STATUS              3
#define PCI_REVISION_ID         4
#define PCI_CLASS_CODE          5
#define PCI_CACHE_LINE_SIZE     6
#define PCI_LATENCY_TIMER       7
#define PCI_HEADER_TYPE         8
#define PCI_BIST                9
#define PCI_BASE_ADDRESS_0     10
#define PCI_BASE_ADDRESS_1     11
#define PCI_BASE_ADDRESS_2     12
#define PCI_BASE_ADDRESS_3     13
#define PCI_BASE_ADDRESS_4     14
#define PCI_BASE_ADDRESS_5     15
#define PCI_BASE_ADDRESS_6     16
#define PCI_BASE_ADDRESS_7     17
#define PCI_BASE_ADDRESS_8     18
#define PCI_INTERRUPT_LINE     19
#define PCI_INTERRUPT_PIN      20
#define PCI_MIN_GNT            21
#define PCI_MAX_LAT            22
#define PCI_NUM_REGISTERS      23

/*-------------------------------------------------------------------
  Enumeration of supported screen refresh rates
  -------------------------------------------------------------------*/

typedef FxI32 HwcScreenRefresh;

#define HWC_REFRESH_60Hz   GR_REFRESH_60Hz   
#define HWC_REFRESH_70Hz   GR_REFRESH_70Hz   
#define HWC_REFRESH_72Hz   GR_REFRESH_72Hz   
#define HWC_REFRESH_75Hz   GR_REFRESH_75Hz   
#define HWC_REFRESH_80Hz   GR_REFRESH_80Hz   
#define HWC_REFRESH_90Hz   GR_REFRESH_90Hz   
#define HWC_REFRESH_100Hz  GR_REFRESH_100Hz  
#define HWC_REFRESH_85Hz   GR_REFRESH_85Hz   
#define HWC_REFRESH_120Hz  GR_REFRESH_120Hz  
#define HWC_REFRESH_NONE   GR_REFRESH_NONE  

/*-------------------------------------------------------------------
  Enumeration of supported screen resolutions
  -------------------------------------------------------------------*/

typedef FxI32 HwcScreenResolution;
#define HWC_RESOLUTION_320x200    GR_RESOLUTION_320x200   
#define HWC_RESOLUTION_320x240    GR_RESOLUTION_320x240   
#define HWC_RESOLUTION_400x256    GR_RESOLUTION_400x256   
#define HWC_RESOLUTION_512x384    GR_RESOLUTION_512x384   
#define HWC_RESOLUTION_640x200    GR_RESOLUTION_640x200   
#define HWC_RESOLUTION_640x350    GR_RESOLUTION_640x350   
#define HWC_RESOLUTION_640x400    GR_RESOLUTION_640x400   
#define HWC_RESOLUTION_640x480    GR_RESOLUTION_640x480   
#define HWC_RESOLUTION_800x600    GR_RESOLUTION_800x600   
#define HWC_RESOLUTION_960x720    GR_RESOLUTION_960x720   
#define HWC_RESOLUTION_856x480    GR_RESOLUTION_856x480   
#define HWC_RESOLUTION_512x256    GR_RESOLUTION_512x256   
#define HWC_RESOLUTION_1024x768   GR_RESOLUTION_1024x768  
#define HWC_RESOLUTION_1280x1024  GR_RESOLUTION_1280x1024 
#define HWC_RESOLUTION_1600x1200  GR_RESOLUTION_1600x1200 
#define HWC_RESOLUTION_400x300    GR_RESOLUTION_400x300   
#define HWC_RESOLUTION_1152x864   GR_RESOLUTION_1152x864  
#define HWC_RESOLUTION_1280x960   GR_RESOLUTION_1280x960  
#define HWC_RESOLUTION_1600x1024  GR_RESOLUTION_1600x1024  
#define HWC_RESOLUTION_1792x1344  GR_RESOLUTION_1792x1344 
#define HWC_RESOLUTION_1856x1392  GR_RESOLUTION_1856x1392 
#define HWC_RESOLUTION_1920x1440  GR_RESOLUTION_1920x1440 
#define HWC_RESOLUTION_2048x1536  GR_RESOLUTION_2048x1536 
#define HWC_RESOLUTION_2048x2048  GR_RESOLUTION_2048x2048
#define HWC_RESOLUTION_320x32     GR_RESOLUTION_320x32
#define HWC_RESOLUTION_NONE       GR_RESOLUTION_NONE 

#ifdef HWC_RESOLUTION_MAX
#undef HWC_RESOLUTION_MAX
#endif
#ifdef HWC_RESOLUTION_MIN
#undef HWC_RESOLUTION_MIN
#endif
#define HWC_RESOLUTION_MIN       GR_RESOLUTION_MIN 
#define HWC_RESOLUTION_MAX       GR_RESOLUTION_MAX 

/*-------------------------------------------------------------------
  Vertex for wireframe rendering
  -------------------------------------------------------------------*/
typedef struct {
    FxFloat x;
    FxFloat y;
} HwcVertex;

/*-------------------------------------------------------------------
  256 color palette tables
  -------------------------------------------------------------------*/
typedef struct {
    unsigned long argb[256];
} HwcPal256;

/*-------------------------------------------------------------------
  translation tables for decompressing NCC format
  -------------------------------------------------------------------*/
typedef struct {
    unsigned char yRGB[16];
    int iRGB[4][3];
    int qRGB[4][3];
} HwcNccTable;

/*-------------------------------------------------------------------
  Enumeration of context states
  -------------------------------------------------------------------*/

typedef FxU32   HwcContextState;
#define HWC_CTX_UNALLOCATED  0
#define HWC_CTX_ALLOCATED    1
#define HWC_CTX_MAPPED       2

/*-------------------------------------------------------------------
  Enumeration of buffers accessed by chip
  -------------------------------------------------------------------*/

typedef FxU32   HwcBufferType;
#define HWC_BUF_3D_FRONT         0
#define HWC_BUF_3D_BACK          1
#define HWC_BUF_3D_AUX           2
#define HWC_BUF_3D_DEPTH         3
#define HWC_BUF_3D_ALPHA         4
#define HWC_BUF_3D_TRIPLE        5 // XXX Chris it would be nice if this was 2

#define HWC_BUF_FIFO             6
#define HWC_BUF_MEM_FIFO         6
#define HWC_BUF_TEXTURE          7
#define HWC_BUF_TEXTURE0         7

/* the next two values are deliberately the same and represent
   the current render buffer
 */
#define HWC_BUF_3D_COLOR         8
#define HWC_BUF_3D_RENDER        8

#define HWC_BUF_2D_STRETCH_SRC   9
#define HWC_BUF_2D_SRC          10
#define HWC_BUF_2D_DST          11
#define HWC_BUF_DESKTOP0        12
#define HWC_BUF_DESKTOP0_LEFT   12
#define HWC_BUF_DESKTOP1        13
#define HWC_BUF_DESKTOP1_LEFT   13
#define HWC_BUF_OVERLAY0        14
#define HWC_BUF_OVERLAY0_LEFT   14
#define HWC_BUF_OVERLAY1        15
#define HWC_BUF_OVERLAY1_LEFT   15
#define HWC_BUF_CURSOR0         16
#define HWC_BUF_CURSOR1         17
#define HWC_BUF_VIDEO0          18
#define HWC_BUF_VIDEO1          19
#define HWC_BUF_YUV             20
#define HWC_BUF_MS              21
#define HWC_BUF_COLOR0          22
#define HWC_BUF_COLOR1          23
#define HWC_BUF_COLOR2          24
#define HWC_BUF_TEXTURE1        25
#define HWC_BUF_DESKTOP0_RIGHT  26
#define HWC_BUF_DESKTOP1_RIGHT  27
#define HWC_BUF_OVERLAY0_RIGHT  28
#define HWC_BUF_OVERLAY1_RIGHT  29
#define HWC_BUF_VIDEOIN         30
#define HWC_BUF_VIDEOIN_ANC0    31
#define HWC_BUF_VIDEOIN_ANC1    32
#define HWC_BUF_AGP_FIFO        33
#define HWC_BUF_GEDBG_FIFO      34
#define HWC_BUF_COUNT           35
#define HWC_BUF_NONE            0xff

/*-------------------------------------------------------------------
  Enumeration of origin locations
  -------------------------------------------------------------------*/

typedef FxU32   HwcOriginLocation;
#define HWC_ORIGIN_UPPER_LEFT  0x0
#define HWC_ORIGIN_LOWER_LEFT  0x1

/*-------------------------------------------------------------------
  Enumeration of color formats
  -------------------------------------------------------------------*/

typedef FxI32   HwcColorFormat;
#define HWC_COLORFORMAT_ARGB   0x0
#define HWC_COLORFORMAT_ABGR   0x1
#define HWC_COLORFORMAT_RGBA   0x2
#define HWC_COLORFORMAT_BGRA   0x3

/*-------------------------------------------------------------------
  Buffer initialization control
  -------------------------------------------------------------------*/

typedef FxU32 HwcBufferInit;
#define HWC_BUFINIT_NORMAL    0x0
#define HWC_BUFINIT_DITHER    0x1
#define HWC_BUFINIT_RANDOM    0x2

/*-------------------------------------------------------------------
  Enumeration of pixel types in buffers accessed by chip
  -------------------------------------------------------------------*/

typedef FxU32 HwcPixelFormat;

#define HWC_PIXFMT_1BPP        0 // 1 Bpp
#define HWC_PIXFMT_P_8         1 // 8BPP Palletted -> RGB
#define HWC_PIXFMT_P_8_6666    2 // 8-bit Palletted to ARGB 6666
#define HWC_PIXFMT_A_8         3 // 8-bit Alpha only
#define HWC_PIXFMT_I_8         4 // 8-bit Intensity
#define HWC_PIXFMT_AI_44       5 // Alpha-Intensity 4-4
#define HWC_PIXFMT_RGB_332     6 // 8-bit RGB 3-3-2
#define HWC_PIXFMT_YUYV        7 // v[31:24] yb[23:16] u[15:8] ya[7:0]
#define HWC_PIXFMT_YUV_422     HWC_PIXFMT_YUYV // XXX LOOOK needs to die
#define HWC_PIXFMT_UYVY        8 // yb[31:24] v[23:16] ya[15:8] u[7:0]
#define HWC_PIXFMT_UYV_422     HWC_PIXFMT_UYVY // XXX LOOOK needs to die
#define HWC_PIXFMT_YUV_411     9
#define HWC_PIXFMT_YIQ_422    10 // NCC Compressed texture

#define HWC_PIXFMT_AP_88      11 // Alpha/Palletted RGB (8-8, expands to 8-8-8-8)
#define HWC_PIXFMT_AI_88      12 // Alpha-Intensity (8-8)
#define HWC_PIXFMT_AYIQ_8422  13 // NCC Compressed texture with 8 bit alpha
#define HWC_PIXFMT_RGB_565    14 // RGB 565
#define HWC_PIXFMT_ARGB_1555  15 // ARGB 1-5-5-5
#define HWC_PIXFMT_ARGB_8332  16 // ARGB 8-3-3-2
#define HWC_PIXFMT_ARGB_4444  17 // ARGB 4-4-4-4
#define HWC_PIXFMT_FBCMP_16   18 // FB Compress Format
#define HWC_PIXFMT_FBCMP_32   19 // FB Compress Format
#define HWC_PIXFMT_TXCMP_4    20 // Texture Compressed Format

#define HWC_PIXFMT_RGB_888    21 // 24-bit per pixel RGB
#define HWC_PIXFMT_ARGB_8888  22 // 32-bit per pixel ARGB
#define HWC_PIXFMT_AA_16      23 // AA 16bpp (multisample) format color 4*RGB565, aux 4*X8A8 or 4*D16
#define HWC_PIXFMT_AA_32      24 // AA 32bpp (multisample) format color 4*ARGB8888, aux 4*S8D24
#define HWC_PIXFMT_AP_44      25 // Alpha/Palletted RGB 4-4 DVD Subpicture info
#define HWC_PIXFMT_AYUV_32    26 // A Y U V, 8 bits per. A is [31:24]
#define HWC_PIXFMT_AA_1555    27 // AA 1555 format (multisample) format color 4*ARGB555, aux 4*X8A8 or 4*D16
#define HWC_PIXFMT_VIP_ANC    28 // VideoIn Ancillary Data (8bits)
#define HWC_NUM_PIXEL_FORMATS 29

// When adding a hwc pixel format, remember to edit these functions:
//    hwcFormatToBPP() in hwc\buffers.c
//    hwcPixelFormatToString() in hwc\buffers.c
//    hwcInitPixelFormats() in hwc\fxhwc.c
// If it's a format recognized by video:
//    vlFormatHwc2Vid() in diags\video\vidlib.c
//    vlFormatVid2Hwc() in diags\video\vidlib.c
//    vlDumpHwcBuffer() in diags\video\vidlib.c
//    new define in diags\video\vd_streams.h // sync up with verilog copy


typedef FxU32 HwcPixel;

// YUYV 422
// UYUV 422
// YUV 420 planar  
// YUV 444

/*-------------------------------------------------------------------
   Resources a test needs in order to be able to run
  -------------------------------------------------------------------*/

typedef FxU32 HwcTestType;
#define HWC_TEST_NUM_3D_BUF(_x)  ((_x)&0x3)
#define HWC_TEST_3D_SINGLE_BUF   BIT(0)
#define HWC_TEST_3D_DOUBLE_BUF   BIT(1)
#define HWC_TEST_3D_TRIPLE_BUF   0x3
#define HWC_TEST_3D_DEPTH        BIT(2)
#define HWC_TEST_3D_AUX          BIT(2)
#define HWC_TEST_3D_STENCIL      BIT(3)
#define HWC_TEST_3D_MULTI_SAMPLE BIT(4)
#define HWC_TEST_2D_DST_BUF      BIT(5)
#define HWC_TEST_2D_SRC_BUF      BIT(6)
#define HWC_TEST_STEREO          BIT(7)
#define HWC_TEST_VIDEO0          BIT(8) 
#define HWC_TEST_VIDEO1          BIT(9) 
#define HWC_TEST_FIFO            BIT(10) 
#define HWC_TEST_LFB             BIT(11) 
#define HWC_TEST_2D_HOST_BUF     BIT(12) 
#define HWC_MODULE_TEST          BIT(31)

#define HWC_TEST_3D     (HWC_TEST_LFB|HWC_TEST_FIFO|HWC_TEST_3D_DOUBLE_BUF|HWC_TEST_3D_DEPTH)
#define HWC_TEST_2D     (HWC_TEST_2D_DST_BUF|HWC_TEST_2D_SRC_BUF)

/*-------------------------------------------------------------------
  Enumeration of supported vector traces (mostly in device dependant 
  HWC file (e.g. sst2\include\sst2hwc.h)
  -------------------------------------------------------------------*/

#define HWC_VEC_GLOBAL            0
#define HWC_VEC_IO                1
#define HWC_VEC_MEM_SGRAM         2
#define HWC_VEC_MEM_AGP           3
#define HWC_VEC_CONTINUE          BIT(31)
#define HWC_VEC_ID_MASK           (HWC_VEC_CONTINUE-1)

typedef FxU32 HwcVectorControl;
#define HWC_VECCTL_ENABLE_CAPTURE       BIT(0)
#define HWC_VECCTL_ONE_FILE             BIT(1)

/*-------------------------------------------------------------------
  Enumeration of memory transcations
  -------------------------------------------------------------------*/

typedef FxU32 HwcIoTransaction;
#define HWC_IO_READ   0
#define HWC_IO_WRITE  1
#define HWC_MEM_READ  2
#define HWC_MEM_WRITE 3
#define HWC_CFG_READ  4
#define HWC_CFG_WRITE 5
#define HWC_AGP_READ  6
#define HWC_AGP_WRITE 7
#define HWC_READ_TRANSACTION(_t)  (((_t) & 0x1 ) == 0 )
#define HWC_WRITE_TRANSACTION(_t) (((_t) & 0x1 ) != 0 )

typedef struct {
  HwcIoTransaction transaction;
  FxU32  size;
  FxU32  addr;
  FxU32  data;
} HwcIoVec;

typedef struct {
   FxU32 tValid; // time in pico seconds when the data is valid at the upstream unit.
   FxU32 tTaken; // time in pico seconds when THIS unit took the data.  
   void *pPSimPrivate; // pointer to psim private data structure.
} HwcTransInfo;

#ifdef HWC_CSIM

/* HSIM flags, passed in from the diag environment */
#define HSIM_HW_SIMULATION              BIT(0)
#define HSIM_TREX_STANDALONE            BIT(1)
#define HSIM_TREX_BACKDOOR_TEXWRITES    BIT(2)

typedef struct {
  FxI32 left;
  FxI32 top;
  FxI32 right;
  FxI32 bottom;
} HwcRect;

typedef struct {
    FxU32 minx, miny;
    FxU32 maxx, maxy;
} HwcPixelLimits; 

#define HWC_ALLOCATE_LINEAR  0
#define HWC_ALLOCATE_RLINEAR 1
#define HWC_ALLOCATE_RANDOM  2

typedef struct HwcMemRegion_s {
    struct HwcMemRegion_s *next;
    struct HwcMemRegion_s *children;
    FxU32 baseAddress;
    FxU32 sizeInBytes;
    FxU32 memType;
    FxU32 allocMethod;
} HwcMemRegion ;

/*-------------------------------------------------------------------
  Hints defining buffer characteristics
  -------------------------------------------------------------------*/

typedef FxU32 HwcBufferHints;
#define HWC_BUFFER_3D                 BIT(0)   // buffer wil be used for 3d rendering
#define HWC_BUFFER_AUX                BIT(1)   // buffer will be used as an aux buffer
#define HWC_BUFFER_STAGGERED          BIT(2)   // buffer uses staggered alignment
#define HWC_BUFFER_TILED              BIT(3)   // buffer is tiled
#define HWC_BUFFER_HOST_BYTE_SWIZZLE  BIT(4)   // Host port byte swizzle (1=enable)
#define HWC_BUFFER_HOST_WORD_SWIZZLE  BIT(5)   // Host port word swizzle (1=enable)

#define HWC_BUFFER_MEM_LOCAL          0        // buffer is in local (video) memory
#define HWC_BUFFER_MEM_AGP            1        // buffer is in AGP
#define HWC_BUFFER_MEM_SYSTEM         2        // buffer is in system memory

#define HWC_BUFFER_MEM_SHIFT          6
#define HWC_BUFFER_MEM                (SST_MASK(2)<<HWC_BUFFER_MEM_SHIFT)

// scanline packing is performed on linear surfaces only
#   define HWC_BUFFER_PACK_SRC        0x0UL    // alignment depends on pixel format (8, 16, 32)
#   define HWC_BUFFER_PACK_8          0x1UL    // stride aligned on byte boundary
#   define HWC_BUFFER_PACK_16         0x2UL    // stride aligned on 16 bit boundary
#   define HWC_BUFFER_PACK_32         0x3UL    // stride aligned on 32 bit boundary

#define HWC_BUFFER_PACK_SHIFT         8
#define HWC_BUFFER_PACK               (SST_MASK(2)<<HWC_BUFFER_PACK_SHIFT)

#define HWC_BUFFER_TILE_MODE_SHIFT   10
#define HWC_BUFFER_TILE_MODE         (SST_MASK(2)<<HWC_BUFFER_TILE_MODE_SHIFT)

#define HWC_BUFFER_INTERLEAVED        BIT(12)  // buffer is scan line interleaved
#define HWC_BUFFER_PURE_LINEAR        BIT(13)  // buffer is a linear memory segment
#define HWC_BUFFER_BLOCK_LINEAR       BIT(14)  // buffer is a block linear memory segment
#define HWC_BUFFER_BASEADDR           BIT(15)  // buffer position specified by client
// the following attribute affect csim execution only and don't relate to hardware
#define HWC_BUFFER_UPDATED            BIT(26)  // buffer has been auto updated
#define HWC_BUFFER_WIREFRAME          BIT(27)  // use wireframe rendering mode
#define HWC_BUFFER_NO_ALLOC           BIT(28)  // initialize but don't allocate buffer
#define HWC_BUFFER_VISIBLE            BIT(29)  // display buffer in window (sim only)
#define HWC_BUFFER_SHOW_UPDATES       BIT(30)  // reflect buffer updates in gui window
#define HWC_BUFFER_AUTO_UPDATE        BIT(31)  // auto update buffers data (sim only)

typedef FxU32 HwcTextureAllocation;
#define HWC_TEXTURE_LINEAR        0
#define HWC_TEXTURE_TILED         1
#define HWC_TEXTURE_BLOCK_LINEAR  2

/*-------------------------------------------------------------------
  Structure describing buffer (surface)
  -------------------------------------------------------------------*/
typedef struct _HwcBuffer {
    struct _HwcBuffer *next;      /* next buffer for this device */
    char           *name;         /* buffer name */
    struct _HwcContext     *hwc;  /* pointer to device I belong to */
    HwcBufferType  type;          /* type of buffer */
    HwcPixelFormat format;        /* type of pixels in buffer */
    FxU32          width;         /* width of buffer */
    FxU32          height;        /* height of buffer */
    FxU32          bpp;           /* bits per pixel */
    FxU32          hints;         /* hints for surface attributes */
    void          *palette;       /* palette to use to display buffer */
    FxU32          srcPack;       /* Source packing 0, 1=>8, 2=>16, 3=>32 */
    FxU32          log2NumChips;  /* Log2 of number of chips being used by surface */
    FxU32          log2Interleave;/* Log2 of number of contiguous scanlines being used by surface */
    HwcMemRegion   reg;           /* memory region occupied by buffer (LOOOK duplicates info) */

    /* device dependant information */

    FxU32          physicalBase;  /* start of surface in physical memory */
    FxU32          physicalEnd;   /* end of allocated physical memory */
    FxU32          sizeInBytes;   /* the extent in physical memory of this buffer */
    FxU32          alignment;     /* required memory alignment */
    FxU32          stride;        /* device/address mode dependant */
    FxU32          pitch;         /* stride of surface in linear space */
    FxU32          state;         /* device dependant */
    FxBool         agp;           /* surface is in AGP memory */
    void          *ddHW;          /* device dependent data */
    void          *hDC;                   
    void          *hPen;                   
    void          *hBitmap;                   
    FxBool      (* Delete)(struct _HwcBuffer *); /* delete buffer */
    FxBool      (* AutoUpdate)(struct _HwcBuffer *, FxU32 msg); /* callback for auto updating */
} HwcBuffer;

typedef struct {
    FxU32 fbHostLinearBase;
    FxU32 fbHostWidth;
    FxU32 fbHostHeight;
    FxU32 fbHostPitch;
} HwcFBHostBuffer;

typedef struct _HwcBufferSpec {
    void           *nextXXX;      /* next buffer for this device */
    void           *nameXXX;      /* buffer name */
    void           *hwcXXX;       /* pointer to device I belong to */
    HwcBufferType  type;          /* type of buffer */
    HwcPixelFormat format;        /* type of pixels in buffer */
    FxU32          width;         /* width of buffer */
    FxU32          height;        /* height of buffer */
    FxU32          bpp;           /* bits per pixel */
    FxU32          hints;         /* hints for surface attributes */
    void          *paletteXXX;    /* palette to use to display buffer */
    FxU32          srcPackXXX;    /* Source packing 0, 1=>8, 2=>16, 3=>32 */
    FxU32          log2NumChipsXXX;/* Log2 of number of chips being used by surface */
    FxU32          log2InterleaveXXX;/* Log2 of number of contiguous scanlines being used by surface */
    HwcMemRegion   regXXX;        /* memory region occupied by buffer (LOOOK duplicates info) */

    /* device dependant information */

    FxU32          physicalBase;  /* start of surface in physical memory */
    FxU32          physicalEndXXX;   /* end of allocated physical memory */
    FxU32          sizeInBytesXXX;   /* the extent in physical memory of this buffer */
    FxU32          alignmentXXX;     /* required memory alignment */
    FxU32          stride;        /* device/address mode dependant */
    FxU32          pitch;         /* stride of surface in linear space */
    FxU32          state;         /* device dependant */
} HwcBufferSpec;

#define BUFF2SPEC(spec, buff)   \
    (spec)->type = (buff)->type;        \
    (spec)->format = (buff)->format;        \
    (spec)->width = (buff)->width;        \
    (spec)->height = (buff)->height;        \
    (spec)->bpp = (buff)->bpp;        \
    (spec)->hints = (buff)->hints;        \
    (spec)->physicalBase = (buff)->physicalBase;        \
    (spec)->stride = (buff)->stride;        \
    (spec)->pitch = (buff)->pitch;        \
    (spec)->state = (buff)->state;

/*-------------------------------------------------------------------
  Structure describing window to view chip info
  -------------------------------------------------------------------*/
typedef struct _HwcWindow {
    struct _HwcWindow *next;              /* pointer to next window */
    FxBool stdBuffer;             /* is buffer standard */
    HwcBufferType which;          /* which std buffer */
    struct _HwcSimulator *hws;            /* pointer to simulator I'm displaying */
    HwcBuffer *buffer;            /* buffer to display */
    void *hDC;                   
    void *hWnd;
    void *hWndDisplaySettings;    /* the display settings dialog box */
    FxU32 viewID;
    void *photoHandle;
    int videoFilter;              /* video filtering is enabled */
    int zoom;                     /* zoom factor */
    char hex;                     /* hex (or dec) display */
    FxU32 width, height;          /* width and height of visible area */
    char statusBar;               /* status bar is visible */
    int mx, my;                   /* last mouse position */
    FxU32 xOffset, yOffset;       /* scrolling offsets */
    char fileSaveName[256];       /* file save name */
    FxBool clientWindow;          /* window belongs to client */
    void *clientWndProc;          /* client windows procedure */
    FxBool (* SetBuffer)(struct _HwcWindow *pWnd, HwcBuffer *); /* change windows buffer */
    FxBool (* Delete)(struct _HwcWindow *pWnd); /* delete window */
} HwcWindow;

/* GUI API */

#define HWC_ALIGN_BITS SST_MASK(5)
#define HWC_ALIGN_MODE_SHIFT 6
#define HWC_ALIGN_ANY  (0 << 6 )
#define HWC_ALIGN_ODD  (1 << 6 )
#define HWC_ALIGN_EVEN (2 << 6 )

/*-------------------------------------------------------------------
  Enumeration of supported simulation modes
  -------------------------------------------------------------------*/
typedef FxU32 HwcSimMode;
#define HWC_SIM_NONE         0
#define HWC_SIM_NO_EXECUTE   1
#define HWC_SIM_FUNCTIONAL   2
#define HWC_SIM_BIT_ACCURATE 3
#define HWC_SIM_NO_RENDER    4

/*-------------------------------------------------------------------
  Structure describing a SSTX memory mapped register
  -------------------------------------------------------------------*/

#define HWC_REG_TYPE_MASK 0xffff
#define HWC_REG_ATTR_MASK 0xffff0000
#define HWC_REG_ATTR_SHIFT 16

#define HWC_REG_FMT_BITFIELD           0
#define HWC_REG_FMT_FLOAT              1
#define HWC_REG_FMT_12_4               2
#define HWC_REG_FMT_VERTEX             3
#define HWC_REG_FMT_FLOAT_VERTEX       4
#define HWC_REG_FMT_13_4               5
#define HWC_REG_FMT_12_12              6
#define HWC_REG_FMT_STRUCT             7

#define HWC_REG_READABLE               (1 << HWC_REG_ATTR_SHIFT)
#define HWC_REG_WRITEABLE              (2 << HWC_REG_ATTR_SHIFT)
#define HWC_REG_PER_TMU                (4 << HWC_REG_ATTR_SHIFT)
#define HWC_REG_RESERVED               (8 << HWC_REG_ATTR_SHIFT)

typedef FxU32 HwcAddressMap;
#define HWC_ADDR_CONFIG 0
#define HWC_ADDR_IO     1
#define HWC_ADDR_MEM    2
#define HWC_ADDR_AGP    3

/* These defines are used in the address field of sst2FieldValue[].
** They must not interfere with true addresses.  Therefore, make sure
** that (bogus_address % 4 != 0).
*/
#define HWC_FIELD_COMMENT 1
#define HWC_FIELD_NULLADDR 2
#define HWC_FIELD_ENDSECTION 0xffffffff

typedef struct {
    char *abbrName;				    /* Abbreviated name */
    char *longName;				    /* Full Value name  */
} HwcFieldType_ValueType;

typedef struct {
    FxU32 addr;                     /* Address of Register */
    char *abbrName;				    /* Abbreviated Field name */
    char *longName;				    /* Full Field name */
    int size;		                /* Size of field */
    int offset;						/* Offset of field */
    HwcFieldType_ValueType *value;  /* Descriptions of Values */
} HwcFieldType;

typedef struct {
    char *name;
    FxU32 addr;
    FxU32 size;
    FxU32 mask;
    FxU32 attr;
    char *description;
} HwcCfgAddrDesc;

typedef struct {
    char *name;                 /* register name (for printing) */
    FxU32 attr;                 /* register attributes */
    FxU32 mask;                 /* bitmask to reduce to proper size */
    FxU32 fieldDesc;            /* offset for field description(s), if any */
} HwcRegInfo;

/*-------------------------------------------------------------------
  Structure describing a range of SSTX registers (e.g. 2d registers,
  3d registers, IO registers, command registers etc.)
  -------------------------------------------------------------------*/

typedef struct {
    char       *name;             /* name of register range */
    FxU32       offset;           /* offset from address base */
    FxU32       size;             /* size of register range (in bytes) */
    HwcRegInfo *info;             /* information on each register */
} HwcRegRange;

/*-------------------------------------------------------------------
  Structure describing the register ranges pointed to by a PCI base
  address register.
  -------------------------------------------------------------------*/

typedef struct {
    FxU32       *nRanges;         /* number of ranges in map */
    HwcRegRange *range;           /* range information */
} HwcAddrMap;

#endif

typedef struct {
    FxU32  framesDrawn;         /* # of frames drawn */
    FxU32  bufferSwaps;         /* # of buffer swaps */
    FxU32  pointsDrawn;         /* # points drawn, XXX current hw does not draw points */
    FxU32  linesDrawn;          /* # lines drawn */

    FxU32  texDownloads;        /* # of texDownload calls */
    FxU32  texBytes;            /* # of texture bytes downloaded */

    FxU32  nccDownloads;        /* # of NCC palette download calls */
    FxU32  nccBytes;            /* # of NCC palette bytes downloaded */
    FxU32  palDownloads;        /* # of palette download calls */
    FxU32  palBytes;            /* # of palette bytes downloaded */
    FxU32  pixelsOut2D;         /* # 2d pixels drawn (including buffer clears) */
    FxU32  pixelsIn;            /* # pixels processed (minus buffer clears) */
    FxU32  chromaFail;          /* # pixels not drawn due to chroma key */
    FxU32  zFuncFail;           /* # pixels not drawn due to Z comparison */
    FxU32  aFuncFail;           /* # pixels not drawn due to alpha comparison */
    FxU32  pixelsOut;           /* # pixels drawn (including buffer clears) */
    FxU32  trisProcessed;       /* # triangles sent to hardware */
    FxU32  trisDrawn;           /* # triangles drawn */
    FxU32  trisDrawnPerFrame;   /* # triangles drawn from start of frame */
    FxU32  stencilFail;         /* # pixels not drawn due to stencil test */
    FxU32  stencilOp;           /* TBD */

    /* simulator statistics only valid if simulator enabled */

    FxU32 pGroupsDrawn;         /* # pixel groups drawn */
    FxU32 pGroupsDrawnPerTri;   /* # pixel groups drawn from start of triangle */
    FxU32 pGroupsDrawnPerFrame; /* # pixel groups drawn from start of frame */
    FxU32 raBlockCount;         /* # spans */
    FxU32 raEmptyBlocks;        /* # number of empty blocks processed */
    FxU32 raPixelsIn;           /* # pixels sent down pipe */
    FxU32 raPixelsOut;          /* ?? */
} HwcStats;

// events supported by hwcGUIChange
typedef FxU32 HwcChangeEvent;
#define HWC_GUI_END_FRAME       0
#define HWC_GUI_BUFFER_SWAP     1
#define HWC_GUI_POINTS_DRAWN    2
#define HWC_GUI_LINES_DRAWN     3
#define HWC_GUI_TEX_DOWNLOADS   4
#define HWC_GUI_TEX_BYTES       5
#define HWC_GUI_NCC_DOWNLOADS   6
#define HWC_GUI_NCC_BYTES       7
#define HWC_GUI_PAL_DOWNLOADS   8
#define HWC_GUI_PAL_BYTES       9
#define HWC_GUI_PIXELS_OUT_2D  10
#define HWC_GUI_PIXELS_IN      11
#define HWC_GUI_CHROMA_FAIL    12
#define HWC_GUI_ZFUNC_FAIL     13
#define HWC_GUI_AFUNC_FAIL     14
#define HWC_GUI_PIXELS_OUT     15
#define HWC_GUI_TRIS_PROCESSED 16
#define HWC_GUI_TRIS_DRAWN     17
#define HWC_GUI_STENCIL_FAIL   18
#define HWC_GUI_STENCIL_OP     19

/* 
   callback to allow the HWC code to use the client 
   command fifo management code to do writes etc.
 */

typedef void (* HwcWriteCallback)( volatile FxU32 * const linaddr,
                                   const FxU32 val );

/* Define a new device type */

typedef struct _HwcDeviceType {
    char *name;
    HwcDeviceClass devClass;
    FxU16 vendorID;
    FxU16 deviceID;
    FxBool (* InitBoard) (struct _HwcContext *hwc);
} HwcDeviceType;

// module element types
#define HWC_MODULE        0
#define HWC_INPUT_STREAM  1
#define HWC_OUTPUT_STREAM 2
#define HWC_MEMORY_STREAM 3

// stream element types
#define HWC_U_PARAM 1
#define HWC_I_PARAM 2
#define HWC_S_PARAM 3
#define HWC_PARAM_TYPE(_p) (_p&0xffff)
#define HWC_NEWLINE (1UL<<31)
#define HWC_SPACES  (1UL<<30)

typedef struct {
    char *name;
    FxU32 type; // module, input stream, output stream
    char *description;
} HwcModuleElement;

typedef struct {
    char *name;
    FxU32 type; // unsigned, unsigned, signed or structure
    FxU32 size; // size in bits of field or pointer to stream definition must be < 64
    FxU32 count; // number of elements of this type
    char *description;
    FxU32 detail;
    char *fileName;
    FxBool generate;
    FxI32  breakPoint;         /* break on vector */
} HwcStreamElement;

// WARNING the following two structures HwcModuleInfo and HwcFCStreamInfo are also defined
// in sst2/src/common/pli/sv_common.h any changes made to these structures MUST also be
// made there.

typedef struct _HwcModuleInfo {
    char   *name;
    FxBool enabled;
    void   *stream;               /* stream to output traces to */
    FxU32  detail;                /* level of detail we are interested in */
    FxU32  nextVector;            /* next trace number */
    FxI32  breakPoint;            /* break on vector */
} HwcModuleInfo;

typedef struct {
  FxU32 maxStreams;
  HwcModuleInfo *stream;
} HwcFCStreamInfo;         

typedef void HwcStream;

FX_ENTRY void FX_CALL hwcWriteVStruct(struct _HwcSimulator *hws, FxU32 id, void *p);

typedef FxI32 HwcOptionFunc;
#define HWC_OPT_PICK   0 // determine option value
#define HWC_OPT_SHOW   1 // list option choices

#define HWC_OPT_NONE    0
#define HWC_OPT_BOOL    1
#define HWC_OPT_UINT16  2
#define HWC_OPT_UINT32  3
#define HWC_OPT_INT16   4
#define HWC_OPT_INT32   5
#define HWC_OPT_STRING  6
#define HWC_OPT_FLOAT   7
#define HWC_OPT_DOUBLE  8
#define HWC_OPT_SLIST   9
#define HWC_OPT_SVLIST 10
#define HWC_OPT_TEXFMT 11
#define HWC_OPT_END    -1

typedef struct {
    char *name;
    FxI32 value;
} HwcStringValue;

typedef struct _HwcTestOption {
    char *key;
    char *envKey;
    FxI32 type;
    void *var;
    FxBool (* parseArg)(HwcOptionFunc func, struct _HwcTestOption *opt, char *argValue);
    char *desc;
    void *options;
} HwcTestOption;

typedef struct {
  char *name;                   /* name of test */
  char *version;                /* version of test */
  char *author;                 /* author of test */
  HwcTestType type;             /* test type */
  char *description;            /* description of test */
  char *fileName;               /* default filename for test vectors */
  HwcVectorControl ctrl;        /* test vector control */
  FxU32 detail;                 /* default detail reporting level */
  HwcTestOption *testOptions;   /* test specific options */
} HwcTestDescription;

typedef struct _HwcSimulator {
    struct _HwcContext *hwc;            /* the context which I'm simulating */
    HwcStats stats;             /* performance statistics */
    char *name;                 /* name of simulator */
    char *version;              /* version of simulator */
#ifdef HWC_CSIM
    struct _HwcSimulator *next;       /* next simulator in chain */
    FxU32 simID;
    volatile void *simMem;            /* sim memory */
    FxU8 *agpMem;                     /* AGP true base address */
    FxBool videoSave;                 /* 1 entry stack */
    FxU32 lastRead;	                  /* holds the last data read */
    FxU32 hWndCount;                  /* count of CSIM open windows XXX do we need this */
    void *hwndCsimMainControl;        /* XXX is this per device */
    HwcWindow *mainWindow;            /* the main control window */
    HwcWindow *windows;               /* linked list of all windows */
    FxBool fpuSave;                   /* save/restore floating point on entry */
    HwcSimMode csimMode;              /* none, no execute, functional */
    FxU32 simVirtAddr[HWC_NUM_BASE_ADDR]; /* virtual memory base address */
    FxU32 simPhysAddr[HWC_NUM_BASE_ADDR]; /* physical memory base address */
    FxU32 simMemLength[HWC_NUM_BASE_ADDR];/* lengths of memory segments */
    struct {                          /* environmental controls */
        int paused;                   /* Go or Break? */
        int pauseAfterNextSwap;       /* break after next swap */
        int pauseAfterNextTriangle;   /* break after next triangle */
        int flushOnCommands;          /* flush dirty pixels after each command */
        int saveAfterSwap;            /* save image file after all swaps */
        FxU32 bitAccurate;            /* bit accurate rendering flag */
        FxI32 flushCount;             /* flush count */
        FxI32 curFlushCount;          /* current flush counter */
        int resetPixelsIn;            /* reset hwStats.pixelIn every frame */
        int skipRendering;            /* go fast mode (real fast) */
        FxBool valid;                 /* has any of the dynamic text change */
        FxU32 pauseAfterSwapNum;
        FxU32 pauseAfterTriangleNum;
        FxBool multiZ;                /* enable multiZ subsamples     */
        FxBool ieeeMath;              /* use IEEE math  */
        FxI32 rasterAlg;              /* raster algorithm for tris */
        FxBool videoEnabled;          /* is video enabled */
        FxBool triBoundsCheck;        /* check triangle bounds        */
        FxI8 fastFill;                /* optimize fast fills */
        double loderr;
        FxU32 statsVerboseLevel;      /* statistcs control */
        FxU32 fbCompress;             /* buffer compresseion control */
	    FxU32 fbCompressTrace;        /* concatenated x,y for tracing compression */
        HwcPixelLimits pixelTrace;    /* pixel trace limits */ 
        FxU32 ditherSub;              /* force dither subtract off */ 
        FxBool dither2x2;             /* force dither to 2x2 matrix */ 
        FxBool forceLinearAddressing; /* don't use tiled addrssing */
        FxI32 rasterCheck;            /* check triangle rasterization */
        FxBool disableYBanding;       /* disable Y banding in the raster */
        FxBool disableColumnBanding;  /* disable column banding in the raster */
        FxU32  sliConfig;             /* width of column for banding */
        FxBool useClientWindows;      /* use client windows for display */
        FxU32 memWidth;               /* memory controller width */
        FxBool raVis;                 /* enable raster visualization */
        FxBool forceHWOrder;          /* force csim to execute in hardware order */
        FxBool checkTextureData;      /* downloaded texture data is forced to self address */
        FxU32 numVertSyncs;           /* --nsync number of vert blanks to wait for */
    } environment;

    struct {
        HwcTestType type;          /* test type */
        FxBool initialized;
        FxBool useHSIM;
        HwcVectorControl control;  /* vector control flags */
        FxU32  numModules;         /* how many modules do we have */
        HwcModuleInfo *module;     /* per module info */
        FxU32  nextVector;         /* next trace number */
    } vecInfo;
#endif

    FxBool (* GetConfigData)(struct _HwcSimulator *hws, HwcPciRegister reg, FxU32 *data );
    FxBool (* SetConfigData)(struct _HwcSimulator *hws, HwcPciRegister reg, FxU32  data );
    FxBool (* IOStore8)( struct _HwcSimulator *hws, FxU16 addr, FxU8  data );
    FxBool (* IOStore16)( struct _HwcSimulator *hws, FxU16 addr, FxU16 data );
    FxBool (* IOStore32)( struct _HwcSimulator *hws, FxU16 addr, FxU32 data );

    FxU8   (* IOLoad8)( struct _HwcSimulator *hws, FxU16 addr );
    FxU16  (* IOLoad16)( struct _HwcSimulator *hws, FxU16 addr );
    FxU32  (* IOLoad32)( struct _HwcSimulator *hws, FxU16 addr );

    void   (* Store8)( struct _HwcSimulator *hws, volatile void * addr, FxU8  data );
    void   (* Store16)( struct _HwcSimulator *hws, volatile void * addr, FxU16 data );
    void   (* Store32)( struct _HwcSimulator *hws, volatile void * addr, FxU32 data );

    FxU8   (* Load8)( struct _HwcSimulator *hws, volatile void * addr );
    FxU16  (* Load16)( struct _HwcSimulator *hws, volatile void * addr );
    FxU32  (* Load32)( struct _HwcSimulator *hws, volatile void * addr );
    FxBool (* Map)( struct _HwcSimulator *hws);
    FxBool (* Stall)( struct _HwcSimulator *hws, FxU32 nClocks);
    FxBool (* Unmap)( struct _HwcSimulator *hws);
    FxU32  (* Shutdown)( struct _HwcSimulator *hws);
    FxBool (* VideoEnable)( struct _HwcSimulator *hws, FxBool enable );
    FxBool (* VideoUpdate)( struct _HwcSimulator *hws);
    HwcPixel (* ReadPixel)(struct _HwcSimulator *hws, HwcBufferSpec *pBuff, int x, int y);
    HwcPixel (* ReadColor)(struct _HwcSimulator *hws, HwcBufferSpec *pBuff, int x, int y);
    FxBool (* WritePixel)(struct _HwcSimulator *hws, HwcBufferSpec *pBuff, int x, int y, HwcPixel pix);
    void   (* GUIUpdateRect)(struct _HwcSimulator *hws, int x, int y, int w, int h, FxU32 addr, FxU32 memType, HwcBufferType which);
    void   (* GUIKeepAlive)(struct _HwcSimulator *hws, int count);
    void   (* GUIChange )(struct _HwcSimulator *hws, HwcChangeEvent which, FxI32 count);
    FxU32  (* ReadMemory )(struct _HwcSimulator *hws, FxU32 addr, FxU32 size, FxU32 memType, FxU32 chipID);
    void   (* WriteMemory)(struct _HwcSimulator *hws, FxU32 addr, FxU32 size, FxU32 memType, FxU32 mask, FxU32 data);
    HwcTransInfo * (* GetTransInfo)(struct _HwcSimulator *hws);
    void   (* BufferInit)(struct _HwcSimulator * hws, HwcBufferSpec *pBuff, FxI32 xmin, FxI32 ymin, FxI32 w, FxI32 h,
				          FxU32 pix, HwcBufferInit ctl);
    void   (* AddViewWindow)(struct _HwcSimulator* hws, HwcWindow* cw);
    FxBool (* ReadAGPMemory)(struct _HwcSimulator *hws, FxU32 addr, FxU32 *data);
    FxBool (* WriteAGPMemory)(struct _HwcSimulator *hws, FxU32 addr, FxU32 data);
} HwcSimulator;

typedef struct {
    FxU32 bitsPerTexel;    // number of bits per texel
    FxU32 minWidth;        // width of smallest meaningful block
    FxU32 minHeight;       // height of smallest meaningfull block
    FxU32 log2BlockWidth;  // log2 of block width in block linear mode
    FxU32 log2BlockHeight; // log2 of block height in block linear mode
} HwcTexelInfo;

typedef struct {
    FxU32 agpStrap;
    FxU32 vendorID;
    FxU32 deviceID;
    FxU32 command;
    FxU32 status;
    FxU32 revision;
    FxU32 classCode;
    FxU32 cacheLinesize;
    FxU32 latencyTimer;
    FxU32 headerType;
    FxU32 bist;
    FxU32 subVendorID;
    FxU32 subSyatemID;
    FxU32 romBaseAddr;
    FxU32 capPtr;
    FxU32 interuptLine;
    FxU32 interuptPin;
    FxU32 minimumGrantTime;
    FxU32 maxLatencyTime;
    FxU32 fabID;
    FxU32 acpiReset;
    FxU32 cfgStatus;
    FxU32 cfgScratch;
    FxU32 capID;
    FxU32 capIdNext;
    FxU32 agpRev;
    FxU32 agpStatus;
    FxU32 agpCmd;
    FxU32 acpiCapId;
    FxU32 acpiCapIdNext;
    FxU32 acpiCapIdPmc;
    FxU32 acpiStatus;
    FxU32 pmcsrBse;
    FxU32 cfgData;
} HwcConfigData;

typedef enum _HwcIfaceType {
    HwcAgpIface = 0,
} HwcIfaceType;

typedef struct _HwcContext {   /* Hardware Context Structure */
    FxU32 size;                /* size of this structure */
    FxU32 bn;                  /* index into list of boards*/
    HwcContextState state;     /* allocated, mapped ? */
    HwcDeviceType *devType;    /* device type */
    HwcConfigData config;
    HwcSimulator *hws;         /* list of simulators attached to this context */
    HwcSimulator *streamSim;   /* the simulator we are generating streams for */
    struct _HwcInfo *hwcInfo;   /* back pointer to hwc info */
    FxU32 devNum;              /* PCI device number */
    HwcDevice *dev;            /* device info */
    FxU32 fbiRevision;         /* FBI revision number */
    FxU32 fbiVideoWidth;       /* video display X-resolution */
    FxU32 fbiVideoHeight;      /* video display Y-resolution */
    FxU32 fbiVideoRefresh;     /* video refresh rate */
    FxU32 fbiConfig;           /* FBI strapping pins */
    FxU32 fbiMemoryFifoEn;     /* FBI memory fifo enabled */
    FxU32 fbiMemType;          /* FBI memory type (poweron strapping bits) */
    FxU32 memSizeInBytes;      /* sim memory size (in Bytes) */
    FxU32 agpSizeInBytes;      /* AGP memory size (in Bytes) */
    HwcMemRegion fbMemReg;     /* fb memory region */
    HwcTexelInfo *texelInfo;   /* info describing supported textures */
    volatile void *virtAddr[HWC_NUM_BASE_ADDR];/* virtual memory base address */
    FxU32 physAddr[HWC_NUM_BASE_ADDR];         /* physical memory base address */
    FxU32 memLength[HWC_NUM_BASE_ADDR];        /* lengths of memory segments */
    FxU16 virtPort;            /* virtual i/o port base address */
    FxU16 physPort;            /* physical i/o port base address */
    void *devInfo;             /* board specific info */
    FxU32 hwVirtAddr[HWC_NUM_BASE_ADDR];       /* hw virtual address of memory regions */
    HwcBuffer *buffers;         /* list of buffers allocated on board */
    HwcBuffer *stdBuffers[HWC_BUF_COUNT]; /* standard buffers */
    HwcBuffer  *videoBuff;      /* video buffer */
    HwcBuffer   msBuffer;       /* ms buffer */
    int memEnabled;             /* local memory access enabled */
    int agpEnabled;             /* AGP memory access enabled */
    int ioEnabled;              /* i/o port access enabled */
    FxU32 iDevice;              /* interface for csim server library */
    FxU32 devNode;              /* for simulated device */
    struct {
        FxFloat r;
        FxFloat g;
        FxFloat b;
        FxU32 tableR[256];
        FxU32 tableG[256];
        FxU32 tableB[256];
    } gamma;
    HwcFieldType *fieldDefs;    /* field definitions */
    void *agpIface;             /* opaque pointer to a IAGPMemory interface */

    /* Callbacks */

    FxBool (* InitRegisters)(struct _HwcContext *hwc);
    FxBool (* InitRenderingRegisters)(struct _HwcContext *hwc);
    FxBool (* InitGuiRegisters)(struct _HwcContext *hwc);
    FxBool (* InitGammaRGB)(struct _HwcContext *hwc, FxFloat gammaR, FxFloat gammaG, FxFloat gammaB);
    FxBool (* InitCmdFifo)( struct _HwcContext *hwc, int which, FxU32 fifoStart,
                FxU32 size, FxBool disableHoles, FxBool agpEnable);
    FxBool (* Idle)(struct _HwcContext *hwc);
    FxBool (* IdleNoNop)(struct _HwcContext *hwc);
    FxBool (* Vsync)( struct _HwcContext *hwc);
    FxBool (* VsyncNot)( struct _HwcContext *hwc);
    FxBool (* MapBoard) (struct _HwcContext *hwc);
    FxBool (* UnmapBoard)(struct _HwcContext *hwc);
    FxBool (* ShutdownBoard) (struct _HwcContext *hwc);
    FxBool (* InitVideo)(struct _HwcContext *hwc, HwcScreenResolution resolution,
                         HwcScreenRefresh refresh, void *timing);
    FxBool (* AllocMemRegion)(struct _HwcContext *hwc, FxU32 memType, FxU32 sizeInBytes, FxU32 align, HwcMemRegion *pBlock, FxU32 hints);
    FxBool (* FreeMemRegion)(struct _HwcContext *hwc, HwcMemRegion *pBlock);
    FxBool (* AllocBuffer)(HwcBuffer *pBuff);
    FxBool (* FreeBuffer)(HwcBuffer *pBuff);
    FxBool (* SetBuffer)(HwcBuffer *pBuff, HwcBufferType type);
    HwcRegInfo * (* RegisterInfo)(struct _HwcContext *hwc, HwcAddressMap addrMap, FxU32 addr); 
    FxBool (* SetIface)(struct _HwcContext *hwc, HwcIfaceType itype, void* iface);
} HwcContext;


typedef FxBool (FX_CALL *PfnInitSim)(HwcContext * hwc, HwcSimulator * hws);

/*-------------------------------------------------------------------
   the root of all HWC information
  -------------------------------------------------------------------*/
typedef struct _HwcInfo {
    FxBool  initialized; /* HWC has been initialized */
    FxBool  agpInitialized;
    FxU32   maxDevices;  /* maximum number of PCI devices */
    HwcDevice devices[HWC_MAX_PCI_DEVICES];
    FxU32   pciVxdVer;   /* vxd version we are using */
    const HwcPciRegister *pciRegs;
    FxU32  configMechanism;
    FxBool busDetected;
    FxU32  uiMethod;
    HwcDeviceType *deviceTypes; /* NULL terminated list of supported 
                                   device types */
    HwcContext boardInfo[HWC_MAX_BOARDS]; /* pool of hardware contexts */
    int pollLimit;	 /* number of pixels to poll msg Q after */
    int pollCount;	 /* current pixel counter */
    FxU32 lastIO;
    FxU32 fbiMemSizeMB;  /* Default fb memory size in MB */
    FxU32 agpFree;       /* AGP Free location in bytes XXX AGP is per hwc */
    FxU8 *agpVirtAddr;   /* AGP virtual base address */
    FxU32 agpSizeInBytes;/* AGP memory size (in Bytes) */
    FxU32 agpBaseAddrH;  /* upper 4 bits of AGP physical base address */
    FxU32 agpBaseAddrL;  /* lower 32 bits of AGP physical base address */
    FxU32 agpRqDepth;    /* AGP request depth */
    HwcMemRegion agpMemReg;     /* AGP memory region */
    HwcMemRegion sysMemReg;     /* System memory region */
    void *hInstance; /* module handle */
    void *hAccel;
    FxBool ignoreErrors;
    FxBool debug;
    FxU32 errorCount;
    struct {
        FxFloat r;
        FxFloat g;
        FxFloat b;
    } gamma;
    FxU32 randx;
    FxU32 memDebugLevel;
    FxU32 memDebugFlags;
    FxU32 bitsPerPixel[HWC_NUM_PIXEL_FORMATS];

    /*
     * Callback functions...
     */
    FxBool  (FX_CALL *pfnInitSim)(HwcContext * hwc, HwcSimulator * hws);
    
    void    (FX_CALL *pfnSetDebugLevel)(const int level, const int value);
    int     (FX_CALL *pfnGetDebugLevel)(const int level);

    void    (FX_CALL *pfnGdbgVprintf)(int cont, const char *format, va_list args);
    void    (*pfnGdbgPrintf)(const char *format, ...);
    int     (*pfnGdbgVinfo)(const int level, const char *format, va_list args);
    int     (*pfnGdbgInfo)(const int level, const char *format, ...);
    int     (*pfnGdbgVinfoMore)(const int level, const char *format, va_list args);
    int     (*pfnGdbgInfoMore)(const int level, const char *format, ...);
    int     (*pfnGdbgVerror)(const char *kind, const char *format, va_list args);
    void    (*pfnGdbgError)(const char *kind, const char *format, ...);
    void    (FX_CALL *pfnPrintRegisterFields)(HwcContext *hwc, HwcRegInfo *ri, FxU32 data, const int level);

    FxU32   (FX_CALL *pfnStore32)(volatile void *addr, FxU32 data);
    FxU32   (FX_CALL *pfnLoad32)(volatile void *addr);
    FxU32   (FX_CALL *pfnRead32)(FxU32 * addr);
    void    (FX_CALL *pfnWrite32)(FxU32 * addr, FxU32 data);
    void    (FX_CALL *pfnWriteVStruct)(HwcSimulator * hws, FxU32 id, void *p);
    void    (FX_CALL *pfnWriteVvector)(HwcSimulator * hws, FxU32 id, FxU32 detail, const char *format, va_list args);
    void    (FX_CALL *pfnWriteVector)(HwcSimulator * hws, FxU32 id, FxU32 detail, const char *format, ...);

    void    (FX_CALL *pfnMemInit)( FxU32 flags );
    void    (FX_CALL *pfnMemShutdown)( void );
    FxBool  (FX_CALL *pfnInitMemRegion)(HwcMemRegion * pRegion, FxI32 baseAddress, FxI32 sizeInBytes);
    void    (FX_CALL *pfnAGPVirtToPhys)(void *_virtAddr, FxU32 * physAddrH, FxU32 * physAddrL);
    void*   (FX_CALL *pfnAGPMemAlloc)(FxU32 sizeBytes);
    FxBool  (FX_CALL *pfnMemCheck)( char *where, FxU32 line);
    void*   (FX_CALL *pfnMemRealloc)(void *ptr, size_t sizeInBytes, char *where, FxU32 line);
    char*   (FX_CALL *pfnStrDup)(const char *s, char *where, FxU32 line);
    void*   (FX_CALL *pfnMemAlignedCalloc)(size_t num, size_t sizeInBytes, FxU32 alignment, char *where, FxU32 line);
    void    (FX_CALL *pfnMemFree)(void *mem, char *where, FxU32 line);
    int     (FX_CALL *pfnMemDerefFree)(void *mem, char *where, FxU32 line);
    void    (FX_CALL *pfnMemStats)(HwcMemStats *s);
} HwcInfo;

extern HwcInfo hwcInfo;

/*
   SST Hardware Initialization routine protypes
  
   If all initialization routines are called, it is assumed they are called
   in the following order:
     0. hwcInit();
     1. hwcAGPInit();
     2. hwcMapBoard();
     3. hwcInitRegisters();
     4. hwcInitGamma();
     5. hwcInitVideo();
     6. hwcShutdown();
  
   hwcShutdown() is called at the end of an application to turn off
    the graphics subsystem
  
*/

FX_ENTRY HwcPCIState FX_CALL hwcDeviceState( HwcDevice *dev );

FX_ENTRY void FX_CALL hwcPrintRegisterFields(HwcContext *hwc, HwcRegInfo *ri, FxU32 data, const int level );

FX_ENTRY FxBool FX_CALL hwcGetConfigData( HwcDevice *dev, HwcPciRegister reg,
                                          FxU32 *data );

FX_ENTRY FxBool FX_CALL hwcSetConfigData( HwcDevice *dev, HwcPciRegister reg, 
                                          FxU32 *data );

FX_ENTRY HwcDevice * FX_CALL hwcFindDevice(FxU32 vendorID, FxU32 deviceID, 
                                              FxU32 cardNum);

FX_ENTRY FxBool FX_CALL hwcMapPhysicalToLinear(FxU32 *linear_addr, FxU32 physical_addr,FxU32 *length);

FX_ENTRY const char * FX_CALL hwcGetVendorName( FxU32 vendor_id );

FX_ENTRY const char * FX_CALL hwcGetClassName( FxU32 class_code , FxU32 deviceID
);

FX_ENTRY FxBool FX_CALL hwcFindMTRRMatch(FxU32 pBaseAddrs, FxU32 psz, 
                                         HwcMemType type, FxU32 *mtrrNum);

FX_ENTRY FxBool FX_CALL hwcFindFreeMTRR(FxU32 *mtrrNum);

FX_ENTRY FxBool FX_CALL hwcSetMTRR(FxU32 mtrrNo, FxU32 pBaseAddr, FxU32 psz, HwcMemType type);

FX_ENTRY FxBool  FX_CALL hwcIOStore8 (FxU16 port, FxU8 data);
FX_ENTRY FxBool  FX_CALL hwcIOStore16(FxU16 port, FxU16 data);
FX_ENTRY FxBool  FX_CALL hwcIOStore32(FxU16 port, FxU32 data);
FX_ENTRY FxU8  FX_CALL hwcIOLoad8  (FxU16 port);
FX_ENTRY FxU16 FX_CALL hwcIOLoad16 (FxU16 port);
FX_ENTRY FxU32 FX_CALL hwcIOLoad32 (FxU16 port);

FxBool hwcInitializeDDio(void);
void hwcUnmapPhysical( FxU32 linear_addr, FxU32 length );

FX_ENTRY HwcInfo * FX_CALL hwcInit(void);
FX_ENTRY FxU32 FX_CALL hwcInt32Random(FxU32 maxr);
FX_ENTRY FxU32 FX_CALL hwcGetSeed(void);
FX_ENTRY void FX_CALL hwcSetSeed(FxU32 seed);

FX_ENTRY FxBool FX_CALL hwcAGPInit(void);
FX_ENTRY FxBool FX_CALL _hwcInitMemRegion(HwcMemRegion *pRegion, FxI32 start, FxI32 sizeInBytes);
FX_ENTRY FxBool FX_CALL _hwcAllocMemReg( HwcMemRegion *pBlock, HwcMemRegion *pRegion, FxU32 sizeInBytes, FxU32 align, FxU32 hints);
FX_ENTRY void FX_CALL _hwcFreeMemReg( HwcMemRegion *pBlock, HwcMemRegion *pRegion);
FX_ENTRY HwcContext * FX_CALL hwcAllocContext(HwcDevice *dev);
FX_ENTRY HwcSimulator * FX_CALL hwcFindSimulator(HwcContext *, char *name);
FX_ENTRY FxBool FX_CALL hwcInitSim(HwcContext *hwc, HwcSimulator *hws);
FX_ENTRY void FX_CALL hwcFreeContext(HwcContext *hwc);
FX_ENTRY FxU32 FX_CALL hwcEnumHardware( HwcHWEnumCallback *cb );
FX_ENTRY FxBool FX_CALL hwcMapBoard( HwcContext *hwc);
FX_ENTRY FxBool FX_CALL hwcInitCmdFifo( HwcContext *hwc, int which, FxU32 fifoStart,
                FxU32 size, FxBool disableHoles, FxBool agpEnable);
FX_ENTRY FxBool FX_CALL hwcInitRegisters(HwcContext *hwc);
FX_ENTRY FxBool FX_CALL hwcInitGuiRegisters( HwcContext *hwc);
FX_ENTRY FxBool FX_CALL hwcInitRenderingRegisters(HwcContext *hwc);
FX_ENTRY FxBool FX_CALL hwcInitGamma(HwcContext *hwc, FxFloat gamma);
FX_ENTRY FxBool FX_CALL hwcInitGammaRGB(HwcContext *hwc, FxFloat r, FxFloat g, FxFloat b);
FX_ENTRY FxBool FX_CALL hwcInitVideo(HwcContext *hwc, FxU32 resolution,
		FxU32 refresh, void *);
FX_ENTRY FxBool FX_CALL hwcIdle(HwcContext *hwc);
FX_ENTRY FxBool FX_CALL hwcIdleNoNop(HwcContext *hwc);
FX_ENTRY FxBool FX_CALL hwcInitUSWC(HwcContext *hwc);
FX_ENTRY FxBool FX_CALL hwcUnmapBoard(HwcContext *hwc);
FX_ENTRY FxBool FX_CALL hwcStall( HwcContext *hwc, FxU32 nClocks);
FX_ENTRY void   FX_CALL hwcShutdown(void);

FX_ENTRY void   FX_CALL hwcInvokeDebugger(void);
FX_ENTRY void   FX_CALL hwcGUIKeepAlive(int count);
FX_ENTRY void FX_CALL hwcDrawLine(struct _HwcBuffer *pBuff, HwcVertex *a, HwcVertex *b);
FX_ENTRY void FX_CALL hwcDrawTriangle(struct _HwcBuffer *pBuff, HwcVertex *a, HwcVertex *b, HwcVertex *c);
FX_ENTRY void FX_CALL hwcFillRectangle(struct _HwcBuffer *pBuff, FxI32 x, FxI32 y, FxI32 w, FxI32 h);
FX_ENTRY FxBool FX_CALL hwcVsync(HwcContext *hwc);
FX_ENTRY FxBool FX_CALL hwcVsyncNot(HwcContext *hwc);
FX_ENTRY FxBool FX_CALL hwcSimVectorInit(HwcSimulator *hws, HwcTestDescription *test);

FX_ENTRY FxBool FX_CALL hwcVectorInit(HwcContext *hwc, HwcTestDescription *test);
FX_ENTRY FxBool FX_CALL hwcSimCaptureVector(HwcSimulator *hws, FxU32 id, FxU32 detail, char *name, FxI32 breakPoint);
FX_ENTRY FxBool FX_CALL hwcSetVector(char *name, FxBool trace, FxU32 detail, char *fileName, FxI32 breakPoint);

FX_ENTRY FxBool FX_CALL hwcCaptureVector(HwcContext *hwc, FxU32 id, FxU32 detail, char *name, FxI32 breakPoint);
FX_ENTRY void FX_CALL hwcWriteVector(HwcSimulator *hws, 
                               FxU32 id, FxU32 detail, const char *format, ...);
FX_ENTRY void FX_CALL hwcWriteMemoryStream(HwcSimulator *hws, FxU32 startAddress, FxU32 endAddress, 
                                           FxBool writeAGP);
FX_ENTRY void FX_CALL hwcIoVector(HwcSimulator *hws, HwcIoTransaction transaction, 
                                  FxU32 size, FxU32 addr, FxU32 data);

FX_ENTRY FxBool FX_CALL hwcAllocMemRegion(struct _HwcContext *hwc, FxU32 memType, FxU32 sizeInBytes, FxU32 align, HwcMemRegion *pBlock, FxU32 hints);
FX_ENTRY FxBool FX_CALL hwcFreeMemRegion(struct _HwcContext *hwc, HwcMemRegion *pBlock);

FX_ENTRY FxBool FX_CALL
hwcInitBuffer(HwcBuffer *pBuff, struct _HwcContext *hwc, FxU32 width, FxU32 height, 
              HwcBufferType type, HwcPixelFormat format);

FX_ENTRY const char * FX_CALL hwcPixelFormatToString(HwcPixelFormat format);
FX_ENTRY FxBool FX_CALL hwcAllocBuffer(HwcBuffer *pBuff, struct _HwcContext *hwc, 
                FxU32 width, FxU32 height, HwcBufferType type, 
                HwcPixelFormat format, FxU32 hints);
FX_ENTRY FxBool FX_CALL hwcAllocNamedBuffer(HwcBuffer *buff, struct _HwcContext *hwc, int width, int height, HwcBufferType type, 
         HwcPixelFormat format, FxU32 hints, char *name);
FX_ENTRY FxBool FX_CALL hwcShowBuffer(HwcBuffer *pBuff, FxBool visible);
 FX_ENTRY FxBool FX_CALL hwcUpdateBuffer(HwcBuffer *pBuff);
FX_ENTRY FxBool FX_CALL hwcReallocBuffer(HwcBuffer *pBuff, struct _HwcContext *hwc, FxU32 width, FxU32 height, 
                                         HwcBufferType type, HwcPixelFormat format, FxU32 hints);

FX_ENTRY FxBool FX_CALL hwcFreeBuffer(HwcBuffer *pBuff);

FX_ENTRY FxU32 FX_CALL hwcFormatToBPP( HwcPixelFormat pixelFormat);

FX_ENTRY FxBool FX_CALL hwcBufferSave(struct _HwcSimulator *hws, HwcBuffer *pBuff,
                                      const char *filename, FxU32 type);

FX_ENTRY FxBool FX_CALL hwcInit3dBuffers( FxU32 hWnd,
                                          HwcPixelFormat pixFormat,
                                          HwcColorFormat cFormat,
                                          HwcOriginLocation yOrigin,
                                          int nColBuffers,
                                          int nAuxBuffers);

FX_ENTRY const char * FX_CALL hwcGetEnv(const char *name);
FX_ENTRY void FX_CALL hwcBufferInit(HwcBuffer *pBuff, FxI32 xmin, FxI32 ymin, 
                                    FxI32 w, FxI32 h, FxU32 pix, HwcBufferInit ctl);
FX_ENTRY FxI32 FX_CALL hwcGetEnvI32(const char *name, const FxI32 defaultVal, const char *msg);
FX_ENTRY FxBool FX_CALL hwcSetBuffer(HwcBuffer *pBuff, HwcBufferType type);
FxBool hwcWindowSetBuffer(HwcWindow *pWnd, HwcBuffer *pBuff);
FX_ENTRY FxBool FX_CALL hwcClientWindowSetBuffer(void *hWnd, HwcBuffer *pBuff);
FX_ENTRY HwcBuffer * FX_CALL hwcGetBuffer(struct _HwcContext *hwc, HwcBufferType type);
FX_ENTRY void FX_CALL hwcPrintBuffer(HwcBuffer *pBuff);
FX_ENTRY FxBool FX_CALL hwcGUIOpen( struct _HwcSimulator *hws);
FX_ENTRY void FX_CALL hwcGUIUnmapBoard( struct _HwcSimulator *hws);
FX_ENTRY void FX_CALL hwcGUIShutdown( void );
FX_ENTRY HwcWindow * FX_CALL hwcNewViewWindow(HwcBuffer *pBuff);
FX_ENTRY HwcWindow * FX_CALL hwcNewClientWindow(void *hWnd, HwcBuffer *pBuff);
FX_ENTRY HwcWindow * FX_CALL hwcNewSimViewWindow(struct _HwcSimulator *hws, HwcBuffer *pBuff);
FX_ENTRY void FX_CALL hwcDeleteWindow(HwcWindow *cw);
FX_ENTRY void FX_CALL hwcDeleteAllWindows(struct _HwcSimulator *hws);
FX_ENTRY void FX_CALL hwcRefreshWindow(HwcWindow *cw);
FX_ENTRY void FX_CALL hwcRefreshBuffer(HwcBuffer *pBuff);
FX_ENTRY void FX_CALL hwcRepaintWindow(HwcWindow *cw, void *hDc, HwcRect *prect);
FX_ENTRY void FX_CALL hwcGUIDoBreak(struct _HwcSimulator *hws);
FX_ENTRY void FX_CALL hwcGUIUpdateDynamicText(struct _HwcSimulator *hws);
FX_ENTRY void FX_CALL hwcDeleteWindow(HwcWindow *cw);
FX_ENTRY void FX_CALL hwcApplyToAllWindows(struct _HwcSimulator *hws, void (FX_CALL *)(HwcWindow *));
FX_ENTRY void FX_CALL hwcGUIChange(struct _HwcSimulator *hws, HwcChangeEvent which, FxI32 count);
FX_ENTRY void FX_CALL hwcGUIReadMessageQueue(void);
FX_ENTRY int FX_CALL hwcKbHit( void );
FX_ENTRY int FX_CALL hwcGetCH( void );
FX_ENTRY void FX_CALL hwcUpdateMem(struct _HwcSimulator *hws, FxU32 addr, FxU32 size);
FX_ENTRY void FX_CALL hwcRefresh(struct _HwcSimulator *hws);
FX_ENTRY void FX_CALL hwcRefreshAll(HwcContext *hwc);
FX_ENTRY void FX_CALL hwcVideoUpdate(struct _HwcContext *hwc);

/* backdoor memory accessors */

FX_ENTRY FxU32 FX_CALL hwcReadMemory(HwcContext * hwc, FxU32 addr, FxU32 size, FxU32 memType, FxU32 chipID);
FX_ENTRY void  FX_CALL hwcWriteMemory(HwcContext * hwc, FxU32 addr, FxU32 size, FxU32 memType, FxU32 mask, FxU32 data);
FX_ENTRY HwcPixel FX_CALL hwcReadPixel(HwcSimulator *hws, HwcBuffer *pBuff, int x, int y);
FX_ENTRY void FX_CALL hwcPixelToRGB(HwcBuffer *pBuff, HwcPixel pix, FxU8 *r, FxU8 *g, FxU8 *b);
FX_ENTRY void FX_CALL hwcReadColor(HwcSimulator *hws, HwcBuffer *pBuff, int x, int y, 
                                   FxU8 *a, FxU8 *r, FxU8 *g, FxU8 *b);

FX_ENTRY FxBool FX_CALL 
hwcComparePixel(HwcBuffer *pBuff, int x, int y, HwcPixel *pPixel);
FX_ENTRY FxBool FX_CALL 
hwcCompareRect(HwcBuffer *pBuff, int x, int y, int width, int height, HwcPixel *pPixel);
FX_ENTRY FxBool FX_CALL hwcWritePixel(HwcSimulator *hws, HwcBuffer *pBuff, int x, int y, HwcPixel pixel);
FX_ENTRY void FX_CALL 
hwcSetPixel(HwcBuffer *pBuff, int x, int y, HwcPixel pixel);
FX_ENTRY void FX_CALL 
hwcSetRect(HwcBuffer *pBuff, int x, int y, int width, int height, HwcPixel pixel);
FX_ENTRY FxBool FX_CALL hwcLockBuffer(HwcBuffer *buffer);
FX_ENTRY FxBool FX_CALL hwcUnlockBuffer(HwcBuffer *buffer);

/* should be put into a driver */
FX_ENTRY FxU32  FX_CALL hwcAGPWriteMem32( volatile void *virtAddr, FxU32 value);
FX_ENTRY FxFloat  FX_CALL hwcAGPWriteMem32f( volatile void *virtAddr, FxFloat value);
FX_ENTRY FxU32 FX_CALL hwcAGPReadMem32( volatile void *virtAddr);
FX_ENTRY void  FX_CALL hwcAGPPhysWriteMem32( FxU32 physAddrHi, FxU32 physAddrLo , FxU32 value);
FX_ENTRY FxU32 FX_CALL hwcAGPPhysReadMem32(  FxU32 physAddrHi, FxU32 physAddrLo );
FX_ENTRY FxU32* FX_CALL hwcAGPPhysToVirt( FxU32 physAddrHi, FxU32 physAddrLo );
FX_ENTRY void  FX_CALL hwcAGPVirtToPhys( void *virtAddr, FxU32 *physAddrHi, FxU32 *physAddrLo ); 

/* These are the CSIM interface routines */
#ifdef HWC_CSIM	
	FX_ENTRY FxU8    FX_CALL hwcStore8 (volatile void *, FxU8);
	FX_ENTRY FxU16   FX_CALL hwcStore16(volatile void *, FxU16);
	FX_ENTRY FxU32   FX_CALL hwcStore32(volatile void *, FxU32);
	FX_ENTRY FxFloat FX_CALL hwcStore32f(volatile void *, FxFloat);
	FX_ENTRY FxU8    FX_CALL hwcLoad8 (volatile void *);
	FX_ENTRY FxU16   FX_CALL hwcLoad16(volatile void *);
	FX_ENTRY FxU32   FX_CALL hwcLoad32(volatile void *);
#endif

/* Here are the macro defines for talking to hardware */
#ifdef HWC_CSIM			/* CSIM macros */
	#define GET8(s)		hwcLoad8 (&(s))
	#define GET16(s)	hwcLoad16(&(s))
	#define GET(s)		hwcLoad32(&(s))

	#define SET8(d,s)	hwcStore8 (&(d),s)
	#define SET16(d,s)	hwcStore16(&(d),s)
	#define SET(d,s)	hwcStore32(&(d),s)
	#define SETF(d,s)	hwcStore32f(&(d),s)

	#define AGPMEMINIT()       agpMemInit()
	#define AGPMEMALLOC(sst,size)       agpMemAlloc(sst,size)
	#define AGPWRV(v,d)        hwcAGPWriteMem32(&(v),d)
	#define AGPWRVF(v,f)       hwcAGPWriteMem32f(&(v),f)
	#define AGPRDV(v)          hwcAGPReadMem32(&(v))
	#define AGPWRP(aHi,aLo,d)  AGPWRV( *hwcAGPPhysToVirt(aHi,aLo), d )
	#define AGPRDP(aHi,aLo)    AGPRDV( *hwcAGPPhysToVirt(aHi,aLo) )
#else				/* REAL hw */
	#define GET8(s) (s)
	#define GET16(s) (s)
	#define GET(s) (s)
	#define SET8(d,s) (d = s)
	#define SET16(d,s) (d = s)
	#define SET(d,s) (d = s)
	#define SETF(d,s) ((*(float *)&(d)) = s)

	#define AGPMEMINIT()       GDBG_ERROR("AGPMEMINIT","nyi for hardware\n");
	#define AGPMEMALLOC(sst,size)     GDBG_ERROR("AGPMEMALLOC","nyi for hardware\n");
	#define AGPWRV(v,d)        (v) = (d)
	#define AGPWRVF(v,d)       (v) = (d)
	#define AGPRDV(v)          (v)
	#define AGPWRP(aHi,aLo,d)  AGPWRV( *hwcAGPPhysToVirt(aHi,aLo), d )
	#define AGPRDP(aHi,aLo)    AGPRDP( *hwcAGPPhysToVirt(aHi,aLo) )
#endif

#define GET_IO8(p)      hwcIOLoad8(p)
#define GET_IO16(p)     hwcIOLoad16(p)
#define GET_IO(p)       hwcIOLoad32(p)
#define SET_IO8(p,d)    hwcIOStore8(p,d)
#define SET_IO16(p,d)   hwcIOStore16(p,d)
#define SET_IO(p,d)     hwcIOStore32(p,d)

/*-------------------------------------------------------------------
   internal HWC stuff not meant for external use
  -------------------------------------------------------------------*/
FX_ENTRY HwcDevice * FX_CALL hwcFakeDevice( FxU16 vendorID, 
                                            FxU16 deviceID, HwcPCIState state);

#if defined(BUILD_HWC) || defined(BUILD_DIAGS)

FX_ENTRY FxBool FX_CALL hwcPCIInit( void );
FX_ENTRY FxBool FX_CALL hwcPCIShutdown( void );
HwcContext *hwcDeviceToHwc(FxU32 deviceNumber);
FX_ENTRY void FX_CALL hwcWrite32(FxU32 *addr, FxU32 data);
FX_ENTRY FxU32 FX_CALL hwcRead32(FxU32 *addr);

/* GMT: Init code SET/GET always go thru subroutines (allows for P6 fencing) */
#define IGET(A)   hwcRead32((FxU32 *) &(A))
#define ISET(A,D) hwcWrite32((FxU32 *) &(A), D)

FxU32
hwcFakeAddressGetBoard(FxU32 a);

FxBool
hwcBadAddress(FxU32 a);

/* this is the FAKE address where the hardware lives
   we use a large address so attempts to write to it get an access violation
   and it has 28 zero bits so that we can easily figure out the board number
   and the offset into the board 
 */
#define HWC_DEV_TO_HWC(_devNum) ( (_devNum >= HWC_MAX_PCI_DEVICES ) ? NULL : hwcInfo.devices[_devNum].hwc )
#define HWC_BAD_ADDRESS(a) hwcBadAddress((FxU32)a)
// #define HWC_FAKE_ADDRESS_MAKE(boardNum) (void *)((boardNum+1)<<28)
#define HWC_FAKE_ADDRESS_GET_BOARD(a) hwcFakeAddressGetBoard((FxU32)a)
#define HWC_FAKE_ADDRESS_GET_CSIM(a) \
		hwcInfo.boardInfo[HWC_FAKE_ADDRESS_GET_BOARD(a)].csim
#define HWC_FAKE_ADDRESS_GET_HWC(a) \
		&hwcInfo.boardInfo[HWC_FAKE_ADDRESS_GET_BOARD(a)]
#define HWC_FAKE_ADDRESS_GET_OFFSET(a) ((FxU32)a&0x0FFFFFFF)
/* #define HWC_FAKE_ADDRESS_GET_BASE(a) ((FxI32)a&~0x0FFFFFFF) */

#define HWC_BAD_PORT(a) (((FxU32)a&0xF000)==0)
#define HWC_FAKE_PORT_MAKE(boardNum) (FxU16)((boardNum+1)<<12)
#define HWC_FAKE_PORT_GET_BOARD(a) ((((FxU16)a>>12)&0xF)-1)
#define HWC_FAKE_PORT_GET_CSIM(a) \
		hwcInfo.boardInfo[HWC_FAKE_PORT_GET_BOARD(a)].csim
#define HWC_FAKE_PORT_GET_HWC(a) \
		&hwcInfo.boardInfo[HWC_FAKE_PORT_GET_BOARD(a)]
#define HWC_FAKE_PORT_GET_OFFSET(a) ((FxU16)a&0x0FFF)

// if debug info turned on then GDBG_INFO does something
#ifdef GDBG_INFO_ON
#define HWC_TRACE(_hws, _id) ((_hws->vecInfo.numModules > 0 ) && _hws->vecInfo.module[_id].enabled )
#define HWC_TRACE_LEVEL(_hws, _id) (((_hws->vecInfo.numModules > 0 ) && _hws->vecInfo.module[_id].enabled ) ? \
                                    _hws->vecInfo.module[_id].detail : 0 )  
#define HWC_TRACE_VECNUM(_hws, _id) (((_hws->vecInfo.numModules > 0 ) && _hws->vecInfo.module[_id].enabled ) ? \
                                    _hws->vecInfo.module[_id].nextVector : 0 )  
#define HWC_TRACE_MODULE(_hws, _id) ((_hws->vecInfo.numModules > 0 ) && ( _hws->vecInfo.type & HWC_MODULE_TEST ) && ( _hws->vecInfo.module[_id].enabled ))
#define HWC_VECTOR hwcWriteVector
#else
#define HWC_TRACE(_hws, _id) ( 0 )
#define HWC_TRACE_LEVEL(_hws, _id) (0)
#define HWC_TRACE_VECNUM(_hws, _id) (0)
#define HWC_VECTOR if (0) (void)
#define HWC_TRACE_MODULE(_hws, _id) ( 0 )
#endif

void _hwcGUIUpdateRect(HwcSimulator *hws, int x, int y, int w, int h, FxU32 addr, FxU32 memType, HwcBufferType which);
void _hwcGUIKeepAlive(HwcSimulator *hws, int count);
void _hwcGUIChange(HwcSimulator *hws, FxU32 HwcChangeEvent, FxI32 count);

void _hwcGUIUpdateRect1(HwcSimulator *hws, int xmin, int ymin, int w, int h, FxU32 addr, FxU32 memType, HwcBufferType which);
void _hwcGUIKeepAlive1(HwcSimulator *hws, int count);
void _hwcGUIChange1(HwcSimulator *hws, HwcChangeEvent which, FxI32 count);

FxU32 _hwcFetchRegister( FxU32 offset, FxU32 size_in_bytes, 
                   FxU32 device_number, FxU32 config_mechanism  );

void
_hwcUpdateRegister( FxU32 offset, FxU32 data, FxU32 size_in_bytes,  
                    FxU32 device_number, FxU32 config_mechanism  );

#endif /* BUILD_HWC */

#ifdef __cplusplus
}
#endif

#endif /* !__FXHWC_H__ */