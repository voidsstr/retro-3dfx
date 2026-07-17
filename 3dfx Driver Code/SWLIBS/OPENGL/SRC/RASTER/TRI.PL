#!/usr/sbin/perl -P
#if 0 /* Perl cannot handle C-style comments */
/*
** Copyright 1996, 1997, Silicon Graphics, Inc.
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
**
*/
#endif

##########################################################################
# Declarations
##########################################################################

#include "fr_modes.h"

#define fr_INTERPOLANTS	(($mode & (__FR_SETUP_DEPTH|__FR_SETUP_SMOOTH|__FR_SETUP_TEXTURE)) != 0)

#define fr_CI_COLOR	  	($fr_RGB == 0)
#define fr_RGB_COLOR	  	($fr_RGB == 1)
#define fr_RGBA_COLOR	  	(fr_RGB_COLOR && ($mode & __FR_SETUP_ALPHA))

#define fr_FLAT_SHADING		(($mode & __FR_SETUP_SMOOTH) == 0)
#define fr_SMOOTH_SHADING	(($mode & __FR_SETUP_SMOOTH) != 0)

#define fr_DITHER		(($mode & __FR_SETUP_DITHER) != 0)

#define fr_TEXTURING			(($mode & __FR_SETUP_TEXTURE) != 0)
#define fr_PERSPECTIVE			(($mode & __FR_SETUP_PERSPECTIVE) != 0)
#define fr_AFFINE_TEXTURING		(fr_TEXTURING && !fr_PERSPECTIVE)
#define fr_PERSPECTIVE_TEXTURING	(fr_TEXTURING && fr_PERSPECTIVE)

#define fr_DEPTH_BUFFER			(($mode & __FR_SETUP_DEPTH) != 0)
#define fr_STENCIL_BUFFER		(($mode & __FR_SETUP_STENCIL) != 0)

#define TRI_RASTERIZER_RETURN_TYPE	"void"
#define TRI_RASTERIZER_CALLING_CONV	""
#define TRI_RASTERIZER_NAME		"__glRenderTriangle"
#define TRI_RASTERIZER_SIGNATURE	"(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2)"
#define TRI_RASTERIZER_TABLE_NAME_RGB	"__fr_rgb_tri_rast_table"
#define TRI_RASTERIZER_TABLE_NAME_CI	"__fr_ci_tri_rast_table"

##########################################################################
# Global Variables
##########################################################################

##########################################################################
# Subroutines
##########################################################################

sub my_open {
    local($fd) = $_[0];
    local($name) = $_[1];
    open($fd, $name) || die "Can't open $name: $!, stopped";
}

sub SetDefines {
    local($fh) = $_[0];
    local($mode) = $_[1];
    local($prefix) = $_[2];

    printf($fh "%s#define FR_INTERPOLANTS %d\n", $prefix, fr_INTERPOLANTS);
    printf($fh "%s#define FR_CI_COLOR %d\n", $prefix, fr_CI_COLOR);
    printf($fh "%s#define FR_RGB_COLOR %d\n", $prefix, fr_RGB_COLOR);
    printf($fh "%s#define FR_RGBA_COLOR %d\n", $prefix, fr_RGBA_COLOR);
    printf($fh "%s#define FR_SMOOTH_SHADING %d\n", $prefix, fr_SMOOTH_SHADING);
    printf($fh "%s#define FR_FLAT_SHADING %d\n", $prefix, fr_FLAT_SHADING);
    printf($fh "%s#define FR_TEXTURING %d\n", $prefix, fr_TEXTURING);
    printf($fh "%s#define FR_AFFINE_TEXTURING %d\n", $prefix,
	   fr_AFFINE_TEXTURING);
    printf($fh "%s#define FR_PERSPECTIVE_TEXTURING %d\n", $prefix,
	   fr_PERSPECTIVE_TEXTURING);
    printf($fh "%s#define FR_DEPTH_BUFFER %d\n", $prefix, fr_DEPTH_BUFFER);
    printf($fh "%s#define FR_STENCIL_BUFFER %d\n", $prefix, fr_STENCIL_BUFFER);
    printf($fh "%s#define FR_DITHER %d\n", $prefix, fr_DITHER);
    if (fr_DITHER) {
	printf($fh "%s#define FR_COLOR_ROUND COLOR_ROUND_DITHER\n",
	       $prefix);
    } else {
	printf($fh "%s#define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER\n",
	       $prefix);
    }
}

sub UnsetDefines {
    local($fh) = $_[0];
    printf($fh "#undef FR_INTERPOLANTS\n");
    printf($fh "#undef FR_CI_COLOR\n");
    printf($fh "#undef FR_RGB_COLOR\n");
    printf($fh "#undef FR_RGBA_COLOR\n");
    printf($fh "#undef FR_SMOOTH_SHADING\n");
    printf($fh "#undef FR_FLAT_SHADING\n");
    printf($fh "#undef FR_TEXTURING\n");
    printf($fh "#undef FR_AFFINE_TEXTURING\n");
    printf($fh "#undef FR_PERSPECTIVE_TEXTURING\n");
    printf($fh "#undef FR_DEPTH_BUFFER\n");
    printf($fh "#undef FR_STENCIL_BUFFER\n");
    printf($fh "#undef FR_DITHER\n");
    printf($fh "#undef FR_COLOR_ROUND\n");
}

sub gen_TriProc {
    local($fh) = $_[0];
    local($name) = $_[1];

    printf($fh "#define FR_TRI_RASTERIZER_PROTOTYPE ");
    printf($fh "%s %s %s%s\n", TRI_RASTERIZER_RETURN_TYPE,
	   TRI_RASTERIZER_CALLING_CONV, $name, TRI_RASTERIZER_SIGNATURE);
    printf($fh "#include \"tri.inc\"\n");
    printf($fh "#undef FR_TRI_RASTERIZER_PROTOTYPE\n");
}

sub TemplateInstance {
    local($procs) = $_[0];
    local($table) = $_[1];
    local($mode) = $_[2];
    local($name) = sprintf("%s%s_%X", TRI_RASTERIZER_NAME,
			   $fr_RGB ? "RGB" : "CI", $mode);

    do SetDefines($procs, $mode, "");
    printf $procs "/*\n";
    do SetDefines($procs, $mode, " * ");
    printf $procs " */\n";

    do gen_TriProc($procs, $name);

    do UnsetDefines($procs);

    printf $table "    %s,\n",   $name;
}

sub NullInstance {
    local($fh) = $_[0];
    printf $fh "    0,\n";
}

sub init_Table {
    local($fh) = $_[0];
    local($ret_type) = $_[1];
    local($call_conv) = $_[2];
    local($name) = $_[3];
    local($count) = $_[4];
    local($sig) = $_[5];

    printf($fh "%s (%s *%s[%d])%s =\n{\n", $ret_type, $call_conv, $name,
	   $count, $sig);
}

sub end_Table {
    local($fh) = $_[0];
    printf($fh "};\n");
}

sub proto_Table {
    local($fh) = $_[0];
    local($ret_type) = $_[1];
    local($call_conv) = $_[2];
    local($name) = $_[3];
    local($count) = $_[4];
    local($sig) = $_[5];

    printf($fh "extern %s (%s *%s[%d])%s;\n", $ret_type, $call_conv, $name,
	   $count, $sig);
}

sub isSupportedMode {
    local($mode) = $_[0];

    if (fr_PERSPECTIVE && !fr_TEXTURING) {
	return 0;
    }

    return 1;
}

##########################################################################
# Begin code generation
##########################################################################

if ($fr_RGB == 1) {
    do my_open(RGBPROCS, ">rgbtripr.tmp");
    do my_open(RGBTABLE, ">rgbtrit1.tmp");
    do my_open(RGBHDR,   ">g_rgbtri.h");

    do init_Table(RGBTABLE, TRI_RASTERIZER_RETURN_TYPE,
		  TRI_RASTERIZER_CALLING_CONV, TRI_RASTERIZER_TABLE_NAME_RGB,
		  __FR_SETUP_TABLE_SIZE_RGB, TRI_RASTERIZER_SIGNATURE);

    for ($ii = 0; $ii < __FR_SETUP_TABLE_SIZE_RGB; $ii++) {
	if (do isSupportedMode($ii)) {
	    do TemplateInstance(RGBPROCS, RGBTABLE, $ii);
	} else {
	    do NullInstance(RGBTABLE);
	}
    }

    do end_Table(RGBTABLE);

    # Write header file
    do proto_Table(RGBHDR, TRI_RASTERIZER_RETURN_TYPE,
		   TRI_RASTERIZER_CALLING_CONV, TRI_RASTERIZER_TABLE_NAME_RGB,
		   __FR_SETUP_TABLE_SIZE_RGB, TRI_RASTERIZER_SIGNATURE);

} else {

    do my_open(CIPROCS, ">citripr.tmp");
    do my_open(CITABLE, ">citrit1.tmp");
    do my_open(CIHDR,   ">g_citri.h");

    do init_Table(CITABLE, TRI_RASTERIZER_RETURN_TYPE,
		  TRI_RASTERIZER_CALLING_CONV, TRI_RASTERIZER_TABLE_NAME_CI,
		  __FR_SETUP_TABLE_SIZE_CI, TRI_RASTERIZER_SIGNATURE);

    for ($ii = 0; $ii < __FR_SETUP_TABLE_SIZE_CI; $ii++) {
	if (do isSupportedMode($ii)) {
	    do TemplateInstance(CIPROCS, CITABLE, $ii);
	} else {
	    do NullInstance(CITABLE);
	}
    }

    do end_Table(CITABLE);

    # Write header file
    do proto_Table(CIHDR, TRI_RASTERIZER_RETURN_TYPE,
		   TRI_RASTERIZER_CALLING_CONV, TRI_RASTERIZER_TABLE_NAME_CI,
		   __FR_SETUP_TABLE_SIZE_CI, TRI_RASTERIZER_SIGNATURE);
}
