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
** File name: soaldef.h
**
** Description: Lighting function case bit definitions. These are used by
** the code in ProcessLights to pick which lighting function to use.
**
** $Revision: 2$
** $Date: 10/11/00 8:50:34 PM$
**
** $Log: 
**  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    Napalm    1.0         05/11/00 Scott Kephart   
** $
*/


// Diffuse
#define DIFF_AR   (1 << 0)
#define DIFF_AG   (1 << 1)
#define DIFF_AB   (1 << 2)
#define DIFF_N     (1 << 3)
#define DIFF_R     (1 << 4)
#define DIFF_G     (1 << 5)
#define DIFF_B     (1 << 6)

#define DIFF_A     (1 << 7)
#define DIFF_F     (1 << 8)
#define MAX_DIRECTIONAL_DIFF DIFF_N+DIFF_F+DIFF_A+DIFF_B+DIFF_G+DIFF_R+DIFF_AB+DIFF_AG+DIFF_AR
#define MAX_POINT_SPOT_DIFF DIFF_N+DIFF_F+DIFF_A+DIFF_B+DIFF_G+DIFF_R+DIFF_AB+DIFF_AG+DIFF_AR
         

// Specular
#define SPEC_R     (1 << 0)
#define SPEC_G     (1 << 1)
#define SPEC_B     (1 << 2)
#define SPEC_A     (1 << 3)
#define SPEC_L     (1 << 4)


#define SPEC_E0    (1 << 5)
#define SPEC_EB    (1 << 6)

#define MAX_DIRECTIONAL_SPEC SPEC_R+SPEC_G+SPEC_B+SPEC_L+SPEC_A+SPEC_E0+SPEC_EB
#define MAX_POINT_SPOT_SPEC SPEC_R+SPEC_G+SPEC_B+SPEC_A+SPEC_L+SPEC_E0+SPEC_EB

