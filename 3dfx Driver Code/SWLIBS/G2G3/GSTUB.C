#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stddef.h>
#include <3dfx.h>
#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <glide2.h>
#include <glide3.h>
#include <g3.h>
#include <gdebug.h>
#include <windows.h>

static FILE *errfile;

static float unbias_value[4] = {
    /* 00, 1<<19, pre-bias number was positive */ (float)(1<<19),
    /* 01, 3<<18, pre-bias number was negative */ (float)(3<<18),
    /* 10, 3<<18, pre-bias number was positive */ (float)(3<<18),
    /* 11, 1<<19, pre-bias number was negative */ (float)(1<<19)
};

#define UNBIAS(a)                                   \
  if ((a) > (float)(1<<15)) {  /* 32K */            \
    (a) -= unbias_value[((*(FxU32*)&(a))>>21)&3]; }

typedef struct{
  float x, y, z;          /* X, Y, Z */
  float r, g, b;          /* R, G, B */
  float ooz;              /* 65535/Z (used for Z-buffering) */
  float a;                /* Alpha */
  float oow;              /* 1/W (used for W-buffering, texturing) */
  GrTmuVertex  tmuvtx[GLIDE_NUM_TMU];
} Glide2Vertex;

#define DLL_GET(a,b) g3##a = (G3##a##PTR) GetProcAddress(module, "_"#a#b); \
    if (g3##a == NULL) { \
        fprintf(errfile, "Cannot GetProcAddress for: %s\n", #a); \
        return 0; \
    } else

static int grGlide3Init(void) {
    static HINSTANCE module;

    if ((errfile = fopen("stub.err", "w")) == NULL) {
        fprintf(stderr, "Cannot create log file\n");
        return 0;
    }
    
    if ((module = LoadLibrary("GLIDE3X")) == 0) {
        fprintf(errfile, "Cannot load glide3x.dll\n");
        return 0;
    }

    // get G3 routines

    DLL_GET(grDrawPoint, @4);
    DLL_GET(grDrawLine, @8);
    DLL_GET(grDrawTriangle, @12);
    DLL_GET(grVertexLayout, @12);
    DLL_GET(grDrawVertexArray, @12);
    DLL_GET(grDrawVertexArrayContiguous, @16);
    DLL_GET(grAADrawTriangle, @24);
    DLL_GET(grBufferClear, @12);
    DLL_GET(grBufferSwap, @4);
    DLL_GET(grRenderBuffer, @4);
    DLL_GET(grErrorSetCallback, @4);
    DLL_GET(grFinish, @0);
    DLL_GET(grFlush, @0);
    DLL_GET(grSstWinOpen, @28);
    DLL_GET(grSstWinClose, @4);
    DLL_GET(grSelectContext, @4);
    DLL_GET(grSstOrigin, @4);
    DLL_GET(grSstSelect, @4);
    DLL_GET(grAlphaBlendFunction, @16);
    DLL_GET(grAlphaCombine, @20);
    DLL_GET(grAlphaControlsITRGBLighting, @4);
    DLL_GET(grAlphaTestFunction, @4);
    DLL_GET(grAlphaTestReferenceValue, @4);
    DLL_GET(grChromakeyMode, @4);
    DLL_GET(grChromakeyValue, @4);
    DLL_GET(grClipWindow, @16);
    DLL_GET(grColorCombine, @20);
    DLL_GET(grColorMask, @8);
    DLL_GET(grConstantColorValue, @4);
    DLL_GET(grCullMode, @4);
    DLL_GET(grDepthBiasLevel, @4);
    DLL_GET(grDepthBufferFunction, @4);
    DLL_GET(grDepthBufferMode, @4);
    DLL_GET(grDepthMask, @4);
    DLL_GET(grDisableAllEffects, @0);
    DLL_GET(grDitherMode, @4);
    DLL_GET(grFogColorValue, @4);
    DLL_GET(grFogMode, @4);
    DLL_GET(grFogTable, @4);
    DLL_GET(grLoadGammaTable, @16);
    DLL_GET(grSplash, @20);
    DLL_GET(grGet, @12);
    DLL_GET(grGetString, @4);
    DLL_GET(grQueryResolutions, @8);
    DLL_GET(grReset, @4);
    DLL_GET(grGetProcAddress, @4);
    DLL_GET(grEnable, @4);
    DLL_GET(grDisable, @4);
    DLL_GET(grCoordinateSpace, @4);
    DLL_GET(grDepthRange, @8);
    DLL_GET(grViewport, @16);
    DLL_GET(grTexCalcMemRequired, @16);
    DLL_GET(grTexTextureMemRequired, @8);
    DLL_GET(grTexMinAddress, @4);
    DLL_GET(grTexMaxAddress, @4);
    DLL_GET(grTexNCCTable, @4);
    DLL_GET(grTexSource, @16);
    DLL_GET(grTexClampMode, @12);
    DLL_GET(grTexCombine, @28);
    DLL_GET(grTexDetailControl, @16);
    DLL_GET(grTexFilterMode, @12);
    DLL_GET(grTexLodBiasValue, @8);
    DLL_GET(grTexDownloadMipMap, @16);
    DLL_GET(grTexDownloadMipMapLevel, @32);
    DLL_GET(grTexDownloadMipMapLevelPartial, @40);
    DLL_GET(grTexDownloadTable, @8);
    DLL_GET(grTexDownloadTablePartial, @16);
    DLL_GET(grTexMipMapMode, @12);
    DLL_GET(grTexMultibase, @8);
    DLL_GET(grTexMultibaseAddress, @20);
    DLL_GET(grLfbLock, @24);
    DLL_GET(grLfbUnlock, @8);
    DLL_GET(grLfbConstantAlpha, @4);
    DLL_GET(grLfbConstantDepth, @4);
    DLL_GET(grLfbWriteColorSwizzle, @8);
    DLL_GET(grLfbWriteColorFormat, @4);
    DLL_GET(grLfbWriteRegion, @36);
    DLL_GET(grLfbReadRegion, @28);
    DLL_GET(grGlideInit, @0);
    DLL_GET(grGlideShutdown, @0);
    DLL_GET(grGlideGetState, @4);
    DLL_GET(grGlideSetState, @4);
    DLL_GET(grGlideGetVertexLayout, @4);
    DLL_GET(grGlideSetVertexLayout, @4);
    DLL_GET(gu3dfGetInfo, @8);
    DLL_GET(gu3dfLoad, @8);
    DLL_GET(guFogTableIndexToW, @4);
    DLL_GET(guFogGenerateExp, @8);
    DLL_GET(guFogGenerateExp2, @8);
    DLL_GET(guFogGenerateLinear, @12);

    return 1;
}

#define UNIMP(a) if (errfile) { \
    fprintf(errfile, "Unimplemented function %s (at line: %d)\n", \
                        #a,__LINE__); \
    fflush(errfile); \
} else

FX_EXPORT void FX_CSTYLE 
grDrawPlanarPolygon( int nverts, const int ilist[], const GrVertex vlist[]
) {
    int i;

    for (i = 0; i < nverts-2; i++) {
        grDrawTriangle(&vlist[ilist[0]],
                         &vlist[ilist[i+1]],
                         &vlist[ilist[i+2]]);
    }
}

FX_EXPORT void FX_CSTYLE
grDrawPlanarPolygonVertexList( int nverts, const GrVertex vlist[] ){
    g3grDrawVertexArrayContiguous(G3GR_POLYGON, nverts,
				  (void *)vlist, sizeof(GrVertex));
}

FX_EXPORT void FX_CSTYLE
grDrawPolygon( int nverts, const int ilist[], const GrVertex vlist[] ){
    int i;

    for (i = 0; i < nverts-2; i++) {
        grDrawTriangle(&vlist[ilist[0]],
                         &vlist[ilist[i+1]],
                         &vlist[ilist[i+2]]);
    }
}


FX_EXPORT void FX_CSTYLE
grDrawPolygonVertexList( int nverts, const GrVertex vlist[] ){
    g3grDrawVertexArrayContiguous(G3GR_POLYGON, nverts,
				  (void *)vlist, sizeof(GrVertex));
}

FX_EXPORT void FX_CSTYLE
grDrawPoint( const GrVertex *pt ){
// LOOOK horrible hack until we fix it in hardware
#ifndef notdef
    GrVertex va = *pt;
    UNBIAS(va.x);
    UNBIAS(va.y);
    g3grDrawPoint(&va);
#else
    g3grDrawPoint(pt);
#endif
}

FX_EXPORT void FX_CSTYLE
grDrawLine( const GrVertex *v1, const GrVertex *v2 ){
// LOOOK horrible hack until we fix it in hardware
#ifndef notdef
    GrVertex va = *v1, vb = *v2;
    UNBIAS(va.x);
    UNBIAS(va.y);
    UNBIAS(vb.x);
    UNBIAS(vb.y);
    g3grDrawLine(&va, &vb);
#else
    g3grDrawLine(v1, v2);
#endif
}

FX_EXPORT void FX_CSTYLE
grDrawTriangle( const GrVertex *a, const GrVertex *b, const GrVertex *c ){
// LOOOK horrible hack until we fix it in hardware
#ifndef notdef
    GrVertex va = *a, vb = *b, vc = *c;
    UNBIAS(va.x);
    UNBIAS(va.y);
    UNBIAS(vb.x);
    UNBIAS(vb.y);
    UNBIAS(vc.x);
    UNBIAS(vc.y);
    g3grDrawTriangle(&va, &vb, &vc);
#else
    g3grDrawTriangle(a, b, c);
#endif
}

FX_EXPORT void FX_CSTYLE
grBufferClear( GrColor_t color, GrAlpha_t alpha, FxU16 depth ){
    g3grBufferClear(color, alpha, depth);
}

FX_EXPORT int FX_CSTYLE
grBufferNumPending( void ){
    int pending;
    g3grGet(G3GR_PENDING_BUFFERSWAPS, sizeof(int), &pending);
    return pending;
}

FX_EXPORT void FX_CSTYLE
grBufferSwap( int swap_interval ){
    g3grBufferSwap(swap_interval);
}

FX_EXPORT void FX_CSTYLE
grRenderBuffer( GrBuffer_t buffer ){
    g3grRenderBuffer(buffer);    
}

FX_EXPORT void FX_CSTYLE 
grErrorSetCallback( GrErrorCallbackFnc_t fnc ){
    g3grErrorSetCallback(fnc);
}

FX_EXPORT void FX_CSTYLE 
grSstIdle(void){
    g3grFinish();
}

FX_EXPORT FxU32 FX_CSTYLE 
grSstVideoLine( void ){
    UNIMP(grSstVideoLine);
    return 0;
}

FX_EXPORT FxBool FX_CSTYLE 
grSstVRetraceOn( void ){
    UNIMP(grSstVRetraceOn);
    return 1;
}

FX_EXPORT FxBool FX_CSTYLE 
grSstIsBusy( void ){
    UNIMP(grSstIsBusy);
    return 0;
}

static G3GrContext_t g3Context = 0;

FX_EXPORT FxBool FX_CSTYLE 
grSstWinOpen( FxU32                hWnd,
              GrScreenResolution_t screen_resolution,
              GrScreenRefresh_t    refresh_rate,
              GrColorFormat_t      color_format,
              GrOriginLocation_t   origin_location,
              int                  nColBuffers,
              int                  nAuxBuffers) {

// hack for Mango/Barrage

if(!hWnd)
    hWnd = (FxU32)GetForegroundWindow();

    if ( g3Context == 0 ) {
        g3Context = g3grSstWinOpen(hWnd, screen_resolution, refresh_rate,
                                   color_format, origin_location, 
                                   nColBuffers, nAuxBuffers);
    } else {
        // LOOOK should really start from scratch
        grSstOrigin( origin_location );
    }

    g3grCoordinateSpace(G3GR_WINDOW_COORDS);
    g3grReset(G3GR_VERTEX_PARAMETER);
    g3grVertexLayout(G3GR_PARAM_XY, offsetof(Glide2Vertex, x),
                     G3GR_PARAM_ENABLE);
    g3grVertexLayout(G3GR_PARAM_RGB, offsetof(Glide2Vertex, r),
                     G3GR_PARAM_ENABLE);
    g3grVertexLayout(G3GR_PARAM_Z, offsetof(Glide2Vertex, ooz),
                     G3GR_PARAM_ENABLE);
    g3grVertexLayout(G3GR_PARAM_A, offsetof(Glide2Vertex, a),
                     G3GR_PARAM_ENABLE);
    g3grVertexLayout(G3GR_PARAM_Q, offsetof(Glide2Vertex, oow),
                     G3GR_PARAM_ENABLE);

#ifndef notdef
#ifndef notdef
    g3grVertexLayout(G3GR_PARAM_Q0, offsetof(Glide2Vertex, oow),
                     G3GR_PARAM_ENABLE);
#else
    g3grVertexLayout(G3GR_PARAM_Q0, offsetof(Glide2Vertex, tmuvtx[0].oow),
                     G3GR_PARAM_ENABLE);
#endif
#endif

    g3grVertexLayout(G3GR_PARAM_ST0, offsetof(Glide2Vertex, tmuvtx[0].sow),
                     G3GR_PARAM_ENABLE);
    g3grVertexLayout(G3GR_PARAM_ST1, offsetof(Glide2Vertex, tmuvtx[1].sow),
                     G3GR_PARAM_ENABLE);

    return (g3Context != 0);
}

FX_EXPORT void FX_CSTYLE
grSstWinClose( void ){
    g3grSstWinClose(g3Context);
}

FX_EXPORT FxBool FX_CSTYLE
grSstControl( FxU32 code ){
    UNIMP(grSstControl);
    return FXTRUE;
}

FX_EXPORT FxBool FX_CSTYLE 
grSstQueryHardware( GrHwConfiguration *hwconfig )
{
    int i, j, num_boards, num_tmus, fbi_rev, fb_ram, tmu_rev, tmu_ram;
    char *board_name;

    g3grGet(G3GR_NUM_BOARDS, sizeof(int), &num_boards);

    memset(hwconfig, 0, sizeof(GrHwConfiguration));
    if (num_boards > 0) {

        hwconfig->num_sst = num_boards;
        board_name = (char*)g3grGetString(G3GR_HARDWARE);
        g3grGet(G3GR_NUM_TMU, 4, &num_tmus);
        g3grGet(G3GR_REVISION_FB, 4, &fbi_rev);
        g3grGet(G3GR_MEMORY_FB, 4, &fb_ram);
        fb_ram >>= 20;  // turn bytes into Mbytes
        g3grGet(G3GR_REVISION_TMU, 4, &tmu_rev);
        g3grGet(G3GR_MEMORY_TMU, 4, &tmu_ram);
        tmu_ram >>= 20; // turn bytes into Mbytes

        for ( i=0 ; i<num_boards ; i++ ) {

          if (strstr(board_name, "Rush")    ||
              strstr(board_name, "Banshee") ||
              strstr(board_name, "Voodoo3") ||
              strstr(board_name, "SST2")) {
          
            if (strstr(board_name, "Rush"))
              hwconfig->SSTs[i].type = GR_SSTTYPE_SST96;
            else
              hwconfig->SSTs[i].type = GR_SSTTYPE_Banshee;

            hwconfig->SSTs[i].sstBoard.SST96Config.fbRam = fb_ram;
            hwconfig->SSTs[i].sstBoard.SST96Config.nTexelfx = num_tmus;
            hwconfig->SSTs[i].sstBoard.SST96Config.tmuConfig.tmuRev = tmu_rev;
            hwconfig->SSTs[i].sstBoard.SST96Config.tmuConfig.tmuRam = tmu_ram;
          } else {

            // "Voodoo Graphics", "Voodoo2", and anything else

            if (strstr(board_name, "Voodoo2"))
              // this ugly little bit of code mimics the behavior in
              // $/devel/cvg/glide/src/disst.c:grSstQueryHardware()
              hwconfig->SSTs[i].type = !getenv("FX_GLIDE_REPORT_REAL_HW")
                                       ? GR_SSTTYPE_VOODOO
                                       : GR_SSTTYPE_Voodoo2;
            else
              hwconfig->SSTs[i].type = GR_SSTTYPE_VOODOO;

            hwconfig->SSTs[i].sstBoard.VoodooConfig.fbRam = fb_ram;
            hwconfig->SSTs[i].sstBoard.VoodooConfig.fbiRev = fbi_rev;
            hwconfig->SSTs[i].sstBoard.VoodooConfig.nTexelfx = num_tmus;
            hwconfig->SSTs[i].sstBoard.VoodooConfig.sliDetect = FXFALSE;
            for ( j=0 ; j<num_tmus ; j++ ) {
               hwconfig->SSTs[i].sstBoard.VoodooConfig.tmuConfig[j].tmuRev = tmu_rev;
               hwconfig->SSTs[i].sstBoard.VoodooConfig.tmuConfig[j].tmuRam = tmu_ram;
            }
          }
        }
        return FXTRUE;
    } else {
        return FXFALSE;
    }
}

FX_EXPORT FxBool FX_CSTYLE 
grSstQueryBoards( GrHwConfiguration *hwconfig ){
    UNIMP(grSstQueryBoards);
    return FXTRUE;
}

FX_EXPORT void FX_CSTYLE
grSstOrigin(GrOriginLocation_t  origin){
    g3grSstOrigin(origin);
}

FX_EXPORT void FX_CSTYLE 
grSstSelect( int which_sst ){
    g3grSstSelect(which_sst);
}

FX_EXPORT FxU32 FX_CSTYLE 
grSstScreenHeight( void ){
    FxI32 viewport[4];
    g3grGet(G3GR_VIEWPORT, sizeof(viewport[0])*4, viewport);
    return viewport[3];
}

FX_EXPORT FxU32 FX_CSTYLE 
grSstScreenWidth( void ){
    FxI32 viewport[4];
    g3grGet(G3GR_VIEWPORT, sizeof(viewport[0])*4, viewport);
    return viewport[2];
}

FX_EXPORT FxU32 FX_CSTYLE 
grSstStatus( void ){
	UNIMP(grSstStatus);
    return 0;
}

FX_EXPORT void FX_CSTYLE
grSstPerfStats(GrSstPerfStats_t *pStats){
    UNIMP(grSstPerfStats);    
}

FX_EXPORT void FX_CSTYLE
grSstResetPerfStats(void){
    UNIMP(grSstResetPerfStats);    
}

FX_EXPORT void FX_CSTYLE
grResetTriStats(){
    UNIMP(grResetTriStats);    
}

FX_EXPORT void FX_CSTYLE
grTriStats(FxU32 *trisProcessed, FxU32 *trisDrawn){
    UNIMP(grTriStats);    
}

FX_EXPORT void FX_CSTYLE
grAlphaBlendFunction(
                     GrAlphaBlendFnc_t rgb_sf,   GrAlphaBlendFnc_t rgb_df,
                     GrAlphaBlendFnc_t alpha_sf, GrAlphaBlendFnc_t alpha_df
                     ){
    g3grAlphaBlendFunction(rgb_sf, rgb_df, alpha_sf, alpha_df);
}

FX_EXPORT void FX_CSTYLE
grAlphaCombine(
               GrCombineFunction_t function, GrCombineFactor_t factor,
               GrCombineLocal_t local, GrCombineOther_t other,
               FxBool invert
               ){
    g3grAlphaCombine(function, factor, local, other, invert);
}

FX_EXPORT void FX_CSTYLE
grAlphaControlsITRGBLighting( FxBool enable ){
    g3grAlphaControlsITRGBLighting(enable);
    // UNIMP(grAlphaControlsITRGBLighting);    
}

FX_EXPORT void FX_CSTYLE
grAlphaTestFunction( GrCmpFnc_t function ){
    g3grAlphaTestFunction(function);
}

FX_EXPORT void FX_CSTYLE
grAlphaTestReferenceValue( GrAlpha_t value ){
    g3grAlphaTestReferenceValue(value);
}

FX_EXPORT void FX_CSTYLE 
grChromakeyMode( GrChromakeyMode_t mode ){
    g3grChromakeyMode(mode);
}

FX_EXPORT void FX_CSTYLE 
grChromakeyValue( GrColor_t value ){
    g3grChromakeyValue(value);
}

FX_EXPORT void FX_CSTYLE 
grClipWindow( FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy ){
    // hack for Mango/Barrage
    if ( g3Context == 0 ) {
       grSstWinOpen(0,GR_RESOLUTION_640x480, GR_REFRESH_60Hz,GR_COLORFORMAT_ARGB,GR_ORIGIN_UPPER_LEFT,2,1);
    }
    g3grClipWindow(minx, miny, maxx, maxy);
}

FX_EXPORT void FX_CSTYLE 
grColorCombine(GrCombineFunction_t function, GrCombineFactor_t factor,
               GrCombineLocal_t local, GrCombineOther_t other,
               FxBool invert ){
    g3grColorCombine(function, factor, local, other, invert);  
}

FX_EXPORT void FX_CSTYLE
grColorMask( FxBool rgb, FxBool a ){
//    g3grColorMask(rgb, a);
    g3grColorMask(rgb, 0);
}

FX_EXPORT void FX_CSTYLE 
grCullMode( GrCullMode_t mode ){
    g3grCullMode(mode);    
}

FX_EXPORT void FX_CSTYLE 
grConstantColorValue( GrColor_t value ){
    g3grConstantColorValue(value);
}

FX_EXPORT void FX_CSTYLE 
grConstantColorValue4( float a, float r, float g, float b ){
    UNIMP(grConstantColorValue4);    
}

FX_EXPORT void FX_CSTYLE 
grDepthBiasLevel( FxI16 level ){
    g3grDepthBiasLevel(level);
}

FX_EXPORT void FX_CSTYLE 
grDepthBufferFunction( GrCmpFnc_t function ){
    g3grDepthBufferFunction(function);
}

FX_EXPORT void FX_CSTYLE 
grDepthBufferMode( GrDepthBufferMode_t mode ){
    g3grDepthBufferMode(mode);
}

FX_EXPORT void FX_CSTYLE 
grDepthMask( FxBool mask ){
    g3grDepthMask(mask);
}

FX_EXPORT void FX_CSTYLE 
grDisableAllEffects( void ){
    g3grDisableAllEffects();
    // UNIMP(grDisableAllEffects);    
}

FX_EXPORT void FX_CSTYLE 
grDitherMode( GrDitherMode_t mode ){
    g3grDitherMode(mode);
}

FX_EXPORT void FX_CSTYLE 
grFogColorValue( GrColor_t fogcolor ){
    g3grFogColorValue(fogcolor);
}

FX_EXPORT void FX_CSTYLE 
grFogMode( GrFogMode_t mode ){
    // LOOOK disabled for barrarge, 
    g3grFogMode(mode);
}

FX_EXPORT void FX_CSTYLE 
grFogTable( const GrFog_t ft[] ){
    int tableSize;
    g3grGet(G3GR_FOG_TABLE_ENTRIES, sizeof(tableSize), &tableSize);
    g3grFogTable(ft);
}

FX_EXPORT void FX_CSTYLE 
grGammaCorrectionValue( float value ){
    UNIMP(grGammaCorrectionValue);
}

FX_EXPORT void FX_CSTYLE
grSplash(float x, float y, float width, float height, FxU32 frame){
    UNIMP(grSplash);    
}

FX_EXPORT FxU32 FX_CSTYLE 
grTexTextureMemRequired( FxU32     evenOdd,
                         GrTexInfo *info   ) {
    G3GrTexInfo g3info;

    g3info.smallLodLog2 = 8-info->smallLod;
    g3info.largeLodLog2 = 8-info->largeLod;
    g3info.aspectRatioLog2 = -(info->aspectRatio-3);
    g3info.format = info->format;
    g3info.data = info->data;
    return g3grTexTextureMemRequired(evenOdd, &g3info);
}

FX_EXPORT FxU32 FX_CSTYLE 
grTexCalcMemRequired ( GrLOD_t small_lod, 
                       GrLOD_t large_lod, 
                       GrAspectRatio_t aspect, 
                       GrTextureFormat_t format ) {
    return g3grTexCalcMemRequired(8-small_lod,
                                  8-large_lod,
                                  -(aspect-3),
                                  format);
}

FX_EXPORT FxU32 FX_CSTYLE 
grTexMinAddress( GrChipID_t tmu ){
    return g3grTexMinAddress(tmu);
}


FX_EXPORT FxU32 FX_CSTYLE 
grTexMaxAddress( GrChipID_t tmu ){
    return g3grTexMaxAddress(tmu);
}


FX_EXPORT void FX_CSTYLE 
grTexNCCTable( GrChipID_t tmu, GrNCCTable_t table ){
    g3grTexNCCTable(table);
    // UNIMP(grTexNCCTable);    
}

FX_EXPORT void FX_CSTYLE 
grTexSource( GrChipID_t tmu,
             FxU32      startAddress,
             FxU32      evenOdd,
             GrTexInfo  *info ){
    G3GrTexInfo g3info;

    g3info.smallLodLog2 = 8-info->smallLod;
    g3info.largeLodLog2 = 8-info->largeLod;
    g3info.aspectRatioLog2 = -(info->aspectRatio-3);
    g3info.format = info->format;
    g3info.data = info->data;
    g3grTexSource(tmu, startAddress, evenOdd, &g3info);
}

FX_EXPORT void FX_CSTYLE 
grTexClampMode(
               GrChipID_t tmu,
               GrTextureClampMode_t s_clampmode,
               GrTextureClampMode_t t_clampmode
               ){
    g3grTexClampMode(tmu, s_clampmode, t_clampmode);
}

FX_EXPORT void FX_CSTYLE 
grTexCombine(
             GrChipID_t tmu,
             GrCombineFunction_t rgb_function,
             GrCombineFactor_t rgb_factor, 
             GrCombineFunction_t alpha_function,
             GrCombineFactor_t alpha_factor,
             FxBool rgb_invert,
             FxBool alpha_invert
             ){
    g3grTexCombine(tmu, rgb_function, rgb_factor, alpha_function, alpha_factor,
                   rgb_invert, alpha_invert);
}

FX_EXPORT void FX_CSTYLE 
grTexCombineFunction(
                     GrChipID_t tmu,
                     GrTextureCombineFnc_t fnc
                     ){
    guTexCombineFunction(tmu, fnc);
}

FX_EXPORT void FX_CSTYLE 
grTexDetailControl(
                   GrChipID_t tmu,
                   int lod_bias,
                   FxU8 detail_scale,
                   float detail_max
                   ){
    // grTexDetailControl(tmu, lod_bias, detail_scale, detail_max);
    // UNIMP(grTexDetailControl);
}

FX_EXPORT void FX_CSTYLE 
grTexFilterMode(
                GrChipID_t tmu,
                GrTextureFilterMode_t minfilter_mode,
                GrTextureFilterMode_t magfilter_mode
                ){
    g3grTexFilterMode(tmu, minfilter_mode, magfilter_mode);
}


FX_EXPORT void FX_CSTYLE 
grTexLodBiasValue(GrChipID_t tmu, float bias ){    
    g3grTexLodBiasValue(tmu, bias);
}

FX_EXPORT void FX_CSTYLE 
grTexDownloadMipMap( GrChipID_t tmu,
                     FxU32      startAddress,
                     FxU32      evenOdd,
                     GrTexInfo  *info ){
    G3GrTexInfo g3info;

    g3info.smallLodLog2 = 8-info->smallLod;
    g3info.largeLodLog2 = 8-info->largeLod;
    g3info.aspectRatioLog2 = -(info->aspectRatio-3);
    g3info.format = info->format;
    g3info.data = info->data;
    g3grTexDownloadMipMap(tmu, startAddress, evenOdd, &g3info);
}

FX_EXPORT void FX_CSTYLE 
grTexDownloadMipMapLevel( GrChipID_t        tmu,
                          FxU32             startAddress,
                          GrLOD_t           thisLod,
                          GrLOD_t           largeLod,
                          GrAspectRatio_t   aspectRatio,
                          GrTextureFormat_t format,
                          FxU32             evenOdd,
                          void              *data ){
    g3grTexDownloadMipMapLevel(tmu,
                               startAddress,
                               8-thisLod,
                               8-largeLod,
                               -(aspectRatio-3),
                               format,
                               evenOdd,
                               data);
}

FX_EXPORT void FX_CSTYLE 
grTexDownloadMipMapLevelPartial( GrChipID_t        tmu,
                                 FxU32             startAddress,
                                 GrLOD_t           thisLod,
                                 GrLOD_t           largeLod,
                                 GrAspectRatio_t   aspectRatio,
                                 GrTextureFormat_t format,
                                 FxU32             evenOdd,
                                 void              *data,
                                 int               start,
                                 int               end ){
    g3grTexDownloadMipMapLevelPartial(tmu,
              startAddress,
				      8-thisLod,
				      8-largeLod,
				      -(aspectRatio-3),
				      format,
				      evenOdd,
				      data,
				      start,
				      end);
}

FX_EXPORT void FX_CSTYLE 
grCheckForRoom(FxI32 n){
    UNIMP(grCheckForRoom);    
}

FX_EXPORT void FX_CSTYLE
grTexDownloadTable( GrChipID_t   tmu,
	 			    GrTexTable_t type, 
                    void         *data ){
    g3grTexDownloadTable(type, data);
}

FX_EXPORT void FX_CSTYLE
grTexDownloadTablePartial( GrChipID_t   tmu,
                           GrTexTable_t type, 
                           void         *data,
                           int          start,
                           int          end ){
    g3grTexDownloadTablePartial(type, data, start, end);
}

FX_EXPORT void FX_CSTYLE 
grTexMipMapMode( GrChipID_t     tmu, 
                 GrMipMapMode_t mode,
                 FxBool         lodBlend ){
    g3grTexMipMapMode(tmu, mode, lodBlend);
}

FX_EXPORT void FX_CSTYLE 
grTexMultibase( GrChipID_t tmu,
                FxBool     enable ){
	UNIMP(grTexMultibase);
}

FX_EXPORT void FX_CSTYLE
grTexMultibaseAddress( GrChipID_t       tmu,
                       GrTexBaseRange_t range,
                       FxU32            startAddress,
                       FxU32            evenOdd,
                       GrTexInfo        *info ){
	UNIMP(grTexMultibaseAddress);
}

#define MAX_MIPMAPS 1024
static FxU32 texMemBase;
static GrMipMapId_t currentMipMap;
static GrMipMapInfo mipMaps[MAX_MIPMAPS];

FX_EXPORT GrMipMapId_t FX_CSTYLE 
guTexAllocateMemory( GrChipID_t tmu,
                     FxU8 odd_even_mask,
                     int width, int height,
                     GrTextureFormat_t format,
                     GrMipMapMode_t mm_mode,
                     GrLOD_t small_lod, GrLOD_t large_lod,
                     GrAspectRatio_t aspect_ratio,
                     GrTextureClampMode_t s_clamp_mode,
                     GrTextureClampMode_t t_clamp_mode,
                     GrTextureFilterMode_t minfilter_mode,
                     GrTextureFilterMode_t magfilter_mode,
                     float lod_bias,
                     FxBool trilinear ) {
    GrMipMapId_t id;
    FxU32 memRequired, memAvailable;
    G3GrTexInfo g3info;

    g3info.smallLodLog2 = 8-small_lod;
    g3info.largeLodLog2 = 8-large_lod;
    g3info.aspectRatioLog2 = -(aspect_ratio-3);
    g3info.format = format;
    memRequired = g3grTexTextureMemRequired(odd_even_mask, &g3info);

    if ((texMemBase < 0x200000) && (texMemBase + memRequired > 0x200000)) {
        texMemBase = 0x200000;
    }

    memAvailable = guTexMemQueryAvail(tmu);

    if ((memAvailable < memRequired) ||
        (currentMipMap >= MAX_MIPMAPS)) {
        return GR_NULL_MIPMAP_HANDLE;
    }

    id = currentMipMap++;
    mipMaps[id].tmu            = tmu;
    mipMaps[id].odd_even_mask  = odd_even_mask;
    mipMaps[id].width          = width;
    mipMaps[id].height         = height;
    mipMaps[id].format         = format;
    mipMaps[id].mipmap_mode    = mm_mode;
    mipMaps[id].magfilter_mode = magfilter_mode;
    mipMaps[id].minfilter_mode = minfilter_mode;
    mipMaps[id].s_clamp_mode   = s_clamp_mode;
    mipMaps[id].t_clamp_mode   = t_clamp_mode;
    mipMaps[id].lod_min        = 8-small_lod;
    mipMaps[id].lod_max        = 8-large_lod;
    mipMaps[id].aspect_ratio   = -(aspect_ratio-3);
    mipMaps[id].lod_bias       = (FxU32)lod_bias;
    mipMaps[id].trilinear      = trilinear;
    mipMaps[id].data           = 0;
    mipMaps[id].valid          = FXTRUE;
    mipMaps[id].tmu_base_address = texMemBase;
    texMemBase += memRequired;

    return id;
}

FX_EXPORT FxBool FX_CSTYLE 
guTexChangeAttributes(
                      GrMipMapId_t mmid,
                      int width, int height,
                      GrTextureFormat_t fmt,
                      GrMipMapMode_t mm_mode,
                      GrLOD_t smallest_lod, GrLOD_t largest_lod,
                      GrAspectRatio_t aspect,
                      GrTextureClampMode_t s_clamp_mode,
                      GrTextureClampMode_t t_clamp_mode,
                      GrTextureFilterMode_t minFilterMode,
                      GrTextureFilterMode_t magFilterMode
                      ){
	UNIMP(guTexChangeAttributes);
    return FXTRUE;
}

FX_EXPORT void FX_CSTYLE 
guTexCombineFunction(
                     GrChipID_t tmu,
                     GrTextureCombineFnc_t fnc
                     ){
    switch ( fnc )  {
    case GR_TEXTURECOMBINE_ZERO:
        g3grTexCombine( tmu, GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
                        GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
			FXFALSE, FXFALSE );
	break;
    case GR_TEXTURECOMBINE_DECAL:
        g3grTexCombine( tmu, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
			GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
			FXFALSE, FXFALSE );
	break;
    case GR_TEXTURECOMBINE_ONE:
        g3grTexCombine( tmu, GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
			GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
			FXTRUE, FXTRUE );
	break;
    case GR_TEXTURECOMBINE_ADD:
        g3grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
			GR_COMBINE_FACTOR_ONE,
			GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
			GR_COMBINE_FACTOR_ONE, FXFALSE, FXFALSE );
	break;
    case GR_TEXTURECOMBINE_MULTIPLY:
        g3grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER,
			GR_COMBINE_FACTOR_LOCAL,
			GR_COMBINE_FUNCTION_SCALE_OTHER,
			GR_COMBINE_FACTOR_LOCAL, FXFALSE, FXFALSE );
	break;
    case GR_TEXTURECOMBINE_DETAIL:
        g3grTexCombine( tmu, GR_COMBINE_FUNCTION_BLEND,
			GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR,
			GR_COMBINE_FUNCTION_BLEND,
			GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR,
			FXFALSE, FXFALSE );
	break;
    case GR_TEXTURECOMBINE_DETAIL_OTHER:
        g3grTexCombine( tmu, GR_COMBINE_FUNCTION_BLEND,
			GR_COMBINE_FACTOR_DETAIL_FACTOR,
			GR_COMBINE_FUNCTION_BLEND,
			GR_COMBINE_FACTOR_DETAIL_FACTOR, FXFALSE, FXFALSE );
	break;
    case GR_TEXTURECOMBINE_TRILINEAR_ODD:
        g3grTexCombine( tmu, GR_COMBINE_FUNCTION_BLEND,
			GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION,
			GR_COMBINE_FUNCTION_BLEND,
			GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION,
			FXFALSE, FXFALSE );
	break;
    case GR_TEXTURECOMBINE_TRILINEAR_EVEN:
        g3grTexCombine( tmu, GR_COMBINE_FUNCTION_BLEND,
			GR_COMBINE_FACTOR_LOD_FRACTION,
			GR_COMBINE_FUNCTION_BLEND,
			GR_COMBINE_FACTOR_LOD_FRACTION, FXFALSE, FXFALSE );
	break;
    case GR_TEXTURECOMBINE_SUBTRACT:
        g3grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
			GR_COMBINE_FACTOR_ONE,
			GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
			GR_COMBINE_FACTOR_ONE, FXFALSE, FXFALSE );
	break;
    case GR_TEXTURECOMBINE_OTHER:
        g3grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER,
			GR_COMBINE_FACTOR_ONE,
			GR_COMBINE_FUNCTION_SCALE_OTHER,
			GR_COMBINE_FACTOR_ONE, FXFALSE, FXFALSE );
	break;
    }
}

FX_EXPORT GrMipMapId_t FX_CSTYLE 
guTexGetCurrentMipMap( GrChipID_t tmu ){
	UNIMP(guTexGetCurrentMipMap);
    return 0;
}

FX_EXPORT GrMipMapInfo * FX_CSTYLE 
guTexGetMipMapInfo( GrMipMapId_t mmid ){
    static GrMipMapInfo foo;
	UNIMP(guTexGetMipMapInfo);
    return &foo;
}

FX_EXPORT FxU32 FX_CSTYLE 
guTexMemQueryAvail( GrChipID_t tmu ){
    return (g3grTexMaxAddress(tmu) - texMemBase + 8);
}

FX_EXPORT void FX_CSTYLE 
guTexMemReset( void ){
    UNIMP(guTexMemReset);    
}

FX_EXPORT void FX_CSTYLE 
guTexDownloadMipMap(
                    GrMipMapId_t mmid,
                    const void *src,
                    const GuNccTable *table
                    ){
    G3GrTexInfo g3info;

    if (table) {
        fprintf(errfile, "ncctable is valid\n");
        fflush(errfile);
    }
    mipMaps[mmid].data = (void *)src;
    g3info.smallLodLog2 = mipMaps[mmid].lod_min;
    g3info.largeLodLog2 = mipMaps[mmid].lod_max;
    g3info.aspectRatioLog2 = mipMaps[mmid].aspect_ratio;
    g3info.format = mipMaps[mmid].format;
    g3info.data = mipMaps[mmid].data;
    g3grTexDownloadMipMap(mipMaps[mmid].tmu, mipMaps[mmid].tmu_base_address, 
			  mipMaps[mmid].odd_even_mask, &g3info);
}

FX_EXPORT void FX_CSTYLE 
guTexDownloadMipMapLevel(
                         GrMipMapId_t mmid,
                         GrLOD_t lod,
                         const void **src
                         ){
    UNIMP(guTexDownloadMipMapLevel);
}

FX_EXPORT void FX_CSTYLE 
guTexSource( GrMipMapId_t mmid ){
    GrMipMapInfo *m = &mipMaps[mmid];
    G3GrTexInfo g3info;

    g3info.smallLodLog2 = m->lod_min;
    g3info.largeLodLog2 = m->lod_max;
    g3info.aspectRatioLog2 = m->aspect_ratio;
    g3info.format = m->format;
    g3info.data = m->data;
    g3grTexSource(m->tmu, m->tmu_base_address, 
		  m->odd_even_mask, &g3info);
    g3grTexFilterMode(m->tmu, m->minfilter_mode, m->magfilter_mode);
    g3grTexMipMapMode(m->tmu, m->mipmap_mode, 0);
    g3grTexClampMode(m->tmu, m->s_clamp_mode, m->t_clamp_mode);
}

FX_EXPORT FxBool FX_CSTYLE
grLfbLock( GrLock_t type, GrBuffer_t buffer, GrLfbWriteMode_t writeMode,
           GrOriginLocation_t origin, FxBool pixelPipeline, 
           GrLfbInfo_t *info ){
    return g3grLfbLock(type, buffer, writeMode, origin, pixelPipeline, 
                       (G3GrLfbInfo_t *)info);
}

FX_EXPORT FxBool FX_CSTYLE
grLfbUnlock( GrLock_t type, GrBuffer_t buffer ){
    return g3grLfbUnlock(type, buffer);
}

FX_EXPORT void FX_CSTYLE 
grLfbConstantAlpha( GrAlpha_t alpha ){
    UNIMP(grLfbConstantAlpha);    
}

FX_EXPORT void FX_CSTYLE 
grLfbConstantDepth( FxU16 depth ){
    UNIMP(grLfbConstantDepth);    
}

FX_EXPORT void FX_CSTYLE 
grLfbWriteColorSwizzle(FxBool swizzleBytes, FxBool swapWords){
    UNIMP(grLfbWriteColorSwizzle);    
}

FX_EXPORT void FX_CSTYLE
grLfbWriteColorFormat(GrColorFormat_t colorFormat){
    UNIMP(grLfbWriteColorFormat);    
}


FX_EXPORT FxBool FX_CSTYLE
grLfbWriteRegion( GrBuffer_t dst_buffer, 
                  FxU32 dst_x, FxU32 dst_y, 
                  GrLfbSrcFmt_t src_format, 
                  FxU32 src_width, FxU32 src_height, 
                  FxI32 src_stride, void *src_data ){
    return g3grLfbWriteRegion(dst_buffer, dst_x, dst_y,
                              src_format, src_width, src_height,
                              FXFALSE,
                              src_stride, src_data);
}

FX_EXPORT FxBool FX_CSTYLE
grLfbReadRegion( GrBuffer_t src_buffer,
                 FxU32 src_x, FxU32 src_y,
                 FxU32 src_width, FxU32 src_height,
                 FxU32 dst_stride, void *dst_data ){
    return g3grLfbReadRegion(src_buffer, src_x, src_y,
                             src_width, src_height,
                             dst_stride, dst_data);
}

FX_EXPORT void FX_CSTYLE
grAADrawLine(const GrVertex *v1, const GrVertex *v2){
	g3grEnable(G3GR_AA_ORDERED);
	grDrawLine(v1, v2);
	g3grDisable(G3GR_AA_ORDERED);
}

FX_EXPORT void FX_CSTYLE
grAADrawPoint(const GrVertex *pt ){
	g3grEnable(G3GR_AA_ORDERED);
	g3grDrawPoint(pt);
	g3grDisable(G3GR_AA_ORDERED);
}

FX_EXPORT void FX_CSTYLE
grAADrawPolygon(const int nverts, const int ilist[], const GrVertex
vlist[]){
	g3grEnable(G3GR_AA_ORDERED);
	// g3grDrawPolygon(pt);
	g3grDisable(G3GR_AA_ORDERED);
	UNIMP(grAADrawPolygon);
}

FX_EXPORT void FX_CSTYLE
grAADrawPolygonVertexList(const int nverts, const GrVertex vlist[]){
    UNIMP(grAADrawPolygonVertexList);    
}

FX_EXPORT void FX_CSTYLE
grAADrawTriangle(
                 const GrVertex *a, const GrVertex *b, const GrVertex *c,
                 FxBool ab_antialias, FxBool bc_antialias, FxBool
ca_antialias
                 ){
	g3grAADrawTriangle(a, b, c, ab_antialias, bc_antialias, ca_antialias);
}

FX_EXPORT void FX_CSTYLE
grGlideInit( void ){
    if (!grGlide3Init()) {
        fprintf(stderr, "Failed to init Glide3\n");
        exit(EXIT_FAILURE);
    }
    g3grGlideInit();

}

FX_EXPORT void FX_CSTYLE
grGlideShutdown( void ){
    g3grGlideShutdown();
}

FX_EXPORT void FX_CSTYLE
grGlideGetVersion( char version[80] ){
    const char *str;

    if (g3grGetString) {
        str = g3grGetString(G3GR_VERSION);
        strncpy(version, str, 79);
    }
}

static char *g3state;
static GrState *currentG2State = NULL;

FX_EXPORT void FX_CSTYLE
grGlideGetState( GrState *state ){
    if (g3state == NULL) {
        FxU32 stateSize;
        if (g3grGet(G3GR_GLIDE_STATE_SIZE, sizeof(stateSize), &stateSize)) {
            g3state = malloc(stateSize);
        }
    }

    if (g3state == NULL) {
        fprintf(errfile, "Could not allocate GrState\n");
        exit(EXIT_FAILURE);
    }
        
    currentG2State = state;
    g3grGlideGetState(g3state);
}

FX_EXPORT void FX_CSTYLE
grGlideSetState( const GrState *state ){
    if (state == currentG2State) {
        g3grGlideSetState(g3state);
    } else {
        fprintf(errfile, "Setting to an unknown state\n");
        exit(EXIT_FAILURE);
    }
}

FX_EXPORT void FX_CSTYLE
grGlideShamelessPlug(const FxBool on){
    UNIMP(grGlideShamelessPlug);    
}

FX_EXPORT void FX_CSTYLE
grHints(GrHint_t hintType, FxU32 hintMask){
  switch (hintType) {
    case GR_HINT_STWHINT:
      g3grVertexLayout(G3GR_PARAM_Q0,  GR_VERTEX_OOW_TMU0_OFFSET<<2, (hintMask & GR_STWHINT_W_DIFF_TMU0)!= 0);
      g3grVertexLayout(G3GR_PARAM_Q1,  GR_VERTEX_OOW_TMU1_OFFSET<<2, (hintMask & GR_STWHINT_W_DIFF_TMU1)!= 0);
      break;
    case GR_HINT_FIFOCHECKHINT:
      break;
    case GR_HINT_FPUPRECISION:
      break;
    case GR_HINT_ALLOW_MIPMAP_DITHER:
      /* Regardless of the game hint, force the user selection */
      if ( hintMask )
	       g3grEnable(G3GR_ALLOW_MIPMAP_DITHER);
	  else g3grDisable(G3GR_ALLOW_MIPMAP_DITHER);
      break;
    default:
      // GR_CHECK_F( myName, 1, "invalid hints type" );
      break;
  }
}

FX_EXPORT void FX_CSTYLE
guAADrawTriangleWithClip( const GrVertex *a, const GrVertex
						 *b, const GrVertex *c){
    UNIMP(guAADrawTriangleWithClip);  
}

FX_EXPORT void FX_CSTYLE
guDrawTriangleWithClip(
                       const GrVertex *a,
                       const GrVertex *b,
                       const GrVertex *c
                       ){
//   UNIMP(guDrawTriangleWithClip);
    grDrawTriangle(a, b, c);
}

FX_EXPORT void FX_CSTYLE
guDrawPolygonVertexListWithClip( int nverts, const GrVertex vlist[] ){
    UNIMP(guDrawPolygonVertexListWithClip);    
}

FX_EXPORT void FX_CSTYLE
guAlphaSource( GrAlphaSource_t mode ){
    switch ( mode ) {
    case GR_ALPHASOURCE_CC_ALPHA:
        g3grAlphaCombine( GR_COMBINE_FUNCTION_LOCAL, 
			  GR_COMBINE_FACTOR_NONE, 
			  GR_COMBINE_LOCAL_CONSTANT, 
			  GR_COMBINE_OTHER_NONE, 
			  FXFALSE );
	break;
    case GR_ALPHASOURCE_ITERATED_ALPHA:
        g3grAlphaCombine( GR_COMBINE_FUNCTION_LOCAL, 
			  GR_COMBINE_FACTOR_NONE, 
			  GR_COMBINE_LOCAL_ITERATED, 
			  GR_COMBINE_OTHER_NONE, 
			  FXFALSE );
	break;
    case GR_ALPHASOURCE_TEXTURE_ALPHA:
        g3grAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, 
			  GR_COMBINE_FACTOR_ONE, 
			  GR_COMBINE_LOCAL_NONE, 
			  GR_COMBINE_OTHER_TEXTURE, 
			  FXFALSE );
	break;
    case GR_ALPHASOURCE_TEXTURE_ALPHA_TIMES_ITERATED_ALPHA:
        g3grAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, 
			  GR_COMBINE_FACTOR_LOCAL, 
			  GR_COMBINE_LOCAL_ITERATED, 
			  GR_COMBINE_OTHER_TEXTURE, 
			  FXFALSE );
	break;
    }
}

FX_EXPORT void FX_CSTYLE
guColorCombineFunction( GrColorCombineFnc_t fnc ){
  switch ( fnc )
  {
  case GR_COLORCOMBINE_ZERO:
    g3grColorCombine( GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_NONE, FXFALSE );
    break;

  case GR_COLORCOMBINE_CCRGB:
    g3grColorCombine( GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_NONE, FXFALSE );
    break;

  case GR_COLORCOMBINE_ITRGB_DELTA0:
//    _g3grColorCombineDelta0Mode( FXTRUE );
    /* FALL THRU */
  case GR_COLORCOMBINE_ITRGB:
    g3grColorCombine( GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE );
    break;

  case GR_COLORCOMBINE_DECAL_TEXTURE:
    g3grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
    break;

  case GR_COLORCOMBINE_TEXTURE_TIMES_CCRGB:
    g3grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL, GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
    break;

  case GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB_DELTA0:
//    _g3grColorCombineDelta0Mode( FXTRUE );
    /* FALL THRU */
  case GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB:
    g3grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
    break;

  case GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB_ADD_ALPHA:
    g3grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA, GR_COMBINE_FACTOR_LOCAL, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
    break;

  case GR_COLORCOMBINE_TEXTURE_TIMES_ALPHA:
    g3grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL_ALPHA, GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
    break;

  case GR_COLORCOMBINE_TEXTURE_TIMES_ALPHA_ADD_ITRGB:
    g3grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_LOCAL_ALPHA, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
    break;

  case GR_COLORCOMBINE_TEXTURE_ADD_ITRGB:
    g3grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_ONE, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
    break;

  case GR_COLORCOMBINE_TEXTURE_SUB_ITRGB:
    g3grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL, GR_COMBINE_FACTOR_ONE, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
    break;

  case GR_COLORCOMBINE_CCRGB_BLEND_ITRGB_ON_TEXALPHA:
    g3grColorCombine( GR_COMBINE_FUNCTION_BLEND, GR_COMBINE_FACTOR_TEXTURE_ALPHA, GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_ITERATED, FXFALSE );
    break;

  case GR_COLORCOMBINE_DIFF_SPEC_A:
    g3grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_LOCAL_ALPHA, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
    break;

  case GR_COLORCOMBINE_DIFF_SPEC_B:
    g3grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA, GR_COMBINE_FACTOR_LOCAL, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
    break;

  case GR_COLORCOMBINE_ONE:
    g3grColorCombine( GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_NONE, FXTRUE );
    break;
    
  default:
    UNIMP(color combine mode);
    break;
  }
}

FX_EXPORT int FX_CSTYLE
guEncodeRLE16( void *dst, 
               void *src, 
               FxU32 width, 
               FxU32 height ){
    UNIMP(guEncodeRLE16);
    return 1;
}

FX_EXPORT FxU16 * FX_CSTYLE
guTexCreateColorMipMap( void ){
	UNIMP(guTexCreateColorMipMap);
    return 0;
}
FX_EXPORT float FX_CSTYLE
guFogTableIndexToW( int i ){
    return g3guFogTableIndexToW(i);
}

#define kInternalFogTableEntryCount GR_FOG_TABLE_SIZE

FX_EXPORT void FX_CSTYLE
guFogGenerateExp( GrFog_t fogtable[], float density ){
  int   i;
  float f;
  float scale;
  float dp;

  GDBG_INFO(99,"guFogGenerateExp(0x%x,%g)\n",fogtable,density);
  dp = density * guFogTableIndexToW( kInternalFogTableEntryCount - 1 );
  scale = 1.0F / ( 1.0F - ( float ) exp( -dp ) );

  for ( i = 0; i < kInternalFogTableEntryCount; i++ ) {
     dp = density * guFogTableIndexToW( i );
     f = ( 1.0F - ( float ) exp( -dp ) ) * scale;

     if ( f > 1.0F )
        f = 1.0F;
     else if ( f < 0.0F )
        f = 0.0F;

     f *= 255.0F;
     fogtable[i] = ( GrFog_t ) f;
  }
}

FX_EXPORT void FX_CSTYLE
guFogGenerateExp2( GrFog_t fogtable[], float density ){
  int   i;
  float f;
  float scale;
  float dp;

  GDBG_INFO(99,"guFogGenerateExp2(0x%x,%g)\n",fogtable,density);
  dp = density * guFogTableIndexToW( kInternalFogTableEntryCount - 1 );
  scale = 1.0F / ( 1.0F - ( float ) exp( -( dp * dp ) ) );

  for ( i = 0; i < kInternalFogTableEntryCount; i++ ) {
     dp = density * guFogTableIndexToW( i );
     f = ( 1.0F - ( float ) exp( -( dp * dp ) ) ) * scale;

     if ( f > 1.0F )
        f = 1.0F;
     else if ( f < 0.0F )
        f = 0.0F;

     f *= 255.0F;
     fogtable[i] = ( GrFog_t ) f;
  }
}

FX_EXPORT void FX_CSTYLE
guFogGenerateLinear(GrFog_t fogtable[],
					float nearZ, float farZ ){
   int i;
   float world_w;
   float f;

  GDBG_INFO(99,"guFogGenerateLinear(0x%x,%g,%g)\n",fogtable,nearZ,farZ);
  for ( i = 0; i < kInternalFogTableEntryCount; i++ ) {
    world_w = guFogTableIndexToW( i );
    if ( world_w > 65535.0F )
      world_w = 65535.0F;

    f = ( world_w - nearZ ) / ( farZ - nearZ );
    if ( f > 1.0F )
      f = 1.0F;
    else if ( f < 0.0F )
      f = 0.0F;
    f *= 255.0F;
    fogtable[i] = ( GrFog_t ) f;
  }
}

FX_EXPORT FxU32 FX_CSTYLE
guEndianSwapWords( FxU32 value ){
    return (value>>16) | ((0xffff&value)<<16);
}

FX_EXPORT FxU16 FX_CSTYLE
guEndianSwapBytes( FxU16 value ){
    return (value >> 8) | ((0xff&value)<<8);
}

FX_EXPORT FxBool FX_CSTYLE
gu3dfGetInfo( const char *filename, Gu3dfInfo *info ){
    FxBool status = g3gu3dfGetInfo(filename, info);

    if (status) {
        info->header.small_lod = 8-info->header.small_lod;
        info->header.large_lod = 8-info->header.large_lod;
        info->header.aspect_ratio = -(info->header.aspect_ratio-3);
    }
    return status;
}

FX_EXPORT FxBool FX_CSTYLE
gu3dfLoad( const char *filename, Gu3dfInfo *info ){
    FxBool status;

    info->header.small_lod = 8-info->header.small_lod;
    info->header.large_lod = 8-info->header.large_lod;
    info->header.aspect_ratio = -(info->header.aspect_ratio-3);

    status = g3gu3dfLoad(filename, info);

    info->header.small_lod = 8-info->header.small_lod;
    info->header.large_lod = 8-info->header.large_lod;
    info->header.aspect_ratio = -(info->header.aspect_ratio-3);

    return status;
}

FX_EXPORT void FX_CALL 
ConvertAndDownloadRle( GrChipID_t        tmu,
                        FxU32             startAddress,
                        GrLOD_t           thisLod,
                        GrLOD_t           largeLod,
                        GrAspectRatio_t   aspectRatio,
                        GrTextureFormat_t format,
                        FxU32             evenOdd,
                        FxU8              *bm_data,
                        long              bm_h,
                        FxU32             u0,
                        FxU32             v0,
                        FxU32             width,
                        FxU32             height,
                        FxU32             dest_width,
                        FxU32             dest_height,
                        FxU16             *tlut)
{
  UNIMP(ConvertAndDownloadRle);
}  
