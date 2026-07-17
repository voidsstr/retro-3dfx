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
** $Date: 10/11/00 7:33:26 PM$ 
**
*/
 #pragma warn_unusedvar off
#include <glide.h>
#include <atutil.h>
#include <atrender.h>
#include <atinput.h>
#include <macinput.h>
#include "atdemop.h"

#include <Types.h>
#include <Memory.h>
#include <Quickdraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Menus.h>
#include <Windows.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <OSUtils.h>
#include <ToolUtils.h>
#include <GestaltEqu.h>
#include <LowMem.h> /* for LMGetRomNase, etc. */
#include <FSM.h>
#include <sioux.h>
#include <Processes.h>
#include <Displays.h>
#include <Files.h>

#ifdef __USE_DS__
#include "DrawSprocket.h"
#else
#include <Displays.h>
#endif

#ifdef ATBSOUND
#include <ataudio.h>
#endif
#include <assert.h>

#include <3dsinc.h>	/* for TDSSetErrorFP() */

#define MAX_PATH_SIZE 2048

/* frontend for demo applications so that they can run in different OS
   environments (WIN95, DOS, MacOS etc. without requiring code changes.
   Assumes a simple model of 1 display surface. Handles input from a 
   single window
 */

/* variables that are limited in scope to this file. Used primarily
   for environement variables and the like.
 */
ProcessInfoRec app_info;
char app_name[65] = "\pATDemo";
FSSpec app_spec;
ProcessSerialNumber app_psn;
int debugger_avail = FXFALSE;
char app_path[MAX_PATH_SIZE];
char *app_argv[1];
long start_ticks;

#ifdef __USE_DS__
DSpContextReference theContext;
GDHandle context_device;
#else
CWindowRecord app_win;
#endif

#define WINDOW_ID 128
#define SMALL_WINDOW_ID 129


static void atdErrorCallback( FxBool fatal, const char *format, ... );

extern int AppMain(int argc, char **argv); /* in the programs main file */


#ifdef __USE_DS__
void set_display_env(void);
void reset_display_env(void);
void activate_context(void);
void deactivate_context(void);
#endif

 /* Debugger types */
typedef enum DebuggerType {
     kNoDebugger,
     kMacsBug,
     kTMON,
     kJasiksDebugger,
     kOtherDebugger
 } DebuggerType;


 // Private defines for some low memory globals.
 #define MacJmp      ((Ptr *)0x0120)     // MacsBug jumptable [pointer].
 #define MacJmpByte  ((UInt8 *)0x0120)   // MacsBug flags in 24 bit mode [byte].
 #define MacJmpFlag  ((UInt8 *)0x0BFF)   // MacsBug flag [byte].

 // Debugger flag bits.
 #define kDebuggerInstalledBit   5

/****************************************************************************/
/*                           GetDebuggerInfo                                */
/****************************************************************************/
/*
 * Make sure a debugger is running for testing purposes
 */

static Boolean
GetDebuggerInfo(
	DebuggerType *  outDebuggerType,
	UInt16 *        outDebuggerSignature ) {
	Boolean theResult = false;
	SInt32  theResponse;
	char jasik_name[63] = "\pThe Debugger";

	jasik_name[13] = 0;
	// Initialize return values to defaults.
	*outDebuggerType = kNoDebugger;
	*outDebuggerSignature = '  ';

	if ( Gestalt( gestaltAddressingModeAttr, &theResponse ) == noErr ) {

		UInt16  theDebugFlags;

		// As documented in the "MacsBug Reference & Debugging Guide", page 412
		// if we have a 32 bit capable Memory Manager, debugger flags are at 0x0BFF
		// if we have a 24 bit capable Memory Manager, debugger flags are at 0x0120

		if ( (theResponse & (1L << gestalt32BitCapable)) != 0 ) {
			theDebugFlags = *MacJmpFlag;
		} else {
			theDebugFlags = *MacJmpByte;
		}

		if ( (theDebugFlags & (1L << kDebuggerInstalledBit)) != 0 ) {

			Ptr theDebuggerEntry;
			Ptr theROMBaseWorld;

			// There is a debugger installed.
			theResult = true;

			// Get the debugger entry.
			theDebuggerEntry = StripAddress( *MacJmp );

			// Get the ROM base.
			theROMBaseWorld = StripAddress( LMGetROMBase() );

			// Compare the debugger entry to the ROM base.
			if ( theDebuggerEntry < theROMBaseWorld ) {

				UInt16  **theDebuggerWorld;

				// It's not a ROM based debugger.
				// Get the debugger world.
				theDebuggerWorld = (UInt16 **) StripAddress( theDebuggerEntry - sizeof(Ptr) );

				// Get the debugger signature.
				*outDebuggerSignature = **theDebuggerWorld;

				// Get the debugger type.
				switch ( *outDebuggerSignature ) {

					case 'MT':
					{
						/* check to see if it is The Debugger */
						FCBRecPtr rp;
						char *tmp_ptr;
						int Jasiks = false;

						tmp_ptr = LMGetFCBSPtr();
						tmp_ptr += 2;
						rp = (void*)tmp_ptr;
						while(rp->fcbFlNm != 0) {
							char *cs;
							cs = (void*)rp->fcbCName;
							if(!strcmp(cs, jasik_name) &&  rp->fcbFType == 'DBGR') {
								Jasiks = true;
								break;
							}
							rp++;
						}
						if(Jasiks) {
							*outDebuggerType = kJasiksDebugger;
						} else {
							*outDebuggerType = kMacsBug;
						}
						break;
					}

					case 'WH':
						*outDebuggerType = kTMON;
						break;

					default:
						*outDebuggerType = kOtherDebugger;
						break;

				}

			}

		}

     }

     return theResult;
 }


/****************************************************************************/
/*                              set_display_env                             */
/****************************************************************************/
/*
 * Take over the screen, get rid of menu bar, set resolution...
 */
#ifdef __USE_DS__
static Boolean mev(EventRecord *ev) {
	#pragma unused (ev)
	return false;
}

void set_display_env(void) {
	DSpContextAttributes atr, atr2;
	//DSpContextAttributes theDesiredAttributes;
	//DSpAltBufferReference theUnderlay, theOverlay;
	UInt32 theDisplayWidth, theDisplayHeight;
	//CGrafPtr theAltBufferPort;
	//GDHandle theAltBufferGDevice, theOldGDevice;
	//GrafPtr theOldPort;
	OSStatus theError;
	DisplayIDType dtype;
	OSErr os_err;

	/* initialize the draw sprockets */
	theError = DSpStartup();
	assert(theError == 0);

	/* clear out all the garbage */
	memset(&atr, 0, sizeof(DSpContextAttributes));

	/* set up the original specifications */
	atr.frequency				= 75L << 16;
	atr.displayWidth			= 640;
	atr.displayHeight			= 480;
	atr.backBufferDepthMask		= kDSpDepthMask_16;
	atr.displayDepthMask		= kDSpDepthMask_16;
	atr.backBufferBestDepth		= 16;
	atr.displayBestDepth		= 16;
	atr.pageCount				= 1;

	#ifdef AT_DEBUGGING
	theError = DSpSetDebugMode( true );
	assert(theError == 0 || theError == kDSpConfirmSwitchWarning);
	#endif

	theError = DMGetDisplayIDByGDevice( LMGetMainDevice(), &dtype, false );	
	assert(theError == noErr);
	theError = DSpGetFirstContext( dtype, &theContext );
	assert(theError == noErr);
	while(theError == noErr) {
		DSpContextReference tc;
		theError = DSpContext_GetAttributes( theContext, &atr );
		assert(theError == noErr);

		theError = DSpContext_Reserve(theContext, &atr);
		assert(theError == 0 || theError == kDSpConfirmSwitchWarning);

		
		theError = DSpContext_SetState( theContext, kDSpContextState_Active );
		assert(theError == 0 || theError == kDSpConfirmSwitchWarning);

		while(!Button()){}

		theError = DSpContext_SetState( theContext, kDSpContextState_Inactive );
		assert(theError == 0 || theError == kDSpConfirmSwitchWarning);

		theError = DSpContext_Release(theContext);
		assert(theError == 0 || theError == kDSpConfirmSwitchWarning);

		tc = theContext;
		theError = DSpGetNextContext( tc, &theContext );
		assert(theError == noErr || theError == kDSpContextNotFoundErr);
	}
	theError = DSpShutdown();
	assert(theError == 0);
	ExitToShell();
	

	theError = DSpFindBestContext( &atr, &theContext );	
	assert(theError == 0 || theError == kDSpConfirmSwitchWarning);

	theDisplayWidth = 640;
	theDisplayHeight = 480;
	
	theError = DSpContext_Reserve(theContext, &atr);
	assert(theError == 0 || theError == kDSpConfirmSwitchWarning);

	
	theError = DSpContext_GetDisplayID(theContext, &dtype);
	assert(theError == 0);

	os_err = DMGetGDeviceByDisplayID(dtype, &context_device, FXFALSE);
	assert(os_err == noErr);
}


void activate_context(void) {

	OSStatus theError;
	
	/*
	** fade out all displays to black, you must have at least one reserved
	** context to do this or you will get an error.  A game should always
	** fade to black before activating a context because if the activation
	** causes a resolution change the user will see a very ugly twitch in
	** the display.
	*/

	//theError = DSpContext_FadeGammaOut( NULL, NULL );
	//assert(theError == 0);

	/* put the context into the active state */

	theError = DSpContext_SetState( theContext, kDSpContextState_Active );
	assert(theError == 0 || theError == kDSpConfirmSwitchWarning);

	/* fade back in */
	//theError = DSpContext_FadeGammaIn( NULL, NULL );
	//assert(theError == 0);
}


void deactivate_context(void) {

	OSStatus theError;
	
	/*
	** fade out all displays to black, you must have at least one reserved
	** context to do this or you will get an error.  A game should always
	** fade to black before activating a context because if the activation
	** causes a resolution change the user will see a very ugly twitch in
	** the display.
	*/
	//theError = DSpContext_FadeGammaOut( NULL, NULL );
	//assert(theError == 0 || theError == kDSpConfirmSwitchWarning);

	/* put the context into the inactive state */
	theError = DSpContext_SetState( theContext, kDSpContextState_Inactive );
	assert(theError == 0 || theError == kDSpConfirmSwitchWarning);

	/* fade back in */
	//theError = DSpContext_FadeGammaIn( NULL, NULL );
	//assert(theError == 0 || theError == kDSpConfirmSwitchWarning);
	
}


/****************************************************************************/
/*                              reset_display_env                           */
/****************************************************************************/
/*
 * Reset the resolution and display depth to the users choice in Monitors CDEV
 */

void reset_display_env(void) {
	OSStatus theError;

	/* remove the context */
	theError = DSpContext_Release(theContext);
	assert(theError == 0 || theError == kDSpConfirmSwitchWarning);

	/* kill the draw sprockets */
	theError = DSpShutdown();
	assert(theError == 0 || theError == kDSpConfirmSwitchWarning);
}

#endif
/****************************************************************************/
/*                              mac_init                                    */
/****************************************************************************/
/*
 * Do all the toolboxy, macintosh stuff to get the program running
 */

static void mac_init(void) {
	int i;

	/* Initialize all the needed managers. */
	MaxApplZone();
	for(i = 0; i < 5; i++) {
		MoreMasters();
	}
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
	start_ticks = TickCount();
}

/****************************************************************************/
/*                              get_app_info                                */
/****************************************************************************/
/*
 * Initialize a ProcessInfoRec for the application
 */

static ProcessInfoRec *get_app_info(void) {
	OSErr err;
	DirInfo block;
	Str255 dir_str;
	
	GetCurrentProcess(&app_psn);
	app_info.processInfoLength = sizeof(ProcessInfoRec);
	app_info.processName = (void*)&app_name[0];
	app_info.processAppSpec = &app_spec;
	err = GetProcessInformation(&app_psn, &app_info);
	assert(err == noErr);
	app_path[0] = 0;
	block.ioDrParID = app_spec.parID;
	block.ioNamePtr = dir_str;
	do {
		block.ioVRefNum = app_spec.vRefNum;
		block.ioFDirIndex = -1;
		block.ioDrDirID = block.ioDrParID;
		err = PBGetCatInfoSync((CInfoPBRec*)&block);
		assert(err == noErr);
		dir_str[0]++;
		if(strlen(app_path) + dir_str[0] >= MAX_PATH_SIZE) {
			ExitToShell();
		}
		dir_str[dir_str[0]] = ':';
		memmove(&app_path[dir_str[0]], &app_path[0], strlen(app_path));
		memcpy(app_path, &dir_str[1], dir_str[0]);
	} while (block.ioDrDirID != 2);
	app_path[strlen(app_path) - 1] = 0;
	return &app_info;
}


/****************************************************************************/
/*                            atdSetName                                    */
/****************************************************************************/
/*
 * Initializes the application name, in both pascal and c styles
 */

void atdSetName(char *name) {
	int len = strlen(name);
	int i;
	
	if(len > 63) {
		len = 63;
	}
	
	for( i = 0; i < len; i++) {
		app_name[i+1] = name[i];
	}
	app_name[len+1] = 0;
	app_name[0] = len;
	app_argv[0] = &app_name[1];
}

/****************************************************************************/
/*                            atdSetFocus                                   */
/****************************************************************************/
/*
 * Sets the focus for the app (????)
 */

void atdSetFocus(void) {
}

/*-------------------------------------------------------------------
  Function: atdPrintf
  Date: 10/11/96
  Implementor(s): mlwp
  Library: AT Demo
  Description:
    Multi platform printf function
  Arguments:
    format - printf style format string
    ... - arguments determined by format string
  Return:
    none
  -------------------------------------------------------------------*/

void
atdPrintf( const char *format, ... ) {
    char buff[256];
    Str255 buffer;
    size_t len;
    extern int debugger_avail;
    va_list args;

    if(!debugger_avail) {
        return;
    }

    va_start( args, format );
    vsprintf( buff, format, args );
    va_end( args );
    len = strlen(buff);
    memcpy((void*)&buffer[1], buff, len);
    buffer[0] = len;
    DebugStr(buffer);
}

/*-------------------------------------------------------------------
  Function: atdGetString
  Date: 10/11/96
  Implementor(s): mlwp
  Library: AT Demo
  Description:
    Multi platform get string function
  Arguments:
    c        - buffer to receieve string
    buffSize - size of buffer
  Return:
    String read, or NULL on error
  -------------------------------------------------------------------*/

char *
atdGetString(char *c, int buffSize) {
    FXUNUSED(c);
    FXUNUSED(buffSize);
    DebugStr("\patdGetString: Not implemented yet.");

    return NULL;
}

/*-------------------------------------------------------------------
  Function: atdGetChar
  Date: 10/11/96
  Implementor(s): mlwp
  Library: AT Demo
  Description:
    Multi platform get character function
  Arguments:
    None
  Return:
    Character read
  -------------------------------------------------------------------*/

int
atdGetChar(void) {
    DebugStr("\patdGetChar: Not implemented yet.");
}

/****************************************************************************/
/*                               main                                       */
/****************************************************************************/
/*
 * Create a default window, check for options
 */

int main(void) {
	DebuggerType dt;
	UInt16 ui16;
	Rect device_box;
 	int failcount = 0; /* number of times RenderLoop has failed */


	SIOUXSettings.initializeTB = FALSE;
	SIOUXSettings.standalone = TRUE;
	SIOUXSettings.setupmenus = FALSE;
	SIOUXSettings.autocloseonquit = TRUE;
	SIOUXSettings.asktosaveonclose = FALSE;
	SIOUXSettings.showstatusline = FALSE;

	/* atuErrorSetCallback( atdErrorCallback );*/
	TDSSetErrorFP(stderr);
	/*
	 * Initialize the system so the program can run...
	 */

	mac_init();
	GetDebuggerInfo(&dt, &ui16);

	if(dt != kNoDebugger && 0) {
		debugger_avail = FXTRUE;
	} else {
		debugger_avail = FXFALSE;
	}
	/*
	 * Initialize the global variables
	 */
	_atdInitGlobals();
	_atGlobals.appInfo = get_app_info();
    _atGlobals.eventMode = ATI_EM_DEFAULT;

    #ifdef AT_RAVE_DRIVER
	_atGlobals.driverName = "Rave";
	#endif
	
    _atGlobals.argc = 1;
    _atGlobals.argv = app_argv;
	
	#ifdef ATBSOUND
	_atGlobals.soundAvailable = ataInit(NULL);
	#else
	_atGlobals.soundAvailable = FXFALSE;
	#endif

	/*
	 * init the display environment, and show the window
	 */
	#ifdef __USE_DS__
	set_display_env();
	#endif
	
	/* initialize the window */
	#ifndef __USE_DS__
	device_box = (**GetMainDevice()).gdRect;
	GetNewCWindow(WINDOW_ID, &app_win, (WindowPtr)-1L);
	MoveWindow((void*)&app_win, 0, 0, false);
	SizeWindow((void*)&app_win, device_box.right - device_box.left,
					device_box.bottom - device_box.top, false);
	/*ShowWindow((void*)&app_win);*/
	_atGlobals.hWndMain = &app_win;
	#else
	_atGlobals.hWndMain = context_device;
	#endif

    #ifdef __USE_DS__
	activate_context();
	#endif

    if (!AppInitGraphics()) {
       atuError(FXTRUE, "Can't initialize graphics\n");
    }


    /* initialize input library */

    if (!atiInit((void*)atiMsgFunc)) {
       atuError(FXTRUE, "Can't initialize input\n");
    }
    /* run the application */


	AppMain(0L, 0L);

	atiShutdown();
	AppTermGraphics();

	#ifdef __USE_DS__
	deactivate_context();
	reset_display_env();
	#endif
	return 0;
}
#if 0
static counter = 0;
KeyMap my_keys;
static short is_pressed(unsigned short k )
{
	#if 0
	KeyMap mk;
	memcpy(mk, &my_keys[counter & 0x3ff], sizeof(KeyMap));
	
	return ( ( mk[k>>3] >> (k & 7) ) & 1);
	#else
	unsigned char *b = (void*)my_keys;
	return ( ( b[k>>3] >> (k & 7) ) & 1);
	#endif
}


void key_hack(void);
void key_hack(void) {
	extern AtiState atiState;

	/*memcpy((void*)&my_keys[counter&0x3ff], (void*)0x174, sizeof(KeyMap));
	counter++;*/
	GetKeys((void*)my_keys);

	/* escape to quit */
	if(is_pressed(0x35)) {
		AtiKeyEvent ev;
		ev.state = ATI_KEY_PRESS;
		ev.code = ATI_KEY_ESCAPE;
		atiState.kbFunc(&ev, FXFALSE);
	}
	
	/* left arrow to increase rotational speed */
	if(is_pressed(0x7b) || is_pressed(0x56)) {
		AtiKeyEvent ev;
		ev.state = ATI_KEY_PRESS;
		ev.code = ATI_KEY_LEFT;
		atiState.kbFunc(&ev, FXFALSE);
	}

	/* left arrow to decrease rotational speed */
	if(is_pressed(0x7c) || is_pressed(0x58)) {
		AtiKeyEvent ev;
		ev.state = ATI_KEY_PRESS;
		ev.code = ATI_KEY_RIGHT;
		atiState.kbFunc(&ev, FXFALSE);
	}

	/* up arrow to zoom in */
	if(is_pressed(0x7e) || is_pressed(0x5b)) {
		AtiKeyEvent ev;
		ev.state = ATI_KEY_PRESS;
		ev.code = ATI_KEY_UP;
		atiState.kbFunc(&ev, FXFALSE);
	}

	/* down arrow to zoom out */
	if(is_pressed(0x7d) || is_pressed(0x54)) {
		AtiKeyEvent ev;
		ev.state = ATI_KEY_PRESS;
		ev.code = ATI_KEY_DOWN;
		atiState.kbFunc(&ev, FXFALSE);
	}
	
	/* F2 to change textures */
	if(is_pressed(0x78)) {
		AtiKeyEvent ev;
		ev.state = ATI_KEY_PRESS;
		ev.code = ATI_KEY_F2;
		atiState.kbFunc(&ev, FXFALSE);
	}
	
	/* F1 to toggle between sphere and cube */
	if(is_pressed(0x7a)) {
		AtiKeyEvent ev;
		ev.state = ATI_KEY_PRESS;
		ev.code = ATI_KEY_F1;
		atiState.kbFunc(&ev, FXFALSE);
	}
	
}
#endif
	
	
