/*-*-c++-*-*/
/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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

extern  unsigned long _cdecl
P6Stuff(unsigned long physicalAddress);

#include <conio.h>

#if defined ( __DJGPP__ ) && defined ( __DOS32__ )

#include <stdlib.h>
#include <dpmi.h>
#include <sys/nearptr.h>
#include <fxdpmi.h>
#include <fxpci.h>
/*
** DpmiMapPhysicalToLinear
*/

/* We ought to also unmap the physical address mapping */

FxU32
DpmiMapPhysicalToLinear( FxU32 paddr, FxU32 length )
{
  __dpmi_meminfo info;
  FxU32 laddr;
  
  info.address = paddr;
  info.size = length;
  
  isP6 = P6Stuff(paddr);               /* Set stuff up for the P6 */
  
  if ( __dpmi_physical_address_mapping( &info ) )
    return 0;                                       /* failure */
  
  if ( __dpmi_lock_linear_region( &info ) )
    return 0;                                       /* failure */
  
  /* force segment limit to 0xFFFFFFFF -- all of address space */
  /* calling this function multiple times is ok, but they don't nest */
  
  if ( !__djgpp_nearptr_enable() )
    return 0;
  
  /* This isn't safe, since the base address can change. */
  
  return info.address + __djgpp_conventional_base;  /* success */
} /* DpmiMapPhysicalToLinear */

void
DpmiUnmapMemory( void )
{
  
} /* DpmiUnmapMemory */

#elif defined ( __WATCOMC__ ) && defined ( __DOS32__ )

#include <stdlib.h>
#include <i86.h>
#include <dos.h>
#include <fxdpmi.h>
#include <string.h>
#include <stdio.h>

#define DPMI_INTERRUPT 0x31

typedef unsigned long (far *FUNCPTR)();

#ifndef FP_OFF
#define FP_OFF(p)       ((unsigned long) p)
#endif

#define VXDLDR_DEVICE_ID        0x0027  // for dynamically loading Vxds
#define VOODOO_DEVICE_ID        0x39d2  // Ours.

/* dos mode msr management stuff */
extern FxBool __cdecl 
D32GetMSR(FxU32 inS, FxU32 outS);

extern FxBool __cdecl 
D32SetMSR(FxU32 inS, FxU32 outS);

static FxBool   onWindows   = FXFALSE;
static FUNCPTR  pVxdldr     = NULL;
static FUNCPTR  pVoodoo     = NULL;
static FxU32    vxdRefCount = 0;
static FxU32    isP6        = 0;

#if DEBUG
static int  verbose = 1;
#else
static const int  verbose = 0;
#endif

/* If we loaded the vxd once, but closed in the interim then make sure
 * that we use the the vxd functionality rather than dos.
 */
#define UPDATE_VXD() \
if (onWindows && (pVoodoo == NULL)) { \
  if (verbose) { \
    printf("DPMI: Updating vxd.\n"); \
    fflush(stdout); \
  } \
  DpmiLoadVXDs(); \
}

static void  DpmiUnhookFxmemmap();
static FxU32 DpmiHookFxmemmap(FxU32 laddr, FxU32 size);

static FxU32 VoodooMessage(FUNCPTR pSST, FxU32 data0, FxU32 data1, FxU32 function);

/*
** DpmiMapPhysicalToLinear
*/
FxU32
DpmiMapPhysicalToLinear( FxU32 paddr, FxU32 length)
{
  FxU32 laddr = 0;
  union REGS r;

  if (verbose) {
    printf("DpmiMapPhysicalToLinear: Checking for Intel P6\n");
    fflush(stdout);
  }

  isP6 = P6Stuff(paddr);

  /* Hook ourselves into the fxmemmap.vxd to monitor screen switches.
   * If we're on windows then the vxd can do the physical to linear
   * mapping for us. Otherwise, use dpmi.
   */
  if (verbose) {
    printf("DpmiMapPhysicalToLinear: Checking for FxMemMap to map board\n");
    fflush(stdout);
  }
  laddr = DpmiHookFxmemmap(paddr, length);

  /* If we don't have the vxd then try straight dpmi */
  if (pVoodoo == NULL) {
    /* function 0x800 (Physical Address Mapping) */
    r.w.ax = 0x800;
    
    /*
    ** BX:CX = physical address
    ** SI:DI = length
    */
    r.w.bx = ( FxU16 ) ( paddr >> 16 );
    r.w.cx = ( FxU16 ) ( paddr & 0x0000FFFF );
    r.w.si = ( FxU16 ) ( length >> 16 );
    r.w.di = ( FxU16 ) ( length & 0x0000FFFF );
    int386( DPMI_INTERRUPT, &r, &r );
    
    /* if cflag set then an error occured */
    if ( r.w.cflag == 0 ) {
      laddr = r.w.bx;
      laddr <<= 16;
      laddr |= r.w.cx;
    } else {
      laddr = 0;
    }
  }

  return laddr;
} /* DpmiMapPhysicalToLinear */

FxBool
DpmiUnmapMemory(FxU32 pAddr, FxU32 length)
{
  union REGS r;
  FxBool retVal = FXFALSE;

  if (verbose) {
    printf("DPMI: DpmiUnmapMemory(0x%X : 0x%X)\n", pAddr, length);
    fflush(stdout);
  }
  
  /* Check to see that we have dpmi 1.0 or better for the free call. */
  if (pVoodoo == NULL) {
    /* GetDpmiVersion */
    r.w.ax = 0x400;
    int386(DPMI_INTERRUPT, &r, &r);

    if (verbose) {
      printf("DPMI: DpmiUnmapMemory: \n\t"
             "Version: 0x%X : (0x%X : 0x%X)\n\t"
             "Flags: 0x%X\n\t"
             "Processor: 0x%X\n",
             r.x.eax, (FxU32)r.h.ah, (FxU32)r.h.al,
             (FxU32)r.w.bx,
             (FxU32)r.h.cl);
      fflush(stdout);
    }

    if ((r.w.cflag == 0) && (r.h.ah >= 1)) {
      /* function 0x801 (Free Physical Address Mapping) */
      r.w.ax = 0x801;

      /* BX:CX = physical address */
      r.w.bx = ( FxU16 ) ( pAddr >> 16 );
      r.w.cx = ( FxU16 ) ( pAddr & 0x0000FFFF );
      int386( DPMI_INTERRUPT, &r, &r );

      if (verbose) {
        printf("DPMI: DpmiUnmapMemory: FreePhysicalMapping: 0x%X : (0x%X : 0x%X)\n",
               pAddr, r.w.cflag, r.w.ax);
        fflush(stdout);
      }
  
      retVal = (r.w.cflag == 0);
    }
  } else {
    /* Nuke the mapping reference */
    if(vxdRefCount > 0) {
      VoodooMessage(pVoodoo, 0, 0, PROCUNMAPPHYS); 
      vxdRefCount--;

      if (verbose) {
        printf("UnmapPhys: 0x%X\n", vxdRefCount);
        fflush(stdout);
      }
    }

    /* Unhook and free the vxd. */
    if (vxdRefCount == 0) DpmiUnhookFxmemmap();
  }

  return retVal;
}

void 
DpmiUnloadVxd(void)
{
  if (pVoodoo != NULL) {
    /* Get rid of all of the current mappings to the vxd */
    while(vxdRefCount > 0) {
      VoodooMessage(pVoodoo, 0, 0, PROCUNMAPPHYS);
      vxdRefCount--;

      if (verbose) {
        printf("UnmapPhys: 0x%X\n", vxdRefCount);
        fflush(stdout);
      }
    }

    /* Unhook and free the vxd. */
    DpmiUnhookFxmemmap();
  }
}

/*
** DpmiAllocDosMem
*/
void* 
DpmiAllocDosMem( FxU16 size, DpmiSelector_t *pSel )
{
  union REGS  r;
  void       *ptr;
  FxU32       seg;
  
  /*
   ** AX = DPMI function 0x100
   */
  r.w.ax = 0x100;
  
  /*
   ** BX = # of paragraphs to allocate
   */
  r.w.bx = ( FxI16 ) ( size / 16 + 1 );  
  int386( DPMI_INTERRUPT, &r, &r );
  
  if ( r.w.cflag ) {
    ptr = 0;
  } else {
    seg  = r.w.ax;
    *pSel = r.w.dx;
    
    ptr = ( void * ) ( seg << 4 );
  }
  
  return ptr;

} /* DpmiAllocDosMem */

/*
** DpmiFreeDosMem
*/
FxBool
DpmiFreeDosMem( DpmiSelector_t sel )
{
  union REGS r;

  r.w.ax = 0x101;
  r.w.bx = sel;
  int386( DPMI_INTERRUPT, &r, &r );

  if ( r.w.cflag )
     return FXFALSE;
  else
     return FXTRUE;
} /* DpmiFreeDosMem */

/*
** DpmiExecuteRealModeInterrupt
*/
void
DpmiExecuteRealModeInterrupt( int intno, DpmiRMI *RMI )
{
  union  REGS r;
  struct SREGS sr;

  r.x.eax = 0x0300;           // DPMI service "execute real mode interrupt"
  r.x.ebx = intno;            // BL = function to execute
  r.x.ecx = 0;                // CX = 0
  segread( &sr );
  sr.es   = FP_SEG( RMI );   // ES:EDI -> buffer to RMI struct
  r.x.edi = ( unsigned ) RMI;

  int386x( DPMI_INTERRUPT, &r, &r, &sr );
} /* DpmiExecuteRealModeInterrupt */

/*
** DpmiExecuteRealModeProcedure
*/
void
DpmiExecuteRealModeProcedure( FxU16 proc_seg, FxU16 proc_off, DpmiRMI *RMI )
{
  union  REGS r;
  struct SREGS sr;

  RMI->CS = proc_seg;         // RMI CS:IP points to real mode procedure
  RMI->IP = proc_off;

  r.x.eax = 0x0301;           // DPMI service "execute real mode procedure"
  r.x.ebx = 0;                // BL = flags
  r.x.ecx = 0;                // CX = number of words to copy from PM stack to RM stack
  segread( &sr );
  sr.es   = FP_SEG( RMI );   // ES:EDI -> buffer to RMI struct
  r.x.edi = ( unsigned ) RMI;

  int386x( DPMI_INTERRUPT, &r, &r, &sr );
} /* DpmiExecuteRealModeProcedure */

/*
 *
 * This part implements the DpmiHookVxd() and DpmiUnhookVxd() procedures.
 *
 * This routine looks deceptively simple. Don't be fooled. It is a freakin' 
 * nightmare to get right, so please be gentle if you change it.
 *
 *
 * Given a VxD's device id, return a far pointer useable in protected mode
 * to call the VxD. This is then our direct connection to the VxD.
 *
 * This is used in 2 places:
 *
 *   Once to talk to the VXDLDR.VXD (Device id 0x27), which dynamically loads 
 *      fxmemmap.vxd for DOS programs.
 *
 *   To talk to fxmemmap.vxd (bogus device id 0x3df0, till we get a real one
 *      from MS), and inform it of the Voodoo physical address
 *      so we can auto-switch the passthru on FOCUS changes.
 *
 * Uses int 2F, eax = 0x1684 service, ebx = device id.
 */

char far *
AllocStringInDosMemory(char *string)
{
    union REGS r;
    struct SREGS s;
    int length = (strlen(string) + 16) >> 4;
    char far *new;

    /* Page 34, Watcom Programmer's Guide */
    memset(&s, 0, sizeof(s));
    memset(&r, 0, sizeof(r));
    r.w.ax = 0x0100;
    r.w.bx = length;            /* No. of 16byte blocks to alloc. */
    if (verbose) {
      printf("DPMI: allocating %d blocks of DOS memory\n", length);
      fflush(stdout);
    }
    int386x(DPMI_INTERRUPT, &r, &r, &s);

    /*
    ** ax = DOS real mode segment  
    ** dx = Protected mode selector,
    ** carry set if failure, clear if succeeded.
    */ 

    if ( r.w.cflag ) {
        if (verbose) {
          printf("DPMI: DOS mem alloc failed!\n");
          fflush(stdout);
        }
        return NULL;
    }

    new = MK_FP(r.w.dx, 0);
    if (verbose) {
      printf("selector = %d, new ptr = %.04x:%.08x\n", r.w.dx, 
             FP_SEG(new), FP_OFF(new));
      fflush(stdout);
    }
    if (new) {
      _fstrcpy(new, string);
    } else {
        if (verbose) {
          printf("DPMI: DOS mem: NULL ptr!\n");
          fflush(stdout);
        }
    }
    return new;
} /* AllocStringInDosMemory */

void
FreeStringInDosMemory(char far *string)
{
  union REGS r;
  struct SREGS s;
  
  memset(&s, 0, sizeof(s));
  memset(&r, 0, sizeof(r));
  r.w.ax = 0x0101;
  r.w.dx = FP_SEG(string);
  int386x(DPMI_INTERRUPT, &r, &r, &s);
  if (verbose) {
    printf("DPMI: Freed Dos memory\n");
    fflush(stdout);
  }
} /* FreeStringInDosMemory */

FUNCPTR 
GetDeviceAPI(FxU16 device_id)
{
  struct SREGS s;
  union  REGS  r;
  union {
    FUNCPTR p;
    short   s[3];
  } x; 
  
  /* 
   * Execute int 0x2f, with ax = 0x1684, bx = devID. 
   * Returns real mode proc in es:di
   */
  if (verbose) {
    printf("DPMI: GetDeviceAPI, devid = %.04x\n", device_id);
    fflush(stdout);
  }
  
  memset(&r, 0, sizeof(r));
  memset(&s, 0, sizeof(s));
  r.x.eax = 0x1684;
  r.x.ebx = device_id;
  int386x(0x2f, &r, &r, &s);
  
  if ( r.w.cflag ) {
    if (verbose) {
      printf("DPMI: GetDeviceAPI failed!\n");
      fflush(stdout);
    }
    return NULL;
  }
  
  /* Construct far pointer 16:32 from es: edi */
  x.s[0] = r.w.di;
  x.s[1] = (r.x.edi >> 16);
  x.s[2] = s.es;
  
  if (verbose) {
    long far *p;

    printf("DPMI: GetDeviceAPI: Devid=%.04x, FuncPtr%.04x:%.08x\n",
           device_id, FP_SEG(x.p), FP_OFF(x.p));
    fflush(stdout);

    /* Dump some data at this address to confirm INT 30h 's */
    p = (long far *) x.p;
    if (p != NULL) printf("DPMI: %.08x %.08x %.08x %.08x\n", 
                          p[0], p[1], p[2], p[3]);
    else printf("DPMI: GetDeviceID returns NULL!!!\n");
    fflush(stdout);
  }
  
  return (x.p);
} /* GetDeviceAPI */

/******************************************************************************
 *
 * Deliver an int2F message to our fxmemmap.vxd
 *
 *****************************************************************************/
FxU32
VoodooMessageHelper(
                    FxU16 seg,          /* eax */
                    long off,           /* ebx */
                    FxU32 data0,        /* ecx */
                    FxU32 data1,        /* edx */
                    FxU32 function      /* esi */
                    );
#pragma aux VoodooMessageHelper =                     \
  "push       eax"                                    \
  "push       ebx"                                    \
                                                      \
  "mov        eax, ecx"                               \
  "shr        ecx, 16"                                \
  "mov        ebx, ecx"                               \
                                                      \
  "mov        ecx, edx"                               \
  "shr        edx, 16"                                \
                                                      \
  "call       pword ptr [esp]"                        \
                                                      \
  "add        esp, 8"                                 \
  parm        [eax] [ebx] [ecx] [edx] [esi]           \
  modify      [eax];

static FxU32
VoodooMessage(FUNCPTR pSST, FxU32 data0, FxU32 data1, FxU32 function)
{
  if (verbose) {    
    printf("DPMI: VoodooMessage: pSST: 0x%lX data0: 0x%lX data1: 0x%lX fn: 0x%lX\n",
           (FxU32)pSST, data0, data1, function);
    fflush(stdout);
  }

  if (pSST == NULL) {
    if (verbose) {
      printf("DPMI: VoodooMessage: NULL ptr\n");
      fflush(stdout);
    }
    return 0;
  }

  return VoodooMessageHelper(FP_SEG(pSST), FP_OFF(pSST), data0, data1, function);
} /* VoodooMessage */


/******************************************************************************
 *
 * Get VxD Version Info() .. This is only a test.. in a real emergency, don't.
 *
 *****************************************************************************/
long
GetVxdVersionHelper(FxU16 seg, FxU32 offset);
#pragma aux GetVxdVersionHelper =                       \
  "push       eax"                                    \
  "push       ebx"                                    \
  "xor        eax, eax"                               \
  "call       pword ptr [esp]"                        \
  "add        esp, 8"                                 \
  parm        [eax] [ebx]                             \
  modify      [eax];

long
GetVxdVersion(FUNCPTR pVxd)
{
  if (pVxd == NULL) {
    if (verbose) {
      printf("DPMI: GetVxdVersion: Null ptr\n");
      fflush(stdout);
    }
    return 0;
  }
  return GetVxdVersionHelper(FP_SEG(pVxd), FP_OFF(pVxd));
} /* GetVxdVersion */

/******************************************************************************
 *
 * Load a dynamically loadable Vxd (fxmemmap.vxd in our case).
 *
 *****************************************************************************/
int
DynLoadVxdHelper(int pseg, int poff, int nseg, int noff);
#pragma aux DynLoadVxdHelper =                          \
  "push   ds "                                        \
                                                      \
  "push       eax"                                    \
  "push       ebx"                                    \
                                                      \
  "push       ecx"                                    \
  "pop        ds"                                     \
                                                      \
  "mov        eax, 1"                                 \
  "call       pword ptr [esp]"                        \
  "adc        eax, eax"                               \
  "and        eax, 1"                                 \
                                                      \
  "add        esp, 8"                                 \
  "pop        ds"                                     \
  parm        [eax] [ebx] [ecx] [edx]                 \
  modify      [eax];

int 
DynLoadVxd(FUNCPTR pldr, char* name)
{
  /*
   * This is a call to VXDLDR.VXD, with ds:dx = name, ax = 1
   */
  char far *p;
  int cc;
  
  if (verbose) {
    printf("DPMI: DynLoadVxd: %s\n", name);
    fflush(stdout);
  }
  
  if (pldr == NULL) {
    if (verbose) {
      printf("DPMI: DynLoadVxd: pLdr = NULL\n");
      fflush(stdout);
    }
    return 0;
  }
  
  p = AllocStringInDosMemory(name);
  
  if (p == NULL) {
    if (verbose) {
      printf("DPMI: failed DynLoadVxd\n");
      fflush(stdout);
    }
    return 0;
  } else {
    if (verbose) {
      printf("Dos memory: %.04x:%.08x = %s\n", FP_SEG(p), FP_OFF(p), p);
      fflush(stdout);
    }
  }
  
  cc = DynLoadVxdHelper(FP_SEG(pldr), FP_OFF(pldr), FP_SEG(p), FP_OFF(p));
  
  FreeStringInDosMemory(p);
  
  if (cc) {
    /* Carry is set, call failed! */
    if (verbose) {
      printf("DPMI: DynLoadVxd failed\n");
      fflush(stdout);
    }
    return 0;
  }
  
  if (verbose) {
    printf("DPMI: DynLoadVxd OK\n");
    fflush(stdout);
  }

  return 1;
} /* DynLoadVxd */

/******************************************************************************
 *
 * Unload a dynamically loadable Vxd (fxmemmap.vxd in our case).
 *
 *****************************************************************************/
int
DynUnloadVxdHelper(int pseg, int poff, int nseg, int noff);
#pragma aux DynUnloadVxdHelper =                        \
  "push   ds "                                        \
                                                      \
  "push       eax"                                    \
  "push       ebx"                                    \
                                                      \
  "push       ecx"                                    \
  "pop        ds"                                     \
                                                      \
  "mov        eax, 2"                                 \
  "mov        ebx, -1"                                \
  "call       pword ptr [esp]"                        \
                                                      \
  "add        esp, 8"                                 \
  "pop        ds"                                     \
  parm        [eax] [ebx] [ecx] [edx]                 \
  modify      [eax];

int 
DynUnloadVxd(FUNCPTR pldr, char *name)
{
  /*
   * This is a call to VXDLDR.VXD, with ds:dx = name, ax = 1
   */
  char far *p;
  
  p = AllocStringInDosMemory(name);
  if (p == NULL) {
    if (verbose) {
      printf("DPMI: failed DynUnloadVxd\n");
      fflush(stdout);
   }
    return 0;
  } else {
    if (verbose) {
      printf("Dos memory: %.04x:%.08x = %s\n", FP_SEG(p), FP_OFF(p), p);
      fflush(stdout);
    }
  }
  
  DynUnloadVxdHelper(FP_SEG(pldr), FP_OFF(pldr), FP_SEG(p), FP_OFF(p));
  
  FreeStringInDosMemory(p);
  return 1;  
} /* DynUnloadVxd */

static void
DpmiUnhookFxmemmap()
{
  if (pVoodoo != NULL) {
    DynUnloadVxd(pVxdldr, "FXMEMMAP");    
    pVoodoo = NULL;
  }

  if (verbose) {
    printf("DPMI: DpmiUnhookFxmemmap (0x%X : 0x%X) : 0x%X\n",
           pVxdldr, pVoodoo, vxdRefCount);
    fflush(stdout);
  }
} /* DpmiUnhookFxmemmap */

static int
DpmiLoadVXDs() {
  int i;
  int retVal = 0;

  if (verbose) {
    printf("DPMI: DpmiLoadVXDs (0x%X : 0x%X)\n",
           pVxdldr, pVoodoo);
    fflush(stdout);
  }

  /* Get an entry point into the VXDLDR services. */  
  if (pVxdldr == NULL) {
    pVxdldr = GetDeviceAPI(VXDLDR_DEVICE_ID);
    if (pVxdldr == NULL) {
      if (verbose) {
        printf("DPMI: Couldn't get VXDLDR entry point\n");
        fflush(stdout);
      }
      goto __errExit;
    }

    i = GetVxdVersion(pVxdldr);
    if (verbose) {
      printf("VXDLDR Version: %.08x\n", i);
      fflush(stdout);
    }
  } else if (verbose) {
    printf("DPMI: Already loaded vxdldr.vxd(0x%X)\n", pVxdldr);
    fflush(stdout);
  }
  
  /*
   * Dynamically load Fxmemmap.vxd.
   */
  
  /*
   * Get an entry point into the Fxmemmap.vxd services.
   */
  retVal = (pVoodoo != NULL);
  if (!retVal) {
    if (DynLoadVxd(pVxdldr, "fxmemmap.vxd") == 0) {
      if (verbose) {
        printf("DPMI: DynLoadVxd failed!\n");
        fflush(stdout);
      }
      goto __errExit;
    }

    pVoodoo = GetDeviceAPI(VOODOO_DEVICE_ID);
    retVal = (pVoodoo != NULL);
    if (!retVal) {
      if (verbose) {
        printf("DPMI: GetDeviceAPI for fxmemmap failed!\n");
        fflush(stdout);
      }
      goto __errExit;
    } else if (verbose) {
      printf("DPMI: GetDeviceAPI for fxmemmap.vxd OK!\n");
      fflush(stdout);
    }
  } else if (verbose) {
    printf("DPMI: Already loaded fxmemmap.vxd(0x%X)\n", pVoodoo);
    fflush(stdout);
  }
  
__errExit:
  return retVal;
} /* DpmiLoadVXDs */


/******************************************************************************
 *
 * The actual DpmiHookFxmemmap() and DpmiUnhookFxmemmap() functions.
 *
 *****************************************************************************/
static FxU32
DpmiHookFxmemmap(FxU32 laddr, FxU32 size)
{
  FxU32 retVal = 0;
  if (pVoodoo == NULL) onWindows = DpmiLoadVXDs();

#if DEBUG
  if (!verbose) verbose = ((getenv("SST_INITDEBUG") != NULL) ||
                           (getenv("SSTV2_INITDEBUG") != NULL));
#endif

  if (verbose) {
    printf("DPMI: In DpmiHookFxMemmap, addr=%.08x, size=%.08x\n",
                      laddr, size);
    fflush(stdout);
  }
  
  if ((laddr == 0) || (size == 0)) {
    if (verbose) {
      printf("DPMI: Bad addr/size\n");
      fflush(stdout);
    }
    goto DHFBadExit;
  }

  /* Send the vxd the physical address that was just mapped.
   *
   * NB: We are explicitly keeping track of the vxd ref count because
   * we need to make sure that it correctly gets decremented and
   * unloaded when we exit.  
   */
  if (pVoodoo != NULL) {
    FxU32 i;

    retVal = (laddr | 
              ((getenv("SST_DUALHEAD") == NULL) &&
               (getenv("SSTV2_DUALHEAD") == NULL)));

    if (verbose) {
      printf("DPMI: Loading fxmemmap.vxd\n");
      printf("DPMI: mapping %.08x size %.08x\n", retVal, size);
      fflush(stdout);
    }

    i = VoodooMessage(pVoodoo, (FxU32)&retVal, size, PROCMAPPHYS16);
    vxdRefCount += (i != 0);
    
    if (verbose) {
      printf("DPMI: Finally: (%s : 0x%X) : %.08x\n", 
             (i ? "Success" : "Failure"), i, retVal);
      fflush(stdout);
    }
  }
  
 DHFBadExit:
  return retVal;
} /* DpmiHookFxmemmap */

FxBool
DpmiGetMSR(FxU32 ins, FxU32 outs)
{
  if (!isP6) return FXFALSE;
  /* lbe (whoever they are) wants their game to run full speed on
   * 'real' dos even though they have a perfectly reasonable windows
   * version. What the hell is up w/ that?
   *
   * if (!onWindows) return FXFALSE; 
   */
  UPDATE_VXD();

  if (verbose) {
    printf("DpmiGetMsr(0x%x, 0x%x)\n", ins, outs);
    fflush(stdout);
  }

  if (pVoodoo != NULL) {
    /* This is DOS Running Under W95 */
    /* PCILib & VXD know what ins and outs are! */
    VoodooMessage(pVoodoo, ins, outs, PROCGETMSR);
  } else {
    /* This is DOS under DOS */
    D32GetMSR(ins, outs);
  }

  return FXTRUE;

} /* DpmiGetMSR */

FxBool
DpmiSetMSR(FxU32 ins, FxU32 outs)
{
  if (!isP6) return FXFALSE;
  /* lbe (whoever they are) wants their game to run full speed on
   * 'real' dos even though they have a perfectly reasonable windows
   * version. What the hell is up w/ that?
   *
   * if (!onWindows) return FXFALSE; 
   */
  UPDATE_VXD();

  if (verbose) {
    printf("DpmiSetMsr(0x%x, 0x%x)\n", ins, outs);
    fflush(stdout);
  }

  if (pVoodoo != NULL) {
    if (verbose) {
      typedef struct {
        FxU32 msrNum;
        FxU32 msrLo;
        FxU32 msrHi;
      } MSRInfo;

      printf("DpmiSetMsr() : 0x%X : (0x%X : 0x%X)\n",
             ((MSRInfo*)ins)->msrNum,
             ((MSRInfo*)ins)->msrLo,
             ((MSRInfo*)ins)->msrHi);
      fflush(stdout);
    }
      
    /* This is DOS Running Under W95 */
    /* PCILib & VXD know what ins and outs are! */
    VoodooMessage(pVoodoo, ins, outs, PROCSETMSR);
  } else {
    /* This is DOS under DOS */
    D32SetMSR(ins, outs);
  }

  return FXTRUE;

} /* DpmiSetMSR */

void
DpmiCheckVxd(FxBool *lonWindows, FxU32 *vxdVer)
{
  if (!(*lonWindows = DpmiLoadVXDs()))
    *vxdVer = 0;
  else
    VoodooMessage(pVoodoo, (FxU32) NULL, (FxU32) vxdVer, PROCGETVERSION);

  onWindows = *lonWindows;
  if (verbose) {
    printf("DpmiCheckVxd: %s : 0x%X\n",
                      (onWindows ? "Win95" : "Dos"),
                      *vxdVer);
    fflush(stdout);
  }
} /* DpmiCheckVxd */

FxBool
DpmiSetPassThroughBase(FxU32* const pBaseAddr, FxU32 hwBaseLen)
{
  FxBool retVal = FXFALSE;

  /* This only makes sense on dos under windows where we
   * actually have a vxd to do the passthrough thing.
   */
  if (onWindows && (pVoodoo != NULL)) {
    if (verbose) {
      printf("DpmiSetPassThroughBase: 0x%X\n", pBaseAddr);
      fflush(stdout);
    }
    
    VoodooMessage(pVoodoo, (FxU32)pBaseAddr, hwBaseLen, PROCSETPASSTHROUGHBASE16);
    retVal = FXTRUE;
  } 

  return retVal;
}

FxBool
DpmiOutputDebugString(const char* msgBuf)
{
  FxBool retVal = FXFALSE;

  /* This only makes sense on dos under windows where we
   * actually have a vxd to switch to that can do the 
   * output debugstring by hand.
   */
  if (onWindows && (pVoodoo != NULL)) {
    FxU32 temp;

    if (verbose) {
      printf("DpmiOutputDebugString: (%s : 0x%X)\n", 
                        msgBuf, msgBuf);
      fflush(stdout);
    }

    VoodooMessage(pVoodoo, (FxU32)msgBuf, (FxU32)&temp, PROCOUTPUTDEBUGSTRING16);
    retVal = FXTRUE;
  } 

  return retVal;
}

/* Ganked from vmm.h */
#define PC_USER         0x00040000  /* make the pages ring 3 accessible */

FxBool
DpmiLinearRangeSetPermission(const FxU32 addrBase, const FxU32 addrLen, const FxBool writeableP)
{
  FxBool retVal = FXFALSE;

  /* This only makes sense on dos under windows where we
   * actually have a vxd to switch to that can do the 
   * output debugstring by hand.
   */
  if (onWindows && (pVoodoo != NULL)) {
    FxU32 vxdParamArray[3];
    FxU32 tempVal;

    /* Set the user accessable bit. We don't dork w/ the
     * rest of the bits.
     */
    vxdParamArray[0] = addrBase;
    vxdParamArray[1] = addrLen;
    vxdParamArray[2] = (writeableP ? PC_USER : 0);
    
    if (verbose) {
      printf("DpmiLinearRangeSetPermission: (0x%lX : 0x%lX) : %s\n",
             addrBase, addrLen,
             (writeableP ? "Writeable" : "Protected"));
      fflush(stdout);
    }

    VoodooMessage(pVoodoo, (FxU32)vxdParamArray, (FxU32)&tempVal, PROCSETADDRPERM16);
    retVal = FXTRUE;
  }

  return retVal;
}

#else

#  error Compiler not supported

#endif
