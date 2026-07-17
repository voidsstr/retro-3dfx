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
** $Date: 10/11/00 7:33:27 PM$ 
**
*/

#include <atutil.h>
#include <atrender.h>

#include <glide.h>

#include <atinput.h>
#include <atscene.h>
#include <atdemop.h>
#include <fxos.h>
#include "banner.inc"

#ifdef BENCHMARK
AtBenchmarkInfo _atBenchmarkInfo;
FILE *benchmark_fp;
time_t temp_time;
#endif

/*-------------------------------------------------------------------
  Function: atdKeyboardFunc();
  Date: 10/23/96
  Implementor(s): mlwp
  Library: AT Input
  Description:
    Specify callback for keyboard events
  Arguments:
    kbFunc - keyboard handler
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atdKeyboardFunc(AtiKeyboardCB kbFunc) {
   _atGlobals.kbFunc = kbFunc;
}

/*-------------------------------------------------------------------
  Function: atdJoystickFunc();
  Date: 10/23/96
  Implementor(s): mlwp
  Library: AT Input
  Description:
    Specify callback for joystick events
  Arguments:
    joyFunc - joystick handler
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atdJoystickFunc(AtiJoystickCB joyFunc) {
   _atGlobals.joyFunc = joyFunc;
}

/*-------------------------------------------------------------------
  Function: atdResetFunc();
  Date: 10/23/96
  Implementor(s): mlwp
  Library: AT Input
  Description:
    Specify callback to reset application state
  Arguments:
    resetFunc - joystick handler
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atdResetFunc(AtdResetCB resetFunc) {
   _atGlobals.resetFunc = resetFunc;
}

static void 
mungePlug( void ) {
    AtrImg *i = _atGlobals.plugImg;
    FxU16 *p, *pb, *data, tmp;
    FxU32 row, col;

    data = i->data;
    for ( row = 0; row < i->height>>1; row++ ) {
        for ( col = 0; col < i->width; col++ ) {
            p = data+row*i->width+col;
            pb = data+(i->height-row-1)*i->width+col;
            tmp = *p;
            *p = *pb;
            *pb = tmp;

            *p = (((*p>>11)&0x1f) | ( *p & 0x7e0) | ((*p&0x1f)<<11));
            *pb = (((*pb>>11)&0x1f) | ( *pb & 0x7e0) | ((*pb&0x1f)<<11));
        }
    }
}

void
atdShamelessPlug( FxBool on ) {
    _atGlobals.plug = on;
}

FxBool
atdDrawPlug( FxU32 displayWidth, FxU32 displayHeight ) {
    AtrEnv env;
    static AtrImg plugImg;
    FxU32 x = 0, y = 0;

    if ( plugImg.data == NULL ) {
        plugImg.format = ATR_IMGFMT_RGB_565;
        plugImg.width = banner_width;
        plugImg.height = banner_height;
        plugImg.data = banner_data;
        _atGlobals.plugImg = &plugImg;
        mungePlug();
    }


    x = displayWidth - _atGlobals.plugImg->width;

    env.flags = 0;
    atrPushEnv(&env);
    atrPushMaterial( _atGlobals.plugMaterial );
    atrRenderImg(_atGlobals.plugImg, x, 0);
    atrPopMaterial(FXTRUE);
    atrPopEnv(FXTRUE);

    return FXTRUE;
}

void 
atdResetKeyboard(void) {
    int i;

    for ( i = 0; i < 256; i++ )
       _atGlobals.keyMap[i] = 0;
}

void
atdResetState(void) {

    /* reset keyboard state so playback has key state correct */

    atdResetKeyboard();

    _atGlobals.curFrame = 0 ;

    if ( _atGlobals.eventMode != ATI_EM_PLAYBACK )
        _atGlobals.numFrames = 0;

   if ( _atGlobals.resetFunc )
       _atGlobals.resetFunc();
}

void 
atdJoyFunc(AtiJoystickEvent *ev, FxBool preRecorded) {

    if (( _atGlobals.eventMode == ATI_EM_PLAYBACK ) && !preRecorded )
        return;

    _atGlobals.joyx = ev->x;
    _atGlobals.joyy = ev->y;
    _atGlobals.joyBut[0] = ev->button[0];
    _atGlobals.joyBut[1] = ev->button[1];
    _atGlobals.joyBut[2] = ev->button[2];
    _atGlobals.joyBut[3] = ev->button[3];

        if ( _atGlobals.joyFunc )
                _atGlobals.joyFunc(ev, preRecorded);
}

FxBool 
atdInitJoystick( void ) {
   if ( _atGlobals.haveJoystick=atiQueryDevice(ATI_DEV_JOYSTICK)) {
       atiJoystickFunc(atdJoyFunc);
   }
   
   return FXTRUE;
}

void
atdClearScreen( void ) {
    atrSelectCanvas( _atGlobals.canvas );
    atrClearCanvas( 0.0f, 0.0f, 0.0f, ATR_WBUFFER_CLEAR );
    /* swap it */
    atrSwapBuffer( 0 );
    /* clear the new buffer at full size again */
    atrClearCanvas( 0.0f, 0.0f, 0.0f, ATR_WBUFFER_CLEAR );
}

void
togglePerformance(FxBool full) {
    /* toggle performance readout */
    if ( _atGlobals.showPerformance != ATD_PERF_NONE )
      _atGlobals.showPerformance = ATD_PERF_NONE ;
    else {
      _atGlobals.showPerformance = full ? ATD_PERF_ALL : ATD_PERF_FPS;
    }

    if ( !_atGlobals.fullView ) 
        atdClearScreen();
}

void
toggleRecord(void) {
    /* toggle record mode */
    if ( _atGlobals.eventMode != ATI_EM_RECORD ) {
        if ( atiEventMode(ATI_EM_RECORD) ) {
            _atGlobals.eventMode = ATI_EM_RECORD;
            atdResetState(); /* ensure we are in a known state */
        }
      } else {
        atiEventMode(_atGlobals.eventMode = ATI_EM_DEFAULT);
        atdResetKeyboard();
      }
}

void
togglePlayback(void) {
    if ( _atGlobals.eventMode == ATI_EM_DEFAULT ) {
        if (atiEventMode(ATI_EM_PLAYBACK)) {
            _atGlobals.eventMode = ATI_EM_PLAYBACK;
            _atGlobals.numFrames = atiGetNumFrames();
            atdResetState();
        }
    } else {
        atiEventMode(_atGlobals.eventMode = ATI_EM_DEFAULT);
        atdResetKeyboard();
    }
}

void 
atdKbdFunc(AtiKeyEvent *ev, FxBool preRecorded) {

    _atGlobals.keyMap[ev->code] = ev->state;

    if ( ( ev->code == ATI_KEY_SCROLLLOCK ) && ( ev->state == ATI_KEY_PRESS )) {
        _atGlobals.showEvents = !_atGlobals.showEvents;
        return;
    }

    /* if we are playing back a script and we get a real event
     * stop playback
     */

    if ( !preRecorded && ev->state && 
        (_atGlobals.eventMode == ATI_EM_PLAYBACK )) {
        atiEventMode(_atGlobals.eventMode = ATI_EM_DEFAULT);
        atdResetKeyboard();
        if ( ev->code == ATI_KEY_ENTER )
            return;
    }

    if ( _atGlobals.kbFunc != NULL )
        _atGlobals.kbFunc(ev, preRecorded);

    /*
     * Deal with everything that is event based. . . . we don't care about
     * key up events, so return.
     */

    if ( ev->state == ATI_KEY_RELEASE )
        return;

    /*
     * Deal with all events that do not rely on the control key being 
     * depressed.
     */
    if( !( _atGlobals.keyMap[ATI_KEY_LCTRL] || _atGlobals.keyMap[ATI_KEY_RCTRL] ) ) {
        switch( ev->code ) {
          case ATI_KEY_ESCAPE:
            if( _atGlobals.help ) {
                _atGlobals.help = FXFALSE;
              } else {
                _atGlobals.done=FXTRUE;
              }
            break;
            /* turn off keyboard input for benchmarking */
#ifndef BENCHMARK
          case ATI_KEY_F1:
            _atGlobals.help = !_atGlobals.help;
            if ( !_atGlobals.fullView ) 
                atdClearScreen();
            break;
          case ATI_KEY_BREAK:
            _atGlobals.print = !_atGlobals.print;
            break;
          case ATI_KEY_S:
            if( _atGlobals.keyMap[ATI_KEY_LSHIFT] || 
                _atGlobals.keyMap[ATI_KEY_RSHIFT] )
            _atGlobals.sync = !_atGlobals.sync;
            break;
          case ATI_KEY_ENTER:
            if ( !preRecorded )
                togglePlayback();
            break;
#endif
        }
      }

    /*
     * Deal with all events that rely on a control key being depressed.
     */

    /* turn off keyboard input for benchmarking */
#ifndef BENCHMARK
    if( _atGlobals.keyMap[ATI_KEY_LCTRL] || _atGlobals.keyMap[ATI_KEY_RCTRL] ) {
        switch( ev->code ) {
          case ATI_KEY_D:
            /* state reset */
            atiEventMode(_atGlobals.eventMode = ATI_EM_DEFAULT);
            atdResetState();
            break;
          case ATI_KEY_J:
            /* calibrate joystick */
            atdJoystickCalibrate();
            break;
          case ATI_KEY_P:
              togglePerformance( _atGlobals.keyMap[ATI_KEY_LSHIFT] || 
                                 _atGlobals.keyMap[ATI_KEY_RSHIFT] );
              break;
          case ATI_KEY_R:
              toggleRecord();
              break;
          case ATI_KEY_W:
            /* toggle wireframe/solid */
            _atGlobals.wireframe = !_atGlobals.wireframe;
            if ( _atGlobals.wireframe )
              atsRenderMode(ATS_RM_WIREFRAME);
            else 
              atsRenderMode(ATS_RM_SOLID);
            break;
          case ATI_KEY_F:
            /* toggle fog */
            _atGlobals.fog = !_atGlobals.fog;
            break;
          case ATI_KEY_B:
            /* toggle bilinear filtering. */
            _atGlobals.bilinear = !_atGlobals.bilinear;
            break;
          case ATI_KEY_M:
            /* toggle mipmapping. */
            _atGlobals.mipMap = !_atGlobals.mipMap;
            break;
          case ATI_KEY_T:
            /* toggle texturing. */
            _atGlobals.texture = !_atGlobals.texture;
            break;
          case ATI_KEY_A:
            /* toggle sound */
            if ( _atGlobals.soundAvailable )
                _atGlobals.sound = !_atGlobals.sound;
            break;
          case ATI_KEY_S:
            /* splash  */
            _atGlobals.plug = !_atGlobals.plug;
            if ( !_atGlobals.fullView ) 
                atdClearScreen();
            break;
          }
    }
#endif
}

FxBool 
atdInitKeyboard( void ) {
   if ( _atGlobals.haveKeyboard=atiQueryDevice(ATI_DEV_KEYBOARD)) {
       atiKeyboardFunc(atdKbdFunc);
       atdResetKeyboard();
   }
   
   return FXTRUE;
}

void 
atdFocusFunc(FxBool gained) {
    atrPause(!gained);
    if (gained)
        atdResetKeyboard();
}

void 
atdWinCloseFunc(void) {
    _atGlobals.done = FXTRUE;
}

void 
atdEventInit( void ) {
    _atGlobals.done = FXFALSE;
    _atGlobals.showPerf = FXFALSE;
    _atGlobals.curFrame = 0 ;
   
    atiEventHistoryFile( _atGlobals.historyFileName );
    atiEventMode(_atGlobals.eventMode);

    if ( _atGlobals.eventMode == ATI_EM_PLAYBACK ) {
        _atGlobals.numFrames = atiGetNumFrames();
    }

    atdInitKeyboard();
    atdInitJoystick();
    atiFocusLostFunc(atdFocusFunc);
    atiFocusGainFunc(atdFocusFunc);
    atiWinCloseFunc(atdWinCloseFunc);

    /* setup material callbacks */

    atsMaterialFuncs( (void *)atdPushMaterial, (void *)atdPopMaterial);

    /* allocate materials for banner and help screen */

    _atGlobals.helpMaterial = atrMaterialAllocate(1);
    _atGlobals.plugMaterial = atrMaterialAllocate(1);

    if (( _atGlobals.helpMaterial == NULL ) ||
        ( _atGlobals.plugMaterial == NULL )) {
        atuError(FXTRUE, "atdEventInit: could not allocate materials\n");
    }

    /* Initialize help material */

    _atGlobals.helpMaterial->acuFunction = GR_COMBINE_FUNCTION_SCALE_OTHER;
    _atGlobals.helpMaterial->acuFactor = GR_COMBINE_FACTOR_ONE;
    _atGlobals.helpMaterial->acuLocal = GR_COMBINE_LOCAL_CONSTANT;
    _atGlobals.helpMaterial->acuOther = GR_COMBINE_OTHER_TEXTURE;
    _atGlobals.helpMaterial->acuInvert = FXFALSE;

    _atGlobals.helpMaterial->ccuFunction =  GR_COMBINE_FUNCTION_SCALE_OTHER;
    _atGlobals.helpMaterial->ccuFactor = GR_COMBINE_FACTOR_ONE;
    _atGlobals.helpMaterial->ccuLocal = GR_COMBINE_LOCAL_NONE;
    _atGlobals.helpMaterial->ccuOther = GR_COMBINE_OTHER_TEXTURE;
    _atGlobals.helpMaterial->ccuInvert = FXFALSE;

    _atGlobals.helpMaterial->abuSrcFactor = GR_BLEND_SRC_ALPHA;
    _atGlobals.helpMaterial->abuDstFactor = GR_BLEND_ONE_MINUS_SRC_ALPHA;

    _atGlobals.helpMaterial->depthMask  = FXFALSE;

    _atGlobals.helpMaterial->chromaKeyEnable = FXFALSE;
    _atGlobals.helpMaterial->constant = (FxU32)(150<<24);
    atrMaterialModify(_atGlobals.helpMaterial);

    /* Initialize banner material */

    _atGlobals.plugMaterial->acuFunction = GR_COMBINE_FUNCTION_SCALE_OTHER;
    _atGlobals.plugMaterial->acuFactor = GR_COMBINE_FACTOR_ONE;
    _atGlobals.plugMaterial->acuLocal = GR_COMBINE_LOCAL_CONSTANT;
    _atGlobals.plugMaterial->acuOther = GR_COMBINE_OTHER_TEXTURE;
    _atGlobals.plugMaterial->acuInvert = FXFALSE;

    _atGlobals.plugMaterial->ccuFunction =  GR_COMBINE_FUNCTION_SCALE_OTHER;
    _atGlobals.plugMaterial->ccuFactor = GR_COMBINE_FACTOR_ONE;
    _atGlobals.plugMaterial->ccuLocal = GR_COMBINE_LOCAL_NONE;
    _atGlobals.plugMaterial->ccuOther = GR_COMBINE_OTHER_TEXTURE;
    _atGlobals.plugMaterial->ccuInvert = FXFALSE;

    _atGlobals.plugMaterial->abuSrcFactor = GR_BLEND_SRC_ALPHA;
    _atGlobals.plugMaterial->abuDstFactor = GR_BLEND_ONE_MINUS_SRC_ALPHA;

    _atGlobals.plugMaterial->depthMask  = FXFALSE;

    _atGlobals.plugMaterial->chromaKeyEnable = FXTRUE;
    _atGlobals.plugMaterial->chromaKeyValue = 0;
    _atGlobals.plugMaterial->constant = (FxU32)(150<<24);
    atrMaterialModify(_atGlobals.plugMaterial);
}

void 
atdHandleEvents( void ) {
   atiHandleEvents();
   _atGlobals.curFrame++ ;

   if ( _atGlobals.eventMode != ATI_EM_PLAYBACK )
        _atGlobals.numFrames++;

   if ((_atGlobals.eventMode == ATI_EM_PLAYBACK) &&
       (_atGlobals.curFrame>=_atGlobals.numFrames)) {
       if ( _atGlobals.repeatCount > 0 ) {
           _atGlobals.repeatCount--;
           if ( _atGlobals.repeatCount == 0 )
               _atGlobals.done=FXTRUE;
       }
       atdResetState();
       atiReplayEvents();
   }
}

void 
atdEventClose( void ) {
    atiShutdown();
}

void 
atdPushMaterial( AtrMaterial *m ) {
    AtrMaterial tmp;
        AtrTexMMMode mmMode;
    AtrTexFilter minFilter, magFilter;

        mmMode = m->texMMMode[0];
        minFilter = m->texMinFilter[0];
        magFilter = m->texMagFilter[0];

    if ( !_atGlobals.texture ) {
        tmp = *m;
        m = &tmp;
        atrMaterialSetup( &tmp, ATR_MAT_GSHADE);
    }

        m->texMMMode[0] = _atGlobals.mipMap ? ATR_TEXMIPMAP_NEAREST : 
                                          ATR_TEXMIPMAP_DISABLE;
        m->texMinFilter[0] = _atGlobals.bilinear ? ATR_TEXFILTER_BILINEAR : 
                                               ATR_TEXFILTER_POINT_SAMPLED;
        m->texMagFilter[0] = _atGlobals.bilinear ? ATR_TEXFILTER_BILINEAR : 
                                               ATR_TEXFILTER_POINT_SAMPLED;

    atrPushMaterial(m);

        m->texMMMode[0] = mmMode;
        m->texMinFilter[0] = minFilter;
        m->texMagFilter[0] = magFilter;
}

void atdPopMaterial( FxBool update ) {
    atrPopMaterial(update);
}

/*-------------------------------------------------------------------
  Function: atdDrawHelp
  Date: 10/22/96
  Implementor(s): mlwp
  Library: AT Demo
  Description:
    Draw the help screen. Preload it if necessary
  Arguments:
    None
  Return:
    FXTRUE on success, FXFALSE on error
  -------------------------------------------------------------------*/

static void 
mungeHelp(void) {
    AtrImg *i = _atGlobals.helpImage;
    FxU32 count = i->width*i->height; 
    FxU16 *p = i->data, pix;
    FxU16 r, g, b;

    if ( i->format != ATR_IMGFMT_ARGB_1555 )
        return;

    while ( count-- > 0 ) {
        pix = *p;
        b = pix & 0x1f; pix >>= 5;
        g = pix & 0x1f; pix >>= 5;
        r = pix & 0x1f; pix >>= 5;

        *p = ( r << 11 ) | ( g << 6 ) | b;
        p++;
    }

    i->format = ATR_IMGFMT_RGB_565;
}

void atdSetHelpFile(char *name) {
    _atGlobals.helpFile = strdup(name);
}

FxBool
atdDrawHelp(void) {
    FxU32 x = 0, y = 0;
    AtrEnv env;

    if (( _atGlobals.helpImage == NULL ) && 
        ( _atGlobals.helpFile != NULL )) {
        _atGlobals.helpImage = atrImgAllocate( 1 );
        if ( !atrImgCreateFromFile( _atGlobals.helpImage, 
                                    _atGlobals.helpFile)) {
            free(_atGlobals.helpFile);
            _atGlobals.helpFile = NULL;
            return FXFALSE;
        }

    }

    mungeHelp();

    if ( _atGlobals.helpImage->width < _atGlobals.caps.width )
        x = (_atGlobals.caps.width - _atGlobals.helpImage->width )>>1;

    if ( _atGlobals.helpImage->height < _atGlobals.caps.height )
        y = (_atGlobals.caps.height - _atGlobals.helpImage->height )>>1;

    env.flags = 0;
    atrPushEnv(&env);
    atrPushMaterial(_atGlobals.helpMaterial);
    atrRenderImg(_atGlobals.helpImage, x, y);
    atrPopMaterial(FXTRUE);
    atrPopEnv(FXTRUE);

    return FXTRUE;
}


FxBool
atdDrawHelpBuffer(FxU32 buffer) {
    FxU32 x = 0, y = 0;
    AtrEnv env;

    if (( _atGlobals.helpImage == NULL ) && 
        ( _atGlobals.helpFile != NULL )) {
        _atGlobals.helpImage = atrImgAllocate( 1 );
        if ( !atrImgCreateFromFile( _atGlobals.helpImage, 
                                    _atGlobals.helpFile)) {
            free(_atGlobals.helpFile);
            _atGlobals.helpFile = NULL;
            return FXFALSE;
        }

    }

    mungeHelp();

    if ( _atGlobals.helpImage->width < _atGlobals.caps.width )
        x = (_atGlobals.caps.width - _atGlobals.helpImage->width )>>1;

    if ( _atGlobals.helpImage->height < _atGlobals.caps.height )
        y = (_atGlobals.caps.height - _atGlobals.helpImage->height )>>1;

    env.flags = 0;
    atrPushEnv(&env);
    atrPushMaterial(_atGlobals.helpMaterial);
    atrRenderImgBuffer(_atGlobals.helpImage, x, y, buffer);
    atrPopMaterial(FXTRUE);
    atrPopEnv(FXTRUE);

    return FXTRUE;
}



void AppRenderScene( FxU32 displayWidth, FxU32 displayHeight);

void
atdRenderScene(void) {
    FxU32 displayWidth, displayHeight;
    FxFloat size;
    char tmpBuf[80];

    if (!_atGlobals.bAppActive)
        return;

    if ( _atGlobals.canvas == NULL ) {
        if ((_atGlobals.canvas = atrCanvasAllocate( 1 )) == NULL ) {
            atuError(FXTRUE, " atdRenderScene: could not create canvas\n");
        }
    }

    /* configure the canvas */

    atrBeginScene(&displayWidth, &displayHeight);

#ifdef BENCHMARK
    /* start the timer for the benchmark */
    if( _atBenchmarkInfo.playbackOn )
      {
        _atBenchmarkInfo.startTime = clock();
      }
#endif

    size = displayHeight*.1f;
    if ( size > 15.f )
        size = 15.f;

    atrDitherMode(_atGlobals.dithering ? ATR_DITHER_ENABLE : ATR_DITHER_DISABLE );
                
        _atGlobals.canvas->xMin = 0;
    _atGlobals.canvas->yMin = 0;
        _atGlobals.canvas->xMax = displayWidth;
        _atGlobals.canvas->yMax = displayHeight;

    atrSelectCanvas( _atGlobals.canvas );

    AppRenderScene( displayWidth, displayHeight);


    /*--------------------------------------------------------
      draw annotations
      --------------------------------------------------------*/

    atrSelectCanvas( _atGlobals.canvas );

    if( _atGlobals.help && ( _atGlobals.helpImage != NULL ) ) {
#ifdef __WIN32__
        if ( !_atGlobals.fullScreen )
            atdWinHelp();
                else 
#endif
        atdDrawHelp();
    } else {
#ifndef BENCHMARK
      /* turn off the plug if a benchmark */
        if ( _atGlobals.plug && _atGlobals.fullScreen )
            atdDrawPlug(displayWidth, displayHeight);
#endif

        if ( _atGlobals.showPerformance != ATD_PERF_NONE )
            _atrDrawString( _atGlobals.perfStats, size, size, 0.5f, size+.5f,
                            !_atGlobals.fullView );
        if ( _atGlobals.showPerformance == ATD_PERF_ALL )
           _atrDrawString( _atGlobals.perfStats1, size, size, 0.5f, 0.5f ,
                            !_atGlobals.fullView);

        switch ( _atGlobals.eventMode ) {
        case ATI_EM_DEFAULT:
            break;
        case ATI_EM_PLAYBACK:
            if ( _atGlobals.showEvents ) {
                sprintf(tmpBuf, "PB: NF %d CF %d", 
                        _atGlobals.numFrames, _atGlobals.curFrame);
                    _atrDrawString( tmpBuf, size, size, 
                            0.5f, displayHeight-size-0.5,
                            !_atGlobals.fullView );
            }
            break;
        case ATI_EM_RECORD:
            sprintf(tmpBuf, "RM: CF %d", _atGlobals.curFrame);
            _atrDrawString( tmpBuf, size, size, 
                            0.5f, displayHeight-size-0.5f,
                            !_atGlobals.fullView );
            break;
        }
    }

#ifdef BENCHMARK
    /* stop the timer for the benchmark */
    if( _atBenchmarkInfo.playbackOn )
      {
        _atBenchmarkInfo.endTime = clock();
      }
#endif

    atrEndScene();

#ifdef BENCHMARK
    /* collect the cumulative time and increment the number of frames rendered */
    if( _atBenchmarkInfo.playbackOn && !_atBenchmarkInfo.playbackDone )
      {
        _atBenchmarkInfo.frameTime = _atBenchmarkInfo.endTime - _atBenchmarkInfo.startTime;
        _atBenchmarkInfo.totalTime += ( double )_atBenchmarkInfo.frameTime;
        _atBenchmarkInfo.numFrames++;
      }

    /* collect the cummulative info for multiple bencmarks and dump statistics */
    if( _atBenchmarkInfo.playbackOn && _atBenchmarkInfo.playbackDone )
      {
        /* calculate the benchmark values */
        _atBenchmarkInfo.totalTimeSec = _atBenchmarkInfo.totalTime / ( double )CLOCKS_PER_SEC;
        _atBenchmarkInfo.fps = ( double )_atBenchmarkInfo.numFrames / _atBenchmarkInfo.totalTimeSec;
        _atBenchmarkInfo.scaledFps = _atBenchmarkInfo.fps * BENCHMARK_FPS_SCALE1;

        /* check to see if we need to run benchmark again or quit and accumulate accordingly */
        if( --_atBenchmarkInfo.currentBenchmarks == 0 )
          {
            _atGlobals.done = FXTRUE;
          }
        else
          {
            _atBenchmarkInfo.playbackDone = FXFALSE;
            _atBenchmarkInfo.averagedBenchmarks += _atBenchmarkInfo.fps;
          }

        /* dump the benchmark statistics to a file and close it */
        if( _atBenchmarkInfo.dumpStats )
          {
            /* get the time stamp */
            _atBenchmarkInfo.timeStamp = time( &temp_time );
            _atBenchmarkInfo.timeStampString = asctime( localtime( &_atBenchmarkInfo.timeStamp ) );

            /* dump the demo statistics to a file */
            benchmark_fp = fopen( _atBenchmarkInfo.filename, "a" );
            if( !benchmark_fp )
              atuError( FXTRUE, "Could not open the benchmark stats file.\n" );

            fprintf( benchmark_fp, "Benchmark Statistics %s", _atBenchmarkInfo.timeStampString );
            fprintf( benchmark_fp, "   Number of Frames = %d\n", _atBenchmarkInfo.numFrames );
            fprintf( benchmark_fp, "   Total TIme = %4.4lf\n", _atBenchmarkInfo.totalTimeSec );
            fprintf( benchmark_fp, "   Frames per Second = %4.2lf\n", _atBenchmarkInfo.fps );
            fprintf( benchmark_fp, "   Benchmark Value = %4.2lf\n", _atBenchmarkInfo.scaledFps );         

            fclose( benchmark_fp );
          }
        
        /* if running benchmark again, reset accumulators */
        if( !_atBenchmarkInfo.playbackDone )
          {
            _atBenchmarkInfo.totalTime = ( double )0.0;
            _atBenchmarkInfo.numFrames = 0;
          }
        else
          /* if not running the benchmark again, print final results */
          {
            _atBenchmarkInfo.averagedBenchmarks += _atBenchmarkInfo.fps;
            _atBenchmarkInfo.averagedBenchmarks = _atBenchmarkInfo.averagedBenchmarks / ( double )_atBenchmarkInfo.numBenchmarks;
            
            if( _atBenchmarkInfo.dumpStats )
              {
                benchmark_fp = fopen( _atBenchmarkInfo.filename, "a" );
                if( !benchmark_fp )
                  atuError( FXTRUE, "Could not open the benchmark stats file.\n" );
                
                fprintf( benchmark_fp, "\n\nAverage FPS of all iterations = %4.2lf\n", _atBenchmarkInfo.averagedBenchmarks );

                fclose( benchmark_fp );
              }
          }
      }
#endif

    if ( _atGlobals.print ) {
        char buff[20];

        sprintf(buff,"frm%04d.ppm", _atGlobals.curFrame) ;
        if (!atdDumpFrameBuffer(buff)) {
            atuError(FXTRUE, "could not dump frame buffer\n");
        }
    }

    atrSwapBuffer( _atGlobals.sync );

    _atGlobals.numSamples++;
    atrStatsRetrieve( &_atGlobals.stats );
    _atGlobals.totalTris += _atGlobals.stats.totalTris;
    _atGlobals.pixelsOut += _atGlobals.stats.pixelsOut;
    _atGlobals.peakTexCacheMisses[0] += 
                                 _atGlobals.stats.peakTexCacheMissesInFrame[0];
    _atGlobals.peakTexCacheMisses[1] += 
                                 _atGlobals.stats.peakTexCacheMissesInFrame[1];
    atrStatsReset();
    if( _atGlobals.numSamples == ATD_SAMPLE_FREQ ) {
        _atGlobals.fps = ( float )( _atGlobals.numSamples / timer(1) );
        _atGlobals.tps = ( float )( _atGlobals.totalTris / (1000.0*timer(1)));
        _atGlobals.mps = ( float )(_atGlobals.pixelsOut / (1000000.0*timer(1)));
        switch ( _atGlobals.showPerformance ) {
        case ATD_PERF_NONE:
            break;
        case ATD_PERF_FPS:
            sprintf( _atGlobals.perfStats, "%4.1f FPS", _atGlobals.fps);
            break;
        case ATD_PERF_ALL:
            sprintf( _atGlobals.perfStats, "%4.1f FPS %4.1f KTPS",
                     _atGlobals.fps, _atGlobals.tps);
            sprintf( _atGlobals.perfStats1, "%4.1f MPS %4.1f %4.1f DPS", 
                     _atGlobals.mps, 
                     ((float)_atGlobals.peakTexCacheMisses[0] ) / timer(1),
                     ((float)_atGlobals.peakTexCacheMisses[1] ) / timer(1));
            break;
        }
        _atGlobals.pixelsOut = 0;
        _atGlobals.totalTris = 0;
        _atGlobals.numSamples = 0;
        _atGlobals.peakTexCacheMisses[0] = 0;
        _atGlobals.peakTexCacheMisses[1] = 0;
        timer( 0 );
    }
}

int
atdEventLoop(void) {
    strcpy(_atGlobals.perfStats, "INIT");
    _atGlobals.perfStats1[0]  = 0;

#ifdef __WIN32__
    ShowWindow(_atGlobals.hWndMain, SW_SHOWNORMAL);
#endif
    atdSetFocus();
    // atdPause(0);

    while (!_atGlobals.done) {
        atdHandleEvents();

		if ( !_atGlobals.bMinimized )
            atdRenderScene();

        #if macintosh
        if (gSoundAttnFlag) {
            soundCleanUp();
        }
        #endif /* macintosh */
    }

    return 1;
}
