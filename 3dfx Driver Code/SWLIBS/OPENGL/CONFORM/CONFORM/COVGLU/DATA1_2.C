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

#include "shell.h"


enumTestRec enum_TessProperty[] = {
    GLU_TESS_BOUNDARY_ONLY, "GLU_TESS_BOUNDARY_ONLY",
    GLU_TESS_TOLERANCE, "GLU_TESS_TOLERANCE",
    GLU_TESS_WINDING_RULE, "GLU_TESS_WINDING_RULE",
    -1, "End of List"
};

enumTestRec enum_TessWindingRule[] = {
    GLU_TESS_WINDING_ODD, "GLU_TESS_WINDING_ODD", 
    GLU_TESS_WINDING_NONZERO, "GLU_TESS_WINDING_NONZERO",
    GLU_TESS_WINDING_POSITIVE, "GLU_TESS_WINDING_POSITIVE",
    GLU_TESS_WINDING_NEGATIVE, "GLU_TESS_WINDING_NEGATIVE",
    GLU_TESS_WINDING_ABS_GEQ_TWO, "GLU_TESS_WINDING_ABS_GEQ_TWO",
    -1, "End of List"
};
