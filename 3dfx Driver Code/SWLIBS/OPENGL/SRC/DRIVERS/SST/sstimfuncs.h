/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** $Revision: 2$ 
** $Date: 10/11/00 8:03:06 PM$ 
**
*/

/* Add SST specific immediate mode function prototypes here! */
extern void APIENTRY __glsstim_AlphaFunc( GLenum, GLclampf);
extern void APIENTRY __glsstim_BlendFunc( GLenum, GLenum);
extern void APIENTRY __glsstim_ColorMask( GLboolean, GLboolean, GLboolean, GLboolean);
extern void APIENTRY __glsstim_Clear(GLbitfield mask);
extern void APIENTRY __glsstim_CullFace(GLenum cfm);
extern void APIENTRY __glsstim_DepthFunc(GLenum zf);
extern void APIENTRY __glsstim_DepthMask(GLboolean enabled);
extern void APIENTRY __glsstim_DrawBuffer(GLenum mode);
extern void APIENTRY __glsstim_Fogfv(GLenum p, const GLfloat pv[]);
extern void APIENTRY __glsstim_Fogf(GLenum p, const GLfloat f);
extern void APIENTRY __glsstim_Fogiv(GLenum p, const GLint pv[]);
extern void APIENTRY __glsstim_Fogi(GLenum p, const GLint i);
extern void APIENTRY __glsstim_FrontFace(GLenum dir);


extern void APIENTRY __glsstim_Disable( GLenum);
extern void APIENTRY __glsstim_Enable( GLenum);
