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

#define PERSPECTIVE	(($mode & __FR_TEXEL_PERSPECTIVE) != 0)
#define RGBMODE		(($mode & __FR_TEXEL_NEED_RGB) != 0)
#define HAS_RGB		(($mode & __FR_TEXEL_HAS_RGB) != 0)
#define HAS_ALPHA	(($mode & __FR_TEXEL_HAS_ALPHA) != 0)
#define NEED_ALPHA	(($mode & __FR_TEXEL_NEED_ALPHA) != 0)
#define TF_LUMINANCE	(RGBMODE && !HAS_RGB && !HAS_ALPHA)
#define TF_LUMALPHA	(RGBMODE && !HAS_RGB &&  HAS_ALPHA)
#define TF_RGB		(RGBMODE &&  HAS_RGB && !HAS_ALPHA)
#define TF_RGBA		(RGBMODE &&  HAS_RGB &&  HAS_ALPHA)
#define TF_INDEX	(($mode & __FR_TEXEL_HAS_INDEX) != 0)


#define TEXEL_RETURN_TYPE	"void"
#define TEXEL_CALL_CONV		"__fastcall"
#define TEXEL_NAME		"__glFRExtractTexels"
#define TEXEL_SIGNATURE		"(GLbitfield mask, __GLtri *tr)"
#define TEXEL_TABLE_NAME	"__fr_texel_table"

#define TEXEL_HEIGHT		"(tr->texHeight)"
#define TEXEL_LOG_HEIGHT	"(tr->texHeightLog2)"

#define TEXEL_WIDTHS	8
#define TEXEL_SHAPES	2
#define TEXEL_HINTS	2
#define TEXEL_NEEDS	4
#define TEXEL_FORMATS	8

##########################################################################
# Global Variables
##########################################################################

@TexShapes = (0, __FR_TEXEL_SQUARE);
@TexHints = (0, __FR_TEXEL_PERSPECTIVE);
@TexNeeds = (0, __FR_TEXEL_NEED_RGB,
	     __FR_TEXEL_NEED_RGB|__FR_TEXEL_NEED_ALPHA);
@TexFormats = (__FR_TEXEL_HAS_INDEX,
	       __FR_TEXEL_HAS_RGB,
	       __FR_TEXEL_HAS_ALPHA,
	       __FR_TEXEL_HAS_RGB|__FR_TEXEL_HAS_ALPHA,
	       __FR_TEXEL_HAS_INDEX|__FR_TEXEL_HAS_RGB,
	       __FR_TEXEL_HAS_INDEX|__FR_TEXEL_HAS_ALPHA,
	       __FR_TEXEL_HAS_INDEX|__FR_TEXEL_HAS_RGB|__FR_TEXEL_HAS_ALPHA);

##########################################################################
# Subroutines
##########################################################################

sub my_open {
    local($fd) = $_[0];
    local($name) = $_[1];
    open($fd, $name) || die "Can't open $name: $!, stopped";
}

sub skipComment {
    local($fh) = $_[0];
    local($cnt) = $_[1];

    if (0 >= $cnt) {
	return;
    } elsif (1 == $cnt) {
	printf $fh "/**/\n";
    } else {
	printf $fh "/*\n";
	while ($cnt-- > 2) {
	    printf $fh " *\n";
	}
	printf $fh " */\n";
    }
}

sub SetDefines {
    local($fh) = $_[0];
    local($mode) = $_[1];
    local($size) = ($mode & __FR_TEXEL_LOG_WIDTH) + __FR_TEXEL_WIDTH_BIAS;

    printf($fh "\\#define FR_TEX_WIDTH %d\n", 1<<$size);
    printf($fh "\\#define FR_TEX_LOG_WIDTH %d\n", $size);
    if ($mode & __FR_TEXEL_SQUARE) {
	printf($fh "\\#define FR_TEX_HEIGHT %d\n", 1<<$size);
	printf($fh "\\#define FR_TEX_LOG_HEIGHT %d\n", $size);
    } else {
	printf($fh "\\#define FR_TEX_HEIGHT %s\n", TEXEL_HEIGHT);
	printf($fh "\\#define FR_TEX_LOG_HEIGHT %s\n", TEXEL_LOG_HEIGHT);
    }

    printf($fh "#define FR_TEX_PERSPECTIVE %d\n", PERSPECTIVE);
    printf($fh "#define FR_TEX_RGBMODE %d\n", RGBMODE);
    printf($fh "#define FR_TEX_NEED_ALPHA %d\n", RGBMODE && NEED_ALPHA);
    printf($fh "#define FR_TEX_LUMINANCE %d\n", TF_LUMINANCE);
    printf($fh "#define FR_TEX_LUMINANCE_ALPHA %d\n", TF_LUMALPHA);
    printf($fh "#define FR_TEX_RGB %d\n", TF_RGB);
    printf($fh "#define FR_TEX_RGBA %d\n", TF_RGBA);
    printf($fh "#define FR_TEX_TABLE %d\n", RGBMODE && TF_INDEX);
}

sub UnsetDefines {
    local($fh) = $_[0];
    printf($fh "\\#undef FR_TEX_WIDTH\n");
    printf($fh "\\#undef FR_TEX_LOG_WIDTH\n");
    printf($fh "\\#undef FR_TEX_HEIGHT\n");
    printf($fh "\\#undef FR_TEX_LOG_HEIGHT\n");
    printf($fh "#undef FR_TEX_PERSPECTIVE\n");
    printf($fh "#undef FR_TEX_RGBMODE\n");
    printf($fh "#undef FR_TEX_NEED_ALPHA\n");
    printf($fh "#undef FR_TEX_LUMINANCE\n");
    printf($fh "#undef FR_TEX_LUMINANCE_ALPHA\n");
    printf($fh "#undef FR_TEX_RGB\n");
    printf($fh "#undef FR_TEX_RGBA\n");
    printf($fh "#undef FR_TEX_TABLE\n");
}

sub gen_TexelProc {
    local($fh) = $_[0];
    local($mode) = $_[1];

    printf($fh "#define FR_EXTRACT_TEXELS_PROTOTYPE ");
    printf($fh "%s %s %s_%X%s\n", TEXEL_RETURN_TYPE, TEXEL_CALL_CONV,
	   TEXEL_NAME, $mode, TEXEL_SIGNATURE);
    printf($fh "#include \"texel.inc\"\n");
    printf($fh "#undef FR_EXTRACT_TEXELS_PROTOTYPE\n");
}

#if 0
sub gen_TexelProc_asm {
    local($fh) = $_[0];
    local($mode) = $_[1];

    printf($fh "#define FR_ZTEST_PROTOTYPE ");
    printf($fh "__declspec(naked) %s %s %s_%X%s\n", ZTEST_RETURN_TYPE, ZTEST_CALL_CONV,
	   ZTEST_NAME, $mode, ZTEST_SIGNATURE);
    printf($fh "#include \"ztestasm.inc\"\n");
    printf($fh "#undef FR_ZTEST_PROTOTYPE\n");
}
#endif

sub TemplateInstance {
    local($fh) = $_[0];
    local($mode) = $_[1];

    printf(TABLE1 "    %s_%X,\n", TEXEL_NAME,  $mode);

    do SetDefines($fh, $mode);
    printf $fh "/*\n";
    do SetDefines($fh, $mode);
    printf $fh "*/\n";
    do gen_TexelProc($fh, $mode);
#if 0
    printf $fh "\\#ifndef __GL_USE_INTEL_ASM\n";
    do gen_TexelProc($fh, $mode);
    printf $fh "\\#else\n";
    do gen_TexelProc_asm($fh, $mode);
    printf $fh "\\#endif\n";
#endif
    do UnsetDefines($fh);
}

sub NullInstance {
    printf TABLE1 "    0,\n";
}

sub init_Table {
    local($fh) = $_[0];
    local($ret_type) = $_[1];
    local($call_conv) = $_[2];
    local($name) = $_[3];
    local($count) = $_[4];
    local($sig) = $_[5];

    do skipComment($fh, 3);
    printf($fh "%s (%s *%s[%d])%s =\n{\n", $ret_type, $call_conv, $name,
	   $count, $sig);
}

sub end_Table {
    local($fh) = $_[0];
    printf($fh "};\n");
}

sub proto_Table {
    local($ret_type) = $_[0];
    local($cconv) = $_[1];
    local($name) = $_[2];
    local($count) = $_[3];
    local($sig) = $_[4];

    printf(HEADER "extern %s (%s *%s[%d])%s;\n", $ret_type, $cconv, $name,
	   $count, $sig);
}

sub isSupportedMode {
    local($mode) = $_[0];

    if (RGBMODE) {
	if (NEED_ALPHA && !HAS_ALPHA) {
	    return 0;
	}
    } else {
	if (!TF_INDEX || NEED_ALPHA || HAS_ALPHA || HAS_RGB) {
	    return 0;
	}
    }

    return 1;
}

##########################################################################
# Begin code generation
##########################################################################

do my_open(PROCS,  ">texelpr.tmp");
do my_open(TABLE1, ">texelt1.tmp");
do my_open(HEADER, ">g_texel.h");

do init_Table(TABLE1, TEXEL_RETURN_TYPE, TEXEL_CALL_CONV,
	      TEXEL_TABLE_NAME, __FR_TEXEL_TABLE_SIZE, TEXEL_SIGNATURE);

for ($ii = 0; $ii < __FR_TEXEL_TABLE_SIZE; $ii++) {
    if (do isSupportedMode($ii)) {
	do TemplateInstance(PROCS, $ii);
    } else {
	do NullInstance();
    }
}

do end_Table(TABLE1);

do proto_Table(TEXEL_RETURN_TYPE, TEXEL_CALL_CONV, TEXEL_TABLE_NAME,
	       __FR_TEXEL_TABLE_SIZE, TEXEL_SIGNATURE);
