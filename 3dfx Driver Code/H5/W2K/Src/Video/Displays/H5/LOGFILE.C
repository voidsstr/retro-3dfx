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

/****************************************************************************
*
* FUNCTION:     FlushLogFileBuffer()
*
* DESCRIPTION:
*
****************************************************************************/

VOID
FlushLogFileBuffer ( PDEV *ppdev )
{
  // if there's any data in buffer, flush it to disk
  if (0 < dwBufCnt)
  {
    DWORD numBytes;
    DWORD retval;


    // call ioctl to have miniport write data to a file
    retval = EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_3DFX_WRITE_LOG_FILE,
                                Buf,
                                dwBufCnt,
                                NULL,
                                0,
                                &numBytes);
    if (retval)
    {
      DISPDBG((0, "FlushLogFileBuffer ioctl failed, returned %08lXh", retval));
    }
  }

  // re-initialize vars
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
* FUNCTION:     h3printf()
*
* DESCRIPTION:
*
****************************************************************************/

VOID
h3printf ( PDEV *ppdev, LPSTR szFormat, ... )
{
  extern int _cdecl vsprintf(char *, const char *, va_list);
  WriteLogFile(ppdev, h3_buf, vsprintf(h3_buf, szFormat, (LPVOID)(&szFormat+1)));
}

#endif // ENABLE_LOG_FILE

