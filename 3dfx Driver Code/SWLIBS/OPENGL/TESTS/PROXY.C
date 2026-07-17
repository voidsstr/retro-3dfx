#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include <windows.h>
#include <glide.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "winutil.h"

HWND hWndMain;
HDC hDC;

#define CHKERROR() \
{ \
    int e = glGetError(); \
    if (e) printf("got error %d at line %d\n", e, __LINE__); \
}

static int
logOg(int i) {
    int n;

    for (n=0; i; n++) {
	i >>= 1;
    }
    return n;
}

static int
proxy(int w, int h) {
    int retw, reth;
    
    glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,
		 GL_UNSIGNED_BYTE, NULL);
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,
			     &retw);
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,
			     &reth);
    return (retw != 0 && reth != 0);
}


static void
test(void) {
    unsigned int buf[64];
    int max, err;
    int w, h;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
    printf("max texture size %d\n", max);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, max*2, 4, 0, GL_RGBA,
		 GL_UNSIGNED_BYTE, buf);
    err = glGetError();
    if (err == GL_INVALID_VALUE) {
	printf("got the correct error from texture too large\n");
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, max*2, 0, GL_RGBA,
		 GL_UNSIGNED_BYTE, buf);
    err = glGetError();
    if (err == GL_INVALID_VALUE) {
	printf("got the correct error from texture too large\n");
    }

    for (w=1; w <= max*2; w <<= 1) {
	for (h=1; h <= max*2; h <<= 1) {
	    printf("%3d x %3d: %s\n", w, h, proxy(w,h) ? "yes" : "no");
	}
    }
}

void main( int argc, char **argv) {
    int getkey(void);

    initApplication(GetModuleHandle(NULL), 0, FALSE);

    test();
    while (!getkey());
    return;
}
