/* banshee */

#include <stdio.h>
#include <stdlib.h>
#include <i86.h>
#include <math.h>

#include "lua.h"
#include "luadebug.h"
#include "lualib.h"
#include "lauxlib.h"

#include "3dfx.h"
#include "fxpci.h"
#include "glide.h"
#include "h3regs.h"
#include <h3cinit.h>

#include "mdclua.h"
#include "memtst.h"

#include "banshee.h"



void lm_ban_pci_test(void);
void lm_ban_agp_test(void);
void lm_ban_fbi_test(void);
void lm_ban_sli_fbi_test(void);
void lm_ban_fbi_size(void);
void lm_ban_mem_clock(void);
void lm_ban_bansheeCrc(void);
void lm_ban_bansheeMemCrc(void);
void lm_ban_bansheeSliMemCrc(void);
// void lm_ban_test_rom(void);
void lm_ban_hwCursor(void);
void lm_ban_desktopstart_check(void);
void lm_ban_bansheeChecksumRom(void);
void lm_ban_bansheeCheckSerialRom(void);
void lm_ban_bansheeCheckVPD(void);
void lm_ban_banBOvl(void);                /* overlay enable */
void lm_ban_h3InitPlls(void);             /* grx, memclock */
void lm_ban_setmclk(void);
void lm_ban_waitForVsync(void);
void lm_ban_blit2d(void);
void lm_ban_resetvoodoo3(void);

void lm_ban_constFill(void);
void lm_ban_alternateFill(void);
void lm_ban_rndmFill(void);

static struct luafn_reg_struct luafn_list[] = {
  { "pci_test",lm_ban_pci_test},
  { "agp_test",lm_ban_agp_test},
  { "fbi_test",lm_ban_fbi_test},
  { "sli_fbi_test",lm_ban_sli_fbi_test},
  { "fbi_size", lm_ban_fbi_size },
  { "mem_clock", lm_ban_mem_clock },
  { "bansheeCrc", lm_ban_bansheeCrc },
  { "bansheeMemCrc", lm_ban_bansheeMemCrc },
  { "bansheeSliMemCrc", lm_ban_bansheeSliMemCrc },
//   { "test_rom", lm_ban_test_rom },
  { "hwCursor", lm_ban_hwCursor },
  { "desktopstart_check", lm_ban_desktopstart_check },
  { "checksumRom", lm_ban_bansheeChecksumRom },
  { "serial_rom_test", lm_ban_bansheeCheckSerialRom },
  { "checkVPD", lm_ban_bansheeCheckVPD },
  { "banBOvl",lm_ban_banBOvl },
  { "h3InitPlls", lm_ban_h3InitPlls },
  { "setmclk", lm_ban_setmclk },
  { "waitForVsync", lm_ban_waitForVsync },
  { "reset_voodoo3", lm_ban_resetvoodoo3 },
  { "blit2d", lm_ban_blit2d },
  { "constFill", lm_ban_constFill },
  { "alternateFill", lm_ban_alternateFill },
  { "rndmFill", lm_ban_rndmFill },
  { NULL, NULL }};


void lm_ban_rndmFill(void) {
  FxU32 seed = luaL_check_number(1);
  FxU32 memSize;

  memSize =  TO_MB(bansheeFbiSize(card));
  mdc_l_check_card(card);
  lua_pushnumber(rndmFill((FxU32 *)card->NatMem1.MappedAddr, memSize,seed));
}


void lm_ban_constFill(void) {
  FxU32 pattern = luaL_check_number(1);
  FxU32 memSize;

  memSize =  TO_MB(bansheeFbiSize(card));
  mdc_l_check_card(card);
  lua_pushnumber(constFill((FxU32 *)card->NatMem1.MappedAddr, memSize, pattern));
}

void lm_ban_alternateFill(void) {
  FxU32 pat1 = luaL_check_number(1);
  FxU32 pat2 = luaL_check_number(2);
  FxU32 memSize;

  memSize =  TO_MB(bansheeFbiSize(card));
  mdc_l_check_card(card);

  lua_pushnumber(alternateFill((FxU32 *)card->NatMem1.MappedAddr, memSize, pat1, pat2));
}

void init_banshee_module();
void lm_ban_init() {
   init_banshee_module(); // banshee.c
   mdcl_register_fns(luafn_list);
}

void lm_ban_hwCursor(void) {
  int enable = luaL_check_number(1);
  int xLoc   = luaL_check_number(2);
  int yLoc   = luaL_check_number(3);

  mdc_l_check_card(card);

  bansheeHWCursor(card,enable,xLoc,yLoc);

}

void lm_ban_desktopstart_check(void) {
  FxU32 startaddr = luaL_check_number(1);

  mdc_l_check_card(card);
  bansheeDesktopStartCheck(card, startaddr);

}

void lm_ban_resetvoodoo3(void) {
  
}

void lm_ban_waitForVsync(void) {
  int state = luaL_check_number(1);
  mdc_l_check_card(card);

  bansheeWaitForVsync(card,state);
}




void lm_ban_bansheeCrc(void) {
  mdc_l_check_card(card);
  lua_pushnumber((double)bansheeCrc(card));
}

void lm_ban_bansheeMemCrc(void) {
  mdc_l_check_card(card);
  lua_pushnumber((double)bansheeMemCrc(card));
}

void lm_ban_bansheeSliMemCrc(void) {
  mdc_l_check_card(card);
  lua_pushnumber((double)bansheeSliMemCrc(card));
}

void ban_bilinearOvl(LPCARDINFO);

void lm_ban_banBOvl(void) {
  mdc_l_check_card(card);

  ban_bilinearOvl(card);
  
}


void lm_ban_setmclk(void) {
  double desired_freq = luaL_check_number(1);
  FxU32 restore_pll = luaL_check_number(2);
  int verbose = luaL_check_number(3);
  SstIORegs *regs;
  FxU32 old_pllCtrl1;
  FxDouble actual_freq, float_n, temp_error;
  FxDouble loop_freq, ref = 14.31818L, comp_freq, tol = .5L,best_tol;
  int temp_m, temp_n, temp_p, pllctrl1;
  int i, n, m;
  int best_m=0, best_n=0, best_p=0;

  mdc_l_check_card(card);
  
  regs = (SstIORegs *)card->NatMem0.MappedAddr;

  if (restore_pll)	{
     regs->pllCtrl1 = restore_pll;
     printf("PLLCTRL1 register has been restored to %X.\n",restore_pll);
     return;
  }

  old_pllCtrl1 = regs->pllCtrl1;
  printf("\nOriginal setting of PLLCTRL1 register = %X.\n",old_pllCtrl1);

  best_tol = tol;

  printf("Detecting the new PLL setting for frequency of %lf...\n",desired_freq);

  if (verbose) {
     printf("  N        M   P   Act Freq    %%Error   Loop Freq    Comp Freq  PLLCTRL\n");
  }
  for (temp_m=1; temp_m<63; temp_m++) {
     for (temp_p=1; temp_p<3; temp_p++)
	 {
	 float_n = ((( ((desired_freq * pow(2.0, (double)temp_p))) * (double)(temp_m + 2))) / ref);
	 float_n = float_n - 2;
         temp_n = (int) floor(float_n);

	 if (temp_n > 255) temp_n = 255;
         if (temp_n < 0) temp_n = 0;

         for (i=0; i<2; i++) {
            loop_freq = (ref * (double)(temp_n+i + 2)) / (double)(temp_m + 2);
            actual_freq = loop_freq / pow(2.0, (double)temp_p);
            temp_error = 100 * fabs(actual_freq - desired_freq) / desired_freq;
            comp_freq = ref / (double)(temp_m + 2);

            if ((temp_error < tol) && ((temp_n+i) < 256)) {
               n = (temp_n+i) << 8;
	       m = temp_m << 2;
	       pllctrl1 = n | m | temp_p;
	       if (verbose) {
                   printf("%3d(%2X)(%1d)  %3d  %1d  %10.5lf  %6.3lf   %10.5lf  %10.5lf    %4X\n",
                      temp_n+i,temp_n+i,i, temp_m, temp_p, actual_freq, temp_error,
                      loop_freq, comp_freq, pllctrl1);
    	       }
	       if ((temp_error < best_tol) && (temp_p == 1)) {
		 if ((temp_m ==1) || (temp_m == 3)) {
		    best_tol = temp_error;
	            best_n = temp_n+i;
	            best_m = temp_m;
	            best_p = temp_p;
		 }
	       }
	    }
           }   /* end for i=0 */
         }  /* end for temp_p */
	}  /* end for temp_m  */

        loop_freq = (ref * (double)(best_n + 2)) / (double)(best_m + 2);
        actual_freq = loop_freq / pow(2.0, (double)best_p);
        temp_error = 100 * fabs(actual_freq - desired_freq) / desired_freq;
        n = best_n << 8;
        m = best_m << 2;
        pllctrl1 = n | m | best_p;

        printf("\nNew setting for PLLCTRL1 is:\n");
        printf("   N    M    P   Act Freq  %%Error    PLLCTRL1\n");
        printf("  %3d %3d   %1d   %10.5lf  %6.3lf    %4X\n",
                   best_n, best_m, best_p, actual_freq, temp_error,
                   pllctrl1);

     regs->pllCtrl1 = pllctrl1;
     printf("\nPLLCTRL1 register has been updated to %X.\n\n",pllctrl1);

     lua_pushnumber(old_pllCtrl1);
}


void lm_ban_h3InitPlls(void) {
  int grxClock, memClock;
  SstIORegs *regs;
//  FxU32 dramInit1;

  mdc_l_check_card(card);

  grxClock = luaL_check_number(1);
  memClock = luaL_check_number(2);

  if (!grxClock) {
     grxClock = 100;
  }
  if (!memClock) {
     memClock = 100;
  }

#if 0
  if (getenv("SSTH3_MEMCLOCK")) {
    printf("(before) SSTH3_MEMCLOCK = %s\n", getenv("SSTH3_MEMCLOCK"));
  }
  // set environment variables
  sprintf(s,"SSTH3_GRXCLOCK=%d",grxClock);
  putenv(s);
  sprintf(s,"SSTH3_MEMCLOCK=%d",memClock);
  putenv(s);
  
  if (getenv("SSTH3_MEMCLOCK")) {
    printf("(after) SSTH3_MEMCLOCK = %s\n", getenv("SSTH3_MEMCLOCK"));
  }
#endif

  printf("setting plls (iobase = 0x%X), grx = %d, mem = %d\n",
	card->PCIBase2, grxClock, memClock);

  h3InitPlls(card->PCIBase2, grxClock, memClock);

  regs = (SstIORegs *)card->NatMem0.MappedAddr;
#ifdef SCREW_UP_DRAMINIT1
  dramInit1 = regs->dramInit1;

  if (memClock < 100) {
    printf("setting slow dramInit1, old = 0x%X, new = 0x%X\n",dramInit1,
  	regs->dramInit1 = (dramInit1 & (1 << 30) ) | 0x24C031);
  } else {
    printf("setting fast dramInit1, old = 0x%X, new = 0x%X\n",dramInit1,
  	regs->dramInit1 = (dramInit1 & (1 << 30) ) | 0x20C031);
  }
#endif
}


void lm_ban_bansheeChecksumRom(void) {
  char version_str[30];
  int  calc_mb_cksm=0;
  int  check_cksm  = luaL_check_number(1);
  int  mb_bios_cksm = luaL_check_number(2);

  mdc_l_check_card(card);

  if (bansheeChecksumRom(card, mb_bios_cksm,version_str,check_cksm,&calc_mb_cksm) == FXTRUE) {
    /* it worked! */
    lua_pushnumber(FXTRUE);
    lua_pushstring(version_str);
    lua_pushnumber(calc_mb_cksm);
  }
  else {
    /* it failed! */
    lua_pushnumber(FXFALSE);
    lua_pushstring(version_str);
    lua_pushnumber(calc_mb_cksm);
  }
}


void lm_ban_bansheeCheckSerialRom(void) {

  mdc_l_check_card(card);
  mdc_l_push_fxbool(bansheeCheckSerialRom(card));
}

void lm_ban_bansheeCheckVPD(void) {
  char *assembly_number = luaL_check_string(1);
  int assbly_size = luaL_check_number(2);
  char *serial_number = luaL_check_string(3);
  char *eco_level = luaL_check_string(4);
  int  suppress_msg = luaL_check_number(5);

  mdc_l_check_card(card);
  mdc_l_push_fxbool(bansheeCheckVPD(card, assembly_number, assbly_size, serial_number, eco_level, suppress_msg));
}


lua_Object mdc_getluaindex(int ref, char *index) {
  lua_pushobject(lua_getref(ref));
  lua_pushstring(index);
  return (lua_gettable());
}

void lm_ban_blit2d(void) {
  lua_Object p_obj = lua_getparam(1);
  int param;

  lua_pushobject(p_obj);
  param = lua_ref(0);

  mdc_l_check_card(card);

  if (!lua_istable(p_obj)) {
    lua_error("blit2d() needs a table as the first argument");
  }

  if (!lua_isnil(mdc_getluaindex(param,"dstX"))) {
    b2d_setdest(lua_getnumber(mdc_getluaindex(param,"dstX")),
                lua_getnumber(mdc_getluaindex(param,"dstY")));
  }

  if (!lua_isnil(mdc_getluaindex(param,"srcX"))) {
    b2d_setsrc(lua_getnumber(mdc_getluaindex(param,"srcX")),
                lua_getnumber(mdc_getluaindex(param,"srcY")));
  }

  if (!lua_isnil(mdc_getluaindex(param,"colorFore"))) {
    b2d_setcolor((FxU32)lua_getnumber(mdc_getluaindex(param,"colorFore")));
  }

  if (!lua_isnil(mdc_getluaindex(param,"srcSizeX"))) {
    b2d_setsrcsize(lua_getnumber(mdc_getluaindex(param,"srcSizeX")),
                   lua_getnumber(mdc_getluaindex(param,"srcSizeY")));
  }

  if (!lua_isnil(mdc_getluaindex(param,"dstSizeX"))) {
    b2d_setdstsize(lua_getnumber(mdc_getluaindex(param,"dstSizeX")),
                   lua_getnumber(mdc_getluaindex(param,"dstSizeY")));
  }
 
  banshee2dBlit(card,lua_getnumber(mdc_getluaindex(param,"blitType")));
}


void lm_ban_pci_test(void) {
  FxU32 testcode = luaL_check_number(1);
  FxU32 SSTRegister = luaL_check_number(2);
  FxU32 mask = luaL_check_number(3);

    /* do tests on this card */
  mdc_l_check_card(card);
  mdc_l_push_fxbool(bansheePciTest(card,testcode,SSTRegister,mask));
}

void lm_ban_agp_test(void) {
  FxU32 testcode = luaL_check_number(1);   // 2-AGP 2x, 4-AGP 4x
    /* do tests on this card */
  mdc_l_check_card(card);
  mdc_l_push_fxbool(bansheeAgpTest(card,testcode));
}

void lm_ban_fbi_test(void) {
  mdc_l_check_card(card);
  lua_callback_fn = lua_getparam(1);  // callback function

  bansheeFbiTest(card,mdcl_chksumCallback);

}

void lm_ban_sli_fbi_test(void) {
  mdc_l_check_card(card);
  mdc_l_push_fxbool(bansheeSliFbiTest(card));

}

void lm_ban_fbi_size(void) {
  mdc_l_check_card(card);

  lua_pushnumber(bansheeFbiSize(card));
}

void lm_ban_mem_clock(void) {
  mdc_l_check_card(card);

  lua_pushnumber((double)bansheeMemClock(card));
}
