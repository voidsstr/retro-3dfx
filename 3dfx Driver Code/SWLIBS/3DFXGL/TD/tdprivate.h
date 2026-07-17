#ifndef TDPRIVATE_H
#define TDPRIVATE_H
/*
 *  tdPrivate.h
 *                                                     09/01/95  4.00  NDR
 *
 *    Three-D Object Library - Library of 3-D object
 *    manipulation routines.
 *
 *    Private Header File
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
#include "td.h"


/* defines */
#define TD_NAME "TD-Library"

/* math defines */
#define TD_BIG     9e+9
#define TD_PI      3.14159265358979323846264338327950288
#define TD_PI_HALF 1.57079632679489661923132169163975144

/* macro defines */
#define _tdMax(a, b) ((a) > (b) ? (a) : (b))
#define _tdSqr(a) ((a) * (a))
#define _tdAbs(a) (((a) < 0) ? (-(a)) : (a))


/* functions */

/*
 *  _tdReadRGBTeximage()
 *
 *    Reads a texture file in the Iris RGB (.rgb) file format.
 *
 *    filename - name of the file containing the texture data of the
 *    form path + name + ".rgb" extension. 
 *
 */
TDteximage *_tdReadRGBTeximage(char *filename)
;

/*
 *  _tdPrintf()
 *
 *    Prints out information in a standard way e.g., 
 *    TD-Library: INFO - this is an information message.
 *    Prints out error messages on the stderr, warning and
 *    info messages on stdout.
 *
 *    Uses the global variable TD_Verbose to mask out certain
 *    message printing.
 *
 *    type - type of message to be printed, one of the following:
 *    TD_INFO     - informative message
 *    TD_WARNING  - warning message
 *    TD_ERROR    - error message
 *    TD_NOMEMORY - out of memory message
 *    TD_FATAL    - fatal error message
 */
void _tdPrintf(
  TDmessage type,     /* type of message */
  char *format, ...   /* format e.g., "%s %s\n", "hello", "world" */
	       )
;

/*
 *  _tdNewGroup()
 * 
 *    Allocates memory for a group data structure, and fills the new
 *    structure with default values.
 *
 *    name - name of the new group, should not be NULL.
 *
 *    RETURNs a new group pointer if all went well,
 *    otherwise an error message is printed, and NULL returned.
 */
TDgroup *_tdNewGroup(char *name)
;

/*
 *  _tdNewFace()
 * 
 *    Allocates memory for a face data structure, and 
 *    fills the fields with default values.
 *
 *    num_vertices    - number of vertices to allocate memory for
 *    num_texvertices - number of texvertices to allocate memory for
 *    num_normals     - number of normals to allocate memory for 
 *
 *    RETURNs a new face pointer if all went well,
 *    otherwise an error message is printed, and NULL returned.
 */
TDface *_tdNewFace(int num_vertices, int num_texvertices, int num_normals)
;

/*
 *  _tdNewMaterial()
 * 
 *    Allocates memory for a material data structure, and 
 *    fills the fields with default values.
 *
 *    name - name of the new group, should not be NULL.
 *
 *    RETURNs a new material pointer if all went well,
 *    otherwise an error message is printed, and NULL returned.
 */
TDmaterial *_tdNewMaterial(char *name)
;

/*
 *  _tdDeleteMaterials()
 * 
 *    Frees memory allocated for a material data structures
 *
 *    material - pointer to a list of materials to be freed.
 *
 *    NOTE that this routine goes through the WHOLE LIST of
 *    materials (until it hits a NULL) and frees ALL of them
 *     
 */
void _tdDeleteMaterials(TDmaterial *material)
;

/*
 *  _tdDeleteFaces()
 * 
 *    Frees memory allocated for a face data structures
 *
 *    face - pointer to a list of faces to be freed.
 *
 *    NOTE that this routine goes through the WHOLE LIST of
 *    faces (until it hits a NULL) and frees ALL of them
 */
void _tdDeleteFaces(TDface *face)
;

/*
 *  _tdDeleteGroups()
 * 
 *    Frees memory allocated for a group data structures
 *
 *    group - pointer to a list of groups to be freed.
 *
 *    NOTE that this routine goes through the WHOLE LIST of
 *    groups (until it hits a NULL) and frees ALL of them
 *    including any memory associated with them (such as faces)
 */
void _tdDeleteGroups(TDgroup *group)
;

/*
 *  _tdBoundingBox()
 *
 *    Computes the bounding box around an object, and
 *    the dimensions (volumetric) of the object.
 *
 *    object - a pointer to a valid object
 *
 */
void _tdBoundingBox(TDobject *object)
;

#endif /* TDPRIVATE_H */
