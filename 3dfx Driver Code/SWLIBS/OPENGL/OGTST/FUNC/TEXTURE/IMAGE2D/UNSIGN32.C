/*
 * unsign32.c - test unsigned textures: number of source components is 3,
 *		  number of destination components is 2
 */

#include "ogtst.h"       /* include test environment */

#define NUM_TESTS 16

#define BYTE_ONE	0xff
#define BYTE_PATTERN_1	0x11
#define BYTE_PATTERN_2	0x44
#define BYTE_PATTERN_3	0xaa
#define BYTE_PATTERN_4	0xdd

#define SHORT_ONE	0xffff
#define SHORT_PATTERN_1 0x2222
#define SHORT_PATTERN_2 0x5555
#define SHORT_PATTERN_3 0xbbbb
#define SHORT_PATTERN_4 0xeeee

#define INT_ONE		0xffffffff
#define INT_PATTERN_1	0x33333333
#define INT_PATTERN_2	0x66666666
#define INT_PATTERN_3	0xcccccccc
#define INT_PATTERN_4	0xffffffff


#define COMPON	3
#define TEXSIZE	8
#define TEXSIZE_4	(TEXSIZE >> 2)

/*ARGSUSED*/
TESTMOD(unsign32)
{
    GLint width, height, doDlist, flag, i, j, k;

    GLubyte tubyte[TEXSIZE * TEXSIZE * COMPON], byte_pattern;
    GLushort tushort[TEXSIZE * TEXSIZE * COMPON], short_pattern;
    GLuint tuint[TEXSIZE * TEXSIZE * COMPON], int_pattern;

	/* init texture */

        /* unsigned byte */
    k = 0;
    for (i = 0; i <TEXSIZE; i ++){
        for(j = 0; j < TEXSIZE; j++){
	    if ( i % 2 )
		byte_pattern = (j % 2) ? BYTE_PATTERN_1 : BYTE_PATTERN_2;
	    else
		byte_pattern = (j % 2) ? BYTE_PATTERN_3 : BYTE_PATTERN_4;
	
            tubyte[k] = byte_pattern;
            tubyte[k + 1] = BYTE_ONE;
            tubyte[k + 2] = ~byte_pattern;
	    k += COMPON;
        }
    }

        /* unsigned short */
    k = 0;
    for (i = 0; i <TEXSIZE; i ++){
        for(j = 0; j < TEXSIZE; j++){
	    if ( i % 2 )
		short_pattern = (j % 2) ? SHORT_PATTERN_1 : SHORT_PATTERN_2;
	    else
		short_pattern = (j % 2) ? SHORT_PATTERN_3 : SHORT_PATTERN_4;
	    
            tushort[k] = short_pattern;
            tushort[k + 1] = ~short_pattern;
            tushort[k + 2] = SHORT_ONE;
	    k += COMPON;
        }
    }

        /* unsigned int */
    k = 0;
    for (i = 0; i <TEXSIZE; i ++){
        for(j = 0; j < TEXSIZE; j++){
	    if ( i % 2 )
		int_pattern = (j % 2) ? INT_PATTERN_1 : INT_PATTERN_2;
	    else
		int_pattern = (j % 2) ? INT_PATTERN_3 : INT_PATTERN_4;
	    
            tuint[k] = int_pattern;
            tuint[k + 1] = INT_ONE;
            tuint[k + 2] = ~int_pattern;
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

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); 

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -10.0);

    glColor3f(1.0, 1.0, 1.0);

    for (i = 0, j = 0; i < (TEXSIZE*TEXSIZE*COMPON); i = i + COMPON, j++){
        ogEnvLog(5, "ubyte(%i): 0x%8x\n", j, ogLib2Ub("MODULATE", tubyte[i], 0xff, 1.0, 1.0, 1.0, 1.0));
    }
    for (i = 0, j = 0; i < (TEXSIZE*TEXSIZE*COMPON); i = i + COMPON, j++){
        ogEnvLog(5, "ushort(%i): 0x%8x\n", j, ogLib2Us("MODULATE", tushort[i], 0xffff, 1.0, 1.0, 1.0, 1.0));
    }
    for (i = 0, j = 0; i < (TEXSIZE*TEXSIZE*COMPON); i = i + COMPON, j++){
        ogEnvLog(5, "uint(%i): 0x%8x\n", j, ogLib2Ui("MODULATE", tuint[i], 0xffffffff, 1.0, 1.0, 1.0, 1.0));
    }
    
    for (i = 0; i != NUM_TESTS; ){
	flag = (i > (NUM_TESTS >> 1)) ? 1 : 0;
        doDlist = (i + flag) % 2;

        if (doDlist)
            glNewList(1, GL_COMPILE);

	switch( i % 8 ){
	  case 0:
	  case 6:
	    glTexImage2D(GL_TEXTURE_2D, 0, 2, TEXSIZE, 
				TEXSIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, tubyte);
	    break;

	  case 1:
	  case 4:
	    glTexImage2D(GL_TEXTURE_2D, 0, 2, TEXSIZE, 
			TEXSIZE, 0, GL_RGB, GL_UNSIGNED_SHORT, tushort);
	    break;

	  case 2:
	  case 5:
	    glTexImage2D(GL_TEXTURE_2D, 0, 2, TEXSIZE, 
				TEXSIZE, 0, GL_RGB, GL_UNSIGNED_INT, tuint);
	    break;
	  default:
	    glTexImage2D(GL_TEXTURE_2D, 0, 2, 0, TEXSIZE, 
					0, GL_RGB, GL_UNSIGNED_INT, tuint);
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
            glCallList(1);
        }

	i ++;

	if ((i % 4) == 0)
	    glTranslatef(-360.0, 120.0, 0.0);
	else
	    glTranslatef(120.0, 0, 0.0);

    }

}

CLEANUP(unsign32)
{
    glDisable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                                GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    ogLibSetDefaultTextures();

    glDeleteLists(1, 1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
