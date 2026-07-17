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
** $Date: 10/11/00 7:31:23 PM$ 
**
*/

#include <atutil.h>

#ifdef __WIN32__

#undef   _WIN32
#define  _WIN32
#include "dsound.h"
#include "wave.h"

/*-----------------------------------------------------------------
  Module Data Definitions
  -----------------------------------------------------------------*/
typedef struct {
    char                *name;
    LPDIRECTSOUNDBUFFER dsBuffer;
    DWORD               soundBytes;
    void                *soundData;
} Sound;

#define  _SOUND_IMPLEMENTATION_
#include "ataudio.h"

/*-----------------------------------------------------------------
  Module Storage
  -----------------------------------------------------------------*/
static LPDIRECTSOUND lpDS;

/*-----------------------------------------------------------------
  Module Implementation
  -----------------------------------------------------------------*/
void _downloadSound( Sound *s ) {
    FxBool done = FXFALSE;
    void  *ptr1, *ptr2;
    FxU32 len1, len2;
    int err;
    while( !done ) {
        err = IDirectSoundBuffer_Lock(s->dsBuffer, 0, s->soundBytes, 
                                      &ptr1, &len1, &ptr2, &len2, 0 );
        switch( err ) {
          case DS_OK:
            memcpy( ptr1, s->soundData, len1 );
            if ( ptr2 ) memcpy( ptr2, (char*)s->soundData+len1, len2 );
            IDirectSoundBuffer_Unlock(s->dsBuffer, ptr1, len1, ptr2, len2 );
            done = FXTRUE;
            break;
          case DSERR_BUFFERLOST:
            err = IDirectSoundBuffer_Restore( s->dsBuffer );
            if ( err != DS_OK ) done = FXTRUE;
            break;
          default:
            atuError( FXTRUE, "Error locking dsBuffer.\n" );
            break;
        }
    }
    if ( err != DS_OK ) 
      atuError( FXTRUE, "Couldn't download sound data.\n" );
    return;
}

/*-------------------------------------------------------------------
  Function: ataSoundPlay
  Date: 7/26
  Implementor(s): jdt, da
  Library: AT Audio
  Description: 
  Play a sound.
  Arguments:
  s - sound to play
  flags - if 0, plays the sound once and stops
        - if ATA_SOUND_LOOP, loops the sound when it's overn.
  Return:
  none
  -------------------------------------------------------------------*/
void ataSoundPlay( Sound *s, int flags ) {
    FxBool done = FXFALSE;
    int err;
    if ( flags != 0 && flags != ATA_SOUND_LOOP )
        atuError( FXTRUE, "soundPlay: 2nd parameter must be 0 or ATA_SOUND_LOOP.\n" );
    while( !done ) {
        err = IDirectSoundBuffer_Play( s->dsBuffer, 0, 0, flags );
        switch( err ) {
          case DS_OK:
            done = FXTRUE;
            break;
          case DSERR_BUFFERLOST:
            _downloadSound( s );
            break;
          default:
            atuError( FXTRUE, "soundPlay: Error playing sound.\n" );
            break;
        }
    }
    return;
}


/*-------------------------------------------------------------------
  Function: ataSoundStop
  Date: 7/26
  Implementor(s): jdt
  Library: AT Audio
  Description:
  Kill a sound that is looping or playing
  Arguments:
  s - sound to kill
  Return:
  none
  -------------------------------------------------------------------*/
void  ataSoundStop( Sound *s ) {
    IDirectSoundBuffer_Stop( s->dsBuffer );
    IDirectSoundBuffer_SetCurrentPosition( s->dsBuffer, 0 );
    return;
}


/*-------------------------------------------------------------------
  Function: ataSoundNew
  Date: 7/16/96
  Implementor(s): jdt
  Library: AT Audio
  Description: 
  Create a new sound from an on disk audio file.  File must be in
  current directory.
  Arguments:
  soundFile - path to sound file
  Return:
  pointer to newly allocated sound structure
  -------------------------------------------------------------------*/
Sound *ataSoundNew( char *soundFile ) {
    int err;

    DWORD dummy;
    WAVEFORMATEX *fmt;
    DSBUFFERDESC dsbd;

    Sound *s = calloc( sizeof( Sound ), 1 );
    
    s->name = strdup( soundFile );

    /*---------------------------------------------------
      Load Wave Data
      ---------------------------------------------------*/
    err = WaveLoadFile( soundFile, 
                        &(s->soundBytes),
                        &dummy, 
                        &fmt,
                        &((char*)s->soundData) );
    if ( err ) {
        atuError( FXFALSE, "soundNew: Error loading sound file.\n" );
        return 0;
    }

    /*---------------------------------------------------
      Create DSB
      ---------------------------------------------------*/
    memset( &dsbd, 0, sizeof( dsbd ) );
    dsbd.lpwfxFormat   = (LPWAVEFORMATEX)fmt;
    dsbd.dwSize        = sizeof( DSBUFFERDESC );
    dsbd.dwBufferBytes = s->soundBytes;
    dsbd.dwFlags       = 0;

    err = IDirectSound_CreateSoundBuffer( lpDS, &dsbd, &(s->dsBuffer), 0 );
    if ( err != DS_OK ) {
      atuError( FXFALSE, "soundNew: Failed to create secondary buffer." );
      ataSoundDelete( s );
      ataShutdown();
      return 0;
  }

    /*---------------------------------------------------
      Fill DSB
      ---------------------------------------------------*/
    _downloadSound( s );

    GlobalFree( fmt );
    return s;
}

/*-------------------------------------------------------------------
  Function: ataSoundDelete
  Date: 7/16
  Implementor(s): jdt
  Library: AT Audio
  Description:
  Delete a sound allocated with ataSoundNew
  Arguments:
  s - sound to delete
  Return:
  none
  -------------------------------------------------------------------*/
void ataSoundDelete( Sound *s ) {
    if ( s ) {
        if ( s->name ) free( s->name );
        if ( s->soundData ) GlobalFree( s->soundData );
        if ( s->dsBuffer ) IDirectSound_Release( s->dsBuffer );
    } else 
      atuError( FXFALSE, "Tried to delete a null pointer.\n" );
}

/*-------------------------------------------------------------------
  Function: ataInit
  Date: 7/16
  Implementor(s): jdt
  Library: AT Audio
  Description:
  Initialize AT audio library.  As library is built on DSOUND this requires
  the handle of the application window.  Sounds are automatically muted when
  the application window loses focus.
  Arguments:
  handle - windows handle
  Return:
     FXTRUE if scuccessful, FXFALSE otherwise
  -------------------------------------------------------------------*/
FxBool ataInit( FxU32 handle ) {
    int err;
    HWND winHandle = (HWND)handle;

    err = DirectSoundCreate( 0, &lpDS, 0 );
    if ( err != DS_OK ) {
        return FXFALSE;
    }

    if ( !handle ) {
        return FXFALSE;
    }

    err = IDirectSound_SetCooperativeLevel( lpDS, winHandle, DSSCL_NORMAL );
    if ( err != DS_OK ) {
        return FXFALSE;
    }

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: ataShutdown
  Date: 7/16
  Implementor(s): jdt
  Library: AT Audio
  Description:
  Shutdown audio library
  Arguments:
  none
  Return:
  none
  -------------------------------------------------------------------*/
void ataShutdown() {
    if ( lpDS ) IDirectSound_Release( lpDS );
}

#else

/*-----------------------------------------------------------------
  Module Data Definitions
  -----------------------------------------------------------------*/
typedef struct {
    int unused;
} Sound;

#define  _SOUND_IMPLEMENTATION_
#include "ataudio.h"

void  ataInit( FxU32 handle ) {
}

void  ataShutdown() {
}

Sound *ataSoundNew( char *soundFile ) {
    Sound *s = 0;
    FXUNUSED( soundFile );
    return s;
}

void ataSoundStop( Sound *s ) {
   FXUNUSED( s );
   return;
}

void  ataSoundDelete( Sound *s ) {
    FXUNUSED( s );
}

void  ataSoundPlay( Sound *s, int flags ) {
    FXUNUSED( s );
}

#endif


