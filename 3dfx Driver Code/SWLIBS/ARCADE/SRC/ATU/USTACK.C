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
** $Date: 10/11/00 7:34:55 PM$ 
**
*/

#include "atutil.h"
#include <string.h>

/*-------------------------------------------------------------------
  Function: atuStackAllocate
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Util ( growable stack )
  Description:
    Allocates a new stack or array of stacks.
  Arguments:
    num_stacks - number of stacks to allocate if allocating an array
  Return:
    pointer to new stack(s)
  -------------------------------------------------------------------*/
AtuStack *atuStackAllocate( FxU32 num ) {
    FxU32 *tmp;

#ifdef AT_DEBUGGING
    if ( !num ) 
        atuError( FXTRUE, "atuStackAllocate(): Invalid parameter.\n" );    
#endif    

    tmp = atuMemCalloc( num * sizeof( AtuStack ) + 4, 1 );
    *tmp = num;
    return (void*)(tmp+1);
} 

/*-------------------------------------------------------------------
  Function: atuStackDeallocate
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Util ( growable stack )
  Description:
    Safely deallocates stacks allocated with atuStackAllocate
  Arguments:
    stack - pointer to stacks
  Return:
    none
  -------------------------------------------------------------------*/
void atuStackDeallocate( AtuStack stack[] ) {
    FxU32 num;
    FxU32 *pointer;
    FxU32 index;
#ifdef AT_DEBUGGING
    if ( !stack ) 
        atuError( FXTRUE, "atuStackDeallocate(): Invalid parameter.\n" );
#endif
    pointer = (FxU32*)stack;
    num = *(pointer-1);
    for ( index = 0; index < num; index++ ) {
        if ( stack[index].base ) atuMemFree( stack[index].base );
    }
    atuMemFree( (void*)(pointer-1) );
    return;
}

/*-------------------------------------------------------------------
  Function: atuStackInit
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Util ( growable stack )
  Description:
    Initialize a stack allocated with atuStackAllocate
  Arguments:
    stack - pointer to stack
    elementSize - size of each stack element
    granularity - amount to grow the stack on overflow( in elements )
  Return:
    none
  -------------------------------------------------------------------*/
void atuStackInit( AtuStack *stack, FxU32 elementSize, FxU32 granularity ) {
#ifdef AT_DEBUGGING
    if ( !stack || !elementSize || !granularity ) {
        atuError( FXTRUE, "atuStackInit():Illegal argument.\n" );
    }
#endif
    stack->elementSize = elementSize;
    stack->granularity = granularity;
    return;
}

/*-------------------------------------------------------------------
  Function: atuStackPush
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Util ( growable stack )
  Description:
    Push an object on the stack.  An elementSize chunk of memory is 
    copied from the region pointed to by data.
  Arguments:
    stack - pointer to an initialized stack
    data - pointer to memory to be pushed
  Return:
    none
  -------------------------------------------------------------------*/
void atuStackPush( AtuStack *stack, void *data ) {
#ifdef AT_DEBUGGING
    if ( !stack || !data ) {
        atuError( FXTRUE, "atuStackPush():Illegal argument.\n" );
    }

    if ( stack->elementSize == 0 || stack->granularity == 0 ) {
        atuError( FXTRUE, "atuStackPush():Unitialized stack.\n" );
    }
#endif

	if ( stack->top == stack->max ) {
        FxU32 oldSize, chunkSize;
		unsigned char *tmp;
		tmp = stack->base;
		oldSize = stack->max - stack->base;
		chunkSize = stack->elementSize * stack->granularity;
		stack->base = (unsigned char *) atuMemMalloc( (oldSize + chunkSize) * sizeof( unsigned char ) );
		stack->top = stack->base + oldSize;
		stack->max = stack->top + chunkSize;
		if (tmp) {
		    memcpy( stack->base, tmp, oldSize );
		    atuMemFree( tmp );
        }
	}
	memcpy( stack->top, data, stack->elementSize );
	stack->top+=stack->elementSize;
    return;
}

/*-------------------------------------------------------------------
  Function: atuStackPop
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Util ( growable stack )
  Description: 
    Pop an element off of the stack and copy to region of memory
    pointed to by data.
  Arguments:
    stack - pointer to a stack
    data - pointer to memory region to receive top of stack
  Return:
    none
  -------------------------------------------------------------------*/
void atuStackPop( AtuStack *stack, void *data ) {
#ifdef AT_DEBUGGING
    if ( !stack || !data ) {
        atuError( FXTRUE, "atuStackPop():Illegal argument.\n" );
    }

    if ( stack->elementSize == 0 || stack->granularity == 0 ) {
        atuError( FXTRUE, "atuStackPop():Unitialized stack.\n" );
    }
#endif
    if ( stack->top > stack->base ) {
        stack->top -= stack->elementSize;
        memcpy( data, stack->top, stack->elementSize );
    }
    return;
}

/*-------------------------------------------------------------------
  Function: atuStackReset
  Date: 3/15/96
  Implementor(s): jdt
  Library: AT Util ( growable stack )
  Description:
    Reset a stack;
  Arguments:
    stack - pointer to a stack
  Return:
    none
  -------------------------------------------------------------------*/
void atuStackReset( AtuStack *stack ) {
#ifdef AT_DEBUGGING
    if ( !stack ) {
        atuError( FXTRUE, "atuStackReset():Illegal argument.\n" );
    }

    if ( stack->elementSize == 0 || stack->granularity == 0 ) {
        atuError( FXTRUE, "atuStackReset():Unitialized stack.\n" );
    }
#endif
    stack->top = stack->base;
    return;
}

/*-------------------------------------------------------------------
  Function: atuStackSize
  Date: 3/15/96
  Implementor(s): jdt
  Library: AT Util ( growable stack )
  Description:
    Returns the size of a stack, in elements.
  Arguments:
    stack - pointer to a stack
  Return:
    number of elements in stack
  -------------------------------------------------------------------*/
FxU32 atuStackSize( AtuStack *stack ) {
#ifdef AT_DEBUGGING
    if ( !stack ) {
        atuError( FXTRUE, "atuStackSize():Illegal argument.\n" );
    }

    if ( stack->elementSize == 0 || stack->granularity == 0 ) {
        atuError( FXTRUE, "atuStackSize():Unitialized stack.\n" );
    }
#endif
    return( ( stack->top - stack->base ) / stack->elementSize );
}

