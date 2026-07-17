#ifndef TD_H
#define TD_H
/*
 *  td.h                                               08/26/94  0.00  NDR
 *                                                     09/07/94  1.00  NDR
 *                                                     09/08/94  2.00  NDR
 *                                                     11/15/94  3.00  NDR
 *                                                     01/03/95  3.10  NDR
 *                                                     09/01/95  4.00  NDR
 *
 *    Three-D Object Library - Library of 3-D object
 *    manipulation routines.
 *
 *    Header File
 *
 *    author: Nathan Drew Robins
 *    email: narobins@es.com
 *
 *
 *    Copyright 1996 by Evans & Sutherland Computer Corporation.  
 *    ALL RIGHTS RESERVED. 
 *    
 *    Permission to use, copy, modify, and distribute this software 
 *    for any purpose and without fee is hereby granted, provided 
 *    that the above copyright notice appear in all copies and that 
 *    both the copyright notice and this notice appear in supporting
 *    documentation.  You may not use the name "Evans & Sutherland 
 *    Computer Corporation" or any trademark of Evans & Sutherland 
 *    Computer Corporation in advertising or publicity pertaining 
 *    to the software without specific, written prior permission. 
 *    
 *    THIS SOFTWARE AND ANY ASSOCIATED MATERIALS IS PROVIDED TO YOU 
 *    "AS-IS" AND WITHOUT REPRESENTATION OR WARRANTY OF ANY KIND, 
 *    EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, 
 *    ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 *    PURPOSE OR NONINFRINGEMENT OF ANY PATENT, COPYRIGHT, TRADE 
 *    SECRET OR OTHER RIGHT.  EVANS & SUTHERLAND COMPUTER CORPORATION 
 *    DOES NOT WARRANT THAT USE OF THE SOFTWARE WILL BE UNINTERRUPTED
 *    OR ERROR-FREE, OR THAT DEFECTS IN THE SOFTWARE WILL BE CORRECTED.
 *    
 *    IN NO EVENT SHALL EVANS & SUTHERLAND COMPUTER CORPORATION BE 
 *    LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT, SPECIAL, INCIDENTAL, 
 *    INDIRECT OR CONSEQUENTIAL  DAMAGES OF ANY KIND, OR ANY DAMAGES 
 *    WHATSOEVER, INCLUDING WITHOUT LIMITATION, LOSS OF PROFIT, LOSS 
 *    OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF THIRD PARTIES, 
 *    WHETHER OR NOT EVANS & SUTHERLAND COMPUTER CORPORATION  HAS 
 *    BEEN ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED 
 *    AND ON ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION 
 *    WITH THE POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *    
 *    US Government Users Restricted Rights 
 *    Use, duplication, or disclosure by the Government is subject 
 *    to restricted rights set forth in FAR 52.227.19(c)(2) or 
 *    subparagraph (c)(1)(ii) of the Rights in Technical Data and 
 *    Computer Software clause at DFARS 252.227-7013 and/or in 
 *    similar or successor clauses in the FAR or the DOD or NASA FAR 
 *    Supplement. Contractor/manufacturer is Evans & Sutherland Computer 
 *    Corporation, 600 Komas Drive, Salt Lake City, UT 84108. 
 *    
 *    Use of this software outside the United States is subject to 
 *    US export laws and regulations.  You agree to comply with all 
 *    such laws and regulations.
 *    
 *    This statement is made under and shall be construed and 
 *    governed by the laws of the State of Utah, without regard to 
 *    conflict of laws principles.
 * 
 */


/* includes */
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>


/* defines */
#define TD_VERTEX                 (0)       /* send vertex data */
#define TD_NORMAL                 (1 << 1)  /* send normal data */
#define TD_TEXVERTEX              (1 << 2)  /* send texture vertex data */
#define TD_COLOR                  (1 << 3)  /* send a color */
#define TD_MATERIAL               (1 << 4)  /* send material data */
#define TD_ALL                    (0xFFFF)  /* send all the above */

/* enumerants */

typedef enum
{
    TD_INFO = 0,   /* information message */
    TD_WARNING,    /* warning message */
    TD_ERROR,      /* error message */
    TD_NOMEMORY,   /* out of memory message */
    TD_FATAL       /* fatal error message */
} TDmessage;

/* primitive enumerants */
typedef enum
{
    TD_POINTS = GL_POINTS,   /* use the same numbering as OpenGL */
    TD_LINES,          
    TD_LINE_LOOP,      
    TD_LINE_STRIP,     
    TD_TRIANGLES,      
    TD_TRIANGLE_STRIP, 
    TD_TRIANGLE_FAN,   
    TD_QUADS,          
    TD_QUAD_STRIP,     
    TD_POLYGON         
} TDprimitive;

/* boolean enumerants */
typedef enum
{
    TD_FALSE = 0,
    TD_TRUE
} TDboolean;


/* structs */

/* translation data structure - (x, y, z) translation */
typedef struct
{
    GLfloat x;        /* translation in X direction */
    GLfloat y;        /* translation in Y direction */
    GLfloat z;        /* translation in Z direction */
    GLfloat x_delta;  /* translation delta in X direction */
    GLfloat y_delta;  /* translation delta in Y direction */
    GLfloat z_delta;  /* translation delta in Z direction */
} TDtranslation;

/* rotation data structure - (i, j, k) rotation */
typedef struct 
{
    GLfloat i;        /* rotation around X axis */
    GLfloat j;        /* rotation around Y axis */
    GLfloat k;        /* rotation around Z axis */
    GLfloat i_delta;  /* rotation delta around X axis */
    GLfloat j_delta;  /* rotation delta around Y axis */
    GLfloat k_delta;  /* rotation delta around Z axis */
} TDrotation;

/* color data structure */
typedef struct
{
    GLfloat r;  /* red component of color */
    GLfloat g;  /* green component of color */
    GLfloat b;  /* blue component of color */
    GLfloat a;  /* alpha component of color */
} TDcolor;

/* texture image data structure */
typedef struct
{
    GLint width;            /* width of texture image */
    GLint height;           /* height of texture image */
    GLubyte *data;          /* texture image data */
} TDteximage;

/* material data structure */
typedef struct _TDmaterial
{
    char *name;      /* name of material */

    TDcolor color;       /* color */
    TDcolor ambient;     /* ambient component */
    TDcolor diffuse;     /* diffuse component */
    TDcolor specular;    /* specular component */
    TDcolor emmission;   /* emmissive component */
    GLfloat shininess;   /* shininess component */

    /* TODO - add mask for turning ambient/diffuse etc off/on, also, 
     *        texture clamping modes, and Tf (color masks)
     */

    TDteximage *teximage;      /* pointer to texture image */
    GLint dlist;               /* display list for the texture */

    struct _TDmaterial *next;  /* pointer to the next material */
} TDmaterial;

/* vertex data structure - (x, y, z, w) point */
typedef struct
{
    GLfloat x;  /* X position */
    GLfloat y;  /* Y position */
    GLfloat z;  /* Z position */
    GLfloat w;  /* W position */
} TDvertex;

/* texture vertex data structure - (u, v) */
typedef struct
{
    GLfloat u;  /* U value */
    GLfloat v;  /* V value */
    GLfloat w;  /* W value */
} TDtexvertex;

/* vertex normal data structure - (i, j, k) point */
typedef struct
{
    GLfloat i;  /* I vector */
    GLfloat j;  /* J vector */
    GLfloat k;  /* K vector */
} TDnormal;

/* face data structure (connectivity - indexes into vertices[]) */
typedef struct _TDface
{
    int num_vertices;      /* number of vertices in this face */
    int num_texvertices;   /* number of texture vertices in this face */
    int num_normals;       /* number of normals in this face */

    int *vertices;         /* pointer to a list of vertex indices */
    int *texvertices;      /* pointer to a list of texture vertex indices */
    int *normals;          /* pointer to a list of normal indices */

    TDnormal normal;       /* facet normal for this face */

    struct _TDface *next;  /* pointer to the next face (NULL = last) */
} TDface;

/* group data structure */
typedef struct _TDgroup
{
    char *name;                 /* name of this group */

    TDmaterial *material;      /* pointer to the material for this group */

    int num_faces;             /* number of faces (polygons) */
    TDface *faces;             /* pointer to the face (polygon) data */

    /* structures needed for decimation */
    int num_triangles;         /* number of triangles */
    TDface *triangles;         /* pointer to the triangle data */
    int num_quads;             /* number of quads */
    TDface *quads;             /* pointer to the quad data */

    GLint dlist;               /* display list for this group */

    struct _TDgroup *next;     /* pointer to the next group (NULL = last) */
} TDgroup;

/* object data structure */
typedef struct _TDobject
{
    char *name;                /* name of this object */

    TDvertex center;           /* the center of the object */
    TDvertex maximum;          /* maximum values (bounding box info) */
    TDvertex minimum;          /* minimum values (bounding box info) */

    GLfloat width;             /* width of object (X dimension) */
    GLfloat height;            /* height of object (Y dimension) */
    GLfloat depth;             /* depth of object (Z dimension) */

    TDtranslation translation; /* translation of object */
    TDrotation rotation;       /* rotation of object */

    int num_vertices;          /* number of vertices */
    int num_texvertices;       /* number of texture vertices */
    int num_normals;           /* number of normals */

    TDvertex *vertices;        /* array of vertices */
    TDtexvertex *texvertices;  /* array of texture vertices */
    TDnormal *normals;         /* array of vertex normals */

    int num_materials;         /* number of materials */
    TDmaterial *materials;     /* pointer to the materials in object */

    int num_groups;            /* number of groups in object */
    TDgroup *groups;           /* pointer to the groups in object */

    int num_faces;             /* number of faces in object */

    GLint display_list;        /* the display list number for this object */

    struct _TDobject *next;    /* next object in list - currently not used */
} TDobject;


/* functions */

/*
 *  tdDraw()
 *
 *    mode - primitive drawing mode.  Can be one of the following:
 *     TD_POINTS          - vertexes only only
 *     TD_LINES           - lines drawn around each polygon
 *     TD_LINE_LOOP       - line loop drawn around each polygon
 *     TD_LINE_STRIP      - line strip drawn around each polygon
 *     TD_TRIANGLES       - triangles (must use tdGenTriangles())
 *     TD_TRIANGLE_STRIP  - triangle strips (UNIMPLEMENTED)
 *     TD_TRIANGLE_FAN    - triangle fan 
 *     TD_QUADS           - quadrilaterals (must use tdGenQuads())
 *     TD_QUAD_STRIP      - quad strips (UNIMPLEMENTED)
 *     TD_POLYGON         - polygons 
 *    mask - a mask indicating what type(s) of data to use to draw with.
 *    A bitwise OR of one or more of the following:
 *     TD_VERTEX                 - send vertex data (default)
 *     TD_NORMAL                 - send normal data
 *     TD_TEXVERTEX              - send texture vertex data
 *     TD_MATERIAL               - send material data
 *     TD_ALL                    - send all of the above
 *
 *    RETURNs TD_TRUE if all went well, TD_FALSE if an error occurred
 */
TDboolean tdDraw(TDobject *object, TDprimitive mode, int mask)
;

/*
 *  tdGenDList()
 * 
 *    Generates a display list out of an object
 *
 *    mode - primitive drawing mode.  Can be one of the following:
 *     TD_POINTS          - vertexes only only
 *     TD_LINES           - lines drawn around each polygon
 *     TD_LINE_LOOP       - line loop drawn around each polygon
 *     TD_LINE_STRIP      - line strip drawn around each polygon
 *     TD_TRIANGLES       - triangles (must use tdGenTriangles())
 *     TD_TRIANGLE_STRIP  - triangle strips (UNIMPLEMENTED)
 *     TD_TRIANGLE_FAN    - triangle fan 
 *     TD_QUADS           - quadrilaterals (must use tdGenQuads())
 *     TD_QUAD_STRIP      - quad strips (UNIMPLEMENTED)
 *     TD_POLYGON         - polygons 
 *    mask - a mask indicating what type(s) of data to use to draw with.
 *    A bitwise OR of one or more of the following:
 *     TD_VERTEX                 - send vertex data (default)
 *     TD_NORMAL                 - send normal data
 *     TD_TEXVERTEX              - send texture vertex data
 *     TD_MATERIAL               - send material data
 *     TD_ALL                    - send all of the above
 *
 *    RETURNs the list number allocated if all went well,
 *    otherwise, returns 0.
 */
int tdGenDList(TDobject *object, TDprimitive mode, int mask)
;

/*
 *  tdWriteObject()
 *
 *    Writes in object data in any supported format.  It uses the
 *    extension on the pathname to decide what type of file format
 *    to write in the file.  Supported extensions (file formats) are:
 *    .obj  - Wavefront .obj file format + .mtl material files. 
 *    .aoff - DEC Object File Format (OFF) + .geom, etc. files (UNIMPLEMENTED)
 *    .dxf  - Drawing Interchange File format (Autodesk) (UNIMPLEMENTED)
 *    .iob  - Lightwave 3D Object File format (UNIMPLEMENTED)
 *    .tddd - Imagine File format (UNIMPLEMENTED)
 *    
 *    pathname - filename of the file the object is to be written in.  It
 *    must be of the form path + name + extension.  Where path is the name of
 *    the path (full pathname if needed, but will go from current working 
 *    directory) and name is the actual name of the object and extension is
 *    one of the valid extensions above.
 *
 *    RETURNs TD_TRUE all went well, TD_FALSE otherwise.
 */
TDboolean tdWriteObject(TDobject *object, char *pathname)
;

/*
 *  tdReadObject()
 * 
 *    Reads in object data in any supported format.  It uses the
 *    extension on the pathname to decide what type of file format
 *    the data is in.  Supported extensions (file formats) are:
 *    .obj  - Wavefront .obj file format + .mtl material files. 
 *    .aoff - DEC Object File Format (OFF) + .geom, etc. files (UNIMPLEMENTED)
 *    .dxf  - Drawing Interchange File format (Autodesk) (UNIMPLEMENTED)
 *    .iob  - Lightwave 3D Object File format (UNIMPLEMENTED)
 *    .tddd - Imagine File format (UNIMPLEMENTED)
 *    
 *    pathname - filename of the object to be read in.  It must be of the
 *    form path + name + extension.  Where path is the name of the path
 *    (full pathname if needed, but will go from current working directory)
 *    and name is the actual name of the object and extension is one of the
 *    valid extensions above.
 *
 *    RETURNs a pointer to the object if all went well,
 *    otherwise, returns NULL.
 */
TDobject *tdReadObject(char *pathname)
;

/*
 *  tdReadTeximage()
 *
 *    Reads a texture file in any supported format.  It uses the
 *    extension on the filename to decide what type of file format
 *    the data is in.  Supported extensions (file formats) are:
 *    .rgb  - Iris RGB format
 *    .gif  - Graphics Interchange Format (UNIMPLEMENTED)
 *    .jpeg - JPeg format (UNIMPLEMENTED)
 *    .tif  - TIFF format (UNIMPLEMENTED)
 *
 *    filename - name of the file containing the texture data of the
 *    form path + name + extension.  Where path is the name of the
 *    path (full pathname if needed, but will go from current working
 *    directory) and name is the actual name of the file and extension
 *    is one of the valid extensions above.
 */
TDteximage *tdReadTeximage(char *filename)
;

/*
 *  tdGenTriangles()
 *
 *    Generates triangles from polygonal data.  It does this by
 *    marching along the edge of the polygon, and connects the first
 *    vertex to the next two, then connects the first vertex to 
 *    vertexes 3 and 4 and so on. (Essentially making a triangle fan
 *    out of each polygon.)  This algorithm assumes that the
 *    polygons are convex.
 *
 *    object - a pointer to a valid object with polygonal data
 *
 *    RETURNs the number of triangles the object was broken into if
 *    all went well, otherwise, returns 0.
 */
int tdGenTriangles(TDobject *object)
;

/*
 *  tdGenQuads()
 *
 *    Generates quadrilaterals (quads) from polygonal data.  It does this by
 *    marching along the edge of the polygon, and connects the first
 *    vertex to the next three, then connects the first vertex to 
 *    vertexes 4 and 5 and so on.  Unfortunately, since a polygon can
 *    consist of an odd number of vertices, this algorithm generates
 *    some degenerate quads, that is, quads that have repeated vertices.
 *    (Basically a triangle with the last vertex repeated).  This algorithm
 *    assumes that the polygons are convex.
 *
 *    object - a pointer to a valid object with polygonal data
 *
 *    RETURNs the number of quads the object was broken into.
 */
int tdGenQuads(TDobject *object)
;

/*
 *  tdGenSmoothNormals()
 *
 *    Generates smooth vertex normals.  Overrides any existing normals in
 *    the object.  Generates smooth normals by calculating the cross
 *    product of two vectors in the same plane as the face (effectively
 *    getting the normal of the face), and uses this normal as the 
 *    normal for all vertices in the face.  Assuming that vertices are
 *    shared between faces, some of the vertex normals for a given face
 *    will be written more than once with different values each time,
 *    which gives a smoother appearance to the object.
 *    
 *    object - a pointer to a valid object with vertices
 *
 *    RETURNs a pointer to the normals if all went well,
 *    otherwise, returns NULL.
 *
 */
TDnormal *tdGenSmoothNormals(TDobject *object)
;

/*
 *  tdGenSpheremapTexvertices()
 *
 *    Generates texture coordinates according to a spherical projection
 *    of the texture map.  Sometimes referred to as spheremap, or
 *    reflection map texture coordinates.  It generates these by
 *    using the normal to calculate where that vertex would map onto
 *    a sphere.  Since it is impossible to map something flat perfectly
 *    onto something spherical, there is distortion at the poles.  This
 *    particular implementation causes the poles along the X axis to be
 *    distorted.
 *
 *    object - a pointer to a valid object with normals
 *
 *    RETURNs a pointer to the vertices if all went well,
 *    otherwise returns NULL.
 *
 */
TDtexvertex *tdGenSpheremapTexvertices(TDobject *object)
;

/*
 *  tdGenLinearTexvertices()
 *
 *    Generates texture coordinates according to a linear projection
 *    of the texture map.  It generates these by linearly mapping the
 *    vertices onto a square.
 *
 *    object - a pointer to a valid object with normals
 *
 *    RETURNs a pointer to the vertices if all went well,
 *    otherwise returns NULL.
 */
TDtexvertex *tdGenLinearTexvertices(TDobject *object)
;

/*
 *  tdVerbose()
 *
 *    Set the verbosity level (all messages on by default)
 *
 *    verbose - boolean value, one of the following:
 *    GL_TRUE   - print out everything (default)
 *    GL_FALSE  - only print out error messages (no warning or info)
 */
void tdVerbose(TDboolean verbose)
;

/*
 *  tdNewObject()
 * 
 *    Allocates memory for an object data structure, and fills the new
 *    structure with default values.
 *
 *    name - name of the new object, should not be NULL.
 *
 *    RETURNs a new object pointer if all went well,
 *    otherwise an error message is printed, and NULL returned.
 *
 */
TDobject *tdNewObject(char *name)
;

/*
 *  tdDeleteObject()
 * 
 *    Frees memory allocated for an object data structure
 *
 *    object - pointer to an object to be freed.
 *
 */
void tdDeleteObject(TDobject *object)
;

/*
 *  tdSize()
 *
 *    Translates an object to the origin (0,0,0) and
 *    scales it by a scalefactor.
 *
 *    object - a pointer to a valid object
 *    scalefactor - factor to scale object by. If it 
 *    is zero (0.0), then the object is scaled by the
 *    2.0 / maximum dimension of the object (this causes the
 *    object to be scaled to between -1.0 and 1.0 in all
 *    dimensions uniformly (that is, the aspect ratio is
 *    preserved).
 *
 *    RETURNs the scalefactor used.  If object is NULL,
 *    0.0 is returned.
 *
 */
float tdSize(TDobject *object, float scalefactor)
;

/*
 *  tdGetError()
 *
 *    Get the last error (if one)
 *
 */
TDmessage tdGetError()
;

#endif /* TD_H */
