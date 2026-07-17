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
** $Date: 10/11/00 7:33:40 PM$ 
**
*/

#include <math.h>
#include <assert.h>
#include <float.h>
#include <atmath.h>

/*-------------------------------------------------------------------
  Function: atmBoxEmpty
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Set bounding box to be NULL (such that any point extends it) 
  Arguments:
    box - bounding box
  Return:
    Nothing
  -------------------------------------------------------------------*/


void atmBoxEmpty(AtmBox *box) { 
  box->min[0] = box->min[1] = box->min[2] = FLT_MAX;
  box->max[0] = box->max[1] = box->max[2] = -FLT_MAX;
}

/*-------------------------------------------------------------------
  Function: atmBoxAroundPoints
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Compute a bounding box surounding a set of points. This function 
    adds the volumes of the specified boxes into the volume of the
    box. If you wish to compute the volume occupied by just these
    boxes call atsBoxEmpty first.
  Arguments:
    box  - bounding box
    n_point - number of points
    stride  - number of floats in a vertex (assumes x,y,z are first)
    pts     - the points
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atmBoxAroundPoints(AtmBox *box, const int n_points, 
                         const int stride, const float *pts)
{ 
   int i;

    for (i=0; i<n_points; i++) {
       if (box->min[0] > pts[0])  box->min[0] = pts[0];
       if (box->min[1] > pts[1])  box->min[1] = pts[1]; 
       if (box->min[2] > pts[2])  box->min[2] = pts[2];
 
       if (box->max[0] < pts[0])  box->max[0] = pts[0];
       if (box->max[1] < pts[1])  box->max[1] = pts[1];
       if (box->max[2] < pts[2])  box->max[2] = pts[2]; 
       pts += stride;
    }
}

/*-------------------------------------------------------------------
  Function: atmBoxAddXformedPoints
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Transform the specified points by the specified matrix and then
    add them into the specified bounding box.
  Arguments:
    box  - bounding box
    n_point - number of points
    stride  - number of floats in a vertex (assumes x,y,z are first)
    pts     - the points
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atmBoxAddXformedPoints(AtmBox *box, int n_points, int stride, float *pts,
                            AtmMatrix4x4 m)
{ 
   int i;
   AtmVector3 tmp;

   for (i=0; i<n_points; i++) {
      atmVector3Matrix4x4Mult( tmp, pts, m );

      if (box->min[0] > tmp[0])  box->min[0] = tmp[0];
      if (box->min[1] > tmp[1])  box->min[1] = tmp[1]; 
      if (box->min[2] > tmp[2])  box->min[2] = tmp[2];

      if (box->max[0] < tmp[0])  box->max[0] = tmp[0];
      if (box->max[1] < tmp[1])  box->max[1] = tmp[1];
      if (box->max[2] < tmp[2])  box->max[2] = tmp[2]; 

      pts += stride;
   }
}

/*-------------------------------------------------------------------
  Function: atmBoxAroundBoxes
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Compute a bounding box surounding a set of boxes. This function 
    adds the volumes of the specified boxes into the volume of the
    box. If you wish to compute the volume occupied by just these
    boxes call atsBoxEmpty first.
  Arguments:
    box     - bounding box
    nboxes  - number of boxes in list
    boxes   - list of boxes
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atmBoxAroundBoxes(AtmBox *box, int nboxes, AtmBox *boxes)
{ 
    int i;

    for ( i = 0; i < nboxes; i++ ) {
        if (box->min[0] > boxes->min[0])  box->min[0] = boxes->min[0]; 
        if (box->min[1] > boxes->min[1])  box->min[1] = boxes->min[1];
        if (box->min[2] > boxes->min[2])  box->min[2] = boxes->min[2];
      
        if (box->max[0] < boxes->max[0])  box->max[0] = boxes->max[0];
        if (box->max[1] < boxes->max[1])  box->max[1] = boxes->max[1];
        if (box->max[2] < boxes->max[2])  box->max[2] = boxes->max[2];
        boxes++;
    }
}

/*-------------------------------------------------------------------
  Function: atmBoxAroundSpheres
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Compute a bounding box surrounding a list of spheres. This function 
    adds the volumes of the specified spheres into the volume of the
    box. If you wish to compute the volume occupied by just these
    spheres call atsBoxEmpty first.
  Arguments:
    box       - bounding box
    nspheres  - number of spheres in list
    spheres   - list of spheres
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atmBoxAroundSpheres(AtmBox *box, int nspheres, AtmSphere *spheres)
{ 
    int i;
    float minx, miny, minz, maxx, maxy, maxz;

    for ( i = 0; i < nspheres; i++ ) {
        minx = spheres->center[0]-spheres->radius;
        maxx = spheres->center[0]+spheres->radius;
        miny = spheres->center[1]-spheres->radius;
        maxy = spheres->center[1]+spheres->radius;
        minz = spheres->center[2]-spheres->radius;
        maxz = spheres->center[2]+spheres->radius;

        if (box->min[0] > minx)  box->min[0] = minx; 
        if (box->min[1] > miny)  box->min[1] = miny;
        if (box->min[2] > minz)  box->min[2] = minz;
      
        if (box->max[0] < maxx)  box->max[0] = maxx;
        if (box->max[1] < maxy)  box->max[1] = maxy;
        if (box->max[2] < maxz)  box->max[2] = maxz;
        spheres++;
    }
}

/*-------------------------------------------------------------------
  Function: atmVector3Matrix4x4Mult
  Date: 5/17/96
  Implementor(s): mlwp
  Library: AT Math
  Description:
    Multiply a 3D vector by a 4x4 Matrix
  Arguments:
    m - matrix
    v - vector
    vDest - destination vector
  Return:
    none
  -------------------------------------------------------------------*/
void atmVector3Matrix4x4Mult( AtmVector3 vDest, const AtmVector3 v, const AtmMatrix4x4 m ) {
    AtmVector3 tmp;
#ifdef AT_DEBUGGING
    if ( !vDest || !v || !m ) 
        atuError( FXTRUE, "atmVector3Matrix4x4Mult(): Invalid parameter.\n" );    
#endif

    tmp[0] = v[0] * m[0]  + v[1] * m[4]  + v[2] * m[8] + m[12];

    tmp[1] = v[0] * m[1]  + v[1] * m[5]  + v[2] * m[9] + m[13];

    vDest[2] = v[0] * m[2]  + v[1] * m[6]  + v[2] * m[10] + m[14];

    vDest[0] = tmp[0];
    vDest[1] = tmp[1];
    return;
}

/*-------------------------------------------------------------------
  Function: atmBoxXform
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Transform the bounding box,  putting another (aligned) bounding box 
    around - it is exact for cubes but somewhat larger for "round" objects.
  Arguments:
    dst     - xformed bounding box
    src     - original bounding box
    m       - the transform
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atmBoxXform(AtmBox *dst, AtmBox *src, AtmMatrix4x4 m)
{
  float pts[24], *p;
  AtmVector3 q;
  int i;

  for ( i = 0, p = pts;  i < 8;  i++) {
    q[0] = (i & 4) ? src->min[0] : src->max[0]; 
    q[1] = (i & 2) ? src->min[1] : src->max[1]; 
    q[2] = (i & 1) ? src->min[2] : src->max[2]; 
    
    /* Transform point */

    p[0] = q[0] * m[0]  + q[1] * m[4]  + q[2] * m[8] + m[12];

    p[1] = q[0] * m[1]  + q[1] * m[5]  + q[2] * m[9] + m[13];

    p[2] = q[0] * m[2]  + q[1] * m[6]  + q[2] * m[10] + m[14];

    p += 3;
  }
    
  atmBoxEmpty(dst);
  atmBoxAroundPoints (dst, 8, 3, pts);
}

/*-------------------------------------------------------------------
  Function: atmBoxToSphere
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Given a bounding box convert it to a bounding sphere
  Arguments:
    box     - bounding box
    sphere  - bounding sphere
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atmBoxToSphere(AtmBox *box, AtmSphere *sphere) {
   AtmVector3 rad;

   rad[0] = 0.5f*(box->max[0]-box->min[0]);
   rad[1] = 0.5f*(box->max[1]-box->min[1]);
   rad[2] = 0.5f*(box->max[2]-box->min[2]);

   sphere->center[0] = rad[0]+box->min[0];
   sphere->center[1] = rad[1]+box->min[1];
   sphere->center[2] = rad[2]+box->min[2];

   sphere->radius = (float)sqrt( rad[0]*rad[0] + rad[1]*rad[1] + rad[2]*rad[2]);
}

/*-------------------------------------------------------------------
  Function: atmSphereToBox
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Given a bounding sphere convert it to a bounding box
  Arguments:
    sphere  - bounding sphere
    box     - bounding volume
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atmSphereToBox(AtmSphere *sphere, AtmBox *box) {
    box->min[0] = sphere->center[0]-sphere->radius;
    box->min[1] = sphere->center[1]-sphere->radius;
    box->min[2] = sphere->center[2]-sphere->radius;

    box->max[0] = sphere->center[0]+sphere->radius;
    box->max[1] = sphere->center[1]+sphere->radius;
    box->max[2] = sphere->center[2]+sphere->radius;
}

/*-------------------------------------------------------------------
  Function: atmSphereAroundPoints
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Compute a bounding sphere that surrounds a set of points
  Arguments:
    sphere   - the computed bounding sphere
    n_points - number of points
    stride  - number of floats in a vertex (assumes x,y,z are first)
    pts     - the points
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atmSphereAroundPoints(AtmSphere *sphere, int n_points, int stride, float *pts)
{ 
    float *p;
    int i;
    AtmVector3 c;
    float r, max_r, dx, dy, dz;

    if ( n_points == 0 ) {
        sphere->radius = 0.0f;
        return;
    }

    /* compute a center */

    c[0] = 0.0f; c[1] = 0.0f; c[2] = 0.0f;

    for (i=0, p = pts; i<n_points; i++, p++) {
        c[0] += p[0]; c[1] += p[1], c[2] += p[2];
        p += stride;
    }

    sphere->center[0] = c[0] /= n_points; 
    sphere->center[0] = c[1] /= n_points; 
    sphere->center[0] = c[2] /= n_points;

    /* compute a radius */

    max_r = 0.0f;

    for (i=0, p = pts; i<n_points; i++, p++) {
        dx = p[0]-c[0]; dy = p[1]-c[1]; dz = p[2]-c[2]; 
        r = dx*dx+dy*dy+dz*dz;
        if ( r > max_r ) max_r = r;
        p += stride;
    }
  
    sphere->radius = (float)sqrt(max_r);
}

/*-------------------------------------------------------------------
  Function: atmSphereAroundBoxes
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Compute a bounding sphere that surrounds a set of boxes
  Arguments:
    sphere   - the computed bounding sphere
    n_boxes  - number of boxes
    boxes    - the list of boxes
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atmSphereAroundBoxes(AtmSphere *sphere, int n_boxes, AtmBox *boxes)
{ 
    AtmBox *b;
    int i;
    AtmVector3 c;
    float r, max_r, dx, dy, dz;

    if ( n_boxes == 0 ) {
        sphere->radius = 0.0f;
        return;
    }

    /* compute a center */

    c[0] = 0.0f; c[1] = 0.0f; c[2] = 0.0f;

    for (i=0, b = boxes; i<n_boxes; i++, b++) {
        c[0] += b->min[0]+b->max[0];
        c[1] += b->min[1]+b->max[1];
        c[2] += b->min[2]+b->max[2];
    }

    sphere->center[0] = c[0] /= (2*n_boxes); 
    sphere->center[0] = c[1] /= (2*n_boxes); 
    sphere->center[0] = c[2] /= (2*n_boxes);

    /* compute a radius */

    max_r = 0.0f;

    for (i=0, b = boxes; i<n_boxes; i++, b++) {
        dx = b->min[0]-c[0];
        dy = b->min[1]-c[1];
        dz = b->min[2]-c[2];

        r = dx*dx+dy*dy+dz*dz;
        if ( r > max_r ) max_r = r;

        dx = b->max[0]-c[0];
        dy = b->max[1]-c[1];
        dz = b->max[2]-c[2];

        r = dx*dx+dy*dy+dz*dz;
        if ( r > max_r ) max_r = r;
    }
  
    sphere->radius = (float)sqrt(max_r);
}

/*-------------------------------------------------------------------
  Function: atmSphereAroundSpheres
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Compute a bounding sphere that surrounds a set of spheres
  Arguments:
    sphere     - the computed bounding sphere
    n_spheres  - number of spheres
    spheres    - the list of spheres
  Return:
    Nothing
  TBD:
    This routine needs to be optimized
  -------------------------------------------------------------------*/

void atmSphereAroundSpheres(AtmSphere *sphere, int n_spheres, AtmSphere *spheres)
{ 
    AtmSphere *s;
    AtmBox b;
    int i;
    AtmVector3 c;
    float r, max_r, dx, dy, dz;

    if ( n_spheres == 0 ) {
        sphere->radius = 0.0f;
        return;
    }

    /* compute a center */

    atmBoxEmpty(&b);
    atmBoxAroundSpheres(&b, n_spheres, spheres);

    sphere->center[0] = c[0] = 0.5f*(b.min[0]+b.max[0]);
    sphere->center[1] = c[1] = 0.5f*(b.min[1]+b.max[1]);
    sphere->center[2] = c[2] = 0.5f*(b.min[2]+b.max[2]);

    /* compute a radius */

    max_r = 0.0f;

    for (i=0, s = spheres; i<n_spheres; i++, s++) {
        dx = (s->center[0]+s->radius)-c[0];
        dy = (s->center[1]+s->radius)-c[1];
        dz = (s->center[2]+s->radius)-c[2];
  
        r = dx*dx+dy*dy+dz*dz;
        if ( r > max_r ) max_r = r;

        dx = (s->center[0]-s->radius)-c[0];
        dy = (s->center[1]-s->radius)-c[1];
        dz = (s->center[2]-s->radius)-c[2];
  
        r = dx*dx+dy*dy+dz*dz;
        if ( r > max_r ) max_r = r;
    }
  
    sphere->radius = (float)sqrt(max_r);
}

/*-------------------------------------------------------------------
  Function: atmSphereOrthoXform
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Transform the bounding sphere by an orthogonal transform.
    A transform is orthogonal if it consists only of: rotations,
    translations & uniform scales. Such transforms preserve angles
    but not distances.
  Arguments:
    dst     - xformed bounding sphere
    src     - original bounding sphere
    m       - the transform
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atmSphereOrthoXform(AtmSphere *dst, const AtmSphere *src, 
                         const AtmMatrix4x4 m)
{
    float r = ATM_SQRT(m[0]*m[0]+m[1]*m[1]+m[2]*m[2]);

    atmVector3Matrix4x4Mult( dst->center, src->center, m ); 

    dst->radius = r * src->radius;
}
/*-------------------------------------------------------------------
  Function: atmSegFromPoints
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Compute a line segement given its end points
  Arguments:
    dst     - the computed line segment
    p0      - start point
    p1      - end point
  Return:
    FXTRUE if segment defined
    FXFALSE if points are coincident
  -------------------------------------------------------------------*/

FxBool atmSegFromPoints(AtmSeg* dst, float *p0, float *p1) {
    float l, invl;

    ATM_VEC3_COPY(dst->pos, p0);
    ATM_VEC3_SUB(dst->dir, p1, p0); 
    
    if ((l = atmSqrt(ATM_VEC3_DOT(dst->dir, dst->dir))) == 0.0f) 
        return FXFALSE;

    invl = 1.0f/l;

    ATM_VEC3_SCALE(dst->dir, invl, dst->dir);
    
    dst->length = l;

    return FXTRUE; 
}

/*-------------------------------------------------------------------
  Function: atmSegClip
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Clip a segment using specified start and stop paramters
  Arguments:
    dst     - the clipped segment
    src     - the original segment
    tnear   - the start of the clipped segment
    tfar    - the end of the clipped segment
  Return:
    Pick status
  -------------------------------------------------------------------*/

void atmSegClip(AtmSeg* dst, const AtmSeg* src, float tnear, float tfar) {
    if ( tnear > tfar )
        tnear = tfar;
    ATM_VEC3_ADD_SCALED(dst->pos, src->pos, tnear, src->dir);
    if ( dst != src )
        ATM_VEC3_COPY(dst->dir, src->dir);
    dst->length = tfar-tnear;
}

/*-------------------------------------------------------------------
  Function: atmPlaneFromPoints
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Compute plane equation given 3 noncolinear points
  Arguments:
    dst      - the computed plane equation
    p0       - the vertices
    p1 
    p2
  Return:
    FXTRUE if points define a plane (are not colinear)
    FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool atmPlaneFromPoints(AtmPlane* dst, float *p0, float *p1, float *p2) {
    AtmVector3 p01, p02;
    float l, l1, l2, invl;

    ATM_VEC3_SUB(p01, p1, p0); 
    ATM_VEC3_SUB(p02, p2, p0); 

    /* scale these delta's to reduce numerical inaccuracies */

    if ((l1 = atmSqrt(ATM_VEC3_DOT(p01, p01))) == 0.0f) 
        return FXFALSE;

    if ((l2 = atmSqrt(ATM_VEC3_DOT(p02, p02))) == 0.0f) 
        return FXFALSE;

    l = ATM_MAX(l1, l2);
    invl = 1.0f/l;

    ATM_VEC3_SCALE(p01, invl, p01);
    ATM_VEC3_SCALE(p02, invl, p02);

    atmVector3Cross(dst->normal, p01, p02);

    if ((l = atmSqrt(ATM_VEC3_DOT(dst->normal, dst->normal))) == 0.0f) 
        return FXFALSE;

    invl = 1.0f/l;

    ATM_VEC3_SCALE(dst->normal, invl, dst->normal);

    dst->offset = ATM_VEC3_DOT(p0, dst->normal);

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atmDistPointToLine
  Date: 5/28/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Compute the distance from a point to a line.
  Arguments:
    pt       - the point
    seg      - the line
  Return:
    distance from point to line
  Notes:
    distance between point and line is defined as the minimum distance
    between point and all points in the line. This occurs at the point
    r such that (pt-r) is perpendicular to the line. Any point on the
    segment has the form c+ta so t must satify (pt-c-ta).a = 0 or
    t = (pt-c).a (since for a line segment a is a unit vector). This
    gives the distance as being:

       |(p-c) - ((p-c).a)a|
  -------------------------------------------------------------------*/

float atmDistPointToLine(const AtmVector3 pt, const AtmSeg *seg) {
    AtmVector3 pc, tmp;
    float t;

    ATM_VEC3_SUB(pc, pt, seg->pos); 

    t = ATM_VEC3_DOT(pc, seg->dir);
    
    ATM_VEC3_ADD_SCALED(tmp, pc, -t, seg->dir);

    return(atmVector3Magnitude(tmp));
}

/*-------------------------------------------------------------------
  Function: atmDistPointToPoint
  Date: 7/11/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Compute the distance between two points
  Arguments:
    pt1      - first point
    pt2      - second point
  Return:
    distance between points
  -------------------------------------------------------------------*/

float atmDistPointToPoint(const AtmVector3 pt1, const AtmVector3 pt2) {
    AtmVector3 tmp;

    ATM_VEC3_SUB(tmp, pt1, pt2); 

    return(atmVector3Magnitude(tmp));
}

/*-------------------------------------------------------------------
  Function: atmDistPointToPlane
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Compute the distance from a point to a plane.
  Arguments:
    pt       - the point
    plane    - the plane
  Return:
    Signed distance to plane
  -------------------------------------------------------------------*/

float atmDistPointToPlane(const AtmVector3 pt, const AtmPlane *plane) {
    float dist;

    dist = ATM_VEC3_DOT(plane->normal, pt) - plane->offset;

    return(dist);
}

/*-------------------------------------------------------------------
  Function: atmPlaneFromNormPt
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Compute plane equation given normal and point
  Arguments:
    dst      - the computed plane equation
    normal   - plane normal
    p        - point on plane 
  Return:
    nothing
  -------------------------------------------------------------------*/

void atmPlaneFromNormPt(AtmPlane* dst, AtmVector3 normal, AtmVector3 p) {
    ATM_VEC3_COPY(dst->normal, normal);
    dst->offset = ATM_VEC3_DOT( normal, p);
}

/*-------------------------------------------------------------------
  Function: atmPlaneOrthXform
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Compute the image of a plane under an orthogonal transform.    
             
    ph = pXX'h (X' = inverse of X)
    inverse of matrix
         |R 0|  is | R' 0| where T* = -TR`
         |T 1|     | T* 1|
    for orthgonal transformations R' is the transpose of R
  Arguments:
    dst      - the computed plane equation
    pln      - original plane
    m        - the orthogonal transform
  Return:
    nothing
  -------------------------------------------------------------------*/

void 
atmPlaneOrthoXform(AtmPlane* dst, const AtmPlane* pln, const AtmMatrix4x4 m) {
    float r = atmSqrt(m[0]*m[0]+m[1]*m[1]+m[2]*m[2]);
    const float *n = pln->normal;
    float inv_r = 1.0f/r;

    inv_r = 1.0f;

    dst->normal[0] = inv_r * ( n[0] * m[0] + n[1] * m[4] + n[2] * m[8]);

    dst->normal[1] = inv_r * ( n[0] * m[1] + n[1] * m[5] + n[2] * m[9]);

    dst->normal[2] = inv_r * ( n[0] * m[2] + n[1] * m[6] + n[2] * m[10]);

    dst->offset = ( m[12]*m[0]+m[13]*m[1]+m[14]*m[2])*pln->normal[0] +
                  ( m[12]*m[4]+m[13]*m[5]+m[14]*m[6])*pln->normal[1] +
                  ( m[12]*m[8]+m[13]*m[9]+m[14]*m[10])*pln->normal[2] +
                  pln->offset;

    dst->offset *= inv_r;
}

/*-------------------------------------------------------------------
  Function: atmCylAroundSegs
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Put a bounding cylinder around a set of segments. 

    This routine assumes that the segements are pointing in the same
    general direction.

  Arguments:
    cyl     - the cylinder
    segs    - the segments
    nsegs   - number of segments
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atmCylAroundSegs(AtmCylinder* dst, const AtmSeg** segs, int nsegs) {
    int i;
    AtmVector3 seg_end, ref, axis, tmp, tmp2;
    AtmSeg cyl_seg;
    float t, l, invl, dist, min, max, maxRadius;

    ATM_VEC3_SET(axis, 0.0f, 0.0f, 0.0f);

    /* compute cylinder axis */

    for ( i = 0; i < nsegs; i++ ) {
        ATM_VEC3_ADD(axis, axis, segs[i]->dir);
    }

    if ((l = atmVector3Magnitude(axis)) == 0.0f) {
        ATM_VEC3_COPY(axis, segs[0]->dir);
    } else {
        invl = 1.0f/l;

        ATM_VEC3_SCALE(axis, invl, axis);
    }

    /* compute some point on axis */
  
    ATM_VEC3_SET(ref, 0.0f, 0.0f, 0.0f);

    for ( i = 0; i < nsegs; i++ ) {
        ATM_VEC3_ADD(ref, ref, segs[i]->pos);
    }

    ATM_VEC3_SCALE(ref, 1.0f/((float)nsegs), ref);

    /* find center point, halfLength and radius of cylinder */

    min = FLT_MAX; max = - FLT_MAX;
    ATM_VEC3_COPY(cyl_seg.dir, axis);
    ATM_VEC3_COPY(cyl_seg.pos, ref);
    cyl_seg.length = 1.0f; /* this is somewhat arbitrary, just need two points
                              on line */

    maxRadius = 0.0f;

    for ( i = 0; i < nsegs; i++ ) {
        ATM_VEC3_SUB(tmp, segs[i]->pos, ref); 
        t = ATM_VEC3_DOT(tmp, axis);
        min = ATM_MIN(t, min);
        max = ATM_MAX(t, max);
        ATM_VEC3_ADD_SCALED(tmp2, tmp, segs[i]->length, segs[i]->dir);
        t = ATM_VEC3_DOT(tmp2, axis);
        min = ATM_MIN(t, min);
        max = ATM_MAX(t, max);
        ATM_VEC3_ADD_SCALED(seg_end, segs[i]->pos, segs[i]->length, 
                            segs[i]->dir);
        dist = atmDistPointToLine(segs[i]->pos, &cyl_seg);
        maxRadius = ATM_MAX(maxRadius, dist);
        dist = atmDistPointToLine(seg_end, &cyl_seg);
        maxRadius = ATM_MAX(maxRadius, dist);
    }

    dst->halfLength = 0.5f*(min+max);
    dst->radius = maxRadius;
    ATM_VEC3_ADD_SCALED(dst->center, ref, dst->halfLength, axis);
    ATM_VEC3_COPY(dst->axis, axis);
}

/*-------------------------------------------------------------------
  Function: atmCylinderOrthoXform
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Transform the bounding cylinder by an orthogonal transform.
    A transform is orthogonal if it consists only of: rotations,
    translations & uniform scales. Such transforms preserve angles
    but not distances.
  Arguments:
    dst     - xformed bounding cylinder
    cyl     - original bounding cylinder
    m       - the transform
  Return:
    Nothing
  TBD:
    Break out the axis transformation component
    Special case orthonormal transform
  -------------------------------------------------------------------*/

void atmCylinderOrthoXform(AtmCylinder* dst, const AtmCylinder* cyl, 
                      const AtmMatrix4x4 m) {
    float r = atmSqrt(m[0]*m[0]+m[1]*m[1]+m[2]*m[2]);
    float inv_r = 1.0f/r;
    const float *v = cyl->axis;

    /* transform center */

    atmVector3Matrix4x4Mult( dst->center, cyl->center, m );

    /* transform axis */

    dst->axis[0] = inv_r * ( v[0] * m[0] + v[1] * m[1] + v[2] * m[2]);

    dst->axis[1] = inv_r * ( v[0] * m[1] + v[1] * m[5] + v[2] * m[9]);

    dst->axis[2] = inv_r * ( v[0] * m[2] + v[1] * m[6] + v[2] * m[10]);

    dst->radius = r * cyl->radius;
    dst->halfLength = r * cyl->halfLength;
}

/*-------------------------------------------------------------------
  Function: atmCylinderContainsSeg
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a ray intersects a cylinder

    ANSI C code from the article
    "Intersecting a Ray with a Cylinder"
     by Joseph M. Cychosz and Warren N. Waggenspack, Jr.,
     (3ksnn64@ecn.purdue.edu, mewagg@mewnw.dnet.lsu.edu)
     in "Graphics Gems IV", Academic Press, 1994

  Arguments:
    cyl     - the cylinder
    seg     - the segment
    tnear   - starting param of segement in box
    tfar    - end param of segment in box
  Return:
    Pick status
  TBD:
    This routine still does not work correctly, needs review
  -------------------------------------------------------------------*/

FxU32 atmCylinderContainsSeg(const AtmCylinder *cyl, const AtmSeg* seg,
                        float* tnear, float* tfar) {
    AtmVector3     RC;              /* Ray base to cylinder base   */
    float          d;               /* Shortest distance between   */
                                    /*   the ray and the cylinder  */
    float          t, s;            /* Distances along the ray     */
    AtmVector3     n, D, O;
    float          ln;
    float          hit1, hit2;
    AtmPlane       hs;
    FxU32          status;
    AtmSeg         tseg;
    AtmVector3     tmp;
    float          lnear, lfar;

    /* intersect infinite ray against infinite cylinder */

    RC[0] = seg->dir[0] - cyl->center[0];
    RC[1] = seg->dir[1] - cyl->center[1];
    RC[2] = seg->dir[2] - cyl->center[2];

    atmVector3Cross( n, (float *)seg->dir, (float *)cyl->axis);

    if  ( (ln = ATM_VEC3_LENGTH (n)) == 0. ) {   /* ray parallel to cyl */ 
        d = ATM_VEC3_DOT(RC,cyl->axis);
        D[0] = RC[0] - d*cyl->axis[0];
        D[1] = RC[1] - d*cyl->axis[1];
        D[2] = RC[2] - d*cyl->axis[2];
        d = ATM_VEC3_LENGTH(D);
        if ( d > cyl->radius )
            return ATM_IS_FALSE;
        hit1 = -FLT_MAX;
        hit2 =  FLT_MAX;
    } else { /* ray and cylinder not parallel */

        atmVector3Normalize( n, n );
        d  = (float)fabs (ATM_VEC3_DOT(RC,n));  /* shortest distance */

        if ( d > cyl->radius )
            return ATM_IS_FALSE;

        atmVector3Cross(O, RC,  (float *)cyl->axis);
        t = - ATM_VEC3_DOT(O,n) / ln;
        atmVector3Cross(O, n, (float *)cyl->axis);
        atmVector3Normalize(O, O);
        s = (float)fabs (sqrt(cyl->radius*cyl->radius - d*d) / 
               ATM_VEC3_DOT(seg->dir, O));
        hit1 = t - s; /* entering distance        */
        hit2 = t + s;  /* exiting  distance        */
    }

    /* check if finite portion of ray hits infinite cylinder */

    if (( hit1 > seg->length ) || ( hit2 < 0 ))
        return ATM_IS_FALSE; /* no intersection */

    lnear = ATM_MAX(hit1, 0);
    lfar = ATM_MIN(hit2, seg->length);

    /* now intersect segment against half spaces defining top and bottom
     * of cylinder
     */

    /* clip against cylinder top */

    ATM_VEC3_COPY(hs.normal, cyl->axis);
    ATM_VEC3_ADD_SCALED(tmp, cyl->center, cyl->halfLength, cyl->axis); 
    hs.offset = ATM_VEC3_DOT( hs.normal, tmp);

    status = 0xffff;

    status &= atmHalfSpaceContainsSeg(&hs, &tseg, &hit1, &hit2);

    if (status == 0)
        return 0;

    if ( hit1 > lnear ) lnear = hit1;
    if ( hit2 < lfar ) lfar = hit2;

    /* clip against cylinder bottom */

    ATM_VEC3_SCALE(hs.normal, -1.0f, cyl->axis);
    ATM_VEC3_ADD_SCALED(tmp, cyl->center, -cyl->halfLength, cyl->axis); 
    hs.offset = ATM_VEC3_DOT( hs.normal, tmp);

    status &= atmHalfSpaceContainsSeg(&hs, &tseg, &hit1, &hit2);

    if (status == 0)
        return 0;

    if ( hit1 > lnear ) lnear = hit1;
    if ( hit2 < lfar ) lfar = hit2;

    if ( ( hit1 <= 0) && ( 0 <= hit2 ) )
        status |= ATM_IS_START_IN;

    if ( ( hit1 <= seg->length ) && ( seg->length <= hit2 ) )
        status |= ATM_IS_END_IN;

    if ( status == ( ATM_IS_START_IN | ATM_IS_END_IN ))
        status |= ATM_IS_ALL_IN;

    status = ATM_IS_TRUE | ATM_IS_MAYBE;
    *tnear = lnear;
    *tfar = lfar;

    return status;
}

/*-------------------------------------------------------------------
  Function: atmBoxContainsPoint
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a box contains a point
  Arguments:
    box      - the box
    pt       - the point
  Return:
    FXTRUE if point is inside box
    FXFALSE if point is outside box
  -------------------------------------------------------------------*/

FxBool atmBoxContainsPoint(const AtmBox *box, const AtmVector3 pt) {
    return ATM_PT_IN_BOX(pt, box);
}

/*-------------------------------------------------------------------
  Function: atmSphereContainsPoint
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a sphere contains a point
  Arguments:
    sphere   - the sphere
    pt       - the point
  Return:
    FXTRUE if point is inside sphere
    FXFALSE if point is outside sphere
  -------------------------------------------------------------------*/

FxBool atmSphereContainsPoint(const AtmSphere *sphere, const AtmVector3 pt) {
    return ATM_PT_IN_SPHERE(pt, sphere) ;
}

/*-------------------------------------------------------------------
  Function: atmCylinderContainsPoint
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a cylinder contains a point. This is true if:

        |(p-c).a| < h and |(1-(p-c).a)(p-c)| < r

  Arguments:
    cylinder - the cylinder
    pt       - the point
  Return:
    FXTRUE if point is inside cylinder
    FXFALSE if point is outside cylinder
  -------------------------------------------------------------------*/

FxBool atmCylinderContainsPoint(const AtmCylinder *cyl, const AtmVector3 pt) {
    AtmVector3 pc;
    float pca;

    ATM_VEC3_SUB(pc, pt, cyl->center); 

    pca = ATM_VEC3_DOT(pc, cyl->axis);

    return (( ATM_ABS(pca) <= cyl->halfLength ) &&
            ((ATM_ABS(1-pca)*ATM_VEC3_DOT(pc, pc)) <= cyl->radius ));
}

/*-------------------------------------------------------------------
  Function: atmHalfSpaceContainsPoint
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a half space contains a point

  Arguments:
    plane    - the plane
    pt       - the point
  Return:
    FXTRUE if point is inside half space
    FXFALSE if point is outside half space
  -------------------------------------------------------------------*/

FxBool atmHalfSpaceContainsPoint(const AtmPlane *plane, const AtmVector3 pt) {
    return ATM_PT_IN_HALF_SPACE(pt, plane);
}

/*-------------------------------------------------------------------
  Function: atmSegIsectPlane
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a ray intersects a plane
  Arguments:
    seg      - the segment
    p        - the plane
    t        - the segment parameter of the intersection
  Return:
    Pick status
  -------------------------------------------------------------------*/

FxU32 atmSegIsectPlane(const AtmSeg* seg, const AtmPlane* p, float* t) {
    float v;

    v = ATM_VEC3_DOT(p->normal, seg->dir);

    if ( v == 0.0f )
        return ATM_IS_FALSE; /* segment does not intersect plane */

    *t = ( p->offset - ATM_VEC3_DOT(p->normal, seg->pos) )/v;

    if (( *t < 0 ) || ( *t > seg->length ))
        return ATM_IS_FALSE; /* intersection not within segment */

    return (ATM_IS_TRUE | ATM_IS_MAYBE);
}

/*-------------------------------------------------------------------
  Function: atmSegIsectTri
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a ray intersects a triangle
  Arguments:
    seg      - the segment
    v0       - the vertices
    v1 
    v2
    t        - the segment parameter of the intersection
  Return:
    Pick status
  -------------------------------------------------------------------*/

FxU32 atmSegIsectTri(AtmSeg* seg, float *p0, float *p1, float *p2, float *t) {
    AtmPlane plane; /* plane equation */
    AtmVector3 p;   /* point of intersection of ray and polygon */
    float ax, ay, az, max;
    float alpha, beta;
    int i0, i1, i2;
    float u0, v0, u1, v1, u2, v2;
    FxU32 status;
    
    atmPlaneFromPoints(&plane, p0, p1, p2);

    if ( ( atmSegIsectPlane(seg, &plane, t) & ATM_IS_TRUE ) == 0 ) 
        return ATM_IS_FALSE;

    /* compute point of intersection */

    ATM_VEC3_ADD_SCALED(p, seg->pos, *t, seg->dir); 

   /* To determine if the intersection point is within the triangle we represent 
      the point in the form: Pp0 = alpha*p10+beta*p20. This will be inside the 
      triangle if: alpha, beta >= 0 and alpha+beta <= 1. 

      To simplify this test we project the triangle onto the coordinate 
      plane perpendicular to the most dominant portion of the triangles 
      normal. This simplifies our problem and gives us the largest area 
      triangle to work with.
    */

    ax = ATM_ABS(plane.normal[0]); 
    ay = ATM_ABS(plane.normal[1]);
    az = ATM_ABS(plane.normal[2]);

    i0 = 0; max = ax;
    if ( ay > max ) { i0 = 1; max = ay; }
    if ( az > max ) i0 = 2; 

    i1 = (i0+1)%3; i2 = (i1+1)%3;

    /* now determine alpha and beta */

    u0 = p[i1]-p0[i1]; v0 = p[i2]-p0[i1];
    u1 = p1[i1]-p0[i1]; v1 = p1[i2]-p0[i1];
    u2 = p2[i1]-p0[i1]; v2 = p2[i2]-p0[i1];

    status = ATM_IS_FALSE;

    if ( u1 == 0.0f ) {
        beta = u0/u2;
        if ( ( beta >= 0.0f ) && ( beta <= 1.0f )) {
            alpha = (v0 - beta*v2)/v1;
            if ((alpha >= 0.) && ( (alpha+beta) <= 1.0f ))
               status = ( ATM_IS_TRUE | ATM_IS_MAYBE );
        }
    } else {
        beta = (v0*u1-u0*v1)/(v2*u1-u2*v1);
        if ( ( beta >= 0.0f ) && ( beta <= 1.0f )) {
            alpha = (u0 - beta*u2)/u1;
            if ((alpha >= 0.) && ( (alpha+beta) <= 1.0f ))
               status = ( ATM_IS_TRUE | ATM_IS_MAYBE );
        }
    } 

    /* TBD: in the future we could return more info: such as the interpolated 
       normal, point of intersection etc. For now we just give status.
     */

    return status;
}

/*-------------------------------------------------------------------
  Function: atmHalfSpaceContainsSeg
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether (and where) a half space contains a segment
  Arguments:
    hs      - the half space
    seg     - the segment
    tnear   - starting param of segement in box
    tfar    - end param of segment in box
  Return:
    Pick status
  -------------------------------------------------------------------*/

FxU32 atmHalfSpaceContainsSeg(const AtmPlane* hs, const AtmSeg *seg, float* d1, float* d2) {
    AtmVector3 seg_end;
    FxU32 status = ATM_IS_FALSE;
    float t, d;

    ATM_VEC3_ADD_SCALED(seg_end, seg->pos, seg->length, seg->dir);

    if ( ATM_PT_IN_HALF_SPACE(seg->pos, hs) ) status |= ATM_IS_START_IN;

    if ( ATM_PT_IN_HALF_SPACE(seg_end, hs) ) status |= ATM_IS_END_IN;

    if ( status == ATM_IS_FALSE )
        return status;

    if ( status == (ATM_IS_START_IN|ATM_IS_END_IN) ) {
        *d1 = 0.0f;
        *d2 = seg->length;
        status |= ATM_IS_TRUE | ATM_IS_MAYBE | ATM_IS_ALL_IN ;
        return status;
    } else {
         d = ATM_VEC3_DOT(seg->dir, hs->normal);
         if ( d == 0.0f ) { /* should never happen */
             atuError( FXTRUE, "atmHalfSpaceContainsSeg(): divide by zero.\n" );
             t = 0.0f;
         } else {
             t = (ATM_VEC3_DOT(seg->pos, hs->normal)+hs->offset)/d;
         }

         if ( status & ATM_IS_START_IN ) {
             *d1 = 0.0f;
             *d2 = t;
         } else {
             *d1 = t;
             *d2 = seg->length;
         }
         
         status |= ATM_IS_TRUE | ATM_IS_MAYBE ;
         return status;
    } 
}


/*-------------------------------------------------------------------
  Function: atmBoxContainsSeg
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a box contains a segment
  Arguments:
    box     - the box
    seg     - the segment
    tnear   - starting param of segement in box
    tfar    - end param of segment in box
  Return:
    Pick status
  -------------------------------------------------------------------*/

FxU32 atmBoxContainsSeg(const AtmBox *box, const AtmSeg* seg, float* tnear, float* tfar) {
    AtmVector3  seg_end; 
    register int i;
    float gnear, gfar, lnear, lfar ;
    FxU32 status = 0;

    ATM_VEC3_ADD_SCALED(seg_end, seg->pos, seg->length, seg->dir);

    lnear = gnear = 0.0f;
    lfar = gfar = seg->length;

    for (i=0; i<3; i++) {

        /* clip line segment to lower bound */

        if(seg->pos[i] < box->min[i]) { /* origin out on lower bound */
           if ( seg_end[i] < box->min[i] )
               return ATM_IS_FALSE;
           else lnear = (box->min[i]-seg->pos[i])/seg->dir[i];
        } else if ( seg_end[i] < box->min[i] )
                   lfar = (box->min[i]-seg->pos[i])/seg->dir[i];

        /* clip line segment to upper bound */
           
        if(seg->pos[i] > box->max[i]) { /* origin out on upper bound */
           if ( seg_end[i] > box->max[i] )
               return ATM_IS_FALSE;
           else lnear = (box->max[i]-seg->pos[i])/seg->dir[i];
        } else if ( seg_end[i] > box->max[i] ) {
               lfar = (box->max[i]-seg->pos[i])/seg->dir[i];
        }

        if ( gnear < lnear ) gnear = lnear;
        if ( gfar > lfar ) gfar = lfar;
    }

    status = 0;
    *tnear = gnear;
    *tfar = gfar;

    if ( gnear == 0.0f )
        status |= ATM_IS_START_IN;

    if ( gfar == seg->length )
        status |= ATM_IS_END_IN;

    if ( status == (ATM_IS_START_IN | ATM_IS_END_IN) )
        status |= ATM_IS_ALL_IN;

    status |= ATM_IS_TRUE | ATM_IS_MAYBE;

    return status;
}
           
/*-------------------------------------------------------------------
  Function: atmSphereContainsSeg
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a sphere contains a segment
  Arguments:
    sphere  - the sphere
    seg     - the segment
    tnear   - starting param of segement in box
    tfar    - end param of segment in box
  Return:
    Pick status
  -------------------------------------------------------------------*/

FxU32 atmSphereContainsSeg(const AtmSphere *sphere, const AtmSeg* seg,
                        float* tnear, float* tfar) {
    AtmVector3 ray_ac; 
    float a, b, c, d;
    float r = sphere->radius;
    float hit1, hit2;
    FxU32 status;

    ATM_VEC3_SUB(ray_ac, seg->pos, sphere->center); 

    /* for an intersection of the ray and sphere: (A + b*t)^2 == r^2 */
    /* coefficients of quadratic equation */

    a = ATM_VEC3_DOT(seg->dir, seg->dir);
    b = ATM_VEC3_DOT(ray_ac, seg->dir);
    c = ATM_VEC3_DOT(ray_ac, ray_ac)- r*r;
    d = b * b - a * c;  /* discriminant */

    if (d < 0)  /* no hit */
        return ATM_IS_FALSE;

    d = (float)sqrt (d) / a;
    a = - b / a;

    /* 2 possible hits at a +- d */

    hit1 = a - d;
    hit2 = a + d;

    assert(hit1 <= hit2);

    if (( hit1 > seg->length ) || ( hit2 < 0 ))
        return ATM_IS_FALSE; /* no intersection */

    status = 0;

    if ( ( hit1 <= 0) && ( 0 <= hit2 ) )
        status |= ATM_IS_START_IN;

    if ( ( hit1 <= seg->length ) && ( seg->length <= hit2 ) )
        status |= ATM_IS_END_IN;

    if ( status == ( ATM_IS_START_IN | ATM_IS_END_IN ))
        status |= ATM_IS_ALL_IN;

    status = ATM_IS_TRUE | ATM_IS_MAYBE;

    *tnear = ATM_MAX(hit1, 0);
    *tfar = ATM_MIN(hit2, seg->length);

    return status;
} 

/*-------------------------------------------------------------------
  Function: atmHalfSpaceContainsSphere
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a half space contains a sphere
  Arguments:
    plane   - the half space
    sphere  - the sphere
  Return:
    Pick status
  -------------------------------------------------------------------*/

FxU32 atmHalfSpaceContainsSphere(const AtmPlane *plane, const AtmSphere *sphere) {
    float d = atmDistPointToPlane(sphere->center, plane);

    d = d*d;

    if ( ATM_PT_IN_HALF_SPACE(sphere->center, plane)) {
        if ( ATM_SQ(sphere->radius) < d )
           return ( ATM_IS_MAYBE | ATM_IS_TRUE | ATM_IS_ALL_IN );
        else return ( ATM_IS_MAYBE | ATM_IS_TRUE );
    } else {
        if ( d < ATM_SQ(sphere->radius) )
            return ( ATM_IS_MAYBE | ATM_IS_TRUE );
        else return ATM_IS_FALSE;
    }
}

/*-------------------------------------------------------------------
  Function: atmHalfSpaceContainsCylinder
  Date: 7/5/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a half space contains a cylinder. This is a loose
    test it simply tests the circumsribing sphere against the half space.
  Arguments:
    plane   - the half space
    cyl     - the cylinder
  Return:
    Pick status
  -------------------------------------------------------------------*/

FxU32 atmHalfSpaceContainsCylinder(const AtmPlane *plane, const AtmCylinder *cyl) {
    float d = atmDistPointToPlane(cyl->center, plane);
    float r = ATM_MAX(cyl->radius, cyl->halfLength);

    d = d*d;

    if ( ATM_PT_IN_HALF_SPACE(cyl->center, plane)) {
        if ( ATM_SQ(r) < d )
           return ( ATM_IS_MAYBE | ATM_IS_TRUE | ATM_IS_ALL_IN );
        else return ( ATM_IS_MAYBE | ATM_IS_TRUE );
    } else {
        if ( d < ATM_SQ(r) )
            return ( ATM_IS_MAYBE | ATM_IS_TRUE );
        else return ATM_IS_FALSE;
    }
}

/*-------------------------------------------------------------------
  Function: atmSphereContainsSphere
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a sphere contains a sphere
  Arguments:
    s1      - a sphere
    s2      - another sphere
  Return:
    Pick status
  TBD:
    For performance reasons we don't currently check for complete containment
    how important is this.
  -------------------------------------------------------------------*/

FxU32 atmSphereContainsSphere(const AtmSphere *s1, const AtmSphere* s2) {
    float d = ATM_VEC3_SQR_DISTANCE(s1->center, s2->center);

    if ( d <= ATM_SQ(s1->radius + s2->radius ) )
        return ( ATM_IS_MAYBE | ATM_IS_TRUE );
    else return ATM_IS_FALSE;
} 

/*-------------------------------------------------------------------
  Function: atmSphereContainsBox
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a sphere contains a box
  Arguments:
    sphere  - the sphere
    box     - the box
  Return:
    Pick status
  -------------------------------------------------------------------*/

FxU32 atmSphereContainsBox( const AtmSphere *sphere, const AtmBox *box) {
    float  dmin;
    float  r2 = ATM_SQ( sphere->radius );
    int    i;

    dmin = 0.0f;

    /* find the point within the square which is closet the center of the
     * sphere
     */

    for( i = 0; i < 3; i++ ) {
        if( sphere->center[i] < box->min[i] ) 
            dmin += ATM_SQ(sphere->center[i] - box->min[i] ); 
        else if( sphere->center[i] > box->max[i] ) 
                dmin += ATM_SQ( sphere->center[i] - box->max[i] );     
    }

    if ( dmin <= r2 ) 
         return (ATM_IS_MAYBE | ATM_IS_TRUE);
    else return (ATM_IS_FALSE);
}

/*-------------------------------------------------------------------
  Function: atmHalfSpaceContainsBox
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a half space contains a box
  Arguments:
    plane    - a half space
    box      - a box
  Return:
    Pick status
  -------------------------------------------------------------------*/

FxU32 atmHalfSpaceContainsBox(const AtmPlane* plane, const AtmBox* box) {
    FxU32 all_in = FXTRUE, some_in = FXFALSE;
    AtmVector3 p;
    int i;
    FxU32 t;

    for ( i = 0;  i < 8;  i++) {
        p[0] = (i & 4) ? box->min[0] : box->max[0]; 
        p[1] = (i & 2) ? box->min[1] : box->max[1]; 
        p[2] = (i & 1) ? box->min[2] : box->max[2]; 

        t = ATM_PT_IN_HALF_SPACE(p, plane);

        all_in &= t; some_in |= t;
    }

    if ( all_in ) {
        return ( ATM_IS_MAYBE | ATM_IS_TRUE | ATM_IS_ALL_IN );
    } else {
        if ( some_in )
            return ( ATM_IS_MAYBE | ATM_IS_TRUE );
        else return ( ATM_IS_FALSE );
    }
}

/*-------------------------------------------------------------------
  Function: atmBoxContainsBox
  Date: 5/17/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a box contains another box
  Arguments:
    b1    - a box
    b2    - a box
  Return:
    Pick status
  TBD:
    For performance reasons we don't currently check for complete containment
    how important is this.
  -------------------------------------------------------------------*/

FxU32 atmBoxContainsBox(const AtmBox* b1, const AtmBox* b2) {
    int i ;

    for ( i = 0; i < 3; i++ ) {
        if (( b1->max[i] < b2->min[i] ) || ( b1->min[i] > b2->max[i] ))  
            return ATM_IS_FALSE;
    }

    return ( ATM_IS_MAYBE | ATM_IS_TRUE );
}

/*-------------------------------------------------------------------
  Function: atmPolytopeOrthXform
  Date: 7/5/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Compute the image of a polytope under an orthogonal transform.    

  Arguments:
    dst      - the computed plane equation
    pln      - original plane
    m        - the orthogonal transform
  Return:
    nothing
  -------------------------------------------------------------------*/

void atmPolytopeOrthoXform(AtmPolytope* dst, const AtmPolytope *src, 
                        const AtmMatrix4x4 m) {
    FxU32 i;

    if ( dst->numPlanes != src->numPlanes ) {
        atuError(FXTRUE, "atmPolytopeOrthoXform: polytopes are incompatible\n");
    }

    for ( i = 0; i < dst->numPlanes; i++ ) {
        atmPlaneOrthoXform(dst->planes+i, src->planes+i, m);
    }
}

/*-------------------------------------------------------------------
  Function: atmPolytopeSpaceContainsPoint
  Date: 7/5/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a polytope contains a point

  Arguments:
    ptp      - the polytope
    pt       - the point
  Return:
    FXTRUE if point is inside polytope
    FXFALSE if point is outside polytope
  -------------------------------------------------------------------*/

FxBool atmPolytopeContainsPoint(const AtmPolytope* ptp, const AtmVector3 pt) {
    FxU32 i;

    for ( i = 0; i < ptp->numPlanes; i++ ) {
        if (! atmHalfSpaceContainsPoint(ptp->planes+i, pt) )
            return FXFALSE;
    }

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atmPolytopeContainsSphere
  Date: 7/5/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a polytope contains a sphere
  Arguments:
    ptp     - the polytope
    sphere  - the sphere
  Return:
    Pick status
  -------------------------------------------------------------------*/

FxU32 atmPolytopeContainsSphere(const AtmPolytope* ptp, const AtmSphere *sphere) {
    FxU32 i;
    FxU32 status = ~0UL;

    for ( i = 0; i < ptp->numPlanes; i++ ) {
        status &= atmHalfSpaceContainsSphere(ptp->planes+i, sphere);
        if ( status == 0 )
            break;
    }

    return status;
}

/*-------------------------------------------------------------------
  Function: atmPolytopeContainsBox
  Date: 7/5/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a polytope contains a box
  Arguments:
    ptp     - the polytope
    box     - the box
  Return:
    Pick status
  -------------------------------------------------------------------*/

FxU32 atmPolytopeContainsBox(const AtmPolytope* ptp, const AtmBox *box) {
    FxU32 i;
    FxU32 status = ~0UL;

    for ( i = 0; i < ptp->numPlanes; i++ ) {
        status &= atmHalfSpaceContainsBox(ptp->planes+i, box);
        if ( status == 0 )
            break;
    }

    return status;
}

/*-------------------------------------------------------------------
  Function: atmPolytopeContainsCylinder
  Date: 7/5/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a polytope contains a cylinder
  Arguments:
    ptp     - the polytope
    cyl     - the cylinder
  Return:
    Pick status
  -------------------------------------------------------------------*/

FxU32 atmPolytopeContainsCylinder(const AtmPolytope* ptp, const AtmCylinder *cyl) {
    FxU32 i;
    FxU32 status = ~0UL;

    for ( i = 0; i < ptp->numPlanes; i++ ) {
        status &= atmHalfSpaceContainsCylinder(ptp->planes+i, cyl);
        if ( status == 0 )
            break;
    }

    return status;
}

/*-------------------------------------------------------------------
  Function: atmHalfSpaceContainsPolytope
  Date: 7/5/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a half space contains a polytope
  Arguments:
    hs       - the half space (plane)
    ptp      - the polytope
  Return:
    Pick status
  -------------------------------------------------------------------*/

FxU32 atmHalfSpaceContainsPolytope(const AtmPlane* hs, const AtmPolytope *ptp) {
    FxU32 i;
    FxU32 status = ~0UL;
    float v;

    /* reduce problem to comparing each of polytopes planes against the specified 
       plane 
     */

    for ( i = 0; i < ptp->numPlanes; i++ ) {

        /* check if planes are parellel */
        
        v = ATM_VEC3_DOT(ptp->planes[i].normal, hs->normal);
        if ( ATM_ALMOST_EQUAL(fabs(v), 1.0f) ) { /* planes are parallel */
            if ( v > 0.0f ) { 
                /* both planes normals are pointing in same direction, they must
                   have none zero intersection, check if polytope is completely
                   contained (using the half space defining equation

                        p.n <= offset */

                if ( ptp->planes[i].offset <= hs->offset )
                     status &= (ATM_IS_ALL_IN | ATM_IS_TRUE | ATM_IS_MAYBE);
                else status &= (ATM_IS_TRUE | ATM_IS_MAYBE);
            } else {
                /*  planes normals are pointing in opposite directions their 
                    intersection is either partial or empty, again use the
                    defining equations to determine result */

                if ( -ptp->planes[i].offset <= hs->offset )
                    status &= (ATM_IS_TRUE | ATM_IS_MAYBE);
                else return ATM_IS_FALSE;
            }
        } else { /* if planes are not parallel they partially intersect */
            status &= (ATM_IS_TRUE | ATM_IS_MAYBE );
        }
    }

    return status;
}

/*-------------------------------------------------------------------
  Function: atmPolytopeContainsPolytope
  Date: 7/5/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a polytope contains a polytope
  Arguments:
    ptp1     - the first polytope
    ptp2     - the second polytope
  Return:
    Pick status
  -------------------------------------------------------------------*/

FxU32 atmPolytopeContainsPolytope(const AtmPolytope* ptp1, const AtmPolytope *ptp2) {
    FxU32 i;
    FxU32 status = ~0UL;

    for ( i = 0; i < ptp1->numPlanes; i++ ) {
        status &= atmHalfSpaceContainsPolytope(ptp1->planes+i, ptp2);
        if ( status == 0 )
            break;
    }

    return status;
}

/*-------------------------------------------------------------------
  Function: atmPolytopeContainsSeg
  Date: 7/5/96
  Implementor(s): mlwp
  Library: ATB Math Library
  Description: 
    Determines whether a polytope contains a segment and compute the 
    intersection of the segment with the polytope
  Arguments:
    ptp     - the polytope
    seg     - the segment
    d1      - starting param of segement in polytope
    d2      - end param of segment in polytope
  Return:
    Pick status
  -------------------------------------------------------------------*/

FxU32 atmPolytopeContainsSeg(const AtmPolytope *ptp, const AtmSeg* seg,
                           float* d1, float* d2) {
    FxU32 i;
    FxU32 status = ~0UL;
    float t1, t2, t1max = 0.0f, t2min = seg->length;

    for ( i = 0; i < ptp->numPlanes; i++ ) {
        status &= atmHalfSpaceContainsSeg(ptp->planes+i, seg, &t1, &t2);
        if ( status == 0 )
            break;
        t1max = ATM_MAX(t1max, t1);
        t2min = ATM_MIN(t2min, t2);
        if ( t1max >= t2min )
            return ATM_IS_FALSE;
    }

    *d1 = t1max;
    *d2 = t2min;
    return status;
}
