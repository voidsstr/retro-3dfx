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
** $Date: 10/11/00 7:34:45 PM$ 
**
*/
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <3ds.h>
#include "atscenep.h"

static char *ext = "3ds";
static FxBool tds_inited = FXFALSE;

static AtsNode **shapes;

static float vertex_scale = 1.0f;
static int texture_wrap_fix = FXFALSE;
static int verbose = FXFALSE;

#define UWRAP 0x0008
#define VWRAP 0x0010

static void CreateTriangle( AtrTriSet *triset, int has_texmap, short flags,
                     AtrVertex *tVertex1, AtrVertex *tVertex2, AtrVertex *tVertex3) {
    float d;
    float maxu,minu;
    float maxv,minv;

    if ( texture_wrap_fix && has_texmap ) {
        if ( flags & UWRAP ) {
            maxu = minu = tVertex1->s0;
            if (tVertex2->s0>maxu) maxu = tVertex2->s0;
            else if (tVertex2->s0<minu) minu = tVertex2->s0; 
            if (tVertex3->s0>maxu) maxu = tVertex3->s0;
            else if (tVertex3->s0<minu) minu = tVertex3->s0; 
            if ((maxu-minu)>0.8) {
                d = (float)ceil(maxu-minu);
                if (tVertex1->s0<.5)  tVertex1->s0 += d;
                if (tVertex2->s0<.5)  tVertex2->s0 += d;
                if (tVertex3->s0<.5)  tVertex3->s0 += d;
            }
        }
        
        if ( flags & VWRAP ) {
            maxv = minv = tVertex1->t0;
            if (tVertex2->t0>maxv) maxv = tVertex2->t0;
            else if (tVertex2->t0<minv) minv = tVertex2->t0; 
            if (tVertex3->t0>maxv) maxv = tVertex3->t0;
            else if (tVertex3->t0<minv) minv = tVertex3->t0; 
            if ((maxv-minv)>0.8) {
                d = (float)ceil(maxv-minv);
                if (tVertex1->t0<.5)  tVertex1->t0 += d;
                if (tVertex2->t0<.5)  tVertex2->t0 += d;
                if (tVertex3->t0<.5)  tVertex3->t0 += d;
            }
        }
    }

    atrTriSetVertex( triset, tVertex1 ); 
    atrTriSetVertex( triset, tVertex2 ); 
    atrTriSetVertex( triset, tVertex3 ); 
}
    
static void ScaleVertices(TDSFile *tdsfile) {
    tds_point *verts, *lverts;
    tds_tex_vert *tex_verts;
    int i, j, num_verts;
    float min_u = FLT_MAX, max_u = -FLT_MAX;
    float min_v = FLT_MAX, max_v = -FLT_MAX;
    float min_x = FLT_MAX, max_x = -FLT_MAX;
    float min_y = FLT_MAX, max_y = -FLT_MAX;
    float min_z = FLT_MAX, max_z = -FLT_MAX;

    for ( i = 0; i < tdsfile->num_n_tri_objs; i++ ) {
        num_verts = tdsfile->num_verts_in_object[i];
        verts = tdsfile->world_verts[i];
        lverts = tdsfile->local_verts[i];
        tex_verts =  tdsfile->tex_verts[i];

        for ( j = 0; j < num_verts; j++ ) {
            verts[j].x *= vertex_scale;
            verts[j].y *= vertex_scale;
            verts[j].z *= vertex_scale;

            lverts[j].x *= vertex_scale;
            lverts[j].y *= vertex_scale;
            lverts[j].z *= vertex_scale;

            min_u = ATM_MIN(min_u, tex_verts[j].u);
            max_u = ATM_MAX(max_u, tex_verts[j].u);

            min_v = ATM_MIN(min_v, tex_verts[j].v);
            max_v = ATM_MAX(max_v, tex_verts[j].v);

            min_x = ATM_MIN(min_x, verts[j].x);
            max_x = ATM_MAX(max_x, verts[j].x);

            min_y = ATM_MIN(min_y, verts[j].y);
            max_y = ATM_MAX(max_y, verts[j].y);

            min_z = ATM_MIN(min_z, verts[j].z);
            max_z = ATM_MAX(max_z, verts[j].z);
        }
    }

    if ( verbose ) {
        printf("texture coordinate range U (%f, %f) V (%f, %f)\n",
                min_u, max_u, min_v, max_v);

        printf("coordinate range X (%f, %f) Y (%f, %f) Z (%f, %f)\n",
                min_x, max_x, min_y, max_y, min_z, max_z);
    }
}

static int detectOrderChange( float *t ) {
    AtmVector3 in;

    atmVector3Cross( in, &(t[0]), &(t[4]) );
    return ( atmVector3Dot( in, &(t[8]) ) < 0.0f );
}

static void translate3DSMatrixIntoAtbXform( AtrXform *atbT, tds_matrix *tdsT ) {
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

    atbT->data[12] = (*tdsT)[3][0]*vertex_scale;
    atbT->data[13] = (*tdsT)[3][2]*vertex_scale;

    atbT->data[14] = (*tdsT)[3][1]*vertex_scale;

    atbT->data[15] = (*tdsT)[3][3];
    return;
}


static void ComputeStaticBounds( TDSFile *tdsfile, AtmSphere *bsphere) {
    tds_point *verts;
    tds_point center;
    int i, j, count, num_verts;
    float radius, d, dx, dy, dz;

    center.x = center.y = center.z = 0.0f;
    count = 0;

    for ( i = 0; i < tdsfile->num_n_tri_objs; i++ ) {
        num_verts = tdsfile->num_verts_in_object[i];
        verts = tdsfile->world_verts[i];

        for ( j = 0; j < num_verts; j++ ) {
            center.x += verts[j].x;
            center.y += verts[j].y;
            center.z += verts[j].z;
            count++;
        }
    }

    center.x /= count;
    center.y /= count;
    center.z /= count;
          
    radius = 0.0f;

    for ( i = 0; i < tdsfile->num_n_tri_objs; i++ ) {
        num_verts = tdsfile->num_verts_in_object[i];
        verts = tdsfile->world_verts[i];

        for ( j = 0; j < num_verts; j++ ) {
            dx = verts[j].x-center.x;
            dy = verts[j].y-center.y;
            dz = verts[j].z-center.z;
            d = dx*dx+dy*dy+dz*dz;
            if ( d > radius ) radius = d;
        }
    }
  
    bsphere->center[0] = center.x;
    bsphere->center[1] = center.y;
    bsphere->center[2] = center.z;

    bsphere->radius = (float)sqrt(radius);
}

/* The animation consists of a set of instances (references to shapes)
 * each instance has a name, a set of transformations (one for each frame)
 * and a pointer to the object it is instancing. Since, in the interests
 * of performance we have flattened the tree we can have nodes which don't
 * point anywhere (they just used to have heirarchy beneath them). To keep
 * the group code happy we create a dummy shape here to represent this
 * empty instance.
 */

static void CreateInstances( TDSFile *tdsfile, AtsNode *anim )
{
    int i;
    int node_index = 0;
    int num_nodes = 0;
    int frame;
    int num_frames;
    int total_transforms;
    AtrXform *xforms, *txforms;
    AtsNode *null_shape = NULL;
    char **instance_names;
    char *name;

    /*
     * Figure out how many keyframed objects there are in the 3ds file.
     */

    num_frames = tdsfile->animation_length;

    num_nodes = 0;

    for ( i = 0; i < 65536; i++ ) {
        if ( tdsfile->key_nodes[i] && 
           ( tdsfile->key_nodes[i]->type == TDS_OBJECT_NODE_TAG ) ) {
            num_nodes++;
        }
    }

    total_transforms = num_nodes*num_frames;

    if (!(xforms = atrXformAllocate(total_transforms))) {
        atuError( FXTRUE, "Not able to allocate xforms\n" );
    }

    /*
     * Iterate through each frame of animation, filling in the world matrices
     * for each node.
     */

    txforms = xforms;

    for ( frame = 0; frame < num_frames; frame++ ) {
        node_index = 0;
        for( i = 0; i < 65536; i++ ) {
            if( tdsfile->key_nodes[i] && 
                ( tdsfile->key_nodes[i]->type == TDS_OBJECT_NODE_TAG ) ) {
                /*
                 * Copy the world matrix for this object out of the internal 3ds
                 * data structures.
                 */
                translate3DSMatrixIntoAtbXform( txforms, &tdsfile->world_matrices[i][frame] );
                txforms++;
                node_index++;
            }
        }
    }

    /*
     * Build the reference between nodes and the objects which they contain.
     * This list is NULL terminated.
     */

    if (!(instance_names = atuMemCalloc(num_nodes+1, sizeof(char *)))) {
        atuError( FXTRUE, "CreateInstances: out of memory.\n" );
    }

    instance_names[num_nodes] = NULL;

    node_index = 0;

    for ( i = 0; i < 65536; i++ ) {
       if( tdsfile->key_nodes[i] && 
         ( tdsfile->key_nodes[i]->type == TDS_OBJECT_NODE_TAG ) ) {
          if ( tdsfile->key_node_to_object_index[i] == -1 ) {
              /* use NULL shape */
              if (( null_shape == NULL ) && (!( null_shape = atsShapeNew()))) {
                  atuError( FXTRUE, "CreateInstances: out of memory.\n" );
              }
              atsGroupAddChild(anim, null_shape);
           } else atsGroupAddChild(anim, 
                           shapes[tdsfile->key_node_to_object_index[i]]);


          if( tdsfile->key_nodes[i]->nodes.object_node.instance_name[0] != '\0' ){
            name = tdsfile->key_nodes[i]->nodes.object_node.instance_name;
          } else {
            name = tdsfile->object_names[tdsfile->key_node_to_object_index[i]];
          }

          if (!(instance_names[node_index] = strdup(name))) {
              atuError( FXTRUE, "CreateInstances: out of memory.\n" );
          }

          node_index++;
        }
    }
  
    atsAnimInstanceNames(anim, instance_names);
  
    atsAnimXforms(anim, (FxU16)num_frames, xforms);
}

static void CreateShapes( AtsNode *group, TDSFile *tdsfile, FxBool usetextures,
                          FxBool use_animation)
{
    int i;
    tds_point *verts;
    tds_point *vert_normals;
    tds_tex_vert *tex_verts;
    tds_face *faces;
    int ov1, ov2, ov3;
    int num_objects = tdsfile->num_n_tri_objs;
    AtsMaterial **materials, *material ;
    int *mat_list;
    AtmSphere bsphere;
    int has_texmap;

    if ( use_animation ) {
        if (!(shapes = (AtsNode **)atuMemMalloc(num_objects*sizeof(AtsShape *)))) {
            atuError( FXTRUE, "Not able to allocate shapes in CreateShapes\n" );
        }
    }

    if (!(materials = atuMemMalloc(tdsfile->num_materials*sizeof(AtsMaterial *)))) {
        atuError( FXTRUE, "Not able to allocate materials in CreateShapes\n" );
    }

    if (!(mat_list = atuMemMalloc(tdsfile->num_materials*sizeof(int)))) {
        atuError( FXTRUE, "Not able to allocate materials in CreateShapes\n" );
    }

    /*
     * Assign values to the materials.
     */

    for ( i = 0; i < tdsfile->num_materials; i++ ) {

        if (!(material = atsMaterialNew() ) ) {
            atuError( FXTRUE, 
                      "Not able to allocate materials in CreateShapes\n" );
        }

        materials[i] = material;

        if ( tdsfile->materials[i].has_texmap && usetextures ) {
            FxU32 typeFlag = ATR_MAT_DECAL_X_LIGHTING; 
            AtsTexture *pTexture = atsTextureFindOrDownload(
                                         tdsfile->materials[i].texmap.mapname );

            if ( atsTextureHasAlpha( pTexture ) )
                 typeFlag |=  ATR_MAT_FB_BLEND;
#ifdef notdef
#endif

            atsMaterialTexture(material, 0, pTexture );
            atrMaterialSetup(&material->material, typeFlag);
            material->material.diffuse.r  = 1.0f;
            material->material.diffuse.g  = 1.0f;
            material->material.diffuse.b  = 1.0f;
            material->material.emissive.r = 0.6f;
            material->material.emissive.g = 0.6f;
            material->material.emissive.b = 0.6f;
        } else if( tdsfile->materials[i].has_reflmap && usetextures ) {
            atsMaterialTexture( material, 0, 
                                atsTextureFindOrDownload(tdsfile->materials[i].reflmap.mapname ));

            atrMaterialSetup( &material->material, ATR_MAT_EMAP );
        } else {
            atrMaterialSetup( &material->material, ATR_MAT_GSHADE );
            material->material.emissive.r = 
              ( float )tdsfile->materials[i].ambient_color[0] / 255.0f;
            material->material.emissive.g = 
              ( float )tdsfile->materials[i].ambient_color[1] / 255.0f;
            material->material.emissive.b = 
              ( float )tdsfile->materials[i].ambient_color[2] / 255.0f;
            
            material->material.diffuse.r = 
              ( float )tdsfile->materials[i].diffuse_color[0] / 255.0f;
            material->material.diffuse.g = 
              ( float )tdsfile->materials[i].diffuse_color[1] / 255.0f;
            material->material.diffuse.b = 
              ( float )tdsfile->materials[i].diffuse_color[2] / 255.0f;
        }
    }

    /*
     * Create a shape for each of the objects in the 3ds file.
     * Not that these shapes they do 
     * not have a one to one mapping with the hierarchy in the 3ds file
     * and may be used more than once in the model.
     */

    for ( i = 0; i < num_objects; i++ ) {
        AtsNode *shape;
        int j;
        int material_index = -1;
        int order_change ;
        AtrTriSet *triset = NULL;
        FxBool more_meshes;

        if (!( shape = atsShapeNew())) {
            atuError( FXTRUE, "Not able to allocate shape in CreateShapes\n" );
        }

        if ( use_animation )
             shapes[i] = shape;
        else atsGroupAddChild(group, shape);

        order_change = ( use_animation && 
                         detectOrderChange( (float*)(tdsfile->mesh_matrices[i]) ) );

        /*
         * Save the name of the object.
         */

        atsNodeName( shape, tdsfile->object_names[i] );

        /*
         * Set up pointers to some useful data structures within
         * the internal 3DS stuff.
         */

        if ( use_animation )
             verts =           tdsfile->local_verts[i];
        else verts =           tdsfile->world_verts[i];
      
        vert_normals =    tdsfile->vert_normals[i];
        tex_verts =       tdsfile->tex_verts[i];
        faces =           tdsfile->faces[i];

        /* create triangle meshes for shape */

        more_meshes = FXTRUE;

        for ( j = 0; j < tdsfile->num_materials; j++ )
            mat_list[j] = 0;

        while ( more_meshes ) {
            material = NULL;
            more_meshes = FXFALSE;
            
            for ( j = 0; j < tdsfile->num_faces_in_object[i]; j++ ) {
                AtrVertex tVertex1, tVertex2, tVertex3;

                memset( &tVertex1, 0, sizeof( AtrVertex ) );
                memset( &tVertex2, 0, sizeof( AtrVertex ) );
                memset( &tVertex3, 0, sizeof( AtrVertex ) );
                
                /* Is this the first vertex of this mesh? */
                
                if ( material == NULL ) {
                    /* have we processed this material yet for this shape */
                    if (mat_list[faces[j].material_index] != 0 ) 
                      continue;
                    more_meshes = FXTRUE;
                    mat_list[faces[j].material_index] = 1;
                    triset = atsTriSetNew();
                    atrTriSetBegin( triset );
                    material = materials[faces[j].material_index];
                    material_index = faces[j].material_index;
                } else if (faces[j].material_index != material_index ) 
                  continue;
                
                ov1 = faces[j].v1;
                ov2 = faces[j].v3;
                ov3 = faces[j].v2;
                
                
                if ( order_change ) {
                    /* Vertex v1 */
                    tVertex1.i = -vert_normals[ov1].x;
                    tVertex1.k = vert_normals[ov1].y;
                    tVertex1.j = vert_normals[ov1].z;
                    if ( tdsfile->materials[faces[j].material_index].has_texmap ) {
                        tVertex1.s0 = tex_verts[ov1].u ;
                        tVertex1.t0 = 1.0f - tex_verts[ov1].v;
                    }
                    tVertex1.x = -verts[ov1].x;
                    tVertex1.z = verts[ov1].y;
                    tVertex1.y = verts[ov1].z;
                    
                    /* Vertex v2 */
                    tVertex2.i = -vert_normals[ov2].x;
                    tVertex2.k =  vert_normals[ov2].y;
                    tVertex2.j =  vert_normals[ov2].z;
                    if ( tdsfile->materials[faces[j].material_index].has_texmap ) {
                        tVertex2.s0 = tex_verts[ov2].u;
                        tVertex2.t0 = 1.0f - tex_verts[ov2].v;
                    }
                    tVertex2.x = -verts[ov2].x;
                    tVertex2.z = verts[ov2].y;
                    tVertex2.y = verts[ov2].z;
                    
                    /* Vertex v3 */
                    tVertex3.i = -vert_normals[ov3].x;
                    tVertex3.k = vert_normals[ov3].y;
                    tVertex3.j = vert_normals[ov3].z;
                    has_texmap = tdsfile->materials[faces[j].material_index].has_texmap;
                    if ( tdsfile->materials[faces[j].material_index].has_texmap ) {
                        tVertex3.s0 = tex_verts[ov3].u;
                        tVertex3.t0 = 1.0f - tex_verts[ov3].v;
                    }
                    tVertex3.x = -verts[ov3].x;
                    tVertex3.z = verts[ov3].y;
                    tVertex3.y = verts[ov3].z;
                    CreateTriangle(triset, has_texmap, faces[j].flags, &tVertex1, &tVertex2, &tVertex3);
                } else {
                    /* Vertex v1 */
                    tVertex1.i = vert_normals[ov1].x;
                    tVertex1.k = vert_normals[ov1].y;
                    tVertex1.j = vert_normals[ov1].z;
                    if ( tdsfile->materials[faces[j].material_index].has_texmap ) {
                        tVertex1.s0 = tex_verts[ov1].u;
                        tVertex1.t0 = 1.0f - tex_verts[ov1].v;
                    }
                    tVertex1.x = verts[ov1].x;
                    tVertex1.z = verts[ov1].y;
                    tVertex1.y = verts[ov1].z;
                    
                    /* Vertex v2 */
                    tVertex2.i = vert_normals[ov2].x;
                    tVertex2.k = vert_normals[ov2].y;
                    tVertex2.j = vert_normals[ov2].z;
                    if ( tdsfile->materials[faces[j].material_index].has_texmap ) {
                        tVertex2.s0 = tex_verts[ov2].u;
                        tVertex2.t0 = 1.0f - tex_verts[ov2].v;
                    }
                    tVertex2.x = verts[ov2].x;
                    tVertex2.z = verts[ov2].y;
                    tVertex2.y = verts[ov2].z;
                    
                    /* Vertex v3 */
                    tVertex3.i = vert_normals[ov3].x;
                    tVertex3.k = vert_normals[ov3].y;
                    tVertex3.j = vert_normals[ov3].z;
                    has_texmap = tdsfile->materials[faces[j].material_index].has_texmap;
                    if ( tdsfile->materials[faces[j].material_index].has_texmap ) {
                        tVertex3.s0 = tex_verts[ov3].u;
                        tVertex3.t0 = 1.0f - tex_verts[ov3].v;
                    }
                    tVertex3.x = verts[ov3].x;
                    tVertex3.z = verts[ov3].y;
                    tVertex3.y = verts[ov3].z;
                    CreateTriangle(triset, has_texmap, faces[j].flags, &tVertex1, &tVertex2, &tVertex3);
                }
            }
     
            if ( material != NULL ) {
                atrTriSetCalcNormals();
                atrTriSetEnd( triset );
                atsShapeAddPart(shape, (AtsObject *)triset, material);
            }
        }
            
        atsComputeBSphere(shape, &bsphere);
        
        atsNodeBSphere(shape, &bsphere);
    }
    
    atuMemFree(materials);
    atuMemFree(mat_list);
}

static AtmCMesh *atsLoadCMeshFrom3DS( const char *filename) {
    static TDSFile  tdsfile;
    char full_path[256];
    AtmCMesh *cmesh;
    AtmVector3 v0, v1, v2;
    int objnum, facenum;
    float grid_unit_size = 0.0f;

    atsConverterGetFloatAttr(NULL, ATS_CATTR_CMESH_GRID_SIZE, &grid_unit_size) ;

    if ( ( atuFileLocate(filename, full_path) == NULL ) || 
         ( TDSReadFile( &tdsfile, full_path ) == 0 )) {
       atuError(FXFALSE, "can't open 3d studio file %s\n", filename);
       return NULL;
    }

    TDSConvertToATBCoordSystem( &tdsfile );

    if ((cmesh = atsCMeshNew()) == NULL ) {
        atuError(FXFALSE, "out of memory\n");
        return NULL;
    }
 
    if ( grid_unit_size != 0.0 )
       atmCMeshGridSize(cmesh, grid_unit_size);

    for( objnum = 0; objnum < tdsfile.num_n_tri_objs; objnum++ ) {
        tds_point *tdsverts;
        tds_face *tdsfaces;

        tdsverts = tdsfile.world_verts[objnum];
        tdsfaces = tdsfile.faces[objnum];
        for( facenum = 0; facenum < tdsfile.num_faces_in_object[objnum]; facenum++ ) {
            v0[0] = tdsverts[tdsfaces[facenum].v1].x;
            v0[1] = tdsverts[tdsfaces[facenum].v1].y;
            v0[2] = tdsverts[tdsfaces[facenum].v1].z;
            v1[0] = tdsverts[tdsfaces[facenum].v2].x;
            v1[1] = tdsverts[tdsfaces[facenum].v2].y;
            v1[2] = tdsverts[tdsfaces[facenum].v2].z;
            v2[0] = tdsverts[tdsfaces[facenum].v3].x;
            v2[1] = tdsverts[tdsfaces[facenum].v3].y;
            v2[2] = tdsverts[tdsfaces[facenum].v3].z;

            if (!atmCMeshAddTriangle(cmesh, v0, v1, v2) ) {
                atuError(FXFALSE, "Error adding triangle to mesh\n");
                return NULL;
            }
        }
    }

    if ( atmCMeshClose(cmesh) )
        return cmesh;
    else {
        atuError(FXFALSE, "Error creating collision grid\n");
        return NULL;
    }
}


AtsObject *atsLoadFromSeq3DS( const char *filename ) {
    AtmBox bbox;
    int i;
    AtsNode *group, *c;
    static TDSFile  tdsfile;
    AtmSphere bsphere;
    char full_path[256];
    FxBool use_animation = FXFALSE, usetextures = FXTRUE;
    AtsType *targetType;
    int startFrame = 0, endFrame = 0;
    FxBool loadSequence = FXFALSE;
    int frameNumber;
    char buff[80];


    if ( !atsConverterGetIntAttr(NULL, ATS_CATTR_VERBOSE, &verbose) )
        verbose = FXFALSE;

    if ( !atsConverterGetFloatAttr(NULL, ATS_CATTR_VERTEX_SCALE, &vertex_scale) )
        vertex_scale = 1.0f;

    if ( verbose )
        printf("vertex scale %f\n", vertex_scale);

    if ( !atsConverterGetIntAttr(NULL, ATS_CATTR_TEXTURE_WRAP_FIX,&texture_wrap_fix) )
        texture_wrap_fix = FXFALSE;

    if ( verbose )
        printf("texture wrapping fix %s\n", texture_wrap_fix ? "enabled":
                                                               "disabled");

    if ( !atsConverterGetIntAttr(NULL, ATS_CATTR_LOAD_TEXTURES, &usetextures))
        usetextures = FXTRUE;

    if ( verbose )
        printf("loading textures %s\n", usetextures?"enabled":"disabled");
    
    /*
    if ( atsConverterGetAttr(NULL, ATS_CATTR_TARGET_TYPE, (void *)&targetType) ) {
        if (atsIsSubClassOf(targetType, atsGetTypeFromName("Anim"))) {
            use_animation = FXTRUE; 
        }

        if (atsIsSubClassOf(targetType, atsGetTypeFromName("CMesh"))) {
            return atsLoadCMeshFrom3DS( filename);
        }
    }
    */


    /* TDSConvertToATBCoordSystem( &tdsfile ); */

    /*
     * Move the coords in the 3ds file from world to local and build
     * the matrices for each frame of animation for each object.
     */

    /*
    if ((group = atsAnimNew()) == NULL ) {
        atuError( FXTRUE, "could not allocate anim\n");
    }
    */

    if ((group = atsSeqNew()) == NULL ) {
         atuError(FXFALSE, " could not allocate group for seq\n" );
         return NULL;
    }

    if ( atsConverterGetAttr(NULL, ATS_CATTR_TARGET_TYPE, (void *)&targetType) ) {
        if (atsIsSubClassOf(targetType, atsGetTypeFromName("Seq"))) {
            loadSequence = FXTRUE;
        }
    }

    if ( loadSequence ) {
        atsConverterGetIntAttr(NULL, ATS_CATTR_START_FRAME, &startFrame);
        atsConverterGetIntAttr(NULL, ATS_CATTR_END_FRAME, &endFrame);
        for ( frameNumber= startFrame; frameNumber<= endFrame; frameNumber++ ) {
            sprintf(buff, filename, frameNumber);

            if ( ( atuFileLocate(buff, full_path) == NULL ) || 
                 ( TDSReadFile( &tdsfile, full_path ) == 0 )) {
               atuError(FXFALSE, "can't open 3d studio file %s\n", filename);
               return NULL;
            }

            /*c = Process_File ( buff, (frameNumber == startFrame) ) ;*/
			if (( c = atsAnimNew()) == NULL ) {
				atuError( FXTRUE, "could not allocate anim\n");
		    }

            ScaleVertices(&tdsfile);
            /*
             * Calculate the bounding boxes for the geometry.
             */
            TDSCalcBoundingBoxes( &tdsfile );
            /*
             * Recalculate the vertex normals now that the geometry is correct.
             */
            for( i = 0; i < tdsfile.num_n_tri_objs; i++ ) {
                TDSCalcFaceNormals( &tdsfile, i );
                TDSCalcVertexNormals( &tdsfile, i );
            }
            /*
             * Create the static objects which make up the parts which are 
             * referred to in the hierarchy of the 3ds model.
             */
            CreateShapes( c, &tdsfile, usetextures, use_animation );
        
            ComputeStaticBounds( &tdsfile, &bsphere);
            atsNodeBSphere( c, &bsphere);
            atsComputeBSphere( c, &bsphere);
            atsNodeBSphere( c, &bsphere);
            atsComputeBBox( c, &bbox);

            if ( c != NULL ) {
                atsGroupAddChild( group, c);
            } else {
                atuError(FXFALSE, "error in processing file %s\n", buff);
                return NULL;
            }
        }
    } else {
            //Process_File ( filename, FXTRUE ) ;
            ScaleVertices(&tdsfile);
            /*
             * Calculate the bounding boxes for the geometry.
             */
            TDSCalcBoundingBoxes( &tdsfile );
            /*
             * Recalculate the vertex normals now that the geometry is correct.
             */
            for( i = 0; i < tdsfile.num_n_tri_objs; i++ ) {
                TDSCalcFaceNormals( &tdsfile, i );
                TDSCalcVertexNormals( &tdsfile, i );
            }
            /*
             * Create the static objects which make up the parts which are 
             * referred to in the hierarchy of the 3ds model.
             */
            CreateShapes( group, &tdsfile, usetextures, use_animation );
        
            ComputeStaticBounds( &tdsfile, &bsphere);
            atsNodeBSphere(group, &bsphere);
            atsComputeBSphere(group, &bsphere);
            atsNodeBSphere(group, &bsphere);
            atsComputeBBox(group, &bbox);
            return group;
    }
    return group;
}


AtsObject *atsLoadFrom3DS( const char *filename ) {
    AtmBox bbox;
    int i;
    AtsNode *group;
    static TDSFile  tdsfile;
    AtmSphere bsphere;
    char full_path[256];
    FxBool use_animation = FXFALSE, usetextures = FXTRUE;
    AtsType *targetType;


	// Denis Added this to allow to treat a bunch of 3ds files as a sequence
	// just like for the rtg files.
    if ( atsConverterGetAttr(NULL, ATS_CATTR_TARGET_TYPE, (void *)&targetType) ) {
        if (atsIsSubClassOf(targetType, atsGetTypeFromName("Seq"))) {
            return atsLoadFromSeq3DS( filename );
        }
    }


    if ( !atsConverterGetIntAttr(NULL, ATS_CATTR_VERBOSE, &verbose) )
        verbose = FXFALSE;

    if ( !atsConverterGetFloatAttr(NULL, ATS_CATTR_VERTEX_SCALE, &vertex_scale) )
        vertex_scale = 1.0f;

    if ( verbose )
        printf("vertex scale %f\n", vertex_scale);

    if ( !atsConverterGetIntAttr(NULL, ATS_CATTR_TEXTURE_WRAP_FIX,&texture_wrap_fix) )
        texture_wrap_fix = FXFALSE;

    if ( verbose )
        printf("texture wrapping fix %s\n", texture_wrap_fix ? "enabled":
                                                               "disabled");

    if ( !atsConverterGetIntAttr(NULL, ATS_CATTR_LOAD_TEXTURES, &usetextures))
        usetextures = FXTRUE;

    if ( verbose )
        printf("loading textures %s\n", usetextures?"enabled":"disabled");
    
    if ( atsConverterGetAttr(NULL, ATS_CATTR_TARGET_TYPE, (void *)&targetType) ) {
        if (atsIsSubClassOf(targetType, atsGetTypeFromName("Anim"))) {
            use_animation = FXTRUE; 
        }

        if (atsIsSubClassOf(targetType, atsGetTypeFromName("CMesh"))) {
            return atsLoadCMeshFrom3DS( filename);
        }
    }

    if ( ( atuFileLocate(filename, full_path) == NULL ) || 
         ( TDSReadFile( &tdsfile, full_path ) == 0 )) {
       atuError(FXFALSE, "can't open 3d studio file %s\n", filename);
       return NULL;
    }

    /* TDSConvertToATBCoordSystem( &tdsfile ); */

    /*
     * Move the coords in the 3ds file from world to local and build
     * the matrices for each frame of animation for each object.
     */

    if( use_animation ) {
        if( tdsfile.has_animation ) {
            TDSUseAnimation( &tdsfile );  
        } else {
            use_animation = FXFALSE;
            atuError( FXFALSE, "WARNING: Trying to animation in a 3DS file that has no keyframe data.\n" );
        }
    }

    if ((group = atsAnimNew()) == NULL ) {
        atuError( FXTRUE, "could not allocate anim\n");
    }
  
    ScaleVertices(&tdsfile);

    /*
     * Calculate the bounding boxes for the geometry.
     */

    TDSCalcBoundingBoxes( &tdsfile );

    /*
     * Recalculate the vertex normals now that the geometry is correct.
     */

    for( i = 0; i < tdsfile.num_n_tri_objs; i++ ) {
        TDSCalcFaceNormals( &tdsfile, i );
        TDSCalcVertexNormals( &tdsfile, i );
    }

    /*
     * Create the static objects which make up the parts which are 
     * referred to in the hierarchy of the 3ds model.
     */

    CreateShapes( group, &tdsfile, usetextures, use_animation );

    if( use_animation )
        CreateInstances( &tdsfile, group ); 

    if ( use_animation )
        atuMemFree(shapes);

    ComputeStaticBounds( &tdsfile, &bsphere);
    atsNodeBSphere(group, &bsphere);

    atsComputeBSphere(group, &bsphere);
    atsNodeBSphere(group, &bsphere);

    atsComputeBBox(group, &bbox);

    return group;
}

void atsInit3DS(void) {

    if (!tds_inited) {
      TDSInit(30000);
      tds_inited = FXTRUE;
    }
}

void atsTerm3DS(void) {
}
