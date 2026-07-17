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
** $Revision: 4$
** $Date: 10/11/00 7:52:07 PM$
*/
#define PROCNAME1(fbmode) __glDrawLine_##fbmode##_Flat
#define PROCNAME2(fbmode) __glDrawLine_##fbmode##_Flat_Dither
#define PROCNAME3(fbmode) __glDrawLine_##fbmode##_Smooth
#define PROCNAME4(fbmode) __glDrawLine_##fbmode##_Smooth_Dither
#define PROCNAME5(fbmode) __glDrawStippledLine_##fbmode##_Flat
#define PROCNAME6(fbmode) __glDrawStippledLine_##fbmode##_Flat_Dither
#define PROCNAME7(fbmode) __glDrawStippledLine_##fbmode##_Smooth
#define PROCNAME8(fbmode) __glDrawStippledLine_##fbmode##_Smooth_Dither
