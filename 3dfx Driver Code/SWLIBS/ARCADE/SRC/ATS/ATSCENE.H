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
** $Date: 10/11/00 7:34:29 PM$ 
**
*/

#ifndef _ATSCENE_H_
#   define _ATSCENE_H_

#include <atutil.h>
#include <atrender.h>
#include <atmath.h>

#define ATS_CATTR_INT           0
#define ATS_CATTR_FLOAT         1
#define ATS_CATTR_POINTER       2

#define ATS_CATTR_LOAD_TEXTURES       0
#define ATS_CATTR_TARGET_TYPE         1
#define ATS_CATTR_CMESH_GRID_SIZE     2
#define ATS_CATTR_START_FRAME         3
#define ATS_CATTR_END_FRAME           4
#define ATS_CATTR_VERTEX_SCALE        5
#define ATS_CATTR_TEXTURE_WRAP_FIX    6
#define ATS_CATTR_VERBOSE             7

BEGINFDECL()

#define ATS_RM_SOLID     0
#define ATS_RM_WIREFRAME 1
extern FxU32 _atsRenderMode ;
FxBool atsRenderMode(FxU32 renderMode);

typedef void AtsObject;

typedef void (* AtsPushMaterialFunc)( AtsObject *m );
typedef void (* AtsPopMaterialFunc)( FxBool update );

FxBool atsMaterialFuncs( AtsPushMaterialFunc push, AtsPopMaterialFunc pop);

extern AtsPushMaterialFunc _atsPushMaterial;
extern AtsPopMaterialFunc _atsPopMaterial;

FxBool atsConverterAttr(const char *ext, const int argID, void *val);
FxBool atsConverterGetAttr(const char *ext, const int argID, void **val);
FxBool atsConverterIntAttr(const char *ext, const int argID, int val);
FxBool atsConverterGetIntAttr(const char *ext, const int argID, int *val);
FxBool atsConverterFloatAttr(const char *ext, const int argID, float val);
FxBool atsConverterGetFloatAttr(const char *ext, const int argID, float *val);
FxBool atuStringCompare(const char *s1, const char *s2);
void atsFrameBegin(void);
float atsGetFrameTime(void);

FxBool atsInit(void);
void atsShutdown(void);
char *atsFileLocate(const char *filename, char *full_path);
void atsPushXform( AtrXform *t );
void atsPopXform( void );
void atsPostCatPushXform( AtrXform *t );
void atsPreCatPushXform( AtrXform *t );
void atsQueryXform( AtrXform *t );

/*
**------------------------------------------------------------------------
** TYPES
**------------------------------------------------------------------------
*/

typedef struct _node AtsNode;

typedef FxU32 (* AtsTravFunc)(AtsObject *object, void *data);

typedef void (* AtsTravVertexFunc)(int count, AtrVertex *v, void *data);

typedef void (* AtsNodeCollisionFunc)(AtsNode *node, void *data);


/*-------------------------------------------------------------------
  Structure: AtsType
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Structure that defines an object type
  Members:
    name            - name of the class
    parent          - parents class
    index           - which entry is this type in the type table
    size            - size of objects of this class
    Init            - initialize an instance of this class
    Clone           - clone function for this class
    Delete          - destructor for this class
    Print           - print function for this class
    Load            - load function for this class
    Store           - store function for this class
    Draw            - draw an instance of this class
    Traverse        - traverse a heirarchy
    Update          - physics simulation
    IsectNode       - intersect itself against another node
    IsectSeg        - intersect itself against a line segment
    IsectSphere     - intersect itself against a sphere
  TBD:
    Add ability to subclass types so that different objects can have
    different methods.
 
    Currently groups don't call their children when Updated. Are there 
    times when we would not want to invoke the objects Updare function.
    Do we want have invisibilty and don't update flags.
  -------------------------------------------------------------------*/

typedef struct _AtsType {
    char       *name;
    struct _AtsType   *parent;
    FxU16      index;
    size_t     size;
    void       (* Init)(AtsObject *obj);
    AtsObject* (* Clone)(AtsObject* node, FxU32 mode);
    FxU32      (* SetMask)(AtsNode *n, FxU32 mode, FxU32 mask);
    void       (* Delete)(AtsObject *obj);
    FxBool     (* Print)(const AtsObject* obj, FILE *stream, FxU32 indent, 
                         FxU32 verbose);
    FxBool     (* Load)(AtsObject *obj, FILE *stream);
    FxBool     (* Fixup)(AtsObject *obj);
    FxBool     (* Store)(AtsObject *obj, FILE *stream);
    void       (* EnumerateReferences)(AtsObject *obj);
    void       (* Draw)(AtsObject* node, FxU32 mask);
    FxU32      (* Traverse)(AtsObject* obj, AtsTravFunc func, void *data);
    void       (* TraverseVertices)(AtsObject* node, AtsTravVertexFunc func, 
                                    void *data);
    void       (* Update)(AtsObject *node);
    FxBool     (* IsectNode)(const AtsNode *node, const AtsNode *other);
    void       (* IsectSeg)(const AtsNode *node, AtmIsect *isect);
    FxBool     (* IsectSphere)(const AtsNode *node, const AtmSphere *sphere);
} AtsType;

extern AtsType **_ats_types ;

extern AtsType *_ats_object_type ;

#define ATS_CALL(t, op) (((AtsType *)t)->op)

#define ATS_PARENT_CALL(t, op) (((AtsType *)t)->parent->op)

#define ATS_NODE_CALL(p, op) (_ats_types[atuMemGetType(p)]->op)

#ifdef AT_DEBUGGING
#define ATS_VALIDATE_NODE(gp, np, t, c, func) {\
    if ( np == NULL ) \
        atuError( FXTRUE, "%s(): NULL paramter.\n", func );\
    if ( atsIsOfType(np, (AtsType *)c )) \
       gp = (t *)np; \
    else atuError( FXTRUE, "Invalid object. in %s, expected %s\n", \
                   func, atsGetTypeName((AtsType *)c) ); \
       gp = (t *)np; \
    }
#else
#define ATS_VALIDATE_NODE(gp, np, t, c, func) { gp = (t *)np; }
#endif

extern AtsType *_ats_node_type;

FxBool atsStoreString(char *s, FILE *stream);
FxBool atsLoadString(char **s, FILE *stream);
FxBool atsStoreType(AtsType *type, FILE *stream);
FxBool atsLoadType(AtsType **type, FILE *stream);
void *atsPointerToID(void *p);
FxBool atsObjectIDProcessed(void *p, FxI32 id);
void *atsIDToPointer(void *);

AtsType* atsGetType(const AtsObject *obj);

AtsType* atsGetTypeFromName(char* type_name);

AtsObject* _atsNew(AtsType *type, char *where, FxU32 line); 
#define atsNew(_t) _atsNew((_t),  __FILE__, __LINE__)

/* different methods for cloning hierarchies */

#define ATS_CLONE_OBJECT    1       /* clone just this object */
#define ATS_CLONE_HIERARCHY 2       /* clone only the hierarchy */
#define ATS_CLONE_ALL       4       /* clone the hierarchy and geometry */

AtsObject* atsClone(AtsObject *obj, FxU32 mode); 

void atsUnrefDelete(AtsObject *obj); 

void atsDelete(AtsObject *obj); 

const char* atsGetTypeName(const AtsType *t);

FxBool  atsIsOfType(const AtsObject *obj, AtsType *type);

FxBool  atsIsSubClassOf(AtsType *type1, AtsType *type2);

FxBool atsIsExactType(const AtsObject *obj, AtsType *type); 

FxU16 atsRef(AtsObject* obj);

FxU16 atsUnref(AtsObject* obj); 

FxU16  atsGetRefCount(const AtsObject* obj);

/* ATS Print levels */

#define ATS_PRINT_ALL        ~0
#define ATS_PRINT_TRANSFORMS 0x0001
#define ATS_PRINT_VERTICES   0x0002
#define ATS_PRINT_HIERARCHY  0x0004

FxBool atsPrint(const AtsObject* obj, FILE *stream, 
                 FxU32 indent, FxU32 verbose);

FxBool atsStore(AtsObject *obj, FILE *stream);

AtsObject* atsLoad(FILE *stream); 

AtsObject *atsFileLoad(const char *filename, AtsType *objectType );

FxBool atsFileStore(const char *filename, AtsObject *obj);

void atsDraw(AtsObject* node, FxU32 mask);

void atsUpdate(AtsObject* node);

void atsTraverse(AtsObject* node, AtsTravFunc func, void *data);

void atsTraverseVertices(AtsObject* node, AtsTravVertexFunc func, void *data);

void atsComputeBBox(AtsObject* node, AtmBox *box);

void atsComputeBSphere(AtsObject* node, AtmSphere *bsphere);

FxBool atsObjectNewType(AtsType *new_type);

/*-------------------------------------------------------------------
  Structure: AtsDictEntry
  Date: 6/15/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Structure that defines an dictionary entry
  Members:
    key      -   name of entry
    value    -   object being stored
  -------------------------------------------------------------------*/

typedef struct {
    char      *key;
    AtsObject *value;
} AtsDictEntry;

/*-------------------------------------------------------------------
  Structure: AtsDict
  Date: 6/15/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
     An AtsDict is a general purpose dictionary for holding objects in the
     data base.

  Members:
    num_entries  - current number of entries in dictionary
    max_entries  - current maximum size of dictionary
    entry        - table of entries
  -------------------------------------------------------------------*/

typedef struct _AtsDict {
    FxU32          num_entries;
    FxU32          max_entries;
    AtsDictEntry  *entries;
} AtsDict;

AtsObject* _atsDictNew(char *where, FxU32 line); 
#define atsDictNew() _atsDictNew(__FILE__, __LINE__)

FxBool atsDictAdd(AtsObject* dict, const char *name, AtsObject *obj);

AtsObject *atsDictFind(const AtsObject* d, char *name, AtsType *nodeType);

FxBool atsDictRemove(AtsObject* dict, char *name);

int atsDictGetSize(const AtsObject* dict);

AtsType* atsDictGetType(void);

void atsDictNewType(AtsType *dt);

/*-------------------------------------------------------------------
  Structure: AtsArray
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Structure that defines an array of elements.
  Members:
    type     -   type of elements
    attr     -   attributes of array
    max_size -   currently allocated space
    cur_size -   number of elements in array
    data     -   pointer to data region
  -------------------------------------------------------------------*/

#define ATS_ARRAY_USER_DEFINED 0x0001

typedef struct {
   FxU16 elem_size;
   FxU16 attr;
   FxU16 max_size; 
   FxU16 cur_size; 
   char *data;
} AtsArray;

int atsArrayMaxSize(AtsArray *a, FxU16 nelem);

FxU16 atsArrayGetMaxSize(AtsArray *a);

FxU16 atsArrayGetSize(AtsArray *a);

int atsArrayInit(AtsArray *a, FxU16 nelem, FxU16 elem_size);

AtsArray *atsArrayCreate(FxU16 nelem, FxU16 elem_size);

void atsArrayData(AtsArray *a, char *ptr);

char *atsArrayGetData(AtsArray *a);

int atsArrayCopy(AtsArray *dst, AtsArray *src);

int atsArrayConcat(AtsArray *dest, AtsArray *src);

int atsArrayAppend(AtsArray *a, char *p);

int atsArraySetAt(AtsArray *a, FxU16 i, char *p);

char *atsArrayGetAt(AtsArray *a, FxU16 i);

int atsArraySetInt(AtsArray *a, FxU16 i, int v);

int atsArrayGetInt(AtsArray *a, FxU16 i, int *v);

int atsArraySetFloat(AtsArray *dest, FxU16 i, float f);

int atsArrayGetFloat(AtsArray *dest, FxU16 i, float *v);

int atsArraySetObj(AtsArray *a, FxU16 i, AtsObject *p);

AtsObject *atsArrayGetObj(AtsArray *dest, FxU16 i);

/*-------------------------------------------------------------------
  Structure: AtsNode
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
      Data common to all nodes
  Members:
    flags        - misc flags
    mask         - groups id mask
    name         - nodes name
    bsphere      - bounding volume
    parents      - array of parent nodes
  -------------------------------------------------------------------*/

typedef struct _node {
    FxU32        flags;
    FxU32        mask;
    char        *name;
    AtmSphere    bsphere;
    AtsArray     parents;
}; /* AtsNode */

FxU32 atsNodeVisible(AtsNode *node);

FxU32 atsMaskCombine(FxU32 v1, FxU32 v2, FxU32 mode);

void atsNodeMask(AtsNode* n, FxU32 mode, FxU32 mask);

FxU32 atsNodeGetMask(AtsNode *node);

void atsNodeName(AtsNode* node, const char *name);

const char* atsNodeGetName(const AtsNode* node);

void atsNodeBSphere(AtsNode* node, AtmSphere *bsphere);

void atsNodeGetBSphere(AtsNode *node, AtmSphere *bsphere);

int atsNodeGetNumParents(const AtsNode* n);

AtsNode *atsNodeGetParent(const AtsNode* n, FxU32 i);

FxU32 atsBoxContainsNode(const AtmBox *box, const AtsNode *node);

FxBool atsSphereContainsNode(const AtmSphere *sphere, const AtsNode *node);

FxBool atsNodeIsectSeg(const AtsNode *node, AtmIsect *isect);

FxBool atsNodeCollide(AtsNode* node1, AtsNode *node2);

void atsNodeNewType(AtsType *new_type);

/*
**------------------------------------------------------------------------
** TYPES
**------------------------------------------------------------------------
*/


/*-------------------------------------------------------------------
  Structure: AtsTexture
  Date: 7/9
  Implementor(s): jdt
  Library: AT Scene
  Description:
      An AtsTexture uses one or more images to define the visual appeareance
      of a texture. Using more than one image provides a movie like facility
      where each image is displayed for a specified amount of time. A texture
      can consist of any number of images.

  Members:
    handle       - texture handle
    img          - list of images texture displays
    numImages    - current number of images in texture
    maxImages    - current size of image list
    curImage     - currently displayed image
  -------------------------------------------------------------------*/
typedef struct {
    AtrTexHandle   handle;
    AtrImg       **img;
    int            numImages;
    int            maxImages;
    int            curImage;
} AtsTexture;

typedef AtrTexelFx (* AtsTMUFunc)( void );

AtsObject* _atsImageNew(char *where, FxU32 line); 
#define    atsImageNew() _atsImageNew(__FILE__, __LINE__)
AtsObject *atsImageCreateFromFile( const char *fileName );
AtsObject *atsTextureCreateFromFile( const char *fileName );
FxBool     atsTextureHasAlpha(AtsTexture *t);
AtsObject *atsTextureFindOrDownload( const char *filename );
AtsObject *atsTextureDownload( const char *filename );
AtsObject* _atsTextureNew(char *where, FxU32 line); 
#define    atsTextureNew() _atsTextureNew(__FILE__, __LINE__)
int        atsTextureNumFrames( AtsObject *t );
void       atsTextureFrame( AtsObject *t, int frame );
AtsObject* atsTextureMovie( const char *filename, int start, int end );
void       atsTextureAddImage(AtsObject *t, AtrImg *img);
FxBool     atsTextureUpdate( AtsObject *t);
void       atsTextureReset( AtsObject *t);
void       atsTMUFunc(AtsTMUFunc func);
FxBool     atsTextureTMU( AtsObject *obj, AtrTexelFx tmu );
AtrTexelFx atsTextureGetTMU( AtsObject *obj );

/*-------------------------------------------------------------------
  Structure: AtsMaterial
  Date: 7/8
  Implementor(s): jdt
  Library: AT Scene Manager
  Description:
  Scene manager container for AtrMaterial
  Members:
  material  - AtrMaterial
  textures  - AtsTexture containers mapped to material's textures
  -------------------------------------------------------------------*/
typedef struct _AtsMat {
    AtrMaterial    material;
    AtsObject     *textures[2];
    struct _AtsMat *successor;
} AtsMaterial;

AtsObject* _atsMaterialNew(char *where, FxU32 line); 
#define atsMaterialNew() _atsMaterialNew(__FILE__, __LINE__)
void atsMaterialTexture(AtsObject *mat, int which, AtsObject *tex);
AtsObject *atsMaterialGetTexture(AtsObject *mat, int which);
void atsMaterialSuccessor(AtsObject *mat, AtsObject *successor);
AtsObject *atsMaterialGetSuccessor(AtsObject *mat);


/*-------------------------------------------------------------------
  Structure: AtsShapePart
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
      When a surface is rendered both the geometry describing the location
      and coonectivity of the surface and the materials describing the 
      appearance and rendering attributes need to be provided. To optimize
      rendering and minimize state changes the surface is broken down into
      a set of parts. Each part consists of a piece of geometry and a single
      material used to render that geometry.  We will add more 
      information later to support sorting of primitives to minimize
      changes in material and to support transparency.
  Members:
    geometry          - vertices and connectivity
    material          - visual appearance and rendering attributes
  -------------------------------------------------------------------*/

typedef struct _AtsShapePart {
    AtsObject    *material;
    AtsObject    *geometry;
} AtsShapePart;

/*-------------------------------------------------------------------
  Structure: AtsShape
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
      A shape represents a single surface as a list of parts. 
  Members:
    node              - data common to all nodes
    parts             - list of parts
    num_parts         - current list of parts list
    max_parts         - current allocated length of parts list
  -------------------------------------------------------------------*/

typedef struct _AtsShape {
    AtsNode        node;
    AtsShapePart   *parts;
    int            num_parts;
    int            max_parts;
} AtsShape;

AtrTriSet* _atsTriSetNew(char *where, FxU32 line); 
#define atsTriSetNew() _atsTriSetNew(__FILE__, __LINE__)

AtmCMesh* _atsCMeshNew(char *where, FxU32 line); 
#define atsCMeshNew() _atsCMeshNew(__FILE__, __LINE__)

AtsType* atsShapeGetType(void); 

FxBool atsShapeAddPart(AtsNode *node, AtsObject *geometry, AtsObject *material);

int atsShapeGetNumParts(AtsNode *node);

FxBool atsShapePart(AtsNode *node, int index, AtsObject *geometry, 
                       AtsObject *material);

FxBool atsShapeGetPart(AtsNode *node, int i, AtsObject **geometry, 
                       AtsObject **material);

AtsNode* _atsShapeNew(char *where, FxU32 line); 
#define atsShapeNew() _atsShapeNew(__FILE__, __LINE__)

AtsType* atsShapeGetType(void); 

void atsShapeNewType(AtsType *gt);

/*-------------------------------------------------------------------
  Structure: AtsPrim
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
      A primitive is used in the construction of shapes to simplify
      the importing of databases from other formats.
  Members:
    primType  -   Type of primitive currently must be a convex polygon
    numVerts  -   Number of vertices in primitive
    nBind     -   Binding for normals (currently must be per vertex)
    cBind     -   Binding for colors (currently must be NONE or per vertex)
    tBind     -   Binding for texture coordinates (must be NONE or per vertex)
    locs      -   Vertex locations
    norms     -   normals
    colors    -   colors
    tex       -   texture ccordinates
    vIdx      -   vertex index list (if vertices indexed)
    nIdx      -   normal index list (if normals indexed)
    cIdx      -   color index list (if colors indexed)
    tIdx      -   texture coordinate index list (if textures indexed)
  -------------------------------------------------------------------*/

#define ATS_BIND_NONE        0x0
#define ATS_BIND_PER_VERTEX  0x1
#define ATS_BIND_PER_FACET   0x2
#define ATS_BIND_INDEXED     0x4
#define ATS_BIND_INDEXED_VERTEX  (ATS_BIND_PER_VERTEX | ATS_BIND_INDEXED )

#define ATS_PRIM_POLYGON    0

#define ATS_PRIM_CW                0    /* clocwise oriented polygons */
#define ATS_PRIM_CCW               1    /* counter clockwise orientation */
#define ATS_PRIM_TWO_SIDED         2    /* primitive is two sided */
#define ATS_PRIM_GEN_FACET_NORMAL  4    /* generate facet normals */

typedef struct {
    FxU32 flags; /* must be zero for now */
    FxU32 primType;
    FxU32 numVerts;

    FxU32 lBind;
    FxU32 nBind;
    FxU32 cBind;
    FxU32 tBind;
 
    AtmVector3 *locations;
    AtmVector3 *normals;
    AtmVector4 *colors;
    AtmVector2 *texCoords;

    /* vertex index lists */

    FxU16 *lIdx;
    FxU16 *nIdx; 
    FxU16 *cIdx; 
    FxU16 *tIdx;
} AtsPrim;

AtsPrim *atsShapeBuildInit(FxU32 maxVerts, FxU32 maxIdx);
FxBool atsShapeAddPrim(void);
void atsShapeMaterial(AtsMaterial *mat);
AtsNode *atsShapeConstruct(FxBool autoNormals);

/*
**------------------------------------------------------------------------
** TYPES
**------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
  Structure: AtsGroup
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
     An AtsGroup is the base class used for constructing scene hierarchies 
     and is derived from AtsNode. The functions allow children to be added 
     to and deleted from an AtsGroup node and queries to be made about an 
     AtsGroup nodes children. The group maintains a list of its children
     which is dynamically resized as the number of children increases. In
     order to not be continually reallocating space the array may be larger
     than is strictly necessary based on the current number of children. As
     children are added to the group their reference count is incremented, as
     they are removed from the group their reference count is decremented.

  Members:
    node         - data common to all nodes
    num_children - current number of children in group
    max_children - current size of child array
    children     - array of child nodes
  -------------------------------------------------------------------*/

typedef struct _AtsGroup {
    AtsNode        node;
    FxU16          num_children;
    FxU16          max_children;
    AtsNode**      children;
} AtsGroup;

extern AtsType *_ats_group_type ;

AtsObject* atsFind(AtsObject* node, const char *name, const AtsType *type);

AtsNode* _atsGroupNew(char *where, FxU32 line); 
#define atsGroupNew() _atsGroupNew(__FILE__, __LINE__)

AtsType*    atsGroupGetType(void);

FxBool      atsGroupAddChild(AtsNode* group, AtsNode *child);
  
FxBool      atsGroupInsertChild(AtsNode* group, int index, AtsNode *child);

int         atsGroupRemoveChild(AtsNode* group, AtsNode *child);

int         atsGroupFindChildIndexByName(AtsNode* g, const char *child_name);

int         atsGroupReplaceChild(AtsNode* group, AtsNode *oldn, AtsNode *newn);

AtsNode*    atsGroupGetChild(const AtsNode* group, int i);

int         atsGroupGetSize(const AtsNode* group);

int         atsGroupFindChild(const AtsNode* group, AtsNode *n);

void        atsGroupFlatten(AtsNode* node, FxU32 mode);

void        atsGroupNewType(AtsType *group_type); 

/*-------------------------------------------------------------------
  Structure: AtsSequence
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
      An AtsSequence is a group that sequences through a range of its 
      children, drawing each child for a specified duration. Each child 
      in a sequence can be thought of as a frame in an animation. A 
      sequence can consist of any number of children, and each child has 
      its own duration. You can control whether an entire sequence repeats 
      from start to end, repeats from end to start, or terminates. 

  Members:
    group             - data common to all groups
    state             - sequence state (ATS_SEQ_STOPPED, ATS_SEQ_PAUSED, ATS_SEQ_RUNNING)
    mode              - sequence mode ( ATS_SEQ_CYCLE, ATS_SEQ_SWING)
    startTime         - time at which cycle started
    duration          - time for each sequence
    offset            - offset (in seconds) from start of cycle
    basis             - time or frame based 
    framesRendered    - frames rendered so far
  -------------------------------------------------------------------*/

typedef struct _AtsSeq {
    AtsGroup       group;
    int            state;
    int            mode;
    float          startTime;
    float          duration;
    float          offset;
    int            numReps;
    int            basis;
    int            framesRendered;
} AtsSeq;

/* valid sequence states */

#define ATS_SEQ_STOPPED 0
#define ATS_SEQ_RUNNING 1
#define ATS_SEQ_PAUSED  2

/* atsSeqControl() */

#define ATS_SEQ_STOP    0
#define ATS_SEQ_START   1
#define ATS_SEQ_PAUSE   2
#define ATS_SEQ_RESUME  3

#define PFSEQ_ALL       -1

/* atsSeqMode() */ 

#define ATS_SEQ_CYCLE   0
#define ATS_SEQ_SWING   1

/* atsSeqBasis() */

#define ATS_SEQ_TIME    0
#define ATS_SEQ_FRAME   1

AtsNode* _atsSeqNew(char *where, FxU32 line); 
#define atsSeqNew() _atsSeqNew(__FILE__, __LINE__)
AtsType*    atsSeqGetType(void);
void        atsSeqMode(AtsNode* seq, int mode);
int         atsSeqGetMode(const AtsNode* seq);
void        atsSeqDuration(AtsNode* seq, float duration);
float       atsSeqGetDuration(const AtsNode *seq);
void        atsSeqOffset(AtsNode* node, float offset);
float       atsSeqGetOffset(const AtsNode *node);
void        atsSeqReps(AtsNode* seq, int numReps);
int         atsSeqGetReps(const AtsNode* seq);
void        atsSeqControl(AtsNode* seq, int action);
void        atsSeqBasis(AtsNode *ats, int basis);
int         atsSeqGetBasis(AtsNode *ats);
void        atsSeqNext(AtsNode *node);
void        atsSeqNewType(AtsType *ft);

/*-------------------------------------------------------------------
  Structure: AtsLOD
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
      An AtsLOD is a group where each of its children represent different visual
      representations of the same object.

    group        - information common to all groups 
    center       - center point (used in determining child to display)
    numRanges    - number of ranges currently in LOD
    maxRanges    - current size of range array
    range        - distance to use this LOD (1 entry per child)
  -------------------------------------------------------------------*/

#define ATS_LOD_OFF       0
#define ATS_LOD_MAX       1
#define ATS_LOD_MIN       2
#define ATS_LOD_LEVEL     3
#define ATS_LOD_DISTANCE  4

typedef struct _AtsLOD {
    AtsGroup       group;
    AtmVector3     center;
    int            maxRanges;
    float         *range;
} AtsLOD;

AtsNode* _atsLODNew(char *where, FxU32 line); 
#define atsLODNew() _atsLODNew(__FILE__, __LINE__)
AtsType*    atsLODGetType(void);
void        atsLODNewType(AtsType *ft);
void        atsLODCenter(AtsNode* lod, AtmVector3 center);
void        atsLODGetCenter(AtsNode* lod, AtmVector3 center);
void        atsLODRange(AtsNode* lod, int index, float range);
int         atsLODGetNumRanges(const AtsNode* lod);
float       atsLODGetRange(const AtsNode* lod, int index);
void        atsLODControl(int mode, float param);
void        atsGetLODControl(int *mode, float *param);

/*-------------------------------------------------------------------
  Structure: AtsTransform
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
      A transform node defines a new coordinate system to apply to its children
  Members:
    group             - information shared by all groups
    flags             - flags describing transform state
    xform             - transform to apply to children
  -------------------------------------------------------------------*/

#define ATS_TRANS_STATIC  0x0001 /* transform is static */

typedef struct _AtsFrame {
    AtsGroup        group;
    FxU32           flags;
    AtrXform        xform;
} AtsFrame;

AtsNode* _atsFrameNew(char *where, FxU32 line); 
#define atsFrameNew() _atsFrameNew(__FILE__, __LINE__)

void atsFrameXform(AtsNode *node, AtrXform *xform);

AtrXform* atsFrameGetXform(AtsNode *node);

void atsFrameStatic(AtsNode *node, FxBool is_static);

FxBool atsFrameGetStatic(AtsNode *node);

AtsType* atsFrameGetType(void); 

void atsFrameNewType(AtsType *gt);

/*-------------------------------------------------------------------
  Structure: AtsAnim
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
      An animation is a group node which animates each of its children. 
      To specify the animation you pass in an array of transforms. 
      If you have n children in the node and you have m frames of 
      animation you must provide nxm transforms otherwise the results 
      are unpredictable. 

      The frame_count array is reference counted.
  Members:
    group             - information shared by all groups
    current_frame     - current frame number
    num_frames        - number of frames in each behaviour
    xforms            - list of transforms defining the animations
    instance_names    - NULL terminated list of instance names
  -------------------------------------------------------------------*/

typedef struct _AtsAnim {
    AtsGroup   group;
    FxU16      current_frame;
    FxU16      num_frames;
    AtrXform   *xforms;
    char      **instance_names;
} AtsAnim;

AtsNode* _atsAnimNew(char *where, FxU32 line); 
#define atsAnimNew() _atsAnimNew(__FILE__, __LINE__)

AtsType* atsAnimGetType(void); 

void atsAnimXforms(AtsNode *node, FxU16 num_frames, AtrXform *xforms); 

void atsAnimGetXforms(AtsNode *node, FxU16 *num_frames, AtrXform **xforms);

void atsAnimInstanceNames(AtsNode *node, char **instance_names);

char **atsAnimGetInstanceNames(AtsNode *node);

void atsAnimFrame(AtsNode *a, const FxU16 frame);

FxU16  atsAnimGetFrame(AtsNode *a);

int atsAnimNameToIndex(AtsNode* a, const char *name);

FxU16 atsAnimGetNumFrames(AtsNode *node);

void atsAnimNextFrame(AtsNode *node);

void atsAnimNewType(AtsType *gt);

/*-------------------------------------------------------------------
  Structure: AtsPath
  Date: 6/2/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
      Identifies a specific instance of a node by specifying the 
      list of nodes traversed from the root node. 
  Members:
    cur_size          - current number of elements in path
    max_size          - maximum number of elements in path
    nodes             - list of nodes
  -------------------------------------------------------------------*/

typedef struct _path {
    FxU32 cur_size;
    FxU32 max_size;
    AtsNode **nodes;
} AtsPath;

AtsPath* _atsPathNew(char *where, FxU32 line); 
#define atsPathNew() _atsPathNew(__FILE__, __LINE__)
void atsPathExtend(AtsPath *path, int nelem );
void atsPathPushNode(AtsPath *path, const AtsNode *node);
void atsPathPopNode(AtsPath *path);
AtsPath *atsPathCopy(AtsPath *src);
void atsPathFree(AtsPath *path);

/* Scene culling functions
       Only the first two options are currently supported
 */

#define ATS_CULL_NONE   0 /* don't cull */
#define ATS_CULL_NODES  1 /* cull at scene node level, using bounding spheres */
#define ATS_CULL_GEOM   2 /* cull geometry using bounding boxes */
#define ATS_CULL_SORT   4 /* sort geometry based on material */

void atsSelectCamera( AtrCamera *c );
void atsQueryCamera( AtrCamera *c );
void atsCullMode(FxU32 cullMode);
FxU32 atsGetCullMode(void);
void atsCullVolume(AtmPolytope *viewVolume);
AtmPolytope *atsGetCullVolume(void);
FxU32 atsNodeVisible(AtsNode *node);

/*-------------------------------------------------------------------
  Structure: AtsBboard
  Date: 7/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
      An AtsBboard is a subclass of shape objects used to represent such 
      objects as trees, clouds, explosions etc. which have one or more axes 
      of symmetry. Each part is a separate object with its own location and
      whose orientation toward the viewpoint is constrained by the Bboard's
      type. Currently supported types are:

          ATS_BB_AXIAL
          ATS_BB_POINT
          ATS_BB_VIEW

      In all cases the object is rotated so that its local Z direction 
      coincides with the direction of view. An axial billboard rotate about 
      a specified axis (by default the Y axis) and is used to model such 
      objects as trees, signposts etc. The default value works for objects on a
      horizontal ground plane but fails when an object is oriented at an angle, 
      for example, on the incline of a mountain, then the axis of rotation also 
      needs to be oriented at a similar angle. A point billboard maintains its up 
      direction and can be used to model such objects as clouds and explosions.
      A view aligned billboard is rotated so that ist local coordinate system
      coincides with the viewers (cameras) coodinate system and so the object 
      rotates to always stay aligned with the camera even when the camera elevates, 
      pitches and rolls. 

      To rotate the billboard to face the camera, you determine the line between the 
      billboard's origin and the camera's origin; call this the billboard-to-camera 
      line. The axis of rotation and the billboard-to-camera line define a plane. 
      The local z-axis of the billboard is then rotated into that plane, pivoting 
      around the axis of rotation. If the axis of rotation and the 
      billboard-to-camera line are coincident (the same line), then the plane cannot 
      be established, and the rotation results of the billboard are undefined. For 
      example, if the axis of rotation is set to (0,1,0) (the y-axis) and the camera 
      flies over the object, then the object will spin as the camera passes directly 
      over the y-axis. It is undefined at the pole. Another example of this 
      ill-defined behavior occurs when the axis of rotation to (0,0,1) (the z-axis). 
      and sets the camera to look directly down the z-axis of the object. 

    shape        - information common to all shapes 
    flags        - bboard flags
    mode         - type of billboard
    axis         - if an axial billboard axis about which object rotates
    maxPos       - current size of position array
    pos          - position array (1 entry per part)
  -------------------------------------------------------------------*/

#define ATS_BB_AXIAL 0
#define ATS_BB_POINT 1
#define ATS_BB_VIEW  2

typedef struct _AtsBboard {
    AtsShape       shape;
    int            axisKind;   /* special case coordinate axis aligned case */
    int            mode;        /* whether axial (default), point, or view   */
    AtmVector3     axis;        /* if AXIAL, axis about which bboard rotates  */
                                /* if POINT, defines the up (+Y) direction    */
    int            maxPos;
    AtmVector3    *pos;
} AtsBboard;

AtsNode* _atsBboardNew(char *where, FxU32 line); 
#define atsBboardNew() _atsBboardNew(__FILE__, __LINE__)
void    atsBboardPos(AtsNode* pBill, int i, AtmVector3 pos);
void    atsBboardGetPos(AtsNode* pBill, int i, AtmVector3 pos);
void    atsBboardAxis(AtsNode* pBill, AtmVector3 axis);
void    atsBboardGetAxis(AtsNode* pBill, AtmVector3 axis);
void    atsBboardMode(AtsNode* pBill, int mode);
int     atsBboardGetMode(AtsNode* pBill);
AtsType* atsBboardGetType(void);
void    atsBboardNewType(AtsType *ft);

ENDFDECL()
#endif
