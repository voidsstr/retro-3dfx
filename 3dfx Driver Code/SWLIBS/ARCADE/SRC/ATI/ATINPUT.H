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
** $Date: 10/11/00 7:33:32 PM$ 
**
*/

#ifndef _ATINPUT_H_
#   define _ATINPUT_H_

#ifdef __WIN32__
#include <windows.h>
#endif
#ifdef __DOS32__
#include <dos.h>
#include <conio.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <3dfx.h>
#include <gci.h>
#include <atutil.h>

BEGINFDECL()

#define ATI_DEFAULT_EVENT_FILE "events.bin"

/* available devices */

typedef FxU32 AtiDevice;

#define ATI_DEV_FRAME       0
#define ATI_DEV_KEYBOARD    1
#define ATI_DEV_GCI         2
#define ATI_DEV_JOYSTICK    3

/* input modes */

#define ATI_EM_CLOSED    0
#define ATI_EM_DEFAULT   1
#define ATI_EM_RECORD    2
#define ATI_EM_PLAYBACK  3

/*-------------------------------------------------------------------
  Structure: AtiKeyEvent
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description: 
    Container for keyboard events.
  Members:
    key        - scan code for keys
    state      - key press or release
  -------------------------------------------------------------------*/

typedef struct {
    FxU16   code;
    FxU8    state;
} AtiKeyEvent;

/* key codes */

#define ATI_KEY_NOT_USED             0
#define ATI_KEY_ESCAPE               1
#define ATI_KEY_1                    2
#define ATI_KEY_2                    3
#define ATI_KEY_3                    4
#define ATI_KEY_4                    5
#define ATI_KEY_5                    6
#define ATI_KEY_6                    7
#define ATI_KEY_7                    8
#define ATI_KEY_8                    9
#define ATI_KEY_9                    10
#define ATI_KEY_0                    11
#define ATI_KEY_MINUS                12
#define ATI_KEY_EQUAL                13
#define ATI_KEY_BACKSPACE            14
#define ATI_KEY_TAB                  15
#define ATI_KEY_Q                    16
#define ATI_KEY_W                    17
#define ATI_KEY_E                    18
#define ATI_KEY_R                    19
#define ATI_KEY_T                    20
#define ATI_KEY_Y                    21
#define ATI_KEY_U                    22
#define ATI_KEY_I                    23
#define ATI_KEY_O                    24
#define ATI_KEY_P                    25
#define ATI_KEY_LBRACKET             26
#define ATI_KEY_RBRACKET             27
#define ATI_KEY_ENTER                28
#define ATI_KEY_LCTRL                29
#define ATI_KEY_A                    30
#define ATI_KEY_S                    31
#define ATI_KEY_D                    32
#define ATI_KEY_F                    33
#define ATI_KEY_G                    34
#define ATI_KEY_H                    35
#define ATI_KEY_J                    36
#define ATI_KEY_K                    37
#define ATI_KEY_L                    38
#define ATI_KEY_SEMICOLON            39
#define ATI_KEY_LPAREN               40
#define ATI_KEY_RPARNE               41
#define ATI_KEY_LSHIFT               42
#define ATI_KEY_Z                    44
#define ATI_KEY_X                    45
#define ATI_KEY_C                    46
#define ATI_KEY_V                    47
#define ATI_KEY_B                    48
#define ATI_KEY_N                    49
#define ATI_KEY_M                    50
#define ATI_KEY_COMMA                51
#define ATI_KEY_PERIOD               52
#define ATI_KEY_DIVIDE               53
#define ATI_KEY_RSHIFT               54
#define ATI_KEY_KEYPAD_MULT          55
#define ATI_KEY_LALT                 56
#define ATI_KEY_SPACE                57
#define ATI_KEY_CAPS                 58
#define ATI_KEY_F1                   59
#define ATI_KEY_F2                   60
#define ATI_KEY_F3                   61
#define ATI_KEY_F4                   62
#define ATI_KEY_F5                   63
#define ATI_KEY_F6                   64
#define ATI_KEY_F7                   65
#define ATI_KEY_F8                   66
#define ATI_KEY_F9                   67
#define ATI_KEY_F10                  68
#define ATI_KEY_BREAK                69
#define ATI_KEY_SCROLLLOCK           70
#define ATI_KEY_KEYPAD_7             71
#define ATI_KEY_KEYPAD_8             72
#define ATI_KEY_KEYPAD_9             73
#define ATI_KEY_KEYPAD_MINUS         74
#define ATI_KEY_KEYPAD_4             75
#define ATI_KEY_KEYPAD_5             76
#define ATI_KEY_KEYPAD_6             77
#define ATI_KEY_KEYPAD_PLUS          78
#define ATI_KEY_KEYPAD_1             79
#define ATI_KEY_KEYPAD_2             80
#define ATI_KEY_KEYPAD_3             81
#define ATI_KEY_KEYPAD_0             82
#define ATI_KEY_KEYPAD_DECIMAL       83
#define ATI_KEY_F11                  87
#define ATI_KEY_F12                  88
#define ATI_KEY_KEYPAD_ENTER         156
#define ATI_KEY_RCTRL                157
#define ATI_KEY_PRNTSCREEN1          170
#define ATI_KEY_KEYPAD_DIVIDE        181
#define ATI_KEY_PRNTSCREEN2          183
#define ATI_KEY_RALT                 184
#define ATI_KEY_NUMLOCK              197
#define ATI_KEY_HOME                 199
#define ATI_KEY_UP                   200
#define ATI_KEY_PAGEUP               201
#define ATI_KEY_LEFT                 203
#define ATI_KEY_RIGHT                205
#define ATI_KEY_END                  207
#define ATI_KEY_DOWN                 208
#define ATI_KEY_PAGEDOWN             209
#define ATI_KEY_INSERT               210
#define ATI_KEY_DELETE               211

/* key states */

#define ATI_KEY_RELEASE   0
#define ATI_KEY_PRESS   255

/*-------------------------------------------------------------------
  Structure: AtiJoystick
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description: 
    Container for joystick events.
  Members:
    joyx       - x coordinate of joystick
    joyy       - y coordinate of joystick
    buttons    - state of buttons
  -------------------------------------------------------------------*/

#define ATI_BUTTON_UP    0
#define ATI_BUTTON_DOWN  1

typedef struct {
    float        x ;
    float        y;
    int          button[32] ;
} AtiJoystickEvent;

/*-------------------------------------------------------------------
  Structure: AtiGCIEvent
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description: 
    Container for GCI events.
  -------------------------------------------------------------------*/

typedef GciJammaState AtiGCIEvent;

/*-------------------------------------------------------------------
  Structure: AtiFrameEvent
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description: 
    Container for end of frame events.  A frame event is generated after 
    all input events for a given frame have been processed.
  Members:
    frameNumber - frame number since start of session
  -------------------------------------------------------------------*/

typedef struct {
    FxU32     frameNumber;
} AtiFrameEvent;

/*-------------------------------------------------------------------
  Structure: AtiEvent
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description: 
    Container for input events.  Includes a device id and a 
    value.
  Members:
    device     - device generating event
    keyEvent   - keyEvent for keyboard devices
    joyEvent   - keyEvent for joystick devices
    gciEvent   - gciEvent for GCI devices
    frameEvent - frameEvent for end of frame events
  -------------------------------------------------------------------*/

typedef union {
    AtiKeyEvent          key;
    AtiJoystickEvent     joy;
    AtiGCIEvent          gci;
    AtiFrameEvent        frame;
} AtiAnyEvent;

typedef struct {
    AtiDevice    device;
    AtiAnyEvent  ev;
} AtiEvent;

typedef void (* AtiKeyboardCB)(AtiKeyEvent *ev, FxBool preRecorded);

typedef void (* AtiJoystickCB)(AtiJoystickEvent *ev, FxBool preRecorded);

typedef void (* AtiGCICB)(AtiGCIEvent *ev, FxBool preRecorded);

typedef void (* AtiEventCB)(AtiEvent *ev);

typedef void (* AtiFocusCB)(FxBool focus);

typedef void (* AtiWinCloseCB)(void);

typedef void (* AtiMessageCB)(void);

#define ATI_QUEUE_EMPTY  0

/*-------------------------------------------------------------------
  Structure: AtiEventHeader
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description: 
    Header for event files
  Members:
    magic      - magic number to validate file type
    version    - format version number
    numFrames  - number of data frames in file
  -------------------------------------------------------------------*/

typedef struct {
    FxU32 magic ;
    FxU32 versionNumber ;
    FxU32 numFrames ;
} AtiEventHeader ;

/*-------------------------------------------------------------------
  Structure: AtiState
  Date: 6/11/96
  Implementor(s): jdt, mlwp
  Library: AT Input
  Description: 
    Internal state for ATI
  Members:
    eventMode      - current event mode
    currentFrame   - current frame number (on playback)
    numFrames      - number of frames so far
    kbFunc         - callback function for keyboard events
    joyFunc        - callback function for joystick events
    gciFunc        - callback function for GCI events
    putEventFunc   - callback function for put events
    focusLost      - callback function for focus lost events
    focusGain      - callback function for focus gain events
    winClose       - callback function for window close events
    eventFile      - file pointer for retrieving/storing input events
    eventFileName  - name of file for retrieving/storing input events
    haveJoystick   - joystick present
    haveKeyboard   - keyboard present
    haveGCI        - GCI hardware present
    firstEvent     - pointer to first event in queue
    lastEvent      - pointer to last event in queue
    queue          - array where queue elements are stored
  -------------------------------------------------------------------*/

#define ATI_MAX_NUM_EVENTS          256

typedef struct {
   FxU32              eventMode; /* must be first element in structure */
   FxU32              currentFrame;
   FxU32              numFrames;
   AtiKeyboardCB      kbFunc;
   AtiJoystickCB      joyFunc;
   AtiGCICB           gciFunc;
   AtiEventCB         putEventFunc;
   AtiFocusCB         focusLost;
   AtiFocusCB         focusGain;
   AtiMessageCB       msgFunc;
   AtiWinCloseCB      winClose;
   FILE              *eventFile;
   char              *eventFileName;
   FxBool             bResizingDisabled;  /* do not allow resizing */
   FxBool             bMinimized;         /* input window is minimized */
   FxBool             haveJoystick;
   FxBool             haveKeyboard;
   FxBool             haveGCI;
   FxU32              firstEvent;
   FxU32              lastEvent;
   AtiEvent           queue[ATI_MAX_NUM_EVENTS];
   
} AtiState;

#ifdef __WIN32__
LRESULT CALLBACK atiWindowProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam);
#endif

extern AtiState atiState ;

FxBool atiInit(AtiMessageCB msgFunc);

void atiShutdown(void);

FxBool atiQueryDevice(FxU32 device);

void atiEventHistoryFile(char *fileName);

FxBool atiEventMode(FxU32 eventMode);

FxBool atiHandleEvents(void);

AtiKeyboardCB atiKeyboardFunc(AtiKeyboardCB kbFunc);

AtiJoystickCB atiJoystickFunc(AtiJoystickCB jsFunc);

AtiGCICB atiGCIFunc(AtiGCICB gciFunc);

AtiMessageCB atiMessageFunc(AtiMessageCB msgFunc);

AtiFocusCB atiFocusLostFunc(AtiFocusCB func);

AtiFocusCB atiFocusGainFunc(AtiFocusCB func);

AtiWinCloseCB atiWinCloseFunc(AtiWinCloseCB winCloseFunc);

void atiJoystickCalibrate(void);

atiGetCurrentFrame(void);

atiGetNumFrames(void);

FxBool atiReplayEvents(void);

void atiDeliverEvents(void);

void atiPollDevices(void);

void _atiPutEvent( AtiEvent *ev );

AtiEventCB atiPutEventFunc(AtiEventCB putEventFunc);

// Added to allow to select between JOYSTICKID1 and JOYSTICKID2
// default is JOYSTICKID1
void atiSelectJoystickID( UINT joyID );
void atiInitJoystick( void );

ENDFDECL()

#endif
