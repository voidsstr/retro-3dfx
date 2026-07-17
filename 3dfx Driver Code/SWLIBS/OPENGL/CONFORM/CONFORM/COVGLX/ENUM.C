/*
** Copyright 1997, Silicon Graphics, Inc.
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
*/

#include <stdio.h>
#include <stdlib.h>
#include "shell.h"


static struct EnumCheckRec {
    char *name;
    GLenum value;
    GLenum true;
} enumCheck[] = {
    {
	"GLX_ACCUM_ALPHA_SIZE", GLX_ACCUM_ALPHA_SIZE, 17
    },
    {
	"GLX_ACCUM_BLUE_SIZE", GLX_ACCUM_BLUE_SIZE, 16
    },
    {
	"GLX_ACCUM_GREEN_SIZE", GLX_ACCUM_GREEN_SIZE, 15
    },
    {
	"GLX_ACCUM_RED_SIZE", GLX_ACCUM_RED_SIZE, 14
    },
    {
	"GLX_ALPHA_SIZE", GLX_ALPHA_SIZE, 11
    },
    {
	"GLX_AUX_BUFFERS", GLX_AUX_BUFFERS, 7
    },
    {
	"GLX_BAD_ATTRIBUTE", GLX_BAD_ATTRIBUTE, 2
    },
    {
	"GLX_BAD_SCREEN", GLX_BAD_SCREEN, 1
    },
    {
	"GLX_BAD_VISUAL", GLX_BAD_VISUAL, 4
    },
    {
	"GLX_BLUE_SIZE", GLX_BLUE_SIZE, 10
    },
    {
	"GLX_BUFFER_SIZE", GLX_BUFFER_SIZE, 2
    },
    {
	"GLX_DEPTH_SIZE", GLX_DEPTH_SIZE, 12
    },
    {
	"GLX_DOUBLEBUFFER", GLX_DOUBLEBUFFER, 5
    },
    {
	"GLX_GREEN_SIZE", GLX_GREEN_SIZE, 9
    },
    {
	"GLX_LEVEL", GLX_LEVEL, 3
    },
    {
	"GLX_NO_EXTENSION", GLX_NO_EXTENSION, 3
    },
    {
	"GLX_RED_SIZE", GLX_RED_SIZE, 8
    },
    {
	"GLX_RGBA", GLX_RGBA, 4
    },
    {
	"GLX_STEREO", GLX_STEREO, 6
    },
    {
	"GLX_STENCIL_SIZE", GLX_STENCIL_SIZE, 13
    },
    {
	"GLX_USE_GL", GLX_USE_GL, 1
    },
};


void VerifyEnums(void)
{
    struct EnumCheckRec *p, *end;

    p = enumCheck;
    end = p + (sizeof(enumCheck) / sizeof(struct EnumCheckRec));

    Output("Enumeration check.\n");
    while (p < end) {
	Output("\t%s (%d) = %d.\n", p->name, p->true, p->value);
	if (p->value != p->true) {
	    printf("covglx failed.\n\n");
	    exit(1);
	}    
	p++;
    }
    Output("\n");
}
