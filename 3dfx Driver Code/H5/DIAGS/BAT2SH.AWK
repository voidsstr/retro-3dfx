# bat2sh.awk
#
#
BEGIN {
    printf ("#!/bin/sh\n");
}

/^REM/		{ $1 = "#" }
/^setlocal/	{ next }
/^@echo/	{ next }
/^set EXFLAGS/	{ next }
/^call/		{ $1 = "" ; $2 = substr($2,1,length($2)-4) ".sh" ;  }
/^erase/	{ $1 = "rm -f" }
/^ttime/	{ $1 = "time" }
/%EXFLAGS%/	{for (i=NF; i > 0; i--) if ( $i == "%EXFLAGS%" ) $i = "$*"; }
{ print }