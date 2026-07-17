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

#define ZTEST_RETURN_TYPE	"void"
#define ZTEST_CALL_CONV		"__fastcall"
#define ZTEST_NAME		"__glDepthTest"
#define ZTEST_SIGNATURE		"(GLbitfield mask, __GLtri *tr)"
#define ZTEST_TABLE_NAME	"__fr_ztest_table"

#define ZTEST_MASK	7

##########################################################################
# Global Variables
##########################################################################

# Size in bytes
@DepthSize = (2, 4);
@DepthType = ("__GLzValue16", "__GLzValue");
@DepthTest = ("0", "<", "==", "<=", ">", "!=", ">=", "1");
@DepthTest_asm = ("0", "jae", "jne", "ja", "jbe", "je", "jb", "1");

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
    local($size) = $mode & __FR_DEPTH32 ? 1 : 0;
    local($type) = $DepthType[$size];
    local($test) = $mode & ZTEST_MASK;
    local($test_op) = $DepthTest[$test];
    local($test_opcode) = $DepthTest_asm[$test];

    printf($fh "#define FR_ZSIZE %d\n", $DepthSize[$size]);
    printf($fh "#define FR_ZTYPE %s\n", $type);
    printf($fh "#define FR_ZTEST %d\n", $test);
    printf($fh "#define FR_ZTEST_OP %s\n", $test_op);
    printf($fh "#define FR_ZTEST_OPCODE %s\n", $test_opcode);
    printf($fh "#define FR_ZWRITE %d\n", ($mode & __FR_DEPTH_WRITE) != 0);
    printf($fh "#define FR_ZSTENCIL %d\n", ($mode & __FR_DEPTH_STENCIL) != 0);
}

sub UnsetDefines {
    local($fh) = $_[0];
    printf($fh "#undef FR_ZSIZE\n");
    printf($fh "#undef FR_ZTYPE\n");
    printf($fh "#undef FR_ZTEST\n");
    printf($fh "#undef FR_ZTEST_OP\n");
    printf($fh "#undef FR_ZTEST_OPCODE\n");
    printf($fh "#undef FR_ZWRITE\n");
    printf($fh "#undef FR_ZSTENCIL\n");
}

sub gen_ZTestProc {
    local($fh) = $_[0];
    local($mode) = $_[1];

    printf($fh "#define FR_ZTEST_PROTOTYPE ");
    printf($fh "%s %s %s_%X%s\n", ZTEST_RETURN_TYPE, ZTEST_CALL_CONV,
	   ZTEST_NAME, $mode, ZTEST_SIGNATURE);
    printf($fh "#include \"ztest.inc\"\n");
    printf($fh "#undef FR_ZTEST_PROTOTYPE\n");
}

sub gen_ZTestProc_asm {
    local($fh) = $_[0];
    local($mode) = $_[1];

    printf($fh "#define FR_ZTEST_PROTOTYPE ");
    printf($fh "__declspec(naked) %s %s %s_%X%s\n", ZTEST_RETURN_TYPE, ZTEST_CALL_CONV,
	   ZTEST_NAME, $mode, ZTEST_SIGNATURE);
    printf($fh "#include \"ztestasm.inc\"\n");
    printf($fh "#undef FR_ZTEST_PROTOTYPE\n");
}

sub TemplateInstance {
    local($fh) = $_[0];
    local($mode) = $_[1];

    printf(TABLE1 "    %s_%X,\n", ZTEST_NAME,  $mode);

    do SetDefines($fh, $mode);
    printf $fh "/*\n";
    do SetDefines($fh, $mode);
    printf $fh "*/\n";
    printf $fh "\\#ifndef __GL_USE_INTEL_ASM\n";
    do gen_ZTestProc($fh, $mode);
    printf $fh "\\#else\n";
    do gen_ZTestProc_asm($fh, $mode);
    printf $fh "\\#endif\n";
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

##########################################################################
# Begin code generation
##########################################################################

do my_open(PROCS,  ">ztestpr.tmp");
do my_open(TABLE1, ">ztestt1.tmp");
do my_open(HEADER, ">g_ztest.h");

printf PROCS "\
\#define NEVER 0\n\
\#define ALWAYS 7\n\
\#define ZTESTFAILOPCODE FR_ZTEST_OPCODE\n\
";

do init_Table(TABLE1, ZTEST_RETURN_TYPE, ZTEST_CALL_CONV,
	      ZTEST_TABLE_NAME, __FR_ZTEST_TABLE_SIZE, ZTEST_SIGNATURE);

for ($ii = 0; $ii < __FR_ZTEST_TABLE_SIZE; $ii++) {
    do TemplateInstance(PROCS, $ii);
}

do end_Table(TABLE1);

do proto_Table(ZTEST_RETURN_TYPE, ZTEST_CALL_CONV, ZTEST_TABLE_NAME,
	       __FR_ZTEST_TABLE_SIZE, ZTEST_SIGNATURE);
