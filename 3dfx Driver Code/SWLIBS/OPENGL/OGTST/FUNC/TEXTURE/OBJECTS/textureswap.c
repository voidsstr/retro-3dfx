#include <stdio.h>
#include "ogtst.h"
#include "GL/glu.h"

#ifdef GL_VERSION_1_1

#if 1
static int comps = 4;
static int intfmt = GL_RGBA8;
static int hstfmt = GL_RGBA;
#else
static int comps = 1;
static int intfmt = GL_LUMINANCE8;
static int hstfmt = GL_LUMINANCE;
#endif

static int totsize = 0;

static void checkGLError(void) 
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
	ogEnvLog(OG_LFAIL,"GL Error occurred:  %s (0x%04x)\n",
                 gluErrorString(err), err);
    }
}

static void check_texture(int min, int width, int height)
{
    unsigned char *buf;
    unsigned char *rbuf;
    int ii, i, j, k, ind = 0, size, lod, end;

    size = comps*width*height*sizeof(unsigned char);

    buf = (unsigned char *)ogLibMalloc(size);
    rbuf = (unsigned char *)ogLibMalloc(size);
    bzero(rbuf,size);

    end = min==GL_LINEAR ? width : 1;

    for( lod=0,ii=width; ii>=end; ii>>=1,lod++ ) {
	ind = 0;
        for( j=0; j<ii; j++ )
	    for( i=0; i<ii; i++ )
	        for( k=0; k<comps; k++ )
	                buf[ind++] = (0xff&i+0xff&(0xffff-j))&0xff;

        glGetTexImage(GL_TEXTURE_2D, lod, hstfmt, GL_UNSIGNED_BYTE, rbuf);

        if( bcmp(buf,rbuf,comps*ii*ii*sizeof(unsigned char)) ) {
	    ogEnvLog(OG_LFAIL,"Readback Failed (lod %d width %d)\n",lod,width);
#if 0
	    for( i=0; i<comps*ii*ii; i++ ) {
	        if( buf[i]!=rbuf[i] ) {
		}
	    }
#endif
        }
    }
    ogLibFree(buf);
    ogLibFree(rbuf);
}

static void init_texture(int min, int width, int height)
{
    unsigned char *buf;
    int ii, i, j, k, ind = 0, size, lod, end;

    size = comps*width*height*sizeof(unsigned char);

    buf = (unsigned char *)ogLibMalloc(size);

    end = min==GL_LINEAR ? width : 1;

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);

    for( lod=0,ii=width; ii>=end; ii>>=1,lod++ ) {
	ind = 0;
        for( j=0; j<ii; j++ )
	    for( i=0; i<ii; i++ )
	        for( k=0; k<comps; k++ )
	                buf[ind++] = (0xff&i+0xff&(0xffff-j))&0xff;

	totsize += ind;

        glTexImage2D(GL_TEXTURE_2D, lod, intfmt, ii, ii, 0, 
                     hstfmt, GL_UNSIGNED_BYTE, buf);
    }
    ogLibFree(buf);
}

static void draw(int width, int height)
{
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0);
    glVertex2f(0, 0);
    glTexCoord2f(1, 0);
    glVertex2f(width, 0);
    glTexCoord2f(0, 1);
    glVertex2f(0, height);
    glTexCoord2f(1, 1);
    glVertex2f(width, height);
    glEnd();
}

static void dotest(int seed, int numtex, int width, int height)
{
    unsigned int texids[1024];
    int i, w;

    glGenTextures(numtex,texids);

    ogLibSetSeed(seed);

    for( i=0; i<numtex; i++ ) {
        glBindTexture(GL_TEXTURE_2D, texids[i]);
	w = width>>(ogLibBitRand(2));
        init_texture(GL_LINEAR_MIPMAP_LINEAR,w,w);
        checkGLError();
#if 0
	draw(width,height);
	glFlush();
        check_texture(GL_LINEAR_MIPMAP_LINEAR,w,w);
        checkGLError();
#endif
    }

    ogLibSetSeed(seed);

    for( i=0; i<numtex; i++ ) {
        glBindTexture(GL_TEXTURE_2D, texids[i]);
	w = width>>(ogLibBitRand(2));
	draw(width,height);
	glFlush();
        check_texture(GL_LINEAR_MIPMAP_LINEAR,w,w);
        checkGLError();
    }

    glDeleteTextures(numtex,texids);
}
#endif

TESTMOD(textureswap)
{
#ifdef GL_VERSION_1_1 
    if (IS_ONEONE()) {

	/* default to 16mb boards */
	int tex_size = 0;
	int numtex = 200;
	int width = 256;
	int height = 256;

        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tex_size);

        /* hack:  64mb returns 2048 */
        if( tex_size==2048 ) {
	    numtex = 200;
	    width = 512; 
	    height = 512;
        }

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_TEXTURE_2D);

	ogEnvLog(1,"texsize %d numtex %d width %d\n",tex_size,numtex,width);

        while (pass--) {
	    totsize = 0;
            dotest(pass,numtex,width,height);
	    ogEnvLog(1,"Pass %d:  total bytes downloaded:  %d\n",pass,totsize);
        }

	glDisable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
    }
#endif
}

CLEANUP(textureswap)
{
#ifdef GL_VERSION_1_1 
    glViewport(0, 0, ogEnvQuery(OG_XWSIZE), ogEnvQuery(OG_YWSIZE));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    glTexCoord4f(0, 0, 0, 1);
#endif
}
