BEGIN {
    printf("#include <stdio.h>\n");
    printf("#include <windows.h>\n");
    printf("#include <gl/gl.h>\n");
    printf("#include \"g_imfncs.h\"\n");
    printf("#include \"g_disp.h\"\n");
    printf("#include \"g_lcfncs.h\"\n");
    printf("#include \"..\sst\sst_imfncs.h\"\n");
    printf("#include \"dbfncs.h\"\n");
    printf("#include \"db_trace.h\"\n\n");

    # these needed only because of the vertex routine's tricking the dispatch table
    printf("#include \"context.h\"\n");
    printf("#include \"global.h\"\n");
}

function indentify(len) {
    for (i=0; i < len; i++) {
	printf(" ");
    }
}

{
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
    numParam = 0;
    while (left > 0) {
	if ($n=="const") {
	    const[numParam] = 1;
	    n++; left--;
	} else {
	    const[numParam] = 0;
	}
	ptype[numParam] = $n; n++; left--;
	pname[numParam] = $n; n++; left--;
	numParam++;
    }

    #
    # Make prototype of routine.
    #
    prefix = "__gldb_";
    header = sprintf("%s%s %s APIENTRY %s%s ",
		     constReturn ? "const " : "", returnType, 
		     constReturn ? "*" : "", prefix, funcName);
    headerLen = length(header);
    printf("%s", header);

    if (numParam == 0) {
	printf("(void)\n");
    } else {
	printf("(");
	for (n = 0; n < numParam; n++) {
	    if (n > 0) indentify(headerLen+1);
	    printf("%s%s %s", const[n]==1 ? "const " : "", ptype[n], pname[n]);
	    if (n < numParam-1) printf(",\n");
	    else printf(")\n");
	}
    }
    printf("{\n");

    #
    # Get a gc pointer, because these routines need it for tricking the dispatch table.
    #
    if (index(funcName,"Vertex2") > 0 ||
	index(funcName,"Vertex3") > 0 ||
	index(funcName,"Vertex4") > 0) {
	printf("\t__GL_SETUP();\n\n");
    }
    
    # 
    # Print all parameters.
    #
    printf("\tif (__gl_debug_verbosity >= 5) {\n");
    if (funcName ~ /glVertex/)
	printf("\tfprintf(__gl_debug_log, \"    %-16s(\");\n", funcName);
    else
	printf("\tfprintf(__gl_debug_log, \"%-20s(\");\n", funcName);
    for (n = 0; n < numParam; n++) {
	typestr = "%x";
	pnamestr = pname[n];
	if (substr(pname[n],1,1)=="*") {
	    sub("*", "", pnamestr);
	}
	printf("\tfprintf(__gl_debug_log, \"%s:%s \", %s);\n", pnamestr, typestr, pnamestr);
    }
    printf("\tfprintf(__gl_debug_log, \")\\n\");\n");
    printf("\t}\n\n");

    #
    # if a vertex routine, temporarily replace the dispatch table ptr for it
    # because the gl calls that directly
    #
    if (index(funcName,"Vertex2") > 0) {
	printf("\tgc->dispatchState->vertex.Vertex2fv = __gl_real_immed->vertex.Vertex2fv;\n");
    }
    if (index(funcName,"Vertex3") > 0) {
	printf("\tgc->dispatchState->vertex.Vertex3fv = __gl_real_immed->vertex.Vertex3fv;\n");
    }
    if (index(funcName,"Vertex4") > 0) {
	printf("\tgc->dispatchState->vertex.Vertex4fv = __gl_real_immed->vertex.Vertex4fv;\n");
    }

    # 
    # call the routine
    #

    table = "dispatch";
    if (index(funcName,"Vertex2") > 0 ||
	index(funcName,"Vertex3") > 0 ||
	index(funcName,"Vertex4") > 0) {
      table = "vertex";
    } else if (index(funcName,"Color3") > 0 ||
	index(funcName,"Color4") > 0 ||
	index(funcName,"Indexd") > 0 ||
	index(funcName,"Indexf") > 0 ||
	index(funcName,"Indexi") > 0 ||
	index(funcName,"Indexs") > 0) {
      table = "color";
    } else if (index(funcName,"Normal3") > 0) {
      table = "normal";
    } else if (index(funcName,"TexCoord1") > 0 ||
	index(funcName,"TexCoord2") > 0 ||
	index(funcName,"TexCoord3") > 0 ||
	index(funcName,"TexCoord4") > 0) {
      table = "texCoord";
    } else if (index(funcName,"RasterPos2") > 0 ||
	index(funcName,"RasterPos3") > 0 ||
	index(funcName,"RasterPos4") > 0) {
      table = "rasterPos";
    } else if (index(funcName,"Rect") > 0) {
      table = "rect";
    }

    if (returnType != "void") {
      printf("\treturn\n");
    }
    header = sprintf("        (*__gl_real_immed->%s.%s)", table, funcName);
    headerLen = length(header);
    printf("%s", header);
    if (numParam == 0) {
	printf("();\n");
    } else {
	printf("(");
	for (n = 0; n < numParam; n++) {
	    #if (n > 0) indentify(headerLen+1);
	    pnamestr = pname[n];
	    if (substr(pname[n],1,1)=="*") {
	      sub("*", "", pnamestr);
	    }
	    printf("%s", pnamestr);
	    if (n < numParam-1) printf(",");
	    else printf(");\n");
	}
    }

    #
    # Restore the dispatch table entry.
    #
    if (index(funcName,"Vertex2") > 0) {
	printf("\tgc->dispatchState->vertex.Vertex2fv = __gldb_Vertex2fv;\n");
    }
    if (index(funcName,"Vertex3") > 0) {
	printf("\tgc->dispatchState->vertex.Vertex3fv = __gldb_Vertex3fv;\n");
    }
    if (index(funcName,"Vertex4") > 0) {
	printf("\tgc->dispatchState->vertex.Vertex4fv = __gldb_Vertex4fv;\n");
    }

    # 
    # Increment the counter for this routine.
    #
    printf("\n");
    printf("\t__gl_debug_table.%s++;\n", funcName);

    #
    # Some routines have shared counter, so update those too.
    # Yes, this is just a subset of the routines under "table" 
    # above, but the Index et al routines are not that important yet.
    #
    groupName = "";
    if (index(funcName,"Vertex2") > 0 ||
	index(funcName,"Vertex3") > 0 ||
	index(funcName,"Vertex4") > 0) {
      groupName = "Vertex";
    } else if (index(funcName,"Color3") > 0 ||
	       index(funcName,"Color4") > 0) {
      groupName = "Color";
    } else if (index(funcName,"Normal3") > 0) {
      groupName = "Normal";
    } else if (index(funcName,"TexCoord1") > 0 ||
	index(funcName,"TexCoord2") > 0 ||
	index(funcName,"TexCoord3") > 0 ||
	index(funcName,"TexCoord4") > 0) {
      groupName = "TexCoord";
    }
    if (groupName != "") {
      printf("\t__gl_debug_table.%s++;\n", groupName);
    }

    #
    # Begin, End, and Vertex need to call special funcs.
    #
    if (funcName == "Begin") {
	printf("\t__gldb_StatBegin(mode);\n");
    } else if (funcName == "End") {
	printf("\t__gldb_StatEnd();\n");
    } else if (index(funcName,"Vertex2") > 0 ||
	       index(funcName,"Vertex3") > 0 ||
	       index(funcName,"Vertex4") > 0) {
	printf("\t__gldb_StatVertex();\n");
    }

    printf("}\n");
    printf("\n");
}

END {
}

