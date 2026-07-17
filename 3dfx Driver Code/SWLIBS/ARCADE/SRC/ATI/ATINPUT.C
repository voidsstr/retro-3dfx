/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 7:33:31 PM$ 
**
*/

#include <math.h>
#include <string.h>
#include <atinput.h>
#ifndef AT_INPUT_DIRECTX
#include <joy.h>
#else
#include <windows.h>
#include <mmsystem.h>
#endif

#define MAGIC(_a, _b, _c, _d) (((_a)<<24)|((_b)<<16)|((_c)<<8)|(_d))
#define HISTORY_MAGIC MAGIC('h', 'i', 's', 't')
#define HISTORY_VERSION 01 /* last digit is minor version number */

FxBool _atiInitJoystick( void );

AtiState atiState = { ATI_EM_CLOSED };

static float last_joyx = 0.0f, last_joyy = 0.0f;
#ifdef AT_INPUT_DIRECTX
static int minx, miny, maxx, maxy, centerx, centery;
#endif
#define MAX_BUTTONS 32
static int  last_button[MAX_BUTTONS] ;
static FxBool moved_last = FXFALSE;

static FxBool atiDeliverEvent( AtiEvent *e, FxBool preRecorded) {

    if ( atiState.eventMode == ATI_EM_RECORD ) {
         if ( fwrite( e, sizeof( *e ), 1, atiState.eventFile ) != 1 ) {
             atuError(FXFALSE, "Error writing event file\n");
             return FXFALSE;
        }
    }

    switch ( e->device ) {
    case ATI_DEV_KEYBOARD:
        if ( atiState.kbFunc != NULL )
            atiState.kbFunc(&e->ev.key, preRecorded);
        break;
    case ATI_DEV_GCI:
        if ( atiState.gciFunc != NULL )
            atiState.gciFunc(&e->ev.gci, preRecorded);
        break;
    case ATI_DEV_JOYSTICK:
        if ( atiState.joyFunc != NULL )
            atiState.joyFunc(&e->ev.joy, preRecorded);
        break;
    }

    return FXTRUE;
}

#ifdef __WIN32__
void atiJoystickCalibrate(void) {
}
#elif defined( __DOS32__ ) && !defined( __DJGPP )
void atiJoystickCalibrate(void) {
   if (atiState.haveJoystick) {
      joystick_calibrate();
   }
}
#endif

/*
 * Function initializes joystick on the game_port only
 */

static UINT _joyID = JOYSTICKID1;

void atiSelectJoystickID( UINT joyID )
{
	_joyID = joyID;
}

FxBool 
_atiInitJoystick( void ) {
	int i = 0;
#ifndef AT_INPUT_DIRECTX

#ifdef __DJGPP__
   atiState.haveJoystick = 0;
   return FXFALSE;
#else /* __DJGPP */
   if (!joystick_detect())
      return FXFALSE;
   joystick_calibrate();
#endif /* __DJGPP__ */

#else

   // HERE IS DIRECTX SHIT !!!!!!!
    MMRESULT mmError;
    JOYCAPS  jcCaps;
    JOYINFOEX jiInfo;
	UINT		nb_joysticks;


	nb_joysticks = joyGetNumDevs();

    /* get center */

    jiInfo.dwSize = sizeof(jiInfo);
    jiInfo.dwFlags = JOY_RETURNALL;
    //mmError  = joyGetPosEx(JOYSTICKID1 , &jiInfo);
    mmError  = joyGetPosEx( _joyID , &jiInfo);

    /* is there a joystick attached??? */

    if(mmError!=JOYERR_NOERROR){
       // atuError(FXFALSE, "No Joystick detected");
       return FXFALSE; /* no joystick */
    }

    centerx = jiInfo.dwXpos;
    centery = jiInfo.dwYpos;

    //mmError = joyGetDevCaps(JOYSTICKID1 , &jcCaps , sizeof(jcCaps) );
    mmError = joyGetDevCaps( _joyID, &jcCaps , sizeof(jcCaps) );

    if(mmError == MMSYSERR_NODRIVER){
        return FXFALSE; /* no joystick */
    } else{
        minx = jcCaps.wXmin;
        maxx = jcCaps.wXmax;
        miny = jcCaps.wYmin;
        maxy = jcCaps.wYmax;
    }
#endif

   atiState.haveJoystick = 1;   
   last_joyx = 0.0f, last_joyy = 0.0f;
   for ( i = 0; i < MAX_BUTTONS ; i++)
	   last_button[i] = 0;
   return FXTRUE;
}

static void _AtiGCICallback(AtiGCIEvent *ev) {
    AtiEvent e;
 
    e.device = ATI_DEV_GCI;
    e.ev.gci = *ev;
    atiDeliverEvent( &e, FXFALSE );
}

static FxBool InitGCI( void ) {
   static GciConfig game_params;

   /*
    * Check for gci hardware else use other input
    */

#ifndef __DJGPP__
   if( !gciOpen( &game_params ) ) {
#ifdef ARCADE_ONLY
      atuError( FXTRUE, "InitGCI:  error encountered" );
#endif /* ARCADE_ONLY */
      return FXFALSE;
   }
   
   return gciHasGameFrame();
#else /* __DJGPP__ */
   return FXFALSE;
#endif /* __DJGPP */
}

static FxBool InitKeyboard(void) {
#ifdef __DOS32__
#ifdef __DJGPP__

  /*
   * Do nothing for DJGPP since there is no keyboard handler.
   */
  return FXFALSE;

#else /* __DJGPP */
FxBool InitDos(void);

    return InitDos();
#endif /* __DJGPP */
#endif
#ifdef __WIN32__
FxBool InitWin32(void);

    return InitWin32();
#endif
}

/*-------------------------------------------------------------------
  Function: atiInit();
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description:
    Initializes the input library state and input acquisition hardware
  Arguments:
    msgFunc - function to process system messages
  Return:
    FXTRUE  - success
    FXFALSE - failure
  -------------------------------------------------------------------*/

FxBool atiInit(AtiMessageCB msgFunc) {
#ifdef AT_DEBUGGING
    if ( atiState.eventMode != ATI_EM_CLOSED ) 
        atuError( FXTRUE, "atiInit(): Library already initialized.\n" );
#endif    

    atiState.msgFunc = msgFunc;
    atiState.eventMode = ATI_EM_DEFAULT;
    atiState.currentFrame = 0;
    atiState.numFrames = 0;
    atiState.kbFunc = NULL;
    atiState.joyFunc = NULL;
    atiState.gciFunc = NULL;
    atiState.putEventFunc = NULL;
    atiState.focusLost = NULL;
    atiState.focusGain = NULL;
    atiState.winClose = NULL;
    atiState.eventFile = NULL;
    atiState.eventFileName = NULL;
    atiState.bResizingDisabled = FXTRUE;  /* do not allow resizing */
    atiState.bMinimized = FXFALSE;        /* input window is minimized */
    atiState.firstEvent = atiState.lastEvent = 0;

    atiState.haveKeyboard= InitKeyboard();

    atiState.haveJoystick = _atiInitJoystick();

    if( getenv( "GCI_GAMEFRAME" ) == NULL )
         atiState.haveGCI = FXFALSE;
    else atiState.haveGCI = InitGCI();
   
   return FXTRUE;
}

// Added to allow an app to reinit the joystick at anytime
// this is the only way an app could select another joystick
// then JOYSTICKID1 
void atiInitJoystick( void )
{
    atiState.haveJoystick = _atiInitJoystick();
}

/*-------------------------------------------------------------------
  Function: atiShutdown();
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description:
    Reset the input library state and close input acquisition hardware
  Arguments:
    none
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atiShutdown(void) {
#if defined ( __DOS32__ ) && !defined( __DJGPP__ )
   void CloseKeyboardISR(void);

   CloseKeyboardISR();
#endif
    atiState.eventMode = ATI_EM_CLOSED;
}

/*-------------------------------------------------------------------
  Function: atiQueryDevice();
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description:
    Determine which input devices are available
  Arguments:
    device - which deveice are we intersted in
  Return:
    FXTRUE  if deveice is available
    FXFALSE if deveice is unavailable
  -------------------------------------------------------------------*/

FxBool atiQueryDevice(FxU32 device) {
    switch ( device ) {
    case ATI_DEV_KEYBOARD:
        return atiState.haveKeyboard ;   
    case ATI_DEV_GCI:
        return atiState.haveGCI ;   
    case ATI_DEV_JOYSTICK:
        return atiState.haveJoystick ;   
    default:
        atuError(FXFALSE, "Invalid device\n", device);
        return FXFALSE;
    }
}

/*-------------------------------------------------------------------
  Function: atiEventHistoryFile();
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description:
    Specify file for reading/writing event data
  Arguments:
    fileName - what file to use
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atiEventHistoryFile(char *fileName) {
    if ( atiState.eventFileName != NULL )
        free(atiState.eventFileName);

    atiState.eventFileName = strdup(fileName);
}

static FxBool atiCloseEventFile(void) {
    AtiEventHeader h;

    if ( atiState.eventMode == ATI_EM_RECORD ) {
        h.magic = HISTORY_MAGIC;
        h.versionNumber = HISTORY_VERSION;
        h.numFrames = atiState.currentFrame;
        rewind(atiState.eventFile);
        
        if ( fwrite( &h, sizeof( h ), 1, atiState.eventFile ) != 1 ) {
            atuError(FXFALSE, "Error writing history file %s\n",
                     atiState.eventFileName);
            return FXFALSE;
        }
    }

    fclose(atiState.eventFile);

    return FXTRUE;
}

static FxBool atiOpenEventFile(FxU32 eventMode) {
    AtiEventHeader h;

    switch ( eventMode ) {
    case ATI_EM_RECORD:
        atiState.eventFile = fopen(atiState.eventFileName, "wb");
        if ( atiState.eventFile == NULL ) {
            atuError(FXFALSE, "atiOpenEventFile could not open %s\n",
                     atiState.eventFileName);
            return FXFALSE;
        }
        if ( fwrite( &h, sizeof( h ), 1, atiState.eventFile ) != 1 ) {
            atuError(FXFALSE, "Error writing history file\n");
            return FXFALSE;
        }
        atiState.currentFrame = 0;
        atiState.numFrames = 0;
        break;
    case ATI_EM_PLAYBACK:
        atiState.eventFile = fopen(atiState.eventFileName, "rb");
        if ( atiState.eventFile == NULL ) {
            atuError(FXFALSE, "atiOpenEventFile could not open %s\n",
                     atiState.eventFileName);
            return FXFALSE;
        }
        if ( fread( &h, sizeof( h ), 1, atiState.eventFile ) != 1 ) {
            atuError(FXFALSE, "Error writing history file\n");
            return FXFALSE;
        }
        if  ( h.magic != HISTORY_MAGIC ) {
            atuError(FXFALSE, "Invalid event file %s\n",
                     atiState.eventFileName);
            return FXFALSE;
        }
        if ( h.versionNumber != HISTORY_VERSION ) {
            atuError(FXFALSE, "Old event file %s\n",
                     atiState.eventFileName);
            return FXFALSE;
        }
        atiState.numFrames = h.numFrames ;
        atiState.currentFrame = 0;
        break;
    default:
        atuError(FXFALSE, "atiOpenEventFile: bad mode %d\n", eventMode);
        return FXFALSE;
    }

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atiGetNumFrames();
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description:
    Return the number of recorded frames
  Arguments:
    None
  Return:
    Number of recorded frames
  -------------------------------------------------------------------*/

atiGetNumFrames(void) {
    if ( atiState.eventMode == ATI_EM_PLAYBACK )
        return atiState.numFrames ;
    else return 0;
}

/*-------------------------------------------------------------------
  Function: atiGetCurrentFrame();
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description:
    Return the current frame number
  Arguments:
    None
  Return:
    Current frame number
  -------------------------------------------------------------------*/

atiGetCurrentFrame(void) {
    return atiState.currentFrame;
}

/*-------------------------------------------------------------------
  Function: atiEventMode();
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description:
    Determine new event mode
  Arguments:
    eventMode - ATI_EM_DEFAULT, ATI_EM_RECORD, ATI_EM_PLAYBACK
  Return:
    FXTRUE  on success
    FXFALSE on failure
  -------------------------------------------------------------------*/

FxBool atiEventMode(FxU32 eventMode) {
    if (eventMode == atiState.eventMode)
        return FXTRUE;

    if ( atiState.eventMode != ATI_EM_DEFAULT )
        atiCloseEventFile();

    switch (eventMode) {
    case ATI_EM_DEFAULT:
        break;
    case ATI_EM_RECORD: 
    case ATI_EM_PLAYBACK: 
        if (!atiOpenEventFile(eventMode))
            return FXFALSE;
        break;
    default:
        atuError(FXFALSE, "Invalid event mode %d\n", eventMode);
        return FXFALSE;
    }

    atiState.eventMode = eventMode;
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atiHandleEvents();
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description:
    Process all events for this frame
  Arguments:
    none
  Return:
    Nothing
  -------------------------------------------------------------------*/

void
atiPollDevices(void) {
    FxBool moved_this ;

    if (( atiState.haveJoystick ) && ( atiState.joyFunc != NULL )) {
        AtiEvent j;
        j.device = ATI_DEV_JOYSTICK;
#ifndef AT_INPUT_DIRECTX

#ifndef __DJGPP__
        joystick_read( &j.ev.joy.x, &j.ev.joy.y, &j.ev.joy.button[0], 
                       &j.ev.joy.button[1], &j.ev.joy.button[2], &j.ev.joy.button[3] );
#endif /* __DJGPP__ */

#else
        {
            JOYINFOEX jiInfo;
            int x, y;

            jiInfo.dwSize  = sizeof(JOYINFOEX);
            jiInfo.dwFlags = JOY_RETURNALL;

            //joyGetPosEx(JOYSTICKID1, &jiInfo);
            joyGetPosEx( _joyID, &jiInfo);

            x = jiInfo.dwXpos;
            y = jiInfo.dwYpos;
   
            if( x < centerx ) {
                j.ev.joy.x = -( float )( x - centerx ) / ( minx - centerx );
            } else {
                j.ev.joy.x = ( float )( x - centerx ) / ( maxx - centerx );
            }

            if( y < centery ) {
                j.ev.joy.y = ( float )( y - centery ) / ( miny - centery );
            } else {
                j.ev.joy.y = -( float )( y - centery ) / ( maxy - centery );
            }

            j.ev.joy.button[0] = jiInfo.dwButtons&JOY_BUTTON1;
            j.ev.joy.button[1] = jiInfo.dwButtons&JOY_BUTTON2;
            j.ev.joy.button[2] = jiInfo.dwButtons&JOY_BUTTON3;
            j.ev.joy.button[3] = jiInfo.dwButtons&JOY_BUTTON4;

            j.ev.joy.button[4] = jiInfo.dwButtons&JOY_BUTTON5;
            j.ev.joy.button[5] = jiInfo.dwButtons&JOY_BUTTON6;
            j.ev.joy.button[6] = jiInfo.dwButtons&JOY_BUTTON7;
            j.ev.joy.button[7] = jiInfo.dwButtons&JOY_BUTTON8;
            j.ev.joy.button[8] = jiInfo.dwButtons&JOY_BUTTON9;
            j.ev.joy.button[9] = jiInfo.dwButtons&JOY_BUTTON10;
            j.ev.joy.button[10] = jiInfo.dwButtons&JOY_BUTTON11;
            j.ev.joy.button[11] = jiInfo.dwButtons&JOY_BUTTON12;
            j.ev.joy.button[12] = jiInfo.dwButtons&JOY_BUTTON13;
            j.ev.joy.button[13] = jiInfo.dwButtons&JOY_BUTTON14;
            j.ev.joy.button[14] = jiInfo.dwButtons&JOY_BUTTON15;
            j.ev.joy.button[15] = jiInfo.dwButtons&JOY_BUTTON16;
            j.ev.joy.button[16] = jiInfo.dwButtons&JOY_BUTTON17;
            j.ev.joy.button[17] = jiInfo.dwButtons&JOY_BUTTON18;
            j.ev.joy.button[18] = jiInfo.dwButtons&JOY_BUTTON19;
            j.ev.joy.button[19] = jiInfo.dwButtons&JOY_BUTTON20;
            j.ev.joy.button[20] = jiInfo.dwButtons&JOY_BUTTON21;
            j.ev.joy.button[21] = jiInfo.dwButtons&JOY_BUTTON22;
            j.ev.joy.button[22] = jiInfo.dwButtons&JOY_BUTTON23;
            j.ev.joy.button[23] = jiInfo.dwButtons&JOY_BUTTON24;
            j.ev.joy.button[24] = jiInfo.dwButtons&JOY_BUTTON25;
            j.ev.joy.button[25] = jiInfo.dwButtons&JOY_BUTTON26;
            j.ev.joy.button[26] = jiInfo.dwButtons&JOY_BUTTON27;
            j.ev.joy.button[27] = jiInfo.dwButtons&JOY_BUTTON28;
            j.ev.joy.button[28] = jiInfo.dwButtons&JOY_BUTTON29;
            j.ev.joy.button[29] = jiInfo.dwButtons&JOY_BUTTON30;
            j.ev.joy.button[30] = jiInfo.dwButtons&JOY_BUTTON31;
            j.ev.joy.button[31] = jiInfo.dwButtons&JOY_BUTTON32;
        }
#endif

         /* filter noisy joystick input, see if anythings changed */

        moved_this = ((fabs(j.ev.joy.x) > 0.125 ) || (fabs(j.ev.joy.y) > 0.125 ));

#if 0
        if ( moved_last || moved_this ||
            (j.ev.joy.button[0] != last_button[0]) ||
            (j.ev.joy.button[1] != last_button[1]) ||
            (j.ev.joy.button[2] != last_button[2]) ||
            (j.ev.joy.button[3] != last_button[3])) {
                atiDeliverEvent( &j, FXFALSE );
        } 
#endif
        if ( moved_last || moved_this )	{
                atiDeliverEvent( &j, FXFALSE );
        } else {
			int i = 0;
			FxBool done = FXFALSE;
			while ( i < MAX_BUTTONS && !done )	{
				if ( j.ev.joy.button[i] != last_button[i] )	{
	                atiDeliverEvent( &j, FXFALSE );
					done = FXTRUE;
				} // endif
				i++;
			}
			if ( !done &&
				(		j.ev.joy.button[16] != 0 
					||	j.ev.joy.button[17] != 0
					||  j.ev.joy.button[18] != 0 
					||	j.ev.joy.button[19] != 0
				)
			   )
			{
	                atiDeliverEvent( &j, FXFALSE );
			} 
		}

        moved_last = moved_this;
        last_joyx = j.ev.joy.x;
        last_joyy = j.ev.joy.y;
		memcpy( last_button, j.ev.joy.button, 32 * sizeof( int ) );
        /*
		last_button[0] = j.ev.joy.button[0];
        last_button[1] = j.ev.joy.button[1];
        last_button[2] = j.ev.joy.button[2];
        last_button[3] = j.ev.joy.button[3];
		*/
    }
}

void
atiDeliverEvents() {
    AtiEvent e;

    /* process any event/interrupt driven devices */

    while ( atiState.firstEvent != atiState.lastEvent ) {
        atiDeliverEvent( &atiState.queue[atiState.firstEvent], FXFALSE);

        atiState.firstEvent = ( atiState.firstEvent + 1 ) % ATI_MAX_NUM_EVENTS;
    }

    /* finally process any polled devices */

    atiPollDevices();

    /* write end of frame record */

    e.device = ATI_DEV_FRAME;
    e.ev.frame.frameNumber = atiState.currentFrame;
    atiDeliverEvent( &e, FXFALSE );
}

static FxBool ProcessInputEvents() {

    /* if we are using the gci for events, just let it handle them */

    if (atiState.haveGCI) {
#ifndef __DJGPP__
        gciHandleEvents(_AtiGCICallback);
#endif
        return FXTRUE;
    }

    /* Process system messages */

    if (atiState.msgFunc) {
        (* atiState.msgFunc)();
    }

    atiDeliverEvents();

    return FXTRUE;
}

static FxBool PlaybackEvents() {
    AtiEvent e;

    if ( atiState.currentFrame >= atiState.numFrames )
        return FXFALSE;

    while ( atiState.eventMode == ATI_EM_PLAYBACK ) {
        if ( fread( &e, sizeof( e ), 1, atiState.eventFile ) != 1 ) {
            atuError(FXFALSE, "Error reading event file\n");
            return FXFALSE;
        }

        if ( e.device == ATI_DEV_FRAME )
            break;

       atiDeliverEvent(&e, FXTRUE);
    }

    return FXTRUE;
}

FxBool atiHandleEvents(void) {
    if ( atiState.eventMode == ATI_EM_PLAYBACK ) {
        if (!PlaybackEvents())
            return FXFALSE;
    }

    if (!ProcessInputEvents())
        return FXFALSE;

    atiState.currentFrame++;
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atiReplayEvents();
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description:
    Replay the event file
  Arguments:
    None
  Return:
    Nothing
  -------------------------------------------------------------------*/

FxBool atiReplayEvents(void) {
    if ( atiState.eventMode == ATI_EM_PLAYBACK ) {
        /* rewind(atiState.eventFile); */
        fclose(atiState.eventFile);
        atiOpenEventFile(atiState.eventMode);
        atiState.currentFrame = 0;
        return FXTRUE;
    } else return FXFALSE;
}

/*-------------------------------------------------------------------
  Function: atiKeyboardFunc();
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description:
    Specify callback for keyboard events
  Arguments:
    kbFunc - keyboard handler
  Return:
    Prevously installed CB
  -------------------------------------------------------------------*/

AtiKeyboardCB atiKeyboardFunc(AtiKeyboardCB kbFunc) {
   AtiKeyboardCB old = atiState.kbFunc ;
   atiState.kbFunc = kbFunc;
   return old;
}

/*-------------------------------------------------------------------
  Function: atiJoystickFunc();
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description:
    Specify callback for joystick events 
  Arguments:
    joyFunc - joystick handler
  Return:
    Prevously installed CB
  -------------------------------------------------------------------*/

AtiJoystickCB atiJoystickFunc(AtiJoystickCB joyFunc) {
   AtiJoystickCB old = atiState.joyFunc ;
   atiState.joyFunc = joyFunc;
   return old;
}

/*-------------------------------------------------------------------
  Function: atiGCIFunc();
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description:
    Specify callback for gci events
  Arguments:
    gciFunc - gci handler
  Return:
    Prevously installed CB
  -------------------------------------------------------------------*/

AtiGCICB atiGCIFunc(AtiGCICB gciFunc) {
   AtiGCICB old = atiState.gciFunc ;
   atiState.gciFunc = gciFunc;
   return old;
}

/*-------------------------------------------------------------------
  Function: atiMsgFunc();
  Date: 10/11/96
  Implementor(s): mlwp
  Library: AT Input
  Description:
    Specify callback for processing system messages
  Arguments:
    msgFunc - message handler
  Return:
    Prevously installed CB
  -------------------------------------------------------------------*/

AtiMessageCB atiMessageFunc(AtiMessageCB msgFunc) {
   AtiMessageCB old = atiState.msgFunc ;
   atiState.msgFunc = msgFunc;
   return old;
}

/*-------------------------------------------------------------------
  Function: atiFocusLostFunc();
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description:
    Specify callback for focus lost events
  Arguments:
    focusFunc - focus lost handler
  Return:
    Prevously installed CB
  -------------------------------------------------------------------*/

AtiFocusCB atiFocusLostFunc( AtiFocusCB focusFunc) {
   AtiFocusCB old = atiState.focusLost ;
   atiState.focusLost = focusFunc;
   return old;
}

/*-------------------------------------------------------------------
  Function: atiFocusGainFunc();
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description:
    Specify callback for focus gained events
  Arguments:
    focusFunc - focus gained handler
  Return:
    Prevously installed CB
  -------------------------------------------------------------------*/

AtiFocusCB atiFocusGainFunc(AtiFocusCB focusFunc) {
   AtiFocusCB old = atiState.focusGain ;
   atiState.focusGain = focusFunc;
   return old;
}

/*-------------------------------------------------------------------
  Function: atiWinCloseFunc();
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description:
    Specify callback for window closed events
  Arguments:
    winCloseFunc - window closed handler
  Return:
    Prevously installed CB
  -------------------------------------------------------------------*/

AtiWinCloseCB atiWinCloseFunc(AtiWinCloseCB winCloseFunc) {
   AtiWinCloseCB old = atiState.winClose ;
   atiState.winClose = winCloseFunc;
   return old;
}

/*-------------------------------------------------------------------
  Function: atiPutEventFunc();
  Date: 6/16/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description:
    Specify callback for put events
  Arguments:
    PutEventFunc - put event handler
  Return:
    Prevously installed CB
  -------------------------------------------------------------------*/

AtiEventCB atiPutEventFunc(AtiEventCB putEventFunc) {
   AtiEventCB old = atiState.putEventFunc ;
   atiState.putEventFunc = putEventFunc;
   return old;
}

/*-------------------------------------------------------------------
  Function: _atiPutEvent
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description:
    Internal function to insert an event into the input queue.
  Arguments:
    event - event to add
  Return:
    none
  -------------------------------------------------------------------*/

void _atiPutEvent( AtiEvent *ev ) {

    if ( atiState.putEventFunc )
        ( * atiState.putEventFunc ) (ev);

    if ( atiState.firstEvent == ( atiState.lastEvent + 1 ) % ATI_MAX_NUM_EVENTS ) 
        return; /* Queue full */

    atiState.queue[atiState.lastEvent] = *ev;
    atiState.lastEvent = ( atiState.lastEvent + 1 ) % ATI_MAX_NUM_EVENTS;
}
