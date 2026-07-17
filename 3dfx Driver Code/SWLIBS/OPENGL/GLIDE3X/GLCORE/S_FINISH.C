/*
** Copyright 1991-1997, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 2$
** $Date: 10/11/00 7:56:10 PM$
*/
#include "context.h"
#include "g_imfncs.h"
#include "global.h"

/*
** Finish any current rendering in progress.
** All rendering happens instantly in the samplegl, so this
** is a noop.
*/
void APIENTRY __glim_Finish(void)
{
	int stack_base;
	int *retAddr;
    __GL_SETUP_NOT_IN_BEGIN();

   	retAddr = &stack_base;
	retAddr -= 1;
	if ( *retAddr & 0x80000000 ) 
		return;

	__GL_API_FLUSH();

    (*gc->procs.finish)(gc);
}

/*
** Flush any transport data over to the server.
** This is a noop in the samplegl.
*/
void APIENTRY __glim_Flush(void)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_FLUSH();

    (*gc->procs.flush)(gc);
}

