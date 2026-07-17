#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <atmath.h>
#include <atscene.h>

/* bldmesh.c
   Create a collision mesh given a node.

   atmCMeshFindBelow( cmesh, &iSect )
   atmCMeshIsectSeg( cmesh, &iSect )

   TBD: support nodes which define transformations.
 */
   

static FxBool processNode(AtsNode *n, AtmCMesh *cmesh);
static FxBool processLOD(AtsNode *n, AtmCMesh *cmesh);
static FxBool processGroup(AtsNode *n, AtmCMesh *cmesh);
static FxBool processTriSet(AtrTriSet *t, AtmCMesh *cmesh);
static FxBool processShape(AtsNode *n, AtmCMesh *cmesh);

static FxBool
processLOD(AtsNode *n, AtmCMesh *cmesh) {
    AtsLOD *lod = (AtsLOD *)n;

    /* use the finest level of detail, assumed to be LOD 0 */

    if ( lod->group.num_children > 0 ) {
        processNode(lod->group.children[0], cmesh);
    }

    return FXTRUE;
}

static FxBool
processGroup(AtsNode *n, AtmCMesh *cmesh) {
    int i;
    AtsGroup *group = (AtsGroup *)n;

    for ( i = 0; i < group->num_children; i++ )  {
        processNode(group->children[i], cmesh);
    }

    return FXTRUE;
}

static FxBool
processTriSet(AtrTriSet *t, AtmCMesh *cmesh) {
    _AtrTriSetNode *n = t->nodes;

    while( n ) {
        FxU32 *connectivity = n->connectivity;
        AtrVertex *verts = n->vertices;

        while( *connectivity ) {
            AtmVector3 v0, v1, v2;
            AtrVertex *a = verts+(((*connectivity)>>16)&0xFF);
            AtrVertex *b = verts+(((*connectivity)>>8)&0xFF);
            AtrVertex *c = verts+((*connectivity)&0xFF);

            v0[0] = a->x;
            v0[1] = a->y;
            v0[2] = a->z;
            v1[0] = c->x;
            v1[1] = c->y;
            v1[2] = c->z;
            v2[0] = b->x;
            v2[1] = b->y;
            v2[2] = b->z;
            atmCMeshAddTriangle( cmesh, v0, v1, v2);
            connectivity++;
        }

		n = n->next;
    }

    return FXTRUE;
}

static FxBool
processShape(AtsNode *n, AtmCMesh *cmesh) {
    int i;
    AtsShape *shape = (AtsShape *)n;

    for ( i = 0; i < shape->num_parts; i++ )  {
        AtsShapePart *sp = &shape->parts[i];
   
        processTriSet((AtrTriSet *)(sp->geometry), cmesh);
    }

    return FXTRUE;
}

static FxBool
processNode(AtsNode *n, AtmCMesh *cmesh) {
    AtsType *t = atsGetType( n ) ;

    if ( t == atsShapeGetType() ) {
        return processShape(n, cmesh);
    }
    
    if ( t == atsGroupGetType() ) {
        return processGroup(n, cmesh);
    }

    if ( t == atsLODGetType() ) {
        return processLOD(n, cmesh);
    }

    atuError(FXTRUE, "node type currently not supported %s",
                     atsGetTypeName(t));
    return FXFALSE;
}

/* build a collision mesh containing all the geometry in the node */

AtmCMesh *
buildCMesh(AtsNode *n) {
    AtmCMesh    *cmesh;
    AtmSphere    bsphere;

    /* create the mesh */

    cmesh = atsCMeshNew();

    /* determine reasonable grid element size */

    atsComputeBSphere(n, &bsphere);
    atmCMeshGridSize( cmesh, bsphere.radius/10.0f );

    /* collect all the geometry in the node */

    processNode(n, cmesh);

    /*  Complete the creation of the collision Mesh */

    atmCMeshClose( cmesh );

    return cmesh;
}
