/*
** Copyright (c) 2000, 3Dfx Interactive, Inc.
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
** File name: k3dldef.h
**
** Description: Lighting function case bit definitions. These are used by
** the code in ProcessLight() to pick which lighting function to use.
**
**
** $Log: 
**  7    3dfx      1.6         10/11/00 Brent           Forced check in to enforce
**       branching.
**  6    3dfx      1.5         09/28/00 Allen Hansen    started vertex blend
**       support in fastpath (still ifdef'd out)
**  5    3dfx      1.4         09/15/00 Allen Hansen    In the 3DNow path, moved
**       the texture copy from before culling to after culling ... saves a copy
**       which may not need to be done at all.  Also will make the texgen code
**       simpler.
**  4    3dfx      1.3         08/27/00 Allen Hansen    added colorvertex support
**       for 3dnow path
**  3    3dfx      1.2         07/08/00 Allen Hansen    swapped order of diffuse
**       and normals
**  2    3dfx      1.1         07/05/00 Allen Hansen    support for asm clip code
**       generation to 3dnow code
**  1    3dfx      1.0         07/03/00 Allen Hansen    
** $
*/



#define K3DL_QUAD1			(1 << 0)	// quadradic = 0,0,1 (p&s only)
#define K3DL_AMBIENT		(1 << 1)	// diffuse ambient
#define K3DL_NORMAL			(1 << 2)	// has normals
#define K3DL_DIFF			(1 << 3)	// diffuse
#define K3DL_SPECULAR		(1 << 4)	// specular
#define K3DL_LOCAL_VIEWER	(1 << 5)	// Local Viewer
#define K3DL_COLORVERTEX	(1 << 6)	// color vertex (any except emmisive)
#define K3DL_FALLOFF1		(1 << 7)	// falloff = 1.0 (spot only)

#define K3DXFORMBIT_SPECULAR_COLOR		(1 << 0)
#define K3DXFORMBIT_DIFFUSE_COLOR		(1 << 1)
#define K3DXFORMBIT_USERCLIPPLANE		(1 << 2)
#define K3DXFORMBIT_CLIPPED				(1 << 3)
#define K3DXFORMBIT_VERTEX_BLENDS_SHIFT	(4)
#define K3DXFORMBIT_VERTEX_BLENDS		(3 << K3DXFORMBIT_VERTEX_BLENDS_SHIFT)	// 2 bits (0-3 blends)
