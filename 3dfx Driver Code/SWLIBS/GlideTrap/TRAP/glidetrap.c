/*
** Glide Trap 1.0
** Hector Yee
** yhy1@cornell.edu
** yee@3dfx.com
** 6/8/98
**
** Utility to trap glide calls to a binary file
** for playback
 *
 * THIS C FILE HAS BEEN HACKED BY AN EXPERIMENTAL PERL SCRIPT
 * makegt.pl
 * on 12 Jun 98

*/

#include <stdlib.h>
/* Include 3dfx.h to prevent glide.h from
 * including it.  This way we can redefine
 * FX_ENTRY as a DLL export marker.
 */
#include <3dfx.h>

#ifdef FX_ENTRY
#undef FX_ENTRY
#endif
#define FX_ENTRY __declspec( dllexport )

#include <glide.h>
#include <windows.h>
#include "Trap.h"        /* Class to trap glide calls */
/*  contains the dynamic glide function prototypes */
#include "GlideTrap.h"

static FxBool fast;    /* whether to use fast compare or not */
static FxBool tmuset,texalignset; /* host settable # tmus or texture alignment */
static FxU32 numtmu,texalign;

extern FxBool okToRecord;

grAADrawTriangle_fpt GNX_grAADrawTriangle=STUB_grAADrawTriangle;
grAlphaBlendFunction_fpt GNX_grAlphaBlendFunction=STUB_grAlphaBlendFunction;
grAlphaCombine_fpt GNX_grAlphaCombine=STUB_grAlphaCombine;
grAlphaControlsITRGBLighting_fpt GNX_grAlphaControlsITRGBLighting=STUB_grAlphaControlsITRGBLighting;
grAlphaTestFunction_fpt GNX_grAlphaTestFunction=STUB_grAlphaTestFunction;
grAlphaTestReferenceValue_fpt GNX_grAlphaTestReferenceValue=STUB_grAlphaTestReferenceValue;
grBufferClear_fpt GNX_grBufferClear=STUB_grBufferClear;
grBufferSwap_fpt GNX_grBufferSwap=STUB_grBufferSwap;
grChromakeyMode_fpt GNX_grChromakeyMode=STUB_grChromakeyMode;
grChromakeyValue_fpt GNX_grChromakeyValue=STUB_grChromakeyValue;
grClipWindow_fpt GNX_grClipWindow=STUB_grClipWindow;
grColorCombine_fpt GNX_grColorCombine=STUB_grColorCombine;
grColorMask_fpt GNX_grColorMask=STUB_grColorMask;
grConstantColorValue_fpt GNX_grConstantColorValue=STUB_grConstantColorValue;
grCoordinateSpace_fpt GNX_grCoordinateSpace=STUB_grCoordinateSpace;
grCullMode_fpt GNX_grCullMode=STUB_grCullMode;
grDepthBiasLevel_fpt GNX_grDepthBiasLevel=STUB_grDepthBiasLevel;
grDepthBufferFunction_fpt GNX_grDepthBufferFunction=STUB_grDepthBufferFunction;
grDepthBufferMode_fpt GNX_grDepthBufferMode=STUB_grDepthBufferMode;
grDepthMask_fpt GNX_grDepthMask=STUB_grDepthMask;
grDepthRange_fpt GNX_grDepthRange=STUB_grDepthRange;
grDisable_fpt GNX_grDisable=STUB_grDisable;
grDisableAllEffects_fpt GNX_grDisableAllEffects=STUB_grDisableAllEffects;
grDitherMode_fpt GNX_grDitherMode=STUB_grDitherMode;
grDrawLine_fpt GNX_grDrawLine=STUB_grDrawLine;
grDrawPoint_fpt GNX_grDrawPoint=STUB_grDrawPoint;
grDrawTriangle_fpt GNX_grDrawTriangle=STUB_grDrawTriangle;
grDrawVertexArray_fpt GNX_grDrawVertexArray=STUB_grDrawVertexArray;
grDrawVertexArrayContiguous_fpt GNX_grDrawVertexArrayContiguous=STUB_grDrawVertexArrayContiguous;
grEnable_fpt GNX_grEnable=STUB_grEnable;
grErrorSetCallback_fpt GNX_grErrorSetCallback=STUB_grErrorSetCallback;
grFinish_fpt GNX_grFinish=STUB_grFinish;
grFlush_fpt GNX_grFlush=STUB_grFlush;
grFogColorValue_fpt GNX_grFogColorValue=STUB_grFogColorValue;
grFogMode_fpt GNX_grFogMode=STUB_grFogMode;
grFogTable_fpt GNX_grFogTable=STUB_grFogTable;
grGet_fpt GNX_grGet=STUB_grGet;
grGetProcAddress_fpt GNX_grGetProcAddress=STUB_grGetProcAddress;
grGetString_fpt GNX_grGetString=STUB_grGetString;
grGlideGetState_fpt GNX_grGlideGetState=STUB_grGlideGetState;
grGlideGetVertexLayout_fpt GNX_grGlideGetVertexLayout=STUB_grGlideGetVertexLayout;
grGlideInit_fpt GNX_grGlideInit=STUB_grGlideInit;
grGlideSetState_fpt GNX_grGlideSetState=STUB_grGlideSetState;
grGlideSetVertexLayout_fpt GNX_grGlideSetVertexLayout=STUB_grGlideSetVertexLayout;
grGlideShutdown_fpt GNX_grGlideShutdown=STUB_grGlideShutdown;
grLfbConstantAlpha_fpt GNX_grLfbConstantAlpha=STUB_grLfbConstantAlpha;
grLfbConstantDepth_fpt GNX_grLfbConstantDepth=STUB_grLfbConstantDepth;
grLfbLock_fpt GNX_grLfbLock=STUB_grLfbLock;
grLfbReadRegion_fpt GNX_grLfbReadRegion=STUB_grLfbReadRegion;
grLfbUnlock_fpt GNX_grLfbUnlock=STUB_grLfbUnlock;
grLfbWriteColorFormat_fpt GNX_grLfbWriteColorFormat=STUB_grLfbWriteColorFormat;
grLfbWriteColorSwizzle_fpt GNX_grLfbWriteColorSwizzle=STUB_grLfbWriteColorSwizzle;
grLfbWriteRegion_fpt GNX_grLfbWriteRegion=STUB_grLfbWriteRegion;
grLoadGammaTable_fpt GNX_grLoadGammaTable=STUB_grLoadGammaTable;
grQueryResolutions_fpt GNX_grQueryResolutions=STUB_grQueryResolutions;
grRenderBuffer_fpt GNX_grRenderBuffer=STUB_grRenderBuffer;
grReset_fpt GNX_grReset=STUB_grReset;
grSelectContext_fpt GNX_grSelectContext=STUB_grSelectContext;
grSplash_fpt GNX_grSplash=STUB_grSplash;
grSstOrigin_fpt GNX_grSstOrigin=STUB_grSstOrigin;
grSstSelect_fpt GNX_grSstSelect=STUB_grSstSelect;
grSstWinClose_fpt GNX_grSstWinClose=STUB_grSstWinClose;
grSstWinOpen_fpt GNX_grSstWinOpen=STUB_grSstWinOpen;
grTexCalcMemRequired_fpt GNX_grTexCalcMemRequired=STUB_grTexCalcMemRequired;
grTexClampMode_fpt GNX_grTexClampMode=STUB_grTexClampMode;
grTexCombine_fpt GNX_grTexCombine=STUB_grTexCombine;
grTexDetailControl_fpt GNX_grTexDetailControl=STUB_grTexDetailControl;
grTexDownloadMipMap_fpt GNX_grTexDownloadMipMap=STUB_grTexDownloadMipMap;
grTexDownloadMipMapLevel_fpt GNX_grTexDownloadMipMapLevel=STUB_grTexDownloadMipMapLevel;
grTexDownloadMipMapLevelPartial_fpt GNX_grTexDownloadMipMapLevelPartial=STUB_grTexDownloadMipMapLevelPartial;
grTexDownloadTable_fpt GNX_grTexDownloadTable=STUB_grTexDownloadTable;
grTexDownloadTablePartial_fpt GNX_grTexDownloadTablePartial=STUB_grTexDownloadTablePartial;
grTexFilterMode_fpt GNX_grTexFilterMode=STUB_grTexFilterMode;
grTexLodBiasValue_fpt GNX_grTexLodBiasValue=STUB_grTexLodBiasValue;
grTexMaxAddress_fpt GNX_grTexMaxAddress=STUB_grTexMaxAddress;
grTexMinAddress_fpt GNX_grTexMinAddress=STUB_grTexMinAddress;
grTexMipMapMode_fpt GNX_grTexMipMapMode=STUB_grTexMipMapMode;
grTexMultibase_fpt GNX_grTexMultibase=STUB_grTexMultibase;
grTexMultibaseAddress_fpt GNX_grTexMultibaseAddress=STUB_grTexMultibaseAddress;
grTexNCCTable_fpt GNX_grTexNCCTable=STUB_grTexNCCTable;
grTexSource_fpt GNX_grTexSource=STUB_grTexSource;
grTexTextureMemRequired_fpt GNX_grTexTextureMemRequired=STUB_grTexTextureMemRequired;
grVertexLayout_fpt GNX_grVertexLayout=STUB_grVertexLayout;
grViewport_fpt GNX_grViewport=STUB_grViewport;
gu3dfGetInfo_fpt GNX_gu3dfGetInfo=STUB_gu3dfGetInfo;
gu3dfLoad_fpt GNX_gu3dfLoad=STUB_gu3dfLoad;
guFogGenerateExp_fpt GNX_guFogGenerateExp=STUB_guFogGenerateExp;
guFogGenerateExp2_fpt GNX_guFogGenerateExp2=STUB_guFogGenerateExp2;
guFogGenerateLinear_fpt GNX_guFogGenerateLinear=STUB_guFogGenerateLinear;
guFogTableIndexToW_fpt GNX_guFogTableIndexToW=STUB_guFogTableIndexToW;
guGammaCorrectionRGB_fpt GNX_guGammaCorrectionRGB=STUB_guGammaCorrectionRGB;

__inline FxU32 MEM_ALIGN(FxU32 texAddr)
{
    FxU32 remainder;

    if (texalignset)
    {
        remainder=texAddr % texalign;
        if (remainder!=0)
        {
            texAddr+=texalign-remainder;
        }
    }    
    return texAddr;
}



// Create the DLL instance like this:
// static HINSTANCE glideDLLInst = NULL;
// glideDLLInst = LoadLibrary(glidename);
static FxBool GetProcAddresses(HINSTANCE glideDLLInst)
{
    if(!glideDLLInst) return FALSE;

    GNX_grAADrawTriangle = (grAADrawTriangle_fpt)GetProcAddress(glideDLLInst,"_grAADrawTriangle@24");
    if(! GNX_grAADrawTriangle) return FALSE;
    GNX_grAlphaBlendFunction = (grAlphaBlendFunction_fpt)GetProcAddress(glideDLLInst,"_grAlphaBlendFunction@16");
    if(! GNX_grAlphaBlendFunction) return FALSE;
    GNX_grAlphaCombine = (grAlphaCombine_fpt)GetProcAddress(glideDLLInst,"_grAlphaCombine@20");
    if(! GNX_grAlphaCombine) return FALSE;
    GNX_grAlphaControlsITRGBLighting = (grAlphaControlsITRGBLighting_fpt)GetProcAddress(glideDLLInst,"_grAlphaControlsITRGBLighting@4");
    if(! GNX_grAlphaControlsITRGBLighting) return FALSE;
    GNX_grAlphaTestFunction = (grAlphaTestFunction_fpt)GetProcAddress(glideDLLInst,"_grAlphaTestFunction@4");
    if(! GNX_grAlphaTestFunction) return FALSE;
    GNX_grAlphaTestReferenceValue = (grAlphaTestReferenceValue_fpt)GetProcAddress(glideDLLInst,"_grAlphaTestReferenceValue@4");
    if(! GNX_grAlphaTestReferenceValue) return FALSE;
    GNX_grBufferClear = (grBufferClear_fpt)GetProcAddress(glideDLLInst,"_grBufferClear@12");
    if(! GNX_grBufferClear) return FALSE;
    GNX_grBufferSwap = (grBufferSwap_fpt)GetProcAddress(glideDLLInst,"_grBufferSwap@4");
    if(! GNX_grBufferSwap) return FALSE;
    GNX_grChromakeyMode = (grChromakeyMode_fpt)GetProcAddress(glideDLLInst,"_grChromakeyMode@4");
    if(! GNX_grChromakeyMode) return FALSE;
    GNX_grChromakeyValue = (grChromakeyValue_fpt)GetProcAddress(glideDLLInst,"_grChromakeyValue@4");
    if(! GNX_grChromakeyValue) return FALSE;
    GNX_grClipWindow = (grClipWindow_fpt)GetProcAddress(glideDLLInst,"_grClipWindow@16");
    if(! GNX_grClipWindow) return FALSE;
    GNX_grColorCombine = (grColorCombine_fpt)GetProcAddress(glideDLLInst,"_grColorCombine@20");
    if(! GNX_grColorCombine) return FALSE;
    GNX_grColorMask = (grColorMask_fpt)GetProcAddress(glideDLLInst,"_grColorMask@8");
    if(! GNX_grColorMask) return FALSE;
    GNX_grConstantColorValue = (grConstantColorValue_fpt)GetProcAddress(glideDLLInst,"_grConstantColorValue@4");
    if(! GNX_grConstantColorValue) return FALSE;
    GNX_grCoordinateSpace = (grCoordinateSpace_fpt)GetProcAddress(glideDLLInst,"_grCoordinateSpace@4");
    if(! GNX_grCoordinateSpace) return FALSE;
    GNX_grCullMode = (grCullMode_fpt)GetProcAddress(glideDLLInst,"_grCullMode@4");
    if(! GNX_grCullMode) return FALSE;
    GNX_grDepthBiasLevel = (grDepthBiasLevel_fpt)GetProcAddress(glideDLLInst,"_grDepthBiasLevel@4");
    if(! GNX_grDepthBiasLevel) return FALSE;
    GNX_grDepthBufferFunction = (grDepthBufferFunction_fpt)GetProcAddress(glideDLLInst,"_grDepthBufferFunction@4");
    if(! GNX_grDepthBufferFunction) return FALSE;
    GNX_grDepthBufferMode = (grDepthBufferMode_fpt)GetProcAddress(glideDLLInst,"_grDepthBufferMode@4");
    if(! GNX_grDepthBufferMode) return FALSE;
    GNX_grDepthMask = (grDepthMask_fpt)GetProcAddress(glideDLLInst,"_grDepthMask@4");
    if(! GNX_grDepthMask) return FALSE;
    GNX_grDepthRange = (grDepthRange_fpt)GetProcAddress(glideDLLInst,"_grDepthRange@8");
    if(! GNX_grDepthRange) return FALSE;
    GNX_grDisable = (grDisable_fpt)GetProcAddress(glideDLLInst,"_grDisable@4");
    if(! GNX_grDisable) return FALSE;
    GNX_grDisableAllEffects = (grDisableAllEffects_fpt)GetProcAddress(glideDLLInst,"_grDisableAllEffects@0");
    if(! GNX_grDisableAllEffects) return FALSE;
    GNX_grDitherMode = (grDitherMode_fpt)GetProcAddress(glideDLLInst,"_grDitherMode@4");
    if(! GNX_grDitherMode) return FALSE;
    GNX_grDrawLine = (grDrawLine_fpt)GetProcAddress(glideDLLInst,"_grDrawLine@8");
    if(! GNX_grDrawLine) return FALSE;
    GNX_grDrawPoint = (grDrawPoint_fpt)GetProcAddress(glideDLLInst,"_grDrawPoint@4");
    if(! GNX_grDrawPoint) return FALSE;
    GNX_grDrawTriangle = (grDrawTriangle_fpt)GetProcAddress(glideDLLInst,"_grDrawTriangle@12");
    if(! GNX_grDrawTriangle) return FALSE;
    GNX_grDrawVertexArray = (grDrawVertexArray_fpt)GetProcAddress(glideDLLInst,"_grDrawVertexArray@12");
    if(! GNX_grDrawVertexArray) return FALSE;
    GNX_grDrawVertexArrayContiguous = (grDrawVertexArrayContiguous_fpt)GetProcAddress(glideDLLInst,"_grDrawVertexArrayContiguous@16");
    if(! GNX_grDrawVertexArrayContiguous) return FALSE;
    GNX_grEnable = (grEnable_fpt)GetProcAddress(glideDLLInst,"_grEnable@4");
    if(! GNX_grEnable) return FALSE;
    GNX_grErrorSetCallback = (grErrorSetCallback_fpt)GetProcAddress(glideDLLInst,"_grErrorSetCallback@4");
    if(! GNX_grErrorSetCallback) return FALSE;
    GNX_grFinish = (grFinish_fpt)GetProcAddress(glideDLLInst,"_grFinish@0");
    if(! GNX_grFinish) return FALSE;
    GNX_grFlush = (grFlush_fpt)GetProcAddress(glideDLLInst,"_grFlush@0");
    if(! GNX_grFlush) return FALSE;
    GNX_grFogColorValue = (grFogColorValue_fpt)GetProcAddress(glideDLLInst,"_grFogColorValue@4");
    if(! GNX_grFogColorValue) return FALSE;
    GNX_grFogMode = (grFogMode_fpt)GetProcAddress(glideDLLInst,"_grFogMode@4");
    if(! GNX_grFogMode) return FALSE;
    GNX_grFogTable = (grFogTable_fpt)GetProcAddress(glideDLLInst,"_grFogTable@4");
    if(! GNX_grFogTable) return FALSE;
    GNX_grGet = (grGet_fpt)GetProcAddress(glideDLLInst,"_grGet@12");
    if(! GNX_grGet) return FALSE;
    GNX_grGetProcAddress = (grGetProcAddress_fpt)GetProcAddress(glideDLLInst,"_grGetProcAddress@4");
    if(! GNX_grGetProcAddress) return FALSE;
    GNX_grGetString = (grGetString_fpt)GetProcAddress(glideDLLInst,"_grGetString@4");
    if(! GNX_grGetString) return FALSE;
    GNX_grGlideGetState = (grGlideGetState_fpt)GetProcAddress(glideDLLInst,"_grGlideGetState@4");
    if(! GNX_grGlideGetState) return FALSE;
    GNX_grGlideGetVertexLayout = (grGlideGetVertexLayout_fpt)GetProcAddress(glideDLLInst,"_grGlideGetVertexLayout@4");
    if(! GNX_grGlideGetVertexLayout) return FALSE;
    GNX_grGlideInit = (grGlideInit_fpt)GetProcAddress(glideDLLInst,"_grGlideInit@0");
    if(! GNX_grGlideInit) return FALSE;
    GNX_grGlideSetState = (grGlideSetState_fpt)GetProcAddress(glideDLLInst,"_grGlideSetState@4");
    if(! GNX_grGlideSetState) return FALSE;
    GNX_grGlideSetVertexLayout = (grGlideSetVertexLayout_fpt)GetProcAddress(glideDLLInst,"_grGlideSetVertexLayout@4");
    if(! GNX_grGlideSetVertexLayout) return FALSE;
    GNX_grGlideShutdown = (grGlideShutdown_fpt)GetProcAddress(glideDLLInst,"_grGlideShutdown@0");
    if(! GNX_grGlideShutdown) return FALSE;
    GNX_grLfbConstantAlpha = (grLfbConstantAlpha_fpt)GetProcAddress(glideDLLInst,"_grLfbConstantAlpha@4");
    if(! GNX_grLfbConstantAlpha) return FALSE;
    GNX_grLfbConstantDepth = (grLfbConstantDepth_fpt)GetProcAddress(glideDLLInst,"_grLfbConstantDepth@4");
    if(! GNX_grLfbConstantDepth) return FALSE;
    GNX_grLfbLock = (grLfbLock_fpt)GetProcAddress(glideDLLInst,"_grLfbLock@24");
    if(! GNX_grLfbLock) return FALSE;
    GNX_grLfbReadRegion = (grLfbReadRegion_fpt)GetProcAddress(glideDLLInst,"_grLfbReadRegion@28");
    if(! GNX_grLfbReadRegion) return FALSE;
    GNX_grLfbUnlock = (grLfbUnlock_fpt)GetProcAddress(glideDLLInst,"_grLfbUnlock@8");
    if(! GNX_grLfbUnlock) return FALSE;
    GNX_grLfbWriteColorFormat = (grLfbWriteColorFormat_fpt)GetProcAddress(glideDLLInst,"_grLfbWriteColorFormat@4");
    if(! GNX_grLfbWriteColorFormat) return FALSE;
    GNX_grLfbWriteColorSwizzle = (grLfbWriteColorSwizzle_fpt)GetProcAddress(glideDLLInst,"_grLfbWriteColorSwizzle@8");
    if(! GNX_grLfbWriteColorSwizzle) return FALSE;
    GNX_grLfbWriteRegion = (grLfbWriteRegion_fpt)GetProcAddress(glideDLLInst,"_grLfbWriteRegion@36");
    if(! GNX_grLfbWriteRegion) return FALSE;
    GNX_grLoadGammaTable = (grLoadGammaTable_fpt)GetProcAddress(glideDLLInst,"_grLoadGammaTable@16");
    if(! GNX_grLoadGammaTable) return FALSE;
    GNX_grQueryResolutions = (grQueryResolutions_fpt)GetProcAddress(glideDLLInst,"_grQueryResolutions@8");
    if(! GNX_grQueryResolutions) return FALSE;
    GNX_grRenderBuffer = (grRenderBuffer_fpt)GetProcAddress(glideDLLInst,"_grRenderBuffer@4");
    if(! GNX_grRenderBuffer) return FALSE;
    GNX_grReset = (grReset_fpt)GetProcAddress(glideDLLInst,"_grReset@4");
    if(! GNX_grReset) return FALSE;
    GNX_grSelectContext = (grSelectContext_fpt)GetProcAddress(glideDLLInst,"_grSelectContext@4");
    if(! GNX_grSelectContext) return FALSE;
    GNX_grSplash = (grSplash_fpt)GetProcAddress(glideDLLInst,"_grSplash@20");
    if(! GNX_grSplash) return FALSE;
    GNX_grSstOrigin = (grSstOrigin_fpt)GetProcAddress(glideDLLInst,"_grSstOrigin@4");
    if(! GNX_grSstOrigin) return FALSE;
    GNX_grSstSelect = (grSstSelect_fpt)GetProcAddress(glideDLLInst,"_grSstSelect@4");
    if(! GNX_grSstSelect) return FALSE;
    GNX_grSstWinClose = (grSstWinClose_fpt)GetProcAddress(glideDLLInst,"_grSstWinClose@4");
    if(! GNX_grSstWinClose) return FALSE;
    GNX_grSstWinOpen = (grSstWinOpen_fpt)GetProcAddress(glideDLLInst,"_grSstWinOpen@28");
    if(! GNX_grSstWinOpen) return FALSE;
    GNX_grTexCalcMemRequired = (grTexCalcMemRequired_fpt)GetProcAddress(glideDLLInst,"_grTexCalcMemRequired@16");
    if(! GNX_grTexCalcMemRequired) return FALSE;
    GNX_grTexClampMode = (grTexClampMode_fpt)GetProcAddress(glideDLLInst,"_grTexClampMode@12");
    if(! GNX_grTexClampMode) return FALSE;
    GNX_grTexCombine = (grTexCombine_fpt)GetProcAddress(glideDLLInst,"_grTexCombine@28");
    if(! GNX_grTexCombine) return FALSE;
    GNX_grTexDetailControl = (grTexDetailControl_fpt)GetProcAddress(glideDLLInst,"_grTexDetailControl@16");
    if(! GNX_grTexDetailControl) return FALSE;
    GNX_grTexDownloadMipMap = (grTexDownloadMipMap_fpt)GetProcAddress(glideDLLInst,"_grTexDownloadMipMap@16");
    if(! GNX_grTexDownloadMipMap) return FALSE;
    GNX_grTexDownloadMipMapLevel = (grTexDownloadMipMapLevel_fpt)GetProcAddress(glideDLLInst,"_grTexDownloadMipMapLevel@32");
    if(! GNX_grTexDownloadMipMapLevel) return FALSE;
    GNX_grTexDownloadMipMapLevelPartial = (grTexDownloadMipMapLevelPartial_fpt)GetProcAddress(glideDLLInst,"_grTexDownloadMipMapLevelPartial@40");
    if(! GNX_grTexDownloadMipMapLevelPartial) return FALSE;
    GNX_grTexDownloadTable = (grTexDownloadTable_fpt)GetProcAddress(glideDLLInst,"_grTexDownloadTable@8");
    if(! GNX_grTexDownloadTable) return FALSE;
    GNX_grTexDownloadTablePartial = (grTexDownloadTablePartial_fpt)GetProcAddress(glideDLLInst,"_grTexDownloadTablePartial@16");
    if(! GNX_grTexDownloadTablePartial) return FALSE;
    GNX_grTexFilterMode = (grTexFilterMode_fpt)GetProcAddress(glideDLLInst,"_grTexFilterMode@12");
    if(! GNX_grTexFilterMode) return FALSE;
    GNX_grTexLodBiasValue = (grTexLodBiasValue_fpt)GetProcAddress(glideDLLInst,"_grTexLodBiasValue@8");
    if(! GNX_grTexLodBiasValue) return FALSE;
    GNX_grTexMaxAddress = (grTexMaxAddress_fpt)GetProcAddress(glideDLLInst,"_grTexMaxAddress@4");
    if(! GNX_grTexMaxAddress) return FALSE;
    GNX_grTexMinAddress = (grTexMinAddress_fpt)GetProcAddress(glideDLLInst,"_grTexMinAddress@4");
    if(! GNX_grTexMinAddress) return FALSE;
    GNX_grTexMipMapMode = (grTexMipMapMode_fpt)GetProcAddress(glideDLLInst,"_grTexMipMapMode@12");
    if(! GNX_grTexMipMapMode) return FALSE;
    GNX_grTexMultibase = (grTexMultibase_fpt)GetProcAddress(glideDLLInst,"_grTexMultibase@8");
    if(! GNX_grTexMultibase) return FALSE;
    GNX_grTexMultibaseAddress = (grTexMultibaseAddress_fpt)GetProcAddress(glideDLLInst,"_grTexMultibaseAddress@20");
    if(! GNX_grTexMultibaseAddress) return FALSE;
    GNX_grTexNCCTable = (grTexNCCTable_fpt)GetProcAddress(glideDLLInst,"_grTexNCCTable@4");
    if(! GNX_grTexNCCTable) return FALSE;
    GNX_grTexSource = (grTexSource_fpt)GetProcAddress(glideDLLInst,"_grTexSource@16");
    if(! GNX_grTexSource) return FALSE;
    GNX_grTexTextureMemRequired = (grTexTextureMemRequired_fpt)GetProcAddress(glideDLLInst,"_grTexTextureMemRequired@8");
    if(! GNX_grTexTextureMemRequired) return FALSE;
    GNX_grVertexLayout = (grVertexLayout_fpt)GetProcAddress(glideDLLInst,"_grVertexLayout@12");
    if(! GNX_grVertexLayout) return FALSE;
    GNX_grViewport = (grViewport_fpt)GetProcAddress(glideDLLInst,"_grViewport@16");
    if(! GNX_grViewport) return FALSE;
    GNX_gu3dfGetInfo = (gu3dfGetInfo_fpt)GetProcAddress(glideDLLInst,"_gu3dfGetInfo@8");
    if(! GNX_gu3dfGetInfo) return FALSE;
    GNX_gu3dfLoad = (gu3dfLoad_fpt)GetProcAddress(glideDLLInst,"_gu3dfLoad@8");
    if(! GNX_gu3dfLoad) return FALSE;
    GNX_guFogGenerateExp = (guFogGenerateExp_fpt)GetProcAddress(glideDLLInst,"_guFogGenerateExp@8");
    if(! GNX_guFogGenerateExp) return FALSE;
    GNX_guFogGenerateExp2 = (guFogGenerateExp2_fpt)GetProcAddress(glideDLLInst,"_guFogGenerateExp2@8");
    if(! GNX_guFogGenerateExp2) return FALSE;
    GNX_guFogGenerateLinear = (guFogGenerateLinear_fpt)GetProcAddress(glideDLLInst,"_guFogGenerateLinear@12");
    if(! GNX_guFogGenerateLinear) return FALSE;
    GNX_guFogTableIndexToW = (guFogTableIndexToW_fpt)GetProcAddress(glideDLLInst,"_guFogTableIndexToW@4");
    if(! GNX_guFogTableIndexToW) return FALSE;
    GNX_guGammaCorrectionRGB = (guGammaCorrectionRGB_fpt)GetProcAddress(glideDLLInst,"_guGammaCorrectionRGB@12");
    if(! GNX_guGammaCorrectionRGB) return FALSE;

    return TRUE;
}











FX_ENTRY void FX_CALL grAADrawTriangle(const void *a,
                                       const void *b,
                                       const void *c,
                                       FxBool ab_antialias,
                                       FxBool bc_antialias,
                                       FxBool ca_antialias)
{
    trap_grAADrawTriangle(a, b, c, ab_antialias, bc_antialias, ca_antialias);
    okToRecord = 0;
    GNX_grAADrawTriangle(a, b, c, ab_antialias, bc_antialias, ca_antialias);
    okToRecord = 1;
}

void FX_CALL STUB_grAADrawTriangle(const void *a,
                                   const void *b,
                                   const void *c,
                                   FxBool ab_antialias,
                                   FxBool bc_antialias,
                                   FxBool ca_antialias)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grAlphaBlendFunction(GrAlphaBlendFnc_t rgb_sf,
                                           GrAlphaBlendFnc_t rgb_df,
                                           GrAlphaBlendFnc_t alpha_sf,
                                           GrAlphaBlendFnc_t alpha_df)
{
    trap_grAlphaBlendFunction(rgb_sf, rgb_df, alpha_sf, alpha_df);
    okToRecord = 0;
    GNX_grAlphaBlendFunction(rgb_sf, rgb_df, alpha_sf, alpha_df);
    okToRecord = 1;
}

void FX_CALL STUB_grAlphaBlendFunction(GrAlphaBlendFnc_t rgb_sf,
                                       GrAlphaBlendFnc_t rgb_df,
                                       GrAlphaBlendFnc_t alpha_sf,
                                       GrAlphaBlendFnc_t alpha_df)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grAlphaCombine(GrCombineFunction_t function,
                                     GrCombineFactor_t factor,
                                     GrCombineLocal_t local,
                                     GrCombineOther_t other,
                                     FxBool invert)
{
    trap_grAlphaCombine(function, factor, local, other, invert);
    okToRecord = 0;
    GNX_grAlphaCombine(function, factor, local, other, invert);
    okToRecord = 1;
}

void FX_CALL STUB_grAlphaCombine(GrCombineFunction_t function,
                                 GrCombineFactor_t factor,
                                 GrCombineLocal_t local,
                                 GrCombineOther_t other,
                                 FxBool invert)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grAlphaControlsITRGBLighting(FxBool enable)
{
    trap_grAlphaControlsITRGBLighting(enable);
    okToRecord = 0;
    GNX_grAlphaControlsITRGBLighting(enable);
    okToRecord = 1;
}

void FX_CALL STUB_grAlphaControlsITRGBLighting(FxBool enable)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grAlphaTestFunction(GrCmpFnc_t function)
{
    trap_grAlphaTestFunction(function);
    okToRecord = 0;
    GNX_grAlphaTestFunction(function);
    okToRecord = 1;
}

void FX_CALL STUB_grAlphaTestFunction(GrCmpFnc_t function)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grAlphaTestReferenceValue(GrAlpha_t value)
{
    trap_grAlphaTestReferenceValue(value);
    okToRecord = 0;
    GNX_grAlphaTestReferenceValue(value);
    okToRecord = 1;
}

void FX_CALL STUB_grAlphaTestReferenceValue(GrAlpha_t value)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grBufferClear(GrColor_t color,
                                    GrAlpha_t alpha,
                                    FxU32 depth)
{
    trap_grBufferClear(color, alpha, depth);
    okToRecord = 0;
    GNX_grBufferClear(color, alpha, depth);
    okToRecord = 1;
}

void FX_CALL STUB_grBufferClear(GrColor_t color,
                                GrAlpha_t alpha,
                                FxU32 depth)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grBufferSwap(FxU32 swap_interval)
{
    trap_grBufferSwap(swap_interval);
    okToRecord = 0;
    GNX_grBufferSwap(swap_interval);
    okToRecord = 1;
}

void FX_CALL STUB_grBufferSwap(int swap_interval)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grChromakeyMode(GrChromakeyMode_t mode)
{
    trap_grChromakeyMode(mode);
    okToRecord = 0;
    GNX_grChromakeyMode(mode);
    okToRecord = 1;
}

void FX_CALL STUB_grChromakeyMode(GrChromakeyMode_t mode)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grChromakeyValue(GrColor_t value)
{
    trap_grChromakeyValue(value);
    okToRecord = 0;
    GNX_grChromakeyValue(value);
    okToRecord = 1;
}

void FX_CALL STUB_grChromakeyValue(GrColor_t value)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grClipWindow(FxU32 minx,
                                   FxU32 miny,
                                   FxU32 maxx,
                                   FxU32 maxy)
{
    trap_grClipWindow(minx, miny, maxx, maxy);
    okToRecord = 0;
    GNX_grClipWindow(minx, miny, maxx, maxy);
    okToRecord = 1;
}

void FX_CALL STUB_grClipWindow(FxU32 minx,
                               FxU32 miny,
                               FxU32 maxx,
                               FxU32 maxy)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grColorCombine(GrCombineFunction_t function,
                                     GrCombineFactor_t factor,
                                     GrCombineLocal_t local,
                                     GrCombineOther_t other,
                                     FxBool invert)
{
    trap_grColorCombine(function, factor, local, other, invert);
    okToRecord = 0;
    GNX_grColorCombine(function, factor, local, other, invert);
    okToRecord = 1;
}

void FX_CALL STUB_grColorCombine(GrCombineFunction_t function,
                                 GrCombineFactor_t factor,
                                 GrCombineLocal_t local,
                                 GrCombineOther_t other,
                                 FxBool invert)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grColorMask(FxBool rgb,
                                  FxBool a)
{
    trap_grColorMask(rgb, a);
    okToRecord = 0;
    GNX_grColorMask(rgb, a);
    okToRecord = 1;
}

void FX_CALL STUB_grColorMask(FxBool rgb,
                              FxBool a)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grConstantColorValue(GrColor_t value)
{
    trap_grConstantColorValue(value);
    okToRecord = 0;
    GNX_grConstantColorValue(value);
    okToRecord = 1;
}

void FX_CALL STUB_grConstantColorValue(GrColor_t value)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grCoordinateSpace(GrCoordinateSpaceMode_t mode)
{
    trap_grCoordinateSpace(mode);
    okToRecord = 0;
    GNX_grCoordinateSpace(mode);
    okToRecord = 1;
}

void FX_CALL STUB_grCoordinateSpace(GrCoordinateSpaceMode_t mode)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grCullMode(GrCullMode_t mode)
{
    trap_grCullMode(mode);
    okToRecord = 0;
    GNX_grCullMode(mode);
    okToRecord = 1;
}

void FX_CALL STUB_grCullMode(GrCullMode_t mode)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grDepthBiasLevel(FxI32 level)
{
    trap_grDepthBiasLevel(level);
    okToRecord = 0;
    GNX_grDepthBiasLevel(level);
    okToRecord = 1;
}

void FX_CALL STUB_grDepthBiasLevel(FxI32 level)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grDepthBufferFunction(GrCmpFnc_t function)
{
    trap_grDepthBufferFunction(function);
    okToRecord = 0;
    GNX_grDepthBufferFunction(function);
    okToRecord = 1;
}

void FX_CALL STUB_grDepthBufferFunction(GrCmpFnc_t function)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grDepthBufferMode(GrDepthBufferMode_t mode)
{
    trap_grDepthBufferMode(mode);
    okToRecord = 0;
    GNX_grDepthBufferMode(mode);
    okToRecord = 1;
}

void FX_CALL STUB_grDepthBufferMode(GrDepthBufferMode_t mode)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grDepthMask(FxBool mask)
{
    trap_grDepthMask(mask);
    okToRecord = 0;
    GNX_grDepthMask(mask);
    okToRecord = 1;
}

void FX_CALL STUB_grDepthMask(FxBool mask)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grDepthRange(FxFloat n,
                                   FxFloat f)
{
    trap_grDepthRange(n, f);
    okToRecord = 0;
    GNX_grDepthRange(n, f);
    okToRecord = 1;
}

void FX_CALL STUB_grDepthRange(FxFloat n,
                               FxFloat f)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grDisable(GrEnableMode_t mode)
{
    trap_grDisable(mode);
    okToRecord = 0;
    GNX_grDisable(mode);
    okToRecord = 1;
}

void FX_CALL STUB_grDisable(GrEnableMode_t mode)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grDisableAllEffects( void)
{
    trap_grDisableAllEffects();
    okToRecord = 0;
    GNX_grDisableAllEffects();
    okToRecord = 1;
}

void FX_CALL STUB_grDisableAllEffects( void)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grDitherMode(GrDitherMode_t mode)
{
    trap_grDitherMode(mode);
    okToRecord = 0;
    GNX_grDitherMode(mode);
    okToRecord = 1;
}

void FX_CALL STUB_grDitherMode(GrDitherMode_t mode)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grDrawLine(const void *v1,
                                 const void *v2)
{
    trap_grDrawLine(v1, v2);
    okToRecord = 0;
    GNX_grDrawLine(v1, v2);
    okToRecord = 1;
}

void FX_CALL STUB_grDrawLine(const void *v1,
                             const void *v2)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grDrawPoint(const void *pt)
{
    trap_grDrawPoint(pt);
    okToRecord = 0;
    GNX_grDrawPoint(pt);
    okToRecord = 1;
}

void FX_CALL STUB_grDrawPoint(const void *pt)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grDrawTriangle(const void *a,
                                     const void *b,
                                     const void *c)
{
    trap_grDrawTriangle(a, b, c);
    okToRecord = 0;
    GNX_grDrawTriangle(a, b, c);
    okToRecord = 1;
}

void FX_CALL STUB_grDrawTriangle(const void *a,
                                 const void *b,
                                 const void *c)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grDrawVertexArray(FxU32 mode,
                                        FxU32 Count,
                                        void *pointers)
{
    trap_grDrawVertexArray(mode, Count, pointers);
    okToRecord = 0;
    GNX_grDrawVertexArray(mode, Count, pointers);
    okToRecord = 1;
}

void FX_CALL STUB_grDrawVertexArray(FxU32 mode,
                                    FxU32 Count,
                                    void *pointers)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grDrawVertexArrayContiguous(FxU32 mode,
                                                  FxU32 Count,
                                                  void *pointers,
                                                  FxU32 stride)
{
    trap_grDrawVertexArrayContiguous(mode, Count, pointers, stride);
    okToRecord = 0;
    GNX_grDrawVertexArrayContiguous(mode, Count, pointers, stride);
    okToRecord = 1;
}

void FX_CALL STUB_grDrawVertexArrayContiguous(FxU32 mode,
                                              FxU32 Count,
                                              void *pointers,
                                              FxU32 stride)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grEnable(GrEnableMode_t mode)
{
    trap_grEnable(mode);
    okToRecord = 0;
    GNX_grEnable(mode);
    okToRecord = 1;
}

void FX_CALL STUB_grEnable(GrEnableMode_t mode)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grErrorSetCallback(GrErrorCallbackFnc_t fnc)
{
    trap_grErrorSetCallback(fnc);
    okToRecord = 0;
    GNX_grErrorSetCallback(fnc);
    okToRecord = 1;
}

void FX_CALL STUB_grErrorSetCallback(GrErrorCallbackFnc_t fnc)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grFinish( void)
{
    trap_grFinish();
    okToRecord = 0;
    GNX_grFinish();
    okToRecord = 1;
}

void FX_CALL STUB_grFinish( void)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grFlush( void)
{
    trap_grFlush();
    okToRecord = 0;
    GNX_grFlush();
    okToRecord = 1;
}

void FX_CALL STUB_grFlush( void)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grFogColorValue(GrColor_t fogcolor)
{
    trap_grFogColorValue(fogcolor);
    okToRecord = 0;
    GNX_grFogColorValue(fogcolor);
    okToRecord = 1;
}

void FX_CALL STUB_grFogColorValue(GrColor_t fogcolor)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grFogMode(GrFogMode_t mode)
{
    trap_grFogMode(mode);
    okToRecord = 0;
    GNX_grFogMode(mode);
    okToRecord = 1;
}

void FX_CALL STUB_grFogMode(GrFogMode_t mode)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grFogTable(const GrFog_t ft[])
{
    FxU32 size;
    grGet(GR_FOG_TABLE_ENTRIES,4,&size);
    trap_grFogTable(ft,size);
    okToRecord = 0;
    GNX_grFogTable(ft);
    okToRecord = 1;
}

void FX_CALL STUB_grFogTable(const GrFog_t ft[])
{
    /* This function is stubbed out. */

}

FX_ENTRY FxU32 FX_CALL grGet(FxU32 pname,
                             FxU32 plength,
                             FxI32 *params)
{
    FxU32 result;
    trap_grGet(pname, plength, params);

    okToRecord = 0;
    result=GNX_grGet(pname, plength, params);
    okToRecord = 1;

    if ((tmuset) && (pname==GR_NUM_TMU))            *params=numtmu;
    if ((texalignset) && (pname==GR_TEXTURE_ALIGN)) *params=texalign;

    return result;
}

FxBool FX_CALL STUB_grGet(FxU32 pname,
                          FxU32 plength,
                          FxI32 *params)
{
    return (FxBool) 0;/* This function is stubbed out. */

}

FX_ENTRY GrProc FX_CALL grGetProcAddress(char *procName)
{
    GrProc proc;

    okToRecord = 0;
    proc=GNX_grGetProcAddress(procName);
    okToRecord = 1;
    return trap_grGetProcAddress(procName, proc);    
}

GrProc FX_CALL STUB_grGetProcAddress(char *procName)
{
    return (GrProc) 0;/* This function is stubbed out. */

}

FX_ENTRY const char * FX_CALL grGetString(FxU32 pname)
{
    const char *ret_val;
    trap_grGetString(pname);
    okToRecord = 0;
    ret_val = GNX_grGetString(pname);
    okToRecord = 1;
    if (pname == GR_EXTENSION)
      ret_val = "";
    return ret_val;
}

const char * FX_CALL STUB_grGetString(FxU32 pname)
{
    return NULL;/* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grGlideGetState(void *state)
{
    FxU32 length;                

    okToRecord = 0;
    GNX_grGlideGetState(state);
    okToRecord = 1;

    okToRecord = 0;
    GNX_grGet(GR_GLIDE_STATE_SIZE,4,&length);    
    okToRecord = 1;
    trap_grGlideGetState(state,length);    
}

void FX_CALL STUB_grGlideGetState(void *state)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grGlideGetVertexLayout(void *vlayout)
{
    FxU32 length;

    okToRecord = 0;
    GNX_grGlideGetVertexLayout(vlayout);
    okToRecord = 1;

    okToRecord = 0;
    GNX_grGet(GR_GLIDE_VERTEXLAYOUT_SIZE,4,&length);    
    okToRecord = 1;
    trap_grGlideGetVertexLayout(vlayout,length);    
}

void FX_CALL STUB_grGlideGetVertexLayout(void *layout)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grGlideInit( void)
{
    trap_grGlideInit();
    okToRecord = 0;
    GNX_grGlideInit();
    okToRecord = 1;
}

void FX_CALL STUB_grGlideInit( void)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grGlideSetState(const void *state)
{
    FxU32 length;                
    
    okToRecord = 0;
    GNX_grGet(GR_GLIDE_STATE_SIZE,4,&length);
    okToRecord = 1;

    trap_grGlideSetState(state,length);
    okToRecord = 0;
    GNX_grGlideSetState(state);
    okToRecord = 1;
}

void FX_CALL STUB_grGlideSetState(const void *state)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grGlideSetVertexLayout(const void *vlayout)
{
    FxU32 length;                
    
    okToRecord = 0;
    GNX_grGet(GR_GLIDE_VERTEXLAYOUT_SIZE,4,&length);
    okToRecord = 1;

    trap_grGlideSetVertexLayout(vlayout,length);
    okToRecord = 0;
    GNX_grGlideSetVertexLayout(vlayout);
    okToRecord = 1;
}

void FX_CALL STUB_grGlideSetVertexLayout(const void *layout)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grGlideShutdown( void)
{
    trap_grGlideShutdown();
    okToRecord = 0;
    GNX_grGlideShutdown();
    okToRecord = 1;
}

void FX_CALL STUB_grGlideShutdown( void)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grLfbConstantAlpha(GrAlpha_t alpha)
{
    trap_grLfbConstantAlpha(alpha);
    okToRecord = 0;
    GNX_grLfbConstantAlpha(alpha);
    okToRecord = 1;
}

void FX_CALL STUB_grLfbConstantAlpha(GrAlpha_t alpha)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grLfbConstantDepth(FxU32 depth)
{
    trap_grLfbConstantDepth(depth);
    okToRecord = 0;
    GNX_grLfbConstantDepth(depth);
    okToRecord = 1;
}

void FX_CALL STUB_grLfbConstantDepth(FxU32 depth)
{
    /* This function is stubbed out. */

}

FX_ENTRY FxBool FX_CALL grLfbLock(GrLock_t type,
                                  GrBuffer_t buffer,
                                  GrLfbWriteMode_t writeMode,
                                  GrOriginLocation_t origin,
                                  FxBool pixelPipeline,
                                  GrLfbInfo_t *info)
{
    FxBool result;
    
    okToRecord = 0;
    result=GNX_grLfbLock(type, buffer, writeMode, origin, pixelPipeline, info);
    okToRecord = 1;
    trap_grLfbLock(type, buffer, writeMode, origin, pixelPipeline,result);

    return result;
}

FxBool FX_CALL STUB_grLfbLock(GrLock_t type,
                              GrBuffer_t buffer,
                              GrLfbWriteMode_t writeMode,
                              GrOriginLocation_t origin,
                              FxBool pixelPipeline,
                              GrLfbInfo_t *info)
{
    return (FxBool) 0;/* This function is stubbed out. */

}

FX_ENTRY FxBool FX_CALL grLfbReadRegion(GrBuffer_t src_buffer,
                                        FxU32 src_x,
                                        FxU32 src_y,
                                        FxU32 src_width,
                                        FxU32 src_height,
                                        FxU32 dst_stride,
                                        void *dst_data)
{
    FxBool ret_val;
    trap_grLfbReadRegion(src_buffer, src_x, src_y, src_width, src_height, dst_stride, dst_data);
    okToRecord = 0;
    ret_val = GNX_grLfbReadRegion(src_buffer, src_x, src_y, src_width, src_height, dst_stride, dst_data);
    okToRecord = 1;
    return ret_val;
}

FxBool FX_CALL STUB_grLfbReadRegion(GrBuffer_t src_buffer,
                                    FxU32 src_x,
                                    FxU32 src_y,
                                    FxU32 src_width,
                                    FxU32 src_height,
                                    FxU32 dst_stride,
                                    void *dst_data)
{
    return (FxBool) 0;/* This function is stubbed out. */

}

FX_ENTRY FxBool FX_CALL grLfbUnlock(GrLock_t type,
                                    GrBuffer_t buffer)
{
    FxBool ret_val;
    trap_grLfbUnlock(type, buffer);
    okToRecord = 0;
    ret_val = GNX_grLfbUnlock(type, buffer);
    okToRecord = 1;
    return ret_val;
}

FxBool FX_CALL STUB_grLfbUnlock(GrLock_t type,
                                GrBuffer_t buffer)
{
    return (FxBool) 0;/* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grLfbWriteColorFormat(GrColorFormat_t colorFormat)
{
    trap_grLfbWriteColorFormat(colorFormat);
    okToRecord = 0;
    GNX_grLfbWriteColorFormat(colorFormat);
    okToRecord = 1;
}

void FX_CALL STUB_grLfbWriteColorFormat(GrColorFormat_t colorFormat)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grLfbWriteColorSwizzle(FxBool swizzleBytes,
                                             FxBool swapWords)
{
    trap_grLfbWriteColorSwizzle(swizzleBytes, swapWords);
    okToRecord = 0;
    GNX_grLfbWriteColorSwizzle(swizzleBytes, swapWords);
    okToRecord = 1;
}

void FX_CALL STUB_grLfbWriteColorSwizzle(FxBool swizzleBytes,
                                         FxBool swapWords)
{
    /* This function is stubbed out. */

}

FX_ENTRY FxBool FX_CALL grLfbWriteRegion(GrBuffer_t dst_buffer,
                                         FxU32 dst_x,
                                         FxU32 dst_y,
                                         GrLfbSrcFmt_t src_format,
                                         FxU32 src_width,
                                         FxU32 src_height,
                                         FxBool pixelPipeline,
                                         FxI32 src_stride,
                                         void *src_data)
{
    FxBool ret_val;
    trap_grLfbWriteRegion(dst_buffer, dst_x, dst_y, src_format, src_width, src_height, pixelPipeline, src_stride, src_data);
    okToRecord = 0;
    ret_val = GNX_grLfbWriteRegion(dst_buffer, dst_x, dst_y, src_format, src_width, src_height, pixelPipeline, src_stride, src_data);
    okToRecord = 1;
    return ret_val;
}

FxBool FX_CALL STUB_grLfbWriteRegion(GrBuffer_t dst_buffer,
                                     FxU32 dst_x,
                                     FxU32 dst_y,
                                     GrLfbSrcFmt_t src_format,
                                     FxU32 src_width,
                                     FxU32 src_height,
                                     FxBool pixelPipeline,
                                     FxI32 src_stride,
                                     void *src_data)
{
    return (FxBool) 0;/* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grLoadGammaTable(FxU32 nentries,
                                       FxU32 *red,
                                       FxU32 *green,
                                       FxU32 *blue)
{
    trap_grLoadGammaTable(nentries, red, green, blue);
    okToRecord = 0;
    GNX_grLoadGammaTable(nentries, red, green, blue);
    okToRecord = 1;
}

void FX_CALL STUB_grLoadGammaTable(FxU32 nentries,
                                   FxU32 *red,
                                   FxU32 *green,
                                   FxU32 *blue)
{
    /* This function is stubbed out. */

}

FX_ENTRY FxI32 FX_CALL grQueryResolutions(const GlideResolution *resTemplate,
                                          GlideResolution *output)
{
    FxI32 result;

    okToRecord = 0;
    result=GNX_grQueryResolutions(resTemplate, output);
    okToRecord = 1;
    trap_grQueryResolutions(resTemplate, output,result);

    return result;
}

FxI32 FX_CALL STUB_grQueryResolutions(const GlideResolution *resTemplate,
                                      GlideResolution *output)
{
    return (FxI32) 0;/* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grRenderBuffer(GrBuffer_t buffer)
{
    trap_grRenderBuffer(buffer);
    okToRecord = 0;
    GNX_grRenderBuffer(buffer);
    okToRecord = 1;
}

void FX_CALL STUB_grRenderBuffer(GrBuffer_t buffer)
{
    /* This function is stubbed out. */

}

FX_ENTRY FxBool FX_CALL grReset(FxU32 what)
{
    FxBool ret_val;
    trap_grReset(what);
    okToRecord = 0;
    ret_val = GNX_grReset(what);
    okToRecord = 1;
    return ret_val;
}

FxBool FX_CALL STUB_grReset(FxU32 what)
{
    return (FxBool) 0;/* This function is stubbed out. */

}

FX_ENTRY FxBool FX_CALL grSelectContext(GrContext_t context)
{
    FxBool ret_val;
    trap_grSelectContext(context);
    okToRecord = 0;
    ret_val = GNX_grSelectContext(context);
    okToRecord = 1;
    return ret_val;
}

FxBool FX_CALL STUB_grSelectContext(GrContext_t context)
{
    return (FxBool) 0;/* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grSplash(float x,
                               float y,
                               float width,
                               float height,
                               FxU32 frame)
{
    trap_grSplash(x, y, width, height, frame);
    okToRecord = 0;
    GNX_grSplash(x, y, width, height, frame);
    okToRecord = 1;
}

void FX_CALL STUB_grSplash(float x,
                           float y,
                           float width,
                           float height,
                           FxU32 frame)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grSstOrigin(GrOriginLocation_t origin)
{
    trap_grSstOrigin(origin);
    okToRecord = 0;
    GNX_grSstOrigin(origin);
    okToRecord = 1;
}

void FX_CALL STUB_grSstOrigin(GrOriginLocation_t origin)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grSstSelect(int which_sst)
{
    trap_grSstSelect(which_sst);
    okToRecord = 0;
    GNX_grSstSelect(which_sst);
    okToRecord = 1;
}

void FX_CALL STUB_grSstSelect(int which_sst)
{
    /* This function is stubbed out. */

}

FX_ENTRY FxBool FX_CALL grSstWinClose(GrContext_t context)
{
    FxBool ret_val;
    trap_grSstWinClose(context);
    okToRecord = 0;
    ret_val = GNX_grSstWinClose(context);
    okToRecord = 1;
    return ret_val;
}

FxBool FX_CALL STUB_grSstWinClose(GrContext_t context)
{
    return (FxBool) 0;/* This function is stubbed out. */

}

FX_ENTRY GrContext_t FX_CALL grSstWinOpen(FxU32 hWnd,
                                          GrScreenResolution_t screen_resolution,
                                          GrScreenRefresh_t refresh_rate,
                                          GrColorFormat_t color_format,
                                          GrOriginLocation_t origin_location,
                                          int nColBuffers,
                                          int nAuxBuffers)
{
    GrContext_t ret_val;
    trap_grSstWinOpen(hWnd, screen_resolution, refresh_rate, color_format, origin_location, nColBuffers, nAuxBuffers);
    okToRecord = 0;
    ret_val = GNX_grSstWinOpen(hWnd, screen_resolution, refresh_rate, color_format, origin_location, nColBuffers, nAuxBuffers);
    okToRecord = 1;
    return ret_val;
}

GrContext_t FX_CALL STUB_grSstWinOpen(FxU32 hWnd,
                                      GrScreenResolution_t screen_resolution,
                                      GrScreenRefresh_t refresh_rate,
                                      GrColorFormat_t color_format,
                                      GrOriginLocation_t origin_location,
                                      int nColBuffers,
                                      int nAuxBuffers)
{
    return (GrContext_t) 0;/* This function is stubbed out. */

}

FX_ENTRY FxU32 FX_CALL grTexCalcMemRequired(GrLOD_t lodmin,
                                            GrLOD_t lodmax,
                                            GrAspectRatio_t aspect,
                                            GrTextureFormat_t fmt)
{
    FxU32 mem;
    trap_grTexCalcMemRequired(lodmin, lodmax, aspect, fmt);
    okToRecord = 0;
    mem=GNX_grTexCalcMemRequired(lodmin, lodmax, aspect, fmt);
    okToRecord = 1;
    return MEM_ALIGN(mem);    
}

FxU32 FX_CALL STUB_grTexCalcMemRequired(GrLOD_t lodmin,
                                        GrLOD_t lodmax,
                                        GrAspectRatio_t aspect,
                                        GrTextureFormat_t fmt)
{
    return (FxU32) 0;/* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grTexClampMode(GrChipID_t tmu,
                                     GrTextureClampMode_t s_clampmode,
                                     GrTextureClampMode_t t_clampmode)
{
    trap_grTexClampMode(tmu, s_clampmode, t_clampmode);
    okToRecord = 0;
    GNX_grTexClampMode(tmu, s_clampmode, t_clampmode);
    okToRecord = 1;
}

void FX_CALL STUB_grTexClampMode(GrChipID_t tmu,
                                 GrTextureClampMode_t s_clampmode,
                                 GrTextureClampMode_t t_clampmode)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grTexCombine(GrChipID_t tmu,
                                   GrCombineFunction_t rgb_function,
                                   GrCombineFactor_t rgb_factor,
                                   GrCombineFunction_t alpha_function,
                                   GrCombineFactor_t alpha_factor,
                                   FxBool rgb_invert,
                                   FxBool alpha_invert)
{
    trap_grTexCombine(tmu, rgb_function, rgb_factor, alpha_function, alpha_factor, rgb_invert, alpha_invert);
    okToRecord = 0;
    GNX_grTexCombine(tmu, rgb_function, rgb_factor, alpha_function, alpha_factor, rgb_invert, alpha_invert);
    okToRecord = 1;
}

void FX_CALL STUB_grTexCombine(GrChipID_t tmu,
                               GrCombineFunction_t rgb_function,
                               GrCombineFactor_t rgb_factor,
                               GrCombineFunction_t alpha_function,
                               GrCombineFactor_t alpha_factor,
                               FxBool rgb_invert,
                               FxBool alpha_invert)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grTexDetailControl(GrChipID_t tmu,
                                         int lod_bias,
                                         FxU8 detail_scale,
                                         float detail_max)
{
    trap_grTexDetailControl(tmu, lod_bias, detail_scale, detail_max);
    okToRecord = 0;
    GNX_grTexDetailControl(tmu, lod_bias, detail_scale, detail_max);
    okToRecord = 1;
}

void FX_CALL STUB_grTexDetailControl(GrChipID_t tmu,
                                     int lod_bias,
                                     FxU8 detail_scale,
                                     float detail_max)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grTexDownloadMipMap(GrChipID_t tmu,
                                          FxU32 startAddress,
                                          FxU32 evenOdd,
                                          GrTexInfo *info)
{
    FxU32 size;
    
    okToRecord = 0;
    size=GNX_grTexTextureMemRequired(evenOdd,info);
    okToRecord = 1;
    trap_grTexDownloadMipMap(tmu, startAddress, evenOdd, info,size);
    okToRecord = 0;
    GNX_grTexDownloadMipMap(tmu, startAddress, evenOdd, info);
    okToRecord = 1;
}

void FX_CALL STUB_grTexDownloadMipMap(GrChipID_t tmu,
                                      FxU32 startAddress,
                                      FxU32 evenOdd,
                                      GrTexInfo *info)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grTexDownloadMipMapLevel(GrChipID_t tmu,
                                               FxU32 startAddress,
                                               GrLOD_t thisLod,
                                               GrLOD_t largeLod,
                                               GrAspectRatio_t aspectRatio,
                                               GrTextureFormat_t format,
                                               FxU32 evenOdd,
                                               void *data)
{
    FxU32 size;

    okToRecord = 0;
    size=GNX_grTexCalcMemRequired(thisLod,thisLod,aspectRatio,format);
    okToRecord = 1;
    trap_grTexDownloadMipMapLevel(tmu, startAddress, thisLod, largeLod, aspectRatio, format, evenOdd, data,size);
    okToRecord = 0;
    GNX_grTexDownloadMipMapLevel(tmu, startAddress, thisLod, largeLod, aspectRatio, format, evenOdd, data);
    okToRecord = 1;
}

void FX_CALL STUB_grTexDownloadMipMapLevel(GrChipID_t tmu,
                                           FxU32 startAddress,
                                           GrLOD_t thisLod,
                                           GrLOD_t largeLod,
                                           GrAspectRatio_t aspectRatio,
                                           GrTextureFormat_t format,
                                           FxU32 evenOdd,
                                           void *data)
{
    /* This function is stubbed out. */

}

FX_ENTRY FxBool FX_CALL grTexDownloadMipMapLevelPartial(GrChipID_t tmu,
                                                        FxU32 startAddress,
                                                        GrLOD_t thisLod,
                                                        GrLOD_t largeLod,
                                                        GrAspectRatio_t aspectRatio,
                                                        GrTextureFormat_t format,
                                                        FxU32 evenOdd,
                                                        void *data,
                                                        int start,
                                                        int end)
{
    FxU32 size;
    FxBool ret_val;

    size=GetMipMapSize(aspectRatio,thisLod,format);
    size=size*(end-start+1);
    trap_grTexDownloadMipMapLevelPartial(tmu, startAddress, thisLod, largeLod, aspectRatio, format, evenOdd, data, start, end,size);
    okToRecord = 0;
    ret_val = GNX_grTexDownloadMipMapLevelPartial(tmu, startAddress, thisLod, largeLod, aspectRatio, format, evenOdd, data, start, end);
    okToRecord = 1;
    return ret_val;
}

FxBool FX_CALL STUB_grTexDownloadMipMapLevelPartial(GrChipID_t tmu,
                                                    FxU32 startAddress,
                                                    GrLOD_t thisLod,
                                                    GrLOD_t largeLod,
                                                    GrAspectRatio_t aspectRatio,
                                                    GrTextureFormat_t format,
                                                    FxU32 evenOdd,
                                                    void *data,
                                                    int start,
                                                    int end)
{
    return (FxBool) 0;/* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grTexDownloadTable(GrTexTable_t type,
                                         void *data)
{
    trap_grTexDownloadTable(type, data);
    okToRecord = 0;
    GNX_grTexDownloadTable(type, data);
    okToRecord = 1;
}

void FX_CALL STUB_grTexDownloadTable(GrTexTable_t type,
                                     void *data)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grTexDownloadTablePartial(GrTexTable_t type,
                                                void *data,
                                                int start,
                                                int end)
{
    trap_grTexDownloadTablePartial(type, data, start, end);
    okToRecord = 0;
    GNX_grTexDownloadTablePartial(type, data, start, end);
    okToRecord = 1;
}

void FX_CALL STUB_grTexDownloadTablePartial(GrTexTable_t type,
                                            void *data,
                                            int start,
                                            int end)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grTexFilterMode(GrChipID_t tmu,
                                      GrTextureFilterMode_t minfilter_mode,
                                      GrTextureFilterMode_t magfilter_mode)
{
    trap_grTexFilterMode(tmu, minfilter_mode, magfilter_mode);
    okToRecord = 0;
    GNX_grTexFilterMode(tmu, minfilter_mode, magfilter_mode);
    okToRecord = 1;
}

void FX_CALL STUB_grTexFilterMode(GrChipID_t tmu,
                                  GrTextureFilterMode_t minfilter_mode,
                                  GrTextureFilterMode_t magfilter_mode)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grTexLodBiasValue(GrChipID_t tmu,
                                        float bias)
{
    trap_grTexLodBiasValue(tmu, bias);
    okToRecord = 0;
    GNX_grTexLodBiasValue(tmu, bias);
    okToRecord = 1;
}

void FX_CALL STUB_grTexLodBiasValue(GrChipID_t tmu,
                                    float bias)
{
    /* This function is stubbed out. */

}

FX_ENTRY FxU32 FX_CALL grTexMaxAddress(GrChipID_t tmu)
{
    FxU32 ret_val;
    trap_grTexMaxAddress(tmu);
    okToRecord = 0;
    ret_val = GNX_grTexMaxAddress(tmu);
    okToRecord = 1;
    return ret_val;
}

FxU32 FX_CALL STUB_grTexMaxAddress(GrChipID_t tmu)
{
    return (FxU32) 0;/* This function is stubbed out. */

}

FX_ENTRY FxU32 FX_CALL grTexMinAddress(GrChipID_t tmu)
{
    FxU32 ret_val;
    trap_grTexMinAddress(tmu);
    okToRecord = 0;
    ret_val = GNX_grTexMinAddress(tmu);
    okToRecord = 1;
    return ret_val;
}

FxU32 FX_CALL STUB_grTexMinAddress(GrChipID_t tmu)
{
    return (FxU32) 0;/* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grTexMipMapMode(GrChipID_t tmu,
                                      GrMipMapMode_t mode,
                                      FxBool lodBlend)
{
    trap_grTexMipMapMode(tmu, mode, lodBlend);
    okToRecord = 0;
    GNX_grTexMipMapMode(tmu, mode, lodBlend);
    okToRecord = 1;
}

void FX_CALL STUB_grTexMipMapMode(GrChipID_t tmu,
                                  GrMipMapMode_t mode,
                                  FxBool lodBlend)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grTexMultibase(GrChipID_t tmu,
                                     FxBool enable)
{
    trap_grTexMultibase(tmu, enable);
    okToRecord = 0;
    GNX_grTexMultibase(tmu, enable);
    okToRecord = 1;
}

void FX_CALL STUB_grTexMultibase(GrChipID_t tmu,
                                 FxBool enable)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grTexMultibaseAddress(GrChipID_t tmu,
                                            GrTexBaseRange_t range,
                                            FxU32 startAddress,
                                            FxU32 evenOdd,
                                            GrTexInfo *info)
{
    trap_grTexMultibaseAddress(tmu, range, startAddress, evenOdd, info);
    okToRecord = 0;
    GNX_grTexMultibaseAddress(tmu, range, startAddress, evenOdd, info);
    okToRecord = 1;
}

void FX_CALL STUB_grTexMultibaseAddress(GrChipID_t tmu,
                                        GrTexBaseRange_t range,
                                        FxU32 startAddress,
                                        FxU32 evenOdd,
                                        GrTexInfo *info)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grTexNCCTable(GrNCCTable_t table)
{
    trap_grTexNCCTable(table);
    okToRecord = 0;
    GNX_grTexNCCTable(table);
    okToRecord = 1;
}

void FX_CALL STUB_grTexNCCTable(GrNCCTable_t table)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grTexSource(GrChipID_t tmu,
                                  FxU32 startAddress,
                                  FxU32 evenOdd,
                                  GrTexInfo *info)
{
    
    trap_grTexSource(tmu, startAddress, evenOdd, info);
    okToRecord = 0;
    GNX_grTexSource(tmu, startAddress, evenOdd, info);
    okToRecord = 1;
}

void FX_CALL STUB_grTexSource(GrChipID_t tmu,
                              FxU32 startAddress,
                              FxU32 evenOdd,
                              GrTexInfo *info)
{
    /* This function is stubbed out. */

}

FX_ENTRY FxU32 FX_CALL grTexTextureMemRequired(FxU32 evenOdd,
                                               GrTexInfo *info)
{
    FxU32 mem;

    trap_grTexTextureMemRequired(evenOdd, info);
    okToRecord = 0;
    mem = GNX_grTexTextureMemRequired(evenOdd, info);
    okToRecord = 1;

    return MEM_ALIGN(mem);
}

FxU32 FX_CALL STUB_grTexTextureMemRequired(FxU32 evenOdd,
                                           GrTexInfo *info)
{
    return (FxU32) 0;/* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grVertexLayout(FxU32 param,
                                     FxI32 offset,
                                     FxU32 mode)
{
    trap_grVertexLayout(param, offset, mode);
    okToRecord = 0;
    GNX_grVertexLayout(param, offset, mode);
    okToRecord = 1;
}

void FX_CALL STUB_grVertexLayout(FxU32 param,
                                 FxI32 offset,
                                 FxU32 mode)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL grViewport(FxI32 x,
                                 FxI32 y,
                                 FxI32 width,
                                 FxI32 height)
{
    trap_grViewport(x, y, width, height);
    okToRecord = 0;
    GNX_grViewport(x, y, width, height);
    okToRecord = 1;
}

void FX_CALL STUB_grViewport(FxI32 x,
                             FxI32 y,
                             FxI32 width,
                             FxI32 height)
{
    /* This function is stubbed out. */

}

FX_ENTRY FxBool FX_CALL gu3dfGetInfo(const char *filename,
                                     Gu3dfInfo *info)
{
    FxBool ret_val;
    trap_gu3dfGetInfo(filename, info);
    okToRecord = 0;
    ret_val = GNX_gu3dfGetInfo(filename, info);
    okToRecord = 1;
    return ret_val;
}

FxBool FX_CALL STUB_gu3dfGetInfo(const char *filename,
                                 Gu3dfInfo *info)
{
    return (FxBool) 0;/* This function is stubbed out. */

}

FX_ENTRY FxBool FX_CALL gu3dfLoad(const char *filename,
                                  Gu3dfInfo *data)
{
    FxBool ret_val;
    trap_gu3dfLoad(filename, data);
    okToRecord = 0;
    ret_val = GNX_gu3dfLoad(filename, data);
    okToRecord = 1;
    return ret_val;
}

FxBool FX_CALL STUB_gu3dfLoad(const char *filename,
                              Gu3dfInfo *data)
{
    return (FxBool) 0;/* This function is stubbed out. */

}

FX_ENTRY void FX_CALL guFogGenerateExp(GrFog_t *fogtable,
                                       float density)
{
    trap_guFogGenerateExp(fogtable, density);
    okToRecord = 0;
    GNX_guFogGenerateExp(fogtable, density);
    okToRecord = 1;
}

void FX_CALL STUB_guFogGenerateExp(GrFog_t *fogtable,
                                   float density)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL guFogGenerateExp2(GrFog_t *fogtable,
                                        float density)
{
    trap_guFogGenerateExp2(fogtable, density);
    okToRecord = 0;
    GNX_guFogGenerateExp2(fogtable, density);
    okToRecord = 1;
}

void FX_CALL STUB_guFogGenerateExp2(GrFog_t *fogtable,
                                    float density)
{
    /* This function is stubbed out. */

}

FX_ENTRY void FX_CALL guFogGenerateLinear(GrFog_t *fogtable,
                                          float nearZ,
                                          float farZ)
{
    trap_guFogGenerateLinear(fogtable, nearZ, farZ);
    okToRecord = 0;
    GNX_guFogGenerateLinear(fogtable, nearZ, farZ);
    okToRecord = 1;
}

void FX_CALL STUB_guFogGenerateLinear(GrFog_t *fogtable,
                                      float nearZ,
                                      float farZ)
{
    /* This function is stubbed out. */

}

FX_ENTRY float FX_CALL guFogTableIndexToW(int i)
{
    float ret_val;
    trap_guFogTableIndexToW(i);
    okToRecord = 0;
    ret_val = GNX_guFogTableIndexToW(i);
    okToRecord = 1;
    return ret_val;
}

float FX_CALL STUB_guFogTableIndexToW(int i)
{
    return (float) 0;/* This function is stubbed out. */

}

FX_ENTRY void FX_CALL guGammaCorrectionRGB(FxFloat red,
                                           FxFloat green,
                                           FxFloat blue)
{
    trap_guGammaCorrectionRGB(red, green, blue);
    okToRecord = 0;
    GNX_guGammaCorrectionRGB(red, green, blue);
    okToRecord = 1;
}

void FX_CALL STUB_guGammaCorrectionRGB(FxFloat red,
                                       FxFloat green,
                                       FxFloat blue)
{
    /* This function is stubbed out. */

}
BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  ul_reason_for_call, 
                      LPVOID lpReserved)
{
    char *dll;
    char *trapdir;
    char trapfile[128];
    char msg[5000];
    char tmsg[500];
    static FxBool loaded=FXFALSE;
    static HINSTANCE glideDLLInst = NULL;    
    char *tmp;
    
    switch( ul_reason_for_call ) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
        if(!loaded) {
            WIN32_FIND_DATA findData;

            /* get trap dir */
            tmp=getenv("TRAP_DIR");
            if (tmp==NULL)
            {
                MessageBox(0,"Environment variable TRAP_DIR not set!\n","Error",MB_OK | MB_ICONSTOP);
                exit(1);
            }

            trapdir = (char*)malloc(strlen(tmp)+2);
            strcpy(trapdir, tmp);

            if ((trapdir[strlen(trapdir)-1] == '\\') ||
                (trapdir[strlen(trapdir)-1] == '/'))
                trapdir[strlen(trapdir)-1] = '\0';

            if ((FindFirstFile(trapdir, &findData) == INVALID_HANDLE_VALUE)||
                !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                MessageBox(0,"TRAP_DIR doesn't point to an existing directory!\n","Error",MB_OK | MB_ICONSTOP);
                exit(1);
            }

            sprintf(msg, "TRAP_DIR is \"%s\"\n", trapdir);

            strcat(trapdir, "/");
            strcpy(trapfile, trapdir);
            strcat(trapfile, TOKENF);
            if (FindFirstFile(trapfile, &findData) != INVALID_HANDLE_VALUE) {
                MessageBox(0,"TRAP_DIR has an existing GlideTrap recording in it!\n","Error",MB_OK | MB_ICONSTOP);
                exit(1);
            }

            /* get glide path */
            tmp=getenv("TRAP_GLIDE");
            if (tmp==NULL)
            {
                MessageBox(0,"Environment variable TRAP_GLIDE not set!","Error",MB_OK | MB_ICONSTOP);
                exit(1);
            }
            dll=strdup(tmp);

            /* get echo status */
            tmp=getenv("TRAP_ECHO");
            if (tmp!=NULL)
            {
                if (atoi(tmp)==1)
                {
                    SetScreenEcho(FXTRUE);
                    strcat(msg,"Screen Echo is ON\n");
                } else
                {
                    SetScreenEcho(FXFALSE);
                    strcat(msg,"Screen Echo is OFF\n");
                }
            } else
            {
                SetScreenEcho(FXFALSE);
                strcat(msg,"Screen Echo is OFF\n");
            }
            

            /* get fast compare */
            tmp=getenv("TRAP_FASTCOMP");
            if (tmp!=NULL)
            {
                if (atoi(tmp)==1)
                {
                    fast=FXTRUE;                
                    strcat(msg,"Fastcompare ON\n");
                } else
                {
                    fast=FXFALSE;
                    strcat(msg,"Fastcompare OFF\n");
                }
            } else
            {
                fast=FXFALSE;
                strcat(msg,"Fastcompare OFF\n");
            }            

            SetFastComp(fast);

            /* get number of tmus */
            tmp=getenv("TRAP_NUM_TMU");
            if (tmp!=NULL)
            {
                tmuset=FXTRUE;
                numtmu=atoi(tmp);                
                sprintf(tmsg,"NUM TMUs= %u\n",numtmu);
                strcat(msg,tmsg);
            } else
            {
                tmuset=FXFALSE;                
            }

            /* get texture alignment */
            tmp=getenv("TRAP_TEXALIGN");
            if (tmp!=NULL)
            {
                texalignset=FXTRUE;
                texalign=atoi(tmp);
                sprintf(tmsg,"Texture Alignment= %u bytes\n",texalign);
                strcat(msg,tmsg);
            } else
            {
                texalignset=FXFALSE;                
            }
            
            /* init trap functions */
            CTrapInit(trapdir);            

            glideDLLInst = LoadLibrary(dll);                    
            if(GetProcAddresses(glideDLLInst)) {
                // We loaded ok, so let the user know which DLL
                // we used.  This is just sanity checking
                strcat(msg,dll);
                if (getenv("TRAP_SETTINGS"))
                  MessageBox(0,msg,"Glide Trap Settings",MB_OK);
                loaded=FXTRUE;                    
            } else {
                strcat(dll," not found! Passthru mode disabled.");
                MessageBox(0,dll,"Glide Trap",MB_OK);
            }
            free(trapdir);
            free(dll);
        }
        break;
    case DLL_THREAD_DETACH:        
    case DLL_PROCESS_DETACH:        
        /* russp, 4/19/98
         *
         * I really don't understand this, but if you leave in any
         * of the following 3 lines of code, HalfLife won't record.
         * Something weird is definitely going on here.  Removing
         * these guys doesn't appear to affect the correct running
         * of GlideTrap though.  So ... they get whacked.
         *
         *   CTrapDone();
         *   if(loaded)FreeLibrary(glideDLLInst);        
         *   loaded = FXFALSE;
         */
    default:
        ;
    }
    return TRUE;
}
