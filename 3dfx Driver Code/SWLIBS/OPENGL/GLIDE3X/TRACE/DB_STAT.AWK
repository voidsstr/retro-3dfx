
BEGIN {
    # Grrr. Because mks awk has a bug where cmd-line variables are not seen by
    # the script until after BEGIN is called, we have to work around it with this.
    do_begin_stuff = 1;
}


{
    if (do_begin_stuff) {
        if (OUTPUT == "header") {
	    printf("struct __GLdebugStat { \n");
	    # add the generic ones
	    printf("\tint %20-s char *str%s;\n",
		   "HistTri[__GL_DEBUG_HISTOGRAM_SIZE];", "HistTri");
	    printf("\tint %20-s char *str%s;\n",
		   "HistTriStrip[__GL_DEBUG_HISTOGRAM_SIZE];", "HistTriStrip");
	    printf("\tint %20-s char *str%s;\n",
		   "HistTriFan[__GL_DEBUG_HISTOGRAM_SIZE];", "HistTriFan");
	    printf("\tint %20-s char *str%s;\n",
		   "HistQuad[__GL_DEBUG_HISTOGRAM_SIZE];", "HistQuad");
	    printf("\tint %20-s char *str%s;\n",
		   "HistQuadStrip[__GL_DEBUG_HISTOGRAM_SIZE];", "HistQuadStrip");
	    printf("\tint %20-s char *str%s;\n", "TotalTri;", "TotalTri");
	    printf("\tint %20-s char *str%s;\n", "TriTri;", "TriTri");
	    printf("\tint %20-s char *str%s;\n", "TriStripTri;", "TriStripTri");
	    printf("\tint %20-s char *str%s;\n", "TriFanTri;", "TriFanTri");
	    printf("\tint %20-s char *str%s;\n", "QuadTri;", "QuadTri");
	    printf("\tint %20-s char *str%s;\n", "QuadStripTri;", "QuadStripTri");
	    printf("\tint %20-s char *str%s;\n", "Vertex;", "Vertex");
	    printf("\tint %20-s char *str%s;\n", "TexCoord;", "TexCoord");
	    printf("\tint %20-s char *str%s;\n", "Color;", "Color");
	    printf("\tint %20-s char *str%s;\n", "Normal;", "Normal");
	} else if (OUTPUT == "table") {
	    printf("#include <stdio.h>\n");
	    printf("#include <windows.h>\n");
	    printf("#include <GL/gl.h>\n");
	    printf("#include \"g_imfncs.h\"\n");
	    printf("#include \"g_disp.h\"\n");
	    printf("#include \"db_trace.h\"\n");
	    printf("struct __GLdebugStat __gl_debug_table = {\n");
	} else if (OUTPUT == "cumtable") {
	    printf("struct __GLdebugStat __gl_debug_cum_table = {\n");
	}
	if (OUTPUT == "table" || OUTPUT == "cumtable") {
	    printf("\t{0,0,0,0,0,0,0,0,0,0}, \"HistTri\",\n");
	    printf("\t{0,0,0,0,0,0,0,0,0,0}, \"HistTriStrip\",\n");
	    printf("\t{0,0,0,0,0,0,0,0,0,0}, \"HistTriFan\",\n");
	    printf("\t{0,0,0,0,0,0,0,0,0,0}, \"HistQuad\",\n");
	    printf("\t{0,0,0,0,0,0,0,0,0,0}, \"HistQuadStrip\",\n");
	    printf("\t0, \"Total tris\",\n");
	    printf("\t0, \"Tri tris\",\n");
	    printf("\t0, \"TriStrip tris\",\n");
	    printf("\t0, \"TriFan tris\",\n");
	    printf("\t0, \"Quad tris\",\n");
	    printf("\t0, \"QuadStrip tris\",\n");
	    printf("\t0, \"Vertex\",\n");
	    printf("\t0, \"TexCoord\",\n");
	    printf("\t0, \"Color\",\n");
	    printf("\t0, \"Normal\",\n");
	}
	do_begin_stuff = 0;
    }

    if (NF==0) continue;
    if (substr($1,1,1)=="#") continue;

    if ($1=="const") {
	funcName = $3;
	returnType = $2;
	n = 4;
	left = NF - 3;
	constReturn = 1;
    } else {
	funcName = $2;
	returnType = $1;
	n = 3;
	left = NF - 2;
	constReturn = 0;
    }
    if (substr(funcName,1,1)=="*") {
	sub("*", "", funcName);
    }
    if (OUTPUT == "header") {
        printf("\tint %20-s char *str%s;\n", funcName ";", funcName);
    } else if (OUTPUT == "table" || OUTPUT == "cumtable") {
        printf("\t0, \"%s\",\n", funcName);
    }
}

END {
  if (OUTPUT=="header") {
    printf("#include \"db_stat_gen.h\"\n");
    printf("};\n\n");
    printf("extern struct __GLdebugStat __gl_debug_table;\n");
    printf("extern struct __GLdebugStat __gl_debug_cum_table;\n");

  } else if (OUTPUT=="table" || OUTPUT=="cumtable") {
    printf("#include \"db_stat_gen.c\"\n");
    printf("};\n\n");
  }
}
