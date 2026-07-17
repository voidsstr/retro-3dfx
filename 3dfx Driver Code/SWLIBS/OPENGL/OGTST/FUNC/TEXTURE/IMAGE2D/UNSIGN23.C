/*
 * unsign23.c - test unsigned textures: number of source components is 2,
 *		  number of destination components is 3
 */

#include "ogtst.h"       /* include test environment */

#define NUM_TESTS 16

#define BYTE_ONE	0xff
#define BYTE_PATTERN_1	0x11
#define BYTE_PATTERN_2	0x44
#define BYTE_PATTERN_3	0xaa
#define BYTE_PATTERN_4	0xdd
#define BYTE_ALPHA_1	0xdd
#define BYTE_ALPHA_2    0xaa
#define BYTE_ALPHA_3    0x44
#define BYTE_ALPHA_4    0x11

#define SHORT_ONE	0xffff
#define SHORT_PATTERN_1 0x2222
#define SHORT_PATTERN_2 0x5555
#define SHORT_PATTERN_3 0xbbbb
#define SHORT_PATTERN_4 0xeeee
#define SHORT_ALPHA_1   0xeeee
#define SHORT_ALPHA_2   0xbbbb
#define SHORT_ALPHA_3   0x5555
#define SHORT_ALPHA_4   0x2222

#define INT_ONE		0xffffffff
#define INT_PATTERN_1	0x33333333
#define INT_PATTERN_2	0x66666666
#define INT_PATTERN_3	0xcccccccc
#define INT_PATTERN_4	0xffffffff
#define INT_ALPHA_1	0xffffffff
#define INT_ALPHA_2	0xcccccccc
#define INT_ALPHA_3	0x66666666
#define INT_ALPHA_4	0x33333333


#define COMPON	2
#define TEXSIZE	8
#define TEXSIZE_4	(TEXSIZE >> 2)

/*ARGSUSED*/
TESTMOD(unsign23)
{
    GLint width, height, doDlist, flag, i, j, k;

    GLubyte tubyte[TEXSIZE * TEXSIZE * COMPON], byte_pattern, byte_alpha;
    GLushort tushort[TEXSIZE * TEXSIZE * COMPON], short_pattern, short_alpha;
    GLuint tuint[TEXSIZE * TEXSIZE * COMPON], int_pattern, int_alpha;

	/* init texture */

        /* unsigned byte */
    k = 0;
    for (i = 0; i <TEXSIZE; i ++){
        for(j = 0; j < TEXSIZE; j++){
	    if ( i % 2 ){
		byte_pattern = (j % 2) ? BYTE_PATTERN_1 : BYTE_PATTERN_2;
        	byte_alpha = (j % 2) ? BYTE_ALPHA_1 : BYTE_ALPHA_2;
	    }else{
		byte_pattern = (j % 2) ? BYTE_PATTERN_3 : BYTE_PATTERN_4;
        	byte_alpha = (j % 2) ? BYTE_ALPHA_3 : BYTE_ALPHA_4;
	    }
            tubyte[k] = byte_pattern;
            tubyte[k + 1] = byte_alpha;
	    k += COMPON;
        }
    }

        /* unsigned short */
    k = 0;
    for (i = 0; i <TEXSIZE; i ++){
        for(j = 0; j < TEXSIZE; j++){
	    if ( i % 2 ){
		short_pattern = (j % 2) ? SHORT_PATTERN_1 : SHORT_PATTERN_2;
        	short_alpha = (j % 2) ? SHORT_ALPHA_1 : SHORT_ALPHA_2;
	    }else{
		short_pattern = (j % 2) ? SHORT_PATTERN_3 : SHORT_PATTERN_4;
        	short_alpha = (j % 2) ? SHORT_ALPHA_3 : SHORT_ALPHA_4;
	    }
            tushort[k] = short_pattern;
            tushort[k + 1] = short_alpha;
	    k += COMPON;
        }
    }

        /* unsigned int */
    k = 0;
    for (i = 0; i <TEXSIZE; i ++){
        for(j = 0; j < TEXSIZE; j++){
	    if ( i % 2 ){
		int_pattern = (j % 2) ? INT_PATTERN_1 : INT_PATTERN_2;
        	int_alpha = (j % 2) ? INT_ALPHA_1 : INT_ALPHA_2;
	    }else{
		int_pattern = (j % 2) ? INT_PATTERN_3 : INT_PATTERN_4;
        	int_alpha = (j % 2) ? INT_ALPHA_3 : INT_ALPHA_4;
	    }
            tuint[k] = int_pattern;
            tuint[k + 1] = int_alpha;
	    k += COMPON;
        }
    }

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    width = ogEnvQuery(OG_XWSIZE) - 1;
    height = ogEnvQuery(OG_YWSIZE) - 1;
    glOrtho(-0.5, width + 0.5, -0.5, height + 0.5, 0.01, 1000.0);

    glClearColor(0.25, 0.25, 0.25, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); 
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -10.0);

    glColor3f(1.0, 1.0, 1.0);

    for (i = 0, j = 0; i < (TEXSIZE*TEXSIZE*COMPON); i = i + COMPON, j++){
        ogEnvLog(5, "ubyte(%i): 0x%8x\n", j, ogLib3Ub("DECAL", tubyte[i], tubyte[i], tubyte[i], 1.0, 1.0, 1.0, 1.0));
    }
    for (i = 0, j = 0; i < (TEXSIZE*TEXSIZE*COMPON); i = i + COMPON, j++){
        ogEnvLog(5, "ushort(%i): 0x%8x\n", j, ogLib3Us("DECAL", tushort[i], tushort[i], tushort[i], 1.0, 1.0, 1.0, 1.0));
    }
    for (i = 0, j = 0; i < (TEXSIZE*TEXSIZE*COMPON); i = i + COMPON, j++){
        ogEnvLog(5, "uint(%i): 0x%8x\n", j, ogLib3Ui("DECAL", tuint[i], tuint[i], tuint[i], 1.0, 1.0, 1.0, 1.0));
    }
    
    for (i = 0; i != NUM_TESTS; ){
	flag = (i > (NUM_TESTS >> 1)) ? 1 : 0;
        doDlist = (i + flag) % 2;

        if (doDlist)
            glNewList(i + 1, GL_COMPILE);

	switch( i % 8 ){
	  case 0:
	  case 6:
	    glTexImage2D(GL_TEXTURE_2D, 0, 3, TEXSIZE, TEXSIZE, 
			0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, tubyte);
	    break;

	  case 1:
	  case 4:
	    glTexImage2D(GL_TEXTURE_2D, 0, 3, TEXSIZE, TEXSIZE, 
			0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_SHORT, tushort);
	    break;

	  case 2:
	  case 5:
	    glTexImage2D(GL_TEXTURE_2D, 0, 3, TEXSIZE, TEXSIZE, 
				0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_INT, tuint);
	    break;
	  default:
	    glTexImage2D(GL_TEXTURE_2D, 0, 3, 0, TEXSIZE, 
				0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_INT, tuint);
	    break;
	}

	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0);
        glVertex2f(10.0, 10.0); 

	glTexCoord2f(1.0, 0.0);
        glVertex2f(100.0, 10.0); 

	glTexCoord2f(1.0, 1.0);
        glVertex2f(100.0, 100.0);

	glTexCoord2f(0.0, 1.0);
        glVertex2f(10.0, 100.0); 
	glEnd();

	if (doDlist){
            glEndList();
            glCallList(i + 1);
        }

	i ++;

	if ((i % 4) == 0)
	    glTranslatef(-360.0, 120.0, 0.0);
	else
	    glTranslatef(120.0, 0, 0.0);

    }

}

CLEANUP(unsign23)
{
    glDisable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                                GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    ogLibSetDefaultTextures();

    glDeleteLists(1, NUM_TESTS);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0, 0.0, 0.0, 0.0);
}
