#ifndef _MACSOUND_H_
#define _MACSOUND_H_

// type definitions -- need to be exported to a header file
typedef enum mac_sound_cat {
	CAT_LOOP_FLAG	= 0x01,		// sound will be looped continuously
	CAT_LONG_FLAG	= 0x02,		// sound is played from disk
	SHORT_BANG		= 0,
	SHORT_LOOP		= CAT_LOOP_FLAG,
	LONG_BANG		= CAT_LONG_FLAG,
	LONG_LOOP		= CAT_LONG_FLAG | CAT_LOOP_FLAG,
	CAT_FIRST		= 0,
	CAT_LAST		= 3,
	CAT_COUNT		= CAT_LAST + 1,
	CAT_SHORT		= 0,
	CAT_LONG		= 2
} SoundCat;

typedef struct mac_3dSoundInfo sound3DInfo;	// to be defined later

typedef struct mac_sound {
	char *			name;			// the name of the sound
	sound3DInfo *	sound3d;		// reserved for future use
	long			soundBytes;		// length of the sound
	SoundCat		soundCategory;
	short			soundRef;		// either resource ID or file refnum
} Sound;

// global variables
extern Boolean gSoundAttnFlag;

// Mac-specific global functions
void soundCleanUp( void );		// for the Mac, but needs to be called in main event loop


#endif /* _MACSOUND_H_ */ÿ
