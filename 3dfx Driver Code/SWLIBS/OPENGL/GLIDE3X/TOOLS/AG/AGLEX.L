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
#include <string.h>

#include "ag.h"
#include "y.tab.h"

%}
%%
^([^@\n].*)?\n	{ yylval.str = yytext; return VANILLA; }
^@		{ return LEADER; }
[$~-]?[_a-zA-Z0-9]+	{ yylval.str = strdup(yytext); return IDENT; }
[$]?'[^']+'		{ if (yytext[0] == '$')
				yytext[1] = '$';
			  yylval.str = strdup(yytext + 1); 
			  yylval.str[strlen(yylval.str) - 1] = '\0';
			  return IDENT; }
\.		{ return DOT; }
[,\[\]+*:#()]	{ return yytext[0]; }
[ \n\t]		;
;.*$		;

%%
