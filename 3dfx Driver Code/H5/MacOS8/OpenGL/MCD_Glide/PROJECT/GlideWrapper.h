#if PROFILE


 void 
glideWrapperDrawPoint( const void *pt );

 void 
glideWrapperDrawLine( const void *v1, const void *v2 );

 void 
glideWrapperDrawTriangle( const void *a, const void *b, const void *c );

 void 
glideWrapperVertexLayout(FxU32 param, FxI32 offset, FxU32 mode);

 void  
glideWrapperDrawVertexArray(FxU32 mode, FxU32 Count, void *pointers);

 void  
glideWrapperDrawVertexArrayContiguous(FxU32 mode, FxU32 Count, void *pointers, FxU32 stride);

 void 
glideWrapperAADrawTriangle(
                 const void *a, const void *b, const void *c,
                 FxBool ab_antialias, FxBool bc_antialias, FxBool ca_antialias
                 );

 void 
glideWrapperBufferClear( GrColor_t color, GrAlpha_t alpha, FxU32 depth );

 void 
glideWrapperBufferSwap( FxU32 swap_interval );

 void 
glideWrapperRenderBuffer( GrBuffer_t buffer );

 void  
glideWrapperErrorSetCallback( GrErrorCallbackFnc_t fnc );

 void  
glideWrapperFinish(void);

 void  
glideWrapperFlush(void);

 GrContext_t  
glideWrapperSstWinOpen(
          FxU32                hWnd,
          GrScreenResolution_t screen_resolution,
          GrScreenRefresh_t    refresh_rate,
          GrColorFormat_t      color_format,
          GrOriginLocation_t   origin_location,
          int                  nColBuffers,
          int                  nAuxBuffers);

 FxBool 
glideWrapperSstWinClose( GrContext_t context );

 FxBool 
glideWrapperSelectContext( GrContext_t context );

 void 
glideWrapperSstOrigin(GrOriginLocation_t  origin);

 void  
glideWrapperSstSelect( int which_sst );

 void 
glideWrapperAlphaBlendFunction(
                     GrAlphaBlendFnc_t rgb_sf,   GrAlphaBlendFnc_t rgb_df,
                     GrAlphaBlendFnc_t alpha_sf, GrAlphaBlendFnc_t alpha_df
                     );

 void 
glideWrapperAlphaCombine(
               GrCombineFunction_t function, GrCombineFactor_t factor,
               GrCombineLocal_t local, GrCombineOther_t other,
               FxBool invert
               );

 void 
glideWrapperAlphaControlsITRGBLighting( FxBool enable );

 void 
glideWrapperAlphaTestFunction( GrCmpFnc_t function );

 void 
glideWrapperAlphaTestReferenceValue( GrAlpha_t value );

 void  
glideWrapperChromakeyMode( GrChromakeyMode_t mode );

 void  
glideWrapperChromakeyValue( GrColor_t value );

 void  
glideWrapperClipWindow( FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy );

 void  
glideWrapperColorCombine(
               GrCombineFunction_t function, GrCombineFactor_t factor,
               GrCombineLocal_t local, GrCombineOther_t other,
               FxBool invert );

 void 
glideWrapperColorMask( FxBool rgb, FxBool a );

 void  
glideWrapperCullMode( GrCullMode_t mode );

 void  
glideWrapperConstantColorValue( GrColor_t value );

 void  
glideWrapperDepthBiasLevel( FxI32 level );

 void  
glideWrapperDepthBufferFunction( GrCmpFnc_t function );

 void  
glideWrapperDepthBufferMode( GrDepthBufferMode_t mode );

 void  
glideWrapperDepthMask( FxBool mask );

 void  
glideWrapperDisableAllEffects( void );

 void  
glideWrapperDitherMode( GrDitherMode_t mode );

 void  
glideWrapperFogColorValue( GrColor_t fogcolor );

 void  
glideWrapperFogMode( GrFogMode_t mode );

 void  
glideWrapperFogTable( const GrFog_t ft[] );

 void  
glideWrapperLoadGammaTable( FxU32 nentries, FxU32 *red, FxU32 *green, FxU32 *blue);

 void 
glideWrapperSplash(float x, float y, float width, float height, FxU32 frame);

 FxU32  
glideWrapperGet( FxU32 pname, FxU32 plength, FxI32 *params );

 const char *  
glideWrapperGetString( FxU32 pname );

 FxI32  
glideWrapperQueryResolutions( const GrResolution *resTemplate, GrResolution *output );

 FxBool  
glideWrapperReset( FxU32 what );

 GrProc 
glideWrapperGetProcAddress( char *procName );

 void  
glideWrapperEnable( GrEnableMode_t mode );

 void  
glideWrapperDisable( GrEnableMode_t mode );

 void  
glideWrapperCoordinateSpace( GrCoordinateSpaceMode_t mode );

 void  
glideWrapperDepthRange( FxFloat n, FxFloat f );

 void  
glideWrapperViewport( FxI32 x, FxI32 y, FxI32 width, FxI32 height );

 FxU32  
glideWrapperTexCalcMemRequired(
                     GrLOD_t lodmin, GrLOD_t lodmax,
                     GrAspectRatio_t aspect, GrTextureFormat_t fmt);

 FxU32  
glideWrapperTexTextureMemRequired( FxU32     evenOdd,
                                 GrTexInfo *info   );

 FxU32  
glideWrapperTexMinAddress( GrChipID_t tmu );

 FxU32  
glideWrapperTexMaxAddress( GrChipID_t tmu );

 void  
glideWrapperTexNCCTable( GrNCCTable_t table );

 void  
glideWrapperTexSource( GrChipID_t tmu,
             FxU32      startAddress,
             FxU32      evenOdd,
             GrTexInfo  *info );

 void  
glideWrapperTexClampMode(
               GrChipID_t tmu,
               GrTextureClampMode_t s_clampmode,
               GrTextureClampMode_t t_clampmode
               );

 void  
glideWrapperTexCombine(
             GrChipID_t tmu,
             GrCombineFunction_t rgb_function,
             GrCombineFactor_t rgb_factor, 
             GrCombineFunction_t alpha_function,
             GrCombineFactor_t alpha_factor,
             FxBool rgb_invert,
             FxBool alpha_invert
             );

 void  
glideWrapperTexDetailControl(
                   GrChipID_t tmu,
                   int lod_bias,
                   FxU8 detail_scale,
                   float detail_max
                   );

 void  
glideWrapperTexFilterMode(
                GrChipID_t tmu,
                GrTextureFilterMode_t minfilter_mode,
                GrTextureFilterMode_t magfilter_mode
                );


 void  
glideWrapperTexLodBiasValue(GrChipID_t tmu, float bias );

 void  
glideWrapperTexDownloadMipMap( GrChipID_t tmu,
                     FxU32      startAddress,
                     FxU32      evenOdd,
                     GrTexInfo  *info );

 void  
glideWrapperTexDownloadMipMapLevel( GrChipID_t        tmu,
                          FxU32             startAddress,
                          GrLOD_t           thisLod,
                          GrLOD_t           largeLod,
                          GrAspectRatio_t   aspectRatio,
                          GrTextureFormat_t format,
                          FxU32             evenOdd,
                          void              *data );

 FxBool  
glideWrapperTexDownloadMipMapLevelPartial( GrChipID_t        tmu,
                                 FxU32             startAddress,
                                 GrLOD_t           thisLod,
                                 GrLOD_t           largeLod,
                                 GrAspectRatio_t   aspectRatio,
                                 GrTextureFormat_t format,
                                 FxU32             evenOdd,
                                 void              *data,
                                 int               start,
                                 int               end );

 void 
glideWrapperTexDownloadTable( GrTexTable_t type, 
                    void         *data );

 void 
glideWrapperTexDownloadTablePartial( GrTexTable_t type, 
                           void         *data,
                           int          start,
                           int          end );

 void  
glideWrapperTexMipMapMode( GrChipID_t     tmu, 
                 GrMipMapMode_t mode,
                 FxBool         lodBlend );

 void  
glideWrapperTexMultibase( GrChipID_t tmu,
                FxBool     enable );

 void 
glideWrapperTexMultibaseAddress( GrChipID_t       tmu,
                       GrTexBaseRange_t range,
                       FxU32            startAddress,
                       FxU32            evenOdd,
                       GrTexInfo        *info );

 FxBool 
glideWrapperLfbLock( GrLock_t type, GrBuffer_t buffer, GrLfbWriteMode_t writeMode,
           GrOriginLocation_t origin, FxBool pixelPipeline, 
           GrLfbInfo_t *info );

 FxBool 
glideWrapperLfbUnlock( GrLock_t type, GrBuffer_t buffer );

 void  
glideWrapperLfbConstantAlpha( GrAlpha_t alpha );

 void  
glideWrapperLfbConstantDepth( FxU32 depth );

 void  
glideWrapperLfbWriteColorSwizzle(FxBool swizzleBytes, FxBool swapWords);

 void 
glideWrapperLfbWriteColorFormat(GrColorFormat_t colorFormat);

 FxBool 
glideWrapperLfbWriteRegion( GrBuffer_t dst_buffer, 
                  FxU32 dst_x, FxU32 dst_y, 
                  GrLfbSrcFmt_t src_format, 
                  FxU32 src_width, FxU32 src_height, 
                  FxBool pixelPipeline,
                  FxI32 src_stride, void *src_data );

 FxBool 
glideWrapperLfbReadRegion( GrBuffer_t src_buffer,
                 FxU32 src_x, FxU32 src_y,
                 FxU32 src_width, FxU32 src_height,
                 FxU32 dst_stride, void *dst_data );

 void 
glideWrapperGlideInit( void );

 void 
glideWrapperGlideShutdown( void );

 void 
glideWrapperGlideGetState( void *state );

 void 
glideWrapperGlideSetState( const void *state );

 void 
glideWrapperGlideGetVertexLayout( void *layout );

 void 
glideWrapperGlideSetVertexLayout( const void *layout );




/*****************************************/

#ifndef NO_DEFINES

#define grDrawPoint	glideWrapperDrawPoint
#define grDrawLine glideWrapperDrawLine
#define grDrawTriangle glideWrapperDrawTriangle
#define grVertexLayout glideWrapperVertexLayout
#define grDrawVertexArray glideWrapperDrawVertexArray
#define grDrawVertexArrayContiguous glideWrapperDrawVertexArrayContiguous
#define grAADrawTriangle glideWrapperAADrawTriangle
#define grBufferClear glideWrapperBufferClear
#define grBufferSwap glideWrapperBufferSwap
#define grRenderBuffer glideWrapperRenderBuffer
#define grErrorSetCallback glideWrapperErrorSetCallback
#define grFinish glideWrapperFinish
#define grFlush glideWrapperFlush
#define grSstWinOpen glideWrapperSstWinOpen
#define grSstWinClose glideWrapperSstWinClose
#define grSelectContext glideWrapperSelectContext
#define grSstOrigin glideWrapperSstOrigin
#define grSstSelect glideWrapperSstSelect
#define grAlphaBlendFunction glideWrapperAlphaBlendFunction
#define grAlphaCombine glideWrapperAlphaCombine
#define grAlphaControlsITRGBLighting glideWrapperAlphaControlsITRGBLighting
#define grAlphaTestFunction glideWrapperAlphaTestFunction
#define grAlphaTestReferenceValue glideWrapperAlphaTestReferenceValue
#define grChromakeyMode glideWrapperChromakeyMode
#define grChromakeyValue glideWrapperChromakeyValue
#define grClipWindow glideWrapperClipWindow
#define grColorCombine glideWrapperColorCombine
#define grColorMask glideWrapperColorMask
#define grCullMode glideWrapperCullMode
#define grConstantColorValue glideWrapperConstantColorValue
#define grDepthBiasLevel glideWrapperDepthBiasLevel
#define grDepthBufferFunction glideWrapperDepthBufferFunction
#define grDepthBufferMode glideWrapperDepthBufferMode
#define grDepthMask glideWrapperDepthMask
#define grDisableAllEffects glideWrapperDisableAllEffects
#define grDitherMode glideWrapperDitherMode
#define grFogColorValue glideWrapperFogColorValue
#define grFogMode glideWrapperFogMode
#define grFogTable glideWrapperFogTable
#define grLoadGammaTable glideWrapperLoadGammaTable
#define grSplash glideWrapperSplash
#define grGet glideWrapperGet
#define grGetString glideWrapperGetString
#define grQueryResolutions glideWrapperQueryResolutions
#define grReset glideWrapperReset
#define grGetProcAddress glideWrapperGetProcAddress
#define grEnable glideWrapperEnable
#define grDisable glideWrapperDisable
#define grCoordinateSpace glideWrapperCoordinateSpace
#define grDepthRange glideWrapperDepthRange
#define grViewport glideWrapperViewport
#define grTexCalcMemRequired glideWrapperTexCalcMemRequired
#define grTexTextureMemRequired glideWrapperTexTextureMemRequired
#define grTexMinAddress glideWrapperTexMinAddress
#define grTexMaxAddress glideWrapperTexMaxAddress
#define grTexNCCTable glideWrapperTexNCCTable
#define grTexSource glideWrapperTexSource
#define grTexClampMode glideWrapperTexClampMode
#define grTexCombine glideWrapperTexCombine
#define grTexDetailControl glideWrapperTexDetailControl
#define grTexFilterMode glideWrapperTexFilterMode
#define grTexLodBiasValue glideWrapperTexLodBiasValue
#define grTexDownloadMipMap glideWrapperTexDownloadMipMap
#define grTexDownloadMipMapLevel glideWrapperTexDownloadMipMapLevel
#define grTexDownloadMipMapLevelPartial glideWrapperTexDownloadMipMapLevelPartial
#define grTexDownloadTable glideWrapperTexDownloadTable
#define grTexDownloadTablePartial glideWrapperTexDownloadTablePartial
#define grTexMipMapMode glideWrapperTexMipMapMode
#define grTexMultibase glideWrapperTexMultibase
#define grTexMultibaseAddress glideWrapperTexMultibaseAddress
#define grLfbLock glideWrapperLfbLock
#define grLfbUnlock glideWrapperLfbUnlock
#define grLfbConstantAlpha glideWrapperLfbConstantAlpha
#define grLfbConstantDepth glideWrapperLfbConstantDepth
#define grLfbWriteColorSwizzle glideWrapperLfbWriteColorSwizzle
#define grLfbWriteColorFormat glideWrapperLfbWriteColorFormat
#define grLfbWriteRegion glideWrapperLfbWriteRegion
#define grLfbReadRegion glideWrapperLfbReadRegion
#define grGlideInit glideWrapperGlideInit
#define grGlideShutdown glideWrapperGlideShutdown
#define grGlideGetState glideWrapperGlideGetState
#define grGlideSetState glideWrapperGlideSetState
#define grGlideGetVertexLayout glideWrapperGlideGetVertexLayout
#define grGlideSetVertexLayout glideWrapperGlideSetVertexLayout

#endif // NO_DEFINES

#endif // PROFILE

