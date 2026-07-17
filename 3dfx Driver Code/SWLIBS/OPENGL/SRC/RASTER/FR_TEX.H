/*
** Copyright 1996, 1997, Silicon Graphics, Inc.
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
*/
#ifndef __FR_TEX_H
#define __FR_TEX_H


/**********************************************************************/

/* Filter procs */
void __glFRNearestFilterUVScaled(__GLtexture *tex, GLuint lod,
				 GLint fs, GLint ft, __GLtexel *result);
void __glFRLinearFilterUVScaled(__GLtexture *tex, GLuint lod,
				GLint fs, GLint ft, __GLtexel *result);
void __glFRNearestFilter(__GLtexture *tex, GLuint lod,
			 GLint fs, GLint ft, __GLtexel *result);
void __glFRLinearFilter(__GLtexture *tex, GLuint lod,
			GLint fs, GLint ft, __GLtexel *result);
void __glFR_NMNFilter(__GLtexture *tex, GLuint lod,
		      GLint fs, GLint ft, __GLtexel *result);
void __glFR_LMNFilter(__GLtexture *tex, GLuint lod,
		      GLint fs, GLint ft, __GLtexel *result);
void __glFR_NMLFilter(__GLtexture *tex, GLuint lod,
		      GLint fs, GLint ft, __GLtexel *result);
void __glFR_LMLFilter(__GLtexture *tex, GLuint lod,
		      GLint fs, GLint ft, __GLtexel *result);

/* Generic spanlets */
void __fastcall __glGenericExtractTexels_1(GLbitfield mask, __GLtri *tr);
void __fastcall __glGenericExtractTexels_3(GLbitfield mask, __GLtri *tr);
void __fastcall __glGenericExtractTexels_4(GLbitfield mask, __GLtri *tr);
void __fastcall __glGenericExtractTexelsPC_1(GLbitfield mask, __GLtri *tr);
void __fastcall __glGenericExtractTexelsPC_3(GLbitfield mask, __GLtri *tr);
void __fastcall __glGenericExtractTexelsPC_4(GLbitfield mask, __GLtri *tr);

void __fastcall __glGenericExtractTexelsRho_1(GLbitfield mask, __GLtri *tr);
void __fastcall __glGenericExtractTexelsRho_3(GLbitfield mask, __GLtri *tr);
void __fastcall __glGenericExtractTexelsRho_4(GLbitfield mask, __GLtri *tr);
void __fastcall __glGenericExtractTexelsPCRho_1(GLbitfield mask, __GLtri *tr);
void __fastcall __glGenericExtractTexelsPCRho_3(GLbitfield mask, __GLtri *tr);
void __fastcall __glGenericExtractTexelsPCRho_4(GLbitfield mask, __GLtri *tr);

void __fastcall __glGenericExtractTexelsMipmap_1(GLbitfield mask, __GLtri *tr);
void __fastcall __glGenericExtractTexelsMipmap_3(GLbitfield mask, __GLtri *tr);
void __fastcall __glGenericExtractTexelsMipmap_4(GLbitfield mask, __GLtri *tr);
void __fastcall
__glGenericExtractTexelsPCMipmap_1(GLbitfield mask, __GLtri *tr);
void __fastcall
__glGenericExtractTexelsPCMipmap_3(GLbitfield mask, __GLtri *tr);
void __fastcall
__glGenericExtractTexelsPCMipmap_4(GLbitfield mask, __GLtri *tr);

#endif /* __FR_TEX_H */
