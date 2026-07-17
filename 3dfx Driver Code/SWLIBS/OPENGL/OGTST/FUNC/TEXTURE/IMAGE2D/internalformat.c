#include "ogtst.h"       /* include test environment */

#define BYTE_ONE 0xff
#define BYTE_ALPHA_1	0x33
#define BYTE_ALPHA_2	0xaa
#define BYTE_ALPHA_3	0xcc
#define BYTE_ALPHA_4	0xff

#define COMPON	4
#define TEXSIZE	4
#define TEXSIZE_4	(TEXSIZE >> 2)

static GLint InternalFormats[] = 
{
1,
2,
3,
4,
/* EXT_texture */
GL_ALPHA4,
GL_ALPHA8,
GL_ALPHA12,
GL_ALPHA16,
GL_LUMINANCE4,
GL_LUMINANCE8,
GL_LUMINANCE12,
GL_LUMINANCE16,
GL_LUMINANCE4_ALPHA4,
GL_LUMINANCE6_ALPHA2,
GL_LUMINANCE8_ALPHA8,
GL_LUMINANCE12_ALPHA4,
GL_LUMINANCE12_ALPHA12,
GL_LUMINANCE16_ALPHA16,
GL_INTENSITY,
GL_INTENSITY4,
GL_INTENSITY8,
GL_INTENSITY12,
GL_INTENSITY16,
GL_RGB4,
GL_RGB4,
GL_RGB5,
GL_RGB8,
GL_RGB10,
GL_RGB12,
GL_RGB16,
GL_RGBA2,
GL_RGBA4,
GL_RGB5_A1,
GL_RGBA8,
GL_RGB10_A2,
GL_RGBA12,
GL_RGBA16,
};

/*ARGSUSED*/
TESTMOD(internalformat)
{

    GLint i, j,width,height;

    GLubyte tubyte[TEXSIZE * TEXSIZE * COMPON];

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    width = ogEnvQuery(OG_XWSIZE) - 1;
    height = ogEnvQuery(OG_YWSIZE) - 1;
    glOrtho(-0.5, width + 0.5, -0.5, height + 0.5, 0.01, 1000.0);

/*  RE does not support alpha textures */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0.25, 0.25, 0.25, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

	/* unsigned byte */
    for (i = 0; i <TEXSIZE; i ++){
      for(j=0; j<TEXSIZE; j++){
        if (((j + i*TEXSIZE_4) % TEXSIZE) < TEXSIZE_4){
           tubyte[i*TEXSIZE*COMPON + j*COMPON] = BYTE_ONE;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 1] = BYTE_ONE;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 2] = BYTE_ONE;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 3] = BYTE_ALPHA_1;
        }else if (((j + i*TEXSIZE_4) % TEXSIZE) < (TEXSIZE_4*2)){
           tubyte[i*TEXSIZE*COMPON + j*COMPON] = BYTE_ONE;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 1] = 0;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 2] = 0;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 3] = BYTE_ALPHA_2;
        }else if (((j + i*TEXSIZE_4) % TEXSIZE) < (TEXSIZE_4*3)){
           tubyte[i*TEXSIZE*COMPON + j*COMPON] = 0;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 1] = BYTE_ONE;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 2] = 0;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 3] = BYTE_ALPHA_3;
        }else{
           tubyte[i*TEXSIZE*COMPON + j*COMPON] = 0;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 1] = 0;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 2] = BYTE_ONE;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 3] = BYTE_ALPHA_4;
        }
      }
    }


    glColor3f(1.0, 1.0, 1.0);
    glLoadIdentity();
    glTranslatef(0, 0, -10.0);

    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); 

    for (i = 0; i != sizeof(InternalFormats)/sizeof(GLint); ){
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormats[i], TEXSIZE, TEXSIZE, 0, 
				       GL_RGBA, GL_UNSIGNED_BYTE, tubyte);

        glBegin(GL_POLYGON);
	   glTexCoord2f(0.0, 0.0);
	   glVertex2f(10.0, 10.0); 

	   glTexCoord2f(1.0, 0.0);
	   glVertex2f(80.0, 10.0); 

	   glTexCoord2f(1.0, 1.0);
	   glVertex2f(80.0, 80.0);

	   glTexCoord2f(0.0, 1.0);
	   glVertex2f(10.0, 80.0); 
        glEnd();
        glTranslatef(80.0, 0, 0.0);
	i ++;

	if ((i % 7) == 0) glTranslatef(-7.*80., 80.0, -10.0);

    }

}

CLEANUP(internalformat)
{
    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_BLEND);

    glDisable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                                GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    ogLibSetDefaultTextures();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0, 0.0, 0.0, 0.0);
}

