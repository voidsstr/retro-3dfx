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
** $Date: 10/11/00 7:29:24 PM$ 
**
*/

#include <atrender.h>
#include <atinput.h>
#include <glide.h>
#include <math.h>
#include <atdemo.h>

/*----------------------------------------------------------------------------
  Types
  ----------------------------------------------------------------------------*/

typedef struct _OBJECT {
    char        *name;
    AtrMaterial *surface;
    AtrMaterial *shadow;
    AtrTriSet   *geom;
    AtrXform    *pos;
    FxU8        shadowId;
    struct _OBJECT *next;
} Object;

typedef struct {
    AtrLight *projector;
    AtrLight *light;
    AtrXform *pos;
} LightSource;

typedef struct {
    AtrCanvas *canvas;
    AtrCamera *camera;
    AtrXform  *pos;
    AtrColor  clearColor;
} PointOfView;


/*----------------------------------------------------------------------------
  Globals
  ----------------------------------------------------------------------------*/
static char *APP_NAME = "Shadow";

static char    gKeyMap[255];
static FxBool  gTrigger;
static FxBool  gButton;
static FxFloat gJoyX;
static FxFloat gJoyY;
static FxBool  gDone;
static FxBool  gShowProj;

static Object       *head;
static AtrImg       *shadowImg;
static LightSource  *light;
static AtrTexHandle shadowTex;

static AtrImg       *projImg;
static AtrTexHandle projTex;
static AtrMaterial  *projMaterial;

static AtrEnv       *mainEnv;

static PointOfView  *povLight;
static PointOfView  *povMain;
static PointOfView  *povAux;

static AtrCanvas    *canvasGrab;

AtrXform *lightPos;
AtrXform *cameraPos;

static AtmVector3 cameraPosition = {  60.0f, 55.0f, -60.0f },
                  cameraTarget   = {  0.0f,  5.0f,   0.0f }, 
                  up             = {  0.0f,  1.0f,   0.0f };

/*----------------------------------------------------------------------------
  Hodgepodge
  ----------------------------------------------------------------------------*/
AtrMaterial *newSurface(float r, float g, float b, 
                        float re, float ge, float be,
                        FxU32 flags ) {
    AtrMaterial *m = atrMaterialAllocate(1);
    atrMaterialSetup( m, flags );  
    m->diffuse.r = r;
    m->diffuse.g = g;
    m->diffuse.b = b;
    m->emissive.r = re;
    m->emissive.g = ge;
    m->emissive.b = be;
    return m;
}

AtrXform *newPosition( float x, float y, float z ) {
    AtrXform *xf = atrXformAllocate( 1 );
    xf->data[12] = x;
    xf->data[13] = y;
    xf->data[14] = z;
    return xf;
}

PointOfView *newPOV(FxU32 minx, FxU32 miny, FxU32 width, FxU32 height, 
                    float aspect, float fovDegrees, 
                    float r, float g, float b, 
                    AtrXform *pos) {
    PointOfView *p;
    p = atuMemCalloc( sizeof( PointOfView ), 1 );
    p->canvas = atrCanvasAllocate( 1 );
    p->camera = atrCameraAllocate( 1 );

    p->canvas->xMin = minx, p->canvas->xMax = minx + width  - 1;
    p->canvas->yMin = miny, p->canvas->yMax = miny + height - 1;

    p->camera->fovRadians = fovDegrees * ATM_DEGREE;
    p->camera->aspectRatio = aspect;

    p->pos = pos;

    return p;
}

void selectPOV( PointOfView *p ) {
    atrSelectCanvas( p->canvas );
    p->camera->lcsToWCS = *(p->pos);
    atrSelectCamera( p->camera );
    atrClearCanvas( p->clearColor.r, p->clearColor.g, p->clearColor.b, ATR_WBUFFER_CLEAR );
}

LightSource *newLightSource( AtrXform *pos ) {
    LightSource *l;
    l = atuMemCalloc( sizeof( LightSource ), 1 );

    l->projector                 = atrLightAllocate( 1 );
    l->projector->flags          = ATR_LIGHT_PROJECTOR;
    l->projector->project.fovInX = 90.0f * ATM_DEGREE;
    l->projector->project.fovInY = 90.0f * ATM_DEGREE;
    l->projector->project.tag    = 0;

    l->light            = atrLightAllocate( 1 );
    l->light->flags     = ATR_LIGHT_POSITIONAL;
    l->light->color.r   = 1.0f;
    l->light->color.g   = 1.0f;
    l->light->color.b   = 1.0f;

    l->pos = pos;

    return l;
}

typedef enum { PROJECTOR = 0x01, LIGHT = 0x02 } LightFlag;
void selectLightSource( LightSource *l, LightFlag mode ) {
    atrRemoveLight( 0 );
    
    if ( mode & PROJECTOR ) {
        l->projector->lcsToWCS = *(l->pos);
        atrAddLight( l->projector, 0 );
    }

    if ( mode & LIGHT ) {
        l->light->lcsToWCS = *(l->pos);
        atrAddLight( l->light, 0 );
    }
}

void updatePosition( AtrXform *target, float dRot, float dHoriz, float dX, float dY, float dZ ) {
    AtmVector3 tmp;
    AtrXform   tmpX;

    atrXformReturnToOrthonormal( target, target );

    if ( dHoriz != 0.0f )  {
        /* rotate */
        atrXformSetRotation( &tmpX, &(target->data[0]), dHoriz );
        atrXformCat( target, &tmpX, target );
    } else {
        /* rotate */
        atrXformSetRotation( &tmpX, &(target->data[4]), dRot );
        atrXformCat( target, &tmpX, target );
    }

    /* translate */
    atmVector3Scale( tmp, &(target->data[0]), dX );
    atmVector3Add( &(target->data[12]), &(target->data[12]), tmp );

    atmVector3Scale( tmp, &(target->data[4]), dY );
    atmVector3Add( &(target->data[12]), &(target->data[12]), tmp );

    atmVector3Scale( tmp, &(target->data[8]), dZ );
    atmVector3Add( &(target->data[12]), &(target->data[12]), tmp );
    return;
}

FxU32 mungeIdIntoColor( FxU8 id ) {
    FxU32 redBits;
    FxU32 greenBits;
    FxU32 blueBits;

    redBits   = ( id & 0xF8 ) << 16;
    greenBits = ( ( id & 0x07 ) << 5 ) <<  8;
    blueBits  = 0;
    return ( redBits | greenBits | blueBits );
}

Object *newObject(char *name, 
                  FxU8 shadowId, 
                  AtrMaterial *surface, 
                  AtrTriSet *geom,
                  AtrXform  *pos) {
    Object *o;
    o = atuMemCalloc( sizeof( Object ), 1 );
    o->name = strdup( name );
    o->geom = geom;
    o->shadowId = shadowId;

    o->surface = surface;
    if ( shadowId ) {
        o->shadow = atrMaterialAllocate( 1 );
        atrMaterialSetup(o->shadow, 
                         ATR_MAT_LIGHTSRC_CONSTANT | 
                         ATR_MAT_LIGHTING_ADD );
        o->shadow->constant = mungeIdIntoColor( shadowId );

        o->surface->successor = atrMaterialAllocate( 1 );
        atrMaterialSetup( o->surface->successor, 
                         ATR_MAT_TEX_PROJECTED | ATR_MAT_FB_ZERO );
        o->surface->successor->projectedTag = 0;
        o->surface->successor->texture[0] = shadowTex;
        o->surface->successor->texMinFilter[0] = 
          ATR_TEXFILTER_POINT_SAMPLED;
        o->surface->successor->texMagFilter[0] = 
          ATR_TEXFILTER_POINT_SAMPLED;
        o->surface->successor->atestFunc = ATR_CMP_NOTEQUAL;
        o->surface->successor->aReference = shadowId;
    } else {
        o->shadow = 0;
    }
        
    o->pos = pos;
    return o;
}

/*---------------------------------------------------------------------------
  Global Actions
  ---------------------------------------------------------------------------*/

void addObject( Object *o ) {
    o->next = head;
    head = o;
}

void renderObjects( void ) {
    Object *object;

    /*---------------------------------------------------
      Select Shadow Canvas/Camera
      ---------------------------------------------------*/
    selectPOV( povLight );

    /*---------------------------------------------------
      Add Shadow Projector
      ---------------------------------------------------*/
    selectLightSource( light, PROJECTOR );

    /*---------------------------------------------------
      Render Shadow Image
      ---------------------------------------------------*/
    object = head;
    while( object ) {
        if ( object->shadow ) {
            atrPushMaterial( object->shadow );
            atrPushXform( object->pos );
            atrRenderTriSet( object->geom );
            atrPopXform( 0 );
            atrPopMaterial( 0 );
        }
        object = object->next;
    }

    /*---------------------------------------------------
      Grab Shadow Image
      ---------------------------------------------------*/
    atrSelectCanvas( canvasGrab );
    atrGrabImg( shadowImg, 0, 0, ATR_BUFFER_BACKBUFFER );

    /*---------------------------------------------------
      Associate the new shadow image with the current 
      Texture
      ---------------------------------------------------*/
    atrTexAssociate( shadowTex, shadowImg );
    atrTexAssociate( projTex, projImg );

    /*---------------------------------------------------
      Select Main Canvas/Camera
      ---------------------------------------------------*/
    selectPOV( povMain );

    /*---------------------------------------------------
      Add Main Light
      ---------------------------------------------------*/
    selectLightSource( light, PROJECTOR | LIGHT );

    /*---------------------------------------------------
      Render Main Image
      ---------------------------------------------------*/
    object = head;
    while( object ) {
        if ( gShowProj && object->shadowId )
          atrPushMaterial( projMaterial );
        else
          atrPushMaterial( object->surface );
        atrPushXform( object->pos );
        atrRenderTriSet( object->geom );
        atrPopXform( 0 );
        atrPopMaterial( 0 );
        object = object->next;
    }

    /*---------------------------------------------------
      Select Aux Canvas/Camera
      ---------------------------------------------------*/
    selectPOV( povAux );

    /*---------------------------------------------------
      Add Aux Light
      ---------------------------------------------------*/
    selectLightSource( light, PROJECTOR );

    /*---------------------------------------------------
      Render Aux Image
      ---------------------------------------------------*/
    object = head;
    while( object ) {
        if ( gShowProj && object->shadowId )
          atrPushMaterial( projMaterial );
        else
          atrPushMaterial( object->surface );
        atrPushXform( object->pos );
        atrRenderTriSet( object->geom );
        atrPopXform( 0 );
        atrPopMaterial( 0 );
        object = object->next;
    }

    atrRemoveLight( 5 );
    
    return;
}

void preFrame( void ) {
    FxU32 displayWidth, displayHeight;

    atiHandleEvents();
    if ( !gButton ) {
        if ( !gTrigger ) 
          updatePosition( lightPos, gJoyX*0.04f, 0.0f,  0.0f, 0.0f, gJoyY*0.5f );
        else 
          updatePosition( lightPos, 0.0f, 0.0f, gJoyX*0.5f, 0.0f, gJoyY*0.5f );
    } else {
        updatePosition( lightPos, 0.0f, gJoyX*0.4f, 0.0f, gJoyY*0.5f, 0.0f );
    }
}

void postFrame( void ) {
}

/*-------------------------------------------------------------------------------
  Initialzation Routines
  -------------------------------------------------------------------------------*/

void kbFunc(AtiKeyEvent *ev, FxBool preRecorded) {
    gKeyMap[ev->code] = ev->state;

    if ( gKeyMap[ATI_KEY_LCTRL] || gKeyMap[ATI_KEY_RCTRL] ) {
        if ( ev->state == ATI_KEY_PRESS ) {
            switch( ev->code ) {
                default:
                break;
            }
        }
    }

    if ( ev->state == ATI_KEY_PRESS ) {
        switch( ev->code ) {
          case ATI_KEY_ESCAPE:
            gDone = FXTRUE;
            break;
          case ATI_KEY_P:
            gShowProj = !gShowProj;
            break;
          default:
            break;
        }
    }

    if ( ev->state == ATI_KEY_RELEASE ) {
        switch( ev->code ) {
          default:
            break;
        }
    }
}

void winCloseFunc(void) {
    gDone = FXTRUE;
}


void joyFunc(AtiJoystickEvent *ev, FxBool preRecorded) {

    gJoyX = (fabs(ev->x)>0.2f)?ev->x:0.0f;
    gJoyY = (fabs(ev->y)>0.2f)?ev->y:0.0f;

    gTrigger = ev->button[0];
    gButton  = ev->button[1];
}

void initEvents( void ) {
    atiWinCloseFunc(winCloseFunc);
}

void appInit( void ) {
    atdSetName( APP_NAME );
    atdEventInit();
    atdKeyboardFunc(kbFunc);
    atdJoystickFunc(joyFunc);
    initEvents();
    grDitherMode( GR_DITHER_DISABLE );
    atuSetLoadPath( getenv( "AT_LOAD_PATH" ) );

    /*--------------------------------------------------
      Initialize Env
      --------------------------------------------------*/
    mainEnv   = atrEnvAllocate(1);
    mainEnv->flags = ATR_HSR_WBUFFER;
    atrPushEnv( mainEnv );


    /*--------------------------------------------------
      Create Initial Positions
      --------------------------------------------------*/
    lightPos = newPosition( 0.0f, 5.0f, 0.0f );

    cameraPos = atrXformAllocate( 1 );
    atrXformPointAt( cameraPos, cameraTarget, cameraPosition, up );
    
    light = newLightSource( lightPos );

    /*--------------------------------------------------
      Create Points of View
      --------------------------------------------------*/
    povMain = newPOV( 0, 0, 480, 480,
                      1.0f, 60.0f, 
                      0.0f, 0.0f, 0.3f,
                      cameraPos );
    povLight = newPOV(481, 352, 126, 126,
                      1.0f, 90.0f, 
                      0.0f, 0.0f, 0.1f,
                      lightPos );
    povAux = newPOV(480, 0, 128, 128,
                    1.0f, 60.0f, 
                    0.0f, 0.0f, 0.3f,
                    lightPos );

    /*--------------------------------------------------
      Create Textures, Special Materials
      --------------------------------------------------*/
    shadowImg = atrImgAllocate( 1 );
    shadowImg->format  = ATR_IMGFMT_AI_88;
    shadowImg->width   = 128;
    shadowImg->height  = 128;
    shadowImg->nLevels = 1;
    shadowImg->name    = "shadow";
    shadowImg->table   = 0;
    shadowImg->data    = atuMemCalloc(sizeof( FxU16 ) * 128 * 128, 
                                      1 );
    shadowTex = atrTexNewHandle( ATR_TEXELFX_0 );

    projTex = atrTexNewHandle( ATR_TEXELFX_0 );
    projImg = atrImgAllocate( 1 );
    projMaterial = atrMaterialAllocate( 1 );
    *projImg = *shadowImg;
    projImg->format = ATR_IMGFMT_RGB_565;
    atrTexAssociate( projTex, projImg );
    atrMaterialSetup( projMaterial, ATR_MAT_TEX_PROJECTED );
    projMaterial->texture[0] = projTex;
    projMaterial->projectedTag = 0;

    canvasGrab = atrCanvasAllocate( 1 );
    canvasGrab->xMin = 480, canvasGrab->yMin = 351;
    canvasGrab->xMax = 480+127, canvasGrab->yMax = 351+127;

    head = 0;

    return;
}

void appShutdown( void ) {
}


/*--------------------------------------------------------------------------------
  Main Event Loop
  --------------------------------------------------------------------------------*/
int 
AppMain( int argc, char **argv ) {
    appInit();

    addObject( newObject( "plane",
                          200,
                          newSurface(0.3f, 0.3f, 0.3f,
                                     0.0f, 0.0f, 0.0f,
                                     ATR_MAT_GSHADE ),
                          atrPrimPlane( 100.0f ),
                          newPosition( 0.0f, 0.0f, 0.0f ) ) );
    addObject( newObject( "cube0",
                          11,
                          newSurface(0.5f, 0.0f, 0.5f,
                                     0.0f, 0.0f, 0.0f,
                                     ATR_MAT_GSHADE ),
                          atrPrimCube( 10.0f ),
                          newPosition( 0.0f, 5.0f, 15.0f ) ) );
    addObject( newObject( "cube1",
                          42,
                          newSurface(0.3f, 0.3f, 0.0f, 
                                     0.0f, 0.0f, 0.0f,
                                     ATR_MAT_GSHADE ),
                          atrPrimCube( 10.0f ),
                          newPosition( 0.0f, 5.0f, -15.0f ) ) );
    addObject( newObject( "sphere0",
                          83,
                          newSurface(0.5f, 0.0f, 0.0f, 
                                     0.0f, 0.0f, 0.0f,
                                     ATR_MAT_GSHADE ),
                          atrPrimSphere( 5.0f, 10, 10 ),
                          newPosition( -15.0f, 5.0f, 0.0f ) ) );
    addObject( newObject( "sphere1",
                          94,
                          newSurface(0.0f, 0.5f, 0.0f, 
                                     0.0f, 0.0f, 0.0f,
                                     ATR_MAT_GSHADE ),
                          atrPrimSphere( 5.0f, 10, 10 ),
                          newPosition( 15.0f, 5.0f, 0.0f ) ) );
    addObject( newObject( "light",
                          0,
                          newSurface(1.0f, 1.0f, 1.0f,
                                     1.0f, 1.0f, 1.0f,
                                     ATR_MAT_GSHADE ),
                          atrPrimArrow( 4.0f ),
                          lightPos ) );

    atdEventLoop();

    return 0;
}

void
AppRenderScene( FxU32 displayWidth, FxU32 displayHeight) {
    preFrame();
    renderObjects();
    postFrame();
}
