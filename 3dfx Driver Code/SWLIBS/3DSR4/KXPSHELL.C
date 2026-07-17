#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "kview.h"
#include "database.h"

#include "pxp.h"
#include "kxp.h"
#include "dialog.h"

#include "joy.h"

#ifdef DEBUG_OUTPUT
extern FILE * output;
#endif


void DebugSerial( char *buf, ... )
{
   char buffer[1024];
   va_list arglist;
   
   va_start( arglist, buf );
   vsprintf( buffer, buf, arglist );
   va_end( arglist );
   gfx_debug_print( buffer );
}


static State init_state = { VERSION };
static State state = { VERSION };

static cancel = 0;

static Ini_State ini_file = {
  /* resolution_radio = */	RESMODE_640_480,
  /* texturing_radio = */	TEXMODE_POINT,
  /* mode_radio = */		CAMERA_FLIP,
  /* restrict_radio = */	RESTRICT_NONE,
  /* calibrate_radio = */	JOY_CAL_NO
};


#include "init.3de"


int ClientUsesInitDialog(void)
{
   return(0);
}


#pragma off (unreferenced)
void ClientSetStateVar(int id, void *ptr)
{
   OVL o;
   ulong *ul;
   char *s;
   
   ul=(ulong *)ptr;
   s=(char *)ptr;
   o.ul = *ul;
}


ulong ClientGetStateVar(int id)
{
   return 1;
}


int ClientVarSize(int id)
{
   return(1);
}


DlgEntry *ClientDialog(int n)
{
   return(NULL);
}
#pragma on (unreferenced)


char  *ClientGetState(int *size)
{
   *size = sizeof(State);
   return((char *)&state);
}

void ClientResetState()
{
   state = init_state;
}


void feel_ok( Dialog *d, int mouse )
{
   /* If this feel function is the result of a mouse click, * then it
      is necessary to make sure the user releases * the mouse inside
      the region of the button. */
   if(mouse)
      if(!(press_button(d)))
	 return;
   
   see_button(d);
   gfx_butup();
   
   /* this indicates to the dialog library that the dialog * should be
      terminated. */
   dialog_done = 1;
   cancel = 0;
}


void feel_cancel( Dialog *d, int mouse )
{
   /* If this feel function is the result of a mouse click, * then it
      is necessary to make sure the user releases * the mouse inside
      the region of the button. */
   if(mouse)
      if(!(press_button(d)))
	 return;
   
   see_button(d);
   gfx_butup();
   
   /* this indicates to the dialog library that the dialog * should be
      terminated. */
   dialog_done = 1;
   cancel = 1;
}


void read_ini_file (
  Ini_State *		ini_file) {
  FILE *		fp;
  static char		file[1024];
  static char		path[1024];
  static Ini_State	tmp_ini;

  gfx_get_paths(GFX_HOME_PATH, 0, path, file);
  sprintf(file, "%s\\3dfx\\startup.bin", path);
  fp = fopen (file, "rb");

  if (fp == NULL) {
    return;
  }
  else {
    if (fread (&tmp_ini, 1, sizeof (tmp_ini), fp) == sizeof (tmp_ini))
      *ini_file = tmp_ini;
    fclose (fp);
  }		   
}  
  

void write_ini_file (
  Ini_State *		ini_file) {
  FILE *		fp;
  static char		file[1024];
  static char		path[1024];

  gfx_get_paths(GFX_HOME_PATH, 0, path, file);
  sprintf(file, "%s\\3dfx\\startup.bin", path);
  fp = fopen (file, "wb");

  if (fp == NULL) {
    return;
  }
  else {
    fwrite (ini_file, sizeof (*ini_file), 1, fp);
    fclose (fp);
  }		   
}  
  

void ClientStartup(EXPbuf *buf)
{
   FeelSub main_feel[] = {
      OK, feel_ok,
      CANCEL, feel_cancel,
      -1, FNULL
   };


   int		joystick_status = 0;

   
   if(studio_version()<300)
   {
      buf->opcode=buf->usercode=EXP_TERMINATE;
      return;
   }

   get_my_fp_status();

   read_ini_file (&ini_file);

   init_dialog( Startup, NULL, NULL);
   {
     RadSub main_radio[] = {
       RES640, feel_radio, &ini_file.resolution, RESMODE_640_480,
       RES800, feel_radio, &ini_file.resolution, RESMODE_800_600,
       MMNONE, feel_radio, &ini_file.texturing, TEXMODE_NONE,
       MMPOINT, feel_radio, &ini_file.texturing, TEXMODE_POINT,
       MMBI, feel_radio, &ini_file.texturing, TEXMODE_BI,
       MMTRI, feel_radio, &ini_file.texturing, TEXMODE_TRI,
       MODEFLIP, feel_radio, &ini_file.mode, CAMERA_FLIP,
       MODEROT, feel_radio, &ini_file.mode, CAMERA_ROTATE,
       MODEFLY, feel_radio, &ini_file.mode, CAMERA_FLY,
       VIEWALL, feel_radio, &ini_file.restrict, RESTRICT_NONE,
       VIEWSEL, feel_radio, &ini_file.restrict, RESTRICT_SELECTED,
       JOYCALY, feel_radio, &ini_file.calibrate, JOY_CAL_YES,
       JOYCALN, feel_radio, &ini_file.calibrate, JOY_CAL_NO,
       -1, FNULL
     };

     ready_dialog( Startup, NULL, NULL, main_feel, main_radio, NULL, NULL );
   }
   center_dialog( Startup );
   save_under_dialog( Startup );
   draw_dialog( Startup );
   do_dialog( Startup, -1 );
   restore_under_dialog();

   write_ini_file (&ini_file);

   joystick_status = joystick_detect();

   switch (joystick_status) {
   case 0: {
     int		status;
     
     gfx_alert (0,
		"[No joystick detected, check connection.|"
		"Plugin may be unstable.][OK]", status);
   } break;

   case 2: {
     int		status;
     
     gfx_alert (0,
		"[Joystick detected but it needs calibration.|"
		"Starting calibration.][OK]", status);
     joystick_calibrate();
   } break;

   case 1: {
     if (ini_file.calibrate == JOY_CAL_YES)
       joystick_calibrate();
   } break;

   }
  
   if (cancel) {
     buf->opcode   = EXP_NOP;
     buf->usercode = USR_CANCEL;
   }
   else {
     buf->opcode   = EXP_NOP;
     buf->usercode = USR_START;
   }
}


void ClientUserCode(EXPbuf *buf)
{
  char		object_name[11];

  switch(buf->usercode)
    {
    case USR_START:
      buf->opcode   = EXP_NOP;
      buf->usercode = do_kview (NULL,
				ini_file.resolution,
				ini_file.texturing,
				ini_file.mode,
				ini_file.restrict);
      break;

    case USR_SELECT_ROTATION_OBJECT:
      buf->opcode = EXP_PICK_OBJECT;
      buf->usercode = USR_CONTINUE_SELECT_ROTATION_OBJECT;
      strcpy(buf->data.string, "Pick a 3DS mesh to center view on.");
      break;
    case USR_CONTINUE_SELECT_ROTATION_OBJECT:
      strcpy (object_name, buf->data.object.name);
      buf->opcode = EXP_NOP;
      buf->usercode = do_kview (object_name,
				ini_file.resolution,
				ini_file.texturing,
				ini_file.mode,
				ini_file.restrict);
      break;
      
    case USR_END:
      buf->opcode = buf->usercode = EXP_TERMINATE;
      buf->status = 1;
      break;

    case USR_CANCEL:
      buf->opcode = buf->usercode = EXP_TERMINATE;
      buf->status = 1;
      break;
      
    default:
      buf->opcode = buf->usercode = EXP_TERMINATE;
      buf->status = 0;
      break;
   }
}

void ClientTerminate(void)
{
  if (!cancel) {
//    free_db (kxp_database);
    if (!getenv ("DEBUG_NO_SHUTDOWN"))
      atrShutdown ();
  }

  if (malloc_calls != 0) {
    error_printf (__FILE__, __LINE__, "malloc()'s - free()'s = %d\n",
		  malloc_calls);
  }
  
  gfx_clearscreen ();
  gfx_redraw();

#ifdef DEBUG_OUTPUT
  fclose (output);
#endif  
}

int ClientIsUniversal(void) {
  return (1);
}
