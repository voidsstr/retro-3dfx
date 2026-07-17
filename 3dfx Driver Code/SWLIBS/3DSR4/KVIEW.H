/*
 * kview.h
 */

/* User codes. */
#define USR_START				(0x0200)
#define USR_SELECT_ROTATION_OBJECT		(0x0201)
#define USR_CONTINUE_SELECT_ROTATION_OBJECT	(0x0202)
#define USR_END					(0x0210)
#define USR_CANCEL				(0x0211)

/* Version test value */
#define VERSION (0x121a7ddf)


/* Global variables */

typedef struct
{
   unsigned long version;
} State;


extern int do_kview(char *, int, int, int, int);
extern void DebugSerial(char *, ...);


#define TRUE	(-1)
#define FALSE	(0)


/* constants to make the code easier to decipher */
#define TEXMODE_NONE	(0)
#define TEXMODE_POINT	(1)
#define TEXMODE_BI	(2)
#define TEXMODE_TRI	(3)

#define RESMODE_640_480 (0)
#define RESMODE_800_600 (1)

#define RESTRICT_NONE	(0)
#define RESTRICT_SELECTED (1)

#define CAMERA_FLY	(2)
#define CAMERA_FLIP	(1)
#define CAMERA_ROTATE	(4)
#define CAMERA_ANIMATE	(8)

#define JOY_CAL_YES	(0)
#define JOY_CAL_NO	(1)
