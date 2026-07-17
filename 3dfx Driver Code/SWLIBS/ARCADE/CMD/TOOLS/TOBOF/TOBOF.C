/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
**
** $Revision: 4$ 
** $Date: 10/11/00 7:30:03 PM$ 
**
** Convert one or more files into bof format placing the results in a single database
*/

#include <stdio.h>
#include <string.h>
#include <atscene.h>

typedef FxBool (* CmdProc)(int argc, char **argv);

FxBool setPath(int argc, char **argv);
FxBool processSourceFile(int argc, char **argv);
FxBool loadObject(int argc, char **argv);
FxBool loadTexture(int argc, char **argv);
FxBool loadImage(int argc, char **argv);
FxBool loadMovie(int argc, char **argv);
FxBool loadCMesh(int argc, char **argv);
FxBool loadSeq(int argc, char **argv);
FxBool verbose(int argc, char **argv);

typedef struct {
    char *cmdName;
    CmdProc cmdProc;
} CmdEntry;

typedef struct {
    char *name;
    int   value;
} EnumDesc;

/* set of available commands must be NULL terminated */

CmdEntry cmdList[] = { 
                       { "p", setPath },
                       { "path", setPath },
                       { "i", processSourceFile},
                       { "include", processSourceFile},
                       { "o", loadObject},
                       { "object", loadObject},
                       { "t", loadTexture},
                       { "texture", loadTexture},
                       { "image", loadImage},
                       { "m", loadMovie},
                       { "movie", loadMovie},
                       { "c", loadCMesh},
                       { "cmesh", loadCMesh},
                       { "seq", loadSeq},
                       { "sequence", loadSeq},
                       { "verbose", verbose},
                       {NULL, NULL }};

FxBool fPrintTree = FXFALSE;
char *outFileName = NULL;
AtsNode *dict = NULL;
FxU32 lineNumber = 0;
char *currentFile = "None";

FxBool executeCommand(int argc, char **argv) {
     CmdEntry *ent;

     for ( ent = cmdList; ent->cmdName; ent++ ) {
         if ( atuStringCompare(ent->cmdName, argv[0]) )
              return ((* ent->cmdProc) ( argc, argv ));
     }

     atuError(FXFALSE, "Unknown command %s\n", argv[0]);
     return FXFALSE;
}

FxBool setPath(int argc, char **argv) {
    char buff[1024];
    int i;
    
    if ( argc == 1 ) {
        atuError(FXFALSE, "missing path specification\n");
        return FXFALSE;
    }

    buff[0] = 0;

    for ( i = 1; i < argc; i++ )
        strcat(buff, argv[i]);

    atuSetLoadPath(buff);

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: printUseage
  Date: 6/8/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Help info for tobof
  Arguments:
    None
  Return:
    Pointer to loaded shape, or NULL if error
  -------------------------------------------------------------------*/

static void printUseage( void )
{
    fprintf(stdout, "useage: tobof options <filenames> \n"
      "options: \n"
      "    -h                  to get this message\n"
      "    -o <filename>       output file name\n"
      "    -f <filename>       command file to process\n"
      "\n");
}

/*-------------------------------------------------------------------
  Function: basename
  Date: 6/8/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Remove extension from file name
  Arguments:
    fileName - original filename
    buff     - working buffer (used to contain name minus extension)
  Return:
    Name minus file extension
  -------------------------------------------------------------------*/

char *basename(const char *fileName, char *buff) {
    char *p;
    const char *q, *r;

    p = buff; q = fileName; r = q+strlen(fileName);

    while (( *q != '.' ) && ( q != r ))
        *p++ = *q++;

    *p = 0;

    return buff;
}

/*-------------------------------------------------------------------
  Function: processSourceFile
  Date: 6/8/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Process a file containing a list of comnmands. See readme.txt for
    details.
    
  Arguments:
    sourceFileName name of file containing commands
  Return:
    Returns FXTRUE on success or FXFALSE on errors
  -------------------------------------------------------------------*/

FxBool processSourceFile(int argc, char **argv) {
    FILE *fp;
    char buf[256];
    char *token;
    int numArgs;
    FxU32 _lineNumber = lineNumber;
    char* _currentFile = currentFile;

#define MAX_ARGS 20

    char *argList[MAX_ARGS];

    if ( argc == 1 ) {
        atuError(FXFALSE, "missing source file name\n");
        return FXFALSE;
    }

    if ((fp = fopen(argv[1], "r")) == NULL ) {
        atuError(FXFALSE, "Can't open file %s\n", argv[1]);
        return FXFALSE;
    }

    currentFile = argv[1];
    lineNumber = 1;
    while (!feof(fp)) {
        lineNumber++;
        fgets(buf, sizeof(buf), fp);
        if (feof(fp))
           break;

        numArgs = 0;

        for( token = strtok( buf, " \r\n\t" ); token;
             token = strtok( NULL, " \r\n\t" ) ) {
            if (numArgs == MAX_ARGS ) {
                lineNumber = _lineNumber;
                currentFile = _currentFile ;
                atuError(FXFALSE, "too many arguments max is %d\n", MAX_ARGS);
                return FXFALSE;
            }
            argList[numArgs++] = token;
        }

        if (( numArgs == 0 ) || ( argList[0][0] == '#' ))
            continue;

        if (!executeCommand(numArgs, argList)) {
            atuError(FXTRUE, "Error on line %d file %s processing terminated\n",
                     lineNumber, currentFile);
        }
    }

    currentFile = _currentFile ;
    lineNumber = _lineNumber;
    return FXTRUE;
}

FxBool loadObject(int argc, char **argv) {
    AtmSphere bsphere;
    AtsNode *n = NULL;
    const char *name = NULL;
    char *fileName;
    FxBool useAnimation = FXFALSE;
    AtsType *objectType ;
    int i;
    int wrapFix = FXFALSE;
    float vertexScale = 1.0f;

    /* parse command line */

    for ( i = 1; i < argc; i++ ) {
        if ( atuStringCompare(argv[i], "-name") && ( i < (argc-1)))  {
            name = argv[++i];
        } else if ( atuStringCompare(argv[i], "-file") && ( i < (argc-1)))  {
            fileName = argv[++i];
        } else if ( atuStringCompare(argv[i], "-anim") )  {
            useAnimation = FXTRUE;
        } else if ( atuStringCompare(argv[i], "-vs") && ( i < (argc-1)))  {
            vertexScale = (float)atof(argv[++i]);
            atsConverterFloatAttr(NULL, ATS_CATTR_VERTEX_SCALE, vertexScale);
        } else if ( atuStringCompare(argv[i], "-wf") ) {
            wrapFix = FXTRUE;
        } else {
            atuError(FXTRUE, "unknown argument %s\n", argv[i]);
        }
    }

    atsConverterIntAttr(NULL, ATS_CATTR_TEXTURE_WRAP_FIX, wrapFix);

    if ( fileName == NULL ) {
        atuError(FXFALSE, "Object filename not specified\n");
        return FXFALSE;
    }

    if ( name )
         printf("loading node %s from file %s\n", name, fileName);
    else printf("loading node from %s\n", fileName);

    if ( useAnimation )
          objectType = atsGetTypeFromName("Anim");
    else  objectType = atsGetTypeFromName("Node");

    /* load the object */

    if (( n = atsFileLoad(fileName, objectType)) == NULL ) {
        printf("could not load file %s\n", fileName);
        return FXFALSE;
    }

    /* compute objects bounding volume */

    atsComputeBSphere(n, &bsphere);
    atsNodeBSphere(n, &bsphere);

    /* set objects name */
    
    if (( name == NULL ) && ( (name = atsNodeGetName(n)) == NULL )) {
        name = fileName;
    }

    /* add object to dictionary */

    atsDictAdd(dict, name, n);
}

int findEnum(char *s, EnumDesc *enumList) {
    int i ;

    for ( i = 0; enumList[i].name != NULL; i++ ) {
        if ( atuStringCompare(s, enumList[i].name) )
            return enumList[i].value;
    }

    atuError(FXTRUE, "unknown value %s at line %d\n", s);
}

EnumDesc seqModeList[] = { { "ATS_SEQ_CYCLE", ATS_SEQ_CYCLE },
                         { "ATS_SEQ_SWING", ATS_SEQ_SWING },
                         { NULL, 0 }};

EnumDesc seqBasisList[] = { { "ATS_SEQ_TIME", ATS_SEQ_TIME },
                            { "ATS_SEQ_FRAME", ATS_SEQ_FRAME },
                            { NULL, 0 }};

FxBool verbose(int argc, char **argv) {
    int verbose = FXTRUE;

    if ( argc > 1 )
         verbose = atoi(argv[1]);

    atsConverterIntAttr(NULL, ATS_CATTR_VERBOSE, verbose);
   
    return FXTRUE;
}

FxBool loadSeq(int argc, char **argv) {
    AtmSphere bsphere;
    AtsNode *n = NULL;
    const char *name = NULL;
    char *fileName;
    float duration = 1.0f;
    int mode = ATS_SEQ_CYCLE;
    int basis = ATS_SEQ_TIME;
    int numReps = 0;
    int startFrame = 0;
    int endFrame = 0;
    AtsType *objectType ;
    int i;
    float vertexScale;

    /* parse command line */

    for ( i = 1; i < argc; i++ ) {
        if ( atuStringCompare(argv[i], "-name") && ( i < (argc-1)))  {
            name = argv[++i];
        } else if ( atuStringCompare(argv[i], "-file") && ( i < (argc-1)))  {
            fileName = argv[++i];
        } else if ( atuStringCompare(argv[i], "-duration") && ( i < (argc-1))) {
            duration = (float)atof(argv[++i]);
        } else if ( atuStringCompare(argv[i], "-numreps") && ( i < (argc-1))) {
            numReps = atoi(argv[++i]);
        } else if ( atuStringCompare(argv[i], "-startFrame") && ( i < (argc-1))) {
            startFrame = atoi(argv[++i]);
        } else if ( atuStringCompare(argv[i], "-endFrame") && ( i < (argc-1))) {
            endFrame = atoi(argv[++i]);
        } else if ( atuStringCompare(argv[i], "-mode") && ( i < (argc-1))) {
            mode = findEnum(argv[++i], seqModeList);
        } else if ( atuStringCompare(argv[i], "-basis") && ( i < (argc-1))) {
            basis = findEnum(argv[++i], seqBasisList);
        } else if ( atuStringCompare(argv[i], "-vs") && ( i < (argc-1)))  {
            vertexScale = (float)atof(argv[++i]);
            atsConverterFloatAttr(NULL, ATS_CATTR_VERTEX_SCALE, vertexScale);
        } else {
            atuError(FXTRUE, "unknown argument %s\n", argv[i]);
        }
    }

    if ( fileName == NULL ) {
        atuError(FXFALSE, "Object filename not specified\n");
        return FXFALSE;
    }

    atsConverterIntAttr(NULL, ATS_CATTR_START_FRAME, startFrame);
    atsConverterIntAttr(NULL, ATS_CATTR_END_FRAME, endFrame);

    if ( name )
         printf("loading sequence %s from file %s frames (%d, %d)\n", 
                 name, fileName, startFrame, endFrame);
    else printf("loading sequence from frames (%d, %d)%s\n", 
                 fileName, startFrame, endFrame);

    objectType = atsGetTypeFromName("Seq");

    /* load the object */

    if (( n = atsFileLoad(fileName, objectType)) == NULL ) {
        printf("could not load file %s\n", fileName);
        return FXFALSE;
    }

    /* compute objects bounding volume */

    atsComputeBSphere(n, &bsphere);
    atsNodeBSphere(n, &bsphere);
  
    atsSeqDuration(n, duration);
    atsSeqBasis(n, basis);
    atsSeqMode(n, mode);
    atsSeqReps(n, numReps);

    /* set objects name */
    
    if (( name == NULL ) && ( (name = atsNodeGetName(n)) == NULL )) {
        name = fileName;
    }

    /* add object to dictionary */

    atsDictAdd(dict, name, n);
}

FxBool loadCMesh(int argc, char **argv) {
    AtsObject *obj = NULL;
    const char *name = NULL;
    float grid_size = 0.0f;
    char *fileName;
    int i;

    /* parse command line */

    for ( i = 1; i < argc; i++ ) {
        if ( atuStringCompare(argv[i], "-name") && ( i < (argc-1)))  {
            name = argv[++i];
        } else if ( atuStringCompare(argv[i], "-file") && ( i < (argc-1)))  {
            fileName = argv[++i];
        } else if ( atuStringCompare(argv[i], "-gridSize") && ( i < (argc-1))){
            grid_size = (float)atof(argv[++i]);
        } else {
            atuError(FXTRUE, "unknown argument %s\n", argv[i]);
        }
    }

    if ( fileName == NULL ) {
        atuError(FXFALSE, "Object filename not specified\n");
        return FXFALSE;
    }

    if ( name )
         printf("loading collision mesh %s from file %s\n", name, fileName);
    else printf("loading collision mesh from %s\n", fileName);

    atsConverterFloatAttr(NULL, ATS_CATTR_CMESH_GRID_SIZE, grid_size);

    /* load the object */

    if (( obj = atsFileLoad(fileName, atsGetTypeFromName("CMesh"))) == NULL ) {
        printf("could not load file %s\n", fileName);
        return FXFALSE;
    }

    /* set objects name */
    
    if ( name == NULL ) {
        name = fileName;
    }

    /* add object to dictionary */

    atsDictAdd(dict, name, obj);
}

FxBool loadImage(int argc, char **argv) {
    AtsObject *t = NULL;
	AtrImg  *img;
    const char *name = NULL;
    char *fileName;
    int i;

    /* parse command line */

    for ( i = 1; i < argc; i++ ) {
        if ( atuStringCompare(argv[i], "-name") && ( i < (argc-1)))  {
            name = argv[++i];
        } else if ( atuStringCompare(argv[i], "-file") && ( i < (argc-1)))  {
            fileName = argv[++i];
        } else {
            atuError(FXTRUE, "unknown argument %s\n", argv[i]);
        }
    }

    if ( fileName == NULL ) {
        atuError(FXFALSE, "Object filename not specified\n");
        return FXFALSE;
    }

    if ( name )
         printf("loading image %s from file %s\n", name, fileName);
    else printf("loading image file %s\n", fileName);

    /* load the object */

    if (( t = atsImageCreateFromFile(fileName)) == NULL ) {
        printf("could not load image file %s\n", fileName);
        return FXFALSE;
    }

	img = (AtrImg *)t;

    /* set objects name */
    
    if ( name == NULL ) {
        name = fileName;
    }

    /* add object to dictionary */

    atsDictAdd(dict, name, t);
}

FxBool loadTexture(int argc, char **argv) {
    AtsObject *t = NULL;
    const char *name = NULL;
    char *fileName;
    int i;

    /* parse command line */

    for ( i = 1; i < argc; i++ ) {
        if ( atuStringCompare(argv[i], "-name") && ( i < (argc-1)))  {
            name = argv[++i];
        } else if ( atuStringCompare(argv[i], "-file") && ( i < (argc-1)))  {
            fileName = argv[++i];
        } else {
            atuError(FXTRUE, "unknown argument %s\n", argv[i]);
        }
    }

    if ( fileName == NULL ) {
        atuError(FXFALSE, "Object filename not specified\n");
        return FXFALSE;
    }

    if ( name )
         printf("loading texture %s from file %s\n", name, fileName);
    else printf("loading texture file %s\n", fileName);

    /* load the object */

    if (( t = atsFileLoad(fileName, atsGetTypeFromName("Texture"))) == NULL ) {
        printf("could not load texture file %s\n", fileName);
        return FXFALSE;
    }

    /* set objects name */
    
    if ( name == NULL ) {
        name = fileName;
    }

    /* add object to dictionary */

    atsDictAdd(dict, name, t);
}

FxBool loadMovie(int argc, char **argv) {
    AtsObject *t = NULL;
    const char *name = NULL;
    char *fileName;
    int startFrame = 0;
    int endFrame = 0;
    int i;

    /* parse command line */

    for ( i = 1; i < argc; i++ ) {
        if ( atuStringCompare(argv[i], "-name") && ( i < (argc-1)))  {
            name = argv[++i];
        } else if ( atuStringCompare(argv[i], "-file") && ( i < (argc-1)))  {
            fileName = argv[++i];
        } else if ( atuStringCompare(argv[i], "-startFrame") && ( i < (argc-1))) {
            startFrame = atoi(argv[++i]);
        } else if ( atuStringCompare(argv[i], "-endFrame") && ( i < (argc-1))) {
            endFrame = atoi(argv[++i]);
        } else {
            atuError(FXTRUE, "unknown argument %s\n", argv[i]);
        }
    }

    if ( fileName == NULL ) {
        atuError(FXFALSE, "Object filename not specified\n");
        return FXFALSE;
    }

    if ( name )
         printf("loading movie %s from file %s start frame %d, end frame %d\n", 
                name, fileName, startFrame, endFrame);
    else printf("loading movie file %s start frame %d, end frame %d\n", 
                fileName, startFrame, endFrame);

    /* load the movie */
 
    if (( t = atsTextureMovie( fileName, startFrame, endFrame )) == NULL ) {
        printf("could not load movie file %s\n", fileName);
        return FXFALSE;
    }

    /* set objects name */
    
    if ( name == NULL ) {
        name = fileName;
    }

    /* add object to dictionary */

    atsDictAdd(dict, name, t);
}

/*-------------------------------------------------------------------
  Function: parseArgs
  Date: 6/8/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    process command arguments
  Arguments:
    None
  Return:
    Returns FXTRUE on success or FXFALSE on errors
  -------------------------------------------------------------------*/

FxBool parseArgs(int argc, char **argv) {
    FxI32 idx;
    char *tmpVec[2];

    for ( idx = 1; idx < argc; idx++ ) {
        if ( argv[idx][0] == '-' ) { /* argument starts with a '-' */
            switch ( argv[idx][1] ) {
            case 'f':
                if ( idx >= argc-1 ) {
                    atuError(FXFALSE, "missing filename\n");
                    return FXFALSE;
                }
                idx++;
                tmpVec[0] = argv[0];
                tmpVec[1] = argv[idx];
                if (!processSourceFile(2, tmpVec))
                    return FXFALSE;
                continue;
            case 'o':
                if ( idx >= argc-1 ) {
                    atuError(FXFALSE, "missing output filename\n");
                    return FXFALSE;
                }
                idx++;
                outFileName = argv[idx];
                continue;
            case 'p':
                fPrintTree = FXTRUE;
                continue;
            case 'h':
                printUseage();
                continue;
            default:
                atuError(FXFALSE, "unknown option %s\n", argv[idx]);
                return FXFALSE;
            }
        } 
    }

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: main (tobof)
  Date: 6/8/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Load a list of object descriptions into a single database
  Arguments:
    argc - number of arguments
    argv - arguments
  Return:
    status code 
  -------------------------------------------------------------------*/

main(int argc, char **argv) {
    AtrDriverInfo info;

    /* initialize rendering library */

    info.info = (void *)1; /* number of TMU's */

    if (!atrInit("XXX", &info))
        atuError( FXTRUE, "Couldn't initialize rendering library.\n" );

    /* initialize scene manager */

    if (!atsInit())
        atuError( FXTRUE, "Couldn't initialize scene manager.\n" );

    atuSetLoadPath(getenv("AT_LOAD_PATH"));

    if ((dict = atsDictNew()) == NULL ) {
        atuError(FXTRUE, "Could not create dictionary\n");
    }

    if ( !parseArgs(argc, argv)) {
        atuError(FXTRUE, "%s command error\n", argv[0]);
    }

    if ( fPrintTree ) {
        atsPrint(dict, stdout, 0, 0);

        printf("press <RETURN> to continue\n");
        getchar();
    }

    if ( outFileName == NULL ) {
        outFileName = "group.bof"; 
    }

    printf("writing to file %s\n", outFileName);

    if ( atsFileStore(outFileName, dict) != FXTRUE ) {
        atuError(FXTRUE, "Error creating file %s\n", outFileName);
    }

    atsShutdown();

    return 0;
}
