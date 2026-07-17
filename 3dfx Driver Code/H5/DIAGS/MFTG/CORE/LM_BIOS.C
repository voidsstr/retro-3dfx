#include <stdio.h>
#include <string.h>
#include <conio.h>

#include "3dfx.h"
#include "fxpci.h"
#include "h3regs.h"
#include "h3defs.h"
#include "h3cinit.h"

#include "lua.h"
#include "luadebug.h"
#include "lualib.h"
#include "lauxlib.h"

#include "mdclua.h"
#include "vgatst\ediag_ex.h"
#include "ediag.h"
#include "vgamisc.h"
#include "banshee.h"
#include "misc.h"


/* BIOS-VGA */
void lm_bios_set_vga_mode(void);
void lm_bios_get_vga_mode(void);
void lm_bios_video_test(void);
void lm_bios_extvideo_test(void);
void lm_bios_vga_core_test(void);
void lm_bios_ddccheck(void);               /* bios DDC check */
void lm_bios_tvOutputEnable(void);         /* uses vga scratch register */


static struct luafn_reg_struct luafn_list[] = {
  /* BIOS-VGA */
  { "set_vga_mode",lm_bios_set_vga_mode},
  { "get_vga_mode",lm_bios_get_vga_mode},
  { "vga_core_test", lm_bios_vga_core_test},
  { "video_test", lm_bios_video_test},
  { "extvideo_test", lm_bios_extvideo_test}, 
  { "ddccheck", lm_bios_ddccheck },
  { "tvOutputEnable", lm_bios_tvOutputEnable },
  { NULL, NULL }};

void lm_bios_init() {
   mdcl_register_fns(luafn_list);
}


// void lm_bios_test_rom(void) {
//   mdc_l_push_fxbool(fnTestVGAROM());
// }


void lm_bios_vga_core_test(void) {
  int retval;

  retval = bansheeTestVGACore(card);

  lua_pushnumber(retval);
  return;
}



void bansheeTestVESAVideo(LPCARDINFO card,void (*cbfn)(int), char *v_parm, int testmode) {
  char *argv[] = { "(exepath)", "-T=NATMODE", NULL, NULL, NULL};
  int argc = 0;
  char **argv_walk = argv;
  char s[60];

  if (testmode == -1) {
       sprintf(s,"-P=ALL"); 
       argv[2] = s;
       argv[3] = v_parm; /* video mode stuff, could be NULL */
  }
  else if (testmode == 0x007f)  {
       sprintf(s,"-P=SELECT"); 
       argv[2] = s;
       argv[3] = v_parm; /* video mode stuff, could be NULL */
  }
  else {
	argv[2] = v_parm; /* video mode stuff, could be NULL */
  }

  printf("calling ediag, args: ");
  while (*argv_walk) {
    printf("argv[%d] = '%s' ",argc,*argv_walk);
    argc++;
    argv_walk++;
  }
  printf("\n");


  ediag_main(argc, argv, cbfn);
}

void bansheeTestVideo(LPCARDINFO card,void (*cbfn)(int), int mode) {
  char *argv[] = { "(exepath)", "-T=VGAMODE" , NULL, NULL, NULL};
  int argc = 0;
  char **argv_walk = argv;
  char s[60];

  if (mode == -1)  {
       sprintf(s,"-P=ALL"); 
       argv[2] = s;
  }
  else if (mode == 0x007f) {
       sprintf(s,"-P=SELECT"); 
       argv[2] = s;
  }
  else {
       sprintf(s,"-P=MODE=%d",mode); 
       argv[2] = s;
  }

  printf("calling ediag, args: ");
  while (*argv_walk) {
    printf("argv[%d] = '%s' ",argc,*argv_walk);
    argc++;
    argv_walk++;
  }
  printf("\n");


  ediag_main(argc, argv,cbfn);
}




void lm_bios_get_vga_mode(void) {
  int retval;

  retval = GetMode();

  lua_pushnumber(retval);
  return;
}

void lm_bios_set_vga_mode(void) {
  int mode;
  lua_Object arg1 = lua_getparam(1);
  int retval;

  if (!lua_isnumber(arg1)) {
    lua_error("lm_bios_set_vga_mode(): arg1 should be a number");
    return;
  }

  mode = lua_getnumber(arg1);

  /* sanity check the mode? */

  retval = SetMode(mode);

  lua_pushnumber(retval);
  return;
}


void lm_bios_extvideo_test(void) {
  char *s = NULL;
  char str[100];
  lua_Object mode_tst = lua_getparam(2);
  lua_Object modeparam = lua_getparam(3);
  int mode,testmode;

  lua_callback_fn = lua_getparam(1);  // callback function
  testmode = lua_getnumber(mode_tst);

  if (testmode == 0x55AA)	/* test all video modes? */
    mode = -1;
  else if (testmode == 0x007f)  /* test subset of video modes */
    mode = 0x7f;
  else   {			 /* else, test individual mode */
    mode = 0;
    if (lua_isstring(modeparam)) {
      s = str;
      sprintf(str,"-P=%s",lua_getstring(modeparam));
    }
  }

  mdc_l_check_card(card);
  bansheeTestVESAVideo(card,mdcl_chksumCallback,s,mode);

/*
 * mdc_l_push_fxbool(
 *    mdc_get_yesno("Did ALL the screens display okay? (y/n) "));
 */

}

void lm_bios_video_test(void) {
  lua_Object mode_tst = lua_getparam(2);
  lua_Object mode_num = lua_getparam(3);
  int mode,testmode;

  lua_callback_fn = lua_getparam(1);  // callback function


  testmode = lua_getnumber(mode_tst);

  if (testmode == 0x55AA)	/* test all video modes? */
    mode = -1;
  else if (testmode == 0x007f)  /* test subset of video modes */
    mode = 0x7f;
  else   {			 /* else, test individual mode */
      mode = lua_getnumber(mode_num);
      printf("video_test(): mode number = %d (0x%X)\n",mode_num,mode_num);
  }

  mdc_l_check_card(card);
  bansheeTestVideo(card,mdcl_chksumCallback,mode);

}




void lm_bios_ddcCallback(int frame,int data) {
  if (lua_callback_fn && lua_isfunction(lua_callback_fn)) {
    lua_pushnumber(frame);
    lua_pushnumber(data);
    lua_callfunction(lua_callback_fn);
  } 
}

int bddc_check(int iter, void (*cbfn)(int count,int data));

void lm_bios_ddccheck(void) {
  lua_callback_fn = lua_getparam(1); // callback function
  lua_pushnumber(bddc_check(5,lm_bios_ddcCallback));
}



/* lm_bios_tvOutputEnable()
 *
 * CRTC Reg. 1Eh defined as follows:
 *    Bit   7: 1-Monitor attached,0-No monitor attached
 *          6: 1-Composite/0-SVideo
 *          5: 1-Turn on TV/0-Turn off TV
 *        4-3: 00-PALBGDHI, 01-PALN
 *             10-PALM,     11-PALNc
 *          2: 1-Turn on LCD/0-Turn off LCD
 *        1-0: 00-CRT, 01-NTSC, 10-PAL
 * 
 * To activate this setting, must issue video mode
 * set afterwards!
 */
void lm_bios_tvOutputEnable(void) {
  int setting;

  int mode = luaL_check_number(1);  /* 0 = CRT,   1 = NTSC, 2 = PAL BGDHI */
				     /* 3 = PAL M, 4 = PAL N, 5 = PAL Nc   */
				     /* 6 = LCD			      */
  int tvout_type = luaL_check_number(2);  /* 1=Composite video, 0=SVideo  */

    mode = mode & 0xFF;
    switch (mode) {
     default:
     case 0:		// Turn on CRT
       setting = 0x00;		// 0x00000000
	break;
     
     case 1:		// Enable NTSC TV
       setting = 0x21;		// 0x00100001
	break;
     
     case 2:		// Enable PAL BGDHI TV
       setting = 0x22;		// 0x00100010
	break;
     
     case 3:		// Enable PAL M TV
       setting = 0x32;		// 0x0011 0010;
	break;
     
     case 4:		// Enable PAL N TV
       setting = 0x2A;		// 0x0010 1010;
	break;
     
     case 5:		// Enable PAL Nc TV
       setting = 0x3A;		// 0x0011 1010;
	break;
     
     case 6:		// Enable LCD Panel
       setting = 0x04;		// 0x0000 0100;
	break;
     
    }
  
    if (tvout_type)	{	// Enable composite video out
      printf("Enabling composite TV output\n");
      setting |= 0x40;		// 0x0100 0000 Set bit 6 for composite video
    }
 
    printf("CR 1E setting = %x\n",setting);

    mdc_l_check_card(card);

    outpw( 0x3D4, (setting << 8) | 0x1E);
  }
