/**************************************************************************
 *									  *
 * 		 Copyright (C) 1989, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/
/*
 * Defines for translating from architecture names to indices in
 * the archSpecs array of machines specific specification in the test data
 * structure.
 *
 * An architecture that is specified in the spec file as XXX gets the
 * corresponding #define ARCH_XXX where the case of the letters in XXX is the
 * same.
 *
 * Used by main.c (see setHWConditions()) and the perl script (spec.pl) that
 * creates spec.c from spec.ogtst
 *
 * IMPORTANT NOTE:
 * The value of an ARCH_XXX #define is used to index into the order of the
 * elements of the archSpec array in the Test data structure defined in
 * env/env.h.
 * spec.pl assumes that all #defines of the form ARCH_XXX in this file describe  
 * a valid architecture.  IT ALSO ASSUMES THAT THE ORDER WHICH THE ARCHITECTURES
 * APPEAR IN THIS FILE CORRESPOND TO THE ORDER OF THE ARRAY ELEMENTS IN THE
 * archSpec ARRAY. In other words if '#define ARCH_XXX EXPR' is the N'th
 * architecture defining line then EXPR should evaluate to N-1. 
 *
 * $Id: architectures.h,v 1.2 1997/09/12 07:02:49 pho Exp $
 */
#define ARCH_Generic   0
#define ARCH_VG1       0
