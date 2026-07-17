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
** $Date: 10/11/00 7:34:39 PM$ 
**
*/

#include <stdio.h>
#include <atscenep.h>

static char *ext = "rad";
static char *converterName = "atsLoadFromRad";

#define BUF_SIZE    200    /* maximum length of a line in file */
#define MAX_VERTEX 1000    /* maximum number of vertices in a tri set */

/*-------------------------------------------------------------------
  Function: ReadMaterial
  Date: 6/8/96
  Implementor(s): mlwp after gmct
  Library: AT Scene Manager Library
  Description: 
    Read a material from a .rad file
  Arguments:
    fp  - where to read it from
    mat - material to initialize
  Return:
    Nothing
  -------------------------------------------------------------------*/

static void ReadMaterial( FILE *fp, AtsMaterial *mat ) {
    char buf[BUF_SIZE];
    char matname[BUF_SIZE];
    char texture_filename[BUF_SIZE];
    AtmVector3 diffuse_color;

    fgets( buf, sizeof(buf), fp );
    *matname = '\0';
    sscanf( buf, "material_name: %s", matname );

    fgets( buf, sizeof(buf), fp );
    *texture_filename = '\0';
    sscanf( buf, "texture_file: %s", texture_filename );

    fgets( buf, sizeof(buf), fp );
    sscanf( buf, "diffuse_color: %f %f %f", &diffuse_color[0], 
          &diffuse_color[1], &diffuse_color[2] );

#if 0
    printf( "material_name: \"%s\"\n", matname );
    printf( "texture_file: \"%s\"\n", texture_filename );
    printf( "diffuse_color: %f %f %f\n", diffuse_color[0], 
          diffuse_color[1], diffuse_color[2] );
#endif

#if 1
    mat->material.emissive.r = diffuse_color[0];
    mat->material.emissive.g = diffuse_color[1];
    mat->material.emissive.b = diffuse_color[2];
#endif

    if( *texture_filename != '\0' ) {
      atrMaterialSetup(&mat->material, 
                       ATR_MAT_DECAL_X_LIGHTING | 
                       ATR_MAT_FB_ASSIGN );
      atsMaterialTexture(mat, 0, atsTextureFindOrDownload( texture_filename ));
    }
    mat->material.irgbSrc = ATR_IRGBSRC_STATIC;
}

/*-------------------------------------------------------------------
  Function: atsLoadFromRad
  Date: 6/8/96
  Implementor(s): mlwp after gmct
  Library: AT Scene Manager Library
  Description: 
    Convert a rad file into ATB scene 
  Arguments:
    filename  - where to read it from
  Return:
    Node (a shape) representing the room read
  -------------------------------------------------------------------*/

/* put this here since some compilers don't like large local declarations */

static AtrVertex vertexList[MAX_VERTEX];

AtsObject *atsLoadFromRad(const char *filename) {
    FILE *fp;
    char buf[BUF_SIZE];
    int meshnum, vertnum, facenum;
    float u_scale, v_scale;
    int has_lightmaps;
    AtsNode *shape;
    AtmSphere bsphere;
    int num_meshes;

    if ((shape = atsShapeNew()) == NULL ) {
        atuError(FXFALSE, "%s: unable to create shape\n", converterName);
        return NULL;
    }

    if ( atuFileLocate(filename, buf) == NULL ) {
        atuError( FXFALSE, "%s: Not able to open file \"%s\"\n", 
                  converterName, filename );
        return NULL;
    }

    fp = fopen( buf, "r" );

    if( !fp ) {
        atuError( FXFALSE, "%s: Not able to open \"%s\"\n", converterName, filename );
        return NULL;
    }
  
    fgets( buf, sizeof(buf), fp );
    sscanf( buf, "num_objects: %d", &num_meshes );

    for( meshnum = 0; meshnum < num_meshes; meshnum++ ) {
        int numVertices, numFaces;
        AtsMaterial *mat;
        AtrTriSet *tset;

        mat = atsMaterialNew();
        tset = atsTriSetNew();

        atrMaterialSetup(&mat->material, ATR_MAT_GSHADE | ATR_MAT_FB_ASSIGN );
        ReadMaterial( fp, mat );

        u_scale = 1.0f;
        v_scale = 1.0f;

        fgets( buf, sizeof(buf), fp );
        sscanf( buf, "has_lightmaps: %d", &has_lightmaps );

        fgets( buf, sizeof(buf), fp );
        sscanf( buf, "num_verts: %d", &numVertices );
                
        if (numVertices > MAX_VERTEX)   {
            fprintf(stderr, "In room reading, found more than %d vertices. I'm afraid you are going to have"
                          " to recompile.... (hint: the variable is vertexList[]). \n", MAX_VERTEX);
                  exit(0);
          }     

        for ( vertnum = 0; vertnum < numVertices; vertnum++ ) {
            fgets( buf, sizeof(buf), fp );

            sscanf( buf, "%f %f %f %f %f %f %f %f\n", 
                  &vertexList[vertnum].x,
                  &vertexList[vertnum].y,
                  &vertexList[vertnum].z,
                  &vertexList[vertnum].s0,
                  &vertexList[vertnum].t0,
                  &vertexList[vertnum].r,
                  &vertexList[vertnum].g,
                  &vertexList[vertnum].b );

#if 1
                  if( !has_lightmaps && ( !mat->textures[0] ) )
                    {
                      vertexList[vertnum].r *= mat->material.emissive.r;
                      vertexList[vertnum].g *= mat->material.emissive.g;
                      vertexList[vertnum].b *= mat->material.emissive.b;
                    }
#endif

                  vertexList[vertnum].r /= 255.0f;
                  vertexList[vertnum].g /= 255.0f;
                  vertexList[vertnum].b /= 255.0f;

                  vertexList[vertnum].t0 = 1.0f - vertexList[vertnum].t0;
                  vertexList[vertnum].s0 *= u_scale;
                                  vertexList[vertnum].t0 *= v_scale;
        }

        fgets( buf, sizeof(buf), fp );
        sscanf( buf, "num_faces: %d", &numFaces );

        atrTriSetBegin( tset );

        for( facenum = 0; facenum < numFaces; facenum++ ) {
            int tmp[3];

            fgets( buf, sizeof(buf), fp );
            sscanf( buf, "%d %d %d\n", &tmp[0], &tmp[1], &tmp[2] );
                        
            atrTriSetVertex( tset, &vertexList[tmp[0]] );
            atrTriSetVertex( tset, &vertexList[tmp[1]] );
            atrTriSetVertex( tset, &vertexList[tmp[2]] );
        }
        

        if( has_lightmaps ) {
            //---------------------------------------------------------
            //  Configure material for lightmaps.
            //---------------------------------------------------------

            if ( mat->material.texSrc[0] == ATR_TEXSRC_DECAL )
                atrMaterialSetup( &mat->material, ATR_MAT_DECAL );
            mat->successor = atsMaterialNew( );
            mat->material.successor = &mat->successor->material;

            atrMaterialSetup( &mat->successor->material, ATR_MAT_LMAP | ATR_MAT_FB_MULTIPLY );

            mat->successor->material.lmCoords[0][0] = 0.0f;
            mat->successor->material.lmCoords[0][1] = 1.0f;
            mat->successor->material.lmCoords[1][0] = 1.0f;
            mat->successor->material.lmCoords[1][1] = 1.0f;
            mat->successor->material.lmCoords[2][0] = 0.5f;
            mat->successor->material.lmCoords[2][1] = 0.0f;

            mat->successor->material.texSClamp[0] = ATR_TEXCLAMP_CLAMP;
            mat->successor->material.texTClamp[0] = ATR_TEXCLAMP_CLAMP;


            //---------------------------------------------------------
            //  Load lightmap pointers into triset
            //---------------------------------------------------------
            for( facenum = 0; facenum < numFaces; facenum++ ) {
                static char lmName[256];
                static char tmpStr[256];
                fgets( buf, sizeof(buf), fp );
                sscanf( buf, "%s", tmpStr );
                atrTriSetLightMap( tset, ((AtsTexture *)atsTextureDownload(tmpStr))->handle);
            }
        }
        atrTriSetCalcNormals();
        atrTriSetEnd( tset );
        atsShapeAddPart(shape, (AtsNode *)tset, mat);
    }

    fclose( fp );

    atsComputeBSphere(shape, &bsphere);
   
    atsNodeBSphere(shape, &bsphere);

    atsNodeName(shape, filename);

    return shape;
}

/*-------------------------------------------------------------------
  Function: atsInitRad
  Date: 6/8/96
  Implementor(s): mlwp after gmct
  Library: AT Scene Manager Library
  Description: 
    Initialize the rad converter
  Arguments:
    None
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsInitRad(void) {
}

/*-------------------------------------------------------------------
  Function: atsTermRad
  Date: 6/8/96
  Implementor(s): mlwp after gmct
  Library: AT Scene Manager Library
  Description: 
    Terminate the rad converter
  Arguments:
    None
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsTermRad(void) {
}
