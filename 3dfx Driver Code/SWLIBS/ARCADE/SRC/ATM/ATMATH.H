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
** $Date: 10/11/00 7:33:39 PM$ 
**
*/

#ifndef _ATMATH_H_
#	define _ATMATH_H_

/*
**------------------------------------------------------------------------
** TYPES
**------------------------------------------------------------------------
*/

#include <3dfx.h>
#include <atutil.h>
#include <stdio.h>

BEGINFDECL()

/*
**------------------------------------------------------------------------
** MATH CONSTANTS
**------------------------------------------------------------------------
*/

#define ATM_PI (3.1415926535f)
#define ATM_DEGREE (.01745328f)

/*
**------------------------------------------------------------------------
** MATRIX FUNCTIONS
**------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
  Structure: AtmMatrix3x3
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    3x3 Matrix of floats
  Members:
    (elements) 0 1 2
               3 4 5
               6 7 8
  -------------------------------------------------------------------*/
typedef float AtmMatrix3x3[9];

/*-------------------------------------------------------------------
  Structure: AtmMatrix4x4
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    4x4 Matrix of floats
  Members:
    (elements) 0  1  2  3
               4  5  6  7
               8  9 10 11
              12 13 14 15
  -------------------------------------------------------------------*/
typedef float AtmMatrix4x4[16];

void  atmMatrix3x3SetIdentity( AtmMatrix3x3 m );
void  atmMatrix4x4SetIdentity( AtmMatrix4x4 m );
void  atmMatrix3x3SetXRotation( float radians, AtmMatrix3x3 m );
void  atmMatrix3x3SetYRotation( float radians, AtmMatrix3x3 m );
void  atmMatrix3x3SetZRotation( float radians, AtmMatrix3x3 m );  
void  atmMatrix4x4SetXRotation( float radians, AtmMatrix4x4 m );
void  atmMatrix4x4SetYRotation( float radians, AtmMatrix4x4 m );
void  atmMatrix4x4SetZRotation( float radians, AtmMatrix4x4 m );  
void  atmMatrix3x3Cat( AtmMatrix3x3 mDest, 
                       AtmMatrix3x3 mLeft, 
                       AtmMatrix3x3 mRight );
void  atmMatrix4x4Cat( AtmMatrix4x4 mDest, 
                       AtmMatrix4x4 mLeft, 
                       AtmMatrix4x4 mRight );
void  atmMatrix3x3Add( AtmMatrix3x3 mDest, 
                       AtmMatrix3x3 mLeft, 
                       AtmMatrix3x3 mRight );
void  atmMatrix3x3Sub( AtmMatrix3x3 mDest, 
                       AtmMatrix3x3 mLeft, 
                       AtmMatrix3x3 mRight );
void  atmMatrix3x3Scale( AtmMatrix3x3 mDest, 
                         AtmMatrix3x3 mSrc,
                         float scalar );
void  atmMatrix3x3Print( FILE *stream, 
                         const AtmMatrix3x3 m, 
                         FxU32 indent );
void  atmMatrix4x4Print( FILE *stream, 
                         const AtmMatrix4x4 m, 
                         FxU32 indent );
void  atmMatrix3x3Transpose( AtmMatrix3x3 dest, AtmMatrix3x3 src );
void  atmMatrix4x4Transpose( AtmMatrix4x4 dest, AtmMatrix4x4 src );
void  atmMatrix3x3Invert( AtmMatrix3x3 dest, AtmMatrix3x3 src );
void  atmMatrix4x4Invert( AtmMatrix4x4 dest, AtmMatrix4x4 src );
void  atmMatrix3x3Assign( AtmMatrix3x3 dest, AtmMatrix3x3 src );
void  atmMatrix4x4Assign( AtmMatrix4x4 dest, AtmMatrix4x4 src );

/*
**------------------------------------------------------------------------
** VECTOR FUNCTIONS
**------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
  Structure: AtmVector3
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Array of 3 floats representing a 3 dimensional vector.
  Members:
    (elements) [0 1 2] == [ i j k ]
    i - x-axis aligned component
    j - y-axis aligned component
    k - z-axis aligned component
  -------------------------------------------------------------------*/
typedef float AtmVector4[4]; /* i, j, k, l */
typedef float AtmVector3[3]; /* i, j ,k */
typedef float AtmVector2[2]; /* i, j */
typedef float AtmColor[3]; /* r, g, b */
typedef float AtmRotation[4]; /* rotation about an axis i, j, k by theta */

void  atmVector3Set( AtmVector3 v, float i, float j, float k );
void  atmVector2Set( AtmVector2 v, float i, float j );
void  atmVector3Add( AtmVector3 dest, AtmVector3 src1, AtmVector3 src2 );
void  atmVector2Add( AtmVector2 dest, AtmVector2 src1, AtmVector2 src2 );
void  atmVector3Sub( AtmVector3 dest, AtmVector3 src1, AtmVector3 src2 );
void  atmVector2Sub( AtmVector3 dest, AtmVector3 src1, AtmVector3 src2 );
float atmVector3Dot( AtmVector3 v1, AtmVector3 v2 );
void  atmVector3Cross( AtmVector3 dest, AtmVector3 src1, AtmVector3 src2 );
void  atmVector3Normalize( AtmVector3 dest, AtmVector3 src );
void  atmVector3Scale( AtmVector3 dest, AtmVector3 src, float scalar );
float atmVector3Magnitude( const AtmVector3 v );
void  atmVector3Midpoint( AtmVector3 dest, AtmVector3 v1, AtmVector3 v2 ); 
void  atmVector3Matrix3x3Mult( AtmVector3 vDest, 
                               AtmVector3 vSrc, 
                               AtmMatrix3x3 m );
void  atmVector3Assign( AtmVector3 dest, const AtmVector3 src );
void  atmVector2Assign( AtmVector2 dest, const AtmVector2 src );
void  atmVector3Print( FILE *stream, const AtmVector3 v, FxU32 indent );
void  atmVector2Print( FILE *stream, const AtmVector2 v, FxU32 indent );

/*
**------------------------------------------------------------------------
** GENERAL MATH
**------------------------------------------------------------------------
*/
float atmOOSqrt( float number );

/*
**------------------------------------------------------------------------
** Bounding Volume definitions
**------------------------------------------------------------------------
*/

#define ATM_SQ(_x) ((_x)*(_x))

#define ATM_ABS(_x) ( ((_x) > 0 ) ? (_x) : (-(_x)) )

#define ATM_MIN(_x1,_x2) (((_x1) < (_x2)) ? (_x1) : (_x2))

#define ATM_MAX(_x1,_x2) (((_x1) > (_x2)) ? (_x1) : (_x2))

#define ATM_CLAMP(_x, _lo, _hi) \
	(((_x) < (_lo)) ? (_lo) : (_x) > (_hi) ? (_hi) : (_x))


#define atmSqrt(x) ((float)sqrt(x))

/* macro for fast square roots */

#define ATM_SQRT(_x) \
        (((_x) > 0.9996f && (_x) < 1.001f) ? ( 0.5f + 0.5f*(_x) ) : atmSqrt(_x))                       

/* #define ATM_PI       3.14159265358979323846f */

#define ATM_DEG2RAD(x) ((x)*ATM_PI/180.0f)
#define ATM_RAD2DEG(x) ((x)*180.0f/ATM_PI)          

/* Floating point epsilon comparisons */

#define ATM_ALMOST   (1e-4)

#define ATM_ALMOST_EQUAL(v0, v1) \
((v0) >= (v1) - ATM_ALMOST && (v0) <= (v1) + ATM_ALMOST ) 

/* matrix classificatio for optimized transforms */

#define ATM_MAT_TRANS	0x01	/* something in 4th row */
#define ATM_MAT_ROT	0x02	/* rot/skew 3x3 includes off-diagonal */
#define ATM_MAT_SCALE	0x04	/* scaling going on */
#define ATM_MAT_NONORTHO	0x08	/* 3x3 not orthogonal */
#define ATM_MAT_PROJ	0x10	/* something in 4th column */
#define ATM_MAT_HOM_SCALE	0x20	/* [3][3] not 1.0 */
#define ATM_MAT_MIRROR	0x40	/* 3x3 mirrors */

int atmGetMatType(const AtmMatrix4x4 mtx);

void atmVector3Matrix4x4Mult( AtmVector3 vDest, const AtmVector3 v, const AtmMatrix4x4 m );

/*
 * 3 Vectors
 */

#define ATM_VEC3_SET(_dst, _x, _y, _z) \
    (((_dst)[0] = (_x)), \
     ((_dst)[1] = (_y)), \
     ((_dst)[2] = (_z)))

#define ATM_VEC3_COPY(_dst, _v) \
    (((_dst)[0] = (_v)[0]),     \
     ((_dst)[1] = (_v)[1]),     \
     ((_dst)[2] = (_v)[2]))

#define ATM_VEC3_NEGATE(_dst, _v) \
    (((_dst)[0] = -(_v)[0]),     \
     ((_dst)[1] = -(_v)[1]),     \
     ((_dst)[2] = -(_v)[2]))

#define ATM_VEC3_ADD(_dst, _v1, _v2)     \
    (((_dst)[0] = (_v1)[0] + (_v2)[0]), \
     ((_dst)[1] = (_v1)[1] + (_v2)[1]), \
     ((_dst)[2] = (_v1)[2] + (_v2)[2]))

#define ATM_VEC3_SUB(_dst, _v1, _v2)     \
    (((_dst)[0] = (_v1)[0] - (_v2)[0]), \
     ((_dst)[1] = (_v1)[1] - (_v2)[1]), \
     ((_dst)[2] = (_v1)[2] - (_v2)[2]))

#define ATM_VEC3_COMBINE(_dst, _s1, _v1, _s2, _v2)   \
    (((_dst)[0] = (_s1)*(_v1)[0] + (_s2)*(_v2)[0]), \
     ((_dst)[1] = (_s1)*(_v1)[1] + (_s2)*(_v2)[1]), \
     ((_dst)[2] = (_s1)*(_v1)[2] + (_s2)*(_v2)[2]))

#define ATM_VEC3_ADD_SCALED(_dst, _v1, _s2, _v2)   \
    (((_dst)[0] = (_v1)[0] + (_s2)*(_v2)[0]), \
     ((_dst)[1] = (_v1)[1] + (_s2)*(_v2)[1]), \
     ((_dst)[2] = (_v1)[2] + (_s2)*(_v2)[2]))

#define ATM_VEC3_SCALE(_dst, _s, _v) \
    (((_dst)[0] = (_s) * (_v)[0]),     \
     ((_dst)[1] = (_s) * (_v)[1]),     \
     ((_dst)[2] = (_s) * (_v)[2]))

#define ATM_VEC3_DOT(_v0, _v1) \
    ((_v0)[0] * (_v1)[0] +    \
     (_v0)[1] * (_v1)[1] +    \
     (_v0)[2] * (_v1)[2])

#define ATM_VEC3_LENGTH(_v) \
    (atmSqrt(ATM_VEC3_DOT((_v), (_v))))

#define ATM_VEC3_SQR_DISTANCE(_v1, _v2) \
    (ATM_SQ((_v1)[0]-(_v2)[0]) + \
     ATM_SQ((_v1)[1]-(_v2)[1]) + \
     ATM_SQ((_v1)[2]-(_v2)[2]))

#define ATM_VEC3_DISTANCE(_v1, _v2) \
    (atmSqrt(ATM_VEC3_SQR_DISTANCE((_v1), (_v2))))

#define ATM_VEC3_EQUAL(_v1, _v2)	\
	((_v1)[0] == (_v2)[0] && \
	 (_v1)[1] == (_v2)[1] && \
	 (_v1)[2] == (_v2)[2])

#define ATM_VEC3_ALMOST_EQUAL_TOL(_v1, _v2, tol)			\
((_v1)[0] >= (_v2)[0] - (tol) && (_v1)[0] <= (_v2)[0] + (tol) &&	\
 (_v1)[1] >= (_v2)[1] - (tol) && (_v1)[1] <= (_v2)[1] + (tol) &&	\
 (_v1)[2] >= (_v2)[2] - (tol) && (_v1)[2] <= (_v2)[2] + (tol))

#define ATM_VEC3_ALMOST_EQUAL(_v0, _v1) ATM_VEC3_ALMOST_EQUAL_TOL(_v1, _v2, ATM_ALMOST)

/*
**------------------------------------------------------------------------
** TYPES
**------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
  Structure: AtmBox
  Date: 5/17/96
  Implementor(s): mlwp
  Library: AT Math Library
  Description: 
    Defines an axis aligned rectilinear bounding volume by its lower and upper
    limits. A point (x, y, z) is in a box b if

           minx <= x <= maxx, miny <= y <= maxy, minz <= z <= maxz

  Members:
    min          - minimum x, y, z values included in this volume
    max          - maximum x, y, z values included in this volume
  -------------------------------------------------------------------*/

typedef struct {
   AtmVector3 min;
   AtmVector3 max;
} AtmBox;

#define ATM_PT_IN_BOX(_pt, _box) \
   ((_pt)[0] >= (_box)->min[0] && \
    (_pt)[1] >= (_box)->min[1] && \
    (_pt)[2] >= (_box)->min[2] && \
    (_pt)[0] <= (_box)->max[0] && \
    (_pt)[1] <= (_box)->max[1] && \
    (_pt)[2] <= (_box)->max[2])

#define ATM_BOX_EXTEND_BOX(_dst, _box) \
    ((_box)->min[0] < (_dst)->min[0] ? (_dst)->min[0] = (_box)->min[0] : 0, \
     (_box)->min[1] < (_dst)->min[1] ? (_dst)->min[1] = (_box)->min[1] : 0, \
     (_box)->min[2] < (_dst)->min[2] ? (_dst)->min[2] = (_box)->min[2] : 0, \
     (_box)->max[0] > (_dst)->max[0] ? (_dst)->max[0] = (_box)->max[0] : 0, \
     (_box)->max[1] > (_dst)->max[1] ? (_dst)->max[1] = (_box)->max[1] : 0, \
     (_box)->max[2] > (_dst)->max[2] ? (_dst)->max[2] = (_box)->max[2] : 0)

/* 
   Always testing against both bounds is not redundant because
   box may be "empty" in one or more dimensions.
 */

#define ATM_PT_EXTEND_BOX(_dst, _pt) \
    ((_pt)[0] < (_dst)->min[0] ? (_dst)->min[0] = (_pt)[0] : 0, \
     (_pt)[1] < (_dst)->min[1] ? (_dst)->min[1] = (_pt)[1] : 0, \
     (_pt)[2] < (_dst)->min[2] ? (_dst)->min[2] = (_pt)[2] : 0, \
     (_pt)[0] > (_dst)->max[0] ? (_dst)->max[0] = (_pt)[0] : 0, \
     (_pt)[1] > (_dst)->max[1] ? (_dst)->max[1] = (_pt)[1] : 0, \
     (_pt)[2] > (_dst)->max[2] ? (_dst)->max[2] = (_pt)[2] : 0)

void atmBoxEmpty(AtmBox *box);

void atmBoxAroundPoints(AtmBox *box, const int n_points, const int stride, 
                        const float *pts);

void atmBoxAddXformedPoints(AtmBox *box, int n_points, int stride, float *pts,
                            AtmMatrix4x4 m);
void atmBoxAroundBoxes(AtmBox *box, int nboxes, AtmBox *boxes);

void atmBoxXform(AtmBox *dst, AtmBox *src, AtmMatrix4x4 m);

/*-------------------------------------------------------------------
  Structure: AtmSphere
  Date: 5/17/96
  Implementor(s): mlwp
  Library: AT Math Library
  Description: 
    Defines an spherical bounding volume by its center and radius. A point 
    p is in the sphere if 

            |p-center| <= radius 

  Members:
    center       - center of the sphere
    radius       - radius of the sphere
  -------------------------------------------------------------------*/

typedef struct {
   AtmVector3 center;
   float      radius;
} AtmSphere;

#define ATM_PT_IN_SPHERE(_pt, _sp) \
   (ATM_VEC3_SQR_DISTANCE((_sp)->center, (_pt)) <= ATM_SQ((_sp)->radius))

void atmSphereToBox(AtmSphere *sphere, AtmBox *box);

void atmSphereAroundPoints(AtmSphere *sphere, int n_points, int stride, float *pts);
 
void atmSphereAroundBoxes(AtmSphere *sphere, int n_boxes, AtmBox *boxes);

void atmSphereAroundSpheres(AtmSphere *sphere, int n_spheres, AtmSphere *spheres);

void atmSphereOrthoXform(AtmSphere *dst, const AtmSphere *src, const AtmMatrix4x4 m);

/*-------------------------------------------------------------------
  Structure: AtmPlane
  Date: 5/17/96
  Implementor(s): mlwp
  Library: AT Math Library
  Description: 
    Defines a bounding plane.  A plane is defined by a normal (unit vector) 
    and an offset. For any point pt on the plane we have:
  
         pt dot normal = offset     
  
   and the equation of the plane is:

         x dot normal - offset = 0

   The plane subdivides space into two half space. A point is considered to be
   in the half space defined by the plane if:

         p.normal <= offset

   Note: this implies that the half space is in the direction away from the 
   normal (as with all the other bounding volumes).

  Members:
    normal       - unit normal of plane
    offset       - offset of plane from origin
  -------------------------------------------------------------------*/

typedef struct {
    AtmVector3	normal;
    float	offset;
} AtmPlane;

#define ATM_PT_IN_HALF_SPACE(_pt, _pln) \
       (ATM_VEC3_DOT((_pt), (_pln)->normal) <= (_pln)->offset)

FxBool atmPlaneFromPoints(AtmPlane* dst, float *p0, float *p1, float *p2);

void atmPlaneFromNormPt(AtmPlane* dst, AtmVector3 normal, AtmVector3 p) ;

void atmPlaneOrthoXform(AtmPlane* dst, const AtmPlane* pln, const AtmMatrix4x4 m);

float atmDistPointToPlane(const AtmVector3 pt, const AtmPlane *plane);

/*-------------------------------------------------------------------
  Structure: AtmCylinder
  Date: 5/17/96
  Implementor(s): mlwp
  Library: AT Math Library
  Description: 
    Defines a bounding cylinder.  A cylinder is defined by its center point, 
    axis (unit vector), radius and halfLength. A point is in the cylinder if:

        |(p-c).a| < h and |(p-c)-((p-c).a)a| < r

  Members:
    center       - center of cylinder (lies on axis)
    radius       - radius of cylinder
    axis         - axis of cylinder (a unit vector)
    halfLength   - distance from center along axis to top or bottom
  -------------------------------------------------------------------*/

typedef struct {
    AtmVector3	center;
    float	radius;
    AtmVector3	axis;
    float	halfLength;
} AtmCylinder;

void atmCylinderOrthoXform(AtmCylinder* dst, const AtmCylinder* cyl, const AtmMatrix4x4 m);

/*-------------------------------------------------------------------
  Structure: AtmSegment
  Date: 5/17/96
  Implementor(s): mlwp
  Library: AT Math Library
  Description: 
    Segments are used primarily for testing for intersections, segements can be
    tested against the above standard volume types, nodes in the scene graph,
    geometry (primitives) and polygons (triangles).
   
    An AtmSeg represents a line segment starting at position pos, extending for
    a distance length in the direction dir. The direction vector dir must be
    a unit vector and length must be positive.

    When a source object (such as a line segment) is tested against some target
    object the test returns a status which describes the results of the test. 
    Possible values for this test are:
  
           ATM_IS_FALSE	        Segment does not intersect target
           ATM_IS_START_IN      Start point of segment inside target
           ATM_IS_END_IN        End point of segment inside target
           ATM_IS_MAYBE	        Ray possibly intersects target
           ATM_IS_TRUE	        Ray intersects target
           ATM_IS_ALL_IN	Segment is wholey contained by target

    Note: ATM_IS_TRUE will be set if ATM_IS_ALL_IN is set. For performance
    reasons some tests are not exhaustive and so will check if the source object
    is completely inside or completely outside of the target object, and if 
    this test fails they may return ATM_IS_MAYBE. 

  Members:
    pos          - starting point of line segment
    dir          - unit vector in direction of segment
    length       - length of segment (must be positive)
  -------------------------------------------------------------------*/

typedef struct {
    AtmVector3	pos;
    AtmVector3	dir;
    float	length;
} AtmSeg;

#define ATM_IS_FALSE	      0x000
#define ATM_IS_START_IN       0x001
#define ATM_IS_END_IN	      0x002
#define ATM_IS_MAYBE	      0x004 
#define ATM_IS_TRUE	      0x008 
#define ATM_IS_ALL_IN	      0x010

float atmDistPointToLine(const AtmVector3 pt, const AtmSeg *seg);

float atmDistPointToPoint(const AtmVector3 pt1, const AtmVector3 pt2);

void atmSegClip(AtmSeg* dst, const AtmSeg* src, float tnear, float tfar);

FxBool atmSegFromPoints(AtmSeg* dst, float *p0, float *p1);

FxBool atmBoxContainsPoint(const AtmBox *box, const AtmVector3 pt);

FxBool atmSphereContainsPoint(const AtmSphere *sphere, const AtmVector3 pt);

FxBool atmCylinderContainsPoint(const AtmCylinder *cyl, const AtmVector3 pt);

FxBool atmHalfSpaceContainsPoint(const AtmPlane *plane, const AtmVector3 pt);

FxU32 atmSegIsectPlane(const AtmSeg* seg, const AtmPlane* p, float* t);

FxU32 atmSegIsectTri(AtmSeg* seg, float *p0, float *p1, float *p2, float *t);

FxU32 atmHalfSpaceContainsSeg(const AtmPlane *hs, const AtmSeg* seg,
                           float* d1, float* d2);

FxU32 atmBoxContainsSeg(const AtmBox *box, const AtmSeg* seg, 
                        float* tnear, float* tfar);

FxU32 atmSphereContainsSeg(const AtmSphere *sphere, const AtmSeg* seg,
                        float* tnear, float* tfar);

FxU32 atmCylinderContainsSeg(const AtmCylinder *cyl, const AtmSeg* seg,
                        float* tnear, float* tfar);

void atmBoxAroundSpheres(AtmBox *box, int nspheres, AtmSphere *spheres);

void atmBoxToSphere(AtmBox *box, AtmSphere *sphere);

FxU32 atmHalfSpaceContainsBox(const AtmPlane *plane, const AtmBox* box);

FxU32 atmBoxContainsBox(const AtmBox* b1, const AtmBox* b2);

FxU32 atmHalfSpaceContainsSphere(const AtmPlane *plane,const AtmSphere *sphere);

FxU32 atmHalfSpaceContainsCylinder(const AtmPlane *plane, const AtmCylinder *cyl);

FxU32 atmSphereContainsSphere(const AtmSphere *s1, const AtmSphere* s2);

FxU32 atmSphereContainsBox( const AtmSphere *sphere, const AtmBox *box);

void atmCylAroundSegs(AtmCylinder* dst, const AtmSeg** segs, int nseg);

/*-------------------------------------------------------------------
  Structure: AtmPolytope
  Date: 7/5/96
  Implementor(s): mlwp
  Library: AT Math Library
  Description: 
    A AtmPolytope is a set of half spaces whose intersection defines a convex, 
    possibly semi-infinite, volume which may be used for culling and other 
    intersection testing where a tighter bound than a AtmBox, AtmSphere, or 
    AtmCylinder is required.

  Members:
    normal       - unit normal of plane
    offset       - offset of plane from origin
  -------------------------------------------------------------------*/

typedef struct {
    FxU32 numPlanes;
    AtmPlane *planes;
} AtmPolytope;

void atmPolytopeOrthoXform(AtmPolytope* dst, const AtmPolytope *src, 
                        const AtmMatrix4x4 m);

FxBool atmPolytopeContainsPoint(const AtmPolytope* ptp, const AtmVector3 pt);

FxU32 atmPolytopeContainsSphere(const AtmPolytope* ptp, const AtmSphere *sphere);

FxU32 atmPolytopeContainsBox(const AtmPolytope* ptp, const AtmBox *box);

FxU32 atmPolytopeContainsCylinder(const AtmPolytope* ptp, const AtmCylinder *cyl);

FxU32 atmHalfSpaceContainsPolytope(const AtmPlane* hs, const AtmPolytope *ptp);

FxU32 atmPolytopeContainsPolytope(const AtmPolytope* ptp, const AtmPolytope *ptope);

FxU32 atmPolytopeContainsSeg(const AtmPolytope *ptp, const AtmSeg* seg,
                           float* d1, float* d2);

/*-------------------------------------------------------------------
  Structure: AtmCTri
  Date: 5/28/96
  Implementor(s): mlwp
  Library: AT Math Library
  Description: 
    Defines a set of segments to be tested against scene graphs and 
    collision meshes.

  Members:
    plane           plane equation of triangle
    v               the points that make up the triangle
    flat_normal     flat normal for this triangle
    i1, i2          Indices of the lower two valued components of the flat 
                    normal vector.
    mask            id mask of triangle
  -------------------------------------------------------------------*/

typedef struct {
  AtmPlane plane;
  AtmVector3 v[3];
  int i1, i2;
  FxU32 mask;
} AtmCTri;

/*-------------------------------------------------------------------
  Structure: AtmIsect
  Date: 6/3/96
  Implementor(s): mlwp
  Library: AT Math Library
  Description: 
    Defines a segment to be tested against scene graphs and 
    collision meshes and the results of the test.

  Members:
    mode             - how to perform search, what data to return
    seg              - segment to be tested
    mask             - what surfaces are of interest

    hit_status       - did we intersect, what data is valid
    hit_seg          - the clipped segment
    hit_mask         - mask of surface we intersected with
    hit_point        - location of hit
    hit_v[3]         - vertices of triangle we intersected with
    hit_normal       - normal at intersection point
    hit_path         - path to node we intersected
  -------------------------------------------------------------------*/

typedef struct {
    FxU32          mode;
    AtmSeg         seg;
    FxU32          mask;

    FxU32          hit_status;
    AtmSeg         hit_seg;
    FxU32          hit_mask;
    AtmVector3     hit_point;
    AtmVector3     hit_v[3];
    AtmVector3     hit_normal;
    void          *hit_path;
} AtmIsect;

/* search mode */

#define ATM_IS_PRIM               0x0001    /* report intersections at the primitive level */
#define ATM_IS_GEOM               0x0002    /* report intersections at the geometry level */
#define ATM_IS_NODE               0x0004    /* report intersections at the node level */

/* possible values for hit status */

#define ATM_HIT_VALID             0x0001    /* intersection occurred */
#define ATM_HIT_SEG               0x0002    /* segment valid */
#define ATM_HIT_MASK              0x0004    /* mask valid */
#define ATM_HIT_POINT             0x0008    /* point valid */
#define ATM_HIT_VERTS             0x0010    /* triangle vertices valid */
#define ATM_HIT_NORM              0x0020    /* normal valid */
#define ATM_HIT_PATH              0x0040    /* path valid */

/* return values for all traversal callbacks, including intersection */

#define ATM_TRAV_CONT		0
#define ATM_TRAV_TERM		1
#define ATM_TRAV_PRUNE		2

/* define modes for traversing tree */

#define ATM_TRAV_SELF                     0x0000
#define ATM_TRAV_DESCEND                  0x0001
#define ATM_TRAV_DESCEND_SET_FROM_CHILD   0x0002
#define ATM_TRAV_OP                       0x00f0
#define ATM_TRAV_SET                      0x0010
#define ATM_TRAV_OR                       0x0020
#define ATM_TRAV_AND                      0x0040

/* flag indicating all values */

#define ATM_NODE_MASK_ALL ~0UL

/*-------------------------------------------------------------------
  Structure: AtmGridCell
  Date: 5/28/96
  Implementor(s): mlwp
  Library: AT Math Library
  Description: 
    One cell within the collision cache

  Members:
    num_tris        number of triangles within this cell
    tris            list of the triangles within this cell
  -------------------------------------------------------------------*/

typedef struct
{
  int num_tris;
  AtmCTri **tris;
} AtmGridCell;

/*-------------------------------------------------------------------
  Structure: AtmCMesh
  Date: 5/28/96
  Implementor(s): mlwp
  Library: AT Math Library
  Description: 
    Defines a collision cache to perform optimized line intersections 
    with. 

  Members:
    int_x_size      number of cells in x dimension
    int_z_size      number of cells in z dimension
    mask            id mask for checking material type
    bounds          bounding volume for this collision cache
    cells           cells making up collision grid
    grid_unit_size  size of grid cell in local coordinates
    num_tris        current number of triangles in collision grid
    max_tris        maximum number of triangles in collision grid
    tri_list        list of triangles in collision grid
  -------------------------------------------------------------------*/

typedef struct {
    FxBool gridMesh;
    FxU32 int_x_size, int_z_size;
    FxU32 mask;
    AtmBox bounds;
    AtmGridCell **cells;
    float grid_unit_size;
    FxU32 num_tris;
    FxU32 max_tris;
    AtmCTri *tri_list;
} AtmCMesh;

AtmCMesh *atmCMeshNew();

void atmCMeshDefault( AtmCMesh *cmesh );

void atmCMeshDelete(AtmCMesh *cmesh);

void atmCMeshPrint(const AtmCMesh *cmesh, FILE *stream, FxU32 indent, FxU32 verbose);

void atmCMeshDraw(const AtmCMesh *cmesh, FxBool drawFilled);

AtmCMesh *atmCMeshClone(AtmCMesh *cmesh);

void atmCMeshGridSize(AtmCMesh *cmesh, const float grid_unit_size);

void atmCMeshIDMask(AtmCMesh *cmesh, FxU32 idMask);

FxBool atmCMeshAddTriangle(AtmCMesh *cmesh, AtmVector3 v0, AtmVector3 v1,
                           AtmVector3 v2);

FxBool atmCMeshClose(AtmCMesh *cmesh);

FxBool atmCMeshIsectSeg(const AtmCMesh *cmesh, AtmIsect *isect);

FxBool atmCMeshFindBelow(AtmCMesh *cmesh, AtmIsect *isect);

FxBool atmCMeshFind( AtmCMesh *cmesh, AtmIsect *isect);

void _atsCMeshPopulateGridCells( AtmCMesh *cmesh );

void _atsCMeshAllocateGridCells( AtmCMesh *cmesh );

ENDFDECL()

#endif
