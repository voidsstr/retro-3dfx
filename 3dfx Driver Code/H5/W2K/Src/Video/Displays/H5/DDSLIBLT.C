/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** File Name:   DDSLIBLT.C
**
** Description:	Internal DDRAW blt functions for SLI/AA mode
**	
*/

/****************************************************************************
*
* INTERNAL FUNCTIONS:
*
* sli_ColorFill       --- fill surface with a color, always use SRCCOPY ROP
* sli_PuntColorFill   --- fill surface with a color, always use SRCCOPY ROP
* sli_PuntColorFillZ  --- fill Z surface with a color, always use SRCCOPY ROP
* sli_DoBltNoSP       --- blt w/o source/pattern, may have dest. color key
* sli_DoBltS          --- screen to screen blt, also handle stretch/shrink
* sli_SystemToVideo   --- perform host to screen blt
*
****************************************************************************/

#include "precomp.h"

#if defined(SLI_AA)
#include <ddsli2d.h>

#pragma intrinsic (memcpy, memset)

/*
 * We use one scanline buffer to read the source because it gains performance,
 * and we do not need to special case overlapping in the x direction.
 *
 * We only use the second buffer if we need to read the destination before
 * writing back. Otherwise, we should not use the second buffer as it is
 * slower.
 */
// maximum x resolution: 2048, maximum color depth: 4 bytes per pixel

BYTE scanBuf1[2048* 4];
BYTE scanBuf2[2048* 4];



/***************************************************************************/
/*                             DEFINES                                     */
/***************************************************************************/

// pointers to functions returning voids

typedef void (__stdcall * PBLTFUNC) (BLT_PARAMS);
typedef void (__stdcall * PSBLTFUNC) (LPDDHAL_BLTDATA, BYTE *, BYTE *, DWORD, DWORD, DWORD, DWORD);
typedef void (* PFILLSCANFUNC) (BYTE *, BYTE *, DWORD);

typedef enum tag_colorKey { none, src, dst, both } CKEY_TYPE;

// not implemented features

#ifdef DEBUG
 #define NOT_SUPPORTED  _asm { int 3 }
#else
 #define NOT_SUPPORTED
#endif

// macro to convert SSTG_PIX_FMT_* to bytes per pixel

#define GET_BYTE_DEPTH(BytesPerPel, PelFormat) \
        switch(PelFormat) \
        { \
            case SSTG_PIXFMT_8BPP:  \
                BytesPerPel = 1;    \
                break;              \
            case SSTG_PIXFMT_16BPP: \
                BytesPerPel = 2;    \
                break;              \
            case SSTG_PIXFMT_32BPP: \
                BytesPerPel = 4;    \
                break;              \
            default: /* format not supported */ \
                BytesPerPel = 0;    \
                break;              \
        }

// macro to advance to the next scanline

#define NEXT_SCAN { srcPtr += srcPitch; dstPtr += dstPitch; }

/***************************************************************************/
/*                       FUNCTION PROTOTYPES                               */
/***************************************************************************/

// parameter list for blt functions

#define BLT_PARAMS \
    LPDDHAL_BLTDATA  pbd, \
    DWORD dstWidthBytes, DWORD dstHeight, DWORD srcPitch, DWORD dstPitch, \
    BYTE *srcPtr, BYTE *dstPtr, DWORD srcBytesPerPel, DWORD dstBytesPerPel

void __stdcall sBltSrcToScanBuf(LPDDHAL_BLTDATA, BYTE *, BYTE *, DWORD, DWORD, DWORD, DWORD);
void __stdcall xsBltSrcToScanBuf(LPDDHAL_BLTDATA,BYTE *, BYTE *, DWORD, DWORD, DWORD, DWORD);

void __stdcall bltDSa   (BLT_PARAMS);
void __stdcall bltDSan  (BLT_PARAMS);
void __stdcall bltDSna  (BLT_PARAMS);
void __stdcall bltDSno  (BLT_PARAMS);
void __stdcall bltDSo   (BLT_PARAMS);
void __stdcall bltDSon  (BLT_PARAMS);
void __stdcall bltDSx   (BLT_PARAMS);
void __stdcall bltDSxn  (BLT_PARAMS);
void __stdcall bltNop   (BLT_PARAMS);
void __stdcall bltS     (BLT_PARAMS);
void __stdcall bltSDna  (BLT_PARAMS);
void __stdcall bltSDno  (BLT_PARAMS);
void __stdcall bltSn    (BLT_PARAMS);

void __stdcall xBltDSa  (BLT_PARAMS);
void __stdcall xBltDSan (BLT_PARAMS);
void __stdcall xBltDSna (BLT_PARAMS);
void __stdcall xBltDSno (BLT_PARAMS);
void __stdcall xBltDSo  (BLT_PARAMS);
void __stdcall xBltDSon (BLT_PARAMS);
void __stdcall xBltDSx  (BLT_PARAMS);
void __stdcall xBltDSxn (BLT_PARAMS);
void __stdcall xBltS    (BLT_PARAMS);
void __stdcall xBltSDna (BLT_PARAMS);
void __stdcall xBltSDno (BLT_PARAMS);
void __stdcall xBltSn   (BLT_PARAMS);

void __stdcall sBltS    (BLT_PARAMS);

/***************************************************************************/
/*                         FUNCTION TABLES                                 */
/***************************************************************************/

// VRAM/system memory to VRAM opaque blt, no stretch/shrink

PBLTFUNC BltFuncTab[16] =
{
    (PBLTFUNC) &bltNop,     // BLACKNESS is supported by sli_DoBltNoSP()
    (PBLTFUNC) &bltDSon,    // NOTSRCERASE
    (PBLTFUNC) &bltDSna,
    (PBLTFUNC) &bltSn,      // NOTSRCCOPY
    (PBLTFUNC) &bltSDna,    // SRCERASE
    (PBLTFUNC) &bltNop,     // DSTINVERT is supported by sli_DoBltNoSP()
    (PBLTFUNC) &bltDSx,     // SRCINVERT
    (PBLTFUNC) &bltDSan,
    (PBLTFUNC) &bltDSa,     // SRCAND
    (PBLTFUNC) &bltDSxn,
    (PBLTFUNC) &bltNop,     // rop D is supported by sli_DoBltNoSP()
    (PBLTFUNC) &bltDSno,    // MERGEPAINT
    (PBLTFUNC) &bltS,       // SRCCOPY
    (PBLTFUNC) &bltSDno,
    (PBLTFUNC) &bltDSo,     // SRCPAINT
    (PBLTFUNC) &bltNop,     // WHITENESS is supported by sli_DoBltNoSP()
};

// VRAM/system memory to VRAM transparent blt, no stretch/shrink

PBLTFUNC xBltFuncTab[16] =
{
    (PBLTFUNC) &bltNop,     // BLACKNESS is supported by sli_DoBltNoSP()
    (PBLTFUNC) &xBltDSon,   // NOTSRCERASE
    (PBLTFUNC) &xBltDSna,
    (PBLTFUNC) &xBltSn,     // NOTSRCCOPY
    (PBLTFUNC) &xBltSDna,   // SRCERASE
    (PBLTFUNC) &bltNop,     // DSTINVERT is supported by sli_DoBltNoSP()
    (PBLTFUNC) &xBltDSx,    // SRCINVERT
    (PBLTFUNC) &xBltDSan,
    (PBLTFUNC) &xBltDSa,    // SRCAND
    (PBLTFUNC) &xBltDSxn,
    (PBLTFUNC) &bltNop,     // rop D is supported by sli_DoBltNoSP()
    (PBLTFUNC) &xBltDSno,   // MERGEPAINT
    (PBLTFUNC) &xBltS,      // SRCCOPY
    (PBLTFUNC) &xBltSDno,
    (PBLTFUNC) &xBltDSo,    // SRCPAINT
    (PBLTFUNC) &bltNop,     // WHITENESS is supported by sli_DoBltNoSP()
};

// VRAM/system memory to VRAM opaque blt, stretch/shrink

PBLTFUNC sBltFuncTab[16] =
{
    (PBLTFUNC) &bltNop,     // BLACKNESS is supported by sli_DoBltNoSP()
    (PBLTFUNC) &xBltDSon,   // NOTSRCERASE
    (PBLTFUNC) &xBltDSna,
    (PBLTFUNC) &xBltSn,     // NOTSRCCOPY
    (PBLTFUNC) &xBltSDna,   // SRCERASE
    (PBLTFUNC) &bltNop,     // DSTINVERT is supported by sli_DoBltNoSP()
    (PBLTFUNC) &xBltDSx,    // SRCINVERT
    (PBLTFUNC) &xBltDSan,
    (PBLTFUNC) &xBltDSa,    // SRCAND
    (PBLTFUNC) &xBltDSxn,
    (PBLTFUNC) &bltNop,     // rop D is supported by sli_DoBltNoSP()
    (PBLTFUNC) &xBltDSno,   // MERGEPAINT
    (PBLTFUNC) &sBltS,      // SRCCOPY
    (PBLTFUNC) &xBltSDno,
    (PBLTFUNC) &xBltDSo,    // SRCPAINT
    (PBLTFUNC) &bltNop,     // WHITENESS is supported by sli_DoBltNoSP()
};


/***************************************************************************/
/*                       INTERNAL FUNCTIONS                                */
/***************************************************************************/

void __stdcall xBltDSa(BLT_PARAMS)
{
    NOT_SUPPORTED
}

void __stdcall xBltDSan(BLT_PARAMS)
{
    NOT_SUPPORTED
}

void __stdcall xBltDSna(BLT_PARAMS)
{
    NOT_SUPPORTED
}

void __stdcall xBltDSno(BLT_PARAMS)
{
    NOT_SUPPORTED
}

void __stdcall xBltDSo(BLT_PARAMS)
{
    NOT_SUPPORTED
}

void __stdcall xBltDSon(BLT_PARAMS)
{
    NOT_SUPPORTED
}

void __stdcall xBltDSx(BLT_PARAMS)
{
    NOT_SUPPORTED
}

void __stdcall xBltDSxn(BLT_PARAMS)
{
    NOT_SUPPORTED
}

void __stdcall xBltS(BLT_PARAMS)
{
    CKEY_TYPE colorKey;
    DWORD srcColorkeyMin, srcColorkeyMax;
    DWORD dstColorkeyMin, dstColorkeyMax;
    DWORD dwData, i;
    WORD  wData;
    BYTE *pSrcData, *pDstData, bData, *pData;

    colorKey = none;

    if (pbd->dwFlags & DDBLT_KEYSRCOVERRIDE)
    {
        colorKey = src;
        srcColorkeyMin = pbd->bltFX.ddckSrcColorkey.dwColorSpaceLowValue;
        srcColorkeyMax = pbd->bltFX.ddckSrcColorkey.dwColorSpaceHighValue;
    }

    if (pbd->dwFlags & DDBLT_KEYDESTOVERRIDE)
    {
        colorKey = (src==colorKey)? both : dst;
        dstColorkeyMin = pbd->bltFX.ddckDestColorkey.dwColorSpaceLowValue;
        dstColorkeyMax = pbd->bltFX.ddckDestColorkey.dwColorSpaceHighValue;
    }

    if (src == colorKey)    // do not write if src pixel matches color key
    {
        switch(srcBytesPerPel)
        {
        case 1: // 8bpp
            while (dstHeight--)
            {
                memcpy(scanBuf1, srcPtr, dstWidthBytes);
                pDstData = dstPtr;
                for (i=0; i < dstWidthBytes; i++)
                {
                    bData = scanBuf1[i];
                    if ((bData < srcColorkeyMin) || (bData > srcColorkeyMax))
                    {
                        *((BYTE *)pDstData) = bData;
                    }
                    ((BYTE *)pDstData)++;
                }
                NEXT_SCAN
            }
            break;

        case 2: // 16bpp
            while (dstHeight--)
            {
                memcpy(scanBuf1, srcPtr, dstWidthBytes);
                pDstData = dstPtr;
                pData = scanBuf1;
                for (i=0; i < (dstWidthBytes >> 1); i++)
                {
                    wData = *((WORD *)pData)++;
                    if ((wData < srcColorkeyMin) || (wData > srcColorkeyMax))
                    {
                        *((WORD *)pDstData) = wData;
                    }
                    ((WORD *)pDstData)++;
                }
                NEXT_SCAN
            }
            break;

        case 4: // 32bpp
            while (dstHeight--)
            {
                memcpy(scanBuf1, srcPtr, dstWidthBytes);
                pDstData = dstPtr;
                pData = scanBuf1;
                for (i=0; i < (dstWidthBytes >> 2); i++)
                {
                    dwData = *((DWORD *)pData)++ & 0xffffff;
                    if ((dwData < srcColorkeyMin) || (dwData > srcColorkeyMax))
                    {
                        *((DWORD *)pDstData) = dwData;
                    }
                    ((DWORD *)pDstData)++;
                }
                NEXT_SCAN
            }
            break;
        } // end: switch(bytesPerPel)
    } // endif: use src color key
    else
    {
        if (dst == colorKey ) // update dst. pixel if matches color key
        {
            switch(dstBytesPerPel)
            {
            case 1: // 8bpp
                while (dstHeight--)
                {
                    memcpy(scanBuf1, srcPtr, dstWidthBytes);
                    memcpy(scanBuf2, dstPtr, dstWidthBytes);
                    pSrcData = scanBuf1;
                    pData = scanBuf2;
                    pDstData = dstPtr;
                    for (i=0; i < dstWidthBytes; i++)
                    {
                        bData = *((BYTE *)pData)++;
                        if ((bData >= dstColorkeyMin) &&
                            (bData <= dstColorkeyMax)
                           )
                        {
                            *((BYTE *)pDstData) = *((BYTE *)pSrcData);
                        }
                        ((BYTE *)pDstData)++;
                        ((BYTE *)pSrcData)++;
                    }
                    NEXT_SCAN
                }
                break;

            case 2: // 16bpp
                while (dstHeight--)
                {
                    memcpy(scanBuf1, srcPtr, dstWidthBytes);
                    memcpy(scanBuf2, dstPtr, dstWidthBytes);
                    pSrcData = scanBuf1;
                    pData = scanBuf2;
                    pDstData = dstPtr;
                    for (i=0; i < (dstWidthBytes >> 1); i++)
                    {
                        wData = *((WORD *)pData)++;
                        if ((wData >= dstColorkeyMin) &&
                            (wData <= dstColorkeyMax)
                           )
                        {
                            *((WORD *)pDstData) = *((WORD *)pSrcData);
                        }
                        ((WORD *)pDstData)++;
                        ((WORD *)pSrcData)++;
                    }
                    NEXT_SCAN
                }
                break;

            case 4: // 32bpp
                while (dstHeight--)
                {
                    memcpy(scanBuf1, srcPtr, dstWidthBytes);
                    memcpy(scanBuf2, dstPtr, dstWidthBytes);
                    pSrcData = scanBuf1;
                    pData = scanBuf2;
                    pDstData = dstPtr;
                    for (i=0; i < (dstWidthBytes >> 2); i++)
                    {
                        dwData = *((DWORD *)pData)++ & 0xffffff;
                        if ((dwData >= dstColorkeyMin) &&
                            (dwData <= dstColorkeyMax)
                           )
                        {
                            *((DWORD *)pDstData) = *((DWORD *)pSrcData);
                        }
                        ((DWORD *)pDstData)++;
                        ((DWORD *)pSrcData)++;
                    }
                    NEXT_SCAN
                }
                break;
            } // end: switch(bytesPerPel)
        } // endif: use dst color key
        else
        {   // use both src and dst color key

            switch(dstBytesPerPel)
            {
            case 1:
                while (dstHeight--)
                {
                    memcpy(scanBuf1, srcPtr, dstWidthBytes);
                    memcpy(scanBuf2, dstPtr, dstWidthBytes);
                    pSrcData = scanBuf1;
                    pData = scanBuf2;
                    pDstData = dstPtr;
                    for (i=0; i < dstWidthBytes; i++)
                    {
                        bData = *((BYTE *)pSrcData);
                        if ((bData < srcColorkeyMin) || (bData > srcColorkeyMax))
                        {
                            bData = *((BYTE *)pData)++;
                            if ((bData >= dstColorkeyMin) &&
                                (bData <= dstColorkeyMax)
                               )
                            {
                                *((BYTE *)pDstData) = *((BYTE *)pSrcData);
                            }
                         }
                         ((BYTE *)pSrcData)++;
                         ((BYTE *)pDstData)++;
                    }
                    NEXT_SCAN
                }
                break;

            case 2:
                while (dstHeight--)
                {
                    memcpy(scanBuf1, srcPtr, dstWidthBytes);
                    memcpy(scanBuf2, dstPtr, dstWidthBytes);
                    pSrcData = scanBuf1;
                    pData = scanBuf2;
                    pDstData = dstPtr;
                    for (i=0; i < (dstWidthBytes >> 1); i++)
                    {
                        wData = *((WORD *)pSrcData);
                        if ((wData < srcColorkeyMin) || (wData > srcColorkeyMax))
                        {
                            wData = *((WORD *)pData)++;
                            if ((wData >= dstColorkeyMin) &&
                                (wData <= dstColorkeyMax)
                               )
                            {
                                *((WORD *)pDstData) = *((WORD *)pSrcData);
                            }
                         }
                         ((WORD *)pSrcData)++;
                         ((WORD *)pDstData)++;
                    }
                    NEXT_SCAN
                }
                break;

            case 4:
                while (dstHeight--)
                {
                    memcpy(scanBuf1, srcPtr, dstWidthBytes);
                    memcpy(scanBuf2, dstPtr, dstWidthBytes);
                    pSrcData = scanBuf1;
                    pData = scanBuf2;
                    pDstData = dstPtr;
                    for (i=0; i < (dstWidthBytes >> 1); i++)
                    {
                        dwData = *((DWORD *)pSrcData) & 0xffffff;
                        if ((dwData < srcColorkeyMin) || (dwData > srcColorkeyMax))
                        {
                            dwData = *((DWORD *)pData)++ & 0xffffff;
                            if ((dwData >= dstColorkeyMin) &&
                                (dwData <= dstColorkeyMax)
                               )
                            {
                                *((DWORD *)pDstData) = *((DWORD *)pSrcData);
                            }
                         }
                         ((DWORD *)pSrcData)++;
                         ((DWORD *)pDstData)++;
                    }
                    NEXT_SCAN
                }
                break;
            } // end: switch(bytesPerPel)
        } // endif: use src and dst color key
    } // endif: use src color key
}

void __stdcall xBltSDna(BLT_PARAMS)
{
}

void __stdcall xBltSDno(BLT_PARAMS)
{
}

void __stdcall xBltSn(BLT_PARAMS)
{
}

/*--------------------------------------------------------------------*/
void __stdcall bltDSa(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ &= *((DWORD *)pSrcData)++;
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ &= *((BYTE *)pSrcData)++;
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltDSan(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ =
                ~( *((DWORD *)pSrcData)++ & *((DWORD *)pDstData) );
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ =
                ~( *((BYTE *)pSrcData)++ & *((BYTE *)pDstData) );
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltDSna(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ =
                ~( *((DWORD *)pSrcData)++ ) & *((DWORD *)pDstData);
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ =
                ~( *((BYTE *)pSrcData)++ ) & *((BYTE *)pDstData);
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltDSno(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ =
                ~( *((DWORD *)pSrcData)++ ) | *((DWORD *)pDstData);
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ =
                ~( *((BYTE *)pSrcData)++ ) | *((BYTE *)pDstData);
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltDSo(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ |= *((DWORD *)pSrcData)++;
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ |= *((BYTE *)pSrcData)++;
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltDSon(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ =
                ~( *((DWORD *)pSrcData)++ | *((DWORD *)pDstData) );
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ =
                ~( *((BYTE *)pSrcData)++ | *((BYTE *)pDstData) );
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltDSx(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ ^= *((DWORD *)pSrcData)++;
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ ^= *((BYTE *)pSrcData)++;
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltDSxn(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ =
                ~( *((DWORD *)pSrcData)++ ^ *((DWORD *)pDstData) );
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ =
                ~( *((BYTE *)pSrcData)++ ^ *((BYTE *)pDstData) );
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltNop(BLT_PARAMS)
{
    /* nothing to do, hurry home */
}

void __stdcall bltS(BLT_PARAMS)
{
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(dstPtr, scanBuf1, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltSDna(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ =
                ~( *((DWORD *)pDstData) ) & *((DWORD *)pSrcData)++;
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ =
                ~( *((BYTE *)pDstData) ) & *((BYTE *)pSrcData)++;
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltSDno(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ =
                ~( *((DWORD *)pDstData) ) | *((DWORD *)pSrcData)++;
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ =
                ~( *((BYTE *)pDstData) ) | *((BYTE *)pSrcData)++;
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltSn(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = dstPtr;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ =
                ~( *((DWORD *)pSrcData)++ );
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ =
                ~( *((BYTE *)pSrcData)++ );
        }
        NEXT_SCAN
    }
}
/*--------------------------------------------------------------------*/

/*----------------------------------------------------------------------
Function name:  initBuffer

Description:    initialize the scanline buffer
----------------------------------------------------------------------*/
void initBuffer(BYTE * pBuf, DWORD bpp, DWORD color, DWORD dstWidthBytes)
{
  DWORD i;
  BYTE * pData;

  switch(bpp)
  {
    case 1: // 8bpp
      memset(pBuf, color, dstWidthBytes);
      break;
    case 2: // 16bpp
      pData = pBuf;
      for (i=0; i < (dstWidthBytes >> 1); i++)
      {
        *((WORD *)pData)++ = (WORD)color;
      }
      break;
    case 4: // 32bpp
      pData = pBuf;
      for (i=0; i < (dstWidthBytes >> 2); i++)
      {
        *((DWORD *)pData)++ = (DWORD)color;
      }
      break;
   }
}

/*----------------------------------------------------------------------
Function name:  fillScanAND

Description:    primitive to perform a bitwise AND operation on data
----------------------------------------------------------------------*/
void fillScanAND(BYTE *dstPtr, BYTE *srcPtr, DWORD dstWidthBytes)
{
  DWORD rightEdge, i;
  BYTE *pDstData;

  // read frame buffer into scanline buffer
  memcpy(scanBuf2, dstPtr, dstWidthBytes);

  // bitwise AND source data with scanline buffer
  rightEdge = dstWidthBytes & 3;
  pDstData = scanBuf2;
  for (i=0; i < (dstWidthBytes >> 2); i++)
  {
    *((DWORD *)pDstData)++ &= *((DWORD *)srcPtr)++;
  }
  for (i=0; i < rightEdge; i++)
  {
    *((BYTE *)pDstData)++ &= *((BYTE *)srcPtr)++;
  }

  // write scanline buffer back to frame buffer
  memcpy(dstPtr, scanBuf2, dstWidthBytes);
}

/*----------------------------------------------------------------------
Function name:  fillScanOR

Description:    primitive to perform a bitwise OR operation on data
----------------------------------------------------------------------*/
void fillScanOR(BYTE *dstPtr, BYTE *srcPtr, DWORD dstWidthBytes)
{
  DWORD rightEdge, i;
  BYTE *pDstData;

  // read frame buffer into scanline buffer
  memcpy(scanBuf2, dstPtr, dstWidthBytes);

  // bitwise OR source data in with scanline buffer
  rightEdge = dstWidthBytes & 3;
  pDstData = scanBuf2;
  for (i=0; i < (dstWidthBytes >> 2); i++)
  {
    *((DWORD *)pDstData)++ |= *((DWORD *)srcPtr)++;
  }
  for (i=0; i < rightEdge; i++)
  {
    *((BYTE *)pDstData)++ |= *((BYTE *)srcPtr)++;
  }

  // write scanline buffer back to frame buffer
  memcpy(dstPtr, scanBuf2, dstWidthBytes);
}

/*----------------------------------------------------------------------
Function name:  fillScanSRC

Description:    primitive to copy source data to destination
----------------------------------------------------------------------*/
void fillScanSRC(BYTE *dstPtr, BYTE *srcPtr, DWORD dstWidthBytes)
{
  memcpy(dstPtr, srcPtr, dstWidthBytes);
}

/*----------------------------------------------------------------------
Function name:  sli_ColorFill

Description:    Fill a rectangular region of a surface.

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall
sli_ColorFill(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD BytesPerPel)
{
    DWORD dstWidth, dstHeight, dstPitch, dstWidthBytes;
    BYTE  *dstPtr;
    LPDDRAWI_DDRAWSURFACE_LCL lpDstSurf;

#ifdef WINNT
    ASSERTDD(DDSCAPS_VIDEOMEMORY == (pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY),
             "sli_ColorFill called with non video memory destination");
#endif

    FXBUSYWAIT(ppdev);

    MODIFY_SLI_READ(ppdev, ENABLE_SLI_READ);

    dstWidth = pbd->rDest.right - pbd->rDest.left;
    dstHeight = pbd->rDest.bottom - pbd->rDest.top;
    lpDstSurf = pbd->lpDDDestSurface;
    dstWidthBytes = dstWidth * BytesPerPel;

    dstPitch = ( IS_TILED(GET_HW_ADDR(lpDstSurf)) ) ?
                _DS(ddTilePitch) : lpDstSurf->lpGbl->lPitch;

    dstPtr = (BYTE *)
        ( lpDstSurf->lpGbl->fpVidMem +
          (pbd->rDest.top * dstPitch) + (pbd->rDest.left * BytesPerPel)
        );
#ifdef WINNT
    // lpDstSurf must be in video memory
    (DWORD)dstPtr += (DWORD)ppdev->pjLfbBase;
#endif

    initBuffer(scanBuf1, BytesPerPel, pbd->bltFX.dwFillColor, dstWidthBytes);

    while (dstHeight--)
    {
        memcpy(dstPtr, scanBuf1, dstWidthBytes);
        dstPtr += dstPitch; // next scan
    }

  MODIFY_SLI_READ(ppdev, DISABLE_SLI_READ);

  pbd->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

} // sli_ColorFill


/*----------------------------------------------------------------------
Function name:  sli_ColorFillZ

Description:    Fill the Z buffer using direct frame buffer write.  This
                routine assumes that we offset the start of the z buffer by
                one tile, and there is an extra row of tiles at the bottom.

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall
sli_ColorFillZ(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD BytesPerPel)
{
    DWORD dstWidth, dstHeight, dstWidthBytes, fillWidth;
    DWORD xStart, xEnd, wrapWidth, hwPitch, linearPitch, xAdvance;
    BYTE  *dstPtr, *wrapPtr;
    LPDDRAWI_DDRAWSURFACE_LCL lpDstSurf;

#ifdef WINNT
    ASSERTDD(DDSCAPS_VIDEOMEMORY == (pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY),
             "sli_ColorFillZ called with non video memory destination");
#endif

    FXBUSYWAIT(ppdev);

    MODIFY_SLI_READ(ppdev, ENABLE_SLI_READ);

    dstWidth = pbd->rDest.right - pbd->rDest.left;
    dstHeight = pbd->rDest.bottom - pbd->rDest.top;
    lpDstSurf = pbd->lpDDDestSurface;
    dstWidthBytes = dstWidth * BytesPerPel;

    hwPitch = _DS(ddTileStride) << SST_TILE_WIDTH_BITS;
    linearPitch = lpDstSurf->lpGbl->lPitch;

    dstPtr = (BYTE *)
        ( lpDstSurf->lpGbl->fpVidMem +
          (pbd->rDest.top * linearPitch) + (pbd->rDest.left * BytesPerPel)
        );
#ifdef WINNT
    // lpDstSurf must be in video memory
    (DWORD)dstPtr += (DWORD)ppdev->pjLfbBase;
#endif

    initBuffer(scanBuf1, BytesPerPel, pbd->bltFX.dwFillColor, dstWidthBytes);

    if ( IS_TILED(GET_HW_ADDR(lpDstSurf)) )
    {
      xStart = pbd->rDest.left * BytesPerPel;
      xEnd = pbd->rDest.right * BytesPerPel;
      wrapWidth = SST_TILE_WIDTH - 
                  (hwPitch - (lpDstSurf->lpGbl->wWidth * BytesPerPel));

      if ( (long)xEnd < (long)(hwPitch - wrapWidth) )
      {
        while (dstHeight--)
        {
          memcpy(dstPtr, scanBuf1, dstWidthBytes);
          dstPtr += linearPitch; // next scan
        }      
      }
      else
      {
        // Special case for pixels that belong to the same scanline but 
        // lie in a tile that is on the next row.

        fillWidth = (dstWidthBytes > wrapWidth)?
                    (hwPitch - wrapWidth - xStart) : 0;
        wrapWidth = dstWidthBytes - fillWidth;

        wrapPtr = (BYTE *) (lpDstSurf->lpGbl->fpVidMem + 
                            (pbd->rDest.top * linearPitch));
#ifdef WINNT
        // lpDstSurf must be in video memory
        wrapPtr += (DWORD)ppdev->pjLfbBase;
#endif

        if (_DD(ddSLIModeEnabled))
        {
          xAdvance = ((linearPitch << SST_TILE_HEIGHT_BITS) * _DS(dwNumUnits))
                      - SST_TILE_WIDTH;
        }
        else
        {
          xAdvance = (linearPitch << SST_TILE_HEIGHT_BITS) - SST_TILE_WIDTH;
        }

        while (dstHeight--)
        {
          memcpy(dstPtr, scanBuf1, fillWidth);
          memcpy((wrapPtr + xAdvance), scanBuf1, wrapWidth);
          dstPtr += linearPitch; // next scan
          wrapPtr += linearPitch; // next scan
        }      
      }
    }
    else // linear Z buffer fill
    {
      while (dstHeight--)
      {
        memcpy(dstPtr, scanBuf1, dstWidthBytes);
        dstPtr += linearPitch; // next scan
      }
    }

    MODIFY_SLI_READ(ppdev, DISABLE_SLI_READ);

    pbd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;

} // sli_ColorFillZ


/*----------------------------------------------------------------------
Function name:  sli_PuntColorFillZ

Description:    Fill the Z buffer using direct frame buffer write.  This
                routine assumes that we offset the start of the z buffer by
                one tile, and there is an extra row of tiles at the bottom.

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall
sli_PuntColorFillZ(NT9XDEVICEDATA *ppdev, PSOLIDCOLORPARAMS pParams)
{
    DWORD dstWidth, dstHeight, dstWidthBytes, fillWidth;
    DWORD xStart, xEnd, wrapWidth, hwPitch, linearPitch, xAdvance;
    BYTE  *dstPtr, *wrapPtr;
    PFILLSCANFUNC fillScan;

    FXBUSYWAIT(ppdev);

    MODIFY_SLI_READ(ppdev, ENABLE_SLI_READ);

    dstWidth = pParams->dstRight - pParams->dstLeft;
    dstHeight = pParams->dstBottom - pParams->dstTop;
    dstWidthBytes = dstWidth * pParams->BytesPerPel;

    hwPitch = _DS(ddTileStride) << SST_TILE_WIDTH_BITS;
    linearPitch = pParams->linearPitch;

    dstPtr = (BYTE *)
        ( pParams->fpVidMem + (pParams->dstTop * linearPitch) + 
          (pParams->dstLeft * pParams->BytesPerPel)
        );
#ifdef WINNT
    // the pParams->fpVidMem value already has pjLfbBase added to it
#endif

    initBuffer(scanBuf1, pParams->BytesPerPel, pParams->fillData, 
      dstWidthBytes);

    switch (pParams->bltRop)
    {
      case ((SSTG_ROP_AND << 16) | (SSTG_ROP_AND << 8) | (SSTG_ROP_AND)):
        fillScan = (PFILLSCANFUNC) &fillScanAND;
        break;
      case ((SSTG_ROP_OR << 16) | (SSTG_ROP_OR << 8) | (SSTG_ROP_OR)):
        fillScan = (PFILLSCANFUNC) &fillScanOR;
        break;
      case ((SSTG_ROP_SRC << 16) | (SSTG_ROP_SRC << 8) | (SSTG_ROP_SRC)):
      default:
        fillScan = (PFILLSCANFUNC) &fillScanSRC;
        break;
    }

    if (pParams->isTiled)
    {
      xStart = pParams->dstLeft * pParams->BytesPerPel;
      xEnd = pParams->dstRight * pParams->BytesPerPel;
      wrapWidth = SST_TILE_WIDTH - 
#ifdef WINNT
                  (hwPitch - (ppdev->cyScreen * pParams->BytesPerPel));
#else
                  (hwPitch - (_DS(hres) * pParams->BytesPerPel));
#endif

      if ( (long)xEnd < (long)(hwPitch - wrapWidth) )
      {
        while (dstHeight--)
        {
          (* fillScan)(dstPtr, scanBuf1, dstWidthBytes);
          dstPtr += linearPitch; // next scan
        }      
      }
      else
      {
        // Special case for pixels that belong to the same scanline but 
        // lie in a tile that is on the next row.

        fillWidth = (dstWidthBytes > wrapWidth)?
                    (hwPitch - wrapWidth - xStart) : 0;
        wrapWidth = dstWidthBytes - fillWidth;

        wrapPtr = (BYTE *) (pParams->fpVidMem + (pParams->dstTop * linearPitch));
#ifdef WINNT
        // the pParams->fpVidMem value already has pjLfbBase added to it
#endif

        if (_DD(ddSLIModeEnabled))
        {
          xAdvance = ((linearPitch << SST_TILE_HEIGHT_BITS) * _DS(dwNumUnits)) - SST_TILE_WIDTH;
        }
        else
        {
          xAdvance = (linearPitch << SST_TILE_HEIGHT_BITS) - SST_TILE_WIDTH;
        }

        while (dstHeight--)
        {
          (* fillScan)(dstPtr, scanBuf1, fillWidth);
          (* fillScan)((wrapPtr + xAdvance), scanBuf1, wrapWidth);
          dstPtr += linearPitch; // next scan
          wrapPtr += linearPitch; // next scan
        }      
      }
    }
    else // linear Z buffer fill
    {
      while (dstHeight--)
      {
        (* fillScan)(dstPtr, scanBuf1, dstWidthBytes);
        dstPtr += linearPitch; // next scan
      }
    }

    MODIFY_SLI_READ(ppdev, DISABLE_SLI_READ);

    return DDHAL_DRIVER_HANDLED;

} // sli_PuntColorFillZ

/*----------------------------------------------------------------------
Function name:  sli_PuntColorFill

Description:    Fill a rectangular region of a surface.

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall 
sli_PuntColorFill(NT9XDEVICEDATA *ppdev, PSOLIDCOLORPARAMS pParams)
{
    DWORD dstWidth, dstHeight, dstPitch, dstWidthBytes;
    BYTE  *dstPtr;

    FXBUSYWAIT(ppdev);

    MODIFY_SLI_READ(ppdev, ENABLE_SLI_READ);

    dstWidth = pParams->dstRight - pParams->dstLeft;
    dstHeight = pParams->dstBottom - pParams->dstTop;
    dstWidthBytes = dstWidth * pParams->BytesPerPel;

    dstPitch = pParams->Pitch;

    dstPtr = (BYTE *)
        ( pParams->fpVidMem +
          (pParams->dstTop * dstPitch) + 
          (pParams->dstLeft * pParams->BytesPerPel)
        );
#ifdef WINNT
    // the pParams->fpVidMem value already has pjLfbBase added to it
#endif

    initBuffer(scanBuf1, pParams->BytesPerPel, pParams->fillData, 
      dstWidthBytes);

    while (dstHeight--)
    {
        memcpy(dstPtr, scanBuf1, dstWidthBytes);
        dstPtr += dstPitch; // next scan
    }

  MODIFY_SLI_READ(ppdev, DISABLE_SLI_READ);

  return DDHAL_DRIVER_HANDLED;

} // sli_PuntColorFill

/*----------------------------------------------------------------------
Function name:  sli_DoBltNoSP

Description:    Handle blts that don't use a source or pattern.

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall
sli_DoBltNoSP(NT9XDEVICEDATA   *ppdev,
              LPDDHAL_BLTDATA   pbd,
              DWORD             rop3,
              DWORD             BytesPerPel)
{
  DWORD dstWidth, dstHeight, dstPitch, dstWidthBytes;
  DWORD rightEdge, i;
  BYTE  *dstPtr, *pData;
  LPDDRAWI_DDRAWSURFACE_LCL lpDstSurf;

#ifdef WINNT
    ASSERTDD(DDSCAPS_VIDEOMEMORY == (pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY),
             "sli_DoBltNoSP called with non video memory destination");
#endif

  FXBUSYWAIT(ppdev);

  MODIFY_SLI_READ(ppdev, ENABLE_SLI_READ);

  if (pbd->dwFlags & DDBLT_KEYDESTOVERRIDE)
  {
    // Software path cannot handle destination colorkey.

    pbd->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_HANDLED;
  }
  else
  {
    switch(rop3)
    {
    case ( HIWORD(BLACKNESS) ):
        pbd->bltFX.dwFillColor = rop3;
        sli_ColorFill(ppdev, pbd, BytesPerPel);
        break;

    case ( HIWORD(WHITENESS) ):
        pbd->bltFX.dwFillColor = 0xffffffff;
        sli_ColorFill(ppdev, pbd, BytesPerPel);
        break;

    case ( HIWORD(DSTINVERT) ):
        dstWidth = pbd->rDest.right - pbd->rDest.left;
        dstHeight = pbd->rDest.bottom - pbd->rDest.top;
        lpDstSurf = pbd->lpDDDestSurface;
        dstWidthBytes = dstWidth * BytesPerPel;

        dstPitch = ( IS_TILED(GET_HW_ADDR(lpDstSurf)) ) ?
            _DS(ddTilePitch) : lpDstSurf->lpGbl->lPitch;

        dstPtr = (BYTE *)
            ( lpDstSurf->lpGbl->fpVidMem +
              (pbd->rDest.top * dstPitch) + (pbd->rDest.left * BytesPerPel)
            );
#ifdef WINNT
        // lpDstSurf must be in video memory
        (DWORD)dstPtr += (DWORD)ppdev->pjLfbBase;
#endif

        rightEdge = dstWidthBytes & 3;

        while (dstHeight--)
        {
            memcpy(scanBuf1, dstPtr, dstWidthBytes);
            pData = scanBuf1;
            for (i=0; i < (dstWidthBytes >> 2); i++)
            {
                *((DWORD *)pData)++ = ~(*((DWORD *)pData));
            }
            for (i=0; i < rightEdge; i++)       // invert right edge
            {
                *((BYTE *)pData)++ = ~(*((BYTE *)pData));
            }
            memcpy(dstPtr, scanBuf1, dstWidthBytes);
            dstPtr += dstPitch;                 // next scan
        }
        break;

    case (0xaa):
        break;
    } // end: switch(rop3)
  } // endif: dst color key specified

  MODIFY_SLI_READ(ppdev, DISABLE_SLI_READ);

  pbd->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

} // sli_DoBltNoSP

/*----------------------------------------------------------------------
Function name:  xsBltSrcToScanBuf

Description:    Apply color keys, then performs pixel replication in X,
                put result in stretch buffer.

Return:         void
----------------------------------------------------------------------*/
void __stdcall xsBltSrcToScanBuf(
    LPDDHAL_BLTDATA pbd, BYTE *srcPtr, BYTE *dstPtr,
    DWORD srcWidth, DWORD dstWidth,
    DWORD srcBytesPerPel, DWORD dstBytesPerPel)
{
    CKEY_TYPE colorKey;
    DWORD srcColorkeyMin, srcColorkeyMax;
    DWORD dstColorkeyMin, dstColorkeyMax, dwData;
    WORD  wData;
    int   twoSrcX, twoDstX, xErr;
    BYTE  *pSrc, *pDst, bData;
    DWORD srcWidthBytes, dstWidthBytes;

    colorKey = none;

    if (pbd->dwFlags & DDBLT_KEYSRCOVERRIDE)
    {
        colorKey = src;
        srcColorkeyMin = pbd->bltFX.ddckSrcColorkey.dwColorSpaceLowValue;
        srcColorkeyMax = pbd->bltFX.ddckSrcColorkey.dwColorSpaceHighValue;
    }

    if (pbd->dwFlags & DDBLT_KEYDESTOVERRIDE)
    {
        colorKey = (src==colorKey)? both : dst;
        dstColorkeyMin = pbd->bltFX.ddckDestColorkey.dwColorSpaceLowValue;
        dstColorkeyMax = pbd->bltFX.ddckDestColorkey.dwColorSpaceHighValue;
    }

    // replicate in X, result is in scan buffer 2

    srcWidthBytes = srcWidth * srcBytesPerPel;
    dstWidthBytes = dstWidth * dstBytesPerPel;
    twoSrcX = (int) (srcWidth + srcWidth);
    twoDstX = (int) (dstWidth + dstWidth);
    xErr = twoSrcX + (int)srcWidth - twoDstX;

    pSrc = (BYTE *)scanBuf1;
    pDst = (BYTE *)scanBuf2;
    memcpy(scanBuf1, srcPtr, srcWidthBytes); // read source into buffer 1
    memcpy(scanBuf2, dstPtr, dstWidthBytes); // read dst into buffer 2

    if (src == colorKey)    // do not write if src pixel matches color key
    {
        switch(srcBytesPerPel)
        {
        case 1: // 8bpp
            bData = *((BYTE *)pSrc);
            while (dstWidth--)
            {
                if ((bData < srcColorkeyMin) || (bData > srcColorkeyMax))
                {
                    *((BYTE *)pDst) = bData;
                }
                ((BYTE *)pDst)++;

                while (xErr >= 0)
                {
                    bData = * (++((BYTE *)pSrc));
                    xErr -= twoDstX;
                }
                xErr += twoSrcX;
            }
            break;

        case 2: // 16bpp
            wData = *((WORD *)pSrc);
            while (dstWidth--)
            {
                if ((wData < srcColorkeyMin) || (wData > srcColorkeyMax))
                {
                    *((WORD *)pDst) = wData;
                }
                ((WORD *)pDst)++;

                while (xErr >= 0)
                {
                    wData = * (++((WORD *)pSrc));
                    xErr -= twoDstX;
                }
                xErr += twoSrcX;
            }
            break;

        case 4: // 32bpp
            dwData = *((DWORD *)pSrc) & 0xffffff;
            while (dstWidth--)
            {
                if ((dwData < srcColorkeyMin) || (dwData > srcColorkeyMax))
                {
                    *((DWORD *)pDst) = dwData;
                }
                ((DWORD *)pDst)++;

                while (xErr >= 0)
                {
                    dwData = * (++((DWORD *)pSrc)) & 0xffffff;
                    xErr -= twoDstX;
                }
                xErr += twoSrcX;
            }
            break;

        } // end: switch(srcBytesPerPel)
    } // endif: use src color key
    else
    {
        if (dst == colorKey ) // update dst. pixel if matches color key
        {
            switch(srcBytesPerPel)
            {
            case 1: // 8bpp
                while (dstWidth--)
                {
                    bData = *((BYTE *)pDst);
                    if ((bData >= dstColorkeyMin) &&
                        (bData <= dstColorkeyMax)
                       )
                    {
                        *((BYTE *)pDst) = *((BYTE *)pSrc);
                    }
                    ((BYTE *)pDst)++;

                    while (xErr >= 0)
                    {
                        ((BYTE *)pSrc)++;
                        xErr -= twoDstX;
                    }
                    xErr += twoSrcX;
                }
                break;

            case 2: // 16bpp
                while (dstWidth--)
                {
                    wData = *((WORD *)pDst);
                    if ((wData >= dstColorkeyMin) &&
                        (wData <= dstColorkeyMax)
                       )
                    {
                        *((WORD *)pDst) = *((WORD *)pSrc);
                    }
                    ((WORD *)pDst)++;

                    while (xErr >= 0)
                    {
                        ((WORD *)pSrc)++;
                        xErr -= twoDstX;
                    }
                    xErr += twoSrcX;
                }
                break;

            case 4: // 32bpp
                while (dstWidth--)
                {
                    dwData = *((DWORD *)pDst);
                    if ((dwData >= dstColorkeyMin) &&
                        (dwData <= dstColorkeyMax)
                       )
                    {
                        *((DWORD *)pDst) = *((DWORD *)pSrc);
                    }
                    ((DWORD *)pDst)++;

                    while (xErr >= 0)
                    {
                        ((DWORD *)pSrc)++;
                        xErr -= twoDstX;
                    }
                    xErr += twoSrcX;
                }
                break;

            } // end: switch(bytesPerPel)
        } // endif: use dst color key
    } // endif: use src color key
}

/*----------------------------------------------------------------------
Function name:  sBltSrcToScanBuf

Description:    Performs pixel replication in X, put result in
                stretch buffer.

Return:         void
----------------------------------------------------------------------*/
void __stdcall sBltSrcToScanBuf(
    LPDDHAL_BLTDATA  pbd, BYTE *srcPtr, BYTE *dstPtr,
    DWORD srcWidth, DWORD dstWidth,
    DWORD srcBytesPerPel, DWORD dstBytesPerPel)
{
    int   twoSrcX, twoDstX, xErr;
    BYTE  *pSrc, *pDst;
    DWORD srcWidthBytes;

    // replicate in X, result is in scan buffer 2

    srcWidthBytes = srcWidth * srcBytesPerPel;
    twoSrcX = (int) (srcWidth + srcWidth);
    twoDstX = (int) (dstWidth + dstWidth);
    xErr = twoSrcX + (int)srcWidth - twoDstX;

    pSrc = (BYTE *)scanBuf1;
    pDst = (BYTE *)scanBuf2;
    memcpy(scanBuf1, srcPtr, srcWidthBytes);

    switch(srcBytesPerPel)
    {
    case 1: // 8bpp
        while (dstWidth--)
        {
            *((BYTE *)pDst)++ = *((BYTE *)pSrc);

            while (xErr >= 0)
            {
                ((BYTE *)pSrc)++;
                xErr -= twoDstX;
            }
            xErr += twoSrcX;
        }
        break;

    case 2: // 16bpp
        while (dstWidth--)
        {
            *((WORD *)pDst)++ = *((WORD *)pSrc);

            while (xErr >= 0)
            {
                ((WORD *)pSrc)++;
                xErr -= twoDstX;
            }
            xErr += twoSrcX;
        }
        break;

    case 4: // 32bpp
        while (dstWidth--)
        {
            *((DWORD *)pDst)++ = *((DWORD *)pSrc);

            while (xErr >= 0)
            {
                ((DWORD *)pSrc)++;
                xErr -= twoDstX;
            }
            xErr += twoSrcX;
        }
        break;

    } // end: switch(srcBytesPerPel)
}

/*----------------------------------------------------------------------
Function name:  sBltS

Description:    Performs pixel replication for stretch blt.

Return:         void
----------------------------------------------------------------------*/
void __stdcall sBltS(BLT_PARAMS)
{
    int   twoSrcY, twoDstY, yErr;
    DWORD srcWidth, dstWidth, srcHeight;
    PSBLTFUNC pfn;

    srcWidth = pbd->rSrc.right - pbd->rSrc.left;
    dstWidth = pbd->rDest.right - pbd->rDest.left;
    srcHeight = pbd->rSrc.bottom - pbd->rSrc.top;

    twoSrcY = (int) (srcHeight + srcHeight);
    twoDstY = (int) (dstHeight + dstHeight);
    yErr = twoSrcY + (int)srcHeight - twoDstY;

    if ( (pbd->dwFlags & DDBLT_KEYSRCOVERRIDE) ||
         (pbd->dwFlags & DDBLT_KEYDESTOVERRIDE) )
    {
        pfn = (PSBLTFUNC) &xsBltSrcToScanBuf;   // apply color keys
    }
    else
    {
        pfn = (PSBLTFUNC) &sBltSrcToScanBuf;    // no color keys
    }

    // stretch or shrink one scan, store result in scanline buffer

    (*pfn)(pbd, srcPtr, dstPtr, srcWidth, dstWidth, srcBytesPerPel, dstBytesPerPel);

    while (dstHeight--)
    {
        memcpy(dstPtr, scanBuf2, dstWidthBytes); // replicate in Y
        dstPtr += dstPitch;

        while (yErr >= 0)
        {
            srcPtr += srcPitch; // advance to next source scan
            yErr -= twoDstY;
        }

        // stretch or shrink next scan, store result in scanline buffer
        (*pfn)(pbd, srcPtr, dstPtr, srcWidth, dstWidth, srcBytesPerPel, dstBytesPerPel);

        yErr += twoSrcY;

    } // while (dstHeight--)
}

/*----------------------------------------------------------------------
Function name:  sli_DoBltS

Description:    Handle blts that use a source only, no pattern. Source
                can be from video memory or system memory. Destination
                is currently only in video memory.

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
sli_DoBltS(NT9XDEVICEDATA   *ppdev,
           LPDDHAL_BLTDATA  pbd,
           DWORD            rop3,
           DWORD            srcPixelFormat,
           DWORD            dstPixelFormat,
           BLT_TYPE         blt_type)
{
  DWORD dstWidth, dstHeight, srcWidth, srcHeight;
  DWORD srcWidthBytes, dstWidthBytes;
  DWORD srcBytesPerPel, dstBytesPerPel, srcPitch, dstPitch, i;
  BYTE  *dstPtr, *srcPtr;
  LPDDRAWI_DDRAWSURFACE_LCL lpDstSurf, lpSrcSurf;

#ifdef WINNT
    ASSERTDD(DDSCAPS_VIDEOMEMORY == (pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY),
             "sli_DoBltS called with non video memory destination");
#endif

  FXBUSYWAIT(ppdev);

  MODIFY_SLI_READ(ppdev, ENABLE_SLI_READ);

  if (srcPixelFormat != dstPixelFormat)
  {
    // Software path cannot handle format conversion.

    pbd->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_HANDLED;
  }

  GET_BYTE_DEPTH(srcBytesPerPel, srcPixelFormat);
  GET_BYTE_DEPTH(dstBytesPerPel, dstPixelFormat);

  srcWidth = pbd->rSrc.right - pbd->rSrc.left;
  srcHeight = pbd->rSrc.bottom - pbd->rSrc.top;
  lpSrcSurf = pbd->lpDDSrcSurface;
  srcWidthBytes = srcWidth * srcBytesPerPel;

  if ( (vid2vid == blt_type) && IS_TILED(GET_HW_ADDR(lpSrcSurf)) )
  {
    srcPitch = _DS(ddTilePitch); // valid only if source is from video mem.
  }
  else
  {
    srcPitch = lpSrcSurf->lpGbl->lPitch;
  }

  dstWidth = pbd->rDest.right - pbd->rDest.left;
  dstHeight = pbd->rDest.bottom - pbd->rDest.top;
  lpDstSurf = pbd->lpDDDestSurface;
  dstWidthBytes = dstWidth * dstBytesPerPel;

  dstPitch = ( IS_TILED(GET_HW_ADDR(lpDstSurf)) ) ?
            _DS(ddTilePitch) : lpDstSurf->lpGbl->lPitch;

  // 
  // Only check for overlap if the src and dst are the same surface
  // Check for overlaps. Since we are reading source into a scanline buffer
  // for performance reasons, we do not need to worry about overlaping in
  // the x direction.

  if ((lpSrcSurf == lpDstSurf) &&
      (pbd->rDest.top >= pbd->rSrc.top) &&
      (pbd->rDest.top <= pbd->rSrc.bottom))
  {
    // copy from bottom up

    srcPtr = (BYTE *)
        ( lpSrcSurf->lpGbl->fpVidMem + ((pbd->rSrc.bottom - 1) * srcPitch) +
          (pbd->rSrc.left * srcBytesPerPel)
        );
#ifdef WINNT
    if (DDSCAPS_VIDEOMEMORY & lpSrcSurf->ddsCaps.dwCaps)
      (DWORD)srcPtr += (DWORD)ppdev->pjLfbBase;
#endif
    dstPtr = (BYTE *)
        ( lpDstSurf->lpGbl->fpVidMem + ((pbd->rDest.bottom - 1) * dstPitch) +
          (pbd->rDest.left * dstBytesPerPel)
        );
#ifdef WINNT
    // lpDstSurf must be in video memory
    (DWORD)dstPtr += (DWORD)ppdev->pjLfbBase;
#endif
    srcPitch = 0 - srcPitch;
    dstPitch = 0 - dstPitch;
  }
  else
  {
    srcPtr = (BYTE *)
        ( lpSrcSurf->lpGbl->fpVidMem +
          (pbd->rSrc.top * srcPitch) + (pbd->rSrc.left * srcBytesPerPel)
        );
#ifdef WINNT
    if (DDSCAPS_VIDEOMEMORY & lpSrcSurf->ddsCaps.dwCaps)
      (DWORD)srcPtr += (DWORD)ppdev->pjLfbBase;
#endif
    dstPtr = (BYTE *)
        ( lpDstSurf->lpGbl->fpVidMem +
          (pbd->rDest.top * dstPitch) + (pbd->rDest.left * dstBytesPerPel)
        );
#ifdef WINNT
    // lpDstSurf must be in video memory
    (DWORD)dstPtr += (DWORD)ppdev->pjLfbBase;
#endif
  }

  // check for stretch/shrink blt

  i = rop3 & 0xf;
  if ( (srcWidth == dstWidth) && (srcHeight == dstHeight) )
  {
    if ( (pbd->dwFlags & DDBLT_KEYSRCOVERRIDE) ||
         (pbd->dwFlags & DDBLT_KEYDESTOVERRIDE) )
    {
        (* xBltFuncTab[i])(pbd, dstWidthBytes, dstHeight, srcPitch, dstPitch,
            srcPtr, dstPtr, srcBytesPerPel, dstBytesPerPel);
    }
    else
    {
        (* BltFuncTab[i])(pbd, dstWidthBytes, dstHeight, srcPitch, dstPitch,
            srcPtr, dstPtr, srcBytesPerPel, dstBytesPerPel);
    }
  }
  else // stretch/shrink blt
  {
        (* sBltFuncTab[i])(pbd, dstWidthBytes, dstHeight, srcPitch, dstPitch,
            srcPtr, dstPtr, srcBytesPerPel, dstBytesPerPel);

  } // endif: non stretch/shrink blt

  MODIFY_SLI_READ(ppdev, DISABLE_SLI_READ);

  pbd->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

} // sli_DoBltS

/*----------------------------------------------------------------------
Function name:  sli_3DDoBltS

Description:    Blt a surface from linear video memory to tiled
                video memory using the 3D engine.  The surface
                must have a total width that's equal to a power of 2.

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
sli_3DDoBltS(NT9XDEVICEDATA   *ppdev,
             LPDDHAL_BLTDATA  pbd,
             DWORD            rop3,
             DWORD            srcPixelFormat,
             DWORD            dstPixelFormat)
{
  DWORD                     tLOD,lodmax, dwSrcSize, aspectRatio;
  int                       addrOffset;
  float                     rSrcX0, rSrcY0, rSrcX1, rSrcY1, rDestX0, rDestY0, rDestX1, rDestY1;
  DWORD                     dstWidth, dstHeight, srcWidth, srcHeight;
  DWORD                     srcSurfWidth, srcSurfHeight, dstPitch;
  DWORD                     srcBytesPerPel, dstBytesPerPel;
  DWORD                     dstAddr, srcAddr;
  DWORD                     texFormat, dstFormat, fbzMode, chromaKey, chromaRange;
  LPDDRAWI_DDRAWSURFACE_LCL lpDstSurf, lpSrcSurf;


  tLOD = 0;
  lodmax = 0;

  // Get current source surface information
  GET_BYTE_DEPTH(srcBytesPerPel, srcPixelFormat);
  lpSrcSurf = pbd->lpDDSrcSurface;
  srcWidth = (DWORD) lpSrcSurf->lpGbl->wWidth;
  srcHeight = (DWORD) lpSrcSurf->lpGbl->wHeight;
  srcAddr = GET_HW_ADDR(lpSrcSurf);
  
  // Get current destination surface information
  GET_BYTE_DEPTH(dstBytesPerPel, dstPixelFormat);
  lpDstSurf = pbd->lpDDDestSurface;
  dstWidth = (DWORD) lpDstSurf->lpGbl->wWidth;
  dstHeight = (DWORD) lpDstSurf->lpGbl->wHeight;
  dstAddr = GET_HW_ADDR(lpDstSurf) & ~(0x80000000);

  // We know it's tiled, since the other cases are
  // handled in DDSli2DScn2Scn(), but I'll leave this
  // in for future use.
  //if(IS_TILED(GET_HW_ADDR(lpDstSurf)))
    dstPitch = SST_BUFFER_MEMORY_TILED | _FF(ddTileStride);
  //else
  //  dstPitch = lpDstSurf->lpGbl->lPitch;

  // Calculate the new Width and Height
  for (srcSurfWidth=0x1; srcSurfWidth < srcWidth; srcSurfWidth<<=1);
  for (srcSurfHeight=0x1; srcSurfHeight < srcHeight; srcSurfHeight<<=1);

  if (srcSurfWidth > srcSurfHeight)
  {
    for (lodmax = 0; srcSurfWidth > (DWORD)(1 << lodmax); lodmax++);

    tLOD |=SST_LOD_S_IS_WIDER;

    for (aspectRatio = 0; (DWORD)(1 << aspectRatio ) < (srcSurfWidth / srcSurfHeight); aspectRatio++);

    // We must handle > 1:8 aspect ratios.
    if(aspectRatio > 3)
    {
      srcSurfHeight = srcSurfHeight << (aspectRatio-3);
      aspectRatio = 3;
    }

    // Calculate the source coordinates
    rSrcX0 = (float) (pbd->rSrc.left * 256) / (float)srcSurfWidth;
    rSrcY0 = (float) (pbd->rSrc.top * 256) / (float)srcSurfWidth;
    rSrcX1  = (float)(pbd->rSrc.right * 256) / (float)srcSurfWidth;
    rSrcY1 =  (float)(pbd->rSrc.bottom * 256) / (float)srcSurfWidth;
  }
  else
  {
    for (lodmax = 0; srcSurfHeight >(DWORD)(1 << lodmax); lodmax++);
    for (aspectRatio= 0; (DWORD)(1 << aspectRatio) < (srcSurfHeight / srcSurfWidth); aspectRatio++);

    // This seems to be the best solution for now
    if(aspectRatio > 3)
    {
      return sli_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat, vid2vid);
    }

    // Calculate the source coordinates
    rSrcX0 = (float) (pbd->rSrc.left * 256) / (float)srcSurfHeight;
    rSrcY0 = (float) (pbd->rSrc.top * 256) / (float)srcSurfHeight;
    rSrcX1 = (float)(pbd->rSrc.right * 256 ) / (float)srcSurfHeight;
    rSrcY1 = (float)(pbd->rSrc.bottom * 256) / (float)srcSurfHeight;
  }

  tLOD |= (aspectRatio) << SST_LOD_ASPECT_SHIFT;

  //big texture case
  if (lodmax > 8)
  {
    dwSrcSize = srcSurfWidth * srcSurfHeight * srcBytesPerPel;  //in byte
    lodmax = 11 - lodmax;
    tLOD |= SST_TBIG;
    tLOD |= (lodmax << (SST_LODMIN_SHIFT +2 )) |
            (lodmax << (SST_LODMAX_SHIFT +2 ));
    
    //changes it so that if max LOD = 10 ( 2K texture) lodmax = 2;
    lodmax = 3 - lodmax;
    
    for( addrOffset = 0; lodmax > 0 ; lodmax--)
    {
      addrOffset += dwSrcSize >>  (2 * (lodmax-1));
    }
  }
  else
  {
    // find the size of lod 3 ( 0 for H4)
    dwSrcSize = 256 * ( 256 >> aspectRatio) * srcBytesPerPel;  //in byte
    
    lodmax = 8 - lodmax;
    tLOD |= ( lodmax << (SST_LODMIN_SHIFT +2 )) |
            ( lodmax << (SST_LODMAX_SHIFT +2 ));
    
    for( addrOffset = 0; lodmax > 0; lodmax--)
    {
      addrOffset -= dwSrcSize >> (2 *(lodmax-1));
    }
  }

  srcAddr += addrOffset;
  srcAddr = (srcAddr & 0x1FFFFFF) + ((srcAddr & 0x2000000) >> 24);
  
  // store the destination coordinates
  rDestX0 = (float) pbd->rDest.left;
  rDestY0 = (float) pbd->rDest.top;
  rDestX1 = (float) pbd->rDest.right;
  rDestY1 = (float) pbd->rDest.bottom;
  
  if(srcBytesPerPel == 2)
    texFormat = TEXFMT_RGB_565;
  else
    texFormat = TEXFMT_ARGB_8888;
  
  if(dstBytesPerPel == 2)
    dstFormat = SST_RM_16BPP;
  else
    dstFormat = SST_RM_32BPP;
  
  // Now we need to clip if dest rect is outside
  // Don't need to!  I just tested this. DirectDraw will not pass us
  // the blt call if the Dest Rectangle is outside the destination
  // surface rectangle!
  // test for colorkeying
  fbzMode = SST_RGBWRMASK;

  if (pbd->dwFlags & DDBLT_KEYSRCOVERRIDE)
  {
    fbzMode |= SST_ENCHROMAKEY;
    chromaKey = pbd->bltFX.ddckSrcColorkey.dwColorSpaceLowValue;
    chromaRange = pbd->bltFX.ddckSrcColorkey.dwColorSpaceHighValue;
    
    if(srcBytesPerPel == 2)
    {
      chromaKey = ((chromaKey & 0xf800) << 8) | ((chromaKey & 0xe000) << 3) |
                  ((chromaKey & 0x7e0)  << 5) | ((chromaKey & 0x600)  >> 2) |
                  ((chromaKey & 0x1f)   << 3) | ((chromaKey & 0x1c)   >> 2);
      chromaRange = ((chromaRange & 0xf800) << 8) | ((chromaRange & 0xe000) << 3) |
                    ((chromaRange & 0x7e0)  << 5) | ((chromaRange & 0x600)  >> 2) |
                    ((chromaRange & 0x1f)   << 3) | ((chromaRange & 0x1c)   >> 2);
    }

    if(chromaKey != chromaRange)
      chromaRange |= SST_ENCHROMARANGE;
  }

  {
    CMDFIFO_PROLOG(cmdFifo);

    if (pbd->dwFlags & DDBLT_KEYSRCOVERRIDE)
    {
      CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 2) );
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 2, 1, chromaKey, 0x1 ) );
      SETPD( cmdFifo, ghw->chromaKey, chromaKey );
      SETPD( cmdFifo, ghw->chromaRange, chromaRange );
    }

    CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + 20 + 7 * PH1_SIZE + 12 + 3 * ( PH4_SIZE +3));
    
    // Reset the clipping registers
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, clipLeftRight, 0xf ) );
    SETPD( cmdFifo, ghw0->clipLeftRight, lpDstSurf->lpGbl->wWidth);
    SETPD( cmdFifo, ghw0->clipBottomTop, lpDstSurf->lpGbl->wHeight);
    // let setDX6State know about a buffer addr change    
    _D3(last).clipLeftRight = lpDstSurf->lpGbl->wWidth;
    _D3(last).clipBottomTop = lpDstSurf->lpGbl->wHeight;
    
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, colBufferAddr, 0x0 ) );
    SETPD( cmdFifo, ghw0->colBufferAddr,   dstAddr);
    SETPD( cmdFifo, ghw0->colBufferStride, dstPitch);
    // let setDX6State know about a buffer addr change
    _D3(last).colBufferAddr = dstAddr;
    
    //set textureMode for TMU0
    //loads texture and enbales point sampled filter
    SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R3, textureMode, TMU2CHIP(TREX0)));
    SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->textureMode,
                    SST_TCLAMPS | SST_TCLAMPT |
                    (texFormat << SST_TFORMAT_SHIFT)|
                    SST_TC_REPLACE | SST_TCA_REPLACE |
                    SST_TMINFILTER | SST_TMAGFILTER);
    SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->tLOD, tLOD );
    SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->texBaseAddr, srcAddr);
    
    // filtered, no fog, no alpha blending, no z buffering
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 4, 1, fbzColorPath, 0xf ) );
    SETPD( cmdFifo, ghw->fbzColorPath, SST_ENTEXTUREMAP | SST_PARMADJUST |
                              SST_RGBAZ_CLAMP );
    SETPD( cmdFifo, ghw->fogMode, 0 );
    SETPD( cmdFifo, ghw->alphaMode, 0 );
    SETPD( cmdFifo, ghw->fbzMode, fbzMode );
    
    //need a NOP
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, 0, nopCMD, 0xF));
    SETPD( cmdFifo, ghw->nopCMD,0);
    
    //combineMode for LFB
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 1 ) );
    SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode,
                    SST_CM_CC_OTHERSELECT_TRGB |
                    SST_CM_USE_COMBINE_MODE );
    //combineMode for TMU0
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 2 ) );
    SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU0)->combineMode,
                    SST_CM_TC_LOCALSELECT_LOCAL_TRGB |
                    SST_CM_USE_COMBINE_MODE );
    //combineMode for TMU1
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 4 ) );
    SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU1)->combineMode,
                    SST_CM_TC_LOCALSELECT_LOCAL_TRGB |
                    SST_CM_USE_COMBINE_MODE );
    
    SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R1|R2, renderMode, 0 ) );
    SETPD( cmdFifo, ghw->renderMode, dstFormat            |
    						                     SST_RM_YORIGIN_SELECT|
                                     SST_RM_RED_WMASK     |
                                     SST_RM_GREEN_WMASK   |
                                     SST_RM_BLUE_WMASK );
    SETPD( cmdFifo, ghw->stencilMode, SST_STENCIL_MODE_DISABLE);
    SETPD( cmdFifo, ghw->stencilOp, 0); // D3DSTENCILOP_KEEP
    
    // Send down coordinates
    SETPH( cmdFifo, CMDFIFO_BUILD_PK3( CMD_START, 4, (SST_SETUP_ST0 | SST_SETUP_W0 |SST_SETUP_FAN), 1) );
    SETFPD( cmdFifo, ghw0->sVx,   rDestX0 );
    SETFPD( cmdFifo, ghw0->sVy,   rDestY0 );
    SETFPD( cmdFifo, ghw0->sOow0, 1.0f );
    SETFPD( cmdFifo, ghw0->sSow0, rSrcX0 );
    SETFPD( cmdFifo, ghw0->sTow0, rSrcY0 );
    
    SETFPD( cmdFifo, ghw0->sVx,   rDestX0 );
    SETFPD( cmdFifo, ghw0->sVy,   rDestY1 );
    SETFPD( cmdFifo, ghw0->sOow0, 1.0f );
    SETFPD( cmdFifo, ghw0->sSow0, rSrcX0 );
    SETFPD( cmdFifo, ghw0->sTow0, rSrcY1);
    
    SETFPD( cmdFifo, ghw0->sVx,   rDestX1 );
    SETFPD( cmdFifo, ghw0->sVy,   rDestY1 );
    SETFPD( cmdFifo, ghw0->sOow0, 1.0f );
    SETFPD( cmdFifo, ghw0->sSow0, rSrcX1 );
    SETFPD( cmdFifo, ghw0->sTow0, rSrcY1 )
    
    SETFPD( cmdFifo, ghw0->sVx,   rDestX1 );
    SETFPD( cmdFifo, ghw0->sVy,   rDestY0 );
    SETFPD( cmdFifo, ghw0->sOow0, 1.0f );
    SETFPD( cmdFifo, ghw0->sSow0, rSrcX1 );
    SETFPD( cmdFifo, ghw0->sTow0, rSrcY0 );
    
    CMDFIFO_EPILOG(cmdFifo);
  }

  // Need to tell Direct3D driver that stuff has changed!
  if (_D3(lastContext)!= 0)
  {
    RC *pRc = (RC *)_D3(lastContext);
    UPDATE_HW_STATE(SC_SOMETHING | SC_BUFFERS | SC_FOG | SC_TEXTUREFACTOR | SC_ZBIAS | SC_NEED_NOP | SC_TLOD | SC_ALPHABLEND);
    UPDATE_FOG_STATE( SC_FOGALL );
  }
  
  if(pbd->dwFlags & DDBLT_WAIT)
    FXBUSYWAIT(ppdev);   //wait for blt done
  
  pbd->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;
} // sli_3DDoBltS

/*----------------------------------------------------------------------
Function name:  PromoteSrcToPower2Width

Description:    Promote Source Surface to a power of 2 width for
                sli_3DDoBltS() function

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
PromoteSrcToPower2Width(NT9XDEVICEDATA  *ppdev,
                        LPDDHAL_BLTDATA pbd)
{
  FXSURFACEDATA               *surfaceData, *oldSurfaceData;
  DWORD                       pWidth, bWidth, height, pixelByteDepth;
  DWORD                       pNewWidth, bNewWidth, pNewHeight;
  DWORD                       tileFlag;
  LPDDRAWI_DDRAWSURFACE_LCL   psurf;
  LPDDRAWI_DDRAWSURFACE_GBL   psurf_gbl;
  DWORD                       bltDstFormat, bltDstSize ;
  DWORD                       bltRop, bltCommand;
  DWORD                       bltSrcFormat;


  psurf = pbd->lpDDSrcSurface;
  psurf_gbl = psurf->lpGbl;
  pWidth = psurf_gbl->wWidth;
  height = psurf_gbl->wHeight;
  
  // Now find out the new width
  for(pNewWidth = 1; pNewWidth < pWidth; pNewWidth <<= 1);
  for(pNewHeight = 1; pNewHeight < height; pNewHeight <<=1 );
  
  // Test for > 1:8 width to height ratios
  if((pNewHeight/pNewWidth) > 8)
  {
    D3DPRINT(0,"PromoteSrcToPower2Width: Don't handle height/width ratios greater than 8");
    pbd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
    return DDHAL_DRIVER_HANDLED;
  }

  if(psurf_gbl->ddpfSurface.dwFlags & DDPF_RGB)
    pixelByteDepth = (DWORD)(psurf_gbl->ddpfSurface.dwRGBBitCount) >> 3;
  else
  {
    D3DPRINT(0,"PromoteSrcToPower2Width: Don't handle anything but RGB surfaces.");
    // Not really Out of Video Memory 
    pbd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
    return DDHAL_DRIVER_HANDLED;
  }

  bWidth = pWidth * pixelByteDepth;
  bNewWidth = pNewWidth * pixelByteDepth;

  surfaceData = (FXSURFACEDATA*) DDMALLOCZ(sizeof(FXSURFACEDATA), 0);
  
  if(!surfaceData)
  {
    D3DPRINT(0,"PromoteSrcToPower2Width: FXSURFACEDATA allocation failed. Aborting request.");
    pbd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
    return DDHAL_DRIVER_HANDLED;
  }

  // Initialize the new surfaceData structure
  surfaceData->heapID = HEAP_INVALID;
  surfaceData->AAheapID = HEAP_INVALID;
  surfaceData->overlayShrinkFlag = FALSE;
  surfaceData->doShrink = FALSE;
  surfaceData->inFlipChain = FALSE;

  // Allocate new surfaces from memory manager.
  pbd->ddRVal = memMgr_allocSurface(ppdev,
                                    psurf->ddsCaps.dwCaps,           // [IN] DirectDraw surface capabilities
                                    (DWORD) bNewWidth,               // [IN] width in bytes
                                    (DWORD) pNewHeight,              // [IN] height in scanlines
                                    (DWORD) (bNewWidth + 0x7F)>> 7L, // [IN] width in tiles
                                    (height + 0x1F)>> 5L,            // [IN] height in tiles
                                    &(surfaceData->lfbPtr),          // [OUT] lfb address of surface
                                    &(surfaceData->hwPtr),           // [OUT] hardware offset of surface
                                    &(surfaceData->lPitch),          // [OUT] hardware pitch of surface
                                    &tileFlag,                       // [OUT] MEM_IN_TILED or MEM_IN_LINEAR
                                    &(surfaceData->heapID),          // [OUT] DirectDraw heap number
                                    &(surfaceData->pvmHeap));
  
  // Check if surface allocated.
  if (DD_OK != pbd->ddRVal)
  {
    D3DPRINT(0,"PromoteSrcToPower2Width: Out of Video Memory. Aborting request. %s",
             (tileFlag == MEM_IN_TILED) ? "Tiled memory" : "Linear memory");
    // Remove the newly created memory structure
    DDFREE((void*)surfaceData);
    pbd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
    return DDHAL_DRIVER_HANDLED;
  }

  // Wait for any other 2D/3D commands to finish
  FXBUSYWAIT(ppdev);

  // Store the old surfacedata pointer
  oldSurfaceData = (FXSURFACEDATA*) (pbd->lpDDSrcSurface->lpGbl->dwReserved1);
  
  if (pixelByteDepth == 2)
  {
    BLTFMT(surfaceData->lPitch, SSTG_PIXFMT_16BPP, bltDstFormat);
    BLTFMT(oldSurfaceData->lPitch, SSTG_PIXFMT_16BPP, bltSrcFormat);
  }
  else
  {
    BLTFMT(surfaceData->lPitch, SSTG_PIXFMT_32BPP, bltDstFormat);
    BLTFMT(oldSurfaceData->lPitch, SSTG_PIXFMT_32BPP, bltSrcFormat);
  }
  
  BLTSIZE(pWidth, height, bltDstSize);
  bltRop = (SSTG_ROP_SRC << 16 )| (SSTG_ROP_SRC << 8 ) | SSTG_ROP_SRC;
  bltCommand = SSTG_ROP_SRC << SSTG_ROP0_SHIFT;
  bltCommand |= SSTG_BLT | SSTG_GO | SSTG_CLIPSELECT;

  {
    CMDFIFO_PROLOG(hwPtr);

    // 2 NOPs for safety
    CMDFIFO_CHECKROOM(hwPtr, 4);
    SETPH( hwPtr, CMDFIFO_BUILD_PK1(1, 0, nopCMD, 0xF));
    SETPD( hwPtr, ghw->nopCMD,0);
    SETPH( hwPtr, CMDFIFO_BUILD_PK1(1, 0, nopCMD, 0xF));
    SETPD( hwPtr, ghw->nopCMD,0);
    BUMP(4);
    
    // Now let's copy over the old surface to the new one
    CMDFIFO_CHECKROOM(hwPtr, 13);
    SETPH( hwPtr, CMDFIFO_BUILD_PK2( dstBaseAddrBit
                                    | dstFormatBit
                                    | ropBit
                                    | srcBaseAddrBit
                                    | clip1minBit
                                    | clip1maxBit
                                    | srcFormatBit
                                    | srcSizeBit
                                    | srcXYBit
                                    | dstSizeBit
                                    | dstXYBit
                                    | commandBit ) );
    SETPD(hwPtr, ghw2D->dstBaseAddr, surfaceData->hwPtr);
    SETPD(hwPtr, ghw2D->dstFormat, bltDstFormat);
    SETPD(hwPtr, ghw2D->rop, bltRop );
    SETPD(hwPtr, ghw2D->srcBaseAddr, oldSurfaceData->hwPtr);
    SETPD(hwPtr, ghw2D->clip1min, 0);
    SETPD(hwPtr, ghw2D->clip1max, bltDstSize);
    SETPD(hwPtr, ghw2D->srcFormat, bltSrcFormat);
    SETPD(hwPtr, ghw2D->srcSize, bltDstSize);
    SETPD(hwPtr, ghw2D->srcXY,0);
    SETPD(hwPtr, ghw2D->dstSize, bltDstSize);
    SETPD(hwPtr, ghw2D->dstXY,0);
    SETPD(hwPtr, ghw2D->command, bltCommand);
    BUMP(13);
    
    // 2 NOPs for safety
    CMDFIFO_CHECKROOM(hwPtr, 4);
    SETPH( hwPtr, CMDFIFO_BUILD_PK1(1, 0, nopCMD, 0xF));
    SETPD( hwPtr, ghw->nopCMD,0);
    SETPH( hwPtr, CMDFIFO_BUILD_PK1(1, 0, nopCMD, 0xF));
    SETPD( hwPtr, ghw->nopCMD,0);
    BUMP(4);
    
    CMDFIFO_EPILOG(hwPtr);
  }

  // Let go of the old surface memory
  memMgr_freeSurface(ppdev,
                     psurf->ddsCaps.dwCaps,              // [IN] DirectDraw surface capabilities
                     oldSurfaceData->lfbPtr,             // [IN] lfb address of surface
                     oldSurfaceData->hwPtr,              // [IN] hardware offset of surface
                     GETMEMTYPE(oldSurfaceData->hwPtr),  // [IN] MEM_IN_TILED or MEM_IN_LINEAR
                     oldSurfaceData->heapID,             // [IN] DirectDraw heap number
                     oldSurfaceData->pvmHeap);
  
  // Remove the old surfacedata memory structure
  DDFREE((void*)oldSurfaceData);
  
  // update the global data structure
  psurf_gbl->dwReserved1 = (DWORD)surfaceData;
  UPDATE_BLOCK_DATA(surfaceData, &psurf_gbl->dwReserved1);
  psurf_gbl->fpVidMem = surfaceData->lfbPtr;
  psurf_gbl->lPitch = surfaceData->lPitch;
  
  FXBUSYWAIT(ppdev);   //wait for blt done
  
  return DDHAL_DRIVER_HANDLED;
}  // PromoteSrcToPower2Width

#endif

