#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <conio.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include "sos.h"
#include "sosm.h"
#include "sosez.h"

#define MAX_SAMPLES  68
#define MAX_BANKS     4
#define MAX_CHANNELS 32
#define MAX_VOLUME 9100

#define MKPITCH(_pitch) ((_pitch)*5)

typedef struct {long    address;
                long    sample_rate;
                long    loop_count;
                long    priority;
} SAMPLE_DATA;

typedef struct {
    SAMPLE_DATA info[64];
} sample_data;

// external variables located in sosez.c
extern   W32  wDIGIDeviceID;
extern   W32  wMIDIDeviceID;
extern   W32  hDIGIDriver;
extern   W32  hMIDIDriver;

static short    loop_table[68];
static short    priority_table[64];
int            is_sound = 0;

// pointer to sample
_SOS_SAMPLE   sSample;

// size of the buffer to use for data 
#define     _DATA_BUFFERSIZE     0x8000

// var for switching buffers when a sample buffer finishes
W32        wSOSSamplePending = _FALSE;

// buffer flag
W32        wSOSActiveSample = 0;   

// pointer to the buffers
PSTR        pBuffer[ 2 ];

unsigned int offset, length;

#define FXFALSE 0
#define FXTRUE  1

PSOSSAMPLE SBs[68][4];
W32        hSample;

void error( int fatal, const char *format, ... ) {
    va_list args;
    va_start( args, format );
    vfprintf( stdout, format, args );
    va_end( args );

    if ( fatal )
      {
        exit( -1 );
      }
}

static void filterSample( _SOS_SAMPLE   *sample) {
   short *p = ( short *) sample->pSample;
   short *q = ( short *) sample->pSample;
   int len = sample->wLength>>3;
   int i;

    for ( i = 0; i < len; i++ ) {
        *p++ = *q++;
        *p++ = *q++;
        q += 2;
    }

    sample->wRate = 22050;
    sample->wLength >>= 1;
    return;

}

FILE *readWaveFile(char *fileName, unsigned int *o, unsigned int *l) {
   FILE *hFile;
   _WAVHEADER waveHeader;

   if ( ( hFile = fopen( fileName, "rb") ) == NULL ) {
       error(1, "could not open file %s\n", fileName);
   }

   printf("opened file %s\n", fileName);

   if (fread( &waveHeader, 1, sizeof(waveHeader), hFile ) <= 0) {
       error(1, "error reading header\n");
   }

   if( !strncmp( (char *)&waveHeader, "RIFF", 0x04 ) ) {
         // set size of the sample and pointer to the sample
         *o = sizeof( waveHeader );
         *l = waveHeader.dwDataLength - sizeof( waveHeader );
         sSample.wBitsPerSample = waveHeader.wBitsPerSample;
         sSample.wChannels      = waveHeader.wChannels;
         if( waveHeader.wBitsPerSample == 0x08 )
            sSample.wFormat    = _PCM_UNSIGNED;
         else
            sSample.wFormat    = 0x00;
         sSample.wRate         = waveHeader.dwSamplesPerSec;
         printf("sample rate %d, bits per sample %d, # channels %d\n",
                sSample.wRate, sSample.wBitsPerSample, sSample.wChannels );
         /* sSample.wRate         = 22050; */
         /* sSample.wChannels      = 1; */

   } else {
       error(1, "not a wave file %s\n", &waveHeader);
   }

   return (hFile);
}


// function to handle the sample done callbacks, the next sample
// will be started in here.
VOID cdecl sosDIGISampleCallback( PSOSSAMPLE pSample )
{

   // check if there is a new sample chunk to play
   if ( sSample.wLength )
   {
      // set length of new sample buffer
      pSample->wLength  =  sSample.wLength;

      // set the buffer sample pointer to the next buffer
      pSample->pSample  =  ( PSTR )pBuffer[ wSOSActiveSample ];

      // set the Processed flag
      wSOSSamplePending = _FALSE;
   }
} 

void streamFile(char *fileName) {
   W32        hSample;
   W32        wReadSize;
   FILE      *hFile;
   int        repeatCount = 2;

   /* pointer to the data buffers for streaming. */

   pBuffer[ 0 ] = (PSTR)malloc( _DATA_BUFFERSIZE );
   pBuffer[ 1 ] = (PSTR)malloc( _DATA_BUFFERSIZE );
   
   // attempt to open file
   if ( ( hFile = readWaveFile( fileName, &offset, &length) ) == NULL ) {
      // display error
      printf( "ERROR: file not found!\n" );

      // free memory allocated for streaming buffers
      free( pBuffer[ 0 ] );
      free( pBuffer[ 1 ] );

      // close streaming file
      fclose( hFile );

      // uninitialize system
      sosEZUnInitSystem();

      // exit
      exit( 1 );
   }

   // read the first buffer
   fseek(hFile, offset, SEEK_SET);
   wReadSize = fread( pBuffer[0], 1, _DATA_BUFFERSIZE, hFile );

   // set size of the sample and pointer to the sample
   sSample.pSample            =  ( PSTR )pBuffer[ 0 ];
   sSample.wLength            =  ( DWORD )wReadSize;
   sSample.wPanPosition       =  _PAN_CENTER;
   sSample.wVolume            =  MK_VOLUME( 0x7fff, 0x7fff );
   sSample.pfnSampleProcessed = sosDIGISampleCallback;

   // start digital sample
   hSample  =  sosDIGIStartSample( hDIGIDriver, &sSample );

   // loop through until the entire file is read and played
   do
   {
      // flip the active sample flag
      wSOSActiveSample ^= 0x01;

      // read in the next buffer to be played and set the size
      wReadSize = fread( pBuffer[wSOSActiveSample], 1, _DATA_BUFFERSIZE,
                         hFile );

      if (wReadSize == 0 ) {
          if ( --repeatCount <= 0 ) {
               sosDIGIStopSample( hDIGIDriver, hSample );
              return;
          }
          fseek(hFile, offset, SEEK_SET);
          printf("repeating track\n");
          wReadSize = fread( pBuffer[wSOSActiveSample], 1, _DATA_BUFFERSIZE, 
                             hFile );
      }

      // setup the size to play
      sSample.wLength = (DWORD)wReadSize;

      // set the sample pending flag
      wSOSSamplePending = _TRUE;

      // wait until the next sample is started
      while( wSOSSamplePending && wReadSize && !kbhit() );

   } while( wReadSize && !kbhit() );

   // wait for sample to complete

   while( !sosDIGISampleDone( hDIGIDriver, hSample ) && !kbhit() )  
      printf( "%x\n",sosDIGISampleDone( hDIGIDriver, hSample ) );

   // free memory allocated for streaming buffers
   free( pBuffer[ 0 ] );
   free( pBuffer[ 1 ] );

   // close streaming file
   fclose( hFile );
}

void LoadSamples(void) {
	int   		fd;
	int			count;
  	int 		i;
	char		filename[50];
	char		tmp_string[5];
    sample_data samp_dat;

	fd=open("samples\\SAMPLES.BIN",O_RDONLY|O_BINARY,0);

	read(fd, &samp_dat, sizeof(samp_dat));
	for(count=0;count<64;count++){
		loop_table[count] = (short)samp_dat.info[count].loop_count;
		priority_table[count] = (short)samp_dat.info[count].priority;
	}
	
	close(fd);

	for(count=0;count<64;count++){
		strcpy(filename , "samples\\");
		itoa(count+1 , tmp_string ,10);
		strcat(filename , tmp_string);
		strcat(filename , ".WAV");

        /* load digital sample */

        if (( SBs[count][0]  =  sosEZLoadSample( filename )) != _NULL ) {
            for ( i = 1; i < 4; i++ ) {
               if ( ( SBs[count][i] = (PSOSSAMPLE)malloc( sizeof( _SOS_SAMPLE  ) ) ) == _NULL) {
                   error(1, "could not clone sound\n");
               }
               *SBs[count][i] = *SBs[count][0];
            }
        }

	}/* for */			 
    printf("loaded sound samples\n");
}

/*
 ** Init_Sound
 *
 *  PARAMETERS:     hwnd    - Window handle, NULL for DOS
 *
 *  DESCRIPTION:    Initialize sound driver
 *
 *  RETURNS:        0 if successful, error code otherwise
 *
 */

int Init_Sound(void *hwnd) {
    int i, j;

    /* retrieve configuration information from .cfg file */

    if ( !sosEZGetConfig( "hmiset.cfg" ) ) {
        /* display error */

        printf( "ERROR : Could not locate 'hmiset.cfg' file. Make\n" );
        printf( "        sure that you have run 'setup.exe' first.\n" );

        exit( 1 );
   }

   /* initialize system */

   if ( sosEZInitSystem( wDIGIDeviceID, wMIDIDeviceID ) ) {
      // display error
      printf( "ERROR :  Could not initialize digital/MIDI driver. Make\n" );
      printf( "         sure that the .386 files are in the current\n" );
      printf( "         directory.\n" );

      // exit
      exit( 1 );
   }

   for ( i = 0; i < MAX_SAMPLES; i++ ) {
       for ( j = 0; j < MAX_BANKS; j++ ) {
           SBs[i][j] = NULL;
       }
   }

   LoadSamples();

   is_sound = 1;

   return (0);
}
/*
 ** CloseSounds
 *
 *  PARAMETERS:     None
 *
 *  DESCRIPTION:    Shutdown sound driver
 *
 *  RETURNS:        Nothing
 *
 */


void CloseSounds(void){
   /* uninitialize system */
   sosEZUnInitSystem();
}

/*
 ** StopSfx
 *
 *  PARAMETERS:     channel sound to stop
 *
 *  DESCRIPTION:    Stop all instances of this sound
 *
 *  RETURNS:        Nothing
 *
 */

void StopSfx(int channel){
    int count;

    if (( channel >= MAX_SAMPLES) || (is_sound==0))
        return; /* return if no sound card. */

    channel /= 10;  /* stop all instances of this sound */

    for ( count = 0; count < MAX_BANKS; count++ ) {
        if(SBs[channel][count]==NULL) continue;

        if ( SBs[channel][count]->wFlags & _SACTIVE ) {
            sosDIGIStopSample( hDIGIDriver, SBs[channel][count]->hSample );
        }
    }
}

/*
 ** StopAllSfx
 *
 *  PARAMETERS:     None
 *
 *  DESCRIPTION:    Stop sound effects on all channels
 *
 *  RETURNS:        Nothing
 *
 */

void StopAllSfx(void){
    int channel, count;

    if(is_sound==0) return; /* return if no sound card. */

    for ( channel = 0; channel < MAX_SAMPLES; channel++ ) {
        for ( count = 0; count < MAX_BANKS; count++ ) {
            if(SBs[channel][count]==NULL) continue;

            if ( SBs[channel][count]->wFlags & _SACTIVE ) {
                sosDIGIStopSample( hDIGIDriver, SBs[channel][count]->hSample );
            }
        }
    }
}

/*
 ** StartSfx
 *
 *  PARAMETERS:     FxNum   - The sound effect number to play.
 *
 *  DESCRIPTION:    Plays the desired effect.Each effect has 4 banks.
 *
 *  RETURNS:        FxNum*10+(sound bank) - Gives sound effect and bank number of effect.
 *
 */

int StartSfx(int FxNum) {
    int count = 0;

    if (( FxNum >= MAX_SAMPLES) || (is_sound==0))
        return 0; /* return if no sound card. */

    for(count=0;count<4;count++){
        if(SBs[FxNum][count]==NULL) continue;
     
        if ( ( SBs[FxNum][count]->wFlags & _SACTIVE ) == 0 ) {
            if(loop_table[FxNum]==0) /* Looping Sound Effect. */
                 SBs[FxNum][count]->wLoopCount = -1;
            else SBs[FxNum][count]->wLoopCount = 0 ;
            hSample  =  sosDIGIStartSample( hDIGIDriver, SBs[FxNum][count] );
            return(FxNum*10+count);
        }
    }

    return(FxNum*10+count);
}
/*
 ** StartSpeech
 *
 *  PARAMETERS: FxNum - Speech sample to play.
 *              vol   - Volume of sample.
 *
 *  DESCRIPTION:Sets Volume and plays sample.
 *
 *  RETURNS:    returns 0.
 *
 */

int StartSpeech(int FxNum, int vol){
    if (( FxNum >= MAX_SAMPLES) || (is_sound==0))
        return (0); /* return if no sound card. */

    if(vol > MAX_VOLUME) {
        vol = MAX_VOLUME;
    }

    if(vol < 0) {
        vol = 0;
    }

    return 0;
}

/*
 ** StartSfxAttr
 *
 *  PARAMETERS: FxNum - Sound Effect Number to play.
 *              vol   - Volume (range: 0 and MAX_VOLUME)
 *              pitch - Frequency of the effect. ( range: 0x700, 0xA00 ) 
 *              pan   - pan.
 *
 *  DESCRIPTION:Starts fx with required parameters.
 *
 *  RETURNS:    FxNum*10+(sound bank)
 *
 */

int StartSfxAttr(int FxNum, int vol, int pitch, int pan){
    int count = 0;

    if (( FxNum >= MAX_SAMPLES) || (is_sound==0))
        return 0; /* return if no sound card. */

    for(count=0;count<4;count++){
        if(SBs[FxNum][count]==NULL) continue;
     
        if ( ( SBs[FxNum][count]->wFlags & _SACTIVE ) == 0 ) {
            if(loop_table[FxNum]==0) /* Looping Sound Effect. */
                 SBs[FxNum][count]->wLoopCount = -1;
            else SBs[FxNum][count]->wLoopCount = 0 ;
            if ( (hSample  =  sosDIGIStartSample( hDIGIDriver, SBs[FxNum][count] )) != _ERR_NO_SLOTS ) {
                sosDIGISetSampleVolume(hDIGIDriver, hSample, MK_VOLUME(7*vol, 7*vol));
                sosDIGISetSampleRate(hDIGIDriver, hSample, MKPITCH(pitch));
                sosDIGISetPanLocation(hDIGIDriver, hSample, (pan<<8)+_PAN_CENTER);
            }
            return(FxNum*10+count);
        }
    }

    return(FxNum*10+count);
}

/*
 ** SetSfxVol/Pan/Pitch
 *
 *  DESCRIPTION:functions for setting volume,pan,and pitch of a sample.
 *
*/

void SetSfxVol(int channel, int vol){

    if (( (channel/10) >= MAX_SAMPLES) || (is_sound==0))
        return; /* return if no sound card. */

    if((channel!=-1) && (SBs[channel/10][channel%10] != NULL)) {
       if(vol > MAX_VOLUME) {
           vol = MAX_VOLUME;
       }
       if(vol < 0) {
           vol = 0;
       }
       sosDIGISetSampleVolume(hDIGIDriver, SBs[channel/10][channel%10]->hSample, MK_VOLUME(7*vol, 7*vol));
   }
}

void SetSfxPitch(int channel, int pitch){

    if (( (channel/10) >= MAX_SAMPLES) || (is_sound==0))
        return; /* return if no sound card. */

    if((channel!=-1) && (SBs[channel/10][channel%10] != NULL)) {
        sosDIGISetSampleRate(hDIGIDriver, SBs[channel/10][channel%10]->hSample, MKPITCH(pitch));
    }
}

void SetSfxPan(int channel, int pan){

    if (( (channel/10) >= MAX_SAMPLES) || (is_sound==0))
        return; /* return if no sound card. */

    if((channel!=-1) && (SBs[channel/10][channel%10] != NULL)) {
        sosDIGISetPanLocation(hDIGIDriver, SBs[channel/10][channel%10]->hSample, (pan<<10)+_PAN_CENTER);
    }
}

void SetSfxAttr(int channel, int vol, int pitch, int pan){

    if (( (channel/10) >= MAX_SAMPLES) || (is_sound==0))
        return; /* return if no sound card. */

    if((channel!=-1) && (SBs[channel/10][channel%10] != NULL)) {
       if(vol > MAX_VOLUME) {
           vol = MAX_VOLUME;
       }
       if(vol < 0) {
           vol = 0;
       }
       sosDIGISetSampleVolume(hDIGIDriver, SBs[channel/10][channel%10]->hSample, MK_VOLUME(7*vol, 7*vol));
       sosDIGISetSampleRate(hDIGIDriver, SBs[channel/10][channel%10]->hSample, MKPITCH(pitch));
       sosDIGISetPanLocation(hDIGIDriver, SBs[channel/10][channel%10]->hSample, (pan<<10)+_PAN_CENTER);
    }
}

VOID  main( int argc, char **argv ) {
   Init_Sound(NULL);

   streamFile(argv[1]);

   StartSfxAttr(11, MAX_VOLUME, 0x700, 0);

   getchar();

   StartSfxAttr(11, MAX_VOLUME, 0xA00, 0);

   getchar();

   StartSfxAttr(11, MAX_VOLUME/1.3, 0xA00, 0);

   getchar();

   StartSfxAttr(11, MAX_VOLUME, 0xA00, -32);

   getchar();

   StartSfx(11);

   getchar();

   StartSfx(7);

   getchar();

   StartSfx(11);

   getchar();

   CloseSounds();
}
