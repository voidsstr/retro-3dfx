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
#include "fr_fbtype.h"

#define fr_CI_COLOR	  	($fr_RGB == 0)
#define fr_RGB_COLOR	  	($fr_RGB && (($mode & __FR_PIXEL_ALPHA) == 0))
#define fr_RGBA_COLOR	  	($fr_RGB && (($mode & __FR_PIXEL_ALPHA) != 0))

#define fr_FLAT_SHADING		(($mode & __FR_PIXEL_SMOOTH) == 0)
#define fr_SMOOTH_SHADING	(($mode & __FR_PIXEL_SMOOTH) != 0)

#define fr_TEXENV		($mode & __FR_PIXEL_TEXENV)
#define fr_TEXTURING		(fr_TEXENV != 0)
#define fr_MODULATE		(fr_TEXENV == __FR_PIXEL_MODULATE)
#define fr_DECAL		($fr_RGB && (fr_TEXENV == __FR_PIXEL_DECAL))
#define fr_BLEND		(fr_TEXENV == __FR_PIXEL_BLEND)
#define fr_BLEND_INTENSITY	(fr_TEXENV == __FR_PIXEL_BLEND_INTENSITY)
#define fr_REPLACE		(fr_TEXENV == __FR_PIXEL_REPLACE)
#define fr_REPLACE_ALPHA	(fr_TEXENV == __FR_PIXEL_REPLACE_ALPHA)
#define fr_MODULATE_ALPHA	(fr_TEXENV == __FR_PIXEL_MODULATE_ALPHA)
#define fr_ADD			(fr_CI_COLOR && (fr_TEXENV == __FR_PIXEL_ADD))

#define fr_TEXALPHA (fr_TEXTURING && (($mode & __FR_PIXEL_TEXTURE_ALPHA) != 0))

#define GENCLR_RETURN_TYPE	"void"
#define GENCLR_CALL_CONV	"__fastcall"
#define GENCLR_NAME		"__glGenColors"
#define GENCLR_SIGNATURE	"(unsigned int mask, __GLtri *tr)"
#define GENCLR_TABLE_NAME_RGB	"__fr_rgb_genclr_%x"
#define GENCLR_TABLE_NAME_CI	"__fr_ci_genclr_table"

##########################################################################
# Global Variables
##########################################################################

@RGBTextureModes = (__FR_PIXEL_MODULATE, __FR_PIXEL_DECAL, __FR_PIXEL_BLEND,
		    __FR_PIXEL_REPLACE, __FR_PIXEL_BLEND_INTENSITY,
		    __FR_PIXEL_REPLACE_ALPHA, __FR_PIXEL_MODULATE_ALPHA);
@CITextureModes = (__FR_PIXEL_REPLACE, __FR_PIXEL_ADD);

#include "fblist.pl"

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
    local($prefix) = $_[2];

    printf($fh "%s#define FR_CI_COLOR %d\n", $prefix, fr_CI_COLOR);
    printf($fh "%s#define FR_RGB_COLOR %d\n", $prefix, fr_RGB_COLOR);
    printf($fh "%s#define FR_RGBA_COLOR %d\n", $prefix, fr_RGBA_COLOR);
    printf($fh "%s#define FR_SMOOTH_SHADING %d\n", $prefix, fr_SMOOTH_SHADING);
    printf($fh "%s#define FR_FLAT_SHADING %d\n", $prefix, fr_FLAT_SHADING);
    printf($fh "%s#define FR_TEXTURING %d\n", $prefix, fr_TEXTURING);
    printf($fh "%s#define FR_MODULATE %d\n", $prefix, fr_MODULATE);
    printf($fh "%s#define FR_DECAL %d\n", $prefix, fr_DECAL);
    printf($fh "%s#define FR_BLEND %d\n", $prefix, fr_BLEND);
    printf($fh "%s#define FR_BLEND_INTENSITY %d\n",$prefix,fr_BLEND_INTENSITY);
    printf($fh "%s#define FR_REPLACE %d\n", $prefix, fr_REPLACE);
    printf($fh "%s#define FR_REPLACE_ALPHA %d\n", $prefix, fr_REPLACE_ALPHA);
    printf($fh "%s#define FR_MODULATE_ALPHA %d\n", $prefix, fr_MODULATE_ALPHA);
    printf($fh "%s#define FR_ADD %d\n", $prefix, fr_ADD);
    printf($fh "%s#define FR_TEXALPHA %d\n", $prefix, fr_TEXALPHA);
}

sub UnsetDefines {
    local($fh) = $_[0];
    printf($fh "#undef FR_CI_COLOR\n");
    printf($fh "#undef FR_RGB_COLOR\n");
    printf($fh "#undef FR_RGBA_COLOR\n");
    printf($fh "#undef FR_SMOOTH_SHADING\n");
    printf($fh "#undef FR_FLAT_SHADING\n");
    printf($fh "#undef FR_TEXTURING\n");
    printf($fh "#undef FR_MODULATE\n");
    printf($fh "#undef FR_DECAL\n");
    printf($fh "#undef FR_BLEND\n");
    printf($fh "#undef FR_BLEND_INTENSITY\n");
    printf($fh "#undef FR_REPLACE\n");
    printf($fh "#undef FR_REPLACE_ALPHA\n");
    printf($fh "#undef FR_MODULATE_ALPHA\n");
    printf($fh "#undef FR_ADD\n");
    printf($fh "#undef FR_TEXALPHA\n");
}

sub gen_GenColorProc {
    local($fh) = $_[0];
    local($name) = $_[1];

    printf($fh "#define FR_GENCOLOR_PROTOTYPE ");
    printf($fh "%s %s %s%s\n", GENCLR_RETURN_TYPE, GENCLR_CALL_CONV,
	   $name, GENCLR_SIGNATURE);
    printf($fh "#include \"genclr.inc\"\n");
    printf($fh "#undef FR_GENCOLOR_PROTOTYPE\n");
}

sub TemplateInstance {
    local($procs) = $_[0];
    local($table) = $_[1];
    local($mode) = $_[2];
    local($pixel) = $_[3];
    local($name) = sprintf("%s%s%x_%X", GENCLR_NAME, $fr_RGB ? "RGB" : "CI",
			   $pixel, $mode);

    printf($table "    %s,\n", $name);

    do SetDefines($procs, $mode, "");
    printf $procs "/*\n";
    do SetDefines($procs, $mode, " * ");
    printf $procs " */\n";
    do gen_GenColorProc($procs, $name);
    do UnsetDefines($procs);
}

sub NullInstance {
    local($table) = $_[0];
    printf $table "    0,\n";
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
    local($fh) = $_[0];
    local($ret_type) = $_[1];
    local($cconv) = $_[2];
    local($name) = $_[3];
    local($count) = $_[4];
    local($sig) = $_[5];

    printf($fh "extern %s (%s *%s[%d])%s;\n", $ret_type, $cconv, $name,
	   $count, $sig);
}

sub isSupportedMode {
    local($mode) = $_[0];
    local($alpha) = ($mode & __FR_PIXEL_TEXTURE_ALPHA);

    if (fr_TEXTURING) {

	# Generate texture alpha only if needed
	if ($alpha && !(fr_RGBA_COLOR || fr_PIXEL_DECAL)) {
	    return 0
	}

	# Look for a supported texenv
	# Does perl support pointers?!
	if ($fr_RGB) {

	    # The following modes require TEXALPHA, and are enforced
	    # at validation.
	    if (!fr_TEXALPHA) {
		# DECAL w/o TEXALPHA is just REPLACE
		# ALPHA textures w/o ALPHA is just silly
		if (fr_DECAL || fr_REPLACE_ALPHA || fr_MODULATE_ALPHA) {
		    return 0;
		}
	    }

	    # If we don't need alpha, don't bother with alpha textures
	    if (!fr_RGBA_COLOR && (fr_REPLACE_ALPHA || fr_MODULATE_ALPHA)) {
		return 0;
	    }

	    foreach $m (@RGBTextureModes) {
		if (fr_TEXENV == $m) {
		    return 1;
		}
	    }
	} else {
	    foreach $m (@CITextureModes) {
		if (fr_TEXENV == $m) {
		    return 1;
		}
	    }
	}

	# Still here?  texenv must be invalid
	return 0;

    } elsif ($alpha) {
	# Don't care about texture alpha if not texturing!
	return 0;
    }
    return 1;
}

##########################################################################
# Begin code generation
##########################################################################

sub gen_rgb_procs {
    local($pixel) = $_[0];
    local($header)   = $_[1];

    local($fname) = sprintf("g_rgbcl%x.c", $pixel);
    local($tname) = sprintf(GENCLR_TABLE_NAME_RGB, $pixel);

    # temporary files
    local($procs) = "rgbclrt0.tmp";
    local($table) = "rgbclrt1.tmp";
    local($tmpfl) = "rgbclrt2.tmp";

    local($ii);

    do my_open(PROCS, ">$procs");
    do my_open(TABLE, ">$table");

    do init_Table(TABLE, GENCLR_RETURN_TYPE, GENCLR_CALL_CONV,
		  $tname, __FR_PIXEL_TABLE_SIZE_RGB,
		  GENCLR_SIGNATURE);

    for ($ii = 0; $ii < __FR_PIXEL_TABLE_SIZE_RGB; $ii++) {
	if (do isSupportedMode($ii)) {
	    do TemplateInstance(PROCS, TABLE, $ii, $pixel);
	} else {
	    do NullInstance(TABLE);
	}
    }
    do end_Table(TABLE);
    do proto_Table($header, GENCLR_RETURN_TYPE, GENCLR_CALL_CONV,
		   $tname, __FR_PIXEL_TABLE_SIZE_RGB,
		   GENCLR_SIGNATURE);
    close(PROCS);
    close(TABLE);

    system "cat $procs | /usr/lib/acpp -P -C | sed 's/^\\\\#/#/' | \
	    awk -f squeeze.awk > $tmpfl";
    system "echo \"#ifdef __GL_PC_RAST\" > $fname";
    system "cat genclr.pro >> $fname";
    system "echo \"\n#define PIXEL_TYPE $pixel\" >> $fname";
    system "echo \"#include \\\"fr_fbconf.h\\\"\" >> $fname";
    system "cat $tmpfl $table >> $fname";
    system "echo \"\\n#endif /* __GL_PC_RAST */\" >> $fname";

    unlink $procs, $table, $tmpfl;
}

if ($fr_RGB == 1) {

    do my_open(HEADER,   ">g_rgbclr.h");
    for ($ii = 0; $ii <= $#RGBPixelFormats; $ii++) {
	do gen_rgb_procs($RGBPixelFormats[$ii], HEADER);
    }
    close(HEADER);

} else {

    do my_open(CIPROCS, ">ciclrpr.tmp");
    do my_open(CITABLE, ">ciclrt1.tmp");
    do my_open(CIHDR,   ">g_ciclr.h");

    do init_Table(CITABLE, GENCLR_RETURN_TYPE, GENCLR_CALL_CONV,
		  GENCLR_TABLE_NAME_CI, __FR_PIXEL_TABLE_SIZE_CI,
		  GENCLR_SIGNATURE);

    for ($ii = 0; $ii < __FR_PIXEL_TABLE_SIZE_CI; $ii++) {
	if (do isSupportedMode($ii)) {
	    do TemplateInstance(CIPROCS, CITABLE, $ii);
	} else {
	    do NullInstance(CITABLE);
	}
    }
    do end_Table(CITABLE);
    do proto_Table(CIHDR, GENCLR_RETURN_TYPE, GENCLR_CALL_CONV,
		   GENCLR_TABLE_NAME_CI, __FR_PIXEL_TABLE_SIZE_CI,
		   GENCLR_SIGNATURE);
}
