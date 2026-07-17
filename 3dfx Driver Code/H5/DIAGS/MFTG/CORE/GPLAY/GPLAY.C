/*
**
** GlidePlay 1.0
**
** Hector Yee
** yee@3dfx.com yhy1@cornell.edu
**
** 6/16/98
**
** This app plays back files captured using glidetrap.exe
**
** modified for command line use
** 8/10/98
**
*/

#include <glide.h>
#include "trap.h"
#include <stdio.h>

#include "gplay.h"

FxBool playing;
FxBool bufferswap;
FxBool frontbuffer;
FxU32  count;
FxBool echo;		/* screen echo */
FxBool spc;			/* traps space during playback */
FxBool notready;	/* makes sure can't call front buffer when not initialized */
FxU32  framestart,frame;
FxU32  frameend,framebreak;

void (*cb_fn)(int frame) = 0;

void play_step(FILE *infile);

void play_step(FILE *infile)
/* Big Switch statement to parse file */
{
	int token;
	token=fgetc(infile);
	count++;
	if (echo) printf("%8u.\t",count);	

	if (feof(infile))
	{
			playing=FXFALSE;
			return;
	};

	switch(token)
	{
	case token_grAADrawTriangle:
		play_grAADrawTriangle(infile);
	break;

	case token_grAlphaBlendFunction:
		play_grAlphaBlendFunction(infile);
	break;

	case token_grAlphaCombine:
		play_grAlphaCombine(infile);
	break;

	case token_grAlphaControlsITRGBLighting:
		play_grAlphaControlsITRGBLighting(infile);
	break;

	case token_grAlphaTestFunction:
		play_grAlphaTestFunction(infile);
	break;

	case token_grAlphaTestReferenceValue:
		play_grAlphaTestReferenceValue(infile);
	break;

	case token_grBufferClear:
		play_grBufferClear(infile);
	break;

	case token_grBufferSwap:
		bufferswap=FXTRUE;
		frame++;
		play_grBufferSwap(infile);
                if (cb_fn) {
                   cb_fn(frame-1);
                }
	break;

	case token_grChromakeyMode:
		play_grChromakeyMode(infile);
	break;

	case token_grChromakeyValue:
		play_grChromakeyValue(infile);
	break;

	case token_grClipWindow:
		play_grClipWindow(infile);
	break;

	case token_grColorCombine:
		play_grColorCombine(infile);
	break;

	case token_grColorMask:
		play_grColorMask(infile);
	break;

	case token_grConstantColorValue:
		play_grConstantColorValue(infile);
	break;

	case token_grCoordinateSpace:
		play_grCoordinateSpace(infile);
	break;

	case token_grCullMode:
		play_grCullMode(infile);
	break;

	case token_grDepthBiasLevel:
		play_grDepthBiasLevel(infile);
	break;

	case token_grDepthBufferFunction:
		play_grDepthBufferFunction(infile);
	break;

	case token_grDepthBufferMode:
		play_grDepthBufferMode(infile);
	break;

	case token_grDepthMask:
		play_grDepthMask(infile);
	break;

	case token_grDepthRange:
		play_grDepthRange(infile);
	break;

	case token_grDisable:
		play_grDisable(infile);
	break;

	case token_grDisableAllEffects:
		play_grDisableAllEffects(infile);
	break;

	case token_grDitherMode:
		play_grDitherMode(infile);
	break;

	case token_grDrawLine:
		play_grDrawLine(infile);
	break;

	case token_grDrawPoint:
		play_grDrawPoint(infile);
	break;

	case token_grDrawTriangle:
		play_grDrawTriangle(infile);
	break;

	case token_grDrawVertexArray:
		play_grDrawVertexArray(infile);
	break;

	case token_grDrawVertexArrayContiguous:
		play_grDrawVertexArrayContiguous(infile);
	break;

	case token_grEnable:
		play_grEnable(infile);
	break;

	case token_grErrorSetCallback:
		play_grErrorSetCallback(infile);
	break;

	case token_grFinish:
		play_grFinish(infile);
	break;

	case token_grFlush:
		play_grFlush(infile);
	break;

	case token_grFogColorValue:
		play_grFogColorValue(infile);
	break;

	case token_grFogMode:
		play_grFogMode(infile);
	break;

	case token_grFogTable:
		play_grFogTable(infile);
	break;

	case token_grGet:
		play_grGet(infile);
	break;

	case token_grGetProcAddress:
		play_grGetProcAddress(infile);
	break;

	case token_grGetString:
		play_grGetString(infile);
	break;

	case token_grGlideGetState:
		play_grGlideGetState(infile);
	break;

	case token_grGlideGetVertexLayout:
		play_grGlideGetVertexLayout(infile);
	break;

	case token_grGlideInit:
		// play_grGlideInit(infile);
	break;

	case token_grGlideSetState:
		play_grGlideSetState(infile);
	break;

	case token_grGlideSetVertexLayout:
		play_grGlideSetVertexLayout(infile);
	break;

	case token_grGlideShutdown:
		play_grGlideShutdown(infile);
	break;

	case token_grLfbConstantAlpha:
		play_grLfbConstantAlpha(infile);
	break;

	case token_grLfbConstantDepth:
		play_grLfbConstantDepth(infile);
	break;

	case token_grLfbLock:
		play_grLfbLock(infile);
	break;

	case token_grLfbReadRegion:
		play_grLfbReadRegion(infile);
	break;

	case token_grLfbUnlock:
		play_grLfbUnlock(infile);
	break;

	case token_grLfbWriteColorFormat:
		play_grLfbWriteColorFormat(infile);
	break;

	case token_grLfbWriteColorSwizzle:
		play_grLfbWriteColorSwizzle(infile);
	break;

	case token_grLfbWriteRegion:
		play_grLfbWriteRegion(infile);
	break;

	case token_grLoadGammaTable:
		play_grLoadGammaTable(infile);
	break;

	case token_grQueryResolutions:
		play_grQueryResolutions(infile);
	break;

	case token_grRenderBuffer:
		play_grRenderBuffer(infile);
	break;

	case token_grReset:
		play_grReset(infile);
	break;

	case token_grSelectContext:
		play_grSelectContext(infile);
	break;

	case token_grSplash:
		// play_grSplash(infile);
	break;

	case token_grSstOrigin:
		play_grSstOrigin(infile);
	break;

	case token_grSstSelect:
		play_grSstSelect(infile);
	break;

	case token_grSstWinClose:
		play_grSstWinClose(infile);
	break;

	case token_grSstWinOpen:
		notready=FXFALSE;
		play_grSstWinOpen(infile);
	break;

	case token_grTexCalcMemRequired:
		play_grTexCalcMemRequired(infile);
	break;

	case token_grTexClampMode:
		play_grTexClampMode(infile);
	break;

	case token_grTexCombine:
		play_grTexCombine(infile);
	break;

	case token_grTexDetailControl:
		play_grTexDetailControl(infile);
	break;

	case token_grTexDownloadMipMap:
		play_grTexDownloadMipMap(infile);
	break;

	case token_grTexDownloadMipMapLevel:
		play_grTexDownloadMipMapLevel(infile);
	break;

	case token_grTexDownloadMipMapLevelPartial:
		play_grTexDownloadMipMapLevelPartial(infile);
	break;

	case token_grTexDownloadTable:
		play_grTexDownloadTable(infile);
	break;

	case token_grTexDownloadTablePartial:
		play_grTexDownloadTablePartial(infile);
	break;

	case token_grTexFilterMode:
		play_grTexFilterMode(infile);
	break;

	case token_grTexLodBiasValue:
		play_grTexLodBiasValue(infile);
	break;

	case token_grTexMaxAddress:
		play_grTexMaxAddress(infile);
	break;

	case token_grTexMinAddress:
		play_grTexMinAddress(infile);
	break;

	case token_grTexMipMapMode:
		play_grTexMipMapMode(infile);
	break;

	case token_grTexMultibase:
		play_grTexMultibase(infile);
	break;

	case token_grTexMultibaseAddress:
		play_grTexMultibaseAddress(infile);
	break;

	case token_grTexNCCTable:
		play_grTexNCCTable(infile);
	break;

	case token_grTexSource:
		play_grTexSource(infile);
	break;

	case token_grTexTextureMemRequired:
		play_grTexTextureMemRequired(infile);
	break;

	case token_grVertexLayout:
		play_grVertexLayout(infile);
	break;

	case token_grViewport:
		play_grViewport(infile);
	break;

	case token_gu3dfGetInfo:
		play_gu3dfGetInfo(infile);
	break;

	case token_gu3dfLoad:
		play_gu3dfLoad(infile);
	break;

	case token_guFogGenerateExp:
		play_guFogGenerateExp(infile);
	break;

	case token_guFogGenerateExp2:
		play_guFogGenerateExp2(infile);
	break;

	case token_guFogGenerateLinear:
		play_guFogGenerateLinear(infile);
	break;

	case token_guFogTableIndexToW:
		play_guFogTableIndexToW(infile);
	break;

	case token_guGammaCorrectionRGB:
		play_guGammaCorrectionRGB(infile);
	break;

	  default:
		  printf("Undefined Token!\n");
		  break;
	}
}

int gplayTrace(char *n_indir, FxU32 n_framestart, FxU32 n_frameend, void (*cbfn)(int frame))
{
	char def_indir[100] = "unreal";
        char *indir;
	FILE *infile;
	FxU32 tmp;	

	/* init stuff */
	notready=FXTRUE;
	frontbuffer=FXFALSE;
	echo=FXFALSE;
	SetScreenEcho(FXFALSE);
	playing=FXTRUE;
	infile=NULL;
	count=0;
	frame=0;

        cb_fn = cbfn;

        if (!n_indir || (*n_indir == 0)) {
           indir = def_indir;
           framestart = 1;
           frameend = 2;
        } else {
           indir = n_indir;
           framestart = n_framestart;
           frameend = n_frameend;
        }

	infile=BatchInit(indir);

        if (infile == NULL) {
           return 1;
        }

	printf("Playing from frame %u to %u.\n",framestart,frameend);	
	
	while(playing)
	{
		SetNoDraw(FXTRUE);
		printf("Skipping Frames: ");
		while (playing && frame<framestart-1)
		{
			tmp=frame;
			play_step(infile);
			if (frame!=tmp) printf("%u ",frame);
		}
		SetNoDraw(FXFALSE);
		
		printf("\nPlaying Frames: ");
		while (playing && frame<frameend)
		{
			
			tmp=frame;
			play_step(infile);
			if (frame!=tmp) printf("%u ",frame);
		}
		playing=FXFALSE;
	} /* while */	
	PlayDone(infile);
	printf("\nDone.\n");	
	return 0;
}
