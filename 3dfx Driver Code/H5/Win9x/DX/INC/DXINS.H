/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** $Log: 
**  2    3dfx      1.0.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 1     6/02/99 6:51a Michael
** Branch from h3
** 
** 4     10/01/98 11:59a Adrians
** Added Event Notification to the instrumentation code.
*/

#ifndef __DXINS_H
#define __DXINS_H

#if defined( INSTRUMENTATION )

// Opcodes to read the Pentium clock
#define _rdtsc _asm _emit 0x0f _asm _emit 0x31
#define _rdpmc _asm _emit 0x0f _asm _emit 0x33

// Read the initial clock value
#define READ_TSC( hi64, lo64 ) \
_asm { \
  _rdtsc \
  _asm mov   lo64, eax \
  _asm mov   hi64, edx \
}

// Calculate the clock difference    
#define DIFF_TSC( hi64, lo64 ) \
_asm { \
  _rdtsc \
  _asm sub   eax, lo64 \
  _asm sbb   edx, hi64 \
  _asm mov   lo64, eax \
  _asm mov   hi64, edx \
}

#define READ_PMC( hi64, lo64, counter ) \
_asm { \
  _asm mov   ecx, counter \
  _rdpmc \
  _asm mov   lo64, eax \
  _asm mov   hi64, edx \
}

#define DIFF_PMC( hi64, lo64, counter ) \
_asm { \
  _asm mov   ecx, counter \
  _rdpmc \
  _asm sub   eax, lo64 \
  _asm sbb   edx, hi64 \
  _asm mov   lo64, eax \
  _asm mov   hi64, edx \
}

// Instrumentation enable
#define INS_ENABLE    (1 << 0)
#define INS_PMCENABLE (1 << 1)

#define INS_EVENT_SHIFT     2
#define INS_EVENT_MASK      (0xF << INS_EVENT_SHIFT)
#define INS_EVENT_FLIP      (0 << INS_EVENT_SHIFT)
#define INS_EVENT_ENDSCENE  (1 << INS_EVENT_SHIFT)

// Define for the event name
#define INS_EVENT_NAME  "insDXEvent"

// Function Macros
#define INS_ENTRY( category )   insStart( category )
#define INS_EXIT( )             insExit( )
#define INS_EXITEVENT( event )  insExitEvent( event );

#define INS_QSTART( )         insQStart( )
#define INS_QEND( )           insQEnd( )

typedef enum {
INSQ_CALLCOUNT,
INSQ_TOTALTIME,
INSQ_QUEUETIME,
INSQ_PMC0,
INSQ_PMC1,
INSQ_MAX = 16
} INSQUERY, *LPINSQUERY;

typedef struct {
DWORD insQuery[INSQ_MAX];
} INSQUERY_S, *LPINQUERY_S;

// Define constant offsets for DirectX instrumentation entries
typedef enum {
INSC_D3DCLEAR,                      // 0
INSC_D3DCONTEXTCREATE,              // 1
INSC_D3DCONTEXTDESTROY,             // 2
INSC_D3DCONTEXTDESTROYALL,          // 3
INSC_D3DDRAWONEPRIMITIVE,           // 4
INSC_D3DDRAWONEINDEXEDPRIMITIVE,    // 5
INSC_D3DDRAWPRIMITIVES,             // 6
INSC_D3DRENDERSTATE,                // 7
INSC_D3DRENDERPRIMITIVE,            // 8
INSC_D3DHANDLECREATE,               // 9
INSC_D3DHANDLEDESTROY,              // 10
INSC_D3DHANDLESWAP,                 // 11
INSC_D3DGETSTATE,                   // 12
INSC_D3DHANDLEGETSURF,              // 13
INSC_D3DSCENECAPTURE,               // 14
INSC_D3DDRAWPRIMITIVES2,            // 15
INSC_D3DCLEAR2,                     // 16
INSC_D3DVALIDATETEXTURESTAGESTATE,  // 17
INSC_DDBLT,                         // 18
INSC_DDGETBLTSTATUS,                // 19
INSC_DDWAITFORVERTICALBLANK,        // 20
INSC_DDGETSCANLINE,                 // 21
INSC_DDFLIP,                        // 22
INSC_DDGETFLIPSTATUS,               // 23
INSC_DDFLIPTOGDISURFACE,            // 24
INSC_DDSETEXCLUSIVEMODE,            // 25
INSC_DDDESTROYDRIVER,               // 26
INSC_DDSETPALETTE,                  // 27
INSC_DDSETCOLORKEY,                 // 28
INSC_DDCREATEPLETTE,                // 29
INSC_DDDESTROYPALETTE,              // 30
INSC_DDSETENTRIES,                  // 31 
INSC_DDCANCREATESURFACE,            // 32 
INSC_DDCREATESURFACE,               // 33
INSC_DDDESTROYSURFACE,              // 34
INSC_DDADDATTACHEDSURFACE,          // 35
INSC_DDLOCK,                        // 36
INSC_DDUNLOCK,                      // 37
INSC_DDGETDRIVERINFO,               // 38
INSC_DDUPDATEOVERLAY,               // 39
INSC_DDSETOVERLAYPOSITION,          // 40
INSC_MAX                            // 41
} INSCATEGORY;

// Define the instrumentation header
typedef struct INS_HEADER
{
  DWORD   flags;
  INSQUERY_S insCategory[INSC_MAX];
} INSHEADER;

extern INSHEADER insHeader;
extern DWORD totalTimeH, totalTimeL;
extern DWORD counter0H, counter0L;
extern DWORD counter1H, counter1L;
extern DWORD queueTime, queueTimeH, queueTimeL;
extern DWORD insCategory;
extern HANDLE insEventHandle;

static void insStart( DWORD category )
{
  queueTime = 0;
  insCategory = category;
  insHeader.insCategory[insCategory].insQuery[INSQ_CALLCOUNT]++;  
  
  if( insHeader.flags & INS_PMCENABLE )
  {
    READ_PMC( counter0H, counter0L, 0 );
    READ_PMC( counter1H, counter1L, 1 );
  }
  
  READ_TSC( totalTimeH, totalTimeL );
}

static void insExit( )
{
  DIFF_TSC( totalTimeH, totalTimeL );
  insHeader.insCategory[insCategory].insQuery[INSQ_TOTALTIME] += totalTimeL;
  insHeader.insCategory[insCategory].insQuery[INSQ_QUEUETIME] += queueTime;

  if( insHeader.flags & INS_PMCENABLE )
  {
    DIFF_PMC( counter0H, counter0L, 0 );
    insHeader.insCategory[insCategory].insQuery[INSQ_PMC0] += counter0L;
  
    DIFF_PMC( counter1H, counter1L, 1 );
    insHeader.insCategory[insCategory].insQuery[INSQ_PMC1] += counter1L;
  }
}

static void insExitEvent( DWORD event )
{
  DIFF_TSC( totalTimeH, totalTimeL );
  insHeader.insCategory[insCategory].insQuery[INSQ_TOTALTIME] += totalTimeL;
  insHeader.insCategory[insCategory].insQuery[INSQ_QUEUETIME] += queueTime;

  if( insHeader.flags & INS_PMCENABLE )
  {
    DIFF_PMC( counter0H, counter0L, 0 );
    insHeader.insCategory[insCategory].insQuery[INSQ_PMC0] += counter0L;
  
    DIFF_PMC( counter1H, counter1L, 1 );
    insHeader.insCategory[insCategory].insQuery[INSQ_PMC1] += counter1L;
  }
  
  if( (insHeader.flags & INS_EVENT_MASK) == event )
    SetEvent( insEventHandle );  
}

static void insQStart( )
{
  READ_TSC( queueTimeH, queueTimeL );
}

static void insQEnd( )
{
  DIFF_TSC( queueTimeH, queueTimeL );
  queueTime += queueTimeL;
}

#else
  #define INS_ENTRY( category )
  #define INS_EXIT( )
  #define INS_EXITEVENT( event )

  #define INS_QSTART( )
  #define INS_QEND( )
#endif

#endif // __DX_INS_H
