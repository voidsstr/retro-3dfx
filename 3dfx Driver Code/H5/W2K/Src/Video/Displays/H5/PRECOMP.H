/******************************Module*Header*******************************\
* Module Name: precomp.h
*
* Common headers used throughout the display driver.  This entire include
* file will typically be pre-compiled.
*
* Copyright (c) 1993-1996 Microsoft Corporation
\**************************************************************************/

#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <windef.h>

#if (_WIN32_WINNT >= 0x0500)
#define _CRTIMP

#if (DX == 8) // This must be defined before any DX DDK files are included
#define DIRECTDRAW_VERSION  0x0800 
#define DIRECT3D_VERSION    0x0800 
#endif 

#if (DX == 8) 
#include <d3d8.h>
#endif 

#include <d3d.h>

#endif

#include <winerror.h>
#include <wingdi.h>
#include <winddi.h>
#include <devioctl.h>
#include <ntddvdeo.h>
#include <ioaccess.h>

#include "logfile.h"

#include "3dfx.h"
#include "h3g.h"
#include "h3gdefs.h"
#include "h3defs.h"
#include "h3regs.h"

#include "lines.h"

#ifdef USE_DDRAW_CODE
#include "ddvpe32.h"
#endif

#include "driver.h"
#include "hw.h"
#include "debug.h"

#include "memcheck.h"

#ifdef USE_DDRAW_CODE
#include "ddfxnt95.h"
#endif

#include "hwcext.h"
#include "glicd.h"
#include "fnproto.h"

#if (_WIN32_WINNT >= 0x0500)
#include "gdi.h"    // For GradientFill
#endif
