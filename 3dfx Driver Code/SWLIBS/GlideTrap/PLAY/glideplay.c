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
*/

#include <glide.h>
#include "trap.h"
#include "texmem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef __WIN32__
#include <conio.h>
#endif

FxBool playing;
FxBool bufferswap;
FxBool frontbuffer;
FxI32  count;
FxBool echo;        /* screen echo */
FxBool spc;            /* traps space during playback */
FxBool notready;    /* makes sure can't call front buffer when not initialized */
FxI32  frame;
FxI32  framebreak;
FxBool filter=FXFALSE;
FxBool glideInit=FXFALSE;

extern char dir_base[MAX_PATH_LEN];
extern GrScreenResolution_t g_screen_resolution;

int xyResolution[][2] = {
  {  320,  200 },
  {  320,  240 },
  {  400,  256 },
  {  512,  384 },
  {  640,  200 },
  {  640,  350 },
  {  640,  400 },
  {  640,  480 },
  {  800,  600 },
  {  960,  720 },
  {  856,  480 },
  {  512,  256 },
  { 1024,  768 },
  { 1280, 1024 },
  { 1600, 1200 },
  {  400,  300 },
  { 1152,  864 },
  { 1280,  960 },
  { 1600, 1024 },
  { 1792, 1344 },
  { 1856, 1392 },
  { 1920, 1440 },
  { 2048, 1536 },
  { 2048, 2048 },
  {    0,    0 },  // safety margin
  {    0,    0 },
  {    0,    0 },
  {    0,    0 },
  {    0,    0 },
  {    0,    0 },
  {    0,    0 },
  {    0,    0 }
};

void print_help()
{
    printf("\nGlidePlay 1.0\n");
    printf("Hector Yee (yee@3dfx.com; yhy1@cornell.edu)\n");
    printf("---------------------------------------\n");
    printf(" B      - Bufferswap\n");
    printf(" G      - Skip to command\n");
    printf(" K      - Skip to frame\n");
    printf(" D      - Dump frame buffer to PPM file\n");
    printf(" E      - Toggle Echo\n");    
    printf(" F      - Toggle Render to Front/Back Buffer\n");    
    printf(" H      - Help\n");
    printf(" P      - Continuous Play (SPC to stop)\n");
    printf(" Q      - Quit\n");
    printf(" S      - Step\n");    
    printf(" RETURN - Play till grBufferSwap()\n");
    printf("---------------------------------------\n");
    printf("Frame %d\n",(int)frame);
    if (echo) printf("Echo is ON\n"); else printf("Echo is OFF\n");
    if (frontbuffer) printf("Rendering to FRONT\n"); else printf("Rendering to BACK\n"); 
    printf("---------------------------------------\n");
}

void print_filter_help()
{
printf(
"\nGlideFilter\n"
"------------------------------------------------------------------------\n"
"This option is used to play back a range of frames from a previously\n"
"recorded GlideTrap session.  In addition to the playback directory,\n"
"You will be asked for a starting and ending frame that you wish to see.\n"
"All Glide state (including textures) up to the starting frame of\n"
"interest will be sent down through Glide API calls as efficiently as\n"
"possible.  In other words, N-1 frames of state won't be sent down just\n"
"because you want to see frame N.  Only the current (or \"visible\")\n"
"state will be sent down.\n"
"\n"
"All \"output\" will be sent to Glide3.  If you would like to capture\n"
"the output of the filter, you need only use the Glide3 trapping DLL,\n"
"just as you would for recording from any other Glide3 application.\n"
"------------------------------------------------------------------------\n"
);
}

void play_step(FILEBUFF *infile)
/* Big Switch statement to parse file */
{
    int token;
    token=fileBuffReadFxU8(infile);
    count++;
    if (echo) printf("%8d.\t",(int)count);

    if (fileBuffEOF(infile)) {
            playing=FXFALSE;
            if (echo)
              printf("EOF\n");
            return;
    }

    switch(token) {
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
        glideInit = FXTRUE;
        play_grGlideInit(infile);
    break;

    case token_grGlideSetState:
        play_grGlideSetState(infile);
    break;

    case token_grGlideSetVertexLayout:
        play_grGlideSetVertexLayout(infile);
    break;

    case token_grGlideShutdown:
        glideInit = FXFALSE;
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
        play_grSplash(infile);
    break;

    case token_grSstOrigin:
        play_grSstOrigin(infile);
    break;

    case token_grSstSelect:
        play_grSstSelect(infile);
    break;

    case token_grSstWinClose:
        if (!getenv("FX_GLIDE_TEST"))
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

    case token_grChromaRangeModeExt:
        play_grChromaRangeModeExt(infile);
        break;

    case token_grChromaRangeExt:
        play_grChromaRangeExt(infile);
        break;

    case token_grTexChromaModeExt:
        play_grTexChromaModeExt(infile);
        break;

    case token_grTexChromaRangeExt:
        play_grTexChromaRangeExt(infile);
        break;

    case token_grDrawTextureLineExt:
        play_grDrawTextureLineExt(infile);
        break;

      default:
          printf("Undefined Token!\n");
          break;
    }
    fflush(stdout);
}

void DumpFilteredState(void)
{
  int i;

  if (gstate.grABF.set)
    grAlphaBlendFunction(gstate.grABF.rgb_sf,
                         gstate.grABF.rgb_df,
                         gstate.grABF.alpha_sf,
                         gstate.grABF.alpha_df);

  if (gstate.grAC.set)
    grAlphaCombine(gstate.grAC.function,
                   gstate.grAC.factor,
                   gstate.grAC.local,
                   gstate.grAC.other,
                   gstate.grAC.invert);

  if (gstate.grACIL.set)
    grAlphaControlsITRGBLighting(gstate.grACIL.enable);

  if (gstate.grATF.set)
    grAlphaTestFunction(gstate.grATF.function);

  if (gstate.grATRV.set)
    grAlphaTestReferenceValue(gstate.grATRV.value);

  /* TODO: leave this guy out?
  if (gstate.grBC.set)
    grBufferClear(gstate.grBC.color,
                  gstate.grBC.alpha,
                  gstate.grBC.depth);
  */

  // TODO: move this to end?
  if (gstate.grBS.set)
    grBufferSwap(gstate.grBS.swap_interval);

  if (gstate.grCKM.set)
    grChromakeyMode(gstate.grCKM.mode);

  if (gstate.grCKV.set)
    grChromakeyValue(gstate.grCKV.value);

  if (gstate.grCW.set)
    grClipWindow(gstate.grCW.minx,
                 gstate.grCW.miny,
                 gstate.grCW.maxx,
                 gstate.grCW.maxy);

  if (gstate.grCC.set)
    grColorCombine(gstate.grCC.function,
                   gstate.grCC.factor,
                   gstate.grCC.local,
                   gstate.grCC.other,
                   gstate.grCC.invert);

  if (gstate.grColorMask.set)
    grColorMask(gstate.grColorMask.rgb,
                gstate.grColorMask.a);

  if (gstate.grCCV.set)
    grConstantColorValue(gstate.grCCV.value);

  if (gstate.grCS.set)
    grCoordinateSpace(gstate.grCS.mode);

  if (gstate.grCullMode.set)
    grCullMode(gstate.grCullMode.mode);

  if (gstate.grDBL.set)
    grDepthBiasLevel(gstate.grDBL.level);

  if (gstate.grDBF.set)
    grDepthBufferFunction(gstate.grDBF.function);

  if (gstate.grDBM.set)
    grDepthBufferMode(gstate.grDBM.mode);

  if (gstate.grDepthMask.set)
    grDepthMask(gstate.grDepthMask.mask);

  if (gstate.grDR.set)
    grDepthRange(gstate.grDR.n,
                 gstate.grDR.f);

  if (gstate.grDAE.set)
    grDisableAllEffects();

  if (gstate.grDitherMode.set)
    grDitherMode(gstate.grDitherMode.mode);

  for ( i=0 ; i<MAX_ENABLE_DISABLE ; i++ ) {
    if (gstate.grED.set[i]) {
      if (gstate.grED.enable[i]) grEnable(i);
      else                       grDisable(i);
    }
  }

  if (gstate.grFCV.set)
    grFogColorValue(gstate.grFCV.fogcolor);

  if (gstate.grFM.set)
    grFogMode(gstate.grFM.mode);

  if (gstate.grFT.set)
    grFogTable(gstate.grFT.data);

  if (gstate.grLCA.set)
    grLfbConstantAlpha(gstate.grLCA.alpha);

  if (gstate.grLCD.set)
    grLfbConstantDepth(gstate.grLCD.depth);

  if (gstate.grLWCF.set)
    grLfbWriteColorFormat(gstate.grLWCF.colorFormat);

  if (gstate.grLWCS.set)
    grLfbWriteColorSwizzle(gstate.grLWCS.swizzleBytes,
                           gstate.grLWCS.swapWords);

  if (gstate.grLGT.set)
    grLoadGammaTable(gstate.grLGT.nentries,
                     gstate.grLGT.data_r,
                     gstate.grLGT.data_g,
                     gstate.grLGT.data_b);

  if (gstate.grRB.set)
    grRenderBuffer(gstate.grRB.buffer);

  for ( i=0 ; i<MAX_RESET ; i++ ) {
    if (gstate.grR.set[i])
      grReset(i);
  }

  if (gstate.grSO.set)
    grSstOrigin(gstate.grSO.origin);

  for ( i=0 ; i<MAX_TMU ; i++ ) {

    if (gstate.grTCM[i].set)
      grTexClampMode(i, 
                     gstate.grTCM[i].s_clampmode,
                     gstate.grTCM[i].t_clampmode);

    if (gstate.grTC[i].set)
      grTexCombine(i, 
                   gstate.grTC[i].rgb_function,
                   gstate.grTC[i].rgb_factor,
                   gstate.grTC[i].alpha_function,
                   gstate.grTC[i].alpha_factor,
                   gstate.grTC[i].rgb_invert,
                   gstate.grTC[i].alpha_invert);

    if (gstate.grTDC[i].set)
      grTexDetailControl(i, 
                         gstate.grTDC[i].lod_bias,
                         gstate.grTDC[i].detail_scale,
                         gstate.grTDC[i].detail_max);

    if (gstate.grTFM[i].set)
      grTexFilterMode(i, 
                      gstate.grTFM[i].minfilter_mode,
                      gstate.grTFM[i].magfilter_mode);

    if (gstate.grTLBV[i].set)
      grTexLodBiasValue(i, 
                        gstate.grTLBV[i].bias);

    if (gstate.grTMMM[i].set)
      grTexMipMapMode(i, 
                      gstate.grTMMM[i].mode,
                      gstate.grTMMM[i].lodBlend);

    if (gstate.grTMB[i].set)
      grTexMultibase(i, 
                     gstate.grTMB[i].enable);

    if (gstate.grTMBA[i].set)
      grTexMultibaseAddress(i, 
                            gstate.grTMBA[i].range,
                            gstate.grTMBA[i].startAddress,
                            gstate.grTMBA[i].evenOdd,
                            &gstate.grTMBA[i].info);

    if (gstate.grTS[i].set)
      grTexSource(i, 
                  gstate.grTS[i].startAddress,
                  gstate.grTS[i].evenOdd,
                  &gstate.grTS[i].info);
  }

  if (gstate.grTDT.set) {
    if ((gstate.grTDT.type == GR_TEXTABLE_PALETTE) ||
        (gstate.grTDT.type == GR_TEXTABLE_PALETTE_6666_EXT)) {
      grTexDownloadTable(gstate.grTDT.type,
                         &gstate.grTDT.texpalette);
    } else {
      grTexDownloadTable(gstate.grTDT.type,
                         &gstate.grTDT.ncctable);
    }
  }

  if (gstate.grTNT.set)
    grTexNCCTable(gstate.grTNT.table);

  for ( i=0 ; i<MAX_VERTEX_LAYOUT ; i++ ) {
    if (gstate.grVL[i].set)
      grVertexLayout(i,
                     gstate.grVL[i].offset,
                     gstate.grVL[i].mode);
  }

  if (gstate.grVP.set)
    grViewport(gstate.grVP.x,
               gstate.grVP.y,
               gstate.grVP.width,
               gstate.grVP.height);

  // -------------------------------------------------------------
  // gu*

  if (gstate.guGCRGB.set)
    guGammaCorrectionRGB(gstate.guGCRGB.red,
                         gstate.guGCRGB.green,
                         gstate.guGCRGB.blue);

  // -------------------------------------------------------------
  // extensions

  if (gstate.grCRME.set && MYEXT_grChromaRangeModeExt)
    MYEXT_grChromaRangeModeExt(gstate.grCRME.mode);

  if (gstate.grCRE.set && MYEXT_grChromaRangeExt)
    MYEXT_grChromaRangeExt(gstate.grCRE.color,
                           gstate.grCRE.range,
                           gstate.grCRE.match_mode);

  for ( i=0 ; i<MAX_TMU ; i++ ) {

    if (gstate.grTCME[i].set && MYEXT_grTexChromaModeExt)
      MYEXT_grTexChromaModeExt(i,
                               gstate.grTCME[i].mode);

    if (gstate.grTCRE[i].set && MYEXT_grTexChromaRangeExt)
      MYEXT_grTexChromaRangeExt(i,
                                gstate.grTCRE[i].min,
                                gstate.grTCRE[i].max,
                                gstate.grTCRE[i].mode);
  }
}

void DumpTextures(void)
{
  int         i, count;
  TexMemList *cur;
  void       *data;
  grTexDownloadMipMapState             *mm_state;
  grTexDownloadMipMapLevelState        *mml_state;
  grTexDownloadMipMapLevelPartialState *mmlp_state;


  // -------------------------------------------------------------
  // download all the textures

  printf("\n");
  for ( count=0, i=0 ; i<MAX_TMU ; i++ ) {
    for ( cur=tmu_map[i] ; cur ; cur=cur->next ) {

      count++;
      printf("\rDownloading textures %d", count);
      fflush(stdout);

      mm_state = cur->data;
      switch (mm_state->token) {

        case token_grTexDownloadMipMap:
          load_texture(mm_state->filename,
                      &mm_state->info.data,
                       mm_state->info.format);
          //printf("MM tmu %d, size %d\n", mm_state->tmu, mm_state->size);
          grTexDownloadMipMap(mm_state->tmu,
                              mm_state->startAddress,
                              mm_state->evenOdd,
                              &mm_state->info);
          free(mm_state->info.data);
          mm_state->info.data = NULL;
          break;

        case token_grTexDownloadMipMapLevel:
          mml_state = cur->data;
          data=NULL;
          load_texture(mml_state->filename, &data, mml_state->format);
          //printf("MML tmu %d, size %d\n", mml_state->tmu, mml_state->size);
          grTexDownloadMipMapLevel(mml_state->tmu,
                                   mml_state->startAddress,
                                   mml_state->thisLod,
                                   mml_state->largeLod,
                                   mml_state->aspectRatio,
                                   mml_state->format,
                                   mml_state->evenOdd,
                                   data);
          free(data);
          break;

        case token_grTexDownloadMipMapLevelPartial:
          mmlp_state = cur->data;
          data=NULL;
          load_texture(mmlp_state->filename, &data, mmlp_state->format);
          //printf("MMLP tmu %d, size %d\n", mmlp_state->tmu, mmlp_state->size);
          grTexDownloadMipMapLevelPartial(mmlp_state->tmu,
                                          mmlp_state->startAddress,
                                          mmlp_state->thisLod,
                                          mmlp_state->largeLod,
                                          mmlp_state->aspectRatio,
                                          mmlp_state->format,
                                          mmlp_state->evenOdd,
                                          data,
                                          mmlp_state->start,
                                          mmlp_state->end);
          free(data);
          break;

        default:
          printf("Eek!  Unknown texture download token.  Fix me!\n");
          exit(0);
      }
    }
  }
  printf("\rDownloading textures %d ... done!\n", count);

  // -------------------------------------------------------------
  // optionally free of all the texture data structure junk

  FreeAllTextureData();
}

void filter_step(FILEBUFF *infile)
/* Big Switch statement to parse file */
{
    int token;
    token=fileBuffReadFxU8(infile);
    count++;

    if (fileBuffEOF(infile)) {
            playing=FXFALSE;
            return;
    }

    //printf("token=%d\n", token);
    switch(token) {

      case token_grAADrawTriangle:
        filter_grAADrawTriangle(token, infile);
        break;

      case token_grAlphaBlendFunction:
        filter_grAlphaBlendFunction(token, infile);
        break;

      case token_grAlphaCombine:
        filter_grAlphaCombine(token, infile);
        break;

      case token_grAlphaControlsITRGBLighting:
        filter_grAlphaControlsITRGBLighting(token, infile);
        break;

      case token_grAlphaTestFunction:
        filter_grAlphaTestFunction(token, infile);
        break;

      case token_grAlphaTestReferenceValue:
        filter_grAlphaTestReferenceValue(token, infile);
        break;

      case token_grBufferClear:
        filter_grBufferClear(token, infile);
        break;

      case token_grBufferSwap:
        frame++;
        filter_grBufferSwap(token, infile);
        break;

      case token_grChromakeyMode:
        filter_grChromakeyMode(token, infile);
        break;

      case token_grChromakeyValue:
        filter_grChromakeyValue(token, infile);
        break;

      case token_grClipWindow:
        filter_grClipWindow(token, infile);
        break;

      case token_grColorCombine:
        filter_grColorCombine(token, infile);
        break;

      case token_grColorMask:
        filter_grColorMask(token, infile);
        break;

      case token_grConstantColorValue:
        filter_grConstantColorValue(token, infile);
        break;

      case token_grCoordinateSpace:
        filter_grCoordinateSpace(token, infile);
        break;

      case token_grCullMode:
        filter_grCullMode(token, infile);
        break;

      case token_grDepthBiasLevel:
        filter_grDepthBiasLevel(token, infile);
        break;

      case token_grDepthBufferFunction:
        filter_grDepthBufferFunction(token, infile);
        break;

      case token_grDepthBufferMode:
        filter_grDepthBufferMode(token, infile);
        break;

      case token_grDepthMask:
        filter_grDepthMask(token, infile);
        break;

      case token_grDepthRange:
        filter_grDepthRange(token, infile);
        break;

      case token_grDisable:
        filter_grDisable(token, infile);
        break;

      case token_grDisableAllEffects:
        filter_grDisableAllEffects(token, infile);
        break;

      case token_grDitherMode:
        filter_grDitherMode(token, infile);
        break;

      case token_grDrawLine:
        filter_grDrawLine(token, infile);
        break;

      case token_grDrawPoint:
        filter_grDrawPoint(token, infile);
        break;

      case token_grDrawTriangle:
        filter_grDrawTriangle(token, infile);
        break;

      case token_grDrawVertexArray:
        filter_grDrawVertexArray(token, infile);
        break;

      case token_grDrawVertexArrayContiguous:
        filter_grDrawVertexArrayContiguous(token, infile);
        break;

      case token_grEnable:
        filter_grEnable(token, infile);
        break;

      case token_grErrorSetCallback:
        filter_grErrorSetCallback(token, infile);
        break;

      case token_grFinish:
        filter_grFinish(token, infile);
        break;

      case token_grFlush:
        filter_grFlush(token, infile);
        break;

      case token_grFogColorValue:
        filter_grFogColorValue(token, infile);
        break;

      case token_grFogMode:
        filter_grFogMode(token, infile);
        break;

      case token_grFogTable:
        filter_grFogTable(token, infile);
        break;

      case token_grGet:
        filter_grGet(token, infile);
        break;

      case token_grGetProcAddress:
        filter_grGetProcAddress(token, infile);
        break;

      case token_grGetString:
        filter_grGetString(token, infile);
        break;

      case token_grGlideGetState:

        // Download all the current GlideState image, and then
        // re-initialized our GlideState to empty
        DumpFilteredState();
        memset(&gstate, 0, sizeof(GlideState));

        play_grGlideGetState(infile);
        break;

      case token_grGlideGetVertexLayout:

        // Download all the current GlideState image, and then
        // re-initialized our GlideState to empty
        DumpFilteredState();
        memset(&gstate, 0, sizeof(GlideState));

        play_grGlideGetVertexLayout(infile);
        break;

      case token_grGlideInit:
        // don't filter this guy
        glideInit = FXTRUE;
        play_grGlideInit(infile);
        break;

      case token_grGlideSetState:
        // Mark our GlideState as being totally clean
        memset(&gstate, 0, sizeof(GlideState));

        play_grGlideSetState(infile);
        break;

      case token_grGlideSetVertexLayout:
        // Mark our GlideState.grVertextLayoutState as being totally clean
        memset(gstate.grVL,0, sizeof(grVertexLayoutState)*MAX_VERTEX_LAYOUT);

        play_grGlideSetVertexLayout(infile);
        break;

      case token_grGlideShutdown:
        // don't filter this guy
        glideInit = FXFALSE;
        play_grGlideShutdown(infile);
        break;

      case token_grLfbConstantAlpha:
        filter_grLfbConstantAlpha(token, infile);
        break;

      case token_grLfbConstantDepth:
        filter_grLfbConstantDepth(token, infile);
        break;

      case token_grLfbLock:
        filter_grLfbLock(token, infile);
        break;

      case token_grLfbReadRegion:
        filter_grLfbReadRegion(token, infile);
        break;

      case token_grLfbUnlock:
        filter_grLfbUnlock(token, infile);
        break;

      case token_grLfbWriteColorFormat:
        filter_grLfbWriteColorFormat(token, infile);
        break;

      case token_grLfbWriteColorSwizzle:
        filter_grLfbWriteColorSwizzle(token, infile);
        break;

      case token_grLfbWriteRegion:
        filter_grLfbWriteRegion(token, infile);
        break;

      case token_grLoadGammaTable:
        filter_grLoadGammaTable(token, infile);
        break;

      case token_grQueryResolutions:
        filter_grQueryResolutions(token, infile);
        break;

      case token_grRenderBuffer:
        filter_grRenderBuffer(token, infile);
        break;

      case token_grReset:
        filter_grReset(token, infile);
        break;

      case token_grSelectContext:
        filter_grSelectContext(token, infile);
        break;

      case token_grSplash:
        filter_grSplash(token, infile);
        break;

      case token_grSstOrigin:
        filter_grSstOrigin(token, infile);
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
        filter_grTexCalcMemRequired(token, infile);
        break;

      case token_grTexClampMode:
        filter_grTexClampMode(token, infile);
        break;

      case token_grTexCombine:
        filter_grTexCombine(token, infile);
        break;

      case token_grTexDetailControl:
        filter_grTexDetailControl(token, infile);
        break;

      case token_grTexDownloadMipMap:
        filter_grTexDownloadMipMap(token, infile);
        break;

      case token_grTexDownloadMipMapLevel:
        filter_grTexDownloadMipMapLevel(token, infile);
        break;

      case token_grTexDownloadMipMapLevelPartial:
        filter_grTexDownloadMipMapLevelPartial(token, infile);
        break;

      case token_grTexDownloadTable:
        filter_grTexDownloadTable(token, infile);
        break;

      case token_grTexDownloadTablePartial:
        filter_grTexDownloadTablePartial(token, infile);
        break;

      case token_grTexFilterMode:
        filter_grTexFilterMode(token, infile);
        break;

      case token_grTexLodBiasValue:
        filter_grTexLodBiasValue(token, infile);
        break;

      case token_grTexMaxAddress:
        filter_grTexMaxAddress(token, infile);
        break;

      case token_grTexMinAddress:
        filter_grTexMinAddress(token, infile);
        break;

      case token_grTexMipMapMode:
        filter_grTexMipMapMode(token, infile);
        break;

      case token_grTexMultibase:
        filter_grTexMultibase(token, infile);
        break;

      case token_grTexMultibaseAddress:
        filter_grTexMultibaseAddress(token, infile);
        break;

      case token_grTexNCCTable:
        filter_grTexNCCTable(token, infile);
        break;

      case token_grTexSource:
        filter_grTexSource(token, infile);
        break;

      case token_grTexTextureMemRequired:
        filter_grTexTextureMemRequired(token, infile);
        break;

      case token_grVertexLayout:
        filter_grVertexLayout(token, infile);
        break;

      case token_grViewport:
        filter_grViewport(token, infile);
        break;

      case token_gu3dfGetInfo:
        filter_gu3dfGetInfo(token, infile);
        break;

      case token_gu3dfLoad:
        filter_gu3dfLoad(token, infile);
        break;

      case token_guFogGenerateExp:
        filter_guFogGenerateExp(token, infile);
        break;

      case token_guFogGenerateExp2:
        filter_guFogGenerateExp2(token, infile);
        break;

      case token_guFogGenerateLinear:
        filter_guFogGenerateLinear(token, infile);
        break;

      case token_guFogTableIndexToW:
        filter_guFogTableIndexToW(token, infile);
        break;

      case token_guGammaCorrectionRGB:
        filter_guGammaCorrectionRGB(token, infile);
        break;

      case token_grChromaRangeModeExt:
        filter_grChromaRangeModeExt(token, infile);
        break;

      case token_grChromaRangeExt:
        filter_grChromaRangeExt(token, infile);
        break;

      case token_grTexChromaModeExt:
        filter_grTexChromaModeExt(token, infile);
        break;

      case token_grTexChromaRangeExt:
        filter_grTexChromaRangeExt(token, infile);
        break;

      case token_grDrawTextureLineExt:
        filter_grDrawTextureLineExt(token, infile);
        break;

      default:
        printf("Undefined Token!\n");
        break;
    }
}

// sort of based on tlib.c:tlScreenDump()
static void
screenDump(char *filename, int width, int height,
           int frame_id, char *recording)
{
  FILE *fp;
  FxU16 *region, pixel;
  FxU8  *ppmData;
  int i, count;
  char realname[64];

  // convert to lower case and append ".ppm" if necessary
  strcpy(realname, filename);
  for ( count=strlen(realname), i=0 ; i<count ; i++ ) {
    if ((realname[i] >= 'A') && (realname[i] <= 'Z'))
      realname[i] += 'a' - 'A';
  }
  if (!strstr(realname, ".ppm"))
    strcat(realname, ".ppm");

  // open file
  if (!(fp=fopen(realname, "wb"))) {
    printf("Unable to fopen(%s).  Screen Dump failed.\n", realname);
    return;
  }

  region = malloc(width * height * sizeof(FxU16));
  ppmData = malloc(width * height * 3);
  grLfbReadRegion( GR_BUFFER_FRONTBUFFER,
                   0, 0, width, height,
                   width*2, region );

  for ( i=0 ; i<width*height ; i++ ) {
#ifdef __WIN32__
    pixel = region[i];
#else
    pixel = (i & 1) ? region[i-1] : region[i+1];  // endian swap
#endif
    ppmData[3*i+0] = (pixel&0xF800)>>8;   // red
    ppmData[3*i+1] = (pixel&0x7E0) >>3;   // green
    ppmData[3*i+2] = (pixel&0x1F)  <<3;   // blue
  }

  // spit out PPM header
  fprintf(fp, "P6\n");
  fprintf(fp, "# frame %d from GlidePlay of directory %s\n",
              frame_id, recording);
  fprintf(fp, "%d %d\n", width, height);
  fprintf(fp, "255\n");

  // write the data
  fwrite(ppmData, width*height*3, 1, fp);

  // all done
  free(ppmData);
  free(region);
  fclose(fp);

  printf("Successfully wrote %s\n", realname);

  return;
}

#ifndef __WIN32__
FxBool kbhit(void)
{
  return FXFALSE;
}
#endif

int main(int argc, char *argv[])
{
    char key;
    char input[50];
    FILEBUFF *infile;
    FxI32 tmp;
    int framestart=0, frameend=0;

    /* init stuff */
    notready=FXTRUE;
    frontbuffer=FXFALSE;
    echo=FXTRUE;
    SetScreenEcho(FXTRUE);
    playing=FXTRUE;
    infile=NULL;
    count=0;
    frame=0;

    // -----------------------------------------------------------------
    // filter mode

    if ((argc == 2) &&
        ((argv[1][0]=='-') || (argv[1][0]=='/') || (argv[1][0]=='\\')) &&
        ((argv[1][1]=='f') || (argv[1][0]=='F'))) {
int count=0;

      print_filter_help();

      echo = FXFALSE;
      SetScreenEcho(echo);

      infile = FilterInit(&framestart, &frameend);
      printf("Filtering frames: ");
      while (playing && frame<framestart-1) {
          tmp=frame;
          filter_step(infile);
          if (frame!=tmp) printf("%d ",(int)frame);
          count++;
      }

      DumpFilteredState();
      DumpTextures();

      printf("\nPlaying Frames: ");
      while (playing && frame<frameend) {
          tmp=frame;
          play_step(infile);
          if (frame!=tmp) {
            printf("%d ",(int)frame);
          }
      }

    } else if (argc == 4) {

        // -------------------------------------------------------------
        // command line playback mode

        infile = CommandLinePlayInit(argc, argv, &framestart, &frameend);
        echo = FXFALSE;
        SetScreenEcho(echo);

        SetNoDraw(FXTRUE);
        printf("Skipping Frames: ");
        while (playing && frame<framestart-1) {
            tmp=frame;
            play_step(infile);
            if (frame!=tmp) printf("%d ",(int)frame);
        }

        SetNoDraw(FXFALSE);
        printf("\nPlaying Frames: ");
        while (playing && frame<frameend) {
            tmp=frame;
            play_step(infile);
            if (frame!=tmp) printf("%d ",(int)frame);
        }

    } else {

      // -------------------------------------------------------------
      // interactive playback mode

      print_help();
      infile = PlayInit();

      while(playing) {

        /* keyboard handler */
        printf("eh? ");
        fflush(stdout);
        gets(input);
        key=toupper(input[0]);
        
        switch(key) {

          case 0: /* play till bufferswap or kbhit()*/
            bufferswap=FXFALSE;
            spc=FXFALSE;
            while (playing && !bufferswap && !spc) {
                play_step(infile);
                if (kbhit())
                    spc=FXTRUE;
            }
            if (playing)
              printf("Paused at frame %d\n", (int)frame);
            break;

          case 'Q': /* quit */
            playing=FXFALSE;            
            glideInit = FXFALSE;
            grGlideShutdown();
            break;

          case 'D': // dump frame buffer contents to PPM file
            printf("Filename to dump frame buffer to: ");
            gets(input);
            if (strlen(input)>0)
              screenDump(input,
                         xyResolution[g_screen_resolution][0],
                         xyResolution[g_screen_resolution][1],
                         frame, dir_base);
            break;

          case 'B': /* bufferswap */
            if (notready) {
                printf("Can't -- Glide or HW not initialized\n");
            } else {
                grBufferSwap(1);
                printf("Buffer Swapped\n");
            }
            break;

          case 'F': /* toggle rendering to front buffer */
            if (notready) {
                printf("Can't -- Glide or HW not initialized\n");
            } else {
                if (frontbuffer) {
                    printf("<< Rendering to BACK BUFFER! >>\n");
                    frontbuffer=FXFALSE;
                    grRenderBuffer(GR_BUFFER_BACKBUFFER);
                } else {
                    printf("<< Rendering to FRONT BUFFER! >>\n");
                    frontbuffer=FXTRUE;
                    grRenderBuffer(GR_BUFFER_FRONTBUFFER);
                }
            }
            break;

          case 'E': /* echo on/off */
            echo = !echo;
            SetScreenEcho(echo);
            printf("Screen Echo %s\n", echo ? "ON" : "OFF");
            break;

          case 'S': /* step mode */
            play_step(infile);
            //if (!notready) grFinish();
            break;

          case 'P': /* play mode */
            spc=FXFALSE;
            while (playing && !spc) {
                play_step(infile);
                if (kbhit())
                    spc=FXTRUE;
            }
            if (playing)
              printf("Paused at frame %d\n", (int)frame);
            break;

          case 'G': /* play to command # */
            spc=FXFALSE;
            printf("Skip to command: ");
            gets(input);
            framebreak = atoi(input);
            while (playing && (framebreak>count))
                play_step(infile);
            printf("Command %d reached\n",(int)count);
            break;

          case 'K':            
            printf("Skip to frame: ");
            gets(input);
            framebreak = atoi(input);
            if (framebreak<frame) {
                printf("Already at frame %d\n",(int)frame);
            } else {
                SetNoDraw(FXTRUE);
                while (playing && (frame<framebreak-1)) {
                    tmp=frame;
                    play_step(infile);
                    if (frame!=tmp) printf("FRAME %d\n",(int)frame);
                }
                SetNoDraw(FXFALSE);
                while (playing && (frame<framebreak) && !spc) {
                    tmp=frame;
                    play_step(infile);
                    if (kbhit())
                        spc=FXTRUE;
                    if (frame!=tmp) printf("FRAME %d\n",(int)frame);
                }
            }
            break;
        } /* switch */
      } /* while */    
    }
    if (glideInit) {
      grGlideShutdown();
      if (echo)
        printf("\t\t< automatically calling grGlideShutdown() >\n");
    }

    PlayDone(infile);
    printf("\nDone.\n");

    return 0;
}
