/******************************Module*Header*******************************\
* Module Name: driver.h
*
* Contains prototypes for the display driver.
*
* Copyright (c) 1992-1996 Microsoft Corporation
* Copyright (c) 1997 3Dfx Interactive, Inc.
\**************************************************************************/

#if (DXDDKVERSION == 7)
#include "dx7todx8.h"
#endif

//////////////////////////////////////////////////////////////////////
// Put all the conditional-compile constants here.  There had better
// not be many!

// Multi-board support can be enabled by setting this to 1:

#define MULTI_BOARDS            0

// This is the maximum number of boards we'll support in a single
// virtual driver:

#if MULTI_BOARDS
    #define MAX_BOARDS          16
    #define IBOARD(ppdev)       ((ppdev)->iBoard)
#else
    #define MAX_BOARDS          1
    #define IBOARD(ppdev)       0
#endif

// Useful for visualizing the 2-d heap:

#define DEBUG_HEAP              0

// on NT5, use ddraws memory management so we can
// have ddraw surfaces and gdi device bitmaps in
// offscreen memory at the same time
#if (_WIN32_WINNT >= 0x0500)
  #define USE_NT5_DDMEMMGR          1
  #define ENABLE_LINEAR_DFBS        1  //Linear DFBs require USE_NT5_DDMEMMGR
  #define ENABLE_RECONFIG_VIDMEM    (ENABLE_LINEAR_DFBS && 1)
#else
  #define USE_NT5_DDMEMMGR          0
  #define ENABLE_LINEAR_DFBS        0  //Linear DFBs require USE_NT5_DDMEMMGR
  #define ENABLE_RECONFIG_VIDMEM    0
#endif

#define REDUCED_MEMORY_MAPPINGS     1 // needs to be in sync with miniport's h3.c setting

#ifdef SLI_AA
#define ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP     1
#endif

//////////////////////////////////////////////////////////////////////
// Miscellaneous shared stuff
#define _MAP_FRAMEBUFFER_ONCE		1

#define STANDARD_DEBUG_PREFIX   "H5: "      // All debug output is prefixed
                                            //   by this string

#define DLL_NAME                L"3dfxvs"  // Name of the DLL in UNICODE

#if defined(MS_VIEW)
#define ALLOC_TAG               'xfd3'
#else
#define ALLOC_TAG               '1_5H'      // Four byte tag used for tracking
#endif //MSVIEW                             //   memory allocations (characters
                                            //   are in reverse order)

#define CLIP_LIMIT          50  // We'll be taking 800 bytes of stack space

#define DRIVER_EXTRA_SIZE   0   // Size of the DriverExtra information in the
                                //   DEVMODE structure

#define TMP_BUFFER_SIZE     8192  // Size in bytes of 'pvTmpBuffer'.  Has to
                                  //   be at least enough to store an entire
                                  //   scan line (i.e., 6400 for 1600x1200x32).

#if defined(_ALPHA_)
    #define XFER_BUFFERS    16  // Defines the maximum number of write buffers
                                //   possible on any Alpha.  Must be a power
#else                           //   of two.
    #define XFER_BUFFERS    1   // On non-alpha systems, we don't have to
                                //   worry about the chip caching our bus
#endif                          //   writes.

// #define XFER_MASK           (XFER_BUFFERS - 1)

typedef struct _CLIPENUM {
    LONG    c;
    RECTL   arcl[CLIP_LIMIT];   // Space for enumerating complex clipping

} CLIPENUM;                         /* ce, pce */

typedef struct _PDEV PDEV;      // Handy forward declaration

VOID vSetClipping(PDEV*, RECTL*);
VOID vResetClipping(PDEV*);

#ifdef  OPENGL_ICD
//////////////////////////////////////////////////////////////////////
// OpenGL ICD Stuff

#define GL_DRV_VERSION          2               // as per ICD spec
#define GL_DRV_DRIVER_VERSION   1               // ??? Not sure why '1'
#define GL_DRV_NAME             L"3Dfx\0"    // L for 'unicode', 16 bit chars
#define GL_DRV_NAME_LEN         20

#endif  // OPENGL_ICD

//////////////////////////////////////////////////////////////////////
// Text stuff

#define GLYPH_CACHE_HEIGHT  48  // Number of scans to allocate for glyph cache,
                                //   divided by pel size

#define GLYPH_CACHE_CX      64  // Maximal width of glyphs that we'll consider
                                //   caching

#define GLYPH_CACHE_CY      64  // Maximum height of glyphs that we'll consider
                                //   caching

// #define MAX_GLYPH_SIZE      ((GLYPH_CACHE_CX * GLYPH_CACHE_CY + 31) / 8)
#define MAX_GLYPH_SIZE      (((GLYPH_CACHE_CX+7) * GLYPH_CACHE_CY + 31) / 8)
                                // Maximum amount of off-screen memory required
                                //   to cache a glyph, in bytes.  Byte packed
                                //   and dword aligned.

#define GLYPH_ALLOC_SIZE    8100
                                // Do all cached glyph memory allocations
                                //   in 8k chunks

#define HGLYPH_SENTINEL     ((ULONG) -1)
                                // GDI will never give us a glyph with a
                                //   handle value of 0xffffffff, so we can
                                //   use this as a sentinel for the end of
                                //   our linked lists

#define GLYPH_HASH_SIZE     256

#define GLYPH_HASH_FUNC(x)  ((x) & (GLYPH_HASH_SIZE - 1))

typedef struct _CACHEDGLYPH CACHEDGLYPH;
typedef struct _CACHEDGLYPH
{
    CACHEDGLYPH*    pcgNext;    // Points to next glyph that was assigned
                                //   to the same hash table bucket
    HGLYPH          hg;         // Handles in the bucket-list are kept in
                                //   increasing order
    POINTL          ptlOrigin;  // Origin of glyph bits

    // Device specific fields below here:

    LONG            cx;         // Glyph width
    LONG            cy;         // Glyph height
    LONG            cxcy;       // Packed width and height, (y,x).
    ULONG           ulLinearStart;
                                // Linear start address of glyph in off-screen
                                //   memory

    LONG            cd;         // Number of dwords to be transferred
} CACHEDGLYPH;  /* cg, pcg */

typedef struct _GLYPHALLOC GLYPHALLOC;
typedef struct _GLYPHALLOC
{
    GLYPHALLOC*     pgaNext;    // Points to next glyph structure that
                                //   was allocated for this font
    CACHEDGLYPH     acg[1];     // This array is a bit misleading, because
                                //   the CACHEDGLYPH structures are actually
                                //   variable sized
} GLYPHAALLOC;  /* ga, pga */

typedef struct _CACHEDFONT CACHEDFONT;
typedef struct _CACHEDFONT
{
    CACHEDFONT*     pcfNext;    // Points to next entry in CACHEDFONT list
    CACHEDFONT*     pcfPrev;    // Points to previous entry in CACHEDFONT list
    GLYPHALLOC*     pgaChain;   // Points to start of allocated memory list
    CACHEDGLYPH*    pcgNew;     // Points to where in the current glyph
                                //   allocation structure a new glyph should
                                //   be placed
    LONG            cjAlloc;    // Bytes remaining in current glyph allocation
                                //   structure
    CACHEDGLYPH     cgSentinel; // Sentinel entry of the end of our bucket
                                //   lists, with a handle of HGLYPH_SENTINEL
    CACHEDGLYPH*    apcg[GLYPH_HASH_SIZE];
                                // Hash table for glyphs

} CACHEDFONT;   /* cf, pcf */

typedef struct _XLATECOLORS {       // Specifies foreground and background
ULONG   iBackColor;                 //   colours for faking a 1bpp XLATEOBJ
ULONG   iForeColor;
} XLATECOLORS;                          /* xlc, pxlc */

BOOL bEnableText(PDEV*);
VOID vDisableText(PDEV*);
VOID vAssertModeText(PDEV*, BOOL);

VOID vFastText(GLYPHPOS*, ULONG, BYTE*, ULONG, ULONG, RECTL*, RECTL*,
               FLONG, RECTL*, RECTL*);
VOID vClearMemDword(ULONG*, ULONG);

//////////////////////////////////////////////////////////////////////
// Dither stuff

// Describes a single colour tetrahedron vertex for dithering:

typedef struct _VERTEX_DATA {
    ULONG ulCount;              // Number of pixels in this vertex
    ULONG ulVertex;             // Vertex number
} VERTEX_DATA;                      /* vd, pv */

VERTEX_DATA*    vComputeSubspaces(ULONG, VERTEX_DATA*);
VOID            vDitherColor(ULONG*, VERTEX_DATA*, VERTEX_DATA*, ULONG);

//////////////////////////////////////////////////////////////////////
// Brush stuff

// 'Slow' brushes are used when we don't have hardware pattern capability,
// and we have to handle patterns using screen-to-screen blts:

#define SLOW_BRUSH_CACHE_DIM    3   // Controls the number of brushes cached
                                    //   in off-screen memory, when we don't
                                    //   have the S3 hardware pattern support.
                                    //   We allocate 3 x 3 brushes, so we can
                                    //   cache a total of 9 brushes:
#define SLOW_BRUSH_COUNT        (SLOW_BRUSH_CACHE_DIM * SLOW_BRUSH_CACHE_DIM)
#define SLOW_BRUSH_DIMENSION    64  // After alignment is taken care of,
                                    //   every off-screen brush cache entry
                                    //   will be 64 pels in both dimensions
#define SLOW_BRUSH_ALLOCATION   (SLOW_BRUSH_DIMENSION + 8)
                                    // Actually allocate 72x72 pels for each
                                    //   pattern, using the 8 extra for brush
                                    //   alignment

// 'Fast' brushes are used when we have hardware pattern capability:

#define FAST_BRUSH_COUNT        16  // Total number of non-hardware brushes
                                    //   cached off-screen
#define FAST_BRUSH_DIMENSION    8   // Every off-screen brush cache entry
                                    //   is 8 pels in both dimensions
#define FAST_BRUSH_ALLOCATION   8   // We have to align ourselves, so this is
                                    //   the dimension of each brush allocation

// Common to both implementations:

#define RBRUSH_2COLOR           1   // For RBRUSH flags

#define TOTAL_BRUSH_COUNT       max(FAST_BRUSH_COUNT, SLOW_BRUSH_COUNT)
                                    // This is the maximum number of brushes
                                    //   we can possibly have cached off-screen
#define TOTAL_BRUSH_SIZE        64  // We'll only ever handle 8x8 patterns,
                                    //   and this is the number of pels

typedef struct _BRUSHENTRY BRUSHENTRY;

// NOTE: Changes to the RBRUSH or BRUSHENTRY structures must be reflected
//       in strucs.inc!

typedef struct _RBRUSH {
    FLONG       fl;             // Type flags
    BOOL        bTransparent;   // TRUE if brush was realized for a transparent
                                //   blt (meaning colours are white and black),
                                //   FALSE if not (meaning it's already been
                                //   colour-expanded to the correct colours).
                                //   Value is undefined if the brush isn't
                                //   2 colour.
    ULONG       ulForeColor;    // Foreground colour if 1bpp
    ULONG       ulBackColor;    // Background colour if 1bpp
    POINTL      ptlBrushOrg;    // Brush origin of cached pattern.  Initial
                                //   value should be -1
    BRUSHENTRY* apbe[MAX_BOARDS];// Points to brush-entry that keeps track
                                //   of the cached off-screen brush bits
    ULONG       aulPattern[1];  // Open-ended array for keeping copy of the
      // Don't put anything     //   actual pattern bits in case the brush
      //   after here, or       //   origin changes, or someone else steals
      //   you'll be sorry!     //   our brush entry (declared as a ULONG
                                //   for proper dword alignment)

} RBRUSH;                           /* rb, prb */

typedef struct _BRUSHENTRY {
    RBRUSH*     prbVerify;      // We never dereference this pointer to
                                //   find a brush realization; it is only
                                //   ever used in a compare to verify
                                //   that for a given realized brush, our
                                //   off-screen brush entry is still valid.
    LONG        x;              // x-position of cached pattern
    LONG        y;              // y-position of cached pattern

} BRUSHENTRY;                       /* be, pbe */

typedef union _RBRUSH_COLOR {
    RBRUSH*     prb;
    ULONG       iSolidColor;
} RBRUSH_COLOR;                     /* rbc, prbc */

#ifdef PATTERN_JUNK
BOOL bEnableBrushCache(PDEV*);
VOID vDisableBrushCache(PDEV*);
VOID vAssertModeBrushCache(PDEV*, BOOL);
#endif

//////////////////////////////////////////////////////////////////////
// Stretch stuff

typedef struct _STR_BLT {
    PDEV*   ppdev;
    PBYTE   pjSrcScan;
    LONG    lDeltaSrc;
    LONG    XSrcStart;
    PBYTE   pjDstScan;
    LONG    lDeltaDst;
    LONG    XDstStart;
    LONG    XDstEnd;
    LONG    YDstStart;
    LONG    YDstCount;
    ULONG   ulXDstToSrcIntCeil;
    ULONG   ulXDstToSrcFracCeil;
    ULONG   ulYDstToSrcIntCeil;
    ULONG   ulYDstToSrcFracCeil;
    ULONG   ulXFracAccumulator;
    ULONG   ulYFracAccumulator;
} STR_BLT;

typedef VOID (*PFN_DIRSTRETCH)(STR_BLT*);

VOID vDirectStretch8Narrow(STR_BLT*);
VOID vDirectStretch8(STR_BLT*);
VOID vDirectStretch16(STR_BLT*);
VOID vDirectStretch32(STR_BLT*);

#if USE_NT5_DDMEMMGR

/////////////////////////////////////////////////////////////////////////
// Heap stuff

typedef struct _DSURF DSURF;

typedef enum {
    DT_SCREEN       = 0x0,  // Surface is kept in screen memory
    DT_DIB          = 0x1,  // Surface is really a DIB, not in off-screen
                            //   memory
    DT_DIRECTDRAW   = 0x2,  // Surface is really a DirectDraw surface

} DSURFTYPE;                    /* dt, pdt */

typedef struct _DSURF
{
    DSURFTYPE dt;           // DSURF status flags
    PDEV*     ppdev;        // Points to our PDEV
#if !ENABLE_RECONFIG_VIDMEM
    LONG      x;            // x pixel coordinate of left edge of allocation
                            //   if not DT_DIB
    LONG      y;            // y pixel coordinate of right edge of allocation
                            //   if not DT_DIB
#endif
    LONG      cx;           // Bitmap width in pixels
    LONG      cy;           // Bitmap height in pixels
    union {
        FLATPTR   fpVidMem; // Offset from start of video-memory if not DT_DIB
        VOID*     pvScan0;  // Bits location in system-memory if DT_DIB
    };
    LONG      lDelta;       // Distance from one scan to the next.
    VIDEOMEMORY*  pvmHeap;  // DirectDraw heap this was allocated from if
                            //   not DT_DIB and not DT_DIRECTDRAW
    DSURF*    pdsurfDiscardableNext;
                            // Linked list of discardable surface allocations.
                            //   This list is traversed from the start to
                            //   throw out any allocations
    HSURF     hsurf;        // Handle to associated GDI surface (if any)

    // The following are used for DT_DIB only...

    ULONG     cBlt;         // Counts down the number of blts necessary at
                            //   the current uniqueness before we'll consider
                            //   putting the DIB back into off-screen memory
    ULONG     iUniq;        // Tells us whether there have been any heap
                            //   'free's since the last time we looked at
                            //   this DIB

} DSURF;                          /* dsurf, pdsurf */

// Number of blts necessary before we'll consider putting a DIB DFB back
// into off-screen memory:

#define HEAP_COUNT_DOWN     6

DSURF* pVidMemAllocate(PDEV*, LONG, LONG, DSURF*);
VOID vVidMemFree(DSURF*);
BOOL bMoveDibToOffscreenDfbIfRoom(PDEV*, DSURF*);
BOOL bMoveOldestOffscreenDfbToDib(PDEV*);
BOOL bMoveOffscreenDfbInHeapToDib(PDEV*, VIDEOMEMORY *, DWORD, DWORD);
BOOL bMoveAllDfbsFromOffscreenToDibs(PDEV* ppdev);

BOOL bEnableOffscreenHeap(PDEV*);
VOID vDisableOffscreenHeap(PDEV*);
BOOL bAssertModeOffscreenHeap(PDEV*, BOOL);

#else

/////////////////////////////////////////////////////////////////////////
// Heap stuff

typedef enum {
    OH_FREE = 0,        // The off-screen allocation is available for use
    OH_DISCARDABLE,     // The allocation is occupied by a discardable bitmap
                        //   that may be moved out of off-screen memory
    OH_PERMANENT,       // The allocation is occupied by a permanent bitmap
                        //   that cannot be moved out of off-screen memory
} OHSTATE;

typedef struct _DSURF DSURF;
typedef struct _OH OH;
typedef struct _OH
{
    OHSTATE  ohState;       // State of off-screen allocation
    LONG     x;             // x-coordinate of left edge of allocation
    LONG     y;             // y-coordinate of top edge of allocation
    LONG     cx;            // Width in pixels of allocation
    LONG     cy;            // Height in pixels of allocation
    LONG     cxReserved;    // Dimensions of original reserved rectangle;
    LONG     cyReserved;    //   zero if rectangle is not 'reserved'
    OH*      pohNext;       // When OH_FREE or OH_RESERVE, points to the next
                            //   free node, in ascending cxcy value.  This is
                            //   kept as a circular doubly-linked list with a
                            //   sentinel at the end.
                            // When OH_DISCARDABLE, points to the next most
                            //   recently created allocation.  This is kept as
                            //   a circular doubly-linked list.
    OH*      pohPrev;       // Opposite of 'pohNext'
    ULONG    cxcy;          // Width and height in a dword for searching
    OH*      pohLeft;       // Adjacent allocation when in-use or available
    OH*      pohUp;
    OH*      pohRight;
    OH*      pohDown;
    DSURF*   pdsurf;        // Points to our DSURF structure
    VOID*    pvScan0;       // Points to start of first scan-line
};                              /* oh, poh */

// This is the smallest structure used for memory allocations:

typedef struct _OHALLOC OHALLOC;
typedef struct _OHALLOC
{
    OHALLOC* pohaNext;
    OH       aoh[1];
} OHALLOC;                      /* oha, poha */

typedef struct _HEAP
{
    LONG     cxMax;         // Largest possible free space by area
    LONG     cyMax;
    OH       ohFree;        // Head of the free list, containing those
                            //   rectangles in off-screen memory that are
                            //   available for use.  pohNext points to
                            //   hte smallest available rectangle, and pohPrev
                            //   points to the largest available rectangle,
                            //   sorted by cxcy.
    OH       ohDiscardable; // Head of the discardable list that contains all
                            //   bitmaps located in offscreen memory that
                            //   are eligible to be tossed out of the heap.
                            //   It is kept in order of creation: pohNext
                            //   points to the most recently created; pohPrev
                            //   points to the least recently created.
    OH       ohPermanent;   // List of permanently allocated rectangles
    OH*      pohFreeList;   // List of OH node data structures available
    OHALLOC* pohaChain;     // Chain of allocations
} HEAP;                         /* heap, pheap */

typedef enum {
    DT_SCREEN,              // Surface is kept in screen memory
    DT_DIB                  // Surface is kept as a DIB
} DSURFTYPE;                    /* dt, pdt */

typedef struct _DSURF
{
    DSURFTYPE dt;           // DSURF status (whether off-screen or in a DIB)
    SIZEL     sizl;         // Size of the original bitmap (could be smaller
                            //   than poh->sizl)
    PDEV*     ppdev;        // Need this for deleting the bitmap
    union {
        OH*         poh;    // If DT_SCREEN, points to off-screen heap node
        SURFOBJ*    pso;    // If DT_DIB, points to locked GDI surface
    };

    // The following are used for DT_DIB only...

    ULONG     cBlt;         // Counts down the number of blts necessary at
                            //   the current uniqueness before we'll consider
                            //   putting the DIB back into off-screen memory
    ULONG     iUniq;        // Tells us whether there have been any heap
                            //   'free's since the last time we looked at
                            //   this DIB
} DSURF;                          /* dsurf, pdsurf */

// GDI expects dword alignment for any bitmaps on which it is expected
// to draw.  Since we occasionally ask GDI to draw directly on our off-
// screen bitmaps, this means that any off-screen bitmaps must be dword
// aligned in the frame buffer.  We enforce this merely by ensuring that
// all off-screen bitmaps are four-pel aligned (we may waste a couple of
// pixels at the higher colour depths):

#define HEAP_X_ALIGNMENT    4

// Number of blts necessary before we'll consider putting a DIB DFB back
// into off-screen memory:

#define HEAP_COUNT_DOWN     6

// Flags for 'pohAllocate':

typedef enum {
    FLOH_ONLY_IF_ROOM       = 0x0001,   // Don't kick stuff out of off-
                                        //   screen memory to make room
    FLOH_MAKE_PERMANENT     = 0x0002,   // Allocate a permanent entry
    FLOH_RESERVE            = 0x0004,   // Allocate an off-screen entry,
                                        //   but let it be used by discardable
                                        //   bitmaps until it's needed
} FLOH;

// Publicly callable heap APIs:

OH*  pohAllocate(PDEV*, POINTL*, LONG, LONG, FLOH);
BOOL bOhCommit(PDEV*, OH*, BOOL);
OH*  pohFree(PDEV*, OH*);

OH*  pohMoveOffscreenDfbToDib(PDEV*, OH*);
BOOL bMoveDibToOffscreenDfbIfRoom(PDEV*, DSURF*);
BOOL bMoveAllDfbsFromOffscreenToDibs(PDEV* ppdev);

BOOL bEnableOffscreenHeap(PDEV*);
VOID vDisableOffscreenHeap(PDEV*);
BOOL bAssertModeOffscreenHeap(PDEV*, BOOL);

#endif

/////////////////////////////////////////////////////////////////////////
// Command Fifo stuff

#define HW_CMDFIFO_TOTAL_SIZE   0x10000 // Total size in bytes reserved
                                        //   for the command fifo
                                        //   (must be a power of 2).

/* We claim space at the end of the fifo for:
 *   1 nop (1 32-bit word)
 *   1 jmp (2 32-bit words)
 */
#define H3_FIFO_END_ADJUST  (3 * sizeof(ULONG))


/////////////////////////////////////////////////////////////////////////
// Pointer stuff

#define HW_POINTER_DIMENSION    64      // Maximum dimension of default
                                        //   (built-in) hardware pointer
#define HW_POINTER_TOTAL_SIZE   1024    // Total size in bytes required
                                        //   to define the hardware pointer
                                        //   (must be a power of 2 for
                                        //   allocating space for the shape)


typedef VOID (FNSHOWPOINTER)(PDEV*, VOID*, BOOL);
typedef VOID (FNMOVEPOINTER)(PDEV*, VOID*, LONG, LONG);
typedef BOOL (FNSETPOINTERSHAPE)(PDEV*, VOID*, LONG, LONG, LONG, LONG, LONG,
                                 LONG, BYTE*);
typedef VOID (FNENABLEPOINTER)(PDEV*, VOID*, BOOL);

BOOL bEnablePointer(PDEV*);
VOID vDisablePointer(PDEV*);
VOID vAssertModePointer(PDEV*, BOOL);

/////////////////////////////////////////////////////////////////////////
// Palette stuff

BOOL bEnablePalette(PDEV*);
VOID vDisablePalette(PDEV*);
VOID vAssertModePalette(PDEV*, BOOL);

BOOL bInitializePalette(PDEV*, DEVINFO*);
VOID vUninitializePalette(PDEV*);

#define MAX_CLUT_SIZE (sizeof(VIDEO_CLUT) + (sizeof(ULONG) * 256))

/////////////////////////////////////////////////////////////////////////
// DirectDraw stuff

// There's a 64K granularity that applies to the mapping of the frame
// buffer into the application's address space:

#define ROUND_UP_TO_64K(x)  (((ULONG)(x) + 0x10000 - 1) & ~(0x10000 - 1))

#if 0
typedef struct _FLIPRECORD
{
    FLATPTR         fpFlipFrom;        // Identifies the surface which was
                                       //   'flipped from'.  We have to prevent
                                       //   all drawing to this surface until we
                                       //   know that a vertical retrace has
                                       //   happened since the command was given,
                                       //   so it's now non-visible and thus safe
                                       //   to draw on.
    LONGLONG        liFlipTime;        // Time at which last flip
                                       //   occured
    LONGLONG        liFlipDuration;    // Precise amount of time it
                                       //   takes from vblank to vblank
    DWORD           dwScanLine;        // Scan line that was current the last time
                                       //   we sampled whether the flip was
                                       //   complete
    BOOL            bFlipFlag;         // True if we think a flip is
                                       //   still pending
} FLIPRECORD;
#endif

BOOL bEnableDirectDraw(PDEV*);
VOID vDisableDirectDraw(PDEV*);
VOID vAssertModeDirectDraw(PDEV*, BOOL);

//////////////////////////////////////////////////////////////////////
// Low-level blt function prototypes

typedef VOID (FNFILL)(PDEV*, LONG, RECTL*, ULONG, RBRUSH_COLOR, POINTL*);
typedef VOID (FNXFER)(PDEV*, LONG, RECTL*, ULONG, SURFOBJ*, POINTL*,
                      RECTL*, XLATEOBJ*);
typedef VOID (FNPATXFER)(PDEV*, LONG, RECTL*, ULONG, SURFOBJ*, POINTL*,
                      RECTL*, XLATEOBJ*, RBRUSH_COLOR, POINTL*, BOOL);
typedef VOID (FNCOPY)(PDEV*, LONG, RECTL*, ULONG, POINTL*, RECTL*);
typedef VOID (FNPATCOPY)(PDEV*, LONG, RECTL*, ULONG, POINTL*, RECTL*, RBRUSH_COLOR, POINTL*, BOOL);
typedef VOID (FNFASTPATREALIZE)(PDEV*, RBRUSH*, POINTL*, BOOL);
typedef VOID (FNIMAGETRANSFER)(PDEV*, BYTE*, LONG, LONG, LONG, ULONG);
typedef BOOL (FNTEXTOUT)(SURFOBJ*, STROBJ*, FONTOBJ*, CLIPOBJ*, RECTL*,
                         BRUSHOBJ*, BRUSHOBJ*);
typedef VOID (FNLINETOTRIVIAL)(PDEV*, LONG, LONG, LONG, LONG, ULONG, MIX);
typedef VOID (FNLINETOCLIPPED)(PDEV*, LONG, LONG, LONG, LONG, ULONG, MIX, RECTL*);

FNXFER              vXferNativeSrccopy;

FNFILL              vMmFillPatFast;
FNFILL              vMmFillPatSlow;
FNFILL              vMmFillSolid;
FNXFER              vMmXfer1bpp;
FNXFER              vXfer4bpp;
FNXFER              vXfer8bpp;
FNXFER              vXfer162432bpp;
FNXFER              vMmXferNative;
FNCOPY              vMmCopyBlt;
FNPATXFER           vPatXfer1bpp;
FNPATXFER           vPatXfer4bpp;
FNPATXFER           vPatXfer8bpp;
FNPATXFER           vPatXferNative;
FNPATCOPY           vPatCopyBlt;
FNFASTPATREALIZE    vMmFastPatRealize;
FNIMAGETRANSFER     vMmImageTransferMm16;
FNIMAGETRANSFER     vMmImageTransferMm32;
FNTEXTOUT           bMmTextOut;

FNLINETOTRIVIAL     vLineToTrivial;
FNLINETOCLIPPED     vLineToClipped;

VOID vPutBits(PDEV*, SURFOBJ*, RECTL*, POINTL*);
VOID vGetBits(PDEV*, SURFOBJ*, RECTL*, POINTL*);

VOID vH3LoadPatternRegisters( PDEV*, RBRUSH*, BOOL);
VOID vH3LoadPatternSolid( PDEV*, ULONG, BOOL);


////////////////////////////////////////////////////////////////////////
// Capabilities flags
//
// These are private flags passed to us from the Banshee miniport.  They
// come from the 'DriverSpecificAttributeFlags' field of the
// 'VIDEO_MODE_INFORMATION' structure (found in 'ntddvdeo.h') passed
// to us via an 'VIDEO_QUERY_AVAIL_MODES' or 'VIDEO_QUERY_CURRENT_MODE'
// IOCTL.
//
// NOTE: These definitions must match those in the miniport's 'h3.h'!

typedef enum {
    CAPS_SW_POINTER         = 0x00000001,   // No hardware pointer; use software pointer instead
    CAPS_AGP_FIFO           = 0x00000002,   // Use AGP command fifo
} CAPS;

// DIRECT_ACCESS(ppdev) returns TRUE if GDI and DCI can directly access
// the frame buffer.  It returns FALSE if there are hardware bugs
// when reading words or dwords from the frame buffer that cause non-x86
// systems to crash.  It will also return FALSE is the Alpha frame buffer
// is mapped in using 'sparse space'.

#define DIRECT_ACCESS(ppdev)    1

////////////////////////////////////////////////////////////////////////
// Status flags

typedef enum {
    STAT_GLYPH_CACHE        = 0x0001,   // Glyph cache successfully allocated
    STAT_BRUSH_CACHE        = 0x0002,   // Brush cache successfully allocated
    STAT_DIRECTDRAW         = 0x0004,   // DirectDraw is currently enabled
    //STAT_FIFO_IN_USE        = 0x8000,   // cmdfifo is in use
} STATUS;

////////////////////////////////////////////////////////////////////////
// Command fifo stuff

#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)

#define CMDFIFOUNBUMPEDWORDS  (ppdev->fifoData.unbumpedWords)
#define CMDFIFOEPILOGPTR      (ppdev->fifoData.epilogPtr)
#define CMDFIFOSTART          (ppdev->fifoData.fifoStart)

#endif

typedef struct
{
    ULONG*            fifoPtr;          // Current write pointer into fifo
    LONG              fifoRoom;         // Space in DWORDS until next fifo check
    ULONG             fifoLastRead;     // Last known hw read ptr

    /* Basic command fifo characteristics. These should be
     * considered logically const after their initialization.
     */
    BYTE*             fifoStart;        // Virtual address of start of fifo
    BYTE*             fifoEnd;          // Virtual address of fba fifo
    ULONG             fifoOffset;       // Offset from fb base to fifo start
    ULONG             fifoSize;         // Size in bytes of the fifo
    ULONG             fifoJmpHdr;       // Type0 packet for jmp to fifo start
#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
    ULONG             fifoJmpHdr2;      // AGP fifo needs extra jmp header word
    ULONG             unbumpedWords;
    ULONG             epilogPtr;
#endif

    /* Fifo checking information. In units of usuable bytes until
     * the appropriate condition.
     */
    LONG              roomToReadPtr;    // Last known hw ptr
    LONG              roomToEnd;        // # of bytes until last usable address before fifoEnd

#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
    // Physical address of AGP command fifo

    ULONG             physBaseL;
    ULONG             physBaseH;
#endif

#if DBG
    LONG              writesSinceHeader;// Debug variable, count of WSH.
    LONG              numRegsInPacket;  // Debug variable.
#endif

    ULONG             ulCmdFifoBump; // IGX-NVH 01.07.00

} FIFO_DATA;

//
// Physical address definition.  From miniport.h
//

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS; // windbgkd

// AGP info structure - Must match miniports structure.

typedef struct
{
	PHYSICAL_ADDRESS    physAddr;   // Physical address of AGP command fifo
	PVOID               PhysicalReserveContext;
	PVOID               virtualAddr;
	PVOID               VirtualReserveContext;
	ULONG               agpFifoSizeInB;
	
} H3_AGP_INFO;


////////////////////////////////////////////////////////////////////////
// FXSURFACEDATA structure shared with Win9x ddraw

// defines for dwSurfaceFlags
#define FX_ORIGINALLY_Z16   0x00000001
#define FX_ORIGINALLY_Z32   0x00000002

#ifdef USE_DDRAW_CODE
typedef struct {
  DWORD         hwPtr;
  DWORD         lfbPtr;
  DWORD         overlayShrinkFlag;
  DWORD         doShrink;
  DWORD         shrinkWidth;
  DWORD         shrinkHeight;
  DWORD         pixelFormat;
  DWORD         inFlipChain;
  DWORD         overlayAddr;
#if (USE_NT5_DDMEMMGR)
  DWORD         shrinkSrcY;
  DWORD         shrinkSrcX;
  DWORD         shrinkSrcWidth;
  DWORD         shrinkSrcHeight;
#endif
  DWORD         lPitch;
  DWORD         heapID;
  VIDEOMEMORY   *pvmHeap;
#ifdef SLI_AA
  DWORD         AAhwPtr;
  DWORD         AAlfbPtr;
  DWORD         AAlPitch;
  DWORD         AAheapID;
  VIDEOMEMORY   *AApvmHeap;
#endif
  DWORD         dwSurfaceFlags;
} FXSURFACEDATA;

// defines for ddMiscFlags bit flags
#define DDMF_VSYNC_POLARITY_BIT   6

#define DDMF_TRIPPLE_BUFFER       BIT(4)
#define DDMF_VSYNC_POLARITY_MASK  BIT(DDMF_VSYNC_POLARITY_BIT)

#if (_WIN32_WINNT >= 0x0500)
#define KMVT        1
//Buffer for KMVT
// Note:  If you modify anything in this structure
//        you need to change the same sturcture in
//        h3.h in miniport driver.
//

typedef struct{
    ULONG dwSize;
    ULONG dwStatus;
    ULONG dwVPAddr0;
    ULONG dwVPAddr1;
    ULONG dwVPAddr2;
    ULONG dwVPStride;
    ULONG dwOVLOffset;
    ULONG dwOldBuf;
    DWORD dwVBILines;

	// V3TV
    ULONG fpVidMem[3];
    ULONG dwVideoLines;			// video data
	ULONG dwVideoLinesOrig;
    ULONG dwVideoSurfacePitch;	// surface pitch
	ULONG dwVBILinesOrig;
    ULONG dwVidInFormat;
    ULONG dwCounter;
	ULONG dwDataWidth;
	ULONG dwDataHeight;

	//PLD 656 variables
    BOOLEAN bVMIPLDinUse;
    BOOLEAN bLastFieldEven;
    BOOLEAN bLastIntrVBI;
    BOOLEAN bVBIcropped;
    ULONG dwLastBufferFilled;
    BOOLEAN  bSawVBIintr;

	// other need info
	BOOLEAN  bDoublePitch;
	ULONG dwPLDVidLines;
	BOOLEAN  bUsingEAV;
	BOOLEAN  bUseReset;

	//V3TV
    //UCHAR     bSkipNextField;
    BOOLEAN	  bInitialized;
    UCHAR	  fTransferFull;
    ULONG	  dwCurrentTransfer;
    ULONG     dwQueuedTransfers;
    ULONG	  dwBusMasterTransfers;
    ULONG	  dwFieldCount;
	BOOLEAN	  bReportForVideo;
	UCHAR	  bFirstFieldBuffer;
	ULONG	  dwIMask;	
	BOOLEAN	  fWDMActive;
	ULONG     decoderWidth;
	ULONG	  dwResetCounter;
} KMVTBUFF;

//value for dwStatus
#define BUFF1_IN_USE    0x0000
#define BUFF2_IN_USE    0x0001
#define BUFF3_IN_USE    0x0002
#define BUFFER_IN_USE_MASK    0x3
#define ONE_BUFFER      0x0100
#define TWO_BUFFERS     0x0200
#define THREE_BUFFERS   0x0300
#define VP_BUFF_MASK    0x0300
#define VP_BUFF_SHIFT   8

#define BOB_ON          0x10000
#define INTERLEAVE_ON   0x20000

#endif

////////////////////////////////////////////////////////////////////////
// GLOBALDATA structure shared with Win9x ddraw

typedef struct _DDGLOBAL  DDGLOBAL;
typedef struct _FXGLOBAL  fxGlobal;
typedef struct _D3GLOBAL  d3Global;

#ifdef SLI_AA
#define MEM_DECODE_SIZE                 128*1024*1024   // 128MB
#define IO_DECODE_SIZE                  256

#define SLI_AA_REGULAR_DEVICE           (0x00)
#define SLI_AA_MASTER_DEVICE            (0x01)
#define SLI_AA_SLAVE_DEVICE             (0x02)
#define HWINFO_SST_MAX_NUM_CHIPS        (0x04)

// index in HwDeviceExtension->sliMappedAddress[][these_defines]
#define HWINFO_SST_IOREGS_INDEX         (0x00)
#define HWINFO_SST_CMDFIFOREGS_INDEX    (0x01)
#define HWINFO_SST_2DREGS_INDEX         (0x02)
#define HWINFO_SST_3DREGS_INDEX         (0x03)
#define HWINFO_SST_FB_INDEX             (0x04)
#define HWINFO_SST_IO_INDEX             (0x05)
#define HWINFO_SST_MAX_CHIP_INDEX       (0x06)
#endif

#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
typedef struct
{
  DWORD   ddPtr;
  LPVOID  pvmHeap;
} DRIVER_DD_HEAP_INFO;
#endif

typedef struct
{
#if ENABLE_RECONFIG_VIDMEM
  DWORD               TotalVRAM;
#endif
  DWORD               memclk;                   // memory clock in MHz

#ifdef SLI_AA
  DWORD               dwType;                   // Master or Slave
//  DWORD               dwSwitchCursor;           // Call at to Promote to Overlay Time to use Host Base Cursor
//#ifdef RD_ABORT_ERROR
//  DWORD               dwSLIMode;                // State of SLI, Enabled, Disabled, Not in Use
//#endif
#endif
  DWORD               dwNumUnits;               // Number of Master + Slaves

  DWORD               refresh;
  DWORD               pitch;

#if USE_D3D_CODE && (_WIN32_WINNT >= 0x0500)
  DWORD               ModeUses2PixPerClkRender; // This is set at modeset time.
#endif

  DWORD               stretchBltStart;
  DWORD               stretchBltSize;

#if ENABLE_RECONFIG_VIDMEM
  DWORD               ddLinearHeapStart;
  DWORD               ddLinearHeapSize;

  DWORD               ddSecondaryHeapStart;
  DWORD               ddSecondaryHeapSize;

  DWORD               ddTiledHeapStart;
  DWORD               ddTiledHeapSize;

  DWORD               gdiDesktopStart;
  DWORD               gdiDesktopSize;

  DWORD               ddNumColorBuff;
#endif

#if (_WIN32_WINNT >= 0x0500)
  DWORD               ddExclusiveMode;
  DWORD               ddPrimaryInTile;

  DWORD               ddLinearHeap1Start;
  DWORD               ddLinearHeap2Start;
  DWORD               ddLinearHeap3Start;
  DWORD               ddLinearHeap4Start;

  DWORD               ddTiledHeap0Start;
  DWORD               ddTiledHeap1Start;
  DWORD               ddTiledHeap2Start;

  DWORD               secondaryFrontBuffer;
  DWORD               secondaryBackBuffer;
  DWORD               secondaryZBuffer;
  DWORD               secondaryThirdBuffer;

#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
  DWORD               bUseSliExtraLinearHeap;
  DWORD               sliFrontBuffer;
  DWORD               sliBackBuffer;
  DWORD               sliZBuffer;
  DWORD               sliThirdBuffer;

  DWORD               sliTileCtrl;
  DWORD               sliTileCompare;

  DRIVER_DD_HEAP_INFO sliExtraHeap;
#endif
#endif

#if USE_D3D_CODE
  DWORD               dd3DSurfaceCount;
  DWORD               ddTileStride;
  DWORD               ddTileHeight;
  DWORD               ddTilePitch;
  DWORD               ddTileMark;
#endif
  DWORD               lastOverlayAddress;
  DWORD               ddSwapCount;

#ifdef SLI_AA
  DWORD               regBase[HWINFO_SST_MAX_NUM_CHIPS * HWINFO_SST_MAX_CHIP_INDEX];
  DWORD               ddMultiChipConfig;
#endif

#if 0
  //
  // gdiFlags is a GDI driver entrypoint "fast path" check
  // if gdiFlags is zero, then we need not be concerned with things like
  // restoring invariant 2d registers, worrying about s/w cursors, etc.
  // + DD will set the SDATA_GDIFLAGS_2D_DIRTY flag when it clobbers any of
  //   gdi's invariant 2d registers
  // + GDI will set SDATA_GDIFLAGS_SW_CURSOR if there's a s/w cursor enabled
  //
  DWORD               gdiFlags;       // for gdi fast-path test

  // DD and D3D status & h/w state flags
  // + when glide changes 3D state, D3D needs to resend its state
  // + when D3D changes state, glide command buffer flushes need to
  //      be prefaced with the required state
  // actually implemented this way?  perhaps just placing a direct pointer
  // into d3d's saved "last context" and invalidating that will do all
  // that we want...? -KMW
  DWORD               ddFlags;        // need this?
#endif
  DWORD               d3dFlags;       // need this?
  #define D3DF_UPDATED  1             // d3d updated hw state

#if USE_D3D_CODE && (_WIN32_WINNT >= 0x0500)
  DWORD               pD3colbuff;
  DWORD               pD3context;
  DWORD               pD3auxbuff;
  DWORD               pD3changed;
#endif

  DWORD               ddRunTimeVersion;
  DWORD               ddMiscFlags;

  //
  // ddraw, d3d internal, adapter-specific global info.
  //
#if 0
  DWORD               pddglobal;
  DWORD               pfxglobal;
  DWORD               pd3global;
#else
  DDGLOBAL            *pddglobal;
  fxGlobal            *pfxglobal;
  d3Global            *pd3global;
#endif

#if 0
  // Memory management global info.
  DWORD               mmEvictAddr;
  DWORD               mmReclaimAddr;
  DWORD               mmFlags;
#endif

#ifdef CSIM
  DWORD               fakeVerticalBlank;
#endif

  DWORD               dwOvlOffset;

  DWORD               dwRBitMask;
  DWORD               dwGBitMask;
  DWORD               dwBBitMask;
#if 0
  DWORD               fReset;

  FLIPRECORD          flipRecord;     // Used to track vertical blank status
#endif

  FXSURFACEDATA       ddPrimarySurfaceData;
#if 0
  DWORD               ddHasShrinkBuffer;
#endif
  FXSURFACEDATA       ddOverlayShrinkSurfaceData;
#if 0
  DWORD               ddShrinkBufferFree;
  DWORD               ddOverlayShrinkPitch;
#endif
  DWORD               ddVisibleOverlaySurf;
#if (USE_NT5_DDMEMMGR)
  DWORD               dwCurrentSKSurf;
  DWORD               dwShrinkSurfAddr1;
  DWORD               dwShrinkSurfAddr2;
  DWORD               dwTxtrSurfAddr1;
#endif
#ifdef KMVT
  KMVTBUFF             * KMVTBuff;  //KMVTBUFF is allocated in miniport dirver
                                    //it is used for both KMVT functions and IRQ
#endif
  DWORD                bVMIPLDpresent; //PLD is present or not
#if defined(DEBUGFIFO) && defined(DBG) && defined(H3_FIFO)  // Fifo debug stuff
  DWORD               fifoDwordCount;
  DWORD               InPacket;       // =0 expecting header, =1 not expecting hdr
  DWORD               WordsLeftInPacket;  // # of words left in packet, counting header
  DWORD               CurrPH;         // Current packet header in progress
  DWORD               currPHHwPtr;
  DWORD               currPHData;
  DWORD               LastPH;         // Last packet header written
  DWORD               lastPHHwPtr;
  DWORD               lastPHData;
#endif
} GLOBALDATA;

#endif // USE_DDRAW_CODE

typedef enum {
	GAMMA_DESKTOP	= 0x0000,
	GAMMA_GLIDE		= 0x0001,
	GAMMA_TRANSIENT = 0x0002,
	GAMMA_INVALID
} GAMMA_STATE;

#define TVOUT_SUPPORTED 1
#ifdef TVOUT_SUPPORTED
typedef enum _vmi_config         //TV-OUT: values for dwVmiTv in global data 
{
   H3VMI_NOT_USED,               //VMI not used
   H3VMI_VIDEO_IN,               //VMI used for video-in
   H3VMI_TV_OUT                  //VMI used for tv-out
   
} VmiConfig;
#endif //def TVOUT_SUPPORTED

////////////////////////////////////////////////////////////////////////
// The Physical Device data structure

typedef struct  _PDEV
{
    // -------------------------------------------------------------------
    // NOTE: Changes between here and NOTE1 in the PDEV structure must be
    // reflected in i386\strucs.inc (assuming you're on an x86, of course)!

    LONG        xOffset;                // Pixel offset from (0, 0) to current
    LONG        yOffset;                //   DFB located in off-screen memory
    BYTE*       pjMmBase;               // Start of memory mapped I/O
    BYTE*       pjBase;                 // Board base for memory mapped I/O
    BYTE*       pjLfbBase;              // Board base for linear framebuffer
    BYTE*       pjH3Base;               // Start of 2D memory mapped I/O
    SstRegs*    p3DBase;                // Start of 3D memory mapped registers.
    BYTE*       pjCmdAgpBase;           // Start of commandAgp memory mapped I/O
#if REDUCED_MEMORY_MAPPINGS
    BYTE*       pjYUVPlanar;            // Start of YUV planar
#endif
    BYTE*       pjFifoBase;             // Start of command fifo memory mapped I/O
    BYTE*       pjScreenBase;           // Points to local video memory base address
    BYTE*       pjScreen;               // Start address of displayed screen
    ULONG       ulScreenOffset;         // Offset of displayed screen from FB base.
    LONG        lDelta;                 // Distance from one scan to the next.
    #if ENABLE_LINEAR_DFBS
    // NVH
    // Linear Device Bitmaps will most likely have a different pitch than
    // the frame buffer.
    ULONG       lDeltaSrc;
    ULONG       lDeltaDst;
    ULONG       fpVidMemSrc;
    ULONG       fpVidMemDst;
    #endif
    LONG        cjPelSize;              // 1 if 8bpp, 2 if 16bpp, 3 if 24bpp,
                                        //   4 if 32bpp
    ULONG       iBitmapFormat;          // BMF_8BPP or BMF_16BPP or BMF_32BPP
                                        //   (our current colour depth)
    LONG        iBoard;                 // Logical multi-board identifier
                                        //   (zero by default)

    // -------------------------------------------------------------------
    // NOTE1: Changes up to here in the PDEV structure must be reflected in
    // i386\strucs.inc (assuming you're on an x86, of course)!

    CAPS        flCaps;                 // Capabilities flags
    STATUS      flStatus;               // Status flags
    BOOL        bEnabled;               // In graphics mode (not full-screen)

    HANDLE      hDriver;                // Handle to \Device\Screen
    HDEV        hdevEng;                // Engine's handle to PDEV
    HSURF       hsurfScreen;            // Engine's handle to screen surface
    DSURF*      pdsurfScreen;           // Our private DSURF for the screen

    LONG        cxScreen;               // Visible screen width
    LONG        cyScreen;               // Visible screen height
    LONG        cxMemory;               // Width of Video RAM
    LONG        cyMemory;               // Height of Video RAM
    LONG        cBitsPerPel;            // Bits per pel (8, 15, 16, 24 or 32)
    ULONG       ulMode;                 // Mode the mini-port driver is in.
    ULONG		ulScreenFormat;         // Src/Dst format value for HW reg.

    FLONG       flHooks;                // What we're hooking from GDI
    ULONG       ulWhite;                // 0xff if 8bpp, 0xffff if 16bpp,
                                        //   0xffffffff if 32bpp
    UCHAR*      pjIoBase;               // Mapped IO port base for this PDEV
    VOID*       pvTmpBuffer;            // General purpose temporary buffer,
                                        //   TMP_BUFFER_SIZE bytes in size
                                        //   (Remember to synchronize if you
                                        //   use this for device bitmaps or
                                        //   async pointers)
    HSEMAPHORE  csCrtc;                 // Used for synchronizing access to
                                        //   the CRTC register

    ////////// Low-level blt function pointers:

    FNFILL*             pfnFillSolid;
    FNFILL*             pfnFillPat;
    FNXFER*             pfnXfer1bpp;
    FNXFER*             pfnXfer4bpp;
    FNXFER*             pfnXferNative;
    FNCOPY*             pfnCopyBlt;
    FNFASTPATREALIZE*   pfnFastPatRealize;

    ////////// Palette stuff:

    PALETTEENTRY* pPal;                 // The palette if palette managed
    HPALETTE    hpalDefault;            // GDI handle to the default palette.
    FLONG       flRed;                  // Red mask for 16/32bpp bitfields
    FLONG       flGreen;                // Green mask for 16/32bpp bitfields
    FLONG       flBlue;                 // Blue mask for 16/32bpp bitfields
    ULONG       cPaletteShift;          // number of bits the 8-8-8 palette must
                                        // be shifted by to fit in the hardware
                                        // palette.
    ////////// Heap stuff:

#if USE_NT5_DDMEMMGR
    VIDEOMEMORY* pvmList;               // Points to the video-memory heap list
                                        //   as supplied by DirectDraw, needed
                                        //   for heap allocations
    ULONG       cHeaps;                 // Count of video-memory heaps
#else
    HEAP        heap;                   // All our off-screen heap data
#endif
    ULONG       iHeapUniq;              // Incremented every time room is freed
                                        //   in the off-screen heap
    SURFOBJ*    psoPunt;                // Wrapper surface for having GDI draw
                                        //   on off-screen bitmaps
    SURFOBJ*    psoPunt2;               // Another one for off-screen to off-
                                        //   screen blts
#if USE_NT5_DDMEMMGR
    DSURF*      pdsurfDiscardableList;  // Linked list of discardable bitmaps,
                                        //   in order of oldest to newest
#else
    OH*         pohScreen;              // Off-screen heap structure for the
                                        //   visible screen
#endif

    ////////// Configuration stuff

    USHORT     usVendorID;
    USHORT     usDeviceID;
    ULONG      usChipRev;
	ULONG      usSubSystemID;

    ////////// Banking stuff:

    LONG        cjBank;                 // Size of a bank, in bytes

    ////////// Pointer stuff:

    BOOL        bHwPointerActive;       // Currently using the h/w pointer?
    LONG        xPointerHot;            // xHot of current hardware pointer
    LONG        yPointerHot;            // yHot of current hardware pointer

    LONG        cjPointerOffset;        // Byte offset from start of frame
                                        //   buffer to off-screen memory where
                                        //   we stored the pointer shape
    VOID*       pvPointerShape;         // Points to pointer shape

    LONG        xyPointer;              // Start xy-position for the current
                                        //   Banshee pointer

//    VOID*       pvPointerData;          // Points to ajPointerData[0]
//    BYTE        ajPointerData[POINTER_DATA_SIZE];
//                                        // Private work area for downloaded
//                                        //   miniport pointer code

//    FNSHOWPOINTER*      pfnShowPointer;
//    FNMOVEPOINTER*      pfnMovePointer;
//    FNSETPOINTERSHAPE*  pfnSetPointerShape;
//    FNENABLEPOINTER*    pfnEnablePointer;

    LONG        iBrushCache;            // Index for next brush to be allocated
    LONG        cBrushCache;            // Total number of brushes cached
    BRUSHENTRY  abe[TOTAL_BRUSH_COUNT]; // Keeps track of brush cache
    POINTL      ptlReRealize;           // Work area for 864/964 pattern
                                        //   hardware bug work-around

    ////////// Text stuff:

    SURFOBJ*    psoText;                // 1bpp surface to which we will have
                                        //   GDI draw our glyphs for us

    ULONG       ulGlyphCurrent;         // Linear address of next glyph to be
                                        //   cached in off-screen memory
    ULONG       ulGlyphStart;           // Linear address of start of glyph
                                        //   cache
    ULONG       ulGlyphEnd;             // Linear address of end of glyph
                                        //   cache
    CACHEDFONT  cfSentinel;             // Sentinel for the doubly-linked list
                                        //   we use to keep track of all
                                        //   cached font allocations
#if USE_NT5_DDMEMMGR
    LONG        cyText;                 // height of text cache
#endif

#if USE_DDRAW_CODE
    /////////// DirectDraw stuff:
    ULONG flip_pending; // NVH

#if 0
//    FLIPRECORD  flipRecord;             // Used to track vertical blank status
    ULONG       ulRefreshRate;          // Refresh rate in Hz
    ULONG       ulMinOverlayStretch;    // Minimum stretch ratio for this mode,
                                        //   expressed as a multiple of 1000
    ULONG       ulFifoValue;            // Optimial FIFO value for this mode
#endif
#if !USE_NT5_DDMEMMGR
    OH*         pohDirectDraw;          // Off-screen heap allocation for use
                                        //   by DirectDraw
#if 0
    OH*         pohVideoEngineScratch;  // Location of one entire scan line that
                                        //   can be used for temporary memory
                                        //   by the 868/968 pixel formatter
    FLATPTR     fpVisibleOverlay;       // Frame buffer offset to currently
                                        //   visible overlay; will be zero if
                                        //   no overlay is visible
    DWORD       dwOverlayFlipOffset;    // Overlay flip offset
#endif
#endif
    LONG        cyDDMemoryExtra;        // extra height of Video RAM
                                        // when cyMemory clamped to 0xFFF
#if USE_NT5_DDMEMMGR
    LONG        cyDDSlopHeight;
    LONG        cyDDHeap;
#endif

    GLOBALDATA  DriverData;
#if USE_D3D_CODE && (_WIN32_WINNT >= 0x0500)
    PFND3DPARSEUNKNOWNCOMMAND pD3DParseUnknownCommand;
#endif
#endif

    // Gamma stuff
    DWORD       dwRedGamma;
    DWORD       dwGreenGamma;
    DWORD       dwBlueGamma;
    DWORD       GammaTable[256];
    DWORD       GlideGammaTable[256];
    DWORD       TransientGammaTable[256];

    // Misc. Banshee stuff
    FIFO_DATA   fifoData;
    HANDLE      hCSIM;                  // Handle to CSIM engine dll
    BYTE        *CSIM_LFBbase;          // Permanently mapped LFB address
    ULONG       CSIM_LFBlength;         // and length for use by simulator.
#if defined(CSIM)
    BYTE        *CSIM_pjBase;           // real addr of V3 memory mapped regs
#endif
    ULONG       ulRop3;                 // Saved Rop3 for line code.
    ULONG       ulCmd;                  // Command storage for fastfill/textout

#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
    BOOL        doAGPFifo;              //  Use AGP code path for command fifo?
#endif

#ifdef TVOUT_SUPPORTED
    // TV Out and LCD stuff
	DWORD       dwVmiTv;               // VMI usage as video-in or tv-out
	DWORD       dwTvoActive;           // TV out is selected as display device
	BOOL        bTvoCapable;           // This adapter is capable of supporting TV Out.
	BOOL        bAllowPALCRT;          // If True then simultanious PAL and CRT display is Ok.
    FxU32       ulDevicesThatHaveTheirStatesSaved;
    FxU32       ulSavedStatesOfSavedDevices;
    ULONG       ulFrequency;           // Refresh rate in Hz
#endif //def TVOUT_SUPPORTED

    DWORD       cpuType;
    ULONG       ulCustomerNumber;

    BOOL        bGDIMadeDirectAccess;

	ULONG ulCmdFifoDisabled;
	
} PDEV;

VOID hwcSetContextDWORD(VOID);

#define P6_INTELCPU_WITH_KNI	(0x80000000L)	// for cpuType

/////////////////////////////////////////////////////////////////////////
// Miscellaneous prototypes:

VOID vStretchDIB(PDEV*, RECTL*, VOID*, LONG, RECTL*, RECTL*);  // For ddraw.c
BOOL bIntersect(RECTL*, RECTL*, RECTL*);
LONG cIntersect(RECTL*, RECTL*, LONG);
DWORD getAvailableModes(HANDLE, PVIDEO_MODE_INFORMATION*, DWORD*);

BOOL bInitializeModeFields(PDEV*, GDIINFO*, DEVINFO*, DEVMODEW*);
BOOL bFastFill(PDEV*, LONG, POINTFIX*, ULONG, ULONG, RBRUSH*, POINTL*, RECTL*);

BOOL bGetDeviceInfo(PDEV*);
BOOL bEnableHardware(PDEV*);
VOID vDisableHardware(PDEV*);
BOOL bAssertModeHardware(PDEV*, BOOL);

extern BYTE gaRop3FromMix[];

BOOL GetRegDWORD(PDEV *ppdev, const char *varname, ULONG *pValue);
BOOL SetRegDWORD(PDEV *ppdev, const char *varname, ULONG Value);
BOOL SetRegSZ(PDEV *ppdev, const char *varname, const char *pValue);


/////////////////////////////////////////////////////////////////////////
// The x86 C compiler insists on making a divide and modulus operation
// into two DIVs, when it can in fact be done in one.  So we use this
// macro.
//
// Note: QUOTIENT_REMAINDER implicitly takes unsigned arguments.

#if defined(_X86_)

#define QUOTIENT_REMAINDER(ulNumerator, ulDenominator, ulQuotient, ulRemainder) \
{                                                               \
    __asm mov eax, ulNumerator                                  \
    __asm sub edx, edx                                          \
    __asm div ulDenominator                                     \
    __asm mov ulQuotient, eax                                   \
    __asm mov ulRemainder, edx                                  \
}

#else

#define QUOTIENT_REMAINDER(ulNumerator, ulDenominator, ulQuotient, ulRemainder) \
{                                                               \
    ulQuotient  = (ULONG) ulNumerator / (ULONG) ulDenominator;  \
    ulRemainder = (ULONG) ulNumerator % (ULONG) ulDenominator;  \
}

#endif

/////////////////////////////////////////////////////////////////////////
// OVERLAP - Returns TRUE if the same-size lower-right exclusive
//           rectangles defined by 'pptl' and 'prcl' overlap:

#define OVERLAP(prcl, pptl)                                             \
    (((prcl)->right  > (pptl)->x)                                   &&  \
     ((prcl)->bottom > (pptl)->y)                                   &&  \
     ((prcl)->left   < ((pptl)->x + (prcl)->right - (prcl)->left))  &&  \
     ((prcl)->top    < ((pptl)->y + (prcl)->bottom - (prcl)->top)))

/////////////////////////////////////////////////////////////////////////
// SWAP - Swaps the value of two variables, using a temporary variable

#define SWAP(a, b, tmp) { (tmp) = (a); (a) = (b); (b) = (tmp); }

//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// CONVERT_TO_BYTES - Converts to byte count.

#define CONVERT_TO_BYTES(x, pdev)   ( (x) * pdev->cjPelSize)

//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// CONVERT_FROM_BYTES - Converts to byte count.

#define CONVERT_FROM_BYTES(x, pdev)	( (x) / pdev->cjPelSize)

//////////////////////////////////////////////////////////////////////

// These Mul prototypes are thunks for multi-board support:

ULONG   MulGetModes(HANDLE, ULONG, DEVMODEW*);
DHPDEV  MulEnablePDEV(DEVMODEW*, PWSTR, ULONG, HSURF*, ULONG, ULONG*,
                      ULONG, DEVINFO*, HDEV, PWSTR, HANDLE);
VOID    MulCompletePDEV(DHPDEV, HDEV);
HSURF   MulEnableSurface(DHPDEV);
BOOL    MulStrokePath(SURFOBJ*, PATHOBJ*, CLIPOBJ*, XFORMOBJ*, BRUSHOBJ*,
                      POINTL*, LINEATTRS*, MIX);
BOOL    MulFillPath(SURFOBJ*, PATHOBJ*, CLIPOBJ*, BRUSHOBJ*, POINTL*,
                    MIX, FLONG);
BOOL    MulBitBlt(SURFOBJ*, SURFOBJ*, SURFOBJ*, CLIPOBJ*, XLATEOBJ*,
                  RECTL*, POINTL*, POINTL*, BRUSHOBJ*, POINTL*, ROP4);
VOID    MulDisablePDEV(DHPDEV);
VOID    MulDisableSurface(DHPDEV);
BOOL    MulAssertMode(DHPDEV, BOOL);
VOID    MulMovePointer(SURFOBJ*, LONG, LONG, RECTL*);
ULONG   MulSetPointerShape(SURFOBJ*, SURFOBJ*, SURFOBJ*, XLATEOBJ*, LONG,
                           LONG, LONG, LONG, RECTL*, FLONG);
ULONG   MulDitherColor(DHPDEV, ULONG, ULONG, ULONG*);
BOOL    MulSetPalette(DHPDEV, PALOBJ*, FLONG, ULONG, ULONG);
BOOL    MulCopyBits(SURFOBJ*, SURFOBJ*, CLIPOBJ*, XLATEOBJ*, RECTL*, POINTL*);
BOOL    MulTextOut(SURFOBJ*, STROBJ*, FONTOBJ*, CLIPOBJ*, RECTL*, RECTL*,
                   BRUSHOBJ*, BRUSHOBJ*, POINTL*, MIX);
VOID    MulDestroyFont(FONTOBJ*);
BOOL    MulPaint(SURFOBJ*, CLIPOBJ*, BRUSHOBJ*, POINTL*, MIX);
BOOL    MulRealizeBrush(BRUSHOBJ*, SURFOBJ*, SURFOBJ*, SURFOBJ*, XLATEOBJ*,
                        ULONG);
HBITMAP MulCreateDeviceBitmap(DHPDEV, SIZEL, ULONG);
VOID    MulDeleteDeviceBitmap(DHSURF);
BOOL    MulStretchBlt(SURFOBJ*, SURFOBJ*, SURFOBJ*, CLIPOBJ*, XLATEOBJ*,
                      COLORADJUSTMENT*, POINTL*, RECTL*, RECTL*, POINTL*,
                      ULONG);

// These Dbg prototypes are thunks for debugging:

ULONG   DbgGetModes(HANDLE, ULONG, DEVMODEW*);
DHPDEV  DbgEnablePDEV(DEVMODEW*, PWSTR, ULONG, HSURF*, ULONG, ULONG*,
                      ULONG, DEVINFO*, HDEV, PWSTR, HANDLE);
VOID    DbgCompletePDEV(DHPDEV, HDEV);
VOID    DbgResetPDEV(DHPDEV,DHPDEV);
HSURF   DbgEnableSurface(DHPDEV);
BOOL    DbgLineTo(SURFOBJ*, CLIPOBJ*, BRUSHOBJ*, LONG, LONG, LONG, LONG,
                  RECTL*, MIX);
BOOL    DbgStrokePath(SURFOBJ*, PATHOBJ*, CLIPOBJ*, XFORMOBJ*, BRUSHOBJ*,
                      POINTL*, LINEATTRS*, MIX);
BOOL    DbgFillPath(SURFOBJ*, PATHOBJ*, CLIPOBJ*, BRUSHOBJ*, POINTL*,
                    MIX, FLONG);
BOOL    DbgBitBlt(SURFOBJ*, SURFOBJ*, SURFOBJ*, CLIPOBJ*, XLATEOBJ*,
                  RECTL*, POINTL*, POINTL*, BRUSHOBJ*, POINTL*, ROP4);
VOID    DbgDisablePDEV(DHPDEV);
VOID    DbgDisableSurface(DHPDEV);
BOOL    DbgAssertMode(DHPDEV, BOOL);
VOID    DbgMovePointer(SURFOBJ*, LONG, LONG, RECTL*);
ULONG   DbgSetPointerShape(SURFOBJ*, SURFOBJ*, SURFOBJ*, XLATEOBJ*, LONG,
                           LONG, LONG, LONG, RECTL*, FLONG);
ULONG   DbgDitherColor(DHPDEV, ULONG, ULONG, ULONG*);
BOOL    DbgSetPalette(DHPDEV, PALOBJ*, FLONG, ULONG, ULONG);
BOOL    DbgCopyBits(SURFOBJ*, SURFOBJ*, CLIPOBJ*, XLATEOBJ*, RECTL*, POINTL*);
BOOL    DbgTextOut(SURFOBJ*, STROBJ*, FONTOBJ*, CLIPOBJ*, RECTL*, RECTL*,
                   BRUSHOBJ*, BRUSHOBJ*, POINTL*, MIX);
VOID    DbgDestroyFont(FONTOBJ*);
BOOL    DbgPaint(SURFOBJ*, CLIPOBJ*, BRUSHOBJ*, POINTL*, MIX);
BOOL    DbgRealizeBrush(BRUSHOBJ*, SURFOBJ*, SURFOBJ*, SURFOBJ*, XLATEOBJ*,
                        ULONG);
HBITMAP DbgCreateDeviceBitmap(DHPDEV, SIZEL, ULONG);
VOID    DbgDeleteDeviceBitmap(DHSURF);
BOOL    DbgStretchBlt(SURFOBJ*, SURFOBJ*, SURFOBJ*, CLIPOBJ*, XLATEOBJ*,
                      COLORADJUSTMENT*, POINTL*, RECTL*, RECTL*, POINTL*,
                      ULONG);
ULONG   DbgEscape(SURFOBJ*, ULONG, ULONG, VOID*, ULONG, VOID*);
BOOL    DbgGetDirectDrawInfo(DHPDEV, DD_HALINFO*, DWORD*, VIDEOMEMORY*,
                             DWORD*, DWORD*);
BOOL    DbgEnableDirectDraw(DHPDEV, DD_CALLBACKS*, DD_SURFACECALLBACKS*,
                            DD_PALETTECALLBACKS*);
VOID    DbgDisableDirectDraw(DHPDEV);

VOID DbgSynchronize(DHPDEV,RECTL*);
BOOL DbgGradientFill(SURFOBJ*,CLIPOBJ*,XLATEOBJ*,TRIVERTEX*,ULONG,PVOID,
                     ULONG,RECTL*,POINTL*,ULONG);
BOOL DbgIcmSetDeviceGammaRamp(DHPDEV,ULONG,PVOID);

#ifdef CSIM
  BOOL bEnableCSIM(PDEV*);
  VOID vDisableCSIM(PDEV*);
  DWORD H3Hal_Dyn_Init( void );
  void csimInitDriver( FxU32 numBytes, volatile FxU32 *memory, volatile FxU32 *hw );
#endif

// H3 functions

VOID SetInvariantReg( PDEV *ppdev);
VOID vClearDesktopSurface(PDEV*);

#ifdef H3_FIFO

FxBool CmdFifo0Init( PDEV *ppdev, FxU32 fifoStart, FxU32 size, FxBool disableHoles, FxBool agpEnable);
void CmdFifo0Disable( PDEV *ppdev );
void H3MakeRoom( PDEV* ppdev, ULONG **, LONG n );

# if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
void NTAGPEPILOG(PDEV * ppdev, FxU32 hwPtr);
# endif

#endif


#if (_WIN32_WINNT >= 0x0500)
#include "gdi.h"
VOID vClipAndRender(GFNPB * ppb);   // For gradient fill
#endif


// Glide on NT stuff

BOOL miscGlideMapMemoryBases(PDEV *ppdev, HANDLE hProcess );
BOOL miscGlideUnmapMemoryBases(PDEV *ppdev, HANDLE hProcess);
LONG hwcExt(DWORD *lpInput, DWORD *lpOutput, PDEV *ppdev);
LONG hwcShareCPUType(DWORD *lpInput, DWORD *lpOutput, PDEV *ppdev);
VOID miscSaveExtState( PDEV *ppdev );
VOID miscRestoreExtState( PDEV *ppdev );

#define GLDATA_GDIFLAGS_HWC_EXCLUSIVE         0x00000001
#define GLDATA_GDIFLAGS_PPDEV_DISABLED        0x00000002
#define GLDATA_GDIFLAGS_PPDEV_DESTROYED       0x00000004
#define GLDATA_GDIFLAGS_CMDFIFO_SURFACE_LOST  0x00000008

#define GLIDE_EXCLUSION(glideState) \
	if ( glideState.glideGDIFlags & GLDATA_GDIFLAGS_HWC_EXCLUSIVE ) return TRUE;

//#define ENABLE_V3_W2K_GLIDE_CHANGES   1

// DWF
// macro to enable nop commands in the 2D and 3D portions of the chip. this was done to fix what has been 
// termed the WAX( windows accelerator ) problem. apparently, if the source of a copy is the destination of the
// previous copy and not enough cycles have passed the chip will hang. do not do this code if CSIM is enabled
#define WAX_WORKAROUND 1
#ifndef CSIM
#ifdef WAX_WORKAROUND
#define	WAXFIX_NOP_CODE\
	CHECK_FIFO_ROOM( ppdev, 1 );\
    GWH_BEGIN_PKT4_PACKET(1, SSTCP_PKT4_NOPCMD, 0x001);\
    SET3D(1, ppdev->p3DBase->nopCMD, 0x0);\
    GWH_END_PKT4_PACKET( 1 );
#else
#define WAXFIX_NOP_CODE
#endif  // ifdef WAX_WORKAROUND
#else
#define WAXFIX_NOP_CODE
#endif  // ifndef CSIM
// DWFE
