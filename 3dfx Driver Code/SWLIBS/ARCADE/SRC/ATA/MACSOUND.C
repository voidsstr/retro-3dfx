//--------------------------------------------------------------------------------
//	mac atasound.c
//	by Linc Madison, Wirehead Systems, Oakland, California
//	created:	November 1, 1996
//	modified:	1996/11/04  17:30
//
//	MacOS implementation of sound functions for ATA for 3Dfx
//	Revision history:
//	1996/11/01	first cut: "mac atasound.c"
//	1996/11/04	typo correction, general cleanup, #include files, etc.
//--------------------------------------------------------------------------------

#if macintosh

// Macintosh #include files
#include <sound.h>	// Apple MacOS sound header file
#include <Resources.h>

#undef USING_APPLE_SOUND_SPROCKET
#ifdef USING_APPLE_SOUND_SPROCKET
	#include <SoundSprocket.h>
	#include <QD3D.h>
	// etc.
#endif USING_APPLE_SOUND_SPROCKET


// other #includes, 3Dfx, etc.
#include "atutil.h"
#include <assert.h>
#include <string.h>
#include <events.h>

// #include files -- 3Dfx
// "ataudio.h" needs to be included _after_ the typedefs above
#include "atutil.h"
#include "ataudio.h"

// function prototypes -- functions declared as null for Macintosh
void _downloadSound( Sound * s );
Sound * ataSoundNew( char * soundFile );
void ataSoundDelete( Sound * s );

// function prototypes -- functions actually used for Macintosh
FxBool ataInit( FxU32 handle );	// input parameter is ignored on Mac
void ataShutdown( void );
void ataSoundPlay( Sound * s, int flags );
void ataSoundStop( Sound * s );
void soundCleanUp( void );		// for the Mac, but needs to be called in main event loop

// function prototypes -- static Mac-only functions
static pascal void macCallback( SndChannelPtr channel, SndCommand * command );
static pascal void macCompletion( SndChannelPtr channel );

// defined constants
#define MAX_CHANNELS	CAT_COUNT	// number of sound channels to allocate
	// for now, MAX_CHANNELS should be the same as the number of sound categories above
#define kCB_SIGN		'3D'		// signature for our callbacks
static const SndCommand flush = { flushCmd, 0, 0L };
static const SndCommand	shush = { quietCmd, 0, 0L };

// static variables
static Boolean					soundFlag[ MAX_CHANNELS ];
static SndChannelPtr			soundChan[ MAX_CHANNELS ];
static FilePlayCompletionUPP	pfCompletion;

// only the SHORT_BANG channel issues callbacks; if that changes, "cback" will need to
// be changed to non-const to allow for param2 = (long)(&soundFlag[ channelNumber ]);
/* obviously, the above comment has become invalid - djh... */
static const SndCommand sb_cback = { callBackCmd, kCB_SIGN, 0L };
static SndCommand sl_cback = { callBackCmd, kCB_SIGN, 0L};
static const Boolean	kAsync = true;			// true == play sounds asynchronously
static const Boolean	kWaitForQueue = false;	// false == wait for space in cmd queue
static const Boolean	kKillItNow = true;		// true == silence channel immediately

// global variables
Boolean gSoundAttnFlag;


// function declarations

//--------------------------------------------------------------------------------
//	void ataInit( FxU32 handle )
//
//	by Linc Madison, 11/1/96
//	Purpose:	Initialize sound routines
//	Scope:		public
//	Input:		handle -- MS-Win object; ignored in Macintosh version
//	Returns:	FXTRUE or FXFALSE, if successful or unsuccessful
//	
//	sets all the sound attention flags to false, allocates sound channels,
//	will initialize 3-D sound routines (not yet implemented)
//--------------------------------------------------------------------------------
FxBool ataInit( FxU32 handle )
{
#pragma unused( handle )

	extern SndChannelPtr			soundChan[ MAX_CHANNELS ];
	extern Boolean					gSoundAttnFlag, soundFlag[ MAX_CHANNELS ];
	SndCallBackUPP					pfCallback;
	extern FilePlayCompletionUPP	pfCompletion;
	OSErr							err;
	int								i;

	sl_cback.param2 = (long)(&soundFlag[ SHORT_LOOP ]);
	pfCallback = NewSndCallBackProc( macCallback );
	pfCompletion = NewFilePlayCompletionProc( macCompletion );
	gSoundAttnFlag = false;

	for (i = 0; i < MAX_CHANNELS; i++)
	{
		soundFlag[ i ] = false;
		// change "initStereo" to "initMono" if using mono sounds 3-D localized with SSp
		err = SndNewChannel( &soundChan[ i ], sampledSynth, initStereo, pfCallback );
		if (err != noErr)
		{
			return FXFALSE;
		}
		assert( soundChan[ i ] );
		// insert SoundSprocket initialization code here, SSp
	}

	// the main program needs to allocate and initialize a global array of Sound structs,
	// pre-load and pre-lock the sound resource handles, and open all the sound files and
	// save their refNums.  -- Linc 11/1/96

	return FXTRUE;
}



//--------------------------------------------------------------------------------
//	void ataShutdown( void )
//
//	by Linc Madison, 11/1/96
//	Purpose:	Shut down sound routines
//	Scope:		public
//	Input:		void
//	Returns:	void
//	
//	dispose of the sound channels created by ataInit()
//--------------------------------------------------------------------------------
void ataShutdown( void )
{
	int				i;
	
	// could add various clean-up routines, sanity checks, etc., here if there
	// is any circumstance where this function would be called before program
	// termination. -- Linc 11/1/96
	
	for (i = 0; i < MAX_CHANNELS; i++)
	{
		SndDisposeChannel( soundChan[ i ], kKillItNow );
	}
	return;
}



//--------------------------------------------------------------------------------
//	void ataSoundPlay( Sound * s, int flags )
//
//	by Linc Madison, 11/1/96
//	Purpose:	Play a sound, either once or looping continuously
//	Scope:		public
//	Input:		s:		Sound struct, tells whether to play from file or
//						resource, as well as the file refnum or resId
//				flags:	tells whether to play once (0) or loop (ATA_SOUND_LOOP)
//	Returns:	void
//	
//	Use the Sound struct's "soundCategory" parameter to determine whether the
//	sound is played from disk (LONG) or memory (SHORT), and whether it is a
//	one-off sound (BANG) or a continuously looping sound (LOOP).  Play the
//	sound in the corresponding channel.
//
//	Sound resources for SHORT_LOOP sounds are assumed to have the header set
//	correctly for the loopStart and loopEnd parameters.
//--------------------------------------------------------------------------------
void ataSoundPlay( Sound * s, int flags )
{
	SoundCat				theCat = s->soundCategory;
	SndListHandle			theSndH;
	SoundHeaderPtr			theSndHeader;
	long					theOffset;
	SndCommand				loopCmd  = { soundCmd, 0, 0L };
	OSErr					err;
	SndChannelPtr			theChan;
	const long				bufSize = 0x10000;	// 64K buffer for play from disk
	FilePlayCompletionUPP	theFPC = NULL;
	
	if (flags != 0 && flags != ATA_SOUND_LOOP)				// illegal flags value
	{
		atuError(FXFALSE, "ataSoundPlay flags must be 0 or ATA_SOUND_LOOP!.\n");
		return;
	}
	
	if ( theCat < CAT_FIRST || theCat > CAT_LAST )		// value out of range
	{
		atuError(FXFALSE, "ataSoundPlay detected illegal soundCategory value in a Sound.\n");
		return;
	}
	
	// if ( (theCat is looping) != (flags is looping) )
	if ( (theCat & CAT_LOOP_FLAG != 0) != (flags == ATA_SOUND_LOOP) )
	{
		// if "theCat" and "flags" conflict, change "theCat" to correspond to "flags"
		theCat ^= CAT_LOOP_FLAG;
		s->soundCategory = theCat;
	}
	
	theChan = soundChan[ theCat ];
	assert( theChan );
	SndDoImmediate( theChan, &flush );		// clear any sounds remaining in this channel
	SndDoImmediate( theChan, &shush );
	// tell the channel what sound it's playing
	theChan->userInfo = (long)(s);
	
	if ( ! (theCat & CAT_LONG_FLAG) )		// short sound, play resource from memory
	{
		theSndH = (SndListHandle) GetResource( soundListRsrc, s->soundRef );
		if (theSndH == NULL)
		{
			err = ResError();
			return;
		}
		// assume for now that all the sound resources are pre-locked  -- Linc 11/1/96
	//	HLockHi( (Handle) theSndH );
		GetSoundHeaderOffset( theSndH, &theOffset );
		theSndHeader = (SoundHeaderPtr)( (long)(*theSndH) + theOffset );
	}
	switch (theCat)
	{
		case SHORT_BANG:
			err = SndPlay( theChan, theSndH, kAsync );
			if (err != noErr)
			{
				return;
			}
			// param2 of the cback command needs to be specified as the soundFlag if
			// any other channel could ever issue a callback -- Linc 11/1/96
			err = SndDoCommand( theChan, &sb_cback, kWaitForQueue );
			if (err != noErr)
			{
				return;
			}
		break;
		
		case SHORT_LOOP:
			err = SndPlay( theChan, theSndH, kAsync );
			if (err != noErr)
			{
				return;
			}
			// param2 of the cback command needs to be specified as the soundFlag if
			// any other channel could ever issue a callback -- Linc 11/1/96
			err = SndDoCommand( theChan, &sl_cback, kWaitForQueue );
			if (err != noErr)
			{
				return;
			}
		break;
		/*case SHORT_LOOP:
			// assume that the sound header has already been fixed; otherwise...
			// FixBrokenSoundLoop( theSndHeader );
			loopCmd.param2 = (long)(theSndHeader);
			err = SndDoImmediate( theChan, &loopCmd );
			WaitNextEvent(0, &tevt, 0, NULL);
			if (err != noErr)
			{
				return;
			}
			// don't need a callback command for SHORT_LOOP
		break;*/
		
		case LONG_BANG:
		case LONG_LOOP:
			// SSFP( chan, FileRef, RsrcNum, BufSize, *Buf, AudioSelection, completion, async );
			// NULL buffer pointer means auto-allocate; NULL AudioSel means the whole file
			err = SndStartFilePlay( theChan, s->soundRef, 0, bufSize,
					NULL, NULL, pfCompletion, kAsync );
			if (err != noErr)
			{
				return;
			}
		break;

		default:
			// deal with error
		return;		
	}
	return;
}



//--------------------------------------------------------------------------------
//	void ataSoundStop( Sound * s )
//
//	by Linc Madison, 11/1/96
//	Purpose:	Stop a specific sound currently playing
//	Scope:		public
//	Input:		s:	Sound struct for the selected sound to stop
//	Returns:	void
//	
//	Determine the channel corresponding to the input Sound and check to see
//	if that sound is currently playing on that channel.  If so, stop the
//	sound using the routine appropriate to the method of sound play.
//--------------------------------------------------------------------------------
void ataSoundStop( Sound * s )
{
	SndChannelPtr	theChan;
	SoundCat		theCat = s->soundCategory;
	OSErr			err;
	
	if ( theCat < CAT_FIRST || theCat > CAT_LAST )
	{
		return;
	}
	theChan = soundChan[ theCat ];
	
	if ( ((Sound *)(theChan->userInfo))->soundRef != s->soundRef )
	{
		// sound that we were supposed to stop isn't playing, so just return
		return;
	}

	switch (theCat)
	{
		case SHORT_BANG:
			soundFlag[ theCat ] = gSoundAttnFlag = true;
			err = SndDoImmediate( theChan, &flush );	// ignore any error here
			err = SndDoImmediate( theChan, &shush );
			if (err != noErr)
			{
				return;
			}
		break;
		case SHORT_LOOP:
			theChan->userInfo = 0L;		// prevents looped sound from re-queueing
			soundFlag[ theCat ] = gSoundAttnFlag = true;
			err = SndDoImmediate( theChan, &flush );	// ignore any error here
			err = SndDoImmediate( theChan, &shush );
			if (err != noErr)
			{
				return;
			}
		break;
		
		case LONG_BANG:
		case LONG_LOOP:
			theChan->userInfo = 0L;		// prevents looped sound from re-queueing
			err = SndStopFilePlay( theChan, kKillItNow );
			if (err != noErr)
			{
				return;
			}
		break;
		
		default:
		break;
	}
	
	return;
}



//--------------------------------------------------------------------------------
//	static pascal void macCallback( SndChannelPtr channel, SndCommand * command )
//
//	by Linc Madison, 11/1/96
//	Purpose:	notify the program that a sound has finished playing
//	Scope:		private
//	Input:		channel:	the sound channel that just completed playing
//				command:	the SndCommand that queued the callback
//	Returns:	void
//	
//	Check the "signature" parameter (param1) of the callback command to
//	ensure that we don't respond to a spurious callback.  Then set a flag
//	that must be checked in the main event loop to execute any required
//	cleanup routines.  This function must be fully interrupt-safe, but it
//	is not 680x0-compatible (does not restore A5 world).
//--------------------------------------------------------------------------------
static pascal void macCallback( SndChannelPtr channel, SndCommand * command )
{
#pragma unused( channel )
	extern Boolean gSoundAttnFlag, soundFlag[ MAX_CHANNELS ];
	
	if (command->param1 != kCB_SIGN)	// verify that it's our callback, not spurious
		return;
	
	// for now, a callback can only occur in the SHORT_BANG channel
	// if this assumption is broken, the address of the channel's attention flag
	// must be passed as a long in param2 when the callBackCmd is installed....
	if(command->param2) {
		Boolean *tmpb = (Boolean*)command->param2;
		gSoundAttnFlag = *tmpb = FXTRUE;
	} else {
		gSoundAttnFlag = soundFlag[ SHORT_BANG ] = FXTRUE;
	}
	// main event loop has to watch for gSoundAttnFlag to go true, then call
	// soundCleanUp();
	return;
}



//--------------------------------------------------------------------------------
//	static pascal void macCompletion( SndChannelPtr channel )
//
//	by Linc Madison, 11/1/96
//	Purpose:	notify the program that a sound-from-disk has finished playing
//	Scope:		private
//	Input:		channel:	the sound channel that just finished playing
//	Returns:	void
//	
//	This routine will only be called for LONG_ sounds (play from disk).  Therefore,
//	set a flag for the LONG_BANG or LONG_LOOP channel.  The flag will be caught in
//	the main event loop, executing a routine to re-queue a LONG_LOOP sound.  No
//	action is required for a LONG_BANG sound, but the flag is provided for future
//	use.  This function must be fully interrupt-safe, but it is not 68K-compatible
//	(does not restore A5 world).
//--------------------------------------------------------------------------------
// for now, the file completion callback is only called on LOOPED play from disk
static pascal void macCompletion( SndChannelPtr channel )
{
	extern Boolean gSoundAttnFlag, soundFlag[ MAX_CHANNELS ];
	Sound * s;
	
	s = (Sound *)(channel->userInfo);
	gSoundAttnFlag = soundFlag[ s->soundCategory ] = true;
	// main event loop has to watch for gSoundAttnFlag to go true, then call
	// soundCleanUp();
	return;
}



//--------------------------------------------------------------------------------
//	void soundCleanUp( void )
//
//	by Linc Madison, 11/1/96
//	Purpose:	perform any required actions on completion of playing a sound
//	Scope:		public (called from the main event loop)
//	Input:		void
//	Returns:	void
//	
//	Check each of the individual channel's flags and respond accordingly.
//	No cleanup is required on the LONG_BANG channel.  The LONG_LOOP channel
//	just re-queues the same sound for another play.  The SHORT_BANG channel
//	may require that the resource handle be unlocked (if the handles are not
//	kept locked for the whole game).  Cleanup in the SHORT_LOOP channel will
//	only occur when the sound is shut off by an ataStopSound() call, but it
//	will then require the same actions as a SHORT_BANG sound.
//
//	This function is not interrupt-safe.  It must be called by the main event
//	loop in response to (gSoundAttnFlag == true).
//--------------------------------------------------------------------------------
void soundCleanUp( void )
{
	extern Boolean			soundFlag[ MAX_CHANNELS ], gSoundAttnFlag;
	extern SndChannelPtr	soundChan[ MAX_CHANNELS ];
	SndChannelPtr			theChan;
	Sound *					s;
	OSErr					err;
	const long				bufSize = 0x10000;	// 64K buffer again
	
	if(soundFlag[ SHORT_BANG ]) {

		soundFlag[ SHORT_BANG ] = false;
		theChan = soundChan[ SHORT_BANG ];
		// release the handle lock if appropriate
		theChan->userInfo = 0L;

	} else if(soundFlag[ SHORT_LOOP ]) {


		soundFlag[ SHORT_LOOP ] = false;
		theChan = soundChan[ SHORT_LOOP ];
		// Re-queue the same sound to loop over and over again.
		// If userInfo is zero, the sound has been stopped by ataSoundStop().
		if (theChan->userInfo != 0)
		{
			s = (Sound *)(theChan->userInfo);
			ataSoundStop(s);
			ataSoundPlay(s, ATA_SOUND_LOOP);
		}
	} else if (soundFlag[ LONG_BANG ]) {
		// nothing needs doing
		soundFlag[ LONG_BANG ] = false;
		theChan = soundChan[ LONG_BANG ];
		theChan->userInfo = 0L;
	} else if (soundFlag[ LONG_LOOP ]) {
		soundFlag[ LONG_LOOP ] = false;
		theChan = soundChan[ LONG_LOOP ];
		// Re-queue the same sound to loop over and over again.
		// If userInfo is zero, the sound has been stopped by ataSoundStop().
		if (theChan->userInfo != 0)
		{
			s = (Sound *)(theChan->userInfo);
			// SSFP( chan, FileRef, RsrcNum, BufSize, *Buf, AudioSelection, completion, async );
			// NULL buffer pointer means auto-allocate; NULL AudioSel means the whole file
			err = SndStartFilePlay( theChan, s->soundRef, 0, bufSize,
					NULL, NULL, pfCompletion, kAsync );
			if (err != noErr)
			{
				// problem re-queueing loop sound from disk
				// should complain here...
			}
		}
	}
	
	gSoundAttnFlag = false;			// clear the master flag
	return;
}



//--------------------------------------------------------------------------------
//	void _downloadSound( Sound * s )
//
//	by Linc Madison, 11/1/96
//	Purpose:	(none)
//	Scope:		public
//	Input:		(ignored)
//	Returns:	void
//	
//	Present only for linking compatibility with MS-Win source code.
//--------------------------------------------------------------------------------
void _downloadSound( Sound * s )
{
#pragma unused( s )
	return;
}


//--------------------------------------------------------------------------------
//	Sound * ataSoundNew( char * soundFile )
//
//	by Linc Madison, 11/1/96
//	Purpose:	(none)
//	Scope:		public
//	Input:		(ignored)
//	Returns:	(NULL)
//	
//	Present only for linking compatibility with MS-Win source code.
//--------------------------------------------------------------------------------
Sound * ataSoundNew( char * soundFile )
{
	Sound *tmps;
	char tmp_str[2048];
	size_t len;
	Handle th;
	short ref_num;
	ResType type;
	extern FSSpec app_spec;

	len = strlen(soundFile);
	if(len > 255) {
		len = 255;
	}
	tmp_str[0] = len;
	strcpy((void*)&tmp_str[1], soundFile);
	SetResLoad(false);
	th = GetNamedResource('snd ', (void*)tmp_str);
	assert(th);
	GetResInfo(th, &ref_num, &type, (void*)tmp_str);
	SetResLoad(true);
	tmps = (Sound *) malloc(sizeof(Sound));
	assert(tmps);
	tmps->soundRef = ref_num;
	tmps->sound3d = NULL;
	tmps->soundCategory = SHORT_BANG;
	tmps->name = strdup(soundFile);
	return tmps;
}


//--------------------------------------------------------------------------------
//	void ataSoundDelete( Sound * s )
//
//	by Linc Madison, 11/1/96
//	Purpose:	(none)
//	Scope:		public
//	Input:		(ignored)
//	Returns:	void
//	
//	Present only for linking compatibility with MS-Win source code.
//--------------------------------------------------------------------------------
void ataSoundDelete( Sound * s )
{
	free(s);
}

#endif macintosh
