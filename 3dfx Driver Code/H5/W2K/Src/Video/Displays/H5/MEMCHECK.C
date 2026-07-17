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
** $Revision: 4$
** $Date: 10/11/00 8:44:24 PM$
**
*/

/***************************************************************************
* I N C L U D E S
****************************************************************************/

#include "precomp.h"

#ifdef MEMCHECK

/***************************************************************************
* D E F I N E S
****************************************************************************/

#define GARBAGE         0xCC
#define MEMCHK_DBGLVL   1

/***************************************************************************
* T Y P E D E F S
****************************************************************************/

typedef struct tagBLOCKINFO
{
  struct tagBLOCKINFO *pbiNext;         // next block info struct in linked list
  BYTE                *pb;              // ptr to mem allocated by app
  ULONG               size;             // size of mem allocated by app
  BOOL                fReferenced;      // flag for tracking use
  char                szFileName[13];   // filename where mem was allocated
  ULONG               ulLineNo;         // line number where mem was allocated
  DWORD               dwData;
} BLOCKINFO;

/***************************************************************************
* S T A T I C   V A R I A B L E S
****************************************************************************/

static BLOCKINFO  *pbiHead = NULL;

/****************************************************************************
*
* FUNCTION:     GetBlockInfo()
*
* DESCRIPTION:
*
****************************************************************************/

static BLOCKINFO *
GetBlockInfo ( BYTE *pb )
{
  BLOCKINFO *pbi;


  for (pbi = pbiHead; pbi != NULL; pbi = pbi->pbiNext)
  {
    BYTE  *pbStart = pbi->pb;
    BYTE  *pbEnd   = pbi->pb + pbi->size - 1;

    if ((pb >= pbStart) && (pb <= pbEnd))
      break;
  }

  ASSERTDD2(NULL != pbi);

  return pbi;
}

/****************************************************************************
*
* FUNCTION:    CreateBlockInfo()
*
* DESCRIPTION:
*
****************************************************************************/

static BOOL
CreateBlockInfo ( BYTE *pbNew, ULONG sizeNew, const char *pszFileName, ULONG ulLineNo, DWORD dwData )
{
  BLOCKINFO *pbi;
  int       i;


  ASSERTDD2(pbNew != NULL && sizeNew != 0);

#ifndef MS_VIEW
  pbi = (BLOCKINFO *)EngAllocMem(FL_ZERO_MEMORY, sizeof(BLOCKINFO), '3HMD');
#else
  pbi = (BLOCKINFO *)EngAllocMem(FL_ZERO_MEMORY, sizeof(BLOCKINFO), ALLOC_TAG);
#endif //MS_VIEW
  if (NULL != pbi)
  {
    pbi->pb = pbNew;
    pbi->size = sizeNew;
    pbi->pbiNext = pbiHead;
    for (i = 0; i < 13 && pszFileName[i]; i++)
      pbi->szFileName[i] = pszFileName[i];
    pbi->szFileName[i] = '\0';
    pbi->ulLineNo = ulLineNo;
    pbi->dwData = dwData;
    pbiHead = pbi;

    DISPDBG((MEMCHK_DBGLVL, "CreateBlockInfo: pb=%08lXh, size=%ld, pbi=%08lXh, dwData=%08lX [%s(%d)]",
             pbi->pb, pbi->size, pbi, pbi->dwData, pbi->szFileName, pbi->ulLineNo));
  }

  return (NULL != pbi);
}

/****************************************************************************
*
* FUNCTION:    FreeBlockInfo()
*
* DESCRIPTION:
*
****************************************************************************/

static void
FreeBlockInfo ( BYTE * pbToFree )
{
  BLOCKINFO *pbi, *pbiPrev;


  pbiPrev = NULL;
  for (pbi = pbiHead; pbi != NULL; pbi = pbi->pbiNext)
  {
    if (pbi->pb == pbToFree)
    {
      if (pbiPrev == NULL)
        pbiHead = pbi->pbiNext;
      else
        pbiPrev->pbiNext = pbi->pbiNext;
      break;
    }
    pbiPrev = pbi;
  }

  ASSERTDD2(NULL != pbi);

  DISPDBG((MEMCHK_DBGLVL, "FreeBlockInfo: pb=%08lXh, size=%ld, pbi=%08lXh, dwData=%08lX [%s(%d)]",
           pbi->pb, pbi->size, pbi, pbi->dwData, pbi->szFileName, pbi->ulLineNo));

  memset(pbi, GARBAGE, sizeof(BLOCKINFO));

  EngFreeMem(pbi);
}

/****************************************************************************
*
* FUNCTION:    UpdateBlockInfo()
*
* DESCRIPTION:
*
****************************************************************************/

static void
UpdateBlockInfo ( BYTE *pbOld, BYTE *pbNew, ULONG sizeNew )
{
  BLOCKINFO *pbi;

  ASSERTDD2(pbNew != NULL && sizeNew != 0);

  pbi = GetBlockInfo(pbOld);
  ASSERTDD2(pbOld == pbi->pb);

  pbi->pb = pbNew;
  pbi->size = sizeNew;
}

/****************************************************************************
*
* FUNCTION:    sizeofBlock()
*
* DESCRIPTION:
*
****************************************************************************/

static ULONG
sizeofBlock ( BYTE *pb )
{
  BLOCKINFO *pbi;


  pbi = GetBlockInfo(pb);
  ASSERTDD2(pb == pbi->pb);

  return pbi->size;
}

/****************************************************************************
*
* FUNCTION:    ClearMemoryRefs()
*
* DESCRIPTION:
*
****************************************************************************/

void
ClearMemoryRefs ( void )
{
  BLOCKINFO *pbi;

  for (pbi = pbiHead; pbi != NULL; pbi = pbi->pbiNext)
    pbi->fReferenced = FALSE;
}

/****************************************************************************
*
* FUNCTION:    NoteMemoryRef()
*
* DESCRIPTION:
*
****************************************************************************/

void
NoteMemoryRef ( void *pv )
{
  BLOCKINFO *pbi;

  pbi = GetBlockInfo((BYTE *)pv);
  pbi->fReferenced = TRUE;
}

/****************************************************************************
*
* FUNCTION:    CheckMemoryRefs()
*
* DESCRIPTION:
*
****************************************************************************/

void
CheckMemoryRefs ( void )
{
  BLOCKINFO *pbi;


  for (pbi = pbiHead; pbi != NULL; pbi = pbi->pbiNext)
  {
    ASSERTDD2(pbi->pb != NULL && pbi->size != 0);
    if (! pbi->fReferenced)
      DISPDBG((0, "Possible Memory Leak: %d bytes allocated at %s(%d), pbi=0x%08lXh, pb=0x%8lXh",
               pbi->size, pbi->szFileName, pbi->ulLineNo, pbi, pbi->pb));
    //ASSERTDD2(pbi->fReferenced);
  }
}

/****************************************************************************
*
* FUNCTION:    fValidPointer()
*
* DESCRIPTION:
*
****************************************************************************/

BOOL
fValidPointer ( void *pv, ULONG size )
{
  BLOCKINFO *pbi;
  BYTE *pb = (BYTE *)pv;


  ASSERTDD2(pv != NULL && size != 0);

  pbi = GetBlockInfo(pb);

  ASSERTDD2(pb+size <= pbi->pb+pbi->size);

  return TRUE;
}

/****************************************************************************
*
* FUNCTION:    NoteActiveRefs()
*
* DESCRIPTION:
*
****************************************************************************/

void
NoteActiveRefs ( void )
{
  BLOCKINFO *pbi;
  int       i = 0;


  for (pbi = pbiHead; pbi != NULL; pbi = pbi->pbiNext)
  {
    // if dwData is NULL, then assume it's referenced
    // this seems necessary since the pdsurf isn't stored anywhere
    // inside the SURFOBJ struct when a device bitmap is moved to
    // system memory
    if (0 == pbi->dwData)
    {
      NoteMemoryRef(pbi->pb);
      i++;
    }
    else if (*(DWORD *)pbi->dwData == (DWORD)pbi->pb)
      NoteMemoryRef(pbi->pb);
  }

  if (0 != i)
    DISPDBG((MEMCHK_DBGLVL, "NoteActiveRefs: %d blocks with dwData = 0", i));
}

/****************************************************************************
*
* FUNCTION:    CheckMemoryIntegrity()
*
* DESCRIPTION:
*
****************************************************************************/

void
CheckMemoryIntegrity ( void )
{
  ClearMemoryRefs();

  NoteActiveRefs();

  CheckMemoryRefs();
}

/***************************************************************************
*
* FUNCTION:     strrchr_
*
* DESCRIPTION:
*
****************************************************************************/

static const char * __fastcall
strrchr_ (const char *string, int c)
{
  const char  *pch;


  pch = string + strlen(string) - 1;
  while ((pch >= string) && ((char)c != *pch))
    pch--;

  if (pch < string)
    return NULL;

  return pch;
}

/****************************************************************************
*
* FUNCTION:    UpdateBlockData()
*
* DESCRIPTION:
*
****************************************************************************/

void
UpdateBlockData ( BYTE *pb, DWORD dwNewData )
{
  BLOCKINFO *pbi;


  ASSERTDD2(pb != NULL);

  pbi = GetBlockInfo(pb);

  if (pb == pbi->pb)
    pbi->dwData = dwNewData;
}

/****************************************************************************
*
* FUNCTION:    AllocMem()
*
* DESCRIPTION:
*
****************************************************************************/

PVOID
AllocMem ( ULONG      fl,
           ULONG      cj,
           ULONG      tag,
           const char *pszFileName,
           ULONG      ulLineNo,
           DWORD      dwData )
{
  BYTE        *pb;
  const char  *pszFile;


  ASSERTDD2(0 != cj);
  ASSERTDD2(NULL != pszFileName);

  CheckMemoryIntegrity();

  pb = EngAllocMem(fl,cj,tag);
  if (NULL != pb)
  {
    if (! (FL_ZERO_MEMORY & fl))
      memset(pb, GARBAGE, cj);

    pszFile = strrchr_(pszFileName,'\\');
    if (NULL != pszFile)
      pszFile++;

    if (! CreateBlockInfo(pb, cj, pszFile, ulLineNo, dwData))
    {
      EngFreeMem(pb);
      pb = NULL;
    }
  }

  return pb;
}

/****************************************************************************
*
* FUNCTION:    FreeMem()
*
* DESCRIPTION:
*
****************************************************************************/

VOID
FreeMem ( PVOID pv )
{
  //ASSERTDD2(NULL != pv)

  CheckMemoryIntegrity();

  if (NULL != pv)
  {
    memset(pv, GARBAGE, sizeofBlock(pv));
    FreeBlockInfo(pv);

    EngFreeMem(pv);
  }
}

#endif

