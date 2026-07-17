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

#include <GL/gl.h>

void	gtst_init_matrixstuff(void) ;
void	gtst_mmode(int) ;
int	gtst_getmmode(void) ;
void	gtst_loadmatrix(float *) ;
void	gtst_getmatrix(float *,GLint) ;
void	gtst_multmatrix(float *) ;
void	gtst_scale(float,float,float) ;
void	gtst_translate(float,float,float) ;
void	gtst_rotate(float,float,float,float) ;
void	gtst_rot(float,char) ;
void	gtst_lookat(float,float,float,float,float,float,short) ;
void	gtst_polarview(float,short,short,short) ;
void	gtst_perspective(short,float,float,float) ;
void	gtst_ortho2(float,float,float,float) ;
void	gtst_ortho(float,float,float,float,float,float) ;
void	gtst_frustum(float,float,float,float,float,float) ;
void	gtst_pick(short *,GLint) ;
GLint	gtst_endpick(short *) ;

