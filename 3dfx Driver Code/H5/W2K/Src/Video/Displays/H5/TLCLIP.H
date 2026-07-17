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
** File name: tlclip.h
**
** Description: Clippnig support macros for Software T&L HAL
**
** $Revision: 5$
** $Date: 10/11/00 8:45:05 PM$
**
** $Log: 
**  5    3dfx      1.3.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  4    Napalm Shared1.3         01/28/00 Scott Kephart   Big T&L Merge: FVF
**       handling changes
**  3    Napalm Shared1.2         10/26/99 Scott Kephart   rename clip.h to
**       tlclip.h to avoid a conflict with the  Windows 2K driver
**  2    Napalm Shared1.1         10/26/99 Scott Kephart   
**  1    Napalm Shared1.0         10/25/99 Scott Kephart   
** $
 * 
 * 1     1/19/00 8:43a Skephart
 * H5TOT Before the Napalm code freeze
** 
** 8     10/26/99 12:54a Skephart
** Added #ifdef TnL_HAL
*/

#ifdef TnL_HAL
//----------------------------------------------------------------------------
// Clipping macros
//----------------------------------------------------------------------------
// Interpolation flags
#define TLCLIP_INTERPOLATE_COLOR       (1<< 0)
#define TLCLIP_INTERPOLATE_SPECULAR    (1<< 1)
#define TLCLIP_INTERPOLATE_TEXTURE     (1<< 2)

// Non guardband clipping bits
#define TLCLIP_LEFT   D3DCLIP_LEFT
#define TLCLIP_RIGHT  D3DCLIP_RIGHT
#define TLCLIP_TOP    D3DCLIP_TOP
#define TLCLIP_BOTTOM D3DCLIP_BOTTOM
#define TLCLIP_FRONT  D3DCLIP_FRONT
#define TLCLIP_BACK   D3DCLIP_BACK

//----------------------------------------------------------------------------
// User define clip plane bits.
// Each of these bits is set if the vertex is clipped by the associated
// clip plane.
//----------------------------------------------------------------------------
#define TLCLIP_USERCLIPPLANE0    D3DCLIP_GEN0
#define TLCLIP_USERCLIPPLANE1    D3DCLIP_GEN1
#define TLCLIP_USERCLIPPLANE2    D3DCLIP_GEN2
#define TLCLIP_USERCLIPPLANE3    D3DCLIP_GEN3
#define TLCLIP_USERCLIPPLANE4    D3DCLIP_GEN4
#define TLCLIP_USERCLIPPLANE5    D3DCLIP_GEN5
#define TLCLIP_USERPLANES_ALL    (TLCLIP_USERCLIPPLANE0 |       \
                                      TLCLIP_USERCLIPPLANE1 |   \
                                      TLCLIP_USERCLIPPLANE2 |   \
                                      TLCLIP_USERCLIPPLANE3 |   \
                                      TLCLIP_USERCLIPPLANE4 |   \
                                      TLCLIP_USERCLIPPLANE5 );


//---------------------------------------------------------------------
// Guard band clipping bits
//
// A guard bit is set when a point is out of guard band
// Guard bits should be cleared before a call to clip a triangle, because
// they are the same as CLIPPED_... bits
//
// Example of clipping bits setting for X coordinate:
//
// if -w < x < w           no clipping bit is set
// if -w*ax1 < x <= -w     RRCLIP_LEFT bit is set
// if x < -w*ax1           RRCLIPGB_LEFT bit is set
//---------------------------------------------------------------------

#define TLCLIPGB_LEFT    (TLCLIP_USERCLIPPLANE5 << 1)
#define TLCLIPGB_RIGHT   (TLCLIP_USERCLIPPLANE5 << 2)
#define TLCLIPGB_TOP     (TLCLIP_USERCLIPPLANE5 << 3)
#define TLCLIPGB_BOTTOM  (TLCLIP_USERCLIPPLANE5 << 4)




#define TLCLIP_ALL (TLCLIP_LEFT  | TLCLIP_RIGHT   |         \
                    TLCLIP_TOP   | TLCLIP_BOTTOM  |         \
                    TLCLIP_FRONT | TLCLIP_BACK    |         \
                    TLCLIP_USERPLANES_ALL)

#define TLCLIPGB_ALL (TLCLIPGB_LEFT | TLCLIPGB_RIGHT |      \
                      TLCLIPGB_TOP | TLCLIPGB_BOTTOM |      \
                      TLCLIP_FRONT | TLCLIP_BACK     |      \
                      TLCLIP_USERPLANES_ALL)

// If only these bits are set, then this point is inside the guard band
#define TLCLIP_INGUARDBAND (TLCLIP_LEFT | TLCLIP_RIGHT |    \
                            TLCLIP_TOP  | TLCLIP_BOTTOM)


//---------------------------------------------------------------------
// Bit numbers for each clip flag
//
#define TLCLIP_LEFTBIT     1
#define TLCLIP_RIGHTBIT    2
#define TLCLIP_TOPBIT      3
#define TLCLIP_BOTTOMBIT   4
#define TLCLIP_FRONTBIT    5
#define TLCLIP_BACKBIT     6

#define TLCLIP_USERCLIPLANE0BIT     7
#define TLCLIP_USERCLIPLANE1BIT     8
#define TLCLIP_USERCLIPLANE2BIT     9
#define TLCLIP_USERCLIPLANE3BIT     10
#define TLCLIP_USERCLIPLANE4BIT     11
#define TLCLIP_USERCLIPLANE5BIT     12

#define TLCLIPGB_LEFTBIT   13
#define TLCLIPGB_RIGHTBIT  14
#define TLCLIPGB_TOPBIT    15
#define TLCLIPGB_BOTTOMBIT 16

#define CLIPPED_LEFT    (TLCLIP_USERCLIPPLANE5 << 1)
#define CLIPPED_RIGHT   (TLCLIP_USERCLIPPLANE5 << 2)
#define CLIPPED_TOP     (TLCLIP_USERCLIPPLANE5 << 3)
#define CLIPPED_BOTTOM  (TLCLIP_USERCLIPPLANE5 << 4)
#define CLIPPED_FRONT   (TLCLIP_USERCLIPPLANE5 << 5)
#define CLIPPED_BACK    (TLCLIP_USERCLIPPLANE5 << 6)

#define CLIPPED_ENABLE  (TLCLIP_USERCLIPPLANE5 << 7) // wireframe enable flag

#define CLIPPED_ALL (CLIPPED_LEFT|CLIPPED_RIGHT     \
             |CLIPPED_TOP|CLIPPED_BOTTOM            \
             |CLIPPED_FRONT|CLIPPED_BACK)



#endif //TnL_HAL
