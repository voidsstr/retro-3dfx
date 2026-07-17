/*  
** THIS SOFTWARE IS SUBJECT TO COPYRIGHT PROTECTION AND IS OFFERED ONLY
** PURSUANT TO THE 3DFX GLIDE GENERAL PUBLIC LICENSE. THERE IS NO RIGHT
** TO USE THE GLIDE TRADEMARK WITHOUT PRIOR WRITTEN PERMISSION OF 3DFX
** INTERACTIVE, INC. A COPY OF THIS LICENSE MAY BE OBTAINED FROM THE 
** DISTRIBUTOR OR BY CONTACTING 3DFX INTERACTIVE INC(info@3dfx.com). 
** THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER 
** EXPRESSED OR IMPLIED. SEE THE 3DFX GLIDE GENERAL PUBLIC LICENSE FOR A
** FULL TEXT OF THE NON-WARRANTY PROVISIONS.  
** 
** USE, DUPLICATION OR DISCLOSURE BY THE GOVERNMENT IS SUBJECT TO
** RESTRICTIONS AS SET FORTH IN SUBDIVISION (C)(1)(II) OF THE RIGHTS IN
** TECHNICAL DATA AND COMPUTER SOFTWARE CLAUSE AT DFARS 252.227-7013,
** AND/OR IN SIMILAR OR SUCCESSOR CLAUSES IN THE FAR, DOD OR NASA FAR
** SUPPLEMENT. UNPUBLISHED RIGHTS RESERVED UNDER THE COPYRIGHT LAWS OF
** THE UNITED STATES.  
** 
** COPYRIGHT 3DFX INTERACTIVE, INC. 1999, ALL RIGHTS RESERVED
 */



#include <assert.h>

#include <3dfx.h>

#include "fxpci.h"
#include "pcilib.h"

/* Callback declarations */
static FxBool pciInitializeHSIM(void);
static FxBool pciShutdownHSIM(void);
static const char* pciIdentifierHSIM(void);

static FxU8  pciPortInByteHSIM(FxU16 port);
static FxU16 pciPortInWordHSIM(FxU16 port);
static FxU32 pciPortInLongHSIM(FxU16 port);
  
static FxBool pciPortOutByteHSIM(FxU16 port, FxU8 data);
static FxBool pciPortOutWordHSIM(FxU16 port, FxU16 data);
static FxBool pciPortOutLongHSIM(FxU16 port, FxU32 data);

static FxBool pciMapLinearHSIM(FxU32 busNumber, FxU32 physAddr,
                             FxU32* linearAddr, FxU32* length);
static FxBool pciUnmapLinearHSIM(FxU32 linearAddr, FxU32 length);

static FxBool pciSetPermissionHSIM(const FxU32 addrBase, const FxU32 addrLen,
                                 const FxBool writePermP);

static FxBool pciMsrGetHSIM(MSRInfo* in, MSRInfo* out);
static FxBool pciMsrSetHSIM(MSRInfo* in, MSRInfo* out);

static FxBool pciOutputStringHSIM(const char* msg);
static FxBool pciSetPassThroughBaseHSIM(FxU32* baseAddr, FxU32 baseAddrLen);

static char pciIdent[] = "@#% fxPCI for HSIM";

static const FxPlatformIOProcs __ioProcsHSIM = {
  pciInitializeHSIM,
  pciShutdownHSIM,
  pciIdentifierHSIM,

  pciPortInByteHSIM,
  pciPortInWordHSIM,
  pciPortInLongHSIM,
  
  pciPortOutByteHSIM,
  pciPortOutWordHSIM,
  pciPortOutLongHSIM,

  pciMapLinearHSIM,
  pciUnmapLinearHSIM,
  pciSetPermissionHSIM,

  pciMsrGetHSIM,
  pciMsrSetHSIM,

  pciOutputStringHSIM,
  pciSetPassThroughBaseHSIM
};
const FxPlatformIOProcs* ioProcsHSIM = &__ioProcsHSIM;

FxBool pciPlatformInit(void)
{
  gCurPlatformIO = ioProcsHSIM;

  return(FXTRUE);
}


/* Basic platform init/shutdown stuff */
static FxBool
pciInitializeHSIM(void)
{
  return FXTRUE;
}

static FxBool
pciShutdownHSIM(void)
{
  return FXTRUE;
}

static const char* 
pciIdentifierHSIM(void)
{
  return pciIdent;
}

/* Device address space management stuff */

static FxBool
pciMapLinearHSIM(FxU32 busNumber, FxU32 physical_addr,
               FxU32 *linear_addr, FxU32 *length)
{
  assert(0);
  
  return FXTRUE;
}

static FxBool
pciUnmapLinearHSIM(FxU32 linear_addr, FxU32 length) 
{
  assert(0);
}

/* Platform port io stuff */

FxU8 
pciPortInByteHSIM (unsigned short port)
{
  assert(0);
} /* pioInByte */

FxU16
pciPortInWordHSIM (unsigned short port)
{
  assert(0);
} /* pioInWord */

FxU32
pciPortInLongHSIM (unsigned short port)
{
  assert(0);
} /* pioInLong */

FxBool
pciPortOutByteHSIM (unsigned short port, FxU8 data)
{
  assert(0);
} /* pioOutByte */

FxBool
pciPortOutWordHSIM (unsigned short port, FxU16 data)
{
  assert(0);
} /* pioOutWord */

FxBool
pciPortOutLongHSIM (unsigned short port, FxU32 data)
{
  assert(0);
} /* pioOutLong */

static FxBool 
pciMsrGetHSIM(MSRInfo* in, MSRInfo* out)
{
  assert(0);
}

static FxBool 
pciMsrSetHSIM(MSRInfo* in, MSRInfo* out)
{
  assert(0);
}

/* Platform utilities. */
static FxBool
pciOutputStringHSIM(const char* msg)
{
  assert(0);
}

static FxBool
pciSetPermissionHSIM(const FxU32 addrBase, const FxU32 addrLen,
                   const FxBool writePermP)
{
  return FXFALSE;
}

static FxBool
pciSetPassThroughBaseHSIM(FxU32* baseAddr, FxU32 baseAddrLen)
{
  return FXFALSE;
}



