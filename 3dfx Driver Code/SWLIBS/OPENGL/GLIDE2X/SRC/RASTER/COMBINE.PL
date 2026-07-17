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

#define fr_CI_MODE		($fr_RGB == 0)
#define fr_RGB_MODE		($fr_RGB != 0)

#define fr_BLEND		(($mode & __FR_COMBINE_BLEND) != 0)
#define fr_SRC_BLEND_FUNC	(fr_BLEND ? ($mode & __FR_BLEND_SRC_FUNC) : -1)
#define fr_DST_BLEND_FUNC	(fr_BLEND ? ($mode & __FR_BLEND_DST_FUNC) : -1)
#define fr_COMBINE_OP		($mode & __FR_COMBINE_OPERATION)
#define fr_LOGICOP		(fr_BLEND ? -1 : fr_COMBINE_OP)

#define fr_NEED_SRC		(fr_BLEND || \
				 ((fr_LOGICOP != __FR_LO_CLEAR) && \
				  (fr_LOGICOP != __FR_LO_NOOP) && \
				  (fr_LOGICOP != __FR_LO_INVERT) && \
				  (fr_LOGICOP != __FR_LO_SET)))

#define fr_DST_COMPS		(fr_BLEND && \
				 (fr_DST_BLEND_FUNC != __FR_BLEND_DST_ZERO))

#define fr_NEED_DST		(fr_DST_COMPS || \
				 (!fr_BLEND && \
				  (fr_LOGICOP != __FR_LO_CLEAR) && \
				  (fr_LOGICOP != __FR_LO_COPY) && \
				  (fr_LOGICOP != __FR_LO_COPY_INVERTED) && \
				  (fr_LOGICOP != __FR_LO_SET)))

#define fr_DITHER		(($mode & __FR_COMBINE_DITHER) != 0)
#define fr_ALPHA		(($mode & __FR_COMBINE_ALPHA) != 0)
#define fr_CLAMP		(fr_BLEND && \
				 ((fr_COMBINE_OP == __FR_BLEND_SA_ONE) || \
				  (fr_COMBINE_OP == __FR_BLEND_SAS_ONE)))

#define COMBINE_RETURN_TYPE	"void"
#define COMBINE_CALL_CONV	"__fastcall"
#define COMBINE_NAME		"__glCombinePixels"
#define COMBINE_SIGNATURE	"(unsigned int mask, __GLtri *tr)"
#define COMBINE_TABLE_NAME_RGB	"__fr_rgb_combine_%x"
#define COMBINE_TABLE_NAME_CI	"__fr_ci_combine_table"

##########################################################################
# Global Variables
##########################################################################

@DitherModes = (0, __FR_COMBINE_DITHER);
@BlendModes = (0, __FR_COMBINE_BLEND);
@AlphaModes = (0, __FR_COMBINE_ALPHA);
@BlendOps = (0, __FR_BLEND_SA_ZERO, __FR_BLEND_SA_ONE, __FR_BLEND_SA_MSA,
	     __FR_BLEND_MSA_SA, __FR_BLEND_SAS_ONE);
@SupportedModes = ();

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

    printf($fh "%s#define FR_CI_MODE %d\n", $prefix, fr_CI_MODE);
    printf($fh "%s#define FR_RGB_MODE %d\n", $prefix, fr_RGB_MODE);
    printf($fh "%s#define FR_BLEND %d\n", $prefix, fr_BLEND);
    printf($fh "%s#define FR_SRC_BLEND_FUNC %d\n", $prefix, fr_SRC_BLEND_FUNC);
    printf($fh "%s#define FR_DST_BLEND_FUNC %d\n", $prefix, fr_DST_BLEND_FUNC);
    printf($fh "%s#define FR_LOGICOP %d\n", $prefix, fr_LOGICOP);
    printf($fh "%s#define FR_DITHER %d\n", $prefix, fr_DITHER);
    printf($fh "%s#define FR_NEED_SRC %d\n", $prefix, fr_NEED_SRC);
    printf($fh "%s#define FR_NEED_DST %d\n", $prefix, fr_NEED_DST);
    printf($fh "%s#define FR_DST_COMPS %d\n", $prefix, fr_DST_COMPS);
    printf($fh "%s#define FR_ALPHA %d\n", $prefix, fr_ALPHA);
    printf($fh "%s#define FR_CLAMP %d\n", $prefix, fr_CLAMP);
}

sub UnsetDefines {
    local($fh) = $_[0];

    printf($fh "#undef FR_CI_MODE\n");
    printf($fh "#undef FR_RGB_MODE\n");
    printf($fh "#undef FR_BLEND\n");
    printf($fh "#undef FR_SRC_BLEND_FUNC\n");
    printf($fh "#undef FR_DST_BLEND_FUNC\n");
    printf($fh "#undef FR_LOGICOP\n");
    printf($fh "#undef FR_DITHER\n");
    printf($fh "#undef FR_NEED_SRC\n");
    printf($fh "#undef FR_NEED_DST\n");
    printf($fh "#undef FR_DST_COMPS\n");
    printf($fh "#undef FR_ALPHA\n");
    printf($fh "#undef FR_CLAMP\n");
}

sub gen_CombineProc {
    local($fh) = $_[0];
    local($name) = $_[1];

    printf($fh "#define FR_COMBINE_PROTOTYPE ");
    printf($fh "%s %s %s%s\n", COMBINE_RETURN_TYPE, COMBINE_CALL_CONV,
	   $name, COMBINE_SIGNATURE);
    printf($fh "#include \"combine.inc\"\n");
    printf($fh "#undef FR_COMBINE_PROTOTYPE\n");
}

sub TemplateInstance {
    local($procs) = $_[0];
    local($table) = $_[1];
    local($mode) = $_[2];
    local($pixel) = $_[3];
    local($name) = sprintf("%s%s%x_%X", COMBINE_NAME, $fr_RGB ? "RGB" : "CI",
			   $pixel, $mode);

    printf($table "    %s,\n", $name);

    do SetDefines($procs, $mode, "");
    printf $procs "/*\n";
    do SetDefines($procs, $mode, " * ");
    printf $procs " */\n";
    do gen_CombineProc($procs, $name);
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

    foreach $m (@SupportedModes) {
	if ($mode == $m) {
	    return 1;
	}
    }
    return 0;
}

##########################################################################
# Begin code generation
##########################################################################

sub gen_rgb_procs {
    local($pixel) = $_[0];
    local($header)   = $_[1];

    local($fname) = sprintf("g_rgbcb%x.c", $pixel);
    local($tname) = sprintf(COMBINE_TABLE_NAME_RGB, $pixel);

    # temporary files
    local($procs) = "rgbcmbt0.tmp";
    local($table) = "rgbcmbt1.tmp";
    local($tmpfl) = "rgbcmbt2.tmp";

    local($ii);

    do my_open(PROCS, ">$procs");
    do my_open(TABLE, ">$table");

    do init_Table(TABLE, COMBINE_RETURN_TYPE, COMBINE_CALL_CONV,
		  $tname, __FR_COMBINE_TABLE_SIZE_RGB,
		  COMBINE_SIGNATURE);

    for ($ii = 0; $ii < __FR_COMBINE_TABLE_SIZE_RGB; $ii++) {
	if (do isSupportedMode($ii)) {
	    do TemplateInstance(PROCS, TABLE, $ii, $pixel);
	} else {
	    do NullInstance(TABLE);
	}
    }
    do end_Table(TABLE);
    do proto_Table($header, COMBINE_RETURN_TYPE, COMBINE_CALL_CONV,
		   $tname, __FR_COMBINE_TABLE_SIZE_RGB,
		   COMBINE_SIGNATURE);
    close(PROCS);
    close(TABLE);

    system "cat fr_modes.h $procs | /usr/lib/acpp -P -C | sed 's/^\\\\#/#/' | \
	    awk -f squeeze.awk > $tmpfl";
    system "echo \"#ifdef __GL_PC_RAST\" > $fname";
    system "cat combine.pro >> $fname";
    system "echo \"\n#define PIXEL_TYPE $pixel\" >> $fname";
    system "echo \"#include \\\"fr_fbconf.h\\\"\" >> $fname";
    system "cat $tmpfl $table >> $fname";
    system "echo \"\\n#endif /* __GL_PC_RAST */\" >> $fname";

    unlink $procs, $table, $tmpfl;
}

if ($fr_RGB) {

    # Build list of supported modes
    foreach $dm (@DitherModes) {
	foreach $am (@AlphaModes) {
	    foreach $bm (@BlendModes) {
		if ($bm) {
		    if ($am) {
			foreach $op (@BlendOps) {
			    local($mode) = $dm+$am+$bm+$op;
			    push (SupportedModes, $mode);
			}
		    }
		} else {
		    for ($op = 0; $op < 16; $op++) {
			local($mode) = $dm+$am+$bm+$op;
			push (SupportedModes, $mode);
		    }
		}
	    }
	}
    }

    do my_open(HEADER,   ">g_rgbcmb.h");
    for ($ii = 0; $ii <= $#RGBPixelFormats; $ii++) {
	do gen_rgb_procs($RGBPixelFormats[$ii], HEADER);
    }
    close(HEADER);

} else {
    do my_open(CIPROCS, ">cicmbpr.tmp");
    do my_open(CITABLE, ">cicmbt1.tmp");
    do my_open(CIHDR,   ">g_cicmb.h");

    do init_Table(CITABLE, COMBINE_RETURN_TYPE, COMBINE_CALL_CONV,
		  COMBINE_TABLE_NAME_CI, __FR_COMBINE_TABLE_SIZE_CI,
		  COMBINE_SIGNATURE);

    for ($ii = 0; $ii < __FR_COMBINE_TABLE_SIZE_CI; $ii++) {
	do TemplateInstance(CIPROCS, CITABLE, $ii);
    }

    do end_Table(CITABLE);

    do proto_Table(CIHDR, COMBINE_RETURN_TYPE, COMBINE_CALL_CONV,
		   COMBINE_TABLE_NAME_CI, __FR_COMBINE_TABLE_SIZE_CI,
		   COMBINE_SIGNATURE);
}
