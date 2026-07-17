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
** $Date: 10/11/00 7:28:07 PM$ 
**
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef __WIN32__
#include <conio.h>
#endif
#include <glide.h>
#include <atrender.h>
#include <atscene.h>
#include <atinput.h>
#include <atdemo.h>
#include <ataudio.h>

/*
 * GLOBAL VARIABLES
 */


#define MAX_SHAPES 3

typedef FxU32 tri[3];
AtrVertex *vList;
tri       *triList;
FxU32     numVerts;
FxU32     numTris;
int whichView = 0;
AtrTriSet *sphereModel, *cubeModel, *planeModel;

AtrCamera   *camera;
AtrMaterial *cubeMaterial, *planeMaterial;
AtrXform    *toObject;
AtrTexHandle tex[30];
AtrImg       *img[30];
AtrLight    *light;

/* projected texture stuff */

AtrImg      *gProjImg;
AtrMaterial *gProjMat;
AtrTexHandle gProjTex, gPlaneProjTex;
AtrLight    *gProjLight;
char        *projImageFilename = "projgord.3df";

AtrStats    stats;
float       distance = 10.0f;
float       xShift = 0.;
float       angle;
float       radius = 1.0f;
FxU32       subdivisions = 1;
FxBool      immediate = 0;
AtrCanvas  *canvas;
AtmVector3 position;
FxBool gPaused = FALSE;
int whichTexture =0, numTextures = 0;
Sound *sound = NULL;
FxBool sound_state = FXFALSE;
AtrEnv      *env;
FxBool     gProjOn = FXFALSE;
AtmVector3  lightPosition  = {  0.0f, 0.0f,  3.0f },
            lightTarget    = {  0.0f,  0.0f,   0.0f },
            up             = {  0.0f,  1.0f,   0.0f };
int gShape = 1;

static AtsNode *findObject(AtsNode *top_level, char *modelName, AtsType *t) {
    AtsNode *model;

    if (( model = atsDictFind(top_level, modelName, t)) == NULL ) {
        atuError(FXTRUE, "Could not find model %s\n", modelName);
    }

    return model;
}

static void CreateObjects( void ) {
    char *modelFileName = "cube.bof";
    AtsNode *top_level;
    AtsType *dictType = atsGetTypeFromName("Dict");
    AtsType *imageType = atsGetTypeFromName("Image");

    if (( top_level = atsFileLoad( modelFileName, dictType )) == NULL ) {
        atuError(FXTRUE, " could not load cube models %s\n", modelFileName);
    }

    /*
     * Create all graphics objects.
     */


    _atGlobals.helpImage = (AtrImg*)findObject(top_level, "helpscreen", imageType);
}

AtrTriSet *buildPlane( float s ) {
    AtrTriSet *set = atrTriSetAllocate( 1 );
    AtrVertex verts[4];

    atrTriSetBegin( set );

    verts[0].x  = -s;
    verts[0].y  = s;
    verts[0].z  = 0.0f;
    verts[0].i  = 0.0f;
    verts[0].j  = 0.0f;
    verts[0].k  = -1.0f;
    verts[0].s0 = 0.0f;
    verts[0].t0 = 1.0f;
   
    verts[1].x  = s;
    verts[1].y  = s;
    verts[1].z  = 0.0f;
    verts[1].i  = 0.0f;
    verts[1].j  = 0.0f;
    verts[1].k  = -1.0f;
    verts[1].s0 = 1.0f;
    verts[1].t0 = 1.0f;
   
    verts[2].x  = s;
    verts[2].y  = -s;
    verts[2].z  = 0.0f;
    verts[2].i  = 0.0f;
    verts[2].j  = 0.0f;
    verts[2].k  = -1.0f;
    verts[2].s0 = 1.0f;
    verts[2].t0 = 0.0f;
   
    verts[3].x  = -s;
    verts[3].y  = -s;
    verts[3].z  = 0.0f;
    verts[3].i  = 0.0f;
    verts[3].j  = 0.0f;
    verts[3].k  = -1.0f;
    verts[3].s0 = 0.0f;
    verts[3].t0 = 0.0f;
   
    atrTriSetVertex( set, &verts[0] );
    atrTriSetVertex( set, &verts[1] );
    atrTriSetVertex( set, &verts[2] );

    atrTriSetVertex( set, &verts[0] );
    atrTriSetVertex( set, &verts[2] );
    atrTriSetVertex( set, &verts[3] );
 
    atrTriSetEnd( set );
    return set;
}

static void createModels( float radius, int subdivisions ) {
    sphereModel = atrPrimSphere( radius, subdivisions * 4, subdivisions * 4 );
	cubeModel = atrPrimCube(radius*2.0f);
    planeModel = buildPlane(4.0f);
}
  

static void drawSphereModel( void ) {
    atrRenderTriSet( sphereModel );
}

static void drawCubeModel( void ) {
    if ( _atGlobals.wireframe )
         atrRenderTriSetWF( cubeModel );
    else atrRenderTriSet( cubeModel );
}

#define STEP 0.1f

static char *usage = 
    "[-r radius][-s subdivisions]\n"                
    "-r radius - where radius is a positive floating point value\n"
    "            the default radius is `1.0f\n"
    "-s subdivisions - where subdivisions is the integer number of times\n"
    "                  to re-subdivide the starting shape.\n"
    "-i        - render in immediate mode.\n";

void kbFunc(AtiKeyEvent *ev, FxBool preRecorded);

void kbFunc(AtiKeyEvent *ev, FxBool preRecorded) {
    FXUNUSED(preRecorded);

    if ( ev->state == ATI_KEY_PRESS ) {
        
        switch( ev->code ) {
        case ATI_KEY_P:
            gProjOn = !gProjOn;
            break;
		case ATI_KEY_UP:
			distance += STEP;
			break;
		case ATI_KEY_DOWN:
			distance -= STEP;
			break;
		case ATI_KEY_LEFT:
			xShift -= .1;
			break;
		case ATI_KEY_RIGHT:
			xShift += .1;
			break;
		case ATI_KEY_F2: 
			 whichTexture = (whichTexture+1)%numTextures;
             break;
		case ATI_KEY_F3: 
             gShape = ( gShape+1) % MAX_SHAPES;
        case ATI_KEY_KEYPAD_PLUS:
             whichView++;
             if ( whichView > 3 )
                 whichView = 0;
        }
    }
}

void
AppRenderScene( FxU32 displayWidth, FxU32 displayHeight) {
    int i;
    FXUNUSED (displayWidth);

	if ( sound_state != _atGlobals.sound ) {
        if ( _atGlobals.sound )
            ataSoundPlay(sound, ATA_SOUND_LOOP);
        else ataSoundStop( sound );
        sound_state = _atGlobals.sound;
	}

   /*--------------------------------------------------------
     Clear The WBUFFER and the BACKBUFFER
     --------------------------------------------------------*/

    if ( _atGlobals.fog ) {
        atrClearCanvas( 0.5f, 0.5f, 0.5f, ATR_WBUFFER_CLEAR );
    } else {
        atrClearCanvas( 0.0f, 1.0f, 0.0f, ATR_WBUFFER_CLEAR );
    }
                
    atrAddLight( light, 0 );

    if ( gProjOn )    {
        atrAddLight( gProjLight , 0 );
        if ( _atGlobals.caps.numTex == 1 ) {
			atrMaterialSetup( cubeMaterial, ATR_MAT_DECAL );
			if ( _atGlobals.fog ) {
			    atrMaterialSetup( gProjMat, ATR_MAT_TEX_PROJECTED|ATR_MAT_FB_MULT_MP_FOG );
			} else {
				atrMaterialSetup( gProjMat, ATR_MAT_TEX_PROJECTED|ATR_MAT_FB_MULTIPLY );
			}

			cubeMaterial->texture[0] = tex[whichTexture];
			cubeMaterial->successor = gProjMat;
        } else {
            atrMaterialSetup( cubeMaterial, ATR_MAT_TEX_DECAL_X_PROJECTED | 
                                            ATR_MAT_FB_ASSIGN );
			cubeMaterial->texture[0] = tex[whichTexture];
			cubeMaterial->texture[1] = gProjTex;
        }
		atrMaterialSetup( planeMaterial, ATR_MAT_TEX_PROJECTED | 
                                         ATR_MAT_FB_ASSIGN );
		planeMaterial->texture[0] = gPlaneProjTex;
    } else {
        cubeMaterial->successor = NULL;
        atrMaterialSetup( cubeMaterial, ATR_MAT_DECAL );
	    atrMaterialSetup( planeMaterial, ATR_MAT_GSHADE );
    }

    atmVector3Set( position, xShift, 0.0f, distance );

    atmVector3Assign( &camera->lcsToWCS.data[12], position );
        
    atrXformSetYRotation( toObject, angle );

    switch ( whichView ) {
    case 0:
	    camera->fovRadians = 60.0f*ATM_DEGREE;
        camera->xOffset = 0;
        break;
    case 1:
        camera->xOffset = 2; /* -20.0f*ATM_DEGREE; */
	    camera->fovRadians = 20.0f*ATM_DEGREE;
        break;
    case 2:
        camera->xOffset = 0; /* -20.0f*ATM_DEGREE; */
	    camera->fovRadians = 20.0f*ATM_DEGREE;
        break;
    case 3:
        camera->xOffset = -2; /* -20.0f*ATM_DEGREE; */
	    camera->fovRadians = 20.0f*ATM_DEGREE;
        break;
    }

    atrSelectCanvas( canvas );
    atrSelectCamera( camera );

    if ( _atGlobals.fog ) {
        if ( gProjOn &&  ( _atGlobals.caps.numTex == 1 )) {
            env->flags = ATR_HSR_WBUFFER|ATR_FOG_ON_DEPTH_MP;
        } else {
            env->flags = ATR_HSR_WBUFFER|ATR_FOG_ON_DEPTH;
        }
    } else {
        env->flags = ATR_HSR_WBUFFER;
    }

    atrEnvFogTable(env, ATR_FOGFUNC_LINEAR, 0.2f, 1.0f, 10.0f*radius);

    atrPushEnv( env );

    for ( i = 0 ; i < 5; i++ ) {

        toObject->data[12] = i*3-7.5;
        atrPushXform( toObject );

		cubeMaterial->texture[0] = tex[i];
	    atdPushMaterial( cubeMaterial );

        switch ( gShape ) {
        case 0:
            break;
        case 1:
            drawCubeModel();
            break;
        case 2:
            drawSphereModel();
            break;
        }

	    atdPopMaterial( FXFALSE );

        atrPopXform( FXTRUE );
    }

    atrPopEnv( FXTRUE );

    if ( _atGlobals.fog ) {
        env->flags = ATR_HSR_WBUFFER|ATR_FOG_ON_DEPTH;
    } else {
        env->flags = ATR_HSR_WBUFFER;
    }

    atrPushEnv( env );

	atdPushMaterial( planeMaterial );

    atrRenderTriSet( planeModel );

	atdPopMaterial( FXFALSE );

    atrPopEnv( FXTRUE );

    atrRemoveLight( 0 );

	atrDrawString( "Hello World", 30.0f, 30.0f, 0.5f, displayHeight-30.0f);

    angle += ATM_DEGREE;
    if ( angle > 2.0f * ATM_PI )
        angle = 0.0f;
}

static void
InitTexture(char *name) {
	img[numTextures] = atrImgAllocate( 1 );
	if (!atrImgCreateFromFile( img[numTextures], name )) {
		atuError(FXTRUE, "Can't find image file %s\n", name);
	}
	tex[numTextures] = atrTexNewHandle( 0 );
    atrTexAssociate( tex[numTextures], img[numTextures] );
	numTextures++;
}

static void
resetFunc(void) {
    whichTexture = 0;
	distance = radius * -5.0f;
    cubeMaterial->texture[0] = tex[whichTexture];
    gShape = 1;
    gProjOn = FXFALSE;
    _atGlobals.fog = FXFALSE;
}

static void
AppInit(void) {
    atdSetName("Cube");
    atdSetHelpFile("fliphelp.tga");

    atdEventInit();
    atdKeyboardFunc(kbFunc);
    atdResetFunc(resetFunc);

    if ( !atsInit( ) ) {
      atuError( FXTRUE, "Couldn't initialize scene library.\n" );
    }

    if ( _atGlobals.soundAvailable ) {
        if ((sound = ataSoundNew("sndtrak.wav")) == NULL ) {
            atuError(FXTRUE, "could not load sound\n");
        }
    }
    
    env = atrEnvAllocate( 1 );
    env->fogColor.r = 0.5f;
    env->fogColor.g = 0.5f;
    env->fogColor.b = 0.5f;
    env->flags = ATR_HSR_WBUFFER;
    atrPushEnv( env );

    atuSetLoadPath( getenv( "AT_LOAD_PATH" ) );

    // CreateObjects();

	InitTexture("anubis.3df");
    InitTexture( "marble.3df" );
    // InitTexture( "win95.ppm" );
	InitTexture( "ramp.3df" );
	InitTexture( "fire1.3df" );
    InitTexture("3dfx.3df");
    InitTexture("tank.3df");

    cubeMaterial = atrMaterialAllocate( 1 );
    cubeMaterial->texture[0] = tex[whichTexture];
    cubeMaterial->diffuse.r = 1.0f;
    cubeMaterial->diffuse.g = 1.0f;
    cubeMaterial->diffuse.b = 1.0f;
	cubeMaterial->atestFunc = GR_CMP_ALWAYS ; // GR_CMP_GREATER ; // GR_CMP_ALWAYS
	cubeMaterial->aReference = 40;

    planeMaterial = atrMaterialAllocate( 1 );
    planeMaterial->diffuse.r = 1.0f;
    planeMaterial->diffuse.g = 1.0f;
    planeMaterial->diffuse.b = 1.0f;
    planeMaterial->texture[0] = gPlaneProjTex;

    light = atrLightAllocate( 1 );
    light->flags = ATR_LIGHT_DIRECTED;
    light->color.r = 1.0f;
    light->color.g = 1.0f;
    light->color.b = 1.0f;
//    atrXformSetYRotation( &light->lcsToWCS, 30.0f * ATM_DEGREE );

    /* projected texture stuff */

    gProjImg = atrImgAllocate( 1 );

    if ( !atrImgCreateFromFile( gProjImg, projImageFilename ) )
        atuError( FXTRUE, "Couldn't load %s\n", projImageFilename );

    gProjImg->nLevels = 1;

    gProjLight = atrLightAllocate( 1 );
    gProjLight->flags = ATR_LIGHT_PROJECTOR;
    gProjLight->project.tag = 0;
    gProjLight->project.fovInX = 100.0f * ATM_DEGREE;
    gProjLight->project.fovInY = 100.0f * ATM_DEGREE;
    atrXformPointAt( &gProjLight->lcsToWCS, lightTarget, lightPosition, up );

#ifdef notdef
    /* initialize position */
    atmVector3Assign( &gProjLight->lcsToWCS.data[12], lightPosition );
#endif

    if ( _atGlobals.caps.numTex > 1 ) {
        gProjTex = atrTexNewHandle( ATR_TEXELFX_1 ); /* choose the second TMU */
		cubeMaterial->projectedTag = 0;
    } else {
        gProjTex = atrTexNewHandle( ATR_TEXELFX_0 ); /* choose the first TMU */
        gProjMat = atrMaterialAllocate( 1 );
		
        gProjMat->texture[0] = gProjTex;
        gProjMat->projectedTag = 0;
    }
	atrTexAssociate( gProjTex, gProjImg );
    gPlaneProjTex = atrTexNewHandle( ATR_TEXELFX_0 );
    atrTexAssociate( gPlaneProjTex, gProjImg );

    toObject = atrXformAllocate( 1 );
    atrXformSetIdentity( toObject );

    camera = atrCameraAllocate( 1 );

    canvas      = atrCanvasAllocate( 1 );

    angle = 0.0f;
    createModels( radius, subdivisions );
    atrStatsReset();
    resetFunc();
}

int
AppMain(int argc, char **argv) {

    FXUNUSED(argc);
    FXUNUSED(argv);

    AppInit();

	atdEventLoop();

    return 1;
}
