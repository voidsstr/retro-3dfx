#ifndef __glapimacro
#define __glapimacro

/*
** Copyright 1996,1997 Silicon Graphics, Inc.
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
** $Date: 10/11/00 7:57:02 PM$
*/
/************************************************************************
 *                                                                      *
 *                               GLiNT                                  *
 *                                                                      *
 *                                                                      *
 *                   Copyright (C) 1995  3Dlabs Inc. Ltd.               *
 *                                                                      *
 *                                                                      *
 * This software and its associated documentation contains proprietary, *
 * confidential and trade secret information of 3Dlabs Ltd. and except  *
 * as provided by written agreement with 3Dlabs Ltd.                    *
 *                                                                      *
 * a) no part may be disclosed, distributed, reproduced, transmitted,   *
 *    transcribed, stored in a retieval system, adapted or translated   *
 *    in any form or by any means electronic, mechanical, magnetic,     *
 *    optical, chemical, manual or otherwise,                           *
 *                                                                      *
 *    and                                                               *
 *                                                                      *
 * b) the recipient is not entitled to discover through reverse         *
 *    engineering or reverse compiling or other such techniques or      *
 *    processes the trade secrets contained therein or in the           *
 *    documentation.                                                    *
 *                                                                      *
 ************************************************************************/

/*
** This file contains macros that appear at the start of every entry
** point in OpenGL. Depending upon the nature of the api call the macro
** is different - some need to cause buffer flushing and others don't
** for example.
*/

#if defined(__GL_VCACHE_ALLOW_CAT_PRIMITIVE)
#define __GL_API_FLUSH_CAT_PRIMITIVE()				\
    if (gc->vertexCache.vertexCacheState & VC_CAT_PRIMITIVE) {	\
	__glProcessVertexCache(gc, GL_END_VCACHE);		\
	gc->vertexCache.vertexCacheState &= ~VC_CAT_PRIMITIVE;	\
    }
#else
#define __GL_API_FLUSH_CAT_PRIMITIVE()
#endif

/*
** Note that entry points are labelled with either glim__ or gllc__.  In
** the case of the gllc__ entry points the checks are not needed and if 
** they render at the same time as building up the list, they call the 
** glim__ routines anyway.
*/

/* called by glBegin */
#define	__GL_API_BGN_RENDER()

/* called by glEnd */
#define	__GL_API_END_RENDER()

/*
** called by any rendering call other than a bgn..end type. If we get
** here, we need to flush any primitives that are currently outstanding.
** we might also need to ensure that the inverse transpose matrix is
** up to date. Again we only get here outside of a bgn..end, so any flush
** is of an end type.
*/
#define	__GL_API_NOTBE_RENDER() \
	__GL_API_FLUSH_CAT_PRIMITIVE()

/*
** eval api calls - do nothing at the moment as they call into the 
** __glim_ functions, which will perform any testing etc.
*/
#define	__GL_API_EVAL()

/*
** called by any API that will change state.  If this happens, then
** any outstanding cached rendering needs to be flushed out. It is
** assumed that if we are here, we are not in a bgn..end, so the flush
** has to happen as though it's an glEnd() type flush.
*/
#define	__GL_API_STATE() \
	__GL_API_FLUSH_CAT_PRIMITIVE()

/* Finish/flush type routines */
#define	__GL_API_FLUSH() \
	__GL_API_FLUSH_CAT_PRIMITIVE()

/* called by any api call that will get information, no flush is needed. */
#define	__GL_API_GET()

/*
** called by any pixel operation..bitmap..readpixels..accum type things.
** there is a need to flush since the operations depend upon the 
** buffer being up-to-date.
*/
#define	__GL_API_PIXEL_OP() \
	__GL_API_FLUSH_CAT_PRIMITIVE()

/*
** called by vapi type calls - vertex, color... This is here for
** completeness - these calls do not need to check for any buffer
** flushing.
*/
#define	__GL_API_VAPI()

/* This is a placeholder as well that doesn't need action */
#define	__GL_API_BLAND()

#endif /* __glapimacro */
