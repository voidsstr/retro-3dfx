/* 
// Trap.h: interface for the Trap functions
//
// Hector Yee
// yhy1@cornell.edu
// yee@3dfx.com
// 6/8/98
//
// This set of functions records glide calls for playback
// Based on glide 3.0
// 
////////////////////////////////////////////////////////////////////*/

#if !defined(AFX_TRAP_H__991E6543_FF8E_11D1_B7A3_00A0C9C84654__INCLUDED_)
#define AFX_TRAP_H__991E6543_FF8E_11D1_B7A3_00A0C9C84654__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <glide.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_PATH	255
#ifdef __cplusplus
extern "C" {
#endif

#include "glideres.h"

/* tokens file prefix*/
#define TOKENF "tokens.trp"
/* textures file prefix */
#define TEXF "t"
/* LFB file prefix */
#define LFBF "f"
/* Invalid data */
#define INVALID "invalid!"
/* texture/lfb file names */
#define TEXNAMELEN 15	
/* some arbitrary number to store temporary fog values that are discarded */
/* actual tables use grGet */
#define BIG_NUM 300
/* initial size of dynamic array */
#define DARRAYSIZE 50

	/* linked list structure */
	typedef struct _list {
		unsigned long token;
		void *data;
		struct _list *next;
	} List;
	

	/* user defined */
	/* Local functions... inlined in trap.c */
	
	void my_grVertexLayout(FxU32 param, FxI32 offset, FxU32 mode);
	void my_outputvertex(void *p);
	void * my_inputvertex(FILE *infile);
	void my_inputvnalloc(void *vtx, FILE *infile);
	int getvertexsize();

	void Init();
	FILE* PlayInit();
	FILE * BatchInit(char *indir);
	void PlayDone(FILE* infile);
	void CTrapDone();
	void SetScreenEcho(FxBool echo);
	void SetFastComp(FxBool fast);
	void SetNoDraw(FxBool nd);
	FxU32 GetMipMapSize(GrAspectRatio_t aspectRatio, GrLOD_t thisLod,GrTextureFormat_t format);
	void transformvertex(void *invert, GrVertex *outvert);


	void play_grAADrawTriangle(FILE* infile);
	void play_grAlphaBlendFunction(FILE* infile);
	void play_grAlphaCombine(FILE* infile);
	void play_grAlphaControlsITRGBLighting(FILE* infile);
	void play_grAlphaTestFunction(FILE* infile);
	void play_grAlphaTestReferenceValue(FILE* infile);
	void play_grBufferClear(FILE* infile);
	void play_grBufferSwap(FILE* infile);
	void play_grChromakeyMode(FILE* infile);
	void play_grChromakeyValue(FILE* infile);
	void play_grClipWindow(FILE* infile);
	void play_grColorCombine(FILE* infile);
	void play_grColorMask(FILE* infile);
	void play_grConstantColorValue(FILE* infile);
	void play_grCoordinateSpace(FILE* infile);
	void play_grCullMode(FILE* infile);
	void play_grDepthBiasLevel(FILE* infile);
	void play_grDepthBufferFunction(FILE* infile);
	void play_grDepthBufferMode(FILE* infile);
	void play_grDepthMask(FILE* infile);
	void play_grDepthRange(FILE* infile);
	void play_grDisable(FILE* infile);
	void play_grDisableAllEffects(FILE* infile);
	void play_grDitherMode(FILE* infile);
	void play_grDrawLine(FILE* infile);
	void play_grDrawPoint(FILE* infile);
	void play_grDrawTriangle(FILE* infile);
	void play_grDrawVertexArray(FILE* infile);
	void play_grDrawVertexArrayContiguous(FILE* infile);
	void play_grEnable(FILE* infile);
	void play_grErrorSetCallback(FILE* infile);
	void play_grFinish(FILE* infile);
	void play_grFlush(FILE* infile);
	void play_grFogColorValue(FILE* infile);
	void play_grFogMode(FILE* infile);
	void play_grFogTable(FILE* infile);
	void play_grGet(FILE* infile);
	void play_grGetProcAddress(FILE* infile);
	void play_grGetString(FILE* infile);
	void play_grGlideGetState(FILE* infile);
	void play_grGlideGetVertexLayout(FILE* infile);
	void play_grGlideInit(FILE* infile);
	void play_grGlideSetState(FILE* infile);
	void play_grGlideSetVertexLayout(FILE* infile);
	void play_grGlideShutdown(FILE* infile);
	void play_grLfbConstantAlpha(FILE* infile);
	void play_grLfbConstantDepth(FILE* infile);
	void play_grLfbLock(FILE* infile);
	void play_grLfbReadRegion(FILE* infile);
	void play_grLfbUnlock(FILE* infile);
	void play_grLfbWriteColorFormat(FILE* infile);
	void play_grLfbWriteColorSwizzle(FILE* infile);
	void play_grLfbWriteRegion(FILE* infile);
	void play_grLoadGammaTable(FILE* infile);
	void play_grQueryResolutions(FILE* infile);
	void play_grRenderBuffer(FILE* infile);
	void play_grReset(FILE* infile);
	void play_grSelectContext(FILE* infile);
	void play_grSplash(FILE* infile);
	void play_grSstOrigin(FILE* infile);
	void play_grSstSelect(FILE* infile);
	void play_grSstWinClose(FILE* infile);
	void play_grSstWinOpen(FILE* infile);
	void play_grTexCalcMemRequired(FILE* infile);
	void play_grTexClampMode(FILE* infile);
	void play_grTexCombine(FILE* infile);
	void play_grTexDetailControl(FILE* infile);
	void play_grTexDownloadMipMap(FILE* infile);
	void play_grTexDownloadMipMapLevel(FILE* infile);
	void play_grTexDownloadMipMapLevelPartial(FILE* infile);
	void play_grTexDownloadTable(FILE* infile);
	void play_grTexDownloadTablePartial(FILE* infile);
	void play_grTexFilterMode(FILE* infile);
	void play_grTexLodBiasValue(FILE* infile);
	void play_grTexMaxAddress(FILE* infile);
	void play_grTexMinAddress(FILE* infile);
	void play_grTexMipMapMode(FILE* infile);
	void play_grTexMultibase(FILE* infile);
	void play_grTexMultibaseAddress(FILE* infile);
	void play_grTexNCCTable(FILE* infile);
	void play_grTexSource(FILE* infile);
	void play_grTexTextureMemRequired(FILE* infile);
	void play_grVertexLayout(FILE* infile);
	void play_grViewport(FILE* infile);
	void play_gu3dfGetInfo(FILE* infile);
	void play_gu3dfLoad(FILE* infile);
	void play_guFogGenerateExp(FILE* infile);
	void play_guFogGenerateExp2(FILE* infile);
	void play_guFogGenerateLinear(FILE* infile);
	void play_guFogTableIndexToW(FILE* infile);
	void play_guGammaCorrectionRGB(FILE* infile);

	enum
	{
	  token_grAADrawTriangle=0,
	  token_grAlphaBlendFunction=1,
	  token_grAlphaCombine=2,
	  token_grAlphaControlsITRGBLighting=3,
	  token_grAlphaTestFunction=4,
	  token_grAlphaTestReferenceValue=5,
	  token_grBufferClear=6,
	  token_grBufferSwap=7,
	  token_grChromakeyMode=8,
	  token_grChromakeyValue=9,
	  token_grClipWindow=10,
	  token_grColorCombine=11,
	  token_grColorMask=12,
	  token_grConstantColorValue=13,
	  token_grCoordinateSpace=14,
	  token_grCullMode=15,
	  token_grDepthBiasLevel=16,
	  token_grDepthBufferFunction=17,
	  token_grDepthBufferMode=18,
	  token_grDepthMask=19,
	  token_grDepthRange=20,
	  token_grDisable=21,
	  token_grDisableAllEffects=22,
	  token_grDitherMode=23,
	  token_grDrawLine=24,
	  token_grDrawPoint=25,
	  token_grDrawTriangle=26,
	  token_grDrawVertexArray=27,
	  token_grDrawVertexArrayContiguous=28,
	  token_grEnable=29,
	  token_grErrorSetCallback=30,
	  token_grFinish=31,
	  token_grFlush=32,
	  token_grFogColorValue=33,
	  token_grFogMode=34,
	  token_grFogTable=35,
	  token_grGet=36,
	  token_grGetProcAddress=37,
	  token_grGetString=38,
	  token_grGlideGetState=39,
	  token_grGlideGetVertexLayout=40,
	  token_grGlideInit=41,
	  token_grGlideSetState=42,
	  token_grGlideSetVertexLayout=43,
	  token_grGlideShutdown=44,
	  token_grLfbConstantAlpha=45,
	  token_grLfbConstantDepth=46,
	  token_grLfbLock=47,
	  token_grLfbReadRegion=48,
	  token_grLfbUnlock=49,
	  token_grLfbWriteColorFormat=50,
	  token_grLfbWriteColorSwizzle=51,
	  token_grLfbWriteRegion=52,
	  token_grLoadGammaTable=53,
	  token_grQueryResolutions=54,
	  token_grRenderBuffer=55,
	  token_grReset=56,
	  token_grSelectContext=57,
	  token_grSplash=58,
	  token_grSstOrigin=59,
	  token_grSstSelect=60,
	  token_grSstWinClose=61,
	  token_grSstWinOpen=62,
	  token_grTexCalcMemRequired=63,
	  token_grTexClampMode=64,
	  token_grTexCombine=65,
	  token_grTexDetailControl=66,
	  token_grTexDownloadMipMap=67,
	  token_grTexDownloadMipMapLevel=68,
	  token_grTexDownloadMipMapLevelPartial=69,
	  token_grTexDownloadTable=70,
	  token_grTexDownloadTablePartial=71,
	  token_grTexFilterMode=72,
	  token_grTexLodBiasValue=73,
	  token_grTexMaxAddress=74,
	  token_grTexMinAddress=75,
	  token_grTexMipMapMode=76,
	  token_grTexMultibase=77,
	  token_grTexMultibaseAddress=78,
	  token_grTexNCCTable=79,
	  token_grTexSource=80,
	  token_grTexTextureMemRequired=81,
	  token_grVertexLayout=82,
	  token_grViewport=83,
	  token_gu3dfGetInfo=84,
	  token_gu3dfLoad=85,
	  token_guFogGenerateExp=86,
	  token_guFogGenerateExp2=87,
	  token_guFogGenerateLinear=88,
	  token_guFogTableIndexToW=89,
	  token_guGammaCorrectionRGB=90,
	  token_grChromaRangeModeExt=91,
	  token_grChromaRangeExt=92,
	  token_grTexChromaModeExt=93,
	  token_grTexChromaRangeExt=94,
	  token_grDrawTextureLineExt=95	
	};

#ifdef __cplusplus
}
#endif
#endif // !defined(AFX_TRAP_H__991E6543_FF8E_11D1_B7A3_00A0C9C84654__INCLUDED_)
