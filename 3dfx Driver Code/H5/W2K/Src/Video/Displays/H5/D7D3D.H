/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** $Revision: 10$
** $Date: 10/31/00 2:01:46 AM$
**
*/

#ifndef __D7D3D_H__
#define __D7D3D_H__

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)

//-----------------------------------------------------------------------------
//                       DX7 Texture management definitions
//-----------------------------------------------------------------------------

typedef struct _PALHNDL
{
  DWORD         dwFlags;
  WORD          wStartIndex;
  WORD          wNumEntries;
  PALETTEENTRY  ColorTable[256];    // array of palette entries, could be dynamic later
} PALHNDL, *PPALHNDL;

// This value will cause us to grow our arrays for surface handles, palette handles and
// state blocks in 4k blocks (or full pages on intel cpu's)
#define LISTGROWSIZE    1024

typedef struct _HNDLLIST
{
#if HL_LINKED_LIST
  struct _HNDLLIST  *pNext;           // this must be the first element in struct!
#endif

  LPVOID    pDDLcl;                   // owning ddraw pointer as a key
  struct _TXTRHNDL  **ppTxtrHndlList; // array of TXTRHNDL ptrs
                                      // the array is dynamically allocated
                                      // and the index is the surface handle
                                      // since handle=0 is not used, then
                                      // ppTxtrHndlList[0] is used to hold the
                                      // number of entries in ppTxtrHndlList
                                      // array, if allocated
  PALHNDL   **ppPalHndlList;          // array of PALHNDL ptrs
                                      // the array is dynamically allocated
                                      // and the index is the palette handle
                                      // since handle=0 is not used, then
                                      // ppPalHndlList[0] is used to hold the
                                      // number of entries in ppPalHndlList
                                      // array, if allocated
} HNDLLIST;
typedef HNDLLIST FAR* PHNDLLIST;

#define FLAGS_PER_DWORD                   32
#define SET_SB_RS_FLAG(pSB,state)         pSB->uc.bStoredRS[(state)/FLAGS_PER_DWORD] |= (1 << ((state) % FLAGS_PER_DWORD))
#define SET_SB_TSS_FLAG(pSB,tmu,state)    pSB->uc.bStoredTSS[(tmu)][(state)/FLAGS_PER_DWORD] |= (1 << ((state) % FLAGS_PER_DWORD))
#define IS_SB_RS_FLAG_SET(pSB,state)      pSB->uc.bStoredRS[(state)/FLAGS_PER_DWORD] & (1 << ((state) % FLAGS_PER_DWORD))
#define IS_SB_TSS_FLAG_SET(pSB,tmu,state) pSB->uc.bStoredTSS[(tmu)][(state)/FLAGS_PER_DWORD] & (1 << ((state) % FLAGS_PER_DWORD))

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
#define IS_STATEBLOCK_STREAM_FLAG_SET(pSB,state)            pSB->uc.bStoredStream & (1 << ((state) % FLAGS_PER_DWORD))
#define SET_STATEBLOCK_STREAM_FLAG(pSB,state)               pSB->uc.bStoredStream |= (1 << ((state) % FLAGS_PER_DWORD))
#endif
// As of the bld 2082 DDK, this is the max value in the D3DRENDERSTATETYPE enum in d3dtypes.h
#define MAX_RENDERSTATES              D3DRENDERSTATE_CLIPPLANEENABLE      // = 152
// As of the bld 2072 DDK, this is the max value in the D3DTEXTURESTAGESTATETYPE enum in d3dtypes.h
//#define MAX_TEXTURESTAGESTATES        D3DTSS_TEXTURETRANSFORMFLAGS        // = 24
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
#define MAX_TEXTURESTAGESTATES        D3DTSS_RESULTARG+1                    // = 28 + 1 (wrap)
#else
#define MAX_TEXTURESTAGESTATES        D3DTSS_TEXTURETRANSFORMFLAGS+1        // = 24 + 1 (wrap)
#endif

typedef struct _STATEBLOCK
{
  DWORD dwHandle;
  BOOL  bCompressed;

  union
  {
    // stored state block info (uncompressed)
    struct
    {
      // we're going to waste the entry in index 0 in each of these arrays
      DWORD RenderStates[MAX_RENDERSTATES+1];
      DWORD TssStates[NUMTEXTUREUNITS+1][MAX_TEXTURESTAGESTATES+1];

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
      D3DHAL_DP2SETSTREAMSOURCE StreamSource[1];    // Single stream only
      D3DHAL_DP2SETINDICES      StreamIndices;
      DWORD                     dwShaderHandle;
#endif
      // store 32 flags per dword
      BOOL  bStoredRS[((MAX_RENDERSTATES+1)+(FLAGS_PER_DWORD-1))/FLAGS_PER_DWORD];
      BOOL  bStoredTSS[NUMTEXTUREUNITS+1][((MAX_TEXTURESTAGESTATES+1)+(FLAGS_PER_DWORD-1))/FLAGS_PER_DWORD];
    } uc;
    // stored state block info (compressed)
    struct
    {
      DWORD dwNumRS;
      DWORD dwNumTSS[NUMTEXTUREUNITS+1];
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
      DWORD dwSizeSetStreamSource;
      DWORD dwSizeSetIndices;
      DWORD dwSizeShaderHandle;
#endif
      struct
      {
        DWORD dwType;   // either D3DRENDERSTATE_xxx or an index
        DWORD dwValue;  // the data
      } pair[1];
    } cc;
  };
} STATEBLOCK, *PSTATEBLOCK;

extern HNDLLIST *GetHndlListPtr(NT9XDEVICEDATA *ppdev, LPVOID pDDLcl);
extern VOID ReleaseTxtrHndl(NT9XDEVICEDATA *ppdev, HNDLLIST *pHndlList, DWORD dwTxtrHndl);

typedef struct _RC  RC;
extern HRESULT SetRenderTarget(RC *,DWORD,DWORD);
extern HRESULT Clear(RC *,DWORD,DWORD,D3DVALUE,DWORD,RECT *,DWORD);
extern HRESULT FastFill(RC *, D3DHAL_DP2CLEAR *, DWORD);
extern HRESULT FastFill_2pass(RC *, D3DHAL_DP2CLEAR *, DWORD);
extern HRESULT PaletteSet(RC *,DWORD,DWORD,DWORD);
extern HRESULT PaletteUpdate(RC *,DWORD,WORD,WORD,BYTE *);
extern HRESULT BeginStateBlock(RC *, DWORD);
extern HRESULT EndStateBlock(RC *);
extern HRESULT DeleteStateBlock(RC *, DWORD);
extern HRESULT ExecuteStateBlock(RC *, DWORD);
extern HRESULT CaptureStateBlock(RC *, DWORD);

extern DWORD bufferStateSetup(RC *pRc, DWORD unconditionalSet);

extern VOID ReleaseContextStateBlocks(RC *);

#endif  // DX7

#endif // __D7D3D_H__

