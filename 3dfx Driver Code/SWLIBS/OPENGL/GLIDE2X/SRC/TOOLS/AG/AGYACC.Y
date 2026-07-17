%{
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
#include <stdio.h>
#include <ctype.h>

#include "ag.h"

static int mi;
static char mem[20][100];
static enum eval tmem[20];
static int line_number = 1;

%}

%token VANILLA
%token LEADER
%token IDENT
%token DOT

%nonassoc DOT

%union {
  char *str;
  enum eval val;
  int i;
};

%type <str> VANILLA IDENT
%type <val> mexpr
%type <str> memlsts

%%

start	:
	| line start
	;

line	: VANILLA
	{
	  printf("%s", $1);
	  line_number++;
	}
	| LEADER IDENT ':'
	{
	  printf("%s->spec.label.where = cb; ", $2);
	  printf("resolve(gc, %s, cb);\n", $2);
	  line_number++;
	}
	|
	LEADER IDENT 
	{
	  printf("cb = %s(gc, cb", $2);
	} '(' parms ')'
	{
	  printf(");\n");
	  fflush(stdout);
	  line_number++;
	}
	| LEADER IDENT
	{
	  printf("cb = emit_code(gc, cb, %s(gc", $2);
	}
	parms
	{
	  printf("));\n");
	  fflush(stdout);
	  line_number++;
	}
	;

parms	:
	|
	parmlist
	;

parmlist :
	expr
	|
	expr ',' parmlist
	;

expr	: IDENT DOT IDENT
	{
	  printf(", value(gc, offsetof(%s, %s))", $3, $1);
	}
	| '#' IDENT
	{
	  printf(", value(gc, %s)", $2);
	}
	| IDENT
	{
	  if ((isdigit($1[0])) || ($1[0] == '-') || ($1[0] == '~')) {
	    printf(", value(gc, %s)", $1);
	  } else {
	    printf(", %s", $1);
	  }
	}
	| '[' { mi = 0; } memlsts ']'
	{
	  int i;
	  int ok;
	  char *slot[3];

          /* By now, we've run through the whole address expression,
	   * storing up 'mi' terms in the tmem[] and mem[] arrays.
	   * Now we're going to run thrugh the arrays, trying
	   * to recognise an Intel address expression.
	   */

	  slot[0] = slot[1] = slot[2] = NULL;

	  ok = 1;
	  for (i = 0; ok && (i < mi); i++)
	    if (tmem[i] == reg) {
	      if (slot[1] == NULL)
		slot[1] = mem[i];
	      else if (slot[2] == NULL)
		slot[2] = mem[i];
	      else
		yyerror("Too many registers in address");
	    } else if (tmem[i] == literal) {
	      if (slot[0] == NULL)
		slot[0] = mem[i];
	      else
		ok = 0;
	    }
	    
	  if (ok) {
	    int i;
	    
	    if ((strcmp($3, "1") != 0) && (slot[2] == NULL)) {
	      slot[2] = slot[1];
	      slot[1] = NULL;
	    }

	    for (i =0; i < 3; i++)
	      if (slot[i] == NULL)
		slot[i] = "NULL";
	    
	    if ((strcmp($3, "1") == 0) && (slot[2] == "NULL")) {
	      printf(", mem2(gc, %s, %s)", slot[0], slot[1]);
	    } else {
	      printf(", mem(gc, %s, %s, %s, %s)", 
		     slot[0],
		     slot[1],
		     slot[2],
		     $3);
	    }
	  } else {
		printf(", xmem(gc, %s, ", $3);
		for (i = 0; i < mi; i++)
		  printf("%s, ", mem[i]);
		printf("NULL)");
	      }
	}
	;

memlsts	: memlst '*' IDENT
	{
	  $$ = $3;
	}
	|
	memlst
	{
	  $$ = "1";
	}
	;

memlst	: mexpr
	| mexpr '+' memlst
	;

mexpr	: IDENT DOT IDENT
	{
	  sprintf(mem[mi], "value(gc, offsetof(%s, %s))", $3, $1);
	  tmem[mi++] = literal;
	}
	| IDENT
	{
	  if (isdigit($1[0]) || ($1[0] == '-')) {
	    sprintf(mem[mi], "value(gc, %s)", $1);
	    tmem[mi++] = literal;
	  } else {
	    if ($1[0] == '$') {
		  sprintf(mem[mi], "%s", $1 + 1);
	          tmem[mi++] = reg;
	    } else if ((strcmp($1, "eax") == 0) ||
		  (strcmp($1, "ebx") == 0) ||
		  (strcmp($1, "ecx") == 0) ||
		  (strcmp($1, "edx") == 0) ||
		  (strcmp($1, "ebp") == 0) ||
		  (strcmp($1, "esp") == 0) ||
		  (strcmp($1, "esi") == 0) ||
		  (strcmp($1, "edi") == 0)) {
		  sprintf(mem[mi], "%s", $1);
		  tmem[mi++] = reg;
	    } else {
	      sprintf(mem[mi], "%s", $1);
	      tmem[mi++] = literal;
	    }
	  }
	}
	| '#' IDENT
	{
	  sprintf(mem[mi], "value(gc, %s)", $2);
	  tmem[mi++] = literal;
	}
	;


%%

static
yyerror (s)
     char *s;
{
  fprintf(stderr, "ERROR at line %d: %s\n", line_number, s);
  return 1;
}

