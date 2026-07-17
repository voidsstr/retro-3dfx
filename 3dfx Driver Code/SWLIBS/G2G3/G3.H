typedef  void (FX_CALL *G3grDrawPointPTR)( const void *pt );
typedef void (FX_CALL *G3grDrawLinePTR)( const void *v1, const void *v2 );
typedef void (FX_CALL *G3grDrawTrianglePTR)( const void *a, const void *b, const void *c );
typedef void (FX_CALL *G3grVertexLayoutPTR)(FxU32 param, FxI32 offset, FxU32 mode);
typedef void (FX_CALL *G3grDrawVertexArrayPTR)(FxU32 mode, FxU32 Count, void *pointers);
typedef void (FX_CALL *G3grDrawVertexArrayContiguousPTR)(FxU32 mode, FxU32 Count, void *pointers, FxU32 stride);
typedef void (FX_CALL *G3grAADrawTrianglePTR)(
                 const void *a, const void *b, const void *c,
                 FxBool ab_antialias, FxBool bc_antialias, FxBool ca_antialias
                 );
typedef void (FX_CALL *G3grBufferClearPTR)( G3GrColor_t color, G3GrAlpha_t alpha, FxU32 depth );
typedef void (FX_CALL *G3grBufferSwapPTR)( int swap_interval );
typedef void (FX_CALL *G3grRenderBufferPTR)( G3GrBuffer_t buffer );
typedef void (*G3GrErrorCallbackFnc_t)( const char *string, FxBool fatal );
typedef void (FX_CALL *G3grErrorSetCallbackPTR)( G3GrErrorCallbackFnc_t fnc );
typedef void (FX_CALL *G3grFinishPTR)(void);
typedef void (FX_CALL *G3grFlushPTR)(void);
typedef G3GrContext_t (FX_CALL *G3grSstWinOpenPTR)(
          FxU32                hWnd,
          GrScreenResolution_t screen_resolution,
          GrScreenRefresh_t    refresh_rate,
          GrColorFormat_t      color_format,
          GrOriginLocation_t   origin_location,
          int                  nColBuffers,
          int                  nAuxBuffers);
typedef FxBool (FX_CALL *G3grSstWinClosePTR)( G3GrContext_t context );
typedef FxBool (FX_CALL *G3grSelectContextPTR)( G3GrContext_t context );
typedef void (FX_CALL *G3grSstOriginPTR)(G3GrOriginLocation_t  origin);
typedef void (FX_CALL *G3grSstSelectPTR)( int which_sst );
typedef void (FX_CALL *G3grAlphaBlendFunctionPTR)(
                     G3GrAlphaBlendFnc_t rgb_sf,   G3GrAlphaBlendFnc_t rgb_df,
                     G3GrAlphaBlendFnc_t alpha_sf, G3GrAlphaBlendFnc_t alpha_df
                     );
typedef void (FX_CALL *G3grAlphaCombinePTR)(
               G3GrCombineFunction_t function, G3GrCombineFactor_t factor,
               G3GrCombineLocal_t local, G3GrCombineOther_t other,
               FxBool invert
               );
typedef void (FX_CALL *G3grAlphaControlsITRGBLightingPTR)( FxBool enable );
typedef void (FX_CALL *G3grAlphaTestFunctionPTR)( G3GrCmpFnc_t function );
typedef void (FX_CALL *G3grAlphaTestReferenceValuePTR)( G3GrAlpha_t value );
typedef void (FX_CALL *G3grChromakeyModePTR)( G3GrChromakeyMode_t mode );
typedef void (FX_CALL *G3grChromakeyValuePTR)( G3GrColor_t value );
typedef void (FX_CALL *G3grClipWindowPTR)( FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy );
typedef void (FX_CALL *G3grColorCombinePTR)(
               G3GrCombineFunction_t function, G3GrCombineFactor_t factor,
               G3GrCombineLocal_t local, G3GrCombineOther_t other,
               FxBool invert );
typedef void (FX_CALL *G3grColorMaskPTR)( FxBool rgb, FxBool a );
typedef void (FX_CALL *G3grCullModePTR)( G3GrCullMode_t mode );
typedef void (FX_CALL *G3grConstantColorValuePTR)( G3GrColor_t value );
typedef void (FX_CALL *G3grDepthBiasLevelPTR)( FxU32 level );
typedef void (FX_CALL *G3grDepthBufferFunctionPTR)( G3GrCmpFnc_t function );
typedef void (FX_CALL *G3grDepthBufferModePTR)( G3GrDepthBufferMode_t mode );
typedef void (FX_CALL *G3grDepthMaskPTR)( FxBool mask );
typedef void (FX_CALL *G3grDisableAllEffectsPTR)( void );
typedef void (FX_CALL *G3grDitherModePTR)( G3GrDitherMode_t mode );
typedef void (FX_CALL *G3grFogColorValuePTR)( G3GrColor_t fogcolor );
typedef void (FX_CALL *G3grFogModePTR)( G3GrFogMode_t mode );
typedef void (FX_CALL *G3grFogTablePTR)( const G3GrFog_t ft[] );
typedef void (FX_CALL *G3grLoadGammaTablePTR)( FxU32 nentries, FxU32 *red, FxU32 *green, FxU32 *blue);
typedef void (FX_CALL *G3grSplashPTR)(float x, float y, float width, float height, FxU32 frame);
typedef FxBool (FX_CALL *G3grGetPTR)( FxU32 pname, FxU32 plength, FxI32 *params );
typedef const char * (FX_CALL *G3grGetStringPTR)( FxU32 pname );
typedef FxI32 (FX_CALL *G3grQueryResolutionsPTR)( const G3GrResolution *resTemplate, G3GrResolution *output );
typedef FxBool (FX_CALL *G3grResetPTR)( FxU32 what );
typedef G3GrProc (FX_CALL *G3grGetProcAddressPTR)( char *procName );
typedef void (FX_CALL *G3grEnablePTR)( G3GrEnableMode_t mode );
typedef void (FX_CALL *G3grDisablePTR)( G3GrEnableMode_t mode );
typedef void (FX_CALL *G3grCoordinateSpacePTR)( G3GrCoordinateSpaceMode_t mode );
typedef void (FX_CALL *G3grDepthRangePTR)( FxFloat n, FxFloat f );
typedef void (FX_CALL *G3grViewportPTR)( FxI32 x, FxI32 y, FxI32 width, FxI32 height );
typedef FxU32 (FX_CALL *G3grTexCalcMemRequiredPTR)(
                     G3GrLOD_t lodmin, G3GrLOD_t lodmax,
                     G3GrAspectRatio_t aspect, G3GrTextureFormat_t fmt);
typedef FxU32 (FX_CALL *G3grTexTextureMemRequiredPTR)( FxU32     evenOdd,
                                 G3GrTexInfo *info   );
typedef FxU32 (FX_CALL *G3grTexMinAddressPTR)( G3GrChipID_t tmu );
typedef FxU32 (FX_CALL *G3grTexMaxAddressPTR)( G3GrChipID_t tmu );
typedef void (FX_CALL *G3grTexNCCTablePTR)( G3GrNCCTable_t table );
typedef void (FX_CALL *G3grTexSourcePTR)( G3GrChipID_t tmu,
             FxU32      startAddress,
             FxU32      evenOdd,
             G3GrTexInfo  *info );
typedef void (FX_CALL *G3grTexClampModePTR)(
               G3GrChipID_t tmu,
               G3GrTextureClampMode_t s_clampmode,
               G3GrTextureClampMode_t t_clampmode
               );
typedef void (FX_CALL *G3grTexCombinePTR)(
             G3GrChipID_t tmu,
             G3GrCombineFunction_t rgb_function,
             G3GrCombineFactor_t rgb_factor, 
             G3GrCombineFunction_t alpha_function,
             G3GrCombineFactor_t alpha_factor,
             FxBool rgb_invert,
             FxBool alpha_invert
             );
typedef void (FX_CALL *G3grTexDetailControlPTR)(
                   G3GrChipID_t tmu,
                   int lod_bias,
                   FxU8 detail_scale,
                   float detail_max
                   );
typedef void (FX_CALL *G3grTexFilterModePTR)(
                G3GrChipID_t tmu,
                G3GrTextureFilterMode_t minfilter_mode,
                G3GrTextureFilterMode_t magfilter_mode
                );
typedef void (FX_CALL *G3grTexLodBiasValuePTR)(G3GrChipID_t tmu, float bias );
typedef void (FX_CALL *G3grTexDownloadMipMapPTR)( G3GrChipID_t tmu,
                     FxU32      startAddress,
                     FxU32      evenOdd,
                     G3GrTexInfo  *info );
typedef void (FX_CALL *G3grTexDownloadMipMapLevelPTR)( G3GrChipID_t        tmu,
                          FxU32             startAddress,
                          G3GrLOD_t           thisLod,
                          G3GrLOD_t           largeLod,
                          G3GrAspectRatio_t   aspectRatio,
                          G3GrTextureFormat_t format,
                          FxU32             evenOdd,
                          void              *data );
typedef FxBool (FX_CALL *G3grTexDownloadMipMapLevelPartialPTR)( G3GrChipID_t        tmu,
                                 FxU32             startAddress,
                                 G3GrLOD_t           thisLod,
                                 G3GrLOD_t           largeLod,
                                 G3GrAspectRatio_t   aspectRatio,
                                 G3GrTextureFormat_t format,
                                 FxU32             evenOdd,
                                 void              *data,
                                 int               start,
                                 int               end );
typedef void (FX_CALL *G3grTexDownloadTablePTR)( G3GrTexTable_t type, 
                    void         *data );
typedef void (FX_CALL *G3grTexDownloadTablePartialPTR)( G3GrTexTable_t type, 
                           void         *data,
                           int          start,
                           int          end );
typedef void (FX_CALL *G3grTexMipMapModePTR)( G3GrChipID_t     tmu, 
                 G3GrMipMapMode_t mode,
                 FxBool         lodBlend );
typedef void (FX_CALL *G3grTexMultibasePTR)( G3GrChipID_t tmu,
                FxBool     enable );
typedef void (FX_CALL *G3grTexMultibaseAddressPTR)( G3GrChipID_t       tmu,
                       G3GrTexBaseRange_t range,
                       FxU32            startAddress,
                       FxU32            evenOdd,
                       G3GrTexInfo        *info );
typedef FxBool (FX_CALL *G3grLfbLockPTR)( G3GrLock_t type, G3GrBuffer_t buffer, G3GrLfbWriteMode_t writeMode,
           G3GrOriginLocation_t origin, FxBool pixelPipeline, 
           G3GrLfbInfo_t *info );
typedef FxBool (FX_CALL *G3grLfbUnlockPTR)( G3GrLock_t type, G3GrBuffer_t buffer );
typedef void (FX_CALL *G3grLfbConstantAlphaPTR)( G3GrAlpha_t alpha );
typedef void (FX_CALL *G3grLfbConstantDepthPTR)( FxU32 depth );
typedef void (FX_CALL *G3grLfbWriteColorSwizzlePTR)(FxBool swizzleBytes, FxBool swapWords);
typedef void (FX_CALL *G3grLfbWriteColorFormatPTR)(G3GrColorFormat_t colorFormat);
typedef FxBool (FX_CALL *G3grLfbWriteRegionPTR)( G3GrBuffer_t dst_buffer, 
                  FxU32 dst_x, FxU32 dst_y, 
                  G3GrLfbSrcFmt_t src_format, 
                  FxU32 src_width, FxU32 src_height, 
                  FxBool pixelPipeline,
                  FxI32 src_stride, void *src_data );
typedef FxBool (FX_CALL *G3grLfbReadRegionPTR)( G3GrBuffer_t src_buffer,
                 FxU32 src_x, FxU32 src_y,
                 FxU32 src_width, FxU32 src_height,
                 FxU32 dst_stride, void *dst_data );
typedef void (FX_CALL *G3grGlideInitPTR)( void );
typedef void (FX_CALL *G3grGlideShutdownPTR)( void );
typedef void (FX_CALL *G3grGlideGetStatePTR)( void *state );
typedef void (FX_CALL *G3grGlideSetStatePTR)( const void *state );
typedef void (FX_CALL *G3grGlideGetVertexLayoutPTR)( void *layout );
typedef void (FX_CALL *G3grGlideSetVertexLayoutPTR)( const void *layout );
typedef FxBool (FX_CALL *G3gu3dfGetInfoPTR)( const char *filename, 
                                             Gu3dfInfo *info);
typedef FxBool (FX_CALL *G3gu3dfLoadPTR)( const char *filename, Gu3dfInfo *data);
typedef float (FX_CALL *G3guFogTableIndexToWPTR)( int i);
typedef void (FX_CALL *G3guFogGenerateExpPTR)( GrFog_t fogtable[], float density);
typedef void (FX_CALL *G3guFogGenerateExp2PTR)( GrFog_t fogtable[], float density);
typedef void (FX_CALL *G3guFogGenerateLinearPTR)( GrFog_t fogtable[], float nearZ, float farZ);

G3grDrawPointPTR g3grDrawPoint;
G3grDrawLinePTR g3grDrawLine;
G3grDrawTrianglePTR g3grDrawTriangle;
G3grVertexLayoutPTR g3grVertexLayout;
G3grDrawVertexArrayPTR g3grDrawVertexArray;
G3grDrawVertexArrayContiguousPTR g3grDrawVertexArrayContiguous;
G3grAADrawTrianglePTR g3grAADrawTriangle;
G3grBufferClearPTR g3grBufferClear;
G3grBufferSwapPTR g3grBufferSwap;
G3grRenderBufferPTR g3grRenderBuffer;
G3grErrorSetCallbackPTR g3grErrorSetCallback;
G3grFinishPTR g3grFinish;
G3grFlushPTR g3grFlush;
G3grSstWinOpenPTR g3grSstWinOpen;
G3grSstWinClosePTR g3grSstWinClose;
G3grSelectContextPTR g3grSelectContext;
G3grSstOriginPTR g3grSstOrigin;
G3grSstSelectPTR g3grSstSelect;
G3grAlphaBlendFunctionPTR g3grAlphaBlendFunction;
G3grAlphaCombinePTR g3grAlphaCombine;
G3grAlphaControlsITRGBLightingPTR g3grAlphaControlsITRGBLighting;
G3grAlphaTestFunctionPTR g3grAlphaTestFunction;
G3grAlphaTestReferenceValuePTR g3grAlphaTestReferenceValue;
G3grChromakeyModePTR g3grChromakeyMode;
G3grChromakeyValuePTR g3grChromakeyValue;
G3grClipWindowPTR g3grClipWindow;
G3grColorCombinePTR g3grColorCombine;
G3grColorMaskPTR g3grColorMask;
G3grCullModePTR g3grCullMode;
G3grConstantColorValuePTR g3grConstantColorValue;
G3grDepthBiasLevelPTR g3grDepthBiasLevel;
G3grDepthBufferFunctionPTR g3grDepthBufferFunction;
G3grDepthBufferModePTR g3grDepthBufferMode;
G3grDepthMaskPTR g3grDepthMask;
G3grDisableAllEffectsPTR g3grDisableAllEffects;
G3grDitherModePTR g3grDitherMode;
G3grFogColorValuePTR g3grFogColorValue;
G3grFogModePTR g3grFogMode;
G3grFogTablePTR g3grFogTable;
G3grLoadGammaTablePTR g3grLoadGammaTable;
G3grSplashPTR g3grSplash;
G3grGetPTR g3grGet;
G3grGetStringPTR g3grGetString;
G3grQueryResolutionsPTR g3grQueryResolutions;
G3grResetPTR g3grReset;
G3grGetProcAddressPTR g3grGetProcAddress;
G3grEnablePTR g3grEnable;
G3grDisablePTR g3grDisable;
G3grCoordinateSpacePTR g3grCoordinateSpace;
G3grDepthRangePTR g3grDepthRange;
G3grViewportPTR g3grViewport;
G3grTexCalcMemRequiredPTR g3grTexCalcMemRequired;
G3grTexTextureMemRequiredPTR g3grTexTextureMemRequired;
G3grTexMinAddressPTR g3grTexMinAddress;
G3grTexMaxAddressPTR g3grTexMaxAddress;
G3grTexNCCTablePTR g3grTexNCCTable;
G3grTexSourcePTR g3grTexSource;
G3grTexClampModePTR g3grTexClampMode;
G3grTexCombinePTR g3grTexCombine;
G3grTexDetailControlPTR g3grTexDetailControl;
G3grTexFilterModePTR g3grTexFilterMode;
G3grTexLodBiasValuePTR g3grTexLodBiasValue;
G3grTexDownloadMipMapPTR g3grTexDownloadMipMap;
G3grTexDownloadMipMapLevelPTR g3grTexDownloadMipMapLevel;
G3grTexDownloadMipMapLevelPartialPTR g3grTexDownloadMipMapLevelPartial;
G3grTexDownloadTablePTR g3grTexDownloadTable;
G3grTexDownloadTablePartialPTR g3grTexDownloadTablePartial;
G3grTexMipMapModePTR g3grTexMipMapMode;
G3grTexMultibasePTR g3grTexMultibase;
G3grTexMultibaseAddressPTR g3grTexMultibaseAddress;
G3grLfbLockPTR g3grLfbLock;
G3grLfbUnlockPTR g3grLfbUnlock;
G3grLfbConstantAlphaPTR g3grLfbConstantAlpha;
G3grLfbConstantDepthPTR g3grLfbConstantDepth;
G3grLfbWriteColorSwizzlePTR g3grLfbWriteColorSwizzle;
G3grLfbWriteColorFormatPTR g3grLfbWriteColorFormat;
G3grLfbWriteRegionPTR g3grLfbWriteRegion;
G3grLfbReadRegionPTR g3grLfbReadRegion;
G3grGlideInitPTR g3grGlideInit;
G3grGlideShutdownPTR g3grGlideShutdown;
G3grGlideGetStatePTR g3grGlideGetState;
G3grGlideSetStatePTR g3grGlideSetState;
G3grGlideGetVertexLayoutPTR g3grGlideGetVertexLayout;
G3grGlideSetVertexLayoutPTR g3grGlideSetVertexLayout;
G3gu3dfGetInfoPTR g3gu3dfGetInfo;
G3gu3dfLoadPTR g3gu3dfLoad;
G3guFogTableIndexToWPTR g3guFogTableIndexToW;
G3guFogGenerateExpPTR g3guFogGenerateExp;
G3guFogGenerateExp2PTR g3guFogGenerateExp2;
G3guFogGenerateLinearPTR g3guFogGenerateLinear;
