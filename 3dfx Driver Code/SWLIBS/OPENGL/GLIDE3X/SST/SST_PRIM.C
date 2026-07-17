
/*
** $Revision: 2$
** $Date: 10/11/00 7:58:03 PM$
*/
#include "context.h"
#include "global.h"
#include "g_imfncs.h"

void __glSSTEarlyInitPrimitiveState(__GLcontext *gc) {
    SSTPrimState *ps = &gc->primState;
    if (ps->vBuf == NULL) {
        GLuint i;
        char *c;
        if ((c = getenv("__GL_VERTEX_BUFFER_SIZE")) != NULL) {
	    ps->vBufSize = atoi(c);
        } else {
            ps->vBufSize = MAX_VERTEX_BUFFER_SIZE;
        }

        ps->vBuf = (IVert*) gc->imports.malloc(gc,
					       ps->vBufSize * sizeof (IVert));

        ps->vList = (IVert**) gc->imports.malloc(gc, 2 * ps->vBufSize *
						    sizeof (IVert*));

        ps->nextTable = (GLuint *) gc->imports.malloc(gc, (3+ps->vBufSize)*
						      sizeof(GLuint));
        ps->nextTable[0] = ps->vBufSize-2;
        ps->nextTable[1] = ps->vBufSize-1;
        ps->nextTable[ps->vBufSize-1] = 0;
        ps->nextWritePointer = &ps->nextTable[3];
        ps->nextReadPointer = &ps->nextTable[0];

        for (i = 0; i < ps->vBufSize; i++) {
	    ps->vList[i] = ps->vList[i+ps->vBufSize] = &ps->vBuf[i];
            ps->nextTable[i+2] = i;
	}
    }
}

