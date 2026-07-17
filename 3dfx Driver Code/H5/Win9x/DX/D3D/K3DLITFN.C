/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** File name: k3dlitef.c
**
** Description: Lighting Functions for K3D -- generated code
**
** $Revision: 5$
** $Date: 10/11/00 8:50:41 PM$
**
** $Log: 
**  5    3dfx      1.4         10/11/00 Brent           Forced check in to enforce
**       branching.
**  4    3dfx      1.3         08/27/00 Allen Hansen    added colorvertex support
**       for 3dnow path
**  3    3dfx      1.2         07/21/00 Allen Hansen    added "dwK3dCmpTmp" global
**  2    3dfx      1.1         07/08/00 Allen Hansen    Fixed some of the point and
**       spot lights I had goofed up
**  1    3dfx      1.0         07/03/00 Allen Hansen    
** $
*/

#include "precomp.h"

#if( DX >= 7 )
#ifdef TnL_HAL

#ifndef WINNT
#include <d3dhal.h>
#include "d6fvf.h"
#include "fxglobal.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "fifomgr.h"
#include "d3tri.h"
#include "d6global.h"
#include "d3contxt.h"
#endif

ALIGN32 void Null_K3d_Lite(RC *pRc, TLLIGHT *pL) 
{
 // Athlon's don't like to branch 
 // to a return instuction
 _asm nop	
}

DWORD dwK3dCmpTmp;		// just a global we'll use for compares

#pragma warning(disable : 4799)  /* this shuts up the "no emms" warning */

//**********************************************************************************************
// Directional Lights
// We always do ambient (because it's just a straight add)
//**********************************************************************************************

//Case # 1
//Directional Light, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, ColorVertex 0
//#define NAME LightK3d_Dir_A0_N0_DX_SX_LX
#define NAME LightK3d_Dir_A1_N0_DX_SX_LX_V0		// directional light # 1
#define DIRECTIONAL			1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			0
#define DO_DIFFUSE			0
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Directional Light, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, ColorVertex 0	// same as case #1

//Case # 2
//Directional Light, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, ColorVertex 0
//#define NAME LightK3d_Dir_A0_N1_D1_S0_LX
#define NAME LightK3d_Dir_A1_N1_D1_S0_LX_V0		// directional light # 2
#define DIRECTIONAL			1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case # 3
//Directional Light, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, ColorVertex 0
//#define NAME LightK3d_Dir_A0_N1_D0_S1_L0
#define NAME LightK3d_Dir_A1_N1_D0_S1_L0_V0		// directional light # 3
#define DIRECTIONAL			1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case # 4
//Directional Light, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, ColorVertex 0
//#define NAME LightK3d_Dir_A0_N1_D1_S1_L0
#define NAME LightK3d_Dir_A1_N1_D1_S1_L0_V0		// directional light # 4
#define DIRECTIONAL			1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case # 5
//Directional Light, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, ColorVertex 0
//#define NAME LightK3d_Dir_A0_N1_D0_S1_L1
#define NAME LightK3d_Dir_A1_N1_D0_S1_L1_V0		// directional light # 5
#define DIRECTIONAL			1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case # 6
//Directional Light, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, ColorVertex 0
//#define NAME LightK3d_Dir_A0_N1_D1_S1_L1
#define NAME LightK3d_Dir_A1_N1_D1_S1_L1_V0		// directional light # 6
#define DIRECTIONAL			1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

// COLORVERTEX

//Case # 7
//Directional Light, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, ColorVertex 1
//#define NAME LightK3d_Dir_A0_N0_DX_SX_LX
#define NAME LightK3d_Dir_A1_N0_DX_SX_LX_V1		// directional light # 7
#define DIRECTIONAL			1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			0
#define DO_DIFFUSE			0
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Directional Light, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, ColorVertex 1	// same as case #7

//Case # 8
//Directional Light, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, ColorVertex 1
//#define NAME LightK3d_Dir_A0_N1_D1_S0_LX
#define NAME LightK3d_Dir_A1_N1_D1_S0_LX_V1		// directional light # 8
#define DIRECTIONAL			1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case # 9
//Directional Light, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, ColorVertex 1
//#define NAME LightK3d_Dir_A0_N1_D0_S1_L0
#define NAME LightK3d_Dir_A1_N1_D0_S1_L0_V1		// directional light # 9
#define DIRECTIONAL			1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case # 10
//Directional Light, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, ColorVertex 1
//#define NAME LightK3d_Dir_A0_N1_D1_S1_L0
#define NAME LightK3d_Dir_A1_N1_D1_S1_L0_V1		// directional light #10
#define DIRECTIONAL			1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case # 11
//Directional Light, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, ColorVertex 1
//#define NAME LightK3d_Dir_A0_N1_D0_S1_L1
#define NAME LightK3d_Dir_A1_N1_D0_S1_L1_V1		// directional light #11
#define DIRECTIONAL			1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case # 12
//Directional Light, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, ColorVertex 1
//#define NAME LightK3d_Dir_A0_N1_D1_S1_L1
#define NAME LightK3d_Dir_A1_N1_D1_S1_L1_V1		// directional light #12
#define DIRECTIONAL			1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"



//**********************************************************************************************
// Point Lights
//**********************************************************************************************

//Case # 1	Point Light, Quadradic 0, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q0_A0_N0_DX_SX_LX
//Case # 2	Point Light, Quadradic 1, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q1_A0_N0_DX_SX_LX

//Case # 3	Point Light, Quadradic 0, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0	// point light # 1
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			0
#define DO_DIFFUSE			0
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case # 4	Point Light, Quadradic 1, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0	// point light # 2
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			0
#define DO_DIFFUSE			0
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case # 5	Point Light, Quadradic 0, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q0_A0_N1_D0_S0_LX_V0
//Case # 6	Point Light, Quadradic 1, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q1_A0_N1_D0_S0_LX_V0
//Case # 7	Point Light, Quadradic 0, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0	// same as #1
//#define NAME LightK3d_Point_Q0_A1_N1_D0_S0_LX_V0
//Case # 8	Point Light, Quadradic 1, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0	// same as #2
//#define NAME LightK3d_Point_Q1_A1_N1_D0_S0_LX_V0
//Case # 9	Point Light, Quadradic 0, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q0_A0_N0_DX_SX_LX_V0
//Case #10	Point Light, Quadradic 1, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q1_A0_N0_DX_SX_LX_V0
//Case #11	Point Light, Quadradic 0, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0	// same as #1
//#define NAME LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0
//Case #12	Point Light, Quadradic 1, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0	// same as #2
//#define NAME LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0

//Case #13	Point Light, Quadradic 0, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Point_Q0_A0_N1_D1_S0_LX_V0	// point light # 3
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #14	Point Light, Quadradic 1, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Point_Q1_A0_N1_D1_S0_LX_V0	// point light # 4
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #15	Point Light, Quadradic 0, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Point_Q0_A1_N1_D1_S0_LX_V0	// point light # 5
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case #16	Point Light, Quadradic 1, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Point_Q1_A1_N1_D1_S0_LX_V0	// point light # 6
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case #17	Point Light, Quadradic 0, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q0_A0_N0_DX_SX_LX_V0
//Case #18	Point Light, Quadradic 1, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q1_A0_N0_DX_SX_LX_V0
//Case #19	Point Light, Quadradic 0, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0	// same as #1
//#define NAME LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0
//Case #20	Point Light, Quadradic 1, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0	// same as #2
//#define NAME LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0

//Case #21	Point Light, Quadradic 0, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Point_Q0_A0_N1_D0_S1_L0_V0	// point light # 7
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case #22	Point Light, Quadradic 1, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Point_Q1_A0_N1_D0_S1_L0_V0	// point light # 8
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case #23	Point Light, Quadradic 0, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Point_Q0_A1_N1_D0_S1_L0_V0	// point light # 9
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case #24	Point Light, Quadradic 1, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Point_Q1_A1_N1_D0_S1_L0_V0	// point light #10
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case #25	Point Light, Quadradic 0, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q0_A0_N0_DX_SX_LX_V0
//Case #26	Point Light, Quadradic 1, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q1_A0_N0_DX_SX_LX_V0
//Case #27	Point Light, Quadradic 0, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0	// same as #1
//#define NAME LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0
//Case #28	Point Light, Quadradic 1, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0	// same as #2
//#define NAME LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0

//Case #29	Point Light, Quadradic 0, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Point_Q0_A0_N1_D1_S1_L0_V0	// point light #11
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case #30	Point Light, Quadradic 1, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Point_Q1_A0_N1_D1_S1_L0_V0	// point light #12
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case #31	Point Light, Quadradic 0, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Point_Q0_A1_N1_D1_S1_L0_V0	// point light #13
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case #32	Point Light, Quadradic 1, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Point_Q1_A1_N1_D1_S1_L0_V0	// point light #14
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case #34	Point Light, Quadradic 0, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q0_A0_N0_DX_SX_LX_V0
//Case #34	Point Light, Quadradic 1, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q1_A0_N0_DX_SX_LX_V0
//Case #35	Point Light, Quadradic 0, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0	// same as #1
//#define NAME LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0
//Case #36	Point Light, Quadradic 1, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0	// same as #2
//#define NAME LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0
//Case #37	Point Light, Quadradic 0, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q0_A0_N1_D0_S0_LX_V0
//Case #38	Point Light, Quadradic 1, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q1_A0_N1_D0_S0_LX_V0
//Case #39	Point Light, Quadradic 0, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0	// same as #1
//#define NAME LightK3d_Point_Q0_A1_N1_D0_S0_LX_V0
//Case #40	Point Light, Quadradic 1, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0	// same as #2
//#define NAME LightK3d_Point_Q1_A1_N1_D0_S0_LX_V0
//Case #41	Point Light, Quadradic 0, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q0_A0_N0_DX_SX_LX_V0
//Case #42	Point Light, Quadradic 1, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q1_A0_N0_DX_SX_LX_V0
//Case #43	Point Light, Quadradic 0, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0	// same as #1
//#define NAME LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0
//Case #44	Point Light, Quadradic 1, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0	// same as #2
//#define NAME LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0
//Case #45	Point Light, Quadradic 0, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q0_A0_N1_D1_S0_LX_V0
//Case #46	Point Light, Quadradic 1, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q1_A0_N1_D1_S0_LX_V0
//Case #47	Point Light, Quadradic 0, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0	// same as #3
//#define NAME LightK3d_Point_Q0_A1_N1_D1_S0_LX_V0
//Case #48	Point Light, Quadradic 1, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0	// same as #4
//#define NAME LightK3d_Point_Q1_A1_N1_D1_S0_LX_V0
//Case #49	Point Light, Quadradic 0, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q0_A0_N0_DX_SX_LX_V0
//Case #50	Point Light, Quadradic 1, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q1_A0_N0_DX_SX_LX_V0
//Case #51	Point Light, Quadradic 0, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0	// same as #1
//#define NAME LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0
//Case #52	Point Light, Quadradic 1, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0	// same as #2
//#define NAME LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0

//Case #53	Point Light, Quadradic 0, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Point_Q0_A0_N1_D0_S1_L1_V0	// point light #15
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case #54	Point Light, Quadradic 1, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Point_Q1_A0_N1_D0_S1_L1_V0	// point light #16
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case #55	Point Light, Quadradic 0, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Point_Q0_A1_N1_D0_S1_L1_V0	// point light #17
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case #56	Point Light, Quadradic 1, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Point_Q1_A1_N1_D0_S1_L1_V0	// point light #18
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case #57	Point Light, Quadradic 0, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q0_A0_N0_DX_SX_LX_V0
//Case #58	Point Light, Quadradic 1, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0	// null light
//#define NAME LightK3d_Point_Q1_A0_N0_DX_SX_LX_V0
//Case #59	Point Light, Quadradic 0, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0	// same as #1
//#define NAME LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0
//Case #60	Point Light, Quadradic 1, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0	// same as #2
//#define NAME LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0

//Case #61	Point Light, Quadradic 0, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Point_Q0_A0_N1_D1_S1_L1_V0	// point light #19
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case #62	Point Light, Quadradic 1, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Point_Q1_A0_N1_D1_S1_L1_V0	// point light #20
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case #63	Point Light, Quadradic 0, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Point_Q0_A1_N1_D1_S1_L1_V0	// point light #21
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"


//Case #64	Point Light, Quadradic 1, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Point_Q1_A1_N1_D1_S1_L1_V0	// point light #22
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"


// Colorvertex
//Case #65	Point Light, Quadradic 0, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q0_A0_N0_DX_SX_LX_V1
//Case #66	Point Light, Quadradic 1, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q1_A0_N0_DX_SX_LX_V1

//Case #67	Point Light, Quadradic 0, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1	// point light # 23
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			0
#define DO_DIFFUSE			0
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"



//Case #68	Point Light, Quadradic 1, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1	// point light # 24
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			0
#define DO_DIFFUSE			0
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #69	Point Light, Quadradic 0, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q0_A0_N1_D0_S0_LX_V1
//Case #70	Point Light, Quadradic 1, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q1_A0_N1_D0_S0_LX_V1
//Case #71	Point Light, Quadradic 0, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1	// same as #23
//#define NAME LightK3d_Point_Q0_A1_N1_D0_S0_LX_V1
//Case #72	Point Light, Quadradic 1, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1	// same as #24
//#define NAME LightK3d_Point_Q1_A1_N1_D0_S0_LX_V1
//Case #73	Point Light, Quadradic 0, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q0_A0_N0_DX_SX_LX_V1
//Case #74	Point Light, Quadradic 1, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q1_A0_N0_DX_SX_LX_V1
//Case #75	Point Light, Quadradic 0, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1	// same as #23
//#define NAME LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1
//Case #76	Point Light, Quadradic 1, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1	// same as #24
//#define NAME LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1

//Case #77	Point Light, Quadradic 0, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Point_Q0_A0_N1_D1_S0_LX_V1	// point light # 25
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #78	Point Light, Quadradic 1, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Point_Q1_A0_N1_D1_S0_LX_V1	// point light # 26
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #79	Point Light, Quadradic 0, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Point_Q0_A1_N1_D1_S0_LX_V1	// point light # 27
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #80	Point Light, Quadradic 1, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Point_Q1_A1_N1_D1_S0_LX_V1	// point light # 28
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #81	Point Light, Quadradic 0, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q0_A0_N0_DX_SX_LX_V1
//Case #82	Point Light, Quadradic 1, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q1_A0_N0_DX_SX_LX_V1
//Case #83	Point Light, Quadradic 0, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1	// same as #23
//#define NAME LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1
//Case #84	Point Light, Quadradic 1, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1	// same as #24
//#define NAME LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1

//Case #85	Point Light, Quadradic 0, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Point_Q0_A0_N1_D0_S1_L0_V1	// point light # 29
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #86	Point Light, Quadradic 1, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Point_Q1_A0_N1_D0_S1_L0_V1	// point light # 30
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #87	Point Light, Quadradic 0, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Point_Q0_A1_N1_D0_S1_L0_V1	// point light # 31
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #88	Point Light, Quadradic 1, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Point_Q1_A1_N1_D0_S1_L0_V1	// point light #32
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #89	Point Light, Quadradic 0, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q0_A0_N0_DX_SX_LX_V1
//Case #90	Point Light, Quadradic 1, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q1_A0_N0_DX_SX_LX_V1
//Case #91	Point Light, Quadradic 0, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1	// same as #23
//#define NAME LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1
//Case #92	Point Light, Quadradic 1, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1	// same as #24
//#define NAME LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1

//Case #93	Point Light, Quadradic 0, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Point_Q0_A0_N1_D1_S1_L0_V1	// point light #33
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #94	Point Light, Quadradic 1, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Point_Q1_A0_N1_D1_S1_L0_V1	// point light #34
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #95	Point Light, Quadradic 0, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Point_Q0_A1_N1_D1_S1_L0_V1	// point light #35
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #96	Point Light, Quadradic 1, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Point_Q1_A1_N1_D1_S1_L0_V1	// point light #36
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #97	Point Light, Quadradic 0, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q0_A0_N0_DX_SX_LX_V1
//Case #98	Point Light, Quadradic 1, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q1_A0_N0_DX_SX_LX_V1
//Case #99	Point Light, Quadradic 0, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1	// same as #23
//#define NAME LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1
//Case #100	Point Light, Quadradic 1, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1	// same as #24
//#define NAME LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1
//Case #101	Point Light, Quadradic 0, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q0_A0_N1_D0_S0_LX_V1
//Case #102	Point Light, Quadradic 1, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q1_A0_N1_D0_S0_LX_V1
//Case #103	Point Light, Quadradic 0, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1	// same as #23
//#define NAME LightK3d_Point_Q0_A1_N1_D0_S0_LX_V1
//Case #104	Point Light, Quadradic 1, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1	// same as #24
//#define NAME LightK3d_Point_Q1_A1_N1_D0_S0_LX_V1
//Case #105	Point Light, Quadradic 0, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q0_A0_N0_DX_SX_LX_V1
//Case #106	Point Light, Quadradic 1, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q1_A0_N0_DX_SX_LX_V1
//Case #107	Point Light, Quadradic 0, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1	// same as #23
//#define NAME LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1
//Case #108	Point Light, Quadradic 1, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1	// same as #24
//#define NAME LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1
//Case #109	Point Light, Quadradic 0, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q0_A0_N1_D1_S0_LX_V1
//Case #110	Point Light, Quadradic 1, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q1_A0_N1_D1_S0_LX_V1
//Case #111	Point Light, Quadradic 0, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1	// same as #25
//#define NAME LightK3d_Point_Q0_A1_N1_D1_S0_LX_V1
//Case #112	Point Light, Quadradic 1, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1	// same as #26
//#define NAME LightK3d_Point_Q1_A1_N1_D1_S0_LX_V1
//Case #113	Point Light, Quadradic 0, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q0_A0_N0_DX_SX_LX_V1
//Case #114	Point Light, Quadradic 1, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q1_A0_N0_DX_SX_LX_V1
//Case #115	Point Light, Quadradic 0, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1	// same as #23
//#define NAME LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1
//Case #116	Point Light, Quadradic 1, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1	// same as #24
//#define NAME LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1

//Case #117	Point Light, Quadradic 0, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Point_Q0_A0_N1_D0_S1_L1_V1	// point light #37
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #118	Point Light, Quadradic 1, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Point_Q1_A0_N1_D0_S1_L1_V1	// point light #38
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #119	Point Light, Quadradic 0, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Point_Q0_A1_N1_D0_S1_L1_V1	// point light #39
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #120	Point Light, Quadradic 1, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Point_Q1_A1_N1_D0_S1_L1_V1	// point light #40
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #121	Point Light, Quadradic 0, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q0_A0_N0_DX_SX_LX_V1
//Case #122	Point Light, Quadradic 1, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1	// null light
//#define NAME LightK3d_Point_Q1_A0_N0_DX_SX_LX_V1
//Case #123	Point Light, Quadradic 0, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1	// same as #23
//#define NAME LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1
//Case #124	Point Light, Quadradic 1, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1	// same as #24
//#define NAME LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1

//Case #125	Point Light, Quadradic 0, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Point_Q0_A0_N1_D1_S1_L1_V1	// point light #41
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #126	Point Light, Quadradic 1, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Point_Q1_A0_N1_D1_S1_L1_V1	// point light #42
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #127	Point Light, Quadradic 0, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Point_Q0_A1_N1_D1_S1_L1_V1	// point light #43
#define POINT				1
#define DO_QUAD				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"


//Case #128	Point Light, Quadradic 1, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Point_Q1_A1_N1_D1_S1_L1_V1	// point light #44
#define POINT				1
#define DO_QUAD				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"



//**********************************************************************************************
// Spot Lights
//**********************************************************************************************

//Case # 1	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N0_DX_SX_LX_V0
//Case # 2	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N0_DX_SX_LX_V0

//Case # 3	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0	// spot light # 1
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			0
#define DO_DIFFUSE			0
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case # 4	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0	// spot light # 2
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			0
#define DO_DIFFUSE			0
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case # 5	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N1_D0_S0_LX_V0
//Case # 6	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N1_D0_S0_LX_V0
//Case # 7	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0 // same as case #1
//#define NAME LightK3d_Spot_Q0_F0_A1_N1_D0_S0_LX_V0
//Case # 8	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0 // same as case #2
//#define NAME LightK3d_Spot_Q1_F0_A1_N1_D0_S0_LX_V0
//Case # 9	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N0_DX_SX_LX_V0
//Case #10	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N0_DX_SX_LX_V0
//Case #11	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0 // same as case #1
//#define NAME LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0
//Case #12	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0 // same as case #2
//#define NAME LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0

//Case #13	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q0_F0_A0_N1_D1_S0_LX_V0	// spot light #	3
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #14	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q1_F0_A0_N1_D1_S0_LX_V0	// spot light #	4
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #15	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q0_F0_A1_N1_D1_S0_LX_V0	// spot light #	5
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #16	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q1_F0_A1_N1_D1_S0_LX_V0	// spot light #	6
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #17	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N0_DX_SX_LX_V0
//Case #18	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N0_DX_SX_LX_V0
//Case #19	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0 // same as case #1
//#define NAME LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0
//Case #20	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0 // same as case #2
//#define NAME LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0

//Case #21	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q0_F0_A0_N1_D0_S1_L0_V0	// spot light #	7
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #22	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q1_F0_A0_N1_D0_S1_L0_V0	// spot light #	8
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #23	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q0_F0_A1_N1_D0_S1_L0_V0	// spot light #	9
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #24	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q1_F0_A1_N1_D0_S1_L0_V0	// spot light #10
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #25	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N0_DX_SX_LX_V0
//Case #26	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N0_DX_SX_LX_V0
//Case #27	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0 // same as case #1
//#define NAME LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0
//Case #28	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0 // same as case #2
//#define NAME LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0

//Case #29	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q0_F0_A0_N1_D1_S1_L0_V0	// spot light #11
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #30	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q1_F0_A0_N1_D1_S1_L0_V0	// spot light #12
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #31	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q0_F0_A1_N1_D1_S1_L0_V0	// spot light #13
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #32	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q1_F0_A1_N1_D1_S1_L0_V0	// spot light #14
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #33	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N0_DX_SX_LX_V0
//Case #34	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N0_DX_SX_LX_V0
//Case #35	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0	// same as case #1
//#define NAME LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0
//Case #36	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0	// same as case #2
//#define NAME LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0
//Case #37	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N1_D0_S0_LX_V0
//Case #38	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N1_D0_S0_LX_V0
//Case #39	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0	// same as case #1
//#define NAME LightK3d_Spot_Q0_F0_A1_N1_D0_S0_LX_V0
//Case #40	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0	// same as case #2
//#define NAME LightK3d_Spot_Q1_F0_A1_N1_D0_S0_LX_V0
//Case #41	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N0_DX_SX_LX_V0
//Case #42	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N0_DX_SX_LX_V0
//Case #43	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0	// same as case #1
//#define NAME LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0
//Case #44	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0	// same as case #2
//#define NAME LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0
//Case #45	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0	// same as case #3
//#define NAME LightK3d_Spot_Q0_F0_A0_N1_D1_S0_LX_V0
//Case #46	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0	// same as case #4
//#define NAME LightK3d_Spot_Q1_F0_A0_N1_D1_S0_LX_V0
//Case #47	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0	// same as case #5
//#define NAME LightK3d_Spot_Q0_F0_A1_N1_D1_S0_LX_V0
//Case #48	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0	// same as case #6
//#define NAME LightK3d_Spot_Q1_F0_A1_N1_D1_S0_LX_V0
//Case #49	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N0_DX_SX_LX_V0
//Case #50	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N0_DX_SX_LX_V0
//Case #51	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0	// same as case #1
//#define NAME LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0
//Case #52	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0	// same as case #2
//#define NAME LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0

//Case #53	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Spot_Q0_F0_A0_N1_D0_S1_L1_V0	// spot light #15
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #53	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Spot_Q1_F0_A0_N1_D0_S1_L1_V0	// spot light #16
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #55	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Spot_Q0_F0_A1_N1_D0_S1_L1_V0	// spot light #17
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #56	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Spot_Q1_F0_A1_N1_D0_S1_L1_V0	// spot light #18
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #57	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N0_DX_SX_LX_V0
//Case #58	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N0_DX_SX_LX_V0
//Case #59	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0	// same as case #1
//#define NAME LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0
//Case #60	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0	// same as case #2
//#define NAME LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0

//Case #61	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Spot_Q0_F0_A0_N1_D1_S1_L1_V0	// spot light #19
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #62	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Spot_Q1_F0_A0_N1_D1_S1_L1_V0	// spot light #20
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #63	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Spot_Q0_F0_A1_N1_D1_S1_L1_V0	// spot light #21
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #64	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Spot_Q1_F0_A1_N1_D1_S1_L1_V0	// spot light #22
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

/*------------------------- COLORVERTEX ON -------------------------*/
//Case #65	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N0_DX_SX_LX_V1
//Case #66	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N0_DX_SX_LX_V1

//Case #67	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1	// spot light #23
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			0
#define DO_DIFFUSE			0
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #68	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1	// spot light #24
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			0
#define DO_DIFFUSE			0
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #69	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N1_D0_S0_LX_V1
//Case #70	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N1_D0_S0_LX_V1
//Case #71	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1 // same as case #65
//#define NAME LightK3d_Spot_Q0_F0_A1_N1_D0_S0_LX_V1
//Case #72	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1 // same as case #66
//#define NAME LightK3d_Spot_Q1_F0_A1_N1_D0_S0_LX_V1
//Case #73	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N0_DX_SX_LX_V1
//Case #74	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N0_DX_SX_LX_V1
//Case #75	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1 // same as case #65
//#define NAME LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1
//Case #76	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1 // same as case #66
//#define NAME LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1

//Case #77	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q0_F0_A0_N1_D1_S0_LX_V1	// spot light #25
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #78	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q1_F0_A0_N1_D1_S0_LX_V1	// spot light #26
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #79	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q0_F0_A1_N1_D1_S0_LX_V1	// spot light #27
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #80	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q1_F0_A1_N1_D1_S0_LX_V1	// spot light #28
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #81	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N0_DX_SX_LX_V1
//Case #82	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N0_DX_SX_LX_V1
//Case #83	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1 // same as case #65
//#define NAME LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1
//Case #84	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1 // same as case #66
//#define NAME LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1

//Case #85	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q0_F0_A0_N1_D0_S1_L0_V1	// spot light #29
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #86	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q1_F0_A0_N1_D0_S1_L0_V1	// spot light #30
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #87	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q0_F0_A1_N1_D0_S1_L0_V1	// spot light #31
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #88	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q1_F0_A1_N1_D0_S1_L0_V1	// spot light #32
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #89	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N0_DX_SX_LX_V1
//Case #90	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N0_DX_SX_LX_V1
//Case #91	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1 // same as case #65
//#define NAME LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1
//Case #92	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1 // same as case #66
//#define NAME LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1

//Case #93	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q0_F0_A0_N1_D1_S1_L0_V1	// spot light #33
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #94	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q1_F0_A0_N1_D1_S1_L0_V1	// spot light #34
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #95	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q0_F0_A1_N1_D1_S1_L0_V1	// spot light #35
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #96	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q1_F0_A1_N1_D1_S1_L0_V1	// spot light #36
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #97	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N0_DX_SX_LX_V1
//Case #98	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N0_DX_SX_LX_V1
//Case #99	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1 // same as case #65
//#define NAME LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1
//Case #100	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1 // same as case #66
//#define NAME LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1
//Case #101	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N1_D0_S0_LX_V1
//Case #102	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N1_D0_S0_LX_V1
//Case #103	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1 // same as case #65
//#define NAME LightK3d_Spot_Q0_F0_A1_N1_D0_S0_LX_V1
//Case #104	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1 // same as case #66
//#define NAME LightK3d_Spot_Q1_F0_A1_N1_D0_S0_LX_V1
//Case #105	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N0_DX_SX_LX_V1
//Case #106	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N0_DX_SX_LX_V1
//Case #107	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1 // same as case #65
//#define NAME LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1
//Case #108	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1 // same as case #66
//#define NAME LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1
//Case #109	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1 // same as case #67
//#define NAME LightK3d_Spot_Q0_F0_A0_N1_D1_S0_LX_V1
//Case #110	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1 // same as case #68
//#define NAME LightK3d_Spot_Q1_F0_A0_N1_D1_S0_LX_V1
//Case #111	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1 // same as case #69
//#define NAME LightK3d_Spot_Q0_F0_A1_N1_D1_S0_LX_V1
//Case #112	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1 // same as case #70
//#define NAME LightK3d_Spot_Q1_F0_A1_N1_D1_S0_LX_V1
//Case #113	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N0_DX_SX_LX_V1
//Case #114	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N0_DX_SX_LX_V1
//Case #115	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1 // same as case #65
//#define NAME LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1
//Case #116	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1 // same as case #66
//#define NAME LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1

//Case #117	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Spot_Q0_F0_A0_N1_D0_S1_L1_V1	// spot light #37
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #118	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Spot_Q1_F0_A0_N1_D0_S1_L1_V1	// spot light #38
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #119	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Spot_Q0_F0_A1_N1_D0_S1_L1_V1	// spot light #39
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #120	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Spot_Q1_F0_A1_N1_D0_S1_L1_V1	// spot light #40
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #121	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F0_A0_N0_DX_SX_LX_V1
//Case #122	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F0_A0_N0_DX_SX_LX_V1
//Case #123	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1 // same as case #65
//#define NAME LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1
//Case #124	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1 // same as case #66
//#define NAME LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1

//Case #125	Spot Light, Quadradic 0, Falloff 0, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Spot_Q0_F0_A0_N1_D1_S1_L1_V1	// spot light #41
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #126	Spot Light, Quadradic 1, Falloff 0, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Spot_Q1_F0_A0_N1_D1_S1_L1_V1	// spot light #42
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #127	Spot Light, Quadradic 0, Falloff 0, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Spot_Q0_F0_A1_N1_D1_S1_L1_V1	// spot light #43
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #128	Spot Light, Quadradic 1, Falloff 0, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Spot_Q1_F0_A1_N1_D1_S1_L1_V1	// spot light #44
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				0
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"

/*------------------------- COLORVERTEX OFF, FALLOFF ON -------------------------*/
/*------------------------- COLORVERTEX ON -------------------------*/
//Case #129	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N0_DX_SX_LX_V0
//Case #130	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N0_DX_SX_LX_V0

//Case #131	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0	// spot light #45
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			0
#define DO_DIFFUSE			0
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #132	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0	// spot light #46
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			0
#define DO_DIFFUSE			0
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #133	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N1_D0_S0_LX_V0
//Case #134	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N1_D0_S0_LX_V0
//Case #135	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0 // same as case #129
//#define NAME LightK3d_Spot_Q0_F1_A1_N1_D0_S0_LX_V0
//Case #136	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0 // same as case #130
//#define NAME LightK3d_Spot_Q1_F1_A1_N1_D0_S0_LX_V0
//Case #137	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N0_DX_SX_LX_V0
//Case #138	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N0_DX_SX_LX_V0
//Case #139	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0 // same as case #129
//#define NAME LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0
//Case #140	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0 // same as case #130
//#define NAME LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0

//Case #141	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q0_F1_A0_N1_D1_S0_LX_V0	// spot light #47
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #142	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q1_F1_A0_N1_D1_S0_LX_V0	// spot light #48
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #143	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q0_F1_A1_N1_D1_S0_LX_V0	// spot light #49
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #144	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q1_F1_A1_N1_D1_S0_LX_V0	// spot light #50
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #145	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N0_DX_SX_LX_V0
//Case #146	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N0_DX_SX_LX_V0
//Case #147	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0 // same as case #129
//#define NAME LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0
//Case #148	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0 // same as case #130
//#define NAME LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0

//Case #149	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q0_F1_A0_N1_D0_S1_L0_V0	// spot light #51
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #150	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q1_F1_A0_N1_D0_S1_L0_V0	// spot light #52
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #151	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q0_F1_A1_N1_D0_S1_L0_V0	// spot light #53
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #152	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q1_F1_A1_N1_D0_S1_L0_V0	// spot light #54
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #153	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N0_DX_SX_LX_V0
//Case #154	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N0_DX_SX_LX_V0
//Case #155	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0 // same as case #129
//#define NAME LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0
//Case #156	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0 // same as case #130
//#define NAME LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0

//Case #157	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q0_F1_A0_N1_D1_S1_L0_V0	// spot light #55
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #158	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q1_F1_A0_N1_D1_S1_L0_V0	// spot light #56
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #159	Spot Light, Quadradic 0, Falloff 1, Diffuse 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q0_F1_A1_N1_D1_S1_L0_V0	// spot light #57
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #160	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q1_F1_A1_N1_D1_S1_L0_V0	// spot light #58
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #161	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N0_DX_SX_LX_V0
//Case #162	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N0_DX_SX_LX_V0
//Case #163	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0 // same as case #129
//#define NAME LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0
//Case #164	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0 // same as case #130
//#define NAME LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0
//Case #165	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N1_D0_S0_LX_V0
//Case #166	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N1_D0_S0_LX_V0
//Case #167	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0 // same as case #129
//#define68AME LightK3d_Spot_Q0_F1_A1_N1_D0_S0_LX_V0
//Case #168	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 0 // same as case #130
//#define NAME LightK3d_Spot_Q1_F1_A1_N1_D0_S0_LX_V0
//Case #169	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N0_DX_SX_LX_V0
//Case #170	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N0_DX_SX_LX_V0
//Case #171	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0 // same as case #129
//#define NAME LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0
//Case #172	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0 // same as case #130
//#define NAME LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0
//Case #173	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0 // same as case #131
//#define NAME LightK3d_Spot_Q0_F1_A0_N1_D1_S0_LX_V0
//Case #174	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0 // same as case #132
//#define NAME LightK3d_Spot_Q1_F1_A0_N1_D1_S0_LX_V0
//Case #175	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0 // same as case #133
//#define NAME LightK3d_Spot_Q0_F1_A1_N1_D1_S0_LX_V0
//Case #176	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 0 // same as case #134
//#define NAME LightK3d_Spot_Q1_F1_A1_N1_D1_S0_LX_V0
//Case #177	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N0_DX_SX_LX_V0
//Case #178	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N0_DX_SX_LX_V0
//Case #179	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0 // same as case #129
//#define NAME LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0
//Case #180	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0 // same as case #130
//#define NAME LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0

//Case #181	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Spot_Q0_F1_A0_N1_D0_S1_L1_V0	// spot light #59
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #182	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Spot_Q1_F1_A0_N1_D0_S1_L1_V0	// spot light #60
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #183	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Spot_Q0_F1_A1_N1_D0_S1_L1_V0	// spot light #61
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #184	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Spot_Q1_F1_A1_N1_D0_S1_L1_V0	// spot light #62
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #185	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N0_DX_SX_LX_V0
//Case #186	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N0_DX_SX_LX_V0
//Case #187	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0 // same as case #129
//#define NAME LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0
//Case #188	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0 // same as case #130
//#define NAME LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0

//Case #189	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Spot_Q0_F1_A0_N1_D1_S1_L1_V0	// spot light #63
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #190	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Spot_Q1_F1_A0_N1_D1_S1_L1_V0	// spot light #64
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #191	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Spot_Q0_F1_A1_N1_D1_S1_L1_V0	// spot light #65
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

//Case #192	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 0
#define NAME LightK3d_Spot_Q1_F1_A1_N1_D1_S1_L1_V0	// spot light #66
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		0
#include "k3dlf.h"

/*------------------------- COLORVERTEX ON -------------------------*/
//Case #193	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N0_DX_SX_LX_V1
//Case #194	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N0_DX_SX_LX_V1

//Case #195	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1	// spot light #67
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			0
#define DO_DIFFUSE			0
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #196	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 0
#define NAME LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1	// spot light #68
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			0
#define DO_DIFFUSE			0
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #197	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N1_D0_S0_LX_V1
//Case #198	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N1_D0_S0_LX_V1
//Case #199	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1 // same as case #195
//#define NAME LightK3d_Spot_Q0_F1_A1_N1_D0_S0_LX_V1
//Case #200	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 0, colorVertex 1 // same as case #196
//#define NAME LightK3d_Spot_Q1_F1_A1_N1_D0_S0_LX_V1
//Case #201	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N0_DX_SX_LX_V1
//Case #202	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N0_DX_SX_LX_V1
//Case #203	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1 // same as case #195
//#define NAME LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1
//Case #204	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1 // same as case #196
//#define NAME LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1

//Case #205	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q0_F1_A0_N1_D1_S0_LX_V1	// spot light #69
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #206	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q1_F1_A0_N1_D1_S0_LX_V1	// spot light #70
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #207	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q0_F1_A1_N1_D1_S0_LX_V1	// spot light #71
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #208	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q1_F1_A1_N1_D1_S0_LX_V1	// spot light #72
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			0
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #209	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N0_DX_SX_LX_V1
//Case #210	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N0_DX_SX_LX_V1
//Case #211	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1 // same as case #195
//#define NAME LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1
//Case #212	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1 // same as case #196
//#define NAME LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1

//Case #213	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q0_F1_A0_N1_D0_S1_L0_V1	// spot light #73
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #214	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q1_F1_A0_N1_D0_S1_L0_V1	// spot light #74
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #215	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q0_F1_A1_N1_D0_S1_L0_V1	// spot light #75
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #216	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q1_F1_A1_N1_D0_S1_L0_V1	// spot light #76
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #217	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N0_DX_SX_LX_V1
//Case #218	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N0_DX_SX_LX_V1
//Case #219	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1 // same as case #195
//#define NAME LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1
//Case #220	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1 // same as case #196
//#define NAME LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1

//Case #221	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q0_F1_A0_N1_D1_S1_L0_V1	// spot light #77
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #222	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q1_F1_A0_N1_D1_S1_L0_V1	// spot light #78
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #223	Spot Light, Quadradic 0, Falloff 1, Diffuse 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q0_F1_A1_N1_D1_S1_L0_V1	// spot light #79
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #223	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 0, colorVertex 1
#define NAME LightK3d_Spot_Q1_F1_A1_N1_D1_S1_L0_V1	// spot light #80
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		0
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #225	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N0_DX_SX_LX_V1
//Case #226	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N0_DX_SX_LX_V1
//Case #227	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1 // same as case #195
//#define NAME LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1
//Case #228	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 0, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1 // same as case #196
//#define NAME LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1
//Case #229	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N1_D0_S0_LX_V1
//Case #230	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N1_D0_S0_LX_V1
//Case #231	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1 // same as case #195
//#define NAME LightK3d_Spot_Q0_F1_A1_N1_D0_S0_LX_V1
//Case #232	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 1, Diffuse 0, Specular 0, LocalViewer 1, colorVertex 1 // same as case #196
//#define NAME LightK3d_Spot_Q1_F1_A1_N1_D0_S0_LX_V1
//Case #233	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N0_DX_SX_LX_V1
//Case #234	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N0_DX_SX_LX_V1
//Case #235	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1 // same as case #195
//#define NAME LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1
//Case #236	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 0, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1 // same as case #196
//#define NAME LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1
//Case #237	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1 // same as case #197
//#define NAME LightK3d_Spot_Q0_F1_A0_N1_D1_S0_LX_V1
//Case #238	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1 // same as case #198
//#define NAME LightK3d_Spot_Q1_F1_A0_N1_D1_S0_LX_V1
//Case #239	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1 // same as case #199
//#define NAME LightK3d_Spot_Q0_F1_A1_N1_D1_S0_LX_V1
//Case #240	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 1, Diffuse 1, Specular 0, LocalViewer 1, colorVertex 1 // same as case #200
//#define NAME LightK3d_Spot_Q1_F1_A1_N1_D1_S0_LX_V1
//Case #241	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N0_DX_SX_LX_V1
//Case #242	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N0_DX_SX_LX_V1
//Case #243	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1 // same as case #195
//#define NAME LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1
//Case #244	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 0, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1 // same as case #196
//#define NAME LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1

//Case #245	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Spot_Q0_F1_A0_N1_D0_S1_L1_V1	// spot light #81
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #246	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Spot_Q1_F1_A0_N1_D0_S1_L1_V1	// spot light #82
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #247	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Spot_Q0_F1_A1_N1_D0_S1_L1_V1	// spot light #83
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #248	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 1, Diffuse 0, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Spot_Q1_F1_A1_N1_D0_S1_L1_V1	// spot light #84
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			0
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #249	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q0_F1_A0_N0_DX_SX_LX_V1
//Case #250	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1 // null light
//#define NAME LightK3d_Spot_Q1_F1_A0_N0_DX_SX_LX_V1
//Case #251	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1 // same as case #195
//#define NAME LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1
//Case #252	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 0, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1 // same as case #196
//#define NAME LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1

//Case #253	Spot Light, Quadradic 0, Falloff 1, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Spot_Q0_F1_A0_N1_D1_S1_L1_V1	// spot light #85
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #254	Spot Light, Quadradic 1, Falloff 1, Ambient 0, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Spot_Q1_F1_A0_N1_D1_S1_L1_V1	// spot light #86
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	0
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #255	Spot Light, Quadradic 0, Falloff 1, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Spot_Q0_F1_A1_N1_D1_S1_L1_V1	// spot light #87
#define SPOT				1
#define DO_QUAD				0
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"

//Case #256	Spot Light, Quadradic 1, Falloff 1, Ambient 1, Normal 1, Diffuse 1, Specular 1, LocalViewer 1, colorVertex 1
#define NAME LightK3d_Spot_Q1_F1_A1_N1_D1_S1_L1_V1	// spot light #88
#define SPOT				1
#define DO_QUAD				1
#define FALLOFF				1
#define DO_DIFFUSE_AMBIENT	1
#define HAS_NORMALS			1
#define DO_DIFFUSE			1
#define DO_SPECULAR			1
#define DO_LOCALVIEWER		1
#define DO_COLORVERTEX		1
#include "k3dlf.h"




//********************************************************************************
// Tables
//********************************************************************************
// Directionals: QUAD1 is always 0, DIFFA is always 0, Falloff doesn't matter
const TLLIGHTVERTEXFN	K3D_DirectionalLight_fns[] = { 
	// specular = 0, local viewer = 0, color vertex = 0
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V0, &LightK3d_Dir_A1_N0_DX_SX_LX_V0,	// no normals
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V0, &LightK3d_Dir_A1_N0_DX_SX_LX_V0,	// normals but colors off
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V0, &LightK3d_Dir_A1_N0_DX_SX_LX_V0,	// no normals
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N1_D1_S0_LX_V0, &LightK3d_Dir_A1_N1_D1_S0_LX_V0,	// normals, diffuse
	// specular = 1, local viewer = 0, color vertex = 0
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V0, &LightK3d_Dir_A1_N0_DX_SX_LX_V0,	// no normals
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N1_D0_S1_L0_V0, &LightK3d_Dir_A1_N1_D0_S1_L0_V0,	// normals, specular
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V0, &LightK3d_Dir_A1_N0_DX_SX_LX_V0,	// no normals
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N1_D1_S1_L0_V0, &LightK3d_Dir_A1_N1_D1_S1_L0_V0,	// normals, diffuse + specular
	// specular = 0, local viewer = 1, color vertex = 0
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V0, &LightK3d_Dir_A1_N0_DX_SX_LX_V0,	// no normals
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V0, &LightK3d_Dir_A1_N0_DX_SX_LX_V0,	// normals but colors off
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V0, &LightK3d_Dir_A1_N0_DX_SX_LX_V0,	// no normals
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N1_D1_S0_LX_V0, &LightK3d_Dir_A1_N1_D1_S0_LX_V0,	// normals, diffuse
	// specular = 1, local viewer = 1, color vertex = 0
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V0, &LightK3d_Dir_A1_N0_DX_SX_LX_V0,	// no normals
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N1_D0_S1_L1_V0, &LightK3d_Dir_A1_N1_D0_S1_L1_V0,	// normals, specular-local
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V0, &LightK3d_Dir_A1_N0_DX_SX_LX_V0,	// no normals
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N1_D1_S1_L1_V0, &LightK3d_Dir_A1_N1_D1_S1_L1_V0,	// normals, diffuse + specular-local
	// specular = 0, local viewer = 0, color vertex = 1
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V1, &LightK3d_Dir_A1_N0_DX_SX_LX_V1,	// no normals
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V1, &LightK3d_Dir_A1_N0_DX_SX_LX_V1,	// normals but colors off
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V1, &LightK3d_Dir_A1_N0_DX_SX_LX_V1,	// no normals
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N1_D1_S0_LX_V1, &LightK3d_Dir_A1_N1_D1_S0_LX_V1,	// normals, diffuse
	// specular = 1, local viewer = 0, color vertex = 1
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V1, &LightK3d_Dir_A1_N0_DX_SX_LX_V1,	// no normals
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N1_D0_S1_L0_V1, &LightK3d_Dir_A1_N1_D0_S1_L0_V1,	// normals, specular
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V1, &LightK3d_Dir_A1_N0_DX_SX_LX_V1,	// no normals
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N1_D1_S1_L0_V1, &LightK3d_Dir_A1_N1_D1_S1_L0_V1,	// normals, diffuse + specular
	// specular = 0, local viewer = 1, color vertex = 1
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V1, &LightK3d_Dir_A1_N0_DX_SX_LX_V1,	// no normals
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V1, &LightK3d_Dir_A1_N0_DX_SX_LX_V1,	// normals but colors off
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V1, &LightK3d_Dir_A1_N0_DX_SX_LX_V1,	// no normals
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N1_D1_S0_LX_V1, &LightK3d_Dir_A1_N1_D1_S0_LX_V1,	// normals, diffuse
	// specular = 1, local viewer = 1, color vertex = 1
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V1, &LightK3d_Dir_A1_N0_DX_SX_LX_V1,	// no normals
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N1_D0_S1_L1_V1, &LightK3d_Dir_A1_N1_D0_S1_L1_V1,	// normals, specular-local
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N0_DX_SX_LX_V1, &LightK3d_Dir_A1_N0_DX_SX_LX_V1,	// no normals
	&Null_K3d_Lite, &Null_K3d_Lite, &LightK3d_Dir_A1_N1_D1_S1_L1_V1, &LightK3d_Dir_A1_N1_D1_S1_L1_V1,	// normals, diffuse + specular-local
	};

// Points: Falloff doesn't matter here
const TLLIGHTVERTEXFN	K3D_PointLight_fns[] = {
	// specular = 0, local viewer = 0, color vertex = 0
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0,	// no normals
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0,	// normals but no colors
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Point_Q0_A0_N1_D1_S0_LX_V0, &LightK3d_Point_Q1_A0_N1_D1_S0_LX_V0, &LightK3d_Point_Q0_A1_N1_D1_S0_LX_V0, &LightK3d_Point_Q1_A1_N1_D1_S0_LX_V0,	// diffuse
	// specular = 1, local viewer = 0, color vertex = 0
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Point_Q0_A0_N1_D0_S1_L0_V0, &LightK3d_Point_Q1_A0_N1_D0_S1_L0_V0, &LightK3d_Point_Q0_A1_N1_D0_S1_L0_V0, &LightK3d_Point_Q1_A1_N1_D0_S1_L0_V0,	// normals, specular only
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Point_Q0_A0_N1_D1_S1_L0_V0, &LightK3d_Point_Q1_A0_N1_D1_S1_L0_V0, &LightK3d_Point_Q0_A1_N1_D1_S1_L0_V0, &LightK3d_Point_Q1_A1_N1_D1_S1_L0_V0,	// diffuse + specular
	// specular = 0, local viewer = 1, color vertex = 0
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0,	// no normals
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0,	// normals but no colors
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Point_Q0_A0_N1_D1_S0_LX_V0, &LightK3d_Point_Q1_A0_N1_D1_S0_LX_V0, &LightK3d_Point_Q0_A1_N1_D1_S0_LX_V0, &LightK3d_Point_Q1_A1_N1_D1_S0_LX_V0,	// diffuse
	// specular = 1, local viewer = 1, color vertex = 0
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Point_Q0_A0_N1_D0_S1_L1_V0, &LightK3d_Point_Q1_A0_N1_D0_S1_L1_V0, &LightK3d_Point_Q0_A1_N1_D0_S1_L1_V0, &LightK3d_Point_Q1_A1_N1_D0_S1_L1_V0,	// normals, specular only
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V0, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Point_Q0_A0_N1_D1_S1_L1_V0, &LightK3d_Point_Q1_A0_N1_D1_S1_L1_V0, &LightK3d_Point_Q0_A1_N1_D1_S1_L1_V0, &LightK3d_Point_Q1_A1_N1_D1_S1_L1_V0,	// diffuse + specular
	// specular = 0, local viewer = 0, color vertex = 1
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1,	// no normals
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1,	// normals but no colors
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Point_Q0_A0_N1_D1_S0_LX_V1, &LightK3d_Point_Q1_A0_N1_D1_S0_LX_V1, &LightK3d_Point_Q0_A1_N1_D1_S0_LX_V1, &LightK3d_Point_Q1_A1_N1_D1_S0_LX_V1,	// diffuse
	// specular = 1, local viewer = 0, color vertex = 1
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Point_Q0_A0_N1_D0_S1_L0_V1, &LightK3d_Point_Q1_A0_N1_D0_S1_L0_V1, &LightK3d_Point_Q0_A1_N1_D0_S1_L0_V1, &LightK3d_Point_Q1_A1_N1_D0_S1_L0_V1,	// normals, specular only
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Point_Q0_A0_N1_D1_S1_L0_V1, &LightK3d_Point_Q1_A0_N1_D1_S1_L0_V1, &LightK3d_Point_Q0_A1_N1_D1_S1_L0_V1, &LightK3d_Point_Q1_A1_N1_D1_S1_L0_V1,	// diffuse + specular
	// specular = 0, local viewer = 1, color vertex = 1
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1,	// no normals
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1,	// normals but no colors
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Point_Q0_A0_N1_D1_S0_LX_V1, &LightK3d_Point_Q1_A0_N1_D1_S0_LX_V1, &LightK3d_Point_Q0_A1_N1_D1_S0_LX_V1, &LightK3d_Point_Q1_A1_N1_D1_S0_LX_V1,	// diffuse
	// specular = 1, local viewer = 1, color vertex = 1
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Point_Q0_A0_N1_D0_S1_L1_V1, &LightK3d_Point_Q1_A0_N1_D0_S1_L1_V1, &LightK3d_Point_Q0_A1_N1_D0_S1_L1_V1, &LightK3d_Point_Q1_A1_N1_D0_S1_L1_V1,	// normals, specular only
	&Null_K3d_Lite,                       &Null_K3d_Lite,                       &LightK3d_Point_Q0_A1_N0_DX_SX_LX_V1, &LightK3d_Point_Q1_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Point_Q0_A0_N1_D1_S1_L1_V1, &LightK3d_Point_Q1_A0_N1_D1_S1_L1_V1, &LightK3d_Point_Q0_A1_N1_D1_S1_L1_V1, &LightK3d_Point_Q1_A1_N1_D1_S1_L1_V1,	// diffuse + specular
	};

const TLLIGHTVERTEXFN	K3D_SpotLight_fns[] = {
	// specular = 0, local viewer = 0, color vertex = 0, falloff = 0
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0,	// no normals
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0,	// normals but no colors
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Spot_Q0_F0_A0_N1_D1_S0_LX_V0, &LightK3d_Spot_Q1_F0_A0_N1_D1_S0_LX_V0, &LightK3d_Spot_Q0_F0_A1_N1_D1_S0_LX_V0, &LightK3d_Spot_Q1_F0_A1_N1_D1_S0_LX_V0,	// diffuse
	// specular = 1, local viewer = 0, color vertex = 0, falloff = 0
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Spot_Q0_F0_A0_N1_D1_S1_L0_V0, &LightK3d_Spot_Q1_F0_A0_N1_D1_S1_L0_V0, &LightK3d_Spot_Q0_F0_A1_N1_D0_S1_L0_V0, &LightK3d_Spot_Q1_F0_A1_N1_D0_S1_L0_V0,	// normals, specular only
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Spot_Q0_F0_A0_N1_D1_S1_L0_V0, &LightK3d_Spot_Q1_F0_A0_N1_D1_S1_L0_V0, &LightK3d_Spot_Q0_F0_A1_N1_D1_S1_L0_V0, &LightK3d_Spot_Q1_F0_A1_N1_D1_S1_L0_V0,	// diffuse + specular
	// specular = 0, local viewer = 1, color vertex = 0, falloff = 0
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0,	// no normals
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0,	// normals but no colors
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Spot_Q0_F0_A0_N1_D1_S0_LX_V0, &LightK3d_Spot_Q1_F0_A0_N1_D1_S0_LX_V0, &LightK3d_Spot_Q0_F0_A1_N1_D1_S0_LX_V0, &LightK3d_Spot_Q1_F0_A1_N1_D1_S0_LX_V0,	// diffuse
	// specular = 1, local viewer = 1, color vertex = 0, falloff = 0
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Spot_Q0_F0_A0_N1_D1_S1_L1_V0, &LightK3d_Spot_Q1_F0_A0_N1_D1_S1_L1_V0, &LightK3d_Spot_Q0_F0_A1_N1_D0_S1_L1_V0, &LightK3d_Spot_Q1_F0_A1_N1_D0_S1_L1_V0,	// normals, specular only
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Spot_Q0_F0_A0_N1_D1_S1_L1_V0, &LightK3d_Spot_Q1_F0_A0_N1_D1_S1_L1_V0, &LightK3d_Spot_Q0_F0_A1_N1_D1_S1_L1_V0, &LightK3d_Spot_Q1_F0_A1_N1_D1_S1_L1_V0,	// diffuse + specular
	// specular = 0, local viewer = 0, color vertex = 1, falloff = 0
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1,	// no normals
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1,	// normals but no colors
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Spot_Q0_F0_A0_N1_D1_S0_LX_V1, &LightK3d_Spot_Q1_F0_A0_N1_D1_S0_LX_V1, &LightK3d_Spot_Q0_F0_A1_N1_D1_S0_LX_V1, &LightK3d_Spot_Q1_F0_A1_N1_D1_S0_LX_V1,	// diffuse
	// specular = 1, local viewer = 0, color vertex = 1, falloff = 0
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Spot_Q0_F0_A0_N1_D1_S1_L0_V1, &LightK3d_Spot_Q1_F0_A0_N1_D1_S1_L0_V1, &LightK3d_Spot_Q0_F0_A1_N1_D0_S1_L0_V1, &LightK3d_Spot_Q1_F0_A1_N1_D0_S1_L0_V1,	// normals, specular only
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Spot_Q0_F0_A0_N1_D1_S1_L0_V1, &LightK3d_Spot_Q1_F0_A0_N1_D1_S1_L0_V1, &LightK3d_Spot_Q0_F0_A1_N1_D1_S1_L0_V1, &LightK3d_Spot_Q1_F0_A1_N1_D1_S1_L0_V1,	// diffuse + specular
	// specular = 0, local viewer = 1, color vertex = 1, falloff = 0
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1,	// no normals
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1,	// normals but no colors
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Spot_Q0_F0_A0_N1_D1_S0_LX_V1, &LightK3d_Spot_Q1_F0_A0_N1_D1_S0_LX_V1, &LightK3d_Spot_Q0_F0_A1_N1_D1_S0_LX_V1, &LightK3d_Spot_Q1_F0_A1_N1_D1_S0_LX_V1,	// diffuse
	// specular = 1, local viewer = 1, color vertex = 1, falloff = 0
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Spot_Q0_F0_A0_N1_D1_S1_L1_V1, &LightK3d_Spot_Q1_F0_A0_N1_D1_S1_L1_V1, &LightK3d_Spot_Q0_F0_A1_N1_D0_S1_L1_V1, &LightK3d_Spot_Q1_F0_A1_N1_D0_S1_L1_V1,	// normals, specular only
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F0_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F0_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Spot_Q0_F0_A0_N1_D1_S1_L1_V1, &LightK3d_Spot_Q1_F0_A0_N1_D1_S1_L1_V1, &LightK3d_Spot_Q0_F0_A1_N1_D1_S1_L1_V1, &LightK3d_Spot_Q1_F0_A1_N1_D1_S1_L1_V1,	// diffuse + specular
	// specular = 0, local viewer = 0, color vertex = 0, falloff = 1
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0,	// no normals
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0,	// normals but no colors
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Spot_Q0_F1_A0_N1_D1_S0_LX_V0, &LightK3d_Spot_Q1_F1_A0_N1_D1_S0_LX_V0, &LightK3d_Spot_Q0_F1_A1_N1_D1_S0_LX_V0, &LightK3d_Spot_Q1_F1_A1_N1_D1_S0_LX_V0,	// diffuse
	// specular = 1, local viewer = 0, color vertex = 0, falloff = 1 
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Spot_Q0_F1_A0_N1_D1_S1_L0_V0, &LightK3d_Spot_Q1_F1_A0_N1_D1_S1_L0_V0, &LightK3d_Spot_Q0_F1_A1_N1_D0_S1_L0_V0, &LightK3d_Spot_Q1_F1_A1_N1_D0_S1_L0_V0,	// normals, specular only
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Spot_Q0_F1_A0_N1_D1_S1_L0_V0, &LightK3d_Spot_Q1_F1_A0_N1_D1_S1_L0_V0, &LightK3d_Spot_Q0_F1_A1_N1_D1_S1_L0_V0, &LightK3d_Spot_Q1_F1_A1_N1_D1_S1_L0_V0,	// diffuse + specular
	// specular = 0, local viewer = 1, color vertex = 0, falloff = 1
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0,	// no normals
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0,	// normals but no colors
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Spot_Q0_F1_A0_N1_D1_S0_LX_V0, &LightK3d_Spot_Q1_F1_A0_N1_D1_S0_LX_V0, &LightK3d_Spot_Q0_F1_A1_N1_D1_S0_LX_V0, &LightK3d_Spot_Q1_F1_A1_N1_D1_S0_LX_V0,	// diffuse
	// specular = 1, local viewer = 1, color vertex = 0, falloff = 1
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Spot_Q0_F1_A0_N1_D1_S1_L1_V0, &LightK3d_Spot_Q1_F1_A0_N1_D1_S1_L1_V0, &LightK3d_Spot_Q0_F1_A1_N1_D0_S1_L1_V0, &LightK3d_Spot_Q1_F1_A1_N1_D0_S1_L1_V0,	// normals, specular only
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V0, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V0,	// no normals
	&LightK3d_Spot_Q0_F1_A0_N1_D1_S1_L1_V0, &LightK3d_Spot_Q1_F1_A0_N1_D1_S1_L1_V0, &LightK3d_Spot_Q0_F1_A1_N1_D1_S1_L1_V0, &LightK3d_Spot_Q1_F1_A1_N1_D1_S1_L1_V0,	// diffuse + specular
	// specular = 0, local viewer = 0, color vertex = 1, falloff = 1
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1,	// no normals
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1,	// normals but no colors
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Spot_Q0_F1_A0_N1_D1_S0_LX_V1, &LightK3d_Spot_Q1_F1_A0_N1_D1_S0_LX_V1, &LightK3d_Spot_Q0_F1_A1_N1_D1_S0_LX_V1, &LightK3d_Spot_Q1_F1_A1_N1_D1_S0_LX_V1,	// diffuse
	// specular = 1, local viewer = 0, color vertex = 1, falloff = 1 
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Spot_Q0_F1_A0_N1_D1_S1_L0_V1, &LightK3d_Spot_Q1_F1_A0_N1_D1_S1_L0_V1, &LightK3d_Spot_Q0_F1_A1_N1_D0_S1_L0_V1, &LightK3d_Spot_Q1_F1_A1_N1_D0_S1_L0_V1,	// normals, specular only
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Spot_Q0_F1_A0_N1_D1_S1_L0_V1, &LightK3d_Spot_Q1_F1_A0_N1_D1_S1_L0_V1, &LightK3d_Spot_Q0_F1_A1_N1_D1_S1_L0_V1, &LightK3d_Spot_Q1_F1_A1_N1_D1_S1_L0_V1,	// diffuse + specular
	// specular = 0, local viewer = 1, color vertex = 1, falloff = 1
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1,	// no normals
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1,	// normals but no colors
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Spot_Q0_F1_A0_N1_D1_S0_LX_V1, &LightK3d_Spot_Q1_F1_A0_N1_D1_S0_LX_V1, &LightK3d_Spot_Q0_F1_A1_N1_D1_S0_LX_V1, &LightK3d_Spot_Q1_F1_A1_N1_D1_S0_LX_V1,	// diffuse
	// specular = 1, local viewer = 1, color vertex = 1, falloff = 1
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Spot_Q0_F1_A0_N1_D1_S1_L1_V1, &LightK3d_Spot_Q1_F1_A0_N1_D1_S1_L1_V1, &LightK3d_Spot_Q0_F1_A1_N1_D0_S1_L1_V1, &LightK3d_Spot_Q1_F1_A1_N1_D0_S1_L1_V1,	// normals, specular only
	&Null_K3d_Lite,                         &Null_K3d_Lite,                         &LightK3d_Spot_Q0_F1_A1_N0_DX_SX_LX_V1, &LightK3d_Spot_Q1_F1_A1_N0_DX_SX_LX_V1,	// no normals
	&LightK3d_Spot_Q0_F1_A0_N1_D1_S1_L1_V1, &LightK3d_Spot_Q1_F1_A0_N1_D1_S1_L1_V1, &LightK3d_Spot_Q0_F1_A1_N1_D1_S1_L1_V1, &LightK3d_Spot_Q1_F1_A1_N1_D1_S1_L1_V1,	// diffuse + specular
	};


#endif	// TnL_HAL
#endif	// DX >= 7
