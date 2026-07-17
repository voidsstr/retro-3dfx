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
** File name: soalitfn.c
**
** Description: Lighting Functions for SOA -- generated code
**
** $Revision: 9$
** $Date: 10/11/00 8:50:22 PM$
**
** $Log: 
**  9    3dfx      1.3.1.1.1.2 10/11/00 Brent           Forced check in to enforce
**       branching.
**  8    3dfx      1.3.1.1.1.1 08/29/00 Allen Hansen    got rid of light functions
**       when the results were zero (replaced with Null_Lite())
**  7    3dfx      1.3.1.1.1.0 08/21/00 Allen Hansen    changed compiler check from
**       SSECPP to VCPP 
**  6    3dfx      1.3.1.1     05/16/00 Bob Johnston    Consolodating VERT_BUFF and
**       TnL_HAL defines to just use TnL_HAL
**  5    3dfx      1.3.1.0     05/11/00 Scott Kephart   Complete re-write --
**       lighting broken out in to many, many cases. Do not edit this code -- it's
**       machine generated!
**  4    Napalm    1.3         04/19/00 Scott Kephart   Begining of optimized spot
**       lights
**  3    Napalm    1.2         04/19/00 Scott Kephart   Big lighting change - Part
**       I
**       Lighting is now split into two parts, diffuse and specular. 
**  2    Napalm    1.1         03/17/00 Scott Kephart   Added support for Visual
**       C++ processor pack Beta
**  1    Napalm    1.0         03/08/00 Scott Kephart   
** $
*/

#include "precomp.h"

#if( DX >= 7 )
#ifdef TnL_HAL
#if defined(VCPP)

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

ALIGN32 void Null_Lite(RC *pRc, TLLIGHT *pL) {};

//***********************************************
// Directional Cases
//***********************************************

//-----------------------------------------------
// Directional Diffuse
//-----------------------------------------------
//Case # 0
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Normal 0, Diffuse(R = 0, G = 0, B = 0
//Null_Lite


//Case # 0
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Normal 0, Diffuse(R = 0, G = 0, B = 0
#define NAME DIR_DIFF_AR1_AG0_AB0_N0_R0_G0_B0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define NORMALS 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 1
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Normal 0, Diffuse(R = 0, G = 0, B = 0
#define NAME DIR_DIFF_AR0_AG1_AB0_N0_R0_G0_B0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define NORMALS 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 2
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Normal 0, Diffuse(R = 0, G = 0, B = 0
#define NAME DIR_DIFF_AR1_AG1_AB0_N0_R0_G0_B0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define NORMALS 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 3
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Normal 0, Diffuse(R = 0, G = 0, B = 0
#define NAME DIR_DIFF_AR0_AG0_AB1_N0_R0_G0_B0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define NORMALS 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 4
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Normal 0, Diffuse(R = 0, G = 0, B = 0
#define NAME DIR_DIFF_AR1_AG0_AB1_N0_R0_G0_B0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define NORMALS 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 5
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Normal 0, Diffuse(R = 0, G = 0, B = 0
#define NAME DIR_DIFF_AR0_AG1_AB1_N0_R0_G0_B0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define NORMALS 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 6
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Normal 0, Diffuse(R = 0, G = 0, B = 0
#define NAME DIR_DIFF_AR1_AG1_AB1_N0_R0_G0_B0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define NORMALS 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 7
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Normal 1, Diffuse(R = 0, G = 0, B = 0
#define NAME DIR_DIFF_AR0_AG0_AB0_N1_R0_G0_B0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define NORMALS 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 8
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Normal 1, Diffuse(R = 0, G = 0, B = 0
#define NAME DIR_DIFF_AR1_AG0_AB0_N1_R0_G0_B0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define NORMALS 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 9
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Normal 1, Diffuse(R = 0, G = 0, B = 0
#define NAME DIR_DIFF_AR0_AG1_AB0_N1_R0_G0_B0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define NORMALS 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 10
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Normal 1, Diffuse(R = 0, G = 0, B = 0
#define NAME DIR_DIFF_AR1_AG1_AB0_N1_R0_G0_B0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define NORMALS 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 11
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Normal 1, Diffuse(R = 0, G = 0, B = 0
#define NAME DIR_DIFF_AR0_AG0_AB1_N1_R0_G0_B0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define NORMALS 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 12
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Normal 1, Diffuse(R = 0, G = 0, B = 0
#define NAME DIR_DIFF_AR1_AG0_AB1_N1_R0_G0_B0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define NORMALS 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 13
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Normal 1, Diffuse(R = 0, G = 0, B = 0
#define NAME DIR_DIFF_AR0_AG1_AB1_N1_R0_G0_B0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define NORMALS 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 14
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Normal 1, Diffuse(R = 0, G = 0, B = 0
#define NAME DIR_DIFF_AR1_AG1_AB1_N1_R0_G0_B0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define NORMALS 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 15
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Normal 1, Diffuse(R = 1, G = 0, B = 0
#define NAME DIR_DIFF_AR0_AG0_AB0_N1_R1_G0_B0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define NORMALS 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 16
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Normal 1, Diffuse(R = 1, G = 0, B = 0
#define NAME DIR_DIFF_AR1_AG0_AB0_N1_R1_G0_B0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define NORMALS 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 17
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Normal 1, Diffuse(R = 1, G = 0, B = 0
#define NAME DIR_DIFF_AR0_AG1_AB0_N1_R1_G0_B0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define NORMALS 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 18
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Normal 1, Diffuse(R = 1, G = 0, B = 0
#define NAME DIR_DIFF_AR1_AG1_AB0_N1_R1_G0_B0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define NORMALS 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 19
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Normal 1, Diffuse(R = 1, G = 0, B = 0
#define NAME DIR_DIFF_AR0_AG0_AB1_N1_R1_G0_B0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define NORMALS 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 20
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Normal 1, Diffuse(R = 1, G = 0, B = 0
#define NAME DIR_DIFF_AR1_AG0_AB1_N1_R1_G0_B0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define NORMALS 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 21
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Normal 1, Diffuse(R = 1, G = 0, B = 0
#define NAME DIR_DIFF_AR0_AG1_AB1_N1_R1_G0_B0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define NORMALS 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 22
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Normal 1, Diffuse(R = 1, G = 0, B = 0
#define NAME DIR_DIFF_AR1_AG1_AB1_N1_R1_G0_B0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define NORMALS 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#include "soadd.h"


//Case # 23
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Normal 1, Diffuse(R = 0, G = 1, B = 0
#define NAME DIR_DIFF_AR0_AG0_AB0_N1_R0_G1_B0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define NORMALS 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#include "soadd.h"


//Case # 24
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Normal 1, Diffuse(R = 0, G = 1, B = 0
#define NAME DIR_DIFF_AR1_AG0_AB0_N1_R0_G1_B0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define NORMALS 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#include "soadd.h"


//Case # 25
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Normal 1, Diffuse(R = 0, G = 1, B = 0
#define NAME DIR_DIFF_AR0_AG1_AB0_N1_R0_G1_B0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define NORMALS 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#include "soadd.h"


//Case # 26
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Normal 1, Diffuse(R = 0, G = 1, B = 0
#define NAME DIR_DIFF_AR1_AG1_AB0_N1_R0_G1_B0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define NORMALS 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#include "soadd.h"


//Case # 27
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Normal 1, Diffuse(R = 0, G = 1, B = 0
#define NAME DIR_DIFF_AR0_AG0_AB1_N1_R0_G1_B0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define NORMALS 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#include "soadd.h"


//Case # 28
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Normal 1, Diffuse(R = 0, G = 1, B = 0
#define NAME DIR_DIFF_AR1_AG0_AB1_N1_R0_G1_B0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define NORMALS 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#include "soadd.h"


//Case # 29
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Normal 1, Diffuse(R = 0, G = 1, B = 0
#define NAME DIR_DIFF_AR0_AG1_AB1_N1_R0_G1_B0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define NORMALS 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#include "soadd.h"


//Case # 30
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Normal 1, Diffuse(R = 0, G = 1, B = 0
#define NAME DIR_DIFF_AR1_AG1_AB1_N1_R0_G1_B0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define NORMALS 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#include "soadd.h"


//Case # 31
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Normal 1, Diffuse(R = 1, G = 1, B = 0
#define NAME DIR_DIFF_AR0_AG0_AB0_N1_R1_G1_B0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define NORMALS 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#include "soadd.h"


//Case # 32
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Normal 1, Diffuse(R = 1, G = 1, B = 0
#define NAME DIR_DIFF_AR1_AG0_AB0_N1_R1_G1_B0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define NORMALS 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#include "soadd.h"


//Case # 33
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Normal 1, Diffuse(R = 1, G = 1, B = 0
#define NAME DIR_DIFF_AR0_AG1_AB0_N1_R1_G1_B0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define NORMALS 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#include "soadd.h"


//Case # 34
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Normal 1, Diffuse(R = 1, G = 1, B = 0
#define NAME DIR_DIFF_AR1_AG1_AB0_N1_R1_G1_B0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define NORMALS 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#include "soadd.h"


//Case # 35
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Normal 1, Diffuse(R = 1, G = 1, B = 0
#define NAME DIR_DIFF_AR0_AG0_AB1_N1_R1_G1_B0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define NORMALS 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#include "soadd.h"


//Case # 36
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Normal 1, Diffuse(R = 1, G = 1, B = 0
#define NAME DIR_DIFF_AR1_AG0_AB1_N1_R1_G1_B0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define NORMALS 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#include "soadd.h"


//Case # 37
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Normal 1, Diffuse(R = 1, G = 1, B = 0
#define NAME DIR_DIFF_AR0_AG1_AB1_N1_R1_G1_B0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define NORMALS 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#include "soadd.h"


//Case # 38
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Normal 1, Diffuse(R = 1, G = 1, B = 0
#define NAME DIR_DIFF_AR1_AG1_AB1_N1_R1_G1_B0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define NORMALS 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#include "soadd.h"


//Case # 39
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Normal 1, Diffuse(R = 0, G = 0, B = 1
#define NAME DIR_DIFF_AR0_AG0_AB0_N1_R0_G0_B1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define NORMALS 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#include "soadd.h"


//Case # 40
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Normal 1, Diffuse(R = 0, G = 0, B = 1
#define NAME DIR_DIFF_AR1_AG0_AB0_N1_R0_G0_B1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define NORMALS 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#include "soadd.h"


//Case # 41
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Normal 1, Diffuse(R = 0, G = 0, B = 1
#define NAME DIR_DIFF_AR0_AG1_AB0_N1_R0_G0_B1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define NORMALS 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#include "soadd.h"


//Case # 42
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Normal 1, Diffuse(R = 0, G = 0, B = 1
#define NAME DIR_DIFF_AR1_AG1_AB0_N1_R0_G0_B1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define NORMALS 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#include "soadd.h"


//Case # 43
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Normal 1, Diffuse(R = 0, G = 0, B = 1
#define NAME DIR_DIFF_AR0_AG0_AB1_N1_R0_G0_B1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define NORMALS 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#include "soadd.h"


//Case # 44
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Normal 1, Diffuse(R = 0, G = 0, B = 1
#define NAME DIR_DIFF_AR1_AG0_AB1_N1_R0_G0_B1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define NORMALS 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#include "soadd.h"


//Case # 45
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Normal 1, Diffuse(R = 0, G = 0, B = 1
#define NAME DIR_DIFF_AR0_AG1_AB1_N1_R0_G0_B1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define NORMALS 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#include "soadd.h"


//Case # 46
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Normal 1, Diffuse(R = 0, G = 0, B = 1
#define NAME DIR_DIFF_AR1_AG1_AB1_N1_R0_G0_B1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define NORMALS 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#include "soadd.h"


//Case # 47
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Normal 1, Diffuse(R = 1, G = 0, B = 1
#define NAME DIR_DIFF_AR0_AG0_AB0_N1_R1_G0_B1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define NORMALS 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#include "soadd.h"


//Case # 48
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Normal 1, Diffuse(R = 1, G = 0, B = 1
#define NAME DIR_DIFF_AR1_AG0_AB0_N1_R1_G0_B1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define NORMALS 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#include "soadd.h"


//Case # 49
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Normal 1, Diffuse(R = 1, G = 0, B = 1
#define NAME DIR_DIFF_AR0_AG1_AB0_N1_R1_G0_B1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define NORMALS 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#include "soadd.h"


//Case # 50
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Normal 1, Diffuse(R = 1, G = 0, B = 1
#define NAME DIR_DIFF_AR1_AG1_AB0_N1_R1_G0_B1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define NORMALS 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#include "soadd.h"


//Case # 51
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Normal 1, Diffuse(R = 1, G = 0, B = 1
#define NAME DIR_DIFF_AR0_AG0_AB1_N1_R1_G0_B1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define NORMALS 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#include "soadd.h"


//Case # 52
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Normal 1, Diffuse(R = 1, G = 0, B = 1
#define NAME DIR_DIFF_AR1_AG0_AB1_N1_R1_G0_B1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define NORMALS 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#include "soadd.h"


//Case # 53
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Normal 1, Diffuse(R = 1, G = 0, B = 1
#define NAME DIR_DIFF_AR0_AG1_AB1_N1_R1_G0_B1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define NORMALS 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#include "soadd.h"


//Case # 54
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Normal 1, Diffuse(R = 1, G = 0, B = 1
#define NAME DIR_DIFF_AR1_AG1_AB1_N1_R1_G0_B1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define NORMALS 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#include "soadd.h"


//Case # 55
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Normal 1, Diffuse(R = 0, G = 1, B = 1
#define NAME DIR_DIFF_AR0_AG0_AB0_N1_R0_G1_B1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define NORMALS 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#include "soadd.h"


//Case # 56
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Normal 1, Diffuse(R = 0, G = 1, B = 1
#define NAME DIR_DIFF_AR1_AG0_AB0_N1_R0_G1_B1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define NORMALS 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#include "soadd.h"


//Case # 57
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Normal 1, Diffuse(R = 0, G = 1, B = 1
#define NAME DIR_DIFF_AR0_AG1_AB0_N1_R0_G1_B1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define NORMALS 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#include "soadd.h"


//Case # 58
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Normal 1, Diffuse(R = 0, G = 1, B = 1
#define NAME DIR_DIFF_AR1_AG1_AB0_N1_R0_G1_B1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define NORMALS 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#include "soadd.h"


//Case # 59
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Normal 1, Diffuse(R = 0, G = 1, B = 1
#define NAME DIR_DIFF_AR0_AG0_AB1_N1_R0_G1_B1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define NORMALS 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#include "soadd.h"


//Case # 60
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Normal 1, Diffuse(R = 0, G = 1, B = 1
#define NAME DIR_DIFF_AR1_AG0_AB1_N1_R0_G1_B1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define NORMALS 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#include "soadd.h"


//Case # 61
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Normal 1, Diffuse(R = 0, G = 1, B = 1
#define NAME DIR_DIFF_AR0_AG1_AB1_N1_R0_G1_B1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define NORMALS 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#include "soadd.h"


//Case # 62
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Normal 1, Diffuse(R = 0, G = 1, B = 1
#define NAME DIR_DIFF_AR1_AG1_AB1_N1_R0_G1_B1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define NORMALS 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#include "soadd.h"


//Case # 63
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Normal 1, Diffuse(R = 1, G = 1, B = 1
#define NAME DIR_DIFF_AR0_AG0_AB0_N1_R1_G1_B1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define NORMALS 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#include "soadd.h"


//Case # 64
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Normal 1, Diffuse(R = 1, G = 1, B = 1
#define NAME DIR_DIFF_AR1_AG0_AB0_N1_R1_G1_B1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define NORMALS 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#include "soadd.h"


//Case # 65
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Normal 1, Diffuse(R = 1, G = 1, B = 1
#define NAME DIR_DIFF_AR0_AG1_AB0_N1_R1_G1_B1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define NORMALS 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#include "soadd.h"


//Case # 66
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Normal 1, Diffuse(R = 1, G = 1, B = 1
#define NAME DIR_DIFF_AR1_AG1_AB0_N1_R1_G1_B1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define NORMALS 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#include "soadd.h"


//Case # 67
//Directional Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Normal 1, Diffuse(R = 1, G = 1, B = 1
#define NAME DIR_DIFF_AR0_AG0_AB1_N1_R1_G1_B1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define NORMALS 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#include "soadd.h"


//Case # 68
//Directional Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Normal 1, Diffuse(R = 1, G = 1, B = 1
#define NAME DIR_DIFF_AR1_AG0_AB1_N1_R1_G1_B1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define NORMALS 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#include "soadd.h"


//Case # 69
//Directional Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Normal 1, Diffuse(R = 1, G = 1, B = 1
#define NAME DIR_DIFF_AR0_AG1_AB1_N1_R1_G1_B1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define NORMALS 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#include "soadd.h"


//Case # 70
//Directional Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Normal 1, Diffuse(R = 1, G = 1, B = 1
#define NAME DIR_DIFF_AR1_AG1_AB1_N1_R1_G1_B1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define NORMALS 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#include "soadd.h"




//-----------------------------------------------
// Directional Specular
//-----------------------------------------------
//Case # 0
//Directional Light, Specular Component (R = 0, G = 0, B = 0), Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
//Null_Lite


//Case # 0
//Directional Light, Specular Component (R = 1, G = 0, B = 0), Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME DIR_SPEC_R1_G0_B0_L0_E00_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soads.h"


//Case # 1
//Directional Light, Specular Component (R = 0, G = 1, B = 0), Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME DIR_SPEC_R0_G1_B0_L0_E00_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soads.h"


//Case # 2
//Directional Light, Specular Component (R = 1, G = 1, B = 0), Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME DIR_SPEC_R1_G1_B0_L0_E00_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soads.h"


//Case # 3
//Directional Light, Specular Component (R = 0, G = 0, B = 1), Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME DIR_SPEC_R0_G0_B1_L0_E00_EB0
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soads.h"


//Case # 4
//Directional Light, Specular Component (R = 1, G = 0, B = 1), Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME DIR_SPEC_R1_G0_B1_L0_E00_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soads.h"


//Case # 5
//Directional Light, Specular Component (R = 0, G = 1, B = 1), Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME DIR_SPEC_R0_G1_B1_L0_E00_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soads.h"


//Case # 6
//Directional Light, Specular Component (R = 1, G = 1, B = 1), Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME DIR_SPEC_R1_G1_B1_L0_E00_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soads.h"


//Case # 
//Directional Light, Specular Component (R = 0, G = 0, B = 0), Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
//Null_Lite


//Case # 7
//Directional Light, Specular Component (R = 1, G = 0, B = 0), Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME DIR_SPEC_R1_G0_B0_L1_E00_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soads.h"


//Case # 8
//Directional Light, Specular Component (R = 0, G = 1, B = 0), Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME DIR_SPEC_R0_G1_B0_L1_E00_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soads.h"


//Case # 9
//Directional Light, Specular Component (R = 1, G = 1, B = 0), Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME DIR_SPEC_R1_G1_B0_L1_E00_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soads.h"


//Case # 10
//Directional Light, Specular Component (R = 0, G = 0, B = 1), Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME DIR_SPEC_R0_G0_B1_L1_E00_EB0
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soads.h"


//Case # 11
//Directional Light, Specular Component (R = 1, G = 0, B = 1), Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME DIR_SPEC_R1_G0_B1_L1_E00_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soads.h"


//Case # 12
//Directional Light, Specular Component (R = 0, G = 1, B = 1), Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME DIR_SPEC_R0_G1_B1_L1_E00_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soads.h"


//Case # 13
//Directional Light, Specular Component (R = 1, G = 1, B = 1), Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME DIR_SPEC_R1_G1_B1_L1_E00_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soads.h"


//Case # 
//Directional Light, Specular Component (R = 0, G = 0, B = 0), Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
//Null_Lite


//Case # 14
//Directional Light, Specular Component (R = 1, G = 0, B = 0), Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME DIR_SPEC_R1_G0_B0_L0_E01_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soads.h"


//Case # 15
//Directional Light, Specular Component (R = 0, G = 1, B = 0), Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME DIR_SPEC_R0_G1_B0_L0_E01_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soads.h"


//Case # 16
//Directional Light, Specular Component (R = 1, G = 1, B = 0), Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME DIR_SPEC_R1_G1_B0_L0_E01_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soads.h"


//Case # 17
//Directional Light, Specular Component (R = 0, G = 0, B = 1), Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME DIR_SPEC_R0_G0_B1_L0_E01_EB0
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soads.h"


//Case # 18
//Directional Light, Specular Component (R = 1, G = 0, B = 1), Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME DIR_SPEC_R1_G0_B1_L0_E01_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soads.h"


//Case # 19
//Directional Light, Specular Component (R = 0, G = 1, B = 1), Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME DIR_SPEC_R0_G1_B1_L0_E01_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soads.h"


//Case # 20
//Directional Light, Specular Component (R = 1, G = 1, B = 1), Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME DIR_SPEC_R1_G1_B1_L0_E01_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soads.h"


//Case # 
//Directional Light, Specular Component (R = 0, G = 0, B = 0), Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
//Null_Lite


//Case # 21
//Directional Light, Specular Component (R = 1, G = 0, B = 0), Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME DIR_SPEC_R1_G0_B0_L1_E01_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soads.h"


//Case # 22
//Directional Light, Specular Component (R = 0, G = 1, B = 0), Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME DIR_SPEC_R0_G1_B0_L1_E01_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soads.h"


//Case # 23
//Directional Light, Specular Component (R = 1, G = 1, B = 0), Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME DIR_SPEC_R1_G1_B0_L1_E01_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soads.h"


//Case # 24
//Directional Light, Specular Component (R = 0, G = 0, B = 1), Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME DIR_SPEC_R0_G0_B1_L1_E01_EB0
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soads.h"


//Case # 25
//Directional Light, Specular Component (R = 1, G = 0, B = 1), Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME DIR_SPEC_R1_G0_B1_L1_E01_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soads.h"


//Case # 26
//Directional Light, Specular Component (R = 0, G = 1, B = 1), Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME DIR_SPEC_R0_G1_B1_L1_E01_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soads.h"


//Case # 27
//Directional Light, Specular Component (R = 1, G = 1, B = 1), Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME DIR_SPEC_R1_G1_B1_L1_E01_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soads.h"


//Case # 
//Directional Light, Specular Component (R = 0, G = 0, B = 0), Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
//Null_Lite


//Case # 28
//Directional Light, Specular Component (R = 1, G = 0, B = 0), Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME DIR_SPEC_R1_G0_B0_L0_E00_EB1
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soads.h"


//Case # 29
//Directional Light, Specular Component (R = 0, G = 1, B = 0), Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME DIR_SPEC_R0_G1_B0_L0_E00_EB1
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soads.h"


//Case # 30
//Directional Light, Specular Component (R = 1, G = 1, B = 0), Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME DIR_SPEC_R1_G1_B0_L0_E00_EB1
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soads.h"


//Case # 31
//Directional Light, Specular Component (R = 0, G = 0, B = 1), Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME DIR_SPEC_R0_G0_B1_L0_E00_EB1
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soads.h"


//Case # 32
//Directional Light, Specular Component (R = 1, G = 0, B = 1), Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME DIR_SPEC_R1_G0_B1_L0_E00_EB1
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soads.h"


//Case # 33
//Directional Light, Specular Component (R = 0, G = 1, B = 1), Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME DIR_SPEC_R0_G1_B1_L0_E00_EB1
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soads.h"


//Case # 34
//Directional Light, Specular Component (R = 1, G = 1, B = 1), Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME DIR_SPEC_R1_G1_B1_L0_E00_EB1
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soads.h"


//Case # 
//Directional Light, Specular Component (R = 0, G = 0, B = 0), Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
//Null_Lite


//Case # 35
//Directional Light, Specular Component (R = 1, G = 0, B = 0), Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME DIR_SPEC_R1_G0_B0_L1_E00_EB1
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soads.h"


//Case # 36
//Directional Light, Specular Component (R = 0, G = 1, B = 0), Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME DIR_SPEC_R0_G1_B0_L1_E00_EB1
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soads.h"


//Case # 37
//Directional Light, Specular Component (R = 1, G = 1, B = 0), Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME DIR_SPEC_R1_G1_B0_L1_E00_EB1
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soads.h"


//Case # 38
//Directional Light, Specular Component (R = 0, G = 0, B = 1), Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME DIR_SPEC_R0_G0_B1_L1_E00_EB1
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soads.h"


//Case # 39
//Directional Light, Specular Component (R = 1, G = 0, B = 1), Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME DIR_SPEC_R1_G0_B1_L1_E00_EB1
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soads.h"


//Case # 40
//Directional Light, Specular Component (R = 0, G = 1, B = 1), Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME DIR_SPEC_R0_G1_B1_L1_E00_EB1
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soads.h"


//Case # 41
//Directional Light, Specular Component (R = 1, G = 1, B = 1), Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME DIR_SPEC_R1_G1_B1_L1_E00_EB1
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soads.h"



//***********************************************
// Point Cases
//***********************************************

//-----------------------------------------------
// Point Diffuse
//-----------------------------------------------

//Case # 0
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 0
//Null_Lite


//Case # 0
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB0_R0_G0_B0_A0_N0
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 1
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB0_R0_G0_B0_A0_N0
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 2
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB0_R0_G0_B0_A0_N0
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 3
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB1_R0_G0_B0_A0_N0
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 4
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB1_R0_G0_B0_A0_N0
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 5
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB1_R0_G0_B0_A0_N0
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 6
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB1_R0_G0_B0_A0_N0
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 7
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB0_R0_G0_B0_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 8
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB0_R0_G0_B0_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 9
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB0_R0_G0_B0_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 10
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB0_R0_G0_B0_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 11
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB1_R0_G0_B0_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 12
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB1_R0_G0_B0_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 13
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB1_R0_G0_B0_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 14
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB1_R0_G0_B0_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 15
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 1, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB0_R1_G0_B0_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 16
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 1, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB0_R1_G0_B0_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 17
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 1, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB0_R1_G0_B0_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 18
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 1, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB0_R1_G0_B0_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 19
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 1, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB1_R1_G0_B0_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 20
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 1, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB1_R1_G0_B0_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 21
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 1, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB1_R1_G0_B0_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 22
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 1, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB1_R1_G0_B0_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 23
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 1, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB0_R0_G1_B0_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 24
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 1, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB0_R0_G1_B0_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 25
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 1, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB0_R0_G1_B0_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 26
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 1, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB0_R0_G1_B0_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 27
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 1, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB1_R0_G1_B0_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 28
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 1, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB1_R0_G1_B0_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 29
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 1, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB1_R0_G1_B0_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 30
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 1, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB1_R0_G1_B0_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 31
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 1, G = 1, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB0_R1_G1_B0_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 32
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 1, G = 1, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB0_R1_G1_B0_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 33
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 1, G = 1, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB0_R1_G1_B0_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 34
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 1, G = 1, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB0_R1_G1_B0_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 35
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 1, G = 1, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB1_R1_G1_B0_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 36
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 1, G = 1, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB1_R1_G1_B0_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 37
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 1, G = 1, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB1_R1_G1_B0_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 38
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 1, G = 1, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB1_R1_G1_B0_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 39
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 1
#define NAME POINT_DIFF_AR0_AG0_AB0_R0_G0_B1_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 40
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 1
#define NAME POINT_DIFF_AR1_AG0_AB0_R0_G0_B1_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 41
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 1
#define NAME POINT_DIFF_AR0_AG1_AB0_R0_G0_B1_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 42
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 1
#define NAME POINT_DIFF_AR1_AG1_AB0_R0_G0_B1_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 43
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 1
#define NAME POINT_DIFF_AR0_AG0_AB1_R0_G0_B1_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 44
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 1
#define NAME POINT_DIFF_AR1_AG0_AB1_R0_G0_B1_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 45
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 1
#define NAME POINT_DIFF_AR0_AG1_AB1_R0_G0_B1_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 46
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 1
#define NAME POINT_DIFF_AR1_AG1_AB1_R0_G0_B1_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 47
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 1, G = 0, B = 1
#define NAME POINT_DIFF_AR0_AG0_AB0_R1_G0_B1_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 48
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 1, G = 0, B = 1
#define NAME POINT_DIFF_AR1_AG0_AB0_R1_G0_B1_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 49
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 1, G = 0, B = 1
#define NAME POINT_DIFF_AR0_AG1_AB0_R1_G0_B1_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 50
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 1, G = 0, B = 1
#define NAME POINT_DIFF_AR1_AG1_AB0_R1_G0_B1_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 51
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 1, G = 0, B = 1
#define NAME POINT_DIFF_AR0_AG0_AB1_R1_G0_B1_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 52
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 1, G = 0, B = 1
#define NAME POINT_DIFF_AR1_AG0_AB1_R1_G0_B1_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 53
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 1, G = 0, B = 1
#define NAME POINT_DIFF_AR0_AG1_AB1_R1_G0_B1_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 54
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 1, G = 0, B = 1
#define NAME POINT_DIFF_AR1_AG1_AB1_R1_G0_B1_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 55
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 1, B = 1
#define NAME POINT_DIFF_AR0_AG0_AB0_R0_G1_B1_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 56
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 1, B = 1
#define NAME POINT_DIFF_AR1_AG0_AB0_R0_G1_B1_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 57
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 1, B = 1
#define NAME POINT_DIFF_AR0_AG1_AB0_R0_G1_B1_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 58
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 1, B = 1
#define NAME POINT_DIFF_AR1_AG1_AB0_R0_G1_B1_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 59
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 1, B = 1
#define NAME POINT_DIFF_AR0_AG0_AB1_R0_G1_B1_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 60
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 1, B = 1
#define NAME POINT_DIFF_AR1_AG0_AB1_R0_G1_B1_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 61
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 1, B = 1
#define NAME POINT_DIFF_AR0_AG1_AB1_R0_G1_B1_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 62
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 1, B = 1
#define NAME POINT_DIFF_AR1_AG1_AB1_R0_G1_B1_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 63
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 1, G = 1, B = 1
#define NAME POINT_DIFF_AR0_AG0_AB0_R1_G1_B1_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 64
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 1, G = 1, B = 1
#define NAME POINT_DIFF_AR1_AG0_AB0_R1_G1_B1_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 65
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 1, G = 1, B = 1
#define NAME POINT_DIFF_AR0_AG1_AB0_R1_G1_B1_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 66
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 1, G = 1, B = 1
#define NAME POINT_DIFF_AR1_AG1_AB0_R1_G1_B1_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 67
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 1, G = 1, B = 1
#define NAME POINT_DIFF_AR0_AG0_AB1_R1_G1_B1_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 68
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 1, G = 1, B = 1
#define NAME POINT_DIFF_AR1_AG0_AB1_R1_G1_B1_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 69
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 1, G = 1, B = 1
#define NAME POINT_DIFF_AR0_AG1_AB1_R1_G1_B1_A0_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 70
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 1, G = 1, B = 1
#define NAME POINT_DIFF_AR1_AG1_AB1_R1_G1_B1_A0_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 71
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB0_R0_G0_B0_A1_N0
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 72
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB0_R0_G0_B0_A1_N0
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 73
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB0_R0_G0_B0_A1_N0
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 74
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB0_R0_G0_B0_A1_N0
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 75
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB1_R0_G0_B0_A1_N0
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 76
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB1_R0_G0_B0_A1_N0
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 77
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB1_R0_G0_B0_A1_N0
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 78
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB1_R0_G0_B0_A1_N0
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 79
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB0_R0_G0_B0_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 80
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB0_R0_G0_B0_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 81
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB0_R0_G0_B0_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 82
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB0_R0_G0_B0_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 83
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB1_R0_G0_B0_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 84
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB1_R0_G0_B0_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 85
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB1_R0_G0_B0_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 86
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB1_R0_G0_B0_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 87
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 1, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB0_R1_G0_B0_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 88
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 1, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB0_R1_G0_B0_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 89
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 1, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB0_R1_G0_B0_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 90
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 1, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB0_R1_G0_B0_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 91
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 1, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB1_R1_G0_B0_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 92
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 1, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB1_R1_G0_B0_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 93
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 1, G = 0, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB1_R1_G0_B0_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 94
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 1, G = 0, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB1_R1_G0_B0_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 95
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 1, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB0_R0_G1_B0_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 96
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 1, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB0_R0_G1_B0_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 97
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 1, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB0_R0_G1_B0_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 98
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 1, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB0_R0_G1_B0_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 99
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 1, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB1_R0_G1_B0_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 100
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 1, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB1_R0_G1_B0_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 101
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 1, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB1_R0_G1_B0_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 102
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 1, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB1_R0_G1_B0_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 103
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 1, G = 1, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB0_R1_G1_B0_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 104
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 1, G = 1, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB0_R1_G1_B0_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 105
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 1, G = 1, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB0_R1_G1_B0_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 106
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 1, G = 1, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB0_R1_G1_B0_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 107
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 1, G = 1, B = 0
#define NAME POINT_DIFF_AR0_AG0_AB1_R1_G1_B0_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 108
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 1, G = 1, B = 0
#define NAME POINT_DIFF_AR1_AG0_AB1_R1_G1_B0_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 109
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 1, G = 1, B = 0
#define NAME POINT_DIFF_AR0_AG1_AB1_R1_G1_B0_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 110
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 1, G = 1, B = 0
#define NAME POINT_DIFF_AR1_AG1_AB1_R1_G1_B0_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 111
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 1
#define NAME POINT_DIFF_AR0_AG0_AB0_R0_G0_B1_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 112
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 1
#define NAME POINT_DIFF_AR1_AG0_AB0_R0_G0_B1_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 113
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 1
#define NAME POINT_DIFF_AR0_AG1_AB0_R0_G0_B1_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 114
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 1
#define NAME POINT_DIFF_AR1_AG1_AB0_R0_G0_B1_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 115
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 1
#define NAME POINT_DIFF_AR0_AG0_AB1_R0_G0_B1_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 116
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 1
#define NAME POINT_DIFF_AR1_AG0_AB1_R0_G0_B1_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 117
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 1
#define NAME POINT_DIFF_AR0_AG1_AB1_R0_G0_B1_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 118
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 1
#define NAME POINT_DIFF_AR1_AG1_AB1_R0_G0_B1_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 119
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 1, G = 0, B = 1
#define NAME POINT_DIFF_AR0_AG0_AB0_R1_G0_B1_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 120
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 1, G = 0, B = 1
#define NAME POINT_DIFF_AR1_AG0_AB0_R1_G0_B1_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 121
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 1, G = 0, B = 1
#define NAME POINT_DIFF_AR0_AG1_AB0_R1_G0_B1_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 122
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 1, G = 0, B = 1
#define NAME POINT_DIFF_AR1_AG1_AB0_R1_G0_B1_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 123
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 1, G = 0, B = 1
#define NAME POINT_DIFF_AR0_AG0_AB1_R1_G0_B1_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 124
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 1, G = 0, B = 1
#define NAME POINT_DIFF_AR1_AG0_AB1_R1_G0_B1_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 125
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 1, G = 0, B = 1
#define NAME POINT_DIFF_AR0_AG1_AB1_R1_G0_B1_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 126
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 1, G = 0, B = 1
#define NAME POINT_DIFF_AR1_AG1_AB1_R1_G0_B1_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 127
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 1, B = 1
#define NAME POINT_DIFF_AR0_AG0_AB0_R0_G1_B1_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 128
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 1, B = 1
#define NAME POINT_DIFF_AR1_AG0_AB0_R0_G1_B1_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 129
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 1, B = 1
#define NAME POINT_DIFF_AR0_AG1_AB0_R0_G1_B1_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 130
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 1, B = 1
#define NAME POINT_DIFF_AR1_AG1_AB0_R0_G1_B1_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 131
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 1, B = 1
#define NAME POINT_DIFF_AR0_AG0_AB1_R0_G1_B1_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 132
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 1, B = 1
#define NAME POINT_DIFF_AR1_AG0_AB1_R0_G1_B1_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 133
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 1, B = 1
#define NAME POINT_DIFF_AR0_AG1_AB1_R0_G1_B1_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 134
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 1, B = 1
#define NAME POINT_DIFF_AR1_AG1_AB1_R0_G1_B1_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 135
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 1, G = 1, B = 1
#define NAME POINT_DIFF_AR0_AG0_AB0_R1_G1_B1_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 136
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 1, G = 1, B = 1
#define NAME POINT_DIFF_AR1_AG0_AB0_R1_G1_B1_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 137
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 1, G = 1, B = 1
#define NAME POINT_DIFF_AR0_AG1_AB0_R1_G1_B1_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 138
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 1, G = 1, B = 1
#define NAME POINT_DIFF_AR1_AG1_AB0_R1_G1_B1_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 139
//Point Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 1, G = 1, B = 1
#define NAME POINT_DIFF_AR0_AG0_AB1_R1_G1_B1_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 140
//Point Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 1, G = 1, B = 1
#define NAME POINT_DIFF_AR1_AG0_AB1_R1_G1_B1_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 141
//Point Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 1, G = 1, B = 1
#define NAME POINT_DIFF_AR0_AG1_AB1_R1_G1_B1_A1_N1
#define SPOT 0
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 142
//Point Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 1, G = 1, B = 1
#define NAME POINT_DIFF_AR1_AG1_AB1_R1_G1_B1_A1_N1
#define SPOT 0
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"



//-----------------------------------------------
// Point & Spot Specular
//-----------------------------------------------

//Case # 0
//POINT Light, Specular Component (R = 0, G = 0, B = 0), Attenuation = 0 Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
//Null_Lite


//Case # 0
//POINT Light, Specular Component (R = 1, G = 0, B = 0), Attenuation = 0 Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G0_B0_A0_L0_E00_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 1
//POINT Light, Specular Component (R = 0, G = 1, B = 0), Attenuation = 0 Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G1_B0_A0_L0_E00_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 2
//POINT Light, Specular Component (R = 1, G = 1, B = 0), Attenuation = 0 Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G1_B0_A0_L0_E00_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 3
//POINT Light, Specular Component (R = 0, G = 0, B = 1), Attenuation = 0 Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G0_B1_A0_L0_E00_EB0
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 4
//POINT Light, Specular Component (R = 1, G = 0, B = 1), Attenuation = 0 Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G0_B1_A0_L0_E00_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 5
//POINT Light, Specular Component (R = 0, G = 1, B = 1), Attenuation = 0 Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G1_B1_A0_L0_E00_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 6
//POINT Light, Specular Component (R = 1, G = 1, B = 1), Attenuation = 0 Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G1_B1_A0_L0_E00_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 
//POINT Light, Specular Component (R = 0, G = 0, B = 0), Attenuation = 1 Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
//Null_Lite


//Case # 7
//POINT Light, Specular Component (R = 1, G = 0, B = 0), Attenuation = 1 Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G0_B0_A1_L0_E00_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 8
//POINT Light, Specular Component (R = 0, G = 1, B = 0), Attenuation = 1 Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G1_B0_A1_L0_E00_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 9
//POINT Light, Specular Component (R = 1, G = 1, B = 0), Attenuation = 1 Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G1_B0_A1_L0_E00_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 10
//POINT Light, Specular Component (R = 0, G = 0, B = 1), Attenuation = 1 Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G0_B1_A1_L0_E00_EB0
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 11
//POINT Light, Specular Component (R = 1, G = 0, B = 1), Attenuation = 1 Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G0_B1_A1_L0_E00_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 12
//POINT Light, Specular Component (R = 0, G = 1, B = 1), Attenuation = 1 Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G1_B1_A1_L0_E00_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 13
//POINT Light, Specular Component (R = 1, G = 1, B = 1), Attenuation = 1 Local Viewer = 0, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G1_B1_A1_L0_E00_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 
//POINT Light, Specular Component (R = 0, G = 0, B = 0), Attenuation = 0 Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
//Null_Lite


//Case # 14
//POINT Light, Specular Component (R = 1, G = 0, B = 0), Attenuation = 0 Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G0_B0_A0_L1_E00_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 15
//POINT Light, Specular Component (R = 0, G = 1, B = 0), Attenuation = 0 Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G1_B0_A0_L1_E00_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 16
//POINT Light, Specular Component (R = 1, G = 1, B = 0), Attenuation = 0 Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G1_B0_A0_L1_E00_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 17
//POINT Light, Specular Component (R = 0, G = 0, B = 1), Attenuation = 0 Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G0_B1_A0_L1_E00_EB0
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 18
//POINT Light, Specular Component (R = 1, G = 0, B = 1), Attenuation = 0 Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G0_B1_A0_L1_E00_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 19
//POINT Light, Specular Component (R = 0, G = 1, B = 1), Attenuation = 0 Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G1_B1_A0_L1_E00_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 20
//POINT Light, Specular Component (R = 1, G = 1, B = 1), Attenuation = 0 Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G1_B1_A0_L1_E00_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 
//POINT Light, Specular Component (R = 0, G = 0, B = 0), Attenuation = 1 Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
//Null_Lite


//Case # 21
//POINT Light, Specular Component (R = 1, G = 0, B = 0), Attenuation = 1 Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G0_B0_A1_L1_E00_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 22
//POINT Light, Specular Component (R = 0, G = 1, B = 0), Attenuation = 1 Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G1_B0_A1_L1_E00_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 23
//POINT Light, Specular Component (R = 1, G = 1, B = 0), Attenuation = 1 Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G1_B0_A1_L1_E00_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 24
//POINT Light, Specular Component (R = 0, G = 0, B = 1), Attenuation = 1 Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G0_B1_A1_L1_E00_EB0
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 25
//POINT Light, Specular Component (R = 1, G = 0, B = 1), Attenuation = 1 Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G0_B1_A1_L1_E00_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 26
//POINT Light, Specular Component (R = 0, G = 1, B = 1), Attenuation = 1 Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G1_B1_A1_L1_E00_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 27
//POINT Light, Specular Component (R = 1, G = 1, B = 1), Attenuation = 1 Local Viewer = 1, Exponent0 = 0, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G1_B1_A1_L1_E00_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 0
#include "soapss.h"


//Case # 
//POINT Light, Specular Component (R = 0, G = 0, B = 0), Attenuation = 0 Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
//Null_Lite


//Case # 28
//POINT Light, Specular Component (R = 1, G = 0, B = 0), Attenuation = 0 Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G0_B0_A0_L0_E01_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 29
//POINT Light, Specular Component (R = 0, G = 1, B = 0), Attenuation = 0 Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G1_B0_A0_L0_E01_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 30
//POINT Light, Specular Component (R = 1, G = 1, B = 0), Attenuation = 0 Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G1_B0_A0_L0_E01_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 31
//POINT Light, Specular Component (R = 0, G = 0, B = 1), Attenuation = 0 Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G0_B1_A0_L0_E01_EB0
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 32
//POINT Light, Specular Component (R = 1, G = 0, B = 1), Attenuation = 0 Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G0_B1_A0_L0_E01_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 33
//POINT Light, Specular Component (R = 0, G = 1, B = 1), Attenuation = 0 Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G1_B1_A0_L0_E01_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 34
//POINT Light, Specular Component (R = 1, G = 1, B = 1), Attenuation = 0 Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G1_B1_A0_L0_E01_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 
//POINT Light, Specular Component (R = 0, G = 0, B = 0), Attenuation = 1 Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
//Null_Lite


//Case # 35
//POINT Light, Specular Component (R = 1, G = 0, B = 0), Attenuation = 1 Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G0_B0_A1_L0_E01_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 36
//POINT Light, Specular Component (R = 0, G = 1, B = 0), Attenuation = 1 Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G1_B0_A1_L0_E01_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 37
//POINT Light, Specular Component (R = 1, G = 1, B = 0), Attenuation = 1 Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G1_B0_A1_L0_E01_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 38
//POINT Light, Specular Component (R = 0, G = 0, B = 1), Attenuation = 1 Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G0_B1_A1_L0_E01_EB0
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 39
//POINT Light, Specular Component (R = 1, G = 0, B = 1), Attenuation = 1 Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G0_B1_A1_L0_E01_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 40
//POINT Light, Specular Component (R = 0, G = 1, B = 1), Attenuation = 1 Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G1_B1_A1_L0_E01_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 41
//POINT Light, Specular Component (R = 1, G = 1, B = 1), Attenuation = 1 Local Viewer = 0, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G1_B1_A1_L0_E01_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 
//POINT Light, Specular Component (R = 0, G = 0, B = 0), Attenuation = 0 Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
//Null_Lite


//Case # 42
//POINT Light, Specular Component (R = 1, G = 0, B = 0), Attenuation = 0 Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G0_B0_A0_L1_E01_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 43
//POINT Light, Specular Component (R = 0, G = 1, B = 0), Attenuation = 0 Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G1_B0_A0_L1_E01_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 44
//POINT Light, Specular Component (R = 1, G = 1, B = 0), Attenuation = 0 Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G1_B0_A0_L1_E01_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 45
//POINT Light, Specular Component (R = 0, G = 0, B = 1), Attenuation = 0 Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G0_B1_A0_L1_E01_EB0
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 46
//POINT Light, Specular Component (R = 1, G = 0, B = 1), Attenuation = 0 Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G0_B1_A0_L1_E01_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 47
//POINT Light, Specular Component (R = 0, G = 1, B = 1), Attenuation = 0 Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G1_B1_A0_L1_E01_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 48
//POINT Light, Specular Component (R = 1, G = 1, B = 1), Attenuation = 0 Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G1_B1_A0_L1_E01_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 
//POINT Light, Specular Component (R = 0, G = 0, B = 0), Attenuation = 1 Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
//Null_Lite


//Case # 49
//POINT Light, Specular Component (R = 1, G = 0, B = 0), Attenuation = 1 Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G0_B0_A1_L1_E01_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 50
//POINT Light, Specular Component (R = 0, G = 1, B = 0), Attenuation = 1 Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G1_B0_A1_L1_E01_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 51
//POINT Light, Specular Component (R = 1, G = 1, B = 0), Attenuation = 1 Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G1_B0_A1_L1_E01_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 52
//POINT Light, Specular Component (R = 0, G = 0, B = 1), Attenuation = 1 Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G0_B1_A1_L1_E01_EB0
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 53
//POINT Light, Specular Component (R = 1, G = 0, B = 1), Attenuation = 1 Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G0_B1_A1_L1_E01_EB0
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 54
//POINT Light, Specular Component (R = 0, G = 1, B = 1), Attenuation = 1 Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R0_G1_B1_A1_L1_E01_EB0
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 55
//POINT Light, Specular Component (R = 1, G = 1, B = 1), Attenuation = 1 Local Viewer = 1, Exponent0 = 1, Big Exponent = 0
#define NAME POINT_SPOT_SPEC_R1_G1_B1_A1_L1_E01_EB0
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 1
#define EXP_BIG 0
#include "soapss.h"


//Case # 
//POINT Light, Specular Component (R = 0, G = 0, B = 0), Attenuation = 0 Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
//Null_Lite


//Case # 56
//POINT Light, Specular Component (R = 1, G = 0, B = 0), Attenuation = 0 Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R1_G0_B0_A0_L0_E00_EB1
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 57
//POINT Light, Specular Component (R = 0, G = 1, B = 0), Attenuation = 0 Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R0_G1_B0_A0_L0_E00_EB1
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 58
//POINT Light, Specular Component (R = 1, G = 1, B = 0), Attenuation = 0 Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R1_G1_B0_A0_L0_E00_EB1
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 59
//POINT Light, Specular Component (R = 0, G = 0, B = 1), Attenuation = 0 Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R0_G0_B1_A0_L0_E00_EB1
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 60
//POINT Light, Specular Component (R = 1, G = 0, B = 1), Attenuation = 0 Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R1_G0_B1_A0_L0_E00_EB1
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 61
//POINT Light, Specular Component (R = 0, G = 1, B = 1), Attenuation = 0 Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R0_G1_B1_A0_L0_E00_EB1
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 62
//POINT Light, Specular Component (R = 1, G = 1, B = 1), Attenuation = 0 Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R1_G1_B1_A0_L0_E00_EB1
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 
//POINT Light, Specular Component (R = 0, G = 0, B = 0), Attenuation = 1 Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
//Null_Lite


//Case # 63
//POINT Light, Specular Component (R = 1, G = 0, B = 0), Attenuation = 1 Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R1_G0_B0_A1_L0_E00_EB1
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 64
//POINT Light, Specular Component (R = 0, G = 1, B = 0), Attenuation = 1 Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R0_G1_B0_A1_L0_E00_EB1
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 65
//POINT Light, Specular Component (R = 1, G = 1, B = 0), Attenuation = 1 Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R1_G1_B0_A1_L0_E00_EB1
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 66
//POINT Light, Specular Component (R = 0, G = 0, B = 1), Attenuation = 1 Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R0_G0_B1_A1_L0_E00_EB1
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 67
//POINT Light, Specular Component (R = 1, G = 0, B = 1), Attenuation = 1 Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R1_G0_B1_A1_L0_E00_EB1
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 68
//POINT Light, Specular Component (R = 0, G = 1, B = 1), Attenuation = 1 Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R0_G1_B1_A1_L0_E00_EB1
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 69
//POINT Light, Specular Component (R = 1, G = 1, B = 1), Attenuation = 1 Local Viewer = 0, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R1_G1_B1_A1_L0_E00_EB1
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 0
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 
//POINT Light, Specular Component (R = 0, G = 0, B = 0), Attenuation = 0 Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
//Null_Lite


//Case # 70
//POINT Light, Specular Component (R = 1, G = 0, B = 0), Attenuation = 0 Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R1_G0_B0_A0_L1_E00_EB1
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 71
//POINT Light, Specular Component (R = 0, G = 1, B = 0), Attenuation = 0 Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R0_G1_B0_A0_L1_E00_EB1
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 72
//POINT Light, Specular Component (R = 1, G = 1, B = 0), Attenuation = 0 Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R1_G1_B0_A0_L1_E00_EB1
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 73
//POINT Light, Specular Component (R = 0, G = 0, B = 1), Attenuation = 0 Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R0_G0_B1_A0_L1_E00_EB1
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 74
//POINT Light, Specular Component (R = 1, G = 0, B = 1), Attenuation = 0 Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R1_G0_B1_A0_L1_E00_EB1
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 75
//POINT Light, Specular Component (R = 0, G = 1, B = 1), Attenuation = 0 Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R0_G1_B1_A0_L1_E00_EB1
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 76
//POINT Light, Specular Component (R = 1, G = 1, B = 1), Attenuation = 0 Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R1_G1_B1_A0_L1_E00_EB1
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 0
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 
//POINT Light, Specular Component (R = 0, G = 0, B = 0), Attenuation = 1 Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
//Null_Lite


//Case # 77
//POINT Light, Specular Component (R = 1, G = 0, B = 0), Attenuation = 1 Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R1_G0_B0_A1_L1_E00_EB1
#define DO_SR 1
#define DO_SG 0
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 78
//POINT Light, Specular Component (R = 0, G = 1, B = 0), Attenuation = 1 Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R0_G1_B0_A1_L1_E00_EB1
#define DO_SR 0
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 79
//POINT Light, Specular Component (R = 1, G = 1, B = 0), Attenuation = 1 Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R1_G1_B0_A1_L1_E00_EB1
#define DO_SR 1
#define DO_SG 1
#define DO_SB 0
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 80
//POINT Light, Specular Component (R = 0, G = 0, B = 1), Attenuation = 1 Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R0_G0_B1_A1_L1_E00_EB1
#define DO_SR 0
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 81
//POINT Light, Specular Component (R = 1, G = 0, B = 1), Attenuation = 1 Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R1_G0_B1_A1_L1_E00_EB1
#define DO_SR 1
#define DO_SG 0
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 82
//POINT Light, Specular Component (R = 0, G = 1, B = 1), Attenuation = 1 Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R0_G1_B1_A1_L1_E00_EB1
#define DO_SR 0
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"


//Case # 84
//POINT Light, Specular Component (R = 1, G = 1, B = 1), Attenuation = 1 Local Viewer = 1, Exponent0 = 0, Big Exponent = 1
#define NAME POINT_SPOT_SPEC_R1_G1_B1_A1_L1_E00_EB1
#define DO_SR 1
#define DO_SG 1
#define DO_SB 1
#define ATTENUATE 1
#define LOCAL_VIEWER 1
#define EXP0 0
#define EXP_BIG 1
#include "soapss.h"



//***********************************************
// Spot Cases
//***********************************************

//-----------------------------------------------
// Spot Diffuse
//-----------------------------------------------

//Case # 0
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 0
//Null_Lite


//Case # 1
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB0_R0_G0_B0_A0_N0
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 2
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB0_R0_G0_B0_A0_N0
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 3
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB0_R0_G0_B0_A0_N0
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 4
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB1_R0_G0_B0_A0_N0
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 5
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB1_R0_G0_B0_A0_N0
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 6
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB1_R0_G0_B0_A0_N0
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 7
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB1_R0_G0_B0_A0_N0
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 8
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB0_R0_G0_B0_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 9
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB0_R0_G0_B0_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 10
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB0_R0_G0_B0_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 11
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB0_R0_G0_B0_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 12
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB1_R0_G0_B0_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 13
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB1_R0_G0_B0_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 14
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB1_R0_G0_B0_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 15
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB1_R0_G0_B0_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 24
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 1, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB0_R1_G0_B0_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 25
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 1, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB0_R1_G0_B0_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 26
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 1, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB0_R1_G0_B0_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 27
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 1, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB0_R1_G0_B0_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 28
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 1, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB1_R1_G0_B0_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 29
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 1, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB1_R1_G0_B0_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 30
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 1, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB1_R1_G0_B0_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 31
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 1, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB1_R1_G0_B0_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 40
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 1, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB0_R0_G1_B0_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 41
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 1, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB0_R0_G1_B0_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 42
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 1, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB0_R0_G1_B0_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 43
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 1, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB0_R0_G1_B0_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 44
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 1, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB1_R0_G1_B0_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 45
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 1, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB1_R0_G1_B0_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 46
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 1, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB1_R0_G1_B0_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 47
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 1, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB1_R0_G1_B0_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 56
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 1, G = 1, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB0_R1_G1_B0_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 57
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 1, G = 1, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB0_R1_G1_B0_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 58
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 1, G = 1, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB0_R1_G1_B0_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 59
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 1, G = 1, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB0_R1_G1_B0_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 60
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 1, G = 1, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB1_R1_G1_B0_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 61
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 1, G = 1, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB1_R1_G1_B0_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 62
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 1, G = 1, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB1_R1_G1_B0_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 63
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 1, G = 1, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB1_R1_G1_B0_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 72
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 1
#define NAME SPOT_DIFF_AR0_AG0_AB0_R0_G0_B1_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 73
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 1
#define NAME SPOT_DIFF_AR1_AG0_AB0_R0_G0_B1_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 74
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 1
#define NAME SPOT_DIFF_AR0_AG1_AB0_R0_G0_B1_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 75
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 1
#define NAME SPOT_DIFF_AR1_AG1_AB0_R0_G0_B1_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 76
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 1
#define NAME SPOT_DIFF_AR0_AG0_AB1_R0_G0_B1_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 77
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 1
#define NAME SPOT_DIFF_AR1_AG0_AB1_R0_G0_B1_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 78
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 1
#define NAME SPOT_DIFF_AR0_AG1_AB1_R0_G0_B1_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 79
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 1
#define NAME SPOT_DIFF_AR1_AG1_AB1_R0_G0_B1_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 88
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 1, G = 0, B = 1
#define NAME SPOT_DIFF_AR0_AG0_AB0_R1_G0_B1_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 89
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 1, G = 0, B = 1
#define NAME SPOT_DIFF_AR1_AG0_AB0_R1_G0_B1_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 90
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 1, G = 0, B = 1
#define NAME SPOT_DIFF_AR0_AG1_AB0_R1_G0_B1_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 91
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 1, G = 0, B = 1
#define NAME SPOT_DIFF_AR1_AG1_AB0_R1_G0_B1_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 92
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 1, G = 0, B = 1
#define NAME SPOT_DIFF_AR0_AG0_AB1_R1_G0_B1_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 93
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 1, G = 0, B = 1
#define NAME SPOT_DIFF_AR1_AG0_AB1_R1_G0_B1_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 94
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 1, G = 0, B = 1
#define NAME SPOT_DIFF_AR0_AG1_AB1_R1_G0_B1_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 95
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 1, G = 0, B = 1
#define NAME SPOT_DIFF_AR1_AG1_AB1_R1_G0_B1_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 104
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 1, B = 1
#define NAME SPOT_DIFF_AR0_AG0_AB0_R0_G1_B1_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 105
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 1, B = 1
#define NAME SPOT_DIFF_AR1_AG0_AB0_R0_G1_B1_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 106
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 1, B = 1
#define NAME SPOT_DIFF_AR0_AG1_AB0_R0_G1_B1_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 107
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 1, B = 1
#define NAME SPOT_DIFF_AR1_AG1_AB0_R0_G1_B1_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 108
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 1, B = 1
#define NAME SPOT_DIFF_AR0_AG0_AB1_R0_G1_B1_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 109
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 1, B = 1
#define NAME SPOT_DIFF_AR1_AG0_AB1_R0_G1_B1_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 110
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 1, B = 1
#define NAME SPOT_DIFF_AR0_AG1_AB1_R0_G1_B1_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 111
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 1, B = 1
#define NAME SPOT_DIFF_AR1_AG1_AB1_R0_G1_B1_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 120
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 1, G = 1, B = 1
#define NAME SPOT_DIFF_AR0_AG0_AB0_R1_G1_B1_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 121
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 1, G = 1, B = 1
#define NAME SPOT_DIFF_AR1_AG0_AB0_R1_G1_B1_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 122
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 1, G = 1, B = 1
#define NAME SPOT_DIFF_AR0_AG1_AB0_R1_G1_B1_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 123
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 1, G = 1, B = 1
#define NAME SPOT_DIFF_AR1_AG1_AB0_R1_G1_B1_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 124
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 1, G = 1, B = 1
#define NAME SPOT_DIFF_AR0_AG0_AB1_R1_G1_B1_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 125
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 1, G = 1, B = 1
#define NAME SPOT_DIFF_AR1_AG0_AB1_R1_G1_B1_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 126
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 1, G = 1, B = 1
#define NAME SPOT_DIFF_AR0_AG1_AB1_R1_G1_B1_A0_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 127
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 1, G = 1, B = 1
#define NAME SPOT_DIFF_AR1_AG1_AB1_R1_G1_B1_A0_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 0
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 128
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB0_R0_G0_B0_A1_N0
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 129
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB0_R0_G0_B0_A1_N0
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 130
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB0_R0_G0_B0_A1_N0
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 131
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB0_R0_G0_B0_A1_N0
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 132
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB1_R0_G0_B0_A1_N0
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 133
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB1_R0_G0_B0_A1_N0
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 134
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB1_R0_G0_B0_A1_N0
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 135
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB1_R0_G0_B0_A1_N0
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 0
#include "soapsd.h"


//Case # 136
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB0_R0_G0_B0_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 137
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB0_R0_G0_B0_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 138
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB0_R0_G0_B0_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 139
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB0_R0_G0_B0_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 140
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB1_R0_G0_B0_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 141
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB1_R0_G0_B0_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 142
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB1_R0_G0_B0_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 143
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB1_R0_G0_B0_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 152
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 1, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB0_R1_G0_B0_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 153
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 1, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB0_R1_G0_B0_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 154
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 1, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB0_R1_G0_B0_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 155
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 1, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB0_R1_G0_B0_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 156
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 1, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB1_R1_G0_B0_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 157
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 1, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB1_R1_G0_B0_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 158
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 1, G = 0, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB1_R1_G0_B0_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 159
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 1, G = 0, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB1_R1_G0_B0_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 168
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 1, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB0_R0_G1_B0_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 169
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 1, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB0_R0_G1_B0_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 170
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 1, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB0_R0_G1_B0_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 171
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 1, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB0_R0_G1_B0_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 172
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 1, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB1_R0_G1_B0_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 173
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 1, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB1_R0_G1_B0_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 174
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 1, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB1_R0_G1_B0_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 175
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 1, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB1_R0_G1_B0_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 184
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 1, G = 1, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB0_R1_G1_B0_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 185
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 1, G = 1, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB0_R1_G1_B0_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 186
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 1, G = 1, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB0_R1_G1_B0_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 187
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 1, G = 1, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB0_R1_G1_B0_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 188
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 1, G = 1, B = 0
#define NAME SPOT_DIFF_AR0_AG0_AB1_R1_G1_B0_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 189
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 1, G = 1, B = 0
#define NAME SPOT_DIFF_AR1_AG0_AB1_R1_G1_B0_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 190
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 1, G = 1, B = 0
#define NAME SPOT_DIFF_AR0_AG1_AB1_R1_G1_B0_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 191
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 1, G = 1, B = 0
#define NAME SPOT_DIFF_AR1_AG1_AB1_R1_G1_B0_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 0
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 200
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 1
#define NAME SPOT_DIFF_AR0_AG0_AB0_R0_G0_B1_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 201
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 0, B = 1
#define NAME SPOT_DIFF_AR1_AG0_AB0_R0_G0_B1_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 202
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 1
#define NAME SPOT_DIFF_AR0_AG1_AB0_R0_G0_B1_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 203
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 0, B = 1
#define NAME SPOT_DIFF_AR1_AG1_AB0_R0_G0_B1_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 204
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 1
#define NAME SPOT_DIFF_AR0_AG0_AB1_R0_G0_B1_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 205
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 0, B = 1
#define NAME SPOT_DIFF_AR1_AG0_AB1_R0_G0_B1_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 206
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 1
#define NAME SPOT_DIFF_AR0_AG1_AB1_R0_G0_B1_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 207
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 0, B = 1
#define NAME SPOT_DIFF_AR1_AG1_AB1_R0_G0_B1_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 216
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 1, G = 0, B = 1
#define NAME SPOT_DIFF_AR0_AG0_AB0_R1_G0_B1_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 217
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 1, G = 0, B = 1
#define NAME SPOT_DIFF_AR1_AG0_AB0_R1_G0_B1_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 218
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 1, G = 0, B = 1
#define NAME SPOT_DIFF_AR0_AG1_AB0_R1_G0_B1_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 219
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 1, G = 0, B = 1
#define NAME SPOT_DIFF_AR1_AG1_AB0_R1_G0_B1_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 220
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 1, G = 0, B = 1
#define NAME SPOT_DIFF_AR0_AG0_AB1_R1_G0_B1_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 221
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 1, G = 0, B = 1
#define NAME SPOT_DIFF_AR1_AG0_AB1_R1_G0_B1_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 222
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 1, G = 0, B = 1
#define NAME SPOT_DIFF_AR0_AG1_AB1_R1_G0_B1_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 223
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 1, G = 0, B = 1
#define NAME SPOT_DIFF_AR1_AG1_AB1_R1_G0_B1_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 0
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 232
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 0, G = 1, B = 1
#define NAME SPOT_DIFF_AR0_AG0_AB0_R0_G1_B1_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 233
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 0, G = 1, B = 1
#define NAME SPOT_DIFF_AR1_AG0_AB0_R0_G1_B1_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 234
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 0, G = 1, B = 1
#define NAME SPOT_DIFF_AR0_AG1_AB0_R0_G1_B1_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 235
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 0, G = 1, B = 1
#define NAME SPOT_DIFF_AR1_AG1_AB0_R0_G1_B1_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 236
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 0, G = 1, B = 1
#define NAME SPOT_DIFF_AR0_AG0_AB1_R0_G1_B1_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 237
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 0, G = 1, B = 1
#define NAME SPOT_DIFF_AR1_AG0_AB1_R0_G1_B1_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 238
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 0, G = 1, B = 1
#define NAME SPOT_DIFF_AR0_AG1_AB1_R0_G1_B1_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 239
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 0, G = 1, B = 1
#define NAME SPOT_DIFF_AR1_AG1_AB1_R0_G1_B1_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 0
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 248
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 0), Diffuse(R = 1, G = 1, B = 1
#define NAME SPOT_DIFF_AR0_AG0_AB0_R1_G1_B1_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 249
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 0), Diffuse(R = 1, G = 1, B = 1
#define NAME SPOT_DIFF_AR1_AG0_AB0_R1_G1_B1_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 250
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 0), Diffuse(R = 1, G = 1, B = 1
#define NAME SPOT_DIFF_AR0_AG1_AB0_R1_G1_B1_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 251
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 0), Diffuse(R = 1, G = 1, B = 1
#define NAME SPOT_DIFF_AR1_AG1_AB0_R1_G1_B1_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 0
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 252
//Spot Light, Diffuse Component Ambient(R = 0, G = 0, B = 1), Diffuse(R = 1, G = 1, B = 1
#define NAME SPOT_DIFF_AR0_AG0_AB1_R1_G1_B1_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 253
//Spot Light, Diffuse Component Ambient(R = 1, G = 0, B = 1), Diffuse(R = 1, G = 1, B = 1
#define NAME SPOT_DIFF_AR1_AG0_AB1_R1_G1_B1_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 0
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 254
//Spot Light, Diffuse Component Ambient(R = 0, G = 1, B = 1), Diffuse(R = 1, G = 1, B = 1
#define NAME SPOT_DIFF_AR0_AG1_AB1_R1_G1_B1_A1_N1
#define SPOT 1
#define DO_AR 0
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"


//Case # 255
//Spot Light, Diffuse Component Ambient(R = 1, G = 1, B = 1), Diffuse(R = 1, G = 1, B = 1
#define NAME SPOT_DIFF_AR1_AG1_AB1_R1_G1_B1_A1_N1
#define SPOT 1
#define DO_AR 1
#define DO_AG 1
#define DO_AB 1
#define DO_R 1
#define DO_G 1
#define DO_B 1
#define ATTENUATE 1
#define FALLOFF 0
#define NORMALS 1
#include "soapsd.h"




//********************************************************************************
// Tables
//********************************************************************************

TLLIGHT_SOA_FUNC_TABLE SOA_Light_fns =  { 
    {	// Directional lights w/o specular
    &Null_Lite,                        &DIR_DIFF_AR1_AG0_AB0_N0_R0_G0_B0, &DIR_DIFF_AR0_AG1_AB0_N0_R0_G0_B0, &DIR_DIFF_AR1_AG1_AB0_N0_R0_G0_B0, 
    &DIR_DIFF_AR0_AG0_AB1_N0_R0_G0_B0, &DIR_DIFF_AR1_AG0_AB1_N0_R0_G0_B0, &DIR_DIFF_AR0_AG1_AB1_N0_R0_G0_B0, &DIR_DIFF_AR1_AG1_AB1_N0_R0_G0_B0, 
    &DIR_DIFF_AR0_AG0_AB0_N1_R0_G0_B0, &DIR_DIFF_AR1_AG0_AB0_N1_R0_G0_B0, &DIR_DIFF_AR0_AG1_AB0_N1_R0_G0_B0, &DIR_DIFF_AR1_AG1_AB0_N1_R0_G0_B0, 
    &DIR_DIFF_AR0_AG0_AB1_N1_R0_G0_B0, &DIR_DIFF_AR1_AG0_AB1_N1_R0_G0_B0, &DIR_DIFF_AR0_AG1_AB1_N1_R0_G0_B0, &DIR_DIFF_AR1_AG1_AB1_N1_R0_G0_B0, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &DIR_DIFF_AR0_AG0_AB0_N1_R1_G0_B0, &DIR_DIFF_AR1_AG0_AB0_N1_R1_G0_B0, &DIR_DIFF_AR0_AG1_AB0_N1_R1_G0_B0, &DIR_DIFF_AR1_AG1_AB0_N1_R1_G0_B0, 
    &DIR_DIFF_AR0_AG0_AB1_N1_R1_G0_B0, &DIR_DIFF_AR1_AG0_AB1_N1_R1_G0_B0, &DIR_DIFF_AR0_AG1_AB1_N1_R1_G0_B0, &DIR_DIFF_AR1_AG1_AB1_N1_R1_G0_B0, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &DIR_DIFF_AR0_AG0_AB0_N1_R0_G1_B0, &DIR_DIFF_AR1_AG0_AB0_N1_R0_G1_B0, &DIR_DIFF_AR0_AG1_AB0_N1_R0_G1_B0, &DIR_DIFF_AR1_AG1_AB0_N1_R0_G1_B0, 
    &DIR_DIFF_AR0_AG0_AB1_N1_R0_G1_B0, &DIR_DIFF_AR1_AG0_AB1_N1_R0_G1_B0, &DIR_DIFF_AR0_AG1_AB1_N1_R0_G1_B0, &DIR_DIFF_AR1_AG1_AB1_N1_R0_G1_B0, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &DIR_DIFF_AR0_AG0_AB0_N1_R1_G1_B0, &DIR_DIFF_AR1_AG0_AB0_N1_R1_G1_B0, &DIR_DIFF_AR0_AG1_AB0_N1_R1_G1_B0, &DIR_DIFF_AR1_AG1_AB0_N1_R1_G1_B0, 
    &DIR_DIFF_AR0_AG0_AB1_N1_R1_G1_B0, &DIR_DIFF_AR1_AG0_AB1_N1_R1_G1_B0, &DIR_DIFF_AR0_AG1_AB1_N1_R1_G1_B0, &DIR_DIFF_AR1_AG1_AB1_N1_R1_G1_B0, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &DIR_DIFF_AR0_AG0_AB0_N1_R0_G0_B1, &DIR_DIFF_AR1_AG0_AB0_N1_R0_G0_B1, &DIR_DIFF_AR0_AG1_AB0_N1_R0_G0_B1, &DIR_DIFF_AR1_AG1_AB0_N1_R0_G0_B1, 
    &DIR_DIFF_AR0_AG0_AB1_N1_R0_G0_B1, &DIR_DIFF_AR1_AG0_AB1_N1_R0_G0_B1, &DIR_DIFF_AR0_AG1_AB1_N1_R0_G0_B1, &DIR_DIFF_AR1_AG1_AB1_N1_R0_G0_B1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &DIR_DIFF_AR0_AG0_AB0_N1_R1_G0_B1, &DIR_DIFF_AR1_AG0_AB0_N1_R1_G0_B1, &DIR_DIFF_AR0_AG1_AB0_N1_R1_G0_B1, &DIR_DIFF_AR1_AG1_AB0_N1_R1_G0_B1, 
    &DIR_DIFF_AR0_AG0_AB1_N1_R1_G0_B1, &DIR_DIFF_AR1_AG0_AB1_N1_R1_G0_B1, &DIR_DIFF_AR0_AG1_AB1_N1_R1_G0_B1, &DIR_DIFF_AR1_AG1_AB1_N1_R1_G0_B1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &DIR_DIFF_AR0_AG0_AB0_N1_R0_G1_B1, &DIR_DIFF_AR1_AG0_AB0_N1_R0_G1_B1, &DIR_DIFF_AR0_AG1_AB0_N1_R0_G1_B1, &DIR_DIFF_AR1_AG1_AB0_N1_R0_G1_B1, 
    &DIR_DIFF_AR0_AG0_AB1_N1_R0_G1_B1, &DIR_DIFF_AR1_AG0_AB1_N1_R0_G1_B1, &DIR_DIFF_AR0_AG1_AB1_N1_R0_G1_B1, &DIR_DIFF_AR1_AG1_AB1_N1_R0_G1_B1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &DIR_DIFF_AR0_AG0_AB0_N1_R1_G1_B1, &DIR_DIFF_AR1_AG0_AB0_N1_R1_G1_B1, &DIR_DIFF_AR0_AG1_AB0_N1_R1_G1_B1, &DIR_DIFF_AR1_AG1_AB0_N1_R1_G1_B1, 
    &DIR_DIFF_AR0_AG0_AB1_N1_R1_G1_B1, &DIR_DIFF_AR1_AG0_AB1_N1_R1_G1_B1, &DIR_DIFF_AR0_AG1_AB1_N1_R1_G1_B1, &DIR_DIFF_AR1_AG1_AB1_N1_R1_G1_B1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite
    },
    {	// Directional lights w/specular
    &Null_Lite,                    &DIR_SPEC_R1_G0_B0_L0_E00_EB0, &DIR_SPEC_R0_G1_B0_L0_E00_EB0, &DIR_SPEC_R1_G1_B0_L0_E00_EB0, 
    &DIR_SPEC_R0_G0_B1_L0_E00_EB0, &DIR_SPEC_R1_G0_B1_L0_E00_EB0, &DIR_SPEC_R0_G1_B1_L0_E00_EB0, &DIR_SPEC_R1_G1_B1_L0_E00_EB0, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite,                    &DIR_SPEC_R1_G0_B0_L1_E00_EB0, &DIR_SPEC_R0_G1_B0_L1_E00_EB0, &DIR_SPEC_R1_G1_B0_L1_E00_EB0, 
    &DIR_SPEC_R0_G0_B1_L1_E00_EB0, &DIR_SPEC_R1_G0_B1_L1_E00_EB0, &DIR_SPEC_R0_G1_B1_L1_E00_EB0, &DIR_SPEC_R1_G1_B1_L1_E00_EB0, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite,                    &DIR_SPEC_R1_G0_B0_L0_E01_EB0, &DIR_SPEC_R0_G1_B0_L0_E01_EB0, &DIR_SPEC_R1_G1_B0_L0_E01_EB0, 
    &DIR_SPEC_R0_G0_B1_L0_E01_EB0, &DIR_SPEC_R1_G0_B1_L0_E01_EB0, &DIR_SPEC_R0_G1_B1_L0_E01_EB0, &DIR_SPEC_R1_G1_B1_L0_E01_EB0, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite,                    &DIR_SPEC_R1_G0_B0_L1_E01_EB0, &DIR_SPEC_R0_G1_B0_L1_E01_EB0, &DIR_SPEC_R1_G1_B0_L1_E01_EB0, 
    &DIR_SPEC_R0_G0_B1_L1_E01_EB0, &DIR_SPEC_R1_G0_B1_L1_E01_EB0, &DIR_SPEC_R0_G1_B1_L1_E01_EB0, &DIR_SPEC_R1_G1_B1_L1_E01_EB0, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite,                    &DIR_SPEC_R1_G0_B0_L0_E00_EB1, &DIR_SPEC_R0_G1_B0_L0_E00_EB1, &DIR_SPEC_R1_G1_B0_L0_E00_EB1, 
    &DIR_SPEC_R0_G0_B1_L0_E00_EB1, &DIR_SPEC_R1_G0_B1_L0_E00_EB1, &DIR_SPEC_R0_G1_B1_L0_E00_EB1, &DIR_SPEC_R1_G1_B1_L0_E00_EB1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite,                    &DIR_SPEC_R1_G0_B0_L1_E00_EB1, &DIR_SPEC_R0_G1_B0_L1_E00_EB1, &DIR_SPEC_R1_G1_B0_L1_E00_EB1, 
    &DIR_SPEC_R0_G0_B1_L1_E00_EB1, &DIR_SPEC_R1_G0_B1_L1_E00_EB1, &DIR_SPEC_R0_G1_B1_L1_E00_EB1, &DIR_SPEC_R1_G1_B1_L1_E00_EB1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite
    },
    {	// Point lights w/o specular
    &Null_Lite,                             &POINT_DIFF_AR1_AG0_AB0_R0_G0_B0_A0_N0, &POINT_DIFF_AR0_AG1_AB0_R0_G0_B0_A0_N0, &POINT_DIFF_AR1_AG1_AB0_R0_G0_B0_A0_N0, 
    &POINT_DIFF_AR0_AG0_AB1_R0_G0_B0_A0_N0, &POINT_DIFF_AR1_AG0_AB1_R0_G0_B0_A0_N0, &POINT_DIFF_AR0_AG1_AB1_R0_G0_B0_A0_N0, &POINT_DIFF_AR1_AG1_AB1_R0_G0_B0_A0_N0, 
    &POINT_DIFF_AR0_AG0_AB0_R0_G0_B0_A0_N1, &POINT_DIFF_AR1_AG0_AB0_R0_G0_B0_A0_N1, &POINT_DIFF_AR0_AG1_AB0_R0_G0_B0_A0_N1, &POINT_DIFF_AR1_AG1_AB0_R0_G0_B0_A0_N1, 
    &POINT_DIFF_AR0_AG0_AB1_R0_G0_B0_A0_N1, &POINT_DIFF_AR1_AG0_AB1_R0_G0_B0_A0_N1, &POINT_DIFF_AR0_AG1_AB1_R0_G0_B0_A0_N1, &POINT_DIFF_AR1_AG1_AB1_R0_G0_B0_A0_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &POINT_DIFF_AR0_AG0_AB0_R1_G0_B0_A0_N1, &POINT_DIFF_AR1_AG0_AB0_R1_G0_B0_A0_N1, &POINT_DIFF_AR0_AG1_AB0_R1_G0_B0_A0_N1, &POINT_DIFF_AR1_AG1_AB0_R1_G0_B0_A0_N1, 
    &POINT_DIFF_AR0_AG0_AB1_R1_G0_B0_A0_N1, &POINT_DIFF_AR1_AG0_AB1_R1_G0_B0_A0_N1, &POINT_DIFF_AR0_AG1_AB1_R1_G0_B0_A0_N1, &POINT_DIFF_AR1_AG1_AB1_R1_G0_B0_A0_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &POINT_DIFF_AR0_AG0_AB0_R0_G1_B0_A0_N1, &POINT_DIFF_AR1_AG0_AB0_R0_G1_B0_A0_N1, &POINT_DIFF_AR0_AG1_AB0_R0_G1_B0_A0_N1, &POINT_DIFF_AR1_AG1_AB0_R0_G1_B0_A0_N1, 
    &POINT_DIFF_AR0_AG0_AB1_R0_G1_B0_A0_N1, &POINT_DIFF_AR1_AG0_AB1_R0_G1_B0_A0_N1, &POINT_DIFF_AR0_AG1_AB1_R0_G1_B0_A0_N1, &POINT_DIFF_AR1_AG1_AB1_R0_G1_B0_A0_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &POINT_DIFF_AR0_AG0_AB0_R1_G1_B0_A0_N1, &POINT_DIFF_AR1_AG0_AB0_R1_G1_B0_A0_N1, &POINT_DIFF_AR0_AG1_AB0_R1_G1_B0_A0_N1, &POINT_DIFF_AR1_AG1_AB0_R1_G1_B0_A0_N1, 
    &POINT_DIFF_AR0_AG0_AB1_R1_G1_B0_A0_N1, &POINT_DIFF_AR1_AG0_AB1_R1_G1_B0_A0_N1, &POINT_DIFF_AR0_AG1_AB1_R1_G1_B0_A0_N1, &POINT_DIFF_AR1_AG1_AB1_R1_G1_B0_A0_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &POINT_DIFF_AR0_AG0_AB0_R0_G0_B1_A0_N1, &POINT_DIFF_AR1_AG0_AB0_R0_G0_B1_A0_N1, &POINT_DIFF_AR0_AG1_AB0_R0_G0_B1_A0_N1, &POINT_DIFF_AR1_AG1_AB0_R0_G0_B1_A0_N1, 
    &POINT_DIFF_AR0_AG0_AB1_R0_G0_B1_A0_N1, &POINT_DIFF_AR1_AG0_AB1_R0_G0_B1_A0_N1, &POINT_DIFF_AR0_AG1_AB1_R0_G0_B1_A0_N1, &POINT_DIFF_AR1_AG1_AB1_R0_G0_B1_A0_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &POINT_DIFF_AR0_AG0_AB0_R1_G0_B1_A0_N1, &POINT_DIFF_AR1_AG0_AB0_R1_G0_B1_A0_N1, &POINT_DIFF_AR0_AG1_AB0_R1_G0_B1_A0_N1, &POINT_DIFF_AR1_AG1_AB0_R1_G0_B1_A0_N1, 
    &POINT_DIFF_AR0_AG0_AB1_R1_G0_B1_A0_N1, &POINT_DIFF_AR1_AG0_AB1_R1_G0_B1_A0_N1, &POINT_DIFF_AR0_AG1_AB1_R1_G0_B1_A0_N1, &POINT_DIFF_AR1_AG1_AB1_R1_G0_B1_A0_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &POINT_DIFF_AR0_AG0_AB0_R0_G1_B1_A0_N1, &POINT_DIFF_AR1_AG0_AB0_R0_G1_B1_A0_N1, &POINT_DIFF_AR0_AG1_AB0_R0_G1_B1_A0_N1, &POINT_DIFF_AR1_AG1_AB0_R0_G1_B1_A0_N1, 
    &POINT_DIFF_AR0_AG0_AB1_R0_G1_B1_A0_N1, &POINT_DIFF_AR1_AG0_AB1_R0_G1_B1_A0_N1, &POINT_DIFF_AR0_AG1_AB1_R0_G1_B1_A0_N1, &POINT_DIFF_AR1_AG1_AB1_R0_G1_B1_A0_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &POINT_DIFF_AR0_AG0_AB0_R1_G1_B1_A0_N1, &POINT_DIFF_AR1_AG0_AB0_R1_G1_B1_A0_N1, &POINT_DIFF_AR0_AG1_AB0_R1_G1_B1_A0_N1, &POINT_DIFF_AR1_AG1_AB0_R1_G1_B1_A0_N1, 
    &POINT_DIFF_AR0_AG0_AB1_R1_G1_B1_A0_N1, &POINT_DIFF_AR1_AG0_AB1_R1_G1_B1_A0_N1, &POINT_DIFF_AR0_AG1_AB1_R1_G1_B1_A0_N1, &POINT_DIFF_AR1_AG1_AB1_R1_G1_B1_A0_N1, 
    &POINT_DIFF_AR0_AG0_AB0_R0_G0_B0_A1_N0, &POINT_DIFF_AR1_AG0_AB0_R0_G0_B0_A1_N0, &POINT_DIFF_AR0_AG1_AB0_R0_G0_B0_A1_N0, &POINT_DIFF_AR1_AG1_AB0_R0_G0_B0_A1_N0, 
    &POINT_DIFF_AR0_AG0_AB1_R0_G0_B0_A1_N0, &POINT_DIFF_AR1_AG0_AB1_R0_G0_B0_A1_N0, &POINT_DIFF_AR0_AG1_AB1_R0_G0_B0_A1_N0, &POINT_DIFF_AR1_AG1_AB1_R0_G0_B0_A1_N0, 
    &POINT_DIFF_AR0_AG0_AB0_R0_G0_B0_A1_N1, &POINT_DIFF_AR1_AG0_AB0_R0_G0_B0_A1_N1, &POINT_DIFF_AR0_AG1_AB0_R0_G0_B0_A1_N1, &POINT_DIFF_AR1_AG1_AB0_R0_G0_B0_A1_N1, 
    &POINT_DIFF_AR0_AG0_AB1_R0_G0_B0_A1_N1, &POINT_DIFF_AR1_AG0_AB1_R0_G0_B0_A1_N1, &POINT_DIFF_AR0_AG1_AB1_R0_G0_B0_A1_N1, &POINT_DIFF_AR1_AG1_AB1_R0_G0_B0_A1_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &POINT_DIFF_AR0_AG0_AB0_R1_G0_B0_A1_N1, &POINT_DIFF_AR1_AG0_AB0_R1_G0_B0_A1_N1, &POINT_DIFF_AR0_AG1_AB0_R1_G0_B0_A1_N1, &POINT_DIFF_AR1_AG1_AB0_R1_G0_B0_A1_N1, 
    &POINT_DIFF_AR0_AG0_AB1_R1_G0_B0_A1_N1, &POINT_DIFF_AR1_AG0_AB1_R1_G0_B0_A1_N1, &POINT_DIFF_AR0_AG1_AB1_R1_G0_B0_A1_N1, &POINT_DIFF_AR1_AG1_AB1_R1_G0_B0_A1_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &POINT_DIFF_AR0_AG0_AB0_R0_G1_B0_A1_N1, &POINT_DIFF_AR1_AG0_AB0_R0_G1_B0_A1_N1, &POINT_DIFF_AR0_AG1_AB0_R0_G1_B0_A1_N1, &POINT_DIFF_AR1_AG1_AB0_R0_G1_B0_A1_N1, 
    &POINT_DIFF_AR0_AG0_AB1_R0_G1_B0_A1_N1, &POINT_DIFF_AR1_AG0_AB1_R0_G1_B0_A1_N1, &POINT_DIFF_AR0_AG1_AB1_R0_G1_B0_A1_N1, &POINT_DIFF_AR1_AG1_AB1_R0_G1_B0_A1_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &POINT_DIFF_AR0_AG0_AB0_R1_G1_B0_A1_N1, &POINT_DIFF_AR1_AG0_AB0_R1_G1_B0_A1_N1, &POINT_DIFF_AR0_AG1_AB0_R1_G1_B0_A1_N1, &POINT_DIFF_AR1_AG1_AB0_R1_G1_B0_A1_N1, 
    &POINT_DIFF_AR0_AG0_AB1_R1_G1_B0_A1_N1, &POINT_DIFF_AR1_AG0_AB1_R1_G1_B0_A1_N1, &POINT_DIFF_AR0_AG1_AB1_R1_G1_B0_A1_N1, &POINT_DIFF_AR1_AG1_AB1_R1_G1_B0_A1_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &POINT_DIFF_AR0_AG0_AB0_R0_G0_B1_A1_N1, &POINT_DIFF_AR1_AG0_AB0_R0_G0_B1_A1_N1, &POINT_DIFF_AR0_AG1_AB0_R0_G0_B1_A1_N1, &POINT_DIFF_AR1_AG1_AB0_R0_G0_B1_A1_N1, 
    &POINT_DIFF_AR0_AG0_AB1_R0_G0_B1_A1_N1, &POINT_DIFF_AR1_AG0_AB1_R0_G0_B1_A1_N1, &POINT_DIFF_AR0_AG1_AB1_R0_G0_B1_A1_N1, &POINT_DIFF_AR1_AG1_AB1_R0_G0_B1_A1_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &POINT_DIFF_AR0_AG0_AB0_R1_G0_B1_A1_N1, &POINT_DIFF_AR1_AG0_AB0_R1_G0_B1_A1_N1, &POINT_DIFF_AR0_AG1_AB0_R1_G0_B1_A1_N1, &POINT_DIFF_AR1_AG1_AB0_R1_G0_B1_A1_N1, 
    &POINT_DIFF_AR0_AG0_AB1_R1_G0_B1_A1_N1, &POINT_DIFF_AR1_AG0_AB1_R1_G0_B1_A1_N1, &POINT_DIFF_AR0_AG1_AB1_R1_G0_B1_A1_N1, &POINT_DIFF_AR1_AG1_AB1_R1_G0_B1_A1_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &POINT_DIFF_AR0_AG0_AB0_R0_G1_B1_A1_N1, &POINT_DIFF_AR1_AG0_AB0_R0_G1_B1_A1_N1, &POINT_DIFF_AR0_AG1_AB0_R0_G1_B1_A1_N1, &POINT_DIFF_AR1_AG1_AB0_R0_G1_B1_A1_N1, 
    &POINT_DIFF_AR0_AG0_AB1_R0_G1_B1_A1_N1, &POINT_DIFF_AR1_AG0_AB1_R0_G1_B1_A1_N1, &POINT_DIFF_AR0_AG1_AB1_R0_G1_B1_A1_N1, &POINT_DIFF_AR1_AG1_AB1_R0_G1_B1_A1_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &POINT_DIFF_AR0_AG0_AB0_R1_G1_B1_A1_N1, &POINT_DIFF_AR1_AG0_AB0_R1_G1_B1_A1_N1, &POINT_DIFF_AR0_AG1_AB0_R1_G1_B1_A1_N1, &POINT_DIFF_AR1_AG1_AB0_R1_G1_B1_A1_N1, 
    &POINT_DIFF_AR0_AG0_AB1_R1_G1_B1_A1_N1, &POINT_DIFF_AR1_AG0_AB1_R1_G1_B1_A1_N1, &POINT_DIFF_AR0_AG1_AB1_R1_G1_B1_A1_N1, &POINT_DIFF_AR1_AG1_AB1_R1_G1_B1_A1_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite
    },
    {	// Point lights w/specular
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A0_L0_E00_EB0, &POINT_SPOT_SPEC_R0_G1_B0_A0_L0_E00_EB0, &POINT_SPOT_SPEC_R1_G1_B0_A0_L0_E00_EB0, 
    &POINT_SPOT_SPEC_R0_G0_B1_A0_L0_E00_EB0, &POINT_SPOT_SPEC_R1_G0_B1_A0_L0_E00_EB0, &POINT_SPOT_SPEC_R0_G1_B1_A0_L0_E00_EB0, &POINT_SPOT_SPEC_R1_G1_B1_A0_L0_E00_EB0, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A1_L0_E00_EB0, &POINT_SPOT_SPEC_R0_G1_B0_A1_L0_E00_EB0, &POINT_SPOT_SPEC_R1_G1_B0_A1_L0_E00_EB0, 
    &POINT_SPOT_SPEC_R0_G0_B1_A1_L0_E00_EB0, &POINT_SPOT_SPEC_R1_G0_B1_A1_L0_E00_EB0, &POINT_SPOT_SPEC_R0_G1_B1_A1_L0_E00_EB0, &POINT_SPOT_SPEC_R1_G1_B1_A1_L0_E00_EB0, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A0_L1_E00_EB0, &POINT_SPOT_SPEC_R0_G1_B0_A0_L1_E00_EB0, &POINT_SPOT_SPEC_R1_G1_B0_A0_L1_E00_EB0, 
    &POINT_SPOT_SPEC_R0_G0_B1_A0_L1_E00_EB0, &POINT_SPOT_SPEC_R1_G0_B1_A0_L1_E00_EB0, &POINT_SPOT_SPEC_R0_G1_B1_A0_L1_E00_EB0, &POINT_SPOT_SPEC_R1_G1_B1_A0_L1_E00_EB0, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A1_L1_E00_EB0, &POINT_SPOT_SPEC_R0_G1_B0_A1_L1_E00_EB0, &POINT_SPOT_SPEC_R1_G1_B0_A1_L1_E00_EB0, 
    &POINT_SPOT_SPEC_R0_G0_B1_A1_L1_E00_EB0, &POINT_SPOT_SPEC_R1_G0_B1_A1_L1_E00_EB0, &POINT_SPOT_SPEC_R0_G1_B1_A1_L1_E00_EB0, &POINT_SPOT_SPEC_R1_G1_B1_A1_L1_E00_EB0, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A0_L0_E01_EB0, &POINT_SPOT_SPEC_R0_G1_B0_A0_L0_E01_EB0, &POINT_SPOT_SPEC_R1_G1_B0_A0_L0_E01_EB0, 
    &POINT_SPOT_SPEC_R0_G0_B1_A0_L0_E01_EB0, &POINT_SPOT_SPEC_R1_G0_B1_A0_L0_E01_EB0, &POINT_SPOT_SPEC_R0_G1_B1_A0_L0_E01_EB0, &POINT_SPOT_SPEC_R1_G1_B1_A0_L0_E01_EB0, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A1_L0_E01_EB0, &POINT_SPOT_SPEC_R0_G1_B0_A1_L0_E01_EB0, &POINT_SPOT_SPEC_R1_G1_B0_A1_L0_E01_EB0, 
    &POINT_SPOT_SPEC_R0_G0_B1_A1_L0_E01_EB0, &POINT_SPOT_SPEC_R1_G0_B1_A1_L0_E01_EB0, &POINT_SPOT_SPEC_R0_G1_B1_A1_L0_E01_EB0, &POINT_SPOT_SPEC_R1_G1_B1_A1_L0_E01_EB0, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A0_L1_E01_EB0, &POINT_SPOT_SPEC_R0_G1_B0_A0_L1_E01_EB0, &POINT_SPOT_SPEC_R1_G1_B0_A0_L1_E01_EB0, 
    &POINT_SPOT_SPEC_R0_G0_B1_A0_L1_E01_EB0, &POINT_SPOT_SPEC_R1_G0_B1_A0_L1_E01_EB0, &POINT_SPOT_SPEC_R0_G1_B1_A0_L1_E01_EB0, &POINT_SPOT_SPEC_R1_G1_B1_A0_L1_E01_EB0, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A1_L1_E01_EB0, &POINT_SPOT_SPEC_R0_G1_B0_A1_L1_E01_EB0, &POINT_SPOT_SPEC_R1_G1_B0_A1_L1_E01_EB0, 
    &POINT_SPOT_SPEC_R0_G0_B1_A1_L1_E01_EB0, &POINT_SPOT_SPEC_R1_G0_B1_A1_L1_E01_EB0, &POINT_SPOT_SPEC_R0_G1_B1_A1_L1_E01_EB0, &POINT_SPOT_SPEC_R1_G1_B1_A1_L1_E01_EB0, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A0_L0_E00_EB1, &POINT_SPOT_SPEC_R0_G1_B0_A0_L0_E00_EB1, &POINT_SPOT_SPEC_R1_G1_B0_A0_L0_E00_EB1, 
    &POINT_SPOT_SPEC_R0_G0_B1_A0_L0_E00_EB1, &POINT_SPOT_SPEC_R1_G0_B1_A0_L0_E00_EB1, &POINT_SPOT_SPEC_R0_G1_B1_A0_L0_E00_EB1, &POINT_SPOT_SPEC_R1_G1_B1_A0_L0_E00_EB1, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A1_L0_E00_EB1, &POINT_SPOT_SPEC_R0_G1_B0_A1_L0_E00_EB1, &POINT_SPOT_SPEC_R1_G1_B0_A1_L0_E00_EB1, 
    &POINT_SPOT_SPEC_R0_G0_B1_A1_L0_E00_EB1, &POINT_SPOT_SPEC_R1_G0_B1_A1_L0_E00_EB1, &POINT_SPOT_SPEC_R0_G1_B1_A1_L0_E00_EB1, &POINT_SPOT_SPEC_R1_G1_B1_A1_L0_E00_EB1, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A0_L1_E00_EB1, &POINT_SPOT_SPEC_R0_G1_B0_A0_L1_E00_EB1, &POINT_SPOT_SPEC_R1_G1_B0_A0_L1_E00_EB1, 
    &POINT_SPOT_SPEC_R0_G0_B1_A0_L1_E00_EB1, &POINT_SPOT_SPEC_R1_G0_B1_A0_L1_E00_EB1, &POINT_SPOT_SPEC_R0_G1_B1_A0_L1_E00_EB1, &POINT_SPOT_SPEC_R1_G1_B1_A0_L1_E00_EB1, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A1_L1_E00_EB1, &POINT_SPOT_SPEC_R0_G1_B0_A1_L1_E00_EB1, &POINT_SPOT_SPEC_R1_G1_B0_A1_L1_E00_EB1, 
    &POINT_SPOT_SPEC_R0_G0_B1_A1_L1_E00_EB1, &POINT_SPOT_SPEC_R1_G0_B1_A1_L1_E00_EB1, &POINT_SPOT_SPEC_R0_G1_B1_A1_L1_E00_EB1, &POINT_SPOT_SPEC_R1_G1_B1_A1_L1_E00_EB1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite
    },
    {	// Spot lights w/o specular
    &Null_Lite,                            &SPOT_DIFF_AR1_AG0_AB0_R0_G0_B0_A0_N0, &SPOT_DIFF_AR0_AG1_AB0_R0_G0_B0_A0_N0, &SPOT_DIFF_AR1_AG1_AB0_R0_G0_B0_A0_N0, 
    &SPOT_DIFF_AR0_AG0_AB1_R0_G0_B0_A0_N0, &SPOT_DIFF_AR1_AG0_AB1_R0_G0_B0_A0_N0, &SPOT_DIFF_AR0_AG1_AB1_R0_G0_B0_A0_N0, &SPOT_DIFF_AR1_AG1_AB1_R0_G0_B0_A0_N0, 
    &SPOT_DIFF_AR0_AG0_AB0_R0_G0_B0_A0_N1, &SPOT_DIFF_AR1_AG0_AB0_R0_G0_B0_A0_N1, &SPOT_DIFF_AR0_AG1_AB0_R0_G0_B0_A0_N1, &SPOT_DIFF_AR1_AG1_AB0_R0_G0_B0_A0_N1, 
    &SPOT_DIFF_AR0_AG0_AB1_R0_G0_B0_A0_N1, &SPOT_DIFF_AR1_AG0_AB1_R0_G0_B0_A0_N1, &SPOT_DIFF_AR0_AG1_AB1_R0_G0_B0_A0_N1, &SPOT_DIFF_AR1_AG1_AB1_R0_G0_B0_A0_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &SPOT_DIFF_AR0_AG0_AB0_R1_G0_B0_A0_N1, &SPOT_DIFF_AR1_AG0_AB0_R1_G0_B0_A0_N1, &SPOT_DIFF_AR0_AG1_AB0_R1_G0_B0_A0_N1, &SPOT_DIFF_AR1_AG1_AB0_R1_G0_B0_A0_N1, 
    &SPOT_DIFF_AR0_AG0_AB1_R1_G0_B0_A0_N1, &SPOT_DIFF_AR1_AG0_AB1_R1_G0_B0_A0_N1, &SPOT_DIFF_AR0_AG1_AB1_R1_G0_B0_A0_N1, &SPOT_DIFF_AR1_AG1_AB1_R1_G0_B0_A0_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &SPOT_DIFF_AR0_AG0_AB0_R0_G1_B0_A0_N1, &SPOT_DIFF_AR1_AG0_AB0_R0_G1_B0_A0_N1, &SPOT_DIFF_AR0_AG1_AB0_R0_G1_B0_A0_N1, &SPOT_DIFF_AR1_AG1_AB0_R0_G1_B0_A0_N1, 
    &SPOT_DIFF_AR0_AG0_AB1_R0_G1_B0_A0_N1, &SPOT_DIFF_AR1_AG0_AB1_R0_G1_B0_A0_N1, &SPOT_DIFF_AR0_AG1_AB1_R0_G1_B0_A0_N1, &SPOT_DIFF_AR1_AG1_AB1_R0_G1_B0_A0_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &SPOT_DIFF_AR0_AG0_AB0_R1_G1_B0_A0_N1, &SPOT_DIFF_AR1_AG0_AB0_R1_G1_B0_A0_N1, &SPOT_DIFF_AR0_AG1_AB0_R1_G1_B0_A0_N1, &SPOT_DIFF_AR1_AG1_AB0_R1_G1_B0_A0_N1, 
    &SPOT_DIFF_AR0_AG0_AB1_R1_G1_B0_A0_N1, &SPOT_DIFF_AR1_AG0_AB1_R1_G1_B0_A0_N1, &SPOT_DIFF_AR0_AG1_AB1_R1_G1_B0_A0_N1, &SPOT_DIFF_AR1_AG1_AB1_R1_G1_B0_A0_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &SPOT_DIFF_AR0_AG0_AB0_R0_G0_B1_A0_N1, &SPOT_DIFF_AR1_AG0_AB0_R0_G0_B1_A0_N1, &SPOT_DIFF_AR0_AG1_AB0_R0_G0_B1_A0_N1, &SPOT_DIFF_AR1_AG1_AB0_R0_G0_B1_A0_N1, 
    &SPOT_DIFF_AR0_AG0_AB1_R0_G0_B1_A0_N1, &SPOT_DIFF_AR1_AG0_AB1_R0_G0_B1_A0_N1, &SPOT_DIFF_AR0_AG1_AB1_R0_G0_B1_A0_N1, &SPOT_DIFF_AR1_AG1_AB1_R0_G0_B1_A0_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &SPOT_DIFF_AR0_AG0_AB0_R1_G0_B1_A0_N1, &SPOT_DIFF_AR1_AG0_AB0_R1_G0_B1_A0_N1, &SPOT_DIFF_AR0_AG1_AB0_R1_G0_B1_A0_N1, &SPOT_DIFF_AR1_AG1_AB0_R1_G0_B1_A0_N1, 
    &SPOT_DIFF_AR0_AG0_AB1_R1_G0_B1_A0_N1, &SPOT_DIFF_AR1_AG0_AB1_R1_G0_B1_A0_N1, &SPOT_DIFF_AR0_AG1_AB1_R1_G0_B1_A0_N1, &SPOT_DIFF_AR1_AG1_AB1_R1_G0_B1_A0_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &SPOT_DIFF_AR0_AG0_AB0_R0_G1_B1_A0_N1, &SPOT_DIFF_AR1_AG0_AB0_R0_G1_B1_A0_N1, &SPOT_DIFF_AR0_AG1_AB0_R0_G1_B1_A0_N1, &SPOT_DIFF_AR1_AG1_AB0_R0_G1_B1_A0_N1, 
    &SPOT_DIFF_AR0_AG0_AB1_R0_G1_B1_A0_N1, &SPOT_DIFF_AR1_AG0_AB1_R0_G1_B1_A0_N1, &SPOT_DIFF_AR0_AG1_AB1_R0_G1_B1_A0_N1, &SPOT_DIFF_AR1_AG1_AB1_R0_G1_B1_A0_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &SPOT_DIFF_AR0_AG0_AB0_R1_G1_B1_A0_N1, &SPOT_DIFF_AR1_AG0_AB0_R1_G1_B1_A0_N1, &SPOT_DIFF_AR0_AG1_AB0_R1_G1_B1_A0_N1, &SPOT_DIFF_AR1_AG1_AB0_R1_G1_B1_A0_N1, 
    &SPOT_DIFF_AR0_AG0_AB1_R1_G1_B1_A0_N1, &SPOT_DIFF_AR1_AG0_AB1_R1_G1_B1_A0_N1, &SPOT_DIFF_AR0_AG1_AB1_R1_G1_B1_A0_N1, &SPOT_DIFF_AR1_AG1_AB1_R1_G1_B1_A0_N1, 
    &SPOT_DIFF_AR0_AG0_AB0_R0_G0_B0_A1_N0, &SPOT_DIFF_AR1_AG0_AB0_R0_G0_B0_A1_N0, &SPOT_DIFF_AR0_AG1_AB0_R0_G0_B0_A1_N0, &SPOT_DIFF_AR1_AG1_AB0_R0_G0_B0_A1_N0, 
    &SPOT_DIFF_AR0_AG0_AB1_R0_G0_B0_A1_N0, &SPOT_DIFF_AR1_AG0_AB1_R0_G0_B0_A1_N0, &SPOT_DIFF_AR0_AG1_AB1_R0_G0_B0_A1_N0, &SPOT_DIFF_AR1_AG1_AB1_R0_G0_B0_A1_N0, 
    &SPOT_DIFF_AR0_AG0_AB0_R0_G0_B0_A1_N1, &SPOT_DIFF_AR1_AG0_AB0_R0_G0_B0_A1_N1, &SPOT_DIFF_AR0_AG1_AB0_R0_G0_B0_A1_N1, &SPOT_DIFF_AR1_AG1_AB0_R0_G0_B0_A1_N1, 
    &SPOT_DIFF_AR0_AG0_AB1_R0_G0_B0_A1_N1, &SPOT_DIFF_AR1_AG0_AB1_R0_G0_B0_A1_N1, &SPOT_DIFF_AR0_AG1_AB1_R0_G0_B0_A1_N1, &SPOT_DIFF_AR1_AG1_AB1_R0_G0_B0_A1_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &SPOT_DIFF_AR0_AG0_AB0_R1_G0_B0_A1_N1, &SPOT_DIFF_AR1_AG0_AB0_R1_G0_B0_A1_N1, &SPOT_DIFF_AR0_AG1_AB0_R1_G0_B0_A1_N1, &SPOT_DIFF_AR1_AG1_AB0_R1_G0_B0_A1_N1, 
    &SPOT_DIFF_AR0_AG0_AB1_R1_G0_B0_A1_N1, &SPOT_DIFF_AR1_AG0_AB1_R1_G0_B0_A1_N1, &SPOT_DIFF_AR0_AG1_AB1_R1_G0_B0_A1_N1, &SPOT_DIFF_AR1_AG1_AB1_R1_G0_B0_A1_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &SPOT_DIFF_AR0_AG0_AB0_R0_G1_B0_A1_N1, &SPOT_DIFF_AR1_AG0_AB0_R0_G1_B0_A1_N1, &SPOT_DIFF_AR0_AG1_AB0_R0_G1_B0_A1_N1, &SPOT_DIFF_AR1_AG1_AB0_R0_G1_B0_A1_N1, 
    &SPOT_DIFF_AR0_AG0_AB1_R0_G1_B0_A1_N1, &SPOT_DIFF_AR1_AG0_AB1_R0_G1_B0_A1_N1, &SPOT_DIFF_AR0_AG1_AB1_R0_G1_B0_A1_N1, &SPOT_DIFF_AR1_AG1_AB1_R0_G1_B0_A1_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &SPOT_DIFF_AR0_AG0_AB0_R1_G1_B0_A1_N1, &SPOT_DIFF_AR1_AG0_AB0_R1_G1_B0_A1_N1, &SPOT_DIFF_AR0_AG1_AB0_R1_G1_B0_A1_N1, &SPOT_DIFF_AR1_AG1_AB0_R1_G1_B0_A1_N1, 
    &SPOT_DIFF_AR0_AG0_AB1_R1_G1_B0_A1_N1, &SPOT_DIFF_AR1_AG0_AB1_R1_G1_B0_A1_N1, &SPOT_DIFF_AR0_AG1_AB1_R1_G1_B0_A1_N1, &SPOT_DIFF_AR1_AG1_AB1_R1_G1_B0_A1_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &SPOT_DIFF_AR0_AG0_AB0_R0_G0_B1_A1_N1, &SPOT_DIFF_AR1_AG0_AB0_R0_G0_B1_A1_N1, &SPOT_DIFF_AR0_AG1_AB0_R0_G0_B1_A1_N1, &SPOT_DIFF_AR1_AG1_AB0_R0_G0_B1_A1_N1, 
    &SPOT_DIFF_AR0_AG0_AB1_R0_G0_B1_A1_N1, &SPOT_DIFF_AR1_AG0_AB1_R0_G0_B1_A1_N1, &SPOT_DIFF_AR0_AG1_AB1_R0_G0_B1_A1_N1, &SPOT_DIFF_AR1_AG1_AB1_R0_G0_B1_A1_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &SPOT_DIFF_AR0_AG0_AB0_R1_G0_B1_A1_N1, &SPOT_DIFF_AR1_AG0_AB0_R1_G0_B1_A1_N1, &SPOT_DIFF_AR0_AG1_AB0_R1_G0_B1_A1_N1, &SPOT_DIFF_AR1_AG1_AB0_R1_G0_B1_A1_N1, 
    &SPOT_DIFF_AR0_AG0_AB1_R1_G0_B1_A1_N1, &SPOT_DIFF_AR1_AG0_AB1_R1_G0_B1_A1_N1, &SPOT_DIFF_AR0_AG1_AB1_R1_G0_B1_A1_N1, &SPOT_DIFF_AR1_AG1_AB1_R1_G0_B1_A1_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &SPOT_DIFF_AR0_AG0_AB0_R0_G1_B1_A1_N1, &SPOT_DIFF_AR1_AG0_AB0_R0_G1_B1_A1_N1, &SPOT_DIFF_AR0_AG1_AB0_R0_G1_B1_A1_N1, &SPOT_DIFF_AR1_AG1_AB0_R0_G1_B1_A1_N1, 
    &SPOT_DIFF_AR0_AG0_AB1_R0_G1_B1_A1_N1, &SPOT_DIFF_AR1_AG0_AB1_R0_G1_B1_A1_N1, &SPOT_DIFF_AR0_AG1_AB1_R0_G1_B1_A1_N1, &SPOT_DIFF_AR1_AG1_AB1_R0_G1_B1_A1_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &SPOT_DIFF_AR0_AG0_AB0_R1_G1_B1_A1_N1, &SPOT_DIFF_AR1_AG0_AB0_R1_G1_B1_A1_N1, &SPOT_DIFF_AR0_AG1_AB0_R1_G1_B1_A1_N1, &SPOT_DIFF_AR1_AG1_AB0_R1_G1_B1_A1_N1, 
    &SPOT_DIFF_AR0_AG0_AB1_R1_G1_B1_A1_N1, &SPOT_DIFF_AR1_AG0_AB1_R1_G1_B1_A1_N1, &SPOT_DIFF_AR0_AG1_AB1_R1_G1_B1_A1_N1, &SPOT_DIFF_AR1_AG1_AB1_R1_G1_B1_A1_N1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite
    },
    {	// Spot lights w/specular
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A0_L0_E00_EB0, &POINT_SPOT_SPEC_R0_G1_B0_A0_L0_E00_EB0, &POINT_SPOT_SPEC_R1_G1_B0_A0_L0_E00_EB0, 
    &POINT_SPOT_SPEC_R0_G0_B1_A0_L0_E00_EB0, &POINT_SPOT_SPEC_R1_G0_B1_A0_L0_E00_EB0, &POINT_SPOT_SPEC_R0_G1_B1_A0_L0_E00_EB0, &POINT_SPOT_SPEC_R1_G1_B1_A0_L0_E00_EB0, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A1_L0_E00_EB0, &POINT_SPOT_SPEC_R0_G1_B0_A1_L0_E00_EB0, &POINT_SPOT_SPEC_R1_G1_B0_A1_L0_E00_EB0, 
    &POINT_SPOT_SPEC_R0_G0_B1_A1_L0_E00_EB0, &POINT_SPOT_SPEC_R1_G0_B1_A1_L0_E00_EB0, &POINT_SPOT_SPEC_R0_G1_B1_A1_L0_E00_EB0, &POINT_SPOT_SPEC_R1_G1_B1_A1_L0_E00_EB0, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A0_L1_E00_EB0, &POINT_SPOT_SPEC_R0_G1_B0_A0_L1_E00_EB0, &POINT_SPOT_SPEC_R1_G1_B0_A0_L1_E00_EB0, 
    &POINT_SPOT_SPEC_R0_G0_B1_A0_L1_E00_EB0, &POINT_SPOT_SPEC_R1_G0_B1_A0_L1_E00_EB0, &POINT_SPOT_SPEC_R0_G1_B1_A0_L1_E00_EB0, &POINT_SPOT_SPEC_R1_G1_B1_A0_L1_E00_EB0, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A1_L1_E00_EB0, &POINT_SPOT_SPEC_R0_G1_B0_A1_L1_E00_EB0, &POINT_SPOT_SPEC_R1_G1_B0_A1_L1_E00_EB0, 
    &POINT_SPOT_SPEC_R0_G0_B1_A1_L1_E00_EB0, &POINT_SPOT_SPEC_R1_G0_B1_A1_L1_E00_EB0, &POINT_SPOT_SPEC_R0_G1_B1_A1_L1_E00_EB0, &POINT_SPOT_SPEC_R1_G1_B1_A1_L1_E00_EB0, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A0_L0_E01_EB0, &POINT_SPOT_SPEC_R0_G1_B0_A0_L0_E01_EB0, &POINT_SPOT_SPEC_R1_G1_B0_A0_L0_E01_EB0, 
    &POINT_SPOT_SPEC_R0_G0_B1_A0_L0_E01_EB0, &POINT_SPOT_SPEC_R1_G0_B1_A0_L0_E01_EB0, &POINT_SPOT_SPEC_R0_G1_B1_A0_L0_E01_EB0, &POINT_SPOT_SPEC_R1_G1_B1_A0_L0_E01_EB0, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A1_L0_E01_EB0, &POINT_SPOT_SPEC_R0_G1_B0_A1_L0_E01_EB0, &POINT_SPOT_SPEC_R1_G1_B0_A1_L0_E01_EB0, 
    &POINT_SPOT_SPEC_R0_G0_B1_A1_L0_E01_EB0, &POINT_SPOT_SPEC_R1_G0_B1_A1_L0_E01_EB0, &POINT_SPOT_SPEC_R0_G1_B1_A1_L0_E01_EB0, &POINT_SPOT_SPEC_R1_G1_B1_A1_L0_E01_EB0, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A0_L1_E01_EB0, &POINT_SPOT_SPEC_R0_G1_B0_A0_L1_E01_EB0, &POINT_SPOT_SPEC_R1_G1_B0_A0_L1_E01_EB0, 
    &POINT_SPOT_SPEC_R0_G0_B1_A0_L1_E01_EB0, &POINT_SPOT_SPEC_R1_G0_B1_A0_L1_E01_EB0, &POINT_SPOT_SPEC_R0_G1_B1_A0_L1_E01_EB0, &POINT_SPOT_SPEC_R1_G1_B1_A0_L1_E01_EB0, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A1_L1_E01_EB0, &POINT_SPOT_SPEC_R0_G1_B0_A1_L1_E01_EB0, &POINT_SPOT_SPEC_R1_G1_B0_A1_L1_E01_EB0, 
    &POINT_SPOT_SPEC_R0_G0_B1_A1_L1_E01_EB0, &POINT_SPOT_SPEC_R1_G0_B1_A1_L1_E01_EB0, &POINT_SPOT_SPEC_R0_G1_B1_A1_L1_E01_EB0, &POINT_SPOT_SPEC_R1_G1_B1_A1_L1_E01_EB0, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A0_L0_E00_EB1, &POINT_SPOT_SPEC_R0_G1_B0_A0_L0_E00_EB1, &POINT_SPOT_SPEC_R1_G1_B0_A0_L0_E00_EB1, 
    &POINT_SPOT_SPEC_R0_G0_B1_A0_L0_E00_EB1, &POINT_SPOT_SPEC_R1_G0_B1_A0_L0_E00_EB1, &POINT_SPOT_SPEC_R0_G1_B1_A0_L0_E00_EB1, &POINT_SPOT_SPEC_R1_G1_B1_A0_L0_E00_EB1, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A1_L0_E00_EB1, &POINT_SPOT_SPEC_R0_G1_B0_A1_L0_E00_EB1, &POINT_SPOT_SPEC_R1_G1_B0_A1_L0_E00_EB1, 
    &POINT_SPOT_SPEC_R0_G0_B1_A1_L0_E00_EB1, &POINT_SPOT_SPEC_R1_G0_B1_A1_L0_E00_EB1, &POINT_SPOT_SPEC_R0_G1_B1_A1_L0_E00_EB1, &POINT_SPOT_SPEC_R1_G1_B1_A1_L0_E00_EB1, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A0_L1_E00_EB1, &POINT_SPOT_SPEC_R0_G1_B0_A0_L1_E00_EB1, &POINT_SPOT_SPEC_R1_G1_B0_A0_L1_E00_EB1, 
    &POINT_SPOT_SPEC_R0_G0_B1_A0_L1_E00_EB1, &POINT_SPOT_SPEC_R1_G0_B1_A0_L1_E00_EB1, &POINT_SPOT_SPEC_R0_G1_B1_A0_L1_E00_EB1, &POINT_SPOT_SPEC_R1_G1_B1_A0_L1_E00_EB1, 
    &Null_Lite,                              &POINT_SPOT_SPEC_R1_G0_B0_A1_L1_E00_EB1, &POINT_SPOT_SPEC_R0_G1_B0_A1_L1_E00_EB1, &POINT_SPOT_SPEC_R1_G1_B0_A1_L1_E00_EB1, 
    &POINT_SPOT_SPEC_R0_G0_B1_A1_L1_E00_EB1, &POINT_SPOT_SPEC_R1_G0_B1_A1_L1_E00_EB1, &POINT_SPOT_SPEC_R0_G1_B1_A1_L1_E00_EB1, &POINT_SPOT_SPEC_R1_G1_B1_A1_L1_E00_EB1, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite, 
    &Null_Lite, &Null_Lite, &Null_Lite, &Null_Lite
    }
};
#endif
#endif
#endif
