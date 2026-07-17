
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
** $Date: 10/11/00 7:33:41 PM$ 
**
*/
#include <float.h>
#include <string.h>
#include <atrender.h>
#include <atmath.h>

#define NEW(x) ((x *)malloc(sizeof(x)))
#define NEW_VEC(x, n) ((x *)malloc((n)*sizeof(x)))

#define GRID_INC 100

/*-------------------------------------------------------------------
  Function: CalculateGridExtents
  Date: 6/2/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Figure out how many cells there are in each direction for the grid.
  Arguments:
    cmesh  - mesh whose extent is being computed
  Return:
    Nothing
  -------------------------------------------------------------------*/

static void CalculateGridExtents( AtmCMesh *cmesh) {
    AtmBox *b = &(cmesh->bounds);

  cmesh->int_x_size = ( int )( ( b->max[0] - b->min[0] + cmesh->grid_unit_size * 0.5f ) / cmesh->grid_unit_size );
  cmesh->int_z_size = ( int )( ( b->max[2] - b->min[2] + cmesh->grid_unit_size * 0.5f ) / cmesh->grid_unit_size );
}

static int MaxVectorComponent( AtmVector3 v ) {
    if( v[0] > v[1] )
        if( v[0] > v[2] )
             return 0;
        else return 2;
    else if( v[1] > v[2] )
              return 1;
         else return 2;
}

static void PreprocessTriRayIntersection( AtmCTri *tri ) {
    int max_normal_component;

    max_normal_component = MaxVectorComponent( tri->plane.normal );

    switch( max_normal_component ) {
    case 0:
        tri->i1 = 1;
        tri->i2 = 2;
        break;
    case 1:
        tri->i1 = 0;
        tri->i2 = 2;
        break;
    case 2:
        tri->i1 = 0;
        tri->i2 = 1;
        break;
    }
}

/*-------------------------------------------------------------------
  Function: atmSegIsectCTri
  Date: 6/2/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Compute the intersection of a line segment with a triangle
  Arguments:
    seg  - the segment
    tri  - the triangle
    t    - segment parameter for intersection point
  Return:
    FXTRUE if they intersect, FXFALSE if not
  -------------------------------------------------------------------*/

FxBool atmSegIsectCTri( const AtmSeg *seg, AtmCTri *tri,
                               float *min_dist_so_far ) {
    AtmVector3 intersection_point;
    float t;
    float u0, u1, u2, v0, v1, v2;
    int inter;
    float alpha, beta;
    
    /* determine if segment intersects plane embedding triangle */
 
    if ( (atmSegIsectPlane(seg, &(tri->plane), &t) & ATM_IS_TRUE ) == 0 )
        return FXFALSE;

    /*
     * Get the intersection point.
     */

    ATM_VEC3_ADD_SCALED(intersection_point, seg->pos, t, seg->dir);

    u0 = intersection_point[tri->i1] - tri->v[0][tri->i1];
    v0 = intersection_point[tri->i2] - tri->v[0][tri->i2];
    u1 = tri->v[1][tri->i1] - tri->v[0][tri->i1];
    u2 = tri->v[2][tri->i1] - tri->v[0][tri->i1];
    v1 = tri->v[1][tri->i2] - tri->v[0][tri->i2];
    v2 = tri->v[2][tri->i2] - tri->v[0][tri->i2];
    inter = 0;

    if ( u1 == 0.0f ) {
        beta = u0 / u2;
        if ((beta >= 0.) && (beta <= 1.)) {
            alpha = (v0 - beta * v2) / v1;
            inter = ((alpha >= 0.) && ((alpha + beta) <= 1.));
        }
    } else {
        beta = (v0 * u1 - u0 * v1) / (v2 * u1 - u2 * v1);
        if ((beta >= 0.) && (beta <= 1.)) {
            alpha = (u0 - beta * u2) / u1;
            inter = ((alpha >= 0) && ((alpha + beta) <= 1.));
        }
    }

    if (inter) {
        *min_dist_so_far = t;
        return FXTRUE ;
    } 

    return FXFALSE;
}

void _atsCMeshAllocateGridCells( AtmCMesh *cmesh ) {
    FxU32 i;

    cmesh->cells = NEW_VEC(AtmGridCell *, cmesh->int_x_size );

    if( !cmesh->cells ) {
        atuError( FXTRUE, "Out of memory in _atsCMeshAllocateGridCells\n" );
    }

    for( i = 0; i < cmesh->int_x_size; i++ ) {
      cmesh->cells[i] = NEW_VEC(AtmGridCell, cmesh->int_z_size );
      if( !cmesh->cells[i] ) {
          atuError( FXTRUE, "Out of memory in _atsCMeshAllocateGridCells\n" );
        }
    }
}

static int CalcClipcode( float xmin, float xmax, float zmin, float zmax, 
                         AtmVector3 p ) {
    int clipcode = 0;

    if( p[0] < xmin )
      clipcode |= 1;
    if( p[0] > xmax )
      clipcode |= 2;
    if( p[2] < zmin )
      clipcode |= 4;
    if( p[2] > zmax )
      clipcode |= 8;

    return clipcode;
}

static int CountNumberOfTrisInCell( AtmCMesh *cmesh, float xmin, float xmax, 
                              float zmin, float zmax ) {
    FxU32 num_tris = 0;
    FxU32 trinum;

    /*
     * Figure out how many tris are in this cell.
     */
    for( trinum = 0; trinum < cmesh->num_tris; trinum++ ) {
        int clip_codes[3];

        clip_codes[0] = CalcClipcode( xmin, xmax, zmin, zmax, cmesh->tri_list[trinum].v[0] );
        clip_codes[1] = CalcClipcode( xmin, xmax, zmin, zmax, cmesh->tri_list[trinum].v[1] );
        clip_codes[2] = CalcClipcode( xmin, xmax, zmin, zmax, cmesh->tri_list[trinum].v[2] );
              
        if( clip_codes[0] & clip_codes[1] & clip_codes[2] ) {
            /*
             * At this point, the triangle is definitely not within the grid
             * cell.
             */
            continue;
        }
        num_tris++;
    }

    return num_tris;
}

static void PutTrisInGridCell( AtmCMesh *cmesh, float xmin, float xmax, 
                        float zmin, float zmax, AtmGridCell *cell )
{
    FxU32 trinum;
    FxU32 cell_tri_index = 0;

    /*
     * Figure out how many tris are in this cell.
     */

    for( trinum = 0; trinum < cmesh->num_tris; trinum++ ) {
        int clip_codes[3];

        clip_codes[0] = CalcClipcode( xmin, xmax, zmin, zmax, cmesh->tri_list[trinum].v[0] );
        clip_codes[1] = CalcClipcode( xmin, xmax, zmin, zmax, cmesh->tri_list[trinum].v[1] );
        clip_codes[2] = CalcClipcode( xmin, xmax, zmin, zmax, cmesh->tri_list[trinum].v[2] );
              
        if( clip_codes[0] & clip_codes[1] & clip_codes[2] ) {
            /*
             * At this point, the triangle is definitely not within the grid
             * cell.
             */
            continue;
        }
        cell->tris[cell_tri_index] = &cmesh->tri_list[trinum];
        cell_tri_index++;
    }
}

void _atsCMeshPopulateGridCells( AtmCMesh *cmesh ) {
    FxU32 x, z;
    float xmin, xmax, zmin, zmax;
    AtmGridCell *cell;

    for( x = 0; x < cmesh->int_x_size; x++ ) {
        for( z = 0; z < cmesh->int_z_size; z++ ) {
            /*
             * Figure out the X-Z range that this cell subtends.
             */
            xmin = cmesh->bounds.min[0] + x * cmesh->grid_unit_size;
            zmin = cmesh->bounds.min[2] + z * cmesh->grid_unit_size;
            xmax = xmin + cmesh->grid_unit_size;
            zmax = zmin + cmesh->grid_unit_size;

            cell = &cmesh->cells[x][z];

            cell->num_tris = CountNumberOfTrisInCell( cmesh, xmin, xmax, zmin, zmax );

            cell->tris = NEW_VEC( AtmCTri *, cell->num_tris );
          
            if( !cell->tris ) {
                atuError( FXTRUE, "Out of memory in _atsCMeshPopulategridCells\n" );
            }
          
            PutTrisInGridCell( cmesh, xmin, xmax, zmin, zmax, cell );
        }
    }
}

static int PointInside2DTriXZ( AtmVector3 a, AtmVector3 b, AtmVector3 c, 
                        float x, float z ) {
    AtmVector3 side;
    AtmVector3 vect;

    atmVector3Sub( side, b, a );
    vect[0] = x - a[0];
    vect[1] = 0.0f;
    vect[2] = z - a[2];
    if( side[0] * vect[2] - vect[0] * side[2] < 0.0f )
        return 0;

    atmVector3Sub( side, c, b );
    vect[0] = x - b[0];
    vect[1] = 0.0f;
    vect[2] = z - b[2];
    if( side[0] * vect[2] - vect[0] * side[2] < 0.0f )
      return 0;
  
    atmVector3Sub( side, a, c );
    vect[0] = x - c[0];
    vect[1] = 0.0f;
    vect[2] = z - c[2];
    if( side[0] * vect[2] - vect[0] * side[2] < 0.0f )
      return 0;
  
    return 1;
}

static void atmCMeshDisplayGridCellIndexed( AtmCMesh *cmesh, FxI32 x, FxI32 z ) {
    AtmGridCell *cell;
    int trinum;
    AtrVertex   a, b, c;

    if( ( x < 0 ) || ( x >= (FxI32)cmesh->int_x_size ) ||
        ( z < 0 ) || ( z >= (FxI32)cmesh->int_z_size ) ) {
          atuError( FXFALSE, 
                "Trying to display an out of bounds grid cell %d %d.\n", x, z );
        return;
    }

    cell = &cmesh->cells[x][z];

    for( trinum = 0; trinum < cell->num_tris; trinum++ )        {
        a.x = cell->tris[trinum]->v[0][0];
        a.y = cell->tris[trinum]->v[0][1];
        a.z = cell->tris[trinum]->v[0][2];
        b.x = cell->tris[trinum]->v[1][0];
        b.y = cell->tris[trinum]->v[1][1];
        b.z = cell->tris[trinum]->v[1][2];
        c.x = cell->tris[trinum]->v[2][0];
        c.y = cell->tris[trinum]->v[2][1];
        c.z = cell->tris[trinum]->v[2][2];
        atrRenderTri( &a, &b, &c);
    }
}

/*-------------------------------------------------------------------
  Function: atmCMeshDefault
  Date: 6/25/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Initailize a cmesh to default values
  Arguments:
    cmesh - mesh to initialize
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atmCMeshDefault( AtmCMesh *cmesh ) {
    cmesh->gridMesh = FXFALSE;
    cmesh->grid_unit_size = 0.0f;
    cmesh->mask = ~0UL;
    cmesh->max_tris = 0;
    cmesh->num_tris = 0;
    cmesh->tri_list = NULL;
    atmBoxEmpty(&(cmesh->bounds));
}

/*-------------------------------------------------------------------
  Function: atmCMeshNew
  Date: 6/2/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Create a new collison mesh
  Arguments:
    None
  Return:
    New collison mesh
  -------------------------------------------------------------------*/

AtmCMesh *atmCMeshNew(void) {
    AtmCMesh *cmesh;

    if (( cmesh = atuMemCalloc(1, sizeof(AtmCMesh))) == NULL ) {
        atuError(FXFALSE, "atmCMeshNew: out of memory\n");
        return NULL;
    } 

    atmCMeshDefault( cmesh );

    return cmesh;
}

/*-------------------------------------------------------------------
  Function: atmCMeshDelete
  Date: 6/26/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Delete a collison mesh
  Arguments:
    cmesh - mesh to delete
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atmCMeshDelete(AtmCMesh *cmesh) {
    FxU32 i;

    if( !cmesh->cells ) {
        atuError( FXTRUE, "Out of memory in _atsCMeshAllocateGridCells\n" );
    }

    for( i = 0; i < cmesh->int_x_size; i++ ) {
      free(cmesh->cells[i]);
    }

    free(cmesh->cells); 

    free(cmesh->tri_list);

    free( cmesh );
}

/*-------------------------------------------------------------------
  Function: atmCMeshClone
  Date: 6/26/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Clone a collison mesh
  Arguments:
    cmesh - mesh to clone
  Return:
    cloned mesh
  -------------------------------------------------------------------*/

AtmCMesh *atmCMeshClone(AtmCMesh *cmesh) {
    AtmCMesh *c;

    if ( ( c = atmCMeshNew()) == NULL ) {
        atuError(FXTRUE, "atmCMeshClone: out of memory\n");
        return NULL;
    }
    
    *c = *cmesh;

    if ( cmesh->num_tris > 0 ) {
        c->tri_list = ( AtmCTri * )malloc( sizeof( AtmCTri ) * cmesh->max_tris );

        if ( cmesh->tri_list == NULL ) {
            atuError(FXTRUE, "atmCMeshClone: out of memory\n");
            return NULL;
        }

        memcpy( c->tri_list, cmesh->tri_list, cmesh->num_tris*sizeof( AtmCTri ));
    }

    /* if grided compute cells */

    if (c->gridMesh) {
        _atsCMeshAllocateGridCells( c );
        _atsCMeshPopulateGridCells( c );
    }

    return c;
}

/*-------------------------------------------------------------------
  Function: atmCMeshDraw
  Date: 8/14/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Draw a collision mesh
  Arguments:
    cmesh  - the mesh 
    drawFilled - draw filled otherwise draw outline
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atmCMeshDraw(const AtmCMesh *cmesh, FxBool drawFilled) {
    FxU32 i;
    AtmCTri *t;
    AtrVertex a, b, c;
#ifdef AT_DEBUGGING
    if ( !cmesh ) 
        atuError( FXTRUE, "atsCMeshDraw(): Invalid parameter.\n" );
#endif    

    for ( i = 0, t = cmesh->tri_list; i < cmesh->num_tris; i++, t++ ) {
        a.x = t->v[0][0]; a.y = t->v[0][1]; a.z = t->v[0][2];
        b.x = t->v[1][0]; b.y = t->v[1][1]; b.z = t->v[1][2];
        c.x = t->v[2][0]; c.y = t->v[2][1]; c.z = t->v[2][2];

        if ( drawFilled ) {
            atrRenderTri(&a, &b, &c);
        } else {
            atrRenderSegment( &a, &b );
            atrRenderSegment( &b, &c );
            atrRenderSegment( &c, &a );
       }
    }
}

/*-------------------------------------------------------------------
  Function: atmCMeshPrint
  Date: 6/26/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Print a collision mesh
  Arguments:
    cmesh  - the mesh 
    stream - stream to print to
    indent - line indentation
    verbose - level of detail
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atmCMeshPrint(const AtmCMesh *cmesh, FILE *stream, FxU32 indent, FxU32 verbose){
    FxU32 i;
    AtmCTri *t;
#ifdef AT_DEBUGGING
    if ( !cmesh || !stream ) 
        atuError( FXTRUE, "atsCMeshPrint(): Invalid parameter.\n" );
#endif    

    fprintf( stream, "%*sCMesh {\n", indent, " ");
    fprintf( stream, "%*snum triangles %d\n", indent+4, " ", cmesh->num_tris);
    if ( cmesh->gridMesh )
        fprintf( stream, "%*sxsize %d, zsize %d, unit size %f\n", 
                 indent+4, " ", cmesh->int_x_size, cmesh->int_z_size, 
                 cmesh->grid_unit_size);

    if ( verbose ) {
        for ( i = 0, t = cmesh->tri_list; i < cmesh->num_tris; i++, t++ ) {
            fprintf( stream, "%*s{\n", indent+4, " ");
            fprintf( stream, "%*splane { %f %f %f %f }\n", 
                     indent+8, " ", 
                     t->plane.normal[0], t->plane.normal[1], t->plane.normal[2],
                     t->plane.offset);
            fprintf( stream, "%*sv0 { %f %f %f }\n", 
                     indent+8, " ", t->v[0][0], t->v[0][1], t->v[0][2]);
            fprintf( stream, "%*sv1 { %f %f %f }\n", 
                     indent+8, " ", t->v[1][0], t->v[1][1], t->v[1][2]);
            fprintf( stream, "%*sv2 { %f %f %f }\n", 
                     indent+8, " ", t->v[2][0], t->v[2][1], t->v[2][2]);
            fprintf( stream, "%*si1 = %d, i2=%d\n", 
                     indent+8, " ", t->i1, t->i2);
            fprintf( stream, "%*s}\n", indent+4, " ");
        }
    }
    fprintf( stream, "%*s}\n", indent, " ");
}

/*-------------------------------------------------------------------
  Function: atmCMeshGridSize
  Date: 6/2/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Specify that collison mesh is to be subdivided on a grid of specified spacing
  Arguments:
    cmesh  - the mesh 
    grid_unit_size - grid spacing
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atmCMeshGridSize(AtmCMesh *cmesh, const float grid_unit_size) {
    cmesh->grid_unit_size = grid_unit_size;
    cmesh->gridMesh = FXTRUE;
}

static void atmCMeshDisplayGridCellFloat( AtmCMesh *cmesh, float x, float z ) {
    int x_index, z_index;

     x_index = ( int )( ( x - cmesh->bounds.min[0] ) / cmesh->grid_unit_size );
     z_index = ( int )( ( z - cmesh->bounds.min[2] ) / cmesh->grid_unit_size );
     atmCMeshDisplayGridCellIndexed( cmesh, x_index, z_index );
}

/*-------------------------------------------------------------------
  Function: atmCMeshAddTriangle
  Date: 6/2/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Add a new triangle to the collison grid
  Arguments:
    cmesh  - mesh whose extent is being computed
    v0, v1, v2 - triangle vertices
  Return:
    FXTRUE if successful, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool atmCMeshAddTriangle(AtmCMesh *cmesh, AtmVector3 v0, AtmVector3 v1, AtmVector3 v2) {
    AtmCTri *t;

    /*
     * check we have space for triangle 
     */

    if ( cmesh->num_tris >= cmesh->max_tris ) {
        if ( cmesh->tri_list == NULL ) {
            cmesh->max_tris = GRID_INC;
            cmesh->tri_list = ( AtmCTri * )malloc( sizeof( AtmCTri ) * cmesh->max_tris );
        } else {
            cmesh->max_tris += GRID_INC;
            cmesh->tri_list = ( AtmCTri * )realloc( cmesh->tri_list,
                                        sizeof( AtmCTri ) * cmesh->max_tris );
        }
  
        if( !cmesh->tri_list ) {
            atuError( FXTRUE, "Out of memory in atmCMeshAddTriangle\n" );
        }
    }

    t = cmesh->tri_list+cmesh->num_tris;

    /*
     * Compute plane equation for embedding plane.
     * If vertices are colinear (zero area triangle) ignore triangle
     */

    if ( !atmPlaneFromPoints(&t->plane, v0, v1, v2)) {
        printf("degenerate triangle\n");
        return FXTRUE;
    }

    t->mask = cmesh->mask;
  
    ATM_VEC3_COPY(t->v[0], v0); ATM_PT_EXTEND_BOX(&(cmesh->bounds), v0);
    ATM_VEC3_COPY(t->v[1], v1); ATM_PT_EXTEND_BOX(&(cmesh->bounds), v1); 
    ATM_VEC3_COPY(t->v[2], v2); ATM_PT_EXTEND_BOX(&(cmesh->bounds), v2); 

    PreprocessTriRayIntersection( t );
  
    cmesh->num_tris++;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atmCMeshIDMask
  Date: 6/2/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Specify current mask for mesh
  Arguments:
    cmesh  - the mesh
    idMask - new id mask
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atmCMeshIDMask(AtmCMesh *cmesh, FxU32 idMask) {
    cmesh->mask = idMask;
}

/*-------------------------------------------------------------------
  Function: atmCMeshClose
  Date: 6/2/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Close collision mesh, perform gridding if enabled
  Arguments:
    cmesh  - the mesh
  Return:
    Nothing
  -------------------------------------------------------------------*/

FxBool atmCMeshClose(AtmCMesh *cmesh) {
    if ( cmesh->gridMesh ) {
        CalculateGridExtents( cmesh );
        _atsCMeshAllocateGridCells( cmesh );
        _atsCMeshPopulateGridCells( cmesh );
    }
  
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: ProcessTriangle
  Date: 6/2/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Intersect a segment against a triangle, update hit status for an intersection
  Arguments:
    cmesh  - mesh whose extent is being computed
  Return:
    Intersection status
  -------------------------------------------------------------------*/

static FxU32 ProcessTriangle(AtmCTri *tri, AtmIsect *isect) {
    AtmSeg *seg = &isect->seg;
    float t;

    if ( ( tri->mask & isect->mask ) == 0 )
        return ATM_IS_FALSE;

    if ( !atmSegIsectCTri( seg, tri, &t ))
        return ATM_IS_FALSE;
          
    /* we have an intersection clip segment to this point */

    atmSegClip(&isect->hit_seg, seg, 0.0f, t);

    isect->hit_status = ATM_HIT_VALID | ATM_HIT_SEG | ATM_HIT_POINT |
                        ATM_HIT_NORM | ATM_HIT_VERTS;

    ATM_VEC3_ADD_SCALED(isect->hit_point, seg->pos, t, seg->dir);
    ATM_VEC3_COPY(isect->hit_normal, tri->plane.normal);
    ATM_VEC3_COPY(isect->hit_v[0], tri->v[0]);
    ATM_VEC3_COPY(isect->hit_v[1], tri->v[1]);
    ATM_VEC3_COPY(isect->hit_v[2], tri->v[2]);
    isect->hit_mask = tri->mask;

    return ( ATM_IS_TRUE | ATM_IS_MAYBE) ;
}

/*-------------------------------------------------------------------
  Function: atmCMeshIsectSeg
  Date: 6/2/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Intersect a list of segments against a mesh
  Arguments:
    cmesh  - mesh whose extent is being computed
    isect  - intersection data
  Return:
    FXTRUE if we got a hit
  -------------------------------------------------------------------*/

FxBool atmCMeshIsectSeg(const AtmCMesh *cmesh, AtmIsect *isect) {
    FxU32 triNum;
    AtmCTri *tri;

    isect->hit_status = 0;
    for ( triNum = 0, tri = cmesh->tri_list; 
          triNum < cmesh->num_tris; triNum++, tri++ ) {
        ProcessTriangle(tri, isect) ;
    }

    return (( isect->hit_status & ATM_HIT_VALID ) != 0 );
}

/*-------------------------------------------------------------------
  Function: atmCMeshFindBelow
  Date: 6/2/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determine the point (if any) below a given point
  Arguments:
    cmesh  - the mesh
    isect  - intersection control
  Return:
    FXTRUE if we intersected surface, false otherwise
  -------------------------------------------------------------------*/

int cell_x1, cell_z1 ;
int num_tris_1, tri_num_1 ;

FxBool atmCMeshFindBelow(AtmCMesh *cmesh, AtmIsect *isect) {
    FxI32 cell_x, cell_z;
    float oo_grid_unit_size;
    AtmGridCell *cell;
    int trinum;
    float ymax = -FLT_MAX; 
    float yinter;
    AtmCTri *tri;
    float *p = isect->seg.pos;
    AtmSeg seg;
    FxBool result;

    isect->hit_status = 0;

    seg = isect->seg;
    seg.pos[1] = 2000.0f;
    ATM_VEC3_SET(seg.dir, 0.0f, -1.0f, 0.0f);
    seg.length = 10000.0f;

    /*
     * Figure out which grid cell this location is in.
     */

    oo_grid_unit_size = 1.0f / cmesh->grid_unit_size;
    cell_x = ( int )( ( p[0] - cmesh->bounds.min[0] ) * oo_grid_unit_size );
    cell_z = ( int )( ( p[2] - cmesh->bounds.min[2] ) * oo_grid_unit_size );

    if( ( cell_x < 0 ) || ( cell_z < 0 ) || 
        ( cell_x >= (FxI32)cmesh->int_x_size ) || 
        ( cell_z >= (FxI32)cmesh->int_z_size ) ) {
#ifdef AT_DEBUGGING
        atuError( FXFALSE, "AtmCMeshFindMaxHeight: Out of bounds\n" );
#endif
        return 0;
    }

    /*
     * Iterated through all of the polys in this grid cell . . . find 
     * the one with the greatest height at this point.
     */

     cell = &cmesh->cells[cell_x][cell_z];

     cell_x1 = cell_x; cell_z1 = cell_z; num_tris_1 = cell->num_tris;

     for( trinum = 0; trinum < cell->num_tris; trinum++ ) {
        tri = cell->tris[trinum];

        result = PointInside2DTriXZ( tri->v[0], tri->v[2], tri->v[1], 
                                     p[0], p[2] ) ;

        if (!result)
            continue;
      
        /* is this triangle of the right type */

        if ((tri->mask & isect->mask ) == 0)
            continue;

        yinter = ( tri->plane.offset - tri->plane.normal[0] * p[0] - tri->plane.normal[2] * p[2] ) / tri->plane.normal[1];

        /*
         * Check for the boundary cases where we really aren't on the tri, but
         * our calculations tell us that we are.  If the point that we compute 
         * using the plane equation of the tri is higher than any points in the 
         * tri, then reject.
         */

        if( ( yinter > tri->v[0][1] + 5.0f ) && ( yinter > tri->v[1][1] + 5.0f ) && 
            ( yinter > tri->v[2][1] + 5.0f ) ) {
            continue;
        }
      
        if( ymax > yinter )
            continue;

        ymax = yinter;

        tri_num_1 = trinum;

        isect->hit_status = ATM_HIT_VALID | ATM_HIT_SEG | ATM_HIT_POINT |
                      ATM_HIT_NORM | ATM_HIT_VERTS;

        ATM_VEC3_SET(isect->hit_point, p[0], yinter, p[2]);
        ATM_VEC3_COPY(isect->hit_seg.pos, isect->seg.pos);
        ATM_VEC3_SET(isect->hit_seg.dir, 0.0f, -1.0f, 0.0f);
        isect->hit_seg.length = FLT_MAX;
        ATM_VEC3_COPY(isect->hit_normal, tri->plane.normal);
        ATM_VEC3_COPY(isect->hit_v[0], tri->v[0]);
        ATM_VEC3_COPY(isect->hit_v[1], tri->v[1]);
        ATM_VEC3_COPY(isect->hit_v[2], tri->v[2]);
        isect->hit_mask = tri->mask;
    }

    return (( isect->hit_status & ATM_HIT_VALID ) != 0);
}

/*-------------------------------------------------------------------
  Function: atmCMeshFind
  Date: 6/2/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Find point in mesh with given x and z
  Arguments:
    cmesh  - mesh whose extent is being computed
    isect  - intersection control
  Return:
    FXTRUE if successful
  -------------------------------------------------------------------*/

FxBool atmCMeshFind( AtmCMesh *cmesh, AtmIsect *isect) {

    isect->seg.pos[1] = FLT_MAX;

    return atmCMeshFindBelow( cmesh, isect);
}
