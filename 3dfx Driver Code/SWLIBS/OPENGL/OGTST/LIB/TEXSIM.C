/**************************************************************************
 *                                                                        *
 *               Copyright (C) 1995, Silicon Graphics, Inc.               *
 *                                                                        *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *                                                                        *
 **************************************************************************/

#include "ogtst.h"
#include <strings.h>
#include <stdio.h>

/*************************************************************************
*   Function to get a final color value from 4 texture components        *
*   of type GLubyte, 4 fragment components and a texturing function      *
*************************************************************************/     
GLint ogLib4Ub(char *func, GLubyte rubt, GLubyte gubt, GLubyte bubt, GLubyte aubt, float crf, float cgf, float cbf, float caf) 
{

    GLfloat   crt, crv, cgt, cgv, cbt, cbv, cat, cav;
    GLubyte   rubv, gubv, bubv, aubv;
    GLint     color;
    
    crt = OGTST_UB_TO_F(rubt);
    cgt = OGTST_UB_TO_F(gubt);
    cbt = OGTST_UB_TO_F(bubt);
    cat = OGTST_UB_TO_F(aubt);
    
    if(strncmp(func, "DECAL", 5) == 0){
	crv = (1 - cat) * crf + cat * crt;
	cgv = (1 - cat) * cgf + cat * cgt;
	cbv = (1 - cat) * cbf + cat * cbt;
	cav = caf;
    }
    else if(strncmp(func, "MODULATE", 8) == 0){
	crv = crt * crf;
	cgv = cgt * cgf;
	cbv = cbt * cbf;
	cav = cat * caf;
    }
    
    rubv = OGTST_F_TO_UB(crv);
    gubv = OGTST_F_TO_UB(cgv);
    bubv = OGTST_F_TO_UB(cbv);
    aubv = OGTST_F_TO_UB(cav);

    color = ((rubv << 24) & 0xff000000) | ((gubv << 16) & 0x00ff0000) | ((bubv << 8) & 0x0000ff00) | ((aubv << 0) & 0x000000ff);
    return(color);
    
}

/*************************************************************************
*   Function to get a final color value from 4 texture components        *
*   of type GLushort, 4 fragment components and a texturing function     *
*************************************************************************/
GLint ogLib4Us(char *func, GLushort rust, GLushort gust, GLushort bust, GLushort aust, float crf, float cgf, float cbf, float caf) 
{
    
    GLfloat   crt, crv, cgt, cgv, cbt, cbv, cat, cav;
    GLubyte   rubv, gubv, bubv, aubv;
    GLint color;
    
    crt = OGTST_US_TO_F(rust);
    cgt = OGTST_US_TO_F(gust);
    cbt = OGTST_US_TO_F(bust);
    cat = OGTST_US_TO_F(aust);
    
    if(strncmp(func, "DECAL", 5) == 0){
	crv = (1 - cat) * crf + cat * crt;
	cgv = (1 - cat) * cgf + cat * cgt;
	cbv = (1 - cat) * cbf + cat * cbt;
	cav = caf;
    }
    else if(strncmp(func, "MODULATE", 8) == 0){
	crv = crt * crf;
	cgv = cgt * cgf;
	cbv = cbt * cbf;
	cav = cat * caf;
    }
    
    rubv = OGTST_F_TO_UB(crv);
    gubv = OGTST_F_TO_UB(cgv);
    bubv = OGTST_F_TO_UB(cbv);
    aubv = OGTST_F_TO_UB(cav);

    color = ((rubv << 24) & 0xff000000) | ((gubv << 16) & 0x00ff0000) | ((bubv << 8) & 0x0000ff00) | ((aubv << 0) & 0x000000ff);
    return(color);
    
}

/*************************************************************************
*   Function to get a final color value from 4 texture components        *
*   of type GLuint, 4 fragment components and a texturing function       *
*************************************************************************/
GLint ogLib4Ui(char *func, GLuint ruit, GLuint guit, GLuint buit, GLuint auit, float crf, float cgf, float cbf, float caf) 
{
    
    GLfloat   crt, crv, cgt, cgv, cbt, cbv, cat, cav;
    GLubyte   rubv, gubv, bubv, aubv;
    GLint color;
    
    crt = OGTST_UI_TO_F(ruit);
    cgt = OGTST_UI_TO_F(guit);
    cbt = OGTST_UI_TO_F(buit);
    cat = OGTST_UI_TO_F(auit);
    
    if(strncmp(func, "DECAL", 5) == 0){
	crv = (1 - cat) * crf + cat * crt;
	cgv = (1 - cat) * cgf + cat * cgt;
	cbv = (1 - cat) * cbf + cat * cbt;
	cav = caf;
    }
    else if(strncmp(func, "MODULATE", 8) == 0){
	crv = crt * crf;
	cgv = cgt * cgf;
	cbv = cbt * cbf;
	cav = cat * caf;
    }
    
    rubv = OGTST_F_TO_UB(crv);
    gubv = OGTST_F_TO_UB(cgv);
    bubv = OGTST_F_TO_UB(cbv);
    aubv = OGTST_F_TO_UB(cav);

    color = ((rubv << 24) & 0xff000000) | ((gubv << 16) & 0x00ff0000) | ((bubv << 8) & 0x0000ff00) | ((aubv << 0) & 0x000000ff);
    return(color);
    
}

/*************************************************************************
*   Function to get a final color value from 3 texture components        *
*   of type GLubyte, 4 fragment components and a texturing function      *
*************************************************************************/
GLint ogLib3Ub(char *func, GLubyte rubt, GLubyte gubt, GLubyte bubt, float crf, float cgf, float cbf, float caf) 
{
    
    GLfloat   crt, crv, cgt, cgv, cbt, cbv, cav;
    GLubyte   rubv, gubv, bubv, aubv;
    GLint color;
    
    crt = OGTST_UB_TO_F(rubt);
    cgt = OGTST_UB_TO_F(gubt);
    cbt = OGTST_UB_TO_F(bubt);
    
    if(strncmp(func, "DECAL", 5) == 0){
	crv = crt;
	cgv = cgt;
	cbv = cbt;
	cav = caf;
    }
    else if(strncmp(func, "MODULATE", 8) == 0){
	crv = crt * crf;
	cgv = cgt * cgf;
	cbv = cbt * cbf;
	cav = caf;
    }
    
    rubv = OGTST_F_TO_UB(crv);
    gubv = OGTST_F_TO_UB(cgv);
    bubv = OGTST_F_TO_UB(cbv);
    aubv = OGTST_F_TO_UB(cav);

    color = ((rubv << 24) & 0xff000000) | ((gubv << 16) & 0x00ff0000) | ((bubv << 8) & 0x0000ff00) | ((aubv << 0) & 0x000000ff);
    return(color);

}

/*************************************************************************
*   Function to get a final color value from 3 texture components        *
*   of type GLuint, 4 fragment components and a texturing function       *
*************************************************************************/
GLint ogLib3Ui(char *func, GLuint ruit, GLuint guit, GLuint buit, float crf, float cgf, float cbf, float caf) 
{
    
    GLfloat   crt, crv, cgt, cgv, cbt, cbv, cav;
    GLubyte   rubv, gubv, bubv, aubv;
    GLint color;
    
    crt = OGTST_UI_TO_F(ruit);
    cgt = OGTST_UI_TO_F(guit);
    cbt = OGTST_UI_TO_F(buit);
    
    if(strncmp(func, "DECAL", 5) == 0){
	crv = crt;
	cgv = cgt;
	cbv = cbt;
	cav = caf;
    }
    else if(strncmp(func, "MODULATE", 8) == 0){
	crv = crt * crf;
	cgv = cgt * cgf;
	cbv = cbt * cbf;
	cav = caf;
    }
    
    rubv = OGTST_F_TO_UB(crv);
    gubv = OGTST_F_TO_UB(cgv);
    bubv = OGTST_F_TO_UB(cbv);
    aubv = OGTST_F_TO_UB(cav);

    color = ((rubv << 24) & 0xff000000) | ((gubv << 16) & 0x00ff0000) | ((bubv << 8) & 0x0000ff00) | ((aubv << 0) & 0x000000ff);
    return(color);
  
}

/*************************************************************************
*   Function to get a final color value from 3 texture components        *
*   of type GLushort, 4 fragment components and a texturing function     *
*************************************************************************/
GLint ogLib3Us(char *func, GLushort rust, GLushort gust, GLushort bust, float crf, float cgf, float cbf, float caf) 
{
    
    GLfloat   crt, crv, cgt, cgv, cbt, cbv, cav;
    GLubyte   rubv, gubv, bubv, aubv;
    GLint color;
    
    crt = OGTST_US_TO_F(rust);
    cgt = OGTST_US_TO_F(gust);
    cbt = OGTST_US_TO_F(bust);
    
    if(strncmp(func, "DECAL", 5) == 0){
	crv = crt;
	cgv = cgt;
	cbv = cbt;
	cav = caf;
    }
    else if(strncmp(func, "MODULATE", 8) == 0){
	crv = crt * crf;
	cgv = cgt * cgf;
	cbv = cbt * cbf;
	cav = caf;
    }
    
    rubv = OGTST_F_TO_UB(crv);
    gubv = OGTST_F_TO_UB(cgv);
    bubv = OGTST_F_TO_UB(cbv);
    aubv = OGTST_F_TO_UB(cav);
    
    color = ((rubv << 24) & 0xff000000) | ((gubv << 16) & 0x00ff0000) | ((bubv << 8) & 0x0000ff00) | ((aubv << 0) & 0x000000ff);
    return(color);
   
}

/*************************************************************************
*   Function to get a final color value from 2 texture components        *
*   of type GLubyte, 4 fragment components and a texturing function      *
*************************************************************************/
GLint ogLib2Ub(char *func, GLubyte lubt, GLubyte aubt, float crf, float cgf, float cbf, float caf) 
{
    
    GLfloat   clt, crv, cgv, cat, cbv, cav;
    GLubyte   rubv, gubv, bubv, aubv;
    GLint color;
    
    clt = OGTST_UB_TO_F(lubt);
    cat = OGTST_UB_TO_F(aubt);
    
    if(strncmp(func, "MODULATE", 8) == 0){
	crv = clt * crf;
	cgv = clt * cgf;
	cbv = clt * cbf;
	cav = cat * caf;
    }
    
    rubv = OGTST_F_TO_UB(crv);
    gubv = OGTST_F_TO_UB(cgv);
    bubv = OGTST_F_TO_UB(cbv);
    aubv = OGTST_F_TO_UB(cav);

    color = ((rubv << 24) & 0xff000000) | ((gubv << 16) & 0x00ff0000) | ((bubv << 8) & 0x0000ff00) | ((aubv << 0) & 0x000000ff);
    return(color);
 
}

/*************************************************************************
*   Function to get a final color value from 2 texture components        *
*   of type GLuint, 4 fragment components and a texturing function       *
*************************************************************************/
GLint ogLib2Ui(char *func, GLuint luit, GLuint auit, float crf, float cgf, float cbf, float caf) 
{
    
    GLfloat   clt, crv, cgv, cat, cbv, cav;
    GLubyte   rubv, gubv, bubv, aubv;
    GLint color;
    
    clt = OGTST_UI_TO_F(luit);
    cat = OGTST_UI_TO_F(auit);
    
    if(strncmp(func, "MODULATE", 8) == 0){
	crv = clt * crf;
	cgv = clt * cgf;
	cbv = clt * cbf;
	cav = cat * caf;
    }
    
    rubv = OGTST_F_TO_UB(crv);
    gubv = OGTST_F_TO_UB(cgv);
    bubv = OGTST_F_TO_UB(cbv);
    aubv = OGTST_F_TO_UB(cav);

    color = ((rubv << 24) & 0xff000000) | ((gubv << 16) & 0x00ff0000) | ((bubv << 8) & 0x0000ff00) | ((aubv << 0) & 0x000000ff);
    return(color);
   
}

/*************************************************************************
*   Function to get a final color value from 2 texture components        *
*   of type GLushort, 4 fragment components and a texturing function     *
*************************************************************************/
GLint ogLib2Us(char *func, GLushort lust, GLushort aust, float crf, float cgf, float cbf, float caf) 
{
    
    GLfloat   clt, crv, cgv, cat, cbv, cav;
    GLubyte   rubv, gubv, bubv, aubv;
    GLint color;
    
    clt = OGTST_US_TO_F(lust);
    cat = OGTST_US_TO_F(aust);
    
    if(strncmp(func, "MODULATE", 8) == 0){
	crv = clt * crf;
	cgv = clt * cgf;
	cbv = clt * cbf;
	cav = cat * caf;
    }
    
    rubv = OGTST_F_TO_UB(crv);
    gubv = OGTST_F_TO_UB(cgv);
    bubv = OGTST_F_TO_UB(cbv);
    aubv = OGTST_F_TO_UB(cav);
    
    color = ((rubv << 24) & 0xff000000) | ((gubv << 16) & 0x00ff0000) | ((bubv << 8) & 0x0000ff00) | ((aubv << 0) & 0x000000ff);
    return(color);
   
}

/*************************************************************************
*   Function to get a final color value from 1 texture component         *
*   of type GLubyte, 4 fragment components and a texturing function      *
*************************************************************************/
GLint ogLib1Ub(char *func, GLubyte lubt, float crf, float cgf, float cbf, float caf) 
{
    
    GLfloat   clt, crv, cgv, cbv, cav;
    GLubyte   rubv, gubv, bubv, aubv;
    GLint color;
    
    clt = OGTST_UB_TO_F(lubt);
    
    if(strncmp(func, "MODULATE", 8) == 0){
	crv = clt * crf;
	cgv = clt * cgf;
	cbv = clt * cbf;
	cav = caf;
    }
    
    rubv = OGTST_F_TO_UB(crv);
    gubv = OGTST_F_TO_UB(cgv);
    bubv = OGTST_F_TO_UB(cbv);
    aubv = OGTST_F_TO_UB(cav);

    color = ((rubv << 24) & 0xff000000) | ((gubv << 16) & 0x00ff0000) | ((bubv << 8) & 0x0000ff00) | ((aubv << 0) & 0x000000ff);
    return(color);
    
}

/*************************************************************************
*   Function to get a final color value from 1 texture component         *
*   of type GLuint, 4 fragment components and a texturing function       *
*************************************************************************/
GLint ogLib1Ui(char *func, GLuint luit, float crf, float cgf, float cbf, float caf) 
{
    
    GLfloat   clt, crv, cgv, cbv, cav;
    GLubyte   rubv, gubv, bubv, aubv;
    GLint color;
    
    clt = OGTST_UI_TO_F(luit);
    
    if(strncmp(func, "MODULATE", 8) == 0){
	crv = clt * crf;
	cgv = clt * cgf;
	cbv = clt * cbf;
	cav = caf;
    }
    
    rubv = OGTST_F_TO_UB(crv);
    gubv = OGTST_F_TO_UB(cgv);
    bubv = OGTST_F_TO_UB(cbv);
    aubv = OGTST_F_TO_UB(cav);

    color = ((rubv << 24) & 0xff000000) | ((gubv << 16) & 0x00ff0000) | ((bubv << 8) & 0x0000ff00) | ((aubv << 0) & 0x000000ff);
    return(color);
    
}

/*************************************************************************
*   Function to get a final color value from 1 texture component         *
*   of type GLushort, 4 fragment components and a texturing function     *
*************************************************************************/
GLint ogLib1Us(char *func, GLushort lust, float crf, float cgf, float cbf, float caf) 
{
    
    GLfloat   clt, crv, cgv, cbv, cav;
    GLubyte   rubv, gubv, bubv, aubv;
    GLint color;
    
    clt = OGTST_US_TO_F(lust);
    
    if(strncmp(func, "MODULATE", 8) == 0){
	crv = clt * crf;
	cgv = clt * cgf;
	cbv = clt * cbf;
	cav = caf;
    }
    
    rubv = OGTST_F_TO_UB(crv);
    gubv = OGTST_F_TO_UB(cgv);
    bubv = OGTST_F_TO_UB(cbv);
    aubv = OGTST_F_TO_UB(cav);

    color = ((rubv << 24) & 0xff000000) | ((gubv << 16) & 0x00ff0000) | ((bubv << 8) & 0x0000ff00) | ((aubv << 0) & 0x000000ff);
    return(color);
 
}
