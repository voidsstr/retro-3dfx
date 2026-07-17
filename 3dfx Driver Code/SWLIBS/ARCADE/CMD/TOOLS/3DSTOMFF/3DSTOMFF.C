/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 7:29:35 PM$ 
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <dos.h>
#include <conio.h>

#include <3dfx.h>
#include <fxos.h>
#include <atrender.h>
#include <atcvt.h>
#include <3ds.h>

#include "scene.c"

int countMeshes( tds_face **faces, int numObjs, int *faceCountArray, int numMaterials ) {
    int object, face, material;
    int numUniqueMeshes = 0;
    for ( object = 0; object < numObjs; object++ ) {
        for( material = 0; material < numMaterials; material++ ) {
            for ( face = 0; face < faceCountArray[object]; face++ ) {
                if ( faces[object][face].material_index == material ) {
                    numUniqueMeshes++;
                    break;
                }
            }
        }
    }
    return numUniqueMeshes;
}

void translate3DSMaterialIntoATBMaterial( AtrMaterial *atbMat, 
                                          tds_material *tdsMat ) {

    if ( tdsMat->has_name ) {
        if ( tdsMat->does_self_illum ) {
            atbMat->r_emissive = ((float)tdsMat->ambient_color[0]) / 255.0f;
            atbMat->g_emissive = ((float)tdsMat->ambient_color[1]) / 255.0f;
            atbMat->b_emissive = ((float)tdsMat->ambient_color[2]) / 255.0f;
        }

        atbMat->r_diffuse = ((float)tdsMat->diffuse_color[0]) / 255.0f;
        atbMat->g_diffuse = ((float)tdsMat->diffuse_color[1]) / 255.0f;
        atbMat->b_diffuse = ((float)tdsMat->diffuse_color[2]) / 255.0f;
        if ( atbMat->r_diffuse != 0.0f || atbMat->g_diffuse != 0.0f ||
             atbMat->b_diffuse != 0.0f ) 
            atbMat->diffuse_contribution = 1.0f;

        atbMat->r_specular = ((float)tdsMat->specular_color[0]) / 255.0f;
        atbMat->g_specular = ((float)tdsMat->specular_color[1]) / 255.0f;
        atbMat->b_specular = ((float)tdsMat->specular_color[2]) / 255.0f;

        if ( tdsMat->has_texmap ) {
            printf( "%s has a texture and textures aren't yet supported.\n", tdsMat->name );
        }

        if ( tdsMat->two_sided ) {
            atbMat->is_two_sided = FXTRUE;
        }

        if ( tdsMat->shade_type == TDS_SHADE_FLAT ) {
            atbMat->flat_shade = FXTRUE;
        }

        if ( tdsMat->has_reflmap ) {
            printf( "%s has a reflection map and reflection maps aren't yet supported.\n", tdsMat->name );
        }

    } else { /* doesn't have name */
        /* default material is a medium gray */
        atbMat->r_diffuse = 0.5f;
        atbMat->g_diffuse = 0.5f;
        atbMat->b_diffuse = 0.5f;
    }
    return;
}

FxBool detectOrderChange( AtrTransform *t ) {
    AtmVector3 in;
    atmVector3Cross( in, &(t->data[0]), &(t->data[4]) );
    return( atmVector3Dot( in, &(t->data[8]) ) < 0.0f );
}

void translate3DSMatrixIntoAtbTransform( AtrTransform *atbT,
                                         tds_matrix *tdsT ) {
#if 1
    atbT->data[0]  = (*tdsT)[0][0];
    atbT->data[1]  = (*tdsT)[0][2];
    atbT->data[2]  = (*tdsT)[0][1];

    atbT->data[3]  = (*tdsT)[0][3];

    atbT->data[4]  = (*tdsT)[2][0];
    atbT->data[5]  = (*tdsT)[2][2];
    atbT->data[6]  = (*tdsT)[2][1];

    atbT->data[7]  = (*tdsT)[2][3];

    atbT->data[8]  = (*tdsT)[1][0];
    atbT->data[9]  = (*tdsT)[1][2];
    atbT->data[10] = (*tdsT)[1][1];

    atbT->data[11] = (*tdsT)[1][3];

    atbT->data[12] = (*tdsT)[3][0];
    atbT->data[13] = (*tdsT)[3][2];

    atbT->data[14] = (*tdsT)[3][1];

    atbT->data[15] = (*tdsT)[3][3];
#elif 0
    atbT->data[0]  = (*tdsT)[0][0];
    atbT->data[1]  = (*tdsT)[0][1];
    atbT->data[2]  = (*tdsT)[0][2];
    atbT->data[4]  = (*tdsT)[1][0];
    atbT->data[5]  = (*tdsT)[1][1];
    atbT->data[6]  = (*tdsT)[1][2];
    atbT->data[8]  = (*tdsT)[2][0];
    atbT->data[9]  = (*tdsT)[2][1];
    atbT->data[10] = (*tdsT)[2][2];
    atbT->data[12] = (*tdsT)[3][0];
    atbT->data[13] = (*tdsT)[3][1];
    atbT->data[14] = (*tdsT)[3][2];
#else
    atbT->data[0]  = 1.0f;
    atbT->data[1]  = 0.0f;
    atbT->data[2]  = 0.0f;
    atbT->data[4]  = 0.0f;
    atbT->data[5]  = 1.0f;
    atbT->data[6]  = 0.0f;
    atbT->data[8]  = 0.0f;
    atbT->data[9]  = 0.0f;
    atbT->data[10] = 1.0f;
    atbT->data[12] = 0.0f;
    atbT->data[13] = 0.0f;
    atbT->data[14] = 0.0f;
#endif
}

char *findTDSObjectName( int object, TDSFile *tdsFile ) {
    FxU32 index;
    for( index = 0; index < 65536; index++ ) {
        if ( tdsFile->key_node_to_object_index[index] == object ) {
            if ( tdsFile->key_nodes[index] && (tdsFile->key_nodes[index]->type == TDS_OBJECT_NODE_TAG) ) {
                if ( *(tdsFile->key_nodes[index]->nodes.object_node.instance_name) != 0 ) {
                    return ( tdsFile->key_nodes[index]->nodes.object_node.instance_name );
                } else {
                    return ( tdsFile->object_names[object] );
                }
            }
        }
    }
    return tdsFile->object_names[object];
}

int main( int argc, char *argv[] ) {
    FILE *infile;
    int index, object;
    static TDSFile tdsFile;
    int numMeshes, numMaterials, numTransforms;
    AtrMaterial  *materialArray;
    AtrMesh      *meshArray;
    int          meshIndex;
    AtrTransform *transformArray;
    RenderTarget *renderTargetArray;
    Scene scene;
    FILE *outfile;

    if ( argc != 3 )  {
        puts( "3dstomff infile.3ds outfile.foo\n" );
        return -1;
    }

    infile = fxFopenPath( argv[1], "rb", getenv( "AT_MODEL_PATH" ), (char**) 0 );
    if ( !infile )  {
        perror( argv[1] );
        return -1;
    }

    TDSInit( 10000 );
    TDSReadFileFromFP( &tdsFile, infile );
    fclose( infile );


    /*
    ** Allocate All Necessary Resources
    */
    numMaterials = tdsFile.num_materials;
    numMeshes = countMeshes( tdsFile.faces, 
                             tdsFile.num_n_tri_objs, 
                             tdsFile.num_faces_in_object,
                             tdsFile.num_materials );
    numTransforms = tdsFile.num_n_tri_objs;

    if ( numMaterials == 0 )
        materialArray = atrMaterialAllocate( 1 );
    else 
        materialArray = atrMaterialAllocate( numMaterials );
    transformArray = atrTransformAllocate( numTransforms );
    meshArray = atrMeshAllocate( numMeshes );
    renderTargetArray = calloc( sizeof( RenderTarget ), numMeshes );
    printf( "total_meshes: %d\n", numMeshes );

    /*
    ** Get Materials
    */
    if ( numMaterials == 0 ) {
        materialArray[0].r_diffuse = 0.5f;
        materialArray[0].g_diffuse = 0.5f;
        materialArray[0].b_diffuse = 0.5f;
    } else {
        for ( index = 0; index < numMaterials; index++ ) {
            translate3DSMaterialIntoATBMaterial( materialArray + index, 
                                                 tdsFile.materials + index );
        }
    }

    /*
    ** Get Objects
    */
    meshIndex = 0;
    for ( object = 0; object < numTransforms; object++ ) {
        int face,material;
        FxBool meshStarted;
        translate3DSMatrixIntoAtbTransform( transformArray+object,
                                            tdsFile.mesh_matrices+object );
        for ( material = 0; material < numMaterials; material++ ) {
            meshStarted = FXFALSE;
            for ( face = 0; face < tdsFile.num_faces_in_object[object]; face++ ) {
                if ( tdsFile.faces[object][face].material_index == material ) {
                    AtmVector3 v1, v2, v3;
                    AtmVector2 t1, t2, t3;
                    if ( !meshStarted ) {
                        if ( meshIndex >= numMeshes ) {
                            printf( "internal logic error.\n" );
                            exit( -1 );
                        }
                        meshStarted = FXTRUE;
                    }

#if 1
                    if ( detectOrderChange( transformArray + object ) ) {
                        v1[0] = tdsFile.local_verts[object][tdsFile.faces[object][face].v1].x;
                        v1[2] = tdsFile.local_verts[object][tdsFile.faces[object][face].v1].y;
                        v1[1] = tdsFile.local_verts[object][tdsFile.faces[object][face].v1].z;
                        t1[0] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v1].u;
                        t1[1] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v1].v;

                        v2[0] = tdsFile.local_verts[object][tdsFile.faces[object][face].v2].x;
                        v2[2] = tdsFile.local_verts[object][tdsFile.faces[object][face].v2].y;
                        v2[1] = tdsFile.local_verts[object][tdsFile.faces[object][face].v2].z;
                        t2[0] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v2].u;
                        t2[1] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v2].v;

                        v3[0] = tdsFile.local_verts[object][tdsFile.faces[object][face].v3].x;
                        v3[2] = tdsFile.local_verts[object][tdsFile.faces[object][face].v3].y;
                        v3[1] = tdsFile.local_verts[object][tdsFile.faces[object][face].v3].z;
                        t3[0] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v3].u;
                        t3[1] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v3].v;
                    } else {
                        v1[0] = tdsFile.local_verts[object][tdsFile.faces[object][face].v1].x;
                        v1[2] = tdsFile.local_verts[object][tdsFile.faces[object][face].v1].y;
                        v1[1] = tdsFile.local_verts[object][tdsFile.faces[object][face].v1].z;
                        t1[0] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v1].u;
                        t1[1] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v1].v;

                        v3[0] = tdsFile.local_verts[object][tdsFile.faces[object][face].v2].x;
                        v3[2] = tdsFile.local_verts[object][tdsFile.faces[object][face].v2].y;
                        v3[1] = tdsFile.local_verts[object][tdsFile.faces[object][face].v2].z;
                        t3[0] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v2].u;
                        t3[1] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v2].v;

                        v2[0] = tdsFile.local_verts[object][tdsFile.faces[object][face].v3].x;
                        v2[2] = tdsFile.local_verts[object][tdsFile.faces[object][face].v3].y;
                        v2[1] = tdsFile.local_verts[object][tdsFile.faces[object][face].v3].z;
                        t2[0] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v3].u;
                        t2[1] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v3].v;
                    }

#elif 0
                    v1[0] = tdsFile.local_verts[object][tdsFile.faces[object][face].v1].x;
                    v1[1] = tdsFile.local_verts[object][tdsFile.faces[object][face].v1].y;
                    v1[2] = tdsFile.local_verts[object][tdsFile.faces[object][face].v1].z;
                    t1[0] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v1].u;
                    t1[1] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v1].v;

                    v2[0] = tdsFile.local_verts[object][tdsFile.faces[object][face].v2].x;
                    v2[1] = tdsFile.local_verts[object][tdsFile.faces[object][face].v2].y;
                    v2[2] = tdsFile.local_verts[object][tdsFile.faces[object][face].v2].z;
                    t2[0] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v2].u;
                    t2[1] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v2].v;

                    v3[0] = tdsFile.local_verts[object][tdsFile.faces[object][face].v3].x;
                    v3[1] = tdsFile.local_verts[object][tdsFile.faces[object][face].v3].y;
                    v3[2] = tdsFile.local_verts[object][tdsFile.faces[object][face].v3].z;
                    t3[0] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v3].u;
                    t3[1] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v3].v;

#else
                    v1[0] = tdsFile.world_verts[object][tdsFile.faces[object][face].v1].x;
                    v1[1] = tdsFile.world_verts[object][tdsFile.faces[object][face].v1].y;
                    v1[2] = tdsFile.world_verts[object][tdsFile.faces[object][face].v1].z;
                    t1[0] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v1].u;
                    t1[1] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v1].v;

                    v2[0] = tdsFile.world_verts[object][tdsFile.faces[object][face].v2].x;
                    v2[1] = tdsFile.world_verts[object][tdsFile.faces[object][face].v2].y;
                    v2[2] = tdsFile.world_verts[object][tdsFile.faces[object][face].v2].z;
                    t2[0] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v2].u;
                    t2[1] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v2].v;

                    v3[0] = tdsFile.world_verts[object][tdsFile.faces[object][face].v3].x;
                    v3[1] = tdsFile.world_verts[object][tdsFile.faces[object][face].v3].y;
                    v3[2] = tdsFile.world_verts[object][tdsFile.faces[object][face].v3].z;
                    t3[0] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v3].u;
                    t3[1] = tdsFile.tex_verts[object][tdsFile.faces[object][face].v3].v;

#endif
                    atrMeshBeginTriangle( meshArray + meshIndex );
                    atrMeshVertex( meshArray + meshIndex, 
                                   v1[0], v1[1], v1[2], 
                                   t1[0], t1[1] );
                    atrMeshVertex( meshArray + meshIndex, 
                                   v2[0], v2[1], v2[2], 
                                   t2[0], t2[1] );
                    atrMeshVertex( meshArray + meshIndex, 
                                   v3[0], v3[1], v3[2], 
                                   t3[0], t3[1] );
                    atrMeshEndTriangle( meshArray + meshIndex );
                }
            }
            if ( meshStarted ) {
                atrMeshCalcFaceNormals( meshArray + meshIndex );
                atrMeshCalcVertexNormals( meshArray + meshIndex );
                renderTargetArray[meshIndex].materialIndex = material;
                renderTargetArray[meshIndex].transformIndex = object;
                renderTargetArray[meshIndex].meshIndex = meshIndex;
                renderTargetArray[meshIndex].name = calloc( sizeof(char),
                                                            strlen( findTDSObjectName( object, &tdsFile ) ) +
                                                            strlen( tdsFile.materials[material].name ) + 2 );
                strcpy( renderTargetArray[meshIndex].name,
                        findTDSObjectName( object, &tdsFile ) );
                strcat( renderTargetArray[meshIndex].name,
                        "." );
                strcat( renderTargetArray[meshIndex].name,
                        tdsFile.materials[material].name );
                printf( "loaded mesh: %s\n", renderTargetArray[meshIndex].name );
                meshIndex++;
            }
        }
    }

    scene.numRenderTargets = numMeshes;
    scene.numTransforms = numTransforms;
    scene.numMaterials = numMaterials;
    scene.numMeshes = numMeshes;
    scene.materialArray = materialArray;
    scene.transformArray = transformArray;
    scene.meshArray = meshArray;
    scene.renderTargetArray = renderTargetArray;

    outfile = fopen( argv[2], "wb" );
    if ( !outfile ) {
        perror( argv[2] );
        exit( -1 );
    }

    storeScene( &scene, outfile );

    return 0;
}

