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
** $Revision: 2$
** $Date: 10/11/00 8:42:35 PM$
**
** $Log: 
**  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 1     10/01/98 6:13p Russ
*/

#include "precomp.h"

#if ENABLE_LOG_FILE

/***************************************************************************
* D E F I N E S
****************************************************************************/

#define BUFFER_SIZE     0x00010000    //  64 KB buffer
//#define BUFFER_SIZE     0x00040000    // 256 KB buffer

/***************************************************************************
* S T A T I C   V A R I A B L E S
****************************************************************************/

static char   Buf[BUFFER_SIZE];
static DWORD  dwBufCnt = 0;
static char   *pCurrBufPos = Buf;
static char   h3_buf[256];

/* retro3dfx: registry-ring log sink. The WRITE_LOG_FILE IOCTL proved unreliable
   (videoprt/build issues), but SetRegSZ (IOCTL_3DFX_SET_REGISTRY_VALUE) is used
   throughout the driver and provably reaches the miniport. So the log is flushed
   as a ring of REG_SZ values RLog00..RLog31 (32 x <=1000 bytes = ~32 KB flight
   recorder) under the miniport's Device0 key; RLogSeq (DWORD) = total chunks
   written (newest slot = (RLogSeq-1) & 31). Agent reads via REGREAD. */
#define RETRO_LOG_RING     32
#define RETRO_LOG_CHUNK    1000
ULONG g_retroLogSeq = 0;

/* retro3dfx: D3D context create/destroy balance (warm-rerun leak hunt) */
LONG g_retroCtxLive = 0;

/* retro3dfx: kernel-pool alloc/free balance for the device-cycle leak hunt.
   Every driver EngAllocMem/EngFreeMem routes through these (see MEMCHECK.H
   non-MEMCHECK path). g_retroPoolLive = outstanding allocations; if it climbs
   monotonically across D3D device create/destroy cycles, a per-device pool
   allocation is not being freed on teardown. */
LONG g_retroPoolLive = 0;
LONG g_retroPoolEverAlloc = 0;

/* retro3dfx: video-memory surface balance (device-cycle leak hunt). */
LONG g_retroVidSurfLive = 0;
LONG g_retroVidSurfEver = 0;
LONG g_retroVidSurfNullFree = 0;

PVOID
retroEngAllocMem(ULONG fl, ULONG cj, ULONG tag)
{
  PVOID p = EngAllocMem(fl, cj, tag);
  if (NULL != p)
  {
    g_retroPoolLive++;
    g_retroPoolEverAlloc++;
  }
  return p;
}

VOID
retroEngFreeMem(PVOID p)
{
  if (NULL != p)
    g_retroPoolLive--;
  EngFreeMem(p);
}

/****************************************************************************
*
* FUNCTION:     FlushLogFileBuffer()
*
* DESCRIPTION:  Flush the accumulated log buffer to the registry ring (proven
*               SetRegSZ channel) AND attempt the legacy file IOCTL (harmless if
*               it no-ops).
*
****************************************************************************/

VOID
FlushLogFileBuffer ( PDEV *ppdev )
{
  if (0 < dwBufCnt)
  {
    DWORD numBytes;
    ULONG outBuf = 0;
    DWORD off;
    char  chunk[RETRO_LOG_CHUNK + 1];
    char  name[8];

    /* legacy file sink attempt (kept; harmless if the IOCTL no-ops) */
    EngDeviceIoControl(ppdev->hDriver, IOCTL_3DFX_WRITE_LOG_FILE,
                       Buf, dwBufCnt, &outBuf, sizeof(outBuf), &numBytes);

    /* registry-ring sink (primary, proven) */
    for (off = 0; off < dwBufCnt; )
    {
      DWORD n = dwBufCnt - off;
      ULONG slot;
      if (n > RETRO_LOG_CHUNK) n = RETRO_LOG_CHUNK;
      memcpy(chunk, Buf + off, n);
      chunk[n] = '\0';
      slot = g_retroLogSeq % RETRO_LOG_RING;
      name[0]='R'; name[1]='L'; name[2]='o'; name[3]='g';
      name[4]=(char)('0' + (slot / 10));
      name[5]=(char)('0' + (slot % 10));
      name[6]='\0';
      SetRegSZ(ppdev, name, chunk);
      g_retroLogSeq++;
      SetRegDWORD(ppdev, "RLogSeq", g_retroLogSeq);
      off += n;
    }
  }

  dwBufCnt = 0;
  pCurrBufPos = Buf;
}

/****************************************************************************
*
* FUNCTION:     WriteLogFile()
*
* DESCRIPTION:
*
****************************************************************************/

static VOID
WriteLogFile ( PDEV *ppdev, LPVOID pBuffer, DWORD BytesToWrite )
{
  if (BUFFER_SIZE < (dwBufCnt + BytesToWrite))
  {
    // this string will go past end of buffer
    // so flush current buffer to disk
    // and reset buffer vars
    FlushLogFileBuffer(ppdev);
  }

  // stuff this string in the buffer and
  // adjust buffer vars
  memcpy(pCurrBufPos,pBuffer,BytesToWrite);
  pCurrBufPos += BytesToWrite;
  dwBufCnt += BytesToWrite;
}

/****************************************************************************
*
* FUNCTION:     retroLogInit()  (retro3dfx)
*
* DESCRIPTION:  Lazy one-time read of the runtime log gate from the registry
*               (HKLM\SYSTEM\CCS\Services\3dfxvs\Device0\Retro3dfxLog, REG_SZ).
*               0/absent = logging off (default, zero benchmark impact),
*               1 = on, >=2 = verbose (adds hot-path heartbeats).
*
****************************************************************************/

// -1 = uninitialized; 0 = off (default); 1 = on; >=2 = verbose
LONG g_retroLogLevel = -1;

// stashed at DrvEnablePDEV so ppdev-less loggers (V5DLog) can reach the file sink
PDEV *g_retroLogPpdev = NULL;

static VOID
retroLogInit ( PDEV *ppdev )
{
  extern int __cdecl atoi(const char *);
  char *pEnvStr;

  if (0 > g_retroLogLevel)
  {
    pEnvStr = ddgetenv(ppdev, "Retro3dfxLog");
    g_retroLogLevel = (NULL == pEnvStr) ? 0 : atoi(pEnvStr);
  }
}

/****************************************************************************
*
* FUNCTION:     h3printf()
*
* DESCRIPTION:  retro3dfx: now runtime-gated on g_retroLogLevel so the fully
*               instrumented (LF=1) driver logs nothing unless enabled via
*               the Retro3dfxLog registry value.
*
****************************************************************************/

VOID
h3printf ( PDEV *ppdev, LPSTR szFormat, ... )
{
  extern int _cdecl vsprintf(char *, const char *, va_list);
  retroLogInit(ppdev);
  if (0 >= g_retroLogLevel)
    return;
  WriteLogFile(ppdev, h3_buf, vsprintf(h3_buf, szFormat, (LPVOID)(&szFormat+1)));
}

/****************************************************************************
*
* FUNCTION:     retroLogForce()  (retro3dfx)
*
* DESCRIPTION:  Always logs (bypasses the runtime gate) and flushes so the
*               tail survives a hang/wedge. For rare flight-recorder lines
*               only (first-call announce, FIFO stalls, wedge breaks).
*
****************************************************************************/

VOID
retroLogForce ( PDEV *ppdev, LPSTR szFormat, ... )
{
  extern int _cdecl vsprintf(char *, const char *, va_list);
  retroLogInit(ppdev);
  WriteLogFile(ppdev, h3_buf, vsprintf(h3_buf, szFormat, (LPVOID)(&szFormat+1)));
  FlushLogFileBuffer(ppdev);
}

/****************************************************************************
*
* FUNCTION:     retroLogRaw()  (retro3dfx)
*
* DESCRIPTION:  Write an already-formatted buffer to the log and flush.
*               Used by V5DLog (which has already vsprintf'd its message).
*               retro3dfx: UNCONDITIONAL (not gated) — V5DLog marks infrequent,
*               diagnostically-important lifecycle events (mode set / surface
*               enable/disable / assert-mode / D3D+texture milestones), so it is
*               always captured. The registry-read gate proved unreliable for
*               enabling logging, and these events are rare enough that always-on
*               costs nothing. Per-op verbose logging (h3printf) stays gated.
*
****************************************************************************/

VOID
retroLogRaw ( PDEV *ppdev, LPVOID pBuffer, DWORD BytesToWrite )
{
  WriteLogFile(ppdev, pBuffer, BytesToWrite);
  FlushLogFileBuffer(ppdev);
}

#endif // ENABLE_LOG_FILE

