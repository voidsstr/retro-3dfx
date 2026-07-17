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
** $Date: 10/11/00 7:34:34 PM$ 
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <atscenep.h>
#include "sdl.h"

static char *ext = "jby";

/*-------------------------------------------------------------------
  Function: readObj
  Date: 6/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Read a JBY file
  Arguments:
    None
  Return:
    Pointer to loaded shape, or NULL if error
  -------------------------------------------------------------------*/

static AtsNode *readObj( FILE *ifp, 
   unsigned int numObjs, unsigned int numTexts, int usetextures ) {
    float r0, g0, b0, diffuse;
    unsigned int i;
    short type;
    char tname[256];
    AtrTriSet **trisets = NULL;
    AtsMaterial **mat;
    AtsTexture **text = NULL;
    AtsNode *shape;
    FxU32 *textureNum = NULL;
    AtmSphere bsphere;

    trisets = (AtrTriSet **)atuMemMalloc(numObjs* sizeof(AtrTriSet*));
    text = (AtsTexture **)atuMemMalloc(numTexts * sizeof(AtsTexture*));
    textureNum = (FxU32 *)atuMemMalloc(numObjs*sizeof(FxU32));
    mat = (AtsMaterial **)atuMemMalloc( numTexts * sizeof(AtsMaterial *) );

    if ( !trisets || !text || !textureNum || !mat )
        atuError(FXTRUE, "jbytoats: Out of memory");

    for (i=0; i<numObjs; i++) {
        fread(&textureNum[i], sizeof(unsigned long), 1, ifp);
 
        trisets[i] = atsTriSetNew();
        atrTriSetLoad(trisets[i], ifp);
    }

    for (i=0; i<numTexts; i++) {
        mat[i] = atsMaterialNew();
        fread(&type, sizeof(short), 1, ifp);
        fread(tname, sizeof(char), 256, ifp);
        fread(&r0, sizeof(float), 1, ifp);
        fread(&g0, sizeof(float), 1, ifp);
        fread(&b0, sizeof(float), 1, ifp);
        fread(&diffuse, sizeof(float), 1, ifp);

        switch(type) {
        case SDL_FC:
            atrMaterialSetup( &mat[i]->material, ATR_MAT_GSHADE );
            mat[i]->material.diffuse.r = r0;
            mat[i]->material.diffuse.g = g0;
            mat[i]->material.diffuse.b = b0;
#if 1
            mat[i]->material.emissive.r = r0 * 0.2f;
            mat[i]->material.emissive.g = g0 * 0.2f;
            mat[i]->material.emissive.b = b0 * 0.2f;
#endif
            break;
        
          case SDL_FT:
              if(tname[0] != '\0') {
                  char tmpstr[200];

                  if(!usetextures || 
                     ((text[i] = atsTextureFindOrDownload(tname)) == NULL)){
                      atuError(FXFALSE,
                        "jbytoats: Can't load texture \"%s\" ... assigning default\n", tmpstr);
                      
                      atrMaterialSetup( &mat[i]->material, ATR_MAT_GSHADE );
                      mat[i]->material.diffuse.r = 0.0f;
                      mat[i]->material.diffuse.g = 0.58f;
                      mat[i]->material.diffuse.b = 1.0f;
                  } else {
                      FxU32 typeFlag = ATR_MAT_DECAL_X_LIGHTING; 
                      if ( atsTextureHasAlpha( text[i] ) )
                         typeFlag |=  ATR_MAT_FB_BLEND;
                      atrMaterialSetup( &mat[i]->material, typeFlag ); 
                    
                      mat[i]->material.diffuse.r = 1.0f;
                      mat[i]->material.diffuse.g = 1.0f;
                      mat[i]->material.diffuse.b = 1.0f;
#if 1
                      mat[i]->material.emissive.r = 0.2f;
                      mat[i]->material.emissive.g = 0.2f;
                      mat[i]->material.emissive.b = 0.2f;
#endif
                      atsMaterialTexture(mat[i], 0, text[i]);
                  }
                } else {
                   atuError(FXFALSE, "jbytoats: FT texture specified, but no name present [%d]\n", i);
              }
              break;
          default:
              break;
        }
    }

    if(!( shape = atsShapeNew())) {
        atuError( FXTRUE, "jbytoats: Not able to allocate shape in CreateShapes\n" );
    }

    for( i = 0; i < numObjs; i++ ) {
       atsShapeAddPart(shape, (AtsObject *)trisets[i], mat[textureNum[i]]);
    }
  
    atsComputeBSphere(shape, &bsphere);

#ifdef notdef
    printf("shape = 0x%lx\n", shape);
    printf("shape bounding sphere = (%f, %f, %f) radius %f\n",
            bsphere.center[0], bsphere.center[1], bsphere.center[2], 
            bsphere.radius);
#endif

    if ( trisets )
        atuMemFree(trisets);

    if ( text )
        atuMemFree(text);

    if ( textureNum )
        atuMemFree(textureNum); 

    if ( mat )
        atuMemFree(mat);

    atsNodeBSphere(shape, &bsphere);
    fclose(ifp);
    return(shape);
}

AtsObject *atsLoadFromJBY(const char *filename) {
    FILE *fp;
    AtsNode *group, *shape;
    FxU32 numMeshes, numVerts, numPolys, numTexts, numNorms;  
    AtmSphere bsphere;
    char full_path[256];
    FxBool use_animation = FXFALSE, usetextures = FXTRUE;

    atsConverterGetIntAttr(NULL, ATS_CATTR_LOAD_TEXTURES, &usetextures);

    if ( ( atuFileLocate(filename, full_path) == NULL ) || 
         ((fp = fopen( full_path, "rb" )) == NULL )) {
        atuError( FXFALSE, "jbytoats: Not able to open JBY file \"%s\"\n", filename );
        return NULL;
    }

    fread( &numMeshes,sizeof( unsigned int ), 1, fp );
    fread( &numVerts, sizeof( unsigned int ), 1, fp );
    fread( &numNorms, sizeof( unsigned int ), 1, fp );
    fread( &numPolys, sizeof( unsigned int ), 1, fp );
    fread( &numTexts, sizeof( unsigned int ), 1, fp );
  
    shape = readObj( fp, numMeshes, numTexts, usetextures );

    if ((group = atsGroupNew()) == NULL ) {
        fprintf(stderr, "could not allocate group\n");
          exit( -1 );
    } 

    atsGroupAddChild(group, shape);
    atsComputeBSphere(group, &bsphere);
    atsNodeBSphere(group, &bsphere);
  
    return group;
}

void atsInitJBY(void) {
}

void atsTermJBY(void) {
}
