/*******************************************************************
 * THIS HEADER HAS BEEN HACKED BY AN EXPERIMENTAL PERL SCRIPT
 * makegt.pl
 * on 12 Jun 98
 *
 * This allows the program to call a trapped version of
 * glide3x.dll to log calls to a binary files for later playback
 * It expects the registry to be set
 * under HKEY_CURRENT_USER/Software/3Dfx/GlideTrap
 * otherwise it creates default values in the registry
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

typedef void ( FX_CALL *grAADrawTriangle_fpt )(const void *a,
                                               const void *b,
                                               const void *c,
                                               FxBool ab_antialias,
                                               FxBool bc_antialias,
                                               FxBool ca_antialias);
void FX_CALL STUB_grAADrawTriangle(const void *a,
                                   const void *b,
                                   const void *c,
                                   FxBool ab_antialias,
                                   FxBool bc_antialias,
                                   FxBool ca_antialias);
typedef void ( FX_CALL *grAlphaBlendFunction_fpt )(GrAlphaBlendFnc_t rgb_sf,
                                                   GrAlphaBlendFnc_t rgb_df,
                                                   GrAlphaBlendFnc_t alpha_sf,
                                                   GrAlphaBlendFnc_t alpha_df);
void FX_CALL STUB_grAlphaBlendFunction(GrAlphaBlendFnc_t rgb_sf,
                                       GrAlphaBlendFnc_t rgb_df,
                                       GrAlphaBlendFnc_t alpha_sf,
                                       GrAlphaBlendFnc_t alpha_df);
typedef void ( FX_CALL *grAlphaCombine_fpt )(GrCombineFunction_t function,
                                             GrCombineFactor_t factor,
                                             GrCombineLocal_t local,
                                             GrCombineOther_t other,
                                             FxBool invert);
void FX_CALL STUB_grAlphaCombine(GrCombineFunction_t function,
                                 GrCombineFactor_t factor,
                                 GrCombineLocal_t local,
                                 GrCombineOther_t other,
                                 FxBool invert);
typedef void ( FX_CALL *grAlphaControlsITRGBLighting_fpt )(FxBool enable);
void FX_CALL STUB_grAlphaControlsITRGBLighting(FxBool enable);
typedef void ( FX_CALL *grAlphaTestFunction_fpt )(GrCmpFnc_t function);
void FX_CALL STUB_grAlphaTestFunction(GrCmpFnc_t function);
typedef void ( FX_CALL *grAlphaTestReferenceValue_fpt )(GrAlpha_t value);
void FX_CALL STUB_grAlphaTestReferenceValue(GrAlpha_t value);
typedef void ( FX_CALL *grBufferClear_fpt )(GrColor_t color,
                                            GrAlpha_t alpha,
                                            FxU32 depth);
void FX_CALL STUB_grBufferClear(GrColor_t color,
                                GrAlpha_t alpha,
                                FxU32 depth);
typedef void ( FX_CALL *grBufferSwap_fpt )(int swap_interval);
void FX_CALL STUB_grBufferSwap(int swap_interval);
typedef void ( FX_CALL *grChromakeyMode_fpt )(GrChromakeyMode_t mode);
void FX_CALL STUB_grChromakeyMode(GrChromakeyMode_t mode);
typedef void ( FX_CALL *grChromakeyValue_fpt )(GrColor_t value);
void FX_CALL STUB_grChromakeyValue(GrColor_t value);
typedef void ( FX_CALL *grClipWindow_fpt )(FxU32 minx,
                                           FxU32 miny,
                                           FxU32 maxx,
                                           FxU32 maxy);
void FX_CALL STUB_grClipWindow(FxU32 minx,
                               FxU32 miny,
                               FxU32 maxx,
                               FxU32 maxy);
typedef void ( FX_CALL *grColorCombine_fpt )(GrCombineFunction_t function,
                                             GrCombineFactor_t factor,
                                             GrCombineLocal_t local,
                                             GrCombineOther_t other,
                                             FxBool invert);
void FX_CALL STUB_grColorCombine(GrCombineFunction_t function,
                                 GrCombineFactor_t factor,
                                 GrCombineLocal_t local,
                                 GrCombineOther_t other,
                                 FxBool invert);
typedef void ( FX_CALL *grColorMask_fpt )(FxBool rgb,
                                          FxBool a);
void FX_CALL STUB_grColorMask(FxBool rgb,
                              FxBool a);
typedef void ( FX_CALL *grConstantColorValue_fpt )(GrColor_t value);
void FX_CALL STUB_grConstantColorValue(GrColor_t value);
typedef void ( FX_CALL *grCoordinateSpace_fpt )(GrCoordinateSpaceMode_t mode);
void FX_CALL STUB_grCoordinateSpace(GrCoordinateSpaceMode_t mode);
typedef void ( FX_CALL *grCullMode_fpt )(GrCullMode_t mode);
void FX_CALL STUB_grCullMode(GrCullMode_t mode);
typedef void ( FX_CALL *grDepthBiasLevel_fpt )(FxI32 level);
void FX_CALL STUB_grDepthBiasLevel(FxI32 level);
typedef void ( FX_CALL *grDepthBufferFunction_fpt )(GrCmpFnc_t function);
void FX_CALL STUB_grDepthBufferFunction(GrCmpFnc_t function);
typedef void ( FX_CALL *grDepthBufferMode_fpt )(GrDepthBufferMode_t mode);
void FX_CALL STUB_grDepthBufferMode(GrDepthBufferMode_t mode);
typedef void ( FX_CALL *grDepthMask_fpt )(FxBool mask);
void FX_CALL STUB_grDepthMask(FxBool mask);
typedef void ( FX_CALL *grDepthRange_fpt )(FxFloat n,
                                           FxFloat f);
void FX_CALL STUB_grDepthRange(FxFloat n,
                               FxFloat f);
typedef void ( FX_CALL *grDisable_fpt )(GrEnableMode_t mode);
void FX_CALL STUB_grDisable(GrEnableMode_t mode);
typedef void ( FX_CALL *grDisableAllEffects_fpt )( void);
void FX_CALL STUB_grDisableAllEffects( void);
typedef void ( FX_CALL *grDitherMode_fpt )(GrDitherMode_t mode);
void FX_CALL STUB_grDitherMode(GrDitherMode_t mode);
typedef void ( FX_CALL *grDrawLine_fpt )(const void *v1,
                                         const void *v2);
void FX_CALL STUB_grDrawLine(const void *v1,
                             const void *v2);
typedef void ( FX_CALL *grDrawPoint_fpt )(const void *pt);
void FX_CALL STUB_grDrawPoint(const void *pt);
typedef void ( FX_CALL *grDrawTriangle_fpt )(const void *a,
                                             const void *b,
                                             const void *c);
void FX_CALL STUB_grDrawTriangle(const void *a,
                                 const void *b,
                                 const void *c);
typedef void ( FX_CALL *grDrawVertexArray_fpt )(FxU32 mode,
                                                FxU32 Count,
                                                void *pointers);
void FX_CALL STUB_grDrawVertexArray(FxU32 mode,
                                    FxU32 Count,
                                    void *pointers);
typedef void ( FX_CALL *grDrawVertexArrayContiguous_fpt )(FxU32 mode,
                                                          FxU32 Count,
                                                          void *pointers,
                                                          FxU32 stride);
void FX_CALL STUB_grDrawVertexArrayContiguous(FxU32 mode,
                                              FxU32 Count,
                                              void *pointers,
                                              FxU32 stride);
typedef void ( FX_CALL *grEnable_fpt )(GrEnableMode_t mode);
void FX_CALL STUB_grEnable(GrEnableMode_t mode);
typedef void ( FX_CALL *grErrorSetCallback_fpt )(GrErrorCallbackFnc_t fnc);
void FX_CALL STUB_grErrorSetCallback(GrErrorCallbackFnc_t fnc);
typedef void ( FX_CALL *grFinish_fpt )( void);
void FX_CALL STUB_grFinish( void);
typedef void ( FX_CALL *grFlush_fpt )( void);
void FX_CALL STUB_grFlush( void);
typedef void ( FX_CALL *grFogColorValue_fpt )(GrColor_t fogcolor);
void FX_CALL STUB_grFogColorValue(GrColor_t fogcolor);
typedef void ( FX_CALL *grFogMode_fpt )(GrFogMode_t mode);
void FX_CALL STUB_grFogMode(GrFogMode_t mode);
typedef void ( FX_CALL *grFogTable_fpt )(const GrFog_t ft[]);
void FX_CALL STUB_grFogTable(const GrFog_t ft[]);
typedef FxBool ( FX_CALL *grGet_fpt )(FxU32 pname,
                                      FxU32 plength,
                                      FxI32 *params);
FxBool FX_CALL STUB_grGet(FxU32 pname,
                          FxU32 plength,
                          FxI32 *params);
typedef GrProc ( FX_CALL *grGetProcAddress_fpt )(char *procName);
GrProc FX_CALL STUB_grGetProcAddress(char *procName);
typedef const char * ( FX_CALL *grGetString_fpt )(FxU32 pname);
const char * FX_CALL STUB_grGetString(FxU32 pname);
typedef void ( FX_CALL *grGlideGetState_fpt )(void *state);
void FX_CALL STUB_grGlideGetState(void *state);
typedef void ( FX_CALL *grGlideGetVertexLayout_fpt )(void *layout);
void FX_CALL STUB_grGlideGetVertexLayout(void *layout);
typedef void ( FX_CALL *grGlideInit_fpt )( void);
void FX_CALL STUB_grGlideInit( void);
typedef void ( FX_CALL *grGlideSetState_fpt )(const void *state);
void FX_CALL STUB_grGlideSetState(const void *state);
typedef void ( FX_CALL *grGlideSetVertexLayout_fpt )(const void *layout);
void FX_CALL STUB_grGlideSetVertexLayout(const void *layout);
typedef void ( FX_CALL *grGlideShutdown_fpt )( void);
void FX_CALL STUB_grGlideShutdown( void);
typedef void ( FX_CALL *grLfbConstantAlpha_fpt )(GrAlpha_t alpha);
void FX_CALL STUB_grLfbConstantAlpha(GrAlpha_t alpha);
typedef void ( FX_CALL *grLfbConstantDepth_fpt )(FxU32 depth);
void FX_CALL STUB_grLfbConstantDepth(FxU32 depth);
typedef FxBool ( FX_CALL *grLfbLock_fpt )(GrLock_t type,
                                          GrBuffer_t buffer,
                                          GrLfbWriteMode_t writeMode,
                                          GrOriginLocation_t origin,
                                          FxBool pixelPipeline,
                                          GrLfbInfo_t *info);
FxBool FX_CALL STUB_grLfbLock(GrLock_t type,
                              GrBuffer_t buffer,
                              GrLfbWriteMode_t writeMode,
                              GrOriginLocation_t origin,
                              FxBool pixelPipeline,
                              GrLfbInfo_t *info);
typedef FxBool ( FX_CALL *grLfbReadRegion_fpt )(GrBuffer_t src_buffer,
                                                FxU32 src_x,
                                                FxU32 src_y,
                                                FxU32 src_width,
                                                FxU32 src_height,
                                                FxU32 dst_stride,
                                                void *dst_data);
FxBool FX_CALL STUB_grLfbReadRegion(GrBuffer_t src_buffer,
                                    FxU32 src_x,
                                    FxU32 src_y,
                                    FxU32 src_width,
                                    FxU32 src_height,
                                    FxU32 dst_stride,
                                    void *dst_data);
typedef FxBool ( FX_CALL *grLfbUnlock_fpt )(GrLock_t type,
                                            GrBuffer_t buffer);
FxBool FX_CALL STUB_grLfbUnlock(GrLock_t type,
                                GrBuffer_t buffer);
typedef void ( FX_CALL *grLfbWriteColorFormat_fpt )(GrColorFormat_t colorFormat);
void FX_CALL STUB_grLfbWriteColorFormat(GrColorFormat_t colorFormat);
typedef void ( FX_CALL *grLfbWriteColorSwizzle_fpt )(FxBool swizzleBytes,
                                                     FxBool swapWords);
void FX_CALL STUB_grLfbWriteColorSwizzle(FxBool swizzleBytes,
                                         FxBool swapWords);
typedef FxBool ( FX_CALL *grLfbWriteRegion_fpt )(GrBuffer_t dst_buffer,
                                                 FxU32 dst_x,
                                                 FxU32 dst_y,
                                                 GrLfbSrcFmt_t src_format,
                                                 FxU32 src_width,
                                                 FxU32 src_height,
                                                 FxBool pixelPipeline,
                                                 FxI32 src_stride,
                                                 void *src_data);
FxBool FX_CALL STUB_grLfbWriteRegion(GrBuffer_t dst_buffer,
                                     FxU32 dst_x,
                                     FxU32 dst_y,
                                     GrLfbSrcFmt_t src_format,
                                     FxU32 src_width,
                                     FxU32 src_height,
                                     FxBool pixelPipeline,
                                     FxI32 src_stride,
                                     void *src_data);
typedef void ( FX_CALL *grLoadGammaTable_fpt )(FxU32 nentries,
                                               FxU32 *red,
                                               FxU32 *green,
                                               FxU32 *blue);
void FX_CALL STUB_grLoadGammaTable(FxU32 nentries,
                                   FxU32 *red,
                                   FxU32 *green,
                                   FxU32 *blue);
typedef FxI32 ( FX_CALL *grQueryResolutions_fpt )(const GlideResolution *resTemplate,
                                                  GlideResolution *output);
FxI32 FX_CALL STUB_grQueryResolutions(const GlideResolution *resTemplate,
                                      GlideResolution *output);
typedef void ( FX_CALL *grRenderBuffer_fpt )(GrBuffer_t buffer);
void FX_CALL STUB_grRenderBuffer(GrBuffer_t buffer);
typedef FxBool ( FX_CALL *grReset_fpt )(FxU32 what);
FxBool FX_CALL STUB_grReset(FxU32 what);
typedef FxBool ( FX_CALL *grSelectContext_fpt )(GrContext_t context);
FxBool FX_CALL STUB_grSelectContext(GrContext_t context);
typedef void ( FX_CALL *grSplash_fpt )(float x,
                                       float y,
                                       float width,
                                       float height,
                                       FxU32 frame);
void FX_CALL STUB_grSplash(float x,
                           float y,
                           float width,
                           float height,
                           FxU32 frame);
typedef void ( FX_CALL *grSstOrigin_fpt )(GrOriginLocation_t origin);
void FX_CALL STUB_grSstOrigin(GrOriginLocation_t origin);
typedef void ( FX_CALL *grSstSelect_fpt )(int which_sst);
void FX_CALL STUB_grSstSelect(int which_sst);
typedef FxBool ( FX_CALL *grSstWinClose_fpt )(GrContext_t context);
FxBool FX_CALL STUB_grSstWinClose(GrContext_t context);
typedef GrContext_t ( FX_CALL *grSstWinOpen_fpt )(FxU32 hWnd,
                                                  GrScreenResolution_t screen_resolution,
                                                  GrScreenRefresh_t refresh_rate,
                                                  GrColorFormat_t color_format,
                                                  GrOriginLocation_t origin_location,
                                                  int nColBuffers,
                                                  int nAuxBuffers);
GrContext_t FX_CALL STUB_grSstWinOpen(FxU32 hWnd,
                                      GrScreenResolution_t screen_resolution,
                                      GrScreenRefresh_t refresh_rate,
                                      GrColorFormat_t color_format,
                                      GrOriginLocation_t origin_location,
                                      int nColBuffers,
                                      int nAuxBuffers);
typedef FxU32 ( FX_CALL *grTexCalcMemRequired_fpt )(GrLOD_t lodmin,
                                                    GrLOD_t lodmax,
                                                    GrAspectRatio_t aspect,
                                                    GrTextureFormat_t fmt);
FxU32 FX_CALL STUB_grTexCalcMemRequired(GrLOD_t lodmin,
                                        GrLOD_t lodmax,
                                        GrAspectRatio_t aspect,
                                        GrTextureFormat_t fmt);
typedef void ( FX_CALL *grTexClampMode_fpt )(GrChipID_t tmu,
                                             GrTextureClampMode_t s_clampmode,
                                             GrTextureClampMode_t t_clampmode);
void FX_CALL STUB_grTexClampMode(GrChipID_t tmu,
                                 GrTextureClampMode_t s_clampmode,
                                 GrTextureClampMode_t t_clampmode);
typedef void ( FX_CALL *grTexCombine_fpt )(GrChipID_t tmu,
                                           GrCombineFunction_t rgb_function,
                                           GrCombineFactor_t rgb_factor,
                                           GrCombineFunction_t alpha_function,
                                           GrCombineFactor_t alpha_factor,
                                           FxBool rgb_invert,
                                           FxBool alpha_invert);
void FX_CALL STUB_grTexCombine(GrChipID_t tmu,
                               GrCombineFunction_t rgb_function,
                               GrCombineFactor_t rgb_factor,
                               GrCombineFunction_t alpha_function,
                               GrCombineFactor_t alpha_factor,
                               FxBool rgb_invert,
                               FxBool alpha_invert);
typedef void ( FX_CALL *grTexDetailControl_fpt )(GrChipID_t tmu,
                                                 int lod_bias,
                                                 FxU8 detail_scale,
                                                 float detail_max);
void FX_CALL STUB_grTexDetailControl(GrChipID_t tmu,
                                     int lod_bias,
                                     FxU8 detail_scale,
                                     float detail_max);
typedef void ( FX_CALL *grTexDownloadMipMap_fpt )(GrChipID_t tmu,
                                                  FxU32 startAddress,
                                                  FxU32 evenOdd,
                                                  GrTexInfo *info);
void FX_CALL STUB_grTexDownloadMipMap(GrChipID_t tmu,
                                      FxU32 startAddress,
                                      FxU32 evenOdd,
                                      GrTexInfo *info);
typedef void ( FX_CALL *grTexDownloadMipMapLevel_fpt )(GrChipID_t tmu,
                                                       FxU32 startAddress,
                                                       GrLOD_t thisLod,
                                                       GrLOD_t largeLod,
                                                       GrAspectRatio_t aspectRatio,
                                                       GrTextureFormat_t format,
                                                       FxU32 evenOdd,
                                                       void *data);
void FX_CALL STUB_grTexDownloadMipMapLevel(GrChipID_t tmu,
                                           FxU32 startAddress,
                                           GrLOD_t thisLod,
                                           GrLOD_t largeLod,
                                           GrAspectRatio_t aspectRatio,
                                           GrTextureFormat_t format,
                                           FxU32 evenOdd,
                                           void *data);
typedef FxBool ( FX_CALL *grTexDownloadMipMapLevelPartial_fpt )(GrChipID_t tmu,
                                                                FxU32 startAddress,
                                                                GrLOD_t thisLod,
                                                                GrLOD_t largeLod,
                                                                GrAspectRatio_t aspectRatio,
                                                                GrTextureFormat_t format,
                                                                FxU32 evenOdd,
                                                                void *data,
                                                                int start,
                                                                int end);
FxBool FX_CALL STUB_grTexDownloadMipMapLevelPartial(GrChipID_t tmu,
                                                    FxU32 startAddress,
                                                    GrLOD_t thisLod,
                                                    GrLOD_t largeLod,
                                                    GrAspectRatio_t aspectRatio,
                                                    GrTextureFormat_t format,
                                                    FxU32 evenOdd,
                                                    void *data,
                                                    int start,
                                                    int end);
typedef void ( FX_CALL *grTexDownloadTable_fpt )(GrTexTable_t type,
                                                 void *data);
void FX_CALL STUB_grTexDownloadTable(GrTexTable_t type,
                                     void *data);
typedef void ( FX_CALL *grTexDownloadTablePartial_fpt )(GrTexTable_t type,
                                                        void *data,
                                                        int start,
                                                        int end);
void FX_CALL STUB_grTexDownloadTablePartial(GrTexTable_t type,
                                            void *data,
                                            int start,
                                            int end);
typedef void ( FX_CALL *grTexFilterMode_fpt )(GrChipID_t tmu,
                                              GrTextureFilterMode_t minfilter_mode,
                                              GrTextureFilterMode_t magfilter_mode);
void FX_CALL STUB_grTexFilterMode(GrChipID_t tmu,
                                  GrTextureFilterMode_t minfilter_mode,
                                  GrTextureFilterMode_t magfilter_mode);
typedef void ( FX_CALL *grTexLodBiasValue_fpt )(GrChipID_t tmu,
                                                float bias);
void FX_CALL STUB_grTexLodBiasValue(GrChipID_t tmu,
                                    float bias);
typedef FxU32 ( FX_CALL *grTexMaxAddress_fpt )(GrChipID_t tmu);
FxU32 FX_CALL STUB_grTexMaxAddress(GrChipID_t tmu);
typedef FxU32 ( FX_CALL *grTexMinAddress_fpt )(GrChipID_t tmu);
FxU32 FX_CALL STUB_grTexMinAddress(GrChipID_t tmu);
typedef void ( FX_CALL *grTexMipMapMode_fpt )(GrChipID_t tmu,
                                              GrMipMapMode_t mode,
                                              FxBool lodBlend);
void FX_CALL STUB_grTexMipMapMode(GrChipID_t tmu,
                                  GrMipMapMode_t mode,
                                  FxBool lodBlend);
typedef void ( FX_CALL *grTexMultibase_fpt )(GrChipID_t tmu,
                                             FxBool enable);
void FX_CALL STUB_grTexMultibase(GrChipID_t tmu,
                                 FxBool enable);
typedef void ( FX_CALL *grTexMultibaseAddress_fpt )(GrChipID_t tmu,
                                                    GrTexBaseRange_t range,
                                                    FxU32 startAddress,
                                                    FxU32 evenOdd,
                                                    GrTexInfo *info);
void FX_CALL STUB_grTexMultibaseAddress(GrChipID_t tmu,
                                        GrTexBaseRange_t range,
                                        FxU32 startAddress,
                                        FxU32 evenOdd,
                                        GrTexInfo *info);
typedef void ( FX_CALL *grTexNCCTable_fpt )(GrNCCTable_t table);
void FX_CALL STUB_grTexNCCTable(GrNCCTable_t table);
typedef void ( FX_CALL *grTexSource_fpt )(GrChipID_t tmu,
                                          FxU32 startAddress,
                                          FxU32 evenOdd,
                                          GrTexInfo *info);
void FX_CALL STUB_grTexSource(GrChipID_t tmu,
                              FxU32 startAddress,
                              FxU32 evenOdd,
                              GrTexInfo *info);
typedef FxU32 ( FX_CALL *grTexTextureMemRequired_fpt )(FxU32 evenOdd,
                                                       GrTexInfo *info);
FxU32 FX_CALL STUB_grTexTextureMemRequired(FxU32 evenOdd,
                                           GrTexInfo *info);
typedef void ( FX_CALL *grVertexLayout_fpt )(FxU32 param,
                                             FxI32 offset,
                                             FxU32 mode);
void FX_CALL STUB_grVertexLayout(FxU32 param,
                                 FxI32 offset,
                                 FxU32 mode);
typedef void ( FX_CALL *grViewport_fpt )(FxI32 x,
                                         FxI32 y,
                                         FxI32 width,
                                         FxI32 height);
void FX_CALL STUB_grViewport(FxI32 x,
                             FxI32 y,
                             FxI32 width,
                             FxI32 height);
typedef FxBool ( FX_CALL *gu3dfGetInfo_fpt )(const char *filename,
                                             Gu3dfInfo *info);
FxBool FX_CALL STUB_gu3dfGetInfo(const char *filename,
                                 Gu3dfInfo *info);
typedef FxBool ( FX_CALL *gu3dfLoad_fpt )(const char *filename,
                                          Gu3dfInfo *data);
FxBool FX_CALL STUB_gu3dfLoad(const char *filename,
                              Gu3dfInfo *data);
typedef void ( FX_CALL *guFogGenerateExp_fpt )(GrFog_t *fogtable,
                                               float density);
void FX_CALL STUB_guFogGenerateExp(GrFog_t *fogtable,
                                   float density);
typedef void ( FX_CALL *guFogGenerateExp2_fpt )(GrFog_t *fogtable,
                                                float density);
void FX_CALL STUB_guFogGenerateExp2(GrFog_t *fogtable,
                                    float density);
typedef void ( FX_CALL *guFogGenerateLinear_fpt )(GrFog_t *fogtable,
                                                  float nearZ,
                                                  float farZ);
void FX_CALL STUB_guFogGenerateLinear(GrFog_t *fogtable,
                                      float nearZ,
                                      float farZ);
typedef float ( FX_CALL *guFogTableIndexToW_fpt )(int i);
float FX_CALL STUB_guFogTableIndexToW(int i);
typedef void ( FX_CALL *guGammaCorrectionRGB_fpt )(FxFloat red,
                                                   FxFloat green,
                                                   FxFloat blue);
void FX_CALL STUB_guGammaCorrectionRGB(FxFloat red,
                                       FxFloat green,
                                       FxFloat blue);
#ifdef __cplusplus
extern "C" {
#endif
