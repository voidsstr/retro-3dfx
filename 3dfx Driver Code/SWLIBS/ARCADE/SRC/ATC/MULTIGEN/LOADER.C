/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
**
** $Revision: 4$ 
** $Date: 10/11/00 7:31:49 PM$ 
**
** Notes:
**      MultiGen's coordinate system s right handed with z vertical, and 
**      X and Y defining defining the horizontal plane.
**
** TBD: material ambient 
**      per vertex dynamically lit colors
**      normals on two sided materials not correct
**      need better matches for texture modes
**      need to understand material priorities
**      how to use intensity, modulate textures with ATR
**      problems with espritmorph.flt how to do LOD morphing 
**      priorities
**      add missing node types
**      search paths, env variables: Currently don't work
**         TXTPATH ( texture directory )
**         FLTEXTERNPATH ( external file directory )
**         FLTPATH ( model directory )
**         TERPATH ( dted directory )
*/

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <windows.h>
#include <glide.h>
#include <texus.h>
#include <atscene.h>    
#include <mgapiall.h>
#include "fltlod.h"

static void ProcessChildren(AtsNode *group, mgrec *fltNode);
static AtsNode* ProcessNode( mgrec* fltNode);
void RealizeTexture(SgTexture* tex);
void OneTexture(mgrec* db, int index, char* name);
void InvalidateTextures(void);

static AtsPrim *prim ;
static MatDef *atbMaterials = NULL;
static int numAtbMaterials = 0;
static int maxAtbMaterials = 0;
static int maxAtbTextures = 0;
static FxU32 gTotalTextureBytes = 0;
static FxU32 gTotalNumPolygons = 0;
static FxU32 gTotalATBTextures = 0;

static FxBool rtTexturesValid = FALSE;
static int rtTexturesMax = 0;
static SgTexture *rtTexturesTable = NULL;
static int maxPredefinedMaterials = 0 ; /* number of materials in header */
static SgMaterial *rtMaterialsTable = NULL ;

static FxBool gVerbose = FXTRUE;

float minU = 10000.0f, maxU = -10000.0f;
float minV = 10000.0f, maxV = -10000.0f;

static void printStatistics(void) {
    printf("texture range u (%f, %f), v (%f, %f)\n", minU, maxU, minV, maxV); 
    printf("num polys %d, numMaterials %d, numTextures %d, texture bytes %d\n",
           gTotalNumPolygons, numAtbMaterials, gTotalATBTextures, gTotalTextureBytes);
}

void AllocateMaterials(int n ) {
    FxU32 bytesNeeded;

    if (( numAtbMaterials +n ) >= maxAtbMaterials ) {
        maxAtbMaterials += n+MAT_INC;
        bytesNeeded = maxAtbMaterials*sizeof(MatDef);
        if ( atbMaterials == NULL ) 
            atbMaterials = (MatDef *)malloc(bytesNeeded);
        else atbMaterials = (MatDef *)realloc(atbMaterials, 
                                                      bytesNeeded);
        if ( atbMaterials == NULL ) 
            atuError(FXTRUE, "Out of memory in allocating materials\n");
    }

}

static void 
findMaterial(MatDef *req) {
    int i;
    MatDef *d;
    FxU32 typeFlag = 0;
    FxBool vertexAlpha;
    FxBool textureAlpha = FXFALSE;
    SgTexture *tex = NULL;
    AtrMaterial *mat;

    vertexAlpha = ( req->transparency < 0.95f );

    if (rtTexturesTable == NULL) {
        req->textureIndex = -1;
    } else if (req->textureIndex > rtTexturesMax) {
        atuError(FXFALSE, "Invalid texture index %d\n", req->textureIndex);
		req->textureIndex = -1;
    } else if ( req->textureIndex != -1 ) {
        tex = rtTexturesTable+req->textureIndex;
        if ( tex->atbTexture == NULL )
            RealizeTexture(tex);

        textureAlpha = atsTextureHasAlpha(tex->atbTexture);
    }

    if ( vertexAlpha ) {
        req->transpMode = textureAlpha ? ATMG_TRANSP_BLEND : 
                                         ATMG_TRANSP_VERTEX;
    } else {
        req->transpMode = textureAlpha ? ATMG_TRANSP_TEXTURE : 
                                         ATMG_TRANSP_NONE;
    }

    for ( i = 0; i < numAtbMaterials; i++ ) {
        d = atbMaterials+i;
        if (( d->mat.dif.r != req->mat.dif.r ) || 
            ( d->mat.dif.g != req->mat.dif.g ) || 
            ( d->mat.dif.b != req->mat.dif.b )) {
            continue;
        }
        if (( d->mat.amb.r != req->mat.amb.r ) || 
            ( d->mat.amb.g != req->mat.amb.g ) || 
            ( d->mat.amb.b != req->mat.amb.b )) {
            continue;
        }
        if (( d->mat.spe.r != req->mat.spe.r ) || 
            ( d->mat.spe.g != req->mat.spe.g ) || 
            ( d->mat.spe.b != req->mat.spe.b )) {
            continue;
        }
        if (( d->mat.emi.r != req->mat.emi.r ) || 
            ( d->mat.emi.g != req->mat.emi.g ) || 
            ( d->mat.emi.b != req->mat.emi.b )) {
            continue;
        }
        if ( d->mat.shine != req->mat.shine ) {
            continue;
        }
        if ( d->transpMode != req->transpMode ) {
            continue;
        }
        if ( d->textureIndex != req->textureIndex ) {
            continue;
        }
        if ( d->lightMode != req->lightMode ) {
            continue;
        }
        if ( d->template != req->template ) {
            continue;
        }
        req->atbMaterial = d->atbMaterial;
        return;
    }

    AllocateMaterials(1) ;
    d = atbMaterials+numAtbMaterials++;
    *d = *req;
    d->atbMaterial = atsMaterialNew();
    req->atbMaterial = d->atbMaterial;

    /* TBD: COLOR_SET(ent->mat, amb.r, amb.g, amb.b); */
    COLOR_SET(d->atbMaterial->material.diffuse, req->mat.dif.r, req->mat.dif.g, req->mat.dif.b);
    COLOR_SET(d->atbMaterial->material.specular, req->mat.spe.r, req->mat.spe.g, req->mat.spe.b);
    COLOR_SET(d->atbMaterial->material.emissive, req->mat.emi.r, req->mat.emi.g, req->mat.emi.b);

    d->atbMaterial->material.specExponent = (int)req->mat.shine;

    if ( d->textureIndex == -1 ) {
        typeFlag |= ATR_MAT_GSHADE;

        if ( d->transpMode != ATMG_TRANSP_NONE ) {
            typeFlag |= ATR_MAT_FB_BLEND;
        }
         
        atrMaterialSetup( &d->atbMaterial->material, typeFlag );
   } else {
       AtrTexelFx tmu = atsTextureGetTMU(tex->atbTexture);

       /* TBD: need to find better matches */
       /* 0=MODULATE, 1= BLEND, 2=DECAL, 3=COLOR */
       switch ( tex->imgEnvType ) {
       case 1: /* BLEND */
           /* TBD: blend texture type not supported */
           typeFlag |= (tmu) ? ATR_MAT_DECAL1 : ATR_MAT_DECAL;
	       break;
       case 0: /* MODULATE */
               switch ( req->lightMode ) {
               case 0: /* Use face color, not illuminated */
               case 1: /* Use vertex colors, not illuminated */
                   typeFlag |= ( ATR_MAT_LIGHTSRC_STATIC | 
                                 ATR_MAT_LIGHTING_MULTIPLY);
                   break;
               case 2: /* Use face color and vertex normal */
               case 3: /* Use vertex color and vertex normal */
                   typeFlag |= ( ATR_MAT_LIGHTSRC_LIGHT | 
                                 ATR_MAT_LIGHTING_MULTIPLY);
                   break;
               default:
                   break;
               }
               typeFlag |= tmu ? ATR_MAT_TEX_DECAL1 : ATR_MAT_TEX_DECAL;
	       break;
       case 3: /* COLOR */
           /* TBD: COLOR texture type not supported */
           typeFlag |= (tmu) ? ATR_MAT_DECAL1 : ATR_MAT_DECAL;
	       break;
       case 2: /* DECAL */
       default:
               typeFlag |= (tmu) ? ATR_MAT_DECAL1 : ATR_MAT_DECAL;
               break;
       }

       if ( d->transpMode != ATMG_TRANSP_NONE ) {
            typeFlag |= ATR_MAT_FB_BLEND;
       }

       atrMaterialSetup( &d->atbMaterial->material, typeFlag);


       d->atbMaterial->material.texMMMode[tmu] = ATR_TEXMIPMAP_NEAREST ; /* use mip maps if available */

       /* 0=repeat, 1=clamp, 2=obsolete */

       d->atbMaterial->material.texSClamp[tmu] = ( tex->imgWrapU == 0 ) ? ATR_TEXCLAMP_WRAP : ATR_TEXCLAMP_CLAMP;
       d->atbMaterial->material.texTClamp[tmu] = ( tex->imgWrapV == 0 ) ? ATR_TEXCLAMP_WRAP : ATR_TEXCLAMP_CLAMP;

       /* 0=POINT, 1=BILINEAR, 2 =obsolete, see apimissing.h */

       d->atbMaterial->material.texMinFilter[tmu] = ( tex->imgMinFilter == 0 ) ? ATR_TEXFILTER_POINT_SAMPLED :
                                                   ATR_TEXFILTER_BILINEAR ;
                
       d->atbMaterial->material.texMagFilter[tmu] = ( tex->imgMagFilter == 0 ) ? ATR_TEXFILTER_POINT_SAMPLED :
                                                     ATR_TEXFILTER_BILINEAR ;

       d->atbMaterial->material.texMagFilter[tmu] = ( tex->imgMagFilter == 0 ) ? ATR_TEXFILTER_POINT_SAMPLED :
                                                     ATR_TEXFILTER_BILINEAR ;

       atsMaterialTexture(d->atbMaterial, tmu, tex->atbTexture);
       atsRef(tex->atbTexture);
   }

    mat = &(d->atbMaterial->material);

    switch ( d->transpMode ) {
    case ATMG_TRANSP_NONE:
        mat->iaSrc        = ATR_IASRC_NONE;
        mat->acuFunction  = GR_COMBINE_FUNCTION_ZERO;
        mat->acuInvert    = FXTRUE;
        break;
    case ATMG_TRANSP_VERTEX:
        mat->iaSrc        = ATR_IASRC_STATIC;
        mat->acuFunction  = GR_COMBINE_FUNCTION_LOCAL;
        mat->acuFactor    = GR_COMBINE_FACTOR_NONE;
        mat->acuLocal     = GR_COMBINE_LOCAL_ITERATED;
        mat->acuOther     = GR_COMBINE_OTHER_NONE;
        mat->acuInvert    = FXFALSE;
        break;
    case ATMG_TRANSP_TEXTURE:
        mat->iaSrc        = ATR_IASRC_NONE;
        mat->acuFunction  = GR_COMBINE_FUNCTION_SCALE_OTHER;
        mat->acuFactor    = GR_COMBINE_FACTOR_ONE;
        mat->acuLocal     = GR_COMBINE_LOCAL_NONE;
        mat->acuOther     = GR_COMBINE_OTHER_TEXTURE;
        mat->acuInvert    = FXFALSE;
        break;
    case ATMG_TRANSP_BLEND:
        mat->iaSrc        = ATR_IASRC_STATIC;
        mat->acuFunction  = GR_COMBINE_FUNCTION_SCALE_OTHER;
        mat->acuFactor    = GR_COMBINE_FACTOR_TEXTURE_ALPHA;
        mat->acuLocal     = GR_COMBINE_LOCAL_NONE;
        mat->acuOther     = GR_COMBINE_OTHER_ITERATED;
        mat->acuInvert    = FXFALSE;
        break;
    }
}

void GetTexturePixels(unsigned char** pixmap, mgrec* db_rec, int tindex) {
    *pixmap = mgGetTextureTexels ( db_rec, tindex );
}

void sgTextureNew(SgTexture *tex, void* db, char* textureName, int tindex) {
    mgrec* db_rec = (mgrec*)db;
    mgrec *texrec;
    int imgWrap;
    char *p, *pTmp, *qTmp;
    FxU32 redOffset, greenOffset, blueOffset, alphaOffset;
    int row, col;

    tex->pixmap = (FxU8*)NULL;
    tex->defined = FXFALSE;
    tex->atbTexture = NULL;

    if ( texrec = mgGetTextureAttributes(db_rec, tindex) ) {
        /* Get image attributes */

        mgGetAttList ( texrec,
                       fltImgType, &tex->type,	//see 'apimissing.h'
                       fltImgWidth,&tex->width,
                       fltImgHeight, &tex->height,
                       fltImgExternalFormat, &tex->extfmt,
                       mgNULL);

        /* Get texture=mapping attributes */

        mgGetAttList ( texrec,
                       fltImgWrap, &imgWrap,       //0=repeat, 1=clamp, 2=obsolete
                       fltImgWrapU, &tex->imgWrapU,//0=repeat, 1=clamp, 2=obsolete, 3=use imgWrap
                       fltImgWrapU, &tex->imgWrapV,//0=repeat, 1=clamp, 2=obsolete, 3=use imgWrap
                       fltImgEnvType, &tex->imgEnvType,	//0=MODULATE, 1=BLEND, 2=DECAL, 3=COLOR
                       fltImgMinFilter,&tex->imgMinFilter,	//see apimissing.h
                       fltImgMagFilter,&tex->imgMagFilter,	//see apimissing.h
                       mgNULL);

        if (tex->imgWrapU == 3) {
            tex->imgWrapU = imgWrap;
        }

        if (tex->imgWrapV == 3) {
            tex->imgWrapV = imgWrap;
        }

    tex->name = strdup(textureName);

#if 0
        int error = 0;
        unsigned char *pattern = 0;
        unsigned char *rmap = 0, *gmap = 0, *bmap = 0;

        error = mgReadImage ( textureName, &pattern,
        &type, &width, &height, &extfmt, &cmapsize,
        &rmap, &gmap, &bmap,
         1 );			//pass a '1' to suppress packing
        if (error == 0) {
            pixmap = pattern;
        }
#else
        /* get a copy of the pixels in 'pixmap'
           Note: 'pixmap' -> packed RGBA if 'type == mgiRGBA'
           else HxW # of red-channel pixels, followed by green and blue for mgiRGB
         */

        GetTexturePixels(&tex->pixmap, db_rec, tindex);

        switch (tex->type) {
        case mgiIntensity:
            break;
        case mgiIntAlpha:
            if ((p = malloc(tex->width*tex->height*2)) == NULL ) {
                atuError(FXTRUE, "Can't allocate texture memory\n");
            }
            pTmp = p;
            qTmp = tex->pixmap;
            alphaOffset = tex->width*tex->height;
            for ( row = 0; row < tex->height; row++ ) {
                for ( col = 0; col < tex->width; col++ ) {
                    *pTmp++ = *qTmp;
                    *pTmp++ = *(qTmp+alphaOffset);
                    qTmp++;
                }
            }
            tex->pixmap = p;
            break;
        case mgiRGB:
            if ((p = malloc(tex->width*tex->height*4)) == NULL ) {
                atuError(FXTRUE, "Can't allocate texture memory\n");
            }
            pTmp = p;
            qTmp = tex->pixmap;
            redOffset = 0;
            greenOffset = redOffset+tex->width*tex->height;
            blueOffset = greenOffset+tex->width*tex->height;
            for ( row = 0; row < tex->height; row++ ) {
                for ( col = 0; col < tex->width; col++ ) {
                    *pTmp++ = *(qTmp+blueOffset);
                    *pTmp++ = *(qTmp+greenOffset);
                    *pTmp++ = *(qTmp+redOffset);
                    *pTmp++ = (FxU8)255;
                    qTmp++;
                }
            }
            tex->pixmap = p;
            break;
        case mgiRGBA:
            break;
        default:
            atuError(FXTRUE, "Unsupported texture format %d\n", tex->type);
            break;
        } 
#endif
    }
}

void sgTextureFree(SgTexture *tex) {
    if (tex->name)
        free(tex->name);
    if (tex->pixmap)
        ;    /* don't know how to delete this right now - need new API call */
}

/*
 * 'RootNode' texture related methods:
 */
void CreateTextureTable(mgrec* db) {
    int result, index, total = 0, maxIndex = -1;
    char name[128];
    int i;

	index = 0;
    result = mgGetFirstTexture( db, &index, name );
    if (result)
        maxIndex = index;
    while (result && (index >= 0)) {
        maxIndex = __max(maxIndex, index);
        total += 1;
        result = mgGetNextTexture(db, &index, name);
    }

    rtTexturesMax = maxIndex;

    /* desc. for textures */

    rtTexturesTable = (SgTexture*)NULL;

    if (total) {
        rtTexturesTable = (SgTexture *)calloc((maxIndex + 1), sizeof(SgTexture));

        if (rtTexturesTable) {
            SgTexture* ourEntry = rtTexturesTable;

            for (i=0; i <= rtTexturesMax; i++) {
                /* undefined entries will be marked by '-1' */
                ourEntry->defined = FALSE; 
                ourEntry++;
            }
            result = mgGetFirstTexture( db, &index, name );
            while (result && (index >= 0)) {
                OneTexture(db, index, name);

                result = mgGetNextTexture(db, &index, name);
            }
        }
    }
    rtTexturesValid = FALSE;
}

void DestroyTextureTable(void) {
    int i;

    if (rtTexturesTable) {
         SgTexture* ourEntry = rtTexturesTable;

        /* Free render dependent objects */

        InvalidateTextures();

        for (i=0; i < rtTexturesMax; i++) {
            sgTextureFree(ourEntry);
            ourEntry++;
        }
        free(rtTexturesTable);
        rtTexturesTable = (SgTexture*)NULL;
    }
}

/*
 * Setup the texture entry of 'index':
 */

void OneTexture(mgrec* db, int index, char* name) {
    if (index <= rtTexturesMax) {
        SgTexture* ourEntry = rtTexturesTable + index;

        sgTextureNew(ourEntry, db, name, index);
    }
}

FxBool CreateTextureObj(int index) {
    SgTexture* ourEntry = rtTexturesTable + index;
    assert(index <= rtTexturesMax);

    return TRUE;
}

FxBool CreateTextureObjs(void) {
#ifdef notdef
    FxBool result;
    SgTexture* ourEntry = rtTexturesTable;

    for (int i=0; i <= rtTexturesMax; i++) {
        if (ourEntry->IsUsed()) { /* only create those materials that are used */
            result = CreateTextureObj(rc, i);
        }
        ourEntry++;
    }
    rtTexturesValid = FXTRUE;

#endif
    return FXTRUE;
}

void InvalidateTextures(void) {
#ifdef notdef
    SgTexture* ourEntry = rtTexturesTable;
	
    if (rtTexturesTable) {
        for (int i=0; i <= rtTexturesMax; i++) {
            if (ourEntry->texture != 0) {
                ;       /* ourEntry->texture->Invalidate() */
                        /* Destroy texture object */
			}
			ourEntry++;
		}
	}
	rtTexturesValid = FALSE;
#endif
}

void RealizeTexture(SgTexture* tex) {
    size_t tex_mem_required;
    Gu3dfInfo *info;
    int target_width, target_height; /* values to resize image to */
    AtsTexture *atbtex;
    FxU32 srcFormat, dstFormat;
    AtrImg     *img;

    if ( ( info = (Gu3dfInfo *)malloc( sizeof(Gu3dfInfo) )) == NULL ) {
        atuError( FXTRUE, "Out of memory allocating system memory textures.\n" )
;
    }

    /* TBD: better heuristic for texture sizing */

    target_width = tex->width;
    target_height = tex->height;
  
    while ( (target_width > 256 ) || ( target_height > 256 )) {
        if ( target_width > 8 )
            target_width >>= 1;
        if ( target_height > 8 )
            target_height >>= 1;
    }

    switch ( tex->type ) {
    case mgiIntensity:
        srcFormat = GR_TEXFMT_INTENSITY_8;
        dstFormat = GR_TEXFMT_INTENSITY_8;
        break;
    case mgiIntAlpha:
        srcFormat = GR_TEXFMT_ALPHA_INTENSITY_88;
        dstFormat = GR_TEXFMT_ALPHA_INTENSITY_44;
        break;
    case mgiRGB:
        srcFormat = GR_TEXFMT_ARGB_8888,
        dstFormat = GR_TEXFMT_RGB_565;
        break;
    case mgiRGBA:
        srcFormat = GR_TEXFMT_ARGB_8888,
        dstFormat = GR_TEXFMT_ARGB_4444;
        break;
    }

    tex_mem_required = txInit3dfInfo( info, dstFormat,
                                      &target_width, &target_height,
                                      -1, TX_AUTORESIZE_GROW );
    /*
     * Make sure txInit3dfInfo didn't fail.
     */

    if ( tex_mem_required == 0 ) {
        atuError( FXTRUE, "Problem with txInit3dfInfo\n" );
    }

    if ( gVerbose)
        printf("loading texture %s original (%d, %d) created (%d, %d) %d bytes\n",
           tex->name, tex->width, tex->height, target_width, target_height,
           tex_mem_required);

    gTotalTextureBytes += tex_mem_required;
    gTotalATBTextures++;

    /*
     * Allocate system memory for the texture.
     */

    if ( ( info->data = malloc( tex_mem_required )) == NULL ) {
        atuError( FXTRUE, "Out of memory allocating system memory textures.\n" )
;
    }

    /*
     * Convert to a texture that can be downloaded to the hardware.
     * TBD: the image is upside down
     */

    txConvert( info, srcFormat,
               tex->width, tex->height, tex->pixmap, TX_DITHER_ERR, NULL );

    if (( atbtex = atsTextureNew()) == NULL ) {
        atuError(FXTRUE, "Could not create texture\n");
    }

    img          = atsImageNew();
    img->format  = info->header.format;
    img->width   = info->header.width;
    img->height  = info->header.height;
#ifdef GLIDE3
    img->nLevels = info->header.large_lod - info->header.small_lod + 1;
#else
    img->nLevels = 1 + info->header.small_lod - info->header.large_lod;
#endif
    img->name    = strdup( tex->name );
    img->data    = info->data;
    img->table   = (AtrImgTable*)&info->table;

    atsTextureAddImage(atbtex, img);

    atsTextureReset( atbtex );

    tex->atbTexture = atbtex;
}

void mgeGetNormColor(mgrec* rec, normcolor* color) {
    mgGetAttList (rec,  fltNColorR, &color->r,
                        fltNColorG, &color->g,
                        fltNColorB, &color->b, mgNULL);
}

void OneMaterial(mgrec *db, int index, mgrec* matrec) {
    /* First good entry is 'index = 1':	 */

    SgMaterial* mat = rtMaterialsTable + index;

	mat->materialCode = index;

    /* mgNameOfMaterial allocs, must dealloc */

    mat->name = mgNameOfMaterial(db, index);
    mgGetNormColor(matrec, fltAmbient, &mat->amb.r, &mat->amb.g, &mat->amb.b);
    mgGetNormColor(matrec, fltDiffuse, &mat->dif.r, &mat->dif.g, &mat->dif.b);
    mgGetNormColor(matrec, fltSpecular, &mat->spe.r, &mat->spe.g, &mat->spe.b);
    mgGetNormColor(matrec, fltEmissive, &mat->emi.r, &mat->emi.g, &mat->emi.b);

    mgGetAttList( matrec, fltShininess, &mat->shine,
                          fltMatAlpha, &mat->alpha,
                          mgNULL );
}

static void CreateMaterialTable(mgrec *db) {
    mgrec *matrec;
    int index;

    maxPredefinedMaterials = mgGetMaterialCount(db);
    rtMaterialsTable = (SgMaterial *)calloc( maxPredefinedMaterials+1, 
                                             sizeof(SgMaterial));
    if ( rtMaterialsTable == NULL )
        atuError(FXTRUE, "can't allocate predefined materials table\n");

    if ( matrec = mgGetFirstMaterial(db, &index)) {
        OneMaterial(db, index, matrec);
        while ( matrec = mgGetNextMaterial(matrec, &index)) {
            OneMaterial(db, index, matrec);
        }
    }
}

static FltNodeMap fltNodeMapping[fvLastNodeType + 1];

void atsInitFlt(void) {
    int i = 0;

    mgInit( (int*)NULL, (char**)NULL );

    /* Init mapping table from API code(a runtime code) => our enums */

    fltNodeMapping[i].ftAPICode = fltHeader;
    fltNodeMapping[i].ftNodeType = fvHeader;	
    i++;
    
    fltNodeMapping[i].ftAPICode = fltGroup;
    fltNodeMapping[i].ftNodeType = fvGroup;		
    i++;
    
    fltNodeMapping[i].ftAPICode = fltObject;
    fltNodeMapping[i].ftNodeType = fvObject;
    i++;
    
    fltNodeMapping[i].ftAPICode = fltPolygon;
    fltNodeMapping[i].ftNodeType = fvPolygon;
    i++;

    fltNodeMapping[i].ftAPICode = fltVertex;
    fltNodeMapping[i].ftNodeType = fvVertex;
    i++;
    
    fltNodeMapping[i].ftAPICode = fltLod;
    fltNodeMapping[i].ftNodeType = fvLodFloat;	
    i++;
    
    fltNodeMapping[i].ftAPICode = fltLightSource;	
    fltNodeMapping[i].ftNodeType = fvLightSource;	
    i++;

    fltNodeMapping[i].ftAPICode = fltDof;	
    fltNodeMapping[i].ftNodeType = fvDof;	
    i++;

    fltNodeMapping[i].ftAPICode = fltSwitch;	
    fltNodeMapping[i].ftNodeType = fvSwitch;	
    i++;
        
    fltNodeMapping[i].ftAPICode = 312; /* fltIRoad; */
    fltNodeMapping[i].ftNodeType = fvRoad;	
    i++;
        
    fltNodeMapping[i].ftAPICode = fltPath;	
    fltNodeMapping[i].ftNodeType = fvPath;	
    i++;
        
    /* The following entry will never match any real flyAPI code */

    fltNodeMapping[i].ftAPICode = (mgcode)-1;
    fltNodeMapping[i].ftNodeType = fvUnknown;
}

void atsTermFlt(void) {
    /* this doesn't exist! */
    /* mgExit( NULL, NULL ); */
}

static BOOL __inline HasChild(mgrec* rec) {
    if (rec == NULL)
        return 0;
    else return mgCountChild(rec) != 0;
}

static FVNodeType GetNodeType(mgrec* fltNode) {
    mgcode ourcode = mgGetCode(fltNode);
    FVNodeType ntype = fvUnknown;
    int i;

    for (i=0; i < fvUnknown; i++) {
        if (fltNodeMapping[i].ftAPICode == ourcode) {
            ntype = fltNodeMapping[i].ftNodeType;
            break;
        }
    }

    if ( ntype == fvUnknown ) {
        atuError(FXTRUE, "unknown node type name %s, id = %d\n",
                 mgGetName( fltNode ), ourcode);
    }

    return ntype;
}

AtsNode *GroupNode(mgrec* fltNode) {
    AtsNode *group;
    FxU16 grpPrio;
    FxU16 grpFBAnimation;
#ifdef notdef
    FxU16 grpAnimation;
    FxU16 grpLayer;
    FxU16 grpSignificance;
    FxU16 grpSpecial1;
    FxU16 grpSpecial2;
#endif

    mgGetAttList(fltNode, fltGrpPrio, &grpPrio, mgNULL);
    mgGetAttList(fltNode, fltGrpFlagAnimationFB, &grpFBAnimation, mgNULL);

#ifdef notdef
    mgGetAttValUS(fltNode, fltGrpFlagAnimation, &grpAnimation);
    mgGetAttValUS(fltNode, fltGrpLayer, &grpLayer);
    mgGetAttValUS(fltNode, fltGrpSignificance, &grpSignificance);
    mgGetAttValUS(fltNode, fltGrpSpecial1, &grpSpecial1);
    mgGetAttValUS(fltNode, fltGrpSpecial2, &grpSpecial2);
#endif

#ifdef notdef /* TBD: the spitfire is marked as animated but this removes half the spitfire */
    if ( grpAnimation ) {
        group = atsSeqNew();
        atsSeqDuration(group, 20.0f);
        atsSeqControl(group, ATS_SEQ_START);
    } else group = atsGroupNew();
#else
    group = atsGroupNew();
#endif

    ProcessChildren(group, fltNode);
   
    return group;
}

AtsNode *DofNode(mgrec* dofNode) {
    AtsNode *frame;
    double dfPutAnchor[3]; /* origin of DOF's local coordinate system */
    double dfPutAlign[3]; /* Point on x-axis of DOF's local coord system */
    double dfPutTrack[3]; /* Point in the xy plane of DOF's lcs */
    
    double dfMin[3]; /* minimum value of each axis w/ repsect of lcs */
    double dfMax[3];
    double dfCur[3];
    double dfIncrement[3]; /* increment for each axis */
    
    double dfMinScale[3]; /* minimum scale of each axis about local origin */
    double dfMaxScale[3];
    double dfCurScale[3];
    double dfIncrementScale[3]; /* increment for scale in each axis */
    
    double dfMinAzim; /* min pitch, rot around x axis */
    double dfMaxAzim; /* max pitch */
    double dfCurAzim; /* cur pitch */
    double dfIncrementAzim; /* increment in pitch */
    
    double dfMinIncl; /* min roll, rot around y axis */
    double dfMaxIncl; /* max roll */
    double dfCurIncl; /* cur roll */
    double dfIncrementIncl; /* increment in roll */
    
    double dfMinTwist; /* min yaw, rot around z axis */
    double dfMaxTwist; /* max yaw */
    double dfCurTwist; /* cur yaw */
    double dfIncrementTwist; /* increment in yaw */
    
    /* getting all DOF attributes in one call */

    mgGetAttList( dofNode,
              fltDofPutAnchorX, &dfPutAnchor[0],
              fltDofPutAnchorY, &dfPutAnchor[1],
              fltDofPutAnchorZ, &dfPutAnchor[2],
              fltDofPutAlignX, &dfPutAlign[0],
              fltDofPutAlignY, &dfPutAlign[1],
              fltDofPutAlignZ, &dfPutAlign[2],
              fltDofPutTrackX, &dfPutTrack[0],
              fltDofPutTrackY, &dfPutTrack[1],
              fltDofPutTrackZ, &dfPutTrack[2],
              fltDofMinX, &dfMin[0],
              fltDofMinY, &dfMin[1],
              fltDofMinZ, &dfMin[2],
              fltDofMaxX, &dfMax[0],
              fltDofMaxY, &dfMax[1],
              fltDofMaxZ, &dfMax[2],
              fltDofCurX, &dfCur[0],
              fltDofCurY, &dfCur[1],
              fltDofCurZ, &dfCur[2],
              fltDofIncrementX, &dfIncrement[0],
              fltDofIncrementY, &dfIncrement[1],
              fltDofIncrementZ, &dfIncrement[2],
              fltDofMinXScale, &dfMinScale[0],
              fltDofMinYScale, &dfMinScale[1],
              fltDofMinZScale, &dfMinScale[2],
              fltDofMaxXScale, &dfMaxScale[0],
              fltDofMaxYScale, &dfMaxScale[1],
              fltDofMaxZScale, &dfMaxScale[2],
              fltDofCurXScale, &dfCurScale[0],
              fltDofCurYScale, &dfCurScale[1],
              fltDofCurZScale, &dfCurScale[2],
              fltDofIncrementXScale, &dfIncrementScale[0],
              fltDofIncrementYScale, &dfIncrementScale[1],
              fltDofIncrementZScale, &dfIncrementScale[2],
              fltDofMinAzim, &dfMinAzim,
              fltDofMaxAzim, &dfMaxAzim,
              fltDofCurAzim, &dfCurAzim,
              fltDofIncrementAzim, &dfIncrementAzim,
              fltDofMinIncl, &dfMinIncl,
              fltDofMaxIncl, &dfMaxIncl,
              fltDofCurIncl, &dfCurIncl,
              fltDofIncrementIncl, &dfIncrementIncl,
              fltDofMinTwist, &dfMinTwist,
              fltDofMaxTwist, &dfMaxTwist,
              fltDofCurTwist, &dfCurTwist,
              fltDofIncrementTwist, &dfIncrementTwist,
              mgNULL );

    frame = atsFrameNew();

    ProcessChildren(frame, dofNode);
   
    return frame;
}

AtsNode *RoadNode(mgrec* fltNode) {
    AtsNode *group;

    group = atsGroupNew();

    ProcessChildren(group, fltNode);
   
    return group;
}

AtsNode *PathNode(mgrec* fltNode) {
    AtsNode *group;

    group = atsGroupNew();

    ProcessChildren(group, fltNode);
   
    return group;
}

AtsNode *LightSourceNode(mgrec* fltNode) {
    AtsNode *group;

    group = atsGroupNew();

    ProcessChildren(group, fltNode);
   
    return group;
}

mgbool mgeGetRGB( FxU8 *r, FxU8 *g, FxU8 *b, mgrec* rec, mgcode colorFd,
	          mgcode intensityFd) {
    mgbool result;
    FxU32 colorIdx;
	
    result = mgGetAttList(rec, colorFd, &colorIdx, mgNULL);
    if (result && (colorIdx < MAXMGCOLORINDEX)) {
        short rr, gg, bb;
        float intensity;

        mgGetAttList(rec, intensityFd, &intensity, mgNULL);

        mgIndex2RGB(mgRec2Db(rec),colorIdx, intensity, &rr, &gg, &bb);
        *r =(FxU8)rr;
        *g =(FxU8)gg;
        *b =(FxU8)bb;

        return mgTRUE;
    } else return mgFALSE;
}

void DefaultMaterial(SgMaterial *mat) {
    memset(mat, 0, sizeof(SgMaterial));
    mat->dif.r = 1.0f;
    mat->dif.g = 1.0f;
    mat->dif.b = 1.0f;
    mat->amb = mat->dif;
    mat->alpha = 1.0f;
}

static void 
PolygonMaterial(mgrec *polyNode, MatDef *matdef) {
    mgbool result;
    float transparency;
    FxU16 itransp;

    /* get base material value */

    if (mgGetAttList(polyNode, fltPolyMaterial, &matdef->mat.materialCode, mgNULL) &&
        (matdef->mat.materialCode < maxPredefinedMaterials)) {
        matdef->mat = *(matdef->mat.materialCode + rtMaterialsTable);
    } else {
        DefaultMaterial(&matdef->mat);
    }

    /* lightMode: 
          0 = Use face color, not illuminated 
          1 = Use vertex colors, not illuminated 
          2 = Use face color and vertex normal 
          3 = Use vertex color and vertex normal 

       N.B. Mode 3 is not supported by ATB rendering, its mapped to 2.
    */

    mgGetAttList(polyNode, fltGcLightMode, &matdef->lightMode, mgNULL);
    
    /* In MultiGen the displayed material's ambient component is the 
       product of the ambient component of the material and the face color:

       Displayed ambient (red) = Material ambient (red)* face color (red) 
       Displayed ambient (green) = Material ambient (green)* face color (green) 
       Displayed ambient (blue) = Material ambient (blue)* face color (blue)

      For example, suppose the material has an ambient component of 
      {1.0,.5,.5} and the face color is {100, 100, 100}. The displayed 
      material has as its ambient color {100, 50, 50}.  

      As with the ambient component, the diffuse component is the product 
      of the diffuse component of the material and the face color.

      ATB currently does not distinguish between diffuse and ambient and 
      so the diffuse component is computed according to the above rules and 
      is used in the material for lighting.
    */

    if (mgeGetRGB(&matdef->facet_r, &matdef->facet_g, &matdef->facet_b, 
                  polyNode, fltPolyPrimeColor, fltPolyPrimeIntensity)) {
        /* blend in color */
        if (( matdef->facet_r != 255 ) || 
            ( matdef->facet_g != 255 ) || 
            ( matdef->facet_b != 255 )) {
            matdef->mat.dif.r *= matdef->facet_r/255.0f;
            matdef->mat.dif.g *= matdef->facet_g/255.0f;
            matdef->mat.dif.b *= matdef->facet_b/255.0f;
            matdef->mat.amb = matdef->mat.dif;
        }
    }

    /* An alpha of 1.0 is fully opaque, while 0.0 is fully transparent. 
       When drawing faces, MultiGen combines the transparency value of 
       the face record with the alpha value of the material record.  The 
       final alpha applied to a face is a floating point number between 
       0.0 (transparent) and 1.0 (opaque), and is computed as follows:

        Final alpha = material alpha * (1.0 - 
                         ((face transparency * object transparency) / 65535)).
   
       Should this be 65535**2 since bothe face and object transparency are 
       0 to 65535? It looks like the material alpha is 0?
     */

    if ( mgGetAttList(polyNode, fltPolyTransparency, &itransp, mgNULL) ) {
		if ( itransp != 0 ) {
			transparency = 1.0f-itransp/65535.0f;
			if ( transparency < 0.0f )
				transparency = 0.0f;
			matdef->transparency *= transparency;
		}
	}

    matdef->transparency *= matdef->mat.alpha;

    /* get template transparency */

    result = mgGetAttList(polyNode, fltPolyMgTemplate,&matdef->template,mgNULL);

    /* get texture-map index, could be -1 (No Texture) */

    mgGetAttList(polyNode, fltPolyTexture, &matdef->textureIndex, mgNULL);

    findMaterial(matdef);
}

static void 
AddPolygon(mgrec *polyNode, float transparency) {
    Icoord coord;
    Vector vector;
    FxU32 i;
    mgrec* vrec;
    FxU8 r, g, b;
    FxBool hasVertexNormals = FXFALSE;
    FxBool hasTexCoords = FXFALSE;
    FxU8 drawType;
    FxU8 template;
    mgbool result;
    FxU8 alt_r, alt_g, alt_b;
    MatDef matdef;

    /* get material */

	matdef.transparency = transparency;
    PolygonMaterial(polyNode, &matdef);

    prim->numVerts = mgCountChild(polyNode);

    /* printf("polygon %d vertices\n", prim->numVerts); */
	
    if ( prim->numVerts < 3 )
       return ; /* NULL triangle */

    gTotalNumPolygons++;
  
    /* get draw type */

    result = mgGetAttList(polyNode, fltPolyDrawType, &drawType, mgNULL);

    /* get template transparency */

    result = mgGetAttList(polyNode, fltPolyMgTemplate, &template, mgNULL);

    result = mgeGetRGB(&alt_r, &alt_g, &alt_b, polyNode, fltPolyAltColor, 
                       fltPolyAltIntensity);

    vrec = mgGetChild(polyNode); 

    for ( i = 0; i < prim->numVerts; i++ ) {
        if (!mgIsCode(vrec, fltVertex)) {
            atuError(FXTRUE, "None vertex Node\n");
        }
        mgGetAttBuf(vrec, fltIcoord, &coord);
        prim->locations[i][0] = (float)coord.x; 
        prim->locations[i][1] = (float)coord.z;
        prim->locations[i][2] = (float)coord.y;

        /* get vertex color/ transparency */

        switch ( matdef.lightMode ) {
        case 0: /* Use face color, not illuminated */
        case 2: /* Use face color and vertex normal */
            prim->colors[i][0] = ((float) matdef.facet_r)/255.0f;
            prim->colors[i][1] = ((float) matdef.facet_g)/255.0f;
            prim->colors[i][2] = ((float) matdef.facet_b)/255.0f;
            break;
        case 1: /* Use vertex colors, not illuminated */
        case 3: /* Use vertex color and vertex normal */
            mgeGetRGB(&r, &g, &b, vrec, fltVColor, fltVIntensity);
            prim->colors[i][0] = ((float) r)/255.0f;
            prim->colors[i][1] = ((float) g)/255.0f;
            prim->colors[i][2] = ((float) b)/255.0f;
            break;
        }

        prim->colors[i][3] = matdef.transparency;
        
        /* do we have a per-vertex normal? */

        mgGetAttBuf(vrec, fltVNormal, (mgrec*)&vector);
		if (qVertexHasNormal(vector)) {
            hasVertexNormals = FXTRUE;
            prim->normals[i][0] = (float)vector.i;
            prim->normals[i][1] = (float)vector.k;
            prim->normals[i][2] = (float)vector.j;
        }
	
        /* Do we have a texture applied to the face? */

        if (matdef.atbMaterial->textures[0] != 0) {
            float vertexU, vertexV;

            mgGetAttList(vrec, fltVU, &vertexU, mgNULL);
            mgGetAttList(vrec, fltVV, &vertexV, mgNULL);
				
            hasTexCoords = FXTRUE;
            prim->texCoords[i][0] = vertexU;
            prim->texCoords[i][1] = vertexV;
 
            minU = ATM_MIN(minU, vertexU);
            minV = ATM_MIN(minV, vertexV);
            maxU = ATM_MAX(maxU, vertexU);
            maxV = ATM_MAX(maxV, vertexV);
        }
            
        vrec = mgGetNext(vrec);
    }

    prim->flags = ATS_PRIM_CCW;
    switch ( drawType ) {
    case 0: /* draw solid backfaced */
        break;
    case 1: /* draw solid no backface */
        prim->flags |= ATS_PRIM_TWO_SIDED;
        break;
    case 2: /* draw wireframe and not closed */
        return; /* not supported yet */
    case 3: /* draw closed wireframe */
        return; /* not supported yet */
    case 4: /* surround with wireframe in alternate color */
        break;
    case 8: /* omni directional light */
        return; /* not supported yet */
    case 9: /* unidirectional light */
        return; /* not supported yet */
    case 10: /* bi directional light */
        return; /* not supported yet */
    }

    prim->lBind = ATS_BIND_PER_VERTEX;
    prim->cBind = ATS_BIND_PER_VERTEX;

    if ( hasVertexNormals ) {
         prim->nBind = ATS_BIND_PER_VERTEX ;
    } else {
         prim->nBind = ATS_BIND_PER_FACET;
         prim->flags |= ATS_PRIM_GEN_FACET_NORMAL;
    } 

    prim->tBind = hasTexCoords ? ATS_BIND_PER_VERTEX : ATS_BIND_NONE;

    atsShapeMaterial(matdef.atbMaterial);

    atsShapeAddPrim();
}

/* ensure shape has a tri set with the specified material */

static void ShapeCheckMaterial(AtsNode *shape, AtsMaterial *mat) {
    int num_parts = atsShapeGetNumParts(shape);
    AtsMaterial *mtmp;
    AtsNode *gtmp;
    int i;

    for ( i = 0; i < num_parts; i++ ) {
        atsShapeGetPart(shape, i, &gtmp, &mtmp);

        if ( mtmp == mat )
            return ;
    }
    
    gtmp = (AtsNode *)atsTriSetNew();

    atsShapeAddPart(shape, gtmp, mat);
}

AtsNode *ObjectNode(mgrec* objNode) {
    mgcode code;
    char *id;
    mgrec* child;
    FVNodeType ftNodeType;
    float transparency = 1.0f;
    FxU16 itransp;

    if ( mgGetAttList(objNode, fltObjTransparency, &itransp, mgNULL) ) {
		if ( itransp != 0 ) {
			transparency = 1.0f-itransp/65535.0f;
			if ( transparency < 0.0f )
				transparency = 0.0f;
		}
	}

    prim = atsShapeBuildInit(MAX_VERTS, 0);

    if (!HasChild(objNode)) {
        return atsShapeConstruct(FXFALSE); /* empty */
    }

    child = mgGetChild(objNode);
    while ( child ) {
        id = mgGetName( child );
        code = mgGetCode(child);
        ftNodeType = GetNodeType(child);
        if ( ftNodeType != fvPolygon ) {
            atuError(FXTRUE, "none vertex in polygon %d\n", ftNodeType);            
        }
        AddPolygon(child, transparency);
        child = mgGetNext(child);
    }

    return atsShapeConstruct(FXFALSE);
}

/* collect any LOD nodes together */

static void ProcessLODNodes(AtsNode *parent, mgrec *fltNode) {
    mgrec* child;
    char *id;
    AtsNode *lod = NULL;
    AtsNode *group;
    int numLOD;
    double switchIn, switchOut;
    Icoord coord;
    AtmVector3 center;

    if (!HasChild(fltNode)) 
        return;

    id = mgGetName( fltNode );

    child = mgGetChild(fltNode);
    while ( child ) {
        FVNodeType ftNodeType = GetNodeType(child);
        id = mgGetName( child );
        if ( ftNodeType == fvLodFloat ) {
            if ( lod == NULL ) { /* first LOD node */
                if ((lod = atsLODNew()) == NULL ) {
                    atuError(FXTRUE, "Could not allocate LOD Node\n");
                }
                atsGroupAddChild(parent, lod);
                mgGetAttBuf(child, fltLodCenterPoint, (mgrec*)&coord);
                center[0] = (float)coord.x;
                center[1] = (float)coord.z;
                center[2] = (float)coord.y;
                atsLODCenter(lod, center);
                numLOD = 0;
            }
            if (( group = atsGroupNew()) == NULL ) {
                atuError(FXTRUE, "Could not allocate LOD Node child\n");
            }
            atsGroupAddChild(lod, group);
            ProcessChildren(group, child);
            if ((id = mgGetName( child )) != NULL ) {
                mgcode code = mgGetCode(child);
                atsNodeName( group, id);
            }
            mgGetAttList(child, fltLodSwitchIn, &switchIn, mgNULL);
            mgGetAttList(child, fltLodSwitchOut, &switchOut, mgNULL);

            atsLODRange(lod, numLOD, (float)switchOut);
			numLOD++;
        }
        child = mgGetNext(child);
    }
}

static void ProcessChildren(AtsNode *group, mgrec *fltNode) {
    AtsNode* node = NULL;
    char *id;

    if (HasChild(fltNode)) {
        mgrec* child;

        /* collect any LOD nodes together */

        ProcessLODNodes(group, fltNode);

        child = mgGetChild(fltNode);
        while ( child ) {
            FVNodeType ftNodeType = GetNodeType(child);
            if ( ftNodeType != fvLodFloat ) {
                id = mgGetName( child );
                node = ProcessNode(child);
                if ( node != NULL )
                    atsGroupAddChild(group, node);
            }
            child = mgGetNext(child);
        }
    }
}

static AtsNode* ProcessNode( mgrec* fltNode) {
    mgcode ourcode;
    FVNodeType ftNodeType;
    AtsNode* node = NULL;
    char* ourid;

    /* Following two lines for debugging only: */

    ourid = mgGetName( fltNode );
    ourcode = mgGetCode(fltNode);

    /* Process ourself: */

    ftNodeType = GetNodeType(fltNode);

    switch ( ftNodeType ) {
    case fvGroup:
    case fvSwitch:
        node = GroupNode(fltNode);
        break;

    case fvObject:
        node = ObjectNode(fltNode);
        break;

    case fvLodFloat:
        atuError(FXTRUE, "LOD node: should not get here\n");
        break;

    case fvDof:
        node = DofNode(fltNode);
        break;

    case fvLightSource:
        node = LightSourceNode(fltNode);
        break;

    case fvRoad:
        node = RoadNode(fltNode);
        break;

    case fvPath:
        node = PathNode(fltNode);
        break;

    case fvUnknown:
    default:
        /* atuError(FXFALSE, "unknown node type %s, %d\n", ourid, ourcode); */
        break;
    }

    if (( node != NULL ) && ( ourid != NULL )) {
        atsNodeName( node, ourid);
    }

    return node;
}

void Optimize(AtsNode *foo) {
}

AtsNode *atsLoadFromFlt(char *fileName) {
    char full_path[256];
    mgrec* CurFlightDb = NULL;
    AtsNode* root = (AtsNode*)NULL;
    mgrec* fltNode;
    FVNodeType ftNodeType ;
    mgrec* child;

    gTotalNumPolygons = 0; 
    numAtbMaterials = 0; 
    gTotalATBTextures = 0; 
    gTotalTextureBytes = 0; 

    if ( ( atuFileLocate(fileName, full_path) == NULL ) ||
         (( CurFlightDb = mgOpenDb(full_path) ) == NULL )) {
        goto end;
    }

    ftNodeType = GetNodeType(CurFlightDb);

    if ((ftNodeType == fvHeader) && HasChild(CurFlightDb)) {
        mgMatrix beadMatrix ;
        if ( mgGetMatrix( CurFlightDb, fltMatrix, &beadMatrix ) ) {
           /* TBD: do something with the matrix */
        }
        
        CreateTextureTable(CurFlightDb);
		CreateMaterialTable(CurFlightDb);
        fltNode = mgGetChild(CurFlightDb);

        /* don't currently support a root as an LOD, map it to a group */
        root = atsGroupNew();
        ProcessChildren(root, CurFlightDb);
        Optimize(root);
    }

    mgCloseDb(CurFlightDb);

    if ( root ) {
        AtmSphere bsphere;

        atsComputeBSphere(root, &bsphere);
        atsNodeBSphere(root, &bsphere);
        goto end;
    } 

end:
    if ( gVerbose ) {
        printStatistics();
    }
    return root;
}
