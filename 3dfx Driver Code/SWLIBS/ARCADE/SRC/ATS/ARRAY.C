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
** $Date: 10/11/00 7:34:27 PM$ 
**
*/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <atscenep.h>

/*-------------------------------------------------------------------
  Function: atsArrayMaxSize
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Sets the maximum number of elements in the array. If the data 
    area is user managed it does nothing. If the data area is managed by
    ATB it will be enlarged to the specified size.  If the number of 
    elements specified is less than the current max array size then the 
    array is unchanged.
  Arguments:
    a         -   pointer to array
    nelem     -   new number of elements in the a:
  Return:
    FXTRUE  if suceeded 
    FXFALSE if failed 
  -------------------------------------------------------------------*/

int atsArrayMaxSize(AtsArray *a, FxU16 nelem) {
    int elem_size = (a->elem_size == 0) ? sizeof(AtsNode *) : a->elem_size;
    if ( a->max_size >= nelem )
        return FXTRUE;

    nelem += 10; /* ensure we have some buffer space */

    if ( a->attr & ATS_ARRAY_USER_DEFINED )
        return FXFALSE;

    a->data = atuMemRealloc(a->data, nelem*elem_size);

    if ( a->data == NULL ) {
#ifdef AT_DEBUGGING
        atuError( FXTRUE, "atsArrayMaxSize(): Not enough memory.\n" );
#endif
        return FXFALSE;
    }

    a->max_size = nelem;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsArrayGetMaxSize
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Return the current maximum number of elements in an array
  Arguments:
    a         -   pointer to array 
  Return:
    number of elements in array
  -------------------------------------------------------------------*/

FxU16 atsArrayGetMaxSize(AtsArray *a) {
    return a->max_size;
}

/*-------------------------------------------------------------------
  Function: atsArrayGetSize
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Return the current number of elements in an array
  Arguments:
    a         -   pointer to array 
  Return:
    number of elements in array
  -------------------------------------------------------------------*/

FxU16 atsArrayGetSize(AtsArray *a) {
    return a->cur_size;
}

/*-------------------------------------------------------------------
  Function: atsArrayInit
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Initialize an array which is on the stack or part of a structure.
    If elem_size == 0, then this is an array of object pointers and 
    are reference counted.
  Arguments:
    a         -   pointer to array data structure
    nelem     -   initial number of elements in array (can be zero)
    elem_size -   size (in bytes) of array elements, if zero these are
                  object pointers
  Return:
    FXTRUE  if suceeded 
    FXFALSE if failed 
  -------------------------------------------------------------------*/

int atsArrayInit(AtsArray *a, FxU16 nelem, FxU16 elem_size) {
    a->attr = 0;
    a->elem_size = elem_size;
    a->max_size = nelem;
    a->data = NULL;

    if ( nelem > 0 )
        return atsArrayMaxSize(a, nelem);
    else return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsArrayNew
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Create a new array data structure
  Arguments:
    nelem     -   initial number of elements in array (can be zero)
    elem_size -   size (in bytes) of array elements, if zero these are
                  object pointers
  Return:
    pointer to new array object
  -------------------------------------------------------------------*/

AtsArray *atsArrayCreate(FxU16 nelem, FxU16 elem_size) {
    AtsArray *a;

    if (!(a = atuMemCalloc(1, sizeof(AtsArray)))) {
#ifdef AT_DEBUGGING
        atuError( FXTRUE, "atsArrayCreate(): Not enough memory.\n" );
#endif
        return NULL;
    }

    if ( atsArrayInit(a, nelem, elem_size) != FXTRUE ) {
#ifdef AT_DEBUGGING
        atuError( FXTRUE, "atsArrayCreate(): Not enough memory.\n" );
#endif
        atuMemFree(a);
        return NULL;
    }

    return a;
}

/*-------------------------------------------------------------------
  Function: atsArrayData
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set the arrays data pointer
  Arguments:
    a         - the array
    ptr       - pointer to user specified data area
  Return:
    nothing
  -------------------------------------------------------------------*/

void atsArrayData(AtsArray *a, char *ptr) {
    int i;

/* if its an array of objects, dereference the objects */

    if ( a->elem_size == 0 ) {
        AtsNode **n = (AtsNode **)(a->data);

        for (i = 0; i < a->cur_size; i++) 
           atsUnrefDelete(n++);
    }

/* if the memory is managed by ATB free it */

    if ( a->attr & ATS_ARRAY_USER_DEFINED == 0 )
        atuMemFree(a->data);

    a->data = ptr;

    a->attr |= ATS_ARRAY_USER_DEFINED;
}

/*-------------------------------------------------------------------
  Function: atsArrayGetData
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Get the arrays data pointer
  Arguments:
    a         - the array
  Return:
    pointer to arrays data area
  -------------------------------------------------------------------*/

char *atsArrayGetData(AtsArray *a) {
    return a->data;
}

/*-------------------------------------------------------------------
  Function: atsArrayCopy
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Copy an array
  Arguments:
    dst        - the destination array
    src        - the source array
  Return:
    FXTRUE if succesful
    FXFALSE if failed
  -------------------------------------------------------------------*/

int atsArrayCopy(AtsArray *dst, AtsArray *src) {
    int i;
    int elem_size = (src->elem_size == 0) ? sizeof(AtsNode *) : src->elem_size;

/* week type checking: check elements are same size */

   if ( src->elem_size != dst->elem_size ) {
#ifdef AT_DEBUGGING
       atuError( FXTRUE, "atsArrayCopy(): incopmatible arrays\n" );
#endif
       return FXFALSE;
   }

/* if its an array of objects, dereference the objects */

    if ( dst->elem_size == 0 ) {
        AtsNode **n = (AtsNode **)(dst->data);

        for (i = 0; i < dst->cur_size; i++) 
           atsUnrefDelete(n++);
    }

    if ( atsArrayMaxSize(dst, src->cur_size ) != FXTRUE ) {
#ifdef AT_DEBUGGING
        atuError( FXTRUE, "atsArrayCopy(): Not enough memory.\n" );
#endif
        return FXFALSE;
     }

     memcpy(dst->data, src->data, src->cur_size*elem_size);

/* if its an array of objects, increment the ref count */

     if ( dst->elem_size == 0 ) {
         AtsNode **n = (AtsNode **)(dst->data);

         for (i = 0; i < src->cur_size; i++) 
            atsRef(n++);
     }

    dst->cur_size = src->cur_size;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsArrayConcat
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Concatenate an arrays elements on another array leaving the original
    elements untouched
  Arguments:
    dst        - the destination array
    src        - the source array
  Return:
    FXTRUE if succesful
    FXFALSE if failed
  -------------------------------------------------------------------*/

int atsArrayConcat(AtsArray *dst, AtsArray *src) {
    int elem_size = (src->elem_size == 0) ? sizeof(AtsNode *) : src->elem_size;
    int i;

/* week type checking: check elements are same size */

    if ( src->elem_size != dst->elem_size ) {
#ifdef AT_DEBUGGING
        atuError( FXTRUE, "atsArrayConcat(): incopmatible arrays\n" );
#endif
        return FXFALSE;
    }

    if ( atsArrayMaxSize(dst, (FxU16)(dst->cur_size + src->cur_size) ) != FXTRUE ) {
#ifdef AT_DEBUGGING
        atuError( FXTRUE, "atsArrayConcat(): Not enough memory.\n" );
#endif
        return FXFALSE;
     }
  
     memcpy(dst->data+elem_size*dst->cur_size, src->data, 
            src->cur_size*elem_size);

/* if its an array of objects, increment the ref count */

     if ( dst->elem_size == 0 ) {
         AtsNode **n = (AtsNode **)(dst->data+elem_size*dst->cur_size);

         for (i = 0; i < src->cur_size; i++) 
            atsRef(n++);
     }

    dst->cur_size += src->cur_size;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsArrayAppend
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Append an element onto an array
    elements untouched
  Arguments:
    a        - the array
    e        - the element
  Return:
    FXTRUE if succesful
    FXFALSE if failed
  -------------------------------------------------------------------*/

int atsArrayAppend(AtsArray *a, char *e) {

    if ( atsArrayMaxSize(a, (FxU16)(a->cur_size + 1) ) != FXTRUE ) {
#ifdef AT_DEBUGGING
        atuError( FXTRUE, "atsArrayAppend(): Not enough memory.\n" );
#endif
        return FXFALSE;
    }

    /* optimize common case where element size is 4 */
  
    switch ( a->elem_size ) {
    case 0: {
            AtsNode **p = (AtsNode **)(a->data);
            AtsNode *n = (AtsNode *)e;
            p[a->cur_size] = n;
            atsRef(n);
        }
        break;
    case 4: {
            FxU32 *p = (FxU32 *)(a->data);
            p[a->cur_size] = (*(FxU32 *)e);
        }
    default:
        memcpy(a->data+a->elem_size*a->cur_size, e, a->elem_size);
    }

    a->cur_size++; 

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsArraySetAt
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Updates the i'th element in the array
  Arguments:
    a        - the array
    i        - which element to update
    p        - the element
  Return:
    FXTRUE if succesful
    FXFALSE if failed
  -------------------------------------------------------------------*/

int atsArraySetAt(AtsArray *a, FxU16 i, char *p) {
    int elem_size = (a->elem_size == 0) ? sizeof(AtsNode *) : a->elem_size;
    if ( i >= a->cur_size ) {
#ifdef AT_DEBUGGING
        atuError( FXTRUE, "atsArraySetAt(): index out of bounds\n" );
#endif
        return FXFALSE;

    }

    if ( a->elem_size == 0 ) {
        AtsNode **n = (AtsNode **)(a->data+elem_size*i);

        if ( *n == ( AtsNode *)p )
            return FXTRUE;
        
        atsUnrefDelete(n);
        atsRef((AtsNode *)p);
        *n = (AtsNode *)p;
    } else {
         memcpy(a->data+elem_size*i, p, elem_size);
    }
    
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsArrayGetAt
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns the i'th element in the array
  Arguments:
    a        - the array
    i        - which element to return
  Return:
    pointer to i'th element
  -------------------------------------------------------------------*/

char *atsArrayGetAt(AtsArray *a, FxU16 i) {
    int elem_size = (a->elem_size == 0) ? sizeof(AtsNode *) : a->elem_size;

    if ( i >= a->cur_size ) {
#ifdef AT_DEBUGGING
        atuError( FXTRUE, "atsArrayGetAt(): index out of bounds\n" );
#endif
        return NULL;
    }

    return a->data+elem_size*i;
}

/*-------------------------------------------------------------------
  Function: atsArraySetInt
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Updates the i'th element in an integer array
  Arguments:
    a        - the array
    i        - which element to update
    v        - the element
  Return:
    FXTRUE if succesful
    FXFALSE if failed
  -------------------------------------------------------------------*/

int atsArraySetInt(AtsArray *a, FxU16 i, int v) {
    int *p = (int *)a->data;

    if ( i >= a->cur_size ) {
#ifdef AT_DEBUGGING
        atuError( FXTRUE, "atsArraySetInt(): index out of bounds\n" );
#endif
        return FXFALSE;
    }

    p[i] = v;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsArrayGetInt
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns the i'th element in the array
  Arguments:
    a        - the array
    i        - which element to return
    v        - returns value
  Return:
    FXTRUE if succesful
    FXFALSE if failed
  -------------------------------------------------------------------*/

int atsArrayGetInt(AtsArray *a, FxU16 i, int *v) {
    int *p = (int *)a->data;

    if ( i >= a->cur_size ) {
#ifdef AT_DEBUGGING
        atuError( FXTRUE, "atsArrayGetInt(): index out of bounds\n" );
#endif
        return FXFALSE;
    }

    *v = p[i];
    
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsArraySetFloat
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Updates the i'th element in a float array
  Arguments:
    a        - the array
    i        - which element to update
    v        - the element
  Return:
    FXTRUE if succesful
    FXFALSE if failed
  -------------------------------------------------------------------*/

int atsArraySetFloat(AtsArray *a, FxU16 i, float f) {
    float *p = (float *)a->data;

    if ( i >= a->cur_size ) {
#ifdef AT_DEBUGGING
        atuError( FXTRUE, "atsArraySetFloat(): index out of bounds\n" );
#endif
        return FXFALSE;
    }

    p[i] = f;
  
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsArrayGetFloat
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns the i'th element in the array
  Arguments:
    a        - the array
    i        - which element to return
    v        - returns value
  Return:
    FXTRUE if succesful
    FXFALSE if failed
  -------------------------------------------------------------------*/

int atsArrayGetFloat(AtsArray *a, FxU16 i, float *v) {
    float *p = (float *)a->data;

    if ( i >= a->cur_size ) {
#ifdef AT_DEBUGGING
        atuError( FXTRUE, "atsArrayGetFloat(): index out of bounds\n" );
#endif
        return FXFALSE;
    }

    *v = p[i];
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsArraySetObj
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Updates the i'th element in the array
  Arguments:
    a        - the array
    i        - which element to update
    p        - the element
  Return:
    FXTRUE if succesful
    FXFALSE if failed
  -------------------------------------------------------------------*/

int atsArraySetObj(AtsArray *a, FxU16 i, AtsObject *p) {
    return atsArraySetAt(a, i, (char *)p); 
}

/*-------------------------------------------------------------------
  Function: atsArrayGetObj
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns the i'th element in the array
  Arguments:
    a        - the array
    i        - which element to return
  Return:
    pointer to i'th element
  -------------------------------------------------------------------*/

AtsObject *atsArrayGetObj(AtsArray *a, FxU16 i) {
    return (AtsObject  *)atsArrayGetAt(a, i); 
}
