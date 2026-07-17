#include "glide.h"

#define NO_DEFINES

#include "GlideWrapper.h"

void glideWrapperDrawPoint(const void * pt)
{
	grDrawPoint(pt);
}

void glideWrapperDrawLine(const void * v1, const void * v2)
{
	grDrawLine(v1, v2);
}

void glideWrapperDrawTriangle(const void * a, const void * b, const void * c)
{
	grDrawTriangle(a, b, c);
}

void glideWrapperVertexLayout(FxU32 param, FxI32 offset, FxU32 mode)
{
	grVertexLayout(param, offset, mode);
}

void glideWrapperDrawVertexArray(FxU32 mode, FxU32 Count, void * pointers)
{
	grDrawVertexArray(mode, Count, pointers);
}

void glideWrapperDrawVertexArrayContiguous(FxU32 mode, FxU32 Count, void * pointers, FxU32 stride)
{
	grDrawVertexArrayContiguous(mode, Count, pointers, stride);
}

void glideWrapperAADrawTriangle(const void * a, const void * b, const void * c, FxBool ab_antialias, FxBool bc_antialias, FxBool ca_antialias)
{
	grAADrawTriangle(a, b, c, ab_antialias, bc_antialias, ca_antialias);
}

void glideWrapperBufferClear(GrColor_t color, GrAlpha_t alpha, FxU32 depth)
{
	grBufferClear(color, alpha, depth);
}

void glideWrapperBufferSwap(FxU32 swap_interval)
{
	grBufferSwap(swap_interval);
}

void glideWrapperRenderBuffer(GrBuffer_t buffer)
{
	grRenderBuffer(buffer);
}

void glideWrapperErrorSetCallback(GrErrorCallbackFnc_t fnc)
{
	grErrorSetCallback(fnc);
}

void glideWrapperFinish(void)
{
	grFinish();
}

void glideWrapperFlush(void)
{
	grFlush();
}

GrContext_t glideWrapperSstWinOpen(FxU32 hWnd, GrScreenResolution_t screen_resolution, GrScreenRefresh_t refresh_rate, GrColorFormat_t color_format, GrOriginLocation_t origin_location, int nColBuffers, int nAuxBuffers)
{
	return grSstWinOpen(hWnd, screen_resolution, refresh_rate, color_format, origin_location, nColBuffers, nAuxBuffers);
}

FxBool glideWrapperSstWinClose(GrContext_t context)
{
	return grSstWinClose(context);
}

FxBool glideWrapperSelectContext(GrContext_t context)
{
	return grSelectContext(context);
}

void glideWrapperSstOrigin(GrOriginLocation_t origin)
{
	grSstOrigin(origin);
}

void glideWrapperSstSelect(int which_sst)
{
	grSstSelect(which_sst);
}

void glideWrapperAlphaBlendFunction(GrAlphaBlendFnc_t rgb_sf, GrAlphaBlendFnc_t rgb_df, GrAlphaBlendFnc_t alpha_sf, GrAlphaBlendFnc_t alpha_df)
{
	grAlphaBlendFunction(rgb_sf, rgb_df, alpha_sf, alpha_df);
}

void glideWrapperAlphaCombine(GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert)
{
	grAlphaCombine(function, factor, local, other, invert);
}

void glideWrapperAlphaControlsITRGBLighting(FxBool enable)
{
	grAlphaControlsITRGBLighting(enable);
}

void glideWrapperAlphaTestFunction(GrCmpFnc_t function)
{
	grAlphaTestFunction(function);
}

void glideWrapperAlphaTestReferenceValue(GrAlpha_t value)
{
	grAlphaTestReferenceValue(value);
}

void glideWrapperChromakeyMode(GrChromakeyMode_t mode)
{
	grChromakeyMode(mode);
}

void glideWrapperChromakeyValue(GrColor_t value)
{
	grChromakeyValue(value);
}

void glideWrapperClipWindow(FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy)
{
	grClipWindow(minx, miny, maxx, maxy);
}

void glideWrapperColorCombine(GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert)
{
	grColorCombine(function, factor, local, other, invert);
}

void glideWrapperColorMask(FxBool rgb, FxBool a)
{
	grColorMask(rgb, a);
}

void glideWrapperCullMode(GrCullMode_t mode)
{
	grCullMode(mode);
}

void glideWrapperConstantColorValue(GrColor_t value)
{
	grConstantColorValue(value);
}

void glideWrapperDepthBiasLevel(FxI32 level)
{
	grDepthBiasLevel(level);
}

void glideWrapperDepthBufferFunction(GrCmpFnc_t function)
{
	grDepthBufferFunction(function);
}

void glideWrapperDepthBufferMode(GrDepthBufferMode_t mode)
{
	grDepthBufferMode(mode);
}

void glideWrapperDepthMask(FxBool mask)
{
	grDepthMask(mask);
}

void glideWrapperDisableAllEffects(void)
{
	grDisableAllEffects();
}

void glideWrapperDitherMode(GrDitherMode_t mode)
{
	grDitherMode(mode);
}

void glideWrapperFogColorValue(GrColor_t fogcolor)
{
	grFogColorValue(fogcolor);
}

void glideWrapperFogMode(GrFogMode_t mode)
{
	grFogMode(mode);
}

void glideWrapperFogTable(const GrFog_t ft[])
{
	grFogTable(ft);
}

void glideWrapperLoadGammaTable(FxU32 nentries, FxU32 * red, FxU32 * green, FxU32 * blue)
{
	grLoadGammaTable(nentries, red, green, blue);
}

void glideWrapperSplash(float x, float y, float width, float height, FxU32 frame)
{
	grSplash(x, y, width, height, frame);
}

FxU32 glideWrapperGet(FxU32 pname, FxU32 plength, FxI32 * params)
{
	return grGet(pname, plength, params);
}

const char * glideWrapperGetString(FxU32 pname)
{
	return grGetString(pname);
}

FxI32 glideWrapperQueryResolutions(const GrResolution * resTemplate, GrResolution * output)
{
	return grQueryResolutions(resTemplate, output);
}

FxBool glideWrapperReset(FxU32 what)
{
	return grReset(what);
}

GrProc glideWrapperGetProcAddress(char * procName)
{
	return grGetProcAddress(procName);
}

void glideWrapperEnable(GrEnableMode_t mode)
{
	grEnable(mode);
}

void glideWrapperDisable(GrEnableMode_t mode)
{
	grDisable(mode);
}

void glideWrapperCoordinateSpace(GrCoordinateSpaceMode_t mode)
{
	grCoordinateSpace(mode);
}

void glideWrapperDepthRange(FxFloat n, FxFloat f)
{
	grDepthRange(n, f);
}

void glideWrapperViewport(FxI32 x, FxI32 y, FxI32 width, FxI32 height)
{
	grViewport(x, y, width, height);
}

FxU32 glideWrapperTexCalcMemRequired(GrLOD_t lodmin, GrLOD_t lodmax, GrAspectRatio_t aspect, GrTextureFormat_t fmt)
{
	return grTexCalcMemRequired(lodmin, lodmax, aspect, fmt);
}

FxU32 glideWrapperTexTextureMemRequired(FxU32 evenOdd, GrTexInfo * info)
{
	return grTexTextureMemRequired(evenOdd, info);
}

FxU32 glideWrapperTexMinAddress(GrChipID_t tmu)
{
	return grTexMinAddress(tmu);
}

FxU32 glideWrapperTexMaxAddress(GrChipID_t tmu)
{
	return grTexMaxAddress(tmu);
}

void glideWrapperTexNCCTable(GrNCCTable_t table)
{
	grTexNCCTable(table);
}

void glideWrapperTexSource(GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo * info)
{
	grTexSource(tmu, startAddress, evenOdd, info);
}

void glideWrapperTexClampMode(GrChipID_t tmu, GrTextureClampMode_t s_clampmode, GrTextureClampMode_t t_clampmode)
{
	grTexClampMode(tmu, s_clampmode, t_clampmode);
}

void glideWrapperTexCombine(GrChipID_t tmu, GrCombineFunction_t rgb_function, GrCombineFactor_t rgb_factor, GrCombineFunction_t alpha_function, GrCombineFactor_t alpha_factor, FxBool rgb_invert, FxBool alpha_invert)
{
	grTexCombine(tmu, rgb_function, rgb_factor, alpha_function, alpha_factor, rgb_invert, alpha_invert);
}

void glideWrapperTexDetailControl(GrChipID_t tmu, int lod_bias, FxU8 detail_scale, float detail_max)
{
	grTexDetailControl(tmu, lod_bias, detail_scale, detail_max);
}

void glideWrapperTexFilterMode(GrChipID_t tmu, GrTextureFilterMode_t minfilter_mode, GrTextureFilterMode_t magfilter_mode)
{
	grTexFilterMode(tmu, minfilter_mode, magfilter_mode);
}

void glideWrapperTexLodBiasValue(GrChipID_t tmu, float bias)
{
	grTexLodBiasValue(tmu, bias);
}

void glideWrapperTexDownloadMipMap(GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo * info)
{
	grTexDownloadMipMap(tmu, startAddress, evenOdd, info);
}

void glideWrapperTexDownloadMipMapLevel(GrChipID_t tmu, FxU32 startAddress, GrLOD_t thisLod, GrLOD_t largeLod, GrAspectRatio_t aspectRatio, GrTextureFormat_t format, FxU32 evenOdd, void * data)
{
	grTexDownloadMipMapLevel(tmu, startAddress, thisLod, largeLod, aspectRatio, format, evenOdd, data);
}

FxBool glideWrapperTexDownloadMipMapLevelPartial(GrChipID_t tmu, FxU32 startAddress, GrLOD_t thisLod, GrLOD_t largeLod, GrAspectRatio_t aspectRatio, GrTextureFormat_t format, FxU32 evenOdd, void * data, int start, int end)
{
	return grTexDownloadMipMapLevelPartial(tmu, startAddress, thisLod, largeLod, aspectRatio, format, evenOdd, data, start, end);
}

void glideWrapperTexDownloadTable(GrTexTable_t type, void * data)
{
	grTexDownloadTable(type, data);
}

void glideWrapperTexDownloadTablePartial(GrTexTable_t type, void * data, int start, int end)
{
	grTexDownloadTablePartial(type, data, start, end);
}

void glideWrapperTexMipMapMode(GrChipID_t tmu, GrMipMapMode_t mode, FxBool lodBlend)
{
	grTexMipMapMode(tmu, mode, lodBlend);
}

void glideWrapperTexMultibase(GrChipID_t tmu, FxBool enable)
{
	grTexMultibase(tmu, enable);
}

void glideWrapperTexMultibaseAddress(GrChipID_t tmu, GrTexBaseRange_t range, FxU32 startAddress, FxU32 evenOdd, GrTexInfo * info)
{
	grTexMultibaseAddress(tmu, range, startAddress, evenOdd, info);
}

FxBool glideWrapperLfbLock(GrLock_t type, GrBuffer_t buffer, GrLfbWriteMode_t writeMode, GrOriginLocation_t origin, FxBool pixelPipeline, GrLfbInfo_t * info)
{
	return grLfbLock(type, buffer, writeMode, origin, pixelPipeline, info);
}

FxBool glideWrapperLfbUnlock(GrLock_t type, GrBuffer_t buffer)
{
	return grLfbUnlock(type, buffer);
}

void glideWrapperLfbConstantAlpha(GrAlpha_t alpha)
{
	grLfbConstantAlpha(alpha);
}

void glideWrapperLfbConstantDepth(FxU32 depth)
{
	grLfbConstantDepth(depth);
}

void glideWrapperLfbWriteColorSwizzle(FxBool swizzleBytes, FxBool swapWords)
{
	grLfbWriteColorSwizzle(swizzleBytes, swapWords);
}

void glideWrapperLfbWriteColorFormat(GrColorFormat_t colorFormat)
{
	grLfbWriteColorFormat(colorFormat);
}

FxBool glideWrapperLfbWriteRegion(GrBuffer_t dst_buffer, FxU32 dst_x, FxU32 dst_y, GrLfbSrcFmt_t src_format, FxU32 src_width, FxU32 src_height, FxBool pixelPipeline, FxI32 src_stride, void * src_data)
{
	return grLfbWriteRegion(dst_buffer, dst_x, dst_y, src_format, src_width, src_height, pixelPipeline, src_stride, src_data);
}

FxBool glideWrapperLfbReadRegion(GrBuffer_t src_buffer, FxU32 src_x, FxU32 src_y, FxU32 src_width, FxU32 src_height, FxU32 dst_stride, void * dst_data)
{
	return grLfbReadRegion(src_buffer, src_x, src_y, src_width, src_height, dst_stride, dst_data);
}

void glideWrapperGlideInit(void)
{
	grGlideInit();
}

void glideWrapperGlideShutdown(void)
{
	grGlideShutdown();
}

void glideWrapperGlideGetState(void * state)
{
	grGlideGetState(state);
}

void glideWrapperGlideSetState(const void * state)
{
	grGlideSetState(state);
}

void glideWrapperGlideGetVertexLayout(void * layout)
{
	grGlideGetVertexLayout(layout);
}

void glideWrapperGlideSetVertexLayout(const void * layout)
{
	grGlideSetVertexLayout(layout);
}

