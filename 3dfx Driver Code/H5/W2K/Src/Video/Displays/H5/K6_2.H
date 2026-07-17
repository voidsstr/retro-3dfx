#ifndef K6_2_H
#define K6_2_H

// ***********************************************************************************
// Include file for K6-2 (MMX+3DNow!) optimization
// ***********************************************************************************

#ifdef K6_2

#include <windows.h>
#include "d3global.h"

extern BOOL __cdecl detect3DX(void);

extern BOOL ADJUST3DNOWPOINTERS(NT9XDEVICEDATA *ppdev);

// fogGenerateLinear from d3dhw\d3fog.c (being used in WB99)
#define FOG_TABLE_SIZE 64
extern void (*fogGenerateLinear)( unsigned char fogtable[FOG_TABLE_SIZE], RC *pRc );
void fogGenerateLinear_Orig( unsigned char fogtable[FOG_TABLE_SIZE], RC *pRc );
void fogGenerateLinear_K62O( unsigned char fogtable[FOG_TABLE_SIZE], RC *pRc );

// We dont want DP-style (old DX5 functions) optimized in Banshee driver
#ifdef _3DNOWDPFUNCS

// fpTriIZT from d3dhw\d3tri.c
typedef void  __stdcall fpTriIZT_Type( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );

extern fpTriIZT_Type* fpTriIZT;
void  __stdcall fpTriIZT_Orig( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );
void  __stdcall fpTriIZT_K62O( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );

// fpTriIZTH from d3dhw\d3tri.c
typedef void  __stdcall fpTriIZTH_Type( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );

extern fpTriIZTH_Type* fpTriIZTH;
void  __stdcall fpTriIZTH_Orig( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );
void  __stdcall fpTriIZTH_K62O( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );

// fpTriIT from d3dhw\d3tri.c
typedef void  __stdcall fpTriIT_Type( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );

extern fpTriIT_Type* fpTriIT;
void  __stdcall fpTriIT_Orig( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );
void  __stdcall fpTriIT_K62O( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );

// fpTriIZ from d3dhw\d3tri.c
typedef void  __stdcall fpTriIZ_Type( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );

extern fpTriIZ_Type* fpTriIZ;
void  __stdcall fpTriIZ_Orig( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );
void  __stdcall fpTriIZ_K62O( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );

// fpTriIZS from d3dhw\d3tri.c
typedef void  __stdcall fpTriIZS_Type( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );

extern fpTriIZS_Type* fpTriIZS;
void  __stdcall fpTriIZS_Orig( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );
void  __stdcall fpTriIZS_K62O( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );

// fpTriIZTS from d3dhw\d3tri.c
typedef void  __stdcall fpTriIZTS_Type( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );

extern fpTriIZTS_Type* fpTriIZTS;
void  __stdcall fpTriIZTS_Orig( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );
void  __stdcall fpTriIZTS_K62O( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );

// Draw indexed primitives

typedef VOID (__stdcall *DPFXN)(RC *, DWORD, WORD *, LPD3DTLVERTEX );

extern void __stdcall dpDrawIndexedTriangleIZ_K62O(RC* ,DWORD ,WORD* ,LPD3DTLVERTEX );
extern void __stdcall dpDrawIndexedTriangleIZT_K62O(RC* ,DWORD ,WORD* ,LPD3DTLVERTEX );
extern void __stdcall dpDrawIndexedTriangleIZTH_K62O(RC* ,DWORD ,WORD* ,LPD3DTLVERTEX );

extern void __stdcall dpDrawTriangleIZ_K62O(RC* ,DWORD ,WORD* ,LPD3DTLVERTEX );
extern void __stdcall dpDrawTriangleIZT_K62O(RC* ,DWORD ,WORD* ,LPD3DTLVERTEX );
extern void __stdcall dpDrawTriangleIZTH_K62O(RC* ,DWORD ,WORD* ,LPD3DTLVERTEX );

extern void __stdcall dpDrawFanIZ_K62O(RC* ,DWORD ,WORD* ,LPD3DTLVERTEX );
extern void __stdcall dpDrawFanIZT_K62O(RC* ,DWORD ,WORD* ,LPD3DTLVERTEX );
extern void __stdcall dpDrawFanIZTH_K62O(RC* ,DWORD ,WORD* ,LPD3DTLVERTEX );

extern void __stdcall dpDrawStripIZTH_K62O(RC* ,DWORD ,WORD* ,LPD3DTLVERTEX );
extern void __stdcall dpDrawStripIZT_K62O(RC* ,DWORD ,WORD* ,LPD3DTLVERTEX );
extern void __stdcall dpDrawStripIZ_K62O(RC* ,DWORD ,WORD* ,LPD3DTLVERTEX );

extern DPFXN dpDrawIndexedTriangleTable[];
extern DPFXN dpDrawTriangleTable[];
extern DPFXN dpDrawFanTable[];
extern DPFXN dpDrawStripTable[];

// need this
extern void __stdcall dpDrawTriangleAll(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawTriangleIZ(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawTriangleIZT(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawTriangleIZTH(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );

extern void __stdcall dpDrawIndexedTriangleAll(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawIndexedTriangleIZ(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawIndexedTriangleIZT(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawIndexedTriangleIZTH(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );

extern void __stdcall dpDrawFanAll(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawFanIZ(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawFanIZT(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawFanIT(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawFanIZTH(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);

extern void __stdcall dpDrawStripAll(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawStripIZ(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawStripIZT(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawStripIT(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawStripIZTH(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);

#endif // _3DNOWDPFUNCS

#endif // K6_2

#endif
